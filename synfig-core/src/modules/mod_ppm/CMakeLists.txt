add_library(mod_ppm MODULE
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_ppm.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mptr_ppm.cpp"
)

target_link_libraries(mod_ppm synfig)

install (
    TARGETS mod_ppm
    DESTINATION lib/synfig/modules
)
