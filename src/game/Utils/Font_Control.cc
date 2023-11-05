#include "Directories.h"
#include "Font.h"
#include "HImage.h"
#include "VObject.h"
#include "VSurface.h"
#include "Font_Control.h"
#include "GameRes.h"
#include "GameMode.h"


SGPFont gp10PointArial;
SGPFont gp10PointArialBold;
SGPFont gp12PointArial;
SGPFont gp12PointArialFixedFont;
SGPFont gp12PointFont1;
SGPFont gp14PointArial;
SGPFont gp14PointHumanist;
SGPFont gp16PointArial;
SGPFont gpBlockFontNarrow;
SGPFont gpBlockyFont;
SGPFont gpBlockyFont2;
SGPFont gpCompFont;
SGPFont gpLargeFontType1;
SGPFont gpSmallCompFont;
SGPFont gpSmallFontType1;
SGPFont gpTinyFontType1;

SGPFont gpHugeFont;


static void CreateFontPaletteTables(SGPFont);


void InitializeFonts(void)
{
#define M(var, file) (CreateFontPaletteTables((var) = LoadFontFile((file))))
	M(gp10PointArial,          FONTSDIR "/font10arial.sti");
	M(gp10PointArialBold,      FONTSDIR "/font10arialbold.sti");
	M(gp12PointArial,          FONTSDIR "/font12arial.sti");
	M(gp12PointArialFixedFont, FONTSDIR "/font12arialfixedwidth.sti");
	M(gp12PointFont1,          FONTSDIR "/font12point1.sti");
	M(gp14PointArial,          FONTSDIR "/font14arial.sti");
	M(gp14PointHumanist,       FONTSDIR "/font14humanist.sti");
	M(gp16PointArial,          FONTSDIR "/font16arial.sti");
	M(gpBlockFontNarrow,       FONTSDIR "/blockfontnarrow.sti");
	M(gpBlockyFont,            FONTSDIR "/blockfont.sti");
	M(gpBlockyFont2,           FONTSDIR "/blockfont2.sti");
	M(gpCompFont,              FONTSDIR "/compfont.sti");
	M(gpLargeFontType1,        FONTSDIR "/largefont1.sti");
	M(gpSmallCompFont,         FONTSDIR "/smallcompfont.sti");
	M(gpSmallFontType1,        FONTSDIR "/smallfont1.sti");
	M(gpTinyFontType1,         FONTSDIR "/tinyfont1.sti");

	if(GameMode::getInstance()->isEditorMode() && isEnglishVersion())
	{
		M(gpHugeFont, FONTSDIR "/hugefont.sti");
	}
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

	if(GameMode::getInstance()->isEditorMode() && isEnglishVersion())
	{
		UnloadFont(gpHugeFont);
	}
}


// Set shades for fonts
void SetFontShade(SGPFont const font, FontShade const shade)
{
	font->CurrentShade(shade);
}


static void CreateFontPaletteTables(SGPFont const f)
{
	f->pShades[FONT_SHADE_RED]     = SHADE_MONO(255, 0, 0);
	f->pShades[FONT_SHADE_BLUE]    = SHADE_MONO(0, 0, 255);
	f->pShades[FONT_SHADE_GREEN]   = SHADE_MONO(0, 255, 0);
	f->pShades[FONT_SHADE_YELLOW]  = SHADE_MONO(255, 255, 0);
	f->pShades[FONT_SHADE_NEUTRAL] = SHADE_STD(255, 255, 255);
	f->pShades[FONT_SHADE_WHITE]   = SHADE_MONO(255, 255, 255);

	// the rest are darkening tables, right down to all-black.
	f->pShades[ 0] = SHADE_STD(165, 165, 165);
	f->pShades[ 7] = SHADE_STD(135, 135, 135);
	f->pShades[ 8] = SHADE_STD(105, 105, 105);
	f->pShades[ 9] = SHADE_STD(75,  75,  75);
	f->pShades[10] = SHADE_STD(45,  45,  45);
	f->pShades[11] = SHADE_STD(36,  36,  36);
	f->pShades[12] = SHADE_STD(27,  27,  27);
	f->pShades[13] = SHADE_STD(18,  18,  18);
	f->pShades[14] = SHADE_STD(9,   9,   9);
	f->pShades[15] = SHADE_STD(0, 0, 0);

	// Set current shade table to neutral color
	f->CurrentShade(FONT_SHADE_NEUTRAL);
}
