#include "Font.h"
#include "HImage.h"
#include "LoadSaveData.h"
#include "Soldier_Control.h"
#include "Types.h"
#include "Utilities.h"
#include "VObject.h"
#include "Font_Control.h"
#include "Debug.h"
#include "VSurface.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>
#include <string_theory/string>


BOOLEAN CreateSGPPaletteFromCOLFile(SGPPaletteEntry* const pal, const char* const col_file)
try
{
	AutoSGPFile f(GCM->openGameResForReading(col_file));

	BYTE data[776];
	f->read(data, sizeof(data));

	DataReader d{data};
	EXTR_SKIP(d, 8); // skip header
	for (UINT i = 0; i != 256; ++i)
	{
		EXTR_U8(d, pal[i].r)
		EXTR_U8(d, pal[i].g)
		EXTR_U8(d, pal[i].b)
	}
	Assert(d.getConsumed() == lengthof(data));

	return true;
}
catch (...) { return false; }


void DisplayPaletteRep(const ST::string& aPalRep, UINT8 ubXPos, UINT8 ubYPos, SGPVSurface* dst)
{
	UINT16 us16BPPColor;
	UINT32 cnt1;
	INT16  sTLX, sTLY, sBRX, sBRY;

	// Create 16BPP Palette
	auto const ubPaletteRep = GetPaletteRepIndexFromID(aPalRep);
	if (!ubPaletteRep)
	{
		return;
	}

	SetFont( LARGEFONT1 );

	auto const ubSize = gpPalRep[*ubPaletteRep].ubPaletteSize;

	for ( cnt1 = 0; cnt1 < ubSize; cnt1++ )
	{
		sTLX = ubXPos + (UINT16)( ( cnt1 % 16 ) * 20 );
		sTLY = ubYPos + (UINT16)( ( cnt1 / 16 ) * 20 );
		sBRX = sTLX + 20;
		sBRY = sTLY + 20;

		auto const * const Clr = &gpPalRep[*ubPaletteRep].rgb[cnt1];
		us16BPPColor = Get16BPPColor(FROMRGB(Clr->r, Clr->g, Clr->b));

		ColorFillVideoSurfaceArea(dst, sTLX, sTLY, sBRX, sBRY, us16BPPColor);
	}

	GPrint(ubXPos + 16 * 20, ubYPos, gpPalRep[*ubPaletteRep].ID);
}
