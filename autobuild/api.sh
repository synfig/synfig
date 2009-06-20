#!/bin/sh
#
# Script to generate API documentation and send it to sf.net
#
# Copyright 2009 Konstantin Dmitriev
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.

export HTMLDIR=$HOME/synfig/api/html
export SOURCEDIR=$HOME/synfig/api/source

set -e

#check for git and doxygen
if ! which git > /dev/null 2>&1; then
	echo "Please install git."
	exit
fi
if ! which doxygen > /dev/null 2>&1; then
	echo "Please install doxygen."
	exit
fi

#fetching sources
if [ ! -d $SOURCEDIR ]; then
	mkdir -p `dirname $SOURCEDIR`
	cd `dirname $SOURCEDIR`
	git clone git://synfig.git.sourceforge.net/gitroot/synfig `basename $SOURCEDIR`
fi

getversion(){
	VERSION=`cat configure| egrep PACKAGE_VERSION=\'`
	VERSION=${VERSION#*\'}
	VERSION=${VERSION%\'}
}

mkdir -p $HTMLDIR

cd $SOURCEDIR
git fetch
git checkout remotes/origin/master

#generating api to htmldir
for module in ETL synfig-core synfig-studio; do
cd $module/trunk
echo "Generating API for $module..."
autoreconf --install --force || ( sed -i 's/^AC_CONFIG_SUBDIRS/# AC_CONFIG_SUBDIRS/' configure.ac && autoreconf --install --force )
getversion
cp -f doxygen.cfg.in doxygen.cfg
sed -i "s/@VERSION@/$VERSION/" doxygen.cfg
sed -i "s/@PACKAGE@/$module/" doxygen.cfg
doxygen doxygen.cfg
rm -rf $HTMLDIR/$module
mv doc/html $HTMLDIR/$module
cp $SOURCEDIR/$module/trunk/doxygen.cfg $HTMLDIR/$module
cd ../..
done

#index.html
DATE=`date -R`
cat > $HTMLDIR/index.html <<EOF
<html><head><title>ETL, synfig, synfigstudio API docs</title></head>
<body>
<h1>ETL, synfig, synfigstudio API docs</h1>
<ul>
<li><a href="ETL/">ETL</a></li>
<li><a href="synfig/">synfig</a></li>
<li><a href="synfigstudio/">synfigstudio</a></li>
</ul>
<p>Generated on: $DATE.</p>
</body></html>
EOF

#beep (because we asking password)
echo -e "\a"; sleep 0.2; echo -e "\a"; sleep 0.2; echo -e "\a"

echo -n "Enter your sf.net username: "
read USERNAME

#push to sf.net
rsync -avP -e ssh $HTMLDIR/ $USERNAME,synfig@web.sourceforge.net:htdocs/api/

