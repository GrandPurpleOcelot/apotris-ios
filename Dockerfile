FROM ubuntu:23.10

# Install apt-available deps
RUN DEBIAN_FRONTEND=noninteractive \
  apt-get update \
  && apt-get install -y python3 ash zsh vim build-essential git make curl meson zstd\
    pkg-config cmake ninja-build gnome-desktop-testing libasound2-dev libpulse-dev \
    libaudio-dev libjack-dev libsndio-dev libx11-dev libxext-dev \
    libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev \
    libxkbcommon-dev libdrm-dev libgbm-dev libgl1-mesa-dev libgles2-mesa-dev \
    libegl1-mesa-dev libdbus-1-dev libibus-1.0-dev libudev-dev fcitx-libs-dev mingw-w64 directx-headers-dev

# Download and extract toolchain based on arch
RUN /bin/bash -c 'set -ex && \
    ARCH=`uname -m` && \
    if [ "$ARCH" == "x86_64" ]; then \
       echo "x86_64" && \
       curl -L "https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz?rev=e434b9ea4afc4ed7998329566b764309&hash=CA590209F5774EE1C96E6450E14A3E26" | tar xJ -C /usr/share; \
    else \
       echo "ARM64 arch?" && \
       curl -L "https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-aarch64-arm-none-eabi.tar.xz?rev=17baf091942042768d55c9a304610954&hash=06E4C2BB7EBE7C70EA4EA51ABF9AAE2D" | tar xJ -C /usr/share; \
    fi'

# Link toolchain into PATH
RUN ln -sf /usr/share/arm-gnu-toolchain-*/bin/* /usr/bin/

# Get missing windows deps and hack them in
RUN curl -L "https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-dlfcn-1.4.1-1-any.pkg.tar.zst" | tar --strip-components=1 -x --zstd -C /usr/x86_64-w64-mingw32/
