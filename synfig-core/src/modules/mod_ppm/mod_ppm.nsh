; The stuff to install
Section "mod_ppm" Sec_mod_ppm

;  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR\lib\synfig\modules"
  
  ; Put file there
  File /oname=mod_ppm.dll "src\modules\mod_ppm\.libs\libmod_ppm.dll"


  FileOpen $0 $INSTDIR\etc\synfig_modules.cfg a
  FileSeek $0 0 END
  FileWrite $0 "mod_ppm"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileClose $0

SectionEnd

