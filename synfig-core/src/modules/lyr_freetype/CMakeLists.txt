add_library(lyr_freetype MODULE
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/lyr_freetype.cpp"
)

if (HARFBUZZ_FOUND)
	target_compile_definitions(lyr_freetype PUBLIC -DHAVE_HARFBUZZ=1)
endif()

if (FRIBIDI_FOUND)
	target_compile_definitions(lyr_freetype PUBLIC -DHAVE_FRIBIDI=1)
endif()

target_include_directories(lyr_freetype PRIVATE ${HARFBUZZ_INCLUDE_DIRS} ${FRIBIDI_INCLUDE_DIRS})

target_link_libraries(lyr_freetype synfig ${FT_LIBRARIES} ${FONT_CONFIG_LIBRARIES} ${HARFBUZZ_LIBRARIES} ${FRIBIDI_LIBRARIES})

install (
    TARGETS lyr_freetype
    DESTINATION lib/synfig/modules
)
