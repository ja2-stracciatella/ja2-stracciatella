#include "Font_Control.h"
#include "Real_Time_Input.h"
#include "Soldier_Find.h"
#include "Turn_Based_Input.h"
#include "PathAI.h"
#include "Soldier_Control.h"
#include "Animation_Control.h"
#include "Animation_Data.h"
#include "Timer_Control.h"
#include "Handle_UI.h"
#include "Isometric_Utils.h"
#include "Input.h"
#include "Overhead.h"
#include "Interface.h"
#include "Cursor_Control.h"
#include "Points.h"
#include "Interactive_Tiles.h"
#include "Interface_Cursors.h"
#include "Environment.h"
#include "OppList.h"
#include "Spread_Burst.h"
#include "Overhead_Map.h"
#include "World_Items.h"
#include "Game_Clock.h"
#include "Interface_Items.h"
#include "UI_Cursors.h"
#include "StrategicMap.h"
#include "Soldier_Profile.h"
#include "Soldier_Create.h"
#include "Soldier_Add.h"
#include "Dialogue_Control.h"
#include "Interface_Dialogue.h"
#include "Tile_Animation.h"
#include "Merc_Entering.h"
#include "Explosion_Control.h"
#include "Message.h"
#include "Strategic_Exit_GUI.h"
#include "Assignments.h"
#include "Map_Screen_Interface.h"
#include "RenderWorld.h"
#include "GameSettings.h"
#include "English.h"
#include "Text.h"
#include "Soldier_Macros.h"
#include "Render_Dirty.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "policy/GamePolicy.h"

static BOOLEAN gfStartLookingForRubberBanding = FALSE;
static UINT16  gusRubberBandX                 = 0;
static UINT16  gusRubberBandY                 = 0;


BOOLEAN	gfBeginBurstSpreadTracking = FALSE;

BOOLEAN	gfRTClickLeftHoldIntercepted = FALSE;
static BOOLEAN gfRTHaveClickedRightWhileLeftDown = FALSE;


static void QueryRTLeftButton(UIEventKind* puiNewEvent);
static void QueryRTRightButton(UIEventKind* puiNewEvent);
static void QueryRTMiddleButton(UIEventKind* puiNewEvent);


void GetRTMouseButtonInput(UIEventKind* puiNewEvent)
{
	QueryRTLeftButton( puiNewEvent );
	QueryRTRightButton( puiNewEvent );
	QueryRTMiddleButton( puiNewEvent );
}


static void QueryRTMiddleButton(UIEventKind* const puiNewEvent)
{
	//static BOOLEAN fClickHoldIntercepted = FALSE;
	//static BOOLEAN fClickIntercepted = FALSE;
	static UINT32  uiSingleClickTime;
	static BOOLEAN fDoubleClickIntercepted = FALSE;
	static BOOLEAN fValidDoubleClickPossible = FALSE;

	if ( gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		const GridNo usMapPos = GetMouseMapPos();
		if (usMapPos == NOWHERE) return;

		// MIDDLE MOUSE BUTTON
		if ( gViewportRegion.ButtonState & MSYS_MIDDLE_BUTTON )
		{
			if ( !fMiddleButtonDown )
			{
				fMiddleButtonDown = TRUE;
				RESETCOUNTER( MMOUSECLICK_DELAY_COUNTER );
			}
		}
		else
		{
			if ( fMiddleButtonDown )
			{
				// OK , FOR DOUBLE CLICKS - TAKE TIME STAMP & RECORD EVENT
				if ( ( GetJA2Clock() - uiSingleClickTime ) < 300 )
				{
					// CHECK HERE FOR DOUBLE CLICK EVENTS
					if ( fValidDoubleClickPossible )
					{
						fDoubleClickIntercepted = TRUE;

						// Do stuff....
					}
				}

				// Capture time!
				uiSingleClickTime = GetJA2Clock();

				fValidDoubleClickPossible = TRUE;

				if ( !fDoubleClickIntercepted )
				{
					// CHECK COMBINATIONS ETC...

					if (gamepolicy(middle_mouse_look)) *puiNewEvent = LC_LOOK;
				}

				// Reset flag
				fMiddleButtonDown = FALSE;
				//fClickHoldIntercepted = FALSE;
				//fClickIntercepted = FALSE;
				fDoubleClickIntercepted = FALSE;


				// Reset counter
				RESETCOUNTER( MMOUSECLICK_DELAY_COUNTER );
			}
		}
	}
}

static void QueryRTLeftButton(UIEventKind* const puiNewEvent)
{
	static UINT32 uiSingleClickTime;
	static BOOLEAN fDoubleClickIntercepted = FALSE;
	static BOOLEAN fValidDoubleClickPossible = FALSE;
	static BOOLEAN fCanCheckForSpeechAdvance = FALSE;

	SOLDIERTYPE* const sel = GetSelectedMan();

	// LEFT MOUSE BUTTON
	if ( gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		if (sel != NULL && sel->pTempObject != NULL) return;

		const GridNo usMapPos = GetMouseMapPos();
		if (usMapPos == NOWHERE && !gfUIShowExitSouth) return;

		if ( gViewportRegion.ButtonState & MSYS_LEFT_BUTTON )
		{
			if ( !fLeftButtonDown )
			{
				fLeftButtonDown = TRUE;
				gfRTHaveClickedRightWhileLeftDown = FALSE;
				RESETCOUNTER( LMOUSECLICK_DELAY_COUNTER );

				if (g_ui_message_overlay == NULL)
				{
					if ( gpItemPointer == NULL )
					{
						switch( gCurrentUIMode )
						{
							case ACTION_MODE:
								if (sel != NULL && gpItemPointer == NULL)
								{
									// OK, check for needing ammo
									if (HandleUIReloading(sel))
									{
										gfRTClickLeftHoldIntercepted = TRUE;
										//fLeftButtonDown              = FALSE;
									}
									else
									{
										if (sel->bDoBurst)
										{
											sel->sStartGridNo = usMapPos;
											ResetBurstLocations();
											*puiNewEvent = A_CHANGE_TO_CONFIM_ACTION;
										}
										else
										{
											gfRTClickLeftHoldIntercepted = TRUE;

											if (UIMouseOnValidAttackLocation(sel))
											{
												// OK< going into confirm will call a function that will automatically move
												// us to shoot in most vases ( grenades need a confirm mode regardless )
												*puiNewEvent = A_CHANGE_TO_CONFIM_ACTION;
												//*puiNewEvent = CA_MERC_SHOOT;
											}
										}
									}
								}
								break;


							case MOVE_MODE:

								gfUICanBeginAllMoveCycle = TRUE;

								if ( !HandleCheckForExitArrowsInput( FALSE ) &&
									gpItemPointer == NULL )
								{
									if (gUIFullTarget != NULL && guiUIFullTargetFlags & OWNED_MERC)
									{
										// Reset , if this guy is selected merc, reset any multi selections...
										if (gUIFullTarget == sel)
										{
											ResetMultiSelection( );
										}
									}
									else
									{
										INT8 const bReturnCode = HandleMoveModeInteractiveClick(usMapPos);
										if ( bReturnCode == -1 )
										{
											//gfRTClickLeftHoldIntercepted = TRUE;
										}
										else if ( bReturnCode == -2 )
										{
											//if ( gGameSettings.fOptions[ TOPTION_RTCONFIRM ] )
											//{
											//	*puiNewEvent = C_WAIT_FOR_CONFIRM;
											//	gfPlotNewMovement = TRUE;
											//}/
											//else
										}
										else if ( bReturnCode == -3 )
										{
											gfRTClickLeftHoldIntercepted = TRUE;
										}
										else if ( bReturnCode == 0 )
										{
											if (sel != NULL)
											{
												if (UIOKMoveDestination(sel, usMapPos) == 1)
												{
													if ( gsCurrentActionPoints != 0 )
													{
														// We're on terrain in which we can walk, walk
														// If we're on terrain,
														if ( !gGameSettings.fOptions[ TOPTION_RTCONFIRM ]  )
														{
															*puiNewEvent = C_WAIT_FOR_CONFIRM;
															gfPlotNewMovement = TRUE;
														}
													}
												}
											}
										}
									}
									//gfRTClickLeftHoldIntercepted = TRUE;
								}
								else
								{
									gfRTClickLeftHoldIntercepted = TRUE;
									fIgnoreLeftUp = TRUE;
								}

								break;
							default:
								break;
							}
						}
				}
				if ( gfUIWaitingForUserSpeechAdvance )
				{
					fCanCheckForSpeechAdvance = TRUE;
				}

			}

			if ( gpItemPointer == NULL )
			{
				if (g_ui_message_overlay == NULL && !gfRTHaveClickedRightWhileLeftDown)
				{
					// HERE FOR CLICK-DRAG CLICK
					switch( gCurrentUIMode )
					{
						case MOVE_MODE:
						case CONFIRM_MOVE_MODE:
						{
							// First check if we clicked on a guy, if so, make selected if it's ours
							const SOLDIERTYPE* const s = FindSoldierFromMouse();
							if (s != NULL)
							{
								// Select guy
								if (s == sel && s->bLife >= OKLIFE && !(s->uiStatusFlags & SOLDIER_VEHICLE))
								{
									*puiNewEvent = M_CHANGE_TO_ADJPOS_MODE;
								}
							}
							else
							{
								// OK, change to rubber banding mode..
								// Have we started this yet?
								if ( !gfStartLookingForRubberBanding && !gRubberBandActive )
								{
									gfStartLookingForRubberBanding = TRUE;
									gusRubberBandX = gusMouseXPos;
									gusRubberBandY = gusMouseYPos;
								}
								else
								{
									// Have we moved....?
									if (std::abs(gusMouseXPos - gusRubberBandX) > 10 || std::abs(gusMouseYPos - gusRubberBandY) > 10)
									{
										gfStartLookingForRubberBanding = FALSE;

										// Stop scrolling:
										gfIgnoreScrolling = TRUE;

										// Anchor cursor....
										RestrictMouseToXYXY( 0, 0, gsVIEWPORT_END_X, gsVIEWPORT_WINDOW_END_Y );

										// OK, settup anchor....
										gRubberBandRect.iLeft = gusRubberBandX;
										gRubberBandRect.iTop = gusRubberBandY;

										gRubberBandActive = TRUE;

										// ATE: If we have stopped scrolling.....
										if (g_scroll_inertia)
										{
											SetRenderFlags( RENDER_FLAG_FULL | RENDER_FLAG_CHECKZ );

											// Restore Interface!
											RestoreInterface( );

											DeleteVideoOverlaysArea( );

											g_scroll_inertia = false;
										}

										*puiNewEvent = RB_ON_TERRAIN;
										return;
									}
								}
							}
							break;
						}
							default:
								break;
					}
				}
			}
		}
		else
		{
			if ( fLeftButtonDown  )
			{
				if ( !fIgnoreLeftUp )
				{
					// set flag for handling single clicks
					// OK , FOR DOUBLE CLICKS - TAKE TIME STAMP & RECORD EVENT
					if ( ( GetJA2Clock() - uiSingleClickTime ) < 300 )
					{
						// CHECK HERE FOR DOUBLE CLICK EVENTS
						if ( fValidDoubleClickPossible )
						{
							if ( gpItemPointer == NULL )
							{
									fDoubleClickIntercepted = TRUE;

									// First check if we clicked on a guy, if so, make selected if it's ours
									if (sel != NULL)
									{
										// Set movement mode
										// OK, only change this if we are stationary!
										//if (gAnimControl[s->usAnimState].uiFlags & ANIM_STATIONARY)
										//if (sel->usAnimState == WALKING)
										{
											sel->fUIMovementFast = TRUE;
											*puiNewEvent = C_MOVE_MERC;
										}
									}
							}
						}

					}

					// Capture time!
					uiSingleClickTime = GetJA2Clock();

					fValidDoubleClickPossible = FALSE;

					if ( !fDoubleClickIntercepted )
					{
						// FIRST CHECK FOR ANYTIME ( NON-INTERVAL ) CLICKS
						switch( gCurrentUIMode )
						{
							case ADJUST_STANCE_MODE:

								// If button has come up, change to mocve mode
								*puiNewEvent = PADJ_ADJUST_STANCE;
								break;
							default:
								break;
						}

						// CHECK IF WE CLICKED-HELD
						if ( COUNTERDONE( LMOUSECLICK_DELAY_COUNTER ) && gpItemPointer != NULL )
						{
							// LEFT CLICK-HOLD EVENT
							// Switch on UI mode
							switch( gCurrentUIMode )
							{
								case CONFIRM_ACTION_MODE:
								case ACTION_MODE:
									if (sel != NULL && sel->bDoBurst)
									{
										sel->sEndGridNo = usMapPos;

										gfBeginBurstSpreadTracking = FALSE;

										if (sel->sEndGridNo != sel->sStartGridNo )
										{
											sel->fDoSpread = TRUE;
											PickBurstLocations(sel);
											*puiNewEvent = CA_MERC_SHOOT;
										}
										else
										{
											sel->fDoSpread = FALSE;
										}
										gfRTClickLeftHoldIntercepted = TRUE;
									}
									break;
								default:
									break;
							}
						}
						//else
						{
							//LEFT CLICK NORMAL EVENT
							// Switch on UI mode
							if ( !gfRTClickLeftHoldIntercepted )
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
											if ( gfUIWaitingForUserSpeechAdvance && fCanCheckForSpeechAdvance )
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
														{
															// First check if we clicked on a guy, if so, make selected if it's ours
															const SOLDIERTYPE* const s = FindSoldierFromMouse();
															if (s && IsOwnedMerc(*s))
															{
																*puiNewEvent = I_SELECT_MERC;
															}
															break;
														}

													case HANDCURSOR_MODE:

															HandleHandCursorClick( usMapPos, puiNewEvent );
															break;

														case ACTION_MODE:
															*puiNewEvent = CA_MERC_SHOOT;
															break;

														case CONFIRM_MOVE_MODE:
															if (sel != NULL)
															{
																if (sel->usAnimState == RUNNING)
																{
																	sel->fUIMovementFast = 2;
																}
																*puiNewEvent = C_MOVE_MERC;
															}

															//*puiNewEvent = C_MOVE_MERC;

															//if ( gGameSettings.fOptions[ TOPTION_RTCONFIRM ] )
															{
																fValidDoubleClickPossible = TRUE;
															}
															break;

														case CONFIRM_ACTION_MODE:
															*puiNewEvent = CA_MERC_SHOOT;
															break;

														case MOVE_MODE:

															if ( !HandleCheckForExitArrowsInput( FALSE ) && gpItemPointer == NULL )
															{
																// First check if we clicked on a guy, if so, make selected if it's ours
																if (gUIFullTarget != NULL && guiUIFullTargetFlags & OWNED_MERC)
																{
																	if ( !( guiUIFullTargetFlags & UNCONSCIOUS_MERC ) )
																	{
																		// Select guy
																		SOLDIERTYPE* const pSoldier = gUIFullTarget;
																		if (pSoldier != NULL && gpItemPointer == NULL)
																		{
																			if( pSoldier->bAssignment >= ON_DUTY && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
																			{
																				PopupAssignmentMenuInTactical();
																			}
																			else
																			{
																				if ( !_KeyDown( ALT ) )
																				{
																					ResetMultiSelection( );
																					*puiNewEvent = I_SELECT_MERC;
																				}
																				else
																				{
																					if ( pSoldier->uiStatusFlags & SOLDIER_MULTI_SELECTED )
																					{
																						pSoldier->uiStatusFlags &= (~SOLDIER_MULTI_SELECTED );
																					}
																					else
																					{
																						pSoldier->uiStatusFlags |= (SOLDIER_MULTI_SELECTED );
																						// Say Confimation...
																						if( !gGameSettings.fOptions[ TOPTION_MUTE_CONFIRMATIONS ] )
																							DoMercBattleSound( pSoldier, BATTLE_SOUND_ATTN1 );

																						// OK, if we have a selected guy.. make him part too....
																						if (sel != NULL)
																						{
																							sel->uiStatusFlags |= SOLDIER_MULTI_SELECTED;
																						}
																					}

																					gfIgnoreOnSelectedGuy = TRUE;


																					EndMultiSoldierSelection( FALSE );
																				}
																			}
																		}
																		else
																		{
																			if ( !_KeyDown( ALT ) )
																			{
																				ResetMultiSelection( );
																				*puiNewEvent = I_SELECT_MERC;
																			}
																			else
																			{
																				if ( pSoldier->uiStatusFlags & SOLDIER_MULTI_SELECTED )
																				{
																					pSoldier->uiStatusFlags &= (~SOLDIER_MULTI_SELECTED );
																				}
																				else
																				{
																					pSoldier->uiStatusFlags |= (SOLDIER_MULTI_SELECTED );
																					// Say Confimation...
																					if( !gGameSettings.fOptions[ TOPTION_MUTE_CONFIRMATIONS ] )
																						DoMercBattleSound( pSoldier, BATTLE_SOUND_ATTN1 );
																				}

																				// OK, if we have a selected guy.. make him part too....
																				if (sel != NULL)
																				{
																					sel->uiStatusFlags |= SOLDIER_MULTI_SELECTED;
																				}

																				gfIgnoreOnSelectedGuy = TRUE;

																				EndMultiSoldierSelection( FALSE );
																			}
																		}
																	}
																	gfRTClickLeftHoldIntercepted = TRUE;
																}
																else
																{
																	INT8 const bReturnCode = HandleMoveModeInteractiveClick(usMapPos);
																	if ( bReturnCode == -1 )
																	{
																		gfRTClickLeftHoldIntercepted = TRUE;
																	}
																	else if ( bReturnCode == -2 )
																	{
																		//if ( gGameSettings.fOptions[ TOPTION_RTCONFIRM ] )
																		//{
																		//	*puiNewEvent = C_WAIT_FOR_CONFIRM;
																		//	gfPlotNewMovement = TRUE;
																		//}/
																		//else
																		//{
																			if (sel != NULL)
																			{
																				BeginDisplayTimedCursor( GetInteractiveTileCursor( guiCurrentUICursor, TRUE ), 300 );

																				if (sel->usAnimState != RUNNING)
																				{
																					*puiNewEvent = C_MOVE_MERC;
																				}
																				else if (GetCurInteractiveTile() != NULL)
																				{
																					sel->fUIMovementFast = TRUE;
																					*puiNewEvent = C_MOVE_MERC;
																				}
																				fValidDoubleClickPossible = TRUE;
																			}
																		//}
																	}
																	else if ( bReturnCode == 0 )
																	{
																		if (sel != NULL)
																		{
																			// First check if we clicked on a guy, if so, make selected if it's ours
																			const SOLDIERTYPE* const s = FindSoldierFromMouse();
																			if (s && IsOwnedMerc(*s))
																			{
																				// Select guy
																				*puiNewEvent = I_SELECT_MERC;
																				gfRTClickLeftHoldIntercepted = TRUE;
																			}
																			else
																			{
																				if ( gsCurrentActionPoints == 0 && !gfUIAllMoveOn && !gTacticalStatus.fAtLeastOneGuyOnMultiSelect )
																				{
																					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
																					gfRTClickLeftHoldIntercepted = TRUE;
																				}
																				else
																				{
																					const BOOLEAN fResult = UIOKMoveDestination(sel, usMapPos);
																					if (fResult == 1)
																					{
																						if (gfUIAllMoveOn)
																						{
																							// ATE: Select everybody in squad and make move!
																							// Make move!
																							*puiNewEvent = C_MOVE_MERC;
																							fValidDoubleClickPossible = TRUE;
																						}
																						else
																						{
																							// We're on terrain in which we can walk, walk
																							// If we're on terrain,
																							if (gGameSettings.fOptions[TOPTION_RTCONFIRM])
																							{
																								*puiNewEvent = C_WAIT_FOR_CONFIRM;
																								gfPlotNewMovement = TRUE;
																							}
																							else
																							{
																								*puiNewEvent = C_MOVE_MERC;
																								fValidDoubleClickPossible = TRUE;
																							}
																						}
																					}
																					else
																					{
																						if (fResult == 2)
																						{
																							ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NOBODY_USING_REMOTE_STR]);
																						}
																						gfRTClickLeftHoldIntercepted = TRUE;
																					}
																				}
																			}
																		}
																	}
																}
																//gfRTClickLeftHoldIntercepted = TRUE;
															}
															else
															{
																gfRTClickLeftHoldIntercepted = TRUE;
															}
															break;

														case LOOKCURSOR_MODE:
															// If we cannot actually do anything, return to movement mode
															*puiNewEvent = LC_LOOK;
															break;

														case JUMPOVER_MODE:

															*puiNewEvent = JP_JUMP;
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

														case RUBBERBAND_MODE:

															EndRubberBanding( );
															*puiNewEvent = A_CHANGE_TO_MOVE;
															break;

														default:
															break;
													}
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
				gfRTClickLeftHoldIntercepted = FALSE;
				fDoubleClickIntercepted = FALSE;
				fCanCheckForSpeechAdvance = FALSE;
				gfStartLookingForRubberBanding = FALSE;

				// Reset counter
				RESETCOUNTER( LMOUSECLICK_DELAY_COUNTER );

			}

		}

	}
	else
	{
		// Set mouse down to false
		//fLeftButtonDown = FALSE;

		//fCanCheckForSpeechAdvance = FALSE;

		// OK, handle special cases like if we are dragging and holding for a burst spread and
		// release mouse over another mouse region
		if ( gfBeginBurstSpreadTracking )
		{
			if (sel != NULL) sel->fDoSpread = FALSE;
			gfBeginBurstSpreadTracking = FALSE;
		}
	}
}


static void QueryRTRightButton(UIEventKind* const puiNewEvent)
{
	static BOOLEAN fClickHoldIntercepted = FALSE;
	static BOOLEAN fClickIntercepted = FALSE;
	static UINT32  uiSingleClickTime;
	static BOOLEAN fDoubleClickIntercepted = FALSE;
	static BOOLEAN fValidDoubleClickPossible = FALSE;

	if ( gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		const GridNo usMapPos = GetMouseMapPos();
		if (usMapPos == NOWHERE) return;

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
				//fIgnoreLeftUp = TRUE;
				gfRTHaveClickedRightWhileLeftDown = TRUE;

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

								if ( !gfUIAllMoveOn )
								{
									fValidDoubleClickPossible = TRUE;

									// OK, our first right-click is an all-cycle
									if ( gfUICanBeginAllMoveCycle )
									{
										// ATE: Here, check if we can do this....
										if (!UIOKMoveDestination(sel, usMapPos))
										{
											ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ CANT_MOVE_THERE_STR ] );
											gfRTClickLeftHoldIntercepted = TRUE;
										}
										//else if ( gsCurrentActionPoints == 0 )
										//{
										//	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
										//	gfRTClickLeftHoldIntercepted = TRUE;
										//}
										else
										{
											*puiNewEvent = M_CYCLE_MOVE_ALL;
										}
									}
									fClickHoldIntercepted = TRUE;
								}
							default:
								break;
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
				// IF HERE, DO A CLICK-HOLD IF IN INTERVAL
				if ( COUNTERDONE( RMOUSECLICK_DELAY_COUNTER ) && !fClickHoldIntercepted )
				{
					if ( gpItemPointer == NULL )
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
							{
								const SOLDIERTYPE* const tgt = gUIFullTarget;
								if (tgt != NULL &&
									!(tgt->uiStatusFlags & SOLDIER_VEHICLE) &&
									guiUIFullTargetFlags & OWNED_MERC &&
									guiUIFullTargetFlags & VISIBLE_MERC &&
									!(guiUIFullTargetFlags & DEAD_MERC))
								{
									PopupAssignmentMenuInTactical();
									fClickHoldIntercepted = TRUE;
								}
								else if (GetSelectedMan() != NULL)
								{
									fShowAssignmentMenu = FALSE;
									CreateDestroyAssignmentPopUpBoxes();
									DetermineWhichAssignmentMenusCanBeShown();

									// ATE:
									if ( !fClickHoldIntercepted )
									{
										*puiNewEvent = U_MOVEMENT_MENU;
										fClickHoldIntercepted = TRUE;
									}
								}
								break;
							}
							default:
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
				// OK , FOR DOUBLE CLICKS - TAKE TIME STAMP & RECORD EVENT
				if ( ( GetJA2Clock() - uiSingleClickTime ) < 300 )
				{
					// CHECK HERE FOR DOUBLE CLICK EVENTS
					if ( fValidDoubleClickPossible )
					{
						fDoubleClickIntercepted = TRUE;

						// Do stuff....
						// OK, check if left button down...
						if ( fLeftButtonDown )
						{
							if ( gpItemPointer == NULL )
							{
								if ( !fClickIntercepted && !fClickHoldIntercepted )
								{
									// ATE:
									if (GetSelectedMan() != NULL)
									{
										//fIgnoreLeftUp = TRUE;
										switch( gCurrentUIMode )
										{
											case CONFIRM_MOVE_MODE:
											case MOVE_MODE:
												if ( gfUIAllMoveOn )
												{
													// OK, now we wish to run!
													gfUIAllMoveOn = 2;
												}
											default:
												break;
										}
									}

								}
							}
						}
					}
				}

				// Capture time!
				uiSingleClickTime = GetJA2Clock();

				fValidDoubleClickPossible = TRUE;

				if ( !fDoubleClickIntercepted )
				{
					// CHECK COMBINATIONS
					if ( fLeftButtonDown )
					{
						if ( gpItemPointer == NULL )
						{
							if ( !fClickHoldIntercepted && !fClickIntercepted )
							{
								// ATE:
								if (GetSelectedMan() != NULL)
								{
									//fIgnoreLeftUp = TRUE;
									switch( gCurrentUIMode )
									{
										case CONFIRM_MOVE_MODE:
										case MOVE_MODE:

											if ( gfUIAllMoveOn )
											{
												gfUIAllMoveOn = FALSE;
												gfUICanBeginAllMoveCycle = TRUE;
											}
										default:
											break;

									}
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

										case CONFIRM_MOVE_MODE:
										case MOVE_MODE:
										case TALKCURSOR_MODE:
											// We have here a change to action mode
											*puiNewEvent = M_CHANGE_TO_ACTION;
											fClickIntercepted = TRUE;
											break;

										case ACTION_MODE:
											// We have here a change to move mode
											*puiNewEvent = A_END_ACTION;
											fClickIntercepted = TRUE;
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

										case HANDCURSOR_MODE:
											// If we cannot actually do anything, return to movement mode
											*puiNewEvent = A_CHANGE_TO_MOVE;
											break;

										case LOOKCURSOR_MODE:

											// If we cannot actually do anything, return to movement mode
											*puiNewEvent = A_CHANGE_TO_MOVE;
											break;

										default:
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
				fDoubleClickIntercepted = FALSE;


				// Reset counter
				RESETCOUNTER( RMOUSECLICK_DELAY_COUNTER );
			}
		}
	}
}


void GetRTMousePositionInput(UIEventKind* const puiNewEvent)
{
	static const SOLDIERTYPE* MoveTargetSoldier = NULL;

	static UINT16 usOldMapPos = 0;

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
			case RUBBERBAND_MODE:
				// ATE: Make sure!
				if (!gRubberBandActive)
				{
					*puiNewEvent = M_ON_TERRAIN;
				}
				else
				{
					*puiNewEvent = RB_ON_TERRAIN;
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

			case LOCKUI_MODE:
				*puiNewEvent = LU_ON_TERRAIN;
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

				if ( HandleOpenDoorMenu( ) )
				{
					*puiNewEvent = A_CHANGE_TO_MOVE;
				}
				break;

			case HANDCURSOR_MODE:

				*puiNewEvent = HC_ON_TERRAIN;
				break;

			case MOVE_MODE:
			{
				if ( usMapPos != usOldMapPos )
				{
					// Set off ALL move....
					gfUIAllMoveOn = FALSE;
				}

				MoveTargetSoldier = NULL;

				// Check for being on terrain
				const SOLDIERTYPE* const sel = GetSelectedMan();
				if (sel != NULL)
				{
					// get cursor for item
					ItemCursor const ubItemCursor = GetActionModeCursor(sel);

					if (IsValidJumpLocation(sel, usMapPos, TRUE))
					{
						*puiNewEvent = JP_ON_TERRAIN;
						gsJumpOverGridNo = usMapPos;
						return;
					}
					else
					{
						const SOLDIERTYPE* const tgt = gUIFullTarget;
						if (tgt != NULL)
						{
							if (IsValidTalkableNPC(tgt, FALSE, FALSE, FALSE) && !_KeyDown(SHIFT) &&
								!AM_AN_EPC(sel) && tgt->bTeam != ENEMY_TEAM && !ValidQuickExchangePosition())
							{
								MoveTargetSoldier = gUIFullTarget;
								*puiNewEvent = T_CHANGE_TO_TALKING;
								return;
							}
							else if (ubItemCursor != AIDCURS)
							{
								// IF it's an ememy, goto confirm action mode
								if ( ( guiUIFullTargetFlags & ENEMY_MERC ) &&
									( guiUIFullTargetFlags & VISIBLE_MERC ) &&
									!( guiUIFullTargetFlags & DEAD_MERC ) )
								{
									MoveTargetSoldier = gUIFullTarget;
									*puiNewEvent = A_ON_TERRAIN;
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

				const SOLDIERTYPE* const tgt = gUIFullTarget;
				if (tgt != NULL)
				{
					if (IsValidTargetMerc(tgt))
					{
						gUITargetSoldier = tgt;
						if (tgt->bTeam == OUR_TEAM && gUIActionModeChangeDueToMouseOver)
						{
							*puiNewEvent = A_CHANGE_TO_MOVE;
							return;
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

			case CONFIRM_MOVE_MODE:

				if ( usMapPos != usOldMapPos )
				{
					// Switch event out of confirm mode
					// Set off ALL move....
					gfUIAllMoveOn = FALSE;

					*puiNewEvent = A_CHANGE_TO_MOVE;
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
						sel->fDoSpread             = TRUE;
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
					if ( usMapPos != usOldMapPos )
					{
						// Switch event out of confirm mode
						*puiNewEvent = CA_END_CONFIRM_ACTION;

					}
					else
					{
						*puiNewEvent = CA_ON_TERRAIN;
					}
				}
				break;
			}
			default:
				break;
		}

		usOldMapPos = usMapPos;

	}
}
