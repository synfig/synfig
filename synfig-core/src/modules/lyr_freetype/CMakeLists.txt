add_library(lyr_freetype MODULE "")

target_sources(lyr_freetype
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/lyr_freetype.cpp"
)

install (
    TARGETS lyr_freetype
    DESTINATION lib/synfig/modules
)
