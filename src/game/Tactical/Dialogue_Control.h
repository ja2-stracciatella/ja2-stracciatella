#ifndef _DIALOG_CONTROL_H
#define _DIALOG_CONTROL_H

#include "GameScreen.h"
#include "JA2Types.h"
#include <string_theory/string>


// An enumeration for dialog quotes
enum DialogQuoteIDs
{
	QUOTE_NONE = 0xFFFF,

	// 0
	QUOTE_SEE_ENEMY = 0,
	QUOTE_SEE_ENEMY_VARIATION,
	QUOTE_IN_TROUBLE_SLASH_IN_BATTLE,
	QUOTE_SEE_CREATURE,
	QUOTE_FIRSTTIME_GAME_SEE_CREATURE,
	QUOTE_TRACES_OF_CREATURE_ATTACK,
	QUOTE_HEARD_SOMETHING,
	QUOTE_SMELLED_CREATURE,
	QUOTE_WEARY_SLASH_SUSPUCIOUS,
	QUOTE_WORRIED_ABOUT_CREATURE_PRESENCE,

	//10
	QUOTE_ATTACKED_BY_MULTIPLE_CREATURES,
	QUOTE_SPOTTED_SOMETHING_ONE,
	QUOTE_SPOTTED_SOMETHING_TWO,
	QUOTE_OUT_OF_AMMO,
	QUOTE_SERIOUSLY_WOUNDED,
	QUOTE_BUDDY_ONE_KILLED,
	QUOTE_BUDDY_TWO_KILLED,
	QUOTE_LEARNED_TO_LIKE_MERC_KILLED,
	QUOTE_FORGETFULL_SLASH_CONFUSED,
	QUOTE_JAMMED_GUN,

	//20
	QUOTE_UNDER_HEAVY_FIRE,
	QUOTE_TAKEN_A_BREATING,
	QUOTE_CLOSE_CALL,
	QUOTE_NO_LINE_OF_FIRE,
	QUOTE_STARTING_TO_BLEED,
	QUOTE_NEED_SLEEP,
	QUOTE_OUT_OF_BREATH,
	QUOTE_KILLED_AN_ENEMY,
	QUOTE_KILLED_A_CREATURE,
	QUOTE_HATED_MERC_ONE,

	//30
	QUOTE_HATED_MERC_TWO,
	QUOTE_LEARNED_TO_HATE_MERC,
	QUOTE_AIM_KILLED_MIKE,
	QUOTE_MERC_QUIT_LEARN_TO_HATE = QUOTE_AIM_KILLED_MIKE,
	QUOTE_HEADSHOT,
	QUOTE_PERSONALITY_TRAIT,
	QUOTE_ASSIGNMENT_COMPLETE,
	QUOTE_REFUSING_ORDER,
	QUOTE_KILLING_DEIDRANNA,
	QUOTE_KILLING_QUEEN,
	QUOTE_ANNOYING_PC,

	//40
	QUOTE_STARTING_TO_WHINE,
	QUOTE_NEGATIVE_COMPANY,
	QUOTE_AIR_RAID,
	QUOTE_WHINE_EQUIPMENT,
	QUOTE_SOCIAL_TRAIT,
	QUOTE_PASSING_DISLIKE,
	QUOTE_EXPERIENCE_GAIN,
	QUOTE_PRE_NOT_SMART,
	QUOTE_POST_NOT_SMART,
	QUOTE_HATED_1_ARRIVES,
	QUOTE_MERC_QUIT_HATED1 = QUOTE_HATED_1_ARRIVES,

	//50
	QUOTE_HATED_2_ARRIVES,
	QUOTE_MERC_QUIT_HATED2 = QUOTE_HATED_2_ARRIVES,
	QUOTE_BUDDY_1_GOOD,
	QUOTE_BUDDY_2_GOOD,
	QUOTE_LEARNED_TO_LIKE_WITNESSED,
	QUOTE_DELAY_CONTRACT_RENEWAL,
	QUOTE_NOT_GETTING_PAID = QUOTE_DELAY_CONTRACT_RENEWAL,
	QUOTE_AIM_SEEN_MIKE,
	QUOTE_PC_DROPPED_OMERTA = QUOTE_AIM_SEEN_MIKE,
	QUOTE_BLINDED,
	QUOTE_DEFINITE_CANT_DO,
	QUOTE_LISTEN_LIKABLE_PERSON,
	QUOTE_ENEMY_PRESENCE,

	//60
	QUOTE_WARNING_OUTSTANDING_ENEMY_AFTER_RT,
	QUOTE_FOUND_SOMETHING_SPECIAL,
	QUOTE_SATISFACTION_WITH_GUN_AFTER_KILL,
	QUOTE_SPOTTED_JOEY,
	QUOTE_RESPONSE_TO_MIGUEL_SLASH_QUOTE_MERC_OR_RPC_LETGO,
	QUOTE_SECTOR_SAFE,
	QUOTE_STUFF_MISSING_DRASSEN,
	QUOTE_KILLED_FACTORY_MANAGER,
	QUOTE_SPOTTED_BLOODCAT,
	QUOTE_END_GAME_COMMENT,

	//70
	QUOTE_ENEMY_RETREATED,
	QUOTE_GOING_TO_AUTO_SLEEP,
	QUOTE_WORK_UP_AND_RETURNING_TO_ASSIGNMENT, // woke up from auto sleep, going back to wo
	QUOTE_ME_TOO, // me too quote, in agreement with whatever the merc previous said
	QUOTE_USELESS_ITEM,
	QUOTE_BOOBYTRAP_ITEM,
	QUOTE_SUSPICIOUS_GROUND,
	QUOTE_DROWNING,
	QUOTE_MERC_REACHED_DESTINATION,
	QUOTE_SPARE2,

	//80
	QUOTE_REPUTATION_REFUSAL,
	QUOTE_DEATH_RATE_REFUSAL, //= 99,
	QUOTE_LAME_REFUSAL, //= 82,
	QUOTE_WONT_RENEW_CONTRACT_LAME_REFUSAL, // ARM: now unused
	QUOTE_ANSWERING_MACHINE_MSG,
	QUOTE_DEPARTING_COMMENT_CONTRACT_NOT_RENEWED_OR_48_OR_MORE,
	QUOTE_HATE_MERC_1_ON_TEAM,// = 100,
	QUOTE_HATE_MERC_2_ON_TEAM,// = 101,
	QUOTE_LEARNED_TO_HATE_MERC_ON_TEAM,// = 102,
	QUOTE_CONTRACTS_OVER,// = 89,

	//90
	QUOTE_ACCEPT_CONTRACT_RENEWAL,
	QUOTE_CONTRACT_ACCEPTANCE,
	QUOTE_JOINING_CAUSE_BUDDY_1_ON_TEAM,// = 103,
	QUOTE_JOINING_CAUSE_BUDDY_2_ON_TEAM,// = 104,
	QUOTE_JOINING_CAUSE_LEARNED_TO_LIKE_BUDDY_ON_TEAM,// = 105,
	QUOTE_REFUSAL_RENEW_DUE_TO_MORALE,// = 95,
	QUOTE_PRECEDENT_TO_REPEATING_ONESELF,// = 106,
	QUOTE_REFUSAL_TO_JOIN_LACK_OF_FUNDS,// = 107,
	QUOTE_DEPART_COMMET_CONTRACT_NOT_RENEWED_OR_TERMINATED_UNDER_48,// = 98,
	QUOTE_DEATH_RATE_RENEWAL,

	//100
	QUOTE_HATE_MERC_1_ON_TEAM_WONT_RENEW,
	QUOTE_HATE_MERC_2_ON_TEAM_WONT_RENEW,
	QUOTE_LEARNED_TO_HATE_MERC_1_ON_TEAM_WONT_RENEW,
	QUOTE_RENEWING_CAUSE_BUDDY_1_ON_TEAM,
	QUOTE_RENEWING_CAUSE_BUDDY_2_ON_TEAM,
	QUOTE_RENEWING_CAUSE_LEARNED_TO_LIKE_BUDDY_ON_TEAM,
	QUOTE_PRECEDENT_TO_REPEATING_ONESELF_RENEW,
	QUOTE_RENEW_REFUSAL_DUE_TO_LACK_OF_FUNDS,
	QUOTE_GREETING,
	QUOTE_SMALL_TALK,

	//110
	QUOTE_IMPATIENT_QUOTE,
	QUOTE_LENGTH_OF_CONTRACT,
	QUOTE_COMMENT_BEFORE_HANG_UP,
	QUOTE_PERSONALITY_BIAS_WITH_MERC_1,
	QUOTE_PERSONALITY_BIAS_WITH_MERC_2,
	QUOTE_MERC_LEAVING_ALSUCO_SOON,
	QUOTE_MERC_GONE_UP_IN_PRICE
};


enum DialogueHandler
{
	DIALOGUE_NO_UI                 = 0,
	DIALOGUE_TACTICAL_UI           = 1,
	DIALOGUE_CONTACTPAGE_UI        = 2,
	DIALOGUE_NPC_UI                = 3,
	DIALOGUE_SPECK_CONTACT_PAGE_UI = 4,
	DIALOGUE_EXTERNAL_NPC_UI       = 5,
	DIALOGUE_SHOPKEEPER_UI         = 6
};


// Gets a external NPC dialogue face by merc profile ID.
// "External" because the NPC is not on our team and might not even exist as a soldier
FACETYPE* GetExternalNPCFace(ProfileID);

// Functions for handling dialogue Q
void InitalizeDialogueControl(void);
void ShutdownDialogueControl(void);
void EmptyDialogueQueue(void);
void HandleDialogue(void);
void HandleImportantMercQuote( SOLDIERTYPE * pSoldier, UINT16 usQuoteNumber );
void HandleImportantMercQuoteLocked(SOLDIERTYPE*, UINT16 quote);

// A higher level function used for tactical quotes
BOOLEAN TacticalCharacterDialogue(const SOLDIERTYPE* pSoldier, UINT16 usQuoteNum);

// A higher level function used for tactical quotes
BOOLEAN DelayedTacticalCharacterDialogue( SOLDIERTYPE *pSoldier, UINT16 usQuoteNum );

// A more general purpose function for processing quotes
void CharacterDialogue(UINT8 ubCharacterNum, UINT16 usQuoteNum, FACETYPE* face, DialogueHandler, BOOLEAN fFromSoldier, bool delayed = false);

void CharacterDialogueUsingAlternateFile(SOLDIERTYPE&, UINT16 quote, DialogueHandler);

BOOLEAN ExecuteCharacterDialogue(UINT8 ubCharacterNum, UINT16 usQuoteNum, FACETYPE* face, DialogueHandler, BOOLEAN fFromSoldier, bool useAlternateDialogueFile);

// Called when a face stops talking...
void HandleDialogueEnd(FACETYPE&);

// Called to advance speech
// Used for option when no speech sound file
void DialogueAdvanceSpeech(void);

BOOLEAN DialogueQueueIsEmpty(void);
BOOLEAN DialogueQueueIsEmptyAndNobodyIsTalking(void);


// set up and shutdown static external NPC faces
void PreloadExternalNPCFaces();
void LoadExternalNPCFace(ProfileID);
void UnloadExternalNPCFaces();

void SayQuoteFromAnyBodyInSector(UINT16 quote_id);
void SayQuoteFromNearbyMercInSector(GridNo, INT8 distance, UINT16 quote_id);
void SayQuote58FromNearbyMercInSector(GridNo, INT8 distance, UINT16 quote_id, INT8 sex);
UINT32 FindDelayForString(const ST::string& str);
void BeginLoggingForBleedMeToos( BOOLEAN fStart );

void UnSetEngagedInConvFromPCAction( SOLDIERTYPE *pSoldier );
void SetEngagedInConvFromPCAction( SOLDIERTYPE *pSoldier );


void SetStopTimeQuoteCallback( MODAL_HOOK pCallBack );

BOOLEAN DialogueActive(void);

extern INT32 giNPCReferenceCount;


BOOLEAN GetMercPrecedentQuoteBitStatus(const MERCPROFILESTRUCT*, UINT8 ubQuoteBit);
void    SetMercPrecedentQuoteBitStatus(MERCPROFILESTRUCT*, UINT8 ubBitToSet);
UINT8	GetQuoteBitNumberFromQuoteID( UINT32 uiQuoteID );
void HandleShutDownOfMapScreenWhileExternfaceIsTalking( void );

void StopAnyCurrentlyTalkingSpeech(void);

// handle pausing of the dialogue queue
void PauseDialogueQueue( void );

// unpause the dialogue queue
void UnPauseDialogueQueue( void );

void SetExternMapscreenSpeechPanelXY( INT16 sXPos, INT16 sYPos );

bool IsMercSayingDialogue(ProfileID);

extern FACETYPE* gpCurrentTalkingFace;

extern MercPopUpBox* g_dialogue_box;

extern BOOLEAN gfFacePanelActive;


class DialogueEvent
{
	public:
		virtual ~DialogueEvent() {}

		virtual bool Execute() = 0;

		static void Add(DialogueEvent*);
};

template<void (&callback)()> class DialogueEventCallback : public DialogueEvent
{
	public:
		bool Execute()
		{
			callback();
			return false;
		}
};

class CharacterDialogueEvent : public DialogueEvent
{
	public:
		CharacterDialogueEvent(SOLDIERTYPE& soldier) : soldier_(soldier) {}

		bool MayExecute() const;

	protected:
		SOLDIERTYPE& soldier_;
};

void MakeCharacterDialogueEventSleep(SOLDIERTYPE&, bool sleep);

void LoadDialogueControlGraphics();
void DeleteDialogueControlGraphics();

#endif
