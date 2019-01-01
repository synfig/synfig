#!/bin/bash

# zlib not required because it is already part of OSX

# Travis-CI workaround
# When we install a package that is already installed inside the travis VM, and we have an outdated brew
# (for faster build), then we get an error and the build fails. Unfortunately brew doesn't have switch to skip
# already installed packages (https://github.com/Homebrew/brew/issues/2491) and recommends to install using
# brew bundle. But! `brew bundle` can't be run without updating brew and fails with error "Your Homebrew is outdated".
# Yep, i know what and this is what i want. So here is workaround: first we check which package is not already installed,
# and if not then install it.


set -e

if ! ( which brew >/dev/null ); then
    echo "No brew found. Installing..."
    /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
fi

WORKDIR=`dirname "$0"`
pushd "${WORKDIR}" > /dev/null
WORKDIR=`pwd`
popd > /dev/null

PACKAGES="adwaita-icon-theme autoconf automake ccache libtool intltool imagemagick gettext pkg-config glibmm libxml++ cairo fftw pango mlt boost gtkmm3 sdl2 sdl2_mixer"

export HOMEBREW_NO_AUTO_UPDATE=1
export HOMEBREW_NO_ANALYTICS=1

export OS=`uname -r | cut -d "." -f1`

if [ $OS -lt 15 ]; then #For OSX < 10.11
    brew install "${WORKDIR}/autobuild/osx/librsvg-2.40.20.rb"
    brew install "${WORKDIR}/autobuild/osx/adwaita-icon-theme.rb"
fi

for pkg in $PACKAGES;
do
    echo "Checking $pkg..."
    brew info "$pkg" | grep --quiet 'Not installed' && brew install "$pkg"
done

#HOMEBREW_NO_AUTO_UPDATE=1 brew bundle -no-upgrade --file=-<<-EOF
#brew "autoconf"
#brew "automake"
#brew "boost"
#brew "cairo"
#brew "fftw"
#brew "gettext"
#brew "glibmm"
#brew "gtkmm3"
#brew "intltool"
#brew "libtool"
#brew "libxml++"
#brew "mlt"
#brew "pango"
#brew "pkg-config"
#EOF
