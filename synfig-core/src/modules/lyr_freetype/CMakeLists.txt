add_library(lyr_freetype MODULE
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/lyr_freetype.cpp"
)

if(FONTCONFIG_FOUND)
	add_definitions(-DWITH_FONTCONFIG)
	target_link_libraries(lyr_freetype PkgConfig::FONTCONFIG)
endif()

if (HARFBUZZ_FOUND)
	target_compile_definitions(lyr_freetype PUBLIC -DHAVE_HARFBUZZ=1)
	target_link_libraries(lyr_freetype PkgConfig::HARFBUZZ)
endif()

if (FRIBIDI_STATIC_LIBRARIES)
	target_compile_definitions(lyr_freetype PRIVATE -DFRIBIDI_LIB_STATIC)
endif()

target_link_libraries(lyr_freetype libsynfig
	PkgConfig::FREETYPE2
	PkgConfig::FRIBIDI
)

install (
    TARGETS lyr_freetype
    DESTINATION lib/synfig/modules
)
