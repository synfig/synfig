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
find_package(PkgConfig REQUIRED)
pkg_check_modules(SIGCPP REQUIRED IMPORTED_TARGET sigc++-2.0)
pkg_check_modules(GTKMM3 REQUIRED IMPORTED_TARGET gtkmm-3.0)
pkg_check_modules(XMLPP REQUIRED IMPORTED_TARGET libxml++-2.6)
pkg_check_modules(FONTCONFIG IMPORTED_TARGET fontconfig) # for FontConfig
pkg_check_modules(JACK IMPORTED_TARGET jack)
pkg_search_module(MLT IMPORTED_TARGET mlt++-7 mlt++) # required for widget_soundwave

if(FONTCONFIG_FOUND)
	add_definitions(-DWITH_FONTCONFIG)
endif()

if (NOT MLT_FOUND)
	add_definitions(-DWITHOUT_MLT) # disable MLT if not found
endif()

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

##
## Sub-directories
##

add_subdirectory(synfigapp)
add_subdirectory(gui)
