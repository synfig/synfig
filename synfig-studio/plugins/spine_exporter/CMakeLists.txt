set(INSTALL_DESTINATION "share/synfig/plugins/spine_exporter")

set(PLUGIN_SPINE_EXPORTER_FILES
    spine_exporter.py
    settings.py
    plugin.xml.in
    CMakeLists.txt
    Makefile.am

    # core
    core/Canvas.py
    core/Param.py
    core/Layer.py

    # bones
    bones/Bone.py
    bones/BoneParser.py

    # exporters
    exporters/spine_json_exporter.py

    # plugin_files
    
)

foreach(CURRENT_FILE ${PLUGIN_SPINE_EXPORTER_FILES})
    get_filename_component(CURRENT_DIRECTORY ${CURRENT_FILE} DIRECTORY)
    file(COPY ${CURRENT_FILE}
        DESTINATION ${SYNFIG_BUILD_ROOT}/${INSTALL_DESTINATION}/${CURRENT_DIRECTORY}
    )

    install(FILES ${CURRENT_FILE}
        DESTINATION ${INSTALL_DESTINATION}/${CURRENT_DIRECTORY}
    )
endforeach()

STUDIO_INTLTOOL_MERGE(
    TARGET_NAME plugin_spine_exporter_xml
    INPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml.in
    INSTALL_DESTINATION ${INSTALL_DESTINATION}
)
