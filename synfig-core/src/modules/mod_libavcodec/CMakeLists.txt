add_library(mod_libavcodec MODULE "")

target_sources(mod_libavcodec
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/mptr.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_av.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(mod_libavcodec synfig)

SYNFIG_INSTALL(
    TARGETS mod_libavcodec
    DESTINATION lib/synfig/modules
)
