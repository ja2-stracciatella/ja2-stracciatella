#include "CharProfile.h"
#include "Directories.h"
#include "Font.h"
#include "IMP_Attribute_Selection.h"
#include "IMP_SkillTraits.h"
#include "IMPVideoObjects.h"
#include "Input.h"
#include "MessageBoxScreen.h"
#include "Render_Dirty.h"
#include "Cursors.h"
#include "Laptop.h"
#include "IMP_Text_System.h"
#include "IMP_Compile_Character.h"
#include "Text.h"
#include "Button_System.h"
#include "Video.h"
#include "ScreenIDs.h"
#include "Font_Control.h"

#include "GameInstance.h"
#include "ContentManager.h"
#include "policy/GamePolicy.h"

#include <string_theory/format>


// width of the slider bar region
#define BAR_WIDTH 423 - 197

// width of the slider bar itself
#define SLIDER_BAR_WIDTH 37

// the sizeof one skill unit on the sliding bar in pixels
#define BASE_SKILL_PIXEL_UNIT_SIZE (423 - 230)

#define MAX_ATTRIBUTE_POINTS (gamepolicy(imp_attribute_max))
#define MIN_ATTRIBUTE_POINTS (gamepolicy(imp_attribute_min))
#define ZERO_ATTRIBUTE_POINTS_VALUE (gamepolicy(imp_attribute_zero_bonus))
#define BONUS_ATTRIBUTE_POINTS (gamepolicy(imp_attribute_bonus))

enum{
	HEALTH_ATTRIBUTE,
	DEXTERITY_ATTRIBUTE,
	AGILITY_ATTRIBUTE,
	STRENGTH_ATTRIBUTE,
	WISDOM_ATTRIBUTE,
	LEADERSHIP_ATTRIBUTE,
	MARKSMANSHIP_SKILL,
	EXPLOSIVE_SKILL,
	MEDICAL_SKILL,
	MECHANICAL_SKILL
};


// the skills as they stand
static INT32 iCurrentStrength    = 55;
static INT32 iCurrentAgility     = 55;
static INT32 iCurrentDexterity   = 55;
static INT32 iCurrentHealth      = 55;
static INT32 iCurrentLeaderShip  = 55;
static INT32 iCurrentWisdom      = 55;
static INT32 iCurrentMarkmanship = 55;
static INT32 iCurrentMechanical  = 55;
static INT32 iCurrentMedical     = 55;
static INT32 iCurrentExplosives  = 55;

// which stat is message about stat at zero about
static INT32 iCurrentStatAtZero = 0;

// total number of bonus points
static INT32 iCurrentBonusPoints = 40;

// diplsay the 0 skill point warning..if skill set to 0, warn character
static BOOLEAN fSkillAtZeroWarning = FALSE;

// is the sliding of the sliding bar active right now?
static BOOLEAN fSlideIsActive = TRUE;

// first time in game
BOOLEAN fFirstIMPAttribTime = TRUE;

// review mode
BOOLEAN fReviewStats = FALSE;

// buttons
static GUIButtonRef giIMPAttributeSelectionButton[1];
static BUTTON_PICS* giIMPAttributeSelectionButtonImage[1];

// slider buttons
static GUIButtonRef giIMPAttributeSelectionSliderButton[20];
static BUTTON_PICS* giIMPAttributeSelectionSliderButtonImage[20];

// mouse regions
static MOUSE_REGION pSliderRegions[10];


//The currently "anchored scroll bar"
static MOUSE_REGION* gpCurrentScrollBox      = NULL;
static INT32         giCurrentlySelectedStat = -1;

// has any of the sliding bars moved?...for re-rendering puposes
static BOOLEAN fHasAnySlidingBarMoved = FALSE;

static INT32 uiBarToReRender = -1;

// are we actually coming back to edit, or are we restarting?
BOOLEAN fReturnStatus = FALSE;

void SetAttributes(void);
void DrawBonusPointsRemaining(void);
void SetGeneratedCharacterAttributes(void);


static void CreateAttributeSliderButtons(void);
static void CreateIMPAttributeSelectionButtons(void);
static void CreateSlideRegionMouseRegions();


void EnterIMPAttributeSelection(void)
{
	// set attributes and skills
	if (!fReturnStatus && fFirstIMPAttribTime)
	{
		// re starting
		SetAttributes();

		gpCurrentScrollBox = NULL;
		giCurrentlySelectedStat = -1;

		// does character have PROBLEMS?
		/*
		if (DoesCharacterHaveAnAttitude())   iCurrentBonusPoints += 10;
		if (DoesCharacterHaveAPersoanlity()) iCurrentBonusPoints += 10;*/
	}
	fReturnStatus = TRUE;
	fFirstIMPAttribTime = FALSE;

	CreateIMPAttributeSelectionButtons();
	CreateAttributeSliderButtons();
	CreateSlideRegionMouseRegions();
	RenderIMPAttributeSelection();
}


void RenderIMPAttributeSelection(void)
{
	RenderProfileBackGround();
	RenderAttributeFrame(51, 87);
	RenderAttributeBoxes();
	RenderAttrib1IndentFrame(51, 30);

	if (!fReviewStats) RenderAttrib2IndentFrame(350, 42);

	// reset rerender flag
	fHasAnySlidingBarMoved = FALSE;

	PrintImpText();
	DrawBonusPointsRemaining();
}


static void DestroyAttributeSliderButtons(void);
static void DestroyIMPAttributeSelectionButtons(void);
static void DestroySlideRegionMouseRegions();


void ExitIMPAttributeSelection(void)
{
	DestroyAttributeSliderButtons();
	DestroySlideRegionMouseRegions();
	DestroyIMPAttributeSelectionButtons();

	fReturnStatus = FALSE;
}


static void DecrementStat(INT32 iStatToDecrement);
static INT32 GetCurrentAttributeValue(INT32 iAttribute);
static void IncrementStat(INT32 iStatToIncrement);
static void ProcessAttributes(void);
static void StatAtZeroBoxCallBack(MessageBoxReturnValue);


// Calculate skill value from mouse position inside skill slide bar.
INT32 MouseXPosToSkillValue(INT32 mouseXPos) {
	return ((mouseXPos - SKILL_SLIDE_START_X - LAPTOP_SCREEN_UL_X)
				* (MAX_ATTRIBUTE_POINTS - MIN_ATTRIBUTE_POINTS))
				/ BASE_SKILL_PIXEL_UNIT_SIZE + MIN_ATTRIBUTE_POINTS;
}



void HandleIMPAttributeSelection(void)
{
	// review mode, do not allow changes
	if (fReviewStats) return;

	// set the currently selectd slider bar
	if (gfLeftButtonState && gpCurrentScrollBox != NULL)
	{
		//if the user is holding down the mouse cursor to left of the start of the slider bars
		if (gusMouseXPos < SKILL_SLIDE_START_X + LAPTOP_SCREEN_UL_X)
		{
			DecrementStat(giCurrentlySelectedStat);
		}
		else if (gusMouseXPos > LAPTOP_SCREEN_UL_X + SKILL_SLIDE_START_X + BAR_WIDTH)
		{
			//else if the user is holding down the mouse button to the right of the scroll bars
			IncrementStat(giCurrentlySelectedStat);
		}
		else
		{
			// get old stat value
			INT32 iCurrentAttributeValue = GetCurrentAttributeValue(giCurrentlySelectedStat);
			INT32 iNewValue = MouseXPosToSkillValue(gusMouseXPos);

			// chenged, move mouse region if change large enough
			if (iCurrentAttributeValue != iNewValue)
			{
				// update screen
				fHasAnySlidingBarMoved = TRUE;
			}

			// change is enough
			if (iNewValue - iCurrentAttributeValue > 0)
			{
				// positive, increment stat
				for (INT32 i = iNewValue - iCurrentAttributeValue; i > 0; --i)
				{
					IncrementStat(giCurrentlySelectedStat);
				}
			}
			else
			{
				// negative, decrement stat
				for (INT32 i = iCurrentAttributeValue - iNewValue; i > 0; --i)
				{
					DecrementStat(giCurrentlySelectedStat);
				}
			}
		}

		RenderIMPAttributeSelection();
	}
	else
	{
		gpCurrentScrollBox = NULL;
		giCurrentlySelectedStat = -1;
	}

	// prcoess current state of attributes
	ProcessAttributes();

	// has any bar moved?
	if (fHasAnySlidingBarMoved)
	{
		// render
		if (uiBarToReRender == -1)
		{
			RenderIMPAttributeSelection();
		}
		else
		{
			RenderAttributeFrameForIndex(51, 87, uiBarToReRender);
			/*
			// print text for screen
			PrintImpText();

			// amt of bonus pts
			DrawBonusPointsRemaining();

			RenderAttributeFrame(51, 87);

			// render attribute boxes
			RenderAttributeBoxes();

			PrintImpText();

			InvalidateRegion(LAPTOP_SCREEN_UL_X + 51, LAPTOP_SCREEN_WEB_UL_Y + 87, LAPTOP_SCREEN_UL_X + 51 + 400, LAPTOP_SCREEN_WEB_UL_Y + 87 + 220);*/
			uiBarToReRender = -1;
			MarkButtonsDirty();
		}

		fHasAnySlidingBarMoved = FALSE;
	}
	if (fSkillAtZeroWarning)
	{
		DoLapTopMessageBox(MSG_BOX_IMP_STYLE, pSkillAtZeroWarning, LAPTOP_SCREEN, MSG_BOX_FLAG_YESNO, StatAtZeroBoxCallBack);
		fSkillAtZeroWarning = FALSE;
	}
}


static void ProcessAttributes(void)
{
	if (iCurrentStrength   < MIN_ATTRIBUTE_POINTS) iCurrentStrength   = MIN_ATTRIBUTE_POINTS;
	if (iCurrentDexterity  < MIN_ATTRIBUTE_POINTS) iCurrentDexterity  = MIN_ATTRIBUTE_POINTS;
	if (iCurrentAgility    < MIN_ATTRIBUTE_POINTS) iCurrentAgility    = MIN_ATTRIBUTE_POINTS;
	if (iCurrentWisdom     < MIN_ATTRIBUTE_POINTS) iCurrentWisdom     = MIN_ATTRIBUTE_POINTS;
	if (iCurrentLeaderShip < MIN_ATTRIBUTE_POINTS) iCurrentLeaderShip = MIN_ATTRIBUTE_POINTS;
	if (iCurrentHealth     < MIN_ATTRIBUTE_POINTS) iCurrentHealth     = MIN_ATTRIBUTE_POINTS;

	if (iCurrentStrength   > MAX_ATTRIBUTE_POINTS) iCurrentStrength   = MAX_ATTRIBUTE_POINTS;
	if (iCurrentDexterity  > MAX_ATTRIBUTE_POINTS) iCurrentDexterity  = MAX_ATTRIBUTE_POINTS;
	if (iCurrentAgility    > MAX_ATTRIBUTE_POINTS) iCurrentAgility    = MAX_ATTRIBUTE_POINTS;
	if (iCurrentWisdom     > MAX_ATTRIBUTE_POINTS) iCurrentWisdom     = MAX_ATTRIBUTE_POINTS;
	if (iCurrentLeaderShip > MAX_ATTRIBUTE_POINTS) iCurrentLeaderShip = MAX_ATTRIBUTE_POINTS;
	if (iCurrentHealth     > MAX_ATTRIBUTE_POINTS) iCurrentHealth     = MAX_ATTRIBUTE_POINTS;
}


static void IncrementStat(INT32 iStatToIncrement)
{
	// review mode, do not allow changes
	if (fReviewStats) return;

	INT32*  val = NULL;
	switch (iStatToIncrement)
	{
		case STRENGTH_ATTRIBUTE:   val = &iCurrentStrength;    break;
		case DEXTERITY_ATTRIBUTE:  val = &iCurrentDexterity;   break;
		case AGILITY_ATTRIBUTE:    val = &iCurrentAgility;     break;
		case WISDOM_ATTRIBUTE:     val = &iCurrentWisdom;      break;
		case LEADERSHIP_ATTRIBUTE: val = &iCurrentLeaderShip;  break;
		case HEALTH_ATTRIBUTE:     val = &iCurrentHealth;      break;
		case MARKSMANSHIP_SKILL:   val = &iCurrentMarkmanship; break;
		case MEDICAL_SKILL:        val = &iCurrentMedical;     break;
		case MECHANICAL_SKILL:     val = &iCurrentMechanical;  break;
		case EXPLOSIVE_SKILL:      val = &iCurrentExplosives;  break;
		default:                   SLOGA("unexpected stat %d", iStatToIncrement); return;
	}

	if (*val == 0)
	{
		if (iCurrentBonusPoints >= ZERO_ATTRIBUTE_POINTS_VALUE)
		{
			*val = MIN_ATTRIBUTE_POINTS;
			iCurrentBonusPoints -= ZERO_ATTRIBUTE_POINTS_VALUE;
			fSkillAtZeroWarning  = FALSE;
		}
	}
	else if (*val < MAX_ATTRIBUTE_POINTS)
	{
		if (iCurrentBonusPoints >= 1)
		{
			++*val;
			--iCurrentBonusPoints;
		}
	}
}


static void DecrementStat(INT32 iStatToDecrement)
{
	// review mode, do not allow changes
	if (fReviewStats) return;

	BOOLEAN may_be_zero = FALSE;
	INT32*  val         = NULL;
	switch (iStatToDecrement)
	{
		case STRENGTH_ATTRIBUTE:   val = &iCurrentStrength;                        break;
		case DEXTERITY_ATTRIBUTE:  val = &iCurrentDexterity;                       break;
		case AGILITY_ATTRIBUTE:    val = &iCurrentAgility;                         break;
		case WISDOM_ATTRIBUTE:     val = &iCurrentWisdom;                          break;
		case LEADERSHIP_ATTRIBUTE: val = &iCurrentLeaderShip;                      break;
		case HEALTH_ATTRIBUTE:     val = &iCurrentHealth;                          break;
		case MARKSMANSHIP_SKILL:   val = &iCurrentMarkmanship; may_be_zero = TRUE; break;
		case MEDICAL_SKILL:        val = &iCurrentMedical;     may_be_zero = TRUE; break;
		case MECHANICAL_SKILL:     val = &iCurrentMechanical;  may_be_zero = TRUE; break;
		case EXPLOSIVE_SKILL:      val = &iCurrentExplosives;  may_be_zero = TRUE; break;
		default:                   SLOGA("unexpected stat %d", iStatToDecrement); return;
	}

	if (*val > MIN_ATTRIBUTE_POINTS)
	{
		--*val;
		++iCurrentBonusPoints;
	}
	else if (may_be_zero && *val == MIN_ATTRIBUTE_POINTS)
	{
		*val = 0;
		iCurrentBonusPoints += ZERO_ATTRIBUTE_POINTS_VALUE;
		iCurrentStatAtZero   = iStatToDecrement;
		fSkillAtZeroWarning  = TRUE;
	}
}


static void BtnIMPAttributeFinishCallback(GUI_BUTTON* btn, INT32 reason);


static void CreateIMPAttributeSelectionButtons(void)
{
	// the finished button
	giIMPAttributeSelectionButtonImage[0] = LoadButtonImage(LAPTOPDIR "/button_2.sti", 0, 1);
	giIMPAttributeSelectionButton[0] = CreateIconAndTextButton
	(
		giIMPAttributeSelectionButtonImage[0], pImpButtonText[11], FONT12ARIAL,
		FONT_WHITE, DEFAULT_SHADOW,
		FONT_WHITE, DEFAULT_SHADOW,
		LAPTOP_SCREEN_UL_X + 136, LAPTOP_SCREEN_WEB_UL_Y + 314, MSYS_PRIORITY_HIGH,
		BtnIMPAttributeFinishCallback
	);
	giIMPAttributeSelectionButton[0]->SetCursor(CURSOR_WWW);
}


static void DestroyIMPAttributeSelectionButtons(void)
{
	// Destroy the buttons needed for the IMP attrib enter page
	RemoveButton(giIMPAttributeSelectionButton[0]);
	UnloadButtonImage(giIMPAttributeSelectionButtonImage[0]);
}


static void BtnIMPAttributeFinishCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		//are we done diting, or just reviewing the stats?
		if (fReviewStats)
		{
			iCurrentImpPage = IMP_FINISH;
		}
		else
		{
			iCurrentImpPage = IMP_ATTRIBUTE_FINISH;
		}
		fButtonPendingFlag = TRUE;
	}
}


void RenderAttributeBoxes(void)
{
	// this function will render the boxes in the sliding attribute bar, based on position
	SetFontAttributes(FONT10ARIAL, FONT_WHITE, NO_SHADOW);

	// run through and render each slider bar
	for (INT32 i = HEALTH_ATTRIBUTE; i <= MECHANICAL_SKILL; ++i)
	{
		INT32 val = GetCurrentAttributeValue(i);

		// Compensate for zeroed skills: x pos is at least 0
		INT16 sX = MAX(0, val - MIN_ATTRIBUTE_POINTS) * BASE_SKILL_PIXEL_UNIT_SIZE / (MAX_ATTRIBUTE_POINTS-MIN_ATTRIBUTE_POINTS);
		INT16 sY = SKILL_SLIDE_START_Y + SKILL_SLIDE_HEIGHT * i;

		sX += SKILL_SLIDE_START_X;
		RenderSliderBar(sX, sY);

		sX += LAPTOP_SCREEN_UL_X;
		sY += LAPTOP_SCREEN_WEB_UL_Y;
		MPrint(sX + 13, sY + 3, ST::format("{}", val));
	}

	SetFontShadow(DEFAULT_SHADOW);
}


static void BtnIMPAttributeSliderLeftCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnIMPAttributeSliderRightCallback(GUI_BUTTON* btn, INT32 reason);


static void CreateAttributeSliderButtons(void)
{
	// Create the buttons for the attribute slider
	// the finished button
	giIMPAttributeSelectionSliderButtonImage[0] = LoadButtonImage(LAPTOPDIR "/attributearrows.sti", 0, 1);
	giIMPAttributeSelectionSliderButtonImage[1] = LoadButtonImage(LAPTOPDIR "/attributearrows.sti", 3, 4);

	for (INT32 iCounter = 0; iCounter < 20; iCounter += 2)
	{
		const INT16 y = LAPTOP_SCREEN_WEB_UL_Y + (99 + iCounter / 2 * 20);
		// left/right buttons - decrement/increment stat
		giIMPAttributeSelectionSliderButton[iCounter    ] = QuickCreateButton(giIMPAttributeSelectionSliderButtonImage[0], LAPTOP_SCREEN_UL_X + 163, y, MSYS_PRIORITY_HIGHEST - 1, BtnIMPAttributeSliderLeftCallback);
		giIMPAttributeSelectionSliderButton[iCounter + 1] = QuickCreateButton(giIMPAttributeSelectionSliderButtonImage[1], LAPTOP_SCREEN_UL_X + 419, y, MSYS_PRIORITY_HIGHEST - 1, BtnIMPAttributeSliderRightCallback);

		giIMPAttributeSelectionSliderButton[iCounter    ]->SetCursor(CURSOR_WWW);
		giIMPAttributeSelectionSliderButton[iCounter + 1]->SetCursor(CURSOR_WWW);
		// set user data
		giIMPAttributeSelectionSliderButton[iCounter    ]->SetUserData(iCounter / 2);
		giIMPAttributeSelectionSliderButton[iCounter + 1]->SetUserData(iCounter / 2);
	}

	MarkButtonsDirty();
}


static void DestroyAttributeSliderButtons(void)
{
	// Destroy the buttons used for attribute manipulation
	// get rid of image
	UnloadButtonImage(giIMPAttributeSelectionSliderButtonImage[0]);
	UnloadButtonImage(giIMPAttributeSelectionSliderButtonImage[1]);

	for (INT32 iCounter = 0; iCounter < 20; iCounter++)
	{
		// get rid of button
		RemoveButton(giIMPAttributeSelectionSliderButton[iCounter]);
	}
}


static void BtnIMPAttributeSliderLeftCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN ||
			reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT)
	{
		INT32 const iValue = btn->GetUserData();
		DecrementStat(iValue);
		fHasAnySlidingBarMoved = TRUE;
		uiBarToReRender = iValue;
	}
}


static void BtnIMPAttributeSliderRightCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN ||
			reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT)
	{
		INT32 const iValue = btn->GetUserData();
		IncrementStat(iValue);
		fHasAnySlidingBarMoved = TRUE;
		uiBarToReRender = iValue;
	}
}


static void SliderRegionButtonCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateSlideRegionMouseRegions()
{
	// Create the mouse regions on the sliding area, that, if the player clicks
	// on, the bar will automatically jump to
	for (UINT32 i = 0; i != lengthof(pSliderRegions); ++i)
	{
		MOUSE_REGION& r = pSliderRegions[i];
		UINT16 const  x = LAPTOP_SCREEN_UL_X + SKILL_SLIDE_START_X;
		UINT16 const  y = LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + i * SKILL_SLIDE_HEIGHT;
		MSYS_DefineRegion(&r, x, y, x + BAR_WIDTH, y + 15, MSYS_PRIORITY_HIGH + 2, CURSOR_WWW, MSYS_NO_CALLBACK, SliderRegionButtonCallback);
		MSYS_SetRegionUserData(&r, 0, i);
	}
}


static void DestroySlideRegionMouseRegions()
{
	// Destroy the regions used for the slider 'jumping'
	FOR_EACH(MOUSE_REGION, i, pSliderRegions) MSYS_RemoveRegion(&*i);
}


static void SliderRegionButtonCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	static INT16 sOldX      = -1; // Changed from relative (to skill bar) to absolute position.
	static INT32 iAttribute = -1;

	//if we already have an anchored slider bar
	if (gpCurrentScrollBox != pRegion && gpCurrentScrollBox != NULL)
		return;

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT)
	{
		if (!fSlideIsActive) return;

		// check to see if we have moved
		if (MSYS_GetRegionUserData(pRegion, 0) != iAttribute)
		{
			// different regions
			iAttribute = MSYS_GetRegionUserData(pRegion, 0);
			sOldX = -1;
			return;
		}

		uiBarToReRender = iAttribute;

		giCurrentlySelectedStat = iAttribute;
		gpCurrentScrollBox = pRegion;

		// get new attribute value x
		INT16 sNewX = pRegion->MouseXPos;

		// sOldX has been reset, set to sNewX
		if (sOldX == -1)
		{
			sOldX = sNewX;
			return;
		}
		// check against old x
		if (sNewX != sOldX)
		{
			// get old stat value
			const INT32 iCurrentAttributeValue = GetCurrentAttributeValue(iAttribute);
			INT32 iNewValue = MouseXPosToSkillValue(sNewX);

			// changed, move mouse region if change large enough
			if (iCurrentAttributeValue != iNewValue)
			{
				// update screen
				fHasAnySlidingBarMoved = TRUE;
			}

			// change is enough
			if (iNewValue - iCurrentAttributeValue > 0)
			{
				// positive, increment stat
				for (INT32 iCounter = iNewValue - iCurrentAttributeValue; iCounter > 0; iCounter--)
				{
					IncrementStat(iAttribute);
				}
			}
			else
			{
				// negative, decrement stat
				for (INT32 iCounter = iCurrentAttributeValue - iNewValue; iCounter > 0; iCounter--)
				{
					DecrementStat(iAttribute);
				}
			}

			sOldX = sNewX;
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (fSlideIsActive)
		{
			// reset slide is active flag
			fSlideIsActive = FALSE;
			return;
		}

		// get mouse XY
		const INT16 sX = pRegion->MouseXPos;

		// which region are we in?

		// get attribute
		iAttribute = MSYS_GetRegionUserData(pRegion, 0);
		uiBarToReRender = iAttribute;

		// get value of attribute
		const INT32 iCurrentAttributeValue = GetCurrentAttributeValue(iAttribute);
		// set the new attribute value based on position of mouse click, include screen resolution
		INT32 iNewAttributeValue = MouseXPosToSkillValue(sX);

		// too high, reset to MAX_ATTRIBUTE_POINTS
		if (iNewAttributeValue > MAX_ATTRIBUTE_POINTS) iNewAttributeValue = MAX_ATTRIBUTE_POINTS;

		// get the delta
		const INT32 iAttributeDelta = iCurrentAttributeValue - iNewAttributeValue;

		// check if increment or decrement
		if (iAttributeDelta > 0)
		{
			// decrement
			for (INT32 iCounter = 0; iCounter < iAttributeDelta; iCounter++)
			{
				DecrementStat(iAttribute);
			}
		}
		else
		{
			// increment attribute
			for (INT32 iCounter = iAttributeDelta; iCounter < 0; iCounter++)
			{
				if (iCurrentAttributeValue == 0) iCounter = 0;
				IncrementStat(iAttribute);
			}
		}

		// update screen
		fHasAnySlidingBarMoved = TRUE;
	}
	else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		// get mouse positions
		const INT16 sX = pRegion->MouseXPos;

		// get attribute
		iAttribute = MSYS_GetRegionUserData(pRegion, 0);
		uiBarToReRender = iAttribute;

		// get value of attribute
		const INT32 iCurrentAttributeValue = GetCurrentAttributeValue(iAttribute);

		// get the boxes bounding x
		INT16 sNewX = (iCurrentAttributeValue - MIN_ATTRIBUTE_POINTS) * BASE_SKILL_PIXEL_UNIT_SIZE / (MAX_ATTRIBUTE_POINTS-MIN_ATTRIBUTE_POINTS) + SKILL_SLIDE_START_X + LAPTOP_SCREEN_UL_X;

		// the sNewX is below 0, reset to zero
		if (sNewX < 0) sNewX = 0;

		if (sX > sNewX && sX < sNewX + SLIDER_BAR_WIDTH)
		{
			// we are within the slide bar, set fact we want to drag and draw
			fSlideIsActive = TRUE;
		}
		else
		{
			// otherwise want to jump to position
			fSlideIsActive =FALSE;
		}
	}
}


// Get the value of the attribute that was passed
static INT32 GetCurrentAttributeValue(INT32 attribute)
{
	INT32 val = 0;
	switch (attribute)
	{
		case HEALTH_ATTRIBUTE:     val = iCurrentHealth;      break;
		case DEXTERITY_ATTRIBUTE:  val = iCurrentDexterity;   break;
		case AGILITY_ATTRIBUTE:    val = iCurrentAgility;     break;
		case STRENGTH_ATTRIBUTE:   val = iCurrentStrength;    break;
		case WISDOM_ATTRIBUTE:     val = iCurrentWisdom;      break;
		case LEADERSHIP_ATTRIBUTE: val = iCurrentLeaderShip;  break;
		case MARKSMANSHIP_SKILL:   val = iCurrentMarkmanship; break;
		case EXPLOSIVE_SKILL:      val = iCurrentExplosives;  break;
		case MEDICAL_SKILL:        val = iCurrentMedical;     break;
		case MECHANICAL_SKILL:     val = iCurrentMechanical;  break;
	}
	return val;
}


void SetAttributes(void)
{
	iCurrentStrength    = 55;
	iCurrentDexterity   = 55;
	iCurrentHealth      = 55;
	iCurrentLeaderShip  = 55;
	iCurrentWisdom      = 55;
	iCurrentAgility     = 55;
	iCurrentMarkmanship = 55;
	iCurrentMechanical  = 55;
	iCurrentMedical     = 55;
	iCurrentExplosives  = 55;

	// reset bonus pts
	iCurrentBonusPoints = BONUS_ATTRIBUTE_POINTS;
	if (gamepolicy(imp_pick_skills_directly))
	{
		iCurrentBonusPoints += DoesPlayerHaveExtraAttibutePointsToDistributeBasedOnSkillSelection();
	}
}


void DrawBonusPointsRemaining(void)
{
	// draws the amount of points remaining player has

	// just reviewing, don't blit stats
	if (fReviewStats) return;

	SetFontAttributes(FONT12ARIAL, FONT_WHITE);
	MPrint(LAPTOP_SCREEN_UL_X + 425, LAPTOP_SCREEN_WEB_UL_Y + 51, ST::format("{}", iCurrentBonusPoints));
	InvalidateRegion(LAPTOP_SCREEN_UL_X + 425, LAPTOP_SCREEN_WEB_UL_Y + 51, LAPTOP_SCREEN_UL_X + 475, LAPTOP_SCREEN_WEB_UL_Y + 71);
}


void SetGeneratedCharacterAttributes(void)
{
	// Copy over the attributes and skills of the player generated character
	iStrength     = iCurrentStrength;
	iDexterity    = iCurrentDexterity;
	iHealth       = iCurrentHealth;
	iLeadership   = iCurrentLeaderShip;
	iWisdom       = iCurrentWisdom;
	iAgility      = iCurrentAgility;
	iMarksmanship = iCurrentMarkmanship;
	iMechanical   = iCurrentMechanical;
	iMedical      = iCurrentMedical;
	iExplosives   = iCurrentExplosives;
}


static void StatAtZeroBoxCallBack(MessageBoxReturnValue const bExitValue)
{
	// yes, so start over, else stay here and do nothing for now
	switch (bExitValue)
	{
		case MSG_BOX_RETURN_YES:
			MarkButtonsDirty();
			break;

		case MSG_BOX_RETURN_NO:
			IncrementStat(iCurrentStatAtZero);
			fHasAnySlidingBarMoved = TRUE;
			MarkButtonsDirty();
			break;
		default:
			break;
	}
}
