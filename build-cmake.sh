#!/usr/bin/env bash
#
# = Usage: =
#    ./build-cmake.sh [OPTIONS]
#
# OPTIONS
# -d = Debug build (standard)
# -r = Release build
# -j NUMBER = Set parallel make jobs (1 <= NUMBER <= 999) (standard is 1)
# -p = Only print out the current active build settings and exit
#
# = Examples =
# Make a debug build
#    ./build-cmake -d
#
# Make a release build
#    ./build-cmake -r
#
# Make a release build with two parallel make jobs
#    ./build-cmake -r -j 2

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

# Parse build options
parse_build_arguments "$@"

# Print build settings
print_build_settings

# Exec build steps
clean_build_dir

gen_dir_structure

build_etl

build_synfig_core

build_synfig_studio

echo "Build successfull"
cd "$pwd_dir"
