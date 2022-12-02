#include "Items.h"
#include "Handle_Items.h"
#include "Overhead.h"
#include "Structure.h"
#include "Weapons.h"
#include "Points.h"
#include "TileDef.h"
#include "WorldDef.h"
#include "Font_Control.h"
#include "Render_Dirty.h"
#include "World_Items.h"
#include "Isometric_Utils.h"
#include "Sys_Globals.h"
#include "StrategicMap.h"
#include "Campaign_Types.h"
#include "Random.h"
#include "Action_Items.h"
#include "GameSettings.h"
#include "Quests.h"
#include "Soldier_Profile.h"
#include "FileMan.h"
#include "ContentManager.h"
#include "GameInstance.h"
#include "MagazineModel.h"
#include "WeaponModels.h"

#include <algorithm>
#include <stdexcept>
#include <vector>

//Global dynamic array of all of the items in a loaded map.
std::vector<WORLDITEM> gWorldItems;

std::vector<WORLDBOMB> gWorldBombs;


static INT32 GetFreeWorldBombIndex(void)
{
	INT32 idx;

	Assert(gWorldBombs.size() <= INT32_MAX);
	for (idx = 0; idx < static_cast<INT32>(gWorldBombs.size()); idx++)
	{
		if (!gWorldBombs[idx].fExists) return idx;
	}

	gWorldBombs.push_back(WORLDBOMB{});

	// Return uiCount.....
	return( idx );
}


static INT32 AddBombToWorld(INT32 iItemIndex)
{
	UINT32	iBombIndex;

	iBombIndex = GetFreeWorldBombIndex( );

	//Add the new world item to the table.
	gWorldBombs[ iBombIndex ].fExists = TRUE;
	gWorldBombs[ iBombIndex ].iItemIndex = iItemIndex;

	return ( iBombIndex );
}


static void RemoveBombFromWorldByItemIndex(INT32 iItemIndex)
{
	// Find the world bomb which corresponds with a particular world item, then
	// remove the world bomb from the table.
	FOR_EACH_WORLD_BOMB(wb)
	{
		if (wb.iItemIndex != iItemIndex) continue;

		wb.fExists = FALSE;
		return;
	}
}


INT32 FindWorldItemForBombInGridNo(const INT16 sGridNo, const INT8 bLevel)
{
	CFOR_EACH_WORLD_BOMB(wb)
	{
		WORLDITEM const& wi = GetWorldItem(wb.iItemIndex);
		if (wi.sGridNo != sGridNo || wi.ubLevel != bLevel) continue;

		return wb.iItemIndex;
	}
	throw std::logic_error("Cannot find bomb item");
}


void FindPanicBombsAndTriggers(void)
{
	// This function searches the bomb table to find panic-trigger-tuned bombs and triggers
	CFOR_EACH_WORLD_BOMB(wb)
	{
		WORLDITEM  const& wi = GetWorldItem(wb.iItemIndex);
		OBJECTTYPE const& o  = wi.o;

		INT8 bPanicIndex;
		switch (o.bFrequency)
		{
			case PANIC_FREQUENCY:   bPanicIndex = 0; break;
			case PANIC_FREQUENCY_2: bPanicIndex = 1; break;
			case PANIC_FREQUENCY_3: bPanicIndex = 2; break;
			default:                continue;
		}

		if (o.usItem == SWITCH)
		{
			INT16                  sGridNo = wi.sGridNo;
			const STRUCTURE* const switch_ = FindStructure(sGridNo, STRUCTURE_SWITCH);
			if (switch_)
			{
				switch (switch_->ubWallOrientation)
				{
					case INSIDE_TOP_LEFT:
					case OUTSIDE_TOP_LEFT:  sGridNo += DirectionInc(SOUTH); break;
					case INSIDE_TOP_RIGHT:
					case OUTSIDE_TOP_RIGHT: sGridNo += DirectionInc(EAST);  break;

					default: break;
				}
			}

			gTacticalStatus.fPanicFlags                      |= PANIC_TRIGGERS_HERE;
			gTacticalStatus.sPanicTriggerGridNo[bPanicIndex]  = sGridNo;
			gTacticalStatus.ubPanicTolerance[bPanicIndex]     = o.ubTolerance;
			if (o.fFlags & OBJECT_ALARM_TRIGGER)
			{
				gTacticalStatus.bPanicTriggerIsAlarm[bPanicIndex] = TRUE;
			}
			if (bPanicIndex + 1 == NUM_PANIC_TRIGGERS) return;
		}
		else
		{
			gTacticalStatus.fPanicFlags |= PANIC_BOMBS_HERE;
		}
	}
}


static INT32 GetFreeWorldItemIndex(void)
{
	INT32 iItemIndex;

	Assert(gWorldItems.size() <= INT32_MAX);
	for (iItemIndex = 0; iItemIndex < static_cast<INT32>(gWorldItems.size()); iItemIndex++)
	{
		if (!gWorldItems[iItemIndex].fExists) return iItemIndex;
	}

	gWorldItems.push_back(WORLDITEM{});
	return iItemIndex;
}


static UINT32 GetNumUsedWorldItems(void)
{
	UINT32 count = 0;
	CFOR_EACH_WORLD_ITEM(wi) ++count;
	return count;
}


INT32 AddItemToWorld(INT16 sGridNo, const OBJECTTYPE* const pObject, const UINT8 ubLevel, const UINT16 usFlags, const INT8 bRenderZHeightAboveLevel, const INT8 bVisible)
{
	// ATE: Check if the gridno is OK
	if (sGridNo == NOWHERE)
	{
		// Display warning.....
		SLOGW("Item {} was given invalid grid location {}. Please report", pObject->usItem, sGridNo);
		return -1;
	}

	const UINT32 iItemIndex = GetFreeWorldItemIndex();
	WORLDITEM& wi = GetWorldItem(iItemIndex);

	//Add the new world item to the table.
	wi.fExists                  = TRUE;
	wi.sGridNo                  = sGridNo;
	wi.ubLevel                  = ubLevel;
	wi.usFlags                  = usFlags;
	wi.bVisible                 = bVisible;
	wi.bRenderZHeightAboveLevel = bRenderZHeightAboveLevel;
	wi.o                        = *pObject;

	// Add a bomb reference if needed
	if (usFlags & WORLD_ITEM_ARMED_BOMB)
	{
		if (AddBombToWorld(iItemIndex) == -1) return -1;
	}

	return iItemIndex;
}


void RemoveItemFromWorld(const INT32 iItemIndex)
{
	WORLDITEM& wi = GetWorldItem(iItemIndex);
	if (!wi.fExists) return;

	// If it's a bomb, remove the appropriate entry from the bomb table
	if (wi.usFlags & WORLD_ITEM_ARMED_BOMB)
	{
		RemoveBombFromWorldByItemIndex(iItemIndex);
	}
	wi.fExists = FALSE;
}


void TrashWorldItems()
{
	FOR_EACH_WORLD_ITEM(wi)
	{
		RemoveItemFromPool(wi);
	}
	gWorldItems.clear();
	gWorldBombs.clear();
}


void SaveWorldItemsToMap(HWFILE const f)
{
	UINT32 const n_actual_world_items = GetNumUsedWorldItems();
	f->write(&n_actual_world_items, sizeof(n_actual_world_items));

	CFOR_EACH_WORLD_ITEM(wi) f->write(&wi, sizeof(WORLDITEM));
}


static void DeleteWorldItemsBelongingToQueenIfThere(void);
static void DeleteWorldItemsBelongingToTerroristsWhoAreNotThere(void);


void LoadWorldItemsFromMap(HWFILE const f)
{
	// If any world items exist, we must delete them now
	TrashWorldItems();

	// Read the number of items that were saved in the map
	UINT32 n_world_items;
	auto itemReplacements = GCM->getMapItemReplacements();
	f->read(&n_world_items, sizeof(n_world_items));

	if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME && !gfEditMode)
	{
		// The sector has already been visited. The items are saved in a different
		// format that will be loaded later on. So, all we need to do is skip the
		// data entirely.
		f->seek(sizeof(WORLDITEM) * n_world_items, FILE_SEEK_FROM_CURRENT);
		return;
	}

	for (UINT32 n = n_world_items; n != 0; --n)
	{
		// Add all of the items to the world indirectly through AddItemToPool, but
		// only if the chance associated with them succeed.
		WORLDITEM wi;
		f->read(&wi, sizeof(wi));
		OBJECTTYPE& o = wi.o;

		if (o.usItem == OWNERSHIP) wi.ubNonExistChance = 0;

		if (!gfEditMode && PreRandom(100) < wi.ubNonExistChance) continue;

		if (!gfEditMode)
		{
			// Check for matching item existance modes and only add if there is a match
			if (wi.usFlags & (gGameOptions.fSciFi ? WORLD_ITEM_SCIFI_ONLY : WORLD_ITEM_REALISTIC_ONLY)) continue;

			// Check if we have a item replacement mapping for this item
			if (itemReplacements.find(o.usItem) != itemReplacements.end())
			{
				auto item = itemReplacements.at(o.usItem);
				if (item == 0)
				{
					SLOGW("Map item #{} removed", o.usItem);
					continue;
				}

				SLOGD("Map item #{} replaced by #{}", o.usItem, item);
				o.usItem = item;
			}

			const ItemModel* item = GCM->getItem(o.usItem);
			if (item->getFlags() & ITEM_NOT_EDITOR) {
				// This item is not placable by Editor. Maybe the map was created for a different item set.
				SLOGW("Skipping non-Editor item #{}({}) at gridNo {}", item->getItemIndex(), item->getInternalName(), wi.sGridNo);
				continue;
			}

			if (!gGameOptions.fGunNut)
			{
				// do replacements?
				const WeaponModel *weapon = item->asWeapon();
				const MagazineModel *mag = item->asAmmo();
				if (weapon && weapon->isInBigGunList())
				{
					const WeaponModel *replacement = GCM->getWeaponByName(item->asWeapon()->getStandardReplacement());

						// everything else can be the same? no.
						INT8 const ammo     = o.ubGunShotsLeft;
						INT8       new_ammo = replacement->ubMagSize * ammo / weapon->ubMagSize;
						if (new_ammo == 0 && ammo > 0) new_ammo = 1;
						o.usItem         = replacement->getItemIndex();
						o.ubGunShotsLeft = new_ammo;
				}
				else if (mag && mag->isInBigGunList())
				{
					const MagazineModel *replacement = GCM->getMagazineByName(mag->getStandardReplacement());

						// Go through status values and scale up/down
						UINT8 const mag_size     = mag->capacity;
						UINT8 const new_mag_size = replacement->capacity;
						for (UINT8 i = 0; i != o.ubNumberOfObjects; ++i)
						{
							o.bStatus[i] = o.bStatus[i] * new_mag_size / (mag_size? mag_size: 1);
						}

						// then replace item #
						o.usItem = replacement->getItemIndex();
				}
			}
		}

		switch (o.usItem)
		{
			case ACTION_ITEM:
				// If we are loading a pit, they are typically loaded without being armed.
				if (o.bActionValue == ACTION_ITEM_SMALL_PIT ||
					o.bActionValue == ACTION_ITEM_LARGE_PIT)
				{
					wi.usFlags      &= ~WORLD_ITEM_ARMED_BOMB;
					wi.bVisible      = BURIED;
					o.bDetonatorType = 0;
				}
				break;

			case MINE:
			case TRIP_FLARE:
			case TRIP_KLAXON:
				if (wi.bVisible == HIDDEN_ITEM && o.bTrap > 0)
				{
					ArmBomb(&o, BOMB_PRESSURE);
					wi.usFlags |= WORLD_ITEM_ARMED_BOMB;
					// this is coming from the map so the enemy must know about it.
					gpWorldLevelData[wi.sGridNo].uiFlags |= MAPELEMENT_ENEMY_MINE_PRESENT;
				}
				break;
		}

		// All armed bombs are buried
		if (wi.usFlags & WORLD_ITEM_ARMED_BOMB) wi.bVisible = BURIED;

		INT32 const item_idx = AddItemToPool(wi.sGridNo, &o, static_cast<Visibility>(wi.bVisible), wi.ubLevel, wi.usFlags, wi.bRenderZHeightAboveLevel);
		GetWorldItem(item_idx).ubNonExistChance = wi.ubNonExistChance;
	}

	if (!gfEditMode)
	{
		DeleteWorldItemsBelongingToTerroristsWhoAreNotThere();
		static const SGPSector medunaBasement(3, MAP_ROW_P, 1);
		if (gWorldSector == medunaBasement)
		{
			DeleteWorldItemsBelongingToQueenIfThere();
		}
	}
}


static void DeleteWorldItemsBelongingToTerroristsWhoAreNotThere(void)
{
	// only do this after Carmen has talked to player and terrorists have been placed
	//if ( CheckFact( FACT_CARMEN_EXPLAINED_DEAL, 0 ) == TRUE )
	{
		CFOR_EACH_WORLD_ITEM(wi)
		{
			// loop through all items, look for ownership
			if (wi.o.usItem != OWNERSHIP) continue;

			const ProfileID pid = wi.o.ubOwnerProfile;
			// if owner is a terrorist
			if (!IsProfileATerrorist(pid)) continue;

			MERCPROFILESTRUCT const& p = GetProfile(pid);
			// and they were not set in the current sector
			if (p.sSector == gWorldSector) continue;

			// then all items in this location should be deleted
			const INT16 sGridNo = wi.sGridNo;
			const UINT8 ubLevel = wi.ubLevel;
			FOR_EACH_WORLD_ITEM(owned_item)
			{
				if (owned_item.sGridNo == sGridNo && owned_item.ubLevel == ubLevel)
				{
					RemoveItemFromPool(owned_item);
				}
			}
		}
	}
	// else the terrorists haven't been placed yet!
}


static void DeleteWorldItemsBelongingToQueenIfThere(void)
{
	MERCPROFILESTRUCT& q = GetProfile(QUEEN);

	if (q.sSector != gWorldSector)
	{
		return;
	}

	CFOR_EACH_WORLD_ITEM(wi)
	{
		// Look for items belonging to the queen
		if (wi.o.usItem         != OWNERSHIP) continue;
		if (wi.o.ubOwnerProfile != QUEEN)     continue;

		// Delete all items on this tile
		const INT16 sGridNo = wi.sGridNo;
		const UINT8 ubLevel = wi.ubLevel;
		FOR_EACH_WORLD_ITEM(item)
		{
			if (item.sGridNo != sGridNo) continue;
			if (item.ubLevel != ubLevel) continue;

			// Upgrade equipment
			switch (item.o.usItem)
			{
				case AUTO_ROCKET_RIFLE:
				{
					// Give her auto rifle
					INT8 const bSlot = FindObjectInSoldierProfile(q, ROCKET_RIFLE);
					if (bSlot != NO_SLOT) q.inv[bSlot] = AUTO_ROCKET_RIFLE;
					break;
				}

				case SPECTRA_HELMET_18:   q.inv[HELMETPOS] = SPECTRA_HELMET_18;   break;
				case SPECTRA_VEST_18:     q.inv[VESTPOS]   = SPECTRA_VEST_18;     break;
				case SPECTRA_LEGGINGS_18: q.inv[LEGPOS]    = SPECTRA_LEGGINGS_18; break;

				default: break;
			}
			RemoveItemFromPool(item);
		}
	}
}


// Refresh item pools
void RefreshWorldItemsIntoItemPools(const std::vector<WORLDITEM>& items)
{
	for (const WORLDITEM& wi : items)
	{
		if (!wi.fExists) continue;
		OBJECTTYPE o = wi.o; // XXX AddItemToPool() may alter the object
		AddItemToPool(wi.sGridNo, &o, static_cast<Visibility>(wi.bVisible), wi.ubLevel, wi.usFlags, wi.bRenderZHeightAboveLevel);
	}
}
