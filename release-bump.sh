#!/bin/sh

set -e

DIRNAME=`dirname "$0"`
cd $DIRNAME
DIRNAME=`pwd`

if [ -z "$1" ]; then
	echo "USAGE: $0 VERSION"
	exit 1
fi

VERSION_NEW="$1"

# Get current version
VERSION_CURRENT=`cat synfig-studio/configure.ac | grep "AC_INIT(\[Synfig Studio\]" | sed 's/.*Studio\],\[\(.*\)\],\[http.*/\1/'`

sed -i "s|Library\],\[${VERSION_CURRENT}\],\[http|Library\],\[${VERSION_NEW}\],\[http|" ETL/configure.ac

sed -i "s|Core\],\[${VERSION_CURRENT}\],\[http|Core\],\[${VERSION_NEW}\],\[http|" synfig-core/configure.ac
sed -i "s|ETL >= ${VERSION_CURRENT}|ETL >= ${VERSION_NEW}|" synfig-core/configure.ac
sed -i "s|version ${VERSION_CURRENT} or|version ${VERSION_NEW} or|" synfig-core/configure.ac


sed -i "s|Studio\],\[${VERSION_CURRENT}\],\[http|Studio\],\[${VERSION_NEW}\],\[http|" synfig-studio/configure.ac
sed -i "s|ETL >= ${VERSION_CURRENT}|ETL >= ${VERSION_NEW}|" synfig-studio/configure.ac
sed -i "s|synfig >= ${VERSION_CURRENT}|synfig >= ${VERSION_NEW}|" synfig-studio/configure.ac


sed -i "s|STUDIO_VERSION \"${VERSION_CURRENT}\"|STUDIO_VERSION \"${VERSION_NEW}\"|" synfig-studio/src/gui/CMakeLists.txt

sed -i "s|version=\"${VERSION_CURRENT}\"|version=\"${VERSION_NEW}\"|" synfig-studio/org.synfig.SynfigStudio.appdata.xml.in
DATE=`date +%Y-%m-%d`
sed -i "s|date=\".*\">|date=\"${DATE}\">|" synfig-studio/org.synfig.SynfigStudio.appdata.xml.in

git add \
	ETL/configure.ac \
	synfig-core/configure.ac \
	synfig-studio/configure.ac \
	synfig-studio/src/gui/CMakeLists.txt \
	synfig-studio/org.synfig.SynfigStudio.appdata.xml.in \
	# end

git commit -m "Release version ${VERSION_NEW}"
