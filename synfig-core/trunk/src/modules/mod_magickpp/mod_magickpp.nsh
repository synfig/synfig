Section "mod_magickpp" Sec_mod_magickpp
  SetOutPath "$INSTDIR\lib\synfig\modules"
  File /oname=mod_magickpp.dll "src\modules\mod_magickpp\.libs\libmod_magickpp.dll"
  FileOpen $0 $INSTDIR\etc\synfig_modules.cfg a
  FileSeek $0 0 END
  FileWrite $0 "mod_magickpp"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileClose $0
SectionEnd

