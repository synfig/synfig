#!/bin/sh
# finish up the installation
# this script should be executed using the sudo command
# this file is copied to sinfg-devel.post_install and sinfg-devel.post_upgrade
# inside the .pkg bundle
LOGFILE="/sinfg_install.log"


umask 022

RESOURCE_DIR=`dirname $0`
PREFIX=/usr/local

cd $RESOURCE_DIR

echo "Creating sinfg-config script"
[ -d $PREFIX ] || mkdir $PREFIX
[ -d $PREFIX/bin ] || mkdir $PREFIX/bin
[ -d $PREFIX/include ] || mkdir $PREFIX/include
[ -d $PREFIX/lib ] || mkdir $PREFIX/lib
[ -d $PREFIX/sbin ] || mkdir $PREFIX/sbin

echo "Cleaning up any previous installation"
[ -d $PREFIX/include/sinfg ] && rm -fr $PREFIX/include/sinfg
ln -s /Library/Frameworks/sinfg.framework/Headers $PREFIX/include/sinfg

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
s:@CONFIG_LIBS@:-F/Library/Frameworks/sinfg.framework:;
s:@SINFG_LIBS@:-F/Library/Frameworks/sinfg.framework:;
s:@CONFIG_CFLAGS@:-framework sinfg:;
' < $RESOURCE_DIR/sinfg-config.in > $PREFIX/bin/sinfg-config
chmod 775 $PREFIX/bin/sinfg-config

echo "Precompiling Headers"
#/usr/bin/c++ -precomp /Library/Frameworks/sinfg.framework/Headers/sinfg.h -o /Library/Frameworks/sinfg.framework/Headers/sinfg.p

echo "Moving sinfg tool"
cp $RESOURCE_DIR/sinfg $PREFIX/bin || exit 1

echo "Done with shell script"


exit 0



