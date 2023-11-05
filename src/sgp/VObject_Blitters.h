#ifndef __VOBJECT_BLITTERS
#define __VOBJECT_BLITTERS

#include "Types.h"

#include "Types.h"


extern SGPRect		ClippingRect;
extern UINT32			guiTranslucentMask;

// Sets the clipping rect and returns the replaced rect
SGPRect SetClippingRect(SGPRect clip);
SGPRect GetClippingRect();


BOOLEAN BltIsClipped(const SGPVObject *srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect);
CHAR8 BltIsClippedOrOffScreen(const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect);


/* Allocate and initialize a Z-buffer for use with the Z-buffer blitters.
 * Doesn't really do much except allocate a chunk of memory, and zero it. */
UINT16* InitZBuffer(UINT32 width, UINT32 height);

/* Free the memory allocated for the Z-buffer. */
void ShutdownZBuffer(UINT16 *pBuffer);

// translucency blitters
void Blt32BPPDataTo32BPPBufferTransZTranslucent(UINT32 *buf, UINT32 uiDestPitchBYTES, UINT16 *zbuf, UINT16 zval, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex);
void Blt32BPPDataTo32BPPBufferTransZNBTranslucent(UINT32 *buf, UINT32 uiDestPitchBYTES, const UINT16 *zbuf, UINT16 zval, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex);
void Blt32BPPDataTo32BPPBufferTransZNBTranslucentClip(UINT32 *buf, UINT32 uiDestPitchBYTES, const UINT16 *zbuf, UINT16 zval, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect const* clipregion);

void Blt8BPPDataTo32BPPBufferMonoShadowClip(UINT32 *buf, const UINT32 bufPitch, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect, const UINT32 colFG, const UINT32 colBG, const UINT32 colSH);

void Blt32BPPDataTo32BPPBufferTransZ(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex);
void Blt32BPPDataTo32BPPBufferTransZClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect);
void Blt32BPPDataTo32BPPBufferTransZNB(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex);
void Blt32BPPDataTo32BPPBufferTransZNBClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect);

void Blt32BPPDataTo32BPPBufferShadow(UINT32 *buf, const UINT32 bufPitch, const SGPVObject *srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex);
void Blt32BPPDataTo32BPPBufferShadowClip(UINT32 *buf, const UINT32 bufPitch, const SGPVObject *srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect);
void Blt32BPPDataTo32BPPBufferShadowZ(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex);
void Blt32BPPDataTo32BPPBufferShadowZClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect);
void Blt32BPPDataTo32BPPBufferShadowZNB(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex);
void Blt32BPPDataTo32BPPBufferShadowZNBClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect);

void Blt32BPPDataTo32BPPBufferTransShadowClip(UINT32 *buf, UINT32 bufPitch, HVOBJECT srcObj, INT32 srcX, INT32 srcY, UINT16 srcIndex, SGPRect *clipRect, const UINT16* palette);

void Blt32BPPDataTo32BPPBufferTransShadowZ(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT16 *palette);
void Blt32BPPDataTo32BPPBufferTransShadowZClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, SGPRect *clipRect, const UINT16 *palette);
void Blt32BPPDataTo32BPPBufferTransShadowZNB(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT16 *palette);
void Blt32BPPDataTo32BPPBufferTransShadowZNBClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, SGPRect *clipRect, const UINT16 *palette);

// Next blitters are for blitting mask as intensity
void Blt32BPPDataTo32BPPBufferIntensityZNB(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex);
void Blt32BPPDataTo32BPPBufferIntensityZ(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex);
void Blt32BPPDataTo32BPPBufferIntensityZClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect);
void Blt32BPPDataTo32BPPBufferIntensityClip(UINT32 *buf, const UINT32 bufPitch, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect);
void Blt32BPPDataTo32BPPBufferIntensity(UINT32 *buf, const UINT32 bufPitch, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex);

void Blt32BPPDataTo32BPPBufferTransparent(UINT32 *buf, const UINT32 bufPitch, const SGPVObject *srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex);
void Blt32BPPDataTo32BPPBufferTransparentClip(UINT32 *buf, const UINT32 bufPitch, const SGPVObject *srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect);

void Blt32BPPDataTo32BPPBufferTransShadow(UINT32 *buf, const UINT32 bufPitch, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT16 *palette);

void Blt16BPPTo16BPP(UINT16 *pDest, UINT32 uiDestPitch, UINT16 *pSrc, UINT32 uiSrcPitch, INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos, UINT32 uiWidth, UINT32 uiHeight);
void Blt32BPPTo32BPP(UINT32 *pDest, UINT32 uiDestPitch, UINT32 *pSrc, UINT32 uiSrcPitch, INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos, UINT32 uiWidth, UINT32 uiHeight);

void Blt32BPPBufferHatchRect(UINT32 *buf, UINT32 bufPitch, SGPRect *area);
void Blt32BPPBufferLooseHatchRectWithColor(UINT32 *buf, UINT32 bufPitch, SGPRect *area, UINT32 color);

/* Filter a rectangular area with the given filter table.  This is used for
 * shading. */
void Blt32BPPBufferFilterRect(UINT32 *buf, UINT32 bufPitch, const FLOAT filterPercent, SGPRect *area);

// ATE: New blitters for showing an outline at color 254
void Blt32BPPDataTo32BPPBufferOutline(UINT32 *buf, const UINT32 bufPitch, const SGPVObject *srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT32 colOutline);
void Blt32BPPDataTo32BPPBufferOutlineClip(UINT32 *buf, const UINT32 bufPitch, const SGPVObject *srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT32 colOutline, const SGPRect *clipRect);
void Blt32BPPDataTo32BPPBufferOutlineZ(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT32 colOutline);
void Blt32BPPDataTo32BPPBufferOutlineZClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const SGPVObject *srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT32 colOutline, const SGPRect *clipRect);
void Blt32BPPDataTo32BPPBufferOutlineZNB(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex);

void Blt32BPPDataTo32BPPBufferTransShadowZNBObscured(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT16 *palette);
void Blt32BPPDataTo32BPPBufferTransShadowZNBObscuredClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, SGPRect *clipRect, const UINT16 *palette);

void Blt32BPPDataTo32BPPBufferOutlineZPixelateObscured(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT32 colShadow);
void Blt32BPPDataTo32BPPBufferOutlineZPixelateObscuredClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const UINT32 colShadow, const SGPRect *clipRect);
void Blt32BPPDataTo32BPPBufferTransZPixelateObscured(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex);
void Blt32BPPDataTo32BPPBufferTransZPixelateObscuredClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect);

// multi z-level blits - moved from RenderWorld.cc
void Blt32BPPDataTo32BPPBufferTransZIncClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect);
void Blt32BPPDataTo32BPPBufferTransZIncClipZSameZBurnsThrough(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect);
void Blt32BPPDataTo32BPPBufferTransZIncObscureClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect);
void Blt32BPPDataTo32BPPBufferTransZTransShadowIncObscureClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect, const INT16 zStrip, const UINT16* palette);
void Blt32BPPDataTo32BPPBufferTransZTransShadowIncClip(UINT32 *buf, const UINT32 bufPitch, UINT16 *zBuf, const UINT16 zVal, const HVOBJECT srcObj, const INT32 srcX, const INT32 srcY, const UINT16 srcIndex, const SGPRect *clipRect, const INT16 zStrip, const UINT16* palette);

#endif
