add_library(mod_magickpp MODULE "")

target_sources(mod_magickpp
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_magickpp.cpp"
)

install (
    TARGETS mod_magickpp
    DESTINATION lib/synfig/modules
)
