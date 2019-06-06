#!/usr/bin/env bash
# -------------------------------------------------------------------------------
# This script builds mlt++ required by synfig-core
# -------------------------------------------------------------------------------
set -e # exit on error

case "$MSYSTEM" in
  MINGW32)
    PREFIX=/mingw32
    MSYS2_ARCH=i686
    ;;
  MINGW64)
    PREFIX=/mingw64
    MSYS2_ARCH=x86_64
    ;;
  *)
    printf "Unsupported mode! Choose MinGW64 (32 or 64 bit)"
    exit 1
    ;;
esac

echo " ======================= Compiling MLT++ ======================= "
wget "https://github.com/mltframework/mlt/releases/download/v6.16.0/mlt-6.16.0.tar.gz"
tar xzf ./mlt-6.16.0.tar.gz

pushd mlt-6.16.0/
echo "Install path: ${PREFIX}"
./configure --prefix=${PREFIX} --target-arch=$MSYS2_ARCH --disable-gtk2 && make && make install
popd
