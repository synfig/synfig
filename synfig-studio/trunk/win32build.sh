#!/bin/sh

OPTIONS=""
OPTIONS="$OPTIONS --disable-optimization"
OPTIONS="$OPTIONS --disable-debug"
#OPTIONS="$OPTIONS --enable-license-key"


BUILDDIR=win32build

[ -e configure ] || ./bootstrap || exit 1

[ -d $BUILDDIR ] && rm -fr $BUILDDIR

mkdir $BUILDDIR

cd $BUILDDIR

../configure $OPTIONS || exit 1

make package


