# Microsoft Developer Studio Project File - Name="TacticalAI" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=TacticalAI - Win32 Demo Bounds Checker
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TacticalAI.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TacticalAI.mak" CFG="TacticalAI - Win32 Demo Bounds Checker"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TacticalAI - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "TacticalAI - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "TacticalAI - Win32 Release with Debug Info" (based on "Win32 (x86) Static Library")
!MESSAGE "TacticalAI - Win32 Bounds Checker" (based on "Win32 (x86) Static Library")
!MESSAGE "TacticalAI - Win32 Debug Demo" (based on "Win32 (x86) Static Library")
!MESSAGE "TacticalAI - Win32 Release Demo" (based on "Win32 (x86) Static Library")
!MESSAGE "TacticalAI - Win32 Demo Release with Debug Info" (based on "Win32 (x86) Static Library")
!MESSAGE "TacticalAI - Win32 Demo Bounds Checker" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Jagged Alliance 2/Development/Programming/Jagged Alliance 2/Build", AVAAAAAA"
# PROP Scc_LocalPath "..\..\..\ja2source\ja2\build"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TacticalAI - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\laptop" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"AI All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TacticalAI - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\ja2\build\laptop" /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"AI All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TacticalAI - Win32 Release with Debug Info"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Tactical"
# PROP BASE Intermediate_Dir "Tactical"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release with Debug"
# PROP Intermediate_Dir "Release with Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "JA2" /FR /YX /FD /c
# ADD CPP /nologo /MT /W4 /GX /Zi /O2 /I "\ja2\build\laptop" /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /D "NDEBUG" /D "RELEASE_WITH_DEBUG_INFO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /Fr /YX"AI All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TacticalAI - Win32 Bounds Checker"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Tactica1"
# PROP BASE Intermediate_Dir "Tactica1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Bounds Checker"
# PROP Intermediate_Dir "Bounds Checker"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "JA2" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\laptop" /D "_DEBUG" /D "BOUNDS_CHECKER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"AI All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TacticalAI - Win32 Debug Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Tactica0"
# PROP BASE Intermediate_Dir "Tactica0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug Demo"
# PROP Intermediate_Dir "Debug Demo"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "JA2" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\laptop" /D "_DEBUG" /D "JA2DEMO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"AI All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TacticalAI - Win32 Release Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Tactica2"
# PROP BASE Intermediate_Dir "Tactica2"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release Demo"
# PROP Intermediate_Dir "Release Demo"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "JA2" /D "RELEASE_WITH_DEBUG_INFO" /Fr /YX /FD /c
# ADD CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\laptop" /D "RELEASE_WITH_DEBUG_INFO" /D "NDEBUG" /D "JA2DEMO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /Fr /YX"AI All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TacticalAI - Win32 Demo Release with Debug Info"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Tactica3"
# PROP BASE Intermediate_Dir "Tactica3"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Demo Release with Debug"
# PROP Intermediate_Dir "Demo Release with Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "JA2" /D "RELEASE_WITH_DEBUG_INFO" /Fr /YX /FD /c
# ADD CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\laptop" /D "RELEASE_WITH_DEBUG_INFO" /D "NDEBUG" /D "JA2DEMO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /Fr /YX"AI All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TacticalAI - Win32 Demo Bounds Checker"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Tactica4"
# PROP BASE Intermediate_Dir "Tactica4"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Demo Bounds Checker"
# PROP Intermediate_Dir "Demo Bounds Checker"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "BOUNDS_CHECKER" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\Tactical" /I "\ja2\Build\Utils" /I "\ja2\Build\TileEngine" /I "\ja2\build\strategic" /I "\ja2\build\laptop" /D "_DEBUG" /D "BOUNDS_CHECKER" /D "JA2DEMO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"AI All.h" /FD /c
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

# Name "TacticalAI - Win32 Release"
# Name "TacticalAI - Win32 Debug"
# Name "TacticalAI - Win32 Release with Debug Info"
# Name "TacticalAI - Win32 Bounds Checker"
# Name "TacticalAI - Win32 Debug Demo"
# Name "TacticalAI - Win32 Release Demo"
# Name "TacticalAI - Win32 Demo Release with Debug Info"
# Name "TacticalAI - Win32 Demo Bounds Checker"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\AIList.c
# End Source File
# Begin Source File

SOURCE=.\AIMain.c
# End Source File
# Begin Source File

SOURCE=.\AIUtils.c
# End Source File
# Begin Source File

SOURCE=.\Attacks.c
# End Source File
# Begin Source File

SOURCE=.\CreatureDecideAction.c
# End Source File
# Begin Source File

SOURCE=.\DecideAction.c
# End Source File
# Begin Source File

SOURCE=.\FindLocations.c
# End Source File
# Begin Source File

SOURCE=.\Knowledge.c
# End Source File
# Begin Source File

SOURCE=.\Medical.c
# End Source File
# Begin Source File

SOURCE=.\Movement.c
# End Source File
# Begin Source File

SOURCE=.\NPC.c
# End Source File
# Begin Source File

SOURCE=.\PanicButtons.c
# End Source File
# Begin Source File

SOURCE=.\QuestDebug.c
# End Source File
# Begin Source File

SOURCE=.\Realtime.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=".\AI All.h"
# End Source File
# Begin Source File

SOURCE=.\ai.h
# End Source File
# Begin Source File

SOURCE=.\AIInternals.h
# End Source File
# Begin Source File

SOURCE=.\AIList.h
# End Source File
# Begin Source File

SOURCE=.\NPC.h
# End Source File
# Begin Source File

SOURCE=.\QuestDebug.h
# End Source File
# Begin Source File

SOURCE=.\Realtime.h
# End Source File
# End Group
# End Target
# End Project
