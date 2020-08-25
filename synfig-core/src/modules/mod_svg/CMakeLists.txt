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

target_link_libraries(mod_svg synfig ${LIBXML_LIBRARIES} ${GLIBMM_LIBRARIES})

SYNFIG_INSTALL(
    TARGETS mod_svg
    DESTINATION lib/synfig/modules
)
