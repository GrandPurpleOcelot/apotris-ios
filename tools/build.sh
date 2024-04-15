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
mkdir -p bin/linux-x64/assets
mkdir -p bin/linux-x64/subprojects/SDL2-2.28.5

cp assets/* bin/linux-x64/assets/.
cp build-linux/Apotris bin/linux-x64/.
cp tools/README_LINUX.txt bin/linux-x64/README.txt

#windows
meson setup --cross-file=meson/mingw.ini $TYPE build-windows && meson compile -C build-windows
mkdir -p bin/windows-x64/assets

cp assets/* bin/windows-x64/assets/.
cp build-windows/Apotris.exe bin/windows-x64/.
