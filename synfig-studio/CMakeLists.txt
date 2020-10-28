## TODO: find out which version is actually required
cmake_minimum_required(VERSION 3.1)

project(synfigstudio)

# using latest stable standard, but not strictly
# the rest is done via target_compile_features instead
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(src)

add_subdirectory(brushes)
if(NOT MSVC)
add_subdirectory(images)
endif()
add_subdirectory(plugins)
add_subdirectory(po)
add_subdirectory(sounds)
add_subdirectory(ui)

file(MAKE_DIRECTORY "${SYNFIG_BUILD_ROOT}/share/appdata")
file(MAKE_DIRECTORY "${SYNFIG_BUILD_ROOT}/share/applications")

if(TARGET desktop_file)
    add_custom_command(
        TARGET desktop_file POST_BUILD
        COMMAND sed -i -E \"s|^Icon=\(.*\)|Icon=/usr/share/icons/hicolor/scalable/apps/org.synfig.SynfigStudio.svg|g\"
            ${SYNFIG_BUILD_ROOT}/share/applications/org.synfig.SynfigStudio.desktop
    )
endif()

if(NOT MSVC)
add_dependencies(synfigstudio
    build_images
    build_pofiles_synfigstudio
)

if(TARGET plugins_xml)
    add_dependencies(synfigstudio plugins_xml)
endif()

if(TARGET appdata_xml)
    add_dependencies(synfigstudio appdata_xml)
endif()

if(TARGET desktop_file)
    add_dependencies(synfigstudio desktop_file)
endif()
endif()
