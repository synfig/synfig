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

SCRIPT_DIR=`dirname "$0"`

# set environment variables
source ${SCRIPT_DIR}/autobuild/msys2/set_env.sh

echo "Selected ARCH: ${ARCH}"

#echo "Sync pacman package databases"
#pacman -Sy

# dlfcn is still required to MLT build, so don't remove it
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
$ARCH-ccache \
$ARCH-libtool \
$ARCH-make \
$ARCH-pkg-config \
$ARCH-dlfcn \
$ARCH-SDL2  \
$ARCH-boost \
$ARCH-cairomm \
$ARCH-fftw \
$ARCH-glibmm \
$ARCH-imagemagick \
$ARCH-libxml++2.6 \
$ARCH-pango \
$ARCH-gtkmm3 \
$ARCH-libmng \
$ARCH-openexr

# build mlt
bash ${SCRIPT_DIR}/autobuild/msys2/build_mlt.sh

# Apply patch to libintl.h. This required because libintl.h redefines sprintf
# and std::sprintf is stop working. But std::sprintf is used by Boost::Odeint library
# so we need it.

patch $PREFIX/include/libintl.h < ${SCRIPT_DIR}/autobuild/msys2/libintl.h.patch


