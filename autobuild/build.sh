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
# == Standart mode ==
# Configure and (re)build:
#    ./build.sh
# Configure and make clean build:
#    ./build.sh all full
# Quick rebuild (without configure):
#    ./build.sh all make
# Quick rebuild od synfig-core (without configure):
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

export PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig:${PREFIX}/lib64/pkgconfig:/usr/local/lib/pkgconfig:/usr/lib/`uname -i`-linux-gnu/pkgconfig/:$PKG_CONFIG_PATH
export PATH=${PREFIX}/bin:$PATH
export LD_LIBRARY_PATH=${PREFIX}/lib:${PREFIX}/lib64:/usr/local/lib:$LD_LIBRARY_PATH
export LDFLAGS="-Wl,-rpath -Wl,\\\$\$ORIGIN/lib"


#============================ NIX stuff ================================
# Assert that FILE exists and is executable
#
# assertExecutable FILE
assertExecutable() {
    local file="$1"
    [[ -f "$file" && -x "$file" ]] || \
        die "Cannot wrap '$file' because it is not an executable file"
}

# construct an executable file that wraps the actual executable
# makeWrapper EXECUTABLE ARGS

# ARGS:
# --argv0 NAME      : set name of executed process to NAME
#                     (otherwise it’s called …-wrapped)
# --set   VAR VAL   : add VAR with value VAL to the executable’s environment
# --unset VAR       : remove VAR from the environment
# --run   COMMAND   : run command before the executable
#                     The command can push extra flags to a magic list variable
#                     extraFlagsArray, which are then added to the invocation
#                     of the executable
# --add-flags FLAGS : add FLAGS to invocation of executable

# --prefix          ENV SEP VAL   : suffix/prefix ENV with VAL, separated by SEP
# --suffix
# --suffix-each     ENV SEP VALS  : like --suffix, but VALS is a list
# --prefix-contents ENV SEP FILES : like --suffix-each, but contents of FILES
#                                   are read first and used as VALS
# --suffix-contents
makeWrapper() {
    local original="$1"
    local wrapper="$2"
    local params varName value command separator n fileNames
    local argv0 flagsBefore flags

    assertExecutable "$original"

    mkdir -p "$(dirname "$wrapper")"

    echo "#! $SHELL -e" > "$wrapper"

    params=("$@")
    for ((n = 2; n < ${#params[*]}; n += 1)); do
        p="${params[$n]}"

        if [[ "$p" == "--set" ]]; then
            varName="${params[$((n + 1))]}"
            value="${params[$((n + 2))]}"
            n=$((n + 2))
            echo "export $varName=${value@Q}" >> "$wrapper"
        elif [[ "$p" == "--set-default" ]]; then
            varName="${params[$((n + 1))]}"
            value="${params[$((n + 2))]}"
            n=$((n + 2))
            echo "export $varName=\${$varName-${value@Q}}" >> "$wrapper"
        elif [[ "$p" == "--unset" ]]; then
            varName="${params[$((n + 1))]}"
            n=$((n + 1))
            echo "unset $varName" >> "$wrapper"
        elif [[ "$p" == "--run" ]]; then
            command="${params[$((n + 1))]}"
            n=$((n + 1))
            echo "$command" >> "$wrapper"
        elif [[ ("$p" == "--suffix") || ("$p" == "--prefix") ]]; then
            varName="${params[$((n + 1))]}"
            separator="${params[$((n + 2))]}"
            value="${params[$((n + 3))]}"
            n=$((n + 3))
            if test -n "$value"; then
                if test "$p" = "--suffix"; then
                    echo "export $varName=\$$varName\${$varName:+${separator@Q}}${value@Q}" >> "$wrapper"
                else
                    echo "export $varName=${value@Q}\${$varName:+${separator@Q}}\$$varName" >> "$wrapper"
                fi
            fi
        elif [[ "$p" == "--suffix-each" ]]; then
            varName="${params[$((n + 1))]}"
            separator="${params[$((n + 2))]}"
            values="${params[$((n + 3))]}"
            n=$((n + 3))
            for value in $values; do
                echo "export $varName=\$$varName\${$varName:+$separator}${value@Q}" >> "$wrapper"
            done
        elif [[ ("$p" == "--suffix-contents") || ("$p" == "--prefix-contents") ]]; then
            varName="${params[$((n + 1))]}"
            separator="${params[$((n + 2))]}"
            fileNames="${params[$((n + 3))]}"
            n=$((n + 3))
            for fileName in $fileNames; do
                contents="$(cat "$fileName")"
                if test "$p" = "--suffix-contents"; then
                    echo "export $varName=\$$varName\${$varName:+$separator}${contents@Q}" >> "$wrapper"
                else
                    echo "export $varName=${contents@Q}\${$varName:+$separator}\$$varName" >> "$wrapper"
                fi
            done
        elif [[ "$p" == "--add-flags" ]]; then
            flags="${params[$((n + 1))]}"
            n=$((n + 1))
            flagsBefore="$flagsBefore $flags"
        elif [[ "$p" == "--argv0" ]]; then
            argv0="${params[$((n + 1))]}"
            n=$((n + 1))
        else
            die "makeWrapper doesn't understand the arg $p"
        fi
    done

    # Note: extraFlagsArray is an array containing additional flags
    # that may be set by --run actions.
    # Silence warning about unexpanded extraFlagsArray:
    # shellcheck disable=SC2016
    echo exec ${argv0:+-a \"$argv0\"} \""$original"\" \
         "$flagsBefore" '"${extraFlagsArray[@]}"' '"$@"' >> "$wrapper"

    chmod +x "$wrapper"
}

addSuffix() {
    suffix="$1"
    shift
    for name in "$@"; do
        echo "$name$suffix"
    done
}

filterExisting() {
    for fn in "$@"; do
        if test -e "$fn"; then
            echo "$fn"
        fi
    done
}

# Syntax: wrapProgram <PROGRAM> <MAKE-WRAPPER FLAGS...>
wrapProgram() {
    local prog="$1"
    local hidden

    assertExecutable "$prog"

    hidden="$(dirname "$prog")/.$(basename "$prog")"-wrapped
    while [ -e "$hidden" ]; do
      hidden="${hidden}_"
    done
    mv "$prog" "$hidden"
    # Silence warning about unexpanded $0:
    # shellcheck disable=SC2016
    makeWrapper "$hidden" "$prog" --argv0 '$0' "${@:2}"
}
# Exit with backtrace and error message
#
# Usage: die "Error message"
die() {
    # Let us be a little sloppy with errors, because otherwise the final
    # invocation of `caller` below will cause the script to exit.
    set +e

    # Print our error message
    printf "\nBuilder called die: %b\n" "$*"
    printf "Backtrace:\n"

    # Print a backtrace.
    local frame=0
    while caller $frame; do
        ((frame++));
    done
    printf "\n"

    exit 1
}
#=======================================================================

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
sed -i "s|^Cflags: -I\\\${includedir}|Cflags: -I$REPO_DIR\/ETL -I\\\${includedir}|" ETL.pc
make install
cd ..
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
sed -i "s|^includedir=.*$|includedir=$REPO_DIR\/synfig-core\/src|" synfig.pc
make install
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
pushd ${REPO_DIR}/synfig-studio/ >/dev/null
/bin/bash ${REPO_DIR}/synfig-studio/bootstrap.sh
popd >/dev/null
/bin/bash ${REPO_DIR}/synfig-studio/configure --prefix=${PREFIX} \
	--includedir=${PREFIX}/include \
	--disable-static \
	--enable-shared \
	--enable-jack \
	--enable-warnings=max $DEBUG
cd ..
}

studio_make()
{
cd synfig-studio
make -j$MAKE_THREADS
make install
for n in AUTHORS COPYING NEWS README
do
  	cp -f ${REPO_DIR}/synfig-studio/$n ${PREFIX}
done

if [ ! -z "$NIX_BUILD_CORES" ]; then
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
etl_build
core_build
studio_build
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
