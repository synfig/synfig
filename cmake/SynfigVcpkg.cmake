if(NOT VCPKG_TOOLCHAIN)
	return()
endif()

# vcpkg supports linux, but the recipes and this file is only tested on Windows
# using the MSVC compiler. Even though, I tried to make this file as generic as possible
if(NOT MSVC)
	message(FATAL_ERROR
		"Synfig does not support building with vcpkg using anything other than the MSVC compiler yet!")
endif()

get_filename_component(VCPKG_TOOLCHAIN_DIR "${CMAKE_TOOLCHAIN_FILE}" DIRECTORY)

# install dependencies
# we could have required the user to pass the option X_VCPKG_APPLOCAL_DEPS_INSTALL
# to enable installing dependencies automatically without this section, but this
# current solution requires less work on the user side, and will give us more control
# (for installing dependencies of gdk-pixbuf loaders for example)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.14")
	if(WIN32)
		macro(install_app_dependencies MOD MOD_NAME)
			install(CODE "
				message(\"-- Installing app dependencies for ${MOD_NAME}...\")
				execute_process(
					COMMAND ${CMAKE_COMMAND} -E copy \"${MOD}\" \"\${CMAKE_INSTALL_PREFIX}/bin/${MOD_NAME}.tmp\"
				)
				execute_process(
					COMMAND powershell -noprofile -executionpolicy Bypass -file ${VCPKG_TOOLCHAIN_DIR}/msbuild/applocal.ps1
						-targetBinary \"\${CMAKE_INSTALL_PREFIX}/bin/${MOD_NAME}.tmp\"
						-installedDir \"${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}$<$<CONFIG:Debug>:/debug>/bin\"
				)
				execute_process(
					COMMAND ${CMAKE_COMMAND} -E remove \"\${CMAKE_INSTALL_PREFIX}/bin/${MOD_NAME}.tmp\"
				)"
			)
		endmacro()
	elseif(UNIX)
		# TODO
	endif()

	# get all targets
	# https://stackoverflow.com/questions/37434946/how-do-i-iterate-over-all-cmake-targets-programmatically/62311397#62311397
	function(get_all_targets var)
			set(targets)
			get_all_targets_recursive(targets ${CMAKE_SOURCE_DIR})
			set(${var} ${targets} PARENT_SCOPE)
	endfunction()

	macro(get_all_targets_recursive targets dir)
			get_property(subdirectories DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
			foreach(subdir ${subdirectories})
					get_all_targets_recursive(${targets} ${subdir})
			endforeach()

			get_property(current_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
			foreach(target ${current_targets})
				get_target_property(target_type ${target} TYPE)
				if ("${target_type}" MATCHES "^(EXECUTABLE|SHARED_LIBRARY|MODULE_LIBRARY)$")
					list(APPEND targets ${target})
				endif()
			endforeach()
	endmacro()

	get_all_targets(all_targets)
	foreach(target ${all_targets})
		install_app_dependencies($<TARGET_FILE:${target}> $<TARGET_FILE_NAME:${target}>)
	endforeach()
else()
	macro(install_app_dependencies MOD MOD_NAME)
	endmacro()
	# this is because generator expressions are required to get target output files
	# and they're not allowed inside install CODE before cmake 3.14
	message(WARNING "At least CMake 3.14 is required for installing the dependencies of synfig (current version: ${CMAKE_VERSION})")
endif()

# gdbus is required by gio on windows as a replacement for dbus on linux
if(WIN32)
	pkg_get_variable(GDBUS gio-2.0 gdbus)
	file(
		COPY "${GDBUS}${CMAKE_EXECUTABLE_SUFFIX}"
		DESTINATION "${SYNFIG_BUILD_ROOT}/bin/"
	)
	install(
		PROGRAMS "${SYNFIG_BUILD_ROOT}/bin/gdbus${CMAKE_EXECUTABLE_SUFFIX}"
		DESTINATION "bin/"
	)
endif()

# gdk-pixbuf loaders
pkg_get_variable(GDKPIXBUF_LOADERS_DIR gdk-pixbuf-2.0 gdk_pixbuf_moduledir)
pkg_get_variable(GDKPIXBUF_QUERYLOADERS gdk-pixbuf-2.0 gdk_pixbuf_query_loaders)

if(GDKPIXBUF_LOADERS_DIR AND GDKPIXBUF_QUERYLOADERS)
	set(SYNFIG_PIXBUF_LOADERS "${SYNFIG_BUILD_ROOT}/lib/gdk-pixbuf-2.0/2.10.0/loaders/")
	file(MAKE_DIRECTORY "${SYNFIG_PIXBUF_LOADERS}")
	file(
		COPY "${GDKPIXBUF_QUERYLOADERS}${CMAKE_EXECUTABLE_SUFFIX}"
		DESTINATION "${SYNFIG_BUILD_ROOT}/bin"
	)

	file(GLOB GDKPIXBUF_MODULES "${GDKPIXBUF_LOADERS_DIR}/*${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set(GDKPIXBUF_MODULES_DEPS)
	foreach(MOD "${GDKPIXBUF_MODULES}")
		file(COPY "${MOD}" DESTINATION "${SYNFIG_PIXBUF_LOADERS}")

		# for these modules to work, their dependencies have to be linked against synfig
		# or present in synfig's bin directory
		if(WIN32)
			get_filename_component(MOD_NAME "${MOD}" NAME)
			add_custom_target(
				copy_${MOD_NAME}_dependencies ALL
				COMMAND ${CMAKE_COMMAND} -E copy "${MOD}" "${SYNFIG_BUILD_ROOT}/bin/${MOD_NAME}"
				COMMAND powershell -noprofile -executionpolicy Bypass -file ${VCPKG_TOOLCHAIN_DIR}/msbuild/applocal.ps1
					-targetBinary "${SYNFIG_BUILD_ROOT}/bin/${MOD_NAME}"
					-installedDir "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}$<$<CONFIG:Debug>:/debug>/bin"
				COMMAND ${CMAKE_COMMAND} -E remove "${SYNFIG_BUILD_ROOT}/bin/${MOD_NAME}"
				DEPENDS "${MOD}"
			)
			install_app_dependencies(${MOD} ${MOD_NAME})
			list(APPEND GDKPIXBUF_MODULES_DEPS "copy_${MOD_NAME}_dependencies")
		elseif(UNIX)
			# TODO: find the gdkpixbuf loader dependencies using something like "ldd",
			# copy them and change their rpath to look at synfig's lib directory
		endif()
	endforeach()

	install(
		DIRECTORY "${SYNFIG_BUILD_ROOT}/lib/gdk-pixbuf-2.0/2.10.0/"
		DESTINATION "lib/gdk-pixbuf-2.0/2.10.0/"
	)

	add_custom_target(
		generate_pixbuf_loaders_cache ALL
		BYPRODUCTS "${SYNFIG_PIXBUF_LOADERS}/../loaders.cache"
		COMMAND "${SYNFIG_BUILD_ROOT}/bin/gdk-pixbuf-query-loaders${CMAKE_EXECUTABLE_SUFFIX}" > "${SYNFIG_PIXBUF_LOADERS}/../loaders.cache"
	)
	add_dependencies(generate_pixbuf_loaders_cache ${GDKPIXBUF_MODULES_DEPS})
else()
	message(WARNING "Gdk-pixbuf loaders directory cannot be found!")
endif()

# Glib schemas
pkg_get_variable(GLIB_SCHEMAS_DIR gio-2.0 schemasdir)

if(GLIB_SCHEMAS_DIR AND EXISTS "${GLIB_SCHEMAS_DIR}/gschemas.compiled")
	set (GLIB_SCHEMAS_COMPILED "${GLIB_SCHEMAS_DIR}/gschemas.compiled")
endif()

if((NOT GLIB_SCHEMAS_DIR OR NOT EXISTS "${GLIB_SCHEMAS_DIR}") AND (NOT GLIB_SCHEMAS_COMPILED OR NOT EXISTS "${GLIB_SCHEMAS_COMPILED}"))
	message(FATAL_ERROR "Could not find glib schemas directory! This will cause synfigstudio to crash\n"
		"Please set GLIB_SCHEMAS_DIR or GLIB_SCHEMAS_COMPILED")
endif()

file (MAKE_DIRECTORY "${SYNFIG_BUILD_ROOT}/share/glib-2.0/schemas")

if(NOT GLIB_SCHEMAS_COMPILED OR NOT EXISTS "${GLIB_SCHEMAS_COMPILED}")
	set(_GLIB_SCHEMAS_COMPILED "${SYNFIG_BUILD_ROOT}/share/glib-2.0/schemas/gschemas.compiled")
	pkg_get_variable(GLIB_COMPILE_SCHEMAS gio-2.0 glib_compile_schemas)
	if(NOT GLIB_COMPILE_SCHEMAS OR NOT EXISTS "${GLIB_COMPILE_SCHEMAS}${CMAKE_EXECUTABLE_SUFFIX}")
		message(FATAL_ERROR "Could not find glib_compile_schemas!")
	endif()

	add_custom_target(
		compile_glib_schemas ALL
		BYPRODUCTS "${SYNFIG_BUILD_ROOT}/share/glib-2.0/schemas/gschemas.compiled"
		COMMAND "${GLIB_COMPILE_SCHEMAS}${CMAKE_EXECUTABLE_SUFFIX}" "${GLIB_SCHEMAS_DIR}" --targetdir="${SYNFIG_BUILD_ROOT}/share/glib-2.0/schemas"
		DEPENDS "${GLIB_SCHEMAS_DIR}/gschema.dtd"
	)
else()
	set(_GLIB_SCHEMAS_COMPILED "${GLIB_SCHEMAS_COMPILED}")
	file(
		COPY "${_GLIB_SCHEMAS_COMPILED}"
		DESTINATION "${SYNFIG_BUILD_ROOT}/share/glib-2.0/schemas"
	)
endif()

install(
	PROGRAMS "${_GLIB_SCHEMAS_COMPILED}"
	DESTINATION "share/glib-2.0/schemas"
)

# fonts
pkg_get_variable(FONTCONFIG_CONFDIR fontconfig confdir)
if(FONTCONFIG_CONFDIR AND EXISTS "${FONTCONFIG_CONFDIR}")
	file(
		COPY "${FONTCONFIG_CONFDIR}/"
		DESTINATION "${SYNFIG_BUILD_ROOT}/etc/fonts"
	)
	install(
		DIRECTORY "${SYNFIG_BUILD_ROOT}/etc/fonts/"
		DESTINATION "etc/fonts"
	)
else()
	message(WARNING "Could not locate fontconfig's fonts directory")
endif()

# icons
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
