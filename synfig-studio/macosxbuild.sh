#!/bin/sh

#PREFIX=/Applications/SynfigStudio.app/Contents/synfig
PREFIX=/Users/darco/Projects/Voria/synfig-build
#OPTIONS="--disable-optimization --enable-debug"
export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig:/usr/lib/pkgconfig:/usr/X11R6/lib/pkgconfig
export MACOSX_DEPLOYMENT_TARGET=10.4
OPTIONS="--prefix=$PREFIX"
OPTIONS="$OPTIONS --enable-optimization=2"
#OPTIONS="$OPTIONS --disable-optimization"
OPTIONS="$OPTIONS --disable-debug"
OPTIONS="$OPTIONS --disable-dependency-tracking"
#OPTIONS="$OPTIONS --enable-debug"
#export LDFLAGS='-undefined dynamic-lookup'
LDFLAGS="-prebind -prebind_all_twolevel_modules -twolevel_namespace -bind_at_load -undefined dynamic_lookup"
BUILDDIR=macosxbuild

ARCH_FLAGS=""
#ARCH_FLAGS="$ARCH_FLAGS -arch ppc"
ARCH_FLAGS="$ARCH_FLAGS -arch i386"

CC="gcc $ARCH_FLAGS"
CXX="g++ $ARCH_FLAGS"

CPP="gcc -E"
CXXCPP="g++ -E"


[ -e configure ] || autoreconf --force --install || exit 1

[ -d $BUILDDIR ] && rm -fr $BUILDDIR

mkdir $BUILDDIR

cd $BUILDDIR

set -x

../configure $OPTIONS LDFLAGS="$LDFLAGS" CC="$CC" CXX="$CXX" CPP="$CPP" CXXCPP="$CXXCPP" || exit 1

make -j2
make install-strip
#make package

#make installer


