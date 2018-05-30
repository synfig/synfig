#! /bin/sh

rm -f ./aclocal.m4
rm -f ./configure
aclocal
automake --foreign --add-missing
autoreconf --install --force