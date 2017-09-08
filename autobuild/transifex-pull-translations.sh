#!/bin/bash

set -e

SCRIPTPATH="${BASH_SOURCE[0]}";
pushd `dirname "${SCRIPTPATH}"` > /dev/null
SCRIPTPATH=`pwd`;
popd  > /dev/null

fix_po()
{
	for FILE in `ls -1 *.po`; do
		sed -i '/Report-Msgid-Bugs-To:/c\"Report-Msgid-Bugs-To: https://github.com/synfig/synfig/issues\\n"' "$FILE" || true
	done
}

cd "$SCRIPTPATH/.."
tx pull -a --force

cd "$SCRIPTPATH/../synfig-core/po"
fix_po

cd "$SCRIPTPATH/../synfig-studio/po"
fix_po

"$SCRIPTPATH/update-languages.sh"
