include(SynfigGettext)
file(STRINGS LINGUAS PO_LANGUAGES)

SYNFIG_PROCESS_PO_FILES(
    ALL
    TARGET_NAME build_pofiles_synfigstudio
    MO_NAME synfigstudio
    LANGUAGES ${PO_LANGUAGES}
    INSTALL_DESTINATION share/locale
)
