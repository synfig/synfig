#!/bin/sh

set -e

AUTORECONF=`command -v autoreconf || true` #if don't set true, script fails with no messages
if [ -z $AUTORECONF ]; then
        echo "*** No autoreconf found, please install it ***"
        exit 1
fi

INTLTOOLIZE=`command -v intltoolize || true`
if [ -z $INTLTOOLIZE ]; then
        echo "*** No intltoolize found, please install the intltool package ***"
        exit 1
fi

LIBTOOLIZE=`command -v libtoolize || true`
if [ -z $LIBTOOLIZE ]; then
        LIBTOOLIZE=`command -v glibtoolize || true`
        if ! [ -z $LIBTOOLIZE ]; then
                echo "Using glibtoolize. Is it OSX?"
        fi
fi

if [ -z $LIBTOOLIZE ]; then
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
# -- force didn't work under MacOS
mv -f po/Makefile.in.in.tmp po/Makefile.in.in

echo "Done! Please run ./configure now."
