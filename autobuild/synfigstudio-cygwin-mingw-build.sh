#!/bin/bash
#
# SynfigStudio Windows package build script
# Copyright (c) 2013 Konstantin Dmitriev
#
# This package is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This package is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
#
# = Usage notes =
# 
# * You need to have Cygwin and NSIS installed
# * Tweak the basic variables (below) before first run
# * Script should be started from the Cygwin terminal, launched as administrator
# * Builds from current repository, current revision. So you should manually checkout the desired revision to build
# * (TODO) If no repository found - then  sources fetched into C:\synfig-build\synfig and the latest master branch is built
# * Executing script without arguments makes a full clean build and produces installer package
# * You can pass arguments to the script to invoke particular stage. 
#	Available stages: mkprep, mketl, mksynfig, mksynfigstudio, mkpackage
#	Example: 
#		synfigstudio-cygwin-mingw-build.sh mkdeps
# * You can pass a custom command to be invoked in the build environment.
#	Example (uninstalls glibmm package from the buildroot):
#		synfigstudio-cygwin-mingw-build.sh make -j2


#TODO: Submit extended portfiles
#TODO: 64bit build
#TODO: Magick++
#TODO: Allow to install without locales


#================= EDIT THOSE VARIABLES BEFORE FIRST RUN! ======================

export CYGWIN_SETUP=/cygdrive/d/synfig-win/cygwin-setup.exe
export NSIS_BINARY="/cygdrive/c/Program Files (x86)/NSIS/makensis.exe"
export SRCPREFIX=/cygdrive/c/synfig-build/
export DEBUG=1

#=========================== EDIT UNTIL HERE ===================================

export DISTPREFIX=$SRCPREFIX/dist

export MINGWPREFIX=/usr/i686-pc-mingw32/sys-root/mingw/

export CHOST=i686-pc-mingw32
export CTARGET=i686-pc-mingw32
export CC=i686-pc-mingw32-gcc
export CXX=i686-pc-mingw32-g++
export F77=i686-pc-mingw32-gfortran
export FC=i686-pc-mingw32-gfortran
export GCJ=i686-pc-mingw32-gcj
export GOC=i686-pc-mingw32-gccgo
export OBJC=i686-pc-mingw32-gcc
export OBJCXX=i686-pc-mingw32-g++
export AR=i686-pc-mingw32-ar
export OBJDUMP=i686-pc-mingw32-objdump
export RANLIB=i686-pc-mingw32-ranlib
export STRIP=i686-pc-mingw32-strip
export RC=i686-pc-mingw32-windres
export CFLAGS=' -O2 -pipe -mms-bitfields'
export CXXFLAGS=' -O2 -pipe -mms-bitfields'
export F77FLAGS=' -mms-bitfields'
export FCFLAGS=' -O2 -pipe -mms-bitfields'
export GCJFLAGS=' -O2 -pipe -mms-bitfields'
export GOCFLAGS=' -mms-bitfields'
export OBJCFLAGS=' -O2 -pipe -mms-bitfields'
export OBJCXXFLAGS=' -O2 -pipe -mms-bitfields'
export PKG_CONFIG=/usr/bin/pkg-config
export PKG_CONFIG_LIBDIR=/usr/i686-pc-mingw32/sys-root/mingw/lib/pkgconfig:/usr/i686-pc-mingw32/sys-root/mingw/share/pkgconfig:/usr/share/pkgconfig
export PKG_CONFIG_SYSTEM_INCLUDE_PATH=/usr/i686-pc-mingw32/sys-root/mingw/include
export PKG_CONFIG_SYSTEM_LIBRARY_PATH=/usr/i686-pc-mingw32/sys-root/mingw/lib
export CPPFLAGS=" -I/usr/i686-pc-mingw32/sys-root/mingw/include "
export LDFLAGS=" -L/usr/i686-pc-mingw32/sys-root/mingw/lib "
export PATH="/usr/i686-pc-mingw32/sys-root/mingw/bin/:$PATH"
alias convert="/usr/bin/convert"

set -e

export SCRIPTPATH=`dirname "$0"`
if [[ $DEBUG == 1 ]]; then
	DEBUG='--enable-debug --enable-optimization=0'
else
	DEBUG=''
fi

# Install dependencies
mkprep()
{

export PREP_VERSION=1

if [[ `cat $SRCPREFIX/prep-done` != "${PREP_VERSION}" ]]; then

$CYGWIN_SETUP \
-K http://cygwinports.org/ports.gpg -s http://mirrors.kernel.org/sources.redhat.com/cygwinports -s http://ftp.linux.kiev.ua/pub/cygwin/ \
-P git \
-P make \
-P gdb \
-P intltool \
-P autoconf \
-P automake \
-P libtool \
-P pkg-config \
-P p7zip \
-P ImageMagick \
-P cygport \
-P mm-common \
-P mingw-gcc-g++  \
-P mingw-cairo \
-P mingw-glibmm2.4 \
-P mingw-pango1.0 \
-P mingw-gtkmm2.4 \
-q

#objdump -p hello.exe | grep "DLL Name"

#TODO: magick++

# libxml++
[ ! -d $SRCPREFIX/mingw-libxmlpp2.6 ] || rm -rf $SRCPREFIX/mingw-libxmlpp2.6
cp -rf $SRCPREFIX/synfig/autobuild/mingw-libxmlpp2.6 $SRCPREFIX/mingw-libxmlpp2.6
cd $SRCPREFIX/mingw-libxmlpp2.6
for action in fetch prep compile install package; do
    cygport mingw-libxml++2.6.cygport $action
done
tar -C / -jxf mingw-libxml++2.6-2.36.0-1.tar.bz2
tar -C / -jxf mingw-libxml++2.6-debuginfo-2.36.0-1.tar.bz2
cd ..
rm -rf $SRCPREFIX/mingw-libxmlpp2.6

# boost
[ ! -d $SRCPREFIX/mingw-boost ] || rm -rf $SRCPREFIX/mingw-boost
cp -rf $SRCPREFIX/synfig/autobuild/mingw-boost $SRCPREFIX/mingw-boost
cd $SRCPREFIX/mingw-boost
for action in fetch prep compile install package; do
    cygport mingw-boost.cygport $action
done
tar -C / -jxf mingw-boost-1.50.0-1.tar.bz2
cd ..
rm -rf $SRCPREFIX/mingw-boost

# there should be no *.la files
rm -rf /usr/i686-pc-mingw32/sys-root/mingw/lib/*.la || true

echo ${PREP_VERSION} > $SRCPREFIX/prep-done

fi
}

mketl()
{
cd $SRCPREFIX/synfig/ETL
autoreconf --install --force
./configure \
--prefix=/usr/i686-pc-mingw32/sys-root/mingw \
--exec-prefix=/usr/i686-pc-mingw32/sys-root/mingw \
--bindir=/usr/i686-pc-mingw32/sys-root/mingw/bin \
--sbindir=/usr/i686-pc-mingw32/sys-root/mingw/sbin \
--libexecdir=/usr/i686-pc-mingw32/sys-root/mingw/lib \
--datadir=/usr/i686-pc-mingw32/sys-root/mingw/share \
--localstatedir=/usr/i686-pc-mingw32/sys-root/mingw/var \
--sysconfdir=/usr/i686-pc-mingw32/sys-root/mingw/etc \
--datarootdir=/usr/i686-pc-mingw32/sys-root/mingw/share \
--docdir=/usr/share/doc/mingw-synfig -C \
--build=i686-pc-cygwin --host=i686-pc-mingw32 \
--enable-shared --disable-static \
--with-libiconv-prefix=no --with-libintl-prefix=no \
--enable-maintainer-mode $DEBUG
make install
}

mksynfig()
{
cd $SRCPREFIX/synfig/synfig-core
libtoolize --copy --force
autoreconf --install --force
./configure \
--prefix=/usr/i686-pc-mingw32/sys-root/mingw \
--exec-prefix=/usr/i686-pc-mingw32/sys-root/mingw \
--bindir=/usr/i686-pc-mingw32/sys-root/mingw/bin \
--sbindir=/usr/i686-pc-mingw32/sys-root/mingw/sbin \
--libexecdir=/usr/i686-pc-mingw32/sys-root/mingw/lib \
--datadir=/usr/i686-pc-mingw32/sys-root/mingw/share \
--localstatedir=/usr/i686-pc-mingw32/sys-root/mingw/var \
--sysconfdir=/usr/i686-pc-mingw32/sys-root/mingw/etc \
--datarootdir=/usr/i686-pc-mingw32/sys-root/mingw/share \
--docdir=/usr/share/doc/mingw-synfig -C \
--build=i686-pc-cygwin --host=i686-pc-mingw32 \
--enable-shared --disable-static \
--with-libiconv-prefix=no --with-libintl-prefix=no \
--with-magickpp=yes --with-boost=/usr/i686-pc-mingw32/sys-root/mingw/ \
--enable-maintainer-mode $DEBUG
make -j2
make install
}

mksynfigstudio()
{
cd $SRCPREFIX/synfig/synfig-studio
./bootstrap.sh
./configure \
--prefix=/usr/i686-pc-mingw32/sys-root/mingw \
--exec-prefix=/usr/i686-pc-mingw32/sys-root/mingw \
--bindir=/usr/i686-pc-mingw32/sys-root/mingw/bin \
--sbindir=/usr/i686-pc-mingw32/sys-root/mingw/sbin \
--libexecdir=/usr/i686-pc-mingw32/sys-root/mingw/lib \
--datadir=/usr/i686-pc-mingw32/sys-root/mingw/share \
--localstatedir=/usr/i686-pc-mingw32/sys-root/mingw/var \
--sysconfdir=/usr/i686-pc-mingw32/sys-root/mingw/etc \
--datarootdir=/usr/i686-pc-mingw32/sys-root/mingw/share \
--docdir=/usr/share/doc/mingw-synfig -C \
--build=i686-pc-cygwin --host=i686-pc-mingw32 \
--enable-shared --disable-static \
--with-libiconv-prefix=no --with-libintl-prefix=no \
--enable-maintainer-mode $DEBUG
make -j2
make install 
cp -rf /usr/i686-pc-mingw32/sys-root/mingw/share/pixmaps/synfigstudio/*  /usr/i686-pc-mingw32/sys-root/mingw/share/pixmaps
rm -rf /usr/i686-pc-mingw32/sys-root/mingw/share/pixmaps/synfigstudio
mkdir -p $MINGWPREFIX/licenses
cp -rf COPYING $MINGWPREFIX/licenses/synfigstudio.txt
}

mkpackage()
{

[ ! -d $DISTPREFIX ] || rm -rf $DISTPREFIX
mkdir -p $DISTPREFIX
[ -d $DISTPREFIX/bin ] || mkdir -p $DISTPREFIX/bin
[ -d $DISTPREFIX/licenses ] || mkdir -p $DISTPREFIX/licenses
[ -d $DISTPREFIX/lib ] || mkdir -p $DISTPREFIX/lib
[ -d $DISTPREFIX/share ] || mkdir -p $DISTPREFIX/share

cd $SRCPREFIX

[ -e ffmpeg-latest-win32-static.7z ] || wget http://ffmpeg.zeranoe.com/builds/win32/static/ffmpeg-latest-win32-static.7z
[ ! -d ffmpeg ] || rm -rf ffmpeg
mkdir -p ffmpeg
cd ffmpeg
7z e ../ffmpeg-latest-win32-static.7z
cp ffmpeg.exe $DISTPREFIX/bin
cp *.txt $DISTPREFIX/licenses
cd ..
rm -rf ffmpeg

[ -e portable-python-3.2.5.1.zip ] || wget http://download.tuxfamily.org/synfig/packages/sources/portable-python-3.2.5.1.zip
[ ! -d python ] || rm -rf python
unzip portable-python-3.2.5.1.zip
[ ! -d $DISTPREFIX/python ] || rm -rf $DISTPREFIX/python
mv python $DISTPREFIX

cp -rf $SRCPREFIX/synfig/synfig-core/examples $DISTPREFIX/
cp -rf $SRCPREFIX/synfig/synfig-studio/COPYING $DISTPREFIX/licenses/synfigstudio.txt

#copy compiled files
#cp -rf $MINGWPREFIX/bin/*.exe $DISTPREFIX/bin/
#TODO: strip binaries?
#cp -rf $MINGWPREFIX/bin/*.dll $DISTPREFIX/bin/
[ -d ${PREFIX}/bin ] || mkdir -p ${PREFIX}/bin
for file in \
   iconv.dll \
   libatk-\*.dll \
   libatkmm-1.6-1.dll \
   libboost_program_options\*.dll \
   libbz2\*.dll \
   libcairo\*.dll \
   libexpat\*.dll \
   libffi\*.dll \
   libfontconfig\*.dll \
   libfreetype\*.dll \
   libgcc_s_\*.dll \
   libgdk\*.dll \
   libgettext\*.dll \
   libgio\*.dll \
   libglib\*.dll \
   libgmodule\*.dll \
   libgobject\*.dll \
   libgthread\*.dll \
   libgtk\*.dll \
   libharfbuzz\*.dll \
   libiconv\*.dll \
   libintl\*.dll \
   libjasper\*.dll \
   libjpeg\*.dll \
   liblzma\*.dll \
   libpango\*.dll \
   libpixman\*.dll \
   libpng\*.dll \
   libsigc\*.dll \
   libstdc++\*.dll \
   libsynfig\*.dll \
   libtiff\*.dll \
   libturbojpeg.dll \
   libxml2\*.dll \
   libxml++\*.dll \
   libz\*.dll \
   pthread\*.dll \
   zlib\*.dll \
   pango-querymodules.exe \
   synfig.exe \
   synfigstudio.exe \
# this extra line is required!
do
	cp $MINGWPREFIX/bin/$file $DISTPREFIX/bin || true
done
cp -rf $MINGWPREFIX/etc $DISTPREFIX
#cp -rf $MINGWPREFIX/lib/gdk-pixbuf-2.0 $DISTPREFIX/lib
cp -rf $MINGWPREFIX/lib/gtk-2.0 $DISTPREFIX/lib
#cp -rf $MINGWPREFIX/lib/pango $DISTPREFIX/lib
cp -rf $MINGWPREFIX/lib/synfig $DISTPREFIX/lib
cp -rf $MINGWPREFIX/share/locale $DISTPREFIX/share
cp -rf $MINGWPREFIX/share/pixmaps $DISTPREFIX/share
if [ -d $DISTPREFIX/share/pixmaps/synfigstudio ]; then
	cp -rf $DISTPREFIX/share/pixmaps/synfigstudio/*  $DISTPREFIX/share/pixmaps
	rm -rf $DISTPREFIX/share/pixmaps/synfigstudio
fi
cp -rf $MINGWPREFIX/share/synfig $DISTPREFIX/share
cp -rf $MINGWPREFIX/share/themes $DISTPREFIX/share

#cleanup


gen_list_nsh()
{
[ ! -e $2.nsh ] || rm $2.nsh
[ ! -e $2-uninst.nsh ] || rm $2-uninst.nsh
for line in `find $1 -print`; do
	directory=`dirname $line`
	line1=`echo $directory | sed "s|\./||g" | sed "s|/|\\\\\|g"`
	line2=`echo $line | sed "s|\./||g" | sed "s|/|\\\\\|g"`
	if [ -d $line ]; then
		echo "RMDir \"\$INSTDIR\\$line2\"" >> $2-uninst.nsh
	else
		echo "SetOutPath \"\$INSTDIR\\$line1\""  >> $2.nsh
		echo "File \"$line2\"" >> $2.nsh
		echo "Delete \"\$INSTDIR\\$line2\"" >> $2-uninst.nsh
	fi
done
# reverse order of uninstall commands
cp $2-uninst.nsh $2-uninst.nsh.tmp
tac $2-uninst.nsh.tmp > $2-uninst.nsh
rm $2-uninst.nsh.tmp
}

cd $DISTPREFIX

#generate file lists

gen_list_nsh bin bin
sed -i '/ffmpeg\.exe/d' bin.nsh		# exclude ffmpeg from he list of binaries - it will go into separate group
gen_list_nsh etc etc
gen_list_nsh examples examples
gen_list_nsh lib/gtk-2.0 lib-gtk
gen_list_nsh lib/synfig lib-synfig
gen_list_nsh licenses licenses
#gen_list_nsh python python # -- takes too long
gen_list_nsh share share


#make installer
cp -f $SRCPREFIX/synfig/autobuild/synfigstudio.nsi ./
"$NSIS_BINARY" -nocd -- synfigstudio.nsi

}

mkall()
{
	mkprep
	mketl
	mksynfig
	mksynfigstudio
	mkpackage
}

if [ -z $1 ]; then
	mkall
else
	echo "Executing custom user command..."
	$@
fi
