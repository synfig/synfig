# fixme(ice0): should be exported by synfig
include_directories(${PROJECT_SOURCE_DIR}/src)

# Google Test based tests
if (TARGET gtest_main)
    set(GTEST_SOURCES
        gtest_angle
        gtest_benchmark
        gtest_bezier
        gtest_bline
        gtest_bone
        gtest_clock
        gtest_filesystem_path
        gtest_handle
        gtest_keyframe
        gtest_node
        gtest_pen
        gtest_reference_counter
        gtest_string
        gtest_surface_etl
        gtest_valuenode_maprange
        gtest_golden_image
        gtest_file_io_sif
        gtest_file_io_sifz
        gtest_file_io_errors
        gtest_file_io_recovery
        gtest_render_basic
        gtest_render_blend
        gtest_export_format
        gtest_export_video
    )

    foreach(TEST_NAME ${GTEST_SOURCES})
        add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
        target_link_libraries(${TEST_NAME} PRIVATE libsynfig GTest::gtest_main)
        target_include_directories(${TEST_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()

    if (NOT WIN32)
        set_target_properties(${GTEST_SOURCES}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test
        )
    endif()

    # Copy test fixtures to build directory
    file(GLOB TEST_FIXTURES "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/*")
    file(COPY ${TEST_FIXTURES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/fixtures/)
endif()
