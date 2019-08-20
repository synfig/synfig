cmake_minimum_required(VERSION 3.1)

##
## Find packages
##

## TODO: check version
if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    find_package(synfig CONFIG REQUIRED)
endif()

find_package(Gettext)

if (Gettext_FOUND)
    set(ENABLE_NLS ON)
    set(GETTEXT_PACKAGE ${PROJECT_NAME})
endif()

## TODO: check if we really need to find deps common with synfig core
include(FindPkgConfig)
pkg_check_modules(SIGCPP REQUIRED sigc++-2.0)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
pkg_check_modules(LIBXML REQUIRED libxml++-2.6)

##
## Config
##

add_definitions(-DHAVE_CONFIG_H)
configure_file(config.h.cmake.in config.h)

##
## Header include dirs
##

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
    SYSTEM
#         ${synfig_INCLUDE_DIRS}
        ${SIGCPP_INCLUDE_DIRS}
        ${GTKMM_INCLUDE_DIRS}
)

##
## Sub-directories
##

add_subdirectory(synfigapp)
add_subdirectory(gui)
