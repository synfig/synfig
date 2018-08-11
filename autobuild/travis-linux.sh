#!/bin/bash
#

REPO_DIR=`dirname "$0"`
pushd ${REPO_DIR}/.. > /dev/null
REPO_DIR=`pwd`
popd > /dev/null

curl https://nixos.org/nix/install | bash
source ~/.nix-profile/etc/profile.d/nix.sh

nix-shell --command ${REPO_DIR}/build-production.sh ${REPO_DIR}/autobuild/default.nix
