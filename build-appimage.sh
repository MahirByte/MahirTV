#!/usr/bin/env bash
set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
APPDIR="$PROJECT_DIR/MahirTV.AppDir"
VERSION="1.0.0"
APPIMAGE_TOOL="$PROJECT_DIR/appimagetool-x86_64.AppImage"

echo "=== MahirTV AppImage Builder ==="
echo ""

# Check for appimagetool
if [ ! -f "$APPIMAGE_TOOL" ]; then
    echo "[INFO] Downloading appimagetool..."
    wget -q "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage" \
         -O "$APPIMAGE_TOOL"
    chmod +x "$APPIMAGE_TOOL"
fi

# Build
echo "[1/4] Building MahirTV..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_INSTALL_PREFIX=/usr \
         -DASSETS_PATH="/usr/share/mahirtv/assets/"
make -j$(nproc)
echo "      Build OK"

# Prepare AppDir
echo "[2/4] Preparing AppDir..."
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/share/mahirtv"
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps"

cp "$BUILD_DIR/MahirTV" "$APPDIR/usr/bin/"
cp -r "$PROJECT_DIR/assets" "$APPDIR/usr/share/mahirtv/"

# Desktop file
cat > "$APPDIR/usr/share/applications/mahirtv.desktop" << EOF
[Desktop Entry]
Name=MahirTV
Comment=Modern Linux TV OS Simulator
Exec=MahirTV
Icon=mahirtv
Terminal=false
Type=Application
Categories=Entertainment;Video;
EOF
cp "$APPDIR/usr/share/applications/mahirtv.desktop" "$APPDIR/mahirtv.desktop"

# Icon (use cursor as placeholder icon, ideally replace with a proper 256x256 PNG)
if [ -f "$PROJECT_DIR/assets/cursor.png" ]; then
    cp "$PROJECT_DIR/assets/cursor.png" "$APPDIR/usr/share/icons/hicolor/256x256/apps/mahirtv.png"
    cp "$PROJECT_DIR/assets/cursor.png" "$APPDIR/mahirtv.png"
fi

# AppRun script
cat > "$APPDIR/AppRun" << 'APPRUN'
#!/usr/bin/env bash
SELF=$(readlink -f "$0")
HERE="${SELF%/*}"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"
exec "${HERE}/usr/bin/MahirTV" "$@"
APPRUN
chmod +x "$APPDIR/AppRun"

# Bundle SDL2 libraries
echo "[3/4] Bundling libraries..."
mkdir -p "$APPDIR/usr/lib"
for lib in libSDL2 libSDL2_ttf libSDL2_image libGLEW; do
    for path in /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib; do
        found=$(find "$path" -name "${lib}*.so*" -maxdepth 2 2>/dev/null | head -1)
        if [ -n "$found" ]; then
            cp -L "$found" "$APPDIR/usr/lib/" 2>/dev/null || true
            echo "      Bundled: $(basename $found)"
        fi
    done
done

# Build AppImage
echo "[4/4] Building AppImage..."
cd "$PROJECT_DIR"
ARCH=x86_64 "$APPIMAGE_TOOL" "$APPDIR" "MahirTV-${VERSION}-x86_64.AppImage" 2>/dev/null

echo ""
echo "=== Done! ==="
echo "AppImage: $PROJECT_DIR/MahirTV-${VERSION}-x86_64.AppImage"
echo ""
echo "Run with:"
echo "  chmod +x MahirTV-${VERSION}-x86_64.AppImage"
echo "  ./MahirTV-${VERSION}-x86_64.AppImage"
