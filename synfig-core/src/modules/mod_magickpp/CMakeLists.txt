add_library(mod_magickpp MODULE "")

include_directories(SYSTEM ${ImageMagick_Magick++_INCLUDE_DIRS})

target_sources(mod_magickpp
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_magickpp.cpp"
)

install (
    TARGETS mod_magickpp
    DESTINATION lib/synfig/modules
)
