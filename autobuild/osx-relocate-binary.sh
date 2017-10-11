#!/bin/bash

set -e

[ -z "$3" ] && echo "usage: \"$0\" <file> <source-prefix> <destination-dir>" && echo "This utility relocates binary <file> from <source-prefix> to <destonation-dir> together with all dependencies and adds rpath support." && exit 1

PREFIX="$2"
PREFIXLEN=${#PREFIX}

DEST="$3"

process_lib() {

FILESRC=$1
if [[ "${FILESRC}" == ${PREFIX}* ]]; then
	#FILEDEST_SHORT=`echo "${FILESRC}" | cut -c1-$PREFIXLEN`
	local FILEDEST_SHORT="${FILESRC:$PREFIXLEN}"
	local FILEDEST="${DEST}/${FILEDEST_SHORT}"
	
	if [ ! -f "${FILEDEST}" ]; then
		
		
		if [ ! -d `dirname "${FILEDEST}"` ]; then
			mkdir -p `dirname "${FILEDEST}"`
		fi
		cp "${FILESRC}" "${FILEDEST}"
		chmod a+rw "$FILEDEST"
		install_name_tool -add_rpath ./ "$FILEDEST" > /dev/null 2>&1 || true
		
		
		echo "Relinking ${FILEDEST_SHORT} ..."
		local FIRST=true
		local LINE=
		local LINE2=
		otool -L "${FILEDEST}" | while read -r LINE; do

			if $FIRST; then
				FIRST=false
			else
 		       	LINE=`echo "$LINE" | sed -e 's/^[ \t]*//' | sed -e 's/ \(.*\)$//'`
            
            	# make sure file isn't referencing itself
            	#A=$(basename "$FILEDEST")
            	#B=$(basename "$LINE")
            	if [ ! "$LINE" == "$FILESRC" ] && [[ "$LINE" == $PREFIX* ]]; then
            		#LINE2=`echo "${LINE}" | cut -c1-$PREFIXLEN`
            		LINE2="${LINE:$PREFIXLEN}"
            		install_name_tool -change "$LINE" "@rpath/$LINE2" "$FILEDEST"
            		process_lib "$LINE"
            	fi
            	
			fi
    	done
		#echo "... finished ${FILEDEST_SHORT}"
	fi
fi

}

echo "Gathering deps for: $1"

#scan "$BASE_FILE"
#scan "$BASE_FILE" subscan

process_lib "$1"

echo "Success."
echo ""

