#!/bin/sh

#OPTIONS="--disable-optimization --enable-debug"

OPTIONS=""
#OPTIONS="$OPTIONS --enable-timelimit=120"
OPTIONS="$OPTIONS --enable-optimization=2"
OPTIONS="$OPTIONS --enable-optimization=3 --enable-g5opt"
#OPTIONS="$OPTIONS --disable-optimization"
OPTIONS="$OPTIONS --disable-debug"

BUILDDIR=macosxbuild

[ -e configure ] || ./bootstrap || exit 1

[ -d $BUILDDIR ] && rm -fr $BUILDDIR

mkdir $BUILDDIR

cd $BUILDDIR

../configure $OPTIONS || exit 1

make package

#make installer


