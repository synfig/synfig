set(INSTALL_DESTINATION "share/synfig/brushes/CD_concept")
set(BRUSH_CD_CONCEPT_FILES
    04_pen.myb
    04_pen_prev.png
    08_pen.myb
    08_pen_prev.png
    Big_BG.myb
    Big_BG_prev.png
    Eraser.myb
    Eraser_prev.png
    readme.txt
)

file(COPY ${BRUSH_CD_CONCEPT_FILES}
    DESTINATION ${SYNFIG_BUILD_ROOT}/${INSTALL_DESTINATION}
)

SYNFIG_INSTALL(FILES ${BRUSH_CD_CONCEPT_FILES}
    DESTINATION ${INSTALL_DESTINATION}
)
