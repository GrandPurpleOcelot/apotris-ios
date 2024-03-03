#!/usr/bin/env sh

# git submodule update --init --recursive --remote
# meson subprojects download

TYPE=--buildtype=release

#gba
meson setup --cross-file=meson/gba.ini $TYPE build-gba && meson compile -C build-gba
mkdir -p bin/gba/tools
mkdir -p bin/gba/audio

cp assets/* bin/gba/audio/.

cp tools/gbfs.exe bin/gba/tools/.
cp tools/buildSoundbank.exe bin/gba/tools/.
cp tools/mmutil.exe bin/gba/tools/.
cp tools/padbin.exe bin/gba/tools/.
cp include/audio_files.h bin/gba/tools/.
cp build-gba/Apotris-base.gba bin/gba/tools/.

cp tools/README.txt bin/gba/.

cp build-gba/Apotris.gba bin/gba/.
cp tools/build.bat bin/gba/.

#linux
meson setup $TYPE build-linux && meson compile -C build-linux
mkdir -p bin/linux/assets
mkdir -p bin/linux/subprojects

cp assets/* bin/linux/assets/.
cp build-linux/subprojects/SDL2-2.28.5/libsdl2.so bin/linux/subprojects/SDL2-2.28.5/libsdl2.so
cp build-linux/subprojects/SDL2-2.28.5/libsdl2.so bin/linux/subprojects/SDL2-2.28.5/SDL2.so
cp build-linux/Apotris bin/linux/.
cp tools/README_LINUX.txt bin/linux/README.txt

#windows
meson setup --cross-file=meson/mingw.ini $TYPE build-windows && meson compile -C build-windows
mkdir -p bin/windows/assets

cp assets/* bin/windows/assets/.
cp build-windows/Apotris.exe bin/windows/.

cp /usr/lib/gcc/x86_64-w64-mingw32/12-win32/libgcc_s_seh-1.dll bin/windows/.
cp /usr/x86_64-w64-mingw32/bin/libdl.dll bin/windows/.
cp /usr/lib/gcc/x86_64-w64-mingw32/12-win32/libstdc++-6.dll bin/windows/.
cp /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll bin/windows/.

cp build-windows/subprojects/SDL2-2.28.5/libsdl2.dll bin/windows/.
cp build-windows/subprojects/libmodplug/libmodplug.dll bin/windows/.
cp subprojects/SFML-2.6.1/extlibs/bin/x86/openal32.dll bin/windows/.
cp build-windows/subprojects/SDL2_mixer-2.6.2/libsdl2mixer.dll bin/windows/.
cp lib/* bin/windows/.
