#!/bin/bash
# Abort on Error
set -e

DIRECTORY=`pwd`
program="$DIRECTORY/synfig-studio/plugins/lottie-exporter/lottie-exporter.py"

# Clone the tests directory
git clone https://gitlab.com/synfig/synfig-tests.git

test_files="$DIRECTORY/synfig-tests/export/lottie/*.sif"
for filename in $test_files;
do 
    echo $filename;
    python3 $program $filename;
done
