#!/bin/sh

set -e

DIRNAME=`dirname "$0"`
cd $DIRNAME
DIRNAME=`pwd`

if ! ( git branch |grep "* testing" ); then
	echo "ERROR: You should be on branch 'testing' to invoke this command."
	exit 1
fi

# Update NEWS files
./ChangeLog-split.sh
git add ETL/NEWS synfig-core/NEWS synfig-studio/NEWS

# Get current version
VERSION_CURRENT=`cat synfig-studio/configure.ac | grep "AC_INIT(\[Synfig Studio\]" | sed 's/.*Studio\],\[\(.*\)\],\[http.*/\1/'`
DATE=`date +%Y-%m-%d`
sed -i "s|date=\".*\">|date=\"${DATE}\">|" synfig-studio/org.synfig.SynfigStudio.appdata.xml.in

git add \
	synfig-studio/org.synfig.SynfigStudio.appdata.xml.in \
	# end

git commit -m "Release version ${VERSION_CURRENT}"
git push upstream testing

#git tag v${VERSION_CURRENT}
#git push --tags upstream
