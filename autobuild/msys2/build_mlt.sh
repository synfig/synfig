#!/usr/bin/env bash
# -------------------------------------------------------------------------------
# This script builds mlt++ on MSYS2 required by synfig-core
# -------------------------------------------------------------------------------
set -e # exit on error

SCRIPT_DIR=`dirname "$0"`

# set environment variables
source ${SCRIPT_DIR}/set_env.sh

VERSION_MLT="6.16.0"
PATH="${PREFIX}/lib/ccache/bin:${PATH}"
MLT_PATH="/opt/mlt-${VERSION_MLT}"

if [ ! -f ${MLT_PATH}/done ]; then

echo " ======================= Compiling MLT++ ======================= "

mkdir -p ${MLT_PATH}

pushd /tmp
wget "https://github.com/mltframework/mlt/releases/download/v${VERSION_MLT}/mlt-${VERSION_MLT}.tar.gz"
tar xzf ./mlt-${VERSION_MLT}.tar.gz

pushd mlt-${VERSION_MLT}/
echo "Install path: ${MLT_PATH}"
./configure --prefix=${MLT_PATH} --target-arch=$MSYS2_ARCH --disable-gtk2
make -j2 --silent
make install
popd

touch ${MLT_PATH}/done

popd
fi
