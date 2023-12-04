target_sources(synfigstudio
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/childrentree.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/keyframetree.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layergrouptree.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layertree.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/metadatatree.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/widget_stripedtreeview.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/canvastreestore.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/childrentreestore.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/historytreestore.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/keyframetreestore.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layergrouptreestore.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layerparamtreestore.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/layertreestore.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/metadatatreestore.cpp"
)
