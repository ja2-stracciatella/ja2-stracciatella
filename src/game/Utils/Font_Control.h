#ifndef __FONT_CONTROL_H
#define __FONT_CONTROL_H

#include "Types.h"


extern SGPFont gp10PointArial;
extern SGPFont gp10PointArialBold;
extern SGPFont gp12PointArial;
extern SGPFont gp12PointArialFixedFont;
extern SGPFont gp12PointFont1;
extern SGPFont gp14PointArial;
extern SGPFont gp14PointHumanist;
extern SGPFont gp16PointArial;
extern SGPFont gpBlockFontNarrow;
extern SGPFont gpBlockyFont;
extern SGPFont gpBlockyFont2;
extern SGPFont gpLargeFontType1;
extern SGPFont gpSmallFontType1;
extern SGPFont gpTinyFontType1;
extern SGPFont gpCompFont;
extern SGPFont gpSmallCompFont;

extern SGPFont gpHugeFont;


// Defines
#define LARGEFONT1		gpLargeFontType1
#define SMALLFONT1		gpSmallFontType1
#define TINYFONT1		gpTinyFontType1
#define FONT12POINT1		gp12PointFont1
#define COMPFONT		gpCompFont
#define SMALLCOMPFONT		gpSmallCompFont
#define MILITARYFONT1		BLOCKFONT
#define FONT10ARIAL		gp10PointArial
#define FONT14ARIAL		gp14PointArial
#define FONT12ARIAL		gp12PointArial
#define FONT10ARIALBOLD	gp10PointArialBold
#define BLOCKFONT		gpBlockyFont
#define BLOCKFONT2		gpBlockyFont2
#define FONT12ARIALFIXEDWIDTH	gp12PointArialFixedFont
#define FONT16ARIAL		gp16PointArial
#define BLOCKFONTNARROW	gpBlockFontNarrow
#define FONT14HUMANIST		gp14PointHumanist

#define HUGEFONT		((GameMode::getInstance()->isEditorMode() && isEnglishVersion()) ? gpHugeFont : gp16PointArial)

#define FONT_MCOLOR_BLACK	0
#define FONT_MCOLOR_WHITE	208
#define FONT_MCOLOR_DKWHITE	134
#define FONT_MCOLOR_DKWHITE2	134
#define FONT_MCOLOR_LTGRAY	134
#define FONT_MCOLOR_LTGRAY2	134
#define FONT_MCOLOR_DKGRAY	136
#define FONT_MCOLOR_LTBLUE	203
#define FONT_MCOLOR_LTRED	162
#define FONT_MCOLOR_RED	163
#define FONT_MCOLOR_DKRED	164
#define FONT_MCOLOR_LTGREEN	184
#define FONT_MCOLOR_LTYELLOW	144

//Grayscale font colors
#define FONT_WHITE		208	//lightest color
#define FONT_GRAY1		133
#define FONT_GRAY2		134	//light gray
#define FONT_GRAY3		135
#define FONT_GRAY4		136	//gray
#define FONT_GRAY5		137
#define FONT_GRAY6		138
#define FONT_GRAY7		139	//dark gray
#define FONT_GRAY8		140
#define FONT_NEARBLACK		141
#define FONT_BLACK		0	//darkest color
//Color font colors
#define FONT_LTRED		162
#define FONT_RED		163
#define FONT_DKRED		218
#define FONT_ORANGE		76
#define FONT_YELLOW		145
#define FONT_DKYELLOW		80
#define FONT_LTGREEN		184
#define FONT_GREEN		185
#define FONT_DKGREEN		186
#define FONT_LTBLUE		71
#define FONT_BLUE		203
#define FONT_DKBLUE		205

#define FONT_BEIGE		130
#define FONT_METALGRAY		94
#define FONT_BURGUNDY		172
#define FONT_LTKHAKI		88
#define FONT_KHAKI		198
#define FONT_DKKHAKI		201

void InitializeFonts(void);
void ShutdownFonts(void);

enum FontShade
{
	FONT_SHADE_GREY_165 = 0,
	FONT_SHADE_BLUE     = 1,
	FONT_SHADE_GREEN    = 2,
	FONT_SHADE_YELLOW   = 3,
	FONT_SHADE_NEUTRAL  = 4,
	FONT_SHADE_WHITE    = 5,
	FONT_SHADE_RED      = 6
};

void SetFontShade(SGPFont, FontShade);

#endif
