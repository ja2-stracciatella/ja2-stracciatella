#ifndef MERCTEXTBOX_H
#define MERCTEXTBOX_H

#include "AutoObj.h"
#include "JA2Types.h"


enum MercPopupBoxFlags
{
	MERC_POPUP_PREPARE_FLAGS_NONE       = 0,
	MERC_POPUP_PREPARE_FLAGS_TRANS_BACK = 1U << 0,
	MERC_POPUP_PREPARE_FLAGS_STOPICON   = 1U << 1,
	MERC_POPUP_PREPARE_FLAGS_SKULLICON  = 1U << 2
};
ENUM_BITSET(MercPopupBoxFlags)

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
MercPopUpBox* PrepareMercPopupBox(MercPopUpBox*, MercPopUpBackground, MercPopUpBorder, wchar_t const* pString, UINT16 usWidth, UINT16 usMarginX, UINT16 usMarginTopY, UINT16 usMarginBottomY, UINT16* pActualWidth, UINT16* pActualHeight, MercPopupBoxFlags flags = MERC_POPUP_PREPARE_FLAGS_NONE);

void RemoveMercPopupBox(MercPopUpBox*);

void RenderMercPopUpBox(MercPopUpBox const*, INT16 sDestX, INT16 sDestY, SGPVSurface* buffer);

typedef SGP::AutoObj<MercPopUpBox, RemoveMercPopupBox> AutoMercPopUpBox;

#endif
