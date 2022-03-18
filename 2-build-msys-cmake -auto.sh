#!/usr/bin/env bash

export PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:/opt/mlt-7.2.0/lib/pkgconfig"
mkdir -p cmake-build-msys && cd cmake-build-msys
for ((;;))
do
echo type Debug for debug build, or Release for a release build
read buildType
if [[ $buildType = Debug ]]
then 
break
elif [[ $buildType = Release ]]
then
break
fi
done
cmake -GNinja -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_INSTALL_PREFIX=./install -DCMAKE_BUILD_TYPE=$buildType ..
cmake --build .
cmake --install . >/dev/null
