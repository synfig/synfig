target_sources(libsynfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/taskblend.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/taskblur.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/taskcontour.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/tasklayer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/taskmesh.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/taskpixelprocessor.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/tasktransformation.cpp"
)

install_all_headers(rendering/common/task)