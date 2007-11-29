#include "LoadSaveLightEffect.h"
#include "WCheck.h"
#include "Debug.h"
#include "Soldier_Control.h"
#include "Weapons.h"
#include "Handle_Items.h"
#include "WorldDef.h"
#include "Animation_Control.h"
#include "Tile_Animation.h"
#include "LightEffects.h"
#include "Isometric_Utils.h"
#include "Explosion_Control.h"
#include "Random.h"
#include "Lighting.h"
#include "Game_Clock.h"
#include "OppList.h"
#include "Tactical_Save.h"
#include "Campaign_Types.h"
#include "FileMan.h"
#include "SaveLoadGame.h"


#define		NUM_LIGHT_EFFECT_SLOTS					25


// GLOBAL FOR LIGHT LISTING
static LIGHTEFFECT gLightEffectData[NUM_LIGHT_EFFECT_SLOTS];
static UINT32      guiNumLightEffects = 0;


#define BASE_FOR_ALL_LIGHTEFFECTS(type, iter)                           \
	for (type*       iter        = gLightEffectData,                      \
	         * const iter##__end = gLightEffectData + guiNumLightEffects; \
	     iter != iter##__end;                                             \
	     ++iter)                                                          \
		if (!iter->fAllocated) continue; else
#define FOR_ALL_LIGHTEFFECTS(iter)  BASE_FOR_ALL_LIGHTEFFECTS(      LIGHTEFFECT, iter)
#define CFOR_ALL_LIGHTEFFECTS(iter) BASE_FOR_ALL_LIGHTEFFECTS(const LIGHTEFFECT, iter)


static LIGHTEFFECT* GetFreeLightEffect(void)
{
	for (LIGHTEFFECT* l = gLightEffectData; l != gLightEffectData + guiNumLightEffects; ++l)
	{
		if (!l->fAllocated) return l;
	}
	if (guiNumLightEffects < NUM_LIGHT_EFFECT_SLOTS)
	{
		return &gLightEffectData[guiNumLightEffects++];
	}
	return NULL;
}


static void UpdateLightingSprite(LIGHTEFFECT* pLight)
{
	CHAR8 LightName[20];
	// Build light....

	sprintf( LightName, "Light%d", pLight->bRadius );

	// Delete old one if one exists...
	if (pLight->light != NULL)
	{
		LightSpriteDestroy(pLight->light);
		pLight->light = NULL;
	}

	// Effect light.....
	LIGHT_SPRITE* const l = LightSpriteCreate(LightName, 0);
	pLight->light = l;
	if (l == NULL)
	{
		// Could not light!
		return;
	}

	LightSpritePower(l, TRUE);
	LightSpritePosition(l, CenterX(pLight->sGridNo) / CELL_X_SIZE, CenterY(pLight->sGridNo) / CELL_Y_SIZE);
}


LIGHTEFFECT* NewLightEffect(const INT16 sGridNo, const INT8 bType)
{
	UINT8				ubDuration=0;
	UINT8				ubStartRadius=0;

	LIGHTEFFECT* const l = GetFreeLightEffect();
	if (l == NULL) return NULL;

	memset(l, 0, sizeof(*l));

	// Set some values...
	l->sGridNo            = sGridNo;
	l->bType              = bType;
	l->light              = NULL;
	l->uiTimeOfLastUpdate = GetWorldTotalSeconds();

  switch( bType )
  {
		case LIGHT_FLARE_MARK_1:

			ubDuration				= 6;
			ubStartRadius			= 6;
			break;

  }

	l->ubDuration = ubDuration;
	l->bRadius    = ubStartRadius;
	l->bAge       = 0;
	l->fAllocated = TRUE;

	UpdateLightingSprite(l);

  // Handle sight here....
	AllTeamsLookForAll( FALSE );

	return l;
}


void DecayLightEffects( UINT32 uiTime )
{
  // age all active tear gas clouds, deactivate those that are just dispersing
	FOR_ALL_LIGHTEFFECTS(l)
  {
		// ATE: Do this every so ofte, to acheive the effect we want...
		if (uiTime - l->uiTimeOfLastUpdate <= 350) continue;

		const UINT16 usNumUpdates = (uiTime - l->uiTimeOfLastUpdate) / 350;

		l->uiTimeOfLastUpdate = uiTime;

		BOOLEAN fDelete = FALSE;
		for (UINT32 i = 0; i < usNumUpdates; ++i)
		{
			l->bAge++;

			// if this cloud remains effective (duration not reached)
			if (l->bAge >= l->ubDuration)
			{
				fDelete = TRUE;
				break;
			}

			// calculate the new cloud radius
			// cloud expands by 1 every turn outdoors, and every other turn indoors
			if (l->bAge % 2) l->bRadius--;

			if (l->bRadius == 0)
			{
				fDelete = TRUE;
				break;
			}

			UpdateLightingSprite(l);
		}

		if (fDelete)
		{
			l->fAllocated = FALSE;
			if (l->light != NULL) LightSpriteDestroy(l->light);
		}

		// Handle sight here....
		AllTeamsLookForAll(FALSE);
  }
}



BOOLEAN SaveLightEffectsToSaveGameFile( HWFILE hFile )
{
	/*
	UINT32	uiNumberOfLights=0;

	//loop through and count the number of active slots
	CFOR_ALL_LIGHTEFFECTS(l)
	{
		++uiNumberOfLights;
	}

	//Save the Number of Light Effects
	if (!FileWrite(hFile, &uiNumberOfLights, sizeof(UINT32))) return FALSE;

	//if there are lights to save
	if( uiNumberOfLights != 0 )
	{
		//loop through and save each active slot
		CFOR_ALL_LIGHTEFFECTS(l)
		{
			if (!InjectLightEffectIntoFile(hFile, l)) return FALSE;
		}
	}
*/
	return( TRUE );
}


BOOLEAN LoadLightEffectsFromLoadGameFile( HWFILE hFile )
{
	//no longer need to load Light effects.  They are now in temp files
	if( guiSaveGameVersion < 76 )
	{
		memset( gLightEffectData, 0, sizeof( LIGHTEFFECT ) *  NUM_LIGHT_EFFECT_SLOTS );

		//Load the Number of Light Effects
		if (!FileRead(hFile, &guiNumLightEffects, sizeof(UINT32))) return FALSE;

		//if there are lights saved.
		if( guiNumLightEffects != 0 )
		{
			//loop through and apply the light effects to the map
			for (UINT32 uiCount = 0; uiCount < guiNumLightEffects; ++uiCount)
			{
				if (!ExtractLightEffectFromFile(hFile, &gLightEffectData[uiCount])) return FALSE;
			}
		}

		//loop through and apply the light effects to the map
		FOR_ALL_LIGHTEFFECTS(l)
		{
			UpdateLightingSprite(l);
		}
	}

	return( TRUE );
}



BOOLEAN SaveLightEffectsToMapTempFile( INT16 sMapX, INT16 sMapY, INT8 bMapZ )
{
	UINT32	uiNumLightEffects=0;
	HWFILE	hFile;
	CHAR8		zMapName[ 128 ];

	//get the name of the map
	GetMapTempFileName( SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS, zMapName, sMapX, sMapY, bMapZ );

	//delete file the file.
	FileDelete( zMapName );

	//loop through and count the number of Light effects
	CFOR_ALL_LIGHTEFFECTS(l)
	{
		++uiNumLightEffects;
	}

	//if there are no Light effects
	if( uiNumLightEffects == 0 )
	{
		//set the fact that there are no Light effects for this sector
		ReSetSectorFlag( sMapX, sMapY, bMapZ, SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS );

		return( TRUE );
	}

	//Open the file for writing
	hFile = FileOpen(zMapName, FILE_ACCESS_WRITE | FILE_OPEN_ALWAYS);
	if( hFile == 0 )
	{
		//Error opening map modification file
		return( FALSE );
	}


	//Save the Number of Light Effects
	if (!FileWrite(hFile, &uiNumLightEffects, sizeof(UINT32)))
	{
		//Close the file
		FileClose( hFile );

		return( FALSE );
	}

	//loop through and save the number of Light effects
	CFOR_ALL_LIGHTEFFECTS(l)
	{
		if (!InjectLightEffectIntoFile(hFile, l))
		{
			FileClose(hFile);
			return FALSE;
		}
	}

	//Close the file
	FileClose( hFile );

	SetSectorFlag( sMapX, sMapY, bMapZ, SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS );

	return( TRUE );
}



BOOLEAN LoadLightEffectsFromMapTempFile( INT16 sMapX, INT16 sMapY, INT8 bMapZ )
{
	UINT32	uiCnt=0;
	HWFILE	hFile;
	CHAR8		zMapName[ 128 ];

	GetMapTempFileName( SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS, zMapName, sMapX, sMapY, bMapZ );

	//Open the file for reading, Create it if it doesnt exist
	hFile = FileOpen(zMapName, FILE_ACCESS_READ | FILE_OPEN_EXISTING);
	if( hFile == 0 )
	{
		//Error opening file
		return( FALSE );
	}

	//Clear out the old list
	ResetLightEffects();


	//Load the Number of Light Effects
	if (!FileRead(hFile, &guiNumLightEffects, sizeof(UINT32)))
	{
		FileClose( hFile );
		return( FALSE );
	}

	//loop through and load the list
	for( uiCnt=0; uiCnt<guiNumLightEffects;uiCnt++)
	{
		if (!ExtractLightEffectFromFile(hFile, &gLightEffectData[uiCnt]))
		{
			FileClose( hFile );
			return( FALSE );
		}
	}

	//loop through and apply the light effects to the map
	FOR_ALL_LIGHTEFFECTS(l)
	{
		UpdateLightingSprite(l);
	}

	FileClose( hFile );

	return( TRUE );
}


void ResetLightEffects()
{
	//Clear out the old list
	memset( gLightEffectData, 0, sizeof( LIGHTEFFECT ) * NUM_LIGHT_EFFECT_SLOTS );
	guiNumLightEffects = 0;
}
