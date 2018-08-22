#!/bin/bash

# zlib not required because it already part of OSX
# disable brew auto-update for travis-ci
SCRIPT_PATH=$(cd `dirname "$0"`; pwd)
HOMEBREW_NO_AUTO_UPDATE=1 brew bundle -no-upgrade --file=$SCRIPT_PATH/Brewfile
#install autoconf automake libtool intltool gettext pkg-config glibmm libxml++ cairo fftw pango mlt boost gtkmm3
