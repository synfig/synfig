set(INSTALL_DESTINATION "share/synfig/ui")
set(UI_FILES
	canvas_options.glade
	dialog_canvasdependencies.glade
	dialog_workspaces.glade
	preview_options.glade
	shortcuts_window.glade
	history_toolbar.glade
	right_toolbar.glade
	file_top_toolbar.glade
	top_toolbar.glade
	studio_menubar.xml
)

foreach(UI_FILE ${UI_FILES})
	configure_file(${UI_FILE} ${SYNFIG_BUILD_ROOT}/${INSTALL_DESTINATION}/${UI_FILE} COPYONLY)
endforeach()

install(FILES ${UI_FILES}
	DESTINATION ${INSTALL_DESTINATION}
)

