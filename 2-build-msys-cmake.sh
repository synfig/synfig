#!/usr/bin/env bash

export PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:/opt/mlt-7.2.0/lib/pkgconfig"
mkdir -p cmake-build-msys && cd cmake-build-msys
mode="Release"
if [[ -n $1 ]] 
then
mode=$1
fi
cmake -GNinja -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_INSTALL_PREFIX=./install -DCMAKE_BUILD_TYPE=${mode} ..
cmake --build .
cmake --install . >/dev/null
