add_library(mod_bmp MODULE
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mptr_bmp.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_bmp.cpp"
)

target_link_libraries(mod_bmp synfig)

install (
    TARGETS mod_bmp
    DESTINATION lib/synfig/modules
)
