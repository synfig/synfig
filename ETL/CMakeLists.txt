cmake_minimum_required(VERSION 2.8)

project(ETL)

option(ENABLE_TESTS "Enable tests" OFF)

find_package(Threads)
if (CMAKE_USE_PTHREADS_INIT)
    set(HAVE_LIBPTHREAD ON)
endif()

## for tests
include_directories(${CMAKE_SOURCE_DIR})

## for test to find generated etl_profile.h
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(ETL)

if (${ENABLE_TESTS})
    enable_testing()
    add_subdirectory(test)
endif()
