#!/bin/bash

cd ..
pushd ETL
mkdir build && pushd build
cmake -GNinja .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="-fdiagnostics-color"
cmake --build . -- all test
sudo cmake --build . -- install
popd # build
popd # ETL
pushd synfig-core
mkdir build && pushd build
cmake -GNinja .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="-fdiagnostics-color"
cmake --build . -- all
sudo cmake --build . -- install
popd # build
popd # synfig-core
pushd synfig-studio
mkdir build && pushd build
cmake -GNinja .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="-fdiagnostics-color"
cmake --build . -- all
# this will take a while; alternatively, you can move/copy required images
# to build/images directory and skip this step
cmake --build . -- build_images
sudo cmake --build . -- install
popd # build
popd # synfig-studio