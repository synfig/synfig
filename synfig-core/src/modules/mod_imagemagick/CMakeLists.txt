add_library(mod_imagemagick MODULE
        "${CMAKE_CURRENT_LIST_DIR}/mptr_imagemagick.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_imagemagick.cpp"
)

target_link_libraries(mod_imagemagick synfig)

install (
    TARGETS mod_imagemagick
    DESTINATION lib/synfig/modules
)
