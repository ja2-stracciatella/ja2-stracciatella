#include "Language_Defines.h"
#include "Text.h"
#include "FileMan.h"
#include "JA2_Demo_Ads.h"
#include "GameSettings.h"
#include "Encrypted_File.h"


BOOLEAN LoadItemInfo(UINT16 ubIndex, STR16 pNameString, STR16 pInfoString )
{
	UINT32 uiStartSeekAmount = (SIZE_ITEM_NAME + SIZE_SHORT_ITEM_NAME + SIZE_ITEM_INFO) * ubIndex;
	return
		LoadEncryptedDataFromFile(ITEMSTRINGFILENAME, pNameString, uiStartSeekAmount,  SIZE_ITEM_NAME) &&
		(
			pInfoString == NULL ||
			LoadEncryptedDataFromFile(ITEMSTRINGFILENAME, pInfoString, uiStartSeekAmount + SIZE_ITEM_NAME + SIZE_SHORT_ITEM_NAME, SIZE_ITEM_INFO)
		);
}

BOOLEAN LoadShortNameItemInfo(UINT16 ubIndex, STR16 pNameString )
{
	return LoadEncryptedDataFromFile(ITEMSTRINGFILENAME, pNameString, (SIZE_SHORT_ITEM_NAME + SIZE_ITEM_NAME + SIZE_ITEM_INFO) * ubIndex, SIZE_ITEM_NAME);
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
