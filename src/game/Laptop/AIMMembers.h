#ifndef __AIMMEMBERS_H_
#define __AIMMEMBERS_H_

#include "JA2Types.h"
#include <string_theory/string>


void EnterAIMMembers(void);
void ExitAIMMembers(void);
void HandleAIMMembers(void);
void RenderAIMMembers(void);


void DrawNumeralsToScreen(INT32 iNumber, INT8 bWidth, UINT16 usLocX, UINT16 usLocY, SGPFont, UINT32 ubColor);

void DisplayTextForMercFaceVideoPopUp(const ST::string& str);
void EnterInitAimMembers(void);
void RenderAIMMembersTopLevel(void);

// if merc is still annoyed, reset back to 0
void ResetMercAnnoyanceAtPlayer(ProfileID);

void DisableNewMailMessage(void);
void DisplayPopUpBoxExplainingMercArrivalLocationAndTime(void);


// enumerated types used for the Video Conferencing Display
enum AIMVideoMode
{
	AIM_VIDEO_NOT_DISPLAYED_MODE,          // The video popup is not displayed
	AIM_VIDEO_POPUP_MODE,                  // The title bar pops up out of the Contact button
	AIM_VIDEO_INIT_MODE,                   // When the player first tries to contact the merc, it will be snowy for a bit
	AIM_VIDEO_FIRST_CONTACT_MERC_MODE,     // The popup that is displayed when first contactinf the merc
	AIM_VIDEO_HIRE_MERC_MODE,              // The popup which deals with the contract length, and transfer funds
	AIM_VIDEO_MERC_ANSWERING_MACHINE_MODE, // The popup which will be instread of the AIM_VIDEO_FIRST_CONTACT_MERC_MODE if the merc is not there
	AIM_VIDEO_MERC_UNAVAILABLE_MODE,       // The popup which will be instread of the AIM_VIDEO_FIRST_CONTACT_MERC_MODE if the merc is unavailable
	AIM_VIDEO_POPDOWN_MODE,                // The title bars pops down to the contact button
};

// which mode are we in during video conferencing?..0 means no video conference
extern AIMVideoMode gubVideoConferencingMode;

#endif
