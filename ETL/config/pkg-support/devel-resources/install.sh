#!/bin/sh
# finish up the installation
# this script should be executed using the sudo command
# this file is copied to ETL-devel.post_install and ETL-devel.post_upgrade
# inside the .pkg bundle
echo "Running post-install script"
umask 022
sleep 1

RESOURCE_DIR=`dirname $0`

cd RESOURCE_DIR

echo "Creating ETL-config script"
sleep 1
[ -d /usr/local ] || mkdir /usr/local
[ -d /usr/local/bin ] || mkdir /usr/local/bin
[ -d /usr/local/include ] || mkdir /usr/local/include
[ -d /usr/local/lib ] || mkdir /usr/local/lib
[ -d /usr/local/sbin ] || mkdir /usr/local/sbin

# If we already have a directory called ETL in our include directory, nuke it
[ -d /usr/local/include/ETL ] && rm -fr /usr/local/include/ETL

ln -s /usr/local/include/ETL /Library/Frameworks/ETL.framework/Headers
sed '
s:@exec_prefix@:/usr/local:g;
s:@prefix@:/usr/local:g;
s:@bindir@:$exec_prefix/bin:g;
s:@libdir@:$exec_prefix/lib:g;
s:@includedir@:$prefix/include:g;
s:@VERSION@:@_VERSION_@:g;
s:@PACKAGE@:@_PACKAGE_@:g;
s:@LIBS@::g;
' < $RESOURCE_DIR/ETL-config.in > /usr/local/bin/ETL-config
chmod 775 /usr/local/bin/ETL-config

echo "Done with shell script"
sleep 1

exit 0


#echo "Precompiling Headers"
#/usr/bin/cc -precomp ~/Library/Frameworks/SDL.framework/Headers/SDL.h -o ~/Library/Frameworks/SDL.framework/Headers/SDL.p

# open up the README file
#open ~/"Readme SDL Developer.txt"
