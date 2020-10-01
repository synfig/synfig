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

echo "Selected ARCH: ${MINGW_PACKAGE_PREFIX}"

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
$MINGW_PACKAGE_PREFIX-gcc \
$MINGW_PACKAGE_PREFIX-ccache \
$MINGW_PACKAGE_PREFIX-libtool \
$MINGW_PACKAGE_PREFIX-make \
$MINGW_PACKAGE_PREFIX-pkg-config \
$MINGW_PACKAGE_PREFIX-dlfcn \
$MINGW_PACKAGE_PREFIX-SDL2  \
$MINGW_PACKAGE_PREFIX-boost \
$MINGW_PACKAGE_PREFIX-cairomm \
$MINGW_PACKAGE_PREFIX-ffmpeg \
$MINGW_PACKAGE_PREFIX-fftw \
$MINGW_PACKAGE_PREFIX-glibmm \
$MINGW_PACKAGE_PREFIX-imagemagick \
$MINGW_PACKAGE_PREFIX-libxml++2.6 \
$MINGW_PACKAGE_PREFIX-pango \
$MINGW_PACKAGE_PREFIX-gtkmm3 \
$MINGW_PACKAGE_PREFIX-openexr \
$MINGW_PACKAGE_PREFIX-libmng

# build mlt
bash ${SCRIPT_DIR}/autobuild/msys2/build_mlt.sh

# Apply patch to libintl.h. This required because libintl.h redefines sprintf
# and std::sprintf is stop working. But std::sprintf is used by Boost::Odeint library
# so we need it

patch $MINGW_PREFIX/include/libintl.h < ${SCRIPT_DIR}/autobuild/msys2/libintl.h.patch
