target_sources(libsynfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/renderergl.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/surfacegl.cpp"
)

include(${CMAKE_CURRENT_LIST_DIR}/task/CMakeLists.txt)

install_all_headers(rendering/software)
