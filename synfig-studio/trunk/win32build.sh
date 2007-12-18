#!/bin/sh

OPTIONS=""
OPTIONS="$OPTIONS --disable-optimization"
#OPTIONS="$OPTIONS --disable-debug"
OPTIONS="$OPTIONS --enable-debug"


BUILDDIR=win32build

[ -e configure ] || ./bootstrap || exit 1

[ -d $BUILDDIR ] && rm -fr $BUILDDIR

mkdir $BUILDDIR

cd $BUILDDIR

../configure --host=mingw32 --prefix=C:/PROGRA~1/Synfig $OPTIONS || exit 1

make package


