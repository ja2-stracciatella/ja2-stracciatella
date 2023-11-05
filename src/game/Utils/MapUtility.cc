#include "Directories.h"
#include "Input.h"
#include "MapUtility.h"
#include "SGP.h"
#include "Font.h"
#include "HImage.h"
#include "Local.h"
#include "Screens.h"
#include "VObject.h"
#include "VSurface.h"
#include "WorldDef.h"
#include "Overhead.h"
#include "FileMan.h"
#include "LoadScreen.h"
#include "Overhead_Map.h"
#include "Radar_Screen.h"
#include "VObject_Blitters.h"
#include "STIConvert.h"
#include "Font_Control.h"
#include "WorldDat.h"
#include "English.h"
#include "Map_Information.h"
#include "Line.h"
#include "MemMan.h"
#include "Video.h"
#include "UILayout.h"

#include "ContentManager.h"
#include "GameInstance.h"

#define MINIMAP_X_SIZE		88
#define MINIMAP_Y_SIZE		44

#define WINDOW_SIZE		2

static float     gdXStep;
static float     gdYStep;
static SGPVSurface* giMiniMap;

// Utililty file for sub-sampling/creating our radar screen maps
// Loops though our maps directory and reads all .map files, subsamples an area, color
// quantizes it into an 8-bit image ans writes it to an sti file in radarmaps.


ScreenID MapUtilScreenHandle()
{
	static INT16 fNewMap = TRUE;
	InputAtom InputEvent;
	static FDLG_LIST *FListNode;
	static INT16 sFiles = 0, sCurFile = 0;
	static FDLG_LIST *FileList = NULL;

	INT16 sX1, sX2, sY1, sY2, sTop, sBottom, sLeft, sRight;


	FLOAT dX, dY, dStartX, dStartY;
	INT32 iX, iY, iSubX1, iSubY1, iSubX2, iSubY2, iWindowX, iWindowY;
	SGPPaletteEntry pPalette[ 256 ];

	FRAME_BUFFER->Fill(0x000000FF);

	if ( fNewMap )
	{
		fNewMap = FALSE;

		giMiniMap = AddVideoSurface(MINIMAP_X_SIZE, MINIMAP_Y_SIZE, PIXEL_DEPTH);

		// USING BRET's STUFF FOR LOOPING FILES/CREATING LIST, hence AddToFDlgList.....
		try
		{
			std::vector<std::string> files = GCM->getAllMaps();
			for (const std::string &file : files)
			{
				FileList = AddToFDlgList(FileList, file.c_str());
				++sFiles;
			}
		}
		catch (...) { /* XXX ignore */ }

		FListNode = FileList;
	}

	//OK, we are here, now loop through files
	if ( sCurFile == sFiles || FListNode== NULL )
	{
		requestGameExit();
		return( MAPUTILITY_SCREEN );
	}

	char zFilename[260];
	strcpy(zFilename, FListNode->filename);

	// OK, load maps and do overhead shrinkage of them...
	try         { LoadWorld(zFilename); }
	catch (...) { return ERROR_SCREEN;  }

	// Render small map
	InitNewOverheadDB(giCurrentTilesetID);

	gfOverheadMapDirty = TRUE;

	RenderOverheadMap(0, WORLD_COLS / 2, 0, 0, SCREEN_WIDTH, 320, TRUE);

	TrashOverheadMap( );

	// OK, NOW PROCESS OVERHEAD MAP ( SHOUIDL BE ON THE FRAMEBUFFER )
	gdXStep	= float(SCREEN_WIDTH) / MINIMAP_X_SIZE;
	gdYStep	= float(320) / MINIMAP_Y_SIZE;
	dStartX = dStartY = 0;

	// Adjust if we are using a restricted map...
	if ( gMapInformation.ubRestrictedScrollID != 0 )
	{
		CalculateRestrictedMapCoords(NORTH, &sX1,    &sY1,     &sX2,   &sTop, SCREEN_WIDTH, 320);
		CalculateRestrictedMapCoords(SOUTH, &sX1,    &sBottom, &sX2,   &sY2,  SCREEN_WIDTH, 320);
		CalculateRestrictedMapCoords(WEST,  &sX1,    &sY1,     &sLeft, &sY2,  SCREEN_WIDTH, 320);
		CalculateRestrictedMapCoords(EAST,  &sRight, &sY1,     &sX2,   &sY2,  SCREEN_WIDTH, 320);

		gdXStep	= float(sRight - sLeft) / float(MINIMAP_X_SIZE);
		gdYStep	= float(sBottom - sTop) / float(MINIMAP_Y_SIZE);

		dStartX = sLeft;
		dStartY = sTop;
	}

	//LOCK BUFFERS

	dX = dStartX;
	dY = dStartY;


	std::string zFilename2(GCM->getRadarMapResourceName(FileMan::replaceExtension(zFilename, ".sti")));
	{
		SGPVSurface::Lock ldst(giMiniMap);
		UINT32* const pDestBuf         = ldst.Buffer<UINT32>();
		UINT32  const uiDestPitchBYTES = ldst.Pitch();

		{
			SGPVSurface::Lock lsrc(FRAME_BUFFER);
			UINT32* const pSrcBuf          = lsrc.Buffer<UINT32>();
			UINT32  const uiSrcPitchBYTES  = lsrc.Pitch();

			for ( iX = 0; iX < MINIMAP_X_SIZE; iX++ )
			{
				dY = dStartY;

				for ( iY = 0; iY < MINIMAP_Y_SIZE; iY++ )
				{
					//OK, AVERAGE PIXELS
					iSubX1 = (INT32)dX - WINDOW_SIZE;

					iSubX2 = (INT32)dX + WINDOW_SIZE;

					iSubY1 = (INT32)dY - WINDOW_SIZE;

					iSubY2 = (INT32)dY + WINDOW_SIZE;

					INT32 iCount = 0;
					UINT32 bR = 0, bG = 0, bB = 0;
					UINT32 bAvR = 0, bAvG = 0, bAvB = 0;

					for ( iWindowX = iSubX1; iWindowX < iSubX2; iWindowX++ )
					{
						for ( iWindowY = iSubY1; iWindowY < iSubY2; iWindowY++ )
						{
							if (0 <= iWindowX && iWindowX < SCREEN_WIDTH &&
									0 <= iWindowY && iWindowY < 320)
							{
								UINT8 *rgb = reinterpret_cast<UINT8 *>(pSrcBuf)
										+ iWindowY * uiSrcPitchBYTES / 4
										+ iWindowX;

								bR += rgb[3];
								bG += rgb[2];
								bB += rgb[1];

								// Average!
								iCount++;
							}
						}

					}

					if ( iCount > 0 )
					{
						bAvR = bR / iCount;
						bAvG = bG / iCount;
						bAvB = bB / iCount;
					}

					//Write into dest!
					pDestBuf[iY * uiDestPitchBYTES / 4 + iX] = RGB(bAvR, bAvG, bAvB);

					//Increment
					dY += gdYStep;

				}

				//Increment
				dX += gdXStep;
			}
		}

		// FIXME: maxrd2 - need to use PNG for 32bpp data
		WriteSTIFile((UINT8 *)pDestBuf, pPalette, MINIMAP_X_SIZE, MINIMAP_Y_SIZE, zFilename2.c_str(), CONVERT_ETRLE_COMPRESS, 0);
	}

	SetFontAttributes(TINYFONT1, FONT_MCOLOR_DKGRAY);
	mprintf(10, 340, L"Writing radar image %hs", zFilename2.c_str());
	mprintf(10, 350, L"Using tileset %ls", gTilesets[giCurrentTilesetID].zName);

	InvalidateScreen( );

	while (DequeueEvent(&InputEvent))
	{
		if (InputEvent.usEvent == KEY_DOWN && InputEvent.usParam == SDLK_ESCAPE)
		{ // Exit the program
			requestGameExit();
		}
	}

	// Set next
	FListNode = FListNode->pNext;
	sCurFile++;

	return( MAPUTILITY_SCREEN );
}
