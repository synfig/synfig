#!/usr/bin/env bash

export PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:/opt/mlt-6.16.0/lib/pkgconfig"
mkdir -p cmake-build-msys && cd cmake-build-msys
cmake -GNinja -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_INSTALL_PREFIX=./install -DCMAKE_BUILD_TYPE=Release ..
ninja
ninja install
