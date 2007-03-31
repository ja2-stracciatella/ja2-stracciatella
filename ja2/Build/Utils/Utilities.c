#include "Types.h"
#include <stdio.h>
#include "SGP.h"
#include "time.h"
#include "VObject.h"
#include "FileMan.h"
#include "Utilities.h"
#include "Font_Control.h"
#include "Overhead.h"
#include "Overhead_Types.h"
#include "WCheck.h"
#include "Sys_Globals.h"
#include "Debug.h"
#include "VSurface.h"


BOOLEAN CreateSGPPaletteFromCOLFile( SGPPaletteEntry *pPalette, SGPFILENAME ColFile )
{
  HWFILE     hFileHandle;
	BYTE			 bColHeader[ 8 ];
	UINT32		 cnt;

	//See if files exists, if not, return error
	if ( !FileExists( ColFile ) )
	{
		// Return FALSE w/ debug
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Cannot find COL file");
		return( FALSE );
	}

	// Open and read in the file
	hFileHandle = FileOpen(ColFile, FILE_ACCESS_READ);
	if (hFileHandle == 0)
	{
		// Return FALSE w/ debug
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Cannot open COL file");
		return( FALSE );
	}

	// Skip header
  FileRead(hFileHandle, bColHeader, sizeof(bColHeader));

	// Read in a palette entry at a time
	for ( cnt = 0; cnt < 256; cnt++ )
	{
	  FileRead(hFileHandle, &pPalette[cnt].peRed,   sizeof(UINT8));
	  FileRead(hFileHandle, &pPalette[cnt].peGreen, sizeof(UINT8));
	  FileRead(hFileHandle, &pPalette[cnt].peBlue,  sizeof(UINT8));
	}

	// Close file
	FileClose( hFileHandle );

	return( TRUE );
}

BOOLEAN DisplayPaletteRep( PaletteRepID aPalRep, UINT8 ubXPos, UINT8 ubYPos, UINT32 uiDestSurface )
{
	UINT16										us16BPPColor;
	UINT32										cnt1;
	UINT8											ubSize, ubType;
	INT16											 sTLX, sTLY, sBRX, sBRY;
	UINT8											ubPaletteRep;

	// Create 16BPP Palette
	CHECKF( GetPaletteRepIndexFromID( aPalRep, &ubPaletteRep ) );

	SetFont( LARGEFONT1 );

	ubType = gpPalRep[ ubPaletteRep ].ubType;
	ubSize = gpPalRep[ ubPaletteRep ].ubPaletteSize;

	for ( cnt1 = 0; cnt1 < ubSize; cnt1++ )
	{
		sTLX = ubXPos + (UINT16)( ( cnt1 % 16 ) * 20 );
		sTLY = ubYPos + (UINT16)( ( cnt1 / 16 ) * 20 );
		sBRX = sTLX + 20;
		sBRY = sTLY + 20;

		us16BPPColor = Get16BPPColor( FROMRGB( gpPalRep[ ubPaletteRep ].r[ cnt1 ], gpPalRep[ ubPaletteRep ].g[ cnt1 ], gpPalRep[ ubPaletteRep ].b[ cnt1 ] ) );

		ColorFillVideoSurfaceArea( uiDestSurface, sTLX, sTLY, sBRX, sBRY, us16BPPColor );

	}

	gprintf( ubXPos + ( 16 * 20 ), ubYPos, L"%S", gpPalRep[ ubPaletteRep ].ID );

	return( TRUE );
}


BOOLEAN	 WrapString( wchar_t *pStr, wchar_t *pStr2, size_t Length, UINT16 usWidth, INT32 uiFont )
{
	UINT32 Cur, uiLet, uiNewLet, uiHyphenLet;
	wchar_t *curletter,transletter;
	BOOLEAN	fLineSplit = FALSE;
	HVOBJECT	hFont;

	// CHECK FOR WRAP
	Cur=0;
	uiLet = 0;
	curletter=pStr;

	// GET FONT
	hFont = GetFontObject( uiFont );

	// LOOP FORWARDS AND COUNT
	while((*curletter)!=0)
	{
		transletter=GetIndex(*curletter);
		Cur+=GetWidth( hFont, transletter );

		if ( Cur > usWidth )
		{
			// We are here, loop backwards to find a space
			// Generate second string, and exit upon completion.
			uiHyphenLet = uiLet;	//Save the hyphen location as it won't change.
			uiNewLet = uiLet;
			while((*curletter)!=0)
			{
				if ( (*curletter) == 32 )
				{
					 // Split Line!
					 fLineSplit = TRUE;

					 pStr[ uiNewLet ] = (INT16)'\0';

					 wcscpy( pStr2, &(pStr[ uiNewLet + 1 ]) );
				}

				if ( fLineSplit )
					break;

				uiNewLet--;
				curletter--;

			}
			if( !fLineSplit)
			{
				//We completed the check for a space, but failed, so use the hyphen method.
				swprintf(pStr2, Length, L"-%S", &pStr[uiHyphenLet]);
				#if 0 /* XXX typo? */
				pStr[uiHyphenLet] = (INT16)'/0';
				#else
				pStr[uiHyphenLet] = L'\0';
				#endif
				fLineSplit = TRUE;  //hyphen method
				break;
			}
		}

//		if ( fLineSplit )
//			break;

		uiLet++;
		curletter++;
	}

	return( fLineSplit );

}


SGPFILENAME	gCheckFilenames[] =
{
  "DATA/INTRO.SLF",
  "DATA/LOADSCREENS.SLF",
  "DATA/MAPS.SLF",
	"DATA/NPC_SPEECH.SLF",
	"DATA/SPEECH.SLF",
};


BOOLEAN DoJA2FilesExistsOnDrive( CHAR8 *zCdLocation )
{
	BOOLEAN fFailed = FALSE;
	CHAR8		zCdFile[ SGPFILENAME_LEN ];
  INT32   cnt;
	HWFILE	hFile;

  for ( cnt = 0; cnt < 4; cnt++ )
  {
		// OK, build filename
		sprintf( zCdFile, "%s%s", zCdLocation, gCheckFilenames[ cnt ] );

		hFile = FileOpen(zCdFile, FILE_ACCESS_READ | FILE_OPEN_EXISTING);

		// Check if it exists...
		if ( !hFile )
		{
			fFailed = TRUE;
      break;
		}
		FileClose( hFile );
	}

	return( !fFailed );
}
