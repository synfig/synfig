#!/bin/sh

OPT_FLAGS="--disable-optimization --disable-debug --enable-license-key"

./configure --host=mingw32 --prefix=C:/PROGRA~1/Synfig $OPT_FLAGS

make clean

make installer

