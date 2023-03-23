target_sources(libsynfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/optimizer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderqueue.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/resource.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/surface.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/task.cpp"
)

file(GLOB RENDERING_HEADERS "${CMAKE_CURRENT_LIST_DIR}/*.h")

install(
    FILES ${RENDERING_HEADERS}
    DESTINATION include/synfig/rendering
)

## TODO: make configurable
include(${CMAKE_CURRENT_LIST_DIR}/common/CMakeLists.txt)

if(WITH_OPENGL)
	include(${CMAKE_CURRENT_LIST_DIR}/opengl/CMakeLists.txt)
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DWITH_OPENGL")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DWITH_OPENGL")
endif(WITH_OPENGL)

include(${CMAKE_CURRENT_LIST_DIR}/primitive/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/software/CMakeLists.txt)
