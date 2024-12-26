#include "Font_Control.h"
#include "Real_Time_Input.h"
#include "Soldier_Find.h"
#include "Turn_Based_Input.h"
#include "Soldier_Control.h"
#include "Animation_Control.h"
#include "Timer_Control.h"
#include "Handle_UI.h"
#include "Isometric_Utils.h"
#include "Input.h"
#include "Overhead.h"
#include "Interface.h"
#include "Interactive_Tiles.h"
#include "Interface_Cursors.h"
#include "Spread_Burst.h"
#include "Interface_Items.h"
#include "UI_Cursors.h"
#include "Dialogue_Control.h"
#include "Interface_Dialogue.h"
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

static BOOLEAN gfStartLookingForRubberBanding = false;
static UINT16  gusRubberBandX                 = 0;
static UINT16  gusRubberBandY                 = 0;


BOOLEAN	gfBeginBurstSpreadTracking = false;

BOOLEAN	gfRTClickLeftHoldIntercepted = false;
static BOOLEAN gfRTHaveClickedRightWhileLeftDown = false;


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
	//static BOOLEAN fClickHoldIntercepted = false;
	//static BOOLEAN fClickIntercepted = false;
	static UINT32  uiSingleClickTime;
	static BOOLEAN fDoubleClickIntercepted = false;
	static BOOLEAN fValidDoubleClickPossible = false;

	if ( gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		const GridNo usMapPos = guiCurrentCursorGridNo;
		if (usMapPos == NOWHERE) return;

		// MIDDLE MOUSE BUTTON
		if ( gViewportRegion.ButtonState & MSYS_MIDDLE_BUTTON )
		{
			if ( !fMiddleButtonDown )
			{
				fMiddleButtonDown = true;
				RESETCOUNTER( MMOUSECLICK_DELAY_COUNTER );
			}
		}
		else
		{
			if ( fMiddleButtonDown && !_KeyDown(SHIFT) )
			{
				// OK , FOR double CLICKS - TAKE TIME STAMP & RECORD EVENT
				if ( ( GetJA2Clock() - uiSingleClickTime ) < 300 )
				{
					// CHECK HERE FOR double CLICK EVENTS
					if ( fValidDoubleClickPossible )
					{
						fDoubleClickIntercepted = true;

						// Do stuff....
					}
				}

				// Capture time!
				uiSingleClickTime = GetJA2Clock();

				fValidDoubleClickPossible = true;

				if ( !fDoubleClickIntercepted )
				{
					// CHECK COMBINATIONS ETC...

					if (gamepolicy(middle_mouse_look)) *puiNewEvent = LC_LOOK;
				}

				// Reset flag
				fMiddleButtonDown = false;
				//fClickHoldIntercepted = false;
				//fClickIntercepted = false;
				fDoubleClickIntercepted = false;


				// Reset counter
				RESETCOUNTER( MMOUSECLICK_DELAY_COUNTER );
			}
		}
	}
}

static void QueryRTLeftButton(UIEventKind* const puiNewEvent)
{
	static UINT32 uiSingleClickTime;
	static BOOLEAN fDoubleClickIntercepted = false;
	static BOOLEAN fValidDoubleClickPossible = false;
	static BOOLEAN fCanCheckForSpeechAdvance = false;

	SOLDIERTYPE* const sel = GetSelectedMan();

	// LEFT MOUSE BUTTON
	if ( gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		if (sel != NULL && sel->pTempObject != NULL) return;

		const GridNo usMapPos = guiCurrentCursorGridNo;
		if (usMapPos == NOWHERE && !(gfScrolledToBottom && gusMouseYPos >= SCREEN_HEIGHT - NO_PX_SHOW_EXIT_CURS)) return;

		if ( gViewportRegion.ButtonState & MSYS_LEFT_BUTTON )
		{
			if ( !fLeftButtonDown )
			{
				fLeftButtonDown = true;
				gfRTHaveClickedRightWhileLeftDown = false;
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
										gfRTClickLeftHoldIntercepted = true;
										//fLeftButtonDown              = false;
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
											gfRTClickLeftHoldIntercepted = true;

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

								gfUICanBeginAllMoveCycle = true;

								if ( !HandleCheckForExitArrowsInput( false ) &&
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
											//gfRTClickLeftHoldIntercepted = true;
										}
										else if ( bReturnCode == -2 )
										{
											//if ( gGameSettings.fOptions[ TOPTION_RTCONFIRM ] )
											//{
											//	*puiNewEvent = C_WAIT_FOR_CONFIRM;
											//	gfPlotNewMovement = true;
											//}/
											//else
										}
										else if ( bReturnCode == -3 )
										{
											gfRTClickLeftHoldIntercepted = true;
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
															gfPlotNewMovement = true;
														}
													}
												}
											}
										}
									}
									//gfRTClickLeftHoldIntercepted = true;
								}
								else
								{
									gfRTClickLeftHoldIntercepted = true;
									fIgnoreLeftUp = true;
								}

								break;
							default:
								break;
							}
						}
				}
				if ( gfUIWaitingForUserSpeechAdvance )
				{
					fCanCheckForSpeechAdvance = true;
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
							const SOLDIERTYPE* const s = gUIFullTarget;
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
									gfStartLookingForRubberBanding = true;
									gusRubberBandX = gusMouseXPos;
									gusRubberBandY = gusMouseYPos;
								}
								else
								{
									// Have we moved....?
									if (std::abs(gusMouseXPos - gusRubberBandX) > 10 || std::abs(gusMouseYPos - gusRubberBandY) > 10)
									{
										gfStartLookingForRubberBanding = false;

										// Stop scrolling:
										gfIgnoreScrolling = true;

										// Anchor cursor....
										RestrictMouseToXYXY( 0, 0, gsVIEWPORT_END_X, gsVIEWPORT_WINDOW_END_Y );

										// OK, settup anchor....
										gRubberBandRect.iLeft = gusRubberBandX;
										gRubberBandRect.iTop = gusRubberBandY;

										gRubberBandActive = true;

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
					// OK , FOR double CLICKS - TAKE TIME STAMP & RECORD EVENT
					if ( ( GetJA2Clock() - uiSingleClickTime ) < 300 )
					{
						// CHECK HERE FOR double CLICK EVENTS
						if ( fValidDoubleClickPossible )
						{
							if ( gpItemPointer == NULL )
							{
									fDoubleClickIntercepted = true;

									// First check if we clicked on a guy, if so, make selected if it's ours
									if (sel != NULL)
									{
										// Set movement mode
										// OK, only change this if we are stationary!
										//if (gAnimControl[s->usAnimState].uiFlags & ANIM_STATIONARY)
										//if (sel->usAnimState == WALKING)
										{
											sel->fUIMovementFast = true;
											*puiNewEvent = C_MOVE_MERC;
										}
									}
							}
						}

					}

					// Capture time!
					uiSingleClickTime = GetJA2Clock();

					fValidDoubleClickPossible = false;

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
						if (COUNTERDONE(LMOUSECLICK_DELAY_COUNTER, false))
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

										gfBeginBurstSpreadTracking = false;

										if (sel->sEndGridNo != sel->sStartGridNo )
										{
											sel->fDoSpread = true;
											PickBurstLocations(sel);
											*puiNewEvent = CA_MERC_SHOOT;
										}
										else
										{
											sel->fDoSpread = false;
										}
										gfRTClickLeftHoldIntercepted = true;
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
									if ( !HandleCheckForExitArrowsInput( true ) )
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
															const SOLDIERTYPE* const s = gUIFullTarget;
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
																fValidDoubleClickPossible = true;
															}
															break;

														case CONFIRM_ACTION_MODE:
															*puiNewEvent = CA_MERC_SHOOT;
															break;

														case MOVE_MODE:

															if ( !HandleCheckForExitArrowsInput( false ) && gpItemPointer == NULL )
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

																					gfIgnoreOnSelectedGuy = true;


																					EndMultiSoldierSelection( false );
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

																				gfIgnoreOnSelectedGuy = true;

																				EndMultiSoldierSelection( false );
																			}
																		}
																	}
																	gfRTClickLeftHoldIntercepted = true;
																}
																else
																{
																	INT8 const bReturnCode = HandleMoveModeInteractiveClick(usMapPos);
																	if ( bReturnCode == -1 )
																	{
																		gfRTClickLeftHoldIntercepted = true;
																	}
																	else if ( bReturnCode == -2 )
																	{
																		//if ( gGameSettings.fOptions[ TOPTION_RTCONFIRM ] )
																		//{
																		//	*puiNewEvent = C_WAIT_FOR_CONFIRM;
																		//	gfPlotNewMovement = true;
																		//}/
																		//else
																		//{
																			if (sel != NULL)
																			{
																				BeginDisplayTimedCursor( GetInteractiveTileCursor( guiCurrentUICursor, true ), 300 );

																				if (sel->usAnimState != RUNNING)
																				{
																					*puiNewEvent = C_MOVE_MERC;
																				}
																				else if (GetCurInteractiveTile() != NULL)
																				{
																					sel->fUIMovementFast = true;
																					*puiNewEvent = C_MOVE_MERC;
																				}
																				fValidDoubleClickPossible = true;
																			}
																		//}
																	}
																	else if ( bReturnCode == 0 )
																	{
																		if (sel != NULL)
																		{
																			// First check if we clicked on a guy, if so, make selected if it's ours
																			const SOLDIERTYPE* const s = gUIFullTarget;
																			if (s && IsOwnedMerc(*s))
																			{
																				// Select guy
																				*puiNewEvent = I_SELECT_MERC;
																				gfRTClickLeftHoldIntercepted = true;
																			}
																			else
																			{
																				if ( gsCurrentActionPoints == 0 && !gfUIAllMoveOn && !gTacticalStatus.fAtLeastOneGuyOnMultiSelect )
																				{
																					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
																					gfRTClickLeftHoldIntercepted = true;
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
																							fValidDoubleClickPossible = true;
																						}
																						else
																						{
																							// We're on terrain in which we can walk, walk
																							// If we're on terrain,
																							if (gGameSettings.fOptions[TOPTION_RTCONFIRM])
																							{
																								*puiNewEvent = C_WAIT_FOR_CONFIRM;
																								gfPlotNewMovement = true;
																							}
																							else
																							{
																								*puiNewEvent = C_MOVE_MERC;
																								fValidDoubleClickPossible = true;
																							}
																						}
																					}
																					else
																					{
																						if (fResult == 2)
																						{
																							ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NOBODY_USING_REMOTE_STR]);
																						}
																						gfRTClickLeftHoldIntercepted = true;
																					}
																				}
																			}
																		}
																	}
																}
																//gfRTClickLeftHoldIntercepted = true;
															}
															else
															{
																gfRTClickLeftHoldIntercepted = true;
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

															//HandleTalkingMenuEscape( true );
															break;

														case EXITSECTORMENU_MODE:

															RemoveSectorExitMenu( false );
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
				fLeftButtonDown = false;
				fIgnoreLeftUp   = false;
				gfRTClickLeftHoldIntercepted = false;
				fDoubleClickIntercepted = false;
				fCanCheckForSpeechAdvance = false;
				gfStartLookingForRubberBanding = false;

				// Reset counter
				RESETCOUNTER( LMOUSECLICK_DELAY_COUNTER );

			}

		}

	}
	else
	{
		// Set mouse down to false
		//fLeftButtonDown = false;

		//fCanCheckForSpeechAdvance = false;

		// OK, handle special cases like if we are dragging and holding for a burst spread and
		// release mouse over another mouse region
		if ( gfBeginBurstSpreadTracking )
		{
			if (sel != NULL) sel->fDoSpread = false;
			gfBeginBurstSpreadTracking = false;
		}
	}
}


static void QueryRTRightButton(UIEventKind* const puiNewEvent)
{
	static BOOLEAN fClickHoldIntercepted = false;
	static BOOLEAN fClickIntercepted = false;
	static UINT32  uiSingleClickTime;
	static BOOLEAN fDoubleClickIntercepted = false;
	static BOOLEAN fValidDoubleClickPossible = false;

	if ( gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		const GridNo usMapPos = guiCurrentCursorGridNo;
		if (usMapPos == NOWHERE) return;

		// RIGHT MOUSE BUTTON
		if ( gViewportRegion.ButtonState & MSYS_RIGHT_BUTTON )
		{
			if ( !fRightButtonDown )
			{
				fRightButtonDown = true;
				RESETCOUNTER( RMOUSECLICK_DELAY_COUNTER );
			}

			// CHECK COMBINATIONS
			if ( fLeftButtonDown )
			{
				//fIgnoreLeftUp = true;
				gfRTHaveClickedRightWhileLeftDown = true;

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
									fValidDoubleClickPossible = true;

									// OK, our first right-click is an all-cycle
									if ( gfUICanBeginAllMoveCycle )
									{
										// ATE: Here, check if we can do this....
										if (!UIOKMoveDestination(sel, usMapPos))
										{
											ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ CANT_MOVE_THERE_STR ] );
											gfRTClickLeftHoldIntercepted = true;
										}
										//else if ( gsCurrentActionPoints == 0 )
										//{
										//	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
										//	gfRTClickLeftHoldIntercepted = true;
										//}
										else
										{
											*puiNewEvent = M_CYCLE_MOVE_ALL;
										}
									}
									fClickHoldIntercepted = true;
								}
							default:
								break;
						}

						// ATE: Added cancel of burst mode....
						if ( gfBeginBurstSpreadTracking )
						{
							gfBeginBurstSpreadTracking = false;
							gfRTClickLeftHoldIntercepted = true;
							sel->fDoSpread = false;
							fClickHoldIntercepted = true;
							*puiNewEvent = A_END_ACTION;
							gCurrentUIMode = MOVE_MODE;
						}

					}
				}
			}
			else
			{
				// IF HERE, DO A CLICK-HOLD IF IN INTERVAL
				if (COUNTERDONE(RMOUSECLICK_DELAY_COUNTER, false) && !fClickHoldIntercepted)
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
									fClickHoldIntercepted = true;
								}
								else if (GetSelectedMan() != NULL)
								{
									fShowAssignmentMenu = false;
									CreateDestroyAssignmentPopUpBoxes();
									DetermineWhichAssignmentMenusCanBeShown();

									// ATE:
									if ( !fClickHoldIntercepted )
									{
										*puiNewEvent = U_MOVEMENT_MENU;
										fClickHoldIntercepted = true;
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
				// OK , FOR double CLICKS - TAKE TIME STAMP & RECORD EVENT
				if ( ( GetJA2Clock() - uiSingleClickTime ) < 300 )
				{
					// CHECK HERE FOR double CLICK EVENTS
					if ( fValidDoubleClickPossible )
					{
						fDoubleClickIntercepted = true;

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
										//fIgnoreLeftUp = true;
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

				fValidDoubleClickPossible = true;

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
									//fIgnoreLeftUp = true;
									switch( gCurrentUIMode )
									{
										case CONFIRM_MOVE_MODE:
										case MOVE_MODE:

											if ( gfUIAllMoveOn )
											{
												gfUIAllMoveOn = false;
												gfUICanBeginAllMoveCycle = true;
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
											fClickIntercepted = true;
											break;

										case ACTION_MODE:
											// We have here a change to move mode
											*puiNewEvent = A_END_ACTION;
											fClickIntercepted = true;
											break;

										case CONFIRM_ACTION_MODE:
											HandleRightClickAdjustCursor(sel, usMapPos);
											fClickIntercepted = true;
											break;

										case MENU_MODE:
											// If we get a hit here and we're in menu mode, quit the menu mode
											EndMenuEvent( guiCurrentEvent );
											fClickIntercepted = true;
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
				fRightButtonDown = false;
				fClickHoldIntercepted = false;
				fClickIntercepted = false;
				fDoubleClickIntercepted = false;


				// Reset counter
				RESETCOUNTER( RMOUSECLICK_DELAY_COUNTER );
			}
		}
	}
}


void GetRTMousePositionInput(UIEventKind* const puiNewEvent)
{
	static const SOLDIERTYPE* MoveTargetSoldier = NULL;
	static GridNo usOldMapPos = NOWHERE;

	const GridNo usMapPos = guiCurrentCursorGridNo;
	if (usMapPos == NOWHERE) return;

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
				gfUIAllMoveOn = false;
			}

			MoveTargetSoldier = NULL;

			// Check for being on terrain
			const SOLDIERTYPE* const sel = GetSelectedMan();
			if (sel != NULL)
			{
				// get cursor for item
				ItemCursor const ubItemCursor = GetActionModeCursor(sel);

				if (IsValidJumpLocation(sel, usMapPos, true))
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
						if (IsValidTalkableNPC(tgt, false, false, false) && !_KeyDown(SHIFT) &&
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

			if ( !gfIsUsingTouch && usMapPos != usOldMapPos )
			{
				// Switch event out of confirm mode
				// Set off ALL move....
				gfUIAllMoveOn = false;

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
					sel->fDoSpread             = true;
					gfBeginBurstSpreadTracking = true;
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

void TacticalViewPortTouchCallbackRT(MOUSE_REGION* region, UINT32 reason) {
	static GridNo gLastUpGridNo = NOWHERE;
	static SOLDIERTYPE* gLastDownUIFullTarget = NULL;

	if (reason & MSYS_CALLBACK_REASON_TFINGER_doubleTAP) {
		if (gLastUpGridNo != NOWHERE && gLastUpGridNo == guiCurrentCursorGridNo) {
			switch( gCurrentUIMode )
			{
				case CONFIRM_MOVE_MODE:
				case MOVE_MODE:
					guiPendingOverrideEvent = C_MOVE_MERC;
					break;
				default:
					break;
			}
		}
	} else if (reason & MSYS_CALLBACK_REASON_TFINGER_DWN) {
		gUIFingersDown += 1;
		if (gUIFingersDown == 1) {
			gLastDownUIFullTarget = gUIFullTarget;

			switch( gCurrentUIMode )
			{
				case CONFIRM_MOVE_MODE:
					gfPlotNewMovement = true;
					guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
					break;
				case CONFIRM_ACTION_MODE:
					gfPlotNewMovement = true;
					guiPendingOverrideEvent = M_CHANGE_TO_ACTION;
				default:
					break;
			}
		}
		if (gUIFingersDown >= 2) {
			switch( gCurrentUIMode )
			{
				case MOVE_MODE:
				case CONFIRM_MOVE_MODE:
					TogglePanMode();
					break;
				default:
					break;
			}
		}
	} else if (reason & MSYS_CALLBACK_REASON_MBUTTON_DWN) {
		if (_KeyDown(SHIFT) && gpItemPointer == NULL) {
			switch( gCurrentUIMode )
			{
				case MOVE_MODE:
				case CONFIRM_MOVE_MODE:
					TogglePanMode();
					break;
				default:
					break;
			}
		}
	} else if (reason & MSYS_CALLBACK_REASON_MBUTTON_UP) {
		if (gCurrentUIMode == PAN_MODE) {
			TogglePanMode();
		}
	} else if (reason & MSYS_CALLBACK_REASON_TFINGER_UP) {
		auto selected = GetSelectedMan();

		if (gUIFingersDown == 1) {
			gLastUpGridNo = guiCurrentCursorGridNo;
		}

		if (guiCurrentCursorGridNo == NOWHERE || selected == NULL) return;

		if ( gpItemPointer != NULL ) {
			// If we went up with item in hand change to confirm mode
			gfPlotNewMovement = true;
			guiPendingOverrideEvent = C_WAIT_FOR_CONFIRM;
		} else {
			switch( gCurrentUIMode )
			{
				case MOVE_MODE:
				case CONFIRM_MOVE_MODE:
				case LOOKCURSOR_MODE:
				case HANDCURSOR_MODE:
				case TALKCURSOR_MODE:
					if (gUIFullTarget != NULL && gUIFullTarget == gLastDownUIFullTarget &&
							(guiUIFullTargetFlags & OWNED_MERC) &&
							!(guiUIFullTargetFlags & UNCONSCIOUS_MERC))
					{
						// If we went down and up on the same merc select it
						guiPendingOverrideEvent = I_SELECT_MERC;
					} else if (gCurrentUIMode == MOVE_MODE) {
						if ( !HandleCheckForExitArrowsInput(true) )
						{
							// If we went up in movement mode, change to confirm movement mode
							gfPlotNewMovement = true;
							guiPendingOverrideEvent = C_WAIT_FOR_CONFIRM;
						}
					}
					break;
				case ACTION_MODE:
				case CONFIRM_ACTION_MODE:
					// If we went up in action mode, change to confirm action mode
					gfPlotNewMovement = true;
					guiPendingOverrideEvent = CA_ON_TERRAIN;
					break;
				case ADJUST_STANCE_MODE:
					guiPendingOverrideEvent = PADJ_ADJUST_STANCE;
					break;
				case PAN_MODE:
					TogglePanMode();
					break;
				default:
					break;
			}
		}
		// We dont get the second up event, so reset here
		gUIFingersDown = 0;
		gLastDownUIFullTarget = NULL;
	} else if (reason & MSYS_CALLBACK_REASON_TFINGER_REPEAT) {
		auto selected = GetSelectedMan();

		if (selected == gUIFullTarget && selected == gLastDownUIFullTarget) {
			switch( gCurrentUIMode )
			{
				case MOVE_MODE:
				case CONFIRM_MOVE_MODE:
					guiPendingOverrideEvent = M_CHANGE_TO_ADJPOS_MODE;
					break;
				default:
					break;
			}
		}
	}
}
