#!/bin/sh

echo +++ Running autoreconf ... &&
autoreconf --install --force &&
echo +++ Running intltoolize ... &&
intltoolize --force --copy &&
#echo +++ Running libtoolize ... &&
#libtoolize --copy &&
#echo +++ Running aclocal ... &&
#aclocal -I m4 &&
#echo +++ Running autoconf ... &&
#autoconf && 
#echo +++ Running automake --add-missing ... &&
#automake --add-missing --copy --gnu -Wno-portability &&
#echo +++ Running automake ... &&
#automake Makefile src/Makefile &&
echo You may now run ./configure ||
( echo ERROR.; false )

