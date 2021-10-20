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
		${CAIROMM_LIBRARY}
		${GDK_LIBRARY}
		${GDKMM_LIBRARY}
		${GIOMM_LIBRARY}
		${GLIB_LIBRARIES}
		${GLIBMM_LIBRARY}
		${GTK_LIBRARY}
		${GTKMM_LIBRARY}
		${PANGOMM_LIBRARY}
	)
else()
	## TODO: check if we really need to find deps common with synfig core
	include(FindPkgConfig)
	if(PKG_CONFIG_FOUND)
		pkg_check_modules(SIGCPP REQUIRED sigc++-2.0)
		pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
		pkg_check_modules(LIBXML REQUIRED libxml++-2.6)
		pkg_check_modules(PANGO REQUIRED pango) # used in TimeTrackView, OneMoment, etc.
		pkg_check_modules(CAIRO REQUIRED cairo)
		pkg_check_modules(PANGOCAIRO REQUIRED pangocairo)
		pkg_check_modules(MLT mlt++) # required for widget_soundwave
	endif()

	foreach(pkg_config_lib SIGCPP GTKMM LIBXML MLT FFTW PANGO CAIRO PANGOCAIRO)
		include_directories(${${pkg_config_lib}_INCLUDE_DIRS})
		link_directories(${${pkg_config_lib}_LIBRARY_DIRS})
	endforeach()
endif()

if(FONT_CONFIG_FOUND)
	add_definitions(-DWITH_FONTCONFIG)
endif()

if (NOT MLT_FOUND)
	add_definitions(-DWITHOUT_MLT) # disable MLT if not found
endif()

##
## Config
##

if(FONT_CONFIG_FOUND)
add_definitions(-DWITH_FONTCONFIG)
endif()

add_definitions(-DHAVE_CONFIG_H)
configure_file(config.h.cmake.in config.h)

##
## Header include dirs
##

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
        ${SIGCPP_INCLUDE_DIRS}
        ${GTKMM_INCLUDE_DIRS}
)

##
## Search path for libs (needed to link with libsynfig)
##
link_directories(${CMAKE_PREFIX_PATH}/lib)

##
## Sub-directories
##

add_subdirectory(synfigapp)
add_subdirectory(gui)
