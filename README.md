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

- meson (at least version 1.0.0 or later)
  - If your system has an out of date meson, install `pip` and run `pip install meson` to get the latest
- git
- A compiler for your system (`cl.exe`, `gcc`, etc.)
- A compiler for the GBA (`arm-none-eabi-gcc`)
  - On Ubuntu you really want to install what's 
    [here](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) and follow [this guide](https://askubuntu.com/questions/1243252/how-to-install-arm-none-eabi-gdb-on-ubuntu-20-04-lts-focal-fossa) to use it.
  - On macOS just `brew install --cask gcc-arm-embedded`

### Ubuntu 22.04 and Windows (WSL) dependencies

#### For all systems

Install the following packages:
```bash
sudo apt install build-essential git meson cmake 
```
also install `mingw-w64` if you plan to cross compile for Windows

#### For Linux native builds

Install the following dependencies of SDL2:
```bash
sudo apt-get install build-essential git make \
  pkg-config cmake ninja-build gnome-desktop-testing libasound2-dev libpulse-dev \
  libaudio-dev libjack-dev libsndio-dev libx11-dev libxext-dev \
  libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev \
  libxkbcommon-dev libdrm-dev libgbm-dev libgl1-mesa-dev libgles2-mesa-dev \
  libegl1-mesa-dev libdbus-1-dev libibus-1.0-dev libudev-dev fcitx-libs-dev
```

## Compiling

Once you have everything, `git clone` or [download] this repository, then
navigate to the directory in a terminal (or "command-line") window, and run the
following command:

```sh
# Fetch remote dependencies
git submodules update --init --recursive
meson subprojects download

# Native build
meson setup build
ninja -C build

# GBA Build
meson setup --cross-file=meson/gba.ini build-gba
ninja -C build-gba

# MingW build (Windows using Linux, WSL, or macOS to build)
# Note: the following command is a really bad hack that probably has no consequences but is required
sudo ln -sf $(which x86_64-w64-mingw32-g++-posix) $(which x86_64-w64-mingw32-g++)
meson setup --cross-file=meson/mingw.ini build-mingw
ninja -C build-mingw
```

The built Apotris ROMs (.elf, .gba) will be in your `build-gba` directory.

The built Apotris executable and its required `assets/` folder will be in your build directories for other builds.
