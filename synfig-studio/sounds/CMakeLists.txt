set(SOUNDS
    renderdone.wav
    readme.txt
)

foreach (SOUND IN ITEMS ${SOUNDS})
    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/${SOUND}
        DESTINATION share/synfig/sounds
    )
endforeach()
