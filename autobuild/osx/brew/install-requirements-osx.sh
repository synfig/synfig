#!/bin/bash

# zlib not required because it already part of OSX
# disable brew auto-update for travis-ci
HOMEBREW_NO_AUTO_UPDATE=1 brew install autoconf automake libtool intltool gettext pkg-config glibmm libxml++ cairo fftw pango mlt boost gtkmm3
