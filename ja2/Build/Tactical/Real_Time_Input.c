#ifdef PRECOMPILEDHEADERS
	#include "Tactical All.h"
#else
	#include <stdio.h>
	#include <string.h>
	#include "wcheck.h"
	#include "stdlib.h"
	#include "debug.h"
	#include "math.h"
	#include "jascreens.h"
	#include "pathai.h"
	#include "Soldier Control.h"
	#include "Animation Control.h"
	#include "Animation Data.h"
	#include "Event Pump.h"
	#include "Timer Control.h"
	#include "cursors.h"
	#include "Handle UI.h"
	#include "Isometric Utils.h"
	#include "input.h"
	#include "overhead.h"
	#include "Sys Globals.h"
	#include "screenids.h"
	#include "interface.h"
	#include "cursor control.h"
	#include "points.h"
	#include "Interactive Tiles.h"
	#include "interface cursors.h"
	#include "Sound Control.h"
	#include "environment.h"
	#include "opplist.h"
	#include "spread burst.h"
	#include "overhead map.h"
	#include "world items.h"
	#include "Game Clock.h"
	#include "interface items.h"
	#include "physics.h"
	#include "ui cursors.h"
	#include "worldman.h"
	#include "strategicmap.h"
	#include "soldier profile.h"
	#include "soldier create.h"
	#include "soldier add.h"
	#include "dialogue control.h"
	#include "interface dialogue.h"
	#include "interactive tiles.h"
	#include "messageboxscreen.h"
	#include "gameloop.h"
	#include "gamescreen.h"
	#include "spread burst.h"
	#include "tile animation.h"
	#include "merc entering.h"
	#include "explosion control.h"
	#include "message.h"
	#include "Strategic Exit GUI.h"
	#include "Assignments.h"
	#include "Map Screen Interface.h"
	#include "renderworld.h"
	#include	"GameSettings.h"
	#include "english.h"
	#include "text.h"
	#include "soldier macros.h"
#endif


extern UIKEYBOARD_HOOK					gUIKeyboardHook;
extern BOOLEAN	fRightButtonDown;
extern BOOLEAN	fLeftButtonDown;
extern BOOLEAN fIgnoreLeftUp;
extern UINT32  guiCurrentEvent;
extern UINT32 guiCurrentUICursor;
extern void DetermineWhichAssignmentMenusCanBeShown( void );
extern BOOLEAN gfIgnoreOnSelectedGuy;
extern INT16 gsOverItemsGridNo;
extern INT16 gsOverItemsLevel;
extern UINT32	guiUITargetSoldierId;
extern BOOLEAN	gfUIShowExitSouth;


BOOLEAN	gfStartLookingForRubberBanding	 = FALSE;
UINT16	gusRubberBandX									 = 0;
UINT16	gusRubberBandY									 = 0;


BOOLEAN	gfBeginBurstSpreadTracking = FALSE;

BOOLEAN	gfRTClickLeftHoldIntercepted = FALSE;
BOOLEAN	gfRTHaveClickedRightWhileLeftDown = FALSE;

extern BOOLEAN ValidQuickExchangePosition( );


void	GetRTMouseButtonInput( UINT32 *puiNewEvent )
{
	 QueryRTLeftButton( puiNewEvent );
	 QueryRTRightButton( puiNewEvent );
}

void	QueryRTLeftButton( UINT32 *puiNewEvent )
{
	UINT16	usSoldierIndex;
	SOLDIERTYPE *pSoldier;
	UINT32	uiMercFlags;
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

		if ( gusSelectedSoldier != NOBODY )
		{
			if ( MercPtrs[ gusSelectedSoldier ]->pTempObject != NULL )
			{
				return;
			}
		}

		if ( gViewportRegion.ButtonState & MSYS_LEFT_BUTTON )
		{
			if ( !fLeftButtonDown )
			{
				fLeftButtonDown = TRUE;
				gfRTHaveClickedRightWhileLeftDown = FALSE;
				RESETCOUNTER( LMOUSECLICK_DELAY_COUNTER );
				RESETCOUNTER( RUBBER_BAND_START_DELAY );

				if ( giUIMessageOverlay == -1 )
				{
					if ( gpItemPointer == NULL )
					{
						switch( gCurrentUIMode )
						{
							case ACTION_MODE:

								if ( gusSelectedSoldier != NOBODY )
								{
									if(	GetSoldier( &pSoldier, gusSelectedSoldier ) && gpItemPointer == NULL )
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
													PauseRT( FALSE );
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
											{
												{
													BOOLEAN fResult;

													if ( gusSelectedSoldier != NOBODY )
													{
														if ( ( fResult = UIOKMoveDestination( MercPtrs[ gusSelectedSoldier ], usMapPos ) ) == 1 )
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

							// First check if we clicked on a guy, if so, make selected if it's ours
							if(  FindSoldierFromMouse( &usSoldierIndex, &uiMercFlags ) )
							{
								 // Select guy
								 if ( (uiMercFlags & SELECTED_MERC) && !( uiMercFlags & UNCONSCIOUS_MERC ) && !( MercPtrs[ usSoldierIndex ]->uiStatusFlags & SOLDIER_VEHICLE ) )
								 {
										*puiNewEvent = M_CHANGE_TO_ADJPOS_MODE;
								 }
							}
							else
							{
								//if ( COUNTERDONE( RUBBER_BAND_START_DELAY )  )
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
											if ( gfScrollInertia != FALSE  )
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
							}
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
									if( gusSelectedSoldier != NO_SOLDIER )
									{
										// Set movement mode
										// OK, only change this if we are stationary!
										//if ( gAnimControl[ MercPtrs[ gusSelectedSoldier ]->usAnimState ].uiFlags & ANIM_STATIONARY )
										//if ( MercPtrs[ gusSelectedSoldier ]->usAnimState == WALKING )
										{
											MercPtrs[ gusSelectedSoldier ]->fUIMovementFast = TRUE;
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

									if(	GetSoldier( &pSoldier, gusSelectedSoldier ) )
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

															// First check if we clicked on a guy, if so, make selected if it's ours
															if(  FindSoldierFromMouse( &usSoldierIndex, &uiMercFlags ) )
															{
																 // Select guy
																 if ( uiMercFlags & OWNED_MERC )
																 {
																		*puiNewEvent = I_SELECT_MERC;
																 }

															}
															break;

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
																if ( MercPtrs[ gusSelectedSoldier ]->usAnimState != RUNNING )
																{
																	*puiNewEvent = C_MOVE_MERC;
																}
																else
																{
																	MercPtrs[ gusSelectedSoldier ]->fUIMovementFast = 2;
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
															//		if ( !( gAnimControl[ MercPtrs[ gusSelectedSoldier ]->usAnimState ].uiFlags & ANIM_STATIONARY ) )
															//		{

																		//gUITargetShotWaiting  = TRUE;
																		//gsUITargetShotGridNo	= usMapPos;
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
																			 if(	GetSoldier( &pSoldier, gusUIFullTargetID ) && gpItemPointer == NULL )
																			 {
																					if( pSoldier->bAssignment >= ON_DUTY && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
																					{
																						PopupAssignmentMenuInTactical( pSoldier );
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
																									MercPtrs[ gusSelectedSoldier ]->uiStatusFlags |= (SOLDIER_MULTI_SELECTED );
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
																							MercPtrs[ gusSelectedSoldier ]->uiStatusFlags |= (SOLDIER_MULTI_SELECTED );
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

																				if(	GetSoldier( &pSoldier, gusSelectedSoldier ) )
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
																			if(	GetSoldier( &pSoldier, gusSelectedSoldier ) )
																			{
																				// First check if we clicked on a guy, if so, make selected if it's ours
																				if(  FindSoldierFromMouse( &usSoldierIndex, &uiMercFlags ) && ( uiMercFlags & OWNED_MERC ) )
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
																							if ( ( fResult = UIOKMoveDestination( MercPtrs[ gusSelectedSoldier ], usMapPos ) ) == 1 )
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
																											if ( OK_CONTROLLABLE_MERC( pTeamSoldier ) && pTeamSoldier->bAssignment == MercPtrs[ gusSelectedSoldier ]->bAssignment )
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

															PauseRT( FALSE );

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
			if(	GetSoldier( &pSoldier, gusSelectedSoldier ) )
			{
				pSoldier->fDoSpread = FALSE;
			}
			gfBeginBurstSpreadTracking = FALSE;
		}
	}
}

void	QueryRTRightButton( UINT32 *puiNewEvent )
{
	static BOOLEAN	fClickHoldIntercepted = FALSE;
	static BOOLEAN	fClickIntercepted = FALSE;
	static UINT32		uiSingleClickTime;
	static BOOLEAN	fDoubleClickIntercepted = FALSE;
	static BOOLEAN	fValidDoubleClickPossible = FALSE;

	SOLDIERTYPE *pSoldier;
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
										if ( !UIOKMoveDestination( MercPtrs[ gusSelectedSoldier ], usMapPos ) )
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
							MercPtrs[ gusSelectedSoldier ]->fDoSpread				 = FALSE;
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

                  if ( GetSoldier( &pSoldier, gusSelectedSoldier ) )
                  {
									  if ( ( guiUIFullTargetFlags & OWNED_MERC ) && ( guiUIFullTargetFlags & VISIBLE_MERC ) && !( guiUIFullTargetFlags & DEAD_MERC )&&!( pSoldier ?  pSoldier->uiStatusFlags & SOLDIER_VEHICLE : 0 ) )
									  {
										  //if( pSoldier->bAssignment >= ON_DUTY )
										  {
                        PopupAssignmentMenuInTactical( pSoldier );
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
  									break;
	  						}
              }

							if ( gCurrentUIMode == ACTION_MODE || gCurrentUIMode == TALKCURSOR_MODE )
							{
								PauseRT( FALSE );
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

											if ( GetSoldier( &pSoldier, gusSelectedSoldier ) )
											{
												HandleRightClickAdjustCursor( pSoldier, usMapPos );
											}
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


extern BOOLEAN ConfirmActionCancel( UINT16 usMapPos, UINT16 usOldMapPos );

extern BOOLEAN	gUIActionModeChangeDueToMouseOver;


void GetRTMousePositionInput( UINT32 *puiNewEvent )
{
	UINT16						usMapPos;
	static UINT16			usOldMapPos = 0;
	static UINT32			uiMoveTargetSoldierId = NO_SOLDIER;
	SOLDIERTYPE								 *pSoldier;
	static BOOLEAN		fOnValidGuy = FALSE;


	if (!GetMouseMapPos( &usMapPos ) )
	{
		return;
	}

  if ( gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		// Handle highlighting stuff
		HandleObjectHighlighting( );

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

				if ( usMapPos != usOldMapPos )
				{
					// Set off ALL move....
					gfUIAllMoveOn = FALSE;
				}

				uiMoveTargetSoldierId = NO_SOLDIER;

				// Check for being on terrain
				if(	GetSoldier( &pSoldier, gusSelectedSoldier ) )
				{
						UINT16 usItem;
						UINT8					ubItemCursor;

						 // Alrighty, check what's in our hands = if a 'friendly thing', like med kit, look for our own guys
						 usItem = pSoldier->inv[HANDPOS].usItem;

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

			case ACTION_MODE:

				// First check if we are on a guy, if so, make selected if it's ours
				// Check if the guy is visible
				guiUITargetSoldierId = NOBODY;

				fOnValidGuy = FALSE;

				if ( gfUIFullTargetFound  )
				//if ( gfUIFullTargetFound )
				{
						if ( IsValidTargetMerc( (UINT8)gusUIFullTargetID ) )
						{
							 guiUITargetSoldierId = gusUIFullTargetID;

							 if ( MercPtrs[ gusUIFullTargetID ]->bTeam != gbPlayerNum )
							 {
								fOnValidGuy = TRUE;
							 }
							 else
							 {
									if ( gUIActionModeChangeDueToMouseOver )
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

				// DONOT CANCEL IF BURST
				if(	GetSoldier( &pSoldier, gusSelectedSoldier ) )
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



