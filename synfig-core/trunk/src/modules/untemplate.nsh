Section "un.@MODNAME@"
	Delete "$INSTDIR\lib\synfig\modules\@MODNAME@.dll"
	RMDir "$INSTDIR\lib\synfig\modules"
	RMDir "$INSTDIR\lib\synfig"
	RMDir "$INSTDIR\lib"
	RMDir "$INSTDIR"
SectionEnd

