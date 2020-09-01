add_library(mod_magickpp MODULE "")

# Work around for ImageMagick flags issues:
# https://gitlab.kitware.com/cmake/cmake/issues/15007
# https://gitlab.kitware.com/cmake/cmake/issues/14568
# TODO: don't hard-code
# TODO: inspect these values
set(MAGICKCORE_QUANTUM_DEPTH 16)
set(MAGICKCORE_HDRI_ENABLE 0)
configure_file(config.h.cmake.in config.h)

target_include_directories(mod_magickpp BEFORE PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(mod_magickpp SYSTEM PRIVATE ${ImageMagick_Magick++_INCLUDE_DIRS})

target_compile_definitions(mod_magickpp PRIVATE IMAGEMAGICK_CONFIG)

target_sources(mod_magickpp
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_magickpp.cpp"
)

target_link_libraries(mod_magickpp synfig ${MAGICKCORE_LIBRARIES} ${ImageMagick_LIBRARIES})

SYNFIG_INSTALL(
    TARGETS mod_magickpp
    DESTINATION lib/synfig/modules
)
