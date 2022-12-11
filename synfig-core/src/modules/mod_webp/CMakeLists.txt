add_library(mod_ffmpeg MODULE
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_webp.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mptr_webp.cpp"
)

target_link_libraries(mod_webp libsynfig)

install (
    TARGETS mod_webp
    DESTINATION lib/synfig/modules
)
