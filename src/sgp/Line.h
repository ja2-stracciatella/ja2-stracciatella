#ifndef ___LINE___H
#define ___LINE___H

#include "Types.h"


/* Example Usage:
 * // don't send pitch, send width in pixels
 * SetClippingRegionAndImageWidth(uiPitch, 15, 15, 30, 30);
 *
 * LineDraw( TRUE, 10, 10, 200, 200, color, pImageData);
 *    OR
 * RectangleDraw( TRUE, 10, 10, 200, 200, color, pImageData);
 */


void SetClippingRegionAndImageWidth(int iImageWidth, int iClipStartX, int iClipStartY, int iClipWidth, int iClipHeight);

// NOTE:
//	Don't send fClip==TRUE to LineDraw if you don't have to. So if you know
//  that your line will be within the region you want it to be in, set
//	fClip == FALSE.
void PixelDraw(BOOLEAN fClip, INT32 xp, INT32 yp, UINT32 sColor, UINT16* pScreen);
void LineDraw(BOOLEAN fClip, int XStart, int YStart, int XEnd, int YEnd, UINT32 Color, UINT16* ScreenPtr);
void RectangleDraw(const BOOLEAN fClip, const int XStart, const int YStart, const int XEnd, const int YEnd, const UINT32 Color, UINT16 *ScreenPtr);

#endif
