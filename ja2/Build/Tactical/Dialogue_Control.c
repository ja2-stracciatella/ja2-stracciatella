#include "SGP.h"
#include "Soldier_Control.h"
#include "Encrypted_File.h"
#include "Faces.h"
#include "WCheck.h"
#include "Gap.h"
#include "Overhead.h"
#include "Sound_Control.h"
#include "Dialogue_Control.h"
#include "Message.h"
#include "Render_Dirty.h"
#include "Soldier_Profile.h"
#include "WordWrap.h"
#include "SysUtil.h"
#include "VObject_Blitters.h"
#include "AIMMembers.h"
#include "Mercs.h"
#include "Interface_Dialogue.h"
#include "MercTextBox.h"
#include "RenderWorld.h"
#include "Soldier_Macros.h"
#include "Squads.h"
#include "ScreenIDs.h"
#include "Interface_Utils.h"
#include "StrategicMap.h"
#include "PreBattle_Interface.h"
#include "Game_Clock.h"
#include "Quests.h"
#include "Cursors.h"
#include "GameScreen.h"
#include "Random.h"
#include "Map_Screen_Helicopter.h"
#include "GameSettings.h"
#include "ShopKeeper_Interface.h"
#include "Map_Screen_Interface.h"
#include "Text.h"
#include "Merc_Contract.h"
#include "History.h"
#include "Town_Militia.h"
#include "Meanwhile.h"
#include "SkillCheck.h"
#include "Interface_Control.h"
#include "Finances.h"
#include "Civ_Quotes.h"
#include "Map_Screen_Interface_Map.h"
#include "OppList.h"
#include "AI.h"
#include "WorldMan.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Campaign.h"
#include "End_Game.h"
#include "LOS.h"
#include "QArray.h"
#include "JAScreens.h"
#include "Video.h"
#include "SoundMan.h"
#include "MemMan.h"
#include "Button_System.h"
#include "FileMan.h"
#include "Container.h"


#define		DIALOGUESIZE					480
#define   QUOTE_MESSAGE_SIZE		520

#define		TALK_PANEL_FACE_X				6
#define		TALK_PANEL_FACE_Y				9
#define		TALK_PANEL_NAME_X				5
#define		TALK_PANEL_NAME_Y				114
#define		TALK_PANEL_NAME_WIDTH		92
#define		TALK_PANEL_NAME_HEIGHT	15
#define		TALK_PANEL_MENU_STARTY	8
#define		TALK_PANEL_MENU_HEIGHT	24
#define		TALK_MENU_WIDTH					96
#define		TALK_MENU_HEIGHT				16

#define		DIALOGUE_DEFAULT_SUBTITLE_WIDTH		200
#define		TEXT_DELAY_MODIFIER			60


typedef struct
{
	UINT16	usQuoteNum;
	UINT8		ubCharacterNum;
	INT8		bUIHandlerID;
	INT32		iFaceIndex;
	INT32		iTimeStamp;
	UINT32	uiSpecialEventFlag;
	UINT32	uiSpecialEventData;
	UINT32	uiSpecialEventData2;
	UINT32	uiSpecialEventData3;
	UINT32	uiSpecialEventData4;
	BOOLEAN	fFromSoldier;
	BOOLEAN	fDelayed;
	BOOLEAN fPauseTime;
} DIALOGUE_Q_STRUCT, *DIALOGUE_Q_STRUCT_PTR;

extern BOOLEAN gfSKIScreenExit;
extern SOLDIERTYPE *pProcessingSoldier;
extern BOOLEAN fProcessingAMerc;
extern UINT32 guiPendingScreen;
extern BOOLEAN fReDrawFace;
extern BOOLEAN gfWaitingForTriggerTimer;


BOOLEAN fExternFacesLoaded = FALSE;

UINT32 uiExternalStaticNPCFaces[ NUMBER_OF_EXTERNAL_NPC_FACES ];
UINT32 uiExternalFaceProfileIds[ NUMBER_OF_EXTERNAL_NPC_FACES ]=
{
	97,
	106,
	148,
	156,
	157,
	158,
};

UINT8	gubMercValidPrecedentQuoteID[ NUMBER_VALID_MERC_PRECEDENT_QUOTES ] =
					{ 80, 81, 82, 83, 86, 87, 88, 95, 97, 99, 100, 101, 102 };


extern INT32 iInterfaceDialogueBox;
extern BOOLEAN	gfRerenderInterfaceFromHelpText;
extern UINT32 guiSKI_TransactionButton;

UINT16	gusStopTimeQuoteList[ ] =
{
	QUOTE_BOOBYTRAP_ITEM,
	QUOTE_SUSPICIOUS_GROUND
};

UINT8							gubNumStopTimeQuotes = 2;


// QUEUE UP DIALOG!
#define		INITIAL_Q_SIZE				10
HQUEUE		ghDialogueQ						= NULL;
FACETYPE	*gpCurrentTalkingFace	= NULL;
UINT8			gubCurrentTalkingID   = NO_PROFILE;
INT8			gbUIHandlerID;

INT32				giNPCReferenceCount = 0;
INT32				giNPCSpecialReferenceCount = 0;

INT16       gsExternPanelXPosition     = DEFAULT_EXTERN_PANEL_X_POS;
INT16       gsExternPanelYPosition     = DEFAULT_EXTERN_PANEL_Y_POS;

BOOLEAN			gfDialogueQueuePaused = FALSE;
UINT16			gusSubtitleBoxWidth;
UINT16			gusSubtitleBoxHeight;
INT32				giTextBoxOverlay = -1;
BOOLEAN			gfFacePanelActive = FALSE;
UINT32			guiScreenIDUsedWhenUICreated;
wchar_t				gzQuoteStr[ QUOTE_MESSAGE_SIZE ];
MOUSE_REGION	gTextBoxMouseRegion;
MOUSE_REGION	gFacePopupMouseRegion;
BOOLEAN				gfUseAlternateDialogueFile = FALSE;

// set the top position value for merc dialogue pop up boxes
INT16 gsTopPosition = 20;


INT32 iDialogueBox = -1;


extern void HandlePendingInitConv( );
extern void DrawFace( INT16 sCharNumber );

// the next said quote will pause time
BOOLEAN fPausedTimeDuringQuote = FALSE;
BOOLEAN fWasPausedDuringDialogue = FALSE;
extern BOOLEAN gfLockPauseState;

INT8	gubLogForMeTooBleeds = FALSE;


// has the text region been created?
BOOLEAN fTextBoxMouseRegionCreated = FALSE;
BOOLEAN	fExternFaceBoxRegionCreated = FALSE;

// due to last quote system?
BOOLEAN fDialogueBoxDueToLastMessage = FALSE;

// last quote timers
UINT32 guiDialogueLastQuoteTime = 0;
UINT32 guiDialogueLastQuoteDelay = 0;


extern BOOLEAN ContinueDialogue(SOLDIERTYPE *pSoldier, BOOLEAN fDone );
extern	BOOLEAN		DoSkiMessageBox( UINT8 ubStyle, const wchar_t *zString, UINT32 uiExitScreen, UINT8 ubFlags, MSGBOX_CALLBACK ReturnCallback );



void UnPauseGameDuringNextQuote( void )
{
	fPausedTimeDuringQuote = FALSE;
}


static void PauseTimeDuringNextQuote(void)
{
	fPausedTimeDuringQuote = TRUE;
}

BOOLEAN DialogueActive( )
{
	if ( gpCurrentTalkingFace != NULL )
	{
		return( TRUE );
	}

	return( FALSE );
}

BOOLEAN InitalizeDialogueControl()
{
	ghDialogueQ = CreateQueue( INITIAL_Q_SIZE, sizeof( DIALOGUE_Q_STRUCT_PTR ) );

	// Initalize subtitle popup box
	//

	giNPCReferenceCount = 0;


	if ( ghDialogueQ == NULL )
	{
		return( FALSE );
	}
	else
	{
		return( TRUE );
	}
}

void ShutdownDialogueControl()
{
  if( ghDialogueQ != NULL )
	{
		// Empty
		EmptyDialogueQueue( );

		// Delete
		DeleteQueue( ghDialogueQ );
		ghDialogueQ=NULL;
	}

	// shutdown external static NPC faces
	ShutdownStaticExternalNPCFaces( );

	// gte rid of portraits for cars
	UnLoadCarPortraits( );
	//
}



void InitalizeStaticExternalNPCFaces( void )
{
	INT32 iCounter = 0;
	// go and grab all external NPC faces that are needed for the game who won't exist as soldiertypes

	if( fExternFacesLoaded == TRUE )
	{
		return;
	}

	fExternFacesLoaded = TRUE;

	for( iCounter = 0; iCounter < NUMBER_OF_EXTERNAL_NPC_FACES; iCounter++ )
	{
		uiExternalStaticNPCFaces[ iCounter ] = ( UINT32 )InitFace( ( UINT8 )( uiExternalFaceProfileIds[ iCounter ] ), NOBODY, FACE_FORCE_SMALL );
	}
}

void ShutdownStaticExternalNPCFaces( void )
{
	INT32 iCounter = 0;

	if( fExternFacesLoaded == FALSE )
	{
		return;
	}

	fExternFacesLoaded = FALSE;

	// remove all external npc faces
	for( iCounter = 0; iCounter < NUMBER_OF_EXTERNAL_NPC_FACES; iCounter++ )
	{
		DeleteFace( uiExternalStaticNPCFaces[ iCounter ] );
	}
}


void EmptyDialogueQueue( )
{
	// If we have anything left in the queue, remove!
  if( ghDialogueQ != NULL )
	{
/*
DEF:  commented out because the Queue system ?? uses a contiguous memory block ??? for the queue
	so you cant delete a single node.  The DeleteQueue, below, will free the entire memory block

		numDialogueItems = QueueSize( ghDialogueQ );

		for ( cnt = numDialogueItems-1; cnt >= 0; cnt-- )
		{
			if ( PeekQueue( ghDialogueQ, &QItem ) )
			{
					MemFree( QItem );
			}
		}
*/

		// Delete list
		DeleteQueue( ghDialogueQ );
		ghDialogueQ=NULL;

		// Recreate list
		ghDialogueQ = CreateQueue( INITIAL_Q_SIZE, sizeof( DIALOGUE_Q_STRUCT_PTR ) );

	}

  gfWaitingForTriggerTimer = FALSE;
}


BOOLEAN DialogueQueueIsEmpty( )
{
	INT32										numDialogueItems;

  if( ghDialogueQ != NULL )
	{
		numDialogueItems = QueueSize( ghDialogueQ );

		if ( numDialogueItems == 0 )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}


BOOLEAN	DialogueQueueIsEmptyOrSomebodyTalkingNow( )
{
	if ( gpCurrentTalkingFace != NULL )
	{
		return( FALSE );
	}

	if ( !DialogueQueueIsEmpty( ) )
	{
		return( FALSE );
	}

	return( TRUE );
}

void DialogueAdvanceSpeech( )
{
	// Shut them up!
	InternalShutupaYoFace( gpCurrentTalkingFace->iID, FALSE );
}


void StopAnyCurrentlyTalkingSpeech( )
{
	// ATE; Make sure guys stop talking....
	if ( gpCurrentTalkingFace != NULL )
	{
		InternalShutupaYoFace( gpCurrentTalkingFace->iID, TRUE );
	}
}


static void CreateTalkingUI(INT8 bUIHandlerID, INT32 iFaceIndex, UINT8 ubCharacterNum, SOLDIERTYPE* pSoldier, wchar_t* zQuoteStr, size_t Length);


// ATE: Handle changes like when face goes from
// 'external' to on the team panel...
void HandleDialogueUIAdjustments( )
{
	SOLDIERTYPE							*pSoldier;

	// OK, check if we are still taking
	if ( gpCurrentTalkingFace != NULL )
	{
		if ( gpCurrentTalkingFace->fTalking )
		{
			// ATE: Check for change in state for the guy currently talking on 'external' panel....
			if ( gfFacePanelActive )
			{
				pSoldier = FindSoldierByProfileID( gubCurrentTalkingID, FALSE );

				if ( pSoldier )
				{
					if ( 0 )
					{
						// A change in plans here...
						// We now talk through the interface panel...
						if ( gpCurrentTalkingFace->iVideoOverlay != -1 )
						{
							RemoveVideoOverlay( gpCurrentTalkingFace->iVideoOverlay );
							gpCurrentTalkingFace->iVideoOverlay = -1;
						}
						gfFacePanelActive = FALSE;

						RemoveVideoOverlay( giTextBoxOverlay );
						giTextBoxOverlay = -1;

						if ( fTextBoxMouseRegionCreated )
						{
							MSYS_RemoveRegion( &gTextBoxMouseRegion );
							fTextBoxMouseRegionCreated = FALSE;
						}


						// Setup UI again!
						CreateTalkingUI( gbUIHandlerID, pSoldier->iFaceIndex, pSoldier->ubProfile, pSoldier, gzQuoteStr, lengthof(gzQuoteStr));
					}
				}
			}
		}
	}
}


static void CheckForStopTimeQuotes(UINT16 usQuoteNum);
static BOOLEAN ExecuteCharacterDialogue(UINT8 ubCharacterNum, UINT16 usQuoteNum, INT32 iFaceIndex, UINT8 bUIHandlerID, BOOLEAN fFromSoldier);
static void HandleTacticalSpeechUI(UINT8 ubCharacterNum, INT32 iFaceIndex);


void HandleDialogue( )
{
	INT32 iQSize;
	DIALOGUE_Q_STRUCT				*QItem;
	static BOOLEAN					fOldEngagedInConvFlagOn = FALSE;
	BOOLEAN fDoneTalking = FALSE;
	SOLDIERTYPE *pSoldier = NULL;
	CHAR16	zText[ 512 ];
	CHAR16	zMoney[128];

	// we don't want to just delay action of some events, we want to pause the whole queue, regardless of the event
	if( gfDialogueQueuePaused )
	{
		return;
	}

	iQSize = QueueSize( ghDialogueQ );


	if ( iQSize == 0 && gpCurrentTalkingFace == NULL )
	{
		HandlePendingInitConv( );
	}

	HandleCivQuote( );

	// Alrighty, check for a change in state, do stuff appropriately....
	// Turned on
	if ( fOldEngagedInConvFlagOn == FALSE && ( gTacticalStatus.uiFlags & ENGAGED_IN_CONV ) )
	{
		// OK, we have just entered...
		fOldEngagedInConvFlagOn = TRUE;

		// pause game..
		PauseGame();
		LockPauseState( 14 );
	}
	else if ( fOldEngagedInConvFlagOn == TRUE && !( gTacticalStatus.uiFlags & ENGAGED_IN_CONV ) )
	{
		// OK, we left...
		fOldEngagedInConvFlagOn = FALSE;

		// Unpause game..
		UnLockPauseState();
		UnPauseGame();

		// if we're exiting boxing with the UI lock set then DON'T OVERRIDE THIS!
		if ( !(gTacticalStatus.bBoxingState == WON_ROUND || gTacticalStatus.bBoxingState == LOST_ROUND || gTacticalStatus.bBoxingState == DISQUALIFIED) && !( gTacticalStatus.uiFlags & IGNORE_ENGAGED_IN_CONV_UI_UNLOCK ) )
		{
			guiPendingOverrideEvent = LU_ENDUILOCK;
			HandleTacticalUI( );

			// ATE: If this is NOT the player's turn.. engage AI UI lock!
			if ( gTacticalStatus.ubCurrentTeam != gbPlayerNum )
			{
				// Setup locked UI
				guiPendingOverrideEvent = LU_BEGINUILOCK;
				HandleTacticalUI( );
			}
		}

		gTacticalStatus.uiFlags &= ( ~IGNORE_ENGAGED_IN_CONV_UI_UNLOCK );
	}


	if ( gTacticalStatus.uiFlags & ENGAGED_IN_CONV )
	{
		// Are we in here because of the dialogue system up?
		if ( !gfInTalkPanel )
		{
			// ATE: NOT if we have a message box pending....
			if ( guiPendingScreen != MSG_BOX_SCREEN && guiCurrentScreen != MSG_BOX_SCREEN  )
			{
				// No, so we should lock the UI!
				guiPendingOverrideEvent = LU_BEGINUILOCK;
				HandleTacticalUI( );
			}
		}
	}

	// OK, check if we are still taking
	if ( gpCurrentTalkingFace != NULL )
	{
		if ( gpCurrentTalkingFace->fTalking )
		{
			// ATE: OK, MANAGE THE DISPLAY OF OUR CURRENTLY ACTIVE FACE IF WE / IT CHANGES STATUS
			// THINGS THAT CAN CHANGE STATUS:
			//		CHANGE TO MAPSCREEN
			//		CHANGE TO GAMESCREEN
			//		CHANGE IN MERC STATUS TO BE IN A SQUAD
			//    CHANGE FROM TEAM TO INV INTERFACE

			// Where are we and where did this face once exist?
			if ( guiScreenIDUsedWhenUICreated == GAME_SCREEN && guiCurrentScreen == MAP_SCREEN )
			{
				// GO FROM GAMESCREEN TO MAPSCREEN
				// REMOVE OLD UI
				// Set face inactive!
				//gpCurrentTalkingFace->fCanHandleInactiveNow = TRUE;
				//SetAutoFaceInActive( gpCurrentTalkingFace->iID );
				//gfFacePanelActive = FALSE;

				// delete face panel if there is one!
				if ( gfFacePanelActive )
				{
					// Set face inactive!
					if ( gpCurrentTalkingFace->iVideoOverlay != -1 )
					{
						RemoveVideoOverlay( gpCurrentTalkingFace->iVideoOverlay );
						gpCurrentTalkingFace->iVideoOverlay = -1;
					}

					if ( fExternFaceBoxRegionCreated )
					{
						fExternFaceBoxRegionCreated = FALSE;
						MSYS_RemoveRegion(&(gFacePopupMouseRegion) );
					}

					// Set face inactive....
					gpCurrentTalkingFace->fCanHandleInactiveNow = TRUE;
					SetAutoFaceInActive( gpCurrentTalkingFace->iID );
					HandleTacticalSpeechUI( gubCurrentTalkingID, gpCurrentTalkingFace->iID );

          // ATE: Force mapscreen to set face active again.....
        	fReDrawFace = TRUE;
      		DrawFace( bSelectedInfoChar );

					gfFacePanelActive = FALSE;
				}

				guiScreenIDUsedWhenUICreated = guiCurrentScreen;
			}
			else if ( guiScreenIDUsedWhenUICreated == MAP_SCREEN && guiCurrentScreen == GAME_SCREEN )
			{
				HandleTacticalSpeechUI( gubCurrentTalkingID, gpCurrentTalkingFace->iID );
				guiScreenIDUsedWhenUICreated = guiCurrentScreen;
			}
			return;
		}
		else
		{
			// Check special flags
			// If we are done, check special face flag for trigger NPC!
			if ( gpCurrentTalkingFace->uiFlags & FACE_PCTRIGGER_NPC )
			{
				 // Decrement refrence count...
				 giNPCReferenceCount--;

				 TriggerNPCRecord( (UINT8)gpCurrentTalkingFace->uiUserData1, (UINT8)gpCurrentTalkingFace->uiUserData2 );
				 //Reset flag!
				 gpCurrentTalkingFace->uiFlags &= (~FACE_PCTRIGGER_NPC );
			}

			if ( gpCurrentTalkingFace->uiFlags & FACE_MODAL )
			{
			  gpCurrentTalkingFace->uiFlags &= (~FACE_MODAL );

				EndModalTactical( );

				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Ending Modal Tactical Quote." );

			}

			if ( gpCurrentTalkingFace->uiFlags & FACE_TRIGGER_PREBATTLE_INT )
			{
				UnLockPauseState();
				InitPreBattleInterface( (GROUP*)gpCurrentTalkingFace->uiUserData1, TRUE );
				//Reset flag!
				gpCurrentTalkingFace->uiFlags &= (~FACE_TRIGGER_PREBATTLE_INT );
			}

			gpCurrentTalkingFace = NULL;
			gubCurrentTalkingID	 = NO_PROFILE;
			gTacticalStatus.ubLastQuoteProfileNUm = NO_PROFILE;
			fDoneTalking = TRUE;
		}
	}

	if( ( fDoneTalking ) && ( fWasPausedDuringDialogue ) )
	{
		fWasPausedDuringDialogue = FALSE;

		// unlock pause state
		UnLockPauseState();
		UnPauseGame();

	}

	if ( iQSize == 0 )
	{

		if ( gfMikeShouldSayHi == TRUE )
		{
			SOLDIERTYPE * pMike;
			INT16	sPlayerGridNo;
			UINT8	ubPlayerID;

			pMike = FindSoldierByProfileID( MIKE, FALSE );
			if ( pMike )
			{
				sPlayerGridNo = ClosestPC( pMike, NULL );
				if (sPlayerGridNo != NOWHERE )
				{
					ubPlayerID = WhoIsThere2( sPlayerGridNo, 0 );
					if (ubPlayerID != NOBODY)
					{
						InitiateConversation( pMike, MercPtrs[ ubPlayerID ], NPC_INITIAL_QUOTE, 0 );
						gMercProfiles[ pMike->ubProfile ].ubMiscFlags2 |= PROFILE_MISC_FLAG2_SAID_FIRSTSEEN_QUOTE;
						// JA2Gold: special hack value of 2 to prevent dialogue from coming up more than once
						gfMikeShouldSayHi = 2;
					}
				}
			}
		}

		return;
	}

  // ATE: Remove any civ quotes....
  // ShutDownQuoteBoxIfActive( TRUE );

	// If here, pick current one from queue and play

	// Get new one
	RemfromQueue( ghDialogueQ, &QItem );

	// If we are in auto bandage, ignore any quotes!
	if ( gTacticalStatus.fAutoBandageMode )
	{
		if( QItem -> fPauseTime )
		{
			UnLockPauseState();
			UnPauseGame();
		}

		// Delete memory
		MemFree( QItem );
		return;
	}

	// Check time delay

	// Alrighty, check if this one is to be delayed until we gain control.
	// If so, place it back in!
	if ( QItem->fDelayed )
	{
		// Are we not in our turn and not interrupted
		if ( gTacticalStatus.ubCurrentTeam != gbPlayerNum )
		{
			//Place back in!
			// Add to queue
			ghDialogueQ = AddtoQueue( ghDialogueQ, &QItem );

			return;
		}
	}

	// ATE: OK: If a battle sound, and delay value was given, set time stamp
	// now...
	if ( QItem->uiSpecialEventFlag == DIALOGUE_SPECIAL_EVENT_DO_BATTLE_SND )
	{
		if ( QItem->uiSpecialEventData2 != 0 )
		{
			if ( ( GetJA2Clock( ) - QItem->iTimeStamp ) < QItem->uiSpecialEventData2 )
			{
				//Place back in!
				// Add to queue
				ghDialogueQ = AddtoQueue( ghDialogueQ, &QItem );

				return;
			}
		}
	}

	// Try to find soldier...
	pSoldier = FindSoldierByProfileID( QItem->ubCharacterNum, TRUE );

	if ( pSoldier != NULL )
	{
		if ( SoundIsPlaying( pSoldier->uiBattleSoundID ) )
		{
			//Place back in!
			// Add to queue
			ghDialogueQ = AddtoQueue( ghDialogueQ, &QItem );

			return;
		}
	}

	if ( (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) && ( QItem->uiSpecialEventFlag == 0 ) )
	{
		QItem-> fPauseTime = TRUE;
	}

	if( QItem-> fPauseTime )
	{
		if( GamePaused( ) == FALSE )
		{
			PauseGame();
			LockPauseState( 15 );
			fWasPausedDuringDialogue = TRUE;
		}
	}

	// Now play first item in queue
	// If it's not a 'special' dialogue event, continue
	if ( QItem->uiSpecialEventFlag == 0 )
	{
		if( pSoldier )
		{
			// wake grunt up to say
			if( pSoldier->fMercAsleep )
			{
				pSoldier->fMercAsleep = FALSE;

				// refresh map screen
				fCharacterInfoPanelDirty = TRUE;
				fTeamPanelDirty = TRUE;

				// allow them to go back to sleep
				TacticalCharacterDialogueWithSpecialEvent( pSoldier, QItem->usQuoteNum, DIALOGUE_SPECIAL_EVENT_SLEEP, 1,0 );
			}
		}

		gTacticalStatus.ubLastQuoteSaid = (UINT8)QItem->usQuoteNum;
		gTacticalStatus.ubLastQuoteProfileNUm = (UINT8)QItem->ubCharacterNum;

		// Setup face pointer
		gpCurrentTalkingFace = &gFacesData[ QItem->iFaceIndex ];
		gubCurrentTalkingID   = QItem->ubCharacterNum;

		ExecuteCharacterDialogue( QItem->ubCharacterNum, QItem->usQuoteNum, QItem->iFaceIndex, QItem->bUIHandlerID, QItem->fFromSoldier );

	}
	else if( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SKIP_A_FRAME )
	{

	}
	else if( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_LOCK_INTERFACE )
	{
		// locking or unlocking?
		if( QItem->uiSpecialEventData )
		{
			switch( QItem->uiSpecialEventData2 )
			{
				case( MAP_SCREEN ):
					fLockOutMapScreenInterface = TRUE;
				break;
			}
		}
		else
		{
			switch( QItem->uiSpecialEventData2 )
			{
				case( MAP_SCREEN ):
					fLockOutMapScreenInterface = FALSE;
				break;
			}
		}
	}
	else if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_REMOVE_EPC )
	{
		gMercProfiles[ (UINT8) QItem->uiSpecialEventData ].ubMiscFlags &= ~PROFILE_MISC_FLAG_FORCENPCQUOTE;
		UnRecruitEPC( (UINT8) QItem->uiSpecialEventData );
		ReBuildCharactersList();
	}
	else if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CONTRACT_WANTS_TO_RENEW )
	{
		HandleMercIsWillingToRenew( (UINT8)QItem->uiSpecialEventData );
	}
	else if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CONTRACT_NOGO_TO_RENEW )
	{
		HandleMercIsNotWillingToRenew( (UINT8)QItem->uiSpecialEventData );
	}
	else
	{
		if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_USE_ALTERNATE_FILES )
		{
			gfUseAlternateDialogueFile = TRUE;

			// Setup face pointer
			gpCurrentTalkingFace = &gFacesData[ QItem->iFaceIndex ];
			gubCurrentTalkingID   = QItem->ubCharacterNum;

			ExecuteCharacterDialogue( QItem->ubCharacterNum, QItem->usQuoteNum, QItem->iFaceIndex, QItem->bUIHandlerID, QItem->fFromSoldier );

			gfUseAlternateDialogueFile = FALSE;

		}
		// We could have a special flag, but dialogue as well
		else if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_PCTRIGGERNPC )
		{
			// Setup face pointer
			gpCurrentTalkingFace = &gFacesData[ QItem->iFaceIndex ];
			gubCurrentTalkingID   = QItem->ubCharacterNum;

			ExecuteCharacterDialogue( QItem->ubCharacterNum, QItem->usQuoteNum, QItem->iFaceIndex, QItem->bUIHandlerID, QItem->fFromSoldier );

			// Setup face with data!
			gpCurrentTalkingFace->uiFlags				|= FACE_PCTRIGGER_NPC;
			gpCurrentTalkingFace->uiUserData1		 = QItem->uiSpecialEventData;
			gpCurrentTalkingFace->uiUserData2		 = QItem->uiSpecialEventData2;

		}
		else if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SHOW_CONTRACT_MENU )
		{
			// Setup face pointer
			// ATE: THis is working with MARK'S STUFF :(
			// Need this stuff so that bSelectedInfoChar is set...
			SetInfoChar( pSoldier->ubID );

			fShowContractMenu = TRUE;
			RebuildContractBoxForMerc( pSoldier );
			bSelectedContractChar = bSelectedInfoChar;
			pProcessingSoldier = pSoldier;
			fProcessingAMerc = TRUE;
		}
		else if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_DO_BATTLE_SND )
		{
			// grab soldier ptr from profile ID
			pSoldier = FindSoldierByProfileID( QItem->ubCharacterNum, FALSE );

			// Do battle snounds......
			if ( pSoldier )
			{
				InternalDoMercBattleSound( pSoldier,  (UINT8)QItem->uiSpecialEventData, 0 );
			}
		}

		if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SIGNAL_ITEM_LOCATOR_START )
		{
			// Turn off item lock for locators...
			gTacticalStatus.fLockItemLocators = FALSE;

			// Slide to location!
			SlideToLocation( 0,  (UINT16)QItem->uiSpecialEventData );

			gpCurrentTalkingFace = &gFacesData[ QItem->iFaceIndex ];
			gubCurrentTalkingID   = QItem->ubCharacterNum;

			ExecuteCharacterDialogue( QItem->ubCharacterNum, QItem->usQuoteNum, QItem->iFaceIndex, QItem->bUIHandlerID, QItem->fFromSoldier );

		}

		if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_ENABLE_AI )
		{
			//OK, allow AI to work now....
			UnPauseAI();
		}

		if( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_TRIGGERPREBATTLEINTERFACE )
		{
			UnLockPauseState();
			InitPreBattleInterface( (GROUP*)QItem->uiSpecialEventData, TRUE );
		}
		if( QItem->uiSpecialEventFlag & DIALOGUE_ADD_EVENT_FOR_SOLDIER_UPDATE_BOX )
		{
			INT32 iReason = 0;
			SOLDIERTYPE *pUpdateSoldier = NULL;

			iReason = QItem->uiSpecialEventData;

			switch( iReason )
			{
				case( UPDATE_BOX_REASON_ADDSOLDIER ):
					pUpdateSoldier = &Menptr[ QItem->uiSpecialEventData2 ];
					if( pUpdateSoldier->bActive == TRUE )
					{
						AddSoldierToUpdateBox( pUpdateSoldier );
					}
				break;
				case( UPDATE_BOX_REASON_SET_REASON ):
					SetSoldierUpdateBoxReason( QItem->uiSpecialEventData2 );
				break;
				case( UPDATE_BOX_REASON_SHOW_BOX ):
					ShowUpdateBox( );
				break;
			}
		}
		if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_BEGINPREBATTLEINTERFACE )
		{
			// Setup face pointer
			gpCurrentTalkingFace = &gFacesData[ QItem->iFaceIndex ];
			gubCurrentTalkingID   = QItem->ubCharacterNum;

			ExecuteCharacterDialogue( QItem->ubCharacterNum, QItem->usQuoteNum, QItem->iFaceIndex, QItem->bUIHandlerID, QItem->fFromSoldier );

			// Setup face with data!
			gpCurrentTalkingFace->uiFlags				|= FACE_TRIGGER_PREBATTLE_INT;
			gpCurrentTalkingFace->uiUserData1		 = QItem->uiSpecialEventData;
			gpCurrentTalkingFace->uiUserData2		 = QItem->uiSpecialEventData2;
		}

		if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SHOPKEEPER )
		{

			if( QItem->uiSpecialEventData < 3 )
			{
				// post a notice if the player wants to withdraw money from thier account to cover the difference?
				swprintf( zMoney, lengthof(zMoney), L"%d", QItem->uiSpecialEventData2 );
				InsertCommasForDollarFigure( zMoney );
				InsertDollarSignInToString( zMoney );
			}

			switch( QItem->uiSpecialEventData  )
			{
				case( 0 ):
						swprintf( zText, lengthof(zText), SkiMessageBoxText[ SKI_SHORT_FUNDS_TEXT ], zMoney );

						//popup a message stating the player doesnt have enough money
						DoSkiMessageBox( MSG_BOX_BASIC_STYLE, zText, SHOPKEEPER_SCREEN, MSG_BOX_FLAG_OK, ConfirmDontHaveEnoughForTheDealerMessageBoxCallBack );
				break;
				case( 1 ):
						//if the player is trading items
						swprintf( zText, lengthof(zText), SkiMessageBoxText[ SKI_QUESTION_TO_DEDUCT_MONEY_FROM_PLAYERS_ACCOUNT_TO_COVER_DIFFERENCE ], zMoney );

						//ask them if we should deduct money out the players account to cover the difference
						DoSkiMessageBox( MSG_BOX_BASIC_STYLE, zText, SHOPKEEPER_SCREEN, MSG_BOX_FLAG_YESNO, ConfirmToDeductMoneyFromPlayersAccountMessageBoxCallBack );

				break;
				case( 2 ):
						swprintf( zText, lengthof(zText), SkiMessageBoxText[ SKI_QUESTION_TO_DEDUCT_MONEY_FROM_PLAYERS_ACCOUNT_TO_COVER_COST ], zMoney );

						//ask them if we should deduct money out the players account to cover the difference
						DoSkiMessageBox( MSG_BOX_BASIC_STYLE, zText, SHOPKEEPER_SCREEN, MSG_BOX_FLAG_YESNO, ConfirmToDeductMoneyFromPlayersAccountMessageBoxCallBack );
				break;
				case( 3 ):
					// this means a dialogue event is in progress
					giShopKeepDialogueEventinProgress = QItem->uiSpecialEventData2;
				break;
				case( 4 ):
					// this means a dialogue event has ended
					giShopKeepDialogueEventinProgress = -1;
				break;
				case( 5 ):
					// this means a dialogue event has ended
					gfSKIScreenExit = TRUE;
				break;

				case( 6 ):
					if( guiCurrentScreen == SHOPKEEPER_SCREEN )
					{
						DisableButton( guiSKI_TransactionButton );
					}
				break;
				case( 7 ):
					if( guiCurrentScreen == SHOPKEEPER_SCREEN )
					{
						EnableButton( guiSKI_TransactionButton );
					}
				break;
			}

		}

		if( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_EXIT_MAP_SCREEN )
		{
			// select sector
			ChangeSelectedMapSector( ( INT16 )QItem->uiSpecialEventData, ( INT16 )QItem->uiSpecialEventData2, ( INT8 )QItem->uiSpecialEventData3 );
			RequestTriggerExitFromMapscreen( MAP_EXIT_TO_TACTICAL );
		}
		else if( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_DISPLAY_STAT_CHANGE )
		{
			// grab soldier ptr from profile ID
			pSoldier = FindSoldierByProfileID( QItem->ubCharacterNum, FALSE );

			if ( pSoldier )
			{
				CHAR16 wTempString[ 128 ];

				// tell player about stat increase
				BuildStatChangeString( wTempString, lengthof(wTempString), pSoldier->name, ( BOOLEAN ) QItem->uiSpecialEventData, ( INT16 ) QItem->uiSpecialEventData2, ( UINT8 ) QItem->uiSpecialEventData3 );
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, wTempString );
			}
		}
		else if( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_UNSET_ARRIVES_FLAG )
		{
			gTacticalStatus.bMercArrivingQuoteBeingUsed = FALSE;
		}

		/*
		else if( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_DISPLAY_INVASION_MESSAGE )
		{
			HandlePlayerNotifyInvasionByEnemyForces( (INT16)(QItem->uiSpecialEventData % MAP_WORLD_X), (INT16)(QItem->uiSpecialEventData / MAP_WORLD_X), 0, NULL );
		}
		*/
		else if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SKYRIDERMAPSCREENEVENT )
		{
			// Setup face pointer
			gpCurrentTalkingFace = &gFacesData[ QItem->iFaceIndex ];
			gubCurrentTalkingID   = QItem->ubCharacterNum;

			// handle the monologue event
			HandleSkyRiderMonologueEvent( QItem->uiSpecialEventData, QItem->uiSpecialEventData2 );
		}

		if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_MINESECTOREVENT )
		{
			// Setup face pointer
			gpCurrentTalkingFace = &gFacesData[ QItem->iFaceIndex ];
			gubCurrentTalkingID   = QItem->ubCharacterNum;

			// set up the mine highlgith events
			SetUpAnimationOfMineSectors( QItem->uiSpecialEventData );
		}

		//Switch on our special events
		if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_GIVE_ITEM )
		{
			if ( QItem->bUIHandlerID == DIALOGUE_NPC_UI )
			{
				HandleNPCItemGiven( (UINT8)QItem->uiSpecialEventData, (OBJECTTYPE*)QItem->uiSpecialEventData2, (INT8)QItem->uiSpecialEventData3 );
			}
		}
		else if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_TRIGGER_NPC )
		{
			if ( QItem->bUIHandlerID == DIALOGUE_NPC_UI )
			{
				HandleNPCTriggerNPC( (UINT8)QItem->uiSpecialEventData, (UINT8)QItem->uiSpecialEventData2, (BOOLEAN)QItem->uiSpecialEventData3, (UINT8)QItem->uiSpecialEventData4 );
			}
		}
		else if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_GOTO_GRIDNO )
		{
			if ( QItem->bUIHandlerID == DIALOGUE_NPC_UI )
			{
				HandleNPCGotoGridNo( (UINT8)QItem->uiSpecialEventData, (UINT16)QItem->uiSpecialEventData2, (UINT8)QItem->uiSpecialEventData3 );
			}
		}
		else if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_DO_ACTION )
		{
			if ( QItem->bUIHandlerID == DIALOGUE_NPC_UI )
			{
				HandleNPCDoAction( (UINT8)QItem->uiSpecialEventData, (UINT16)QItem->uiSpecialEventData2, (UINT8)QItem->uiSpecialEventData3 );
			}
		}
		else if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CLOSE_PANEL )
		{
			if ( QItem->bUIHandlerID == DIALOGUE_NPC_UI )
			{
				HandleNPCClosePanel( );
			}
		}
		else if ( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SHOW_UPDATE_MENU )
		{
			SetUpdateBoxFlag( TRUE );
		}
		else if( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CONTINUE_TRAINING_MILITIA )
		{
			// grab soldier ptr from profile ID
			pSoldier = FindSoldierByProfileID( ( UINT8 )( QItem->uiSpecialEventData ), FALSE );

			// if soldier valid...
			if( pSoldier != NULL )
			{
				HandleInterfaceMessageForContinuingTrainingMilitia( pSoldier );
			}
		}
		else if( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_ENTER_MAPSCREEN )
		{
			if( !(guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
			{
				gfEnteringMapScreen = TRUE;
				fEnterMapDueToContract = TRUE;
			}
		}
		else if( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING )
		{
			// grab soldier ptr from profile ID
			pSoldier = FindSoldierByProfileID( QItem->ubCharacterNum, FALSE );

			// if soldier valid...
			if( pSoldier != NULL )
			{
				// .. remove the fired soldier again
				BeginStrategicRemoveMerc( pSoldier, (UINT8)QItem->uiSpecialEventData );
			}
		}
		else if( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING_NO_ASK_EQUIP )
		{
			// grab soldier ptr from profile ID
			pSoldier = FindSoldierByProfileID( QItem->ubCharacterNum, FALSE );

			// if soldier valid...
			if( pSoldier != NULL )
			{
				// .. remove the fired soldier again
				StrategicRemoveMerc( pSoldier );
			}
		}
		else if( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_MULTIPURPOSE )
		{
			if ( QItem->uiSpecialEventData & MULTIPURPOSE_SPECIAL_EVENT_DONE_KILLING_DEIDRANNA )
			{
				HandleDoneLastKilledQueenQuote( );
			}
			else if ( QItem->uiSpecialEventData & MULTIPURPOSE_SPECIAL_EVENT_TEAM_MEMBERS_DONE_TALKING )
			{
				HandleDoneLastEndGameQuote( );
			}
		}
		else if( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SLEEP )
		{
			// no soldier, leave now
			if( pSoldier == NULL )
			{
				return;
			}

			// wake merc up or put them back down?
			if( QItem->uiSpecialEventData == 1 )
			{
				pSoldier -> fMercAsleep = TRUE;
			}
			else
			{
				pSoldier -> fMercAsleep = FALSE;
			}

			// refresh map screen
			fCharacterInfoPanelDirty = TRUE;
			fTeamPanelDirty = TRUE;
		}
	}

	// grab soldier ptr from profile ID
	pSoldier = FindSoldierByProfileID( QItem->ubCharacterNum, FALSE );

	if ( pSoldier && pSoldier->bTeam == gbPlayerNum )
	{
		CheckForStopTimeQuotes( QItem->usQuoteNum );
	}

	if( QItem -> fPauseTime )
	{
		fWasPausedDuringDialogue = TRUE;
	}

	// Delete memory
	MemFree( QItem );
}


BOOLEAN DelayedTacticalCharacterDialogue( SOLDIERTYPE *pSoldier, UINT16 usQuoteNum )
{
	if ( pSoldier->ubProfile == NO_PROFILE )
	{
		return( FALSE );
	}

  if (pSoldier->bLife < CONSCIOUSNESS )
   return( FALSE );

	if ( pSoldier->uiStatusFlags & SOLDIER_GASSED )
		return( FALSE );

	if ( (AM_A_ROBOT( pSoldier )) )
	{
		return( FALSE );
	}

  if (pSoldier->bLife < OKLIFE && usQuoteNum != QUOTE_SERIOUSLY_WOUNDED )
   return( FALSE );

	if( pSoldier->bAssignment == ASSIGNMENT_POW )
	{
		return( FALSE );
	}

	return( CharacterDialogue( pSoldier->ubProfile, usQuoteNum, pSoldier->iFaceIndex, DIALOGUE_TACTICAL_UI, TRUE, TRUE ) );
}


BOOLEAN TacticalCharacterDialogueWithSpecialEvent( SOLDIERTYPE *pSoldier, UINT16 usQuoteNum, UINT32 uiFlag, UINT32 uiData1, UINT32 uiData2 )
{
	if ( pSoldier->ubProfile == NO_PROFILE )
	{
		return( FALSE );
	}

	if ( uiFlag != DIALOGUE_SPECIAL_EVENT_DO_BATTLE_SND && uiData1 != BATTLE_SOUND_DIE1 )
	{
		if (pSoldier->bLife < CONSCIOUSNESS )
		 return( FALSE );

		if ( pSoldier->uiStatusFlags & SOLDIER_GASSED )
			return( FALSE );

	}

	return( CharacterDialogueWithSpecialEvent( pSoldier->ubProfile, usQuoteNum, pSoldier->iFaceIndex, DIALOGUE_TACTICAL_UI, TRUE, FALSE, uiFlag, uiData1, uiData2 ) );
}


static BOOLEAN CharacterDialogueWithSpecialEventEx(UINT8 ubCharacterNum, UINT16 usQuoteNum, INT32 iFaceIndex, UINT8 bUIHandlerID, BOOLEAN fFromSoldier, BOOLEAN fDelayed, UINT32 uiFlag, UINT32 uiData1, UINT32 uiData2, UINT32 uiData3);


BOOLEAN TacticalCharacterDialogueWithSpecialEventEx( SOLDIERTYPE *pSoldier, UINT16 usQuoteNum, UINT32 uiFlag, UINT32 uiData1, UINT32 uiData2, UINT32 uiData3 )
{
	if ( pSoldier->ubProfile == NO_PROFILE )
	{
		return( FALSE );
	}

	if ( uiFlag != DIALOGUE_SPECIAL_EVENT_DO_BATTLE_SND && uiData1 != BATTLE_SOUND_DIE1 )
	{
		if (pSoldier->bLife < CONSCIOUSNESS )
		 return( FALSE );

		if ( pSoldier->uiStatusFlags & SOLDIER_GASSED )
			return( FALSE );

		if ( (AM_A_ROBOT( pSoldier )) )
		{
			return( FALSE );
		}

		if (pSoldier->bLife < OKLIFE && usQuoteNum != QUOTE_SERIOUSLY_WOUNDED )
		 return( FALSE );

		if( pSoldier->bAssignment == ASSIGNMENT_POW )
		{
			return( FALSE );
		}

	}

	return( CharacterDialogueWithSpecialEventEx( pSoldier->ubProfile, usQuoteNum, pSoldier->iFaceIndex, DIALOGUE_TACTICAL_UI, TRUE, FALSE, uiFlag, uiData1, uiData2, uiData3 ) );
}


BOOLEAN TacticalCharacterDialogue( SOLDIERTYPE *pSoldier, UINT16 usQuoteNum )
{
	if ( pSoldier->ubProfile == NO_PROFILE )
	{
		return( FALSE );
	}

	if ( AreInMeanwhile( ) )
	{
		return( FALSE );
	}

  if (pSoldier->bLife < CONSCIOUSNESS )
   return( FALSE );

  if (pSoldier->bLife < OKLIFE && usQuoteNum != QUOTE_SERIOUSLY_WOUNDED )
   return( FALSE );

	if ( pSoldier->uiStatusFlags & SOLDIER_GASSED )
		return( FALSE );

	if ( (AM_A_ROBOT( pSoldier )) )
	{
		return( FALSE );
	}

	if( pSoldier->bAssignment == ASSIGNMENT_POW )
	{
		return( FALSE );
	}

	// OK, let's check if this is the exact one we just played, if so, skip.
	if ( pSoldier->ubProfile == gTacticalStatus.ubLastQuoteProfileNUm &&
			 usQuoteNum == gTacticalStatus.ubLastQuoteSaid )
	{
		return( FALSE );
	}


	// If we are a robot, play the controller's quote!
	if ( pSoldier->uiStatusFlags & SOLDIER_ROBOT )
	{
		if ( CanRobotBeControlled( pSoldier ) )
		{
			return( TacticalCharacterDialogue( MercPtrs[ pSoldier->ubRobotRemoteHolderID ], usQuoteNum ) );
		}
		else
		{
			return( FALSE );
		}
	}

	if ( AM_AN_EPC( pSoldier ) && !(gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags & PROFILE_MISC_FLAG_FORCENPCQUOTE) )
		return( FALSE );

	// Check for logging of me too bleeds...
	if ( usQuoteNum == QUOTE_STARTING_TO_BLEED )
	{
		if ( gubLogForMeTooBleeds )
		{
			// If we are greater than one...
			if ( gubLogForMeTooBleeds > 1 )
			{
				//Replace with me too....
				usQuoteNum = QUOTE_ME_TOO;
			}
			gubLogForMeTooBleeds++;
		}
	}

	return( CharacterDialogue( pSoldier->ubProfile, usQuoteNum, pSoldier->iFaceIndex, DIALOGUE_TACTICAL_UI, TRUE, FALSE ) );
}

// This function takes a profile num, quote num, faceindex and a UI hander ID.
// What it does is queues up the dialog to be ultimately loaded/displayed
//				FACEINDEX
//						The face index is an index into an ACTIVE face. The face is considered to
//						be active, and if it's not, either that has to be handled by the UI handler
//						ir nothing will show.  What this function does is set the face to talking,
//						and the face sprite system should handle the rest.
//				bUIHandlerID
//						Because this could be used in any place, the UI handleID is used to differentiate
//						places in the game. For example, specific things happen in the tactical engine
//						that may not be the place where in the AIM contract screen uses.....

// NB;				The queued system is not yet implemented, but will be transpatent to the caller....


BOOLEAN CharacterDialogueWithSpecialEvent( UINT8 ubCharacterNum, UINT16 usQuoteNum, INT32 iFaceIndex, UINT8 bUIHandlerID, BOOLEAN fFromSoldier, BOOLEAN fDelayed, UINT32 uiFlag, UINT32 uiData1, UINT32 uiData2 )
{
	DIALOGUE_Q_STRUCT				*QItem;

	// Allocate new item
	QItem = MemAlloc( sizeof( DIALOGUE_Q_STRUCT ) );
	memset( QItem, 0, sizeof( DIALOGUE_Q_STRUCT ) );

	QItem->ubCharacterNum = ubCharacterNum;
	QItem->usQuoteNum			= usQuoteNum;
	QItem->iFaceIndex			= iFaceIndex;
	QItem->bUIHandlerID		= bUIHandlerID;
	QItem->iTimeStamp			= GetJA2Clock( );
	QItem->fFromSoldier		= fFromSoldier;
	QItem->fDelayed				= fDelayed;

	// Set flag for special event
	QItem->uiSpecialEventFlag		= uiFlag;
	QItem->uiSpecialEventData		= uiData1;
	QItem->uiSpecialEventData2	= uiData2;

	// Add to queue
	ghDialogueQ = AddtoQueue( ghDialogueQ, &QItem );

	if ( uiFlag & DIALOGUE_SPECIAL_EVENT_PCTRIGGERNPC )
	{
		// Increment refrence count...
		giNPCReferenceCount++;
	}

	return( TRUE );
}


// Do special event as well as dialogue!
static BOOLEAN CharacterDialogueWithSpecialEventEx(UINT8 ubCharacterNum, UINT16 usQuoteNum, INT32 iFaceIndex, UINT8 bUIHandlerID, BOOLEAN fFromSoldier, BOOLEAN fDelayed, UINT32 uiFlag, UINT32 uiData1, UINT32 uiData2, UINT32 uiData3)
{
	DIALOGUE_Q_STRUCT				*QItem;

	// Allocate new item
	QItem = MemAlloc( sizeof( DIALOGUE_Q_STRUCT ) );
	memset( QItem, 0, sizeof( DIALOGUE_Q_STRUCT ) );

	QItem->ubCharacterNum = ubCharacterNum;
	QItem->usQuoteNum			= usQuoteNum;
	QItem->iFaceIndex			= iFaceIndex;
	QItem->bUIHandlerID		= bUIHandlerID;
	QItem->iTimeStamp			= GetJA2Clock( );
	QItem->fFromSoldier		= fFromSoldier;
	QItem->fDelayed				= fDelayed;

	// Set flag for special event
	QItem->uiSpecialEventFlag		= uiFlag;
	QItem->uiSpecialEventData		= uiData1;
	QItem->uiSpecialEventData2	= uiData2;
	QItem->uiSpecialEventData3	= uiData3;

	// Add to queue
	ghDialogueQ = AddtoQueue( ghDialogueQ, &QItem );

	if ( uiFlag & DIALOGUE_SPECIAL_EVENT_PCTRIGGERNPC )
	{
		// Increment refrence count...
		giNPCReferenceCount++;
	}

	return( TRUE );
}


BOOLEAN CharacterDialogue( UINT8 ubCharacterNum, UINT16 usQuoteNum, INT32 iFaceIndex, UINT8 bUIHandlerID, BOOLEAN fFromSoldier, BOOLEAN fDelayed )
{
	DIALOGUE_Q_STRUCT				*QItem;

	// Allocate new item
	QItem = MemAlloc( sizeof( DIALOGUE_Q_STRUCT ) );
	memset( QItem, 0, sizeof( DIALOGUE_Q_STRUCT ) );

	QItem->ubCharacterNum = ubCharacterNum;
	QItem->usQuoteNum			= usQuoteNum;
	QItem->iFaceIndex			= iFaceIndex;
	QItem->bUIHandlerID		= bUIHandlerID;
	QItem->iTimeStamp			= GetJA2Clock( );
	QItem->fFromSoldier		= fFromSoldier;
	QItem->fDelayed				= fDelayed;

	// check if pause already locked, if so, then don't mess with it
	if( gfLockPauseState == FALSE )
	{
		QItem->fPauseTime     = fPausedTimeDuringQuote;
	}

	fPausedTimeDuringQuote = FALSE;

	// Add to queue
	ghDialogueQ = AddtoQueue( ghDialogueQ, &QItem );

	return( TRUE );
}


BOOLEAN SpecialCharacterDialogueEvent( UINT32 uiSpecialEventFlag, UINT32 uiSpecialEventData1, UINT32 uiSpecialEventData2, UINT32 uiSpecialEventData3, INT32 iFaceIndex, UINT8 bUIHandlerID )
{
	DIALOGUE_Q_STRUCT				*QItem;

	// Allocate new item
	QItem = MemAlloc( sizeof( DIALOGUE_Q_STRUCT ) );
	memset( QItem, 0, sizeof( DIALOGUE_Q_STRUCT ) );

	QItem->uiSpecialEventFlag		= uiSpecialEventFlag;
	QItem->uiSpecialEventData		= uiSpecialEventData1;
	QItem->uiSpecialEventData2	= uiSpecialEventData2;
	QItem->uiSpecialEventData3	= uiSpecialEventData3;
	QItem->iFaceIndex			= iFaceIndex;
	QItem->bUIHandlerID		= bUIHandlerID;
	QItem->iTimeStamp			= GetJA2Clock( );

	// if paused state not already locked
	if( gfLockPauseState == FALSE )
	{
		QItem->fPauseTime     = fPausedTimeDuringQuote;
	}

	fPausedTimeDuringQuote = FALSE;

	// Add to queue
	ghDialogueQ = AddtoQueue( ghDialogueQ, &QItem );


	return( TRUE );
}

BOOLEAN SpecialCharacterDialogueEventWithExtraParam( UINT32 uiSpecialEventFlag, UINT32 uiSpecialEventData1, UINT32 uiSpecialEventData2, UINT32 uiSpecialEventData3, UINT32 uiSpecialEventData4, INT32 iFaceIndex, UINT8 bUIHandlerID )
{
	DIALOGUE_Q_STRUCT				*QItem;

	// Allocate new item
	QItem = MemAlloc( sizeof( DIALOGUE_Q_STRUCT ) );
	memset( QItem, 0, sizeof( DIALOGUE_Q_STRUCT ) );

	QItem->uiSpecialEventFlag		= uiSpecialEventFlag;
	QItem->uiSpecialEventData		= uiSpecialEventData1;
	QItem->uiSpecialEventData2	= uiSpecialEventData2;
	QItem->uiSpecialEventData3	= uiSpecialEventData3;
	QItem->uiSpecialEventData4	= uiSpecialEventData4;
	QItem->iFaceIndex			= iFaceIndex;
	QItem->bUIHandlerID		= bUIHandlerID;
	QItem->iTimeStamp			= GetJA2Clock( );

	// if paused state not already locked
	if( gfLockPauseState == FALSE )
	{
		QItem->fPauseTime     = fPausedTimeDuringQuote;
	}

	fPausedTimeDuringQuote = FALSE;

	// Add to queue
	ghDialogueQ = AddtoQueue( ghDialogueQ, &QItem );


	return( TRUE );
}


static BOOLEAN GetDialogue(UINT8 ubCharacterNum, UINT16 usQuoteNum, UINT32 iDataSize, wchar_t* zDialogueText, size_t Length, CHAR8* zSoundString);


// execute specific character dialogue
static BOOLEAN ExecuteCharacterDialogue(UINT8 ubCharacterNum, UINT16 usQuoteNum, INT32 iFaceIndex, UINT8 bUIHandlerID, BOOLEAN fFromSoldier)
{
	CHAR8		zSoundString[ 164 ];
	SOLDIERTYPE *pSoldier;

	// Check if we are dead now or not....( if from a soldier... )

	// Try to find soldier...
	pSoldier = FindSoldierByProfileID( ubCharacterNum, TRUE );

	if ( pSoldier != NULL )
	{
		// Check vital stats
		if (pSoldier->bLife < CONSCIOUSNESS )
		{
			return( FALSE );
		}

		if ( pSoldier->uiStatusFlags & SOLDIER_GASSED )
			return( FALSE );

		if ( (AM_A_ROBOT( pSoldier )) )
		{
			return( FALSE );
		}

		if (pSoldier->bLife < OKLIFE && usQuoteNum != QUOTE_SERIOUSLY_WOUNDED )
		{
			return( FALSE );
		}

		if( pSoldier->bAssignment == ASSIGNMENT_POW )
		{
			return( FALSE );
		}

		// sleeping guys don't talk.. go to standby to talk
		if( pSoldier->fMercAsleep == TRUE )
		{
			// check if the soldier was compaining about lack of sleep and was alseep, if so, leave them alone
			if( ( usQuoteNum == QUOTE_NEED_SLEEP ) || ( usQuoteNum == QUOTE_OUT_OF_BREATH ) )
			{
				// leave them alone
				return ( TRUE );
			}

			// may want to wake up any character that has VERY important dialogue to say
			// MC to flesh out

		}

		// now being used in a different way...
		/*
		if ( ( (usQuoteNum == QUOTE_PERSONALITY_TRAIT &&
					(gMercProfiles[ubCharacterNum].bPersonalityTrait == FORGETFUL ||
					 gMercProfiles[ubCharacterNum].bPersonalityTrait == CLAUSTROPHOBIC ||
					 gMercProfiles[ubCharacterNum].bPersonalityTrait == NERVOUS ||
					 gMercProfiles[ubCharacterNum].bPersonalityTrait == NONSWIMMER ||
					 gMercProfiles[ubCharacterNum].bPersonalityTrait == FEAR_OF_INSECTS))
					//usQuoteNum == QUOTE_STARTING_TO_WHINE ||
#ifdef JA2BETAVERSION
					|| usQuoteNum == QUOTE_WHINE_EQUIPMENT) && (guiCurrentScreen != QUEST_DEBUG_SCREEN) )
#else
          ) )
#endif

		{
			// This quote might spawn another quote from someone
			iLoop = 0;
			for ( pTeamSoldier = MercPtrs[ iLoop ]; iLoop <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; iLoop++,pTeamSoldier++ )
			{
				if ( (pTeamSoldier->ubProfile != ubCharacterNum) && (OK_INSECTOR_MERC( pTeamSoldier )) && (SpacesAway( pSoldier->sGridNo, pTeamSoldier->sGridNo ) < 5) )
				{
					// if this merc disliked the whining character sufficiently and hasn't already retorted
					if ( gMercProfiles[ pTeamSoldier->ubProfile ].bMercOpinion[ ubCharacterNum ] < -2 && !( pTeamSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_ANNOYING_MERC ) )
					{
						// make a comment!
						TacticalCharacterDialogue( pTeamSoldier, QUOTE_ANNOYING_PC );
						pTeamSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_ANNOYING_MERC;
						break;
					}
				}
			}
		}
		*/
	}
	else
	{
		// If from a soldier, and he does not exist anymore, donot play!
		if ( fFromSoldier )
		{
			return( FALSE );
		}
	}

	// Check face index
	CHECKF( iFaceIndex != -1 );

  if (!GetDialogue(ubCharacterNum, usQuoteNum, DIALOGUESIZE, gzQuoteStr, lengthof(gzQuoteStr), zSoundString))
  {
    return( FALSE );
  }

	if( bUIHandlerID == DIALOGUE_EXTERNAL_NPC_UI )
	{
		// external NPC
		SetFaceTalking(iFaceIndex, zSoundString, gzQuoteStr);
	}
	else
	{
		// start "talking" system (portrait animation and start wav sample)
		SetFaceTalking(iFaceIndex, zSoundString, gzQuoteStr);
	}
	// pSoldier can be null here... ( if NOT from an alive soldier )
	CreateTalkingUI( bUIHandlerID, iFaceIndex, ubCharacterNum, pSoldier, gzQuoteStr, lengthof(gzQuoteStr));

	// Set global handleer ID value, used when face desides it's done...
	gbUIHandlerID = bUIHandlerID;

	guiScreenIDUsedWhenUICreated = guiCurrentScreen;

	return( TRUE );
}


static void DisplayTextForExternalNPC(UINT8 ubCharacterNum, STR16 zQuoteStr);
static void HandleExternNPCSpeechFace(INT32 iIndex);
static void HandleTacticalNPCTextUI(UINT8 ubCharacterNum, wchar_t* zQuoteStr);
static void HandleTacticalTextUI(INT32 iFaceIndex, SOLDIERTYPE* pSoldier, wchar_t* zQuoteStr);


static void CreateTalkingUI(INT8 bUIHandlerID, INT32 iFaceIndex, UINT8 ubCharacterNum, SOLDIERTYPE* pSoldier, wchar_t* zQuoteStr, size_t Length)
{

	// Show text, if on
  if ( gGameSettings.fOptions[ TOPTION_SUBTITLES ] || !gFacesData[ iFaceIndex ].fValidSpeech )
	{
		switch( bUIHandlerID )
		{
			case DIALOGUE_TACTICAL_UI:

				HandleTacticalTextUI( iFaceIndex, pSoldier, zQuoteStr );
				break;

			case DIALOGUE_NPC_UI:

				HandleTacticalNPCTextUI( ubCharacterNum, zQuoteStr );
				break;

			case DIALOGUE_CONTACTPAGE_UI:
				DisplayTextForMercFaceVideoPopUp( zQuoteStr, Length);
				break;

			case DIALOGUE_SPECK_CONTACT_PAGE_UI:
				DisplayTextForSpeckVideoPopUp( zQuoteStr );
				break;
			case DIALOGUE_EXTERNAL_NPC_UI:

				DisplayTextForExternalNPC( ubCharacterNum, zQuoteStr );
				break;

			case DIALOGUE_SHOPKEEPER_UI:
				InitShopKeeperSubTitledText( zQuoteStr );
				break;
		}
	}

	if ( gGameSettings.fOptions[ TOPTION_SPEECH ] )
	{
		switch( bUIHandlerID )
		{
			case DIALOGUE_TACTICAL_UI:

				HandleTacticalSpeechUI( ubCharacterNum, iFaceIndex );
				break;

			case DIALOGUE_CONTACTPAGE_UI:
				break;

			case DIALOGUE_SPECK_CONTACT_PAGE_UI:
				break;
			case DIALOGUE_EXTERNAL_NPC_UI:
				HandleExternNPCSpeechFace( iFaceIndex );
				break;
		}
	}
}


static INT8* GetDialogueDataFilename(UINT8 ubCharacterNum, UINT16 usQuoteNum, BOOLEAN fWavFile)
{
	static UINT8 zFileName[164];
	UINT8		ubFileNumID;

	// Are we an NPC OR an RPC that has not been recruited?
	// ATE: Did the || clause here to allow ANY RPC that talks while the talking menu is up to use an npc quote file
	if ( gfUseAlternateDialogueFile )
	{
		if ( fWavFile )
		{
			// build name of wav file (characternum + quotenum)
			#ifdef RUSSIAN
				sprintf( zFileName,"NPC_SPEECH/g_%03d_%03d.wav",ubCharacterNum,usQuoteNum );
			#else
				sprintf( zFileName,"NPC_SPEECH/d_%03d_%03d.wav",ubCharacterNum,usQuoteNum );
			#endif
		}
		else
		{
			// assume EDT files are in EDT directory on HARD DRIVE
			sprintf( zFileName,"NPCDATA/d_%03d.EDT", ubCharacterNum );
		}
	}
	else if ( ubCharacterNum >= FIRST_RPC &&
			( !( gMercProfiles[ ubCharacterNum ].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED )
			|| ProfileCurrentlyTalkingInDialoguePanel( ubCharacterNum )
			|| (gMercProfiles[ ubCharacterNum ].ubMiscFlags & PROFILE_MISC_FLAG_FORCENPCQUOTE) )
			)
	{
		ubFileNumID = ubCharacterNum;

		// ATE: If we are merc profile ID #151-154, all use 151's data....
		if ( ubCharacterNum >= 151 && ubCharacterNum <= 154 )
		{
			ubFileNumID = 151;
		}

		// If we are character #155, check fact!
		if ( ubCharacterNum == 155 && !gubFact[ 220 ] )
		{
			ubFileNumID = 155;
		}


		if ( fWavFile )
		{
			sprintf( zFileName,"NPC_SPEECH/%03d_%03d.wav",ubFileNumID,usQuoteNum );
		}
		else
		{
		// assume EDT files are in EDT directory on HARD DRIVE
			sprintf( zFileName,"NPCDATA/%03d.EDT", ubFileNumID );
		}
	}
	else
	{
		if ( fWavFile )
		{
			#ifdef RUSSIAN
				if( ubCharacterNum >= FIRST_RPC && gMercProfiles[ ubCharacterNum ].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED )
				{
					sprintf( zFileName,"SPEECH/r_%03d_%03d.wav",ubCharacterNum,usQuoteNum );
				}
				else
			#endif
			{	// build name of wav file (characternum + quotenum)
				sprintf( zFileName,"SPEECH/%03d_%03d.wav",ubCharacterNum,usQuoteNum );
			}
		}
		else
		{
			// assume EDT files are in EDT directory on HARD DRIVE
			sprintf( zFileName,"MERCEDT/%03d.EDT", ubCharacterNum );
		}
	}

	return( zFileName );
}

// Used to see if the dialog text file exists
BOOLEAN DialogueDataFileExistsForProfile( UINT8 ubCharacterNum, UINT16 usQuoteNum, BOOLEAN fWavFile, UINT8 **ppStr )
{
  UINT8 *pFilename;

	pFilename = GetDialogueDataFilename( ubCharacterNum, usQuoteNum, fWavFile );

	if ( ppStr )
	{
		(*ppStr ) = pFilename;
	}

	return( FileExists( pFilename ) );
}


static BOOLEAN GetDialogue(UINT8 ubCharacterNum, UINT16 usQuoteNum, UINT32 iDataSize, wchar_t* zDialogueText, size_t Length, CHAR8* zSoundString)
{
  UINT8 *pFilename;

   // first things first  - grab the text (if player has SUBTITLE PREFERENCE ON)
   //if ( gGameSettings.fOptions[ TOPTION_SUBTITLES ] )
   {
			if ( DialogueDataFileExistsForProfile( ubCharacterNum, 0, FALSE, &pFilename ) )
			{
				LoadEncryptedDataFromFile( pFilename, zDialogueText, usQuoteNum * iDataSize, iDataSize );
				if(zDialogueText[0] == 0)
        {
					swprintf( zDialogueText, Length, L"I have no text in the EDT file ( %d ) %S", usQuoteNum, pFilename );

#ifndef JA2BETAVERSION
          return( FALSE );
#endif
        }
			}
			else
			{
				swprintf( zDialogueText, Length, L"I have no text in the file ( %d ) %S", usQuoteNum , pFilename );

#ifndef JA2BETAVERSION
          return( FALSE );
#endif
			}
   }


	// CHECK IF THE FILE EXISTS, IF NOT, USE DEFAULT!
	pFilename = GetDialogueDataFilename( ubCharacterNum, usQuoteNum, TRUE );

	// Copy
	strcpy( zSoundString, pFilename );

  // Double check it exists....

//#ifndef JA2TESTVERSION

/*
  if ( !FileExists( pFilename ) )
  {
		CHAR8 sString[512];

		sprintf( sString, "ERROR: Missing file for character # %d, quote # %d", ubCharacterNum, usQuoteNum );
    ShowCursor(TRUE);
    ShowCursor(TRUE);
    ShutdownWithErrorBox( sString );
  }
*/

//#endif

 return(TRUE);
}


// Handlers for tactical UI stuff
static void HandleTacticalNPCTextUI(UINT8 ubCharacterNum, wchar_t* zQuoteStr)
{
	wchar_t zText[ QUOTE_MESSAGE_SIZE ];

	// Setup dialogue text box
	if ( guiCurrentScreen != MAP_SCREEN )
	{
		gTalkPanel.fRenderSubTitlesNow = TRUE;
		gTalkPanel.fSetupSubTitles = TRUE;
	}

	// post message to mapscreen message system
	swprintf( gTalkPanel.zQuoteStr, lengthof(gTalkPanel.zQuoteStr), L"\"%S\"", zQuoteStr );
	swprintf( zText, lengthof(zText), L"%S: \"%S\"", gMercProfiles[ ubCharacterNum ].zNickname, zQuoteStr );
	MapScreenMessage( FONT_MCOLOR_WHITE, MSG_DIALOG, L"%S",  zText );
}


static void ExecuteTacticalTextBox(INT16 sLeftPosition, STR16 pString);


// Handlers for tactical UI stuff
static void DisplayTextForExternalNPC(UINT8 ubCharacterNum, STR16 zQuoteStr)
{
	wchar_t								zText[ QUOTE_MESSAGE_SIZE ];
	INT16									sLeft;


	// Setup dialogue text box
	if ( guiCurrentScreen != MAP_SCREEN )
	{
		gTalkPanel.fRenderSubTitlesNow = TRUE;
		gTalkPanel.fSetupSubTitles = TRUE;
	}

	// post message to mapscreen message system
	swprintf( gTalkPanel.zQuoteStr, lengthof(gTalkPanel.zQuoteStr), L"\"%S\"", zQuoteStr );
	swprintf( zText, lengthof(zText), L"%S: \"%S\"", gMercProfiles[ ubCharacterNum ].zNickname, zQuoteStr );
	MapScreenMessage( FONT_MCOLOR_WHITE, MSG_DIALOG, L"%S",  zText );

	if ( guiCurrentScreen == MAP_SCREEN )
	{
  	sLeft			 = ( gsExternPanelXPosition + 97 );
		gsTopPosition = gsExternPanelYPosition;
	}
  else
  {
	  sLeft			 = ( 110 );
  }

	ExecuteTacticalTextBox( sLeft, gTalkPanel.zQuoteStr );
}


static void HandleTacticalTextUI(INT32 iFaceIndex, SOLDIERTYPE* pSoldier, wchar_t* zQuoteStr)
{
	wchar_t								zText[ QUOTE_MESSAGE_SIZE ];
	INT16									sLeft = 0;

	//BUild text
	// How do we do this with defines?
	//swprintf(zText, L"\xb4\xa2 %S: \xb5 \"%S\"", gMercProfiles[ubCharacterNum].zNickname, zQuoteStr);
	swprintf( zText, lengthof(zText), L"\"%S\"", zQuoteStr );
	sLeft	= 110;


	//previous version
	//sLeft = 110;

	ExecuteTacticalTextBox( sLeft, zText );

	swprintf( zText, lengthof(zText), L"%S: \"%S\"", gMercProfiles[ pSoldier->ubProfile ].zNickname, zQuoteStr );
	MapScreenMessage( FONT_MCOLOR_WHITE, MSG_DIALOG, L"%S",  zText );
}


static void ExecuteTacticalTextBoxForLastQuote(INT16 sLeftPosition, STR16 pString)
{
	UINT32 uiDelay = FindDelayForString( pString );

	fDialogueBoxDueToLastMessage = TRUE;

	guiDialogueLastQuoteTime = GetJA2Clock();

	guiDialogueLastQuoteDelay = ( ( uiDelay < FINAL_TALKING_DURATION ) ?  FINAL_TALKING_DURATION : uiDelay );

	// now execute box
	ExecuteTacticalTextBox(sLeftPosition, pString );
}


static void RenderSubtitleBoxOverlay(VIDEO_OVERLAY* pBlitter);
static void TextOverlayClickCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void ExecuteTacticalTextBox(INT16 sLeftPosition, STR16 pString)
{
	VIDEO_OVERLAY_DESC		VideoOverlayDesc;

	// check if mouse region created, if so, do not recreate
	if( fTextBoxMouseRegionCreated == TRUE )
	{
		return;
	}

	memset( &VideoOverlayDesc, 0, sizeof( VIDEO_OVERLAY_DESC ) );

	// Prepare text box
	iDialogueBox = PrepareMercPopupBox( iDialogueBox , BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, pString, DIALOGUE_DEFAULT_SUBTITLE_WIDTH, 0, 0, 0, &gusSubtitleBoxWidth, &gusSubtitleBoxHeight );

	VideoOverlayDesc.sLeft			 = sLeftPosition;
	VideoOverlayDesc.sTop				 = gsTopPosition;
	VideoOverlayDesc.sRight			 = VideoOverlayDesc.sLeft + gusSubtitleBoxWidth;
	VideoOverlayDesc.sBottom		 = VideoOverlayDesc.sTop + gusSubtitleBoxHeight;
	VideoOverlayDesc.sX					 = VideoOverlayDesc.sLeft;
	VideoOverlayDesc.sY					 = VideoOverlayDesc.sTop;
	VideoOverlayDesc.BltCallback = RenderSubtitleBoxOverlay;

	giTextBoxOverlay =  RegisterVideoOverlay( 0, &VideoOverlayDesc );

	gsTopPosition = 20;

	//Define main region
	MSYS_DefineRegion( &gTextBoxMouseRegion,  VideoOverlayDesc.sLeft, VideoOverlayDesc.sTop,  VideoOverlayDesc.sRight, VideoOverlayDesc.sBottom, MSYS_PRIORITY_HIGHEST,
						 CURSOR_NORMAL, MSYS_NO_CALLBACK, TextOverlayClickCallback );
	// Add region
	MSYS_AddRegion(&(gTextBoxMouseRegion) );

	fTextBoxMouseRegionCreated = TRUE;
}


static void FaceOverlayClickCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void RenderFaceOverlay(VIDEO_OVERLAY* pBlitter);


static void HandleExternNPCSpeechFace(INT32 iIndex)
{
	INT32 iFaceIndex;
	VIDEO_OVERLAY_DESC		VideoOverlayDesc;
	INT32									iFaceOverlay;


	// grab face index
	iFaceIndex = iIndex;

	// Enable it!
	SetAutoFaceActive( FACE_AUTO_DISPLAY_BUFFER, FACE_AUTO_RESTORE_BUFFER, iFaceIndex , 0, 0 );

	// Set flag to say WE control when to set inactive!
	gFacesData[ iFaceIndex ].uiFlags |= FACE_INACTIVE_HANDLED_ELSEWHERE;

	if ( guiCurrentScreen != MAP_SCREEN )
	{
		// Setup video overlay!
		VideoOverlayDesc.sLeft			 = 10;
		VideoOverlayDesc.sTop				 = 20;
		VideoOverlayDesc.sRight			 = VideoOverlayDesc.sLeft + 99;
		VideoOverlayDesc.sBottom		 = VideoOverlayDesc.sTop + 98;
		VideoOverlayDesc.sX					 = VideoOverlayDesc.sLeft;
		VideoOverlayDesc.sY					 = VideoOverlayDesc.sTop;
		VideoOverlayDesc.BltCallback = RenderFaceOverlay;
	}
	else
	{
		// Setup video overlay!

		VideoOverlayDesc.sLeft			 = gsExternPanelXPosition;
		VideoOverlayDesc.sTop				 = gsExternPanelYPosition;

		VideoOverlayDesc.sRight			 = VideoOverlayDesc.sLeft + 99;
		VideoOverlayDesc.sBottom		 = VideoOverlayDesc.sTop + 98;
		VideoOverlayDesc.sX					 = VideoOverlayDesc.sLeft;
		VideoOverlayDesc.sY					 = VideoOverlayDesc.sTop;
		VideoOverlayDesc.BltCallback = RenderFaceOverlay;
	}

	iFaceOverlay =  RegisterVideoOverlay( 0, &VideoOverlayDesc );
	gpCurrentTalkingFace->iVideoOverlay = iFaceOverlay;

	RenderAutoFace( iFaceIndex );

	// ATE: Create mouse region.......
	if ( !fExternFaceBoxRegionCreated )
	{
		fExternFaceBoxRegionCreated = TRUE;

		//Define main region
		MSYS_DefineRegion( &gFacePopupMouseRegion,  VideoOverlayDesc.sLeft, VideoOverlayDesc.sTop,  VideoOverlayDesc.sRight, VideoOverlayDesc.sBottom, MSYS_PRIORITY_HIGHEST,
							 CURSOR_NORMAL, MSYS_NO_CALLBACK, FaceOverlayClickCallback );
		// Add region
		MSYS_AddRegion(&(gFacePopupMouseRegion) );

	}

	gfFacePanelActive = TRUE;
}


static void HandleTacticalSpeechUI(UINT8 ubCharacterNum, INT32 iFaceIndex)
{
	VIDEO_OVERLAY_DESC		VideoOverlayDesc;
	INT32									iFaceOverlay;
	SOLDIERTYPE						*pSoldier;
	BOOLEAN								fDoExternPanel = FALSE;

	memset( &VideoOverlayDesc, 0, sizeof( VIDEO_OVERLAY_DESC ) );

	// Get soldier pointer, if there is one...
	// Try to find soldier...
	pSoldier = FindSoldierByProfileID( ubCharacterNum, FALSE );

	// PLEASE NOTE:  pSoldier may legally be NULL (e.g. Skyrider) !!!

	if ( pSoldier == NULL )
	{
		fDoExternPanel = TRUE;
	}
	else
	{
		// If we are not an active face!
		if ( guiCurrentScreen != MAP_SCREEN )
		{
			fDoExternPanel = TRUE;
		}
	}

	if ( fDoExternPanel )
	{
		// Enable it!
		SetAutoFaceActive( FACE_AUTO_DISPLAY_BUFFER, FACE_AUTO_RESTORE_BUFFER, iFaceIndex , 0, 0 );

		// Set flag to say WE control when to set inactive!
		gFacesData[ iFaceIndex ].uiFlags |= ( FACE_INACTIVE_HANDLED_ELSEWHERE | FACE_MAKEACTIVE_ONCE_DONE );

		// IF we are in tactical and this soldier is on the current squad
		if ( ( guiCurrentScreen == GAME_SCREEN ) && ( pSoldier != NULL ) && ( pSoldier->bAssignment == iCurrentTacticalSquad ) )
		{
			// Make the interface panel dirty..
			// This will dirty the panel next frame...
			gfRerenderInterfaceFromHelpText = TRUE;
		}

		// Setup video overlay!
		VideoOverlayDesc.sLeft			 = 10;
		VideoOverlayDesc.sTop				 = 20;
		VideoOverlayDesc.sRight			 = VideoOverlayDesc.sLeft + 99;
		VideoOverlayDesc.sBottom		 = VideoOverlayDesc.sTop + 98;
		VideoOverlayDesc.sX					 = VideoOverlayDesc.sLeft;
		VideoOverlayDesc.sY					 = VideoOverlayDesc.sTop;
		VideoOverlayDesc.BltCallback = RenderFaceOverlay;

		iFaceOverlay =  RegisterVideoOverlay( 0, &VideoOverlayDesc );
		gpCurrentTalkingFace->iVideoOverlay = iFaceOverlay;

		RenderAutoFace( iFaceIndex );

		// ATE: Create mouse region.......
		if ( !fExternFaceBoxRegionCreated )
		{
			fExternFaceBoxRegionCreated = TRUE;

			//Define main region
			MSYS_DefineRegion( &gFacePopupMouseRegion,  VideoOverlayDesc.sLeft, VideoOverlayDesc.sTop,  VideoOverlayDesc.sRight, VideoOverlayDesc.sBottom, MSYS_PRIORITY_HIGHEST,
								 CURSOR_NORMAL, MSYS_NO_CALLBACK, FaceOverlayClickCallback );
			// Add region
			MSYS_AddRegion(&(gFacePopupMouseRegion) );
		}

		gfFacePanelActive = TRUE;

	}
	else if ( guiCurrentScreen == MAP_SCREEN )
	{
		// Are we in mapscreen?
		// If so, set current guy active to talk.....
		if ( pSoldier != NULL )
		{
			ContinueDialogue( pSoldier, FALSE );
		}
	}

}


void HandleDialogueEnd( FACETYPE *pFace )
{
	if ( gGameSettings.fOptions[ TOPTION_SPEECH ] )
	{

		if ( pFace != gpCurrentTalkingFace )
		{
			//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"HandleDialogueEnd() face mismatch." );
			return;
		}

		if ( pFace->fTalking )
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"HandleDialogueEnd() face still talking." );
			return;
		}

		switch( gbUIHandlerID )
		{
			case DIALOGUE_TACTICAL_UI:

			if ( gfFacePanelActive )
			{
				// Set face inactive!
				pFace->fCanHandleInactiveNow = TRUE;
				SetAutoFaceInActive( pFace->iID );
				gfFacePanelActive = FALSE;

				if ( fExternFaceBoxRegionCreated )
				{
					fExternFaceBoxRegionCreated = FALSE;
					MSYS_RemoveRegion(&(gFacePopupMouseRegion) );
				}
			}
			break;
			case DIALOGUE_NPC_UI:
			break;
			case DIALOGUE_EXTERNAL_NPC_UI:
				pFace->fCanHandleInactiveNow = TRUE;
				SetAutoFaceInActive( pFace->iID );
				gfFacePanelActive = FALSE;

				if ( fExternFaceBoxRegionCreated )
				{
					fExternFaceBoxRegionCreated = FALSE;
					MSYS_RemoveRegion(&(gFacePopupMouseRegion) );
				}

			break;
		}
	}


  if ( gGameSettings.fOptions[ TOPTION_SUBTITLES ] || !pFace->fValidSpeech )
	{
		switch( gbUIHandlerID )
		{
			case DIALOGUE_TACTICAL_UI:
			case DIALOGUE_EXTERNAL_NPC_UI:
				// Remove if created
				if ( giTextBoxOverlay != -1 )
				{
					RemoveVideoOverlay( giTextBoxOverlay );
					giTextBoxOverlay = -1;

					if ( fTextBoxMouseRegionCreated )
					{
						RemoveMercPopupBoxFromIndex( iDialogueBox );

						// reset box id
						iDialogueBox = -1;
						MSYS_RemoveRegion( &gTextBoxMouseRegion );
						fTextBoxMouseRegionCreated = FALSE;
					}

				}

				break;

			case DIALOGUE_NPC_UI:


				// Remove region
				if ( gTalkPanel.fTextRegionOn )
				{
					MSYS_RemoveRegion(&(gTalkPanel.TextRegion) );
					gTalkPanel.fTextRegionOn = FALSE;

				}

				SetRenderFlags( RENDER_FLAG_FULL );
				gTalkPanel.fRenderSubTitlesNow = FALSE;

				// Delete subtitle box
				gTalkPanel.fDirtyLevel = DIRTYLEVEL2;
				RemoveMercPopupBoxFromIndex( iInterfaceDialogueBox );
				iInterfaceDialogueBox = -1;
				break;

			case DIALOGUE_CONTACTPAGE_UI:
				break;

			case DIALOGUE_SPECK_CONTACT_PAGE_UI:
				break;



		}
	}

  TurnOffSectorLocator();

  gsExternPanelXPosition     = DEFAULT_EXTERN_PANEL_X_POS;
  gsExternPanelYPosition     = DEFAULT_EXTERN_PANEL_Y_POS;

}


static void RenderFaceOverlay(VIDEO_OVERLAY* pBlitter)
{
	UINT32 uiDestPitchBYTES, uiSrcPitchBYTES;
	UINT8	 *pDestBuf, *pSrcBuf;
	INT16 sFontX, sFontY;
	SOLDIERTYPE *pSoldier;
	wchar_t					zTownIDString[50];


	if ( gpCurrentTalkingFace == NULL )
	{
		return;
	}

	if ( gfFacePanelActive )
	{
		pSoldier = FindSoldierByProfileID( gpCurrentTalkingFace->ubCharacterNum, FALSE );


		// a living soldier?..or external NPC?..choose panel based on this
		if( pSoldier )
		{
			BltVideoObjectFromIndex( pBlitter->uiDestBuff, guiCOMPANEL, 0, pBlitter->sX, pBlitter->sY);
		}
		else
		{
			BltVideoObjectFromIndex( pBlitter->uiDestBuff, guiCOMPANELB, 0, pBlitter->sX, pBlitter->sY);
		}

		// Display name, location ( if not current )
		SetFont( BLOCKFONT2 );
		SetFontBackground( FONT_MCOLOR_BLACK );
		SetFontForeground( FONT_MCOLOR_LTGRAY );

		if ( pSoldier )
		{
		  //reset the font dest buffer
		  SetFontDestBuffer( pBlitter->uiDestBuff, 0,0,640,480, FALSE);

			VarFindFontCenterCoordinates( (INT16)( pBlitter->sX + 12 ), (INT16)( pBlitter->sY + 55 ), 73, 9, BLOCKFONT2, &sFontX, &sFontY, L"%S", pSoldier->name );
			mprintf( sFontX, sFontY, L"%S", pSoldier->name );

			// What sector are we in, ( and is it the same as ours? )
			if ( pSoldier->sSectorX != gWorldSectorX || pSoldier->sSectorY != gWorldSectorY || pSoldier->bSectorZ != gbWorldSectorZ || pSoldier->fBetweenSectors )
			{
				GetSectorIDString( pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ, zTownIDString, lengthof(zTownIDString), FALSE );

        ReduceStringLength( zTownIDString, lengthof(zTownIDString), 64 , BLOCKFONT2 );

				VarFindFontCenterCoordinates( (INT16)( pBlitter->sX + 12 ), (INT16)( pBlitter->sY + 68 ), 73, 9, BLOCKFONT2, &sFontX, &sFontY, L"%S", zTownIDString );
				mprintf( sFontX, sFontY, L"%S", zTownIDString );
			}

		  //reset the font dest buffer
		  SetFontDestBuffer(FRAME_BUFFER, 0,0,640,480, FALSE);

		  // Display bars
		  DrawLifeUIBarEx( pSoldier, (INT16)( pBlitter->sX + 69 ), (INT16)( pBlitter->sY + 47 ), 3, 42, FALSE, pBlitter->uiDestBuff );
		  DrawBreathUIBarEx( pSoldier, (INT16)( pBlitter->sX + 75 ), (INT16)( pBlitter->sY + 47 ), 3, 42, FALSE, pBlitter->uiDestBuff );
		  DrawMoraleUIBarEx( pSoldier, (INT16)( pBlitter->sX + 81 ), (INT16)( pBlitter->sY + 47 ), 3, 42, FALSE, pBlitter->uiDestBuff );

		}
		else
		{
			VarFindFontCenterCoordinates( (INT16)( pBlitter->sX + 9 ), (INT16)( pBlitter->sY + 55 ), 73, 9, BLOCKFONT2, &sFontX, &sFontY, L"%S", gMercProfiles[ gpCurrentTalkingFace->ubCharacterNum ].zNickname );
			mprintf( sFontX, sFontY, L"%S", gMercProfiles[ gpCurrentTalkingFace->ubCharacterNum ].zNickname );
		}

		//RenderAutoFace( gpCurrentTalkingFace->iID );


		pDestBuf = LockVideoSurface( pBlitter->uiDestBuff, &uiDestPitchBYTES);
		pSrcBuf = LockVideoSurface( gpCurrentTalkingFace->uiAutoDisplayBuffer, &uiSrcPitchBYTES);

		Blt16BPPTo16BPP((UINT16 *)pDestBuf, uiDestPitchBYTES,
					(UINT16 *)pSrcBuf, uiSrcPitchBYTES,
					(INT16)( pBlitter->sX + 14 ), (INT16)( pBlitter->sY + 6 ),
					0 , 0,
					gpCurrentTalkingFace->usFaceWidth, gpCurrentTalkingFace->usFaceHeight );

		UnLockVideoSurface( pBlitter->uiDestBuff );
		UnLockVideoSurface( gpCurrentTalkingFace->uiAutoDisplayBuffer );

		InvalidateRegion( pBlitter->sX, pBlitter->sY, pBlitter->sX + 99, pBlitter->sY + 98 );
	}
}


static void RenderSubtitleBoxOverlay(VIDEO_OVERLAY* pBlitter)
{
	if ( giTextBoxOverlay != -1 )
	{
		RenderMercPopUpBoxFromIndex( iDialogueBox, pBlitter->sX, pBlitter->sY,  pBlitter->uiDestBuff );

		InvalidateRegion( pBlitter->sX, pBlitter->sY, pBlitter->sX + gusSubtitleBoxWidth, pBlitter->sY + gusSubtitleBoxHeight );
	}
}


void SayQuoteFromAnyBodyInSector( UINT16 usQuoteNum )
{
	UINT8	ubMercsInSector[ 20 ] = { 0 };
	UINT8	ubNumMercs = 0;
	UINT8	ubChosenMerc;
	SOLDIERTYPE *pTeamSoldier;
	INT32 cnt;

	// Loop through all our guys and randomly say one from someone in our sector

	// set up soldier ptr as first element in mercptrs list
	cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

	// run through list
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pTeamSoldier++ )
	{
		// Add guy if he's a candidate...
		if ( OK_INSECTOR_MERC( pTeamSoldier ) && !AM_AN_EPC( pTeamSoldier ) && !( pTeamSoldier->uiStatusFlags & SOLDIER_GASSED ) && !(AM_A_ROBOT( pTeamSoldier )) && !pTeamSoldier->fMercAsleep )
		{
			if ( gTacticalStatus.bNumFoughtInBattle[ ENEMY_TEAM ] == 0 )
			{
				// quotes referring to Deidranna's men so we skip quote if there were no army guys fought
				if ( (usQuoteNum == QUOTE_SECTOR_SAFE) && (pTeamSoldier->ubProfile == IRA || pTeamSoldier->ubProfile == MIGUEL || pTeamSoldier->ubProfile == SHANK ) )
				{

					continue;
				}
				if ( (usQuoteNum == QUOTE_ENEMY_PRESENCE ) && (pTeamSoldier->ubProfile == IRA || pTeamSoldier->ubProfile == DIMITRI || pTeamSoldier->ubProfile == DYNAMO || pTeamSoldier->ubProfile == SHANK ) )
				{
					continue;
				}
			}

			ubMercsInSector[ ubNumMercs ] = (UINT8)cnt;
			ubNumMercs++;
		}
	}

	// If we are > 0
	if ( ubNumMercs > 0 )
	{
		ubChosenMerc = (UINT8)Random( ubNumMercs );

		// If we are air raid, AND red exists somewhere...
		if ( usQuoteNum == QUOTE_AIR_RAID )
		{
			for ( cnt = 0; cnt < ubNumMercs; cnt++ )
			{
				if ( ubMercsInSector[ cnt ] == 11 )
				{
					ubChosenMerc = (UINT8)cnt;
					break;
				}
			}
		}


		TacticalCharacterDialogue( MercPtrs[ ubMercsInSector[ ubChosenMerc ] ], usQuoteNum );
	}

}


void SayQuoteFromAnyBodyInThisSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ, UINT16 usQuoteNum )
{
	UINT8	ubMercsInSector[ 20 ] = { 0 };
	UINT8	ubNumMercs = 0;
	UINT8	ubChosenMerc;
	SOLDIERTYPE *pTeamSoldier;
	INT32 cnt;

	// Loop through all our guys and randomly say one from someone in our sector

	// set up soldier ptr as first element in mercptrs list
	cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

	// run through list
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pTeamSoldier++ )
	{
		if ( pTeamSoldier->bActive )
		{
			// Add guy if he's a candidate...
			if( pTeamSoldier->sSectorX == sSectorX && pTeamSoldier->sSectorY == sSectorY && pTeamSoldier -> bSectorZ == bSectorZ  && !AM_AN_EPC( pTeamSoldier ) && !( pTeamSoldier->uiStatusFlags & SOLDIER_GASSED ) && !(AM_A_ROBOT( pTeamSoldier )) && !pTeamSoldier->fMercAsleep )
			{
				ubMercsInSector[ ubNumMercs ] = (UINT8)cnt;
				ubNumMercs++;
			}
		}
	}

	// If we are > 0
	if ( ubNumMercs > 0 )
	{
		ubChosenMerc = (UINT8)Random( ubNumMercs );

		// If we are air raid, AND red exists somewhere...
		if ( usQuoteNum == QUOTE_AIR_RAID )
		{
			for ( cnt = 0; cnt < ubNumMercs; cnt++ )
			{
				if ( ubMercsInSector[ cnt ] == 11 )
				{
					ubChosenMerc = (UINT8)cnt;
					break;
				}
			}
		}


		TacticalCharacterDialogue( MercPtrs[ ubMercsInSector[ ubChosenMerc ] ], usQuoteNum );
	}
}

void SayQuoteFromNearbyMercInSector( INT16 sGridNo, INT8 bDistance, UINT16 usQuoteNum )
{
	UINT8	ubMercsInSector[ 20 ] = { 0 };
	UINT8	ubNumMercs = 0;
	UINT8	ubChosenMerc;
	SOLDIERTYPE *pTeamSoldier;
	INT32 cnt;

	// Loop through all our guys and randomly say one from someone in our sector

	// set up soldier ptr as first element in mercptrs list
	cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

	// run through list
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pTeamSoldier++ )
	{
		// Add guy if he's a candidate...
		if ( OK_INSECTOR_MERC( pTeamSoldier ) && PythSpacesAway( sGridNo, pTeamSoldier->sGridNo ) < bDistance && !AM_AN_EPC( pTeamSoldier ) && !( pTeamSoldier->uiStatusFlags & SOLDIER_GASSED ) && !(AM_A_ROBOT( pTeamSoldier )) && !pTeamSoldier->fMercAsleep &&
			SoldierTo3DLocationLineOfSightTest( pTeamSoldier, sGridNo, 0, 0, (UINT8)MaxDistanceVisible(), TRUE ) )
		{
			if ( usQuoteNum == 66 && (INT8) Random( 100 ) > EffectiveWisdom( pTeamSoldier ) )
			{
				continue;
			}
			ubMercsInSector[ ubNumMercs ] = (UINT8)cnt;
			ubNumMercs++;
		}
	}

	// If we are > 0
	if ( ubNumMercs > 0 )
	{
		ubChosenMerc = (UINT8)Random( ubNumMercs );

		if (usQuoteNum == 66)
		{
			SetFactTrue( FACT_PLAYER_FOUND_ITEMS_MISSING );
		}
		TacticalCharacterDialogue( MercPtrs[ ubMercsInSector[ ubChosenMerc ] ], usQuoteNum );

	}

}

void SayQuote58FromNearbyMercInSector( INT16 sGridNo, INT8 bDistance, UINT16 usQuoteNum, INT8 bSex )
{
	UINT8	ubMercsInSector[ 20 ] = { 0 };
	UINT8	ubNumMercs = 0;
	UINT8	ubChosenMerc;
	SOLDIERTYPE *pTeamSoldier;
	INT32 cnt;

	// Loop through all our guys and randomly say one from someone in our sector

	// set up soldier ptr as first element in mercptrs list
	cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

	// run through list
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pTeamSoldier++ )
	{
		// Add guy if he's a candidate...
		if ( OK_INSECTOR_MERC( pTeamSoldier ) && PythSpacesAway( sGridNo, pTeamSoldier->sGridNo ) < bDistance && !AM_AN_EPC( pTeamSoldier ) && !( pTeamSoldier->uiStatusFlags & SOLDIER_GASSED ) && !(AM_A_ROBOT( pTeamSoldier )) && !pTeamSoldier->fMercAsleep &&
			SoldierTo3DLocationLineOfSightTest( pTeamSoldier, sGridNo, 0, 0, (UINT8)MaxDistanceVisible(), TRUE ) )
		{
			// ATE: This is to check gedner for this quote...
			if ( QuoteExp_GenderCode[ pTeamSoldier->ubProfile ] == 0 && bSex == FEMALE )
			{
				continue;
			}

			if ( QuoteExp_GenderCode[ pTeamSoldier->ubProfile ] == 1 && bSex == MALE )
			{
				continue;
			}

			ubMercsInSector[ ubNumMercs ] = (UINT8)cnt;
			ubNumMercs++;
		}
	}

	// If we are > 0
	if ( ubNumMercs > 0 )
	{
		ubChosenMerc = (UINT8)Random( ubNumMercs );
		TacticalCharacterDialogue( MercPtrs[ ubMercsInSector[ ubChosenMerc ] ], usQuoteNum );
	}

}


static void TextOverlayClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	static BOOLEAN fLButtonDown = FALSE;

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		fLButtonDown = TRUE;
	}

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP && fLButtonDown )
	{
		if(  gpCurrentTalkingFace != NULL )
		{
			InternalShutupaYoFace( gpCurrentTalkingFace->iID, FALSE );

			// Did we succeed in shutting them up?
			if ( !gpCurrentTalkingFace->fTalking )
			{
				// shut down last quote box
				ShutDownLastQuoteTacticalTextBox( );
			}
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		fLButtonDown = FALSE;
	}
}


static void FaceOverlayClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	static BOOLEAN fLButtonDown = FALSE;

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		fLButtonDown = TRUE;
	}

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP && fLButtonDown )
	{
		if(  gpCurrentTalkingFace != NULL )
		{
			InternalShutupaYoFace( gpCurrentTalkingFace->iID, FALSE );
		}

	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		fLButtonDown = FALSE;
	}
}

void ShutDownLastQuoteTacticalTextBox( void )
{
	if( fDialogueBoxDueToLastMessage )
	{
		RemoveVideoOverlay( giTextBoxOverlay );
		giTextBoxOverlay = -1;

		if ( fTextBoxMouseRegionCreated )
		{
			MSYS_RemoveRegion( &gTextBoxMouseRegion );
			fTextBoxMouseRegionCreated = FALSE;
		}

		fDialogueBoxDueToLastMessage = FALSE;
	}
}

UINT32 FindDelayForString( STR16 sString )
{
	return( wcslen( sString ) * TEXT_DELAY_MODIFIER );
}

void BeginLoggingForBleedMeToos( BOOLEAN fStart )
{
	gubLogForMeTooBleeds = fStart;
}


void SetEngagedInConvFromPCAction( SOLDIERTYPE *pSoldier )
{
	// OK, If a good give, set engaged in conv...
	gTacticalStatus.uiFlags |= ENGAGED_IN_CONV;
	gTacticalStatus.ubEngagedInConvFromActionMercID = pSoldier->ubID;
}

void UnSetEngagedInConvFromPCAction( SOLDIERTYPE *pSoldier )
{
	if ( gTacticalStatus.ubEngagedInConvFromActionMercID == pSoldier->ubID )
	{
		// OK, If a good give, set engaged in conv...
		gTacticalStatus.uiFlags &= ( ~ENGAGED_IN_CONV );
	}
}


static BOOLEAN IsStopTimeQuote(UINT16 usQuoteNum)
{
	INT32 cnt;

	for ( cnt = 0; cnt < gubNumStopTimeQuotes; cnt++ )
	{
		if ( gusStopTimeQuoteList[ cnt ] == usQuoteNum )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}


static void CheckForStopTimeQuotes(UINT16 usQuoteNum)
{
	if ( IsStopTimeQuote( usQuoteNum ) )
	{
		// Stop Time, game
		EnterModalTactical( TACTICAL_MODAL_NOMOUSE );

		gpCurrentTalkingFace->uiFlags		|= FACE_MODAL;

		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Starting Modal Tactical Quote." );

	}
}

void SetStopTimeQuoteCallback( MODAL_HOOK pCallBack )
{
	gModalDoneCallback = pCallBack;
}


BOOLEAN IsMercSayingDialogue( UINT8 ubProfileID )
{
	if ( gpCurrentTalkingFace != NULL && gubCurrentTalkingID == ubProfileID )
	{
		return( TRUE );
	}
	return( FALSE );
}


static BOOLEAN IsQuoteInPrecedentArray(UINT32 uiQuoteID);


static BOOLEAN ShouldMercSayPrecedentToRepeatOneSelf(UINT8 ubMercID, UINT32 uiQuoteID)
{
	UINT8	ubQuoteBit=0;

	//If the quote is not in the array
	if( !IsQuoteInPrecedentArray( uiQuoteID ) )
	{
		return( FALSE );
	}

	ubQuoteBit = GetQuoteBitNumberFromQuoteID( uiQuoteID );
	if( ubQuoteBit == 0 )
		return( FALSE );

	if( GetMercPrecedentQuoteBitStatus( ubMercID, ubQuoteBit ) )
	{
		return( TRUE );
	}
	else
	{
		SetMercPrecedentQuoteBitStatus( ubMercID, ubQuoteBit );
	}

	return( FALSE );
}



BOOLEAN GetMercPrecedentQuoteBitStatus( UINT8 ubMercID, UINT8 ubQuoteBit )
{
	if( gMercProfiles[ ubMercID ].uiPrecedentQuoteSaid & ( 1 << ( ubQuoteBit - 1 ) ) )
		return( TRUE );
	else
		return( FALSE );
}

BOOLEAN SetMercPrecedentQuoteBitStatus( UINT8 ubMercID, UINT8 ubBitToSet )
{
	//Set the bit
	gMercProfiles[ ubMercID ].uiPrecedentQuoteSaid |= 1 << ( ubBitToSet - 1 );

	return( TRUE );
}


static BOOLEAN IsQuoteInPrecedentArray(UINT32 uiQuoteID)
{
	UINT8	ubCnt;

	//If the quote id is above or below the ones in the array
	if( uiQuoteID < gubMercValidPrecedentQuoteID[ 0 ] ||
			uiQuoteID > gubMercValidPrecedentQuoteID[ NUMBER_VALID_MERC_PRECEDENT_QUOTES-1 ] )
	{
		return( FALSE );
	}


	//loop through all the quotes
	for( ubCnt=0; ubCnt<NUMBER_VALID_MERC_PRECEDENT_QUOTES;ubCnt++)
	{
		if( gubMercValidPrecedentQuoteID[ ubCnt ] == uiQuoteID )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}



UINT8	GetQuoteBitNumberFromQuoteID( UINT32 uiQuoteID )
{
	UINT8 ubCnt;

	//loop through all the quotes
	for( ubCnt=0; ubCnt<NUMBER_VALID_MERC_PRECEDENT_QUOTES;ubCnt++)
	{
		if( gubMercValidPrecedentQuoteID[ ubCnt ] == uiQuoteID )
		{
			return( ubCnt );
		}
	}

	return( 0 );
}

void HandleShutDownOfMapScreenWhileExternfaceIsTalking( void )
{

	if ( ( fExternFaceBoxRegionCreated ) && ( gpCurrentTalkingFace) )
	{
		RemoveVideoOverlay( gpCurrentTalkingFace->iVideoOverlay );
		gpCurrentTalkingFace->iVideoOverlay = -1;
	}

}


void HandleImportantMercQuote( SOLDIERTYPE * pSoldier, UINT16 usQuoteNumber )
{
	// wake merc up for THIS quote
	if( pSoldier->fMercAsleep )
	{
		TacticalCharacterDialogueWithSpecialEvent( pSoldier, usQuoteNumber, DIALOGUE_SPECIAL_EVENT_SLEEP, 0,0 );
		TacticalCharacterDialogue( pSoldier, usQuoteNumber );
		TacticalCharacterDialogueWithSpecialEvent( pSoldier, usQuoteNumber, DIALOGUE_SPECIAL_EVENT_SLEEP, 1,0 );
	}
	else
	{
		TacticalCharacterDialogue( pSoldier, usQuoteNumber );
	}
}


// handle pausing of the dialogue queue
void PauseDialogueQueue( void )
{
	gfDialogueQueuePaused = TRUE;
}

// unpause the dialogue queue
void UnPauseDialogueQueue( void )
{
	gfDialogueQueuePaused = FALSE;
}


void SetExternMapscreenSpeechPanelXY( INT16 sXPos, INT16 sYPos )
{
  gsExternPanelXPosition     = sXPos;
  gsExternPanelYPosition     = sYPos;
}
