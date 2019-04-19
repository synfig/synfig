target_sources(synfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/blur.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/blur_iir_coefficients.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/contour.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/fft.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mesh.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/packedsurface.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/resample.cpp"
)

install_all_headers(rendering/software/function)
