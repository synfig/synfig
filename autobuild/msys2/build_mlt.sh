#!/usr/bin/env bash
# -------------------------------------------------------------------------------
# This script builds mlt++ required by synfig-core
# -------------------------------------------------------------------------------
set -e # exit on error

SCRIPT_DIR=`dirname "$0"`

# set environment variables
source ${SCRIPT_DIR}/set_env.sh

VERSION_MLT="6.16.0"
PATH="${PREFIX}/lib/ccache/bin:${PATH}"

if [ ! -f /opt/mlt-${VERSION_MLT}/done ]; then

echo " ======================= Compiling MLT++ ======================= "

mkdir -p /opt/mlt-${VERSION_MLT}/

cd /tmp
wget "https://github.com/mltframework/mlt/releases/download/v${VERSION_MLT}/mlt-${VERSION_MLT}.tar.gz"
tar xzf ./mlt-${VERSION_MLT}.tar.gz

pushd mlt-${VERSION_MLT}/
echo "Install path: ${PREFIX}"
./configure --prefix=/opt/mlt-${VERSION_MLT} --target-arch=$MSYS2_ARCH --disable-gtk2
make -j2 --silent
make install
popd

touch /opt/mlt-${VERSION_MLT}/done

fi
