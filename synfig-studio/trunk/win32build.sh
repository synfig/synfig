#!/bin/sh

#OPT_FLAGS="--disable-optimization --disable-debug --enable-license-key"
OPTIONS="--disable-optimization --disable-debug"


BUILDDIR=win32build

[ -e configure ] || ./bootstrap || exit 1

 [ -d $BUILDDIR ] && rm -fr $BUILDDIR

mkdir $BUILDDIR

cd $BUILDDIR

../configure $OPTIONS || exit 1

make -j2
#make package

make installer

