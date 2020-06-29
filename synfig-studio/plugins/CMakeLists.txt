add_subdirectory(add-skeleton-simple)
add_subdirectory(lottie-exporter)
add_subdirectory(view-unhide-all-layers)

add_custom_target(plugins_xml)
add_dependencies(plugins_xml
    plugin_add_skeleton_simple_xml
    plugin_lottie_exporter_xml
    plugin_view_unhide_all_layers_xml
)
