#!/bin/bash

# libtool for synfig-core glibtoolize
# gettext for autopoint

# autopoint is not in PATH after install via brew (conflicting with system gettext https://github.com/Homebrew/legacy-homebrew/issues/24070)
# so we can do `brew link --force gettext` or just add it to PATH before configuring which is preferable because we need it only for compiling

#export PATH=/usr/local/opt/gettext/bin:$PATH
# assume we have ccache and gettext already installed
export PATH="/usr/local/opt/ccache/libexec:/usr/local/opt/gettext/bin:$PATH"
SCRIPT_PATH=$(cd `dirname "$0"`; pwd)

MAKE_THREADS=2
MAKE_OPTIONS="-j$MAKE_THREADS --silent LIBTOOLFLAGS=--silent"
export CFLAGS="-fdiagnostics-color=always $CFLAGS"
export CXXFLAGS="-fdiagnostics-color=always $CXXFLAGS"
#CONFIGURE_FLAGS="--enable-optimization=0"

set -e

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


# move to synfig root dir
pushd "$SCRIPT_PATH/../../../"

travis_fold_start ETL "Building ETL"
pushd ETL
  ./bootstrap.sh
  ./configure
  make install $MAKE_OPTIONS
popd # ETL
travis_fold_end ETL

travis_fold_start synfig-core "Building Synfig Core"
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
  make install $MAKE_OPTIONS
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