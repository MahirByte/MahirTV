![Logo]((https://files.catbox.moe/qeqg95.png))
# MahirTV

A modern Linux TV OS simulator written in C++ with SDL2 + OpenGL.

## Features

- Fullscreen modern TV OS interface (dark blue / blue theme)
- Setup wizard: language → account → 3D save data selection → loading → home
- 3D perspective grid with animated save data circles (draw-to-connect mechanic)
- Custom blue cursor
- App launcher: YouTube, Games, Settings, Browser, Music, Store
- Amaranth font (English) + Alkatra font (Bangla)
- Nintendo-style smooth animations and transitions
- Creates `mtv_root/` data directory on first launch

---

## Dependencies

Install on Ubuntu/Debian:
```bash
sudo apt update
sudo apt install -y \
    build-essential cmake \
    libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev \
    libglew-dev libgl1-mesa-dev \
    pkg-config
```

Install on Fedora/RHEL:
```bash
sudo dnf install -y \
    cmake gcc-c++ \
    SDL2-devel SDL2_ttf-devel SDL2_image-devel \
    glew-devel mesa-libGL-devel \
    pkgconfig
```

Install on Arch Linux:
```bash
sudo pacman -S --needed \
    cmake sdl2 sdl2_ttf sdl2_image glew
```

---

## Fonts (Required)

Download from Google Fonts and place in `assets/fonts/`:

1. **Amaranth** — https://fonts.google.com/specimen/Amaranth
   - `Amaranth-Regular.ttf`
   - `Amaranth-Bold.ttf`

2. **Alkatra** — https://fonts.google.com/specimen/Alkatra
   - `Alkatra-Regular.ttf`

Quick download script:
```bash
cd assets/fonts
# Install gfonts downloader or manually download the above files
# Alternatively the app will fall back to DejaVu if fonts are missing
```

If the font files are missing, MahirTV automatically falls back to **DejaVu Sans** (usually pre-installed on Linux).

---

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

Binary will be at `build/MahirTV`.

---

## Run

```bash
./build/MahirTV
```

- Press **F11** to toggle fullscreen
- Press **ESC** on the Home screen to quit
- Use **arrow keys** to navigate, **Enter** to select

---

## Build AppImage

```bash
chmod +x build-appimage.sh
./build-appimage.sh
```

This will produce `MahirTV-x86_64.AppImage` in the project root.

Requirements for AppImage build:
```bash
sudo apt install -y fuse libfuse2
wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage
```

---

## Project Structure

```
mahirtv-cpp/
├── CMakeLists.txt
├── README.md
├── build-appimage.sh
├── assets/
│   ├── cursor.png          ← Blue cursor image
│   └── fonts/
│       ├── Amaranth-Regular.ttf
│       ├── Amaranth-Bold.ttf
│       └── Alkatra-Regular.ttf
└── src/
    ├── main.cpp            ← Entry point + mtv_root creation
    ├── App.h / App.cpp     ← Main app + state machine
    ├── Shader.h / .cpp     ← OpenGL shader wrapper
    ├── TextRenderer.h/.cpp ← SDL_ttf → GL texture rendering
    ├── Math.h              ← Vec/Mat math utilities
    └── screens/
        ├── LanguageScreen  ← Step 1: Select English / Bangla
        ├── AccountScreen   ← Step 2: Sign in / Sign up / Guest
        ├── SaveDataScreen  ← Step 3: 3D ground + draw-to-connect
        ├── LoadingScreen   ← Step 4: White screen + blue spinner
        └── HomeScreen      ← Step 5: TV OS home + app grid
```

---

## mtv_root Directory

Created automatically on first launch:
```
mtv_root/
├── manifest.json
├── accdat/
│   └── account.json        ← Account data
├── applications/
│   ├── YouTube/
│   ├── Settings/
│   ├── Games/
│   ├── Browser/
│   ├── Music/
│   └── Store/
├── savedata/
│   ├── slot1/
│   ├── slot2/
│   └── slot3/
├── bin/
├── cache/
├── logs/
└── tmp/
```

---

## Controls

| Key | Action |
|-----|--------|
| Arrow Keys | Navigate apps |
| Enter | Select / Open app |
| ESC | Back / Quit |
| F11 | Toggle fullscreen |
| Mouse | Navigate all screens |
| Click + Drag | Draw to connect save data (SaveData screen) |
| Right Click | Close app overlay |
