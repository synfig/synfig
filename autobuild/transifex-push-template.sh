#!/bin/sh

set -e

SCRIPTPATH="${BASH_SOURCE[0]}";
pushd `dirname "${SCRIPTPATH}"` > /dev/null
SCRIPTPATH=`pwd`;
popd  > /dev/null

cd "$SCRIPTPATH/../synfig-core/po"
bash $SCRIPTPATH/transifex-fix-pot.sh synfig.pot

cd "$SCRIPTPATH/../synfig-studio/po"
rm synfigstudio.pot || true
intltool-update --pot
bash $SCRIPTPATH/transifex-fix-pot.sh synfigstudio.pot

cd "$SCRIPTPATH/.."
tx push -s
