set(INSTALL_DESTINATION "share/synfig/plugins/add-skeleton-simple")
set(PLUGIN_ADD_SKELETON_SIMPLE_FILES
    add-skeleton-simple.py
    stickman.sif
)

file(COPY ${PLUGIN_ADD_SKELETON_SIMPLE_FILES}
    DESTINATION ${SYNFIG_BUILD_ROOT}/${INSTALL_DESTINATION}
)

install(FILES ${PLUGIN_ADD_SKELETON_SIMPLE_FILES}
    DESTINATION ${INSTALL_DESTINATION}
)

find_program(INTLTOOL_MERGE intltool-merge)
if(NOT INTLTOOL_MERGE)
    message(FATAL_ERROR "intltool-merge not found. Please install it (usually part of an 'intltool' package)")
endif(NOT INTLTOOL_MERGE)

add_custom_command(
    OUTPUT ${SYNFIG_BUILD_ROOT}/${INSTALL_DESTINATION}/plugin.xml
    COMMAND ${INTLTOOL_MERGE} -x -u
        -c ${CMAKE_CURRENT_BINARY_DIR}/../../po/.intltool-merge-cache
        ${CMAKE_CURRENT_SOURCE_DIR}/../../po
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml.in
        ${SYNFIG_BUILD_ROOT}/${INSTALL_DESTINATION}/plugin.xml
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml.in
)

add_custom_target(plugin_add_skeleton_simple_xml
    DEPENDS ${SYNFIG_BUILD_ROOT}/${INSTALL_DESTINATION}/plugin.xml
)

install(FILES ${SYNFIG_BUILD_ROOT}/${INSTALL_DESTINATION}/plugin.xml
    DESTINATION ${INSTALL_DESTINATION}
)
