add_subdirectory(add-skeleton-simple)
add_subdirectory(lottie-exporter)
add_subdirectory(view-unhide-all-layers)

add_custom_target(plugins_xml)

if(TARGET plugin_add_skeleton_simple_xml)
    add_dependencies(plugins_xml plugin_add_skeleton_simple_xml)
endif()

if(TARGET plugin_lottie_exporter_xml)
    add_dependencies(plugins_xml plugin_lottie_exporter_xml)
endif()

if(TARGET plugin_view_unhide_all_layers_xml)
    add_dependencies(plugins_xml plugin_view_unhide_all_layers_xml)
endif()
