#include "Buffer.h"
#include "Directories.h"
#include "Font_Control.h"
#include "LoadSaveData.h"
#include "Types.h"
#include "Overhead.h"
#include "AI.h"
#include "Soldier_Profile.h"
#include "Soldier_Control.h"
#include "NPC.h"
#include "Isometric_Utils.h"
#include "Quests.h"
#include "Interface_Dialogue.h"
#include "Game_Clock.h"
#include "FileMan.h"
#include "Random.h"
#include "Items.h"
#include "Interface.h"
#include "Assignments.h"
#include "Soldier_Macros.h"
#include "Dialogue_Control.h"
#include "Strategic_Town_Loyalty.h"
#include "Message.h"
#include "Timer_Control.h"
#include "Soldier_Add.h"
#include "Soldier_Tile.h"
#include "Weapons.h"
#include "Meanwhile.h"
#include "QuestText.h"
#include "SkillCheck.h"
#include "Render_Fun.h"
#include "StrategicMap.h"
#include "Text.h"
#include "Arms_Dealer_Init.h"
#include "Interface_Items.h"
#include "OppList.h"
#include "Animation_Control.h"
#include "Scheduling.h"
#include "Tactical_Save.h"
#include "Campaign_Types.h"
#include "MemMan.h"
#include "Debug.h"
#include "GameRes.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "WeaponModels.h"
#include "slog/slog.h"

#define NUM_NPC_QUOTE_RECORDS  50
#define NUM_CIVQUOTE_SECTORS   20
#define MINERS_CIV_QUOTE_INDEX 16

static const INT16 gsCivQuoteSector[NUM_CIVQUOTE_SECTORS][2] =
{
	{  2, MAP_ROW_A },
	{  2, MAP_ROW_B },
	{ 13, MAP_ROW_B },
	{ 13, MAP_ROW_C },
	{ 13, MAP_ROW_D },
	{  8, MAP_ROW_F },
	{  9, MAP_ROW_F },
	{  8, MAP_ROW_G },
	{  9, MAP_ROW_G },
	{  1, MAP_ROW_H },

	{  2, MAP_ROW_H },
	{  3, MAP_ROW_H },
	{  8, MAP_ROW_H },
	{ 13, MAP_ROW_H },
	{ 14, MAP_ROW_I },
	{ 11, MAP_ROW_L },
	{ 12, MAP_ROW_L },
	{  0, 0         }, // THIS ONE USED NOW - FOR bSectorZ > 0.....
	{  0, 0         },
	{  0, 0         },
};

#define NO_FACT                 (MAX_FACTS - 1)
#define NO_QUEST                255
#define QUEST_NOT_STARTED_NUM   100
#define QUEST_DONE_NUM          200
#define NO_QUOTE                255
#define IRRELEVANT              255
#define MUST_BE_NEW_DAY         254
#define NO_MOVE                 65535
#define INITIATING_FACTOR       30

#define TURN_FLAG_ON(a, b)  ((a) |= (b))
#define TURN_FLAG_OFF(a, b) ((a) &= ~(b))
#define CHECK_FLAG(a, b)    ((a) & (b))

#define QUOTE_FLAG_SAID               0x0001
#define QUOTE_FLAG_ERASE_ONCE_SAID    0x0002
#define QUOTE_FLAG_SAY_ONCE_PER_CONVO 0x0004

#define TURN_UI_OFF         65000
#define TURN_UI_ON          65001
#define SPECIAL_TURN_UI_OFF 65002
#define SPECIAL_TURN_UI_ON  65003

#define LARGE_AMOUNT_MONEY 1000

#define ACCEPT_ANY_ITEM 1000
#define ANY_RIFLE       1001

#define NUM_REAL_APPROACHES APPROACH_RECRUIT


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


struct NPCQuoteInfo
{
	UINT32  ubIdentifier;

	UINT16  fFlags;

	// conditions
	union
	{
		INT16 sRequiredItem;      // item NPC must have to say quote
		INT16 sRequiredGridno;    // location for NPC req'd to say quote
	};
	UINT16  usFactMustBeTrue;   // ...before saying quote
	UINT16  usFactMustBeFalse;  // ...before saying quote
	UINT8   ubQuest;            // quest must be current to say quote
	UINT8   ubFirstDay;         // first day quote can be said
	UINT8   ubLastDay;          // last day quote can be said
	UINT8   ubApproachRequired; // must use this approach to generate quote
	UINT8   ubOpinionRequired;  // opinion needed for this quote

	// quote to say (if any)
	UINT8   ubQuoteNum;         // this is the quote to say
	UINT8   ubNumQuotes;        // total # of quotes to say

	// actions
	UINT8   ubStartQuest;
	UINT8   ubEndQuest;
	UINT8   ubTriggerNPC;
	UINT8   ubTriggerNPCRec;
	UINT16  usSetFactTrue;
	UINT16  usGiftItem;         // item NPC gives to merc after saying quote
	UINT16  usGoToGridno;
	INT16   sActionData;        // special action value
};

static NPCQuoteInfo* gpNPCQuoteInfoArray[NUM_PROFILES];
static NPCQuoteInfo* gpBackupNPCQuoteInfoArray[NUM_PROFILES];
static NPCQuoteInfo* gpCivQuoteInfoArray[NUM_CIVQUOTE_SECTORS];

// Warning: cheap hack approaching
static BOOLEAN gfTriedToLoadQuoteInfoArray[NUM_PROFILES];

INT8 const gbFirstApproachFlags[] = { 0x01, 0x02, 0x04, 0x08 };


static UINT8 const gubAlternateNPCFileNumsForQueenMeanwhiles[]  = { 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176 };
static UINT8 const gubAlternateNPCFileNumsForElliotMeanwhiles[] = { 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196 };

static NPCQuoteInfo* ExtractNPCQuoteInfoArrayFromFile(HWFILE const f)
{
	SGP::Buffer<NPCQuoteInfo> buf(NUM_NPC_QUOTE_RECORDS);
	for (NPCQuoteInfo* i = buf; i != buf + NUM_NPC_QUOTE_RECORDS; ++i)
	{
		BYTE data[32];
		FileRead(f, data, sizeof(data));

		BYTE const* d = data;
		if(isRussianVersion())
		{
			EXTR_U32( d, i->ubIdentifier);
		}
		else
		{
			i->ubIdentifier = 0;
		}
		EXTR_U16( d, i->fFlags)
		EXTR_I16( d, i->sRequiredItem)
		EXTR_U16( d, i->usFactMustBeTrue)
		EXTR_U16( d, i->usFactMustBeFalse)
		EXTR_U8(  d, i->ubQuest)
		EXTR_U8(  d, i->ubFirstDay)
		EXTR_U8(  d, i->ubLastDay)
		EXTR_U8(  d, i->ubApproachRequired)
		EXTR_U8(  d, i->ubOpinionRequired)
		EXTR_U8(  d, i->ubQuoteNum)
		EXTR_U8(  d, i->ubNumQuotes)
		EXTR_U8(  d, i->ubStartQuest)
		EXTR_U8(  d, i->ubEndQuest)
		EXTR_U8(  d, i->ubTriggerNPC)
		EXTR_U8(  d, i->ubTriggerNPCRec)
		EXTR_SKIP(d, 1)
		EXTR_U16( d, i->usSetFactTrue)
		EXTR_U16( d, i->usGiftItem)
		EXTR_U16( d, i->usGoToGridno)
		EXTR_I16( d, i->sActionData)
		if(!isRussianVersion())
		{
			EXTR_SKIP(d, 4);
		}
		Assert(d == endof(data));
	}
	return buf.Release();
}


static void ConditionalExtractNPCQuoteInfoArrayFromFile(HWFILE const f, NPCQuoteInfo*& q)
{
	UINT8 present;
	FileRead(f, &present, sizeof(present));
	FreeNull(q);
	if (!present) return;
	q = ExtractNPCQuoteInfoArrayFromFile(f);
}


static void ConditionalInjectNPCQuoteInfoArrayIntoFile(HWFILE const f, NPCQuoteInfo const* const q)
{
	if (!q)
	{
		static UINT8 const zero = 0;
		FileWrite(f, &zero, sizeof(zero));
		return;
	}

	static UINT8 const one = 1;
	FileWrite(f, &one, sizeof(one));

	for (NPCQuoteInfo const* i = q; i != q + NUM_NPC_QUOTE_RECORDS; ++i)
	{
		BYTE  data[32];
		BYTE* d = data;
		if(isRussianVersion())
		{
			INJ_U32( d, i->ubIdentifier);
		}
		INJ_U16( d, i->fFlags)
		INJ_I16( d, i->sRequiredItem)
		INJ_U16( d, i->usFactMustBeTrue)
		INJ_U16( d, i->usFactMustBeFalse)
		INJ_U8(  d, i->ubQuest)
		INJ_U8(  d, i->ubFirstDay)
		INJ_U8(  d, i->ubLastDay)
		INJ_U8(  d, i->ubApproachRequired)
		INJ_U8(  d, i->ubOpinionRequired)
		INJ_U8(  d, i->ubQuoteNum)
		INJ_U8(  d, i->ubNumQuotes)
		INJ_U8(  d, i->ubStartQuest)
		INJ_U8(  d, i->ubEndQuest)
		INJ_U8(  d, i->ubTriggerNPC)
		INJ_U8(  d, i->ubTriggerNPCRec)
		INJ_SKIP(d, 1)
		INJ_U16( d, i->usSetFactTrue)
		INJ_U16( d, i->usGiftItem)
		INJ_U16( d, i->usGoToGridno)
		INJ_I16( d, i->sActionData)
		if(!isRussianVersion())
		{
			INJ_SKIP(d, 4);
		}
		Assert(d == endof(data));
		FileWrite(f, data, sizeof(data));
	}
}


//
// NPC QUOTE LOW LEVEL ROUTINES
//


static NPCQuoteInfo* LoadQuoteFile(UINT8 ubNPC)
try
{
	CHAR8						zFileName[255];

	if ( ubNPC == PETER || ubNPC == ALBERTO || ubNPC == CARLO )
	{
		// use a copy of Herve's data file instead!
		sprintf(zFileName, NPCDATADIR "/%03d.npc", HERVE);
	}
	else if ( ubNPC < FIRST_RPC || (ubNPC < FIRST_NPC && gMercProfiles[ ubNPC ].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED ) )
	{
		sprintf(zFileName, NPCDATADIR "/000.npc");
	}
	else
	{
		sprintf(zFileName, NPCDATADIR "/%03d.npc", ubNPC);
	}

	// ATE: Put some stuff i here to use a different NPC file if we are in a meanwhile.....
	if ( AreInMeanwhile( ) )
	{
		// If we are the queen....
		if ( ubNPC == QUEEN )
		{
			sprintf(zFileName, NPCDATADIR "/%03d.npc", gubAlternateNPCFileNumsForQueenMeanwhiles[GetMeanwhileID()]);
		}

		// If we are elliot....
		if ( ubNPC == ELLIOT )
		{
			sprintf(zFileName, NPCDATADIR "/%03d.npc", gubAlternateNPCFileNumsForElliotMeanwhiles[GetMeanwhileID()]);
		}

	}

	AutoSGPFile f(GCM->openGameResForReading(zFileName));
	return ExtractNPCQuoteInfoArrayFromFile(f);
}
catch (...) { return 0; }


static void RevertToOriginalQuoteFile(UINT8 ubNPC)
{
	if ( gpBackupNPCQuoteInfoArray[ ubNPC ] && gpNPCQuoteInfoArray[ubNPC] )
	{
		MemFree( gpNPCQuoteInfoArray[ubNPC] );
		gpNPCQuoteInfoArray[ubNPC] = gpBackupNPCQuoteInfoArray[ubNPC];
		gpBackupNPCQuoteInfoArray[ubNPC] = NULL;
	}
}


static void BackupOriginalQuoteFile(UINT8 ubNPC)
{
	gpBackupNPCQuoteInfoArray[ ubNPC ] = gpNPCQuoteInfoArray[ ubNPC ];
	gpNPCQuoteInfoArray[ubNPC] = NULL;
}


static NPCQuoteInfo* EnsureQuoteFileLoaded(UINT8 const ubNPC)
{
	if (ubNPC == ROBOT) return 0;

	NPCQuoteInfo*& q         = gpNPCQuoteInfoArray[ubNPC];
	bool           load_file = !q;

	if (FIRST_RPC <= ubNPC && ubNPC < FIRST_NPC)
	{
		if (GetProfile(ubNPC).ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED)
		{ // recruited
			if (!gpBackupNPCQuoteInfoArray[ubNPC])
			{
				// no backup stored of current script, so need to backup
				load_file = true;
				// set pointer to back up script!
				BackupOriginalQuoteFile(ubNPC);
			}
			// else have backup, are recruited, nothing special
		}
		else
		{ // not recruited
			if (gpBackupNPCQuoteInfoArray[ubNPC])
			{
				// backup stored, restore backup
				RevertToOriginalQuoteFile(ubNPC);
			}
			// else are no backup, nothing special
		}
	}

	if (load_file)
	{
		q = LoadQuoteFile(ubNPC);
		if (!q)
		{
			if (!gfTriedToLoadQuoteInfoArray[ubNPC]) // don't report the error a second time
			{
				SLOGE(DEBUG_TAG_NPC, "NPC needs NPC file: %d.", ubNPC );
				gfTriedToLoadQuoteInfoArray[ubNPC] = TRUE;
			}
			// error message at this point!
		}
	}

	return q;
}


bool ReloadQuoteFile(UINT8 const ubNPC)
{
	FreeNull(gpNPCQuoteInfoArray[ubNPC]);
	FreeNull(gpBackupNPCQuoteInfoArray[ubNPC]);
	return EnsureQuoteFileLoaded(ubNPC);
}


static bool ReloadQuoteFileIfLoaded(UINT8 const ubNPC)
{
	NPCQuoteInfo*& q = gpNPCQuoteInfoArray[ubNPC];
	if (!q) return TRUE;
	FreeNull(q);
	return EnsureQuoteFileLoaded(ubNPC);
}


static void RefreshNPCScriptRecord(UINT8 const ubNPC, UINT8 const record)
{
	if (ubNPC == NO_PROFILE)
	{
		// loop through all PCs, and refresh their copy of this record
		for (UINT8 i = 0; i != FIRST_RPC; ++i)
		{
			RefreshNPCScriptRecord(i, record);
		}
		for (UINT8 i = FIRST_RPC; i != FIRST_NPC; ++i)
		{
			if (!(GetProfile(ubNPC).ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED)) continue;
			if (!gpBackupNPCQuoteInfoArray[ubNPC]) continue;
			RefreshNPCScriptRecord(i, record);
		}
		return;
	}

	NPCQuoteInfo* const quotes = gpNPCQuoteInfoArray[ubNPC];
	if (!quotes) return;

	NPCQuoteInfo& q = quotes[record];
	// already used? so we don't have to refresh!
	if (CHECK_FLAG(q.fFlags, QUOTE_FLAG_SAID)) return;

	SGP::Buffer<NPCQuoteInfo> new_quotes(LoadQuoteFile(ubNPC));
	if (!new_quotes) return;
	q = new_quotes[record];
}


//
// CIV QUOTE LOW LEVEL ROUTINES
//


static NPCQuoteInfo* LoadCivQuoteFile(UINT8 const idx)
{
	char const* filename;
	char        buf[255];
	if (idx == MINERS_CIV_QUOTE_INDEX)
	{
		filename = NPCDATADIR "/miners.npc";
	}
	else
	{
		sprintf(buf, NPCDATADIR "/%c%d.npc", 'A' + gsCivQuoteSector[idx][1] - 1, gsCivQuoteSector[idx][0]);
		filename = buf;
	}
	AutoSGPFile f(GCM->openGameResForReading(filename));
	return ExtractNPCQuoteInfoArrayFromFile(f);
}


static NPCQuoteInfo* EnsureCivQuoteFileLoaded(UINT8 const idx)
try
{
	NPCQuoteInfo*& q = gpCivQuoteInfoArray[idx];
	if (!q) q = LoadCivQuoteFile(idx);
	return q;
}
catch (...) { return 0; }


static bool ReloadCivQuoteFileIfLoaded(UINT8 const idx)
try
{
	NPCQuoteInfo*& q = gpCivQuoteInfoArray[idx];
	if (!q) return true;
	FreeNull(q);
	q = LoadCivQuoteFile(idx);
	return true;
}
catch (...) { return false; }


void ShutdownNPCQuotes()
{
	FOR_EACH(NPCQuoteInfo*, i, gpNPCQuoteInfoArray)       FreeNull(*i);
	FOR_EACH(NPCQuoteInfo*, i, gpBackupNPCQuoteInfoArray) FreeNull(*i);
	FOR_EACH(NPCQuoteInfo*, i, gpCivQuoteInfoArray)       FreeNull(*i);
}


//
// GENERAL LOW LEVEL ROUTINES
//

void ReloadAllQuoteFiles(void)
{
	UINT8		ubProfile, ubLoop;

	for ( ubProfile = FIRST_RPC; ubProfile < NUM_PROFILES; ubProfile++ )
	{
		// zap backup if any
		FreeNull(gpBackupNPCQuoteInfoArray[ubProfile]);
		ReloadQuoteFileIfLoaded( ubProfile );
	}
	// reload all civ quote files
	for ( ubLoop = 0; ubLoop < NUM_CIVQUOTE_SECTORS; ubLoop++ )
	{
		ReloadCivQuoteFileIfLoaded( ubLoop );
	}
}


//
// THE REST
//


void SetQuoteRecordAsUsed( UINT8 ubNPC, UINT8 ubRecord )
{
	NPCQuoteInfo* const quotes = EnsureQuoteFileLoaded(ubNPC);
	if (!quotes) return;
	quotes[ubRecord].fFlags |= QUOTE_FLAG_SAID;
}


static INT32 CalcThreateningEffectiveness(UINT8 const ubMerc)
{
	// effective threat is 1/3 strength, 1/3 weapon deadliness, 1/3 leadership

	SOLDIERTYPE* const s = FindSoldierByProfileIDOnPlayerTeam(ubMerc);
	if (!s) return 0;

	UINT16 const item_idx = s->inv[HANDPOS].usItem;
	INT32 deadliness =
		GCM->getItem(item_idx)->isWeapon() ? GCM->getWeapon(item_idx)->ubDeadliness :
		0;

	if (deadliness == 0) deadliness = -30; // penalize!

	INT32 const strength = EffectiveStrength(s);
	return (EffectiveLeadership(s) + strength + deadliness) / 2;
}


UINT8 CalcDesireToTalk(UINT8 const ubNPC, UINT8 const ubMerc, Approach const bApproach)
{
	INT32 iWillingness;
	INT32	iPersonalVal, iTownVal, iApproachVal;
	INT32 iEffectiveLeadership;

	MERCPROFILESTRUCT const& pNPCProfile  = GetProfile(ubNPC);
	MERCPROFILESTRUCT const& pMercProfile = GetProfile(ubMerc);

	iPersonalVal = 50 + pNPCProfile.bMercOpinion[ubMerc];

	// ARM: NOTE - for towns which don't use loyalty (San Mona, Estoni, Tixa, Orta )
	// loyalty will always remain 0 (this was OKed by Ian)
	iTownVal = gTownLoyalty[pNPCProfile.bTown].ubRating;
	iTownVal = iTownVal * pNPCProfile.bTownAttachment / 100;

	if (bApproach == NPC_INITIATING_CONV || bApproach == APPROACH_GIVINGITEM )
	{
		iApproachVal = 100;
	}
	else if ( bApproach == APPROACH_THREATEN )
	{
		iEffectiveLeadership = CalcThreateningEffectiveness(ubMerc) * pMercProfile.usApproachFactor[bApproach - 1] / 100;
		iApproachVal = pNPCProfile.ubApproachVal[bApproach - 1] * iEffectiveLeadership / 50;
	}
	else
	{
		iEffectiveLeadership = (INT32)pMercProfile.bLeadership * pMercProfile.usApproachFactor[bApproach - 1] / 100;
		iApproachVal = pNPCProfile.ubApproachVal[bApproach - 1] * iEffectiveLeadership / 50;
	}
	// NB if town attachment is less than 100% then we should make personal value proportionately more important!
	if (pNPCProfile.bTownAttachment < 100)
	{
		iPersonalVal = iPersonalVal * (100 + (100 - pNPCProfile.bTownAttachment) ) / 100;
	}
	iWillingness = (iPersonalVal / 2 + iTownVal / 2) * iApproachVal	/ 100;

	if (bApproach == NPC_INITIATING_CONV)
	{
		iWillingness -= INITIATING_FACTOR;
	}

	if (iWillingness < 0)
	{
		iWillingness = 0;
	}

	return( (UINT8) iWillingness );
}


static void ApproachedForFirstTime(MERCPROFILESTRUCT& pNPCProfile, Approach const bApproach)
{
	UINT8		ubLoop;
	UINT32	uiTemp;

	pNPCProfile.bApproached |= gbFirstApproachFlags[bApproach - 1];
	for (ubLoop = 1; ubLoop <= NUM_REAL_APPROACHES; ubLoop++)
	{
		uiTemp = (UINT32) pNPCProfile.ubApproachVal[ubLoop - 1] * (UINT32) pNPCProfile.ubApproachMod[bApproach - 1][ubLoop - 1] / 100;
		if (uiTemp > 255)
		{
			uiTemp = 255;
		}
		pNPCProfile.ubApproachVal[ubLoop-1] = (UINT8) uiTemp;
	}
}


static UINT8 NPCConsiderQuote(UINT8 ubNPC, UINT8 ubMerc, Approach, UINT8 ubQuoteNum, UINT8 ubTalkDesire, NPCQuoteInfo* pNPCQuoteInfoArray);


static UINT8 NPCConsiderTalking(UINT8 const ubNPC, UINT8 const ubMerc, Approach const approach, UINT8 const record, NPCQuoteInfo* const pNPCQuoteInfoArray, NPCQuoteInfo** const ppResultQuoteInfo, UINT8* const pubQuoteNum)
{
	// This function returns the opinion level required of the "most difficult" quote
	// that the NPC is willing to say to the merc.  It can also provide the quote #.
	SOLDIERTYPE const* const s = FindSoldierByProfileID(ubNPC);
	if (!s) return 0;

	if (ppResultQuoteInfo) *ppResultQuoteInfo = 0;
	if (pubQuoteNum)       *pubQuoteNum       = 0;

	UINT8 talk_desire = 0;
	if (approach <= NUM_REAL_APPROACHES)
	{
		MERCPROFILESTRUCT& p = GetProfile(ubNPC);
		// What's our willingness to divulge?
		talk_desire = CalcDesireToTalk(ubNPC, ubMerc, approach);
		if (approach < NUM_REAL_APPROACHES && !(p.bApproached & gbFirstApproachFlags[approach - 1]))
		{
			ApproachedForFirstTime(p, approach);
		}
	}
	else if (ubNPC == PABLO && approach == APPROACH_SECTOR_NOT_SAFE) // for Pablo, consider as threaten
	{
		MERCPROFILESTRUCT& p = GetProfile(ubNPC);
		// What's our willingness to divulge?
		talk_desire = CalcDesireToTalk(ubNPC, ubMerc, APPROACH_THREATEN);
		if (p.bApproached & gbFirstApproachFlags[APPROACH_THREATEN - 1])
		{
			ApproachedForFirstTime(p, APPROACH_THREATEN);
		}
	}

	UINT8 first_quote_record;
	UINT8 last_quote_record;
	switch (approach)
	{
		case TRIGGER_NPC:
			first_quote_record = record;
			last_quote_record  = record;
			break;

		default:
			first_quote_record = 0;
			last_quote_record  = NUM_NPC_QUOTE_RECORDS - 1;
			break;
	}

	bool          fQuoteFound              = false;
	UINT8         ubHighestOpinionRequired = 0;
	NPCQuoteInfo* pNPCQuoteInfo            = 0;
	UINT8         ubQuote                  = 0;
	for (UINT8 i = first_quote_record; i <= last_quote_record; ++i)
	{
		pNPCQuoteInfo = &pNPCQuoteInfoArray[i];

		// Check if we have the item / are in right spot
		if (pNPCQuoteInfo->sRequiredItem > 0)
		{
			if (FindObjectInSoldierProfile(GetProfile(ubNPC), pNPCQuoteInfo->sRequiredItem) == NO_SLOT)
			{
				continue;
			}
		}
		else if (pNPCQuoteInfo->sRequiredGridno < 0)
		{
			if (s->sGridNo != -pNPCQuoteInfo->sRequiredGridno)
			{
				continue;
			}
		}

		if (!NPCConsiderQuote(ubNPC, ubMerc, approach, i, talk_desire, pNPCQuoteInfoArray)) continue;

		if (approach != NPC_INITIATING_CONV)
		{
			// we do have a quote to say, and we want to say this one right away!
			if (ppResultQuoteInfo) *ppResultQuoteInfo = pNPCQuoteInfo;
			if (pubQuoteNum)       *pubQuoteNum       = i;
			return pNPCQuoteInfo->ubOpinionRequired;
		}

		// want to find the quote with the highest required opinion rating that we're willing
		// to say
		if (pNPCQuoteInfo->ubOpinionRequired > ubHighestOpinionRequired)
		{
			fQuoteFound              = true;
			ubHighestOpinionRequired = pNPCQuoteInfo->ubOpinionRequired;
			ubQuote                  = pNPCQuoteInfo->ubQuoteNum;
		}
	}

	// Whew, checked them all.  If we found a quote, return the appropriate values.
	if (fQuoteFound)
	{
		if (ppResultQuoteInfo) *ppResultQuoteInfo = pNPCQuoteInfo;
		if (pubQuoteNum)       *pubQuoteNum       = ubQuote;
		return ubHighestOpinionRequired;
	}
	else
	{
		if (ppResultQuoteInfo) *ppResultQuoteInfo = 0;
		if (pubQuoteNum)       *pubQuoteNum       = 0;
		return 0;
	}
}


static UINT8 UseQuote(NPCQuoteInfo* const quotes, NPCQuoteInfo** const quote, UINT8* const quote_id, UINT8 const quote_to_use)
{
	*quote    = &quotes[quote_to_use];
	*quote_id = quote_to_use;
	return quotes[quote_to_use].ubOpinionRequired;
}


static UINT8 HandleNPCBeingGivenMoneyByPlayer(UINT8 ubNPC, UINT32 uiMoneyAmount);


static UINT8 NPCConsiderReceivingItemFromMerc(UINT8 const ubNPC, UINT8 const ubMerc, OBJECTTYPE const* const o, NPCQuoteInfo* const pNPCQuoteInfoArray, NPCQuoteInfo** const ppResultQuoteInfo, UINT8* const pubQuoteNum)
{
	// This function returns the opinion level required of the "most difficult" quote
	// that the NPC is willing to say to the merc.  It can also provide the quote #.
	*ppResultQuoteInfo = 0;
	*pubQuoteNum       = 0;

	// don't accept any items when we are the player's enemy
	if (CheckFact(FACT_NPC_IS_ENEMY, ubNPC) && ubNPC != JOE) return 0;

	// How much do we want to talk with this merc?
	UINT8 const ubTalkDesire = CalcDesireToTalk(ubNPC, ubMerc, APPROACH_GIVINGITEM);

	UINT16 item_to_consider = o->usItem;
	if (GCM->getItem(item_to_consider)->getItemClass() == IC_GUN && item_to_consider != ROCKET_LAUNCHER)
	{
		UINT8 const weapon_class = GCM->getWeapon(item_to_consider)->ubWeaponClass;
		if (weapon_class == RIFLECLASS || weapon_class == MGCLASS)
		{
			item_to_consider = ANY_RIFLE; // treat all rifles the same
		}
	}
	switch (item_to_consider)
	{
		case HEAD_2:
		case HEAD_3:
		//case HEAD_4: // NOT Slay's head; it's different
		case HEAD_5:
		case HEAD_6:
		case HEAD_7:
			// all treated the same in the NPC code
			item_to_consider = HEAD_2;
			break;

		case MONEY:
		case SILVER:
		case GOLD:
		{
			Fact const fact =
				o->uiMoneyAmount < LARGE_AMOUNT_MONEY ? FACT_SMALL_AMOUNT_OF_MONEY :
				FACT_LARGE_AMOUNT_OF_MONEY;
			SetFactTrue(fact);
			item_to_consider = MONEY;
			break;
		}

		case WINE:
		case BEER:
			item_to_consider = ALCOHOL;
			break;

		default:
			break;
	}

	if (o->bStatus[0] < 50)
	{
		SetFactTrue(FACT_ITEM_POOR_CONDITION);
	}
	else
	{
		SetFactFalse(FACT_ITEM_POOR_CONDITION);
	}

	for (UINT8 i = 0; i != NUM_NPC_QUOTE_RECORDS; ++i)
	{
		NPCQuoteInfo& q = pNPCQuoteInfoArray[i];

		// First see if we want that item....
		INT16 const req_item = q.sRequiredItem;
		if (req_item <= 0) continue;
		if (req_item != item_to_consider && req_item != ACCEPT_ANY_ITEM) continue;

		// Now see if everyhting else is OK
		if (!NPCConsiderQuote(ubNPC, ubMerc, APPROACH_GIVINGITEM, i, ubTalkDesire, pNPCQuoteInfoArray)) continue;

		switch (ubNPC)
		{
			case DARREN:
				if (item_to_consider == MONEY && q.sActionData == NPC_ACTION_DARREN_GIVEN_CASH)
				{
					if (o->uiMoneyAmount < 1000)
					{
						// refuse, bet too low - record 15
						return UseQuote(pNPCQuoteInfoArray, ppResultQuoteInfo, pubQuoteNum, 15);
					}
					else if (o->uiMoneyAmount > 5000)
					{
						// refuse, bet too high - record 16
						return UseQuote(pNPCQuoteInfoArray, ppResultQuoteInfo, pubQuoteNum, 16);
					}
					else
					{
						// record amount of bet
						gMercProfiles[DARREN].iBalance = o->uiMoneyAmount;
						SetFactFalse(FACT_DARREN_EXPECTING_MONEY);

						// if never fought before, use record 17
						// if fought before, today, use record 31
						// else use record 18
						if (!(gpNPCQuoteInfoArray[DARREN][17].fFlags & QUOTE_FLAG_SAID)) // record 17 not used
						{
							return UseQuote(pNPCQuoteInfoArray, ppResultQuoteInfo, pubQuoteNum, 17);
						}
						else
						{
							// find Kingpin, if he's in his house, invoke the script to move him to the bar
							UINT8                    id      = 31;
							SOLDIERTYPE const* const kingpin = FindSoldierByProfileID(KINGPIN);
							if (kingpin)
							{
								UINT8 const room = GetRoom(kingpin->sGridNo);
								// first boxer, bring kingpin over
								if (IN_KINGPIN_HOUSE(room)) id = 18;
							}

							return UseQuote(pNPCQuoteInfoArray, ppResultQuoteInfo, pubQuoteNum, id);
						}
					}
				}
				break;

			case ANGEL:
				if (item_to_consider == MONEY && q.sActionData == NPC_ACTION_ANGEL_GIVEN_CASH)
				{
					if (o->uiMoneyAmount < GCM->getItem(LEATHER_JACKET_W_KEVLAR)->getPrice())
					{ // refuse, bet too low - record 8
						return UseQuote(pNPCQuoteInfoArray, ppResultQuoteInfo, pubQuoteNum, 8);
					}
					else if (o->uiMoneyAmount > GCM->getItem(LEATHER_JACKET_W_KEVLAR)->getPrice())
					{ // refuse, bet too high - record 9
						return UseQuote(pNPCQuoteInfoArray, ppResultQuoteInfo, pubQuoteNum, 9);
					}
					else
					{ // accept - record 10
						return UseQuote(pNPCQuoteInfoArray, ppResultQuoteInfo, pubQuoteNum, 10);
					}
				}
				break;

			case MADAME:
				if (item_to_consider == MONEY)
				{
					if (GetProfile(ubMerc).bSex == FEMALE)
					{
						// say quote about not catering to women!
						return UseQuote(pNPCQuoteInfoArray, ppResultQuoteInfo, pubQuoteNum, 5);
					}

					switch (o->uiMoneyAmount)
					{
						case 100:
						case 200: // Carla
							if (!CheckFact(FACT_CARLA_AVAILABLE, 0)) goto madame_default;
							gMercProfiles[MADAME].bNPCData += (INT8)(o->uiMoneyAmount / 100);
							TriggerNPCRecord(MADAME, 16);
							break;

						case 500:
						case 1000: // Cindy
							if (!CheckFact(FACT_CINDY_AVAILABLE, 0)) goto madame_default;
							gMercProfiles[MADAME].bNPCData += (INT8)(o->uiMoneyAmount / 500);
							TriggerNPCRecord(MADAME, 17);
							break;

						case 300:
						case 600: // Bambi
							if (!CheckFact(FACT_BAMBI_AVAILABLE, 0)) goto madame_default;
							gMercProfiles[MADAME].bNPCData += (INT8)(o->uiMoneyAmount / 300);
							TriggerNPCRecord(MADAME, 18);
							break;

						case 400:
						case 800: // Maria
							if (gubQuest[QUEST_RESCUE_MARIA] != QUESTINPROGRESS) goto madame_default;
							gMercProfiles[MADAME].bNPCData += (INT8)(o->uiMoneyAmount / 400);
							TriggerNPCRecord(MADAME, 19);
							break;

						default:
madame_default:
							// play quotes 39-42 (plus 44 if quest 22 on) plus 43 if >1 PC
							// and return money
							return UseQuote(pNPCQuoteInfoArray, ppResultQuoteInfo, pubQuoteNum, 25);
					}
				}
				break;

			case JOE:
				if (item_to_consider == MONEY && q.sActionData != NPC_ACTION_JOE_GIVEN_CASH)
				{
					break;
				}
				goto check_give_money;

			case GERARD:
				if (item_to_consider == MONEY && q.sActionData != NPC_ACTION_GERARD_GIVEN_CASH)
				{
					break;
				}
				goto check_give_money;

			case STEVE:
			case VINCE:
			case WALTER:
			case FRANK:
check_give_money:
				if (item_to_consider == MONEY)
				{
					if (ubNPC == VINCE || ubNPC == STEVE)
					{
						if (!CheckFact(FACT_VINCE_EXPECTING_MONEY, ubNPC) &&
								gMercProfiles[ubNPC].iBalance < 0             &&
								q.sActionData != NPC_ACTION_DONT_ACCEPT_ITEM)
						{
							MERCPROFILESTRUCT& p = GetProfile(ubNPC);
							// increment balance
							p.iBalance          += (INT32)o->uiMoneyAmount;
							p.uiTotalCostToDate += o->uiMoneyAmount;
							if (p.iBalance > 0) p.iBalance = 0;
							ScreenMsg(FONT_YELLOW, MSG_INTERFACE, TacticalStr[BALANCE_OWED_STR], p.zNickname, -p.iBalance);
						}
						else if (!CheckFact(FACT_VINCE_EXPECTING_MONEY, ubNPC) &&
								q.sActionData != NPC_ACTION_DONT_ACCEPT_ITEM)
						{
							// just accept cash!
							if (ubNPC == VINCE)
							{
								*ppResultQuoteInfo = &pNPCQuoteInfoArray[8];
							}
							else
							{
								*ppResultQuoteInfo = &pNPCQuoteInfoArray[7];
							}
							return (*ppResultQuoteInfo)->ubOpinionRequired;
						}
						else
						{
							// handle the player giving NPC some money
							UINT8 const quote_id = HandleNPCBeingGivenMoneyByPlayer(ubNPC, o->uiMoneyAmount);
							return UseQuote(pNPCQuoteInfoArray, ppResultQuoteInfo, pubQuoteNum, quote_id);
						}
					}
					else
					{
						// handle the player giving NPC some money
						UINT8 const quote_id = HandleNPCBeingGivenMoneyByPlayer(ubNPC, o->uiMoneyAmount);
						return UseQuote(pNPCQuoteInfoArray, ppResultQuoteInfo, pubQuoteNum, quote_id);
					}
				}
				break;

			case KINGPIN:
				if (item_to_consider == MONEY && gubQuest[QUEST_KINGPIN_MONEY] == QUESTINPROGRESS)
				{
					UINT8 const quote_id = HandleNPCBeingGivenMoneyByPlayer(ubNPC, o->uiMoneyAmount);
					return UseQuote(pNPCQuoteInfoArray, ppResultQuoteInfo, pubQuoteNum, quote_id);
				}
				break;

			default:
				if (item_to_consider == MONEY && (ubNPC == SKYRIDER || (FIRST_RPC <= ubNPC && ubNPC < FIRST_NPC)))
				{
					MERCPROFILESTRUCT& p = GetProfile(ubNPC);
					if (p.iBalance < 0 && q.sActionData != NPC_ACTION_DONT_ACCEPT_ITEM)
					{
						// increment balance
						p.iBalance          += (INT32)o->uiMoneyAmount;
						p.uiTotalCostToDate += o->uiMoneyAmount;
						if (p.iBalance > 0) p.iBalance = 0;
						ScreenMsg(FONT_YELLOW, MSG_INTERFACE, TacticalStr[BALANCE_OWED_STR], p.zNickname, -p.iBalance);
					}
				}
				break;
		}
		// This is great!
		// Return desire value
		return UseQuote(pNPCQuoteInfoArray, ppResultQuoteInfo, pubQuoteNum, i);
	}

	return 0;
}


// handle money being npc being
static UINT8 HandleNPCBeingGivenMoneyByPlayer(UINT8 const ubNPC, UINT32 const uiMoneyAmount)
{
	UINT8 quote_id=0; // Wmaybe-uninitialized
	switch( ubNPC )
	{
		// handle for STEVE and VINCE
		case STEVE:
		case VINCE:
			{
				INT32		iCost;

				iCost = (INT32) CalcMedicalCost( ubNPC );

				// check amount of money
				if ( (INT32)uiMoneyAmount + giHospitalTempBalance + giHospitalRefund >= iCost )
				{
					// enough cash, check how much help is needed
					if( CheckFact( FACT_WOUNDED_MERCS_NEARBY , ubNPC) )
					{
						quote_id = 26;
					}
					else if( CheckFact( FACT_ONE_WOUNDED_MERC_NEARBY, ubNPC ) )
					{
						quote_id = 25;
					}

					if ( giHospitalRefund > 0 )
					{
						giHospitalRefund = __max( 0, giHospitalRefund - iCost + uiMoneyAmount );
					}
					giHospitalTempBalance = 0;
				}
				else
				{
					wchar_t sTempString[ 100 ];
					swprintf(sTempString, lengthof(sTempString), L"$%ld", iCost - uiMoneyAmount - giHospitalTempBalance);

					// not enough cash
					ScreenMsg( FONT_MCOLOR_LTYELLOW,
						MSG_INTERFACE,
						g_langRes->Message[ STR_NEED_TO_GIVE_MONEY ],
						gMercProfiles[ ubNPC ].zNickname,
						sTempString );
					quote_id = 27;
					giHospitalTempBalance += uiMoneyAmount;
				}
			}
			break;
		case KINGPIN:
			if ( (INT32) uiMoneyAmount < -gMercProfiles[ KINGPIN ].iBalance )
			{
				quote_id = 9;
			}
			else
			{
				quote_id = 10;
			}
			gMercProfiles[ KINGPIN ].iBalance += (INT32) uiMoneyAmount;
			break;
		case WALTER:
			if ( gMercProfiles[ WALTER ].iBalance == 0 )
			{
				quote_id = 12;
			}
			else
			{
				quote_id = 13;
			}
			gMercProfiles[ WALTER ].iBalance += uiMoneyAmount;
			break;
		case FRANK:
			gArmsDealerStatus[ ARMS_DEALER_FRANK ].uiArmsDealersCash += uiMoneyAmount;
			quote_id = 0;
			break;

		case GERARD:
			gMercProfiles[ GERARD ].iBalance += uiMoneyAmount;
			if ( (gMercProfiles[ GERARD ].iBalance) >= 10000 )
			{
				quote_id = 12;
			}
			else
			{
				quote_id = 11;
			}
			break;
		case JOE:
			gMercProfiles[ JOE ].iBalance += uiMoneyAmount;
			if ( (gMercProfiles[ JOE ].iBalance) >= 10000 )
			{
				quote_id = 7;
			}
			else
			{
				quote_id = 6;
			}
			break;

		default:
			quote_id = 0;
	}
	return quote_id;
}


static UINT8 NPCConsiderQuote(UINT8 const ubNPC, UINT8 const ubMerc, Approach const ubApproach, UINT8 const ubQuoteNum, UINT8 const ubTalkDesire, NPCQuoteInfo* const pNPCQuoteInfoArray)
{
	//This function looks at a quote and determines if conditions for it have been met.
	// Returns 0 if none , 1 if one is found
	NPCQuoteInfo *				pNPCQuoteInfo;
	UINT32								uiDay;
	BOOLEAN								fTrue;

	MERCPROFILESTRUCT const* const pNPCProfile = ubNPC != NO_PROFILE ? &GetProfile(ubNPC) : 0;

	// How much do we want to talk with this merc?
	uiDay = GetWorldDay();

	pNPCQuoteInfo = &(pNPCQuoteInfoArray[ubQuoteNum]);

	if (ubApproach != NPC_INITIATING_CONV && ubMerc != NO_PROFILE)
	{
		SLOGD(DEBUG_TAG_QUESTS, "New Approach for NPC ID: %d '%ls' against Merc: %d '%ls'\n\
														\tTesting Record #: %d",
					ubNPC, GetProfile(ubNPC).zNickname, ubMerc, GetProfile(ubMerc).zNickname, ubQuoteNum);
	}

	if (CHECK_FLAG( pNPCQuoteInfo->fFlags, QUOTE_FLAG_SAID ))
	{
		if (ubApproach != NPC_INITIATING_CONV)
		{
			SLOGD(DEBUG_TAG_QUESTS, "Quote Already Said, leaving");
		}
		// skip quotes already said
		return( FALSE );
	}

	// if the quote is quest-specific, is the player on that quest?
	if (pNPCQuoteInfo->ubQuest != NO_QUEST)
	{
		if (ubApproach != NPC_INITIATING_CONV)
		{
			SLOGD(DEBUG_TAG_QUESTS, "Quest(%d:'%ls') Must be in Progress, status is %d. %s",
						pNPCQuoteInfo->ubQuest, QuestDescText[ pNPCQuoteInfo->ubQuest ],
						gubQuest[pNPCQuoteInfo->ubQuest],
						(gubQuest[pNPCQuoteInfo->ubQuest] != QUESTINPROGRESS) ? "False, return" : "True" );
		}

		if (pNPCQuoteInfo->ubQuest > QUEST_DONE_NUM)
		{
			if (gubQuest[pNPCQuoteInfo->ubQuest - QUEST_DONE_NUM] != QUESTDONE)
			{
				return( FALSE );
			}
		}
		else if (pNPCQuoteInfo->ubQuest > QUEST_NOT_STARTED_NUM)
		{
			if (gubQuest[pNPCQuoteInfo->ubQuest - QUEST_NOT_STARTED_NUM] != QUESTNOTSTARTED)
			{
				return( FALSE );
			}
		}
		else
		{
			if (gubQuest[pNPCQuoteInfo->ubQuest] != QUESTINPROGRESS)
			{
				return( FALSE );
			}
		}
	}

	// if there are facts to be checked, check them
	if (pNPCQuoteInfo->usFactMustBeTrue != NO_FACT)
	{
		fTrue = CheckFact((Fact)pNPCQuoteInfo->usFactMustBeTrue, ubNPC);
		if (ubApproach != NPC_INITIATING_CONV)
		{
			SLOGD(DEBUG_TAG_QUESTS, "Fact (%d:'%ls') Must be True, status is %s",
						pNPCQuoteInfo->usFactMustBeTrue, FactDescText[pNPCQuoteInfo->usFactMustBeTrue],
						fTrue ? "True" : "False, returning");
		}
		if (!fTrue) return FALSE;
	}

	if (pNPCQuoteInfo->usFactMustBeFalse != NO_FACT)
	{
		fTrue = CheckFact((Fact)pNPCQuoteInfo->usFactMustBeFalse, ubNPC);
		if (ubApproach != NPC_INITIATING_CONV)
		{
			SLOGD(DEBUG_TAG_QUESTS, "Fact(%d:'%ls') Must be False status is  %s",
						pNPCQuoteInfo->usFactMustBeFalse, FactDescText[pNPCQuoteInfo->usFactMustBeFalse],
						(fTrue == TRUE) ? "True, return" : "FALSE" );
		}
		if (fTrue)	return( FALSE );
	}

	// check for required approach
	// since the "I hate you" code triggers the record, triggering has to work properly
	// with the other value that is stored!
	if ( pNPCQuoteInfo->ubApproachRequired || !(ubApproach == APPROACH_FRIENDLY || ubApproach == APPROACH_DIRECT || ubApproach == TRIGGER_NPC ) )
	{
		if (ubApproach != NPC_INITIATING_CONV)
		{
			SLOGD(DEBUG_TAG_QUESTS, "Approach Taken(%d) must equal required Approach(%d) = %s",
						ubApproach, pNPCQuoteInfo->ubApproachRequired,
						(ubApproach != pNPCQuoteInfo->ubApproachRequired) ? "TRUE, return" : "FALSE" );
		}
		if ( pNPCQuoteInfo->ubApproachRequired == APPROACH_ONE_OF_FOUR_STANDARD )
		{
			// friendly to recruit will match
			if ( ubApproach < APPROACH_FRIENDLY || ubApproach > APPROACH_RECRUIT )
			{
				return( FALSE );
			}
		}
		else if ( pNPCQuoteInfo->ubApproachRequired == APPROACH_FRIENDLY_DIRECT_OR_RECRUIT )
		{
			if ( ubApproach != APPROACH_FRIENDLY && ubApproach != APPROACH_DIRECT && ubApproach != APPROACH_RECRUIT )
			{
				return( FALSE );
			}
		}
		else if (ubApproach != pNPCQuoteInfo->ubApproachRequired)
		{
			return( FALSE );
		}
	}

	// check time constraints on the quotes
	if (pNPCProfile != NULL && pNPCQuoteInfo->ubFirstDay == MUST_BE_NEW_DAY)
	{
		if (ubApproach != NPC_INITIATING_CONV)
		{
			SLOGD(DEBUG_TAG_QUESTS, "Time constraints. Current Day(%d) must <= Day last spoken too (%d) : %s",
						uiDay, pNPCProfile->ubLastDateSpokenTo,
						(uiDay <= pNPCProfile->ubLastDateSpokenTo) ? "TRUE, return" : "FALSE" );
		}

		if (uiDay <= pNPCProfile->ubLastDateSpokenTo)
		{
			// too early!
			return( FALSE );
		}
	}
	else if (uiDay < pNPCQuoteInfo->ubFirstDay)
	{
		if (ubApproach != NPC_INITIATING_CONV)
		{
			SLOGD(DEBUG_TAG_QUESTS, "Current Day(%d) is before Required first day(%d) = %s",
						uiDay, pNPCQuoteInfo->ubFirstDay,
						(uiDay < pNPCQuoteInfo->ubFirstDay) ? "False, returning" : "True" );
		}
		// too early!
		return( FALSE );
	}

	if (uiDay > pNPCQuoteInfo->ubLastDay && uiDay < 255 )
	{
		if (ubApproach != NPC_INITIATING_CONV)
		{
			SLOGD(DEBUG_TAG_QUESTS, "Current Day(%d) is after Required first day(%d) = %s",
						uiDay, pNPCQuoteInfo->ubFirstDay,
						(uiDay > pNPCQuoteInfo->ubLastDay) ? "TRUE, returning" : "FALSE" );
		}
		// too late!
		return( FALSE );
	}

	// check opinion required
	if ((pNPCQuoteInfo->ubOpinionRequired != IRRELEVANT) && (ubApproach != TRIGGER_NPC))
	{
		if (ubApproach != NPC_INITIATING_CONV)
		{
			SLOGD(DEBUG_TAG_QUESTS, "Opinion Required.  Talk Desire (%d), Opinion Required(%d) : %s",
						ubTalkDesire, pNPCQuoteInfo->ubOpinionRequired,
						(ubTalkDesire < pNPCQuoteInfo->ubOpinionRequired) ? "False, return" : "False, continue" );
		}

		if (ubTalkDesire < pNPCQuoteInfo->ubOpinionRequired )
		{
			return( FALSE );
		}
	}

	if (ubApproach != NPC_INITIATING_CONV)
	{
		SLOGD(DEBUG_TAG_QUESTS, "Return the quote opinion value! = TRUE");
	}
	// Return the quote opinion value!
	return( TRUE );

}


static void ReplaceLocationInNPCData(NPCQuoteInfo* pNPCQuoteInfoArray, INT16 sOldGridNo, INT16 sNewGridNo)
{
	UINT8							ubFirstQuoteRecord, ubLastQuoteRecord, ubLoop;
	NPCQuoteInfo *		pNPCQuoteInfo;

	ubFirstQuoteRecord = 0;
	ubLastQuoteRecord = NUM_NPC_QUOTE_RECORDS - 1;
	for (ubLoop = ubFirstQuoteRecord; ubLoop <= ubLastQuoteRecord; ubLoop++)
	{
		pNPCQuoteInfo = &(pNPCQuoteInfoArray[ ubLoop ]);
		if (sOldGridNo == -pNPCQuoteInfo->sRequiredGridno)
		{
			pNPCQuoteInfo->sRequiredGridno = -sNewGridNo;
		}
		if (sOldGridNo == pNPCQuoteInfo->usGoToGridno)
		{
			pNPCQuoteInfo->usGoToGridno = sNewGridNo;
		}
	}
}


void ReplaceLocationInNPCDataFromProfileID(UINT8 const ubNPC, INT16 const sOldGridNo, INT16 const sNewGridNo)
{
	NPCQuoteInfo* const quotes = EnsureQuoteFileLoaded(ubNPC);
	if (!quotes) return; // error
	ReplaceLocationInNPCData(quotes, sOldGridNo, sNewGridNo);
}


static void ResetOncePerConvoRecords(NPCQuoteInfo* pNPCQuoteInfoArray)
{
	UINT8									ubLoop;

	for ( ubLoop = 0; ubLoop < NUM_NPC_QUOTE_RECORDS; ubLoop++ )
	{
		if ( CHECK_FLAG(pNPCQuoteInfoArray[ubLoop].fFlags, QUOTE_FLAG_SAY_ONCE_PER_CONVO) )
		{
			TURN_FLAG_OFF( pNPCQuoteInfoArray[ubLoop].fFlags, QUOTE_FLAG_SAID );
		}
	}
}


void ResetOncePerConvoRecordsForNPC( UINT8 ubNPC )
{
	NPCQuoteInfo* const quotes = EnsureQuoteFileLoaded(ubNPC);
	if (!quotes) return; // error
	ResetOncePerConvoRecords(quotes);
}


void ResetOncePerConvoRecordsForAllNPCsInLoadedSector( void )
{
	UINT8	ubLoop;

	if ( gWorldSectorX == 0 || gWorldSectorY == 0 )
	{
		return;
	}

	for ( ubLoop = FIRST_RPC; ubLoop < NUM_PROFILES; ubLoop++ )
	{
		if ( gMercProfiles[ ubLoop ].sSectorX == gWorldSectorX &&
				gMercProfiles[ ubLoop ].sSectorY == gWorldSectorY &&
				gMercProfiles[ ubLoop ].bSectorZ == gbWorldSectorZ &&
				gpNPCQuoteInfoArray[ ubLoop ] != NULL )
		{
			ResetOncePerConvoRecordsForNPC( ubLoop );
		}
	}
}


static void NPCClosePanel()
{
	DialogueEvent::Add(new DialogueEventCallback<DeleteTalkingMenu>());
}


static void TalkingMenuGiveItem(ProfileID const npc, OBJECTTYPE* const object, INT8 const inv_pos)
{
	class DialogueEventGiveItem : public DialogueEvent
	{
		public:
			DialogueEventGiveItem(ProfileID const npc, OBJECTTYPE* const object, INT8 const inv_pos) :
				object_(object),
				npc_(npc),
				inv_pos_(inv_pos)
			{}

			bool Execute()
			{
				HandleNPCItemGiven(npc_, object_, inv_pos_);
				return false;
			}

		private:
			OBJECTTYPE* const object_;
			ProfileID   const npc_;
			INT8        const inv_pos_;
	};

	DialogueEvent::Add(new DialogueEventGiveItem(npc, object, inv_pos));
}


static void ReturnItemToPlayer(ProfileID const merc, OBJECTTYPE* const o)
{
	if (!o) return;

	SOLDIERTYPE* const s = FindSoldierByProfileID(merc);

	// Try to auto place object and then if it fails, put into cursor
	if (!AutoPlaceObject(s, o, FALSE))
	{
		InternalBeginItemPointer(s, o, NO_SLOT);
	}
	DirtyMercPanelInterface(s, DIRTYLEVEL2);
}


static void TriggerClosestMercWhoCanSeeNPC(UINT8 ubNPC, NPCQuoteInfo* pQuotePtr);


void ConverseFull(UINT8 const ubNPC, UINT8 const ubMerc, Approach bApproach, UINT8 const approach_record, OBJECTTYPE* const o)
{
	static INT32 giNPCSpecialReferenceCount = 0;

	NPCQuoteInfo					QuoteInfo;
	NPCQuoteInfo *				pQuotePtr = &(QuoteInfo);
	UINT8									ubLoop, ubQuoteNum, ubRecordNum;
	UINT32								uiDay;

	Assert((bApproach == APPROACH_GIVINGITEM) == (o != 0));

	SOLDIERTYPE* const pNPC = FindSoldierByProfileID(ubNPC);
	if ( pNPC )
	{
		// set delay for civ AI movement
		pNPC->uiTimeSinceLastSpoke = GetJA2Clock();

		if (!CheckFact(FACT_CURRENT_SECTOR_IS_SAFE, ubNPC))
		{
			if ( bApproach != TRIGGER_NPC && bApproach != APPROACH_GIVEFIRSTAID && bApproach != APPROACH_DECLARATION_OF_HOSTILITY && bApproach != APPROACH_ENEMY_NPC_QUOTE )
			{
				if ( NPCHasUnusedRecordWithGivenApproach( ubNPC, APPROACH_SECTOR_NOT_SAFE ) )
				{
					// override with sector-not-safe approach
					bApproach = APPROACH_SECTOR_NOT_SAFE;
				}
			}
		}

		// make sure civ is awake now
		pNPC->fAIFlags &= (~AI_ASLEEP);
	}

	NPCQuoteInfo* const pNPCQuoteInfoArray = EnsureQuoteFileLoaded(ubNPC);
	if (!pNPCQuoteInfoArray)
	{ // error!!!
		ReturnItemToPlayer(ubMerc, o);
		return;
	}

	MERCPROFILESTRUCT& p = GetProfile(ubNPC);
	switch( bApproach )
	{
		case NPC_INITIAL_QUOTE:
			// reset stuff
			ResetOncePerConvoRecords( pNPCQuoteInfoArray );

			// CHEAP HACK
			// Since we don't have CONDITIONAL once-per-convo refreshes, do this in code
			// NB fact 281 is 'Darren has explained boxing rules'
			if (ubNPC == DARREN && !CheckFact(FACT_281, DARREN))
			{
				TURN_FLAG_OFF( pNPCQuoteInfoArray[11].fFlags, QUOTE_FLAG_SAID );
			}

			// turn the NPC to face us
			// this '1' value is a dummy....
			NPCDoAction( ubNPC, NPC_ACTION_TURN_TO_FACE_NEAREST_MERC, 1 );

			if (p.ubLastDateSpokenTo > 0)
			{
				uiDay = GetWorldDay();
				if (uiDay > p.ubLastDateSpokenTo)
				{
					NPCConsiderTalking( ubNPC, ubMerc, APPROACH_SPECIAL_INITIAL_QUOTE, 0, pNPCQuoteInfoArray, &pQuotePtr, &ubRecordNum );
					if (pQuotePtr != NULL)
					{
						// converse using this approach instead!
						ReturnItemToPlayer(ubMerc, o);
						Converse(ubNPC, ubMerc, APPROACH_SPECIAL_INITIAL_QUOTE);
						return;
					}
					// subsequent times approached intro
					ubQuoteNum = QUOTE_SUBS_INTRO;
				}
				else
				{
					// say nothing!
					ReturnItemToPlayer(ubMerc, o);
					return;
				}
			}
			else
			{
				// try special initial quote first
				NPCConsiderTalking( ubNPC, ubMerc, APPROACH_SPECIAL_INITIAL_QUOTE, 0, pNPCQuoteInfoArray, &pQuotePtr, &ubRecordNum );
				if (pQuotePtr != NULL)
				{
					// converse using this approach instead!
					ReturnItemToPlayer(ubMerc, o);
					Converse(ubNPC, ubMerc, APPROACH_SPECIAL_INITIAL_QUOTE);
					return;
				}

				NPCConsiderTalking( ubNPC, ubMerc, APPROACH_INITIAL_QUOTE, 0, pNPCQuoteInfoArray, &pQuotePtr, &ubRecordNum );
				if (pQuotePtr != NULL)
				{
					// converse using this approach instead!
					ReturnItemToPlayer(ubMerc, o);
					Converse(ubNPC, ubMerc, APPROACH_INITIAL_QUOTE);
					return;
				}

				// first time approached intro
				ubQuoteNum = QUOTE_INTRO;
			}
			TalkingMenuDialogue( ubQuoteNum );
			p.ubLastQuoteSaid          = ubQuoteNum;
			p.bLastQuoteSaidWasSpecial = FALSE;
			break;
		case NPC_WHOAREYOU:
			ubQuoteNum = QUOTE_INTRO;
			TalkingMenuDialogue( ubQuoteNum );
			// For now, DO NOT remember for 'Come again?'
			break;
		case APPROACH_REPEAT:
			if (p.ubLastQuoteSaid == NO_QUOTE)
			{
				// this should never occur now!
				TalkingMenuDialogue( QUOTE_INTRO );
			}
			else
			{
				if (p.bLastQuoteSaidWasSpecial)
				{
					pQuotePtr = &(pNPCQuoteInfoArray[p.ubLastQuoteSaid]);
					// say quote and following consecutive quotes
					for (ubLoop = 0; ubLoop < pQuotePtr->ubNumQuotes; ubLoop++)
					{
						// say quote #(pQuotePtr->ubQuoteNum + ubLoop)
						TalkingMenuDialogue( (UINT8)( pQuotePtr->ubQuoteNum + ubLoop ) );
					}
				}
				else
				{
					TalkingMenuDialogue(p.ubLastQuoteSaid);
				}
			}
			break;
		default:
			switch( bApproach )
			{
				case APPROACH_GIVINGITEM:
					// first start by triggering any introduction quote if there is one...
					if (p.ubLastDateSpokenTo > 0)
					{
						uiDay = GetWorldDay();
						if (uiDay > p.ubLastDateSpokenTo)
						{
							NPCConsiderTalking( ubNPC, ubMerc, APPROACH_SPECIAL_INITIAL_QUOTE, 0, pNPCQuoteInfoArray, &pQuotePtr, &ubRecordNum );
							if (pQuotePtr != NULL)
							{
								// converse using this approach instead!
								Converse(ubNPC, ubMerc, APPROACH_SPECIAL_INITIAL_QUOTE);

								if ( ubNPC == DARREN )
								{
									// then we have to make this give attempt fail
									ReturnItemToPlayer(ubMerc, o);
									return;
								}
							}
						}
					}
					else
					{
						NPCConsiderTalking( ubNPC, ubMerc, APPROACH_INITIAL_QUOTE, 0, pNPCQuoteInfoArray, &pQuotePtr, &ubRecordNum );
						if (pQuotePtr != NULL)
						{
							// converse using this approach instead!
							Converse(ubNPC, ubMerc, APPROACH_INITIAL_QUOTE);
						}
					}

					// If we are approaching because we want to give an item, do something different
					NPCConsiderReceivingItemFromMerc(ubNPC, ubMerc, o, pNPCQuoteInfoArray, &pQuotePtr, &ubRecordNum);
					break;
				case TRIGGER_NPC:
					// if triggering, pass in the approach data as the record to consider
					SLOGD(DEBUG_TAG_AI, "Handling trigger %ls/%d at %ld", gMercProfiles[ubNPC].zNickname, approach_record, GetJA2Clock());
					NPCConsiderTalking(ubNPC, ubMerc, bApproach, approach_record, pNPCQuoteInfoArray, &pQuotePtr, &ubRecordNum);
					break;
				default:
					NPCConsiderTalking( ubNPC, ubMerc, bApproach, 0, pNPCQuoteInfoArray, &pQuotePtr, &ubRecordNum );
					break;
			}
			if (pQuotePtr == NULL)
			{
				// say random everyday quote
				// do NOT set last quote said!
				switch( bApproach )
				{
					case APPROACH_FRIENDLY:
						if (p.bFriendlyOrDirectDefaultResponseUsedRecently)
						{
							ubQuoteNum = QUOTE_GETLOST;
						}
						else
						{
							ubQuoteNum = QUOTE_FRIENDLY_DEFAULT1 + (UINT8) Random( 2 );
							p.bFriendlyOrDirectDefaultResponseUsedRecently = TRUE;
						}
						break;
					case APPROACH_DIRECT:
						if (p.bFriendlyOrDirectDefaultResponseUsedRecently)
						{
							ubQuoteNum = QUOTE_GETLOST;
						}
						else
						{
							ubQuoteNum = QUOTE_DIRECT_DEFAULT;
							p.bFriendlyOrDirectDefaultResponseUsedRecently = TRUE;
						}
						break;
					case APPROACH_THREATEN:
						if (p.bThreatenDefaultResponseUsedRecently)
						{
							ubQuoteNum = QUOTE_GETLOST;
						}
						else
						{
							ubQuoteNum = QUOTE_THREATEN_DEFAULT;
							p.bThreatenDefaultResponseUsedRecently = TRUE;
						}
						break;
					case APPROACH_RECRUIT:
						if (p.bRecruitDefaultResponseUsedRecently)
						{
							ubQuoteNum = QUOTE_GETLOST;
						}
						else
						{
							ubQuoteNum = QUOTE_RECRUIT_NO;
							p.bRecruitDefaultResponseUsedRecently = TRUE;
						}
						break;
					case APPROACH_GIVINGITEM:
						ubQuoteNum = QUOTE_GIVEITEM_NO;

						/*
						CC - now handled below
						*/
						break;
					case TRIGGER_NPC:
						// trigger did not succeed - abort!!
						return;
					default:
						ubQuoteNum = QUOTE_INTRO;
						break;
				}
				TalkingMenuDialogue( ubQuoteNum );
				p.ubLastQuoteSaid          = ubQuoteNum;
				p.bLastQuoteSaidWasSpecial = FALSE;
				if (ubQuoteNum == QUOTE_GETLOST)
				{
					if (ubNPC == 70 || ubNPC == 120)
					{
						// becomes an enemy
						NPCDoAction( ubNPC, NPC_ACTION_BECOME_ENEMY, 0 );
					}
					// close panel at end of speech
					NPCClosePanel();
				}
				else if ( ubQuoteNum == QUOTE_GIVEITEM_NO )
				{
					// close panel at end of speech
					NPCClosePanel();
					if ( pNPC )
					{
						switch( ubNPC )
						{
							case JIM:
							case JACK:
							case OLAF:
							case RAY:
							case OLGA:
							case TYRONE:
								// Start combat etc
								CancelAIAction(pNPC);
								AddToShouldBecomeHostileOrSayQuoteList(pNPC);
							default:
								break;
						}
					}
				}
			}
			else
			{
				// turn before speech?
				if ( pQuotePtr->sActionData <= -NPC_ACTION_TURN_TO_FACE_NEAREST_MERC )
				{
					SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubNPC);
					ZEROTIMECOUNTER( pSoldier->AICounter );
					if (pSoldier->bNextAction == AI_ACTION_WAIT)
					{
						pSoldier->bNextAction = AI_ACTION_NONE;
						pSoldier->usNextActionData = 0;
					}
					NPCDoAction( ubNPC, (UINT16) -(pQuotePtr->sActionData), ubRecordNum );
				}
				if (pQuotePtr->ubQuoteNum != NO_QUOTE)
				{
					// say quote and following consecutive quotes
					for (ubLoop = 0; ubLoop < pQuotePtr->ubNumQuotes; ubLoop++)
					{
						TalkingMenuDialogue( (UINT8)( pQuotePtr->ubQuoteNum + ubLoop  ) );
					}
					p.ubLastQuoteSaid          = ubRecordNum;
					p.bLastQuoteSaidWasSpecial = TRUE;
				}
				// set to "said" if we should do so
				if (pQuotePtr->fFlags & QUOTE_FLAG_ERASE_ONCE_SAID || pQuotePtr->fFlags & QUOTE_FLAG_SAY_ONCE_PER_CONVO)
				{
					TURN_FLAG_ON( pQuotePtr->fFlags, QUOTE_FLAG_SAID );
				}

				// Carry out implications (actions) of this record

				// Give NPC item if appropriate
				if (bApproach == APPROACH_GIVINGITEM )
				{
					if ( pQuotePtr->sActionData != NPC_ACTION_DONT_ACCEPT_ITEM )
					{
						PlaceObjectInSoldierProfile(ubNPC, o);

						// Find the GIVER....
						SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubMerc);

						// Is this one of us?
						if ( pSoldier->bTeam == OUR_TEAM )
						{
							INT8 const bSlot = FindExactObj(pSoldier, o);
							if (bSlot != NO_SLOT)
							{
								RemoveObjs(&pSoldier->inv[bSlot], o->ubNumberOfObjects);
								DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
							}
						}
						else
						{
							RemoveObjectFromSoldierProfile(ubMerc, o->usItem);
						}
					}
					// CC: now handled below
					/*
					else
					{
						// ATE: Here, put back into inventory or place on ground....
						{
							SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubMerc);

							// Try to auto place object and then if it fails, put into cursor
							if (!AutoPlaceObject(pSoldier, o, FALSE))
							{
								InternalBeginItemPointer(pSoldier, o, NO_SLOT);
							}
							DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );

						}
					}
					*/
				}
				else if (bApproach == APPROACH_RECRUIT)
				{
					// the guy just joined our party
				}

				// Set things
				if (pQuotePtr->usSetFactTrue != NO_FACT)
				{
					SetFactTrue((Fact)pQuotePtr->usSetFactTrue);
				}
				if (pQuotePtr->ubEndQuest != NO_QUEST)
				{
					EndQuest( pQuotePtr->ubEndQuest, gWorldSectorX, gWorldSectorY );
				}
				if (pQuotePtr->ubStartQuest != NO_QUEST)
				{
					StartQuest( pQuotePtr->ubStartQuest, gWorldSectorX, gWorldSectorY );
				}

				// Give item to merc?
				if ( pQuotePtr->usGiftItem >= TURN_UI_OFF )
				{
					switch ( pQuotePtr->usGiftItem )
					{
						case TURN_UI_OFF:
							if ( !(gTacticalStatus.uiFlags & INCOMBAT) )
							{
								gTacticalStatus.uiFlags |= ENGAGED_IN_CONV;
								// Increment refrence count...
								giNPCReferenceCount = 1;
							}
							break;
						case TURN_UI_ON:
							// while the special ref count is set, ignore standard off
							if ( giNPCSpecialReferenceCount == 0 )
							{
								gTacticalStatus.uiFlags &= ~ENGAGED_IN_CONV;
								// Decrement refrence count...
								giNPCReferenceCount = 0;
							}
							break;
						case SPECIAL_TURN_UI_OFF:
							if ( !(gTacticalStatus.uiFlags & INCOMBAT) )
							{

								gTacticalStatus.uiFlags |= ENGAGED_IN_CONV;
								// Increment refrence count...
								giNPCReferenceCount = 1;
								if ( giNPCSpecialReferenceCount < 0 )
								{
									// ???
									giNPCSpecialReferenceCount = 0;
								}
								// increment SPECIAL reference count
								giNPCSpecialReferenceCount += 1;
							}
							break;
						case SPECIAL_TURN_UI_ON:
							// Decrement SPECIAL reference count
							giNPCSpecialReferenceCount -= 1;
							// if special count is now 0, turn reactivate UI
							if ( giNPCSpecialReferenceCount == 0 )
							{
								gTacticalStatus.uiFlags &= ~ENGAGED_IN_CONV;
								giNPCReferenceCount = 0;
							}
							else if ( giNPCSpecialReferenceCount < 0 )
							{
								// ???
								giNPCSpecialReferenceCount = 0;
							}
							break;
					}
				}
				else if ( pQuotePtr->usGiftItem != 0 )
				{
					{
						INT8 bInvPos;

						SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubNPC);

						// Look for item....
						bInvPos = FindObj( pSoldier, pQuotePtr->usGiftItem );

						AssertMsg( bInvPos != NO_SLOT, "NPC.C:  Gift item does not exist in NPC." );

						TalkingMenuGiveItem( ubNPC, &(pSoldier->inv[ bInvPos ] ), bInvPos );
					}
				}
				// Action before movement?
				if ( pQuotePtr->sActionData < 0 && pQuotePtr->sActionData > -NPC_ACTION_TURN_TO_FACE_NEAREST_MERC )
				{
					SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubNPC);
					ZEROTIMECOUNTER( pSoldier->AICounter );
					if (pSoldier->bNextAction == AI_ACTION_WAIT)
					{
						pSoldier->bNextAction = AI_ACTION_NONE;
						pSoldier->usNextActionData = 0;
					}
					NPCDoAction( ubNPC, (UINT16) -(pQuotePtr->sActionData), ubRecordNum );
				}
				else if ( pQuotePtr->usGoToGridno == NO_MOVE && pQuotePtr->sActionData > 0 )
				{
					SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubNPC);
					ZEROTIMECOUNTER( pSoldier->AICounter );
					if (pSoldier->bNextAction == AI_ACTION_WAIT)
					{
						pSoldier->bNextAction = AI_ACTION_NONE;
						pSoldier->usNextActionData = 0;
					}
					NPCDoAction( ubNPC, (UINT16) (pQuotePtr->sActionData), ubRecordNum );
				}

				// Movement?
				if ( pQuotePtr->usGoToGridno != NO_MOVE )
				{
					SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubNPC);

					// stupid hack CC
					if (pSoldier && ubNPC == KYLE)
					{
						// make sure he has keys
						pSoldier->bHasKeys = TRUE;
					}
					if (pSoldier && pSoldier->sGridNo == pQuotePtr->usGoToGridno )
					{
						// search for quotes to trigger immediately!
						pSoldier->ubQuoteRecord = ubRecordNum + 1; // add 1 so that the value is guaranteed nonzero
						NPCReachedDestination( pSoldier, TRUE );
					}
					else
					{
						// turn off cowering
						if ( pNPC->uiStatusFlags & SOLDIER_COWERING)
						{
							//pNPC->uiStatusFlags &= ~SOLDIER_COWERING;
							EVENT_InitNewSoldierAnim( pNPC, STANDING, 0 , FALSE );
						}

						pSoldier->ubQuoteRecord = ubRecordNum + 1; // add 1 so that the value is guaranteed nonzero

						if (pQuotePtr->sActionData == NPC_ACTION_TELEPORT_NPC)
						{
							BumpAnyExistingMerc( pQuotePtr->usGoToGridno );
							TeleportSoldier(*pSoldier, pQuotePtr->usGoToGridno, false);
							// search for quotes to trigger immediately!
							NPCReachedDestination( pSoldier, FALSE );
						}
						else
						{
							NPCGotoGridNo( ubNPC, pQuotePtr->usGoToGridno, ubRecordNum );
						}
					}
				}

				// Trigger other NPC?
				// ATE: Do all triggers last!
				if ( pQuotePtr->ubTriggerNPC != IRRELEVANT )
				{
					// Check for special NPC trigger codes
					if ( pQuotePtr->ubTriggerNPC == 0 )
					{
						TriggerClosestMercWhoCanSeeNPC( ubNPC, pQuotePtr );
					}
					else if ( pQuotePtr->ubTriggerNPC == 1 )
					{
						// trigger self
						TriggerNPCRecord( ubNPC, pQuotePtr->ubTriggerNPCRec );
					}
					else
					{
						TriggerNPCRecord( pQuotePtr->ubTriggerNPC, pQuotePtr->ubTriggerNPCRec );
					}
				}

				// Ian says it is okay to take this out!
				/*
				if (bApproach == APPROACH_ENEMY_NPC_QUOTE)
				{
					NPCClosePanel();
				}
				*/

			}
			break;
	}

	// Set last day spoken!
	switch( bApproach )
	{
		case APPROACH_FRIENDLY:
		case APPROACH_DIRECT:
		case APPROACH_THREATEN:
		case APPROACH_RECRUIT:
		case NPC_INITIATING_CONV:
		case NPC_INITIAL_QUOTE:
		case APPROACH_SPECIAL_INITIAL_QUOTE:
		case APPROACH_DECLARATION_OF_HOSTILITY:
		case APPROACH_INITIAL_QUOTE:
		case APPROACH_GIVINGITEM:
			p.ubLastDateSpokenTo = (UINT8)GetWorldDay();
			break;
		default:
			break;
	}

	/* If the approach was changed, always return the item. Otherwise check to see
	 * if the record in question specified refusal */
	if (bApproach != APPROACH_GIVINGITEM || !pQuotePtr || pQuotePtr->sActionData == NPC_ACTION_DONT_ACCEPT_ITEM)
	{
		ReturnItemToPlayer(ubMerc, o);
	}
}


void Converse(UINT8 const ubNPC, UINT8 const ubMerc, Approach const bApproach)
{
	ConverseFull(ubNPC, ubMerc, bApproach, 0, 0);
}


INT16 NPCConsiderInitiatingConv(const SOLDIERTYPE* const pNPC)
{
	UINT8         const ubNPC              = pNPC->ubProfile;
	NPCQuoteInfo* const pNPCQuoteInfoArray = EnsureQuoteFileLoaded(ubNPC);
	if (!pNPCQuoteInfoArray) return NOWHERE; // error

	const INT16  sMyGridNo           = pNPC->sGridNo;
	INT16        sDesiredMercDist    = 100;
	UINT8        ubDesiredMerc       = NOBODY;
	UINT8        ubHighestTalkDesire = 0;
	SOLDIERTYPE* pDesiredMerc        = NULL; // XXX HACK000E
	// loop through all mercs
	for (UINT8 ubMerc = 0; ubMerc < guiNumMercSlots; ++ubMerc)
	{
		const SOLDIERTYPE* const pMerc = MercSlots[ubMerc];
		if (pMerc == NULL) continue;

		// only look for mercs on the side of the player
		if (pMerc->bSide != OUR_TEAM) continue;

		// only look for active mercs
		if (pMerc->bAssignment >= ON_DUTY) continue;

		// if they're not visible, don't think about it
		if (pNPC->bOppList[ubMerc] != SEEN_CURRENTLY) continue;

		/* what's the opinion required for the highest-opinion quote that we would
		 * say to this merc */
		const UINT8 ubTalkDesire = NPCConsiderTalking(pNPC->ubProfile, pMerc->ubProfile, NPC_INITIATING_CONV, 0, pNPCQuoteInfoArray, NULL, NULL);
		if (ubTalkDesire == 0) continue;

		if (ubTalkDesire > ubHighestTalkDesire)
		{
			ubHighestTalkDesire = ubTalkDesire;
			ubDesiredMerc       = ubMerc;
			pDesiredMerc        = &GetMan(ubMerc);
			sDesiredMercDist    = PythSpacesAway(sMyGridNo, pDesiredMerc->sGridNo);
		}
		else if (ubTalkDesire == ubHighestTalkDesire)
		{
			INT16 const sDist = PythSpacesAway(sMyGridNo, GetMan(ubMerc).sGridNo);
			if (sDist < sDesiredMercDist)
			{
				// we can say the same thing to this merc, and they're closer!
				ubDesiredMerc    = ubMerc;
				sDesiredMercDist = sDist;
			}
		}
	}

	return ubDesiredMerc == NOBODY ? NOWHERE : pDesiredMerc->sGridNo;
}


void NPCReachedDestination( SOLDIERTYPE * pNPC, BOOLEAN fAlreadyThere )
{
	// perform action or whatever after reaching our destination
	UINT8		ubNPC;
	NPCQuoteInfo *				pQuotePtr;
	UINT8									ubLoop;
	UINT8									ubQuoteRecord;

	if ( pNPC->ubQuoteRecord == 0 )
	{
		ubQuoteRecord = 0;
	}
	else
	{
		ubQuoteRecord = (UINT8) (pNPC->ubQuoteRecord - 1);
	}

	// Clear values!
	pNPC->ubQuoteRecord = 0;
	if (pNPC->bTeam == OUR_TEAM)
	{
		// the "under ai control" flag was set temporarily; better turn it off now
		pNPC->uiStatusFlags &= (~SOLDIER_PCUNDERAICONTROL);
		// make damn sure the AI_HANDLE_EVERY_FRAME flag is turned off
		pNPC->fAIFlags &= (AI_HANDLE_EVERY_FRAME);
	}

	ubNPC = pNPC->ubProfile;
	NPCQuoteInfo* const pNPCQuoteInfoArray = EnsureQuoteFileLoaded(ubNPC);
	if (!pNPCQuoteInfoArray) return; // error

	pQuotePtr = &(pNPCQuoteInfoArray[ubQuoteRecord]);
	// either we are supposed to consider a new quote record
	// (indicated by a negative gridno in the has-item field)
	// or an action to perform once we reached this gridno

	if ( pNPC->sGridNo == pQuotePtr->usGoToGridno )
	{
		// check for an after-move action
		if ( pQuotePtr->sActionData > 0)
		{
			NPCDoAction( ubNPC, (UINT16) pQuotePtr->sActionData, ubQuoteRecord );
		}
	}

	for ( ubLoop = 0; ubLoop < NUM_NPC_QUOTE_RECORDS; ubLoop++ )
	{
		pQuotePtr = &(pNPCQuoteInfoArray[ubLoop]);
		if ( pNPC->sGridNo == -(pQuotePtr->sRequiredGridno ) )
		{
			if ( NPCConsiderQuote( ubNPC, 0, TRIGGER_NPC, ubLoop, 0, pNPCQuoteInfoArray ) )
			{
				if (fAlreadyThere)
				{
					TriggerNPCRecord( ubNPC, ubLoop );
				}
				else
				{
					// trigger this quote
					TriggerNPCRecordImmediately( ubNPC, ubLoop );
				}
				return;
			}
		}
	}
}


// Trigger an NPC record
static void NPCTriggerNPC(UINT8 const npc, UINT8 const record, Approach const approach, bool const show_dialogue_menu)
{
	class DialogueEventTriggerNPC : public DialogueEvent
	{
		public:
			DialogueEventTriggerNPC(ProfileID const npc, UINT8 const record, Approach const approach, bool const show_dialogue_menu) :
				npc_(npc),
				record_(record),
				show_dialogue_menu_(show_dialogue_menu),
				approach_(approach)
			{}

			bool Execute()
			{
				HandleNPCTriggerNPC(npc_, record_, show_dialogue_menu_, approach_);
				return false;
			}

		private:
			ProfileID const npc_;
			UINT8     const record_;
			bool      const show_dialogue_menu_;
			Approach  const approach_;
	};

	DialogueEvent::Add(new DialogueEventTriggerNPC(npc, record, approach, show_dialogue_menu));
}


void TriggerNPCRecord(UINT8 const ubTriggerNPC, UINT8 const record)
{
	// Check if we have a quote to trigger...

	NPCQuoteInfo* const quotes = EnsureQuoteFileLoaded(ubTriggerNPC);
	if (!quotes) return; // error
	NPCQuoteInfo const& q                = quotes[record];
	bool         const  display_dialogue = q.ubQuoteNum != IRRELEVANT;

	if (NPCConsiderQuote(ubTriggerNPC, 0, TRIGGER_NPC, record, 0, quotes))
	{
		NPCTriggerNPC(ubTriggerNPC, record, TRIGGER_NPC, display_dialogue);
	}
	else
	{ // don't do anything
		SLOGW(DEBUG_TAG_AI, "trigger of %d, record %d cannot proceed, possible error", ubTriggerNPC, record);
	}
}


void TriggerNPCRecordImmediately(UINT8 const ubTriggerNPC, UINT8 const record)
{
	// Check if we have a quote to trigger...

	NPCQuoteInfo* const quotes = EnsureQuoteFileLoaded(ubTriggerNPC);
	if (!quotes) return; // error
	NPCQuoteInfo const& q                = quotes[record];
	bool         const  display_dialogue = q.ubQuoteNum != IRRELEVANT;

	if (NPCConsiderQuote(ubTriggerNPC, 0, TRIGGER_NPC, record, 0, quotes))
	{ // trigger IMMEDIATELY
		HandleNPCTriggerNPC(ubTriggerNPC, record, display_dialogue, TRIGGER_NPC);
	}
	else
	{ // don't do anything
		SLOGW(DEBUG_TAG_AI, "trigger of %d, record %d cannot proceed, possible error", ubTriggerNPC, record);
	}
}


void PCsNearNPC( UINT8 ubNPC )
{
	UINT8									ubLoop;
	NPCQuoteInfo *				pQuotePtr;

	NPCQuoteInfo* const pNPCQuoteInfoArray = EnsureQuoteFileLoaded(ubNPC);
	if (!pNPCQuoteInfoArray) return; // error

	// see what this triggers...
	SetFactTrue( FACT_PC_NEAR );

	// Clear values!
	// Get value for NPC
	SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubNPC);
	pSoldier->ubQuoteRecord = 0;

	for ( ubLoop = 0; ubLoop < NUM_NPC_QUOTE_RECORDS; ubLoop++ )
	{
		pQuotePtr = &(pNPCQuoteInfoArray[ubLoop]);
		if ( pSoldier->sGridNo == -(pQuotePtr->sRequiredGridno ) )
		{
			if ( NPCConsiderQuote( ubNPC, 0, TRIGGER_NPC, ubLoop, 0, pNPCQuoteInfoArray ) )
			{
				// trigger this quote IMMEDIATELY!
				TriggerNPCRecordImmediately( ubNPC, ubLoop );
				break;
			}
		}
	}

	// reset fact
	SetFactFalse( FACT_PC_NEAR );
}

BOOLEAN PCDoesFirstAidOnNPC( UINT8 ubNPC )
{
	UINT8									ubLoop;
	NPCQuoteInfo *				pQuotePtr;

	NPCQuoteInfo* const pNPCQuoteInfoArray = EnsureQuoteFileLoaded(ubNPC);
	if (!pNPCQuoteInfoArray) return FALSE; // error

	SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubNPC);
	// Clear values!
	pSoldier->ubQuoteRecord = 0;

	// Set flag...
	gMercProfiles[ ubNPC ].ubMiscFlags2 |= PROFILE_MISC_FLAG2_BANDAGED_TODAY;

	for ( ubLoop = 0; ubLoop < NUM_NPC_QUOTE_RECORDS; ubLoop++ )
	{
		pQuotePtr = &(pNPCQuoteInfoArray[ubLoop]);
		if ( pQuotePtr->ubApproachRequired == APPROACH_GIVEFIRSTAID )
		{
			if ( NPCConsiderQuote( ubNPC, 0, TRIGGER_NPC, ubLoop, 0, pNPCQuoteInfoArray ) )
			{
				// trigger this quote IMMEDIATELY!
				TriggerNPCRecordImmediately( ubNPC, ubLoop );
				return( TRUE );
			}
		}
	}
	return( FALSE );
}


static void TriggerClosestMercWhoCanSeeNPC(UINT8 ubNPC, NPCQuoteInfo* pQuotePtr)
{
	// Loop through all mercs, gather closest mercs who can see and trigger one!
	UINT8	ubNumMercs = 0;

	const SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubNPC);

	// Loop through all our guys and randomly say one from someone in our sector
	SOLDIERTYPE* mercs_in_sector[40];
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		// Add guy if he's a candidate...
		if (OkControllableMerc(s) &&
				s->bOppList[pSoldier->ubID] == SEEN_CURRENTLY)
		{
			mercs_in_sector[ubNumMercs++] = s;
		}
	}

	// If we are > 0
	if ( ubNumMercs > 0 )
	{
		SOLDIERTYPE* const chosen = mercs_in_sector[Random(ubNumMercs)];

		// Post action to close panel
		NPCClosePanel( );

		// If 64, do something special
		if ( pQuotePtr->ubTriggerNPCRec == QUOTE_RESPONSE_TO_MIGUEL_SLASH_QUOTE_MERC_OR_RPC_LETGO )
		{
			class CharacterDialogueEventPCTriggerNPC : public CharacterDialogueEvent
			{
				public:
					CharacterDialogueEventPCTriggerNPC(SOLDIERTYPE& s) : CharacterDialogueEvent(s) {}

					bool Execute()
					{
						if (!MayExecute()) return true;

						SOLDIERTYPE const& s = soldier_;
						ExecuteCharacterDialogue(s.ubProfile, QUOTE_RESPONSE_TO_MIGUEL_SLASH_QUOTE_MERC_OR_RPC_LETGO, s.face, DIALOGUE_TACTICAL_UI, TRUE, false);

						// Setup face with data!
						FACETYPE& f = *gpCurrentTalkingFace;
						f.uiFlags          |= FACE_PCTRIGGER_NPC;
						f.u.trigger.npc     = MIGUEL;
						f.u.trigger.record  = 6;

						return false;
					}
			};

			DialogueEvent::Add(new CharacterDialogueEventPCTriggerNPC(*chosen));
			++giNPCReferenceCount;
		}
		else
		{
			TacticalCharacterDialogue(chosen, pQuotePtr->ubTriggerNPCRec);
		}
	}

}

BOOLEAN TriggerNPCWithIHateYouQuote( UINT8 ubTriggerNPC )
{
	// Check if we have a quote to trigger...
	NPCQuoteInfo	*pQuotePtr;
	UINT8					ubLoop;

	NPCQuoteInfo* const pNPCQuoteInfoArray = EnsureQuoteFileLoaded(ubTriggerNPC);
	if (!pNPCQuoteInfoArray) return FALSE; // error

	for ( ubLoop = 0; ubLoop < NUM_NPC_QUOTE_RECORDS; ubLoop++ )
	{
		pQuotePtr = &(pNPCQuoteInfoArray[ubLoop]);
		if ( NPCConsiderQuote( ubTriggerNPC, 0, APPROACH_DECLARATION_OF_HOSTILITY, ubLoop, 0, pNPCQuoteInfoArray ) )
		{
			// trigger this quote!
			// reset approach required value so that we can trigger it
			//pQuotePtr->ubApproachRequired = TRIGGER_NPC;
			NPCTriggerNPC(ubTriggerNPC, ubLoop, APPROACH_DECLARATION_OF_HOSTILITY, true);
			gMercProfiles[ ubTriggerNPC ].ubMiscFlags |= PROFILE_MISC_FLAG_SAID_HOSTILE_QUOTE;
			return( TRUE );
		}
	}
	return( FALSE );

}


BOOLEAN NPCHasUnusedRecordWithGivenApproach(UINT8 const ubNPC, Approach const approach)
{
	// Check if we have a quote that could be used

	NPCQuoteInfo* const quotes = EnsureQuoteFileLoaded(ubNPC);
	if (!quotes) return FALSE; // error

	for (UINT8 i = 0; i != NUM_NPC_QUOTE_RECORDS; ++i)
	{
		if (!NPCConsiderQuote(ubNPC, 0, approach, i, 0, quotes)) continue;
		return TRUE;
	}
	return FALSE;
}


BOOLEAN NPCHasUnusedHostileRecord(UINT8 const ubNPC, Approach const approach)
{
	/* this is just like the standard check BUT we must skip any records using
	 * fact 289 and print debug msg for any records which can't be marked as used
	 */
	// Check if we have a quote that could be used
	NPCQuoteInfo* const quotes = EnsureQuoteFileLoaded(ubNPC);
	if (!quotes) return FALSE; // error

	for (UINT8 i = 0; i != NUM_NPC_QUOTE_RECORDS; ++i)
	{
		if (!NPCConsiderQuote(ubNPC, 0, approach, i, 0, quotes)) continue;
		NPCQuoteInfo const& q = quotes[i];
		if (q.usFactMustBeTrue == FACT_NPC_HOSTILE_OR_PISSED_OFF) continue;
		return TRUE;
	}
	return FALSE;
}


BOOLEAN NPCWillingToAcceptItem(UINT8 const ubNPC, UINT8 const ubMerc, OBJECTTYPE* const o)
{
	// Check if we have a quote that could be used, that applies to this item

	NPCQuoteInfo* const quotes = EnsureQuoteFileLoaded(ubNPC);
	if (!quotes) return FALSE; // error

	NPCQuoteInfo* q;
	UINT8         ubQuoteNum;
	NPCConsiderReceivingItemFromMerc(ubNPC, ubMerc, o, quotes, &q, &ubQuoteNum);
	return q != 0;
}


BOOLEAN GetInfoForAbandoningEPC(UINT8 const ubNPC, UINT16* const pusQuoteNum, Fact* const fact_to_set_true)
{
	// Check if we have a quote that could be used
	NPCQuoteInfo* const quotes = EnsureQuoteFileLoaded(ubNPC);
	if (!quotes) return FALSE; // error

	for (UINT8 i = 0; i != NUM_NPC_QUOTE_RECORDS; ++i)
	{
		if (!NPCConsiderQuote(ubNPC, 0, APPROACH_EPC_IN_WRONG_SECTOR, i, 0, quotes)) continue;
		NPCQuoteInfo const& q = quotes[i];
		*pusQuoteNum      = q.ubQuoteNum;
		*fact_to_set_true = (Fact)q.usSetFactTrue;
		return TRUE;
	}
	return FALSE;
}


BOOLEAN TriggerNPCWithGivenApproach(UINT8 const ubTriggerNPC, Approach const approach)
{
	// Check if we have a quote to trigger...

	NPCQuoteInfo* const quotes = EnsureQuoteFileLoaded(ubTriggerNPC);
	if (!quotes) return FALSE; // error

	for (UINT8 i = 0; i != NUM_NPC_QUOTE_RECORDS; ++i)
	{
		if (!NPCConsiderQuote(ubTriggerNPC, 0, approach, i, 0, quotes)) continue;

		bool const show_panel = quotes[i].ubQuoteNum != IRRELEVANT;
		NPCTriggerNPC(ubTriggerNPC, i, approach, show_panel);
		return TRUE;
	}
	return FALSE;
}


void SaveNPCInfoToSaveGameFile(HWFILE const f)
{
	FOR_EACH(NPCQuoteInfo* const, i, gpNPCQuoteInfoArray)
	{
		ConditionalInjectNPCQuoteInfoArrayIntoFile(f, *i);
	}

	FOR_EACH(NPCQuoteInfo* const, i, gpCivQuoteInfoArray)
	{
		ConditionalInjectNPCQuoteInfoArrayIntoFile(f, *i);
	}
}


void LoadNPCInfoFromSavedGameFile(HWFILE const f, UINT32 const uiSaveGameVersion)
{
	UINT32		cnt;
	UINT32		uiNumberToLoad=0;

	// If we are trying to restore a saved game prior to version 44, use the
	// MAX_NUM_SOLDIERS, else use NUM_PROFILES.  Dave used the wrong define!
	if( uiSaveGameVersion >= 44 )
		uiNumberToLoad = NUM_PROFILES;
	else
		uiNumberToLoad = MAX_NUM_SOLDIERS;

	//Loop through all the NPC quotes
	for( cnt=0; cnt<uiNumberToLoad; cnt++ )
	{
		ConditionalExtractNPCQuoteInfoArrayFromFile(f, gpNPCQuoteInfoArray[cnt]);
	}

	if ( uiSaveGameVersion >= 56 )
	{
		for( cnt = 0; cnt < NUM_CIVQUOTE_SECTORS; cnt++)
		{
			ConditionalExtractNPCQuoteInfoArrayFromFile(f, gpCivQuoteInfoArray[cnt]);
		}
	}

	if ( uiSaveGameVersion < 88 )
	{
		RefreshNPCScriptRecord( NO_PROFILE, 5 ); // special pass-in value for "replace PC scripts"
		RefreshNPCScriptRecord( DARYL, 11 );
		RefreshNPCScriptRecord( DARYL, 14 );
		RefreshNPCScriptRecord( DARYL, 15 );
	}
	if ( uiSaveGameVersion < 89 )
	{
		RefreshNPCScriptRecord( KINGPIN, 23 );
		RefreshNPCScriptRecord( KINGPIN, 27 );
	}
	if ( uiSaveGameVersion < 90 )
	{
		RefreshNPCScriptRecord( KINGPIN, 25 );
		RefreshNPCScriptRecord( KINGPIN, 26 );
	}
	if ( uiSaveGameVersion < 92 )
	{
		RefreshNPCScriptRecord( MATT, 14 );
		RefreshNPCScriptRecord( AUNTIE, 8 );
	}
	if ( uiSaveGameVersion < 93 )
	{
		RefreshNPCScriptRecord( JENNY, 7 );
		RefreshNPCScriptRecord( JENNY, 8 );
		RefreshNPCScriptRecord( FRANK, 7 );
		RefreshNPCScriptRecord( FRANK, 8 );
		RefreshNPCScriptRecord( FATHER, 12 );
		RefreshNPCScriptRecord( FATHER, 13 );
	}
	if ( uiSaveGameVersion < 94 )
	{
		RefreshNPCScriptRecord( CONRAD, 0 );
		RefreshNPCScriptRecord( CONRAD, 2 );
		RefreshNPCScriptRecord( CONRAD, 9 );
	}
	if ( uiSaveGameVersion < 95 )
	{
		RefreshNPCScriptRecord( WALDO, 6 );
		RefreshNPCScriptRecord( WALDO, 7 );
		RefreshNPCScriptRecord( WALDO, 10 );
		RefreshNPCScriptRecord( WALDO, 11 );
		RefreshNPCScriptRecord( WALDO, 12 );
	}
	if ( uiSaveGameVersion < 96 )
	{
		RefreshNPCScriptRecord( HANS, 18 );
		RefreshNPCScriptRecord( ARMAND, 13 );
		RefreshNPCScriptRecord( DARREN, 4 );
		RefreshNPCScriptRecord( DARREN, 5 );
	}
	if ( uiSaveGameVersion < 97 )
	{
		RefreshNPCScriptRecord( JOHN, 22 );
		RefreshNPCScriptRecord( JOHN, 23 );
		RefreshNPCScriptRecord( SKYRIDER, 19 );
		RefreshNPCScriptRecord( SKYRIDER, 21 );
		RefreshNPCScriptRecord( SKYRIDER, 22 );
	}

	if ( uiSaveGameVersion < 98 )
	{
		RefreshNPCScriptRecord( SKYRIDER, 19 );
		RefreshNPCScriptRecord( SKYRIDER, 21 );
		RefreshNPCScriptRecord( SKYRIDER, 22 );
	}
}


void SaveBackupNPCInfoToSaveGameFile(HWFILE const f)
{
	FOR_EACH(NPCQuoteInfo* const, i, gpBackupNPCQuoteInfoArray)
	{
		ConditionalInjectNPCQuoteInfoArrayIntoFile(f, *i);
	}
}


void LoadBackupNPCInfoFromSavedGameFile(HWFILE const f)
{
	FOR_EACH(NPCQuoteInfo*, i, gpBackupNPCQuoteInfoArray)
	{
		ConditionalExtractNPCQuoteInfoArrayFromFile(f, *i);
	}
}


void TriggerFriendWithHostileQuote( UINT8 ubNPC )
{
	UINT8						ubMercsAvailable[ 40 ] = { 0 };
	UINT8						ubNumMercsAvailable = 0, ubChosenMerc;

	SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubNPC);
	if (!pSoldier)
	{
		return;
	}
	const INT8 bTeam = pSoldier->bTeam;

	// Loop through all our guys and find one to yell
	CFOR_EACH_IN_TEAM(s, bTeam)
	{
		// Add guy if he's a candidate...
		if (pSoldier->bInSector && s->bLife >= OKLIFE && s->bBreath >= OKBREATH && s->bOppCnt > 0 && s->ubProfile != NO_PROFILE)
		{
			if (bTeam == CIV_TEAM && pSoldier->ubCivilianGroup != NON_CIV_GROUP && s->ubCivilianGroup != pSoldier->ubCivilianGroup)
			{
				continue;
			}

			if (!(gMercProfiles[s->ubProfile].ubMiscFlags & PROFILE_MISC_FLAG_SAID_HOSTILE_QUOTE))
			{
				ubMercsAvailable[ubNumMercsAvailable] = s->ubProfile;
				ubNumMercsAvailable++;
			}
		}
	}

	if ( bTeam == CIV_TEAM && pSoldier->ubCivilianGroup != NON_CIV_GROUP && gTacticalStatus.fCivGroupHostile[ pSoldier->ubCivilianGroup ] == CIV_GROUP_NEUTRAL )
	{
		CivilianGroupMemberChangesSides( pSoldier );
	}

	if (ubNumMercsAvailable > 0)
	{
		PauseAITemporarily();
		ubChosenMerc = (UINT8) Random( ubNumMercsAvailable );
		TriggerNPCWithIHateYouQuote( ubMercsAvailable[ ubChosenMerc ] );
	}
	else
	{
		// done... we should enter combat mode with this soldier's team starting,
		// after all the dialogue is completed
		NPCDoAction( ubNPC, NPC_ACTION_ENTER_COMBAT, 0 );
	}
}


UINT8 ActionIDForMovementRecord(UINT8 const ubNPC, UINT8 const record)
{
	// Check if we have a quote to trigger...
	NPCQuoteInfo* const quotes = EnsureQuoteFileLoaded(ubNPC);
	if (!quotes) return FALSE; // error

	switch (quotes[record].sActionData)
	{
		case NPC_ACTION_TRAVERSE_MAP_EAST:  return QUOTE_ACTION_ID_TRAVERSE_EAST;
		case NPC_ACTION_TRAVERSE_MAP_SOUTH: return QUOTE_ACTION_ID_TRAVERSE_SOUTH;
		case NPC_ACTION_TRAVERSE_MAP_WEST:  return QUOTE_ACTION_ID_TRAVERSE_WEST;
		case NPC_ACTION_TRAVERSE_MAP_NORTH: return QUOTE_ACTION_ID_TRAVERSE_NORTH;
		default:                            return QUOTE_ACTION_ID_CHECKFORDEST;
	}
}


void HandleNPCChangesForTacticalTraversal(const SOLDIERTYPE* s)
{
	if (!s || s->ubProfile == NO_PROFILE || s->fAIFlags & AI_CHECK_SCHEDULE)
	{
		return;
	}

	MERCPROFILESTRUCT& p = GetProfile(s->ubProfile);
	switch (s->ubQuoteActionID)
	{
		case QUOTE_ACTION_ID_TRAVERSE_EAST:  p.sSectorX++; break;
		case QUOTE_ACTION_ID_TRAVERSE_SOUTH: p.sSectorY++; break;
		case QUOTE_ACTION_ID_TRAVERSE_WEST:  p.sSectorX--; break;
		case QUOTE_ACTION_ID_TRAVERSE_NORTH: p.sSectorY--; break;
		default: return;
	}

	// Call to change the NPC's Sector Location
	ChangeNpcToDifferentSector(p, p.sSectorX, p.sSectorY, p.bSectorZ);
}


void HandleVictoryInNPCSector(INT16 const x, INT16 const y, INT16 const z)
{
	// handle special cases of victory in certain sector

	// not the surface?..leave
	if (z != 0) return;

	switch (SECTOR(x, y))
	{
		case SEC_F10:
			// we won over the hillbillies
			// set fact they are dead
			if (!CheckFact(FACT_HILLBILLIES_KILLED, KEITH))
			{
				SetFactTrue(FACT_HILLBILLIES_KILLED);
			}

			// check if keith is out of business
			if (CheckFact(FACT_KEITH_OUT_OF_BUSINESS, KEITH) == TRUE)
			{
				SetFactFalse(FACT_KEITH_OUT_OF_BUSINESS);
			}
			break;
	}
}


BOOLEAN HandleShopKeepHasBeenShutDown( UINT8 ubCharNum )
{
	// check if shopkeep has been shutdown, if so handle
	switch( ubCharNum )
	{
		case( KEITH ):
		{
			// if keith out of business, do action and leave
			if( CheckFact( FACT_KEITH_OUT_OF_BUSINESS, KEITH ) == TRUE )
			{
				TriggerNPCRecord( KEITH, 11 );

				return( TRUE );
			}
			else if( CheckFact( FACT_LOYALTY_LOW, KEITH ) == TRUE )
			{
				// loyalty is too low
				TriggerNPCRecord( KEITH, 7 );

				return( TRUE );
			}
		}
	}

	return( FALSE );
}

void UpdateDarrelScriptToGoTo( SOLDIERTYPE * pSoldier )
{
	// change destination in Darrel record 10 to go to a gridno adjacent to the
	// soldier's gridno, and destination in record 11
	const SOLDIERTYPE* const pDarrel = FindSoldierByProfileID(DARREL);
	if ( !pDarrel )
	{
		return;
	}

	// find a spot to an alternate location nearby
	INT16 sAdjustedGridNo = FindGridNoFromSweetSpotExcludingSweetSpot(pDarrel, pSoldier->sGridNo, 5);
	if (sAdjustedGridNo == NOWHERE)
	{
		// yikes! try again with a bigger radius!
		sAdjustedGridNo = FindGridNoFromSweetSpotExcludingSweetSpot(pDarrel, pSoldier->sGridNo, 10);
		if (sAdjustedGridNo == NOWHERE)
		{
			// ok, now we're completely foobar
			return;
		}
	}

	NPCQuoteInfo* const quotes = EnsureQuoteFileLoaded(DARREL);
	quotes[10].usGoToGridno    = sAdjustedGridNo;
	quotes[11].sRequiredGridno = -sAdjustedGridNo;
	quotes[11].ubTriggerNPC    = pSoldier->ubProfile;
}


BOOLEAN RecordHasDialogue(UINT8 const ubNPC, UINT8 const ubRecord)
{
	NPCQuoteInfo* const quotes = EnsureQuoteFileLoaded(ubNPC);
	if (!quotes) return FALSE; // error
	NPCQuoteInfo const& q = quotes[ubRecord];
	return q.ubQuoteNum != NO_QUOTE && q.ubQuoteNum != 0;
}


static INT8 FindCivQuoteFileIndex(INT16 const x, INT16 const y, INT16 const z)
{
	if (z > 0) return MINERS_CIV_QUOTE_INDEX;

	for (UINT8 i = 0; i != NUM_CIVQUOTE_SECTORS; ++i)
	{
		if (gsCivQuoteSector[i][0] != x) continue;
		if (gsCivQuoteSector[i][1] != y) continue;
		return i;
	}
	return -1;
}


INT8 ConsiderCivilianQuotes(INT16 const x, INT16 const y, INT16 const z, BOOLEAN const set_as_used)
{
	INT8 const quote_file_idx = FindCivQuoteFileIndex(x, y, z);
	if (quote_file_idx == -1) return -1; // no hints for this sector

	NPCQuoteInfo* const quotes = EnsureCivQuoteFileLoaded(quote_file_idx);
	if (!quotes) return -1; // error

	for (INT8 i = 0; i != NUM_NPC_QUOTE_RECORDS; ++i)
	{
		if (!NPCConsiderQuote(NO_PROFILE, NO_PROFILE, APPROACH_NONE, i, 0, quotes)) continue;
		NPCQuoteInfo& q = quotes[i];

		if (set_as_used)
		{
			TURN_FLAG_ON(q.fFlags, QUOTE_FLAG_SAID);
		}

		if (q.ubStartQuest != NO_QUEST)
		{
			StartQuest(q.ubStartQuest, gWorldSectorX, gWorldSectorY);
		}

		return q.ubQuoteNum;
	}

	return -1;
}
