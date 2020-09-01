add_library(lyr_freetype MODULE "")

include_directories(
    SYSTEM
        ${PANGO_INCLUDE_DIRS}
)

target_sources(lyr_freetype
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/lyr_freetype.cpp"
)

target_link_libraries(lyr_freetype synfig ${CAIRO_LIBRARIES} ${PANGO_LIBRARIES} ${PANGOCAIRO_LIBRARIES} ${FT_LIBRARIES} ${FONT_CONFIG_LIBRARIES})

SYNFIG_INSTALL(
    TARGETS lyr_freetype
    DESTINATION lib/synfig/modules
)
