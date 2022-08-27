vcpkg_check_linkage(ONLY_DYNAMIC_LIBRARY)

vcpkg_download_distfile(ARCHIVE
    URLS "https://ftp2.nluug.nl/windowing/gnome/sources/libsigc++/2.10/libsigc++-2.10.8.tar.xz"
    FILENAME "libsigc++-2.10.8.tar.xz"
    SHA512 8B22FD8AE4ECA3EBC1B65B68D4DC022E7BBDE6D1D02A557E64D2FDA2682E2E39B357AF6D8B68E8741C287701BE9FFFD27125C6171790455A7657E0EA55CC08B3
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE ${ARCHIVE}
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE ${ARCHIVE}
)

vcpkg_configure_meson(
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
        -Dbuild-examples=false
        -Dbuild-documentation=false
)

vcpkg_install_meson()
vcpkg_fixup_pkgconfig()
vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/COPYING" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
