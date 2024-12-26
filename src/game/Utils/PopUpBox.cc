#include "Font.h"
#include "PopUpBox.h"
#include "SysUtil.h"
#include "Debug.h"
#include "VObject.h"
#include "Video.h"
#include "VSurface.h"
#include "UILayout.h"

#include <string_theory/string>

#include <stdexcept>


#define MAX_POPUP_BOX_COUNT 20


struct PopUpString
{
	ST::utf32_buffer codepoints;
	UINT8 ubForegroundColor;
	UINT8 ubBackgroundColor;
	UINT8 ubHighLight;
	UINT8 ubShade;
	UINT8 ubSecondaryShade;
	BOOLEAN fHighLightFlag;
	BOOLEAN fShadeFlag;
	BOOLEAN fSecondaryShadeFlag;
};

struct PopUpBox
{
	SGPBox pos;
	UINT32 uiLeftMargin;
	UINT32 uiRightMargin;
	UINT32 uiBottomMargin;
	UINT32 uiTopMargin;
	UINT32 uiLineSpace;
	const SGPVObject* iBorderObjectIndex;
	SGPVSurface* iBackGroundSurface;
	UINT32 uiFlags;
	SGPVSurface* uiBuffer;
	UINT32 uiSecondColumnMinimunOffset;
	UINT32 uiSecondColumnCurrentOffset;
	UINT32 uiBoxMinWidth;
	BOOLEAN fUpdated;
	BOOLEAN fShowBox;
	SGPFont font;

	PopUpString* Text[MAX_POPUP_BOX_STRING_COUNT];
	PopUpString* pSecondColumnString[MAX_POPUP_BOX_STRING_COUNT];
};

static PopUpBox* PopUpBoxList[MAX_POPUP_BOX_COUNT];

#define FOR_EACH_POPUP_BOX(iter) \
	FOR_EACH(PopUpBox*, iter, PopUpBoxList) \
		if (*iter == NULL) continue; else


#define BORDER_WIDTH  16
#define BORDER_HEIGHT  8
#define TOP_LEFT_CORNER     0
#define TOP_EDGE            4
#define TOP_RIGHT_CORNER    1
#define SIDE_EDGE           5
#define BOTTOM_LEFT_CORNER  2
#define BOTTOM_EDGE         4
#define BOTTOM_RIGHT_CORNER 3


UINT32 GetLineSpace(const PopUpBox* const box)
{
	// return number of pixels between lines for this box
	return box->uiLineSpace;
}


void SpecifyBoxMinWidth(PopUpBox* const box, INT32 iMinWidth)
{
	box->uiBoxMinWidth = iMinWidth;

	// check if the box is currently too small
	if (box->pos.w < iMinWidth) box->pos.w = iMinWidth;
}


PopUpBox* CreatePopUpBox(const SGPPoint Position, const UINT32 uiFlags, SGPVSurface* const buffer, const SGPVObject* const border, SGPVSurface* const background, const UINT32 margin_l, const UINT32 margin_t, const UINT32 margin_b, const UINT32 margin_r, const UINT32 line_space)
{
	// find first free box
	FOR_EACH(PopUpBox*, i, PopUpBoxList)
	{
		if (*i == NULL)
		{
			PopUpBox* const box = new PopUpBox{};
			SetBoxXY(box, Position.iX, Position.iY);
			box->uiFlags            = uiFlags;
			box->uiBuffer           = buffer;
			box->iBorderObjectIndex = border;
			box->iBackGroundSurface = background;
			box->uiLeftMargin       = margin_l;
			box->uiRightMargin      = margin_r;
			box->uiTopMargin        = margin_t;
			box->uiBottomMargin     = margin_b;
			box->uiLineSpace        = line_space;

			*i = box;
			return box;
		}
	}

	// ran out of available popup boxes - probably not freeing them up right!
	throw std::runtime_error("Out of popup box slots");
}


UINT32 GetTopMarginSize(const PopUpBox* const box)
{
	// return size of top margin, for mouse region offsets
	return box->uiTopMargin;
}


void ShadeStringInBox(PopUpBox* const box, INT32 const line, bool const shade)
{
	PopUpString* const s = box->Text[line];
	if (s) s->fShadeFlag = shade;
}


void ShadeStringInBox(PopUpBox* const box, INT32 const line, PopUpShade const shade)
{
	PopUpString* const s = box->Text[line];
	if (!s) return;

	s->fShadeFlag          = shade == POPUP_SHADE;
	s->fSecondaryShadeFlag = shade == POPUP_SHADE_SECONDARY;
}


void SetBoxXY(PopUpBox* const box, const INT16 x, const INT16 y)
{
	box->pos.x    = x;
	box->pos.y    = y;
	box->fUpdated = false;
}


void SetBoxX(PopUpBox* const box, const INT16 x)
{
	box->pos.x    = x;
	box->fUpdated = false;
}


void SetBoxY(PopUpBox* const box, const INT16 y)
{
	box->pos.y    = y;
	box->fUpdated = false;
}


SGPBox const& GetBoxArea(PopUpBox const* const box)
{
	return box->pos;
}


// adds a FIRST column string to the CURRENT popup box
void AddMonoString(PopUpBox* box, const ST::string& str)
{
	INT32 iCounter = 0;

	// find first free slot in list
	for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT && box->Text[iCounter] != NULL; iCounter++);

	if ( iCounter >= MAX_POPUP_BOX_STRING_COUNT )
	{
		// using too many text lines, or not freeing them up properly
		SLOGA("AddMonoString: too many text lines");
		return;
	}

	PopUpString* const pStringSt    = new PopUpString{};

	box->Text[iCounter]                      = pStringSt;
	box->Text[iCounter]->codepoints          = str.to_utf32();
	box->Text[iCounter]->fShadeFlag          = false;
	box->Text[iCounter]->fHighLightFlag      = false;
	box->Text[iCounter]->fSecondaryShadeFlag = false;

	box->fUpdated = false;
}


static void RemoveBoxSecondaryText(PopUpBox*, INT32 hStringHandle);


void AddSecondColumnMonoString(PopUpBox* box, const ST::string& str)
{
	INT32 iCounter=0;

	// find the LAST USED text string index
	for (iCounter = 0; iCounter + 1 < MAX_POPUP_BOX_STRING_COUNT && box->Text[iCounter + 1] != NULL; iCounter++);

	if ( iCounter >= MAX_POPUP_BOX_STRING_COUNT )
	{
		// using too many text lines, or not freeing them up properly
		SLOGA("AddSecondColumnMonoString: too many text lines");
		return;
	}

	PopUpString* const pStringSt    = new PopUpString{};

	RemoveBoxSecondaryText(box, iCounter);

	box->pSecondColumnString[iCounter]                 = pStringSt;
	box->pSecondColumnString[iCounter]->codepoints     = str.to_utf32();
	box->pSecondColumnString[iCounter]->fShadeFlag     = false;
	box->pSecondColumnString[iCounter]->fHighLightFlag = false;
}


UINT32 GetNumberOfLinesOfTextInBox(const PopUpBox* const box)
{
	INT32 iCounter = 0;

	// count number of lines
	// check string size
	for( iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++ )
	{
		if (box->Text[iCounter] == NULL) break;
	}

	return( iCounter );
}


void SetBoxFont(PopUpBox* const box, SGPFont const font)
{
	box->font     = font;
	box->fUpdated = false;
}


void SetBoxSecondColumnMinimumOffset(PopUpBox* const box, const UINT32 uiWidth)
{
	box->uiSecondColumnMinimunOffset = uiWidth;
}


SGPFont GetBoxFont(const PopUpBox* const box)
{
	return box->font;
}


// set the foreground color of this string in this pop up box
void SetBoxLineForeground(PopUpBox* const box, const INT32 iStringValue, const UINT8 ubColor)
{
	box->Text[iStringValue]->ubForegroundColor = ubColor;
}


void SetBoxSecondaryShade(PopUpBox* const box, UINT8 const colour)
{
	FOR_EACH(PopUpString*, i, box->Text)
	{
		PopUpString* const p = *i;
		if (p) p->ubSecondaryShade = colour;
	}
}


void SetBoxForeground(PopUpBox* const box, UINT8 const colour)
{
	FOR_EACH(PopUpString*, i, box->Text)
	{
		PopUpString* const p = *i;
		if (p) p->ubForegroundColor = colour;
	}
}


void SetBoxBackground(PopUpBox* const box, UINT8 const colour)
{
	FOR_EACH(PopUpString*, i, box->Text)
	{
		PopUpString* const p = *i;
		if (p) p->ubBackgroundColor = colour;
	}
}


void SetBoxHighLight(PopUpBox* const box, UINT8 const colour)
{
	FOR_EACH(PopUpString*, i, box->Text)
	{
		PopUpString* const p = *i;
		if (p) p->ubHighLight = colour;
	}
}


void SetBoxShade(PopUpBox* const box, UINT8 const colour)
{
	FOR_EACH(PopUpString*, i, box->Text)
	{
		PopUpString* const p = *i;
		if (p) p->ubShade = colour;
	}
}


void SetBoxSecondColumnForeground(PopUpBox* const box, UINT8 const colour)
{
	FOR_EACH(PopUpString*, i, box->pSecondColumnString)
	{
		PopUpString* const p = *i;
		if (p) p->ubForegroundColor = colour;
	}
}


void SetBoxSecondColumnBackground(PopUpBox* const box, UINT8 const colour)
{
	FOR_EACH(PopUpString*, i, box->pSecondColumnString)
	{
		PopUpString* const p = *i;
		if (p) p->ubBackgroundColor = colour;
	}
}


void SetBoxSecondColumnHighLight(PopUpBox* const box, UINT8 const colour)
{
	FOR_EACH(PopUpString*, i, box->pSecondColumnString)
	{
		PopUpString* const p = *i;
		if (p) p->ubHighLight = colour;
	}
}


void SetBoxSecondColumnShade(PopUpBox* const box, UINT8 const colour)
{
	FOR_EACH(PopUpString*, i, box->pSecondColumnString)
	{
		PopUpString* const p = *i;
		if (p) p->ubShade = colour;
	}
}


void HighLightBoxLine(PopUpBox* const box, const INT32 iLineNumber)
{
	// highlight iLineNumber Line in box
	PopUpString* const line = box->Text[iLineNumber];
	if (line != NULL)
	{
		line->fHighLightFlag = true;
	}
}


BOOLEAN GetBoxShadeFlag(const PopUpBox* const box, const INT32 iLineNumber)
{
	if (box->Text[iLineNumber] != NULL)
	{
		return box->Text[iLineNumber]->fShadeFlag;
	}
	return false;
}


void UnHighLightBox(PopUpBox* const box)
{
	FOR_EACH(PopUpString*, i, box->Text)
	{
		PopUpString* const p = *i;
		if (p) p->fHighLightFlag = false;
	}
}


static void RemoveBoxPrimaryText(PopUpBox*, INT32 hStringHandle);


void RemoveAllBoxStrings(PopUpBox* const box)
{
	for (UINT32 i = 0; i < MAX_POPUP_BOX_STRING_COUNT; ++i)
	{
		RemoveBoxPrimaryText(box, i);
		RemoveBoxSecondaryText(box, i);
	}
	box->fUpdated = false;
}


void RemoveBox(PopUpBox* const box)
{
	FOR_EACH_POPUP_BOX(i)
	{
		if (*i == box)
		{
			*i = NULL;
			RemoveAllBoxStrings(box);
			delete box;
			return;
		}
	}
	SLOGA("RemoveBox: box doesn't exist");
}


void ShowBox(PopUpBox* const box)
{
	if (!box->fShowBox)
	{
		box->fShowBox = true;
		box->fUpdated = false;
	}
}


void HideBox(PopUpBox* const box)
{
	if (box->fShowBox)
	{
		box->fShowBox = false;
		box->fUpdated = false;
	}
}


void DisplayBoxes(SGPVSurface* const uiBuffer)
{
	FOR_EACH_POPUP_BOX(i)
	{
		DisplayOnePopupBox(*i, uiBuffer);
	}
}


static void DrawBox(const PopUpBox*);
static void DrawBoxText(const PopUpBox*);


void DisplayOnePopupBox(PopUpBox* const box, SGPVSurface* const uiBuffer)
{
	if (!box->fUpdated && box->fShowBox && box->uiBuffer == uiBuffer)
	{
		box->fUpdated = true;
		if (box->uiFlags & POPUP_BOX_FLAG_RESIZE) ResizeBoxToText(box);
		DrawBox(box);
		DrawBoxText(box);
	}
}


void ForceUpDateOfBox(PopUpBox* const box)
{
	box->fUpdated = false;
}


static void DrawBox(const PopUpBox* const box)
{
	const UINT16 x = box->pos.x;
	const UINT16 y = box->pos.y;
	UINT16       w = box->pos.w;
	const UINT16 h = box->pos.h;

	// make sure it will fit on screen!
	Assert(x + w <= SCREEN_WIDTH);
	Assert(y + h <= SCREEN_HEIGHT);

	// subtract 4 because the 2 2-pixel corners are handled separately
	const UINT32 uiNumTilesWide = (w - 4) / BORDER_WIDTH;
	const UINT32 uiNumTilesHigh = (h - 4) / BORDER_HEIGHT;

	SGPVSurface* const dst = box->uiBuffer;

	// blit in texture first, then borders
	SGPBox const clip = { 0, 0, w, h };
	BltVideoSurface(dst, box->iBackGroundSurface, x, y, &clip);

	const SGPVObject* const border = box->iBorderObjectIndex;

	// blit in 4 corners (they're 2x2 pixels)
	BltVideoObject(dst, border, TOP_LEFT_CORNER,     x,         y);
	BltVideoObject(dst, border, TOP_RIGHT_CORNER,    x + w - 2, y);
	BltVideoObject(dst, border, BOTTOM_RIGHT_CORNER, x + w - 2, y + h - 2);
	BltVideoObject(dst, border, BOTTOM_LEFT_CORNER,  x,         y + h - 2);

	// blit in edges
	if (uiNumTilesWide > 0)
	{
		// full pieces
		for (UINT32 i = 0; i < uiNumTilesWide; ++i)
		{
			const INT32 lx = x + 2 + i * BORDER_WIDTH;
			BltVideoObject(dst, border, TOP_EDGE,    lx, y);
			BltVideoObject(dst, border, BOTTOM_EDGE, lx, y + h - 2);
		}

		// partial pieces
		const INT32 lx = x + w - 2 - BORDER_WIDTH;
		BltVideoObject(dst, border, TOP_EDGE,    lx, y);
		BltVideoObject(dst, border, BOTTOM_EDGE, lx, y + h - 2);
	}
	if (uiNumTilesHigh > 0)
	{
		// full pieces
		for (UINT32 i = 0; i < uiNumTilesHigh; ++i)
		{
			const INT32 ly = y + 2 + i * BORDER_HEIGHT;
			BltVideoObject(dst, border, SIDE_EDGE, x,         ly);
			BltVideoObject(dst, border, SIDE_EDGE, x + w - 2, ly);
		}

		// partial pieces
		const INT32 ly = y + h - 2 - BORDER_HEIGHT;
		BltVideoObject(dst, border, SIDE_EDGE, x,         ly);
		BltVideoObject(dst, border, SIDE_EDGE, x + w - 2, ly);
	}

	InvalidateRegion(x, y, x + w, y + h);
}


static void DrawBoxText(const PopUpBox* const box)
{
	SGPFont const font = box->font;
	INT32 const tlx  = box->pos.x + box->uiLeftMargin;
	INT32 const tly  = box->pos.y + box->uiTopMargin;
	INT32 const brx  = box->pos.x + box->pos.w - box->uiRightMargin;
	INT32 const bry  = box->pos.y + box->pos.h - box->uiBottomMargin;
	INT32 const w    = box->pos.w - (box->uiRightMargin + box->uiLeftMargin + 2);
	INT32 const h    = GetFontHeight(font);

	SetFont(font);
	SetFontDestBuffer(box->uiBuffer, tlx - 1, tly, brx, bry);

	for (UINT32 i = 0; i < MAX_POPUP_BOX_STRING_COUNT; ++i)
	{
		// there is text in this line?
		const PopUpString* const text = box->Text[i];
		if (text)
		{
			// are we highlighting?...shading?..or neither
			if (text->fHighLightFlag)
			{
				SetFontForeground(text->ubHighLight);
			}
			else if (text->fSecondaryShadeFlag)
			{
				SetFontForeground(text->ubSecondaryShade);
			}
			else if (text->fShadeFlag)
			{
				SetFontForeground(text->ubShade);
			}
			else
			{
				SetFontForeground(text->ubForegroundColor);
			}

			SetFontBackground(text->ubBackgroundColor);

			const INT32 y = tly + i * (h + box->uiLineSpace);
			INT16 uX;
			INT16 uY;
			if (box->uiFlags & POPUP_BOX_FLAG_CENTER_TEXT)
			{
				FindFontCenterCoordinates(tlx, y, w, h, text->codepoints, font, &uX, &uY);
			}
			else
			{
				uX = tlx;
				uY = y;
			}
			MPrint(uX, uY, text->codepoints);
		}

		// there is secondary text in this line?
		const PopUpString* const second = box->pSecondColumnString[i];
		if (second)
		{
			// are we highlighting?...shading?..or neither
			if (second->fHighLightFlag)
			{
				SetFontForeground(second->ubHighLight);
			}
			else if (second->fShadeFlag)
			{
				SetFontForeground(second->ubShade);
			}
			else
			{
				SetFontForeground(second->ubForegroundColor);
			}

			SetFontBackground(second->ubBackgroundColor);

			const INT32 y = tly + i * (h + box->uiLineSpace);
			INT16 uX;
			INT16 uY;
			if (box->uiFlags & POPUP_BOX_FLAG_CENTER_TEXT)
			{
				FindFontCenterCoordinates(tlx, y, w, h, second->codepoints, font, &uX, &uY);
			}
			else
			{
				uX = tlx + box->uiSecondColumnCurrentOffset;
				uY = y;
			}
			MPrint(uX, uY, second->codepoints);
		}
	}

	if (box->uiBuffer != guiSAVEBUFFER)
	{
		InvalidateRegion(tlx - 1, tly, brx, bry);
	}

	SetFontDestBuffer(FRAME_BUFFER);
}


void ResizeBoxToText(PopUpBox* const box)
{
	SGPFont const font = box->font;
	UINT32 max_lw = 0; // width of left  column
	UINT32 max_rw = 0; // width of right column
	UINT i;
	for (i = 0; i < MAX_POPUP_BOX_STRING_COUNT; ++i)
	{
		const PopUpString* const l = box->Text[i];
		if (l == NULL) break;

		const UINT32 lw = StringPixLength(l->codepoints, font);
		if (lw > max_lw) max_lw = lw;

		const PopUpString* const r = box->pSecondColumnString[i];
		if (r != NULL)
		{
			const UINT32 rw = StringPixLength(r->codepoints, font);
			if (rw > max_rw) max_rw = rw;
		}
	}

	const UINT32 r_off = max_lw + box->uiSecondColumnMinimunOffset;
	box->uiSecondColumnCurrentOffset = r_off;

	UINT32 w = box->uiLeftMargin + r_off + max_rw + box->uiRightMargin;
	if (w < box->uiBoxMinWidth) w = box->uiBoxMinWidth;
	box->pos.w = w;

	box->pos.h = box->uiTopMargin + i * (GetFontHeight(font) + box->uiLineSpace) + box->uiBottomMargin;
}


BOOLEAN IsBoxShown(const PopUpBox* const box)
{
	if (box == NULL) return false;
	return box->fShowBox;
}


void MarkAllBoxesAsAltered( void )
{
	FOR_EACH_POPUP_BOX(i)
	{
		ForceUpDateOfBox(*i);
	}
}


void HideAllBoxes( void )
{
	FOR_EACH_POPUP_BOX(i)
	{
		HideBox(*i);
	}
}


static void RemoveBoxPrimaryText(PopUpBox* const Box, const INT32 hStringHandle)
{
	Assert(Box != NULL);
	Assert( hStringHandle < MAX_POPUP_BOX_STRING_COUNT );

	// remove & release primary text
	if (Box->Text[hStringHandle] != NULL)
	{
		delete Box->Text[hStringHandle];
		Box->Text[hStringHandle] = NULL;
	}
}


static void RemoveBoxSecondaryText(PopUpBox* const Box, const INT32 hStringHandle)
{
	Assert(Box != NULL);
	Assert( hStringHandle < MAX_POPUP_BOX_STRING_COUNT );

	// remove & release secondary strings
	if (Box->pSecondColumnString[hStringHandle] != NULL)
	{
		delete Box->pSecondColumnString[hStringHandle];
		Box->pSecondColumnString[hStringHandle] = NULL;
	}
}
