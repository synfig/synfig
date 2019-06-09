#!/bin/sh

set -e

AUTORECONF=`command -v autoreconf || true` #if don't set true, script fails with no messages
if [ -z $AUTORECONF ]; then
        echo "*** No autoreconf found, please install it ***"
        exit 1
fi


rm -f aclocal.m4

echo "running autoreconf..."
autoreconf --install --force

echo "Done! Please run ./configure now."
