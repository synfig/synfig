add_library(lyr_freetype MODULE
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/lyr_freetype.cpp"
)

target_include_directories(lyr_freetype PRIVATE ${PANGO_INCLUDE_DIRS})

target_link_libraries(lyr_freetype synfig ${CAIRO_LIBRARIES} ${PANGO_LIBRARIES} ${PANGOCAIRO_LIBRARIES} ${FT_LIBRARIES} ${FONT_CONFIG_LIBRARIES})

install (
    TARGETS lyr_freetype
    DESTINATION lib/synfig/modules
)
