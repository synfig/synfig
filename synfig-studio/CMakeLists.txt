## TODO: find out which version is actually required
cmake_minimum_required(VERSION 3.1)
include(SynfigIntltool)

project(synfigstudio)

# using latest stable standard, but not strictly
# the rest is done via target_compile_features instead
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(src)

add_subdirectory(brushes)
add_subdirectory(images)
add_subdirectory(plugins)
add_subdirectory(po)
add_subdirectory(sounds)
add_subdirectory(ui)

file(MAKE_DIRECTORY "${SYNFIG_BUILD_ROOT}/share/appdata")
file(MAKE_DIRECTORY "${SYNFIG_BUILD_ROOT}/share/applications")

STUDIO_INTLTOOL_MERGE(
    TARGET_NAME appdata_xml
    INPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/org.synfig.SynfigStudio.appdata.xml.in
    OUTPUT_FILE "org.synfig.SynfigStudio.appdata.xml"
    INSTALL_DESTINATION "share/appdata"
)

STUDIO_INTLTOOL_MERGE(
    DESKTOP
    TARGET_NAME desktop_file
    INPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/org.synfig.SynfigStudio.desktop.in
    OUTPUT_FILE "org.synfig.SynfigStudio.desktop"
    INSTALL_DESTINATION "share/applications"
)

add_custom_command(
    TARGET desktop_file POST_BUILD
    COMMAND sed -i -E \"s|^Icon=\(.*\)|Icon=/usr/share/icons/hicolor/scalable/apps/org.synfig.SynfigStudio.svg|g\"
        ${SYNFIG_BUILD_ROOT}/share/applications/org.synfig.SynfigStudio.desktop
)

if(LINUX)
    add_dependencies(synfigstudio
        build_images
        build_pofiles_synfigstudio
        plugins_xml
        appdata_xml
        desktop_file
    )
elseif(WIN32)
    add_dependencies(synfigstudio
        build_images
        build_pofiles_synfigstudio
    )
endif()
