add_library(mod_png MODULE "")

target_sources(mod_png
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mptr_cairo_png.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mptr_png.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_cairo_png.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_png_spritesheet.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_png.cpp"
)

target_link_libraries(mod_png synfig ${LIBPNG_LIBRARIES})

SYNFIG_INSTALL(
    TARGETS mod_png
    DESTINATION lib/synfig/modules
)
