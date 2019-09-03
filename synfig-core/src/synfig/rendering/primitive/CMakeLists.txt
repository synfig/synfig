target_sources(synfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/bend.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/contour.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/intersector.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mesh.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/polyspan.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/transformation.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/transformationaffine.cpp"
)

install_all_headers(rendering/primitive)
