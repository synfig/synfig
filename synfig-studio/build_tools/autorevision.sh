#!/bin/sh

# Copyright 2008 Paul Wise
# Copyright 2009 Konstantin Dmitriev
#
# This package is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This package is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.

get_git_id(){
	export SCM=git
	export REVISION_ID=`cd "$1"; git log --no-color -1 | head -n 1 | cut -f 2 -d ' ' | cut -c -7`
	export BRANCH="`cd "$1"; git branch -a --no-color --contains HEAD | sed -e s/\*\ // | sed -e s/\(no\ branch\)// | tr '\n' ' ' | tr -s ' ' | sed s/^' '//`"
	if ( echo $BRANCH | egrep origin/master > /dev/null ); then
		#give a priority to master branch
		BRANCH='master'
	else
		BRANCH=`echo $BRANCH | cut -d ' ' -f 1`
		BRANCH=${BRANCH##*/}
	fi
	export REVISION=`git show --pretty=format:%ci HEAD |  head -c 10 | tr -d '-'`
	export COMPARE=`git rev-parse --git-dir`
	# The extra * at the end is for Modified
	#REVISION="$REVISION"`cd "$1"; [[ $(git status 2> /dev/null | tail -n1) != "nothing to commit (working directory clean)" ]] && echo "*"`
}

get_git_svn_id(){
	export SCM=git-svn
	export REVISION=`cd "$1"; git svn find-rev HEAD`
	export COMPARE="$1/.git/"
	if [ x = "x$REVISION" ] ; then
		# The extra M at the end is for Modified
		export REVISION=`cd "$1"; git svn find-rev \`git rev-list --max-count=1 --grep='git-svn-id: ' HEAD\``M
	else
		export REVISION="$REVISION"`cd "$1"; git diff --quiet HEAD || echo M`
	fi
}

get_svn_id(){
	export SCM=svn
	export REVISION=`cd "$1"; svnversion || svn info | sed -n 's/^Revision: \(.*\)/\1/p'`
}


HEADER="$2/autorevision.h"
SCM=none


if [ ! -f "$HEADER" ] ; then
	touch -t 197001010101 "$HEADER"
fi


# Extract the revision from SVN/git/etc
if git rev-parse --git-dir > /dev/null  2>&1 ; then
	get_git_id "."
elif [ -d "$1/.git/svn" ] ; then
	get_git_svn_id "$1"
elif [ -d "$1/../.git/svn" ] ; then
	get_git_svn_id "$1/.."
elif [ -d "$1/../../.git/svn" ] ; then
	get_git_svn_id "$1/../.."
elif [ -d "$1/.svn" ] ; then
	COMPARE="$1/.svn"
	get_svn_id "$1"
elif [ -d "$1/_svn" ] ; then
	COMPARE="$1/_svn"
	get_svn_id "$1"
fi


# Allow overriding both REVISION and DEVEL_VERSION
if [ -f "$2/autorevision.conf" ] ; then
	SCM=manual
	. "$2/autorevision.conf"
fi


# Abort if the header is newer
if [ "$COMPARE" -ot "$HEADER" ] ; then exit; fi

# Set the development version string
if [ x = "x$DEVEL_VERSION" ] ; then
	if [ x != "x$REVISION" ] ; then
		if [ $SCM = svn ] ; then
			DEVEL_VERSION="SVN r$REVISION"
		elif [ $SCM = git-svn ] ; then
			DEVEL_VERSION="SVN r$REVISION (via git)"
		elif [ $SCM = git ] ; then
			DEVEL_VERSION="Revision: ${REVISION}\\\\nBranch: ${BRANCH}\\\\nRevision ID: ${REVISION_ID}"
		elif [ $SCM = manual ] ; then
			DEVEL_VERSION="$REVISION (manually configured)"
		fi
	fi
fi


# Output the header
if [ x != "x$DEVEL_VERSION" ] ; then
	printf "#define SHOW_EXTRA_INFO\n" > "$HEADER"
	printf "#define DEVEL_VERSION \"$DEVEL_VERSION\"\n" >> "$HEADER"
fi
