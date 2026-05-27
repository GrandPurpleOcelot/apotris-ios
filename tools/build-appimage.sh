#!/bin/bash
#
# build-appimage.sh - Package Apotris Linux build as an AppImage
#
# Prerequisites:
#   1. Build Apotris for Linux first:
#      meson setup -Db_lto=true -Db_lto_mode=thin --buildtype=release build-linux
#      meson compile -C build-linux
#      strip build-linux/Apotris
#
# Usage:
#   ./tools/build-appimage.sh          # use build-linux/ directory
#   ./tools/build-appimage.sh build-foo  # use a different build directory
#

set -euo pipefail

APP_NAME="Apotris"
BUILD_DIR="${1:-build-linux}"
BINARY="$BUILD_DIR/$APP_NAME"

if [ ! -f "$BINARY" ]; then
	echo "Error: $BINARY not found. Build Apotris for Linux first."
	exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
APPIMG_ARCH="$(uname -m)"
APPIMAGETOOL="$PROJECT_DIR/$BUILD_DIR/appimagetool-$APPIMG_ARCH.AppImage"

# ------------------------------------------------------------------
# 1. Download appimagetool if missing
# ------------------------------------------------------------------
if [ ! -f "$APPIMAGETOOL" ]; then
	echo "==> Downloading appimagetool for $APPIMG_ARCH..."
	wget -q -O "$APPIMAGETOOL" \
		"https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-$APPIMG_ARCH.AppImage"
	chmod +x "$APPIMAGETOOL"
fi

# ------------------------------------------------------------------
# 2. Build the AppDir
# ------------------------------------------------------------------
APPDIR="$PROJECT_DIR/AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps"
mkdir -p "$APPDIR/usr/share/apotris"

echo "==> Creating AppDir at $APPDIR"

cp "$BINARY" "$APPDIR/usr/bin/$APP_NAME"
cp -r "$PROJECT_DIR/assets" "$APPDIR/usr/share/apotris/assets"
cp -r "$PROJECT_DIR/license" "$APPDIR/usr/share/apotris/license"
find "$APPDIR/usr/share/apotris" -name meson.build -delete

echo "==> Copying icon..."
if command -v convert &>/dev/null; then
	convert "$PROJECT_DIR/sprites/logo512.png" -resize 256x256 \
		"$APPDIR/usr/share/icons/hicolor/256x256/apps/$APP_NAME.png"
else
	cp "$PROJECT_DIR/sprites/logo512.png" \
		"$APPDIR/usr/share/icons/hicolor/256x256/apps/$APP_NAME.png"
fi

echo "==> Installing .desktop file..."
cp "$PROJECT_DIR/dist/Apotris.desktop" "$APPDIR/usr/share/applications/$APP_NAME.desktop"

# Root symlinks (AppImage spec)
ln -s usr/bin/"$APP_NAME" "$APPDIR/AppRun"
ln -s usr/share/applications/"$APP_NAME.desktop" "$APPDIR/$APP_NAME.desktop"
ln -s usr/share/icons/hicolor/256x256/apps/"$APP_NAME.png" "$APPDIR/$APP_NAME.png"

# ------------------------------------------------------------------
# 3. Run appimagetool to produce the AppImage
# ------------------------------------------------------------------
echo "==> Creating AppImage..."
cd "$PROJECT_DIR"
APPIMAGE_EXTRACT_AND_RUN=1 "$APPIMAGETOOL" "$APPDIR" "$APP_NAME-$APPIMG_ARCH.AppImage"

OUTFILE="$PROJECT_DIR/$APP_NAME-$APPIMG_ARCH.AppImage"
if [ -f "$OUTFILE" ]; then
	chmod +x "$OUTFILE"
	echo ""
	echo "==> AppImage created: $OUTFILE"
fi

echo "==> Cleaning up AppDir..."
rm -rf "$APPDIR"
echo "==> Done!"
