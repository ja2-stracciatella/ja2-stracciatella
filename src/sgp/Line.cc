#include "Line.h"


static int giImageWidth = 0;
static int giClipXMin   = 0;
static int giClipXMax   = 0;
static int giClipYMin   = 0;
static int giClipYMax   = 0;


void SetClippingRegionAndImageWidth(int iImageWidth, int iClipStartX, int iClipStartY, int iClipWidth, int iClipHeight)
{
	giImageWidth = iImageWidth;
	giClipXMin = iClipStartX;
	giClipXMax = iClipStartX + iClipWidth - 1;
	giClipYMin = iClipStartY;
	giClipYMax = iClipStartY + iClipHeight - 1;
}


static BOOLEAN Clipt(FLOAT denom, FLOAT num, FLOAT* tE, FLOAT* tL)
{
	if (denom > 0.0f)
	{
		FLOAT t = num / denom;
		if (t > *tL) return FALSE;
		if (t > *tE) *tE = t;
	}
	else if (denom < 0.0f)
	{
		FLOAT t = num / denom;
		if (t < *tE) return FALSE;
		if (t < *tL) *tL = t;
	}
	else if (num > 0)
	{
		return FALSE;
	}

	return TRUE;
}


static BOOLEAN ClipPoint(int x, int y)
{
	return
		giClipXMin <= x && x <= giClipXMax &&
		giClipYMin <= y && y <= giClipYMax;
}


static BOOLEAN Clip2D(int* ix0, int* iy0, int* ix1, int* iy1)
{
	FLOAT x0 = *ix0;
	FLOAT x1 = *ix1;
	FLOAT y0 = *iy0;
	FLOAT y1 = *iy1;

	FLOAT dx = x1 - x0;
	FLOAT dy = y1 - y0;

	BOOLEAN visible = FALSE;
	if (dx == 0.0 && dy == 0.0 && ClipPoint(*ix0, *iy0))
		visible = TRUE;
	else
	{
		FLOAT te = 0.0f;
		FLOAT tl = 1.0f;
		if (Clipt( dx, (FLOAT)giClipXMin - x0, &te, &tl) &&
				Clipt(-dx, x0 - (FLOAT)giClipXMax, &te, &tl) &&
				Clipt( dy, (FLOAT)giClipYMin - y0, &te, &tl) &&
				Clipt(-dy, y0 - (FLOAT)giClipYMax, &te, &tl))
		{
			visible = TRUE;
			if (tl < 1.0f)
			{
				x1 = x0 + tl * dx;
				y1 = y0 + tl * dy;
			}
			if (te > 0)
			{
				x0 = x0 + te * dx;
				y0 = y0 + te * dy;
			}
		}
	}

	*ix0 = x0;
	*ix1 = x1;
	*iy0 = y0;
	*iy1 = y1;

	return visible;
}


static void DrawHorizontalRun(UINT32** ScreenPtr, int XAdvance, int RunLength, int Color);
static void DrawVerticalRun(UINT32** ScreenPtr, int XAdvance, int RunLength, int Color);


/* Draws a line between the specified endpoints in color Color. */
void LineDraw(BOOLEAN const fClip, int XStart, int YStart, int XEnd, int YEnd, UINT32 const Color, UINT32* ScreenPtr)
{
	if (fClip && !Clip2D(&XStart, &YStart, &XEnd, &YEnd)) return;

	/* We'll always draw top to bottom, to reduce the number of cases we have to
	 * handle, and to make lines between the same endpoints draw the same pixels
	 */
	if (YStart > YEnd)
	{
		int Temp;
		Temp = YStart;
		YStart = YEnd;
		YEnd = Temp;
		Temp = XStart;
		XStart = XEnd;
		XEnd = Temp;
	}

	int const pitch = giImageWidth >> 2;

	// point to the bitmap address first pixel to draw
	ScreenPtr += YStart * pitch + XStart;

	/*	Figure out whether we're going left or right, and how far we're
	going horizontally */
	int XAdvance;
	int XDelta = XEnd - XStart;
	if (XDelta < 0)
	{
		XAdvance = -1;
		XDelta = -XDelta;
	}
	else
	{
		XAdvance = 1;
	}
	/* Figure out how far we're going vertically */
	int YDelta = YEnd - YStart;

	/* Special-case horizontal, vertical, and diagonal lines, for speed
	and to avoid nasty boundary conditions and division by 0 */
	if (XDelta == 0)
	{
		/* Vertical line */
		for (int i = 0; i <= YDelta; i++)
		{
			*ScreenPtr = Color;
			ScreenPtr += pitch;
		}
		return;
	}
	if (YDelta == 0)
	{
		/* Horizontal line */
		for (int i = 0; i <= XDelta; i++)
		{
			*ScreenPtr = Color;
			ScreenPtr += XAdvance;
		}
		return;
	}
	if (XDelta == YDelta)
	{
		/* Diagonal line */
		for (int i = 0; i <= XDelta; i++)
		{
			*ScreenPtr = Color;
			ScreenPtr += XAdvance + pitch;
		}
		return;
	}

	/* Determine whether the line is X or Y major, and handle accordingly */
	if (XDelta >= YDelta)
	{
		/* X major line */
		/* Minimum # of pixels in a run in this line */
		int WholeStep = XDelta / YDelta;

		/* Error term adjust each time Y steps by 1; used to tell when one extra
		 * pixel should be drawn as part of a run, to account for fractional steps
		 * along the X axis per 1-pixel steps along Y */
		int AdjUp = (XDelta % YDelta) * 2;

		/* Error term adjust when the error term turns over, used to factor out the
		 * X step made at that time */
		int AdjDown = YDelta * 2;

		/* Initial error term; reflects an initial step of 0.5 along the Y axis */
		int ErrorTerm = (XDelta % YDelta) - (YDelta * 2);

		/* The initial and last runs are partial, because Y advances only 0.5 for
		 * these runs, rather than 1. Divide one full run, plus the initial pixel,
		 * between the initial and last runs */
		int InitialPixelCount = (WholeStep / 2) + 1;
		int FinalPixelCount = InitialPixelCount;

		/* If the basic run length is even and there's no fractional advance, we
		 * have one pixel that could go to either the initial or last partial run,
		 * which we'll arbitrarily allocate to the last run */
		if (AdjUp == 0 && (WholeStep & 0x01) == 0)
		{
			InitialPixelCount--;
		}
		/* If there're an odd number of pixels per run, we have 1 pixel that can't
		 * be allocated to either the initial or last partial run, so we'll add 0.5
		 * to error term so this pixel will be handled by the normal full-run loop
		 */
		if ((WholeStep & 0x01) != 0)
		{
			ErrorTerm += YDelta;
		}
		/* Draw the first, partial run of pixels */
		DrawHorizontalRun(&ScreenPtr, XAdvance, InitialPixelCount, Color);
		/* Draw all full runs */
		for (int i = 0; i < YDelta - 1; i++)
		{
			int RunLength = WholeStep;  /* run is at least this long */
			/* Advance the error term and add an extra pixel if the error
			term so indicates */
			if ((ErrorTerm += AdjUp) > 0)
			{
				RunLength++;
				ErrorTerm -= AdjDown;   /* reset the error term */
			}
			/* Draw this scan line's run */
			DrawHorizontalRun(&ScreenPtr, XAdvance, RunLength, Color);
		}
		/* Draw the final run of pixels */
		DrawHorizontalRun(&ScreenPtr, XAdvance, FinalPixelCount, Color);
	}
	else
	{
		/* Y major line */

		/* Minimum # of pixels in a run in this line */
		int WholeStep = YDelta / XDelta;

		/* Error term adjust each time X steps by 1; used to tell when 1 extra
		 * pixel should be drawn as part of a run, to account for fractional steps
		 * along the Y axis per 1-pixel steps along X */
		int AdjUp = (YDelta % XDelta) * 2;

		/* Error term adjust when the error term turns over, used to factor out the
		 * Y step made at that time */
		int AdjDown = XDelta * 2;

		/* Initial error term; reflects initial step of 0.5 along the X axis */
		int ErrorTerm = (YDelta % XDelta) - (XDelta * 2);

		/* The initial and last runs are partial, because X advances only 0.5 for
		 * these runs, rather than 1. Divide one full run, plus the initial pixel,
		 * between the initial and last runs */
		int InitialPixelCount = (WholeStep / 2) + 1;
		int FinalPixelCount = InitialPixelCount;

		/* If the basic run length is even and there's no fractional advance, we
		 * have 1 pixel that could go to either the initial or last partial run,
		 * which we'll arbitrarily allocate to the last run */
		if (AdjUp == 0 && (WholeStep & 0x01) == 0)
		{
			InitialPixelCount--;
		}
		/* If there are an odd number of pixels per run, we have one pixel that
		 * can't be allocated to either the initial or last partial run, so we'll
		 * add 0.5 to the error term so this pixel will be handled by the normal
		 * full-run loop */
		if ((WholeStep & 0x01) != 0)
		{
			ErrorTerm += XDelta;
		}
		/* Draw the first, partial run of pixels */
		DrawVerticalRun(&ScreenPtr, XAdvance, InitialPixelCount, Color);

		/* Draw all full runs */
		for (int i = 0; i < XDelta - 1; i++)
		{
			int RunLength = WholeStep;  /* run is at least this long */
			/* Advance the error term and add an extra pixel if the error
			term so indicates */
			if ((ErrorTerm += AdjUp) > 0)
			{
				RunLength++;
				ErrorTerm -= AdjDown;   /* reset the error term */
			}
			/* Draw this scan line's run */
			DrawVerticalRun(&ScreenPtr, XAdvance, RunLength, Color);
		}
		/* Draw the final run of pixels */
		DrawVerticalRun(&ScreenPtr, XAdvance, FinalPixelCount, Color);
	}
}


// Draws a pixel in the specified color
void PixelDraw(const BOOLEAN fClip, const INT32 xp, const INT32 yp, const UINT32 sColor, UINT32* const pScreen)
{
	if (fClip && !ClipPoint(xp, yp)) return;

	pScreen[yp * (giImageWidth >> 1) + xp] = sColor;
}


/* Draws a horizontal run of pixels, then advances the bitmap pointer to the
 * first pixel of the next run. */
static void DrawHorizontalRun(UINT32** const ScreenPtr, int const XAdvance, int const RunLength, int const Color)
{
	UINT32* WorkingScreenPtr = *ScreenPtr;

	for (int i = 0; i < RunLength; i++)
	{
		*WorkingScreenPtr = Color;
		WorkingScreenPtr += XAdvance;
	}
	/* Advance to the next scan line */
	WorkingScreenPtr += giImageWidth >> 2;
	*ScreenPtr = WorkingScreenPtr;
}


/* Draws a vertical run of pixels, then advances the bitmap pointer to the
 * first pixel of the next run. */
static void DrawVerticalRun(UINT32** const ScreenPtr, int const XAdvance, int const RunLength, int const Color)
{
	UINT32* WorkingScreenPtr = *ScreenPtr;

	int const pitch = giImageWidth >> 2;
	for (int i = 0; i < RunLength; i++)
	{
		*WorkingScreenPtr = Color;
		WorkingScreenPtr += pitch;
	}
	/* Advance to the next column */
	WorkingScreenPtr += XAdvance;
	*ScreenPtr = WorkingScreenPtr;
}


/* Draws a rectangle between the specified endpoints in color Color. */
void RectangleDraw(const BOOLEAN fClip, const int XStart, const int YStart, const int XEnd, const int YEnd, const UINT32 Color, UINT32 *ScreenPtr)
{
	LineDraw(fClip, XStart, YStart, XEnd,   YStart, Color, ScreenPtr);
	LineDraw(fClip, XStart, YEnd,   XEnd,   YEnd,   Color, ScreenPtr);
	LineDraw(fClip, XStart, YStart, XStart, YEnd,   Color, ScreenPtr);
	LineDraw(fClip, XEnd,   YStart, XEnd,   YEnd,   Color, ScreenPtr);
}
