vcpkg_check_linkage(ONLY_DYNAMIC_LIBRARY)

vcpkg_download_distfile(ARCHIVE
    URLS "https://download.gnome.org/sources/pangomm/2.46/pangomm-2.46.2.tar.xz"
    FILENAME "pangomm-2.46.2.tar.xz"
    SHA512 79CAD137EC9F22E57A177332DF6C6E735AA3BAF81DF36FC706E81E7A89F23CE3179337507388F65A0D0A74733990DC3547C50243A4559DFD0BB065B1828C0474
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE ${ARCHIVE}
)

vcpkg_configure_meson(
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
        -Dmsvc14x-parallel-installable=false
        -Dbuild-documentation=false
)

vcpkg_install_meson()
vcpkg_fixup_pkgconfig()
vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/COPYING" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
