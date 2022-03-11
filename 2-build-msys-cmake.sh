#!/usr/bin/env bash

export PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:/opt/mlt-7.2.0/lib/pkgconfig"
mkdir -p cmake-build-msys && cd cmake-build-msys
echo type Debug for a debug build, or Release for a release build
read buildType
cmake -GNinja -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_INSTALL_PREFIX=./install -DCMAKE_BUILD_TYPE=$buildType ..
cmake --build .
cmake --install . >/dev/null
