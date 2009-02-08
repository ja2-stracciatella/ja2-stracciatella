#ifndef NPC_H
#define NPC_H

#include "Facts.h"
#include "MapScreen.h"


#define NUM_NPC_QUOTE_RECORDS 50

//#define IRRELEVANT 255
//#define NO_QUEST 255
//#define NO_FACT 255
//#define NO_QUOTE 255
#define MUST_BE_NEW_DAY 254
#define INITIATING_FACTOR 30

#define TURN_FLAG_ON( a, b ) (a |= b)
#define TURN_FLAG_OFF( a, b ) (a &= ~(b))
#define CHECK_FLAG( a, b) (a & b)

#define QUOTE_FLAG_SAID								0x0001
#define QUOTE_FLAG_ERASE_ONCE_SAID		0x0002
#define QUOTE_FLAG_SAY_ONCE_PER_CONVO	0x0004

#define	NPC_TALK_RADIUS						4

#define TURN_UI_OFF 65000
#define TURN_UI_ON  65001
#define SPECIAL_TURN_UI_OFF 65002
#define SPECIAL_TURN_UI_ON 65003

#define LARGE_AMOUNT_MONEY 1000

#define ACCEPT_ANY_ITEM 1000
#define ANY_RIFLE 1001

struct NPCQuoteInfo
{
#if defined RUSSIAN
	UINT8		ubIdentifier[4];
#endif

	UINT16	fFlags;

	// conditions
	union
	{
		INT16		sRequiredItem;			// item NPC must have to say quote
		INT16		sRequiredGridno;		// location for NPC req'd to say quote
	};
	UINT16	usFactMustBeTrue;		// ...before saying quote
	UINT16 	usFactMustBeFalse;	// ...before saying quote
	UINT8		ubQuest;						// quest must be current to say quote
	UINT8		ubFirstDay;					// first day quote can be said
	UINT8		ubLastDay;					// last day quote can be said
	UINT8		ubApproachRequired;	// must use this approach to generate quote
	UINT8		ubOpinionRequired;	// opinion needed for this quote     13 bytes

	// quote to say (if any)
	UINT8		ubQuoteNum;					// this is the quote to say
	UINT8		ubNumQuotes;				// total # of quotes to say          15 bytes

	// actions
	UINT8		ubStartQuest;
	UINT8		ubEndQuest;
	UINT8		ubTriggerNPC;
	UINT8		ubTriggerNPCRec;
	UINT8		ubFiller;				//                                       20 bytes
	UINT16	usSetFactTrue;
	UINT16	usGiftItem;			// item NPC gives to merc after saying quote
	UINT16	usGoToGridno;
	INT16		sActionData;		// special action value

#if !defined RUSSIAN
	UINT8		ubUnused[4];
#endif
};																									// 32 bytes
CASSERT(sizeof(NPCQuoteInfo) == 32)


enum Approaches
{
	APPROACH_FRIENDLY = 1,
	APPROACH_DIRECT,
	APPROACH_THREATEN,
	APPROACH_RECRUIT,
	APPROACH_REPEAT,

	APPROACH_GIVINGITEM,
	NPC_INITIATING_CONV,
	NPC_INITIAL_QUOTE,
	NPC_WHOAREYOU,
	TRIGGER_NPC,

	APPROACH_GIVEFIRSTAID,
	APPROACH_SPECIAL_INITIAL_QUOTE,
	APPROACH_ENEMY_NPC_QUOTE,
	APPROACH_DECLARATION_OF_HOSTILITY,
	APPROACH_EPC_IN_WRONG_SECTOR,

	APPROACH_EPC_WHO_IS_RECRUITED,
	APPROACH_INITIAL_QUOTE,
	APPROACH_CLOSING_SHOP,
	APPROACH_SECTOR_NOT_SAFE,
	APPROACH_DONE_SLAPPED,	// 20

	APPROACH_DONE_PUNCH_0,
	APPROACH_DONE_PUNCH_1,
	APPROACH_DONE_PUNCH_2,
	APPROACH_DONE_OPEN_STRUCTURE,
	APPROACH_DONE_GET_ITEM,					// 25

	APPROACH_DONE_GIVING_ITEM,
	APPROACH_DONE_TRAVERSAL,
	APPROACH_BUYSELL,
	APPROACH_ONE_OF_FOUR_STANDARD,
	APPROACH_FRIENDLY_DIRECT_OR_RECRUIT,	// 30
};

enum StandardQuoteIDs
{
	QUOTE_INTRO = 0,
	QUOTE_SUBS_INTRO,
	QUOTE_FRIENDLY_DEFAULT1,
	QUOTE_FRIENDLY_DEFAULT2,
	QUOTE_GIVEITEM_NO,
	QUOTE_DIRECT_DEFAULT,
	QUOTE_THREATEN_DEFAULT,
	QUOTE_RECRUIT_NO,
	QUOTE_BYE,
	QUOTE_GETLOST
};

#define NUM_REAL_APPROACHES APPROACH_RECRUIT

extern INT8	gbFirstApproachFlags[4];

extern void ShutdownNPCQuotes( void );

extern void SetQuoteRecordAsUsed( UINT8 ubNPC, UINT8 ubRecord );

// uiApproachData is used for approach things like giving items, etc.
extern UINT8 CalcDesireToTalk( UINT8 ubNPC, UINT8 ubMerc, INT8 bApproach );
extern void Converse( UINT8 ubNPC, UINT8 ubMerc, INT8 bApproach, UINT32 uiApproachData );

extern void NPCReachedDestination( SOLDIERTYPE * pNPC, BOOLEAN fAlreadyThere );
extern void PCsNearNPC( UINT8 ubNPC );
extern BOOLEAN PCDoesFirstAidOnNPC( UINT8 ubNPC );
extern void TriggerNPCRecord( UINT8 ubTriggerNPC, UINT8 ubTriggerNPCRec );
extern BOOLEAN TriggerNPCWithIHateYouQuote( UINT8 ubTriggerNPC );

extern void TriggerNPCRecordImmediately( UINT8 ubTriggerNPC, UINT8 ubTriggerNPCRec );

extern BOOLEAN TriggerNPCWithGivenApproach( UINT8 ubTriggerNPC, UINT8 ubApproach, BOOLEAN fShowPanel );


extern BOOLEAN ReloadQuoteFile( UINT8 ubNPC );
void ReloadAllQuoteFiles(void);

// Save and loads the npc info to a saved game file
void SaveNPCInfoToSaveGameFile(HWFILE);
void LoadNPCInfoFromSavedGameFile(HWFILE, UINT32 uiSaveGameVersion);

extern void TriggerFriendWithHostileQuote( UINT8 ubNPC );

extern void ReplaceLocationInNPCDataFromProfileID( UINT8 ubNPC, INT16 sOldGridNo, INT16 sNewGridNo );

extern UINT8 ActionIDForMovementRecord( UINT8 ubNPC, UINT8 ubRecord );

// total amount given to doctors
extern UINT32 uiTotalAmountGivenToDoctors;

// given a victory in this sector, handle specific facts
void HandleVictoryInNPCSector( INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ );

// check if this shopkeep has been shutdown, if so do soething and return the fact
BOOLEAN HandleShopKeepHasBeenShutDown( UINT8 ubCharNum );

BOOLEAN NPCHasUnusedRecordWithGivenApproach( UINT8 ubNPC, UINT8 ubApproach );
BOOLEAN NPCWillingToAcceptItem( UINT8 ubNPC, UINT8 ubMerc, OBJECTTYPE * pObj );

void SaveBackupNPCInfoToSaveGameFile(HWFILE);
void LoadBackupNPCInfoFromSavedGameFile(HWFILE, UINT32 uiSaveGameVersion);

#ifdef JA2BETAVERSION
void ToggleNPCRecordDisplay( void );
#endif

void UpdateDarrelScriptToGoTo( SOLDIERTYPE * pSoldier );

#define WALTER_BRIBE_AMOUNT 20000

BOOLEAN GetInfoForAbandoningEPC(UINT8 ubNPC, UINT16* pusQuoteNum, Fact* fact_to_set_true);

BOOLEAN RecordHasDialogue( UINT8 ubNPC, UINT8 ubRecord );

INT8 ConsiderCivilianQuotes( INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ,  BOOLEAN fSetAsUsed );

void ResetOncePerConvoRecordsForNPC( UINT8 ubNPC );

void HandleNPCChangesForTacticalTraversal(const SOLDIERTYPE* s);

BOOLEAN NPCHasUnusedHostileRecord( UINT8 ubNPC, UINT8 ubApproach );

void ResetOncePerConvoRecordsForAllNPCsInLoadedSector( void );

extern NPCQuoteInfo* gpNPCQuoteInfoArray[];

INT16 NPCConsiderInitiatingConv(const SOLDIERTYPE* pNPC);

#endif
