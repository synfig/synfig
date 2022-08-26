cmake_minimum_required(VERSION 2.8.12)
project(cmake_wrapper)

include(conanbuildinfo.cmake)
conan_basic_setup(KEEP_RPATHS)

set(THIRD_PARTY_LIBS ${CONAN_LIBS})

add_subdirectory(source_subfolder)
