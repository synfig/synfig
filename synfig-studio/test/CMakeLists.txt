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
