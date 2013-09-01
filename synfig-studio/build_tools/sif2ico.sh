#!/bin/sh

NAME=`basename "$1" .sif`

OS=`uname -o`

if [[ "$OS" == "Cygwin" ]]; then
GAMMA="-gamma 2.2"
fi

synfig "$1" -w 16 -h 16 -o "$1-16x16.png"
synfig "$1" -w 32 -h 32 -o "$1-32x32.png"
synfig "$1" -w 48 -h 48 -o "$1-48x48.png"
synfig "$1" -w 64 -h 64 -o "$1-64x64.png"
convert "$1-16x16.png" "$1-32x32.png" "$1-48x48.png" "$1-64x64.png" -background transparent $GAMMA ${NAME}.ico
rm -f "$1-16x16.png" "$1-32x32.png" "$1-48x48.png" "$1-64x64.png"
