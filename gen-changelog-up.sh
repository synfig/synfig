#!/bin/bash

# https://stackoverflow.com/questions/59895/how-to-get-the-source-directory-of-a-bash-script-from-within-the-script-itself
SOURCE="${BASH_SOURCE[0]}"
DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
CHANGELOG="${DIR}/ChangeLog.md"

START="$1"
END="$2"

# Iterate each commit from given range (1.4.x branch)
COMMITS=`git log ${START}...${END} --pretty=format:'%H'`
if [ ! -z "${COMMITS}" ]; then
while IFS= read -r CMT; do
    TAGS=""
    # Get list of changed files
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
    MESSAGE="${MESSAGE}${TAGS}"
    echo "----- ${MESSAGE}"
    # Check if commit with same name is exists in ChangeLog.md (and only once)
    RESULT=`cat "${CHANGELOG}" | grep "${MESSAGE}"`
    LINES_COUNT=`echo "${RESULT}" | wc -l`
    if [[ ${LINES_COUNT} == 1 ]] && [ ! -z "${RESULT}" ]; then
	echo "${RESULT}"
    else
	echo "-----"
	echo "Not OK"
    fi
    

done <<< "$COMMITS"
fi
