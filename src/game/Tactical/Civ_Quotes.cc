#include "Civ_Quotes.h"
#include "Directories.h"
#include "Font_Control.h"
#include "Local.h"
#include "MouseSystem.h"
#include "Soldier_Find.h"
#include "StrategicMap.h"
#include "Timer_Control.h"
#include "MessageBoxScreen.h"
#include "Queen_Command.h"
#include "Overhead.h"
#include "AI.h"
#include "Render_Dirty.h"
#include "MercTextBox.h"
#include "Text.h"
#include "ScreenIDs.h"
#include "Animation_Data.h"
#include "Video.h"
#include "Message.h"
#include "RenderWorld.h"
#include "Cursors.h"
#include "Dialogue_Control.h"
#include "Quests.h"
#include "MapScreen.h"
#include "Strategic_Town_Loyalty.h"
#include "NPC.h"
#include "Strategic_Mines.h"
#include "Random.h"
#include "FileMan.h"
#include "UILayout.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>
#include <string_theory/string>


#define CIV_QUOTE_TEXT_SIZE		160


#define DIALOGUE_DEFAULT_WIDTH		200
#define EXTREAMLY_LOW_TOWN_LOYALTY	20
#define HIGH_TOWN_LOYALTY		80
#define CIV_QUOTE_HINT			99


static UINT8 const gubNumEntries[NUM_CIV_QUOTES] =
{
	15,
	15,
	15,
	15,
	15,
	15,
	15,
	15,
	15,
	15,

	15,
	15,
	15,
	15,
	15,
	15,
	15,
	15,
	15,
	15,

	5,
	5,
	15,
	15,
	15,
	15,
	15,
	15,
	15,
	15,

	15,
	15,
	2,
	15,
	15,
	10,
	10,
	5,
	3,
	10,

	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3
};


struct QUOTE_SYSTEM_STRUCT
{
	BOOLEAN bActive;
	MOUSE_REGION MouseRegion;
	VIDEO_OVERLAY *video_overlay;
	MercPopUpBox *dialogue_box;
	UINT32 uiTimeOfCreation;
	UINT32 uiDelayTime;
	SOLDIERTYPE *pCiv;
};


static QUOTE_SYSTEM_STRUCT gCivQuoteData;

static UINT16 gusCivQuoteBoxWidth;
static UINT16 gusCivQuoteBoxHeight;


static BOOLEAN GetCivQuoteText(UINT8 ubCivQuoteID, UINT8 ubEntryID, ST::string& zQuote)
try
{
	char zFileName[164];

	// Build filename....
	if ( ubCivQuoteID == CIV_QUOTE_HINT )
	{
		if (gWorldSector.z > 0)
		{
			sprintf(zFileName, NPCDATADIR "/civ%02d.edt", CIV_QUOTE_MINERS_NOT_FOR_PLAYER);
		}
		else
		{
			sprintf(zFileName, NPCDATADIR "/%c%d.edt", 'a' + (gWorldSector.y - 1) , gWorldSector.x);
		}
	}
	else
	{
		sprintf(zFileName, NPCDATADIR "/civ%02d.edt", ubCivQuoteID);
	}

	zQuote = GCM->loadEncryptedString(zFileName, CIV_QUOTE_TEXT_SIZE * ubEntryID, CIV_QUOTE_TEXT_SIZE);
	return !zQuote.empty();
}
catch (...) { return FALSE; }


static void SurrenderMessageBoxCallBack(MessageBoxReturnValue const ubExitValue)
{
	if ( ubExitValue == MSG_BOX_RETURN_YES )
	{
		// CJC Dec 1 2002: fix multiple captures
		BeginCaptureSquence();

		// Do capture....
		FOR_EACH_IN_TEAM(i, OUR_TEAM)
		{
			SOLDIERTYPE& s = *i;
			if (!s.bInSector) continue;
			if (s.bLife == 0) continue;
			EnemyCapturesPlayerSoldier(&s);
			RemoveSoldierFromTacticalSector(s);
		}

		EndCaptureSequence( );

		gfSurrendered = TRUE;
		SetCustomizableTimerCallbackAndDelay( 3000, CaptureTimerCallback, FALSE );

		ActionDone( gCivQuoteData.pCiv );
	}
	else
	{
		ActionDone( gCivQuoteData.pCiv );
	}
}


static void ShutDownQuoteBox(BOOLEAN fForce)
{
	if ( !gCivQuoteData.bActive )
	{
		return;
	}

	// Check for min time....
	if ( ( GetJA2Clock( ) - gCivQuoteData.uiTimeOfCreation ) > 300 || fForce )
	{
		RemoveVideoOverlay(gCivQuoteData.video_overlay);

		// Remove mouse region...
		MSYS_RemoveRegion( &(gCivQuoteData.MouseRegion) );

		RemoveMercPopupBox(gCivQuoteData.dialogue_box);
		gCivQuoteData.dialogue_box = 0;
		gCivQuoteData.bActive      = FALSE;

		// do we need to do anything at the end of the civ quote?
		if ( gCivQuoteData.pCiv && gCivQuoteData.pCiv->bAction == AI_ACTION_OFFER_SURRENDER )
		{
			DoMessageBox(MSG_BOX_BASIC_STYLE, g_langRes->Message[STR_SURRENDER], GAME_SCREEN, MSG_BOX_FLAG_YESNO, SurrenderMessageBoxCallBack, NULL);
		}
	}
}

BOOLEAN ShutDownQuoteBoxIfActive( )
{
	if ( gCivQuoteData.bActive )
	{
		ShutDownQuoteBox( TRUE );

		return( TRUE );
	}

	return( FALSE );
}


INT8 GetCivType(const SOLDIERTYPE* pCiv)
{
	if ( pCiv->ubProfile != NO_PROFILE )
	{
		return( CIV_TYPE_NA );
	}

	// ATE: Check if this person is married.....
	// 1 ) check sector....
	static const SGPSector marriedSector(10, 6);
	if (gWorldSector == marriedSector)
	{
		// 2 ) the only female....
		if ( pCiv->ubCivilianGroup == 0 && pCiv->bTeam != OUR_TEAM && pCiv->ubBodyType == REGFEMALE )
		{
			// She's a ho!
			return( CIV_TYPE_MARRIED_PC );
		}
	}

	// OK, look for enemy type - MUST be on enemy team, merc bodytype
	if ( pCiv->bTeam == ENEMY_TEAM && IS_MERC_BODY_TYPE( pCiv ) )
	{
		return( CIV_TYPE_ENEMY );
	}

	if ( pCiv->bTeam != CIV_TEAM && pCiv->bTeam != MILITIA_TEAM )
	{
		return( CIV_TYPE_NA );
	}

	switch( pCiv->ubBodyType )
	{
		case REGMALE:
		case BIGMALE:
		case STOCKYMALE:
		case REGFEMALE:
		case FATCIV:
		case MANCIV:
		case MINICIV:
		case DRESSCIV:
		case CRIPPLECIV:
			return( CIV_TYPE_ADULT );

		case ADULTFEMALEMONSTER:
		case AM_MONSTER:
		case YAF_MONSTER:
		case YAM_MONSTER:
		case LARVAE_MONSTER:
		case INFANT_MONSTER:
		case QUEENMONSTER:
			return( CIV_TYPE_NA );

		case HATKIDCIV:
		case KIDCIV:
			return( CIV_TYPE_KID );

		default:
			return( CIV_TYPE_NA );
	}
}


static void RenderCivQuoteBoxOverlay(VIDEO_OVERLAY* pBlitter)
{
	if (gCivQuoteData.video_overlay == NULL)
		return;
	RenderMercPopUpBox(gCivQuoteData.dialogue_box, pBlitter->sX, pBlitter->sY,  pBlitter->uiDestBuff);
	InvalidateRegion(pBlitter->sX, pBlitter->sY, pBlitter->sX + gusCivQuoteBoxWidth,
				pBlitter->sY + gusCivQuoteBoxHeight);
}


static void QuoteOverlayClickCallback(MOUSE_REGION* pRegion, UINT32 iReason)
{
	static BOOLEAN fLButtonDown = FALSE;

	if (iReason & MSYS_CALLBACK_POINTER_DWN )
	{
		fLButtonDown = TRUE;
	}

	if (iReason & MSYS_CALLBACK_POINTER_UP && fLButtonDown )
	{
		// Shutdown quote box....
		ShutDownQuoteBox( FALSE );
	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		fLButtonDown = FALSE;
	}
}


void BeginCivQuote( SOLDIERTYPE *pCiv, UINT8 ubCivQuoteID, UINT8 ubEntryID, INT16 sX, INT16 sY )
{
	// OK, do we have another on?
	if ( gCivQuoteData.bActive )
	{
		// Delete?
		ShutDownQuoteBox( TRUE );
	}

	// get text
	ST::string zQuote;
	if ( !GetCivQuoteText( ubCivQuoteID, ubEntryID, zQuote ) )
	{
		return;
	}

	ST::string gzCivQuote;
	gzCivQuote = ST::format("\"{}\"", zQuote);

	if ( ubCivQuoteID == CIV_QUOTE_HINT )
	{
		MapScreenMessage( FONT_MCOLOR_WHITE, MSG_DIALOG, ST::format("{}", gzCivQuote) );
	}

	// Prepare text box
	gCivQuoteData.dialogue_box = PrepareMercPopupBox(0, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, gzCivQuote, DIALOGUE_DEFAULT_WIDTH, 0, 0, 0, &gusCivQuoteBoxWidth, &gusCivQuoteBoxHeight);

	// OK, find center for box......
	sX = sX - ( gusCivQuoteBoxWidth / 2 );
	sY = sY - ( gusCivQuoteBoxHeight / 2 );

	// OK, limit to screen......
	{
		if ( sX < 0 )
		{
			sX = 0;
		}

		// CHECK FOR LEFT/RIGHT
		if (sX + gusCivQuoteBoxWidth > SCREEN_WIDTH)
		{
			sX = SCREEN_WIDTH - gusCivQuoteBoxWidth;
		}

		// Now check for top
		if ( sY < gsVIEWPORT_WINDOW_START_Y )
		{
			sY = gsVIEWPORT_WINDOW_START_Y;
		}

		// Check for bottom
		sY = std::min(int(sY), gsVIEWPORT_WINDOW_END_Y - gusCivQuoteBoxHeight);
	}

	UINT16 const w = gusCivQuoteBoxWidth;
	UINT16 const h = gusCivQuoteBoxHeight;

	gCivQuoteData.video_overlay = RegisterVideoOverlay(RenderCivQuoteBoxOverlay, sX, sY, w, h);

	//Define main region
	MSYS_DefineRegion(&gCivQuoteData.MouseRegion, sX, sY, sX + w, sY + h, MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL, MSYS_NO_CALLBACK, QuoteOverlayClickCallback);

	gCivQuoteData.bActive = TRUE;

	gCivQuoteData.uiTimeOfCreation = GetJA2Clock( );

	gCivQuoteData.uiDelayTime = FindDelayForString( gzCivQuote ) + 500;

	gCivQuoteData.pCiv = pCiv;

}


static UINT8 DetermineCivQuoteEntry(SOLDIERTYPE* pCiv, UINT8* pubCivHintToUse, BOOLEAN fCanUseHints)
{
	UINT8   ubCivType;
	BOOLEAN bCivLowLoyalty = FALSE;
	BOOLEAN bCivHighLoyalty = FALSE;
	INT8    bCivHint;
	BOOLEAN bMiners = FALSE;

	(*pubCivHintToUse) = 0;

	ubCivType = GetCivType( pCiv );

	if ( ubCivType == CIV_TYPE_ENEMY )
	{
		// Determine what type of quote to say...
		// Are are we going to attack?

		if ( pCiv->bAction == AI_ACTION_TOSS_PROJECTILE || pCiv->bAction == AI_ACTION_FIRE_GUN ||
			pCiv->bAction == AI_ACTION_FIRE_GUN || pCiv->bAction == AI_ACTION_KNIFE_MOVE )
		{
			return( CIV_QUOTE_ENEMY_THREAT );
		}
		else if ( pCiv->bAction == AI_ACTION_OFFER_SURRENDER )
		{
			return( CIV_QUOTE_ENEMY_OFFER_SURRENDER );
		}
		// Hurt?
		else if ( pCiv->bLife < 30 )
		{
			return( CIV_QUOTE_ENEMY_HURT );
		}
		// elite?
		else if ( pCiv->ubSoldierClass == SOLDIER_CLASS_ELITE )
		{
			return( CIV_QUOTE_ENEMY_ELITE );
		}
		else
		{
			return( CIV_QUOTE_ENEMY_ADMIN );
		}
	}

	// Are we in a town sector?
	UINT8 const bTownId = GetTownIdForSector(gWorldSector);

	// If a married PC...
	if ( ubCivType == CIV_TYPE_MARRIED_PC )
	{
		return( CIV_QUOTE_PC_MARRIED );
	}

	// CIV GROUPS FIRST!
	// Hicks.....
	if ( pCiv->ubCivilianGroup == HICKS_CIV_GROUP )
	{
		// Are they friendly?
		//if ( gTacticalStatus.fCivGroupHostile[ HICKS_CIV_GROUP ] < CIV_GROUP_WILL_BECOME_HOSTILE )
		if ( pCiv->bNeutral )
		{
			return( CIV_QUOTE_HICKS_FRIENDLY );
		}
		else
		{
			return( CIV_QUOTE_HICKS_ENEMIES );
		}
	}

	// Goons.....
	if ( pCiv->ubCivilianGroup == KINGPIN_CIV_GROUP )
	{
		// Are they friendly?
		//if ( gTacticalStatus.fCivGroupHostile[ KINGPIN_CIV_GROUP ] < CIV_GROUP_WILL_BECOME_HOSTILE )
		if ( pCiv->bNeutral )
		{
			return( CIV_QUOTE_GOONS_FRIENDLY );
		}
		else
		{
			return( CIV_QUOTE_GOONS_ENEMIES );
		}
	}

	// ATE: Cowering people take precedence....
	if ( ( pCiv->uiStatusFlags & SOLDIER_COWERING ) || ( pCiv->bTeam == CIV_TEAM && ( gTacticalStatus.uiFlags & INCOMBAT ) ) )
	{
		if ( ubCivType == CIV_TYPE_ADULT )
		{
			return( CIV_QUOTE_ADULTS_COWER );
		}
		else
		{
			return( CIV_QUOTE_KIDS_COWER );
		}
	}

	// Kid slaves...
	if ( pCiv->ubCivilianGroup == FACTORY_KIDS_GROUP )
	{
		// Check fact.....
		if ( CheckFact( FACT_DOREEN_HAD_CHANGE_OF_HEART, 0 ) || !CheckFact( FACT_DOREEN_ALIVE, 0 ) )
		{
			return( CIV_QUOTE_KID_SLAVES_FREE );
		}
		else
		{
			return( CIV_QUOTE_KID_SLAVES );
		}
	}

	// BEGGERS
	if ( pCiv->ubCivilianGroup == BEGGARS_CIV_GROUP )
	{
		// Check if we are in a town...
		if (bTownId != BLANK_SECTOR && gWorldSector.z == 0)
		{
			if ( bTownId == SAN_MONA && ubCivType == CIV_TYPE_ADULT )
			{
				return( CIV_QUOTE_SAN_MONA_BEGGERS );
			}
		}

		// DO normal beggers...
		if ( ubCivType == CIV_TYPE_ADULT )
		{
			return( CIV_QUOTE_ADULTS_BEGGING );
		}
		else
		{
			return( CIV_QUOTE_KIDS_BEGGING );
		}
	}

	// REBELS
	if ( pCiv->ubCivilianGroup == REBEL_CIV_GROUP )
	{
		// DO normal beggers...
		if ( ubCivType == CIV_TYPE_ADULT )
		{
			return( CIV_QUOTE_ADULTS_REBELS );
		}
		else
		{
			return( CIV_QUOTE_KIDS_REBELS );
		}
	}

	// Do miltitia...
	if ( pCiv->bTeam == MILITIA_TEAM )
	{
		// Different types....
		if ( pCiv->ubSoldierClass == SOLDIER_CLASS_GREEN_MILITIA )
		{
			return( CIV_QUOTE_GREEN_MILITIA );
		}
		if ( pCiv->ubSoldierClass == SOLDIER_CLASS_REG_MILITIA )
		{
			return( CIV_QUOTE_MEDIUM_MILITIA );
		}
		if ( pCiv->ubSoldierClass == SOLDIER_CLASS_ELITE_MILITIA )
		{
			return( CIV_QUOTE_ELITE_MILITIA );
		}
	}

	// If we are in medunna, and queen is dead, use these...
	if ( bTownId == MEDUNA && CheckFact( FACT_QUEEN_DEAD, 0 ) )
	{
		return( CIV_QUOTE_DEIDRANNA_DEAD );
	}

	// if in a town
	if (bTownId != BLANK_SECTOR && gWorldSector.z == 0 && gfTownUsesLoyalty[bTownId])
	{
		// Check loyalty special quotes.....
		// EXTREMELY LOW TOWN LOYALTY...
		if ( gTownLoyalty[ bTownId ].ubRating < EXTREAMLY_LOW_TOWN_LOYALTY )
		{
			bCivLowLoyalty = TRUE;
		}

		// HIGH TOWN LOYALTY...
		if ( gTownLoyalty[ bTownId ].ubRating >= HIGH_TOWN_LOYALTY )
		{
			bCivHighLoyalty = TRUE;
		}
	}


	// ATE: OK, check if we should look for a civ hint....
	if ( fCanUseHints )
	{
		bCivHint = ConsiderCivilianQuotes(gWorldSector,  FALSE);
	}
	else
	{
		bCivHint = -1;
	}

	// ATE: check miners......
	if ( pCiv->ubSoldierClass == SOLDIER_CLASS_MINER )
	{
		bMiners = TRUE;

		// If not a civ hint available...
		if ( bCivHint == -1 )
		{
			// Check if they are under our control...

			// Should I go talk to miner?
			// Not done yet.

			// Are they working for us?
			INT8 const bMineId = GetIdOfMineForSector(gWorldSector);
			Assert(bMineId >= 0);

			if ( PlayerControlsMine( bMineId ) )
			{
				return( CIV_QUOTE_MINERS_FOR_PLAYER );
			}
			else
			{
				return( CIV_QUOTE_MINERS_NOT_FOR_PLAYER );
			}
		}
	}


	// Is one availible?
	// If we are to say low loyalty, do chance
	if ( bCivHint != -1 && bCivLowLoyalty && !bMiners )
	{
		if ( Random( 100 ) < 25 )
		{
			// Get rid of hint...
			bCivHint = -1;
		}
	}

	// Say hint if availible...
	if ( bCivHint != -1 )
	{
		if ( ubCivType == CIV_TYPE_ADULT )
		{
			(*pubCivHintToUse) = bCivHint;

			// Set quote as used...
			ConsiderCivilianQuotes(gWorldSector, TRUE);

			// retrun value....
			return( CIV_QUOTE_HINT );
		}
	}

	if ( bCivLowLoyalty )
	{
		if ( ubCivType == CIV_TYPE_ADULT )
		{
			return( CIV_QUOTE_ADULTS_EXTREMLY_LOW_LOYALTY );
		}
		else
		{
			return( CIV_QUOTE_KIDS_EXTREMLY_LOW_LOYALTY );
		}
	}

	if ( bCivHighLoyalty )
	{
		if ( ubCivType == CIV_TYPE_ADULT )
		{
			return( CIV_QUOTE_ADULTS_HIGH_LOYALTY );
		}
		else
		{
			return( CIV_QUOTE_KIDS_HIGH_LOYALTY );
		}
	}


	// All purpose quote here....
	if ( ubCivType == CIV_TYPE_ADULT )
	{
		return( CIV_QUOTE_ADULTS_ALL_PURPOSE );
	}
	else
	{
		return( CIV_QUOTE_KIDS_ALL_PURPOSE );
	}

}


void HandleCivQuote( )
{
	if ( gCivQuoteData.bActive )
	{
		// Check for min time....
		if ( ( GetJA2Clock( ) - gCivQuoteData.uiTimeOfCreation ) > gCivQuoteData.uiDelayTime )
		{
			// Stop!
			ShutDownQuoteBox( TRUE );
		}
	}
}

void StartCivQuote( SOLDIERTYPE *pCiv )
{
	UINT8 ubCivQuoteID;
	INT16 sX, sY;
	UINT8 ubEntryID = 0;
	INT16 sScreenX, sScreenY;
	UINT8 ubCivHintToUse;

	// ATE: Check for old quote.....
	// This could have been stored on last attempt...
	if ( pCiv->bCurrentCivQuote == CIV_QUOTE_HINT )
	{
		// Determine which quote to say.....
		// CAN'T USE HINTS, since we just did one...
		pCiv->bCurrentCivQuote = -1;
		pCiv->bCurrentCivQuoteDelta = 0;
		ubCivQuoteID = DetermineCivQuoteEntry( pCiv, &ubCivHintToUse, FALSE );
	}
	else
	{
		// Determine which quote to say.....
		ubCivQuoteID = DetermineCivQuoteEntry( pCiv, &ubCivHintToUse, TRUE );
	}

	// Determine entry id
	// ATE: Try and get entry from soldier pointer....
	if ( ubCivQuoteID != CIV_QUOTE_HINT )
	{
		if ( pCiv->bCurrentCivQuote == -1 )
		{
			// Pick random one
			pCiv->bCurrentCivQuote      = (INT8)Random(gubNumEntries[ubCivQuoteID] - 2);
			pCiv->bCurrentCivQuoteDelta = 0;
		}

		ubEntryID = pCiv->bCurrentCivQuote + pCiv->bCurrentCivQuoteDelta;
	}
	else
	{
		ubEntryID = ubCivHintToUse;

		// ATE: set value for quote ID.....
		pCiv->bCurrentCivQuote = ubCivQuoteID;
		pCiv->bCurrentCivQuoteDelta = ubEntryID;

	}

	// Determine location...
	// Get location of civ on screen.....
	GetSoldierScreenPos( pCiv, &sScreenX, &sScreenY );
	sX = sScreenX;
	sY = sScreenY;

	// begin quote
	BeginCivQuote( pCiv, ubCivQuoteID, ubEntryID, sX, sY );

	// Increment use
	if ( ubCivQuoteID != CIV_QUOTE_HINT )
	{
		pCiv->bCurrentCivQuoteDelta++;

		if ( pCiv->bCurrentCivQuoteDelta == 2 )
		{
			pCiv->bCurrentCivQuoteDelta = 0;
		}
	}
}


void InitCivQuoteSystem( )
{
	gCivQuoteData = QUOTE_SYSTEM_STRUCT{};
	gCivQuoteData.bActive = FALSE;
	gCivQuoteData.video_overlay = NULL;
	gCivQuoteData.dialogue_box  = 0;
}


void SaveCivQuotesToSaveGameFile(HWFILE const f)
{
	f->seek(NUM_CIV_QUOTES * 2, FILE_SEEK_FROM_CURRENT);
}


void LoadCivQuotesFromLoadGameFile(HWFILE const f)
{
	f->seek(NUM_CIV_QUOTES * 2, FILE_SEEK_FROM_CURRENT);
}
