#!/bin/bash

set -x


if [ ! -f ./logo_tmp.sif ]; then
	cp -f $2/logo.sif ./logo_tmp.sif
fi

export hash=`git rev-parse --short HEAD`
export commit_date=`git show --pretty=format:%ci HEAD |  head -c 10`
export branch=`git branch -a --no-color --contains HEAD | sed -e s/\*\ // | sed -e s/\(no\ branch\)// | head -n 1 | sed s/^' '//`
export branch=`echo $branch | egrep origin/master > /dev/null && echo master || echo $branch | cut -d ' ' -f 1 | sed -e 's/.*\///'`
sed "s|@GIT_BRANCH@|$branch|" "$2/splash_screen_development.sif.in" | sed "s|@GIT_DATE@|$commit_date|" | sed "s|@GIT_HASH@|$hash|" | sed "s|logo.sif#|logo_tmp.sif#|" > "$1"

