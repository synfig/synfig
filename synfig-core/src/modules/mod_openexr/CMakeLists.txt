add_library(mod_openexr MODULE "")

target_sources(mod_openexr
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/trgt_openexr.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mptr_openexr.cpp"
)

message(STATUS "OpenEXR include: ${OPENEXR_INCLUDE_DIRS}")

target_include_directories(mod_openexr SYSTEM PRIVATE ${OPENEXR_INCLUDE_DIRS})

target_link_libraries(mod_openexr synfig ${OPENEXR_LIBRARIES})

SYNFIG_INSTALL(
    TARGETS mod_openexr
    DESTINATION lib/synfig/modules
)
