#!/usr/bin/env bash
#
# = Usage: =
#    ./run-cmake.sh [-r] [command]
#
# Options
# -r = Use the release build (standard is debug build)
#
# = Examples: =
# Run Synfig Studio debug build
#    ./run-cmake.sh
# Run Synfig Studio release build
#    ./run-cmake.sh -r
# Run Synfig CLI (debug build) with parameters
#    ./run-cmake.sh "synfig --help-all"
# Create a bash session for running the portable build (debug build)
#    ./run-cmake.sh bash

#Define dir paths
pwd_dir="$PWD"
absolute_script_path="$(readlink -f "$0")"
absolute_base_dir="$(dirname "$absolute_script_path")"

cd ${absolute_base_dir}

# Include build folder names and run function
source ./autobuild/build-cmake-common.sh

if [ "$1" == "-r" ]
    then
        cmake_build_dir="$cmake_release_build_dir"
        shift
    else
        cmake_build_dir="$cmake_debug_build_dir"
fi

if [ ! -d "./${cmake_build_dir}/${out_dir}" ]
    then
        echo "Output directory not found!"
        echo "You have to run build-cmake.sh first!"
        exit
fi

if [ "$#" -eq 1 ]
    then
        current_run_cmd="$1"
        run_command_in_outenv "$current_run_cmd"
    else
        run_command_in_outenv
fi
