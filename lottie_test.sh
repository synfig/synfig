#!/bin/bash
# Don't abort on Error
# set -e

DIRECTORY=`pwd`
program="$DIRECTORY/synfig-studio/plugins/lottie-exporter/lottie-exporter.py"

# Clone the tests directory
git clone https://gitlab.com/synfig/synfig-tests.git

test_files="$DIRECTORY/synfig-tests/export/lottie/*.sif"

for filename in $test_files;
do 
    pass=1
    python3 $program $filename || { # This line catches errors in python script
        ((Error_count=Error_count+1))
        ((pass=0))
    }

    # To display whether a file ran successfully or not
    if [ $pass -eq 1 ]
    then
        echo "[OK] $filename";
    else
        echo "[FAIL] $filename";
    fi
done
echo -e "\nFiles failed to export: $Error_count\n"

if [ $Error_count > 0 ]
    then
        exit 1
    else
        exit 0
fi
