#!/bin/bash

# Make sure we have all dependencies installed
echo "Checking dependencies..."
DEB_LIST_MINIMAL="\
    build-essential \
    autoconf automake \
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
    libmlt-dev libmlt++-dev \
    libjasper-dev \
    x11proto-xext-dev libdirectfb-dev libxfixes-dev libxinerama-dev libxdamage-dev libxcomposite-dev libxcursor-dev libxft-dev libxrender-dev libxt-dev libxrandr-dev libxi-dev libxext-dev libx11-dev \
    libatk1.0-dev \
    libgl1-mesa-dev \
    imagemagick \
    bzip2"
if which dnf >/dev/null; then
    #
    #  Fedora >= 22
    #
    PKG_LIST="git"
    if [[ $MODE == 'package' ]]; then
        PKG_LIST="${PKG_LIST} \
            debootstrap \
            rsync"
    else
        PKG_LIST="${PKG_LIST} \
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
            boost-program-options \
            shared-mime-info \
            OpenEXR-devel \
            libmng-devel \
            ImageMagick-c++-devel \
            jack-audio-connection-kit-devel \
            mlt-devel \
            ocl-icd-devel \
            opencl-headers \
            gtkmm30-devel \
            glibmm24-devel"
    fi
    if ! ( rpm -qv $PKG_LIST ); then
        echo "Running dnf (you need root privelegies to do that)..."
        su -c "dnf install $PKG_LIST" || true
    fi
elif which yum >/dev/null; then
    #
    #  Fedora
    #
    PKG_LIST="git"
    if [[ $MODE == 'package' ]]; then
        PKG_LIST="${PKG_LIST} \
            debootstrap \
            rsync"
    else
        PKG_LIST="${PKG_LIST} \
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
            boost-program-options \
            shared-mime-info \
            OpenEXR-devel \
            libmng-devel \
            ImageMagick-c++-devel \
            jack-audio-connection-kit-devel \
            mlt-devel \
            ocl-icd-devel \
            opencl-headers \
            gtkmm30-devel \
            glibmm24-devel"
    fi
    if ! ( rpm -qv $PKG_LIST ); then
        echo "Running yum (you need root privelegies to do that)..."
        su -c "yum install $PKG_LIST" || true
    fi
elif which zypper >/dev/null; then
    #
    #  OpenSUSE
    #
    PKG_LIST="git"
    if [[ $MODE == 'package' ]]; then
        PKG_LIST="${PKG_LIST} \
            debootstrap \
            rsync"
    else
        PKG_LIST="${PKG_LIST} libpng-devel libjpeg-devel freetype-devel fontconfig-devel atk-devel pango-devel cairo-devel gtk3-devel gettext-devel libxml2-devel libxml++-devel gcc-c++ autoconf automake libtool libtool-ltdl-devel boost-devel boost-program-options shared-mime-info"
        PKG_LIST="${PKG_LIST} OpenEXR-devel libmng-devel ImageMagick-c++-devel gtkmm3-devel glibmm2-devel"
    fi
    if ! ( rpm -qv $PKG_LIST ); then
        echo "Running zypper (you need root privelegies to do that)..."
        su -c "zypper install $PKG_LIST" || true
    fi
elif which apt-get >/dev/null; then
    if [[ $MODE == 'package' ]]; then
        if [[ `cat /etc/chroot.id` == "Synfig Packages Buildroot v${BUILDROOT_VERSION}" ]]; then
            #we are inside of chroot
            PKG_LIST="$DEB_LIST_MINIMAL rpm alien xsltproc wget python"
        else
            #we have to prepare chroot
            PKG_LIST="git-core debootstrap rsync"
        fi
    else
        if ( cat /etc/altlinux-release | egrep "ALT Linux" ); then
            #
            #  ALT Linux case
            #
            PKG_LIST=" \
                rpm-build \
                boost-program_options-devel \
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
            "
        else
            #
            #  Ubuntu/Debian case
            #
            PKG_LIST=" \
                ${DEB_LIST_MINIMAL} \
                git-core \
                libmng-dev \
                libjack-jackd2-dev \
                libgtkmm-3.0-dev \
                libglibmm-2.4-dev \
                libsigc++-2.0-dev \
                libxml++2.6-dev \
                libboost-program-options-dev \
                libboost-system-dev \
                libboost-filesystem-dev \
                libboost-chrono-dev \
                libmagick++-dev \
            "
        fi
    fi
    echo "Running apt-get (you need root privelegies to do that)..."
    echo
    sudo apt-get update || true
    sudo apt-get install -y $PKG_LIST
    sudo apt-get install -y autopoint || true # Ubuntu special case
else
    if [[ $MODE == 'package' ]]; then
        if ! ( which git && which debootstrap ) ; then
            echo "ERROR: Please install 'git' and 'debootstrap'."
            exit;
        fi
    else
        echo "WARNING: This build script does not works with package mangement systems other than yum, zypper or apt! You should install dependent packages manually."
        echo "REQUIRED PACKAGES: libpng-devel libjpeg-devel freetype-devel fontconfig-devel atk-devel pango-devel cairo-devel gtk3-devel gettext-devel libxml2-devel libxml++-devel gcc-c++ autoconf automake libtool libtool-ltdl-devel shared-mime-info OpenEXR-devel libmng-devel ImageMagick-c++-devel gtkmm30-devel glibmm24-devel"
        echo ""
        read
    fi
fi
echo "Done."
