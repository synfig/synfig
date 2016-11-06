cmake_minimum_required(VERSION 2.8)

project(ETL)

find_package(Threads)
if (CMAKE_USE_PTHREADS_INIT)
    set(HAVE_LIBPTHREAD ON)
endif()

add_subdirectory(ETL)
