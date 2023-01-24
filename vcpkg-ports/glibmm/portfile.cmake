vcpkg_check_linkage(ONLY_DYNAMIC_LIBRARY)

vcpkg_download_distfile(ARCHIVE
    URLS "https://download.gnome.org/sources/glibmm/2.66/glibmm-2.66.4.tar.xz"
    FILENAME "glibmm-2.66.4.tar.xz"
    SHA512 9B1E0C09A11182384313EA4A7BA484EBAB894528E08169A610387F207B5A7F8DB9338466CD3E9EB3FA55E1C12817351EA27C39D6503208AF67BA619F9D249C75
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE ${ARCHIVE}
)

vcpkg_configure_meson(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS 
        -Dbuild-examples=false
        -Dbuild-documentation=false
        -Dmsvc14x-parallel-installable=false
)
vcpkg_install_meson()
vcpkg_copy_pdbs()

vcpkg_fixup_pkgconfig()

# Handle copyright and readme
file(INSTALL "${SOURCE_PATH}/COPYING" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
file(INSTALL "${SOURCE_PATH}/README" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME readme.txt)
