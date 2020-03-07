set(UIFILES
    canvas_options.glade
    preview_options.glade
    dialog_workspaces.glade
)

foreach (UIFILE IN ITEMS ${UIFILES})
    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/${UIFILE}
        DESTINATION share/synfig/ui
    )
endforeach()
