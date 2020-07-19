find_program(INTLTOOL_MERGE intltool-merge)

function(STUDIO_INTLTOOL_MERGE)
    set(_desktop DESKTOP)
    set(_targetName TARGET_NAME)
    set(_inputFile INPUT_FILE)
    set(_outputFile OUTPUT_FILE)
    set(_installDestination INSTALL_DESTINATION)

    if(INTLTOOL_MERGE)
        cmake_parse_arguments(
            _parsedArguments
            "${_desktop}"
            "${_targetName};${_inputFile};${_installDestination};${_outputFile}"
            ""
            ${ARGN}
        )

        if(_parsedArguments_OUTPUT_FILE)
            set(_OUTPUT_FILE "${SYNFIG_BUILD_ROOT}/${_parsedArguments_INSTALL_DESTINATION}/${_parsedArguments_OUTPUT_FILE}")
        else()
            set(_OUTPUT_FILE "${SYNFIG_BUILD_ROOT}/${_parsedArguments_INSTALL_DESTINATION}/plugin.xml")
        endif()

        if(_parsedArguments_DESKTOP)
            set(INTLTOOL_MERGE_OPTION "-d")
        else()
            set(INTLTOOL_MERGE_OPTION "-x")
        endif()

        add_custom_command(
            OUTPUT ${_OUTPUT_FILE}
            COMMAND ${INTLTOOL_MERGE} ${INTLTOOL_MERGE_OPTION} -u
            -c ${CMAKE_BINARY_DIR}/synfig-studio/po/.intltool-merge-cache
            ${CMAKE_SOURCE_DIR}/synfig-studio/po
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
