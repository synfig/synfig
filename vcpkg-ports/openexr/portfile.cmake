if (VCPKG_CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
  message(FATAL_ERROR "UWP build not supported")
endif()

vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO openexr/openexr
  REF 420da6a57a3ecaeab481b993dd590eeec8fe0c52
  SHA512 5D8A5B0111394385AAACBDC6225C7CE75FA4E0126555B77A290EA51CABBE92D7C9AD41EBDDDC779E9018651262E4777D3A07166D862FEEFC57AC8BD3460DC76A
  HEAD_REF master
)

vcpkg_configure_cmake(SOURCE_PATH ${SOURCE_PATH}
  PREFER_NINJA
  OPTIONS
    -DCMAKE_DEBUG_POSTFIX=_d
    -DOPENEXR_INSTALL_PKG_CONFIG=TRUE
)

vcpkg_install_cmake()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/OpenEXR)
vcpkg_fixup_pkgconfig()

vcpkg_copy_tools(
  TOOL_NAMES
    exr2aces
    exrenvmap
    exrheader
    exrinfo
    exrmakepreview
    exrmaketiled
    exrmultipart
    exrmultiview
    exrstdattr
  AUTO_CLEAN
)
vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

if (VCPKG_CMAKE_SYSTEM_NAME OR VCPKG_LIBRARY_LINKAGE STREQUAL static)
  file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/bin ${CURRENT_PACKAGES_DIR}/debug/bin)
endif()

file(INSTALL ${SOURCE_PATH}/LICENSE.md DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
