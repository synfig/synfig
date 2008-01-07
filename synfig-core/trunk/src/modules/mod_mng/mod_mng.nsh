Section "mod_mng" Sec_mod_mng
  SetOutPath "$INSTDIR\lib\synfig\modules"
  File /oname=mod_mng.dll "src\modules\mod_mng\.libs\libmod_mng.dll"
  FileOpen $0 $INSTDIR\etc\synfig_modules.cfg a
  FileSeek $0 0 END
  FileWrite $0 "mod_mng"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileClose $0
SectionEnd

