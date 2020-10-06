#include "Directories.h"
#include "Font.h"
#include "HImage.h"
#include "Laptop.h"
#include "Insurance.h"
#include "Insurance_Contract.h"
#include "MercPortrait.h"
#include "Merc_Hiring.h"
#include "MessageBoxScreen.h"
#include "VObject.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Insurance_Text.h"
#include "Soldier_Profile.h"
#include "Overhead.h"
#include "Soldier_Add.h"
#include "Game_Clock.h"
#include "Finances.h"
#include "History.h"
#include "Game_Event_Hook.h"
#include "LaptopSave.h"
#include "Text.h"
#include "Random.h"
#include "Strategic_Status.h"
#include "Assignments.h"
#include "Map_Screen_Interface.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "MemMan.h"
#include "Debug.h"
#include "ScreenIDs.h"
#include "JAScreens.h"
#include "Font_Control.h"
#include "EMail.h"

#include <string_theory/format>
#include <string_theory/string>


#define INS_CTRCT_ORDER_GRID_WIDTH			132
#define INS_CTRCT_ORDER_GRID_HEIGHT			216

#define INS_CTRCT_ORDER_GRID_X				(LAPTOP_SCREEN_UL_X         +  76)
#define INS_CTRCT_ORDER_GRID_Y				(LAPTOP_SCREEN_WEB_UL_Y     + 126)
#define INS_CTRCT_ORDER_GRID_OFFSET_X			(INS_CTRCT_ORDER_GRID_WIDTH +   2)

#define INS_CTRCT_OG_FACE_OFFSET_X			5
#define INS_CTRCT_OG_FACE_OFFSET_Y			4


#define INS_CTRCT_OG_NICK_NAME_OFFSET_X			57
#define INS_CTRCT_OG_NICK_NAME_OFFSET_Y			13

#define INS_CTRCT_OG_HAS_CONTRACT_OFFSET_X		INS_CTRCT_OG_NICK_NAME_OFFSET_X
#define INS_CTRCT_OG_HAS_CONTRACT_OFFSET_Y		INS_CTRCT_OG_NICK_NAME_OFFSET_Y + 13

#define INS_CTRCT_TITLE_Y				(48 + LAPTOP_SCREEN_WEB_UL_Y)//52 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_CTRCT_FIRST_BULLET_TEXT_X			86 + LAPTOP_SCREEN_UL_X
#define INS_CTRCT_FIRST_BULLET_TEXT_Y			65 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_CTRCT_SECOND_BULLET_TEXT_X			INS_CTRCT_FIRST_BULLET_TEXT_X
#define INS_CTRCT_SECOND_BULLET_TEXT_Y			93 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_CTRCT_INTSRUCTION_TEXT_WIDTH		375

#define INS_CTRCT_RED_BAR_UNDER_INSTRUCTION_TEXT_Y	123 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_CTRCT_EMPLYMNT_CNTRCT_TEXT_OFFSET_X		4
#define INS_CTRCT_EMPLYMNT_CNTRCT_TEXT_OFFSET_Y		54

#define INS_CTRCT_LENGTH_OFFSET_X			INS_CTRCT_EMPLYMNT_CNTRCT_TEXT_OFFSET_X
#define INS_CTRCT_LENGTH_OFFSET_Y			71

#define INS_CTRCT_DAYS_REMAINING_OFFSET_Y		87

#define INS_CTRCT_INSURANCE_CNTRCT_OFFSET_Y		108

#define INS_CTRCT_PREMIUM_OWING_OFFSET_Y		160


#define INS_CTRCT_OG_BOX_OFFSET_X			92
#define INS_CTRCT_OG_BOX_WIDTH				35

#define INS_CTRCT_ACCEPT_BTN_X				( 132 / 2 - 43 / 2 ) //6
#define INS_CTRCT_ACCEPT_BTN_Y				193

#define INS_CTRCT_CLEAR_BTN_X				86

#define INS_CTRCT_BOTTON_LINK_Y				351 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_CTRCT_BOTTOM_LINK_RED_BAR_X			171 + LAPTOP_SCREEN_UL_X
#define INS_CTRCT_BOTTON_LINK_RED_BAR_Y			INS_CTRCT_BOTTON_LINK_Y + 41


#define INS_CTRCT_BOTTOM_LINK_RED_BAR_OFFSET		117

#define INS_CTRCT_BOTTOM_LINK_RED_WIDTH			97

#define INS_CTRCT_CONTRACT_STATUS_TEXT_WIDTH		74

// this is the percentage of daily salary used as a base to calculate daily insurance premiums
#define INSURANCE_PREMIUM_RATE				5

#define INS_CTRCT_SKILL_BASE				42
#define INS_CTRCT_FITNESS_BASE				85
#define INS_CTRCT_EXP_LEVEL_BASE			3
#define INS_CTRCT_SURVIVAL_BASE				90


static SGPVObject* guiInsOrderGridImage;
static SGPVObject* guiInsOrderBulletImage;

static UINT8 gubNumberofDisplayedInsuranceGrids;

static BOOLEAN gfChangeInsuranceFormButtons = FALSE;

static SOLDIERTYPE* g_insurance_merc_array[20];
static UINT         g_n_insurable_mercs;
UINT16              gusCurrentInsuranceMercIndex;


//link to the varios pages
static MOUSE_REGION gSelectedInsuranceContractLinkRegion[2];

static BUTTON_PICS* guiInsContractPrevButtonImage;
static void BtnInsContractPrevButtonCallback(GUI_BUTTON *btn, INT32 reason);
static GUIButtonRef guiInsContractPrevBackButton;

static BUTTON_PICS* guiInsContractNextButtonImage;
static void BtnInsContractNextButtonCallBack(GUI_BUTTON *btn, INT32 reason);
static GUIButtonRef guiInsContractNextBackButton;


//Graphic for Accept, Clear button for form 1
static BUTTON_PICS* guiInsuranceAcceptClearFormButtonImage;


struct InsuranceInfo
{
	GUIButtonRef button;
	SOLDIERTYPE* soldier;
};


static InsuranceInfo insurance_info[3];


static void SetNumberOfDisplayedInsuranceMercs(void)
{
	gubNumberofDisplayedInsuranceGrids = MIN(g_n_insurable_mercs - gusCurrentInsuranceMercIndex, 3);
}


static GUIButtonRef MakeButtonBig(BUTTON_PICS* img, const ST::string& text, INT16 x, const GUI_CALLBACK click, INT8 offset_x)
{
	const INT16 text_col   = INS_FONT_COLOR;
	const INT16 shadow_col = INS_FONT_SHADOW;
	GUIButtonRef const btn = CreateIconAndTextButton(img, text, INS_FONT_BIG, text_col, shadow_col, text_col, shadow_col, x, INS_INFO_ARROW_BUTTON_Y, MSYS_PRIORITY_HIGH, click);
	btn->SetCursor(CURSOR_WWW);
	btn->SpecifyTextOffsets(offset_x, 16, FALSE);
	return btn;
}


static void BuildInsuranceArray(void);
static void CreateDestroyInsuranceContractFormButtons(BOOLEAN fCreate);
static void SelectInsuranceContractRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);


void EnterInsuranceContract()
{
	UINT16 usPosX,i;

	//build the list of mercs that are can be displayed
	BuildInsuranceArray();

	SetNumberOfDisplayedInsuranceMercs();

	InitInsuranceDefaults();



	// load the Insurance title graphic and add it
	guiInsOrderGridImage = AddVideoObjectFromFile(LAPTOPDIR "/insordergrid.sti");

	// load the Insurance bullet graphic and add it
	guiInsOrderBulletImage = AddVideoObjectFromFile(LAPTOPDIR "/bullet.sti");


	usPosX = INS_CTRCT_BOTTOM_LINK_RED_BAR_X;
	for(i=0; i<2; i++)
	{
		MSYS_DefineRegion(&gSelectedInsuranceContractLinkRegion[i], usPosX, INS_CTRCT_BOTTON_LINK_RED_BAR_Y-37,
					(UINT16)(usPosX + INS_CTRCT_BOTTOM_LINK_RED_WIDTH),
					INS_CTRCT_BOTTON_LINK_RED_BAR_Y+2, MSYS_PRIORITY_HIGH,
					CURSOR_WWW, MSYS_NO_CALLBACK, SelectInsuranceContractRegionCallBack);
		MSYS_SetRegionUserData( &gSelectedInsuranceContractLinkRegion[i], 0, i );

		usPosX += INS_CTRCT_BOTTOM_LINK_RED_BAR_OFFSET;
	}


	//left arrow
	guiInsContractPrevButtonImage	= LoadButtonImage(LAPTOPDIR "/insleftbutton.sti", 2, 0, -1, 1, -1);
	guiInsContractPrevBackButton  = MakeButtonBig(guiInsContractPrevButtonImage, InsContractText[INS_CONTRACT_PREVIOUS], INS_INFO_LEFT_ARROW_BUTTON_X, BtnInsContractPrevButtonCallback, 17);

	//Right arrow
	guiInsContractNextButtonImage	= LoadButtonImage(LAPTOPDIR "/insrightbutton.sti", 2, 0, -1, 1, -1);
	guiInsContractNextBackButton  = MakeButtonBig(guiInsContractNextButtonImage, InsContractText[INS_CONTRACT_NEXT], INS_INFO_RIGHT_ARROW_BUTTON_X, BtnInsContractNextButtonCallBack, 18);

	//create the new set of buttons
	CreateDestroyInsuranceContractFormButtons( TRUE );

	//RenderInsuranceContract();
}


void ExitInsuranceContract()
{
	UINT8 i;

	RemoveInsuranceDefaults();

	DeleteVideoObject(guiInsOrderGridImage);
	DeleteVideoObject(guiInsOrderBulletImage);

	for(i=0; i<2; i++)
		MSYS_RemoveRegion( &gSelectedInsuranceContractLinkRegion[i]);

	//the previous button
	UnloadButtonImage( guiInsContractPrevButtonImage );
	RemoveButton( guiInsContractPrevBackButton );

	//the next button
	UnloadButtonImage( guiInsContractNextButtonImage );
	RemoveButton( guiInsContractNextBackButton );

	CreateDestroyInsuranceContractFormButtons( FALSE );
}


static void EnableDisableInsuranceContractAcceptButtons(void);


void HandleInsuranceContract()
{
	if( gfChangeInsuranceFormButtons )
	{
		//remove the old buttons from the page
		CreateDestroyInsuranceContractFormButtons( FALSE );

		//Get the new number of displayed insurance grids
		SetNumberOfDisplayedInsuranceMercs();

		//create the new set of buttons
		CreateDestroyInsuranceContractFormButtons( TRUE );

		//reset the flag
		gfChangeInsuranceFormButtons = FALSE;

		//force a redraw of the screen to erase the old buttons
		fPausedReDrawScreenFlag = TRUE;
		RenderInsuranceContract();

		MarkButtonsDirty();
	}

	EnableDisableInsuranceContractAcceptButtons();
}


static BOOLEAN AreAnyAimMercsOnTeam(void);
static void DisableInsuranceContractNextPreviousbuttons(void);
static BOOLEAN DisplayOrderGrid(UINT8 ubGridNumber, SOLDIERTYPE*);
static void InsContractNoMercsPopupCallBack(MessageBoxReturnValue);


void RenderInsuranceContract()
{
	ST::string sText;
	UINT16  usNextMercID;
	UINT16  usPosX;


	SetFontShadow( INS_FONT_SHADOW );

	DisplayInsuranceDefaults();

	//disable the next or previous button depending on how many more mercs we have to display
	DisableInsuranceContractNextPreviousbuttons();

	usPosX = INS_CTRCT_BOTTOM_LINK_RED_BAR_X;

	//Display the red bar under the link at the bottom.  and the text
	DisplaySmallRedLineWithShadow( usPosX, INS_CTRCT_BOTTON_LINK_RED_BAR_Y, (UINT16)(usPosX+INS_CTRCT_BOTTOM_LINK_RED_WIDTH), INS_CTRCT_BOTTON_LINK_RED_BAR_Y);
	DisplayWrappedString(usPosX, INS_CTRCT_BOTTON_LINK_Y + 18, INS_CTRCT_BOTTOM_LINK_RED_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, pMessageStrings[MSG_HOMEPAGE], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	usPosX += INS_CTRCT_BOTTOM_LINK_RED_BAR_OFFSET;

	//Display the red bar under the link at the bottom.  and the text
	DisplaySmallRedLineWithShadow( usPosX, INS_CTRCT_BOTTON_LINK_RED_BAR_Y, (UINT16)(usPosX+INS_CTRCT_BOTTOM_LINK_RED_WIDTH), INS_CTRCT_BOTTON_LINK_RED_BAR_Y);
	sText = GetInsuranceText(INS_SNGL_HOW_DOES_INS_WORK);
	DisplayWrappedString( usPosX, INS_CTRCT_BOTTON_LINK_Y + 12, INS_CTRCT_BOTTOM_LINK_RED_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);


	//Display the title slogan
	sText = GetInsuranceText(INS_SNGL_ENTERING_REVIEWING_CLAIM);
	DrawTextToScreen(sText, LAPTOP_SCREEN_UL_X, INS_CTRCT_TITLE_Y, LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X, INS_FONT_BIG, INS_FONT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	BltVideoObject(FRAME_BUFFER, guiInsOrderBulletImage, 0, INS_CTRCT_FIRST_BULLET_TEXT_X, INS_CTRCT_FIRST_BULLET_TEXT_Y);

	//Display the first instruction sentence
	sText = GetInsuranceText(INS_MLTI_TO_PURCHASE_INSURANCE);
	DisplayWrappedString(INS_CTRCT_FIRST_BULLET_TEXT_X + INSURANCE_BULLET_TEXT_OFFSET_X, INS_CTRCT_FIRST_BULLET_TEXT_Y, INS_CTRCT_INTSRUCTION_TEXT_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	BltVideoObject(FRAME_BUFFER, guiInsOrderBulletImage, 0, INS_CTRCT_FIRST_BULLET_TEXT_X, INS_CTRCT_SECOND_BULLET_TEXT_Y);

	//Display the second instruction sentence
	sText = GetInsuranceText(INS_MLTI_ONCE_SATISFIED_CLICK_ACCEPT);
	DisplayWrappedString(INS_CTRCT_FIRST_BULLET_TEXT_X + INSURANCE_BULLET_TEXT_OFFSET_X, INS_CTRCT_SECOND_BULLET_TEXT_Y, INS_CTRCT_INTSRUCTION_TEXT_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Display the red bar under the instruction text
	DisplaySmallRedLineWithShadow( INS_CTRCT_FIRST_BULLET_TEXT_X, INS_CTRCT_RED_BAR_UNDER_INSTRUCTION_TEXT_Y, INS_CTRCT_FIRST_BULLET_TEXT_X+INS_CTRCT_INTSRUCTION_TEXT_WIDTH, INS_CTRCT_RED_BAR_UNDER_INSTRUCTION_TEXT_Y);


	usNextMercID =	gusCurrentInsuranceMercIndex;
	const UINT count_insurance_grids = gubNumberofDisplayedInsuranceGrids;
	for (UINT i = 0; i < count_insurance_grids; ++i)
	{
		SOLDIERTYPE* const s = g_insurance_merc_array[usNextMercID++];
		DisplayOrderGrid(i, s);
	}

	//if there are no valid mercs to insure
	if (count_insurance_grids == 0)
	{
		//if there where AIM mercs ( on short contract )
		if( AreAnyAimMercsOnTeam( ) )
		{
			//Display Error Message, all aim mercs are on short contract
			sText = GetInsuranceText(INS_MLTI_ALL_AIM_MERCS_ON_SHORT_CONTRACT);
		}
		else
		{
			//Display Error Message, no valid mercs
			sText = GetInsuranceText(INS_MLTI_NO_QUALIFIED_MERCS);
		}
		DoLapTopMessageBox(MSG_BOX_RED_ON_WHITE, sText, LAPTOP_SCREEN, MSG_BOX_FLAG_OK, InsContractNoMercsPopupCallBack);
	}



	SetFontShadow(DEFAULT_SHADOW);
	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void BtnInsContractPrevButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (gusCurrentInsuranceMercIndex > 2) gusCurrentInsuranceMercIndex -= 3;
		// signal that we want to change the number of forms on the page
		gfChangeInsuranceFormButtons = TRUE;
	}
}


static void BtnInsContractNextButtonCallBack(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gusCurrentInsuranceMercIndex += 3;
		// signal that we want to change the number of forms on the page
		gfChangeInsuranceFormButtons = TRUE;
	}
}


static INT32 CalculateSoldiersInsuranceContractLength(const SOLDIERTYPE* pSoldier);
static BOOLEAN CanSoldierExtendInsuranceContract(const SOLDIERTYPE* pSoldier);
static UINT32 GetTimeRemainingOnSoldiersContract(const SOLDIERTYPE* pSoldier);
static UINT32 GetTimeRemainingOnSoldiersInsuranceContract(const SOLDIERTYPE* pSoldier);


static BOOLEAN DisplayOrderGrid(const UINT8 ubGridNumber, SOLDIERTYPE* const pSoldier)
try
{
	INT32   iCostOfContract=0;
	ST::string sText;
	BOOLEAN fDisplayMercContractStateTextColorInRed = FALSE;

	Assert(ubGridNumber < 3);

	insurance_info[ubGridNumber].soldier = pSoldier;

	const INT32 dx = INS_CTRCT_ORDER_GRID_X + INS_CTRCT_ORDER_GRID_OFFSET_X * ubGridNumber;
	const INT32 dy = INS_CTRCT_ORDER_GRID_Y;

	BltVideoObject(FRAME_BUFFER, guiInsOrderGridImage, 0, dx, dy);

	MERCPROFILESTRUCT const& p       = GetProfile(pSoldier->ubProfile);
	bool              const  is_dead = IsMercDead(p);
	{
		// load the mercs face graphic and add it
		AutoSGPVObject uiInsMercFaceImage(LoadSmallPortrait(p));

		//if the merc is dead, shade the face red
		if (is_dead)
		{
			//if the merc is dead
			//shade the face red, (to signify that he is dead)
			uiInsMercFaceImage->pShades[0] = Create16BPPPaletteShaded(uiInsMercFaceImage->Palette(), DEAD_MERC_COLOR_RED, DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE);

			//set the red pallete to the face
			uiInsMercFaceImage->CurrentShade(0);
		}

		//Get and display the mercs face
		BltVideoObject(FRAME_BUFFER, uiInsMercFaceImage.get(), 0, dx + INS_CTRCT_OG_FACE_OFFSET_X, dy + INS_CTRCT_OG_FACE_OFFSET_Y);
	}

	//display the mercs nickname
	DrawTextToScreen(p.zNickname, dx + INS_CTRCT_OG_NICK_NAME_OFFSET_X, dy + INS_CTRCT_OG_NICK_NAME_OFFSET_Y, 0, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Get the text to display the mercs current insurance contract status
	if (is_dead)
	{
		//if the merc has a contract
		if( pSoldier->usLifeInsurance )
		{
			//Display the contract text
			sText = GetInsuranceText(INS_SNGL_DEAD_WITH_CONTRACT);
		}
		else
		{
			//Display the contract text
			sText = GetInsuranceText(INS_SNGL_DEAD_NO_CONTRACT);
		}
		DisplayWrappedString(dx + INS_CTRCT_OG_HAS_CONTRACT_OFFSET_X, dy + INS_CTRCT_OG_HAS_CONTRACT_OFFSET_Y, INS_CTRCT_CONTRACT_STATUS_TEXT_WIDTH, 2, INS_FONT_SMALL, INS_FONT_COLOR_RED, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	}
	else
	{
		//if the merc has a contract
		if( pSoldier->usLifeInsurance )
		{
			//if the soldier can extend their insurance
			if( CanSoldierExtendInsuranceContract( pSoldier ) )
			{
				//Display the contract text
				sText = GetInsuranceText(INS_SNGL_PARTIALLY_INSURED);
				fDisplayMercContractStateTextColorInRed = TRUE;
			}
			else
			{
				//Display the contract text
				sText = GetInsuranceText(INS_SNGL_CONTRACT);
				fDisplayMercContractStateTextColorInRed = FALSE;
			}
		}
		else
		{
			//Display the contract text
			sText = GetInsuranceText(INS_SNGL_NOCONTRACT);
			fDisplayMercContractStateTextColorInRed = TRUE;
		}
		const UINT8 colour = (fDisplayMercContractStateTextColorInRed ? INS_FONT_COLOR_RED : INS_FONT_COLOR);
		DisplayWrappedString(dx + INS_CTRCT_OG_HAS_CONTRACT_OFFSET_X, dy + INS_CTRCT_OG_HAS_CONTRACT_OFFSET_Y, INS_CTRCT_CONTRACT_STATUS_TEXT_WIDTH, 2, INS_FONT_SMALL, colour, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	}



	//Display the Emplyment contract text
	sText = GetInsuranceText(INS_SNGL_EMPLOYMENT_CONTRACT);
	DrawTextToScreen(sText, dx + INS_CTRCT_EMPLYMNT_CNTRCT_TEXT_OFFSET_X, dy + INS_CTRCT_EMPLYMNT_CNTRCT_TEXT_OFFSET_Y, INS_CTRCT_ORDER_GRID_WIDTH, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Display the merc contract Length text
	sText = GetInsuranceText(INS_SNGL_LENGTH);
	DrawTextToScreen(sText, dx + INS_CTRCT_LENGTH_OFFSET_X, dy + INS_CTRCT_LENGTH_OFFSET_Y, 0, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Display the mercs contract length
	sText = ST::format("{}", pSoldier->iTotalContractLength);
	DrawTextToScreen(sText, dx + INS_CTRCT_OG_BOX_OFFSET_X, dy + INS_CTRCT_LENGTH_OFFSET_Y, INS_CTRCT_OG_BOX_WIDTH, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);


	//Display the days remaining for the emplyment contract text
	sText = GetInsuranceText(INS_SNGL_DAYS_REMAINING);
	DrawTextToScreen(sText, dx + INS_CTRCT_LENGTH_OFFSET_X, dy + INS_CTRCT_DAYS_REMAINING_OFFSET_Y, 0, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);


	//display the amount of time the merc has left on their Regular contract
	if (is_dead)
		sText = pMessageStrings[MSG_LOWERCASE_NA];
	else
		sText = ST::format("{}", GetTimeRemainingOnSoldiersContract( pSoldier ));

	DrawTextToScreen(sText, dx + INS_CTRCT_OG_BOX_OFFSET_X, dy + INS_CTRCT_DAYS_REMAINING_OFFSET_Y, INS_CTRCT_OG_BOX_WIDTH, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

	//Display the Insurqance contract
	sText = GetInsuranceText(INS_SNGL_INSURANCE_CONTRACT);
	DrawTextToScreen(sText, dx + INS_CTRCT_EMPLYMNT_CNTRCT_TEXT_OFFSET_X, dy + INS_CTRCT_INSURANCE_CNTRCT_OFFSET_Y, INS_CTRCT_ORDER_GRID_WIDTH, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);


	sText = GetInsuranceText(INS_SNGL_LENGTH);
	DrawTextToScreen(sText, dx + INS_CTRCT_LENGTH_OFFSET_X, dy + INS_CTRCT_LENGTH_OFFSET_Y + 54, 0, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Display the insurance days remaining text
	sText = GetInsuranceText(INS_SNGL_DAYS_REMAINING);
	DrawTextToScreen(sText, dx + INS_CTRCT_LENGTH_OFFSET_X, dy + INS_CTRCT_DAYS_REMAINING_OFFSET_Y + 54, 0, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);


	//
	//display the amount of time the merc has left on the insurance contract
	//

	//if the soldier has insurance, disply the length of time the merc has left
	if (is_dead)
		sText = pMessageStrings[MSG_LOWERCASE_NA];
	else if( pSoldier->usLifeInsurance != 0 )
		sText = ST::format("{}", GetTimeRemainingOnSoldiersInsuranceContract( pSoldier ));

	else
		sText = ST::format("{}", 0);

	DrawTextToScreen(sText, dx + INS_CTRCT_OG_BOX_OFFSET_X, dy + INS_CTRCT_DAYS_REMAINING_OFFSET_Y + 54, INS_CTRCT_OG_BOX_WIDTH, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);


	//
	// Calculate the insurance cost
	//

	//if the soldier can get insurance, calculate a new cost
	if( CanSoldierExtendInsuranceContract( pSoldier ) )
	{
		iCostOfContract =CalculateInsuranceContractCost( CalculateSoldiersInsuranceContractLength( pSoldier ), pSoldier->ubProfile );
	}

	else
	{
		iCostOfContract = 0;
	}

	if( iCostOfContract < 0 )
	{
		//shouldnt get in here now since we can longer give refunds
		SLOGA("DisplayOrderGrid: negative contract cost");
	}
	else
	{
		//Display the premium owing text
		sText = GetInsuranceText(INS_SNGL_PREMIUM_OWING);
		DrawTextToScreen(sText, dx + INS_CTRCT_EMPLYMNT_CNTRCT_TEXT_OFFSET_X, dy + INS_CTRCT_PREMIUM_OWING_OFFSET_Y, INS_CTRCT_ORDER_GRID_WIDTH, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

		//display the amount of refund
		sText = SPrintMoney(iCostOfContract);
	}


	if (is_dead)
	{
		sText = "$0";
	}
	//display the amount owing
	DrawTextToScreen(sText, dx + 32, dy + 179, 72, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);


	//
	// Get the insurance contract length for the merc
	//
	sText = ST::format("{}", CalculateSoldiersInsuranceContractLength( pSoldier ));


	//Display the length of time the player can get for the insurance contract
	DrawTextToScreen(sText, dx + INS_CTRCT_OG_BOX_OFFSET_X, dy + INS_CTRCT_LENGTH_OFFSET_Y + 52 + 2, INS_CTRCT_OG_BOX_WIDTH, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

	return( TRUE );
}
catch (...) { return FALSE; }


static void HandleAcceptButton(SOLDIERTYPE* s);


static void BtnInsuranceAcceptClearFormButtonCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		UINT         const idx = btn->GetUserData();
		SOLDIERTYPE* const s   = insurance_info[idx].soldier;

		HandleAcceptButton(s);

		//redraw the screen
		fPausedReDrawScreenFlag = TRUE;
	}
}


static void SelectInsuranceContractRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		UINT32 uiInsuranceLink = MSYS_GetRegionUserData( pRegion, 0 );

		if( uiInsuranceLink == 0 )
			guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE;
		else if( uiInsuranceLink == 1 )
			guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE_INFO;
	}
}


static void DisableInsuranceContractNextPreviousbuttons(void)
{
	//disable the next button if there is no more mercs to display
	EnableButton(guiInsContractNextBackButton, gusCurrentInsuranceMercIndex + gubNumberofDisplayedInsuranceGrids < g_n_insurable_mercs);
	//if we are currently displaying the first set of mercs, disable the previous button
	EnableButton(guiInsContractPrevBackButton, gusCurrentInsuranceMercIndex >= 3);
}


static void CreateDestroyInsuranceContractFormButtons(BOOLEAN fCreate)
{
	static BOOLEAN	fButtonsCreated = FALSE;

	if( fCreate && !fButtonsCreated )
	{
		//place the 3 accept buttons for the different forms

		//The accept button image
		guiInsuranceAcceptClearFormButtonImage = LoadButtonImage(LAPTOPDIR "/acceptclearbox.sti", 0, 1);

		ST::string text = InsContractText[INS_CONTRACT_ACCEPT];
		const INT16          text_col   = INS_FONT_BTN_COLOR;
		const INT16          shadow_col = INS_FONT_BTN_SHADOW_COLOR;
		const INT16          y          = INS_CTRCT_ORDER_GRID_Y + INS_CTRCT_ACCEPT_BTN_Y;
		for (UINT i = 0; i != gubNumberofDisplayedInsuranceGrids; ++i)
		{
			const INT16 x   = INS_CTRCT_ORDER_GRID_X + INS_CTRCT_ORDER_GRID_OFFSET_X * i + INS_CTRCT_ACCEPT_BTN_X;
			GUIButtonRef const btn = CreateIconAndTextButton(guiInsuranceAcceptClearFormButtonImage, text, INS_FONT_MED, text_col, shadow_col, text_col, shadow_col, x, y, MSYS_PRIORITY_HIGH, BtnInsuranceAcceptClearFormButtonCallback);
			insurance_info[i].button = btn;
			btn->SetCursor(CURSOR_LAPTOP_SCREEN);
			btn->SetUserData(i);
		}

		fButtonsCreated = TRUE;
	}

	if( fButtonsCreated && ! fCreate )
	{
		//the accept image
		UnloadButtonImage(guiInsuranceAcceptClearFormButtonImage);

		for (UINT i = 0; i != gubNumberofDisplayedInsuranceGrids; ++i)
		{
			RemoveButton(insurance_info[i].button);
		}

		fButtonsCreated = FALSE;
	}
}


static void HandleAcceptButton(SOLDIERTYPE* s)
{
	PurchaseOrExtendInsuranceForSoldier(s, CalculateSoldiersInsuranceContractLength(s));

	RenderInsuranceContract();
}


// determines if a merc will run out of their insurance contract
void DailyUpdateOfInsuredMercs()
{
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		//if the merc has life insurance
		if (s->usLifeInsurance)
		{
			//if the merc wasn't just hired
			if ((INT16)GetWorldDay() != s->iStartOfInsuranceContract)
			{
				//if the contract has run out of time
				if (GetTimeRemainingOnSoldiersInsuranceContract(s) <= 0)
				{
					//if the soldier isn't dead
					if (!IsMercDead(GetProfile(s->ubProfile)))
					{
						s->usLifeInsurance = 0;
						s->iTotalLengthOfInsuranceContract = 0;
						s->iStartOfInsuranceContract = 0;
					}
				}
			}
		}
	}
}


#define MIN_INSURANCE_RATIO	0.1f
#define MAX_INSURANCE_RATIO	10.0f


static FLOAT DiffFromNormRatio(INT16 sThisValue, INT16 sNormalValue);


INT32	CalculateInsuranceContractCost( INT32 iLength, UINT8 ubMercID )
{
	INT16  sTotalSkill=0;
	FLOAT  flSkillFactor, flFitnessFactor, flExpFactor, flSurvivalFactor;
	FLOAT  flRiskFactor;
	UINT32 uiDailyInsurancePremium;
	UINT32 uiTotalInsurancePremium;
	const SOLDIERTYPE* const pSoldier = FindSoldierByProfileIDOnPlayerTeam(ubMercID);

	// only mercs with at least 2 days to go on their employment contract are insurable
	// def: 2/5/99.  However if they already have insurance is SHOULD be ok
	if( GetTimeRemainingOnSoldiersContract( pSoldier ) < 2 && !( pSoldier->usLifeInsurance != 0 && GetTimeRemainingOnSoldiersContract( pSoldier ) >= 1 ) )
	{
		return( 0 );
	}

	//If the merc is currently being held captive, get out
	if (pSoldier->bAssignment == ASSIGNMENT_POW)
	{
		return( 0 );
	}

	/*
	replaced with the above check

	if (iLength < 2)
	{
		return(0);
	}*/

	MERCPROFILESTRUCT const& p = GetProfile(ubMercID);

	// calculate the degree of training
	sTotalSkill = (p.bMarksmanship + p.bMedical + p.bMechanical + p.bExplosive + p.bLeadership) / 5;
	flSkillFactor = DiffFromNormRatio( sTotalSkill, INS_CTRCT_SKILL_BASE );

	// calc relative fitness level
	flFitnessFactor = DiffFromNormRatio(p.bLife, INS_CTRCT_FITNESS_BASE);

	// calc relative experience
	flExpFactor = DiffFromNormRatio(p.bExpLevel, INS_CTRCT_EXP_LEVEL_BASE);

	// calc player's survival rate (death rate subtracted from 100)
	flSurvivalFactor = DiffFromNormRatio( (INT16) (100 - CalcDeathRate()), INS_CTRCT_SURVIVAL_BASE );

	// calculate the overall insurability risk factor for this merc by combining all the subfactors
	flRiskFactor = flSkillFactor * flFitnessFactor * flExpFactor * flSurvivalFactor;

	// restrict the overall factor to within reasonable limits
	if (flRiskFactor < MIN_INSURANCE_RATIO)
	{
		flRiskFactor = MIN_INSURANCE_RATIO;
	}
	else
	if (flRiskFactor > MAX_INSURANCE_RATIO)
	{
		flRiskFactor = MAX_INSURANCE_RATIO;
	}

	// premium depend on merc's salary, the base insurance rate, and the individual's risk factor at this time
	uiDailyInsurancePremium = (UINT32)(p.sSalary * INSURANCE_PREMIUM_RATE * flRiskFactor / 100 + 0.5);
	// multiply by the insurance contract length
	uiTotalInsurancePremium = uiDailyInsurancePremium * iLength;

	return( uiTotalInsurancePremium );
}


// values passed in must be such that exceeding the normal value REDUCES insurance premiums
static FLOAT DiffFromNormRatio(INT16 sThisValue, INT16 sNormalValue)
{
	FLOAT flRatio;

	if (sThisValue > 0)
	{
		flRatio = (FLOAT) sNormalValue / sThisValue;

		// restrict each ratio to within a reasonable range
		if (flRatio < MIN_INSURANCE_RATIO)
		{
			flRatio = MIN_INSURANCE_RATIO;
		}
		else
		if (flRatio > MAX_INSURANCE_RATIO)
		{
			flRatio = MAX_INSURANCE_RATIO;
		}
	}
	else
	{
		// use maximum allowable ratio
		flRatio = MAX_INSURANCE_RATIO;
	}

	return( flRatio );
}


static void InsContractNoMercsPopupCallBack(MessageBoxReturnValue const bExitValue)
{
	// yes, so start over, else stay here and do nothing for now
	if( bExitValue == MSG_BOX_RETURN_OK )
	{
		guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE;
	}
}


static BOOLEAN MercIsInsurable(const SOLDIERTYPE*);


static void BuildInsuranceArray(void)
{
	g_n_insurable_mercs = 0;

	// store profile #s of all insurable mercs in an array
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (MercIsInsurable(s))
		{
			g_insurance_merc_array[g_n_insurable_mercs++] = s;
		}
	}
}


void AddLifeInsurancePayout(SOLDIERTYPE* const pSoldier)
{
	UINT8  ubPayoutID;
	UINT32 uiTimeInMinutes;
	UINT32 uiCostPerDay;
	UINT32 uiDaysToPay;


	Assert(pSoldier != NULL);
	Assert(pSoldier->ubProfile != NO_PROFILE);

	MERCPROFILESTRUCT const& p = GetProfile(pSoldier->ubProfile);

	//if we need to add more array elements
	if (LaptopSaveInfo.pLifeInsurancePayouts.size() <= LaptopSaveInfo.ubNumberLifeInsurancePayoutUsed)
	{
		LaptopSaveInfo.pLifeInsurancePayouts.push_back(LIFE_INSURANCE_PAYOUT{});
	}

	for( ubPayoutID = 0; ubPayoutID < LaptopSaveInfo.ubNumberLifeInsurancePayoutUsed; ubPayoutID++ )
	{
		//get an empty element in the array
		if( !LaptopSaveInfo.pLifeInsurancePayouts[ ubPayoutID ].fActive )
			break;
	}

	LIFE_INSURANCE_PAYOUT* const lip = &LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID];
	lip->ubSoldierID = pSoldier->ubID;
	lip->ubMercID    = pSoldier->ubProfile;
	lip->fActive     = TRUE;

	// This uses the merc's latest salaries, ignoring that they may be higher than the salaries paid under the current
	// contract if the guy has recently gained a level.  We could store his daily salary when he was last contracted,
	// and use that, but it still doesn't easily account for the fact that renewing a leveled merc early means that the
	// first part of his contract is under his old salary and the second part is under his new one.  Therefore, I chose
	// to ignore this wrinkle, and let the player awlays get paid out using the higher amount.  ARM

	// figure out which of the 3 salary rates the merc has is the cheapest, and use it to calculate the paid amount, to
	// avoid getting back more than the merc cost if he was on a 2-week contract!

	// start with the daily salary
	uiCostPerDay = p.sSalary;

	// consider weekly salary / day
	if (p.uiWeeklySalary / 7 < uiCostPerDay)
	{
		uiCostPerDay = p.uiWeeklySalary / 7;
	}

	// consider biweekly salary / day
	if (p.uiBiWeeklySalary / 14 < uiCostPerDay)
	{
		uiCostPerDay = p.uiBiWeeklySalary / 14;
	}

	// calculate how many full, insured days of work the merc is going to miss
	uiDaysToPay = pSoldier->iTotalLengthOfInsuranceContract - ( GetWorldDay() + 1 - pSoldier->iStartOfInsuranceContract );

	// calculate & store how much is to be paid out
	lip->iPayOutPrice = uiDaysToPay * uiCostPerDay;

	// 4pm next day
	uiTimeInMinutes = GetMidnightOfFutureDayInMinutes( 1 ) + 16 * 60;

	// if the death was suspicious, or he's already been investigated twice or more
	if (p.ubSuspiciousDeath || (gStrategicStatus.ubInsuranceInvestigationsCnt >= 2))
	{
		// fraud suspected, claim will be investigated first
		AddStrategicEvent( EVENT_INSURANCE_INVESTIGATION_STARTED, uiTimeInMinutes, ubPayoutID );
	}
	else
	{
		// is ok, make a prompt payment
		AddStrategicEvent( EVENT_PAY_LIFE_INSURANCE_FOR_DEAD_MERC, uiTimeInMinutes, ubPayoutID );
	}

	LaptopSaveInfo.ubNumberLifeInsurancePayoutUsed++;
}


void StartInsuranceInvestigation( UINT8	ubPayoutID )
{
	const LIFE_INSURANCE_PAYOUT* const lip = &LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID];
	UINT8 ubDays;

	// send an email telling player an investigation is taking place
	if (gStrategicStatus.ubInsuranceInvestigationsCnt == 0)
	{
		// first offense
		AddEmailWithSpecialData(INSUR_SUSPIC, INSUR_SUSPIC_LENGTH, INSURANCE_COMPANY, GetWorldTotalMin(), lip->iPayOutPrice, lip->ubMercID);
	}
	else
	{
		// subsequent offense
		AddEmailWithSpecialData(INSUR_SUSPIC_2, INSUR_SUSPIC_2_LENGTH, INSURANCE_COMPANY, GetWorldTotalMin(), lip->iPayOutPrice, lip->ubMercID);
	}

	if (GetProfile(lip->ubMercID).ubSuspiciousDeath == VERY_SUSPICIOUS_DEATH)
	{
		// the fact that you tried to cheat them gets realized very quickly. :-)
		ubDays = 1;
	}
	else
	{
		// calculate how many days the investigation will take
		ubDays = (UINT8) (2 + gStrategicStatus.ubInsuranceInvestigationsCnt + Random(3));		// 2-4 days, +1 for every previous investigation
	}

	// post an event to end the investigation that many days in the future (at 4pm)
	AddStrategicEvent( EVENT_INSURANCE_INVESTIGATION_OVER, GetMidnightOfFutureDayInMinutes( ubDays ) + 16 * 60, ubPayoutID );

	// increment counter of all investigations
	gStrategicStatus.ubInsuranceInvestigationsCnt++;
}


void EndInsuranceInvestigation( UINT8	ubPayoutID )
{
	const LIFE_INSURANCE_PAYOUT* const lip = &LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID];
	// send an email telling player the investigation is over
	if (GetProfile(lip->ubMercID).ubSuspiciousDeath == VERY_SUSPICIOUS_DEATH)
	{
		// fraud, no payout!
		AddEmailWithSpecialData(INSUR_1HOUR_FRAUD, INSUR_1HOUR_FRAUD_LENGTH, INSURANCE_COMPANY, GetWorldTotalMin(), lip->iPayOutPrice, lip->ubMercID);
	}
	else
	{
		AddEmailWithSpecialData(INSUR_INVEST_OVER, INSUR_INVEST_OVER_LENGTH, INSURANCE_COMPANY, GetWorldTotalMin(), lip->iPayOutPrice, lip->ubMercID);

		// only now make a payment (immediately)
		InsuranceContractPayLifeInsuranceForDeadMerc( ubPayoutID );
	}
}


void InsuranceContractPayLifeInsuranceForDeadMerc( UINT8 ubPayoutID )
{
	LIFE_INSURANCE_PAYOUT* const lip = &LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID];
	//if the mercs id number is the same what is in the soldier array
	SOLDIERTYPE& s = GetMan(lip->ubSoldierID);
	if (lip->ubSoldierID == s.ubID)
	{
		// and if the soldier is still active ( player hasn't removed carcass yet ), reset insurance flag
		if (s.bActive) s.usLifeInsurance = 0;
	}

	//add transaction to players account
	AddTransactionToPlayersBook(INSURANCE_PAYOUT, lip->ubMercID, GetWorldTotalMin(), lip->iPayOutPrice);

	//add to the history log the fact that the we paid the insurance claim
	AddHistoryToPlayersLog(HISTORY_INSURANCE_CLAIM_PAYOUT, lip->ubMercID, GetWorldTotalMin(), -1, -1);

	//if there WASNT an investigation
	if (GetProfile(lip->ubMercID).ubSuspiciousDeath == 0)
	{
		//Add an email telling the user that he received an insurance payment
		AddEmailWithSpecialData(INSUR_PAYMENT, INSUR_PAYMENT_LENGTH, INSURANCE_COMPANY, GetWorldTotalMin(), lip->iPayOutPrice, lip->ubMercID);
	}

	LaptopSaveInfo.ubNumberLifeInsurancePayoutUsed--;
	lip->fActive = FALSE;
}


//Gets called at the very end of the game
void InsuranceContractEndGameShutDown()
{
	//Free up the memory allocated to the insurance payouts
	LaptopSaveInfo.pLifeInsurancePayouts.clear();
}


static BOOLEAN MercIsInsurable(const SOLDIERTYPE* pSoldier)
{
	// only A.I.M. mercs currently on player's team
	if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
	{
		// with more than one day left on their employment contract are eligible for insurance
		// the second part is because the insurance doesn't pay for any working day already started at time of death
		//if( ( (pSoldier->iEndofContractTime - GetWorldTotalMin()) > 24 * 60) || pSoldier->usLifeInsurance )
		if( CanSoldierExtendInsuranceContract( pSoldier ) || pSoldier->usLifeInsurance )
		{
			// who aren't currently being held POW
			// POWs are also uninsurable - if already insured, that insurance IS valid but no new contracts or extension allowed
			if (pSoldier->bAssignment != ASSIGNMENT_POW)
			{
				return(TRUE);
			}
		}
	}

	return(FALSE);
}


static void EnableDisableInsuranceContractAcceptButtons(void)
{
	for (UINT i = 0; i != gubNumberofDisplayedInsuranceGrids; ++i)
	{
		const InsuranceInfo* const ins = &insurance_info[i];
		const SOLDIERTYPE* const s = ins->soldier;
		if (s == NULL) continue;

		EnableButton(ins->button, CanSoldierExtendInsuranceContract(s));
	}
}


static UINT32 GetTimeRemainingOnSoldiersInsuranceContract(const SOLDIERTYPE* pSoldier)
{
	//if the soldier has life insurance
	if( pSoldier->usLifeInsurance )
	{
		//if the insurance contract hasnt started yet
		if( (INT32)GetWorldDay() < pSoldier->iStartOfInsuranceContract )
			return( pSoldier->iTotalLengthOfInsuranceContract );
		else
			return( ( pSoldier->iTotalLengthOfInsuranceContract - ( GetWorldDay() - pSoldier->iStartOfInsuranceContract ) ) );
	}
	else
		return( 0 );
}


static UINT32 GetTimeRemainingOnSoldiersContract(const SOLDIERTYPE* pSoldier)
{
	INT32 iDayMercLeaves = ( pSoldier->iEndofContractTime / 1440 ) - 1;

	//Since the merc is leaving in the afternoon, we must adjust since the time left would be different if we did the calc
	//at 11:59 or 12:01 ( noon )
	if( pSoldier->iEndofContractTime % 1440 )
		iDayMercLeaves++;

	// Subtract todays day number
	iDayMercLeaves = iDayMercLeaves - GetWorldDay();

	if( iDayMercLeaves > pSoldier->iTotalContractLength )
		iDayMercLeaves = pSoldier->iTotalContractLength;

	return( iDayMercLeaves );
	//return( ( pSoldier->iEndofContractTime - (INT32)GetWorldTotalMin( ) ) / 1440 );
}


static INT32 CalcStartDayOfInsurance(SOLDIERTYPE* pSoldier);


void PurchaseOrExtendInsuranceForSoldier( SOLDIERTYPE *pSoldier, UINT32 uiInsuranceLength )
{
	INT32	iAmountOfMoneyTransfer = -1;

	AssertMsg(pSoldier != NULL, "Soldier pointer is NULL!");

	//if the user doesnt have insruance already,
	if( !(pSoldier->usLifeInsurance ) )
	{
		//specify the start date of the contract
		pSoldier->iStartOfInsuranceContract = CalcStartDayOfInsurance( pSoldier );
		pSoldier->uiStartTimeOfInsuranceContract = GetWorldTotalMin();
	}

	//transfer money
	iAmountOfMoneyTransfer = CalculateInsuranceContractCost( uiInsuranceLength, pSoldier->ubProfile );

	//if the user did have insruance already,
	if( pSoldier->usLifeInsurance )
	{
		//specify the start date of the contract
		pSoldier->iStartOfInsuranceContract = CalcStartDayOfInsurance( pSoldier );
	}

	//add transaction to finaces page
	//if the player has life insurance
	if( pSoldier->usLifeInsurance )
	{
		//if the player is extending the contract
		if( iAmountOfMoneyTransfer > 0 )
			AddTransactionToPlayersBook(	EXTENDED_INSURANCE, pSoldier->ubProfile, GetWorldTotalMin(), -( iAmountOfMoneyTransfer ) );
		else
			SLOGA("negative Cost of Life Insurance, something went horribly wrong");
	}
	else
	{
		//if the player doesnt have enough money, tell him
		if( LaptopSaveInfo.iCurrentBalance < iAmountOfMoneyTransfer )
		{
			ST::string sText = GetInsuranceText(INS_MLTI_NOT_ENOUGH_FUNDS);
			if( guiCurrentScreen == LAPTOP_SCREEN )
				DoLapTopMessageBox( MSG_BOX_RED_ON_WHITE, sText, LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL);
			else
				DoMapMessageBox( MSG_BOX_RED_ON_WHITE, sText, MAP_SCREEN, MSG_BOX_FLAG_OK, NULL);
		}
		else
		{
			//else if the player has enought to cover the bill, let him

			//the player just purchased life insurance
			AddTransactionToPlayersBook(	PURCHASED_INSURANCE, pSoldier->ubProfile, GetWorldTotalMin(), -( iAmountOfMoneyTransfer ) );

			//add an entry in the history page for the purchasing of life insurance
			AddHistoryToPlayersLog(HISTORY_PURCHASED_INSURANCE, pSoldier->ubProfile, GetWorldTotalMin(), -1, -1 );

			//Set that we have life insurance
			pSoldier->usLifeInsurance = 1;
		}
	}

	pSoldier->iTotalLengthOfInsuranceContract += uiInsuranceLength;

	//make sure the length doesnt exceed the contract length
	if( ( GetTimeRemainingOnSoldiersInsuranceContract( pSoldier ) ) > GetTimeRemainingOnSoldiersContract( pSoldier ) )
	{
		pSoldier->iTotalLengthOfInsuranceContract -= GetTimeRemainingOnSoldiersInsuranceContract( pSoldier ) - GetTimeRemainingOnSoldiersContract( pSoldier );
	}
}


static BOOLEAN CanSoldierExtendInsuranceContract(const SOLDIERTYPE* pSoldier)
{
	if( CalculateSoldiersInsuranceContractLength( pSoldier ) != 0 )
		return( TRUE );
	else
		return( FALSE );
}


static INT32 CalculateSoldiersInsuranceContractLength(const SOLDIERTYPE* pSoldier)
{
	INT32 iInsuranceContractLength=0;
	UINT32 uiTimeRemainingOnSoldiersContract = GetTimeRemainingOnSoldiersContract( pSoldier );

	if (IsMercDead(GetProfile(pSoldier->ubProfile))) return 0;

	// only mercs with at least 2 days to go on their employment contract are insurable
	// def: 2/5/99.  However if they already have insurance is SHOULD be ok
	if( uiTimeRemainingOnSoldiersContract < 2 && !( pSoldier->usLifeInsurance != 0 && uiTimeRemainingOnSoldiersContract >= 1 ) )
	{
		return( 0 );
	}

	//
	//Calculate the insurance contract length
	//

	//if the soldier has an insurance contract, dont deduct a day
	if( pSoldier->usLifeInsurance || DidGameJustStart() )
		iInsuranceContractLength = uiTimeRemainingOnSoldiersContract - GetTimeRemainingOnSoldiersInsuranceContract( pSoldier );

	//else deduct a day
	else
		iInsuranceContractLength = uiTimeRemainingOnSoldiersContract - GetTimeRemainingOnSoldiersInsuranceContract( pSoldier ) - 1;

	//make sure the length doesnt exceed the contract length
	if( ( GetTimeRemainingOnSoldiersInsuranceContract( pSoldier ) + iInsuranceContractLength ) > uiTimeRemainingOnSoldiersContract )
	{
		iInsuranceContractLength = uiTimeRemainingOnSoldiersContract - GetTimeRemainingOnSoldiersInsuranceContract( pSoldier );
	}

	//Is the mercs insurace contract is less then a day, set it to 0
	if( iInsuranceContractLength < 0 )
		iInsuranceContractLength = 0;

	if( pSoldier->usLifeInsurance && pSoldier->iStartOfInsuranceContract >= (INT32)GetWorldDay() && iInsuranceContractLength < 2 )
		iInsuranceContractLength = 0;

	return( iInsuranceContractLength );
}


static INT32 CalcStartDayOfInsurance(SOLDIERTYPE* pSoldier)
{
	UINT32	uiDayToStartInsurance=0;

	//if the soldier was just hired ( in transit ), and the game didnt just start
	if( pSoldier->bAssignment == IN_TRANSIT && !DidGameJustStart() )
	{
		uiDayToStartInsurance = GetWorldDay( );
	}
	else
	{
		//Get tomorows date ( and convert it to days )
		uiDayToStartInsurance = GetMidnightOfFutureDayInMinutes( 1 ) / 1440;
	}

	return( uiDayToStartInsurance );
}


static BOOLEAN AreAnyAimMercsOnTeam(void)
{
	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC) return TRUE;
	}
	return FALSE;
}
