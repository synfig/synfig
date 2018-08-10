#!/bin/bash
#

set -e

WORKDIR=`dirname "$0"`
pushd "${WORKDIR}" > /dev/null
WORKDIR=`pwd`
popd > /dev/null

nix-shell "${WORKDIR}/autobuild/default.nix"
