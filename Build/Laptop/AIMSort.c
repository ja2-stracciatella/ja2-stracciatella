#include "Cursors.h"
#include "Font.h"
#include "Laptop.h"
#include "AIMSort.h"
#include "AIM.h"
#include "VObject.h"
#include "WCheck.h"
#include "WordWrap.h"
#include "Soldier_Profile.h"
#include "Game_Clock.h"
#include "Text.h"
#include "Multi_Language_Graphic_Utils.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Debug.h"
#include "Font_Control.h"


//#define

#define		AIM_SORT_FONT_TITLE									FONT14ARIAL
#define		AIM_SORT_FONT_SORT_TEXT							FONT10ARIAL

#define		AIM_SORT_COLOR_SORT_TEXT						AIM_FONT_MCOLOR_WHITE
#define		AIM_SORT_SORT_BY_COLOR							146
#define		AIM_SORT_LINK_TEXT_COLOR						146

#define		AIM_SORT_GAP_BN_ICONS				60
#define		AIM_SORT_CHECKBOX_SIZE			10
#define		AIM_SORT_ON									0
#define		AIM_SORT_OFF								1

#define		AIM_SORT_SORT_BY_X					IMAGE_OFFSET_X + 155
#define		AIM_SORT_SORT_BY_Y					IMAGE_OFFSET_Y + 96
#define		AIM_SORT_SORT_BY_WIDTH			190
#define		AIM_SORT_SORT_BY_HEIGHT			81

#define		AIM_SORT_TO_MUGSHOTS_X			IMAGE_OFFSET_X + 89
#define		AIM_SORT_TO_MUGSHOTS_Y			IMAGE_OFFSET_Y + 184
#define		AIM_SORT_TO_MUGSHOTS_SIZE		54

#define		AIM_SORT_TO_STATS_X					AIM_SORT_TO_MUGSHOTS_X
#define		AIM_SORT_TO_STATS_Y					AIM_SORT_TO_MUGSHOTS_Y + AIM_SORT_GAP_BN_ICONS
#define		AIM_SORT_TO_STATS_SIZE			AIM_SORT_TO_MUGSHOTS_SIZE

#define		AIM_SORT_TO_ALUMNI_X				AIM_SORT_TO_MUGSHOTS_X
#define		AIM_SORT_TO_ALUMNI_Y				AIM_SORT_TO_STATS_Y + AIM_SORT_GAP_BN_ICONS
#define		AIM_SORT_TO_ALUMNI_SIZE			AIM_SORT_TO_MUGSHOTS_SIZE

#define		AIM_SORT_AIM_MEMBER_X				AIM_SORT_SORT_BY_X
#define		AIM_SORT_AIM_MEMBER_Y				105 + LAPTOP_SCREEN_WEB_DELTA_Y
#define		AIM_SORT_AIM_MEMBER_WIDTH		AIM_SORT_SORT_BY_WIDTH

#define		AIM_SORT_SORT_BY_TEXT_X			AIM_SORT_SORT_BY_X + 9
#define		AIM_SORT_SORT_BY_TEXT_Y			AIM_SORT_SORT_BY_Y + 8

#define		AIM_SORT_ASC_DESC_WIDTH			100


#define		AIM_SORT_MUGSHOT_TEXT_X			266
#define		AIM_SORT_MUGSHOT_TEXT_Y			230 + LAPTOP_SCREEN_WEB_DELTA_Y

#define		AIM_SORT_MERC_STATS_TEXT_X	AIM_SORT_MUGSHOT_TEXT_X
#define		AIM_SORT_MERC_STATS_TEXT_Y	293 + LAPTOP_SCREEN_WEB_DELTA_Y

#define		AIM_SORT_ALUMNI_TEXT_X			AIM_SORT_MUGSHOT_TEXT_X
#define		AIM_SORT_ALUMNI_TEXT_Y			351 + LAPTOP_SCREEN_WEB_DELTA_Y

#define		AIM_SORT_FIRST_SORT_CLOUMN_GAP	22


typedef struct AIMSortInfo
{
	UINT16         const x;
	UINT16         const y;
	UINT32         const align;
	const wchar_t* const text;
	MOUSE_CALLBACK const click;
	MOUSE_REGION         region;
} AIMSortInfo;


static void SelectAscendBoxRegionCallBack(    MOUSE_REGION* pRegion, INT32 iReason);
static void SelectDescendBoxRegionCallBack(   MOUSE_REGION* pRegion, INT32 iReason);
static void SelectExpBoxRegionCallBack(       MOUSE_REGION* pRegion, INT32 iReason);
static void SelectExplosiveBoxRegionCallBack( MOUSE_REGION* pRegion, INT32 iReason);
static void SelectMarkBoxRegionCallBack(      MOUSE_REGION* pRegion, INT32 iReason);
static void SelectMechanicalBoxRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectMedicalBoxRegionCallBack(   MOUSE_REGION* pRegion, INT32 iReason);
static void SelectPriceBoxRegionCallBack(     MOUSE_REGION* pRegion, INT32 iReason);


static AIMSortInfo g_aim_sort_info[] =
{
	{ AIM_SORT_SORT_BY_X +   9, AIM_SORT_SORT_BY_Y + 34, LEFT_JUSTIFIED,  str_aim_sort_price,        SelectPriceBoxRegionCallBack      },
	{ AIM_SORT_SORT_BY_X +   9, AIM_SORT_SORT_BY_Y + 47, LEFT_JUSTIFIED,  str_aim_sort_experience,   SelectExpBoxRegionCallBack        },
	{ AIM_SORT_SORT_BY_X +   9, AIM_SORT_SORT_BY_Y + 60, LEFT_JUSTIFIED,  str_aim_sort_marksmanship, SelectMarkBoxRegionCallBack       },
	{ AIM_SORT_SORT_BY_X + 111, AIM_SORT_SORT_BY_Y + 34, LEFT_JUSTIFIED,  str_aim_sort_medical,      SelectMedicalBoxRegionCallBack    },
	{ AIM_SORT_SORT_BY_X + 111, AIM_SORT_SORT_BY_Y + 47, LEFT_JUSTIFIED,  str_aim_sort_explosives,   SelectExplosiveBoxRegionCallBack  },
	{ AIM_SORT_SORT_BY_X + 111, AIM_SORT_SORT_BY_Y + 60, LEFT_JUSTIFIED,  str_aim_sort_mechanical,   SelectMechanicalBoxRegionCallBack },
	{ AIM_SORT_SORT_BY_X + 172, AIM_SORT_SORT_BY_Y +  4, RIGHT_JUSTIFIED, str_aim_sort_ascending,    SelectAscendBoxRegionCallBack     },
	{ AIM_SORT_SORT_BY_X + 172, AIM_SORT_SORT_BY_Y + 17, RIGHT_JUSTIFIED, str_aim_sort_descending,   SelectDescendBoxRegionCallBack    }
};

UINT8			gubCurrentSortMode;
UINT8			gubCurrentListMode;
UINT8			gubOldListMode;

// Mouse stuff
//Clicking on To Mugshot
MOUSE_REGION    gSelectedToMugShotRegion;

//Clicking on ToStats
MOUSE_REGION    gSelectedToStatsRegion;

//Clicking on ToStats
MOUSE_REGION    gSelectedToArchiveRegion;


static SGPVObject* guiSortByBox;
static SGPVObject* guiToAlumni;
static SGPVObject* guiToMugShots;
static SGPVObject* guiToStats;
static SGPVObject* guiSelectLight;


void GameInitAimSort()
{
	gubCurrentSortMode=0;
	gubCurrentListMode=AIM_DESCEND;
	gubOldListMode=AIM_DESCEND;
}


static void SelectToArchiveRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectToMugShotRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectToStatsRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);


BOOLEAN EnterAimSort()
{
	SGPFILENAME ImageFile;

	//Everytime into Aim Sort, reset array.
	for (UINT8 i = 0; i < MAX_NUMBER_MERCS; ++i)
	{
		AimMercArray[i] = i;
	}

	InitAimDefaults();

	// load the SortBy box graphic and add it
	guiSortByBox = AddVideoObjectFromFile("LAPTOP/SortBy.sti");
	CHECKF(guiSortByBox != NO_VOBJECT);

	// load the ToAlumni graphic and add it
	GetMLGFilename(ImageFile, MLG_TOALUMNI);
	guiToAlumni = AddVideoObjectFromFile(ImageFile);
	CHECKF(guiToAlumni != NO_VOBJECT);

	// load the ToMugShots graphic and add it
	GetMLGFilename(ImageFile, MLG_TOMUGSHOTS);
	guiToMugShots = AddVideoObjectFromFile(ImageFile);
	CHECKF(guiToMugShots != NO_VOBJECT);

	// load the ToStats graphic and add it
	GetMLGFilename(ImageFile, MLG_TOSTATS);
	guiToStats = AddVideoObjectFromFile(ImageFile);
	CHECKF(guiToStats != NO_VOBJECT);

	// load the SelectLight graphic and add it
	guiSelectLight = AddVideoObjectFromFile("LAPTOP/SelectLight.sti");
	CHECKF(guiSelectLight != NO_VOBJECT);


	//** Mouse Regions **

	//Mouse region for the ToMugShotRegion
	MSYS_DefineRegion( &gSelectedToMugShotRegion, AIM_SORT_TO_MUGSHOTS_X, AIM_SORT_TO_MUGSHOTS_Y, (AIM_SORT_TO_MUGSHOTS_X + AIM_SORT_TO_MUGSHOTS_SIZE), (AIM_SORT_TO_MUGSHOTS_Y + AIM_SORT_TO_MUGSHOTS_SIZE), MSYS_PRIORITY_HIGH,
							 CURSOR_WWW, MSYS_NO_CALLBACK, SelectToMugShotRegionCallBack );

	//Mouse region for the ToStatsRegion
	MSYS_DefineRegion( &gSelectedToStatsRegion, AIM_SORT_TO_STATS_X, AIM_SORT_TO_STATS_Y, (AIM_SORT_TO_STATS_X + AIM_SORT_TO_STATS_SIZE), (AIM_SORT_TO_STATS_Y + AIM_SORT_TO_STATS_SIZE), MSYS_PRIORITY_HIGH,
							 CURSOR_WWW, MSYS_NO_CALLBACK, SelectToStatsRegionCallBack );

	//Mouse region for the ToArhciveRegion
	MSYS_DefineRegion( &gSelectedToArchiveRegion, AIM_SORT_TO_ALUMNI_X, AIM_SORT_TO_ALUMNI_Y, (AIM_SORT_TO_ALUMNI_X + AIM_SORT_TO_ALUMNI_SIZE), (AIM_SORT_TO_ALUMNI_Y + AIM_SORT_TO_ALUMNI_SIZE), MSYS_PRIORITY_HIGH,
							 CURSOR_WWW, MSYS_NO_CALLBACK, SelectToArchiveRegionCallBack );

	for (AIMSortInfo* i = g_aim_sort_info; i != endof(g_aim_sort_info); ++i)
	{
		const UINT16 txt_w = StringPixLength(i->text, AIM_SORT_FONT_SORT_TEXT);
		const UINT16 x = i->x - (i->align == LEFT_JUSTIFIED ? 0 : 4 + txt_w);
		const UINT16 w = AIM_SORT_CHECKBOX_SIZE + 4 + txt_w;
		const UINT16 y = i->y;
		const UINT16 h = AIM_SORT_CHECKBOX_SIZE;
		MSYS_DefineRegion(&i->region, x, y, x + w, y + h, MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, i->click);
	}

	InitAimMenuBar();
	RenderAimSort();

	return( TRUE );
}


static BOOLEAN SortMercArray(void);


void ExitAimSort()
{
	// Sort the merc array
	SortMercArray();
	RemoveAimDefaults();

	DeleteVideoObject(guiSortByBox);
	DeleteVideoObject(guiToAlumni);
	DeleteVideoObject(guiToMugShots);
	DeleteVideoObject(guiToStats);
	DeleteVideoObject(guiSelectLight);

  MSYS_RemoveRegion( &gSelectedToMugShotRegion);
  MSYS_RemoveRegion( &gSelectedToStatsRegion);
  MSYS_RemoveRegion( &gSelectedToArchiveRegion);

	for (AIMSortInfo* i = g_aim_sort_info; i != endof(g_aim_sort_info); ++i)
	{
		MSYS_RemoveRegion(&i->region);
	}

	ExitAimMenuBar();

}

void HandleAimSort()
{

}


static void DrawSelectLight(UINT8 ubMode, UINT8 ubImage);


void RenderAimSort()
{
	DrawAimDefaults();
	BltVideoObject(FRAME_BUFFER, guiSortByBox,  0, AIM_SORT_SORT_BY_X,     AIM_SORT_SORT_BY_Y);
	BltVideoObject(FRAME_BUFFER, guiToMugShots, 0, AIM_SORT_TO_MUGSHOTS_X, AIM_SORT_TO_MUGSHOTS_Y);
	BltVideoObject(FRAME_BUFFER, guiToStats,    0, AIM_SORT_TO_STATS_X,    AIM_SORT_TO_STATS_Y);
	BltVideoObject(FRAME_BUFFER, guiToAlumni,   0, AIM_SORT_TO_ALUMNI_X,   AIM_SORT_TO_ALUMNI_Y);

	// Draw the aim slogan under the symbol
	DisplayAimSlogan();

	//Display AIM Member text
	DrawTextToScreen(AimSortText[AIM_AIMMEMBERS], AIM_SORT_AIM_MEMBER_X, AIM_SORT_AIM_MEMBER_Y, AIM_SORT_AIM_MEMBER_WIDTH, AIM_MAINTITLE_FONT, AIM_MAINTITLE_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Display sort title
	DrawTextToScreen(AimSortText[SORT_BY], AIM_SORT_SORT_BY_TEXT_X, AIM_SORT_SORT_BY_TEXT_Y, 0, AIM_SORT_FONT_TITLE, AIM_SORT_SORT_BY_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	// Display all the sort by text
	for (const AIMSortInfo* i = g_aim_sort_info; i != endof(g_aim_sort_info); ++i)
	{
		const UINT16 x = i->x + (i->align == LEFT_JUSTIFIED ? 14 : -AIM_SORT_ASC_DESC_WIDTH - 4);
		DrawTextToScreen(i->text, x, i->y + 2, AIM_SORT_ASC_DESC_WIDTH, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_COLOR_SORT_TEXT, FONT_MCOLOR_BLACK, i->align);
	}

	// Display text for the 3 icons
	DrawTextToScreen(AimSortText[MUGSHOT_INDEX],   AIM_SORT_MUGSHOT_TEXT_X,    AIM_SORT_MUGSHOT_TEXT_Y,    0, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_LINK_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(AimSortText[MERCENARY_FILES], AIM_SORT_MERC_STATS_TEXT_X, AIM_SORT_MERC_STATS_TEXT_Y, 0, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_LINK_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(AimSortText[ALUMNI_GALLERY],  AIM_SORT_ALUMNI_TEXT_X,     AIM_SORT_ALUMNI_TEXT_Y,     0, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_LINK_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	DrawSelectLight(gubCurrentSortMode, AIM_SORT_ON);
	DrawSelectLight(gubCurrentListMode, AIM_SORT_ON);

	DisableAimButton();

  MarkButtonsDirty( );

	RenderWWWProgramTitleBar( );

  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void SelectToMugShotRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX;
	}
}


static void SelectToStatsRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_AIM_MEMBERS;
	}
}


static void SelectToArchiveRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_AIM_MEMBERS_ARCHIVES;
	}
}


static void SetSortCriterion(const UINT8 criterion)
{
	if (gubCurrentSortMode == criterion) return;
	DrawSelectLight(gubCurrentSortMode, AIM_SORT_OFF);
	gubCurrentSortMode = criterion;
	DrawSelectLight(criterion, AIM_SORT_ON);
}


static void SelectPriceBoxRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) SetSortCriterion(0);
}


static void SelectExpBoxRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) SetSortCriterion(1);
}


static void SelectMarkBoxRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) SetSortCriterion(2);
}


static void SelectMedicalBoxRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) SetSortCriterion(3);
}


static void SelectExplosiveBoxRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) SetSortCriterion(4);
}


static void SelectMechanicalBoxRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) SetSortCriterion(5);
}


static void SelectAscendBoxRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if(gubCurrentListMode != AIM_ASCEND)
		{
			gubCurrentListMode = AIM_ASCEND;
			DrawSelectLight(gubCurrentListMode, AIM_SORT_ON);
			DrawSelectLight(gubOldListMode, AIM_SORT_OFF);
			gubOldListMode = gubCurrentListMode;
		}
	}
}


static void SelectDescendBoxRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if(gubCurrentListMode != AIM_DESCEND )
		{
			gubCurrentListMode = AIM_DESCEND;
			DrawSelectLight(gubCurrentListMode, AIM_SORT_ON);
			DrawSelectLight(gubOldListMode, AIM_SORT_OFF);
			gubOldListMode = gubCurrentListMode;
		}
	}
}


static void DrawSelectLight(const UINT8 ubMode, const UINT8 ubImage)
{
	const AIMSortInfo* const asi = &g_aim_sort_info[ubMode];
	const INT32 x = asi->x;
	const INT32 y = asi->y;
	BltVideoObject(FRAME_BUFFER, guiSelectLight, ubImage, x, y);
	InvalidateRegion(x, y, x + AIM_SORT_CHECKBOX_SIZE, y + AIM_SORT_CHECKBOX_SIZE);
}


static INT32 QsortCompare(const void* pNum1, const void* pNum2);


static BOOLEAN SortMercArray(void)
{
	qsort(AimMercArray, MAX_NUMBER_MERCS, sizeof(UINT8), QsortCompare);

	return(TRUE);
}


static INT32 CompareValue(const INT32 Num1, const INT32 Num2);


static INT32 QsortCompare(const void* pNum1, const void* pNum2)
{
	UINT8 Num1 = *(UINT8*)pNum1;
	UINT8 Num2 = *(UINT8*)pNum2;

	switch( gubCurrentSortMode )
	{
		//Price						INT16	uiWeeklySalary
		case 0:
			return( CompareValue((INT32)gMercProfiles[ Num1 ].uiWeeklySalary,  (INT32)gMercProfiles[Num2].uiWeeklySalary ) );
		//Experience			INT16	bExpLevel
		case 1:
			return( CompareValue((INT32)gMercProfiles[ Num1 ].bExpLevel,  (INT32)gMercProfiles[Num2].bExpLevel) );
		//Marksmanship		INT16	bMarksmanship
		case 2:
			return( CompareValue((INT32)gMercProfiles[ Num1 ].bMarksmanship,  (INT32)gMercProfiles[Num2].bMarksmanship ) );
		//Medical					INT16	bMedical
		case 3:
			return( CompareValue((INT32)gMercProfiles[ Num1 ].bMedical,  (INT32)gMercProfiles[Num2].bMedical ) );
		//Explosives			INT16	bExplosive
		case 4:
			return( CompareValue((INT32)gMercProfiles[ Num1 ].bExplosive,  (INT32)gMercProfiles[Num2].bExplosive ) );
		//Mechanical			INT16	bMechanical
		case 5:
			return( CompareValue((INT32)gMercProfiles[ Num1 ].bMechanical,  (INT32)gMercProfiles[Num2].bMechanical ) );

		default:
			Assert( 0 );
			return( 0 );
	}
}


static INT32 CompareValue(const INT32 Num1, const INT32 Num2)
{
	// Ascending
	if( gubCurrentListMode == AIM_ASCEND)
	{
		if( Num1 < Num2)
			return(-1);
		else if( Num1 == Num2)
			return(0);
		else
			return(1);
	}

	// Descending
	else if( gubCurrentListMode == AIM_DESCEND )
	{
		if( Num1 > Num2)
			return(-1);
		else if( Num1 == Num2)
			return(0);
		else
			return(1);
	}

	return( 0 );
}
