; The stuff to install
Section "mod_openexr" Sec_mod_openexr

;  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR\lib\sinfg\modules"
  
  ; Put file there
  File /oname=mod_openexr.dll "src\modules\mod_openexr\.libs\libmod_openexr-0.dll"


  FileOpen $0 $INSTDIR\etc\synfig_modules.cfg a
  FileSeek $0 0 END
  FileWrite $0 "mod_openexr"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileClose $0

SectionEnd

