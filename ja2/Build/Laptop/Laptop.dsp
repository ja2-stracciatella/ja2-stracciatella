# Microsoft Developer Studio Project File - Name="Laptop" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Laptop - Win32 Demo Bounds Checker
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Laptop.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Laptop.mak" CFG="Laptop - Win32 Demo Bounds Checker"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Laptop - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Laptop - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Laptop - Win32 Release with Debug Info" (based on "Win32 (x86) Static Library")
!MESSAGE "Laptop - Win32 Bounds Checker" (based on "Win32 (x86) Static Library")
!MESSAGE "Laptop - Win32 Debug Demo" (based on "Win32 (x86) Static Library")
!MESSAGE "Laptop - Win32 Release Demo" (based on "Win32 (x86) Static Library")
!MESSAGE "Laptop - Win32 Demo Release with Debug Info" (based on "Win32 (x86) Static Library")
!MESSAGE "Laptop - Win32 Demo Bounds Checker" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Jagged Alliance 2/Development/Programming/Jagged Alliance 2/Build", AVAAAAAA"
# PROP Scc_LocalPath "..\..\..\ja2source\ja2\build"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Laptop - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Editor" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /YX"Laptop All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Laptop - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Laptop__"
# PROP BASE Intermediate_Dir "Laptop__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Editor" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"Laptop All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Laptop - Win32 Release with Debug Info"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Laptop_0"
# PROP BASE Intermediate_Dir "Laptop_0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release with Debug"
# PROP Intermediate_Dir "Release with Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Editor" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "JA2" /YX /FD /c
# ADD CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Editor" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "NDEBUG" /D "RELEASE_WITH_DEBUG_INFO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"Laptop All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Laptop - Win32 Bounds Checker"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Laptop_2"
# PROP BASE Intermediate_Dir "Laptop_2"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Bounds Checker"
# PROP Intermediate_Dir "Bounds Checker"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Editor" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "_DEBUG" /D "JA2" /D "WIN32" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Editor" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "_DEBUG" /D "BOUNDS_CHECKER" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /D "_VTUNE_PROFILING" /FR /YX"Laptop All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Laptop - Win32 Debug Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Laptop_1"
# PROP BASE Intermediate_Dir "Laptop_1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug Demo"
# PROP Intermediate_Dir "Debug Demo"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Editor" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "_DEBUG" /D "JA2" /D "WIN32" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Editor" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "_DEBUG" /D "JA2DEMO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"Laptop All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Laptop - Win32 Release Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Laptop_3"
# PROP BASE Intermediate_Dir "Laptop_3"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release Demo"
# PROP Intermediate_Dir "Release Demo"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Editor" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "JA2" /D "RELEASE_WITH_DEBUG_INFO" /FR /YX /FD /c
# ADD CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Editor" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "RELEASE_WITH_DEBUG_INFO" /D "NDEBUG" /D "JA2DEMO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"Laptop All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Laptop - Win32 Demo Release with Debug Info"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Laptop_4"
# PROP BASE Intermediate_Dir "Laptop_4"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Demo Release with Debug"
# PROP Intermediate_Dir "Demo Release with Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Editor" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "JA2" /D "RELEASE_WITH_DEBUG_INFO" /FR /YX /FD /c
# ADD CPP /nologo /MT /W4 /GX /Zi /O2 /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Editor" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "RELEASE_WITH_DEBUG_INFO" /D "NDEBUG" /D "JA2DEMO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"Laptop All.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Laptop - Win32 Demo Bounds Checker"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Laptop_5"
# PROP BASE Intermediate_Dir "Laptop_5"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Demo Bounds Checker"
# PROP Intermediate_Dir "Demo Bounds Checker"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Editor" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "BOUNDS_CHECKER" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "\Standard Gaming Platform" /I "\ja2\Build" /I "\ja2\Build\TileEngine" /I "\ja2\Build\Utils" /I "\ja2\build\TacticalAI" /I "\ja2\build\Communications" /I "\ja2\build\Editor" /I "\ja2\build\Tactical" /I "\ja2\build\strategic" /D "_DEBUG" /D "BOUNDS_CHECKER" /D "JA2DEMO" /D "WIN32" /D "_WINDOWS" /D "JA2" /D "PRECOMPILEDHEADERS" /FR /YX"Laptop All.h" /FD /c
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

# Name "Laptop - Win32 Release"
# Name "Laptop - Win32 Debug"
# Name "Laptop - Win32 Release with Debug Info"
# Name "Laptop - Win32 Bounds Checker"
# Name "Laptop - Win32 Debug Demo"
# Name "Laptop - Win32 Release Demo"
# Name "Laptop - Win32 Demo Release with Debug Info"
# Name "Laptop - Win32 Demo Bounds Checker"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\aim.c
# End Source File
# Begin Source File

SOURCE=.\AimArchives.c
# End Source File
# Begin Source File

SOURCE=.\AimFacialIndex.c
# End Source File
# Begin Source File

SOURCE=.\AimHistory.c
# End Source File
# Begin Source File

SOURCE=.\AimLinks.c
# End Source File
# Begin Source File

SOURCE=.\AimMembers.c
# End Source File
# Begin Source File

SOURCE=.\AimPolicies.c
# End Source File
# Begin Source File

SOURCE=.\AimSort.c
# End Source File
# Begin Source File

SOURCE=.\BobbyR.c
# End Source File
# Begin Source File

SOURCE=.\BobbyRAmmo.c
# End Source File
# Begin Source File

SOURCE=.\BobbyRArmour.c
# End Source File
# Begin Source File

SOURCE=.\BobbyRGuns.c
# End Source File
# Begin Source File

SOURCE=.\BobbyRMailOrder.c
# End Source File
# Begin Source File

SOURCE=.\BobbyRMisc.c
# End Source File
# Begin Source File

SOURCE=.\BobbyRShipments.c
# End Source File
# Begin Source File

SOURCE=.\BobbyRUsed.c
# End Source File
# Begin Source File

SOURCE=.\BrokenLink.c
# End Source File
# Begin Source File

SOURCE=.\CharProfile.c
# End Source File
# Begin Source File

SOURCE=.\email.c
# End Source File
# Begin Source File

SOURCE=.\files.c
# End Source File
# Begin Source File

SOURCE=.\finances.c
# End Source File
# Begin Source File

SOURCE=".\florist Cards.c"
# End Source File
# Begin Source File

SOURCE=".\florist Gallery.c"
# End Source File
# Begin Source File

SOURCE=".\florist Order Form.c"
# End Source File
# Begin Source File

SOURCE=.\florist.c
# End Source File
# Begin Source File

SOURCE=.\funeral.c
# End Source File
# Begin Source File

SOURCE=.\history.c
# End Source File
# Begin Source File

SOURCE=".\IMP AboutUs.c"
# End Source File
# Begin Source File

SOURCE=".\IMP Attribute Entrance.c"
# End Source File
# Begin Source File

SOURCE=".\IMP Attribute Finish.c"
# End Source File
# Begin Source File

SOURCE=".\IMP Attribute Selection.c"
# End Source File
# Begin Source File

SOURCE=".\IMP Begin Screen.c"
# End Source File
# Begin Source File

SOURCE=".\IMP Compile Character.c"
# End Source File
# Begin Source File

SOURCE=".\IMP Confirm.c"
# End Source File
# Begin Source File

SOURCE=".\IMP Finish.c"
# End Source File
# Begin Source File

SOURCE=".\IMP HomePage.c"
# End Source File
# Begin Source File

SOURCE=".\IMP MainPage.c"
# End Source File
# Begin Source File

SOURCE=".\IMP Personality Entrance.c"
# End Source File
# Begin Source File

SOURCE=".\IMP Personality Finish.c"
# End Source File
# Begin Source File

SOURCE=".\IMP Personality Quiz.c"
# End Source File
# Begin Source File

SOURCE=".\IMP Portraits.c"
# End Source File
# Begin Source File

SOURCE=".\IMP Text System.c"
# End Source File
# Begin Source File

SOURCE=".\IMP Voices.c"
# End Source File
# Begin Source File

SOURCE=.\IMPVideoObjects.c
# End Source File
# Begin Source File

SOURCE=".\insurance Comments.c"
# End Source File
# Begin Source File

SOURCE=".\insurance Contract.c"
# End Source File
# Begin Source File

SOURCE=".\insurance Info.c"
# End Source File
# Begin Source File

SOURCE=.\insurance.c
# End Source File
# Begin Source File

SOURCE=.\laptop.c
# End Source File
# Begin Source File

SOURCE=".\mercs Account.c"
# End Source File
# Begin Source File

SOURCE=".\mercs Files.c"
# End Source File
# Begin Source File

SOURCE=".\mercs No Account.c"
# End Source File
# Begin Source File

SOURCE=.\mercs.c
# End Source File
# Begin Source File

SOURCE=.\personnel.c

!IF  "$(CFG)" == "Laptop - Win32 Release"

!ELSEIF  "$(CFG)" == "Laptop - Win32 Debug"

!ELSEIF  "$(CFG)" == "Laptop - Win32 Release with Debug Info"

# ADD CPP /W4 /O2

!ELSEIF  "$(CFG)" == "Laptop - Win32 Bounds Checker"

!ELSEIF  "$(CFG)" == "Laptop - Win32 Debug Demo"

!ELSEIF  "$(CFG)" == "Laptop - Win32 Release Demo"

# ADD BASE CPP /W4 /O2
# ADD CPP /W4 /O2

!ELSEIF  "$(CFG)" == "Laptop - Win32 Demo Release with Debug Info"

# ADD BASE CPP /W4 /O2
# ADD CPP /W4 /O2

!ELSEIF  "$(CFG)" == "Laptop - Win32 Demo Bounds Checker"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sirtech.c
# End Source File
# Begin Source File

SOURCE=".\Store Inventory.c"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\aim.h
# End Source File
# Begin Source File

SOURCE=.\AimMembers.h
# End Source File
# Begin Source File

SOURCE=.\BobbyRAmmo.h
# End Source File
# Begin Source File

SOURCE=.\BobbyRArmour.h
# End Source File
# Begin Source File

SOURCE=.\BobbyRGuns.h
# End Source File
# Begin Source File

SOURCE=.\BobbyRMailOrder.h
# End Source File
# Begin Source File

SOURCE=.\BobbyRMisc.h
# End Source File
# Begin Source File

SOURCE=.\BobbyRShipments.h
# End Source File
# Begin Source File

SOURCE=.\BobbyRUsed.h
# End Source File
# Begin Source File

SOURCE=.\BrokenLink.h
# End Source File
# Begin Source File

SOURCE=.\CharProfile.h
# End Source File
# Begin Source File

SOURCE=.\email.h
# End Source File
# Begin Source File

SOURCE=.\files.h
# End Source File
# Begin Source File

SOURCE=.\finances.h
# End Source File
# Begin Source File

SOURCE=".\florist Cards.h"
# End Source File
# Begin Source File

SOURCE=".\florist Gallery.h"
# End Source File
# Begin Source File

SOURCE=".\florist Order Form.h"
# End Source File
# Begin Source File

SOURCE=.\florist.h
# End Source File
# Begin Source File

SOURCE=.\funeral.h
# End Source File
# Begin Source File

SOURCE=.\history.h
# End Source File
# Begin Source File

SOURCE=".\IMP AboutUs.h"
# End Source File
# Begin Source File

SOURCE=".\IMP Attribute Entrance.h"
# End Source File
# Begin Source File

SOURCE=".\IMP Attribute Finish.h"
# End Source File
# Begin Source File

SOURCE=".\IMP Attribute Selection.h"
# End Source File
# Begin Source File

SOURCE=".\IMP Begin Screen.h"
# End Source File
# Begin Source File

SOURCE=".\IMP Compile Character.h"
# End Source File
# Begin Source File

SOURCE=".\IMP Confirm.h"
# End Source File
# Begin Source File

SOURCE=".\IMP Finish.h"
# End Source File
# Begin Source File

SOURCE=".\IMP HomePage.h"
# End Source File
# Begin Source File

SOURCE=".\IMP MainPage.h"
# End Source File
# Begin Source File

SOURCE=".\IMP Personality Entrance.h"
# End Source File
# Begin Source File

SOURCE=".\IMP Personality Finish.h"
# End Source File
# Begin Source File

SOURCE=".\IMP Personality Quiz.h"
# End Source File
# Begin Source File

SOURCE=".\IMP Portraits.h"
# End Source File
# Begin Source File

SOURCE=".\IMP Text System.h"
# End Source File
# Begin Source File

SOURCE=".\IMP Voices.h"
# End Source File
# Begin Source File

SOURCE=.\IMPVideoObjects.h
# End Source File
# Begin Source File

SOURCE=".\insurance Comments.h"
# End Source File
# Begin Source File

SOURCE=".\insurance Contract.h"
# End Source File
# Begin Source File

SOURCE=".\insurance Info.h"
# End Source File
# Begin Source File

SOURCE=".\Insurance Text.h"
# End Source File
# Begin Source File

SOURCE=.\insurance.h
# End Source File
# Begin Source File

SOURCE=".\Laptop All.h"
# End Source File
# Begin Source File

SOURCE=.\laptop.h
# End Source File
# Begin Source File

SOURCE=.\LaptopSave.h
# End Source File
# Begin Source File

SOURCE=.\mercs.h
# End Source File
# Begin Source File

SOURCE=.\personnel.h
# End Source File
# Begin Source File

SOURCE=.\sirtech.h
# End Source File
# Begin Source File

SOURCE=".\Store Inventory.h"
# End Source File
# End Group
# End Target
# End Project
