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

echo "Creating synfig-config script"
[ -d $PREFIX ] || mkdir $PREFIX
[ -d $PREFIX/bin ] || mkdir $PREFIX/bin
[ -d $PREFIX/include ] || mkdir $PREFIX/include
[ -d $PREFIX/lib ] || mkdir $PREFIX/lib
[ -d $PREFIX/sbin ] || mkdir $PREFIX/sbin

echo "Cleaning up any previous installation"
[ -d $PREFIX/include/synfig ] && rm -fr $PREFIX/include/synfig
ln -s /Library/Frameworks/synfig.framework/Headers $PREFIX/include/synfig

sed '
s:@exec_prefix@:/usr/local:g;
s:@prefix@:/usr/local:g;
s:@bindir@:$exec_prefix/bin:g;
s:@libdir@:$exec_prefix/lib:g;
s:@includedir@:$prefix/include:g;
s:@VERSION@:@_VERSION_@:g;
s:@PACKAGE@:@_PACKAGE_@:g;
s:@LIBS@::g;
s:@VERSION@:@_VERSION_@:;
s:@PACKAGE@:@_PACKAGE_@:;
s:@CONFIG_LIBS@:-F/Library/Frameworks/synfig.framework:;
s:@SYNFIG_LIBS@:-F/Library/Frameworks/synfig.framework:;
s:@CONFIG_CFLAGS@:-framework synfig:;
' < $RESOURCE_DIR/synfig-config.in > $PREFIX/bin/synfig-config
chmod 775 $PREFIX/bin/synfig-config

echo "Precompiling Headers"
#/usr/bin/c++ -precomp /Library/Frameworks/synfig.framework/Headers/synfig.h -o /Library/Frameworks/synfig.framework/Headers/synfig.p

echo "Moving synfig tool"
cp $RESOURCE_DIR/synfig $PREFIX/bin || exit 1

echo "Done with shell script"


exit 0



