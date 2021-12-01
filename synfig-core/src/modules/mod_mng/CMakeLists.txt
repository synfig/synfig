add_library(mod_mng MODULE
        "${CMAKE_CURRENT_LIST_DIR}/trgt_mng.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_compile_options(mod_mng PRIVATE ${LIBMNG_CFLAGS})
target_link_libraries(mod_mng PRIVATE synfig ${LIBMNG_LINK_LIBRARIES})

install (
    TARGETS mod_mng
    DESTINATION lib/synfig/modules
)
