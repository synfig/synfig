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

# WORKAROUND 2013-08-15:
# Patch the generated po/Makefile.in.in file so that locale files are installed
# in the correct location on OS X and Free-BSD systems.  This is a workaround
# for a bug in intltool.  
# See https://launchpad.net/bugs/398571 and https://bugs.launchpad.net/bugs/992047
#
# TODO: Drop this hack, and bump our intltool version requiement once the issue
#       is fixed in intltool

sed 's/itlocaledir = $(prefix)\/$(DATADIRNAME)\/locale/itlocaledir = $(datarootdir)\/locale/' < po/Makefile.in.in > po/Makefile.in.in.tmp
mv po/Makefile.in.in.tmp po/Makefile.in.in

echo You may now run ./configure ||
( echo ERROR.; false )

