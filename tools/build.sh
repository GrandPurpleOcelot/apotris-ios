#!/usr/bin/env sh

meson subprojects download
git submodule update --init --recursive --remote

# #gba
# meson setup --cross-file=meson/gba.ini build-gba && meson compile -C build-gba
# mkdir -p bin/gba/tools
# mkdir -p bin/gba/audio

# cp assets/* bin/gba/audio/.

# cp tools/gbfs.exe bin/gba/tools/.
# cp tools/buildSoundbank.exe bin/gba/tools/.
# cp tools/mmutil.exe bin/gba/tools/.
# cp tools/padbin.exe bin/gba/tools/.
# cp include/audio_files.h bin/gba/tools/.
# cp build-gba/Apotris-base.gba bin/gba/tools/.

# cp tools/README.txt bin/gba/.

# cp build-gba/Apotris.gba bin/gba/.
# cp tools/build.bat bin/gba/.

# #linux
# meson setup build-linux && meson compile -C build-linux
# mkdir -p bin/linux/assets

# cp assets/* bin/linux/assets/.
# cp -r build-linux/subprojects bin/linux/subprojects
# cp build-linux/Apotris bin/linux/.

#window
meson setup --cross-file=meson/mingw.ini build-windows && meson compile -C build-windows
mkdir -p bin/windows/assets

cp assets/* bin/windows/assets/.
cp build-windows/Apotris.exe bin/windows/.
cp lib/*.dll bin/windows/.
cp lib/win-x64/*.dll bin/windows/.
