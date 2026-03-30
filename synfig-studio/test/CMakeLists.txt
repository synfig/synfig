add_executable(test_app_layerduplicate app_layerduplicate.cpp)
target_link_libraries(test_app_layerduplicate PRIVATE synfigapp)
target_include_directories(test_app_layerduplicate PRIVATE ${PROJECT_SOURCE_DIR}/src)
add_test(NAME test_app_layerduplicate COMMAND test_app_layerduplicate)

add_executable(test_smach smach.cpp)
target_link_libraries(test_smach PRIVATE synfigapp libsynfig)
target_include_directories(test_smach PRIVATE ${PROJECT_SOURCE_DIR}/src)
add_test(NAME test_smach COMMAND test_smach)

if (NOT WIN32)
set_target_properties(
        test_app_layerduplicate test_smach
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test
)
endif()

# Google Test based tests
if (TARGET gtest_main)
    set(GTEST_STUDIO_SOURCES
        gtest_app_layerduplicate
        gtest_smach
    )

    foreach(TEST_NAME ${GTEST_STUDIO_SOURCES})
        add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
        target_link_libraries(${TEST_NAME} PRIVATE synfigapp libsynfig GTest::gtest_main)
        target_include_directories(${TEST_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()

    if (NOT WIN32)
        set_target_properties(${GTEST_STUDIO_SOURCES}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test
        )
    endif()
endif()
