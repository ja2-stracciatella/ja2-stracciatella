#include "Font.h"
#include "HImage.h"
#include "LoadSaveData.h"
#include "Types.h"
#include "Utilities.h"
#include "VObject.h"
#include "FileMan.h"
#include "Font_Control.h"
#include "Overhead.h"
#include "Overhead_Types.h"
#include "Debug.h"
#include "VSurface.h"


BOOLEAN CreateSGPPaletteFromCOLFile(SGPPaletteEntry* const pal, const char* const col_file)
try
{
	AutoSGPFile f(FileMan::openForReadingSmart(col_file, true));

	BYTE data[776];
	FileRead(f, data, sizeof(data));

	const BYTE* d = data;
	EXTR_SKIP(d, 8); // skip header
	for (UINT i = 0; i != 256; ++i)
	{
		EXTR_U8(d, pal[i].r)
		EXTR_U8(d, pal[i].g)
		EXTR_U8(d, pal[i].b)
	}
	Assert(d == endof(data));

	return TRUE;
}
catch (...) { return FALSE; }


void DisplayPaletteRep(const PaletteRepID aPalRep, const UINT8 ubXPos, const UINT8 ubYPos, SGPVSurface* const dst)
{
	UINT16										us16BPPColor;
	UINT32										cnt1;
	UINT8											ubSize;
	INT16											 sTLX, sTLY, sBRX, sBRY;

	// Create 16BPP Palette
	const UINT8 ubPaletteRep = GetPaletteRepIndexFromID(aPalRep);

	SetFont( LARGEFONT1 );

	ubSize = gpPalRep[ ubPaletteRep ].ubPaletteSize;

	for ( cnt1 = 0; cnt1 < ubSize; cnt1++ )
	{
		sTLX = ubXPos + (UINT16)( ( cnt1 % 16 ) * 20 );
		sTLY = ubYPos + (UINT16)( ( cnt1 / 16 ) * 20 );
		sBRX = sTLX + 20;
		sBRY = sTLY + 20;

		const SGPPaletteEntry* Clr = &gpPalRep[ubPaletteRep].rgb[cnt1];
		us16BPPColor = Get16BPPColor(FROMRGB(Clr->r, Clr->g, Clr->b));

		ColorFillVideoSurfaceArea(dst, sTLX, sTLY, sBRX, sBRY, us16BPPColor);
	}

	gprintf(ubXPos + 16 * 20, ubYPos, L"%hs", gpPalRep[ubPaletteRep].ID);
}
