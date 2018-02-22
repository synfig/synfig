##
## Configure
##

set(STUDIO_VERSION "1.3.5")
set(SHOW_EXTRA_INFO OFF) # this needs some fixes to work
configure_file(autorevision.h.cmake.in autorevision.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

##
## Targets
##

add_executable(synfigstudio main.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(
	synfigstudio 
	PRIVATE
	IMAGE_DIR="${CMAKE_INSTALL_PREFIX}/share/pixmaps")

target_sources(synfigstudio
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/duck.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/adjust_window.cpp"
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
        "${CMAKE_CURRENT_LIST_DIR}/preview.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renddesc.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/splash.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/statemanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/valuelink.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/workarea.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main_win32.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mainwindow.cpp"
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

target_link_libraries(synfigstudio
    ${GTKMM_LIBRARIES}
    ${Gettext_LIBRARIES}
    synfig
    synfigapp
)

install(
    TARGETS synfigstudio
    DESTINATION bin
)
