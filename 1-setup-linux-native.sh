#!/bin/bash

# There are 3 types of dependencies:
# 1. Build tools:
# a) autoconf, automake, make, libtool (if you use autotools build system)
# b) cmake, ninja/make (if you use CMake build system)
# gcc/clang as primary compiler
# intltool, gettext (for internationalization)
# pkg-config (used for library search)
# shared-mime-info (used to register .sif file extension in system)
#
# 2. Libraries:
# glibmm (platform depended functions)
# mlt++ imagemagick openexr libmng libpng libjpeg (various formats reading functions)
# libxml++ libxml2 libxslt (.sif XML reading)
# cairo fftw pango (image transformation and rendering functions)
# gtkmm3 (Synfig Studio GUI)
# libsig++ (GUI signals and events)
# sdl2 sdl2_mixer jack (audio output and synchronization)
#
# 3. Runtime dependencies:
# adwaita-icon-theme (used to render GUI icons and styles)
# python3-lxml (lottie exporter plugin)

set -e

# 1 - Detect which Linux OS is in use
echo "Detecting Linux OS..."

# Check if /etc/os-release file is available
if [ -f /etc/os-release ]; then
    source /etc/os-release
    if [ -z "$ID_LIKE" ] && [ ! -z $ID ]; then
        ID_LIKE=$ID
    fi
    echo "ID_LIKE=$ID_LIKE"
    echo "VERSION_ID=$VERSION_ID"
fi

# Fallback whether /etc/os-release is not available, or if ID_LIKE is not set
if [ -z "$ID_LIKE" ]; then
    if command -v dnf >/dev/null; then
        # Fedora DNF package manager
        export ID_LIKE="fedora"
        export VERSION_ID=23
    elif command -v yum >/dev/null; then
        # Fedora YUM package manager
        export ID_LIKE="fedora"
        export VERSION_ID=22
    elif which zypper >/dev/null; then
        # openSUSE package manager
        export ID_LIKE="suse opensuse"
    elif command -v pacman >/dev/null; then
        # Arch Linux package manager
        export ID_LIKE="arch"
    elif command -v apt-get >/dev/null; then
        # Debian / Ubuntu package manager
        export ID_LIKE="debian"
    fi
fi

# 2 - Install the required packages
echo "Checking dependencies..."

if ([ "$ID_LIKE" == "fedora" ] && [ VERSION_ID > 22 ]); then
    PKG_LIST="git \
            intltool \
            libpng-devel \
            libjpeg-devel \
            fftw-devel \
            freetype-devel \
            fribidi-devel \
            fontconfig-devel \
            atk-devel \
            pango-devel \
            cairo-devel \
            gtk3-devel \
            gettext-devel \
            libxml2-devel \
            libxml++-devel \
            gcc-c++ \
            autoconf \
            automake \
            libtool \
            libtool-ltdl-devel \
            shared-mime-info \
            OpenEXR-devel \
            libmng-devel \
            ImageMagick-c++-devel \
            mlt-devel \
            ocl-icd-devel \
            opencl-headers \
            gtkmm30-devel \
            glibmm24-devel \
            SDL2-devel \
            SDL2_mixer-devel \
            libxslt-devel python-devel python3-lxml"
    # Fedora 34 and onward uses pipewire
    if dnf -C list installed pipewire-jack-audio-connection-kit &> /dev/null; then
        PKG_LIST="$PKG_LIST pipewire-jack-audio-connection-kit-devel"
    else
        PKG_LIST="$PKG_LIST jack-audio-connection-kit-devel"
    fi

    if ! ( rpm -qv $PKG_LIST ); then
        echo "Running dnf (root privileges are needed)..."
        sudo dnf install $PKG_LIST || true
    fi

elif ([ "$ID_LIKE" == "fedora" ] && [ VERSION_ID <= 22]); then
    PKG_LIST="git \
            intltool \
            libpng-devel \
            libjpeg-devel \
            fftw-devel \
            freetype-devel \
            fribidi-devel \
            fontconfig-devel \
            atk-devel \
            pango-devel \
            cairo-devel \
            gtk3-devel \
            gettext-devel \
            libxml2-devel \
            libxml++-devel \
            gcc-c++ \
            autoconf \
            automake \
            libtool \
            libtool-ltdl-devel \
            shared-mime-info \
            OpenEXR-devel \
            libmng-devel \
            ImageMagick-c++-devel \
            jack-audio-connection-kit-devel \
            mlt-devel \
            ocl-icd-devel \
            opencl-headers \
            gtkmm30-devel \
            glibmm24-devel \
            SDL2-devel \
            SDL2_mixer-devel \
            libxslt-devel python-devel python3-lxml"

    if ! ( rpm -qv $PKG_LIST ); then
        echo "Running yum (root privileges are needed)..."
        su -c "yum install $PKG_LIST" || true
    fi

elif [ "$ID_LIKE" == "suse opensuse" ]; then
    PKG_LIST="git libpng-devel libjpeg-devel freetype-devel fontconfig-devel atk-devel pango-devel cairo-devel gtk3-devel gettext-devel libxml2-devel libxml++-devel gcc-c++ autoconf automake libtool libtool-ltdl-devel shared-mime-info"
    PKG_LIST="${PKG_LIST} OpenEXR-devel libmng-devel ImageMagick-c++-devel gtkmm3-devel glibmm2-devel"

    if ! ( rpm -qv $PKG_LIST ); then
        echo "Running zypper (root privileges are needed)..."
        su -c "zypper install $PKG_LIST" || true

        # Add python-lxml repository -> 3rd party
        echo "Adding third party repository for python-lxml..."
        su -c "zypper addrepo https://download.opensuse.org/repositories/devel:languages:python/openSUSE_Tumbleweed/devel:languages:python.repo"
        su -c "zypper refresh"
        su -c "zypper install python-lxml"
    fi

elif [ "$ID_LIKE" == "arch" ]; then
    PKG_LIST="git \
            automake autoconf \
            cairo \
            freetype2 \
            fftw \
            gtk3 \
            gettext \
            gtkmm3 \
            glibmm \
            gcc \
            imagemagick \
            pkg-config \
            intltool \
            jack \
            libxml2 \
            libxml++2.6 \
            libtool \
            libpng \
            libsigc++ \
            libjpeg \
            libmng \
            mlt \
            openexr \
            shared-mime-info \
            cmake make \
            python-lxml"
    echo "Running pacman (root privileges are needed)..."
    echo
    sudo pacman -S --needed --noconfirm $PKG_LIST || true
elif [ -f /etc/altlinux-release ]; then
            #  ALT Linux case
            PKG_LIST=" \
                gcc-c++ \
                git-core \
                shared-mime-info \
                intltool \
                gettext \
                libjpeg-devel \
                fontconfig \
                libfreetype-devel \
                libfribidi-devel \
                fontconfig-devel \
                libxml2-devel \
                libtiff-devel \
                libjasper-devel \
                libdirectfb-devel \
                libfftw3-devel \
                libXfixes-devel \
                libXinerama-devel \
                libXdamage-devel \
                libXcomposite-devel \
                libXcursor-devel \
                libXft-devel \
                libXrender-devel \
                libXt-devel \
                libXrandr-devel \
                libXi-devel \
                libXext-devel \
                libX11-devel \
                libatk-devel \
                libpng-devel \
                bzip2 \
                libjack-devel \
                libmng-devel \
                libgtkmm3-devel \
                libglibmm-devel \
                libsigc++2-devel \
                libxml++2-devel \
                libImageMagick-devel \
                libmlt++-devel \
                libxslt-devel python-devel python-module-lxml"
    echo "Running apt-get (root privileges required)..."
    echo
    su -c "( apt-get update || true ) && apt-get install -y -q $PKG_LIST"
elif [ "$ID_LIKE" == "debian" ] || [ "$ID_LIKE" == "ubuntu" ] || [ "$ID_LIKE" == "ubuntu debian" ]; then
            #  Debian / Ubuntu
            PKG_LIST=" \
                build-essential \
                autoconf automake autopoint \
                shared-mime-info \
                libltdl3-dev \
                libtool \
                intltool \
                gettext \
                libpng-dev \
                libfftw3-dev \
                fontconfig \
                libfreetype6-dev \
                libfribidi-dev \
                libfontconfig1-dev \
                libxml2-dev \
                libtiff5-dev \
                libmlt-dev libmlt++-dev libmlt-data \
                libgl1-mesa-dev \
                imagemagick \
                libsdl2-dev \
                libsdl2-mixer-dev \
                bzip2 \
                git-core \
                libmng-dev \
                libjack-jackd2-dev \
                libgtkmm-3.0-dev \
                libglibmm-2.4-dev \
                libsigc++-2.0-dev \
                libxml++2.6-dev \
                libmagick++-dev \
                libxslt-dev python3 python3-lxml"

    echo "Running apt-get (root privileges are needed)..."
    echo
    sudo apt-get update -qq || true
    sudo apt-get install -y -q $PKG_LIST
else
    echo "WARNING: This build script does not work with package management systems other than yum, zypper, apt or pacman! You should install dependent packages manually."
    echo "REQUIRED PACKAGES: "
    echo "libpng-devel libjpeg-devel freetype-devel fontconfig-devel atk-devel pango-devel cairo-devel gtk3-devel gettext-devel libxml2-devel libxml++-devel gcc-c++ autoconf automake libtool libtool-ltdl-devel shared-mime-info OpenEXR-devel libmng-devel ImageMagick-c++-devel gtkmm30-devel glibmm24-devel"
    echo ""
fi

echo "Done."
