target_sources(synfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/layer_bitmap.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_composite.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_composite_fork.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_duplicate.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_filtergroup.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_group.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_invisible.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_meshtransform.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_mime.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_motionblur.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_pastecanvas.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_polygon.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_rendering_task.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_shape.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_solidcolor.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_sound.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_skeleton.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_skeletondeformation.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer_switch.cpp"
)

file(GLOB LAYERS_HEADERS "${CMAKE_CURRENT_LIST_DIR}/*.h")

SYNFIG_INSTALL(
    FILES ${LAYERS_HEADERS}
    DESTINATION include/synfig/layers
)
