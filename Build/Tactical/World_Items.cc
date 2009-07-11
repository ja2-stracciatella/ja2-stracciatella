#include <stdexcept>

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
#include "MemMan.h"
#include "FileMan.h"

#ifdef JA2BETAVERSION
#	include "Message.h"
#endif


//Global dynamic array of all of the items in a loaded map.
WORLDITEM *		gWorldItems = NULL;
UINT32				guiNumWorldItems = 0;

WORLDBOMB *		gWorldBombs = NULL;
UINT32				guiNumWorldBombs = 0;


static INT32 GetFreeWorldBombIndex(void)
{
	UINT32 uiCount;
	WORLDBOMB *newWorldBombs;
	UINT32	uiOldNumWorldBombs;

	for(uiCount=0; uiCount < guiNumWorldBombs; uiCount++)
	{
		if (!gWorldBombs[uiCount].fExists) return (INT32)uiCount;
	}

	uiOldNumWorldBombs = guiNumWorldBombs;
	guiNumWorldBombs += 10;
	//Allocate new table with max+10 items.
	newWorldBombs = REALLOC(gWorldBombs, WORLDBOMB, guiNumWorldBombs);

	//Clear the rest of the new array
	memset( &newWorldBombs[ uiOldNumWorldBombs ], 0,
		sizeof( WORLDBOMB ) * ( guiNumWorldBombs - uiOldNumWorldBombs ) );
	gWorldBombs = newWorldBombs;

	// Return uiCount.....
	return( uiCount );
}


static INT32 AddBombToWorld(INT32 iItemIndex)
{
	UINT32	iBombIndex;

	iBombIndex = GetFreeWorldBombIndex( );

	//Add the new world item to the table.
	gWorldBombs[ iBombIndex ].fExists										= TRUE;
	gWorldBombs[ iBombIndex ].iItemIndex								= iItemIndex;

	return ( iBombIndex );
}


static void RemoveBombFromWorldByItemIndex(INT32 iItemIndex)
{
	// Find the world bomb which corresponds with a particular world item, then
	// remove the world bomb from the table.
	FOR_ALL_WORLD_BOMBS(wb)
	{
		if (wb->iItemIndex != iItemIndex) continue;

		wb->fExists = FALSE;
		return;
	}
}


INT32 FindWorldItemForBombInGridNo(const INT16 sGridNo, const INT8 bLevel)
{
	CFOR_ALL_WORLD_BOMBS(wb)
	{
		WORLDITEM const& wi = GetWorldItem(wb->iItemIndex);
		if (wi.sGridNo != sGridNo || wi.ubLevel != bLevel) continue;

		return wb->iItemIndex;
	}
	throw std::logic_error("Cannot find bomb item");
}


void FindPanicBombsAndTriggers(void)
{
	// This function searches the bomb table to find panic-trigger-tuned bombs and triggers
	CFOR_ALL_WORLD_BOMBS(wb)
	{
		WORLDITEM  const& wi = GetWorldItem(wb->iItemIndex);
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
	UINT32 uiCount;
	WORLDITEM *newWorldItems;
	UINT32	uiOldNumWorldItems;

	for(uiCount=0; uiCount < guiNumWorldItems; uiCount++)
	{
		if (!gWorldItems[uiCount].fExists) return (INT32)uiCount;
	}

	uiOldNumWorldItems = guiNumWorldItems;
	guiNumWorldItems += 10;
	//Allocate new table with max+10 items.
	newWorldItems = REALLOC(gWorldItems, WORLDITEM, guiNumWorldItems);

	//Clear the rest of the new array
	memset( &newWorldItems[ uiOldNumWorldItems ], 0,
		sizeof( WORLDITEM ) * ( guiNumWorldItems - uiOldNumWorldItems ) );
	gWorldItems = newWorldItems;

	// Return uiCount.....
	return( uiCount );
}


static UINT32 GetNumUsedWorldItems(void)
{
	UINT32 count = 0;
	CFOR_ALL_WORLD_ITEMS(wi) ++count;
	return count;
}


INT32 AddItemToWorld(INT16 sGridNo, const OBJECTTYPE* const pObject, const UINT8 ubLevel, const UINT16 usFlags, const INT8 bRenderZHeightAboveLevel, const INT8 bVisible)
{
	// ATE: Check if the gridno is OK
	if (sGridNo == NOWHERE)
	{
#ifdef JA2BETAVERSION
		// Display warning.....
		ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Error: Item %d was given invalid grid location %d. Please report", pObject->usItem, sGridNo);
#endif
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
	if( gWorldItems )
	{
		FOR_ALL_WORLD_ITEMS(wi)
		{
			RemoveItemFromPool(wi);
		}
		MemFree( gWorldItems );
		gWorldItems = NULL;
		guiNumWorldItems = 0;
	}
	if ( gWorldBombs )
	{
		MemFree( gWorldBombs );
		gWorldBombs = NULL;
		guiNumWorldBombs = 0;
	}
}


#ifdef JA2EDITOR

void SaveWorldItemsToMap( HWFILE fp )
{
	UINT32		uiActualNumWorldItems;


	uiActualNumWorldItems = GetNumUsedWorldItems( );

	FileWrite(fp, &uiActualNumWorldItems, 4);

	CFOR_ALL_WORLD_ITEMS(wi) FileWrite(fp, wi, sizeof(WORLDITEM));
}

#endif


static void DeleteWorldItemsBelongingToQueenIfThere(void);
static void DeleteWorldItemsBelongingToTerroristsWhoAreNotThere(void);


void LoadWorldItemsFromMap(HWFILE const f)
{
	// Start loading itmes...

	UINT32			i;
	WORLDITEM		dummyItem;
	UINT32			uiNumWorldItems;

	//If any world items exist, we must delete them now.
	TrashWorldItems();

	//Read the number of items that were saved in the map.
	FileRead(f, &uiNumWorldItems, sizeof(uiNumWorldItems));

	if( gTacticalStatus.uiFlags & LOADING_SAVED_GAME && !gfEditMode )
	{ //The sector has already been visited.  The items are saved in a different format that will be
		//loaded later on.  So, all we need to do is skip the data entirely.
		FileSeek(f, sizeof(WORLDITEM) * uiNumWorldItems, FILE_SEEK_FROM_CURRENT);
		return;
	}
	else for ( i = 0; i < uiNumWorldItems; i++ )
	{	//Add all of the items to the world indirectly through AddItemToPool, but only if the chance
		//associated with them succeed.
		FileRead(f, &dummyItem, sizeof(dummyItem));
		if( dummyItem.o.usItem == OWNERSHIP )
		{
			dummyItem.ubNonExistChance = 0;
		}
		if( gfEditMode || dummyItem.ubNonExistChance <= PreRandom( 100 ) )
		{
			if( !gfEditMode )
			{
				//check for matching item existance modes and only add if there is a match!
				if( dummyItem.usFlags & WORLD_ITEM_SCIFI_ONLY && !gGameOptions.fSciFi ||
					  dummyItem.usFlags & WORLD_ITEM_REALISTIC_ONLY && gGameOptions.fSciFi )
				{ //no match, so don't add item to world
					continue;
				}

				if ( !gGameOptions.fGunNut )
				{
					UINT16	usReplacement;

					// do replacements?
					if ( Item[ dummyItem.o.usItem ].usItemClass == IC_GUN )
					{
						INT8		bAmmo, bNewAmmo;

						usReplacement = StandardGunListReplacement( dummyItem.o.usItem );
						if ( usReplacement )
						{
							// everything else can be the same? no.
							bAmmo = dummyItem.o.ubGunShotsLeft;
							bNewAmmo = (Weapon[ usReplacement ].ubMagSize * bAmmo) / Weapon[ dummyItem.o.usItem ].ubMagSize;
							if ( bAmmo > 0 && bNewAmmo == 0 )
							{
								bNewAmmo = 1;
							}

							dummyItem.o.usItem = usReplacement;
							dummyItem.o.ubGunShotsLeft = bNewAmmo;
						}
					}
					if ( Item[ dummyItem.o.usItem ].usItemClass == IC_AMMO )
					{
						usReplacement = StandardGunListAmmoReplacement( dummyItem.o.usItem );
						if ( usReplacement )
						{
							UINT8		ubLoop;

							// go through status values and scale up/down
							for ( ubLoop = 0; ubLoop < dummyItem.o.ubNumberOfObjects; ubLoop++ )
							{
								dummyItem.o.bStatus[ ubLoop ] = dummyItem.o.bStatus[ ubLoop ] * Magazine[ Item[ usReplacement ].ubClassIndex ].ubMagSize / Magazine[ Item[ dummyItem.o.usItem ].ubClassIndex ].ubMagSize;
							}

							// then replace item #
							dummyItem.o.usItem = usReplacement;
						}
					}
				}
			}
			if (dummyItem.o.usItem == ACTION_ITEM)
			{ //if we are loading a pit, they are typically loaded without being armed.
				if( dummyItem.o.bActionValue == ACTION_ITEM_SMALL_PIT || dummyItem.o.bActionValue == ACTION_ITEM_LARGE_PIT )
				{
					dummyItem.usFlags &= ~WORLD_ITEM_ARMED_BOMB;
					dummyItem.bVisible = BURIED;
					dummyItem.o.bDetonatorType = 0;
				}
			}

			else if ( dummyItem.bVisible == HIDDEN_ITEM && dummyItem.o.bTrap > 0 && ( dummyItem.o.usItem == MINE || dummyItem.o.usItem == TRIP_FLARE || dummyItem.o.usItem == TRIP_KLAXON) )
			{
				ArmBomb( &dummyItem.o, BOMB_PRESSURE );
				dummyItem.usFlags |= WORLD_ITEM_ARMED_BOMB;
				// this is coming from the map so the enemy must know about it.
				gpWorldLevelData[ dummyItem.sGridNo ].uiFlags |= MAPELEMENT_ENEMY_MINE_PRESENT;

			}

			if ( dummyItem.usFlags & WORLD_ITEM_ARMED_BOMB )
			{ //all armed bombs are buried
				dummyItem.bVisible = BURIED;
			}
			INT32 const iItemIndex = AddItemToPool(dummyItem.sGridNo, &dummyItem.o, static_cast<Visibility>(dummyItem.bVisible), dummyItem.ubLevel, dummyItem.usFlags, dummyItem.bRenderZHeightAboveLevel);
			GetWorldItem(iItemIndex).ubNonExistChance = dummyItem.ubNonExistChance;
		}
	}

	if ( !gfEditMode )
	{
		DeleteWorldItemsBelongingToTerroristsWhoAreNotThere();
		if ( gWorldSectorX == 3 && gWorldSectorY == MAP_ROW_P && gbWorldSectorZ == 1 )
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
		CFOR_ALL_WORLD_ITEMS(wi)
		{
			// loop through all items, look for ownership
			if (wi->o.usItem != OWNERSHIP) continue;

			const ProfileID pid = wi->o.ubOwnerProfile;
			// if owner is a terrorist
			if (!IsProfileATerrorist(pid)) continue;

			MERCPROFILESTRUCT const& p = GetProfile(pid);
			// and they were not set in the current sector
			if (p.sSectorX == gWorldSectorX && p.sSectorY == gWorldSectorY) continue;

			// then all items in this location should be deleted
			const INT16 sGridNo = wi->sGridNo;
			const UINT8 ubLevel = wi->ubLevel;
			FOR_ALL_WORLD_ITEMS(owned_item)
			{
				if (owned_item->sGridNo == sGridNo && owned_item->ubLevel == ubLevel)
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

	if (q.sSectorX != gWorldSectorX ||
			q.sSectorY != gWorldSectorY ||
			q.bSectorZ != gbWorldSectorZ)
	{
		return;
	}

	CFOR_ALL_WORLD_ITEMS(wi)
	{
		// Look for items belonging to the queen
		if (wi->o.usItem         != OWNERSHIP) continue;
		if (wi->o.ubOwnerProfile != QUEEN)     continue;

		// Delete all items on this tile
		const INT16 sGridNo = wi->sGridNo;
		const UINT8 ubLevel = wi->ubLevel;
		FOR_ALL_WORLD_ITEMS(item)
		{
			if (item->sGridNo != sGridNo) continue;
			if (item->ubLevel != ubLevel) continue;

			// Upgrade equipment
			switch (item->o.usItem)
			{
				case AUTO_ROCKET_RIFLE:
				{
					// Give her auto rifle
					const INT8 bSlot = FindObjectInSoldierProfile(QUEEN, ROCKET_RIFLE);
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
void RefreshWorldItemsIntoItemPools(const WORLDITEM* const items, const INT32 item_count)
{
	for (const WORLDITEM* i = items; i != items + item_count; ++i)
	{
		if (!i->fExists) continue;
		OBJECTTYPE o = i->o; // XXX AddItemToPool() may alter the object
		AddItemToPool(i->sGridNo, &o, static_cast<Visibility>(i->bVisible), i->ubLevel, i->usFlags, i->bRenderZHeightAboveLevel);
	}
}
