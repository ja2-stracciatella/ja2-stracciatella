# Microsoft Developer Studio Project File - Name="TileEngine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=TileEngine - Win32 Demo Bounds Checker
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TileEngine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TileEngine.mak" CFG="TileEngine - Win32 Demo Bounds Checker"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TileEngine - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "TileEngine - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "TileEngine - Win32 Release with Debug Info" (based on "Win32 (x86) Static Library")
!MESSAGE "TileEngine - Win32 Bounds Checker" (based on "Win32 (x86) Static Library")
!MESSAGE "TileEngine - Win32 Debug Demo" (based on "Win32 (x86) Static Library")
!MESSAGE "TileEngine - Win32 Release Demo" (based on "Win32 (x86) Static Library")
!MESSAGE "TileEngine - Win32 Demo Release with Debug Info" (based on "Win32 (x86) Static Library")
!MESSAGE "TileEngine - Win32 Demo Bounds Checker" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Jagged Alliance 2/Development/Programming/Jagged Alliance 2/Build", AVAAAAAA"
# PROP Scc_LocalPath "..\..\..\ja2source\ja2\build"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TileEngine - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\build\tacticalai" /I "\ja2\build\Editor" /I "\ja2\build\strategic" /I "\ja2\build\Laptop" /D "NDEBUG" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"TileEngine All.h" /FD /O2b2 /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TileEngine - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Editor" /I "\ja2\build\strategic" /I "\ja2\build\Laptop" /D "_DEBUG" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"TileEngine All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TileEngine - Win32 Release with Debug Info"

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
# ADD BASE CPP /nologo /MT /W3 /GX /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\build\tacticalai" /I "\ja2\build\Editor" /I "\ja2\build\strategic" /I "\ja2\build\Laptop" /D "NDEBUG" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /FR /YX /FD /O2b2 /c
# ADD CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\build\tacticalai" /I "\ja2\build\Editor" /I "\ja2\build\strategic" /I "\ja2\build\Laptop" /D "NDEBUG" /D "RELEASE_WITH_DEBUG_INFO" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"TileEngine All.h" /FD /O2b2 /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TileEngine - Win32 Bounds Checker"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TileEng0"
# PROP BASE Intermediate_Dir "TileEng0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Bounds Checker"
# PROP Intermediate_Dir "Bounds Checker"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Editor" /I "\ja2\build\strategic" /I "\ja2\build\Laptop" /D "_DEBUG" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Editor" /I "\ja2\build\strategic" /I "\ja2\build\Laptop" /D "_DEBUG" /D "BOUNDS_CHECKER" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"TileEngine All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TileEngine - Win32 Debug Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TileEngi"
# PROP BASE Intermediate_Dir "TileEngi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug Demo"
# PROP Intermediate_Dir "Debug Demo"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Editor" /I "\ja2\build\strategic" /I "\ja2\build\Laptop" /D "_DEBUG" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Editor" /I "\ja2\build\strategic" /I "\ja2\build\Laptop" /D "_DEBUG" /D "JA2DEMO" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"TileEngine All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TileEngine - Win32 Release Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TileEng1"
# PROP BASE Intermediate_Dir "TileEng1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release Demo"
# PROP Intermediate_Dir "Release Demo"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\build\tacticalai" /I "\ja2\build\Editor" /I "\ja2\build\strategic" /I "\ja2\build\Laptop" /D "NDEBUG" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "RELEASE_WITH_DEBUG_INFO" /FR /YX /FD /O2b2 /c
# ADD CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\build\tacticalai" /I "\ja2\build\Editor" /I "\ja2\build\strategic" /I "\ja2\build\Laptop" /D "RELEASE_WITH_DEBUG_INFO" /D "NDEBUG" /D "JA2DEMO" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"TileEngine All.h" /FD /O2b2 /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TileEngine - Win32 Demo Release with Debug Info"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TileEng2"
# PROP BASE Intermediate_Dir "TileEng2"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Demo Release with Debug"
# PROP Intermediate_Dir "Demo Release with Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\build\tacticalai" /I "\ja2\build\Editor" /I "\ja2\build\strategic" /I "\ja2\build\Laptop" /D "NDEBUG" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "RELEASE_WITH_DEBUG_INFO" /FR /YX /FD /O2b2 /c
# ADD CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\build\tacticalai" /I "\ja2\build\Editor" /I "\ja2\build\strategic" /I "\ja2\build\Laptop" /D "RELEASE_WITH_DEBUG_INFO" /D "NDEBUG" /D "JA2DEMO" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"TileEngine All.h" /FD /O2b2 /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TileEngine - Win32 Demo Bounds Checker"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TileEng3"
# PROP BASE Intermediate_Dir "TileEng3"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Demo Bounds Checker"
# PROP Intermediate_Dir "Demo Bounds Checker"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Editor" /I "\ja2\build\strategic" /I "\ja2\build\Laptop" /D "CALLBACKTIMER" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "BOUNDS_CHECKER" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Editor" /I "\ja2\build\strategic" /I "\ja2\build\Laptop" /D "_DEBUG" /D "BOUNDS_CHECKER" /D "JA2DEMO" /D "CALLBACKTIMER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"TileEngine All.h" /FD /c
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

# Name "TileEngine - Win32 Release"
# Name "TileEngine - Win32 Debug"
# Name "TileEngine - Win32 Release with Debug Info"
# Name "TileEngine - Win32 Bounds Checker"
# Name "TileEngine - Win32 Debug Demo"
# Name "TileEngine - Win32 Release Demo"
# Name "TileEngine - Win32 Demo Release with Debug Info"
# Name "TileEngine - Win32 Demo Bounds Checker"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=".\Ambient Control.c"
# End Source File
# Begin Source File

SOURCE=.\Buildings.c
# End Source File
# Begin Source File

SOURCE=.\environment.c
# End Source File
# Begin Source File

SOURCE=".\Exit Grids.c"
# End Source File
# Begin Source File

SOURCE=".\Explosion Control.c"
# End Source File
# Begin Source File

SOURCE=".\Fog Of War.c"
# End Source File
# Begin Source File

SOURCE=".\Interactive Tiles.c"
# End Source File
# Begin Source File

SOURCE=".\Isometric Utils.c"
# End Source File
# Begin Source File

SOURCE=.\LightEffects.c
# End Source File
# Begin Source File

SOURCE=.\lighting.c
# End Source File
# Begin Source File

SOURCE=".\Map Edgepoints.c"
# End Source File
# Begin Source File

SOURCE=".\overhead map.c"
# End Source File
# Begin Source File

SOURCE=".\phys math.c"
# End Source File
# Begin Source File

SOURCE=.\physics.c
# End Source File
# Begin Source File

SOURCE=.\pits.c
# End Source File
# Begin Source File

SOURCE=".\Radar Screen.c"
# End Source File
# Begin Source File

SOURCE=".\Render Dirty.c"
# End Source File
# Begin Source File

SOURCE=".\Render Fun.c"
# End Source File
# Begin Source File

SOURCE=.\renderworld.c
# End Source File
# Begin Source File

SOURCE=.\SaveLoadMap.c
# End Source File
# Begin Source File

SOURCE=".\Shade Table Util.c"
# End Source File
# Begin Source File

SOURCE=".\Simple Render Utils.c"
# End Source File
# Begin Source File

SOURCE=.\Smell.c
# End Source File
# Begin Source File

SOURCE=.\SmokeEffects.c
# End Source File
# Begin Source File

SOURCE=.\structure.c
# End Source File
# Begin Source File

SOURCE=.\sysutil.c
# End Source File
# Begin Source File

SOURCE=".\Tactical Placement GUI.c"
# End Source File
# Begin Source File

SOURCE=".\Tile Animation.c"
# End Source File
# Begin Source File

SOURCE=".\Tile Cache.c"
# End Source File
# Begin Source File

SOURCE=".\Tile Surface.c"
# End Source File
# Begin Source File

SOURCE=.\TileDat.c
# End Source File
# Begin Source File

SOURCE=.\tiledef.c
# End Source File
# Begin Source File

SOURCE=.\WorldDat.c
# End Source File
# Begin Source File

SOURCE=.\worlddef.c
# End Source File
# Begin Source File

SOURCE=.\worldman.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=".\Ambient Control.h"
# End Source File
# Begin Source File

SOURCE=".\Ambient Types.h"
# End Source File
# Begin Source File

SOURCE=.\Buildings.h
# End Source File
# Begin Source File

SOURCE=.\edit_sys.h
# End Source File
# Begin Source File

SOURCE=.\environment.h
# End Source File
# Begin Source File

SOURCE=".\Exit Grids.h"
# End Source File
# Begin Source File

SOURCE=".\Fog Of War.h"
# End Source File
# Begin Source File

SOURCE=".\Interactive Tiles.h"
# End Source File
# Begin Source File

SOURCE=".\Isometric Utils.h"
# End Source File
# Begin Source File

SOURCE=.\lighting.h
# End Source File
# Begin Source File

SOURCE=".\Map Edgepoints.h"
# End Source File
# Begin Source File

SOURCE=.\pits.h
# End Source File
# Begin Source File

SOURCE=".\Radar Screen.h"
# End Source File
# Begin Source File

SOURCE=".\render dirty.h"
# End Source File
# Begin Source File

SOURCE=".\Render Fun.h"
# End Source File
# Begin Source File

SOURCE=.\renderworld.h
# End Source File
# Begin Source File

SOURCE=.\SaveLoadMap.h
# End Source File
# Begin Source File

SOURCE=".\Shade Table Util.h"
# End Source File
# Begin Source File

SOURCE=".\Simple Render Utils.h"
# End Source File
# Begin Source File

SOURCE=.\Smell.h
# End Source File
# Begin Source File

SOURCE=".\Structure Internals.h"
# End Source File
# Begin Source File

SOURCE=.\structure.h
# End Source File
# Begin Source File

SOURCE=.\sysutil.h
# End Source File
# Begin Source File

SOURCE=".\Tile Animation.h"
# End Source File
# Begin Source File

SOURCE=.\TileDat.h
# End Source File
# Begin Source File

SOURCE=.\tiledef.h
# End Source File
# Begin Source File

SOURCE=".\TileEngine All.h"
# End Source File
# Begin Source File

SOURCE=.\WorldDat.h
# End Source File
# Begin Source File

SOURCE=.\worlddef.h
# End Source File
# Begin Source File

SOURCE=.\worldman.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
