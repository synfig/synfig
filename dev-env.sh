#!/bin/bash
#

set -e

WORKDIR=`dirname "$0"`
pushd "${WORKDIR}" > /dev/null
WORKDIR=`pwd`
popd > /dev/null

. $HOME/.nix-profile/etc/profile.d/nix.sh
nix-shell "${WORKDIR}/autobuild/default.nix"
