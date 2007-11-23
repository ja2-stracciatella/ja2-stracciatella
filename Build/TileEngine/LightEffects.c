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
LIGHTEFFECT				gLightEffectData[ NUM_LIGHT_EFFECT_SLOTS ];
UINT32						guiNumLightEffects = 0;


static INT32 GetFreeLightEffect(void)
{
	UINT32 uiCount;

	for(uiCount=0; uiCount < guiNumLightEffects; uiCount++)
	{
		if(( gLightEffectData[uiCount].fAllocated==FALSE ) )
			return( (INT32)uiCount );
	}

	if( guiNumLightEffects < NUM_LIGHT_EFFECT_SLOTS )
		return( (INT32) guiNumLightEffects++ );

	return( -1 );
}


static void RecountLightEffects(void)
{
	INT32 uiCount;

	for(uiCount=guiNumLightEffects-1; (uiCount >=0) ; uiCount--)
	{
		if( ( gLightEffectData[uiCount].fAllocated ) )
		{
			guiNumLightEffects=(UINT32)(uiCount+1);
			break;
		}
	}
}


static void UpdateLightingSprite(LIGHTEFFECT* pLight)
{
	CHAR8 LightName[20];
	// Build light....

	sprintf( LightName, "Light%d", pLight->bRadius );

	// Delete old one if one exists...
	if( pLight->iLight!=(-1) )
	{
		LightSpriteDestroy(ID2LIGHT(pLight->iLight));
		pLight->iLight = -1;
	}

	// Effect light.....
	LIGHT_SPRITE* const l = LightSpriteCreate(LightName, 0);
	pLight->iLight = LIGHT2ID(l);
	if (l == NULL)
	{
		// Could not light!
		return;
	}

	LightSpritePower(l, TRUE);
	LightSpritePosition(l, CenterX(pLight->sGridNo) / CELL_X_SIZE, CenterY(pLight->sGridNo) / CELL_Y_SIZE);
}


INT32 NewLightEffect( INT16 sGridNo, INT8 bType )
{
	LIGHTEFFECT *pLight;
	INT32				iLightIndex;
	UINT8				ubDuration=0;
	UINT8				ubStartRadius=0;

	if( ( iLightIndex = GetFreeLightEffect() )==(-1) )
		return(-1);

	memset( &gLightEffectData[ iLightIndex ], 0, sizeof( LIGHTEFFECT ) );

	pLight = &gLightEffectData[ iLightIndex ];

	// Set some values...
	pLight->sGridNo									= sGridNo;
	pLight->bType										= bType;
	pLight->iLight									= -1;
	pLight->uiTimeOfLastUpdate			= GetWorldTotalSeconds( );

  switch( bType )
  {
		case LIGHT_FLARE_MARK_1:

			ubDuration				= 6;
			ubStartRadius			= 6;
			break;

  }

	pLight->ubDuration	= ubDuration;
	pLight->bRadius     = ubStartRadius;
	pLight->bAge				= 0;
	pLight->fAllocated  = TRUE;

	UpdateLightingSprite( pLight );

  // Handle sight here....
	AllTeamsLookForAll( FALSE );

	return( iLightIndex );
}


static void RemoveLightEffectFromTile(INT16 sGridNo)
{
	LIGHTEFFECT *pLight;
	UINT32 cnt;

	// Set to unallocated....
  for ( cnt = 0; cnt < guiNumLightEffects; cnt++ )
  {
		pLight = &gLightEffectData[ cnt ];

		if ( pLight->fAllocated )
		{
			if ( pLight->sGridNo == sGridNo )
			{
				pLight->fAllocated = FALSE;

				// Remove light....
				if( pLight->iLight != (-1) )
				{
					LightSpriteDestroy(ID2LIGHT(pLight->iLight));
				}
				break;
			}
		}
	}

}


void DecayLightEffects( UINT32 uiTime )
{
	LIGHTEFFECT *pLight;
	UINT32 cnt, cnt2;
	BOOLEAN	  fDelete = FALSE;
  UINT16    usNumUpdates = 1;

  // age all active tear gas clouds, deactivate those that are just dispersing
  for ( cnt = 0; cnt < guiNumLightEffects; cnt++ )
  {
		pLight = &gLightEffectData[ cnt ];

		fDelete = FALSE;

		if ( pLight->fAllocated )
		{
			// ATE: Do this every so ofte, to acheive the effect we want...
			if ( ( uiTime - pLight->uiTimeOfLastUpdate ) > 350 )
			{
        usNumUpdates = ( UINT16 ) ( ( uiTime - pLight->uiTimeOfLastUpdate ) / 350 );

				pLight->uiTimeOfLastUpdate = uiTime;

        for ( cnt2 = 0; cnt2 < usNumUpdates; cnt2++ )
        {
				  pLight->bAge++;

				  // if this cloud remains effective (duration not reached)
				  if ( pLight->bAge < pLight->ubDuration)
				  {
					  // calculate the new cloud radius
					  // cloud expands by 1 every turn outdoors, and every other turn indoors
					  if ( ( pLight->bAge % 2 ) )
					  {
						  pLight->bRadius--;
					  }

					  if ( pLight->bRadius == 0 )
					  {
						  // Delete...
						  fDelete = TRUE;
              break;
					  }
					  else
					  {
						  UpdateLightingSprite( pLight );
					  }
				  }
				  else
				  {
					  fDelete = TRUE;
            break;
				  }
        }

				if ( fDelete )
				{
					pLight->fAllocated = FALSE;

					if( pLight->iLight != (-1) )
					{
						LightSpriteDestroy(ID2LIGHT(pLight->iLight));
					}
				}

        // Handle sight here....
		    AllTeamsLookForAll( FALSE );
			}
    }
  }
}



BOOLEAN SaveLightEffectsToSaveGameFile( HWFILE hFile )
{
	/*
	UINT32	uiNumberOfLights=0;
	UINT32	uiCnt;

	//loop through and count the number of active slots
	for( uiCnt=0; uiCnt<guiNumLightEffects; uiCnt++)
	{
		if( gLightEffectData[ uiCnt ].fAllocated )
		{
			uiNumberOfLights++;
		}
	}

	//Save the Number of Light Effects
	if (!FileWrite(hFile, &uiNumberOfLights, sizeof(UINT32))) return FALSE;

	//if there are lights to save
	if( uiNumberOfLights != 0 )
	{
		//loop through and save each active slot
		for( uiCnt=0; uiCnt < guiNumLightEffects; uiCnt++)
		{
			if( gLightEffectData[ uiCnt ].fAllocated )
			{
				//Save the Light effect Data
				if (!FileWrite(hFile, &gLightEffectData[uiCnt], sizeof(LIGHTEFFECT))) return FALSE;
			}
		}
	}
*/
	return( TRUE );
}


BOOLEAN LoadLightEffectsFromLoadGameFile( HWFILE hFile )
{
	UINT32	uiCount;

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
			for(uiCount=0; uiCount < guiNumLightEffects; uiCount++)
			{
				//Load the Light effect Data
				if (!FileRead(hFile, &gLightEffectData[uiCount], sizeof(LIGHTEFFECT))) return FALSE;
			}
		}


		//loop through and apply the light effects to the map
		for(uiCount=0; uiCount < guiNumLightEffects; uiCount++)
		{
			if( gLightEffectData[uiCount].fAllocated )
				UpdateLightingSprite( &( gLightEffectData[uiCount] ) );
		}
	}

	return( TRUE );
}



BOOLEAN SaveLightEffectsToMapTempFile( INT16 sMapX, INT16 sMapY, INT8 bMapZ )
{
	UINT32	uiNumLightEffects=0;
	HWFILE	hFile;
	CHAR8		zMapName[ 128 ];
	UINT32	uiCnt;

	//get the name of the map
	GetMapTempFileName( SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS, zMapName, sMapX, sMapY, bMapZ );

	//delete file the file.
	FileDelete( zMapName );

	//loop through and count the number of Light effects
	for( uiCnt=0; uiCnt<guiNumLightEffects; uiCnt++)
	{
		if( gLightEffectData[ uiCnt ].fAllocated )
			uiNumLightEffects++;
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
	for( uiCnt=0; uiCnt < guiNumLightEffects; uiCnt++)
	{
		//if the Light is active
		if( gLightEffectData[ uiCnt ].fAllocated )
		{
			//Save the Light effect Data
			if (!FileWrite(hFile, &gLightEffectData[uiCnt], sizeof(LIGHTEFFECT)))
			{
				//Close the file
				FileClose( hFile );

				return( FALSE );
			}
		}
	}

	//Close the file
	FileClose( hFile );

	SetSectorFlag( sMapX, sMapY, bMapZ, SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS );

	return( TRUE );
}



BOOLEAN LoadLightEffectsFromMapTempFile( INT16 sMapX, INT16 sMapY, INT8 bMapZ )
{
	UINT32	uiCount;
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
		//Load the Light effect Data
		if (!FileRead(hFile, &gLightEffectData[uiCnt], sizeof(LIGHTEFFECT)))
		{
			FileClose( hFile );
			return( FALSE );
		}
	}


	//loop through and apply the light effects to the map
	for(uiCount=0; uiCount < guiNumLightEffects; uiCount++)
	{
		if( gLightEffectData[uiCount].fAllocated )
			UpdateLightingSprite( &( gLightEffectData[uiCount] ) );
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
