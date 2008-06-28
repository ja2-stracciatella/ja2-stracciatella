#include <stdexcept>

#include "Font.h"
#include "HImage.h"
#include "Local.h"
#include "MercTextBox.h"
#include "PODObj.h"
#include "VObject.h"
#include "VSurface.h"
#include "Font_Control.h"
#include "WordWrap.h"
#include "Render_Dirty.h"
#include "Debug.h"
#include "Video.h"
#include "MemMan.h"


#define		TEXT_POPUP_WINDOW_TEXT_OFFSET_X		8
#define		TEXT_POPUP_WINDOW_TEXT_OFFSET_Y		8
#define		TEXT_POPUP_STRING_WIDTH						296
#define		TEXT_POPUP_GAP_BN_LINES						10
#define		TEXT_POPUP_FONT										FONT12ARIAL
#define		TEXT_POPUP_COLOR									FONT_MCOLOR_WHITE

#define		MERC_TEXT_FONT										FONT12ARIAL
#define		MERC_TEXT_COLOR										FONT_MCOLOR_WHITE

#define		MERC_TEXT_MIN_WIDTH								10
#define		MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X		10
#define		MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_Y		10

#define		MERC_BACKGROUND_WIDTH										350
#define		MERC_BACKGROUND_HEIGHT									200

// the max number of pop up boxes availiable to user
#define MAX_NUMBER_OF_POPUP_BOXES 10


// both of the below are index by the enum for thier types - background and border in
// MercTextBox.h

// filenames for border popup .sti's
static char const* const zMercBorderPopupFilenames[] =
{
 "INTERFACE/TactPopUp.sti",
 "INTERFACE/TactRedPopUp.sti",
 "INTERFACE/TactBluePopUp.sti",
 "INTERFACE/TactPopUpMain.sti",
 "INTERFACE/LaptopPopup.sti"
};

// filenames for background popup .pcx's
static char const* const zMercBackgroundPopupFilenames[] =
{
  "INTERFACE/TactPopupBackground.pcx",
  "INTERFACE/TactPopupWhiteBackground.pcx",
  "INTERFACE/TactPopupGreyBackground.pcx",
  "INTERFACE/TactPopupBackgroundMain.pcx",
  "INTERFACE/LaptopPopupBackground.pcx",
	"INTERFACE/imp_popup_background.pcx"
};


struct MercPopUpBox
{
	SGPVSurface*      uiSourceBufferIndex;
	UINT8             ubBackgroundIndex;
	UINT8             ubBorderIndex;
	SGPVSurface*      uiMercTextPopUpBackground;
	SGPVObject*       uiMercTextPopUpBorder;
	BOOLEAN           fMercTextPopupInitialized;
	BOOLEAN           fMercTextPopupSurfaceInitialized;
};


// the list of boxes
static MercPopUpBox* gpPopUpBoxList[MAX_NUMBER_OF_POPUP_BOXES];

// the flags
static SGPVObject* guiBoxIcons;
static SGPVObject* guiSkullIcons;


static MercPopUpBox* SetCurrentPopUpBox(UINT32 const uiId)
{
	// given id of the box, find it in the list and set to current

	//make sure the box id is valid
	if( uiId == (UINT32) -1 )
	{
		//ScreenMsg( FONT_MCOLOR_WHITE, MSG_BETAVERSION, L"Error: Trying to set Current Popup Box using -1 as an ID" );
		return 0;
	}

	// see if box inited
	return gpPopUpBoxList[uiId];
}


void InitMercPopupBox()
{
	INT32 iCounter = 0;

	// init the pop up box list
	for( iCounter = 0; iCounter < MAX_NUMBER_OF_POPUP_BOXES; iCounter++ )
	{
		// set ptr to null
		gpPopUpBoxList[ iCounter ] = NULL;
	}

	guiBoxIcons   = AddVideoObjectFromFile("INTERFACE/msgboxicons.sti");     // stop icon
	guiSkullIcons = AddVideoObjectFromFile("INTERFACE/msgboxiconskull.sti"); // skull icon
}


// Tactical Popup
static void LoadTextMercPopupImages(MercPopUpBox* const box, MercPopUpBackground const ubBackgroundIndex, MercPopUpBorder const ubBorderIndex)
{
  // this function will load the graphics associated with the background and border index values
	box->uiMercTextPopUpBackground = AddVideoSurfaceFromFile(zMercBackgroundPopupFilenames[ubBackgroundIndex]);
	box->uiMercTextPopUpBorder     = AddVideoObjectFromFile(zMercBorderPopupFilenames[ubBorderIndex]);
	box->fMercTextPopupInitialized = TRUE;

	// so far so good, return successful
	box->ubBackgroundIndex = ubBackgroundIndex;
	box->ubBorderIndex     = ubBorderIndex;
}


static void RemoveTextMercPopupImages(MercPopUpBox* const box)
{
	//this procedure will remove the background and border video surface/object from the indecies
	if (!box->fMercTextPopupInitialized) return;

	DeleteVideoSurface(box->uiMercTextPopUpBackground);
	DeleteVideoObject(box->uiMercTextPopUpBorder);
	box->fMercTextPopupInitialized = FALSE;
}


BOOLEAN RenderMercPopUpBoxFromIndex(const INT32 iBoxId, const INT16 sDestX, const INT16 sDestY, SGPVSurface* const buffer)
{
	MercPopUpBox const* const box = SetCurrentPopUpBox(iBoxId);
	if (!box) return FALSE;

	// will render/transfer the image from the buffer in the data structure to the buffer specified by user
	SGPVSurface* const vs = box->uiSourceBufferIndex;
	BltVideoSurface(buffer, vs, sDestX, sDestY, NULL);

	//Invalidate!
	if (buffer == FRAME_BUFFER)
	{
		InvalidateRegion(sDestX, sDestY, sDestX + vs->Width(), sDestY + vs->Height());
	}

	return TRUE;
}


static INT32 AddPopUpBoxToList(MercPopUpBox* pPopUpTextBox)
{
	INT32 iCounter = 0;

	// attempt to add box to list
	for( iCounter = 0; iCounter < MAX_NUMBER_OF_POPUP_BOXES; iCounter++ )
	{
		if( gpPopUpBoxList[ iCounter ] == NULL )
		{
			// found a spot, inset
			gpPopUpBoxList[ iCounter ] = pPopUpTextBox;

			// set as current
			SetCurrentPopUpBox( iCounter );

			// return index value
			return( iCounter );
		}
	}

	throw std::runtime_error("Out of merc popup box slots");
}


static void GetMercPopupBoxFontColor(UINT8 ubBackgroundIndex, UINT8* pubFontColor, UINT8* pubFontShadowColor);


INT32 PrepareMercPopupBox(INT32 const iBoxId, MercPopUpBackground const ubBackgroundIndex, MercPopUpBorder const ubBorderIndex, wchar_t const* const pString, UINT16 usWidth, UINT16 const usMarginX, UINT16 const usMarginTopY, UINT16 const usMarginBottomY, UINT16* const pActualWidth, UINT16* const pActualHeight, MercPopupBoxFlags const flags)
try
{
	if (usWidth >= SCREEN_WIDTH) return -1;

	if (usWidth <= MERC_TEXT_MIN_WIDTH) usWidth = MERC_TEXT_MIN_WIDTH;

	MercPopUpBox*             box;
	SGP::PODObj<MercPopUpBox> new_box(0);
	// check id value, if -1, box has not been inited yet
	if (iBoxId == -1)
	{
		// no box yet
		box = new_box.Allocate();
		LoadTextMercPopupImages(box, ubBackgroundIndex, ubBorderIndex);
	}
	else
	{
		// has been created already,
		// Check if these images are different
		box = gpPopUpBoxList[iBoxId];
		Assert(box);

		if (ubBackgroundIndex != box->ubBackgroundIndex ||
				ubBorderIndex     != box->ubBorderIndex     ||
				!box->fMercTextPopupInitialized)
		{
			//Remove old, set new
			RemoveTextMercPopupImages(box);
			LoadTextMercPopupImages(box, ubBackgroundIndex, ubBorderIndex);
		}
	}

	UINT16 const usStringPixLength = StringPixLength(pString, TEXT_POPUP_FONT);
	UINT16       usTextWidth;
	if (usStringPixLength < usWidth - MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X * 2)
	{
		usWidth     = usStringPixLength + MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X * 2;
		usTextWidth = usWidth           - MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X * 2 + 1;
	}
	else
	{
		usTextWidth = usWidth - MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X * 2 + 1 - usMarginX;
	}

	UINT16 const usNumberVerticalPixels = IanWrappedStringHeight(usTextWidth, 2, TEXT_POPUP_FONT, pString);
	UINT16       usHeight               = usNumberVerticalPixels + MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_Y * 2;

	// Add height for margins
	usHeight += usMarginTopY + usMarginBottomY;

	// Add width for margins
	usWidth += usMarginX * 2;

	// Add width for iconic...
	if (flags & (MERC_POPUP_PREPARE_FLAGS_STOPICON | MERC_POPUP_PREPARE_FLAGS_SKULLICON))
	{
		// Make minimun height for box...
		if (usHeight < 45) usHeight = 45;
		usWidth += 35;
	}

	if (usWidth >= MERC_BACKGROUND_WIDTH) usWidth = MERC_BACKGROUND_WIDTH - 1;

	// make sure the area isnt bigger than the background texture
	if (usHeight >= MERC_BACKGROUND_HEIGHT) return -1;

	// Create a background video surface to blt the face onto
	SGPVSurface* const vs = AddVideoSurface(usWidth, usHeight, PIXEL_DEPTH);
	box->uiSourceBufferIndex              = vs;
	box->fMercTextPopupSurfaceInitialized = TRUE;

	*pActualWidth  = usWidth;
	*pActualHeight = usHeight;

	if (flags & MERC_POPUP_PREPARE_FLAGS_TRANS_BACK)
	{
		// Zero with yellow,
		// Set source transparcenty
		vs->SetTransparency(FROMRGB(255, 255, 0));
		vs->Fill(Get16BPPColor(FROMRGB(255, 255, 0)));
	}
	else
	{
		SGPRect DestRect;
		DestRect.iLeft   = 0;
		DestRect.iTop    = 0;
		DestRect.iRight  = usWidth;
		DestRect.iBottom = usHeight;
		BltVideoSurface(vs, box->uiMercTextPopUpBackground, 0, 0, &DestRect);
	}

	const SGPVObject* const hImageHandle = box->uiMercTextPopUpBorder;

	UINT16 usPosY = 0;
	//blit top row of images
	for (UINT16 i = TEXT_POPUP_GAP_BN_LINES; i < usWidth - TEXT_POPUP_GAP_BN_LINES; i += TEXT_POPUP_GAP_BN_LINES)
	{
		//TOP ROW
	  BltVideoObject(vs, hImageHandle, 1, i, usPosY);
		//BOTTOM ROW
	  BltVideoObject(vs, hImageHandle, 6, i, usHeight - TEXT_POPUP_GAP_BN_LINES + 6);
	}

	//blit the left and right row of images
	UINT16 usPosX = 0;
	for (UINT16 i= TEXT_POPUP_GAP_BN_LINES; i < usHeight - TEXT_POPUP_GAP_BN_LINES; i += TEXT_POPUP_GAP_BN_LINES)
	{
	  BltVideoObject(vs, hImageHandle, 3, usPosX,               i);
	  BltVideoObject(vs, hImageHandle, 4, usPosX + usWidth - 4, i);
	}

	//blt the corner images for the row
	//top left
	BltVideoObject(vs, hImageHandle, 0, 0,                                 usPosY);
	//top right
	BltVideoObject(vs, hImageHandle, 2, usWidth - TEXT_POPUP_GAP_BN_LINES, usPosY);
	//bottom left
	BltVideoObject(vs, hImageHandle, 5, 0,                                 usHeight - TEXT_POPUP_GAP_BN_LINES);
	//bottom right
	BltVideoObject(vs, hImageHandle, 7, usWidth - TEXT_POPUP_GAP_BN_LINES, usHeight - TEXT_POPUP_GAP_BN_LINES);

	// Icon if ness....
	if (flags & MERC_POPUP_PREPARE_FLAGS_STOPICON)
	{
		BltVideoObject(vs, guiBoxIcons, 0, 5, 4);
	}
	if (flags & MERC_POPUP_PREPARE_FLAGS_SKULLICON)
	{
		BltVideoObject(vs, guiSkullIcons, 0, 9, 4);
	}

	//Get the font and shadow colors
	UINT8 ubFontColor;
	UINT8 ubFontShadowColor;
	GetMercPopupBoxFontColor(ubBackgroundIndex, &ubFontColor, &ubFontShadowColor);

	SetFontShadow(ubFontShadowColor);
	SetFontDestBuffer(vs, 0, 0, usWidth, usHeight);

	//Display the text
	INT16 sDispTextXPos = MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X + usMarginX;

	if (flags & (MERC_POPUP_PREPARE_FLAGS_STOPICON | MERC_POPUP_PREPARE_FLAGS_SKULLICON))
	{
		sDispTextXPos += 30;
	}

	DisplayWrappedString(sDispTextXPos, MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_Y + usMarginTopY, usTextWidth, 2, MERC_TEXT_FONT, ubFontColor, pString, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	SetFontDestBuffer(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	SetFontShadow(DEFAULT_SHADOW);

	if (iBoxId == -1)
	{
		// now return attemp to add to pop up box list, if successful will return index
		INT32 const new_id = AddPopUpBoxToList(box);
		new_box.Release();
		return new_id;
	}
	else
	{
		// set as current box
		SetCurrentPopUpBox(iBoxId);
		return iBoxId;
	}
}
catch (...) { return -1; }


//Deletes the surface thats contains the border, background and the text.
BOOLEAN RemoveMercPopupBoxFromIndex(UINT32 const uiId)
{
	MercPopUpBox* const box = SetCurrentPopUpBox(uiId);
	if (!box) return FALSE;

	// now check to see if inited...
	if (!box->fMercTextPopupSurfaceInitialized) return TRUE; // XXX TRUE?

	// now find this box in the list
	for (INT32 i = 0; i != MAX_NUMBER_OF_POPUP_BOXES; ++i)
	{
		if (gpPopUpBoxList[i] != box) continue;
		gpPopUpBoxList[i] = 0;
		break;
	}
	DeleteVideoSurface(box->uiSourceBufferIndex);

	//DEF Added 5/26
	//Delete the background and the border
	RemoveTextMercPopupImages(box);

	MemFree(box);
	return TRUE;
}


//Pass in the background index, and pointers to the font and shadow color
static void GetMercPopupBoxFontColor(UINT8 ubBackgroundIndex, UINT8* pubFontColor, UINT8* pubFontShadowColor)
{
	switch( ubBackgroundIndex )
	{
		case BASIC_MERC_POPUP_BACKGROUND:
			*pubFontColor = TEXT_POPUP_COLOR;
			*pubFontShadowColor = DEFAULT_SHADOW;
			break;

		case WHITE_MERC_POPUP_BACKGROUND:
			*pubFontColor = 2;
			*pubFontShadowColor = FONT_MCOLOR_WHITE;
			break;

		case GREY_MERC_POPUP_BACKGROUND:
			*pubFontColor = 2;
			*pubFontShadowColor = NO_SHADOW;
			break;

		case LAPTOP_POPUP_BACKGROUND:
			*pubFontColor = TEXT_POPUP_COLOR;
			*pubFontShadowColor = DEFAULT_SHADOW;
			break;

		default:
			*pubFontColor = TEXT_POPUP_COLOR;
			*pubFontShadowColor = DEFAULT_SHADOW;
			break;
	}
}
