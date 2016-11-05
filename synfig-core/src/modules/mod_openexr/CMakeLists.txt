add_library(mod_openexr MODULE "")

target_sources(mod_openexr
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/trgt_openexr.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mptr_openexr.cpp"
)

install (
    TARGETS mod_openexr
    DESTINATION lib/synfig/modules
)
