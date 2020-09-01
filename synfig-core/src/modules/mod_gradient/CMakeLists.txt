add_library(mod_gradient MODULE "")

target_sources(mod_gradient
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/conicalgradient.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/radialgradient.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/curvegradient.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/lineargradient.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/spiralgradient.cpp"
)

target_link_libraries(mod_gradient synfig ${CAIRO_LIBRARIES})

SYNFIG_INSTALL(
    TARGETS mod_gradient
    DESTINATION lib/synfig/modules
)
