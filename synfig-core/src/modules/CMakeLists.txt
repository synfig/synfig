##
## Includes
##

include_directories(
    SYSTEM
        ${SIGCPP_INCLUDE_DIRS}
        ${CAIRO_INCLUDE_DIRS}
        ${GLIBMM_INCLUDE_DIRS}
        ${GIOMM_INCLUDE_DIRS}
)

##
## Modules
##

## TODO: make configurable
## TODO: write some macros

set(MODS_ENABLED
    lyr_freetype
    lyr_std
    mod_bmp
    mod_dv
    mod_example
    mod_ffmpeg
    mod_filter
    mod_geometry
    mod_gif
    mod_gradient
    mod_imagemagick
    mod_jpeg
#    mod_libavcodec # - build failure
#    mod_magickpp # - made optional
    mod_noise
    mod_openexr
    mod_particle
    mod_png
    mod_ppm
    mod_svg
    mod_yuv420p
#    mptr_mplayer # - "This code has vulnerabilities"
)
if(LIBMNG_FOUND)
  list(APPEND MODS_ENABLED mod_mng)
else()
  message(STATUS "mod_mng: Disabled")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${SYNFIG_BUILD_ROOT}/lib/synfig/modules)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SYNFIG_BUILD_ROOT}/lib/synfig/modules)

## Magick++ support
option(WITH_MAGICPP "Enable Magic++ module (output format support)" OFF)
option(WITHOUT_MAGICPP "Disable Magic++ module (output format support)" OFF)

if (${WITH_MAGICPP} AND ${WITHOUT_MAGICPP})
    message(FATAL_ERROR "You cannot request both WITH_MAGICPP and WITHOUT_MAGICPP!")
endif()

if (NOT ${WITHOUT_MAGICPP} AND ImageMagick_Magick++_FOUND)
    list(APPEND MODS_ENABLED mod_magickpp)
elseif (${WITH_MAGICPP} AND NOT ImageMagick_Magick++_FOUND)
    message(SEND_ERROR "Magic++ support is requested, but the package cannot be located.")
endif()
## Magick++ support (end)

## Process selected modules
set(SYNFIG_MODULES_CONTENT "")

message("-- Synfig modules:")
foreach(MOD IN ITEMS ${MODS_ENABLED})
    message("--   ${MOD}")
    set(SYNFIG_MODULES_CONTENT "${SYNFIG_MODULES_CONTENT}\n${MOD}")
    add_subdirectory(${MOD})
endforeach(MOD)

file(WRITE synfig_modules.cfg ${SYNFIG_MODULES_CONTENT})

configure_file(
    synfig_modules.cfg
    ${SYNFIG_BUILD_ROOT}/etc/synfig_modules.cfg
    COPYONLY
)

SYNFIG_INSTALL(
    FILES synfig_modules.cfg
    DESTINATION etc
)
