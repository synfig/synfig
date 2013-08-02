#!/bin/bash

export CYGWIN_SETUP=/cygdrive/d/synfig-win/cygwin-setup.exe
export PREFIX=/src/synfig

# Install dependencies
#-K http://cygwinports.org/ports.gpg -s ftp://ftp.cygwinports.org/pub/cygwinports -s http://mirrors.163.com/cygwin \
$CYGWIN_SETUP \
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
-P libcairo-devel \
-P libpango1.0-devel \
-P libboost-devel \
-P libboost1.50 \
-P libjpeg-devel \
-P libpng-devel \
-P p7zip \
-q

#TODO: magick++

$CYGWIN_SETUP \
-K http://cygwinports.org/ports.gpg -s ftp://ftp.cygwinports.org/pub/cygwinports -s http://mirrors.163.com/cygwin \
-P libxml++2.6-devel  \
-P libgtkmm2.4-devel \
-q

cd $PREFIX/ETL
autoreconf --install --force
./configure --prefix=/usr
make install

cd $PREFIX/synfig-core
libtoolize --copy --force
autoreconf --install --force
./configure --prefix=/usr
make -j4
make install