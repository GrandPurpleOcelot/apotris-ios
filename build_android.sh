#!/bin/bash
set -e

if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "Error: ANDROID_NDK_HOME is not set."
    exit 1
fi

# Determine host system for NDK toolchain path
HOST_OS=$(uname -s | tr '[:upper:]' '[:lower:]')
# Normalize linux-x86_64
if [ "$HOST_OS" == "linux" ]; then
    HOST_TAG="linux-x86_64"
elif [ "$HOST_OS" == "darwin" ]; then
    HOST_TAG="darwin-x86_64"
else
    HOST_TAG="windows-x86_64" # Guess
fi

TOOLCHAIN="$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/$HOST_TAG"
TARGET="aarch64-none-linux-android21"

# Generate cross file
mkdir -p meson/cross
cat > meson/cross/android-aarch64.ini <<EOF
[binaries]
c = '$TOOLCHAIN/bin/aarch64-linux-android21-clang'
cpp = '$TOOLCHAIN/bin/aarch64-linux-android21-clang++'
ar = '$TOOLCHAIN/bin/llvm-ar'
strip = '$TOOLCHAIN/bin/llvm-strip'

[host_machine]
system = 'android'
cpu_family = 'aarch64'
cpu = 'aarch64'
endian = 'little'

[built-in options]
c_args = ['-target', '$TARGET', '-fPIC']
cpp_args = ['-target', '$TARGET', '-fPIC']
c_link_args = ['-target', '$TARGET', '-lOpenSLES', '-llog', '-landroid', '-lGLESv2']
cpp_link_args = ['-target', '$TARGET', '-lOpenSLES', '-llog', '-landroid', '-lGLESv2']
default_library = 'shared'
b_staticpic = true

[properties]
glesv2_dir = '$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/$HOST_TAG/sysroot/usr'
glesv2_has_headers = true
glesv2_include_dirs = ['$glesv2_dir/include']
glesv2_libs = ['GLESv2', 'OpenSLES', 'log', 'android']

[sdl2:project options]
default_library = 'shared'
use_hidapi = 'disabled'

[sdl2_mixer:project options]
default_library = 'shared'
EOF

echo "Generated meson/cross/android-aarch64.ini"

# Setup Meson
echo "Running Meson Setup..."
if [ -d "build-android" ]; then
    meson setup --reconfigure build-android --cross-file meson/cross/android-aarch64.ini -Dbuildtype=debugoptimized
else
    meson setup build-android --cross-file meson/cross/android-aarch64.ini -Dbuildtype=debugoptimized
fi

SDL_JAVA_SRC="$(find subprojects -type d -path "*/SDL2-*/android-project/app/src/main/java/org/libsdl/app" -print -quit)"

# Prepare Java files (Copy from SDL2 subproject)
echo "Copying SDL2 Java files..."
mkdir -p android/app/src/main/java/org/libsdl/app # Ensure destination exists
if [ -n "$SDL_JAVA_SRC" ]; then
    cp "$SDL_JAVA_SRC"/*.java android/app/src/main/java/org/libsdl/app/
    
    # PATCH: Disable HIDDeviceManager to prevent UnsatisfiedLinkError when HIDAPI is disabled in C build
    echo "Patching SDLActivity.java to disable HIDAPI..."
    sed -i 's/mHIDDeviceManager = HIDDeviceManager.acquire(this);/\/\/ mHIDDeviceManager = HIDDeviceManager.acquire(this);/g' android/app/src/main/java/org/libsdl/app/SDLActivity.java
    
    echo "Java files copied and patched."
else
    echo "Warning: Could not find SDL2 Java sources. You might need to copy them manually."
fi

# Compile
echo "Compiling..."
meson compile -C build-android

# Copy Libs
echo "Copying libraries..."
mkdir -p android/app/src/main/jniLibs/arm64-v8a
# Copy the game lib
cp build-android/libmain.so android/app/src/main/jniLibs/arm64-v8a/

# Find and copy SDL2 libs
# Use find to locate them in the build directory
echo "Locating SDL2 libraries..."
# Match lowercase names found in build
SDL2_LIB=$(find build-android -name "libsdl2.so" -o -name "libSDL2.so" | head -n 1)
SDL2_MIXER_LIB=$(find build-android -name "libsdl2mixer.so" -o -name "libSDL2_mixer.so" | head -n 1)
SOLOUD_LIB=$(find build-android -name "lib*oLoud.so" | head -n 1)

if [ -f "$SDL2_LIB" ]; then
    # Rename to libSDL2.so as expected by standard SDLActivity
    cp "$SDL2_LIB" android/app/src/main/jniLibs/arm64-v8a/libSDL2.so
    echo "Copied libSDL2.so"
else
    echo "Error: Could not find libsdl2.so in build-android"
    exit 1
fi

if [ -f "$SDL2_MIXER_LIB" ]; then
    # Rename to libSDL2_mixer.so
    cp "$SDL2_MIXER_LIB" android/app/src/main/jniLibs/arm64-v8a/libSDL2_mixer.so
    echo "Copied libSDL2_mixer.so"
else
    echo "Error: Could not find libsdl2mixer.so in build-android"
    exit 1
fi

if [ -f "$SOLOUD_LIB" ]; then
    cp "$SOLOUD_LIB" android/app/src/main/jniLibs/arm64-v8a/libSoLoud.so
    echo "Copied libSoLoud.so"
else
    echo "Error: Could not find libSoLoud shared library in build-android"
    # exit 1 # Optional
fi

# Copy Vorbis dependencies if they exist (required by SDL2_mixer)
find build-android -name "libvorbis.so" -exec cp {} android/app/src/main/jniLibs/arm64-v8a/ \;
find build-android -name "libvorbisfile.so" -exec cp {} android/app/src/main/jniLibs/arm64-v8a/ \;
find build-android -name "libvorbisenc.so" -exec cp {} android/app/src/main/jniLibs/arm64-v8a/ \;
find build-android -name "libogg.so" -exec cp {} android/app/src/main/jniLibs/arm64-v8a/ \;

# Copy libc++_shared.so
LIBCPP=$(find "$TOOLCHAIN" -name "libc++_shared.so" | grep "aarch64" | head -n 1)
if [ -n "$LIBCPP" ]; then
    cp "$LIBCPP" android/app/src/main/jniLibs/arm64-v8a/
else
    echo "Warning: libc++_shared.so not found."
fi

# Copy Assets
echo "Copying assets..."
mkdir -p android/app/src/main/assets/assets
mkdir -p android/app/src/main/assets/data
cp -r assets/* android/app/src/main/assets/assets/ 2>/dev/null || true
cp -r data/* android/app/src/main/assets/data/ 2>/dev/null || true

# Generate file list for dynamic loading
echo "Generating assets file list..."
(cd assets && find . -type f) | sed 's|^\./||' > android/app/src/main/assets/assets/file_list.txt

# Gradle Setup & Build
echo "Configuring Gradle..."
cd android

# 1. Ensure gradle.properties has AndroidX enabled
if ! grep -q "android.useAndroidX=true" gradle.properties 2>/dev/null; then
    echo "android.useAndroidX=true" >> gradle.properties
    echo "android.enableJetifier=true" >> gradle.properties
fi

# 2. Generate Wrapper if missing or broken (using the robust workaround)
if [ ! -f "./gradlew" ] || [ ! -f "gradle/wrapper/gradle-wrapper.jar" ]; then
    echo "Gradle wrapper missing or broken. Generating..."
    if command -v gradle &> /dev/null; then
        # Create temp build file to bypass version incompatibilities
        cat > wrapper.gradle <<EOF
task wrapper(type: Wrapper) {
    gradleVersion = '8.0'
}
EOF
        gradle -b wrapper.gradle wrapper
        rm wrapper.gradle
    else
        echo "Error: 'gradlew' is missing and 'gradle' is not installed."
        echo "Please install Gradle or commit the 'gradlew' script to the repository."
        exit 1
    fi
fi

# 3. Build the APK
echo "Building APK..."
chmod +x gradlew
./gradlew assembleDebug

echo "Build Complete!"
echo "APK location: android/app/build/outputs/apk/debug/app-debug.apk"
