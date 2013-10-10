#!/bin/sh

set -e

SCRIPTPATH="${BASH_SOURCE[0]}";
pushd `dirname "${SCRIPTPATH}"` > /dev/null
SCRIPTPATH=`pwd`;
popd  > /dev/null

cd "$SCRIPTPATH/.."
tx pull -a --force
