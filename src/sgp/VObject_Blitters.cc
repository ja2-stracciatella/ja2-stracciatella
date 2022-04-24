#include <stdint.h>
#include "Debug.h"
#include "HImage.h"
#include "Local.h"
#include "MemMan.h"
#include "Shading.h"
#include "VObject.h"
#include "VObject_Blitters.h"
#include "VSurface.h"
#include "WCheck.h"


SGPRect	ClippingRect;
							//555      565
UINT32	guiTranslucentMask=0x3def; //0x7bef;		// mask for halving 5,6,5


/*
 * Difference or Zero
 * NB: the types of the parameters are int32_t to ensure the compiler can
 * safely promote 16 bit unsigned integers.
 */
static uint16_t DoZ(int32_t const a, int32_t const b)
{
	return a > b ? static_cast<uint16_t>(a - b) : 0;
}


/* Blit an image into the destination buffer, using an ETRLE brush as a source,
 * and a 16-bit buffer as a destination. As it is blitting, it checks the Z
 * value of the ZBuffer, and if the pixel's Z level is below that of the current
 * pixel, it is written on, and the Z value is NOT updated to the current value,
 * for any non-transparent pixels. The Z-buffer is 16 bit, and must be the same
 * dimensions (including pitch) as the destination.
 * Blits every second pixel ("Translucents"). */
void Blt8BPPDataTo16BPPBufferTransZNBClipTranslucent(UINT16* const buf, UINT32 const uiDestPitchBYTES, UINT16* const zbuf, UINT16 const zval, HVOBJECT const hSrcVObject, INT32 const iX, INT32 const iY, UINT16 const usIndex, SGPRect const* clipregion)
{
	Assert(hSrcVObject);
	Assert(buf);

	// Get offsets from index into structure.
	ETRLEObject const& e      = hSrcVObject->SubregionProperties(usIndex);
	int32_t      const height = e.usHeight; // NB: safe automatic conversion
	int32_t      const width  = e.usWidth;

	// Add to start position of dest buffer.
	int32_t const x = iX + e.sOffsetX;
	int32_t const y = iY + e.sOffsetY;

	if (!clipregion) clipregion = &ClippingRect;

	/* Calculate rows hanging off each side of the screen and check if the whole
	 * thing is clipped. */
	UINT16 const left_skip   = DoZ(clipregion->iLeft, x);
	if (left_skip   >= width)  return;
	UINT16       top_skip    = DoZ(clipregion->iTop,  y);
	if (top_skip    >= height) return;
	UINT16 const right_skip  = DoZ(x + width,  clipregion->iRight);
	if (right_skip  >= width)  return;
	UINT16 const bottom_skip = DoZ(y + height, clipregion->iBottom);
	if (bottom_skip >= height) return;

	// Calculate the remaining rows and columns to blit.
	INT32 const blit_length = width  - left_skip - right_skip;
	INT32       blit_height = height - top_skip  - bottom_skip;

	UINT32         const pitch     = uiDestPitchBYTES / 2;
	UINT8   const*       src       = hSrcVObject->PixData(e);
	UINT16*              dst       = buf  + pitch * (y + top_skip) + (x + left_skip);
	UINT16  const*       zdst      = zbuf + pitch * (y + top_skip) + (x + left_skip);
	UINT16  const* const pal       = hSrcVObject->CurrentShade();
	UINT32               line_skip = pitch - blit_length;

	for (; top_skip > 0; --top_skip)
	{
		for (;;)
		{
			UINT32 const px_count = *src++;
			if (px_count & 0x80) continue;
			if (px_count == 0)   break;
			src += px_count;
		}
	}

	UINT32 const translucent_mask = guiTranslucentMask;
	do
	{
		INT32  ls_count;
		UINT32 px_count;
		for (ls_count = left_skip; ls_count > 0; ls_count -= px_count)
		{
			px_count = *src++;
			if (px_count & 0x80)
			{
				px_count &= 0x7F;
				if (px_count > static_cast<UINT32>(ls_count))
				{
					px_count -= ls_count;
					ls_count  = blit_length;
					goto BlitTransparent;
				}
			}
			else
			{
				if (px_count > static_cast<UINT32>(ls_count))
				{
					src      += ls_count;
					px_count -= ls_count;
					ls_count  = blit_length;
					goto BlitNonTransLoop;
				}
				src += px_count;
			}
		}

		ls_count = blit_length;
		while (ls_count > 0)
		{
			px_count = *src++;
			if (px_count & 0x80)
			{ // Skip transparent pixels.
				px_count &= 0x7F;
BlitTransparent:
				if (px_count > static_cast<UINT32>(ls_count)) px_count = ls_count;
				ls_count -= px_count;
				dst      += px_count;
				zdst     += px_count;
			}
			else
			{ // Blit non-transparent pixels.
BlitNonTransLoop:
				UINT32 unblitted = 0;
				if (px_count > static_cast<UINT32>(ls_count))
				{
					unblitted = px_count - ls_count;
					px_count  = ls_count;
				}
				ls_count -= px_count;

				do
				{
					if (*zdst > zval) continue;
					*dst =
						(pal[*src] >> 1 & translucent_mask) +
						(*dst      >> 1 & translucent_mask);
				}
				while (++src, ++dst, ++zdst, --px_count > 0);
				src += unblitted;
			}
		}

		while (*src++ != 0) {} // Skip along until we hit and end-of-line marker.
		dst  += line_skip;
		zdst += line_skip;
	}
	while (--blit_height > 0);
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
void Blt8BPPDataTo16BPPBufferTransZTranslucent( UINT16* const buf, UINT32 const uiDestPitchBYTES, UINT16* const zbuf, UINT16 const zval, HVOBJECT const hSrcVObject, INT32 const iX, INT32 const iY, UINT16 const usIndex )
{

	// Assertions
	Assert( hSrcVObject );
	Assert( buf );

	// Get Offsets from Index into structure
	ETRLEObject const& e      = hSrcVObject->SubregionProperties(usIndex);
	UINT32             height = e.usHeight;
	UINT32      const  width  = e.usWidth;

	// Add to start position of dest buffer
	INT32 const x = iX + e.sOffsetX;
	INT32 const y = iY + e.sOffsetY;

	// Validations
	CHECKV(x >= 0);
	CHECKV(y >= 0);

	UINT32 const  pitch           = uiDestPitchBYTES / 2;
	UINT8  const* src             = hSrcVObject->PixData(e);
	UINT16 * dst                  = buf + pitch * y + x;
	UINT16 * zdst                 = zbuf + pitch * y + x;
	UINT16 const* const pal       = hSrcVObject->CurrentShade();
	UINT32 line_skip              = pitch - width;
	UINT32 const translucent_mask = guiTranslucentMask;

	do
	{
		for (;;)
		{
			UINT8 data = *src++;

			if (data == 0) break;
			if (data & 0x80)
			{
				data 		&= 0x7F;
				dst 	+= data;
				zdst	+= data;
			}
			else
			{
				do
				{
					if (*zdst > zval) continue;
					*zdst = zval;
					*dst =
						((pal[*src] >> 1) & translucent_mask) +
						((*dst      >> 1) & translucent_mask);
				}
				while (++src, ++dst, ++zdst, --data > 0);
			}
		}
		dst 	+= line_skip;
		zdst	+= line_skip;
	}
	while (--height > 0);
}


/* Blit an image into the destination buffer, using an ETRLE brush as a source,
 * and a 16-bit buffer as a destination. As it is blitting, it checks the Z
 * value of the ZBuffer, and if the pixel's Z level is below that of the current
 * pixel, it is written on, and the Z value is NOT updated to the current value,
 * for any non-transparent pixels. The Z-buffer is 16 bit, and must be the same
 * dimensions (including pitch) as the destination.
 * Blits every second pixel ("Translucents"). */
void Blt8BPPDataTo16BPPBufferTransZNBTranslucent(UINT16* const buf, UINT32 const uiDestPitchBYTES, UINT16* const zbuf, UINT16 const zval, HVOBJECT const hSrcVObject, INT32 const iX, INT32 const iY, UINT16 const usIndex)
{
	Assert(hSrcVObject);
	Assert(buf);

	// Get offsets from index into structure.
	ETRLEObject const& e      = hSrcVObject->SubregionProperties(usIndex);
	UINT32             height = e.usHeight;
	UINT32      const  width  = e.usWidth;

	// Add to start position of dest buffer.
	INT32 const x = iX + e.sOffsetX;
	INT32 const y = iY + e.sOffsetY;

	CHECKV(x >= 0);
	CHECKV(y >= 0);

	UINT32         const pitch     = uiDestPitchBYTES / 2;
	UINT8   const*       src       = hSrcVObject->PixData(e);
	UINT16*              dst       = buf  + pitch * y + x;
	UINT16  const*       zdst      = zbuf + pitch * y + x;
	UINT16  const* const pal       = hSrcVObject->CurrentShade();
	UINT32               line_skip = pitch - width;

	UINT32 const translucent_mask = guiTranslucentMask;
	do
	{
		for (;;)
		{
			UINT8 data = *src++;

			if (data == 0) break;
			if (data & 0x80)
			{
				data &= 0x7F;
				dst  += data;
				zdst += data;
			}
			else
			{
				do
				{
					if (*zdst > zval) continue;
					*dst =
						((pal[*src] >> 1) & translucent_mask) +
						((*dst      >> 1) & translucent_mask);
				}
				while (++src, ++dst, ++zdst, --data > 0);
			}
		}
		dst  += line_skip;
		zdst += line_skip;
	}
	while (--height > 0);
}


UINT16* InitZBuffer(const UINT32 width, const UINT32 height)
{
	return new UINT16[width * height]{};
}


void ShutdownZBuffer(UINT16* const pBuffer)
{
	delete[] pBuffer;
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferMonoShadowClip

	Uses a bitmap an 8BPP template for blitting. Anywhere a 1 appears in the bitmap, a shadow
	is blitted to the destination (a black pixel). Any other value above zero is considered a
	forground color, and zero is background. If the parameter for the background color is zero,
	transparency is used for the background.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBufferMonoShadowClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 usForeground, UINT16 usBackground, UINT16 usShadow )
{
	UINT32 Unblitted;
	UINT8  *DestPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		TopSkip--;
	}

	do
	{
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount)) PxCount = LSCount;
				LSCount -= PxCount;
				if (usBackground == 0)
				{
					DestPtr += 2 * PxCount;
				}
				else
				{
					while (PxCount-- != 0)
					{
						*(UINT16*)DestPtr = usBackground;
						DestPtr += 2;
					}
				}
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					Unblitted = PxCount - LSCount;
					PxCount = LSCount;
				}
				else
				{
					Unblitted = 0;
				}
				LSCount -= PxCount;

				do
				{
					switch (*SrcPtr++)
					{
						case 0:  if (usBackground != 0) *(UINT16*)DestPtr = usBackground; break;
						case 1:  if (usShadow != 0)     *(UINT16*)DestPtr = usShadow;     break;
						default:                        *(UINT16*)DestPtr = usForeground; break;
					}
					DestPtr += 2;
				}
				while (--PxCount > 0);
				SrcPtr += Unblitted;
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit and end-of-line marker
		DestPtr += LineSkip;
	}
	while (--BlitHeight > 0);
}


/**********************************************************************************************
	Blt16BPPTo16BPP

	Copies a rect of 16 bit data from a video buffer to a buffer position of the brush
	in the data area, for later blitting. Used to copy background information for mercs
	etc. to their unblit buffer, for later reblitting. Does NOT clip.

**********************************************************************************************/
void Blt16BPPTo16BPP(UINT16 *pDest, UINT32 uiDestPitch, UINT16 *pSrc, UINT32 uiSrcPitch, INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos, UINT32 uiWidth, UINT32 uiHeight)
{
	UINT32 i;

	for (i = 0; i < uiHeight; i++)
	{
		memcpy(
			(UINT8*)pDest + uiDestPitch * (iDestYPos + i) + 2 * iDestXPos,
			(UINT8*)pSrc  + uiSrcPitch  * (iSrcYPos  + i) + 2 * iSrcXPos,
			uiWidth * 2
		);
	}
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
void Blt8BPPDataTo16BPPBufferTransZPixelateObscured( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT32 LineSkip;
	UINT8  *DestPtr, *ZPtr;
	UINT32 uiLineFlag;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32             usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	// Validations
	CHECKV(iTempX >= 0);
	CHECKV(iTempY >= 0);

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	UINT16 const* const p16BPPPalette = hSrcVObject->CurrentShade();
	LineSkip=(uiDestPitchBYTES-(usWidth*2));
	uiLineFlag=(iTempY&1);

	do
	{
		for (;;)
		{
			UINT8 data = *SrcPtr++;

			if (data == 0) break;
			if (data & 0x80)
			{
				data &= 0x7F;
				DestPtr += 2 * data;
				ZPtr += 2 * data;
			}
			else
			{
				do
				{
					if (*(UINT16*)ZPtr < usZValue)
					{
						*(UINT16*)ZPtr = usZValue;
					}
					else
					{
						if (uiLineFlag != (((uintptr_t)DestPtr & 2) != 0)) continue;
					}
					*(UINT16*)DestPtr = p16BPPPalette[*SrcPtr];
				}
				while (SrcPtr++, DestPtr += 2, ZPtr += 2, --data > 0);
			}
		}
		DestPtr += LineSkip;
		ZPtr += LineSkip;
		uiLineFlag ^= 1;
	}
	while (--usHeight > 0);
}


/* Blit an image into the destination buffer, using an ETRLE brush as a source
 * and a 16-bit buffer as a destination. As it is blitting, it checks the Z-
 * value of the ZBuffer, and if the pixel's Z level is below that of the current
 * pixel, it is written on, and the Z value is updated to the current value, for
 * any non-transparent pixels. The Z-buffer is 16 bit, and must be the same
 * dimensions (including Pitch) as the destination. */
void Blt8BPPDataTo16BPPBufferTransZ(UINT16* const buf, UINT32 const uiDestPitchBYTES, UINT16* const zbuf, UINT16 const zval, HVOBJECT const hSrcVObject, INT32 const iX, INT32 const iY, UINT16 const usIndex)
{
	Assert(hSrcVObject);
	Assert(buf);

	// Get offsets from index into structure
	ETRLEObject const& e      = hSrcVObject->SubregionProperties(usIndex);
	UINT32             height = e.usHeight;
	UINT32      const  width  = e.usWidth;

	// Add to start position of dest buffer
	INT32 const x = iX + e.sOffsetX;
	INT32 const y = iY + e.sOffsetY;

	// Validations
	CHECKV(x >= 0);
	CHECKV(y >= 0);

	UINT8 const*        src       = hSrcVObject->PixData(e);
	UINT32        const pitch     = uiDestPitchBYTES / 2;
	UINT16*             dst       = buf  + pitch * y + x;
	UINT16*             zdst      = zbuf + pitch * y + x;
	UINT16 const* const pal       = hSrcVObject->CurrentShade();
	UINT32              line_skip = pitch - width;

	for (;;)
	{
		UINT8 data = *src++;
		if (data == 0)
		{
			if (--height == 0) break;
			dst  += line_skip;
			zdst += line_skip;
		}
		else if (data & 0x80)
		{
			data &= 0x7F;
			dst  += data;
			zdst += data;
		}
		else
		{
			do
			{
				if (*zdst <= zval)
				{
					*zdst = zval;
					*dst  = pal[*src];
				}
				++src;
				++dst;
				++zdst;
			}
			while (--data != 0);
		}
	}
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferTransZNB

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on. The Z value is
	NOT updated by this version. The Z-buffer is 16 bit, and	must be the same dimensions
	(including Pitch) as the destination.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBufferTransZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32             usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	// Validations
	CHECKV(iTempX >= 0);
	CHECKV(iTempY >= 0);

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	UINT16 const* const p16BPPPalette = hSrcVObject->CurrentShade();
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	do
	{
		for (;;)
		{
			UINT8 data = *SrcPtr++;

			if (data == 0) break;
			if (data & 0x80)
			{
				data &= 0x7F;
				DestPtr += 2 * data;
				ZPtr += 2 * data;
			}
			else
			{
				do
				{
					if (*(UINT16*)ZPtr <= usZValue)
					{
						*(UINT16*)DestPtr = p16BPPPalette[*SrcPtr];
					}
					SrcPtr++;
					DestPtr += 2;
					ZPtr += 2;
				}
				while (--data > 0);
			}
		}
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while (--usHeight > 0);
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
void Blt8BPPDataTo16BPPBufferTransShadow(UINT16* pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, const UINT16* p16BPPPalette)
{
	UINT8  *DestPtr;
	UINT32 LineSkip;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32          usHeight = pTrav.usHeight;
	UINT32   const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	// Validations
	CHECKV(iTempX >= 0);
	CHECKV(iTempY >= 0);

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	do
	{
		for (;;)
		{
			UINT8 data = *SrcPtr++;

			if (data == 0) break;
			if (data & 0x80)
			{
				data &= 0x7F;
				DestPtr += 2 * data;
			}
			else
			{
				do
				{
					UINT8 px = *SrcPtr++;

					if (px == 254)
					{
						*(UINT16*)DestPtr = ShadeTable[2 * (*(UINT16*)DestPtr)];
					}
					else
					{
						*(UINT16*)DestPtr = p16BPPPalette[2 * px];
					}
					DestPtr += 2;
				}
				while (--data > 0);
			}
		}
		DestPtr += LineSkip;
	}
	while (--usHeight > 0);
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
void Blt8BPPDataTo16BPPBufferTransShadowZ(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, const UINT16* p16BPPPalette)
{
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32          usHeight = pTrav.usHeight;
	UINT32    const usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	// Validations
	CHECKV(iTempX >= 0);
	CHECKV(iTempY >= 0);

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	do
	{
		for (;;)
		{
			UINT8 data = *SrcPtr++;

			if (data == 0) break;
			if (data & 0x80)
			{
				data &= 0x7F;
				DestPtr += 2 * data;
			}
			else
			{
				do
				{
					UINT8 px = *SrcPtr++;

					if (px == 254)
					{
						if (*(UINT16*)ZPtr < usZValue)
						{
							*(UINT16*)DestPtr = ShadeTable[2 * (*(UINT16*)DestPtr)];
						}
					}
					else
					{
						if (*(UINT16*)ZPtr <= usZValue)
						{
							*(UINT16*)DestPtr = p16BPPPalette[2 * px];
						}
					}
					DestPtr += 2;
				}
				while (--data > 0);
			}
		}
		DestPtr += LineSkip;
	}
	while (--usHeight > 0);
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
void Blt8BPPDataTo16BPPBufferTransShadowZNB(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, const UINT16* p16BPPPalette)
{
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32             usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	// Validations
	CHECKV(iTempX >= 0);
	CHECKV(iTempY >= 0);

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	do
	{
		for (;;)
		{
			UINT8 data = *SrcPtr++;

			if (data == 0) break;
			if (data & 0x80)
			{
				data &= 0x7F;
				DestPtr += 2 * data;
				ZPtr += 2 * data;
			}
			else
			{
				do
				{
					UINT8 px = *SrcPtr++;

					if (px == 254)
					{
						if (*(UINT16*)ZPtr < usZValue)
						{
							*(UINT16*)DestPtr = ShadeTable[*(UINT16*)DestPtr];
						}
					}
					else
					{
						if (*(UINT16*)ZPtr <= usZValue)
						{
							*(UINT16*)DestPtr = p16BPPPalette[px];
						}
					}
					DestPtr += 2;
					ZPtr += 2;
				}
				while (--data > 0);
			}
		}
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while (--usHeight > 0);
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferTransShadowZNBObscured

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on. The Z value is NOT
	updated. If the source pixel is 254, it is considered a shadow, and the destination
	buffer is darkened rather than blitted on. The Z-buffer is 16 bit, and must be the same
	dimensions (including Pitch) as the destination.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBufferTransShadowZNBObscured(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, const UINT16* p16BPPPalette)
{
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;
	UINT32 uiLineFlag;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32             usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	// Validations
	CHECKV(iTempX >= 0);
	CHECKV(iTempY >= 0);

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	LineSkip=(uiDestPitchBYTES-(usWidth*2));
	uiLineFlag=(iTempY&1);

	do
	{
		for (;;)
		{
			UINT8 data = *SrcPtr++;

			if (data == 0) break;
			if (data & 0x80)
			{
				data &= 0x7F;
				DestPtr += 2 * data;
				ZPtr += 2 * data;
			}
			else
			{
				do
				{
					UINT8 px = *SrcPtr++;

					if (px == 254)
					{
						if (*(UINT16*)ZPtr < usZValue)
						{
							*(UINT16*)DestPtr = ShadeTable[*(UINT16*)DestPtr];
						}
					}
					else
					{
						if (*(UINT16*)ZPtr <= usZValue ||
								uiLineFlag == (((uintptr_t)DestPtr & 2) != 0)) // XXX ugly, can be done better by just examining every other pixel
						{
							*(UINT16*)DestPtr = p16BPPPalette[px];
						}
					}
					DestPtr += 2;
					ZPtr += 2;
				}
				while (--data > 0);
			}
		}
		DestPtr += LineSkip;
		ZPtr += LineSkip;
		uiLineFlag ^= 1;
	}
	while (--usHeight > 0);
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
void Blt8BPPDataTo16BPPBufferTransShadowZClip(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect* clipregion, const UINT16* p16BPPPalette)
{
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	UINT8 PxCount, px = 0;
	UINT32 Unblitted, LSCount;


	while (TopSkip)
	{
		px = *SrcPtr++;
		if (px & 0x80)  continue;
		if (px)
		{
			SrcPtr += px;
			continue;
		}
		TopSkip--;
	}

	do
	{
		Unblitted = 0;
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					Unblitted = 0;
					goto BlitTransparent;
				}
			}
			else
			{
				if (px > LSCount)
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					Unblitted = 0;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		Unblitted = 0;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7f;
				if (PxCount > static_cast<UINT32>(LSCount))
					PxCount = LSCount;

				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
				ZPtr    += 2 * PxCount;
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					PxCount -= LSCount;
					Unblitted = PxCount;
					PxCount = LSCount;
				}
				LSCount -= PxCount;

				do
				{
					if (*ZPtr < usZValue )
					{
						*ZPtr = usZValue;
						px = *SrcPtr++;
						if (px != 254)
							*(UINT16*)DestPtr = p16BPPPalette[px];
						else
							*(UINT16*)DestPtr = ShadeTable[*(UINT16*)DestPtr];
					}

					DestPtr += 2;
					ZPtr += 2;
				}
				while ( --px > 0 );
				SrcPtr += Unblitted;
			}
		}
		while (*SrcPtr++ != 0);
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while ( --BlitHeight > 0 );
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
void Blt8BPPDataTo16BPPBufferTransShadowClip(UINT16* pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect* clipregion, const UINT16* p16BPPPalette)
{
	UINT8  *DestPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	UINT8 PxCount, px = 0;
	UINT32 Unblitted;
	INT32 LSCount;

	while (TopSkip)
	{
		px = *SrcPtr++;
		if (px & 0x80)  continue;
		if (px)
		{
			SrcPtr += px;
			continue;
		}
		TopSkip--;
	}

	do
	{
		Unblitted = 0;
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					Unblitted = 0;
					goto BlitTransparent;
				}
			}
			else
			{
				if (px > LSCount)
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					Unblitted = 0;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		Unblitted = 0;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7f;
				if (PxCount > static_cast<UINT32>(LSCount))
					PxCount = LSCount;

				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					PxCount -= LSCount;
					Unblitted = PxCount;
					PxCount = LSCount;
				}
				LSCount -= PxCount;

				do
				{
					px = *SrcPtr++;
					if (px != 254)
						*(UINT16*)DestPtr = p16BPPPalette[px];
					else
						*(UINT16*)DestPtr = ShadeTable[*(UINT16*)DestPtr];
					DestPtr += 2;
				}
				while ( --px > 0 );
				SrcPtr += Unblitted;
			}
		}
		while (*SrcPtr++ != 0);
		DestPtr += LineSkip;
	}
	while ( --BlitHeight > 0 );
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
void Blt8BPPDataTo16BPPBufferTransShadowZNBClip(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect* clipregion, const UINT16* p16BPPPalette)
{
	UINT32 Unblitted;
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		TopSkip--;
	}

	do
	{
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount)) PxCount = LSCount;
				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
				ZPtr    += 2 * PxCount;
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					Unblitted = PxCount - LSCount;
					PxCount = LSCount;
				}
				else
				{
					Unblitted = 0;
				}
				LSCount -= PxCount;

				do
				{
					UINT8 px = *SrcPtr++;

					if (px == 254)
					{
						if (*(UINT16*)ZPtr < usZValue)
						{
							*(UINT16*)DestPtr = ShadeTable[*(UINT16*)DestPtr];
						}
					}
					else
					{
						if (*(UINT16*)ZPtr <= usZValue)
						{
							*(UINT16*)DestPtr = p16BPPPalette[px];
						}
					}
					DestPtr += 2;
					ZPtr += 2;
				}
				while (--PxCount > 0);
				SrcPtr += Unblitted;
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit and end-of-line marker
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while (--BlitHeight > 0);
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
void Blt8BPPDataTo16BPPBufferTransShadowZNBObscuredClip(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect* clipregion, const UINT16* p16BPPPalette)
{
	UINT32 Unblitted, uiLineFlag;
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	uiLineFlag = (iTempY + TopSkip) & 1;

	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		TopSkip--;
	}

	do
	{
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount)) PxCount = LSCount;
				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
				ZPtr    += 2 * PxCount;
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					Unblitted = PxCount - LSCount;
					PxCount = LSCount;
				}
				else
				{
					Unblitted = 0;
				}
				LSCount -= PxCount;

				do
				{
					UINT8 px = *SrcPtr++;

					if (px == 254)
					{
						if (*(UINT16*)ZPtr < usZValue)
						{
							*(UINT16*)DestPtr = ShadeTable[*(UINT16*)DestPtr];
						}
					}
					else
					{
						if (*(UINT16*)ZPtr <= usZValue ||
								uiLineFlag == (((uintptr_t)DestPtr & 2) != 0)) // XXX ugly, can be done better by just examining every other pixel
						{
							*(UINT16*)DestPtr = p16BPPPalette[px];
						}
					}
					DestPtr += 2;
					ZPtr += 2;
				}
				while (--PxCount > 0);
				SrcPtr += Unblitted;
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit and end-of-line marker
		DestPtr += LineSkip;
		ZPtr += LineSkip;
		uiLineFlag ^= 1;
	}
	while (--BlitHeight > 0);
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferShadowZ

	Creates a shadow using a brush, but modifies the destination buffer only if the current
	Z level is equal to higher than what's in the Z buffer at that pixel location. It
	updates the Z buffer with the new Z level.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBufferShadowZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav    = hSrcVObject->SubregionProperties(usIndex);
	UINT32             usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	// Validations
	CHECKV(iTempX >= 0);
	CHECKV(iTempY >= 0);

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	do
	{
		for (;;)
		{
			UINT8 data = *SrcPtr++;

			if (data == 0) break;
			if (data & 0x80)
			{
				data &= 0x7F;
				DestPtr += 2 * data;
				ZPtr += 2 * data;
			}
			else
			{
				SrcPtr += data;
				do
				{
					if (*(UINT16*)ZPtr < usZValue)
					{
						*(UINT16*)ZPtr = usZValue;
						*(UINT16*)DestPtr = ShadeTable[*(UINT16*)DestPtr];
					}
					DestPtr += 2;
					ZPtr += 2;
				}
				while (--data  > 0);
			}
		}
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while (--usHeight > 0);
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferShadowZClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBufferShadowZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		TopSkip--;
	}

	do
	{
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount)) PxCount = LSCount;
				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
				ZPtr    += 2 * PxCount;
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				SrcPtr += PxCount;
				if (PxCount > static_cast<UINT32>(LSCount)) PxCount = LSCount;
				LSCount -= PxCount;

				do
				{
					if (*(UINT16*)ZPtr < usZValue)
					{
						*(UINT16*)ZPtr = usZValue;
						*(UINT16*)DestPtr = ShadeTable[*(UINT16*)DestPtr];
					}
					DestPtr += 2;
					ZPtr += 2;
				}
				while (--PxCount > 0);
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit and end-of-line marker
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while (--BlitHeight > 0);
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferShadowZNB

	Creates a shadow using a brush, but modifies the destination buffer only if the current
	Z level is equal to higher than what's in the Z buffer at that pixel location. It does
	NOT update the Z buffer with the new Z value.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBufferShadowZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav    = hSrcVObject->SubregionProperties(usIndex);
	UINT32             usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	// Validations
	CHECKV(iTempX >= 0);
	CHECKV(iTempY >= 0);

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	do
	{
		for (;;)
		{
			UINT8 data = *SrcPtr++;

			if (data == 0) break;
			if (data & 0x80)
			{
				data &= 0x7F;
				DestPtr += 2 * data;
				ZPtr += 2 * data;
			}
			else
			{
				do
				{
					if (*(UINT16*)ZPtr < usZValue)
					{
						*(UINT16*)DestPtr = ShadeTable[*(UINT16*)DestPtr];
					}
				}
				while (SrcPtr++, DestPtr += 2, ZPtr += 2, --data > 0);
			}
		}
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while (--usHeight > 0);
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferShadowZNBClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, the Z value is
	not updated,	for any non-transparent pixels. The Z-buffer is 16 bit, and	must be the
	same dimensions (including Pitch) as the destination.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBufferShadowZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT32 Unblitted;
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		TopSkip--;
	}

	do
	{
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount)) PxCount = LSCount;
				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
				ZPtr    += 2 * PxCount;
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					Unblitted = PxCount - LSCount;
					PxCount = LSCount;
				}
				else
				{
					Unblitted = 0;
				}
				LSCount -= PxCount;

				do
				{
					if (*(UINT16*)ZPtr < usZValue)
					{
						*(UINT16*)DestPtr = ShadeTable[*(UINT16*)DestPtr];
					}
				}
				while (SrcPtr++, DestPtr += 2, ZPtr += 2, --PxCount > 0);
				SrcPtr += Unblitted;
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit and end-of-line marker
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while (--BlitHeight > 0);
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferTransZClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBufferTransZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT32 Unblitted;
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	UINT16 const* const p16BPPPalette = hSrcVObject->CurrentShade();
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		TopSkip--;
	}

	do
	{
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount)) PxCount = LSCount;
				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
				ZPtr    += 2 * PxCount;
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					Unblitted = PxCount - LSCount;
					PxCount = LSCount;
				}
				else
				{
					Unblitted = 0;
				}
				LSCount -= PxCount;

				do
				{
					if (*(UINT16*)ZPtr <= usZValue)
					{
						*(UINT16*)ZPtr = usZValue;
						*(UINT16*)DestPtr = p16BPPPalette[*SrcPtr];
					}
					SrcPtr++;
					DestPtr += 2;
					ZPtr += 2;
				}
				while (--PxCount > 0);
				SrcPtr += Unblitted;
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit and end-of-line marker
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while (--BlitHeight > 0);
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferTransZNBClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on. The Z value is NOT
	updated in this version. The Z-buffer is 16 bit, and must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBufferTransZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT32 Unblitted;
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	UINT16 const* const p16BPPPalette = hSrcVObject->CurrentShade();
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		TopSkip--;
	}

	do
	{
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount)) PxCount = LSCount;
				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
				ZPtr    += 2 * PxCount;
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					Unblitted = PxCount - LSCount;
					PxCount = LSCount;
				}
				else
				{
					Unblitted = 0;
				}
				LSCount -= PxCount;

				do
				{
					if (*(UINT16*)ZPtr <= usZValue)
					{
						*(UINT16*)DestPtr = p16BPPPalette[*SrcPtr];
					}
					SrcPtr++;
					DestPtr += 2;
					ZPtr += 2;
				}
				while (--PxCount > 0);
				SrcPtr += Unblitted;
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit and end-of-line marker
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while (--BlitHeight > 0);
}


/* Blit a subrect from a flat 8 bit surface to a 16-bit buffer. */
void Blt8BPPDataSubTo16BPPBuffer(UINT16* const buf, UINT32 const uiDestPitchBYTES, SGPVSurface* const hSrcVSurface, UINT8* const pSrcBuffer, UINT32 const src_pitch, INT32 const x, INT32 const y, SGPBox const* const rect)
{
	Assert(hSrcVSurface);
	Assert(pSrcBuffer);
	Assert(buf);

	CHECKV(x >= 0);
	CHECKV(y >= 0);

	UINT32 const LeftSkip   = rect->x;
	UINT32 const TopSkip    = rect->y * src_pitch;
	UINT32 const BlitLength = rect->w;
	UINT32       BlitHeight = rect->h;
	UINT32 const src_skip   = src_pitch - BlitLength;

	UINT32        const pitch     = uiDestPitchBYTES / 2;
	UINT8  const*       src       = pSrcBuffer + TopSkip + LeftSkip;
	UINT16*             dst       = buf + pitch * y + x;
	UINT16 const* const pal       = hSrcVSurface->p16BPPPalette;
	UINT32              line_skip = pitch - BlitLength;

	do
	{
		UINT32 w = BlitLength;
		do
		{
			*dst++ = pal[*src++];
		}
		while (--w != 0);
		src += src_skip;
		dst += line_skip;
	}
	while (--BlitHeight != 0);
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBuffer

	Blits from a flat surface to a 16-bit buffer.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBuffer( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPVSurface* hSrcVSurface, UINT8 *pSrcBuffer, INT32 iX, INT32 iY)
{
	INT32  iTempX, iTempY;

	// Assertions
	Assert( hSrcVSurface != NULL );
	Assert( pSrcBuffer != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	UINT32 const usWidth  = hSrcVSurface->Width();
	UINT32 const usHeight = hSrcVSurface->Height();

	// Add to start position of dest buffer
	iTempX = iX;
	iTempY = iY;

	// Validations
	CHECKV(iTempX >= 0);
	CHECKV(iTempY >= 0);

	UINT8*  SrcPtr        = pSrcBuffer;
	UINT16* DestPtr       = pBuffer + uiDestPitchBYTES / 2 * iTempY + iTempX;
	UINT16* p16BPPPalette = hSrcVSurface->p16BPPPalette;

	for (size_t h = usHeight; h != 0; --h)
	{
		for (size_t w = 0; w != usWidth; ++w)
		{
			DestPtr[w] = p16BPPPalette[SrcPtr[w]];
		}

		SrcPtr  += usWidth;
		DestPtr += uiDestPitchBYTES / 2;
	}
}


/* Blit from a flat surface to a 16-bit buffer, dividing the source image into
 * exactly half the size, optionally from a sub-region.
 * - Source rect is in source units.
 * - In order to make sure the same pixels are skipped, always align the top and
 *   left coordinates to the same factor of two.
 * - A rect specifying an odd number of pixels will divide out to an even number
 *   of pixels blitted to the destination. */
void Blt8BPPDataTo16BPPBufferHalf(UINT16* const dst_buf, UINT32 const uiDestPitchBYTES, SGPVSurface* const src_surface, UINT8 const* const src_buf, UINT32 const src_pitch, INT32 const x, INT32 const y, SGPBox const* const rect)
{
	Assert(src_surface);
	Assert(src_buf);
	Assert(dst_buf);

	CHECKV(x >= 0);
	CHECKV(y >= 0);

	UINT8 const* src = src_buf;
	UINT32       width;
	UINT32       height;
	if (rect)
	{
		width  = rect->w;
		height = rect->h;
		CHECKV(0 < width  && width  <= src_surface->Width());
		CHECKV(0 < height && height <= src_surface->Height());

		src += src_pitch * rect->y + rect->x;
	}
	else
	{
		width  = src_surface->Width();
		height = src_surface->Height();
	}

	UINT16*             dst      = dst_buf + uiDestPitchBYTES / 2 * y + x;
	UINT32        const src_skip = (src_pitch - width / 2) * 2;
	UINT32        const dst_skip = uiDestPitchBYTES / 2 - width / 2;
	UINT16 const* const pal      = src_surface->p16BPPPalette;

	height /= 2;
	do
	{
		UINT32 w = width / 2;
		do
		{
			*dst++ = pal[*src];
			src += 2;
		}
		while (--w > 0);
		src += src_skip;
		dst += dst_skip;
	}
	while (--height > 0);
}


void SetClippingRect(SGPRect *clip)
{
	Assert(clip!=NULL);
	Assert(clip->iLeft < clip->iRight);
	Assert(clip->iTop < clip->iBottom);
	ClippingRect = *clip;
}


void GetClippingRect(SGPRect *clip)
{
	Assert(clip!=NULL);
	*clip = ClippingRect;
}


/**********************************************************************************************
	Blt16BPPBufferPixelateRectWithColor

		Given an 8x8 pattern and a color, pixelates an area by repeatedly "applying the color" to pixels whereever there
		is a non-zero value in the pattern.

		KM:  Added Nov. 23, 1998
		This is all the code that I moved from Blt16BPPBufferPixelateRect().
		This function now takes a color field (which previously was
		always black.  The 3rd assembler line in this function:

				mov	ax, usColor	// color of pixel

		used to be:

				xor	eax, eax	// color of pixel (black or 0)

	  This was the only internal modification I made other than adding the usColor argument.

*********************************************************************************************/
static void Blt16BPPBufferPixelateRectWithColor(UINT16* pBuffer, UINT32 uiDestPitchBYTES, SGPRect* area, const UINT8 Pattern[8][8], UINT16 usColor)
{
	INT32  width, height;
	UINT32 LineSkip;
	UINT16 *DestPtr;
	INT32	iLeft, iTop, iRight, iBottom;

	// Assertions
	Assert( pBuffer != NULL );
	Assert( Pattern != NULL );

	iLeft = std::max(ClippingRect.iLeft, area->iLeft);
	iTop = std::max(ClippingRect.iTop, area->iTop);
	iRight = std::min(ClippingRect.iRight - 1, int(area->iRight));
	iBottom = std::min(ClippingRect.iBottom - 1, int(area->iBottom));

	DestPtr=(pBuffer+(iTop*(uiDestPitchBYTES/2))+iLeft);
	width=iRight-iLeft+1;
	height=iBottom-iTop+1;
	LineSkip=(uiDestPitchBYTES-(width*2));

	CHECKV(width  >= 1);
	CHECKV(height >= 1);

	UINT32 row = 0;
	do
	{
		UINT32 col = 0;
		UINT32 w = width;

		do
		{
			if (Pattern[row][col] != 0) *DestPtr = usColor;
			DestPtr++;
			col = (col + 1) % 8;
		}
		while (--w > 0);
		DestPtr += LineSkip / 2;
		row = (row + 1) % 8;
	}
	while (--height > 0);
}


//Uses black hatch color
void Blt16BPPBufferHatchRect(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area )
{
	const UINT8 Pattern[8][8] =
	{
		{ 1,0,1,0,1,0,1,0 },
		{ 0,1,0,1,0,1,0,1 },
		{ 1,0,1,0,1,0,1,0 },
		{ 0,1,0,1,0,1,0,1 },
		{ 1,0,1,0,1,0,1,0 },
		{ 0,1,0,1,0,1,0,1 },
		{ 1,0,1,0,1,0,1,0 },
		{ 0,1,0,1,0,1,0,1 }
	};
	Blt16BPPBufferPixelateRectWithColor( pBuffer, uiDestPitchBYTES, area, Pattern, 0 );
}

void Blt16BPPBufferLooseHatchRectWithColor(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area, UINT16 usColor )
{
	const UINT8 Pattern[8][8] =
	{
		{ 1,0,0,0,1,0,0,0 },
		{ 0,0,0,0,0,0,0,0 },
		{ 0,0,1,0,0,0,1,0 },
		{ 0,0,0,0,0,0,0,0 },
		{ 1,0,0,0,1,0,0,0 },
		{ 0,0,0,0,0,0,0,0 },
		{ 0,0,1,0,0,0,1,0 },
		{ 0,0,0,0,0,0,0,0 }
	};
	Blt16BPPBufferPixelateRectWithColor( pBuffer, uiDestPitchBYTES, area, Pattern, usColor );
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferShadow

	Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
	image as a mask. Any Non-zero index pixels are used to darken destination pixels.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBufferShadow( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex)
{
	UINT8  *DestPtr;
	UINT32 LineSkip;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav    = hSrcVObject->SubregionProperties(usIndex);
	UINT32             usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	// Validations
	CHECKV(iTempX >= 0);
	CHECKV(iTempY >= 0);

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	do
	{
		for (;;)
		{
			UINT8 data = *SrcPtr++;

			if (data == 0) break;
			if (data & 0x80)
			{
				data &= 0x7F;
				DestPtr += 2 * data;
			}
			else
			{
				SrcPtr += data;
				do
				{
					*(UINT16*)DestPtr = ShadeTable[*(UINT16*)DestPtr];
					DestPtr += 2;
				}
				while (--data  > 0);
			}
		}
		DestPtr += LineSkip;
	}
	while (--usHeight > 0);
}


/* Blit an image into the destination buffer, using an ETRLE brush as a source
 * and a 16-bit buffer as a destination. */
void Blt8BPPDataTo16BPPBufferTransparent(UINT16* const buf, UINT32 const uiDestPitchBYTES, SGPVObject const* const hSrcVObject, INT32 const iX, INT32 const iY, UINT16 const usIndex)
{
	Assert(hSrcVObject);
	Assert(buf);

	// Get offsets from index into structure
	ETRLEObject const& e      = hSrcVObject->SubregionProperties(usIndex);
	UINT32             height = e.usHeight;
	UINT32      const  width  = e.usWidth;

	// Add to start position of dest buffer
	INT32 const x = iX + e.sOffsetX;
	INT32 const y = iY + e.sOffsetY;

	CHECKV(x >= 0);
	CHECKV(y >= 0);

	UINT32        const pitch     = uiDestPitchBYTES / 2;
	UINT8  const*       src       = hSrcVObject->PixData(e);
	UINT16*             dst       = buf + pitch * y + x;
	UINT16 const* const pal       = hSrcVObject->CurrentShade();
	UINT32              line_skip = pitch - width;

	for (;;)
	{
		UINT8 data = *src++;
		if (data == 0)
		{
			if (--height == 0) break;
			dst += line_skip;
		}
		else if (data & 0x80)
		{ // Transparent
			dst += data & 0x7F;
		}
		else
		{
			do
			{
				*dst++ = pal[*src++];
			}
			while (--data != 0);
		}
	}
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferTransparentClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. Clips the brush.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBufferTransparentClip(UINT16* const pBuffer, const UINT32 uiDestPitchBYTES, const SGPVObject* const hSrcVObject, const INT32 iX, const INT32 iY, const UINT16 usIndex, const SGPRect* const clipregion)
{
	UINT32 Unblitted;
	UINT8  *DestPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	UINT16 const* const p16BPPPalette = hSrcVObject->CurrentShade();
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	UINT32 LSCount;
	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		TopSkip--;
	}

	do
	{
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount)) PxCount = LSCount;
				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					Unblitted = PxCount - LSCount;
					PxCount = LSCount;
				}
				else
				{
					Unblitted = 0;
				}
				LSCount -= PxCount;

				do
				{
					*(UINT16*)DestPtr = p16BPPPalette[*SrcPtr++];
					DestPtr += 2;
				}
				while (--PxCount > 0);
				SrcPtr += Unblitted;
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit and end-of-line marker
		DestPtr += LineSkip;
	}
	while (--BlitHeight > 0);
}


/**********************************************************************************************
BltIsClipped

	Determines whether a given blit will need clipping or not. Returns TRUE/FALSE.

**********************************************************************************************/
BOOLEAN BltIsClipped(const SGPVObject* const hSrcVObject, const INT32 iX, const INT32 iY, const UINT16 usIndex, const SGPRect* const clipregion)
{
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	if(std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth))
		return(TRUE);

	if (std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth))
		return(TRUE);

	if(std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight))
		return(TRUE);

	if (std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight))
		return(TRUE);

	return(FALSE);
}



/**********************************************************************************************
Blt8BPPDataTo16BPPBufferShadowClip

	Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
	image as a mask. Any Non-zero index pixels are used to darken destination pixels. Blitter
	clips brush if it doesn't fit on the viewport.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBufferShadowClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT8  *DestPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	UINT32 LSCount;
	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		TopSkip--;
	}

	do
	{
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount)) PxCount = LSCount;
				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				SrcPtr += PxCount;
				if (PxCount > static_cast<UINT32>(LSCount)) PxCount = LSCount;
				LSCount -= PxCount;

				do
				{
					*(UINT16*)DestPtr = ShadeTable[*(UINT16*)DestPtr];
					DestPtr += 2;
				}
				while (--PxCount > 0);
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit and end-of-line marker
		DestPtr += LineSkip;
	}
	while (--BlitHeight > 0);
}


void Blt16BPPBufferFilterRect(UINT16* pBuffer, UINT32 uiDestPitchBYTES, const UINT16* filter_table, SGPRect* area)
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

	CHECKV(width  >= 1);
	CHECKV(height >= 1);

	do
	{
		UINT32 w = width;

		do
		{
			*DestPtr = filter_table[*DestPtr];
			DestPtr++;
		}
		while (--w > 0);
		DestPtr = (UINT16*)((UINT8*)DestPtr + LineSkip);
	}
	while (--height > 0);
}


/**********************************************************************************************
BltIsClippedOrOffScreen

	Determines whether a given blit will need clipping or not. Returns TRUE/FALSE.

**********************************************************************************************/
CHAR8 BltIsClippedOrOffScreen( HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion )
{
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	INT32 gLeftSkip   = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	INT32 gTopSkip    = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	INT32 gRightSkip  = std::clamp(iTempX + (INT32)usWidth  - ClipX2, 0, (INT32)usWidth);
	INT32 gBottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

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


// ATE New blitter for rendering a differrent color for value 254. Can be transparent if outline is SGP_TRANSPARENT
void Blt8BPPDataTo16BPPBufferOutline(UINT16* const buf, UINT32 const uiDestPitchBYTES, SGPVObject const* const hSrcVObject, INT32 const iX, INT32 const iY, UINT16 const usIndex, INT16 const outline)
{
	Assert(hSrcVObject);
	Assert(buf);

	// Get offsets from index into structure
	ETRLEObject const& e      = hSrcVObject->SubregionProperties(usIndex);
	UINT32             height = e.usHeight;
	UINT32      const  width  = e.usWidth;

	// Add to start position of dest buffer
	INT32 const x = iX + e.sOffsetX;
	INT32 const y = iY + e.sOffsetY;

	CHECKV(x >= 0);
	CHECKV(y >= 0);

	UINT8  const*       src       = hSrcVObject->PixData(e);
	UINT32        const pitch     = uiDestPitchBYTES / 2;
	UINT16*             dst       = buf + pitch * y + x;
	UINT32              line_skip = pitch - width;
	UINT16 const* const pal       = hSrcVObject->CurrentShade();

	for (;;)
	{
		UINT8 data = *src++;
		if (data == 0)
		{
			if (--height == 0) break;
			dst += line_skip;
		}
		else if (data & 0x80)
		{
			dst += data & 0x7F;
		}
		else
		{
			do
			{
				UINT8 const px = *src++;
				if (px != 254)
				{
					*dst = pal[px];
				}
				else if (outline != SGP_TRANSPARENT)
				{
					*dst = outline;
				}
				++dst;
			}
			while (--data != 0);
		}
	}
}


// ATE New blitter for rendering a differrent color for value 254. Can be transparent if s16BPPColor is SGP_TRANSPARENT
void Blt8BPPDataTo16BPPBufferOutlineClip(UINT16* const pBuffer, const UINT32 uiDestPitchBYTES, const SGPVObject* const hSrcVObject, const INT32 iX, const INT32 iY, const UINT16 usIndex, const INT16 s16BPPColor, const SGPRect* const clipregion)
{
	UINT32 Unblitted;
	UINT8  *DestPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));
	UINT16 const* const p16BPPPalette = hSrcVObject->CurrentShade();

	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		TopSkip--;
	}

	do
	{
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount)) PxCount = LSCount;
				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					Unblitted = PxCount - LSCount;
					PxCount = LSCount;
				}
				else
				{
					Unblitted = 0;
				}
				LSCount -= PxCount;

				do
				{
					UINT32 src = *SrcPtr++;

					if (src != 254)
					{
						*(UINT16*)DestPtr = p16BPPPalette[src];
					}
					else if (s16BPPColor != SGP_TRANSPARENT)
					{
						*(UINT16*)DestPtr = s16BPPColor;
					}
					DestPtr += 2;
				}
				while (--PxCount > 0);
				SrcPtr += Unblitted;
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit and end-of-line marker
		DestPtr += LineSkip;
	}
	while (--BlitHeight > 0);
}


void Blt8BPPDataTo16BPPBufferOutlineZClip(UINT16* const pBuffer, const UINT32 uiDestPitchBYTES, UINT16* const pZBuffer, const UINT16 usZValue, const HVOBJECT hSrcVObject, const INT32 iX, const INT32 iY, const UINT16 usIndex, const INT16 s16BPPColor, const SGPRect* const clipregion)
{
	UINT32 Unblitted;
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);

	LineSkip=(uiDestPitchBYTES-(BlitLength*2));
	UINT16 const* const p16BPPPalette = hSrcVObject->CurrentShade();

	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		TopSkip--;
	}

	do
	{
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount)) PxCount = LSCount;
				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
				ZPtr    += 2 * PxCount;
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					Unblitted = PxCount - LSCount;
					PxCount = LSCount;
				}
				else
				{
					Unblitted = 0;
				}
				LSCount -= PxCount;

				do
				{
					if (*(UINT16*)ZPtr <= usZValue)
					{
						UINT32 src = *SrcPtr;

						if (src != 254)
						{
							*(UINT16*)ZPtr = usZValue;
							*(UINT16*)DestPtr = p16BPPPalette[src];
						}
						else
						{
							*(UINT16*)DestPtr = s16BPPColor;
						}
					}
					SrcPtr++;
					DestPtr += 2;
					ZPtr += 2;
				}
				while (--PxCount > 0);
				SrcPtr += Unblitted;
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit an end-of-line marker
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while (--BlitHeight > 0);
}


void Blt8BPPDataTo16BPPBufferOutlineZPixelateObscuredClip(UINT16* const pBuffer, const UINT32 uiDestPitchBYTES, UINT16* const pZBuffer, const UINT16 usZValue, const HVOBJECT hSrcVObject, const INT32 iX, const INT32 iY, const UINT16 usIndex, const INT16 s16BPPColor, const SGPRect* const clipregion)
{
	UINT32 Unblitted;
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;
	UINT32 uiLineFlag;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);

	LineSkip=(uiDestPitchBYTES-(BlitLength*2));
	UINT16 const* const p16BPPPalette = hSrcVObject->CurrentShade();
	uiLineFlag=(iTempY&1);


	uiLineFlag = (iTempY + TopSkip) & 1;

	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		TopSkip--;
	}

	do
	{
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount)) PxCount = LSCount;
				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
				ZPtr    += 2 * PxCount;
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					Unblitted = PxCount - LSCount;
					PxCount = LSCount;
				}
				else
				{
					Unblitted = 0;
				}
				LSCount -= PxCount;

				do
				{
					if (*(UINT16*)ZPtr <= usZValue)
					{
						*(UINT16*)ZPtr = usZValue;
					}
					else
					{
						// XXX original code updates Z value in one of two cases on this path, seems wrong
						if (uiLineFlag != (((uintptr_t)DestPtr & 2) != 0)) continue;
					}

					UINT8 px = *SrcPtr;
					if (px == 254)
					{
						*(UINT16*)DestPtr = s16BPPColor;
					}
					else
					{
						// XXX original code writes garbage (lower 8 bit are the colour index) into the Z buffer at this point
						*(UINT16*)DestPtr = p16BPPPalette[px];
					}
				}
				while (SrcPtr++, DestPtr += 2, ZPtr += 2, --PxCount > 0);
				SrcPtr += Unblitted;
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit and end-of-line marker
		DestPtr += LineSkip;
		ZPtr += LineSkip;
		uiLineFlag ^= 1;
	}
	while (--BlitHeight > 0);
}


void Blt8BPPDataTo16BPPBufferOutlineShadow(UINT16* const pBuffer, const UINT32 uiDestPitchBYTES, const SGPVObject* const hSrcVObject, const INT32 iX, const INT32 iY, const UINT16 usIndex)
{
	UINT8  *DestPtr;
	UINT32 LineSkip;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32             usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	// Validations
	CHECKV(iTempX >= 0);
	CHECKV(iTempY >= 0);

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	do
	{
		for (;;)
		{
			UINT8 data = *SrcPtr++;

			if (data == 0) break;
			if (data & 0x80)
			{
				data &= 0x7F;
				DestPtr += data * 2;
			}
			else
			{
				do
				{
					if (*SrcPtr++ != 254)
					{
						*(UINT16*)DestPtr = ShadeTable[*(UINT16*)DestPtr];
					}
					DestPtr += 2;
				}
				while (--data > 0);
			}
		}
		DestPtr += LineSkip;
	}
	while (--usHeight > 0);
}


void Blt8BPPDataTo16BPPBufferOutlineShadowClip(UINT16* const pBuffer, const UINT32 uiDestPitchBYTES, const SGPVObject* const hSrcVObject, const INT32 iX, const INT32 iY, const UINT16 usIndex, const SGPRect* const clipregion)
{
#if 1 // XXX TODO
	UNIMPLEMENTED
#else
	UINT8  *DestPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	UINT32 Unblitted;
	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET ShadeTable
		xor		eax, eax
		mov		ebx, TopSkip
		xor		ecx, ecx

		or		ebx, ebx		// check for nothing clipped on top
		jz		LeftSkipSetup

TopSkipLoop:						// Skips the number of lines clipped at the top

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
		mov		ebx, LeftSkip		// check for nothing clipped on the left
		or		ebx, ebx
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, ebx
		je		LSSkip2			// if equal, skip whole, and start blit with new run
		jb		LSSkip1			// if less, skip whole thing

		add		esi, ebx		// skip partial run, jump into normal loop for rest
		sub		ecx, ebx
		mov		ebx, BlitLength
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx		// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx		// skip whole run, continue skipping
		sub		ebx, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, ebx
		je		BlitLineSetup		// if equal, skip whole, and start blit with new run
		jb		LSTrans1		// if less, skip whole thing

		sub		ecx, ebx		// skip partial run, jump into normal loop for rest
		mov		ebx, BlitLength
		jmp		BlitTransparent


LSTrans1:
		sub		ebx, ecx		// skip whole run, continue skipping
		jmp		LeftSkipLoop




BlitLineSetup:						// Does any actual blitting (trans/non) for the line
		mov		ebx, BlitLength
		mov		Unblitted, 0

BlitDispatch:

		or		ebx, ebx		// Check to see if we're done blitting
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
#endif
}


void Blt8BPPDataTo16BPPBufferOutlineZ(UINT16* const pBuffer, const UINT32 uiDestPitchBYTES, UINT16* const pZBuffer, const UINT16 usZValue, const HVOBJECT hSrcVObject, const INT32 iX, const INT32 iY, const UINT16 usIndex, const INT16 s16BPPColor)
{
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32             usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	// Validations
	CHECKV(iTempX >= 0);
	CHECKV(iTempY >= 0);

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	UINT16 const* const p16BPPPalette = hSrcVObject->CurrentShade();
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

#if 1 // XXX TODO
	do
	{
		for (;;)
		{
			UINT8 data = *SrcPtr++;

			if (data == 0) break;
			if (data & 0x80)
			{
				data &= 0x7F;
				DestPtr += 2 * data;
				ZPtr += 2 * data;
			}
			else
			{
				do
				{
					if (*(UINT16*)ZPtr <= usZValue)
					{
						UINT8 px = *SrcPtr;

						if (px == 254)
						{
							*(UINT16*)DestPtr = s16BPPColor;
						}
						else
						{
							*(UINT16*)ZPtr = usZValue; // XXX TODO original code writes garbage into the Z buffer, but comment says don't write at all
							*(UINT16*)DestPtr = p16BPPPalette[px];
						}
					}
					SrcPtr++;
					DestPtr += 2;
					ZPtr += 2;
				}
				while (--data > 0);
			}
		}
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while (--usHeight > 0);
#else
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
#endif
}


void Blt8BPPDataTo16BPPBufferOutlineZPixelateObscured(UINT16* const pBuffer, const UINT32 uiDestPitchBYTES, UINT16* const pZBuffer, const UINT16 usZValue, const HVOBJECT hSrcVObject, const INT32 iX, const INT32 iY, const UINT16 usIndex, const INT16 s16BPPColor)
{
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;
	UINT32 uiLineFlag;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32             usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	// Validations
	CHECKV(iTempX >= 0);
	CHECKV(iTempY >= 0);

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	UINT16 const* const p16BPPPalette = hSrcVObject->CurrentShade();
	LineSkip=(uiDestPitchBYTES-(usWidth*2));
	uiLineFlag=(iTempY&1);

#if 1 // XXX TODO
	do
	{
		for (;;)
		{
			UINT8 data = *SrcPtr++;

			if (data == 0) break;
			if (data & 0x80)
			{
				data &= 0x7F;
				DestPtr += 2 * data;
				ZPtr += 2 * data;
			}
			else
			{
				do
				{
					if (*(UINT16*)ZPtr < usZValue)
					{
						*(UINT16*)ZPtr = usZValue;
					}
					else
					{
						// XXX original code updates Z value in one of two cases on this path, seems wrong
						if (uiLineFlag != (((uintptr_t)DestPtr & 2) != 0)) continue;
					}

					UINT8 px = *SrcPtr;
					if (px == 254)
					{
						*(UINT16*)DestPtr = s16BPPColor;
					}
					else
					{
						*(UINT16*)DestPtr = p16BPPPalette[px];
					}
				}
				while (SrcPtr++, DestPtr += 2, ZPtr += 2, --data > 0);
			}
		}
		DestPtr += LineSkip;
		ZPtr += LineSkip;
		uiLineFlag ^= 1;
	}
	while (--usHeight > 0);
#else
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
#endif
}


// This is the same as above, but DONOT WRITE to Z!
void Blt8BPPDataTo16BPPBufferOutlineZNB(UINT16* const pBuffer, const UINT32 uiDestPitchBYTES, UINT16* const pZBuffer, const UINT16 usZValue, const HVOBJECT hSrcVObject, const INT32 iX, const INT32 iY, const UINT16 usIndex)
{
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32             usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	// Validations
	CHECKV(iTempX >= 0);
	CHECKV(iTempY >= 0);

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	UINT16 const* const p16BPPPalette = hSrcVObject->CurrentShade();
	LineSkip=(uiDestPitchBYTES-(usWidth*2));

	do
	{
		for (;;)
		{
			UINT8 data = *SrcPtr++;

			if (data == 0) break;
			if (data & 0x80)
			{
				data &= 0x7F;
				DestPtr += 2 * data;
				ZPtr += 2 * data;
			}
			else
			{
				do
				{
					if (*(UINT16*)ZPtr < usZValue)
					{
						UINT8 px = *SrcPtr;
						if (px != 254)
						{
							*(UINT16*)DestPtr = p16BPPPalette[px];
						}
					}
				}
				while (SrcPtr++, DestPtr += 2, ZPtr += 2, --data > 0);
			}
		}
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while (--usHeight > 0);
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferIntensityZ

	Creates a shadow using a brush, but modifies the destination buffer only if the current
	Z level is equal to higher than what's in the Z buffer at that pixel location. It
	updates the Z buffer with the new Z level.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBufferIntensityZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
#if 1 // XXX TODO
	UNIMPLEMENTED
#else
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	// Validations
	CHECKV(iTempX >= 0);
	CHECKV(iTempY >= 0);

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
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
#endif
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferIntensityZClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBufferIntensityZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
#if 1 // XXX TODO
	UNIMPLEMENTED
#else
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	UINT32 Unblitted;
	INT32  LSCount;
	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET IntensityTable
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0		// check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:						// Skips the number of lines clipped at the top

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
		je		LSSkip2			// if equal, skip whole, and start blit with new run
		jb		LSSkip1			// if less, skip whole thing

		add		esi, LSCount		// skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx		// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx		// skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup		// if equal, skip whole, and start blit with new run
		jb		LSTrans1		// if less, skip whole thing

		sub		ecx, LSCount		// skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx		// skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:						// Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0		// Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:					// blit non-transparent pixels

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

BlitTransparent:					// skip transparent pixels

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


RightSkipLoop:						// skip along until we hit and end-of-line marker


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
#endif
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferIntensityZNB

	Creates a shadow using a brush, but modifies the destination buffer only if the current
	Z level is equal to higher than what's in the Z buffer at that pixel location. It does
	NOT update the Z buffer with the new Z value.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBufferIntensityZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex )
{
#if 1 // XXX TODO
	UNIMPLEMENTED
#else
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	// Validations
	CHECKV(iTempX >= 0);
	CHECKV(iTempY >= 0);

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
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
#endif
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferIntensityClip

	Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
	image as a mask. Any Non-zero index pixels are used to darken destination pixels. Blitter
	clips brush if it doesn't fit on the viewport.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBufferIntensityClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
#if 1 // XXX TODO
	UNIMPLEMENTED
#else
	UINT8  *DestPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));

	UINT32 Unblitted;
	__asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET IntensityTable
		xor		eax, eax
		mov		ebx, TopSkip
		xor		ecx, ecx

		or		ebx, ebx		// check for nothing clipped on top
		jz		LeftSkipSetup

TopSkipLoop:						// Skips the number of lines clipped at the top

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
		mov		ebx, LeftSkip		// check for nothing clipped on the left
		or		ebx, ebx
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, ebx
		je		LSSkip2			// if equal, skip whole, and start blit with new run
		jb		LSSkip1			// if less, skip whole thing

		add		esi, ebx		// skip partial run, jump into normal loop for rest
		sub		ecx, ebx
		mov		ebx, BlitLength
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx		// skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx		// skip whole run, continue skipping
		sub		ebx, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, ebx
		je		BlitLineSetup		// if equal, skip whole, and start blit with new run
		jb		LSTrans1		// if less, skip whole thing

		sub		ecx, ebx		// skip partial run, jump into normal loop for rest
		mov		ebx, BlitLength
		jmp		BlitTransparent


LSTrans1:
		sub		ebx, ecx		// skip whole run, continue skipping
		jmp		LeftSkipLoop




BlitLineSetup:						// Does any actual blitting (trans/non) for the line
		mov		ebx, BlitLength
		mov		Unblitted, 0

BlitDispatch:

		or		ebx, ebx		// Check to see if we're done blitting
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
#endif
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferIntensity

	Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
	image as a mask. Any Non-zero index pixels are used to darken destination pixels.

**********************************************************************************************/
void Blt8BPPDataTo16BPPBufferIntensity( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex)
{
#if 1 // XXX TODO
	UNIMPLEMENTED
#else
	UINT8  *DestPtr;
	UINT32 LineSkip;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	// Validations
	CHECKV(iTempX >= 0);
	CHECKV(iTempY >= 0);

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*iTempY) + (iTempX*2);
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
#endif
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
void Blt8BPPDataTo16BPPBufferTransZClipPixelateObscured( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion)
{
	UINT32 uiLineFlag;
	UINT32 Unblitted;
	UINT8  *DestPtr, *ZPtr;
	UINT32 LineSkip;
	INT32  LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
	INT32  ClipX1, ClipY1, ClipX2, ClipY2;

	// Assertions
	Assert( hSrcVObject != NULL );
	Assert( pBuffer != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(usIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

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
	LeftSkip = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	RightSkip = std::clamp(iTempX + (INT32)usWidth - ClipX2, 0, (INT32)usWidth);
	TopSkip = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	BottomSkip = std::clamp(iTempY + (INT32)usHeight - ClipY2, 0, (INT32)usHeight);

	// calculate the remaining rows and columns to blit
	BlitLength=((INT32)usWidth-LeftSkip-RightSkip);
	BlitHeight=((INT32)usHeight-TopSkip-BottomSkip);

	// check if whole thing is clipped
	if((LeftSkip >=(INT32)usWidth) || (RightSkip >=(INT32)usWidth))
		return;

	// check if whole thing is clipped
	if((TopSkip >=(INT32)usHeight) || (BottomSkip >=(INT32)usHeight))
		return;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);
	DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES*(iTempY+TopSkip)) + ((iTempX+LeftSkip)*2);
	UINT16 const* const p16BPPPalette = hSrcVObject->CurrentShade();
	LineSkip=(uiDestPitchBYTES-(BlitLength*2));
	uiLineFlag=(iTempY&1);

	uiLineFlag = (iTempY + TopSkip) & 1;

	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		TopSkip--;
	}

	do
	{
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > static_cast<UINT32>(LSCount)) PxCount = LSCount;
				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
				ZPtr    += 2 * PxCount;
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > static_cast<UINT32>(LSCount))
				{
					Unblitted = PxCount - LSCount;
					PxCount = LSCount;
				}
				else
				{
					Unblitted = 0;
				}
				LSCount -= PxCount;

				do
				{
					if (*(UINT16*)ZPtr < usZValue)
					{
						*(UINT16*)ZPtr = usZValue;
					}
					else
					{
						if (uiLineFlag != (((uintptr_t)DestPtr & 2) != 0)) continue;
					}
					*(UINT16*)DestPtr = p16BPPPalette[*SrcPtr];
				}
				while (SrcPtr++, DestPtr += 2, ZPtr += 2, --PxCount > 0);
				SrcPtr += Unblitted;
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit and end-of-line marker
		DestPtr += LineSkip;
		ZPtr += LineSkip;
		uiLineFlag ^= 1;
	}
	while (--BlitHeight > 0);
}
