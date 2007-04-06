; The stuff to install
Section "Synfig Core"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR\bin"
  
  ; Put file there
  File "src\synfig\.libs\libsynfig-0.dll"

SectionEnd

Section "un.Synfig Core"
	Delete "$INSTDIR\bin\libsynfig-0.dll"
	RMDir "$INSTDIR\bin"
	RMDir "$INSTDIR"
SectionEnd


