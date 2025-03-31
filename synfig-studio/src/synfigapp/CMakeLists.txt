# Define the shared library
add_library(synfigapp SHARED "")

# Include necessary directories
target_include_directories(synfigapp SYSTEM PUBLIC ${LIBXML_INCLUDE_DIRS})

# List source files
target_sources(synfigapp
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/action.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/action_param.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/action_system.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/blineconvert.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/wplistconverter.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/canvasinterface.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/inputdevice.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/instance.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/settings.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/timegather.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/uimanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/value_desc.cpp"
)

# Find and link cURL
find_package(CURL REQUIRED)

# Link against necessary libraries
target_link_libraries(synfigapp
    PRIVATE
        libsynfig
        CURL::libcurl
)

# Include additional CMakeLists files
include(actions/CMakeLists.txt)
include(vectorizer/CMakeLists.txt)

# Define installation rules
if(WIN32)
    install(
        TARGETS synfigapp
        DESTINATION bin
    )
else()
    install(
        TARGETS synfigapp
        DESTINATION lib
    )
endif()
