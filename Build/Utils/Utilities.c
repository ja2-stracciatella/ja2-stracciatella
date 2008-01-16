#include "Font.h"
#include "HImage.h"
#include "Types.h"
#include "VObject.h"
#include "FileMan.h"
#include "Font_Control.h"
#include "Overhead.h"
#include "Overhead_Types.h"
#include "WCheck.h"
#include "Debug.h"
#include "VSurface.h"


BOOLEAN CreateSGPPaletteFromCOLFile( SGPPaletteEntry *pPalette, SGPFILENAME ColFile )
{
  HWFILE     hFileHandle;
	BYTE			 bColHeader[ 8 ];
	UINT32		 cnt;

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


BOOLEAN DisplayPaletteRep(const PaletteRepID aPalRep, const UINT8 ubXPos, const UINT8 ubYPos, SGPVSurface* const dst)
{
	UINT16										us16BPPColor;
	UINT32										cnt1;
	UINT8											ubSize;
	INT16											 sTLX, sTLY, sBRX, sBRY;
	UINT8											ubPaletteRep;

	// Create 16BPP Palette
	CHECKF( GetPaletteRepIndexFromID( aPalRep, &ubPaletteRep ) );

	SetFont( LARGEFONT1 );

	ubSize = gpPalRep[ ubPaletteRep ].ubPaletteSize;

	for ( cnt1 = 0; cnt1 < ubSize; cnt1++ )
	{
		sTLX = ubXPos + (UINT16)( ( cnt1 % 16 ) * 20 );
		sTLY = ubYPos + (UINT16)( ( cnt1 / 16 ) * 20 );
		sBRX = sTLX + 20;
		sBRY = sTLY + 20;

		const SGPPaletteEntry* Clr = &gpPalRep[ubPaletteRep].rgb[cnt1];
		us16BPPColor = Get16BPPColor(FROMRGB(Clr->peRed, Clr->peGreen, Clr->peBlue));

		ColorFillVideoSurfaceArea(dst, sTLX, sTLY, sBRX, sBRY, us16BPPColor);
	}

	gprintf(ubXPos + 16 * 20, ubYPos, L"%hs", gpPalRep[ubPaletteRep].ID);

	return( TRUE );
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

		hFile = FileOpen(zCdFile, FILE_ACCESS_READ);

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
