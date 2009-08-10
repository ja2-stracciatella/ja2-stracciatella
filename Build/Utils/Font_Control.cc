#include "Directories.h"
#include "Font.h"
#include "HImage.h"
#include "VObject.h"
#include "VSurface.h"
#include "WCheck.h"
#include "Font_Control.h"
#include "MemMan.h"


Font gp10PointArial;
Font gp10PointArialBold;
Font gp12PointArial;
Font gp12PointArialFixedFont;
Font gp12PointFont1;
Font gp14PointArial;
Font gp14PointHumanist;
Font gp16PointArial;
Font gpBlockFontNarrow;
Font gpBlockyFont;
Font gpBlockyFont2;
Font gpCompFont;
Font gpLargeFontType1;
Font gpSmallCompFont;
Font gpSmallFontType1;
Font gpTinyFontType1;

#if defined JA2EDITOR && defined ENGLISH
Font gpHugeFont;
#endif


static void CreateFontPaletteTables(Font);


void InitializeFonts(void)
{
#define M(var, file) (CreateFontPaletteTables((var) = LoadFontFile((file))))
	M(gp10PointArial,          FONTSDIR "/FONT10ARIAL.sti");
	M(gp10PointArialBold,      FONTSDIR "/FONT10ARIALBOLD.sti");
	M(gp12PointArial,          FONTSDIR "/FONT12ARIAL.sti");
	M(gp12PointArialFixedFont, FONTSDIR "/FONT12ARIALFIXEDWIDTH.sti");
	M(gp12PointFont1,          FONTSDIR "/FONT12POINT1.sti");
	M(gp14PointArial,          FONTSDIR "/FONT14ARIAL.sti");
	M(gp14PointHumanist,       FONTSDIR "/FONT14HUMANIST.sti");
	M(gp16PointArial,          FONTSDIR "/FONT16ARIAL.sti");
	M(gpBlockFontNarrow,       FONTSDIR "/BLOCKFONTNARROW.sti");
	M(gpBlockyFont,            FONTSDIR "/BLOCKFONT.sti");
	M(gpBlockyFont2,           FONTSDIR "/BLOCKFONT2.sti");
	M(gpCompFont,              FONTSDIR "/COMPFONT.sti");
	M(gpLargeFontType1,        FONTSDIR "/LARGEFONT1.sti");
	M(gpSmallCompFont,         FONTSDIR "/SMALLCOMPFONT.sti");
	M(gpSmallFontType1,        FONTSDIR "/SMALLFONT1.sti");
	M(gpTinyFontType1,         FONTSDIR "/TINYFONT1.sti");

#if defined JA2EDITOR && defined ENGLISH
	M(gpHugeFont, FONTSDIR "/HUGEFONT.sti");
#endif
#undef M

	// Set default for font system
	SetFontDestBuffer(FRAME_BUFFER);
}


void ShutdownFonts(void)
{
	UnloadFont(gp10PointArial);
	UnloadFont(gp10PointArialBold);
	UnloadFont(gp12PointArial);
	UnloadFont(gp12PointArialFixedFont);
	UnloadFont(gp12PointFont1);
	UnloadFont(gp14PointArial);
	UnloadFont(gp14PointHumanist);
	UnloadFont(gp16PointArial);
	UnloadFont(gpBlockFontNarrow);
	UnloadFont(gpBlockyFont);
	UnloadFont(gpBlockyFont2);
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
void SetFontShade(Font const font, FontShade const shade)
{
	CHECKV(0 <= shade && shade < 16);
	font->CurrentShade(shade);
}


static void CreateFontPaletteTables(Font const f)
{
	const SGPPaletteEntry* const pal = f->Palette();
	f->pShades[FONT_SHADE_RED]     = Create16BPPPaletteShaded(pal, 255,   0,   0, TRUE);
	f->pShades[FONT_SHADE_BLUE]    = Create16BPPPaletteShaded(pal,   0,   0, 255, TRUE);
	f->pShades[FONT_SHADE_GREEN]   = Create16BPPPaletteShaded(pal,   0, 255,   0, TRUE);
	f->pShades[FONT_SHADE_YELLOW]  = Create16BPPPaletteShaded(pal, 255, 255,   0, TRUE);
	f->pShades[FONT_SHADE_NEUTRAL] = Create16BPPPaletteShaded(pal, 255, 255, 255, FALSE);
	f->pShades[FONT_SHADE_WHITE]   = Create16BPPPaletteShaded(pal, 255, 255, 255, TRUE);

	// the rest are darkening tables, right down to all-black.
	f->pShades[ 0] = Create16BPPPaletteShaded(pal, 165, 165, 165, FALSE);
	f->pShades[ 7] = Create16BPPPaletteShaded(pal, 135, 135, 135, FALSE);
	f->pShades[ 8] = Create16BPPPaletteShaded(pal, 105, 105, 105, FALSE);
	f->pShades[ 9] = Create16BPPPaletteShaded(pal,  75,  75,  75, FALSE);
	f->pShades[10] = Create16BPPPaletteShaded(pal,  45,  45,  45, FALSE);
	f->pShades[11] = Create16BPPPaletteShaded(pal,  36,  36,  36, FALSE);
	f->pShades[12] = Create16BPPPaletteShaded(pal,  27,  27,  27, FALSE);
	f->pShades[13] = Create16BPPPaletteShaded(pal,  18,  18,  18, FALSE);
	f->pShades[14] = Create16BPPPaletteShaded(pal,   9,   9,   9, FALSE);
	f->pShades[15] = Create16BPPPaletteShaded(pal,   0,   0,   0, FALSE);

	// Set current shade table to neutral color
	f->CurrentShade(FONT_SHADE_NEUTRAL);
}
