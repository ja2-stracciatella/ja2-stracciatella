#ifdef PRECOMPILEDHEADERS
#else
	#include "Language_Defines.h"
	#include "Text.h"
	#include "FileMan.h"
	#include "JA2_Demo_Ads.h"
	#include "GameSettings.h"
#endif

BOOLEAN LoadItemInfo(UINT16 ubIndex, STR16 pNameString, STR16 pInfoString )
{
	UINT16 str_name[SIZE_ITEM_NAME / 2];
	HWFILE		hFile;
	UINT32		uiBytesRead;
	UINT16		i;
	UINT32		uiStartSeekAmount;

	hFile = FileOpen(ITEMSTRINGFILENAME, FILE_ACCESS_READ, FALSE);
	if ( !hFile )
	{
		return( FALSE );
	}

	// Get current mercs bio info
	uiStartSeekAmount = ( ( SIZE_SHORT_ITEM_NAME + SIZE_ITEM_NAME + SIZE_ITEM_INFO) * ubIndex );

	// Skip short names
	uiStartSeekAmount += SIZE_SHORT_ITEM_NAME;

	if ( FileSeek( hFile, uiStartSeekAmount, FILE_SEEK_FROM_START ) == FALSE )
	{
		FileClose(hFile);
		return( FALSE );
	}

	if (!FileRead(hFile, str_name, sizeof(str_name), &uiBytesRead))
	{
		FileClose(hFile);
		return( FALSE );
	}

	// Decrement, by 1, any value > 32
	for (i = 0; i < SIZE_ITEM_NAME / 2 - 1 && str_name[i] != 0; i++ )
	{
		pNameString[i] = (str_name[i] > 33 ? str_name[i] - 1 : str_name[i]);
		#ifdef POLISH
			switch( pNameString[ i ] )
			{
				case 260:		pNameString[ i ] = 165;		break;
				case 262:		pNameString[ i ] = 198;		break;
				case 280:		pNameString[ i ] = 202;		break;
				case 321:		pNameString[ i ] = 163;		break;
				case 323:		pNameString[ i ] = 209;		break;
				case 211:		pNameString[ i ] = 211;		break;

				case 346:		pNameString[ i ] = 338;		break;
				case 379:		pNameString[ i ] = 175;		break;
				case 377:		pNameString[ i ] = 143;		break;
				case 261:		pNameString[ i ] = 185;		break;
				case 263:		pNameString[ i ] = 230;		break;
				case 281:		pNameString[ i ] = 234;		break;

				case 322:		pNameString[ i ] = 179;		break;
				case 324:		pNameString[ i ] = 241;		break;
				case 243:		pNameString[ i ] = 243;		break;
				case 347:		pNameString[ i ] = 339;		break;
				case 380:		pNameString[ i ] = 191;		break;
				case 378:		pNameString[ i ] = 376;		break;
			}
		#endif
	}
	pNameString[i] = L'\0';

	// condition added by Chris - so we can get the name without the item info
	// when desired, by passing in a null pInfoString

	if (pInfoString != NULL)
	{
		UINT16 str_info[SIZE_ITEM_INFO / 2];

		// Get the additional info
		uiStartSeekAmount = ((SIZE_ITEM_NAME + SIZE_SHORT_ITEM_NAME + SIZE_ITEM_INFO) * ubIndex ) + SIZE_ITEM_NAME + SIZE_SHORT_ITEM_NAME;
		if ( FileSeek( hFile, uiStartSeekAmount, FILE_SEEK_FROM_START ) == FALSE )
		{
			FileClose(hFile);
			return( FALSE );
		}

		if (!FileRead(hFile, str_info, sizeof(str_info), &uiBytesRead))
		{
			FileClose(hFile);
			return( FALSE );
		}

		// Decrement, by 1, any value > 32
		for (i = 0; i < SIZE_ITEM_INFO / 2 - 1 && str_info[i] != 0; i++ )
		{
			pInfoString[i] = (str_info[i] > 33 ? str_info[i] - 1 : str_info[i]);
			#ifdef POLISH
				switch( pInfoString[ i ] )
				{
					case 260:		pInfoString[ i ] = 165;		break;
					case 262:		pInfoString[ i ] = 198;		break;
					case 280:		pInfoString[ i ] = 202;		break;
					case 321:		pInfoString[ i ] = 163;		break;
					case 323:		pInfoString[ i ] = 209;		break;
					case 211:		pInfoString[ i ] = 211;		break;

					case 346:		pInfoString[ i ] = 338;		break;
					case 379:		pInfoString[ i ] = 175;		break;
					case 377:		pInfoString[ i ] = 143;		break;
					case 261:		pInfoString[ i ] = 185;		break;
					case 263:		pInfoString[ i ] = 230;		break;
					case 281:		pInfoString[ i ] = 234;		break;

					case 322:		pInfoString[ i ] = 179;		break;
					case 324:		pInfoString[ i ] = 241;		break;
					case 243:		pInfoString[ i ] = 243;		break;
					case 347:		pInfoString[ i ] = 339;		break;
					case 380:		pInfoString[ i ] = 191;		break;
					case 378:		pInfoString[ i ] = 376;		break;
				}
			#endif
		}
		pInfoString[i] = L'\0';
	}

	FileClose(hFile);
	return(TRUE);
}

BOOLEAN LoadShortNameItemInfo(UINT16 ubIndex, STR16 pNameString )
{
	UINT16 str[SIZE_ITEM_NAME / 2];
	HWFILE		hFile;
//  wchar_t		DestString[ SIZE_MERC_BIO_INFO ];
	UINT32		uiBytesRead;
	UINT16		i;
	UINT32		uiStartSeekAmount;

	hFile = FileOpen(ITEMSTRINGFILENAME, FILE_ACCESS_READ, FALSE);
	if ( !hFile )
	{
		return( FALSE );
	}


	// Get current mercs bio info
	uiStartSeekAmount = ( ( SIZE_SHORT_ITEM_NAME + SIZE_ITEM_NAME + SIZE_ITEM_INFO ) * ubIndex );

	if ( FileSeek( hFile, uiStartSeekAmount, FILE_SEEK_FROM_START ) == FALSE )
	{
		FileClose(hFile);
		return( FALSE );
	}

	if (!FileRead(hFile, str, sizeof(str), &uiBytesRead))
	{
		FileClose(hFile);
		return( FALSE );
	}

	// Decrement, by 1, any value > 32
	for (i = 0; i < SIZE_ITEM_NAME / 2 - 1 && str[i] != 0; i++ )
	{
		pNameString[i] = (str[i] > 33 ? str[i] - 1 : str[i]);
		#ifdef POLISH
			switch( pNameString[ i ] )
			{
				case 260:		pNameString[ i ] = 165;		break;
				case 262:		pNameString[ i ] = 198;		break;
				case 280:		pNameString[ i ] = 202;		break;
				case 321:		pNameString[ i ] = 163;		break;
				case 323:		pNameString[ i ] = 209;		break;
				case 211:		pNameString[ i ] = 211;		break;

				case 346:		pNameString[ i ] = 338;		break;
				case 379:		pNameString[ i ] = 175;		break;
				case 377:		pNameString[ i ] = 143;		break;
				case 261:		pNameString[ i ] = 185;		break;
				case 263:		pNameString[ i ] = 230;		break;
				case 281:		pNameString[ i ] = 234;		break;

				case 322:		pNameString[ i ] = 179;		break;
				case 324:		pNameString[ i ] = 241;		break;
				case 243:		pNameString[ i ] = 243;		break;
				case 347:		pNameString[ i ] = 339;		break;
				case 380:		pNameString[ i ] = 191;		break;
				case 378:		pNameString[ i ] = 376;		break;
			}
		#endif
	}
	pNameString[i] = '\0';

	FileClose(hFile);
	return(TRUE);
}


void LoadAllItemNames( void )
{
	UINT16 usLoop;

	for (usLoop = 0; usLoop < MAXITEMS; usLoop++)
	{
		LoadItemInfo( usLoop, ItemNames[usLoop], NULL );

		// Load short item info
		LoadShortNameItemInfo( usLoop, ShortItemNames[usLoop] );

	}
}

void LoadAllExternalText( void )
{
	LoadAllItemNames();
}

const wchar_t* GetWeightUnitString( void )
{
	if ( gGameSettings.fOptions[ TOPTION_USE_METRIC_SYSTEM ] ) // metric
	{
		return( pMessageStrings[ MSG_KILOGRAM_ABBREVIATION ] );
	}
	else
	{
		return( pMessageStrings[ MSG_POUND_ABBREVIATION ] );
	}
}

FLOAT GetWeightBasedOnMetricOption( UINT32 uiObjectWeight )
{
	FLOAT fWeight = 0.0f;

	//if the user is smart and wants things displayed in 'metric'
	if ( gGameSettings.fOptions[ TOPTION_USE_METRIC_SYSTEM ] ) // metric
	{
		fWeight = (FLOAT)uiObjectWeight;
	}

	//else the user is a caveman and display it in pounds
	else
	{
		fWeight = uiObjectWeight * 2.2f;
	}

	return( fWeight );
}
