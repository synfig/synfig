vcpkg_download_distfile(ARCHIVE
    URLS "https://github.com/ImageMagick/ImageMagick/archive/refs/tags/7.1.1-29.tar.gz"
    FILENAME "ImageMagick-7.1.1-29.tar.gz"
    SHA512 44d516e13b434677fcb391dbfae3386faae8b73f18ef8dac1f75ac1e56831761ec2e0fdbee5c5e7c2d1af384a28d09b1fdd0970676052a25ec18f6814efbc6f4
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE "${ARCHIVE}"
    PATCHES
      disable_complex_fftw.patch
#      dont_include_win32config.patch
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
