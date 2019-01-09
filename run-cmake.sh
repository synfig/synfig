#!/usr/bin/env bash
#
# = Usage: =
#    ./run-cmake.sh [command]
#
# = Examples: =
#
# == Standard mode ==
# Run Synfig Studio
#    ./run-cmake.sh
# Run Synfig CLI with parameters
#    ./run-cmake.sh "synfig --help-all"
# Create a bash session for running the portable build
#    ./run-cmake.sh bash

#Define dir paths
pwd_dir="$PWD"
absolute_script_path="$(readlink -f "$0")"
absolute_base_dir="$(dirname "$absolute_script_path")"

cd ${absolute_base_dir}

# Include build folder names and run function
source ./autobuild/build-cmake-common.sh

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
