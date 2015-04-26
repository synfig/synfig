; example2.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install makensisw.exe into a directory that the user selects,

!include "MUI2.nsh"

;--------------------------------

; The name of the installer
Name "Synfig Studio @VERSION@"

; The file to write
OutFile "synfigstudio-@VERSION@.exe"

; The default installation directory and registry
!include "arch-specific.nsh"

; Request application privileges for Windows Vista
RequestExecutionLevel highest

!define MUI_ABORTWARNING

!define SHCNE_ASSOCCHANGED 0x8000000
!define SHCNF_IDLIST 0

!define PRODUCT_REG_KEY "Software\synfigstudio\0.0"
!define PRODUCT_UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\synfigstudio"
!define PRODUCT_UNINSTALL_EXE "uninstall-synfigstudio.exe"

;--------------------------------

;!define MUI_HEADERIMAGE
;!define MUI_HEADERIMAGE_BITMAP "installer_logo.bmp"
;!define MUI_WELCOMEFINISHPAGE_BITMAP ".\share\pixmaps\installer_logo.bmp"

; Pages

!insertmacro MUI_PAGE_WELCOME
;!insertmacro MUI_PAGE_LICENSE ".\alphalicense.txt"
!insertmacro MUI_PAGE_LICENSE ".\licenses\synfigstudio.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

;--------------------------------

; The stuff to install
Section "Synfig Studio"

  SectionIn RO

  SetOutPath "$INSTDIR\bin"
  !include "bin.nsh"
  
  SetOutPath "$INSTDIR\etc"
  !include "etc.nsh"

  SetOutPath "$INSTDIR\lib"
  !include "lib-gdk-pixbuf.nsh"
  !include "lib-gtk.nsh"
  !include "lib-pango.nsh"
  !include "lib-synfig.nsh"
  
  SetOutPath "$INSTDIR\licenses"
  !include "licenses.nsh"
  
  SetOutPath "$INSTDIR\share"
  !include "share.nsh"

  ;SetOutPath "$INSTDIR\python"
  ;!include "python.nsh"
  SetOutPath "$INSTDIR"
  File /r /x .* python

  WriteRegStr HKLM "${PRODUCT_REG_KEY}" "Path" "$INSTDIR"
  WriteRegStr HKLM "${PRODUCT_REG_KEY}" "Version" "@VERSION@"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "${PRODUCT_UNINSTALL_KEY}" "DisplayName" "Synfig Studio"
  WriteRegStr HKLM "${PRODUCT_UNINSTALL_KEY}" "DisplayVersion" "@VERSION@"
  WriteRegStr HKLM "${PRODUCT_UNINSTALL_KEY}" "UninstallString" '"$INSTDIR\${PRODUCT_UNINSTALL_EXE}"'
  WriteRegDWORD HKLM "${PRODUCT_UNINSTALL_KEY}" "NoModify" 1
  WriteRegDWORD HKLM "${PRODUCT_UNINSTALL_KEY}" "NoRepair" 1


	WriteRegStr HKCR ".sif" "" "Synfig.Composition"
	WriteRegStr HKCR ".sif" "Content Type" "image/x-sif"
	WriteRegStr HKCR ".sif" "PerceivedType" "image"

	WriteRegStr HKCR ".sifz" "" "Synfig.Composition"
	WriteRegStr HKCR ".sifz" "Content Type" "image/x-sifz"
	WriteRegStr HKCR ".sifz" "PerceivedType" "image"

	WriteRegStr HKCR "Synfig.Composition" "" "Synfig Composition File"
	WriteRegStr HKCR "Synfig.Composition\DefaultIcon" "" "$INSTDIR\share\pixmaps\sif_icon.ico"
	WriteRegStr HKCR "Synfig.Composition\shell" "" "open"
	WriteRegStr HKCR "Synfig.Composition\shell\open\command" "" '$INSTDIR\bin\synfigstudio.exe "%1"'
	
	System::Call 'Shell32::SHChangeNotify(i ${SHCNE_ASSOCCHANGED}, i ${SHCNF_IDLIST}, i 0, i 0)'
  WriteUninstaller "${PRODUCT_UNINSTALL_EXE}"
SectionEnd

Section "FFMpeg"
	SetOutPath "$INSTDIR\bin"
	File "bin\ffmpeg.exe"
SectionEnd

Section "Examples"
	!include "examples.nsh"
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  SetOutPath "$INSTDIR\bin"

  SetShellVarContext All
  CreateDirectory "$SMPROGRAMS\Synfig"
  CreateShortCut "$SMPROGRAMS\Synfig\Uninstall Synfig Studio.lnk" "$INSTDIR\uninstall-synfigstudio.exe" "" "$INSTDIR\uninstall-synfigstudio.exe" 0
  CreateShortCut "$SMPROGRAMS\Synfig\Synfig Studio.lnk" "$INSTDIR\bin\synfigstudio.exe" "" "$INSTDIR\share\pixmaps\synfig_icon.ico" 0
  CreateShortCut "$SMPROGRAMS\Synfig\Synfig Studio (Debug Console).lnk" "$INSTDIR\bin\synfigstudio.exe" "--console" "$INSTDIR\share\pixmaps\synfig_icon.ico" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  DeleteRegKey HKCR "Synfig.Composition\shell\open\command" 
  DeleteRegKey HKCR "Synfig.Composition\DefaultIcon" 
  DeleteRegKey HKCR "Synfig.Composition\shell"
  DeleteRegKey HKCR "Synfig.Composition" 
  DeleteRegKey HKCR ".sif"
  DeleteRegKey HKCR ".sifz"
  
  ; Remove registry keys
  DeleteRegKey HKLM "${PRODUCT_REG_KEY}"
  DeleteRegKey HKLM "${PRODUCT_UNINSTALL_KEY}"

  ; Remove files and uninstaller
  Delete "$INSTDIR\${PRODUCT_UNINSTALL_EXE}"
  !include "bin-uninst.nsh"
  !include "etc-uninst.nsh"
  !include "examples-uninst.nsh"
  !include "lib-gdk-pixbuf-uninst.nsh"
  !include "lib-gtk-uninst.nsh"
  !include "lib-pango-uninst.nsh"
  !include "lib-synfig-uninst.nsh"
  RMDir "$INSTDIR\lib"
  !include "licenses-uninst.nsh"
  ;!include "python-uninst.nsh"
  RMDir /r "$INSTDIR\python"
  !include "share-uninst.nsh"
  RMDir "$INSTDIR\share"

  ; Remove shortcuts, if any
  SetShellVarContext All
  Delete "$SMPROGRAMS\Synfig\Uninstall Synfig Studio.lnk"
  Delete "$SMPROGRAMS\Synfig\Synfig Studio.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Synfig"
  RMDir "$INSTDIR"

	System::Call 'Shell32::SHChangeNotify(i ${SHCNE_ASSOCCHANGED}, i ${SHCNF_IDLIST}, i 0, i 0)'
SectionEnd

Function .onInit
	; Get installer location
	ReadRegStr $R0 HKLM "${PRODUCT_UNINSTALL_KEY}" "UninstallString"
;	IfErrors 0 +2
;	ReadRegStr $R0 HKCU "${PRODUCT_UNINSTALL_KEY}" "UninstallString"
	
	StrCmp $R0 "" done

	; Get current installed version
	ReadRegStr $R1 HKLM "${PRODUCT_UNINSTALL_KEY}" "DisplayVersion"
;	IfErrors 0 +2
;	ReadRegStr $R1 HKCU "${PRODUCT_UNINSTALL_KEY}" "DisplayVersion"

;  StrCmp $R1 ${PRODUCT_VERSION} done

	MessageBox MB_YESNOCANCEL|MB_ICONEXCLAMATION "A previous version of Synfig Studio appears to be installed. Would you like to uninstall it first?" IDNO done IDCANCEL abortInstall

	; Run the uninstaller
	
	ClearErrors
	; CopyFiles "$R0" $TEMP
	ExecWait '$R0 _?=$INSTDIR'
	IfErrors no_remove_uninstaller
	Delete $R0
	RMDir $INSTDIR

no_remove_uninstaller:
;    Delete "$TEMP\$R0"
	
    ; Check that the user completed the uninstallation by examining the registry
    ReadRegStr $R0 HKLM "${PRODUCT_UNINSTALL_KEY}" "UninstallString"
	StrCmp $R0 "" done abortInstall
	ReadRegStr $R0 HKCU "${PRODUCT_UNINSTALL_KEY}" "UninstallString"
	StrCmp $R0 "" done abortInstall

abortInstall:
    MessageBox MB_OK|MB_ICONEXCLAMATION "Unable to uninstall previous version of Synfig Studio"
    Abort

done:
    BringToFront

FunctionEnd
