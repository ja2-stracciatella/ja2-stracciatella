#include "Campaign_Types.h"
#include "CharProfile.h"
#include "Directories.h"
#include "Font.h"
#include "IMPVideoObjects.h"
#include "LoadSaveMercProfile.h"
#include "Merc_Hiring.h"
#include "Text.h"
#include "Render_Dirty.h"
#include "Cursors.h"
#include "Laptop.h"
#include "IMP_Compile_Character.h"
#include "IMP_Text_System.h"
#include "IMP_Confirm.h"
#include "Items.h"
#include "Finances.h"
#include "Soldier_Profile.h"
#include "Soldier_Profile_Type.h"
#include "Soldier_Control.h"
#include "IMP_Portraits.h"
#include "Overhead.h"
#include "History.h"
#include "Game_Clock.h"
#include "Game_Event_Hook.h"
#include "LaptopSave.h"
#include "SaveLoadGame.h"
#include "Strategic.h"
#include "Random.h"
#include "Button_System.h"
#include "Font_Control.h"
#include "FileMan.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "policy/GamePolicy.h"
#include "policy/IMPPolicy.h"
#include "Logger.h"

#include <string_theory/string>


static BUTTON_PICS* giIMPConfirmButtonImage[2];
GUIButtonRef giIMPConfirmButton[2];


struct FacePosInfo
{
	UINT8 eye_x;
	UINT8 eye_y;
	UINT8 mouth_x;
	UINT8 mouth_y;
};

static const FacePosInfo g_face_info[] =
{
	{  8,  5,  8, 21 },
	{  9,  4,  9, 23 },
	{  8,  5,  7, 24 },
	{  6,  6,  7, 25 },
	{ 13,  5, 11, 23 },
	{ 11,  5, 10, 24 },
	{  8,  4,  8, 24 },
	{  8,  4,  8, 24 },
	{  4,  4,  5, 25 },
	{  5,  5,  6, 24 },
	{  7,  5,  7, 24 },
	{  5,  7,  6, 26 },
	{  7,  6,  7, 24 },
	{ 11,  5,  9, 23 },
	{  8,  5,  7, 24 },
	{  5,  6,  5, 26 }
};


static void BtnIMPConfirmNo(GUI_BUTTON *btn, INT32 reason);
static void BtnIMPConfirmYes(GUI_BUTTON *btn, INT32 reason);


static void CreateConfirmButtons(void);


void EnterIMPConfirm( void )
{
	// create buttons
	CreateConfirmButtons( );
}

void RenderIMPConfirm( void )
{

	// the background
	RenderProfileBackGround( );

	// indent
	RenderAvgMercIndentFrame(90, 40 );

	// highlight answer
	PrintImpText( );
}


static void DestroyConfirmButtons(void);


void ExitIMPConfirm( void )
{
	// destroy buttons
	DestroyConfirmButtons( );
}

void HandleIMPConfirm( void )
{
}


static void MakeButton(UINT idx, const ST::string& text, INT16 y, GUI_CALLBACK click)
{
	BUTTON_PICS* const img = LoadButtonImage(LAPTOPDIR "/button_2.sti", 0, 1);
	giIMPConfirmButtonImage[idx] = img;
	const INT16 text_col   = FONT_WHITE;
	const INT16 shadow_col = DEFAULT_SHADOW;
	GUIButtonRef const btn = CreateIconAndTextButton(img, text, FONT12ARIAL, text_col, shadow_col, text_col, shadow_col, LAPTOP_SCREEN_UL_X + 136, y, MSYS_PRIORITY_HIGH, click);
	giIMPConfirmButton[idx] = btn;
	btn->SetCursor(CURSOR_WWW);
}


static void CreateConfirmButtons(void)
{
	// create buttons for confirm screen
	const INT16 dy = LAPTOP_SCREEN_WEB_UL_Y;
	MakeButton(0, pImpButtonText[16], dy + 254, BtnIMPConfirmYes);
	MakeButton(1, pImpButtonText[17], dy + 314, BtnIMPConfirmNo);
}


static void DestroyConfirmButtons(void)
{
	// destroy buttons for confirm screen

	RemoveButton(giIMPConfirmButton[ 0 ] );
	UnloadButtonImage(giIMPConfirmButtonImage[ 0 ] );


	RemoveButton(giIMPConfirmButton[ 1 ] );
	UnloadButtonImage(giIMPConfirmButtonImage[ 1 ] );
}


static void GiveItemsToPC(UINT8 ubProfileId);


static BOOLEAN AddCharacterToPlayersTeam(void)
{
	MERC_HIRE_STRUCT HireMercStruct;


	// last minute change to make sure merc with right face has not only the right body, but body specific skills...
	// ie. small mercs have martial arts, but big guys and women don't
	if (!fLoadingCharacterForPreviousImpProfile)
	{
		HandleMercStatsForChangesInFace();
	}

	HireMercStruct = MERC_HIRE_STRUCT{};

	HireMercStruct.ubProfileID = ( UINT8 )( PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ) ;

	if (!fLoadingCharacterForPreviousImpProfile)
	{
		// give them items
		GiveItemsToPC( 	HireMercStruct.ubProfileID );
	}

	HireMercStruct.bWhatKindOfMerc = MERC_TYPE__PLAYER_CHARACTER;

	HireMercStruct.sSector = SGPSector(g_merc_arrive_sector);
	HireMercStruct.fUseLandingZoneForArrival = TRUE;

	HireMercStruct.fCopyProfileItemsOver = TRUE;

	// indefinite contract length
	HireMercStruct.iTotalContractLength = -1;

	HireMercStruct.ubInsertionCode	= INSERTION_CODE_ARRIVING_GAME;
	HireMercStruct.uiTimeTillMercArrives = GetMercArrivalTimeOfDay( );

	const FacePosInfo* const fi = &g_face_info[iPortraitNumber];
	SetProfileFaceData(HireMercStruct.ubProfileID, 200 + iPortraitNumber, fi->eye_x, fi->eye_y, fi->mouth_x, fi->mouth_y);

	//if we succesfully hired the merc
	if (!HireMerc(HireMercStruct))
	{
		return(FALSE);
	}
	else
	{
		return ( TRUE );
	}
}

static void BtnIMPConfirmYes(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (LaptopSaveInfo.fIMPCompletedFlag)
		{
			// already here, leave
			return;
		}

		if (LaptopSaveInfo.iCurrentBalance < COST_OF_PROFILE)
		{
			// not enough
			return;
		}

		// line moved by CJC Nov 28 2002 to AFTER the check for money
		LaptopSaveInfo.fIMPCompletedFlag = AddCharacterToPlayersTeam();
		if (!LaptopSaveInfo.fIMPCompletedFlag) return; // only if merc hiring failed: no charge, give it another go

		SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId);
		if (!pSoldier) return;

		if (fLoadingCharacterForPreviousImpProfile && gamepolicy(imp_load_keep_inventory))
		{
			IMPSavedProfileLoadInventory(gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].zNickname, pSoldier);
			// re-add letter, since it just got wiped and almost certainly is not present in the import
			if (pSoldier->ubID == 0 && FindObj(pSoldier, LETTER) == NO_SLOT) {
				CreateSpecialItem(pSoldier, LETTER);
			}
		}

		// charge the player
		AddTransactionToPlayersBook(IMP_PROFILE, (UINT8)(PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId), GetWorldTotalMin(), -COST_OF_PROFILE);
		AddHistoryToPlayersLog(HISTORY_CHARACTER_GENERATED, 0, GetWorldTotalMin(), SGPSector(-1, -1));

		fButtonPendingFlag = TRUE;
		iCurrentImpPage = IMP_HOME_PAGE;

		// send email notice
		//AddEmail(IMP_EMAIL_PROFILE_RESULTS, IMP_EMAIL_PROFILE_RESULTS_LENGTH, IMP_PROFILE_RESULTS, GetWorldTotalMin());
		AddFutureDayStrategicEvent(EVENT_DAY2_ADD_EMAIL_FROM_IMP, 60 * 7, 0, 2);
		//RenderCharProfile();

		ResetCharacterStats();

		//Display a popup msg box telling the user when and where the merc will arrive
		//DisplayPopUpBoxExplainingMercArrivalLocationAndTime();

		//reset the id of the last merc so we dont get the DisplayPopUpBoxExplainingMercArrivalLocationAndTime() pop up box in another screen by accident
		LaptopSaveInfo.sLastHiredMerc.iIdOfMerc = -1;
	}
}


// fixed? by CJC Nov 28 2002
static void BtnIMPConfirmNo(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		iCurrentImpPage = IMP_FINISH;

#if 0 // XXX was commented out
		LaptopSaveInfo.fIMPCompletedFlag = FALSE;
		ResetCharacterStats();

		fButtonPendingFlag = TRUE;
		iCurrentImpPage = IMP_HOME_PAGE;
#endif
	}
}


static void MakeProfileInvItemAnySlot(MERCPROFILESTRUCT&, UINT16 usItem, UINT8 ubStatus, UINT8 ubHowMany);
static void MakeProfileInvItemThisSlot(MERCPROFILESTRUCT&, UINT32 uiPos, UINT16 usItem, UINT8 ubStatus, UINT8 ubHowMany);


static void GiveItemsToPC(UINT8 ubProfileId)
{
	// gives starting items to merc
	// NOTE: Any guns should probably be from those available in regular gun set

	MERCPROFILESTRUCT& p = GetProfile(ubProfileId);

	for (const ItemModel *item : GCM->getIMPPolicy()->getInventory())
	{
		MakeProfileInvItemAnySlot(p, item->getItemIndex(), 100, 1);
	}

	if ( PreRandom( 100 ) < (UINT32) p.bWisdom )
	{
		MakeProfileInvItemThisSlot(p, HELMETPOS, STEEL_HELMET, 100, 1);
	}

	if (p.bMarksmanship >= 80)
	{
		for (const ItemModel *item : GCM->getIMPPolicy()->getGoodShooterItems())
		{
			MakeProfileInvItemAnySlot(p, item->getItemIndex(), 100, 1);
		}
	}
	else
	{
		for (const ItemModel *item : GCM->getIMPPolicy()->getNormalShooterItems())
		{
			MakeProfileInvItemAnySlot(p, item->getItemIndex(), 100, 1);
		}
	}


	// OPTIONAL EQUIPMENT: depends on skills & special skills

	if (p.bMedical >= 60)
	{
		// strong medics get full medic kit
		MakeProfileInvItemAnySlot(p, MEDICKIT, 100, 1);
	}
	else
	if (p.bMedical >= 30)
	{
		// passable medics get first aid kit
		MakeProfileInvItemAnySlot(p, FIRSTAIDKIT, 100, 1);
	}

	if (p.bMechanical >= 50)
	{
		// mechanics get toolkit
		MakeProfileInvItemAnySlot(p, TOOLKIT, 100, 1);
	}

	if (p.bExplosive >= 50)
	{
		// loonies get TNT & Detonator
		MakeProfileInvItemAnySlot(p, TNT, 100, 1);
		MakeProfileInvItemAnySlot(p, DETONATOR, 100, 1);
	}


	// check for special skills
	if (HasSkillTrait(p, LOCKPICKING) && iMechanical)
	{
		MakeProfileInvItemAnySlot(p, LOCKSMITHKIT, 100, 1);
	}

	if (HasSkillTrait(p, HANDTOHAND))
	{
		MakeProfileInvItemAnySlot(p, BRASS_KNUCKLES, 100, 1);
	}

	if (HasSkillTrait(p, ELECTRONICS) && iMechanical)
	{
		MakeProfileInvItemAnySlot(p, METALDETECTOR, 100, 1);
	}

	if (HasSkillTrait(p, NIGHTOPS))
	{
		MakeProfileInvItemAnySlot(p, BREAK_LIGHT, 100, 2);
	}

	if (HasSkillTrait(p, THROWING))
	{
		MakeProfileInvItemAnySlot(p, THROWING_KNIFE, 100, 1);
	}

	if (HasSkillTrait(p, STEALTHY))
	{
		MakeProfileInvItemAnySlot(p, SILENCER, 100, 1);
	}

	if (HasSkillTrait(p, KNIFING))
	{
		MakeProfileInvItemAnySlot(p, COMBAT_KNIFE, 100, 1);
	}
}


static INT32 FirstFreeBigEnoughPocket(MERCPROFILESTRUCT const&, UINT16 usItem);


static void MakeProfileInvItemAnySlot(MERCPROFILESTRUCT& p, UINT16 const usItem, UINT8 const ubStatus, UINT8 const ubHowMany)
{
	INT32 const iSlot = FirstFreeBigEnoughPocket(p, usItem);
	if (iSlot == -1)
	{
		// no room, item not received
		return;
	}

	// put the item into that slot
	MakeProfileInvItemThisSlot(p, iSlot, usItem, ubStatus, ubHowMany);
}


static void MakeProfileInvItemThisSlot(MERCPROFILESTRUCT& p, UINT32 const uiPos, UINT16 const usItem, UINT8 const ubStatus, UINT8 const ubHowMany)
{
	p.inv[uiPos]        = usItem;
	p.bInvStatus[uiPos] = ubStatus;
	p.bInvNumber[uiPos] = ubHowMany;
}


static INT32 FirstFreeBigEnoughPocket(MERCPROFILESTRUCT const& p, UINT16 const usItem)
{
	UINT32 uiPos;


	// if it fits into a small pocket
	if (GCM->getItem(usItem)->getPerPocket() != 0)
	{
		// check small pockets first
		for (uiPos = SMALLPOCK1POS; uiPos <= SMALLPOCK8POS; uiPos++)
		{
			if (p.inv[uiPos] == NONE)
			{
				return(uiPos);
			}
		}
	}

	// check large pockets
	for (uiPos = BIGPOCK1POS; uiPos <= BIGPOCK4POS; uiPos++)
	{
		if (p.inv[uiPos] == NONE)
		{
			return(uiPos);
		}
	}


	return(-1);
}

void ResetIMPCharactersEyesAndMouthOffsets(const UINT8 ubMercProfileID)
{
	MERCPROFILESTRUCT& p = GetProfile(ubMercProfileID);
	if (p.ubFaceIndex < 200 || p.ubFaceIndex >= 200 + lengthof(g_face_info) || ubMercProfileID >= PROF_HUMMER) return;

	const FacePosInfo* const fi = &g_face_info[p.ubFaceIndex - 200];
	p.usEyesX  = fi->eye_x;
	p.usEyesY  = fi->eye_y;
	p.usMouthX = fi->mouth_x;
	p.usMouthY = fi->mouth_y;
}
