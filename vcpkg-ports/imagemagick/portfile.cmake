vcpkg_download_distfile(ARCHIVE
    URLS "https://github.com/ImageMagick/ImageMagick/archive/refs/tags/7.1.0-39.tar.gz"
    FILENAME "ImageMagick-7.1.0-39.tar.gz"
    SHA512 A655DA17BE5E7B1A9CA95960DD29261ACD21222A2087BB187E9A06B29446F3755CF305B86185FED9D8B4BA30F46D1D679159B4670089ECCF93153A6A7FA63160
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE "${ARCHIVE}"
    PATCHES
      disable_complex_fftw.patch
      dont_include_win32config.patch
      fix_make_dependency.patch
      fix_ssize_t_undefined.patch
)

vcpkg_configure_make(
  SOURCE_PATH "${SOURCE_PATH}"
  DETERMINE_BUILD_TRIPLET
  USE_WRAPPERS
  OPTIONS
    --with-fftw
    --with-fontconfig
    --with-freetype
    --with-jpeg
    --with-lzma
    --with-pango
    --with-png
    --with-xml
    --with-zlib
    --with-zstd
    --with-openexr
    # FIXME (gdi32 is used in MagickCore which is a C project, while gdi32 is c++)
    # msvc automatically uses C mode when compiling files with extension "c"
    --without-gdi32
    --disable-docs
)

vcpkg_install_make()
vcpkg_fixup_pkgconfig()
vcpkg_copy_tools(
  TOOL_NAMES
    animate
    compare
    composite
    conjure
    convert
    display
    identify
    import
    magick
    magick-script
    mogrify
    montage
    stream
  AUTO_CLEAN
  SEARCH_DIR "${CURRENT_PACKAGES_DIR}/tools/imagemagick/bin"
)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/tools/imagemagick/bin")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/tools/imagemagick/debug")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
