#!/usr/bin/env bash
#
# This file is required by build-cmake.sh and shouldn't be called directly.
# It contains the functions and variable definitions which are needed
# for the build process.

# Define standard build settings
build_mode="Debug"
make_jobs=1
print_build_settings_and_exit="false"
write_portable_run_code="true"
synfigstudio_data_prefix=""
incremental_build="false"

# Define build dirs
cmake_debug_build_dir="_debug"
cmake_release_build_dir="_production"
cmake_build_dir="$cmake_debug_build_dir"
etl_build_dir="etl"
synfig_build_dir="synfig-core"
synfigstudio_build_dir="synfig-studio"
out_dir="out"

# Define commands
env_run_command="synfigstudio"
make_build_command="make -j $make_jobs"

# Define cmake option strings
cmake_build_type_option="-DCMAKE_BUILD_TYPE=$build_mode"
cmake_prefix_option="-DCMAKE_PREFIX_PATH=${absolute_base_dir}/${cmake_build_dir}/${out_dir}"
cmake_install_prefix_option="-DCMAKE_INSTALL_PREFIX=${absolute_base_dir}/${cmake_build_dir}/${out_dir}"
cmake_cxxflags_option="-DCMAKE_CXX_FLAGS=-I ${absolute_base_dir}/${cmake_build_dir}/${out_dir}/include"
cmake_dataprefix_option=""

build_etl() {
    echo "Building ETL"

    # Navigate to target build dir
    cd ${absolute_base_dir}
    cd "./${cmake_build_dir}/${etl_build_dir}"
    
    # Configure, make and install
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

    # Navigate to target build dir
    cd ${absolute_base_dir}
    cd "./${cmake_build_dir}/${synfig_build_dir}"
    
    # Configure, make and install
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

    # Navigate to target build dir
    cd ${absolute_base_dir}
    cd "./${cmake_build_dir}/${synfigstudio_build_dir}"
    
    # Configure make command
    synfig_studio_make_cmd="make build_images"
    
    # Don't rerender the images if incremental is activated
    if [ "$incremental_build" == "true" ]
        then
            synfig_studio_make_cmd="make synfigstudio"
    fi
    
    # Configure, make and install
    cmake "$cmake_build_type_option" "$cmake_prefix_option" "$cmake_install_prefix_option" "$cmake_cxxflags_option" "$cmake_dataprefix_option" ../../synfig-studio/ && $make_build_command && run_command_in_outenv "$synfig_studio_make_cmd" && make install
    
    if [ $? -ne 0 ]
        then
            echo "Failed to build synfig-studio"
            cd "$pwd_dir"
            exit
    fi

    echo "Build synfig-studio"
}

clean_build_dir() {
    if [ "$incremental_build" == "false" ]
        then
            echo "Removing old cmake build dir if exist"

            cd ${absolute_base_dir}
            rm -rf "./${cmake_build_dir}/"
            
            echo "Removed old cmake build dir"
    fi
}

gen_dir_structure() {
    if [ "$incremental_build" == "false" ]
        then
            echo "Construct directory structure"

            cd ${absolute_base_dir}
            mkdir -p "./${cmake_build_dir}/${etl_build_dir}"
            mkdir -p "./${cmake_build_dir}/${synfig_build_dir}"
            mkdir -p "./${cmake_build_dir}/${synfigstudio_build_dir}"
            mkdir -p "./${cmake_build_dir}/${out_dir}"
            
            echo "Constructed directory structure"
    fi
}

get_run_cmd_prefix() {
    echo "PATH=\"${absolute_base_dir}/${cmake_build_dir}/${out_dir}/bin:${PATH}\" LD_LIBRARY_PATH=\"${absolute_base_dir}/${cmake_build_dir}/${out_dir}/lib\" XDG_DATA_DIRS=\"${absolute_base_dir}/${cmake_build_dir}/${out_dir}/share:${XDG_DATA_DIRS}\""
}

parse_build_arguments() {
    make_jobs_parameter=$make_jobs
    
    ## Parse arguments and set variables (if an option was found)
    while [ "$1" != '' ]
        do
            [ $1 == "-d" ] && build_mode="Debug"
            [ $1 == "-r" ] && build_mode="Release"
            [ $1 == "-p" ] && print_build_settings_and_exit="true"
            [ $1 == "-n" ] && write_portable_run_code="false"
            [ $1 == "-i" ] && incremental_build="true"
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
            cmake_dataprefix_option="-DDATA_PREFIX=${synfigstudio_data_prefix}"
    fi
    
    # Check for plausible values in the jobs parameter
    if [ $make_jobs_parameter -ge 1 ] || [ $make_jobs_parameter -le 999 ]
        then
            make_jobs=$make_jobs_parameter
    fi
    
    # Set custom make command
    make_build_command="make -j $make_jobs"
}

print_build_settings() {
    echo "Build mode: $build_mode"
    echo "Build dir: $cmake_build_dir"
    echo "Build jobs: $make_jobs"
    echo "Incremental build: $incremental_build"
    [ ! $synfigstudio_data_prefix == "" ] && echo "Custom data prefix: $synfigstudio_data_prefix"
    
    # If script was started with '-p' stop the script here
    if [ $print_build_settings_and_exit == "true" ]
        then
            exit
    fi
}

run_command_in_outenv() {
    # Check parameters count
    if [ "$#" -eq 1 ]
        then
            env_run_command="$1"
    fi

    # Set env and run command
    bash -c "export $(get_run_cmd_prefix) && $env_run_command"
}

write_portable_run_code() {
    # Only write, if it is enabled
    if [ $write_portable_run_code == "true" ]
        then
            # Navigate to out dir
            cd ${absolute_base_dir}
            cd "./${cmake_build_dir}/${out_dir}"

            file_name="run-portable.sh"
            filepath_write_target="./${file_name}"
            
            echo "Writing runcode to: ${absolute_base_dir}/${cmake_build_dir}/${out_dir}/${file_name}"
            
            # Define the bash script code, which runs Synfig Studio
            portable_run_code_line_01="#!/usr/bin/env bash"
            portable_run_code_line_02="# Usage:"
            portable_run_code_line_03="#   $filepath_write_target [command]"
            portable_run_code_line_04="#   or $filepath_write_target \"[command] [args]\""
            portable_run_code_line_05="#"
            portable_run_code_line_06="# Where:"
            portable_run_code_line_07="#   command = Command to exec in the out env (Default: \"synfigstudio\")"
            portable_run_code_line_08="#   args = Parameters passed to the command (Default: empty)"
            portable_run_code_line_09="# "
            portable_run_code_line_10="# Examples:"
            portable_run_code_line_11="#   Run Synfig Studio:"
            portable_run_code_line_12="#     $filepath_write_target"
            portable_run_code_line_13="#   Get help of the Synfig CLI program:"
            portable_run_code_line_14="#     $filepath_write_target \"synfig --help\""
            portable_run_code_line_15="#   Get bash env to use the Synfig tools directly"
            portable_run_code_line_16="#     $filepath_write_target bash"
            portable_run_code_line_17="#       Now you can type synfig or synfigstudio directly to run them"
            portable_run_code_line_18="#       You can type \"exit\" to end this session"
            portable_run_code_line_19=""
            portable_run_code_line_20="run_command_string=\"\$1\""
            portable_run_code_line_21="[ \"\$1\" == '' ] && run_command_string=synfigstudio"
            portable_run_code_line_22=""
            portable_run_code_line_23="$(get_run_cmd_prefix) \$run_command_string"
            portable_run_code_line_24=""
            
            # Write out the code (line by line).
            echo -e "$portable_run_code_line_01" > "$filepath_write_target"
            echo -e "$portable_run_code_line_02" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_03" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_04" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_05" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_06" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_07" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_08" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_09" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_10" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_11" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_12" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_13" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_14" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_15" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_16" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_17" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_18" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_19" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_20" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_21" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_22" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_23" >> "$filepath_write_target"
            echo -e "$portable_run_code_line_24" >> "$filepath_write_target"
            
            # Make the runscript executable
            chmod 755 "$filepath_write_target"
    fi
}
