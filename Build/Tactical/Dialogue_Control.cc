#include "Font.h"
#include "Font_Control.h"
#include "GameLoop.h"
#include "Handle_UI.h"
#include "Interface.h"
#include "Isometric_Utils.h"
#include "MessageBoxScreen.h"
#include "Soldier_Control.h"
#include "Encrypted_File.h"
#include "Faces.h"
#include "VObject.h"
#include "VSurface.h"
#include "WCheck.h"
#include "Overhead.h"
#include "Dialogue_Control.h"
#include "Message.h"
#include "Render_Dirty.h"
#include "Soldier_Profile.h"
#include "WordWrap.h"
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
#include "GameSettings.h"
#include "ShopKeeper_Interface.h"
#include "Map_Screen_Interface.h"
#include "Meanwhile.h"
#include "SkillCheck.h"
#include "Interface_Control.h"
#include "Civ_Quotes.h"
#include "OppList.h"
#include "AI.h"
#include "WorldMan.h"
#include "LOS.h"
#include "QArray.h"
#include "JAScreens.h"
#include "Video.h"
#include "SoundMan.h"
#include "Container.h"


#define DIALOGUESIZE 240
#define   QUOTE_MESSAGE_SIZE		520

#define		DIALOGUE_DEFAULT_SUBTITLE_WIDTH		200
#define		TEXT_DELAY_MODIFIER			60


typedef SGP::Queue<DialogueEvent*> DialogueQueue;


BOOLEAN fExternFacesLoaded = FALSE;

FACETYPE* uiExternalStaticNPCFaces[NUMBER_OF_EXTERNAL_NPC_FACES];
const ProfileID g_external_face_profile_ids[] =
{
	SKYRIDER,
	FRED,
	MATT,
	OSWALD,
	CALVIN,
	CARL
};
CASSERT(lengthof(g_external_face_profile_ids) == NUMBER_OF_EXTERNAL_NPC_FACES)


static UINT8 const gubMercValidPrecedentQuoteID[] =
{
	QUOTE_REPUTATION_REFUSAL,
	QUOTE_DEATH_RATE_REFUSAL,
	QUOTE_LAME_REFUSAL,
	QUOTE_WONT_RENEW_CONTRACT_LAME_REFUSAL,
	QUOTE_HATE_MERC_1_ON_TEAM,
	QUOTE_HATE_MERC_2_ON_TEAM,
	QUOTE_LEARNED_TO_HATE_MERC_ON_TEAM,
	QUOTE_REFUSAL_RENEW_DUE_TO_MORALE,
	QUOTE_REFUSAL_TO_JOIN_LACK_OF_FUNDS,
	QUOTE_DEATH_RATE_RENEWAL,
	QUOTE_HATE_MERC_1_ON_TEAM_WONT_RENEW,
	QUOTE_HATE_MERC_2_ON_TEAM_WONT_RENEW,
	QUOTE_LEARNED_TO_HATE_MERC_1_ON_TEAM_WONT_RENEW
};


static UINT16 const gusStopTimeQuoteList[] =
{
	QUOTE_BOOBYTRAP_ITEM,
	QUOTE_SUSPICIOUS_GROUND
};


// QUEUE UP DIALOG!
#define		INITIAL_Q_SIZE				10
static DialogueQueue* ghDialogueQ;
FACETYPE	*gpCurrentTalkingFace	= NULL;
UINT8			gubCurrentTalkingID   = NO_PROFILE;
static DialogueHandler gbUIHandlerID;

INT32				giNPCReferenceCount = 0;

static INT16 gsExternPanelXPosition = DEFAULT_EXTERN_PANEL_X_POS;
static INT16 gsExternPanelYPosition = DEFAULT_EXTERN_PANEL_Y_POS;

static BOOLEAN        gfDialogueQueuePaused = FALSE;
static UINT16         gusSubtitleBoxWidth;
static UINT16         gusSubtitleBoxHeight;
static VIDEO_OVERLAY* g_text_box_overlay = NULL;
BOOLEAN			gfFacePanelActive = FALSE;
static UINT32         guiScreenIDUsedWhenUICreated;
static wchar_t        gzQuoteStr[QUOTE_MESSAGE_SIZE];
static MOUSE_REGION   gTextBoxMouseRegion;
static MOUSE_REGION   gFacePopupMouseRegion;
static BOOLEAN        gfUseAlternateDialogueFile = FALSE;

// set the top position value for merc dialogue pop up boxes
static INT16 gsTopPosition = 20;


MercPopUpBox* g_dialogue_box;


static BOOLEAN fWasPausedDuringDialogue = FALSE;

static INT8 gubLogForMeTooBleeds = FALSE;


// has the text region been created?
static BOOLEAN fTextBoxMouseRegionCreated  = FALSE;
static BOOLEAN fExternFaceBoxRegionCreated = FALSE;


BOOLEAN DialogueActive( )
{
	if ( gpCurrentTalkingFace != NULL )
	{
		return( TRUE );
	}

	return( FALSE );
}


void InitalizeDialogueControl()
{
	ghDialogueQ         = new DialogueQueue(INITIAL_Q_SIZE);
	giNPCReferenceCount = 0;
}

void ShutdownDialogueControl()
{
	if (ghDialogueQ != NULL)
	{
		delete ghDialogueQ;
		ghDialogueQ = NULL;
		gfWaitingForTriggerTimer = FALSE;
	}

	// shutdown external static NPC faces
	ShutdownStaticExternalNPCFaces();

	// gte rid of portraits for cars
	UnLoadCarPortraits();
}


void InitalizeStaticExternalNPCFaces( void )
{
#ifndef JA2DEMO
	INT32 iCounter = 0;
	// go and grab all external NPC faces that are needed for the game who won't exist as soldiertypes

	if (fExternFacesLoaded) return;

	fExternFacesLoaded = TRUE;

	for( iCounter = 0; iCounter < NUMBER_OF_EXTERNAL_NPC_FACES; iCounter++ )
	{
		uiExternalStaticNPCFaces[iCounter] = InitFace(g_external_face_profile_ids[iCounter], NULL, FACE_FORCE_SMALL);
	}
#endif
}


void ShutdownStaticExternalNPCFaces( void )
{
	INT32 iCounter = 0;

	if (!fExternFacesLoaded) return;

	fExternFacesLoaded = FALSE;

	// remove all external npc faces
	for( iCounter = 0; iCounter < NUMBER_OF_EXTERNAL_NPC_FACES; iCounter++ )
	{
		DeleteFace( uiExternalStaticNPCFaces[ iCounter ] );
	}
}


void EmptyDialogueQueue()
{
	// If we have anything left in the queue, remove!
	if (ghDialogueQ != NULL)
	{
		delete ghDialogueQ;
		ghDialogueQ = new DialogueQueue(INITIAL_Q_SIZE);
	}

	gfWaitingForTriggerTimer = FALSE;
}


BOOLEAN DialogueQueueIsEmpty( )
{
	return ghDialogueQ && ghDialogueQ->IsEmpty();
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
	InternalShutupaYoFace(gpCurrentTalkingFace, FALSE);
}


void StopAnyCurrentlyTalkingSpeech( )
{
	// ATE; Make sure guys stop talking....
	if ( gpCurrentTalkingFace != NULL )
	{
		InternalShutupaYoFace(gpCurrentTalkingFace, TRUE);
	}
}


static void CreateTalkingUI(DialogueHandler, FACETYPE* face, UINT8 ubCharacterNum, const wchar_t* zQuoteStr);


// ATE: Handle changes like when face goes from
// 'external' to on the team panel...
void HandleDialogueUIAdjustments( )
{
	// OK, check if we are still taking
	if ( gpCurrentTalkingFace != NULL )
	{
		if ( gpCurrentTalkingFace->fTalking )
		{
			// ATE: Check for change in state for the guy currently talking on 'external' panel....
			if ( gfFacePanelActive )
			{
				const SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(gubCurrentTalkingID);
				if ( pSoldier )
				{
					if ( 0 )
					{
						// A change in plans here...
						// We now talk through the interface panel...
						if (gpCurrentTalkingFace->video_overlay != NULL)
						{
							RemoveVideoOverlay(gpCurrentTalkingFace->video_overlay);
							gpCurrentTalkingFace->video_overlay = NULL;
						}
						gfFacePanelActive = FALSE;

						RemoveVideoOverlay(g_text_box_overlay);
						g_text_box_overlay = NULL;

						if ( fTextBoxMouseRegionCreated )
						{
							MSYS_RemoveRegion( &gTextBoxMouseRegion );
							fTextBoxMouseRegionCreated = FALSE;
						}

						// Setup UI again!
						CreateTalkingUI(gbUIHandlerID, pSoldier->face, pSoldier->ubProfile, gzQuoteStr);
					}
				}
			}
		}
	}
}


static void CheckForStopTimeQuotes(UINT16 usQuoteNum);
static void HandleTacticalSpeechUI(UINT8 ubCharacterNum, FACETYPE* face);


void HandleDialogue()
{
	static BOOLEAN fOldEngagedInConvFlagOn = FALSE;

	// we don't want to just delay action of some events, we want to pause the whole queue, regardless of the event
	if (gfDialogueQueuePaused) return;

	bool const empty = ghDialogueQ->IsEmpty();

	if (empty && gpCurrentTalkingFace == NULL)
	{
		HandlePendingInitConv();
	}

	HandleCivQuote();

	// Alrighty, check for a change in state, do stuff appropriately....
	// Turned on
	if (!fOldEngagedInConvFlagOn && gTacticalStatus.uiFlags & ENGAGED_IN_CONV)
	{
		// OK, we have just entered...
		fOldEngagedInConvFlagOn = TRUE;

		PauseGame();
		LockPauseState(LOCK_PAUSE_14);
	}
	else if (fOldEngagedInConvFlagOn && !(gTacticalStatus.uiFlags & ENGAGED_IN_CONV))
	{
		// OK, we left...
		fOldEngagedInConvFlagOn = FALSE;

		UnLockPauseState();
		UnPauseGame();

		// if we're exiting boxing with the UI lock set then DON'T OVERRIDE THIS!
		if (!(gTacticalStatus.uiFlags & IGNORE_ENGAGED_IN_CONV_UI_UNLOCK))
		{
			switch (gTacticalStatus.bBoxingState)
			{
				case WON_ROUND:
				case LOST_ROUND:
				case DISQUALIFIED:
					break;

				default:
					guiPendingOverrideEvent = LU_ENDUILOCK;
					HandleTacticalUI();

					// ATE: If this is NOT the player's turn.. engage AI UI lock!
					if (gTacticalStatus.ubCurrentTeam != gbPlayerNum)
					{
						// Setup locked UI
						guiPendingOverrideEvent = LU_BEGINUILOCK;
						HandleTacticalUI();
					}
					break;
			}
		}

		gTacticalStatus.uiFlags &= ~IGNORE_ENGAGED_IN_CONV_UI_UNLOCK;
	}

	if (gTacticalStatus.uiFlags & ENGAGED_IN_CONV &&
			!gfInTalkPanel                            && // Are we in here because of the dialogue system up?
			guiPendingScreen != MSG_BOX_SCREEN        && // ATE: NOT if we have a message box pending
			guiCurrentScreen != MSG_BOX_SCREEN)
	{
		// No, so we should lock the UI!
		guiPendingOverrideEvent = LU_BEGINUILOCK;
		HandleTacticalUI();
	}

	// OK, check if we are still taking
	if (gpCurrentTalkingFace != NULL)
	{
		if (gpCurrentTalkingFace->fTalking)
		{
			// ATE: OK, MANAGE THE DISPLAY OF OUR CURRENTLY ACTIVE FACE IF WE / IT CHANGES STATUS
			// THINGS THAT CAN CHANGE STATUS:
			//		CHANGE TO MAPSCREEN
			//		CHANGE TO GAMESCREEN
			//		CHANGE IN MERC STATUS TO BE IN A SQUAD
			//    CHANGE FROM TEAM TO INV INTERFACE

			// Where are we and where did this face once exist?
			if (guiScreenIDUsedWhenUICreated == GAME_SCREEN && guiCurrentScreen == MAP_SCREEN)
			{
				// GO FROM GAMESCREEN TO MAPSCREEN

				// delete face panel if there is one!
				if (gfFacePanelActive)
				{
					// Set face inactive!
					if (gpCurrentTalkingFace->video_overlay != NULL)
					{
						RemoveVideoOverlay(gpCurrentTalkingFace->video_overlay);
						gpCurrentTalkingFace->video_overlay = NULL;
					}

					if (fExternFaceBoxRegionCreated)
					{
						fExternFaceBoxRegionCreated = FALSE;
						MSYS_RemoveRegion(&gFacePopupMouseRegion);
					}

					// Set face inactive....
					gpCurrentTalkingFace->fCanHandleInactiveNow = TRUE;
					SetAutoFaceInActive(gpCurrentTalkingFace);
					HandleTacticalSpeechUI(gubCurrentTalkingID, gpCurrentTalkingFace);

          // ATE: Force mapscreen to set face active again.....
        	fReDrawFace = TRUE;
					DrawFace();

					gfFacePanelActive = FALSE;
				}

				guiScreenIDUsedWhenUICreated = guiCurrentScreen;
			}
			else if (guiScreenIDUsedWhenUICreated == MAP_SCREEN && guiCurrentScreen == GAME_SCREEN)
			{
				HandleTacticalSpeechUI(gubCurrentTalkingID, gpCurrentTalkingFace);
				guiScreenIDUsedWhenUICreated = guiCurrentScreen;
			}
			return;
		}

		// Check special flags
		// If we are done, check special face flag for trigger NPC!
		if (gpCurrentTalkingFace->uiFlags & FACE_PCTRIGGER_NPC)
		{
			 // Decrement refrence count...
			 giNPCReferenceCount--;

			 FACETYPE& f = *gpCurrentTalkingFace;
			 TriggerNPCRecord(f.u.trigger.npc, f.u.trigger.record);
			 //Reset flag!
			 gpCurrentTalkingFace->uiFlags &= ~FACE_PCTRIGGER_NPC;
		}

		if (gpCurrentTalkingFace->uiFlags & FACE_MODAL)
		{
			gpCurrentTalkingFace->uiFlags &= ~FACE_MODAL;
			EndModalTactical();
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Ending Modal Tactical Quote.");
		}

		if (gpCurrentTalkingFace->uiFlags & FACE_TRIGGER_PREBATTLE_INT)
		{
			UnLockPauseState();
			InitPreBattleInterface(gpCurrentTalkingFace->u.initiating_battle.group, TRUE);
			//Reset flag!
			gpCurrentTalkingFace->uiFlags &= ~FACE_TRIGGER_PREBATTLE_INT;
		}

		gpCurrentTalkingFace = NULL;
		gubCurrentTalkingID	 = NO_PROFILE;
		gTacticalStatus.ubLastQuoteProfileNUm = NO_PROFILE;

		if (fWasPausedDuringDialogue)
		{
			fWasPausedDuringDialogue = FALSE;
			UnLockPauseState();
			UnPauseGame();
		}
	}

	if (empty)
	{
		if (gfMikeShouldSayHi == TRUE)
		{
			SOLDIERTYPE* const pMike = FindSoldierByProfileID(MIKE);
			if (pMike)
			{
				INT16 const sPlayerGridNo = ClosestPC(pMike, NULL);
				if (sPlayerGridNo != NOWHERE)
				{
					SOLDIERTYPE* const player = WhoIsThere2(sPlayerGridNo, 0);
					if (player != NULL)
					{
						InitiateConversation(pMike, player, NPC_INITIAL_QUOTE);
						gMercProfiles[pMike->ubProfile].ubMiscFlags2 |= PROFILE_MISC_FLAG2_SAID_FIRSTSEEN_QUOTE;
						// JA2Gold: special hack value of 2 to prevent dialogue from coming up more than once
						gfMikeShouldSayHi = 2;
					}
				}
			}
		}

		return;
	}

	// If here, pick current one from queue and play
	DialogueEvent* const d = ghDialogueQ->Remove();

	// If we are in auto bandage, ignore any quotes!
	if (gTacticalStatus.fAutoBandageMode)
	{
		if (d->fPauseTime)
		{
			UnLockPauseState();
			UnPauseGame();
		}

		delete d;
		return;
	}

	if (d->Execute())
	{
		ghDialogueQ->Add(d);
	}
	else
	{
		delete d;
	}
}


void DialogueEvent::Add(DialogueEvent* const d)
{
	try
	{
		ghDialogueQ->Add(d);
	}
	catch (...)
	{
		delete d;
		throw;
	}
}


void DialogueEvent::Pause()
{
	if (GamePaused()) return;

	PauseGame();
	LockPauseState(LOCK_PAUSE_15);
	fWasPausedDuringDialogue = TRUE;
}


bool CharacterDialogueEvent::MayExecute() const
{
	return !SoundIsPlaying(soldier_.uiBattleSoundID);
}


void MakeCharacterDialogueEventSleep(SOLDIERTYPE& s, bool const sleep)
{
	class CharacterDialogueEventSleep : public CharacterDialogueEvent
	{
		public:
			CharacterDialogueEventSleep(SOLDIERTYPE& soldier, bool const sleep) :
				CharacterDialogueEvent(soldier),
				sleep_(sleep)
			{}

			bool Execute()
			{
				if (!MayExecute()) return true;

				soldier_.fMercAsleep     = sleep_; // wake merc up or put them back down?
				fCharacterInfoPanelDirty = TRUE;
				fTeamPanelDirty          = TRUE;
				return false;
			}

		private:
			bool const sleep_;
	};

	DialogueEvent::Add(new CharacterDialogueEventSleep(s, sleep));
}


static bool CanSayQuote(SOLDIERTYPE const& s, UINT16 const quote)
{
	if (s.ubProfile == NO_PROFILE)        return false;
	INT8 const min_life = quote == QUOTE_SERIOUSLY_WOUNDED ? CONSCIOUSNESS : OKLIFE;
	if (s.bLife < min_life)               return false;
	if (AM_A_ROBOT(&s))                   return false;
	if (s.uiStatusFlags & SOLDIER_GASSED) return false;
	if (s.bAssignment == ASSIGNMENT_POW)  return false;
	return true;
}


BOOLEAN DelayedTacticalCharacterDialogue( SOLDIERTYPE *pSoldier, UINT16 usQuoteNum )
{
	if (!CanSayQuote(*pSoldier, usQuoteNum)) return FALSE;
	CharacterDialogue(pSoldier->ubProfile, usQuoteNum, pSoldier->face, DIALOGUE_TACTICAL_UI, TRUE, TRUE);
	return TRUE;
}


BOOLEAN TacticalCharacterDialogue(const SOLDIERTYPE* pSoldier, UINT16 usQuoteNum)
{
	if (!CanSayQuote(*pSoldier, usQuoteNum)) return FALSE;

	if ( AreInMeanwhile( ) )
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
			return TacticalCharacterDialogue(pSoldier->robot_remote_holder, usQuoteNum);
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

	CharacterDialogue(pSoldier->ubProfile, usQuoteNum, pSoldier->face, DIALOGUE_TACTICAL_UI, TRUE, FALSE);
	return TRUE;
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


void CharacterDialogue(UINT8 const character, UINT16 const quote, FACETYPE* const face, DialogueHandler const dialogue_handler, BOOLEAN const fFromSoldier, BOOLEAN const fDelayed)
{
	class DialogueEventQuote : public DialogueEvent
	{
		public:
			DialogueEventQuote(ProfileID const character, UINT16 const quote, FACETYPE* const face_, DialogueHandler const dialogue_handler, bool const from_soldier, bool const delayed) :
				quote_(quote),
				character_(character),
				dialogue_handler_(dialogue_handler),
				face(face_),
				from_soldier_(from_soldier),
				delayed_(delayed)
			{}

			bool Execute()
			{
				// Check if this one is to be delayed until we gain control.
				if (delayed_ && gTacticalStatus.ubCurrentTeam != gbPlayerNum) return true;

				// Try to find soldier...
				SOLDIERTYPE* s = FindSoldierByProfileIDOnPlayerTeam(character_);
				if (s && SoundIsPlaying(s->uiBattleSoundID))
				{ // Place back in!
					return true;
				}

				if (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN) Pause();

				if (s && s->fMercAsleep) // wake grunt up to say
				{
					s->fMercAsleep = FALSE;

					// refresh map screen
					fCharacterInfoPanelDirty = TRUE;
					fTeamPanelDirty = TRUE;

					// allow them to go back to sleep
					MakeCharacterDialogueEventSleep(*s, true);
				}

				gTacticalStatus.ubLastQuoteSaid       = quote_;
				gTacticalStatus.ubLastQuoteProfileNUm = character_;

				ExecuteCharacterDialogue(character_, quote_, face, dialogue_handler_, from_soldier_);

				s = FindSoldierByProfileID(character_);
				if (s && s->bTeam == gbPlayerNum)
				{
					CheckForStopTimeQuotes(quote_);
				}

				return false;
			}

		private:
			UINT16          const quote_;
			UINT8           const character_;
			DialogueHandler const dialogue_handler_;
			FACETYPE*       const face;
			bool            const from_soldier_;
			bool            const delayed_;
	};

	DialogueEvent::Add(new DialogueEventQuote(character, quote, face, dialogue_handler, fFromSoldier, fDelayed));
}


void CharacterDialogueUsingAlternateFile(SOLDIERTYPE& s, UINT16 const quote, DialogueHandler const handler)
{
	class CharacterDialogueEventUsingAlternateFile : public CharacterDialogueEvent
	{
		public:
			CharacterDialogueEventUsingAlternateFile(SOLDIERTYPE& soldier, UINT16 const quote, DialogueHandler const handler) :
				CharacterDialogueEvent(soldier),
				quote_(quote),
				handler_(handler)
			{}

			bool Execute()
			{
				if (!MayExecute()) return true;

				gfUseAlternateDialogueFile = TRUE;
				SOLDIERTYPE const& s = soldier_;
				ExecuteCharacterDialogue(s.ubProfile, quote_, s.face, handler_, TRUE);
				gfUseAlternateDialogueFile = FALSE;
				return false;
			}

		private:
			UINT16          const quote_;
			DialogueHandler const handler_;
	};

	DialogueEvent::Add(new CharacterDialogueEventUsingAlternateFile(s, gTacticalStatus.ubGuideDescriptionToUse, handler));
}


static BOOLEAN GetDialogue(UINT8 ubCharacterNum, UINT16 usQuoteNum, UINT32 iDataSize, wchar_t* zDialogueText, size_t Length, CHAR8* zSoundString);


// execute specific character dialogue
BOOLEAN ExecuteCharacterDialogue(UINT8 const ubCharacterNum, UINT16 const usQuoteNum, FACETYPE* const face, DialogueHandler const bUIHandlerID, BOOLEAN const fFromSoldier)
{
	gpCurrentTalkingFace = face;
	gubCurrentTalkingID  = ubCharacterNum;

	CHAR8		zSoundString[ 164 ];

	// Check if we are dead now or not....( if from a soldier... )

	// Try to find soldier...
	const SOLDIERTYPE* const pSoldier = FindSoldierByProfileIDOnPlayerTeam(ubCharacterNum);
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
		if (pSoldier->fMercAsleep)
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
			FOR_ALL_IN_TEAM(s, gbPlayerNum)
			{
				if (s->ubProfile != ubCharacterNum &&
						OkControllableMerc(s) &&
						SpacesAway(pSoldier->sGridNo, s->sGridNo) < 5)
				{
					// if this merc disliked the whining character sufficiently and hasn't already retorted
					if (gMercProfiles[s->ubProfile].bMercOpinion[ubCharacterNum] < -2 &&
							!(s->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_ANNOYING_MERC))
					{
						// make a comment!
						TacticalCharacterDialogue(s, QUOTE_ANNOYING_PC);
						s->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_ANNOYING_MERC;
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
	CHECKF(face != NULL);

  if (!GetDialogue(ubCharacterNum, usQuoteNum, DIALOGUESIZE, gzQuoteStr, lengthof(gzQuoteStr), zSoundString))
  {
    return( FALSE );
  }

	if( bUIHandlerID == DIALOGUE_EXTERNAL_NPC_UI )
	{
		// external NPC
		SetFaceTalking(face, zSoundString, gzQuoteStr);
	}
	else
	{
		// start "talking" system (portrait animation and start wav sample)
		SetFaceTalking(face, zSoundString, gzQuoteStr);
	}
	CreateTalkingUI(bUIHandlerID, face, ubCharacterNum, gzQuoteStr);

	// Set global handleer ID value, used when face desides it's done...
	gbUIHandlerID = bUIHandlerID;

	guiScreenIDUsedWhenUICreated = guiCurrentScreen;

	return( TRUE );
}


static void DisplayTextForExternalNPC(UINT8 ubCharacterNum, const wchar_t* zQuoteStr);
static void HandleExternNPCSpeechFace(FACETYPE* face);
static void HandleTacticalNPCTextUI(UINT8 ubCharacterNum, const wchar_t* zQuoteStr);
static void HandleTacticalTextUI(ProfileID profile_id, const wchar_t* zQuoteStr);


static void CreateTalkingUI(DialogueHandler const bUIHandlerID, FACETYPE* const face, UINT8 const ubCharacterNum, wchar_t const* const zQuoteStr)
{
	// Show text, if on
	if (gGameSettings.fOptions[TOPTION_SUBTITLES] || !face->fValidSpeech)
	{
		switch (bUIHandlerID)
		{
			case DIALOGUE_TACTICAL_UI:           HandleTacticalTextUI(            ubCharacterNum, zQuoteStr); break;
			case DIALOGUE_NPC_UI:                HandleTacticalNPCTextUI(         ubCharacterNum, zQuoteStr); break;
			case DIALOGUE_CONTACTPAGE_UI:        DisplayTextForMercFaceVideoPopUp(                zQuoteStr); break;
			case DIALOGUE_SPECK_CONTACT_PAGE_UI: DisplayTextForSpeckVideoPopUp(                   zQuoteStr); break;
			case DIALOGUE_EXTERNAL_NPC_UI:       DisplayTextForExternalNPC(       ubCharacterNum, zQuoteStr); break;
			case DIALOGUE_SHOPKEEPER_UI:         InitShopKeeperSubTitledText(                     zQuoteStr); break;
		}
	}

	if (gGameSettings.fOptions[TOPTION_SPEECH])
	{
		switch (bUIHandlerID)
		{
			case DIALOGUE_TACTICAL_UI:           HandleTacticalSpeechUI(ubCharacterNum, face); break;
			case DIALOGUE_CONTACTPAGE_UI:                                                      break;
			case DIALOGUE_SPECK_CONTACT_PAGE_UI:                                               break;
			case DIALOGUE_EXTERNAL_NPC_UI:       HandleExternNPCSpeechFace(face);              break;
		}
	}
}


const char* GetDialogueDataFilename(UINT8 ubCharacterNum, UINT16 usQuoteNum, BOOLEAN fWavFile)
{
	static char zFileName[164];
	UINT8		ubFileNumID;

	// Are we an NPC OR an RPC that has not been recruited?
	// ATE: Did the || clause here to allow ANY RPC that talks while the talking menu is up to use an npc quote file
	if ( gfUseAlternateDialogueFile )
	{
		if ( fWavFile )
		{
			// build name of wav file (characternum + quotenum)
			sprintf(zFileName, "NPC_SPEECH/d_%03d_%03d.wav", ubCharacterNum, usQuoteNum);
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
		if (ubCharacterNum >= HERVE && ubCharacterNum <= CARLO)
		{
			ubFileNumID = HERVE;
		}

		// If we are character #155, check fact!
		if ( ubCharacterNum == MANNY && !gubFact[FACT_MANNY_IS_BARTENDER] )
		{
			ubFileNumID = MANNY;
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
#if defined RUSSIAN || defined RUSSIAN_GOLD
			if (ubCharacterNum >= FIRST_RPC && gMercProfiles[ubCharacterNum].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED)
			{
				sprintf(zFileName, "SPEECH/r_%03d_%03d.wav", ubCharacterNum, usQuoteNum);
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


static BOOLEAN GetDialogue(UINT8 ubCharacterNum, UINT16 usQuoteNum, UINT32 iDataSize, wchar_t* zDialogueText, size_t Length, CHAR8* zSoundString)
{
   // first things first  - grab the text (if player has SUBTITLE PREFERENCE ON)
   //if ( gGameSettings.fOptions[ TOPTION_SUBTITLES ] )
   {
			const char* pFilename = GetDialogueDataFilename(ubCharacterNum, 0, FALSE);
			bool success;
			try
			{
				LoadEncryptedDataFromFile(pFilename, zDialogueText, usQuoteNum * iDataSize, iDataSize);
				success = zDialogueText[0] != L'\0';
			}
			catch (...) { success = false; }
			if (!success)
			{
				swprintf(zDialogueText, Length, L"I have no text in the EDT file (%d) %hs", usQuoteNum, pFilename);
#ifndef JA2BETAVERSION
				return( FALSE );
#endif
			}
   }

	// CHECK IF THE FILE EXISTS, IF NOT, USE DEFAULT!
	const char* pFilename = GetDialogueDataFilename(ubCharacterNum, usQuoteNum, TRUE);
	strcpy( zSoundString, pFilename );
 return(TRUE);
}


// Handlers for tactical UI stuff
static void HandleTacticalNPCTextUI(const UINT8 ubCharacterNum, const wchar_t* const zQuoteStr)
{
	// Setup dialogue text box
	if ( guiCurrentScreen != MAP_SCREEN )
	{
		gTalkPanel.fRenderSubTitlesNow = TRUE;
		gTalkPanel.fSetupSubTitles = TRUE;
	}

	// post message to mapscreen message system
	swprintf( gTalkPanel.zQuoteStr, lengthof(gTalkPanel.zQuoteStr), L"\"%ls\"", zQuoteStr );
	MapScreenMessage(FONT_MCOLOR_WHITE, MSG_DIALOG, L"%ls: \"%ls\"", GetProfile(ubCharacterNum)->zNickname, zQuoteStr);
}


static void ExecuteTacticalTextBox(INT16 sLeftPosition, const wchar_t* pString);


// Handlers for tactical UI stuff
static void DisplayTextForExternalNPC(const UINT8 ubCharacterNum, const wchar_t* const zQuoteStr)
{
	INT16									sLeft;

	// Setup dialogue text box
	if ( guiCurrentScreen != MAP_SCREEN )
	{
		gTalkPanel.fRenderSubTitlesNow = TRUE;
		gTalkPanel.fSetupSubTitles = TRUE;
	}

	// post message to mapscreen message system
	swprintf( gTalkPanel.zQuoteStr, lengthof(gTalkPanel.zQuoteStr), L"\"%ls\"", zQuoteStr );
	MapScreenMessage(FONT_MCOLOR_WHITE, MSG_DIALOG, L"%ls: \"%ls\"", GetProfile(ubCharacterNum)->zNickname, zQuoteStr);

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


static void HandleTacticalTextUI(const ProfileID profile_id, const wchar_t* const zQuoteStr)
{
	wchar_t								zText[ QUOTE_MESSAGE_SIZE ];
	INT16									sLeft = 0;

	swprintf( zText, lengthof(zText), L"\"%ls\"", zQuoteStr );
	sLeft	= 110;


	//previous version
	//sLeft = 110;

	ExecuteTacticalTextBox( sLeft, zText );

	MapScreenMessage(FONT_MCOLOR_WHITE, MSG_DIALOG, L"%ls: \"%ls\"", GetProfile(profile_id)->zNickname, zQuoteStr);
}


static void RenderSubtitleBoxOverlay(VIDEO_OVERLAY* pBlitter);
static void TextOverlayClickCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void ExecuteTacticalTextBox(const INT16 sLeftPosition, const wchar_t* const pString)
{
	// check if mouse region created, if so, do not recreate
	if (fTextBoxMouseRegionCreated) return;

	// Prepare text box
	g_dialogue_box = PrepareMercPopupBox(g_dialogue_box, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, pString, DIALOGUE_DEFAULT_SUBTITLE_WIDTH, 0, 0, 0, &gusSubtitleBoxWidth, &gusSubtitleBoxHeight);

	INT16  const x = sLeftPosition;
	INT16  const y = gsTopPosition;
	UINT16 const w = gusSubtitleBoxWidth;
	UINT16 const h = gusSubtitleBoxHeight;

	g_text_box_overlay = RegisterVideoOverlay(RenderSubtitleBoxOverlay, x, y, w, h);

	gsTopPosition = 20;

	//Define main region
	MSYS_DefineRegion(&gTextBoxMouseRegion, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL, MSYS_NO_CALLBACK, TextOverlayClickCallback);

	fTextBoxMouseRegionCreated = TRUE;
}


static void FaceOverlayClickCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void RenderFaceOverlay(VIDEO_OVERLAY* pBlitter);


static void HandleExternNPCSpeechFace(FACETYPE* const face)
{
	// Enable it!
	SetAutoFaceActive(FACE_AUTO_DISPLAY_BUFFER, FACE_AUTO_RESTORE_BUFFER, face, 0, 0);

	// Set flag to say WE control when to set inactive!
	face->uiFlags |= FACE_INACTIVE_HANDLED_ELSEWHERE;

	INT16       x;
	INT16       y;
	INT16 const w = 99;
	INT16 const h = 98;
	if (guiCurrentScreen != MAP_SCREEN)
	{
		x = 10;
		y = 20;
	}
	else
	{
		x = gsExternPanelXPosition;
		y = gsExternPanelYPosition;
	}

	gpCurrentTalkingFace->video_overlay = RegisterVideoOverlay(RenderFaceOverlay, x, y, w, h);

	RenderAutoFace(face);

	// ATE: Create mouse region.......
	if ( !fExternFaceBoxRegionCreated )
	{
		fExternFaceBoxRegionCreated = TRUE;

		//Define main region
		MSYS_DefineRegion(&gFacePopupMouseRegion, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL, MSYS_NO_CALLBACK, FaceOverlayClickCallback);
	}

	gfFacePanelActive = TRUE;
}


static void HandleTacticalSpeechUI(const UINT8 ubCharacterNum, FACETYPE* const face)
{
	BOOLEAN								fDoExternPanel = FALSE;

	// Get soldier pointer, if there is one...
	SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubCharacterNum);

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
		SetAutoFaceActive(FACE_AUTO_DISPLAY_BUFFER, FACE_AUTO_RESTORE_BUFFER, face, 0, 0);

		// Set flag to say WE control when to set inactive!
		face->uiFlags |= FACE_INACTIVE_HANDLED_ELSEWHERE | FACE_MAKEACTIVE_ONCE_DONE;

		// IF we are in tactical and this soldier is on the current squad
		if ( ( guiCurrentScreen == GAME_SCREEN ) && ( pSoldier != NULL ) && ( pSoldier->bAssignment == iCurrentTacticalSquad ) )
		{
			// Make the interface panel dirty..
			// This will dirty the panel next frame...
			gfRerenderInterfaceFromHelpText = TRUE;
		}

		INT16 const x = 10;
		INT16 const y = 20;
		INT16 const w = 99;
		INT16 const h = 98;

		gpCurrentTalkingFace->video_overlay = RegisterVideoOverlay(RenderFaceOverlay, x, y, w, h);

		RenderAutoFace(face);

		// ATE: Create mouse region.......
		if ( !fExternFaceBoxRegionCreated )
		{
			fExternFaceBoxRegionCreated = TRUE;

			//Define main region
			MSYS_DefineRegion(&gFacePopupMouseRegion, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL, MSYS_NO_CALLBACK, FaceOverlayClickCallback);
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
				SetAutoFaceInActive(pFace);
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
				SetAutoFaceInActive(pFace);
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
				if (g_text_box_overlay != NULL)
				{
					RemoveVideoOverlay(g_text_box_overlay);
					g_text_box_overlay = NULL;

					if ( fTextBoxMouseRegionCreated )
					{
						RemoveMercPopupBox(g_dialogue_box);
						g_dialogue_box = 0;

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
				RemoveMercPopupBox(g_interface_dialogue_box);
				g_interface_dialogue_box = 0;
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
	INT16 sFontX, sFontY;
	wchar_t					zTownIDString[50];


	if ( gpCurrentTalkingFace == NULL )
	{
		return;
	}

	if ( gfFacePanelActive )
	{
		const SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(gpCurrentTalkingFace->ubCharacterNum);

		// a living soldier?..or external NPC?..choose panel based on this
		const SGPVObject* const vo = (pSoldier ? guiCOMPANEL : guiCOMPANELB);
		BltVideoObject(pBlitter->uiDestBuff, vo, 0, pBlitter->sX, pBlitter->sY);

		// Display name, location ( if not current )
		SetFontAttributes(BLOCKFONT2, FONT_MCOLOR_LTGRAY);

		if ( pSoldier )
		{
		  //reset the font dest buffer
		  SetFontDestBuffer(pBlitter->uiDestBuff);

			FindFontCenterCoordinates(pBlitter->sX + 12, pBlitter->sY + 55, 73, 9, pSoldier->name, BLOCKFONT2, &sFontX, &sFontY);
			MPrint(sFontX, sFontY, pSoldier->name);

			// What sector are we in, ( and is it the same as ours? )
			if ( pSoldier->sSectorX != gWorldSectorX || pSoldier->sSectorY != gWorldSectorY || pSoldier->bSectorZ != gbWorldSectorZ || pSoldier->fBetweenSectors )
			{
				GetSectorIDString( pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ, zTownIDString, lengthof(zTownIDString), FALSE );

        ReduceStringLength( zTownIDString, lengthof(zTownIDString), 64 , BLOCKFONT2 );

				FindFontCenterCoordinates(pBlitter->sX + 12, pBlitter->sY + 68, 73, 9, zTownIDString, BLOCKFONT2, &sFontX, &sFontY);
				MPrint(sFontX, sFontY, zTownIDString);
			}

		  //reset the font dest buffer
			SetFontDestBuffer(FRAME_BUFFER);

			DrawSoldierUIBars(pSoldier, pBlitter->sX + 69, pBlitter->sY + 47, FALSE, pBlitter->uiDestBuff);
		}
		else
		{
			FindFontCenterCoordinates(pBlitter->sX + 9, pBlitter->sY + 55, 73, 9, gMercProfiles[gpCurrentTalkingFace->ubCharacterNum].zNickname, BLOCKFONT2, &sFontX, &sFontY);
			MPrint(sFontX, sFontY, gMercProfiles[gpCurrentTalkingFace->ubCharacterNum].zNickname);
		}

		//RenderAutoFace( gpCurrentTalkingFace->iID );

		SGPBox const r = { 0, 0, gpCurrentTalkingFace->usFaceWidth, gpCurrentTalkingFace->usFaceHeight };
		BltVideoSurface(pBlitter->uiDestBuff, gpCurrentTalkingFace->uiAutoDisplayBuffer, pBlitter->sX + 14, pBlitter->sY + 6, &r);

		InvalidateRegion( pBlitter->sX, pBlitter->sY, pBlitter->sX + 99, pBlitter->sY + 98 );
	}
}


static void RenderSubtitleBoxOverlay(VIDEO_OVERLAY* pBlitter)
{
	if (g_text_box_overlay == NULL) return;

	RenderMercPopUpBox(g_dialogue_box, pBlitter->sX, pBlitter->sY, pBlitter->uiDestBuff);
	InvalidateRegion(pBlitter->sX, pBlitter->sY, pBlitter->sX + gusSubtitleBoxWidth, pBlitter->sY + gusSubtitleBoxHeight);
}


/* Let Red talk, if he is in the list and the quote is QUOTE_AIR_RAID.  Choose
 * somebody else otherwise */
static void ChooseRedIfPresentAndAirRaid(SOLDIERTYPE*const*const mercs_in_sector, size_t merc_count, UINT16 quote)
{
	if (merc_count == 0) return;

	SOLDIERTYPE* chosen;
	if (quote == QUOTE_AIR_RAID)
	{
		for (SOLDIERTYPE*const* i = mercs_in_sector; i != mercs_in_sector + merc_count; ++i)
		{
			if ((*i)->ubProfile == RED)
			{
				chosen = *i;
				goto talk;
			}
		}
	}
	chosen = mercs_in_sector[Random(merc_count)];
talk:
	TacticalCharacterDialogue(chosen, quote);
}


void SayQuoteFromAnyBodyInSector( UINT16 usQuoteNum )
{
	// Loop through all our guys and randomly say one from someone in our sector
	size_t merc_count = 0;
	SOLDIERTYPE* mercs_in_sector[20];
	FOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		// Add guy if he's a candidate...
		if (OkControllableMerc(s) &&
				!AM_AN_EPC(s) &&
				!(s->uiStatusFlags & SOLDIER_GASSED) &&
				!AM_A_ROBOT(s) &&
				!s->fMercAsleep)
		{
			if (gTacticalStatus.bNumFoughtInBattle[ENEMY_TEAM] == 0)
			{
				// quotes referring to Deidranna's men so we skip quote if there were no army guys fought
				if (usQuoteNum == QUOTE_SECTOR_SAFE &&
						(s->ubProfile == IRA || s->ubProfile == MIGUEL || s->ubProfile == SHANK))
				{
					continue;
				}
				if (usQuoteNum == QUOTE_ENEMY_PRESENCE &&
						(s->ubProfile == IRA || s->ubProfile == DIMITRI || s->ubProfile == DYNAMO || s->ubProfile == SHANK))
				{
					continue;
				}
			}

			mercs_in_sector[merc_count++] = s;
		}
	}

	ChooseRedIfPresentAndAirRaid(mercs_in_sector, merc_count, usQuoteNum);
}


void SayQuoteFromAnyBodyInThisSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ, UINT16 usQuoteNum )
{
	// Loop through all our guys and randomly say one from someone in our sector
	size_t merc_count = 0;
	SOLDIERTYPE* mercs_in_sector[20];
	FOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		// Add guy if he's a candidate...
		if (s->sSectorX == sSectorX && s->sSectorY == sSectorY && s->bSectorZ == bSectorZ && !AM_AN_EPC(s) && !(s->uiStatusFlags & SOLDIER_GASSED) && !AM_A_ROBOT(s) && !s->fMercAsleep)
		{
			mercs_in_sector[merc_count++] = s;
		}
	}

	ChooseRedIfPresentAndAirRaid(mercs_in_sector, merc_count, usQuoteNum);
}


void SayQuoteFromNearbyMercInSector( INT16 sGridNo, INT8 bDistance, UINT16 usQuoteNum )
{
	UINT8	ubNumMercs = 0;

	// Loop through all our guys and randomly say one from someone in our sector

	// run through list
	SOLDIERTYPE* mercs_in_sector[20];
	FOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		// Add guy if he's a candidate...
		if (OkControllableMerc(s) &&
				PythSpacesAway(sGridNo, s->sGridNo) < bDistance &&
				!AM_AN_EPC(s) &&
				!(s->uiStatusFlags & SOLDIER_GASSED) &&
				!AM_A_ROBOT(s) &&
				!s->fMercAsleep &&
				SoldierTo3DLocationLineOfSightTest(s, sGridNo, 0, 0, MaxDistanceVisible(), TRUE))
		{
			if (usQuoteNum == 66 && Random(100) > EffectiveWisdom(s))
			{
				continue;
			}
			mercs_in_sector[ubNumMercs++] = s;
		}
	}

	if ( ubNumMercs > 0 )
	{
		SOLDIERTYPE* const chosen = mercs_in_sector[Random(ubNumMercs)];
		if (usQuoteNum == 66)
		{
			SetFactTrue( FACT_PLAYER_FOUND_ITEMS_MISSING );
		}
		TacticalCharacterDialogue(chosen, usQuoteNum);
	}
}


void SayQuote58FromNearbyMercInSector( INT16 sGridNo, INT8 bDistance, UINT16 usQuoteNum, INT8 bSex )
{
	UINT8	ubNumMercs = 0;

	// Loop through all our guys and randomly say one from someone in our sector

	// run through list
	SOLDIERTYPE* mercs_in_sector[20];
	FOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		// Add guy if he's a candidate...
		if (OkControllableMerc(s) &&
				PythSpacesAway(sGridNo, s->sGridNo) < bDistance &&
				!AM_AN_EPC(s) &&
				!(s->uiStatusFlags & SOLDIER_GASSED) &&
				!AM_A_ROBOT(s) &&
				!s->fMercAsleep &&
				SoldierTo3DLocationLineOfSightTest(s, sGridNo, 0, 0, MaxDistanceVisible(), TRUE))
		{
			// ATE: This is to check gedner for this quote...
			if (QuoteExp_GenderCode[s->ubProfile] == 0 && bSex == FEMALE)
			{
				continue;
			}

			if (QuoteExp_GenderCode[s->ubProfile] == 1 && bSex == MALE)
			{
				continue;
			}

			mercs_in_sector[ubNumMercs++] = s;
		}
	}

	if ( ubNumMercs > 0 )
	{
		SOLDIERTYPE* const chosen = mercs_in_sector[Random(ubNumMercs)];
		TacticalCharacterDialogue(chosen, usQuoteNum);
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
			InternalShutupaYoFace(gpCurrentTalkingFace, FALSE);
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
			InternalShutupaYoFace(gpCurrentTalkingFace, FALSE);
		}

	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		fLButtonDown = FALSE;
	}
}


UINT32 FindDelayForString(const wchar_t* const sString)
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


static bool IsStopTimeQuote(UINT16 const quote_id)
{
	for (UINT16 const* i = gusStopTimeQuoteList; i != endof(gusStopTimeQuoteList); ++i)
	{
		if (*i == quote_id) return true;
	}
	return false;
}


static void CheckForStopTimeQuotes(UINT16 const usQuoteNum)
{
	if (!IsStopTimeQuote(usQuoteNum)) return;
	// Stop Time, game
	EnterModalTactical(TACTICAL_MODAL_NOMOUSE);
	gpCurrentTalkingFace->uiFlags |= FACE_MODAL;
	ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Starting Modal Tactical Quote.");
}


void SetStopTimeQuoteCallback( MODAL_HOOK pCallBack )
{
	gModalDoneCallback = pCallBack;
}


BOOLEAN IsMercSayingDialogue(UINT8 ubProfileID)
{
	if ( gpCurrentTalkingFace != NULL && gubCurrentTalkingID == ubProfileID )
	{
		return( TRUE );
	}
	return( FALSE );
}


BOOLEAN GetMercPrecedentQuoteBitStatus(const MERCPROFILESTRUCT* const p, UINT8 const ubQuoteBit)
{
	return (p->uiPrecedentQuoteSaid & 1 << (ubQuoteBit - 1)) != 0;
}


void SetMercPrecedentQuoteBitStatus(MERCPROFILESTRUCT* const p, UINT8 const ubBitToSet)
{
	p->uiPrecedentQuoteSaid |= 1 << (ubBitToSet - 1);
}


UINT8	GetQuoteBitNumberFromQuoteID(UINT32 const uiQuoteID)
{
	for (size_t i = 0; i != lengthof(gubMercValidPrecedentQuoteID); ++i)
	{
		if (gubMercValidPrecedentQuoteID[i] == uiQuoteID) return i;
	}
	return 0;
}


void HandleShutDownOfMapScreenWhileExternfaceIsTalking( void )
{
	if ( ( fExternFaceBoxRegionCreated ) && ( gpCurrentTalkingFace) )
	{
		RemoveVideoOverlay(gpCurrentTalkingFace->video_overlay);
		gpCurrentTalkingFace->video_overlay = NULL;
	}
}


void HandleImportantMercQuote(SOLDIERTYPE* const s, UINT16 const usQuoteNumber)
{
	// Wake merc up for THIS quote
	bool const asleep = s->fMercAsleep;
	if (asleep) MakeCharacterDialogueEventSleep(*s, false);
	TacticalCharacterDialogue(s, usQuoteNumber);
	if (asleep) MakeCharacterDialogueEventSleep(*s, true);
}


void HandleImportantMercQuoteLocked(SOLDIERTYPE* const s, UINT16 const quote)
{
	LockMapScreenInterface(true);
	HandleImportantMercQuote(s, quote);
	LockMapScreenInterface(false);
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
