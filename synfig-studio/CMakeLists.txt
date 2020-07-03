## TODO: find out which version is actually required
cmake_minimum_required(VERSION 3.1)

project(synfigstudio)

# using latest stable standard, but not strictly
# the rest is done via target_compile_features instead
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(src)

add_subdirectory(brushes)
add_subdirectory(images)
add_subdirectory(plugins)
add_subdirectory(po)
add_subdirectory(sounds)
add_subdirectory(ui)
