#include "Font.h"
#include "Items.h"
#include "Action_Items.h"
#include "Handle_Items.h"
#include "Overhead.h"
#include "Soldier_Find.h"
#include "Weapons.h"
#include "Points.h"
#include "TileDef.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "Interface.h"
#include "RenderWorld.h"
#include "Animation_Control.h"
#include "Font_Control.h"
#include "Render_Dirty.h"
#include "World_Items.h"
#include "Text.h"
#include "Timer_Control.h"
#include "WCheck.h"
#include "Interface_Items.h"
#include "Physics.h"
#include "Soldier_Profile.h"
#include "Interface_Dialogue.h"
#include "Quests.h"
#include "Message.h"
#include "Isometric_Utils.h"
#include "LOS.h"
#include "Dialogue_Control.h"
#include "AI.h"
#include "Soldier_Macros.h"
#include "Interface_Panels.h"
#include "Strategic_Town_Loyalty.h"
#include "Soldier_Functions.h"
#include "Map_Screen_Helicopter.h"
#include "PathAI.h"
#include "FOV.h"
#include "MessageBoxScreen.h"
#include "Explosion_Control.h"
#include "SkillCheck.h"
#include "Campaign.h"
#include "Random.h"
#include "Structure_Wrap.h"
#include "Interactive_Tiles.h"
#include "SaveLoadMap.h"
#include "ShopKeeper_Interface.h"
#include "Arms_Dealer_Init.h"
#include "Soldier_Add.h"
#include "Sound_Control.h"
#include "Squads.h"
#include "Rotting_Corpses.h"
#include "Soldier_Ani.h"
#include "OppList.h"
#include "QArray.h"
#include "Render_Fun.h"
#include "Environment.h"
#include "Map_Information.h"
#include "GameSettings.h"
#include "StrategicMap.h"
#include "End_Game.h"
#include "Interface_Control.h"
#include "Map_Screen_Interface_Map_Inventory.h"
#include "ScreenIDs.h"
#include "VSurface.h"
#include "MemMan.h"
#include "Debug.h"


#define					NUM_ITEMS_LISTED			8
#define					NUM_ITEM_FLASH_SLOTS	50
#define					MIN_LOB_RANGE					6


ITEM_POOL_LOCATOR				FlashItemSlots[ NUM_ITEM_FLASH_SLOTS ];
UINT32									guiNumFlashItemSlots = 0;


// Disgusting hacks: have to keep track of these values for accesses in callbacks
static SOLDIERTYPE *	gpTempSoldier;
static INT16					gsTempGridno;
static INT8						bTempFrequency;


extern BOOLEAN ItemIsCool( OBJECTTYPE * pObj );
extern INT8	gbItemPointerSrcSlot;
extern void MAPEndItemPointer( );
extern BOOLEAN	gfResetUIMovementOptimization;

SOLDIERTYPE *		gpBoobyTrapSoldier;
ITEM_POOL *			gpBoobyTrapItemPool;
INT16						gsBoobyTrapGridNo;
INT8            gbBoobyTrapLevel;
BOOLEAN					gfDisarmingBuriedBomb;
extern BOOLEAN	gfDontChargeAPsToPickup;
INT8						gbTrapDifficulty;
BOOLEAN					gfJustFoundBoobyTrap = FALSE;


BOOLEAN	HandleCheckForBadChangeToGetThrough( SOLDIERTYPE *pSoldier, SOLDIERTYPE *pTargetSoldier, INT16 sTargetGridNo , INT8 bLevel )
{
	BOOLEAN						fBadChangeToGetThrough = FALSE;

	if ( pTargetSoldier != NULL )
	{
		if ( SoldierToSoldierBodyPartChanceToGetThrough( pSoldier, pTargetSoldier, pSoldier->bAimShotLocation ) < OK_CHANCE_TO_GET_THROUGH )
		{
			fBadChangeToGetThrough = TRUE;
		}
	}
	else
	{
		if ( SoldierToLocationChanceToGetThrough( pSoldier, sTargetGridNo, (INT8) bLevel, 0, NOBODY ) < OK_CHANCE_TO_GET_THROUGH )
		{
			fBadChangeToGetThrough = TRUE;
		}
	}

	if ( fBadChangeToGetThrough )
	{
		if ( gTacticalStatus.sCantGetThroughSoldierGridNo != pSoldier->sGridNo ||  gTacticalStatus.sCantGetThroughGridNo != sTargetGridNo || gTacticalStatus.ubCantGetThroughID != pSoldier->ubID )
		{
			gTacticalStatus.fCantGetThrough = FALSE;
		}

		// Have we done this once already?
		if ( !gTacticalStatus.fCantGetThrough )
		{
			gTacticalStatus.fCantGetThrough								= TRUE;
			gTacticalStatus.sCantGetThroughGridNo					= sTargetGridNo;
			gTacticalStatus.ubCantGetThroughID						= pSoldier->ubID;
			gTacticalStatus.sCantGetThroughSoldierGridNo	= pSoldier->sGridNo;

			// PLay quote
			TacticalCharacterDialogue( pSoldier, QUOTE_NO_LINE_OF_FIRE );
			return( FALSE );
		}
		else
		{
			// Is this a different case?
			if ( gTacticalStatus.sCantGetThroughGridNo != sTargetGridNo || gTacticalStatus.ubCantGetThroughID != pSoldier->ubID || gTacticalStatus.sCantGetThroughSoldierGridNo != pSoldier->sGridNo )
			{
				// PLay quote
				gTacticalStatus.sCantGetThroughGridNo	= sTargetGridNo;
				gTacticalStatus.ubCantGetThroughID		= pSoldier->ubID;

				TacticalCharacterDialogue( pSoldier, QUOTE_NO_LINE_OF_FIRE );
				return( FALSE );
			}
		}
	}
	else
	{
		gTacticalStatus.fCantGetThrough = FALSE;
	}

	return( TRUE );
}


static BOOLEAN HandItemWorks(SOLDIERTYPE* pSoldier, INT8 bSlot);
static void StartBombMessageBox(SOLDIERTYPE* pSoldier, INT16 sGridNo);


INT32 HandleItem( SOLDIERTYPE *pSoldier, UINT16 usGridNo, INT8 bLevel, UINT16 usHandItem, BOOLEAN fFromUI )
{
	SOLDIERTYPE				*pTargetSoldier = NULL;
	UINT16						usSoldierIndex;
	INT16							sTargetGridNo;
	INT16							sAPCost;
	INT16							sActionGridNo;
	UINT8							ubDirection;
	INT16							sAdjustedGridNo;
	BOOLEAN						fDropBomb = FALSE;
  BOOLEAN						fAddingTurningCost = FALSE;
  BOOLEAN						fAddingRaiseGunCost = FALSE;
	LEVELNODE					*pIntNode;
	STRUCTURE					*pStructure;
	INT16							sGridNo;


	// Remove any previous actions
	pSoldier->ubPendingAction		 = NO_PENDING_ACTION;

	// here is where we would set a different value if the weapon mode is on
	// "attached weapon"
	pSoldier->usAttackingWeapon = usHandItem;

	// Find soldier flags depend on if it's our own merc firing or a NPC
  //if ( FindSoldier( usGridNo, &usSoldierIndex, &uiMercFlags, FIND_SOLDIER_GRIDNO )  )
	if ( ( usSoldierIndex = WhoIsThere2( usGridNo, bLevel ) ) != NO_SOLDIER )
	{
		pTargetSoldier = MercPtrs[ usSoldierIndex ];

    if ( fFromUI )
    {
	    // ATE: Check if we are targeting an interactive tile, and adjust gridno accordingly...
	    pIntNode = GetCurInteractiveTileGridNoAndStructure( &sGridNo, &pStructure );

  		if ( pIntNode != NULL && pTargetSoldier == pSoldier )
      {
        // Truncate target sioldier
        pTargetSoldier = NULL;
      }
    }
	}

	// ATE: If in realtime, set attacker count to 0...
	if ( !(gTacticalStatus.uiFlags & INCOMBAT) )
	{
		DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Setting attack busy count to 0 due to no combat");
		gTacticalStatus.ubAttackBusyCount = 0;
	}

  if ( pTargetSoldier )
  {
    pTargetSoldier->bBeingAttackedCount = 0;
  }


	// Check our soldier's life for unconscious!
	if ( pSoldier->bLife < OKLIFE )
	{
		return( ITEM_HANDLE_UNCONSCIOUS );
	}

	if (HandItemWorks( pSoldier, HANDPOS ) == FALSE )
	{
		return( ITEM_HANDLE_BROKEN );
	}

	if ( fFromUI && pSoldier->bTeam == gbPlayerNum && pTargetSoldier && (pTargetSoldier->bTeam == gbPlayerNum || pTargetSoldier->bNeutral) && pTargetSoldier->ubBodyType != CROW && Item[ usHandItem ].usItemClass != IC_MEDKIT )
	{
		if ( pSoldier->ubProfile != NO_PROFILE  )
		{
			// nice mercs won't shoot other nice guys or neutral civilians
			if ( (gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags3 & PROFILE_MISC_FLAG3_GOODGUY) && ( (pTargetSoldier->ubProfile == NO_PROFILE && pTargetSoldier->bNeutral) || gMercProfiles[ pTargetSoldier->ubProfile ].ubMiscFlags3 & PROFILE_MISC_FLAG3_GOODGUY) )
			{
				TacticalCharacterDialogue( pSoldier, QUOTE_REFUSING_ORDER );
				return( ITEM_HANDLE_REFUSAL );
			}
			if ( pTargetSoldier->ubProfile != NO_PROFILE )
			{
				// buddies won't shoot each other
				if ( WhichBuddy( pSoldier->ubProfile, pTargetSoldier->ubProfile ) != -1 )
				{
					TacticalCharacterDialogue( pSoldier, QUOTE_REFUSING_ORDER );
					return( ITEM_HANDLE_REFUSAL );
				}
			}

			// any recruited rebel will refuse to fire on another rebel or neutral nameless civ
			if ( pSoldier->ubCivilianGroup == REBEL_CIV_GROUP && (pTargetSoldier->ubCivilianGroup == REBEL_CIV_GROUP || ( pTargetSoldier->bNeutral && pTargetSoldier->ubProfile == NO_PROFILE && pTargetSoldier->ubCivilianGroup == NON_CIV_GROUP && pTargetSoldier->ubBodyType != CROW ) ) )
			{
				TacticalCharacterDialogue( pSoldier, QUOTE_REFUSING_ORDER );
				return( ITEM_HANDLE_REFUSAL );
			}
		}

	}

	// Check HAND ITEM
	if ( Item[ usHandItem ].usItemClass == IC_GUN || Item[ usHandItem ].usItemClass == IC_THROWING_KNIFE )
	{
		// WEAPONS
		if ( usHandItem == ROCKET_RIFLE || usHandItem == AUTO_ROCKET_RIFLE )
		{
			// check imprint ID
			// NB not-imprinted value is NO_PROFILE
			// imprinted value is profile for mercs & NPCs and NO_PROFILE + 1 for generic dudes
			if (pSoldier->ubProfile != NO_PROFILE)
			{
				if ( pSoldier->inv[ pSoldier->ubAttackingHand ].ubImprintID != pSoldier->ubProfile )
				{
					if ( pSoldier->inv[ pSoldier->ubAttackingHand ].ubImprintID == NO_PROFILE )
					{
						// first shot using "virgin" gun... set imprint ID
						pSoldier->inv[ pSoldier->ubAttackingHand ].ubImprintID = pSoldier->ubProfile;

						// this could be an NPC (Krott)
						if (pSoldier->bTeam == gbPlayerNum)
						{
							PlayJA2Sample(RG_ID_IMPRINTED, HIGHVOLUME, 1, MIDDLE);

      			  ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"\"%ls\"", TacticalStr[ GUN_GOT_FINGERPRINT ] );

							return( ITEM_HANDLE_BROKEN );
						}
					}
					else
					{
						// access denied!
						if (pSoldier->bTeam == gbPlayerNum)
						{
							PlayJA2Sample(RG_ID_INVALID, HIGHVOLUME, 1, MIDDLE);

							//if (Random( 100 ) < (UINT32) pSoldier->bWisdom)
							//{
							//	DoMercBattleSound( pSoldier, BATTLE_SOUND_CURSE1 );
							//}
							//else
							//{
							//	TacticalCharacterDialogue( pSoldier, QUOTE_USELESS_ITEM );
							//}
						}
						return( ITEM_HANDLE_BROKEN );
					}
				}
			}
			else
			{
				// guaranteed not to be controlled by the player, so no feedback required
				if ( pSoldier->inv[ pSoldier->ubAttackingHand ].ubImprintID != (NO_PROFILE + 1) )
				{
					if ( pSoldier->inv[ pSoldier->ubAttackingHand ].ubImprintID == NO_PROFILE )
					{
						pSoldier->inv[ pSoldier->ubAttackingHand ].ubImprintID = (NO_PROFILE + 1);
					}
					else
					{
						return( ITEM_HANDLE_BROKEN );
					}
				}
			}
		}

		// IF we are not a throwing knife, check for ammo, reloading...
		if ( Item[ usHandItem ].usItemClass != IC_THROWING_KNIFE )
		{
			// CHECK FOR AMMO!
			if ( !EnoughAmmo( pSoldier, fFromUI, HANDPOS ) )
			{
				//ATE: Reflect that we need to reset for bursting
				pSoldier->fDoSpread = FALSE;
				return( ITEM_HANDLE_NOAMMO );
			}

			// Check if we are reloading
			if ( (gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT) )
			{
				if ( pSoldier->fReloading )
				{
					return( ITEM_HANDLE_RELOADING );
				}
			}
		}

		// Get gridno - either soldier's position or the gridno
		if ( pTargetSoldier != NULL )
		{
			sTargetGridNo	= pTargetSoldier->sGridNo;
		}
		else
		{
			sTargetGridNo	= usGridNo;
		}

		// If it's a player guy, check ChanceToGetThrough to play quote
		if ( fFromUI && (gTacticalStatus.uiFlags & TURNBASED ) && (gTacticalStatus.uiFlags & INCOMBAT) )
		{
			// Don't do if no spread!
			if ( !pSoldier->fDoSpread )
			{
				if ( !HandleCheckForBadChangeToGetThrough( pSoldier, pTargetSoldier, sTargetGridNo , bLevel ) )
				{
					return( ITEM_HANDLE_OK );
				}
			}
		}


		// Get AP COSTS
		// ATE: OK something funny going on here - AI seems to NEED FALSE here,
		// Our guys NEED TRUE. We shoulkd at some time make sure the AI and
		// our guys are deducting/checking in the same manner to avoid
		// these differences.
		sAPCost = CalcTotalAPsToAttack( pSoldier, sTargetGridNo, TRUE, pSoldier->bAimTime );


		GetAPChargeForShootOrStabWRTGunRaises( pSoldier, sTargetGridNo, TRUE, &fAddingTurningCost, &fAddingRaiseGunCost );

		// If we are standing and are asked to turn AND raise gun, ignore raise gun...
		if ( gAnimControl[ pSoldier->usAnimState ].ubHeight == ANIM_STAND )
		{
			if ( fAddingRaiseGunCost )
			{
				pSoldier->fDontChargeTurningAPs = TRUE;
			}
		}
		else
		{
			// If raising gun, don't charge turning!
		  if ( fAddingTurningCost )
			{
				pSoldier->fDontChargeReadyAPs = TRUE;
			}
		}




		// If this is a player guy, show message about no APS
		if ( EnoughPoints( pSoldier, sAPCost, 0, fFromUI ) )
		{
			if ( (pSoldier->ubProfile != NO_PROFILE) && (gMercProfiles[ pSoldier->ubProfile ].bPersonalityTrait == PSYCHO) )
			{
				// psychos might possibly switch to burst if they can
				if ( !pSoldier->bDoBurst && IsGunBurstCapable( pSoldier, HANDPOS, FALSE ) )
				{
					// chance of firing burst if we have points... chance decreasing when ordered to do aimed shot

					// temporarily set burst to true to calculate action points
					pSoldier->bDoBurst = TRUE;
					sAPCost = CalcTotalAPsToAttack( pSoldier, sTargetGridNo, TRUE, 0 );
					// reset burst mode to false (which is what it was at originally)
					pSoldier->bDoBurst = FALSE;

					if ( EnoughPoints( pSoldier, sAPCost, 0, FALSE ) )
					{
						// we have enough points to do this burst, roll the dice and see if we want to change
						if ( Random( 3 + pSoldier->bAimTime ) == 0 )
						{
							DoMercBattleSound( pSoldier, BATTLE_SOUND_LAUGH1 );
							pSoldier->bDoBurst = TRUE;
							pSoldier->bWeaponMode = WM_BURST;

        			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[ 26 ], pSoldier->name );
						}
					}

				}
			}

			// Deduct points if our target is different!
			// if attacking a new target (or if the specific target is uncertain)

		// DEF:  Made into an event
//		EVENT_FireSoldierWeapon( pSoldier, sTargetGridNo );
			if (fFromUI)
			{
				// set the target level; if the AI calls this it will have set the level already...
				pSoldier->bTargetLevel = (INT8) gsInterfaceLevel;
			}

			if ( Item[ usHandItem ].usItemClass != IC_THROWING_KNIFE )
			{
				// If doing spread, set down the first gridno.....
				if ( pSoldier->fDoSpread )
				{
					if ( pSoldier->sSpreadLocations[ 0 ] != 0 )
					{
						SendBeginFireWeaponEvent( pSoldier, pSoldier->sSpreadLocations[ 0 ] );
					}
					else
					{
						SendBeginFireWeaponEvent( pSoldier, sTargetGridNo );
					}
				}
				else
				{
					SendBeginFireWeaponEvent( pSoldier, sTargetGridNo );
				}

				// ATE: Here to make cursor go back to move after LAW shot...
				if ( fFromUI && usHandItem == ROCKET_LAUNCHER )
				{
					guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
				}

			}
			else
			{
				UINT8 ubDirection;
				// Start knife throw attack

				// Get direction
				ubDirection = (UINT8)GetDirectionFromGridNo( sTargetGridNo, pSoldier );

				EVENT_SoldierBeginKnifeThrowAttack( pSoldier, sTargetGridNo, ubDirection );

			}

			if (fFromUI)
			{
				// Descrease aim by two if in real time
				if ( (gTacticalStatus.uiFlags & REALTIME ) || !(gTacticalStatus.uiFlags & INCOMBAT) )
				{
					 //pSoldier->bShownAimTime -= 2;
					 //if ( pSoldier->bShownAimTime < REFINE_AIM_1 )
					 //{
					//		pSoldier->bShownAimTime = REFINE_AIM_1;
					 //}
					 //pSoldier->fPauseAim = TRUE;
				}

				// If in turn based - refresh aim to first level
				if ( gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT) )
				{
					pSoldier->bShownAimTime = REFINE_AIM_1;

					// Locate to soldier if he's about to shoot!
					if ( pSoldier->bTeam != gbPlayerNum  )
					{
						ShowRadioLocator( pSoldier->ubID, SHOW_LOCATOR_NORMAL );
					}

				}
			}

			// OK, set UI
			SetUIBusy( pSoldier->ubID );
		}
		else
		{
			return( ITEM_HANDLE_NOAPS );
		}

		return( ITEM_HANDLE_OK );
	}


	//TRY PUNCHING
	if ( Item[ usHandItem ].usItemClass == IC_PUNCH )
	{
		 INT16	sCnt;
		 INT16	sSpot;
		 UINT8		ubGuyThere;
		 INT16		sGotLocation = NOWHERE;
		 BOOLEAN	fGotAdjacent = FALSE;

		 for ( sCnt = 0; sCnt < NUM_WORLD_DIRECTIONS; sCnt++ )
		 {
				sSpot = NewGridNo( pSoldier->sGridNo, DirectionInc( sCnt ) );

        // Make sure movement costs are OK....
        if ( gubWorldMovementCosts[ sSpot ][ sCnt ][ bLevel ] >= TRAVELCOST_BLOCKED )
        {
          continue;
        }

				// Check for who is there...
				ubGuyThere = WhoIsThere2( sSpot, pSoldier->bLevel );

				if ( pTargetSoldier != NULL && ubGuyThere == pTargetSoldier->ubID )
				{
					// We've got a guy here....
					// Who is the one we want......
					sGotLocation = sSpot;
					sAdjustedGridNo	= pTargetSoldier->sGridNo;
					ubDirection		= ( UINT8 )sCnt;
					break;
				}
		 }

		 if ( sGotLocation == NOWHERE )
		 {
			 // See if we can get there to punch
			 sActionGridNo =  FindAdjacentGridEx( pSoldier, usGridNo, &ubDirection, &sAdjustedGridNo, TRUE, FALSE );
			 if ( sActionGridNo != -1 )
			 {
					// OK, we've got somebody...
					sGotLocation = sActionGridNo;

					fGotAdjacent = TRUE;
			 }
		 }

		 // Did we get a loaction?
		 if ( sGotLocation != NOWHERE )
		 {
				pSoldier->sTargetGridNo = usGridNo;

				pSoldier->usActionData	= usGridNo;
				// CHECK IF WE ARE AT THIS GRIDNO NOW
				if ( pSoldier->sGridNo != sGotLocation && fGotAdjacent )
				{
					// SEND PENDING ACTION
					pSoldier->ubPendingAction = MERC_PUNCH;
					pSoldier->sPendingActionData2  = sAdjustedGridNo;
					pSoldier->bPendingActionData3  = ubDirection;
					pSoldier->ubPendingActionAnimCount = 0;

					// WALK UP TO DEST FIRST
					EVENT_InternalGetNewSoldierPath( pSoldier, sGotLocation, pSoldier->usUIMovementMode, FALSE, TRUE );
				}
				else
				{
					pSoldier->bAction = AI_ACTION_KNIFE_STAB;
					EVENT_SoldierBeginPunchAttack( pSoldier, sAdjustedGridNo, ubDirection );
				}

				// OK, set UI
				SetUIBusy( pSoldier->ubID );

				gfResetUIMovementOptimization = TRUE;

				return( ITEM_HANDLE_OK );
		 }
		 else
		 {
				return( ITEM_HANDLE_CANNOT_GETTO_LOCATION );
		 }
	}

	//USING THE MEDKIT
	if ( Item[ usHandItem ].usItemClass == IC_MEDKIT )
	{
		// ATE: AI CANNOT GO THROUGH HERE!
		UINT16 usMapPos;
		BOOLEAN	fHadToUseCursorPos = FALSE;

		if (gTacticalStatus.fAutoBandageMode)
		{
			usMapPos = usGridNo;
		}
		else
		{
			GetMouseMapPos( &usMapPos );
		}

		 // See if we can get there to stab
		 sActionGridNo =  FindAdjacentGridEx( pSoldier, usGridNo, &ubDirection, &sAdjustedGridNo, TRUE, FALSE );
		 if ( sActionGridNo == -1 )
		 {
			 // Try another location...
			 sActionGridNo =  FindAdjacentGridEx( pSoldier, usMapPos, &ubDirection, &sAdjustedGridNo, TRUE, FALSE );

			 if ( sActionGridNo == -1 )
			 {
					return( ITEM_HANDLE_CANNOT_GETTO_LOCATION );
			 }

			 if ( !gTacticalStatus.fAutoBandageMode )
			 {
					fHadToUseCursorPos = TRUE;
			 }
		 }

		 // Calculate AP costs...
		 sAPCost = GetAPsToBeginFirstAid( pSoldier );
		 sAPCost += PlotPath( pSoldier, sActionGridNo, NO_COPYROUTE, FALSE, TEMPORARY, (UINT16)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints);

			if ( EnoughPoints( pSoldier, sAPCost, 0, fFromUI ) )
			{
				// OK, set UI
				SetUIBusy( pSoldier->ubID );

				// CHECK IF WE ARE AT THIS GRIDNO NOW
				if ( pSoldier->sGridNo != sActionGridNo )
				{
					// SEND PENDING ACTION
					pSoldier->ubPendingAction = MERC_GIVEAID;

					if ( fHadToUseCursorPos )
					{
						pSoldier->sPendingActionData2  = usMapPos;
					}
					else
					{
						if ( pTargetSoldier != NULL )
						{
							pSoldier->sPendingActionData2  = pTargetSoldier->sGridNo;
						}
						else
						{
							pSoldier->sPendingActionData2  = usGridNo;
						}
					}
					pSoldier->bPendingActionData3  = ubDirection;
					pSoldier->ubPendingActionAnimCount = 0;

					// WALK UP TO DEST FIRST
					EVENT_InternalGetNewSoldierPath( pSoldier, sActionGridNo, pSoldier->usUIMovementMode, FALSE, TRUE );
				}
				else
				{
					EVENT_SoldierBeginFirstAid( pSoldier, sAdjustedGridNo, ubDirection );
				}

				if ( fFromUI )
				{
					guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
				}

				return( ITEM_HANDLE_OK );
			}
		 else
		 {
				return( ITEM_HANDLE_NOAPS );
		 }
	}

  if ( usHandItem == WIRECUTTERS )
	{
		 // See if we can get there to stab
		 sActionGridNo =  FindAdjacentGridEx( pSoldier, usGridNo, &ubDirection, &sAdjustedGridNo, TRUE, FALSE );
		 if ( sActionGridNo != -1 )
		 {
				// Calculate AP costs...
				sAPCost = GetAPsToCutFence( pSoldier );
				sAPCost += PlotPath( pSoldier, sActionGridNo, NO_COPYROUTE, FALSE, TEMPORARY, (UINT16)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints);

				if ( EnoughPoints( pSoldier, sAPCost, 0, fFromUI ) )
				{
					// CHECK IF WE ARE AT THIS GRIDNO NOW
					if ( pSoldier->sGridNo != sActionGridNo )
					{
						// SEND PENDING ACTION
						pSoldier->ubPendingAction = MERC_CUTFFENCE;
						pSoldier->sPendingActionData2  = sAdjustedGridNo;
						pSoldier->bPendingActionData3  = ubDirection;
						pSoldier->ubPendingActionAnimCount = 0;

						// WALK UP TO DEST FIRST
						EVENT_InternalGetNewSoldierPath( pSoldier, sActionGridNo, pSoldier->usUIMovementMode , FALSE, TRUE );
					}
					else
					{
						EVENT_SoldierBeginCutFence( pSoldier, sAdjustedGridNo, ubDirection );
					}

					// OK, set UI
					SetUIBusy( pSoldier->ubID );

					if ( fFromUI )
					{
						guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
					}

					return( ITEM_HANDLE_OK );
			 }
			 else
			 {
					return( ITEM_HANDLE_NOAPS );
			 }
		}
		else
		{
			return( ITEM_HANDLE_CANNOT_GETTO_LOCATION );
		}
	}

  if ( usHandItem == TOOLKIT )
	{
		 UINT8 ubMercID;
		 BOOLEAN	fVehicle = FALSE;
		 INT16		sVehicleGridNo=-1;

		 // For repair, check if we are over a vehicle, then get gridnot to edge of that vehicle!
		 if ( IsRepairableStructAtGridNo( usGridNo, &ubMercID ) == 2 )
		 {
				INT16 sNewGridNo;
				UINT8	ubDirection;

				sNewGridNo = FindGridNoFromSweetSpotWithStructDataFromSoldier( pSoldier, pSoldier->usUIMovementMode, 5, &ubDirection, 0, MercPtrs[ ubMercID ] );

				if ( sNewGridNo != NOWHERE )
				{
					usGridNo = sNewGridNo;

					sVehicleGridNo = MercPtrs[ ubMercID ]->sGridNo;

					fVehicle = TRUE;
				}
		 }

		 // See if we can get there to stab
		 sActionGridNo =  FindAdjacentGridEx( pSoldier, usGridNo, &ubDirection, &sAdjustedGridNo, TRUE, FALSE );

		 if ( sActionGridNo != -1 )
		 {
				// Calculate AP costs...
				sAPCost = GetAPsToBeginRepair( pSoldier );
				sAPCost += PlotPath( pSoldier, sActionGridNo, NO_COPYROUTE, FALSE, TEMPORARY, (UINT16)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints);

				if ( EnoughPoints( pSoldier, sAPCost, 0, fFromUI ) )
				{
					// CHECK IF WE ARE AT THIS GRIDNO NOW
					if ( pSoldier->sGridNo != sActionGridNo )
					{
						// SEND PENDING ACTION
						pSoldier->ubPendingAction = MERC_REPAIR;
						pSoldier->sPendingActionData2  = sAdjustedGridNo;

						if ( fVehicle )
						{
							pSoldier->sPendingActionData2  = sVehicleGridNo;
						}

						pSoldier->bPendingActionData3  = ubDirection;
						pSoldier->ubPendingActionAnimCount = 0;

						// WALK UP TO DEST FIRST
						EVENT_InternalGetNewSoldierPath( pSoldier, sActionGridNo, pSoldier->usUIMovementMode, FALSE, TRUE );
					}
					else
					{
						EVENT_SoldierBeginRepair( pSoldier, sAdjustedGridNo, ubDirection );
					}

					// OK, set UI
					SetUIBusy( pSoldier->ubID );

					if ( fFromUI )
					{
						guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
					}

					return( ITEM_HANDLE_OK );
			 }
			 else
			 {
					return( ITEM_HANDLE_NOAPS );
			 }
		}
		else
		{
			return( ITEM_HANDLE_CANNOT_GETTO_LOCATION );
		}
	}

  if ( usHandItem == GAS_CAN )
	{
		 UINT8 ubMercID;
		 INT16		sVehicleGridNo=-1;

		 // For repair, check if we are over a vehicle, then get gridnot to edge of that vehicle!
		 if ( IsRefuelableStructAtGridNo( usGridNo, &ubMercID ) )
		 {
				INT16 sNewGridNo;
				UINT8	ubDirection;

				sNewGridNo = FindGridNoFromSweetSpotWithStructDataFromSoldier( pSoldier, pSoldier->usUIMovementMode, 5, &ubDirection, 0, MercPtrs[ ubMercID ] );

				if ( sNewGridNo != NOWHERE )
				{
					usGridNo = sNewGridNo;

					sVehicleGridNo = MercPtrs[ ubMercID ]->sGridNo;

				}
		 }

		 // See if we can get there to stab
		 sActionGridNo =  FindAdjacentGridEx( pSoldier, usGridNo, &ubDirection, &sAdjustedGridNo, TRUE, FALSE );

		 if ( sActionGridNo != -1 )
		 {
				// Calculate AP costs...
				sAPCost = GetAPsToRefuelVehicle( pSoldier );
				sAPCost += PlotPath( pSoldier, sActionGridNo, NO_COPYROUTE, FALSE, TEMPORARY, (UINT16)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints);

				if ( EnoughPoints( pSoldier, sAPCost, 0, fFromUI ) )
				{
					// CHECK IF WE ARE AT THIS GRIDNO NOW
					if ( pSoldier->sGridNo != sActionGridNo )
					{
						// SEND PENDING ACTION
						pSoldier->ubPendingAction = MERC_FUEL_VEHICLE;
						pSoldier->sPendingActionData2  = sAdjustedGridNo;

						pSoldier->sPendingActionData2  = sVehicleGridNo;
						pSoldier->bPendingActionData3  = ubDirection;
						pSoldier->ubPendingActionAnimCount = 0;

						// WALK UP TO DEST FIRST
						EVENT_InternalGetNewSoldierPath( pSoldier, sActionGridNo, pSoldier->usUIMovementMode, FALSE, TRUE );
					}
					else
					{
						EVENT_SoldierBeginRefuel( pSoldier, sAdjustedGridNo, ubDirection );
					}

					// OK, set UI
					SetUIBusy( pSoldier->ubID );

					if ( fFromUI )
					{
						guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
					}

					return( ITEM_HANDLE_OK );
			 }
			 else
			 {
					return( ITEM_HANDLE_NOAPS );
			 }
		}
		else
		{
			return( ITEM_HANDLE_CANNOT_GETTO_LOCATION );
		}
	}


  if ( usHandItem == JAR )
	{
		 sActionGridNo =  FindAdjacentGridEx( pSoldier, usGridNo, &ubDirection, &sAdjustedGridNo, TRUE, FALSE );

		 if ( sActionGridNo != -1 )
		 {
				// Calculate AP costs...
				sAPCost = GetAPsToUseJar( pSoldier, sActionGridNo );
				sAPCost += PlotPath( pSoldier, sActionGridNo, NO_COPYROUTE, FALSE, TEMPORARY, (UINT16)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints);

				if ( EnoughPoints( pSoldier, sAPCost, 0, fFromUI ) )
				{
					// CHECK IF WE ARE AT THIS GRIDNO NOW
					if ( pSoldier->sGridNo != sActionGridNo )
					{
						// SEND PENDING ACTION
						pSoldier->ubPendingAction = MERC_TAKEBLOOD;
						pSoldier->sPendingActionData2  = sAdjustedGridNo;
						pSoldier->bPendingActionData3  = ubDirection;
						pSoldier->ubPendingActionAnimCount = 0;

						// WALK UP TO DEST FIRST
						EVENT_InternalGetNewSoldierPath( pSoldier, sActionGridNo, pSoldier->usUIMovementMode, FALSE, TRUE );
					}
					else
					{
						EVENT_SoldierBeginTakeBlood( pSoldier, sAdjustedGridNo, ubDirection );
					}

					// OK, set UI
					SetUIBusy( pSoldier->ubID );

					if ( fFromUI )
					{
						guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
					}

					return( ITEM_HANDLE_OK );
			 }
			 else
			 {
					return( ITEM_HANDLE_NOAPS );
			 }
		}
		else
		{
			return( ITEM_HANDLE_CANNOT_GETTO_LOCATION );
		}
	}

  if ( usHandItem == STRING_TIED_TO_TIN_CAN )
	{
		 STRUCTURE					*pStructure;
		 LEVELNODE					*pIntTile;

		 // Get structure info for in tile!
		 pIntTile = GetCurInteractiveTileGridNoAndStructure( &usGridNo, &pStructure );

		 // We should not have null here if we are given this flag...
		 if ( pIntTile != NULL )
		 {
			 sActionGridNo =  FindAdjacentGridEx( pSoldier, usGridNo, &ubDirection, &sAdjustedGridNo, FALSE, TRUE );

			 if ( sActionGridNo != -1 )
			 {
					// Calculate AP costs...
					sAPCost = AP_ATTACH_CAN;
					sAPCost += PlotPath( pSoldier, sActionGridNo, NO_COPYROUTE, FALSE, TEMPORARY, (UINT16)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints);

					if ( EnoughPoints( pSoldier, sAPCost, 0, fFromUI ) )
					{
						// CHECK IF WE ARE AT THIS GRIDNO NOW
						if ( pSoldier->sGridNo != sActionGridNo )
						{
							// SEND PENDING ACTION
							pSoldier->ubPendingAction = MERC_ATTACH_CAN;
							pSoldier->sPendingActionData2  = usGridNo;
							pSoldier->bPendingActionData3  = ubDirection;
							pSoldier->ubPendingActionAnimCount = 0;

							// WALK UP TO DEST FIRST
							EVENT_InternalGetNewSoldierPath( pSoldier, sActionGridNo, pSoldier->usUIMovementMode, FALSE, TRUE );
						}
						else
						{
							EVENT_SoldierBeginTakeBlood( pSoldier, usGridNo, ubDirection );
						}

						// OK, set UI
						SetUIBusy( pSoldier->ubID );

						if ( fFromUI )
						{
							guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
						}

						return( ITEM_HANDLE_OK );
				 }
				 else
				 {
						return( ITEM_HANDLE_NOAPS );
				 }
			}
			else
			{
				return( ITEM_HANDLE_CANNOT_GETTO_LOCATION );
			}
		}
		else
		{
			 return( ITEM_HANDLE_CANNOT_GETTO_LOCATION );
		}
	}

	// Check for remote detonator cursor....
	if ( Item[ usHandItem ].ubCursor == REMOTECURS )
	{
		sAPCost = AP_USE_REMOTE;

		if ( EnoughPoints( pSoldier, sAPCost, 0, fFromUI ) )
		{
			DeductPoints( pSoldier, sAPCost, 0 );
			if ( usHandItem == XRAY_DEVICE )
			{
  			PlayJA2Sample(USE_X_RAY_MACHINE, SoundVolume(HIGHVOLUME, pSoldier->sGridNo), 1, SoundDir(pSoldier->sGridNo));

				ActivateXRayDevice( pSoldier );
				return( ITEM_HANDLE_OK );
			}
			else // detonator
			{
				// Save gridno....
				pSoldier->sPendingActionData2  = usGridNo;

				EVENT_SoldierBeginUseDetonator( pSoldier );

				if ( fFromUI )
				{
					guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
				}

				// Now start anim....
				return( ITEM_HANDLE_OK );
			}
		}
		else
		{
			return( ITEM_HANDLE_NOAPS );
		}
	}


	// Check for mine.. anything without a detonator.....
	if ( Item[ usHandItem ].ubCursor == BOMBCURS )
	{
		fDropBomb = TRUE;
	}

	// Check for a bomb like a mine, that uses a pressure detonator
	if ( Item[ usHandItem ].ubCursor == INVALIDCURS )
	{
		// Found detonator...
		if ( FindAttachment( &(pSoldier->inv[ pSoldier->ubAttackingHand ] ), DETONATOR) != ITEM_NOT_FOUND || FindAttachment( &(pSoldier->inv[ pSoldier->ubAttackingHand ] ), REMDETONATOR ) != ITEM_NOT_FOUND )
		{
			fDropBomb = TRUE;
		}
	}

	if ( fDropBomb )
	{
		// Save gridno....
		pSoldier->sPendingActionData2  = usGridNo;

		if ( pSoldier->sGridNo != usGridNo )
		{
			// SEND PENDING ACTION
			pSoldier->ubPendingAction = MERC_DROPBOMB;
			pSoldier->ubPendingActionAnimCount = 0;

			// WALK UP TO DEST FIRST
			EVENT_InternalGetNewSoldierPath( pSoldier, usGridNo, pSoldier->usUIMovementMode, FALSE, TRUE );
		}
		else
		{
			EVENT_SoldierBeginDropBomb( pSoldier );
		}

		// OK, set UI
		SetUIBusy( pSoldier->ubID );

		if ( fFromUI )
		{
			guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
		}

		return( ITEM_HANDLE_OK );
	}

	//USING THE BLADE
	if ( Item[ usHandItem ].usItemClass == IC_BLADE )
	{
		 // See if we can get there to stab
		 if ( pSoldier->ubBodyType == BLOODCAT )
		 {
			sActionGridNo =  FindNextToAdjacentGridEx( pSoldier, usGridNo, &ubDirection, &sAdjustedGridNo, TRUE, FALSE );
		 }
		 else if ( CREATURE_OR_BLOODCAT( pSoldier ) && PythSpacesAway( pSoldier->sGridNo, usGridNo ) > 1 )
		 {
			sActionGridNo =  FindNextToAdjacentGridEx( pSoldier, usGridNo, &ubDirection, &sAdjustedGridNo, TRUE, FALSE );
			if (sActionGridNo == -1)
			{
			 sActionGridNo =  FindAdjacentGridEx( pSoldier, usGridNo, &ubDirection, &sAdjustedGridNo, TRUE, FALSE );
			}
		 }
		 else
		 {
			sActionGridNo =  FindAdjacentGridEx( pSoldier, usGridNo, &ubDirection, &sAdjustedGridNo, TRUE, FALSE );
		 }

		 if ( sActionGridNo != -1 )
		 {
				pSoldier->usActionData = sActionGridNo;

				// CHECK IF WE ARE AT THIS GRIDNO NOW
				if ( pSoldier->sGridNo != sActionGridNo )
				{
					// SEND PENDING ACTION
					pSoldier->ubPendingAction = MERC_KNIFEATTACK;
					pSoldier->sPendingActionData2  = sAdjustedGridNo;
					pSoldier->bPendingActionData3  = ubDirection;
					pSoldier->ubPendingActionAnimCount = 0;

					// WALK UP TO DEST FIRST
					EVENT_InternalGetNewSoldierPath( pSoldier, sActionGridNo, pSoldier->usUIMovementMode, FALSE, TRUE );
				}
				else
				{
					// for the benefit of the AI
					pSoldier->bAction = AI_ACTION_KNIFE_STAB;
					EVENT_SoldierBeginBladeAttack( pSoldier, sAdjustedGridNo, ubDirection );
				}

				// OK, set UI
				SetUIBusy( pSoldier->ubID );

				if ( fFromUI )
				{
					guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
					gfResetUIMovementOptimization = TRUE;
				}

				return( ITEM_HANDLE_OK );
		 }
		 else
		 {
				return( ITEM_HANDLE_CANNOT_GETTO_LOCATION );
		 }
	}


	if ( Item[ usHandItem ].usItemClass == IC_TENTACLES )
	{
		 // See if we can get there to stab
		 //pSoldier->sTargetGridNo = sTargetGridNo;
		 //pSoldier->sLastTarget = sTargetGridNo;
		 //pSoldier->ubTargetID = WhoIsThere2( sTargetGridNo, pSoldier->bTargetLevel );

		 gTacticalStatus.ubAttackBusyCount++;
		 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("!!!!!!! Starting swipe attack, incrementing a.b.c in HandleItems to %d", gTacticalStatus.ubAttackBusyCount) );


 	   sAPCost = CalcTotalAPsToAttack( pSoldier, sGridNo, FALSE, pSoldier->bAimTime );

		 DeductPoints( pSoldier, sAPCost, 0 );

		 EVENT_InitNewSoldierAnim( pSoldier, QUEEN_SWIPE, 0 , FALSE );

		 //FireWeapon( pSoldier, sTargetGridNo );
		 pSoldier->bAction = AI_ACTION_KNIFE_STAB;

		 return( ITEM_HANDLE_OK );
	}


	// THIS IS IF WE WERE FROM THE UI
	if ( Item[ usHandItem ].usItemClass == IC_GRENADE || Item[ usHandItem ].usItemClass == IC_LAUNCHER || Item[ usHandItem ].usItemClass == IC_THROWN )
	{
		INT16 sCheckGridNo;

		// Get gridno - either soldier's position or the gridno
		if ( pTargetSoldier != NULL )
		{
			sTargetGridNo	= pTargetSoldier->sGridNo;
		}
		else
		{
			sTargetGridNo	= usGridNo;
		}

		sAPCost = MinAPsToAttack( pSoldier, sTargetGridNo, TRUE );

		// Check if these is room to place mortar!
		if ( usHandItem == MORTAR )
		{
			ubDirection = (UINT8)GetDirectionFromGridNo( sTargetGridNo, pSoldier );

			// Get new gridno!
			sCheckGridNo = NewGridNo( (UINT16)pSoldier->sGridNo, (UINT16)DirectionInc( ubDirection ) );

			if ( !OKFallDirection( pSoldier, sCheckGridNo, pSoldier->bLevel, ubDirection , pSoldier->usAnimState ) )
			{
				return( ITEM_HANDLE_NOROOM );
			}

			pSoldier->fDontChargeAPsForStanceChange = TRUE;
		}
		else if ( usHandItem == GLAUNCHER || usHandItem == UNDER_GLAUNCHER )
		{
			GetAPChargeForShootOrStabWRTGunRaises( pSoldier, sTargetGridNo, TRUE, &fAddingTurningCost, &fAddingRaiseGunCost );

			// If we are standing and are asked to turn AND raise gun, ignore raise gun...
			if ( gAnimControl[ pSoldier->usAnimState ].ubHeight == ANIM_STAND )
			{
				if ( fAddingRaiseGunCost )
				{
					pSoldier->fDontChargeTurningAPs = TRUE;
				}
			}
			else
			{
				// If raising gun, don't charge turning!
				if ( fAddingTurningCost )
				{
					pSoldier->fDontChargeReadyAPs = TRUE;
				}
			}
		}

		// If this is a player guy, show message about no APS
		if ( EnoughPoints( pSoldier, sAPCost, 0, fFromUI ) )
		{
			pSoldier->ubAttackingHand = HANDPOS;
			pSoldier->usAttackingWeapon = usHandItem;
			pSoldier->bTargetLevel    = bLevel;

			// Look at the cursor, if toss cursor...
			if ( Item[ usHandItem ].ubCursor == TOSSCURS )
			{
				pSoldier->sTargetGridNo = sTargetGridNo;
				//	pSoldier->sLastTarget = sTargetGridNo;
				pSoldier->ubTargetID = WhoIsThere2( sTargetGridNo, pSoldier->bTargetLevel );

				// Increment attack counter...
				gTacticalStatus.ubAttackBusyCount++;

				// ATE: Don't charge turning...
				pSoldier->fDontChargeTurningAPs = TRUE;

				FireWeapon( pSoldier, sTargetGridNo );
			}
			else
			{

				SendBeginFireWeaponEvent( pSoldier, sTargetGridNo );

			}

			// OK, set UI
			SetUIBusy( pSoldier->ubID );

			return( ITEM_HANDLE_OK );

		}
		else
		{
			return( ITEM_HANDLE_NOAPS );
		}

		return( ITEM_HANDLE_OK );
	}

	// CHECK FOR BOMB....
	if ( Item[ usHandItem ].ubCursor == INVALIDCURS )
	{
		// Found detonator...
		if ( FindAttachment( &(pSoldier->inv[ usHandItem ] ), DETONATOR) != ITEM_NOT_FOUND || FindAttachment( &(pSoldier->inv[ usHandItem ] ), REMDETONATOR ) )
		{
			StartBombMessageBox( pSoldier, usGridNo );

			if ( fFromUI )
			{
				guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
			}

			return( ITEM_HANDLE_OK );
		}
	}

	return( ITEM_HANDLE_OK );
}


void HandleSoldierDropBomb( SOLDIERTYPE *pSoldier, INT16 sGridNo )
{
	// Does this have detonator that needs info?
	if ( FindAttachment( &(pSoldier->inv[ HANDPOS ] ), DETONATOR) != ITEM_NOT_FOUND || FindAttachment( &(pSoldier->inv[ HANDPOS ] ), REMDETONATOR ) != ITEM_NOT_FOUND )
	{
		StartBombMessageBox( pSoldier, sGridNo );
	}
	else
	{
		// We have something... all we do is place...
		if ( ArmBomb( &(pSoldier->inv[ HANDPOS ]), 0 ) )
		{
			// EXPLOSIVES GAIN (25):  Place a bomb, or buried and armed a mine
			StatChange( pSoldier, EXPLODEAMT, 25, FALSE );

			pSoldier->inv[ HANDPOS ].bTrap = __min( 10, ( EffectiveExplosive( pSoldier ) / 20) + (EffectiveExpLevel( pSoldier ) / 3) );
			pSoldier->inv[ HANDPOS ].ubBombOwner = pSoldier->ubID + 2;

			// we now know there is something nasty here
			gpWorldLevelData[ sGridNo ].uiFlags |= MAPELEMENT_PLAYER_MINE_PRESENT;

			AddItemToPool( sGridNo, &(pSoldier->inv[ HANDPOS ] ), BURIED, pSoldier->bLevel, WORLD_ITEM_ARMED_BOMB, 0 );
			DeleteObj( &(pSoldier->inv[ HANDPOS ]) );
		}
	}
}

void HandleSoldierUseRemote( SOLDIERTYPE *pSoldier, INT16 sGridNo )
{
	StartBombMessageBox( pSoldier, sGridNo );
}

void SoldierHandleDropItem( SOLDIERTYPE *pSoldier )
{
	// LOOK IN PANDING DATA FOR ITEM TO DROP, AND LOCATION
	if ( pSoldier->pTempObject != NULL )
	{
		if ( pSoldier->bVisible != -1 )
		{
			PlayJA2Sample(THROW_IMPACT_2, SoundVolume(MIDVOLUME, pSoldier->sGridNo), 1, SoundDir(pSoldier->sGridNo));
		}

		AddItemToPool( pSoldier->sGridNo, pSoldier->pTempObject, 1, pSoldier->bLevel, 0 , -1 );
		NotifySoldiersToLookforItems( );

		MemFree( pSoldier->pTempObject );
		pSoldier->pTempObject = NULL;
	}
}


void HandleSoldierThrowItem( SOLDIERTYPE *pSoldier, INT16 sGridNo )
{
	// Determine what to do
	UINT8 ubDirection;

	// Set attacker to NOBODY, since it's not a combat attack
	pSoldier->ubTargetID = NOBODY;

	// Alrighty, switch based on stance!
	switch( gAnimControl[ pSoldier->usAnimState ].ubHeight )
	{
		case ANIM_STAND:

			// CHECK IF WE ARE NOT ON THE SAME GRIDNO
			if ( sGridNo == pSoldier->sGridNo )
			{
				PickDropItemAnimation( pSoldier );
			}
			else
			{
				// CHANGE DIRECTION AT LEAST
				ubDirection = (UINT8)GetDirectionFromGridNo( sGridNo, pSoldier );

				SoldierGotoStationaryStance( pSoldier );

				EVENT_SetSoldierDesiredDirection( pSoldier, ubDirection );
				pSoldier->fTurningUntilDone = TRUE;

				// Draw item depending on distance from buddy
				if ( GetRangeFromGridNoDiff( sGridNo, pSoldier->sGridNo ) < MIN_LOB_RANGE )
				{
					pSoldier->usPendingAnimation = LOB_ITEM;
				}
				else
				{
					pSoldier->usPendingAnimation = THROW_ITEM;
				}

			}
			break;

		case ANIM_CROUCH:
		case ANIM_PRONE:

			// CHECK IF WE ARE NOT ON THE SAME GRIDNO
			if ( sGridNo == pSoldier->sGridNo )
			{
				// OK, JUST DROP ITEM!
				if ( pSoldier->pTempObject != NULL )
				{
					AddItemToPool( sGridNo, pSoldier->pTempObject, 1, pSoldier->bLevel, 0, -1 );
					NotifySoldiersToLookforItems( );

					MemFree( pSoldier->pTempObject );
					pSoldier->pTempObject = NULL;
				}
			}
			else
			{
				// OK, go from prone/crouch to stand first!
				ubDirection = (UINT8)GetDirectionFromGridNo( sGridNo, pSoldier );
				EVENT_SetSoldierDesiredDirection( pSoldier, ubDirection );

				ChangeSoldierState( pSoldier, THROW_ITEM, 0 , FALSE );
			}
	}

}


void SoldierGiveItem( SOLDIERTYPE *pSoldier, SOLDIERTYPE *pTargetSoldier, OBJECTTYPE *pObject, INT8 bInvPos )
{
	INT16 sActionGridNo, sAdjustedGridNo;
	UINT8	ubDirection;

	 // Remove any previous actions
	 pSoldier->ubPendingAction		 = NO_PENDING_ACTION;

	 // See if we can get there to stab
	 sActionGridNo =  FindAdjacentGridEx( pSoldier, pTargetSoldier->sGridNo, &ubDirection, &sAdjustedGridNo, TRUE, FALSE );
	 if ( sActionGridNo != -1 )
	 {
			// SEND PENDING ACTION
			pSoldier->ubPendingAction = MERC_GIVEITEM;

			pSoldier->bPendingActionData5 = bInvPos;
			// Copy temp object
			pSoldier->pTempObject	= MemAlloc( sizeof( OBJECTTYPE ) );
			*pSoldier->pTempObject = *pObject;


			pSoldier->sPendingActionData2  = pTargetSoldier->sGridNo;
			pSoldier->bPendingActionData3  = ubDirection;
			pSoldier->uiPendingActionData4 = pTargetSoldier->ubID;
			pSoldier->ubPendingActionAnimCount = 0;

			// Set soldier as engaged!
			pSoldier->uiStatusFlags |= SOLDIER_ENGAGEDINACTION;

			// CHECK IF WE ARE AT THIS GRIDNO NOW
			if ( pSoldier->sGridNo != sActionGridNo )
			{
				// WALK UP TO DEST FIRST
				EVENT_InternalGetNewSoldierPath( pSoldier, sActionGridNo, pSoldier->usUIMovementMode, FALSE, TRUE );
			}
			else
			{
				EVENT_SoldierBeginGiveItem( pSoldier );
			  // CHANGE DIRECTION OF TARGET TO OPPOSIDE DIRECTION!
			  EVENT_SetSoldierDesiredDirection( pSoldier, ubDirection );
			}

			// Set target as engaged!
			pTargetSoldier->uiStatusFlags |= SOLDIER_ENGAGEDINACTION;
	 }
}

BOOLEAN SoldierDropItem( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj )
{
	pSoldier->pTempObject = MemAlloc( sizeof( OBJECTTYPE ) );
	if (pSoldier->pTempObject == NULL)
	{
		// OUT OF MEMORY! YIKES!
		return( FALSE );
	}
	*pSoldier->pTempObject = *pObj;
	PickDropItemAnimation( pSoldier );
	return( TRUE );
}

void SoldierPickupItem( SOLDIERTYPE *pSoldier, INT32 iItemIndex, INT16 sGridNo, INT8 bZLevel )
{
	INT16							sActionGridNo;

	// Remove any previous actions
	pSoldier->ubPendingAction		 = NO_PENDING_ACTION;

	sActionGridNo = AdjustGridNoForItemPlacement( pSoldier, sGridNo );

	// SET PENDING ACTIONS!
	pSoldier->ubPendingAction = MERC_PICKUPITEM;
	pSoldier->uiPendingActionData1 = iItemIndex;
	pSoldier->sPendingActionData2  = sActionGridNo;
	pSoldier->uiPendingActionData4 = sGridNo;
	pSoldier->bPendingActionData3  = bZLevel;
	pSoldier->ubPendingActionAnimCount = 0;

	// Deduct points!
	//sAPCost = GetAPsToPickupItem( pSoldier, sGridNo );
	//DeductPoints( pSoldier, sAPCost, 0 );
	SetUIBusy( pSoldier->ubID );

	// CHECK IF NOT AT SAME GRIDNO
	if ( pSoldier->sGridNo != sActionGridNo )
	{
		if ( pSoldier->bTeam == gbPlayerNum )
		{
			EVENT_InternalGetNewSoldierPath( pSoldier, sActionGridNo, pSoldier->usUIMovementMode, TRUE, TRUE );

			// Say it only if we don;t have to go too far!
			if ( pSoldier->usPathDataSize > 5 )
			{
				DoMercBattleSound(  pSoldier, BATTLE_SOUND_OK1 );
			}
		}
		else
		{
			EVENT_InternalGetNewSoldierPath( pSoldier, sActionGridNo, pSoldier->usUIMovementMode, FALSE, TRUE );
		}
	}
	else
	{
		// DO ANIMATION OF PICKUP NOW!
		PickPickupAnimation( pSoldier, pSoldier->uiPendingActionData1, (INT16)( pSoldier->uiPendingActionData4 ), pSoldier->bPendingActionData3 );
	}
}


static void HandleAutoPlaceFail(SOLDIERTYPE* pSoldier, INT32 iItemIndex, INT16 sGridNo)
{
	if (pSoldier->bTeam == gbPlayerNum)
	{
		// Place it in buddy's hand!
		if ( gpItemPointer == NULL )
		{
			InternalBeginItemPointer( pSoldier, &(gWorldItems[ iItemIndex ].o ), NO_SLOT );
		}
		else
		{
			// Add back to world...
			AddItemToPool( sGridNo, &(gWorldItems[ iItemIndex ].o ), 1 , pSoldier->bLevel, 0, -1 );

			// If we are a merc, say DAMN quote....
			if ( pSoldier->bTeam == gbPlayerNum )
			{
				DoMercBattleSound( pSoldier, BATTLE_SOUND_CURSE1 );
			}
		}
	}
}


static void CheckForPickedOwnership(void);
static BOOLEAN ContinuePastBoobyTrap(SOLDIERTYPE* pSoldier, INT16 sGridNo, INT8 bLevel, INT32 iItemIndex, BOOLEAN fInStrategic, BOOLEAN* pfSaidQuote);
static BOOLEAN ItemExistsAtLocation(INT16 sGridNo, INT32 iItemIndex, UINT8 ubLevel);
static BOOLEAN ItemPoolOKForPickup(SOLDIERTYPE* pSoldier, ITEM_POOL* pItemPool, INT8 bZLevel);
static BOOLEAN LookForHiddenItems(INT16 sGridNo, INT8 ubLevel, BOOLEAN fSetLocator, INT8 bZLevel);
static void SwitchMessageBoxCallBack(UINT8 ubExitValue);


void SoldierGetItemFromWorld( SOLDIERTYPE *pSoldier, INT32 iItemIndex, INT16 sGridNo, INT8 bZLevel, BOOLEAN *pfSelectionList )
{
	ITEM_POOL		*		pItemPool;
	ITEM_POOL		*		pItemPoolToDelete = NULL;
	OBJECTTYPE			Object;
	INT32						cnt = 0;
	BOOLEAN					fPickup;
	BOOLEAN					fFailedAutoPlace = FALSE;
	INT32						iItemIndexToDelete;
	BOOLEAN					fShouldSayCoolQuote = FALSE;
	BOOLEAN					fDidSayCoolQuote = FALSE;
  BOOLEAN         fSaidBoobyTrapQuote = FALSE;

	// OK. CHECK IF WE ARE DOING ALL IN THIS POOL....
	if ( iItemIndex == ITEM_PICKUP_ACTION_ALL || iItemIndex == ITEM_PICKUP_SELECTION )
	{
		// DO all pickup!
		// LOOP THROUGH LIST TO FIND NODE WE WANT
		GetItemPool( sGridNo, &pItemPool, pSoldier->bLevel );

		while( pItemPool )
		{
			if ( ItemPoolOKForPickup( pSoldier, pItemPool, bZLevel ) )
			{
				fPickup = TRUE;

				if ( iItemIndex == ITEM_PICKUP_SELECTION )
				{
					if ( !pfSelectionList[ cnt ] )
					{
						fPickup = FALSE;
					}
				}

				// Increment counter...
				//:ATE: Only incremrnt counter for items we can see..
				cnt++;

				if ( fPickup )
				{
					if ( ContinuePastBoobyTrap( pSoldier, sGridNo, bZLevel, pItemPool->iItemIndex, FALSE, &fSaidBoobyTrapQuote ) )
					{
						// Make copy of item
						Object = gWorldItems[pItemPool->iItemIndex].o;

						if ( ItemIsCool( &Object ) )
						{
							fShouldSayCoolQuote = TRUE;
						}

						if (Object.usItem == SWITCH)
						{
							// ask about activating the switch!
							bTempFrequency = Object.bFrequency;
							gpTempSoldier = pSoldier;
							DoMessageBox( MSG_BOX_BASIC_STYLE, TacticalStr[ ACTIVATE_SWITCH_PROMPT ] , GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_YESNO, SwitchMessageBoxCallBack, NULL );
							pItemPool = pItemPool->pNext;
						}
						else
						{
							if ( !AutoPlaceObject( pSoldier, &Object, TRUE ) )
							{
								// check to see if the object has been swapped with one in inventory
								if ( Object.usItem != gWorldItems[ pItemPool->iItemIndex ].o.usItem || Object.ubNumberOfObjects != gWorldItems[ pItemPool->iItemIndex ].o.ubNumberOfObjects )
								{
									// copy back because item changed, and we must make sure the item pool reflects this.
									gWorldItems[pItemPool->iItemIndex].o = Object;
								}

								pItemPoolToDelete = pItemPool;
								pItemPool = pItemPool->pNext;
								fFailedAutoPlace = TRUE;
								// continue, to try and place ay others...
								continue;
							}
							/*
							// handle theft.. will return true if theft has failed ( if soldier was caught )
							if( pSoldier->bTeam == OUR_TEAM )
							{
								// check to see if object was owned by another
								if( Object.fFlags & OBJECT_OWNED_BY_CIVILIAN )
								{
									// owned by a civilian
									if( HandleLoyaltyAdjustmentForRobbery( pSoldier ) == TRUE )
									{
										// implememnt actual tactical reaction for theft..shoot robber, yell out, etc
									}

									// reset who owns object
									Object.fFlags &= ~( OBJECT_OWNED_BY_CIVILIAN );
								}
							}
							*/

							//pItemPoolToDelete = pItemPool;
							iItemIndexToDelete = pItemPool->iItemIndex;
							pItemPool = pItemPool->pNext;
							RemoveItemFromPool( sGridNo, iItemIndexToDelete, pSoldier->bLevel );
						}
					}
					else
					{
						// boobytrap found... stop picking up things!
						break;
					}
				}
				else
				{
					pItemPool = pItemPool->pNext;
				}
			}
			else
			{
				pItemPool = pItemPool->pNext;
			}
		}

		// ATE; If here, and we failed to add any more stuff, put failed one in our cursor...
		if ( pItemPoolToDelete != NULL && fFailedAutoPlace )
		{
			gfDontChargeAPsToPickup = TRUE;
			HandleAutoPlaceFail( pSoldier, pItemPoolToDelete->iItemIndex, sGridNo );
			RemoveItemFromPool( sGridNo, pItemPoolToDelete->iItemIndex, pSoldier->bLevel );
			pItemPoolToDelete = NULL;
		}
	}
	else
	{
		// REMOVE ITEM FROM POOL
		if ( ItemExistsAtLocation( sGridNo, iItemIndex, pSoldier->bLevel ) )
		{

			if ( ContinuePastBoobyTrap( pSoldier, sGridNo, bZLevel, iItemIndex, FALSE, &fSaidBoobyTrapQuote ) )
			{

				// Make copy of item
				Object = gWorldItems[iItemIndex].o;

				if ( ItemIsCool( &Object ) )
				{
					fShouldSayCoolQuote = TRUE;
				}

				if (Object.usItem == SWITCH)
				{
					// handle switch
					bTempFrequency = Object.bFrequency;
					gpTempSoldier = pSoldier;
					DoMessageBox( MSG_BOX_BASIC_STYLE, TacticalStr[ ACTIVATE_SWITCH_PROMPT ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_YESNO, SwitchMessageBoxCallBack, NULL );
				}
				else
				{
/*
					// handle theft.. will return true if theft has failed ( if soldier was caught )
					if( pSoldier->bTeam == OUR_TEAM )
					{
						// check to see if object was owned by another
						if( Object.fFlags & OBJECT_OWNED_BY_CIVILIAN )
						{
							// owned by a civilian
							if( HandleLoyaltyAdjustmentForRobbery( pSoldier ) == TRUE )
							{
								// implememnt actual tactical reaction for theft..shoot robber, yell out, etc
							}

							// reset who owns object
							Object.fFlags &= ~( OBJECT_OWNED_BY_CIVILIAN );
						}
					}
*/
					RemoveItemFromPool( sGridNo, iItemIndex, pSoldier->bLevel );

					if ( !AutoPlaceObject( pSoldier, &(gWorldItems[ iItemIndex ].o ), TRUE ) )
					{
						gfDontChargeAPsToPickup = TRUE;
						HandleAutoPlaceFail( pSoldier, iItemIndex, sGridNo );
					}
				}

			}
		}
	}

	// OK, check if potentially a good candidate for cool quote
	if ( fShouldSayCoolQuote && pSoldier->bTeam == gbPlayerNum )
	{
		// Do we have this quote..?
		if ( QuoteExp_GotGunOrUsedGun[ pSoldier->ubProfile ] == QUOTE_FOUND_SOMETHING_SPECIAL )
		{
			// Have we not said it today?
			if ( !( pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_FOUND_SOMETHING_NICE ) )
			{
				// set flag
				pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_FOUND_SOMETHING_NICE;

				// Say it....
				// We've found something!
				TacticalCharacterDialogue( pSoldier, QUOTE_FOUND_SOMETHING_SPECIAL );

				fDidSayCoolQuote = TRUE;
			}
		}
	}

	// Aknowledge....
	if( pSoldier->bTeam == OUR_TEAM && !fDidSayCoolQuote && !fSaidBoobyTrapQuote )
	{
		DoMercBattleSound( pSoldier, BATTLE_SOUND_GOTIT );
	}


	// OK partner......look for any hidden items!
	if ( pSoldier->bTeam == gbPlayerNum && LookForHiddenItems( sGridNo, pSoldier->bLevel, TRUE, 0 ) )
	{
	  // WISDOM GAIN (5):  Found a hidden object
    StatChange( pSoldier, WISDOMAMT, 5, FALSE );

		// We've found something!
		TacticalCharacterDialogue( pSoldier, (UINT16)( QUOTE_SPOTTED_SOMETHING_ONE + Random( 2 ) ) );
	}

	gpTempSoldier = pSoldier;
	gsTempGridno = sGridNo;
	SetCustomizableTimerCallbackAndDelay( 1000, CheckForPickedOwnership, TRUE );
}


static void BoobyTrapMessageBoxCallBack(UINT8 ubExitValue);
static BOOLEAN DoesItemPoolContainAllHiddenItems(ITEM_POOL* pItemPool);
static ITEM_POOL* GetItemPoolForIndex(INT16 sGridNo, INT32 iItemIndex, UINT8 ubLevel);
static INT16 GetNumOkForDisplayItemsInPool(ITEM_POOL* pItemPool, INT8 bZLevel);


void HandleSoldierPickupItem( SOLDIERTYPE *pSoldier, INT32 iItemIndex, INT16 sGridNo, INT8 bZLevel )
{
	ITEM_POOL		*pItemPool;
	UINT16				usNum;

	// Draw menu if more than one item!
	if ( GetItemPool( sGridNo, &pItemPool, pSoldier->bLevel ) )
	{
		// OK, if an enemy, go directly ( skip menu )
		if ( pSoldier->bTeam != gbPlayerNum )
		{
			SoldierGetItemFromWorld( pSoldier, iItemIndex, sGridNo, bZLevel, NULL );
		}
		else
		{
			if (gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_PLAYER_MINE_PRESENT)
			{
				// have the computer ask us if we want to proceed

				// override the item index passed in with the one for the bomb in this
				// tile
				iItemIndex = FindWorldItemForBombInGridNo( sGridNo, pSoldier->bLevel );
#ifdef JA2TESTVERSION
				if (iItemIndex == -1)
				{
					// WTF????
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_ERROR, L"Cannot find bomb item in gridno %d", sGridNo );
					return;
				}
#endif

				gpBoobyTrapItemPool = GetItemPoolForIndex( sGridNo, iItemIndex, pSoldier->bLevel );
				gpBoobyTrapSoldier = pSoldier;
				gsBoobyTrapGridNo = sGridNo;
				gbBoobyTrapLevel  = pSoldier->bLevel;
				gfDisarmingBuriedBomb = TRUE;
				gbTrapDifficulty = gWorldItems[ iItemIndex ].o.bTrap;

				DoMessageBox( MSG_BOX_BASIC_STYLE, TacticalStr[ DISARM_TRAP_PROMPT ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_YESNO, BoobyTrapMessageBoxCallBack, NULL );
			}
			else
			{
				// OK, only hidden items exist...
				if ( pSoldier->bTeam == gbPlayerNum && DoesItemPoolContainAllHiddenItems( pItemPool ) )
				{
					// He's touched them....
					if ( LookForHiddenItems( sGridNo, pSoldier->bLevel, TRUE, 0 ) )
					{
						// WISDOM GAIN (5):  Found a hidden object
						StatChange( pSoldier, WISDOMAMT, 5, FALSE );

						// We've found something!
						TacticalCharacterDialogue( pSoldier, (UINT16)( QUOTE_SPOTTED_SOMETHING_ONE + Random( 2 ) ) );
					}
					else
					{
						// Say NOTHING quote...
						DoMercBattleSound( pSoldier, BATTLE_SOUND_NOTHING );
					}
				}
				else
				{
					// If only one good item exists....
					if ( ( usNum = GetNumOkForDisplayItemsInPool( pItemPool, bZLevel ) ) == 1 )
					{
						// Find first OK item....
						while( !ItemPoolOKForDisplay( pItemPool, bZLevel  ) )
						{
							pItemPool = pItemPool->pNext;
						}
						SoldierGetItemFromWorld( pSoldier, pItemPool->iItemIndex, sGridNo, bZLevel, NULL );
					}
					else
					{
						if ( usNum != 0 )
						{
							// Freeze guy!
							pSoldier->fPauseAllAnimation = TRUE;

							InitializeItemPickupMenu( pSoldier, sGridNo, pItemPool, 0, 0, bZLevel );

							guiPendingOverrideEvent = G_GETTINGITEM;
						}
						else
						{
							DoMercBattleSound( pSoldier, BATTLE_SOUND_NOTHING );
						}
					}
				}
			}
		}
	}
	else
	{
		// Say NOTHING quote...
		DoMercBattleSound( pSoldier, BATTLE_SOUND_NOTHING );
	}

}


static LEVELNODE* AddItemGraphicToWorld(INVTYPE* pItem, INT16 sGridNo, UINT8 ubLevel)
{
	UINT16			usTileIndex;
	LEVELNODE		*pNode;

	usTileIndex = GetTileGraphicForItem( pItem );

	// OK, Do stuff differently base on level!
	if ( ubLevel == 0 )
	{
		pNode = AddStructToTail( sGridNo, usTileIndex );
		//SET FLAG FOR AN ITEM
		pNode->uiFlags |= LEVELNODE_ITEM;
	}
	else
	{
		AddOnRoofToHead( sGridNo, usTileIndex );
		//SET FLAG FOR AN ITEM
		pNode = gpWorldLevelData[ sGridNo ].pOnRoofHead;
		pNode->uiFlags |= LEVELNODE_ITEM;
	}

	// DIRTY INTERFACE
	fInterfacePanelDirty = DIRTYLEVEL2;

	// DIRTY TILE
	gpWorldLevelData[ sGridNo ].uiFlags |= MAPELEMENT_REDRAW;
	SetRenderFlags(RENDER_FLAG_MARKED);

	return( pNode );
}


static void RemoveItemGraphicFromWorld(INVTYPE* pItem, INT16 sGridNo, UINT8 ubLevel, LEVELNODE* pLevelNode)
{
	LEVELNODE *pNode;

	// OK, Do stuff differently base on level!
	// Loop through and find pointer....
	if ( ubLevel == 0 )
	{
		pNode = gpWorldLevelData[ sGridNo ].pStructHead;
	}
	else
	{
		pNode = gpWorldLevelData[ sGridNo ].pOnRoofHead;
	}

	while( pNode != NULL )
	{
		if ( pNode == pLevelNode )
		{
			// Found one!
			if ( ubLevel == 0 )
			{
				RemoveStructFromLevelNode( sGridNo, pNode );
			}
			else
			{
				RemoveOnRoofFromLevelNode( sGridNo, pNode );
			}

			break;
		}

		pNode = pNode->pNext;
	}

	// DIRTY INTERFACE
	fInterfacePanelDirty = DIRTYLEVEL2;

	// DIRTY TILE
	gpWorldLevelData[ sGridNo ].uiFlags |= MAPELEMENT_REDRAW;
	SetRenderFlags(RENDER_FLAG_MARKED);

	//TEMP RENDER FULL!!!
	SetRenderFlags(RENDER_FLAG_FULL);
}

// INVENTORY POOL STUFF
OBJECTTYPE* AddItemToPool( INT16 sGridNo, OBJECTTYPE *pObject, INT8 bVisible, UINT8 ubLevel, UINT16 usFlags, INT8 bRenderZHeightAboveLevel )
{
	return InternalAddItemToPool( &sGridNo, pObject, bVisible, ubLevel, usFlags, bRenderZHeightAboveLevel, NULL );
}

OBJECTTYPE * AddItemToPoolAndGetIndex( INT16 sGridNo, OBJECTTYPE *pObject, INT8 bVisible, UINT8 ubLevel, UINT16 usFlags, INT8 bRenderZHeightAboveLevel, INT32 * piItemIndex )
{
	return( InternalAddItemToPool( &sGridNo, pObject, bVisible, ubLevel, usFlags, bRenderZHeightAboveLevel, piItemIndex ) );
}


static void AdjustItemPoolVisibility(ITEM_POOL* pItemPool);


OBJECTTYPE* InternalAddItemToPool( INT16 *psGridNo, OBJECTTYPE *pObject, INT8 bVisible, UINT8 ubLevel, UINT16 usFlags, INT8 bRenderZHeightAboveLevel, INT32 * piItemIndex )
{
	ITEM_POOL		*pItemPool;
	ITEM_POOL		*pItemPoolTemp;
	INT32				iWorldItem;
	STRUCTURE		*pStructure, *pBase;
	INT16				sDesiredLevel;
	INT16       sNewGridNo = *psGridNo;
	LEVELNODE		*pNode;
	BOOLEAN			fForceOnGround = FALSE;
	BOOLEAN			fObjectInOpenable = FALSE;
  INT8        bTerrainID;

	Assert( pObject->ubNumberOfObjects <= MAX_OBJECTS_PER_SLOT);

	// ATE: Check if the gridno is OK
	if ( (*psGridNo) == NOWHERE )
	{
		// Display warning.....
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Error: Item %d was given invalid grid location %d for item pool. Please Report.", pObject->usItem, (*psGridNo) );

    (*psGridNo) = sNewGridNo = gMapInformation.sCenterGridNo;

		//return( NULL );
	}

	// CHECK IF THIS ITEM IS IN DEEP WATER....
	// IF SO, CHECK IF IT SINKS...
	// IF SO, DONT'T ADD!
	bTerrainID = GetTerrainType( *psGridNo );

  if ( bTerrainID == DEEP_WATER || bTerrainID == LOW_WATER || bTerrainID == MED_WATER )
  {
		if ( Item[ pObject->usItem ].fFlags & ITEM_SINKS )
		{
			return( NULL );
		}
	}

	// First things first - look at where we are to place the items, and
	// set some flags appropriately

	// On a structure?
	//Locations on roofs without a roof is not possible, so
	//we convert the onroof intention to ground.
	if( ubLevel && !FlatRoofAboveGridNo( *psGridNo ) )
	{
		ubLevel = 0;
	}

	if ( bRenderZHeightAboveLevel == -1 )
	{
		fForceOnGround = TRUE;
		bRenderZHeightAboveLevel = 0;
	}

	// Check structure database
	if ( gpWorldLevelData[ *psGridNo ].pStructureHead && (pObject->usItem != OWNERSHIP) && (pObject->usItem != ACTION_ITEM) )
	{
		// Something is here, check obstruction in future
		sDesiredLevel = ubLevel ? STRUCTURE_ON_ROOF : STRUCTURE_ON_GROUND;
		pStructure = FindStructure( *psGridNo , STRUCTURE_BLOCKSMOVES );
		while( pStructure )
		{
			if( !(pStructure->fFlags &( STRUCTURE_PERSON | STRUCTURE_CORPSE ) ) && pStructure->sCubeOffset == sDesiredLevel )
			{
				// If we are going into a raised struct AND we have above level set to -1
				if ( StructureBottomLevel( pStructure ) != 1 && fForceOnGround )
				{
					break;
				}

				// Adjust the item's gridno to the base of struct.....
				pBase = FindBaseStructure( pStructure );

				// Get LEVELNODE for struct and remove!
				sNewGridNo = pBase->sGridNo;

				// Check for openable flag....
				if ( pStructure->fFlags & STRUCTURE_OPENABLE )
				{
					// ATE: Set a flag here - we need to know later that we're in an openable...
					fObjectInOpenable = TRUE;

					// Something of note is here....
					// SOME sort of structure is here.... set render flag to off
					usFlags |= WORLD_ITEM_DONTRENDER;

					// Openable.. check if it's closed, if so, set visiblity...
					if ( !( pStructure->fFlags & STRUCTURE_OPEN ) )
					{
						// -2 means - don't reveal!
						bVisible = -2;
					}

					bRenderZHeightAboveLevel = CONVERT_INDEX_TO_PIXELS( StructureHeight( pStructure ) );
					break;

				}
				// Else can we place an item on top?
				else if ( pStructure->fFlags & ( STRUCTURE_GENERIC ) )
				{
					UINT8 ubLevel0, ubLevel1, ubLevel2, ubLevel3;

					// If we are going into a raised struct AND we have above level set to -1
					if ( StructureBottomLevel( pStructure ) != 1 && fForceOnGround )
					{
						break;
					}

					// Find most dence area...
					if ( StructureDensity( pStructure, &ubLevel0, &ubLevel1, &ubLevel2, &ubLevel3 ) )
					{
						 if ( ubLevel3 == 0 && ubLevel2 == 0 && ubLevel1 == 0 && ubLevel0 == 0 )
						 {
								bRenderZHeightAboveLevel = 0;
						 }
						 else if ( ubLevel3 >= ubLevel0 && ubLevel3 >= ubLevel2 && ubLevel3 >= ubLevel1 )
						 {
								bRenderZHeightAboveLevel = CONVERT_INDEX_TO_PIXELS( 4 );
						 }
						 else if ( ubLevel2 >= ubLevel0 && ubLevel2 >= ubLevel1 && ubLevel2 >= ubLevel3 )
						 {
								bRenderZHeightAboveLevel = CONVERT_INDEX_TO_PIXELS( 3 );
						 }
						 else if ( ubLevel1 >= ubLevel0 && ubLevel1 >= ubLevel2 && ubLevel1 >= ubLevel3 )
						 {
								bRenderZHeightAboveLevel = CONVERT_INDEX_TO_PIXELS( 2 );
						 }
						 else if ( ubLevel0 >= ubLevel1 && ubLevel0 >= ubLevel2 && ubLevel0 >= ubLevel3 )
						 {
								bRenderZHeightAboveLevel = CONVERT_INDEX_TO_PIXELS( 1 );
						 }
					}

					// Set flag indicating it has an item on top!
					pStructure->fFlags |= STRUCTURE_HASITEMONTOP;
					break;
				}

			}

			pStructure = FindNextStructure( pStructure, STRUCTURE_BLOCKSMOVES );
		}
	}

	if (pObject->usItem == SWITCH && !fObjectInOpenable )
	{
		if (bVisible != -2)
		{
			// switch items which are not hidden inside objects should be considered buried
			bVisible = BURIED;
			// and they are pressure-triggered unless there is a switch structure there
			if (FindStructure( *psGridNo, STRUCTURE_SWITCH ) != NULL)
			{
				pObject->bDetonatorType = BOMB_SWITCH;
			}
			else
			{
				pObject->bDetonatorType = BOMB_PRESSURE;
			}
		}
		else
		{
			// else they are manually controlled
			pObject->bDetonatorType = BOMB_SWITCH;
		}
	}
	else if ( pObject->usItem == ACTION_ITEM )
	{
		switch( pObject->bActionValue )
		{
			case ACTION_ITEM_SMALL_PIT:
			case ACTION_ITEM_LARGE_PIT:
				// mark as known about by civs and creatures
				gpWorldLevelData[ sNewGridNo ].uiFlags |= MAPELEMENT_ENEMY_MINE_PRESENT;
				break;
			default:
				break;
		}
	}

	if ( *psGridNo != sNewGridNo )
	{
		*psGridNo = sNewGridNo;
	}


	//First add the item to the global list.  This is so the game can keep track
	//of where the items are, for file i/o, etc.
	iWorldItem = AddItemToWorld( *psGridNo,  pObject, ubLevel, usFlags, bRenderZHeightAboveLevel, bVisible );

	// Check for and existing pool on the object layer
	if ( GetItemPool( *psGridNo, &pItemPool, ubLevel ) )
	{

		// Add to exitsing pool
		// Add graphic
		pNode = AddItemGraphicToWorld( &(Item[ pObject->usItem ] ), *psGridNo, ubLevel );

		// Set pool head value in levelnode
		pNode->pItemPool = pItemPool;

		// Add New Node
		pItemPoolTemp = pItemPool;
		// Create new pool
		pItemPool = MemAlloc( sizeof( ITEM_POOL ) );

		// Set Next to NULL
		pItemPool->pNext = NULL;
		// Set Item index
		pItemPool->iItemIndex = iWorldItem;
		// Get a link back!
		pItemPool->pLevelNode = pNode;

		if( pItemPoolTemp )
		{
			// Get last item in list
			while( pItemPoolTemp->pNext != NULL )
				pItemPoolTemp = pItemPoolTemp->pNext;

			// Set Next of previous
			pItemPoolTemp->pNext = pItemPool;
		}
		// Set Previous of new one
		pItemPool->pPrev = pItemPoolTemp;

	}
	else
	{
		pNode = AddItemGraphicToWorld( &(Item[ pObject->usItem ] ), *psGridNo, ubLevel );

		// Create new pool
		pItemPool = MemAlloc( sizeof( ITEM_POOL ) );

		pNode->pItemPool = pItemPool;

		// Set prev to NULL
		pItemPool->pPrev = NULL;
		// Set next to NULL
		pItemPool->pNext = NULL;
		// Set Item index
		pItemPool->iItemIndex = iWorldItem;
		// Get a link back!
		pItemPool->pLevelNode = pNode;

		// Set flag to indicate item pool presence
		gpWorldLevelData[*psGridNo].uiFlags |= MAPELEMENT_ITEMPOOL_PRESENT;
	}

	// Set visible!
	pItemPool->bVisible = bVisible;

	// If bbisible is true, render makered world
	if ( bVisible == 1 && GridNoOnScreen( (*psGridNo ) ) )
	{
		//gpWorldLevelData[*psGridNo].uiFlags|=MAPELEMENT_REDRAW;
		//SetRenderFlags(RENDER_FLAG_MARKED);
		SetRenderFlags(RENDER_FLAG_FULL);
	}

	// Set flahs timer
	pItemPool->bFlashColor							 = FALSE;
	pItemPool->sGridNo									 = *psGridNo;
	pItemPool->ubLevel									 = ubLevel;
	pItemPool->usFlags									 = usFlags;
	pItemPool->bVisible									 = bVisible;
	pItemPool->bRenderZHeightAboveLevel	 = bRenderZHeightAboveLevel;

	// ATE: Get head of pool again....
	if ( GetItemPool( *psGridNo, &pItemPool, ubLevel ) )
	{
		AdjustItemPoolVisibility( pItemPool );
	}

	if (piItemIndex)
	{
		*piItemIndex = iWorldItem;
	}

	return( &(gWorldItems[ iWorldItem ].o ) );
}


static BOOLEAN ItemExistsAtLocation(INT16 sGridNo, INT32 iItemIndex, UINT8 ubLevel)
{
	ITEM_POOL		*pItemPool;
	ITEM_POOL		*pItemPoolTemp;

	// Check for an existing pool on the object layer
	if ( GetItemPool( sGridNo, &pItemPool, ubLevel ) )
	{
	// LOOP THROUGH LIST TO FIND NODE WE WANT
		pItemPoolTemp = pItemPool;
		while( pItemPoolTemp != NULL )
		{
			if ( pItemPoolTemp->iItemIndex == iItemIndex )
			{
				return( TRUE );
			}
			pItemPoolTemp = pItemPoolTemp->pNext;
		}

	}

	return( FALSE );
}

BOOLEAN ItemTypeExistsAtLocation( INT16 sGridNo, UINT16 usItem, UINT8 ubLevel, INT32 * piItemIndex )
{
	ITEM_POOL		*pItemPool;
	ITEM_POOL		*pItemPoolTemp;

	// Check for an existing pool on the object layer
	if ( GetItemPool( sGridNo, &pItemPool, ubLevel ) )
	{
	// LOOP THROUGH LIST TO FIND ITEM WE WANT
		pItemPoolTemp = pItemPool;
		while( pItemPoolTemp != NULL )
		{
			if ( gWorldItems[ pItemPoolTemp->iItemIndex ].o.usItem == usItem )
			{
				if ( piItemIndex )
				{
					*piItemIndex = pItemPoolTemp->iItemIndex;
				}
				return( TRUE );
			}
			pItemPoolTemp = pItemPoolTemp->pNext;
		}

	}

	return( FALSE );
}


static INT32 GetItemOfClassTypeInPool(INT16 sGridNo, UINT32 uiItemClass, UINT8 ubLevel)
{
	ITEM_POOL		*pItemPool;
	ITEM_POOL		*pItemPoolTemp;

	// Check for an existing pool on the object layer
	if ( GetItemPool( sGridNo, &pItemPool, ubLevel ) )
	{
	// LOOP THROUGH LIST TO FIND NODE WE WANT
		pItemPoolTemp = pItemPool;
		while( pItemPoolTemp != NULL )
		{
			if ( Item[ gWorldItems[ pItemPoolTemp->iItemIndex ].o.usItem ].usItemClass & uiItemClass )
			{
				return( pItemPoolTemp->iItemIndex );
			}
			pItemPoolTemp = pItemPoolTemp->pNext;
		}

	}

	return( -1 );
}


static ITEM_POOL* GetItemPoolForIndex(INT16 sGridNo, INT32 iItemIndex, UINT8 ubLevel)
{
	ITEM_POOL		*pItemPool;
	ITEM_POOL		*pItemPoolTemp;

	// Check for an existing pool on the object layer
	if ( GetItemPool( sGridNo, &pItemPool, ubLevel ) )
	{
	// LOOP THROUGH LIST TO FIND NODE WE WANT
		pItemPoolTemp = pItemPool;
		while( pItemPoolTemp != NULL )
		{
			if (pItemPoolTemp->iItemIndex == iItemIndex )
			{
				return( pItemPoolTemp );
			}
			pItemPoolTemp = pItemPoolTemp->pNext;
		}

	}

	return( NULL );
}



BOOLEAN DoesItemPoolContainAnyHiddenItems( ITEM_POOL *pItemPool )
{
	// LOOP THROUGH LIST TO FIND NODE WE WANT
	while( pItemPool != NULL )
	{
		if ( gWorldItems[ pItemPool->iItemIndex ].bVisible == HIDDEN_ITEM )
		{
			return( TRUE );
		}

		pItemPool = pItemPool->pNext;
	}

	return( FALSE );
}


static BOOLEAN DoesItemPoolContainAllHiddenItems(ITEM_POOL* pItemPool)
{
	// LOOP THROUGH LIST TO FIND NODE WE WANT
	while( pItemPool != NULL )
	{
		if ( gWorldItems[ pItemPool->iItemIndex ].bVisible != HIDDEN_ITEM )
		{
			return( FALSE );
		}

		pItemPool = pItemPool->pNext;
	}

	return( TRUE );
}


static BOOLEAN LookForHiddenItems(INT16 sGridNo, INT8 ubLevel, BOOLEAN fSetLocator, INT8 bZLevel)
{
	ITEM_POOL *pItemPool = NULL;
	ITEM_POOL *pHeadItemPool = NULL;
	BOOLEAN		fFound = FALSE;

	if ( GetItemPool( sGridNo, &pItemPool, ubLevel ) )
	{
		pHeadItemPool = pItemPool;

		// LOOP THROUGH LIST TO FIND NODE WE WANT
		while( pItemPool != NULL )
		{
			if ( gWorldItems[ pItemPool->iItemIndex ].bVisible == HIDDEN_ITEM && gWorldItems[ pItemPool->iItemIndex ].o.usItem != OWNERSHIP )
			{
				fFound = TRUE;

				gWorldItems[ pItemPool->iItemIndex ].bVisible = INVISIBLE;
			}

			pItemPool = pItemPool->pNext;
		}
	}

	// If found, set item pool visibility...
	if ( fFound )
	{
		SetItemPoolVisibilityOn( pHeadItemPool, INVISIBLE, fSetLocator );
	}

	return( fFound );
}


INT8 GetZLevelOfItemPoolGivenStructure( INT16 sGridNo, UINT8 ubLevel, STRUCTURE *pStructure )
{
	ITEM_POOL *pItemPool;

	if ( pStructure == NULL )
	{
		return( 0 );
	}

	// OK, check if this struct contains items....
	if ( GetItemPool( sGridNo, &pItemPool, ubLevel ) == TRUE )
	{
		return( GetLargestZLevelOfItemPool( pItemPool ) );
	}
	return( 0 );
}

INT8 GetLargestZLevelOfItemPool( ITEM_POOL *pItemPool )
{
	// OK, loop through pools and get any height != 0........
	while( pItemPool != NULL )
	{
		if ( pItemPool->bRenderZHeightAboveLevel > 0 )
		{
			return( pItemPool->bRenderZHeightAboveLevel );
		}

		pItemPool = pItemPool->pNext;
	}

	return( 0 );
}


static BOOLEAN DoesItemPoolContainAllItemsOfHigherZLevel(ITEM_POOL* pItemPool)
{
	// LOOP THROUGH LIST TO FIND NODE WE WANT
	while( pItemPool != NULL )
	{
		if ( pItemPool->bRenderZHeightAboveLevel == 0 )
		{
			return( FALSE );
		}

		pItemPool = pItemPool->pNext;
	}

	return( TRUE );
}


static BOOLEAN DoesItemPoolContainAllItemsOfZeroZLevel(ITEM_POOL* pItemPool)
{
	// LOOP THROUGH LIST TO FIND NODE WE WANT
	while( pItemPool != NULL )
	{
		if ( pItemPool->bRenderZHeightAboveLevel != 0 )
		{
			return( FALSE );
		}

		pItemPool = pItemPool->pNext;
	}

	return( TRUE );
}


static void RemoveItemPool(INT16 sGridNo, UINT8 ubLevel)
{
	ITEM_POOL		*pItemPool;

	// Check for and existing pool on the object layer
	while ( GetItemPool( sGridNo, &pItemPool, ubLevel ) == TRUE )
	{
		RemoveItemFromPool( sGridNo, pItemPool->iItemIndex, ubLevel );
	}
}

void RemoveAllUnburiedItems( INT16 sGridNo, UINT8 ubLevel )
{
	ITEM_POOL		*pItemPool;

	// Check for and existing pool on the object layer
	GetItemPool( sGridNo, &pItemPool, ubLevel );

	while( pItemPool )
	{
		if ( gWorldItems[ pItemPool->iItemIndex ].bVisible == BURIED)
		{
			pItemPool = pItemPool->pNext;
		}
		else
		{
			RemoveItemFromPool( sGridNo, pItemPool->iItemIndex, ubLevel );
			// get new start pointer
			GetItemPool( sGridNo, &pItemPool, ubLevel );
		}
	}
}


static void LoopLevelNodeForShowThroughFlag(LEVELNODE* pNode, INT16 sGridNo, UINT8 ubLevel)
{
	while ( pNode != NULL )
	{
		if ( pNode->uiFlags & LEVELNODE_ITEM )
		{
			if ( ubLevel == 0 )
			{
				// If we are in a room....
				// if ( IsRoofPresentAtGridno( sGridNo ) || gfCaves || gfBasement )
				{
					pNode->uiFlags |= LEVELNODE_SHOW_THROUGH;
				}
			}
			else
			{
				pNode->uiFlags |= LEVELNODE_SHOW_THROUGH;
			}

			if( gGameSettings.fOptions[ TOPTION_GLOW_ITEMS ] )
			{
				pNode->uiFlags |= LEVELNODE_DYNAMIC;
			}

		}
		pNode = pNode->pNext;
	}
}


static void HandleItemObscuredFlag(INT16 sGridNo, UINT8 ubLevel)
{
	LEVELNODE *pNode;

	if ( ubLevel == 0 )
	{
		pNode = gpWorldLevelData[ sGridNo ].pStructHead;
		LoopLevelNodeForShowThroughFlag( pNode, sGridNo,  ubLevel );
	}
	else
	{
		pNode = gpWorldLevelData[ sGridNo ].pOnRoofHead;
		LoopLevelNodeForShowThroughFlag( pNode, sGridNo, ubLevel );
	}
}


static void SetItemPoolLocator(ITEM_POOL* pItemPool);


BOOLEAN SetItemPoolVisibilityOn( ITEM_POOL *pItemPool, INT8 bAllGreaterThan, BOOLEAN fSetLocator )
{
	ITEM_POOL		*pItemPoolTemp;
	BOOLEAN			fAtLeastModified = FALSE, fDeleted = FALSE;
	INT8				bVisibleValue;
	//OBJECTTYPE *pObj;

	pItemPoolTemp = pItemPool;
	while( pItemPoolTemp != NULL )
	{
		bVisibleValue = gWorldItems[ pItemPoolTemp->iItemIndex ].bVisible;

		// Update each item...
		if ( bVisibleValue != VISIBLE )
		{
			if ( gWorldItems[ pItemPoolTemp->iItemIndex ].o.usItem == ACTION_ITEM )
			{
				// NEVER MAKE VISIBLE!
				pItemPoolTemp = pItemPoolTemp->pNext;
				continue;
			}

			// If we have reached a visible value we should not modify, ignore...
			if ( bVisibleValue >= bAllGreaterThan && gWorldItems[ pItemPoolTemp->iItemIndex ].o.usItem != OWNERSHIP )
			{
				// Update the world value
				gWorldItems[ pItemPoolTemp->iItemIndex ].bVisible = VISIBLE;

				fAtLeastModified = TRUE;
			}

			/*
			if ( gWorldItems[ pItemPoolTemp->iItemIndex ].o.usItem == ACTION_ITEM )
			{
				pObj = &(gWorldItems[ pItemPoolTemp->iItemIndex ].o);
				switch( pObj->bActionValue )
				{
					case ACTION_ITEM_SMALL_PIT:
					case ACTION_ITEM_LARGE_PIT:
						if (pObj->bDetonatorType == 0)
						{
							// randomly set to active or destroy the item!
							if (Random( 100 ) < 65)
							{
								ArmBomb( pObj, 0 ); // will be set to pressure type so freq is irrelevant
								gWorldItems[ pItemPoolTemp->iItemIndex ].usFlags |= WORLD_ITEM_ARMED_BOMB;
								AddBombToWorld( pItemPoolTemp->iItemIndex );
							}
							else
							{
								// get pointer to the next element NOW
								pItemPoolTemp	= pItemPoolTemp->pNext;
								// set flag so we don't traverse an additional time
								fDeleted = TRUE;
								// remove item from pool
								RemoveItemFromPool( pItemPool->sGridNo, pItemPool->iItemIndex, pItemPool->ubLevel );
							}
						}
						break;
					default:
						break;
				}
			}
			*/

			if (fDeleted)
			{
				// don't get the 'next' pointer because we did so above

				// reset fDeleted to false so we don't skip moving through the list more than once
				fDeleted = FALSE;
			}
			else
			{
				pItemPoolTemp						= pItemPoolTemp->pNext;
			}

		}
		else
		{
			pItemPoolTemp						= pItemPoolTemp->pNext;
		}
	}

	// If we didn;t find any that should be modified..
	if ( !fAtLeastModified )
	{
		return( FALSE );
	}


	// Update global pool bVisible to true ( if at least one is visible... )
	pItemPoolTemp = pItemPool;
	while( pItemPoolTemp != NULL )
	{
		pItemPoolTemp->bVisible = VISIBLE;

		pItemPoolTemp						= pItemPoolTemp->pNext;
	}

	// Handle obscured flag...
	HandleItemObscuredFlag( pItemPool->sGridNo, pItemPool->ubLevel );

	if ( fSetLocator )
	{
		SetItemPoolLocator( pItemPool );
	}

	return( TRUE );
}


void SetItemPoolVisibilityHidden( ITEM_POOL *pItemPool )
{
	ITEM_POOL		*pItemPoolTemp;

	pItemPoolTemp = pItemPool;
	while( pItemPoolTemp != NULL )
	{
		// Update the world value
		gWorldItems[ pItemPoolTemp->iItemIndex ].bVisible = HIDDEN_IN_OBJECT;
		pItemPoolTemp->bVisible = HIDDEN_IN_OBJECT;

		pItemPoolTemp						= pItemPoolTemp->pNext;
	}
}


// This determines the overall initial visibility of the pool...
// IF ANY are set to VISIBLE, MODIFY
static void AdjustItemPoolVisibility(ITEM_POOL* pItemPool)
{
	ITEM_POOL		*pItemPoolTemp;
	BOOLEAN			fAtLeastModified = FALSE;

	pItemPoolTemp = pItemPool;
	while( pItemPoolTemp != NULL )
	{
		// DEFAULT ITEM POOL TO INVISIBLE....
		pItemPoolTemp->bVisible = INVISIBLE;

		// Update each item...
		// If we have reached a visible value we should not modify, ignore...
		if ( gWorldItems[ pItemPoolTemp->iItemIndex ].bVisible == VISIBLE )
		{
			fAtLeastModified = TRUE;
		}

		pItemPoolTemp						= pItemPoolTemp->pNext;
	}

	// Handle obscured flag...
	HandleItemObscuredFlag( pItemPool->sGridNo, pItemPool->ubLevel );

	// If we didn;t find any that should be modified..
	if ( !fAtLeastModified )
	{
		return;
	}

	// Update global pool bVisible to true ( if at least one is visible... )
	pItemPoolTemp = pItemPool;
	while( pItemPoolTemp != NULL )
	{
		pItemPoolTemp->bVisible = VISIBLE;

		pItemPoolTemp						= pItemPoolTemp->pNext;
	}

	// Handle obscured flag...
	HandleItemObscuredFlag( pItemPool->sGridNo, pItemPool->ubLevel );
}


BOOLEAN RemoveItemFromPool( INT16 sGridNo, INT32 iItemIndex, UINT8 ubLevel )
{
	ITEM_POOL		*pItemPool;
	ITEM_POOL		*pItemPoolTemp;
	BOOLEAN			fItemFound = FALSE;
	LEVELNODE		*pObject;


	// Check for and existing pool on the object layer
	if ( GetItemPool( sGridNo, &pItemPool, ubLevel ) )
	{

		// REMOVE FROM LIST

		// LOOP THROUGH LIST TO FIND NODE WE WANT
		pItemPoolTemp = pItemPool;
		while( pItemPoolTemp != NULL )
		{
			if ( pItemPoolTemp->iItemIndex == iItemIndex )
			{
				fItemFound = TRUE;
				break;
			}
			pItemPoolTemp = pItemPoolTemp->pNext;
		}

		if ( !fItemFound )
		{
			// COULDNOT FIND ITEM? MAYBE SOMEBODY GOT IT BEFORE WE GOT THERE!
			return( FALSE );
		}

		// REMOVE GRAPHIC
		RemoveItemGraphicFromWorld( &(Item[ gWorldItems[ iItemIndex ].o.usItem ] ), sGridNo, ubLevel, pItemPoolTemp->pLevelNode );

		// IF WE ARE LOCATIONG STILL, KILL LOCATOR!
		if ( pItemPoolTemp->bFlashColor != 0 )
		{
			// REMOVE TIMER!
			RemoveFlashItemSlot( pItemPoolTemp );
		}

		// REMOVE PREV
		if ( pItemPoolTemp->pPrev != NULL )
		{
			pItemPoolTemp->pPrev->pNext = pItemPoolTemp->pNext;
		}

		// REMOVE NEXT
		if ( pItemPoolTemp->pNext != NULL )
		{
			pItemPoolTemp->pNext->pPrev = pItemPoolTemp->pPrev;
		}


		// IF THIS NODE WAS THE HEAD, SET ANOTHER AS HEAD AT THIS GRIDNO
		if ( pItemPoolTemp->pPrev == NULL )
		{
			// WE'RE HEAD
			if ( ubLevel == 0 )
			{
				pObject = gpWorldLevelData[ sGridNo ].pStructHead;
			}
			else
			{
				pObject = gpWorldLevelData[ sGridNo ].pOnRoofHead;
			}

			fItemFound = FALSE;
			// LOOP THORUGH OBJECT LAYER
			while( pObject != NULL )
			{
				if ( pObject->uiFlags & LEVELNODE_ITEM )
				{
					// ADJUST TO NEXT GUY FOR HEAD
					pObject->pItemPool = pItemPoolTemp->pNext;
					fItemFound = TRUE;
				}
				pObject = pObject->pNext;
			}

			if (!fItemFound)
			{
				// THIS WAS THE LAST ITEM IN THE POOL!
				gpWorldLevelData[sGridNo].uiFlags &= ~(MAPELEMENT_ITEMPOOL_PRESENT);
			}
		}

		// Find any structure with flag set as having items on top.. if this one did...
		if ( pItemPoolTemp->bRenderZHeightAboveLevel > 0 )
		{
			STRUCTURE		*pStructure;
			ITEM_POOL		*pTempPool;

			// Check if an item pool exists here....
			if ( !GetItemPool( pItemPoolTemp->sGridNo, &pTempPool, pItemPoolTemp->ubLevel ) )
			{
				pStructure = FindStructure( pItemPoolTemp->sGridNo , STRUCTURE_HASITEMONTOP );

				if ( pStructure != NULL )
				{
					// Remove...
					pStructure->fFlags &= (~STRUCTURE_HASITEMONTOP );

					// Re-adjust interactive tile...
					BeginCurInteractiveTileCheck( INTILE_CHECK_SELECTIVE );
				}
			}
		}

		AdjustItemPoolVisibility( pItemPoolTemp );

		// DELETE
		MemFree( pItemPoolTemp );

		RemoveItemFromWorld( iItemIndex );

		return( TRUE );
	}

	return( FALSE );
}

BOOLEAN MoveItemPools( INT16 sStartPos, INT16 sEndPos )
{
	// note, only works between locations on the ground
	ITEM_POOL		*pItemPool;
	WORLDITEM		TempWorldItem;

	// While there is an existing pool
	while( GetItemPool( sStartPos, &pItemPool, 0 ) )
	{
		TempWorldItem = gWorldItems[pItemPool->iItemIndex];
		RemoveItemFromPool( sStartPos, pItemPool->iItemIndex, 0 );
		AddItemToPool( sEndPos, &(TempWorldItem.o), -1, TempWorldItem.ubLevel, TempWorldItem.usFlags, TempWorldItem.bRenderZHeightAboveLevel );
	}
	return( TRUE );
}

BOOLEAN	GetItemPool( UINT16 usMapPos, ITEM_POOL **ppItemPool, UINT8 ubLevel )
{
	LEVELNODE *pObject;

	if ( ubLevel == 0 )
	{
		pObject = gpWorldLevelData[ usMapPos ].pStructHead;
	}
	else
	{
		pObject = gpWorldLevelData[ usMapPos ].pOnRoofHead;
	}

	(*ppItemPool) = NULL;

	// LOOP THORUGH OBJECT LAYER
	while( pObject != NULL )
	{
		if ( pObject->uiFlags & LEVELNODE_ITEM )
		{
			(*ppItemPool) = pObject->pItemPool;

			//DEF added the check because pObject->pItemPool was NULL which was causing problems
			if( *ppItemPool )
				return( TRUE );
			else
				return( FALSE );
		}

		pObject = pObject->pNext;
	}

	return( FALSE );
}



void NotifySoldiersToLookforItems( )
{
	UINT32 cnt;
	SOLDIERTYPE *pSoldier;

	for ( cnt = 0; cnt < guiNumMercSlots; cnt++ )
	{
		pSoldier = MercSlots[ cnt ];

		if ( pSoldier != NULL )
		{
			pSoldier->uiStatusFlags |= SOLDIER_LOOKFOR_ITEMS;
		}
	}

}


void AllSoldiersLookforItems( BOOLEAN fShowLocators )
{
	UINT32 cnt;
	SOLDIERTYPE *pSoldier;

	for ( cnt = 0; cnt < guiNumMercSlots; cnt++ )
	{
		pSoldier = MercSlots[ cnt ];

		if ( pSoldier != NULL )
		{
			RevealRoofsAndItems(pSoldier, TRUE, fShowLocators, pSoldier->bLevel, FALSE );
		}
	}

}


static INT16 GetNumOkForDisplayItemsInPool(ITEM_POOL* pItemPool, INT8 bZLevel)
{
	INT32						cnt;

	//Determine total #
	cnt = 0;
	while( pItemPool != NULL )
	{
		if ( ItemPoolOKForDisplay( pItemPool, bZLevel ) )
		{
			cnt++;
		}

		pItemPool = pItemPool->pNext;
	}

	return( (UINT16) cnt );
}


BOOLEAN AnyItemsVisibleOnLevel(const ITEM_POOL* pItemPool, INT8 bZLevel)
{
	if ( ( gTacticalStatus.uiFlags & SHOW_ALL_ITEMS ) )
	{
		return( TRUE );
	}

	//Determine total #
	while( pItemPool != NULL )
	{
		if ( pItemPool->bRenderZHeightAboveLevel == bZLevel )
		{
			if ( gWorldItems[ pItemPool->iItemIndex ].bVisible == VISIBLE )
			{
				return( TRUE );
			}
		}

		pItemPool = pItemPool->pNext;

	}

	return( FALSE );
}


BOOLEAN ItemPoolOKForDisplay( ITEM_POOL *pItemPool, INT8 bZLevel )
{
	if (gTacticalStatus.uiFlags&SHOW_ALL_ITEMS)
	{
		return( TRUE );
	}

	// Setup some conditions!
	if ( gWorldItems[ pItemPool->iItemIndex ].bVisible != VISIBLE  )
	{
		return( FALSE );
	}

	// If -1, it means find all
	if ( pItemPool->bRenderZHeightAboveLevel != bZLevel && bZLevel != -1 )
		{
		return( FALSE );
	}

	return( TRUE );
}


static BOOLEAN ItemPoolOKForPickup(SOLDIERTYPE* pSoldier, ITEM_POOL* pItemPool, INT8 bZLevel)
{
	if (gTacticalStatus.uiFlags&SHOW_ALL_ITEMS)
	{
		return( TRUE );
	}

	if ( pSoldier->bTeam == gbPlayerNum )
	{
		// Setup some conditions!
		if ( gWorldItems[ pItemPool->iItemIndex ].bVisible != VISIBLE  )
		{
			return( FALSE );
		}
	}

	// If -1, it means find all
	if ( pItemPool->bRenderZHeightAboveLevel != bZLevel && bZLevel != -1 )
		{
		return( FALSE );
	}

	return( TRUE );
}


extern void HandleAnyMercInSquadHasCompatibleStuff( UINT8 ubSquad, OBJECTTYPE *pObject, BOOLEAN fReset );


BOOLEAN DrawItemPoolList( ITEM_POOL *pItemPool, INT16 sGridNo, UINT8 bCommand, INT8 bZLevel, INT16 sXPos, INT16 sYPos )
{
	INT16 sY;
	INVTYPE			*pItem;
	ITEM_POOL		*pTempItemPool;
	wchar_t pStr[ 100 ];
	INT16		cnt = 0, sHeight = 0;
	INT16	 sLargeLineWidth = 0, sLineWidth;
	BOOLEAN			fSelectionDone = FALSE;

	INT8				gbCurrentItemSel = 0;
	INT8				bNumItemsListed = 0;
	INT16				sFontX, sFontY;
	INT16				sLargestLineWidth = 30;
	INT8				bCurStart = 0;
	BOOLEAN			fDoBack;


	// Take a look at each guy in current sqaud and check for compatible ammo...

	// Determine how many there are
	// MOVE HEAD TO CURRENT START
	cnt = 0;
	pTempItemPool = pItemPool;
	while( pTempItemPool != NULL )
	{
		if ( cnt == bCurStart )
		{
			break;
		}

		// ATE: Put some conditions on this....
		if ( ItemPoolOKForDisplay( pTempItemPool, bZLevel ) )
		{
			cnt++;
		}

		pTempItemPool = pTempItemPool->pNext;
	}

	cnt = bCurStart;
	fDoBack = FALSE;
	while( pTempItemPool != NULL )
	{
		// IF WE HAVE MORE THAN THE SET AMOUNT, QUIT NOW!
		if ( cnt == ( bCurStart + NUM_ITEMS_LISTED ) )
		{
			cnt++;
			fDoBack = TRUE;
			break;
		}

		// ATE: Put some conditions on this....
		if ( ItemPoolOKForDisplay( pTempItemPool, bZLevel ) )
		{
			cnt++;
		}

		sHeight += GetFontHeight( SMALLFONT1 ) - 2;

		pTempItemPool = pTempItemPool->pNext;
	}


	pTempItemPool = pItemPool;
	while( pTempItemPool != NULL )
	{
		// ATE: Put some conditions on this....
		if ( ItemPoolOKForDisplay( pTempItemPool, bZLevel ) )
		{
			HandleAnyMercInSquadHasCompatibleStuff( (INT8)CurrentSquad( ), &(gWorldItems[ pTempItemPool->iItemIndex ].o ), FALSE );
		}

		pTempItemPool = pTempItemPool->pNext;
	}

	// IF COUNT IS ALREADY > MAX, ADD A PREV...
	if ( bCurStart >= NUM_ITEMS_LISTED )
	{
		cnt++;
	}

	bNumItemsListed = (INT8)cnt;


	//RENDER LIST!
	// Determine max length
	pTempItemPool = pItemPool;
	while( pTempItemPool != NULL )
	{
		if ( ItemPoolOKForDisplay( pTempItemPool, bZLevel ) )
		{
			// GET ITEM
			pItem = &Item[ gWorldItems[ pTempItemPool->iItemIndex ].o.usItem ];
			// Set string
			if ( gWorldItems[ pTempItemPool->iItemIndex ].o.ubNumberOfObjects > 1 )
			{
				swprintf( pStr, lengthof(pStr), L"%ls (%d)", ShortItemNames[ gWorldItems[ pTempItemPool->iItemIndex ].o.usItem ], gWorldItems[ pTempItemPool->iItemIndex ].o.ubNumberOfObjects );
			}
			else
			{
				wcslcpy(pStr, ShortItemNames[gWorldItems[pTempItemPool->iItemIndex].o.usItem], lengthof(pStr));
			}

			// Get Width
			sLineWidth = StringPixLength( pStr,SMALLFONT1 );

			if ( sLineWidth > sLargeLineWidth )
			{
				sLargeLineWidth = sLineWidth;
			}
			sLargestLineWidth = sLargeLineWidth;
		}
		pTempItemPool = pTempItemPool->pNext;
	}

	// Determine where our mouse is!
	if ( sXPos > ( 640 - sLargestLineWidth ) )
	{
		sFontX = sXPos - sLargestLineWidth;
	}
	else
	{
		sFontX = sXPos + 15;
	}
	sFontY = sYPos;

	// Move up if over interface....
	if ( ( sFontY + sHeight ) > 340 )
	{
		sFontY = 340 - sHeight;
	}

	// Detertime vertiacal center
	sFontY -= ( sHeight / 2 );


	SetFont( SMALLFONT1 );
	SetFontBackground( FONT_MCOLOR_BLACK );
	SetFontForeground( FONT_MCOLOR_DKGRAY );

	// MOVE HEAD TO CURRENT START
	cnt = 0;
	while( pItemPool != NULL )
	{
		if ( cnt == bCurStart )
		{
			break;
		}

		if ( ItemPoolOKForDisplay( pItemPool, bZLevel ) )
		{
			cnt++;
		}

		pItemPool = pItemPool->pNext;
	}

	// START DISPLAY LOOP
	cnt = bCurStart;
	sY	= sFontY;

	// ADD PREV TO THIS LIST!
	if ( bCurStart >= NUM_ITEMS_LISTED )
	{
		// Set string
		if ( cnt == gbCurrentItemSel )
		{
			SetFontForeground( FONT_MCOLOR_LTGRAY );
		}
		else
		{
			SetFontForeground( FONT_MCOLOR_DKGRAY );
		}
		swprintf( pStr, lengthof(pStr), TacticalStr[ ITEMPOOL_POPUP_PREV_STR ] );
		gprintfdirty( sFontX, sY, pStr );
		mprintf( sFontX, sY, pStr );
		sY += GetFontHeight( SMALLFONT1 ) - 2;
		cnt++;
	}

	while( pItemPool != NULL )
	{
		if ( bCommand == ITEMLIST_HANDLE )
		{
			if ( cnt == gbCurrentItemSel )
			{
				SetFontForeground( FONT_MCOLOR_LTGRAY );
			}
			else
			{
				SetFontForeground( FONT_MCOLOR_DKGRAY );
			}
		}


		if ( ItemPoolOKForDisplay( pItemPool, bZLevel ) )
		{
			// GET ITEM
			pItem = &Item[ gWorldItems[ pItemPool->iItemIndex ].o.usItem ];
			// Set string

			if ( gWorldItems[ pItemPool->iItemIndex ].o.ubNumberOfObjects > 1 )
			{
				swprintf( pStr, lengthof(pStr), L"%ls (%d)", ShortItemNames[ gWorldItems[ pItemPool->iItemIndex ].o.usItem ], gWorldItems[ pItemPool->iItemIndex ].o.ubNumberOfObjects );
			}
			else
			{
				wcslcpy(pStr, ShortItemNames[gWorldItems[pItemPool->iItemIndex].o.usItem], lengthof(pStr));
			}

			gprintfdirty( sFontX, sY, pStr );
			mprintf( sFontX, sY, pStr );

			sY += GetFontHeight( SMALLFONT1 ) - 2;
			cnt++;
		}
		pItemPool = pItemPool->pNext;


		if ( fDoBack )
		{
			if ( cnt == ( bNumItemsListed - 1) )
			{
				break;
			}
		}

	}
	if ( fDoBack )
	{
			if ( cnt == ( bNumItemsListed - 1) )
			{
				// Set string
				if ( cnt == gbCurrentItemSel )
				{
					SetFontForeground( FONT_MCOLOR_LTGRAY );
				}
				else
				{
					SetFontForeground( FONT_MCOLOR_DKGRAY );
				}
				swprintf( pStr, lengthof(pStr), TacticalStr[ ITEMPOOL_POPUP_MORE_STR ] );
				gprintfdirty( sFontX, sY, pStr );
				mprintf( sFontX, sY, pStr );
			}
	}

	return( fSelectionDone );

}


static INT8 GetListMouseHotSpot(INT16 sLargestLineWidth, INT8 bNumItemsListed, INT16 sFontX, INT16 sFontY, INT8 bCurStart)
{
	INT16	 cnt = 0;
	INT16  sTestX1, sTestX2, sTestY1, sTestY2;
	INT16  sLineHeight;
	INT8	 gbCurrentItemSel = -1;
	INT8	 bListedItems;

	sLineHeight = GetFontHeight( SMALLFONT1 ) - 2;

	sTestX1 = sFontX;
	sTestX2 = sFontX + sLargestLineWidth;


	bListedItems = ( bNumItemsListed - bCurStart );


	if ( gusMouseXPos < sTestX1 || gusMouseXPos > sTestX2 )
	{
		gbCurrentItemSel = -1;
	}
	else
	{
		// Determine where mouse is!
		for ( cnt = 0; cnt < bListedItems; cnt++ )
		{
			sTestY1 = sFontY + ( sLineHeight * cnt );
			sTestY2 = sFontY + ( sLineHeight * ( cnt + 1 ) );

			if ( gusMouseYPos > sTestY1 && gusMouseYPos < sTestY2 )
			{
				gbCurrentItemSel = (INT8)cnt;
				break;
			}
		}
	}

	// OFFSET START
	gbCurrentItemSel += bCurStart;

	return( gbCurrentItemSel );
}


static INT32 AddFlashItemSlot(ITEM_POOL* pItemPool, ITEM_POOL_LOCATOR_HOOK Callback, UINT8 ubFlags);


static void SetItemPoolLocator(ITEM_POOL* pItemPool)
{
	pItemPool->bFlashColor = 59;

	pItemPool->uiTimerID		= AddFlashItemSlot( pItemPool, NULL, 0 );

}


static void SetItemPoolLocatorWithCallback(ITEM_POOL* pItemPool, ITEM_POOL_LOCATOR_HOOK Callback)
{
	pItemPool->bFlashColor = 59;

	pItemPool->uiTimerID		= AddFlashItemSlot( pItemPool, Callback, 0 );

}


/// ITEM POOL INDICATOR FUNCTIONS


static INT32 GetFreeFlashItemSlot(void)
{
	UINT32 uiCount;

	for(uiCount=0; uiCount < guiNumFlashItemSlots; uiCount++)
	{
		if(( FlashItemSlots[uiCount].fAllocated == FALSE ) )
			return((INT32)uiCount);
	}

	if(guiNumFlashItemSlots < NUM_ITEM_FLASH_SLOTS )
		return((INT32)guiNumFlashItemSlots++);

	return(-1);
}


static void RecountFlashItemSlots(void)
{
INT32 uiCount;

	for(uiCount=guiNumFlashItemSlots-1; (uiCount >=0) ; uiCount--)
	{
		if( ( FlashItemSlots[uiCount].fAllocated ) )
		{
			guiNumFlashItemSlots=(UINT32)(uiCount+1);
			break;
		}
	}
}


static INT32 AddFlashItemSlot(ITEM_POOL* pItemPool, ITEM_POOL_LOCATOR_HOOK Callback, UINT8 ubFlags)
{
	INT32			iFlashItemIndex;

	if( ( iFlashItemIndex = GetFreeFlashItemSlot() )==(-1) )
		return(-1);

	ubFlags |= ITEM_LOCATOR_LOCKED;


	FlashItemSlots[ iFlashItemIndex ].pItemPool	  = pItemPool;

	FlashItemSlots[ iFlashItemIndex ].bRadioFrame				= 0;
	FlashItemSlots[ iFlashItemIndex ].uiLastFrameUpdate = GetJA2Clock( );
	FlashItemSlots[ iFlashItemIndex ].Callback				  = Callback;
	FlashItemSlots[ iFlashItemIndex ].fAllocated				= TRUE;
	FlashItemSlots[ iFlashItemIndex ].ubFlags						= ubFlags;


	return( iFlashItemIndex );
}


BOOLEAN RemoveFlashItemSlot(const ITEM_POOL* pItemPool)
{
	UINT32 uiCount;

	CHECKF( pItemPool != NULL );

	for( uiCount=0; uiCount < guiNumFlashItemSlots; uiCount++)
	{
		if ( FlashItemSlots[ uiCount ].fAllocated )
		{
			if ( FlashItemSlots[ uiCount ].pItemPool == pItemPool )
			{
				FlashItemSlots[ uiCount ].fAllocated = FALSE;

				// Check if we have a callback and call it if so!
				if ( FlashItemSlots[ uiCount ].Callback != NULL )
				{
					FlashItemSlots[ uiCount ].Callback( );
				}

				return( TRUE );
			}
		}
	}

	return( TRUE );
}


void HandleFlashingItems( )
{
	UINT32 cnt;
	ITEM_POOL		*pItemPool;
	LEVELNODE		*pObject;
	ITEM_POOL_LOCATOR	*pLocator;
	BOOLEAN			fDoLocator = FALSE;

	if ( COUNTERDONE( CYCLERENDERITEMCOLOR ) )
	{
			RESETCOUNTER( CYCLERENDERITEMCOLOR );

			for ( cnt = 0; cnt < guiNumFlashItemSlots; cnt++ )
			{
					pLocator  = &( FlashItemSlots[ cnt ] );

					if ( pLocator->fAllocated )
					{
						fDoLocator = TRUE;

						if ( ( pLocator->ubFlags & ITEM_LOCATOR_LOCKED ) )
						{
							if ( gTacticalStatus.fLockItemLocators == FALSE )
							{
							  // Turn off!
								pLocator->ubFlags &= (~ITEM_LOCATOR_LOCKED);
							}
							else
							{
								fDoLocator = FALSE;
							}
						}

						if ( fDoLocator )
						{
							pItemPool = pLocator->pItemPool;

							// Update radio locator
							{
								UINT32			uiClock;

								uiClock = GetJA2Clock( );

								// Update frame values!
								if ( ( uiClock - pLocator->uiLastFrameUpdate ) > 80 )
								{
									pLocator->uiLastFrameUpdate = uiClock;

									// Update frame
									pLocator->bRadioFrame++;

									if ( pLocator->bRadioFrame == 5 )
									{
										pLocator->bRadioFrame = 0;
									}
								}
							}

							// UPDATE FLASH COLOR VALUE
							pItemPool->bFlashColor--;

							if ( pItemPool->ubLevel == 0 )
							{
								pObject = gpWorldLevelData[ pItemPool->sGridNo ].pStructHead;
							}
							else
							{
								pObject = gpWorldLevelData[ pItemPool->sGridNo ].pOnRoofHead;
							}

							// LOOP THORUGH OBJECT LAYER
							while( pObject != NULL )
							{
								if ( pObject->uiFlags & LEVELNODE_ITEM )
								{
									if ( pItemPool->bFlashColor == 1 )
									{
										//pObject->uiFlags &= (~LEVELNODE_DYNAMIC);
										//pObject->uiFlags |= ( LEVELNODE_LASTDYNAMIC  );
									}
									else
									{
										//pObject->uiFlags |= LEVELNODE_DYNAMIC;
									}

								}

								pObject = pObject->pNext;
							}

							if ( pItemPool->bFlashColor == 1 )
							{
								pItemPool->bFlashColor = 0;

								// REMOVE TIMER!
								RemoveFlashItemSlot( pItemPool );

								SetRenderFlags( RENDER_FLAG_FULL );
							}
						}
					}
			}

			RecountFlashItemSlots( );

	}

}


void RenderTopmostFlashingItems( )
{
	UINT32 cnt;
	ITEM_POOL		*pItemPool;
	ITEM_POOL_LOCATOR	*pLocator;

	for ( cnt = 0; cnt < guiNumFlashItemSlots; cnt++ )
	{
		pLocator  = &( FlashItemSlots[ cnt ] );

		if ( pLocator->fAllocated )
		{
			if ( !( pLocator->ubFlags & ( ITEM_LOCATOR_LOCKED ) ) )
			{
				pItemPool = pLocator->pItemPool;

				// Update radio locator
				{
					FLOAT				dOffsetX, dOffsetY;
					FLOAT				dTempX_S, dTempY_S;
					INT16				sX, sY, sXPos, sYPos;
					INT32				iBack;

					ConvertGridNoToCenterCellXY( pItemPool->sGridNo, &sX, &sY );

					dOffsetX = (FLOAT)( sX - gsRenderCenterX );
					dOffsetY = (FLOAT)( sY - gsRenderCenterY );

					// Calculate guy's position
					FloatFromCellToScreenCoordinates( dOffsetX, dOffsetY, &dTempX_S, &dTempY_S );

					sXPos = ( ( gsVIEWPORT_END_X - gsVIEWPORT_START_X ) /2 ) + (INT16)dTempX_S;
					sYPos = ( ( gsVIEWPORT_END_Y - gsVIEWPORT_START_Y ) /2 ) + (INT16)dTempY_S - gpWorldLevelData[ pItemPool->sGridNo ].sHeight;

					// Adjust for offset position on screen
					sXPos -= gsRenderWorldOffsetX;
					sYPos -= gsRenderWorldOffsetY;
					sYPos -= pItemPool->bRenderZHeightAboveLevel;

					// Adjust for render height
					sYPos += gsRenderHeight;

					// Adjust for level height
					if ( pItemPool->ubLevel )
					{
						sYPos -= ROOF_LEVEL_HEIGHT;
					}

					// Center circle!
					sXPos -= 20;
					sYPos -= 20;

					iBack = RegisterBackgroundRect( BGND_FLAG_SINGLE, NULL, sXPos, sYPos, (INT16)(sXPos +40 ), (INT16)(sYPos + 40 ) );
					if ( iBack != -1 )
					{
						SetBackgroundRectFilled( iBack );
					}

					BltVideoObjectFromIndex(  FRAME_BUFFER, guiRADIO, pLocator->bRadioFrame, sXPos, sYPos);

					DrawItemPoolList( pItemPool, pItemPool->sGridNo	, ITEMLIST_DISPLAY, pItemPool->bRenderZHeightAboveLevel, sXPos, sYPos );

				}
			}
		}
	}

}




BOOLEAN VerifyGiveItem( SOLDIERTYPE *pSoldier, SOLDIERTYPE **ppTargetSoldier )
{
	UINT16 usSoldierIndex;
	OBJECTTYPE	*pObject;

	INT16				sGridNo;
	UINT8				ubDirection;
	UINT8				ubTargetMercID;

	// DO SOME CHECKS IF WE CAN DO ANIMATION.....

	// Get items from pending data
	pObject = pSoldier->pTempObject;

	sGridNo		= pSoldier->sPendingActionData2;
	ubDirection = pSoldier->bPendingActionData3;
	ubTargetMercID = (UINT8)pSoldier->uiPendingActionData4;

	usSoldierIndex = WhoIsThere2( sGridNo, pSoldier->bLevel );

	// See if our target is still available
	if ( usSoldierIndex != NOBODY )
	{
		// Check if it's the same merc!
		if ( usSoldierIndex != ubTargetMercID )
		{
			return( FALSE );
		}

		// Look for item in hand....

		*ppTargetSoldier = GetSoldier(usSoldierIndex);

		return( TRUE );
	}
	else
	{
		if ( pSoldier->pTempObject != NULL )
		{
			AddItemToPool( pSoldier->sGridNo, pSoldier->pTempObject, 1, pSoldier->bLevel, 0 , -1 );

			// Place it on the ground!
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ ITEM_HAS_BEEN_PLACED_ON_GROUND_STR ], ShortItemNames[ pSoldier->pTempObject->usItem ] );

			// OK, disengage buddy
			pSoldier->uiStatusFlags &= (~SOLDIER_ENGAGEDINACTION );

			if ( ubTargetMercID != NOBODY )
			{
				MercPtrs[ ubTargetMercID ]->uiStatusFlags &= (~SOLDIER_ENGAGEDINACTION );
			}

			MemFree( pSoldier->pTempObject );
			pSoldier->pTempObject = NULL;

		}
	}

	return( FALSE );
}


void SoldierGiveItemFromAnimation( SOLDIERTYPE *pSoldier )
{
	SOLDIERTYPE *pTSoldier;
	INT8				bInvPos;
	OBJECTTYPE	TempObject;
	UINT8				ubProfile;

	INT16				sGridNo;
	UINT8				ubDirection;
	UINT8				ubTargetMercID;
	UINT16			usItemNum;
	BOOLEAN			fToTargetPlayer = FALSE;

	// Get items from pending data

	// Get objectype and delete
	TempObject = *pSoldier->pTempObject;
	MemFree( pSoldier->pTempObject );
	pSoldier->pTempObject = NULL;


	bInvPos = pSoldier->bPendingActionData5;
	usItemNum = TempObject.usItem;

  // ATE: OK, check if we have an item in the cursor from
  // this soldier and from this inv slot, if so, delete!!!!!!!
  if ( gpItemPointer != NULL )
  {
    if ( pSoldier->ubID == gpItemPointerSoldier->ubID )
    {
      if ( bInvPos == gbItemPointerSrcSlot && usItemNum == gpItemPointer->usItem  )
      {
        // Remove!!!
        EndItemPointer( );
      }
    }
  }

	sGridNo		= pSoldier->sPendingActionData2;
	ubDirection = pSoldier->bPendingActionData3;
	ubTargetMercID = (UINT8)pSoldier->uiPendingActionData4;

	// ATE: Deduct APs!
	DeductPoints( pSoldier, AP_PICKUP_ITEM, 0 );

	if ( VerifyGiveItem( pSoldier, &pTSoldier ) )
	{
		// DAVE! - put stuff here to bring up shopkeeper.......

		//if the user just clicked on an arms dealer
		if( IsMercADealer( pTSoldier->ubProfile ) )
		{
			UnSetEngagedInConvFromPCAction( pSoldier );

			//if the dealer is Micky,
			/*
			if( pTSoldier->ubProfile == MICKY )
			{
				//and the items are alcohol, dont enter the shopkeeper
				if( GetArmsDealerItemTypeFromItemNumber( TempObject.usItem ) == ARMS_DEALER_ALCOHOL )
					return;
			}
			*/

			if ( NPCHasUnusedRecordWithGivenApproach( pTSoldier->ubProfile, APPROACH_BUYSELL ) )
			{
				TriggerNPCWithGivenApproach( pTSoldier->ubProfile, APPROACH_BUYSELL, TRUE );
				return;
			}
			// now also check for buy/sell lines (Oct 13)
			/*
			else if ( NPCWillingToAcceptItem( pTSoldier->ubProfile, pSoldier->ubProfile, &TempObject ) )
			{
				TriggerNPCWithGivenApproach( pTSoldier->ubProfile, APPROACH_GIVINGITEM, TRUE );
				return;
			}*/
			else if ( !NPCWillingToAcceptItem( pTSoldier->ubProfile, pSoldier->ubProfile, &TempObject ) )
			{

				//Enter the shopkeeper interface
				EnterShopKeeperInterfaceScreen( pTSoldier->ubProfile );

// removed the if, because if the player picked up an item straight from the ground or money strait from the money
// interface, the item would NOT have a bInvPos, therefore it would not get added to the dealer, and would get deleted
//				if ( bInvPos != NO_SLOT )
				{
					// MUST send in NO_SLOT, as the SKI wille expect it to exist in inv if not....
					AddItemToPlayersOfferAreaAfterShopKeeperOpen( &TempObject, NO_SLOT );

/*
Changed because if the player gave 1 item from a pile, the rest of the items in the piule would disappear
					// OK, r	emove the item, as the SKI will give it back once done
					DeleteObj( &( pSoldier->inv[ bInvPos ] ) );
*/


					if ( bInvPos != NO_SLOT )
					{
						RemoveObjFrom( &pSoldier->inv[ bInvPos ], TempObject.ubNumberOfObjects );
					}

					DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
				}

				return;
			}
		}

		// OK< FOR NOW HANDLE NPC's DIFFERENT!
		ubProfile = pTSoldier->ubProfile;

		// 1 ) PLayer to NPC = NPC
		// 2 ) Player to player = player;
		// 3 ) NPC to player = player;
		// 4 ) NPC TO NPC = NPC

		// Switch on target...
		// Are we a player dude.. ( target? )
		if ( ubProfile < FIRST_RPC || RPC_RECRUITED( pTSoldier ) )
		{
			fToTargetPlayer = TRUE;
		}


		if ( fToTargetPlayer )
		{
			// begin giving
			DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );

			// We are a merc, add!
			if ( !AutoPlaceObject( pTSoldier, &TempObject, TRUE ) )
			{
				// Erase!
				if ( bInvPos != NO_SLOT )
				{
					DeleteObj( &( pSoldier->inv[ bInvPos ] ) );
					DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
				}

				AddItemToPool( pSoldier->sGridNo, &TempObject, 1, pSoldier->bLevel, 0 , -1 );

				// We could not place it!
				// Drop it on the ground?
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ ITEM_HAS_BEEN_PLACED_ON_GROUND_STR ], ShortItemNames[ usItemNum ] );

				// OK, disengage buddy
				pSoldier->uiStatusFlags &= (~SOLDIER_ENGAGEDINACTION );
				pTSoldier->uiStatusFlags &= (~SOLDIER_ENGAGEDINACTION );
			}
			else
			{
				// Erase!
				if ( bInvPos != NO_SLOT )
				{
					DeleteObj( &( pSoldier->inv[ bInvPos ] ) );
					DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
				}

				// OK, it's given, display message!
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ ITEM_HAS_BEEN_GIVEN_TO_STR ], ShortItemNames[ usItemNum ], pTSoldier->name );
				if (usItemNum == MONEY)
				{
					// are we giving money to an NPC, to whom we owe money?
					if (pTSoldier->ubProfile != NO_PROFILE && gMercProfiles[pTSoldier->ubProfile].iBalance < 0)
					{
						gMercProfiles[pTSoldier->ubProfile].iBalance += TempObject.uiMoneyAmount;
						if (gMercProfiles[pTSoldier->ubProfile].iBalance >= 0)
						{
							// don't let the player accumulate credit (?)
							gMercProfiles[pTSoldier->ubProfile].iBalance = 0;

							// report the payment and set facts to indicate people not being owed money
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ GUY_HAS_BEEN_PAID_IN_FULL_STR ], pTSoldier->name );
						}
						else
						{
							// report the payment
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ GUY_STILL_OWED_STR ], pTSoldier->name, -gMercProfiles[pTSoldier->ubProfile].iBalance );
						}
					}
				}
			}
		}
		else
		{
			// Erase!
			if ( bInvPos != NO_SLOT )
			{
				RemoveObjs( &(pSoldier->inv[ bInvPos ]), TempObject.ubNumberOfObjects );
				DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
			}

			// Now intiate conv
			InitiateConversation( pTSoldier, pSoldier, APPROACH_GIVINGITEM, (INT32)&TempObject );
		}
	}

	// OK, disengage buddy
	pSoldier->uiStatusFlags &= (~SOLDIER_ENGAGEDINACTION );
	pTSoldier->uiStatusFlags &= (~SOLDIER_ENGAGEDINACTION );

}



INT16 AdjustGridNoForItemPlacement( SOLDIERTYPE *pSoldier, INT16 sGridNo )
{
	STRUCTURE		*pStructure;
	INT16				sDesiredLevel;
	INT16				sActionGridNo;
	BOOLEAN			fStructFound = FALSE;
	UINT8				ubDirection;
	INT16				sAdjustedGridNo;
	UINT8				ubTargetID;


	sActionGridNo = sGridNo;

	// Check structure database
	if( gpWorldLevelData[ sGridNo ].pStructureHead )
	{
		// Something is here, check obstruction in future
		sDesiredLevel = pSoldier->bLevel ? STRUCTURE_ON_ROOF : STRUCTURE_ON_GROUND;
		pStructure = FindStructure( (INT16)sGridNo, STRUCTURE_BLOCKSMOVES );
		while( pStructure )
		{
			if( !(pStructure->fFlags & STRUCTURE_PASSABLE) && pStructure->sCubeOffset == sDesiredLevel )
			{
				// Check for openable flag....
				//if ( pStructure->fFlags & ( STRUCTURE_OPENABLE | STRUCTURE_HASITEMONTOP ) )
				{
					fStructFound = TRUE;
					break;
				}
			}
			pStructure = FindNextStructure( pStructure, STRUCTURE_BLOCKSMOVES );
		}
	}

	// ATE: IF a person is found, use adjacent gridno for it!
	ubTargetID = WhoIsThere2( sGridNo, pSoldier->bLevel );

	if ( fStructFound || ( ubTargetID != NOBODY && ubTargetID != pSoldier->ubID ) )
	{
		// GET ADJACENT GRIDNO
		sActionGridNo =  FindAdjacentGridEx( pSoldier, sGridNo, &ubDirection, &sAdjustedGridNo, FALSE, FALSE );

		if ( sActionGridNo == -1 )
		{
			sActionGridNo = sAdjustedGridNo;
		}
	}

	return( sActionGridNo );
}


static void BombMessageBoxCallBack(UINT8 ubExitValue);


static void StartBombMessageBox(SOLDIERTYPE* pSoldier, INT16 sGridNo)
{
	UINT8 ubRoom;

	gpTempSoldier = pSoldier;
	gsTempGridno = sGridNo;
	if (pSoldier->inv[HANDPOS].usItem == REMOTEBOMBTRIGGER)
	{
		DoMessageBox( MSG_BOX_BASIC_SMALL_BUTTONS, TacticalStr[ CHOOSE_BOMB_FREQUENCY_STR ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS, BombMessageBoxCallBack, NULL );
	}
	else if (pSoldier->inv[HANDPOS].usItem == REMOTETRIGGER)
	{
		#ifdef JA2DEMO
			{
				UINT8 ubRoom;

				if ( InARoom( pSoldier->sGridNo, &ubRoom ) && ubRoom == 31 )
				{
					SetOffBombsByFrequency( pSoldier->ubID, FIRST_MAP_PLACED_FREQUENCY + 4 );

					DoMercBattleSound( pSoldier, BATTLE_SOUND_OK1 );
				}
				else
				{
					DoMercBattleSound( pSoldier, BATTLE_SOUND_CURSE1 );
				}
			}
		#else
      // PLay sound....
      PlayJA2Sample(USE_STATUE_REMOTE, HIGHVOLUME, 1, MIDDLEPAN);


			// Check what sector we are in....
			if ( gWorldSectorX == 3 && gWorldSectorY == MAP_ROW_O && gbWorldSectorZ == 0 )
			{
				if ( InARoom( pSoldier->sGridNo, &ubRoom ) && ubRoom == 4 )
				{
					DoMercBattleSound( pSoldier, BATTLE_SOUND_OK1 );

					// Open statue
					ChangeO3SectorStatue( FALSE );

				}
				else
				{
					DoMercBattleSound( pSoldier, BATTLE_SOUND_CURSE1 );
				}
			}
			else
			{
				DoMercBattleSound( pSoldier, BATTLE_SOUND_CURSE1 );
			}

		#endif
	}
	else if ( FindAttachment( &(pSoldier->inv[HANDPOS]), DETONATOR) != ITEM_NOT_FOUND )
	{
		DoMessageBox( MSG_BOX_BASIC_SMALL_BUTTONS, TacticalStr[ CHOOSE_TIMER_STR ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS, BombMessageBoxCallBack, NULL );
	}
	else if ( FindAttachment( &(pSoldier->inv[HANDPOS]), REMDETONATOR) != ITEM_NOT_FOUND )
	{
		DoMessageBox( MSG_BOX_BASIC_SMALL_BUTTONS, TacticalStr[ CHOOSE_REMOTE_FREQUENCY_STR ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS, BombMessageBoxCallBack, NULL );
	}
}


static void BombMessageBoxCallBack(UINT8 ubExitValue)
{
	if (gpTempSoldier)
	{
		if (gpTempSoldier->inv[HANDPOS].usItem == REMOTEBOMBTRIGGER)
		{
			SetOffBombsByFrequency( gpTempSoldier->ubID, ubExitValue );
		}
		else
		{
			INT32 iResult;

			if (FindAttachment( &(gpTempSoldier->inv[HANDPOS]), REMDETONATOR ) != ITEM_NOT_FOUND )
			{
				iResult = SkillCheck( gpTempSoldier, PLANTING_REMOTE_BOMB_CHECK, 0 );
			}
			else
			{
				iResult = SkillCheck( gpTempSoldier, PLANTING_BOMB_CHECK, 0 );
			}

			if ( iResult >= 0 )
			{
				// EXPLOSIVES GAIN (25):  Place a bomb, or buried and armed a mine
				StatChange( gpTempSoldier, EXPLODEAMT, 25, FALSE );
			}
			else
			{
				// EXPLOSIVES GAIN (10):  Failed to place a bomb, or bury and arm a mine
				StatChange( gpTempSoldier, EXPLODEAMT, 10, FROM_FAILURE );

				// oops!  How badly did we screw up?
				if ( iResult >= -20 )
				{
					// messed up the setting
					if ( ubExitValue == 0 )
					{
						ubExitValue = 1;
					}
					else
					{
						// change up/down by 1
						ubExitValue = (UINT8) (ubExitValue + Random( 3 ) - 1);
					}
					// and continue
				}
				else
				{
					// OOPS! ... BOOM!
					IgniteExplosion( NOBODY, gpTempSoldier->sX, gpTempSoldier->sY, (INT16) (gpWorldLevelData[gpTempSoldier->sGridNo].sHeight), gpTempSoldier->sGridNo, gpTempSoldier->inv[ HANDPOS ].usItem, gpTempSoldier->bLevel );
					return;
				}
			}

			if ( ArmBomb( &(gpTempSoldier->inv[HANDPOS]), ubExitValue ) )
			{
				gpTempSoldier->inv[ HANDPOS ].bTrap = __min( 10, ( EffectiveExplosive( gpTempSoldier ) / 20) + (EffectiveExpLevel( gpTempSoldier ) / 3) );
				// HACK IMMINENT!
				// value of 1 is stored in maps for SIDE of bomb owner... when we want to use IDs!
				// so we add 2 to all owner IDs passed through here and subtract 2 later
				gpTempSoldier->inv[HANDPOS].ubBombOwner = gpTempSoldier->ubID + 2;
				AddItemToPool( gsTempGridno, &(gpTempSoldier->inv[HANDPOS]), 1, gpTempSoldier->bLevel, WORLD_ITEM_ARMED_BOMB, 0 );
				DeleteObj( &(gpTempSoldier->inv[HANDPOS]) );
			}
		}
	}

}


static BOOLEAN HandItemWorks(SOLDIERTYPE* pSoldier, INT8 bSlot)
{
	BOOLEAN							fItemJustBroke = FALSE, fItemWorks = TRUE;
	OBJECTTYPE *				pObj;

	pObj = &( pSoldier->inv[ bSlot ] );

	// if the item can be damaged, than we must check that it's in good enough
	// shape to be usable, and doesn't break during use.
	// Exception: land mines.  You can bury them broken, they just won't blow!
	if ( (Item[ pObj->usItem ].fFlags & ITEM_DAMAGEABLE) && (pObj->usItem != MINE) && (Item[ pObj->usItem ].usItemClass != IC_MEDKIT) && pObj->usItem != GAS_CAN )
	{
		// if it's still usable, check whether it breaks
		if ( pObj->bStatus[0] >= USABLE)
		{
			// if a dice roll is greater than the item's status
			if ( (Random(80) + 20) >= (UINT32) (pObj->bStatus[0] + 50) )
			{
				fItemJustBroke = TRUE;
				fItemWorks = FALSE;

				// item breaks, and becomes unusable...  so its status is reduced
				// to somewhere between 1 and the 1 less than USABLE
				pObj->bStatus[0] = (INT8) ( 1 + Random( USABLE - 1 ) );
			}
		}
		else	// it's already unusable
		{
			fItemWorks = FALSE;
		}

		if (!fItemWorks && pSoldier->bTeam == gbPlayerNum)
		{
			// merc says "This thing doesn't work!"
			TacticalCharacterDialogue( pSoldier, QUOTE_USELESS_ITEM );
			if (fItemJustBroke)
			{
				DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
			}
		}
	}

	if ( fItemWorks && bSlot == HANDPOS && Item[ pObj->usItem ].usItemClass == IC_GUN )
	{
		// are we using two guns at once?
		if ( Item[ pSoldier->inv[SECONDHANDPOS].usItem ].usItemClass == IC_GUN &&
			pSoldier->inv[SECONDHANDPOS].bGunStatus >= USABLE &&
			pSoldier->inv[SECONDHANDPOS].ubGunShotsLeft > 0)
		{
			// check the second gun for breakage, and if IT breaks, return false
			return( HandItemWorks( pSoldier, SECONDHANDPOS ) );
		}
	}

	return( fItemWorks );
}


// set off the booby trap in mapscreen
static void SetOffBoobyTrapInMapScreen(SOLDIERTYPE* pSoldier, OBJECTTYPE* pObject)
{
	UINT8 ubPtsDmg = 0;

	// check if trapped item is an explosive, if so then up the amount of dmg
	if( ( pObject -> usItem == TNT )|| (  pObject -> usItem == RDX ) )
	{
		// for explosive
		ubPtsDmg = 0;
	}
	else
	{
		// normal mini grenade dmg
		ubPtsDmg = 0;
	}

	// injure the inventory character
	SoldierTakeDamage( pSoldier, 0, ubPtsDmg, ubPtsDmg, TAKE_DAMAGE_EXPLOSION, NOBODY, NOWHERE, 0, TRUE );

	// play the sound
	PlayJA2Sample(EXPLOSION_1, BTNVOLUME, 1, MIDDLEPAN);
}


static void SetOffBoobyTrap(ITEM_POOL* pItemPool)
{
	if ( pItemPool )
	{
		INT16 sX, sY;
		sX = CenterX( pItemPool->sGridNo );
		sY = CenterY( pItemPool->sGridNo );
		IgniteExplosion( NOBODY, sX, sY, (INT16) (gpWorldLevelData[pItemPool->sGridNo].sHeight + pItemPool->bRenderZHeightAboveLevel), pItemPool->sGridNo, MINI_GRENADE, 0 );
		RemoveItemFromPool( pItemPool->sGridNo, pItemPool->iItemIndex, pItemPool->ubLevel );
	}
}


static void BoobyTrapDialogueCallBack(void);
static void BoobyTrapInMapScreenMessageBoxCallBack(UINT8 ubExitValue);


static BOOLEAN ContinuePastBoobyTrap(SOLDIERTYPE* pSoldier, INT16 sGridNo, INT8 bLevel, INT32 iItemIndex, BOOLEAN fInStrategic, BOOLEAN* pfSaidQuote)
{
	BOOLEAN					fBoobyTrapKnowledge;
	INT8						bTrapDifficulty, bTrapDetectLevel;
	OBJECTTYPE *		pObj;

	pObj = &(gWorldItems[ iItemIndex ].o);

  (*pfSaidQuote) = FALSE;

	if (pObj->bTrap > 0)
	{
		if (pSoldier->bTeam == gbPlayerNum)
		{
			// does the player know about this item?
			fBoobyTrapKnowledge = ((pObj->fFlags & OBJECT_KNOWN_TO_BE_TRAPPED) > 0);

			// blue flag stuff?

			if (!fBoobyTrapKnowledge)
			{
				bTrapDifficulty = pObj->bTrap;
				bTrapDetectLevel = CalcTrapDetectLevel( pSoldier, FALSE );
				if (bTrapDetectLevel >= bTrapDifficulty)
				{
					// spotted the trap!
					pObj->fFlags |= OBJECT_KNOWN_TO_BE_TRAPPED;
					fBoobyTrapKnowledge = TRUE;

					// Make him warn us:

					// Set things up..
					gpBoobyTrapSoldier = pSoldier;
					gpBoobyTrapItemPool = GetItemPoolForIndex( sGridNo, iItemIndex, pSoldier->bLevel );
					gsBoobyTrapGridNo = sGridNo;
				  gbBoobyTrapLevel  = pSoldier->bLevel;
					gfDisarmingBuriedBomb = FALSE;
					gbTrapDifficulty = bTrapDifficulty;

					// And make the call for the dialogue
					SetStopTimeQuoteCallback( BoobyTrapDialogueCallBack );
					TacticalCharacterDialogue( pSoldier, QUOTE_BOOBYTRAP_ITEM );

          (*pfSaidQuote) = TRUE;

					return( FALSE );
				}
			}

			gpBoobyTrapItemPool = GetItemPoolForIndex( sGridNo, iItemIndex, pSoldier->bLevel );
			if (fBoobyTrapKnowledge)
			{
				// have the computer ask us if we want to proceed
				gpBoobyTrapSoldier = pSoldier;
				gsBoobyTrapGridNo = sGridNo;
				gbBoobyTrapLevel  = pSoldier->bLevel;
				gfDisarmingBuriedBomb = FALSE;
				gbTrapDifficulty = pObj->bTrap;

				if( fInStrategic )
				{
					DoMessageBox( MSG_BOX_BASIC_STYLE, TacticalStr[ DISARM_BOOBYTRAP_PROMPT ], MAP_SCREEN, ( UINT8 )MSG_BOX_FLAG_YESNO, BoobyTrapInMapScreenMessageBoxCallBack, NULL );
				}
				else
				{
					DoMessageBox( MSG_BOX_BASIC_STYLE, TacticalStr[ DISARM_BOOBYTRAP_PROMPT ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_YESNO, BoobyTrapMessageBoxCallBack, NULL );
				}
			}
			else
			{
				// oops!
				SetOffBoobyTrap( gpBoobyTrapItemPool );
			}

			return( FALSE );

		}
		// else, enemies etc always know about boobytraps and are not affected by them
	}

	return( TRUE );
}


static void BoobyTrapDialogueCallBack(void)
{
	gfJustFoundBoobyTrap = TRUE;

	// now prompt the user...
	if( guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN )
	{
		DoScreenIndependantMessageBox( TacticalStr[ DISARM_BOOBYTRAP_PROMPT ],  ( UINT8 )MSG_BOX_FLAG_YESNO, BoobyTrapInMapScreenMessageBoxCallBack );
	}
	else
	{
		DoScreenIndependantMessageBox( TacticalStr[ DISARM_BOOBYTRAP_PROMPT ],  ( UINT8 )MSG_BOX_FLAG_YESNO, BoobyTrapMessageBoxCallBack );
	}
}


static void RemoveBlueFlagDialogueCallBack(UINT8 ubExitValue);


static void BoobyTrapMessageBoxCallBack(UINT8 ubExitValue)
{
	if ( gfJustFoundBoobyTrap )
	{
		// NOW award for finding boobytrap
		// WISDOM GAIN:  Detected a booby-trap
		StatChange( gpBoobyTrapSoldier, WISDOMAMT, (UINT16) (3 * gbTrapDifficulty), FALSE );
		// EXPLOSIVES GAIN:  Detected a booby-trap
		StatChange( gpBoobyTrapSoldier, EXPLODEAMT, (UINT16) (3 * gbTrapDifficulty), FALSE );
		gfJustFoundBoobyTrap = FALSE;
	}

	if (ubExitValue == MSG_BOX_RETURN_YES)
	{
		INT32						iCheckResult;
		OBJECTTYPE 			Object;

		iCheckResult = SkillCheck( gpBoobyTrapSoldier, DISARM_TRAP_CHECK, 0 );

		if (iCheckResult >= 0)
		{

			// get the item
			Object = gWorldItems[gpBoobyTrapItemPool->iItemIndex].o;

			// NB owner grossness... bombs 'owned' by the enemy are stored with side value 1 in
			// the map. So if we want to detect a bomb placed by the player, owner is > 1, and
			// owner - 2 gives the ID of the character who planted it
			if ( Object.ubBombOwner > 1 && ( (INT32)Object.ubBombOwner - 2 >= gTacticalStatus.Team[ OUR_TEAM ].bFirstID && Object.ubBombOwner - 2 <= gTacticalStatus.Team[ OUR_TEAM ].bLastID ) )
			{
				// our own bomb! no exp
			}
			else
			{
				// disarmed a boobytrap!
				StatChange( gpBoobyTrapSoldier, EXPLODEAMT, (UINT16) (6 * gbTrapDifficulty), FALSE );
				// have merc say this is good
				DoMercBattleSound( gpBoobyTrapSoldier, BATTLE_SOUND_COOL1 );
			}

			if (gfDisarmingBuriedBomb)
			{
				if (Object.usItem == SWITCH)
				{
					// give the player a remote trigger instead
					CreateItem( REMOTEBOMBTRIGGER, (INT8) (1 + Random( 9 )), &Object );
				}
				else if (Object.usItem == ACTION_ITEM && Object.bActionValue != ACTION_ITEM_BLOW_UP )
				{
					// give the player a detonator instead
					CreateItem( DETONATOR, (INT8) (1 + Random( 9 )), &Object );
				}
				else
				{
					// switch action item to the real item type
					CreateItem( Object.usBombItem, Object.bBombStatus, &Object );
				}

				// remove any blue flag graphic
				RemoveBlueFlag( gsBoobyTrapGridNo, gbBoobyTrapLevel );
			}
			else
			{
				Object.bTrap = 0;
				Object.fFlags &= ~( OBJECT_KNOWN_TO_BE_TRAPPED );
			}

			// place it in the guy's inventory/cursor
			if ( AutoPlaceObject( gpBoobyTrapSoldier, &Object, TRUE ) )
			{
				// remove it from the ground
				RemoveItemFromPool( gsBoobyTrapGridNo, gpBoobyTrapItemPool->iItemIndex, gbBoobyTrapLevel );
			}
			else
			{
				// make sure the item in the world is untrapped
				gWorldItems[ gpBoobyTrapItemPool->iItemIndex ].o.bTrap = 0;
				gWorldItems[ gpBoobyTrapItemPool->iItemIndex ].o.fFlags &= ~( OBJECT_KNOWN_TO_BE_TRAPPED );

				// ATE; If we failed to add to inventory, put failed one in our cursor...
				gfDontChargeAPsToPickup = TRUE;
				HandleAutoPlaceFail( gpBoobyTrapSoldier, gpBoobyTrapItemPool->iItemIndex, gsBoobyTrapGridNo );
				RemoveItemFromPool( gsBoobyTrapGridNo, gpBoobyTrapItemPool->iItemIndex, gbBoobyTrapLevel );
			}
		}
		else
		{
			// oops! trap goes off
      StatChange( gpBoobyTrapSoldier, EXPLODEAMT, (INT8) (3 * gbTrapDifficulty ), FROM_FAILURE );

			DoMercBattleSound( gpBoobyTrapSoldier, BATTLE_SOUND_CURSE1 );

			if (gfDisarmingBuriedBomb)
			{
				SetOffBombsInGridNo( gpBoobyTrapSoldier->ubID, gsBoobyTrapGridNo, TRUE, gbBoobyTrapLevel );
			}
			else
			{
				SetOffBoobyTrap( gpBoobyTrapItemPool );
			}
		}
	}
	else
	{
		if (gfDisarmingBuriedBomb)
		{
			DoMessageBox( MSG_BOX_BASIC_STYLE, TacticalStr[ REMOVE_BLUE_FLAG_PROMPT ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_YESNO, RemoveBlueFlagDialogueCallBack, NULL );
		}
		// otherwise do nothing
	}
}


static void BoobyTrapInMapScreenMessageBoxCallBack(UINT8 ubExitValue)
{
	if ( gfJustFoundBoobyTrap )
	{
		// NOW award for finding boobytrap

		// WISDOM GAIN:  Detected a booby-trap
		StatChange( gpBoobyTrapSoldier, WISDOMAMT, (UINT16) (3 * gbTrapDifficulty), FALSE );
		// EXPLOSIVES GAIN:  Detected a booby-trap
		StatChange( gpBoobyTrapSoldier, EXPLODEAMT, (UINT16) (3 * gbTrapDifficulty), FALSE );
		gfJustFoundBoobyTrap = FALSE;
	}

	if (ubExitValue == MSG_BOX_RETURN_YES)
	{
		INT32						iCheckResult;
		OBJECTTYPE 			Object;

		iCheckResult = SkillCheck( gpBoobyTrapSoldier, DISARM_TRAP_CHECK, 0 );

		if (iCheckResult >= 0)
		{
			// disarmed a boobytrap!
      StatChange( gpBoobyTrapSoldier, EXPLODEAMT, (UINT16) (6 * gbTrapDifficulty), FALSE );


			// have merc say this is good
			DoMercBattleSound( gpBoobyTrapSoldier, BATTLE_SOUND_COOL1 );

			// get the item
			Object = *gpItemPointer;

			if (gfDisarmingBuriedBomb)
			{
				if (Object.usItem == SWITCH)
				{
					// give the player a remote trigger instead
					CreateItem( REMOTEBOMBTRIGGER, (INT8) (1 + Random( 9 )), &Object );
				}
				else if (Object.usItem == ACTION_ITEM && Object.bActionValue != ACTION_ITEM_BLOW_UP )
				{
					// give the player a detonator instead
					CreateItem( DETONATOR, (INT8) (1 + Random( 9 )), &Object );
				}
				else
				{
					// switch action item to the real item type
					CreateItem( Object.usBombItem, Object.bBombStatus, &Object );
				}
			}
			else
			{
				Object.bTrap = 0;
				Object.fFlags &= ~( OBJECT_KNOWN_TO_BE_TRAPPED );
			}

			MAPEndItemPointer( );

			// place it in the guy's inventory/cursor
			if ( !AutoPlaceObject( gpBoobyTrapSoldier, &Object, TRUE ) )
			{
				AutoPlaceObjectInInventoryStash( &Object );
			}

			HandleButtonStatesWhileMapInventoryActive( );
		}
		else
		{
			// oops! trap goes off
      StatChange( gpBoobyTrapSoldier, EXPLODEAMT, (INT8) (3 * gbTrapDifficulty ), FROM_FAILURE );

			DoMercBattleSound( gpBoobyTrapSoldier, BATTLE_SOUND_CURSE1 );

			if (gfDisarmingBuriedBomb)
			{
				SetOffBombsInGridNo( gpBoobyTrapSoldier->ubID, gsBoobyTrapGridNo, TRUE, gbBoobyTrapLevel );
			}
			else
			{
				SetOffBoobyTrap( gpBoobyTrapItemPool );
			}
		}
	}
	else
	{
		if (gfDisarmingBuriedBomb)
		{
			DoMessageBox( MSG_BOX_BASIC_STYLE, TacticalStr[ REMOVE_BLUE_FLAG_PROMPT ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_YESNO, RemoveBlueFlagDialogueCallBack, NULL );
		}
		// otherwise do nothing
	}
}


static void SwitchMessageBoxCallBack(UINT8 ubExitValue)
{
	if ( ubExitValue == MSG_BOX_RETURN_YES )
	{
    // Message that switch is activated...
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[ 60 ] );

		SetOffBombsByFrequency( gpTempSoldier->ubID,  bTempFrequency );
	}
}


static void AddBlueFlag(INT16 sGridNo, INT8 bLevel);


BOOLEAN NearbyGroundSeemsWrong( SOLDIERTYPE * pSoldier, INT16 sGridNo, BOOLEAN fCheckAroundGridno, INT16 * psProblemGridNo )
{
	INT16						sNextGridNo;
	// BOOLEAN fWorthChecking = FALSE, fProblemExists = FALSE, fDetectedProblem = FALSE;
	UINT8						ubDetectLevel, ubDirection;
	MAP_ELEMENT *		pMapElement;
	UINT32					fCheckFlag;
	UINT32					uiWorldBombIndex;
	OBJECTTYPE *		pObj;
	BOOLEAN					fMining, fFoundMetal = FALSE;
//	ITEM_POOL *			pItemPool;
	UINT8						ubMovementCost;

	ubDetectLevel = 0;

	if ( FindObj( pSoldier, METALDETECTOR ) != NO_SLOT )
	{
		fMining = TRUE;
	}
	else
	{
		fMining = FALSE;

		ubDetectLevel = CalcTrapDetectLevel( pSoldier, FALSE );
		/*
		if (pSoldier->bStealthMode)
		{
			ubDetectLevel++;
		}
		switch (pSoldier->usAnimState)
		{
			case CRAWLING:
				ubDetectLevel += 2;
				break;

			case SWATTING:
				ubDetectLevel++;
				break;

			default:
				break;
		}
		*/
	}

	if (pSoldier->bSide == 0)
	{
		fCheckFlag = MAPELEMENT_PLAYER_MINE_PRESENT;
	}
	else
	{
		fCheckFlag = MAPELEMENT_ENEMY_MINE_PRESENT;
	}

	// check every tile around gridno for the presence of "nasty stuff"
	for (ubDirection = 0; ubDirection < 8; ubDirection++)
	{
		if ( fCheckAroundGridno )
		{
			// get the gridno of the next spot adjacent to lastGridno in that direction
			sNextGridNo = NewGridNo( sGridNo, (INT16) DirectionInc( (UINT8) ubDirection ) );

			// don't check directions that are impassable!
			ubMovementCost = gubWorldMovementCosts[ sNextGridNo ][ ubDirection ][ pSoldier->bLevel ];
			if ( IS_TRAVELCOST_DOOR( ubMovementCost ) )
			{
				ubMovementCost = DoorTravelCost( NULL, sNextGridNo, ubMovementCost, FALSE, NULL );
			}
			if ( ubMovementCost >= TRAVELCOST_BLOCKED)
			{
				continue;
			}
		}
		else
		{
			// we should just be checking the gridno
			sNextGridNo = sGridNo;
			ubDirection = 8; // don't loop
		}

		// if this sNextGridNo isn't out of bounds... but it never can be
		pMapElement = &(gpWorldLevelData[sNextGridNo]);

		if (pMapElement->uiFlags & fCheckFlag)
		{
			// already know there's a mine there
			continue;
		}

		// check for boobytraps
		for (uiWorldBombIndex = 0; uiWorldBombIndex < guiNumWorldBombs; uiWorldBombIndex++)
		{
			if (gWorldBombs[uiWorldBombIndex].fExists && gWorldItems[ gWorldBombs[uiWorldBombIndex].iItemIndex ].sGridNo == sNextGridNo)
			{
				pObj = &( gWorldItems[ gWorldBombs[uiWorldBombIndex].iItemIndex ].o );
				if ( pObj->bDetonatorType == BOMB_PRESSURE && !(pObj->fFlags & OBJECT_KNOWN_TO_BE_TRAPPED) && (!(pObj->fFlags & OBJECT_DISABLED_BOMB)) )
				{
					if ( fMining && pObj->bTrap <= 10 )
					{
						// add blue flag
						AddBlueFlag( sNextGridNo, pSoldier->bLevel );
						fFoundMetal = TRUE;
						break;
					}
					else if (ubDetectLevel >= pObj->bTrap)
					{
						if (pSoldier->uiStatusFlags & SOLDIER_PC )
						{
      				// detected exposives buried nearby...
							StatChange( pSoldier, EXPLODEAMT, (UINT16) (pObj->bTrap), FALSE );

							// set item as known
							pObj->fFlags |= OBJECT_KNOWN_TO_BE_TRAPPED;
						}

						*psProblemGridNo = sNextGridNo;
						return( TRUE );
					}
				}
			}
		}

		/*
		// also check for metal items if using a metal detector
		if (fMining)
		{
			// add blue flags where we find metallic objects hidden
			GetItemPool( sNextGridNo, &pItemPool, pSoldier->bLevel );
			while( pItemPool )
			{
				if ( pItemPool->bVisible == BURIED || (pItemPool->bVisible != TRUE && gWorldItems[ pItemPool->iItemIndex ].o.bTrap > 0 ) )
				{
					pObj = &( gWorldItems[ pItemPool->iItemIndex ].o );
					if ( pObj->usItem == ACTION_ITEM && pObj-> )
					{
						switch( pObj->bActionValue )
						{
							case ACTION_ITEM_BLOW_UP:
							case ACTION_ITEM_LOCAL_ALARM:
							case ACTION_ITEM_GLOBAL_ALARM:
								// add blue flag
								AddBlueFlag( sNextGridNo, pSoldier->bLevel );
								fFoundMetal = TRUE;
								break;
							default:
								break;

						}
					}
					else if (Item[ pObj->usItem ].fFlags & ITEM_METAL)
					{
						// add blue flag
						AddBlueFlag( sNextGridNo, pSoldier->bLevel );
						fFoundMetal = TRUE;
						break;
					}
				}
				pItemPool = pItemPool->pNext;
			}
		}
		*/

	}

	*psProblemGridNo = NOWHERE;
	if (fFoundMetal)
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


static void MineSpottedLocatorCallback(void);


void MineSpottedDialogueCallBack( void )
{
	ITEM_POOL * pItemPool;

  // ATE: REALLY IMPORTANT - ALL CALLBACK ITEMS SHOULD UNLOCK
  gTacticalStatus.fLockItemLocators = FALSE;

	GetItemPool( gsBoobyTrapGridNo, &pItemPool, gbBoobyTrapLevel );

	guiPendingOverrideEvent = LU_BEGINUILOCK;

	// play a locator at the location of the mine
	SetItemPoolLocatorWithCallback( pItemPool, MineSpottedLocatorCallback );
}


static void MineSpottedMessageBoxCallBack(UINT8 ubExitValue);


static void MineSpottedLocatorCallback(void)
{
	guiPendingOverrideEvent = LU_ENDUILOCK;

	// now ask the player if he wants to place a blue flag.
	DoMessageBox( MSG_BOX_BASIC_STYLE, TacticalStr[ PLACE_BLUE_FLAG_PROMPT ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_YESNO, MineSpottedMessageBoxCallBack, NULL );
}


static void MineSpottedMessageBoxCallBack(UINT8 ubExitValue)
{
	if (ubExitValue == MSG_BOX_RETURN_YES)
	{
		// place a blue flag where the mine was found
		AddBlueFlag( gsBoobyTrapGridNo, gbBoobyTrapLevel );
	}
}


static void RemoveBlueFlagDialogueCallBack(UINT8 ubExitValue)
{
	if (ubExitValue == MSG_BOX_RETURN_YES)
	{
		RemoveBlueFlag( gsBoobyTrapGridNo, gbBoobyTrapLevel );
	}
}


static void AddBlueFlag(INT16 sGridNo, INT8 bLevel)
{
	LEVELNODE *pNode;

	ApplyMapChangesToMapTempFile( TRUE );
	gpWorldLevelData[ sGridNo ].uiFlags |= MAPELEMENT_PLAYER_MINE_PRESENT;

  pNode = AddStructToTail( sGridNo, BLUEFLAG_GRAPHIC );

	if ( pNode )
	{
		pNode->uiFlags |= LEVELNODE_SHOW_THROUGH;
	}

	ApplyMapChangesToMapTempFile( FALSE );
	RecompileLocalMovementCostsFromRadius( sGridNo, bLevel );
	SetRenderFlags(RENDER_FLAG_FULL);
}

void RemoveBlueFlag( INT16 sGridNo, INT8 bLevel )
{
	ApplyMapChangesToMapTempFile( TRUE );
	gpWorldLevelData[sGridNo].uiFlags &= ~(MAPELEMENT_PLAYER_MINE_PRESENT);

  if ( bLevel == 0 )
  {
	  RemoveStruct( sGridNo, BLUEFLAG_GRAPHIC );
  }
  else
  {
    RemoveOnRoof( sGridNo, BLUEFLAG_GRAPHIC );
  }

	ApplyMapChangesToMapTempFile( FALSE );
	RecompileLocalMovementCostsFromRadius( sGridNo, bLevel );
	SetRenderFlags(RENDER_FLAG_FULL);
}

void MakeNPCGrumpyForMinorOffense( SOLDIERTYPE * pSoldier, SOLDIERTYPE *pOffendingSoldier )
{
	CancelAIAction( pSoldier, TRUE );

	switch( pSoldier->ubProfile )
	{
		case FREDO:
		case FRANZ:
		case HERVE:
		case PETER:
		case ALBERTO:
		case CARLO:
		case MANNY:
		case GABBY:
		case ARNIE:
		case HOWARD:
		case SAM:
		case FATHER:
		case TINA:
		case ARMAND:
		case WALTER:
			gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags3 |= PROFILE_MISC_FLAG3_NPC_PISSED_OFF;
			TriggerNPCWithIHateYouQuote( pSoldier->ubProfile );
			break;
		default:
			// trigger NPCs with quote if available
			AddToShouldBecomeHostileOrSayQuoteList( pSoldier->ubID );
			break;
	}

  if ( pOffendingSoldier )
  {
	  pSoldier->bNextAction = AI_ACTION_CHANGE_FACING;
	  pSoldier->usNextActionData = atan8( pSoldier->sX, pSoldier->sY, pOffendingSoldier->sX, pOffendingSoldier->sY );
  }
}


static void TestPotentialOwner(SOLDIERTYPE* pSoldier)
{
	if ( pSoldier->bActive && pSoldier->bInSector && pSoldier->bLife >= OKLIFE )
	{
		if ( SoldierToSoldierLineOfSightTest( pSoldier, gpTempSoldier, (UINT8) DistanceVisible( pSoldier, DIRECTION_IRRELEVANT, 0, gpTempSoldier->sGridNo, gpTempSoldier->bLevel ), TRUE ) )
		{
			MakeNPCGrumpyForMinorOffense( pSoldier, gpTempSoldier );
		}
	}
}


static void CheckForPickedOwnership(void)
{
	ITEM_POOL *				pItemPool;
	UINT8							ubProfile;
	UINT8							ubCivGroup;
	SOLDIERTYPE *			pSoldier;
	UINT8							ubLoop;

	// LOOP THROUGH LIST TO FIND NODE WE WANT
	GetItemPool( gsTempGridno, &pItemPool, gpTempSoldier->bLevel );

	while( pItemPool )
	{
		if ( gWorldItems[ pItemPool->iItemIndex ].o.usItem == OWNERSHIP )
		{
			if ( gWorldItems[ pItemPool->iItemIndex ].o.ubOwnerProfile != NO_PROFILE )
			{
				ubProfile = gWorldItems[ pItemPool->iItemIndex ].o.ubOwnerProfile;
				pSoldier = FindSoldierByProfileID( ubProfile, FALSE );
				if ( pSoldier )
				{
					TestPotentialOwner( pSoldier );
				}
			}
			if ( gWorldItems[ pItemPool->iItemIndex ].o.ubOwnerCivGroup != NON_CIV_GROUP )
			{
				ubCivGroup = gWorldItems[ pItemPool->iItemIndex ].o.ubOwnerCivGroup;
				if ( ubCivGroup == HICKS_CIV_GROUP && CheckFact( FACT_HICKS_MARRIED_PLAYER_MERC, 0 ) )
				{
					// skip because hicks appeased
					pItemPool = pItemPool->pNext;
					continue;
				}
				for ( ubLoop = gTacticalStatus.Team[ CIV_TEAM ].bFirstID; ubLoop <= gTacticalStatus.Team[ CIV_TEAM ].bLastID; ubLoop++ )
				{
					pSoldier = MercPtrs[ ubLoop ];
					if ( pSoldier && pSoldier->ubCivilianGroup == ubCivGroup )
					{
						TestPotentialOwner( pSoldier );
					}
				}
			}
		}
		pItemPool = pItemPool->pNext;
	}

}


static void LoopLevelNodeForItemGlowFlag(LEVELNODE* pNode, INT16 sGridNo, UINT8 ubLevel, BOOLEAN fOn)
{
	while ( pNode != NULL )
	{
		if ( pNode->uiFlags & LEVELNODE_ITEM )
		{
			if ( fOn )
			{
				pNode->uiFlags |= LEVELNODE_DYNAMIC;
			}
			else
			{
				pNode->uiFlags &= (~LEVELNODE_DYNAMIC);
			}
		}
		pNode = pNode->pNext;
	}
}


static void HandleItemGlowFlag(INT16 sGridNo, UINT8 ubLevel, BOOLEAN fOn)
{
	LEVELNODE *pNode;

	if ( ubLevel == 0 )
	{
		pNode = gpWorldLevelData[ sGridNo ].pStructHead;
		LoopLevelNodeForItemGlowFlag( pNode, sGridNo,  ubLevel, fOn );
	}
	else
	{
		pNode = gpWorldLevelData[ sGridNo ].pOnRoofHead;
		LoopLevelNodeForItemGlowFlag( pNode, sGridNo, ubLevel, fOn );
	}
}

void ToggleItemGlow( BOOLEAN fOn )
{
	UINT32 cnt;

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		HandleItemGlowFlag( ( INT16 )cnt, 0, fOn );
		HandleItemGlowFlag( ( INT16 )cnt, 1, fOn );
	}

	if ( !fOn )
	{
		gGameSettings.fOptions[ TOPTION_GLOW_ITEMS ] = FALSE;
	}
	else
	{
		gGameSettings.fOptions[ TOPTION_GLOW_ITEMS ] = TRUE;
	}

	SetRenderFlags(RENDER_FLAG_FULL);
}

BOOLEAN ContinuePastBoobyTrapInMapScreen( OBJECTTYPE *pObject, SOLDIERTYPE *pSoldier )
{
	BOOLEAN					fBoobyTrapKnowledge;
	INT8						bTrapDifficulty, bTrapDetectLevel;

	if (pObject->bTrap > 0)
	{
		if (pSoldier->bTeam == gbPlayerNum)
		{
			// does the player know about this item?
			fBoobyTrapKnowledge = ((pObject->fFlags & OBJECT_KNOWN_TO_BE_TRAPPED) > 0);

			// blue flag stuff?

			if (!fBoobyTrapKnowledge)
			{
				bTrapDifficulty = pObject->bTrap;
				bTrapDetectLevel = CalcTrapDetectLevel( pSoldier, FALSE );
				if (bTrapDetectLevel >= bTrapDifficulty)
				{
					// spotted the trap!
					pObject->fFlags |= OBJECT_KNOWN_TO_BE_TRAPPED;
					fBoobyTrapKnowledge = TRUE;

					// Make him warn us:
					gpBoobyTrapSoldier = pSoldier;

					// And make the call for the dialogue
					SetStopTimeQuoteCallback( BoobyTrapDialogueCallBack );
					TacticalCharacterDialogue( pSoldier, QUOTE_BOOBYTRAP_ITEM );

					return( FALSE );
				}
			}

			if (fBoobyTrapKnowledge)
			{
				// have the computer ask us if we want to proceed
				gpBoobyTrapSoldier = pSoldier;
				gbTrapDifficulty = pObject->bTrap;
				DoMessageBox( MSG_BOX_BASIC_STYLE, TacticalStr[ DISARM_BOOBYTRAP_PROMPT ], MAP_SCREEN, ( UINT8 )MSG_BOX_FLAG_YESNO, BoobyTrapInMapScreenMessageBoxCallBack, NULL );
			}
			else
			{
				// oops!
				SetOffBoobyTrapInMapScreen( pSoldier, pObject );
			}

			return( FALSE );

		}
		// else, enemies etc always know about boobytraps and are not affected by them
	}

	return( TRUE );
}


// Well, clears all item pools
static void ClearAllItemPools(void)
{
	UINT32 cnt;

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		RemoveItemPool( (INT16)cnt, 0 );
		RemoveItemPool( (INT16)cnt, 1 );
	}
}


// Refresh item pools
void RefreshItemPools( WORLDITEM * pItemList, INT32 iNumberOfItems )
{
	ClearAllItemPools( );

	RefreshWorldItemsIntoItemPools(  pItemList, iNumberOfItems );
}


INT16 FindNearestAvailableGridNoForItem( INT16 sSweetGridNo, INT8 ubRadius )
{
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2, cnt3;
	INT16		sGridNo;
	INT32		uiRange, uiLowestRange = 999999;
	INT16		sLowestGridNo=0;
	INT32					leftmost;
	BOOLEAN	fFound = FALSE;
	SOLDIERTYPE soldier;
	UINT8 ubSaveNPCAPBudget;
	UINT8 ubSaveNPCDistLimit;

	cnt3 = 0;

	//Save AI pathing vars.  changing the distlimit restricts how
	//far away the pathing will consider.
	ubSaveNPCAPBudget = gubNPCAPBudget;
	ubSaveNPCDistLimit = gubNPCDistLimit;
	gubNPCAPBudget = 0;
	gubNPCDistLimit = ubRadius;

	//create dummy soldier, and use the pathing to determine which nearby slots are
	//reachable.
	memset( &soldier, 0, sizeof( SOLDIERTYPE ) );
	soldier.bTeam = 1;
	soldier.sGridNo = sSweetGridNo;

	sTop		= ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	//clear the mapelements of potential residue MAPELEMENT_REACHABLE flags
	//in the square region.
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + (WORLD_COLS * cnt1) + cnt2;
			if ( sGridNo >=0 && sGridNo < WORLD_MAX )
			{
				gpWorldLevelData[ sGridNo ].uiFlags &= (~MAPELEMENT_REACHABLE);
			}
		}
	}

	//Now, find out which of these gridnos are reachable
	//(use the fake soldier and the pathing settings)
	FindBestPath( &soldier, NOWHERE, 0, WALKING, COPYREACHABLE, 0 );

	uiLowestRange = 999999;

	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + ( WORLD_COLS * cnt1 ) + cnt2;
			if ( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS ) &&
				gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE )
			{
				// Go on sweet stop
				if ( NewOKDestination( &soldier, sGridNo, TRUE, soldier.bLevel ) )
				{
					uiRange = GetRangeInCellCoordsFromGridNoDiff( sSweetGridNo, sGridNo );

					if ( uiRange < uiLowestRange )
					{
						sLowestGridNo = sGridNo;
						uiLowestRange = uiRange;
						fFound = TRUE;
					}
				}
			}
		}
	}
	gubNPCAPBudget = ubSaveNPCAPBudget;
	gubNPCDistLimit = ubSaveNPCDistLimit;
	if ( fFound )
	{
		return sLowestGridNo;
	}
	return NOWHERE;
}


BOOLEAN CanPlayerUseRocketRifle( SOLDIERTYPE *pSoldier, BOOLEAN fDisplay )
{
	if ( pSoldier->inv[ pSoldier->ubAttackingHand ].usItem == ROCKET_RIFLE || pSoldier->inv[ pSoldier->ubAttackingHand ].usItem == AUTO_ROCKET_RIFLE )
  {
    // check imprint ID
    // NB not-imprinted value is NO_PROFILE
    // imprinted value is profile for mercs & NPCs and NO_PROFILE + 1 for generic dudes
    if (pSoldier->ubProfile != NO_PROFILE)
    {
	    if ( pSoldier->inv[ pSoldier->ubAttackingHand ].ubImprintID != pSoldier->ubProfile )
	    {
        // NOT a virgin gun...
		    if ( pSoldier->inv[ pSoldier->ubAttackingHand ].ubImprintID != NO_PROFILE )
		    {
			    // access denied!
			    if (pSoldier->bTeam == gbPlayerNum)
			    {
				    PlayJA2Sample(RG_ID_INVALID, HIGHVOLUME, 1, MIDDLE);

            if ( fDisplay )
            {
      			  ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, L"\"%ls\"", TacticalStr[ GUN_NOGOOD_FINGERPRINT ] );
            }
			    }
			    return( FALSE );
		    }
	    }
    }
  }
  return( TRUE );
}
