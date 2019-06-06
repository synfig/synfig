#!/usr/bin/env bash
# -------------------------------------------------------------------------------
# This script builds ETL, synfig-core and synfig-studio
# -------------------------------------------------------------------------------
set -e # exit on error

# assume we are in synfig's root folder
pushd ETL
./bootstrap.sh
./configure --enable-optimization=0
make -j2 --silent
make install
popd

pushd synfig-core
./bootstrap.sh
./configure --enable-optimization=0
make -j2 --silent
make install
popd

# currently where is a bug in synfig-core (in MinGW build) which causes 
# synfig-core to halt before exit, so we skip image generation
pushd synfig-studio
./bootstrap.sh
./configure --enable-optimization=0 --without-images
make -j2 --silent
make install
popd

echo " ============================ Build is successfull ============================ "
