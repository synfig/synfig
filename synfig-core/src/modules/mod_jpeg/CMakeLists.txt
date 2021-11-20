add_library(mod_jpeg MODULE
        "${CMAKE_CURRENT_LIST_DIR}/mptr_jpeg.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_jpeg.cpp"
)

target_compile_options(mod_jpeg PRIVATE ${LIBJPEG_CFLAGS})
target_link_libraries(mod_jpeg synfig ${LIBJPEG_LINK_LIBRARIES})

install (
    TARGETS mod_jpeg
    DESTINATION lib/synfig/modules
)
