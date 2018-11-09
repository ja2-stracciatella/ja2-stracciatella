#include "Cursors.h"
#include "Directories.h"
#include "Font.h"
#include "Laptop.h"
#include "AIMSort.h"
#include "AIM.h"
#include "VObject.h"
#include "WordWrap.h"
#include "Soldier_Profile.h"
#include "Text.h"
#include "GameRes.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Font_Control.h"


//#define

#define AIM_SORT_FONT_TITLE		FONT14ARIAL
#define AIM_SORT_FONT_SORT_TEXT		FONT10ARIAL

#define AIM_SORT_COLOR_SORT_TEXT	AIM_FONT_MCOLOR_WHITE
#define AIM_SORT_SORT_BY_COLOR		FONT_COLOR_P146
#define AIM_SORT_LINK_TEXT_COLOR	FONT_COLOR_P146

#define AIM_SORT_GAP_BN_ICONS		60 * g_ui.m_stdScreenScale
#define AIM_SORT_CHECKBOX_SIZE		10 * g_ui.m_stdScreenScale
#define AIM_SORT_ON			0
#define AIM_SORT_OFF			1

#define AIM_SORT_SORT_BY_X		IMAGE_OFFSET_X + 155 * g_ui.m_stdScreenScale
#define AIM_SORT_SORT_BY_Y		IMAGE_OFFSET_Y + 96 * g_ui.m_stdScreenScale

#define AIM_SORT_TO_MUGSHOTS_X		IMAGE_OFFSET_X + 89 * g_ui.m_stdScreenScale
#define AIM_SORT_TO_MUGSHOTS_Y		IMAGE_OFFSET_Y + 184 * g_ui.m_stdScreenScale
#define AIM_SORT_TO_MUGSHOTS_SIZE	54 * g_ui.m_stdScreenScale

#define AIM_SORT_TO_STATS_X		AIM_SORT_TO_MUGSHOTS_X
#define AIM_SORT_TO_STATS_Y		AIM_SORT_TO_MUGSHOTS_Y + AIM_SORT_GAP_BN_ICONS
#define AIM_SORT_TO_STATS_SIZE		AIM_SORT_TO_MUGSHOTS_SIZE

#define AIM_SORT_TO_ALUMNI_X		AIM_SORT_TO_MUGSHOTS_X
#define AIM_SORT_TO_ALUMNI_Y		AIM_SORT_TO_STATS_Y + AIM_SORT_GAP_BN_ICONS
#define AIM_SORT_TO_ALUMNI_SIZE		AIM_SORT_TO_MUGSHOTS_SIZE

#define AIM_SORT_AIM_MEMBER_X		AIM_SORT_SORT_BY_X
#define AIM_SORT_AIM_MEMBER_Y		(STD_SCREEN_Y + 105 * g_ui.m_stdScreenScale + LAPTOP_SCREEN_WEB_DELTA_Y)
#define AIM_SORT_AIM_MEMBER_WIDTH	190 * g_ui.m_stdScreenScale

#define AIM_SORT_SORT_BY_TEXT_X		AIM_SORT_SORT_BY_X + 9
#define AIM_SORT_SORT_BY_TEXT_Y		AIM_SORT_SORT_BY_Y + 8

#define AIM_SORT_ASC_DESC_WIDTH		100 * g_ui.m_stdScreenScale


#define AIM_SORT_MUGSHOT_TEXT_X		(STD_SCREEN_X + 266 * g_ui.m_stdScreenScale)
#define AIM_SORT_MUGSHOT_TEXT_Y		(STD_SCREEN_Y + 230 * g_ui.m_stdScreenScale + LAPTOP_SCREEN_WEB_DELTA_Y)

#define AIM_SORT_MERC_STATS_TEXT_X	AIM_SORT_MUGSHOT_TEXT_X
#define AIM_SORT_MERC_STATS_TEXT_Y	(STD_SCREEN_Y + 293 * g_ui.m_stdScreenScale + LAPTOP_SCREEN_WEB_DELTA_Y)

#define AIM_SORT_ALUMNI_TEXT_X		AIM_SORT_MUGSHOT_TEXT_X
#define AIM_SORT_ALUMNI_TEXT_Y		(STD_SCREEN_Y + 351 * g_ui.m_stdScreenScale + LAPTOP_SCREEN_WEB_DELTA_Y)


struct AIMSortInfo
{
	UINT16         const x;
	UINT16         const y;
	UINT32         const align;
	UINT16         const index;
	MOUSE_CALLBACK const click;
	MOUSE_REGION         region;
};


static void SelectAscendBoxRegionCallBack(    MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectDescendBoxRegionCallBack(   MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectExpBoxRegionCallBack(       MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectExplosiveBoxRegionCallBack( MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectMarkBoxRegionCallBack(      MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectMechanicalBoxRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectMedicalBoxRegionCallBack(   MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectPriceBoxRegionCallBack(     MOUSE_REGION* pRegion, UINT32 iReason);


static AIMSortInfo g_aim_sort_info[L10n::str_aim_sort_list_SIZE]
{
	{   9, 34, LEFT_JUSTIFIED,  0, SelectPriceBoxRegionCallBack      },
	{   9, 47, LEFT_JUSTIFIED,  1, SelectExpBoxRegionCallBack        },
	{   9, 60, LEFT_JUSTIFIED,  2, SelectMarkBoxRegionCallBack       },
	{ 111, 34, LEFT_JUSTIFIED,  3, SelectMedicalBoxRegionCallBack    },
	{ 111, 47, LEFT_JUSTIFIED,  4, SelectExplosiveBoxRegionCallBack  },
	{ 111, 60, LEFT_JUSTIFIED,  5, SelectMechanicalBoxRegionCallBack },
	{ 172,  4, RIGHT_JUSTIFIED, 6, SelectAscendBoxRegionCallBack     },
	{ 172, 17, RIGHT_JUSTIFIED, 7, SelectDescendBoxRegionCallBack    }
};

UINT8			gubCurrentSortMode;
UINT8			gubCurrentListMode;

// Mouse stuff
//Clicking on To Mugshot
static MOUSE_REGION gSelectedToMugShotRegion;

//Clicking on ToStats
static MOUSE_REGION gSelectedToStatsRegion;

//Clicking on ToStats
static MOUSE_REGION gSelectedToArchiveRegion;


static SGPVObject* guiSortByBox;
static SGPVObject* guiToAlumni;
static SGPVObject* guiToMugShots;
static SGPVObject* guiToStats;
static SGPVObject* guiSelectLight;


void GameInitAimSort()
{
	gubCurrentSortMode=0;
	gubCurrentListMode=AIM_DESCEND;
}


static void SelectToArchiveRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectToMugShotRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectToStatsRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


void EnterAimSort()
{
	//Everytime into Aim Sort, reset array.
	for (UINT8 i = 0; i < MAX_NUMBER_MERCS; ++i)
	{
		AimMercArray[i] = i;
	}

	InitAimDefaults();

	// load the SortBy box graphic and add it
	guiSortByBox = AddVideoObjectFromFile(LAPTOPDIR "/sortby.sti");

	// load the ToAlumni graphic and add it
	guiToAlumni = AddVideoObjectFromFile(MLG_TOALUMNI);

	// load the ToMugShots graphic and add it
	guiToMugShots = AddVideoObjectFromFile(MLG_TOMUGSHOTS);

	// load the ToStats graphic and add it
	guiToStats = AddVideoObjectFromFile(MLG_TOSTATS);

	// load the SelectLight graphic and add it
	guiSelectLight = AddVideoObjectFromFile(LAPTOPDIR "/selectlight.sti");


	//** Mouse Regions **

	//Mouse region for the ToMugShotRegion
	MSYS_DefineRegion(&gSelectedToMugShotRegion, AIM_SORT_TO_MUGSHOTS_X, AIM_SORT_TO_MUGSHOTS_Y,
				(AIM_SORT_TO_MUGSHOTS_X + AIM_SORT_TO_MUGSHOTS_SIZE),
				(AIM_SORT_TO_MUGSHOTS_Y + AIM_SORT_TO_MUGSHOTS_SIZE), MSYS_PRIORITY_HIGH,
				CURSOR_WWW, MSYS_NO_CALLBACK, SelectToMugShotRegionCallBack );

	//Mouse region for the ToStatsRegion
	MSYS_DefineRegion(&gSelectedToStatsRegion, AIM_SORT_TO_STATS_X, AIM_SORT_TO_STATS_Y,
				(AIM_SORT_TO_STATS_X + AIM_SORT_TO_STATS_SIZE),
				(AIM_SORT_TO_STATS_Y + AIM_SORT_TO_STATS_SIZE), MSYS_PRIORITY_HIGH,
				CURSOR_WWW, MSYS_NO_CALLBACK, SelectToStatsRegionCallBack );

	//Mouse region for the ToArhciveRegion
	MSYS_DefineRegion(&gSelectedToArchiveRegion, AIM_SORT_TO_ALUMNI_X, AIM_SORT_TO_ALUMNI_Y,
				(AIM_SORT_TO_ALUMNI_X + AIM_SORT_TO_ALUMNI_SIZE),
				(AIM_SORT_TO_ALUMNI_Y + AIM_SORT_TO_ALUMNI_SIZE), MSYS_PRIORITY_HIGH,
				CURSOR_WWW, MSYS_NO_CALLBACK, SelectToArchiveRegionCallBack );

	FOR_EACH(AIMSortInfo, i, g_aim_sort_info)
	{
		const UINT16 txt_w = StringPixLength(str_aim_sort_list[i->index], AIM_SORT_FONT_SORT_TEXT);
		const UINT16 x = AIM_SORT_SORT_BY_X + i->x * g_ui.m_stdScreenScale - (i->align == LEFT_JUSTIFIED ? 0 : 4 * g_ui.m_stdScreenScale + txt_w);
		const UINT16 w = AIM_SORT_CHECKBOX_SIZE + 4 * g_ui.m_stdScreenScale + txt_w;
		const UINT16 y = AIM_SORT_SORT_BY_Y + i->y * g_ui.m_stdScreenScale;
		const UINT16 h = AIM_SORT_CHECKBOX_SIZE;
		MSYS_DefineRegion(&i->region, x, y, x + w, y + h, MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, i->click);
	}

	InitAimMenuBar();
	RenderAimSort();
}


static void SortMercArray(void);


void ExitAimSort()
{
	// Sort the merc array
	SortMercArray();
	RemoveAimDefaults();

	delete guiSortByBox;
	delete guiToAlumni;
	delete guiToMugShots;
	delete guiToStats;
	delete guiSelectLight;

	MSYS_RemoveRegion( &gSelectedToMugShotRegion);
	MSYS_RemoveRegion( &gSelectedToStatsRegion);
	MSYS_RemoveRegion( &gSelectedToArchiveRegion);

	FOR_EACH(AIMSortInfo, i, g_aim_sort_info)
	{
		MSYS_RemoveRegion(&i->region);
	}

	ExitAimMenuBar();

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
	DrawTextToScreen(AimSortText[AIM_AIMMEMBERS], AIM_SORT_AIM_MEMBER_X, AIM_SORT_AIM_MEMBER_Y, AIM_SORT_AIM_MEMBER_WIDTH, AIM_MAINTITLE_FONT, AIM_MAINTITLE_COLOR, FONT_MCOLOR_TRANSPARENT, CENTER_JUSTIFIED);

	//Display sort title
	DrawTextToScreen(AimSortText[SORT_BY], AIM_SORT_SORT_BY_TEXT_X, AIM_SORT_SORT_BY_TEXT_Y, 0, AIM_SORT_FONT_TITLE, AIM_SORT_SORT_BY_COLOR, FONT_MCOLOR_TRANSPARENT, LEFT_JUSTIFIED);

	// Display all the sort by text
	FOR_EACH(AIMSortInfo const, i, g_aim_sort_info)
	{
		const UINT16 x = AIM_SORT_SORT_BY_X + i->x * g_ui.m_stdScreenScale + (i->align == LEFT_JUSTIFIED ? 14 * g_ui.m_stdScreenScale : -AIM_SORT_ASC_DESC_WIDTH - 4 * g_ui.m_stdScreenScale);
		DrawTextToScreen(str_aim_sort_list[i->index], x, AIM_SORT_SORT_BY_Y + (i->y + 2) * g_ui.m_stdScreenScale, AIM_SORT_ASC_DESC_WIDTH, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_COLOR_SORT_TEXT, FONT_MCOLOR_TRANSPARENT, i->align);
	}

	// Display text for the 3 icons
	DrawTextToScreen(AimSortText[MUGSHOT_INDEX],   AIM_SORT_MUGSHOT_TEXT_X,    AIM_SORT_MUGSHOT_TEXT_Y,    0, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_LINK_TEXT_COLOR, FONT_MCOLOR_TRANSPARENT, LEFT_JUSTIFIED);
	DrawTextToScreen(AimSortText[MERCENARY_FILES], AIM_SORT_MERC_STATS_TEXT_X, AIM_SORT_MERC_STATS_TEXT_Y, 0, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_LINK_TEXT_COLOR, FONT_MCOLOR_TRANSPARENT, LEFT_JUSTIFIED);
	DrawTextToScreen(AimSortText[ALUMNI_GALLERY],  AIM_SORT_ALUMNI_TEXT_X,     AIM_SORT_ALUMNI_TEXT_Y,     0, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_LINK_TEXT_COLOR, FONT_MCOLOR_TRANSPARENT, LEFT_JUSTIFIED);

	DrawSelectLight(gubCurrentSortMode, AIM_SORT_ON);
	DrawSelectLight(gubCurrentListMode, AIM_SORT_ON);

	DisableAimButton();

	MarkButtonsDirty( );

	RenderWWWProgramTitleBar( );

	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void SelectToMugShotRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX;
	}
}


static void SelectToStatsRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_AIM_MEMBERS;
	}
}


static void SelectToArchiveRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
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


static void SelectPriceBoxRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP) SetSortCriterion(0);
}


static void SelectExpBoxRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP) SetSortCriterion(1);
}


static void SelectMarkBoxRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP) SetSortCriterion(2);
}


static void SelectMedicalBoxRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP) SetSortCriterion(3);
}


static void SelectExplosiveBoxRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP) SetSortCriterion(4);
}


static void SelectMechanicalBoxRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP) SetSortCriterion(5);
}


static void SetSortOrder(const UINT8 order)
{
	if (gubCurrentListMode == order) return;
	DrawSelectLight(gubCurrentListMode, AIM_SORT_OFF);
	gubCurrentListMode = order;
	DrawSelectLight(order, AIM_SORT_ON);
}


static void SelectAscendBoxRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP) SetSortOrder(AIM_ASCEND);
}


static void SelectDescendBoxRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP) SetSortOrder(AIM_DESCEND);
}


static void DrawSelectLight(const UINT8 ubMode, const UINT8 ubImage)
{
	const AIMSortInfo* const asi = &g_aim_sort_info[ubMode];
	const INT32 x = AIM_SORT_SORT_BY_X + asi->x * g_ui.m_stdScreenScale;
	const INT32 y = AIM_SORT_SORT_BY_Y + asi->y * g_ui.m_stdScreenScale;
	BltVideoObject(FRAME_BUFFER, guiSelectLight, ubImage, x, y);
	InvalidateRegion(x, y, x + AIM_SORT_CHECKBOX_SIZE, y + AIM_SORT_CHECKBOX_SIZE);
}


static INT32 QsortCompare(const void* pNum1, const void* pNum2);


static void SortMercArray(void)
{
	qsort(AimMercArray, MAX_NUMBER_MERCS, sizeof(UINT8), QsortCompare);
}


static INT32 QsortCompare(const void* pNum1, const void* pNum2)
{
	MERCPROFILESTRUCT const& p1 = GetProfile(*(UINT8*)pNum1);
	MERCPROFILESTRUCT const& p2 = GetProfile(*(UINT8*)pNum2);

	INT32 v1;
	INT32 v2;
	switch (gubCurrentSortMode)
	{
		/* Price        */ case 0: v1 = p1.uiWeeklySalary; v2 = p2.uiWeeklySalary; break;
		/* Experience   */ case 1: v1 = p1.bExpLevel;      v2 = p2.bExpLevel;      break;
		/* Marksmanship */ case 2: v1 = p1.bMarksmanship;  v2 = p2.bMarksmanship;  break;
		/* Medical      */ case 3: v1 = p1.bMedical;       v2 = p2.bMedical;       break;
		/* Explosives   */ case 4: v1 = p1.bExplosive;     v2 = p2.bExplosive;     break;
		/* Mechanical   */ case 5: v1 = p1.bMechanical;    v2 = p2.bMechanical;    break;

		default: SLOGA("QsortCompare: invalid sort mode"); return 0;
	}
	const INT32 ret = (v1 > v2) - (v1 < v2);
	return gubCurrentListMode == AIM_ASCEND ? ret : -ret;
}
