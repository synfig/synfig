; The stuff to install
Section "Synfig Tool"

;  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR\bin"
  
  ; Put file there
  File "src\tool\.libs\synfig.exe"

SectionEnd

Section "un.Synfig Tool Uninstall"
	Delete "$INSTDIR\bin\synfig.exe"
	RMDir "$INSTDIR\bin"
	RMDir "$INSTDIR"
SectionEnd


