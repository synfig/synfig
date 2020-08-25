cmake_minimum_required(VERSION 2.8)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(synfig-studio)
set(STUDIO_VERSION_MAJOR 1)
set(STUDIO_VERSION_MINOR 5)
set(STUDIO_VERSION_PATCH 0)

# Options
option(MACOSX_PACKAGE "Makes Synfig in a .app format" OFF)

set(SYNFIG_BUILD_ROOT ${CMAKE_BINARY_DIR}/output)

if(${MACOSX_PACKAGE})
    set(SYNFIG_BUILD_ROOT ${CMAKE_BINARY_DIR}/output/SynfigStudio.app/Contents/Resources)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${SYNFIG_BUILD_ROOT}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SYNFIG_BUILD_ROOT}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SYNFIG_BUILD_ROOT}/bin)

#set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-inline-functions -fno-inline -g -O0 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-inline-functions -fno-inline -g -O0 -Wall")

# This can be enabled once the address sanitization related errors are resolved.
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=undefined  -fsanitize=address -fno-omit-frame-pointer")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined  -fsanitize=address  -fno-omit-frame-pointer")

message("-- CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

include(SynfigFindGit)
include(SynfigInstall)

add_subdirectory(ETL)
include_directories(ETL)
include_directories(${PROJECT_BINARY_DIR}/ETL) # to find the generated etl_profile.h

# Manually setting ETL Version.
set(ETL_VERSION "1.5.0")

add_subdirectory(synfig-core)

include_directories(synfig-core/src)
add_subdirectory(synfig-studio)

if(WIN32)
    include(InstallMSYS2)
elseif(APPLE)
    include(InstallMacOSX)
endif()

include(CPackConfig)
