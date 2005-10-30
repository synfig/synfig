#!/bin/sh

#OPTIONS="--disable-optimization --enable-debug"

OPTIONS="--enable-optimization=1 --disable-debug"
#OPTIONS="$OPTIONS --enable-timelimit=120"

[ -e configure ] || ./bootstrap || exit 0

[ -d win32build ] && rm -fr win32build

mkdir win32build

cd win32build

../configure --host=mingw32 --prefix=C:/PROGRA~1/Synfig $OPTIONS || exit 0

make package


