#ifndef __MERCS_H
#define __MERCS_H

#include <string_theory/string>

class  MERCListingModel;

#define MERC_BUTTON_UP_COLOR			FONT_MCOLOR_WHITE
#define MERC_BUTTON_DOWN_COLOR			FONT_MCOLOR_DKWHITE


#define MERC_NUM_DAYS_TILL_FIRST_WARNING	7
#define MERC_NUM_DAYS_TILL_ACCOUNT_SUSPENDED	9
#define MERC_NUM_DAYS_TILL_ACCOUNT_INVALID	12


#define MERC_LARRY_ROACHBURN			7


#define DAYS_TIL_M_E_R_C_AVAIL			3

//The players account information for the MERC site
enum
{
	MERC_NO_ACCOUNT,
	MERC_ACCOUNT_SUSPENDED,
	MERC_ACCOUNT_INVALID,
	MERC_ACCOUNT_VALID_FIRST_WARNING,
	MERC_ACCOUNT_VALID,
};
//extern UINT8 gubPlayersMercAccountStatus;
//extern UINT32 guiPlayersMercAccountNumber;


// The video conferencing for the merc page
#define MERC_VIDEO_SPECK_SPEECH_NOT_TALKING 65535
#define MERC_VIDEO_SPECK_HAS_TO_TALK_BUT_QUOTE_NOT_CHOSEN_YET 65534


//used with the gubArrivedFromMercSubSite variable to signify whcih page the player came from
enum
{
	MERC_CAME_FROM_OTHER_PAGE,
	MERC_CAME_FROM_ACCOUNTS_PAGE,
	MERC_CAME_FROM_HIRE_PAGE,
};



void GameInitMercs(void);
void EnterMercs(void);
void ExitMercs(void);
void HandleMercs(void);
void RenderMercs(void);


void InitMercBackGround(void);
void DrawMecBackGround(void);
void RemoveMercBackGround(void);
void DailyUpdateOfMercSite();
ProfileID GetProfileIDFromMERCListingIndex(UINT8 ubMercIndex);
ProfileID GetProfileIDFromMERCListing(const MERCListingModel* listing);
void DisplayTextForSpeckVideoPopUp(const ST::string& str);

void EnterInitMercSite(void);

void GetMercSiteBackOnline(void);

void DisableMercSiteButton(void);

extern UINT16 gusMercVideoSpeckSpeech;

extern UINT8 gubArrivedFromMercSubSite;

extern UINT8 gubCurMercIndex;

extern BOOLEAN gfJustHiredAMercMerc;

void NewMercsAvailableAtMercSiteCallBack(void);

void CalcAproximateAmountPaidToSpeck(void);

void SyncLastMercFromSaveGame();
#endif
