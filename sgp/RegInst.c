//**************************************************************************
//
// Filename :	RegInst.c
//
//	Purpose :	registry routines
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
	#include "RegInst.h"
	#include "WCheck.h"
#endif

//**************************************************************************
//
//				Defines
//
//**************************************************************************

#define REG_KEY_SIZE 50

//**************************************************************************
//
//				Variables
//
//**************************************************************************

// INI strings are not localized
static const TCHAR szSoftware[] = _T("Software");

static CHAR gszRegistryKey[REG_KEY_SIZE];
static CHAR gszAppName[REG_KEY_SIZE];
static CHAR gszProfileName[REG_KEY_SIZE];

//**************************************************************************
//
//				Functions
//
//**************************************************************************

BOOLEAN InitializeRegistryKeys(STR lpszAppName, STR lpszRegistryKey)
{
	CHECKF(lpszAppName != NULL);
	CHECKF(lpszRegistryKey != NULL);
	//CHECKF(gpszRegistryKey == NULL);
	//CHECKF(gpszAppName == NULL);
	//CHECKF(gpszProfileName == NULL);

	// Note: this will leak the original gpszProfileName, but it
	//  will be freed when the application exits.  No assumptions
	//  can be made on how gpszProfileName was allocated.

	strcpy( gszAppName, lpszAppName);
	strcpy( gszRegistryKey, lpszRegistryKey );
	strcpy( gszProfileName, gszAppName);

	return(TRUE);
}

// returns key for HKEY_CURRENT_USER\"Software"\RegistryKey\ProfileName
// creating it if it doesn't exist
// responsibility of the caller to call RegCloseKey() on the returned HKEY
HKEY GetAppRegistryKey()
{
	HKEY hAppKey = NULL;
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;

	assert(gszRegistryKey[0] != '\0');
	//assert(gpszProfileName != NULL);

	if (RegOpenKeyEx(HKEY_CURRENT_USER, szSoftware, 0, KEY_WRITE|KEY_READ,
		&hSoftKey) == ERROR_SUCCESS)
	{
		DWORD dw;
		if (RegCreateKeyEx(hSoftKey, gszRegistryKey, 0, REG_NONE,
			REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
			&hCompanyKey, &dw) == ERROR_SUCCESS)
		{
			RegCreateKeyEx(hCompanyKey, gszProfileName, 0, REG_NONE,
				REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
				&hAppKey, &dw);
		}
	}
	if (hSoftKey != NULL)
		RegCloseKey(hSoftKey);
	if (hCompanyKey != NULL)
		RegCloseKey(hCompanyKey);

	return hAppKey;
}

// returns key for:
//      HKEY_CURRENT_USER\"Software"\RegistryKey\AppName\lpszSection
// creating it if it doesn't exist.
// responsibility of the caller to call RegCloseKey() on the returned HKEY
HKEY GetSectionKey(STR lpszSection)
{
	HKEY hSectionKey = NULL;
	HKEY hAppKey = GetAppRegistryKey();
	DWORD dw;

	assert(lpszSection != NULL);

	if (hAppKey == NULL)
		return NULL;

	RegCreateKeyEx(hAppKey, lpszSection, 0, REG_NONE,
		REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
		&hSectionKey, &dw);
	RegCloseKey(hAppKey);
	return hSectionKey;
}

UINT GetProfileInteger(STR lpszSection, STR lpszEntry, int nDefault)
{
	DWORD dwValue;
	DWORD dwType;
	DWORD dwCount = sizeof(DWORD);
	LONG lResult;

	assert(lpszSection != NULL);
	assert(lpszEntry != NULL);

	if (gszRegistryKey[0] != '\0') // use registry
	{
		HKEY hSecKey = GetSectionKey(lpszSection);
		if (hSecKey == NULL)
			return nDefault;
		lResult = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
			(LPBYTE)&dwValue, &dwCount);
		RegCloseKey(hSecKey);
		if (lResult == ERROR_SUCCESS)
		{
			assert(dwType == REG_DWORD);
			assert(dwCount == sizeof(dwValue));
			return (UINT)dwValue;
		}
		return nDefault;
	}
	else
	{
		assert(gszProfileName[0] != '\0');
		return GetPrivateProfileInt(lpszSection, lpszEntry, nDefault,
			gszProfileName);
	}
}

BOOLEAN GetProfileChar(STR lpszSection, STR lpszEntry, STR lpszDefault, STR lpszValue)
{
	DWORD		dwType, dwCount;
	LONG		lResult;
	BOOLEAN	fRet = TRUE;
	CHAR		strValue[200];

	assert(lpszSection != NULL);
	assert(lpszEntry != NULL);
	assert(lpszDefault != NULL);

	if (gszRegistryKey[0] != '\0')
	{
		HKEY hSecKey = GetSectionKey(lpszSection);
		if (hSecKey == NULL)
		{
			strcpy( lpszValue, lpszDefault );
			return(TRUE);
		}
		lResult = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
			NULL, &dwCount);
		if (lResult == ERROR_SUCCESS)
		{
			assert(dwType == REG_SZ);
			lResult = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
				(LPBYTE)strValue, &dwCount);
		}
		RegCloseKey(hSecKey);
		if (lResult == ERROR_SUCCESS)
		{
			assert(dwType == REG_SZ);
			strcpy( lpszValue, strValue );
			return(TRUE);
		}
		strcpy( lpszValue, lpszDefault );
		return(TRUE);
	}
//	else
//	{
//		assert(gpszProfileName != NULL);
//
//		if (lpszDefault == NULL)
//			lpszDefault = &afxChNil;    // don't pass in NULL
//		TCHAR szT[4096];
//		DWORD dw = ::GetPrivateProfileString(lpszSection, lpszEntry,
//			lpszDefault, szT, _countof(szT), gpszProfileName);
//		assert(dw < 4095);
//		return szT;
//	}

	return( fRet );
}

BOOL GetProfileBinary(STR lpszSection, STR lpszEntry,
	BYTE** ppData, UINT* pBytes)
{
//	DWORD dwType, dwCount;
//	LONG lResult;
//
//	assert(lpszSection != NULL);
//	assert(lpszEntry != NULL);
//	assert(ppData != NULL);
//	assert(pBytes != NULL);
//	*ppData = NULL;
//	*pBytes = 0;
//
//	if (gpszRegistryKey != NULL)
//	{
//		LPBYTE lpByte = NULL;
//		HKEY hSecKey = GetSectionKey(lpszSection);
//		if (hSecKey == NULL)
//			return FALSE;
//
//		lResult = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
//			NULL, &dwCount);
//		*pBytes = dwCount;
//		if (lResult == ERROR_SUCCESS)
//		{
//			assert(dwType == REG_BINARY);
//			*ppData = new BYTE[*pBytes];
//			lResult = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
//				*ppData, &dwCount);
//		}
//		RegCloseKey(hSecKey);
//		if (lResult == ERROR_SUCCESS)
//		{
//			assert(dwType == REG_BINARY);
//			return TRUE;
//		}
//		else
//		{
//			delete [] *ppData;
//			*ppData = NULL;
//		}
//		return FALSE;
//	}
//	else
//	{
//		//assert(gpszProfileName != NULL);
//		//
//		//CString str = GetProfileString(lpszSection, lpszEntry, NULL);
//		//if (str.IsEmpty())
//		//	return FALSE;
//		//assert(str.GetLength()%2 == 0);
//		//int nLen = str.GetLength();
//		//*pBytes = nLen/2;
//		//*ppData = new BYTE[*pBytes];
//		//for (int i=0;i<nLen;i+=2)
//		//{
//		//	(*ppData)[i/2] = (BYTE)
//		//		(((str[i+1] - _T('A')) << 4) + (str[i] - _T('A')));
//		//}
//		return TRUE;
//	}
	return TRUE;
}

BOOL WriteProfileInt(STR lpszSection, STR lpszEntry, int nValue)
{
//	LONG lResult;
//	TCHAR szT[16];
//
//	assert(lpszSection != NULL);
//	assert(lpszEntry != NULL);
//
//	if (gpszRegistryKey != NULL)
//	{
//		HKEY hSecKey = GetSectionKey(lpszSection);
//		if (hSecKey == NULL)
//			return FALSE;
//		lResult = RegSetValueEx(hSecKey, lpszEntry, NULL, REG_DWORD,
//			(LPBYTE)&nValue, sizeof(nValue));
//		RegCloseKey(hSecKey);
//		return lResult == ERROR_SUCCESS;
//	}
//	else
//	{
//		assert(gpszProfileName != NULL);
//
//		wsprintf(szT, _T("%d"), nValue);
//		return ::WritePrivateProfileString(lpszSection, lpszEntry, szT,
//			gpszProfileName);
//	}
	return TRUE;
}

BOOL WriteProfileChar(STR lpszSection, STR lpszEntry, STR lpszValue)
{
	assert(lpszSection != NULL);

	if (gszRegistryKey[0] != '\0')
	{
		LONG lResult;
		if (lpszEntry == NULL) //delete whole section
		{
			HKEY hAppKey = GetAppRegistryKey();
			if (hAppKey == NULL)
				return FALSE;
			lResult = RegDeleteKey(hAppKey, lpszSection);
			RegCloseKey(hAppKey);
		}
		else if (lpszValue == NULL)
		{
			HKEY hSecKey = GetSectionKey(lpszSection);
			if (hSecKey == NULL)
				return FALSE;
			// necessary to cast away const below
			lResult = RegDeleteValue(hSecKey, (LPTSTR)lpszEntry);
			RegCloseKey(hSecKey);
		}
		else
		{
			HKEY hSecKey = GetSectionKey(lpszSection);
			if (hSecKey == NULL)
				return FALSE;
			lResult = RegSetValueEx(hSecKey, lpszEntry, 0, REG_SZ,
				(LPBYTE)lpszValue, (lstrlen(lpszValue)+1)*sizeof(TCHAR));
			RegCloseKey(hSecKey);
		}
		return lResult == ERROR_SUCCESS;
	}
//	else
//	{
//		assert(gpszProfileName != NULL);
//		assert(lstrlen(gpszProfileName) < 4095); // can't read in bigger
//		return ::WritePrivateProfileString(lpszSection, lpszEntry, lpszValue,
//			gpszProfileName);
//	}
	return TRUE;
}

BOOL WriteProfileBinary(STR lpszSection, STR lpszEntry, LPBYTE pData, UINT nBytes)
{
//	assert(lpszSection != NULL);
//
//	if (gpszRegistryKey != NULL)
//	{
//		LONG lResult;
//		HKEY hSecKey = GetSectionKey(lpszSection);
//		if (hSecKey == NULL)
//			return FALSE;
//		lResult = RegSetValueEx(hSecKey, lpszEntry, NULL, REG_BINARY,
//			pData, nBytes);
//		RegCloseKey(hSecKey);
//		return lResult == ERROR_SUCCESS;
//	}
//
//	// convert to string and write out
//	LPTSTR lpsz = new TCHAR[nBytes*2+1];
//	for (UINT i = 0; i < nBytes; i++)
//	{
//		lpsz[i*2] = (TCHAR)((pData[i] & 0x0F) + _T('A')); //low nibble
//		lpsz[i*2+1] = (TCHAR)(((pData[i] >> 4) & 0x0F) + _T('A')); //high nibble
//	}
//	lpsz[i*2] = 0;
//
//	assert(gpszProfileName != NULL);
//
//	BOOL bResult = WriteProfileString(lpszSection, lpszEntry, lpsz);
//	delete[] lpsz;
//	return bResult;
	return TRUE;
}
