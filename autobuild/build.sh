#!/usr/bin/env bash
#
# SynfigStudio build script
# Copyright (c) 2008-2018 Konstantin Dmitriev
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

# = Usage: =
#    ./build.sh [package] [mode]
#
# where:
#   - [package] is all|etl|core|studio
#   - [mode] is full|clean|configure|make
#   
#
# = Examples: =
#
# == Standard mode ==
# Configure and (re)build:
#    ./build.sh
# Configure and make clean build:
#    ./build.sh all full
# Quick rebuild (without configure):
#    ./build.sh all make
# Quick rebuild of synfig-core (without configure):
#    ./build.sh core make

set -e

REPO_DIR=`dirname "$0"`
pushd ${REPO_DIR}/.. > /dev/null
REPO_DIR=`pwd`
popd > /dev/null

RELEASE=8
if [ -z "$PREFIX" ]; then
#PREFIX=$HOME/synfig
PREFIX=`pwd`/build
fi

MAKE_THREADS=2					#count of threads for make

# Allow overriding PREFIX and other settings
if [ -f "./build.conf" ] ; then
	. "./build.conf"
fi

if [ -z $DEBUG ]; then
	export DEBUG=0
fi

if [[ $DEBUG == 1 ]]; then
	DEBUG='--enable-debug --enable-optimization=0'
else
	DEBUG=''
fi

[ -d ETL ] || mkdir ETL
[ -d synfig-core ] || mkdir synfig-core
[ -d synfig-studio ] || mkdir synfig-studio
[ -d "${PREFIX}" ] || mkdir "${PREFIX}"
[ -d "${PREFIX}/bin" ] || mkdir "${PREFIX}/bin"

#========================== VARIABLES ==================================

if [[ `uname` == "Linux" ]]; then
	export PKG_CONFIG_PATH=${PREFIX}/lib64/pkgconfig:/usr/local/lib/pkgconfig:/usr/lib/`uname -i`-linux-gnu/pkgconfig/:${PKG_CONFIG_PATH}
fi
if [[ `uname -o` == "Msys" ]]; then
	PATH="${PREFIX}/lib/ccache/bin:${PATH}"
	# copy MLT
	MLT_REV=1   # Change this when something is changed inside of if block below
	if [ ! -f ${PREFIX}/mlt-${VERSION_MLT}-${MLT_REV}.done ]; then
		VERSION_MLT="6.16.0"
		cp -rf /opt/mlt-${VERSION_MLT}/*.dll ${PREFIX}/bin/
		cp -rf /opt/mlt-${VERSION_MLT}/*.exe ${PREFIX}/bin/
		cp -rf /opt/mlt-${VERSION_MLT}/share ${PREFIX}/bin/
		mkdir -p ${PREFIX}/bin/lib/
		cp -rf /opt/mlt-${VERSION_MLT}/lib/mlt ${PREFIX}/bin/lib/
		touch ${PREFIX}/mlt-${VERSION_MLT}-${MLT_REV}.done
	fi
	export PKG_CONFIG_PATH="/opt/mlt-${VERSION_MLT}/lib/pkgconfig:${PKG_CONFIG_PATH}"
fi
if [[ `uname` == "Darwin" ]]; then
	# autopoint is not in PATH after install via brew (conflicting with system gettext https://github.com/Homebrew/legacy-homebrew/issues/24070)
	# so we can do `brew link --force gettext` or just add it to PATH before configuring which is preferable because we need it only for compiling
	export PATH="/usr/local/opt/ccache/libexec:/usr/local/opt/gettext/bin:${PATH}"
	export LDFLAGS="-L/usr/local/opt/gettext/lib ${LDFLAGS}"
	export LDFLAGS="-L$(brew --prefix libomp)/lib ${LDFLAGS}"
	export LDFLAGS="-L$(brew --prefix libtool)/lib ${LDFLAGS}"
	export CPPFLAGS="-I/usr/local/opt/gettext/include ${CPPFLAGS}"
	export PKG_CONFIG_PATH="/usr/local/opt/libffi/lib/pkgconfig:${PKG_CONFIG_PATH}"

	# Force use system perl, see https://github.com/synfig/synfig/issues/794
	cat > ${PREFIX}/bin/perl <<EOF
#!/bin/sh

/usr/bin/perl "\$@"
EOF
	chmod +x ${PREFIX}/bin/perl

fi
export PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH}
export PATH=${PREFIX}/bin:$PATH
export LD_LIBRARY_PATH=${PREFIX}/lib:${PREFIX}/lib64:/usr/local/lib:$LD_LIBRARY_PATH
export LDFLAGS="-Wl,-rpath -Wl,\\\$\$ORIGIN/lib ${LDFLAGS}"
export CFLAGS="-fdiagnostics-color=always $CFLAGS"
export CXXFLAGS="-fdiagnostics-color=always $CXXFLAGS"

#========================== FUNCTIONS ==================================

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

ccache_show_stats()
{
	if ( which ccache > /dev/null ); then
	ccache -s
	fi
}

#============================== ETL ====================================

etl_clean() {
cd ETL
echo "Cleaning source tree..."
make clean || true
cd ..
}

etl_configure()
{
cd ETL
echo "Going to configure..."
pushd ${REPO_DIR}/ETL/ >/dev/null
/bin/bash ${REPO_DIR}/ETL/bootstrap.sh
popd
/bin/bash ${REPO_DIR}/ETL/configure --prefix=${PREFIX} --includedir=${PREFIX}/include $DEBUG
cd ..
}

etl_make()
{
cd ETL
make -j$MAKE_THREADS
sed -i.bak "s|^Cflags: -I\\\${includedir}|Cflags: -I$REPO_DIR\/ETL -I\\\${includedir}|" ETL.pc
make install
cd ..

ccache_show_stats

}

etl_build()
{
etl_configure
etl_make
}

etl_full()
{
etl_clean
etl_configure
etl_make
}

#======================== Synfig-Core ==================================

core_clean()
{
cd synfig-core
echo "Cleaning source tree..."
make clean || true
cd ..
}

core_configure()
{
cd synfig-core
pushd ${REPO_DIR}/synfig-core/ >/dev/null
/bin/bash ${REPO_DIR}/synfig-core/bootstrap.sh
popd >/dev/null
if [ -e /etc/debian_version ] && [ -z "$BOOST_CONFIGURE_OPTIONS" ]; then
	# Debian/Ubuntu multiarch
	MULTIARCH_LIBDIR="/usr/lib/`uname -m`-linux-gnu/"
	if [ -e "${MULTIARCH_LIBDIR}/libboost_program_options.so" ]; then
		export BOOST_CONFIGURE_OPTIONS="--with-boost-libdir=$MULTIARCH_LIBDIR"
	fi
fi
/bin/bash ${REPO_DIR}/synfig-core/configure --prefix=${PREFIX} \
	--includedir=${PREFIX}/include \
	--disable-static --enable-shared \
	--with-magickpp \
	--without-libavcodec \
	--without-included-ltdl \
	$BOOST_CONFIGURE_OPTIONS \
	$DEBUG
cd ..
}

core_make()
{
cd synfig-core
make -j$MAKE_THREADS
sed -i.bak "s|^includedir=.*$|includedir=$REPO_DIR\/synfig-core\/src|" synfig.pc
make install
cd ..

ccache_show_stats

}

core_build()
{
core_configure
core_make
}

core_full()
{
core_clean
core_configure
core_make
}

#======================== Synfig-Studio ==================================

studio_clean()
{
cd synfig-studio
echo "Cleaning source tree..."
make clean || true
cd ..
}

studio_configure()
{
cd synfig-studio
pushd ${REPO_DIR}/synfig-studio/ >/dev/null
/bin/bash ${REPO_DIR}/synfig-studio/bootstrap.sh
popd >/dev/null
if [[ `uname` == "Linux" ]]; then
	export CONFIGURE_OPTIONS="--enable-jack"
elif [[ `uname -o` == "Msys" ]]; then
	# currently where is a bug in synfig-core (in MinGW build) which causes 
	# synfig-core to halt before exit, so we skip image generation
	export CONFIGURE_OPTIONS="--without-images"
else
	export CONFIGURE_OPTIONS=""
fi

/bin/bash ${REPO_DIR}/synfig-studio/configure --prefix=${PREFIX} \
	--includedir=${PREFIX}/include \
	--disable-static \
	--enable-shared \
	${CONFIGURE_OPTIONS} \
	--enable-warnings=max $DEBUG
cd ..
}

studio_make()
{
cd synfig-studio

make -j$MAKE_THREADS
make install

ccache_show_stats

for n in AUTHORS COPYING NEWS README
do
  	cp -f ${REPO_DIR}/synfig-studio/$n ${PREFIX}
done

if [ ! -z "$NIX_BUILD_CORES" ]; then
	source "`head -n 1 ${MAKE_WRAPPER_PATH}/nix-support/propagated-build-inputs | sed -e 's/[[:space:]]*$//'`/nix-support/setup-hook"
	source "${MAKE_WRAPPER_PATH}/nix-support/setup-hook"
	wrapProgram "${PREFIX}/bin/synfigstudio" \
		  --prefix XDG_DATA_DIRS : "$XDG_ICON_DIRS:$GSETTINGS_SCHEMAS_PATH" \
		  --prefix XCURSOR_PATH : "$ADWAITA_PATH/share/icons" \
		  --set XCURSOR_THEME "Adwaita"
fi

cd ..
msg_done
}

studio_build()
{
studio_configure
studio_make
}

studio_full()
{
studio_clean
studio_configure
studio_make
}

#=============================== ALL ===================================

all_clean()
{
etl_clean
core_clean
studio_clean
}

all_configure()
{
etl_configure
core_configure
studio_configure
}

all_make()
{
etl_make
core_make
studio_make
}

all_build()
{
travis_fold_start ETL "Building ETL"
etl_build
travis_fold_end ETL

travis_fold_start synfig-core "Building Synfig Core"
core_build
travis_fold_end synfig-core

travis_fold_start synfig-studio "Building Synfig Studio"
studio_build
travis_fold_end synfig-studio
}

all_full()
{
etl_full
core_full
studio_full
}

msg_done()
{
echo
echo
echo "Done. Please check your Synfig installation in"
echo " ${PREFIX}"
echo
echo "You can start Synfig by executing"
echo " ${PREFIX}/bin/synfigstudio"
echo
}

#============================== MAIN ===================================

if [ -z $1 ]; then
	ARG1='all'
else
	ARG1=$1
fi

if [ -z $2 ]; then
	ARG2='build'
else
	ARG2=$2
fi

# executing command
${ARG1}_${ARG2}
