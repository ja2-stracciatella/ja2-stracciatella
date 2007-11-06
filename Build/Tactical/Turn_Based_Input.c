#include "Real_Time_Input.h"
#include "Soldier_Find.h"
#include "Turn_Based_Input.h"
#include "WCheck.h"
#include "Debug.h"
#include "JAScreens.h"
#include "PathAI.h"
#include "Soldier_Control.h"
#include "Animation_Control.h"
#include "Animation_Data.h"
#include "Timer_Control.h"
#include "Handle_UI.h"
#include "Isometric_Utils.h"
#include "Input.h"
#include "Overhead.h"
#include "Sys_Globals.h"
#include "ScreenIDs.h"
#include "Interface.h"
#include "Cursor_Control.h"
#include "Points.h"
#include "Interactive_Tiles.h"
#include "Interface_Cursors.h"
#include "Weapons.h"
#include "Lighting.h"
#include "Sound_Control.h"
#include "Environment.h"
#include "Music_Control.h"
#include "AI.h"
#include "Font_Control.h"
#include "WorldMan.h"
#include "Handle_UI_Plan.h"
#include "Message.h"
#include "Overhead_Map.h"
#include "World_Items.h"
#include "Game_Clock.h"
#include "Interface_Items.h"
#include "Physics.h"
#include "UI_Cursors.h"
#include "StrategicMap.h"
#include "Soldier_Profile.h"
#include "Soldier_Create.h"
#include "Soldier_Add.h"
#include "Dialogue_Control.h"
#include "Interface_Dialogue.h"
#include "OppList.h"
#include "MessageBoxScreen.h"
#include "Gameloop.h"
#include "Spread_Burst.h"
#include "Tile_Animation.h"
#include "Merc_Entering.h"
#include "Explosion_Control.h"
#include "Assignments.h"
#include "EditScreen.h"
#include "Structure_Wrap.h"
#include "Exit_Grids.h"
#include "Strategic_Exit_GUI.h"
#include "Auto_Bandage.h"
#include "Options_Screen.h"
#include "Squads.h"
#include "Interface_Panels.h"
#include "Soldier_Tile.h"
#include "Soldier_Functions.h"
#include "Game_Events.h"
#include "English.h"
#include "Random.h"
#include "Map_Screen_Interface.h"
#include "RenderWorld.h"
#include "Quest_Debug_System.h"
//#include "medical.h"
#include "Arms_Dealer_Init.h"
#include "ShopKeeper_Interface.h"
#include "GameSettings.h"
#include "Vehicles.h"
#include "SaveLoadScreen.h"
#include "Air_Raid.h"
#include "Meanwhile.h"
#include "Text.h"
#include "Inventory_Choosing.h"
#include "Soldier_Macros.h"
#include "HelpScreen.h"
#include "Cheats.h"
#include "DisplayCover.h"
#include "History.h"
#include "Strategic_Status.h"
#include "Queen_Command.h"
#include "PreBattle_Interface.h"
#include "_JA25EnglishText.h"
#include "VSurface.h"
#include "MemMan.h"
#include "Button_System.h"
#include "Items.h"
#include "Stubs.h" // XXX

#ifdef JA2TESTVERSION
#	include "Ambient_Control.h"
#endif

#ifdef JA2BETAVERSION
#	include "Strategic_AI.h"
#endif


static BOOLEAN gfFirstCycleMovementStarted = FALSE;

UINT32 guiUITargetSoldierId = NOBODY;


static SOLDIERTYPE* gpExchangeSoldier1;
static SOLDIERTYPE* gpExchangeSoldier2;


UINT8			gubCheatLevel		= STARTING_CHEAT_LEVEL;


static void QueryTBLeftButton(UINT32* puiNewEvent);
static void QueryTBRightButton(UINT32* puiNewEvent);


void	GetTBMouseButtonInput( UINT32 *puiNewEvent )
{
	 QueryTBLeftButton( puiNewEvent );
	 QueryTBRightButton( puiNewEvent );
}


static void QueryTBLeftButton(UINT32* puiNewEvent)
{
	UINT16						usMapPos;
	static BOOLEAN	fClickHoldIntercepted = FALSE;
	static BOOLEAN  fCanCheckForSpeechAdvance = FALSE;
	static INT16		sMoveClickGridNo					= 0;


	// LEFT MOUSE BUTTON
  if ( gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		if (!GetMouseMapPos( &usMapPos ) && !gfUIShowExitSouth )
		{
			return;
		}

		if ( gViewportRegion.ButtonState & MSYS_LEFT_BUTTON )
		{
			if ( !fLeftButtonDown )
			{
				fLeftButtonDown = TRUE;
				RESETCOUNTER( LMOUSECLICK_DELAY_COUNTER );

				{
					switch( gCurrentUIMode )
					{
						case CONFIRM_ACTION_MODE:
						{
							SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
							if (pSoldier != NULL) pSoldier->sStartGridNo = usMapPos;
							break;
						}

						case MOVE_MODE:

							if ( giUIMessageOverlay != -1 )
							{
								EndUIMessage( );
							}
							else
							{
								if ( !HandleCheckForExitArrowsInput( FALSE ) && gpItemPointer == NULL )
								{
									// First check if we clicked on a guy, if so, make selected if it's ours
									const SOLDIERTYPE* const tgt = gUIFullTarget;
									if (tgt != NULL && guiUIFullTargetFlags & OWNED_MERC)
									{
										 if ( !( guiUIFullTargetFlags & UNCONSCIOUS_MERC ) )
										 {
  										 fClickHoldIntercepted = TRUE;

											 // Select guy
											if (gpItemPointer == NULL)
											 {
													if (tgt->bAssignment >= ON_DUTY)
													{
														// do nothing
      										  fClickHoldIntercepted = FALSE;
													}
													else
													{
														*puiNewEvent = I_SELECT_MERC;
													}
												}
												else
												{
											 		*puiNewEvent = I_SELECT_MERC;
												}
										 }
									}
									else
									{
										if ( InUIPlanMode( ) )
										{
											AddUIPlan( usMapPos, UIPLAN_ACTION_MOVETO );
										}
										else
										{
											// We're on terrain in which we can walk, walk
											// If we're on terrain,
											if ( gusSelectedSoldier != NO_SOLDIER )
											{
												INT8 bReturnVal = FALSE;

												const SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);

												bReturnVal = HandleMoveModeInteractiveClick( usMapPos, puiNewEvent );

												// All's OK for interactive tile?
												if ( bReturnVal == -2 )
												{
													// Confirm!
													if ( SelectedMercCanAffordMove(  )  )
													{
														*puiNewEvent = C_WAIT_FOR_CONFIRM;
													}
												}
												else if ( bReturnVal == 0 )
												{
													if ( gfUIAllMoveOn )
													{
														 *puiNewEvent = C_WAIT_FOR_CONFIRM;
													}
													else
													{
														if ( gsCurrentActionPoints == 0 )
														{
															ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
														}
														else if ( SelectedMercCanAffordMove(  )  )
														{
															const BOOLEAN fResult = UIOKMoveDestination(GetSelectedMan(), usMapPos);
															if (fResult == 1)
															{
																// ATE: CHECK IF WE CAN GET TO POSITION
																// Check if we are not in combat
																if ( gsCurrentActionPoints == 0 )
																{
																	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
																}
																else
																{
																	*puiNewEvent = C_WAIT_FOR_CONFIRM;
																}
															}
															else
															{
																if ( fResult == 2 )
																{
																	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NOBODY_USING_REMOTE_STR ] );
																}
																else
																{
																	//if ( usMapPos != sMoveClickGridNo || pSoldier->uiStatusFlags & SOLDIER_ROBOT )
																	//{
																	//	sMoveClickGridNo					= usMapPos;

																		//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ CANT_MOVE_THERE_STR ] );
																		// Goto hand cursor mode....
																	//	*puiNewEvent					  = M_CHANGE_TO_HANDMODE;
																	//	gsOverItemsGridNo				= usMapPos;
																	//	gsOverItemsLevel				= gsInterfaceLevel;
																	//}
																	//else
																	//{
																	//	sMoveClickGridNo = 0;
																	//	*puiNewEvent = M_CHANGE_TO_HANDMODE;
																	//}

																}
																//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, L"Invalid move destination." );
															}
														}
													}
												}
												// OK, our first right-click is an all-cycle
												gfUICanBeginAllMoveCycle = FALSE;
											}
											fClickHoldIntercepted = TRUE;

										}
									}
								}
								else
								{
									fClickHoldIntercepted = TRUE;
									fIgnoreLeftUp = TRUE;
								}

							}
							break;
					}
				}
				if ( gfUIWaitingForUserSpeechAdvance )
				{
					fCanCheckForSpeechAdvance = TRUE;
				}

			}

			// HERE FOR CLICK-DRAG CLICK
			switch( gCurrentUIMode )
			{

				case MOVE_MODE:
						// First check if we clicked on a guy, if so, make selected if it's ours
						if (gUIFullTarget != NULL &&
								guiUIFullTargetFlags & SELECTED_MERC &&
								!(guiUIFullTargetFlags & UNCONSCIOUS_MERC) &&
								!(gUIFullTarget->uiStatusFlags & SOLDIER_VEHICLE))
						{
							*puiNewEvent = M_CHANGE_TO_ADJPOS_MODE;
							fIgnoreLeftUp = FALSE;
						}
						break;
			}

			// IF HERE, DO A CLICK-HOLD IF IN INTERVAL
			if ( COUNTERDONE( LMOUSECLICK_DELAY_COUNTER ) && !fClickHoldIntercepted )
			{
				// Switch on UI mode
				switch( gCurrentUIMode )
				{

				}
			}


		}
		else
		{
			if ( fLeftButtonDown  )
			{
				if ( !fIgnoreLeftUp )
				{

					// FIRST CHECK FOR ANYTIME ( NON-INTERVAL ) CLICKS
					switch( gCurrentUIMode )
					{
						case ADJUST_STANCE_MODE:

							// If button has come up, change to mocve mode
							*puiNewEvent = PADJ_ADJUST_STANCE;
							break;
					}

					// CHECK IF WE CLICKED-HELD
					if ( COUNTERDONE( LMOUSECLICK_DELAY_COUNTER ) )
					{
						// LEFT CLICK-HOLD EVENT
						// Switch on UI mode
						switch( gCurrentUIMode )
						{
						case CONFIRM_ACTION_MODE:
							{
								SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
								if (pSoldier != NULL)
								{
									if ( pSoldier->bDoBurst )
									{
										pSoldier->sEndGridNo = usMapPos;

										gfBeginBurstSpreadTracking = FALSE;

										if ( pSoldier->sEndGridNo != pSoldier->sStartGridNo )
										{
											pSoldier->fDoSpread = TRUE;

											PickBurstLocations( pSoldier );

												*puiNewEvent = CA_MERC_SHOOT;
										}
										else
										{
											pSoldier->fDoSpread = FALSE;
										}

										fClickHoldIntercepted = TRUE;

									}
								}
								break;
							}
						}

					}

					{
						//LEFT CLICK NORMAL EVENT
						// Switch on UI mode
						if ( !fClickHoldIntercepted )
						{
							if ( giUIMessageOverlay != -1 )
							{
								EndUIMessage( );
							}
							else
							{
								if ( !HandleCheckForExitArrowsInput( TRUE ) )
								{
									if ( gpItemPointer != NULL )
									{
										if ( HandleItemPointerClick( usMapPos ) )
										{
											// getout of mode
											EndItemPointer( );

											*puiNewEvent = A_CHANGE_TO_MOVE;
										}
									}
									else
									{
										// Check for wiating for keyboard advance
										if ( gfUIWaitingForUserSpeechAdvance && fCanCheckForSpeechAdvance)
										{
											// We have a key, advance!
											DialogueAdvanceSpeech( );
										}
										else
										{
											switch( gCurrentUIMode )
											{
												case MENU_MODE:

													// If we get a hit here and we're in menu mode, quit the menu mode
													EndMenuEvent( guiCurrentEvent );
													break;

												case IDLE_MODE:

													// First check if we clicked on a guy, if so, make selected if it's ours
													if (gUIFullTarget != NULL &&
															guiUIFullTargetFlags & OWNED_MERC &&
															!(guiUIFullTargetFlags & UNCONSCIOUS_MERC))
													{
														*puiNewEvent = I_SELECT_MERC;
													}
													break;

												case MOVE_MODE:

														// Check if we should activate an interactive tile!
													// Select guy
													if ( ( guiUIFullTargetFlags & OWNED_MERC ) && !( guiUIFullTargetFlags & UNCONSCIOUS_MERC ) )
													{
														const SOLDIERTYPE* const tgt = gUIFullTarget;
														if (tgt != NULL &&
																gpItemPointer == NULL &&
																!(tgt->uiStatusFlags & SOLDIER_VEHICLE) &&
																tgt->bAssignment >= ON_DUTY)
														{
															PopupAssignmentMenuInTactical();
														}
                          }
													break;

												case CONFIRM_MOVE_MODE:

														*puiNewEvent = C_MOVE_MERC;
														break;

												case HANDCURSOR_MODE:

													HandleHandCursorClick( usMapPos, puiNewEvent );
													break;

												case JUMPOVER_MODE:
												{
													const SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
													if (pSoldier != NULL)
													{
														if ( EnoughPoints( pSoldier, gsCurrentActionPoints, 0, TRUE ) )
														{
															*puiNewEvent = JP_JUMP;
														}
													}
													break;
												}

												case ACTION_MODE:

													if ( InUIPlanMode( ) )
													{
														AddUIPlan( usMapPos, UIPLAN_ACTION_FIRE );
													}
													else
													{
														SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
														if (pSoldier != NULL)
														{
															if ( !HandleUIReloading( pSoldier ) )
															{
																// ATE: Reset refine aim..
																pSoldier->bShownAimTime = 0;

																if ( gsCurrentActionPoints == 0 )
																{
																	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
																}
																// Determine if we have enough action points!
																else if ( UIMouseOnValidAttackLocation( pSoldier ) && SelectedMercCanAffordAttack( ) )
																{
																	*puiNewEvent = A_CHANGE_TO_CONFIM_ACTION;
																	pSoldier->sStartGridNo = usMapPos;
																}
															}

														}
													}
													break;

												case CONFIRM_ACTION_MODE:

													*puiNewEvent = CA_MERC_SHOOT;
													break;

												case LOOKCURSOR_MODE:
													// If we cannot actually do anything, return to movement mode
													*puiNewEvent = LC_LOOK;
													break;

												case TALKCURSOR_MODE:

													if ( HandleTalkInit( ) )
													{
														*puiNewEvent = TA_TALKINGMENU;
													}
													break;

												case GETTINGITEM_MODE:

													// Remove menu!
													// RemoveItemPickupMenu( );
													break;

												case TALKINGMENU_MODE:

													//HandleTalkingMenuEscape( TRUE );
													break;

												case EXITSECTORMENU_MODE:

													RemoveSectorExitMenu( FALSE );
													break;

												case OPENDOOR_MENU_MODE:

													CancelOpenDoorMenu( );
													HandleOpenDoorMenu( );
													*puiNewEvent = A_CHANGE_TO_MOVE;
													break;

												}
										}
									}
								}
							}
						}
					}
				}

				// Reset flag
				fLeftButtonDown = FALSE;
				fIgnoreLeftUp   = FALSE;
				fClickHoldIntercepted = FALSE;
				fCanCheckForSpeechAdvance = FALSE;
				gfFirstCycleMovementStarted = FALSE;

				// Reset counter
				RESETCOUNTER( LMOUSECLICK_DELAY_COUNTER );

			}

		}

	}
	else
	{
		// Set mouse down to false
		//fLeftButtonDown = FALSE;

		// OK, handle special cases like if we are dragging and holding for a burst spread and
		// release mouse over another mouse region
		if ( gfBeginBurstSpreadTracking )
		{
			SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
			if (pSoldier != NULL) pSoldier->fDoSpread = FALSE;
			gfBeginBurstSpreadTracking = FALSE;
		}
	}

}


static void QueryTBRightButton(UINT32* puiNewEvent)
{
	static BOOLEAN	fClickHoldIntercepted = FALSE;
	static BOOLEAN	fClickIntercepted = FALSE;
	UINT16				usMapPos;
	BOOLEAN				fDone = FALSE;
	if (!GetMouseMapPos( &usMapPos ) )
	{
		return;
	}

  if ( gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA )
	{

		// RIGHT MOUSE BUTTON
		if ( gViewportRegion.ButtonState & MSYS_RIGHT_BUTTON )
		{
			if ( !fRightButtonDown )
			{
				fRightButtonDown = TRUE;
				RESETCOUNTER( RMOUSECLICK_DELAY_COUNTER );
			}

			// CHECK COMBINATIONS
			if ( fLeftButtonDown )
			{
			}
			else
			{
				if ( gpItemPointer == NULL )
				{
					// IF HERE, DO A CLICK-HOLD IF IN INTERVAL
					if ( COUNTERDONE( RMOUSECLICK_DELAY_COUNTER ) && !fClickHoldIntercepted )
					{
						// Switch on UI mode
						switch( gCurrentUIMode )
						{
							case IDLE_MODE:
							case ACTION_MODE:
							case HANDCURSOR_MODE:
							case LOOKCURSOR_MODE:
							case TALKCURSOR_MODE:
							case MOVE_MODE:

								// Check if we're on terrain
								//if (gUIFullTarget == NULL)
								//{
									// ATE:
									fDone = FALSE;

									if ( ( guiUIFullTargetFlags & OWNED_MERC ) && !( guiUIFullTargetFlags & UNCONSCIOUS_MERC ) )
									{
										const SOLDIERTYPE* const tgt = gUIFullTarget;
										if (tgt != NULL && gpItemPointer == NULL && !(tgt->uiStatusFlags & SOLDIER_VEHICLE))
										{
											//if (tgt->bAssignment >= ON_DUTY)
											{
												PopupAssignmentMenuInTactical();
    										fClickHoldIntercepted = TRUE;
                      }
                    }
                  }

									if( fDone == TRUE )
									{
										break;
									}

									if ( gusSelectedSoldier != NOBODY && !fClickHoldIntercepted )
									{
										*puiNewEvent = U_MOVEMENT_MENU;
										fClickHoldIntercepted = TRUE;
									}
								//}
								//else
								//{
									// If we are on a selected guy
								//	if ( guiUIFullTargetFlags & SELECTED_MERC && !( guiUIFullTargetFlags & UNCONSCIOUS_MERC ) )
								//	{
										//*puiNewEvent = U_POSITION_MENU;
										//fClickHoldIntercepted = TRUE;
								//	}
							//		else if ( guiUIFullTargetFlags & OWNED_MERC )
							//		{
										// If we are on a non-selected guy selected guy
							//		}

								//}
								break;
						}
					}
				}

			}

		}
		else
		{
			if ( fRightButtonDown )
			{

				if ( fLeftButtonDown )
				{
					 fIgnoreLeftUp = TRUE;

					 if ( gpItemPointer == NULL )
					 {
							// ATE:
							if ( gusSelectedSoldier != NOBODY )
							{
								switch( gCurrentUIMode )
								{
									case CONFIRM_MOVE_MODE:
									case MOVE_MODE:

									 if ( gfUICanBeginAllMoveCycle )
									 {
										 *puiNewEvent = M_CYCLE_MOVE_ALL;
									 }
									 else
									 {
										 if ( !gfFirstCycleMovementStarted )
										 {
											 gfFirstCycleMovementStarted = TRUE;

											 // OK, set this guy's movement mode to crawling fo rthat we will start cycling in run.....
												SOLDIERTYPE* const sel = GetSelectedMan();
												if (sel->usUIMovementMode != RUNNING)
											 {
												 // ATE: UNLESS WE ARE IN RUNNING MODE ALREADY
													sel->usUIMovementMode = CRAWLING;
											 }
										 }

										 // Give event to cycle movement
										 *puiNewEvent = M_CYCLE_MOVEMENT;
										 break;
									 }
								}

								// ATE: Added cancel of burst mode....
								if ( gfBeginBurstSpreadTracking )
								{
									gfBeginBurstSpreadTracking = FALSE;
									gfRTClickLeftHoldIntercepted = TRUE;
									GetSelectedMan()->fDoSpread = FALSE;
									fClickHoldIntercepted = TRUE;
									*puiNewEvent = A_END_ACTION;
									gCurrentUIMode = MOVE_MODE;
								}
							}
					 }
				}
				else
				{
					if ( !fClickHoldIntercepted && !fClickIntercepted )
					{
					 if ( gpItemPointer == NULL )
					 {
							// ATE:
							if ( gusSelectedSoldier != NOBODY )
							{
								// Switch on UI mode
								switch( gCurrentUIMode )
								{
									case IDLE_MODE:

										break;

									case MOVE_MODE:

										// We have here a change to action mode
										*puiNewEvent = M_CHANGE_TO_ACTION;
										fClickIntercepted = TRUE;
										break;

									case ACTION_MODE:

										// We have here a change to action mode
										*puiNewEvent = A_CHANGE_TO_MOVE;
										fClickIntercepted = TRUE;
										break;

									case CONFIRM_MOVE_MODE:

										*puiNewEvent = A_CHANGE_TO_MOVE;
										fClickIntercepted = TRUE;
										break;

									case HANDCURSOR_MODE:
										// If we cannot actually do anything, return to movement mode
										*puiNewEvent = A_CHANGE_TO_MOVE;
										break;

									case LOOKCURSOR_MODE:
									case TALKCURSOR_MODE:

										// If we cannot actually do anything, return to movement mode
										*puiNewEvent = A_CHANGE_TO_MOVE;
										break;

									case CONFIRM_ACTION_MODE:
									{
										SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
										if (pSoldier != NULL)
											{
												HandleRightClickAdjustCursor( pSoldier, usMapPos );
											}
											fClickIntercepted = TRUE;
											break;
									}

									case MENU_MODE:

										// If we get a hit here and we're in menu mode, quit the menu mode
										EndMenuEvent( guiCurrentEvent );
										fClickIntercepted = TRUE;
										break;

								}
							}
					 }
					 else
					 {
							if (gUIFullTarget != NULL)
							{
								gfItemPointerDifferentThanDefault = !gfItemPointerDifferentThanDefault;
							}
					 }
					}
				}
			}
			// Reset flag
			fRightButtonDown = FALSE;
			fClickHoldIntercepted = FALSE;
			fClickIntercepted = FALSE;

			// Reset counter
			RESETCOUNTER( RMOUSECLICK_DELAY_COUNTER );
		}
	}
}


void GetTBMousePositionInput( UINT32 *puiNewEvent )
{
	UINT16						usMapPos;
	static UINT16			usOldMapPos = 0;
	BOOLEAN						bHandleCode;
	static BOOLEAN		fOnValidGuy = FALSE;
	static UINT32			uiMoveTargetSoldierId = NO_SOLDIER;


	if (!GetMouseMapPos( &usMapPos ) )
	{
		return;
	}

  if ( gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		// Check if we have an item in our hands...
		if ( gpItemPointer != NULL )
		{
			 *puiNewEvent = A_ON_TERRAIN;
			 return;
		}

		// Switch on modes
		switch( gCurrentUIMode )
		{
			case LOCKUI_MODE:
				*puiNewEvent = LU_ON_TERRAIN;
				break;

			case LOCKOURTURN_UI_MODE:
				*puiNewEvent = LA_ON_TERRAIN;
				break;

			case IDLE_MODE:
				*puiNewEvent = I_ON_TERRAIN;
				break;

			case ENEMYS_TURN_MODE:
				*puiNewEvent = ET_ON_TERRAIN;
				break;

			case LOOKCURSOR_MODE:
				*puiNewEvent = LC_ON_TERRAIN;
				break;

			case TALKCURSOR_MODE:
				if (uiMoveTargetSoldierId != NOBODY &&
						(gUIFullTarget == NULL || gUIFullTarget->ubID != uiMoveTargetSoldierId))
				{
					*puiNewEvent = A_CHANGE_TO_MOVE;
					return;
				}
			 *puiNewEvent = T_ON_TERRAIN;
				break;

			case MOVE_MODE:
			{
				uiMoveTargetSoldierId = NO_SOLDIER;

				// Check for being on terrain
				const SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
				if (pSoldier != NULL)
				{
					 if ( IsValidJumpLocation( pSoldier, usMapPos, TRUE ) )
					 {
							gsJumpOverGridNo = usMapPos;
							*puiNewEvent = JP_ON_TERRAIN;
							return;
					 }
					 else
					 {
						const SOLDIERTYPE* const tgt = gUIFullTarget;
						if (tgt != NULL)
						 {
							 // ATE: Don't do this automatically for enemies......
							if (tgt->bTeam != ENEMY_TEAM)
							 {
									uiMoveTargetSoldierId = tgt->ubID;
									if (IsValidTalkableNPC(tgt, FALSE, FALSE, FALSE) && !_KeyDown(SHIFT) && !AM_AN_EPC(pSoldier) && !ValidQuickExchangePosition())
									{
										*puiNewEvent = T_CHANGE_TO_TALKING;
										return;
									}
							 }
						 }
					 }
				}
				*puiNewEvent = M_ON_TERRAIN;
				break;
			}

			case ACTION_MODE:
			{
				// First check if we are on a guy, if so, make selected if it's ours
				// Check if the guy is visible
				guiUITargetSoldierId = NOBODY;

				fOnValidGuy = FALSE;

				const SOLDIERTYPE* const tgt = gUIFullTarget;
				if (tgt != NULL)
				{
					if (IsValidTargetMerc(tgt))
					{
						guiUITargetSoldierId = gUIFullTarget->ubID;

						if (tgt->bTeam != gbPlayerNum)
						{
							fOnValidGuy = TRUE;
						}
						else
						{
							if (gUIActionModeChangeDueToMouseOver)
							{
								*puiNewEvent = A_CHANGE_TO_MOVE;
								return;
							}
						}
					}
				}
				else
				{
					if ( gUIActionModeChangeDueToMouseOver )
					{
						*puiNewEvent = A_CHANGE_TO_MOVE;
						return;
					}
				}
				*puiNewEvent = A_ON_TERRAIN;
				break;
			}

			case GETTINGITEM_MODE:

				break;

			case TALKINGMENU_MODE:

				if ( HandleTalkingMenu( ) )
				{
					*puiNewEvent = A_CHANGE_TO_MOVE;
				}
				break;

			case EXITSECTORMENU_MODE:

				if ( HandleSectorExitMenu( ) )
				{
					*puiNewEvent = A_CHANGE_TO_MOVE;
				}
				break;

			case OPENDOOR_MENU_MODE:

				if ( ( bHandleCode = HandleOpenDoorMenu( ) ) )
				{
					// OK, IF we are not canceling, set ui back!
					if ( bHandleCode == 2 )
					{
						*puiNewEvent = A_CHANGE_TO_MOVE;
					}
					else
					{
					}
				}
				break;

			case JUMPOVER_MODE:

				// ATE: Make sure!
				if ( gsJumpOverGridNo != usMapPos )
				{
					*puiNewEvent = A_CHANGE_TO_MOVE;
				}
				else
				{
					*puiNewEvent = JP_ON_TERRAIN;
				}
				break;

			case CONFIRM_MOVE_MODE:

				if ( usMapPos != usOldMapPos )
				{
					// Switch event out of confirm mode
					*puiNewEvent = A_CHANGE_TO_MOVE;

					// Set off ALL move....
					gfUIAllMoveOn = FALSE;

					// ERASE PATH
					ErasePath( TRUE );
				}
				break;

			case CONFIRM_ACTION_MODE:
			{
				// DONOT CANCEL IF BURST
				SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
				if (pSoldier != NULL)
				{
					if ( pSoldier->bDoBurst )
					{
						pSoldier->sEndGridNo = usMapPos;

						if ( pSoldier->sEndGridNo != pSoldier->sStartGridNo && fLeftButtonDown )
						{
							pSoldier->fDoSpread = TRUE;
							gfBeginBurstSpreadTracking = TRUE;
						}

						if ( pSoldier->fDoSpread )
						{
							// Accumulate gridno
							AccumulateBurstLocation( usMapPos );

							*puiNewEvent = CA_ON_TERRAIN;
							break;
						}
					}
				}

				// First check if we are on a guy, if so, make selected if it's ours
				if (gUIFullTarget != NULL)
				{
					if (guiUITargetSoldierId != gUIFullTarget->ubID)
					 {
							// Switch event out of confirm mode
							*puiNewEvent = CA_END_CONFIRM_ACTION;
					 }
					 else
					 {
							*puiNewEvent = CA_ON_TERRAIN;
					 }
				}
				else
				{
					// OK, if we were on a guy, and now we are off, go back!
					if ( fOnValidGuy )
					{
						// Switch event out of confirm mode
						*puiNewEvent = CA_END_CONFIRM_ACTION;
					}
					else
					{
						if ( ConfirmActionCancel( usMapPos, usOldMapPos ) )
						{
							// Switch event out of confirm mode
							*puiNewEvent = CA_END_CONFIRM_ACTION;
						}
						else
						{
							*puiNewEvent = CA_ON_TERRAIN;
						}
					}
				}
				break;
			}

			case HANDCURSOR_MODE:

				*puiNewEvent = HC_ON_TERRAIN;
				break;

		}

		usOldMapPos = usMapPos;

	}
}


void GetPolledKeyboardInput( UINT32 *puiNewEvent )
{
	static BOOLEAN	fShifted = FALSE;
	static BOOLEAN	fShifted2 = FALSE;
	static BOOLEAN	fCtrlDown = FALSE;
	static BOOLEAN	fAltDown = FALSE;
	static BOOLEAN	fDeleteDown=FALSE;
	static BOOLEAN	fEndDown=FALSE;

	// CHECK FOR POLLED KEYS!!
	// CHECK FOR CTRL
	switch( gCurrentUIMode )
	{
			case DONT_CHANGEMODE:
			case CONFIRM_MOVE_MODE:
			case CONFIRM_ACTION_MODE:
			case LOOKCURSOR_MODE:
			case TALKCURSOR_MODE:
			case IDLE_MODE:
			case MOVE_MODE:
			case ACTION_MODE:
			case HANDCURSOR_MODE:

				if ( _KeyDown( CTRL )   )
				{
					if ( fCtrlDown == FALSE )
					{
						ErasePath( TRUE );
						gfPlotNewMovement = TRUE;
					}
					fCtrlDown = TRUE;
					*puiNewEvent = HC_ON_TERRAIN;

				}
				if ( !(_KeyDown( CTRL ) ) && fCtrlDown )
				{
					fCtrlDown = FALSE;
					*puiNewEvent = M_ON_TERRAIN;
					gfPlotNewMovement = TRUE;
				}
				break;
	}

	// CHECK FOR ALT
	switch( gCurrentUIMode )
	{
			case MOVE_MODE:

				if ( _KeyDown( ALT )   )
				{
					if ( fAltDown == FALSE )
					{
						// Get currently selected guy and change reverse....
						if ( gusSelectedSoldier != NOBODY )
						{
							gUIUseReverse = TRUE;
							ErasePath( TRUE );
							gfPlotNewMovement = TRUE;
						}
					}
					fAltDown = TRUE;

				}
				if ( !(_KeyDown( ALT ) ) && fAltDown )
				{
					if ( gusSelectedSoldier != NOBODY )
					{
						gUIUseReverse = FALSE;
						ErasePath( TRUE );
						gfPlotNewMovement = TRUE;
					}

					fAltDown = FALSE;
				}
				break;
	}

	// Check realtime input!
	if ( ( ( gTacticalStatus.uiFlags & REALTIME ) || !( gTacticalStatus.uiFlags & INCOMBAT ) ) )
	{
		//if (_KeyDown(SDLK_CAPSLOCK)) //&& !fShifted)
		//{
		//	fShifted = TRUE;
		//	if ( gCurrentUIMode != ACTION_MODE && gCurrentUIMode != CONFIRM_ACTION_MODE )
		//	{
		//		*puiNewEvent = CA_ON_TERRAIN;
		//	}
		//}
		//if (!(_KeyDown(SDLK_CAPSLOCK)) && fShifted)
		//{
		//	fShifted = FALSE;
		//	{
		//		*puiNewEvent = M_ON_TERRAIN;
		//	}
		//}


		if ( _KeyDown( SHIFT )  ) //&& !fShifted )
		{
			fShifted2 = TRUE;
			if ( gCurrentUIMode != MOVE_MODE && gCurrentUIMode != CONFIRM_MOVE_MODE )
			{
			//puiNewEvent = M_ON_TERRAIN;
			}
		}
		if ( !(_KeyDown( SHIFT ) ) && fShifted2 )
		{
			fShifted2 = FALSE;
			if ( gCurrentUIMode != ACTION_MODE && gCurrentUIMode != CONFIRM_ACTION_MODE )
			{
			//	*puiNewEvent = A_ON_TERRAIN;
			}
		}

	}

	if (_KeyDown(SDLK_DELETE))
	{
		DisplayCoverOfSelectedGridNo( );

		fDeleteDown = TRUE;
	}

	if (!_KeyDown(SDLK_DELETE) && fDeleteDown)
	{
		RemoveCoverOfSelectedGridNo();

		fDeleteDown = FALSE;
	}

	if (_KeyDown(SDLK_END))
	{
		DisplayGridNoVisibleToSoldierGrid( );

		fEndDown = TRUE;
	}

	if (!_KeyDown(SDLK_END) && fEndDown)
	{
		RemoveVisibleGridNoAtSelectedGridNo();

		fEndDown = FALSE;
	}
}


static void ChangeCurrentSquad(INT32 iSquad);
static void ChangeSoldiersBodyType(UINT8 ubBodyType, BOOLEAN fCreateNewPalette);
static void CreateNextCivType(void);
static void CreatePlayerControlledMonster(void);
static void CreateRandomItem(void);
static void CycleSelectedMercsItem(void);
static void EscapeUILock(void);
static void GrenadeTest1(void);
static void GrenadeTest2(void);
static void HandleItemMenuKeys(InputAtom* pInputEvent, UINT32* puiNewEvent);
static void HandleMenuKeys(InputAtom* pInputEvent, UINT32* puiNewEvent);
static void HandleOpenDoorMenuKeys(InputAtom* pInputEvent, UINT32* puiNewEvent);
static void HandleSectorExitMenuKeys(InputAtom* pInputEvent, UINT32* puiNewEvent);
static void HandleSelectMercSlot(UINT8 ubPanelSlot, INT8 bCode);
static void HandleStealthChangeFromUIKeys(void);
static void HandleTalkingMenuKeys(InputAtom* pInputEvent, UINT32* puiNewEvent);
static void ObliterateSector(void);
static void RefreshSoldier(void);
static void SetBurstMode(void);
static void TeleportSelectedSoldier(void);
static void TestCapture(void);
static void TestMeanWhile(INT32 iID);
static void ToggleCliffDebug(void);
static void ToggleTreeTops(void);
static void ToggleViewAllItems(void);
static void ToggleViewAllMercs(void);
static void ToggleWireFrame(void);
static void ToggleZBuffer(void);

#ifdef JA2BETAVERSION
static void ToggleMapEdgepoints(void);
#endif


void GetKeyboardInput( UINT32 *puiNewEvent )
{
  InputAtom					InputEvent;
	BOOLEAN						fKeyTaken = FALSE;
	POINT  MousePos;
	//SOLDIERTYPE				*pSoldier;
	UINT16						usMapPos;
	BOOLEAN						fGoodCheatLevelKey = FALSE;

	GetCursorPos(&MousePos);

	GetMouseMapPos( &usMapPos );

  while (DequeueEvent(&InputEvent) == TRUE)
  {
		MouseSystemHook(InputEvent.usEvent, MousePos.x, MousePos.y);

		// handle for fast help text for interface stuff
		if( IsTheInterfaceFastHelpTextActive() )
		{
			if( InputEvent.usEvent == KEY_UP )
			{
				ShutDownUserDefineHelpTextRegions( );
			}

			continue;
		}

		// Check for waiting for keyboard advance
		if ( gfUIWaitingForUserSpeechAdvance && InputEvent.usEvent == KEY_UP )
		{
			// We have a key, advance!
			DialogueAdvanceSpeech( );

			// Ignore anything else
			continue;
		}

		// ATE: if game paused because fo player, unpasue with any key
		if ( gfPauseDueToPlayerGamePause && InputEvent.usEvent == KEY_UP )
		{
			HandlePlayerPauseUnPauseOfGame( );

			continue;
		}

		if ( (InputEvent.usEvent == KEY_DOWN ) )
		{
			if ( giUIMessageOverlay != -1 )
			{
				EndUIMessage( );
				continue;
			}

			// End auto bandage if we want....
			if ( gTacticalStatus.fAutoBandageMode )
			{
				AutoBandage( FALSE );
				*puiNewEvent = LU_ENDUILOCK;
			}
		}

		if ( gUIKeyboardHook != NULL )
		{
			fKeyTaken = gUIKeyboardHook( &InputEvent );
		}
		if ( fKeyTaken )
		{
			continue;
		}

		/*
		if( (InputEvent.usEvent == KEY_DOWN )&& ( InputEvent.usParam == ) )
		{
			HandlePlayerPauseUnPauseOfGame( );
		}
		*/

    if (InputEvent.usEvent == KEY_UP && InputEvent.usParam == SDLK_PAUSE && !(gTacticalStatus.uiFlags & ENGAGED_IN_CONV))
    {
			// Pause game!
			HandlePlayerPauseUnPauseOfGame( );
    }

		// FIRST DO KEYS THAT ARE USED EVERYWHERE!
    if ((InputEvent.usEvent == KEY_DOWN )&& ( InputEvent.usParam == 'x') && ( InputEvent.usKeyState & ALT_DOWN ))
    {
			HandleShortCutExitState( );
    }

		if (InputEvent.usEvent == KEY_UP && InputEvent.usParam == SDLK_ESCAPE)
		{
			if ( AreInMeanwhile() && gCurrentMeanwhileDef.ubMeanwhileID != INTERROGATION )
			{
				DeleteTalkingMenu();
				EndMeanwhile();
			}
		}

		// Break of out IN CONV...
		if ( CHEATER_CHEAT_LEVEL( ) )
		{
			if (InputEvent.usEvent == KEY_DOWN && InputEvent.usParam == SDLK_RETURN && InputEvent.usKeyState & ALT_DOWN)
			{
				if ( gTacticalStatus.uiFlags & ENGAGED_IN_CONV )
				{
					gTacticalStatus.uiFlags &= (~ENGAGED_IN_CONV);
					giNPCReferenceCount = 0;
				}
			}
		}

		if ( gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT) )
		{
			{
				if ( gTacticalStatus.ubCurrentTeam != gbPlayerNum )
				{
					if ( CHEATER_CHEAT_LEVEL( ) )
					{
						if (InputEvent.usEvent == KEY_DOWN && InputEvent.usParam == SDLK_RETURN && InputEvent.usKeyState & ALT_DOWN)
						{
								// ESCAPE ENEMY'S TURN
								EndAIDeadlock();

								// Decrease global busy  counter...
								gTacticalStatus.ubAttackBusyCount = 0;

								guiPendingOverrideEvent = LU_ENDUILOCK;
								UIHandleLUIEndLock( NULL );

						}
						if (InputEvent.usEvent == KEY_DOWN && InputEvent.usParam == SDLK_RETURN && InputEvent.usKeyState & CTRL_DOWN)
						{
							EscapeUILock( );
						}
					}
				}
				else
				{
					if (CHEATER_CHEAT_LEVEL() && InputEvent.usEvent == KEY_DOWN && InputEvent.usParam == SDLK_RETURN && InputEvent.usKeyState & CTRL_DOWN)
					{
							//UNLOCK UI
							EscapeUILock( );
					}
					else if (InputEvent.usEvent == KEY_DOWN && InputEvent.usParam == SDLK_RETURN)
					{
							// Cycle through enemys
							CycleThroughKnownEnemies( );
					}
				}
			}
		}

		if ( gfInTalkPanel )
		{
			HandleTalkingMenuKeys( &InputEvent, puiNewEvent );
		}

		// Do some checks based on what mode we are in
		switch( gCurrentUIMode )
		{
			case EXITSECTORMENU_MODE:

				HandleSectorExitMenuKeys( &InputEvent, puiNewEvent );
				continue;

			case GETTINGITEM_MODE:

				HandleItemMenuKeys( &InputEvent, puiNewEvent );
				continue;

			case MENU_MODE:

				HandleMenuKeys( &InputEvent, puiNewEvent );
				continue;

			case OPENDOOR_MENU_MODE:

				HandleOpenDoorMenuKeys( &InputEvent, puiNewEvent );
				continue;
		}


		// CHECK ESC KEYS HERE....
    if (InputEvent.usEvent == KEY_DOWN && InputEvent.usParam == SDLK_ESCAPE)
    {
			//EscapeUILock( );
#ifdef JA2TESTVERSION
			if ( InAirRaid( ) )
			{
				EndAirRaid( );
			}
#endif

  		// Cancel out of spread burst...
			gfBeginBurstSpreadTracking = FALSE;
			gfRTClickLeftHoldIntercepted = TRUE;
			if (gusSelectedSoldier != NO_SOLDIER) GetSelectedMan()->fDoSpread = FALSE;

			// Befone anything, delete popup box!
			EndUIMessage( );

			// CANCEL FROM PLANNING MODE!
			if ( InUIPlanMode( ) )
			{
				EndUIPlan( );
			}

			if ( InItemDescriptionBox( ) )
			{
				DeleteItemDescriptionBox( );
			}
			else if( InKeyRingPopup( ) )
			{
				DeleteKeyRingPopup( );
			}

			if ( gCurrentUIMode == MENU_MODE )
			{
				// If we get a hit here and we're in menu mode, quit the menu mode
				EndMenuEvent( guiCurrentEvent );
			}

			if ( gCurrentUIMode == HANDCURSOR_MODE )
			{
				*puiNewEvent = A_CHANGE_TO_MOVE;
			}

      if ( !( gTacticalStatus.uiFlags & ENGAGED_IN_CONV ) )
		  {
			  if ( gusSelectedSoldier != NO_SOLDIER )
			  {
				  // If soldier is not stationary, stop
					StopSoldier(GetSelectedMan());
				  *puiNewEvent = A_CHANGE_TO_MOVE;
			  }
			  // ATE: OK, stop any mercs who are moving by selection method....
			  StopRubberBandedMercFromMoving( );
      }

    }

		// CHECK ESC KEYS HERE....
    if (InputEvent.usEvent == KEY_DOWN && InputEvent.usParam == SDLK_BACKSPACE)
    {
      StopAnyCurrentlyTalkingSpeech( );
    }


		// IF UI HAS LOCKED, ONLY ALLOW EXIT!
		if ( gfDisableRegionActive || gfUserTurnRegionActive )
		{
			continue;
		}

		// Check all those we want if enemy's turn
    if ((InputEvent.usEvent == KEY_UP )&& ( InputEvent.usParam == 'q') )
    {
			if ( InputEvent.usKeyState & ALT_DOWN )
			{
				if ( CHEATER_CHEAT_LEVEL( ) )
				{
					static BOOLEAN fShowRoofs = TRUE;
					INT32 x;
					UINT16 usType;

					// Toggle removal of roofs...
					fShowRoofs = !fShowRoofs;

					for ( x = 0; x < WORLD_MAX; x++ )
					{
						for ( usType = FIRSTROOF; usType <= LASTSLANTROOF; usType++ )
						{
							HideStructOfGivenType( x, usType, (BOOLEAN)(!fShowRoofs) );
						}
					}
					InvalidateWorldRedundency();
					SetRenderFlags( RENDER_FLAG_FULL );
				}

			}
			else if ( InputEvent.usKeyState & CTRL_DOWN )
      {
#ifdef JA2BETAVERSION
				if ( CHEATER_CHEAT_LEVEL( ) )
				{
					LeaveTacticalScreen( ANIEDIT_SCREEN );
				}
#endif
      }
      else
			{
				if ( INFORMATION_CHEAT_LEVEL( ) )
				{
					*puiNewEvent = I_SOLDIERDEBUG;
				}
			}
		}

#ifdef JA2TESTVERSION
    if ((InputEvent.usEvent == KEY_DOWN )&& ( InputEvent.usParam == '0') && ( InputEvent.usKeyState & ALT_DOWN ))
    {
			INT32 i = 0;
			INT16	sGridNo;
			INT32 iTime = GetJA2Clock( );
			UINT8	ubLevel;

			for ( i = 0; i < 1000; i++ )
			{
				const SOLDIERTYPE* const sel = GetSelectedMan();
				CalculateLaunchItemChanceToGetThrough(sel, &sel->inv[HANDPOS], usMapPos, 0, 0, &sGridNo, TRUE, &ubLevel, TRUE);
			}

			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Physics 100 times: %d", ( GetJA2Clock( ) - iTime )  );

		}
#endif

		if( InputEvent.usEvent == KEY_DOWN )
		{
			BOOLEAN fAlt, fCtrl, fShift;
			fAlt = InputEvent.usKeyState & ALT_DOWN ? TRUE : FALSE;
			fCtrl = InputEvent.usKeyState & CTRL_DOWN ? TRUE : FALSE;
			fShift = InputEvent.usKeyState & SHIFT_DOWN ? TRUE : FALSE;
			switch( InputEvent.usParam )
			{
				case SDLK_SPACE:
					// nothing in hand and either not in SM panel, or the matching button is enabled if we are in SM panel
					if ( !( gTacticalStatus.uiFlags & ENGAGED_IN_CONV )  &&
							 ( ( gsCurInterfacePanel != SM_PANEL ) || ( ButtonList[ iSMPanelButtons[ NEXTMERC_BUTTON ] ]->uiFlags & BUTTON_ENABLED ) ) )
					{
						if ( !InKeyRingPopup( ) )
						{
							if ( _KeyDown( SHIFT ) )
							{
								SOLDIERTYPE *pNewSoldier;
								INT32				iCurrentSquad;

								if ( gusSelectedSoldier != NO_SOLDIER )
								{
									// only allow if nothing in hand and if in SM panel, the Change Squad button must be enabled
									if (
											 ( ( gsCurInterfacePanel != TEAM_PANEL ) || ( ButtonList[ iTEAMPanelButtons[ CHANGE_SQUAD_BUTTON ] ]->uiFlags & BUTTON_ENABLED ) ) )
									{
										//Select next squad
										iCurrentSquad = CurrentSquad( );

										pNewSoldier = FindNextActiveSquad(GetSelectedMan());

										if ( pNewSoldier->bAssignment != iCurrentSquad )
										{
  										HandleLocateSelectMerc( pNewSoldier->ubID, LOCATEANDSELECT_MERC );

											ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_SQUAD_ACTIVE ], ( CurrentSquad( ) + 1 ) );

							        // Center to guy....
											LocateSoldier(GetSelectedMan(), SETLOCATOR);
										}
									}
								}
							}
							else
							{
								if ( gusSelectedSoldier != NO_SOLDIER )
								{ //Select next merc
									const UINT8 bID = FindNextMercInTeamPanel(GetSelectedMan());
									HandleLocateSelectMerc( bID, LOCATEANDSELECT_MERC );

							    // Center to guy....
									LocateSoldier(GetSelectedMan(), SETLOCATOR);
								}
							}

  						*puiNewEvent = M_ON_TERRAIN;
						}
					}
					break;

				case SDLK_TAB:
					// nothing in hand and either not in SM panel, or the matching button is enabled if we are in SM panel
					if ( ( gpItemPointer == NULL ) &&
							 ( ( gsCurInterfacePanel != SM_PANEL ) || ( ButtonList[ iSMPanelButtons[ UPDOWN_BUTTON ] ]->uiFlags & BUTTON_ENABLED ) ) )
					{
						UIHandleChangeLevel( NULL );

						if ( gsCurInterfacePanel == SM_PANEL )
						{
							// Remember soldier's new value
							gpSMCurrentMerc->bUIInterfaceLevel = (INT8)gsInterfaceLevel;
						}
					}
					break;

				case SDLK_F1:
					if( fShift )
					{
						HandleSelectMercSlot( 0, LOCATE_MERC_ONCE );
					}
#ifdef JA2TESTVERSION
					else if( fAlt )
					{
						TestMeanWhile( 15 );
					}
					else if( fCtrl )
					{
						TestMeanWhile( 10 );
					}
#endif
					else
						HandleSelectMercSlot( 0, LOCATEANDSELECT_MERC );
					break;

				case SDLK_F2:
					if( fShift )
						HandleSelectMercSlot( 1, LOCATE_MERC_ONCE );
#ifdef JA2TESTVERSION
					else if( fAlt )
					{
						TestMeanWhile( 1 );
					}
					else if( fCtrl )
					{
						TestMeanWhile( 11 );
					}
#endif
					else
						HandleSelectMercSlot( 1, LOCATEANDSELECT_MERC );
					break;

				case SDLK_F3:
					if( fShift )
						HandleSelectMercSlot( 2, LOCATE_MERC_ONCE );
#ifdef JA2TESTVERSION
					else if( fAlt )
					{
						TestMeanWhile( 2 );
					}
					else if( fCtrl )
					{
						TestMeanWhile( 12 );
					}
#endif
					else
						HandleSelectMercSlot( 2, LOCATEANDSELECT_MERC );
					break;

				case SDLK_F4:
					if( fShift )
						HandleSelectMercSlot( 3, LOCATE_MERC_ONCE );
#ifdef JA2TESTVERSION
					else if( fAlt )
					{
						TestMeanWhile( 3 );
					}
					else if( fCtrl )
					{
						TestMeanWhile( 13 );
					}
#endif
					else
						HandleSelectMercSlot( 3, LOCATEANDSELECT_MERC );
					break;

				case SDLK_F5:
					if( fShift )
						HandleSelectMercSlot( 4, LOCATE_MERC_ONCE );
#ifdef JA2TESTVERSION
					else if( fAlt )
					{
						TestMeanWhile( 4 );
					}
					else if( fCtrl )
					{
						TestMeanWhile( 14 );
					}
#endif
					else
						HandleSelectMercSlot( 4, LOCATEANDSELECT_MERC );
					break;

				case SDLK_F6:
					if( fShift )
						HandleSelectMercSlot( 5, LOCATE_MERC_ONCE );
#ifdef JA2TESTVERSION
					else if( fAlt )
					{
						TestMeanWhile( 5 );
					}
					else if( fCtrl )
					{
						TestMeanWhile( 15 );
					}
#endif
					else
						HandleSelectMercSlot( 5, LOCATEANDSELECT_MERC );
					break;

#ifdef JA2TESTVERSION
				case SDLK_F7: if (fAlt) TestMeanWhile(16); break;
				case SDLK_F8: if (fAlt) TestMeanWhile( 7); break;

				case SDLK_F9:
					if( fCtrl )
					{
						TestMeanWhile( 8 );
					}
					else
					{
						#ifdef JA2EDITOR
							*puiNewEvent = I_ENTER_EDIT_MODE;
							gfMercResetUponEditorEntry = !fAlt;
						#endif
					}
					break;

				case SDLK_F10: if (fAlt) TestMeanWhile(9); break;
#endif

				case SDLK_F11:
					if( fAlt )
					{
#ifdef JA2TESTVERSION
						// Get selected soldier
						SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
						if (pSoldier != NULL)
						{
							if( pSoldier->ubID == 46 )
							{
								// Change guy to drunk larry
								ForceSoldierProfileID( pSoldier, 47 );
							}
							else
							{
								// Change guy to normal larry
								ForceSoldierProfileID( pSoldier, 46 );
							}

							// Dirty interface
							DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
						}
#endif
					}

					else
					{
						if( DEBUG_CHEAT_LEVEL( ) )
						{
							GetMouseMapPos( &gsQdsEnteringGridNo );
							LeaveTacticalScreen( QUEST_DEBUG_SCREEN );
						}
					}
					break;

				case SDLK_F12:
#ifdef JA2TESTVERSION
					if( fAlt )
					{
						UINT8 ubProfile = TONY;

						GetMouseMapPos( &gsQdsEnteringGridNo );
						AddShopkeeperToGridNo( ubProfile, gsQdsEnteringGridNo );
						EnterShopKeeperInterfaceScreen( ubProfile );
					}
#endif
					//clear tactical of messages
					if( fCtrl )
					{
						ClearTacticalMessageQueue( );
					}
					else if ( !fAlt )
					{
						ClearDisplayedListOfTacticalStrings( );
					}
					break;

				case '1':

					if( fAlt )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							//ChangeSoldiersBodyType( TANK_NW, TRUE );
							//SOLDIERTYPE* const sel = GetSelectedMan();
							//sel->uiStatusFlags |= SOLDIER_CREATURE;
							//EVENT_InitNewSoldierAnim(sel, CRIPPLE_BEG, 0 , TRUE);
						}
					}
					else
						ChangeCurrentSquad( 0 );
					break;

				case '2':

					if( fAlt )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							ChangeSoldiersBodyType( INFANT_MONSTER, TRUE );
						}
					}
					else
						ChangeCurrentSquad( 1 );
					break;

				case '3':

					if( fAlt )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							EVENT_InitNewSoldierAnim(GetSelectedMan(), KID_SKIPPING, 0, TRUE);

							//ChangeSoldiersBodyType( LARVAE_MONSTER, TRUE );
							//SOLDIERTYPE* const s = GetSelectedMan();
							//s->usAttackingWeapon = TANK_CANNON;
							//LocateSoldier(s, FALSE );
							//EVENT_FireSoldierWeapon(s, usMapPos);
						}
					}
					else
						ChangeCurrentSquad( 2 );

					break;

				case '4':

					if( fAlt )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							ChangeSoldiersBodyType( CRIPPLECIV, TRUE );
						}
					}
					else
						ChangeCurrentSquad( 3 );

						//ChangeSoldiersBodyType( BLOODCAT, FALSE );
					break;

				case '5':

					if( fAlt )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							ChangeSoldiersBodyType( YAM_MONSTER, TRUE );
						}
					}
					else
						ChangeCurrentSquad( 4 );
						break;

				case '6':
						ChangeCurrentSquad( 5 );
						break;

				case '7':
						ChangeCurrentSquad( 6 );
						break;

				case '8':
						ChangeCurrentSquad( 7 );
						break;

				case '9':
						ChangeCurrentSquad( 8 );
						break;

				case '0':
						ChangeCurrentSquad( 9 );
						break;

				case 'x':

					if ( !fCtrl && !fAlt )
					{
						// Exchange places...
						//Check if we have a good selected guy
						if ( gusSelectedSoldier != NOBODY )
						{
							SOLDIERTYPE* const pSoldier1 = GetSelectedMan();
							SOLDIERTYPE* const pSoldier2 = gUIFullTarget;

							if (gUIFullTarget != NULL)
							{
								// Check if both OK....
								if ( pSoldier1->bLife >= OKLIFE && pSoldier2->ubID != gusSelectedSoldier )
								{
									if ( pSoldier2->bLife >= OKLIFE )
									{
										if (CanSoldierReachGridNoInGivenTileLimit( pSoldier1, pSoldier2->sGridNo, 1, (INT8)gsInterfaceLevel ) )
										{
											// Exclude enemies....
											if ( !pSoldier2->bNeutral && (pSoldier2->bSide != gbPlayerNum ) )
											{

											}
											else
											{
												if ( CanExchangePlaces( pSoldier1, pSoldier2, TRUE ) )
												{
													// All's good!
													SwapMercPositions( pSoldier1, pSoldier2 );

													DeductPoints( pSoldier1, AP_EXCHANGE_PLACES, 0 );
													DeductPoints( pSoldier2, AP_EXCHANGE_PLACES, 0 );
												}
											}
										}
									}
								}
							}

						}
					}
					break;

				case '/':
					// Center to guy....
					if ( gusSelectedSoldier != NOBODY )
					{
						LocateSoldier(GetSelectedMan(), 10);
					}
					break;

				case 'a':

					if ( fCtrl )
					{
#ifdef GERMAN
						if ( gubCheatLevel == 3 )
						{
							gubCheatLevel++;
							fGoodCheatLevelKey = TRUE;
						}
						else if ( gubCheatLevel == 5 )
						{
							gubCheatLevel++;
							// ATE; We're done.... start cheat mode....
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_CHEAT_LEVEL_TWO ] );
							SetHistoryFact( HISTORY_CHEAT_ENABLED, 0, GetWorldTotalMin(), -1, -1 );
						}
						else
						{
							RESET_CHEAT_LEVEL();
						}
#else
						if ( gubCheatLevel == 1 )
						{
							gubCheatLevel++;
							fGoodCheatLevelKey = TRUE;
						}
						else
						{
							RESET_CHEAT_LEVEL();
						}
#endif
					}
					else
					{
						BeginAutoBandage( );
					}
					break;

				case 'j':

					if( fAlt )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							gfNextFireJam	= TRUE;
						}
					}
					else if ( fCtrl )
					{
#ifdef JA2BETAVERSION
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							ToggleNPCRecordDisplay();
						}
#endif
					}
					break;

				case 'b':

					if( fAlt )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							*puiNewEvent = I_NEW_BADMERC;
						}
					}
					else if( fCtrl )
					{
#ifndef GERMAN
						if ( gubCheatLevel == 2 )
						{
							gubCheatLevel++;
							fGoodCheatLevelKey = TRUE;
						}
						else if ( gubCheatLevel == 3 )
						{
							gubCheatLevel++;
							fGoodCheatLevelKey = TRUE;
						}
						else if ( gubCheatLevel == 5 )
						{
							gubCheatLevel++;
							fGoodCheatLevelKey = TRUE;
						}
						else
						{
							RESET_CHEAT_LEVEL( );
						}
#else
						if ( gubCheatLevel == 6 )
						{
							gubCheatLevel++;
							fGoodCheatLevelKey = TRUE;
						}
						else
						{
							RESET_CHEAT_LEVEL( );
						}
#endif
						//gGameSettings.fOptions[ TOPTION_HIDE_BULLETS ] ^= TRUE;
					}
					else
					{
						// nothing in hand and either not in SM panel, or the matching button is enabled if we are in SM panel
						if ( ( gpItemPointer == NULL ) &&
								 ( ( gsCurInterfacePanel != SM_PANEL ) || ( ButtonList[ iSMPanelButtons[ BURSTMODE_BUTTON ] ]->uiFlags & BUTTON_ENABLED ) ) )
						{
							SetBurstMode();
						}
					}
					break;
				case 'c':

					if( fAlt )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							CreateNextCivType();
						}
					}
					else if( fCtrl )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							ToggleCliffDebug();
						}
					}
					else
					{
						HandleStanceChangeFromUIKeys( ANIM_CROUCH );
					}
					break;

				case 'd':
					if( gTacticalStatus.uiFlags & TURNBASED && gTacticalStatus.uiFlags & INCOMBAT )
					{
						if( gTacticalStatus.ubCurrentTeam == gbPlayerNum )
						{
							// nothing in hand and the Done button for whichever panel we're in must be enabled
							if ( ( gpItemPointer == NULL ) && !gfDisableTacticalPanelButtons &&
									 ( ( ( gsCurInterfacePanel == SM_PANEL   ) && ( ButtonList[ iSMPanelButtons[ SM_DONE_BUTTON ] ]->uiFlags & BUTTON_ENABLED ) ) ||
										 ( ( gsCurInterfacePanel == TEAM_PANEL ) && ( ButtonList[ iTEAMPanelButtons[ TEAM_DONE_BUTTON ] ]->uiFlags & BUTTON_ENABLED ) ) ) )
							{
								if( fAlt )
								{
									INT32 cnt;
									SOLDIERTYPE *pSoldier;

									if ( CHEATER_CHEAT_LEVEL( ) )
									{
										for ( pSoldier = MercPtrs[ gbPlayerNum ], cnt = 0; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pSoldier++)
										{
											if ( pSoldier->bActive && pSoldier->bLife > 0 )
											{
												// Get APs back...
												CalcNewActionPoints( pSoldier );

												fInterfacePanelDirty = DIRTYLEVEL2;
											}
										}
									}
								}
								else //End turn only if in combat and it is the player's turn
									*puiNewEvent = I_ENDTURN;
							}
						}
					}
#ifdef JA2TESTVERSION
					else if( fCtrl )
						AdvanceToNextDay();
#endif
					break;


				case 'e':

					if( fAlt )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							ToggleViewAllMercs();
							ToggleViewAllItems();
						}
					}
#ifdef JA2BETAVERSION
					else if( fCtrl )
					{
						ToggleMapEdgepoints();
					}
#endif
					else
					{
            if ( gusSelectedSoldier != NOBODY )
            {
							SOLDIERTYPE* const sel = GetSelectedMan();
							if (sel->bOppCnt > 0)
			        {
                // Cycle....
								CycleVisibleEnemies(sel);
			        }
              else
              {
		            ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_ENEMIES_IN_SIGHT_STR ] );
              }
            }
					}
					break;

				case 'f':
					if( fCtrl )
					{
						if ( INFORMATION_CHEAT_LEVEL( ) )
						{
							//Toggle Frame Rate Display
							gbFPSDisplay = !gbFPSDisplay;
							EnableFPSOverlay(gbFPSDisplay);
							if( !gbFPSDisplay )
								SetRenderFlags( RENDER_FLAG_FULL );
						}
					}
					else if( fAlt )
					{
						if ( gGameSettings.fOptions[ TOPTION_TRACKING_MODE ] )
						{
							gGameSettings.fOptions[ TOPTION_TRACKING_MODE ] = FALSE;

							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_TACKING_MODE_OFF ] );
						}
						else
						{
							gGameSettings.fOptions[ TOPTION_TRACKING_MODE ] = TRUE;

							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_TACKING_MODE_ON ] );
						}
					}
					else
					{
						INT16 sGridNo;

						//Get the gridno the cursor is at
						GetMouseMapPos( &sGridNo );

						//if there is a selected soldier, and the cursor location is valid
						if( gusSelectedSoldier != NOBODY && sGridNo != NOWHERE )
						{
							//if the cursor is over someone
							if (gUIFullTarget != NULL) sGridNo = gUIFullTarget->sGridNo;
							DisplayRangeToTarget(GetSelectedMan(), sGridNo);
						}
					}
					break;

				case 'g':

					if( fCtrl )
					{
#ifdef GERMAN
						if ( gubCheatLevel == 1 )
						{
							gubCheatLevel++;
							fGoodCheatLevelKey = TRUE;
						}
						else
						{
							RESET_CHEAT_LEVEL();
						}
#else
						if ( gubCheatLevel == 0 )
						{
							gubCheatLevel++;
							fGoodCheatLevelKey = TRUE;
						}
						else
						{
							RESET_CHEAT_LEVEL();
						}
#endif
					}
					else if ( fAlt )
					{

						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							*puiNewEvent = I_NEW_MERC;
						}
					}
					else
					{
						HandlePlayerTogglingLightEffects( TRUE );
					}
					break;

				case SDLK_h:
					if ( fAlt )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							gfReportHitChances = !gfReportHitChances;
						}
					}
					else if( fCtrl )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							*puiNewEvent = I_TESTHIT;
						}
					}
					else
					{
						ShouldTheHelpScreenComeUp( HELP_SCREEN_TACTICAL, TRUE );
					}
					break;

				case 'i':

					if( fAlt )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							CreateRandomItem();
						}
					}
					else if( fCtrl )
					{
#ifdef GERMAN
						if ( gubCheatLevel == 0 )
						{
							fGoodCheatLevelKey = TRUE;
							gubCheatLevel++;
						}
#else
						if ( gubCheatLevel == 4 )
						{
							gubCheatLevel++;
							fGoodCheatLevelKey = TRUE;
							// ATE; We're done.... start cheat mode....
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_CHEAT_LEVEL_TWO ] );
							SetHistoryFact( HISTORY_CHEAT_ENABLED, 0, GetWorldTotalMin(), -1, -1 );
						}
						else
						{
							RESET_CHEAT_LEVEL();
						}
#endif
					}
					else
					{
						ToggleItemGlow(!gGameSettings.fOptions[TOPTION_GLOW_ITEMS]);
					}
					break;

				case '$': break;

				case 'k':
					if( fAlt )
					{
						if ( fCtrl )
						{
							if ( CHEATER_CHEAT_LEVEL( ) )
							{
								// next shot by anybody is auto kill
								gfNextShotKills = !gfNextShotKills;
							}
						}
						else
						{
							if ( CHEATER_CHEAT_LEVEL( ) )
							{
								GrenadeTest1();
							}
						}
					}
					else if( fCtrl )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							GrenadeTest2();
						}
					}
					else
					{
						BeginKeyPanelFromKeyShortcut( );
					}
					break;

				case SDLK_INSERT: GoIntoOverheadMap(); break;

				case SDLK_END:
						if ( gusSelectedSoldier != NOBODY )
						{
							SOLDIERTYPE* const sel = GetSelectedMan();
							if (CheckForMercContMove(sel))
							{
								// Continue
								ContinueMercMovement(sel);
								ErasePath( TRUE );
							}
						}
						break;

				case SDLK_HOME:
						if ( gGameSettings.fOptions[ TOPTION_3D_CURSOR ] )
						{
							gGameSettings.fOptions[ TOPTION_3D_CURSOR ] = FALSE;

							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_3DCURSOR_OFF ] );
						}
						else
						{
							gGameSettings.fOptions[ TOPTION_3D_CURSOR ] = TRUE;

							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_3DCURSOR_ON ] );
						}
						break;

				case SDLK_l:
#ifdef JA2BETAVERSION
					if (fShift)
					{
						gfDisplayStrategicAILogs ^= TRUE;
						const wchar_t* Msg = gfDisplayStrategicAILogs ?
							L"Strategic AI Log visually enabled." :
							L"Strategic AI Log visually disabled.";
						ScreenMsg(FONT_LTKHAKI, MSG_INTERFACE, Msg);
					}
					else
#endif
					{
						if (fAlt)
						{
							if (!(gTacticalStatus.uiFlags & ENGAGED_IN_CONV))
							{
								LeaveTacticalScreen(GAME_SCREEN);
								DoQuickLoad();
							}
						}
						else if (fCtrl)
						{
							if (!(gTacticalStatus.uiFlags & ENGAGED_IN_CONV))
							{
								gfSaveGame = FALSE;
								gfCameDirectlyFromGame = TRUE;
								guiPreviousOptionScreen = GAME_SCREEN;
								LeaveTacticalScreen(SAVE_LOAD_SCREEN);
							}
/*
							if (INFORMATION_CHEAT_LEVEL())
							{
								*puiNewEvent = I_LEVELNODEDEBUG;
								CountLevelNodes();
							}
*/
						}
						else
						{
							// nothing in hand and either not in SM panel, or the matching button is enabled if we are in SM panel
							if (gpItemPointer == NULL &&
									(gsCurInterfacePanel != SM_PANEL || ButtonList[iSMPanelButtons[LOOK_BUTTON]]->uiFlags & BUTTON_ENABLED))
							{
								*puiNewEvent = LC_CHANGE_TO_LOOK;
							}
						}
					}
					break;

				case 'm':
					if( fAlt )
					{
						if ( INFORMATION_CHEAT_LEVEL( ) )
						{
							*puiNewEvent = I_LEVELNODEDEBUG;
							CountLevelNodes();
						}
					}
					else if (!fCtrl)
					{
						// nothing in hand and the Map Screen button for whichever panel we're in must be enabled
						if ( ( gpItemPointer == NULL ) && !gfDisableTacticalPanelButtons &&
								 ( ( ( gsCurInterfacePanel == SM_PANEL   ) && ( ButtonList[ iSMPanelButtons[ SM_MAP_SCREEN_BUTTON ] ]->uiFlags & BUTTON_ENABLED ) ) ||
									 ( ( gsCurInterfacePanel == TEAM_PANEL ) && ( ButtonList[ iTEAMPanelButtons[ TEAM_MAP_SCREEN_BUTTON ] ]->uiFlags & BUTTON_ENABLED ) ) ) )
						{
							// go to Map screen
	            if ( !( gTacticalStatus.uiFlags & ENGAGED_IN_CONV ) )
	            {
  							GoToMapScreenFromTactical();
              }
						}
					}
					break;

				case SDLK_PAGEDOWN:
					if ( CHEATER_CHEAT_LEVEL( ) )
					{
						if( fCtrl )
							AttemptToChangeFloorLevel( +1 ); //try to enter a lower underground level
					}

					if ( guiCurrentScreen != DEBUG_SCREEN )
					{
						if ( gusSelectedSoldier != NOBODY )
						{
							// nothing in hand and either not in SM panel, or the matching button is enabled if we are in SM panel
							if ( ( gpItemPointer == NULL ) )
							{
								GotoLowerStance(GetSelectedMan());
							}
						}
					}
					break;

				case SDLK_PAGEUP:
					if ( CHEATER_CHEAT_LEVEL( ) )
					{
						if( fCtrl )
							AttemptToChangeFloorLevel( -1 ); //try to go up towards ground level
					}

					if ( guiCurrentScreen != DEBUG_SCREEN )
					{
						if ( gusSelectedSoldier != NOBODY )
						{
							// nothing in hand and either not in SM panel, or the matching button is enabled if we are in SM panel
							if ( ( gpItemPointer == NULL ) )
							{
								GotoHeigherStance(GetSelectedMan());
							}
						}
					}
					break;


				case '*':

					if ( gTacticalStatus.uiFlags & RED_ITEM_GLOW_ON  )
					{
						gTacticalStatus.uiFlags &= (~RED_ITEM_GLOW_ON );
					}
					else
					{
						gTacticalStatus.uiFlags |= RED_ITEM_GLOW_ON;
					}
					break;

				case 'n':

					if( fAlt )
					{
						static UINT16 gQuoteNum = 0;

						if ( INFORMATION_CHEAT_LEVEL( ) )
						{
							if (gUIFullTarget != NULL)
							{
								TacticalCharacterDialogue(gUIFullTarget, gQuoteNum);
								gQuoteNum++;
							}
						}
					}
					else if( fCtrl )
					{
#ifdef GERMAN
						if ( gubCheatLevel == 4 )
						{
							fGoodCheatLevelKey = TRUE;
							gubCheatLevel++;
#if 0
							// ATE: Level one reached.....
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_CHEAT_LEVEL_ONE ] );
#endif
						}
						else
						{
							RESET_CHEAT_LEVEL();
						}
#endif
					}
					else
					if( !CycleSoldierFindStack( usMapPos ) )// Are we over a merc stack?
						CycleIntTileFindStack( usMapPos ); // If not, now check if we are over a struct stack
					break;

				case 'o':

					if( fAlt )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							gStrategicStatus.usPlayerKills += NumEnemiesInAnySector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
							ObliterateSector();
						}
					}
					else if(fCtrl)
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							CreatePlayerControlledMonster();
						}
					}
					else
					{
						// nothing in hand and the Options Screen button for whichever panel we're in must be enabled
						if ( ( gpItemPointer == NULL ) && !gfDisableTacticalPanelButtons &&
								 ( ( gsCurInterfacePanel != SM_PANEL ) || ( ButtonList[ iSMPanelButtons[ OPTIONS_BUTTON ] ]->uiFlags & BUTTON_ENABLED ) ) )
						{
							if( !fDisableMapInterfaceDueToBattle )
							{
								// go to Options screen
								guiPreviousOptionScreen = GAME_SCREEN;
								LeaveTacticalScreen( OPTIONS_SCREEN );
							}
						}
					}
					break;

				case 'p':
#ifdef JA2TESTVERSION
	#ifndef JA2DEMO
						if( fCtrl )
						{
							// CTRL-P: Display player's highest progress percentage
							DumpSectorDifficultyInfo();
						}
						else
	#endif
#endif
							HandleStanceChangeFromUIKeys( ANIM_PRONE );
					break;

				case 'r':
					if( gusSelectedSoldier != NO_SOLDIER )
					{
						SOLDIERTYPE* const s = GetSelectedMan();
						if( fAlt ) //reload selected merc's weapon
						{
							if ( CHEATER_CHEAT_LEVEL( ) )
							{
								ReloadWeapon(s, s->ubAttackingHand);
							}
						}
						else
						{
							if (!MercInWater(s) && !(s->uiStatusFlags & SOLDIER_ROBOT))
              {
							  //change selected merc to run
								if (s->usUIMovementMode != WALKING && s->usUIMovementMode != RUNNING)
							  {
									UIHandleSoldierStanceChange(s, ANIM_STAND);
									s->fUIMovementFast = 1;
							  }
							  else
							  {
									s->fUIMovementFast = 1;
									s->usUIMovementMode = RUNNING;
									gfPlotNewMovement = TRUE;
							  }
              }
						}
					}
					break;
				case 's':

					if( fCtrl )
					{
						if( !fDisableMapInterfaceDueToBattle && !( gTacticalStatus.uiFlags & ENGAGED_IN_CONV ) )
						{
							//if the game CAN be saved
							if( CanGameBeSaved() )
							{
								gfSaveGame = TRUE;
								gfCameDirectlyFromGame = TRUE;

								guiPreviousOptionScreen = GAME_SCREEN;
								LeaveTacticalScreen( SAVE_LOAD_SCREEN );
							}
							else
							{
								//Display a message saying the player cant save now
								DoMessageBox( MSG_BOX_BASIC_STYLE, zNewTacticalMessages[ TCTL_MSG__IRON_MAN_CANT_SAVE_NOW ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_OK, NULL, NULL);
							}
						}
					}
					else
					if( fAlt )
					{
						if( !fDisableMapInterfaceDueToBattle && !( gTacticalStatus.uiFlags & ENGAGED_IN_CONV ) )
						{
							//if the game CAN be saved
							if( CanGameBeSaved() )
							{
								guiPreviousOptionScreen = GAME_SCREEN;
								//guiPreviousOptionScreen = guiCurrentScreen;
								DoQuickSave();
							}
							else
							{
								//Display a message saying the player cant save now
								DoMessageBox( MSG_BOX_BASIC_STYLE, zNewTacticalMessages[ TCTL_MSG__IRON_MAN_CANT_SAVE_NOW ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_OK, NULL, NULL );
							}
						}
					}
					else if( gusSelectedSoldier != NOBODY )
					{
						gfPlotNewMovement = TRUE;
						HandleStanceChangeFromUIKeys( ANIM_STAND );
					}
					break;

				case 't':

					if( fAlt )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							TeleportSelectedSoldier();
						}
					}
					else if( fCtrl )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							TestCapture( );

							//EnterCombatMode( gbPlayerNum );
						}
					}
					else
						ToggleTreeTops();
					break;


        case '=':
					//if the display cover or line of sight is being displayed
					if (_KeyDown(SDLK_END) || _KeyDown(SDLK_DELETE))
					{
						if (_KeyDown(SDLK_DELETE))
							ChangeSizeOfDisplayCover( gGameSettings.ubSizeOfDisplayCover + 1 );

						if (_KeyDown(SDLK_END))
							ChangeSizeOfLOS( gGameSettings.ubSizeOfLOS + 1 );
					}
					else
					{

						#ifdef JA2TESTVERSION
							if( fAlt )
							{
								WarpGameTime( 60, TRUE );
								break;
							}
						#endif

							  // ATE: This key will select everybody in the sector
							  if (! (gTacticalStatus.uiFlags & INCOMBAT) )
							  {
								  SOLDIERTYPE *		pSoldier;
								  INT32						cnt;

								  cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;
								  for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++, pSoldier++ )
								  {

									  // Check if this guy is OK to control....
									  if ( OK_CONTROLLABLE_MERC( pSoldier ) && !( pSoldier->uiStatusFlags & ( SOLDIER_VEHICLE | SOLDIER_PASSENGER | SOLDIER_DRIVER ) ) )
									  {
        									pSoldier->uiStatusFlags |= SOLDIER_MULTI_SELECTED;
								  }
								}
								EndMultiSoldierSelection( TRUE );
							  }
					}
					break;

				case 'u':

					if( fAlt )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							RefreshSoldier();
						}
					}
					else if( fCtrl )
					{
						INT32 cnt;
						SOLDIERTYPE *pSoldier;

#ifdef GERMAN
						if ( gubCheatLevel == 2 )
						{
							fGoodCheatLevelKey = TRUE;
							gubCheatLevel++;
						}
						else
						{
							RESET_CHEAT_LEVEL();
						}
#endif

						if ( CHEATER_CHEAT_LEVEL( ) && gusSelectedSoldier != NOBODY )
						{
							for ( pSoldier = MercPtrs[ gbPlayerNum ], cnt = 0; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pSoldier++)
							{
								if ( pSoldier->bActive && pSoldier->bLife > 0 )
								{
									// Get breath back
									pSoldier->bBreath								= pSoldier->bBreathMax;

									// Get life back
									pSoldier->bLife									= pSoldier->bLifeMax;
									pSoldier->bBleeding							= 0;

									fInterfacePanelDirty = DIRTYLEVEL2;
								}
							}
						}
					}

					else if ( gusSelectedSoldier != NO_SOLDIER )
						*puiNewEvent = M_CHANGE_TO_ACTION;
					break;

				case 'v':
					if( fAlt )
					{
#ifdef JA2TESTVERSION
						if( gfDoVideoScroll ^= TRUE )
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Video Scroll ON"  );
						else
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Video Scroll OFF"  );
#endif
					}
					else if( fCtrl )
					{
						#ifdef SGP_VIDEO_DEBUGGING
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"VObjects:  %d", guiVObjectSize );
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"VSurfaces:  %d", guiVSurfaceSize );
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"SGPVideoDump.txt updated..." );
							PerformVideoInfoDumpIntoFile( "SGPVideoDump.txt", TRUE );
						#endif
					}
					else
						DisplayGameSettings( );

					break;

				case SDLK_w:
					if( fAlt )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							if ( InItemDescriptionBox( ) )
							{
								// Swap item in description panel...
								CycleItemDescriptionItem( );

							}
							else
							{
								CycleSelectedMercsItem();
							}
						}
					}
					else if( fCtrl )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							if ( gusSelectedSoldier != NOBODY )
							{
								CreateItem(FLAMETHROWER, 100, &GetSelectedMan()->inv[HANDPOS]);
							}
						}
					}
					else
						ToggleWireFrame();
					break;

				case 'y':
					if( fAlt )
					{
						if ( CHEATER_CHEAT_LEVEL( ) )
						{
							QuickCreateProfileMerc( CIV_TEAM, MARIA ); //Ira

							// Recruit!
							RecruitEPC( MARIA );
						}

						SOLDIERTYPE* const robot = FindSoldierByProfileID(ROBOT, FALSE);
						if (robot != NULL)
						{
							OBJECTTYPE Object;
							CreateItem(G41, 100, &Object);
							AutoPlaceObject(robot, &Object, FALSE);
						}
					}
					else
					{
						if ( INFORMATION_CHEAT_LEVEL( ) )
						{
							*puiNewEvent = I_LOSDEBUG;
						}
					}
					//else if( gusSelectedSoldier != NO_SOLDIER )
					break;
				case 'z':
					if( fCtrl )
					{
						if ( INFORMATION_CHEAT_LEVEL( ) )
						{
							ToggleZBuffer();
						}
					}
					else if ( fAlt )
					{
						// Toggle squad's stealth mode.....
						// For each guy on squad...
						{
							SOLDIERTYPE				*pTeamSoldier;
							INT8							bLoop;
							BOOLEAN						fStealthOn = FALSE;

							// Check if at least one guy is on stealth....
							for (bLoop=gTacticalStatus.Team[gbPlayerNum].bFirstID, pTeamSoldier=MercPtrs[bLoop]; bLoop <= gTacticalStatus.Team[gbPlayerNum].bLastID; bLoop++, pTeamSoldier++)
							{
								if ( OK_CONTROLLABLE_MERC( pTeamSoldier ) && pTeamSoldier->bAssignment == CurrentSquad( ) )
								{
									if ( pTeamSoldier->bStealthMode )
									{
										fStealthOn = TRUE;
									}
								}
							}

							fStealthOn = !fStealthOn;

							for (bLoop=gTacticalStatus.Team[gbPlayerNum].bFirstID, pTeamSoldier=MercPtrs[bLoop]; bLoop <= gTacticalStatus.Team[gbPlayerNum].bLastID; bLoop++, pTeamSoldier++)
							{
								if ( OK_CONTROLLABLE_MERC( pTeamSoldier ) && pTeamSoldier->bAssignment == CurrentSquad( ) && !AM_A_ROBOT( pTeamSoldier ) )
								{
									if ( gpSMCurrentMerc != NULL && bLoop == gpSMCurrentMerc->ubID )
									{
										gfUIStanceDifferent = TRUE;
									}

									pTeamSoldier->bStealthMode = fStealthOn;
								}
							}

							fInterfacePanelDirty = DIRTYLEVEL2;

							// OK, display message
							if ( fStealthOn )
							{
								ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_SQUAD_ON_STEALTHMODE ] );
							}
							else
							{
								ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_SQUAD_OFF_STEALTHMODE ] );
							}
						}
					}
					else
					{
						// nothing in hand and either not in SM panel, or the matching button is enabled if we are in SM panel
						if ( ( gpItemPointer == NULL ) )
						{
              HandleStealthChangeFromUIKeys( );
						}
					}
					break;

				case '-':
				case '_':
					//if the display cover or line of sight is being displayed
					if (_KeyDown(SDLK_END) || _KeyDown(SDLK_DELETE))
					{
						if (_KeyDown(SDLK_DELETE))
							ChangeSizeOfDisplayCover( gGameSettings.ubSizeOfDisplayCover - 1 );

						if (_KeyDown(SDLK_END))
							ChangeSizeOfLOS( gGameSettings.ubSizeOfLOS - 1 );
					}
					else
					{
						if( fAlt )
						{
							const UINT32 vol = MusicGetVolume();
							MusicSetVolume(vol > 20 ? vol - 20 : 0);
						}
						else if( fCtrl )
						{
	#ifdef JA2TESTVERSION
							gTacticalStatus.bRealtimeSpeed = max( 1, gTacticalStatus.bRealtimeSpeed - 1 );
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Decreasing Realtime speed to %d", gTacticalStatus.bRealtimeSpeed );
	#endif
						}
						else
						{
	#ifdef JA2TESTVERSION
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Using Normal Scroll Speed"  );
							gubCurScrollSpeedID = 1;
	#endif
						}
					}
					break;
				case '+':

#ifdef JA2TESTVERSION
					if( fAlt )
					{
						const UINT32 vol = MusicGetVolume();
  					MusicSetVolume(min(vol + 20, MAXVOLUME));
					}
					else if( fCtrl )
					{
						gTacticalStatus.bRealtimeSpeed = min( MAX_REALTIME_SPEED_VAL, gTacticalStatus.bRealtimeSpeed+1 );
						ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Increasing Realtime speed to %d", gTacticalStatus.bRealtimeSpeed );
					}
					else
					{
						ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Using Higher Scroll Speed"  );
						gubCurScrollSpeedID = 2;
					}
#endif
					break;
				case '`':

					// Switch panels...
					{
						ToggleTacticalPanels();

						if ( CHEATER_CHEAT_LEVEL( ) )
            {
              //EnvBeginRainStorm( 1 );
            }

					}
					break;

			}

#ifdef GERMAN
			if ( !fGoodCheatLevelKey && gubCheatLevel < 5 )
			{
				RESET_CHEAT_LEVEL( );
			}
#else
			if ( !fGoodCheatLevelKey && gubCheatLevel < 4 )
			{
				RESET_CHEAT_LEVEL( );
			}
#endif

		}
	}
}


static void HandleTalkingMenuKeys(InputAtom* pInputEvent, UINT32* puiNewEvent)
{
	// CHECK ESC KEYS HERE....
	if ( pInputEvent->usEvent == KEY_UP )
	{
		if (pInputEvent->usParam == SDLK_ESCAPE)
		{
			//Handle esc in talking menu
			if ( HandleTalkingMenuEscape( TRUE, TRUE ) )
			{
				*puiNewEvent = A_CHANGE_TO_MOVE;
			}
		}
		else if (pInputEvent->usParam == SDLK_BACKSPACE)
		{
			HandleTalkingMenuBackspace();
		}

	}
}


static void HandleSectorExitMenuKeys(InputAtom* pInputEvent, UINT32* puiNewEvent)
{
	// CHECK ESC KEYS HERE....
  if (pInputEvent->usEvent == KEY_UP && pInputEvent->usParam == SDLK_ESCAPE)
  {
		//Handle esc in talking menu
		RemoveSectorExitMenu( FALSE );

		*puiNewEvent = A_CHANGE_TO_MOVE;
	}
}


static void HandleOpenDoorMenuKeys(InputAtom* pInputEvent, UINT32* puiNewEvent)
{
	// CHECK ESC KEYS HERE....
  if (pInputEvent->usEvent == KEY_UP && pInputEvent->usParam == SDLK_ESCAPE)
  {
		//Handle esc in talking menu
		CancelOpenDoorMenu( );
		HandleOpenDoorMenu( );
		*puiNewEvent = A_CHANGE_TO_MOVE;
	}
}


static void HandleMenuKeys(InputAtom* pInputEvent, UINT32* puiNewEvent)
{
	// CHECK ESC KEYS HERE....
  if (pInputEvent->usEvent == KEY_UP && pInputEvent->usParam == SDLK_ESCAPE)
  {
		//Handle esc in talking menu
		CancelMovementMenu( );

		*puiNewEvent = A_CHANGE_TO_MOVE;
	}
}


static void HandleItemMenuKeys(InputAtom* pInputEvent, UINT32* puiNewEvent)
{
	// CHECK ESC KEYS HERE....
  if (pInputEvent->usEvent == KEY_UP && pInputEvent->usParam == SDLK_ESCAPE)
  {
		//Handle esc in talking menu
		RemoveItemPickupMenu( );
		*puiNewEvent = A_CHANGE_TO_MOVE;
	}
}

BOOLEAN HandleCheckForExitArrowsInput( BOOLEAN fAdjustConfirm )
{
	INT16 sMapPos;

	// If not in move mode, return!
	if ( gCurrentUIMode != MOVE_MODE )
	{
		return( FALSE );
	}

	if ( gusSelectedSoldier == NOBODY )
	{
		return( FALSE );
	}

	// ATE: Remove confirm for exit arrows...
	fAdjustConfirm = TRUE;
	gfUIConfirmExitArrows = TRUE;


	//Return right away, saying that we are in this mode, don't do any normal stuff!
	if ( guiCurrentUICursor == NOEXIT_EAST_UICURSOR || guiCurrentUICursor == NOEXIT_WEST_UICURSOR
		  || guiCurrentUICursor == NOEXIT_NORTH_UICURSOR || guiCurrentUICursor == NOEXIT_SOUTH_UICURSOR ||
				 guiCurrentUICursor == NOEXIT_GRID_UICURSOR )
	{
		// Yeah, but add a message....
		if( gfInvalidTraversal )
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ CANNOT_LEAVE_SECTOR_FROM_SIDE_STR ] );
			gfInvalidTraversal = FALSE;
		}
		else if( gfRobotWithoutControllerAttemptingTraversal )
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, gzLateLocalizedString[ 1 ] );
			gfRobotWithoutControllerAttemptingTraversal = FALSE;
		}
		else if( gfLoneEPCAttemptingTraversal )
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, pExitingSectorHelpText[EXIT_GUI_ESCORTED_CHARACTERS_CANT_LEAVE_SECTOR_ALONE_STR], GetSelectedMan()->name);
			gfLoneEPCAttemptingTraversal = FALSE;
		}
		else if( gubLoneMercAttemptingToAbandonEPCs )
		{
			const SOLDIERTYPE* const sel = GetSelectedMan();
			wchar_t str[256];
			if( gubLoneMercAttemptingToAbandonEPCs == 1 )
			{ //Use the singular version of the string
				if (gMercProfiles[sel->ubProfile ].bSex == MALE)
				{ //male singular
					swprintf(str, lengthof(str), pExitingSectorHelpText[EXIT_GUI_MERC_CANT_ISOLATE_EPC_HELPTEXT_MALE_SINGULAR], sel->name, MercPtrs[gbPotentiallyAbandonedEPCSlotID]->name);
				}
				else
				{ //female singular
					swprintf(str, lengthof(str), pExitingSectorHelpText[EXIT_GUI_MERC_CANT_ISOLATE_EPC_HELPTEXT_FEMALE_SINGULAR], sel->name, MercPtrs[gbPotentiallyAbandonedEPCSlotID]->name);
				}
			}
			else
			{ //Use the plural version of the string
				if (gMercProfiles[sel->ubProfile].bSex == MALE)
				{ //male plural
					swprintf(str, lengthof(str), pExitingSectorHelpText[EXIT_GUI_MERC_CANT_ISOLATE_EPC_HELPTEXT_MALE_PLURAL], sel->name);
				}
				else
				{ //female plural
					swprintf(str, lengthof(str), pExitingSectorHelpText[EXIT_GUI_MERC_CANT_ISOLATE_EPC_HELPTEXT_FEMALE_PLURAL], sel->name);
				}
			}
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, str );
			gubLoneMercAttemptingToAbandonEPCs = FALSE;
		}
		else
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[MERC_IS_TOO_FAR_AWAY_STR], GetSelectedMan()->name);
		}

		return( TRUE );
	}

	// Check if we want to exit!
	if ( guiCurrentUICursor == EXIT_GRID_UICURSOR || guiCurrentUICursor == CONFIRM_EXIT_GRID_UICURSOR  )
	{
		if ( fAdjustConfirm )
		{
			if ( !gfUIConfirmExitArrows )
			{
				gfUIConfirmExitArrows = TRUE;
			}
			else
			{
				if ( !GetMouseMapPos( &sMapPos ) )
				{
					return( FALSE );
				}

				// Goto next sector
				//SimulateMouseMovement( gusMouseXPos - 5, gusMouseYPos );
				InitSectorExitMenu( DIRECTION_EXITGRID, sMapPos );
			}
		}
		return( TRUE );
	}

	// Check if we want to exit!
	if ( guiCurrentUICursor == EXIT_EAST_UICURSOR || guiCurrentUICursor == CONFIRM_EXIT_EAST_UICURSOR  )
	{
		if ( fAdjustConfirm )
		{
			if ( !gfUIConfirmExitArrows )
			{
				gfUIConfirmExitArrows = TRUE;
			}
			else
			{
				// Goto next sector
				//SimulateMouseMovement( gusMouseXPos - 5, gusMouseYPos );
				InitSectorExitMenu( EAST, 0 );
			}
		}
		return( TRUE );
	}
	if ( guiCurrentUICursor == EXIT_WEST_UICURSOR || guiCurrentUICursor == CONFIRM_EXIT_WEST_UICURSOR  )
	{
		if ( fAdjustConfirm )
		{
			if ( !gfUIConfirmExitArrows )
			{
				gfUIConfirmExitArrows = TRUE;
			}
			else
			{
				// Goto next sector
				//SimulateMouseMovement( gusMouseXPos + 5, gusMouseYPos );
				InitSectorExitMenu( WEST, 0);
			}
		}
		return( TRUE );
	}
	if ( guiCurrentUICursor == EXIT_NORTH_UICURSOR || guiCurrentUICursor == CONFIRM_EXIT_NORTH_UICURSOR )
	{
		if ( fAdjustConfirm )
		{
			if ( !gfUIConfirmExitArrows )
			{
				gfUIConfirmExitArrows = TRUE;
			}
			else
			{
				// Goto next sector
				//SimulateMouseMovement( gusMouseXPos, gusMouseYPos + 5 );
				InitSectorExitMenu( NORTH , 0);
			}
		}
		return( TRUE );
	}
	if ( guiCurrentUICursor == EXIT_SOUTH_UICURSOR || guiCurrentUICursor == CONFIRM_EXIT_SOUTH_UICURSOR )
	{
		if ( fAdjustConfirm )
		{
			if ( !gfUIConfirmExitArrows )
			{
				gfUIConfirmExitArrows = TRUE;
			}
			else
			{
				// Goto next sector
				//SimulateMouseMovement( gusMouseXPos, gusMouseYPos - 5);
				InitSectorExitMenu( SOUTH, 0 );
			}
		}
		return( TRUE );
	}
	return( FALSE );
}


//Simple function implementations called by keyboard input

static void CreateRandomItem(void)
{
	OBJECTTYPE		Object;
	UINT16 usMapPos;
	if ( GetMouseMapPos( &usMapPos ) )
	{
		CreateItem( (UINT16) (Random( 35 ) + 1), 100, &Object );
		AddItemToPool( usMapPos, &Object, -1 , 0, 0, 0 );
	}
}


static void MakeSelectedSoldierTired(void)
{
	// Key to make guy get tired!
	OBJECTTYPE		Object;
	UINT16 usMapPos;
	if ( GetMouseMapPos( &usMapPos ) )
	{
		CreateItem( (UINT16)TNT, 100, &Object );
		AddItemToPool( usMapPos, &Object, -1, 0, 0, 0 );
	}

	// CHECK IF WE'RE ON A GUY ( EITHER SELECTED, OURS, OR THEIRS
	const SOLDIERTYPE* const tgt = gUIFullTarget;
	if (tgt != NULL)
	{
		//FatigueCharacter(tgt);
		fInterfacePanelDirty = DIRTYLEVEL2;
	}
}


static void ToggleRealTime(UINT32* puiNewEvent)
{
	if ( gTacticalStatus.uiFlags & TURNBASED )
	{
		// Change to real-time
		gTacticalStatus.uiFlags &= (~TURNBASED);
		gTacticalStatus.uiFlags |= REALTIME;

		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Switching to Realtime." );
	}
	else if ( gTacticalStatus.uiFlags & REALTIME )
	{
		// Change to turn-based
		gTacticalStatus.uiFlags |= TURNBASED;
		gTacticalStatus.uiFlags &=( ~REALTIME );

		*puiNewEvent = M_ON_TERRAIN;

		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Switching to Turnbased." );
	}

	// Plot new path!
	gfPlotNewMovement = TRUE;
}


static void ToggleViewAllMercs(void)
{
	// Set option to show all mercs
	if ( gTacticalStatus.uiFlags&SHOW_ALL_MERCS )
	{
		gTacticalStatus.uiFlags&= (~SHOW_ALL_MERCS );
	}
	else
	{
		gTacticalStatus.uiFlags|= SHOW_ALL_MERCS;
	}

	// RE-RENDER
	SetRenderFlags(RENDER_FLAG_FULL);
}


static void ToggleViewAllItems(void)
{
	// Set option to show all mercs
	if( gTacticalStatus.uiFlags & SHOW_ALL_ITEMS )
	{
		gTacticalStatus.uiFlags &= ~SHOW_ALL_ITEMS ;
	}
	else
	{
		gTacticalStatus.uiFlags |= SHOW_ALL_ITEMS;
	}

	ToggleItemGlow(gGameSettings.fOptions[TOPTION_GLOW_ITEMS]);

	// RE-RENDER
	SetRenderFlags(RENDER_FLAG_FULL);
}


static void TestExplosion(void)
{
	UINT16 usMapPos;
	if ( GetMouseMapPos( &usMapPos ) )
	{
		EXPLOSION_PARAMS	ExpParams ;
		ExpParams.uiFlags			= 0;
		ExpParams.ubOwner			= NOBODY;
		ExpParams.ubTypeID		= STUN_BLAST;
		ExpParams.sGridNo			= usMapPos;

		GenerateExplosion( &ExpParams );

		PlayJA2Sample(EXPLOSION_1, MIDVOLUME, 1, MIDDLEPAN);
	}
}


static void CycleSelectedMercsItem(void)
{
	// Cycle selected guy's item...
	SOLDIERTYPE* const tgt = gUIFullTarget;
	if (tgt != NULL)
	{
		UINT16 usOldItem = tgt->inv[HANDPOS].usItem;

		usOldItem++;

		if ( usOldItem > MAX_WEAPONS )
		{
			usOldItem = 0;
		}

		CreateItem(usOldItem, 100, &tgt->inv[HANDPOS]);
		DirtyMercPanelInterface(tgt, DIRTYLEVEL2);
	}
}


static void ToggleWireFrame(void)
{
	if ( gGameSettings.fOptions[ TOPTION_TOGGLE_WIREFRAME ] )
	{
		gGameSettings.fOptions[ TOPTION_TOGGLE_WIREFRAME ] = FALSE;

		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_WIREFRAMES_REMOVED ] );
	}
	else
	{
		gGameSettings.fOptions[ TOPTION_TOGGLE_WIREFRAME ] = TRUE;

		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_WIREFRAMES_ADDED ] );
	}

	SetRenderFlags(RENDER_FLAG_FULL );
}


static void RefreshSoldier(void)
{
	// CHECK IF WE'RE ON A GUY ( EITHER SELECTED, OURS, OR THEIRS
	SOLDIERTYPE* const tgt = gUIFullTarget;
	if (tgt != NULL) ReviveSoldier(tgt);
}


static void ChangeSoldiersBodyType(UINT8 ubBodyType, BOOLEAN fCreateNewPalette)
{
	if( gusSelectedSoldier != NO_SOLDIER )
	{
		SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
		if (pSoldier != NULL)
		{
			pSoldier->ubBodyType = ubBodyType;
			EVENT_InitNewSoldierAnim( pSoldier, STANDING, 0 , TRUE );
			//SetSoldierAnimationSurface( pSoldier, pSoldier->usAnimState );
			if( fCreateNewPalette )
			{
				CreateSoldierPalettes( pSoldier );

				switch( ubBodyType )
				{
					case ADULTFEMALEMONSTER:
					case AM_MONSTER:
					case YAF_MONSTER:
					case YAM_MONSTER:
					case LARVAE_MONSTER:
					case INFANT_MONSTER:
					case QUEENMONSTER:

						pSoldier->uiStatusFlags |= SOLDIER_MONSTER;
						memset( &(pSoldier->inv), 0, sizeof( OBJECTTYPE ) * NUM_INV_SLOTS );
						AssignCreatureInventory( pSoldier );

      			CreateItem( CREATURE_YOUNG_MALE_SPIT  ,			100, &(pSoldier->inv[HANDPOS]) );

						break;

					case TANK_NW:
					case TANK_NE:

						pSoldier->uiStatusFlags |= SOLDIER_VEHICLE;
						//pSoldier->inv[ HANDPOS ].usItem = TANK_CANNON;

						pSoldier->inv[ HANDPOS ].usItem = MINIMI;
						pSoldier->bVehicleID = (INT8)AddVehicleToList( pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ, HUMMER );

						break;
				}

			}
		}
	}
}


static void TeleportSelectedSoldier(void)
{
	UINT16 usMapPos;
	// CHECK IF WE'RE ON A GUY ( EITHER SELECTED, OURS, OR THEIRS
	SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
	if (pSoldier != NULL)
	{
		if ( GetMouseMapPos( &usMapPos ) )
		{
			// Check level first....
			if ( gsInterfaceLevel == 0 )
			{
				SetSoldierHeight( pSoldier, 0 );
				TeleportSoldier( pSoldier, usMapPos, FALSE );
				EVENT_StopMerc( pSoldier, pSoldier->sGridNo, pSoldier->bDirection );
			}
			else
			{
				// Is there a roof?
				if ( FindStructure( usMapPos, STRUCTURE_ROOF ) != NULL )
				{
					SetSoldierHeight( pSoldier, 50.0 );

					TeleportSoldier( pSoldier, usMapPos, TRUE );
					EVENT_StopMerc( pSoldier, pSoldier->sGridNo, pSoldier->bDirection );
				}
			}
		}
	}
}


static void ToggleTreeTops(void)
{
	if ( gGameSettings.fOptions[ TOPTION_TOGGLE_TREE_TOPS ] )
	{
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ REMOVING_TREETOPS_STR ] );
		WorldHideTrees( );
		gTacticalStatus.uiFlags |= NOHIDE_REDUNDENCY;
	}
	else
	{
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ SHOWING_TREETOPS_STR ]);
		WorldShowTrees( );
		gTacticalStatus.uiFlags &= (~NOHIDE_REDUNDENCY);
	}
	gGameSettings.fOptions[ TOPTION_TOGGLE_TREE_TOPS ] = !gGameSettings.fOptions[ TOPTION_TOGGLE_TREE_TOPS ];

	// FOR THE NEXT RENDER LOOP, RE-EVALUATE REDUNDENT TILES
	InvalidateWorldRedundency( );
}


static void ToggleZBuffer(void)
{
	// Set option to show all mercs
	if ( gTacticalStatus.uiFlags&SHOW_Z_BUFFER )
	{
		gTacticalStatus.uiFlags&= (~SHOW_Z_BUFFER );
		SetRenderFlags(SHOW_Z_BUFFER);
	}
	else
	{
		gTacticalStatus.uiFlags|= SHOW_Z_BUFFER;
	}
}


static void TogglePlanningMode(void)
{
	UINT16 usMapPos;
	// DO ONLY IN TURNED BASED!
	if ( gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT) )
	{
		// CANCEL FROM PLANNING MODE!
		if ( InUIPlanMode( ) )
		{
			EndUIPlan( );
		}
		else if ( GetMouseMapPos( &usMapPos ) )
		{
			switch( gCurrentUIMode )
			{
				case MOVE_MODE:
					if ( gusSelectedSoldier != NO_SOLDIER )
					{
						SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
						BeginUIPlan( pSoldier );
						AddUIPlan( usMapPos, UIPLAN_ACTION_MOVETO );
					}
					break;
				case ACTION_MODE:
					if ( gusSelectedSoldier != NO_SOLDIER )
					{
						SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
						BeginUIPlan( pSoldier );
						AddUIPlan( usMapPos, UIPLAN_ACTION_FIRE );
					}
					break;
			}
		}
	}
}


static void SetBurstMode(void)
{
	if (gusSelectedSoldier != NO_SOLDIER) ChangeWeaponMode(GetSelectedMan());
}


static void ObliterateSector(void)
{
	INT32 cnt;
	SOLDIERTYPE *pTSoldier;

	// Kill everybody!
	cnt = gTacticalStatus.Team[ gbPlayerNum ].bLastID + 1;

	#ifdef JA2BETAVERSION
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Obliterating Sector!" );
	#endif

	for ( pTSoldier = MercPtrs[ cnt ]; cnt < MAX_NUM_SOLDIERS; pTSoldier++, cnt++ )
	{
		if ( pTSoldier->bActive && !pTSoldier->bNeutral && (pTSoldier->bSide != gbPlayerNum ) )
		{
				//	ANITILE_PARAMS	AniParams;
			//		memset( &AniParams, 0, sizeof( ANITILE_PARAMS ) );
			//		AniParams.sGridNo							= pTSoldier->sGridNo;
			//		AniParams.ubLevelID						= ANI_STRUCT_LEVEL;
			//	AniParams.usTileType				  = FIRSTEXPLOSION;
				//	AniParams.usTileIndex					= FIRSTEXPLOSION1;
				//	AniParams.sDelay							= 80;
				//	AniParams.sStartFrame					= 0;
				//	AniParams.uiFlags							= ANITILE_FORWARD;

				//	CreateAnimationTile( &AniParams );
					//PlayJA2Sample(EXPLOSION_1, MIDVOLUME, 1, MIDDLEPAN);

			EVENT_SoldierGotHit(pTSoldier, 0, 400, 0, pTSoldier->bDirection, 320, NOBODY, FIRE_WEAPON_NO_SPECIAL, pTSoldier->bAimShotLocation, NOWHERE);
		}
	}
}


static void RandomizeMercProfile(void)
{
	SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
	if (pSoldier != NULL)
	{
		// Change guy!
		ForceSoldierProfileID( pSoldier, (UINT8)Random( 30 ) );

		// Dirty interface
		DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
	}
}


static void JumpFence(void)
{
	INT8							bDirection;
	SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
	if (pSoldier != NULL)
	{
		if ( FindFenceJumpDirection( pSoldier, pSoldier->sGridNo, pSoldier->bDirection, &bDirection ) )
		{
			BeginSoldierClimbFence( pSoldier );
		}
	}
}


static void CreateNextCivType(void)
{
	INT16							sWorldX, sWorldY;
	SOLDIERCREATE_STRUCT		MercCreateStruct;
	UINT16 usMapPos;
	static						INT8 bBodyType = FATCIV;
	// Get Grid Corrdinates of mouse
	if ( GetMouseWorldCoordsInCenter( &sWorldX, &sWorldY ) && GetMouseMapPos( &usMapPos ) )
	{
		INT8							iNewIndex;

		memset( &MercCreateStruct, 0, sizeof( MercCreateStruct ) );
		MercCreateStruct.ubProfile		= NO_PROFILE;
		MercCreateStruct.sSectorX			= gWorldSectorX;
		MercCreateStruct.sSectorY			= gWorldSectorY;
		MercCreateStruct.bSectorZ			= gbWorldSectorZ;
		MercCreateStruct.bBodyType		= bBodyType;
		MercCreateStruct.bDirection = SOUTH;

		bBodyType++;

		if ( bBodyType > KIDCIV )
		{
			bBodyType = FATCIV;
		}

		MercCreateStruct.bTeam					= CIV_TEAM;
		MercCreateStruct.sInsertionGridNo		= usMapPos;
		RandomizeNewSoldierStats( &MercCreateStruct );

		if ( TacticalCreateSoldier( &MercCreateStruct, &iNewIndex ) )
		{
			AddSoldierToSector( iNewIndex );

			// So we can see them!
			AllTeamsLookForAll(NO_INTERRUPTS);

		}
	}
}


static void ToggleCliffDebug(void)
{
	// Set option to show all mercs
	if ( gTacticalStatus.uiFlags&DEBUGCLIFFS )
	{
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Cliff debug OFF." );

		gTacticalStatus.uiFlags&= (~DEBUGCLIFFS );
		SetRenderFlags(RENDER_FLAG_FULL);
	}
	else
	{
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Cliff debug ON." );

		gTacticalStatus.uiFlags|= DEBUGCLIFFS;
	}
}


static void CreateCow(void)
{
	INT16							sWorldX, sWorldY;
	SOLDIERCREATE_STRUCT		MercCreateStruct;
	UINT16 usMapPos;
	// Get Grid Corrdinates of mouse
	if ( GetMouseWorldCoordsInCenter( &sWorldX, &sWorldY ) && GetMouseMapPos( &usMapPos ) )
	{
		INT8							iNewIndex;

		memset( &MercCreateStruct, 0, sizeof( MercCreateStruct ) );
		MercCreateStruct.ubProfile		= NO_PROFILE;
		MercCreateStruct.sSectorX			= gWorldSectorX;
		MercCreateStruct.sSectorY			= gWorldSectorY;
		MercCreateStruct.bSectorZ			= gbWorldSectorZ;
		MercCreateStruct.bBodyType		= COW;
		//MercCreateStruct.bTeam				= SOLDIER_CREATE_AUTO_TEAM;
		MercCreateStruct.bTeam				= CIV_TEAM;
		MercCreateStruct.sInsertionGridNo		= usMapPos;
		RandomizeNewSoldierStats( &MercCreateStruct );

		if ( TacticalCreateSoldier( &MercCreateStruct, &iNewIndex ) )
		{
			AddSoldierToSector( iNewIndex );

			// So we can see them!
			AllTeamsLookForAll(NO_INTERRUPTS);

		}
	}
}


static void CreatePlayerControlledCow(void)
{
	INT16							sWorldX, sWorldY;
	SOLDIERCREATE_STRUCT		MercCreateStruct;
	UINT16 usMapPos;
	// Get Grid Corrdinates of mouse
	if ( GetMouseWorldCoordsInCenter( &sWorldX, &sWorldY ) && GetMouseMapPos( &usMapPos ) )
	{
		INT8							iNewIndex;

		memset( &MercCreateStruct, 0, sizeof( MercCreateStruct ) );
		MercCreateStruct.ubProfile		= 12;
		MercCreateStruct.sSectorX			= gWorldSectorX;
		MercCreateStruct.sSectorY			= gWorldSectorY;
		MercCreateStruct.bSectorZ			= gbWorldSectorZ;
		MercCreateStruct.bBodyType		= COW;
		MercCreateStruct.sInsertionGridNo		= usMapPos;
		MercCreateStruct.bTeam					= SOLDIER_CREATE_AUTO_TEAM;
		MercCreateStruct.fPlayerMerc		= TRUE;

		RandomizeNewSoldierStats( &MercCreateStruct );

		if ( TacticalCreateSoldier( &MercCreateStruct, &iNewIndex ) )
		{
			AddSoldierToSector( iNewIndex );

			// So we can see them!
			AllTeamsLookForAll(NO_INTERRUPTS);
		}
	}
}


static void GrenadeTest1(void)
{
	// Get mousexy
	INT16 sX, sY;
	if ( GetMouseXY( &sX, &sY ) )
	{
		OBJECTTYPE		Object;
		Object.usItem = MUSTARD_GRENADE;
		Object.bStatus[ 0 ] = 100;
		Object.ubNumberOfObjects = 1;
		CreatePhysicalObject( &Object, 60,  (FLOAT)(sX * CELL_X_SIZE), (FLOAT)(sY * CELL_Y_SIZE ), 256, -20, 20, 158, NOBODY, THROW_ARM_ITEM, 0 );
	}
}


static void GrenadeTest2(void)
{
	// Get mousexy
	INT16 sX, sY;
	if ( GetMouseXY( &sX, &sY ) )
	{
		OBJECTTYPE		Object;
		Object.usItem = HAND_GRENADE;
		Object.bStatus[ 0 ] = 100;
		Object.ubNumberOfObjects = 1;
		CreatePhysicalObject( &Object, 60,  (FLOAT)(sX * CELL_X_SIZE), (FLOAT)(sY * CELL_Y_SIZE ), 256, 0, -30, 158, NOBODY, THROW_ARM_ITEM, 0 );
	}
}


static void GrenadeTest3(void)
{
	// Get mousexy
	INT16 sX, sY;
	if ( GetMouseXY( &sX, &sY ) )
	{
		OBJECTTYPE		Object;
		Object.usItem = HAND_GRENADE;
		Object.bStatus[ 0 ] = 100;
		Object.ubNumberOfObjects = 1;
		CreatePhysicalObject( &Object, 60,  (FLOAT)(sX * CELL_X_SIZE), (FLOAT)(sY * CELL_Y_SIZE ), 256, -10, 10, 158, NOBODY, THROW_ARM_ITEM, 0 );
	}
}


static void CreatePlayerControlledMonster(void)
{
	INT16							sWorldX, sWorldY;
	UINT16 usMapPos;
	if ( GetMouseWorldCoordsInCenter( &sWorldX, &sWorldY ) && GetMouseMapPos( &usMapPos ) )
	{
		SOLDIERCREATE_STRUCT		MercCreateStruct;
		INT8							iNewIndex;

		memset( &MercCreateStruct, 0, sizeof( MercCreateStruct ) );
		MercCreateStruct.ubProfile		= NO_PROFILE;
		MercCreateStruct.sSectorX			= gWorldSectorX;
		MercCreateStruct.sSectorY			= gWorldSectorY;
		MercCreateStruct.bSectorZ			= gbWorldSectorZ;

		//Note:  only gets called if Alt and/or Ctrl isn't pressed!
		if (_KeyDown(SDLK_INSERT))
			MercCreateStruct.bBodyType		= QUEENMONSTER;
			//MercCreateStruct.bBodyType		= LARVAE_MONSTER;
		else
			MercCreateStruct.bBodyType		= ADULTFEMALEMONSTER;
		MercCreateStruct.bTeam				= SOLDIER_CREATE_AUTO_TEAM;
		MercCreateStruct.sInsertionGridNo		= usMapPos;
		RandomizeNewSoldierStats( &MercCreateStruct );

		if ( TacticalCreateSoldier( &MercCreateStruct, &iNewIndex ) )
		{
			AddSoldierToSector( iNewIndex );
		}
	}
}


static INT8 CheckForAndHandleHandleVehicleInteractiveClick(SOLDIERTYPE* pSoldier, UINT16 usMapPos, BOOLEAN fMovementMode)
{
	// Look for an item pool
	INT16							sActionGridNo;
	UINT8							ubDirection;
	INT16							sAPCost = 0;

	const SOLDIERTYPE* const pTSoldier = gUIFullTarget;
	if (pTSoldier != NULL)
	{
		 if ( OK_ENTERABLE_VEHICLE( pTSoldier ) && pTSoldier->bVisible != -1 && OKUseVehicle( pTSoldier->ubProfile ) )
		 {
			 if ( ( GetNumberInVehicle( pTSoldier->bVehicleID ) == 0 ) || !fMovementMode )
			 {
				 // Find a gridno closest to sweetspot...
				 sActionGridNo = FindGridNoFromSweetSpotWithStructDataFromSoldier( pSoldier, pSoldier->usUIMovementMode, 5, &ubDirection, 0, pTSoldier );

				 if ( sActionGridNo != NOWHERE )
				 {
						// Calculate AP costs...
						//sAPCost = GetAPsToBeginFirstAid( pSoldier );
						sAPCost += PlotPath( pSoldier, sActionGridNo, NO_COPYROUTE, FALSE, TEMPORARY, (UINT16)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints);

						if ( EnoughPoints( pSoldier, sAPCost, 0, TRUE ) )
						{
							DoMercBattleSound( pSoldier, BATTLE_SOUND_OK1 );

							// CHECK IF WE ARE AT THIS GRIDNO NOW
							if ( pSoldier->sGridNo != sActionGridNo )
							{
								// SEND PENDING ACTION
								pSoldier->ubPendingAction = MERC_ENTER_VEHICLE;
								pSoldier->sPendingActionData2  = pTSoldier->sGridNo;
								pSoldier->bPendingActionData3  = ubDirection;
								pSoldier->ubPendingActionAnimCount = 0;

								// WALK UP TO DEST FIRST
								EVENT_InternalGetNewSoldierPath( pSoldier, sActionGridNo, pSoldier->usUIMovementMode, 3 , pSoldier->fNoAPToFinishMove );
							}
							else
							{
								EVENT_SoldierEnterVehicle( pSoldier, pTSoldier->sGridNo, ubDirection );
							}

							// OK, set UI
							SetUIBusy( pSoldier->ubID );
							//guiPendingOverrideEvent = A_CHANGE_TO_MOVE;

							return( -1 );
						}
				 }
			 }
		 }
	}

	return( 0 );
}

void HandleHandCursorClick( UINT16 usMapPos, UINT32 *puiNewEvent )
{
	LEVELNODE					*pIntTile;
  INT16							sIntTileGridNo;
	INT16							sActionGridNo;
	UINT8							ubDirection;
	INT16							sAPCost;
	INT16							sAdjustedGridNo;
	STRUCTURE					*pStructure = NULL;
	BOOLEAN						fIgnoreItems = FALSE;

	SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
	if (pSoldier != NULL)
	{
	  // If we are out of breath, no cursor...
	  if ( pSoldier->bBreath < OKBREATH && pSoldier->bCollapsed )
	  {
		  return;
	  }

		if ( CheckForAndHandleHandleVehicleInteractiveClick( pSoldier, usMapPos, FALSE ) == -1 )
		{
			return;
		}

		// Check if we are on a merc... if so.. steal!
		const SOLDIERTYPE* const tgt = gUIFullTarget;
		if (tgt != NULL)
		{
			 if ( ( guiUIFullTargetFlags & ENEMY_MERC ) && !( guiUIFullTargetFlags & UNCONSCIOUS_MERC ) )
			 {
				sActionGridNo =  FindAdjacentGridEx(pSoldier, tgt->sGridNo, &ubDirection, &sAdjustedGridNo, TRUE, FALSE);
				if ( sActionGridNo == -1 )
				{
					sActionGridNo = sAdjustedGridNo;
				}

				// Steal!
				sAPCost = GetAPsToStealItem( pSoldier, sActionGridNo );

				if ( EnoughPoints( pSoldier, sAPCost, 0, TRUE ) )
				{
					MercStealFromMerc(pSoldier, tgt);

						*puiNewEvent = A_CHANGE_TO_MOVE;

						return;
				}
        else
        {
           return;
        }
			 }
		}

		// Default action gridno to mouse....
		sActionGridNo = usMapPos;

		// If we are over an interactive struct, adjust gridno to this....
		pIntTile = ConditionalGetCurInteractiveTileGridNoAndStructure( &sIntTileGridNo , &pStructure, FALSE );
		if ( pIntTile != NULL )
		{
			sActionGridNo = sIntTileGridNo;

			//if ( pStructure->fFlags & ( STRUCTURE_SWITCH | STRUCTURE_ANYDOOR ) )
			if ( pStructure->fFlags & ( STRUCTURE_SWITCH ) )
			{
				fIgnoreItems = TRUE;
			}

			if ( pStructure->fFlags & ( STRUCTURE_ANYDOOR ) && sActionGridNo != usMapPos )
			{
				fIgnoreItems = TRUE;
			}
		}

		// Check if we are over an item pool
		// ATE: Ignore items will be set if over a switch interactive tile...
		const ITEM_POOL* pItemPool = GetItemPool(sActionGridNo, pSoldier->bLevel);
		if (pItemPool != NULL && ITEMPOOL_VISIBLE(pItemPool) && !fIgnoreItems)
		{
			if ( AM_AN_EPC( pSoldier ) )
			{
				// Display message
				// ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ EPC_CANNOT_DO_THAT ] );
			}
			else if ( UIOkForItemPickup( pSoldier, sActionGridNo ) )
			{
				INT8 bZLevel;

				bZLevel = GetZLevelOfItemPoolGivenStructure( sActionGridNo, pSoldier->bLevel, pStructure );

				SoldierPickupItem( pSoldier, pItemPool->iItemIndex, sActionGridNo, bZLevel );

				*puiNewEvent = A_CHANGE_TO_MOVE;

			}
		}
		else
		{
			if ( pIntTile != NULL && !( pStructure->fFlags & STRUCTURE_HASITEMONTOP ) )
			{
				sActionGridNo =  FindAdjacentGridEx( pSoldier, sIntTileGridNo, &ubDirection, NULL, FALSE, TRUE );
				if ( sActionGridNo == -1 )
				{
					sActionGridNo = sIntTileGridNo;
				}

				// If this is not the same tile as ours, check if we can get to dest!
				if ( sActionGridNo != pSoldier->sGridNo && gsCurrentActionPoints == 0 )
				{
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
				}
				else
				{
					if ( SelectedMercCanAffordMove(  )  )
					{
						*puiNewEvent = C_MOVE_MERC;
					}
				}
			}
			else
			{
				// ATE: Here, the poor player wants to search something that does not exist...
				// Why should we not let them make fools of themselves....?
			  if ( AM_AN_EPC( pSoldier ) )
			  {
				  // Display message
				  // ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
				  ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ EPC_CANNOT_DO_THAT ] );
        }
        else
        {
				  // Check morale, if < threashold, refuse...
				  if ( pSoldier->bMorale < 30 )
				  {
					  TacticalCharacterDialogue( pSoldier, QUOTE_REFUSING_ORDER );
				  }
				  else
				  {
					  if ( gsCurrentActionPoints == 0 )
					  {
						  ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
					  }
					  else
					  {
						  SoldierPickupItem( pSoldier, NOTHING, sActionGridNo, 0 );
						  *puiNewEvent = A_CHANGE_TO_MOVE;
					  }
				  }
        }
			}
		}
	}
}


static void ExchangeMessageBoxCallBack(UINT8 bExitValue)
{
	if ( bExitValue == MSG_BOX_RETURN_YES )
	{
		SwapMercPositions( gpExchangeSoldier1, gpExchangeSoldier2 );
	}
}


INT8 HandleMoveModeInteractiveClick( UINT16 usMapPos, UINT32 *puiNewEvent )
{
	// Look for an item pool
	BOOLEAN						fContinue = TRUE;
	LEVELNODE					*pIntTile;
  INT16							sIntTileGridNo;
	INT16							sActionGridNo;
	UINT8							ubDirection;
	INT8							bReturnCode = 0;
	INT8							bZLevel;
	STRUCTURE					*pStructure = NULL;

	SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
	if (pSoldier != NULL)
	{
	  // If we are out of breath, no cursor...
	 // if ( pSoldier->bBreath < OKBREATH )
	  //{
	//	  return( -1 );
	  //}

		// ATE: If we are a vehicle, no moving!
		if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ VEHICLE_CANT_MOVE_IN_TACTICAL ] );
			return( -3 );
		}

		// OK, check for height differences.....
	  if ( gpWorldLevelData[ usMapPos ].sHeight != gpWorldLevelData[ pSoldier->sGridNo ].sHeight )
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ CANT_GET_THERE ] );
			return( -1 );
		}

		// See if we are over a vehicle, and walk up to it and enter....
		if ( CheckForAndHandleHandleVehicleInteractiveClick( pSoldier, usMapPos, TRUE ) == -1 )
		{
			return( -1 );
		}

		// Check if we are over a civillian....
		SOLDIERTYPE* const tgt = gUIFullTarget;
		if (tgt != NULL)
		{
			if ( ValidQuickExchangePosition( ) )
			{
				// Check if we can...
				if (CanExchangePlaces(pSoldier, tgt, TRUE))
				{
					 gpExchangeSoldier1 = pSoldier;
					gpExchangeSoldier2 = tgt;

					 // Do message box...
					 //DoMessageBox( MSG_BOX_BASIC_STYLE, TacticalStr[ EXCHANGE_PLACES_REQUESTER ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_YESNO, ExchangeMessageBoxCallBack, NULL );
					 SwapMercPositions( gpExchangeSoldier1, gpExchangeSoldier2 );
				}
			}
		  return( -3 );
		}

		pIntTile = GetCurInteractiveTileGridNoAndStructure( &sIntTileGridNo, &pStructure );

		if ( pIntTile != NULL )
		{
			bReturnCode = -3;

			// Check if we are over an item pool, take precedence over that.....
			// EXCEPT FOR SWITCHES!
			const ITEM_POOL* pItemPool = GetItemPool(sIntTileGridNo, pSoldier->bLevel);
			if (pItemPool != NULL && !(pStructure->fFlags & (STRUCTURE_SWITCH | STRUCTURE_ANYDOOR)))
			{
				if ( AM_AN_EPC( pSoldier ) )
				{
					// Display message
					// ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ EPC_CANNOT_DO_THAT ] );
					bReturnCode = -1;
				}
				else if ( UIOkForItemPickup( pSoldier, sIntTileGridNo ) )
				{
					bZLevel = GetLargestZLevelOfItemPool( pItemPool );

					if ( AnyItemsVisibleOnLevel( pItemPool, bZLevel ) )
					{
						fContinue = FALSE;

						SetUIBusy( pSoldier->ubID );

						if ( ( gTacticalStatus.uiFlags & INCOMBAT ) && ( gTacticalStatus.uiFlags & TURNBASED ) )
						{
							//puiNewEvent = C_WAIT_FOR_CONFIRM;
							SoldierPickupItem( pSoldier, pItemPool->iItemIndex, sIntTileGridNo, bZLevel );
						}
						else
						{
							BeginDisplayTimedCursor( OKHANDCURSOR_UICURSOR, 300 );

							SoldierPickupItem( pSoldier, pItemPool->iItemIndex, sIntTileGridNo, bZLevel );
						}
					}
				}
			}

			if ( fContinue )
			{
				SOLDIERTYPE* const sel = GetSelectedMan();
				sActionGridNo = FindAdjacentGridEx(sel, sIntTileGridNo, &ubDirection, NULL, FALSE, TRUE);
				if ( sActionGridNo == -1 )
				{
					sActionGridNo = sIntTileGridNo;
				}

				// If this is not the same tile as ours, check if we can get to dest!
				if (sActionGridNo != sel->sGridNo && gsCurrentActionPoints == 0)
				{
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
					bReturnCode = -1;
				}
				else
				{
					bReturnCode = -2;
				}
			}
		}
	}

	return( bReturnCode );
}


BOOLEAN HandleUIReloading(SOLDIERTYPE* pSoldier)
{
	INT8 bAPs = 0;

	// CHECK OUR CURRENT CURSOR...

	// Do we have the ammo to reload?
	if ( guiCurrentUICursor == GOOD_RELOAD_UICURSOR )
	{
		// Check APs to reload...
		bAPs = GetAPsToAutoReload( pSoldier );

		if ( EnoughPoints( pSoldier, bAPs, 0,TRUE ) )
		{
			// OK, we have some ammo we can reload.... reload now!
			if ( !AutoReload( pSoldier ) )
			{
				// Do we say we could not reload gun...?
			}

      // ATE: Re-examine cursor info!
      gfUIForceReExamineCursorData = TRUE;
		}
		return( TRUE );
	}

	if ( guiCurrentUICursor == BAD_RELOAD_UICURSOR )
	{
		// OK, we have been told to reload but have no ammo...
		//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, L"No ammo to reload." );
		if ( Random( 3 ) == 0 )
		{
			TacticalCharacterDialogue( pSoldier, QUOTE_OUT_OF_AMMO );
		}
		return( TRUE );
	}

	return( FALSE );
}


BOOLEAN ConfirmActionCancel(UINT16 usMapPos, UINT16 usOldMapPos)
{
	// OK, most times we want to leave confirm mode if our
	// gridno is different... but if we are in the grenade throw
	// confirm UI, we want a bigger radius...
	//if ( InAimCubeUI( ) )
	//{
		// Calculate distence between both gridnos.....
	//	if ( GetRangeFromGridNoDiff( GetInAimCubeUIGridNo( ), usOldMapPos ) > 1 )
		//if ( usMapPos != usOldMapPos )
	//	{
	//		return( TRUE );
	//	}
	//
	//else
	{
		if ( usMapPos != usOldMapPos )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}


static void ChangeCurrentSquad(INT32 iSquad)
{
	// only allow if nothing in hand and the Change Squad button for whichever panel we're in must be enabled
	if ( ( gpItemPointer == NULL ) && !gfDisableTacticalPanelButtons &&
			 ( ( gsCurInterfacePanel != TEAM_PANEL ) || ( ButtonList[ iTEAMPanelButtons[ CHANGE_SQUAD_BUTTON ] ]->uiFlags & BUTTON_ENABLED ) ) )
	{
		if ( IsSquadOnCurrentTacticalMap( iSquad ) )
		{
			SetCurrentSquad( iSquad, FALSE );
		}
	}
}


static void HandleSelectMercSlot(UINT8 ubPanelSlot, INT8 bCode)
{
	const UINT8 id = GetPlayerIDFromInterfaceTeamSlot(ubPanelSlot);
	if (id != NOBODY)
	{
		HandleLocateSelectMerc(id, bCode);
		ErasePath(TRUE);
		gfPlotNewMovement = TRUE;
	}
}


static void TestMeanWhile(INT32 iID)
{
	MEANWHILE_DEFINITION MeanwhileDef;
	INT32	cnt;
	SOLDIERTYPE *pSoldier;

	MeanwhileDef.sSectorX = 3;
	MeanwhileDef.sSectorY = 16;
	MeanwhileDef.ubNPCNumber = QUEEN;
	MeanwhileDef.usTriggerEvent = 0;
	MeanwhileDef.ubMeanwhileID = (UINT8)iID;

	if ( iID == INTERROGATION )
	{
		MeanwhileDef.sSectorX = 7;
		MeanwhileDef.sSectorY = 14;

		// Loop through our mercs and set gridnos once some found.....
		// look for all mercs on the same team,
		cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

		for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pSoldier++)
		{
			// Are we a POW in this sector?
			if ( pSoldier->bActive && pSoldier->bInSector )
			{

				ChangeSoldiersAssignment( pSoldier, ASSIGNMENT_POW );

				pSoldier->sSectorX = 7;
				pSoldier->sSectorY = 14;
			}
		}


	}

#ifndef JA2DEMO
	ScheduleMeanwhileEvent( &MeanwhileDef, 10 );
#endif
}


static void EscapeUILock(void)
{
	//UNLOCK UI
	UnSetUIBusy( (UINT8)gusSelectedSoldier );

	// Decrease global busy  counter...
	gTacticalStatus.ubAttackBusyCount = 0;

	guiPendingOverrideEvent = LU_ENDUILOCK;
	UIHandleLUIEndLock( NULL );
}

#ifdef JA2BETAVERSION

#include "Map_Edgepoints.h"


static void ToggleMapEdgepoints(void)
{
	#ifdef JA2EDITOR
		static BOOLEAN fToggleEdgepointDisplay = FALSE;
		if( fToggleEdgepointDisplay ^= TRUE )
		{ //Show edgepoints
			ShowMapEdgepoints();
		}
		else
		{ //Hide edgepoints
			HideMapEdgepoints();
		}
		SetRenderFlags( RENDER_FLAG_FULL );
	#endif
}
#endif

#ifdef JA2BETAVERSION

BOOLEAN gfMercsNeverQuit = FALSE;


static void ToggleMercsNeverQuit(void)
{
	if( gfMercsNeverQuit )
	{
		gfMercsNeverQuit = FALSE;
		ScreenMsg( FONT_RED, MSG_BETAVERSION, L"Merc contract expiring enabled." );
	}
	else
	{
		gfMercsNeverQuit ^= TRUE;
		ScreenMsg( FONT_RED, MSG_BETAVERSION, L"Merc contract expiring disabled." );
	}
}
#endif


void HandleStanceChangeFromUIKeys( UINT8 ubAnimHeight )
{
	// If we have multiple guys selected, make all change stance!
	SOLDIERTYPE *		pSoldier;
	INT32						cnt;

	if ( gTacticalStatus.fAtLeastOneGuyOnMultiSelect )
	{
		// OK, loop through all guys who are 'multi-selected' and
		// check if our currently selected guy is amoung the
		// lucky few.. if not, change to a guy who is...
		cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;
		for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++, pSoldier++ )
		{
			if ( pSoldier->bActive && pSoldier->bInSector )
			{
				if ( pSoldier->uiStatusFlags & SOLDIER_MULTI_SELECTED )
				{
					UIHandleSoldierStanceChange(pSoldier, ubAnimHeight);
				}
			}
		}
	}
	else
	{
		if( gusSelectedSoldier != NO_SOLDIER )
		{
			UIHandleSoldierStanceChange(GetSelectedMan(), ubAnimHeight);
		}
	}
}


static void ToggleStealthMode(SOLDIERTYPE* pSoldier)
{
	// nothing in hand and either not in SM panel, or the matching button is enabled if we are in SM panel
	if ( ( gsCurInterfacePanel != SM_PANEL ) || ( ButtonList[ giSMStealthButton ]->uiFlags & BUTTON_ENABLED ) )
  {
	  // ATE: Toggle stealth
	  if ( gpSMCurrentMerc != NULL && pSoldier->ubID == gpSMCurrentMerc->ubID )
	  {
		  gfUIStanceDifferent = TRUE;
	  }

	  pSoldier->bStealthMode = ! pSoldier->bStealthMode;
	  gfPlotNewMovement   = TRUE;
	  fInterfacePanelDirty = DIRTYLEVEL2;

	  if ( pSoldier->bStealthMode )
	  {
		  ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_MERC_ON_STEALTHMODE ], pSoldier->name );
	  }
	  else
	  {
		  ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_MERC_OFF_STEALTHMODE ], pSoldier->name );
	  }
  }
}


static void HandleStealthChangeFromUIKeys(void)
{
	// If we have multiple guys selected, make all change stance!
	SOLDIERTYPE *		pSoldier;
	INT32						cnt;

	if ( gTacticalStatus.fAtLeastOneGuyOnMultiSelect )
	{
		// OK, loop through all guys who are 'multi-selected' and
		// check if our currently selected guy is amoung the
		// lucky few.. if not, change to a guy who is...
		cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;
		for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++, pSoldier++ )
		{
			if ( pSoldier->bActive && !AM_A_ROBOT( pSoldier ) && pSoldier->bInSector )
			{
				if ( pSoldier->uiStatusFlags & SOLDIER_MULTI_SELECTED )
				{
					ToggleStealthMode( pSoldier );
				}
			}
		}
	}
	else
	{
		if( gusSelectedSoldier != NO_SOLDIER )
    {
			SOLDIERTYPE* const sel = GetSelectedMan();
			if (!AM_A_ROBOT(sel)) ToggleStealthMode(sel);
    }
	}
}


static void TestCapture(void)
{
	INT32 cnt;
	SOLDIERTYPE				*pSoldier;
	UINT32					uiNumChosen = 0;

	//StartQuest( QUEST_HELD_IN_ALMA, gWorldSectorX, gWorldSectorY );
	//EndQuest( QUEST_HELD_IN_ALMA, gWorldSectorX, gWorldSectorY );

	BeginCaptureSquence( );

	gStrategicStatus.uiFlags &= (~STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE );

	// loop through sodliers and pick 3 lucky ones....
	for ( cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID, pSoldier=MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID; cnt++, pSoldier++ )
	{
		if ( pSoldier->bLife >= OKLIFE && pSoldier->bActive && pSoldier->bInSector )
		{
			if ( uiNumChosen < 3 )
			{
				EnemyCapturesPlayerSoldier( pSoldier );

				// Remove them from tectical....
				RemoveSoldierFromGridNo( pSoldier );

				uiNumChosen++;
			}
		}
	}

	EndCaptureSequence( );
}


void PopupAssignmentMenuInTactical(void)
{
#ifndef JA2DEMO
	// do something
	fShowAssignmentMenu = TRUE;
	CreateDestroyAssignmentPopUpBoxes( );
	SetTacticalPopUpAssignmentBoxXY( );
	DetermineBoxPositions( );
	DetermineWhichAssignmentMenusCanBeShown( );
	fFirstClickInAssignmentScreenMask = TRUE;
	gfIgnoreScrolling = TRUE;
#endif
}
