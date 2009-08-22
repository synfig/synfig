; The stuff to install
Section "mod_svg" Sec_mod_svg

;  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR\lib\synfig\modules"
  
  ; Put file there
  File /oname=mod_svg.dll "src\modules\mod_svg\.libs\libmod_svg.dll"


  FileOpen $0 $INSTDIR\etc\synfig_modules.cfg a
  FileSeek $0 0 END
  FileWrite $0 "mod_svg"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileClose $0

SectionEnd
