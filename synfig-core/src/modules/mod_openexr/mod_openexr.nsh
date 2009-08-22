; The stuff to install
Section "mod_openexr" Sec_mod_openexr

;  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR\lib\synfig\modules"
  
  ; Put file there
  File /oname=mod_openexr.dll "src\modules\mod_openexr\.libs\libmod_openexr.dll"

  SetOutPath "$INSTDIR\bin"
  File "src\modules\mod_openexr\.libs\libHalf*.dll"
  File "src\modules\mod_openexr\.libs\libIlmImf*.dll"
  File "src\modules\mod_openexr\.libs\libIlmThread*.dll"
  File "src\modules\mod_openexr\.libs\libIex*.dll"

  FileOpen $0 $INSTDIR\etc\synfig_modules.cfg a
  FileSeek $0 0 END
  FileWrite $0 "mod_openexr"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileClose $0

SectionEnd

