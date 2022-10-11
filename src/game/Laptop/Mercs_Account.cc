#include "Directories.h"
#include "Font.h"
#include "Laptop.h"
#include "Merc_Hiring.h"
#include "Mercs_Account.h"
#include "Mercs.h"
#include "MessageBoxScreen.h"
#include "VObject.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Soldier_Profile.h"
#include "Finances.h"
#include "Game_Clock.h"
#include "Soldier_Add.h"
#include "Overhead.h"
#include "History.h"
#include "LaptopSave.h"
#include "Text.h"
#include "Speck_Quotes.h"
#include "Multi_Language_Graphic_Utils.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "ScreenIDs.h"
#include "Font_Control.h"
#include "GameInstance.h"
#include "ContentManager.h"

#include <string_theory/format>
#include <string_theory/string>


#define MERC_ACCOUNT_TEXT_FONT		FONT14ARIAL
#define MERC_ACCOUNT_TEXT_COLOR		FONT_MCOLOR_WHITE

#define MERC_ACCOUNT_DYNAMIC_TEXT_FONT	FONT12ARIAL
#define MERC_ACCOUNT_DYNAMIC_TEXT_COLOR	FONT_MCOLOR_WHITE
#define MERC_ACCOUNT_DEAD_TEXT_COLOR	FONT_MCOLOR_RED

#define MERC_AC_ORDER_GRID_X		LAPTOP_SCREEN_UL_X + 23
#define MERC_AC_ORDER_GRID_Y		LAPTOP_SCREEN_WEB_UL_Y + 59

#define MERC_AC_ACCOUNT_NUMBER_X	LAPTOP_SCREEN_UL_X + 23
#define MERC_AC_ACCOUNT_NUMBER_Y	LAPTOP_SCREEN_WEB_UL_Y + 13

#define MERC_AC_AUTHORIZE_BUTTON_X	(STD_SCREEN_X + 128)
#define MERC_AC_CANCEL_BUTTON_X		(STD_SCREEN_X + 490)
#define MERC_AC_BUTTON_Y		(STD_SCREEN_Y + 380)

#define MERC_AC_ACCOUNT_NUMBER_TEXT_X	MERC_AC_ACCOUNT_NUMBER_X + 5
#define MERC_AC_ACCOUNT_NUMBER_TEXT_Y	MERC_AC_ACCOUNT_NUMBER_Y + 12

#define MERC_AC_MERC_TITLE_Y		MERC_AC_ORDER_GRID_Y + 14
#define MERC_AC_TOTAL_COST_Y		MERC_AC_ORDER_GRID_Y + 242

#define MERC_AC_FIRST_COLUMN_X		MERC_AC_ORDER_GRID_X + 2
#define MERC_AC_SECOND_COLUMN_X		MERC_AC_ORDER_GRID_X + 222
#define MERC_AC_THIRD_COLUMN_X		MERC_AC_ORDER_GRID_X + 292
#define MERC_AC_FOURTH_COLUMN_X		MERC_AC_ORDER_GRID_X + 382

#define MERC_AC_FIRST_COLUMN_WIDTH	218
#define MERC_AC_SECOND_COLUMN_WIDTH	68
#define MERC_AC_THIRD_COLUMN_WIDTH	88
#define MERC_AC_FOURTH_COLUMN_WIDTH	76

#define MERC_AC_FIRST_ROW_Y		MERC_AC_ORDER_GRID_Y + 42
#define MERC_AC_ROW_SIZE		16


static SGPVObject* guiMercOrderGrid;
static SGPVObject* guiAccountNumberGrid;


INT32		giMercTotalContractCharge;

BOOLEAN	gfMercPlayerDoesntHaveEnoughMoney_DisplayWarning = FALSE;

// The Authorize button
static BUTTON_PICS* guiMercAuthorizeButtonImage;
GUIButtonRef guiMercAuthorizeBoxButton;


// The Back button
GUIButtonRef guiMercBackBoxButton;


static GUIButtonRef MakeButton(const ST::string& text, INT16 x, GUI_CALLBACK click)
{
	const INT16 shadow_col = DEFAULT_SHADOW;
	GUIButtonRef const btn = CreateIconAndTextButton(guiMercAuthorizeButtonImage, text, FONT12ARIAL, MERC_BUTTON_UP_COLOR, shadow_col, MERC_BUTTON_DOWN_COLOR, shadow_col, x, MERC_AC_BUTTON_Y, MSYS_PRIORITY_HIGH, click);
	btn->SetCursor(CURSOR_LAPTOP_SCREEN);
	return btn;
}


static void BtnMercAuthorizeButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnMercBackButtonCallback(GUI_BUTTON* btn, UINT32 reason);


void EnterMercsAccount()
{
	InitMercBackGround();

	// load the Arrow graphic and add it
	const char* const ImageFile = GetMLGFilename(MLG_ORDERGRID);
	guiMercOrderGrid = AddVideoObjectFromFile(ImageFile);

	// load the Arrow graphic and add it
	guiAccountNumberGrid = AddVideoObjectFromFile(LAPTOPDIR "/accountnumber.sti");

	guiMercAuthorizeButtonImage = LoadButtonImage(LAPTOPDIR "/bigbuttons.sti", 0, 1);
	guiMercAuthorizeBoxButton   = MakeButton(MercAccountText[MERC_ACCOUNT_AUTHORIZE], MERC_AC_AUTHORIZE_BUTTON_X, BtnMercAuthorizeButtonCallback);
	guiMercAuthorizeBoxButton->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_SHADED);
	guiMercBackBoxButton        = MakeButton(MercAccountText[MERC_ACCOUNT_HOME],      MERC_AC_CANCEL_BUTTON_X,    BtnMercBackButtonCallback);

	//RenderMercsAccount();

	//if true, will display a msgbox telling user that they dont have enough funds
	gfMercPlayerDoesntHaveEnoughMoney_DisplayWarning = FALSE;
}


void ExitMercsAccount()
{
	DeleteVideoObject(guiMercOrderGrid);
	DeleteVideoObject(guiAccountNumberGrid);

	UnloadButtonImage( guiMercAuthorizeButtonImage );
	RemoveButton( guiMercAuthorizeBoxButton );
	RemoveButton( guiMercBackBoxButton );

	RemoveMercBackGround();
}

void HandleMercsAccount()
{
	//if true, will display a msgbox telling user that they dont have enough funds
	if( gfMercPlayerDoesntHaveEnoughMoney_DisplayWarning )
	{
		gfMercPlayerDoesntHaveEnoughMoney_DisplayWarning = FALSE;

		DoLapTopMessageBox( MSG_BOX_BLUE_ON_GREY, "Transfer failed.  No funds available.", LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL );
	}
}


static void DisplayHiredMercs(void);


void RenderMercsAccount()
{
	ST::string sText;

	DrawMecBackGround();

	BltVideoObject(FRAME_BUFFER, guiMercOrderGrid,     0, MERC_AC_ORDER_GRID_X,     MERC_AC_ORDER_GRID_Y);
	BltVideoObject(FRAME_BUFFER, guiAccountNumberGrid, 0, MERC_AC_ACCOUNT_NUMBER_X, MERC_AC_ACCOUNT_NUMBER_Y);

	//Display Players account number
	sText = ST::format("{} {05d}", MercAccountText[MERC_ACCOUNT_ACCOUNT], LaptopSaveInfo.guiPlayersMercAccountNumber);
	DrawTextToScreen(sText, MERC_AC_ACCOUNT_NUMBER_TEXT_X, MERC_AC_ACCOUNT_NUMBER_TEXT_Y, 0, MERC_ACCOUNT_TEXT_FONT, MERC_ACCOUNT_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Display the order grid titles
	DrawTextToScreen(MercAccountText[MERC_ACCOUNT_MERC],   MERC_AC_FIRST_COLUMN_X,  MERC_AC_MERC_TITLE_Y, MERC_AC_FIRST_COLUMN_WIDTH,  MERC_ACCOUNT_TEXT_FONT, MERC_ACCOUNT_TEXT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	DrawTextToScreen(MercAccountText[MERC_ACCOUNT_DAYS],   MERC_AC_SECOND_COLUMN_X, MERC_AC_MERC_TITLE_Y, MERC_AC_SECOND_COLUMN_WIDTH, MERC_ACCOUNT_TEXT_FONT, MERC_ACCOUNT_TEXT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	DrawTextToScreen(MercAccountText[MERC_ACCOUNT_RATE],   MERC_AC_THIRD_COLUMN_X,  MERC_AC_MERC_TITLE_Y, MERC_AC_THIRD_COLUMN_WIDTH,  MERC_ACCOUNT_TEXT_FONT, MERC_ACCOUNT_TEXT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	DrawTextToScreen(MercAccountText[MERC_ACCOUNT_CHARGE], MERC_AC_FOURTH_COLUMN_X, MERC_AC_MERC_TITLE_Y, MERC_AC_FOURTH_COLUMN_WIDTH, MERC_ACCOUNT_TEXT_FONT, MERC_ACCOUNT_TEXT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	DrawTextToScreen(MercAccountText[MERC_ACCOUNT_TOTAL],  MERC_AC_THIRD_COLUMN_X,  MERC_AC_TOTAL_COST_Y, MERC_AC_THIRD_COLUMN_WIDTH,  MERC_ACCOUNT_TEXT_FONT, MERC_ACCOUNT_TEXT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	DisplayHiredMercs();

	// giMercTotalContractCharge  gets set with the price in DisplayHiredMercs(), so if there is currently no charge, disable the button
	if( giMercTotalContractCharge == 0 )
	{
		DisableButton( guiMercAuthorizeBoxButton );
	}



	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void MercAuthorizePaymentMessageBoxCallBack(MessageBoxReturnValue);


static void BtnMercAuthorizeButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		ST::string wzAuthorizeString;
		ST::string wzDollarAmount;

		wzDollarAmount = SPrintMoney(giMercTotalContractCharge);

		//create the string to show to the user
		wzAuthorizeString = st_format_printf(MercAccountText[MERC_ACCOUNT_AUTHORIZE_CONFIRMATION], wzDollarAmount);

		DoLapTopMessageBox(MSG_BOX_BLUE_ON_GREY, wzAuthorizeString, LAPTOP_SCREEN, MSG_BOX_FLAG_YESNO, MercAuthorizePaymentMessageBoxCallBack);
	}
}


static void BtnMercBackButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_MERC;
		gubArrivedFromMercSubSite = MERC_CAME_FROM_ACCOUNTS_PAGE;
	}
}


static void DisplayHiredMercs(void)
{
	UINT16	usPosY;
	UINT32	uiContractCharge;
	ST::string sTemp;
	UINT8	ubFontColor;

	giMercTotalContractCharge = 0;

	usPosY = MERC_AC_FIRST_ROW_Y + 3;
	for (const MERCListingModel* m : GCM->getMERCListings())
	{
		ProfileID         const  pid = GetProfileIDFromMERCListing(m);
		MERCPROFILESTRUCT const& p   = GetProfile(pid);

		//is the merc on the team, or is owed money
		if (IsMercOnTeam(pid) || p.iMercMercContractLength != 0)
		{
			//if the merc is dead, make the color red, else white
			if (IsMercDead(p))
				ubFontColor = MERC_ACCOUNT_DEAD_TEXT_COLOR;
			else
				ubFontColor = MERC_ACCOUNT_DYNAMIC_TEXT_COLOR;

			//Display Mercs Name
			DrawTextToScreen(p.zName, MERC_AC_FIRST_COLUMN_X + 5, usPosY, MERC_AC_FIRST_COLUMN_WIDTH, MERC_ACCOUNT_DYNAMIC_TEXT_FONT, ubFontColor, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

			//Display The # of days the merc has worked since last paid

			sTemp =  ST::format("{}", p.iMercMercContractLength);
			DrawTextToScreen(sTemp, MERC_AC_SECOND_COLUMN_X, usPosY, MERC_AC_SECOND_COLUMN_WIDTH, MERC_ACCOUNT_DYNAMIC_TEXT_FONT, ubFontColor, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

			//Display the mercs rate
			sTemp = ST::format("${6d}", p.sSalary);
			DrawTextToScreen(sTemp, MERC_AC_THIRD_COLUMN_X, usPosY, MERC_AC_THIRD_COLUMN_WIDTH, MERC_ACCOUNT_DYNAMIC_TEXT_FONT, ubFontColor, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

			//Display the total charge
			uiContractCharge = p.sSalary * p.iMercMercContractLength;
			sTemp = ST::format("${6d}", uiContractCharge);
			DrawTextToScreen(sTemp, MERC_AC_FOURTH_COLUMN_X, usPosY, MERC_AC_FOURTH_COLUMN_WIDTH, MERC_ACCOUNT_DYNAMIC_TEXT_FONT, ubFontColor, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

			giMercTotalContractCharge += uiContractCharge;
			usPosY += MERC_AC_ROW_SIZE;
		}
	}

	sTemp = ST::format("${6d}", giMercTotalContractCharge);
	DrawTextToScreen(sTemp, MERC_AC_FOURTH_COLUMN_X, MERC_AC_TOTAL_COST_Y, MERC_AC_FOURTH_COLUMN_WIDTH, MERC_ACCOUNT_DYNAMIC_TEXT_FONT, MERC_ACCOUNT_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
}


static void SettleMercAccounts(void)
{
	//SOLDIERTYPE *pSoldier;
	INT32	iPartialPayment=0;
	INT32	iContractCharge=0;


	//loop through all the MERC mercs the player has on the team
	for (const MERCListingModel* m : GCM->getMERCListings())
	{
		ProfileID ubMercID = GetProfileIDFromMERCListing(m);

		//if the merc is on the team, or does the player owe money for a fired merc
		if( IsMercOnTeam( ubMercID ) || ( gMercProfiles[ ubMercID ].iMercMercContractLength != 0 ) )
		{
			//Calc the contract charge
			iContractCharge = gMercProfiles[ ubMercID ].sSalary * gMercProfiles[ ubMercID ].iMercMercContractLength;

			//if the player can afford to pay this merc
			if( LaptopSaveInfo.iCurrentBalance >= iPartialPayment + iContractCharge )
			{
				//Then reset the merc contract counter
				gMercProfiles[ ubMercID ].iMercMercContractLength = 0;

				//Add this mercs contract charge to the total
				iPartialPayment += iContractCharge;

				gMercProfiles[ ubMercID ].uiTotalCostToDate += iContractCharge;
			}
		}
	}

	if( iPartialPayment == 0 )
	{
		//if true, will display a msgbox telling user that they dont have enough funds
		gfMercPlayerDoesntHaveEnoughMoney_DisplayWarning = TRUE;
		return;
	}

	// add the transaction to the finance page
	AddTransactionToPlayersBook( PAY_SPECK_FOR_MERC, GetProfileIDFromMERCListingIndex( gubCurMercIndex ), GetWorldTotalMin(), -iPartialPayment );
	AddHistoryToPlayersLog(HISTORY_SETTLED_ACCOUNTS_AT_MERC, GetProfileIDFromMERCListingIndex( gubCurMercIndex ), GetWorldTotalMin(), SGPSector(-1, -1));

	//Increment the amount of money paid to speck
	LaptopSaveInfo.uiTotalMoneyPaidToSpeck += iPartialPayment;

	//If the player only made a partial payment
	if( iPartialPayment != giMercTotalContractCharge )
		gusMercVideoSpeckSpeech = SPECK_QUOTE_PLAYER_MAKES_PARTIAL_PAYMENT;
	else
	{
		gusMercVideoSpeckSpeech = SPECK_QUOTE_PLAYER_MAKES_FULL_PAYMENT;

		//if the merc's account was in suspense, re-enable it
		// CJC Dec 1 2002: an invalid account become valid again.
		//if( LaptopSaveInfo.gubPlayersMercAccountStatus != MERC_ACCOUNT_INVALID )
			LaptopSaveInfo.gubPlayersMercAccountStatus = MERC_ACCOUNT_VALID;


		// Since the player has paid, make sure speck wont complain about the lack of payment
		LaptopSaveInfo.uiSpeckQuoteFlags &= ~SPECK_QUOTE__SENT_EMAIL_ABOUT_LACK_OF_PAYMENT;
	}

	//Go to the merc homepage to say the quote
	guiCurrentLaptopMode = LAPTOP_MODE_MERC;
	gubArrivedFromMercSubSite = MERC_CAME_FROM_ACCOUNTS_PAGE;


	/*
	//if the player doesnt have enough money to fully pay for the all the mercs contract
	if( LaptopSaveInfo.iCurrentBalance < giMercTotalContractCharge )
	{
		INT32	iPartialPayment=0;
		INT32	iContractCharge=0;

		//try to make a partial payment by looping through all the mercs and settling them 1 at a time
		for(i=0; i<NUMBER_OF_MERCS; i++)
		{
			ubMercID = GetMercIDFromMERCArray( (UINT8) i );

			//if the merc is on the team
			if( IsMercOnTeam( ubMercID ) )
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileIDOnPlayerTeam(ubMercID);

				//if we can get the soldier pointer
				if( pSoldier == NULL )
					continue;

				//Calc the contract charge
				iContractCharge = gMercProfiles[ ubMercID ].sSalary * pSoldier->iTotalContractLength;

				//if the player can afford to pay this merc
				if( LaptopSaveInfo.iCurrentBalance > iContractCharge )
				{
					pSoldier = FindSoldierByProfileIDOnPlayerTeam(ubMercID);
					pSoldier->iTotalContractLength = 0;

					iPartialPayment += iContractCharge;
				}
			}
		}

		if( iPartialPayment != 0 )
		{
			// add the transaction to the finance page
			AddTransactionToPlayersBook( PAY_SPECK_FOR_MERC, GetMercIDFromMERCArray( gubCurMercIndex ), GetWorldTotalMin(), -iPartialPayment );
			AddHistoryToPlayersLog(HISTORY_SETTLED_ACCOUNTS_AT_MERC, GetMercIDFromMERCArray( gubCurMercIndex ), GetWorldTotalMin(), SGPSector(-1, -1));
		}


		//DoLapTopMessageBox( MSG_BOX_BLUE_ON_GREY, MercAccountText[MERC_ACCOUNT_NOT_ENOUGH_MONEY], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL);
		//return to the main page and have speck say quote
		guiCurrentLaptopMode = LAPTOP_MODE_MERC;
		gubArrivedFromMercSubSite = MERC_CAME_FROM_ACCOUNTS_PAGE;

		gusMercVideoSpeckSpeech = SPECK_QUOTE_PLAYER_MAKES_PARTIAL_PAYMENT;

		return;
	}

	// add the transaction to the finance page
	AddTransactionToPlayersBook( PAY_SPECK_FOR_MERC, GetMercIDFromMERCArray( gubCurMercIndex ), GetWorldTotalMin(), -giMercTotalContractCharge);
	AddHistoryToPlayersLog(HISTORY_SETTLED_ACCOUNTS_AT_MERC, GetMercIDFromMERCArray( gubCurMercIndex ), GetWorldTotalMin(), SGPSector(-1, -1));

	//reset all the mercs time
	for(i=0; i<NUMBER_OF_MERCS; i++)
	{
		ubMercID = GetMercIDFromMERCArray( (UINT8) i );

		if( IsMercOnTeam( ubMercID ) )
		{
			pSoldier = FindSoldierByProfileIDOnPlayerTeam(ubMercID);
			pSoldier->iTotalContractLength = 0;
		}
	}

	//if the merc's account was in suspense, re-enable it
	if( LaptopSaveInfo.gubPlayersMercAccountStatus != MERC_ACCOUNT_INVALID )
		LaptopSaveInfo.gubPlayersMercAccountStatus = MERC_ACCOUNT_VALID;


	//Go to the merc homepage to say the quote
	guiCurrentLaptopMode = LAPTOP_MODE_MERC;
	gubArrivedFromMercSubSite = MERC_CAME_FROM_ACCOUNTS_PAGE;
	gusMercVideoSpeckSpeech = SPECK_QUOTE_PLAYER_MAKES_FULL_PAYMENT;*/
}


static void MercAuthorizePaymentMessageBoxCallBack(MessageBoxReturnValue const bExitValue)
{
	// yes, clear the form
	if( bExitValue == MSG_BOX_RETURN_YES )
	{
		//if the player owes Speck money, then settle the accounts
		if( giMercTotalContractCharge )
			SettleMercAccounts();
	}
}


UINT32	CalculateHowMuchPlayerOwesSpeck()
{
	UINT32 uiContractCharge=0;
	for (auto m : GCM->getMERCListings())
	{
		ProfileID usMercID = GetProfileIDFromMERCListing(m);
		//Calc salary for the # of days the merc has worked since last paid
		uiContractCharge += gMercProfiles[ usMercID ].sSalary * gMercProfiles[ usMercID ].iMercMercContractLength;
	}

	return( uiContractCharge );
}
