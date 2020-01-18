#include "VideoScale.h"

#include "VObject.h"
#include "Debug.h"

#include <cmath>

template <class PixelType>
static inline void DecodeTRLE8(const ETRLEObject *dstEtrle, UINT8 *dstBuf, const SGPImage *srcImage);

template <class PixelType>
static void ScaleImage(UINT8 *dstBuf, UINT16 dstWidth, UINT16 dstHeight, const UINT8 *srcBuf, UINT16 srcWidth, UINT16 srcHeight, bool yInterpolation);

template <class PixelType>
static inline void pixelInterpolate(PixelType *dst, const PixelType *src, FLOAT fract, bool set);

template <class PixelType>
static inline PixelType pixelFromPalette(const SGPPaletteEntry *pal, const UINT8 src);


SGPImage * ScaleImage(SGPImage *image, DOUBLE factor, bool yInterpolation)
{
	AutoSGPImage scaled(new SGPImage(std::ceil(factor * image->usWidth), std::ceil(factor * image->usHeight), 32));

	if(image->fFlags & IMAGE_BITMAPDATA) {
		scaled->fFlags |= IMAGE_BITMAPDATA;

		SGP::Buffer<UINT8> pImageData;
		SGP::Buffer<ETRLEObject> pETRLEObject;
		UINT32 uiSizePixData = 0;

		if(image->fFlags & IMAGE_TRLECOMPRESSED) {
			scaled->usNumberOfObjects = image->usNumberOfObjects;
			scaled->pETRLEObject.Allocate(scaled->usNumberOfObjects);

			pETRLEObject.Allocate(scaled->usNumberOfObjects);
			memcpy(pETRLEObject, image->pETRLEObject, scaled->usNumberOfObjects * sizeof(ETRLEObject));

			scaled->uiSizePixData = 0;
			for(int i = 0; i < scaled->usNumberOfObjects; i++) {
				ETRLEObject *dst = &scaled->pETRLEObject[i];
				const ETRLEObject *src = &image->pETRLEObject[i];
				dst->sOffsetX = std::floor(factor * src->sOffsetX);
				dst->sOffsetY = std::floor(factor * src->sOffsetY);
				dst->usWidth = std::ceil(factor * src->usWidth);
				dst->usHeight = std::ceil(factor * src->usHeight);
				dst->uiDataOffset = scaled->uiSizePixData;
				dst->uiDataLength = dst->usWidth * dst->usHeight * sizeof(UINT32);
				scaled->uiSizePixData += dst->uiDataLength;

				pETRLEObject[i].uiDataOffset = uiSizePixData;
				pETRLEObject[i].uiDataLength = src->usWidth * src->usHeight * sizeof(UINT32);
				uiSizePixData += pETRLEObject[i].uiDataLength;
			}
			scaled->pImageData.Allocate(scaled->uiSizePixData);
			pImageData.Allocate(uiSizePixData);

			if(image->ubBitDepth == 8)
				DecodeTRLE8<UINT32>(pETRLEObject, pImageData, image);
			else
				Assert(FALSE); // we shouldn't end up here

			for(int i = 0; i < scaled->usNumberOfObjects; i++) {
				const ETRLEObject *dst = &scaled->pETRLEObject[i];
				const ETRLEObject *src = &pETRLEObject[i];
				ScaleImage<UINT32>(&scaled->pImageData[dst->uiDataOffset], dst->usWidth, dst->usHeight,
						&pImageData[src->uiDataOffset], src->usWidth, src->usHeight, yInterpolation);
			}

			scaled->fFlags |= IMAGE_TRLECOMPRESSED;
		} else {
			scaled->pImageData.Allocate(scaled->usWidth * scaled->usHeight * sizeof(UINT32));
			uiSizePixData = image->usWidth * image->usHeight * sizeof(UINT32);
			pImageData.Allocate(uiSizePixData);
			UINT32 *dst = reinterpret_cast<UINT32 *>(static_cast<UINT8 *>(pImageData));
			if(image->ubBitDepth == 8) {
				const UINT8 *src = image->pImageData;
				UINT32 len = image->usWidth * image->usHeight;
				while(len--) {
					const SGPPaletteEntry &pal = image->pPalette[*src++];
					*dst++ = UINT32(pal.r) << 24 | UINT32(pal.g) << 16 | UINT32(pal.b) << 8 | UINT32(pal.a);
				}
			} else if(image->ubBitDepth == 16) {
				const UINT16 *src = reinterpret_cast<const UINT16 *>(static_cast<const UINT8 *>(image->pImageData));
				UINT32 len = image->usWidth * image->usHeight;
				while(len--) {
					const UINT32 r = *src >> 11 & 0x1F;
					const UINT32 g = *src >> 5 & 0x3F;
					const UINT32 b = *src & 0x1F;
					*dst++ = (r << 3 | r >> 2) << 24 | (g << 2 | g >> 4) << 16 | (b << 3 | b >> 2) << 8 | 0xFF;
					src++;
				}
			} else {
				Assert(FALSE); // we shouldn't end up here
			}

			ScaleImage<UINT32>(scaled->pImageData, scaled->usWidth, scaled->usHeight, pImageData, image->usWidth, image->usHeight, yInterpolation);
		}
	}

	if(image->fFlags & IMAGE_APPDATA) {
		UINT8 *buf = scaled->pAppData.Allocate(image->uiAppDataSize);
		memcpy(buf, image->pAppData, image->uiAppDataSize);

		scaled->uiAppDataSize = image->uiAppDataSize;
		scaled->fFlags |= IMAGE_APPDATA;
	}

	return scaled.release();
}

SGPImage * ScaleAlphaImage(SGPImage *image, DOUBLE factor, bool yInterpolation)
{
	Assert(image->ubBitDepth == 8);

	AutoSGPImage scaled(new SGPImage(factor * image->usWidth, factor * image->usHeight, 8));

	if(image->fFlags & IMAGE_PALETTE) {
		SGPPaletteEntry *palette = scaled->pPalette.Allocate(256);
		// FIXME: maxrd2 we don't care about palette actually... drop this block
		memcpy(palette, image->pPalette, sizeof(SGPPaletteEntry) * 256);

		scaled->fFlags |= IMAGE_PALETTE;
	}

	if(image->fFlags & IMAGE_BITMAPDATA) {
		scaled->fFlags |= IMAGE_BITMAPDATA;

		SGP::Buffer<UINT8> pImageData;
		SGP::Buffer<ETRLEObject> pETRLEObject;
		UINT32 uiSizePixData = 0;

		if(image->fFlags & IMAGE_TRLECOMPRESSED) {
			scaled->usNumberOfObjects = image->usNumberOfObjects;
			scaled->pETRLEObject.Allocate(scaled->usNumberOfObjects);

			pETRLEObject.Allocate(scaled->usNumberOfObjects);
			memcpy(pETRLEObject, image->pETRLEObject, scaled->usNumberOfObjects * sizeof(ETRLEObject));

			scaled->uiSizePixData = 0;
			for(int i = 0; i < scaled->usNumberOfObjects; i++) {
				ETRLEObject *dst = &scaled->pETRLEObject[i];
				const ETRLEObject *src = &image->pETRLEObject[i];
				dst->sOffsetX = factor * src->sOffsetX;
				dst->sOffsetY = factor * src->sOffsetY;
				dst->usWidth = factor * src->usWidth;
				dst->usHeight = factor * src->usHeight;
				dst->uiDataOffset = scaled->uiSizePixData;
				dst->uiDataLength = dst->usWidth * dst->usHeight * sizeof(UINT8);
				scaled->uiSizePixData += dst->uiDataLength;

				pETRLEObject[i].uiDataOffset = uiSizePixData;
				pETRLEObject[i].uiDataLength = src->usWidth * src->usHeight * sizeof(UINT8);
				uiSizePixData += pETRLEObject[i].uiDataLength;
			}
			scaled->pImageData.Allocate(scaled->uiSizePixData);
			pImageData.Allocate(uiSizePixData);

			if(image->ubBitDepth == 8)
				DecodeTRLE8<UINT8>(pETRLEObject, pImageData, image);
			else
				Assert(FALSE); // we shouldn't end up here

			for(int i = 0; i < scaled->usNumberOfObjects; i++) {
				const ETRLEObject *dst = &scaled->pETRLEObject[i];
				const ETRLEObject *src = &pETRLEObject[i];
				ScaleImage<UINT8>(&scaled->pImageData[dst->uiDataOffset], dst->usWidth, dst->usHeight,
						&pImageData[src->uiDataOffset], src->usWidth, src->usHeight, yInterpolation);
			}

			scaled->fFlags |= IMAGE_TRLECOMPRESSED;
		} else {
			scaled->pImageData.Allocate(scaled->usWidth * scaled->usHeight * sizeof(UINT8));
			uiSizePixData = image->usWidth * image->usHeight * sizeof(UINT8);
			ScaleImage<UINT8>(scaled->pImageData, scaled->usWidth, scaled->usHeight, image->pImageData, image->usWidth, image->usHeight, yInterpolation);
		}
	}

	if(image->fFlags & IMAGE_APPDATA) {
		UINT8 *buf = scaled->pAppData.Allocate(image->uiAppDataSize);
		memcpy(buf, image->pAppData, image->uiAppDataSize);

		scaled->uiAppDataSize = image->uiAppDataSize;
		scaled->fFlags |= IMAGE_APPDATA;
	}

	return scaled.release();
}

template <class PixelType>
inline void DecodeTRLE8(const ETRLEObject *dstEtrle, UINT8 *dstBuf, const SGPImage *srcImage)
{
	Assert(srcImage->pPalette);
	Assert(dstBuf != NULL);

	for(UINT16 o = 0; o < srcImage->usNumberOfObjects; o++) {
		const ETRLEObject *etrle = &dstEtrle[o];
		const UINT8 *src = srcImage->pImageData + srcImage->pETRLEObject[o].uiDataOffset;
		PixelType *dst = reinterpret_cast<PixelType *>(dstBuf + etrle->uiDataOffset);

		UINT16 linesLeft = etrle->usHeight;
		while(linesLeft--) {
			while(*src) {
				INT16 chunkLen = *src & 0x7F;
				if(*src++ & 0x80) {
					// transparent bytes
					memset(dst, 0, sizeof(PixelType) * chunkLen);
					dst += chunkLen;
				} else {
					// palette entries
					while(chunkLen-- > 0) {
						*dst++ = pixelFromPalette<PixelType>(&srcImage->pPalette[*src], *src);
						src++;
					}
				}
			}
			src++;
		}

		// make sure decode produced correct number of bytes
		Assert(etrle->uiDataLength == reinterpret_cast<UINT8 *>(dst) - (dstBuf + etrle->uiDataOffset));
	}
}

template <class PixelType>
void ScaleImage(UINT8 *dstBuf, UINT16 dstWidth, UINT16 dstHeight, const UINT8 *srcBuf, UINT16 srcWidth, UINT16 srcHeight, bool yInterpolation)
{
	const FLOAT ratioX = FLOAT(dstWidth) / FLOAT(srcWidth);// * 10.0f;
	const FLOAT ratioY = FLOAT(dstHeight) / FLOAT(srcHeight);
	if(ratioX == 1.0f && ratioY == 1.0f) {
		// no scaling needed
		memcpy(dstBuf, srcBuf, sizeof(PixelType) * dstWidth * dstHeight);
		return;
	}

	// TODO: didn't test downscaling yet
	Assert(ratioX >= 1.0f && ratioY >= 1.0f);

	PixelType *dst = reinterpret_cast<PixelType *>(dstBuf);
	const PixelType *src = reinterpret_cast<const PixelType *>(srcBuf);

	// TODO: we could skip clearing whole buffer and just clear pixels that we wont overwrite
	memset(dst, 0, sizeof(PixelType) * dstWidth * dstHeight);

	FLOAT ySrcRatio = ratioY;
	FLOAT yDstRatio = 1.0;
	UINT32 yDst = 0;
	UINT32 ySrc = 0;
	while(yDst < dstHeight && ySrc < srcHeight) {
		if(ySrcRatio <= 0.001f) {
			ySrcRatio = ratioY;
			ySrc++;
			src += srcWidth;
			continue;
		}

		FLOAT ratioLine = yInterpolation ? (yDstRatio < 1.0f ? yDstRatio : ySrcRatio) : 1.0f;
		if(ratioLine > 1.0f)
			ratioLine = 1.0f;

		FLOAT xSrcRatio = ratioX;
		FLOAT xDstRatio = 1.0;
		UINT32 xDst = 0;
		UINT32 xSrc = 0;

		while(xDst < dstWidth && xSrc < srcWidth) {
			if(xDstRatio < 1.0f) {
				pixelInterpolate<PixelType>(&dst[xDst], &src[xSrc], ratioLine * xDstRatio, false); // * MIN(1.0f, xSrcRatio) when upscaling this will always be 1.0f
				xSrcRatio -= xDstRatio;
				xDstRatio = 1.0; // ratioX when downscaling
				xDst++;
			} else if(xSrcRatio >= 1.0f) {
				if(ratioLine >= 1.0f)
					memcpy(&dst[xDst], &src[xSrc], sizeof(PixelType));
				else
					pixelInterpolate<PixelType>(&dst[xDst], &src[xSrc], ratioLine * xDstRatio, false);
				xDst++;
				xSrcRatio -= 1.0f;
			} else if(xSrcRatio > 0.001f) { // 0 < xSrcRatio < 1
				pixelInterpolate<PixelType>(&dst[xDst], &src[xSrc], ratioLine * xSrcRatio, ratioLine >= 1.0f && xDstRatio >= 1.0f);
				xDstRatio = 1.0f - xSrcRatio;
				xSrcRatio = ratioX;
				xSrc++;
			} else { // xSrcRatio <= 0
				xSrcRatio = ratioX;
				xSrc++;
			}
		}

		if(yDstRatio < 1.0f) {
			ratioLine = yInterpolation ? yDstRatio : 1.0f;
			ySrcRatio -= yDstRatio;
			yDstRatio = 1.0; // ratioY when downscaling
			yDst++;
			dst += dstWidth;
		} else if(ySrcRatio >= 1.0f) {
			ratioLine = yInterpolation ? ySrcRatio : 1.0f;
			yDst++;
			dst += dstWidth;
			ySrcRatio -= 1.0f;
		} else if(ySrcRatio > 0.001f) { // 0 < xSrcRatio < 1
			ratioLine = yInterpolation ? ySrcRatio : 1.0f;
			yDstRatio = 1.0f - ySrcRatio;
			ySrcRatio = ratioY;
			ySrc++;
			src += srcWidth;
		} else { // ySrcRatio <= 0
			Assert(FALSE); // we handled this above
		}
	}
}

template <class PixelType>
inline void pixelInterpolate(PixelType *dst, const PixelType *src, FLOAT fract, bool set)
{
	if(set) {
		*dst = FLOAT(*src) * fract;
	} else {
		*dst += FLOAT(*src) * fract;
	}
}

template <>
inline void pixelInterpolate<UINT32>(UINT32 *dstPtr, const UINT32 *srcPtr, FLOAT fract, bool set)
{
	UINT8 *dst = reinterpret_cast<UINT8 *>(dstPtr);
	const UINT8 *src = reinterpret_cast<const UINT8 *>(srcPtr);
	if(set) {
		const FLOAT alpha = FLOAT(*src++) * fract * 1.1;
		*dst++ = alpha > 225. ? 255 : alpha;
		*dst++ = FLOAT(*src++) * fract;
		*dst++ = FLOAT(*src++) * fract;
		*dst = FLOAT(*src) * fract;
	} else {
		const FLOAT alpha = (FLOAT(*dst) + FLOAT(*src++) * fract) * 1.1;
		*dst++ = alpha > 225. ? 255 : alpha;
		*dst++ += FLOAT(*src++) * fract;
		*dst++ += FLOAT(*src++) * fract;
		*dst += FLOAT(*src) * fract;
	}
}

template <class PixelType>
PixelType pixelFromPalette(const SGPPaletteEntry *pal, const UINT8)
{
	return UINT32(pal->r) << 24 | UINT32(pal->g) << 16 | UINT32(pal->b) << 8 | UINT32(pal->a);

}

template <>
UINT8 pixelFromPalette<UINT8>(const SGPPaletteEntry *pal, const UINT8)
{
//	return (UINT32(pal->r) + UINT32(pal->g) + UINT32(pal->b)) / 3 * UINT32(pal->a) / 255;
	return pal->a;
}

