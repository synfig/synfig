#!/bin/sh
#
# Script to generate API documentation and send it to sf.net
#
# Copyright 2009-2010 Konstantin Dmitriev
# Copyright 2010, 2013 Carlos López
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This script has been redesigned to be used in a cron work.
# Adapt the following macros to the proper directories values.

export HTMLDIR=$HOME/synfig/synfig-repository/api
#export SOURCEDIR=$HOME/synfig/synfig-repository/code/synfig
export SOURCEDIR=$HOME/synfig/synfig-repository/api/tmp/synfig

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
	git clone --depth 1 git://github.com/synfig/synfig.git `basename $SOURCEDIR`
fi

mkdir -p $HTMLDIR

cd $SOURCEDIR
git fetch
git reset --hard
git checkout remotes/origin/master

#generating api to htmldir
for module in ETL synfig-core synfig-studio; do
cd $module
echo "Generating API for $module..."
case $module in
	ETL)
		MODULETITLE='Extended Template Library'
		;;
	synfig-core)
		MODULETITLE='Synfig Core'
		;;
	synfig-studio)
		MODULETITLE='Synfig Studio'
		;;
esac
	VERSION=`cat configure.ac |egrep "AC_INIT\(\[$MODULETITLE\],"| sed "s|.*$MODULETITLE\],\[||" | sed "s|\],\[.*||"`
	VERSION=${VERSION#*\'}
	VERSION=${VERSION%\'}
cp -f doxygen.cfg.in doxygen.cfg
sed -i "s/@VERSION@/$VERSION/" doxygen.cfg
sed -i "s/@PACKAGE@/$module/" doxygen.cfg
doxygen doxygen.cfg
rm -rf $HTMLDIR/$module
mv doc/html $HTMLDIR/$module
cp $SOURCEDIR/$module/doxygen.cfg $HTMLDIR/$module
cd ..
done

#index.html
DATE=`date -R`
cat > $HTMLDIR/index.html <<EOF
<html><head><title>ETL, synfig, synfigstudio API docs</title></head>
<body>
<h1>ETL, synfig, synfigstudio API docs</h1>
<ul>
<li><a href="ETL/annotated.html">ETL</a></li>
<li><a href="synfig-core/annotated.html">synfig-core</a></li>
<li><a href="synfig-studio/annotated.html">synfig-studio</a></li>
</ul>
<p>Generated on: $DATE.</p>
</body></html>
EOF

