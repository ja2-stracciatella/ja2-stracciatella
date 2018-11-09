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
#include "Quantize.h"
#include "UILayout.h"

#include "ContentManager.h"
#include "GameInstance.h"

#define MINIMAP_X_SIZE		88
#define MINIMAP_Y_SIZE		44

#define WINDOW_SIZE		2

static float     gdXStep;
static float     gdYStep;
static SGPVSurface* giMiniMap;
static SGPVSurface* gi8BitMiniMap;

// Utililty file for sub-sampling/creating our radar screen maps
// Loops though our maps directory and reads all .map files, subsamples an area, color
// quantizes it into an 8-bit image ans writes it to an sti file in radarmaps.


ScreenID MapUtilScreenHandle()
{
	static SGPPaletteEntry* p24BitValues = NULL;
	static INT16 fNewMap = TRUE;
	InputAtom InputEvent;
	static FDLG_LIST *FListNode;
	static INT16 sFiles = 0, sCurFile = 0;
	static FDLG_LIST *FileList = NULL;

	UINT32 uiRGBColor;

	UINT32 bR, bG, bB, bAvR, bAvG, bAvB;
	UINT32 s16BPPSrc, sDest16BPPColor;

	INT16 sX1, sX2, sY1, sY2, sTop, sBottom, sLeft, sRight;


	FLOAT dX, dY, dStartX, dStartY;
	INT32 iX, iY, iSubX1, iSubY1, iSubX2, iSubY2, iWindowX, iWindowY, iCount;
	SGPPaletteEntry pPalette[ 256 ];


	sDest16BPPColor = -1;
	bAvR = bAvG = bAvB = 0;

	FRAME_BUFFER->Fill(0x000000FF);

	if ( fNewMap )
	{
		fNewMap = FALSE;

		giMiniMap = AddVideoSurface(88, 44, PIXEL_DEPTH);

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

		//Allocate 24 bit Surface
		p24BitValues = MALLOCN(SGPPaletteEntry, MINIMAP_X_SIZE * MINIMAP_Y_SIZE);

		//Allocate 8-bit surface
		gi8BitMiniMap = AddVideoSurface(88, 44, 8);
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

							// FIXME: maxrd2 - this had GetRGBColor
							uiRGBColor = s16BPPSrc;

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

					sDest16BPPColor = RGB(bAvR, bAvG, bAvB);
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


	char zFilename2[260];
	//QUantize!
	{ SGPVSurface::Lock lsrc(gi8BitMiniMap);
		UINT8* const pDataPtr = lsrc.Buffer<UINT8>();
		{ SGPVSurface::Lock ldst(FRAME_BUFFER);
			UINT32* const pDestBuf         = ldst.Buffer<UINT32>();
			UINT32  const uiDestPitchBYTES = ldst.Pitch();
			QuantizeImage(pDataPtr, p24BitValues, MINIMAP_X_SIZE, MINIMAP_Y_SIZE, pPalette);
			gi8BitMiniMap->SetPalette(pPalette);
			// Blit!
			Assert(FALSE);
//			Blt32BPPDataTo32BPPBuffer((UINT16*)pDestBuf, uiDestPitchBYTES, gi8BitMiniMap, pDataPtr, 300, 360); // maxrd2 FIXME? we have 8pp buffer here

			// Write palette!
			{
				INT32 cnt;
				INT32 sX = 0, sY = 420;
				UINT32 usLineColor;

				SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

				for ( cnt = 0; cnt < 256; cnt++ )
				{
					usLineColor = RGB(pPalette[cnt].r, pPalette[cnt].g, pPalette[cnt].b);
					RectangleDraw(TRUE, sX, sY, sX, sY + 10, usLineColor, pDestBuf);
					sX++;
					RectangleDraw(TRUE, sX, sY, sX, sY + 10, usLineColor, pDestBuf);
					sX++;
				}
			}
		}

		std::string zFilename2(GCM->getRadarMapResourceName(FileMan::replaceExtension(zFilename, ".sti")));
		WriteSTIFile( pDataPtr, pPalette, MINIMAP_X_SIZE, MINIMAP_Y_SIZE, zFilename2.c_str(), CONVERT_ETRLE_COMPRESS, 0 );
	}

	SetFontAttributes(TINYFONT1, FONT_MCOLOR_DKGRAY);
	mprintf(10, 340, L"Writing radar image %hs", zFilename2);
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
