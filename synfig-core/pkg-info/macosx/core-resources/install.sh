#!/bin/sh
# finish up the installation
# this script should be executed using the sudo command
# this file is copied to synfig-devel.post_install and synfig-devel.post_upgrade
# inside the .pkg bundle
LOGFILE="/synfig_install.log"

exit 0


umask 022

RESOURCE_DIR=`dirname $0`
PREFIX=/usr/local

cd $RESOURCE_DIR

echo "Creating installation directory tree"
[ -d $PREFIX ] || mkdir $PREFIX
[ -d $PREFIX/bin ] || mkdir $PREFIX/bin
[ -d $PREFIX/include ] || mkdir $PREFIX/include
[ -d $PREFIX/lib ] || mkdir $PREFIX/lib
[ -d $PREFIX/sbin ] || mkdir $PREFIX/sbin

echo "Cleaning up any previous installation"
[ -d $PREFIX/include/synfig ] && rm -fr $PREFIX/include/synfig

ln -s /Library/Frameworks/synfig.framework/Headers $PREFIX/include/synfig

echo "Precompiling Headers"
#/usr/bin/c++ -precomp /Library/Frameworks/synfig.framework/Headers/synfig.h -o /Library/Frameworks/synfig.framework/Headers/synfig.p

echo "Moving synfig tool"
cp $RESOURCE_DIR/synfig $PREFIX/bin || exit 1

echo "Done with shell script"


exit 0

