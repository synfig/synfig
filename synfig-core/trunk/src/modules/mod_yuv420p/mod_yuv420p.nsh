; The stuff to install
Section "mod_yuv420p" Sec_mod_yuv420p

;  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR\lib\sinfg\modules"
  
  ; Put file there
  File /oname=mod_yuv420p.dll "src\modules\mod_yuv420p\.libs\libmod_yuv420p-0.dll"


  FileOpen $0 $INSTDIR\etc\synfig_modules.cfg a
  FileSeek $0 0 END
  FileWrite $0 "mod_yuv420p"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileClose $0

SectionEnd

