#!/usr/bin/env bash
# -------------------------------------------------------------------------------
# This script builds mlt++ on MSYS2 required by synfig-core
# -------------------------------------------------------------------------------
set -e # exit on error

SCRIPT_DIR=`dirname "$0"`

VERSION_MLT="6.16.0"
PATH="${MINGW_PREFIX}/lib/ccache/bin:${PATH}"
MLT_PATH="/opt/mlt-${VERSION_MLT}"

if [ ! -f ${MLT_PATH}/done ] || [ ! -f ${MLT_PATH}/lib/mlt/libmltavformat.dll ]; then

echo " ======================= Compiling MLT++ ======================= "

mkdir -p ${MLT_PATH}

pushd /tmp
wget "https://github.com/mltframework/mlt/releases/download/v${VERSION_MLT}/mlt-${VERSION_MLT}.tar.gz"
tar xzf ./mlt-${VERSION_MLT}.tar.gz

pushd mlt-${VERSION_MLT}/
FIXED_MLT_PATH=`cygpath -m ${MLT_PATH}`
echo "Install path: ${MLT_PATH}"
echo "Fixed MLT Path: ${FIXED_MLT_PATH}"
./configure --prefix=${FIXED_MLT_PATH} --target-arch=$MSYSTEM_CARCH --disable-gtk2 --enable-avformat
make -j2 --silent
make install
popd

touch ${MLT_PATH}/done

popd
fi
