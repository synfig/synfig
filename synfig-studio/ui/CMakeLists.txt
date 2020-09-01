set(INSTALL_DESTINATION "share/synfig/ui")
set(UI_FILES
    canvas_options.glade
    preview_options.glade
    dialog_workspaces.glade
)

file(COPY ${UI_FILES}
    DESTINATION ${SYNFIG_BUILD_ROOT}/${INSTALL_DESTINATION}
)

SYNFIG_INSTALL(FILES ${UI_FILES}
    DESTINATION ${INSTALL_DESTINATION}
)
