#! /bin/sh

set -e

AUTORECONF=`command -v autoreconf || true`
if test -z $AUTORECONF; then
        echo "*** No autoreconf found, please install it ***"
        exit 1
fi


rm -f aclocal.m4
autoreconf --install --force
