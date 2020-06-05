file(STRINGS LINGUAS LANGUAGES)

set(PO_DIR ${SYNFIG_BUILD_ROOT}/share/locale)

foreach(LANGUAGE IN ITEMS ${LANGUAGES})
    list(APPEND POFILES "${CMAKE_CURRENT_SOURCE_DIR}/${LANGUAGE}.po")
endforeach()

find_package(Gettext)
if(GETTEXT_FOUND)
    gettext_process_po_files(
            ALL
            PO_FILES ${POFILES}
    )

    foreach(LANGUAGE ${LANGUAGES})
        add_custom_command(
                TARGET pofiles_2 POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${LANGUAGE}.gmo
                ${PO_DIR}/${LANGUAGE}/LC_MESSAGES/synfigstudio.mo
        )

        install(
                FILES ${PO_DIR}/${LANGUAGE}/LC_MESSAGES/synfigstudio.mo
                DESTINATION share/locale/${LANGUAGE}/LC_MESSAGES/
        )
    endforeach()
endif()
