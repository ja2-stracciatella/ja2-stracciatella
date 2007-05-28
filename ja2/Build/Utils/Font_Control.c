#include "Font.h"
#include "SGP.h"
#include "HImage.h"
#include "VSurface.h"
#include "WCheck.h"
#include "Font_Control.h"
#include "MemMan.h"


INT32 gp10PointArial;
INT32 gp10PointArialBold;
INT32 gp10PointRoman;
INT32 gp12PointArial;
INT32 gp12PointArialFixedFont;
INT32 gp12PointFont1;
INT32 gp12PointRoman;
INT32 gp14PointArial;
INT32 gp14PointHumanist;
INT32 gp14PointSansSerif;
INT32 gp16PointArial;
INT32 gpBlockFontNarrow;
INT32 gpBlockyFont;
INT32 gpBlockyFont2;
INT32 gpClockFont;
INT32 gpCompFont;
INT32 gpLargeFontType1;
INT32 gpSmallCompFont;
INT32 gpSmallFontType1;
INT32 gpTinyFontType1;

#if defined JA2EDITOR && defined ENGLISH
INT32 gpHugeFont;
#endif


static UINT16 CreateFontPaletteTables(INT32 Font);


#define M(var, file) \
	(var) = LoadFontFile((file)); \
	CHECKF(CreateFontPaletteTables((var)));

BOOLEAN	InitializeFonts(void)
{
	M(gp10PointArial,          "FONTS/FONT10ARIAL.sti")
	M(gp10PointArialBold,      "FONTS/FONT10ARIALBOLD.sti")
	M(gp10PointRoman,          "FONTS/FONT10ROMAN.sti")
	M(gp12PointArial,          "FONTS/FONT12ARIAL.sti")
	M(gp12PointArialFixedFont, "FONTS/FONT12ARIALFIXEDWIDTH.sti")
	M(gp12PointFont1,          "FONTS/FONT12POINT1.sti")
	M(gp12PointRoman,          "FONTS/FONT12ROMAN.sti")
	M(gp14PointArial,          "FONTS/FONT14ARIAL.sti")
	M(gp14PointHumanist,       "FONTS/FONT14HUMANIST.sti")
	M(gp14PointSansSerif,      "FONTS/FONT14SANSERIF.sti")
	M(gp16PointArial,          "FONTS/FONT16ARIAL.sti")
	M(gpBlockFontNarrow,       "FONTS/BLOCKFONTNARROW.sti")
	M(gpBlockyFont,            "FONTS/BLOCKFONT.sti")
	M(gpBlockyFont2,           "FONTS/BLOCKFONT2.sti")
	M(gpClockFont,             "FONTS/CLOCKFONT.sti")
	M(gpCompFont,              "FONTS/COMPFONT.sti")
	M(gpLargeFontType1,        "FONTS/LARGEFONT1.sti")
	M(gpSmallCompFont,         "FONTS/SMALLCOMPFONT.sti")
	M(gpSmallFontType1,        "FONTS/SMALLFONT1.sti")
	M(gpTinyFontType1,         "FONTS/TINYFONT1.sti")

#if defined JA2EDITOR && defined ENGLISH
	M(gpHugeFont, "FONTS/HUGEFONT.sti")
#endif

	// Set default for font system
	SetFontDestBuffer(FRAME_BUFFER, 0, 0, 640, 480, FALSE);

	return TRUE;
}

#undef M


void ShutdownFonts(void)
{
	UnloadFont(gp10PointArial);
	UnloadFont(gp10PointArialBold);
	UnloadFont(gp10PointRoman);
	UnloadFont(gp12PointArialFixedFont);
	UnloadFont(gp12PointFont1);
	UnloadFont(gp12PointRoman);
	UnloadFont(gp14PointArial);
	UnloadFont(gp14PointSansSerif);
	UnloadFont(gpBlockyFont);
	UnloadFont(gpClockFont);
	UnloadFont(gpCompFont);
	UnloadFont(gpLargeFontType1);
	UnloadFont(gpSmallCompFont);
	UnloadFont(gpSmallFontType1);
	UnloadFont(gpTinyFontType1);

#if defined JA2EDITOR && defined ENGLISH
	UnloadFont(gpHugeFont);
#endif
}


// Set shades for fonts
BOOLEAN SetFontShade(UINT32 uiFontID, INT8 bColorID)
{
	CHECKF(bColorID >= 0);
	CHECKF(bColorID < 16);

	HVOBJECT pFont = GetFontObject(uiFontID);
	pFont->pShadeCurrent = pFont->pShades[bColorID];
	return TRUE;
}


static UINT16 CreateFontPaletteTables(INT32 Font)
{
	HVOBJECT pObj = GetFontObject(Font);

	for (UINT32 count = 0; count < 16; count++)
	{
		if (count == 4 && pObj->p16BPPPalette == pObj->pShades[count])
		{
			pObj->pShades[count] = NULL;
		}
		else if (pObj->pShades[count] != NULL)
		{
			MemFree(pObj->pShades[count]);
			pObj->pShades[count] = NULL;
		}
	}

	pObj->pShades[FONT_SHADE_RED]     = Create16BPPPaletteShaded(pObj->pPaletteEntry, 255,   0,   0, TRUE);
	pObj->pShades[FONT_SHADE_BLUE]    = Create16BPPPaletteShaded(pObj->pPaletteEntry,   0,   0, 255, TRUE);
	pObj->pShades[FONT_SHADE_GREEN]   = Create16BPPPaletteShaded(pObj->pPaletteEntry,   0, 255,   0, TRUE);
	pObj->pShades[FONT_SHADE_YELLOW]  = Create16BPPPaletteShaded(pObj->pPaletteEntry, 255, 255,   0, TRUE);
	pObj->pShades[FONT_SHADE_NEUTRAL] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 255, 255, 255, FALSE);
	pObj->pShades[FONT_SHADE_WHITE]   = Create16BPPPaletteShaded(pObj->pPaletteEntry, 255, 255, 255, TRUE);

	// the rest are darkening tables, right down to all-black.
	pObj->pShades[ 0] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 165, 165, 165, FALSE);
	pObj->pShades[ 7] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 135, 135, 135, FALSE);
	pObj->pShades[ 8] = Create16BPPPaletteShaded(pObj->pPaletteEntry, 105, 105, 105, FALSE);
	pObj->pShades[ 9] = Create16BPPPaletteShaded(pObj->pPaletteEntry,  75,  75,  75, FALSE);
	pObj->pShades[10] = Create16BPPPaletteShaded(pObj->pPaletteEntry,  45,  45,  45, FALSE);
	pObj->pShades[11] = Create16BPPPaletteShaded(pObj->pPaletteEntry,  36,  36,  36, FALSE);
	pObj->pShades[12] = Create16BPPPaletteShaded(pObj->pPaletteEntry,  27,  27,  27, FALSE);
	pObj->pShades[13] = Create16BPPPaletteShaded(pObj->pPaletteEntry,  18,  18,  18, FALSE);
	pObj->pShades[14] = Create16BPPPaletteShaded(pObj->pPaletteEntry,   9,   9,   9, FALSE);
	pObj->pShades[15] = Create16BPPPaletteShaded(pObj->pPaletteEntry,   0,   0,   0, FALSE);

	// Set current shade table to neutral color
	pObj->pShadeCurrent = pObj->pShades[FONT_SHADE_NEUTRAL];

	return TRUE;
}
