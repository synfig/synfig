##
## Configure
##

set(STUDIO_VERSION "${STUDIO_VERSION_MAJOR}.${STUDIO_VERSION_MINOR}.${STUDIO_VERSION_PATCH}")
set(SHOW_EXTRA_INFO OFF) # this needs some fixes to work
set(DATA_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE PATH "Path, where the share directory with the pixmaps, sounds, plugins and translations is located. Default: Prefix, where Synfig Studio is going to be installed")
set(IMG_EXT "png" CACHE STRING "File extension of the pixmaps (without the \".\"). Default: png")

configure_file(autorevision.h.cmake.in autorevision.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## Defines
set(SYNFIG_DATA_PREFIX "${DATA_PREFIX}/share/synfig")

add_definitions(-DIMAGE_EXT="${IMG_EXT}")
add_definitions(-DIMAGE_DIR="${DATA_PREFIX}/share/pixmaps")
add_definitions(-DSYNFIG_DATADIR="${SYNFIG_DATA_PREFIX}")
add_definitions(-DLOCALEDIR="${DATA_PREFIX}/share/locale")
add_definitions(-DCMAKE_BUILD)
add_definitions(-DDEVEL_VERSION="Revision date: ${GIT_DATE}\\nBranch: ${GIT_BRANCH}\\nRevision ID: ${GIT_HASH}")

if (CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions("-DSHOW_EXTRA_INFO")
	add_definitions(-DETL_VERSION="${ETL_VERSION}")
endif()

##
## Targets
##

if(WIN32)
	configure_file(synfigstudio.rc ${CMAKE_CURRENT_BINARY_DIR}/synfigstudio_configured.rc)
	add_executable(synfigstudio main.cpp ${CMAKE_CURRENT_BINARY_DIR}/synfigstudio_configured.rc)
else()
	add_executable(synfigstudio main.cpp)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_sources(synfigstudio
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/duck.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/timemodel.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/app.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/asyncrenderer.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/autorecover.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/canvasview.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/devicetracker.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/dialogsettings.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/duckmatic.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/iconcontroller.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/instance.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/keymapsettings.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/onemoment.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/pluginmanager.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/preview.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/progresslogger.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/renddesc.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/render.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/resourcehelper.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/splash.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/statemanager.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/timeplotdata.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/waypointrenderer.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/workarea.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/workspacehandler.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/main_win32.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/mainwindow.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/helpers.cpp"
)

include(actionmanagers/CMakeLists.txt)
include(cellrenderer/CMakeLists.txt)
include(dialogs/CMakeLists.txt)
include(dials/CMakeLists.txt)
include(docks/CMakeLists.txt)
include(modules/CMakeLists.txt)
include(states/CMakeLists.txt)
include(trees/CMakeLists.txt)
include(widgets/CMakeLists.txt)
include(workarearenderer/CMakeLists.txt)

add_subdirectory(resources)

target_link_libraries(synfigstudio PRIVATE
	PkgConfig::GTKMM3
	PkgConfig::XMLPP
	libsynfig
	synfigapp
)

if (FONTCONFIG_FOUND)
	target_link_libraries(synfigstudio PRIVATE PkgConfig::FONTCONFIG)
endif ()

if (JACK_FOUND)
	target_compile_definitions(synfigstudio PRIVATE WITH_JACK)
	target_link_libraries(synfigstudio PRIVATE PkgConfig::JACK)
endif ()

if (MLT_FOUND)
	target_link_libraries(synfigstudio PRIVATE PkgConfig::MLT)
endif ()

if(TCMALLOC_LIBRARY)
	target_link_libraries(synfigstudio PRIVATE ${TCMALLOC_LIBRARY})
endif()

install(
	TARGETS synfigstudio
	DESTINATION bin
)

# the following part is responsible for packaging gtk
# TODO: separate this to a separate file maybe?
if (MSVC)
	# gdbus is required by gio as a replacement for dbus on linux
	pkg_get_variable(GDBUS gio-2.0 gdbus)
	file(
		COPY "${GDBUS}.exe"
		DESTINATION "${SYNFIG_BUILD_ROOT}/bin/"
	)
	install(
		PROGRAMS
			"${SYNFIG_BUILD_ROOT}/bin/gdbus.exe"
		DESTINATION "bin/"
	)

	# gdk-pixbuf loaders
	pkg_get_variable(GDKPIXBUF_LOADERS_DIR gdk-pixbuf-2.0 gdk_pixbuf_moduledir)
	pkg_get_variable(GDKPIXBUF_QUERYLOADERS gdk-pixbuf-2.0 gdk_pixbuf_query_loaders)

	if (GDKPIXBUF_LOADERS_DIR AND GDKPIXBUF_QUERYLOADERS)
		set(SYNFIG_PIXBUF_LOADERS "${SYNFIG_BUILD_ROOT}/lib/gdk-pixbuf-2.0/2.10.0/loaders/")
		file(MAKE_DIRECTORY "${SYNFIG_PIXBUF_LOADERS}")
		file(
			COPY "${GDKPIXBUF_QUERYLOADERS}.exe"
			DESTINATION "${SYNFIG_BUILD_ROOT}/bin"
		)

		file(GLOB GDKPIXBUF_MODULES "${GDKPIXBUF_LOADERS_DIR}/*.dll")
		foreach (MOD ${GDKPIXBUF_MODULES})
			file(
				COPY "${MOD}"
				DESTINATION "${SYNFIG_PIXBUF_LOADERS}"
			)

			# for these modules to work, their dependencies have to be linked against synfig
			# or present in synfig's bin directory
			if (VCPKG_TOOLCHAIN)
				get_filename_component(VCPKG_TOOLCHAIN_DIR "${CMAKE_TOOLCHAIN_FILE}" DIRECTORY)
				get_filename_component(MOD_NAME "${MOD}" NAME)

				add_custom_command(TARGET synfigstudio POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy "${MOD}" "${SYNFIG_BUILD_ROOT}/bin/${MOD_NAME}"
					COMMAND powershell -noprofile -executionpolicy Bypass -file ${VCPKG_TOOLCHAIN_DIR}/msbuild/applocal.ps1
						-targetBinary "${SYNFIG_BUILD_ROOT}/bin/${MOD_NAME}"
						-installedDir "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}$<$<CONFIG:Debug>:/debug>/bin"
						-OutVariable out
					COMMAND ${CMAKE_COMMAND} -E remove "${SYNFIG_BUILD_ROOT}/bin/${MOD_NAME}"
				)
			endif ()
		endforeach ()

		install(
			DIRECTORY "${SYNFIG_BUILD_ROOT}/lib/gdk-pixbuf-2.0/2.10.0/loaders"
			DESTINATION "lib/gdk-pixbuf-2.0/2.10.0/"
		)

		add_custom_command(
			OUTPUT "${SYNFIG_PIXBUF_LOADERS}/../loaders.cache"
			COMMAND "${SYNFIG_BUILD_ROOT}/bin/gdk-pixbuf-query-loaders.exe" > "${SYNFIG_PIXBUF_LOADERS}/../loaders.cache"
		)
		add_custom_target(gdkpixbuf_loaders ALL DEPENDS "${SYNFIG_PIXBUF_LOADERS}/../loaders.cache")
		add_dependencies(gdkpixbuf_loaders synfigstudio)
	else ()
		message(WARNING "Gdk-pixbuf loaders directory cannot be found!")
	endif ()

	# Glib schemas
	pkg_get_variable(GLIB_SCHEMAS_DIR gio-2.0 schemasdir)

	if ((NOT GLIB_SCHEMAS_DIR OR NOT EXISTS "${GLIB_SCHEMAS_DIR}") AND (NOT GLIB_SCHEMAS_COMPILED OR NOT EXISTS "${GLIB_SCHEMAS_COMPILED}"))
		message(FATAL_ERROR "Could not find glib schemas directory! This will cause synfigstudio to crash\n"
			"Please set GLIB_SCHEMAS_DIR or GLIB_SCHEMAS_COMPILED")
	endif ()

	if (EXISTS "${GLIB_SCHEMAS_DIR}/gschemas.compiled")
		set (GLIB_SCHEMAS_COMPILED "${GLIB_SCHEMAS_DIR}/gschemas.compiled")
	endif()

	file (MAKE_DIRECTORY "${SYNFIG_BUILD_ROOT}/share/glib-2.0/schemas")

	if (NOT GLIB_SCHEMAS_COMPILED OR NOT EXISTS "${GLIB_SCHEMAS_COMPILED}")
		set(_GLIB_SCHEMAS_COMPILED "${SYNFIG_BUILD_ROOT}/share/glib-2.0/schemas/gschemas.compiled")

		pkg_get_variable(GLIB_COMPILE_SCHEMAS gio-2.0 glib_compile_schemas)

		add_custom_command(
			TARGET synfigstudio POST_BUILD
			COMMAND "${GLIB_COMPILE_SCHEMAS}" "${GLIB_SCHEMAS_DIR}" --targetdir="${SYNFIG_BUILD_ROOT}/share/glib-2.0/schemas"
			MAIN_DEPENDENCY "${GLIB_SCHEMAS_DIR}/gschema.dtd"
		)
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

	# icons
	if (VCPKG_TOOLCHAIN)
		# package hicolor-icon-theme (which is almost completely empty!)
		file(
			COPY "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/hicolor-icon-theme/icons"
			DESTINATION "${SYNFIG_BUILD_ROOT}/share/"
		)

		# package adwaita-icon-theme
		file(
			COPY "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/adwaita-icon-theme/icons"
			DESTINATION "${SYNFIG_BUILD_ROOT}/share/"
		)

		install(
			DIRECTORY "${SYNFIG_BUILD_ROOT}/share/icons"
			DESTINATION "share/"
		)
	endif ()
endif ()
