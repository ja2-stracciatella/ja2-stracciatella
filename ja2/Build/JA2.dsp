# Microsoft Developer Studio Project File - Name="ja2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ja2 - Win32 Demo Bounds Checker
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "JA2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "JA2.mak" CFG="ja2 - Win32 Demo Bounds Checker"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ja2 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ja2 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "ja2 - Win32 Release with Debug Info" (based on "Win32 (x86) Application")
!MESSAGE "ja2 - Win32 Bounds Checker" (based on "Win32 (x86) Application")
!MESSAGE "ja2 - Win32 Debug Demo" (based on "Win32 (x86) Application")
!MESSAGE "ja2 - Win32 Release Demo" (based on "Win32 (x86) Application")
!MESSAGE "ja2 - Win32 Demo Release with Debug Info" (based on "Win32 (x86) Application")
!MESSAGE "ja2 - Win32 Demo Bounds Checker" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Jagged Alliance 2/Development/Programming/Jagged Alliance 2/Build", AVAAAAAA"
# PROP Scc_LocalPath "..\..\ja2source\ja2\build"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ja2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MT /W3 /GX /Zi /O2 /I "\Standard Gaming Platform" /I ".\TileEngine" /I ".\Tactical" /I ".\Utils" /I ".\TacticalAI" /I ".\Communications" /I ".\Editor" /I ".\Strategic" /I ".\laptop" /D "NDEBUG" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"JA2 All.h" /FD /O2b2 /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib smackw32.lib mss32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"\ja2\ja2.exe"

!ELSEIF  "$(CFG)" == "ja2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "\Standard Gaming Platform" /I ".\TileEngine" /I ".\Tactical" /I ".\Utils" /I ".\TacticalAI" /I ".\Communications" /I ".\Editor" /I ".\Strategic" /I ".\Laptop" /D "_DEBUG" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"JA2 All.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 vtuneapi.lib mss32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib smackw32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /nodefaultlib:"libc" /out:"\ja2\ja2.exe"

!ELSEIF  "$(CFG)" == "ja2 - Win32 Release with Debug Info"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ja2___Wi"
# PROP BASE Intermediate_Dir "ja2___Wi"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release with Debug"
# PROP Intermediate_Dir "Release with Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /Zi /O2 /I "\Standard Gaming Platform" /I ".\TileEngine" /I ".\Tactical" /I ".\Utils" /I ".\TacticalAI" /I ".\Communications" /I ".\Editor" /I ".\Strategic" /I ".\laptop" /D "NDEBUG" /D "CALLBACKTIMER" /D "JA2" /D "WIN32" /D "_WINDOWS" /FR /YX /FD /O2b2 /c
# ADD CPP /nologo /G5 /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I ".\TileEngine" /I ".\Tactical" /I ".\Utils" /I ".\TacticalAI" /I ".\Communications" /I ".\Editor" /I ".\Strategic" /I ".\laptop" /D "NDEBUG" /D "RELEASE_WITH_DEBUG_INFO" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"JA2 All.h" /FD /O2b2 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"\ja2\ja2.exe"
# ADD LINK32 vtuneapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib smackw32.lib mss32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"\ja2\ja2.exe"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "ja2 - Win32 Bounds Checker"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ja2___W1"
# PROP BASE Intermediate_Dir "ja2___W1"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Bounds Checker"
# PROP Intermediate_Dir "Bounds Checker"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "\Standard Gaming Platform" /I ".\TileEngine" /I ".\Tactical" /I ".\Utils" /I ".\TacticalAI" /I ".\Communications" /I ".\Editor" /I ".\Strategic" /I ".\Laptop" /D "_DEBUG" /D "JA2" /D "WIN32" /D "_WINDOWS" /D "CALLBACKTIMER" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "\Standard Gaming Platform" /I ".\TileEngine" /I ".\Tactical" /I ".\Utils" /I ".\TacticalAI" /I ".\Communications" /I ".\Editor" /I ".\Strategic" /I ".\Laptop" /D "_DEBUG" /D "BOUNDS_CHECKER" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"JA2 All.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"\ja2\ja2.exe"
# ADD LINK32 vtuneapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib smackw32.lib mss32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"\ja2\ja2.exe"

!ELSEIF  "$(CFG)" == "ja2 - Win32 Debug Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ja2___W0"
# PROP BASE Intermediate_Dir "ja2___W0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug Demo"
# PROP Intermediate_Dir "Debug Demo"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "\Standard Gaming Platform" /I ".\TileEngine" /I ".\Tactical" /I ".\Utils" /I ".\TacticalAI" /I ".\Communications" /I ".\Editor" /I ".\Strategic" /I ".\Laptop" /D "_DEBUG" /D "JA2" /D "WIN32" /D "_WINDOWS" /D "CALLBACKTIMER" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "\Standard Gaming Platform" /I ".\TileEngine" /I ".\Tactical" /I ".\Utils" /I ".\TacticalAI" /I ".\Communications" /I ".\Editor" /I ".\Strategic" /I ".\Laptop" /D "_DEBUG" /D "JA2DEMO" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"JA2 All.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"\ja2\ja2.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib smackw32.lib mss32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"d:\ja2demo\ja2.exe"

!ELSEIF  "$(CFG)" == "ja2 - Win32 Release Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ja2___W2"
# PROP BASE Intermediate_Dir "ja2___W2"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release Demo"
# PROP Intermediate_Dir "Release Demo"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I ".\TileEngine" /I ".\Tactical" /I ".\Utils" /I ".\TacticalAI" /I ".\Communications" /I ".\Editor" /I ".\Strategic" /I ".\laptop" /D "NDEBUG" /D "CALLBACKTIMER" /D "JA2" /D "WIN32" /D "_WINDOWS" /D "RELEASE_WITH_DEBUG_INFO" /FR /YX /FD /O2b2 /c
# ADD CPP /nologo /G5 /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I ".\TileEngine" /I ".\Tactical" /I ".\Utils" /I ".\TacticalAI" /I ".\Communications" /I ".\Editor" /I ".\Strategic" /I ".\laptop" /D "RELEASE_WITH_DEBUG_INFO" /D "NDEBUG" /D "JA2DEMO" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"JA2 All.h" /FD /O2b2 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"\ja2\ja2.exe"
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib smackw32.lib mss32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"d:\ja2demo\ja2.exe"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "ja2 - Win32 Demo Release with Debug Info"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ja2___W3"
# PROP BASE Intermediate_Dir "ja2___W3"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Demo Release with Debug"
# PROP Intermediate_Dir "Demo Release with Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I ".\TileEngine" /I ".\Tactical" /I ".\Utils" /I ".\TacticalAI" /I ".\Communications" /I ".\Editor" /I ".\Strategic" /I ".\laptop" /D "NDEBUG" /D "CALLBACKTIMER" /D "JA2" /D "WIN32" /D "_WINDOWS" /D "RELEASE_WITH_DEBUG_INFO" /FR /YX /FD /O2b2 /c
# ADD CPP /nologo /G5 /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I ".\TileEngine" /I ".\Tactical" /I ".\Utils" /I ".\TacticalAI" /I ".\Communications" /I ".\Editor" /I ".\Strategic" /I ".\laptop" /D "RELEASE_WITH_DEBUG_INFO" /D "NDEBUG" /D "JA2DEMO" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"JA2 All.h" /FD /O2b2 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"\ja2\ja2.exe"
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib smackw32.lib mss32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"d:\ja2demo\ja2.exe"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "ja2 - Win32 Demo Bounds Checker"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ja2___W4"
# PROP BASE Intermediate_Dir "ja2___W4"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Demo Bounds Checker"
# PROP Intermediate_Dir "Demo Bounds Checker"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "\Standard Gaming Platform" /I ".\TileEngine" /I ".\Tactical" /I ".\Utils" /I ".\TacticalAI" /I ".\Communications" /I ".\Editor" /I ".\Strategic" /I ".\Laptop" /D "CALLBACKTIMER" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "BOUNDS_CHECKER" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "\Standard Gaming Platform" /I ".\TileEngine" /I ".\Tactical" /I ".\Utils" /I ".\TacticalAI" /I ".\Communications" /I ".\Editor" /I ".\Strategic" /I ".\Laptop" /D "_DEBUG" /D "BOUNDS_CHECKER" /D "JA2DEMO" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"JA2 All.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"\ja2\ja2.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib smackw32.lib mss32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"d:\ja2demo\ja2.exe"

!ENDIF 

# Begin Target

# Name "ja2 - Win32 Release"
# Name "ja2 - Win32 Debug"
# Name "ja2 - Win32 Release with Debug Info"
# Name "ja2 - Win32 Bounds Checker"
# Name "ja2 - Win32 Debug Demo"
# Name "ja2 - Win32 Release Demo"
# Name "ja2 - Win32 Demo Release with Debug Info"
# Name "ja2 - Win32 Demo Bounds Checker"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\Utils\_Ja25GermanText.c
# End Source File
# Begin Source File

SOURCE=.\aniviewscreen.c
# End Source File
# Begin Source File

SOURCE=.\Credits.c
# End Source File
# Begin Source File

SOURCE=".\Fade Screen.c"
# End Source File
# Begin Source File

SOURCE=.\GameInitOptionsScreen.c
# End Source File
# Begin Source File

SOURCE=.\gameloop.c
# End Source File
# Begin Source File

SOURCE=.\gamescreen.c
# End Source File
# Begin Source File

SOURCE=.\GameSettings.c
# End Source File
# Begin Source File

SOURCE=.\GameVersion.c
# End Source File
# Begin Source File

SOURCE=.\HelpScreen.c
# End Source File
# Begin Source File

SOURCE=.\Init.c
# End Source File
# Begin Source File

SOURCE=.\Intro.c
# End Source File
# Begin Source File

SOURCE=".\JA2 Splash.c"
# End Source File
# Begin Source File

SOURCE=.\Res\ja2.rc
# ADD BASE RSC /l 0x409 /i "Res"
# ADD RSC /l 0x409 /i "Res" /i ".\Res"
# End Source File
# Begin Source File

SOURCE=.\jascreens.c
# End Source File
# Begin Source File

SOURCE=".\Loading Screen.c"
# End Source File
# Begin Source File

SOURCE=.\MainMenuScreen.c
# End Source File
# Begin Source File

SOURCE=.\MessageBoxScreen.c
# End Source File
# Begin Source File

SOURCE=".\Options Screen.c"
# End Source File
# Begin Source File

SOURCE=.\SaveLoadGame.c
# End Source File
# Begin Source File

SOURCE=.\SaveLoadScreen.c
# End Source File
# Begin Source File

SOURCE=.\SCREENS.C
# End Source File
# Begin Source File

SOURCE=".\Sys Globals.c"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\builddefines.h
# End Source File
# Begin Source File

SOURCE=.\Credits.h
# End Source File
# Begin Source File

SOURCE=.\GameInitOptionsScreen.h
# End Source File
# Begin Source File

SOURCE=.\gameloop.h
# End Source File
# Begin Source File

SOURCE=.\GameSettings.h
# End Source File
# Begin Source File

SOURCE=".\JA2 All.h"
# End Source File
# Begin Source File

SOURCE=".\JA2 Demo Ads.h"
# End Source File
# Begin Source File

SOURCE=.\jascreens.h
# End Source File
# Begin Source File

SOURCE=".\Language Defines.h"
# End Source File
# Begin Source File

SOURCE=".\Loading Screen.h"
# End Source File
# Begin Source File

SOURCE=.\local.h
# End Source File
# Begin Source File

SOURCE=.\MessageBoxScreen.h
# End Source File
# Begin Source File

SOURCE=".\Options Screen.h"
# End Source File
# Begin Source File

SOURCE=.\SaveLoadGame.h
# End Source File
# Begin Source File

SOURCE=.\SaveLoadScreen.h
# End Source File
# Begin Source File

SOURCE=.\screenids.h
# End Source File
# Begin Source File

SOURCE=.\SCREENS.H
# End Source File
# Begin Source File

SOURCE=.\Strategic\strategicmap.h
# End Source File
# Begin Source File

SOURCE=".\Sys Globals.h"
# End Source File
# Begin Source File

SOURCE=.\VtuneApi.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Res\jagged3.ico
# End Source File
# End Target
# End Project
