add_library(mod_svg MODULE
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_svg.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/svg_parser.cpp"
)

target_link_libraries(mod_svg PRIVATE libsynfig)

install (
    TARGETS mod_svg
    DESTINATION lib/synfig/modules
)
