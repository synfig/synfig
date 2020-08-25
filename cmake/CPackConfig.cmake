set(CPACK_PACKAGE_NAME "Synfig")
set(CPACK_PACKAGE_VENDOR "Synfig")
set(CPACK_PACKAGE_VERSION_MAJOR ${STUDIO_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${STUDIO_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${STUDIO_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/packaging/PACKAGING.md")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Create and edit 2D animations and compositions")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://synfig.org")
#set(CPACK_PACKAGE_ICON "${SYNFIG_BUILD_ROOT}/share/synfig/icons/classic/logo.png")
set(CPACK_PACKAGE_CONTACT "https://synfig.org")
set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_SOURCE_DIR}/cmake/CPackProjectConfig.cmake")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

set(CPACK_PACKAGE_EXECUTABLES "synfigstudio;Synfig Studio")
set(CPACK_CREATE_DESKTOP_LINKS "synfigstudio")
if(WIN32)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Synfig")
else()
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "synfig")
endif()


## Generator-specific configuration ##

# DEB (Linux .deb bundle)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_SECTION "animation")
if(LINUX)
    set(CPACK_DEB_COMPONENT_INSTALL ON)
    set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 1)
endif()

# NSIS (Windows .exe installer)
set(CPACK_NSIS_MUI_ICON "${SYNFIG_BUILD_ROOT}/share/synfig/icons/classic/synfig_icon.ico")
set(CPACK_NSIS_MUI_HEADERIMAGE "${CMAKE_SOURCE_DIR}/packaging/nsis/header.bmp")
set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${CMAKE_SOURCE_DIR}/packaging/nsis/welcomefinish.bmp")
set(CPACK_NSIS_INSTALLED_ICON_NAME "bin/synfigstudio.exe")
set(CPACK_NSIS_HELP_LINK "${CPACK_PACKAGE_HOMEPAGE_URL}")
set(CPACK_NSIS_URL_INFO_ABOUT "${CPACK_PACKAGE_HOMEPAGE_URL}")
set(CPACK_NSIS_MENU_LINKS "${CPACK_PACKAGE_HOMEPAGE_URL}" "Synfig Homepage")
set(CPACK_NSIS_COMPRESSOR "/SOLID lzma") # zlib|bzip2|lzma
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_MODIFY_PATH ON) # while the name does not suggest it, this also provides the possibility to add desktop icons
set(CPACK_NSIS_MUI_FINISHPAGE_RUN "synfigstudio") # TODO: this results in instance with administrative privileges!

set(CPACK_NSIS_COMPRESSOR "${CPACK_NSIS_COMPRESSOR}\n  SetCompressorDictSize 64") # hack (improve compression)
set(CPACK_NSIS_COMPRESSOR "${CPACK_NSIS_COMPRESSOR}\n  BrandingText '${CPACK_PACKAGE_DESCRIPTION_SUMMARY}'") # hack (overwrite BrandingText)

include(CPack)
