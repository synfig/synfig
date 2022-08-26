cmake_minimum_required(VERSION 3.5)

project(synfig-core)

# using latest stable standard, but not strictly
# the rest is done via target_compile_features instead
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(src)
add_subdirectory(po)

if (${ENABLE_TESTS})
    enable_testing()
    add_subdirectory(test)
endif()

add_dependencies(synfig_bin
    build_pofiles_synfig
)

# FIXME: should this be here?
if(MSVC)
  file(
    COPY "${CMAKE_CURRENT_SOURCE_DIR}/fonts-msvc/"
    DESTINATION "${SYNFIG_BUILD_ROOT}/etc/fonts")
  install(
    DIRECTORY "${SYNFIG_BUILD_ROOT}/etc/fonts/"
    DESTINATION "etc/fonts")
elseif(CONAN_TOOLCHAIN)
  file(
    COPY "${CONAN_INSTALLS_PREFIX}/etc/fonts/"
    DESTINATION "${SYNFIG_BUILD_ROOT}/etc/fonts")
  install(
    DIRECTORY "${SYNFIG_BUILD_ROOT}/etc/fonts/"
    DESTINATION "etc/fonts")
endif()
