cmake_minimum_required(VERSION 2.8)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(synfig-studio)
set(STUDIO_VERSION_MAJOR 1)
set(STUDIO_VERSION_MINOR 5)
set(STUDIO_VERSION_PATCH 0)

set(SYNFIG_BUILD_ROOT ${CMAKE_BINARY_DIR}/output)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${SYNFIG_BUILD_ROOT}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SYNFIG_BUILD_ROOT}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SYNFIG_BUILD_ROOT}/bin)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# MSYS is defined for MSYS Makefiles generator and undefined for Ninja (for example)
# But if you run CMake from MSYS shell, we can still detect MSYS by environment variables (MINGW_PREFIX)
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/tutorials/How-To-Write-Platform-Checks
if(MINGW AND NOT MSYS)
	if (NOT DEFINED ENV{MINGW_PREFIX})
		message(FATAL_ERROR "Currently we only support MSYS2 environment for MinGW compiler on Windows."
				" Please pass '-DMSYS=1' to your CMake command to force using MSYS."
				" Also you need to set MINGW_PREFIX environment variable if your MSYS2 installation is not in standard path (c:\\msys64\\)")
	else()
		set(MSYS ON)
		set(MSYS_ROOT "$ENV{MINGW_PREFIX}/..")
	endif()
endif()

#set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

if(MSVC)
	add_definitions(-D_USE_MATH_DEFINES) # <cmath> header need this define to add M_PI define
	add_definitions(-DNOMINMAX) # windows.h defines min/max and brokes std::min/max
	add_definitions(-DWITHOUT_MLT) # Disable MLT for MSVC build (as there are no such package yet)

	## /MP Enable multi-processor compilation (faster)
	add_compile_options(/MP /W0 /bigobj)
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
else()
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-sinline-functions -fno-inline -g -O0 -Wall")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-inline-functions -fno-inline -g -O0 -Wall")
endif(MSVC)

# This can be enabled once the address sanitization related errors are resolved.
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=undefined  -fsanitize=address -fno-omit-frame-pointer")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined  -fsanitize=address  -fno-omit-frame-pointer")

message("-- CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

include(SynfigFindGit)

add_subdirectory(ETL)
include_directories(ETL)
include_directories(${PROJECT_BINARY_DIR}/ETL) # to find the generated etl_profile.h

# Manually setting ETL Version.
set(ETL_VERSION "1.5.0")

add_subdirectory(synfig-core)

include_directories(synfig-core/src)
add_subdirectory(synfig-studio)

if(MSYS)
	include(InstallMSYS2)
endif()

include(CPackConfig)
