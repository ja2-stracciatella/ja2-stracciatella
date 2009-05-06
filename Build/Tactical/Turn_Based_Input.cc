#include "Handle_Items.h"
#include "Real_Time_Input.h"
#include "Soldier_Find.h"
#include "Structure.h"
#include "TileDat.h"
#include "Turn_Based_Input.h"
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
#include "Sound_Control.h"
#include "Environment.h"
#include "Music_Control.h"
#include "AI.h"
#include "Font_Control.h"
#include "WorldMan.h"
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
#include "GameLoop.h"
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

#ifdef JA2TESTVERSION
#	include "Ambient_Control.h"
#	ifndef JA2DEMO
#		include "MapScreen.h"
#	endif
#endif

#ifdef JA2BETAVERSION
#	include "Strategic_AI.h"
#endif

#ifdef SGP_VIDEO_DEBUGGING
#	include "VObject.h"
#endif


static BOOLEAN gfFirstCycleMovementStarted = FALSE;

const SOLDIERTYPE* gUITargetSoldier = NULL;


UINT8			gubCheatLevel		= STARTING_CHEAT_LEVEL;

static char const cheat_code[] =
#ifdef GERMAN
	"iguana";
#else
	"gabbi";
#endif


static void QueryTBLeftButton(UINT32* puiNewEvent);
static void QueryTBRightButton(UINT32* puiNewEvent);


void	GetTBMouseButtonInput( UINT32 *puiNewEvent )
{
	 QueryTBLeftButton( puiNewEvent );
	 QueryTBRightButton( puiNewEvent );
}


static void QueryTBLeftButton(UINT32* puiNewEvent)
{
	static BOOLEAN	fClickHoldIntercepted = FALSE;
	static BOOLEAN  fCanCheckForSpeechAdvance = FALSE;

	// LEFT MOUSE BUTTON
  if ( gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		const GridNo usMapPos = GetMouseMapPos();
		if (usMapPos == NOWHERE && !gfUIShowExitSouth) return;

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
							SOLDIERTYPE* const sel = GetSelectedMan();
							if (sel != NULL) sel->sStartGridNo = usMapPos;
							break;
						}

						case MOVE_MODE:
							if (g_ui_message_overlay != NULL)
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
										// We're on terrain in which we can walk, walk
										// If we're on terrain,
										const SOLDIERTYPE* const sel = GetSelectedMan();
										if (sel != NULL)
										{
											INT8 const bReturnVal = HandleMoveModeInteractiveClick(usMapPos);
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
														const BOOLEAN fResult = UIOKMoveDestination(sel, usMapPos);
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
														else if (fResult == 2)
														{
															ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NOBODY_USING_REMOTE_STR]);
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
								SOLDIERTYPE* const sel = GetSelectedMan();
								if (sel != NULL && sel->bDoBurst)
								{
									sel->sEndGridNo = usMapPos;

									gfBeginBurstSpreadTracking = FALSE;

									if (sel->sEndGridNo != sel->sStartGridNo)
									{
										sel->fDoSpread = TRUE;
										PickBurstLocations(sel);
										*puiNewEvent = CA_MERC_SHOOT;
									}
									else
									{
										sel->fDoSpread = FALSE;
									}

									fClickHoldIntercepted = TRUE;
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
							if (g_ui_message_overlay != NULL)
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
													const SOLDIERTYPE* const sel = GetSelectedMan();
													if (sel != NULL && EnoughPoints(sel, gsCurrentActionPoints, 0, TRUE))
													{
														*puiNewEvent = JP_JUMP;
													}
													break;
												}

												case ACTION_MODE:
												{
													SOLDIERTYPE* const sel = GetSelectedMan();
													if (sel != NULL && !HandleUIReloading(sel))
													{
														// ATE: Reset refine aim..
														sel->bShownAimTime = 0;

														if (gsCurrentActionPoints == 0)
														{
															ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NO_PATH]);
														}
														// Determine if we have enough action points!
														else if (UIMouseOnValidAttackLocation(sel) && SelectedMercCanAffordAttack())
														{
															*puiNewEvent      = A_CHANGE_TO_CONFIM_ACTION;
															sel->sStartGridNo = usMapPos;
														}
													}
													break;
												}

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
			SOLDIERTYPE* const sel = GetSelectedMan();
			if (sel != NULL) sel->fDoSpread = FALSE;
			gfBeginBurstSpreadTracking = FALSE;
		}
	}

}


static void QueryTBRightButton(UINT32* puiNewEvent)
{
	static BOOLEAN	fClickHoldIntercepted = FALSE;
	static BOOLEAN	fClickIntercepted = FALSE;

	const GridNo usMapPos = GetMouseMapPos();
	if (usMapPos == NOWHERE) return;

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
								if (guiUIFullTargetFlags & OWNED_MERC && !(guiUIFullTargetFlags & UNCONSCIOUS_MERC))
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

								if (GetSelectedMan() != NULL && !fClickHoldIntercepted)
								{
									*puiNewEvent = U_MOVEMENT_MENU;
									fClickHoldIntercepted = TRUE;
								}
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
							SOLDIERTYPE* const sel = GetSelectedMan();
							if (sel != NULL)
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
									sel->fDoSpread = FALSE;
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
							SOLDIERTYPE* const sel = GetSelectedMan();
							if (sel != NULL)
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
										HandleRightClickAdjustCursor(sel, usMapPos);
										fClickIntercepted = TRUE;
										break;

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
	static const SOLDIERTYPE* MoveTargetSoldier = NULL;

	static UINT16			usOldMapPos = 0;
	BOOLEAN						bHandleCode;
	static BOOLEAN		fOnValidGuy = FALSE;

	const GridNo usMapPos = GetMouseMapPos();
	if (usMapPos == NOWHERE) return;

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
				if (MoveTargetSoldier != NULL && gUIFullTarget != MoveTargetSoldier)
				{
					*puiNewEvent = A_CHANGE_TO_MOVE;
					return;
				}
			 *puiNewEvent = T_ON_TERRAIN;
				break;

			case MOVE_MODE:
			{
				MoveTargetSoldier = NULL;

				// Check for being on terrain
				const SOLDIERTYPE* const sel = GetSelectedMan();
				if (sel != NULL)
				{
					 if (IsValidJumpLocation(sel, usMapPos, TRUE))
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
									MoveTargetSoldier = tgt;
									if (IsValidTalkableNPC(tgt, FALSE, FALSE, FALSE) && !_KeyDown(SHIFT) && !AM_AN_EPC(sel) && !ValidQuickExchangePosition())
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
				gUITargetSoldier = NULL;

				fOnValidGuy = FALSE;

				const SOLDIERTYPE* const tgt = gUIFullTarget;
				if (tgt != NULL)
				{
					if (IsValidTargetMerc(tgt))
					{
						gUITargetSoldier = tgt;

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
				SOLDIERTYPE* const sel = GetSelectedMan();
				if (sel != NULL && sel->bDoBurst)
				{
					sel->sEndGridNo = usMapPos;

					if (sel->sEndGridNo != sel->sStartGridNo && fLeftButtonDown)
					{
						sel->fDoSpread = TRUE;
						gfBeginBurstSpreadTracking = TRUE;
					}

					if (sel->fDoSpread)
					{
						// Accumulate gridno
						AccumulateBurstLocation(usMapPos);

						*puiNewEvent = CA_ON_TERRAIN;
						break;
					}
				}

				// First check if we are on a guy, if so, make selected if it's ours
				if (gUIFullTarget != NULL)
				{
					if (gUITargetSoldier != gUIFullTarget)
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
	//static BOOLEAN fShifted = FALSE;
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
					if (!fCtrlDown)
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
					if (!fAltDown)
					{
						// Get currently selected guy and change reverse....
						if (GetSelectedMan() != NULL)
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
					if (GetSelectedMan() != NULL)
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

	SGPPoint MousePos;
	GetMousePos(&MousePos);

	const GridNo usMapPos = GetMouseMapPos();

	while (DequeueEvent(&InputEvent))
  {
		MouseSystemHook(InputEvent.usEvent, MousePos.iX, MousePos.iY);

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
			if (g_ui_message_overlay != NULL)
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
			SOLDIERTYPE* const sel = GetSelectedMan();
			if (sel != NULL) sel->fDoSpread = FALSE;

			// Befone anything, delete popup box!
			EndUIMessage( );

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
			  if (sel != NULL)
			  {
				  // If soldier is not stationary, stop
					StopSoldier(sel);
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
							HideStructOfGivenType(x, usType, !fShowRoofs);
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

			for ( i = 0; i < 1000; i++ )
			{
				const SOLDIERTYPE* const sel = GetSelectedMan();
				INT8 ubLevel;
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

			if (fCtrl && !fAlt)
			{
				if (gubCheatLevel < lengthof(cheat_code) - 1)
				{
					if (InputEvent.usParam == cheat_code[gubCheatLevel])
					{
						if (++gubCheatLevel == lengthof(cheat_code) - 1)
						{
							ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[MSG_CHEAT_LEVEL_TWO]);
							AddHistoryToPlayersLog(HISTORY_CHEAT_ENABLED, 0, GetWorldTotalMin(), -1, -1);
						}
						continue;
					}
				}
				else if (gubCheatLevel == lengthof(cheat_code) - 1 && InputEvent.usParam == 'b')
				{
					++gubCheatLevel;
					continue;
				}
			}

			if (gubCheatLevel < lengthof(cheat_code) - 1) RESET_CHEAT_LEVEL();

			switch( InputEvent.usParam )
			{
				case SDLK_SPACE:
					// nothing in hand and either not in SM panel, or the matching button is enabled if we are in SM panel
					if ( !( gTacticalStatus.uiFlags & ENGAGED_IN_CONV )  &&
							(gsCurInterfacePanel != SM_PANEL || iSMPanelButtons[NEXTMERC_BUTTON]->uiFlags & BUTTON_ENABLED))
					{
						if ( !InKeyRingPopup( ) )
						{
							SOLDIERTYPE* const sel = GetSelectedMan();
							if (sel != NULL)
							{
								if (_KeyDown(SHIFT))
								{
									// only allow if nothing in hand and if in SM panel, the Change Squad button must be enabled
									if (gsCurInterfacePanel != TEAM_PANEL ||
											iTEAMPanelButtons[CHANGE_SQUAD_BUTTON]->uiFlags & BUTTON_ENABLED)
									{
										//Select next squad
										const INT32        iCurrentSquad = CurrentSquad();
										SOLDIERTYPE* const pNewSoldier   = FindNextActiveSquad(sel);
										if ( pNewSoldier->bAssignment != iCurrentSquad )
										{
											HandleLocateSelectMerc(pNewSoldier, LOCATEANDSELECT_MERC);

											ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_SQUAD_ACTIVE ], ( CurrentSquad( ) + 1 ) );

											// Center to guy....
											LocateSoldier(GetSelectedMan(), SETLOCATOR);
										}
									}
								}
								else
								{
									//Select next merc
									SOLDIERTYPE* const next = FindNextMercInTeamPanel(sel);
									HandleLocateSelectMerc(next, LOCATEANDSELECT_MERC);

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
							(gsCurInterfacePanel != SM_PANEL || iSMPanelButtons[UPDOWN_BUTTON]->uiFlags & BUTTON_ENABLED))
					{
						UIHandleChangeLevel( NULL );
					}
					break;

				case SDLK_F1:
				case SDLK_F2:
				case SDLK_F3:
				case SDLK_F4:
				case SDLK_F5:
				case SDLK_F6:
				{
					UINT const idx = InputEvent.usParam - SDLK_F1;
					if (fShift)
					{
						HandleSelectMercSlot(idx, LOCATE_MERC_ONCE);
					}
#if defined JA2TESTVERSION
					else if (fAlt)
					{
						TestMeanWhile(idx != 0 ? idx : 15);
					}
					else if (fCtrl)
					{
						TestMeanWhile(10 + idx);
					}
#endif
					else
					{
						HandleSelectMercSlot(idx, LOCATEANDSELECT_MERC);
					}
					break;
				}

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
						SOLDIERTYPE* const sel = GetSelectedMan();
						if (sel != NULL)
						{
							if (sel->ubProfile == LARRY_NORMAL)
							{
								// Change guy to drunk larry
								ForceSoldierProfileID(sel, LARRY_DRUNK);
							}
							else
							{
								// Change guy to normal larry
								ForceSoldierProfileID(sel, LARRY_NORMAL);
							}

							// Dirty interface
							DirtyMercPanelInterface(sel, DIRTYLEVEL2);
						}
#endif
					}

					else
					{
						if( DEBUG_CHEAT_LEVEL( ) )
						{
							gsQdsEnteringGridNo = GetMouseMapPos();
							LeaveTacticalScreen( QUEST_DEBUG_SCREEN );
						}
					}
					break;

				case SDLK_F12:
#ifdef JA2TESTVERSION
					if( fAlt )
					{
						UINT8 ubProfile = TONY;

						gsQdsEnteringGridNo = GetMouseMapPos();
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
						SOLDIERTYPE* const pSoldier1 = GetSelectedMan();
						if (pSoldier1 != NULL)
						{
							SOLDIERTYPE* const pSoldier2 = gUIFullTarget;
							if (pSoldier2 != NULL)
							{
								// Check if both OK....
								if (pSoldier1->bLife >= OKLIFE && pSoldier2 != pSoldier1)
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
				{
					// Center to guy....
					SOLDIERTYPE* const sel = GetSelectedMan();
					if (sel != NULL) LocateSoldier(sel, 10);
					break;
				}

				case 'a':

					if ( fCtrl )
					{
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
						//gGameSettings.fOptions[ TOPTION_HIDE_BULLETS ] ^= TRUE;
					}
					else
					{
						// nothing in hand and either not in SM panel, or the matching button is enabled if we are in SM panel
						if ( ( gpItemPointer == NULL ) &&
								(gsCurInterfacePanel != SM_PANEL || iSMPanelButtons[BURSTMODE_BUTTON]->uiFlags & BUTTON_ENABLED))
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
									(
										(gsCurInterfacePanel == SM_PANEL   && iSMPanelButtons[SM_DONE_BUTTON]->uiFlags     & BUTTON_ENABLED) ||
										(gsCurInterfacePanel == TEAM_PANEL && iTEAMPanelButtons[TEAM_DONE_BUTTON]->uiFlags & BUTTON_ENABLED)
									))
							{
								if( fAlt )
								{
									if ( CHEATER_CHEAT_LEVEL( ) )
									{
										FOR_ALL_IN_TEAM(s, gbPlayerNum)
										{
											if (s->bLife > 0)
											{
												// Get APs back...
												CalcNewActionPoints(s);
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
						SOLDIERTYPE* const sel = GetSelectedMan();
            if (sel != NULL)
            {
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
						//if there is a selected soldier, and the cursor location is valid
						const SOLDIERTYPE* const sel = GetSelectedMan();
						if (sel != NULL)
						{
							GridNo sGridNo = GetMouseMapPos();
							//if the cursor is over someone
							if (gUIFullTarget != NULL) sGridNo = gUIFullTarget->sGridNo;
							DisplayRangeToTarget(sel, sGridNo);
						}
					}
					break;

				case 'g':

					if( fCtrl )
					{
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
				{
					SOLDIERTYPE* const sel = GetSelectedMan();
					if (sel != NULL && CheckForMercContMove(sel))
					{
						// Continue
						ContinueMercMovement(sel);
						ErasePath(TRUE);
					}
					break;
				}

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
									(gsCurInterfacePanel != SM_PANEL || iSMPanelButtons[LOOK_BUTTON]->uiFlags & BUTTON_ENABLED))
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
								(
									(gsCurInterfacePanel == SM_PANEL   && iSMPanelButtons[SM_MAP_SCREEN_BUTTON]->uiFlags     & BUTTON_ENABLED) ||
									(gsCurInterfacePanel == TEAM_PANEL && iTEAMPanelButtons[TEAM_MAP_SCREEN_BUTTON]->uiFlags & BUTTON_ENABLED)
								))
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
						SOLDIERTYPE* const sel = GetSelectedMan();
						if (sel != NULL && gpItemPointer == NULL) GotoLowerStance(sel);
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
						SOLDIERTYPE* const sel = GetSelectedMan();
						if (sel != NULL && gpItemPointer == NULL) GotoHeigherStance(sel);
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
								(gsCurInterfacePanel != SM_PANEL || iSMPanelButtons[OPTIONS_BUTTON]->uiFlags & BUTTON_ENABLED))
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
				{
					SOLDIERTYPE* const sel = GetSelectedMan();
					if (sel != NULL)
					{
						if( fAlt ) //reload selected merc's weapon
						{
							if ( CHEATER_CHEAT_LEVEL( ) )
							{
								ReloadWeapon(sel, sel->ubAttackingHand);
							}
						}
						else
						{
							if (!MercInWater(sel) && !(sel->uiStatusFlags & SOLDIER_ROBOT))
              {
							  //change selected merc to run
								if (sel->usUIMovementMode != WALKING && sel->usUIMovementMode != RUNNING)
							  {
									UIHandleSoldierStanceChange(sel, ANIM_STAND);
							  }
							  else
							  {
									sel->usUIMovementMode = RUNNING;
									gfPlotNewMovement     = TRUE;
							  }
								sel->fUIMovementFast = 1;
              }
						}
					}
					break;
				}

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
								DoMessageBox(MSG_BOX_BASIC_STYLE, zNewTacticalMessages[TCTL_MSG__IRON_MAN_CANT_SAVE_NOW], GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL);
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
								DoMessageBox(MSG_BOX_BASIC_STYLE, zNewTacticalMessages[TCTL_MSG__IRON_MAN_CANT_SAVE_NOW], GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL);
							}
						}
					}
					else if (GetSelectedMan() != NULL)
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
						if (!(gTacticalStatus.uiFlags & INCOMBAT))
						{
							FOR_ALL_IN_TEAM(s, gbPlayerNum)
							{
								// Check if this guy is OK to control....
								if (OkControllableMerc(s) &&
										!(s->uiStatusFlags & (SOLDIER_VEHICLE | SOLDIER_PASSENGER | SOLDIER_DRIVER)))
								{
									s->uiStatusFlags |= SOLDIER_MULTI_SELECTED;
								}
							}
							EndMultiSoldierSelection(TRUE);
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
						if (CHEATER_CHEAT_LEVEL() && GetSelectedMan() != NULL)
						{
							FOR_ALL_IN_TEAM(s, gbPlayerNum)
							{
								if (s->bLife > 0)
								{
									// Get breath back
									s->bBreath   = s->bBreathMax;
									// Get life back
									s->bLife     = s->bLifeMax;
									s->bBleeding = 0;

									fInterfacePanelDirty = DIRTYLEVEL2;
								}
							}
						}
					}
					else if (GetSelectedMan() != NULL)
					{
						*puiNewEvent = M_CHANGE_TO_ACTION;
					}
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
							SOLDIERTYPE* const sel = GetSelectedMan();
							if (sel != NULL) CreateItem(FLAMETHROWER, 100, &sel->inv[HANDPOS]);
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

						SOLDIERTYPE* const robot = FindSoldierByProfileID(ROBOT);
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
							BOOLEAN						fStealthOn = FALSE;

							// Check if at least one guy is on stealth....
							CFOR_ALL_IN_TEAM(s, gbPlayerNum)
							{
								if (OkControllableMerc(s) &&
										s->bAssignment == CurrentSquad() &&
										s->bStealthMode)
								{
									fStealthOn = TRUE;
								}
							}

							fStealthOn = !fStealthOn;

							FOR_ALL_IN_TEAM(s, gbPlayerNum)
							{
								if (OkControllableMerc(s) &&
										s->bAssignment == CurrentSquad() &&
										!AM_A_ROBOT(s))
								{
									if (gpSMCurrentMerc == s)
									{
										gfUIStanceDifferent = TRUE;
									}

									s->bStealthMode = fStealthOn;
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
							gTacticalStatus.bRealtimeSpeed = MAX( 1, gTacticalStatus.bRealtimeSpeed - 1 );
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
  					MusicSetVolume(MIN(vol + 20, MAXVOLUME));
					}
					else if( fCtrl )
					{
						gTacticalStatus.bRealtimeSpeed = MIN( MAX_REALTIME_SPEED_VAL, gTacticalStatus.bRealtimeSpeed+1 );
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
	// If not in move mode, return!
	if ( gCurrentUIMode != MOVE_MODE )
	{
		return( FALSE );
	}

	const SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL) return FALSE;

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
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, pExitingSectorHelpText[EXIT_GUI_ESCORTED_CHARACTERS_CANT_LEAVE_SECTOR_ALONE_STR], sel->name);
			gfLoneEPCAttemptingTraversal = FALSE;
		}
		else if( gubLoneMercAttemptingToAbandonEPCs )
		{
			wchar_t str[256];
			if( gubLoneMercAttemptingToAbandonEPCs == 1 )
			{ //Use the singular version of the string
				if (gMercProfiles[sel->ubProfile ].bSex == MALE)
				{ //male singular
					swprintf(str, lengthof(str), pExitingSectorHelpText[EXIT_GUI_MERC_CANT_ISOLATE_EPC_HELPTEXT_MALE_SINGULAR], sel->name, gPotentiallyAbandonedEPC->name);
				}
				else
				{ //female singular
					swprintf(str, lengthof(str), pExitingSectorHelpText[EXIT_GUI_MERC_CANT_ISOLATE_EPC_HELPTEXT_FEMALE_SINGULAR], sel->name, gPotentiallyAbandonedEPC->name);
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
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[MERC_IS_TOO_FAR_AWAY_STR], sel->name);
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
				const GridNo sMapPos = GetMouseMapPos();
				if (sMapPos == NOWHERE) return FALSE;

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
	const GridNo usMapPos = GetMouseMapPos();
	if (usMapPos != NOWHERE)
	{
		CreateItem( (UINT16) (Random( 35 ) + 1), 100, &Object );
		AddItemToPool(usMapPos, &Object, INVISIBLE, 0, 0, 0);
	}
}


static void ToggleViewAllMercs(void)
{ // Set option to show all mercs
	gTacticalStatus.uiFlags ^= SHOW_ALL_MERCS;
	SetRenderFlags(RENDER_FLAG_FULL);
}


static void ToggleViewAllItems(void)
{
	gTacticalStatus.uiFlags ^= SHOW_ALL_ITEMS;
	ToggleItemGlow(gGameSettings.fOptions[TOPTION_GLOW_ITEMS]);
	SetRenderFlags(RENDER_FLAG_FULL);
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
	UINT8& show_wireframe = gGameSettings.fOptions[TOPTION_TOGGLE_WIREFRAME];
	show_wireframe = !show_wireframe;
	wchar_t const* const msg = show_wireframe ?
		pMessageStrings[MSG_WIREFRAMES_ADDED] :
		pMessageStrings[MSG_WIREFRAMES_REMOVED];
	ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, msg);
	SetRenderFlags(RENDER_FLAG_FULL);
}


static void RefreshSoldier(void)
{
	// CHECK IF WE'RE ON A GUY ( EITHER SELECTED, OURS, OR THEIRS
	SOLDIERTYPE* const tgt = gUIFullTarget;
	if (tgt != NULL) ReviveSoldier(tgt);
}


static void ChangeSoldiersBodyType(UINT8 ubBodyType, BOOLEAN fCreateNewPalette)
{
	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL) return;

	sel->ubBodyType = ubBodyType;
	EVENT_InitNewSoldierAnim(sel, STANDING, 0 , TRUE);
	if (fCreateNewPalette)
	{
		CreateSoldierPalettes(sel);

		switch (ubBodyType)
		{
			case ADULTFEMALEMONSTER:
			case AM_MONSTER:
			case YAF_MONSTER:
			case YAM_MONSTER:
			case LARVAE_MONSTER:
			case INFANT_MONSTER:
			case QUEENMONSTER:
				sel->uiStatusFlags |= SOLDIER_MONSTER;
				memset(&sel->inv, 0, sizeof(OBJECTTYPE) * NUM_INV_SLOTS);
				AssignCreatureInventory(sel);
				CreateItem(CREATURE_YOUNG_MALE_SPIT, 100, &sel->inv[HANDPOS]);
				break;

			case TANK_NW:
			case TANK_NE:
				sel->uiStatusFlags |= SOLDIER_VEHICLE;
				//sel->inv[HANDPOS].usItem = TANK_CANNON;
				sel->inv[HANDPOS].usItem = MINIMI;
				sel->bVehicleID = AddVehicleToList(sel->sSectorX, sel->sSectorY, sel->bSectorZ, HUMMER);
				break;
		}
	}
}


static void TeleportSelectedSoldier(void)
{
	// CHECK IF WE'RE ON A GUY ( EITHER SELECTED, OURS, OR THEIRS
	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL) return;

	const GridNo usMapPos = GetMouseMapPos();
	if (usMapPos == NOWHERE) return;

	// Check level first....
	if (gsInterfaceLevel == 0)
	{
		SetSoldierHeight(sel, 0);
		TeleportSoldier(sel, usMapPos, FALSE);
		EVENT_StopMerc(sel, sel->sGridNo, sel->bDirection);
	}
	else if (FindStructure(usMapPos, STRUCTURE_ROOF) != NULL) // Is there a roof?
	{
		SetSoldierHeight(sel, 50.0);
		TeleportSoldier(sel, usMapPos, TRUE);
		EVENT_StopMerc(sel, sel->sGridNo, sel->bDirection);
	}
}


static void ToggleTreeTops(void)
{
	UINT8& show_trees = gGameSettings.fOptions[TOPTION_TOGGLE_TREE_TOPS];
	show_trees = !show_trees;
	wchar_t const* const msg = show_trees ?
		TacticalStr[SHOWING_TREETOPS_STR] :
		TacticalStr[REMOVING_TREETOPS_STR];
	ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, msg);
	SetTreeTopStateForMap();
}


static void ToggleZBuffer(void)
{
	UINT32& flags = gTacticalStatus.uiFlags;
	flags ^= SHOW_Z_BUFFER;
	if (!(flags & SHOW_Z_BUFFER)) SetRenderFlags(SHOW_Z_BUFFER);
}


static void SetBurstMode(void)
{
	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel != NULL) ChangeWeaponMode(sel);
}


static void ObliterateSector(void)
{
	#ifdef JA2BETAVERSION
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Obliterating Sector!" );
	#endif

	FOR_ALL_NON_PLAYER_SOLDIERS(pTSoldier)
	{
		if (!pTSoldier->bNeutral && pTSoldier->bSide != gbPlayerNum)
		{
				//	ANITILE_PARAMS	AniParams;
			//		memset( &AniParams, 0, sizeof( ANITILE_PARAMS ) );
			//		AniParams.sGridNo							= pTSoldier->sGridNo;
			//		AniParams.ubLevelID						= ANI_STRUCT_LEVEL;
				//	AniParams.usTileIndex					= FIRSTEXPLOSION1;
				//	AniParams.sDelay							= 80;
				//	AniParams.sStartFrame					= 0;
				//	AniParams.uiFlags							= ANITILE_FORWARD;

				//	CreateAnimationTile( &AniParams );
					//PlayJA2Sample(EXPLOSION_1, MIDVOLUME, 1, MIDDLEPAN);

			EVENT_SoldierGotHit(pTSoldier, 0, 400, 0, pTSoldier->bDirection, 320, NULL, FIRE_WEAPON_NO_SPECIAL, pTSoldier->bAimShotLocation, NOWHERE);
		}
	}
}


static void CreateNextCivType(void)
{
	static INT8 bBodyType = FATCIV;

	const GridNo usMapPos = GetMouseMapPos();
	if (usMapPos == NOWHERE) return;

	SOLDIERCREATE_STRUCT MercCreateStruct;
	memset(&MercCreateStruct, 0, sizeof(MercCreateStruct));
	MercCreateStruct.ubProfile  = NO_PROFILE;
	MercCreateStruct.sSectorX   = gWorldSectorX;
	MercCreateStruct.sSectorY   = gWorldSectorY;
	MercCreateStruct.bSectorZ   = gbWorldSectorZ;
	MercCreateStruct.bBodyType  = bBodyType;
	MercCreateStruct.bDirection = SOUTH;

	if (++bBodyType > KIDCIV) bBodyType = FATCIV;

	MercCreateStruct.bTeam            = CIV_TEAM;
	MercCreateStruct.sInsertionGridNo = usMapPos;
	RandomizeNewSoldierStats(&MercCreateStruct);

	SOLDIERTYPE* const s = TacticalCreateSoldier(MercCreateStruct);
	if (s != NULL)
	{
		AddSoldierToSector(s);

		// So we can see them!
		AllTeamsLookForAll(NO_INTERRUPTS);
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
		CreatePhysicalObject(&Object, 60, sX * CELL_X_SIZE, sY * CELL_Y_SIZE, 256, -20, 20, 158, NULL, THROW_ARM_ITEM, 0);
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
		CreatePhysicalObject(&Object, 60, sX * CELL_X_SIZE, sY * CELL_Y_SIZE, 256, 0, -30, 158, NULL, THROW_ARM_ITEM, 0);
	}
}


static void CreatePlayerControlledMonster(void)
{
	const GridNo usMapPos = GetMouseMapPos();
	if (usMapPos == NOWHERE) return;

	SOLDIERCREATE_STRUCT MercCreateStruct;
	memset(&MercCreateStruct, 0, sizeof(MercCreateStruct));
	MercCreateStruct.ubProfile        = NO_PROFILE;
	MercCreateStruct.sSectorX         = gWorldSectorX;
	MercCreateStruct.sSectorY         = gWorldSectorY;
	MercCreateStruct.bSectorZ         = gbWorldSectorZ;
	//Note:  only gets called if Alt and/or Ctrl isn't pressed!
	MercCreateStruct.bBodyType        = (_KeyDown(SDLK_INSERT) ? QUEENMONSTER : ADULTFEMALEMONSTER);
	MercCreateStruct.bTeam            = CREATURE_TEAM;
	MercCreateStruct.sInsertionGridNo = usMapPos;
	RandomizeNewSoldierStats(&MercCreateStruct);

	SOLDIERTYPE* const s = TacticalCreateSoldier(MercCreateStruct);
	if (s != NULL) AddSoldierToSector(s);
}


static bool CheckForAndHandleHandleVehicleInteractiveClick(SOLDIERTYPE* const s, UINT16 const usMapPos, BOOLEAN const fMovementMode)
{
	SOLDIERTYPE const* const tgt = gUIFullTarget;
	if (!tgt)                          return false;
	if (!OK_ENTERABLE_VEHICLE(tgt))    return false;
	if (tgt->bVisible == -1)           return false;
	if (!OKUseVehicle(tgt->ubProfile)) return false;

	VEHICLETYPE* const v = GetVehicle(tgt->bVehicleID);
	if (GetNumberInVehicle(v) != 0 && fMovementMode) return false;

	// Find a gridno closest to sweetspot
	GridNo const action_pos = FindGridNoFromSweetSpotWithStructDataFromSoldier(s, s->usUIMovementMode, 5, 0, tgt);
	if (action_pos == NOWHERE) return false;

	// Calculate AP costs
	INT16 const ap_cost = PlotPath(s, action_pos, NO_COPYROUTE, FALSE, s->usUIMovementMode, s->bActionPoints);
	if (!EnoughPoints(s, ap_cost, 0, TRUE)) return false;

	DoMercBattleSound(s, BATTLE_SOUND_OK1);

	// Check if we are at this gridno now
	if (s->sGridNo != action_pos)
	{
		// Send pending action
		s->ubPendingAction          = MERC_ENTER_VEHICLE;
		s->sPendingActionData2      = tgt->sGridNo;
		s->ubPendingActionAnimCount = 0;
		// Walk up to dest first
		EVENT_InternalGetNewSoldierPath(s, action_pos, s->usUIMovementMode, 3, s->fNoAPToFinishMove);
		SetUIBusy(s);
	}
	else
	{
		PutSoldierInVehicle(s, v);
	}
	return true;
}


void HandleHandCursorClick( UINT16 usMapPos, UINT32 *puiNewEvent )
{
	LEVELNODE					*pIntTile;
  INT16							sIntTileGridNo;
	INT16							sActionGridNo;
	INT16							sAPCost;
	INT16							sAdjustedGridNo;
	STRUCTURE					*pStructure = NULL;
	BOOLEAN						fIgnoreItems = FALSE;

	SOLDIERTYPE* const pSoldier = GetSelectedMan();
	if (pSoldier != NULL)
	{
	  // If we are out of breath, no cursor...
	  if ( pSoldier->bBreath < OKBREATH && pSoldier->bCollapsed )
	  {
		  return;
	  }

		if (CheckForAndHandleHandleVehicleInteractiveClick(pSoldier, usMapPos, FALSE))
		{
			return;
		}

		// Check if we are on a merc... if so.. steal!
		const SOLDIERTYPE* const tgt = gUIFullTarget;
		if (tgt != NULL)
		{
			 if ( ( guiUIFullTargetFlags & ENEMY_MERC ) && !( guiUIFullTargetFlags & UNCONSCIOUS_MERC ) )
			 {
				sActionGridNo = FindAdjacentGridEx(pSoldier, tgt->sGridNo, NULL, &sAdjustedGridNo, TRUE, FALSE);
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
		if (pItemPool && IsItemPoolVisible(pItemPool) && !fIgnoreItems)
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

				SoldierPickupItem(pSoldier, -1, sActionGridNo, bZLevel);

				*puiNewEvent = A_CHANGE_TO_MOVE;

			}
		}
		else
		{
			if ( pIntTile != NULL && !( pStructure->fFlags & STRUCTURE_HASITEMONTOP ) )
			{
				sActionGridNo = FindAdjacentGridEx(pSoldier, sIntTileGridNo, NULL, NULL, FALSE, TRUE);
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
						  SoldierPickupItem(pSoldier, -1, sActionGridNo, 0);
						  *puiNewEvent = A_CHANGE_TO_MOVE;
					  }
				  }
        }
			}
		}
	}
}


INT8 HandleMoveModeInteractiveClick(UINT16 const usMapPos)
{
	SOLDIERTYPE* const sel = GetSelectedMan();
	if (!sel) return 0;

	// ATE: If we are a vehicle, no moving!
	if (sel->uiStatusFlags & SOLDIER_VEHICLE)
	{
		ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[VEHICLE_CANT_MOVE_IN_TACTICAL]);
		return -3;
	}

	// OK, check for height differences
	if (gpWorldLevelData[usMapPos].sHeight != gpWorldLevelData[sel->sGridNo].sHeight)
	{
		ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[CANT_GET_THERE]);
		return -1;
	}

	// See if we are over a vehicle, and walk up to it and enter
	if (CheckForAndHandleHandleVehicleInteractiveClick(sel, usMapPos, TRUE))
	{
		return -1;
	}

	// Check if we are over a civillian
	SOLDIERTYPE* const tgt = gUIFullTarget;
	if (tgt)
	{
		if (ValidQuickExchangePosition() &&
				CanExchangePlaces(sel, tgt, TRUE))
		{
			SwapMercPositions(sel, tgt);
		}
		return -3;
	}

  INT16            sIntTileGridNo;
	STRUCTURE*       pStructure;
	LEVELNODE* const pIntTile = GetCurInteractiveTileGridNoAndStructure(&sIntTileGridNo, &pStructure);
	if (!pIntTile) return 0;

	// Check if we are over an item pool, take precedence over that, except for switches
	ITEM_POOL const* const pItemPool = GetItemPool(sIntTileGridNo, sel->bLevel);
	if (pItemPool && !(pStructure->fFlags & (STRUCTURE_SWITCH | STRUCTURE_ANYDOOR)))
	{
		if (AM_AN_EPC(sel))
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[EPC_CANNOT_DO_THAT]);
		}
		else if (UIOkForItemPickup(sel, sIntTileGridNo))
		{
			INT8 const bZLevel = GetLargestZLevelOfItemPool(pItemPool);
			if (AnyItemsVisibleOnLevel(pItemPool, bZLevel))
			{
				SetUIBusy(sel);
				if (!(gTacticalStatus.uiFlags & INCOMBAT) && !(gTacticalStatus.uiFlags & TURNBASED))
				{
					BeginDisplayTimedCursor(OKHANDCURSOR_UICURSOR, 300);
				}
				SoldierPickupItem(sel, -1, sIntTileGridNo, bZLevel);
				return -3;
			}
		}
	}

	INT16 sActionGridNo = FindAdjacentGridEx(sel, sIntTileGridNo, NULL, NULL, FALSE, TRUE);
	if (sActionGridNo == -1) sActionGridNo = sIntTileGridNo;

	// If this is not the same tile as ours, check if we can get to dest!
	if (sActionGridNo != sel->sGridNo && gsCurrentActionPoints == 0)
	{
		ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NO_PATH]);
		return -1;
	}
	else
	{
		return -2;
	}
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


BOOLEAN ConfirmActionCancel(UINT16 const usMapPos, UINT16 const usOldMapPos)
{
	return usMapPos != usOldMapPos;
}


static void ChangeCurrentSquad(INT32 iSquad)
{
	// only allow if nothing in hand and the Change Squad button for whichever panel we're in must be enabled
	if ( ( gpItemPointer == NULL ) && !gfDisableTacticalPanelButtons &&
			(gsCurInterfacePanel != TEAM_PANEL || iTEAMPanelButtons[CHANGE_SQUAD_BUTTON]->uiFlags & BUTTON_ENABLED))
	{
		if ( IsSquadOnCurrentTacticalMap( iSquad ) )
		{
			SetCurrentSquad( iSquad, FALSE );
		}
	}
}


static void HandleSelectMercSlot(UINT8 ubPanelSlot, INT8 bCode)
{
	SOLDIERTYPE* const s = GetPlayerFromInterfaceTeamSlot(ubPanelSlot);
	if (s != NULL)
	{
		HandleLocateSelectMerc(s, bCode);
		ErasePath(TRUE);
		gfPlotNewMovement = TRUE;
	}
}


static void TestMeanWhile(INT32 iID)
{
	INT16 x;
	INT16 y;
	if ( iID == INTERROGATION )
	{
		x =  7;
		y = 14;

		// Loop through our mercs and set gridnos once some found.....
		FOR_ALL_IN_TEAM(s, gbPlayerNum)
		{
			if (s->bInSector)
			{
				ChangeSoldiersAssignment(s, ASSIGNMENT_POW);
				s->sSectorX = 7;
				s->sSectorY = 14;
			}
		}
	}
	else
	{
		x =  3;
		y = 16;
	}

#ifndef JA2DEMO
	ScheduleMeanwhileEvent(x, y, 0, iID, QUEEN, 10);
#endif
}


static void EscapeUILock(void)
{
	//UNLOCK UI
	UnSetUIBusy(GetSelectedMan());

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


void HandleStanceChangeFromUIKeys( UINT8 ubAnimHeight )
{
	// If we have multiple guys selected, make all change stance!
	if ( gTacticalStatus.fAtLeastOneGuyOnMultiSelect )
	{
		FOR_ALL_IN_TEAM(s, gbPlayerNum)
		{
			if (s->bInSector && s->uiStatusFlags & SOLDIER_MULTI_SELECTED)
			{
				UIHandleSoldierStanceChange(s, ubAnimHeight);
			}
		}
	}
	else
	{
		SOLDIERTYPE* const sel = GetSelectedMan();
		if (sel != NULL) UIHandleSoldierStanceChange(sel, ubAnimHeight);
	}
}


static void ToggleStealthMode(SOLDIERTYPE* pSoldier)
{
	// nothing in hand and either not in SM panel, or the matching button is enabled if we are in SM panel
	if (gsCurInterfacePanel != SM_PANEL || giSMStealthButton->uiFlags & BUTTON_ENABLED)
  {
	  // ATE: Toggle stealth
	  if (pSoldier == gpSMCurrentMerc) gfUIStanceDifferent = TRUE;

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
	if ( gTacticalStatus.fAtLeastOneGuyOnMultiSelect )
	{
		FOR_ALL_IN_TEAM(s, gbPlayerNum)
		{
			if (!AM_A_ROBOT(s) &&
					s->bInSector &&
					s->uiStatusFlags & SOLDIER_MULTI_SELECTED)
			{
				ToggleStealthMode(s);
			}
		}
	}
	else
	{
		SOLDIERTYPE* const sel = GetSelectedMan();
		if (sel != NULL && !AM_A_ROBOT(sel)) ToggleStealthMode(sel);
	}
}


static void TestCapture(void)
{
	UINT32					uiNumChosen = 0;

	//StartQuest( QUEST_HELD_IN_ALMA, gWorldSectorX, gWorldSectorY );
	//EndQuest( QUEST_HELD_IN_ALMA, gWorldSectorX, gWorldSectorY );

	BeginCaptureSquence( );

	gStrategicStatus.uiFlags &= (~STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE );

	// loop through sodliers and pick 3 lucky ones....
	FOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		if (s->bLife >= OKLIFE && s->bInSector && uiNumChosen < 3)
		{
			EnemyCapturesPlayerSoldier(s);

			// Remove them from tectical....
			RemoveSoldierFromGridNo(s);

			uiNumChosen++;
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
