; The stuff to install
Section "mod_png" Sec_mod_png

;  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR\lib\synfig\modules"
  
  ; Put file there
  File /oname=mod_png.dll "src\modules\mod_png\.libs\libmod_png.dll"


  FileOpen $0 $INSTDIR\etc\synfig_modules.cfg a
  FileSeek $0 0 END
  FileWrite $0 "mod_png"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileClose $0

SectionEnd

