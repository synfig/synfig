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

# vcpkg should be checked before pkg-config, because some vcpkg
# packages have pkg-config support where others is not
if(VCPKG_TOOLCHAIN)
    message(STATUS "Building with vcpkg...")

	set(GLIB_LIB_SUFFIX 2.0)
	set(ATK_LIB_SUFFIX 1.0)
	set(GDK_PIXBUF_LIB_SUFFIX 2.0)
	if(CMAKE_BUILD_TYPE STREQUAL Debug)
	    set(LIB_PNG_SUFFIX d)
	endif()

	# find dependencies

	# Libraries should be named in pkg-config style, so we can use the same names in target_link_libraries and target_include_libraries

	# glib
	find_path(GLIB_INCLUDE_DIR glib.h)
	find_library(GLIB_GLIB_LIBRARY glib-${GLIB_LIB_SUFFIX})
	find_library(GLIB_GIO_LIBRARY gio-${GLIB_LIB_SUFFIX})
	find_library(GLIB_GMODULE_LIBRARY gmodule-${GLIB_LIB_SUFFIX})
	find_library(GLIB_GOBJECT_LIBRARY gobject-${GLIB_LIB_SUFFIX})
	set(GLIB_LIBRARIES ${GLIB_GLIB_LIBRARY} ${GLIB_GIO_LIBRARY} ${GLIB_GMODULE_LIBRARY} ${GLIB_GOBJECT_LIBRARY} CACHE INTERNAL "")

	# atk
	find_path(ATK_INCLUDE_DIR atk/atk.h)
	find_library(ATK_LIBRARY atk-${ATK_LIB_SUFFIX})

	# atkmm (required for gtkmm)
	find_library(ATKMM_LIBRARY atkmm)

	# fftw3
	find_path(FFTW_INCLUDE_DIR fftw3.h)
	find_library(FFTW3_LIBRARY fftw3)
	find_library(FFTW3f_LIBRARY fftw3f)
	find_library(FFTW3l_LIBRARY fftw3l)
	set(FFTW_LIBRARIES ${FFTW3_LIBRARY} ${FFTW3f_LIBRARY} ${FFTW3l_LIBRARY})

	# gdk
	#find_path(GDK_INCLUDE_DIR gdk/gdk.h)
	find_library(GDK_LIBRARY gdk-3.0)
	find_library(GDKMM_LIBRARY gdkmm)
	find_library(GIOMM_LIBRARY giomm)
	find_library(GLIBMM_LIBRARY glibmm-2.0)
	find_library(GTK_LIBRARY gtk-3.0)
	find_library(GTKMM_LIBRARY gtkmm)
	find_library(SIGC_LIBRARY sigc-2.0)
	#find_library(SIGCPP_LIBRARY sigc++-2.0)

    find_library(LIBXML_LIBRARY libxml2)
    find_library(XMLPP_LIBRARY xml++)
	find_library(ZLIB_LIBRARY zlib${LIB_PNG_SUFFIX})
	find_library(LTDL_LIBRARY ltdl)

    #libpng, libjpeg, freetype
    find_library(LIBPNG_LIBRARY libpng16${LIB_PNG_SUFFIX})
    find_library(LIBJPEG_LIBRARIES jpeg${LIB_PNG_SUFFIX})
    find_library(FREETYPE_LIBRARY freetype${LIB_PNG_SUFFIX})

    #set(SIGCPP_LIBRARIES ${SIGC_LIBRARY} ${SIGCPP_LIBRARY})
    set(SIGCPP_LIBRARIES ${SIGC_LIBRARY})
    set(GIOMM_LIBRARIES ${GLIB_GIO_LIBRARY} ${GIOMM_LIBRARY})
    set(GLIBMM_LIBRARIES ${GLIBMM_LIBRARY} ${GLIB_GLIB_LIBRARY})
    set(LIBPNG_LIBRARIES ${LIBPNG_LIBRARY})
    set(FT_LIBRARIES ${FREETYPE_LIBRARY})
    set(XMLPP_LIBRARIES ${LIBXML_LIBRARY} ${XMLPP_LIBRARY})
    set(ZLIB_LIBRARIES ${ZLIB_LIBRARY})

	# pkg-config --libs gtkmm-3.0
    #-LC:/msys64/mingw64/lib -lgtkmm-3.0 -latkmm-1.6 -lgdkmm-3.0 -lgiomm-2.4
    # -lglibmm-2.4 -lgtk-3 -lgdk-3 -lz -lgdi32 -limm32 -lshell32
    # -lharfbuzz -latk-1.0
    # -lgio-2.0 -lsigc-2.0 -lgdk_pixbuf-2.0 -lgobject-2.0 -lglib-2.0 -lintl

    set(GTKMM_LIBRARIES ${ATK_LIBRARY} ${ATKMM_LIBRARY} ${GDKMM_LIBRARY} ${GIOMM_LIBRARY} ${GLIBMM_LIBRARY} ${GTKMM_LIBRARY})

    message(STATUS "+++++++++++++++++++++++ ${GTKMM_LIBRARIES} +++++++++++++++++++++++++++++++")


	# gdk-pixbuf
	find_path(GDK_PIXBUF_INCLUDE_DIR gdk-pixbuf/gdk-pixbuf.h)
	find_library(GDK_PIXBUF_LIBRARY gdk_pixbuf-${GDK_PIXBUF_LIB_SUFFIX})

	# epoxy
	find_path(EPOXY_INCLUDE_DIR epoxy/common.h)
	find_library(EPOXY_LIBRARY epoxy)

	# gettext
	find_path(LIBINTL_INCLUDE_DIR libintl.h)
	find_library(LIBINTL_LIBRARY NAMES intl libintl)

	message(STATUS "")
	message(STATUS "===================================================================================")
	foreach(REQUIRED_LIB
			ATK_LIBRARY
			ATKMM_LIBRARY
			GLIB_GLIB_LIBRARY
			FFTW3_LIBRARY
			FFTW3f_LIBRARY
			FFTW3l_LIBRARY
			GDK_LIBRARY
			GDKMM_LIBRARY
			GIOMM_LIBRARY
			GLIBMM_LIBRARY
			GTK_LIBRARY
			GTKMM_LIBRARY
			SIGC_LIBRARY
			XMLPP_LIBRARY
			ZLIB_LIBRARY
	)
		message(STATUS "Checking for module '${REQUIRED_LIB}'")
		if(${REQUIRED_LIB})
			message(STATUS "  Found ${${REQUIRED_LIB}}")
		else()
			message(STATUS "  NOT FOUND!")
		endif()
	endforeach()
else()
	message(STATUS "Building with pkg-config...")
	include(FindPkgConfig)
    if(PKG_CONFIG_FOUND)
		pkg_check_modules(SIGCPP REQUIRED sigc++-2.0)
		pkg_check_modules(GLIB REQUIRED glib-2.0)
		pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
		pkg_check_modules(GIOMM REQUIRED giomm-2.4)
		pkg_check_modules(LIBXML REQUIRED libxml++-2.6)
		pkg_search_module(MLT mlt++-7 mlt++)
		pkg_check_modules(FFTW REQUIRED fftw3)
		pkg_check_modules(LIBPNG REQUIRED libpng) # for mod_png
		#TODO(ice0): find solution for libmng
		pkg_check_modules(LIBMNG libmng) # for mod_mng (set as optional as it is not correctly installed in Debian)
		pkg_check_modules(LIBJPEG REQUIRED libjpeg) # for mod_mng
		pkg_check_modules(OPENEXR OpenEXR) # for mod_openexr
		pkg_check_modules(MAGICKCORE REQUIRED MagickCore) # for Magick++

		## TODO: move to module where it is actually required
		pkg_check_modules(FT REQUIRED freetype2) # for lyr_freetype
		pkg_check_modules(FONT_CONFIG fontconfig) # for FontConfig
		pkg_check_modules(FRIBIDI REQUIRED fribidi)
		pkg_check_modules(HARFBUZZ REQUIRED harfbuzz)
	endif()
endif()

if (NOT MLT_FOUND)
	add_definitions(-DWITHOUT_MLT) # disable MLT if not found
endif()

foreach(pkg_config_lib 
  SIGCPP 
  GLIBMM 
  GIOMM 
  LIBXML
  MLT 
  FFTW 
  LIBPNG
  LIBMNG
  LIBJPEG
  OPENEXR
  MAGICKCORE)
    include_directories(${${pkg_config_lib}_INCLUDE_DIRS})
    link_directories(${${pkg_config_lib}_LIBRARY_DIRS})
endforeach()

## TODO: should we keep it here?
find_package(ImageMagick COMPONENTS Magick++)

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
