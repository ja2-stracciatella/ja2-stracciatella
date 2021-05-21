#ifndef CURSOR_CONTROL_H
#define CURSOR_CONTROL_H

#include "Types.h"


void    CursorDatabaseClear(void);
BOOLEAN SetCurrentCursorFromDatabase(UINT32 uiCursorIndex);

#define USE_OUTLINE_BLITTER		0x08

#define EXTERN_CURSOR			0xFFF0
#define MAX_COMPOSITES			5
#define CENTER_SUBCURSOR		31000
#define HIDE_SUBCURSOR			32000

#define CENTER_CURSOR			32000
#define RIGHT_CURSOR			32001
#define LEFT_CURSOR			32002
#define TOP_CURSOR			32003
#define BOTTOM_CURSOR			32004

#define CURSOR_TO_FLASH		0x01
#define CURSOR_TO_FLASH2		0x02
#define CURSOR_TO_SUB_CONDITIONALLY	0x04
#define DELAY_START_CURSOR		0x08
#define CURSOR_TO_PLAY_SOUND		0x10


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cursor Database
//
///////////////////////////////////////////////////////////////////////////////////////////////////

struct CursorFileData
{
	const char* Filename;         // If NULL then an extern video object is used
	HVOBJECT    hVObject;
	UINT8       ubFlags;
	UINT8       ubNumberOfFrames; // If != 0 then the cursor is animated
};

struct CursorImage
{
	UINT32 uiFileIndex;
	UINT16 uiSubIndex;
	UINT32 uiCurrentFrame;
	INT16  usPosX;
	INT16  usPosY;
};

struct CursorData
{
	CursorImage	Composites[MAX_COMPOSITES];
	UINT16		usNumComposites;
	INT16		sOffsetX;
	INT16		sOffsetY;
	UINT16		usHeight;
	UINT16		usWidth;
	UINT8		bFlags;
	UINT8		bFlashIndex;
};

extern INT16 gsGlobalCursorYOffset;

// Globals for cursor database offset values
extern UINT16 gsCurMouseHeight;
extern UINT16 gsCurMouseWidth;

void SetExternMouseCursor(SGPVObject const&, UINT16 region_idx);

typedef void (*MOUSEBLT_HOOK)(void);

void InitCursorDatabase(CursorFileData* pCursorFileData, CursorData* pCursorData, UINT16 suNumDataFiles);
void SetMouseBltHook(MOUSEBLT_HOOK pMouseBltOverride);

void SetExternVOData(UINT32 uiCursorIndex, HVOBJECT hVObject, UINT16 usSubIndex);

UINT32 ModifyCursorIndex(UINT32 uiCursorIndex);

#endif
