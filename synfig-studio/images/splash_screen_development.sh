#!/bin/bash

set -x


SRCDIR_RELATIVE=`realpath --relative-to="$PWD" "$2"`

export commit_id=`git log --no-color -1 | head -n 1 | cut -f 2 -d ' ' | cut -c -6`
export commit_date=`git show --pretty=format:%ci HEAD |  head -c 10`
export branch=`git branch -a --no-color --contains HEAD | sed -e s/\*\ // | sed -e s/\(no\ branch\)// | head -n 1 | sed s/^' '//`
export branch=`echo $branch | egrep origin/master > /dev/null && echo master || echo $branch | cut -d ' ' -f 1 | sed -e 's/.*\///'`
sed "s|%branch%|$branch|" "$2/splash_screen_development.sif.in" | sed "s|%commit_date%|$commit_date|" | sed "s|%commit_id%|$commit_id|" | sed "s|synfig_icon.sif#|$SRCDIR_RELATIVE/synfig_icon.sif#|" > "$1"

