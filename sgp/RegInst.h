//**************************************************************************
//
// Filename :	RegInst.h
//
//	Purpose :	prototypes for the registry stuff
//
// Modification history :
//
//		02dec96:HJH				- Creation
//
//**************************************************************************

#ifndef _RegInst_h
#define _RegInst_h

//**************************************************************************
//
//				Includes
//
//**************************************************************************

#include <assert.h>

#include "Types.h"

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
//				Function Prototypes
//
//**************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

// call once per execution of application:
extern BOOLEAN InitializeRegistryKeys(STR strAppName, STR strRegistryKey);

// returns key for HKEY_CURRENT_USER\"Software"\RegistryKey\ProfileName
// creating it if it doesn't exist
// responsibility of the caller to call RegCloseKey() on the returned HKEY
extern HKEY GetAppRegistryKey();

// returns key for:
//      HKEY_CURRENT_USER\"Software"\RegistryKey\AppName\lpszSection
// creating it if it doesn't exist.
// responsibility of the caller to call RegCloseKey() on the returned HKEY
extern HKEY GetSectionKey(STR lpszSection);

extern UINT GetProfileInteger(STR lpszSection, STR lpszEntry, int nDefault);
extern BOOLEAN GetProfileChar(STR lpszSection, STR lpszEntry,	STR lpszDefault, STR lpszValue);
extern BOOL GetProfileBinary(STR lpszSection, STR lpszEntry, BYTE** ppData, UINT* pBytes);
extern BOOL WriteProfileInt(STR lpszSection, STR lpszEntry, int nValue);
extern BOOL WriteProfileChar(STR lpszSection, STR lpszEntry, STR lpszValue);
extern BOOL WriteProfileBinary(STR lpszSection, STR lpszEntry, LPBYTE pData, UINT nBytes);

#ifdef __cplusplus
}
#endif

#endif
