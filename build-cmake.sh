#!/usr/bin/env bash
#
# = Usage: =
#    ./build-cmake.sh

#Define dir paths
pwd_dir="$PWD"
absolute_script_path="$(readlink -f "$0")"
absolute_base_dir="$(dirname "$absolute_script_path")"

cd ${absolute_base_dir}

# Include build folder names and build functions
source ./autobuild/build-cmake-common.sh

if [ $? -ne 0 ]
    then
        echo "Failed to include: ./autobuild/build-cmake-common.sh"
        cd "$pwd_dir"
        exit
fi

# Exec build steps
clean_build_dir

gen_dir_structure

build_etl

build_synfig_core

build_synfig_studio

echo "Build successfull"
cd "$pwd_dir"
