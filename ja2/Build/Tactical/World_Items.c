#include "Items.h"
#include "Handle_Items.h"
#include "Overhead.h"
#include "Weapons.h"
#include "Points.h"
#include "TileDef.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "Interface.h"
#include "Animation_Control.h"
#include "Font_Control.h"
#include "Render_Dirty.h"
#include "World_Items.h"
#include "Debug.h"
#include "Isometric_Utils.h"
#include "Sys_Globals.h"
#include "Tactical_Save.h"
#include "StrategicMap.h"
#include "Campaign_Types.h"
#include "Random.h"
#include "Action_Items.h"
#include "Pits.h"
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

extern UINT16 StandardGunListAmmoReplacement( UINT16 usAmmo );


static INT32 GetFreeWorldBombIndex(void)
{
	UINT32 uiCount;
	WORLDBOMB *newWorldBombs;
	UINT32	uiOldNumWorldBombs;

	for(uiCount=0; uiCount < guiNumWorldBombs; uiCount++)
	{
		if ( gWorldBombs[ uiCount ].fExists == FALSE )
			return( (INT32)uiCount );
	}

	uiOldNumWorldBombs = guiNumWorldBombs;
	guiNumWorldBombs += 10;
	//Allocate new table with max+10 items.
	newWorldBombs = (WORLDBOMB*)MemRealloc( gWorldBombs, sizeof( WORLDBOMB ) * guiNumWorldBombs );
	if (newWorldBombs == NULL)
	{
		return( -1 );
	}

	//Clear the rest of the new array
	memset( &newWorldBombs[ uiOldNumWorldBombs ], 0,
		sizeof( WORLDBOMB ) * ( guiNumWorldBombs - uiOldNumWorldBombs ) );
	gWorldBombs = newWorldBombs;

	// Return uiCount.....
	return( uiCount );
}


static UINT32 GetNumUsedWorldBombs(void)
{
	UINT32 uiCount, uiNumItems;
	uiNumItems = 0;

	if ( guiNumWorldBombs == 0 )
	{
		return( 0 );
	}

	for( uiCount = 0; uiCount < guiNumWorldBombs; uiCount++ )
	{
		if( gWorldBombs[ uiCount ].fExists )
		{
			uiNumItems++;
		}
	}

	return( uiNumItems );
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


static void RemoveBombFromWorld(INT32 iBombIndex)
{
	//Remove the world bomb from the table.
	gWorldBombs[ iBombIndex ].fExists										= FALSE;
}


static void RemoveBombFromWorldByItemIndex(INT32 iItemIndex)
{
	// Find the world bomb which corresponds with a particular world item, then
	// remove the world bomb from the table.
	UINT32	uiBombIndex;

	for (uiBombIndex = 0; uiBombIndex < guiNumWorldBombs; uiBombIndex++)
	{
		if (gWorldBombs[ uiBombIndex ].fExists && gWorldBombs[ uiBombIndex].iItemIndex == iItemIndex )
		{
			RemoveBombFromWorld( uiBombIndex );
			return;
		}
	}
}

INT32 FindWorldItemForBombInGridNo( INT16 sGridNo, INT8 bLevel )
{
	UINT32					uiBombIndex;

	for (uiBombIndex = 0; uiBombIndex < guiNumWorldBombs; uiBombIndex++)
	{
		if (gWorldBombs[ uiBombIndex ].fExists && gWorldItems[ gWorldBombs[ uiBombIndex ].iItemIndex ].sGridNo == sGridNo && gWorldItems[ gWorldBombs[ uiBombIndex ].iItemIndex ].ubLevel == bLevel )
		{
			return( gWorldBombs[ uiBombIndex ].iItemIndex );
		}
	}
	return( -1 );
}


void FindPanicBombsAndTriggers( void )
{
	// This function searches the bomb table to find panic-trigger-tuned bombs and triggers

	UINT32					uiBombIndex;
	OBJECTTYPE *		pObj;
	STRUCTURE *			pSwitch;
	INT16						sGridNo = NOWHERE;
	INT8						bPanicIndex;

	for (uiBombIndex = 0; uiBombIndex < guiNumWorldBombs; uiBombIndex++)
	{
		if (gWorldBombs[ uiBombIndex ].fExists)
		{
			pObj = &(gWorldItems[ gWorldBombs[ uiBombIndex ].iItemIndex ].o);
			if (pObj->bFrequency == PANIC_FREQUENCY || pObj->bFrequency == PANIC_FREQUENCY_2 || pObj->bFrequency == PANIC_FREQUENCY_3 )
			{
				if (pObj->usItem == SWITCH)
				{
					sGridNo = gWorldItems[ gWorldBombs[ uiBombIndex ].iItemIndex ].sGridNo;
					switch( pObj->bFrequency )
					{
						case PANIC_FREQUENCY:
							bPanicIndex = 0;
							break;

						case PANIC_FREQUENCY_2:
							bPanicIndex = 1;
							break;

						case PANIC_FREQUENCY_3:
							bPanicIndex = 2;
							break;

						default:
							// augh!!!
							continue;
					}

					pSwitch = FindStructure( sGridNo, STRUCTURE_SWITCH );
					if (pSwitch)
					{
						switch( pSwitch->ubWallOrientation )
						{
							case INSIDE_TOP_LEFT:
							case OUTSIDE_TOP_LEFT:
								sGridNo += DirectionInc( SOUTH );
								break;
							case INSIDE_TOP_RIGHT:
							case OUTSIDE_TOP_RIGHT:
								sGridNo += DirectionInc( EAST );
								break;
							default:
								break;
						}
					}

					gTacticalStatus.sPanicTriggerGridNo[ bPanicIndex ] = sGridNo;
					gTacticalStatus.ubPanicTolerance[ bPanicIndex ] = pObj->ubTolerance;
					if (pObj->fFlags & OBJECT_ALARM_TRIGGER)
					{
						gTacticalStatus.bPanicTriggerIsAlarm[ bPanicIndex ] = TRUE;
					}
					gTacticalStatus.fPanicFlags |= PANIC_TRIGGERS_HERE;
					bPanicIndex++;
					if (bPanicIndex == NUM_PANIC_TRIGGERS)
					{
						return;
					}
				}
				else
				{
					gTacticalStatus.fPanicFlags |= PANIC_BOMBS_HERE;
				}
			}
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
		if ( gWorldItems[ uiCount ].fExists == FALSE )
			return( (INT32)uiCount );
	}

	uiOldNumWorldItems = guiNumWorldItems;
	guiNumWorldItems += 10;
	//Allocate new table with max+10 items.
	newWorldItems = (WORLDITEM*)MemRealloc( gWorldItems, sizeof( WORLDITEM ) * guiNumWorldItems );
	if (newWorldItems == NULL)
	{
		return( -1 );
	}

	//Clear the rest of the new array
	memset( &newWorldItems[ uiOldNumWorldItems ], 0,
		sizeof( WORLDITEM ) * ( guiNumWorldItems - uiOldNumWorldItems ) );
	gWorldItems = newWorldItems;

	// Return uiCount.....
	return( uiCount );
}


static UINT32 GetNumUsedWorldItems(void)
{
	UINT32 uiCount, uiNumItems;
	uiNumItems = 0;

	if ( guiNumWorldItems == 0 )
	{
		return( 0 );
	}

	for( uiCount = 0; uiCount < guiNumWorldItems; uiCount++ )
	{
		if( gWorldItems[ uiCount ].fExists )
		{
			uiNumItems++;
		}
	}

	return( uiNumItems );
}



INT32 AddItemToWorld( INT16 sGridNo, OBJECTTYPE *pObject, UINT8 ubLevel, UINT16 usFlags, INT8 bRenderZHeightAboveLevel, INT8 bVisible )
{
	UINT32	iItemIndex;
	INT32		iReturn;

	// ATE: Check if the gridno is OK
	if ( (sGridNo) == NOWHERE )
	{
		// Display warning.....
		#ifdef JA2BETAVERSION
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Error: Item %d was given invalid grid location %d. Please report", pObject->usItem, sGridNo);
		#endif
		return( -1 );
	}

	iItemIndex = GetFreeWorldItemIndex( );

	//Add the new world item to the table.
	gWorldItems[ iItemIndex ].fExists										= TRUE;
	gWorldItems[ iItemIndex ].sGridNo										= sGridNo;
	gWorldItems[ iItemIndex ].ubLevel										= ubLevel;
	gWorldItems[ iItemIndex ].usFlags										= usFlags;
	gWorldItems[ iItemIndex ].bVisible									= bVisible;
	gWorldItems[ iItemIndex ].bRenderZHeightAboveLevel  = bRenderZHeightAboveLevel;

	memcpy( &(gWorldItems[ iItemIndex ].o), pObject, sizeof( OBJECTTYPE ) );

	// Add a bomb reference if needed
	if (usFlags & WORLD_ITEM_ARMED_BOMB)
	{
		iReturn = AddBombToWorld( iItemIndex );
		if (iReturn == -1)
		{
			return( -1 );
		}
	}

	return ( iItemIndex );
}

void RemoveItemFromWorld( INT32 iItemIndex )
{
	// Ensure the item still exists, then if it's a bomb,
	// remove the appropriate entry from the bomb table
	if (gWorldItems[ iItemIndex ].fExists)
	{
		if (gWorldItems[ iItemIndex ].usFlags & WORLD_ITEM_ARMED_BOMB)
		{
			RemoveBombFromWorldByItemIndex( iItemIndex );
		}
		gWorldItems[ iItemIndex ].fExists = FALSE;
	}
}

void TrashWorldItems()
{
	UINT32 i;
	if( gWorldItems )
	{
		for( i = 0; i < guiNumWorldItems; i++ )
		{
			if( gWorldItems[ i ].fExists )
			{
				RemoveItemFromPool( gWorldItems[ i ].sGridNo, i, gWorldItems[ i ].ubLevel );
			}
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


void SaveWorldItemsToMap( HWFILE fp )
{
	UINT32 i;
	UINT32		uiActualNumWorldItems;


	uiActualNumWorldItems = GetNumUsedWorldItems( );

	FileWrite(fp, &uiActualNumWorldItems, 4);

	for( i = 0; i < guiNumWorldItems; i++ )
	{
		if( gWorldItems[ i ].fExists )
			FileWrite(fp, &gWorldItems[i], sizeof(WORLDITEM));
	}
}


static void DeleteWorldItemsBelongingToQueenIfThere(void);
static void DeleteWorldItemsBelongingToTerroristsWhoAreNotThere(void);


void LoadWorldItemsFromMap( INT8 **hBuffer )
{
	// Start loading itmes...

	UINT32			i;
	WORLDITEM		dummyItem;
	INT32				iItemIndex;
	UINT32			uiNumWorldItems;

	//If any world items exist, we must delete them now.
	TrashWorldItems();

	//Read the number of items that were saved in the map.
	LOADDATA( &uiNumWorldItems, *hBuffer, 4 );

	if( gTacticalStatus.uiFlags & LOADING_SAVED_GAME && !gfEditMode )
	{ //The sector has already been visited.  The items are saved in a different format that will be
		//loaded later on.  So, all we need to do is skip the data entirely.
		*hBuffer += sizeof( WORLDITEM ) * uiNumWorldItems;
		return;
	}
	else for ( i = 0; i < uiNumWorldItems; i++ )
	{	//Add all of the items to the world indirectly through AddItemToPool, but only if the chance
		//associated with them succeed.
		LOADDATA( &dummyItem, *hBuffer, sizeof( WORLDITEM ) );
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
			if( dummyItem.o.usItem == ACTION_ITEM && gfLoadPitsWithoutArming )
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
			AddItemToPoolAndGetIndex( dummyItem.sGridNo, &dummyItem.o, dummyItem.bVisible, dummyItem.ubLevel, dummyItem.usFlags, dummyItem.bRenderZHeightAboveLevel, &iItemIndex );
			gWorldItems[ iItemIndex ].ubNonExistChance = dummyItem.ubNonExistChance;
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
	UINT32	uiLoop;
	UINT32	uiLoop2;
	INT16		sGridNo;
	UINT8		ubLevel;

	// only do this after Carmen has talked to player and terrorists have been placed
	//if ( CheckFact( FACT_CARMEN_EXPLAINED_DEAL, 0 ) == TRUE )
	{

		for ( uiLoop = 0; uiLoop < guiNumWorldItems; uiLoop++ )
		{
			// loop through all items, look for ownership
			if ( gWorldItems[ uiLoop ].fExists && gWorldItems[ uiLoop ].o.usItem == OWNERSHIP )
			{
				// if owner is a terrorist
				if ( IsProfileATerrorist( gWorldItems[ uiLoop ].o.ubOwnerProfile ) )
				{
					// and they were not set in the current sector
					if ( gMercProfiles[ gWorldItems[ uiLoop ].o.ubOwnerProfile ].sSectorX != gWorldSectorX ||
						gMercProfiles[ gWorldItems[ uiLoop ].o.ubOwnerProfile ].sSectorY != gWorldSectorY )
					{
						// then all items in this location should be deleted
						sGridNo = gWorldItems[ uiLoop ].sGridNo;
						ubLevel = gWorldItems[ uiLoop ].ubLevel;
						for ( uiLoop2 = 0; uiLoop2 < guiNumWorldItems; uiLoop2++ )
						{
							// loop through all items, look for ownership
							if ( gWorldItems[ uiLoop2 ].fExists && gWorldItems[ uiLoop2 ].sGridNo == sGridNo && gWorldItems[ uiLoop2 ].ubLevel == ubLevel )
							{
								RemoveItemFromPool( sGridNo, uiLoop2, ubLevel );
							}
						}
					}
				}
			}
		}
	}
	// else the terrorists haven't been placed yet!
}


static void DeleteWorldItemsBelongingToQueenIfThere(void)
{
	UINT32	uiLoop;
	UINT32	uiLoop2;
	INT16		sGridNo;
	UINT8		ubLevel;
	INT8		bSlot;

	if ( gMercProfiles[ QUEEN ].sSectorX == gWorldSectorX &&
		gMercProfiles[ QUEEN ].sSectorY == gWorldSectorY &&
		gMercProfiles[ QUEEN ].bSectorZ == gbWorldSectorZ )
	{

		for ( uiLoop = 0; uiLoop < guiNumWorldItems; uiLoop++ )
		{
			// loop through all items, look for ownership
			if ( gWorldItems[ uiLoop ].fExists && gWorldItems[ uiLoop ].o.usItem == OWNERSHIP )
			{
				// if owner is the Queen
				if ( gWorldItems[ uiLoop ].o.ubOwnerProfile == QUEEN )
				{
					// then all items in this location should be deleted
					sGridNo = gWorldItems[ uiLoop ].sGridNo;
					ubLevel = gWorldItems[ uiLoop ].ubLevel;
					for ( uiLoop2 = 0; uiLoop2 < guiNumWorldItems; uiLoop2++ )
					{
						// loop through all items, look for those in same tile
						if ( gWorldItems[ uiLoop2 ].fExists && gWorldItems[ uiLoop2 ].sGridNo == sGridNo && gWorldItems[ uiLoop2 ].ubLevel == ubLevel )
						{
							// upgrade equipment
							switch ( gWorldItems[ uiLoop2 ].o.usItem )
							{
								case AUTO_ROCKET_RIFLE:
									bSlot = FindObjectInSoldierProfile( QUEEN, ROCKET_RIFLE );
									if ( bSlot != NO_SLOT )
									{
										// give her auto rifle
										gMercProfiles[ QUEEN ].inv[ bSlot ] = AUTO_ROCKET_RIFLE;
									}
									break;
								case SPECTRA_HELMET_18:
									gMercProfiles[ QUEEN ].inv[ HELMETPOS ] = SPECTRA_HELMET_18;
									break;
								case SPECTRA_VEST_18:
									gMercProfiles[ QUEEN ].inv[ VESTPOS ] = SPECTRA_VEST_18;
									break;
								case SPECTRA_LEGGINGS_18:
									gMercProfiles[ QUEEN ].inv[ LEGPOS ] = SPECTRA_LEGGINGS_18;
									break;
								default:
									break;
							}
							RemoveItemFromPool( sGridNo, uiLoop2, ubLevel );
						}
					}
				}
			}
		}
	}
}


// Refresh item pools
void RefreshWorldItemsIntoItemPools( WORLDITEM * pItemList, INT32 iNumberOfItems )
{
	INT32			i;
	WORLDITEM		dummyItem;

	for ( i = 0; i < iNumberOfItems; i++ )
	{
		if( pItemList[ i ].fExists )
		{
			memcpy( &dummyItem, &( pItemList[ i ] ), sizeof( WORLDITEM ) );

			AddItemToPool( dummyItem.sGridNo, &dummyItem.o, dummyItem.bVisible, dummyItem.ubLevel, dummyItem.usFlags, dummyItem.bRenderZHeightAboveLevel );
		}
	}

}
