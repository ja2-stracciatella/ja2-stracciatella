# Microsoft Developer Studio Project File - Name="Editor" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Editor - Win32 Demo Bounds Checker
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Editor.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Editor.mak" CFG="Editor - Win32 Demo Bounds Checker"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Editor - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Editor - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Editor - Win32 Release with Debug Info" (based on "Win32 (x86) Static Library")
!MESSAGE "Editor - Win32 Bounds Checker" (based on "Win32 (x86) Static Library")
!MESSAGE "Editor - Win32 Debug Demo" (based on "Win32 (x86) Static Library")
!MESSAGE "Editor - Win32 Release Demo" (based on "Win32 (x86) Static Library")
!MESSAGE "Editor - Win32 Demo Release with Debug Info" (based on "Win32 (x86) Static Library")
!MESSAGE "Editor - Win32 Demo Bounds Checker" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Jagged Alliance 2/Development/Programming/Jagged Alliance 2/Build", AVAAAAAA"
# PROP Scc_LocalPath "..\..\..\ja2source\ja2\build"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Editor - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /YX"Editor All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Editor - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"Editor All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Editor - Win32 Release with Debug Info"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Editor__"
# PROP BASE Intermediate_Dir "Editor__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release with Debug"
# PROP Intermediate_Dir "Release with Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "JA2" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"Editor All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Editor - Win32 Bounds Checker"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Editor_1"
# PROP BASE Intermediate_Dir "Editor_1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Bounds Checker"
# PROP Intermediate_Dir "Bounds Checker"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "JA2" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "_DEBUG" /D "BOUNDS_CHECKER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"Editor All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Editor - Win32 Debug Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Editor_0"
# PROP BASE Intermediate_Dir "Editor_0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug Demo"
# PROP Intermediate_Dir "Debug Demo"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "JA2" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "JA2DEMO" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"Editor All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Editor - Win32 Release Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Editor_2"
# PROP BASE Intermediate_Dir "Editor_2"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release Demo"
# PROP Intermediate_Dir "Release Demo"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "JA2" /FR /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "NDEBUG" /D "JA2DEMO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"Editor All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Editor - Win32 Demo Release with Debug Info"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Editor_3"
# PROP BASE Intermediate_Dir "Editor_3"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Demo Release with Debug"
# PROP Intermediate_Dir "Demo Release with Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "JA2" /FR /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "NDEBUG" /D "JA2DEMO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"Editor All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Editor - Win32 Demo Bounds Checker"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Editor_4"
# PROP BASE Intermediate_Dir "Editor_4"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Demo Bounds Checker"
# PROP Intermediate_Dir "Demo Bounds Checker"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "BOUNDS_CHECKER" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "_DEBUG" /D "BOUNDS_CHECKER" /D "JA2DEMO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"Editor All.h" /FD /c
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

# Name "Editor - Win32 Release"
# Name "Editor - Win32 Debug"
# Name "Editor - Win32 Release with Debug Info"
# Name "Editor - Win32 Bounds Checker"
# Name "Editor - Win32 Debug Demo"
# Name "Editor - Win32 Release Demo"
# Name "Editor - Win32 Demo Release with Debug Info"
# Name "Editor - Win32 Demo Bounds Checker"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\Cursor Modes.c"
# End Source File
# Begin Source File

SOURCE=.\edit_sys.c
# End Source File
# Begin Source File

SOURCE=".\Editor Callbacks.c"
# End Source File
# Begin Source File

SOURCE=".\Editor Modes.c"
# End Source File
# Begin Source File

SOURCE=".\Editor Taskbar Creation.c"
# End Source File
# Begin Source File

SOURCE=".\Editor Taskbar Utils.c"
# End Source File
# Begin Source File

SOURCE=".\Editor Undo.c"
# End Source File
# Begin Source File

SOURCE=.\EditorBuildings.c
# End Source File
# Begin Source File

SOURCE=.\EditorItems.c
# End Source File
# Begin Source File

SOURCE=.\EditorMapInfo.c
# End Source File
# Begin Source File

SOURCE=.\EditorMercs.c
# End Source File
# Begin Source File

SOURCE=.\EditorTerrain.c
# End Source File
# Begin Source File

SOURCE=.\editscreen.c
# End Source File
# Begin Source File

SOURCE=".\Item Statistics.c"
# End Source File
# Begin Source File

SOURCE=.\LoadScreen.c
# End Source File
# Begin Source File

SOURCE=.\messagebox.c
# End Source File
# Begin Source File

SOURCE=.\newsmooth.c
# End Source File
# Begin Source File

SOURCE=.\popupmenu.c
# End Source File
# Begin Source File

SOURCE=".\Road Smoothing.c"
# End Source File
# Begin Source File

SOURCE=".\Sector Summary.c"
# End Source File
# Begin Source File

SOURCE=.\selectwin.c
# End Source File
# Begin Source File

SOURCE=.\SmartMethod.c
# End Source File
# Begin Source File

SOURCE=.\smooth.c
# End Source File
# Begin Source File

SOURCE=".\Smoothing Utils.c"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\Button Defines.h"
# End Source File
# Begin Source File

SOURCE=".\Cursor Modes.h"
# End Source File
# Begin Source File

SOURCE=.\edit_sys.h
# End Source File
# Begin Source File

SOURCE=".\Editor All.h"
# End Source File
# Begin Source File

SOURCE=".\Editor Callback Prototypes.h"
# End Source File
# Begin Source File

SOURCE=".\Editor Modes.h"
# End Source File
# Begin Source File

SOURCE=".\Editor Taskbar Creation.h"
# End Source File
# Begin Source File

SOURCE=".\Editor Taskbar Utils.h"
# End Source File
# Begin Source File

SOURCE=".\Editor Undo.h"
# End Source File
# Begin Source File

SOURCE=.\EditorBuildings.h
# End Source File
# Begin Source File

SOURCE=.\EditorDefines.h
# End Source File
# Begin Source File

SOURCE=.\EditorItems.h
# End Source File
# Begin Source File

SOURCE=.\EditorMapInfo.h
# End Source File
# Begin Source File

SOURCE=.\EditorMercs.h
# End Source File
# Begin Source File

SOURCE=.\EditorTerrain.h
# End Source File
# Begin Source File

SOURCE=.\editscreen.h
# End Source File
# Begin Source File

SOURCE=".\Item Statistics.h"
# End Source File
# Begin Source File

SOURCE=.\LoadScreen.h
# End Source File
# Begin Source File

SOURCE=.\messagebox.h
# End Source File
# Begin Source File

SOURCE=.\newsmooth.h
# End Source File
# Begin Source File

SOURCE=.\popupmenu.h
# End Source File
# Begin Source File

SOURCE=".\Road Smoothing.h"
# End Source File
# Begin Source File

SOURCE=".\Sector Summary.h"
# End Source File
# Begin Source File

SOURCE=.\selectwin.h
# End Source File
# Begin Source File

SOURCE=.\SmartMethod.h
# End Source File
# Begin Source File

SOURCE=.\smooth.h
# End Source File
# Begin Source File

SOURCE=".\Smoothing Utils.h"
# End Source File
# Begin Source File

SOURCE=".\Summary Info.h"
# End Source File
# End Group
# End Target
# End Project
