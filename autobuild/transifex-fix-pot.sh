#!/bin/sh

if [ -z "$1" ]; then
	echo "ERROR: Please specify .pot file"
	exit 1
fi

PKGNAME="Package"
if [[ "$1" == "synfig.pot" ]]; then
	PKGNAME="Synfig Core"
fi
if [[ "$1" == "synfigstudio.pot" ]]; then
	PKGNAME="Synfig Studio"
fi

YEAR=`date +%Y`

sed -i "/# SOME DESCRIPTIVE TITLE/c\# Translation file for $PKGNAME package." "$1" || true
sed -i "/# Copyright (C) YEAR/c\# Copyright (C) $YEAR Synfig Contributors" "$1" || true
sed -i "/# This file is distributed under the same license as the PACKAGE package./c\# This file is distributed under the same license as the $PKGNAME package." "$1" || true
