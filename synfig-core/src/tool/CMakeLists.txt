## Command line synfig binary
add_executable(synfig_bin main.cpp)
set_target_properties(synfig_bin PROPERTIES OUTPUT_NAME synfig)

# TODO: more full list of features
target_compile_features(synfig_bin PUBLIC
    cxx_auto_type
    cxx_lambdas
    cxx_override
)

target_sources(synfig_bin
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/definitions.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/joblistprocessor.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/optionsprocessor.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/printing_functions.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/renderprogress.cpp"
)

target_link_libraries(synfig_bin synfig)
target_link_libraries(synfig_bin
    ${Boost_SYSTEM_LIBRARIES}
    ${GIOMM_LIBRARIES}
)

SYNFIG_INSTALL(
    TARGETS synfig_bin
    DESTINATION bin
)
