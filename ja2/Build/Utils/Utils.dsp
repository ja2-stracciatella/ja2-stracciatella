# Microsoft Developer Studio Project File - Name="Utils" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Utils - Win32 Demo Bounds Checker
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Utils.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Utils.mak" CFG="Utils - Win32 Demo Bounds Checker"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Utils - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Utils - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Utils - Win32 Release with Debug Info" (based on "Win32 (x86) Static Library")
!MESSAGE "Utils - Win32 Bounds Checker" (based on "Win32 (x86) Static Library")
!MESSAGE "Utils - Win32 Debug Demo" (based on "Win32 (x86) Static Library")
!MESSAGE "Utils - Win32 Release Demo" (based on "Win32 (x86) Static Library")
!MESSAGE "Utils - Win32 Demo Release with Debug Info" (based on "Win32 (x86) Static Library")
!MESSAGE "Utils - Win32 Demo Bounds Checker" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Jagged Alliance 2/Development/Programming/Jagged Alliance 2/Build", AVAAAAAA"
# PROP Scc_LocalPath "..\..\..\ja2source\ja2\build"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Utils - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir "."
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir "."
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "\ja2\Build\Communications" /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\editor" /D "NDEBUG" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"Utils All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Utils - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir "."
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir "."
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\ja2\build\communications" /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\editor" /D "_DEBUG" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"Utils All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Utils - Win32 Release with Debug Info"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release with Debug Info"
# PROP BASE Intermediate_Dir "Release with Debug Info"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release with Debug"
# PROP Intermediate_Dir "Release with Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "\ja2\Build\Communications" /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\editor" /D "NDEBUG" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /FR /YX /FD /c
# ADD CPP /nologo /MT /W4 /GX /Zi /O2 /I "\ja2\Build\Communications" /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\editor" /D "NDEBUG" /D "RELEASE_WITH_DEBUG_INFO" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /Fr /YX"Utils All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Utils - Win32 Bounds Checker"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Utils__0"
# PROP BASE Intermediate_Dir "Utils__0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Bounds Checker"
# PROP Intermediate_Dir "Bounds Checker"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\ja2\build\communications" /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\editor" /D "_DEBUG" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\ja2\build\communications" /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\editor" /D "_DEBUG" /D "BOUNDS_CHECKER" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"Utils All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Utils - Win32 Debug Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Utils___"
# PROP BASE Intermediate_Dir "Utils___"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug Demo"
# PROP Intermediate_Dir "Debug Demo"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\ja2\build\communications" /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\editor" /D "_DEBUG" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\ja2\build\communications" /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\editor" /D "_DEBUG" /D "JA2DEMO" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"Utils All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Utils - Win32 Release Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Utils__1"
# PROP BASE Intermediate_Dir "Utils__1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release Demo"
# PROP Intermediate_Dir "Release Demo"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W4 /GX /Zi /O2 /I "\ja2\Build\Communications" /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\editor" /D "NDEBUG" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "RELEASE_WITH_DEBUG_INFO" /Fr /YX /FD /c
# ADD CPP /nologo /MT /W4 /GX /Zi /O2 /I "\ja2\Build\Communications" /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\editor" /D "RELEASE_WITH_DEBUG_INFO" /D "NDEBUG" /D "JA2DEMO" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /Fr /YX"Utils All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Utils - Win32 Demo Release with Debug Info"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Utils__2"
# PROP BASE Intermediate_Dir "Utils__2"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Demo Release with Debug"
# PROP Intermediate_Dir "Demo Release with Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W4 /GX /Zi /O2 /I "\ja2\Build\Communications" /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\editor" /D "NDEBUG" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "RELEASE_WITH_DEBUG_INFO" /Fr /YX /FD /c
# ADD CPP /nologo /MT /W4 /GX /Zi /O2 /I "\ja2\Build\Communications" /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\editor" /D "RELEASE_WITH_DEBUG_INFO" /D "NDEBUG" /D "JA2DEMO" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /Fr /YX"Utils All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Utils - Win32 Demo Bounds Checker"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Utils__3"
# PROP BASE Intermediate_Dir "Utils__3"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Demo Bounds Checker"
# PROP Intermediate_Dir "Demo Bounds Checker"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\ja2\build\communications" /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\editor" /D "CALLBACKTIMER" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "BOUNDS_CHECKER" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\ja2\build\communications" /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\editor" /D "_DEBUG" /D "BOUNDS_CHECKER" /D "JA2DEMO" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"Utils All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "Utils - Win32 Release"
# Name "Utils - Win32 Debug"
# Name "Utils - Win32 Release with Debug Info"
# Name "Utils - Win32 Bounds Checker"
# Name "Utils - Win32 Debug Demo"
# Name "Utils - Win32 Release Demo"
# Name "Utils - Win32 Demo Release with Debug Info"
# Name "Utils - Win32 Demo Bounds Checker"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\_DutchText.c
# End Source File
# Begin Source File

SOURCE=.\_EnglishText.c
# End Source File
# Begin Source File

SOURCE=.\_FrenchText.c
# End Source File
# Begin Source File

SOURCE=.\_GermanText.c
# End Source File
# Begin Source File

SOURCE=.\_ItalianText.c
# End Source File
# Begin Source File

SOURCE=.\_Ja25EnglishText.c
# End Source File
# Begin Source File

SOURCE=.\_Ja25GermanText.c
# End Source File
# Begin Source File

SOURCE=.\_PolishText.c
# End Source File
# Begin Source File

SOURCE=.\_RussianText.c
# End Source File
# Begin Source File

SOURCE=".\Animated ProgressBar.c"
# End Source File
# Begin Source File

SOURCE=.\Cinematics.c
# End Source File
# Begin Source File

SOURCE=.\Cursors.c
# End Source File
# Begin Source File

SOURCE=".\Debug Control.c"
# End Source File
# Begin Source File

SOURCE=.\dsutil.c
# End Source File
# Begin Source File

SOURCE=".\Encrypted File.c"
# End Source File
# Begin Source File

SOURCE=".\Event Manager.c"
# End Source File
# Begin Source File

SOURCE=".\Event Pump.c"
# End Source File
# Begin Source File

SOURCE=".\Font Control.c"
# End Source File
# Begin Source File

SOURCE=.\MapUtility.c
# End Source File
# Begin Source File

SOURCE=.\MercTextBox.c
# End Source File
# Begin Source File

SOURCE=.\message.c
# End Source File
# Begin Source File

SOURCE=".\Multi Language Graphic Utils.c"
# End Source File
# Begin Source File

SOURCE=".\Multilingual Text Code Generator.c"
# End Source File
# Begin Source File

SOURCE=".\Music Control.c"
# End Source File
# Begin Source File

SOURCE=.\PopUpBox.c
# End Source File
# Begin Source File

SOURCE=".\Quantize Wrap.cpp"
# End Source File
# Begin Source File

SOURCE=.\Quantize.cpp
# End Source File
# Begin Source File

SOURCE=.\Slider.c
# End Source File
# Begin Source File

SOURCE=".\Sound Control.c"
# End Source File
# Begin Source File

SOURCE=.\STIConvert.c
# End Source File
# Begin Source File

SOURCE=".\Text Input.c"
# End Source File
# Begin Source File

SOURCE=".\Text Utils.c"
# End Source File
# Begin Source File

SOURCE=".\Timer Control.c"
# End Source File
# Begin Source File

SOURCE=.\Utilities.c
# End Source File
# Begin Source File

SOURCE=.\WordWrap.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\_Ja25GermanText.h
# End Source File
# Begin Source File

SOURCE=".\Animated ProgressBar.h"
# End Source File
# Begin Source File

SOURCE=.\cursors.h
# End Source File
# Begin Source File

SOURCE=".\Debug Control.h"
# End Source File
# Begin Source File

SOURCE=.\dsutil.h
# End Source File
# Begin Source File

SOURCE=".\Event Manager.h"
# End Source File
# Begin Source File

SOURCE=".\Event Pump.h"
# End Source File
# Begin Source File

SOURCE=".\Font Control.h"
# End Source File
# Begin Source File

SOURCE=.\message.h
# End Source File
# Begin Source File

SOURCE=".\Multi Language Graphic Utils.h"
# End Source File
# Begin Source File

SOURCE=".\Multilingual Text Code Generator.h"
# End Source File
# Begin Source File

SOURCE=".\Music Control.h"
# End Source File
# Begin Source File

SOURCE=".\Sound Control.h"
# End Source File
# Begin Source File

SOURCE=".\Text Input.h"
# End Source File
# Begin Source File

SOURCE=.\Text.h
# End Source File
# Begin Source File

SOURCE=".\Timer Control.h"
# End Source File
# Begin Source File

SOURCE=.\utilities.h
# End Source File
# Begin Source File

SOURCE=".\Utils All.h"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
