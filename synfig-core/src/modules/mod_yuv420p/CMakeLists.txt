add_library(mod_yuv420p MODULE "")

target_sources(mod_yuv420p
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_yuv.cpp"
)

target_link_libraries(mod_yuv420p synfig)

SYNFIG_INSTALL(
    TARGETS mod_yuv420p
    DESTINATION lib/synfig/modules
)
