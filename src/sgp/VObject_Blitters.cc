#include <stdint.h>
#include "Debug.h"
#include "HImage.h"
#include "Shading.h"
#include "VObject.h"
#include "VObject_Blitters.h"
#include "WCheck.h"
#include "UILayout.h"
#include "RenderWorld.h"

#include <algorithm>
#include <cmath>
#include <utility>

UINT32	guiTranslucentMask=0x3def; //0x7bef;		// mask for halving 5,6,5

SGPRect	ClippingRect;


// original code used to make stuff semi-transparent by drawing every other pixel, new code can use opacity
#define OBSCURE_WITH_ALPHA			1	// 1: use alpha, 0: pixelate

#define BLT32_CLIP_NONE	(nullptr)
#define BLT32_CLIP(x)	(x == nullptr ? &ClippingRect : x)

// BLT32_BLEND_COPY - blits all pixels, this and all other modes blend opacity
#define BLT32_BLEND_COPY			1
// BLT32_BLEND_SHADE - darkens destination pixels by 50% black using - considers source opacity
#define BLT32_BLEND_SHADE			2
// BLT32_BLEND_OBSCURED - blits all pixels, if blit is under z-level - blit is semi-transparent
#define BLT32_BLEND_OBSCURED			3
// HACK254 is mode used by original 8bpp resources. Color index 254 was used only to draw shadows.
// And shadows were drawn to different z-buffer level. In order not to split original image resources
// into two (one for colors and other for shadows) we're using 32bpp color 0x020504** to detect and
// draw shadows. Wherever this hack is used, code or comment MUST contain word HACK254, so it's easier
// to change it in the future.
// BLT32_BLEND_HACK254 - blits color and shadow pixels, shadow pixels are 1 z-level below
#define BLT32_BLEND_HACK254			4
// BLT32_BLEND_HACK254_OBSCURED - blits color and shadow pixels, if blit is under z-level - blit is semi-transparent
#define BLT32_BLEND_HACK254_OBSCURED		5

#define BLT32_Z_NONE	(FALSE)
#define BLT32_Z_CHECK	(FALSE)
#define BLT32_Z_WRITE	(TRUE)

#define BLT32_MULTI_Z	(TRUE)

#define BLT32_OUTLINE_NONE	(0x00000000)
#define BLT32_OUTLINE(color)	(color)

#define Z_STRIP_DELTA_Y  (Z_SUBLAYERS * 10)




/**
 * Kitchensink function used by other blit functions.
 * Compiler should be able to inline code and optimize parameters passed as constants in if() conditions
 */
static inline void
Blt32_Texture(
	UINT32 *buf, const UINT32 bufPitch, // destindation ptr and pitch
	const SGPVObject *srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, // src pointers and offsets
	const BOOLEAN zBufWrite, UINT16 *zBuf, UINT16 zVal, // are we using Z buffer - use BLT32_Z_* defines
	const SGPRect *clipRect, // are we clipping the blit - use BLT32_CLIP_* defines
	const UINT32 colOutline, // are we rendering outline - BLT32_OUTLINE_* defines
	const UINT8 blendType, // are we copying or shading - use BLT32_BLEND_* defines
	const BOOLEAN multiZ=FALSE, const INT16 zStrip=-1) // are we using Multi-Z-buffer - use BLT32_MULTI_Z defines
{
	Assert(srcObj != nullptr);
	Assert(srcObj->BPP() == 32);
	Assert(buf != nullptr);

	const ETRLEObject &etrle = srcObj->SubregionProperties(srcIndex);

	// start position of dest buffer
	const INT32 dstX = srcX + etrle.sOffsetX;
	const INT32 dstY = srcY + etrle.sOffsetY;

	INT32 blitWidth, blitHeight;

	const UINT8 *src;
	UINT8 *dst;
	UINT16 *zDst = nullptr;
	UINT32 srcLineSkip, dstLineSkip;

#if OBSCURE_WITH_ALPHA != 1
	bool everyOtherRow;
#endif

	INT32 srcLeft;
	if(clipRect == nullptr) {
		CHECKV(dstX >= 0);
		CHECKV(dstY >= 0);

		srcLeft = 0;

		blitWidth = etrle.usWidth;
		blitHeight = etrle.usHeight;

		src = reinterpret_cast<const UINT8 *>(srcObj->PixData(etrle));
		srcLineSkip = 0;

		dst = reinterpret_cast<UINT8 *>(buf)
			+ dstY * bufPitch
			+ dstX * sizeof(UINT32);
		dstLineSkip = bufPitch - etrle.usWidth * sizeof(UINT32);

		if(zBuf) {
			zDst = zBuf
				+ dstY * bufPitch / sizeof(UINT32)
				+ dstX;
		}

#if OBSCURE_WITH_ALPHA != 1
		if(blendType == BLT32_BLEND_OBSCURED || blendType == BLT32_BLEND_HACK254_OBSCURED)
			everyOtherRow = (offsetY & 1) ^ !(etrleBlitHeight & 1) ^ (offsetX & 1);
#endif
	} else {
		// Calculate how much is clipped from etrle image size
		srcLeft = std::min(clipRect->iLeft - std::min(int(clipRect->iLeft), dstX), int(etrle.usWidth));
		const INT32 srcRight = std::min(std::max(int(clipRect->iRight), dstX + etrle.usWidth) - clipRect->iRight, int(etrle.usWidth));
		const INT32 srcTop = std::min(clipRect->iTop - std::min(int(clipRect->iTop), dstY), int(etrle.usHeight));
		const INT32 srcBottom = std::min(std::max(int(clipRect->iBottom), dstY + etrle.usHeight) - clipRect->iBottom, int(etrle.usHeight));

		// calculate the remaining rows and columns to blit
		blitWidth = INT32(etrle.usWidth) - srcLeft - srcRight;
		blitHeight = INT32(etrle.usHeight) - srcTop - srcBottom;

		// check if whole etrle is clipped
		if(blitWidth <= 0 || blitHeight <= 0)
			return;

		src = reinterpret_cast<const UINT8 *>(srcObj->PixData(etrle))
			+ srcTop * etrle.usWidth * sizeof(UINT32)
			+ srcLeft * sizeof(UINT32);
		srcLineSkip = (srcLeft + srcRight) * sizeof(UINT32);

		dst = reinterpret_cast<UINT8 *>(buf)
			+ (dstY + srcTop) * bufPitch
			+ (dstX + srcLeft) * sizeof(UINT32);
		dstLineSkip = bufPitch - blitWidth * sizeof(UINT32);

		if(zBuf) {
			zDst = zBuf
				+ (dstY + srcTop) * bufPitch / sizeof(UINT32)
				+ (dstX + srcLeft);
		}

#if OBSCURE_WITH_ALPHA != 1
		if(blendType == BLT32_BLEND_OBSCURED || blendType == BLT32_BLEND_HACK254_OBSCURED)
			everyOtherRow = ((offsetY + srcTopSkip) & 1) ^ !(etrleBlitHeight & 1) ^ ((offsetX + srcLeftSkip) & 1);
#endif
	}

	UINT16 zColsInStripInit, zValInit, zStripIndexInit;
	const INT8 *zValChange;
	if(multiZ) {
		if(srcObj->ppZStripInfo == nullptr) {
			SLOGW("Missing Z-Strip info on multi-Z object");
			return;
		}
		// setup for the z-column blitting stuff

		const UINT16 zStripIndex = zStrip >= 0 ? zStrip : srcIndex;
		if(zStripIndex >= srcObj->SubregionCount()) {
			SLOGW("Invalid Z-Strip index requested on multi-Z object");
			return;
		}
		const ZStripInfo *zInfo = srcObj->ppZStripInfo[zStripIndex];
		if(zInfo == nullptr) {
			SLOGW("Missing Z-Strip info on multi-Z object");
			return;
		}

		zValInit = INT16(zVal) + zInfo->bInitialZChange * Z_STRIP_DELTA_Y;

		// first strip is (pZInfo->ubFirstZStripWidth) pixels wide, next are 20px each
		if(srcLeft > zInfo->ubFirstZStripWidth) {
			zColsInStripInit = srcLeft - zInfo->ubFirstZStripWidth;
			zColsInStripInit = 20 - zColsInStripInit % 20;
		} else if(srcLeft < zInfo->ubFirstZStripWidth) {
			zColsInStripInit = zInfo->ubFirstZStripWidth - srcLeft;
		} else {
			zColsInStripInit = 20;
		}

		zValChange = zInfo->pbZChange;

		if(srcLeft >= zInfo->ubFirstZStripWidth) {
			// Index into array after doing left clipping
			zStripIndexInit = 1 + (srcLeft - zInfo->ubFirstZStripWidth) / 20;

			// calculates the Z-value after left-side clipping
			for(UINT16 i = 0; i < zStripIndexInit; i++) {
				if(zValChange[i] == -1)
					zValInit -= Z_STRIP_DELTA_Y;
				else if(zValChange[i] == 1)
					zValInit += Z_STRIP_DELTA_Y;
			}
		} else {
			zStripIndexInit = 0;
		}
	}


	auto pixelCopy = [](UINT8 *dst, const UINT8 *src) {
		dst[0] = src[0] + (dst[0] * (255 - src[0]) / 255); // alpha
		dst[1] = (src[1] * src[0] / 255) + (dst[1] * (255 - src[0]) / 255); // blue
		dst[2] = (src[2] * src[0] / 255) + (dst[2] * (255 - src[0]) / 255); // green
		dst[3] = (src[3] * src[0] / 255) + (dst[3] * (255 - src[0]) / 255); // red
	};
	auto pixelCopyAlpha = [](UINT8 *dst, const UINT8 *src, const UINT8 &alpha) {
		dst[0] = alpha + (dst[0] * (255 - alpha) / 255); // alpha
		dst[1] = (src[1] * alpha / 255) + (dst[1] * (255 - alpha) / 255); // blue
		dst[2] = (src[2] * alpha / 255) + (dst[2] * (255 - alpha) / 255); // green
		dst[3] = (src[3] * alpha / 255) + (dst[3] * (255 - alpha) / 255); // red
	};
	auto pixelShade = [&pixelCopyAlpha](UINT8 *dst, const UINT8 *src, const UINT32 shade) {
		const UINT8 alpha = src[0]; // semi transparent shadow
		pixelCopyAlpha(dst, reinterpret_cast<const UINT8 *>(&shade), alpha);
	};

	bool borderLine = true;
	while(blitHeight-- > 0) {
		UINT32 w = blitWidth;
		bool borderRow = true;
#if OBSCURE_WITH_ALPHA != 1
		bool everyOtherPixel = everyOtherRow ^ !(etrleBlitHeight & 1);
#endif
		UINT16 zColsInStrip, zStripIndex;
		if(multiZ) {
			zVal = zValInit;
			zStripIndex = zStripIndexInit;
			zColsInStrip = zColsInStripInit;
		}

		while(w--) {
			if(src[0]) { // no need to do anything if source is fully transparent
				if(blendType == BLT32_BLEND_HACK254) {
					if(zBufWrite) {
						Assert(zDst != nullptr);
						if(*reinterpret_cast<UINT8 *>(zDst) < zVal) { // look BLT32_BLEND_HACK254_BROKEN_Z_8BITS definition for explanation
							*reinterpret_cast<UINT8 *>(zDst) = zVal;
							if(src[3] == 0x02 && src[2] == 0x05 && src[1] == 0x04) { // shadow pixel
								pixelShade(dst, src, 0x000000FF);
							} else {
								pixelCopy(dst, src);
							}
						}
					} else {
						if(src[3] == 0x02 && src[2] == 0x05 && src[1] == 0x04) { // shadow pixel
							if(!zDst || *zDst < zVal)
								pixelShade(dst, src, 0x000000FF);
						} else {
							if(!zDst || *zDst <= zVal)
								pixelCopy(dst, src);
						}
					}
				} else if(blendType == BLT32_BLEND_HACK254_OBSCURED) {
					if(src[3] == 0x02 && src[2] == 0x05 && src[1] == 0x04) { // shadow pixel
						if(!zDst || *zDst < zVal) {
							if(zDst && zBufWrite)
								*zDst = zVal;
							pixelShade(dst, src, 0x000000FF);
						}
					} else {
						if(!zDst || *zDst <= zVal) {
							if(zDst && zBufWrite)
								*zDst = zVal;
							pixelCopy(dst, src);
						} else {
#if OBSCURE_WITH_ALPHA == 1
							pixelCopyAlpha(dst, src, src[0] >> 1);
#else
							if(everyOtherPixel)
								pixelCopy(dst, src);
#endif
						}
					}
#if OBSCURE_WITH_ALPHA != 1
					everyOtherPixel = !everyOtherPixel;
#endif
				} else if(blendType == BLT32_BLEND_OBSCURED) {
					if(!zDst || *zDst < zVal) {
						if(zDst && zBufWrite)
							*zDst = zVal;
						pixelCopy(dst, src);
					} else {
#if OBSCURE_WITH_ALPHA == 1
						pixelCopyAlpha(dst, src, src[0] >> 1);
#else
						if(everyOtherPixel)
							pixelCopy(dst, src);
#endif
					}
#if OBSCURE_WITH_ALPHA != 1
					everyOtherPixel = !everyOtherPixel;
#endif
				} else if(blendType == BLT32_BLEND_SHADE) {
					if(!zDst || *zDst < zVal) {
						if(zDst && zBufWrite)
							*zDst = zVal;
						static const UINT32 coShadow = 0x0000007F; // black w/ 50% opacity
						pixelCopyAlpha(dst, reinterpret_cast<const UINT8 *>(&coShadow), (coShadow & 0xFF) * src[0] / 255);
					}
				} else if(blendType == BLT32_BLEND_COPY) {
					if(colOutline && (colOutline & 0xFF) && *src != 0xFF && !borderLine && !borderRow) {
						// sum alpha values of all surrounding pixels to scale outline color's alpha
						UINT16 ouAlpha = (
								*(src - 4 * etrle.usWidth - 4) + // top row pixels
								*(src - 4 * etrle.usWidth) +
								*(src - 4 * etrle.usWidth + 4) +
								*(src - 4) + // central row pixels
								*src +
								*(src + 4) +
								*(src + 4 * etrle.usWidth - 4) + // bottom row pixels
								*(src + 4 * etrle.usWidth) +
								*(src + 4 * etrle.usWidth + 4)
								) * 3 / 9; // full opacity when 3 surrounding pixels have full opacity
						ouAlpha = std::min(ouAlpha, UINT16(255)) * (colOutline & 0xFF) / 255;
						pixelCopyAlpha(dst, reinterpret_cast<const UINT8 *>(&colOutline), ouAlpha);
					}

					if(!zDst || *zDst <= zVal) {
						if(zDst && zBufWrite)
							*zDst = zVal;
						pixelCopy(dst, src);
					}
				} else {
					AssertMsg(FALSE, "Unuspported blit blending mode.");
				}
			}
			src += sizeof(UINT32);
			dst += sizeof(UINT32);
			if(zDst) zDst++;
			if(colOutline) borderRow = w == 1;
			if(multiZ && --zColsInStrip == 0) {
				zColsInStrip = 20;
				const INT8 delta = zValChange[zStripIndex++];
				if(delta < 0)
					zVal -= Z_STRIP_DELTA_Y;
				else if(delta > 0)
					zVal += Z_STRIP_DELTA_Y;
			}
		}
		src += srcLineSkip;
		dst += dstLineSkip;
		if(zDst) zDst += dstLineSkip / sizeof(UINT32);
		if(colOutline) borderLine = etrle.usHeight == 1;
	}
}

/* Blit an image into the destination buffer, using an ETRLE brush as a source,
 * and a 16-bit buffer as a destination. As it is blitting, it checks the Z
 * value of the ZBuffer, and if the pixel's Z level is below that of the current
 * pixel, it is written on, and the Z value is NOT updated to the current value,
 * for any non-transparent pixels. The Z-buffer is 16 bit, and must be the same
 * dimensions (including pitch) as the destination.
 * Blits every second pixel ("Translucents"). */
void Blt32BPPDataTo32BPPBufferTransZNBTranslucentClip(UINT32 *buf, UINT32 const uiDestPitchBYTES, const UINT16 *zbuf, UINT16 const zval, HVOBJECT const hSrcVObject, INT32 const iX, INT32 const iY, UINT16 const usIndex, SGPRect const* clipregion)
{
	Assert(FALSE); // maxrd2 todo
/*
	// translucent_mask = r | g | b; // r, g, b are mask of each component with highest bit unset
	if (*zdst <= zval) {
		*dst =
			(pal[*src] >> 1 & translucent_mask) +
			(*dst      >> 1 & translucent_mask);
	}
*/
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferTransZTranslucent

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

	Blits every second pixel ("Translucents").

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferTransZTranslucent(UINT32 *buf, UINT32 const uiDestPitchBYTES, UINT16 *zbuf, UINT16 const zval, HVOBJECT const hSrcVObject, INT32 const iX, INT32 const iY, UINT16 const usIndex )
{
	Assert(FALSE); // maxrd2 todo
/*
	// translucent_mask = r | g | b; // r, g, b are mask of each component with highest bit unset
	if (*zdst <= zval) {
		*zdst = zval;
		*dst =
			((pal[*src] >> 1) & translucent_mask) +
			((*dst      >> 1) & translucent_mask);
	}
*/
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
}


/* Blit an image into the destination buffer, using an ETRLE brush as a source,
 * and a 16-bit buffer as a destination. As it is blitting, it checks the Z
 * value of the ZBuffer, and if the pixel's Z level is below that of the current
 * pixel, it is written on, and the Z value is NOT updated to the current value,
 * for any non-transparent pixels. The Z-buffer is 16 bit, and must be the same
 * dimensions (including pitch) as the destination.
 * Blits every second pixel ("Translucents"). */
void Blt32BPPDataTo32BPPBufferTransZNBTranslucent(UINT32 *buf, UINT32 const uiDestPitchBYTES, const UINT16 *zbuf, UINT16 const zval, HVOBJECT const hSrcVObject, INT32 const iX, INT32 const iY, UINT16 const usIndex)
{
	Assert(FALSE); // maxrd2 todo
/*
	// translucent_mask = r | g | b; // r, g, b are mask of each component with highest bit unset
	if (*zdst <= zval) {
		*dst =
			((pal[*src] >> 1) & translucent_mask) +
			((*dst      >> 1) & translucent_mask);
	}
*/
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
}


UINT16* InitZBuffer(const UINT32 width, const UINT32 height)
{
	return new UINT16[width * height]{};
}


void ShutdownZBuffer(UINT16 *pBuffer)
{
	delete[] pBuffer;
}


/**********************************************************************************************
Blt8BPPDataTo32BPPBufferMonoShadowClip

	Uses a bitmap an 8BPP template for blitting. Anywhere a 1 appears in the bitmap, a shadow
	is blitted to the destination (a black pixel). Any other value above zero is considered a
	forground color, and zero is background. If the parameter for the background color is zero,
	transparency is used for the background.

**********************************************************************************************/
void Blt8BPPDataTo32BPPBufferMonoShadowClip(UINT32 *buf, const UINT32 bufPitch, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect, const UINT32 colFG, const UINT32 colBG, const UINT32 colSH)
{
	Assert(srcObj != nullptr);
	Assert(srcObj->BPP() == 8);
	Assert(buf != nullptr);

	const ETRLEObject &etrle = srcObj->SubregionProperties(srcIndex);

	// start position of dest buffer
	const INT32 dstX = srcX + etrle.sOffsetX;
	const INT32 dstY = srcY + etrle.sOffsetY;

	if(clipRect == nullptr)
		clipRect = &ClippingRect;

	// Calculate how much is clipped from etrle image size
	const INT32 srcLeft = std::min(clipRect->iLeft - std::min(int(clipRect->iLeft), dstX), int(etrle.usWidth));
	const INT32 srcRight = std::min(std::max(int(clipRect->iRight), dstX + etrle.usWidth) - clipRect->iRight, int(etrle.usWidth));
	const INT32 srcTop = std::min(int(clipRect->iTop) - std::min(int(clipRect->iTop), dstY), int(etrle.usHeight));
	const INT32 srcBottom = std::min(std::max(int(clipRect->iBottom), int(dstY + etrle.usHeight)) - clipRect->iBottom, int(etrle.usHeight));

	// calculate the remaining rows and columns to blit
	const INT32 blitWidth = INT32(etrle.usWidth) - srcLeft - srcRight;
	INT32 blitHeight = INT32(etrle.usHeight) - srcTop - srcBottom;

	// check if whole etrle is clipped
	if(blitWidth <= 0 || blitHeight <= 0)
		return;

	const UINT8 *src = srcObj->PixData(etrle)
			+ srcTop * etrle.usWidth
			+ srcLeft;
	const UINT8 *srcStart = src;
	const UINT8 *srcShadow = src
			- UINT32(std::round(1 * g_ui.m_stdScreenScale)) * etrle.usWidth
			- UINT32(std::round(1 * g_ui.m_stdScreenScale)); // offset shadow by 1px right and down
	UINT8 *dst = reinterpret_cast<UINT8 *>(buf)
			+ (dstY + srcTop) * bufPitch
			+ (dstX + srcLeft) * 4;
	const UINT32 lineSkip = bufPitch - (blitWidth * 4);

	const UINT8 *cBg = reinterpret_cast<const UINT8 *>(&colBG);
	const UINT8 *cFg = reinterpret_cast<const UINT8 *>(&colFG);
	const UINT8 *cSh = reinterpret_cast<const UINT8 *>(&colSH);

	// draw outline instead of shadow for smaller text
	const bool doOutline = etrle.usHeight < 10 * g_ui.m_stdScreenScale;

	while(blitHeight-- > 0) {
		UINT32 w = blitWidth;
		while(w--) {
			if(cBg[0]) {
				dst[0] = cBg[0] + (dst[0] * (255 - cBg[0]) / 255); // alpha
				dst[1] = (cBg[1] * cBg[0] / 255) + (dst[1] * (255 - cBg[0]) / 255); // blue
				dst[2] = (cBg[2] * cBg[0] / 255) + (dst[2] * (255 - cBg[0]) / 255); // green
				dst[3] = (cBg[3] * cBg[0] / 255) + (dst[3] * (255 - cBg[0]) / 255); // red
			}
			if(cSh[0] && srcShadow > srcStart) {
				// shadow
				UINT8 alpha;
				if(doOutline && blitHeight > 0 && w > 0) {
					// draw outline unless we're on last row/column
					UINT16 ouAlpha = (
							*(src - 1 * etrle.usWidth - 1) + // top row pixels
							*(src - 1 * etrle.usWidth) +
							*(src - 1 * etrle.usWidth + 1) +
							*(src - 1) + // central row pixels
							*src +
							*(src + 1) +
							*(src + 1 * etrle.usWidth - 1) + // bottom row pixels
							*(src + 1 * etrle.usWidth) +
							*(src + 1 * etrle.usWidth + 1)
							) * 3 / 9; // full opacity when 3 surrounding pixels have full opacity
					alpha = cSh[0] * UINT8(std::min(ouAlpha, UINT16(255))) / 255;
				} else {
					alpha = cSh[0] * *srcShadow / 255;
				}
				dst[0] = alpha + (dst[0] * (255 - alpha) / 255); // alpha
				dst[1] = (cSh[1] * alpha / 255) + (dst[1] * (255 - alpha) / 255); // blue
				dst[2] = (cSh[2] * alpha / 255) + (dst[2] * (255 - alpha) / 255); // green
				dst[3] = (cSh[3] * alpha / 255) + (dst[3] * (255 - alpha) / 255); // red
			}
			if(cFg[0]) {
				// foreground
				const UINT8 alpha = cFg[0] * *src / 255;
				dst[0] = alpha + (dst[0] * (255 - alpha) / 255); // alpha
				dst[1] = (cFg[1] * alpha / 255) + (dst[1] * (255 - alpha) / 255); // blue
				dst[2] = (cFg[2] * alpha / 255) + (dst[2] * (255 - alpha) / 255); // green
				dst[3] = (cFg[3] * alpha / 255) + (dst[3] * (255 - alpha) / 255); // red
			}
			src++;
			srcShadow++;
			dst += 4;
		}
		dst += lineSkip;
		src += (srcLeft + srcRight) * 4;
		srcShadow += (srcLeft + srcRight) * 4;
	}
}


/**********************************************************************************************
	Blt16BPPTo16BPP

	Copies a rect of 16 bit data from a video buffer to a buffer position of the brush
	in the data area, for later blitting. Used to copy background information for mercs
	etc. to their unblit buffer, for later reblitting. Does NOT clip.

**********************************************************************************************/
void Blt16BPPTo16BPP(UINT16 *pDest, UINT32 uiDestPitch, UINT16 *pSrc, UINT32 uiSrcPitch, INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos, UINT32 uiWidth, UINT32 uiHeight)
{
	for(UINT32 i = 0; i < uiHeight; i++) {
		memcpy(
			pDest + (uiDestPitch / 2) * (iDestYPos + i) + iDestXPos,
			pSrc  + (uiSrcPitch / 2)  * (iSrcYPos  + i) + iSrcXPos,
			uiWidth * 2
		);
	}
}


/**********************************************************************************************
	Blt32BPPTo32BPP

	Copies a rect of 32 bit data from a video buffer to a buffer position of the brush
	in the data area, for later blitting. Used to copy background information for mercs
	etc. to their unblit buffer, for later reblitting. Does NOT clip.

**********************************************************************************************/
void Blt32BPPTo32BPP(UINT32 *pDest, UINT32 uiDestPitch, UINT32 *pSrc, UINT32 uiSrcPitch, INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos, UINT32 uiWidth, UINT32 uiHeight)
{
	for(UINT32 i = 0; i < uiHeight; i++) {
		memcpy(
			pDest + (uiDestPitch / 4) * (iDestYPos + i) + iDestXPos,
			pSrc  + (uiSrcPitch / 4)  * (iSrcYPos  + i) + iSrcXPos,
			uiWidth * 4
		);
	}
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferTransZPixelateObscured

	// OK LIKE NORMAL PIXELATE BUT ONLY PIXELATES STUFF BELOW Z level

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

	Blits every second pixel ("pixelates").

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferTransZPixelateObscured(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_WRITE, zBuf, zVal,
		BLT32_CLIP_NONE,
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_OBSCURED);
}


/**
	Blit an image into the destination buffer, using an ETRLE brush as a source
	and a 32-bit buffer as a destination. As it is blitting, it checks the Z-
	value of the ZBuffer, and if the pixel's Z level is below that of the current
	pixel, it is written on, and the Z value is updated to the current value, for
	any non-transparent pixels. The Z-buffer is 16 bit, and must be the same
	dimensions (including Pitch) as the destination. */
void Blt32BPPDataTo32BPPBufferTransZ(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_WRITE, zBuf, zVal,
		BLT32_CLIP_NONE,
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_COPY);
}

/**
	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 32-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination. */
void Blt32BPPDataTo32BPPBufferTransZClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_WRITE, zBuf, zVal,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_COPY);
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferTransZNB

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 32-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on. The Z value is
	NOT updated by this version. The Z-buffer is 16 bit, and	must be the same dimensions
	(including Pitch) as the destination.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferTransZNB(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_CHECK, zBuf, zVal,
		BLT32_CLIP_NONE,
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_COPY);
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferTransZNBClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 32-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on. The Z value is NOT
	updated in this version. The Z-buffer is 16 bit, and must be the same dimensions (including
	Pitch) as the destination.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferTransZNBClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_CHECK, zBuf, zVal,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_COPY);
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferTransShadow

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. If the source pixel is 254,
	it is considered a shadow, and the destination buffer is darkened rather than blitted on.
	The Z-buffer is 16 bit, and	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferTransShadow(UINT32 *buf, const UINT32 bufPitch, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT16 *palette)
{
	// maxrd2 - FIXME - uses palette from param
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_NONE, nullptr, 0,
		BLT32_CLIP_NONE,
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_HACK254);
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferTransShadowZ

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 32-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. If the source pixel is 254,
	it is considered a shadow, and the destination buffer is darkened rather than blitted on.
	The Z-buffer is 16 bit, and	must be the same dimensions (including Pitch) as the destination.

	WARNING: ZBUFFER ain't written even if function's name implies so
             - this one is identical to Blt32BPPDataTo32BPPBufferTransShadowZNB()

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferTransShadowZ(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT16 *palette)
{
	// maxrd2 - FIXME - uses palette from param
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_CHECK, zBuf, zVal,
		BLT32_CLIP_NONE,
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_HACK254);
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferTransShadowZNB

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 32-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on. The Z value is NOT
	updated. If the source pixel is 254, it is considered a shadow, and the destination
	buffer is darkened rather than blitted on. The Z-buffer is 16 bit, and must be the same
	dimensions (including Pitch) as the destination.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferTransShadowZNB(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT16 *palette)
{
	// maxrd2 - FIXME - uses palette from param
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_CHECK, zBuf, zVal,
		BLT32_CLIP_NONE,
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_HACK254);
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferTransShadowZNBObscured

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 32-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on. The Z value is NOT
	updated. If the source pixel is 254, it is considered a shadow, and the destination
	buffer is darkened rather than blitted on. The Z-buffer is 16 bit, and must be the same
	dimensions (including Pitch) as the destination.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferTransShadowZNBObscured(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT16 *palette)
{
	// maxrd2 - FIXME - uses palette from param
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_CHECK, zBuf, zVal,
		BLT32_CLIP_NONE,
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_HACK254_OBSCURED);
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferTransShadowZNBObscuredClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 32-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on.
	The Z-buffer is 16 bit, and	must be the same dimensions (including Pitch) as the
	destination. Pixels with a value of	254 are shaded instead of blitted. The Z buffer is
	NOT updated.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferTransShadowZNBObscuredClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, SGPRect *clipRect, const UINT16 *palette)
{
	// maxrd2 - FIXME - uses palette from param
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_CHECK, zBuf, zVal,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_HACK254_OBSCURED);
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferTransShadowZClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 32-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination. Pixels with a value of
	254 are shaded instead of blitted.

	WARNING: originally just lower 8bits were written to z-buf

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferTransShadowZClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, SGPRect *clipRect, const UINT16 *palette)
{
	// maxrd2 - FIXME - uses palette from param
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_WRITE, zBuf, zVal,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_HACK254);
}

/**********************************************************************************************
Blt32BPPDataTo32BPPBufferTransShadowClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination. Pixels with a value of
	254 are shaded instead of blitted.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferTransShadowClip(UINT32 *buf, UINT32 bufPitch, HVOBJECT srcObj, INT32 srcX, INT32 srcY, UINT16 srcIndex, SGPRect *clipRect, const UINT16* palette)
{
	// maxrd2 - FIXME - uses palette from param
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_NONE, nullptr, 0,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_HACK254);
}

/**********************************************************************************************
Blt32BPPDataTo32BPPBufferTransShadowZNBClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 32-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on.
	The Z-buffer is 16 bit, and	must be the same dimensions (including Pitch) as the
	destination. Pixels with a value of	254 are shaded instead of blitted. The Z buffer is
	NOT updated.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferTransShadowZNBClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, SGPRect *clipRect, const UINT16 *palette)
{
	// maxrd2 - FIXME - uses palette from param
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_CHECK, zBuf, zVal,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_HACK254);
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferShadow

	Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
	image as a mask. Any Non-zero index pixels are used to darken destination pixels.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferShadow(UINT32 *buf, const UINT32 bufPitch, const SGPVObject *srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex)
{
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_NONE, nullptr, 0,
		BLT32_CLIP_NONE,
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_SHADE);
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferShadowClip

	Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
	image as a mask. Any Non-zero index pixels are used to darken destination pixels. Blitter
	clips brush if it doesn't fit on the viewport.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferShadowClip(UINT32 *buf, const UINT32 bufPitch, const SGPVObject *srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect)
{
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_NONE, nullptr, 0,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_SHADE);
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferShadowZ

	Creates a shadow using a brush, but modifies the destination buffer only if the current
	Z level is equal to higher than what's in the Z buffer at that pixel location. It
	updates the Z buffer with the new Z level.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferShadowZ(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex)
{
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_WRITE, zBuf, zVal,
		BLT32_CLIP_NONE,
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_SHADE);
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferShadowZClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 32-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferShadowZClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect)
{
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_WRITE, zBuf, zVal,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_SHADE);
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferShadowZNB

	Creates a shadow using a brush, but modifies the destination buffer only if the current
	Z level is equal to higher than what's in the Z buffer at that pixel location. It does
	NOT update the Z buffer with the new Z value.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferShadowZNB(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex)
{
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_CHECK, zBuf, zVal,
		BLT32_CLIP_NONE,
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_SHADE);
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferShadowZNBClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 32-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, the Z value is
	not updated,	for any non-transparent pixels. The Z-buffer is 16 bit, and	must be the
	same dimensions (including Pitch) as the destination.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferShadowZNBClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect)
{
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_CHECK, zBuf, zVal,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_SHADE);
}


SGPRect SetClippingRect(SGPRect const clip)
{
	Assert(clip.iLeft < clip.iRight && clip.iTop < clip.iBottom);
	return std::exchange(ClippingRect, clip);
}


SGPRect GetClippingRect()
{
	return ClippingRect;
}


/**********************************************************************************************
	Blt32BPPBufferPixelateRectWithColor

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
static void Blt32BPPBufferPixelateRectWithColor(UINT32* buf, const UINT32 bufPitch, const SGPRect *bufRect, const UINT8 pattern[8][8], const UINT32 color)
{
	Assert(buf != nullptr);
	Assert(pattern != nullptr);

	const INT32 skipLeft = std::max(ClippingRect.iLeft, bufRect->iLeft);
	const INT32 skipTop = std::max(ClippingRect.iTop, bufRect->iTop);
	const INT32 skipRight = std::min(ClippingRect.iRight - 1, int(bufRect->iRight));
	const INT32 skipBottom = std::min(ClippingRect.iBottom - 1, int(bufRect->iBottom));

	UINT32 *dst = reinterpret_cast<UINT32 *>(buf)
			+ skipTop * (bufPitch / 4)
			+ skipLeft;
	const INT32 width = skipRight - skipLeft + 1;
	INT32 height = skipBottom - skipTop + 1;
	const UINT32 lineSkip = bufPitch / 4 - width;

	CHECKV(width >= 1);
	CHECKV(height >= 1);

	UINT32 row = 0;
	while(height--) {
		UINT32 col = 0;
		UINT32 w = width;
		while(w--) {
			if(pattern[row][col] != 0)
				*dst = color;
			col = (col + 1) % 8;
			dst++;
		}
		dst += lineSkip;
		row = (row + 1) % 8;
	}
}


//Uses black hatch color
void Blt32BPPBufferHatchRect(UINT32 *buf, UINT32 bufPitch, SGPRect *area)
{
	const UINT8 Pattern[8][8] = {
		{ 1,0,1,0,1,0,1,0 },
		{ 0,1,0,1,0,1,0,1 },
		{ 1,0,1,0,1,0,1,0 },
		{ 0,1,0,1,0,1,0,1 },
		{ 1,0,1,0,1,0,1,0 },
		{ 0,1,0,1,0,1,0,1 },
		{ 1,0,1,0,1,0,1,0 },
		{ 0,1,0,1,0,1,0,1 }
	};
	Blt32BPPBufferPixelateRectWithColor(buf, bufPitch, area, Pattern, 0x000000FF);
}

void Blt32BPPBufferLooseHatchRectWithColor(UINT32 *buf, UINT32 bufPitch, SGPRect *area, UINT32 color)
{
	const UINT8 Pattern[8][8] = {
		{ 1,0,0,0,1,0,0,0 },
		{ 0,0,0,0,0,0,0,0 },
		{ 0,0,1,0,0,0,1,0 },
		{ 0,0,0,0,0,0,0,0 },
		{ 1,0,0,0,1,0,0,0 },
		{ 0,0,0,0,0,0,0,0 },
		{ 0,0,1,0,0,0,1,0 },
		{ 0,0,0,0,0,0,0,0 }
	};
	Blt32BPPBufferPixelateRectWithColor(buf, bufPitch, area, Pattern, color);
}


void Blt32BPPDataTo32BPPBufferTransparent(UINT32 *buf, const UINT32 bufPitch, const SGPVObject *srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_NONE, nullptr, 0,
		BLT32_CLIP_NONE,
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_COPY);
}

void Blt32BPPDataTo32BPPBufferTransparentClip(UINT32 *buf, const UINT32 bufPitch, const SGPVObject *srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_NONE, nullptr, 0,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_COPY);
}


/**********************************************************************************************
BltIsClipped

	Determines whether a given blit will need clipping or not. Returns TRUE/FALSE.

**********************************************************************************************/

BOOLEAN BltIsClipped(const SGPVObject *srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect)
{
	Assert(srcObj != nullptr);
	Assert(srcObj->BPP() == 32);

	// Get Offsets from Index into structure
	const ETRLEObject &etrle = srcObj->SubregionProperties(srcIndex);
	const UINT32 etrleHeight = etrle.usHeight;
	const UINT32 etrleWidth  = etrle.usWidth;

	// Add to start position of dest buffer
	const INT32 offsetX = srcX + etrle.sOffsetX;
	const INT32 offsetY = srcY + etrle.sOffsetY;

	if(clipRect == nullptr)
		clipRect = &ClippingRect;

	INT32 ClipX1 = clipRect->iLeft;
	INT32 ClipY1 = clipRect->iTop;
	INT32 ClipX2 = clipRect->iRight;
	INT32 ClipY2 = clipRect->iBottom;


	// Calculate rows hanging off each side of the screen
	if(std::min(ClipX1 - std::min(ClipX1, offsetX), (INT32)etrleWidth))
		return(TRUE);

	if(std::min(std::max(ClipX2, (offsetX + (INT32)etrleWidth)) - ClipX2, (INT32)etrleWidth))
		return(TRUE);

	if(std::min(ClipY1 - std::min(ClipY1, offsetY), (INT32)etrleHeight))
		return(TRUE);

	if(std::min(std::max(ClipY2, (offsetY + (INT32)etrleHeight)) - ClipY2, (INT32)etrleHeight))
		return(TRUE);

	return(FALSE);
}


void Blt32BPPBufferFilterRect(UINT32 *buf, UINT32 bufPitch, const FLOAT filterPercent, SGPRect *area)
{
	Assert(buf != nullptr);

	// Clipping
	if(area->iLeft < ClippingRect.iLeft)
		area->iLeft = ClippingRect.iLeft;
	if(area->iTop < ClippingRect.iTop)
		area->iTop = ClippingRect.iTop;
	if(area->iRight >= ClippingRect.iRight)
		area->iRight = ClippingRect.iRight - 1;
	if(area->iBottom >= ClippingRect.iBottom)
		area->iBottom = ClippingRect.iBottom - 1;

	UINT8 *dst = reinterpret_cast<UINT8 *>(buf)
			+ area->iTop * bufPitch
			+ area->iLeft * 4;
	const INT32 width = area->iRight - area->iLeft + 1;
	INT32 height = area->iBottom - area->iTop + 1;
	const UINT32 lineSkip = bufPitch - width * 4;

	CHECKV(width  >= 1);
	CHECKV(height >= 1);

	while(height-- > 0) {
		UINT32 w = width;
		while(w-- > 0) {
			dst++; // skip alpha
			*dst = FLOAT(*dst) * filterPercent; dst++;// blue
			*dst = FLOAT(*dst) * filterPercent; dst++; // green
			*dst = FLOAT(*dst) * filterPercent; dst++; // red
		}
		dst += lineSkip;
	}
}


/**********************************************************************************************
BltIsClippedOrOffScreen

	Determines whether a given blit will need clipping or not. Returns TRUE/FALSE.

**********************************************************************************************/
CHAR8 BltIsClippedOrOffScreen(const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect)
{
	Assert( srcObj != nullptr );

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = srcObj->SubregionProperties(srcIndex);
	UINT32      const  usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = srcX + pTrav.sOffsetX;
	INT32 const iTempY = srcY + pTrav.sOffsetY;

	if(clipRect == nullptr)
		clipRect = &ClippingRect;

	INT32 ClipX1 = clipRect->iLeft;
	INT32 ClipY1 = clipRect->iTop;
	INT32 ClipX2 = clipRect->iRight;
	INT32 ClipY2 = clipRect->iBottom;

	// Calculate rows hanging off each side of the screen
	INT32 gLeftSkip   = std::min(ClipX1 - std::min(ClipX1, iTempX), (INT32)usWidth);
	INT32 gTopSkip    = std::min(ClipY1 - std::min(ClipY1, iTempY), (INT32)usHeight);
	INT32 gRightSkip  = std::min(std::max(ClipX2, iTempX + (INT32)usWidth)  - ClipX2, (INT32)usWidth);
	INT32 gBottomSkip = std::min(std::max(ClipY2, iTempY + (INT32)usHeight) - ClipY2, (INT32)usHeight);

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
void Blt32BPPDataTo32BPPBufferOutline(UINT32 *buf, const UINT32 bufPitch, const SGPVObject *srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT32 colOutline)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_NONE, nullptr, 0,
		BLT32_CLIP_NONE,
		BLT32_OUTLINE(colOutline),
		BLT32_BLEND_COPY);
}


// ATE New blitter for rendering a differrent color for value 254. Can be transparent if s16BPPColor is SGP_TRANSPARENT
void Blt32BPPDataTo32BPPBufferOutlineClip(UINT32 *buf, const UINT32 bufPitch, const SGPVObject *srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT32 colOutline, const SGPRect *clipRect)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_NONE, nullptr, 0,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE(colOutline),
		BLT32_BLEND_COPY);
}


void Blt32BPPDataTo32BPPBufferOutlineZ(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT32 colOutline)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_WRITE, zBuf, zVal,
		BLT32_CLIP_NONE,
		BLT32_OUTLINE(colOutline),
		BLT32_BLEND_COPY);
}

void Blt32BPPDataTo32BPPBufferOutlineZClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const SGPVObject *srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT32 colOutline, const SGPRect *clipRect)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_WRITE, zBuf, zVal,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE(colOutline),
		BLT32_BLEND_COPY);
}

/**
 * WARNING: this function doesn't draw an outline even if it's name implies so
 */
void Blt32BPPDataTo32BPPBufferOutlineZNB(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_CHECK, zBuf, zVal - 1,
		BLT32_CLIP_NONE,
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_COPY);
}

void Blt32BPPDataTo32BPPBufferOutlineZPixelateObscuredClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT32 colShadow, const SGPRect *clipRect)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_WRITE, zBuf, zVal,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_HACK254_OBSCURED);
}

void Blt32BPPDataTo32BPPBufferOutlineZPixelateObscured(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT32 colShadow)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_WRITE, zBuf, zVal,
		BLT32_CLIP_NONE,
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_HACK254_OBSCURED);
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferIntensityZ

	Creates a shadow using a brush, but modifies the destination buffer only if the current
	Z level is equal to higher than what's in the Z buffer at that pixel location. It
	updates the Z buffer with the new Z level.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferIntensityZ(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex)
{
	// Unimplemented - should use:
	// BLT32_Z_WRITE - guessed from function name
	// BLT32_CLIP_NONE
	// BLT32_OUTLINE_NONE

#if 1 // XXX TODO
	(void)srcObj;
	UNIMPLEMENTED
#else
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
Blt32BPPDataTo32BPPBufferIntensityZClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferIntensityZClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect)
{
	// Unimplemented - should use:
	// BLT32_Z_WRITE - guessed from function name
	// BLT32_CLIP(clipRect)
	// BLT32_OUTLINE_NONE

#if 1 // XXX TODO
	(void)srcObj;
	UNIMPLEMENTED
#else
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
Blt32BPPDataTo32BPPBufferIntensityZNB

	Creates a shadow using a brush, but modifies the destination buffer only if the current
	Z level is equal to higher than what's in the Z buffer at that pixel location. It does
	NOT update the Z buffer with the new Z value.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferIntensityZNB(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex)
{
	// Unimplemented - should use:
	// BLT32_Z_CHECK - guessed from function name
	// BLT32_CLIP_NONE
	// BLT32_OUTLINE_NONE

#if 1 // XXX TODO
	(void)srcObj;
	UNIMPLEMENTED
#else
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
Blt32BPPDataTo32BPPBufferIntensityClip

	Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
	image as a mask. Any Non-zero index pixels are used to darken destination pixels. Blitter
	clips brush if it doesn't fit on the viewport.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferIntensityClip(UINT32 *buf, const UINT32 bufPitch, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect)
{
	// Unimplemented - should use:
	// BLT32_Z_NONE
	// BLT32_CLIP(clipRect)
	// BLT32_OUTLINE_NONE

#if 1 // XXX TODO
	(void)srcObj;
	UNIMPLEMENTED
#else
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
Blt32BPPDataTo32BPPBufferIntensity

	Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
	image as a mask. Any Non-zero index pixels are used to darken destination pixels.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferIntensity(UINT32 *buf, const UINT32 bufPitch, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex)
{
	// Unimplemented - should use:
	// BLT32_Z_NONE
	// BLT32_CLIP_NONE
	// BLT32_OUTLINE_NONE

#if 1 // XXX TODO
	(void)srcObj;
	UNIMPLEMENTED
#else
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
Blt32BPPDataTo32BPPBufferTransZClipPixelateObscured

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	NOT updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

	Blits every second pixel ("pixelates").

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferTransZPixelateObscuredClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_WRITE, zBuf, zVal,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_OBSCURED);
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferTransZIncClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value, for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferTransZIncClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_WRITE, zBuf, zVal - 1,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_COPY,
		BLT32_MULTI_Z);
}

/**********************************************************************************************
Blt32BPPDataTo32BPPBufferTransZIncClipSaveZBurnsThrough

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value, for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferTransZIncClipZSameZBurnsThrough(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_WRITE, zBuf, zVal,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_COPY,
		BLT32_MULTI_Z);
}


/**********************************************************************************************
Blt32BPPDataTo32BPPBufferTransZIncObscureClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value, for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

	//ATE: This blitter makes the values that are =< z value pixellate rather than not
	// render at all

**********************************************************************************************/
void Blt32BPPDataTo32BPPBufferTransZIncObscureClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect)
{
	// maxrd2 - FIXME - uses srcObj->CurrentShade();
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_WRITE, zBuf, zVal,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_OBSCURED,
		BLT32_MULTI_Z);
}


/* Blitter Specs
	* 1) 8 to 16 bpp
	* 2) strip z-blitter
	* 3) clipped
	* 4) trans shadow - if value is 254, makes a shadow */
void Blt32BPPDataTo32BPPBufferTransZTransShadowIncObscureClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect, const INT16 zStrip, const UINT16* palette)
{
	// maxrd2 - FIXME - uses palette from param
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_WRITE, zBuf, zVal,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_HACK254_OBSCURED,
		BLT32_MULTI_Z, zStrip);
}

/* Blitter Specs
	* 1) 8 to 16 bpp
	* 2) strip z-blitter
	* 3) clipped
	* 4) trans shadow - if value is 254, makes a shadow */
void Blt32BPPDataTo32BPPBufferTransZTransShadowIncClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect, const INT16 zStrip, const UINT16* palette)
{
	// maxrd2 - FIXME - uses palette from param
	Blt32_Texture(buf, bufPitch, srcObj, srcX, srcY, srcIndex,
		BLT32_Z_WRITE, zBuf, zVal,
		BLT32_CLIP(clipRect),
		BLT32_OUTLINE_NONE,
		BLT32_BLEND_HACK254,
		BLT32_MULTI_Z, zStrip);
}


