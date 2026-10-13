#!/usr/bin/env bash
# Builds everything the Xcode project in this folder links against:
#   build/out/<platform>/libApotrisAll.a   game + all dependencies
#   build/out/<platform>/libSDL2main.a     UIKit entry point (provides main)
#   build/bundle/assets/                   sounds, music, shaders, file_list.txt
#
# Usage: ios/build.sh [iphoneos|iphonesimulator]
# Xcode runs it as a build phase and passes PLATFORM_NAME, ARCHS and
# CONFIGURATION, so normally you just press Run.
#
# Needs: Xcode command line tools, plus `brew install meson ninja cmake`.

set -euo pipefail

# Xcode exports hundreds of build settings into script phases, and clang
# honours some of them (IPHONEOS_DEPLOYMENT_TARGET, SDKROOT, ...) even for
# the Mac-native helper tools meson builds. Re-run with only what we need.
if [ -z "${APOTRIS_CLEAN_ENV:-}" ]; then
    exec env -i APOTRIS_CLEAN_ENV=1 HOME="$HOME" TERM="${TERM:-dumb}" \
        PATH="/opt/homebrew/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin" \
        DEVELOPER_DIR="${DEVELOPER_DIR:-$(xcode-select -p)}" \
        PLATFORM_NAME="${PLATFORM_NAME:-}" ARCHS="${ARCHS:-}" \
        CONFIGURATION="${CONFIGURATION:-}" \
        IOS_MIN="${IPHONEOS_DEPLOYMENT_TARGET:-}" \
        bash "$0" "$@"
fi

IOS_DIR="$(cd "$(dirname "$0")" && pwd)"
SRC_DIR="$(dirname "$IOS_DIR")"

PLATFORM="${1:-${PLATFORM_NAME:-iphoneos}}"
ARCH="${ARCHS:-arm64}"
ARCH="${ARCH%% *}" # Xcode may pass several; device and Apple Silicon simulators are arm64
MIN_IOS="${IOS_MIN:-15.0}"
BUILDTYPE=release
if [ "${CONFIGURATION:-Release}" = "Debug" ]; then
    BUILDTYPE=debugoptimized
fi

SDL2_VERSION=2.28.5        # same as subprojects/sdl2.wrap
SDL2_MIXER_VERSION=2.6.2   # same as subprojects/sdl2_mixer.wrap

for tool in meson ninja cmake pkg-config; do
    if ! command -v "$tool" > /dev/null; then
        echo "error: $tool not found. Install it with: brew install meson ninja cmake pkg-config" >&2
        exit 1
    fi
done

case "$PLATFORM" in
iphoneos) TRIPLE="$ARCH-apple-ios$MIN_IOS" ;;
iphonesimulator) TRIPLE="$ARCH-apple-ios$MIN_IOS-simulator" ;;
*)
    echo "error: unknown platform $PLATFORM" >&2
    exit 1
    ;;
esac

SDK="$(xcrun --sdk "$PLATFORM" --show-sdk-path)"
CC="$(xcrun --sdk "$PLATFORM" --find clang)"
CXX="$(xcrun --sdk "$PLATFORM" --find clang++)"

WORK="$IOS_DIR/build/$PLATFORM-$ARCH"
PREFIX="$WORK/prefix"
OUT="$IOS_DIR/build/out/$PLATFORM"
DOWNLOADS="$IOS_DIR/build/downloads"
mkdir -p "$WORK" "$PREFIX" "$OUT" "$DOWNLOADS"


fetch() { # url file
    if [ ! -f "$DOWNLOADS/$2" ]; then
        curl -fL --retry 3 -o "$DOWNLOADS/$2.part" "$1"
        mv "$DOWNLOADS/$2.part" "$DOWNLOADS/$2"
    fi
}

cmake_build() { # name source_dir [cmake args...]
    local name="$1" src="$2"
    shift 2
    if [ -f "$WORK/$name.done" ]; then
        return
    fi
    echo "== Building $name for $PLATFORM ($ARCH)"
    cmake -S "$src" -B "$WORK/$name-build" -G Ninja \
        -DCMAKE_SYSTEM_NAME=iOS \
        -DCMAKE_OSX_SYSROOT="$PLATFORM" \
        -DCMAKE_OSX_ARCHITECTURES="$ARCH" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET="$MIN_IOS" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DCMAKE_PREFIX_PATH="$PREFIX" \
        -DCMAKE_FIND_ROOT_PATH="$PREFIX" \
        -DBUILD_SHARED_LIBS=OFF \
        "$@"
    cmake --build "$WORK/$name-build"
    cmake --install "$WORK/$name-build"
    touch "$WORK/$name.done"
}

# 1. SDL2 and SDL2_mixer (their CMake builds support UIKit; the meson wraps don't)
fetch "https://github.com/libsdl-org/SDL/releases/download/release-$SDL2_VERSION/SDL2-$SDL2_VERSION.tar.gz" "SDL2-$SDL2_VERSION.tar.gz"
fetch "https://github.com/libsdl-org/SDL_mixer/releases/download/release-$SDL2_MIXER_VERSION/SDL2_mixer-$SDL2_MIXER_VERSION.tar.gz" "SDL2_mixer-$SDL2_MIXER_VERSION.tar.gz"
for archive in "SDL2-$SDL2_VERSION" "SDL2_mixer-$SDL2_MIXER_VERSION"; do
    [ -d "$WORK/$archive" ] || tar -xzf "$DOWNLOADS/$archive.tar.gz" -C "$WORK"
done

cmake_build SDL2 "$WORK/SDL2-$SDL2_VERSION" \
    -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TEST=OFF

# Apotris plays audio through SoLoud and only needs SDL_mixer's headers, so
# skip every codec that would pull in more libraries.
cmake_build SDL2_mixer "$WORK/SDL2_mixer-$SDL2_MIXER_VERSION" \
    -DSDL2MIXER_VENDORED=OFF -DSDL2MIXER_SAMPLES=OFF \
    -DSDL2MIXER_CMD=OFF -DSDL2MIXER_FLAC=OFF -DSDL2MIXER_MOD=OFF \
    -DSDL2MIXER_MIDI=OFF -DSDL2MIXER_MP3=OFF -DSDL2MIXER_OPUS=OFF \
    -DSDL2MIXER_VORBIS=OFF -DSDL2MIXER_WAVPACK=OFF -DSDL2MIXER_WAVE=ON

# SDL2_mixer's CMake installs no pkg-config file on Apple platforms, and
# that is how meson looks for it.
cat > "$PREFIX/lib/pkgconfig/SDL2_mixer.pc" <<EOF
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: SDL2_mixer
Description: mixer library for Simple DirectMedia Layer
Version: $SDL2_MIXER_VERSION
Requires: sdl2
Libs: -L\${libdir} -lSDL2_mixer
Cflags: -I\${includedir}/SDL2
EOF

# 2. Apotris itself, through meson
CROSS="$WORK/ios-cross.ini"
cat > "$CROSS" <<EOF
[binaries]
c = '$CC'
cpp = '$CXX'
objc = '$CC'
objcpp = '$CXX'
ar = '$(xcrun --sdk "$PLATFORM" --find ar)'
strip = '$(xcrun --sdk "$PLATFORM" --find strip)'
pkg-config = '$(command -v pkg-config)'

[host_machine]
system = 'darwin'
subsystem = '$( [ "$PLATFORM" = iphonesimulator ] && echo ios-simulator || echo ios )'
kernel = 'xnu'
cpu_family = '$( [ "$ARCH" = arm64 ] && echo aarch64 || echo x86_64 )'
cpu = '$ARCH'
endian = 'little'

[properties]
needs_exe_wrapper = true
pkg_config_libdir = '$PREFIX/lib/pkgconfig'

[built-in options]
c_args = ['-target', '$TRIPLE', '-isysroot', '$SDK']
cpp_args = ['-target', '$TRIPLE', '-isysroot', '$SDK']
objc_args = ['-target', '$TRIPLE', '-isysroot', '$SDK']
objcpp_args = ['-target', '$TRIPLE', '-isysroot', '$SDK', '-fobjc-arc']
c_link_args = ['-target', '$TRIPLE', '-isysroot', '$SDK']
cpp_link_args = ['-target', '$TRIPLE', '-isysroot', '$SDK']
objcpp_link_args = ['-target', '$TRIPLE', '-isysroot', '$SDK']
default_library = 'static'
EOF

MESON_BUILD="$WORK/meson"
if [ ! -f "$MESON_BUILD/build.ninja" ]; then
    echo "== Configuring Apotris for $PLATFORM ($ARCH)"
    rm -rf "$MESON_BUILD" # a failed configure leaves a half-made directory
    meson setup "$MESON_BUILD" "$SRC_DIR" \
        --cross-file="$CROSS" \
        --buildtype="$BUILDTYPE" \
        -Db_lto=false
fi
echo "== Building Apotris for $PLATFORM ($ARCH)"
meson compile -C "$MESON_BUILD"

# 3. One archive with the game and everything it links, so the Xcode project
# only has to name two libraries. SDL2main stays separate because it defines
# main() and must not be dropped by the linker.
ARCHIVES=()
while IFS= read -r archive; do
    # Skip host tools that subprojects build for the Mac itself.
    if otool -l "$archive" 2> /dev/null | grep -A3 LC_BUILD_VERSION | grep -qE '^ *platform (1|MACOS|macos)$'; then
        continue
    fi
    ARCHIVES+=("$archive")
done < <(find "$MESON_BUILD" -name '*.a' -not -path '*/meson-private/*' | sort)
ARCHIVES+=("$PREFIX/lib/libSDL2.a" "$PREFIX/lib/libSDL2_mixer.a")
xcrun libtool -static -no_warning_for_no_symbols -o "$OUT/libApotrisAll.a" "${ARCHIVES[@]}"
cp "$PREFIX/lib/libSDL2main.a" "$OUT/libSDL2main.a"

# 4. Bundle assets the way the Android port does (assets/ + file_list.txt);
# the game copies them into Documents on first launch.
BUNDLE_ASSETS="$IOS_DIR/build/bundle/assets"
rm -rf "$BUNDLE_ASSETS"
mkdir -p "$BUNDLE_ASSETS"
rsync -a --exclude 'meson.build' "$SRC_DIR/assets/" "$BUNDLE_ASSETS/"
(cd "$BUNDLE_ASSETS" && find . -type f ! -name file_list.txt | sed 's|^\./||' | sort > file_list.txt)
cp -R "$SRC_DIR/license" "$IOS_DIR/build/bundle/license"

echo "== Done: $OUT/libApotrisAll.a"
