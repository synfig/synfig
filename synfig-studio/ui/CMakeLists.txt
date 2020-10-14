set(INSTALL_DESTINATION "share/synfig/ui")
set(UI_FILES
	canvas_options.glade
	preview_options.glade
	dialog_workspaces.glade
	shortcuts_window.glade
)

file(COPY ${UI_FILES}
	DESTINATION ${SYNFIG_BUILD_ROOT}/${INSTALL_DESTINATION}
)

install(FILES ${UI_FILES}
	DESTINATION ${INSTALL_DESTINATION}
)
