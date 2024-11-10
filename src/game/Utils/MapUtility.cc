#include "Input.h"
#include "MapUtility.h"
#include "Overhead_Types.h"
#include "SDL_keycode.h"
#include "SDL_timer.h"
#include "SGP.h"
#include "Font.h"
#include "HImage.h"
#include "Local.h"
#include "VObject.h"
#include "VSurface.h"
#include "WorldDef.h"
#include "FileMan.h"
#include "Overhead_Map.h"
#include "VObject_Blitters.h"
#include "STIConvert.h"
#include "Font_Control.h"
#include "WorldDat.h"
#include "Map_Information.h"
#include "Line.h"
#include "Video.h"
#include "Quantize.h"
#include "UILayout.h"

#include <memory>
#include <string_theory/format>


#define MINIMAP_X_SIZE		88
#define MINIMAP_Y_SIZE		44

#define WINDOW_SIZE		2

static float     gdXStep;
static float     gdYStep;

// Utililty file for sub-sampling/creating our radar screen maps
// Loops though our maps directory and reads all .map files, subsamples an area, color
// quantizes it into an 8-bit image ans writes it to an sti file in radarmaps.


ScreenID MapUtilScreenHandle()
{
	static auto p24BitValues{ std::make_unique<SGPPaletteEntry[]>(MINIMAP_X_SIZE * MINIMAP_Y_SIZE) };

	static SGPVSurface* giMiniMap{ AddVideoSurface(88, 44, PIXEL_DEPTH) };
	static SGPVSurface* gi8BitMiniMap{ AddVideoSurface(88, 44, 8) };

	// Get the names (full path) of all map files in the user's home directory.
	static auto const mapFiles{ FileMan::findFilesInDir(
		RustPointer<char>{ EngineOptions_getStracciatellaHome() }.get(),
		"dat", true, false) };

	// Set the file iterator to the first file.
	static auto currentFile{ mapFiles.begin() };

	UINT32 uiRGBColor;

	UINT32 bR, bG, bB, bAvR, bAvG, bAvB;
	INT16 s16BPPSrc, sDest16BPPColor;

	INT16 sX1, sX2, sY1, sY2, sTop, sBottom, sLeft, sRight;


	FLOAT dX, dY, dStartX, dStartY;
	INT32 iX, iY, iSubX1, iSubY1, iSubX2, iSubY2, iWindowX, iWindowY, iCount;
	SGPPaletteEntry pPalette[ 256 ];


	sDest16BPPColor = -1;
	bAvR = bAvG = bAvB = 0;

	FRAME_BUFFER->Fill(Get16BPPColor(FROMRGB(0, 0, 0)));

	//OK, we are here, now loop through files
	if (currentFile == mapFiles.end())
	{
		requestGameExit();
		return MAPUTILITY_SCREEN;
	}

	// OK, load maps and do overhead shrinkage of them...
	try { LoadWorldAbsolute(*currentFile); }
	catch (...) { return ERROR_SCREEN; }

	// Render small map
	InitNewOverheadDB(giCurrentTilesetID);

	gfOverheadMapDirty = TRUE;

	RenderOverheadMap(0, WORLD_COLS / 2, 0, 0, SCREEN_WIDTH, 320, TRUE);

	TrashOverheadMap( );

	// OK, NOW PROCESS OVERHEAD MAP ( SHOUIDL BE ON THE FRAMEBUFFER )
	gdXStep	= SCREEN_WIDTH / 88.f;
	gdYStep	= 320 / 44.f;
	dStartX = dStartY = 0;

	// Adjust if we are using a restricted map...
	if ( gMapInformation.ubRestrictedScrollID != 0 )
	{

		CalculateRestrictedMapCoords(NORTH, &sX1,    &sY1,     &sX2,   &sTop, SCREEN_WIDTH, 320);
		CalculateRestrictedMapCoords(SOUTH, &sX1,    &sBottom, &sX2,   &sY2,  SCREEN_WIDTH, 320);
		CalculateRestrictedMapCoords(WEST,  &sX1,    &sY1,     &sLeft, &sY2,  SCREEN_WIDTH, 320);
		CalculateRestrictedMapCoords(EAST,  &sRight, &sY1,     &sX2,   &sY2,  SCREEN_WIDTH, 320);

		gdXStep	= (float)( sRight - sLeft )/(float)88;
		gdYStep	= (float)( sBottom - sTop )/(float)44;

		dStartX = sLeft;
		dStartY = sTop;
	}

	//LOCK BUFFERS

	dX = dStartX;
	dY = dStartY;


	{ SGPVSurface::Lock lsrc(FRAME_BUFFER);
		SGPVSurface::Lock ldst(giMiniMap);
		UINT16* const pSrcBuf          = lsrc.Buffer<UINT16>();
		UINT32  const uiSrcPitchBYTES  = lsrc.Pitch();
		UINT16* const pDestBuf         = ldst.Buffer<UINT16>();
		UINT32  const uiDestPitchBYTES = ldst.Pitch();

		for ( iX = 0; iX < 88; iX++ )
		{
			dY = dStartY;

			for ( iY = 0; iY < 44; iY++ )
			{
				//OK, AVERAGE PIXELS
				iSubX1 = (INT32)dX - WINDOW_SIZE;

				iSubX2 = (INT32)dX + WINDOW_SIZE;

				iSubY1 = (INT32)dY - WINDOW_SIZE;

				iSubY2 = (INT32)dY + WINDOW_SIZE;

				iCount = 0;
				bR = bG = bB = 0;

				for ( iWindowX = iSubX1; iWindowX < iSubX2; iWindowX++ )
				{
					for ( iWindowY = iSubY1; iWindowY < iSubY2; iWindowY++ )
					{
						if (0 <= iWindowX && iWindowX < SCREEN_WIDTH &&
								0 <= iWindowY && iWindowY < 320)
						{
							s16BPPSrc = pSrcBuf[ ( iWindowY * (uiSrcPitchBYTES/2) ) + iWindowX ];

							uiRGBColor = GetRGBColor( s16BPPSrc );

							bR += SGPGetRValue( uiRGBColor );
							bG += SGPGetGValue( uiRGBColor );
							bB += SGPGetBValue( uiRGBColor );

							// Average!
							iCount++;
						}
					}

				}

				if ( iCount > 0 )
				{
					bAvR = bR / (UINT8)iCount;
					bAvG = bG / (UINT8)iCount;
					bAvB = bB / (UINT8)iCount;

					sDest16BPPColor = Get16BPPColor( FROMRGB( bAvR, bAvG, bAvB ) );
				}

				//Write into dest!
				pDestBuf[ ( iY * (uiDestPitchBYTES/2) ) + iX ] = sDest16BPPColor;

				SGPPaletteEntry* const dst = &p24BitValues[iY * (uiDestPitchBYTES / 2) + iX];
				dst->r = bAvR;
				dst->g = bAvG;
				dst->b = bAvB;

				//Increment
				dY += gdYStep;

			}

			//Increment
			dX += gdXStep;
		}
	}

	// RENDER!
	BltVideoSurface(FRAME_BUFFER, giMiniMap, 20, 360, NULL);


	ST::string zFilename2;
	//QUantize!
	{ SGPVSurface::Lock lsrc(gi8BitMiniMap);
		UINT8* const pDataPtr = lsrc.Buffer<UINT8>();
		{ SGPVSurface::Lock ldst(FRAME_BUFFER);
			UINT16* const pDestBuf         = ldst.Buffer<UINT16>();
			UINT32  const uiDestPitchBYTES = ldst.Pitch();
			QuantizeImage(pDataPtr, p24BitValues.get(), MINIMAP_X_SIZE, MINIMAP_Y_SIZE, pPalette);
			gi8BitMiniMap->SetPalette(pPalette);
			// Blit!
			Blt8BPPDataTo16BPPBuffer(pDestBuf, uiDestPitchBYTES, gi8BitMiniMap, pDataPtr, 300, 360);

			// Write palette!
			{
				INT32 cnt;
				INT32 sX = 0, sY = 420;
				UINT16 usLineColor;

				SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

				for ( cnt = 0; cnt < 256; cnt++ )
				{
					usLineColor = Get16BPPColor(FROMRGB(pPalette[cnt].r, pPalette[cnt].g, pPalette[cnt].b));
					LineDraw(FALSE, sX, sY, sX, sY + 10, usLineColor, pDestBuf);
					sX++;
					LineDraw(FALSE, sX, sY, sX, sY + 10, usLineColor, pDestBuf);
					sX++;
				}
			}
		}

		zFilename2 = FileMan::replaceExtension(*currentFile, "sti");
		WriteSTIFile(pDataPtr, pPalette, MINIMAP_X_SIZE, MINIMAP_Y_SIZE, zFilename2, CONVERT_ETRLE_COMPRESS, 0);
	}

	SetFontAttributes(TINYFONT1, FONT_MCOLOR_DKGRAY);
	MPrint(10, 340, ST::format("Writing radar image {}", zFilename2));
	MPrint(10, 350, ST::format("Using tileset {}", gTilesets[giCurrentTilesetID].zName));

	InvalidateScreen();
	RefreshScreen();

	InputAtom InputEvent;
	while (DequeueEvent(&InputEvent))
	{
		if (InputEvent.usEvent == KEY_DOWN && InputEvent.usParam == SDLK_ESCAPE)
		{ // Exit the program
			requestGameExit();
		}
	}

	if (_KeyDown(SDLK_SPACE))
	{
		// Wait two seconds to get the chance to see what is happening
		// for debugging purposes.
		SDL_Delay(2000);
	}

	// Set next
	++currentFile;

	return MAPUTILITY_SCREEN;
}
