﻿# The NSIS (http://nsis.sourceforge.net) install script.
# This script is BSD licensed.
SetCompressor /solid /final lzma

!include LogicLib.nsh
!include MUI2.nsh
!include "FileFunc.nsh"

!define VERSION "0.2.0"
!define QUADVERSION "0.2.0.0"
!define guid '{C1B3CE89-4773-4FF3-BFF7-12144DEF2F15}'
!define PROGRAM_NAME "Datovka"

outFile "datovka-${VERSION}-windows.exe"
Name "${PROGRAM_NAME} ${VERSION}"

# default install directory
installDir "$PROGRAMFILES\CZ.NIC\${PROGRAM_NAME}"
installDirRegKey HKLM "Software\${PROGRAM_NAME}" "InstallLocation"
RequestExecutionLevel admin
#give credits to Nullsoft: BrandingText ""
VIAddVersionKey "ProductName" "${PROGRAM_NAME} ${VERSION}"
VIAddVersionKey "CompanyName" "CZ.NIC Labs"
VIAddVersionKey "FileDescription" "(un)install the ${PROGRAM_NAME} ${VERSION}"
VIAddVersionKey "LegalCopyright" "Copyright 2014, CZ.NIC Labs"
VIAddVersionKey "FileVersion" "${QUADVERSION}"
VIAddVersionKey "ProductVersion" "${QUADVERSION}"
VIProductVersion "${QUADVERSION}"

# Global Variables
Var StartMenuFolder

# use ReserveFile for files required before actual installation
# makes the installer start faster
#ReserveFile "System.dll"
#ReserveFile "NsExec.dll"

!define MUI_ICON "datovka.ico"
!define MUI_UNICON "datovka.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP ".\..\common\setup_top.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP ".\..\common\setup_left.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP ".\..\common\setup_left.bmp"
!define MUI_ABORTWARNING
!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_FINISHPAGE_TITLE_3LINES
;!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of ${PROGRAM_NAME} ${VERSION}.$\r$\n$\nNote: It is recommended to close all running ${PROGRAM_NAME} windows before proceeding with the installation of the add-on.$\r$\n$\r$\nClick Next to continue."
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "./../app/COPYING"
!insertmacro MUI_PAGE_DIRECTORY

!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${PROGRAM_NAME}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "CZ.NIC\${PROGRAM_NAME}"
!insertmacro MUI_PAGE_STARTMENU DatovkaStartMenu $StartMenuFolder

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Czech"

section "-hidden.postinstall"
	# copy files
	setOutPath $INSTDIR
  File /r ".\..\app\*"
	File ".\datovka.ico"
  
	# store installation folder
	WriteRegStr HKLM "Software\${PROGRAM_NAME}" "InstallLocation" "$INSTDIR"
	# register uninstaller
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_NAME}" "DisplayName" "${PROGRAM_NAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_NAME}" "UninstallString" "$\"$INSTDIR\uninst.exe$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_NAME}" "QuietUninstallString" "$\"$INSTDIR\uninst.exe$\" /S"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_NAME}" "NoModify" "1"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_NAME}" "NoRepair" "1"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_NAME}" "URLInfoAbout" "https://labs.nic.cz/page/2425/nova-datovka/"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_NAME}" "Publisher" "CZ.NIC Labs"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_NAME}" "Version" "${VERSION}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_NAME}" "DisplayVersion" "${VERSION}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_NAME}" "Contact" "CZ.NIC Labs"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_NAME}" "DisplayIcon" "$\"$INSTDIR\datovka.ico$\""
	${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
	IntFmt $0 "0x%08X" $0
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_NAME}" "EstimatedSize" "$0"
	WriteUninstaller "uninst.exe"

	# start menu items
	!insertmacro MUI_STARTMENU_WRITE_BEGIN DatovkaStartMenu
	CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\uninst.exe" "" "" "" "" "" "Uninstall ${PROGRAM_NAME}"
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Datovka.lnk" "$INSTDIR\datovka.exe" "" "" "" "" "" "Run ${PROGRAM_NAME}"
	!insertmacro MUI_STARTMENU_WRITE_END
  
  CreateShortCut "$DESKTOP\${PROGRAM_NAME}.lnk" "$INSTDIR\${PROGRAM_NAME}.exe" "" "$INSTDIR\datovka.ico" 0

sectionEnd

# setup macros for uninstall functions.
!ifdef UN
!undef UN
!endif
!define UN "un."

# uninstaller section
section "un.Unbound"
  	# deregister uninstall
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_NAME}"
	RMDir /r "$INSTDIR\*" 
  Delete "$DESKTOP\${PROGRAM_NAME}.lnk"
	Delete "$INSTDIR\uninst.exe"   # delete self
	RMDir "$PROGRAMFILES\CZ.NIC\${PROGRAM_NAME}"
	RMDir "$PROGRAMFILES\CZ.NIC"
	RMDir "$INSTDIR"

	# start menu items
	!insertmacro MUI_STARTMENU_GETFOLDER DatovkaStartMenu $StartMenuFolder
	Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
	Delete "$SMPROGRAMS\$StartMenuFolder\Datovka.lnk"
	RMDir "$SMPROGRAMS\CZ.NIC\${PROGRAM_NAME}"
	RMDir "$SMPROGRAMS\CZ.NIC\"
	RMDir "$SMPROGRAMS\$StartMenuFolder"
	DeleteRegKey HKLM "Software\${PROGRAM_NAME}"
   
sectionEnd

Function .onInit

	ReadRegStr $R0 HKLM \
	"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_NAME}" \
	"UninstallString"

	${If} $R0 != ""		
		${If} $LANGUAGE == ${LANG_ENGLISH}
		MessageBox MB_OK|MB_ICONEXCLAMATION \
		"${PROGRAM_NAME} is already installed on your computer. $\n$\nClick OK to remove the \
	  	currently installed Datovka and start installation of version ${VERSION}." \
		IDOK uninst
		Abort
		${EndIf}

		${If} $LANGUAGE == ${LANG_CZECH}
		MessageBox MB_OK|MB_ICONEXCLAMATION \
		"${PROGRAM_NAME} už je na Vašem počítači nainstalována. $\n$\nStiskněte OK pro její odinstalování \
	  	a nahrání nové verze ${VERSION}." \
		IDOK uninst
		Abort
		${EndIf}
    
  ${EndIf}    
    
	;Run the uninstaller
	uninst:
		ClearErrors
		${If} $R0 != "" 
			ExecWait '$R0 _?=$INSTDIR' ;Do not copy the uninstaller to a temp file
		${EndIf}	
		
		IfErrors no_remove_uninstaller done
	
	no_remove_uninstaller:
	
	done:

FunctionEnd