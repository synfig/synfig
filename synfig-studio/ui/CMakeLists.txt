set(UIFILES
    canvas_options.ui
    preview_options.ui
)

foreach (UIFILE IN ITEMS ${UIFILES})
    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/${UIFILE}
        DESTINATION share/synfig/ui
    )
endforeach()
