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
YELLOW='\033[0;33m'
no_color='\033[0m'

Error_count=0
Support_soon=0

# Files which are allowed to fail
failing_files=(070-skeleton.sif 075-skeleton-group.sif 076-skeleton-group-animated.sif 079-skeleton-animated-spline.sif 082-converters-scale-link.sif 083-converters-scale-link-animated.sif)

for filename in $test_files;
do 
    pass=1      # 0 -> File is failing to export unexpectedly, 1 -> File is exporting, 2 -> File is allowed to fail
    python3 $LOTTIE $filename || { # This line catches errors in python script

        # Check if the program is allowed to fail
        for i in ${failing_files[@]}
        do
            if [ $i = `basename $filename` ]
            then
                ((Support_soon=Support_soon+1))
                pass=2
                break
            fi
        done

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
