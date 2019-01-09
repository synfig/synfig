#!/usr/bin/env bash

# Define build dirs
cmake_build_dir="_cmake-build"
etl_build_dir="etl"
synfig_build_dir="synfig-core"
synfigstudio_build_dir="synfig-studio"
out_dir="out"

# Define commands
env_run_command="synfigstudio"

build_etl() {
    echo "Building ETL"

    cd ${absolute_base_dir}
    cd "./${cmake_build_dir}/${etl_build_dir}"
    cmake -DCMAKE_INSTALL_PREFIX="${absolute_base_dir}/${cmake_build_dir}/${out_dir}" ../../ETL/ && make && make install

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
    cmake -DCMAKE_INSTALL_PREFIX="${absolute_base_dir}/${cmake_build_dir}/${out_dir}" -DCMAKE_CXX_FLAGS="-I ${absolute_base_dir}/${cmake_build_dir}/${out_dir}/include" ../../synfig-core/ && make && make install

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
    cmake -DCMAKE_PREFIX_PATH="${absolute_base_dir}/${cmake_build_dir}/${out_dir}" -DCMAKE_INSTALL_PREFIX="${absolute_base_dir}/${cmake_build_dir}/${out_dir}" -DCMAKE_CXX_FLAGS="-I ${absolute_base_dir}/${cmake_build_dir}/${out_dir}/include" ../../synfig-studio/ && make && PATH="${absolute_base_dir}/${cmake_build_dir}/${out_dir}/bin:$PATH" LD_LIBRARY_PATH="${absolute_base_dir}/${cmake_build_dir}/${out_dir}/lib" make build_images && make install
    
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

run_command_in_outenv() {
    if [ "$#" -eq 1 ]
        then
            env_run_command="$1"
    fi

    PATH="${absolute_base_dir}/${cmake_build_dir}/${out_dir}/bin:$PATH" LD_LIBRARY_PATH="${absolute_base_dir}/${cmake_build_dir}/${out_dir}/lib" XDG_DATA_DIRS="${absolute_base_dir}/${cmake_build_dir}/${out_dir}/share:$XDG_DATA_DIRS" $env_run_command
}
