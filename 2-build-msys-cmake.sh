#!/usr/bin/env bash

export PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:/opt/mlt-6.16.0/lib/pkgconfig"
echo "${PKG_CONFIG_PATH}"
ls -la /opt/mlt-6.16.0/lib/pkgconfig/
mkdir -p build && cd build
cmake -GNinja .. -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER_LAUNCHER=ccache
ninja
