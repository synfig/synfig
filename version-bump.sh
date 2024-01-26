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

VERSION_NEW_MAJOR=$(echo "$VERSION_NEW" | cut -d'.' -f1)
VERSION_NEW_MINOR=$(echo "$VERSION_NEW" | cut -d'.' -f2)
VERSION_NEW_PATCH=$(echo "$VERSION_NEW" | cut -d'.' -f3)
VERSION_CURRENT_MAJOR=$(echo "$VERSION_CURRENT" | cut -d'.' -f1)
VERSION_CURRENT_MINOR=$(echo "$VERSION_CURRENT" | cut -d'.' -f2)
VERSION_CURRENT_PATCH=$(echo "$VERSION_CURRENT" | cut -d'.' -f3)
sed -i "s|ETL_VERSION \"${VERSION_CURRENT}\"|ETL_VERSION \"${VERSION_NEW}\"|" CMakeLists.txt
sed -i "s|STUDIO_VERSION \"${VERSION_CURRENT}\"|STUDIO_VERSION \"${VERSION_NEW}\"|" synfig-studio/src/gui/CMakeLists.txt
sed -i "s|STUDIO_VERSION_MAJOR ${VERSION_CURRENT_MAJOR}|STUDIO_VERSION_MAJOR ${VERSION_NEW_MAJOR}|" CMakeLists.txt
sed -i "s|STUDIO_VERSION_MINOR ${VERSION_CURRENT_MINOR}|STUDIO_VERSION_MINOR ${VERSION_NEW_MINOR}|" CMakeLists.txt
sed -i "s|STUDIO_VERSION_PATCH ${VERSION_CURRENT_PATCH}|STUDIO_VERSION_PATCH ${VERSION_NEW_PATCH}|" CMakeLists.txt

sed -i "s|version=\"${VERSION_CURRENT}\"|version=\"${VERSION_NEW}\"|" synfig-studio/org.synfig.SynfigStudio.appdata.xml.in
DATE=`date +%Y-%m-%d`
sed -i "s|date=\".*\">|date=\"${DATE}\">|" synfig-studio/org.synfig.SynfigStudio.appdata.xml.in

git add \
	ETL/configure.ac \
	synfig-core/configure.ac \
	synfig-studio/configure.ac \
	synfig-studio/src/gui/CMakeLists.txt \
	synfig-studio/org.synfig.SynfigStudio.appdata.xml.in \
	CMakeLists.txt \
	synfig-studio/src/gui/CMakeLists.txt \
	# end

git commit -m "Bump version to ${VERSION_NEW}"
