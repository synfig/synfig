#!/bin/bash

export DEBUG=1
export MAKE_THREADS=4

SCRIPT_PATH=$(cd `dirname "$0"`; pwd)
pushd $SCRIPT_PATH
./synfigstudio-osx-build.sh
popd