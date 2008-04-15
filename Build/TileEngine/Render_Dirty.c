#include "Font.h"
#include "Local.h"
#include "WorldDef.h"
#include "RenderWorld.h"
#include "VSurface.h"
#include "Render_Dirty.h"
#include "SysUtil.h"
#include "Video.h"
#include "VObject_Blitters.h"
#include <stdarg.h>
#include "MemMan.h"
#include "Debug.h"

#ifdef JA2BETAVERSION
#	include "JAScreens.h"
#endif


#define		DIRTY_QUEUES				200
#define		BACKGROUND_BUFFERS	500
#define		VIDEO_OVERLAYS			100


// Struct for backgrounds
struct BACKGROUND_SAVE
{
	BOOLEAN fAllocated;
	BOOLEAN fFilled;
	BOOLEAN fFreeMemory;
	UINT32  uiFlags;
	UINT16* pSaveArea;
	UINT16* pZSaveArea;
	INT16   sLeft;
	INT16   sTop;
	INT16   sRight;
	INT16   sBottom;
	INT16   sWidth;
	INT16   sHeight;
	BOOLEAN fPendingDelete;
	BOOLEAN fDisabled;
};


static BACKGROUND_SAVE gBackSaves[BACKGROUND_BUFFERS];
UINT32 guiNumBackSaves=0;

static VIDEO_OVERLAY gVideoOverlays[VIDEO_OVERLAYS];
static UINT32        guiNumVideoOverlays = 0;


#define FOR_ALL_VIDEO_OVERLAYS(iter)                                      \
	for (VIDEO_OVERLAY* iter        = gVideoOverlays,                       \
	                  * iter##__end = gVideoOverlays + guiNumVideoOverlays; \
	     iter != iter##__end;                                               \
	     ++iter)                                                            \
		if (!iter->fAllocated || iter->fDisabled) continue; else


SGPRect gDirtyClipRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };


BOOLEAN		gfViewportDirty=FALSE;


void AddBaseDirtyRect(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom)
{
	if (iLeft < 0)            iLeft = 0;
	if (iLeft > SCREEN_WIDTH) iLeft = SCREEN_WIDTH;

	if (iTop < 0)             iTop = 0;
	if (iTop > SCREEN_HEIGHT) iTop = SCREEN_HEIGHT;

	if (iRight < 0)            iRight = 0;
	if (iRight > SCREEN_WIDTH) iRight = SCREEN_WIDTH;

	if (iBottom < 0)             iBottom = 0;
	if (iBottom > SCREEN_HEIGHT) iBottom = SCREEN_HEIGHT;

	if (iLeft == iRight || iTop == iBottom) return;

	if (iLeft   == gsVIEWPORT_START_X        &&
			iRight  == gsVIEWPORT_END_X          &&
			iTop    == gsVIEWPORT_WINDOW_START_Y &&
			iBottom == gsVIEWPORT_WINDOW_END_Y)
	{
		gfViewportDirty = TRUE;
		return;
	}

	InvalidateRegionEx(iLeft, iTop, iRight, iBottom);
}


BOOLEAN ExecuteBaseDirtyRectQueue( )
{
	if(gfViewportDirty)
	{
		//InvalidateRegion(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X, gsVIEWPORT_END_Y);
		InvalidateScreen( );
		gfViewportDirty=FALSE;
		return(TRUE);
	}



	return( TRUE );
}


static BACKGROUND_SAVE* GetFreeBackgroundBuffer(void)
{
	for (UINT32 i = 0; i < guiNumBackSaves; ++i)
	{
		BACKGROUND_SAVE* const b = &gBackSaves[i];
		if (!b->fAllocated && !b->fFilled) return b;
	}

	if (guiNumBackSaves < BACKGROUND_BUFFERS)
	{
		return &gBackSaves[guiNumBackSaves++];
	}
#ifdef JA2BETAVERSION
	else
	{
		//else display an error message
		DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("ERROR! GetFreeBackgroundBuffer(): Trying to allocate more saves then there is room:  guiCurrentScreen = %d", guiCurrentScreen));
	}
#endif

	return NO_BGND_RECT;
}


static void RecountBackgrounds(void)
{
INT32 uiCount;

	for(uiCount=guiNumBackSaves-1; (uiCount >=0) ; uiCount--)
	{
		if((gBackSaves[uiCount].fAllocated) || (gBackSaves[uiCount].fFilled))
		{
			guiNumBackSaves=(UINT32)(uiCount+1);
			break;
		}
	}
}


BACKGROUND_SAVE* RegisterBackgroundRect(const UINT32 uiFlags, INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom)
{
	const INT32 ClipX1 = gDirtyClipRect.iLeft;
	const INT32 ClipY1 = gDirtyClipRect.iTop;
	const INT32 ClipX2 = gDirtyClipRect.iRight;
	const INT32 ClipY2 = gDirtyClipRect.iBottom;

	const UINT32 usHeight = sBottom - sTop;
	const UINT32 usWidth  = sRight -  sLeft;

	const INT32 iTempX = sLeft;
	const INT32 iTempY = sTop;

	// Clip to rect
	const INT32 uiLeftSkip   = __min(ClipX1 -   min(ClipX1, iTempX),                   (INT32)usWidth);
	const INT32 uiTopSkip    = __min(ClipY1 - __min(ClipY1, iTempY),                   (INT32)usHeight);
	const INT32 uiRightSkip  = __min(  max(ClipX2, iTempX + (INT32)usWidth)  - ClipX2, (INT32)usWidth);
	const INT32 uiBottomSkip = __min(__max(ClipY2, iTempY + (INT32)usHeight) - ClipY2, (INT32)usHeight);

	// check if whole thing is clipped
	if (uiLeftSkip >= (INT32)usWidth  || uiRightSkip  >= (INT32)usWidth)  return NO_BGND_RECT;
	if (uiTopSkip  >= (INT32)usHeight || uiBottomSkip >= (INT32)usHeight) return NO_BGND_RECT;

	// Set re-set values given based on clipping
	sLeft   += uiLeftSkip;
	sRight  -= uiRightSkip;
	sTop    += uiTopSkip;
	sBottom -= uiBottomSkip;

	BACKGROUND_SAVE* const b = GetFreeBackgroundBuffer();
	if (b == NO_BGND_RECT) return NO_BGND_RECT;

	memset(b, 0, sizeof(*b));

	const UINT32 uiBufSize = (sRight - sLeft) * (sBottom - sTop);
	if (uiBufSize == 0) return NO_BGND_RECT;

	if (uiFlags & BGND_FLAG_SAVERECT)
	{
		b->pSaveArea = MALLOCN(UINT16, uiBufSize);
		if (b->pSaveArea == NULL) return NO_BGND_RECT;
	}

	if (uiFlags & BGND_FLAG_SAVE_Z)
	{
		b->pZSaveArea = MALLOCN(UINT16, uiBufSize);
		if (b->pZSaveArea == NULL) return NO_BGND_RECT;
	}

	b->fFreeMemory = TRUE;
	b->fAllocated  = TRUE;
	b->uiFlags     = uiFlags;
	b->sLeft       = sLeft;
	b->sTop        = sTop;
	b->sRight      = sRight;
	b->sBottom     = sBottom;
	b->sWidth      = sRight  - sLeft;
	b->sHeight     = sBottom - sTop;
	b->fFilled     = FALSE;

	return b;
}


void RegisterBackgroundRectSingleFilled(const INT16 left, const INT16 top, const INT16 right, const INT16 bottom)
{
	BACKGROUND_SAVE* const b = RegisterBackgroundRect(BGND_FLAG_SINGLE, left, top, right, bottom);
	if (b == NO_BGND_RECT) return;

	b->fFilled = TRUE;
	AddBaseDirtyRect(b->sLeft, b->sTop, b->sRight, b->sBottom);
}


void RestoreBackgroundRects(void)
{
	UINT32        uiDestPitchBYTES;
	UINT16* const pDestBuf = (UINT16*)LockVideoSurface(FRAME_BUFFER, &uiDestPitchBYTES);
	UINT32        uiSrcPitchBYTES;
	UINT16* const pSrcBuf  = (UINT16*)LockVideoSurface(guiSAVEBUFFER, &uiSrcPitchBYTES);

	for (UINT32 i = 0; i < guiNumBackSaves; ++i)
	{
		const BACKGROUND_SAVE* const b = &gBackSaves[i];
		if (!b->fFilled || b->fDisabled) continue;

		if (b->pSaveArea != NULL)
		{
			Blt16BPPTo16BPP(pDestBuf, uiDestPitchBYTES, b->pSaveArea, b->sWidth * 2, b->sLeft, b->sTop, 0, 0, b->sWidth, b->sHeight);
			AddBaseDirtyRect(b->sLeft, b->sTop, b->sRight, b->sBottom);
		}
		else if (b->pZSaveArea != NULL)
		{
			Blt16BPPTo16BPP(gpZBuffer, uiDestPitchBYTES, b->pZSaveArea, b->sWidth * 2, b->sLeft, b->sTop, 0, 0, b->sWidth, b->sHeight);
		}
		else
		{
			Blt16BPPTo16BPP(pDestBuf, uiDestPitchBYTES, pSrcBuf, uiSrcPitchBYTES, b->sLeft, b->sTop, b->sLeft, b->sTop, b->sWidth, b->sHeight);
			AddBaseDirtyRect(b->sLeft, b->sTop, b->sRight, b->sBottom);
		}
	}

	UnLockVideoSurface(FRAME_BUFFER);
	UnLockVideoSurface(guiSAVEBUFFER);

	EmptyBackgroundRects();
}


void EmptyBackgroundRects(void)
{
	for (UINT32 i = 0; i < guiNumBackSaves; ++i)
	{
		BACKGROUND_SAVE* const b = &gBackSaves[i];
		if (b->fFilled)
		{
			b->fFilled = FALSE;

			if (!b->fAllocated && b->fFreeMemory)
			{
				if (b->pSaveArea  != NULL) MemFree(b->pSaveArea);
				if (b->pZSaveArea != NULL) MemFree(b->pZSaveArea);

				b->fAllocated  = FALSE;
				b->fFreeMemory = FALSE;
				b->fFilled     = FALSE;
				b->pSaveArea   = NULL;

				RecountBackgrounds();
			}
		}

		if (b->uiFlags & BGND_FLAG_SINGLE || b->fPendingDelete)
		{
			if (b->fFreeMemory)
			{
				if (b->pSaveArea != NULL)  MemFree(b->pSaveArea);
				if (b->pZSaveArea != NULL) MemFree(b->pZSaveArea);
			}

			b->fAllocated     = FALSE;
			b->fFreeMemory    = FALSE;
			b->fFilled        = FALSE;
			b->pSaveArea      = NULL;
			b->fPendingDelete = FALSE;

			RecountBackgrounds();
		}
	}
}


void SaveBackgroundRects(void)
{
	UINT32        uiDestPitchBYTES;
	UINT16* const pSrcBuf  = (UINT16*)LockVideoSurface(FRAME_BUFFER, &uiDestPitchBYTES);
	UINT32        uiSrcPitchBYTES;
	UINT16* const pDestBuf = (UINT16*)LockVideoSurface(guiSAVEBUFFER, &uiSrcPitchBYTES);

	for (UINT32 i = 0; i < guiNumBackSaves; ++i)
	{
		BACKGROUND_SAVE* const b = &gBackSaves[i];
		if (!b->fAllocated || b->fDisabled) continue;

		if (b->pSaveArea != NULL)
		{
			Blt16BPPTo16BPP(b->pSaveArea, b->sWidth * 2, pSrcBuf, uiDestPitchBYTES, 0, 0, b->sLeft, b->sTop, b->sWidth, b->sHeight);
		}
		else if (b->pZSaveArea != NULL)
		{
			Blt16BPPTo16BPP(b->pZSaveArea, b->sWidth * 2, gpZBuffer, uiDestPitchBYTES, 0, 0, b->sLeft, b->sTop, b->sWidth, b->sHeight);
		}
		else
		{
			AddBaseDirtyRect(b->sLeft, b->sTop, b->sRight, b->sBottom);
		}

		b->fFilled = TRUE;
	}

	UnLockVideoSurface(FRAME_BUFFER);
	UnLockVideoSurface(guiSAVEBUFFER);
}


void FreeBackgroundRect(BACKGROUND_SAVE* const b)
{
	if (b == NULL) return;

	b->fAllocated = FALSE;
	RecountBackgrounds();
}


void FreeBackgroundRectPending(BACKGROUND_SAVE* const b)
{
	b->fPendingDelete = TRUE;
}


static void FreeBackgroundRectNow(BACKGROUND_SAVE* const b)
{
	if (b->fFreeMemory)
	{
		//MemFree(b->pSaveArea);
		if (b->pZSaveArea != NULL) MemFree(b->pZSaveArea);
	}

	b->fAllocated  = FALSE;
	b->fFreeMemory = FALSE;
	b->fFilled     = FALSE;
	b->pSaveArea   = NULL;
}


void FreeBackgroundRectType(const UINT32 uiFlags)
{
	for (UINT32 i = 0; i < guiNumBackSaves; ++i)
	{
		BACKGROUND_SAVE* const b = &gBackSaves[i];
		if (b->uiFlags & uiFlags) FreeBackgroundRectNow(b);
	}

	RecountBackgrounds();
}


void InitializeBackgroundRects(void)
{
	guiNumBackSaves = 0;
}


void InvalidateBackgroundRects(void)
{
	for (UINT32 i = 0; i < guiNumBackSaves; ++i)
	{
		gBackSaves[i].fFilled = FALSE;
	}
}


void ShutdownBackgroundRects(void)
{
	for (UINT32 i = 0; i < guiNumBackSaves; ++i)
	{
		BACKGROUND_SAVE* const b = &gBackSaves[i];
		if (b->fAllocated) FreeBackgroundRectNow(b);
	}

	RecountBackgrounds();
}


static void DisableBackgroundRect(BACKGROUND_SAVE* const b, const BOOLEAN fDisabled)
{
	b->fDisabled = fDisabled;
}


void UpdateSaveBuffer(void)
{
	// Update saved buffer - do for the viewport size ony!
	BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, 0, gsVIEWPORT_WINDOW_START_Y, SCREEN_WIDTH, gsVIEWPORT_WINDOW_END_Y - gsVIEWPORT_WINDOW_START_Y);
}


void RestoreExternBackgroundRect(const INT16 sLeft, const INT16 sTop, const INT16 sWidth, const INT16 sHeight)
{
	Assert(0 <= sLeft && sLeft + sWidth <= SCREEN_WIDTH && 0 <= sTop && sTop + sHeight <= SCREEN_HEIGHT);

	BlitBufferToBuffer(guiSAVEBUFFER, FRAME_BUFFER, sLeft, sTop, sWidth, sHeight);

	// Add rect to frame buffer queue
	InvalidateRegionEx(sLeft, sTop, sLeft + sWidth, sTop + sHeight);
}


void RestoreExternBackgroundRectGivenID(const BACKGROUND_SAVE* const b)
{
	if (!b->fAllocated) return;
	RestoreExternBackgroundRect(b->sLeft, b->sTop, b->sWidth, b->sHeight);
}


UINT16 gprintfdirty(INT16 x, INT16 y, const wchar_t *pFontString, ...)
{
va_list argptr;
wchar_t	string[512];
UINT16 uiStringLength, uiStringHeight;

	Assert(pFontString!=NULL);

	va_start(argptr, pFontString);       	// Set up variable argument pointer
	vswprintf(string, lengthof(string), pFontString, argptr);	// process gprintf string (get output str)
	va_end(argptr);

	uiStringLength = StringPixLength(string, FontDefault);
	uiStringHeight = GetFontHeight(FontDefault);

	if ( uiStringLength > 0 )
	{
		RegisterBackgroundRectSingleFilled(x, y, x + uiStringLength, y + uiStringHeight);
	}

	return(uiStringLength);
}


UINT16 gprintfinvalidate(INT16 x, INT16 y, const wchar_t* pFontString, ...)
{
va_list argptr;
wchar_t	string[512];
UINT16 uiStringLength, uiStringHeight;

	Assert(pFontString!=NULL);

	va_start(argptr, pFontString);       	// Set up variable argument pointer
	vswprintf(string, lengthof(string), pFontString, argptr);	// process gprintf string (get output str)
	va_end(argptr);

	uiStringLength=StringPixLength(string, FontDefault);
	uiStringHeight=GetFontHeight(FontDefault);

	if ( uiStringLength > 0 )
	{
		InvalidateRegionEx(x, y, (INT16)(x + uiStringLength), (INT16)(y + uiStringHeight));
	}
	return(uiStringLength);
}


UINT16 gprintfRestore(INT16 x, INT16 y, const wchar_t *pFontString, ...)
{
va_list argptr;
wchar_t	string[512];
UINT16 uiStringLength, uiStringHeight;

	Assert(pFontString!=NULL);

	va_start(argptr, pFontString);       	// Set up variable argument pointer
	vswprintf(string, lengthof(string), pFontString, argptr);	// process gprintf string (get output str)
	va_end(argptr);

	uiStringLength=StringPixLength(string, FontDefault);
	uiStringHeight=GetFontHeight(FontDefault);

	if ( uiStringLength > 0 )
	{
		RestoreExternBackgroundRect( x, y, uiStringLength, uiStringHeight );
	}

	return(uiStringLength);
}


static VIDEO_OVERLAY* GetFreeVideoOverlay(void)
{
	VIDEO_OVERLAY* v;
	for (v = gVideoOverlays; v != gVideoOverlays + guiNumVideoOverlays; ++v)
	{
		if (!v->fAllocated) return v;
	}
	if (v == endof(gVideoOverlays)) return NULL;
	++guiNumVideoOverlays;
	return v;
}


VIDEO_OVERLAY* RegisterVideoOverlay(const UINT32 uiFlags, const VIDEO_OVERLAY_DESC* const vod)
{
	INT16 right;
	INT16 bottom;
	if (uiFlags & VOVERLAY_DIRTYBYTEXT)
	{
		const UINT16 uiStringLength = StringPixLength(vod->pzText, vod->uiFontID);
		const UINT16 uiStringHeight = GetFontHeight(vod->uiFontID);
		right  = vod->sLeft + uiStringLength;
		bottom = vod->sTop  + uiStringHeight;
	}
	else
	{
		right  = vod->sRight;
		bottom = vod->sBottom;
	}

	BACKGROUND_SAVE* const bgs = RegisterBackgroundRect(BGND_FLAG_PERMANENT, vod->sLeft, vod->sTop, right, bottom);
	if (bgs == NO_BGND_RECT) return NULL;

	// Get next free topmost blitter index
	VIDEO_OVERLAY* const v = GetFreeVideoOverlay();
	if (v == NULL) return NULL;

	// Init new blitter
	memset(v, 0, sizeof(*v));
	v->uiFlags      = uiFlags;
	v->fAllocated   = 2;
	v->background   = bgs;
	v->uiFontID     = vod->uiFontID;
	v->sX           = vod->sLeft;
	v->sY           = vod->sTop;
	v->ubFontBack   = vod->ubFontBack;
	v->ubFontFore   = vod->ubFontFore;
	v->uiDestBuff   = FRAME_BUFFER;
	v->BltCallback  = vod->BltCallback;
	wcslcpy(v->zText, vod->pzText, lengthof(v->zText));

	// Set disabled flag to true
	if (uiFlags & VOVERLAY_STARTDISABLED)
	{
		v->fDisabled = TRUE;
		DisableBackgroundRect(v->background, TRUE);
	}

	return v;
}


void RemoveVideoOverlay(VIDEO_OVERLAY* const v)
{
	if (v == NULL) return;
	if (!v->fAllocated) return;

	// Check if we are actively scrolling
	if (v->fActivelySaving)
	{
		v->fDeletionPending = TRUE;
	}
	else
	{
		//RestoreExternBackgroundRectGivenID(v->background);

		FreeBackgroundRect(v->background);

		if (v->pSaveArea != NULL) MemFree(v->pSaveArea);
		v->pSaveArea = NULL;

		v->fAllocated = FALSE;
	}
}


// FUnctions for entrie array of blitters
void ExecuteVideoOverlays(void)
{
	FOR_ALL_VIDEO_OVERLAYS(v)
	{
		// If we are scrolling but haven't saved yet, don't!
		if (!v->fActivelySaving && gfScrollInertia > 0) continue;

		// ATE: Wait a frame before executing!
		switch (v->fAllocated)
		{
			case 1: v->BltCallback(v); break;
			case 2: v->fAllocated = 1; break;
		}
	}
}


void ExecuteVideoOverlaysToAlternateBuffer(SGPVSurface* const buffer)
{
	FOR_ALL_VIDEO_OVERLAYS(v)
	{
		if (!v->fActivelySaving) continue;

		SGPVSurface* const old_dst = v->uiDestBuff;
		v->uiDestBuff = buffer;
		v->BltCallback(v);
		v->uiDestBuff = old_dst;
	}
}


void AllocateVideoOverlaysArea(void)
{
	FOR_ALL_VIDEO_OVERLAYS(v)
	{
		// Get buffer size
		const BACKGROUND_SAVE* const bgs       = v->background;
		const UINT32                 uiBufSize = (bgs->sRight - bgs->sLeft) * (bgs->sBottom - bgs->sTop);

		v->fActivelySaving = TRUE;
		v->pSaveArea       = MALLOCN(UINT16, uiBufSize);
	}
}


static void AllocateVideoOverlayArea(VIDEO_OVERLAY* const v)
{
	Assert(v->fAllocated);
	Assert(!v->fDisabled);

	// Get buffer size
	const BACKGROUND_SAVE* const bgs = v->background;
	const UINT32 uiBufSize = (bgs->sRight - bgs->sLeft) * (bgs->sBottom - bgs->sTop);

	v->fActivelySaving = TRUE;
	v->pSaveArea       = MALLOCN(UINT16, uiBufSize);
}


void SaveVideoOverlaysArea(SGPVSurface* const src)
{
	UINT32        uiSrcPitchBYTES;
	UINT16* const pSrcBuf = (UINT16*)LockVideoSurface(src, &uiSrcPitchBYTES);

	FOR_ALL_VIDEO_OVERLAYS(v)
	{
		// OK, if our saved area is null, allocate it here!
		if (v->pSaveArea == NULL)
		{
			AllocateVideoOverlayArea(v);
			if (v->pSaveArea == NULL) continue;
		}

		// Save data from frame buffer!
		const BACKGROUND_SAVE* const b = v->background;
		Blt16BPPTo16BPP(v->pSaveArea, b->sWidth * 2, pSrcBuf, uiSrcPitchBYTES, 0, 0, b->sLeft, b->sTop, b->sWidth, b->sHeight);
	}

	UnLockVideoSurface(src);
}


void DeleteVideoOverlaysArea(void)
{
	FOR_ALL_VIDEO_OVERLAYS(v)
	{
		if (v->pSaveArea != NULL) MemFree(v->pSaveArea);
		v->pSaveArea       = NULL;
		v->fActivelySaving = FALSE;
		if (v->fDeletionPending) RemoveVideoOverlay(v);
	}
}


void RestoreShiftedVideoOverlays(const INT16 sShiftX, const INT16 sShiftY)
{
	const INT32 ClipX1 = 0;
	const INT32 ClipY1 = gsVIEWPORT_WINDOW_START_Y;
	const INT32 ClipX2 = SCREEN_WIDTH;
	const INT32 ClipY2 = gsVIEWPORT_WINDOW_END_Y - 1;

	UINT32        uiDestPitchBYTES;
	UINT16* const pDestBuf = (UINT16*)LockVideoSurface(BACKBUFFER, &uiDestPitchBYTES);

	FOR_ALL_VIDEO_OVERLAYS(v)
	{
		if (v->pSaveArea == NULL) continue;

		// Get restore background values
		const BACKGROUND_SAVE* const b = v->background;
		INT16  sLeft    = b->sLeft;
		INT16  sTop     = b->sTop;
		INT16  sRight   = b->sRight;
		INT16  sBottom  = b->sBottom;
		UINT32 usHeight = b->sHeight;
		UINT32 usWidth  = b->sWidth;

		// Clip!!
		const INT32 iTempX = sLeft - sShiftX;
		const INT32 iTempY = sTop  - sShiftY;

		// Clip to rect
		const INT32 uiLeftSkip   = __min(ClipX1 -   min(ClipX1, iTempX),                   (INT32)usWidth);
		const INT32 uiTopSkip    = __min(ClipY1 - __min(ClipY1, iTempY),                   (INT32)usHeight);
		const INT32 uiRightSkip  = __min(  max(ClipX2, iTempX + (INT32)usWidth)  - ClipX2, (INT32)usWidth);
		const INT32 uiBottomSkip = __min(__max(ClipY2, iTempY + (INT32)usHeight) - ClipY2, (INT32)usHeight);

		// check if whole thing is clipped
		if (uiLeftSkip >= (INT32)usWidth  || uiRightSkip  >= (INT32)usWidth)  continue;
		if (uiTopSkip  >= (INT32)usHeight || uiBottomSkip >= (INT32)usHeight) continue;

		// Set re-set values given based on clipping
		sLeft    = iTempX + (INT16)uiLeftSkip;
		sTop     = iTempY + (INT16)uiTopSkip;
		sRight   = sRight  - sShiftX - (INT16)uiRightSkip;
		sBottom  = sBottom - sShiftY - (INT16)uiBottomSkip;

		usHeight = sBottom - sTop;
		usWidth  = sRight -  sLeft;

		Blt16BPPTo16BPP(pDestBuf, uiDestPitchBYTES, v->pSaveArea, b->sWidth * 2, sLeft, sTop, uiLeftSkip, uiTopSkip, usWidth, usHeight);

		// Once done, check for pending deletion
		if (v->fDeletionPending) RemoveVideoOverlay(v);
	}

	UnLockVideoSurface(BACKBUFFER);
}


// Common callbacks for topmost blitters
void BlitMFont( VIDEO_OVERLAY *pBlitter )
{
	UINT32 uiDestPitchBYTES;
	UINT16* pDestBuf = (UINT16*)LockVideoSurface(pBlitter->uiDestBuff, &uiDestPitchBYTES);

	SetFont( pBlitter->uiFontID );
	SetFontBackground( pBlitter->ubFontBack );
	SetFontForeground( pBlitter->ubFontFore );

	mprintf_buffer(pDestBuf, uiDestPitchBYTES, pBlitter->sX, pBlitter->sY, pBlitter->zText);

	UnLockVideoSurface( pBlitter->uiDestBuff );

}


BOOLEAN BlitBufferToBuffer(SGPVSurface* const src, SGPVSurface* const dst, const UINT16 usSrcX, const UINT16 usSrcY, const UINT16 usWidth, const UINT16 usHeight)
{
	const SGPRect r = { usSrcX, usSrcY, usSrcX + usWidth, usSrcY + usHeight };
	return BltVideoSurface(dst, src, usSrcX, usSrcY, &r);
}


void EnableVideoOverlay(const BOOLEAN fEnable, VIDEO_OVERLAY* const v)
{
	if (v == NULL) return;
	if (!v->fAllocated) return;

	v->fDisabled = !fEnable;
	DisableBackgroundRect(v->background, !fEnable);
}


void SetVideoOverlayTextF(VIDEO_OVERLAY* const v, const wchar_t* Fmt, ...)
{
	if (v == NULL) return;
	if (!v->fAllocated) return;

	va_list Arg;
	va_start(Arg, Fmt);
	vswprintf(v->zText, lengthof(v->zText), Fmt, Arg);
	va_end(Arg);
}


void SetVideoOverlayPos(VIDEO_OVERLAY* const v, const INT16 X, const INT16 Y)
{
	if (v == NULL) return;
	if (!v->fAllocated) return;

	// If position has changed and flags are of type that use dirty rects, adjust
	if (v->uiFlags & VOVERLAY_DIRTYBYTEXT)
	{
		UINT16 uiStringLength = StringPixLength(v->zText, v->uiFontID);
		UINT16 uiStringHeight = GetFontHeight(v->uiFontID);

		// Delete old rect
		// Remove background
		FreeBackgroundRectPending(v->background);

		v->background = RegisterBackgroundRect(BGND_FLAG_PERMANENT, X, Y, X + uiStringLength, Y + uiStringHeight);
		v->sX         = X;
		v->sY         = Y;
	}
}
