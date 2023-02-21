vcpkg_check_linkage(ONLY_DYNAMIC_LIBRARY)

vcpkg_download_distfile(ARCHIVE
    URLS "https://github.com/libxmlplusplus/libxmlplusplus/releases/download/2.42.1/libxml++-2.42.1.tar.xz"
    FILENAME "libxml++-2.42.1.tar.xz"
    SHA512 6C1D3A3150DDA3C933F27CA5A45A764CE6872487B91803AD366774F1B370C2C6E4CF2FC5C55666043DB803AE43A508603C8017AEFEAC458337999B629389E7AB
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
        -Dbuild-tests=false
        -Dbuild-examples=false
)

vcpkg_install_meson()
vcpkg_fixup_pkgconfig()
vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/COPYING" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
