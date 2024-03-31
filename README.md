# Apotris

<https://akouzoukos.com/apotris>

Apotris is a block stacking game for the Gameboy Advance! It features satisfying graphics, responsive controls and a large amount of customization so that you can tailor the game to your preferences!

Join the [discord server](https://discord.com/invite/jQnxmXS7tr) for updates, beta releases, or to give me feedback!

## Docker build

Want to bypass all the work and use a system shipped by the developers intended to build Apotris? Try the docker image:

First, [install Docker](https://docs.docker.com/get-docker/) for your system. Then get it running and open up a terminal:

```bash
# Download and log in to the Apotris builder docker image
docker run -it <template-tag-goes-here> bash
# OR Optional: build the image yourself
docker build -t apotris-builder . && docker run -it apotris-builder bash
# You are now in the docker image environment, use Ctrl+D or "exit" command to return to main OS

# Clone the latest sources
git clone --branch ports https://gitea.com/akouzoukos/apotris.git
# Enter source dir
cd apotris
# From here you can build one of the available targets

# GBA
meson setup --cross-file=meson/gba.ini build-gba && meson compile -C build-gba
# Windows
meson setup --cross-file=meson/mingw.ini build-windows && meson compile -C build-windows
# Linux
meson setup build-linux && meson compile -C build-linux

# Built executables are now in the Docker environment's apotris/build-*/ folders
```

## Meson build

### Overview of requirements for most systems

As all the libraries and tools are compiled from source, the only things you
need to have installed and in your PATH are:

- meson (at least version 1.1.0 or later)
  - If your system has an out of date meson, install `pip` and run `pip install meson` to get the latest
- git
- A compiler for your system (`cl.exe`, `gcc`, etc.)
- A compiler for the GBA (`arm-none-eabi-gcc`)
  - On Windows you can download the [.exe installer](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) for `mingw-w64-i686-arm-none-eabi-gcc`
  - On Ubuntu you really want to install what's
    [here](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) and follow [this guide](https://askubuntu.com/questions/1243252/how-to-install-arm-none-eabi-gdb-on-ubuntu-20-04-lts-focal-fossa) to use it. Make sure to symlink everything `sudo ln -sf /usr/share/arm-gnu-toolchain-YOUR_VERSION/bin/* /usr/bin/`
  - On macOS just `brew install --cask gcc-arm-embedded`

### Windows dependencies

You will need to install:

- [Python 3](https://www.python.org/downloads/windows/)
  - get the latest stable Windows installer for your platform
  - ensure you "add to PATH" when you run the installer (unless you know what you're doing otherwise)
- Meson
  - Run `pip install meson` after installing Python to get meson
- A Windows-native compiler and build tools
  - [Strawberry Perl](https://strawberryperl.com) works out of the box, get the system installer to make it easiest

#### Windows specific instructions

For the GBA target, you will need to update the `meson/gba.ini` file to have the path to your ARM compiler:

```ini
[constants]
path = 'C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\13.2 Rel1\bin'
# ... rest of original file
```

You can avoid editing a file by setting this with a powershell command per-session

```powershell
$env:PATH="$env:PATH;C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\13.2 Rel1\bin"
```

Ensure the version string and directory paths are correct for your installation. They may change with future updates.

### Ubuntu 22.04+ and Windows (WSL) dependencies

#### For all Linux systems

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
  libegl1-mesa-dev libdbus-1-dev libibus-1.0-dev libudev-dev fcitx-libs-dev libglew-dev
```

## Compiling

Once you have everything, `git clone` or [download] this repository, then
navigate to the directory in a terminal (or "command-line") window, and run the
following command:

```sh
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
