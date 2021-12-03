#!/usr/bin/env bash
# -------------------------------------------------------------------------------
# This script builds mlt++ on MSYS2 required by synfig-core
# -------------------------------------------------------------------------------
set -e # exit on error

VERSION_MLT="7.2.0"
# CMake cannot invoke `ccache` binaries in MSYS2 because they are just
# bash scripts so it ends up with "invalid Win32 application" error
# PATH="${MINGW_PREFIX}/lib/ccache/bin:${PATH}"
MLT_PATH="/opt/mlt-${VERSION_MLT}"

if [ ! -f ${MLT_PATH}/done ] || [ ! -f ${MLT_PATH}/lib/mlt/libmltavformat.dll ]; then

echo " ======================= Compiling MLT++ ======================= "

mkdir -p ${MLT_PATH}

pushd /tmp
wget "https://github.com/mltframework/mlt/releases/download/v${VERSION_MLT}/mlt-${VERSION_MLT}.tar.gz"
tar xzf ./mlt-${VERSION_MLT}.tar.gz

pushd mlt-${VERSION_MLT}/
FIXED_MLT_PATH=$(cygpath -m ${MLT_PATH})
echo "Install path: ${MLT_PATH}"
echo "Fixed MLT Path: ${FIXED_MLT_PATH}"
mkdir -p build && pushd build
cmake -G"Ninja" -DMOD_GDK=OFF -DMOD_QT=OFF -DMOD_KDENLIVE=OFF -DCMAKE_INSTALL_PREFIX=${FIXED_MLT_PATH} ..
cmake --build .
cmake --install .
popd
popd

touch ${MLT_PATH}/done

popd
fi
