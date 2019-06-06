#!/usr/bin/env bash
# -------------------------------------------------------------------------------
# This script builds mlt++ required by synfig-core
# -------------------------------------------------------------------------------
set -e # exit on error

# set environment variables
source ./set_env.sh

echo " ======================= Compiling MLT++ ======================= "
wget "https://github.com/mltframework/mlt/releases/download/v6.16.0/mlt-6.16.0.tar.gz"
tar xzf ./mlt-6.16.0.tar.gz

pushd mlt-6.16.0/
echo "Install path: ${PREFIX}"
./configure --prefix=${PREFIX} --target-arch=$MSYS2_ARCH --disable-gtk2
make -j${NUM_THREADS} --silent
make install
popd
