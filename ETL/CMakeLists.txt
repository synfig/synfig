cmake_minimum_required(VERSION 3.5)

project(ETL)

option(ENABLE_TESTS "Enable tests" OFF)

add_subdirectory(ETL)

if (${ENABLE_TESTS})
    enable_testing()
    add_subdirectory(test)
endif()
