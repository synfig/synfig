#!/bin/bash
# Don't abort on Error
# set -e

WORKDIR=`dirname "$0"`
pushd "${WORKDIR}" > /dev/null
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
NC='\033[0m'    # no color

for filename in $test_files;
do 
    pass=1
    python3 $LOTTIE $filename || { # This line catches errors in python script
        ((Error_count=Error_count+1))
        ((pass=0))
    }

    # To display whether a file ran successfully or not
    if [ $pass -eq 1 ]
    then
        echo -e "${GREEN}[OK]${NC} $filename";
    else
        echo -e "${RED}[FAIL]${NC} $filename";
    fi
done
echo -e "\nFiles failed to export: $Error_count\n"

if [ $Error_count > 0 ]
    then
        exit 1
    else
        exit 0
fi
