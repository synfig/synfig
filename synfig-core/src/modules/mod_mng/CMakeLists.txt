add_library(mod_mng MODULE "")

target_sources(mod_mng
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/trgt_mng.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_compile_options(mod_mng PRIVATE ${LIBMNG_CFLAGS})
target_link_libraries(mod_mng synfig ${LIBMNG_LIBRARIES})

SYNFIG_INSTALL(
    TARGETS mod_mng
    DESTINATION lib/synfig/modules
)
