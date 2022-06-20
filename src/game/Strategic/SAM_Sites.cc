#include "SAM_Sites.h"
#include "Campaign_Types.h"
#include "ContentManager.h"
#include "Explosion_Control.h"
#include "GameInstance.h"
#include "Logger.h"
#include "SamSiteModel.h"
#include "SaveLoadMap.h"
#include "StrategicMap.h"
#include "TileDat.h"
#include "TileDef.h"
#include "WorldMan.h"
#include <string_theory/format>


Observable<> OnAirspaceControlUpdated = {};

static void UpdateAndDamageSAMIfFound(INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ, INT16 sGridNo, void*, UINT8 ubDamage, BOOLEAN fIsDestroyed);

void InitializeSAMSites()
{
	// handle SAM site damages
	OnStructureDamaged.addListener("default:sam", UpdateAndDamageSAMIfFound);

	// All SAM sites start game in perfect working condition.
	for (auto samSite : GCM->getSamSites())
	{
		UINT8 ubSectorID = samSite->sectorId;
		StrategicMap[SGPSector(ubSectorID).AsStrategicIndex()].bSAMCondition = 100;
	}

	UpdateAirspaceControl();
}

void UpdateSAMDoneRepair(const SGPSector& sec)
{
	// ATE: If we are below, return right away
	if (sec.z != 0) return;

	UINT8 const sector = sec.AsByte();
	auto samSite = GCM->findSamSiteBySector(sector);
	if (samSite == NULL)
	{
		SLOGW("There is no SAM site at sector {}", sec);
		return;
	}

	UINT16 const good_graphic = GetTileIndexFromTypeSubIndex(EIGHTISTRUCT, samSite->graphicIndex);
	UINT16 const damaged_graphic = good_graphic - 2; // Damaged one (current) is 2 less
	GridNo const gridno = samSite->gridNos[0];
	if (sec == gWorldSector)
	{ // Sector loaded, update graphic
		ApplyMapChangesToMapTempFile app;
		RemoveStruct(gridno, damaged_graphic);
		AddStructToHead(gridno, good_graphic);
	}
	else
	{ // We add temp changes to map not loaded
		RemoveStructFromUnLoadedMapTempFile(gridno, damaged_graphic, sec);
		AddStructToUnLoadedMapTempFile(gridno, good_graphic, sec);
	}

	// SAM site may have been put back into working order
	UpdateAirspaceControl();
}

void UpdateAirspaceControl()
{
	auto samList = GCM->getSamSites();
	SGPSector sMap;
	for (sMap.x = 1; sMap.x < (MAP_WORLD_X - 1); sMap.x++)
	{
		for (sMap.y = 1; sMap.y < (MAP_WORLD_Y - 1); sMap.y++)
		{
			BOOLEAN fEnemyControlsAir = FALSE;
			INT8 bControllingSAM = GCM->getControllingSamSite(sMap.AsByte());
			if (bControllingSAM >= 0 && (UINT8)bControllingSAM < samList.size())
			{
				UINT8 ubSector = samList[bControllingSAM]->sectorId;
				StrategicMapElement* pSAMStrategicMap = &(StrategicMap[SGPSector(ubSector).AsStrategicIndex()]);

				// if the enemies own the controlling SAM site, and it's in working condition
				if ((pSAMStrategicMap->fEnemyControlled) && (pSAMStrategicMap->bSAMCondition >= MIN_CONDITION_FOR_SAM_SITE_TO_WORK))
				{
					fEnemyControlsAir = TRUE;
				}
			}

			StrategicMap[sMap.AsStrategicIndex()].fEnemyAirControlled = fEnemyControlsAir;
		}
	}

	OnAirspaceControlUpdated();
}

INT32 GetNumberOfSAMSitesUnderPlayerControl()
{
	INT32 n = 0;
	for (auto samSite : GCM->getSamSites())
	{
		UINT8 ubSectorID = samSite->sectorId;
		if (!StrategicMap[SGPSector(ubSectorID).AsStrategicIndex()].fEnemyControlled) ++n;
	}
	return n;
}

bool IsThereAFunctionalSAMSiteInSector(const SGPSector& sector)
{
	return IsThisSectorASAMSector(sector) &&
		StrategicMap[sector.AsStrategicIndex()].bSAMCondition >= MIN_CONDITION_FOR_SAM_SITE_TO_WORK;
}

bool IsThisSectorASAMSector(const SGPSector& sector)
{
	if (sector.z != 0) return false;

	UINT8 ubSector = sector.AsByte();
	return (GCM->findSamIDBySector(ubSector) > -1);
}


// a -1 will be returned upon failure
INT8 GetSAMIdFromSector(const SGPSector& sector)
{
	// check if valid sector
	if (sector.z != 0)
	{
		return(-1);
	}

	// get the sector value
	INT16 sSectorValue = sector.AsByte();
	return GCM->findSamIDBySector(sSectorValue);
}

bool DoesSAMExistHere(const SGPSector& sector, GridNo const gridno)
{
	// ATE: If we are below, return right away
	if (sector.z != 0) return false;

	for (auto s : GCM->getSamSites())
	{
		if (s->doesSamExistHere(sector, gridno))
		{
			return true;
		}
	}
	return false;
}

// Look for a SAM site, update
static void UpdateAndDamageSAMIfFound(INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ, INT16 sGridNo, void*, UINT8 ubDamage, BOOLEAN fIsDestroyed)
{
	SGPSector sMap(sSectorX, sSectorY, sSectorZ);
	// OK, First check if SAM exists, and if not, return
	if (!DoesSAMExistHere(sMap, sGridNo))
	{
		return;
	}

	// Damage.....
	INT16 sSectorNo = sMap.AsStrategicIndex();
	SLOGD("SAM site at sector #{} is damaged by {} points", sSectorNo, ubDamage);
	if (StrategicMap[sSectorNo].bSAMCondition >= ubDamage)
	{
		StrategicMap[sSectorNo].bSAMCondition -= ubDamage;
	}
	else
	{
		StrategicMap[sSectorNo].bSAMCondition = 0;
	}

	// SAM site may have been put out of commission...
	UpdateAirspaceControl();

	// ATE: GRAPHICS UPDATE WILL GET DONE VIA NORMAL EXPLOSION CODE.....
}
