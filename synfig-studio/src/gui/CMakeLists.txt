##
## Configure
##

set(STUDIO_VERSION "${STUDIO_VERSION_MAJOR}.${STUDIO_VERSION_MINOR}.${STUDIO_VERSION_PATCH}")
set(SHOW_EXTRA_INFO OFF) # this needs some fixes to work
set(DATA_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE PATH "Path, where the share directory with the pixmaps, sounds, plugins and translations is located. Default: Prefix, where Synfig Studio is going to be installed")
set(IMG_EXT "png" CACHE STRING "File extension of the pixmaps (without the \".\"). Default: png")

configure_file(autorevision.h.cmake.in autorevision.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## Defines
set(SYNFIG_DATA_PREFIX "${DATA_PREFIX}/share/synfig")

add_definitions(-DIMAGE_EXT="${IMG_EXT}")
add_definitions(-DIMAGE_DIR="${DATA_PREFIX}/share/pixmaps")
add_definitions(-DSYNFIG_DATADIR="${SYNFIG_DATA_PREFIX}")
add_definitions(-DLOCALEDIR="${DATA_PREFIX}/share/locale")
add_definitions(-DCMAKE_BUILD)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions("-DSHOW_EXTRA_INFO")
    add_definitions(-DETL_VERSION="${ETL_VERSION}")
endif()

##
## Targets
##

if(WIN32)
    configure_file(synfigstudio.rc ${CMAKE_CURRENT_BINARY_DIR}/synfigstudio.rc)
    add_executable(synfigstudio main.cpp ${CMAKE_CURRENT_BINARY_DIR}/synfigstudio.rc)
else()
    add_executable(synfigstudio main.cpp)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_sources(synfigstudio
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/duck.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/timemodel.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/app.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/asyncrenderer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/audiocontainer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/autorecover.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/canvasview.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/devicetracker.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dialogsettings.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/duckmatic.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/iconcontroller.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/instance.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ipc.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/keymapsettings.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/onemoment.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/pluginmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/preview.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/progresslogger.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renddesc.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/resourcehelper.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/splash.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/statemanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/timeplotdata.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/valuelink.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/waypointrenderer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/workarea.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/workspacehandler.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main_win32.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mainwindow.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/helpers.cpp"
)

include(actionmanagers/CMakeLists.txt)
include(cellrenderer/CMakeLists.txt)
include(dialogs/CMakeLists.txt)
include(dials/CMakeLists.txt)
include(docks/CMakeLists.txt)
include(modules/CMakeLists.txt)
include(states/CMakeLists.txt)
include(trees/CMakeLists.txt)
include(widgets/CMakeLists.txt)
include(workarearenderer/CMakeLists.txt)

target_include_directories(synfigstudio PRIVATE ${MLT_INCLUDE_DIRS}) # required for widget_soundwave

target_link_libraries(synfigstudio
    ${GTKMM_LIBRARIES}
    ${Gettext_LIBRARIES}
    ${MLT_LIBRARIES} # required for widget_soundwave
    ${LIBXML_LIBRARIES} # required for gui/pluginmanager
    synfig
    synfigapp
)

SYNFIG_INSTALL(
    TARGETS synfigstudio
    DESTINATION bin
)
