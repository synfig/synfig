add_library(mod_ffmpeg MODULE
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_ffmpeg.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mptr_ffmpeg.cpp"
)

target_link_libraries(mod_ffmpeg synfig)

install (
    TARGETS mod_ffmpeg
    DESTINATION lib/synfig/modules
)
