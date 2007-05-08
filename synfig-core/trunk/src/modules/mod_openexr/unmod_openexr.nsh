Section "un.mod_openexr"
	Delete "$INSTDIR\bin\libIex-4.dll"
	Delete "$INSTDIR\bin\libIlmImf-4.dll"
	Delete "$INSTDIR\bin\libIlmThread-4.dll"
	Delete "$INSTDIR\bin\libHalf-4.dll"
	Delete "$INSTDIR\lib\synfig\modules\mod_openexr.dll"
	RMDir "$INSTDIR\lib\synfig\modules"
	RMDir "$INSTDIR\lib\synfig"
	RMDir "$INSTDIR\lib"
	RMDir "$INSTDIR\bin"
	RMDir "$INSTDIR"
SectionEnd

