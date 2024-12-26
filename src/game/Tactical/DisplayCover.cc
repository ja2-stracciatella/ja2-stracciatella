#include "DisplayCover.h"
#include "Font_Control.h"
#include "Isometric_Utils.h"
#include "Overhead.h"
#include "Message.h"
#include "Soldier_Find.h"
#include "TileDat.h"
#include "GameSettings.h"
#include "RenderWorld.h"
#include "Interface.h"
#include "Debug.h"
#include "PathAI.h"
#include "WorldMan.h"
#include "OppList.h"
#include "LOS.h"
#include "Weapons.h"
#include "Game_Clock.h"
#include "Animation_Control.h"
#include "Text.h"
#include "StrategicMap.h"
#include "Render_Fun.h"
#include "Items.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "WeaponModels.h"

#include <algorithm>

#define DC_MAX_COVER_RANGE		31

#define DC__SOLDIER_VISIBLE_RANGE	31

#define DC__MIN_SIZE			4
#define DC__MAX_SIZE			11

struct BEST_COVER_STRUCT
{
	INT16 sGridNo;
	INT8  bCover; //% chance that the gridno is fully covered.  ie 100 if safe, 0  is has no cover
};


struct VISIBLE_TO_SOLDIER_STRUCT
{
	INT16   sGridNo;
	INT8    bVisibleToSoldier;
	BOOLEAN fRoof;
};


enum
{
	DC__SEE_NO_STANCES,
	DC__SEE_1_STANCE,
	DC__SEE_2_STANCE,
	DC__SEE_3_STANCE
};


static BEST_COVER_STRUCT gCoverRadius[DC_MAX_COVER_RANGE][DC_MAX_COVER_RANGE];
static INT16             gsLastCoverGridNo   = NOWHERE;
static INT16             gsLastSoldierGridNo = NOWHERE;
static INT8              gbLastStance        = -1;


static VISIBLE_TO_SOLDIER_STRUCT gVisibleToSoldierStruct[DC__SOLDIER_VISIBLE_RANGE][DC__SOLDIER_VISIBLE_RANGE];
static INT16                     gsLastVisibleToSoldierGridNo = NOWHERE;


static void AddCoverTileToEachGridNo(void);
static void CalculateCoverInRadiusAroundGridno(INT16 sTargetGridNo, int bSearchRange);
static INT8 GetCurrentMercForDisplayCoverStance(void);


void DisplayCoverOfSelectedGridNo()
{
	SOLDIERTYPE const* const sel = GetSelectedMan();
	//Only allowed in if there is someone selected
	if (!sel) return;

	//if the cursor is in a the tactical map
	GridNo const sGridNo = guiCurrentCursorGridNo;
	if (sGridNo == NOWHERE) return;

	INT8 const bStance = GetCurrentMercForDisplayCoverStance();

	//if the gridno is different then the last one that was displayed
	if (gsLastCoverGridNo == sGridNo &&
		gbLastStance == bStance &&
		gsLastCoverGridNo == sel->sGridNo)
	{
		return;
	}

	//if the cover is currently being displayed
	if (gsLastCoverGridNo != NOWHERE ||
		gbLastStance != -1 ||
		gsLastSoldierGridNo != NOWHERE)
	{
		//remove the gridnos
		RemoveCoverOfSelectedGridNo();
	}
	else
	{
		//if it is the first time in here

		//pop up a message to say we are in the display cover routine
		ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, zNewTacticalMessages[TCTL_MSG__DISPLAY_COVER]);
	}

	gbLastStance        = bStance;
	gsLastCoverGridNo   = sGridNo;
	gsLastSoldierGridNo = sel->sGridNo;

	//Fill the array of gridno and cover values
	CalculateCoverInRadiusAroundGridno(sGridNo, gGameSettings.ubSizeOfDisplayCover);

	//Add the graphics to each gridno
	AddCoverTileToEachGridNo();

	// Re-render the scene!
	SetRenderFlags(RENDER_FLAG_FULL);
}


static void AddCoverObjectToWorld(INT16 sGridNo, UINT16 usGraphic, BOOLEAN fRoof);


static void AddCoverTileToEachGridNo(void)
{
	BOOLEAN const roof = gsInterfaceLevel != I_GROUND_LEVEL;
	for (UINT32 y = 0; y < DC_MAX_COVER_RANGE; ++y)
	{
		for (UINT32 x = 0; x < DC_MAX_COVER_RANGE; ++x)
		{
			BEST_COVER_STRUCT const& cr    = gCoverRadius[x][y];
			INT8              const  cover = cr.bCover;
			if (cover == -1) continue; // Valid cover?
			Assert(0 <= cover && cover <= 100);

			UINT16 const gfx =
				cover <= 20 ? SPECIALTILE_COVER_1 :
				cover <= 40 ? SPECIALTILE_COVER_2 :
				cover <= 60 ? SPECIALTILE_COVER_3 :
				cover <= 80 ? SPECIALTILE_COVER_4 :
						SPECIALTILE_COVER_5;
			AddCoverObjectToWorld(cr.sGridNo, gfx, roof);
		}
	}
}


static void RemoveCoverObjectFromWorld(INT16 sGridNo, UINT16 usGraphic, BOOLEAN fRoof);


void RemoveCoverOfSelectedGridNo()
{
	if (gsLastCoverGridNo == NOWHERE) return;

	BOOLEAN const roof = gsInterfaceLevel != I_GROUND_LEVEL;
	for (UINT32 y = 0; y < DC_MAX_COVER_RANGE; ++y)
	{
		for (UINT32 x = 0; x < DC_MAX_COVER_RANGE; ++x)
		{
			BEST_COVER_STRUCT const& cr    = gCoverRadius[x][y];
			INT8              const  cover = cr.bCover;
			if (cover == -1) continue; // Valid cover?
			Assert(0 <= cover && cover <= 100);

			UINT16 const gfx =
				cover <= 20 ? SPECIALTILE_COVER_1 :
				cover <= 40 ? SPECIALTILE_COVER_2 :
				cover <= 60 ? SPECIALTILE_COVER_3 :
				cover <= 80 ? SPECIALTILE_COVER_4 :
						SPECIALTILE_COVER_5;
			RemoveCoverObjectFromWorld(cr.sGridNo, gfx, roof);
		}
	}

	// Re-render the scene!
	SetRenderFlags(RENDER_FLAG_FULL);

	gsLastCoverGridNo   = NOWHERE;
	gbLastStance        = -1;
	gsLastSoldierGridNo = NOWHERE;
}


static INT8 CalcCoverForGridNoBasedOnTeamKnownEnemies(const SOLDIERTYPE* pSoldier, INT16 sTargetGridNo, INT8 bStance);
static SOLDIERTYPE* GetCurrentMercForDisplayCover(void);


static void CalculateCoverInRadiusAroundGridno(INT16 const sTargetGridNo, int search_range)
{
	//clear out the array first
	for (INT16 y = 0; y < DC_MAX_COVER_RANGE; ++y)
	{
		for (INT16 x = 0; x < DC_MAX_COVER_RANGE; ++x)
		{
			BEST_COVER_STRUCT& cr = gCoverRadius[x][y];
			cr.sGridNo = -1;
			cr.bCover  = -1;
		}
	}

	if (search_range > DC_MAX_COVER_RANGE / 2) search_range = DC_MAX_COVER_RANGE / 2;

	// Determine maximum horizontal and vertical limits
	INT16 const max_left  = std::min(search_range,              sTargetGridNo % MAXCOL);
	INT16 const max_right = std::min(search_range, MAXCOL - 1 - sTargetGridNo % MAXCOL);
	INT16 const max_up    = std::min(search_range,              sTargetGridNo / MAXROW);
	INT16 const max_down  = std::min(search_range, MAXROW - 1 - sTargetGridNo / MAXROW);

	// Find out which tiles around the location are reachable
	LocalReachableTest(sTargetGridNo, search_range);

	SOLDIERTYPE const* const pSoldier = GetCurrentMercForDisplayCover();

	INT16 x = 0;
	INT16 y = 0;

	// Determine the stance to use
	INT8 const stance = GetCurrentMercForDisplayCoverStance();

	//loop through all the gridnos that we are interested in
	for (INT16 sYOffset = -max_up; sYOffset <= max_down; ++sYOffset)
	{
		for (INT16 sXOffset = -max_left; sXOffset <= max_right; ++sXOffset)
		{
			INT16 const sGridNo = sTargetGridNo + sXOffset + (MAXCOL * sYOffset);

			gCoverRadius[x][y].sGridNo = sGridNo;

			if (!GridNoOnScreen(sGridNo)) continue;

			//if we are to display cover for the roofs, and there is a roof above us
			if (gsInterfaceLevel == I_ROOF_LEVEL && !FlatRoofAboveGridNo(sGridNo))
			{
				continue;
			}

			if (!(gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REACHABLE))
			{
				//skip to the next gridno
				++x;
				continue;
			}

			// if someone (visible) is there, skip
			//Check both bottom level, and top level
			SOLDIERTYPE const* tgt = WhoIsThere2(sGridNo, 0);
			if (!tgt) tgt = WhoIsThere2(sGridNo, 1);
			//if someone is here, and they are an enemy, skip over them
			if (tgt && tgt->bVisible == true && tgt->bTeam != pSoldier->bTeam)
			{
				continue;
			}

			//Calculate the cover for this gridno
			gCoverRadius[x][y].bCover = CalcCoverForGridNoBasedOnTeamKnownEnemies(pSoldier, sGridNo, stance);
			++x;
		}
		++y;
		x = 0;
	}
}


static INT8 CalcCoverForGridNoBasedOnTeamKnownEnemies(SOLDIERTYPE const* const pSoldier, INT16 const sTargetGridNo, INT8 const bStance)
{
	// loop through all the enemies and determine the cover
	INT32 iTotalCoverPoints = 0;
	INT8  bNumEnemies       = 0;
	INT32 iHighestValue     = 0;
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const pOpponent = *i;

		if (pOpponent->bLife < OKLIFE) continue;

		// if this man is neutral / on the same side, he's not an opponent
		if (CONSIDERED_NEUTRAL(pSoldier, pOpponent)) continue;
		if (pSoldier->bSide == pOpponent->bSide)     continue;

		INT8 const* const pbPersOL = pSoldier->bOppList        + pOpponent->ubID;
		INT8 const* const pbPublOL = gbPublicOpplist[OUR_TEAM] + pOpponent->ubID;

		// if this opponent is unknown personally and publicly
		if (*pbPersOL != SEEN_CURRENTLY && *pbPersOL != SEEN_THIS_TURN &&
			*pbPublOL != SEEN_CURRENTLY && *pbPublOL != SEEN_THIS_TURN)
		{
			continue;
		}

		UINT16 const usRange      = GetRangeInCellCoordsFromGridNoDiff(pOpponent->sGridNo, sTargetGridNo);
		UINT16 const usSightLimit = DistanceVisible(pOpponent, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sTargetGridNo, pSoldier->bLevel);

		if (usRange > usSightLimit * CELL_X_SIZE) continue;

		// if actual LOS check fails, then chance to hit is 0, ignore this guy
		if (SoldierToVirtualSoldierLineOfSightTest(pOpponent, sTargetGridNo, pSoldier->bLevel, bStance, usSightLimit, true) == 0)
		{
			continue;
		}

		INT32  const iGetThrough = SoldierToLocationChanceToGetThrough(pOpponent, sTargetGridNo,
										pSoldier->bLevel, bStance, NULL);
		UINT16 const usMaxRange = WeaponInHand(pOpponent) ? GunRange(pOpponent->inv[HANDPOS]) :
						GCM->getWeapon(GLOCK_18)->usRange;
		INT32  const iBulletGetThrough = std::clamp(int(((usMaxRange - usRange) / (float)usMaxRange + .3) * 100), 0, 100);
		if (iBulletGetThrough > 5 && iGetThrough > 0)
		{
			INT32 const iCover = iGetThrough * iBulletGetThrough / 100;
			if (iHighestValue < iCover) iHighestValue = iCover;

			iTotalCoverPoints += iCover;
			++bNumEnemies;
		}
	}

	INT8 bPercentCoverForGridno;
	if (bNumEnemies == 0)
	{
		bPercentCoverForGridno = 100;
	}
	else
	{
		bPercentCoverForGridno = iTotalCoverPoints / bNumEnemies;
		INT32 const iTemp = bPercentCoverForGridno - (iHighestValue / bNumEnemies) + iHighestValue;
		bPercentCoverForGridno = std::clamp(100 - iTemp, 0, 100);
	}
	return bPercentCoverForGridno;
}


static void AddCoverObjectToWorld(INT16 const sGridNo, UINT16 const usGraphic, BOOLEAN const fRoof)
{
	LEVELNODE* const n = fRoof ?
		AddOnRoofToHead(sGridNo, usGraphic) :
		AddObjectToHead(sGridNo, usGraphic);

	n->uiFlags |= LEVELNODE_REVEAL;

	if (NightTime())
	{
		n->ubShadeLevel        = DEFAULT_SHADE_LEVEL;
		n->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
	}
}


static void RemoveCoverObjectFromWorld(INT16 sGridNo, UINT16 usGraphic, BOOLEAN fRoof)
{
	if( fRoof )
	{
		RemoveOnRoof( sGridNo, usGraphic );
	}
	else
	{
		RemoveObject( sGridNo, usGraphic );
	}
}


static SOLDIERTYPE* GetCurrentMercForDisplayCover(void)
{
	return GetSelectedMan();
}


static INT8 GetCurrentMercForDisplayCoverStance(void)
{
	const SOLDIERTYPE* const pSoldier = GetCurrentMercForDisplayCover();
	INT8  bStance;

	switch( pSoldier->usUIMovementMode )
	{
		case PRONE:
		case CRAWLING:
			bStance = ANIM_PRONE;
			break;

		case KNEEL_DOWN:
		case SWATTING:
		case CROUCHING:
			bStance = ANIM_CROUCH;
			break;

		case WALKING:
		case RUNNING:
		case STANDING:
			bStance = ANIM_STAND;
			break;

		default:
			bStance = ANIM_CROUCH;
			break;
	}

	return( bStance );
}


void DisplayRangeToTarget(SOLDIERTYPE* s, INT16 const sTargetGridNo)
{
	if (sTargetGridNo == NOWHERE || sTargetGridNo == 0) return;

	// Get the range to the target location
	UINT16 const usRange = GetRangeInCellCoordsFromGridNoDiff(s->sGridNo, sTargetGridNo) / 10;

	if (WeaponInHand(s))
	{
		//display a string with the weapons range, then range to target
		ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE,
				st_format_printf(zNewTacticalMessages[TCTL_MSG__RANGE_TO_TARGET_AND_GUN_RANGE],
				GCM->getWeapon(s->inv[HANDPOS].usItem)->usRange / 10, usRange));
	}
	else
	{
		//display a string with the range to target
		ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE,
				st_format_printf(zNewTacticalMessages[TCTL_MSG__RANGE_TO_TARGET], usRange));
	}

	//if the target is out of the mercs gun range or knife
	if (!InRange(s, sTargetGridNo))
	{
		UINT16 const item_class = GCM->getItem(s->inv[HANDPOS].usItem)->getItemClass();
		if (item_class == IC_GUN || item_class == IC_THROWING_KNIFE)
		{
			// Display a warning saying so
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[OUT_OF_RANGE_STRING]);
		}
	}
}

static void AddVisibleToSoldierToEachGridNo(void);
static void CalculateVisibleToSoldierAroundGridno(INT16 sTargetGridNo, int bSearchRange);


void DisplayGridNoVisibleToSoldierGrid( )
{
	//INT8 bStance;

	const SOLDIERTYPE* const sel = GetSelectedMan();
	//Only allowed in if there is someone selected
	if (sel == NULL)
		return;

	//if the cursor is in a the tactical map
	const GridNo sGridNo = guiCurrentCursorGridNo;
	if (sGridNo != NOWHERE)
	{
		//if the gridno is different then the last one that was displayed
		if (sGridNo != gsLastVisibleToSoldierGridNo || sel->sGridNo != gsLastSoldierGridNo)
		{
			//if the cover is currently being displayed
			if( gsLastVisibleToSoldierGridNo != NOWHERE || gsLastSoldierGridNo != NOWHERE )
			{
				//remove the gridnos
				RemoveVisibleGridNoAtSelectedGridNo();
			}
			else
			{
				//pop up a message to say we are in the display cover routine
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, zNewTacticalMessages[ TCTL_MSG__LOS ] );
				//increment the display LOS counter ( just seeing how many times people use it )
				//gJa25SaveStruct.uiDisplayLosCounter++;
			}

			gsLastVisibleToSoldierGridNo = sGridNo;
			gsLastSoldierGridNo = sel->sGridNo;


			//Fill the array of gridno and cover values
			CalculateVisibleToSoldierAroundGridno( sGridNo, gGameSettings.ubSizeOfLOS );

			//Add the graphics to each gridno
			AddVisibleToSoldierToEachGridNo();

			// Re-render the scene!
			SetRenderFlags( RENDER_FLAG_FULL );
		}
	}
}


static INT8 CalcIfSoldierCanSeeGridNo(const SOLDIERTYPE* pSoldier, INT16 sTargetGridNo, BOOLEAN fRoof);
static BOOLEAN IsTheRoofVisible(INT16 sGridNo);


static void CalculateVisibleToSoldierAroundGridno(INT16 sTargetGridNo, int bSearchRange)
{
	INT16   sMaxLeft, sMaxRight, sMaxUp, sMaxDown, sXOffset, sYOffset;
	INT16   sGridNo;
	INT16   sCounterX, sCounterY;
	BOOLEAN fRoof=false;

	//clear out the struct
	for (int i = 0; i < DC__SOLDIER_VISIBLE_RANGE; ++i)
	{
		std::fill_n(gVisibleToSoldierStruct[i], DC__SOLDIER_VISIBLE_RANGE, VISIBLE_TO_SOLDIER_STRUCT{});
	}

	if( bSearchRange > ( DC_MAX_COVER_RANGE / 2 ) )
		bSearchRange = ( DC_MAX_COVER_RANGE / 2 );


	// determine maximum horizontal limits
	sMaxLeft  = std::min(bSearchRange, sTargetGridNo % MAXCOL);
	sMaxRight = std::min(bSearchRange, MAXCOL - ((sTargetGridNo % MAXCOL) + 1));

	// determine maximum vertical limits
	sMaxUp   = std::min(bSearchRange, (sTargetGridNo / MAXROW));
	sMaxDown = std::min(bSearchRange, MAXROW - ((sTargetGridNo / MAXROW) + 1));

	const SOLDIERTYPE* const pSoldier = GetCurrentMercForDisplayCover();

	sCounterX=0;
	sCounterY=0;

	//loop through all the gridnos that we are interested in
	for (sYOffset = -sMaxUp; sYOffset <= sMaxDown; sYOffset++)
	{
		sCounterX = 0;
		for (sXOffset = -sMaxLeft; sXOffset <= sMaxRight; sXOffset++)
		{
			sGridNo = sTargetGridNo + sXOffset + (MAXCOL * sYOffset);
			fRoof = false;

			//record the gridno
			gVisibleToSoldierStruct[ sCounterX ][ sCounterY ].sGridNo = sGridNo;

			//if the gridno is NOT on screen
			if( !GridNoOnScreen( sGridNo ) )
			{
				continue;
			}

			//is there a roof above this gridno
			if( FlatRoofAboveGridNo( sGridNo ) )
			{
				if (IsTheRoofVisible(sGridNo) && gWorldSector.z == 0)
				{
					fRoof = true;
				}

				//if wer havent explored the area yet and we are underground, dont show cover
				else if (!(gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) && gWorldSector.z != 0)
				{
					continue;
				}
			}

			/*
			//if we are to display cover for the roofs, and there is a roof above us
			if( gsInterfaceLevel == I_ROOF_LEVEL && !FlatRoofAboveGridNo( sGridNo ) )
			{
				continue;
			}*/
			/*
			// if someone (visible) is there, skip
			//Check both bottom level, and top level
			SOLDIERTYPE* tgt = WhoIsThere2(sGridNo, 0);
			if (tgt == NULL) tgt = WhoIsThere2(sGridNo, 1);
			//if someone is here, and they are an enemy, skip over them
			if (tgt != NULL && tgt->bVisible == true && tgt->bTeam != pSoldier->bTeam)
			{
				continue;
			}

			//Calculate the cover for this gridno
			gCoverRadius[ sCounterX ][ sCounterY ].bCover = CalcCoverForGridNoBasedOnTeamKnownEnemies( pSoldier, sGridNo, bStance );*/

			gVisibleToSoldierStruct[ sCounterX ][ sCounterY ].bVisibleToSoldier = CalcIfSoldierCanSeeGridNo( pSoldier, sGridNo, fRoof );
			gVisibleToSoldierStruct[ sCounterX ][ sCounterY ].fRoof = fRoof;
			sCounterX++;
		}

		sCounterY++;
	}
}


static void AddVisibleToSoldierToEachGridNo(void)
{
	UINT32  uiCntX, uiCntY;
	INT8    bVisibleToSoldier=0;
	BOOLEAN fRoof;
	INT16   sGridNo;

	//loop through all the gridnos
	for(uiCntY=0; uiCntY<DC_MAX_COVER_RANGE ;uiCntY++)
	{
		for(uiCntX=0; uiCntX<DC_MAX_COVER_RANGE ;uiCntX++)
		{
			bVisibleToSoldier = gVisibleToSoldierStruct[ uiCntX ][ uiCntY ].bVisibleToSoldier;
			if( bVisibleToSoldier == -1 )
			{
				continue;
			}

			fRoof = gVisibleToSoldierStruct[ uiCntX ][ uiCntY ].fRoof;
			sGridNo = gVisibleToSoldierStruct[ uiCntX ][ uiCntY ].sGridNo;

			//if the soldier can easily see this gridno.  Can see all 3 positions
			if( bVisibleToSoldier == DC__SEE_3_STANCE )
			{
				AddCoverObjectToWorld( sGridNo, SPECIALTILE_COVER_5, fRoof );
			}

			//cant see a thing
			else if( bVisibleToSoldier == DC__SEE_NO_STANCES )
			{
				AddCoverObjectToWorld( gVisibleToSoldierStruct[ uiCntX ][ uiCntY ].sGridNo, SPECIALTILE_COVER_1, fRoof );
			}

			//can only see prone
			else if( bVisibleToSoldier == DC__SEE_1_STANCE )
			{
				AddCoverObjectToWorld( gVisibleToSoldierStruct[ uiCntX ][ uiCntY ].sGridNo, SPECIALTILE_COVER_2, fRoof );
			}

			//can see crouch or prone
			else if( bVisibleToSoldier == DC__SEE_2_STANCE )
			{
				AddCoverObjectToWorld( gVisibleToSoldierStruct[ uiCntX ][ uiCntY ].sGridNo, SPECIALTILE_COVER_3, fRoof );
			}

			else
			{
				SLOGA("AddVisibleToSoldierToEachGridNo: invalid VisibleToSoldier");
			}
		}
	}
}


void RemoveVisibleGridNoAtSelectedGridNo()
{
	UINT32  uiCntX, uiCntY;
	INT8    bVisibleToSoldier;
	BOOLEAN fRoof;

	//make sure to only remove it when its right
	if( gsLastVisibleToSoldierGridNo == NOWHERE )
	{
		return;
	}

	//loop through all the gridnos
	for(uiCntY=0; uiCntY<DC_MAX_COVER_RANGE ;uiCntY++)
	{
		for(uiCntX=0; uiCntX<DC_MAX_COVER_RANGE ;uiCntX++)
		{
			bVisibleToSoldier = gVisibleToSoldierStruct[ uiCntX ][ uiCntY ].bVisibleToSoldier;
			fRoof = gVisibleToSoldierStruct[ uiCntX ][ uiCntY ].fRoof;

			//if there is a valid cover at this gridno
			if( bVisibleToSoldier == DC__SEE_3_STANCE )
			{
				RemoveCoverObjectFromWorld( gVisibleToSoldierStruct[ uiCntX ][ uiCntY ].sGridNo, SPECIALTILE_COVER_5, fRoof );
			}

			//cant see a thing
			else if( bVisibleToSoldier == DC__SEE_NO_STANCES )
			{
				RemoveCoverObjectFromWorld( gVisibleToSoldierStruct[ uiCntX ][ uiCntY ].sGridNo, SPECIALTILE_COVER_1, fRoof );
			}

			//can only see prone
			else if( bVisibleToSoldier == DC__SEE_1_STANCE )
			{
				RemoveCoverObjectFromWorld( gVisibleToSoldierStruct[ uiCntX ][ uiCntY ].sGridNo, SPECIALTILE_COVER_2, fRoof );
			}

			//can see crouch or prone
			else if( bVisibleToSoldier == DC__SEE_2_STANCE )
			{
				RemoveCoverObjectFromWorld( gVisibleToSoldierStruct[ uiCntX ][ uiCntY ].sGridNo, SPECIALTILE_COVER_3, fRoof );
			}

			else
			{
				SLOGA("RemoveVisibleGridNoAtSelectedGridNo: invalid VisibleToSoldier");
			}
		}
	}

	// Re-render the scene!
	SetRenderFlags( RENDER_FLAG_FULL );

	gsLastVisibleToSoldierGridNo = NOWHERE;
	gsLastSoldierGridNo = NOWHERE;
}


static INT8 CalcIfSoldierCanSeeGridNo(const SOLDIERTYPE* pSoldier, INT16 sTargetGridNo, BOOLEAN fRoof)
{
	INT8    bRetVal=0;
	INT32   iLosForGridNo=0;
	UINT16  usSightLimit=0;
	BOOLEAN bAware=false;

	const SOLDIERTYPE* const tgt = WhoIsThere2(sTargetGridNo, fRoof ? 1 : 0);
	if (tgt != NULL)
	{
		const INT8* const pPersOL  = &pSoldier->bOppList[tgt->ubID];
		const INT8* const pbPublOL = &gbPublicOpplist[pSoldier->bTeam][tgt->ubID];

		// if soldier is known about (SEEN or HEARD within last few turns)
		if (*pPersOL || *pbPublOL)
		{
			bAware = true;
		}
	}


	usSightLimit = DistanceVisible( pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sTargetGridNo, fRoof );

	//
	// Prone
	//
	iLosForGridNo = SoldierToVirtualSoldierLineOfSightTest( pSoldier, sTargetGridNo, fRoof, ANIM_PRONE, (UINT8)usSightLimit, bAware );
	if( iLosForGridNo != 0 )
	{
		bRetVal++;
	}

	//
	// Crouch
	//
	iLosForGridNo = SoldierToVirtualSoldierLineOfSightTest( pSoldier, sTargetGridNo, fRoof, ANIM_CROUCH, (UINT8)usSightLimit, bAware );
	if( iLosForGridNo != 0 )
	{
		bRetVal++;
	}

	//
	// Standing
	//
	iLosForGridNo = SoldierToVirtualSoldierLineOfSightTest( pSoldier, sTargetGridNo, fRoof, ANIM_STAND, (UINT8)usSightLimit, bAware );
	if( iLosForGridNo != 0 )
	{
		bRetVal++;
	}

	return( bRetVal );
}


static BOOLEAN IsTheRoofVisible(INT16 sGridNo)
{
	if (GetRoom(sGridNo) == NO_ROOM) return false;

	if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED )
	{
		if( gTacticalStatus.uiFlags & SHOW_ALL_ROOFS )
			return true;
		else
			return false;
	}
	else
	{
		return true;
	}
}


void ChangeSizeOfDisplayCover( INT32 iNewSize )
{
	//if the new size is smaller or greater, scale it
	if( iNewSize < DC__MIN_SIZE )
	{
		iNewSize = DC__MIN_SIZE;
	}
	else if( iNewSize > DC__MAX_SIZE )
	{
		iNewSize = DC__MAX_SIZE;
	}

	//Set new size
	gGameSettings.ubSizeOfDisplayCover = (UINT8)iNewSize;

	//redisplay the cover
	RemoveCoverOfSelectedGridNo();
	DisplayCoverOfSelectedGridNo( );
}

void ChangeSizeOfLOS( INT32 iNewSize )
{
	//if the new size is smaller or greater, scale it
	if( iNewSize < DC__MIN_SIZE )
	{
		iNewSize = DC__MIN_SIZE;
	}
	else if( iNewSize > DC__MAX_SIZE )
	{
		iNewSize = DC__MAX_SIZE;
	}

	//Set new size
	gGameSettings.ubSizeOfLOS = (UINT8)iNewSize;

	//ReDisplay the los
	RemoveVisibleGridNoAtSelectedGridNo();
	DisplayGridNoVisibleToSoldierGrid( );
}
