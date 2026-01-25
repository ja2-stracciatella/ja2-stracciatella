#ifndef __VOBJECT_BLITTERS
#define __VOBJECT_BLITTERS

#include "Types.h"
#include "VObject.h"

inline SGPRect		ClippingRect;
extern UINT32			guiTranslucentMask;

// Sets the clipping rect and returns the replaced rect
SGPRect SetClippingRect(SGPRect clip);
SGPRect GetClippingRect();

struct ClipInfo
{
	enum Status
	{
		Completely_Clipped,
		Partially_Clipped,
		Not_Clipped
	};

	Status status;
	UINT16 vobjectIndex;
	SGPVObject const *vobject;
	UINT8 const * srcPtr;

	int leftSkip, topSkip, rightSkip, bottomSkip;
	int blitLength, blitHeight;
	int iTempX, iTempY;

	ClipInfo(SGPVObject const * hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect const * clipregion);
};

/* Allocate and initialize a Z-buffer for use with the Z-buffer blitters.
 * Doesn't really do much except allocate a chunk of memory, and zero it. */
UINT16* InitZBuffer(UINT32 width, UINT32 height);

/* Free the memory allocated for the Z-buffer. */
void ShutdownZBuffer(UINT16 *pBuffer);

// translucency blitters
void Blt8BPPDataTo16BPPBufferTransZTranslucent(UINT16* buf, UINT32 uiDestPitchBYTES, UINT16* zbuf, UINT16 zval, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex);
void Blt8BPPDataTo16BPPBufferTransZNBTranslucent(UINT16* buf, UINT32 uiDestPitchBYTES, UINT16* zbuf, UINT16 zval, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex);
void BltTransZNBTranslucent(ClipInfo const& ci, UINT16* buf, UINT32 uiDestPitchBYTES, UINT16* zbuf, UINT16 zval);

void Blt8BPPDataTo16BPPBufferMonoShadowClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect const * clipregion, UINT16 usForeground, UINT16 usBackground, UINT16 usShadow );

void Blt8BPPDataTo16BPPBufferTransZ(UINT16* buf, UINT32 uiDestPitchBYTES, UINT16* zbuf, UINT16 zval, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex);
void Blt8BPPDataTo16BPPBufferTransZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
void BltTransZ(ClipInfo const& ci, UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue);
void BltTransZNB(ClipInfo const& ci, UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue);
void Blt8BPPDataTo16BPPBufferTransShadowZ(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, const UINT16* p16BPPPalette);
void BltTransShadow(ClipInfo const& ci, UINT16* pBuffer, UINT32 uiDestPitchBYTES, const UINT16* p16BPPPalette);
void Blt8BPPDataTo16BPPBufferTransShadowZNB(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, const UINT16* p16BPPPalette);
void Blt8BPPDataTo16BPPBufferShadowZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
void BltShadowZNB(ClipInfo const& ci, UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue);
void Blt8BPPDataTo16BPPBufferShadowZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
void BltShadowZ(ClipInfo const& ci, UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue);
void BltTransShadowZ(ClipInfo const& ci, UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, const UINT16* p16BPPPalette);
void BltTransShadowZNB(ClipInfo const& ci, UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, const UINT16* p16BPPPalette);

void Blt8BPPDataTo16BPPBufferTransparentClip(UINT16* pBuffer, UINT32 uiDestPitchBYTES, const SGPVObject* hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, const SGPRect* clipregion);
void BltTransparent(ClipInfo const& ci, UINT16* pBuffer, UINT32 uiDestPitchBYTES);
void Blt8BPPDataTo16BPPBufferTransparent(UINT16* buf, UINT32 uiDestPitchBYTES, SGPVObject const* hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex);

void Blt8BPPDataTo16BPPBufferTransShadow(UINT16* pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, const UINT16* p16BPPPalette);

void BltShadow(ClipInfo const& ci, UINT16 *pBuffer, UINT32 uiDestPitchBYTES);

void Blt16BPPTo16BPP(UINT16 *pDest, UINT32 uiDestPitch, UINT16 *pSrc, UINT32 uiSrcPitch, INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos, UINT32 uiWidth, UINT32 uiHeight);

void Blt16BPPBufferHatchRect(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area );
void Blt16BPPBufferLooseHatchRectWithColor(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area, UINT16 usColor );

/* Filter a rectangular area with the given filter table.  This is used for
 * shading. */
void Blt16BPPBufferFilterRect(UINT16* pBuffer, UINT32 uiDestPitchBYTES, const UINT16* filter_table, SGPRect* area);

void Blt8BPPDataTo16BPPBufferShadow( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );

void Blt8BPPDataTo16BPPBuffer( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPVSurface* hSrcVSurface, UINT8 *pSrcBuffer, INT32 iX, INT32 iY);
void Blt8BPPDataSubTo16BPPBuffer(UINT16* buf, UINT32 uiDestPitchBYTES, SGPVSurface* hSrcVSurface, UINT8* pSrcBuffer, UINT32 src_pitch, INT32 iX, INT32 iY, SGPBox const* rect);

// Blits from flat 8bpp source, to 16bpp dest, divides in half
void Blt8BPPDataTo16BPPBufferHalf(UINT16* dst_buf, UINT32 uiDestPitchBYTES, SGPVSurface* src_surface, UINT8 const* src_buf, UINT32 src_pitch, INT32 x, INT32 y, SGPBox const* rect);


// ATE: New blitters for showing an outline at color 254
void Blt8BPPDataTo16BPPBufferOutline(    UINT16* buf, UINT32 uiDestPitchBYTES, SGPVObject const* hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 outline);
void BltOutline(ClipInfo const& ci, UINT16* pBuffer, UINT32 uiDestPitchBYTES, INT16 s16BPPColor);
void Blt8BPPDataTo16BPPBufferOutlineZ(                    UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor);
void Blt8BPPDataTo16BPPBufferOutlineShadow(UINT16* pBuffer, UINT32 uiDestPitchBYTES, const SGPVObject* hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex);
void BltOutlineShadow(ClipInfo const& ci, UINT16 * pBuffer, UINT32 uiDestPitchBYTES);
void Blt8BPPDataTo16BPPBufferOutlineZNB(                  UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex);
void Blt8BPPDataTo16BPPBufferOutlineZPixelateObscured(    UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor);
void BltOutlineZPixelateObscured(ClipInfo const& ci, UINT16 *pBuffer, UINT32 uiDestPitchBytes, UINT16* pZBuffer, UINT16 usZValue, INT16 s16BPPColor);
void BltOutlineZ(ClipInfo const& ci, UINT16 *pBuffer, UINT32 uiDestPitchBytes, UINT16* pZBuffer, UINT16 usZValue, INT16 s16BPPColor);

// ATE: New blitter for included shadow, but pixellate if obscured by z
void Blt8BPPDataTo16BPPBufferTransShadowZNBObscured(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, const UINT16* p16BPPPalette);

void BltTransShadowZNBObscured(ClipInfo const& ci, UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, const UINT16* p16BPPPalette);
void BltTransZPixelateObscured(ClipInfo const& ci, UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue);
void Blt8BPPDataTo16BPPBufferTransZPixelateObscured( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );

// SrcPtr must point to the data bytes of a VObject. This function then skips past
// the next TopSkip lines and returns the pointer to the start of the next line.
UINT8 const * SkipLines(UINT8 const * SrcPtr, int TopSkip);

#endif
