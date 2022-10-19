vcpkg_download_distfile(ARCHIVE
    URLS "https://download.gnome.org/sources/libcroco/0.6/libcroco-0.6.13.tar.xz"
    FILENAME "libcroco-0.6.13.tar.xz"
    SHA512 038a3ac9d160a8cf86a8a88c34367e154ef26ede289c93349332b7bc449a5199b51ea3611cebf3a2416ae23b9e45ecf8f9c6b24ea6d16a5519b796d3c7e272d4
)

vcpkg_extract_source_archive(
    SOURCE_PATH
    ARCHIVE ${ARCHIVE}
)

vcpkg_configure_make(
  SOURCE_PATH "${SOURCE_PATH}"
  DETERMINE_BUILD_TRIPLET
  USE_WRAPPERS
)

vcpkg_install_make()
vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(INSTALL "${SOURCE_PATH}/COPYING" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)


