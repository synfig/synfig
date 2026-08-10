target_sources(libsynfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/surfacegl.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderergl.cpp"
)

add_definitions("-DWITH_OPENGL")
include(${CMAKE_CURRENT_LIST_DIR}/internal/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/task/CMakeLists.txt)

install_all_headers(rendering/opengl)
