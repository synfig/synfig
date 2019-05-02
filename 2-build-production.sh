#!/usr/bin/env bash
#
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

export TYPE="_production"
export DEBUG=0

WORKDIR=`dirname "$0"`
pushd "${WORKDIR}" > /dev/null
WORKDIR=`pwd`
popd > /dev/null

[ -d "${WORKDIR}/${TYPE}" ] || mkdir "${WORKDIR}/${TYPE}"

[ -f "${WORKDIR}/build.conf" ] && cp -f "${WORKDIR}/build.conf" "${WORKDIR}/${TYPE}/build.conf"

cd "${WORKDIR}/${TYPE}"

bash ${WORKDIR}/autobuild/build.sh "$1" "$2"
