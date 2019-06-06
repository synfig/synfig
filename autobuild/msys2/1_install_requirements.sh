#!/usr/bin/env bash
# -------------------------------------------------------------------------------
# This script installs all dependencies required for building Synfig Studio with MSYS2
# Execute it once on an MSYS shell, i.e.
#    - use the "MSYS2 MSYS" shortcut in the start menu or
#    - run "msys2.exe" in MSYS2's installation folder
#
# MSYS2 and installed libraries can be updated later by executing
#   pacman -Syu
# in an MSYS shell
# -------------------------------------------------------------------------------

# select if you want to build 32-bit (i686), 64-bit (x86_64), or both
case "$MSYSTEM" in
  MINGW32)
    ARCH=mingw-w64-i686
    PREFIX=/mingw32
    ;;
  MINGW64)
    ARCH=mingw-w64-x86_64
    PREFIX=/mingw64
    ;;
  *)
    printf "Unsupported mode! Choose MinGW64 (32 or 64 bit)"
    exit 1
    #ARCH={mingw-w64-i686,mingw-w64-x86_64}
	#ARCH=mingw-w64-x86_64
    # TODO: need to check compilation for both architectures
    ;;
esac
echo "Selected ARCH: ${ARCH}"

#echo "Sync pacman package databases"
#pacman -Sy

echo "Installing development tools"
# install basic development system, compiler toolchain and build tools
pacman -S --needed --noconfirm --color=auto \
autoconf \
automake-wrapper \
intltool \
make \
patch \
tar \
$ARCH-gcc \
$ARCH-libtool \
$ARCH-make \
$ARCH-pkg-config \
$ARCH-dlfcn \
$ARCH-SDL2  \
$ARCH-boost \
$ARCH-cairomm \
$ARCH-fftw \
$ARCH-glibmm \
$ARCH-libxml++2.6 \
$ARCH-pango \
$ARCH-gtkmm3

# Apply patch to libintl.h. This required because libintl.h redefines sprintf
# and std::sprintf is stop working. But std::sprintf is used by Boost::Odeint library
# so we need it.

SCRIPT_DIR=`dirname "$0"`
patch $PREFIX/include/libintl.h < $SCRIPT_DIR/libintl.h.patch
