#include "LoadSaveLightEffect.h"
#include "WorldDef.h"
#include "LightEffects.h"
#include "Isometric_Utils.h"
#include "Lighting.h"
#include "Game_Clock.h"
#include "OppList.h"
#include "Tactical_Save.h"
#include "Campaign_Types.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <algorithm>

#define NUM_LIGHT_EFFECT_SLOTS 25


// GLOBAL FOR LIGHT LISTING
static LIGHTEFFECT gLightEffectData[NUM_LIGHT_EFFECT_SLOTS];
static UINT32      guiNumLightEffects = 0;


#define BASE_FOR_EACH_LIGHTEFFECT(type, iter)                           \
	for (type*       iter        = gLightEffectData,                      \
		* const iter##__end = gLightEffectData + guiNumLightEffects; \
		iter != iter##__end;                                             \
		++iter)                                                          \
		if (!iter->fAllocated) continue; else
#define FOR_EACH_LIGHTEFFECT(iter)  BASE_FOR_EACH_LIGHTEFFECT(      LIGHTEFFECT, iter)
#define CFOR_EACH_LIGHTEFFECT(iter) BASE_FOR_EACH_LIGHTEFFECT(const LIGHTEFFECT, iter)


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
	// Build light....
	ST::string LightName = ST::format("Light{}", pLight->bRadius);

	// Delete old one if one exists...
	if (pLight->light != NULL)
	{
		LightSpriteDestroy(pLight->light);
		pLight->light = NULL;
	}

	// Effect light.....
	LIGHT_SPRITE* const l = LightSpriteCreate(LightName);
	pLight->light = l;
	if (l == NULL)
	{
		// Could not light!
		return;
	}

	LightSpritePower(l, TRUE);
	LightSpritePosition(l, CenterX(pLight->sGridNo) / CELL_X_SIZE, CenterY(pLight->sGridNo) / CELL_Y_SIZE);
}


LIGHTEFFECT* NewLightEffect(const INT16 sGridNo, UINT8 radius, UINT8 duration)
{
	LIGHTEFFECT* const l = GetFreeLightEffect();
	if (l == NULL) return NULL;

	*l = LIGHTEFFECT{};

	// Set some values...
	l->sGridNo            = sGridNo;
	l->light              = NULL;
	l->uiTimeOfLastUpdate = GetWorldTotalSeconds();
	l->ubDuration = duration;
	l->bRadius    = radius;
	l->bAge       = 0;
	l->fAllocated = TRUE;

	UpdateLightingSprite(l);

	// Handle sight here....
	AllTeamsLookForAll( FALSE );

	return l;
}


void DecayLightEffects(const UINT32 uiTime, const bool updateSightings)
{
	// age all active tear gas clouds, deactivate those that are just dispersing
	FOR_EACH_LIGHTEFFECT(l)
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
		if (updateSightings) AllTeamsLookForAll(FALSE);
	}
}


void LoadLightEffectsFromLoadGameFile(HWFILE const hFile)
{
	std::fill_n(gLightEffectData, NUM_LIGHT_EFFECT_SLOTS, LIGHTEFFECT{});

	//Load the Number of Light Effects
	hFile->read(&guiNumLightEffects, sizeof(UINT32));

	//if there are lights saved.
	if( guiNumLightEffects != 0 )
	{
		//loop through and apply the light effects to the map
		for (UINT32 uiCount = 0; uiCount < guiNumLightEffects; ++uiCount)
		{
			ExtractLightEffectFromFile(hFile, &gLightEffectData[uiCount]);
		}
	}

	//loop through and apply the light effects to the map
	FOR_EACH_LIGHTEFFECT(l)
	{
		UpdateLightingSprite(l);
	}
}


void SaveLightEffectsToMapTempFile(const SGPSector& sMap)
{
	UINT32	uiNumLightEffects=0;

	//get the name of the map
	ST::string const zMapName = GetMapTempFileName(SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS, sMap);
	GCM->tempFiles()->deleteFile( zMapName );

	//loop through and count the number of Light effects
	CFOR_EACH_LIGHTEFFECT(l)
	{
		++uiNumLightEffects;
	}

	//if there are no Light effects
	if( uiNumLightEffects == 0 )
	{
		//set the fact that there are no Light effects for this sector
		ReSetSectorFlag(sMap, SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS);
		return;
	}

	AutoSGPFile hFile(GCM->tempFiles()->openForWriting(zMapName, true));

	//Save the Number of Light Effects
	hFile->write(&uiNumLightEffects, sizeof(UINT32));

	//loop through and save the number of Light effects
	CFOR_EACH_LIGHTEFFECT(l)
	{
		InjectLightEffectIntoFile(hFile, l);
	}

	SetSectorFlag(sMap, SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS);
}


void LoadLightEffectsFromMapTempFile(const SGPSector& sMap)
{
	UINT32	uiCnt=0;

	ST::string const zMapName = GetMapTempFileName(SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS, sMap);
	AutoSGPFile hFile(GCM->tempFiles()->openForReading(zMapName));

	//Clear out the old list
	ResetLightEffects();

	//Load the Number of Light Effects
	hFile->read(&guiNumLightEffects, sizeof(UINT32));

	//loop through and load the list
	for( uiCnt=0; uiCnt<guiNumLightEffects;uiCnt++)
	{
		ExtractLightEffectFromFile(hFile, &gLightEffectData[uiCnt]);
	}

	//loop through and apply the light effects to the map
	FOR_EACH_LIGHTEFFECT(l)
	{
		UpdateLightingSprite(l);
	}
}


void ResetLightEffects()
{
	//Clear out the old list
	std::fill_n(gLightEffectData, NUM_LIGHT_EFFECT_SLOTS, LIGHTEFFECT{});
	guiNumLightEffects = 0;
}
