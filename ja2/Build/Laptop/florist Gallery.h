#ifndef __FLORIST_GALLERY_H
#define __FLORIST_GALLERY_H


#define	FLOR_GALLERY_TEXT_FILE								"BINARYDATA\\FlowerDesc.edt"
#define	FLOR_GALLERY_TEXT_TITLE_SIZE					80 * 2
#define	FLOR_GALLERY_TEXT_PRICE_SIZE					80 * 2
#define	FLOR_GALLERY_TEXT_DESC_SIZE						4 * 80 * 2
#define	FLOR_GALLERY_TEXT_TOTAL_SIZE					6 * 80 * 2



void GameInitFloristGallery();
BOOLEAN EnterFloristGallery();
void ExitFloristGallery();
void HandleFloristGallery();
void RenderFloristGallery();
void EnterInitFloristGallery();

extern	UINT32	guiCurrentlySelectedFlower;
extern	UINT8		gubCurFlowerIndex;


#endif
