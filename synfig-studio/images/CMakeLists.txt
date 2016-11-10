file(GLOB IMAGES "*.sif")

## TODO: don't run this if files are up to date
add_custom_target(build_images)

foreach (SIF IN ITEMS ${IMAGES})
    string(REGEX REPLACE "\\.[^.]*$" "" IMAGE ${SIF})
    set(PNG "${IMAGE}.png")
    add_custom_command(
        TARGET build_images POST_BUILD
#         OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PNG}
        COMMAND synfig ${SIF} --time 0f
        COMMAND mv ${PNG} ${CMAKE_CURRENT_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SIF}
    )
    get_filename_component(PNG_NAME ${PNG} NAME)
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${PNG_NAME}
        DESTINATION share/pixmaps
    )
endforeach()
