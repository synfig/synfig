cmake_minimum_required(VERSION 3.0)

project(ETL)

find_package(Threads)
if (CMAKE_USE_PTHREADS_INIT)
    set(HAVE_LIBPTHREAD ON)
endif()

include(CheckCXXSymbolExists)
check_cxx_symbol_exists(vsnprintf stdio.h HAVE_VSNPRINTF)

## for tests
include_directories(${CMAKE_SOURCE_DIR})

## for test to find generated etl_profile.h
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(ETL)

enable_testing()
add_subdirectory(test)
