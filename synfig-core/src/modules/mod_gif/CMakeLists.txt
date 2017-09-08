add_library(mod_gif MODULE "")

target_sources(mod_gif
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/trgt_gif.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

install (
    TARGETS mod_gif
    DESTINATION lib/synfig/modules
)
