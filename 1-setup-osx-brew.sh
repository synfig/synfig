#!/bin/bash

# zlib not required because it is already part of MacOS/OSX

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
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

WORKDIR=`dirname "$0"`
pushd "${WORKDIR}" > /dev/null
WORKDIR=`pwd`
popd > /dev/null

PACKAGES="\
adwaita-icon-theme \
autoconf \
automake \
ccache \
cmake \
fftw \
fontconfig \
fribidi \
gettext \
glibmm \
gtkmm3 \
imagemagick \
intltool \
libtool \
libxml++ \
libxml2 \
libxslt \
mlt \
ninja \
pkg-config \
python \
sdl2 \
sdl2_mixer"

export HOMEBREW_NO_AUTO_UPDATE=1
export HOMEBREW_NO_ANALYTICS=1

export OS=`uname -r | cut -d "." -f1`

if [ $OS -lt 15 ] && [ -z "$TRAVIS_BUILD_DIR" ]; then # For OSX < 10.11
    # We need librsvg package, but the latest version shipped by Homebrew
    # depends on Rust, which is fails to compile on OSX < 10.11.
    # So, we are rolling back Homebrew's history of packages to use older
    # version of librsvg.
    cd /usr/local/Homebrew/Library/Taps/homebrew/homebrew-core/
    git fetch --unshallow || true
    git checkout a91becd6afc177b0cada2cf9cce2e3bde514053b # librsvg 2.40.20 (without rust) 2017.12.16
    cd /usr/local/Homebrew/
    git checkout 1.4.1
    brew info gobject-introspection | grep >/dev/null 'Not installed' && brew install ${WORKDIR}/autobuild/osx/gobject-introspection.rb
    brew info fribidi | grep >/dev/null 'Not installed' && brew install ${WORKDIR}/autobuild/osx/fribidi.rb # broken url in original formula
fi

for pkg in $PACKAGES;
do
    echo "Checking $pkg..."
    brew info "$pkg" | grep 'Not installed' >/dev/null && brew install "$pkg"
done


# Do not install lxml for GitHub Actions
if [[ -z "${CI}" ]]; then
    STATIC_DEPS=true pip3 install lxml
fi
