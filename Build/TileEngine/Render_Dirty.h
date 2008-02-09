#ifndef __RENDER_DIRTY_H
#define __RENDER_DIRTY_H

#include "JA2Types.h"


#define NO_BGND_RECT -1

#define BGND_FLAG_PERMANENT		0x80000000
#define BGND_FLAG_SINGLE			0x40000000
#define BGND_FLAG_SAVE_Z			0x20000000
#define BGND_FLAG_MERC				0x10000000
#define BGND_FLAG_SAVERECT		0x08000000
#define BGND_FLAG_TOPMOST			0x04000000
#define BGND_FLAG_ANIMATED		0x00000001

#define VOVERLAY_DIRTYBYTEXT					0x00000001
#define	VOVERLAY_STARTDISABLED				0x00000002


// Callback for topmost blitters
typedef void (*OVERLAY_CALLBACK)(VIDEO_OVERLAY*);

// Struct for backgrounds
typedef struct
{
	BOOLEAN		fAllocated;
	BOOLEAN		fFilled;
	BOOLEAN		fFreeMemory;
	BOOLEAN		fZBuffer;
	UINT32		uiFlags;
	INT16			*pSaveArea;
	INT16			*pZSaveArea;
	INT16			sLeft;
	INT16			sTop;
	INT16			sRight;
	INT16			sBottom;
	INT16			sWidth;
	INT16			sHeight;
	BOOLEAN		fPendingDelete;
	BOOLEAN		fDisabled;

} BACKGROUND_SAVE;

// Struct for topmost blitters
struct VIDEO_OVERLAY
{
		UINT32						uiFlags;
		BOOLEAN						fAllocated;
		BOOLEAN						fDisabled;
		BOOLEAN						fActivelySaving;
		BOOLEAN						fDeletionPending;
		INT32						uiBackground;
		BACKGROUND_SAVE		*pBackground;
		INT16							*pSaveArea;
		UINT32						uiFontID;
		INT16							sX;
		INT16							sY;
		UINT8							ubFontBack;
		UINT8							ubFontFore;
		wchar_t						zText[ 200 ];
	SGPVSurface* uiDestBuff;
		OVERLAY_CALLBACK		BltCallback;
};


// Struct for init topmost blitter
typedef struct
{
		INT16				sLeft;
		INT16				sTop;
		INT16				sRight;
		INT16				sBottom;
		UINT32			uiFontID;
		UINT8				ubFontBack;
		UINT8				ubFontFore;
		wchar_t				pzText[ 200 ];
		OVERLAY_CALLBACK		BltCallback;
}	VIDEO_OVERLAY_DESC;


// GLOBAL VARIABLES
SGPRect		gDirtyClipRect;


// DIRTY QUEUE
void AddBaseDirtyRect( INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom );
BOOLEAN ExecuteBaseDirtyRectQueue(void);


// BACKGROUND RECT BUFFERING STUFF
BOOLEAN InitializeBackgroundRects(void);
BOOLEAN ShutdownBackgroundRects(void);
INT32 RegisterBackgroundRect(UINT32 uiFlags, INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom);
BOOLEAN FreeBackgroundRect(INT32 iIndex);
BOOLEAN FreeBackgroundRectPending(INT32 iIndex);
BOOLEAN FreeBackgroundRectType(UINT32 uiFlags);
BOOLEAN RestoreBackgroundRects(void);
BOOLEAN SaveBackgroundRects(void);
BOOLEAN InvalidateBackgroundRects(void);
BOOLEAN UpdateSaveBuffer(void);
BOOLEAN RestoreExternBackgroundRect( INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight );
void SetBackgroundRectFilled( UINT32 uiBackgroundID );
BOOLEAN EmptyBackgroundRects( void );


/* Dirties a single-frame rect exactly the size needed to save the background
 * for a given call to gprintf. Note that this must be called before the
 * backgrounds are saved, and before the actual call to gprintf that writes to
 * the video buffer. */
UINT16 gprintfdirty(INT16 x, INT16 y, const wchar_t *pFontString, ...);

UINT16 gprintfinvalidate(INT16 x, INT16 y, const wchar_t* pFontString, ...);
UINT16 gprintfRestore(INT16 x, INT16 y, const wchar_t *pFontString, ...);


// VIDEO OVERLAY STUFF
VIDEO_OVERLAY* RegisterVideoOverlay(UINT32 uiFlags, const VIDEO_OVERLAY_DESC* pTopmostDesc);
void ExecuteVideoOverlays(void);
void SaveVideoOverlaysArea(SGPVSurface* src);
void DeleteVideoOverlaysArea(void);
void AllocateVideoOverlaysArea(void);
void ExecuteVideoOverlaysToAlternateBuffer(SGPVSurface* buffer);
void RemoveVideoOverlay(VIDEO_OVERLAY*);
BOOLEAN RestoreShiftedVideoOverlays( INT16 sShiftX, INT16 sShiftY );
void EnableVideoOverlay(BOOLEAN fEnable, VIDEO_OVERLAY*);
void SetVideoOverlayTextF(VIDEO_OVERLAY*, const wchar_t* fmt, ...);
void SetVideoOverlayPos(VIDEO_OVERLAY*, INT16 X, INT16 Y);


void BlitMFont( VIDEO_OVERLAY *pBlitter );

BOOLEAN BlitBufferToBuffer(SGPVSurface* src, SGPVSurface* dst, UINT16 usSrcX, UINT16 usSrcY, UINT16 usWidth, UINT16 usHeight);

#endif
