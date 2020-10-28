set(INSTALL_DESTINATION "share/synfig/css")
set(CSS_FILES
	synfig.css
	synfig.mac.css
)

foreach(CSS_FILE ${CSS_FILES})
	configure_file(${CSS_FILE} ${SYNFIG_BUILD_ROOT}/${INSTALL_DESTINATION}/${CSS_FILE} COPYONLY)
endforeach()

install(FILES ${CSS_FILES}
	DESTINATION ${INSTALL_DESTINATION}
)
