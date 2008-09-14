Section "un.mod_openexr"
	Delete "$INSTDIR\bin\libIex*.dll"
	Delete "$INSTDIR\bin\libIlmImf*.dll"
	Delete "$INSTDIR\bin\libIlmThread*.dll"
	Delete "$INSTDIR\bin\libHalf*.dll"
	Delete "$INSTDIR\lib\synfig\modules\mod_openexr.dll"
	RMDir "$INSTDIR\lib\synfig\modules"
	RMDir "$INSTDIR\lib\synfig"
	RMDir "$INSTDIR\lib"
	RMDir "$INSTDIR\bin"
	RMDir "$INSTDIR"
SectionEnd

