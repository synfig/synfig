#!/usr/bin/env bash

set -e

WORKDIR=`dirname "$0"`
cd "${WORKDIR}"
WORKDIR=`pwd`

mkdir -p cmake-build && cd cmake-build

mode="Release"
if [[ -n $1 ]]
then
mode=$1
fi

cmake -GNinja -DENABLE_TESTS=ON -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_INSTALL_PREFIX=./install -DCMAKE_BUILD_TYPE=${mode} ..
cmake --build .
cmake --install . >/dev/null
ctest --output-on-failure

echo
echo
echo "Done. Please check your Synfig installation in"
echo " ${WORKDIR}/cmake-build/output/$mode/"
echo
echo "You can start Synfig by executing"
echo " ${WORKDIR}/cmake-build/output/$mode/bin/synfigstudio"
echo
