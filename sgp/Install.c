//**************************************************************************
//
// Filename :	Install.c
//
//	Purpose :	install routines
//
// Modification history :
//
//		02dec96:HJH				- Creation
//
//**************************************************************************

//**************************************************************************
//
//				Includes
//
//**************************************************************************

#ifdef JA2_PRECOMPILED_HEADERS
	#include "JA2 SGP ALL.H"
#elif defined( WIZ8_PRECOMPILED_HEADERS )
	#include "WIZ8 SGP ALL.H"
#else
	#include "types.h"
	#include <windows.h>
	#include <tchar.h>
	#include <assert.h>
	#include "Install.h"
	#include "RegInst.h"
#endif

//**************************************************************************
//
//				Defines
//
//**************************************************************************

//**************************************************************************
//
//				Typedefs
//
//**************************************************************************

//**************************************************************************
//
//				Functions
//
//**************************************************************************

BOOLEAN	InstallApplication( STR strAppname, STR strPath )
{
	HKEY	hKey;
	BOOL  fRet = TRUE;

	hKey = GetAppRegistryKey();
	RegCloseKey( hKey );

	//hKeySection = GetSectionKey("Startup");
	//RegCloseKey( hKeySection );

	fRet = fRet && WriteProfileChar( "Startup", "InstPath", strPath );

	return(fRet);
}
