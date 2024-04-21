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
cp tools/buildSoundbank.py bin/gba/tools/.
cp tools/mmutil.exe bin/gba/tools/.
cp tools/padbin.exe bin/gba/tools/.
cp tools/mmutil bin/gba/tools/.
cp tools/padbin bin/gba/tools/.
cp tools/gbfs bin/gba/tools/.
cp tools/sound_effects.txt bin/gba/tools/.
cp build-gba/Apotris-base.gba bin/gba/tools/.

cp tools/README.txt bin/gba/.

cp build-gba/Apotris.gba bin/gba/.
cp tools/buildSongs.bat bin/gba/WINDOWS_build.bat
cp tools/buildSongs.sh bin/gba/LINUX_build.sh

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

#switch
meson setup --cross-file=meson/switch.ini $TYPE build-switch && meson compile -C build-switch
mkdir -p bin/switch/

cp build-switch/Apotris.nro bin/switch/.
cp tools/README_SWITCH.txt bin/switch/README.txt
