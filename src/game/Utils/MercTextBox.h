#ifndef MERCTEXTBOX_H
#define MERCTEXTBOX_H

#include "AutoObj.h"
#include "JA2Types.h"

#include <string_theory/string>


enum MercPopupBoxFlags
{
	MERC_POPUP_PREPARE_FLAGS_NONE       = 0,
	MERC_POPUP_PREPARE_FLAGS_TRANS_BACK = 1U << 0,
	MERC_POPUP_PREPARE_FLAGS_STOPICON   = 1U << 1,
	MERC_POPUP_PREPARE_FLAGS_SKULLICON  = 1U << 2
};

void InitMercPopupBox(void);


enum MercPopUpBackground
{
	BASIC_MERC_POPUP_BACKGROUND = 0,
	WHITE_MERC_POPUP_BACKGROUND,
	GREY_MERC_POPUP_BACKGROUND,
	DIALOG_MERC_POPUP_BACKGROUND,
	LAPTOP_POPUP_BACKGROUND,
	IMP_POPUP_BACKGROUND
};

enum MercPopUpBorder
{
	BASIC_MERC_POPUP_BORDER = 0,
	RED_MERC_POPUP_BORDER,
	BLUE_MERC_POPUP_BORDER,
	DIALOG_MERC_POPUP_BORDER,
	LAPTOP_POP_BORDER
};


// create a pop up box if needed, return null pointer on failure
MercPopUpBox* PrepareMercPopupBox(MercPopUpBox* box, MercPopUpBackground ubBackgroundIndex, MercPopUpBorder ubBorderIndex, const ST::utf32_buffer& codepoints, UINT16 usWidth, UINT16 usMarginX, UINT16 usMarginTopY, UINT16 usMarginBottomY, UINT16* pActualWidth, UINT16* pActualHeight, MercPopupBoxFlags flags = MERC_POPUP_PREPARE_FLAGS_NONE);
inline MercPopUpBox* PrepareMercPopupBox(MercPopUpBox* box, MercPopUpBackground ubBackgroundIndex, MercPopUpBorder ubBorderIndex, const ST::string& str, UINT16 usWidth, UINT16 usMarginX, UINT16 usMarginTopY, UINT16 usMarginBottomY, UINT16* pActualWidth, UINT16* pActualHeight, MercPopupBoxFlags flags = MERC_POPUP_PREPARE_FLAGS_NONE)
{
	return PrepareMercPopupBox(box, ubBackgroundIndex, ubBorderIndex, str.to_utf32(), usWidth, usMarginX, usMarginTopY, usMarginBottomY, pActualWidth, pActualHeight, flags);
}

// Now just another way to say 'delete box', so it is ok to call this with a nullptr
void RemoveMercPopupBox(MercPopUpBox*);

void RenderMercPopUpBox(MercPopUpBox const*, INT16 sDestX, INT16 sDestY, SGPVSurface* buffer);

typedef SGP::AutoObj<MercPopUpBox, RemoveMercPopupBox> AutoMercPopUpBox;

#endif
