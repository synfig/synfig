add_library(mod_mng MODULE "")

target_sources(mod_mng
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/trgt_mng.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

install (
    TARGETS mod_mng
    DESTINATION lib/synfig/modules
)
