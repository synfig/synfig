#!/bin/sh

#OPTIONS="--disable-optimization --enable-debug"

PREFIX=/Users/darco/Projects/Voria/synfig-build

export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig:/usr/lib/pkgconfig:/usr/X11R6/lib/pkgconfig
OPTIONS="--prefix=$PREFIX"
#OPTIONS="$OPTIONS --enable-optimization=2"
OPTIONS="$OPTIONS --enable-optimization=3"
#OPTIONS="$OPTIONS --enable-g5opt"
#OPTIONS="$OPTIONS --disable-optimization"
OPTIONS="$OPTIONS --disable-debug"
#OPTIONS="$OPTIONS --with-vimage"
OPTIONS="$OPTIONS --without-openexr"
OPTIONS="$OPTIONS --disable-dependency-tracking"
#OPTIONS="$OPTIONS --enable-universal"

ARCH_FLAGS=""
#ARCH_FLAGS="$ARCH_FLAGS -arch ppc"
ARCH_FLAGS="$ARCH_FLAGS -arch i386"

LDFLAGS="-L$PREFIX/lib"
CFLAGS="-I$PREFIX/include"
CXXFLAGS="-I$PREFIX/include"

CC="gcc $LDFLAGS $CFLAGS $ARCH_FLAGS"
CXX="g++ $LDFLAGS $CFLAGS $ARCH_FLAGS"
CPP="gcc -E"
CXXCPP="g++ -E"

#distcc --version && {
#	CC=distcc
#	CXX="distcc g++"
#}


BUILDDIR=macosxbuild

[ -e configure ] || ( libtoolize --ltdl --copy --force && autoreconf --force --install ) || exit 1

[ -d $BUILDDIR ] && rm -fr $BUILDDIR

mkdir $BUILDDIR

cd $BUILDDIR

echo ../configure $OPTIONS CC=\"$CC\" CXX=\"$CXX\" CPP=\"$CPP\" CXXCPP=\"$CXXCPP\" LDFLAGS=\"$LDFLAGS\" CFLAGS=\"$CFLAGS\" CXXFLAGS=\"$CXXFLAGS\"

../configure $OPTIONS CC="$CC" CXX="$CXX" CPP="$CPP" CXXCPP="$CXXCPP" LDFLAGS="$LDFLAGS" CFLAGS="$CFLAGS" CXXFLAGS="$CXXFLAGS" MACOS_DEPLOYMENT_TARGET=10.4 || exit 1

make -j2
make install-strip

#make package $MAKEFLAGS

#make installer


