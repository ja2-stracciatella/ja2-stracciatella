#ifndef __MERCTEXTBOX_H_
#define __MERCTEXTBOX_H_

#include "Types.h"


#define MERC_POPUP_PREPARE_FLAGS_TRANS_BACK			0x00000001
#define MERC_POPUP_PREPARE_FLAGS_MARGINS				0x00000002
#define MERC_POPUP_PREPARE_FLAGS_STOPICON				0x00000004
#define MERC_POPUP_PREPARE_FLAGS_SKULLICON			0x00000008

BOOLEAN InitMercPopupBox( );


// create a pop up box if needed, return id of box..a -1 means couldn't be added
INT32 PrepareMercPopupBox(  INT32 iBoxId, UINT8 ubBackgroundIndex, UINT8 ubBorderIndex, const wchar_t *pString, UINT16 usWidth, UINT16 usMarginX, UINT16 usMarginTopY, UINT16 usMarginBottomY, UINT16 *pActualWidth, UINT16 *pActualHeight);


// remove this box from the index
BOOLEAN RemoveMercPopupBoxFromIndex( UINT32 uiId );

// render the current pop up box
BOOLEAN RenderMercPopupBox(INT16 sDestX, INT16 sDestY, UINT32 uiBuffer );

// render pop up box with this index value
BOOLEAN RenderMercPopUpBoxFromIndex( INT32 iBoxId, INT16 sDestX, INT16 sDestY, UINT32 uiBuffer );


void RemoveTextMercPopupImages( );


typedef struct {
 UINT32 uiSourceBufferIndex;
 UINT16 sWidth;
 UINT16 sHeight;
 UINT8	ubBackgroundIndex;
 UINT8	ubBorderIndex;
 UINT32	uiMercTextPopUpBackground;
 UINT32 uiMercTextPopUpBorder;
 BOOLEAN	fMercTextPopupInitialized;
 BOOLEAN	fMercTextPopupSurfaceInitialized;
 UINT32		uiFlags;

} MercPopUpBox;


BOOLEAN OverrideMercPopupBox( MercPopUpBox *pMercBox );
BOOLEAN ResetOverrideMercPopupBox( );
BOOLEAN	SetPrepareMercPopupFlags( UINT32 uiFlags );


// background enumeration
enum{
	BASIC_MERC_POPUP_BACKGROUND = 0,
	WHITE_MERC_POPUP_BACKGROUND,
	GREY_MERC_POPUP_BACKGROUND,
	DIALOG_MERC_POPUP_BACKGROUND,
	LAPTOP_POPUP_BACKGROUND,
	IMP_POPUP_BACKGROUND,
};

// border enumeration
enum{
  BASIC_MERC_POPUP_BORDER =0,
  RED_MERC_POPUP_BORDER,
  BLUE_MERC_POPUP_BORDER,
  DIALOG_MERC_POPUP_BORDER,
	LAPTOP_POP_BORDER
};


#endif
