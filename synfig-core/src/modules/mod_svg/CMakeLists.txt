add_library(mod_svg MODULE "")

target_sources(mod_svg
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_svg.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/svg_parser.cpp"
)

install (
    TARGETS mod_svg
    DESTINATION lib/synfig/modules
)
