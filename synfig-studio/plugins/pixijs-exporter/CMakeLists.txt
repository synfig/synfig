configure_file(plugin.xml.in plugin.xml @ONLY)

install(
    FILES
        pixijs-exporter.py
        settings.py
        ${CMAKE_CURRENT_BINARY_DIR}/plugin.xml
    DESTINATION share/synfig/plugins/pixijs-exporter
)
