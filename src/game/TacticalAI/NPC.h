#ifndef NPC_H
#define NPC_H

#include "Facts.h"
#include "NPCQuoteInfo.h"
#include "Types.h"
struct OBJECTTYPE;
struct SOLDIERTYPE;

#define NPC_TALK_RADIUS 4


enum Approach
{
	APPROACH_NONE = 0,

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
	APPROACH_DONE_SLAPPED, // 20

	APPROACH_DONE_PUNCH_0,
	APPROACH_DONE_PUNCH_1,
	APPROACH_DONE_PUNCH_2,
	APPROACH_DONE_OPEN_STRUCTURE,
	APPROACH_DONE_GET_ITEM, // 25

	APPROACH_DONE_GIVING_ITEM,
	APPROACH_DONE_TRAVERSAL,
	APPROACH_BUYSELL,
	APPROACH_ONE_OF_FOUR_STANDARD,
	APPROACH_FRIENDLY_DIRECT_OR_RECRUIT, // 30
};

extern INT8 const gbFirstApproachFlags[4];

void ShutdownNPCQuotes();

extern void SetQuoteRecordAsUsed( UINT8 ubNPC, UINT8 ubRecord );

// uiApproachData is used for approach things like giving items, etc.
UINT8 CalcDesireToTalk(UINT8 ubNPC, UINT8 ubMerc, Approach);
void  ConverseFull(UINT8 ubNPC, UINT8 ubMerc, Approach, UINT8 approach_record, OBJECTTYPE* approach_object);
void  Converse(UINT8 ubNPC, UINT8 ubMerc, Approach);

extern void NPCReachedDestination( SOLDIERTYPE * pNPC, BOOLEAN fAlreadyThere );
extern void PCsNearNPC( UINT8 ubNPC );
extern BOOLEAN PCDoesFirstAidOnNPC( UINT8 ubNPC );
extern void TriggerNPCRecord( UINT8 ubTriggerNPC, UINT8 ubTriggerNPCRec );
extern BOOLEAN TriggerNPCWithIHateYouQuote( UINT8 ubTriggerNPC );

extern void TriggerNPCRecordImmediately( UINT8 ubTriggerNPC, UINT8 ubTriggerNPCRec );

BOOLEAN TriggerNPCWithGivenApproach(UINT8 ubTriggerNPC, Approach);


std::unique_ptr<NPCQuoteInfo []> ExtractNPCQuoteInfoArrayFromFile(HWFILE const f);
bool ReloadQuoteFile(UINT8 ubNPC);
void ReloadAllQuoteFiles(void);

// Save and loads the npc info to a saved game file
void SaveNPCInfoToSaveGameFile(HWFILE);
void LoadNPCInfoFromSavedGameFile(HWFILE, UINT32 uiSaveGameVersion);

extern void TriggerFriendWithHostileQuote( UINT8 ubNPC );

extern void ReplaceLocationInNPCDataFromProfileID( UINT8 ubNPC, INT16 sOldGridNo, INT16 sNewGridNo );

extern UINT8 ActionIDForMovementRecord( UINT8 ubNPC, UINT8 ubRecord );

// given a victory in this sector, handle specific facts
void HandleVictoryInNPCSector(const SGPSector& sector);

// check if this shopkeep has been shutdown, if so do soething and return the fact
BOOLEAN HandleShopKeepHasBeenShutDown( UINT8 ubCharNum );

BOOLEAN NPCHasUnusedRecordWithGivenApproach(UINT8 ubNPC, Approach);
BOOLEAN NPCWillingToAcceptItem( UINT8 ubNPC, UINT8 ubMerc, OBJECTTYPE * pObj );

void SaveBackupNPCInfoToSaveGameFile(HWFILE);
void LoadBackupNPCInfoFromSavedGameFile(HWFILE);
void UpdateDarrelScriptToGoTo( SOLDIERTYPE * pSoldier );

#define WALTER_BRIBE_AMOUNT 20000

BOOLEAN GetInfoForAbandoningEPC(UINT8 ubNPC, UINT16* pusQuoteNum, Fact* fact_to_set_true);

BOOLEAN RecordHasDialogue( UINT8 ubNPC, UINT8 ubRecord );

INT8 ConsiderCivilianQuotes(const SGPSector& sector, BOOLEAN fSetAsUsed);

void ResetOncePerConvoRecordsForNPC( UINT8 ubNPC );

void HandleNPCChangesForTacticalTraversal(const SOLDIERTYPE* s);

BOOLEAN NPCHasUnusedHostileRecord(UINT8 ubNPC, Approach);

void ResetOncePerConvoRecordsForAllNPCsInLoadedSector( void );

INT16 NPCConsiderInitiatingConv(const SOLDIERTYPE* pNPC);

#endif
