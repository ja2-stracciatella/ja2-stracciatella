#ifdef JA2_PRECOMPILED_HEADERS
	#include "JA2 SGP ALL.H"
#elif defined( WIZ8_PRECOMPILED_HEADERS )
	#include "WIZ8 SGP ALL.H"
#else
	#include "DirectDraw Calls.h"
	#include <stdio.h>
	#include "debug.h"
	#if defined( JA2 ) || defined( UTIL )
		#include "video.h"													// JA2
	#else
		#include "video2.h"				// Wiz8
	#endif
	#include "himage.h"
	#include "vobject.h"
	#include "vobject_private.h"
	#include "video_private.h"
	#include "wcheck.h"
	#include "vobject.h"
	#include "vobject_blitters.h"
	#include "shading.h"
#endif

SGPRect	ClippingRect={0, 0, 640, 480};
													//555      565
UINT32	guiTranslucentMask=0x3def; //0x7bef;		// mask for halving 5,6,5

// GLOBALS for pre-calculating skip values
INT32		gLeftSkip, gRightSkip, gTopSkip, gBottomSkip;
BOOLEAN	gfUsePreCalcSkips = FALSE;


//*Experimental**********************************************************************

/**********************************************************************************************
 Blt16BPPDataTo16BPPBufferTransZClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt16BPPDataTo16BPPBufferTransZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
UINT16		*p16BPPPalette;
UINT32		uiOffset;
UINT32		usHeight, usWidth, Unblitted;
UINT8		*SrcPtr, *DestPtr, *ZPtr;
UINT32		LineSkip;
ETRLEObject *pTrav;
INT32		iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
INT32		ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax

		mov		ax, [esi]
		mov		[edi], ax

BlitNTL2:
		add		esi, 2
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker

		xor		eax, eax

RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		js		RSLoop1
		jz		RSEnd

		add		esi, eax
		add		esi, eax
		jmp		RSLoop1

RSEnd:
		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt16BPPDataTo16BPPBufferTransparentClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt16BPPDataTo16BPPBufferTransparentClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
//UINT16		*p16BPPPalette;
//UINT32		uiOffset;
UINT32		usHeight, usWidth, Unblitted;
UINT8		*SrcPtr, *DestPtr;
UINT32		LineSkip;
//ETRLEObject *pTrav;
INT32		iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
INT32		ClipX1, ClipY1, ClipX2, ClipY2;
SixteenBPPObjectInfo *	p16BPPObject;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	p16BPPObject=&(hSrcVObject->p16BPPObject[usIndex]);

	// Get Offsets from Index into structure
	usHeight				= p16BPPObject->usHeight;
	usWidth					= p16BPPObject->usWidth;

	// Add to start position of dest buffer
	iTempX = iX + p16BPPObject->sOffsetX;
	iTempY = iY + p16BPPObject->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	if ( gfUsePreCalcSkips )
	{
		LeftSkip= gLeftSkip;
		RightSkip= gRightSkip;
		TopSkip= gTopSkip;
		BottomSkip= gBottomSkip;
		gfUsePreCalcSkips = FALSE;

	}
	else
	{
		// Calculate rows hanging off each side of the screen
		LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
		RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
		TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
		BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);
	}

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)p16BPPObject->p16BPPData;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
//		mov		edx, p16BPPPalette
		xor		eax, eax
//		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

//BlitNTL1:

		rep		movsw

//BlitNTL2:

//BlitLineEnd:
		add		esi, Unblitted
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker

		xor		eax, eax

RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		js		RSLoop1
		jz		RSEnd

		add		esi, eax
		add		esi, eax
		jmp		RSLoop1

RSEnd:
		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/***********************************************************************************/

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZNBClipTranslucent

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	NOT updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

	Blits every second pixel ("Translucents").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBClipTranslucent( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
UINT16 *p16BPPPalette;
UINT32 uiOffset, uiLineFlag;
UINT32 usHeight, usWidth, Unblitted;
UINT8	 *SrcPtr, *DestPtr, *ZPtr;
UINT32 LineSkip;
ETRLEObject *pTrav;
INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));
	uiLineFlag=(iTempY&1);

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:
		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		xor		eax, eax
		mov		edx, p16BPPPalette
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		shr		eax, 1
		and		eax, [guiTranslucentMask]

		xor		edx, edx
		mov		dx, [edi]
		shr		edx, 1
		and		edx, [guiTranslucentMask]

		add		eax, edx

		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}


/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZTranslucent

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

	Blits every second pixel ("Translucents").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZTranslucent( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
UINT32 usHeight, usWidth, uiOffset, LineSkip;
INT32	 iTempX, iTempY;
UINT16 *p16BPPPalette;
UINT8	 *SrcPtr, *DestPtr, *ZPtr;
UINT32 uiLineFlag;
ETRLEObject *pTrav;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));
	uiLineFlag=(iTempY&1);

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
//		mov		edx, p16BPPPalette

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:
		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL5

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax
		mov		edx, p16BPPPalette
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		shr		eax, 1
		and		eax, guiTranslucentMask

		xor		edx, edx
		mov		dx, [edi]
		shr		edx, 1
		and		edx, guiTranslucentMask

		add		eax, edx
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		xor		uiLineFlag, 1
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZClipTranslucent

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

	Blits every second pixel ("Translucents").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZClipTranslucent( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
UINT16 *p16BPPPalette;
UINT32 uiOffset, uiLineFlag;
UINT32 usHeight, usWidth, Unblitted;
UINT8	 *SrcPtr, *DestPtr, *ZPtr;
UINT32 LineSkip;
ETRLEObject *pTrav;
INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));
	uiLineFlag=(iTempY&1);

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:
		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax
		mov		edx, p16BPPPalette
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		xor		edx, edx
		shr		eax, 1
		mov		dx, [edi]
		and		eax, [guiTranslucentMask]
		shr		edx, 1
		and		edx, [guiTranslucentMask]

		add		eax, edx

		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		xor		uiLineFlag, 1
		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZNBTranslucent

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	NOT updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

	Blits every second pixel ("Translucents").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBTranslucent( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
UINT32 usHeight, usWidth, uiOffset, LineSkip;
INT32	 iTempX, iTempY;
UINT16 *p16BPPPalette;
UINT8	 *SrcPtr, *DestPtr, *ZPtr;
UINT32 uiLineFlag;
ETRLEObject *pTrav;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));
	uiLineFlag=(iTempY&1);

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, p16BPPPalette

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:
		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL5

		xor		edx, edx
		xor		eax, eax
		mov		edx, p16BPPPalette
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		shr		eax, 1
		mov		dx, [edi]
		and		eax, [guiTranslucentMask]

		shr		edx, 1
		and		edx, [guiTranslucentMask]

		add		eax, edx
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		xor		uiLineFlag, 1
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}















/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferTransZIncClip

	Used for large brushes (larger vertically than a single tile). Increments the Z
	value by Z_SUBLAYERS for every WORLD_TILE_Y lines of pixels blitted.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferTransZIncClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	UINT8	 *pPal8BPP;
	UINT16 usZLevel, usZLinesToGo;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip));
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	pPal8BPP = hSrcVObject->pShade8;
	LineSkip=(uiDestPitchBYTES-(BlitLength));
	LineSkipZ=LineSkip*2;

	usZLevel=usZValue;
//	usZLinesToGo=WORLD_TILE_Y;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZLevel
		ja		BlitNTL2

		mov		ax, usZLevel
		mov		[ebx], ax

		xor		eax, eax

		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL2:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		// check for incrementing of z level
		dec		usZLinesToGo
		jnz		RSLoop2

//		mov		ax, usZLevel
//		add		ax, Z_SUBLEVELS
//		mov		usZLevel, ax

//		mov		ax, WORLD_TILE_Y
//		mov		usZLinesToGo, ax

RSLoop2:
		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkipZ

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}


/**********************************************************************************************
 InitZBuffer

	Allocates and initializes a Z buffer for use with the Z buffer blitters. Doesn't really do
	much except allocate a chunk of memory, and zero it.

**********************************************************************************************/
UINT16 *InitZBuffer(UINT32 uiPitch, UINT32 uiHeight)
{
UINT16 *pBuffer;

	if((pBuffer=MemAlloc(uiPitch*uiHeight))==NULL)
		return(NULL);

	memset(pBuffer, 0, (uiPitch*uiHeight));
	return(pBuffer);
}

/**********************************************************************************************
 ShutdownZBuffer

	Frees up the memory allocated for the Z buffer.

**********************************************************************************************/
BOOLEAN ShutdownZBuffer(UINT16 *pBuffer)
{
	MemFree(pBuffer);
	return(TRUE);
}


BOOLEAN BlitZRect(UINT16 *pZBuffer, UINT32 uiPitch, INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom, UINT16 usZValue)
{
INT16 sLeftClip, sTopClip, sRightClip, sBottomClip;
UINT8 *pZPtr;
UINT32 uiLineSkip, usWidth, usHeight;

	sLeftClip=__max(ClippingRect.iLeft, sLeft);
	sRightClip=__min(ClippingRect.iRight, sRight);
	sTopClip=__max(ClippingRect.iTop, sTop);
	sBottomClip=__min(ClippingRect.iBottom, sBottom);

	usHeight=sBottomClip-sTopClip;
	usWidth=sRightClip-sLeftClip;
	pZPtr=(UINT8 *)pZBuffer+(sTopClip*uiPitch)+(sLeftClip*2);
	uiLineSkip=(uiPitch-(usWidth*2));

	if((usHeight==0) || (usWidth==0))
		return(FALSE);

	__asm {
		mov		edi, pZPtr
		xor		eax, eax
		mov		ax, usZValue
		mov		ebx, usWidth
		mov		edx, usHeight

BZR1:
		mov		ecx, ebx
		rep		stosw
		add		edi, uiLineSkip

		dec		edx
		jnz		BZR1
	}

	return(TRUE);
}






//*****************************************************************************
//** 8 Bit Blitters
//**
//*****************************************************************************


/**********************************************************************************************
 Blt8BPPDataTo8BPPBuffer

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBuffer( UINT8 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	static UINT32 uiOffset;
	static UINT32 usHeight, usWidth;
	static UINT8	 *SrcPtr, *DestPtr;
	static UINT32 LineSkip;
	static ETRLEObject *pTrav;
	static INT32	 iTempX, iTempY;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX);
	LineSkip=(uiDestPitchBYTES-(usWidth));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		xor		ebx, ebx
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		movsb

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		movsw

BlitNTL3:

		or		cl, cl
		jz		BlitDispatch

		xor		ebx, ebx

//BlitNTL4:

		rep		movsd

		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
		xor		al, al
		rep		stosb

		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferMonoShadow

	Uses a bitmap an 8BPP template for blitting. Anywhere a 1 appears in the bitmap, a shadow
	is blitted to the destination (a black pixel). Any other value above zero is considered a
	forground color, and zero is background. If the parameter for the background color is zero,
	transparency is used for the background.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferMonoShadow( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT8 ubForeground, UINT8 ubBackground)
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;
	UINT8	 *pPal8BPP;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*iTempY) + (iTempX*2);
	LineSkip=(uiDestPitchBYTES-(usWidth));
	LineSkipZ=LineSkip*2;
	pPal8BPP=hSrcVObject->pShade8;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:

		mov		al, [esi]
		cmp		al, 1
		jne		BlitNTL6

		xor		al, al
		mov		[edi], al
		jmp		BlitNTL5

BlitNTL6:
		or		al, al
		jz		BlitNTL7

		mov		al, ubForeground
		mov		[edi], al
		jmp		BlitNTL5

BlitNTL7:
		cmp		ubBackground, 0
		je		BlitNTL5

		mov		al, ubBackground
		mov		[edi], al

BlitNTL5:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:
		and		ecx, 07fH

		mov		al, ubBackground
		or		al, al
		jz		BlitTrans1

		rep		stosb
		jmp		BlitDispatch


BlitTrans1:
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkipZ
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}


/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferMonoShadowClip

	Uses a bitmap an 8BPP template for blitting. Anywhere a 1 appears in the bitmap, a shadow
	is blitted to the destination (a black pixel). Any other value above zero is considered a
	forground color, and zero is background. If the parameter for the background color is zero,
	transparency is used for the background.

	**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferMonoShadowClip( UINT8 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT8 ubForeground, UINT8 ubBackground)
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip, LineSkipZ;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	UINT8	 *pPal8BPP;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip));
	LineSkip=(uiDestPitchBYTES-(BlitLength));
	LineSkipZ=LineSkip*2;
	pPal8BPP=hSrcVObject->pShade8;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		xor		ecx, ecx
		mov		edx, pPal8BPP

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:
		xor		eax, eax
		mov		al, [esi]
		cmp		al, 1
		jne		BlitNTL3

		// write shadow pixel
		xor		al, al
		mov		[edi], al
		jmp		BlitNTL2

BlitNTL3:
		or		al, al
		jz		BlitNTL4

		// write foreground pixel
		mov		al, ubForeground
		mov		[edi], al
		jmp		BlitNTL2

BlitNTL4:
		cmp		ubBackground, 0
		je		BlitNTL2

		//write background pixel
		mov		al, ubBackground
		mov		[edi], al

BlitNTL2:
		inc		esi
		inc		edi
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels
		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:
		sub		LSCount, ecx

		mov		al, ubBackground
		or		al, al
		jz		BTrans2

		rep		stosb
		jmp		BlitDispatch

BTrans2:
		add		edi, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferTransZPixelate

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferTransZPixelate( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;
	UINT32 uiLineFlag;
	UINT8	 *pPal8BPP;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*iTempY) + (iTempX*2);
	pPal8BPP = hSrcVObject->pShade8;
	LineSkip=(uiDestPitchBYTES-(usWidth));
	LineSkipZ=LineSkip*2;
	uiLineFlag=(iTempY&1);


	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:
		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL5
		jmp		BlitNTL7

BlitNTL6:
		test	edi, 2
		jnz		BlitNTL5

BlitNTL7:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL5

		mov		ax, usZValue
		mov		[ebx], ax

		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL5:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		xor		uiLineFlag, 1
		add		edi, LineSkip
		add		ebx, LineSkipZ
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferTransZNBPixelate

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on. The Z value is
	not updated,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferTransZNBPixelate( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;
	UINT32 uiLineFlag;
	UINT8	 *pPal8BPP;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*iTempY) + (iTempX*2);
	pPal8BPP = hSrcVObject->pShade8;
	LineSkip=(uiDestPitchBYTES-(usWidth));
	LineSkipZ=LineSkip*2;

	uiLineFlag=(iTempY&1);

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:
		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL5
		jmp		BlitNTL7

BlitNTL6:
		test	edi, 2
		jnz		BlitNTL5

BlitNTL7:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL5

		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL5:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkipZ
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferTransZClipPixelate

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferTransZClipPixelate( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	UINT32 uiLineFlag;
	UINT8	 *pPal8BPP;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip));
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	pPal8BPP = hSrcVObject->pShade8;
	LineSkip=(uiDestPitchBYTES-(BlitLength));
	LineSkipZ=LineSkip*2;

	uiLineFlag=(iTempY&1);

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, pPal8BPP
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL2
		jmp		BlitNTL7

BlitNTL6:
		test	edi, 2
		jnz		BlitNTL2

BlitNTL7:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax

		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL2:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		xor		uiLineFlag, 1
		add		edi, LineSkip
		add		ebx, LineSkipZ

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferTransZNBClipPixelate

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, the Z value is
	not updated,	for any non-transparent pixels. The Z-buffer is 16 bit, and must be the same
	dimensions as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferTransZNBClipPixelate( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	UINT32 uiLineFlag;
	UINT8	 *pPal8BPP;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip));
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	pPal8BPP=hSrcVObject->pShade8;
	LineSkip=(uiDestPitchBYTES-(BlitLength));
	LineSkipZ=LineSkip*2;

	uiLineFlag=(iTempY&1);

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, pPal8BPP
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL2
		jmp		BlitNTL7

BlitNTL6:
		test	edi, 2
		jnz		BlitNTL2

BlitNTL7:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		xor		eax, eax

		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL2:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		xor		uiLineFlag, 1
		add		edi, LineSkip
		add		ebx, LineSkipZ

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

















/******************************************************************************
 Blt8BPPDataTo8BPPBufferTransparentClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. Clips the brush.

*******************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferTransparentClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	UINT8	 *pPal8BPP;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip));
	LineSkip=(uiDestPitchBYTES-(BlitLength));
	pPal8BPP=hSrcVObject->pShade8;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, pPal8BPP
//		mov		edx, pointer to shade table here
		xor		eax, eax
		mov		ebx, TopSkip
		xor		ecx, ecx

		or		ebx, ebx							// check for nothing clipped on top
		jz		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		ebx
		jnz		TopSkipLoop




LeftSkipSetup:

		mov		Unblitted, 0
		mov		ebx, LeftSkip					// check for nothing clipped on the left
		or		ebx, ebx
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, ebx
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, ebx							// skip partial run, jump into normal loop for rest
		sub		ecx, ebx
		mov		ebx, BlitLength
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		ebx, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, ebx
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, ebx							// skip partial run, jump into normal loop for rest
		mov		ebx, BlitLength
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		ebx, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop




BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		ebx, BlitLength
		mov		Unblitted, 0

BlitDispatch:

		or		ebx, ebx							// Check to see if we're done blitting
		jz		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, ebx
		jbe		BNTrans1

		sub		ecx, ebx
		mov		Unblitted, ecx
		mov		ecx, ebx

BNTrans1:
		sub		ebx, ecx

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al
		inc		esi
		inc		edi

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al

		mov		al, [esi+1]
		mov		al, [edx+eax]
		mov		[edi+1], al

		add		esi, 2
		add		edi, 2

BlitNTL3:

		or		cl, cl
		jz		BlitLineEnd

BlitNTL4:

		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al

		mov		al, [esi+1]
		mov		al, [edx+eax]
		mov		[edi+1], al

		mov		al, [esi+2]
		mov		al, [edx+eax]
		mov		[edi+2], al

		mov		al, [esi+3]
		mov		al, [edx+eax]
		mov		[edi+3], al

		add		esi, 4
		add		edi, 4

		dec		cl
		jnz		BlitNTL4

BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, ebx
		jbe		BTrans1

		mov		ecx, ebx

BTrans1:

		sub		ebx, ecx
		add		edi, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}


/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferTransparent

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferTransparent( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *pPal8BPP;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX);
	LineSkip=(uiDestPitchBYTES-(usWidth));
	pPal8BPP=hSrcVObject->pShade8;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		xor		ebx, ebx
		xor		ecx, ecx
		mov		edx, pPal8BPP

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		clc
		rcr		cl, 1
		jnc		BlitNTL2

//		movsb

		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al
		inc		esi
		inc		edi

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

//		movsw

		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al

		mov		al, [esi+1]
		mov		al, [edx+eax]
		mov		[edi+1], al

		add		esi, 2
		add		edi, 2

BlitNTL3:

		or		cl, cl
		jz		BlitDispatch

BlitNTL4:

//		rep		movsd

		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al

		mov		al, [esi+1]
		mov		al, [edx+eax]
		mov		[edi+1], al

		mov		al, [esi+2]
		mov		al, [edx+eax]
		mov		[edi+2], al

		mov		al, [esi+3]
		mov		al, [edx+eax]
		mov		[edi+3], al

		add		esi, 4
		add		edi, 4

		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}



/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferTransZ

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferTransZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ, uiZComp;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;
	UINT8	 *pPal8BPP;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*iTempY) + (iTempX*2);
	pPal8BPP = hSrcVObject->pShade8;
	LineSkip=(uiDestPitchBYTES-(usWidth));
	LineSkipZ=LineSkip*2;
	uiZComp=(UINT32)usZValue;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		clc
		rcr		cl, 1
		jnc		BlitNTL2

// do a byte
		mov		ax, [ebx]
		cmp		eax, uiZComp
		ja		BlitNTL4

		mov		eax, uiZComp
		mov		[ebx], ax

		xor		eax, eax
		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al


BlitNTL4:
		inc		esi
		inc		ebx
		inc		edi
		inc		ebx

// do a word
BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL6

// word - first pixel
		mov		ax, [ebx]
		cmp		eax, uiZComp
		ja		BlitNTL3

		mov		eax, uiZComp
		mov		[ebx], ax

		xor		eax, eax
		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al

// word - second
BlitNTL3:
		mov		ax, [ebx+2]
		cmp		eax, uiZComp
		ja		BlitNTL12

		mov		eax, uiZComp
		mov		[ebx+2], ax

		xor		eax, eax
		mov		al, [esi+1]
		mov		al, [edx+eax]
		mov		[edi+1], al

BlitNTL12:

		inc		esi
		inc		edi
		inc		esi
		inc		edi
		add		ebx, 4

// do a dword
BlitNTL6:
		or		cl, cl
		jz		BlitDispatch

BlitNTL7:

// dword - first pixel
		mov		ax, [ebx]
		cmp		eax, uiZComp
		ja		BlitNTL8

		mov		eax, uiZComp
		mov		[ebx], ax

		xor		eax, eax
		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al

// dword - second
BlitNTL8:
		mov		ax, [ebx+2]
		cmp		eax, uiZComp
		ja		BlitNTL9

		mov		eax, uiZComp
		mov		[ebx+2], ax

		xor		eax, eax
		mov		al, [esi+1]
		mov		al, [edx+eax]
		mov		[edi+1], al

BlitNTL9:
// dword - third pixel
		mov		ax, [ebx+4]
		cmp		eax, uiZComp
		ja		BlitNTL10

		mov		eax, uiZComp
		mov		[ebx+4], ax

		xor		eax, eax
		mov		al, [esi+2]
		mov		al, [edx+eax]
		mov		[edi+2], al

// dword - fourth
BlitNTL10:
		mov		ax, [ebx+6]
		cmp		eax, uiZComp
		ja		BlitNTL11

		mov		eax, uiZComp
		mov		[ebx+6], ax

		xor		eax, eax
		mov		al, [esi+3]
		mov		al, [edx+eax]
		mov		[edi+3], al


BlitNTL11:
		add		esi, 4
		add		edi, 4
		add		ebx, 8
		dec		cl
		jnz		BlitNTL7

		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkipZ
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferTransZNB

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on. The Z value is
	not updated,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferTransZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;
	UINT8	 *pPal8BPP;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*iTempY) + (iTempX*2);
	pPal8BPP = hSrcVObject->pShade8;
	LineSkip=(uiDestPitchBYTES-(usWidth));
	LineSkipZ=LineSkip*2;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		xor		eax, eax

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL5

		xor		ah, ah
		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL5:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkipZ
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferTransZNBColor

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on. The Z value is
	not updated,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions as the destination. Any pixels that fail the Z test
	are written to with the specified color.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferTransZNBColor( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT8 ubColor)
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;
	UINT8	 *pPal8BPP;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*iTempY) + (iTempX*2);
	pPal8BPP = hSrcVObject->pShade8;
	LineSkip=(uiDestPitchBYTES-(usWidth));
	LineSkipZ=LineSkip*2;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		xor		eax, eax

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL5

		xor		ah, ah
		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al
		jmp		BlitNTL6

BlitNTL5:
		xor		ah, ah
		mov		al, ubColor
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL6:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkipZ
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferTransZClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferTransZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	UINT8	 *pPal8BPP;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip));
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	pPal8BPP = hSrcVObject->pShade8;
	LineSkip=(uiDestPitchBYTES-(BlitLength));
	LineSkipZ=LineSkip*2;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax

		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL2:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkipZ

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferTransZNBClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, the Z value is
	not updated,	for any non-transparent pixels. The Z-buffer is 16 bit, and must be the same
	dimensions as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferTransZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	UINT8	 *pPal8BPP;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip));
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	pPal8BPP = hSrcVObject->pShade8;
	LineSkip=(uiDestPitchBYTES-(BlitLength));
	LineSkipZ=LineSkip*2;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		xor		eax, eax

		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL2:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkipZ

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferTransZNBClipColor

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, the Z value is
	not updated,	for any non-transparent pixels. The Z-buffer is 16 bit, and must be the same
	dimensions as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferTransZNBClipColor( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT8 ubColor)
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	UINT8	 *pPal8BPP;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip));
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	pPal8BPP = hSrcVObject->pShade8;
	LineSkip=(uiDestPitchBYTES-(BlitLength));
	LineSkipZ=LineSkip*2;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx
		xor		eax, eax

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		xor		ah, ah
		mov		al, [esi]
		mov		al, [edx+eax]
		mov		[edi], al
		jmp		BlitNTL3

BlitNTL2:

		xor		ah, ah
		mov		al, ubColor
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL3:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkipZ

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferShadowZ

	Creates a shadow using a brush, but modifies the destination buffer only if the current
	Z level is equal to higher than what's in the Z buffer at that pixel location. It
	updates the Z buffer with the new Z level.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferShadowZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;
	UINT8	 *pPal8BPP;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*iTempY) + (iTempX*2);
	pPal8BPP = hSrcVObject->pShade8;
	LineSkip=(uiDestPitchBYTES-(usWidth));
	LineSkipZ=LineSkip*2;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax
		mov		al, [edi]
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL5:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkipZ
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferShadowZNB

	Creates a shadow using a brush, but modifies the destination buffer only if the current
	Z level is equal to higher than what's in the Z buffer at that pixel location.
	The Z buffer is NOT updated with the new information.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferShadowZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;
	UINT8	 *pPal8BPP;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*iTempY) + (iTempX*2);
	pPal8BPP = hSrcVObject->pShade8;
	LineSkip=(uiDestPitchBYTES-(usWidth));
	LineSkipZ=LineSkip*2;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		xor		eax, eax
		mov		al, [edi]
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL5:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkipZ
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferShadowZClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, the Z value is
	updated, for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferShadowZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	UINT8	 *pPal8BPP;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip));
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	pPal8BPP = hSrcVObject->pShade8;
	LineSkip=(uiDestPitchBYTES-(BlitLength));
	LineSkipZ=LineSkip*2;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax
		mov		al, [edi]
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL2:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkipZ

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferShadowZNBClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, the Z value is
	NOT updated, for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferShadowZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	UINT8	 *pPal8BPP;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip));
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	pPal8BPP = hSrcVObject->pShade8;
	LineSkip=(uiDestPitchBYTES-(BlitLength));
	LineSkipZ=LineSkip*2;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		xor		eax, eax
		mov		al, [edi]
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL2:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkipZ

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferTransShadowZ

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. If the source pixel is 254,
	it is considered a shadow, and the destination buffer is darkened rather than blitted on.
	The Z-buffer is 16 bit, and	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferTransShadowZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT16 *p16BPPPalette )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;
	UINT8	 *pPal8BPP;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*iTempY) + (iTempX*2);
	LineSkip=(uiDestPitchBYTES-(usWidth));
	LineSkipZ=LineSkip*2;
	pPal8BPP=hSrcVObject->pShade8;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL6

		mov		al, [edi]
//		mov		al, ColorTable[eax] *********************************************
		mov		[edi], al
		jmp		BlitNTL5


BlitNTL6:
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL5:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkipZ
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferTransShadowZNB

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	NOT updated,	for any non-transparent pixels. If the source pixel is 254,
	it is considered a shadow, and the destination buffer is darkened rather than blitted on.
	The Z-buffer is 16 bit, and	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferTransShadowZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT16 *p16BPPPalette )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;
	UINT8	 *pPal8BPP;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*iTempY) + (iTempX*2);
	LineSkip=(uiDestPitchBYTES-(usWidth));
	LineSkipZ=LineSkip*2;
	pPal8BPP=hSrcVObject->pShade8;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL6

//		mov		al, [edi]
//		mov		al, ColorTable[eax] *********************************************
//		mov		[edi], al
		jmp		BlitNTL5


BlitNTL6:
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL5:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkipZ
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}


/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferTransShadowZClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions as the destination. Pixels with a value of
	254 are shaded instead of blitted.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferTransShadowZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 *p16BPPPalette )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	UINT8	 *pPal8BPP;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip));
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength));
	LineSkipZ=LineSkip*2;
	pPal8BPP=hSrcVObject->pShade8;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL3

		mov		al, [edi]
//		mov		ax, ColorTable[eax] ************************************************
		mov		[edi], al
		jmp		BlitNTL2

BlitNTL3:
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL2:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkipZ

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferTransShadowZNBClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	NOT updated,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions as the destination. Pixels with a value of
	254 are shaded instead of blitted.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferTransShadowZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 *p16BPPPalette )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip, LineSkipZ;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	UINT8	 *pPal8BPP;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip));
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*2*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength));
	LineSkipZ=LineSkip*2;
	pPal8BPP=hSrcVObject->pShade8;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, pPal8BPP

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL3

//		mov		al, [edi]
//		mov		ax, ColorTable[eax] ************************************************
//		mov		[edi], al
		jmp		BlitNTL2

BlitNTL3:
		mov		al, [edx+eax]
		mov		[edi], al

BlitNTL2:
		inc		esi
		inc		edi
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
		add		edi, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkipZ

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferShadow

	Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
	image as a mask. Any Non-zero index pixels are used to darken destination pixels.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferShadow( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex)
{
	UINT8 *pPal8BPP;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX);
	pPal8BPP = hSrcVObject->pShade8;
	LineSkip=(uiDestPitchBYTES-(usWidth));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, usHeight
		xor		ecx, ecx
		mov		edx, OFFSET ShadeTable

BlitDispatch:


		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		xor		eax, eax

		add		esi, ecx

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitDispatch

BlitNTL4:

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		mov		ax, [edi+4]
		mov		ax, [edx+eax*2]
		mov		[edi+4], ax

		mov		ax, [edi+6]
		mov		ax, [edx+eax*2]
		mov		[edi+6], ax

		add		edi, 8
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		ebx
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo8BPPBufferShadowClip

	Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
	image as a mask. Any Non-zero index pixels are used to darken destination pixels. Blitter
	clips brush if it doesn't fit on the viewport.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo8BPPBufferShadowClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT8 *pPal8BPP;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip));
	pPal8BPP = hSrcVObject->pShade8;
	LineSkip=(uiDestPitchBYTES-(BlitLength));


	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, pPal8BPP
		xor		eax, eax
		mov		ebx, TopSkip
		xor		ecx, ecx

		or		ebx, ebx							// check for nothing clipped on top
		jz		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		ebx
		jnz		TopSkipLoop




LeftSkipSetup:

		mov		Unblitted, 0
		mov		ebx, LeftSkip					// check for nothing clipped on the left
		or		ebx, ebx
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, ebx
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, ebx							// skip partial run, jump into normal loop for rest
		sub		ecx, ebx
		mov		ebx, BlitLength
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		ebx, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, ebx
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, ebx							// skip partial run, jump into normal loop for rest
		mov		ebx, BlitLength
		jmp		BlitTransparent


LSTrans1:
		sub		ebx, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop




BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		ebx, BlitLength
		mov		Unblitted, 0

BlitDispatch:

		or		ebx, ebx							// Check to see if we're done blitting
		jz		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:

		cmp		ecx, ebx
		jbe		BNTrans1

		sub		ecx, ebx
		mov		Unblitted, ecx
		mov		ecx, ebx

BNTrans1:
		sub		ebx, ecx

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		ax, [edi]
		mov		ax, [edx+eax]
		mov		[edi], ax

		inc		esi
		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		ax, [edi]
		mov		ax, [edx+eax]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax]
		mov		[edi+2], ax

		add		esi, 2
		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitLineEnd

BlitNTL4:

		mov		ax, [edi]
		mov		ax, [edx+eax]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax]
		mov		[edi+2], ax

		mov		ax, [edi+4]
		mov		ax, [edx+eax]
		mov		[edi+4], ax

		mov		ax, [edi+6]
		mov		ax, [edx+eax]
		mov		[edi+6], ax

		add		esi, 4
		add		edi, 8
		dec		cl
		jnz		BlitNTL4

BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
		cmp		ecx, ebx
		jbe		BTrans1

		mov		ecx, ebx

BTrans1:

		sub		ebx, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


RightSkipLoop:


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}




//*****************************************************************************
//** 16 Bit Blitters
//**
//*****************************************************************************

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferMonoShadowClip

	Uses a bitmap an 8BPP template for blitting. Anywhere a 1 appears in the bitmap, a shadow
	is blitted to the destination (a black pixel). Any other value above zero is considered a
	forground color, and zero is background. If the parameter for the background color is zero,
	transparency is used for the background.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferMonoShadowClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 usForeground, UINT16 usBackground, UINT16 usShadow )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:
		xor		eax, eax
		mov		al, [esi]
		cmp		al, 1
		jne		BlitNTL3

		// write shadow pixel
		mov		ax, usShadow

		// only write if not zero
		cmp		ax, 0
		je		BlitNTL2

		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL3:
		or		al, al
		jz		BlitNTL4

		// write foreground pixel
		mov		ax, usForeground
		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL4:
		cmp		usBackground, 0
		je		BlitNTL2

		mov		ax, usBackground
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:
		sub		LSCount, ecx

		mov		ax, usBackground
		or		ax, ax
		jz		BTrans2

		rep		stosw
		jmp		BlitDispatch

BTrans2:
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}





/**********************************************************************************************
	Blt16BPPTo16BPP

	Copies a rect of 16 bit data from a video buffer to a buffer position of the brush
	in the data area, for later blitting. Used to copy background information for mercs
	etc. to their unblit buffer, for later reblitting. Does NOT clip.

**********************************************************************************************/
BOOLEAN Blt16BPPTo16BPP(UINT16 *pDest, UINT32 uiDestPitch, UINT16 *pSrc, UINT32 uiSrcPitch, INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos, UINT32 uiWidth, UINT32 uiHeight)
{
UINT16 *pSrcPtr, *pDestPtr;
UINT32 uiLineSkipDest, uiLineSkipSrc;

	Assert(pDest!=NULL);
	Assert(pSrc!=NULL);

	pSrcPtr=(UINT16 *)((UINT8 *)pSrc+(iSrcYPos*uiSrcPitch)+(iSrcXPos*2));
	pDestPtr=(UINT16 *)((UINT8 *)pDest+(iDestYPos*uiDestPitch)+(iDestXPos*2));
	uiLineSkipDest=uiDestPitch-(uiWidth*2);
	uiLineSkipSrc=uiSrcPitch-(uiWidth*2);

__asm {
	mov		esi, pSrcPtr
	mov		edi, pDestPtr
	mov		ebx, uiHeight
	cld

	mov		ecx, uiWidth
	test	ecx, 1
	jz		BlitDwords

BlitNewLine:

	mov		ecx, uiWidth
	shr		ecx, 1
	movsw

//BlitNL2:

	rep		movsd

	add		edi, uiLineSkipDest
	add		esi, uiLineSkipSrc
	dec		ebx
	jnz		BlitNewLine

	jmp		BlitDone


BlitDwords:
	mov		ecx, uiWidth
	shr		ecx, 1
	rep		movsd

	add		edi, uiLineSkipDest
	add		esi, uiLineSkipSrc
	dec		ebx
	jnz		BlitDwords

BlitDone:

	}

	return(TRUE);
}

/**********************************************************************************************
	Blt16BPPTo16BPPTrans

	Copies a rect of 16 bit data from a video buffer to a buffer position of the brush
	in the data area, for later blitting. Used to copy background information for mercs
	etc. to their unblit buffer, for later reblitting. Does NOT clip. Transparent color is
	not copied.

**********************************************************************************************/
BOOLEAN Blt16BPPTo16BPPTrans(UINT16 *pDest, UINT32 uiDestPitch, UINT16 *pSrc, UINT32 uiSrcPitch, INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos, UINT32 uiWidth, UINT32 uiHeight, UINT16 usTrans)
{
UINT16 *pSrcPtr, *pDestPtr;
UINT32 uiLineSkipDest, uiLineSkipSrc;

	Assert(pDest!=NULL);
	Assert(pSrc!=NULL);

	pSrcPtr=(UINT16 *)((UINT8 *)pSrc+(iSrcYPos*uiSrcPitch)+(iSrcXPos*2));
	pDestPtr=(UINT16 *)((UINT8 *)pDest+(iDestYPos*uiDestPitch)+(iDestXPos*2));
	uiLineSkipDest=uiDestPitch-(uiWidth*2);
	uiLineSkipSrc=uiSrcPitch-(uiWidth*2);

__asm {
	mov		esi, pSrcPtr
	mov		edi, pDestPtr
	mov		ebx, uiHeight
	mov		dx, usTrans

BlitNewLine:
	mov		ecx, uiWidth

Blit2:
	mov		ax, [esi]
	cmp		ax, dx
	je		Blit3

	mov		[edi], ax

Blit3:
	add		esi, 2
	add		edi, 2
	dec		ecx
	jnz		Blit2

	add		edi, uiLineSkipDest
	add		esi, uiLineSkipSrc
	dec		ebx
	jnz		BlitNewLine

	}

	return(TRUE);
}

/**********************************************************************************************
	Blt16BPPTo16BPPMirror

	Copies a rect of 16 bit data from a video buffer to a buffer position of the brush
	in the data area, for later blitting. Used to copy background information for mercs
	etc. to their unblit buffer, for later reblitting. Does NOT clip.

**********************************************************************************************/
BOOLEAN Blt16BPPTo16BPPMirror(UINT16 *pDest, UINT32 uiDestPitch, UINT16 *pSrc, UINT32 uiSrcPitch, INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos, UINT32 uiWidth, UINT32 uiHeight)
{
UINT16 *pSrcPtr, *pDestPtr;
UINT32 uiLineSkipDest, uiLineSkipSrc;
INT32	RightSkip, LeftSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
INT32 iTempX, iTempY, ClipX1, ClipY1, ClipX2, ClipY2;
SGPRect *clipregion=NULL;

	Assert(pDest!=NULL);
	Assert(pSrc!=NULL);

	// Add to start position of dest buffer
	iTempX = iDestXPos;
	iTempY = iDestYPos;

	if(clipregion==NULL)
	{
		ClipX1=0; //ClippingRect.iLeft;
		ClipY1=0; //ClippingRect.iTop;
		ClipX2=640; //ClippingRect.iRight;
		ClipY2=480; //ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - __min(ClipX1, iTempX), (INT32)uiWidth);
	RightSkip=__min(__max(ClipX2, (iTempX+(INT32)uiWidth)) - ClipX2, (INT32)uiWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)uiHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)uiHeight)) - ClipY2, (INT32)uiHeight);

	iTempX=__max(ClipX1, iDestXPos);
	iTempY=__max(ClipY1, iDestYPos);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)uiWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)uiHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)uiWidth) || (RightSkip >=(INT32)uiWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)uiHeight) || (BottomSkip >=(INT32)uiHeight))
		return(TRUE);

	pSrcPtr=(UINT16 *)((UINT8 *)pSrc+(TopSkip*uiSrcPitch)+(RightSkip*2));
	pDestPtr=(UINT16 *)((UINT8 *)pDest+(iTempY*uiDestPitch)+(iTempX*2)+((BlitLength-1)*2));
	uiLineSkipDest=uiDestPitch;//+((BlitLength-1)*2);
	uiLineSkipSrc=uiSrcPitch-(BlitLength*2);

__asm {
	mov		esi, pSrcPtr
	mov		edi, pDestPtr
	mov		ebx, BlitHeight

BlitNewLine:

	mov		ecx, BlitLength
  //add   edi, ecx
  //add   edi, ecx

BlitNTL2:

	mov		ax, [esi]
	mov		[edi], ax
	inc		esi
	dec		edi
	inc		esi
	dec		edi
	dec		ecx
	jnz		BlitNTL2

	add		edi, BlitLength
	add		esi, uiLineSkipSrc
	add		edi, BlitLength
	add		edi, uiLineSkipDest
	dec		ebx
	jnz		BlitNewLine

	}

	return(TRUE);
}

/***********************************************************************************************
	Blt8BPPTo8BPP

	Copies a rect of an 8 bit data from a video buffer to a buffer position of the brush
	in the data area, for later blitting. Used to copy background information for mercs
	etc. to their unblit buffer, for later reblitting. Does NOT clip.

**********************************************************************************************/
BOOLEAN Blt8BPPTo8BPP(UINT8 *pDest, UINT32 uiDestPitch, UINT8 *pSrc, UINT32 uiSrcPitch, INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos, UINT32 uiWidth, UINT32 uiHeight)
{
UINT8 *pSrcPtr, *pDestPtr;
UINT32 uiLineSkipDest, uiLineSkipSrc;

	Assert(pDest!=NULL);
	Assert(pSrc!=NULL);

	pSrcPtr=pSrc+(iSrcYPos*uiSrcPitch)+(iSrcXPos);
	pDestPtr=pDest+(iDestYPos*uiDestPitch)+(iDestXPos);
	uiLineSkipDest=uiDestPitch-(uiWidth);
	uiLineSkipSrc=uiSrcPitch-(uiWidth);

__asm {
	mov		esi, pSrcPtr
	mov		edi, pDestPtr
	mov		ebx, uiHeight
	cld

BlitNewLine:
	mov		ecx, uiWidth

	clc
	rcr		ecx, 1
	jnc		Blit2
	movsb

Blit2:
	clc
	rcr		ecx, 1
	jnc		Blit3

	movsw

Blit3:
	or		ecx, ecx
	jz		BlitLineDone

	rep		movsd

BlitLineDone:

	add		edi, uiLineSkipDest
	add		esi, uiLineSkipSrc
	dec		ebx
	jnz		BlitNewLine

	}

	return(TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZPixelate

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

	Blits every second pixel ("pixelates").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZPixelate( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
UINT32 usHeight, usWidth, uiOffset, LineSkip;
INT32	 iTempX, iTempY;
UINT16 *p16BPPPalette;
UINT8	 *SrcPtr, *DestPtr, *ZPtr;
UINT32 uiLineFlag;
ETRLEObject *pTrav;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));
	uiLineFlag=(iTempY&1);

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, p16BPPPalette

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL5
		jmp		BlitNTL7

BlitNTL6:
		test	edi, 2
		jnz		BlitNTL5

BlitNTL7:
		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL5

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		xor		uiLineFlag, 1
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}


/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZPixelateObscured

	// OK LIKE NORMAL PIXELATE BUT ONLY PIXELATES STUFF BELOW Z level

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

	Blits every second pixel ("pixelates").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZPixelateObscured( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
UINT32 usHeight, usWidth, uiOffset, LineSkip;
INT32	 iTempX, iTempY;
UINT16 *p16BPPPalette;
UINT8	 *SrcPtr, *DestPtr, *ZPtr;
UINT32 uiLineFlag;
ETRLEObject *pTrav;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));
	uiLineFlag=(iTempY&1);

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, p16BPPPalette

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:

		// TEST FOR Z FIRST!
		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL8

		// Write it NOW!
		jmp		BlitNTL7

BlitNTL8:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL5
		jmp		BlitNTL9

BlitNTL6:
		test	edi, 2
		jnz		BlitNTL5

BlitNTL7:

		// Write normal z value
		mov		ax, usZValue
		mov		[ebx], ax
		//jmp   BlitNTL10

BlitNTL9:

		// Write no z
		//mov		ax, 32767
		//mov		[ebx], ax

//BlitNTL10:

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		xor		uiLineFlag, 1
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}


/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZClipPixelate

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

	Blits every second pixel ("pixelates").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZClipPixelate( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
UINT16 *p16BPPPalette;
UINT32 uiOffset, uiLineFlag;
UINT32 usHeight, usWidth, Unblitted;
UINT8	 *SrcPtr, *DestPtr, *ZPtr;
UINT32 LineSkip;
ETRLEObject *pTrav;
INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));
	uiLineFlag=(iTempY&1);

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL2
		jmp		BlitNTL7

BlitNTL6:
		test	edi, 2
		jnz		BlitNTL2

BlitNTL7:
		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax

		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		xor		uiLineFlag, 1
		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZNBPixelate

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	NOT updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

	Blits every second pixel ("pixelates").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBPixelate( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
UINT32 usHeight, usWidth, uiOffset, LineSkip;
INT32	 iTempX, iTempY;
UINT16 *p16BPPPalette;
UINT8	 *SrcPtr, *DestPtr, *ZPtr;
UINT32 uiLineFlag;
ETRLEObject *pTrav;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));
	uiLineFlag=(iTempY&1);

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, p16BPPPalette

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL5
		jmp		BlitNTL7

BlitNTL6:
		test	edi, 2
		jnz		BlitNTL5

BlitNTL7:
		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL5

		// ATE: DONOT WRITE Z VALUE
		//mov		ax, usZValue
		//mov		[ebx], ax

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		xor		uiLineFlag, 1
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZNBClipPixelate

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	NOT updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

	Blits every second pixel ("pixelates").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBClipPixelate( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
UINT16 *p16BPPPalette;
UINT32 uiOffset, uiLineFlag;
UINT32 usHeight, usWidth, Unblitted;
UINT8	 *SrcPtr, *DestPtr, *ZPtr;
UINT32 LineSkip;
ETRLEObject *pTrav;
INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));
	uiLineFlag=(iTempY&1);

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL2
		jmp		BlitNTL7

BlitNTL6:
		test	edi, 2
		jnz		BlitNTL2

BlitNTL7:
		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		//mov		ax, usZValue
		//mov		[ebx], ax

		xor		eax, eax

		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		xor		uiLineFlag, 1
		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZ

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		xor		eax, eax

BlitNTL4:

		mov		ax, usZValue
		cmp		ax, [ebx]
		jb		BlitNTL5

		mov		[ebx], ax

		xor		ah, ah
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		inc		edi
		inc		ebx
		inc		edi
		inc		ebx

		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZNB

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on. The Z value is
	NOT updated by this version. The Z-buffer is 16 bit, and	must be the same dimensions
	(including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		xor		eax, eax

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL5

		xor		ah, ah
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		inc		edi
		inc		ebx
		inc		edi
		inc		ebx
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZNBColor

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on. The Z value is
	NOT updated by this version. The Z-buffer is 16 bit, and	must be the same dimensions
	(including Pitch) as the destination. Any pixels that fail the Z test, are written
	to with the specified color value.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBColor( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT16 usColor)
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		xor		eax, eax

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL5

		xor		ah, ah
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax
		jmp		BlitNTL6

BlitNTL5:
		mov		ax, usColor
		mov		[edi], ax

BlitNTL6:
		inc		esi
		inc		edi
		inc		ebx
		inc		edi
		inc		ebx
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransShadow

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. If the source pixel is 254,
	it is considered a shadow, and the destination buffer is darkened rather than blitted on.
	The Z-buffer is 16 bit, and	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadow( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT16 *p16BPPPalette )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL6

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL5


BlitNTL6:
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransShadowZ

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. If the source pixel is 254,
	it is considered a shadow, and the destination buffer is darkened rather than blitted on.
	The Z-buffer is 16 bit, and	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT16 *p16BPPPalette )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL6

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL5


BlitNTL6:
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransShadowZNB

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on. The Z value is NOT
	updated. If the source pixel is 254, it is considered a shadow, and the destination
	buffer is darkened rather than blitted on. The Z-buffer is 16 bit, and must be the same
	dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT16 *p16BPPPalette )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL5

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL6

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL5


BlitNTL6:
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}


#if 0

BlitNTL4:

		// TEST FOR Z FIRST!
		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL8

		// Write it NOW!
		jmp		BlitNTL7

BlitNTL8:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL5
		jmp		BlitNTL9

BlitNTL6:
		test	edi, 2
		jnz		BlitNTL5

BlitNTL7:

		// Write normal z value
		mov		ax, usZValue
		mov		[ebx], ax
		jmp   BlitNTL10

BlitNTL9:

		// Write high z
		mov		ax, 32767
		mov		[ebx], ax

BlitNTL10:

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax
#endif


/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransShadowZNBObscured

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on. The Z value is NOT
	updated. If the source pixel is 254, it is considered a shadow, and the destination
	buffer is darkened rather than blitted on. The Z-buffer is 16 bit, and must be the same
	dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNBObscured( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT16 *p16BPPPalette )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;
	UINT32 uiLineFlag;



	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	LineSkip=(uiDestPitchBYTES-(usWidth*2));
	uiLineFlag=(iTempY&1);


	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:


		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL8

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL6

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL5


BlitNTL8:

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		je		BlitNTL5

		test	uiLineFlag, 1
		jz		BlitNTL9

		test	edi, 2
		jz		BlitNTL5

		jmp		BlitNTL6

BlitNTL9:
		test	edi, 2
		jnz		BlitNTL5


BlitNTL6:

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		xor		uiLineFlag, 1
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}


/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransShadowZClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination. Pixels with a value of
	254 are shaded instead of blitted.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 *p16BPPPalette )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax

		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL3

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL3:
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}


/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransShadowClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination. Pixels with a value of
	254 are shaded instead of blitted.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 *p16BPPPalette )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:
		xor		eax, eax

		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL3

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL3:
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransShadowZNBClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on.
	The Z-buffer is 16 bit, and	must be the same dimensions (including Pitch) as the
	destination. Pixels with a value of	254 are shaded instead of blitted. The Z buffer is
	NOT updated.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 *p16BPPPalette )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		xor		eax, eax

		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL3

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL3:
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}


/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransShadowZNBClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on.
	The Z-buffer is 16 bit, and	must be the same dimensions (including Pitch) as the
	destination. Pixels with a value of	254 are shaded instead of blitted. The Z buffer is
	NOT updated.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNBObscuredClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 *p16BPPPalette )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted, uiLineFlag;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:

		xor		uiLineFlag, 1
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitPixellate

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL3

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL2

BlitPixellate:

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		je		BlitNTL2

		test	uiLineFlag, 1
		jz		BlitNTL9

		test	edi, 2
		jz		BlitNTL2
		jmp		BlitNTL3

BlitNTL9:
		test	edi, 2
		jnz		BlitNTL2


BlitNTL3:

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		xor		uiLineFlag, 1
		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}



/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransShadowZNBClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below OR EQUAL! that of the current pixel, it is written on.
	The Z-buffer is 16 bit, and	must be the same dimensions (including Pitch) as the
	destination. Pixels with a value of	254 are shaded instead of blitted. The Z buffer is
	NOT updated.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowBelowOrEqualZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 *p16BPPPalette )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		xor		eax, eax

		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL3

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL3:
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}


/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferShadowZ

	Creates a shadow using a brush, but modifies the destination buffer only if the current
	Z level is equal to higher than what's in the Z buffer at that pixel location. It
	updates the Z buffer with the new Z level.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET ShadeTable
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		xor		eax, eax
		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax
		mov		ax, usZValue
		mov		[ebx], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferShadowZClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET ShadeTable
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferShadowZNB

	Creates a shadow using a brush, but modifies the destination buffer only if the current
	Z level is equal to higher than what's in the Z buffer at that pixel location. It does
	NOT update the Z buffer with the new Z value.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET ShadeTable
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		xor		eax, eax
		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferShadowZNBClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, the Z value is
	not updated,	for any non-transparent pixels. The Z-buffer is 16 bit, and	must be the
	same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET ShadeTable
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		xor		eax, eax

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax

		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZNBClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on. The Z value is NOT
	updated in this version. The Z-buffer is 16 bit, and must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		xor		eax, eax

		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZNBClipColor

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on. The Z value is NOT
	updated in this version. The Z-buffer is 16 bit, and must be the same dimensions (including
	Pitch) as the destination. Any pixels that fail the Z test are written to with the
	specified pixel value.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBClipColor( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 usColor)
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx
		xor		eax, eax

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		xor		ah, ah
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax
		jmp		BlitNTL3

BlitNTL2:

		mov		ax, usColor
		mov		[edi], ax

BlitNTL3:
		inc		esi
		inc		edi
		inc		ebx
		inc		edi
		inc		ebx
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataSubTo16BPPBuffer

	Blits a subrect from a flat 8 bit surface to a 16-bit buffer.

**********************************************************************************************/
BOOLEAN Blt8BPPDataSubTo16BPPBuffer( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVSURFACE hSrcVSurface, UINT8 *pSrcBuffer, UINT32 uiSrcPitch, INT32 iX, INT32 iY, SGPRect *pRect)
{
	UINT16 *p16BPPPalette;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip, LeftSkip, RightSkip, TopSkip, BlitLength, SrcSkip, BlitHeight;
	INT32	 iTempX, iTempY;

	// Assertions
	Assert( hSrcVSurface != NULL );
	Assert( pSrcBuffer != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	usHeight				= (UINT32)hSrcVSurface->usHeight;
	usWidth					= (UINT32)hSrcVSurface->usWidth;

	// Add to start position of dest buffer
	iTempX = iX;
	iTempY = iY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );

	LeftSkip=pRect->iLeft;
	RightSkip=usWidth-pRect->iRight;
	TopSkip=pRect->iTop*uiSrcPitch;
	BlitLength=pRect->iRight-pRect->iLeft;
	BlitHeight=pRect->iBottom-pRect->iTop;
	SrcSkip=uiSrcPitch-BlitLength;

	SrcPtr= (UINT8 *)(pSrcBuffer+TopSkip+LeftSkip);
	DestPtr = ((UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2));
	p16BPPPalette = hSrcVSurface->p16BPPPalette;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr					// pointer to current line start address in source
		mov		edi, DestPtr				// pointer to current line start address in destination
		mov		ebx, BlitHeight			// line counter (goes top to bottom)
		mov		edx, p16BPPPalette	// conversion table

		sub		eax, eax
		sub		ecx, ecx

NewRow:
		mov		ecx, BlitLength			// pixels to blit count

BlitLoop:
		mov		al, [esi]
		xor		ah, ah

		shl		eax, 1							// make it into a word index
		mov		ax, [edx+eax]				// get 16-bit version of 8-bit pixel
		mov		[edi], ax						// store it in destination buffer

		inc		edi
		inc		esi
		inc		edi
		dec		ecx
		jnz		BlitLoop

		add		esi, SrcSkip				// move line pointers down one line
		add		edi, LineSkip

		dec		ebx									// check line counter
		jnz		NewRow							// done blitting, exit

//DoneBlit:											// finished blit
	}

	return( TRUE );

}


/**********************************************************************************************
 Blt8BPPDataTo16BPPBuffer

	Blits from a flat surface to a 16-bit buffer.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBuffer( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVSURFACE hSrcVSurface, UINT8 *pSrcBuffer, INT32 iX, INT32 iY)
{
	UINT16 *p16BPPPalette;
//	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
//	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;
	UINT32 rows;

	// Assertions
	Assert( hSrcVSurface != NULL );
	Assert( pSrcBuffer != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	usHeight				= (UINT32)hSrcVSurface->usHeight;
	usWidth					= (UINT32)hSrcVSurface->usWidth;

	// Add to start position of dest buffer
	iTempX = iX;
	iTempY = iY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)pSrcBuffer;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVSurface->p16BPPPalette;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr					// pointer to current line start address in source
		mov		edi, DestPtr				// pointer to current line start address in destination
		mov		ecx, usHeight				// line counter (goes top to bottom)
		mov		rows, ecx
		mov		edx, p16BPPPalette

		sub		eax, eax
		sub		ecx, ecx

		mov		ebx, usWidth				// column counter (goes right to left)
		dec		ebx

ReadMask:
		test	usWidth, 1
		jz		BlitWord

		xor		eax, eax						// clear out the top 24 bits
		mov		al, [esi+ebx]

		shl		eax, 1							// make it into a word index
		mov		ax, [edx+eax]				// get 16-bit version of 8-bit pixel
		mov		[edi+ebx*2], ax			// store it in destination buffer

		dec		ebx
		js		DoneRow

BlitWord:

		test	usWidth, 2
		jz		SetupDwords


		mov		ax, [esi+ebx-1]
		mov		cl, ah
		sub		ah, ah
		and		ecx, 0ffH
		shl		eax, 1
		shl		ecx, 1
		mov		ax, [edx+eax]
		mov		cx, [edx+ecx]
		shl		ecx, 16
		mov		cx, ax
		mov		[edi+ebx*2-2], ecx

		sub		ebx, 2
		js		DoneRow

SetupDwords:


BlitDwords:

		mov		ax, [esi+ebx-1]
		mov		cl, ah
		sub		ah, ah
		and		ecx, 0ffH
		shl		eax, 1
		shl		ecx, 1
		mov		ax, [edx+eax]
		mov		cx, [edx+ecx]
		shl		ecx, 16
		mov		cx, ax
		mov		[edi+ebx*2-2], ecx

		mov		ax, [esi+ebx-3]
		mov		cl, ah
		sub		ah, ah
		and		ecx, 0ffH
		shl		eax, 1
		shl		ecx, 1
		mov		ax, [edx+eax]
		mov		cx, [edx+ecx]
		shl		ecx, 16
		mov		cx, ax
		mov		[edi+ebx*2-6], ecx

		sub		ebx, 4							// decrement column counter
		jns		BlitDwords					// loop until one line is done

DoneRow:
		dec		rows									// check line counter
		jz		DoneBlit						// done blitting, exit

		add		esi, usWidth			// move line pointers down one line
		add		edi, uiDestPitchBYTES
		mov		ebx, usWidth			// column counter (goes right to left)
		dec		ebx
		jmp		ReadMask

DoneBlit:											// finished blit
	}

	return( TRUE );

}


/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferHalf

	Blits from a flat surface to a 16-bit buffer, dividing the source image into
exactly half the size.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferHalf( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVSURFACE hSrcVSurface, UINT8 *pSrcBuffer, UINT32 uiSrcPitch, INT32 iX, INT32 iY)
{
	UINT16 *p16BPPPalette;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
	INT32	 iTempX, iTempY;
	UINT32 uiSrcSkip;

	// Assertions
	Assert( hSrcVSurface != NULL );
	Assert( pSrcBuffer != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	usHeight				= (UINT32)hSrcVSurface->usHeight;
	usWidth					= (UINT32)hSrcVSurface->usWidth;

	// Add to start position of dest buffer
	iTempX = iX;
	iTempY = iY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );

	SrcPtr= (UINT8 *)pSrcBuffer;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVSurface->p16BPPPalette;
	LineSkip=(uiDestPitchBYTES-(usWidth&0xfffffffe));
	uiSrcSkip=(uiSrcPitch*2)-(usWidth&0xfffffffe);

	__asm {

		mov		esi, SrcPtr					// pointer to current line start address in source
		mov		edi, DestPtr				// pointer to current line start address in destination
		mov		ebx, usHeight				// line counter (goes top to bottom)
		shr		ebx, 1							// half the rows
		mov		edx, p16BPPPalette

		xor		eax, eax

BlitSetup:
		mov		ecx, usWidth
		shr		ecx, 1							// divide the width by 2

ReadMask:
		mov		al, [esi]
		xor		ah, ah
		inc		esi									// skip one source byte
		inc		esi

		shl		eax, 1							// make it into a word index
		mov		ax, [edx+eax]				// get 16-bit version of 8-bit pixel
		mov		[edi], ax						// store it in destination buffer
		inc		edi									// next pixel
		inc		edi

		dec		ecx
		jnz		ReadMask


//DoneRow:

		add		esi, uiSrcSkip			// move source pointer down one line
		add		edi, LineSkip

		dec		ebx									// check line counter
		jnz		BlitSetup						// done blitting, exit

//DoneBlit:											// finished blit
	}

	return( TRUE );

}


/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferHalfRect

	Blits from a flat surface to a 16-bit buffer, dividing the source image into
exactly half the size, from a sub-region.
	- Source rect is in source units.
	- In order to make sure the same pixels are skipped, always align the top and
		left coordinates to the same factor of two.
	- A rect specifying an odd number of pixels will divide out to an even
		number of pixels blitted to the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferHalfRect( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVSURFACE hSrcVSurface, UINT8 *pSrcBuffer, UINT32 uiSrcPitch, INT32 iX, INT32 iY, SGPRect *pRect)
{
	UINT16 *p16BPPPalette;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
	INT32	 iTempX, iTempY;
	UINT32 uiSrcSkip;

	// Assertions
	Assert( hSrcVSurface != NULL );
	Assert( pSrcBuffer != NULL );
	Assert( pBuffer != NULL );
	Assert( pRect != NULL );

	// Get Offsets from Index into structure
	usWidth		= (UINT32)(pRect->iRight-pRect->iLeft);
	usHeight	= (UINT32)(pRect->iBottom-pRect->iTop);

	// Add to start position of dest buffer
	iTempX = iX;
	iTempY = iY;

	// Validations
	CHECKF( iTempX   >= 0 );
	CHECKF( iTempY   >= 0 );
	CHECKF(	usWidth  >  0 );
	CHECKF(	usHeight >  0 );
	CHECKF( usHeight <= hSrcVSurface->usHeight);
	CHECKF( usWidth <= hSrcVSurface->usWidth);

	SrcPtr				= (UINT8 *)pSrcBuffer + (uiSrcPitch*pRect->iTop) + (pRect->iLeft);
	DestPtr				= (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVSurface->p16BPPPalette;
	LineSkip			= (uiDestPitchBYTES-(usWidth&0xfffffffe));
	uiSrcSkip			= (uiSrcPitch*2)-(usWidth&0xfffffffe);

	__asm {

		mov		esi, SrcPtr					// pointer to current line start address in source
		mov		edi, DestPtr				// pointer to current line start address in destination
		mov		ebx, usHeight				// line counter (goes top to bottom)
		shr		ebx, 1							// half the rows
		mov		edx, p16BPPPalette

		xor		eax, eax

BlitSetup:
		mov		ecx, usWidth
		shr		ecx, 1							// divide the width by 2

ReadMask:
		mov		al, [esi]
		xor		ah, ah
		inc		esi									// skip one source byte
		inc		esi

		shl		eax, 1							// make it into a word index
		mov		ax, [edx+eax]				// get 16-bit version of 8-bit pixel
		mov		[edi], ax						// store it in destination buffer
		inc		edi									// next pixel
		inc		edi

		dec		ecx
		jnz		ReadMask


//DoneRow:

		add		esi, uiSrcSkip			// move source pointer down one line
		add		edi, LineSkip

		dec		ebx									// check line counter
		jnz		BlitSetup						// done blitting, exit

//DoneBlit:											// finished blit
	}

	return( TRUE );

}

/****************************INCOMPLETE***********************************************/

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferMask

	Blits an image into the destination buffer, using an ETRLE brush as a source, another ETRLE
	for a mask, and a 16-bit buffer as a destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferMask(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, HVOBJECT hMaskObject, INT32 iMOX, INT32 iMOY, UINT16 usMask)
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 uiMOffset;
	UINT32 usHeight, usWidth;
	UINT32 usMHeight, usMWidth;
	UINT8	 *SrcPtr, *DestPtr, *MaskPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav						= &(hSrcVObject->pETRLEObject[usIndex]);
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Get Offsets from Index into structure for mask
	pTrav						=	&(hMaskObject->pETRLEObject[usMask]);
	usMHeight				= (UINT32)pTrav->usHeight;
	usMWidth				= (UINT32)pTrav->usWidth;
	uiMOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	MaskPtr= (UINT8 *)hMaskObject->pPixData + uiMOffset + (iMOY*usMWidth) + iMOX;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		xor		ebx, ebx
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		bl, [esi]
		mov		ax, [edx+ebx*2]
		mov		[edi], ax

		inc		esi
		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		bl, [esi]
		mov		ax, [edx+ebx*2]
		mov		[edi], ax

		mov		bl, [esi+1]
		mov		ax, [edx+ebx*2]
		mov		[edi+2], ax

		add		esi, 2
		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitDispatch

		xor		ebx, ebx

BlitNTL4:

		mov		bl, [esi]
		mov		ax, [edx+ebx*2]
		mov		[edi], ax

		mov		bl, [esi+1]
		mov		ax, [edx+ebx*2]
		mov		[edi+2], ax

		mov		bl, [esi+2]
		mov		ax, [edx+ebx*2]
		mov		[edi+4], ax

		mov		bl, [esi+3]
		mov		ax, [edx+ebx*2]
		mov		[edi+6], ax

		add		esi, 4
		add		edi, 8
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

void SetClippingRect(SGPRect *clip)
{
	Assert(clip!=NULL);
	Assert(clip->iLeft < clip->iRight);
	Assert(clip->iTop < clip->iBottom);

	memcpy(&ClippingRect, clip, sizeof(SGPRect));

}

void GetClippingRect(SGPRect *clip)
{
	Assert(clip!=NULL);

	memcpy(clip, &ClippingRect, sizeof(SGPRect));
}

/**********************************************************************************************
	Blt16BPPBufferPixelateRectWithColor

		Given an 8x8 pattern and a color, pixelates an area by repeatedly "applying the color" to pixels whereever there
		is a non-zero value in the pattern.

		KM:  Added Nov. 23, 1998
		This is all the code that I moved from Blt16BPPBufferPixelateRect().
		This function now takes a color field (which previously was
		always black.  The 3rd assembler line in this function:

				mov		ax, usColor				// color of pixel

		used to be:

				xor   eax, eax					// color of pixel (black or 0)

	  This was the only internal modification I made other than adding the usColor argument.

*********************************************************************************************/
BOOLEAN Blt16BPPBufferPixelateRectWithColor(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area, UINT8 Pattern[8][8], UINT16 usColor )
{
	INT32  width, height;
	UINT32 LineSkip;
	UINT16 *DestPtr;
	INT32	iLeft, iTop, iRight, iBottom;

	// Assertions
	Assert( pBuffer != NULL );
	Assert( Pattern != NULL );

	iLeft=__max(ClippingRect.iLeft, area->iLeft);
	iTop=__max(ClippingRect.iTop, area->iTop);
	iRight=__min(ClippingRect.iRight-1, area->iRight);
	iBottom=__min(ClippingRect.iBottom-1, area->iBottom);

	DestPtr=(pBuffer+(iTop*(uiDestPitchBYTES/2))+iLeft);
	width=iRight-iLeft+1;
	height=iBottom-iTop+1;
	LineSkip=(uiDestPitchBYTES-(width*2));

	CHECKF(width >=1);
	CHECKF(height >=1);

	__asm {
		mov		esi, Pattern				// Pointer to pixel pattern
		mov		edi, DestPtr				// Pointer to top left of rect area
		mov		ax, usColor				// color of pixel
		xor		ebx, ebx						// pattern column index
		xor		edx, edx						// pattern row index


BlitNewLine:
		mov		ecx, width

BlitLine:
		cmp	[esi+ebx], 0
		je	BlitLine2

		mov		[edi], ax

BlitLine2:
		add		edi, 2
		inc		ebx
		and		ebx, 07H
		or		ebx, edx
		dec		ecx
		jnz		BlitLine

		add		edi, LineSkip
		xor		ebx, ebx
		add		edx, 08H
		and		edx, 38H
		dec		height
		jnz		BlitNewLine
	}

	return(TRUE);
}

//KM:  Modified Nov. 23, 1998
//Original prototype (this function) didn't have a color field.  I've added the color field to
//Blt16BPPBufferPixelateRectWithColor(), moved the previous implementation of this function there, and added
//the modification to allow a specific color.
BOOLEAN Blt16BPPBufferPixelateRect(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area, UINT8 Pattern[8][8] )
{
	return Blt16BPPBufferPixelateRectWithColor( pBuffer, uiDestPitchBYTES, area, Pattern, 0 );
}

/**********************************************************************************************
	Blt16BPPBufferHatchRect

		A wrapper for Blt16BPPBufferPixelateRect(), it automatically sends a hatch pattern to it
		of the specified color

*********************************************************************************************/
BOOLEAN Blt16BPPBufferHatchRectWithColor(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area, UINT16 usColor )
{
	UINT8 Pattern[8][8] =
	{
		1,0,1,0,1,0,1,0,
		0,1,0,1,0,1,0,1,
		1,0,1,0,1,0,1,0,
		0,1,0,1,0,1,0,1,
		1,0,1,0,1,0,1,0,
		0,1,0,1,0,1,0,1,
		1,0,1,0,1,0,1,0,
		0,1,0,1,0,1,0,1
	};
	return Blt16BPPBufferPixelateRectWithColor( pBuffer, uiDestPitchBYTES, area, Pattern, usColor );
}

//Uses black hatch color
BOOLEAN Blt16BPPBufferHatchRect(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area )
{
	UINT8 Pattern[8][8] =
	{
		1,0,1,0,1,0,1,0,
		0,1,0,1,0,1,0,1,
		1,0,1,0,1,0,1,0,
		0,1,0,1,0,1,0,1,
		1,0,1,0,1,0,1,0,
		0,1,0,1,0,1,0,1,
		1,0,1,0,1,0,1,0,
		0,1,0,1,0,1,0,1
	};
	return Blt16BPPBufferPixelateRectWithColor( pBuffer, uiDestPitchBYTES, area, Pattern, 0 );
}

BOOLEAN Blt16BPPBufferLooseHatchRectWithColor(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area, UINT16 usColor )
{
	UINT8 Pattern[8][8] =
	{
		1,0,0,0,1,0,0,0,
		0,0,0,0,0,0,0,0,
		0,0,1,0,0,0,1,0,
		0,0,0,0,0,0,0,0,
		1,0,0,0,1,0,0,0,
		0,0,0,0,0,0,0,0,
		0,0,1,0,0,0,1,0,
		0,0,0,0,0,0,0,0,
	};
	return Blt16BPPBufferPixelateRectWithColor( pBuffer, uiDestPitchBYTES, area, Pattern, usColor );
}

BOOLEAN Blt16BPPBufferLooseHatchRect(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area )
{
	UINT8 Pattern[8][8] =
	{
		1,0,0,0,1,0,0,0,
		0,0,0,0,0,0,0,0,
		0,0,1,0,0,0,1,0,
		0,0,0,0,0,0,0,0,
		1,0,0,0,1,0,0,0,
		0,0,0,0,0,0,0,0,
		0,0,1,0,0,0,1,0,
		0,0,0,0,0,0,0,0,
	};
	return Blt16BPPBufferPixelateRectWithColor( pBuffer, uiDestPitchBYTES, area, Pattern, 0 );
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferShadow

	Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
	image as a mask. Any Non-zero index pixels are used to darken destination pixels.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferShadow( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex)
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, usHeight
		xor		ecx, ecx
		mov		edx, OFFSET ShadeTable

BlitDispatch:


		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		xor		eax, eax

		add		esi, ecx

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitDispatch

BlitNTL4:

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		mov		ax, [edi+4]
		mov		ax, [edx+eax*2]
		mov		[edi+4], ax

		mov		ax, [edi+6]
		mov		ax, [edx+eax*2]
		mov		[edi+6], ax

		add		edi, 8
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		ebx
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}


/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransparent

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination.

**********************************************************************************************/

BOOLEAN Blt8BPPDataTo16BPPBufferTransparent( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		xor		ebx, ebx
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		bl, [esi]
		mov		ax, [edx+ebx*2]
		mov		[edi], ax

		inc		esi
		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		bl, [esi]
		mov		ax, [edx+ebx*2]
		mov		[edi], ax

		mov		bl, [esi+1]
		mov		ax, [edx+ebx*2]
		mov		[edi+2], ax

		add		esi, 2
		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitDispatch

		xor		ebx, ebx

BlitNTL4:

		mov		bl, [esi]
		mov		ax, [edx+ebx*2]
		mov		[edi], ax

		mov		bl, [esi+1]
		mov		ax, [edx+ebx*2]
		mov		[edi+2], ax

		mov		bl, [esi+2]
		mov		ax, [edx+ebx*2]
		mov		[edi+4], ax

		mov		bl, [esi+3]
		mov		ax, [edx+ebx*2]
		mov		[edi+6], ax

		add		esi, 4
		add		edi, 8
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}



//*****************************************************************************************
// Blt8BPPDataTo16BPPBufferTransMirror
//
// Blits an 8bpp ETRLE to a 16-bit buffer, mirroring the image, with transparency.
//
// Returns BOOLEAN            - TRUE if successful
//
//  UINT16 *pBuffer           - 16bpp Destination buffer
// UINT32 uiDestPitchBYTES    - Destination pitch in bytes
// HVOBJECT hSrcVObject       - Source VOBJECT handle
// INT32 iX                   - X-location of blit
// INT32 iY                   - Y-location of blit
// UINT16 usIndex             - VOBJECT image index to blit from
//
// Created:  7/28/99 Derek Beland
//*****************************************************************************************
BOOLEAN Blt8BPPDataTo16BPPBufferTransMirror( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 uiDestSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
//	iTempX = iX + pTrav->sOffsetX;
	iTempX = iX + usWidth - pTrav->sOffsetX-1;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	uiDestSkip=(uiDestPitchBYTES+(usWidth*2));

	__asm {
// esi = pointer to source data
// edi = pointer to destination buffer
// eax = 16bpp pixel
// ebx = 8bpp pixel
// ecx = repeat count
// edx = pointer to 8->16bpp conversion table

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		xor		ebx, ebx
		xor		ecx, ecx

BlitDispatch:

// pick up a new byte
		mov		cl, [esi]
		inc		esi
		or		cl, cl
// if bit 7 is set, the run is transparent
		js		BlitTransparent
// if the byte is zero, it marks the end of current line
		jz		BlitDoneLine

//BlitNonTransLoop:

// else we have a normal run of non-transparent bytes
// blit one byte of the count
		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		bl, [esi]
		mov		ax, [edx+ebx*2]
		mov		[edi], ax

		inc		esi
		sub		edi, 2

// blit one word of the count
BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		bl, [esi]
		mov		ax, [edx+ebx*2]
		mov		[edi], ax

		mov		bl, [esi+1]
		mov		ax, [edx+ebx*2]
		mov		[edi-2], ax

		add		esi, 2
		sub		edi, 4

// blit the rest four at a time (unrolled loop)
BlitNTL3:

		or		cl, cl
		jz		BlitDispatch

		xor		ebx, ebx

BlitNTL4:

		mov		bl, [esi]
		mov		ax, [edx+ebx*2]
		mov		[edi], ax

		mov		bl, [esi+1]
		mov		ax, [edx+ebx*2]
		mov		[edi-2], ax

		mov		bl, [esi+2]
		mov		ax, [edx+ebx*2]
		mov		[edi-4], ax

		mov		bl, [esi+3]
		mov		ax, [edx+ebx*2]
		mov		[edi-6], ax

		add		esi, 4
		sub		edi, 8
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		sub		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, uiDestSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}


/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransparentClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. Clips the brush.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransparentClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, TopSkip
		xor		ecx, ecx

		or		ebx, ebx							// check for nothing clipped on top
		jz		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		ebx
		jnz		TopSkipLoop




LeftSkipSetup:

		mov		Unblitted, 0
		mov		ebx, LeftSkip					// check for nothing clipped on the left
		or		ebx, ebx
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, ebx
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, ebx							// skip partial run, jump into normal loop for rest
		sub		ecx, ebx
		mov		ebx, BlitLength
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		ebx, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, ebx
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, ebx							// skip partial run, jump into normal loop for rest
		mov		ebx, BlitLength
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		ebx, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop




BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		ebx, BlitLength
		mov		Unblitted, 0

BlitDispatch:

		or		ebx, ebx							// Check to see if we're done blitting
		jz		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, ebx
		jbe		BNTrans1

		sub		ecx, ebx
		mov		Unblitted, ecx
		mov		ecx, ebx

BNTrans1:
		sub		ebx, ecx

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		inc		esi
		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		xor		eax, eax
		mov		al, [esi+1]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		add		esi, 2
		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitLineEnd

BlitNTL4:

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		xor		eax, eax
		mov		al, [esi+1]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		xor		eax, eax
		mov		al, [esi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+4], ax

		xor		eax, eax
		mov		al, [esi+3]
		mov		ax, [edx+eax*2]
		mov		[edi+6], ax

		add		esi, 4
		add		edi, 8
		dec		cl
		jnz		BlitNTL4

BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, ebx
		jbe		BTrans1

		mov		ecx, ebx

BTrans1:

		sub		ebx, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}




/**********************************************************************************************
 BltIsClipped

	Determines whether a given blit will need clipping or not. Returns TRUE/FALSE.

**********************************************************************************************/
BOOLEAN BltIsClipped(HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion )
{
	UINT32 usHeight, usWidth;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}


	// Calculate rows hanging off each side of the screen
	if(__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth))
		return(TRUE);

	if(__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth))
		return(TRUE);

	if(__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight))
		return(TRUE);

	if(__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight))
		return(TRUE);

	return(FALSE);
}



/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferShadowClip

	Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
	image as a mask. Any Non-zero index pixels are used to darken destination pixels. Blitter
	clips brush if it doesn't fit on the viewport.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferShadowClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET ShadeTable
		xor		eax, eax
		mov		ebx, TopSkip
		xor		ecx, ecx

		or		ebx, ebx							// check for nothing clipped on top
		jz		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		ebx
		jnz		TopSkipLoop




LeftSkipSetup:

		mov		Unblitted, 0
		mov		ebx, LeftSkip					// check for nothing clipped on the left
		or		ebx, ebx
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, ebx
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, ebx							// skip partial run, jump into normal loop for rest
		sub		ecx, ebx
		mov		ebx, BlitLength
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		ebx, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, ebx
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, ebx							// skip partial run, jump into normal loop for rest
		mov		ebx, BlitLength
		jmp		BlitTransparent


LSTrans1:
		sub		ebx, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop




BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		ebx, BlitLength
		mov		Unblitted, 0

BlitDispatch:

		or		ebx, ebx							// Check to see if we're done blitting
		jz		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:

		cmp		ecx, ebx
		jbe		BNTrans1

		sub		ecx, ebx
		mov		Unblitted, ecx
		mov		ecx, ebx

BNTrans1:
		sub		ebx, ecx

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		inc		esi
		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		add		esi, 2
		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitLineEnd

BlitNTL4:

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		mov		ax, [edi+4]
		mov		ax, [edx+eax*2]
		mov		[edi+4], ax

		mov		ax, [edi+6]
		mov		ax, [edx+eax*2]
		mov		[edi+6], ax

		add		esi, 4
		add		edi, 8
		dec		cl
		jnz		BlitNTL4

BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
		cmp		ecx, ebx
		jbe		BTrans1

		mov		ecx, ebx

BTrans1:

		sub		ebx, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


RightSkipLoop:


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}



/**********************************************************************************************
	Blt16BPPBufferShadowRect

		Darkens a rectangular area by 25%. This blitter is used by ShadowVideoObjectRect.

	pBuffer						Pointer to a 16BPP buffer
	uiDestPitchBytes	Pitch of the destination surface
	area							An SGPRect, the area to darken

*********************************************************************************************/
BOOLEAN Blt16BPPBufferShadowRect(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area)
{
INT32  width, height;
UINT32 LineSkip;
UINT16 *DestPtr;

	// Assertions
	Assert( pBuffer != NULL );

	// Clipping
	if( area->iLeft < ClippingRect.iLeft )
		area->iLeft = ClippingRect.iLeft;
	if( area->iTop < ClippingRect.iTop )
		area->iTop = ClippingRect.iTop;
	if( area->iRight >= ClippingRect.iRight )
		area->iRight = ClippingRect.iRight - 1;
	if( area->iBottom >= ClippingRect.iBottom )
		area->iBottom = ClippingRect.iBottom - 1;
	//CHECKF(area->iLeft >= ClippingRect.iLeft );
	//CHECKF(area->iTop >= ClippingRect.iTop );
	//CHECKF(area->iRight <= ClippingRect.iRight );
	//CHECKF(area->iBottom <= ClippingRect.iBottom );

	DestPtr=(pBuffer+(area->iTop*(uiDestPitchBYTES/2))+area->iLeft);
	width=area->iRight-area->iLeft+1;
	height=area->iBottom-area->iTop+1;
	LineSkip=(uiDestPitchBYTES-(width*2));

	CHECKF(width >=1);
	CHECKF(height >=1);

	__asm {
		mov		esi, OFFSET ShadeTable
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, LineSkip
		mov		edx, height

BlitNewLine:
		mov		ecx, width

BlitLine:
		mov		ax, [edi]
		mov		ax, [esi+eax*2]
		mov		[edi], ax
		add		edi, 2
		dec		ecx
		jnz		BlitLine

		add		edi, ebx
		dec		edx
		jnz		BlitNewLine
}

	return(TRUE);
}


/**********************************************************************************************
	Blt16BPPBufferShadowRect

		Darkens a rectangular area by 25%. This blitter is used by ShadowVideoObjectRect.

	pBuffer						Pointer to a 16BPP buffer
	uiDestPitchBytes	Pitch of the destination surface
	area							An SGPRect, the area to darken

*********************************************************************************************/
BOOLEAN Blt16BPPBufferShadowRectAlternateTable(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area)
{
INT32  width, height;
UINT32 LineSkip;
UINT16 *DestPtr;

	// Assertions
	Assert( pBuffer != NULL );

	// Clipping
	if( area->iLeft < ClippingRect.iLeft )
		area->iLeft = ClippingRect.iLeft;
	if( area->iTop < ClippingRect.iTop )
		area->iTop = ClippingRect.iTop;
	if( area->iRight >= ClippingRect.iRight )
		area->iRight = ClippingRect.iRight - 1;
	if( area->iBottom >= ClippingRect.iBottom )
		area->iBottom = ClippingRect.iBottom - 1;
	//CHECKF(area->iLeft >= ClippingRect.iLeft );
	//CHECKF(area->iTop >= ClippingRect.iTop );
	//CHECKF(area->iRight <= ClippingRect.iRight );
	//CHECKF(area->iBottom <= ClippingRect.iBottom );

	DestPtr=(pBuffer+(area->iTop*(uiDestPitchBYTES/2))+area->iLeft);
	width=area->iRight-area->iLeft+1;
	height=area->iBottom-area->iTop+1;
	LineSkip=(uiDestPitchBYTES-(width*2));

	CHECKF(width >=1);
	CHECKF(height >=1);

	__asm {
		mov		esi, OFFSET IntensityTable
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, LineSkip
		mov		edx, height

BlitNewLine:
		mov		ecx, width

BlitLine:
		mov		ax, [edi]
		mov		ax, [esi+eax*2]
		mov		[edi], ax
		add		edi, 2
		dec		ecx
		jnz		BlitLine

		add		edi, ebx
		dec		edx
		jnz		BlitNewLine
}

	return(TRUE);
}


/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferMonoShadow

	Uses a bitmap an 8BPP template for blitting. Anywhere a 1 appears in the bitmap, a shadow
	is blitted to the destination (a black pixel). Any other value above zero is considered a
	forground color, and zero is background. If the parameter for the background color is zero,
	transparency is used for the background.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferMonoShadow( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT16 usForeground, UINT16 usBackground)
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		xor		ebx, ebx
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		xor		ebx, ebx

BlitNTL4:

		mov		bl, [esi]
		cmp		bl, 1
		jb		BlitNTL5

		// write a black shadow
		xor		ax, ax
		mov		[edi], ax

		inc		esi
		add		edi, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch

BlitNTL5:
		or		bl, bl
		jz		BlitNTL6

		mov		ax, usForeground
		mov		[edi], ax

		inc		esi
		add		edi, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch

BlitNTL6:
		cmp		usBackground, 0
		je		BlitNTL7

		mov		ax, usBackground
		mov		[edi], ax

BlitNTL7:
		inc		esi
		add		edi, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch

BlitTransparent:
		and		ecx, 07fH

		mov		ax, usBackground
		or		ax, ax
		jz		BTrans1

		rep		stosw
		jmp		BlitDispatch

BTrans1:
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/*
BOOLEAN Blt8BPPDataTo16BPPBufferFullTransparent( HVOBJECT hDestVObject, HVOBJECT hSrcVObject, UINT16 usX, UINT16 usY, SGPRect *srcRect )
{
	UINT32 uiSrcStart, uiDestStart, uiNumLines, uiLineSize;
//	UINT32 rows, cols;
	UINT8 *pSrc; //, *pSrcTemp;
	UINT16 *pDest; //*pDestTemp,
	UINT32	uiSrcPitch, uiDestPitch;
	UINT16 *p16BPPPalette;
	UINT16 usEffectiveSrcWidth;
	UINT16 usEffectiveDestWidth;
	UINT16 us16BPPSrcTransColor;
	UINT16 us16BPPDestTransColor;
//	UINT16 us16BPPValue;
	UINT32 count;
	UINT8  maskcolor;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( hDestVObject != NULL );

	// Validations
	CHECKF( usX >= 0 );
	CHECKF( usY >= 0 );
	CHECKF( srcRect->iRight > srcRect->iLeft );
	CHECKF( srcRect->iBottom > srcRect->iTop );

	p16BPPPalette = hSrcVObject->p16BPPPalette;
	CHECKF( p16BPPPalette != NULL );

	// Lock Data
	pSrc = LockVideoObjectBuffer( hSrcVObject, &uiSrcPitch );

	// Effective width ( in PIXELS ) is Pitch ( in bytes ) converted to pitch ( IN PIXELS )
	usEffectiveSrcWidth = (UINT16)( uiSrcPitch / ( hSrcVObject->ubBitDepth / 8 ) );

	pDest = (UINT16*)LockVideoObjectBuffer( hDestVObject, &uiDestPitch );

	// Effective width ( in PIXELS ) is Pitch ( in bytes ) converted to pitch ( IN PIXELS )
	usEffectiveDestWidth = (UINT16)( uiDestPitch / ( hDestVObject->ubBitDepth / 8 ) );

	// Determine memcopy coordinates
	uiSrcStart = srcRect->iTop * usEffectiveSrcWidth + srcRect->iLeft;
	uiDestStart = usY * usEffectiveDestWidth + usX;
	uiNumLines = ( srcRect->iBottom - srcRect->iTop );
	uiLineSize = ( srcRect->iRight - srcRect->iLeft );

	CHECKF( hDestVObject->usWidth >= uiLineSize );
	CHECKF( hDestVObject->usHeight >= uiNumLines );

	// Find 16 BPP transparent color
	us16BPPSrcTransColor = Get16BPPColor( hSrcVObject->TransparentColor );
	for(count=0; (count < 256) && (p16BPPPalette[count]!=us16BPPSrcTransColor); count++);

	if(count==256)
	{
		DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_2, String( "Transparency color does not exist in palette table for source object" ));
		maskcolor=0;
	}
	else
			maskcolor=(UINT8)count;

	us16BPPDestTransColor = Get16BPPColor( hDestVObject->TransparentColor );

	// Convert to Pixel specification
	pDest = pDest + uiDestStart;
	pSrc =  pSrc + uiSrcStart;

	__asm {
		mov		esi, pSrc						// pointer to current line start address in source
		mov		edi, pDest					// pointer to current line start address in destination
		mov		ecx, uiNumLines			// line counter (goes top to bottom)
		mov		edx, p16BPPPalette

		mov		ebx, uiLineSize			// column counter (goes right to left)
		dec		ebx

ReadMask:
		mov		ax, [edi+ebx*2]
		cmp		ax, us16BPPDestTransColor
		je		NextColumn
		xor		eax, eax						// clear out the top 24 bits
		mov		al, [esi+ebx]
		cmp		al, maskcolor
		je		NextColumn

		shl		eax, 1							// make it into a word index
		mov		ax, [edx+eax]				// get 16-bit version of 8-bit pixel
		mov		[edi+ebx*2], ax

NextColumn:
		dec		ebx									// decrement column counter
		jns		ReadMask						// loop until one line is done

		dec		ecx									// check line counter
		jz		DoneBlit						// done blitting, exit

		add		esi, uiSrcPitch			// move line pointers down one line
		add		edi, uiDestPitch
		mov		ebx, uiLineSize			// column counter (goes right to left)
		dec		ebx
		jmp		ReadMask						// back into blitting on next line

DoneBlit:											// finished blit
		}

	ReleaseVideoObjectBuffer( hSrcVObject );
	ReleaseVideoObjectBuffer( hDestVObject );

	return( TRUE );

}	*/




// UTILITY FUNCTIONS FOR BLITTING
/*
BOOLEAN ClipReleatedSrcAndDestRectangles( HVOBJECT hDestVObject, HVOBJECT hSrcVObject, RECT *DestRect, RECT *SrcRect )
{

	Assert( hDestVObject != NULL );
	Assert( hSrcVObject != NULL );

	// Check for invalid start positions and clip by ignoring blit
	if ( DestRect->iLeft >= hDestVObject->usWidth || DestRect->iTop >= hDestVObject->usHeight )
	{
		return( FALSE );
	}

	if ( SrcRect->iLeft >= hSrcVObject->usWidth || SrcRect->iTop >= hSrcVObject->usHeight )
	{
		return( FALSE );
	}

	// For overruns
	// Clip destination rectangles
	if ( DestRect->iRight > hDestVObject->usWidth )
	{
		// Both have to be modified or by default streching occurs
		DestRect->iRight = hDestVObject->usWidth;
		SrcRect->iRight = SrcRect->iLeft + ( DestRect->iRight - DestRect->iLeft );
	}
	if ( DestRect->iBottom > hDestVObject->usHeight )
	{
		// Both have to be modified or by default streching occurs
		DestRect->iBottom = hDestVObject->usHeight;
		SrcRect->iBottom = SrcRect->iTop + ( DestRect->iBottom - DestRect->iTop );
	}

	// Clip src rectangles
	if ( SrcRect->iRight > hSrcVObject->usWidth )
	{
		// Both have to be modified or by default streching occurs
		SrcRect->iRight = hSrcVObject->usWidth;
		DestRect->iRight = DestRect->iLeft  + ( SrcRect->iRight - SrcRect->iLeft );
	}
	if ( SrcRect->iBottom > hSrcVObject->usHeight )
	{
		// Both have to be modified or by default streching occurs
		SrcRect->iBottom = hSrcVObject->usHeight;
		DestRect->iBottom = DestRect->iTop + ( SrcRect->iBottom - SrcRect->iTop );
	}

	// For underruns
	// Clip destination rectangles
	if ( DestRect->iLeft < 0 )
	{
		// Both have to be modified or by default streching occurs
		DestRect->iLeft = 0;
		SrcRect->iLeft = SrcRect->iRight - ( DestRect->iRight - DestRect->iLeft );
	}
	if ( DestRect->iTop < 0 )
	{
		// Both have to be modified or by default streching occurs
		DestRect->iTop = 0;
		SrcRect->iTop = SrcRect->iBottom - ( DestRect->iBottom - DestRect->iTop );
	}

	// Clip src rectangles
	if ( SrcRect->iLeft < 0 )
	{
		// Both have to be modified or by default streching occurs
		SrcRect->iLeft = 0;
		DestRect->iLeft = DestRect->iRight  - ( SrcRect->iRight - SrcRect->iLeft );
	}
	if ( SrcRect->iTop < 0 )
	{
		// Both have to be modified or by default streching occurs
		SrcRect->iTop = 0;
		DestRect->iTop = DestRect->iBottom - ( SrcRect->iBottom - SrcRect->iTop );
	}

	return( TRUE );
}


BOOLEAN FillSurface( HVOBJECT hDestVObject, blt_fx *pBltFx )
{
	DDBLTFX				 BlitterFX;

	Assert( hDestVObject != NULL );
	CHECKF( pBltFx != NULL );

	BlitterFX.dwSize = sizeof( DDBLTFX );
	BlitterFX.dwFillColor = pBltFx->ColorFill;

	DDBltSurface( (LPDIRECTDRAWSURFACE2)hDestVObject->pSurfaceData, NULL, NULL, NULL, DDBLT_COLORFILL, &BlitterFX );

	if ( hDestVObject->fFlags & VOBJECT_VIDEO_MEM_USAGE && !hDestVObject->fFlags & VOBJECT_RESERVED_SURFACE )
	{
		UpdateBackupSurface( hDestVObject );
	}

	return( TRUE );
}

BOOLEAN FillSurfaceRect( HVOBJECT hDestVObject, blt_fx *pBltFx )
{
	DDBLTFX				 BlitterFX;

	Assert( hDestVObject != NULL );
	CHECKF( pBltFx != NULL );

	BlitterFX.dwSize = sizeof( DDBLTFX );
	BlitterFX.dwFillColor = pBltFx->ColorFill;

	DDBltSurface( (LPDIRECTDRAWSURFACE2)hDestVObject->pSurfaceData, (LPRECT)&(pBltFx->FillRect), NULL, NULL, DDBLT_COLORFILL, &BlitterFX );

	if ( hDestVObject->fFlags & VOBJECT_VIDEO_MEM_USAGE && !hDestVObject->fFlags & VOBJECT_RESERVED_SURFACE )
	{
		UpdateBackupSurface( hDestVObject );
	}

	return( TRUE );
}


BOOLEAN BltVObjectUsingDD( HVOBJECT hDestVObject, HVOBJECT hSrcVObject, UINT32 fBltFlags, INT32 iDestX, INT32 iDestY, RECT *SrcRect )
{
	UINT32		uiDDFlags;
	RECT			DestRect;

  // Blit using the correct blitter
	if ( fBltFlags & VO_BLT_FAST )
	{

		// Validations
		CHECKF( iDestX >= 0 );
		CHECKF( iDestY >= 0 );

		// Default flags
		uiDDFlags = 0;

		// Convert flags into DD flags, ( for transparency use, etc )
		if ( fBltFlags & VO_BLT_USECOLORKEY )
		{
			uiDDFlags != DDBLTFAST_SRCCOLORKEY;
		}

		// Convert flags into DD flags, ( for transparency use, etc )
		if ( fBltFlags & VO_BLT_USEDESTCOLORKEY )
		{
			uiDDFlags != DDBLTFAST_DESTCOLORKEY;
		}

		if ( uiDDFlags == 0 )
		{
			// Default here is no colorkey
			uiDDFlags = DDBLTFAST_NOCOLORKEY;
		}

		DDBltFastSurface( (LPDIRECTDRAWSURFACE2)hDestVObject->pSurfaceData, iDestX, iDestY, (LPDIRECTDRAWSURFACE2)hSrcVObject->pSurfaceData, SrcRect, uiDDFlags );

	}
	else
	{
		// Normal, specialized blit for clipping, etc

		// Default flags
		uiDDFlags = DDBLT_WAIT;

		// Convert flags into DD flags, ( for transparency use, etc )
		if ( fBltFlags & VO_BLT_USECOLORKEY )
		{
			uiDDFlags |= DDBLT_KEYSRC;
		}

		// Setup dest rectangle
		DestRect.top =  (int)iDestY;
		DestRect.left = (int)iDestX;
		DestRect.bottom = (int)iDestY + ( SrcRect->iBottom - SrcRect->iTop );
		DestRect.right = (int)iDestX + ( SrcRect->iRight - SrcRect->iLeft );

		// Do Clipping of rectangles
		if ( !ClipReleatedSrcAndDestRectangles( hDestVObject, hSrcVObject, &DestRect, SrcRect ) )
		{
			// Returns false because dest start is > dest size
			return( TRUE );
		}

		DDBltSurface( (LPDIRECTDRAWSURFACE2)hDestVObject->pSurfaceData, &DestRect, (LPDIRECTDRAWSURFACE2)hSrcVObject->pSurfaceData,
							SrcRect, uiDDFlags, NULL );

	}

	// Update backup surface with new data
	if ( hDestVObject->fFlags & VOBJECT_VIDEO_MEM_USAGE && !hDestVObject->fFlags & VOBJECT_RESERVED_SURFACE )
	{
		UpdateBackupSurface( hDestVObject );
	}

	return( TRUE );
}


// Blt to backup buffer
BOOLEAN UpdateBackupSurface( HVOBJECT hVObject )
{
	RECT		aRect;

	// Assertions
	Assert( hVObject != NULL );

	// Validations
	CHECKF( hVObject->pSavedSurfaceData != NULL );

	aRect.top = (int)0;
	aRect.left = (int)0;
	aRect.bottom = (int)hVObject->usHeight;
	aRect.right = (int)hVObject->usWidth;

	// Copy all contents into backup buffer
	DDBltFastSurface( (LPDIRECTDRAWSURFACE2)hVObject->pSurfaceData, 0, 0, (LPDIRECTDRAWSURFACE2)hVObject->pSavedSurfaceData, &aRect, DDBLTFAST_NOCOLORKEY );

	return( TRUE );

}

*/


BOOLEAN FillRect8BPP(UINT8 *pBuffer, UINT32 uiDestPitchBYTES, INT32 x1, INT32 y1, INT32 x2, INT32 y2, UINT8 color)
{
INT32		x1real, y1real, x2real, y2real;
UINT32	linelength, lines, lineskip;
UINT8		*startoffset;

	// check parameters
	Assert(pBuffer!=NULL);
	Assert(uiDestPitchBYTES > 0);
	Assert(x2 > x1);
	Assert(y2 > y1);

	// clip edges of rect if hanging off screen

	x1real=__max(0, x1);
	x2real=__min(639, x2);
	y1real=__max(0, y1);
	y2real=__min(479, y2);

	startoffset=pBuffer+(y1real*uiDestPitchBYTES)+x1real;
	lines=y2real-y1real+1;
	linelength=x2real-x1real+1;
	lineskip=uiDestPitchBYTES-linelength;

	__asm {
		mov		edi, startoffset
		mov		al, color
		mov		ah, al
		shl		eax, 16
		mov		al, color
		mov		ah, al
		mov		edx, lines
		mov		ebx, linelength

// edi = destination pointer
// eax = dword of color value
// ebx = line length
// ecx = column counter
// edx = row counter

LineLoop:

		mov		ecx, ebx

		clc
		rcr		ecx, 1
		jnc		FL2

		mov		[edi], al
		inc		edi

FL2:
		clc
		rcr		ecx, 1
		jnc		FL3
		mov		[edi], ax
		add		edi, 2

FL3:
		or		ecx, ecx
		jz		FillLineEnd

		rep		stosd

FillLineEnd:
		add		edi, lineskip
		dec		edx
		jnz		LineLoop

	}
	return(TRUE);
}

BOOLEAN FillRect16BPP(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, INT32 x1, INT32 y1, INT32 x2, INT32 y2, UINT16 color)
{
INT32		x1real, y1real, x2real, y2real;
UINT32	linelength, lines, lineskip;
UINT16		*startoffset;

	// check parameters
	Assert(pBuffer!=NULL);
	Assert(uiDestPitchBYTES > 0);
	Assert(x2 > x1);
	Assert(y2 > y1);

	// clip edges of rect if hanging off screen

	x1real=__max(0, x1);
	x2real=__min(639, x2);
	y1real=__max(0, y1);
	y2real=__min(479, y2);

	startoffset=pBuffer+(y1real*uiDestPitchBYTES/2)+x1real;
	lines=y2real-y1real+1;
	linelength=x2real-x1real+1;
	lineskip=uiDestPitchBYTES-(linelength*2);

	__asm {
		mov		edi, startoffset
		mov		ax, color
		shl		eax, 16
		mov		ax, color
		mov		edx, lines
		mov		ebx, linelength

// edi = destination pointer
// eax = dword of color value
// ebx = line length
// ecx = column counter
// edx = row counter

LineLoop:
		mov		ecx, ebx

		clc
		rcr		ecx, 1
		jnc		FL2

		mov		[edi], ax
		add		edi, 2

FL2:
		or		ecx, ecx
		jz		FillLineEnd

		rep		stosd

FillLineEnd:
		add		edi, lineskip
		dec		edx
		jnz		LineLoop

	}
	return(TRUE);
}


/**********************************************************************************************
 BltIsClippedOrOffScreen

	Determines whether a given blit will need clipping or not. Returns TRUE/FALSE.

**********************************************************************************************/
CHAR8 BltIsClippedOrOffScreen( HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion )
{
	UINT32 usHeight, usWidth;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}


	// Calculate rows hanging off each side of the screen
	gLeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	gRightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	gTopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	gBottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	gfUsePreCalcSkips = TRUE;

	// check if whole thing is clipped
	if((gLeftSkip >=(INT32)usWidth) || (gRightSkip >=(INT32)usWidth))
		return(-1 );

	// check if whole thing is clipped
	if((gTopSkip >=(INT32)usHeight) || (gBottomSkip >=(INT32)usHeight))
		return(-1 );


	if ( gLeftSkip )
		return( TRUE );

	if ( gRightSkip )
		return( TRUE );

	if ( gTopSkip )
		return( TRUE );

	if ( gBottomSkip )
		return( TRUE );


	return(FALSE);
}




// Blt8BPPDataTo16BPPBufferOutline
// ATE New blitter for rendering a differrent color for value 254. Can be transparent if fDoOutline is FALSE
BOOLEAN Blt8BPPDataTo16BPPBufferOutline( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;
	UINT16 *p16BPPPalette;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	LineSkip=(uiDestPitchBYTES-(usWidth*2));
	p16BPPPalette = hSrcVObject->pShadeCurrent;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL6

		//		DO OUTLINE
		//		ONLY IF WE WANT IT!
		mov		al, fDoOutline;
		cmp		al,	1
		jne		BlitNTL5

		mov		ax, s16BPPColor
		mov		[edi], ax
		jmp		BlitNTL5


BlitNTL6:
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}


// ATE New blitter for rendering a differrent color for value 254. Can be transparent if fDoOutline is FALSE
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline, SGPRect *clipregion )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	UINT16 *p16BPPPalette;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));
	p16BPPPalette = hSrcVObject->pShadeCurrent;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:
		xor		eax, eax

		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL3

		//		DO OUTLINE
		//		ONLY IF WE WANT IT!
		mov		al, fDoOutline;
		cmp		al,	1
		jne		BlitNTL2

		mov		ax, s16BPPColor
		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL3:
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}


BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline, SGPRect *clipregion )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	UINT16 *p16BPPPalette;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);

	LineSkip=(uiDestPitchBYTES-(BlitLength*2));
	p16BPPPalette = hSrcVObject->pShadeCurrent;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, usZValue
		cmp		ax, [ebx]
		jb		BlitNTL2

		// CHECK FOR OUTLINE...
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL3

		//		DO OUTLINE
		//		ONLY IF WE WANT IT!
		mov		al, fDoOutline;
		cmp		al,	1
		jne		BlitNTL2

		mov		ax, s16BPPColor
		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL3:

		//Write to z-buffer
		mov		[ebx], ax

		xor		eax, eax

		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		inc		ebx
		inc		ebx
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}


BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZPixelateObscuredClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline, SGPRect *clipregion )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	UINT16 *p16BPPPalette;
	UINT32 uiLineFlag;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);

	LineSkip=(uiDestPitchBYTES-(BlitLength*2));
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	uiLineFlag=(iTempY&1);


	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, usZValue
		cmp		ax, [ebx]
		jb		BlitNTL8

		// Write it now!
		jmp BlitNTL7

BlitNTL8:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL2
		jmp		BlitNTL9


BlitNTL6:

		test	edi, 2
		jnz		BlitNTL2

BlitNTL7:

		mov		[ebx], ax

BlitNTL9:

		// CHECK FOR OUTLINE...
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL3

		//		DO OUTLINE
		//		ONLY IF WE WANT IT!
		mov		al, fDoOutline;
		cmp		al,	1
		jne		BlitNTL2

		mov		ax, s16BPPColor
		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL3:

		//Write to z-buffer
		mov		[ebx], ax

		xor		eax, eax

		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		inc		ebx
		inc		ebx
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		xor		uiLineFlag, 1

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}



BOOLEAN Blt8BPPDataTo16BPPBufferOutlineShadow( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;
	UINT16 *p16BPPPalette;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	LineSkip=(uiDestPitchBYTES-(usWidth*2));
	p16BPPPalette = hSrcVObject->pShadeCurrent;

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		je		BlitNTL5

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax


BlitNTL5:
		inc		esi
		add		edi, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}



BOOLEAN Blt8BPPDataTo16BPPBufferOutlineShadowClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion )
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET ShadeTable
		xor		eax, eax
		mov		ebx, TopSkip
		xor		ecx, ecx

		or		ebx, ebx							// check for nothing clipped on top
		jz		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine


// Check for outline as well
		mov		cl, [esi]
		cmp		cl, 254
		je		TopSkipLoop
//

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		ebx
		jnz		TopSkipLoop




LeftSkipSetup:

		mov		Unblitted, 0
		mov		ebx, LeftSkip					// check for nothing clipped on the left
		or		ebx, ebx
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, ebx
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, ebx							// skip partial run, jump into normal loop for rest
		sub		ecx, ebx
		mov		ebx, BlitLength
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		ebx, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, ebx
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, ebx							// skip partial run, jump into normal loop for rest
		mov		ebx, BlitLength
		jmp		BlitTransparent


LSTrans1:
		sub		ebx, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop




BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		ebx, BlitLength
		mov		Unblitted, 0

BlitDispatch:

		or		ebx, ebx							// Check to see if we're done blitting
		jz		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:

		cmp		ecx, ebx
		jbe		BNTrans1

		sub		ecx, ebx
		mov		Unblitted, ecx
		mov		ecx, ebx

BNTrans1:
		sub		ebx, ecx

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		inc		esi
		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		add		esi, 2
		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitLineEnd

BlitNTL4:

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		mov		ax, [edi+4]
		mov		ax, [edx+eax*2]
		mov		[edi+4], ax

		mov		ax, [edi+6]
		mov		ax, [edx+eax*2]
		mov		[edi+6], ax

		add		esi, 4
		add		edi, 8
		dec		cl
		jnz		BlitNTL4

BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
		cmp		ecx, ebx
		jbe		BTrans1

		mov		ecx, ebx

BTrans1:

		sub		ebx, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


RightSkipLoop:


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}


BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline )
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		xor		eax, eax

BlitNTL4:

		mov		ax, usZValue
		cmp		ax, [ebx]
		jb		BlitNTL5

		// CHECK FOR OUTLINE, BLIT DIFFERENTLY IF WE WANT IT TO!
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL6

		//		DO OUTLINE
		//		ONLY IF WE WANT IT!
		mov		al, fDoOutline;
		cmp		al,	1
		jne		BlitNTL5

		mov		ax, s16BPPColor
		mov		[edi], ax
		jmp		BlitNTL5

BlitNTL6:

		//Donot write to z-buffer
		mov		[ebx], ax

		xor		ah, ah
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		inc		edi
		inc		ebx
		inc		edi
		inc		ebx

		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}


BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZPixelateObscured( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline )
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;
	UINT32 uiLineFlag;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));
	uiLineFlag=(iTempY&1);

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		xor		eax, eax

BlitNTL4:

		mov		ax, usZValue
		cmp		ax, [ebx]
		jbe		BlitNTL8

		// Write it now!
		jmp BlitNTL7

BlitNTL8:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL5
		jmp		BlitNTL9


BlitNTL6:

		test	edi, 2
		jnz		BlitNTL5

BlitNTL7:

		mov		[ebx], ax

BlitNTL9:

		// CHECK FOR OUTLINE, BLIT DIFFERENTLY IF WE WANT IT TO!
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL12

		//		DO OUTLINE
		//		ONLY IF WE WANT IT!
		mov		al, fDoOutline;
		cmp		al,	1
		jne		BlitNTL5

		mov		ax, s16BPPColor
		mov		[edi], ax
		jmp		BlitNTL5

BlitNTL12:

		xor		ah, ah
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		inc		edi
		inc		ebx
		inc		edi
		inc		ebx

		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		xor		uiLineFlag, 1
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}


// This is the same as above, but DONOT WRITE to Z!
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline )
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		xor		eax, eax

BlitNTL4:

		mov		ax, usZValue
		cmp		ax, [ebx]
		jb		BlitNTL5

		// CHECK FOR OUTLINE, BLIT DIFFERENTLY IF WE WANT IT TO!
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL6

		//		DO OUTLINE
		//		ONLY IF WE WANT IT!
		mov		al, fDoOutline;
		cmp		al,	1
		jne		BlitNTL5

		mov		ax, s16BPPColor
		mov		[edi], ax
		jmp		BlitNTL5

BlitNTL6:

		//Donot write to z-buffer
		//mov		[ebx], ax

		xor		ah, ah
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		inc		edi
		inc		ebx
		inc		edi
		inc		ebx

		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}


/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferIntensityZ

	Creates a shadow using a brush, but modifies the destination buffer only if the current
	Z level is equal to higher than what's in the Z buffer at that pixel location. It
	updates the Z buffer with the new Z level.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET IntensityTable
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		xor		eax, eax
		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax
		mov		ax, usZValue
		mov		[ebx], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferIntensityZClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET IntensityTable
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferIntensityZNB

	Creates a shadow using a brush, but modifies the destination buffer only if the current
	Z level is equal to higher than what's in the Z buffer at that pixel location. It does
	NOT update the Z buffer with the new Z value.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
	ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;


	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET IntensityTable
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		xor		eax, eax
		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferIntensityZNBClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, the Z value is
	not updated,	for any non-transparent pixels. The Z-buffer is 16 bit, and	must be the
	same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr, *ZPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET IntensityTable
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		xor		eax, eax

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}



/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferIntensityClip

	Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
	image as a mask. Any Non-zero index pixels are used to darken destination pixels. Blitter
	clips brush if it doesn't fit on the viewport.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth, Unblitted;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET IntensityTable
		xor		eax, eax
		mov		ebx, TopSkip
		xor		ecx, ecx

		or		ebx, ebx							// check for nothing clipped on top
		jz		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		ebx
		jnz		TopSkipLoop




LeftSkipSetup:

		mov		Unblitted, 0
		mov		ebx, LeftSkip					// check for nothing clipped on the left
		or		ebx, ebx
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, ebx
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, ebx							// skip partial run, jump into normal loop for rest
		sub		ecx, ebx
		mov		ebx, BlitLength
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		ebx, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, ebx
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, ebx							// skip partial run, jump into normal loop for rest
		mov		ebx, BlitLength
		jmp		BlitTransparent


LSTrans1:
		sub		ebx, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop




BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		ebx, BlitLength
		mov		Unblitted, 0

BlitDispatch:

		or		ebx, ebx							// Check to see if we're done blitting
		jz		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:

		cmp		ecx, ebx
		jbe		BNTrans1

		sub		ecx, ebx
		mov		Unblitted, ecx
		mov		ecx, ebx

BNTrans1:
		sub		ebx, ecx

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		inc		esi
		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		add		esi, 2
		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitLineEnd

BlitNTL4:

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		mov		ax, [edi+4]
		mov		ax, [edx+eax*2]
		mov		[edi+4], ax

		mov		ax, [edi+6]
		mov		ax, [edx+eax*2]
		mov		[edi+6], ax

		add		esi, 4
		add		edi, 8
		dec		cl
		jnz		BlitNTL4

BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
		cmp		ecx, ebx
		jbe		BTrans1

		mov		ecx, ebx

BTrans1:

		sub		ebx, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


RightSkipLoop:


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}


/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferIntensity

	Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
	image as a mask. Any Non-zero index pixels are used to darken destination pixels.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferIntensity( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex)
{
	UINT16 *p16BPPPalette;
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	UINT8	 *SrcPtr, *DestPtr;
	UINT32 LineSkip;
  ETRLEObject *pTrav;
	INT32	 iTempX, iTempY;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	// Validations
	CHECKF( iTempX >= 0 );
	CHECKF( iTempY >= 0 );


	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, usHeight
		xor		ecx, ecx
		mov		edx, OFFSET IntensityTable

BlitDispatch:


		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		xor		eax, eax

		add		esi, ecx

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitDispatch

BlitNTL4:

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		mov		ax, [edi+4]
		mov		ax, [edx+eax*2]
		mov		[edi+4], ax

		mov		ax, [edi+6]
		mov		ax, [edx+eax*2]
		mov		[edi+6], ax

		add		edi, 8
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		ebx
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
	}

	return(TRUE);

}


/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZClipPixelateObscured

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	NOT updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

	Blits every second pixel ("pixelates").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZClipPixelateObscured( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
UINT16 *p16BPPPalette;
UINT32 uiOffset, uiLineFlag;
UINT32 usHeight, usWidth, Unblitted;
UINT8	 *SrcPtr, *DestPtr, *ZPtr;
UINT32 LineSkip;
ETRLEObject *pTrav;
INT32	 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ usIndex ] );
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;

	// Add to start position of dest buffer
	iTempX = iX + pTrav->sOffsetX;
	iTempY = iY + pTrav->sOffsetY;

	if(clipregion==NULL)
	{
		ClipX1=ClippingRect.iLeft;
		ClipY1=ClippingRect.iTop;
		ClipX2=ClippingRect.iRight;
		ClipY2=ClippingRect.iBottom;
	}
	else
	{
		ClipX1=clipregion->iLeft;
		ClipY1=clipregion->iTop;
		ClipX2=clipregion->iRight;
		ClipY2=clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	LeftSkip=__min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip=__min(max(ClipX2, (iTempX+(INT32)usWidth)) - ClipX2, (INT32)usWidth);
	TopSkip=__min(ClipY1 - __min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip=__min(__max(ClipY2, (iTempY+(INT32)usHeight)) - ClipY2, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return(TRUE);

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return(TRUE);

	SrcPtr= (UINT8 *)hSrcVObject->pPixData + uiOffset;
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	p16BPPPalette = hSrcVObject->pShadeCurrent;
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));
	uiLineFlag=(iTempY&1);

	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0							// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:										// Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		xor		uiLineFlag, 1
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2								// if equal, skip whole, and start blit with new run
		jb		LSSkip1								// if less, skip whole thing

		add		esi, LSCount							// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx							// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx							// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup					// if equal, skip whole, and start blit with new run
		jb		LSTrans1							// if less, skip whole thing

		sub		ecx, LSCount							// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx							// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:									// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0							// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:								// blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		// OK, DO CHECK FOR Z FIRST!
		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL8

		// ONLY WRITE DATA IF WE REALLY SHOULD
		mov		ax, usZValue
		mov		[ebx], ax
		jmp   BlitNTL7

BlitNTL8:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL2
		jmp		BlitNTL7

BlitNTL6:
		test	edi, 2
		jnz		BlitNTL2

BlitNTL7:

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

//BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:											// skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
//		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:												// skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		xor		uiLineFlag, 1
		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
	}

	return(TRUE);

}
