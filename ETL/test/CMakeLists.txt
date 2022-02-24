# for test to find generated etl_profile.h
include_directories(${PROJECT_BINARY_DIR})

# for ETL headers
include_directories(${PROJECT_SOURCE_DIR})

add_executable(angle angle.cpp)
add_test(NAME test_angle COMMAND angle)

add_executable(clock clock.cpp)
add_test(NAME test_clock COMMAND clock)

add_executable(handle handle.cpp)
add_test(NAME test_handle COMMAND handle)

add_executable(hermite hermite.cpp)
add_test(NAME test_hermite COMMAND hermite)

# temporary dependency for `etl::current_working_directory()`
include(FindPkgConfig)
if(PKG_CONFIG_FOUND)
	pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
endif()

add_executable(stringf stringf.cpp)
target_include_directories(stringf PRIVATE ${GLIBMM_INCLUDE_DIRS})
target_link_libraries(stringf PRIVATE ${GLIBMM_LDFLAGS})
add_test(NAME test_stringf COMMAND stringf)

add_executable(pen pen.cpp)
add_test(NAME test_pen COMMAND pen)

add_executable(surface surface.cpp)
add_test(NAME test_surface COMMAND surface)

add_executable(smart_ptr smart_ptr.cpp)
add_test(NAME test_smart_ptr COMMAND smart_ptr)

add_executable(benchmark benchmark.cpp)
add_test(NAME test_benchmark COMMAND benchmark)

set_target_properties(
		angle clock handle hermite stringf pen surface smart_ptr benchmark
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test
)

