##
## Build targets
##

## Main target: libsynfig
add_library(synfig "")

target_sources(synfig
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/activepoint.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/bone.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/blur.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/cairo_operators.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/cairo_renddesc.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/canvas.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/context.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/curve_helper.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/curveset.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/distance.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/exception.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/gamma.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/guid.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/importer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/cairoimporter.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/keyframe.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/loadcanvas.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/module.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mutex.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/node.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/palette.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/paramdesc.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/polynomial_root.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/rect.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renddesc.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/savecanvas.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/surface.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/target.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/time.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/timepointcollect.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/transform.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/uniqueid.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/valuenode.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/waypoint.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/matrix.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/filesystem.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/filesystemnative.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/filesystemgroup.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/filesystemtemporary.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/filecontainer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/filecontainerzip.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/zstreambuf.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/valueoperations.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mesh.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/soundprocessor.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/canvasfilenaming.cpp"
)

## these were added seprately in autotools build, preserving this for now
## TODO: either merge with main list or create new target
target_sources(synfig
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/target_multi.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/target_null.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/target_null_tile.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/target_scanline.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/target_cairo.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/target_tile.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/target_cairo_tile.cpp"
)
target_sources(synfig
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/listimporter.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/cairolistimporter.cpp"
)
target_sources(synfig
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/blinepoint.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/widthpoint.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dashitem.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/gradient.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/type.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/base_types.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/value.cpp"
)

## include sub-dirs
include(color/CMakeLists.txt)
include(layers/CMakeLists.txt)
include(rendering/CMakeLists.txt)
include(valuenodes/CMakeLists.txt)

target_link_libraries(synfig
    ${SIGCPP_LIBRARIES}
    ${GLIBMM_LIBARIES}
    ${GIOMM_LIBRARIES}
    ${CAIRO_LIBRARIES}
    ${LIBXML_LIBRARIES}
    ${MLT_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

## Command line synfig binary
add_executable(synfig_bin main.cpp)
set_target_properties(synfig_bin PROPERTIES OUTPUT_NAME synfig)
target_link_libraries(synfig_bin synfig)
