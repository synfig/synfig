#!/bin/bash


# https://stackoverflow.com/questions/59895/how-to-get-the-source-directory-of-a-bash-script-from-within-the-script-itself
SOURCE="${BASH_SOURCE[0]}"
DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
CHANGELOG="${DIR}/ChangeLog-development.md"

if [ -z "$1" ]; then
    # Read starting commit ID from ChangeLog.md
    START=`cat "${CHANGELOG}" | head -n1 | awk '{print $1;}'`
else
    START="$1"
fi

#START="upstream/changelog"

# Get list of all commits
COMMITS=`git log ${START}...master --pretty=format:'%H' --reverse`

tac "${CHANGELOG}" > "${CHANGELOG}.new"

if [ ! -z "${COMMITS}" ]; then
while IFS= read -r CMT; do
    TAGS=""
    # Get list of changed files
    #echo "... ${CMT} ..."
    FILES=`git diff-tree --no-commit-id --name-only -r ${CMT}`
    while IFS= read -r FILENAME; do
	# check if filename starts with "ETL" "synfig-core" or "synfig-studio"
	if [[ $FILENAME == ETL/* ]]; then
	    if [[ $TAGS != *"[ETL]"* ]]; then
		TAGS="$TAGS [ETL]"
	    fi
	elif [[ $FILENAME == synfig-core/* ]]; then
	    if [[ $TAGS != *"[core]"* ]]; then
		TAGS="$TAGS [core]"
	    fi
	elif [[ $FILENAME == synfig-studio/* ]]; then
	    if [[ $TAGS != *"[studio]"* ]]; then
		TAGS="$TAGS [studio]"
	    fi
	fi
	#echo "...     ${FILENAME}"
    done <<< "$FILES"
    MESSAGE=`git log --format=%B -n 1 ${CMT} | head -n 1`
    echo "${CMT} ${MESSAGE}${TAGS}" >> "${CHANGELOG}.new"
done <<< "$COMMITS"
fi

tac "${CHANGELOG}.new" > "${CHANGELOG}"
rm "${CHANGELOG}.new" ||  true
