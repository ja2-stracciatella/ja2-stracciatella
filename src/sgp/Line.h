#ifndef ___LINE___H
#define ___LINE___H

#include "Types.h"


/* Example Usage:
 * // don't send pitch, send width in pixels
 * SetClippingRegionAndImageWidth(uiPitch, 15, 15, 30, 30);
 *
 * LineDraw( true, 10, 10, 200, 200, colour, pImageData);
 *    OR
 * RectangleDraw( true, 10, 10, 200, 200, colour, pImageData);
 */


void SetClippingRegionAndImageWidth(int iImageWidth, int iClipStartX, int iClipStartY, int iClipWidth, int iClipHeight);

// NOTE:
//	Don't send fClip==true to LineDraw if you don't have to. So if you know
//  that your line will be within the region you want it to be in, set
//	fClip == false.
void PixelDraw(BOOLEAN fClip, INT32 xp, INT32 yp, INT16 sColor, UINT16* pScreen);
void LineDraw(BOOLEAN fClip, int XStart, int YStart, int XEnd, int YEnd, short Color, UINT16* ScreenPtr);
void RectangleDraw(BOOLEAN fClip, int XStart, int YStart, int XEnd, int YEnd, short Color, UINT16* ScreenPtr);

#endif
