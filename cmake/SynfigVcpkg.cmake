get_filename_component(VCPKG_TOOLCHAIN_DIR "${CMAKE_TOOLCHAIN_FILE}" DIRECTORY)
find_package(PkgConfig REQUIRED)

# install dependecies
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.14")
	if(WIN32)
		macro(install_app_dependencies mod mod_name)
			install(CODE "
				message(\"-- Installing app dependencies for ${mod_name}...\")
				execute_process(
					COMMAND ${CMAKE_COMMAND} -E copy \"${mod}\" \"\${CMAKE_INSTALL_PREFIX}/bin/${mod_name}.tmp\"
				)
				execute_process(
					COMMAND powershell -noprofile -executionpolicy Bypass -file ${VCPKG_TOOLCHAIN_DIR}/msbuild/applocal.ps1
						-targetBinary \"\${CMAKE_INSTALL_PREFIX}/bin/${mod_name}.tmp\"
						-installedDir \"${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}$<$<CONFIG:Debug>:/debug>/bin\"
				)
				execute_process(
					COMMAND ${CMAKE_COMMAND} -E remove \"\${CMAKE_INSTALL_PREFIX}/bin/${mod_name}.tmp\"
				)"
			)
		endmacro()
	elseif(UNIX)
		# TODO: make appdeps.py work on windows as well, and don't depend on vcpkg's
		# applocal.ps1, since it's not certain that it will remain the same
		set(APPDEPS "${CMAKE_SOURCE_DIR}/autobuild/appdeps.py")

		if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE MATCHES "^[Dd][Ee][Bb][Uu][Gg]$")
			set(__path_suffix "debug")
		else()
			set(__path_suffix ".")
		endif()

		find_package (Python COMPONENTS Interpreter REQUIRED)

		macro(install_app_dependencies mod mod_name)
			install(CODE "
				message(\"-- Installing app dependencies for ${mod_name}...\")
				execute_process(
					COMMAND \"${Python_EXECUTABLE}\" \"${APPDEPS}\" -L \"${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/${__path_suffix}/lib/\"
						--outdir \"\${CMAKE_INSTALL_PREFIX}/lib/\" \"${mod}\"
				)"
			)
		endmacro()
	endif()
else()
	macro(install_app_dependencies mod mod_name)
	endmacro()
	# this is because generator expressions are required to get target output files
	# and they're not allowed inside install CODE before cmake 3.14
	message(WARNING "At least CMake 3.14 is required for installing the dependencies of synfig (current version: ${CMAKE_VERSION})")
endif()


# this closely follows what vcpkg does:
# https://github.com/microsoft/vcpkg/blob/01b29f6d8212bc845da64773b18665d682f5ab66/scripts/buildsystems/vcpkg.cmake#L697-L741
function(install)
	if(ARGV0 STREQUAL "TARGETS")
		# Will contain the list of targets
		set(parsed_targets "")

		set(component_param "")

		# Parse arguments given to the install function to find targets and (runtime) destination
		set(modifier "") # Modifier for the command in the argument
		set(last_command "") # Last command we found to process
		set(destination "")
		set(library_destination "")
		set(runtime_destination "")

		foreach(arg ${ARGV})
			if(arg MATCHES "^(ARCHIVE|LIBRARY|RUNTIME|OBJECTS|FRAMEWORK|BUNDLE|PRIVATE_HEADER|PUBLIC_HEADER|RESOURCE|INCLUDES)$")
				set(modifier "${arg}")
				continue()
			endif()
			if(arg MATCHES "^(TARGETS|DESTINATION|PERMISSIONS|CONFIGURATIONS|COMPONENT|NAMELINK_COMPONENT|OPTIONAL|EXCLUDE_FROM_ALL|NAMELINK_ONLY|NAMELINK_SKIP|EXPORT)$")
				set(last_command "${arg}")
				continue()
			endif()

			if(last_command STREQUAL "TARGETS")
				list(APPEND parsed_targets "${arg}")
			endif()

			if(last_command STREQUAL "DESTINATION" AND modifier STREQUAL "")
				set(destination "${arg}")
			endif()

			if(last_command STREQUAL "DESTINATION" AND modifier STREQUAL "RUNTIME")
				set(runtime_destination "${arg}")
			endif()

			if(last_command STREQUAL "DESTINATION" AND modifier STREQUAL "LIBRARY")
				set(library_destination "${arg}")
			endif()

			if(last_command STREQUAL "COMPONENT")
				set(component_param "COMPONENT" "${arg}")
			endif()
		endforeach()

		if(NOT destination)
			if(WIN32 AND runtime_destination)
				set(destination "${runtime_destination}")
			elseif(UNIX AND library_destination)
				set(destination "${library_destination}")
			endif()
		endif()

		# calculate rpath
		if(UNIX)
			set(rpath "$ORIGIN/..")
			string(REGEX MATCHALL "/" separators "${destination}")
			foreach(separator ${separators})
				string(APPEND rpath "/..")
			endforeach()
			string(APPEND rpath "/lib")
		endif()

		if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.14")
			cmake_policy(SET CMP0087 NEW)
		endif()

		foreach(target ${parsed_targets})
			install_app_dependencies("$<TARGET_FILE:${target}>" "$<TARGET_FILE_NAME:${target}>")
			if(UNIX)
				set_target_properties(${target} PROPERTIES INSTALL_RPATH "${rpath}")
				message("set_target_properties(${target} PROPERTIES INSTALL_RPATH ${rpath})")
			endif()
		endforeach()
	endif()
	_install(${ARGV})
endfunction()

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
	foreach(MOD ${GDKPIXBUF_MODULES})
		file(COPY "${MOD}" DESTINATION "${SYNFIG_PIXBUF_LOADERS}")
		get_filename_component(MOD_NAME "${MOD}" NAME)

		# for these modules to work, their dependencies have to be linked against synfig
		# or present in synfig's bin directory
		if(WIN32)
			add_custom_target(
				copy_${MOD_NAME}_dependencies ALL
				COMMAND ${CMAKE_COMMAND} -E copy "${MOD}" "${SYNFIG_BUILD_ROOT}/bin/${MOD_NAME}"
				COMMAND powershell -noprofile -executionpolicy Bypass -file ${VCPKG_TOOLCHAIN_DIR}/msbuild/applocal.ps1
					-targetBinary "${SYNFIG_BUILD_ROOT}/bin/${MOD_NAME}"
					-installedDir "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}$<$<CONFIG:Debug>:/debug>/bin"
				COMMAND ${CMAKE_COMMAND} -E remove "${SYNFIG_BUILD_ROOT}/bin/${MOD_NAME}"
				DEPENDS "${MOD}"
			)
		elseif(UNIX)
			# TODO: find the gdkpixbuf loader dependencies using something like "ldd",
			# copy them and change their rpath to look at synfig's lib directory
			add_custom_target(
				copy_${MOD_NAME}_dependencies ALL
			)
		endif()
		install_app_dependencies(${MOD} ${MOD_NAME})
		list(APPEND GDKPIXBUF_MODULES_DEPS "copy_${MOD_NAME}_dependencies")
	endforeach()

	install(
		DIRECTORY "${SYNFIG_BUILD_ROOT}/lib/gdk-pixbuf-2.0/2.10.0/"
		DESTINATION "lib/gdk-pixbuf-2.0/2.10.0/"
	)

	add_custom_target(
		generate_pixbuf_loaders_cache ALL
		BYPRODUCTS "${SYNFIG_PIXBUF_LOADERS}/../loaders.cache"
		COMMAND ${CMAKE_COMMAND} -E env GDK_PIXBUF_MODULEDIR="${SYNFIG_PIXBUF_LOADERS}"
			${SYNFIG_BUILD_ROOT}/bin/gdk-pixbuf-query-loaders${CMAKE_EXECUTABLE_SUFFIX} > "${SYNFIG_PIXBUF_LOADERS}/../loaders.cache"
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
