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

function(SYNFIG_PROCESS_PO_FILES)
    set(_options ALL)
    set(_targetName TARGET_NAME)
    set(_moName MO_NAME)
    set(_installDestination INSTALL_DESTINATION)
    set(_language LANGUAGES)
    set(_moFiles)

    if(MSGFMT_EXECUTABLE)
        CMAKE_PARSE_ARGUMENTS(
                _parsedArguments
                "${_options}"
                "${_targetName};${_moName};${_installDestination}"
                "${_language}"
                ${ARGN}
        )

        foreach(_current_LANGUAGE ${_parsedArguments_LANGUAGES})
            set(_poFile ${CMAKE_CURRENT_SOURCE_DIR}/${_current_LANGUAGE}.po)
            set(_moDirectory ${SYNFIG_BUILD_ROOT}/share/locale/${_current_LANGUAGE}/LC_MESSAGES)
            set(_moFile ${_moDirectory}/${_parsedArguments_MO_NAME}.mo)
            add_custom_command(OUTPUT ${_moFile}
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${_moDirectory}
                    COMMAND ${MSGFMT_EXECUTABLE} -o ${_moFile} ${_poFile}
                    WORKING_DIRECTORY ".."
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
        message(FATAL_ERROR "-- Could not find msgfmt: Cannot process .po files.")
    endif()
endfunction()
