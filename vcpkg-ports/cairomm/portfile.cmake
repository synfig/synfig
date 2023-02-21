vcpkg_check_linkage(ONLY_DYNAMIC_LIBRARY)

vcpkg_download_distfile(ARCHIVE
    URLS "https://cairographics.org/releases/cairomm-1.14.3.tar.xz"
    FILENAME "cairomm-1.14.3.tar.xz"
    SHA512 8DC8A0DE733904742C54D4935B596D4103F2E498C6735894BC6A5A81EB4962C6EA944BAD94102B18B25850F78E948D38F117C566B197BC76DA23A4E88B62EE4E
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE ${ARCHIVE}
)

vcpkg_configure_meson(
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
        -Dbuild-examples=false
        -Dmsvc14x-parallel-installable=false
        -Dbuild-documentation=false
        -Dbuild-tests=false
)

vcpkg_install_meson()
vcpkg_fixup_pkgconfig()
vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/COPYING" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
