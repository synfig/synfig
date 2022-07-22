vcpkg_download_distfile(ARCHIVE
    URLS "https://download.gnome.org/sources/adwaita-icon-theme/42/adwaita-icon-theme-42.0.tar.xz"
    FILENAME "adwaita-icon-theme-42.0.tar.xz"
    SHA512 521BFD44FDC253F5FD01FCDFAA485669849AD73C35354CCFE9B358BA433CFA40EE6D68B70EC2268A4CE0631A48670B03B30811CA68E2E19F9430082454E02015
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
