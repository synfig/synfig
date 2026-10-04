vcpkg_check_linkage(ONLY_DYNAMIC_LIBRARY)

vcpkg_download_distfile(ARCHIVE
    URLS "https://github.com/libxmlplusplus/libxmlplusplus/releases/download/2.42.3/libxml++-2.42.3.tar.xz"
    FILENAME "libxml++-2.42.3.tar.xz"
    SHA512 591D1A203A4C1BB2C27200322429EE68ED12E6DDFD9CB3168E50FB2382102EE113D70B66B485CCAC0B4AABEFA2FF567CCF813D35D3AA70251367BD07722BFB2A
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
