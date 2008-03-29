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
#include "WCheck.h"
#include "Debug.h"
#include "VSurface.h"


BOOLEAN CreateSGPPaletteFromCOLFile(SGPPaletteEntry* const pal, const char* const col_file)
{
	const HWFILE f = FileOpen(col_file, FILE_ACCESS_READ);
	if (f == 0)
	{
		DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot open COL file");
		return FALSE;
	}

	BYTE data[776];
	const BOOLEAN ret = FileRead(f, data, sizeof(data));
	if (ret)
	{
		const BYTE* d = data;
		EXTR_SKIP(d, 8); // skip header
		for (UINT i = 0; i != 256; ++i)
		{
			EXTR_U8(d, pal[i].peRed)
			EXTR_U8(d, pal[i].peGreen)
			EXTR_U8(d, pal[i].peBlue)
		}
		Assert(d == endof(data));
	}

	FileClose(f);
	return ret;
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
