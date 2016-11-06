target_sources(synfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/color.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/colormatrix.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/cairocolor.cpp"
)

file(GLOB COLOR_HEADERS "${CMAKE_CURRENT_LIST_DIR}/*.h")

install(
    FILES ${COLOR_HEADERS}
    DESTINATION include/synfig/color
)
