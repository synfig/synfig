target_sources(synfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/rendererdraftsw.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/rendererlowressw.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderersafe.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/rendererpreviewsw.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderersw.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/surfacesw.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/surfaceswpacked.cpp"
)

include(${CMAKE_CURRENT_LIST_DIR}/function/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/task/CMakeLists.txt)

install_all_headers(rendering/software)
