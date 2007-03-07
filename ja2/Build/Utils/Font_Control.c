#ifdef PRECOMPILEDHEADERS
#else
	#include <stdio.h>
	#include <stdarg.h>
	#include <time.h>
	#include "SGP.h"
	#include "HImage.h"
	#include "VSurface.h"
	#include "WCheck.h"
	#include "Font_Control.h"
	#include "MemMan.h"
#endif


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

INT32						gp10PointArialBold;
HVOBJECT				gvo10PointArialBold;

INT32						gp14PointArial;
HVOBJECT				gvo14PointArial;

INT32						gp12PointArial;
HVOBJECT				gvo12PointArial;

INT32           gpBlockyFont;
HVOBJECT				gvoBlockyFont;

INT32           gpBlockyFont2;
HVOBJECT				gvoBlockyFont2;

INT32						gp12PointArialFixedFont;
HVOBJECT				gvo12PointArialFixedFont;

INT32						gp16PointArial;
HVOBJECT				gvo16PointArial;

INT32						gpBlockFontNarrow;
HVOBJECT				gvoBlockFontNarrow;

INT32						gp14PointHumanist;
HVOBJECT				gvo14PointHumanist;

#if defined( JA2EDITOR ) && defined( ENGLISH )
	INT32					gpHugeFont;
	HVOBJECT			gvoHugeFont;
#endif


BOOLEAN					gfFontsInit = FALSE;

UINT16 CreateFontPaletteTables(HVOBJECT pObj );


BOOLEAN	InitializeFonts( )
{
	// Initialize fonts
//	gpLargeFontType1  = LoadFontFile( "FONTS/lfont1.sti" );
	gpLargeFontType1  = LoadFontFile( "FONTS/LARGEFONT1.sti" );
	gvoLargeFontType1 = GetFontObject( gpLargeFontType1 );
	CHECKF( CreateFontPaletteTables( gvoLargeFontType1 ) );

//	gpSmallFontType1  = LoadFontFile( "FONTS/6b-font.sti" );
	gpSmallFontType1  = LoadFontFile( "FONTS/SMALLFONT1.sti" );
	gvoSmallFontType1 = GetFontObject( gpSmallFontType1 );
	CHECKF( CreateFontPaletteTables( gvoSmallFontType1 ) );

//	gpTinyFontType1  = LoadFontFile( "FONTS/tfont1.sti" );
	gpTinyFontType1  = LoadFontFile( "FONTS/TINYFONT1.sti" );
	gvoTinyFontType1 = GetFontObject( gpTinyFontType1 );
	CHECKF( CreateFontPaletteTables( gvoTinyFontType1 ) );

//	gp12PointFont1	= LoadFontFile( "FONTS/font-12.sti" );
	gp12PointFont1	= LoadFontFile( "FONTS/FONT12POINT1.sti" );
	gvo12PointFont1	= GetFontObject( gp12PointFont1 );
	CHECKF( CreateFontPaletteTables( gvo12PointFont1 ) );


//  gpClockFont  = LoadFontFile( "FONTS/DIGI.sti" );
  gpClockFont  = LoadFontFile( "FONTS/CLOCKFONT.sti" );
  gvoClockFont = GetFontObject( gpClockFont );
  CHECKF( CreateFontPaletteTables( gvoClockFont ) );

//  gpCompFont  = LoadFontFile( "FONTS/compfont.sti" );
  gpCompFont  = LoadFontFile( "FONTS/COMPFONT.sti" );
  gvoCompFont = GetFontObject( gpCompFont );
  CHECKF( CreateFontPaletteTables( gvoCompFont ) );

//  gpSmallCompFont  = LoadFontFile( "FONTS/scfont.sti" );
  gpSmallCompFont  = LoadFontFile( "FONTS/SMALLCOMPFONT.sti" );
  gvoSmallCompFont = GetFontObject( gpSmallCompFont );
  CHECKF( CreateFontPaletteTables( gvoSmallCompFont ) );

//  gp10PointRoman  = LoadFontFile( "FONTS/Roman10.sti" );
  gp10PointRoman  = LoadFontFile( "FONTS/FONT10ROMAN.sti" );
  gvo10PointRoman = GetFontObject( gp10PointRoman );
  CHECKF( CreateFontPaletteTables( gvo10PointRoman ) );

//  gp12PointRoman  = LoadFontFile( "FONTS/Roman12.sti" );
  gp12PointRoman  = LoadFontFile( "FONTS/FONT12ROMAN.sti" );
  gvo12PointRoman = GetFontObject( gp12PointRoman );
  CHECKF( CreateFontPaletteTables( gvo12PointRoman ) );

//  gp14PointSansSerif  = LoadFontFile( "FONTS/SansSerif14.sti" );
  gp14PointSansSerif  = LoadFontFile( "FONTS/FONT14SANSERIF.sti" );
  gvo14PointSansSerif = GetFontObject( gp14PointSansSerif);
  CHECKF( CreateFontPaletteTables( gvo14PointSansSerif) );

//	DEF:	Removed.  Replaced with BLOCKFONT
//  gpMilitaryFont1  = LoadFontFile( "FONTS/milfont.sti" );
//  gvoMilitaryFont1 = GetFontObject( gpMilitaryFont1);
//  CHECKF( CreateFontPaletteTables( gvoMilitaryFont1) );


//  gp10PointArial  = LoadFontFile( "FONTS/Arial10.sti" );
  gp10PointArial  = LoadFontFile( "FONTS/FONT10ARIAL.sti" );
  gvo10PointArial = GetFontObject( gp10PointArial);
  CHECKF( CreateFontPaletteTables( gvo10PointArial) );

//  gp14PointArial  = LoadFontFile( "FONTS/Arial14.sti" );
  gp14PointArial  = LoadFontFile( "FONTS/FONT14ARIAL.sti" );
  gvo14PointArial = GetFontObject( gp14PointArial);
  CHECKF( CreateFontPaletteTables( gvo14PointArial) );

//  gp10PointArialBold  = LoadFontFile( "FONTS/Arial10Bold2.sti" );
  gp10PointArialBold  = LoadFontFile( "FONTS/FONT10ARIALBOLD.sti" );
  gvo10PointArialBold  = GetFontObject( gp10PointArialBold);
  CHECKF( CreateFontPaletteTables( gvo10PointArialBold) );

//  gp12PointArial  = LoadFontFile( "FONTS/Arial12.sti" );
  gp12PointArial  = LoadFontFile( "FONTS/FONT12ARIAL.sti" );
  gvo12PointArial = GetFontObject( gp12PointArial);
  CHECKF( CreateFontPaletteTables( gvo12PointArial) );

//	gpBlockyFont  = LoadFontFile( "FONTS/FONT2.sti" );
	gpBlockyFont  = LoadFontFile( "FONTS/BLOCKFONT.sti" );
  gvoBlockyFont = GetFontObject( gpBlockyFont);
  CHECKF( CreateFontPaletteTables( gvoBlockyFont) );

//	gpBlockyFont2  = LoadFontFile( "FONTS/interface_font.sti" );
	gpBlockyFont2  = LoadFontFile( "FONTS/BLOCKFONT2.sti" );
  gvoBlockyFont2 = GetFontObject( gpBlockyFont2);
  CHECKF( CreateFontPaletteTables( gvoBlockyFont2) );

//	gp12PointArialFixedFont = LoadFontFile( "FONTS/Arial12FixedWidth.sti" );
	gp12PointArialFixedFont = LoadFontFile( "FONTS/FONT12ARIALFIXEDWIDTH.sti" );
	gvo12PointArialFixedFont = GetFontObject( gp12PointArialFixedFont );
	CHECKF( CreateFontPaletteTables( gvo12PointArialFixedFont ) );

	gp16PointArial = LoadFontFile( "FONTS/FONT16ARIAL.sti" );
	gvo16PointArial = GetFontObject( gp16PointArial );
	CHECKF( CreateFontPaletteTables( gvo16PointArial ) );

	gpBlockFontNarrow = LoadFontFile( "FONTS/BLOCKFONTNARROW.sti" );
	gvoBlockFontNarrow = GetFontObject( gpBlockFontNarrow );
	CHECKF( CreateFontPaletteTables( gvoBlockFontNarrow ) );

	gp14PointHumanist = LoadFontFile( "FONTS/FONT14HUMANIST.sti" );
	gvo14PointHumanist = GetFontObject( gp14PointHumanist );
	CHECKF( CreateFontPaletteTables( gvo14PointHumanist ) );

	#if defined( JA2EDITOR ) && defined( ENGLISH )
		gpHugeFont = LoadFontFile( "FONTS/HUGEFONT.sti" );
		gvoHugeFont = GetFontObject( gpHugeFont );
		CHECKF( CreateFontPaletteTables( gvoHugeFont ) );
	#endif

	// Set default for font system
	SetFontDestBuffer( FRAME_BUFFER, 0, 0, 640, 480, FALSE );

	gfFontsInit = TRUE;

	return( TRUE );
}

void ShutdownFonts( )
{
	UnloadFont( gpLargeFontType1 );
	UnloadFont( gpSmallFontType1 );
	UnloadFont( gpTinyFontType1 );
	UnloadFont( gp12PointFont1 );
	UnloadFont( gpClockFont);
  UnloadFont( gpCompFont);
	UnloadFont( gpSmallCompFont);
	UnloadFont( gp10PointRoman);
	UnloadFont( gp12PointRoman);
	UnloadFont( gp14PointSansSerif);
//	UnloadFont( gpMilitaryFont1);
	UnloadFont( gp10PointArial);
  UnloadFont( gp10PointArialBold);
	UnloadFont( gp14PointArial);
  UnloadFont( gpBlockyFont);
	UnloadFont( gp12PointArialFixedFont );
	#if defined( JA2EDITOR ) && defined( ENGLISH )
		UnloadFont( gpHugeFont );
	#endif
}

// Set shades for fonts
BOOLEAN SetFontShade( UINT32 uiFontID, INT8 bColorID )
{
	HVOBJECT pFont;

	CHECKF( bColorID > 0 );
	CHECKF( bColorID < 16 );

	pFont	= GetFontObject( uiFontID );

	pFont->pShadeCurrent = pFont->pShades[ bColorID ];

	return( TRUE );
}

UINT16 CreateFontPaletteTables(HVOBJECT pObj )
{
	UINT32 count;
	SGPPaletteEntry Pal[256];

	for( count = 0; count < 16; count++ )
	{
		if ( (count == 4) && (pObj->p16BPPPalette == pObj->pShades[ count ]) )
			pObj->pShades[ count ] = NULL;
		else if ( pObj->pShades[ count ] != NULL )
		{
			MemFree( pObj->pShades[ count ] );
			pObj->pShades[ count ] = NULL;
		}
	}

	// Build white palette
	for(count=0; count < 256; count++)
	{
		Pal[count].peRed=(UINT8)255;
		Pal[count].peGreen=(UINT8)255;
		Pal[count].peBlue=(UINT8)255;
	}

	pObj->pShades[ FONT_SHADE_RED ]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 255, 0, 0, TRUE);
	pObj->pShades[ FONT_SHADE_BLUE ]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 0, 0, 255, TRUE);
	pObj->pShades[ FONT_SHADE_GREEN ]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 0, 255, 0, TRUE);
	pObj->pShades[ FONT_SHADE_YELLOW ]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 255, 255, 0, TRUE);
	pObj->pShades[ FONT_SHADE_NEUTRAL ]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 255, 255, 255, FALSE);

	pObj->pShades[ FONT_SHADE_WHITE ]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 255, 255, 255, TRUE);


	// the rest are darkening tables, right down to all-black.
	pObj->pShades[0]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 165, 165, 165, FALSE);
	pObj->pShades[7]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 135, 135, 135, FALSE);
	pObj->pShades[8]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 105, 105, 105, FALSE);
	pObj->pShades[9]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 75, 75, 75, FALSE);
	pObj->pShades[10]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 45, 45, 45, FALSE);
	pObj->pShades[11]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 36, 36, 36, FALSE);
	pObj->pShades[12]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 27, 27, 27, FALSE);
	pObj->pShades[13]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 18, 18, 18, FALSE);
	pObj->pShades[14]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 9, 9, 9, FALSE);
	pObj->pShades[15]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 0, 0, 0, FALSE);

	// Set current shade table to neutral color
	pObj->pShadeCurrent=pObj->pShades[4];

	// check to make sure every table got a palette
	//for(count=0; (count < HVOBJECT_SHADE_TABLES) && (pObj->pShades[count]!=NULL); count++);

	// return the result of the check
	//return(count==HVOBJECT_SHADE_TABLES);
	return(TRUE);
}
