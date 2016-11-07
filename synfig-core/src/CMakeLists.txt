# TODO: find out which version is actually required
cmake_minimum_required(VERSION 3.1)

project(synfig)

##
## Find packages
##

# TODO: check if we really need all of them
find_package(Boost REQUIRED system filesystem program_options chrono)

find_package(ZLIB REQUIRED)

## TODO: investigate these more closely
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
if (CMAKE_USE_PTHREADS_INIT)
    set(HAVE_LIBPTHREAD ON)
endif()

include(FindPkgConfig)
pkg_check_modules(SIGCPP REQUIRED sigc++-2.0)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
pkg_check_modules(GIOMM REQUIRED giomm-2.4)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(LIBXML REQUIRED libxml++-2.6)
pkg_check_modules(MLT REQUIRED mlt++)
pkg_check_modules(FFTW REQUIRED fftw3)

# TODO: move to module where it is actually required
pkg_check_modules(PANGO REQUIRED pango)

##
## Configure
##

## set git version
## TODO: put into separate file
find_package(Git)
if (GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} show -s --format=%ad --date=format:%Y%m%d HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
    set(GIT_DATE    "unknown")
    set(GIT_BRANCH  "unknown")
    set(GIT_HASH    "unknown")
endif()

add_definitions(-DHAVE_CONFIG_H)
configure_file(config.h.cmake.in config.h)
configure_file(autorevision.h.cmake.in autorevision.h)

##
## Include
##

## for config
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

##
## Sub-dirs
##

add_subdirectory(synfig)
add_subdirectory(tool)
add_subdirectory(modules)

##
## Build targets
##
