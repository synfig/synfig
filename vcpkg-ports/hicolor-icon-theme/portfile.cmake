vcpkg_download_distfile(ARCHIVE
    URLS "http://icon-theme.freedesktop.org/releases/hicolor-icon-theme-0.17.tar.xz"
    FILENAME "hicolor-icon-theme-0.17.tar.xz"
    SHA512 ECA8655930AA7E234F42630041C0053FDE067B970FAD1F81C55FCD4C5046C03EDFDF2EDE72A3E78FBA2908E7DA53E9463D3C5AE12AB9F5EF261E29A49F9C7A8D
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE "${ARCHIVE}"
)

vcpkg_configure_make(
  SOURCE_PATH "${SOURCE_PATH}"
  DETERMINE_BUILD_TRIPLET
  USE_WRAPPERS
)

vcpkg_install_make()

file(INSTALL "${SOURCE_PATH}/COPYING" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
