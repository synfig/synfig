#!/bin/bash

export CYGWIN_SETUP="/cygdrive/c/synfig-build/cygwin-dist/setup-x86.exe"
export SRCPREFIX=`dirname "$0"`
SRCPREFIX=$(cd "$SRCPREFIX/.."; pwd)

# Install dependencies
#-K http://cygwinports.org/ports.gpg -s ftp://ftp.cygwinports.org/pub/cygwinports -s http://mirrors.163.com/cygwin \
CYGPORT_MIRROR=http://mirrors.kernel.org/sources.redhat.com/cygwinports

$CYGWIN_SETUP \
-K http://cygwinports.org/ports.gpg -s $CYGPORT_MIRROR -s http://ftp.linux.kiev.ua/pub/cygwin/ \
-P git \
-P make \
-P gcc-core \
-P gcc-g++ \
-P gdb \
-P autoconf \
-P automake \
-P libtool \
-P pkg-config \
-P libcairo-devel \
-P libpango1.0-devel \
-P libboost-devel \
-P libboost1.50 \
-P libjpeg-devel \
-P libpng-devel \
-P p7zip \
-P ImageMagick \
-P libxml++2.6-devel  \
-P libgtkmm2.4-devel \
-q

cd $SRCPREFIX/ETL
autoreconf --install --force
./configure --prefix=/usr
make install

cd $SRCPREFIX/synfig-core
./bootstrap.sh
./configure --prefix=/usr
make -j4
make install