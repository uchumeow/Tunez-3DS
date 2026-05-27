# 🎵 Tunez3DS

**Tunez3DS** is a feature-rich, high-performance MP3 player for the Nintendo 3DS, designed with a focus on aesthetics and smooth user experience. It utilizes a modern "Glass & Geometry" UI to bring a contemporary feel to the classic handheld.

## ✨ Features

- **🎨 Modern UI**: Beautiful card-based layouts with depth effects and smooth animations powered by `citro2d`.
- **🌈 Customizable Themes**: Includes 7 built-in themes like *Everforest*, *Midnight Purple*, and *Classic Light*.
- **📊 Audio Engine**: High-quality playback using `ndsp` and `mpg123`.
- **⚡ Advanced Playback**:
  - Adjust playback speed (0.25x - 4x) and pitch.
  - Multiple modes: Normal, Shuffle, Repeat One, and Repeat All.
- **📁 Smart Browser**: Responsive file navigation with support for folders and MP3 metadata (ID3 tags).
- **🖼️ Cover Art**: Automatic loading of embedded or local cover art.
- **🔒 Pocket Mode**: "Lid Protection" settings to prevent accidental track skips while on the move.
- **🔄 Auto-Updates**: Check for and install the latest versions directly from the app.

## 🎮 Controls

| Input | Action |
| :--- | :--- |
| **Touch Screen** | Tap to select, double-tap to play/open |
| **D-Pad Up/Down** | Navigate through lists |
| **D-Pad Left/Right** | Change playback mode / Adjust settings |
| **L / R Buttons** | Skip to Previous / Next track |
| **A / B** | Confirm / Back |
| **X / Y** | Play / Pause / Stop |
| **SELECT** | Open Settings Menu |
| **START** | Exit Application |

## 🚀 Installation

1. Download the latest `.cia` or `.3dsx` from the [Releases](https://github.com/uchumeow/Tunez-3DS/releases/latest) page.
2. **CIA Installation**: Transfer to your SD card and install via **FBI**, or scan the QR code below:
   <br>
   <img src="Tunez3DS_qr.png" alt="Scan me with FBI!" width="200"/>
3. **3DSX Usage**: Place the file in the `/3ds/` folder on your SD card and launch via the Homebrew Launcher.

## 📂 Setup

Place your music files in the `sdmc:/Music` folder on your SD card. Tunez3DS will automatically scan this directory on startup.

## 🛠️ Credits & Technical Details

Built using the [devkitPro](https://devkitpro.org/) toolchain and the following libraries:
- [libctru](https://github.com/smealum/ctru)
- [citro2d](https://github.com/fincs/citro2d) & [citro3d](https://github.com/fincs/citro3d)
- [mpg123](https://www.mpg123.de/)

*This project is a labor of love for the 3DS homebrew community. If you encounter any issues, please feel free to open a [GitHub Issue](https://github.com/uchumeow/Tunez-3DS/issues).*
