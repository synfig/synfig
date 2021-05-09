target_sources(synfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/color.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/colormatrix.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/pixelformat.cpp"
)

file(GLOB COLOR_HEADERS "${CMAKE_CURRENT_LIST_DIR}/*.h")
set(COLOR_HEADERS ${COLOR_HEADERS} "${CMAKE_CURRENT_LIST_DIR}/color.hpp")

install(
    FILES ${COLOR_HEADERS}
    DESTINATION include/synfig/color
)
