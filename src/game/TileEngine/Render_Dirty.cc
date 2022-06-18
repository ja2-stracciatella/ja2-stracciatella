#include <stdexcept>

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
#include "Debug.h"
#include "UILayout.h"

#include <vector>

#include "Logger.h"


#define BACKGROUND_BUFFERS 500


// Struct for backgrounds
struct BACKGROUND_SAVE
{
	BOOLEAN         fAllocated;
	BOOLEAN         fFilled;
	BOOLEAN         fFreeMemory;
	BackgroundFlags uiFlags;
	UINT16*         pSaveArea;
	UINT16*         pZSaveArea;
	INT16           sLeft;
	INT16           sTop;
	INT16           sRight;
	INT16           sBottom;
	INT16           sWidth;
	INT16           sHeight;
	BOOLEAN         fPendingDelete;
	BOOLEAN         fDisabled;
};


static std::vector<BACKGROUND_SAVE*> gBackSaves;
static UINT32 guiNumBackSaves=0;

static VIDEO_OVERLAY* gVideoOverlays;


#define FOR_EACH_VIDEO_OVERLAY(iter)                                  \
	for (VIDEO_OVERLAY* iter = gVideoOverlays; iter; iter = iter->next) \
		if (iter->fDisabled) continue; else

#define FOR_EACH_VIDEO_OVERLAY_SAFE(iter)                             \
	for (VIDEO_OVERLAY* iter = gVideoOverlays, * iter##__next; iter; iter = iter##__next) \
		if (iter##__next = iter->next, iter->fDisabled) continue; else


SGPRect gDirtyClipRect;


static BOOLEAN gfViewportDirty = FALSE;


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


void ExecuteBaseDirtyRectQueue(void)
{
	if (!gfViewportDirty) return;
	gfViewportDirty = FALSE;

	InvalidateScreen();
}


static BACKGROUND_SAVE* GetFreeBackgroundBuffer(void)
{
	for (UINT32 i = 0; i < guiNumBackSaves; ++i)
	{
		BACKGROUND_SAVE* const b = gBackSaves[i];
		if (!b->fAllocated && !b->fFilled) return b;
	}

	if (guiNumBackSaves == gBackSaves.size())
	{
		// out of back saves capacity
		// let's add some more
		const int increment = 100;
		SLOGD("Increasing background slots to {}", gBackSaves.size() + increment);
		for(int i = 0; i < increment; i++) {
			gBackSaves.push_back(new BACKGROUND_SAVE());
		}
	}

	return gBackSaves[guiNumBackSaves++];
}


BACKGROUND_SAVE* RegisterBackgroundRect(BackgroundFlags const uiFlags, INT16 sLeft, INT16 sTop, INT16 const usWidth, INT16 const usHeight)
{
	const INT32 ClipX1 = gDirtyClipRect.iLeft;
	const INT32 ClipY1 = gDirtyClipRect.iTop;
	const INT32 ClipX2 = gDirtyClipRect.iRight;
	const INT32 ClipY2 = gDirtyClipRect.iBottom;

	INT16 sRight  = sLeft + usWidth;
	INT16 sBottom = sTop  + usHeight;

	const INT32 iTempX = sLeft;
	const INT32 iTempY = sTop;

	// Clip to rect
	const INT32 uiLeftSkip   = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	const INT32 uiTopSkip    = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	const INT32 uiRightSkip  = std::clamp(iTempX + (INT32)usWidth  - ClipX2, 0, (INT32)usWidth);
	const INT32 uiBottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// check if whole thing is clipped
	if (uiLeftSkip >= (INT32)usWidth  || uiRightSkip  >= (INT32)usWidth)  return NO_BGND_RECT;
	if (uiTopSkip  >= (INT32)usHeight || uiBottomSkip >= (INT32)usHeight) return NO_BGND_RECT;

	// Set re-set values given based on clipping
	sLeft   += uiLeftSkip;
	sRight  -= uiRightSkip;
	sTop    += uiTopSkip;
	sBottom -= uiBottomSkip;

	BACKGROUND_SAVE* const b = GetFreeBackgroundBuffer();
	*b = BACKGROUND_SAVE{};

	const UINT32 uiBufSize = (sRight - sLeft) * (sBottom - sTop);
	if (uiBufSize == 0) return NO_BGND_RECT;

	if (uiFlags & BGND_FLAG_SAVERECT) b->pSaveArea  = new UINT16[uiBufSize]{};
	if (uiFlags & BGND_FLAG_SAVE_Z)   b->pZSaveArea = new UINT16[uiBufSize]{};

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


void RegisterBackgroundRectSingleFilled(INT16 const x, INT16 const y, INT16 const w, INT16 const h)
{
	BACKGROUND_SAVE* const b = RegisterBackgroundRect(BGND_FLAG_SINGLE, x, y, w, h);
	if (b == NO_BGND_RECT) return;

	b->fFilled = TRUE;
	AddBaseDirtyRect(b->sLeft, b->sTop, b->sRight, b->sBottom);
}


void RestoreBackgroundRects(void)
{
	{ SGPVSurface::Lock lsrc(guiSAVEBUFFER);
		SGPVSurface::Lock ldst(FRAME_BUFFER);
		UINT16* const pSrcBuf          = lsrc.Buffer<UINT16>();
		UINT32        uiSrcPitchBYTES  = lsrc.Pitch();
		UINT16* const pDestBuf         = ldst.Buffer<UINT16>();
		UINT32        uiDestPitchBYTES = ldst.Pitch();

		for (UINT32 i = 0; i < guiNumBackSaves; ++i)
		{
			const BACKGROUND_SAVE* const b = gBackSaves[i];
			if (!b->fFilled || b->fDisabled) continue;

			if (b->pSaveArea != NULL)
			{
				Blt16BPPTo16BPP(pDestBuf, uiDestPitchBYTES, b->pSaveArea, b->sWidth * 2, b->sLeft, b->sTop, 0, 0, b->sWidth, b->sHeight);
				AddBaseDirtyRect(b->sLeft, b->sTop, b->sRight, b->sBottom);
			}
			else if (b->pZSaveArea != NULL)
			{
				Blt16BPPTo16BPP(gpZBuffer, gZBufferPitch, b->pZSaveArea, b->sWidth * sizeof(*b->pZSaveArea), b->sLeft, b->sTop, 0, 0, b->sWidth, b->sHeight);
			}
			else
			{
				Blt16BPPTo16BPP(pDestBuf, uiDestPitchBYTES, pSrcBuf, uiSrcPitchBYTES, b->sLeft, b->sTop, b->sLeft, b->sTop, b->sWidth, b->sHeight);
				AddBaseDirtyRect(b->sLeft, b->sTop, b->sRight, b->sBottom);
			}
		}
	}

	EmptyBackgroundRects();
}


void EmptyBackgroundRects(void)
{
	for (UINT32 i = 0; i < guiNumBackSaves; ++i)
	{
		BACKGROUND_SAVE* const b = gBackSaves[i];
		if (b->fFilled)
		{
			b->fFilled = FALSE;

			if (!b->fAllocated && b->fFreeMemory)
			{
				if (b->pSaveArea  != NULL) delete[] b->pSaveArea;
				if (b->pZSaveArea != NULL) delete[] b->pZSaveArea;

				b->fAllocated  = FALSE;
				b->fFreeMemory = FALSE;
				b->fFilled     = FALSE;
				b->pSaveArea   = NULL;
			}
		}

		if (b->uiFlags & BGND_FLAG_SINGLE || b->fPendingDelete)
		{
			if (b->fFreeMemory)
			{
				if (b->pSaveArea != NULL)  delete[] b->pSaveArea;
				if (b->pZSaveArea != NULL) delete[] b->pZSaveArea;
			}

			b->fAllocated     = FALSE;
			b->fFreeMemory    = FALSE;
			b->fFilled        = FALSE;
			b->pSaveArea      = NULL;
			b->fPendingDelete = FALSE;
		}
	}
}


void SaveBackgroundRects(void)
{
	SGPVSurface::Lock l(FRAME_BUFFER);
	UINT16* const pSrcBuf          = l.Buffer<UINT16>();
	UINT32  const uiDestPitchBYTES = l.Pitch();

	for (UINT32 i = 0; i < guiNumBackSaves; ++i)
	{
		BACKGROUND_SAVE* const b = gBackSaves[i];
		if (!b->fAllocated || b->fDisabled) continue;

		if (b->pSaveArea != NULL)
		{
			Blt16BPPTo16BPP(b->pSaveArea, b->sWidth * 2, pSrcBuf, uiDestPitchBYTES, 0, 0, b->sLeft, b->sTop, b->sWidth, b->sHeight);
		}
		else if (b->pZSaveArea != NULL)
		{
			Blt16BPPTo16BPP(b->pZSaveArea, b->sWidth * sizeof(*b->pZSaveArea), gpZBuffer, gZBufferPitch, 0, 0, b->sLeft, b->sTop, b->sWidth, b->sHeight);
		}
		else
		{
			AddBaseDirtyRect(b->sLeft, b->sTop, b->sRight, b->sBottom);
		}

		b->fFilled = TRUE;
	}
}


void FreeBackgroundRect(BACKGROUND_SAVE* const b)
{
	if (b == NULL) return;

	b->fAllocated = FALSE;
}


void FreeBackgroundRectPending(BACKGROUND_SAVE* const b)
{
	if(b)
	{
		b->fPendingDelete = TRUE;
	}
}


static void FreeBackgroundRectNow(BACKGROUND_SAVE* const b)
{
	if (b->fFreeMemory)
	{
		if (b->pSaveArea)  delete[] b->pSaveArea;
		if (b->pZSaveArea) delete[] b->pZSaveArea;
	}

	b->fAllocated  = FALSE;
	b->fFreeMemory = FALSE;
	b->fFilled     = FALSE;
	b->pSaveArea   = NULL;
}


void FreeBackgroundRectType(BackgroundFlags const uiFlags)
{
	for (UINT32 i = 0; i < guiNumBackSaves; ++i)
	{
		BACKGROUND_SAVE* const b = gBackSaves[i];
		if (b->uiFlags & uiFlags) FreeBackgroundRectNow(b);
	}
}


void InitializeBackgroundRects(void)
{
	guiNumBackSaves = 0;
	gDirtyClipRect.set(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}


void InvalidateBackgroundRects(void)
{
	for (UINT32 i = 0; i < guiNumBackSaves; ++i)
	{
		gBackSaves[i]->fFilled = FALSE;
	}
}


void ShutdownBackgroundRects(void)
{
	for (auto backgroundSave : gBackSaves)
	{
		if (backgroundSave->fAllocated) FreeBackgroundRectNow(backgroundSave);
		delete backgroundSave;
	}
	gBackSaves.clear();
	guiNumBackSaves = 0;
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


/* Dirties a single-frame rect exactly the size needed to save the background
	* for a given call to gprintf. Note that this must be called before the
	* backgrounds are saved, and before the actual call to gprintf that writes to
	* the video buffer. */
static void GDirty(INT16 x, INT16 y, const ST::utf32_buffer& codepoints)
{
	UINT16 const length = StringPixLength(codepoints, FontDefault);
	if (length > 0)
	{
		UINT16 const height = GetFontHeight(FontDefault);
		RegisterBackgroundRectSingleFilled(x, y, length, height);
	}
}


void GDirtyPrint(INT16 x, INT16 y, const ST::utf32_buffer& codepoints)
{
	GDirty(x, y, codepoints);
	MPrint(x, y, codepoints);
}


void GPrintInvalidate(INT16 x, INT16 y, const ST::utf32_buffer& codepoints)
{
	MPrint(x, y, codepoints);

	UINT16 const length = StringPixLength(codepoints, FontDefault);
	if (length > 0)
	{
		UINT16 const height = GetFontHeight(FontDefault);
		InvalidateRegionEx(x, y, x + length, y + height);
	}
}


VIDEO_OVERLAY* RegisterVideoOverlay(OVERLAY_CALLBACK const callback, INT16 const x, INT16 const y, INT16 const w, INT16 const h)
try
{
	BACKGROUND_SAVE* const bgs = RegisterBackgroundRect(BGND_FLAG_PERMANENT, x, y, w, h);
	if (!bgs) return 0;

	VIDEO_OVERLAY* const v    = new VIDEO_OVERLAY{};
	VIDEO_OVERLAY* const head = gVideoOverlays;
	v->prev        = 0;
	v->next        = head;
	v->fAllocated  = 2;
	v->background  = bgs;
	v->sX          = x;
	v->sY          = y;
	v->uiDestBuff  = FRAME_BUFFER;
	v->BltCallback = callback;

	if (head) head->prev = v;
	gVideoOverlays = v;
	return v;
}
catch (...) { return 0; }


VIDEO_OVERLAY* RegisterVideoOverlay(OVERLAY_CALLBACK callback, INT16 x, INT16 y, SGPFont font, UINT8 foreground, UINT8 background, const ST::utf32_buffer& codepoints)
{
	INT16          const w = StringPixLength(codepoints, font);
	INT16          const h = GetFontHeight(font);
	VIDEO_OVERLAY* const v = RegisterVideoOverlay(callback, x, y, w, h);
	if (v)
	{
		v->uiFontID   = font;
		v->ubFontFore = foreground;
		v->ubFontBack = background;
		v->codepoints = codepoints;
	}
	return v;
}


void RemoveVideoOverlay(VIDEO_OVERLAY* const v)
{
	if (!v) return;

	// Check if we are actively scrolling
	if (v->fActivelySaving)
	{
		v->fDeletionPending = TRUE;
	}
	else
	{
		//RestoreExternBackgroundRectGivenID(v->background);

		FreeBackgroundRect(v->background);

		if (v->pSaveArea != NULL) delete v->pSaveArea;
		v->pSaveArea = NULL;

		VIDEO_OVERLAY* const prev = v->prev;
		VIDEO_OVERLAY* const next = v->next;
		*(prev ? &prev->next : &gVideoOverlays) = next;
		if (next) next->prev = prev;
		delete v;
	}
}


// FUnctions for entrie array of blitters
void ExecuteVideoOverlays(void)
{
	FOR_EACH_VIDEO_OVERLAY(v)
	{
		// If we are scrolling but haven't saved yet, don't!
		if (!v->fActivelySaving && g_scroll_inertia) continue;

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
	FOR_EACH_VIDEO_OVERLAY(v)
	{
		if (!v->fActivelySaving) continue;

		SGPVSurface* const old_dst = v->uiDestBuff;
		v->uiDestBuff = buffer;
		v->BltCallback(v);
		v->uiDestBuff = old_dst;
	}
}


static void AllocateVideoOverlayArea(VIDEO_OVERLAY* const v)
{
	Assert(!v->fDisabled);

	// Get buffer size
	const BACKGROUND_SAVE* const bgs      = v->background;
	UINT32                 const buf_size = (bgs->sRight - bgs->sLeft) * (bgs->sBottom - bgs->sTop);

	v->fActivelySaving = TRUE;
	v->pSaveArea       = new UINT16[buf_size]{};
}


void AllocateVideoOverlaysArea(void)
{
	FOR_EACH_VIDEO_OVERLAY(v)
	{
		AllocateVideoOverlayArea(v);
	}
}


void SaveVideoOverlaysArea(SGPVSurface* const src)
{
	SGPVSurface::Lock l(src);
	UINT16* const pSrcBuf         = l.Buffer<UINT16>();
	UINT32  const uiSrcPitchBYTES = l.Pitch();

	FOR_EACH_VIDEO_OVERLAY(v)
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
}


void DeleteVideoOverlaysArea(void)
{
	FOR_EACH_VIDEO_OVERLAY_SAFE(v)
	{
		if (v->pSaveArea != NULL) delete[] v->pSaveArea;
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

	SGPVSurface::Lock l(BACKBUFFER);
	UINT16* const pDestBuf         = l.Buffer<UINT16>();
	UINT32  const uiDestPitchBYTES = l.Pitch();

	FOR_EACH_VIDEO_OVERLAY_SAFE(v)
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
		const INT32 uiLeftSkip   = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
		const INT32 uiTopSkip    = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
		const INT32 uiRightSkip  = std::clamp(iTempX + (INT32)usWidth  - ClipX2, 0, (INT32)usWidth);
		const INT32 uiBottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

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
}


void BlitBufferToBuffer(SGPVSurface* const src, SGPVSurface* const dst, const UINT16 usSrcX, const UINT16 usSrcY, const UINT16 usWidth, const UINT16 usHeight)
{
	SGPBox const r = { usSrcX, usSrcY, usWidth, usHeight };
	BltVideoSurface(dst, src, usSrcX, usSrcY, &r);
}


void EnableVideoOverlay(const BOOLEAN fEnable, VIDEO_OVERLAY* const v)
{
	if (!v) return;
	v->fDisabled             = !fEnable;
	v->background->fDisabled = !fEnable;
}


void SetVideoOverlayText(VIDEO_OVERLAY* v, const ST::utf32_buffer& codepoints)
{
	if (!v) return;
	v->codepoints = codepoints;
}


void SetVideoOverlayPos(VIDEO_OVERLAY* const v, const INT16 X, const INT16 Y)
{
	if (!v) return;

	// If position has changed and there is text, adjust
	if (v->codepoints.size() > 0)
	{
		UINT16 uiStringLength = StringPixLength(v->codepoints, v->uiFontID);
		UINT16 uiStringHeight = GetFontHeight(v->uiFontID);

		// Delete old rect
		// Remove background
		FreeBackgroundRectPending(v->background);

		v->background = RegisterBackgroundRect(BGND_FLAG_PERMANENT, X, Y, uiStringLength, uiStringHeight);
		v->sX         = X;
		v->sY         = Y;
	}
}
