set(INSTALL_DESTINATION "share/synfig/ui")
set(UI_FILES
	canvas_options.glade
	preview_options.glade
	dialog_workspaces.glade
	shortcuts_window.glade
	studio_menubar.xml
	history_toolbar.glade
)

foreach(UI_FILE ${UI_FILES})
	configure_file(${UI_FILE} ${SYNFIG_BUILD_ROOT}/${INSTALL_DESTINATION}/${UI_FILE} COPYONLY)
endforeach()

install(FILES ${UI_FILES}
	DESTINATION ${INSTALL_DESTINATION}
)

