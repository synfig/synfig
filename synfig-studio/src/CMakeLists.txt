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

if(VCPKG_TOOLCHAIN)
	set(GTKMM_LIBRARIES
		${ATK_LIBRARY} ${ATKMM_LIBRARY}
		${GDK_LIBRARY}
		${GDKMM_LIBRARY}
		${GIOMM_LIBRARY}
		${GLIB_LIBRARIES}
		${GLIBMM_LIBRARY}
		${GTK_LIBRARY}
		${GTKMM_LIBRARY}
	)
else()
	## TODO: check if we really need to find deps common with synfig core
	include(FindPkgConfig)
	if(PKG_CONFIG_FOUND)
		pkg_check_modules(SIGCPP REQUIRED IMPORTED_TARGET sigc++-2.0)
		pkg_check_modules(GTKMM3 REQUIRED IMPORTED_TARGET gtkmm-3.0)
		pkg_check_modules(XMLPP REQUIRED IMPORTED_TARGET libxml++-2.6)
		pkg_check_modules(FONTCONFIG IMPORTED_TARGET fontconfig) # for FontConfig
		pkg_check_modules(JACK IMPORTED_TARGET jack)
		pkg_search_module(MLT IMPORTED_TARGET mlt++-7 mlt++) # required for widget_soundwave
	endif()
endif()

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
