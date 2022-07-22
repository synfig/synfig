vcpkg_download_distfile(ARCHIVE
    URLS "https://download.gnome.org/sources/librsvg/2.40/librsvg-2.40.20.tar.xz"
    FILENAME "librsvg-2.40.20.tar.xz"
    SHA512 cdd8224deb4c3786e29f48ed02c32ed9dff5cb15aba574a5ef845801ad3669cfcc3eedb9d359c22213dc7a29de24c363248825adad5877c40abf73b3688ff12f
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE "${ARCHIVE}"
    PATCHES
      use_unix_path.patch
      remove_lm_from_pkgconfig.patch
)

vcpkg_find_acquire_program(PYTHON3)
get_filename_component(PYTHON3_DIR "${PYTHON3}" DIRECTORY)
# this is bad!
# glib-mkenums has #!/bin/env python3
file (COPY_FILE ${PYTHON3} "${PYTHON3_DIR}/python3.exe")

vcpkg_add_to_path("${PYTHON3_DIR}")
vcpkg_add_to_path("${CURRENT_HOST_INSTALLED_DIR}/tools/gdk-pixbuf")
vcpkg_add_to_path("${CURRENT_HOST_INSTALLED_DIR}/tools/glib")

vcpkg_configure_make(
  SOURCE_PATH "${SOURCE_PATH}"
  DETERMINE_BUILD_TRIPLET
  USE_WRAPPERS
  OPTIONS
    --disable-gtk-doc
    --enable-introspection=no
    --enable-pixbuf-loader
)

vcpkg_install_make()
vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
file(INSTALL "${SOURCE_PATH}/COPYING" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
