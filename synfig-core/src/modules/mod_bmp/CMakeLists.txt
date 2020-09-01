add_library(mod_bmp MODULE "")

target_sources(mod_bmp
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mptr_bmp.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_bmp.cpp"
)

target_link_libraries(mod_bmp synfig ${SIGCPP_LIBRARIES})

SYNFIG_INSTALL(
    TARGETS mod_bmp
    DESTINATION lib/synfig/modules
)
