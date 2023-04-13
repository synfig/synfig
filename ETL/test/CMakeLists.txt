# fixme(ice0): ETL should export this paths
# for test to find generated etl_profile.h
include_directories(${PROJECT_BINARY_DIR})

# for ETL headers
include_directories(${PROJECT_SOURCE_DIR})

add_executable(handle handle.cpp)
add_test(NAME test_handle COMMAND handle)

# temporary dependency for `etl::current_working_directory()`
include(FindPkgConfig)
if(PKG_CONFIG_FOUND)
	pkg_check_modules(GLIBMM REQUIRED IMPORTED_TARGET glibmm-2.4)
endif()

set_target_properties(
		handle
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test
)

