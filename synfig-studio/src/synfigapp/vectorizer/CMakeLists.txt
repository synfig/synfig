target_sources(synfigapp
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/centerlinepolygonizer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/centerlineskeletonizer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/centerlinevectorizer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/outlinevectorizer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/centerlineadjustments.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/centerlinecolors.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/centerlinetostrokes.cpp"

)

target_include_directories(synfigapp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
