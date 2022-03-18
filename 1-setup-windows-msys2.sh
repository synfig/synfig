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

SCRIPT_DIR=$(dirname "$0")

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
libtool \
make \
patch \
tar \
$MINGW_PACKAGE_PREFIX-boost \
$MINGW_PACKAGE_PREFIX-ccache \
$MINGW_PACKAGE_PREFIX-cmake \
$MINGW_PACKAGE_PREFIX-dlfcn \
$MINGW_PACKAGE_PREFIX-ffmpeg \
$MINGW_PACKAGE_PREFIX-fftw \
$MINGW_PACKAGE_PREFIX-gcc \
$MINGW_PACKAGE_PREFIX-glibmm \
$MINGW_PACKAGE_PREFIX-gtkmm3 \
$MINGW_PACKAGE_PREFIX-harfbuzz \
$MINGW_PACKAGE_PREFIX-imagemagick \
$MINGW_PACKAGE_PREFIX-libltdl \
$MINGW_PACKAGE_PREFIX-libmng \
$MINGW_PACKAGE_PREFIX-libxml++2.6 \
$MINGW_PACKAGE_PREFIX-make \
$MINGW_PACKAGE_PREFIX-ninja \
$MINGW_PACKAGE_PREFIX-openexr \
$MINGW_PACKAGE_PREFIX-pkgconf \
$MINGW_PACKAGE_PREFIX-SDL2

# build mlt
bash "${SCRIPT_DIR}/autobuild/msys2/build_mlt.sh"
