#!/bin/bash

set -e

export CC="ccache gcc"
export CXX="ccache g++"

#BUILD_FLAGS=(-GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="-fdiagnostics-color")
CXX_FLAGS="-fdiagnostics-color"
BUILD_FLAGS="-GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS=\"${CXX_FLAGS}\""
#echo ${BUILD_FLAGS}
#exit

cd ..
pushd ETL
mkdir -p build && pushd build
cmake .. ${BUILD_FLAGS}
cmake --build . -- all test
sudo cmake --build . -- install
popd # build
popd # ETL

pushd synfig-core
mkdir -p build && pushd build
cmake .. ${BUILD_FLAGS}
cmake --build . -- all
sudo cmake --build . -- install
popd # build
popd # synfig-core

pushd synfig-studio
mkdir -p build && pushd build
cmake .. ${BUILD_FLAGS}
cmake --build . -- all
# this will take a while; alternatively, you can move/copy required images
# to build/images directory and skip this step
cmake --build . -- build_images
sudo cmake --build . -- install
popd # build
popd # synfig-studio