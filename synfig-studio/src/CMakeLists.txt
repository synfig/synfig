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
include(FindPkgConfig REQUIRED)
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

if (MSVC)
	find_program(GDBUS gdbus REQUIRED)
	file(
		COPY "${GDBUS}"
		DESTINATION "${SYNFIG_BUILD_ROOT}/bin/"
	)
	install(
		PROGRAMS "${SYNFIG_BUILD_ROOT}/bin/gdbus.exe"
		DESTINATION "bin/"
	)

	if (VCPKG_TOOLCHAIN AND NOT GLIB_SCHEMAS_DIR AND NOT GLIB_SCHEMAS_COMPILED)
		set(GLIB_SCHEMAS_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/glib-2.0/schemas")
	endif ()

	if ((NOT GLIB_SCHEMAS_DIR OR NOT EXISTS "${GLIB_SCHEMAS_DIR}") AND (NOT GLIB_SCHEMAS_COMPILED OR NOT EXISTS "${GLIB_SCHEMAS_COMPILED}"))
	message(FATAL_ERROR "Could not find glib schemas directory! This will cause synfigstudio to crash\n"
		"Please set GLIB_SCHEMAS_DIR or GLIB_SCHEMAS_COMPILED")
	endif ()

	file (MAKE_DIRECTORY "${SYNFIG_BUILD_ROOT}/share/glib-2.0/schemas")

	if (NOT GLIB_SCHEMAS_COMPILED OR NOT EXISTS "${GLIB_SCHEMAS_COMPILED}")
		set(_GLIB_SCHEMAS_COMPILED "${SYNFIG_BUILD_ROOT}/share/glib-2.0/schemas/gschemas.compiled")
		find_program(GLIB_COMPILE_SCHEMAS glib-compile-schemas REQUIRED)
		add_custom_command(OUTPUT
			"${_GLIB_SCHEMAS_COMPILED}"
			COMMAND "${GLIB_COMPILE_SCHEMAS}" "${GLIB_SCHEMAS_DIR}" --targetdir="${SYNFIG_BUILD_ROOT}/share/glib-2.0/schemas"
			MAIN_DEPENDENCY "${GLIB_SCHEMAS_DIR}/gschema.dtd")
		add_custom_target(glib_schemas DEPENDS "${_GLIB_SCHEMAS_COMPILED}")
		add_dependencies(synfigstudio glib_schemas)
	else ()
		set(_GLIB_SCHEMAS_COMPILED "${GLIB_SCHEMAS_COMPILED}")
		file(
			COPY "${_GLIB_SCHEMAS_COMPILED}"
			DESTINATION "${SYNFIG_BUILD_ROOT}/share/glib-2.0/schemas"
		)
	endif ()
	install(
		PROGRAMS "${_GLIB_SCHEMAS_COMPILED}"
		DESTINATION "share/glib-2.0/schemas"
	)
endif ()
