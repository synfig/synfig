#!/bin/sh

AUTORECONF=`which autoreconf`
if test -z $AUTORECONF; then
        echo "*** No autoreconf found, please install it ***"
        exit 1
fi

INTLTOOLIZE=`which intltoolize`
if test -z $INTLTOOLIZE; then
        echo "*** No intltoolize found, please install the intltool package ***"
        exit 1
fi

autopoint --force
AUTOPOINT='intltoolize --automake --copy' autoreconf --force --install --verbose

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

echo "Done! Please run ./configure now."
