# for test to find generated etl_profile.h
include_directories(${PROJECT_BINARY_DIR})

# for ETL headers
include_directories(${PROJECT_SOURCE_DIR})

add_executable(handle handle.cpp)
add_test(NAME test_handle COMMAND handle)

add_executable(hermite hermite.cpp)
add_test(NAME test_hermite COMMAND hermite)

# temporary dependency for `etl::current_working_directory()`
include(FindPkgConfig)
if(PKG_CONFIG_FOUND)
	pkg_check_modules(GLIBMM REQUIRED IMPORTED_TARGET glibmm-2.4)
endif()

add_executable(stringf stringf.cpp)
target_link_libraries(stringf PRIVATE PkgConfig::GLIBMM)
add_test(NAME test_stringf COMMAND stringf)

add_executable(pen pen.cpp)
add_test(NAME test_pen COMMAND pen)

add_executable(surface surface.cpp)
add_test(NAME test_surface COMMAND surface)

set_target_properties(
		handle hermite stringf pen surface
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test
)

