#include "Font.h"
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
#include "SGP.h"

#ifdef JA2BETAVERSION
#	include "JAScreens.h"
#endif


#define		DIRTY_QUEUES				200
#define		BACKGROUND_BUFFERS	500
#define		VIDEO_OVERLAYS			100


BACKGROUND_SAVE	gBackSaves[BACKGROUND_BUFFERS];
UINT32 guiNumBackSaves=0;

VIDEO_OVERLAY	gVideoOverlays[ VIDEO_OVERLAYS ];
UINT32 guiNumVideoOverlays=0;


SGPRect		gDirtyClipRect = { 0, 0, 640, 480 };


BOOLEAN		gfViewportDirty=FALSE;


void AddBaseDirtyRect( INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom )
{
	SGPRect aRect;

	if ( iLeft < 0 )
	{
		iLeft = 0;
	}
	if ( iLeft > 640 )
	{
		iLeft = 640;
	}


	if ( iTop < 0 )
	{
		iTop = 0;
	}
	if ( iTop > 480 )
	{
		iTop = 480;
	}


	if ( iRight < 0 )
	{
		iRight = 0;
	}
	if ( iRight > 640 )
	{
		iRight = 640;
	}


	if ( iBottom < 0 )
	{
		iBottom = 0;
	}
	if ( iBottom > 480 )
	{
		iBottom = 480;
	}

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

	return(-1);
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




INT32 RegisterBackgroundRect(UINT32 uiFlags, INT16 *pSaveArea, INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom)
{
UINT32 uiBufSize;
INT32 iBackIndex;
INT32  ClipX1, ClipY1, ClipX2, ClipY2;
INT32	uiLeftSkip, uiRightSkip, uiTopSkip, uiBottomSkip;
UINT32 usHeight, usWidth;
INT32	 iTempX, iTempY;


	// Don't register if we are rendering and we are below the viewport
	//if ( sTop >= gsVIEWPORT_WINDOW_END_Y )
	//{
	//	return(-1 );
	//}

	ClipX1= gDirtyClipRect.iLeft;
	ClipY1= gDirtyClipRect.iTop;
	ClipX2= gDirtyClipRect.iRight;
	ClipY2= gDirtyClipRect.iBottom;

	usHeight = sBottom - sTop;
	usWidth  = sRight -  sLeft;

	//if((sClipLeft >= sClipRight) || (sClipTop >= sClipBottom))
	//	return(-1);
	iTempX = sLeft;
	iTempY = sTop;

	// Clip to rect
	uiLeftSkip=__min( ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	uiRightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	uiTopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	uiBottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// check if whole thing is clipped
	if((uiLeftSkip >=(INT32)usWidth) || (uiRightSkip >=(INT32)usWidth))
		return(-1 );

	// check if whole thing is clipped
	if((uiTopSkip >=(INT32)usHeight) || (uiBottomSkip >=(INT32)usHeight))
		return(-1 );

	// Set re-set values given based on clipping
	sLeft  = sLeft + (INT16)uiLeftSkip;
	sRight = sRight - (INT16)uiRightSkip;
	sTop   = sTop + (INT16)uiTopSkip;
	sBottom = sBottom - (INT16)uiBottomSkip;

	if((iBackIndex=GetFreeBackgroundBuffer())==(-1))
		return(-1);

	memset(&gBackSaves[iBackIndex], 0, sizeof(BACKGROUND_SAVE));

	gBackSaves[iBackIndex].fZBuffer=FALSE;

	if(pSaveArea==NULL)
	{
		uiBufSize=((sRight-sLeft)*2)*(sBottom-sTop);

		if(uiBufSize==0)
			return(-1);

		if ( uiFlags & BGND_FLAG_SAVERECT )
		{
			if((gBackSaves[iBackIndex].pSaveArea=MemAlloc(uiBufSize))==NULL)
				return(-1);
		}


		if(uiFlags&BGND_FLAG_SAVE_Z)
		{
			if((gBackSaves[iBackIndex].pZSaveArea=MemAlloc(uiBufSize))==NULL)
				return(-1);
			gBackSaves[iBackIndex].fZBuffer=TRUE;
		}

		gBackSaves[iBackIndex].fFreeMemory=TRUE;
	}
	//else
	//	gBackSaves[iBackIndex].pSaveArea=pSaveArea;

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
	UINT16 usWidth;
	UINT16 usHeight;
	GetCurrentVideoSettings(&usWidth, &usHeight);
	BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, 0, gsVIEWPORT_WINDOW_START_Y, usWidth, gsVIEWPORT_WINDOW_END_Y - gsVIEWPORT_WINDOW_START_Y);
	return TRUE;
}


BOOLEAN RestoreExternBackgroundRect( INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight )
{
	Assert( ( sLeft >= 0 ) && ( sTop >= 0 ) && ( sLeft + sWidth <= 640 ) && ( sTop + sHeight <= 480 ) );

	BlitBufferToBuffer(guiSAVEBUFFER, FRAME_BUFFER, sLeft, sTop, sWidth, sHeight);

	// Add rect to frame buffer queue
	InvalidateRegionEx(sLeft, sTop, sLeft + sWidth, sTop + sHeight);

	return(TRUE);
}


BOOLEAN RestoreExternBackgroundRectGivenID( INT32 iBack )
{
	INT16 sLeft, sTop, sWidth, sHeight;

	if( !gBackSaves[iBack].fAllocated )
	{
		return( FALSE );
	}

	sLeft		= gBackSaves[iBack].sLeft;
	sTop		= gBackSaves[iBack].sTop;
	sWidth	= gBackSaves[iBack].sWidth;
	sHeight	= gBackSaves[iBack].sHeight;

	Assert( ( sLeft >= 0 ) && ( sTop >= 0 ) && ( sLeft + sWidth <= 640 ) && ( sTop + sHeight <= 480 ) );

	BlitBufferToBuffer(guiSAVEBUFFER, FRAME_BUFFER, sLeft, sTop, sWidth, sHeight);

	// Add rect to frame buffer queue
	InvalidateRegionEx(sLeft, sTop, sLeft + sWidth, sTop + sHeight);

	return(TRUE);
}


static BOOLEAN CopyExternBackgroundRect(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight)
{
	Assert( ( sLeft >= 0 ) && ( sTop >= 0 ) && ( sLeft + sWidth <= 640 ) && ( sTop + sHeight <= 480 ) );

	BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, sLeft, sTop, sWidth, sHeight);

	return(TRUE);
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
INT32 iBack;

	Assert(pFontString!=NULL);

	va_start(argptr, pFontString);       	// Set up variable argument pointer
	vswprintf(string, lengthof(string), pFontString, argptr);	// process gprintf string (get output str)
	va_end(argptr);

	uiStringLength = StringPixLength(string, FontDefault);
	uiStringHeight = GetFontHeight(FontDefault);

	if ( uiStringLength > 0 )
	{
		iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, x, y, (INT16)(x + uiStringLength), (INT16)(y + uiStringHeight));

		if ( iBack != -1 )
		{
			SetBackgroundRectFilled( iBack );
		}
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


// OVERLAY STUFF
static INT32 GetFreeVideoOverlay(void)
{
  UINT32 uiCount;

	for(uiCount=0; uiCount < guiNumVideoOverlays; uiCount++)
	{
		if((gVideoOverlays[uiCount].fAllocated==FALSE ) )
			return((INT32)uiCount);
	}

	if( guiNumVideoOverlays < BACKGROUND_BUFFERS )
		return((INT32)guiNumVideoOverlays++);

	return(-1);
}


static void RecountVideoOverlays(void)
{
  INT32 uiCount;

	for(uiCount=guiNumVideoOverlays-1; (uiCount >=0) ; uiCount--)
	{
		if((gVideoOverlays[uiCount].fAllocated) )
		{
			guiNumVideoOverlays=(UINT32)(uiCount+1);
			break;
		}
	}
}


INT32 RegisterVideoOverlay(UINT32 uiFlags, const VIDEO_OVERLAY_DESC* pTopmostDesc)
{
	UINT32 iBlitterIndex;
	UINT32 iBackIndex;
	UINT16 uiStringLength, uiStringHeight;

	if ( uiFlags & VOVERLAY_DIRTYBYTEXT )
	{
		uiStringLength=StringPixLength( pTopmostDesc->pzText, pTopmostDesc->uiFontID );
		uiStringHeight=GetFontHeight( pTopmostDesc->uiFontID );

		iBackIndex = RegisterBackgroundRect( BGND_FLAG_PERMANENT, NULL, pTopmostDesc->sLeft, pTopmostDesc->sTop, (INT16)(pTopmostDesc->sLeft + uiStringLength), (INT16)(pTopmostDesc->sTop + uiStringHeight) );
	}
	else
	{
		// Register background
		iBackIndex = RegisterBackgroundRect( BGND_FLAG_PERMANENT, NULL, pTopmostDesc->sLeft, pTopmostDesc->sTop, pTopmostDesc->sRight, pTopmostDesc->sBottom );
	}


	if ( iBackIndex == -1 )
	{
		return( -1 );
	}


	// Get next free topmost blitter index
	if( ( iBlitterIndex = GetFreeVideoOverlay())==(-1))
		return(-1);

	// Init new blitter
	VIDEO_OVERLAY* v = &gVideoOverlays[iBlitterIndex];
	memset(v, 0, sizeof(*v));
	v->uiFlags      = uiFlags;
	v->fAllocated   = 2;
	v->uiBackground = iBackIndex;
	v->pBackground  = &gBackSaves[iBackIndex];
	v->uiFontID     = pTopmostDesc->uiFontID;
	v->sX           = pTopmostDesc->sX;
	v->sY           = pTopmostDesc->sY;
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

	return iBlitterIndex;
}


static void SetVideoOverlayPendingDelete(INT32 iVideoOverlay)
{
	if ( iVideoOverlay != -1 )
	{
		gVideoOverlays[ iVideoOverlay ].fDeletionPending = TRUE;
	}
}


void RemoveVideoOverlay( INT32 iVideoOverlay )
{

	if ( iVideoOverlay != -1 && gVideoOverlays[ iVideoOverlay ].fAllocated )
	{
		// Check if we are actively scrolling
		if ( gVideoOverlays[ iVideoOverlay ].fActivelySaving )
		{

	//		DebugMsg( TOPIC_JA2, DBG_LEVEL_0, String( "Overlay Actively saving %d %ls", iVideoOverlay, gVideoOverlays[ iVideoOverlay ].zText ) );

			gVideoOverlays[ iVideoOverlay ].fDeletionPending = TRUE;
		}
		else
		{
			//RestoreExternBackgroundRectGivenID( gVideoOverlays[ iVideoOverlay ].uiBackground );

			// Remove background
			FreeBackgroundRect( gVideoOverlays[ iVideoOverlay ].uiBackground );

			//DebugMsg( TOPIC_JA2, DBG_LEVEL_0, String( "Delete Overlay %d %ls", iVideoOverlay, gVideoOverlays[ iVideoOverlay ].zText ) );


			// Remove save buffer if not done so
			if ( gVideoOverlays[ iVideoOverlay ].pSaveArea != NULL )
			{
				MemFree( gVideoOverlays[ iVideoOverlay ].pSaveArea );
			}
			gVideoOverlays[ iVideoOverlay ].pSaveArea = NULL;


			// Set as not allocated
			gVideoOverlays[ iVideoOverlay ].fAllocated   = FALSE;
		}
	}
}


BOOLEAN UpdateVideoOverlay(const VIDEO_OVERLAY_DESC* pTopmostDesc, UINT32 iBlitterIndex)
{
	if ( iBlitterIndex != -1 )
	{

		if ( !gVideoOverlays[ iBlitterIndex ].fAllocated )
		{
			return( FALSE );
		}

		UINT32 uiFlags = pTopmostDesc->uiFlags;

		if (uiFlags & VOVERLAY_DESC_TEXT)
		{
			wcscpy(gVideoOverlays[iBlitterIndex].zText, pTopmostDesc->pzText);
		}

		// If position has changed and flags are of type that use dirty rects, adjust
		if (uiFlags & VOVERLAY_DESC_POSITION)
		{
			if (gVideoOverlays[iBlitterIndex].uiFlags & VOVERLAY_DIRTYBYTEXT)
			{
				UINT16 uiStringLength = StringPixLength(gVideoOverlays[iBlitterIndex].zText, gVideoOverlays[iBlitterIndex].uiFontID);
				UINT16 uiStringHeight = GetFontHeight(gVideoOverlays[iBlitterIndex].uiFontID);

				// Delete old rect
				// Remove background
				FreeBackgroundRectPending(gVideoOverlays[iBlitterIndex].uiBackground);

				gVideoOverlays[iBlitterIndex].uiBackground = RegisterBackgroundRect(BGND_FLAG_PERMANENT, NULL, pTopmostDesc->sLeft, pTopmostDesc->sTop, pTopmostDesc->sLeft + uiStringLength, pTopmostDesc->sTop + uiStringHeight);
				gVideoOverlays[iBlitterIndex].sX           = pTopmostDesc->sX;
				gVideoOverlays[iBlitterIndex].sY           = pTopmostDesc->sY;
			}
		}
	}
	return( TRUE );
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


static void SaveVideoOverlayArea(UINT32 uiSrcBuffer, UINT32 uiCount)
{
	UINT32 iBackIndex;
	UINT32 uiSrcPitchBYTES;
	UINT8	 *pSrcBuf;

	pSrcBuf = LockVideoSurface( uiSrcBuffer, &uiSrcPitchBYTES );

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

	UnLockVideoSurface( uiSrcBuffer );

}


void DeleteVideoOverlaysArea( )
{
	UINT32 uiCount;

	for(uiCount=0; uiCount < guiNumVideoOverlays; uiCount++)
	{
		if( gVideoOverlays[uiCount].fAllocated && !gVideoOverlays[uiCount].fDisabled )
		{
			if ( gVideoOverlays[uiCount].pSaveArea != NULL )
			{
				MemFree( gVideoOverlays[uiCount].pSaveArea );
			}

			gVideoOverlays[uiCount].fActivelySaving = FALSE;

			gVideoOverlays[uiCount].pSaveArea = NULL;

			//DebugMsg( TOPIC_JA2, DBG_LEVEL_0, String( "Removing Overlay Actively saving %d %ls", uiCount, gVideoOverlays[ uiCount ].zText ) );

			// Remove if pending
			if ( gVideoOverlays[uiCount].fDeletionPending )
			{
				RemoveVideoOverlay( uiCount );
			}

		}
	}

}


BOOLEAN RestoreShiftedVideoOverlays( INT16 sShiftX, INT16 sShiftY )
{
	UINT32 uiCount, uiDestPitchBYTES;
	UINT8	 *pDestBuf;
	UINT32 iBackIndex;

	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	INT32	uiLeftSkip, uiRightSkip, uiTopSkip, uiBottomSkip;
	UINT32 usHeight, usWidth;
	INT32	 iTempX, iTempY;
	INT16		sLeft, sTop, sRight, sBottom;

	ClipX1= 0;
	ClipY1= gsVIEWPORT_WINDOW_START_Y;
	ClipX2= 640;
	ClipY2= gsVIEWPORT_WINDOW_END_Y - 1;


	pDestBuf = LockVideoSurface( BACKBUFFER, &uiDestPitchBYTES);

	for(uiCount=0; uiCount <  guiNumVideoOverlays; uiCount++)
	{
		if( gVideoOverlays[uiCount].fAllocated && !gVideoOverlays[uiCount].fDisabled )
		{
				iBackIndex = gVideoOverlays[uiCount].uiBackground;


				if ( gVideoOverlays[uiCount].pSaveArea != NULL )
				{
					// Get restore background values
					sLeft			= gBackSaves[ iBackIndex ].sLeft;
					sTop		  = gBackSaves[ iBackIndex ].sTop;
					sRight		= gBackSaves[ iBackIndex ].sRight;
					sBottom		= gBackSaves[ iBackIndex ].sBottom;
					usHeight  = gBackSaves[ iBackIndex ].sHeight;
					usWidth   = gBackSaves[ iBackIndex ].sWidth;


					// Clip!!
					iTempX = sLeft + sShiftX;
					iTempY = sTop + sShiftY;

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
					sRight   = sRight + sShiftX - (INT16)uiRightSkip;
					sBottom  = sBottom + sShiftY - (INT16)uiBottomSkip;

					usHeight = sBottom - sTop;
					usWidth  = sRight -  sLeft;

					Blt16BPPTo16BPP((UINT16*)pDestBuf, uiDestPitchBYTES, (UINT16*)gVideoOverlays[uiCount].pSaveArea, gBackSaves[iBackIndex].sWidth * 2, sLeft, sTop, uiLeftSkip, uiTopSkip, usWidth, usHeight);

					// Once done, check for pending deletion
					if ( gVideoOverlays[uiCount].fDeletionPending )
					{
						RemoveVideoOverlay( uiCount );
					}

				}
		}

	}

	UnLockVideoSurface( BACKBUFFER );

	return(TRUE);
}


// Common callbacks for topmost blitters
void BlitMFont( VIDEO_OVERLAY *pBlitter )
{
	UINT8	 *pDestBuf;
	UINT32 uiDestPitchBYTES;

	pDestBuf = LockVideoSurface( pBlitter->uiDestBuff, &uiDestPitchBYTES);

	SetFont( pBlitter->uiFontID );
	SetFontBackground( pBlitter->ubFontBack );
	SetFontForeground( pBlitter->ubFontFore );

	mprintf_buffer(pDestBuf, uiDestPitchBYTES, pBlitter->sX, pBlitter->sY, pBlitter->zText);

	UnLockVideoSurface( pBlitter->uiDestBuff );

}


BOOLEAN BlitBufferToBuffer(UINT32 uiSrcBuffer, UINT32 uiDestBuffer, UINT16 usSrcX, UINT16 usSrcY, UINT16 usWidth, UINT16 usHeight)
{
	UINT32 uiDestPitchBYTES, uiSrcPitchBYTES;
	UINT8	 *pDestBuf, *pSrcBuf;
	BOOLEAN fRetVal;

	pDestBuf = LockVideoSurface(uiDestBuffer, &uiDestPitchBYTES);
	pSrcBuf = LockVideoSurface(uiSrcBuffer, &uiSrcPitchBYTES);

	fRetVal = Blt16BPPTo16BPP( (UINT16*)pDestBuf, uiDestPitchBYTES, (UINT16*)pSrcBuf, uiSrcPitchBYTES,
			usSrcX, usSrcY,
			usSrcX, usSrcY,
			usWidth, usHeight);

	UnLockVideoSurface(uiDestBuffer);
	UnLockVideoSurface(uiSrcBuffer);

	return( fRetVal );
}


void EnableVideoOverlay(BOOLEAN fEnable, INT32 iOverlayIndex)
{
	if (iOverlayIndex == -1) return;
	VIDEO_OVERLAY* v = &gVideoOverlays[iOverlayIndex];
	if (!v->fAllocated) return;

	v->fDisabled = !fEnable;
	DisableBackgroundRect(v->uiBackground, !fEnable);
}
