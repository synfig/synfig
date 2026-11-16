#!/usr/bin/env bash
#
# SynfigStudio build script
# Copyright (c) 2008-2018 Konstantin Dmitriev
#
# This file is part of Synfig.
#
# Synfig is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Synfig is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Synfig.  If not, see <https://www.gnu.org/licenses/>.

# = Usage: =
#    ./build.sh [package] [mode]
#
# where:
#   - [package] is all|core|studio
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
pushd "${REPO_DIR}/.." > /dev/null
REPO_DIR=`pwd`
popd > /dev/null

RELEASE=8
if [ -z "$PREFIX" ]; then
#PREFIX=$HOME/synfig
PREFIX=`pwd`/build
fi

# detect number of threads for make (this can be overridden by build.conf)
MAKE_THREADS="$(which nproc >/dev/null && nproc || sysctl -n hw.ncpu || getconf _NPROCESSORS_ONLN || echo 2 2>/dev/null)"

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

mkdir -p synfig-core
mkdir -p synfig-studio
mkdir -p "${PREFIX}/bin"

#========================== VARIABLES ==================================

if [[ `uname` == "Linux" ]]; then
	export PKG_CONFIG_PATH="${PREFIX}/lib64/pkgconfig:/usr/local/lib/pkgconfig:/usr/lib/`uname -i`-linux-gnu/pkgconfig/:${PKG_CONFIG_PATH}"
fi

if [[ `uname` == "MSYS"* ]]; then
	echo "You need to switch to MSYS2 MINGW shell, you are currently using pure MSYS"
	exit 1
fi

if [[ `uname` == "MINGW"* ]]; then # MacOS doesn't support `uname -o` flag
	PATH="${PREFIX}/lib/ccache/bin:${PATH}"
	# copy MLT
	MLT_REV=2   # Change this when something is changed inside of if block below
	if [ ! -f "${PREFIX}/mlt-${MLT_REV}.done" ]; then
		if [ -d "${MINGW_PREFIX}/lib/mlt" ]; then
			cp -rf "${MINGW_PREFIX}"/bin/libmlt*.dll "${PREFIX}/bin/"
			[ -f "${MINGW_PREFIX}/bin/melt.exe" ] && cp -rf "${MINGW_PREFIX}/bin/melt.exe" "${PREFIX}/bin/"
			mkdir -p "${PREFIX}/bin/share/"
			cp -rf "${MINGW_PREFIX}/share/mlt" "${PREFIX}/bin/share/"
			mkdir -p "${PREFIX}/bin/lib/"
			cp -rf "${MINGW_PREFIX}/lib/mlt" "${PREFIX}/bin/lib/"
		else
			OPT_MLT_DIR=$(ls -d /opt/mlt-* 2>/dev/null | tail -n 1)
			if [ -n "$OPT_MLT_DIR" ] && [ -d "$OPT_MLT_DIR" ]; then
				cp -rf "${OPT_MLT_DIR}"/*.dll "${PREFIX}/bin/"
				[ -f "${OPT_MLT_DIR}/melt.exe" ] && cp -rf "${OPT_MLT_DIR}/melt.exe" "${PREFIX}/bin/"
				[ -d "${OPT_MLT_DIR}/share" ] && cp -rf "${OPT_MLT_DIR}/share" "${PREFIX}/bin/"
				mkdir -p "${PREFIX}/bin/lib/"
				[ -d "${OPT_MLT_DIR}/lib/mlt" ] && cp -rf "${OPT_MLT_DIR}/lib/mlt" "${PREFIX}/bin/lib/"
			fi
		fi
		touch "${PREFIX}/mlt-${MLT_REV}.done"
	fi
	if [ ! -d "${MINGW_PREFIX}/lib/mlt" ]; then
		for opt_mlt in /opt/mlt-*/lib/pkgconfig; do
			if [ -d "$opt_mlt" ]; then
				export PKG_CONFIG_PATH="${opt_mlt}:${PKG_CONFIG_PATH}"
			fi
		done
	fi
fi
if [[ `uname` == "Darwin" ]]; then
	# autopoint is not in PATH after install via brew (conflicting with system gettext https://github.com/Homebrew/legacy-homebrew/issues/24070)
	# so we can do `brew link --force gettext` or just add it to PATH before configuring which is preferable because we need it only for compiling
	export PATH="$(brew --prefix ccache)/libexec:$(brew --prefix gettext)/bin:${PATH}"
	# `lib` is required to correctly detect libmng/libjpeg in autotools. Current `configure.ac` script doesn't use
	# `pkg-config` to get library flags (libmng provides pkg-config (.pc) file from 2.0.3, but Debian still using 1.0.1).
	export LDFLAGS="-L$(brew --prefix)/lib ${LDFLAGS}"

	#export LDFLAGS="-L$(brew --prefix gettext)/lib ${LDFLAGS}"
	#export LDFLAGS="-L$(brew --prefix libomp)/lib ${LDFLAGS}"
	#export LDFLAGS="-L$(brew --prefix libtool)/lib ${LDFLAGS}"
	#export CPPFLAGS="-I$(brew --prefix gettext)/include ${CPPFLAGS}"
	#export PKG_CONFIG_PATH="$(brew --prefix libffi)/lib/pkgconfig:${PKG_CONFIG_PATH}"

	# Force use system perl, see https://github.com/synfig/synfig/issues/794
	cat > "${PREFIX}/bin/perl" <<EOF
#!/bin/sh

/usr/bin/perl "\$@"
EOF
	chmod +x "${PREFIX}/bin/perl"

fi
export PKG_CONFIG_PATH="${PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH}"
export PATH="${PREFIX}/bin:$PATH"
export LD_LIBRARY_PATH="${PREFIX}/lib:${PREFIX}/lib64:/usr/local/lib:$LD_LIBRARY_PATH"
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
pushd "${REPO_DIR}/synfig-core/" >/dev/null
/bin/bash "${REPO_DIR}/synfig-core/bootstrap.sh"
popd >/dev/null
if [[ `uname -o` == "Msys" ]]; then
	# Currently there is an error when building with Magick++ on MSYS2
	export CONFIGURE_OPTIONS="--without-magickpp"
else
	export CONFIGURE_OPTIONS="--with-magickpp"
fi
if [[ `uname` == "Darwin" ]]; then
	base_version=10.13 # High Sierra
	version=$(sw_vers -productVersion)
	if [[ $(echo -e $base_version"\n"$version | sort -V | tail -1) == "$base_version" ]]; then
		# this version supports imagemagick
		echo ""
	else
		# Currently there is an error when building with imagemack on OSX >= High Sierra
		export CONFIGURE_OPTIONS="$CONFIGURE_OPTIONS --without-imagemagick --without-magickpp"
	fi
fi
/bin/bash "${REPO_DIR}/synfig-core/configure" --prefix="${PREFIX}" \
	--includedir="${PREFIX}/include" \
	--disable-static --enable-shared \
	--without-libavcodec \
	--without-included-ltdl \
	$CONFIGURE_OPTIONS \
	$DEBUG
cd ..
}

core_make()
{
cd synfig-core
make -j$MAKE_THREADS
sed -i.bak "s|^includedir=.*$|includedir=$REPO_DIR\/synfig-core\/src|" synfig.pc
make install
make check
cd ..

ccache_show_stats

}

core_check()
{
cd synfig-core
make check
cd ..
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
pushd "${REPO_DIR}/synfig-studio/" >/dev/null
/bin/bash "${REPO_DIR}/synfig-studio/bootstrap.sh"
popd >/dev/null
if [[ `uname` == "Linux" ]]; then
	export CONFIGURE_OPTIONS="--enable-jack"
else
	export CONFIGURE_OPTIONS=""
fi

/bin/bash "${REPO_DIR}/synfig-studio/configure" --prefix="${PREFIX}" \
	--includedir="${PREFIX}/include" \
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
make check

ccache_show_stats

for n in AUTHORS COPYING NEWS README
do
  	cp -f "${REPO_DIR}/synfig-studio/$n" "${PREFIX}"
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

studio_check()
{
cd synfig-studio
make check
cd ..
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
core_clean
studio_clean
}

all_configure()
{
core_configure
studio_configure
}

all_make()
{
core_make
studio_make
}

all_build()
{
travis_fold_start synfig-core "Building Synfig Core"
core_build
travis_fold_end synfig-core

travis_fold_start synfig-studio "Building Synfig Studio"
studio_build
travis_fold_end synfig-studio
}

all_full()
{
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
