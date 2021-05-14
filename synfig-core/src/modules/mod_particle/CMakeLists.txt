add_library(mod_particle MODULE
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/random.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/plant.cpp"
)

target_link_libraries(mod_particle synfig)

install (
    TARGETS mod_particle
    DESTINATION lib/synfig/modules
)
