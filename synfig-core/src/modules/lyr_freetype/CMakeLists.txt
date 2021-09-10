add_library(lyr_freetype MODULE
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/lyr_freetype.cpp"
)

if(FONT_CONFIG_FOUND)
  add_definitions(-DWITH_FONTCONFIG)
endif()

if (HARFBUZZ_FOUND)
	target_compile_definitions(lyr_freetype PUBLIC -DHAVE_HARFBUZZ=1)
endif()

target_include_directories(lyr_freetype PRIVATE ${FT_INCLUDE_DIRS} ${FONT_CONFIG_INCLUDE_DIRS} ${HARFBUZZ_INCLUDE_DIRS} ${FRIBIDI_INCLUDE_DIRS})

target_link_libraries(lyr_freetype synfig ${FT_LDFLAGS} ${FONT_CONFIG_LDFLAGS} ${HARFBUZZ_LDFLAGS} ${FRIBIDI_LDFLAGS})

install (
    TARGETS lyr_freetype
    DESTINATION lib/synfig/modules
)
