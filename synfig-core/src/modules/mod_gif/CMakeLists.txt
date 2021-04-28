add_library(mod_gif MODULE
        "${CMAKE_CURRENT_LIST_DIR}/trgt_gif.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(mod_gif synfig)

install (
    TARGETS mod_gif
    DESTINATION lib/synfig/modules
)
