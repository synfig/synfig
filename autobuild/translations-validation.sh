#!/bin/bash

set -e

SCRIPTPATH="${BASH_SOURCE[0]}";
pushd `dirname "${SCRIPTPATH}"` > /dev/null
SCRIPTPATH=`pwd`;
popd  > /dev/null

cd "$SCRIPTPATH/.."
for POTFILE in `ls synfig-core/po/*.po`; do echo $POTFILE; msgfmt --check $POTFILE; done
for POTFILE in `ls synfig-studio/po/*.po`; do echo $POTFILE; msgfmt --check $POTFILE; done
