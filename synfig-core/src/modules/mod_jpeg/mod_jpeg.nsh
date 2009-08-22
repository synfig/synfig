; The stuff to install
Section "mod_jpeg" Sec_mod_jpeg

;  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR\lib\synfig\modules"
  
  ; Put file there
  File /oname=mod_jpeg.dll "src\modules\mod_jpeg\.libs\libmod_jpeg.dll"


  FileOpen $0 $INSTDIR\etc\synfig_modules.cfg a
  FileSeek $0 0 END
  FileWrite $0 "mod_jpeg"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileClose $0

SectionEnd

