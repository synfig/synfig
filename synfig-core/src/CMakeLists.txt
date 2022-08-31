##
## Find packages
##

find_package(ZLIB REQUIRED)
find_package(Intl REQUIRED)

## ETL doesn't install a package configuration so find package will fail
# find_package(ETL REQUIRED)


## TODO: investigate these more closely
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
if (CMAKE_USE_PTHREADS_INIT)
    set(HAVE_LIBPTHREAD ON)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(SIGCPP REQUIRED IMPORTED_TARGET sigc++-2.0)
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(GLIBMM REQUIRED IMPORTED_TARGET glibmm-2.4)
pkg_check_modules(GIOMM REQUIRED IMPORTED_TARGET giomm-2.4)
pkg_check_modules(XMLPP REQUIRED IMPORTED_TARGET libxml++-2.6)
pkg_search_module(MLT IMPORTED_TARGET mlt++-7 mlt++)
pkg_check_modules(FFTW REQUIRED IMPORTED_TARGET fftw3)
pkg_check_modules(LIBPNG REQUIRED IMPORTED_TARGET libpng) # for mod_png
#TODO(ice0): find solution for libmng
pkg_check_modules(LIBMNG IMPORTED_TARGET libmng) # for mod_mng (set as optional as it is not correctly installed in Debian)
pkg_check_modules(LIBJPEG REQUIRED IMPORTED_TARGET libjpeg) # for mod_mng
pkg_check_modules(OPENEXR IMPORTED_TARGET OpenEXR) # for mod_openexr
pkg_check_modules(MAGICKCORE IMPORTED_TARGET MagickCore) # for Magick++
pkg_check_modules(MAGICKPP IMPORTED_TARGET Magick++)
pkg_check_modules(FREETYPE2 REQUIRED IMPORTED_TARGET freetype2) # for lyr_freetype
pkg_check_modules(FONTCONFIG IMPORTED_TARGET fontconfig) # for FontConfig
pkg_check_modules(FRIBIDI REQUIRED IMPORTED_TARGET fribidi)
pkg_check_modules(HARFBUZZ REQUIRED IMPORTED_TARGET harfbuzz)
pkg_check_modules(LIBAVFORMAT IMPORTED_TARGET libavcodec libavformat libavutil libswresample libswscale)

if (NOT MLT_FOUND)
	add_definitions(-DWITHOUT_MLT) # disable MLT if not found
endif()

##
## Configure
##

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
