; The stuff to install
Section "mod_geometry" Sec_mod_geometry

;  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR\lib\sinfg\modules"
  
  ; Put file there
  File /oname=mod_geometry.dll "src\modules\mod_geometry\.libs\libmod_geometry-0.dll"


  FileOpen $0 $INSTDIR\etc\synfig_modules.cfg a
  FileSeek $0 0 END
  FileWrite $0 "mod_geometry"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileClose $0

SectionEnd

