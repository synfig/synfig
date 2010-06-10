; The stuff to install
Section "mod_example" Sec_mod_example

;  SectionIn RO

  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR\lib\synfig\modules"

  ; Put file there
  File /oname=mod_example.dll "src\modules\mod_example\.libs\libmod_example.dll"


  FileOpen $0 $INSTDIR\etc\synfig_modules.cfg a
  FileSeek $0 0 END
  FileWrite $0 "mod_example"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileClose $0

SectionEnd

