#!/bin/sh

set -e

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

LIBTOOLIZE=`which libtoolize`
if test -z $LIBTOOLIZE; then
        LIBTOOLIZE=`which glibtoolize`
        if ! test -z $LIBTOOLIZE; then
                echo "Using glibtoolize. Is it OSX?"
        fi
fi

if test -z $LIBTOOLIZE; then
        echo "*** No libtoolize nor glibtoolize found, please install the intltool package ***"
        exit 1
fi

echo "running libtooize ($LIBTOOLIZE)..."
$LIBTOOLIZE  --ltdl --copy --force

echo "running autopoint..."
autopoint --force

echo "running autoreconf..."
AUTOPOINT='intltoolize --automake --copy' autoreconf --force --install --verbose

# WORKAROUND 2013-08-15:
# Patch the generated po/Makefile.in.in file so that locale files are installed
# in the correct location on OS X and Free-BSD systems.  This is a workaround
# for a bug in intltool.  
# See https://launchpad.net/bugs/398571 and https://bugs.launchpad.net/bugs/992047
#
# TODO: Drop this hack, and bump our intltool version requiement once the issue
#       is fixed in intltool

echo "patching po/Makefile.in.in..."
sed 's/itlocaledir = $(prefix)\/$(DATADIRNAME)\/locale/itlocaledir = $(datarootdir)\/locale/' < po/Makefile.in.in > po/Makefile.in.in.tmp
mv po/Makefile.in.in.tmp po/Makefile.in.in

echo "Done! Please run ./configure now."
