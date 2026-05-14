#!/usr/bin/env bash
# Install all MahirTV runtime dependencies
set -e
echo "Installing MahirTV dependencies..."

sudo apt-get update -qq

# Core SDL2 build deps
sudo apt-get install -y \
    libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev \
    libglew-dev libgl1-mesa-dev \
    g++ cmake make pkg-config \
    unzip zenity

# Bengali / Indic fonts
sudo apt-get install -y fonts-noto-core fonts-noto-extra || true

# WebKit2GTK for embedded browser (MahirTV Browser/YouTube/Music)
sudo apt-get install -y \
    python3-gi python3-gi-cairo gir1.2-gtk-3.0 \
    gir1.2-webkit2-4.1 || \
  sudo apt-get install -y \
    python3-gi python3-gi-cairo gir1.2-gtk-3.0 \
    gir1.2-webkit2-4.0 || true

echo ""
echo "Done. Build MahirTV with: make"
