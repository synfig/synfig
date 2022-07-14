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
)

set (OPTIONS
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
  # FIXME (gdi32 is used in MagickCore which is a C project, while gdi32 is c++)
  # msvc automatically uses C mode when compiling files with extension "c"
  --without-gdi32
  --disable-docs
)

vcpkg_configure_make(
  SOURCE_PATH "${SOURCE_PATH}"
  DETERMINE_BUILD_TRIPLET
  USE_WRAPPERS
  OPTIONS
    ${OPTIONS}
)

set(buildtypes)
if(NOT DEFINED VCPKG_BUILD_TYPE OR VCPKG_BUILD_TYPE STREQUAL "debug")
    set(buildname "DEBUG")
    vcpkg_list(APPEND buildtypes ${buildname})
    set(path_suffix_${buildname} "debug/")
    set(suffix_${buildname} "dbg")
endif()
if(NOT DEFINED VCPKG_BUILD_TYPE OR VCPKG_BUILD_TYPE STREQUAL "release")
    set(buildname "RELEASE")
    vcpkg_list(APPEND buildtypes ${buildname})
    set(path_suffix_${buildname} "")
    set(suffix_${buildname} "rel")
endif()

# this patch is adapted from VisualMagick
set(SSIZE_T_PATCH
" \
/* \n \
  Visual C++ does not define double_t, float_t, or ssize_t by default. \n \
*/ \n \
#ifndef _MAGICKCORE_MAGICK_BASECONFIG_H_2
#define _MAGICKCORE_MAGICK_BASECONFIG_H_2
#ifdef _MSC_VER
  #if !defined(double_t) \n \
    #define MAGICKCORE_HAVE_DOUBLE_T \n \
    #if !defined(__MINGW32__) \n \
      typedef double double_t; \n \
    #endif \n \
  #endif \n \
  #if !defined(float_t) \n \
    #define MAGICKCORE_HAVE_FLOAT_T \n \
    #if !defined(__MINGW32__) \n \
      typedef float float_t; \n \
    #endif \n \
  #endif \n \
  #if !defined(ssize_t) && !defined(__MINGW32__) \n \
    #if defined(_WIN64) \n \
    typedef __int64 ssize_t; \n \
    #else \n \
    typedef long ssize_t; \n \
    #endif \n \
  #endif \n \
#endif \n \
#endif \n"
)

foreach(buildtype IN LISTS buildtypes)
  file(READ "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-${suffix_${buildtype}}/Makefile" FILE_CONTENTS)

  # the target "install-exec-local-utilities" depends on "install-binPROGRAMS"
  # it happens to work when using one job (make install -j1), as it executes
  # "install-binPROGRAMS" first, but it isn't guaranteed when using multiple jobs
  # TODO: fix this properly in the automake files
  string(REPLACE
    "install-exec-local-utilities:" "install-exec-local-utilities: install-binPROGRAMS"
    FILE_CONTENTS ${FILE_CONTENTS})

  # for some reason the Makefile generated uses incorrect sed command
  # specifically:
  # am__base_list = \
  #   sed '$$!N$$!N$$!N$$!N$$!N$$!N$$!Ns/\n/ /g' | \
  #   sed '$$!N$$!N$$!N$$!Ns/\n/ /g'
  #
  # this is incorrect format. it should be
  # am__base_list = \
  #   sed '$$!N;$$!N;$$!N;$$!N;$$!N;$$!N;$$!N;s/\n/ /g' | \
  #   sed '$$!N;$$!N;$$!N;$$!N;s/\n/ /g'
  #
  # it might work on different version of sed than the one provided by msys though
  #
  # I really hate autotools!
  string(REPLACE "$$!N" "$$!N;" FILE_CONTENTS ${FILE_CONTENTS})


  file(WRITE "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-${suffix_${buildtype}}/Makefile" "${FILE_CONTENTS}")

  file(APPEND
    "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-${suffix_${buildtype}}/MagickCore/magick-baseconfig.h"
    "${SSIZE_T_PATCH}"
  )
endforeach()


vcpkg_install_make()
vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
