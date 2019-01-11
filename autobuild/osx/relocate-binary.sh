#!/bin/bash

set -e
#set -x

[ -z "$2" ] && echo "usage: \"$0\" <file> <source-prefix> <destination-dir>" && echo "This utility relocates binary <file> from <source-prefix> to <destonation-dir> together with all dependencies and adds rpath support." && exit 1

readlink_f() {
	TARGET_FILE=$1

	cd `dirname $TARGET_FILE`
	TARGET_FILE=`basename $TARGET_FILE`

	# Iterate down a (possible) chain of symlinks
	while [ -L "$TARGET_FILE" ]
	do
		TARGET_FILE=`readlink $TARGET_FILE`
		cd `dirname $TARGET_FILE`
		TARGET_FILE=`basename $TARGET_FILE`
	done

	# Compute the canonicalized name by finding the physical path 
	# for the directory we're in and appending the target file.
	PHYS_DIR=`pwd -P`
	RESULT=$PHYS_DIR/$TARGET_FILE
	echo $RESULT
}

remove_prefix() {
	local INPUT="$1"
	local OUTPUT=""
	if [[ "$INPUT" == /usr/local/Cellar/* ]]; then
		TMP="${INPUT#*/}" # usr/local/Cellar/pkgname/pkgversion/path
		TMP="${TMP#*/}"  # local/Cellar/pkgname/pkgversion/path
		TMP="${TMP#*/}"  # Cellar/pkgname/pkgversion/path
		TMP="${TMP#*/}"  # pkgname/pkgversion/path
		TMP="${TMP#*/}"  # pkgversion/path
		TMP="${TMP#*/}"  # path
		OUTPUT="${TMP}"
	elif [[ "$INPUT" == /usr/local/opt/* ]]; then
		TMP="${INPUT#*/}" # usr/local/opt/pkgname/path
		TMP="${TMP#*/}"  # local/opt/pkgname/path
		TMP="${TMP#*/}"  # opt/pkgname/path
		TMP="${TMP#*/}"  # pkgname/path
		TMP="${TMP#*/}"  # path
		OUTPUT="${TMP}"
	elif [[ "$INPUT" == /usr/local/lib/* ]]; then
		TMP="${INPUT#*/}" # usr/local/lib/file
		TMP="${TMP#*/}"  # local/lib/file
		TMP="${TMP#*/}"  # lib/file = path
		OUTPUT="${TMP}"
	elif [[ "$INPUT" == /usr/local/share/* ]]; then
		TMP="${INPUT#*/}" # usr/local/share/file
		TMP="${TMP#*/}"  # local/share/file
		TMP="${TMP#*/}"  # share/file = path
		OUTPUT="${TMP}"
	elif [[ "$INPUT" == $PREFIX* ]]; then
		#LINE2=`echo "${INPUT}" | cut -c1-$PREFIXLEN`
		OUTPUT="${INPUT:$PREFIXLEN}"
	fi
	echo "$OUTPUT"
}

process_lib() {

local FILESRC="$1"
local FILEDEST_SHORT="$2"

#local FILE_PARENT="$2"



	local FILEDEST="${DEST}/${FILEDEST_SHORT}"

	if [ ! -f "${FILEDEST}" ]; then
		
		
		if [ ! -d `dirname "${FILEDEST}"` ]; then
			mkdir -p `dirname "${FILEDEST}"`
		fi
		REALPATH=`readlink_f "${FILESRC}"`
		cp "${REALPATH}" "${FILEDEST}"
		chmod a+rw "$FILEDEST"
		
		
		install_name_tool -add_rpath ./ "$FILEDEST" > /dev/null 2>&1 || true
		
		
		echo "Relinking ${FILEDEST_SHORT} ..."
		local FIRST=true
		local LINE=
		local LINE2=
		otool -L "${FILEDEST}" | while read -r LINE; do
			#echo "$LINE"
			if $FIRST; then
				FIRST=false
			else
				LINE=`echo "$LINE" | sed -e 's/^[ \t]*//' | sed -e 's/ \(.*\)$//'`
		    
				# make sure file isn't referencing itself
				#A=$(basename "$FILEDEST")
				#B=$(basename "$LINE")
				if [ ! "$LINE" == "$FILESRC" ]; then
					LINE2=`remove_prefix "$LINE"`
					if [ ! -z "$LINE2" ]; then
					#if [ ! -z "$FILE_PARENT" ]; then
						#echo "   install_name_tool -change \"$LINE\" \"@rpath/$LINE2\" \"$FILEDEST\""
						install_name_tool -change "$LINE" "@rpath/$LINE2" "$FILEDEST"
						process_lib "$LINE" "$LINE2"
					#fi
					
					fi
				fi
            	
			fi
    	done
		#echo "... finished ${FILEDEST_SHORT}"
	fi


}

#scan "$BASE_FILE"
#scan "$BASE_FILE" subscan

#TARGET_FILE=`readlink_f "$1"`
TARGET_FILE="$1"
echo "Gathering deps for: $TARGET_FILE"

export DEST="$3"
#echo "DESTINATION: $DEST"

export PREFIX="$2"
export PREFIXLEN=${#PREFIX}

if [ ! -f "${TARGET_FILE}" ]; then
	echo "ERROR: File not found."
	exit 1
fi

FILEDEST_SHORT=`remove_prefix "${TARGET_FILE}"`

if [[ "${TARGET_FILE}" == ${PREFIX}* ]]; then
	process_lib "${TARGET_FILE}" "${FILEDEST_SHORT}"
fi

echo "Success."
echo ""

