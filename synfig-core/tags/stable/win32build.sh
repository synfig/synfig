#!/bin/sh

#OPTIONS="--disable-optimization --enable-debug"

OPTIONS="--enable-optimization=1 --disable-debug --enable-timelimit=120"

[ -d win32build ] && rm -fr win32build

mkdir win32build

cd win32build

../configure --host=mingw32 --prefix=C:/PROGRA~1/Synfig $OPTIONS

make installer


