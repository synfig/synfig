#[=============================================================[

Although CMake provides functions for processing .po files,
it cannot be used because GETTEXT_PROCESS_PO_FILES
installs files with an invalid name. For example: for
an en.gmo file, we should get the

    ${LOCALE_DIR}/${language}/LC_MESSAGES/${project_name}.mo

and not

    ${LOCALE_DIR}/${language}/LC_MESSAGES/en.mo

as it does, so we need to get the possibility to pass
${project_name} to that function and use it for file naming.

We also added an issue to CMake:
https://gitlab.kitware.com/cmake/cmake/-/issues/20792

#]=============================================================]

find_program(MSGFMT_EXECUTABLE msgfmt)

if(MSGFMT_EXECUTABLE)
    execute_process(COMMAND ${MSGFMT_EXECUTABLE} --version
        OUTPUT_VARIABLE GETTEXT_VERSION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    get_filename_component(MSGMERGE_NAME ${MSGFMT_EXECUTABLE} NAME)
    get_filename_component(MSGMERGE_NAMEWE ${MSGFMT_EXECUTABLE} NAME_WE)

    if (GETTEXT_VERSION MATCHES "^(${MSGMERGE_NAME}|${MSGMERGE_NAMEWE}) \\([^\\)]*\\) ([0-9\\.]+[^ \n]*)")
        set(GETTEXT_VERSION_STRING "${CMAKE_MATCH_2}")
    endif()

    unset(GETTEXT_VERSION)
    unset(MSGMERGE_NAME)
    unset(MSGMERGE_NAMEWE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Gettext
    REQUIRED_VARS GETTEXT_MSGMERGE_EXECUTABLE GETTEXT_MSGFMT_EXECUTABLE
    VERSION_VAR GETTEXT_VERSION_STRING
)

function(STUDIO_GETTEXT_MSGFMT)
    set(_desktop DESKTOP)
    set(_targetName TARGET_NAME)
    set(_inputFile INPUT_FILE)
    set(_outputFile OUTPUT_FILE)
    set(_installDestination INSTALL_DESTINATION)

    cmake_parse_arguments(
            _parsedArguments
            "${_desktop}"
            "${_targetName};${_inputFile};${_installDestination};${_outputFile}"
            ""
            ${ARGN}
    )
	
    if(GETTEXT_MSGFMT_EXECUTABLE)
		if(_parsedArguments_OUTPUT_FILE)
            set(_OUTPUT_FILE "${SYNFIG_BUILD_ROOT}/${_parsedArguments_INSTALL_DESTINATION}/${_parsedArguments_OUTPUT_FILE}")
        else()
            set(_OUTPUT_FILE "${SYNFIG_BUILD_ROOT}/${_parsedArguments_INSTALL_DESTINATION}/plugin.xml")
        endif()
		
		if(_parsedArguments_DESKTOP)
            set(GETTEXT_MSGFMT_OPTION "--desktop")
        else()
            set(GETTEXT_MSGFMT_OPTION "--xml")
        endif()
		
		add_custom_command(
            OUTPUT ${_OUTPUT_FILE}
            COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} ${GETTEXT_MSGFMT_OPTION} -d
            ${CMAKE_SOURCE_DIR}/synfig-studio/po --template
            ${_parsedArguments_INPUT_FILE} -o
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
        message(WARNING "-- Could not find gettext-msgfmt: No translations made for ${_parsedArguments_TARGET_NAME}.")
    endif()
endfunction()

function(SYNFIG_PROCESS_PO_FILES)
    set(_options ALL)
    set(_targetName TARGET_NAME)
    set(_moName MO_NAME)
    set(_installDestination INSTALL_DESTINATION)
    set(_language LANGUAGES)
    set(_moFiles)

    cmake_parse_arguments(
        _parsedArguments
        "${_options}"
        "${_targetName};${_moName};${_installDestination}"
        "${_language}"
        ${ARGN}
    )

    if(MSGFMT_EXECUTABLE)
        foreach(_current_LANGUAGE ${_parsedArguments_LANGUAGES})
            set(_poFile ${CMAKE_CURRENT_SOURCE_DIR}/${_current_LANGUAGE}.po)
            set(_moDirectory ${SYNFIG_BUILD_ROOT}/share/locale/${_current_LANGUAGE}/LC_MESSAGES)
            set(_moFile ${_moDirectory}/${_parsedArguments_MO_NAME}.mo)
            add_custom_command(OUTPUT ${_moFile}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${_moDirectory}
                COMMAND ${MSGFMT_EXECUTABLE} -o ${_moFile} ${_poFile}
                WORKING_DIRECTORY "."
                DEPENDS ${_poFile}
            )

            if(_parsedArguments_INSTALL_DESTINATION)
                install(
                    FILES ${_moFile} DESTINATION
                    ${_parsedArguments_INSTALL_DESTINATION}/${_current_LANGUAGE}/LC_MESSAGES/
                )
            endif()

            list(APPEND _moFiles ${_moFile})
        endforeach()


        if(NOT TARGET build_pofiles)
            add_custom_target(build_pofiles)
        endif()

        if(_parsedArguments_TARGET_NAME)
            if(_parsendArguments_ALL)
                add_custom_target(${_parsedArguments_TARGET_NAME} ALL DEPENDS ${_moFiles})
            else()
                add_custom_target(${_parsedArguments_TARGET_NAME} DEPENDS ${_moFiles})
            endif()

            add_dependencies(build_pofiles ${_parsedArguments_TARGET_NAME})
        endif()
    else()
        message(STATUS "-- Could not find msgfmt: Cannot process .po files for ${_parsedArguments_TARGET_NAME}.")
    endif()
endfunction()
