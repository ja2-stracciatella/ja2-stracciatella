#ifndef __VOBJECT_BLITTERS
#define __VOBJECT_BLITTERS

#include "shading.h"
#include "vsurface.h"
#include "vobject.h"

#ifdef __cplusplus
extern "C" {
#endif

extern SGPRect		ClippingRect;
extern UINT32			guiTranslucentMask;
extern UINT16			White16BPPPalette[ 256 ];

extern void SetClippingRect(SGPRect *clip);
void GetClippingRect(SGPRect *clip);


BOOLEAN BltIsClipped(HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
CHAR8 BltIsClippedOrOffScreen( HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion );


UINT16 *InitZBuffer(UINT32 uiPitch, UINT32 uiHeight);
BOOLEAN ShutdownZBuffer(UINT16 *pBuffer);

// 8-Bit to 8-Bit Blitters

//BOOLEAN Blt8BPPDataTo8BPPBufferTransZIncClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);

BOOLEAN Blt8BPPDataTo8BPPBufferTransZNBColor( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT8 ubColor);
BOOLEAN Blt8BPPDataTo8BPPBufferTransZNBClipColor( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT8 ubColor);

// pixelation blitters
BOOLEAN Blt8BPPDataTo8BPPBufferTransZPixelate( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo8BPPBufferTransZClipPixelate( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo8BPPBufferTransZNBPixelate( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo8BPPBufferTransZNBClipPixelate( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);


BOOLEAN Blt8BPPDataTo8BPPBufferMonoShadowClip( UINT8 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT8 ubForeground, UINT8 ubBackground);

BOOLEAN Blt8BPPDataTo8BPPBufferTransparent( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo8BPPBufferTransparentClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransMirror( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );

BOOLEAN Blt8BPPDataTo8BPPBufferTransZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo8BPPBufferTransZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );

BOOLEAN Blt8BPPDataTo8BPPBufferTransZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo8BPPBufferTransZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);

BOOLEAN Blt8BPPDataTo8BPPBufferShadowZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo8BPPBufferShadowZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );

BOOLEAN Blt8BPPDataTo8BPPBufferShadowZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo8BPPBufferShadowZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);

BOOLEAN Blt8BPPDataTo8BPPBufferTransShadowZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT16 *p16BPPPalette );
BOOLEAN Blt8BPPDataTo8BPPBufferTransShadowZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT16 *p16BPPPalette );

BOOLEAN Blt8BPPDataTo8BPPBufferTransShadowZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 *p16BPPPalette );
BOOLEAN Blt8BPPDataTo8BPPBufferTransShadowZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 *p16BPPPalette );

BOOLEAN Blt8BPPDataTo8BPPBufferShadowClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo8BPPBufferShadow( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo8BPPBuffer( UINT8 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );


// 8-Bit to 16-Bit Blitters

BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBColor( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT16 usColor);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBClipColor( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 usColor);

// pixelation blitters
BOOLEAN Blt8BPPDataTo16BPPBufferTransZPixelate( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo16BPPBufferTransZClipPixelate( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBPixelate( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBClipPixelate( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);


// translucency blitters
BOOLEAN Blt8BPPDataTo16BPPBufferTransZTranslucent( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo16BPPBufferTransZClipTranslucent( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBTranslucent( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBClipTranslucent( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);

BOOLEAN Blt8BPPDataTo16BPPBufferMonoShadowClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 usForeground, UINT16 usBackground, UINT16 usShadow );

BOOLEAN Blt8BPPDataTo16BPPBufferTransZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo16BPPBufferTransZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT16 *p16BPPPalette );
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 *p16BPPPalette );
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT16 *p16BPPPalette );
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 *p16BPPPalette);
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 *p16BPPPalette);

// Next blitters are for blitting mask as intensity
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferIntensity( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );


BOOLEAN Blt8BPPDataTo16BPPBufferTransparentClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransparent( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );

BOOLEAN Blt8BPPDataTo16BPPBufferTransShadow( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT16 *p16BPPPalette );

BOOLEAN Blt8BPPDataTo16BPPBufferShadowClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN DDBlt8BPPDataTo16BPPBufferShadow( HVOBJECT hDestVObject, HVOBJECT hSrcVObject, UINT8 level, COLORVAL maskrgb, UINT16 usX, UINT16 usY, SGPRect *srcRect);

BOOLEAN Blt8BPPTo8BPP(UINT8 *pDest, UINT32 uiDestPitch, UINT8 *pSrc, UINT32 uiSrcPitch, INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos, UINT32 uiWidth, UINT32 uiHeight);
BOOLEAN Blt16BPPTo16BPP(UINT16 *pDest, UINT32 uiDestPitch, UINT16 *pSrc, UINT32 uiSrcPitch, INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos, UINT32 uiWidth, UINT32 uiHeight);
BOOLEAN Blt16BPPTo16BPPTrans(UINT16 *pDest, UINT32 uiDestPitch, UINT16 *pSrc, UINT32 uiSrcPitch, INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos, UINT32 uiWidth, UINT32 uiHeight, UINT16 usTrans);
BOOLEAN Blt16BPPTo16BPPFog(UINT16 *pDest, UINT32 uiDestPitch, UINT16 *pSrc, UINT32 uiSrcPitch, INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos, UINT32 uiWidth, UINT32 uiHeight, UINT8 *pFog, UINT16 usFogPitch);
BOOLEAN Blt16BPPTo16BPPMirror(UINT16 *pDest, UINT32 uiDestPitch, UINT16 *pSrc, UINT32 uiSrcPitch, INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos, UINT32 uiWidth, UINT32 uiHeight);

BOOLEAN Blt8BPPDataTo16BPPBufferFogZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo16BPPBufferFogZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferFogZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo16BPPBufferFogZNBClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);

BOOLEAN Blt16BPPBufferPixelateRectWithColor( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area, UINT8 Pattern[8][8], UINT16 usColor );
BOOLEAN Blt16BPPBufferPixelateRect(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area, UINT8 Pattern[8][8]);

//A wrapper for the Blt16BPPBufferPixelateRect that automatically passes a hatch pattern.
BOOLEAN Blt16BPPBufferHatchRectWithColor(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area, UINT16 usColor );
BOOLEAN Blt16BPPBufferHatchRect(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area );
BOOLEAN Blt16BPPBufferLooseHatchRectWithColor(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area, UINT16 usColor );
BOOLEAN Blt16BPPBufferLooseHatchRect(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area );

BOOLEAN Blt16BPPBufferShadowRect(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area);

BOOLEAN Blt8BPPDataTo16BPPBufferShadow( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );

BOOLEAN Blt8BPPDataTo16BPPBuffer( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVSURFACE hSrcVSurface, UINT8 *pSrcBuffer, INT32 iX, INT32 iY);
BOOLEAN Blt8BPPDataSubTo16BPPBuffer( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVSURFACE hSrcVSurface, UINT8 *pSrcBuffer, UINT32 uiSrcPitch, INT32 iX, INT32 iY, SGPRect *pRect);

// Blits from flat 8bpp source, to 16bpp dest, divides in half
BOOLEAN Blt8BPPDataTo16BPPBufferHalf( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVSURFACE hSrcVSurface, UINT8 *pSrcBuffer, UINT32 uiSrcPitch, INT32 iX, INT32 iY);
BOOLEAN Blt8BPPDataTo16BPPBufferHalfRect( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVSURFACE hSrcVSurface, UINT8 *pSrcBuffer, UINT32 uiSrcPitch, INT32 iX, INT32 iY, SGPRect *pRect);

BOOLEAN DDBlt8BPPDataTo16BPPBuffer( HVOBJECT hDestVObject, HVOBJECT hSrcVObject, UINT16 usX, UINT16 usY, SGPRect *srcRect );
BOOLEAN DDBlt8BPPDataTo16BPPBufferFullTransparent( HVOBJECT hDestVObject, HVOBJECT hSrcVObject, UINT16 usX, UINT16 usY, SGPRect *srcRect );
BOOLEAN DDFillSurface( HVOBJECT hDestVObject, blt_fx *pBltFx );
BOOLEAN DDFillSurfaceRect( HVOBJECT hDestVObject, blt_fx *pBltFx );
BOOLEAN BltVObjectUsingDD( HVOBJECT hDestVObject, HVOBJECT hSrcVObject, UINT32 fBltFlags, INT32 iDestX, INT32 iDestY, RECT *SrcRect );


BOOLEAN BlitZRect(UINT16 *pZBuffer, UINT32 uiPitch, INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom, UINT16 usZValue);

// New 16/16 blitters

BOOLEAN Blt16BPPDataTo16BPPBufferTransparentClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt16BPPDataTo16BPPBufferTransZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt16BPPDataTo16BPPBufferTransparent( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );

// ATE: New blitters for showing an outline at color 254
BOOLEAN Blt8BPPDataTo16BPPBufferOutline( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline );
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline, SGPRect *clipregion );
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZ( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline );
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineShadow( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineShadowClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion );
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZNB( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline );
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZPixelateObscured( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline );
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZPixelateObscuredClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline, SGPRect *clipregion );
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline, SGPRect *clipregion );


// ATE: New blitter for included shadow, but pixellate if obscured by z
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNBObscured( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, UINT16 *p16BPPPalette );

BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNBObscuredClip( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion, UINT16 *p16BPPPalette );
BOOLEAN Blt8BPPDataTo16BPPBufferTransZClipPixelateObscured( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZPixelateObscured( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );

BOOLEAN FillRect8BPP(UINT8 *pBuffer, UINT32 uiDestPitchBYTES, INT32 x1, INT32 y1, INT32 x2, INT32 y2, UINT8 color);
BOOLEAN FillRect16BPP(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, INT32 x1, INT32 y1, INT32 x2, INT32 y2, UINT16 color);


#ifdef __cplusplus
}
#endif

#endif
