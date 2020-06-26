#!/bin/bash
# Don't abort on Error
# set -e

WORKDIR=`dirname "$0"`
pushd "${WORKDIR}/.." > /dev/null
WORKDIR=`pwd`
popd > /dev/null

# Python program to run lottie
LOTTIE="${WORKDIR}/synfig-studio/plugins/lottie-exporter/lottie-exporter.py"

# Clone the tests directory
git clone https://gitlab.com/synfig/synfig-tests.git
test_files="${WORKDIR}/synfig-tests/export/lottie/*.sif"

# Color escape codes in bash
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
no_color='\033[0m'

Error_count=0
Support_soon=0

for filename in $test_files;
do 
    pass=1      # 0 -> File is failing to export unexpectedly, 1 -> File is exporting, 2 -> File is allowed to fail
    python3 $LOTTIE $filename "${filename%.sif}.json" || { # This line catches errors in python script

        # Check if the current file is allowed to fail[ATF]
        if [[ `basename $filename` == *"ATF"* ]]; then
            ((Support_soon=Support_soon+1))
            pass=2
        fi

        # If it is not allowed to fail, then increase the number of errored files
        if [ $pass -eq 1 ]
        then
            ((Error_count=Error_count+1))
            ((pass=0))
        fi
    }

    # To display whether a file ran successfully or not
    if [ $pass -eq 1 ]
    then
        echo -e "${GREEN}[OK]${no_color} $filename";
    elif [ $pass -eq 2 ]
    then
        echo -e "${YELLOW}[SUPPORT SOON]${no_color} $filename";
    else
        echo -e "${RED}[FAIL]${no_color} $filename";
    fi
done
echo -e "\nFiles failed to export: $Error_count"
echo -e "Files failed, but the failure is expected: $Support_soon\n"

if [ $Error_count -gt 0 ]
    then
        exit 1  # Job should fail
    else
        exit 0  # Job should pass
fi
