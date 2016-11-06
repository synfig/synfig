add_library(mod_svg MODULE "")

include_directories(
    SYSTEM
        ${GLIBMM_INCLUDE_DIRS}
        ${LIBXML_INCLUDE_DIRS}
)

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
