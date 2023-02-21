add_library(mod_magickpp MODULE
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_magickpp.cpp"
)

target_link_libraries(mod_magickpp libsynfig PkgConfig::MAGICKCORE PkgConfig::MAGICKPP)

install (
    TARGETS mod_magickpp
    DESTINATION lib/synfig/modules
)
