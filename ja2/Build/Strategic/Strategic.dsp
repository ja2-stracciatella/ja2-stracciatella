# Microsoft Developer Studio Project File - Name="Strategic" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Strategic - Win32 Demo Bounds Checker
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Strategic.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Strategic.mak" CFG="Strategic - Win32 Demo Bounds Checker"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Strategic - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Strategic - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Strategic - Win32 Release with Debug Info" (based on "Win32 (x86) Static Library")
!MESSAGE "Strategic - Win32 Bounds Checker" (based on "Win32 (x86) Static Library")
!MESSAGE "Strategic - Win32 Debug Demo" (based on "Win32 (x86) Static Library")
!MESSAGE "Strategic - Win32 Release Demo" (based on "Win32 (x86) Static Library")
!MESSAGE "Strategic - Win32 Demo Release with Debug Info" (based on "Win32 (x86) Static Library")
!MESSAGE "Strategic - Win32 Demo Bounds Checker" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Jagged Alliance 2/Development/Programming/Jagged Alliance 2/Build", AVAAAAAA"
# PROP Scc_LocalPath "..\..\..\ja2source\ja2\build"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Strategic - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\Laptop" /I "\ja2\build\Editor" /I "\ja2\build\tactical" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /YX"Strategic All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Strategic - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\Laptop" /I "\ja2\build\editor" /I "\ja2\build\tactical" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"Strategic All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Strategic - Win32 Release with Debug Info"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Strategi"
# PROP BASE Intermediate_Dir "Strategi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release with Debug"
# PROP Intermediate_Dir "Release with Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\Laptop" /I "\ja2\build\Editor" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /YX /FD /c
# ADD CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\Laptop" /I "\ja2\build\Editor" /D "NDEBUG" /D "RELEASE_WITH_DEBUG_INFO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"Strategic All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Strategic - Win32 Bounds Checker"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Strateg1"
# PROP BASE Intermediate_Dir "Strateg1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Bounds Checker"
# PROP Intermediate_Dir "Bounds Checker"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\Laptop" /I "\ja2\build\editor" /I "\ja2\build\tactical" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\Laptop" /I "\ja2\build\editor" /I "\ja2\build\tactical" /D "_DEBUG" /D "BOUNDS_CHECKER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"Strategic All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Strategic - Win32 Debug Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Strateg0"
# PROP BASE Intermediate_Dir "Strateg0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug Demo"
# PROP Intermediate_Dir "Debug Demo"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\Laptop" /I "\ja2\build\editor" /I "\ja2\build\tactical" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\Laptop" /I "\ja2\build\editor" /I "\ja2\build\tactical" /D "_DEBUG" /D "JA2DEMO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"Strategic All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Strategic - Win32 Release Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Strateg2"
# PROP BASE Intermediate_Dir "Strateg2"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release Demo"
# PROP Intermediate_Dir "Release Demo"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\Laptop" /I "\ja2\build\Editor" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "RELEASE_WITH_DEBUG_INFO" /FR /YX /FD /c
# ADD CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\Laptop" /I "\ja2\build\Editor" /D "RELEASE_WITH_DEBUG_INFO" /D "NDEBUG" /D "JA2DEMO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"Strategic All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Strategic - Win32 Demo Release with Debug Info"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Strateg3"
# PROP BASE Intermediate_Dir "Strateg3"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Demo Release with Debug"
# PROP Intermediate_Dir "Demo Release with Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\Laptop" /I "\ja2\build\Editor" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "RELEASE_WITH_DEBUG_INFO" /FR /YX /FD /c
# ADD CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\Laptop" /I "\ja2\build\Editor" /D "RELEASE_WITH_DEBUG_INFO" /D "NDEBUG" /D "JA2DEMO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"Strategic All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Strategic - Win32 Demo Bounds Checker"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Strateg4"
# PROP BASE Intermediate_Dir "Strateg4"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Demo Bounds Checker"
# PROP Intermediate_Dir "Demo Bounds Checker"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\Laptop" /I "\ja2\build\editor" /I "\ja2\build\tactical" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "BOUNDS_CHECKER" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Tactical" /I "\ja2\build\Laptop" /I "\ja2\build\editor" /I "\ja2\build\tactical" /D "_DEBUG" /D "BOUNDS_CHECKER" /D "JA2DEMO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"Strategic All.h" /FD /c
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

# Name "Strategic - Win32 Release"
# Name "Strategic - Win32 Debug"
# Name "Strategic - Win32 Release with Debug Info"
# Name "Strategic - Win32 Bounds Checker"
# Name "Strategic - Win32 Debug Demo"
# Name "Strategic - Win32 Release Demo"
# Name "Strategic - Win32 Demo Release with Debug Info"
# Name "Strategic - Win32 Demo Bounds Checker"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\AI Viewer.c"
# End Source File
# Begin Source File

SOURCE=.\Assignments.c
# End Source File
# Begin Source File

SOURCE=".\Auto Resolve.c"
# End Source File
# Begin Source File

SOURCE=".\Campaign Init.c"
# End Source File
# Begin Source File

SOURCE=".\Creature Spreading.c"
# End Source File
# Begin Source File

SOURCE=".\Game Clock.c"
# End Source File
# Begin Source File

SOURCE=".\Game Event Hook.c"
# End Source File
# Begin Source File

SOURCE=".\Game Events.c"
# End Source File
# Begin Source File

SOURCE=".\Game Init.c"
# End Source File
# Begin Source File

SOURCE=".\Hourly Update.c"
# End Source File
# Begin Source File

SOURCE=".\Map Screen Helicopter.c"
# End Source File
# Begin Source File

SOURCE=".\Map Screen Interface Border.c"
# End Source File
# Begin Source File

SOURCE=".\Map Screen Interface Bottom.c"
# End Source File
# Begin Source File

SOURCE=".\Map Screen Interface Map Inventory.c"
# End Source File
# Begin Source File

SOURCE=".\Map Screen Interface Map.c"
# End Source File
# Begin Source File

SOURCE=".\Map Screen Interface TownMine Info.c"
# End Source File
# Begin Source File

SOURCE=".\Map Screen Interface.c"
# End Source File
# Begin Source File

SOURCE=.\mapscreen.c
# End Source File
# Begin Source File

SOURCE=.\Meanwhile.c
# End Source File
# Begin Source File

SOURCE=".\Merc Contract.c"
# End Source File
# Begin Source File

SOURCE=".\Player Command.c"
# End Source File
# Begin Source File

SOURCE=".\PreBattle Interface.c"
# End Source File
# Begin Source File

SOURCE=".\Queen Command.c"
# End Source File
# Begin Source File

SOURCE=".\Quest Debug System.c"
# End Source File
# Begin Source File

SOURCE=.\Quests.c
# End Source File
# Begin Source File

SOURCE=.\QuestText.c
# End Source File
# Begin Source File

SOURCE=.\Scheduling.c
# End Source File
# Begin Source File

SOURCE=".\Strategic AI.c"
# End Source File
# Begin Source File

SOURCE=".\Strategic Event Handler.c"
# End Source File
# Begin Source File

SOURCE=".\Strategic Merc Handler.c"
# End Source File
# Begin Source File

SOURCE=".\Strategic Mines.c"
# End Source File
# Begin Source File

SOURCE=".\Strategic Movement Costs.c"
# End Source File
# Begin Source File

SOURCE=".\Strategic Movement.c"
# End Source File
# Begin Source File

SOURCE=".\Strategic Pathing.c"
# End Source File
# Begin Source File

SOURCE=".\Strategic Status.c"
# End Source File
# Begin Source File

SOURCE=".\Strategic Town Loyalty.c"
# End Source File
# Begin Source File

SOURCE=".\strategic town reputation.c"
# End Source File
# Begin Source File

SOURCE=".\Strategic Turns.c"
# End Source File
# Begin Source File

SOURCE=.\strategic.c
# End Source File
# Begin Source File

SOURCE=.\strategicmap.c
# End Source File
# Begin Source File

SOURCE=".\Town Militia.c"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Assignments.h
# End Source File
# Begin Source File

SOURCE=".\Auto Resolve.h"
# End Source File
# Begin Source File

SOURCE=".\Campaign Init.h"
# End Source File
# Begin Source File

SOURCE=".\Campaign Types.h"
# End Source File
# Begin Source File

SOURCE=".\Creature Spreading.h"
# End Source File
# Begin Source File

SOURCE=".\Game Clock.h"
# End Source File
# Begin Source File

SOURCE=".\Game Event Hook.h"
# End Source File
# Begin Source File

SOURCE=".\Game Events.h"
# End Source File
# Begin Source File

SOURCE=".\Game Init.h"
# End Source File
# Begin Source File

SOURCE=".\Map Screen Helicopter.h"
# End Source File
# Begin Source File

SOURCE=".\Map Screen Interface Border.h"
# End Source File
# Begin Source File

SOURCE=".\Map Screen Interface Bottom.h"
# End Source File
# Begin Source File

SOURCE=".\Map Screen Interface Map Inventory.h"
# End Source File
# Begin Source File

SOURCE=".\Map Screen Interface Map.h"
# End Source File
# Begin Source File

SOURCE=".\Map Screen Interface TownMine Info.h"
# End Source File
# Begin Source File

SOURCE=".\Map Screen Interface.h"
# End Source File
# Begin Source File

SOURCE=.\mapscreen.h
# End Source File
# Begin Source File

SOURCE=".\Merc Contract.h"
# End Source File
# Begin Source File

SOURCE=".\Player Command.h"
# End Source File
# Begin Source File

SOURCE=".\PreBattle Interface.h"
# End Source File
# Begin Source File

SOURCE=".\Queen Command.h"
# End Source File
# Begin Source File

SOURCE=.\Quests.h
# End Source File
# Begin Source File

SOURCE=.\Scheduling.h
# End Source File
# Begin Source File

SOURCE=".\Strategic AI.h"
# End Source File
# Begin Source File

SOURCE=".\Strategic All.h"
# End Source File
# Begin Source File

SOURCE=".\Strategic Event Handler.h"
# End Source File
# Begin Source File

SOURCE=".\Strategic Merc Handler.h"
# End Source File
# Begin Source File

SOURCE=".\Strategic Mines.h"
# End Source File
# Begin Source File

SOURCE=".\Strategic Movement.h"
# End Source File
# Begin Source File

SOURCE=".\Strategic Pathing.h"
# End Source File
# Begin Source File

SOURCE=".\Strategic Status.h"
# End Source File
# Begin Source File

SOURCE=".\Strategic Town Loyalty.h"
# End Source File
# Begin Source File

SOURCE=".\strategic town reputation.h"
# End Source File
# Begin Source File

SOURCE=.\strategic.h
# End Source File
# Begin Source File

SOURCE=.\strategicmap.h
# End Source File
# Begin Source File

SOURCE=".\Town Militia.h"
# End Source File
# End Group
# End Target
# End Project
