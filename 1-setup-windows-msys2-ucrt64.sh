#!/usr/bin/env bash

set -e

err() {
	echo "ERROR: $*" >&2
}

# Verify environment
if [[ -z "${MSYSTEM:-}" ]]; then
	err "Run this script inside MSYS2."
    exit 1
fi

MINGW_PACKAGE_PREFIX="${MINGW_PACKAGE_PREFIX:-mingw-w64-ucrt-x86_64}"

echo "==> Using MSYSTEM=${MSYSTEM}"
echo "==> Using MINGW_PACKAGE_PREFIX=${MINGW_PACKAGE_PREFIX}"
echo

echo "==> Updating package databases and core system..."
pacman -Syu --noconfirm \
	|| {
		err "ERROR: Package update failed"
		exit 1
	}

echo
echo "==> Installing MSYS tools (tar, patch, optional automake-wrapper)..."
pacman -S --needed --noconfirm \
    tar \
    patch \
    automake-wrapper \
	|| {
		err "Could not install tools"
		exit 1
	}

echo
echo "==> Installing toolchain and build tools..."
yes "" | \
pacman -S --needed --noconfirm \
    ${MINGW_PACKAGE_PREFIX}-toolchain \
	${MINGW_PACKAGE_PREFIX}-cmake \
    ${MINGW_PACKAGE_PREFIX}-git \
    ${MINGW_PACKAGE_PREFIX}-ninja \
    ${MINGW_PACKAGE_PREFIX}-pkgconf \
    || {
		err "Could not install toolchain or build tools"
		exit 1
	}

echo
echo "==> Installing Synfig dependencies ..."
pacman -S --needed --noconfirm \
    ${MINGW_PACKAGE_PREFIX}-cairo \
    ${MINGW_PACKAGE_PREFIX}-dlfcn \
    ${MINGW_PACKAGE_PREFIX}-ffmpeg \
    ${MINGW_PACKAGE_PREFIX}-fftw \
    ${MINGW_PACKAGE_PREFIX}-freetype \
    ${MINGW_PACKAGE_PREFIX}-gettext \
    ${MINGW_PACKAGE_PREFIX}-gtk3 \
    ${MINGW_PACKAGE_PREFIX}-harfbuzz \
    ${MINGW_PACKAGE_PREFIX}-imagemagick \
    ${MINGW_PACKAGE_PREFIX}-libjpeg-turbo \
    ${MINGW_PACKAGE_PREFIX}-libltdl \
    ${MINGW_PACKAGE_PREFIX}-libmng \
    ${MINGW_PACKAGE_PREFIX}-libpng \
    ${MINGW_PACKAGE_PREFIX}-libsigc++ \
    ${MINGW_PACKAGE_PREFIX}-libxml++ \
    ${MINGW_PACKAGE_PREFIX}-libxml2 \
    ${MINGW_PACKAGE_PREFIX}-mlt \
    ${MINGW_PACKAGE_PREFIX}-pango \
    ${MINGW_PACKAGE_PREFIX}-python-lxml \
    ${MINGW_PACKAGE_PREFIX}-shared-mime-info \
	|| {
		err "ERROR: Could not install Synfig dependencies"
		exit 1
	}

# Fix cursor 'color-picker' that is missing from the distro. 
# This cursor is needed in a gtk widget that Synfig calls
# If gtk can't find it, it bombs and takes down Synfig with it.
	CURSOR_LOCATION="${MINGW_PREFIX}/share/icons/Adwaita/cursors"
	if [[ ! -f "$CURSOR_LOCATION/color-picker.cur" ]]
		cp "$CURSOR_LOCATION/crosshair.cur" "$CURSOR_LOCATION/color-picker.cur" \
		|| {
			err "ERROR: Could not provide the cursor 'color_picker'"
			exit 1
		}
	fi

# Under Windows, JACK doesn't need to be installed.

# Special case: The librara libxml++-2.6 is that thoroughly outdated that 
# current servers dropped it years ago. Synfig still relies on it.
# Let's just hope it's already installed, from some bygone era when that  
# old chestnur version was still commonly in use.
if pacman -Q ${MINGW_PACKAGE_PREFIX}-libxml++-2.6 >/dev/null 2>&1 ; then
	echo "Package libxml++-2.6 is installed locally, using this copy"
elif pacman -S --needed --noconfirm ${MINGW_PACKAGE_PREFIX}-libxml++-2.6 ; then
	:
else
# 	Aw shucks, it isn't installed and can't be downloaded. Seems you're effed then, sorry.
	err "ERROR: libxml++-2.6 isn't installed, nor on the servers"
	err "Procure it somehow, Synfig still relies on it."
	exit 1
fi

echo
echo "==> Done. "