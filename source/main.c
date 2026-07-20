#include "common.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int scrollTick = 0;
int lastSelected = -1;
AppScreen currentScreen = SCREEN_BROWSER;
int settingsPage = 0;
int settingsCursor = 0;

int selected = 0; // prevent invalid index before first dir scan - arcmage
int entryCount = 0; // empty state must be valid for selection checks - arcmage

static void safestrncpy(char *dst, const char *src, size_t size) {
    if (size == 0) return;
    strncpy(dst, src, size - 1);
    dst[size - 1] = '\0';
} // strncpy wrapper that guarantees null termination - arcmage

static bool validateselection(void) {
    return selected >= 0 && selected < entryCount;
}

#define AUDIO_BUFFER_SIZE (1024 * 1024)

int main(void) {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    topTarget = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    botTarget = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    dynBuf    = C2D_TextBufNew(1024);

    ndspInit();
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    audioBuf = (u8 *)linearAlloc(AUDIO_BUFFER_SIZE); // allocate fixed size audio buffer used by decoder - arcmage
    if (!audioBuf) {
        gfxExit();
        return -1;
    } // allocation check to fix potential memory issues - arcmage
    mpg123_init();
    ptmuInit();

    aptSetSleepAllowed(false);

    loadTheme();
    scanDir(currentDir);

    touchPosition touch;
    bool touching = false;
    int touchStartY = 0;
    int lastTouchY = 0;
    int scrollOffsetAtTouchStart = 0;

    srand(osGetTime());

    while (aptMainLoop()) {
        hidScanInput();
        hidTouchRead(&touch);
        u32 down = hidKeysDown();
        u32 held = hidKeysHeld();

        if (down & KEY_START) break;

        u8 shellState = 1;
        PTMU_GetShellState(&shellState);
        bool lidClosed = (shellState == 0);

        if (currentScreen == SCREEN_SETTINGS) {
            if (down & KEY_L) {
                settingsPage = (settingsPage + 2) % 3;
            }
            if (down & KEY_R) {
                settingsPage = (settingsPage + 1) % 3;
            }

            if (settingsPage == 0) {
                if (down & KEY_DOWN) {
                    if (currentTheme < THEME_COUNT - 1) currentTheme++;
                }
                if (down & KEY_UP) {
                    if (currentTheme > 0) currentTheme--;
                }
            } else if (settingsPage == 1) {
                // Audio Effects (Speed & Pitch)
                if (down & KEY_DRIGHT) {
                    setPlaybackSpeed(playbackSpeed + 0.1f);
                }
                if (down & KEY_DLEFT) {
                    setPlaybackSpeed(playbackSpeed - 0.1f);
                }
                if (down & KEY_DUP) {
                    setPlaybackPitch(playbackPitch + 0.1f);
                }
                if (down & KEY_DDOWN) {
                    setPlaybackPitch(playbackPitch - 0.1f);
                }
                if (down & KEY_X) {
                    setPlaybackSpeed(1.0f);
                    setPlaybackPitch(1.0f);
                }
            } else if (settingsPage == 2) {
                if (down & KEY_A) disableLRSkipClosed = !disableLRSkipClosed;
            }

            if (down & KEY_START || down & KEY_B || down & KEY_SELECT) {
                saveTheme();
                currentScreen = SCREEN_BROWSER;
                settingsPage = 0;
            }
        } else {
            // Browser touch logic
            if (down & KEY_TOUCH) {
                touching = true;
                touchStartY = touch.py;
                lastTouchY = touch.py;
                scrollOffsetAtTouchStart = scrollOffset;

                // Immediate selection update on touch down
                int listY = 40, rowH = 16;
                if (touch.py >= listY && touch.py < listY + PAGE_SIZE * rowH) {
                    int idx = scrollOffset + (touch.py - listY) / rowH;
                    if (idx < entryCount) {
                        selected = idx;
                    }
                }
            } else if (touching && !(held & KEY_TOUCH)) {
                // Touch release - handle action if didn't scroll much
                touching = false;

                // If it was a tap (little movement), trigger action
                if (abs(lastTouchY - touchStartY) < 20) {
                    if (validateselection()) {
                        Entry *e = &entries[selected]; // if -1 selected, memory invalid - arcmage
                        if (e->type == ENTRY_DIR) {
                            safestrncpy(currentDir, e->fullPath, MAX_PATH); // no need for -1 because of safestrncpy - arcmage
                            scanDir(currentDir);
                            selected = 0; // Reset cursor for new directory
                        } else {
                            startPlayback(e->fullPath);
                        }
                    }
                }
            } else if (held & KEY_TOUCH) {
                lastTouchY = touch.py;
                int diffY = touchStartY - touch.py;
                if (abs(diffY) > 5) { // Threshold to distinguish scroll from static touch
                    int rowH = 15;
                    int scrollDiff = diffY / rowH;
                    int newOffset = scrollOffsetAtTouchStart + scrollDiff;
                    if (newOffset < 0) newOffset = 0;
                    int maxScroll = entryCount - PAGE_SIZE;
                    if (maxScroll < 0)
                        maxScroll = 0;
                    if (newOffset > maxScroll)
                        newOffset = maxScroll;
                    if (newOffset < 0)
                        newOffset = 0; // fix negative / false scroll positions and empty folder issues - arcmage
                    if (entryCount <= PAGE_SIZE) newOffset = 0;
                    scrollOffset = newOffset;
                }
            }

            if (down & KEY_SELECT) {
                currentScreen = SCREEN_SETTINGS;
                settingsPage = 0;
            }

            if (disableLRSkipClosed) {
                if (!lidClosed) {
                    if (down & KEY_L) playPrevious();
                    if (down & KEY_R) playNext();
                }
            } else {
                if (down & KEY_L) playPrevious();
                if (down & KEY_R) playNext();
            }

            if (down & KEY_DRIGHT) {
                playbackMode = (playbackMode + 1) % 4;
            }
            if (down & KEY_DLEFT) {
                playbackMode = (playbackMode + 3) % 4;
            }
            if (down & KEY_DOWN) {
                if (selected < entryCount - 1) {
                    selected++;
                    if (selected >= scrollOffset + PAGE_SIZE) scrollOffset++;
                }
            }
            if (down & KEY_UP) {
                if (selected > 0) {
                    selected--;
                    if (selected < scrollOffset) scrollOffset--;
                }
            }
            if (down & KEY_A && validateselection()) {
                Entry *e = &entries[selected];
                if (e->type == ENTRY_DIR) {
                    safestrncpy(currentDir, e->fullPath, MAX_PATH);
                    scanDir(currentDir);
                } else {
                    startPlayback(e->fullPath);
                }
            }
            if (down & KEY_B) {
                const char *slash = strrchr(currentDir, '/');
                char childName[256] = "";
                if (slash) safestrncpy(childName, slash + 1, sizeof(childName)); // no need for -1 because of safestrncpy - arcmage
                if (goUp()) {
                    scanDir(currentDir);
                    if (childName[0]) {
                        for (int i = 0; i < entryCount; i++) {
                            if (entries[i].type == ENTRY_DIR &&
                                !strcmp(entries[i].name, childName)) {
                                selected = i;
                                scrollOffset = selected - PAGE_SIZE / 2;
                                if (scrollOffset < 0) scrollOffset = 0;
                                if (scrollOffset + PAGE_SIZE > entryCount)
                                    scrollOffset = entryCount - PAGE_SIZE;
                                if (scrollOffset < 0) scrollOffset = 0;
                                break;
                            }
                        }
                    }
                }
            }
            if (down & KEY_X && playing) {
                paused = !paused;
                ndspChnSetPaused(0, paused); // Channel 0
            }
            if (down & KEY_Y) stopPlayback();

            // Always increment scrollTick to allow for smooth text scrolling
            scrollTick++; // in the future dont use this, as it is dependent on the frame rate. im too fucking lazy to fix this shit - arcmage
            if (selected != lastSelected) {
                scrollTick   = 0;
                lastSelected = selected;
            }
        }

        fillAudio();
        updateVisualizer();

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        if (currentScreen == SCREEN_SETTINGS) {
            drawSettingsScreen();
        } else {
            drawTopScreen();
            drawBotScreen();
        }
        C3D_FrameEnd(0);
    }

    stopPlayback();
    ptmuExit();
    if (hasArt) {
        if (artImage.subtex) free((void*)artImage.subtex);
        C3D_TexDelete(&artTex);
    }
    mpg123_exit();
    linearFree(audioBuf);
    ndspExit();
    C2D_TextBufDelete(dynBuf);
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}