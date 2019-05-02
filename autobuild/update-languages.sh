#!/bin/bash

set -e

update_linguas() {
    local DIR="$1"
    local LINGUAS_FILE="$DIR/LINGUAS"
    echo "update $LINGUAS_FILE"
    ls -1 "$DIR"/*.po | while read FILE; do
        basename -s .po "$FILE";
    done > "$LINGUAS_FILE"
}

firstword() {
    echo "$1"
}

read_linguas() {
    local FILE="$1"
    echo " - read $FILE"
    while read LINE; do
        local CODE="$(firstword $LINE)"
        if [ ! -z "$CODE" ]; then
            CODE_MAP[$CODE]="$CODE"
            NAME_MAP[$CODE]=""
            COMMENT_MAP[$CODE]=" <------- TODO: Set language name and remove this comment"
        else
            echo "$LINE" >> "$TMPFILE"
        fi
    done < "$FILE"
}

update_languages_inc() {
    local LANGUAGES_FILE="synfig-studio/src/languages.inc.c"
    echo "update LANGUAGES_FILE"

    local TMPFILE="$LANGUAGES_FILE.process"
    rm -f "$TMPFILE"

    declare -A CODE_MAP
    declare -A NAME_MAP
    declare -A COMMENT_MAP

    read_linguas "synfig-core/po/LINGUAS"
    read_linguas "synfig-studio/po/LINGUAS"

    echo " - read $LANGUAGES_FILE"
    while read LINE; do
        local COMMENT="$(echo "$LINE" | sed "s|//|#|" | cut -d \# -f 2- -s)"
        local SUBLINE="$(echo "$LINE" | sed "s|//|#|" | cut -d \# -f 1)"
        local CODE="$(echo "$SUBLINE" | cut -d \" -f 2)"
        local NAME="$(echo "$SUBLINE" | cut -d \" -f 4)"
        if [ ! -z "$CODE" ] || [ ! -z "$NAME" ]; then
            CODE_MAP[$CODE]="$CODE"
            NAME_MAP[$CODE]="$NAME"
            COMMENT_MAP[$CODE]="$COMMENT"
        else
            # write all unknown rows to beginning of file
            echo "$LINE" >> "$TMPFILE"
        fi
    done < "$LANGUAGES_FILE"

    # fit the columns to make beautifulest table in the world
    local CODE_LEN=0
    local NAME_LEN=0
    for i in ${!CODE_MAP[@]}; do
        if [ ${#CODE_MAP[$i]} -gt $CODE_LEN ]; then
            CODE_LEN=${#CODE_MAP[$i]}
        fi
        if [ ${#NAME_MAP[$i]} -gt $NAME_LEN ]; then
            NAME_LEN=${#NAME_MAP[$i]}
        fi
    done
    CODE_LEN=$((CODE_LEN+3))
    NAME_LEN=$((NAME_LEN+3))

    # sort
    SORTED=$(for i in ${!CODE_MAP[@]}; do echo $i; done | sort | while read l; do echo -n "$l "; done)

    # write languages
    NEW_LANGS=
    for i in $SORTED; do
        CODE=${CODE_MAP[$i]}
        NAME=${NAME_MAP[$i]}
        COMMENT=${COMMENT_MAP[$i]}
        if [ ! -z "$COMMENT" ]; then
            COMMENT=" // $COMMENT"
        fi
        if [ -z "$NAME" ]; then
            NEW_LANGS="$NEW_LANGS $CODE"
        fi
        printf "{ %-${CODE_LEN}s %-${NAME_LEN}s },%s\\n" "\"$CODE\"," "\"$NAME\"," "$COMMENT" >> "$TMPFILE"
    done

    # update file
    mv $TMPFILE $LANGUAGES_FILE

    if [ ! -z "$NEW_LANGS" ]; then
        echo ""
        echo "please, enter names for following languages:$NEW_LANGS"
        echo ""
        echo "see file $LANGUAGES_FILE"
        echo ""
    fi
}


cd "$(cd `dirname "$0"`; pwd)/.."

update_linguas "synfig-core/po"
update_linguas "synfig-studio/po"
update_languages_inc
echo "done"
