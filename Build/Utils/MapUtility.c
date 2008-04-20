#ifdef JA2EDITOR

#include "Input.h"
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


#define		MINIMAP_X_SIZE			88
#define		MINIMAP_Y_SIZE			44

#define		WINDOW_SIZE					2

static float     gdXStep;
static float     gdYStep;
static SGPVSurface* giMiniMap;
static SGPVSurface* gi8BitMiniMap;

// Utililty file for sub-sampling/creating our radar screen maps
// Loops though our maps directory and reads all .map files, subsamples an area, color
// quantizes it into an 8-bit image ans writes it to an sti file in radarmaps.


UINT32	MapUtilScreenHandle( )
{
	static SGPPaletteEntry* p24BitValues = NULL;
	static INT16		fNewMap = TRUE;
  InputAtom  InputEvent;
	GETFILESTRUCT FileInfo;
	static FDLG_LIST *FListNode;
	static INT16 sFiles = 0, sCurFile = 0;
	static FDLG_LIST *FileList = NULL;
	UINT32					uiDestPitchBYTES, uiSrcPitchBYTES;
	UINT16						*pDestBuf, *pSrcBuf;
	UINT8						*pDataPtr;

	UINT32					uiRGBColor;

	UINT32					bR, bG, bB, bAvR, bAvG, bAvB;
	INT16						s16BPPSrc, sDest16BPPColor;
	INT32						cnt;

	INT16 sX1, sX2, sY1, sY2, sTop, sBottom, sLeft, sRight;


	FLOAT		dX, dY, dStartX, dStartY;
	INT32		iX, iY, iSubX1, iSubY1, iSubX2, iSubY2, iWindowX, iWindowY, iCount;
	SGPPaletteEntry pPalette[ 256 ];


	sDest16BPPColor = -1;
	bAvR = bAvG = bAvB = 0;

	FillSurface(FRAME_BUFFER, Get16BPPColor(FROMRGB(0, 0, 0)));

	if ( fNewMap )
	{
		fNewMap = FALSE;

		giMiniMap = AddVideoSurface(88, 44, PIXEL_DEPTH);
		if (giMiniMap == NO_VSURFACE) return ERROR_SCREEN;

		// USING BRET's STUFF FOR LOOPING FILES/CREATING LIST, hence AddToFDlgList.....
		if( GetFileFirst("MAPS/*.dat", &FileInfo) )
		{
			do
			{
				FileList = AddToFDlgList(FileList, FileInfo.zFileName);
				sFiles++;
			}
			while (GetFileNext(&FileInfo));
			GetFileClose(&FileInfo);
		}

		FListNode = FileList;

		//Allocate 24 bit Surface
		p24BitValues = MALLOCN(SGPPaletteEntry, MINIMAP_X_SIZE * MINIMAP_Y_SIZE);

		//Allocate 8-bit surface
		gi8BitMiniMap = AddVideoSurface(88, 44, 8);
		if (gi8BitMiniMap == NO_VSURFACE) return ERROR_SCREEN;
	}

	//OK, we are here, now loop through files
	if ( sCurFile == sFiles || FListNode== NULL )
	{
    gfProgramIsRunning = FALSE;
		return( MAPUTILITY_SCREEN );
	}

	char zFilename[260];
	strcpy(zFilename, FListNode->filename);

	// OK, load maps and do overhead shrinkage of them...
	if ( !LoadWorld( zFilename ) )
	{
		return( ERROR_SCREEN );
	}

	// Render small map
	InitNewOverheadDB( (UINT8)giCurrentTilesetID );

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


	pDestBuf = (UINT16*)LockVideoSurface(giMiniMap, &uiDestPitchBYTES);
	pSrcBuf = (UINT16*)LockVideoSurface(FRAME_BUFFER, &uiSrcPitchBYTES);

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
			dst->peRed   = bAvR;
			dst->peGreen = bAvG;
			dst->peBlue  = bAvB;

			//Increment
			dY += gdYStep;

		}

		//Increment
		dX += gdXStep;

	}

	UnLockVideoSurface(giMiniMap);
	UnLockVideoSurface(FRAME_BUFFER);

	// RENDER!
	BltVideoSurface(FRAME_BUFFER, giMiniMap, 20, 360, NULL);


	//QUantize!
	pDataPtr = (UINT8*)LockVideoSurface(gi8BitMiniMap, &uiSrcPitchBYTES);
	pDestBuf = (UINT16*)LockVideoSurface(FRAME_BUFFER, &uiDestPitchBYTES);
	QuantizeImage(pDataPtr, p24BitValues, MINIMAP_X_SIZE, MINIMAP_Y_SIZE, pPalette);
	SetVideoSurfacePalette(gi8BitMiniMap, pPalette);
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
			usLineColor = Get16BPPColor( FROMRGB( pPalette[ cnt ].peRed, pPalette[ cnt ].peGreen, pPalette[ cnt ].peBlue ) );
			RectangleDraw( TRUE, sX, sY, sX, (INT16)( sY+10 ), usLineColor, (UINT8*)pDestBuf );
			sX++;
			RectangleDraw( TRUE, sX, sY, sX, (INT16)( sY+10 ), usLineColor, (UINT8*)pDestBuf );
			sX++;
		}
	}

	UnLockVideoSurface(FRAME_BUFFER);

	// Remove extension
	for ( cnt = strlen( zFilename )-1; cnt >=0; cnt-- )
	{
		if ( zFilename[ cnt ] == '.' )
		{
			zFilename[ cnt ] = '\0';
		}
	}

	char zFilename2[260];
	sprintf( zFilename2, "RADARMAPS/%s.STI", zFilename );
	WriteSTIFile( pDataPtr, pPalette, MINIMAP_X_SIZE, MINIMAP_Y_SIZE, zFilename2, CONVERT_ETRLE_COMPRESS, 0 );

	UnLockVideoSurface(gi8BitMiniMap);

	SetFont( TINYFONT1 );
	SetFontBackground( FONT_MCOLOR_BLACK );
	SetFontForeground( FONT_MCOLOR_DKGRAY );
	mprintf( 10, 340, L"Writing radar image %ls", zFilename2 );

	mprintf(10, 350, L"Using tileset %ls", gTilesets[giCurrentTilesetID].zName);

	InvalidateScreen( );

  while (DequeueEvent(&InputEvent) == TRUE)
  {
		if (InputEvent.usEvent == KEY_DOWN && InputEvent.usParam == SDLK_ESCAPE)
      { // Exit the program
        gfProgramIsRunning = FALSE;
      }
	}

	// Set next
	FListNode = FListNode->pNext;
	sCurFile++;

	return( MAPUTILITY_SCREEN );
}

#endif
