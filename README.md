# Info

https://akouzoukos.com/apotris

Apotris is a block stacking game for the Gameboy Advance! It features satisfying graphics, responsive controls and a large amount of customization so that you can tailor the game to your preferences! 

Join the [discord server](https://discord.com/invite/jQnxmXS7tr) for updates, beta releases, or to give me feedback!

# Dependencies

The linux version requires SFML.

# Build

Run 

```
make <platform>
```

Available Platforms:

| Name            | Platform |
|-----------------|----------|
| Gameboy Advance | gba      |
| Linux           | linux    |
| Windows 32-bit  | w32      |

Omitting `<platform>` builds for all platforms

Builds are placed inside the `bin` folder.

# Meson build (experimental)

## Installation

### Base requirements for all systems

As all the libraries and tools are compiled from source, the only things you
need to have installed and in your PATH are:

- meson (at least version 0.60.0, ideally 1.0.0 or later)
- git
- A compiler for your system (`cl.exe`, `gcc`, etc.)
- A compiler for the GBA (`arm-none-eabi-gcc`)

### Ubuntu 22.04 and Windows (WSL) dependencies

[Install DevKitARM](https://devkitpro.org/wiki/devkitPro_pacman#Debian_and_derivatives) and the `sudo dkp-pacman -S gba-dev` target.

Install the following packages:
```bash
sudo apt install build-essential git meson cmake
```
also install `mingw-w64` if you plan to cross compile for Windows

## Compiling

Once you have everything, `git clone` or [download] this repository, then
navigate to the directory in a terminal (or "command-line") window, and run the
following command:

```sh
# Fetch remote dependencies
meson subprojects download

# Native build
meson setup build
ninja -C build

# GBA Build
source /etc/profile.d/devkit-env.sh # often automatic
sed "2cpath = '$DEVKITARM/bin'" meson/gba.ini > dkarm.ini # note you may need to modify this, see below
meson setup --cross-file=dkarm.ini build-gba
ninja -C build-gba
# MingW build (Windows using Linux, WSL, or macOS to build)
# Note: this next command is really bad hack that probably has no consequences but is required
sudo ln -sf $(which x86_64-w64-mingw32-g++-posix) $(which x86_64-w64-mingw32-g++)
meson setup --cross-file=meson/mingw.ini build-mingw
ninja -C build-mingw
```

The built Apotris ROMs (.elf, .gba) will be in your `build-gba` directory.

The build Apotris executable and its required `assets/` folder will be in your build directories for other builds.

Note the second and fourth commands will fail if you have not set up your path appropriately.

You must use devkitPro for GBA at this time. Set up path references with the following commands to generate a suitable cross file:

MSYS2:

```sh
sed "2cpath = '$(cygpath -m $DEVKITARM/bin)'" meson/gba.ini > dkarm.ini
```

WSL / Linux:

```sh
sed "2cpath = '$DEVKITARM/bin'" meson/gba.ini > dkarm.ini
```
