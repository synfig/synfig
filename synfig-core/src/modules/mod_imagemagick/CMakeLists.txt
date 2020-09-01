add_library(mod_imagemagick MODULE "")

target_sources(mod_imagemagick
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/mptr_imagemagick.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_imagemagick.cpp"
)

target_link_libraries(mod_imagemagick synfig)

SYNFIG_INSTALL(
    TARGETS mod_imagemagick
    DESTINATION lib/synfig/modules
)
