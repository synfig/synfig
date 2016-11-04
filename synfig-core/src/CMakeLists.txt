# TODO: find out which version is actually required
cmake_minimum_required(VERSION 3.1)

project(synfig)

##
## Find packages
##

# TODO: check if we really need all of them
find_package(Boost REQUIRED system filesystem program_options chrono)

find_package(ZLIB REQUIRED)

include(FindPkgConfig)
pkg_check_modules(SIGCPP REQUIRED sigc++-2.0)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
pkg_check_modules(GIOMM REQUIRED giomm-2.4)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(LIBXML REQUIRED libxml++-2.6)
pkg_check_modules(MLT REQUIRED mlt++)

##
## Include
##

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
    SYSTEM
        ${SIGCPP_INCLUDE_DIRS}
        ${GLIBMM_INCLUDE_DIRS}
        ${GIOMM_INCLUDE_DIRS}
        ${CAIRO_INCLUDE_DIRS}
        ${LIBXML_INCLUDE_DIRS}
        ${MLT_INCLUDE_DIRS}
)
##
## Sub-dirs
##

add_subdirectory(synfig)

##
## Build targets
##


