# for test to find generated etl_profile.h
include_directories(${PROJECT_BINARY_DIR})

include_directories(${PROJECT_SOURCE_DIR})

add_executable(angle angle.cpp)
add_test(test_angle angle)

add_executable(fixed fixed.cpp)
add_test(test_fixed fixed)

add_executable(clock clock.cpp)
add_test(test_clock clock)

add_executable(handle handle.cpp)
add_test(test_handle handle)

add_executable(hermite hermite.cpp)
add_test(test_hermite hermite)

add_executable(stringf stringf.cpp)
add_test(test_stringf stringf)

add_executable(pen pen.cpp)
add_test(test_pen pen)

add_executable(surface surface.cpp)
add_test(test_surface surface)

add_executable(smart_ptr smart_ptr.cpp)
add_test(test_smart_ptr smart_ptr)

add_executable(benchmark benchmark.cpp)
add_test(test_benchmark benchmark)

add_executable(spline spline.cpp)
add_test(test_spline spline)

add_executable(value value.cpp)
add_test(test_value value)
