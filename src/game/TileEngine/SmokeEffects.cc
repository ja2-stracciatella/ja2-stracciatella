#include "Directories.h"
#include "GameSettings.h"
#include "ItemModel.h"
#include "LoadSaveSmokeEffect.h"
#include "Overhead.h"
#include "Debug.h"
#include "Soldier_Control.h"
#include "TileDef.h"
#include "Weapons.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "Tile_Animation.h"
#include "SmokeEffects.h"
#include "Isometric_Utils.h"
#include "RenderWorld.h"
#include "Explosion_Control.h"
#include "Random.h"
#include "Game_Clock.h"
#include "OppList.h"
#include "Tactical_Save.h"
#include "Campaign_Types.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <algorithm>
#include <stdexcept>

#define NUM_SMOKE_EFFECT_SLOTS 25


// GLOBAL FOR SMOKE LISTING
static SMOKEEFFECT gSmokeEffectData[NUM_SMOKE_EFFECT_SLOTS];
static UINT32      guiNumSmokeEffects = 0;


#define BASE_FOR_EACH_SMOKE_EFFECT(type, iter)                    \
	for (type* iter        = gSmokeEffectData,                      \
		* iter##__end = gSmokeEffectData + guiNumSmokeEffects; \
		iter != iter##__end;                                       \
		++iter)                                                    \
		if (!iter->fAllocated) continue; else
#define FOR_EACH_SMOKE_EFFECT(iter)  BASE_FOR_EACH_SMOKE_EFFECT(      SMOKEEFFECT, iter)
#define CFOR_EACH_SMOKE_EFFECT(iter) BASE_FOR_EACH_SMOKE_EFFECT(const SMOKEEFFECT, iter)


static SMOKEEFFECT* GetFreeSmokeEffect(void)
{
	for (SMOKEEFFECT* s = gSmokeEffectData; s != gSmokeEffectData + guiNumSmokeEffects; ++s)
	{
		if (!s->fAllocated) return s;
	}
	if (guiNumSmokeEffects < NUM_SMOKE_EFFECT_SLOTS)
	{
		return &gSmokeEffectData[guiNumSmokeEffects++];
	}
	return NULL;
}

// Returns NO_SMOKE_EFFECT if none there...
SmokeEffectID GetSmokeEffectOnTile(INT16 const sGridNo, INT8 const bLevel)
{
	UINT8		ubExtFlags;

	ubExtFlags = gpWorldLevelData[ sGridNo ].ubExtFlags[ bLevel ];

	// Look at worldleveldata to find flags..
	if ( ubExtFlags & ANY_SMOKE_EFFECT )
	{
		// Which smoke am i?
		return( FromWorldFlagsToSmokeType( ubExtFlags ) );
	}

	return( SmokeEffectID::NOTHING );
}


SmokeEffectID FromWorldFlagsToSmokeType(UINT8 ubWorldFlags)
{
	if ( ubWorldFlags & MAPELEMENT_EXT_SMOKE )
	{
		return( SmokeEffectID::SMOKE );
	}
	else if ( ubWorldFlags & MAPELEMENT_EXT_TEARGAS )
	{
		return( SmokeEffectID::TEARGAS );
	}
	else if ( ubWorldFlags & MAPELEMENT_EXT_MUSTARDGAS )
	{
		return( SmokeEffectID::MUSTARDGAS );
	}
	else if ( ubWorldFlags & MAPELEMENT_EXT_CREATUREGAS )
	{
		return( SmokeEffectID::CREATUREGAS );
	}
	else
	{
		return( SmokeEffectID::NOTHING );
	}
}


static UINT8 FromSmokeTypeToWorldFlags(SmokeEffectID const bType)
{
	switch( bType )
	{
		case SmokeEffectID::SMOKE:     return MAPELEMENT_EXT_SMOKE;
		case SmokeEffectID::TEARGAS:    return MAPELEMENT_EXT_TEARGAS;
		case SmokeEffectID::MUSTARDGAS: return MAPELEMENT_EXT_MUSTARDGAS;
		case SmokeEffectID::CREATUREGAS:   return MAPELEMENT_EXT_CREATUREGAS;
		default:                      return 0;
	}
}

bool IsSoldierAlreadyAffectedBySmokeEffect(const SOLDIERTYPE* soldier, const SmokeEffectModel* smokeEffect) {
	if (!soldier) {
		SLOGE("Called IsSoldierAlreadyAffectedBySmokeEffect with nullptr for soldier");
		return false;
	}
	if (!smokeEffect) {
		SLOGE("Called IsSoldierAlreadyAffectedBySmokeEffect with nullptr for smokeEffect");
		return false;
	}

	uint8_t flag = 0;
	switch (smokeEffect->getID()) {
		case SmokeEffectID::CREATUREGAS:
			flag = HIT_BY_CREATUREGAS;
			break;
		case SmokeEffectID::TEARGAS:
			flag = HIT_BY_TEARGAS;
			break;
		case SmokeEffectID::MUSTARDGAS:
			flag = HIT_BY_MUSTARDGAS;
			break;
		default:
			break;
	}

	return soldier->fHitByGasFlags & flag;
}


void NewSmokeEffect(const INT16 sGridNo, const UINT16 usItem, const INT8 bLevel, SOLDIERTYPE* const owner)
{
	SMOKEEFFECT* const pSmoke = GetFreeSmokeEffect();
	if (pSmoke == NULL) return;

	*pSmoke = SMOKEEFFECT{};

	// Set some values...
	pSmoke->sGridNo									= sGridNo;
	pSmoke->usItem									= usItem;
	pSmoke->uiTimeOfLastUpdate			= GetWorldTotalSeconds( );

	// Are we indoors?
	if ( GetTerrainType( sGridNo ) == FLAT_FLOOR )
	{
		pSmoke->bFlags |= SMOKE_EFFECT_INDOORS;
	}

	auto smokeEffect = GCM->getExplosive(usItem)->getSmokeEffect();

	pSmoke->ubDuration	= smokeEffect->duration;
	pSmoke->ubRadius    = smokeEffect->initialRadius;
	pSmoke->bAge		= 0;
	pSmoke->fAllocated  = TRUE;
	pSmoke->bType		= static_cast<int8_t>(smokeEffect->smokeEffect->getID());
	pSmoke->owner       = owner;

	if ( pSmoke->bFlags & SMOKE_EFFECT_INDOORS )
	{
		// Duration is increased by 2 turns...indoors
		pSmoke->ubDuration += 3;
	}

	if ( bLevel )
	{
		pSmoke->bFlags |= SMOKE_EFFECT_ON_ROOF;
	}

	// ATE: FALSE into subsequent-- it's the first one!
	SpreadEffectSmoke(pSmoke, FALSE, bLevel);
}


// Add smoke to gridno
// ( Replacement algorithm uses distance away )
void AddSmokeEffectToTile(SMOKEEFFECT const* const smoke, const SmokeEffectModel* smokeEffect, INT16 const sGridNo, INT8 const bLevel)
{
	BOOLEAN dissipating = FALSE;
	if (smoke->ubDuration - smoke->bAge < 2)
	{
		dissipating = TRUE;
		// Remove old one...
		RemoveSmokeEffectFromTile(sGridNo, bLevel);
	}

	// If smoke effect exists already.... stop
	if (gpWorldLevelData[sGridNo].ubExtFlags[bLevel] & ANY_SMOKE_EFFECT) return;

	// Use the right graphic based on type..
	AnimationFlags ani_flags = ANITILE_FORWARD | ANITILE_SMOKE_EFFECT | ANITILE_LOOPING;
	char const*    cached_file;
	INT16          start_frame;
	if (gGameSettings.fOptions[TOPTION_ANIMATE_SMOKE])
	{
		if (dissipating)
		{
			cached_file = smokeEffect->getDissipatingGraphics().c_str();
		}
		else
		{
			cached_file = smokeEffect->getGraphics().c_str();
		}
		start_frame  = Random(5);
		ani_flags   |= ANITILE_ALWAYS_TRANSLUCENT;
	}
	else
	{
		cached_file = smokeEffect->getStaticGraphics().c_str();
		start_frame  = 0;
		ani_flags   |= ANITILE_PAUSED;
	}

	ANITILE_PARAMS	ani_params;
	ani_params = ANITILE_PARAMS{};
	ani_params.uiFlags     = ani_flags;
	ani_params.zCachedFile = cached_file;
	ani_params.sStartFrame = start_frame;
	ani_params.sGridNo     = sGridNo;
	ani_params.ubLevelID   = (bLevel == 0 ? ANI_STRUCT_LEVEL : ANI_ONROOF_LEVEL);
	ani_params.sDelay      = 300 + Random(300);
	ani_params.sX          = CenterX(sGridNo);
	ani_params.sY          = CenterY(sGridNo);
	ani_params.sZ          = 0;
	CreateAnimationTile(&ani_params);

	gpWorldLevelData[sGridNo].ubExtFlags[bLevel] |= FromSmokeTypeToWorldFlags(smokeEffect->getID());
	SetRenderFlags(RENDER_FLAG_FULL);
}


void RemoveSmokeEffectFromTile( INT16 sGridNo, INT8 bLevel )
{
	ANITILE *pAniTile;
	UINT8		ubLevelID;

	// Get ANI tile...
	if ( bLevel == 0 )
	{
		ubLevelID = ANI_STRUCT_LEVEL;
	}
	else
	{
		ubLevelID = ANI_ONROOF_LEVEL;
	}

	pAniTile = GetCachedAniTileOfType( sGridNo, ubLevelID, ANITILE_SMOKE_EFFECT );

	if ( pAniTile != NULL )
	{
		DeleteAniTile( pAniTile );

		SetRenderFlags( RENDER_FLAG_FULL );
	}

	// Unset flags in world....
	// ( // check to see if we are the last one....
	if ( GetCachedAniTileOfType( sGridNo, ubLevelID, ANITILE_SMOKE_EFFECT ) == NULL )
	{
		gpWorldLevelData[ sGridNo ].ubExtFlags[ bLevel ] &= ( ~ANY_SMOKE_EFFECT );
	}
}


void DecaySmokeEffects(const UINT32 uiTime, const bool updateSightings)
{
	BOOLEAN fUpdate = FALSE;
	BOOLEAN fSpreadEffect;
	INT8    bLevel;
	UINT16  usNumUpdates = 1;

	// reset 'hit by gas' flags
	FOR_EACH_MERC(i) (*i)->fHitByGasFlags = 0;

	// ATE: 1 ) make first pass and delete/mark any smoke effect for update
	// all the deleting has to be done first///

	// age all active tear gas clouds, deactivate those that are just dispersing
	FOR_EACH_SMOKE_EFFECT(pSmoke)
	{
		fSpreadEffect = TRUE;

		if ( pSmoke->bFlags & SMOKE_EFFECT_ON_ROOF )
		{
			bLevel = 1;
		}
		else
		{
			bLevel = 0;
		}


		// Do things differently for combat /vs realtime
		// always try to update during combat
		if (gTacticalStatus.uiFlags & INCOMBAT )
		{
			fUpdate = TRUE;
		}
		else
		{
			// ATE: Do this every so ofte, to acheive the effect we want...
			if ( ( uiTime - pSmoke->uiTimeOfLastUpdate ) > 10 )
			{
				fUpdate = TRUE;

				usNumUpdates = ( UINT16 ) ( ( uiTime - pSmoke->uiTimeOfLastUpdate ) / 10 );
			}
		}

		if ( fUpdate )
		{
			pSmoke->uiTimeOfLastUpdate = uiTime;

			for (UINT32 cnt2 = 0; cnt2 < usNumUpdates; ++cnt2)
			{
				pSmoke->bAge++;

				if ( pSmoke->bAge == 1 )
				{
					// ATE: At least mark for update!
					pSmoke->bFlags |= SMOKE_EFFECT_MARK_FOR_UPDATE;
					fSpreadEffect = FALSE;
				}
				else
				{
					fSpreadEffect = TRUE;
				}

				if ( fSpreadEffect )
				{
					// if this cloud remains effective (duration not reached)
					if ( pSmoke->bAge <= pSmoke->ubDuration)
					{
						// ATE: Only mark now and increse radius - actual drawing is done
						// in another pass cause it could
						// just get erased...
						pSmoke->bFlags |= SMOKE_EFFECT_MARK_FOR_UPDATE;

						// calculate the new cloud radius
						// cloud expands by 1 every turn outdoors, and every other turn indoors

						// ATE: If radius is < maximum, increase radius, otherwise keep at max
						if ( pSmoke->ubRadius < GCM->getExplosive(pSmoke->usItem)->getSmokeEffect()->maxRadius )
						{
							pSmoke->ubRadius++;
						}
					}
					else
					{
						// deactivate tear gas cloud (use last known radius)
						SpreadEffectSmoke(pSmoke, ERASE_SPREAD_EFFECT, bLevel);
						pSmoke->fAllocated = FALSE;
						break;
					}
				}
			}
		}
		else
		{
			// damage anyone standing in cloud
			SpreadEffectSmoke(pSmoke, REDO_SPREAD_EFFECT, bLevel);
		}
	}

	FOR_EACH_SMOKE_EFFECT(pSmoke)
	{
		if ( pSmoke->bFlags & SMOKE_EFFECT_ON_ROOF )
		{
			bLevel = 1;
		}
		else
		{
			bLevel = 0;
		}

		// if this cloud remains effective (duration not reached)
		if ( pSmoke->bFlags & SMOKE_EFFECT_MARK_FOR_UPDATE )
		{
			SpreadEffectSmoke(pSmoke, TRUE, bLevel);
			pSmoke->bFlags &= (~SMOKE_EFFECT_MARK_FOR_UPDATE);
		}
	}

	if (updateSightings) AllTeamsLookForAll(TRUE);
}


void LoadSmokeEffectsFromLoadGameFile(HWFILE const hFile, UINT32 const savegame_version)
{
	UINT32	uiCnt=0;

	//Clear out the old list
	std::fill_n(gSmokeEffectData, NUM_SMOKE_EFFECT_SLOTS, SMOKEEFFECT{});

	//Load the Number of Smoke Effects
	hFile->read(&guiNumSmokeEffects, sizeof(UINT32));

	//This is a TEMP hack to allow us to use the saves
	if (savegame_version < 37 && guiNumSmokeEffects == 0)
	{
		ExtractSmokeEffectFromFile(hFile, &gSmokeEffectData[0]);
	}


	//loop through and load the list
	for( uiCnt=0; uiCnt<guiNumSmokeEffects;uiCnt++)
	{
		ExtractSmokeEffectFromFile(hFile, &gSmokeEffectData[uiCnt]);
		//This is a TEMP hack to allow us to use the saves
		if (savegame_version < 37)
			break;
	}

	//loop through and apply the smoke effects to the map
	FOR_EACH_SMOKE_EFFECT(s)
	{
		const INT8 bLevel = (s->bFlags & SMOKE_EFFECT_ON_ROOF ? 1 : 0);
		SpreadEffectSmoke(s, TRUE, bLevel);
	}
}


void SaveSmokeEffectsToMapTempFile(const SGPSector& sMap)
{
	UINT32	uiNumSmokeEffects=0;

	//get the name of the map
	ST::string const zMapName = GetMapTempFileName(SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS, sMap);

	//delete file the file.
	GCM->tempFiles()->deleteFile( zMapName );

	//loop through and count the number of smoke effects
	CFOR_EACH_SMOKE_EFFECT(s) ++uiNumSmokeEffects;

	//if there are no smoke effects
	if( uiNumSmokeEffects == 0 )
	{
		//set the fact that there are no smoke effects for this sector
		ReSetSectorFlag(sMap, SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS);
		return;
	}

	AutoSGPFile hFile(GCM->tempFiles()->openForWriting(zMapName, true));

	//Save the Number of Smoke Effects
	hFile->write(&uiNumSmokeEffects, sizeof(UINT32));

	CFOR_EACH_SMOKE_EFFECT(s)
	{
		InjectSmokeEffectIntoFile(hFile, s);
	}

	SetSectorFlag(sMap, SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS);
}


void LoadSmokeEffectsFromMapTempFile(const SGPSector& sMap)
{
	UINT32	uiCnt=0;

	ST::string const zMapName = GetMapTempFileName(SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS, sMap);

	AutoSGPFile hFile(GCM->tempFiles()->openForReading(zMapName));

	//Clear out the old list
	ResetSmokeEffects();


	//Load the Number of Smoke Effects
	hFile->read(&guiNumSmokeEffects, sizeof(UINT32));

	//loop through and load the list
	for( uiCnt=0; uiCnt<guiNumSmokeEffects;uiCnt++)
	{
		ExtractSmokeEffectFromFile(hFile, &gSmokeEffectData[uiCnt]);
	}

	//loop through and apply the smoke effects to the map
	FOR_EACH_SMOKE_EFFECT(s)
	{
		const INT8 bLevel = (s->bFlags & SMOKE_EFFECT_ON_ROOF ? 1 : 0);
		SpreadEffectSmoke(s, TRUE, bLevel);
	}
}


void ResetSmokeEffects()
{
	//Clear out the old list
	std::fill_n(gSmokeEffectData, NUM_SMOKE_EFFECT_SLOTS, SMOKEEFFECT{});
	guiNumSmokeEffects = 0;
}


void UpdateSmokeEffectGraphics( )
{
	FOR_EACH_SMOKE_EFFECT(s)
	{
		const INT8 bLevel = (s->bFlags & SMOKE_EFFECT_ON_ROOF ? 1 : 0);
		SpreadEffectSmoke(s, ERASE_SPREAD_EFFECT, bLevel);
		SpreadEffectSmoke(s, TRUE,                bLevel);
	}
}
