#!/bin/sh

#OPTIONS="--disable-optimization --enable-debug"

OPTIONS=""
#OPTIONS="$OPTIONS --enable-timelimit=120"
#OPTIONS="$OPTIONS --enable-optimization=2"
OPTIONS="$OPTIONS --enable-optimization=3 --enable-g5opt"
#OPTIONS="$OPTIONS --disable-optimization"
OPTIONS="$OPTIONS --disable-debug"
#OPTIONS="$OPTIONS --with-vimage"

CC=gcc
CXX=g++

#distcc --version && {
#	CC=distcc
#	CXX="distcc g++"
#}


BUILDDIR=macosxbuild

[ -e configure ] || ./bootstrap || exit 1

[ -d $BUILDDIR ] && rm -fr $BUILDDIR

mkdir $BUILDDIR

cd $BUILDDIR

echo ../configure $OPTIONS CC="$CC" CXX="$CXX"

../configure $OPTIONS CC="$CC" CXX="$CXX" || exit 1

make package $MAKEFLAGS

#make installer


