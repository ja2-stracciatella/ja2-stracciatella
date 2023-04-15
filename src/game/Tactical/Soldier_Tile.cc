#include "Font_Control.h"
#include "Overhead.h"
#include "TileDef.h"
#include "Timer_Control.h"
#include "Render_Fun.h"
#include "Debug.h"
#include "Overhead_Types.h"
#include "Soldier_Control.h"
#include "Animation_Cache.h"
#include "Animation_Data.h"
#include "Animation_Control.h"
#include "PathAI.h"
#include "Random.h"
#include "WorldMan.h"
#include "Isometric_Utils.h"
#include "Render_Dirty.h"
#include "RenderWorld.h"
#include "Points.h"
#include "Lighting.h"
#include "Weapons.h"
#include "Handle_UI.h"
#include "Soldier_Ani.h"
#include "OppList.h"
#include "AI.h"
#include "Faces.h"
#include "Soldier_Macros.h"
#include "Soldier_Profile.h"
#include "Campaign.h"
#include "Structure_Wrap.h"
#include "Items.h"
#include "SoundMan.h"
#include "Soldier_Tile.h"
#include "Soldier_Add.h"
#include "FOV.h"
#include "Message.h"
#include "Text.h"
#include "NPC.h"
#include "Logger.h"
#include <algorithm>

constexpr milliseconds NEXT_TILE_CHECK_DELAY = 700ms;

void SetDelayedTileWaiting( SOLDIERTYPE *pSoldier, INT16 sCauseGridNo, INT8 bValue )
{
	// Cancel AI Action
	// CancelAIAction(pSoldier);

	pSoldier->fDelayedMovement = bValue;
	pSoldier->sDelayedMovementCauseGridNo = sCauseGridNo;

	RESETTIMECOUNTER( pSoldier->NextTileCounter, NEXT_TILE_CHECK_DELAY );

	// ATE: Now update realtime movement speed....
	// check if guy exists here...
	const SOLDIERTYPE* person = WhoIsThere2(sCauseGridNo, pSoldier->bLevel);

	// There may not be anybody there, but it's reserved by them!
	if ( ( gpWorldLevelData[ sCauseGridNo ].uiFlags & MAPELEMENT_MOVEMENT_RESERVED ) )
	{
		person = &GetMan(gpWorldLevelData[sCauseGridNo].ubReservedSoldierID);
	}

	if (person != NULL)
	{
		// if they are our own team members ( both )
		if (person->bTeam == OUR_TEAM && pSoldier->bTeam == OUR_TEAM)
		{
			// Here we have another guy.... save his stats so we can use them for
			// speed determinations....
			pSoldier->bOverrideMoveSpeed = person->ubID;
			pSoldier->fUseMoverrideMoveSpeed = TRUE;
		}
	}
}


static void SetFinalTile(SOLDIERTYPE* pSoldier, INT16 sGridNo, BOOLEAN fGivenUp)
{
	// OK, If we were waiting for stuff, do it here...
	pSoldier->sFinalDestination = pSoldier->sGridNo;

	if ( pSoldier->bTeam == OUR_TEAM  && fGivenUp )
	{
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(TacticalStr[ NO_PATH_FOR_MERC ], pSoldier->name) );
	}

	EVENT_StopMerc(pSoldier);
}


static void MarkMovementReserved(SOLDIERTYPE& s, INT16 const sGridNo)
{
	// Check if we have one reserrved already, and free it first!
	if (s.sReservedMovementGridNo != NOWHERE)
	{
		UnMarkMovementReserved(s);
	}

	// For single-tiled mercs, set this gridno
	gpWorldLevelData[ sGridNo ].uiFlags |= MAPELEMENT_MOVEMENT_RESERVED;

	// Save soldier's reserved ID #
	gpWorldLevelData[sGridNo].ubReservedSoldierID = s.ubID;

	s.sReservedMovementGridNo = sGridNo;
}


void UnMarkMovementReserved(SOLDIERTYPE& s)
{
	INT16 sNewGridNo;

	sNewGridNo = GETWORLDINDEXFROMWORLDCOORDS(s.dYPos, s.dXPos);

	// OK, if NOT in fence anim....
	if (s.usAnimState == HOPFENCE && s.sReservedMovementGridNo != sNewGridNo)
	{
		return;
	}

	// For single-tiled mercs, unset this gridno
	// See if we have one reserved!
	if (s.sReservedMovementGridNo != NOWHERE)
	{
		gpWorldLevelData[s.sReservedMovementGridNo].uiFlags &= ~MAPELEMENT_MOVEMENT_RESERVED;
		s.sReservedMovementGridNo = NOWHERE;
	}
}


static INT8 TileIsClear(SOLDIERTYPE* pSoldier, INT8 bDirection, INT16 sGridNo, INT8 bLevel)
{
	INT16 sTempDestGridNo;
	INT16 sNewGridNo;

	if ( sGridNo == NOWHERE )
	{
		return( MOVE_TILE_CLEAR );
	}

	SOLDIERTYPE* const tgt = WhoIsThere2(sGridNo, bLevel);
	if (tgt != NULL)
	{
		// If this us?
		if (tgt != pSoldier)
		{
			// OK, set flag indicating we are blocked by a merc....
			if ( pSoldier->bTeam != OUR_TEAM ) // CJC: shouldn't this be in all cases???
			//if ( 0 )
			{
				pSoldier->fBlockedByAnotherMerc = TRUE;
				// Set direction we were trying to goto
				pSoldier->bBlockedByAnotherMercDirection = bDirection;

				// Are we only temporarily blocked?
				// Check if our final destination is = our gridno
				if (tgt->sFinalDestination == tgt->sGridNo)
				{
					return( MOVE_TILE_STATIONARY_BLOCKED );
				}
				else
				{
					// OK, if buddy who is blocking us is trying to move too...
					// And we are in opposite directions...
					if (tgt->fBlockedByAnotherMerc &&
						tgt->bBlockedByAnotherMercDirection == OppositeDirection(bDirection))
					{
						// OK, try and get a path around buddy....
						// We have to temporarily make buddy stopped...
						sTempDestGridNo = tgt->sFinalDestination;
						tgt->sFinalDestination = tgt->sGridNo;

						if (PlotPath(pSoldier, pSoldier->sFinalDestination, NO_COPYROUTE, NO_PLOT, pSoldier->usUIMovementMode, pSoldier->bActionPoints))
						{
							pSoldier->bPathStored = FALSE;
							// OK, make guy go here...
							EVENT_GetNewSoldierPath( pSoldier, pSoldier->sFinalDestination, pSoldier->usUIMovementMode );
							// Restore final dest....
							tgt->sFinalDestination = sTempDestGridNo;
							pSoldier->fBlockedByAnotherMerc = FALSE;

							// Is the next tile blocked too?
							sNewGridNo = NewGridNo( (UINT16)pSoldier->sGridNo, DirectionInc( guiPathingData[ 0 ] ) );

							return( TileIsClear( pSoldier, guiPathingData[ 0 ], sNewGridNo, pSoldier->bLevel ) );
						}
						else
						{

							// Not for multi-tiled things...
							if ( !( pSoldier->uiStatusFlags & SOLDIER_MULTITILE ) )
							{
								// If we are to swap and we're near a door, open door first and then close it...?

								// Swap now!
								tgt->fBlockedByAnotherMerc = FALSE;

								// Restore final dest....
								tgt->sFinalDestination = sTempDestGridNo;

								// Swap merc positions.....
								SwapMercPositions(*pSoldier, *tgt);

								// With these two guys swapped, they should try and continue on their way....
								// Start them both again along their way...
								EVENT_GetNewSoldierPath( pSoldier, pSoldier->sFinalDestination, pSoldier->usUIMovementMode );
								EVENT_GetNewSoldierPath(tgt, tgt->sFinalDestination, tgt->usUIMovementMode);
							}
						}
					}
					return( MOVE_TILE_TEMP_BLOCKED );
				}
			}
			else
			{
				//return( MOVE_TILE_STATIONARY_BLOCKED );
				// ATE: OK, put some smartshere...
				// If we are waiting for more than a few times, change to stationary...
				if (tgt->fDelayedMovement >= 105)
				{
					// Set to special 'I want to walk through people' value
					pSoldier->fDelayedMovement = 150;

					return( MOVE_TILE_STATIONARY_BLOCKED );
				}
				if (tgt->sGridNo == tgt->sFinalDestination)
				{
					return( MOVE_TILE_STATIONARY_BLOCKED );
				}
				return( MOVE_TILE_TEMP_BLOCKED );
			}
		}
	}

	if ( ( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_MOVEMENT_RESERVED ) )
	{
		if ( gpWorldLevelData[ sGridNo ].ubReservedSoldierID != pSoldier->ubID )
		{
			return( MOVE_TILE_TEMP_BLOCKED );
		}
	}

	// Are we clear of structs?
	if ( !NewOKDestination( pSoldier, sGridNo, FALSE, pSoldier->bLevel ) )
	{
			// ATE: Fence cost is an exclusiuon here....
			if ( gubWorldMovementCosts[ sGridNo ][ bDirection ][ pSoldier->bLevel ] != TRAVELCOST_FENCE )
			{
				// ATE: HIdden structs - we do something here... reveal it!
				if ( gubWorldMovementCosts[ sGridNo ][ bDirection ][ pSoldier->bLevel ] == TRAVELCOST_HIDDENOBSTACLE )
				{
					gpWorldLevelData[ sGridNo ].uiFlags|=MAPELEMENT_REVEALED;
					gpWorldLevelData[ sGridNo ].uiFlags|=MAPELEMENT_REDRAW;
					SetRenderFlags(RENDER_FLAG_MARKED);
					RecompileLocalMovementCosts( (UINT16)sGridNo );
				}

				// Unset flag for blocked by soldier...
				pSoldier->fBlockedByAnotherMerc = FALSE;
				return( MOVE_TILE_STATIONARY_BLOCKED );
			}
			else
			{
#if 0
				// Check if there is a reserved marker here at least....
				sNewGridNo = NewGridNo( sGridNo, DirectionInc( bDirection ) );

				if ( ( gpWorldLevelData[ sNewGridNo ].uiFlags & MAPELEMENT_MOVEMENT_RESERVED ) )
				{
					if ( gpWorldLevelData[ sNewGridNo ].ubReservedSoldierID != pSoldier->ubID )
					{
						return( MOVE_TILE_TEMP_BLOCKED );
					}
				}
#endif
			}
	}

	// Unset flag for blocked by soldier...
	pSoldier->fBlockedByAnotherMerc = FALSE;

	return( MOVE_TILE_CLEAR );

}



BOOLEAN HandleNextTile( SOLDIERTYPE *pSoldier, INT8 bDirection, INT16 sGridNo, INT16 sFinalDestTile )
{
	INT8  bBlocked;
	INT16 bOverTerrainType;

	// Check for blocking if in realtime
	///if (!(gTacticalStatus.uiFlags & INCOMBAT))

	// ATE: If not on visible tile, return clear ( for path out of map )
	if ( !GridNoOnVisibleWorldTile( sGridNo ) )
	{
		return( TRUE );
	}

	// If animation state is crow, iall is clear
	if ( pSoldier->usAnimState == CROW_FLY )
	{
		return( TRUE );
	}

	{
		bBlocked = TileIsClear( pSoldier, bDirection, sGridNo, pSoldier->bLevel );

		// Check if we are blocked...
		if ( bBlocked != MOVE_TILE_CLEAR )
		{
			// Is the next gridno our destination?
			// OK: Let's check if we are NOT walking off screen
			if ( sGridNo == sFinalDestTile && pSoldier->ubWaitActionToDo == 0 &&
				(pSoldier->bTeam == OUR_TEAM ||
				pSoldier->sAbsoluteFinalDestination == NOWHERE) )
			{
				// Yah, well too bad, stop here.
				SetFinalTile( pSoldier, pSoldier->sGridNo, FALSE );

				return( FALSE );
			}
			// CHECK IF they are stationary
			else if ( bBlocked == MOVE_TILE_STATIONARY_BLOCKED )
			{
				// Stationary,
				INT16 sOldFinalDest;

				// Maintain sFinalDest....
				sOldFinalDest = pSoldier->sFinalDestination;
				EVENT_StopMerc(pSoldier);
				// Restore...
				pSoldier->sFinalDestination = sOldFinalDest;

				SetDelayedTileWaiting( pSoldier, sGridNo, 1 );

				return( FALSE );
			}
			else
			{
				INT16 sOldFinalDest;

				// Maintain sFinalDest....
				sOldFinalDest = pSoldier->sFinalDestination;
				EVENT_StopMerc(pSoldier);
				// Restore...
				pSoldier->sFinalDestination = sOldFinalDest;

				// Setting to two means: try and wait until this tile becomes free....
				SetDelayedTileWaiting( pSoldier, sGridNo, 100 );

				return( FALSE );
			}
		}
		else
		{
			// Mark this tile as reserverd ( until we get there! )
			if (!(gTacticalStatus.uiFlags & INCOMBAT))
			{
				MarkMovementReserved(*pSoldier, sGridNo);
			}

			bOverTerrainType = GetTerrainType( sGridNo );

			// Check if we are going into water!
			if ( bOverTerrainType == LOW_WATER || bOverTerrainType == MED_WATER || bOverTerrainType == DEEP_WATER )
			{
				// Check if we are of prone or crawl height and change stance accordingly....
				switch( gAnimControl[ pSoldier->usAnimState ].ubHeight )
				{
					case ANIM_PRONE:
					case ANIM_CROUCH:

						// Change height to stand
						pSoldier->fContinueMoveAfterStanceChange = TRUE;
						ChangeSoldierStance(pSoldier, ANIM_STAND);
						break;
				}

				// Check animation
				// Change to walking
				if ( pSoldier->usAnimState == RUNNING )
				{
					ChangeSoldierState( pSoldier, WALKING, 0 , FALSE );
				}
			}
		}
	}
	return( TRUE );
}


void HandleNextTileWaiting(SOLDIERTYPE* const pSoldier)
{
	// Buddy is waiting to continue his path
	INT8  bBlocked, bPathBlocked;
	INT16 sCost;
	INT16 sNewGridNo, sCheckGridNo;
	UINT8 fFlags = 0;

	if ( pSoldier->fDelayedMovement )
	{
		if ( TIMECOUNTERDONE( pSoldier->NextTileCounter, NEXT_TILE_CHECK_DELAY ) )
		{
			// Get direction from gridno...
			const UINT8 bCauseDirection = GetDirectionToGridNoFromGridNo(pSoldier->sGridNo, pSoldier->sDelayedMovementCauseGridNo);
			bBlocked = TileIsClear( pSoldier, bCauseDirection, pSoldier->sDelayedMovementCauseGridNo, pSoldier->bLevel );

			// If we are waiting for a temp blockage.... continue to wait
			if ( pSoldier->fDelayedMovement >= 100 &&  bBlocked == MOVE_TILE_TEMP_BLOCKED )
			{
				// ATE: Increment 1
				pSoldier->fDelayedMovement++;

				// Are we close enough to give up? ( and are a pc )
				if ( pSoldier->fDelayedMovement > 120 )
				{
					// Quit...
					SetFinalTile( pSoldier, pSoldier->sGridNo, TRUE );
					pSoldier->fDelayedMovement = FALSE;
				}
				return;
			}

			// Try new path if anything but temp blockage!
			if ( bBlocked != MOVE_TILE_TEMP_BLOCKED )
			{
				// Set to normal delay
				if ( pSoldier->fDelayedMovement >= 100 && pSoldier->fDelayedMovement != 150 )
				{
					pSoldier->fDelayedMovement = 1;
				}

				// Default to pathing through people
				fFlags = PATH_THROUGH_PEOPLE;

				// Now, if we are in the state where we are desparently trying to get out...
				// Use other flag
				// CJC: path-through-people includes ignoring person at dest
				/*
				if ( pSoldier->fDelayedMovement >= 150 )
				{
					fFlags = PATH_IGNORE_PERSON_AT_DEST;
				}
				*/

				// Check destination first!
				if ( pSoldier->sAbsoluteFinalDestination == pSoldier->sFinalDestination )
				{
					// on last lap of scripted move, make sure we get to final dest
					sCheckGridNo = pSoldier->sAbsoluteFinalDestination;
				}
				else if (!NewOKDestination( pSoldier, pSoldier->sFinalDestination, TRUE, pSoldier->bLevel ))
				{
					if ( pSoldier->fDelayedMovement >= 150 )
					{
						// OK, look around dest for the first one!
						sCheckGridNo = FindGridNoFromSweetSpot(pSoldier, pSoldier->sFinalDestination, 6);

						if ( sCheckGridNo == NOWHERE )
						{
							// If this is nowhere, try harder!
							sCheckGridNo = FindGridNoFromSweetSpot(pSoldier, pSoldier->sFinalDestination, 16);
						}
					}
					else
					{
						// OK, look around dest for the first one!
						sCheckGridNo = FindGridNoFromSweetSpotThroughPeople(pSoldier, pSoldier->sFinalDestination, 6);

						if ( sCheckGridNo == NOWHERE )
						{
							// If this is nowhere, try harder!
							sCheckGridNo = FindGridNoFromSweetSpotThroughPeople(pSoldier, pSoldier->sFinalDestination, 16);
						}
					}
				}
				else
				{
					sCheckGridNo = pSoldier->sFinalDestination;
				}

				// Try another path to destination
				// ATE: Allow path to exit grid!
				if ( pSoldier->ubWaitActionToDo == 1 && gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO )
				{
					gfPlotPathToExitGrid = TRUE;
				}

				sCost = (INT16) FindBestPath( pSoldier, sCheckGridNo, pSoldier->bLevel, pSoldier->usUIMovementMode, NO_COPYROUTE, fFlags );
				gfPlotPathToExitGrid = FALSE;

				// Can we get there
				if ( sCost > 0 )
				{
					// Is the next tile blocked too?
					sNewGridNo = NewGridNo(pSoldier->sGridNo, DirectionInc(guiPathingData[0]));

					bPathBlocked = TileIsClear( pSoldier, guiPathingData[ 0 ], sNewGridNo, pSoldier->bLevel );

					if ( bPathBlocked == MOVE_TILE_STATIONARY_BLOCKED )
					{
						// Try to path around everyone except dest person

						if ( pSoldier->ubWaitActionToDo == 1 && gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO )
						{
							gfPlotPathToExitGrid = TRUE;
						}

						FindBestPath(pSoldier, sCheckGridNo, pSoldier->bLevel, pSoldier->usUIMovementMode, NO_COPYROUTE, PATH_IGNORE_PERSON_AT_DEST);

						gfPlotPathToExitGrid = FALSE;

						// Is the next tile in this new path blocked too?
						sNewGridNo = NewGridNo(pSoldier->sGridNo, DirectionInc(guiPathingData[0]));

						bPathBlocked = TileIsClear( pSoldier, guiPathingData[ 0 ], sNewGridNo, pSoldier->bLevel );

						// now working with a path which does not go through people
						pSoldier->ubDelayedMovementFlags &= (~DELAYED_MOVEMENT_FLAG_PATH_THROUGH_PEOPLE);
					}
					else
					{
						// path through people worked fine
						if ( pSoldier->fDelayedMovement < 150 )
						{
							pSoldier->ubDelayedMovementFlags |= DELAYED_MOVEMENT_FLAG_PATH_THROUGH_PEOPLE;
						}
					}

					// Are we clear?
					if ( bPathBlocked == MOVE_TILE_CLEAR )
					{
						// Go for it path!
						if ( pSoldier->ubWaitActionToDo == 1 && gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO )
						{
							gfPlotPathToExitGrid = TRUE;
						}

						//pSoldier->fDelayedMovement = FALSE;
						// ATE: THis will get set in EENT_GetNewSoldierPath....
						pSoldier->usActionData = sCheckGridNo;

						pSoldier->bPathStored = FALSE;

						EVENT_GetNewSoldierPath( pSoldier, sCheckGridNo, pSoldier->usUIMovementMode );
						gfPlotPathToExitGrid = FALSE;
						return;
					}
				}

				pSoldier->fDelayedMovement++;

				if ( pSoldier->fDelayedMovement == 99 )
				{
					// Cap at 99
					pSoldier->fDelayedMovement = 99;
				}

				// Do we want to force a swap?
				if (pSoldier->fDelayedMovement == 3 && (pSoldier->sAbsoluteFinalDestination != NOWHERE || gTacticalStatus.fAutoBandageMode) )
				{
					// with person who is in the way?
					// if either on a mission from god, or two AI guys not on stationary...
					SOLDIERTYPE* const tgt = WhoIsThere2(pSoldier->sDelayedMovementCauseGridNo, pSoldier->bLevel);
					if (tgt != NULL)
					{
						if (pSoldier->ubQuoteRecord != 0 ||
							(pSoldier->bTeam != OUR_TEAM &&
							pSoldier->bOrders != STATIONARY &&
							tgt->bTeam != OUR_TEAM &&
							tgt->bOrders != STATIONARY) ||
							(pSoldier->bTeam == OUR_TEAM &&
							gTacticalStatus.fAutoBandageMode &&
							(tgt->bTeam != CIV_TEAM ||
							tgt->bOrders != STATIONARY)))
						{
							// Swap now!
							//tgt->fBlockedByAnotherMerc = FALSE;

							// Restore final dest....
							//tgt->sFinalDestination = sTempDestGridNo;

							// Swap merc positions.....
							SwapMercPositions(*pSoldier, *tgt);

							// With these two guys swapped, we should try to continue on our way....
							pSoldier->fDelayedMovement = FALSE;

							// We must calculate the path here so that we can give it the "through people" parameter
							if (gTacticalStatus.fAutoBandageMode && pSoldier->sAbsoluteFinalDestination == NOWHERE)
							{
								FindBestPath( pSoldier, pSoldier->sFinalDestination, pSoldier->bLevel, pSoldier->usUIMovementMode, COPYROUTE, PATH_THROUGH_PEOPLE );
							}
							else if (pSoldier->sAbsoluteFinalDestination != NOWHERE && !FindBestPath(pSoldier, pSoldier->sAbsoluteFinalDestination, pSoldier->bLevel, pSoldier->usUIMovementMode, COPYROUTE, PATH_THROUGH_PEOPLE))
							{
								// check to see if we're there now!
								if (pSoldier->sGridNo == pSoldier->sAbsoluteFinalDestination)
								{
									if (pSoldier->ubProfile != NO_PROFILE)
									{
										NPCReachedDestination(pSoldier, FALSE);
									}
									pSoldier->bNextAction = AI_ACTION_WAIT;
									pSoldier->usNextActionData = 500;
									return;
								}
							}
							pSoldier->bPathStored = TRUE;

							EVENT_GetNewSoldierPath(pSoldier, pSoldier->sAbsoluteFinalDestination, pSoldier->usUIMovementMode);
							//EVENT_GetNewSoldierPath(tgt, tgt->sFinalDestination, tgt->usUIMovementMode);
						}
					}
				}

				// Are we close enough to give up? ( and are a pc )
				if ( pSoldier->fDelayedMovement > 20 && pSoldier->fDelayedMovement != 150)
				{
					if ( PythSpacesAway( pSoldier->sGridNo, pSoldier->sFinalDestination ) < 5 && pSoldier->bTeam == OUR_TEAM )
					{
						// Quit...
						SetFinalTile( pSoldier, pSoldier->sGridNo, FALSE );
						pSoldier->fDelayedMovement = FALSE;
					}
				}

				// Are we close enough to give up? ( and are a pc )
				if ( pSoldier->fDelayedMovement > 170 )
				{
					if ( PythSpacesAway( pSoldier->sGridNo, pSoldier->sFinalDestination ) < 5 && pSoldier->bTeam == OUR_TEAM )
					{
						// Quit...
						SetFinalTile( pSoldier, pSoldier->sGridNo, FALSE );
						pSoldier->fDelayedMovement = FALSE;
					}
				}

			}
		}
	}
}


bool TeleportSoldier(SOLDIERTYPE& s, GridNo const gridno, bool const force)
{
	if (!NewOKDestination(&s, gridno, TRUE, 0) && !force) return false;

	// Teleport to this location
	EVENT_SetSoldierPosition(&s, gridno, SSP_NONE);
	s.sFinalDestination = gridno;
	RevealRoofsAndItems(&s, TRUE);
	HandleSight(s, SIGHT_LOOK | SIGHT_RADIO);
	GivingSoldierCancelServices(&s);
	if (s.light) LightSpriteRoofStatus(s.light, s.bLevel != 0);
	return true;
}


// Swap 2 soldier positions
void SwapMercPositions(SOLDIERTYPE& s1, SOLDIERTYPE& s2)
{
	// Save positions
	GridNo gridno1 = s1.sGridNo;
	GridNo gridno2 = s2.sGridNo;

	// Remove each
	RemoveSoldierFromGridNo(s1);
	RemoveSoldierFromGridNo(s2);

	// Test OK destination for each
	bool const canSwap = NewOKDestination(&s1, gridno2, TRUE, 0) && NewOKDestination(&s2, gridno1, TRUE, 0);
	if (canSwap)
	{
		std::swap(gridno1, gridno2);
	}
	// else both soldiers will be reinserted at their old position

	// We must first call EVENT_SetSoldierPosition for one soldier because we currently have
	// two soldiers at NOWHERE and cannot call HandleSight in TeleportSoldier (#1607)
	EVENT_SetSoldierPosition(&s2, gridno2, SSP_NONE);
	TeleportSoldier(s1, gridno1, !canSwap);
	// Now both soldiers have a valid gridno and we can fully update the status of the second
	TeleportSoldier(s2, gridno2, !canSwap);
}


BOOLEAN CanExchangePlaces( SOLDIERTYPE *pSoldier1, SOLDIERTYPE *pSoldier2, BOOLEAN fShow )
{
	// NB checks outside of this function
	if ( EnoughPoints( pSoldier1, AP_EXCHANGE_PLACES, 0, fShow ) )
	{
		if ( EnoughPoints( pSoldier2, AP_EXCHANGE_PLACES, 0, fShow ) )
		{
			if ( ( gAnimControl[ pSoldier2->usAnimState ].uiFlags & ANIM_MOVING ) )
			{
				return( FALSE );
			}

			if ( ( gAnimControl[ pSoldier1->usAnimState ].uiFlags & ANIM_MOVING ) && !(gTacticalStatus.uiFlags & INCOMBAT) )
			{
				return( FALSE );
			}

			if (!InternalIsValidStance(pSoldier1, pSoldier2->bDirection, GetStance(*pSoldier2)) ||
			    !InternalIsValidStance(pSoldier2, pSoldier1->bDirection, GetStance(*pSoldier1)))
			{
				return false;
			}

			if ( pSoldier2->bSide == 0 )
			{
				return( TRUE );
			}

			// hehe - don't allow animals to exchange places
			if ( pSoldier2->uiStatusFlags & ( SOLDIER_ANIMAL ) )
			{
				return( FALSE );
			}

			// must NOT be hostile, must NOT have stationary orders OR militia team, must be >= OKLIFE
			if ( pSoldier2->bNeutral && pSoldier2->bLife >= OKLIFE &&
				pSoldier2->ubCivilianGroup != HICKS_CIV_GROUP &&
				( ( pSoldier2->bOrders != STATIONARY ||
				pSoldier2->bTeam == MILITIA_TEAM ) ||
				( pSoldier2->sAbsoluteFinalDestination != NOWHERE &&
				pSoldier2->sAbsoluteFinalDestination != pSoldier2->sGridNo ) ) )
			{
				return( TRUE );
			}

			if ( fShow )
			{
				if ( pSoldier2->ubProfile == NO_PROFILE )
				{
					ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK,
							TacticalStr[REFUSE_EXCHANGE_PLACES]);
				}
				else
				{
					ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK,
							st_format_printf(gzLateLocalizedString[STR_LATE_03], pSoldier2->name));
				}
			}
		}
	}
	return FALSE;
}
