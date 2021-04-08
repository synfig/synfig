#!/bin/bash


# https://stackoverflow.com/questions/59895/how-to-get-the-source-directory-of-a-bash-script-from-within-the-script-itself
SOURCE="${BASH_SOURCE[0]}"
DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
CHANGELOG="${DIR}/ChangeLog-development.md"

if [ -z "$1" ]; then
    # Read starting commit ID from ChangeLog.last_id
    if [ -f "${DIR}/ChangeLog.last_id" ]; then
	START=`cat "${DIR}/ChangeLog.last_id" | head -n1 | awk '{print $1;}'`
    else
	echo "ERROR: Please specify starting commit id as first argument."
	exit 1
    fi
else
    START="$1"
fi

#START="upstream/changelog"

# Get list of all commits
COMMITS=`git log ${START}...master --pretty=format:'%H' --reverse`

if [ -f "${CHANGELOG}" ]; then
    tac "${CHANGELOG}" > "${CHANGELOG}.new"
fi

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
    MESSAGE=`echo "$MESSAGE" | sed -e "s/#\([0-9]\+\)/\[\\\#\1\]\(https\:\/\/github\.com\/synfig\/synfig\/issues\/\1\)/g"`
    echo "- [${CMT:0:7}](https://github.com/synfig/synfig/commit/${CMT}) ${MESSAGE}${TAGS}" >> "${CHANGELOG}.new"
done <<< "$COMMITS"
fi
#Save last commit ID
git rev-parse master > "${DIR}/ChangeLog.last_id"

tac "${CHANGELOG}.new" > "${CHANGELOG}"
rm "${CHANGELOG}.new" ||  true

