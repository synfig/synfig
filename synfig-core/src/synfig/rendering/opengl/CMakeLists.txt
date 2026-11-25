target_sources(libsynfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/renderergl.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/surfacegl.cpp"
)

# glfw
find_package(glfw3 3.3 REQUIRED)
target_link_libraries(libsynfig PRIVATE glfw)

# glad
include(${CMAKE_CURRENT_LIST_DIR}/glad/CMakeLists.txt)

include(${CMAKE_CURRENT_LIST_DIR}/internal/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/task/CMakeLists.txt)

install_all_headers(rendering/opengl)
