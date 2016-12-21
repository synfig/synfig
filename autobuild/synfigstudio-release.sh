#!/bin/bash

# Usage notes
#
# Running this script will creates release tarballs for all synfig 
# modules and testing them by installing into ~/local-synfig/.
#
# It is also possible to run procedure for each module separately by 
# passing specific arguments to the script:
# * Run procedures for ETL:
#    synfigstudio-release.sh etl
# * Run procedures for synfig-core:
#    synfigstudio-release.sh core
# * Run procedures for synfig-studio:
#    synfigstudio-release.sh studio
#

set -e

export SCRIPTPATH=$(cd `dirname "$0"`; pwd)
export SRCPREFIX=`dirname "$SCRIPTPATH"`


export PREFIX="$HOME/local-synfig"
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$PREFIX/lib/pkgconfig"
export PATH="$PREFIX/bin:$PATH"
export ETL_VERSION=`cat $SRCPREFIX/ETL/configure.ac |egrep "AC_INIT\(\[Extended Template Library\],"| sed "s|.*Library\],\[||" | sed "s|\],\[.*||"`
echo "ETL_VERSION=$ETL_VERSION"
export CORE_VERSION=`cat $SRCPREFIX/synfig-core/configure.ac |egrep "AC_INIT\(\[Synfig Core\],"| sed "s|.*Core\],\[||" | sed "s|\],\[.*||"`
echo "CORE_VERSION=$CORE_VERSION"
export STUDIO_VERSION=`cat $SRCPREFIX/synfig-studio/configure.ac |egrep "AC_INIT\(\[Synfig Studio\],"| sed "s|.*Studio\],\[||" | sed "s|\],\[.*||"`
echo "STUDIO_VERSION=$STUDIO_VERSION"

if [ -e /etc/debian_version ] && [ -z $with_boost_libdir ]; then
	# Debian/Ubuntu multiarch
	MULTIARCH_LIBDIR="/usr/lib/`uname -m`-linux-gnu/"
	if [ -e "${MULTIARCH_LIBDIR}/libboost_program_options.so" ]; then
		export with_boost_libdir=$MULTIARCH_LIBDIR
	fi
fi


if [ -z $THREADS ]; then
	export THREADS=4
fi

pack-etl()
{
	cd $SRCPREFIX/ETL
	autoreconf -if
	./configure --prefix="$PREFIX"
	make distcheck -j${THREADS}
	mv ETL-${ETL_VERSION}.tar.gz ../../
}

test-etl()
{
	cd $SRCPREFIX/../
	tar xf ETL-${ETL_VERSION}.tar.gz
	cd ETL-${ETL_VERSION}
	./configure --prefix="$PREFIX"
	make install -j${THREADS}
	cd ..
	rm -rf $SRCPREFIX/../ETL-${ETL_VERSION}
}

etl()
{
	pack-etl
	test-etl
}

pack-core()
{
	cd $SRCPREFIX/synfig-core
	libtoolize --ltdl --copy -f
	autoreconf --install --force
	./configure --prefix="$PREFIX"
echo "------------------------------------- pack-core make"
	make distcheck -j${THREADS}
	mv synfig-${CORE_VERSION}.tar.gz ../../
}

test-core()
{
echo "------------------------------------- test-core"
	cd $SRCPREFIX/../
	tar xf synfig-${CORE_VERSION}.tar.gz
	cd synfig-${CORE_VERSION}
	./configure --prefix="$PREFIX"
	make install -j${THREADS}
	cd ..
	rm -rf $SRCPREFIX/../synfig-${CORE_VERSION}
}

core()
{
	pack-core
	test-core
}

pack-studio()
{
	cd $SRCPREFIX/synfig-studio
	./bootstrap.sh
	./configure --prefix="$PREFIX"
	make distcheck -j${THREADS}
	mv synfigstudio-${STUDIO_VERSION}.tar.gz ../..
}

test-studio()
{
	cd $SRCPREFIX/../
	tar xf synfigstudio-${STUDIO_VERSION}.tar.gz
	cd synfigstudio-${STUDIO_VERSION}
	./configure --prefix="$PREFIX"
	make install -j${THREADS}
	cd ..
	rm -rf $SRCPREFIX/../synfigstudio-${STUDIO_VERSION}
}

studio()
{
	pack-studio
	test-studio
}

mkall()
{
	etl
	core
	studio
}

do_cleanup()
{
	echo "Cleaning up..."
	if [ ${PREFIX} != ${DEPSPREFIX} ]; then
		[ ! -e ${DEPSPREFIX} ] || mv ${DEPSPREFIX} ${DEPSPREFIX}.off
	fi
	[ ! -e ${SYSPREFIX} ] || mv ${SYSPREFIX} ${SYSPREFIX}.off
	exit
}

#trap do_cleanup INT SIGINT SIGTERM EXIT

if [ -z $1 ]; then
	rm -rf "$PREFIX" || true
	mkall
else
	echo "Executing custom user command..."

	$@
fi

#do_cleanup
