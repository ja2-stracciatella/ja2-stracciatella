#ifndef __RENDER_DIRTY_H
#define __RENDER_DIRTY_H

#include "JA2Types.h"


#define NO_BGND_RECT NULL

#define BGND_FLAG_PERMANENT		0x80000000
#define BGND_FLAG_SINGLE			0x40000000
#define BGND_FLAG_SAVE_Z			0x20000000
#define BGND_FLAG_SAVERECT		0x08000000
#define BGND_FLAG_ANIMATED		0x00000001

#define VOVERLAY_DIRTYBYTEXT					0x00000001
#define	VOVERLAY_STARTDISABLED				0x00000002


// Callback for topmost blitters
typedef void (*OVERLAY_CALLBACK)(VIDEO_OVERLAY*);

// Struct for topmost blitters
struct VIDEO_OVERLAY
{
	UINT32           uiFlags;
	BOOLEAN          fAllocated;
	BOOLEAN          fDisabled;
	BOOLEAN          fActivelySaving;
	BOOLEAN          fDeletionPending;
	BACKGROUND_SAVE* background;
	UINT16*          pSaveArea;
	Font             uiFontID;
	INT16            sX;
	INT16            sY;
	UINT8            ubFontBack;
	UINT8            ubFontFore;
	wchar_t          zText[200];
	SGPVSurface*     uiDestBuff;
	OVERLAY_CALLBACK BltCallback;
};


// Struct for init topmost blitter
typedef struct
{
		INT16				sLeft;
		INT16				sTop;
		INT16				sRight;
		INT16				sBottom;
		Font             uiFontID;
		UINT8				ubFontBack;
		UINT8				ubFontFore;
		wchar_t const*   text;
		OVERLAY_CALLBACK		BltCallback;
}	VIDEO_OVERLAY_DESC;


// GLOBAL VARIABLES
extern SGPRect gDirtyClipRect;


// DIRTY QUEUE
void AddBaseDirtyRect(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom);
void ExecuteBaseDirtyRectQueue(void);


// BACKGROUND RECT BUFFERING STUFF
void             InitializeBackgroundRects(void);
void             ShutdownBackgroundRects(void);
BACKGROUND_SAVE* RegisterBackgroundRect(UINT32 uiFlags, INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom);
void             FreeBackgroundRect(BACKGROUND_SAVE*);
void             FreeBackgroundRectPending(BACKGROUND_SAVE*);
void             FreeBackgroundRectType(UINT32 uiFlags);
void             RestoreBackgroundRects(void);
void             SaveBackgroundRects(void);
void             InvalidateBackgroundRects(void);
void             UpdateSaveBuffer(void);
void             RestoreExternBackgroundRect(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight);
void             RegisterBackgroundRectSingleFilled(INT16 left, INT16 top, INT16 right, INT16 bottom);
void             EmptyBackgroundRects(void);
void             RestoreExternBackgroundRectGivenID(const BACKGROUND_SAVE*);


void GDirtyPrint(INT16 x, INT16 y, wchar_t const* str);
void GDirtyPrintF(INT16 x, INT16 y, wchar_t const* fmt, ...);

void GPrintDirty(INT16 x, INT16 y, wchar_t const* str);
void GPrintDirtyF(INT16 x, INT16 y, wchar_t const* fmt, ...);

void GPrintInvalidate(INT16 x, INT16 y, wchar_t const* str);
void GPrintInvalidateF(INT16 x, INT16 y, wchar_t const* fmt, ...);


// VIDEO OVERLAY STUFF
VIDEO_OVERLAY* RegisterVideoOverlay(UINT32 uiFlags, const VIDEO_OVERLAY_DESC* pTopmostDesc);
void ExecuteVideoOverlays(void);
void SaveVideoOverlaysArea(SGPVSurface* src);
void DeleteVideoOverlaysArea(void);
void AllocateVideoOverlaysArea(void);
void ExecuteVideoOverlaysToAlternateBuffer(SGPVSurface* buffer);
void RemoveVideoOverlay(VIDEO_OVERLAY*);
void RestoreShiftedVideoOverlays(INT16 sShiftX, INT16 sShiftY);
void EnableVideoOverlay(BOOLEAN fEnable, VIDEO_OVERLAY*);
void SetVideoOverlayTextF(VIDEO_OVERLAY*, const wchar_t* fmt, ...);
void SetVideoOverlayPos(VIDEO_OVERLAY*, INT16 X, INT16 Y);


void BlitMFont( VIDEO_OVERLAY *pBlitter );

void BlitBufferToBuffer(SGPVSurface* src, SGPVSurface* dst, UINT16 usSrcX, UINT16 usSrcY, UINT16 usWidth, UINT16 usHeight);

#endif
