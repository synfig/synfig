; The stuff to install
Section "mod_ffmpeg" Sec_mod_ffmpeg

;  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR\lib\synfig\modules"
  
  ; Put file there
  File /oname=mod_ffmpeg.dll "src\modules\mod_ffmpeg\.libs\libmod_ffmpeg.dll"


  FileOpen $0 $INSTDIR\etc\synfig_modules.cfg a
  FileSeek $0 0 END
  FileWrite $0 "mod_ffmpeg"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileClose $0

SectionEnd

