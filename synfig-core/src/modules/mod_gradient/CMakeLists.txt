add_library(mod_gradient MODULE
        "${CMAKE_CURRENT_LIST_DIR}/conicalgradient.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/radialgradient.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/curvegradient.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/lineargradient.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/spiralgradient.cpp"
)

target_link_libraries(mod_gradient synfig)

install (
    TARGETS mod_gradient
    DESTINATION lib/synfig/modules
)
