#ifndef __RENDER_DIRTY_H
#define __RENDER_DIRTY_H

#define BGND_FLAG_PERMANENT		0x80000000
#define BGND_FLAG_SINGLE			0x40000000
#define BGND_FLAG_SAVE_Z			0x20000000
#define BGND_FLAG_MERC				0x10000000
#define BGND_FLAG_SAVERECT		0x08000000
#define BGND_FLAG_TOPMOST			0x04000000
#define BGND_FLAG_ANIMATED		0x00000001

#define VOVERLAY_DIRTYBYTEXT					0x00000001
#define	VOVERLAY_STARTDISABLED				0x00000002


#define VOVERLAY_DESC_TEXT			0x00001000
#define VOVERLAY_DESC_DISABLED	0x00002000
#define VOVERLAY_DESC_POSITION	0x00004000


struct _VIDEO_OVERLAY;

// Callback for topmost blitters
typedef void (*OVERLAY_CALLBACK)(struct _VIDEO_OVERLAY * );

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
typedef struct _VIDEO_OVERLAY
{
		UINT32						uiFlags;
		BOOLEAN						fAllocated;
		BOOLEAN						fDisabled;
		BOOLEAN						fActivelySaving;
		BOOLEAN						fDeletionPending;
		INT32						uiBackground;
		BACKGROUND_SAVE		*pBackground;
		INT16							*pSaveArea;
		UINT32						uiUserData[ 5 ];
		UINT32						uiFontID;
		INT16							sX;
		INT16							sY;
		UINT8							ubFontBack;
		UINT8							ubFontFore;
		wchar_t						zText[ 200 ];
		UINT32						uiDestBuff;
		OVERLAY_CALLBACK		BltCallback;


} VIDEO_OVERLAY;


// Struct for init topmost blitter
typedef struct
{
		UINT32			uiFlags;
		BOOLEAN			fDisabled;
		INT16				sLeft;
		INT16				sTop;
		INT16				sRight;
		INT16				sBottom;
		UINT32			uiFontID;
		INT16				sX;
		INT16				sY;
		UINT8				ubFontBack;
		UINT8				ubFontFore;
		wchar_t				pzText[ 200 ];
		OVERLAY_CALLBACK		BltCallback;

}	VIDEO_OVERLAY_DESC;


// GLOBAL VARIABLES
SGPRect		gDirtyClipRect;


// DIRTY QUEUE
void AddBaseDirtyRect( INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom );
BOOLEAN ExecuteBaseDirtyRectQueue( );


// BACKGROUND RECT BUFFERING STUFF
BOOLEAN InitializeBackgroundRects(void);
BOOLEAN ShutdownBackgroundRects(void);
INT32 RegisterBackgroundRect(UINT32 uiFlags, INT16 *pSaveArea, INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom);
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


// GPRINTF DIRTY STUFF
UINT16 gprintfdirty(INT16 x, INT16 y, const wchar_t *pFontString, ...);
UINT16 gprintfinvalidate(INT16 x, INT16 y, const wchar_t* pFontString, ...);
UINT16 gprintfRestore(INT16 x, INT16 y, const wchar_t *pFontString, ...);


// VIDEO OVERLAY STUFF
INT32 RegisterVideoOverlay( UINT32 uiFlags, VIDEO_OVERLAY_DESC *pTopmostDesc );
void ExecuteVideoOverlays( );
BOOLEAN UpdateVideoOverlay( VIDEO_OVERLAY_DESC *pTopmostDesc, UINT32 iBlitterIndex, BOOLEAN fForceAll );
void SaveVideoOverlaysArea( UINT32 uiSrcBuffer );
void DeleteVideoOverlaysArea( );
void AllocateVideoOverlaysArea( );
void ExecuteVideoOverlaysToAlternateBuffer( UINT32 uiNewDestBuffer );
void RemoveVideoOverlay( INT32 iVideoOverlay );
BOOLEAN RestoreShiftedVideoOverlays( INT16 sShiftX, INT16 sShiftY );
void EnableVideoOverlay( BOOLEAN fEnable, INT32 iOverlayIndex );


void BlitMFont( VIDEO_OVERLAY *pBlitter );

BOOLEAN BlitBufferToBuffer(UINT32 uiSrcBuffer, UINT32 uiDestBuffer, UINT16 usSrcX, UINT16 usSrcY, UINT16 usWidth, UINT16 usHeight);

#endif
