#include "MemMan.h"
#include "FileMan.h"
#include "ImgFmt.h"
#include "HImage.h"
#include "Types.h"
#include "Debug.h"
#include "WCheck.h"


static BOOLEAN STCILoadRGB(HIMAGE hImage, UINT16 fContents, HWFILE hFile, const STCIHeader* pHeader);
static BOOLEAN STCILoadIndexed( HIMAGE hImage, UINT16 fContents, HWFILE hFile, const STCIHeader* pHeader);


BOOLEAN LoadSTCIFileToImage( HIMAGE hImage, UINT16 fContents )
{
	HWFILE			hFile;
	STCIHeader	Header;
	image_type	TempImage;

	// Check that hImage is valid, and that the file in question exists
	Assert( hImage != NULL );

	TempImage = *hImage;

	CHECKF( FileExists( TempImage.ImageFile ) );

	// Open the file and read the header
	hFile = FileOpen(TempImage.ImageFile, FILE_ACCESS_READ);
	CHECKF( hFile );

	if (!FileRead(hFile, &Header, STCI_HEADER_SIZE) || memcmp(Header.cID, STCI_ID_STRING, STCI_ID_LEN) != 0)
	{
		DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Problem reading STCI header." );
		FileClose( hFile );
		return( FALSE );
	}

	// Determine from the header the data stored in the file. and run the appropriate loader
	if (Header.fFlags & STCI_RGB)
	{
		if( !STCILoadRGB( &TempImage, fContents, hFile, &Header ) )
		{
			DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Problem loading RGB image." );
			FileClose( hFile );
			return( FALSE );
		}
	}
	else if (Header.fFlags & STCI_INDEXED)
	{
		if( !STCILoadIndexed( &TempImage, fContents, hFile, &Header ) )
		{
			DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Problem loading palettized image." );
			FileClose( hFile );
			return( FALSE );
		}
	}
	else
	{	// unsupported type of data, or the right flags weren't set!
		DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Unknown data organization in STCI file." );
		FileClose( hFile );
		return( FALSE );
	}

	// Requested data loaded successfully.
	FileClose( hFile );

	// Set some more flags in the temporary image structure, copy it so that hImage points
	// to it, and return.
	if (Header.fFlags & STCI_ZLIB_COMPRESSED)
	{
		TempImage.fFlags |= IMAGE_COMPRESSED;
	}
	TempImage.usWidth = Header.usWidth;
	TempImage.usHeight = Header.usHeight;
	TempImage.ubBitDepth = Header.ubDepth;
	*hImage = TempImage;

	return( TRUE );
}


static BOOLEAN STCILoadRGB(HIMAGE hImage, UINT16 fContents, HWFILE hFile, const STCIHeader* pHeader)
{
	if (fContents & IMAGE_PALETTE && !(fContents & IMAGE_ALLIMAGEDATA))
	{ // RGB doesn't have a palette!
		return( FALSE );
	}

	if (fContents & IMAGE_BITMAPDATA)
	{
		// Allocate memory for the image data and read it in
		hImage->pImageData =	MemAlloc( pHeader->uiStoredSize );
		if (hImage->pImageData == NULL)
		{
			return( FALSE );
		}
		else if (!FileRead(hFile, hImage->pImageData, pHeader->uiStoredSize))
		{
			MemFree( hImage->pImageData );
			return( FALSE );
		}

		hImage->fFlags |= IMAGE_BITMAPDATA;

		if( pHeader->ubDepth == 16)
		{
			// ASSUMPTION: file data is 565 R,G,B

			if (gusRedMask != (UINT16) pHeader->RGB.uiRedMask || gusGreenMask != (UINT16) pHeader->RGB.uiGreenMask || gusBlueMask != (UINT16) pHeader->RGB.uiBlueMask )
			{
				// colour distribution of the file is different from hardware!  We have to change it!
				DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Converting to current RGB distribution!" );
				// Convert the image to the current hardware's specifications
				if (gusRedMask > gusGreenMask && gusGreenMask > gusBlueMask)
				{
					// hardware wants RGB!
					if (gusRedMask == 0x7C00 && gusGreenMask == 0x03E0 && gusBlueMask == 0x001F)
					{	// hardware is 555
						ConvertRGBDistribution565To555( hImage->p16BPPData, pHeader->usWidth * pHeader->usHeight );
						return( TRUE );
					}
					else if (gusRedMask == 0xFC00 && gusGreenMask == 0x03E0 && gusBlueMask == 0x001F)
					{
						ConvertRGBDistribution565To655( hImage->p16BPPData, pHeader->usWidth * pHeader->usHeight );
						return( TRUE );
					}
					else if (gusRedMask == 0xF800 && gusGreenMask == 0x07C0 && gusBlueMask == 0x003F)
					{
						ConvertRGBDistribution565To556( hImage->p16BPPData, pHeader->usWidth * pHeader->usHeight );
						return( TRUE );
					}
					else
					{
						// take the long route
						ConvertRGBDistribution565ToAny( hImage->p16BPPData, pHeader->usWidth * pHeader->usHeight );
						return( TRUE );
					}
				}
				else
				{
					// hardware distribution is not R-G-B so we have to take the long route!
					ConvertRGBDistribution565ToAny( hImage->p16BPPData, pHeader->usWidth * pHeader->usHeight );
					return( TRUE );
				}
			}
		}
	}
#ifdef JA2
	return( TRUE );
#else
// Anything else is an ERROR! --DB
	return(FALSE);
#endif
}


static BOOLEAN STCISetPalette(const STCIPaletteElement* pSTCIPalette, HIMAGE hImage);


static BOOLEAN STCILoadIndexed( HIMAGE hImage, UINT16 fContents, HWFILE hFile, const STCIHeader* pHeader)
{
	UINT32			uiFileSectionSize;

	if (fContents & IMAGE_PALETTE)
	{ // Allocate memory for reading in the palette
		if (pHeader->Indexed.uiNumberOfColours != 256)
		{
			DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Palettized image has bad palette size." );
			return( FALSE );
		}
		uiFileSectionSize = pHeader->Indexed.uiNumberOfColours * STCI_PALETTE_ELEMENT_SIZE;
		STCIPaletteElement* pSTCIPalette = MemAlloc(uiFileSectionSize);
		if (pSTCIPalette == NULL)
		{
			DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Out of memory!" );
			return( FALSE );
		}

		// Read in the palette
		if (!FileRead(hFile, pSTCIPalette, uiFileSectionSize))
		{
			DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Problem loading palette!" );
			MemFree( pSTCIPalette );
			return( FALSE );
		}
		else if (!STCISetPalette( pSTCIPalette, hImage ))
		{
			DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Problem setting hImage-format palette!" );
			MemFree( pSTCIPalette );
			return( FALSE );
		}
		hImage->fFlags |= IMAGE_PALETTE;
		// Free the temporary buffer
		MemFree( pSTCIPalette );
	}
	else if (fContents & (IMAGE_BITMAPDATA | IMAGE_APPDATA))
	{ // seek past the palette
		uiFileSectionSize = pHeader->Indexed.uiNumberOfColours * STCI_PALETTE_ELEMENT_SIZE;
		if (FileSeek( hFile, uiFileSectionSize, FILE_SEEK_FROM_CURRENT) == FALSE)
		{
			DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Problem seeking past palette!" );
			return( FALSE );
		}
	}
	if (fContents & IMAGE_BITMAPDATA)
	{
		if (pHeader->fFlags & STCI_ETRLE_COMPRESSED)
		{
			// load data for the subimage (object) structures
			Assert( sizeof( ETRLEObject ) == STCI_SUBIMAGE_SIZE );
			hImage->usNumberOfObjects = pHeader->Indexed.usNumberOfSubImages;
			uiFileSectionSize = hImage->usNumberOfObjects * STCI_SUBIMAGE_SIZE;
			hImage->pETRLEObject = MemAlloc( uiFileSectionSize );
			if (hImage->pETRLEObject == NULL)
			{
				DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Out of memory!" );
				if (fContents & IMAGE_PALETTE)
				{
					MemFree( hImage->pPalette );
				}
				return( FALSE );
			}
			if (!FileRead(hFile, hImage->pETRLEObject, uiFileSectionSize))
			{
				DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Error loading subimage structures!" );
				if (fContents & IMAGE_PALETTE)
				{
					MemFree( hImage->pPalette );
				}
				MemFree( hImage->pETRLEObject );
				return( FALSE );
			}
			hImage->uiSizePixData = pHeader->uiStoredSize;
			hImage->fFlags |= IMAGE_TRLECOMPRESSED;
		}
		// allocate memory for and read in the image data
		hImage->pImageData = MemAlloc( pHeader->uiStoredSize );
		if (hImage->pImageData == NULL)
		{
			DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Out of memory!" );
			if (fContents & IMAGE_PALETTE)
			{
				MemFree( hImage->pPalette );
			}
			if (hImage->usNumberOfObjects > 0)
			{
				MemFree( hImage->pETRLEObject );
			}
			return( FALSE );
		}
		else if (!FileRead(hFile, hImage->pImageData, pHeader->uiStoredSize))
		{ // Problem reading in the image data!
			DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Error loading image data!" );
			MemFree( hImage->pImageData );
			if (fContents & IMAGE_PALETTE)
			{
				MemFree( hImage->pPalette );
			}
			if (hImage->usNumberOfObjects > 0)
			{
				MemFree( hImage->pETRLEObject );
			}
			return( FALSE );
		}
		hImage->fFlags |= IMAGE_BITMAPDATA;
	}
	else if (fContents & IMAGE_APPDATA) // then there's a point in seeking ahead
	{
		if (FileSeek( hFile, pHeader->uiStoredSize, FILE_SEEK_FROM_CURRENT) == FALSE)
		{
			DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Problem seeking past image data!" );
			return( FALSE );
		}
	}

	if (fContents & IMAGE_APPDATA && pHeader->uiAppDataSize > 0)
	{
		// load application-specific data
		hImage->pAppData = MemAlloc( pHeader->uiAppDataSize );
		if (hImage->pAppData == NULL)
		{
			DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Out of memory!" );
			MemFree( hImage->pAppData );
			if (fContents & IMAGE_PALETTE)
			{
				MemFree( hImage->pPalette );
			}
			if (fContents & IMAGE_BITMAPDATA)
			{
				MemFree( hImage->pImageData );
			}
			if (hImage->usNumberOfObjects > 0)
			{
				MemFree( hImage->pETRLEObject );
			}
			return( FALSE );
		}
		if (!FileRead(hFile, hImage->pAppData, pHeader->uiAppDataSize))
		{
			DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Error loading application-specific data!" );
			MemFree( hImage->pAppData );
			if (fContents & IMAGE_PALETTE)
			{
				MemFree( hImage->pPalette );
			}
			if (fContents & IMAGE_BITMAPDATA)
			{
				MemFree( hImage->pImageData );
			}
			if (hImage->usNumberOfObjects > 0)
			{
				MemFree( hImage->pETRLEObject );
			}
			return( FALSE );
		}
		hImage->uiAppDataSize = pHeader->uiAppDataSize;;
		hImage->fFlags |= IMAGE_APPDATA;
	}
	else
	{
		hImage->pAppData = NULL;
		hImage->uiAppDataSize = 0;
	}
	return( TRUE );
}


static BOOLEAN STCISetPalette(const STCIPaletteElement* pSTCIPalette, HIMAGE hImage)
{
	// Allocate memory for palette
	hImage->pPalette = MemAlloc( sizeof( SGPPaletteEntry ) * 256 );

	if ( hImage->pPalette == NULL )
	{
		return( FALSE );
	}

  // Initialize the proper palette entries
  for (UINT i = 0; i < 256; i++)
  {
		hImage->pPalette[i].peRed   = pSTCIPalette[i].ubRed;
		hImage->pPalette[i].peGreen = pSTCIPalette[i].ubGreen;
		hImage->pPalette[i].peBlue  = pSTCIPalette[i].ubBlue;
		hImage->pPalette[i].peFlags = 0;
  }
  return TRUE;
}
