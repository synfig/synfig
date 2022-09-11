vcpkg_download_distfile(ARCHIVE
    URLS "https://github.com/ImageMagick/ImageMagick/archive/refs/tags/7.1.0-39.tar.gz"
    FILENAME "ImageMagick-7.1.0-39.tar.gz"
    SHA512 A655DA17BE5E7B1A9CA95960DD29261ACD21222A2087BB187E9A06B29446F3755CF305B86185FED9D8B4BA30F46D1D679159B4670089ECCF93153A6A7FA63160
)

vcpkg_extract_source_archive(
    SOURCE_PATH
    ARCHIVE "${ARCHIVE}"
    PATCHES
      disable_complex_fftw.patch
      dont_include_win32config.patch
      fix_make_dependency.patch
      fix_ssize_t_undefined.patch
      fix_win_macros.patch
      use_cxx_to_link.patch
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
    --with-bzlib
    --with-lcms
    --with-rsvg
    # FIXME (gdi+ is used in MagickCore which is a C project, while gdi+ is c++)
    # msvc automatically uses C mode when compiling files with extension "c"
    --without-gdi32
    --without-autotrace
    --without-djvu
    --without-dps
    --without-flif
    --without-fpx
    --without-gslib
    --without-gvc
    --without-heic
    --without-jbig
    --without-jxl
    --without-lqr
    --without-ltdl
    --without-openjp2
    --without-perl
    --without-raqm
    --without-tiff
    --without-webp
    --without-wmf
    --without-raw # libraw exists but it has a bug with its pkgconfig file
    --disable-docs
)

# FIXME
if(CMAKE_HOST_UNIX)
    set(BACKUP_LD_LIBRARY_PATH $ENV{LD_LIBRARY_PATH})
    set(ENV{LD_LIBRARY_PATH} "${CURRENT_INSTALLED_DIR}/lib:${BACKUP_LD_LIBRARY_PATH}")
endif()

vcpkg_install_make()

if(CMAKE_HOST_UNIX)
    set(ENV{LD_LIBRARY_PATH} "${BACKUP_LD_LIBRARY_PATH}")
endif()

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
