#ifndef __FONT_CONTROL_H
#define __FONT_CONTROL_H

#include "BuildDefines.h"
#include "Font.h"

extern      BOOLEAN     gfUseWinFonts;
extern      INT32       giCurWinFont;

// ATE: Use this define to enable winfonts in JA2
// #define     WINFONTS

#ifdef WINFONTS
  #define     USE_WINFONTS( )               ( gfUseWinFonts )
#else
  #define     USE_WINFONTS( )               ( FALSE )
#endif

#define     GET_WINFONT( )                ( giCurWinFont )
#define     SET_USE_WINFONTS( fSet )      ( gfUseWinFonts = fSet );
#define     SET_WINFONT( fFont )          ( giCurWinFont = fFont );

// ATE: A few winfont wrappers..
UINT16    WFGetFontHeight( INT32 FontNum );
INT16     WFStringPixLength( UINT16 *string,INT32 UseFont );





// Global variables for video objects
INT32						gpLargeFontType1;
HVOBJECT				gvoLargeFontType1;

INT32						gpSmallFontType1;
HVOBJECT				gvoSmallFontType1;

INT32						gpTinyFontType1;
HVOBJECT				gvoTinyFontType1;

INT32						gp12PointFont1;
HVOBJECT				gvo12PointFont1;

INT32           gpClockFont;
HVOBJECT        gvoClockFont;

INT32           gpCompFont;
HVOBJECT        gvoCompFont;

INT32           gpSmallCompFont;
HVOBJECT        gvoSmallCompFont;

INT32						gp10PointRoman;
HVOBJECT				gvo10PointRoman;

INT32						gp12PointRoman;
HVOBJECT				gvo12PointRoman;

INT32						gp14PointSansSerif;
HVOBJECT				gvo14PointSansSerif;

//INT32						gpMilitaryFont1;
//HVOBJECT				gvoMilitaryFont1;

INT32						gp10PointArial;
HVOBJECT				gvo10PointArial;

INT32						gp14PointArial;
HVOBJECT				gvo14PointArial;

INT32						gp12PointArial;
HVOBJECT				gvo12PointArial;

INT32						gpBlockyFont;
HVOBJECT				gvoBlockyFont;

INT32						gpBlockyFont2;
HVOBJECT				gvoBlockyFont2;

INT32						gp10PointArialBold;
HVOBJECT				gvo10PointArialBold;

INT32						gp12PointArialFixedFont;
HVOBJECT				gvo12PointArialFixedFont;

INT32						gp16PointArial;
HVOBJECT				gvo16PointArial;

INT32						gpBlockFontNarrow;
HVOBJECT				gvoBlockFontNarrow;

INT32						gp14PointHumanist;
HVOBJECT				gvo14PointHumanist;

#ifdef JA2EDITOR
INT32						gpHugeFont;
HVOBJECT				gvoHugeFont;
#endif

INT32           giSubTitleWinFont;


BOOLEAN					gfFontsInit;

// Defines
#define					LARGEFONT1				gpLargeFontType1
#define					SMALLFONT1				gpSmallFontType1
#define					TINYFONT1				  gpTinyFontType1
#define					FONT12POINT1			gp12PointFont1
#define					CLOCKFONT			    gpClockFont
#define         COMPFONT          gpCompFont
#define         SMALLCOMPFONT     gpSmallCompFont
#define					FONT10ROMAN				gp10PointRoman
#define					FONT12ROMAN				gp12PointRoman
#define					FONT14SANSERIF		gp14PointSansSerif
#define					MILITARYFONT1			BLOCKFONT		//gpMilitaryFont1
#define					FONT10ARIAL				gp10PointArial
#define					FONT14ARIAL				gp14PointArial
#define         FONT12ARIAL       gp12PointArial
#define         FONT10ARIALBOLD   gp10PointArialBold
#define         BLOCKFONT         gpBlockyFont
#define         BLOCKFONT2        gpBlockyFont2
#define					FONT12ARIALFIXEDWIDTH gp12PointArialFixedFont
#define					FONT16ARIAL				gp16PointArial
#define					BLOCKFONTNARROW		gpBlockFontNarrow
#define					FONT14HUMANIST		gp14PointHumanist

#if defined( JA2EDITOR ) && defined( ENGLISH )
	#define				HUGEFONT					gpHugeFont
#else
	#define				HUGEFONT					gp16PointArial
#endif


#define					FONT_SHADE_RED					6
#define					FONT_SHADE_BLUE					1
#define					FONT_SHADE_GREEN				2
#define					FONT_SHADE_YELLOW				3
#define					FONT_SHADE_NEUTRAL			4
#define					FONT_SHADE_WHITE				5

#define					FONT_MCOLOR_BLACK				0
#define					FONT_MCOLOR_WHITE				208
#define					FONT_MCOLOR_DKWHITE			134
#define					FONT_MCOLOR_DKWHITE2		134
#define					FONT_MCOLOR_LTGRAY			134
#define					FONT_MCOLOR_LTGRAY2			134
#define					FONT_MCOLOR_DKGRAY			136
#define					FONT_MCOLOR_LTBLUE			203
#define					FONT_MCOLOR_LTRED				162
#define					FONT_MCOLOR_RED					163
#define					FONT_MCOLOR_DKRED				164
#define					FONT_MCOLOR_LTGREEN			184
#define					FONT_MCOLOR_LTYELLOW		144

//Grayscale font colors
#define	FONT_WHITE				208	//lightest color
#define	FONT_GRAY1				133
#define	FONT_GRAY2				134	//light gray
#define	FONT_GRAY3				135
#define	FONT_GRAY4				136	//gray
#define	FONT_GRAY5				137
#define	FONT_GRAY6				138
#define	FONT_GRAY7				139	//dark gray
#define	FONT_GRAY8				140
#define FONT_NEARBLACK		141
#define	FONT_BLACK				0		//darkest color
//Color font colors
#define	FONT_LTRED				162
#define	FONT_RED					163
#define	FONT_DKRED				218
#define FONT_ORANGE				76
#define	FONT_YELLOW				145
#define FONT_DKYELLOW			80
#define FONT_LTGREEN			184
#define FONT_GREEN				185
#define FONT_DKGREEN			186
#define FONT_LTBLUE				71
#define FONT_BLUE					203
#define FONT_DKBLUE				205

#define FONT_BEIGE				130
#define FONT_METALGRAY		94
#define FONT_BURGUNDY			172
#define FONT_LTKHAKI			88
#define FONT_KHAKI				198
#define FONT_DKKHAKI			201

BOOLEAN	InitializeFonts( );
void ShutdownFonts( );

BOOLEAN SetFontShade( UINT32 uiFontID, INT8 bColorID );

#endif
