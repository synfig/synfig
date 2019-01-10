#!/usr/bin/env bash

# Define build vars
build_mode="Debug"
make_jobs=1
make_build_command="make -j $make_jobs"
print_build_settings_and_exit="false"

# Define build dirs
cmake_debug_build_dir="_debug"
cmake_release_build_dir="_production"
cmake_build_dir="$cmake_debug_build_dir"
etl_build_dir="etl"
synfig_build_dir="synfig-core"
synfigstudio_build_dir="synfig-studio"
out_dir="out"
synfigstudio_data_prefix=""

# Define commands
env_run_command="synfigstudio"

# Define cmake option strings
cmake_build_type_option="-DCMAKE_BUILD_TYPE=$build_mode"
cmake_prefix_option="-DCMAKE_PREFIX_PATH=${absolute_base_dir}/${cmake_build_dir}/${out_dir}"
cmake_install_prefix_option="-DCMAKE_INSTALL_PREFIX=${absolute_base_dir}/${cmake_build_dir}/${out_dir}"
cmake_cxxflags_option="-DCMAKE_CXX_FLAGS=-I ${absolute_base_dir}/${cmake_build_dir}/${out_dir}/include"
cmake_dataprefix_option=""

build_etl() {
    echo "Building ETL"

    cd ${absolute_base_dir}
    cd "./${cmake_build_dir}/${etl_build_dir}"
    cmake "$cmake_build_type_option" "$cmake_install_prefix_option" ../../ETL/ && $make_build_command && make install

    if [ $? -ne 0 ]
        then
            echo "Failed to build ETL"
            cd "$pwd_dir"
            exit
    fi

    cd ${absolute_base_dir}
    
    echo "Build ETL"
}

build_synfig_core() {
    echo "Building synfig-core"

    cd ${absolute_base_dir}
    cd "./${cmake_build_dir}/${synfig_build_dir}"
    cmake "$cmake_build_type_option" "$cmake_install_prefix_option" "$cmake_cxxflags_option" ../../synfig-core/ && $make_build_command && make install

    if [ $? -ne 0 ]
        then
            echo "Failed to build synfig-core"
            cd "$pwd_dir"
            exit
    fi

    cd ${absolute_base_dir}
    
    echo "Build synfig core"
}

build_synfig_studio() {
    echo "Building synfig-studio"

    cd ${absolute_base_dir}
    cd "./${cmake_build_dir}/${synfigstudio_build_dir}"
    cmake "$cmake_build_type_option" "$cmake_prefix_option" "$cmake_install_prefix_option" "$cmake_cxxflags_option" "$cmake_dataprefix_option" ../../synfig-studio/ && $make_build_command && "$(get_run_cmd_prefix)" make build_images && make install
    
    if [ $? -ne 0 ]
        then
            echo "Failed to build synfig-studio"
            cd "$pwd_dir"
            exit
    fi

    echo "Build synfig-studio"
}

clean_build_dir() {
    echo "Removing old cmake build dir if exist"

    cd ${absolute_base_dir}
    rm -rf "./${cmake_build_dir}/"
    
    echo "Removed old cmake build dir"
}

gen_dir_structure() {
    echo "Construct directory structure"

    cd ${absolute_base_dir}
    mkdir -p "./${cmake_build_dir}/${etl_build_dir}"
    mkdir -p "./${cmake_build_dir}/${synfig_build_dir}"
    mkdir -p "./${cmake_build_dir}/${synfigstudio_build_dir}"
    mkdir -p "./${cmake_build_dir}/${out_dir}"
    
    echo "Constructed directory structure"
}

get_run_cmd_prefix() {
    echo "PATH=\"${absolute_base_dir}/${cmake_build_dir}/${out_dir}/bin:${PATH}\" LD_LIBRARY_PATH=\"${absolute_base_dir}/${cmake_build_dir}/${out_dir}/lib\" XDG_DATA_DIRS=\"${absolute_base_dir}/${cmake_build_dir}/${out_dir}/share:${XDG_DATA_DIRS}\""
}

parse_build_arguments() {
    make_jobs_parameter=$make_jobs
    
    ## Parse arguments
    while [ "$1" != '' ]
        do
            [ $1 == "-d" ] && build_mode="Debug"
            [ $1 == "-r" ] && build_mode="Release"
            [ $1 == "-p" ] && print_build_settings_and_exit="true"
            [ $1 == "-j" ] && make_jobs_parameter=$2 && shift
            [ $1 == "--data-prefix" ] && synfigstudio_data_prefix=$2 && shift
            shift
    done
    
    ## Set cmake build dir
    if [ "$build_mode" == "Debug" ]
        then
            cmake_build_dir="$cmake_debug_build_dir"
        else
            cmake_build_dir="$cmake_release_build_dir"
    fi
    
    # Reset values, which depends from the cmake build dir variable
    cmake_build_type_option="-DCMAKE_BUILD_TYPE=$build_mode"
    cmake_prefix_option="-DCMAKE_PREFIX_PATH=${absolute_base_dir}/${cmake_build_dir}/${out_dir}"
    cmake_install_prefix_option="-DCMAKE_INSTALL_PREFIX=${absolute_base_dir}/${cmake_build_dir}/${out_dir}"
    cmake_cxxflags_option="-DCMAKE_CXX_FLAGS=-I ${absolute_base_dir}/${cmake_build_dir}/${out_dir}/include"
    
    # Set cmake data prefix
    if [ ! $synfigstudio_data_prefix == "" ]
        then
            cmake_dataprefix_option="-DDATA_PREFIX=\"${synfigstudio_data_prefix}\""
    fi
    
    # Check for plausible values in the jobs parameter
    if [ $make_jobs_parameter -ge 1 ] || [ $make_jobs_parameter -le 999 ]
        then
            make_jobs=$make_jobs_parameter
    fi
    
    make_build_command="make -j $make_jobs"
}

print_build_settings() {
    echo "Build mode: $build_mode"
    echo "Build dir: $cmake_build_dir"
    echo "Build jobs: $make_jobs"
    [ ! $cmake_dataprefix_option == "" ] && echo "Custom data prefix: $cmake_dataprefix_option"
    
    if [ $print_build_settings_and_exit == "true" ]
        then
            exit
    fi
}

run_command_in_outenv() {
    if [ "$#" -eq 1 ]
        then
            env_run_command="$1"
    fi

    "$(get_run_cmd_prefix) $env_run_command"
}
