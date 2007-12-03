#include "Font.h"
#include "Local.h"
#include "WorldDef.h"
#include "RenderWorld.h"
#include "VSurface.h"
#include "Render_Dirty.h"
#include "SysUtil.h"
#include "WCheck.h"
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


BACKGROUND_SAVE	gBackSaves[BACKGROUND_BUFFERS];
UINT32 guiNumBackSaves=0;

static VIDEO_OVERLAY gVideoOverlays[VIDEO_OVERLAYS];
static UINT32        guiNumVideoOverlays = 0;


SGPRect gDirtyClipRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };


BOOLEAN		gfViewportDirty=FALSE;


void AddBaseDirtyRect( INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom )
{
	SGPRect aRect;

	if (iLeft < 0)            iLeft = 0;
	if (iLeft > SCREEN_WIDTH) iLeft = SCREEN_WIDTH;


	if (iTop < 0)             iTop = 0;
	if (iTop > SCREEN_HEIGHT) iTop = SCREEN_HEIGHT;

	if (iRight < 0)            iRight = 0;
	if (iRight > SCREEN_WIDTH) iRight = SCREEN_WIDTH;


	if (iBottom < 0)             iBottom = 0;
	if (iBottom > SCREEN_HEIGHT) iBottom = SCREEN_HEIGHT;

	if (  ( iRight - iLeft ) == 0 || ( iBottom - iTop ) == 0 )
	{
		return;
	}


	if((iLeft==gsVIEWPORT_START_X) &&
			(iRight==gsVIEWPORT_END_X) &&
			(iTop==gsVIEWPORT_WINDOW_START_Y) &&
			(iBottom==gsVIEWPORT_WINDOW_END_Y))
	{
		gfViewportDirty=TRUE;
		return;
	}

	// Add to list
	aRect.iLeft		= iLeft;
	aRect.iTop		= iTop;
	aRect.iRight	= iRight;
	aRect.iBottom	= iBottom;

	InvalidateRegionEx(aRect.iLeft, aRect.iTop, aRect.iRight, aRect.iBottom);
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


static INT32 GetFreeBackgroundBuffer(void)
{
UINT32 uiCount;

	for(uiCount=0; uiCount < guiNumBackSaves; uiCount++)
	{
		if((gBackSaves[uiCount].fAllocated==FALSE) && (gBackSaves[uiCount].fFilled==FALSE))
			return((INT32)uiCount);
	}


	if(guiNumBackSaves < BACKGROUND_BUFFERS)
		return((INT32)guiNumBackSaves++);
#ifdef JA2BETAVERSION
	else
	{
		//else display an error message
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("ERROR! GetFreeBackgroundBuffer(): Trying to allocate more saves then there is room:  guiCurrentScreen = %d", guiCurrentScreen ) );
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


INT32 RegisterBackgroundRect(UINT32 uiFlags, INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom)
{
INT32 iBackIndex;
INT32  ClipX1, ClipY1, ClipX2, ClipY2;
INT32	uiLeftSkip, uiRightSkip, uiTopSkip, uiBottomSkip;
UINT32 usHeight, usWidth;
INT32	 iTempX, iTempY;


	// Don't register if we are rendering and we are below the viewport
	//if ( sTop >= gsVIEWPORT_WINDOW_END_Y )
	//{
	//	return NO_BGND_RECT;
	//}

	ClipX1= gDirtyClipRect.iLeft;
	ClipY1= gDirtyClipRect.iTop;
	ClipX2= gDirtyClipRect.iRight;
	ClipY2= gDirtyClipRect.iBottom;

	usHeight = sBottom - sTop;
	usWidth  = sRight -  sLeft;

	//if((sClipLeft >= sClipRight) || (sClipTop >= sClipBottom))
	//	return NO_BGND_RECT;
	iTempX = sLeft;
	iTempY = sTop;

	// Clip to rect
	uiLeftSkip=__min( ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	uiRightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	uiTopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	uiBottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// check if whole thing is clipped
	if((uiLeftSkip >=(INT32)usWidth) || (uiRightSkip >=(INT32)usWidth))
		return NO_BGND_RECT;

	// check if whole thing is clipped
	if((uiTopSkip >=(INT32)usHeight) || (uiBottomSkip >=(INT32)usHeight))
		return NO_BGND_RECT;

	// Set re-set values given based on clipping
	sLeft  = sLeft + (INT16)uiLeftSkip;
	sRight = sRight - (INT16)uiRightSkip;
	sTop   = sTop + (INT16)uiTopSkip;
	sBottom = sBottom - (INT16)uiBottomSkip;

	iBackIndex = GetFreeBackgroundBuffer();
	if (iBackIndex == NO_BGND_RECT) return NO_BGND_RECT;

	memset(&gBackSaves[iBackIndex], 0, sizeof(BACKGROUND_SAVE));

	gBackSaves[iBackIndex].fZBuffer=FALSE;

	UINT32 uiBufSize = (sRight - sLeft) * 2 * (sBottom - sTop);

	if (uiBufSize == 0)
		return NO_BGND_RECT;

	if (uiFlags & BGND_FLAG_SAVERECT)
	{
		if ((gBackSaves[iBackIndex].pSaveArea = MemAlloc(uiBufSize)) == NULL)
			return NO_BGND_RECT;
	}

	if (uiFlags & BGND_FLAG_SAVE_Z)
	{
		if ((gBackSaves[iBackIndex].pZSaveArea = MemAlloc(uiBufSize)) == NULL)
			return NO_BGND_RECT;
		gBackSaves[iBackIndex].fZBuffer = TRUE;
	}

	gBackSaves[iBackIndex].fFreeMemory = TRUE;

	gBackSaves[iBackIndex].fAllocated=TRUE;
	gBackSaves[iBackIndex].uiFlags=uiFlags;
	gBackSaves[iBackIndex].sLeft=sLeft;
	gBackSaves[iBackIndex].sTop=sTop;
	gBackSaves[iBackIndex].sRight=sRight;
	gBackSaves[iBackIndex].sBottom=sBottom;
	gBackSaves[iBackIndex].sWidth=(sRight-sLeft);
	gBackSaves[iBackIndex].sHeight=(sBottom-sTop);

	gBackSaves[iBackIndex].fFilled=FALSE;

	return(iBackIndex);
}

void SetBackgroundRectFilled( UINT32 uiBackgroundID )
{
	gBackSaves[uiBackgroundID].fFilled=TRUE;

	AddBaseDirtyRect(gBackSaves[uiBackgroundID].sLeft, gBackSaves[uiBackgroundID].sTop,
										gBackSaves[uiBackgroundID].sRight, gBackSaves[uiBackgroundID].sBottom);

}

BOOLEAN RestoreBackgroundRects(void)
{
UINT32 uiCount, uiDestPitchBYTES, uiSrcPitchBYTES;
UINT8	 *pDestBuf, *pSrcBuf;

	pDestBuf = LockVideoSurface(FRAME_BUFFER, &uiDestPitchBYTES);
	pSrcBuf = LockVideoSurface(guiSAVEBUFFER, &uiSrcPitchBYTES);

	for(uiCount=0; uiCount < guiNumBackSaves; uiCount++)
	{
		if(gBackSaves[uiCount].fFilled && ( !gBackSaves[uiCount].fDisabled) )
		{

			if ( gBackSaves[uiCount].uiFlags & BGND_FLAG_SAVERECT )
			{
				if ( gBackSaves[uiCount].pSaveArea != NULL )
				{
					Blt16BPPTo16BPP( (UINT16*)pDestBuf, uiDestPitchBYTES, (UINT16 *)gBackSaves[uiCount].pSaveArea, gBackSaves[uiCount].sWidth*2,
								gBackSaves[uiCount].sLeft , gBackSaves[uiCount].sTop,
								0, 0,
								gBackSaves[uiCount].sWidth, gBackSaves[uiCount].sHeight);

					AddBaseDirtyRect(gBackSaves[uiCount].sLeft, gBackSaves[uiCount].sTop,
													gBackSaves[uiCount].sRight, gBackSaves[uiCount].sBottom);

				}
			}
			else if ( gBackSaves[uiCount].uiFlags & BGND_FLAG_SAVE_Z )
			{
				if ( gBackSaves[uiCount].fZBuffer )
				{
					Blt16BPPTo16BPP( (UINT16*)gpZBuffer, uiDestPitchBYTES, (UINT16 *)gBackSaves[uiCount].pZSaveArea, gBackSaves[uiCount].sWidth*2,
								gBackSaves[uiCount].sLeft , gBackSaves[uiCount].sTop,
								0, 0,
								gBackSaves[uiCount].sWidth, gBackSaves[uiCount].sHeight);
				}
			}
			else
			{
				Blt16BPPTo16BPP((UINT16 *)pDestBuf, uiDestPitchBYTES,
							(UINT16 *)pSrcBuf, uiSrcPitchBYTES,
							gBackSaves[uiCount].sLeft , gBackSaves[uiCount].sTop,
							gBackSaves[uiCount].sLeft , gBackSaves[uiCount].sTop,
							gBackSaves[uiCount].sWidth, gBackSaves[uiCount].sHeight);

				AddBaseDirtyRect(gBackSaves[uiCount].sLeft, gBackSaves[uiCount].sTop,
													gBackSaves[uiCount].sRight, gBackSaves[uiCount].sBottom);
			}
		}

	}

	UnLockVideoSurface(FRAME_BUFFER);
	UnLockVideoSurface(guiSAVEBUFFER);

	EmptyBackgroundRects( );

	return(TRUE);
}


BOOLEAN EmptyBackgroundRects(void)
{
	UINT32 uiCount;

	for(uiCount=0; uiCount < guiNumBackSaves; uiCount++)
	{
		if(gBackSaves[uiCount].fFilled)
		{
			gBackSaves[uiCount].fFilled=FALSE;

			if(!(gBackSaves[uiCount].fAllocated) && (gBackSaves[uiCount].fFreeMemory==TRUE))
			{
				if ( gBackSaves[uiCount].uiFlags & BGND_FLAG_SAVERECT )
				{
					if ( gBackSaves[uiCount].pSaveArea != NULL )
					{
						MemFree(gBackSaves[uiCount].pSaveArea);
					}
				}
				if(gBackSaves[uiCount].fZBuffer)
					MemFree(gBackSaves[uiCount].pZSaveArea);

				gBackSaves[uiCount].fZBuffer=FALSE;
				gBackSaves[uiCount].fAllocated=FALSE;
				gBackSaves[uiCount].fFreeMemory=FALSE;
				gBackSaves[uiCount].fFilled=FALSE;
				gBackSaves[uiCount].pSaveArea=NULL;

				RecountBackgrounds();
			}
		}

		if(gBackSaves[uiCount].uiFlags&BGND_FLAG_SINGLE || gBackSaves[uiCount].fPendingDelete )
		{
			if(gBackSaves[uiCount].fFreeMemory==TRUE)
			{
				if ( gBackSaves[uiCount].uiFlags & BGND_FLAG_SAVERECT )
				{
					if ( gBackSaves[uiCount].pSaveArea != NULL )
					{
						MemFree(gBackSaves[uiCount].pSaveArea);
					}
				}

				if(gBackSaves[uiCount].fZBuffer)
					MemFree(gBackSaves[uiCount].pZSaveArea);
			}

			gBackSaves[uiCount].fZBuffer=FALSE;
			gBackSaves[uiCount].fAllocated=FALSE;
			gBackSaves[uiCount].fFreeMemory=FALSE;
			gBackSaves[uiCount].fFilled=FALSE;
			gBackSaves[uiCount].pSaveArea=NULL;
			gBackSaves[uiCount].fPendingDelete = FALSE;

			RecountBackgrounds();
		}
	}

	return(TRUE);
}


BOOLEAN SaveBackgroundRects(void)
{
UINT32 uiCount, uiDestPitchBYTES, uiSrcPitchBYTES;
UINT8	 *pDestBuf, *pSrcBuf;

	pSrcBuf = LockVideoSurface(FRAME_BUFFER, &uiDestPitchBYTES);
	pDestBuf = LockVideoSurface(guiSAVEBUFFER, &uiSrcPitchBYTES);

	for(uiCount=0; uiCount < guiNumBackSaves; uiCount++)
	{
		if(gBackSaves[uiCount].fAllocated && ( !gBackSaves[uiCount].fDisabled) )
		{
				if ( gBackSaves[uiCount].uiFlags & BGND_FLAG_SAVERECT )
				{
					if ( gBackSaves[uiCount].pSaveArea != NULL )
					{
						Blt16BPPTo16BPP((UINT16 *)gBackSaves[uiCount].pSaveArea, gBackSaves[uiCount].sWidth*2,
									(UINT16 *)pSrcBuf, uiDestPitchBYTES,
									0, 0,
									gBackSaves[uiCount].sLeft , gBackSaves[uiCount].sTop,
									gBackSaves[uiCount].sWidth, gBackSaves[uiCount].sHeight);

					}

				}
				else if(gBackSaves[uiCount].fZBuffer)
				{
					Blt16BPPTo16BPP(gBackSaves[uiCount].pZSaveArea, gBackSaves[uiCount].sWidth*2,
							(UINT16 *)gpZBuffer, uiDestPitchBYTES,
							0, 0,
							gBackSaves[uiCount].sLeft , gBackSaves[uiCount].sTop,
							gBackSaves[uiCount].sWidth, gBackSaves[uiCount].sHeight);
				}
				else
				{
					AddBaseDirtyRect(gBackSaves[uiCount].sLeft, gBackSaves[uiCount].sTop,
											gBackSaves[uiCount].sRight, gBackSaves[uiCount].sBottom);
				}

				gBackSaves[uiCount].fFilled=TRUE;


		}
	}

	UnLockVideoSurface(FRAME_BUFFER);
	UnLockVideoSurface(guiSAVEBUFFER);

	return(TRUE);
}


BOOLEAN FreeBackgroundRect(INT32 iIndex)
{
	if ( iIndex != -1 )
	{
		gBackSaves[iIndex].fAllocated=FALSE;

		RecountBackgrounds();
	}

	return(TRUE);
}

BOOLEAN FreeBackgroundRectPending(INT32 iIndex)
{
	gBackSaves[iIndex].fPendingDelete = TRUE;

	return(TRUE);
}


static BOOLEAN FreeBackgroundRectNow(INT32 uiCount)
{
	if(gBackSaves[uiCount].fFreeMemory==TRUE)
	{
		//MemFree(gBackSaves[uiCount].pSaveArea);
		if(gBackSaves[uiCount].fZBuffer)
			MemFree(gBackSaves[uiCount].pZSaveArea);
	}

	gBackSaves[uiCount].fZBuffer=FALSE;
	gBackSaves[uiCount].fAllocated=FALSE;
	gBackSaves[uiCount].fFreeMemory=FALSE;
	gBackSaves[uiCount].fFilled=FALSE;
	gBackSaves[uiCount].pSaveArea=NULL;

	RecountBackgrounds();
	return(TRUE);
}

BOOLEAN FreeBackgroundRectType(UINT32 uiFlags)
{
UINT32 uiCount;

	for(uiCount=0; uiCount < guiNumBackSaves; uiCount++)
	{
		if(gBackSaves[uiCount].uiFlags&uiFlags)
		{
			if(gBackSaves[uiCount].fFreeMemory==TRUE)
			{
				//MemFree(gBackSaves[uiCount].pSaveArea);
				if(gBackSaves[uiCount].fZBuffer)
					MemFree(gBackSaves[uiCount].pZSaveArea);
			}

			gBackSaves[uiCount].fZBuffer=FALSE;
			gBackSaves[uiCount].fAllocated=FALSE;
			gBackSaves[uiCount].fFreeMemory=FALSE;
			gBackSaves[uiCount].fFilled=FALSE;
			gBackSaves[uiCount].pSaveArea=NULL;
		}
	}

	RecountBackgrounds();

	return(TRUE);
}


BOOLEAN InitializeBackgroundRects(void)
{
	guiNumBackSaves=0;
	return( TRUE );
}

BOOLEAN InvalidateBackgroundRects(void)
{
UINT32 uiCount;

	for(uiCount=0; uiCount < guiNumBackSaves; uiCount++)
		gBackSaves[uiCount].fFilled=FALSE;

	return(TRUE);
}


BOOLEAN ShutdownBackgroundRects(void)
{
UINT32 uiCount;

	for(uiCount=0; uiCount < guiNumBackSaves; uiCount++)
	{
		if(gBackSaves[uiCount].fAllocated)
			FreeBackgroundRectNow((INT32)uiCount);
	}

	return(TRUE);
}


static void DisableBackgroundRect(INT32 iIndex, BOOLEAN fDisabled)
{
	gBackSaves[iIndex].fDisabled = fDisabled;
}

BOOLEAN UpdateSaveBuffer(void)
{
	// Update saved buffer - do for the viewport size ony!
	BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, 0, gsVIEWPORT_WINDOW_START_Y, SCREEN_WIDTH, gsVIEWPORT_WINDOW_END_Y - gsVIEWPORT_WINDOW_START_Y);
	return TRUE;
}


BOOLEAN RestoreExternBackgroundRect( INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight )
{
	Assert(0 <= sLeft && sLeft + sWidth <= SCREEN_WIDTH && 0 <= sTop && sTop + sHeight <= SCREEN_HEIGHT);

	BlitBufferToBuffer(guiSAVEBUFFER, FRAME_BUFFER, sLeft, sTop, sWidth, sHeight);

	// Add rect to frame buffer queue
	InvalidateRegionEx(sLeft, sTop, sLeft + sWidth, sTop + sHeight);

	return(TRUE);
}


BOOLEAN RestoreExternBackgroundRectGivenID(const INT32 iBack)
{
	const BACKGROUND_SAVE* const b = &gBackSaves[iBack];
	if (!b->fAllocated) return FALSE;
	return RestoreExternBackgroundRect(b->sLeft, b->sTop, b->sWidth, b->sHeight);
}


//*****************************************************************************
// gprintfdirty
//
//		Dirties a single-frame rect exactly the size needed to save the
// background for a given call to gprintf. Note that this must be called before
// the backgrounds are saved, and before the actual call to gprintf that writes
// to the video buffer.
//
//*****************************************************************************
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
		const INT32 iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, x, y, x + uiStringLength, y + uiStringHeight);
		if (iBack != NO_BGND_RECT) SetBackgroundRectFilled(iBack);
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


VIDEO_OVERLAY* RegisterVideoOverlay(UINT32 uiFlags, const VIDEO_OVERLAY_DESC* pTopmostDesc)
{
	UINT32 iBackIndex;
	UINT16 uiStringLength, uiStringHeight;

	if ( uiFlags & VOVERLAY_DIRTYBYTEXT )
	{
		uiStringLength=StringPixLength( pTopmostDesc->pzText, pTopmostDesc->uiFontID );
		uiStringHeight=GetFontHeight( pTopmostDesc->uiFontID );

		iBackIndex = RegisterBackgroundRect(BGND_FLAG_PERMANENT, pTopmostDesc->sLeft, pTopmostDesc->sTop, pTopmostDesc->sLeft + uiStringLength, pTopmostDesc->sTop + uiStringHeight);
	}
	else
	{
		// Register background
		iBackIndex = RegisterBackgroundRect(BGND_FLAG_PERMANENT, pTopmostDesc->sLeft, pTopmostDesc->sTop, pTopmostDesc->sRight, pTopmostDesc->sBottom);
	}

	if (iBackIndex == NO_BGND_RECT) return NULL;

	// Get next free topmost blitter index
	VIDEO_OVERLAY* const v = GetFreeVideoOverlay();
	if (v == NULL) return NULL;

	// Init new blitter
	memset(v, 0, sizeof(*v));
	v->uiFlags      = uiFlags;
	v->fAllocated   = 2;
	v->uiBackground = iBackIndex;
	v->pBackground  = &gBackSaves[iBackIndex];
	v->uiFontID     = pTopmostDesc->uiFontID;
	v->sX           = pTopmostDesc->sLeft;
	v->sY           = pTopmostDesc->sTop;
	v->ubFontBack   = pTopmostDesc->ubFontBack;
	v->ubFontFore   = pTopmostDesc->ubFontFore;
	v->uiDestBuff   = FRAME_BUFFER;
	v->BltCallback  = pTopmostDesc->BltCallback;
	wcslcpy(v->zText, pTopmostDesc->pzText, lengthof(v->zText));

	// Set disabled flag to true
	if (uiFlags & VOVERLAY_STARTDISABLED)
	{
		v->fDisabled = TRUE;
		DisableBackgroundRect(v->uiBackground, TRUE);
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
		//RestoreExternBackgroundRectGivenID(v->uiBackground);

		FreeBackgroundRect(v->uiBackground);

		if (v->pSaveArea != NULL) MemFree(v->pSaveArea);
		v->pSaveArea = NULL;

		v->fAllocated = FALSE;
	}
}


// FUnctions for entrie array of blitters
void ExecuteVideoOverlays( )
{
	UINT32 uiCount;

	for(uiCount=0; uiCount < guiNumVideoOverlays; uiCount++)
	{
		if( gVideoOverlays[uiCount].fAllocated )
		{
				if ( !gVideoOverlays[uiCount].fDisabled )
				{
					// If we are scrolling but havn't saved yet, don't!
					if ( !gVideoOverlays[uiCount].fActivelySaving && gfScrollInertia > 0 )
					{
						continue;
					}

					// ATE: Wait a frame before executing!
					if ( gVideoOverlays[uiCount].fAllocated == 1 )
					{
						// Call Blit Function
						(*(gVideoOverlays[uiCount].BltCallback ) ) ( &(gVideoOverlays[uiCount]) );
					}
					else if ( gVideoOverlays[uiCount].fAllocated == 2 )
					{
						 gVideoOverlays[uiCount].fAllocated = 1;
					}
				}

				// Remove if pending
				//if ( gVideoOverlays[uiCount].fDeletionPending )
				//{
				//	RemoveVideoOverlay( uiCount );
				//}
		}
	}

}


void ExecuteVideoOverlaysToAlternateBuffer( UINT32 uiNewDestBuffer )
{
	UINT32  uiCount;
	UINT32	uiOldDestBuffer;

	for(uiCount=0; uiCount < guiNumVideoOverlays; uiCount++)
	{
		if( gVideoOverlays[uiCount].fAllocated && !gVideoOverlays[uiCount].fDisabled )
		{
			if ( gVideoOverlays[uiCount].fActivelySaving )
			{
				uiOldDestBuffer =  gVideoOverlays[uiCount].uiDestBuff;

				gVideoOverlays[uiCount].uiDestBuff = uiNewDestBuffer;

				// Call Blit Function
			 (*(gVideoOverlays[uiCount].BltCallback ) ) ( &(gVideoOverlays[uiCount]) );

				gVideoOverlays[uiCount].uiDestBuff = uiOldDestBuffer;
			}
		}
	}

}

void AllocateVideoOverlaysArea( )
{
	UINT32 uiCount;
	UINT32 uiBufSize;
	UINT32 iBackIndex;

	for(uiCount=0; uiCount < guiNumVideoOverlays; uiCount++)
	{
		if( gVideoOverlays[uiCount].fAllocated && !gVideoOverlays[uiCount].fDisabled )
		{
			iBackIndex = gVideoOverlays[uiCount].uiBackground;

			// Get buffer size
			uiBufSize=(( gBackSaves[ iBackIndex ].sRight-gBackSaves[ iBackIndex ].sLeft)*2)*(gBackSaves[ iBackIndex ].sBottom-gBackSaves[ iBackIndex ].sTop );

			gVideoOverlays[uiCount].fActivelySaving = TRUE;

			//DebugMsg( TOPIC_JA2, DBG_LEVEL_0, String( "Setting Overlay Actively saving %d %ls", uiCount, gVideoOverlays[ uiCount ].zText ) );

			// Allocate
			if( ( gVideoOverlays[ uiCount ].pSaveArea = MemAlloc( uiBufSize ) ) == NULL )
			{
				continue;
			}
		}
	}

}


static void AllocateVideoOverlayArea(UINT32 uiCount)
{
	UINT32 uiBufSize;
	UINT32 iBackIndex;

	if( gVideoOverlays[uiCount].fAllocated && !gVideoOverlays[uiCount].fDisabled )
	{
		iBackIndex = gVideoOverlays[uiCount].uiBackground;

		// Get buffer size
		uiBufSize=(( gBackSaves[ iBackIndex ].sRight-gBackSaves[ iBackIndex ].sLeft)*2)*(gBackSaves[ iBackIndex ].sBottom-gBackSaves[ iBackIndex ].sTop );

		gVideoOverlays[uiCount].fActivelySaving = TRUE;

		//DebugMsg( TOPIC_JA2, DBG_LEVEL_0, String( "Setting Overlay Actively saving %d %ls", uiCount, gVideoOverlays[ uiCount ].zText ) );

		// Allocate
		if( ( gVideoOverlays[ uiCount ].pSaveArea = MemAlloc( uiBufSize ) ) == NULL )
		{
		}
	}

}


void SaveVideoOverlaysArea( UINT32 uiSrcBuffer )
{
	UINT32 uiCount;
	UINT32 iBackIndex;
	UINT32 uiSrcPitchBYTES;
	UINT8	 *pSrcBuf;

	pSrcBuf = LockVideoSurface( uiSrcBuffer, &uiSrcPitchBYTES );

	for(uiCount=0; uiCount < guiNumVideoOverlays; uiCount++)
	{
		if( gVideoOverlays[uiCount].fAllocated && !gVideoOverlays[uiCount].fDisabled )
		{
			// OK, if our saved area is null, allocate it here!
			if ( gVideoOverlays[uiCount].pSaveArea == NULL )
			{
				AllocateVideoOverlayArea( uiCount );
			}

			if ( gVideoOverlays[uiCount].pSaveArea != NULL )
			{
					iBackIndex = gVideoOverlays[uiCount].uiBackground;

					// Save data from frame buffer!
					Blt16BPPTo16BPP((UINT16 *)gVideoOverlays[uiCount].pSaveArea, gBackSaves[ iBackIndex ].sWidth*2,
								(UINT16 *)pSrcBuf, uiSrcPitchBYTES,
								0, 0,
								gBackSaves[ iBackIndex ].sLeft , gBackSaves[ iBackIndex ].sTop,
								gBackSaves[ iBackIndex ].sWidth, gBackSaves[ iBackIndex ].sHeight );
			}
		}
	}

	UnLockVideoSurface( uiSrcBuffer );
}


void DeleteVideoOverlaysArea( )
{
	UINT32 uiCount;

	for(uiCount=0; uiCount < guiNumVideoOverlays; uiCount++)
	{
		VIDEO_OVERLAY* const v = &gVideoOverlays[uiCount];
		if (v->fAllocated && !v->fDisabled)
		{
			if (v->pSaveArea != NULL) MemFree(v->pSaveArea);
			v->pSaveArea       = NULL;
			v->fActivelySaving = FALSE;
			if (v->fDeletionPending) RemoveVideoOverlay(v);
		}
	}
}


BOOLEAN RestoreShiftedVideoOverlays( INT16 sShiftX, INT16 sShiftY )
{
	UINT32 uiCount, uiDestPitchBYTES;
	UINT8	 *pDestBuf;

	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	INT32	uiLeftSkip, uiRightSkip, uiTopSkip, uiBottomSkip;
	UINT32 usHeight, usWidth;
	INT32	 iTempX, iTempY;
	INT16		sLeft, sTop, sRight, sBottom;

	ClipX1 = 0;
	ClipY1 = gsVIEWPORT_WINDOW_START_Y;
	ClipX2 = SCREEN_WIDTH;
	ClipY2 = gsVIEWPORT_WINDOW_END_Y - 1;


	pDestBuf = LockVideoSurface( BACKBUFFER, &uiDestPitchBYTES);

	for(uiCount=0; uiCount <  guiNumVideoOverlays; uiCount++)
	{
		VIDEO_OVERLAY* const v = &gVideoOverlays[uiCount];
		if (v->fAllocated && !v->fDisabled)
		{
				if (v->pSaveArea != NULL)
				{
					// Get restore background values
					const UINT32 iBackIndex = v->uiBackground;
					sLeft			= gBackSaves[ iBackIndex ].sLeft;
					sTop		  = gBackSaves[ iBackIndex ].sTop;
					sRight		= gBackSaves[ iBackIndex ].sRight;
					sBottom		= gBackSaves[ iBackIndex ].sBottom;
					usHeight  = gBackSaves[ iBackIndex ].sHeight;
					usWidth   = gBackSaves[ iBackIndex ].sWidth;


					// Clip!!
					iTempX = sLeft - sShiftX;
					iTempY = sTop  - sShiftY;

					// Clip to rect
					uiLeftSkip=__min( ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
					uiRightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
					uiTopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
					uiBottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

					// check if whole thing is clipped
					if((uiLeftSkip >=(INT32)usWidth) || (uiRightSkip >=(INT32)usWidth))
						continue;

					// check if whole thing is clipped
					if((uiTopSkip >=(INT32)usHeight) || (uiBottomSkip >=(INT32)usHeight))
						continue;

					// Set re-set values given based on clipping
					sLeft    = iTempX + (INT16)uiLeftSkip;
					sTop     = iTempY + (INT16)uiTopSkip;
					sRight   = sRight  - sShiftX - (INT16)uiRightSkip;
					sBottom  = sBottom - sShiftY - (INT16)uiBottomSkip;

					usHeight = sBottom - sTop;
					usWidth  = sRight -  sLeft;

					Blt16BPPTo16BPP((UINT16*)pDestBuf, uiDestPitchBYTES, (UINT16*)v->pSaveArea, gBackSaves[iBackIndex].sWidth * 2, sLeft, sTop, uiLeftSkip, uiTopSkip, usWidth, usHeight);

					// Once done, check for pending deletion
					if (v->fDeletionPending) RemoveVideoOverlay(v);
				}
		}
	}

	UnLockVideoSurface( BACKBUFFER );

	return(TRUE);
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


BOOLEAN BlitBufferToBuffer(UINT32 uiSrcBuffer, UINT32 uiDestBuffer, UINT16 usSrcX, UINT16 usSrcY, UINT16 usWidth, UINT16 usHeight)
{
	const SGPRect r = { usSrcX, usSrcY, usSrcX + usWidth, usSrcY + usHeight };
	return BltVideoSurface(uiDestBuffer, uiSrcBuffer, usSrcX, usSrcY, &r);
}


void EnableVideoOverlay(const BOOLEAN fEnable, VIDEO_OVERLAY* const v)
{
	if (v == NULL) return;
	if (!v->fAllocated) return;

	v->fDisabled = !fEnable;
	DisableBackgroundRect(v->uiBackground, !fEnable);
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
		FreeBackgroundRectPending(v->uiBackground);

		v->uiBackground = RegisterBackgroundRect(BGND_FLAG_PERMANENT, X, Y, X + uiStringLength, Y + uiStringHeight);
		v->sX           = X;
		v->sY           = Y;
	}
}
