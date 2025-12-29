#ifndef __FLORIST_GALLERY_H
#define __FLORIST_GALLERY_H

#include "Types.h"

BOOLEAN EnterFloristGallery(void);
void ExitFloristGallery(void);
void HandleFloristGallery(void);
void RenderFloristGallery(void);
void EnterInitFloristGallery(void);

extern	UINT32	guiCurrentlySelectedFlower;
extern	UINT8		gubCurFlowerIndex;


#endif
