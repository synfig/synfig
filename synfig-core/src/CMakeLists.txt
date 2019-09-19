# TODO: find out which version is actually required
cmake_minimum_required(VERSION 3.1)

##
## Find packages
##

# TODO: check if we really need all of them
find_package(Boost REQUIRED system)

find_package(ZLIB REQUIRED)

## ETL doesn't install a package configuration so find package will fail
# find_package(ETL REQUIRED)


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
pkg_check_modules(PANGOCAIRO REQUIRED pangocairo) # lyr_freetype
pkg_check_modules(LIBXML REQUIRED libxml++-2.6)
pkg_check_modules(MLT REQUIRED mlt++)
pkg_check_modules(FFTW REQUIRED fftw3)
pkg_check_modules(FT REQUIRED freetype2) # for lyr_freetype
pkg_check_modules(LIBPNG REQUIRED libpng) # for mod_png
pkg_check_modules(LIBMNG REQUIRED libmng) # for mod_mng
pkg_check_modules(LIBJPEG REQUIRED libjpeg) # for mod_mng
pkg_check_modules(OPENEXR REQUIRED OpenEXR) # for mod_openexr
pkg_check_modules(MAGICKCORE REQUIRED MagickCore) # for Magick++

## TODO: move to module where it is actually required
pkg_check_modules(PANGO REQUIRED pango)

## TODO: should we keep it here?
find_package(ImageMagick COMPONENTS Magick++)

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

# Check headers
include(CheckIncludeFile)
CHECK_INCLUDE_FILE(sys/wait.h  HAVE_SYS_WAIT_H)
CHECK_INCLUDE_FILE(signal.h    HAVE_SIGNAL_H)
CHECK_INCLUDE_FILE(termios.h   HAVE_TERMIOS_H)
CHECK_INCLUDE_FILE(process.h   HAVE_PROCESS_H)
CHECK_INCLUDE_FILE(io.h        HAVE_IO_H)
CHECK_INCLUDE_FILE(sys/fcntl.h HAVE_FCNTL_H)

# Check functions
include (CheckFunctionExists)
CHECK_FUNCTION_EXISTS(fork HAVE_FORK)
CHECK_FUNCTION_EXISTS(pipe HAVE_PIPE)
CHECK_FUNCTION_EXISTS(waitpid HAVE_WAITPID)

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
## CMake helpers
##

include(cmake/install_header.cmake)

##
## Sub-dirs
##

add_subdirectory(synfig)
add_subdirectory(tool)
add_subdirectory(modules)

##
## Build targets
##
