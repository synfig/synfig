if(WIN32)
    set(MINGW_PATH $ENV{MINGW_PREFIX})

    set(MINGW_BIN "${MINGW_PATH}/bin")
    set(MINGW_LIB "${MINGW_PATH}/lib")

    # MSYS2 Hacks

    # This code fixes MSYS2 related problems and can be safely removed after these problems are resolved

    # ImageMagick has problems finding modules in MSYS2. For example, if you copy `magick.exe` somewhere
    # outside the `/mingw64/bin` folder and try to do some manipulation with images, for example:
    # `./magick.exe test.jpg test.png`, then this command will fail with error
    # `magick.exe: NoDecodeDelegateForThisImageFormat `JPEG '@ error / Create.c / ReadImage / 562.`
    # `synfig-cli` also fails when trying to make icons and images, so to fix this issue we need to copy ImageMagick's
    # libs inside our build directory. Read more about ImageMagick paths and environment variables:
    # https://imagemagick.org/script/resources.php#environment

    file(GLOB MAGICK_LIBS ${MINGW_LIB}/ImageMagick-*)
    file(COPY ${MAGICK_LIBS} DESTINATION ${SYNFIG_BUILD_ROOT}/lib)

    # End of hacks


    # /output/bin
    file(GLOB MINGW_LIBS
        ${MINGW_BIN}/libatk-1.0-[0-9]*.dll
        ${MINGW_BIN}/libatkmm-1.6-[0-9]*.dll
        ${MINGW_BIN}/libbz2-[0-9]*.dll
        ${MINGW_BIN}/libbrotlicommon.dll
        ${MINGW_BIN}/libbrotlidec.dll
        ${MINGW_BIN}/libcairo-[0-9]*.dll
        ${MINGW_BIN}/libcairo-gobject-[0-9]*.dll
        ${MINGW_BIN}/libcairo-script-interpreter-[0-9]*.dll
        ${MINGW_BIN}/libcairomm-1.0-[0-9]*.dll
        ${MINGW_BIN}/libdatrie-[0-9]*.dll
        ${MINGW_BIN}/libdl.dll
        ${MINGW_BIN}/libepoxy-[0-9]*.dll
        ${MINGW_BIN}/libexpat-[0-9]*.dll
        ${MINGW_BIN}/libffi-[0-9]*.dll
        ${MINGW_BIN}/libfftw3-[0-9]*.dll
        ${MINGW_BIN}/libfftw3f-[0-9]*.dll
        ${MINGW_BIN}/libfribidi-[0-9]*.dll
        ${MINGW_BIN}/libfontconfig-[0-9]*.dll
        ${MINGW_BIN}/libfreetype-[0-9]*.dll
        ${MINGW_BIN}/libgailutil-3-[0-9]*.dll
        ${MINGW_BIN}/libgcc_s_seh-[0-9]*.dll
        ${MINGW_BIN}/libgdk_pixbuf-2.0-[0-9]*.dll
        ${MINGW_BIN}/libgdk-3-[0-9]*.dll
        ${MINGW_BIN}/libgdkmm-3.0-[0-9]*.dll
        ${MINGW_BIN}/libgio-2.0-[0-9]*.dll
        ${MINGW_BIN}/libgiomm-2.4-[0-9]*.dll
        ${MINGW_BIN}/libglib-2.0-[0-9]*.dll
        ${MINGW_BIN}/libglibmm_generate_extra_defs-2.4-[0-9]*.dll
        ${MINGW_BIN}/libglibmm-2.4-[0-9]*.dll
        ${MINGW_BIN}/libgmodule-2.0-[0-9]*.dll
        ${MINGW_BIN}/libgobject-2.0-[0-9]*.dll
        ${MINGW_BIN}/libgraphite[0-9]*.dll
        ${MINGW_BIN}/libgthread-2.0-[0-9]*.dll
        ${MINGW_BIN}/libgtk-3-[0-9]*.dll
        ${MINGW_BIN}/libgtkmm-3.0-[0-9]*.dll
        ${MINGW_BIN}/libharfbuzz-[0-9]*.dll
        ${MINGW_BIN}/libiconv-[0-9]*.dll
        ${MINGW_BIN}/libintl-[0-9]*.dll
        ${MINGW_BIN}/liblzma-[0-9]*.dll
        ${MINGW_BIN}/libltdl-[0-9]*.dll
        ${MINGW_BIN}/libMagick++-[A-Z0-9.-]*.dll
        ${MINGW_BIN}/libMagickCore-[A-Z0-9.-]*.dll
        ${MINGW_BIN}/libMagickWand-[A-Z0-9.-]*.dll
        ${MINGW_BIN}/libpango-1.0-[0-9]*.dll
        ${MINGW_BIN}/libpangocairo-1.0-[0-9]*.dll
        ${MINGW_BIN}/libpangoft2-1.0-[0-9]*.dll
        ${MINGW_BIN}/libpangomm-1.4-[0-9]*.dll
        ${MINGW_BIN}/libpangowin32-1.0-[0-9]*.dll
        ${MINGW_BIN}/libpcre-[0-9]*.dll
        ${MINGW_BIN}/libpixman-1-[0-9]*.dll
        ${MINGW_BIN}/libpng16-1[0-9]*.dll
        ${MINGW_BIN}/libpng16-config
        ${MINGW_BIN}/libquadmath-[0-9]*.dll
        ${MINGW_BIN}/librsvg-2-[0-9]*.dll
        ${MINGW_BIN}/libsigc-2.0-[0-9]*.dll
        ${MINGW_BIN}/libssp-[0-9]*.dll
        ${MINGW_BIN}/libstdc++-[0-9]*.dll
        ${MINGW_BIN}/libthai-[0-9]*.dll
        ${MINGW_BIN}/libtiff-[0-9]*.dll
        ${MINGW_BIN}/libtiffxx-[0-9]*.dll
        ${MINGW_BIN}/libwinpthread-[0-9]*.dll
        ${MINGW_BIN}/libxml++-2.6-[0-9]*.dll
        ${MINGW_BIN}/libxml2-[0-9]*.dll
        ${MINGW_BIN}/zlib[0-9]*.dll
    )
#    file(COPY ${MINGW_LIBS} DESTINATION ${SYNFIG_BUILD_ROOT}/bin)
    install(FILES ${MINGW_LIBS} DESTINATION bin)

    find_program(CYGPATH_EXECUTABLE cygpath)
    if(CYGPATH_EXECUTABLE)
        set(MLT_PATH "/opt/mlt-6.16.0")
        execute_process(
            COMMAND ${CYGPATH_EXECUTABLE} -m ${MLT_PATH}
            OUTPUT_VARIABLE MLT_DIRECTORY
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        message(${MLT_DIRECTORY})
    else()
        message(WARNING "-- cygpath tool not found, using relative path for MLT.")
        set(MLT_DIRECTORY "${MINGW_PATH}/../opt/mlt-6.16.0")
    endif()

    file(GLOB MLT_FILES
        ${MLT_DIRECTORY}/libmlt++-3.dll
        ${MLT_DIRECTORY}/libmlt-6.dll
        ${MLT_DIRECTORY}/melt.exe
    )
    file(COPY ${MLT_FILES} DESTINATION ${SYNFIG_BUILD_ROOT}/bin)
    install(FILES ${MLT_FILES} DESTINATION bin)

   file(GLOB MLT_DIRECTORIES
        ${MLT_DIRECTORY}/lib
        ${MLT_DIRECTORY}/share
    )
    file(COPY ${MLT_DIRECTORIES} DESTINATION ${SYNFIG_BUILD_ROOT}/bin)
    install(DIRECTORY ${MLT_DIRECTORIES} DESTINATION bin)

    # /output/etc
    file(GLOB ETC_DIRECTORIES
        ${MINGW_PATH}/etc/ImageMagick-[0-9]*
        ${MINGW_PATH}/etc/gtk-3.[0-9]*
    )
#    file(COPY ${ETC_DIRECTORIES} DESTINATION ${SYNFIG_BUILD_ROOT}/etc)
    INSTALL(DIRECTORY ${ETC_DIRECTORIES} DESTINATION etc)

    # /output/lib
    file(GLOB LIB_DIRECTORIES
        ${MINGW_LIB}/atkmm-1.[0-9]
        ${MINGW_LIB}/cmake
        ${MINGW_LIB}/gdk-pixbuf-2.[0-9]
        ${MINGW_LIB}/giomm-2.[0-9]
        ${MINGW_LIB}/glibmm-2.[0-9]
        ${MINGW_LIB}/gtkmm-3.[0-9]
        ${MINGW_LIB}/pangomm-1.[0-9]
        ${MINGW_LIB}/sigc++-2.[0-9]
        ${MINGW_LIB}/cairomm-1.[0-9]
        ${MINGW_LIB}/gdkmm-3.[0-9]
        ${MINGW_LIB}/glib-2.[0-9]
        ${MINGW_LIB}/gtk-3.[0-9]
        ${MINGW_LIB}/ImageMagick-*
        ${MINGW_LIB}/libxml++-2.[0-9]
        ${MINGW_LIB}/pkgconfig
    )
#    file(COPY ${LIB_DIRECTORIES} DESTINATION ${SYNFIG_BUILD_ROOT}/lib)
    install(DIRECTORY ${LIB_DIRECTORIES} DESTINATION lib)

#    file(COPY ${MINGW_PATH}/share/icons/Adwaita DESTINATION ${SYNFIG_BUILD_ROOT}/share/icons)
    install(DIRECTORY ${MINGW_PATH}/share/icons/Adwaita DESTINATION share/icons)
endif()
