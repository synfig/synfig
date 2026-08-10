add_executable(test_app_layerduplicate app_layerduplicate.cpp)
target_link_libraries(test_app_layerduplicate PRIVATE synfigapp)
target_include_directories(test_app_layerduplicate PRIVATE ${PROJECT_SOURCE_DIR}/src)
add_test(NAME test_app_layerduplicate COMMAND test_app_layerduplicate)

add_executable(test_smach smach.cpp)
target_link_libraries(test_smach PRIVATE synfigapp libsynfig)
target_include_directories(test_smach PRIVATE ${PROJECT_SOURCE_DIR}/src)
add_test(NAME test_smach COMMAND test_smach)

add_executable(test_json test_json.cpp ${PROJECT_SOURCE_DIR}/src/gui/json.cpp)
target_link_libraries(test_json PRIVATE libsynfig)
target_include_directories(test_json PRIVATE ${PROJECT_SOURCE_DIR}/src)
add_test(NAME test_json COMMAND test_json)

if (NOT WIN32)
set_target_properties(
        test_app_layerduplicate test_smach test_json
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test
)
endif()
