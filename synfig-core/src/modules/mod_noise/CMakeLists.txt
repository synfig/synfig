add_library(mod_noise MODULE "")

target_sources(mod_noise
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/distort.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/random_noise.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/noise.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/valuenode_random.cpp"
)

target_link_libraries(mod_noise synfig ${SIGCPP_LIBRARIES})

SYNFIG_INSTALL(
    TARGETS mod_noise
    DESTINATION lib/synfig/modules
)
