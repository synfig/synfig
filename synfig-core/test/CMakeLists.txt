# for test to find generated config.h
include_directories(${PROJECT_BINARY_DIR})

# for synfig-core headers
include_directories(${PROJECT_SOURCE_DIR})

add_executable(angle angle.cpp)
add_test(NAME test_angle COMMAND angle)

add_executable(fixed fixed.cpp)
add_test(NAME test_fixed COMMAND fixed)

add_executable(clock clock.cpp)
add_test(NAME test_clock COMMAND clock)

add_executable(handle handle.cpp)
add_test(NAME test_handle COMMAND handle)

add_executable(hermite hermite.cpp)
add_test(NAME test_hermite COMMAND hermite)

add_executable(stringf stringf.cpp)
add_test(NAME test_stringf COMMAND stringf)

add_executable(pen pen.cpp)
add_test(NAME test_pen COMMAND pen)

add_executable(surface surface.cpp)
add_test(NAME test_surface COMMAND surface)

add_executable(smart_ptr smart_ptr.cpp)
add_test(NAME test_smart_ptr COMMAND smart_ptr)

add_executable(benchmark benchmark.cpp)
add_test(NAME test_benchmark COMMAND benchmark)

add_executable(bline bline.cpp)
add_test(NAME test_bline COMMAND bline)

add_executable(bone bone.cpp)
add_test(NAME test_bone COMMAND bone)
set_target_properties(
		angle fixed clock handle hermite stringf pen surface smart_ptr benchmark bline bone
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test
)

