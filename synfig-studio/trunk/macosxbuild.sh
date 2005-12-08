#!/bin/sh

#OPTIONS="--disable-optimization --enable-debug"
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/lib/pkgconfig:/usr/X11R6/lib/pkgconfig
export MACOSX_DEPLOYMENT_TARGET=10.4
OPTIONS=""
#OPTIONS="$OPTIONS --enable-timelimit=120"
OPTIONS="$OPTIONS --enable-optimization=2"
#OPTIONS="$OPTIONS --enable-optimization=3 --enable-g5opt"
#OPTIONS="$OPTIONS --disable-optimization"
OPTIONS="$OPTIONS --disable-debug"
#OPTIONS="$OPTIONS --enable-debug"
#export LDFLAGS='-undefined dynamic-lookup'
LDFLAGS="-prebind -prebind_all_twolevel_modules -twolevel_namespace -bind_at_load -undefined dynamic_lookup"
BUILDDIR=macosxbuild

[ -e configure ] || ./bootstrap || exit 1

[ -d $BUILDDIR ] && rm -fr $BUILDDIR

mkdir $BUILDDIR

cd $BUILDDIR

set -x

../configure $OPTIONS LDFLAGS="$LDFLAGS" || exit 1

make -j2
make package

#make installer


