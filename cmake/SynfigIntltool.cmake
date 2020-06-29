find_program(INTLTOOL_MERGE intltool-merge)

function(SYNFIG_INTLTOOL_MERGE)
    set(_targetName TARGET_NAME)
    set(_inputFile INPUT_FILE)
    set(_installDestination INSTALL_DESTINATION)

    if(INTLTOOL_MERGE)
        cmake_parse_arguments(
            _parsedArguments
            ""
            "${_targetName};${_inputFile};${_installDestination}"
            ""
            ${ARGN}
        )

        set(_OUTPUT_FILE ${SYNFIG_BUILD_ROOT}/${_parsedArguments_INSTALL_DESTINATION}/plugin.xml)

        add_custom_command(
            OUTPUT ${_OUTPUT_FILE}
            COMMAND ${INTLTOOL_MERGE} -x -u
            -c ${CMAKE_CURRENT_BINARY_DIR}/../../po/.intltool-merge-cache
            ${CMAKE_CURRENT_SOURCE_DIR}/../../po
            ${_parsedArguments_INPUT_FILE}
            ${_OUTPUT_FILE}
            DEPENDS ${_parsedArguments_INPUT_FILE}
        )

        add_custom_target(${_parsedArguments_TARGET_NAME}
            DEPENDS ${_OUTPUT_FILE}
        )

        install(FILES ${_OUTPUT_FILE}
            DESTINATION ${_parsedArguments_INSTALL_DESTINATION}
        )

    else()
        message(WARNING "-- Could not find intltool-merge: No translations made for ${_name}.")
    endif()
endfunction()