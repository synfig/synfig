target_sources(synfigstudio
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/module.cpp"
)

include(${CMAKE_CURRENT_LIST_DIR}/mod_palette/CMakeLists.txt)
