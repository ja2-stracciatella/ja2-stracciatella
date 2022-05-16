#include "Credits.h"
#include "Cursors.h"
#include "Debug.h"
#include "Directories.h"
#include "English.h"
#include "Font.h"
#include "Font_Control.h"
#include "Input.h"
#include "Local.h"
#include "MouseSystem.h"
#include "Random.h"
#include "Render_Dirty.h"
#include "SysUtil.h"
#include "Text.h"
#include "Timer_Control.h"
#include "Types.h"
#include "VObject.h"
#include "VSurface.h"
#include "Video.h"
#include "WordWrap.h"
#include "UILayout.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/string>


struct CRDT_NODE
{
	INT16        sPosY;
	INT16        sHeightOfString;     // The height of the displayed string
	SGPVSurface* uiVideoSurfaceImage;
	CRDT_NODE*   next;
};

//flags for the credits
#define CRDT_FLAG__TITLE		0x00000001
#define CRDT_FLAG__START_SECTION	0x00000002
#define CRDT_FLAG__END_SECTION		0x00000004


#define CRDT_NAME_OF_CREDIT_FILE	BINARYDATADIR "/credits.edt"

#define CREDITS_LINESIZE		80


//
// Code tokens
//
//new codes:
#define CRDT_START_CODE		'@'
#define CRDT_SEPARATION_CODE		','
#define CRDT_END_CODE			';'

#define CRDT_DELAY_BN_STRINGS_CODE	'D'
#define CRDT_DELAY_BN_SECTIONS_CODE	'B'
#define CRDT_SCROLL_SPEED		'S'
#define CRDT_FONT_JUSTIFICATION	'J'
#define CRDT_TITLE_FONT_COLOR		'C'
#define CRDT_ACTIVE_FONT_COLOR		'R'

//Flags:
#define CRDT_TITLE			'T'
#define CRDT_START_OF_SECTION		'{'
#define CRDT_END_OF_SECTION		'}'


#define CRDT_NAME_LOC_X		(375 + STD_SCREEN_X)
#define CRDT_NAME_LOC_Y		(420 + STD_SCREEN_Y)
#define CRDT_NAME_TITLE_LOC_Y		(435 + STD_SCREEN_Y)
#define CRDT_NAME_FUNNY_LOC_Y		(450 + STD_SCREEN_Y)
#define CRDT_NAME_LOC_WIDTH		260
#define CRDT_NAME_LOC_HEIGHT		( CRDT_NAME_FUNNY_LOC_Y - CRDT_NAME_LOC_Y + GetFontHeight( CRDT_NAME_FONT ) )

#define CRDT_NAME_FONT			FONT12ARIAL


#define CRDT_WIDTH_OF_TEXT_AREA	210
#define CRDT_TEXT_START_LOC		(10 + STD_SCREEN_X)


#define CRDT_SCROLL_PIXEL_AMOUNT	1
#define CRDT_NODE_DELAY_AMOUNT		25

#define CRDT_SPACE_BN_SECTIONS		50
#define CRDT_SPACE_BN_NODES		12

#define CRDT_START_POS_Y		(SCREEN_HEIGHT - 1 - STD_SCREEN_Y)

#define CRDT_EYE_WIDTH			30
#define CRDT_EYE_HEIGHT		12

#define CRDT_EYES_CLOSED_TIME		150


struct CreditFace
{
	const INT16  sX;
	const INT16  sY;
	const INT16  sWidth;
	const INT16  sHeight;

	const INT16  sEyeX;
	const INT16  sEyeY;

	const INT16  sMouthX;
	const INT16  sMouthY;

	const UINT16 sBlinkFreq;
	UINT32       uiLastBlinkTime;
	UINT32       uiEyesClosedTime;
};

static CreditFace gCreditFaces[] =
{
#define M(x, y, w, h, ex, ey, mx, my, f) { x, y, w, h, ex, ey, mx, my, f, 0, 0 }
	M(298, 137, 37, 49, 310, 157, 304, 170, 2500), // Camfield
	M(348, 137, 43, 47, 354, 153, 354, 153, 3700), // Shawn
	M(407, 132, 30, 50, 408, 151, 410, 164, 3000), // Kris
	M(443, 131, 30, 50, 447, 151, 446, 161, 4000), // Ian
	M(487, 136, 43, 50, 493, 155, 493, 155, 3500), // Linda
	M(529, 145, 43, 50, 536, 164, 536, 164, 4000), // Eric
	M(581, 132, 43, 48, 584, 150, 583, 161, 3500), // Lynn
	M(278, 211, 36, 51, 283, 232, 283, 241, 3700), // Norm
	M(319, 210, 34, 49, 323, 227, 320, 339, 4000), // George
	M(358, 211, 38, 49, 364, 226, 361, 239, 3600), // Andrew Stacey
	M(396, 200, 42, 50, 406, 220, 403, 230, 4600), // Scott
	M(444, 202, 43, 51, 452, 220, 452, 231, 2800), // Emmons
	M(493, 188, 36, 51, 501, 207, 499, 217, 4500), // Dave
	M(531, 199, 47, 56, 541, 221, 540, 232, 4000), // Alex
	M(585, 196, 39, 49, 593, 218, 593, 228, 3500)  // Joey
#undef M
};


static MOUSE_REGION gCrdtBackgroundRegion;
static MOUSE_REGION gCrdtMouseRegions[NUM_PEOPLE_IN_CREDITS];

static BOOLEAN g_credits_active;

static SGPVObject* guiCreditBackGroundImage;
static SGPVObject* guiCreditFaces;

static CRDT_NODE* g_credits_head;
static CRDT_NODE* g_credits_tail;

static INT32 giCurrentlySelectedFace;


static SGPFont guiCreditScreenActiveFont;  // the font that is used
static SGPFont guiCreditScreenTitleFont;   // the font that is used
static UINT8 gubCreditScreenActiveColor; // color of the font
static UINT8 gubCreditScreenTitleColor;  // color of a Title node

static UINT32 guiCrdtNodeScrollSpeed;      // speed credits go up at
static UINT32 guiCrdtLastTimeUpdatingNode; // the last time a node was read from the file
static UINT8  gubCrdtJustification;        // the current justification

static UINT32 guiGapBetweenCreditSections;
static UINT32 guiGapBetweenCreditNodes;
static UINT32 guiGapTillReadNextCredit;

static UINT32  guiCurrentCreditRecord;
static BOOLEAN gfPauseCreditScreen;


static BOOLEAN EnterCreditsScreen(void);
static void ExitCreditScreen(void);
static void GetCreditScreenUserInput(void);
static void HandleCreditScreen(void);


ScreenID CreditScreenHandle(void)
{
	if (!g_credits_active)
	{
		if (!EnterCreditsScreen()) return MAINMENU_SCREEN;
	}

	GetCreditScreenUserInput();
	HandleCreditScreen();
	ExecuteBaseDirtyRectQueue();
	EndFrameBufferRender();

	if (!g_credits_active)
	{
		ExitCreditScreen();
		return MAINMENU_SCREEN;
	}

	return CREDIT_SCREEN;
}


static void InitCreditEyeBlinking(void);
static void RenderCreditScreen(void);
static void SelectCreditFaceMovementRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


static BOOLEAN EnterCreditsScreen(void)
try
{
	guiCreditBackGroundImage = AddVideoObjectFromFile(INTERFACEDIR "/credits.sti");
	guiCreditFaces           = AddVideoObjectFromFile(INTERFACEDIR "/credit faces.sti");

	guiCreditScreenActiveFont  = FONT12ARIAL;
	gubCreditScreenActiveColor = FONT_MCOLOR_DKWHITE;
	guiCreditScreenTitleFont   = FONT14ARIAL;
	gubCreditScreenTitleColor  = FONT_MCOLOR_RED;
	guiCrdtNodeScrollSpeed     = CRDT_NODE_DELAY_AMOUNT;
	gubCrdtJustification       = CENTER_JUSTIFIED;
	guiCurrentCreditRecord     = 0;

	guiCrdtLastTimeUpdatingNode = GetJA2Clock();

	guiGapBetweenCreditSections = CRDT_SPACE_BN_SECTIONS;
	guiGapBetweenCreditNodes    = CRDT_SPACE_BN_NODES;
	guiGapTillReadNextCredit    = CRDT_SPACE_BN_NODES;


	MSYS_DefineRegion(&gCrdtBackgroundRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_NORMAL, CURSOR_NORMAL, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);
	for (INT32 i = 0; i != lengthof(gCrdtMouseRegions); ++i)
	{
		// Make a mouse region
		MOUSE_REGION* const  r = &gCrdtMouseRegions[i];
		CreditFace    const& f = gCreditFaces[i];
		UINT16        const  x = f.sX + STD_SCREEN_X;
		UINT16        const  y = f.sY + STD_SCREEN_Y;
		UINT16        const  w = f.sWidth;
		UINT16        const  h = f.sHeight;
		MSYS_DefineRegion(r, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST, CURSOR_WWW, SelectCreditFaceMovementRegionCallBack, NULL);
		MSYS_SetRegionUserData(r, 0, i);
	}

	giCurrentlySelectedFace = -1;
	gfPauseCreditScreen     = FALSE;

	InitCreditEyeBlinking();

	RenderCreditScreen();
	// blit everything to the save buffer (cause the save buffer can bleed through)
	BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, NULL);

	g_credits_active = TRUE;
	return TRUE;
}
catch (...) { return FALSE; }


static void DeleteFirstNode(void);


static void ExitCreditScreen(void)
{
	DeleteVideoObject(guiCreditBackGroundImage);
	DeleteVideoObject(guiCreditFaces);

	while (g_credits_head != NULL) DeleteFirstNode();

	MSYS_RemoveRegion( &gCrdtBackgroundRegion );
	for (size_t i = 0; i != lengthof(gCrdtMouseRegions); ++i)
	{
		MSYS_RemoveRegion(&gCrdtMouseRegions[i]);
	}
}


static BOOLEAN GetNextCreditFromTextFile(void);
static void HandleCreditEyeBlinking();
static void HandleCreditNodes(void);


static void HandleCreditScreen(void)
{
	HandleCreditNodes();
	HandleCreditEyeBlinking();

	//is it time to get a new node
	if (g_credits_tail == NULL || (CRDT_START_POS_Y - (g_credits_tail->sPosY + g_credits_tail->sHeightOfString - 16)) >= (INT16)guiGapTillReadNextCredit)
	{
		//if there are no more credits in the file
		if (!GetNextCreditFromTextFile() && g_credits_tail == NULL)
		{
			g_credits_active = FALSE;
		}
	}

	RestoreExternBackgroundRect(CRDT_NAME_LOC_X, CRDT_NAME_LOC_Y, CRDT_NAME_LOC_WIDTH, CRDT_NAME_LOC_HEIGHT);

	if (giCurrentlySelectedFace != -1)
	{
		DrawTextToScreen(gzCreditNames[giCurrentlySelectedFace],     CRDT_NAME_LOC_X, CRDT_NAME_LOC_Y,       CRDT_NAME_LOC_WIDTH, CRDT_NAME_FONT, FONT_MCOLOR_WHITE, 0, INVALIDATE_TEXT | CENTER_JUSTIFIED);
		DrawTextToScreen(gzCreditNameTitle[giCurrentlySelectedFace], CRDT_NAME_LOC_X, CRDT_NAME_TITLE_LOC_Y, CRDT_NAME_LOC_WIDTH, CRDT_NAME_FONT, FONT_MCOLOR_WHITE, 0, INVALIDATE_TEXT | CENTER_JUSTIFIED);
		DrawTextToScreen(gzCreditNameFunny[giCurrentlySelectedFace], CRDT_NAME_LOC_X, CRDT_NAME_FUNNY_LOC_Y, CRDT_NAME_LOC_WIDTH, CRDT_NAME_FONT, FONT_MCOLOR_WHITE, 0, INVALIDATE_TEXT | CENTER_JUSTIFIED);
	}
}


static void RenderCreditScreen(void)
{
	BltVideoObject(FRAME_BUFFER, guiCreditBackGroundImage, 0, STD_SCREEN_X, STD_SCREEN_Y);
	InvalidateScreen();
}


static void GetCreditScreenUserInput(void)
{
	InputAtom Event;
	while (DequeueEvent(&Event))
	{
		if (Event.usEvent == KEY_DOWN)
		{
			switch (Event.usParam)
			{
				case SDLK_ESCAPE: g_credits_active = FALSE; break;
			}
		}
	}
}


static void DeleteFirstNode(void)
{
	CRDT_NODE* const del = g_credits_head;

	g_credits_head = del->next;

	if (g_credits_tail == del) g_credits_tail = NULL;

	DeleteVideoSurface(del->uiVideoSurfaceImage);
	delete del;
}


static void AddCreditNode(UINT32 uiFlags, const ST::string& pString)
{
	CRDT_NODE* const pNodeToAdd = new CRDT_NODE{};

	//Determine the font and the color to use
	SGPFont  uiFontToUse;
	UINT8 uiColorToUse;
	if (uiFlags & CRDT_FLAG__TITLE)
	{
		uiFontToUse  = guiCreditScreenTitleFont;
		uiColorToUse = gubCreditScreenTitleColor;
	}
	else
	{
		uiFontToUse  = guiCreditScreenActiveFont;
		uiColorToUse = gubCreditScreenActiveColor;
	}

	//Calculate the height of the string
	pNodeToAdd->sHeightOfString = DisplayWrappedString(0, 0, CRDT_WIDTH_OF_TEXT_AREA, 2, uiFontToUse, uiColorToUse, pString, 0, DONT_DISPLAY_TEXT) + 1;

	//starting y position on the screen
	pNodeToAdd->sPosY = CRDT_START_POS_Y;

	SGPVSurface* const vs = AddVideoSurface(CRDT_WIDTH_OF_TEXT_AREA, pNodeToAdd->sHeightOfString, PIXEL_DEPTH);
	pNodeToAdd->uiVideoSurfaceImage = vs;

	vs->SetTransparency(0);
	vs->Fill(0);

	//write the string onto the surface
	SetFontDestBuffer(vs);
	DisplayWrappedString(0, 1, CRDT_WIDTH_OF_TEXT_AREA, 2, uiFontToUse, uiColorToUse, pString, 0, gubCrdtJustification);
	SetFontDestBuffer(FRAME_BUFFER);

	if (g_credits_tail == NULL)
	{
		Assert(g_credits_head == NULL);
		g_credits_head = pNodeToAdd;
	}
	else
	{
		g_credits_tail->next = pNodeToAdd;
	}
	g_credits_tail = pNodeToAdd;
}


static void DisplayCreditNode(const CRDT_NODE*);


static void HandleCreditNodes(void)
{
	if (g_credits_head == NULL) return;

	if (gfPauseCreditScreen) return;

	if (!(GetJA2Clock() - guiCrdtLastTimeUpdatingNode > guiCrdtNodeScrollSpeed)) return;

	for (CRDT_NODE* i = g_credits_head; i != NULL; i = i->next)
	{
		DisplayCreditNode(i);
		i->sPosY -= CRDT_SCROLL_PIXEL_AMOUNT;
	}

	// Restore Background below text
	RestoreExternBackgroundRect(CRDT_TEXT_START_LOC, SCREEN_HEIGHT - STD_SCREEN_Y, CRDT_WIDTH_OF_TEXT_AREA, STD_SCREEN_Y);
	RestoreExternBackgroundRect(CRDT_TEXT_START_LOC, 0, CRDT_WIDTH_OF_TEXT_AREA, STD_SCREEN_Y);

	const CRDT_NODE* const head = g_credits_head;
	if (head->sPosY + head->sHeightOfString < 0)
	{
		DeleteFirstNode();
	}

	guiCrdtLastTimeUpdatingNode = GetJA2Clock();
}


static void DisplayCreditNode(const CRDT_NODE* const pCurrent)
{
	//Restore the background before blitting the text back on
	INT16 y = pCurrent->sPosY + CRDT_SCROLL_PIXEL_AMOUNT;
	INT16 h = pCurrent->sHeightOfString;
	/* Clip to the screen area */
	if (y < STD_SCREEN_Y)
	{
		//upper limit of screen
		h += (y - STD_SCREEN_Y);
		y = STD_SCREEN_Y;
	}
	else if (y + h > (SCREEN_HEIGHT - STD_SCREEN_Y))
	{
		//lower limit of the screen
		h = SCREEN_HEIGHT - STD_SCREEN_Y - y;
	}
	RestoreExternBackgroundRect(CRDT_TEXT_START_LOC, y, CRDT_WIDTH_OF_TEXT_AREA, h);

	BltVideoSurface(FRAME_BUFFER, pCurrent->uiVideoSurfaceImage, CRDT_TEXT_START_LOC, pCurrent->sPosY, NULL);
}


static UINT32 GetNumber(const char* string)
{
	unsigned int v = 0;
	sscanf(string, "%u", &v);
	return v;
}


static void HandleCreditFlags(UINT32 uiFlags);


static BOOLEAN GetNextCreditFromTextFile(void)
{
	ST::string text;
	const UINT32 pos = CREDITS_LINESIZE * guiCurrentCreditRecord++;
	try
	{
		text = GCM->loadEncryptedString(CRDT_NAME_OF_CREDIT_FILE, pos, CREDITS_LINESIZE);
	}
	catch (...) // XXX fishy, should check file size beforehand
	{
		return FALSE;
	}

	UINT32         flags = 0;
	const char* s = text.c_str();
	if (*s == CRDT_START_CODE)
	{
		for (;;)
		{
			++s;
			/* process code */
			switch (*s++)
			{
				case CRDT_DELAY_BN_STRINGS_CODE:  guiGapBetweenCreditNodes    = GetNumber(s); break;
				case CRDT_DELAY_BN_SECTIONS_CODE: guiGapBetweenCreditSections = GetNumber(s); break;
				case CRDT_SCROLL_SPEED:           guiCrdtNodeScrollSpeed      = GetNumber(s); break;
				case CRDT_TITLE_FONT_COLOR:       gubCreditScreenTitleColor   = GetNumber(s); break;
				case CRDT_ACTIVE_FONT_COLOR:      gubCreditScreenActiveColor  = GetNumber(s); break;

				case CRDT_FONT_JUSTIFICATION:
					switch (GetNumber(s))
					{
						case 0:  gubCrdtJustification = LEFT_JUSTIFIED;   break;
						case 1:  gubCrdtJustification = CENTER_JUSTIFIED; break;
						case 2:  gubCrdtJustification = RIGHT_JUSTIFIED;  break;
						default: SLOGA("error parsing credits file (sub)"); break;
					}
					break;

				case CRDT_TITLE:            flags |= CRDT_FLAG__TITLE;         break;
				case CRDT_START_OF_SECTION: flags |= CRDT_FLAG__START_SECTION; break;
				case CRDT_END_OF_SECTION:   flags |= CRDT_FLAG__END_SECTION;   break;

				default: SLOGA("error parsing credits file"); break;
			}

			/* skip till the next code or end of codes */
			while (*s != CRDT_SEPARATION_CODE)
			{
				switch (*s)
				{
					case CRDT_END_CODE:  ++s; goto handle_text;
					case '\0':               goto handle_text;
					default:             ++s; break;
				}
			}
		}
	}

handle_text:
	if (*s != '\0') AddCreditNode(flags, s);
	HandleCreditFlags(flags);
	return TRUE;
}


static void HandleCreditFlags(UINT32 uiFlags)
{
	if (uiFlags & CRDT_FLAG__START_SECTION)
	{
		guiGapTillReadNextCredit = guiGapBetweenCreditNodes;
	}

	if (uiFlags & CRDT_FLAG__END_SECTION)
	{
		guiGapTillReadNextCredit = guiGapBetweenCreditSections;
	}
}


static void SelectCreditFaceMovementRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE)
	{
		giCurrentlySelectedFace = -1;
	}
	else if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE)
	{
		giCurrentlySelectedFace = MSYS_GetRegionUserData(pRegion, 0);
	}
}


static void InitCreditEyeBlinking(void)
{
	const UINT32 now = GetJA2Clock();
	FOR_EACH(CreditFace, f, gCreditFaces)
	{
		f->uiLastBlinkTime = now + Random(f->sBlinkFreq * 2);
	}
}


static void HandleCreditEyeBlinking()
{
	UINT16 gfx = 0;
	FOR_EACHX(CreditFace, i, gCreditFaces, gfx += 3)
	{
		CreditFace&  f   = *i;
		UINT32 const now = GetJA2Clock();
		if (now - f.uiLastBlinkTime > f.sBlinkFreq)
		{
			INT32 const x = f.sEyeX + STD_SCREEN_X;
			INT32 const y = f.sEyeY + STD_SCREEN_Y;
			BltVideoObject(FRAME_BUFFER, guiCreditFaces, gfx, x, y);
			InvalidateRegion(x, y, x + CRDT_EYE_WIDTH, y + CRDT_EYE_HEIGHT);

			f.uiLastBlinkTime  = now;
			f.uiEyesClosedTime = now + CRDT_EYES_CLOSED_TIME + Random(CRDT_EYES_CLOSED_TIME);
		}
		else if (now > f.uiEyesClosedTime)
		{
			RestoreExternBackgroundRect(f.sEyeX + STD_SCREEN_X, f.sEyeY + STD_SCREEN_Y, CRDT_EYE_WIDTH, CRDT_EYE_HEIGHT);

			f.uiEyesClosedTime = 0;
		}
	}
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(Credits, asserts)
{
	EXPECT_EQ(lengthof(gCreditFaces), NUM_PEOPLE_IN_CREDITS);
}

#endif
