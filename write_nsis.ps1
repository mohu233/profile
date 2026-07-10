param($pkgDir, $outExe)
$nsi = @'
!include "MUI2.nsh"
Name "ProfilePower"
OutFile "E:\\Documents\\ProfilePower_Setup.exe"
InstallDir "$PROGRAMFILES\\ProfilePower"
RequestExecutionLevel admin

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_LANGUAGE "SimpChinese"

Section "Install"
    SetOutPath $INSTDIR
    File /r "E:\\Documents\\profile_package\\*.*"
    CreateShortCut "$DESKTOP\\ProfilePower.lnk" "$INSTDIR\\profile.exe"
    CreateDirectory "$SMPROGRAMS\\ProfilePower"
    CreateShortCut "$SMPROGRAMS\\ProfilePower\\ProfilePower.lnk" "$INSTDIR\\profile.exe"
    CreateShortCut "$SMPROGRAMS\\ProfilePower\\Uninstall.lnk" "$INSTDIR\\uninstall.exe"
    WriteUninstaller "$INSTDIR\\uninstall.exe"
    WriteRegStr HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\ProfilePower" "DisplayName" "ProfilePower"
    WriteRegStr HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\ProfilePower" "UninstallString" "$INSTDIR\\uninstall.exe"
    WriteRegStr HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\ProfilePower" "DisplayIcon" "$INSTDIR\\profile.exe"
    WriteRegStr HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\ProfilePower" "Publisher" "ProfilePower"
    WriteRegStr HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\ProfilePower" "DisplayVersion" "1.0"
SectionEnd

Section "Uninstall"
    Delete "$DESKTOP\\ProfilePower.lnk"
    RMDir /r "$SMPROGRAMS\\ProfilePower"
    RMDir /r "$INSTDIR"
    DeleteRegKey HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\ProfilePower"
SectionEnd

'@
$nsiPath = Join-Path $pkgDir "installer.nsi"
[IO.File]::WriteAllText($nsiPath, $nsi, [Text.Encoding]::ASCII)
Write-Host "NSIS script written to: $nsiPath"
