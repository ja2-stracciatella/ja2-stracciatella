#include "Font_Control.h"
#include "Real_Time_Input.h"
#include "Soldier_Find.h"
#include "Turn_Based_Input.h"
#include "WCheck.h"
#include "Debug.h"
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
#include "Environment.h"
#include "OppList.h"
#include "Spread_Burst.h"
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
#include "MessageBoxScreen.h"
#include "Gameloop.h"
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


static BOOLEAN gfStartLookingForRubberBanding = FALSE;
static UINT16  gusRubberBandX                 = 0;
static UINT16  gusRubberBandY                 = 0;


BOOLEAN	gfBeginBurstSpreadTracking = FALSE;

BOOLEAN	gfRTClickLeftHoldIntercepted = FALSE;
static BOOLEAN gfRTHaveClickedRightWhileLeftDown = FALSE;


static void QueryRTLeftButton(UINT32* puiNewEvent);
static void QueryRTRightButton(UINT32* puiNewEvent);


void GetRTMouseButtonInput(UINT32* puiNewEvent)
{
	 QueryRTLeftButton( puiNewEvent );
	 QueryRTRightButton( puiNewEvent );
}


static void QueryRTLeftButton(UINT32* puiNewEvent)
{
	static	UINT32 uiSingleClickTime;
	UINT16	usMapPos;
	BOOLEAN fDone = FALSE;
	static BOOLEAN	fDoubleClickIntercepted = FALSE;
	static BOOLEAN	fValidDoubleClickPossible = FALSE;
	static BOOLEAN  fCanCheckForSpeechAdvance = FALSE;
	static INT16		sMoveClickGridNo					= 0;


	// LEFT MOUSE BUTTON
  if ( gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		if (!GetMouseMapPos( &usMapPos ) && !gfUIShowExitSouth )
		{
			return;
		}

		if (gusSelectedSoldier != NOBODY && GetSelectedMan()->pTempObject != NULL)
		{
			return;
		}

		if ( gViewportRegion.ButtonState & MSYS_LEFT_BUTTON )
		{
			if ( !fLeftButtonDown )
			{
				fLeftButtonDown = TRUE;
				gfRTHaveClickedRightWhileLeftDown = FALSE;
				RESETCOUNTER( LMOUSECLICK_DELAY_COUNTER );

				if ( giUIMessageOverlay == -1 )
				{
					if ( gpItemPointer == NULL )
					{
						switch( gCurrentUIMode )
						{
							case ACTION_MODE:

								if ( gusSelectedSoldier != NOBODY )
								{
									SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
									if (pSoldier != NULL && gpItemPointer == NULL)
									{
										// OK, check for needing ammo
										if ( HandleUIReloading( pSoldier ) )
										{
											gfRTClickLeftHoldIntercepted = TRUE;
											//fLeftButtonDown				= FALSE;
										}
										else
										{
											if ( pSoldier->bDoBurst )
											{
												pSoldier->sStartGridNo = usMapPos;
												ResetBurstLocations( );
												*puiNewEvent = A_CHANGE_TO_CONFIM_ACTION;
											}
											else
											{
												gfRTClickLeftHoldIntercepted = TRUE;

												if ( UIMouseOnValidAttackLocation( pSoldier )  )
												{
													// OK< going into confirm will call a function that will automatically move
													// us to shoot in most vases ( grenades need a confirm mode regardless )
													*puiNewEvent = A_CHANGE_TO_CONFIM_ACTION;
													//*puiNewEvent = CA_MERC_SHOOT;
												}
											}
										}
									}
								}
								break;


							case MOVE_MODE:

								gfUICanBeginAllMoveCycle = TRUE;

								if ( !HandleCheckForExitArrowsInput( FALSE ) && gpItemPointer == NULL )
								{
									if ( gfUIFullTargetFound && ( guiUIFullTargetFlags & OWNED_MERC )  )
									{
										// Reset , if this guy is selected merc, reset any multi selections...
										if ( gusUIFullTargetID == gusSelectedSoldier )
										{
											ResetMultiSelection( );
										}
									}
									else
									{
										INT8 bReturnCode;

										bReturnCode = HandleMoveModeInteractiveClick( usMapPos, puiNewEvent );

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
											if ( gusSelectedSoldier != NOBODY )
											{
												if (UIOKMoveDestination(GetSelectedMan(), usMapPos) == 1)
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
				if ( giUIMessageOverlay == -1 && !gfRTHaveClickedRightWhileLeftDown )
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
								if (s == GetSelectedMan() && s->bLife >= OKLIFE && !(s->uiStatusFlags & SOLDIER_VEHICLE))
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
									gfStartLookingForRubberBanding	 = TRUE;
									gusRubberBandX									 = gusMouseXPos;
									gusRubberBandY									 = gusMouseYPos;
								}
								else
								{
									// Have we moved....?
									if ( abs( gusMouseXPos - gusRubberBandX ) > 10 || abs( gusMouseYPos - gusRubberBandY ) > 10 )
									{
										gfStartLookingForRubberBanding = FALSE;

										// Stop scrolling:
										gfIgnoreScrolling = TRUE;

										// Anchor cursor....
										RestrictMouseToXYXY( 0, 0, gsVIEWPORT_END_X, gsVIEWPORT_WINDOW_END_Y );

										// OK, settup anchor....
										gRubberBandRect.iLeft = gusRubberBandX;
										gRubberBandRect.iTop	= gusRubberBandY;

										gRubberBandActive			= TRUE;

										// ATE: If we have stopped scrolling.....
										if (gfScrollInertia)
										{
											SetRenderFlags( RENDER_FLAG_FULL | RENDER_FLAG_CHECKZ );

											// Restore Interface!
											RestoreInterface( );

											// Delete Topmost blitters saved areas
											DeleteVideoOverlaysArea( );

											gfScrollInertia = FALSE;
										}

										*puiNewEvent = RB_ON_TERRAIN;
										return;
									}
								}
							}
							break;
						}
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
									if( gusSelectedSoldier != NO_SOLDIER )
									{
										// Set movement mode
										// OK, only change this if we are stationary!
										SOLDIERTYPE* const s = GetSelectedMan();
										//if (gAnimControl[s->usAnimState].uiFlags & ANIM_STATIONARY)
										//if (s->usAnimState == WALKING)
										{
											s->fUIMovementFast = TRUE;
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
											gfRTClickLeftHoldIntercepted = TRUE;
										}
									}
									break;
								}
							}
						}
						//else
						{
							//LEFT CLICK NORMAL EVENT
							// Switch on UI mode
							if ( !gfRTClickLeftHoldIntercepted )
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
															if (s != NULL && IsOwnedMerc(s))
															{
																*puiNewEvent = I_SELECT_MERC;
															}
															break;
														}

													case HANDCURSOR_MODE:

															HandleHandCursorClick( usMapPos, puiNewEvent );
															break;

														case ACTION_MODE:

															//*puiNewEvent = A_CHANGE_TO_CONFIM_ACTION;
															//if(	GetSoldier( &pSoldier, gusSelectedSoldier ) )
															//{
															//	pSoldier->sStartGridNo = usMapPos;
															//	ResetBurstLocations( );
															//}
															*puiNewEvent = CA_MERC_SHOOT;
															break;

														case CONFIRM_MOVE_MODE:

															if ( gusSelectedSoldier != NO_SOLDIER )
															{
																SOLDIERTYPE* const s = GetSelectedMan();
																if (s->usAnimState != RUNNING)
																{
																	*puiNewEvent = C_MOVE_MERC;
																}
																else
																{
																	s->fUIMovementFast = 2;
																	*puiNewEvent = C_MOVE_MERC;
																}
															}

															//*puiNewEvent = C_MOVE_MERC;

															//if ( gGameSettings.fOptions[ TOPTION_RTCONFIRM ] )
															{
																fValidDoubleClickPossible = TRUE;
															}
															break;

														case CONFIRM_ACTION_MODE:

															// Check if we are stationary
															//if ( AimCubeUIClick( ) )
															//{
															//	if ( gusSelectedSoldier != NO_SOLDIER )
															//	{
															//		if (!(gAnimControl[GetSelectedMan()->usAnimState].uiFlags & ANIM_STATIONARY))
															//		{

																		//gUITargetShotWaiting  = TRUE;
															//		}
															//		else
															//		{
																	//	*puiNewEvent = CA_MERC_SHOOT;
															//		}
																	*puiNewEvent = CA_MERC_SHOOT;
															//	}
															//}
															break;

														case MOVE_MODE:

																if ( !HandleCheckForExitArrowsInput( FALSE ) && gpItemPointer == NULL )
																{
																	// First check if we clicked on a guy, if so, make selected if it's ours
																	if ( gfUIFullTargetFound && ( guiUIFullTargetFlags & OWNED_MERC )  )
																	{
																		 if ( !( guiUIFullTargetFlags & UNCONSCIOUS_MERC ) )
																		 {
																			 // Select guy
																			SOLDIERTYPE* pSoldier = GetSoldier(gusUIFullTargetID);
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
																								if ( gusSelectedSoldier != NOBODY )
																								{
																									GetSelectedMan()->uiStatusFlags |= SOLDIER_MULTI_SELECTED;
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
																						if ( gusSelectedSoldier != NOBODY )
																						{
																							GetSelectedMan()->uiStatusFlags |= SOLDIER_MULTI_SELECTED;
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
																		INT8 bReturnCode;

																		bReturnCode = HandleMoveModeInteractiveClick( usMapPos, puiNewEvent );

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
																			{
																			  INT16							sIntTileGridNo;

																				SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
																				if (pSoldier != NULL)
																				{
																					BeginDisplayTimedCursor( GetInteractiveTileCursor( guiCurrentUICursor, TRUE ), 300 );

																					if ( pSoldier->usAnimState != RUNNING )
																					{
																						*puiNewEvent = C_MOVE_MERC;
																					}
																					else
																					{
																						if ( GetCurInteractiveTileGridNo( &sIntTileGridNo ) != NULL )
																						{
																							pSoldier->fUIMovementFast = TRUE;
																							*puiNewEvent = C_MOVE_MERC;
																						}
																					}
																					fValidDoubleClickPossible = TRUE;
																				}
																			}
																		}
																		else if ( bReturnCode == 0 )
																		{
																			const SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
																			if (pSoldier != NULL)
																			{
																				// First check if we clicked on a guy, if so, make selected if it's ours
																				const SOLDIERTYPE* const s = FindSoldierFromMouse();
																				if (s != NULL && IsOwnedMerc(s))
																				{
																					 // Select guy
																						*puiNewEvent = I_SELECT_MERC;
																						gfRTClickLeftHoldIntercepted = TRUE;
																				}
																				else
																				{
																					//if ( FindBestPath( pSoldier, usMapPos, pSoldier->bLevel, pSoldier->usUIMovementMode, NO_COPYROUTE, 0 ) == 0 )
																					if ( gsCurrentActionPoints == 0 && !gfUIAllMoveOn && !gTacticalStatus.fAtLeastOneGuyOnMultiSelect )
																					{
																						ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
																						gfRTClickLeftHoldIntercepted = TRUE;
																					}
																					else
																					{
																						BOOLEAN fResult;

																						if ( gusSelectedSoldier != NOBODY )
																						{
																							fResult = UIOKMoveDestination(GetSelectedMan(), usMapPos);
																							if (fResult == 1)
																							{
																								if ( gfUIAllMoveOn )
																								{
																									// ATE: Select everybody in squad and make move!
																									{
#if 0
																										SOLDIERTYPE *		pTeamSoldier;
																										INT32						cnt;
																										SOLDIERTYPE			*pFirstSoldier = NULL;

																										// OK, loop through all guys who are 'multi-selected' and
																										// check if our currently selected guy is amoung the
																										// lucky few.. if not, change to a guy who is...
																										cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;
																										for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++, pTeamSoldier++ )
																										{
																											// Default turn off
																											pTeamSoldier->uiStatusFlags &= (~SOLDIER_MULTI_SELECTED );

																											// If controllable
																											if (OK_CONTROLLABLE_MERC(pTeamSoldier) && pTeamSoldier->bAssignment == GetSelectedMan()->bAssignment)
																											{
																												pTeamSoldier->uiStatusFlags |= SOLDIER_MULTI_SELECTED;
																											}
																										}
																										EndMultiSoldierSelection( FALSE );
#endif

																										// Make move!
																										*puiNewEvent = C_MOVE_MERC;

																										fValidDoubleClickPossible = TRUE;
																									}
																								}
																								else
																								{
																									// We're on terrain in which we can walk, walk
																									// If we're on terrain,
																									if ( gGameSettings.fOptions[ TOPTION_RTCONFIRM ]  )
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
																								if ( fResult == 2 )
																								{
																									ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NOBODY_USING_REMOTE_STR ] );
																								}
																								else
																								{
																									//if ( usMapPos != sMoveClickGridNo || pSoldier->uiStatusFlags & SOLDIER_ROBOT )
																									//{
																								//		sMoveClickGridNo					= usMapPos;

																										//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ CANT_MOVE_THERE_STR ] );

																								//		*puiNewEvent					  = M_CHANGE_TO_HANDMODE;
																								//		gsOverItemsGridNo				= usMapPos;
																								//		gsOverItemsLevel				= gsInterfaceLevel;
																								//	}
																								//	else
																								//	{
																								//		sMoveClickGridNo = 0;
																								//		*puiNewEvent = M_CHANGE_TO_HANDMODE;
																								//	}
																								}
																								//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, L"Invalid move destination." );

																								// Goto hand cursor mode...
																								//*puiNewEvent = M_CHANGE_TO_HANDMODE;

																								gfRTClickLeftHoldIntercepted = TRUE;
																							}
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


#if 0
															fDone = FALSE;
															if(	GetSoldier( &pSoldier, gusUIFullTargetID ) && gpItemPointer == NULL )
															{
																if( ( guiUIFullTargetFlags & OWNED_MERC ) && ( guiUIFullTargetFlags & VISIBLE_MERC ) && !( guiUIFullTargetFlags & DEAD_MERC ) &&( pSoldier->bAssignment >= ON_DUTY )&&!( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
																{
																	fShowAssignmentMenu = TRUE;
																	gfRTClickLeftHoldIntercepted = TRUE;
																	CreateDestroyAssignmentPopUpBoxes( );
																	SetTacticalPopUpAssignmentBoxXY( );
																	DetermineBoxPositions( );
																	DetermineWhichAssignmentMenusCanBeShown( );
																	fFirstClickInAssignmentScreenMask = TRUE;
																	gfIgnoreScrolling = TRUE;

																	fDone = TRUE;
																}
																else
																{
																	fShowAssignmentMenu = FALSE;
																	CreateDestroyAssignmentPopUpBoxes( );
																	DetermineWhichAssignmentMenusCanBeShown( );
																}
															}

															if( fDone == TRUE )
															{
																break;
															}
#endif
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
			SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
			if (pSoldier != NULL) pSoldier->fDoSpread = FALSE;
			gfBeginBurstSpreadTracking = FALSE;
		}
	}
}


static void QueryRTRightButton(UINT32* puiNewEvent)
{
	static BOOLEAN	fClickHoldIntercepted = FALSE;
	static BOOLEAN	fClickIntercepted = FALSE;
	static UINT32		uiSingleClickTime;
	static BOOLEAN	fDoubleClickIntercepted = FALSE;
	static BOOLEAN	fValidDoubleClickPossible = FALSE;

	UINT16	usMapPos;

  if ( gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		if (!GetMouseMapPos( &usMapPos ) )
		{
			return;
		}

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
					if ( gusSelectedSoldier != NOBODY )
					{
						switch( gCurrentUIMode )
						{
							case CONFIRM_MOVE_MODE:
							case MOVE_MODE:

								if ( !gfUIAllMoveOn )
								{
									fValidDoubleClickPossible	= TRUE;

									// OK, our first right-click is an all-cycle
									if ( gfUICanBeginAllMoveCycle )
									{
										// ATE: Here, check if we can do this....
										if (!UIOKMoveDestination(GetSelectedMan(), usMapPos))
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
									const SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
									if (pSoldier != NULL)
                  {
									  if ( ( guiUIFullTargetFlags & OWNED_MERC ) && ( guiUIFullTargetFlags & VISIBLE_MERC ) && !( guiUIFullTargetFlags & DEAD_MERC )&&!( pSoldier ?  pSoldier->uiStatusFlags & SOLDIER_VEHICLE : 0 ) )
									  {
										  //if( pSoldier->bAssignment >= ON_DUTY )
										  {
												PopupAssignmentMenuInTactical();
    										fClickHoldIntercepted = TRUE;
                      }
										  break;
									  }
									  else
									  {
										  fShowAssignmentMenu = FALSE;
										  CreateDestroyAssignmentPopUpBoxes( );
										  DetermineWhichAssignmentMenusCanBeShown( );
									  }

									  // ATE:
									  if ( !fClickHoldIntercepted )
									  {
										  *puiNewEvent = U_MOVEMENT_MENU;
										  fClickHoldIntercepted = TRUE;
									  }
	  							}
  								break;
								}
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
										if ( gusSelectedSoldier != NOBODY )
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
								if ( gusSelectedSoldier != NOBODY )
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
								if ( gusSelectedSoldier != NOBODY )
								{
									// Switch on UI mode
									switch( gCurrentUIMode )
									{
										case IDLE_MODE:

											break;

										case CONFIRM_MOVE_MODE:
										case MOVE_MODE:
										case TALKCURSOR_MODE:

											{
												// We have here a change to action mode
												*puiNewEvent = M_CHANGE_TO_ACTION;
											}
											fClickIntercepted = TRUE;
											break;

										case ACTION_MODE:

											// We have here a change to move mode
											*puiNewEvent = A_END_ACTION;
											fClickIntercepted = TRUE;
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

										case HANDCURSOR_MODE:
											// If we cannot actually do anything, return to movement mode
											*puiNewEvent = A_CHANGE_TO_MOVE;
											break;

										case LOOKCURSOR_MODE:

											// If we cannot actually do anything, return to movement mode
											*puiNewEvent = A_CHANGE_TO_MOVE;
											break;

									}
								}
							}
							else
							{
								if ( gfUIFullTargetFound )
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


void GetRTMousePositionInput(UINT32* puiNewEvent)
{
	UINT16						usMapPos;
	static UINT16			usOldMapPos = 0;
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
			case RUBBERBAND_MODE:

				// ATE: Make sure!
				if ( gRubberBandActive == FALSE )
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

				if ( uiMoveTargetSoldierId != NOBODY )
				{
					 if( gfUIFullTargetFound )
					 {
							if ( gusUIFullTargetID != uiMoveTargetSoldierId )
							{
								*puiNewEvent = A_CHANGE_TO_MOVE;
								return;
							}
					 }
					 else
					 {
							*puiNewEvent = A_CHANGE_TO_MOVE;
							return;
					 }
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

				uiMoveTargetSoldierId = NO_SOLDIER;

				// Check for being on terrain
				const SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
				if (pSoldier != NULL)
				{
						UINT8					ubItemCursor;

						 // get cursor for item
						 ubItemCursor  =  GetActionModeCursor( pSoldier );

						 //
						 if ( IsValidJumpLocation( pSoldier, usMapPos, TRUE ) )
						 {
								*puiNewEvent = JP_ON_TERRAIN;
								gsJumpOverGridNo = usMapPos;
								return;
						 }
						 else
						 {
							 if( gfUIFullTargetFound )
							 {
									if ( IsValidTalkableNPC( (UINT8)gusUIFullTargetID, FALSE, FALSE, FALSE ) && !_KeyDown( SHIFT ) && !AM_AN_EPC( pSoldier ) && MercPtrs[ gusUIFullTargetID ]->bTeam != ENEMY_TEAM && !ValidQuickExchangePosition( ) )
									{
										uiMoveTargetSoldierId = gusUIFullTargetID;
										*puiNewEvent = T_CHANGE_TO_TALKING;
										return;
									}
									else if ( ubItemCursor == AIDCURS )
									{
										// IF it's an ememy, goto confirm action mode
										if ( ( guiUIFullTargetFlags & OWNED_MERC ) && ( guiUIFullTargetFlags & VISIBLE_MERC ) && !( guiUIFullTargetFlags & DEAD_MERC ) )
										{
											//uiMoveTargetSoldierId = gusUIFullTargetID;
											//*puiNewEvent = A_ON_TERRAIN;
											//return;
										}
									}
									else
									{
										// IF it's an ememy, goto confirm action mode
										if ( ( guiUIFullTargetFlags & ENEMY_MERC ) && ( guiUIFullTargetFlags & VISIBLE_MERC ) && !( guiUIFullTargetFlags & DEAD_MERC ) )
										{
											uiMoveTargetSoldierId = gusUIFullTargetID;
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

				// First check if we are on a guy, if so, make selected if it's ours
				// Check if the guy is visible
				guiUITargetSoldierId = NOBODY;

				if ( gfUIFullTargetFound  )
				//if ( gfUIFullTargetFound )
				{
						if ( IsValidTargetMerc( (UINT8)gusUIFullTargetID ) )
						{
							 guiUITargetSoldierId = gusUIFullTargetID;

							if (MercPtrs[gusUIFullTargetID]->bTeam == gbPlayerNum &&
									gUIActionModeChangeDueToMouseOver)
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
					if ( gfUIFullTargetFound )
					{
					 if ( guiUITargetSoldierId != gusUIFullTargetID )
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
				break;
			}
		}

		//if ( gCurrentUIMode != CONFIRM_ACTION_MODE )
		//{
		//	if(	GetSoldier( &pSoldier, gusSelectedSoldier ) )
		//	{
				// Change refine value back to 1
		///		pSoldier->bShownAimTime = REFINE_AIM_1;
		//	}
		//}

		usOldMapPos = usMapPos;

	}
}
