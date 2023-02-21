vcpkg_check_linkage(ONLY_DYNAMIC_LIBRARY)

vcpkg_download_distfile(ARCHIVE
    URLS "https://download.gnome.org/sources/gtkmm/3.24/gtkmm-3.24.6.tar.xz"
    FILENAME "gtkmm-3.24.6.tar.xz"
    SHA512 D6FC2D0689BACA2B968820D1AF86CC0B175B6F38F720D315BC50663D30BD0A841DA57AC4336ABEC236542FB2893B28B2DE84087419B73ABA58B41F07568D4D93
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE ${ARCHIVE}
)

# gkmm uses glib-compile-resources which uses gdk-pixbuf-pixdata, so it has to
# be in the path and it's not enough to add it in ADDITIONAL_*_BINARIES in meson
vcpkg_add_to_path("${CURRENT_HOST_INSTALLED_DIR}/tools/gdk-pixbuf/")

vcpkg_configure_meson(
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
        -Dmsvc14x-parallel-installable=false
        -Dbuild-documentation=false
        -Dbuild-atkmm-api=false
        -Dbuild-x11-api=false
        -Dbuild-demos=false
        -Dbuild-tests=false
        -Dbuild-documentation=false
    ADDITIONAL_NATIVE_BINARIES
        glib-compile-resources='${CURRENT_HOST_INSTALLED_DIR}/tools/glib/glib-compile-resources${VCPKG_HOST_EXECUTABLE_SUFFIX}'
    ADDITIONAL_CROSS_BINARIES
        glib-compile-resources='${CURRENT_HOST_INSTALLED_DIR}/tools/glib/glib-compile-resources${VCPKG_HOST_EXECUTABLE_SUFFIX}'
)

vcpkg_install_meson()
vcpkg_fixup_pkgconfig()
vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/COPYING" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
