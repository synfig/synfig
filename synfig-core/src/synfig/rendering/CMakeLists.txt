target_sources(synfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/optimizer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderqueue.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/resource.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/surface.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/task.cpp"
)

file(GLOB RENDERING_HEADERS "${CMAKE_CURRENT_LIST_DIR}/*.h")

SYNFIG_INSTALL(
    FILES ${RENDERING_HEADERS}
    DESTINATION include/synfig/rendering
)

## TODO: make configurable
include(${CMAKE_CURRENT_LIST_DIR}/common/CMakeLists.txt)
# include(${CMAKE_CURRENT_LIST_DIR}/opengl/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/primitive/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/software/CMakeLists.txt)
