#include "Dialogue_Control.h"
#include "AI.h"
#include "AIMMembers.h"
#include "Assignments.h"
#include "Civ_Quotes.h"
#include "ContentManager.h"
#include "Cursors.h"
#include "Dialogs.h"
#include "Directories.h"
#include "Faces.h"
#include "Facts.h"
#include "Font.h"
#include "Font_Control.h"
#include "Game_Clock.h"
#include "GameInstance.h"
#include "GameLoop.h"
#include "GameRes.h"
#include "GameScreen.h"
#include "GameSettings.h"
#include "Handle_UI.h"
#include "Interface_Control.h"
#include "Interface_Dialogue.h"
#include "Interface_Utils.h"
#include "Isometric_Utils.h"
#include "JAScreens.h"
#include "Logger.h"
#include "LOS.h"
#include "Map_Screen_Interface.h"
#include "MapScreen.h"
#include "Meanwhile.h"
#include "MercProfile.h"
#include "Mercs.h"
#include "MercTextBox.h"
#include "Message.h"
#include "MouseSystem.h"
#include "NPC.h"
#include "OppList.h"
#include "Overhead.h"
#include "Overhead_Types.h"
#include "PreBattle_Interface.h"
#include "QArray.h"
#include "Quests.h"
#include "Random.h"
#include "Render_Dirty.h"
#include "RenderWorld.h"
#include "ScreenIDs.h"
#include "ShopKeeper_Interface.h"
#include "SkillCheck.h"
#include "Soldier_Macros.h"
#include "SoundMan.h"
#include "Squads.h"
#include "StrategicMap.h"
#include "UILayout.h"
#include "Video.h"
#include "VObject.h"
#include "VSurface.h"
#include "WCheck.h"
#include "WordWrap.h"
#include "WorldMan.h"
#include <map>
#include <queue>
#include <string_theory/format>
struct MercPopUpBox;


#define QUOTE_MESSAGE_SIZE			520

#define DIALOGUE_DEFAULT_SUBTITLE_WIDTH	200
#define TEXT_DELAY_MODIFIER			60


typedef std::queue<DialogueEvent*>DialogueQueue;

BOOLEAN fExternFacesLoaded = FALSE;

static std::map<ProfileID, FACETYPE*> externalNPCFaces;

const ProfileID preloadedExternalNPCFaces[] = {
	SKYRIDER,
	FRED,
	MATT,
	OSWALD,
	CALVIN,
	CARL
};


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
static DialogueQueue ghDialogueQ;
FACETYPE *gpCurrentTalkingFace	= NULL;
static ProfileID       gubCurrentTalkingID = NO_PROFILE;
static DialogueHandler gbUIHandlerID;

INT32 giNPCReferenceCount = 0;

static INT16 gsExternPanelXPosition;
static INT16 gsExternPanelYPosition;

static BOOLEAN        gfDialogueQueuePaused = FALSE;
static UINT16         gusSubtitleBoxWidth;
static UINT16         gusSubtitleBoxHeight;
static VIDEO_OVERLAY* g_text_box_overlay = NULL;
BOOLEAN               gfFacePanelActive = FALSE;
static UINT32         guiScreenIDUsedWhenUICreated;
static MOUSE_REGION   gTextBoxMouseRegion;
static MOUSE_REGION   gFacePopupMouseRegion;

MercPopUpBox* g_dialogue_box;


static BOOLEAN fWasPausedDuringDialogue = FALSE;

static INT8 gubLogForMeTooBleeds = FALSE;


// has the text region been created?
static BOOLEAN fTextBoxMouseRegionCreated  = FALSE;
static BOOLEAN fExternFaceBoxRegionCreated = FALSE;


static SGPVObject* guiCOMPANEL;
static SGPVObject* guiCOMPANELB;


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
	giNPCReferenceCount = 0;
	gsExternPanelXPosition = DEFAULT_EXTERN_PANEL_X_POS;
	gsExternPanelYPosition = DEFAULT_EXTERN_PANEL_Y_POS;
}

void ShutdownDialogueControl()
{
	EmptyDialogueQueue();

	// shutdown external static NPC faces
	UnloadExternalNPCFaces();

	// gte rid of portraits for cars
	UnLoadCarPortraits();
}


void LoadExternalNPCFace(ProfileID mercID)
{
	if (externalNPCFaces.find(mercID) == externalNPCFaces.end())
	{
		externalNPCFaces[mercID] = &InitFace(mercID, nullptr, FACE_FORCE_SMALL);
	}
}

FACETYPE* GetExternalNPCFace(ProfileID mercID)
{
	LoadExternalNPCFace(mercID); // ensure we have loaded the face
	return externalNPCFaces.at(mercID);
}

void PreloadExternalNPCFaces()
{
	// go and grab all external NPC faces that are needed for the game who won't exist as soldiertypes

	if (fExternFacesLoaded) return;

	fExternFacesLoaded = TRUE;

	for (size_t i = 0; i < lengthof(preloadedExternalNPCFaces); i++)
	{
		LoadExternalNPCFace(preloadedExternalNPCFaces[i]);
	}
}


void UnloadExternalNPCFaces()
{
	if (!fExternFacesLoaded) return;
	fExternFacesLoaded = FALSE;

	// Remove all external NPC faces.
	for (auto entry : externalNPCFaces)
	{
		DeleteFace(entry.second);
	}
}


void EmptyDialogueQueue()
{
	while(!ghDialogueQ.empty())
		ghDialogueQ.pop();

	gfWaitingForTriggerTimer = FALSE;
}


BOOLEAN DialogueQueueIsEmpty( )
{
	return ghDialogueQ.empty();
}


BOOLEAN	DialogueQueueIsEmptyAndNobodyIsTalking()
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


static void CheckForStopTimeQuotes(UINT16 usQuoteNum);
static void HandleTacticalSpeechUI(UINT8 ubCharacterNum, FACETYPE&);


void HandleDialogue()
{
	static BOOLEAN fOldEngagedInConvFlagOn = FALSE;

	// we don't want to just delay action of some events, we want to pause the whole queue, regardless of the event
	if (gfDialogueQueuePaused) return;

	bool const empty = ghDialogueQ.empty();

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
		LockPauseState(LOCK_PAUSE_ENGAGED_IN_CONV);
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
					if (gTacticalStatus.ubCurrentTeam != OUR_TEAM)
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
		!gfInTalkPanel && // Are we in here because of the dialogue system up?
		guiPendingScreen != MSG_BOX_SCREEN && // ATE: NOT if we have a message box pending
		guiCurrentScreen != MSG_BOX_SCREEN)
	{
		// No, so we should lock the UI!
		guiPendingOverrideEvent = LU_BEGINUILOCK;
		HandleTacticalUI();
	}

	// OK, check if we are still taking
	if (gpCurrentTalkingFace)
	{
		FACETYPE& f = *gpCurrentTalkingFace;
		if (f.fTalking)
		{
			// ATE: OK, MANAGE THE DISPLAY OF OUR CURRENTLY ACTIVE FACE IF WE / IT CHANGES STATUS
			// THINGS THAT CAN CHANGE STATUS:
			//   CHANGE TO MAPSCREEN
			//   CHANGE TO GAMESCREEN
			//   CHANGE IN MERC STATUS TO BE IN A SQUAD
			//   CHANGE FROM TEAM TO INV INTERFACE

			// Where are we and where did this face once exist?
			if (guiScreenIDUsedWhenUICreated == GAME_SCREEN && guiCurrentScreen == MAP_SCREEN)
			{
				// GO FROM GAMESCREEN TO MAPSCREEN

				// delete face panel if there is one!
				if (gfFacePanelActive)
				{
					// Set face inactive!
					if (f.video_overlay)
					{
						RemoveVideoOverlay(f.video_overlay);
						f.video_overlay = 0;
					}

					if (fExternFaceBoxRegionCreated)
					{
						fExternFaceBoxRegionCreated = FALSE;
						MSYS_RemoveRegion(&gFacePopupMouseRegion);
					}

					// Set face inactive....
					f.fCanHandleInactiveNow = TRUE;
					SetAutoFaceInActive(f);
					HandleTacticalSpeechUI(gubCurrentTalkingID, f);

					// ATE: Force mapscreen to set face active again.....
					fReDrawFace = TRUE;
					DrawFace();

					gfFacePanelActive = FALSE;
				}

				guiScreenIDUsedWhenUICreated = guiCurrentScreen;
			}
			else if (guiScreenIDUsedWhenUICreated == MAP_SCREEN && guiCurrentScreen == GAME_SCREEN)
			{
				HandleTacticalSpeechUI(gubCurrentTalkingID, f);
				guiScreenIDUsedWhenUICreated = guiCurrentScreen;
			}
			return;
		}

		// Check special flags
		// If we are done, check special face flag for trigger NPC!
		if (f.uiFlags & FACE_PCTRIGGER_NPC)
		{
			// Decrement refrence count...
			giNPCReferenceCount--;

			TriggerNPCRecord(f.u.trigger.npc, f.u.trigger.record);
			//Reset flag!
			f.uiFlags &= ~FACE_PCTRIGGER_NPC;
		}

		if (f.uiFlags & FACE_MODAL)
		{
			f.uiFlags &= ~FACE_MODAL;
			EndModalTactical();
			SLOGD("Ending Modal Tactical Quote.");
		}

		if (f.uiFlags & FACE_TRIGGER_PREBATTLE_INT)
		{
			UnLockPauseState();
			InitPreBattleInterface(f.u.initiating_battle.group, true);
			//Reset flag!
			f.uiFlags &= ~FACE_TRIGGER_PREBATTLE_INT;
		}

		gpCurrentTalkingFace = NULL;
		gubCurrentTalkingID = NO_PROFILE;
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
	DialogueEvent*const d = ghDialogueQ.front();

	// If we are in auto bandage, ignore any quotes!
	if (gTacticalStatus.fAutoBandageMode || !d->Execute())
	{
		delete d;
		if(!ghDialogueQ.empty()) ghDialogueQ.pop();
	}
}


void DialogueEvent::Add(DialogueEvent* const d)
{
	try
	{
		ghDialogueQ.push(d);
	}
	catch (...)
	{
		delete d;
		throw;
	}
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
	if (s.ubProfile == NO_PROFILE)
		return false;
	INT8 const min_life = quote == QUOTE_SERIOUSLY_WOUNDED ? CONSCIOUSNESS : OKLIFE;
	if (s.bLife < min_life)
		return false;
	if (AM_A_ROBOT(&s))
		return false;
	if (s.uiStatusFlags & SOLDIER_GASSED)
		return false;
	if (s.bAssignment == ASSIGNMENT_POW)
		return false;
	return true;
}


BOOLEAN DelayedTacticalCharacterDialogue( SOLDIERTYPE *pSoldier, UINT16 usQuoteNum )
{
	if (!CanSayQuote(*pSoldier, usQuoteNum))
		return FALSE;
	CharacterDialogue(pSoldier->ubProfile, usQuoteNum, pSoldier->face, DIALOGUE_TACTICAL_UI, TRUE, true);
	return TRUE;
}


BOOLEAN TacticalCharacterDialogue(const SOLDIERTYPE* pSoldier, UINT16 usQuoteNum)
{
	if (!CanSayQuote(*pSoldier, usQuoteNum))
		return FALSE;

	if ( AreInMeanwhile( ) )
	{
		return( FALSE );
	}

	// OK, let's check if this is the exact one we just played, if so, skip.
	if (pSoldier->ubProfile == gTacticalStatus.ubLastQuoteProfileNUm &&
		usQuoteNum == gTacticalStatus.ubLastQuoteSaid)
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

	if (AM_AN_EPC(pSoldier) &&
		!(gMercProfiles[pSoldier->ubProfile].ubMiscFlags & PROFILE_MISC_FLAG_FORCENPCQUOTE))
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

	CharacterDialogue(pSoldier->ubProfile, usQuoteNum, pSoldier->face, DIALOGUE_TACTICAL_UI, TRUE);
	return TRUE;
}

// This function takes a profile num, quote num, faceindex and a UI hander ID.
// What it does is queues up the dialog to be ultimately loaded/displayed
// FACEINDEX
//    The face index is an index into an ACTIVE face. The face is considered to
//    be active, and if it's not, either that has to be handled by the UI handler
//    ir nothing will show.  What this function does is set the face to talking,
//    and the face sprite system should handle the rest.
// bUIHandlerID
//    Because this could be used in any place, the UI handleID is used to differentiate
//    places in the game. For example, specific things happen in the tactical engine
//    that may not be the place where in the AIM contract screen uses.....

// NB; The queued system is not yet implemented, but will be transpatent to the caller....


void CharacterDialogue(UINT8 const character, UINT16 const quote, FACETYPE* const face, DialogueHandler const dialogue_handler, BOOLEAN const fFromSoldier, bool const delayed)
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
				if (delayed_ && gTacticalStatus.ubCurrentTeam != OUR_TEAM) return true;

				// Try to find soldier...
				SOLDIERTYPE* s = FindSoldierByProfileIDOnPlayerTeam(character_);
				if (s && SoundIsPlaying(s->uiBattleSoundID))
				{
					// Place back in!
					return true;
				}

				if (fInMapMode && !GamePaused())
				{
					PauseGame();
					LockPauseState(LOCK_PAUSE_MERC_TALKING);
					fWasPausedDuringDialogue = TRUE;
				}

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

				ExecuteCharacterDialogue(character_, quote_, face, dialogue_handler_, from_soldier_, false);

				s = FindSoldierByProfileID(character_);
				if (s && s->bTeam == OUR_TEAM)
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

	DialogueEvent::Add(new DialogueEventQuote(character, quote, face, dialogue_handler, fFromSoldier, delayed));
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

				SOLDIERTYPE const& s = soldier_;
				ExecuteCharacterDialogue(s.ubProfile, quote_, s.face, handler_, TRUE, true);
				return false;
			}

		private:
			UINT16          const quote_;
			DialogueHandler const handler_;
	};

	DialogueEvent::Add(new CharacterDialogueEventUsingAlternateFile(s, gTacticalStatus.ubGuideDescriptionToUse, handler));
}


static void CreateTalkingUI(DialogueHandler bUIHandlerID, FACETYPE& f, UINT8 ubCharacterNum, const ST::string& zQuoteStr);
static BOOLEAN GetDialogue(const MercProfile &profile, UINT16 usQuoteNum, ST::string& zDialogueText, CHAR8* zSoundString, bool useAlternateDialogueFile);


// execute specific character dialogue
BOOLEAN ExecuteCharacterDialogue(UINT8 const ubCharacterNum, UINT16 const usQuoteNum, FACETYPE* const face, DialogueHandler const bUIHandlerID, BOOLEAN const fFromSoldier, bool useAlternateDialogueFile)
{
	gpCurrentTalkingFace = face;
	gubCurrentTalkingID  = ubCharacterNum;

	CHAR8 zSoundString[164];

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
		) )
		{
			// This quote might spawn another quote from someone
			FOR_EACH_IN_TEAM(s, OUR_TEAM)
			{
				if (s->ubProfile != ubCharacterNum &&
					OkControllableMerc(s) &&
					SpacesAway(pSoldier->sGridNo, s->sGridNo) < 5)
				{
					// if this merc disliked the whining character sufficiently and hasn't
					// already retorted
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
		}*/
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

	ST::string gzQuoteStr;
	if (!GetDialogue(MercProfile(ubCharacterNum), usQuoteNum, gzQuoteStr,
		zSoundString, useAlternateDialogueFile))
	{
		return( FALSE );
	}

	SetFaceTalking(*face, zSoundString, gzQuoteStr);
	CreateTalkingUI(bUIHandlerID, *face, ubCharacterNum, gzQuoteStr);

	// Set global handleer ID value, used when face desides it's done...
	gbUIHandlerID = bUIHandlerID;

	guiScreenIDUsedWhenUICreated = guiCurrentScreen;

	return( TRUE );
}


static void DisplayTextForExternalNPC(UINT8 ubCharacterNum, const ST::string& zQuoteStr);
static void HandleExternNPCSpeechFace(FACETYPE&);
static void HandleTacticalNPCTextUI(UINT8 ubCharacterNum, const ST::string& zQuoteStr);
static void HandleTacticalTextUI(ProfileID profile_id, const ST::string& zQuoteStr);


static void CreateTalkingUI(DialogueHandler bUIHandlerID, FACETYPE& f, UINT8 ubCharacterNum, const ST::string& zQuoteStr)
{
	// Show text, if on
	if (gGameSettings.fOptions[TOPTION_SUBTITLES] || !f.fValidSpeech)
	{
		switch (bUIHandlerID)
		{
			case DIALOGUE_TACTICAL_UI:
				HandleTacticalTextUI(ubCharacterNum, zQuoteStr);
				break;
			case DIALOGUE_NPC_UI:
				HandleTacticalNPCTextUI(ubCharacterNum, zQuoteStr);
				break;
			case DIALOGUE_CONTACTPAGE_UI:
				DisplayTextForMercFaceVideoPopUp(zQuoteStr);
				break;
			case DIALOGUE_SPECK_CONTACT_PAGE_UI:
				DisplayTextForSpeckVideoPopUp(zQuoteStr);
				break;
			case DIALOGUE_EXTERNAL_NPC_UI:
				DisplayTextForExternalNPC(ubCharacterNum, zQuoteStr);
				break;
			case DIALOGUE_SHOPKEEPER_UI:
				InitShopKeeperSubTitledText(zQuoteStr);
				break;
			default:
				break;
		}
	}

	if (gGameSettings.fOptions[TOPTION_SPEECH])
	{
		switch (bUIHandlerID)
		{
			case DIALOGUE_TACTICAL_UI:
				HandleTacticalSpeechUI(ubCharacterNum, f);
				break;
			case DIALOGUE_EXTERNAL_NPC_UI:
				HandleExternNPCSpeechFace(f);
				break;
			//case DIALOGUE_CONTACTPAGE_UI:
			//case DIALOGUE_SPECK_CONTACT_PAGE_UI:
			default:
				break;
		}
	}
}

static BOOLEAN GetDialogue(const MercProfile &profile, UINT16 usQuoteNum, ST::string& zDialogueText, CHAR8* zSoundString, bool useAlternateDialogueFile)
{
	// first things first  - gDIALOGUESIZErab the text (if player has SUBTITLE PREFERENCE ON)
	//if ( gGameSettings.fOptions[ TOPTION_SUBTITLES ] )
	{
		ST::string zFilename = Content::GetDialogueTextFilename(
						profile,
						useAlternateDialogueFile,
						ProfileCurrentlyTalkingInDialoguePanel(profile.getID()));

		bool success = false;
		try
		{
			ST::string* quote = GCM->loadDialogQuoteFromFile(zFilename.c_str(), usQuoteNum);
			if(quote)
			{
				zDialogueText = *quote;
				delete quote;
				success = !zDialogueText.empty();
			}
		}
		catch (...) { success = false; }
		if (!success)
		{
			zDialogueText = ST::format("I have no text in the EDT file ({}) {}", usQuoteNum, zFilename);
			return( FALSE );
		}
	}

	// CHECK IF THE FILE EXISTS, IF NOT, USE DEFAULT!
	ST::string zFilename = Content::GetDialogueVoiceFilename(
		profile, usQuoteNum, useAlternateDialogueFile,
		ProfileCurrentlyTalkingInDialoguePanel(profile.getID()),
		isRussianVersion() || isRussianGoldVersion());

	strcpy(zSoundString, zFilename.c_str());
	return(TRUE);
}


// Handlers for tactical UI stuff
static void HandleTacticalNPCTextUI(UINT8 ubCharacterNum, const ST::string& zQuoteStr)
{
	// Setup dialogue text box
	if ( guiCurrentScreen != MAP_SCREEN )
	{
		gTalkPanel.fRenderSubTitlesNow = TRUE;
		gTalkPanel.fSetupSubTitles = TRUE;
	}

	// post message to mapscreen message system
	gTalkPanel.zQuoteStr = ST::format("\"{}\"", zQuoteStr);
	MapScreenMessage(FONT_MCOLOR_WHITE, MSG_DIALOG, ST::format("{}: \"{}\"", GetProfile(ubCharacterNum).zNickname, zQuoteStr));
}


static void ExecuteTacticalTextBox(INT16 sLeftPosition, INT16 sTopPosition, const ST::string& pString);


// Handlers for tactical UI stuff
static void DisplayTextForExternalNPC(UINT8 ubCharacterNum, const ST::string& zQuoteStr)
{
	INT16 sLeft;
	INT16 sTop;

	// Setup dialogue text box
	if ( guiCurrentScreen != MAP_SCREEN )
	{
		gTalkPanel.fRenderSubTitlesNow = TRUE;
		gTalkPanel.fSetupSubTitles = TRUE;
	}

	// post message to mapscreen message system
	gTalkPanel.zQuoteStr = ST::format("\"{}\"", zQuoteStr);
	MapScreenMessage(FONT_MCOLOR_WHITE, MSG_DIALOG, ST::format("{}: \"{}\"",
				GetProfile(ubCharacterNum).zNickname, zQuoteStr));

	if ( guiCurrentScreen == MAP_SCREEN )
	{
		// on the map screen dialog can be in any place requested
		sLeft = gsExternPanelXPosition + 97;
		sTop = gsExternPanelYPosition;
	}
	else
	{
		// on the tactical screen show message always in the same position (corner
		// of the screen)
		sLeft = 110;
		sTop = 20;
	}

	ExecuteTacticalTextBox( sLeft, sTop, gTalkPanel.zQuoteStr );
}


static void HandleTacticalTextUI(ProfileID profile_id, const ST::string& zQuoteStr)
{
	ST::string zText = ST::format("\"{}\"", zQuoteStr);

	ExecuteTacticalTextBox( g_ui.getTacticalTextBoxX(), g_ui.getTacticalTextBoxY(), zText );

	MapScreenMessage(FONT_MCOLOR_WHITE, MSG_DIALOG, ST::format("{}: \"{}\"", GetProfile(profile_id).zNickname, zQuoteStr));
}


static void RenderSubtitleBoxOverlay(VIDEO_OVERLAY* pBlitter);
static void TextOverlayClickCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void ExecuteTacticalTextBox(INT16 sLeftPosition, INT16 sTopPosition, const ST::string& pString)
{
	// check if mouse region created, if so, do not recreate
	if (fTextBoxMouseRegionCreated) return;

	// Prepare text box
	g_dialogue_box = PrepareMercPopupBox(g_dialogue_box, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, pString, DIALOGUE_DEFAULT_SUBTITLE_WIDTH, 0, 0, 0, &gusSubtitleBoxWidth, &gusSubtitleBoxHeight);

	INT16  const x = sLeftPosition;
	INT16  const y = sTopPosition;
	UINT16 const w = gusSubtitleBoxWidth;
	UINT16 const h = gusSubtitleBoxHeight;

	g_text_box_overlay = RegisterVideoOverlay(RenderSubtitleBoxOverlay, x, y, w, h);

	//Define main region
	MSYS_DefineRegion(&gTextBoxMouseRegion, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL, MSYS_NO_CALLBACK, TextOverlayClickCallback);

	fTextBoxMouseRegionCreated = TRUE;
}


static void FaceOverlayClickCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void RenderFaceOverlay(VIDEO_OVERLAY* pBlitter);


static void HandleExternNPCSpeechFace(FACETYPE& f)
{
	// Enable it!
	SetAutoFaceActive(FACE_AUTO_DISPLAY_BUFFER, FACE_AUTO_RESTORE_BUFFER, f, 0, 0);

	// Set flag to say WE control when to set inactive!
	f.uiFlags |= FACE_INACTIVE_HANDLED_ELSEWHERE;

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

	RenderAutoFace(f);

	// ATE: Create mouse region.......
	if ( !fExternFaceBoxRegionCreated )
	{
		fExternFaceBoxRegionCreated = TRUE;

		//Define main region
		MSYS_DefineRegion(&gFacePopupMouseRegion, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL, MSYS_NO_CALLBACK, FaceOverlayClickCallback);
	}

	gfFacePanelActive = TRUE;
}


static void HandleTacticalSpeechUI(const UINT8 ubCharacterNum, FACETYPE& f)
{
	BOOLEAN fDoExternPanel = FALSE;

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
		SetAutoFaceActive(FACE_AUTO_DISPLAY_BUFFER, FACE_AUTO_RESTORE_BUFFER, f, 0, 0);

		// Set flag to say WE control when to set inactive!
		f.uiFlags |= FACE_INACTIVE_HANDLED_ELSEWHERE | FACE_MAKEACTIVE_ONCE_DONE;

		// IF we are in tactical and this soldier is on the current squad
		if ((guiCurrentScreen == GAME_SCREEN) && (pSoldier != NULL) &&
			(pSoldier->bAssignment == iCurrentTacticalSquad))
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

		RenderAutoFace(f);

		// ATE: Create mouse region.......
		if ( !fExternFaceBoxRegionCreated )
		{
			fExternFaceBoxRegionCreated = TRUE;

			//Define main region
			MSYS_DefineRegion(&gFacePopupMouseRegion, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST,
						CURSOR_NORMAL, MSYS_NO_CALLBACK, FaceOverlayClickCallback);
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


void HandleDialogueEnd(FACETYPE& f)
{
	if ( gGameSettings.fOptions[ TOPTION_SPEECH ] )
	{

		if (&f != gpCurrentTalkingFace)
		{
			//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, "HandleDialogueEnd() face mismatch." );
			return;
		}

		if (f.fTalking)
		{
			SLOGD("HandleDialogueEnd() face still talking." );
			return;
		}

		switch( gbUIHandlerID )
		{
			case DIALOGUE_TACTICAL_UI:
				if ( gfFacePanelActive )
				{
					// Set face inactive!
					f.fCanHandleInactiveNow = TRUE;
					SetAutoFaceInActive(f);
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
				f.fCanHandleInactiveNow = TRUE;
				SetAutoFaceInActive(f);
				gfFacePanelActive = FALSE;

				if ( fExternFaceBoxRegionCreated )
				{
					fExternFaceBoxRegionCreated = FALSE;
					MSYS_RemoveRegion(&(gFacePopupMouseRegion) );
				}
				break;

			default:
				break;
		}
	}


	if (gGameSettings.fOptions[TOPTION_SUBTITLES] || !f.fValidSpeech)
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

			default:
				break;
		}
	}

	TurnOffSectorLocator();

	gsExternPanelXPosition = DEFAULT_EXTERN_PANEL_X_POS;
	gsExternPanelYPosition = DEFAULT_EXTERN_PANEL_Y_POS;

}


static void RenderFaceOverlay(VIDEO_OVERLAY* const blt)
{
	INT16 sFontX;
	INT16 sFontY;

	if (!gfFacePanelActive) return;

	if (!gpCurrentTalkingFace) return;
	FACETYPE    const&       f   = *gpCurrentTalkingFace;
	SOLDIERTYPE const* const s   = FindSoldierByProfileID(f.ubCharacterNum);
	INT16              const x   = blt->sX;
	INT16              const y   = blt->sY;
	SGPVSurface*       const dst = blt->uiDestBuff;

	// a living soldier?..or external NPC?..choose panel based on this
	SGPVObject const* const vo = s ? guiCOMPANEL : guiCOMPANELB;
	BltVideoObject(dst, vo, 0, x, y);

	// Display name, location ( if not current )
	SetFontAttributes(BLOCKFONT2, FONT_MCOLOR_LTGRAY);

	if (s)
	{
		SetFontDestBuffer(dst);

		FindFontCenterCoordinates(x + 12, y + 55, 73, 9, s->name, BLOCKFONT2, &sFontX, &sFontY);
		MPrint(sFontX, sFontY, s->name);

		// What sector are we in, (and is it the same as ours?)
		if (s->sSectorX != gWorldSectorX || s->sSectorY != gWorldSectorY ||
			s->bSectorZ != gbWorldSectorZ || s->fBetweenSectors)
		{
			ST::string sector_id = GetSectorIDString(s->sSectorX, s->sSectorY, s->bSectorZ, FALSE);
			sector_id = ReduceStringLength(sector_id, 64, BLOCKFONT2);
			FindFontCenterCoordinates(x + 12, y + 68, 73, 9, sector_id, BLOCKFONT2, &sFontX, &sFontY);
			MPrint(sFontX, sFontY, sector_id);
		}

		SetFontDestBuffer(FRAME_BUFFER);

		DrawSoldierUIBars(*s, x + 69, y + 47, FALSE, dst);
	}
	else
	{
		ST::string name = GetProfile(f.ubCharacterNum).zNickname;
		FindFontCenterCoordinates(x + 9, y + 55, 73, 9, name, BLOCKFONT2, &sFontX, &sFontY);
		MPrint(sFontX, sFontY, name);
	}

	SGPBox const r = { 0, 0, f.usFaceWidth, f.usFaceHeight };
	BltVideoSurface(dst, f.uiAutoDisplayBuffer, x + 14, y + 6, &r);

	InvalidateRegion(x, y, x + 99, y + 98);
}


static void RenderSubtitleBoxOverlay(VIDEO_OVERLAY* pBlitter)
{
	if (g_text_box_overlay == NULL) return;

	RenderMercPopUpBox(g_dialogue_box, pBlitter->sX, pBlitter->sY, pBlitter->uiDestBuff);
	InvalidateRegion(pBlitter->sX, pBlitter->sY, pBlitter->sX + gusSubtitleBoxWidth, pBlitter->sY + gusSubtitleBoxHeight);
}


// Let Red talk, if he is in the list and the quote is QUOTE_AIR_RAID.  Choose
// somebody else otherwise
static void ChooseRedIfPresentAndAirRaid(SOLDIERTYPE*const*const mercs_in_sector, UINT32 merc_count, UINT16 quote)
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


void SayQuoteFromAnyBodyInSector(UINT16 const quote_id)
{
	// Loop through all our guys and randomly say one from someone in our sector
	INT32       n_mercs = 0;
	SOLDIERTYPE* mercs_in_sector[20];
	FOR_EACH_IN_TEAM(i, OUR_TEAM)
	{
		// Add guy if he's a candidate
		SOLDIERTYPE& s = *i;
		if (!OkControllableMerc(&s))
			continue;
		if (AM_AN_EPC(&s))
			continue;
		if (s.uiStatusFlags & SOLDIER_GASSED)
			continue;
		if (AM_A_ROBOT(&s))
			continue;
		if (s.fMercAsleep)
			continue;

		if (gTacticalStatus.bNumFoughtInBattle[ENEMY_TEAM] == 0)
		{
			// Skip quotes referring to Deidranna's men, if there were no army guys
			// fought
			switch (quote_id)
			{
				case QUOTE_SECTOR_SAFE:
					switch (s.ubProfile)
					{
						case IRA:    continue;
						case MIGUEL: continue;
						case SHANK:  continue;
					}
					break;

				case QUOTE_ENEMY_PRESENCE:
					switch (s.ubProfile)
					{
						case DIMITRI: continue;
						case DYNAMO:  continue;
						case IRA:     continue;
						case SHANK:   continue;
					}
					break;
			}
		}

		mercs_in_sector[n_mercs++] = &s;
	}

	ChooseRedIfPresentAndAirRaid(mercs_in_sector, n_mercs, quote_id);
}


void SayQuoteFromAnyBodyInThisSector(INT16 const x, INT16 const y, INT8 const z, UINT16 const quote_id)
{
	// Loop through all our guys and randomly say one from someone in our sector
	INT32       n_mercs = 0;
	SOLDIERTYPE* mercs_in_sector[20];
	FOR_EACH_IN_TEAM(i, OUR_TEAM)
	{
		// Add guy if he's a candidate
		SOLDIERTYPE& s = *i;
		if (s.sSectorX != x)
			continue;
		if (s.sSectorY != y)
			continue;
		if (s.bSectorZ != z)
			continue;
		if (AM_AN_EPC(&s))
			continue;
		if (s.uiStatusFlags & SOLDIER_GASSED)
			continue;
		if (AM_A_ROBOT(&s))
			continue;
		if (s.fMercAsleep)
			continue;
		mercs_in_sector[n_mercs++] = &s;
	}

	ChooseRedIfPresentAndAirRaid(mercs_in_sector, n_mercs, quote_id);
}


void SayQuoteFromNearbyMercInSector(GridNo const gridno, INT8 const distance, UINT16 const quote_id)
{
	// Loop through all our guys and randomly say one from someone in our sector
	INT32       n_mercs = 0;
	SOLDIERTYPE* mercs_in_sector[20];
	FOR_EACH_IN_TEAM(i, OUR_TEAM)
	{
		// Add guy if he's a candidate
		SOLDIERTYPE& s = *i;
		if (!OkControllableMerc(&s))
			continue;
		if (PythSpacesAway(gridno, s.sGridNo) >= distance)
			continue;
		if (AM_AN_EPC(&s))
			continue;
		if (s.uiStatusFlags & SOLDIER_GASSED)
			continue;
		if (AM_A_ROBOT(&s))
			continue;
		if (s.fMercAsleep)
			continue;
		if (!SoldierTo3DLocationLineOfSightTest(&s, gridno, 0, 0, MaxDistanceVisible(), TRUE))
			continue;
		if (quote_id == QUOTE_STUFF_MISSING_DRASSEN && static_cast<INT8>(Random(100)) > EffectiveWisdom(&s))
			continue;
		mercs_in_sector[n_mercs++] = &s;
	}

	if (n_mercs > 0)
	{
		SOLDIERTYPE* const chosen = mercs_in_sector[Random(n_mercs)];
		if (quote_id == QUOTE_STUFF_MISSING_DRASSEN)
		{
			SetFactTrue(FACT_PLAYER_FOUND_ITEMS_MISSING);
		}
		TacticalCharacterDialogue(chosen, quote_id);
	}
}


void SayQuote58FromNearbyMercInSector(GridNo const gridno, INT8 const distance, UINT16 const quote_id, INT8 const sex)
{
	// Loop through all our guys and randomly say one from someone in our sector
	INT32       n_mercs = 0;
	SOLDIERTYPE* mercs_in_sector[20];
	FOR_EACH_IN_TEAM(i, OUR_TEAM)
	{
		// Add guy if he's a candidate
		SOLDIERTYPE& s = *i;
		if (!OkControllableMerc(&s))
			continue;
		if (PythSpacesAway(gridno, s.sGridNo) >= distance)
			continue;
		if (AM_AN_EPC(&s))
			continue;
		if (s.uiStatusFlags & SOLDIER_GASSED)
			continue;
		if (AM_A_ROBOT(&s))
			continue;
		if (s.fMercAsleep)
			continue;
		if (!SoldierTo3DLocationLineOfSightTest(&s, gridno, 0, 0, MaxDistanceVisible(), TRUE))
			continue;

		// ATE: This is to check gedner for this quote
		switch (QuoteExp_GenderCode[s.ubProfile])
		{
			case 0:
				if (sex == FEMALE)
					continue;
				break;
			case 1:
				if (sex == MALE)
					continue;
				break;
		}

		mercs_in_sector[n_mercs++] = &s;
	}

	if (n_mercs > 0)
	{
		SOLDIERTYPE* const chosen = mercs_in_sector[Random(n_mercs)];
		TacticalCharacterDialogue(chosen, quote_id);
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


UINT32 FindDelayForString(const ST::string& str)
{
	return( (UINT32)str.to_utf32().size() * TEXT_DELAY_MODIFIER );
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
	FOR_EACH(UINT16 const, i, gusStopTimeQuoteList)
	{
		if (*i == quote_id)
			return true;
	}
	return false;
}


static void CheckForStopTimeQuotes(UINT16 const usQuoteNum)
{
	if (!IsStopTimeQuote(usQuoteNum)) return;
	// Stop Time, game
	EnterModalTactical(TACTICAL_MODAL_NOMOUSE);
	gpCurrentTalkingFace->uiFlags |= FACE_MODAL;
	SLOGD("Starting Modal Tactical Quote.");
}


void SetStopTimeQuoteCallback( MODAL_HOOK pCallBack )
{
	gModalDoneCallback = pCallBack;
}


bool IsMercSayingDialogue(ProfileID const pid)
{
	return gpCurrentTalkingFace && gubCurrentTalkingID == pid;
}


BOOLEAN GetMercPrecedentQuoteBitStatus(const MERCPROFILESTRUCT* const p, UINT8 const ubQuoteBit)
{
	return (p->uiPrecedentQuoteSaid & 1 << (ubQuoteBit - 1)) != 0;
}


void SetMercPrecedentQuoteBitStatus(MERCPROFILESTRUCT* const p, UINT8 const ubBitToSet)
{
	p->uiPrecedentQuoteSaid |= 1 << (ubBitToSet - 1);
}


UINT8 GetQuoteBitNumberFromQuoteID(UINT32 const uiQuoteID)
{
	for (size_t i = 0; i != lengthof(gubMercValidPrecedentQuoteID); ++i)
	{
		if (gubMercValidPrecedentQuoteID[i] == uiQuoteID)
			return static_cast<UINT8>(i);
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
	gsExternPanelXPosition = sXPos;
	gsExternPanelYPosition = sYPos;
}


void LoadDialogueControlGraphics()
{
	guiCOMPANEL  = AddVideoObjectFromFile(INTERFACEDIR "/communicationpopup.sti");
	guiCOMPANELB = AddVideoObjectFromFile(INTERFACEDIR "/communicationpopup_2.sti");
}


void DeleteDialogueControlGraphics()
{
	DeleteVideoObject(guiCOMPANEL);
	DeleteVideoObject(guiCOMPANELB);
}


#ifdef WITH_UNITTESTS
#undef FAIL
#include "gtest/gtest.h"

TEST(DialogueControl, asserts)
{
	EXPECT_EQ(lengthof(preloadedExternalNPCFaces), 6);
}

#endif
