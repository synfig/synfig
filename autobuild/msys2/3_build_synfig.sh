#!/usr/bin/env bash
# -------------------------------------------------------------------------------
# This script builds ETL, synfig-core and synfig-studio
# -------------------------------------------------------------------------------
set -e # exit on error

SCRIPT_DIR=`dirname "$0"`

# set environment variables
source ${SCRIPT_DIR}/set_env.sh

ccache -s

# assume we are in synfig's root folder
pushd ETL
./bootstrap.sh
./configure --enable-optimization=0
make -j${NUM_THREADS} --silent
make install
popd

pushd synfig-core
./bootstrap.sh
./configure --enable-optimization=0
make -j${NUM_THREADS} --silent
make install
popd

ccache -s

# currently where is a bug in synfig-core (in MinGW build) which causes 
# synfig-core to halt before exit, so we skip image generation
pushd synfig-studio
./bootstrap.sh
./configure --enable-optimization=0 --without-images
make -j${NUM_THREADS} --silent
make install
popd

echo " ============================ Build is successfull ============================ "
