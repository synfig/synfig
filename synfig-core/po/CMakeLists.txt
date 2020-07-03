include(SynfigGettext)
file(STRINGS LINGUAS PO_LANGUAGES)

SYNFIG_PROCESS_PO_FILES(
    ALL
    TARGET_NAME build_pofiles_synfig
    MO_NAME synfig
    LANGUAGES ${PO_LANGUAGES}
    INSTALL_DESTINATION share/locale
)
