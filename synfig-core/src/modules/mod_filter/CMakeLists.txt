add_library(mod_filter MODULE "")

target_sources(mod_filter
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/halftone3.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/blur.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/lumakey.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/halftone2.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/halftone.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/colorcorrect.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/radialblur.cpp"
)

target_link_libraries(mod_filter synfig ${CAIRO_LIBRARIES})

SYNFIG_INSTALL(
    TARGETS mod_filter
    DESTINATION lib/synfig/modules
)
