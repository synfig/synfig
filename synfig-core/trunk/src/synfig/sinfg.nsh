; The stuff to install
Section "Synfig Core"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR\bin"
  
  ; Put file there
  File "src\sinfg\.libs\libsinfg-0.dll"

SectionEnd

Section "un.Synfig Core"
	Delete "$INSTDIR\bin\libsinfg-0.dll"
	RMDir "$INSTDIR"
SectionEnd


