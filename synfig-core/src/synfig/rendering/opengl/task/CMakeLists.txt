target_sources(libsynfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/taskgl.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/taskpixelcolormatrixgl.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/taskblendgl.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/taskblurgl.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/taskpixelgammagl.cpp"
        # "${CMAKE_CURRENT_LIST_DIR}/taskchromakeygl.cpp"
)
