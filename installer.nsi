; ============================================
; untitled2 - NSIS 妫夎鍖呰剼鏈?; ============================================
; 浣跨敤鏂规晠锛?;   1. 妫夎 NSIS锛歨ttps://nsis.sourceforge.io/Download
;   2. 鍏堣繍琛?build_release.bat
;   3. 鍐嶈繍琛?deploy_release.bat
;   4. 鍙抽敭鏈枃浠?鈫?"Compile NSIS Script"
;      鎴栬繍琛?build_installer.bat
; ============================================

Unicode true
ManifestDPIAware true

; ---- 鐗堟湰淇℃伅锛堟牴鎹渶瑕佷慨鏀癸級 ----
!define PRODUCT_NAME "untitled2"
!define PRODUCT_VERSION "1.0.0"
!define PRODUCT_PUBLISHER "untitled2"
!define PRODUCT_WEB_SITE ""

; ---- 妫夎鍖呮枃浠跺悕 ----
OutFile "untitled2_Setup_v${PRODUCT_VERSION}.exe"

; ---- 榛樿妫夎璺緞 ----
InstallDir "$PROGRAMFILES64\${PRODUCT_NAME}"

; ---- 璇锋眰绠＄悊鍛樻潈闄?----
RequestExecutionLevel admin

; ---- 鐣岄潰璁剧疆 ----
BrandingText " "
ShowInstDetails show
ShowUnInstDetails show

; ---- 鍘嬬缉鍣?----
SetCompressor /SOLID lzma
SetCompressorDictSize 64

; ---- 鍖呭惈 Modern UI 2 ----
!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "x64.nsh"

; ---- 鑷畾涔夊畯 ----
!define MUI_ABORTWARNING
!define MUI_ICON ""
!define MUI_UNICON ""

; ---- 妫夎椤甸潰 ----
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"

; ---- 妫夎鐩綍閫夋嫨椤?----
!define MUI_PAGE_CUSTOMFUNCTION_PRE "CheckPublishExists"
!insertmacro MUI_PAGE_DIRECTORY

; ---- 妫夎杩涘害椤?----
!insertmacro MUI_PAGE_INSTFILES

; ---- 瀹屾垚椤碉紙鍕鹃€夎繍琛岀▼搴忥級 ----
!define MUI_FINISHPAGE_RUN "$INSTDIR\${PRODUCT_NAME}.exe"
!define MUI_FINISHPAGE_RUN_TEXT "绔嬪嵆杩愯 ${PRODUCT_NAME}"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\浣跨敤璇存槑.txt"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "鏌ョ湅浣跨敤璇存槑"
!insertmacro MUI_PAGE_FINISH

; ---- 鍗歌浇椤甸潰 ----
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; ---- 璇█ ----
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "English"

; ---- 妫夎鍓嶆鏌ュ彂甯冪洰褰曟槸鍚﹀瓨鍦?----
Function CheckPublishExists
    IfFileExists "$EXEDIR\publish\${PRODUCT_NAME}.exe" +2
        abort "閿欒锛氭壘涓嶅埌 'publish' 鍙戝竷鐩綍鎴栦富绋嬪簭 exe锛佽鍏堣繍琛?deploy_release.bat銆?
FunctionEnd

; ============================================
; 妫夎娈?; ============================================
Section "妫夎绋嬪簭" SecInstall
    SetOutPath "$INSTDIR"

    ; 璁剧疆鏂囦欢灞炴€?    SetOverwrite on

    ; ---- 澶嶅埗涓荤▼搴?----
    File "publish\${PRODUCT_NAME}.exe"

    ; ---- 澶嶅埗 Qt DLL 鍜屼緷璧?----
    File /r "publish\*.dll"
    File /r "publish\*.conf"
    File /r "publish\qt*.dat"

    ; ---- 澶嶅埗 Qt 鎻掍欢鐩綍 ----
    ; platform 鎻掍欢
    CreateDirectory "$INSTDIR\platforms"
    File /r "publish\platforms\*.dll"

    ; styles 鎻掍欢
    CreateDirectory "$INSTDIR\styles"
    File /r "publish\styles\*.dll"

    ; imageformats 鎻掍欢
    CreateDirectory "$INSTDIR\imageformats"
    File /r "publish\imageformats\*.dll"

    ; iconengines 鎻掍欢
    CreateDirectory "$INSTDIR\iconengines"
    File /r "publish\iconengines\*.dll"

    ; bearer / network 鎻掍欢
    CreateDirectory "$INSTDIR\bearer"
    File /r "publish\bearer\*.dll"

    ; tls 鎻掍欢
    CreateDirectory "$INSTDIR\tls"
    File /r "publish\tls\*.dll"

    ; ---- 澶嶅埗 Qt 妯″潡鐩綍锛堝鏋滄湁锛?----
    ; Qt 6 鍙兘鏈夎繖浜涚洰褰?    CreateDirectory "$INSTDIR\generic"
    File /r "publish\generic\*.*"

    CreateDirectory "$INSTDIR\multimedia"
    File /r "publish\multimedia\*.*"

    ; ---- 澶嶅埗鏁版嵁鐩綍 ----
    IfFileExists "$EXEDIR\publish\modules\*.*" 0 +2
        File /r "publish\modules"

    IfFileExists "$EXEDIR\publish\profiles\*.*" 0 +2
        File /r "publish\profiles"

    ; ---- 澶嶅埗鍚姩鑴氭湰鍜岃鏄?----
    File "publish\鍚姩绋嬪簭.bat"
    File "publish\璇存槑.txt"

    ; ---- 澶嶅埗 Python 妫夎鍖咃紙鐢ㄤ簬鏃?Python 鐜涓嬮潤榛樺畨瑁咃級 ----
    IfFileExists "$EXEDIR\publish\python-3.10.11-amd64.exe" 0 +2
        File "publish\python-3.10.11-amd64.exe"

    ; ---- 澶嶅埗 translations锛圦t 璇█鍖咃紝濡傛灉鏈夌殑璇濓級 ----
    IfFileExists "$EXEDIR\publish\translations\*.*" 0 +2
        File /r "publish\translations"

    ; ---- 鍐欏叆鍗歌浇淇℃伅鍒版敞鍐岃〃 ----
    WriteUninstaller "$INSTDIR\uninst.exe"

    ; 鍐欏叆"娣诲姞/鍒犻櫎绋嬪簭"娉ㄥ唽琛?    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" \
        "DisplayName" "${PRODUCT_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" \
        "UninstallString" "$INSTDIR\uninst.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" \
        "DisplayVersion" "${PRODUCT_VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" \
        "Publisher" "${PRODUCT_PUBLISHER}"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" \
        "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" \
        "NoRepair" 1

    ; 璁＄畻妫夎澶у皬
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" \
        "EstimatedSize" "$0"

    ; ---- 鍒涘缓妗岄潰蹇嵎鏂瑰紡 ----
    CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\${PRODUCT_NAME}.exe" "" "$INSTDIR\${PRODUCT_NAME}.exe" 0

    ; ---- 鍒涘缓寮€濮嬭彍鍗曞揩鎹锋柟寮?----
    CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\${PRODUCT_NAME}.exe"
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\鍗歌浇 ${PRODUCT_NAME}.lnk" "$INSTDIR\uninst.exe"

SectionEnd

; ============================================
; 鍗歌浇娈?; ============================================
Section "Uninstall"
    ; 鍋滄鍙兘姝ｅ湪杩愯鐨勭▼搴?    ExecWait "taskkill /f /im ${PRODUCT_NAME}.exe" $0

    ; 鍒犻櫎寮€濮嬭彍鍗曢」
    RMDir /r "$SMPROGRAMS\${PRODUCT_NAME}"

    ; 鍒犻櫎妗岄潰蹇嵎鏂瑰紡
    Delete "$DESKTOP\${PRODUCT_NAME}.lnk"

    ; 鍒犻櫎妫夎鐩綍锛堜繚鐣?profiles 鐢ㄦ埛鏁版嵁锛岄槻姝㈣鍒狅級
    Delete "$INSTDIR\${PRODUCT_NAME}.exe"
    Delete "$INSTDIR\鍚姩绋嬪簭.bat"
    Delete "$INSTDIR\璇存槑.txt"
    Delete "$INSTDIR\uninst.exe"

    ; 娓呯悊 DLL
    Delete "$INSTDIR\*.dll"
    Delete "$INSTDIR\*.conf"
    Delete "$INSTDIR\*.dat"

    ; 娓呯悊鎻掍欢鐩綍
    RMDir /r "$INSTDIR\platforms"
    RMDir /r "$INSTDIR\styles"
    RMDir /r "$INSTDIR\imageformats"
    RMDir /r "$INSTDIR\iconengines"
    RMDir /r "$INSTDIR\bearer"
    RMDir /r "$INSTDIR\tls"
    RMDir /r "$INSTDIR\generic"
    RMDir /r "$INSTDIR\multimedia"
    RMDir /r "$INSTDIR\translations"
    RMDir /r "$INSTDIR\modules"

    ; 璇㈤棶鏄惁鍒犻櫎 profiles 鐢ㄦ埛鏁版嵁
    IfFileExists "$INSTDIR\profiles\*.*" 0 +4
    MessageBox MB_YESNO|MB_ICONQUESTION \
        "鏄惁鍚屾椂鍒犻櫎鐜閰嶇疆鏁版嵁 (profiles)锛?\n$\n閫夋嫨銆屾槸銆嶅皢鍒犻櫎鎵€鏈夋祻瑙堝櫒鐜鍜岄厤缃€? \
        IDNO +2
        RMDir /r "$INSTDIR\profiles"

    ; 鍒犻櫎妫夎鐩綍鏈韩
    RMDir "$INSTDIR"

    ; 娓呯悊娉ㄥ唽琛?    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"

SectionEnd