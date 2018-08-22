#!/bin/bash

# libtool for synfig-core glibtoolize
# gettext for autopoint

# autopoint is not in PATH after install (conflicting with system gettext https://github.com/Homebrew/legacy-homebrew/issues/24070)
# so we can do `brew link --force gettext` or just add it to PATH before configuring

export PATH=/usr/local/opt/gettext/bin:$PATH
SCRIPT_PATH=$(cd `dirname "$0"`; pwd)

MAKE_THREADS=2
MAKE_OPTIONS="-j$MAKE_THREADS --silent LIBTOOLFLAGS=--silent"

set -e

# enable ccache for speedup
enable_ccache()
{
    if which ccache > /dev/null; then
        echo "ccache found! Enabling ccache..."
        
        # set CC/CXX variables if it is not already
        if [ -z $CC ]; then
            export CC=gcc
        fi
        if [ -z $CXX ]; then
            export CXX=g++
        fi

        export CC="ccache $CC"
        export CXX="ccache $CXX"
        echo "CC=$CC"
        echo "CXX=$CXX"
    else
        echo "ccache not found..."
    fi
}

travis_fold_start()
{
	if [ -n "$TRAVIS" ]; then
		echo -e "travis_fold:start:$1\033[33;1m$2\033[0m"
	fi
}

travis_fold_end()
{
	if [ -n "$TRAVIS" ]; then
		echo -e "\ntravis_fold:end:$1\r"
	fi
}



# enable_ccache

# move to synfig root dir
pushd "$SCRIPT_PATH/../../../"

travis_fold_start ETL "Building ETL"
pushd ETL
./bootstrap.sh
./configure
make install $MAKE_OPTIONS
popd # ETL
travis_fold_end ETL

travis_fold_start synfig-core "Building synfig-core"
pushd synfig-core
./bootstrap.sh
./configure
make install $MAKE_OPTIONS
popd # synfig-core
travis_fold_end synfig-core

travis_fold_start synfig-studio "Building Synfig Studio"
pushd synfig-studio
./bootstrap.sh
./configure
pushd src # enter src directory to skip image building
make install $MAKE_OPTIONS
popd #src
popd # synfig-studio
travis_fold_end synfig-studio

popd # back to start directory



# ============== Helpers ==================



# ./bootstrap.sh
#PATH=/usr/local/opt/gettext/bin:$PATH ./configure

#PATH=/usr/local/opt/gettext/bin:$PATH ./configure --prefix=/Users/az/synfig/

#For compilers to find this software you may need to set:
#    LDFLAGS:  -L/usr/local/opt/libffi/lib
#For pkg-config to find this software you may need to set:
#    PKG_CONFIG_PATH: /usr/local/opt/libffi/lib/pkgconfig


#If you need to have this software first in your PATH run:
#  echo 'export PATH="/usr/local/opt/icu4c/bin:$PATH"' >> ~/.bash_profile
#  echo 'export PATH="/usr/local/opt/icu4c/sbin:$PATH"' >> ~/.bash_profile

#For compilers to find this software you may need to set:
#    LDFLAGS:  -L/usr/local/opt/icu4c/lib
#    CPPFLAGS: -I/usr/local/opt/icu4c/include
#For pkg-config to find this software you may need to set:
#    PKG_CONFIG_PATH: /usr/local/opt/icu4c/lib/pkgconfig

#For compilers to find this software you may need to set:
#    LDFLAGS:  -L/usr/local/opt/zlib/lib
#    CPPFLAGS: -I/usr/local/opt/zlib/include
#For pkg-config to find this software you may need to set:
#    PKG_CONFIG_PATH: /usr/local/opt/zlib/lib/pkgconfig

# zlib not checked