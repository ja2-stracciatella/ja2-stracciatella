#include "Font.h"
#include "HImage.h"
#include "Types.h"
#include "Animated_ProgressBar.h"
#include "Debug.h"
#include "Font_Control.h"
#include "VObject.h"
#include "VSurface.h"
#include "Video.h"
#include "Music_Control.h"
#include "Timer_Control.h"
#include "UILayout.h"

#include <string_theory/string>


#define MAX_PROGRESSBARS 4

enum ProgressBarFlags
{
	PROGRESS_NONE           = 0,
	PROGRESS_PANEL          = 1 << 0,
	PROGRESS_LOAD_BAR       = 1 << 3
};
ENUM_BITSET(ProgressBarFlags)

struct PROGRESSBAR
{
	ProgressBarFlags flags;
	SGPBox           pos;
	UINT16 usPanelLeft, usPanelTop, usPanelRight, usPanelBottom;
	UINT16 usColor, usLtColor, usDkColor;
	ST::string title;
	SGPFont usTitleFont;
	UINT8 ubTitleFontForeColor, ubTitleFontShadowColor;
	SGPFont usMsgFont;
	UINT8 ubMsgFontForeColor, ubMsgFontShadowColor;
	UINT32 fill_colour;
	double rStart, rEnd;
	double rLastActual;
};

static PROGRESSBAR* pBar[MAX_PROGRESSBARS];


void CreateLoadingScreenProgressBar()
{
	CreateProgressBar(0, STD_SCREEN_X + 162, STD_SCREEN_Y + 427, 318, 16);
	pBar[0]->flags |= PROGRESS_LOAD_BAR;
}

void RemoveLoadingScreenProgressBar()
{
	RemoveProgressBar( 0 );
	SetFontShadow(DEFAULT_SHADOW);
}


void CreateProgressBar(const UINT8 ubProgressBarID, const UINT16 x, const UINT16 y, const UINT16 w, const UINT16 h)
{
	PROGRESSBAR* const pNew = new PROGRESSBAR{};

	if( pBar[ ubProgressBarID ] )
		RemoveProgressBar( ubProgressBarID );

	pBar[ ubProgressBarID ] = pNew;
	//Assign coordinates
	pNew->flags                = PROGRESS_NONE;
	pNew->pos.x                = x;
	pNew->pos.y                = y;
	pNew->pos.w                = w;
	pNew->pos.h                = h;
	//Init default data
	pNew->usMsgFont            = FONT12POINT1;
	pNew->ubMsgFontForeColor   = FONT_BLACK;
	pNew->ubMsgFontShadowColor = 0;
	SetRelativeStartAndEndPercentage(ubProgressBarID, 0, 100, {});
	pNew->title.clear();

	//Default the progress bar's color to be red
	pNew->fill_colour = FROMRGB(150, 0, 0);
}


//You may also define a panel to go in behind the progress bar.  You can now assign a title to go with
//the panel.
void DefineProgressBarPanel( UINT32 ubID, UINT8 r, UINT8 g, UINT8 b,
				UINT16 usLeft, UINT16 usTop, UINT16 usRight, UINT16 usBottom )
{
	PROGRESSBAR *pCurr;
	Assert( ubID < MAX_PROGRESSBARS );
	pCurr = pBar[ ubID ];
	if( !pCurr )
		return;

	pCurr->flags |= PROGRESS_PANEL;
	pCurr->usPanelLeft = usLeft;
	pCurr->usPanelTop = usTop;
	pCurr->usPanelRight = usRight;
	pCurr->usPanelBottom = usBottom;
	pCurr->usColor = Get16BPPColor( FROMRGB( r, g, b ) );
	//Calculate the slightly lighter and darker versions of the same rgb color
	pCurr->usLtColor = Get16BPPColor(FROMRGB((UINT8) std::min(255, (int) (r * 1.33)),
						(UINT8) std::min(255, (int) (g * 1.33)),
						(UINT8) std::min(255, (int) (b * 1.33))));
	pCurr->usDkColor = Get16BPPColor( FROMRGB( (UINT8)(r*0.75), (UINT8)(g*0.75), (UINT8)(b*0.75) ) );
}

//Assigning a title for the panel will automatically position the text horizontally centered on the
//panel and vertically centered from the top of the panel, to the top of the progress bar.
void SetProgressBarTitle(UINT32 ubID, const ST::string& str, SGPFont font, UINT8 ubForeColor, UINT8 ubShadowColor)
{
	PROGRESSBAR *pCurr;
	Assert( ubID < MAX_PROGRESSBARS );
	pCurr = pBar[ ubID ];
	if( !pCurr )
		return;
	pCurr->title = str;
	pCurr->usTitleFont = font;
	pCurr->ubTitleFontForeColor = ubForeColor;
	pCurr->ubTitleFontShadowColor = ubShadowColor;
}

//Unless you set up the attributes, any text you pass to SetRelativeStartAndEndPercentage will
//default to FONT12POINT1 in a black color.
void SetProgressBarMsgAttributes(UINT32 ubID, SGPFont const font, UINT8 ubForeColor, UINT8 ubShadowColor)
{
	PROGRESSBAR *pCurr;
	Assert( ubID < MAX_PROGRESSBARS );
	pCurr = pBar[ ubID ];
	if( !pCurr )
		return;
	pCurr->usMsgFont            = font;
	pCurr->ubMsgFontForeColor = ubForeColor;
	pCurr->ubMsgFontShadowColor = ubShadowColor;
}


//When finished, the progress bar needs to be removed.
void RemoveProgressBar( UINT8 ubID )
{
	Assert( ubID < MAX_PROGRESSBARS );
	if( pBar[ubID] )
	{
		delete pBar[ubID];
		pBar[ubID] = NULL;
		return;
	}
}

/* An important setup function.  The best explanation is through example.  The
 * example being the loading of a file -- there are many stages of the map
 * loading.  In JA2, the first step is to load the tileset.  Because it is a
 * large chunk of the total loading of the map, we may gauge that it takes up
 * 30% of the total load.  Because it is also at the beginning, we would pass in
 * the arguments (0, 30, "text").  As the process animates using
 * UpdateProgressBar(0 to 100), the total progress bar will only reach 30% at
 * the 100% mark within UpdateProgressBar.  At that time, you would go onto the
 * next step, resetting the relative start and end percentage from 30 to
 * whatever, until your done. */
void SetRelativeStartAndEndPercentage(UINT8 id, UINT32 uiRelStartPerc, UINT32 uiRelEndPerc, ST::string const&)
{
	Assert(id < MAX_PROGRESSBARS);
	PROGRESSBAR* const bar = pBar[id];
	if (!bar) return;

	bar->rStart = uiRelStartPerc * 0.01;
	bar->rEnd   = uiRelEndPerc   * 0.01;

	//Render the entire panel now, as it doesn't need update during the normal rendering
	if (bar->flags & PROGRESS_PANEL)
	{ // Draw panel
		UINT16 const l = bar->usPanelLeft;
		UINT16 const t = bar->usPanelTop;
		UINT16 const r = bar->usPanelRight;
		UINT16 const b = bar->usPanelBottom;
		ColorFillVideoSurfaceArea(FRAME_BUFFER, l,     t,     r,     b,     bar->usLtColor);
		ColorFillVideoSurfaceArea(FRAME_BUFFER, l + 1, t + 1, r,     b,     bar->usDkColor);
		ColorFillVideoSurfaceArea(FRAME_BUFFER, l + 1, t + 1, r - 1, b - 1, bar->usColor);
		InvalidateRegion(l, t, r, b);
		if (!bar->title.empty())
		{ // Draw title
			SGPFont  const font = bar->usTitleFont;
			INT32 const x    = (r + l - StringPixLength(bar->title, font)) / 2; // Center
			SetFontAttributes(font, bar->ubTitleFontForeColor, bar->ubTitleFontShadowColor);
			MPrint(x, t + 3, bar->title);
		}
	}
}


//This part renders the progress bar at the percentage level that you specify.  If you have set relative
//percentage values in the above function, then the uiPercentage will be reflected based off of the relative
//percentages.
void RenderProgressBar( UINT8 ubID, UINT32 uiPercentage )
{
	static UINT32 uiLastTime = 0;
	UINT32 uiCurTime = GetJA2Clock();
	double rActual;
	PROGRESSBAR *pCurr=NULL;
	//UINT32 r, g;

	Assert( ubID < MAX_PROGRESSBARS );
	pCurr = pBar[ubID];

	if( pCurr == NULL )
		return;

	if( pCurr )
	{
		rActual = pCurr->rStart+(pCurr->rEnd-pCurr->rStart)*uiPercentage*0.01;

		if( rActual - pCurr->rLastActual < 0.01 )
		{
			return;
		}

		pCurr->rLastActual = ( double )( ( INT32)( rActual * 100 ) * 0.01 );

		INT32 const x   = pCurr->pos.x;
		INT32 const y   = pCurr->pos.y;
		INT32 const w   = pCurr->pos.w;
		INT32 const h   = pCurr->pos.h;
		INT32 const end = (INT32)(x + 2.0 + rActual * (w - 4));
		if (end < x + 2 || x + w - 2 < end) return;
		if (pCurr->flags & PROGRESS_LOAD_BAR)
		{
			ColorFillVideoSurfaceArea(FRAME_BUFFER, x, y, end, y + h, Get16BPPColor(pCurr->fill_colour));
		}
		else
		{
			//Border edge of the progress bar itself in gray
			ColorFillVideoSurfaceArea( FRAME_BUFFER,
				x, y, x + w, y + h,
				Get16BPPColor(FROMRGB(160, 160, 160)) );
			//Interior of progress bar in black
			ColorFillVideoSurfaceArea( FRAME_BUFFER,
				x + 2, y + 2, x + w - 2, y + h - 2,
				Get16BPPColor(FROMRGB(  0,   0,   0)) );
			ColorFillVideoSurfaceArea(FRAME_BUFFER,	x + 2, y + 2, end, y + h - 2, Get16BPPColor(FROMRGB(72 , 155, 24)));
		}
		InvalidateRegion(x, y, x + w, y + h);
		RefreshScreen();
	}

	// update music here
	if( uiCurTime > ( uiLastTime + 200 ) )
	{
		MusicPoll();
		uiLastTime = GetJA2Clock();
	}
}

void SetProgressBarColor( UINT8 ubID, UINT8 ubColorFillRed, UINT8 ubColorFillGreen, UINT8 ubColorFillBlue )
{
	PROGRESSBAR *pCurr=NULL;

	Assert( ubID < MAX_PROGRESSBARS );

	pCurr = pBar[ubID];
	if( pCurr == NULL )
		return;

	pCurr->fill_colour = FROMRGB(ubColorFillRed, ubColorFillGreen, ubColorFillBlue);
}
