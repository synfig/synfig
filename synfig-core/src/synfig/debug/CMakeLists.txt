target_sources(libsynfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/debugsurface.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/log.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/measure.cpp"
)

file(GLOB DEBUG_HEADERS "${CMAKE_CURRENT_LIST_DIR}/*.h")

install(
    FILES ${DEBUG_HEADERS}
    DESTINATION include/synfig/debug
)
