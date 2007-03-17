#include "Encrypted_File.h"
#include "FileMan.h"
#include "Debug.h"
#include "Language_Defines.h"


BOOLEAN LoadEncryptedDataFromFile(const char *pFileName, STR16 pDestString, UINT32 uiSeekFrom, UINT32 uiSeekAmount)
{
	UINT16 str[uiSeekAmount / 2];
	HWFILE		hFile;
	UINT16		i;

	hFile = FileOpen(pFileName, FILE_ACCESS_READ);
	if ( !hFile )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "LoadEncryptedDataFromFile: Failed to FileOpen");
		return( FALSE );
	}

	if ( FileSeek( hFile, uiSeekFrom, FILE_SEEK_FROM_START ) == FALSE )
	{
		FileClose(hFile);
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "LoadEncryptedDataFromFile: Failed FileSeek");
		return( FALSE );
	}

	if (!FileRead(hFile, str, sizeof(str)))
	{
		FileClose(hFile);
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "LoadEncryptedDataFromFile: Failed FileRead");
		return( FALSE );
	}

	// Decrement, by 1, any value > 32
	for (i = 0; i < uiSeekAmount / 2 - 1 && str[i] != 0; i++)
	{
		pDestString[i] = (str[i] > 33 ? str[i] - 1 : str[i]);
		#ifdef POLISH
			switch( pDestString[ i ] )
			{
				case 260:		pDestString[ i ] = 165;		break;
				case 262:		pDestString[ i ] = 198;		break;
				case 280:		pDestString[ i ] = 202;		break;
				case 321:		pDestString[ i ] = 163;		break;
				case 323:		pDestString[ i ] = 209;		break;
				case 211:		pDestString[ i ] = 211;		break;

				case 346:		pDestString[ i ] = 338;		break;
				case 379:		pDestString[ i ] = 175;		break;
				case 377:		pDestString[ i ] = 143;		break;
				case 261:		pDestString[ i ] = 185;		break;
				case 263:		pDestString[ i ] = 230;		break;
				case 281:		pDestString[ i ] = 234;		break;

				case 322:		pDestString[ i ] = 179;		break;
				case 324:		pDestString[ i ] = 241;		break;
				case 243:		pDestString[ i ] = 243;		break;
				case 347:		pDestString[ i ] = 339;		break;
				case 380:		pDestString[ i ] = 191;		break;
				case 378:		pDestString[ i ] = 376;		break;
			}
		#endif
	}
	pDestString[i] = L'\0';

	FileClose(hFile);
	return(TRUE);
}
