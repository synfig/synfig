##
## Build targets
##

## Main target: libsynfig
## TODO: optional static linking?
add_library(libsynfig SHARED "")
set_target_properties(libsynfig PROPERTIES PREFIX "")

add_definitions(-DVERSION="${STUDIO_VERSION_MAJOR}.${STUDIO_VERSION_MINOR}.${STUDIO_VERSION_PATCH}")

message(STATUS "MLT DIR: ${MLT_INCLUDE_DIRS}")

# By default, the libtool places its headers in the system directory (/usr/local/include), and
# compiler automatically finds ltdl.h header. But on MacOS, CMake overrides the system directory
# using the `-sysroot` flag, and this header cannot be found. (https://gitlab.kitware.com/cmake/cmake/-/issues/19120)
# So, we need to manually find it.

if(APPLE OR VCPKG_TOOLCHAIN)
    find_path(LTDL_INCLUDE_DIRS ltdl.h)
    find_library(LTDL_LIBRARIES ltdl)
    message(STATUS "Libltdl headers:   ${LTDL_INCLUDE_DIRS}")
    message(STATUS "Libltdl libraries: ${LTDL_LIBRARIES}")
else()
    set(LTDL_LIBRARIES ltdl)
endif()

target_include_directories(libsynfig PUBLIC ${LTDL_INCLUDE_DIRS})

target_sources(libsynfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/activepoint.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/bone.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/blur.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/canvas.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/context.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/curve_helper.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/curveset.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/distance.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/exception.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/guid.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/importer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/keyframe.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/loadcanvas.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/module.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/node.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/os.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/palette.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/paramdesc.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/polynomial_root.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/rect.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renddesc.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/savecanvas.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/string_helper.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/synfig_iterations.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/surface.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/target.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/time.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/timepointcollect.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/transform.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/uniqueid.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/valuenode.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/valuenode_registry.cpp"
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
        "${CMAKE_CURRENT_LIST_DIR}/soundprocessor.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/canvasfilenaming.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/token.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/threadpool.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/curve.cpp"
)

## these were added seprately in autotools build, preserving this for now
## TODO: either merge with main list or create new target
target_sources(libsynfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/target_multi.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/target_scanline.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/target_tile.cpp"
)
target_sources(libsynfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/listimporter.cpp"
)
target_sources(libsynfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/blinepoint.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/widthpoint.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dashitem.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/gradient.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/type.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/base_types.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/value.cpp"
)

##
## Sub-dirs
##

## include sub-dirs
include(color/CMakeLists.txt)
include(layers/CMakeLists.txt)
include(rendering/CMakeLists.txt)
include(valuenodes/CMakeLists.txt)

## TODO: check if we need this for release build
include(debug/CMakeLists.txt)

target_link_libraries(libsynfig PUBLIC
	PkgConfig::SIGCPP
	PkgConfig::GLIBMM
	PkgConfig::GIOMM
	PkgConfig::XMLPP
	PkgConfig::FFTW
	ZLIB::ZLIB
	${LTDL_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${Intl_LIBRARIES}
)

if (MLT_FOUND)
	target_link_libraries(libsynfig PUBLIC PkgConfig::MLT)
endif ()

## Install headers
## TODO: find a better way to do that, maybe?
file(GLOB SYNFIG_HEADERS "${CMAKE_CURRENT_LIST_DIR}/*.h")

install(
    FILES ${SYNFIG_HEADERS}
    DESTINATION include/synfig
)

install(
    TARGETS libsynfig
    EXPORT libsynfig
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

export(
    EXPORT libsynfig
    FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-config.cmake"
)

install(
    EXPORT libsynfig
    FILE "${PROJECT_NAME}-config.cmake"
    DESTINATION "lib/cmake/${PROJECT_NAME}"
)
