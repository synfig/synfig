if(APPLE)
    if(${MACOSX_PACKAGE})
        # Copy App Template
        file(
            COPY ${CMAKE_SOURCE_DIR}/autobuild/osx/app-template/Contents 
            DESTINATION ${CMAKE_BINARY_DIR}/output/SynfigStudio.app
        )
        install(
            DIRECTORY ${CMAKE_SOURCE_DIR}/autobuild/osx/app-template/Contents
            DESTINATION SynfigStudio.app
        )

        add_executable(SynfigStudio ${CMAKE_SOURCE_DIR}/autobuild/osx/synfig_osx_launcher.cpp)
        set_target_properties(SynfigStudio
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/SynfigStudio.app/Contents/MacOS
        )
        install(
            TARGETS SynfigStudio
            DESTINATION SynfigStudio.app/Contents/MacOS
        )

        set(MAC_PORT /usr/local/opt)
        set(OSX_RELOCATE_BINARY ${CMAKE_SOURCE_DIR}/autobuild/osx/relocate-binary.sh)
    
        set(OSX_BINARIES 
            ffmpeg/bin/ffmpeg
            ffmpeg/bin/ffprobe

            libdv/bin/encodedv
            sox/bin/sox

            gdk-pixbuf/bin/gdk-pixbuf-query-loaders
            gdk-pixbuf/bin/gdk-pixbuf-pixdata
            gtk+3/bin/gtk3-demo

            mlt/bin/melt
            imagemagick/bin/animate
            imagemagick/bin/composite
            imagemagick/bin/convert

            python3/bin/python3
        )
        file(GLOB PYTHON_FRAMEWORK
            ${MAC_PORT}/python3/Frameworks/Python.framework/Versions/3.*/Resources/Python.app/Contents/MacOS/Python
        )
        string(REPLACE "${MAC_PORT}/" "" PYTHON_FRAMEWORK ${PYTHON_FRAMEWORK})
        list(APPEND OSX_BINARIES ${PYTHON_FRAMEWORK})
    
        foreach(OSX_BINARY ${OSX_BINARIES}) 
            get_filename_component(OSX_BINARY_NAME ${MAC_PORT}/${OSX_BINARY} NAME) 
            add_custom_command( 
                OUTPUT ${SYNFIG_BUILD_ROOT}/bin/${OSX_BINARY_NAME}
                COMMAND ${OSX_RELOCATE_BINARY} ${MAC_PORT}/${OSX_BINARY} ${MAC_PORT} ${SYNFIG_BUILD_ROOT}
                DEPENDS ${MAC_PORT}/${OSX_BINARY}
            )
            install(CODE "
                execute_process(COMMAND ${OSX_RELOCATE_BINARY} ${MAC_PORT}/${OSX_BINARY} ${MAC_PORT} \${CMAKE_INSTALL_PREFIX}/SynfigStudio.app/Contents/Resources)
            ")

            list(APPEND OSX_RELOCATED_BINARIES ${SYNFIG_BUILD_ROOT}/bin/${OSX_BINARY_NAME})
        endforeach()

        file(GLOB OSX_LIBRARIES
            ${MAC_PORT}/gdk-pixbuf/lib/gdk-pixbuf-2.0/2.10.0/loaders/*.so

            ${MAC_PORT}/librsvg/lib/gdk-pixbuf-2.0/2.10.0/loaders/*.so
            ${MAC_PORT}/librsvg-2.40.20/lib/gdk-pixbuf-2.0/2.10.0/loaders/*.so

            ${MAC_PORT}/gtk+3/lib/gtk-3.0/3.0.0/immodules/*.so
            ${MAC_PORT}/gtk+3/lib/gtk-3.0/3.0.0/printbackends/*.so

            ${MAC_PORT}/cairo/lib/cairo/*.so

            ${MAC_PORT}/mlt/lib/mlt/*.so

            ${MAC_PORT}/imagemagick/lib/ImageMagick*/modules-*/coders/*.so
            ${MAC_PORT}/imagemagick/lib/ImageMagick*/modules-*/filters/*.so
        )

        foreach(OSX_LIBRARY ${OSX_LIBRARIES}) 
            get_filename_component(OSX_LIBRARY_NAME ${OSX_LIBRARY} NAME) 
            add_custom_command( 
                OUTPUT ${SYNFIG_BUILD_ROOT}/lib/${OSX_LIBRARY_NAME}
                COMMAND ${OSX_RELOCATE_BINARY} ${OSX_LIBRARY} ${MAC_PORT} ${SYNFIG_BUILD_ROOT}
                DEPENDS ${OSX_LIBRARY}
            )
            install(CODE "
                execute_process(COMMAND ${OSX_RELOCATE_BINARY} ${OSX_LIBRARY} ${MAC_PORT} \${CMAKE_INSTALL_PREFIX}/SynfigStudio.app/Contents/Resources)
            ")

            list(APPEND OSX_RELOCATED_LIBRARIES ${SYNFIG_BUILD_ROOT}/lib/${OSX_LIBRARY_NAME})
        endforeach()

        file(GLOB PYTHON_LIBRARY
            ${MAC_PORT}/python3/Frameworks/Python.framework/Versions/3.*/lib
        )
        string(REPLACE "${MAC_PORT}/python3/Frameworks/Python.framework/Versions/" "" PYTHON_VERSION ${PYTHON_LIBRARY})
        string(REPLACE "/lib" "" PYTHON_VERSION ${PYTHON_VERSION})
        file(COPY ${PYTHON_LIBRARY} DESTINATION ${SYNFIG_BUILD_ROOT}/Frameworks/Python.framework/Versions/${PYTHON_VERSION})
        install(
            DIRECTORY ${PYTHON_LIBRARY}
            DESTINATION SynfigStudio.app/Contents/Resources/Frameworks/Python.framework/Versions/${PYTHON_VERSION}
        )

        file(GLOB OSX_SHARES
            ${MAC_PORT}/gdk-pixbuf/share/gir-1.0
            ${MAC_PORT}/gdk-pixbuf/share/locale

            ${MAC_PORT}/gtk+3/share/icons
            ${MAC_PORT}/gtk+3/share/gir-1.0
            ${MAC_PORT}/gtk+3/share/glib-2.0
            ${MAC_PORT}/gtk+3/share/locale
            ${MAC_PORT}/gtk+3/share/themes

            ${MAC_PORT}/gsettings-desktop-schemas/share/glib-2.0/share/glib-2.0
            ${MACPORTS}/hicolor-icon-theme/share/icons

            ${MACPORTS}/glib/share/glib-2.0
            ${MACPORTS}/glib/share/locale
            ${MACPORTS}/mlt/share/mlt
        )
        file(COPY ${OSX_SHARES} DESTINATION ${SYNFIG_BUILD_ROOT}/share)
        install(
            DIRECTORY ${OSX_SHARES}
            DESTINATION SynfigStudio.app/Contents/Resources/share
        )

        file(GLOB ADWAITA_ICON_THEME
            ${MACPORTS}/adwaita-icon-theme/share/icons/Adwaita
        )
        file(COPY ${ADWAITA_ICON_THEME} DESTINATION ${SYNFIG_BUILD_ROOT}/share/icons)
        install(
            DIRECTORY ${ADWAITA_ICON_THEME}
            DESTINATION SynfigStudio.app/Contents/Resources/share/icons
        )
        
        add_custom_target(relocate_osx_binaries DEPENDS ${OSX_RELOCATED_BINARIES} ${OSX_RELOCATED_LIBRARIES})
        add_dependencies(SynfigStudio relocate_osx_binaries)

        # Relocate Synfig Binary now.
        file(GLOB OSX_SYNFIG_BINARIES
            ${SYNFIG_BUILD_ROOT}/bin/synfig*
            ${SYNFIG_BUILD_ROOT}/lib/libsynfig*.dylib
            ${SYNFIG_BUILD_ROOT}/lib/synfig/Modules/*.so
        )
        
        foreach(OSX_SYNFIG_BINARY ${OSX_SYNFIG_BINARIES})
            install(CODE "
                execute_process(COMMAND ${OSX_RELOCATE_BINARY} ${OSX_SYNFIG_BINARY} ${SYNFIG_BUILD_ROOT} \${CMAKE_INSTALL_PREFIX}/SynfigStudio.app/Contents/Resources)
            ")
        endforeach()

        install(CODE "
            execute_process(COMMAND chmod +x \${CMAKE_INSTALL_PREFIX}/SynfigStudio.app/Contents/MacOS/SynfigStudio.sh)
        ")
    endif()
endif()