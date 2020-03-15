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
# boost cairo fftw pango (image transformation and rendering functions)
# gtkmm3 (Synfig Studio GUI)
# libsig++ (GUI signals and events)
# sdl2 sdl2_mixer jack (audio output and synchronization)
#
# 3. Runtime dependencies:
# adwaita-icon-theme (used to render GUI icons and styles)
# python3-lxml (lottie exporter plugin)

set -e

echo "Checking dependencies..."
if command -v dnf >/dev/null; then
    #
    #  Fedora >= 22
    #
    PKG_LIST="git \
            intltool \
            libpng-devel \
            libjpeg-devel \
            fftw-devel \
            freetype-devel \
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
            boost-devel \
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
        echo "Running dnf (you need root privileges to do that)..."
        sudo dnf install $PKG_LIST || true
    fi

elif command -v yum >/dev/null; then
    #
    #  Fedora
    #
    PKG_LIST="git \
            intltool \
            libpng-devel \
            libjpeg-devel \
            fftw-devel \
            freetype-devel \
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
            boost-devel \
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
        echo "Running yum (you need root privileges to do that)..."
        su -c "yum install $PKG_LIST" || true
    fi

elif which zypper >/dev/null; then
    #
    #  OpenSUSE
    #
    PKG_LIST="git libpng-devel libjpeg-devel freetype-devel fontconfig-devel atk-devel pango-devel cairo-devel gtk3-devel gettext-devel libxml2-devel libxml++-devel gcc-c++ autoconf automake libtool libtool-ltdl-devel boost-devel shared-mime-info"
    PKG_LIST="${PKG_LIST} OpenEXR-devel libmng-devel ImageMagick-c++-devel gtkmm3-devel glibmm2-devel"

    if ! ( rpm -qv $PKG_LIST ); then
        echo "Running zypper (you need root privileges to do that)..."
        su -c "zypper install $PKG_LIST" || true

        # Add python lxml repository -> 3rd party
        su -c "zypper addrepo https://download.opensuse.org/repositories/devel:languages:python/openSUSE_Tumbleweed/devel:languages:python.repo"
        su -c "zypper refresh"
        su -c "zypper install python-lxml"
    fi

elif command -v pacman >/dev/null; then
    #
    #  Arch Linux
    #
    PKG_LIST="git \
            automake autoconf \
            boost \
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
    echo "Running pacman (you need root privileges to do that)..."
    echo
    sudo pacman -S --needed --noconfirm $PKG_LIST || true
    
elif command -v apt-get >/dev/null; then
        if [ ! -f /etc/altlinux-release ]; then
            #
            #  Ubuntu/Debian
            #
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
                libfontconfig1-dev \
                libxml2-dev \
                libtiff5-dev \
                libmlt-dev libmlt++-dev libmlt-data \
                x11proto-xext-dev libdirectfb-dev libxfixes-dev libxinerama-dev libxdamage-dev libxcomposite-dev libxcursor-dev libxft-dev libxrender-dev libxt-dev libxrandr-dev libxi-dev libxext-dev libx11-dev \
                libatk1.0-dev \
                libgl1-mesa-dev \
                imagemagick \
                libsdl2-dev \
                libsdl2-mixer-dev \
                bzip2
                git-core \
                libmng-dev \
                libjack-jackd2-dev \
                libgtkmm-3.0-dev \
                libglibmm-2.4-dev \
                libsigc++-2.0-dev \
                libxml++2.6-dev \
                libboost-system-dev \
                libmagick++-dev \
                libxslt-dev python-dev python3-lxml\
            "
        else
            #
            #  ALT Linux case
            #
            PKG_LIST=" \
                rpm-build \
                git-core \
                shared-mime-info \
                libltdl3-devel \
                intltool \
                gettext \
                libpng12-devel \
                libjpeg-devel \
                fontconfig \
                libfreetype-devel \
                fontconfig-devel \
                libxml2-devel \
                libtiff-devel \
                libjasper-devel \
                libdirectfb-devel \
                libfftw3-dev \
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
                bzip2 \
                libmng-devel \
                libgtkmm3-devel \
                libglibmm-devel \
                libsigc++2-devel \
                libxml++2-devel \
                libxslt-devel python-devel python3-lxml\
            "
        fi
    echo "Running apt-get (you need root privileges to do that)..."
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
