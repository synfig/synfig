add_library(mod_magickpp MODULE
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_magickpp.cpp"
)

target_compile_options(mod_magickpp PRIVATE ${MAGICKCORE_CFLAGS})
target_link_libraries(mod_magickpp synfig ${MAGICKCORE_LINK_LIBRARIES} ${ImageMagick_LIBRARIES})

install (
    TARGETS mod_magickpp
    DESTINATION lib/synfig/modules
)
