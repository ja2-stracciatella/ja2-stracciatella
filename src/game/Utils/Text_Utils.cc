#include "Directories.h"
#include "Text.h"
#include "Text_Utils.h"
#include "FileMan.h"
#include "GameSettings.h"

#include "src/game/GameRes.h"
#include "ContentManager.h"
#include "GameInstance.h"

#define ITEMSTRINGFILENAME BINARYDATADIR "/itemdesc.edt"


void LoadItemInfo(UINT16 const ubIndex, wchar_t Info[])
{
	UINT32 Seek = (SIZE_SHORT_ITEM_NAME + SIZE_ITEM_NAME + SIZE_ITEM_INFO) * ubIndex;
	GCM->loadEncryptedString(ITEMSTRINGFILENAME, Info, Seek + SIZE_ITEM_NAME + SIZE_SHORT_ITEM_NAME, SIZE_ITEM_INFO);
}


static void LoadAllItemNames(void)
{
	AutoSGPFile File(GCM->openGameResForReading(ITEMSTRINGFILENAME));
	for (UINT32 i = 0; i < MAXITEMS; i++)
	{
		UINT32 Seek = (SIZE_SHORT_ITEM_NAME + SIZE_ITEM_NAME + SIZE_ITEM_INFO) * i;
		GCM->loadEncryptedString(File, ShortItemNames[i], Seek, SIZE_SHORT_ITEM_NAME);
		GCM->loadEncryptedString(File, ItemNames[i], Seek + SIZE_SHORT_ITEM_NAME, SIZE_ITEM_NAME);
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
