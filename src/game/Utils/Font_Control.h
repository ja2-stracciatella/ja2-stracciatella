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

#define FONT_MCOLOR_TRANSPARENT 0x00000000
#define FONT_MCOLOR_BLACK	0x000000FF
#define FONT_MCOLOR_WHITE	0xFFFFFFFF
#define FONT_MCOLOR_DKWHITE	0xBEBEBEFF
#define FONT_MCOLOR_DKWHITE2	0xBEBEBEFF
#define FONT_MCOLOR_LTGRAY	0xBEBEBEFF
#define FONT_MCOLOR_LTGRAY2	0xBEBEBEFF
#define FONT_MCOLOR_DKGRAY	0x898989FF
#define FONT_MCOLOR_LTBLUE	0x0000FFFF
#define FONT_MCOLOR_LTRED	0xFF4040FF
#define FONT_MCOLOR_RED	0xFF0000FF
#define FONT_MCOLOR_DKRED	0xCD0000FF
#define FONT_MCOLOR_LTGREEN	0x00CE00FF
#define FONT_MCOLOR_LTYELLOW	0xFFD73FFF

//Grayscale font colors
#define FONT_WHITE		0xFFFFFFFF	//lightest color
#define FONT_GRAY1		0xD9D9D9FF
#define FONT_GRAY2		0xBEBEBEFF	//light gray
#define FONT_GRAY3		0xA4A4A4FF
#define FONT_GRAY4		0x898989FF	//gray
#define FONT_GRAY5		0x6F6F6FFF
#define FONT_GRAY6		0x545454FF
#define FONT_GRAY7		0x3A3A3AFF	//dark gray
#define FONT_GRAY8		0x1F1F1FFF
#define FONT_NEARBLACK		0x050505FF
#define FONT_BLACK		0x000000FF	//darkest color
//Color font colors
#define FONT_LTRED		0xFF4040FF
#define FONT_RED		0xFF0000FF
#define FONT_DKRED		0xB20F0FFF
#define FONT_ORANGE		0xF0742DFF
#define FONT_YELLOW		0xFFC902FF
#define FONT_DKYELLOW		0xA46013FF
#define FONT_LTGREEN		0x00CE00FF
#define FONT_GREEN		0x007900FF
#define FONT_DKGREEN		0x004800FF
#define FONT_LTBLUE		0x11B2D8FF
#define FONT_BLUE		0x00FF00FF
#define FONT_DKBLUE		0x009A00FF

#define FONT_BEIGE		0xC9C58FFF
#define FONT_METALGRAY		0x4F5C66FF
#define FONT_BURGUNDY		0x710000FF
#define FONT_LTKHAKI		0x869F5FFF
#define FONT_KHAKI		0x4F5A2DFF
#define FONT_DKKHAKI		0x2E341BFF

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
