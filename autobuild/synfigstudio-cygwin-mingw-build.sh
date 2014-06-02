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
# * Download and install Git (http://msysgit.github.io/).
# * Open Git Bash and execute following commands:
# ** mkdir C:\synfig-build
# ** cd C:\synfig-build
# ** git clone https://github.com/synfig/synfig.git
# ** mkdir cygwin-dist
# ** cd synfig
# ** git config --global core.autocrlf input
# ** exit
# * Download Cygwin setup binary (http://www.cygwin.com/) and save it into C:\synfig-build\cygwin-dist\ directory.
# * Run Cygwin setup and install with the default parameters.
# * Download and install NSIS >=3.0 (http://nsis.sourceforge.net/). Install into C:\synfig-build\NSIS\ directory.
# * Open Cygwin console (with administrator previlegies) and run the build script:
# ** bash /cygdrive/c/synfig-build/synfig/autobuild/synfigstudio-cygwin-mingw-build.sh
# * Installation bundle will be written to C:\synfig-build\
#
#
# = Other notes =
# * Builds from current repository, current revision. So you should manually checkout the desired revision to build
# * Executing script without arguments makes a full clean build and produces installer package
# * You can pass arguments to the script to invoke particular stage. 
#	Available stages: mkprep, mketl, mksynfig, mksynfigstudio, mkpackage
#	Example: 
#		synfigstudio-cygwin-mingw-build.sh mkpackage
# * You can pass a custom command to be invoked in the build environment.
#	Example (executes make with respect to the build environment):
#		synfigstudio-cygwin-mingw-build.sh make -j2



#================= EDIT THOSE VARIABLES BEFORE FIRST RUN! ======================

export NSIS_BINARY="/cygdrive/c/synfig-build/NSIS/makensis.exe"
export WORKSPACE="/cygdrive/c/synfig-build/"
if [ -z $ARCH ]; then
    export ARCH="32"
fi
if [ -z $DEBUG ]; then
	export DEBUG=1
fi
if [ -z $THREADS ]; then
	export THREADS=4
fi
#=========================== EDIT UNTIL HERE ===================================

export DISTPREFIX=$WORKSPACE/dist
export SRCPREFIX=`dirname "$0"`
SRCPREFIX=$(cd "$SRCPREFIX/.."; pwd)

if [[ $ARCH == "32" ]]; then
    export TOOLCHAIN_HOST="i686-w64-mingw32"
    export TOOLCHAIN="mingw64-i686" # mingw64-i686 | mingw64-x86_64 | mingw
    export EXT_ARCH=i386
    export CYGWIN_SETUP="/cygdrive/c/synfig-build/cygwin-dist/setup-x86.exe"
elif [[ $ARCH == "64" ]]; then
    export TOOLCHAIN_HOST="x86_64-w64-mingw32"
    export TOOLCHAIN="mingw64-x86_64"
    export EXT_ARCH=x86_64
    export CYGWIN_SETUP="/cygdrive/c/synfig-build/cygwin64-dist/setup-x86_64.exe"
fi

export MINGWPREFIX=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/



set -e

if [[ $DEBUG == 1 ]]; then
	DEBUG='--enable-debug --enable-optimization=0'
else
	DEBUG=''
fi

export VERSION=`cat ${SRCPREFIX}/synfig-core/configure.ac |egrep "AC_INIT\(\[Synfig Core\],"| sed "s|.*Core\],\[||" | sed "s|\],\[.*||"`
pushd "${SRCPREFIX}" > /dev/null
export REVISION=`git show --pretty=format:%ci HEAD |  head -c 10 | tr -d '-'`
popd > /dev/null

prepare_mingw_env()
{
export CBUILD=i686-pc-cygwin
export CHOST=${TOOLCHAIN_HOST}
export CTARGET=${TOOLCHAIN_HOST}
export CC=${TOOLCHAIN_HOST}-gcc
export CXX=${TOOLCHAIN_HOST}-g++
export F77=${TOOLCHAIN_HOST}-gfortran
export FC=${TOOLCHAIN_HOST}-gfortran
export GCJ=${TOOLCHAIN_HOST}-gcj
export GOC=${TOOLCHAIN_HOST}-gccgo
export OBJC=${TOOLCHAIN_HOST}-gcc
export OBJCXX=${TOOLCHAIN_HOST}-g++
export AR=${TOOLCHAIN_HOST}-ar
export OBJDUMP=${TOOLCHAIN_HOST}-objdump
export RANLIB=${TOOLCHAIN_HOST}-ranlib
export STRIP=${TOOLCHAIN_HOST}-strip
export RC=${TOOLCHAIN_HOST}-windres
export CFLAGS=' -O2 -pipe -mms-bitfields'
export CXXFLAGS=' -O2 -pipe -mms-bitfields'
export F77FLAGS=' -mms-bitfields'
export FCFLAGS=' -O2 -pipe -mms-bitfields'
export GCJFLAGS=' -O2 -pipe -mms-bitfields'
export GOCFLAGS=' -mms-bitfields'
export OBJCFLAGS=' -O2 -pipe -mms-bitfields'
export OBJCXXFLAGS=' -O2 -pipe -mms-bitfields'
export PKG_CONFIG=/usr/bin/pkg-config
export PKG_CONFIG_PATH="/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib/pkgconfig"
export PKG_CONFIG_LIBDIR="/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib/pkgconfig:/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share/pkgconfig:/usr/share/pkgconfig"
export PKG_CONFIG_SYSTEM_INCLUDE_PATH=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/include
export PKG_CONFIG_SYSTEM_LIBRARY_PATH=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib
export CPPFLAGS=" -I/usr/${TOOLCHAIN_HOST}/sys-root/mingw/include "
export LDFLAGS=" -L/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib "
export PATH="/usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin/:$PATH"
alias convert="/usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin/convert"
}

mknative()
{
export CBUILD=""
export CHOST=""
export CTARGET=""
export CC="gcc"
export CXX="g++"
export F77=""
export FC=""
export GCJ=""
export GOC=""
export OBJC=""
export OBJCXX=""
export AR=""
export OBJDUMP=""
export RANLIB=""
export STRIP=""
export RC=""
export CFLAGS=""
export CXXFLAGS=""
export F77FLAGS=""
export FCFLAGS=""
export GCJFLAGS=""
export GOCFLAGS=""
export OBJCFLAGS=""
export OBJCXXFLAGS=""
export PKG_CONFIG_PATH="/usr/lib/pkgconfig:/usr/local/lib/pkgconfig"
export PKG_CONFIG_LIBDIR=""
export PKG_CONFIG_SYSTEM_INCLUDE_PATH=""
export PKG_CONFIG_SYSTEM_LIBRARY_PATH=""
export CPPFLAGS=""
export LDFLAGS=""
export PATH="/usr/local/bin:/usr/bin"

$@
}

mkpopt()
{
PKG_NAME=popt
PKG_VERSION=1.10.3

cd $WORKSPACE
[ -e ${PKG_NAME}-${PKG_VERSION}.tar.gz ] || wget http://rpm5.org/files/popt/${PKG_NAME}-${PKG_VERSION}.tar.gz
[ -d ${PKG_NAME}-${PKG_VERSION} ] || tar -xzf ${PKG_NAME}-${PKG_VERSION}.tar.gz
cd ${PKG_NAME}-${PKG_VERSION}
./autogen.sh --noconfigure
./configure --prefix=/usr/local --libdir=/usr/local/lib 
make -j$THREADS install
if [[ $ARCH == "64" ]]; then
	mv /usr/local/lib64/* /usr/local/lib
fi

# remove old version of popt
[ ! -e /usr/bin/cygpopt-0.dll ] || rm /usr/bin/cygpopt-0.dll
}

mkrpm()
{
echo
echo Building rpm...
echo
PKG_NAME=rpm
#PKG_VERSION=4.11.1
PKG_VERSION=4.10.3.1
#PKG_VERSION=4.7.0
TAREXT=bz2

cd $WORKSPACE
[ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://rpm.org/releases/rpm-4.10.x/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
    tar -xjf ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    cd ${PKG_NAME}-${PKG_VERSION}
    patch -p1 < $SRCPREFIX/autobuild/cygwin/${PKG_NAME}-${PKG_VERSION}.patch
    patch -p1 < $SRCPREFIX/autobuild/cygwin/${PKG_NAME}-${PKG_VERSION}-python-fixes.patch
else
    cd ${PKG_NAME}-${PKG_VERSION}
fi
LDFLAGS=" -L/usr/local/lib" CPPFLAGS="-I/usr/include/nspr -I/usr/include/nss -I/usr/include/db4.8/ -I/usr/include/python2.7/" ./autogen.sh \
    --with-external-db \
    --without-lua \
    --enable-python
make -j$THREADS install

cd python
export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
export LDFLAGS=" -L/usr/local/lib" 
python setup.py build
python setup.py install
}

mkpycurl()
{
PKG_NAME=pycurl
PKG_VERSION=7.19.0
TAREXT=gz

cd $WORKSPACE
[ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://pycurl.sourceforge.net/download/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
    tar -xzf ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    cd ${PKG_NAME}-${PKG_VERSION}
else
    cd ${PKG_NAME}-${PKG_VERSION}
fi

python setup.py build
python setup.py install
}

mkurlgrabber()
{
PKG_NAME=urlgrabber
PKG_VERSION=3.9.1
TAREXT=gz

cd $WORKSPACE
[ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://urlgrabber.baseurl.org/download/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
    tar -xzf ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    cd ${PKG_NAME}-${PKG_VERSION}
else
    cd ${PKG_NAME}-${PKG_VERSION}
fi

python setup.py build
python setup.py install
}

mkyum-metadata-parser()
{
PKG_NAME=yum-metadata-parser
PKG_VERSION=1.1.4
TAREXT=gz

cd $WORKSPACE
[ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://yum.baseurl.org/download/yum-metadata-parser/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
    tar -xzf ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    cd ${PKG_NAME}-${PKG_VERSION}
else
    cd ${PKG_NAME}-${PKG_VERSION}
fi

python setup.py build
python setup.py install
}

mkyum()
{
PKG_NAME=yum
PKG_VERSION=3.4.3
TAREXT=gz

cd $WORKSPACE
[ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://yum.baseurl.org/download/3.4/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
    tar -xzf ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    cd ${PKG_NAME}-${PKG_VERSION}
else
    cd ${PKG_NAME}-${PKG_VERSION}
fi
rm INSTALL || true
make install PREFIX="//" DESTDIR=""
}

mkyum-utils()
{
PKG_NAME=yum-utils
PKG_VERSION=1.1.31
TAREXT=gz

cd $WORKSPACE
[ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://yum.baseurl.org/download/yum-utils/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
    tar -xzf ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    cd ${PKG_NAME}-${PKG_VERSION}
else
    cd ${PKG_NAME}-${PKG_VERSION}
fi
make install || true
if [ ! -e /usr/bin/yumdownloader ]; then
    exit 1
fi
}

mkimagemagick()
{
PKG_NAME=ImageMagick
#PKG_VERSION=6.8.6-10
PKG_VERSION=6.8.7-10
TAREXT=bz2

if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION%-*}  --print-errors; then
    cd $WORKSPACE
    [ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://www.imagemagick.org/download/legacy/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
        tar -xjf ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
        cd ${PKG_NAME}-${PKG_VERSION}
    else
        cd ${PKG_NAME}-${PKG_VERSION}
    fi
    [ ! -e config.cache ] || rm config.cache
    autoreconf -i --verbose  # does this really required?
    ./configure \
        --prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
        --exec-prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
        --bindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin \
        --sbindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/sbin \
        --libexecdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib \
        --datadir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
        --localstatedir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/var \
        --sysconfdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/etc \
        --datarootdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
        --docdir=/usr/share/doc/mingw-synfig -C \
        --build=i686-pc-cygwin --host=${TOOLCHAIN_HOST} \
        --disable-static --enable-shared \
        --without-modules \
        --without-perl \
        --without-x \
        --with-threads \
        --with-magick_plus_plus

    make install
fi
}

mklibogg()
{

PKG_NAME=libogg
PKG_VERSION=1.3.1
TAREXT=gz

if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION}  --print-errors; then
    cd $WORKSPACE
    [ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://downloads.xiph.org/releases/ogg/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
        tar -xzf ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    fi
    cd ${PKG_NAME}-${PKG_VERSION}
    [ ! -e config.cache ] || rm config.cache
    ./configure \
        --prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
        --exec-prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
        --bindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin \
        --sbindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/sbin \
        --libexecdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib \
        --datadir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
        --localstatedir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/var \
        --sysconfdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/etc \
        --datarootdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
        --docdir=/usr/share/doc/mingw-synfig -C \
        --build=i686-pc-cygwin --host=${TOOLCHAIN_HOST}

    make all
    make install

fi
}

mklibvorbis()
{
mklibogg

PKG_NAME=libvorbis
PKG_VERSION=1.3.4
TAREXT=gz

if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION}  --print-errors; then
    cd $WORKSPACE
    [ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://downloads.xiph.org/releases/vorbis/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
        tar -xzf ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    fi
    cd ${PKG_NAME}-${PKG_VERSION}
    [ ! -e config.cache ] || rm config.cache
    ./configure \
        --prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
        --exec-prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
        --bindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin \
        --sbindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/sbin \
        --libexecdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib \
        --datadir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
        --localstatedir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/var \
        --sysconfdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/etc \
        --datarootdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
        --docdir=/usr/share/doc/mingw-synfig -C \
        --build=i686-pc-cygwin --host=${TOOLCHAIN_HOST}

    make all
    make install

fi
}

mklibsamplerate()
{
PKG_NAME=libsamplerate
PKG_VERSION=0.1.8
TAREXT=gz

if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION}  --print-errors; then
    cd $WORKSPACE
    [ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://www.mega-nerd.com/SRC/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
        tar -xzf ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    fi
    cd ${PKG_NAME}-${PKG_VERSION}
    [ ! -e config.cache ] || rm config.cache
    ./configure \
        --prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
        --exec-prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
        --bindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin \
        --sbindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/sbin \
        --libexecdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib \
        --datadir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
        --localstatedir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/var \
        --sysconfdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/etc \
        --datarootdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
        --docdir=/usr/share/doc/mingw-synfig -C \
        --build=i686-pc-cygwin --host=${TOOLCHAIN_HOST}

    make all
    make install

fi
}

mksox()
{
PKG_NAME=sox
PKG_VERSION=14.4.1
TAREXT=gz

if ! pkg-config ${PKG_NAME} --exact-version=${PKG_VERSION}  --print-errors; then
    cd $WORKSPACE
    [ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://download.tuxfamily.org/synfig/packages/sources/base/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
        tar -xzf ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    fi
    cd ${PKG_NAME}-${PKG_VERSION}
    [ ! -e config.cache ] || rm config.cache
    ./configure \
        --prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
        --exec-prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
        --bindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin \
        --sbindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/sbin \
        --libexecdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib \
        --datadir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
        --localstatedir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/var \
        --sysconfdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/etc \
        --datarootdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
        --docdir=/usr/share/doc/mingw-synfig -C \
        --build=i686-pc-cygwin --host=${TOOLCHAIN_HOST}

    make all
    make install

fi
}

mkmlt()
{
PKG_NAME=mlt
PKG_VERSION=0.9.0
TAREXT=gz

if ! pkg-config ${PKG_NAME}\+\+ --exact-version=${PKG_VERSION}  --print-errors; then
    cd $WORKSPACE
    #[ -e ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT} ] || wget http://download.tuxfamily.org/synfig/packages/sources/base/${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    #if [ ! -d ${PKG_NAME}-${PKG_VERSION} ]; then
    #    tar -xzf ${PKG_NAME}-${PKG_VERSION}.tar.${TAREXT}
    #fi
    #cd ${PKG_NAME}-${PKG_VERSION}
    if [ ! -d ${PKG_NAME} ]; then
        git clone https://github.com/morevnaproject/mlt
    fi
    cd mlt
    [ ! -e config.cache ] || rm config.cache
    #autoreconf -i --verbose  # does this really required?
    rm -rf /usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib/libmlt* || true
    rm -rf /usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin/libmlt* || true
    ./configure \
        --prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
        --exec-prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
        --bindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin \
        --sbindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/sbin \
        --libexecdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib \
        --datadir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
        --localstatedir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/var \
        --sysconfdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/etc \
        --datarootdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
        --docdir=/usr/share/doc/mingw-synfig -C \
        --build=i686-pc-cygwin --host=${TOOLCHAIN_HOST} \
        --avformat-shared=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/ \
        --enable-gpl --disable-decklink \
        --target-os=MinGW --target-arch=$EXT_ARCH \
        #$DEBUG

    make all
    make install

    mv /usr/${TOOLCHAIN_HOST}/sys-root/mingw/melt.exe /usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin
    mv /usr/${TOOLCHAIN_HOST}/sys-root/mingw/libmlt*.dll /usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin

    mkdir -p /usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin/lib || true
    mkdir -p /usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin/share || true
    cp -rf /usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib/mlt /usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin/lib/
    cp -rf /usr/${TOOLCHAIN_HOST}/sys-root/mingw/share/mlt /usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin/share/

fi
}

mkffmpeg()
{
    export FFMPEG_VERSION=2.2.2
    if ! pkg-config libswscale --exact-version=${PKG_VERSION}  --print-errors; then
        pushd $WORKSPACE
        [ -e ffmpeg-${FFMPEG_VERSION}-win${ARCH}-dev.7z ] || wget http://ffmpeg.zeranoe.com/builds/win${ARCH}/dev/ffmpeg-${FFMPEG_VERSION}-win${ARCH}-dev.7z
        [ -e ffmpeg-${FFMPEG_VERSION}-win${ARCH}-shared.7z ] || wget http://ffmpeg.zeranoe.com/builds/win${ARCH}/shared/ffmpeg-${FFMPEG_VERSION}-win${ARCH}-shared.7z
        [ ! -d ffmpeg ] || rm -rf ffmpeg
        mkdir -p ffmpeg
        cd ffmpeg
        7z x ../ffmpeg-${FFMPEG_VERSION}-win${ARCH}-dev.7z
        cp -rf ffmpeg-${FFMPEG_VERSION}-win${ARCH}-dev/include/* /usr/${TOOLCHAIN_HOST}/sys-root/mingw/include/
        cp -rf ffmpeg-${FFMPEG_VERSION}-win${ARCH}-dev/lib/* /usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib/
        7z x ../ffmpeg-${FFMPEG_VERSION}-win${ARCH}-shared.7z
        cp -rf ffmpeg-${FFMPEG_VERSION}-win${ARCH}-shared/bin/ffmpeg.exe /usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin
        cp -rf ffmpeg-${FFMPEG_VERSION}-win${ARCH}-shared/bin/*.dll /usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin
        mkdir -p /usr/${TOOLCHAIN_HOST}/sys-root/mingw/share/ffmpeg/presets/ || true
        cp -rf ffmpeg-${FFMPEG_VERSION}-win${ARCH}-shared/presets/* /usr/${TOOLCHAIN_HOST}/sys-root/mingw/share/ffmpeg/presets/

		for PKG in libswscale libavformat libavdevice; do
			cat > /usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib/pkgconfig/${PKG}.pc <<EOF
prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw
exec_prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw
libdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib
includedir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/include

Name: ${PKG}
Description: Dynamic module loader for GLib
Version: ${FFMPEG_VERSION}

EOF
		done
        popd
    fi
}

fedora-mingw-install()
{
[ -d $WORKSPACE/mingw-rpms ] || mkdir $WORKSPACE/mingw-rpms

cd $WORKSPACE/mingw-rpms

# Prepare custom yum.conf
cat > $WORKSPACE/mingw-rpms/yum.conf <<EOF
[main]
cachedir=${WORKSPACE}mingw-rpms/yum
keepcache=0
debuglevel=2
logfile=/var/log/yum.log
exactarch=1
obsoletes=1
plugins=1
installonly_limit=3

[fedora]
name=Fedora \$releasever - \$basearch
failovermethod=priority
#mirrorlist=http://mirrors.fedoraproject.org/metalink?repo=fedora-\$releasever&arch=\$basearch
baseurl=http://download.fedoraproject.org/pub/fedora/linux/releases/20/Everything/i386/os/
enabled=1
metadata_expire=7d

[updates]
name=Fedora \$releasever - \$basearch - Updates
failovermethod=priority
#mirrorlist=http://mirrors.fedoraproject.org/metalink?repo=updates-released-f\$releasever&arch=\$basearch
baseurl=http://download.fedoraproject.org/pub/fedora/linux/updates/20/i386/
enabled=1
EOF

URLS=`yumdownloader --urls --resolve -c $WORKSPACE/mingw-rpms/yum.conf --releasever=20 --installroot="$WORKSPACE/mingw-rpms" $1`
for URL in $URLS; do
if ( echo "$URL" | egrep "^http:" > /dev/null ); then
    PKG=`basename $URL`
    if ( echo "$PKG" | egrep "^mingw" > /dev/null ); then
        if ! ( echo $PKG | egrep "^mingw..-headers|^mingw..-gcc|^mingw-|^mingw..-filesystem|^mingw..-binutils|^mingw..-crt|^mingw..-cpp" > /dev/null); then
            echo $PKG
            wget -c "$URL"
            rpm -Uhv --ignoreos --nodeps --force "$PKG"
        fi
    fi
fi
done

# Ensure all dlls have executable flag, otherwise the compiled binaries won't run
chmod a+x /usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin/*.dll
}

# Install dependencies
mkprep()
{

export PREP_VERSION=5

if [[ `cat /prep-done` != "${PREP_VERSION}" ]]; then

$CYGWIN_SETUP \
-s http://www.mirrorservice.org/sites/sourceware.org/pub/cygwin/ \
-P git \
-P make \
-P gcc-core \
-P gcc-g++ \
-P gdb \
-P intltool \
-P autoconf \
-P automake \
-P libtool \
-P pkg-config \
-P p7zip \
-P ImageMagick \
-P cygport \
-P $TOOLCHAIN-gcc  \
-P $TOOLCHAIN-gcc-g++  \
-P zlib-devel \
-P libnspr-devel \
-P liblzma-devel \
-P libnss-devel \
-P libiconv \
-P libdb4.8-devel \
-P python \
-P file-devel \
-P zlib-devel \
-P libdb-devel \
-q

# yum dependencies
$CYGWIN_SETUP \
-s http://www.mirrorservice.org/sites/sourceware.org/pub/cygwin/ \
-P urlgrabber \
-P libglib2.0-devel \
-P libxml2-devel \
-P libsqlite3-devel \
-q

#-P libglib2.0-devel \ # yum req
#-P libsqlite3-devel \ # yum req
#-P libxml2-devel \ # yum req
#-P libcurl-devel \ # pycurl req

mknative mkpopt
mknative mkrpm
#mknative mkurlgrabber
mknative mkyum-metadata-parser


#mknative mkyum
#mknative mkyum-utils

cd $WORKSPACE
wget -c http://fedora.inode.at/fedora/linux/releases/18/Everything/i386/os/Packages/y/yum-3.4.3-47.fc18.noarch.rpm
rpm -Uhv --force --ignoreos --nodeps yum-3.4.3-47.fc18.noarch.rpm
wget -c http://fedora.inode.at/fedora/linux/releases/18/Everything/i386/os/Packages/y/yum-utils-1.1.31-6.fc18.noarch.rpm
rpm -Uhv --force --ignoreos --nodeps yum-utils-1.1.31-6.fc18.noarch.rpm

fedora-mingw-install mingw${ARCH}-libxml++
fedora-mingw-install mingw${ARCH}-cairo
fedora-mingw-install mingw${ARCH}-pango
fedora-mingw-install mingw${ARCH}-boost
fedora-mingw-install mingw${ARCH}-libjpeg-turbo
fedora-mingw-install mingw${ARCH}-gtkmm24
fedora-mingw-install mingw${ARCH}-dlfcn
fedora-mingw-install mingw${ARCH}-SDL
cp /usr/${TOOLCHAIN_HOST}/sys-root/mingw/include/SDL/* /usr/${TOOLCHAIN_HOST}/sys-root/mingw/include/

# Somehow this is required too...
fedora-mingw-install mingw${ARCH}-pcre

# Dependencies for magick++
fedora-mingw-install mingw${ARCH}-libltdl
fedora-mingw-install mingw${ARCH}-libtiff

echo ${PREP_VERSION} > /prep-done

fi
}

mketl()
{
cd $SRCPREFIX/ETL
autoreconf --install --force
./configure \
--prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
--exec-prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
--bindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin \
--sbindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/sbin \
--libexecdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib \
--datadir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
--localstatedir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/var \
--sysconfdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/etc \
--datarootdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
--docdir=/usr/share/doc/mingw-synfig -C \
--build=i686-pc-cygwin --host=${TOOLCHAIN_HOST} \
--enable-shared --disable-static \
--with-libiconv-prefix=no --with-libintl-prefix=no \
--enable-maintainer-mode $DEBUG
make install
}

mksynfig()
{
cd $SRCPREFIX/synfig-core
[ ! -e config.cache ] || rm config.cache
libtoolize --copy --force
autoreconf --install --force
./configure \
--prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
--exec-prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
--bindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin \
--sbindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/sbin \
--libexecdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib \
--datadir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
--localstatedir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/var \
--sysconfdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/etc \
--datarootdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
--docdir=/usr/share/doc/mingw-synfig -C \
--build=i686-pc-cygwin --host=${TOOLCHAIN_HOST} \
--enable-shared --disable-static \
--with-libiconv-prefix=no --with-libintl-prefix=no \
--with-magickpp=yes --with-boost=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/ \
--enable-maintainer-mode $DEBUG
make -j$THREADS
make install
}

mksynfigstudio()
{
cd $SRCPREFIX/synfig-studio
[ ! -e config.cache ] || rm config.cache
./bootstrap.sh
./configure \
--prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
--exec-prefix=/usr/${TOOLCHAIN_HOST}/sys-root/mingw \
--bindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/bin \
--sbindir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/sbin \
--libexecdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/lib \
--datadir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
--localstatedir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/var \
--sysconfdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/etc \
--datarootdir=/usr/${TOOLCHAIN_HOST}/sys-root/mingw/share \
--docdir=/usr/share/doc/mingw-synfig -C \
--build=i686-pc-cygwin --host=${TOOLCHAIN_HOST} \
--enable-shared --disable-static \
--with-libiconv-prefix=no --with-libintl-prefix=no \
--enable-maintainer-mode $DEBUG
make -j$THREADS
make install 
cp -rf /usr/${TOOLCHAIN_HOST}/sys-root/mingw/share/pixmaps/synfigstudio/*  /usr/${TOOLCHAIN_HOST}/sys-root/mingw/share/pixmaps
rm -rf /usr/${TOOLCHAIN_HOST}/sys-root/mingw/share/pixmaps/synfigstudio
mkdir -p $MINGWPREFIX/licenses
cp -rf COPYING $MINGWPREFIX/licenses/synfigstudio.txt

cat > ${MINGWPREFIX}/etc/gtk-2.0/gtkrc <<EOF

# Enable native look
gtk-theme-name = "MS-Windows"

# Use small toolbar buttons
gtk-toolbar-style = 0

EOF
}

mkpackage()
{

[ ! -d $DISTPREFIX ] || rm -rf $DISTPREFIX
mkdir -p $DISTPREFIX
[ -d $DISTPREFIX/bin ] || mkdir -p $DISTPREFIX/bin
[ -d $DISTPREFIX/licenses ] || mkdir -p $DISTPREFIX/licenses
[ -d $DISTPREFIX/lib ] || mkdir -p $DISTPREFIX/lib
[ -d $DISTPREFIX/share ] || mkdir -p $DISTPREFIX/share

cd $WORKSPACE

[ -e portable-python-3.2.5.1.zip ] || wget http://download.tuxfamily.org/synfig/packages/sources/portable-python-3.2.5.1.zip
[ ! -d python ] || rm -rf python
unzip portable-python-3.2.5.1.zip
[ ! -d $DISTPREFIX/python ] || rm -rf $DISTPREFIX/python
mv python $DISTPREFIX

cp -rf $SRCPREFIX/synfig-core/examples $DISTPREFIX/
cp -rf $SRCPREFIX/synfig-studio/COPYING $DISTPREFIX/licenses/synfigstudio.txt

#copy compiled files
#cp -rf $MINGWPREFIX/bin/*.exe $DISTPREFIX/bin/
#TODO: strip binaries?
#cp -rf $MINGWPREFIX/bin/*.dll $DISTPREFIX/bin/
[ -d ${PREFIX}/bin ] || mkdir -p ${PREFIX}/bin
for file in \
   av*.dll \
   ffmpeg.exe \
   iconv.dll \
   libatk-\*.dll \
   libatkmm-1.6-1.dll \
   libboost_program_options\*.dll \
   libbz2\*.dll \
   libcairo\*.dll \
   libdl.dll \
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
   libgomp*.dll \
   libgthread\*.dll \
   libgtk\*.dll \
   libharfbuzz\*.dll \
   libiconv\*.dll \
   libintl\*.dll \
   libjasper\*.dll \
   libjpeg\*.dll \
   libltdl*.dll \
   liblzma\*.dll \
   libMagick*.dll \
   libmlt*.dll \
   libogg*.dll \
   libpango\*.dll \
   libpixman\*.dll \
   libpng\*.dll \
   libsamplerate*.dll \
   libsigc\*.dll \
   libsox*.dll \
   libstdc++\*.dll \
   libsynfig\*.dll \
   libtiff\*.dll \
   libturbojpeg.dll \
   libvorbis*.dll \
   libwinpthread*.dll \
   libxml2\*.dll \
   libxml++\*.dll \
   libz\*.dll \
   postproc*.dll \
   pthread\*.dll \
   SDL.dll \
   swscale*.dll \
   swresample*.dll \
   zlib\*.dll \
   convert.exe \
   pango-querymodules.exe \
   synfig.exe \
   synfigstudio.exe \
   lib \
   share \
# this extra line is required!
do
	cp -rf $MINGWPREFIX/bin/$file $DISTPREFIX/bin || true
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
sed -i '/ffmpeg\.exe/d' bin.nsh		# exclude ffmpeg from the list of binaries - it will go into separate group
gen_list_nsh etc etc
gen_list_nsh examples examples
gen_list_nsh lib/gtk-2.0 lib-gtk
gen_list_nsh lib/synfig lib-synfig
gen_list_nsh licenses licenses
#gen_list_nsh python python # -- takes too long
gen_list_nsh share/locale share-locale
gen_list_nsh share/pixmaps share-pixmaps
gen_list_nsh share/synfig share-synfig
gen_list_nsh share/themes share-themes


#make installer
cp -f $SRCPREFIX/autobuild/synfigstudio.nsi ./
sed -i "s/@VERSION@/$VERSION/g" ./synfigstudio.nsi
cp -f $SRCPREFIX/autobuild/win${ARCH}-specific.nsh ./arch-specific.nsh
"$NSIS_BINARY" -nocd -- synfigstudio.nsi

mv synfigstudio-${VERSION}.exe ../synfigstudio-${VERSION}-${REVISION}-${ARCH}bit.exe

INSTALLER_PATH=`cygpath -w "$WORKSPACE"`
echo
echo
echo
echo "Installer package generated:"
echo "   ${INSTALLER_PATH}synfigstudio-${VERSION}-${REVISION}-${ARCH}bit.exe"
echo
}

mkall()
{
	mkprep
	mkffmpeg
	prepare_mingw_env
	mkimagemagick
	mklibsamplerate
	mksox
	mklibvorbis
	mkmlt
	mketl
	mksynfig
	mksynfigstudio
	mkpackage
}

if [ -z $1 ]; then
	mkall
else
	echo "Executing custom user command..."
	prepare_mingw_env
	$@
fi
