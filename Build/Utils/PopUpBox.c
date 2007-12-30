#include "Font.h"
#include "Local.h"
#include "PopUpBox.h"
#include "SysUtil.h"
#include "Debug.h"
#include "VObject.h"
#include "Video.h"
#include "MemMan.h"
#include "VSurface.h"
#include "WCheck.h"


#define MAX_POPUP_BOX_COUNT 20


typedef struct PopUpString {
	STR16 pString;
	UINT8 ubForegroundColor;
	UINT8 ubBackgroundColor;
	UINT8 ubHighLight;
	UINT8 ubShade;
	UINT8 ubSecondaryShade;
	UINT32 uiFont;
	BOOLEAN fHighLightFlag;
	BOOLEAN fShadeFlag;
	BOOLEAN fSecondaryShadeFlag;
} PopUpString;

typedef struct PopUpBox {
	SGPRect  Dimensions;
	SGPPoint Position;
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

	PopUpString* Text[MAX_POPUP_BOX_STRING_COUNT];
	PopUpString* pSecondColumnString[MAX_POPUP_BOX_STRING_COUNT];
} PopUpBox;

static PopUpBox* PopUpBoxList[MAX_POPUP_BOX_COUNT];


#define BORDER_WIDTH  16
#define BORDER_HEIGHT  8
#define TOP_LEFT_CORNER     0
#define TOP_EDGE            4
#define TOP_RIGHT_CORNER    1
#define SIDE_EDGE           5
#define BOTTOM_LEFT_CORNER  2
#define BOTTOM_EDGE         4
#define BOTTOM_RIGHT_CORNER 3


void SetLineSpace(INT32 hBoxHandle, UINT32 uiLineSpace)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);
	Box->uiLineSpace = uiLineSpace;
}


UINT32 GetLineSpace( INT32 hBoxHandle )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return( 0 );

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);
	// return number of pixels between lines for this box
	return Box->uiLineSpace;
}



void SpecifyBoxMinWidth( INT32 hBoxHandle, INT32 iMinWidth )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	Box->uiBoxMinWidth = iMinWidth;

	// check if the box is currently too small
	if (Box->Dimensions.iRight < iMinWidth)
	{
		Box->Dimensions.iRight = iMinWidth;
	}
}


INT32 CreatePopUpBox(SGPRect Dimensions, SGPPoint Position, UINT32 uiFlags)
{
	INT32 iCounter=0;
	INT32 iCount=0;

	// find first free box
	for(iCounter=0; ( iCounter < MAX_POPUP_BOX_COUNT ) && ( PopUpBoxList[iCounter] != NULL ); iCounter++);

	if( iCounter >= MAX_POPUP_BOX_COUNT )
	{
		// ran out of available popup boxes - probably not freeing them up right!
		Assert(0);
		return NO_POPUP_BOX;
	}

	iCount=iCounter;

	PopUpBox* pBox = MemAlloc(sizeof(*pBox));
	if (pBox == NULL)
	{
		return NO_POPUP_BOX;
	}
	PopUpBoxList[iCount]=pBox;

	memset(pBox, 0, sizeof(*pBox));
	SetBoxPosition(iCount, Position);
	SetBoxSize(iCount, Dimensions);
	pBox->uiFlags = uiFlags;

	for(iCounter=0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++)
	{
		pBox->Text[iCounter] = NULL;
		pBox->pSecondColumnString[iCounter] = NULL;
	}

	SpecifyBoxMinWidth( iCount, 0 );
	SetBoxSecondColumnMinimumOffset( iCount, 0 );
	pBox->uiSecondColumnCurrentOffset = 0;

	pBox->fUpdated = FALSE;

	return iCount;
}


void SetMargins(INT32 hBoxHandle, UINT32 uiLeft, UINT32 uiTop, UINT32 uiBottom, UINT32 uiRight)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	Box->uiLeftMargin   = uiLeft;
	Box->uiRightMargin  = uiRight;
	Box->uiTopMargin    = uiTop;
	Box->uiBottomMargin = uiBottom;

	Box->fUpdated = FALSE;
}


UINT32 GetTopMarginSize( INT32 hBoxHandle )
{
	// return size of top margin, for mouse region offsets

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return( 0 );

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	return Box->uiTopMargin;
}


void ShadeStringInBox( INT32 hBoxHandle, INT32 iLineNumber )
{
	// shade iLineNumber Line in box indexed by hBoxHandle

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	if (Box->Text[iLineNumber] != NULL)
	{
		// shade line
		Box->Text[iLineNumber]->fShadeFlag = TRUE;
	}
}

void UnShadeStringInBox( INT32 hBoxHandle, INT32 iLineNumber )
{
	// unshade iLineNumber in box indexed by hBoxHandle

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	if (Box->Text[iLineNumber] != NULL)
	{
		// shade line
		Box->Text[iLineNumber]->fShadeFlag = FALSE;
	}
}


void SecondaryShadeStringInBox( INT32 hBoxHandle, INT32 iLineNumber )
{
	// shade iLineNumber Line in box indexed by hBoxHandle

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	if (Box->Text[iLineNumber] != NULL)
	{
		// shade line
		Box->Text[iLineNumber]->fSecondaryShadeFlag = TRUE;
	}
}

void UnSecondaryShadeStringInBox( INT32 hBoxHandle, INT32 iLineNumber )
{
	// unshade iLineNumber in box indexed by hBoxHandle

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	if (Box->Text[iLineNumber] != NULL)
	{
		// shade line
		Box->Text[iLineNumber]->fSecondaryShadeFlag = FALSE;
	}
}



void SetBoxBuffer(INT32 hBoxHandle, SGPVSurface* const buffer)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	Box->uiBuffer = buffer;
	Box->fUpdated = FALSE;
}


void SetBoxPosition( INT32 hBoxHandle,SGPPoint Position )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	Box->Position.iX = Position.iX;
	Box->Position.iY = Position.iY;
	Box->fUpdated    = FALSE;
}


void GetBoxPosition( INT32 hBoxHandle, SGPPoint *Position )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	Position->iX = Box->Position.iX;
	Position->iY = Box->Position.iY;
}

void SetBoxSize(INT32 hBoxHandle,SGPRect Dimensions)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	Box->Dimensions = Dimensions;
	Box->fUpdated   = FALSE;
}


void GetBoxSize( INT32 hBoxHandle, SGPRect *Dimensions )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);

	*Dimensions = Box->Dimensions;
}


void SetBorderType(const INT32 hBoxHandle, const SGPVObject* const border)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);
	Box->iBorderObjectIndex = border;
}

void SetBackGroundSurface(const INT32 hBoxHandle, SGPVSurface* const bg)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);
	Box->iBackGroundSurface = bg;
}


static void RemoveBoxPrimaryText(PopUpBox*, INT32 hStringHandle);


// adds a FIRST column string to the CURRENT popup box
void AddMonoString(const INT32 box_handle, const wchar_t* pString)
{
	STR16 pLocalString = NULL;
	INT32 iCounter = 0;

	if (box_handle < 0 || box_handle >= MAX_POPUP_BOX_COUNT) return;
	PopUpBox* const Box = PopUpBoxList[box_handle];
	Assert(Box != NULL);

	// find first free slot in list
	for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT && Box->Text[iCounter] != NULL; iCounter++);

	if ( iCounter >= MAX_POPUP_BOX_STRING_COUNT )
	{
		// using too many text lines, or not freeing them up properly
		Assert(0);
		return;
	}

	PopUpString* pStringSt = MemAlloc(sizeof(*pStringSt));
	if (pStringSt == NULL)
		return;

	pLocalString = MemAlloc(sizeof(*pLocalString) * (wcslen(pString) + 1));
	if (pLocalString == NULL)
		return;

	wcscpy(pLocalString, pString);

	RemoveBoxPrimaryText(Box, iCounter);

	Box->Text[iCounter]                      = pStringSt;
	Box->Text[iCounter]->pString             = pLocalString;
	Box->Text[iCounter]->fShadeFlag          = FALSE;
	Box->Text[iCounter]->fHighLightFlag      = FALSE;
	Box->Text[iCounter]->fSecondaryShadeFlag = FALSE;

	Box->fUpdated = FALSE;
}


static void RemoveBoxSecondaryText(PopUpBox*, INT32 hStringHandle);


void AddSecondColumnMonoString(const INT32 box_handle, const wchar_t* const pString)
{
	STR16 pLocalString=NULL;
	INT32 iCounter=0;

	if (box_handle < 0 || box_handle >= MAX_POPUP_BOX_COUNT) return;
	PopUpBox* const Box = PopUpBoxList[box_handle];
	Assert(Box != NULL );

	// find the LAST USED text string index
	for (iCounter = 0; iCounter + 1 < MAX_POPUP_BOX_STRING_COUNT && Box->Text[iCounter + 1] != NULL; iCounter++);

	if ( iCounter >= MAX_POPUP_BOX_STRING_COUNT )
	{
		// using too many text lines, or not freeing them up properly
		Assert(0);
		return;
	}

	PopUpString* pStringSt = MemAlloc(sizeof(*pStringSt));
	if (pStringSt == NULL)
		return;

	pLocalString = MemAlloc(sizeof(*pLocalString) * (wcslen(pString) + 1));
	if (pLocalString == NULL)
		return;

	wcscpy(pLocalString, pString);

	RemoveBoxSecondaryText(Box, iCounter);

	Box->pSecondColumnString[iCounter]                 = pStringSt;
	Box->pSecondColumnString[iCounter]->pString        = pLocalString;
	Box->pSecondColumnString[iCounter]->fShadeFlag     = FALSE;
	Box->pSecondColumnString[iCounter]->fHighLightFlag = FALSE;
}


static void ResizeBoxForSecondStrings(PopUpBox* const box)
{
	const UINT32 uiBaseWidth = box->uiLeftMargin + box->uiSecondColumnMinimunOffset;

	// check string sizes
	for (INT32 iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; ++iCounter)
	{
		const PopUpString* const line = box->Text[iCounter];
		if (line)
		{
			const UINT32 uiThisWidth = uiBaseWidth + StringPixLength(line->pString, line->uiFont);
			if (uiThisWidth > box->uiSecondColumnCurrentOffset)
			{
				box->uiSecondColumnCurrentOffset = uiThisWidth;
			}
		}
	}
}



UINT32 GetNumberOfLinesOfTextInBox( INT32 hBoxHandle )
{
	INT32 iCounter = 0;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return( 0 );

	// count number of lines
	// check string size
	for( iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++ )
	{
		if( PopUpBoxList[ hBoxHandle ]->Text[ iCounter ] == NULL )
		{
			break;
		}
	}

	return( iCounter );
}



void SetBoxFont(INT32 hBoxHandle, UINT32 uiFont)
{
	UINT32 uiCounter;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	for ( uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++ )
	{
		if (Box->Text[uiCounter] != NULL)
		{
			Box->Text[uiCounter]->uiFont = uiFont;
		}
	}

	// set up the 2nd column font
	SetBoxSecondColumnFont(hBoxHandle, uiFont);

	Box->fUpdated = FALSE;
}

void SetBoxSecondColumnMinimumOffset( INT32 hBoxHandle, UINT32 uiWidth )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBoxList[hBoxHandle]->uiSecondColumnMinimunOffset = uiWidth;
}


void SetBoxSecondColumnFont(INT32 hBoxHandle, UINT32 uiFont)
{
	UINT32 iCounter = 0;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++)
	{
		if (Box->pSecondColumnString[iCounter])
		{
			Box->pSecondColumnString[iCounter]->uiFont = uiFont;
		}
	}

	Box->fUpdated = FALSE;
}

UINT32 GetBoxFont( INT32 hBoxHandle )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return( 0 );

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box);
	Assert(Box->Text[0]);

	// return font id of first line of text of box
	return Box->Text[0]->uiFont;
}


// set the foreground color of this string in this pop up box
void SetBoxLineForeground( INT32 iBox, INT32 iStringValue, UINT8 ubColor )
{
	if ( ( iBox < 0 ) || ( iBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[iBox];

	Assert(Box);
	Assert(Box->Text[iStringValue]);

	Box->Text[iStringValue]->ubForegroundColor = ubColor;
}

void SetBoxSecondaryShade( INT32 iBox, UINT8 ubColor )
{
	UINT32 uiCounter;

	if ( ( iBox < 0 ) || ( iBox >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[iBox];

	Assert(Box);

	for ( uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++ )
	{
		if (Box->Text[uiCounter] != NULL)
		{
			Box->Text[uiCounter]->ubSecondaryShade = ubColor;
		}
	}
}


void SetBoxForeground(INT32 hBoxHandle, UINT8 ubColor)
{
	UINT32 uiCounter;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box != NULL);

	for ( uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++ )
	{
		if (Box->Text[uiCounter] != NULL)
		{
			Box->Text[uiCounter]->ubForegroundColor = ubColor;
		}
	}
}

void SetBoxBackground(INT32 hBoxHandle, UINT8 ubColor)
{
	UINT32 uiCounter;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box != NULL);

	for ( uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++ )
	{
		if (Box->Text[uiCounter] != NULL)
		{
			Box->Text[uiCounter]->ubBackgroundColor = ubColor;
		}
	}
}

void SetBoxHighLight(INT32 hBoxHandle, UINT8 ubColor)
{
	UINT32 uiCounter;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box != NULL);

	for ( uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++ )
	{
		if (Box->Text[uiCounter] != NULL)
		{
			Box->Text[uiCounter]->ubHighLight = ubColor;
		}
	}
}

void SetBoxShade(INT32 hBoxHandle, UINT8 ubColor)
{
	UINT32 uiCounter;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box != NULL);

	for ( uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++ )
	{
		if (Box->Text[uiCounter] != NULL)
		{
			Box->Text[uiCounter]->ubShade = ubColor;
		}
	}
}

void SetBoxSecondColumnForeground(INT32 hBoxHandle, UINT8 ubColor)
{
	UINT32 iCounter = 0;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box != NULL);

	for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++)
	{
		if (Box->pSecondColumnString[iCounter])
		{
			Box->pSecondColumnString[iCounter]->ubForegroundColor = ubColor;
		}
	}
}

void SetBoxSecondColumnBackground(INT32 hBoxHandle, UINT8 ubColor)
{
	UINT32 iCounter = 0;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box != NULL);

	for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++)
	{
		if (Box->pSecondColumnString[iCounter])
		{
			Box->pSecondColumnString[iCounter]->ubBackgroundColor = ubColor;
		}
	}
}

void SetBoxSecondColumnHighLight(INT32 hBoxHandle, UINT8 ubColor)
{
	UINT32 iCounter = 0;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box != NULL);

	for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++)
	{
		if (Box->pSecondColumnString[iCounter])
		{
			Box->pSecondColumnString[iCounter]->ubHighLight = ubColor;
		}
	}
}

void SetBoxSecondColumnShade(INT32 hBoxHandle, UINT8 ubColor)
{
	UINT32 iCounter = 0;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	Assert(Box != NULL);

	for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++)
	{
		if (Box->pSecondColumnString[iCounter])
		{
			Box->pSecondColumnString[iCounter]->ubShade = ubColor;
		}
	}
}


void HighLightBoxLine( INT32 hBoxHandle, INT32 iLineNumber )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	// highlight iLineNumber Line in box indexed by hBoxHandle

	PopUpString* const line = PopUpBoxList[hBoxHandle]->Text[iLineNumber];
	if (line != NULL)
	{
		line->fHighLightFlag = TRUE;
	}
}


BOOLEAN GetBoxShadeFlag( INT32 hBoxHandle, INT32 iLineNumber )
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return(FALSE);

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	if (Box->Text[iLineNumber] != NULL)
	{
		return Box->Text[iLineNumber]->fShadeFlag;
	}


	return( FALSE );
}


void UnHighLightBox(INT32 hBoxHandle)
{
	INT32 iCounter = 0;

	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[hBoxHandle];

	for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++)
	{
		if (Box->Text[iCounter])
			Box->Text[iCounter]->fHighLightFlag = FALSE;
	}
}


void RemoveAllBoxStrings(const INT32 box_handle)
{
	if (box_handle < 0 || box_handle >= MAX_POPUP_BOX_COUNT) return;
	PopUpBox* const Box = PopUpBoxList[box_handle];
	Assert(Box != NULL);

	for (UINT32 i = 0; i < MAX_POPUP_BOX_STRING_COUNT; ++i)
	{
		RemoveBoxPrimaryText(Box, i);
		RemoveBoxSecondaryText(Box, i);
	}
	Box->fUpdated = FALSE;
}


void RemoveBox(INT32 hBoxHandle)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	RemoveAllBoxStrings(hBoxHandle);
	MemFree(PopUpBoxList[hBoxHandle]);
	PopUpBoxList[hBoxHandle] = NULL;
}



void ShowBox(INT32 hBoxHandle)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	if (Box != NULL)
	{
		if (Box->fShowBox == FALSE)
		{
			Box->fShowBox = TRUE;
			Box->fUpdated = FALSE;
		}
	}
}

void HideBox(INT32 hBoxHandle)
{
	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	if (Box != NULL)
	{
		if (Box->fShowBox == TRUE)
		{
			Box->fShowBox = FALSE;
			Box->fUpdated = FALSE;
		}
	}
}


void DisplayBoxes(SGPVSurface* const uiBuffer)
{
	UINT32 uiCounter;

	for( uiCounter=0; uiCounter < MAX_POPUP_BOX_COUNT; uiCounter++ )
	{
		DisplayOnePopupBox( uiCounter, uiBuffer );
	}
}


static void DrawBox(UINT32 uiCounter);
static void DrawBoxText(const PopUpBox*);


void DisplayOnePopupBox(const UINT32 uiIndex, SGPVSurface* const uiBuffer)
{
	if ( ( uiIndex < 0 ) || ( uiIndex >= MAX_POPUP_BOX_COUNT ) )
		return;

	const PopUpBox* Box = PopUpBoxList[uiIndex];

	if (Box != NULL)
	{
		if (Box->uiBuffer == uiBuffer && Box->fShowBox)
		{
			DrawBox(uiIndex);
			DrawBoxText(Box);
		}
	}
}



// force an update of this box
void ForceUpDateOfBox( UINT32 uiIndex )
{
	if ( ( uiIndex < 0 ) || ( uiIndex >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[uiIndex];

	if (Box != NULL)
	{
		Box->fUpdated = FALSE;
	}
}


static void DrawBox(UINT32 uiCounter)
{
	PopUpBox* const box = PopUpBoxList[uiCounter];

	// only update if we need to
	if (box->fUpdated) return;
	box->fUpdated = TRUE;

	if (box->uiFlags & POPUP_BOX_FLAG_RESIZE) ResizeBoxToText(uiCounter);

	const UINT16 x = box->Position.iX;
	const UINT16 y = box->Position.iY;
	UINT16       w = box->Dimensions.iRight  - box->Dimensions.iLeft;
	const UINT16 h = box->Dimensions.iBottom - box->Dimensions.iTop;

	// check if we have a min width, if so then update box for such
	if (w < box->uiBoxMinWidth) w = box->uiBoxMinWidth;

	// make sure it will fit on screen!
	Assert(x + w  < SCREEN_WIDTH);
	Assert(y + h < SCREEN_HEIGHT);

	// subtract 4 because the 2 2-pixel corners are handled separately
	const UINT32 uiNumTilesWide = (w - 4) / BORDER_WIDTH;
	const UINT32 uiNumTilesHigh = (h - 4) / BORDER_HEIGHT;

	SGPVSurface* const dst = box->uiBuffer;

	// blit in texture first, then borders
	const SGPRect clip = { 0, 0, w, h };
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
	const INT32 tlx = box->Position.iX + box->uiLeftMargin;
	const INT32 tly = box->Position.iY + box->uiTopMargin;
	const INT32 brx = box->Position.iX + box->Dimensions.iRight  - box->uiRightMargin;
	const INT32 bry = box->Position.iY + box->Dimensions.iBottom - box->uiBottomMargin;
	const INT32 w   = box->Dimensions.iRight - (box->uiRightMargin + box->uiLeftMargin + 2);

	SetFontDestBuffer(box->uiBuffer, tlx - 1, tly, brx, bry);

	for (UINT32 i = 0; i < MAX_POPUP_BOX_STRING_COUNT; ++i)
	{
		// there is text in this line?
		const PopUpString* const text = box->Text[i];
		if (text)
		{
			SetFont(text->uiFont);

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

			const INT32 h = GetFontHeight(text->uiFont);
			const INT32 y = tly + i * (h + box->uiLineSpace);
			INT16 uX;
			INT16 uY;
			if (box->uiFlags & POPUP_BOX_FLAG_CENTER_TEXT)
			{
				FindFontCenterCoordinates(tlx, y, w, h, text->pString, text->uiFont, &uX, &uY);
			}
			else
			{
				uX = tlx;
				uY = y;
			}
			mprintf(uX, uY, L"%ls", text->pString);
		}

		// there is secondary text in this line?
		const PopUpString* const second = box->pSecondColumnString[i];
		if (second)
		{
			SetFont(second->uiFont);

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

			const INT32 h = GetFontHeight(second->uiFont);
			const INT32 y = tly + i * (h + box->uiLineSpace);
			INT16 uX;
			INT16 uY;
			if (box->uiFlags & POPUP_BOX_FLAG_CENTER_TEXT)
			{
				FindFontCenterCoordinates(tlx, y, w, h, second->pString, second->uiFont, &uX, &uY);
			}
			else
			{
				uX = tlx + box->uiSecondColumnCurrentOffset;
				uY = y;
			}
			mprintf(uX, uY, L"%ls", second->pString);
		}
	}

	if (box->uiBuffer != guiSAVEBUFFER)
	{
		InvalidateRegion(tlx - 1, tly, brx, bry);
	}

	SetFontDestBuffer(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}


void ResizeBoxToText(INT32 hBoxHandle)
{
	// run through lines of text in box and size box width to longest line plus margins
	// height is sum of getfontheight of each line+ spacing
	INT32 iWidth=0;
	INT32 iHeight=0;
	INT32 iCurrString=0;
	INT32 iSecondColumnLength = 0;


	if ( ( hBoxHandle < 0 ) || ( hBoxHandle >= MAX_POPUP_BOX_COUNT ) )
		return;

	PopUpBox* Box = PopUpBoxList[hBoxHandle];

	if (!Box)
		return;

	ResizeBoxForSecondStrings(Box);

	iHeight = Box->uiTopMargin + Box->uiBottomMargin;

	for ( iCurrString = 0; iCurrString < MAX_POPUP_BOX_STRING_COUNT; iCurrString++ )
	{
		if (Box->Text[iCurrString] != NULL)
		{
			if (Box->pSecondColumnString[iCurrString] != NULL)
			{
				iSecondColumnLength = StringPixLength(Box->pSecondColumnString[iCurrString]->pString, Box->pSecondColumnString[iCurrString]->uiFont);
				if (Box->uiSecondColumnCurrentOffset + iSecondColumnLength + Box->uiLeftMargin + Box->uiRightMargin > (UINT32)iWidth)
				{
					iWidth = Box->uiSecondColumnCurrentOffset + iSecondColumnLength + Box->uiLeftMargin + Box->uiRightMargin;
				}
			}

			if (StringPixLength(Box->Text[iCurrString]->pString, Box->Text[iCurrString]->uiFont) + Box->uiLeftMargin + Box->uiRightMargin > (UINT32)iWidth)
				iWidth = StringPixLength(Box->Text[iCurrString]->pString, Box->Text[iCurrString]->uiFont) + Box->uiLeftMargin + Box->uiRightMargin;

			//vertical
			iHeight += GetFontHeight(Box->Text[iCurrString]->uiFont) + Box->uiLineSpace;
		}
		else
		{
			// doesn't support gaps in text array...
			break;
		}
	}
	Box->Dimensions.iBottom = iHeight;
	Box->Dimensions.iRight  = iWidth;
}


BOOLEAN IsBoxShown( UINT32 uiHandle )
{
	if ( ( uiHandle < 0 ) || ( uiHandle >= MAX_POPUP_BOX_COUNT ) )
		return(FALSE);

	const PopUpBox* Box = PopUpBoxList[uiHandle];

	if (Box == NULL)
	{
		return ( FALSE );
	}

	return Box->fShowBox;
}


void MarkAllBoxesAsAltered( void )
{
	INT32 iCounter = 0;

	// mark all boxes as altered
	for( iCounter = 0; iCounter < MAX_POPUP_BOX_COUNT; iCounter++ )
	{
		ForceUpDateOfBox( iCounter );
	}
}


void HideAllBoxes( void )
{
	INT32 iCounter = 0;

	// hide all the boxes that are shown
	for(iCounter=0; iCounter < MAX_POPUP_BOX_COUNT; iCounter++)
	{
		HideBox( iCounter );
	}
}


static void RemoveBoxPrimaryText(PopUpBox* const Box, const INT32 hStringHandle)
{
	Assert(Box != NULL);
	Assert( hStringHandle < MAX_POPUP_BOX_STRING_COUNT );

	// remove & release primary text
	if (Box->Text[hStringHandle] != NULL)
	{
		if (Box->Text[hStringHandle]->pString)
		{
			MemFree(Box->Text[hStringHandle]->pString);
		}

		MemFree(Box->Text[hStringHandle]);
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
		if (Box->pSecondColumnString[hStringHandle]->pString)
		{
			MemFree(Box->pSecondColumnString[hStringHandle]->pString);
		}

		MemFree(Box->pSecondColumnString[hStringHandle]);
		Box->pSecondColumnString[hStringHandle] = NULL;
	}
}
