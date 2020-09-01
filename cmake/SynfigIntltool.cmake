find_program(INTLTOOL_MERGE_EXECUTABLE intltool-merge)
mark_as_advanced(INTLTOOL_MERGE_EXECUTABLE)

if(INTLTOOL_MERGE_EXECUTABLE)
    execute_process(
        COMMAND ${INTLTOOL_MERGE_EXECUTABLE} --version
        OUTPUT_VARIABLE INTLTOOL_VERSION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(INTLTOOL_VERSION MATCHES "^intltool-extract \\(.*\\) [0-9]")
        string(REGEX REPLACE "^intltool-extract \\([^\\)]*\\) ([0-9\\.]+[^ \n]*).*" "\\1" INTLTOOL_VERSION_STRING "${INTLTOOL_VERSION}")
    endif()
    unset(INTLTOOL_VERSION)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Intltool
    REQUIRED_VARS INTLTOOL_MERGE_EXECUTABLE
    VERSION_VAR INTLTOOL_VERSION_STRING
)   

set(INTLTOOL_OPTIONS_DEFAULT "--quiet")

# When MinGW is used without MSYS environment (for example, Ninja
# generator uses the standard Windows `cmd` shell for building), `intltool-merge`
# cannot be started, because it is just a Perl script, and `cmd` does not know how
# to run it. In this case, we need to explicitly add the interpreter to the command.
if(INTLTOOL_MERGE_EXECUTABLE AND MINGW AND NOT MSYS)
    message(STATUS "Fixing intltool-merge command...")
    set(INTLTOOL_MERGE_EXECUTABLE perl ${INTLTOOL_MERGE_EXECUTABLE})
endif()

function(STUDIO_INTLTOOL_MERGE)
    set(_desktop DESKTOP)
    set(_targetName TARGET_NAME)
    set(_inputFile INPUT_FILE)
    set(_outputFile OUTPUT_FILE)
    set(_installDestination INSTALL_DESTINATION)

    if(INTLTOOL_MERGE_EXECUTABLE)
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
            COMMAND ${INTLTOOL_MERGE_EXECUTABLE} ${INTLTOOL_MERGE_OPTION} -u
            -c ${CMAKE_BINARY_DIR}/synfig-studio/po/.intltool-merge-cache
            ${CMAKE_SOURCE_DIR}/synfig-studio/po
            ${_parsedArguments_INPUT_FILE}
            ${_OUTPUT_FILE}
            DEPENDS ${_parsedArguments_INPUT_FILE}
        )

        add_custom_target(${_parsedArguments_TARGET_NAME}
            DEPENDS ${_OUTPUT_FILE}
        )

        SYNFIG_INSTALL(FILES ${_OUTPUT_FILE}
            DESTINATION ${_parsedArguments_INSTALL_DESTINATION}
        )
    else()
        message(WARNING "-- Could not find intltool-merge: No translations made for ${_name}.")
    endif()
endfunction()
