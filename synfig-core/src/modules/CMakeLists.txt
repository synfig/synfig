##
## Includes
##

include_directories(
    SYSTEM
        ${SIGCPP_INCLUDE_DIRS}
        ${CAIRO_INCLUDE_DIRS}
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
    mod_geometry
    mod_gif
    mod_gradient
    mod_imagemagick
    mod_jpeg
#     mod_libavcodec - build failure
#     mod_magickpp - missing include
#     mod_mng - missing include
    mod_noise
#     mod_openexr - missing include
    mod_png
    mod_ppm
    mod_svg
    mod_yuv420p
#     mptr_mplayer - "This code has vulnerabilites"
)

set(SYNFIG_MODULES_CONTENT "")

message("-- Synfig modules:")
foreach(MOD IN ITEMS ${MODS_ENABLED})
    message("--   ${MOD}")
    set(SYNFIG_MODULES_CONTENT "${SYNFIG_MODULES_CONTENT}\n${MOD}")
    add_subdirectory(${MOD})
endforeach(MOD)

file(WRITE synfig_modules.cfg ${SYNFIG_MODULES_CONTENT})

install(
    FILES synfig_modules.cfg
    DESTINATION etc
)
