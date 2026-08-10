# fixme(ice0): should be exported by synfig
include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(test_synfig_mod_geometry_advancedline advancedline.cpp ../advancedline.cpp)
target_link_libraries(test_synfig_mod_geometry_advancedline PRIVATE libsynfig)
add_test(NAME test_synfig_mod_geometry_advancedline COMMAND test_synfig_mod_geometry_advancedline)

if (NOT WIN32)
set_target_properties(
        test_synfig_mod_geometry_advancedline
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test
)
endif()
