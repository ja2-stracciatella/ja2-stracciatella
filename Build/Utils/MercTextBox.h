#ifndef __MERCTEXTBOX_H_
#define __MERCTEXTBOX_H_

#include "Types.h"


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


// create a pop up box if needed, return id of box..a -1 means couldn't be added
INT32 PrepareMercPopupBox(INT32 iBoxId, MercPopUpBackground, MercPopUpBorder, wchar_t const* pString, UINT16 usWidth, UINT16 usMarginX, UINT16 usMarginTopY, UINT16 usMarginBottomY, UINT16* pActualWidth, UINT16* pActualHeight);


// remove this box from the index
BOOLEAN RemoveMercPopupBoxFromIndex( UINT32 uiId );

// render pop up box with this index value
BOOLEAN RenderMercPopUpBoxFromIndex(INT32 iBoxId, INT16 sDestX, INT16 sDestY, SGPVSurface* buffer);


void RemoveTextMercPopupImages(void);


typedef struct {
 SGPVSurface* uiSourceBufferIndex;
 UINT16 sWidth;
 UINT16 sHeight;
 UINT8	ubBackgroundIndex;
 UINT8	ubBorderIndex;
 SGPVSurface* uiMercTextPopUpBackground;
 SGPVObject* uiMercTextPopUpBorder;
 BOOLEAN	fMercTextPopupInitialized;
 BOOLEAN	fMercTextPopupSurfaceInitialized;
 MercPopupBoxFlags uiFlags;
} MercPopUpBox;


void OverrideMercPopupBox(MercPopUpBox* pMercBox);
void ResetOverrideMercPopupBox(void);
void SetPrepareMercPopupFlags(MercPopupBoxFlags);

#endif
