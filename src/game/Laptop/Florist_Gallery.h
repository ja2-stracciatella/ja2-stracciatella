#ifndef __FLORIST_GALLERY_H
#define __FLORIST_GALLERY_H

#include "Types.h"

#define FLOR_GALLERY_TEXT_FILE       BINARYDATADIR "/flowerdesc.edt"
#define FLOR_GALLERY_TEXT_TITLE_SIZE 80
#define FLOR_GALLERY_TEXT_PRICE_SIZE 80
#define FLOR_GALLERY_TEXT_DESC_SIZE  80 * 4
#define FLOR_GALLERY_TEXT_TOTAL_SIZE 80 * 6


BOOLEAN EnterFloristGallery(void);
void ExitFloristGallery(void);
void HandleFloristGallery(void);
void RenderFloristGallery(void);
void EnterInitFloristGallery(void);

extern	UINT32	guiCurrentlySelectedFlower;
extern	UINT8		gubCurFlowerIndex;


#endif
