cmake_minimum_required(VERSION 3.5)

project(synfig-core)

enable_testing()

# using latest stable standard, but not strictly
# the rest is done via target_compile_features instead
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(src)
add_subdirectory(po)

add_dependencies(synfig_bin
    build_pofiles_synfig
)
