#!/bin/sh

#OPTIONS="--disable-optimization --enable-debug"
OPTIONS="--enable-optimization=1 --disable-debug --enable-timelimit=15 --enable-license-key"

make distclean

./configure --host=mingw32 --prefix=C:/PROGRA~1/Synfig $OPTIONS

make installer

