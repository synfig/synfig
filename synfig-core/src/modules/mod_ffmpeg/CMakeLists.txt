add_library(mod_ffmpeg MODULE "")

target_sources(mod_ffmpeg
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_ffmpeg.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mptr_ffmpeg.cpp"
)

target_link_libraries(mod_ffmpeg synfig ${SIGCPP_LIBRARIES})

SYNFIG_INSTALL(
    TARGETS mod_ffmpeg
    DESTINATION lib/synfig/modules
)
