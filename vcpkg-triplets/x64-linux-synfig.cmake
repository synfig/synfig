set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)
set(VCPKG_FIXUP_ELF_RPATH ON)

set(__DYNAMIC_PORTS
  glib
  gdk-pixbuf
  ffmpeg
  glibmm
  gtk3
  gtkmm3
  pangomm
  cairomm
  librsvg
)

foreach(__DYNAMIC_PORT ${__DYNAMIC_PORTS})
  if(PORT MATCHES "${__DYNAMIC_PORT}")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
  endif()
endforeach()
