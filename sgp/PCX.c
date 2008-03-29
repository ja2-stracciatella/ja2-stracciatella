#include "HImage.h"
#include "PCX.h"
#include "MemMan.h"
#include "FileMan.h"


#define PCX_NORMAL         1
#define PCX_RLE            2
#define PCX_256COLOR       4
#define PCX_TRANSPARENT    8
#define PCX_CLIPPED        16
#define PCX_REALIZEPALETTE 32
#define PCX_X_CLIPPING     64
#define PCX_Y_CLIPPING     128
#define PCX_NOTLOADED      256

#define PCX_ERROROPENING   1
#define PCX_INVALIDFORMAT  2
#define PCX_INVALIDLEN     4
#define PCX_OUTOFMEMORY    8


typedef struct PcxHeader
{
	UINT8  ubManufacturer;
	UINT8  ubVersion;
	UINT8  ubEncoding;
	UINT8  ubBitsPerPixel;
	UINT16 usLeft;
	UINT16 usTop;
	UINT16 usRight;
	UINT16 usBottom;
	UINT16 usHorRez;
	UINT16 usVerRez;
	UINT8  ubEgaPalette[48];
	UINT8  ubReserved;
	UINT8  ubColorPlanes;
	UINT16 usBytesPerLine;
	UINT16 usPaletteType;
	UINT8  ubFiller[58];
} PcxHeader;
CASSERT(sizeof(PcxHeader) == 128)


typedef struct PcxObject
{
	UINT8* pPcxBuffer;
	UINT8  ubPalette[768];
	UINT16 usWidth, usHeight;
	UINT32 uiBufferSize;
	UINT16 usPcxFlags;
} PcxObject;


static BOOLEAN SetPcxPalette(PcxObject* pCurrentPcxObject, HIMAGE hImage);
static BOOLEAN BlitPcxToBuffer(PcxObject* pCurrentPcxObject, UINT8* pBuffer, UINT16 usBufferWidth, UINT16 usBufferHeight, UINT16 usX, UINT16 usY, BOOLEAN fTransp);
static PcxObject* LoadPcx(const char* pFilename);


BOOLEAN LoadPCXFileToImage( HIMAGE hImage, UINT16 fContents )
{
	PcxObject *pPcxObject;

	// First Load a PCX Image
	pPcxObject = LoadPcx( hImage->ImageFile );

	if ( pPcxObject == NULL )
	{
		return( FALSE );
	}

	// Set some header information
	hImage->usWidth = pPcxObject->usWidth;
	hImage->usHeight = pPcxObject->usHeight;
	hImage->ubBitDepth = 8;
	hImage->fFlags = hImage->fFlags | fContents;

	// Read and allocate bitmap block if requested
	if ( fContents & IMAGE_BITMAPDATA )
	{
		// Allocate memory for buffer
		hImage->p8BPPData = MALLOCN(UINT8, hImage->usWidth * hImage->usHeight);

		if ( !BlitPcxToBuffer( pPcxObject, hImage->p8BPPData, hImage->usWidth, hImage->usHeight, 0, 0, FALSE ) )
		{
			MemFree( hImage->p8BPPData );
			return( FALSE );
		}
	}

	if ( fContents & IMAGE_PALETTE )
	{
		SetPcxPalette( pPcxObject, hImage );

		// Create 16 BPP palette if flags and BPP justify
		hImage->pui16BPPPalette = Create16BPPPalette( hImage->pPalette );

	}

	// Free and remove pcx object
	MemFree( pPcxObject->pPcxBuffer );
	MemFree( pPcxObject );

	return( TRUE );
}


static PcxObject* LoadPcx(const char* pFilename)
{
  PcxHeader  Header;
  HWFILE     hFileHandle;
  UINT32     uiFileSize;
  UINT8     *pPcxBuffer;

  // Open and read in the file
	hFileHandle = FileOpen(pFilename, FILE_ACCESS_READ);
  if (hFileHandle == 0)
  { // damn we failed to open the file
    return NULL;
  }

  uiFileSize = FileGetSize(hFileHandle);
  if (uiFileSize == 0)
  { // we failed to size up the file
    return NULL;
  }

	// Create enw pCX object
	PcxObject* const pCurrentPcxObject = MALLOC(PcxObject);

	if ( pCurrentPcxObject == NULL )
	{
		return( NULL );
	}

	pCurrentPcxObject->pPcxBuffer = MALLOCN(UINT8, uiFileSize - (sizeof(PcxHeader) + 768));

	if ( pCurrentPcxObject->pPcxBuffer == NULL )
	{
		return( NULL );
	}

  // Ok we now have a file handle, so let's read in the data
  FileRead(hFileHandle, &Header, sizeof(PcxHeader));
  if ((Header.ubManufacturer != 10)||(Header.ubEncoding != 1))
  { // We have an invalid pcx format
    // Delete the object
    MemFree( pCurrentPcxObject->pPcxBuffer );
		MemFree( pCurrentPcxObject );
    return( NULL );
  }

  if (Header.ubBitsPerPixel == 8)
  {
    pCurrentPcxObject->usPcxFlags = PCX_256COLOR;
  } else
  {
    pCurrentPcxObject->usPcxFlags  = 0;
  }

  pCurrentPcxObject->usWidth      = 1 + (Header.usRight - Header.usLeft);
  pCurrentPcxObject->usHeight     = 1 + (Header.usBottom - Header.usTop);
  pCurrentPcxObject->uiBufferSize = uiFileSize - 768 - sizeof(PcxHeader);

  // We are ready to read in the pcx buffer data. Therefore we must lock the buffer
  pPcxBuffer = pCurrentPcxObject->pPcxBuffer;

  FileRead(hFileHandle, pPcxBuffer, pCurrentPcxObject->uiBufferSize);

  // Read in the palette
  FileRead(hFileHandle, pCurrentPcxObject->ubPalette, 768);

	// Close file
	FileClose( hFileHandle );

  return pCurrentPcxObject;
}


static BOOLEAN BlitPcxToBuffer(PcxObject* pCurrentPcxObject, UINT8* pBuffer, UINT16 usBufferWidth, UINT16 usBufferHeight, UINT16 usX, UINT16 usY, BOOLEAN fTransp)
{
  UINT8     *pPcxBuffer;
  UINT8      ubRepCount;
  UINT16     usMaxX, usMaxY;
  UINT32     uiImageSize;
  UINT8      ubCurrentByte = 0;
  UINT8      ubMode;
  UINT16     usCurrentX, usCurrentY;
  UINT32     uiOffset, uiIndex;
  UINT32     uiNextLineOffset, uiStartOffset, uiCurrentOffset;

  pPcxBuffer = pCurrentPcxObject->pPcxBuffer;

  if (((pCurrentPcxObject->usWidth + usX) == usBufferWidth)&&((pCurrentPcxObject->usHeight + usY)== usBufferHeight))
  { // Pre-compute PCX blitting aspects.
    uiImageSize = usBufferWidth * usBufferHeight;
    ubMode      = PCX_NORMAL;
    uiOffset    = 0;
    ubRepCount  = 0;

    // Blit Pcx object. Two main cases, one for transparency (0's are skipped and for without transparency.
    if (fTransp == TRUE)
    {
      for (uiIndex = 0; uiIndex < uiImageSize; uiIndex++)
      {
        if (ubMode == PCX_NORMAL)
        {
          ubCurrentByte = *(pPcxBuffer + uiOffset++);
          if (ubCurrentByte > 0x0BF)
          {
            ubRepCount = ubCurrentByte & 0x03F;
            ubCurrentByte = *(pPcxBuffer + uiOffset++);
            if (--ubRepCount > 0)
            {
              ubMode = PCX_RLE;
            }
          }
        }
        else
        {
          if (--ubRepCount == 0)
          {
            ubMode = PCX_NORMAL;
          }
        }
        if (ubCurrentByte != 0)
        {
          *(pBuffer + uiIndex) = ubCurrentByte;
        }
      }
    }
    else
    {
      for (uiIndex = 0; uiIndex < uiImageSize; uiIndex++)
      {
        if (ubMode == PCX_NORMAL)
        {
          ubCurrentByte = *(pPcxBuffer + uiOffset++);
          if (ubCurrentByte > 0x0BF)
          {
            ubRepCount = ubCurrentByte & 0x03F;
            ubCurrentByte = *(pPcxBuffer + uiOffset++);
            if (--ubRepCount > 0)
            {
              ubMode = PCX_RLE;
            }
          }
        }
        else
        {
          if (--ubRepCount == 0)
          { ubMode = PCX_NORMAL;
          }
        }
        *(pBuffer + uiIndex) = ubCurrentByte;
      }
    }
  } else
  { // Pre-compute PCX blitting aspects.
    if ((pCurrentPcxObject->usWidth + usX) >= usBufferWidth)
    {
      pCurrentPcxObject->usPcxFlags |= PCX_X_CLIPPING;
      usMaxX = usBufferWidth - 1;
    }
    else
    {
      usMaxX = pCurrentPcxObject->usWidth + usX;
    }

    if ((pCurrentPcxObject->usHeight + usY) >= usBufferHeight)
    {
      pCurrentPcxObject->usPcxFlags |= PCX_Y_CLIPPING;
      uiImageSize = pCurrentPcxObject->usWidth * (usBufferHeight - usY);
      usMaxY = usBufferHeight - 1;
    }
    else
    { uiImageSize = pCurrentPcxObject->usWidth * pCurrentPcxObject->usHeight;
      usMaxY = pCurrentPcxObject->usHeight + usY;
    }

    ubMode     = PCX_NORMAL;
    uiOffset   = 0;
    ubRepCount = 0;
    usCurrentX = usX;
    usCurrentY = usY;

    // Blit Pcx object. Two main cases, one for transparency (0's are skipped and for without transparency.
    if (fTransp == TRUE)
    {
      for (uiIndex = 0; uiIndex < uiImageSize; uiIndex++)
      {
        if (ubMode == PCX_NORMAL)
        {
          ubCurrentByte = *(pPcxBuffer + uiOffset++);
          if (ubCurrentByte > 0x0BF)
          {
            ubRepCount = ubCurrentByte & 0x03F;
            ubCurrentByte = *(pPcxBuffer + uiOffset++);
            if (--ubRepCount > 0)
            {
              ubMode = PCX_RLE;
            }
          }
        }
        else
        {
          if (--ubRepCount == 0)
          { ubMode = PCX_NORMAL;
          }
        }
        if (ubCurrentByte != 0)
        { *(pBuffer + (usCurrentY*usBufferWidth) + usCurrentX) = ubCurrentByte;
        }
        usCurrentX++;
        if (usCurrentX > usMaxX)
        {
          usCurrentX = usX;
          usCurrentY++;
        }
      }
    } else
    {
      uiStartOffset = (usCurrentY*usBufferWidth) + usCurrentX;
      uiNextLineOffset = uiStartOffset + usBufferWidth;
      uiCurrentOffset = uiStartOffset;

      for (uiIndex = 0; uiIndex < uiImageSize; uiIndex++)
      {

        if (ubMode == PCX_NORMAL)
        {
          ubCurrentByte = *(pPcxBuffer + uiOffset++);
          if (ubCurrentByte > 0x0BF)
          {
            ubRepCount = ubCurrentByte & 0x03F;
            ubCurrentByte = *(pPcxBuffer + uiOffset++);
            if (--ubRepCount > 0)
            {
              ubMode = PCX_RLE;
            }
          }
        }
        else
        {
          if (--ubRepCount == 0)
          {
            ubMode = PCX_NORMAL;
          }
        }

        if (usCurrentX < usMaxX)
        { // We are within the visible bounds so we write the byte to buffer
          *(pBuffer + uiCurrentOffset) = ubCurrentByte;
          uiCurrentOffset++;
          usCurrentX++;
        }
        else
        { if ((uiCurrentOffset + 1)< uiNextLineOffset)
          { // Increment the uiCurrentOffset
            uiCurrentOffset++;
          }
          else
          { // Go to next line
            usCurrentX = usX;
            usCurrentY++;
            if (usCurrentY > usMaxY)
            {
              break;
            }
            uiStartOffset = (usCurrentY*usBufferWidth) + usCurrentX;
            uiNextLineOffset = uiStartOffset + usBufferWidth;
            uiCurrentOffset = uiStartOffset;
          }
        }
      }
    }
  }

	return( TRUE );
}


static BOOLEAN SetPcxPalette(PcxObject* pCurrentPcxObject, HIMAGE hImage)
{
	UINT16 Index;
	UINT8  *pubPalette;

	pubPalette = &(pCurrentPcxObject->ubPalette[0]);

	// Allocate memory for palette
	hImage->pPalette = MALLOCN(SGPPaletteEntry, 256);

	if ( hImage->pPalette == NULL )
	{
		return( FALSE );
	}

  // Initialize the proper palette entries
  for (Index = 0; Index < 256; Index++)
  {
		hImage->pPalette[ Index ].peRed   = *(pubPalette+(Index*3));
    hImage->pPalette[ Index ].peGreen = *(pubPalette+(Index*3)+1);
    hImage->pPalette[ Index ].peBlue  = *(pubPalette+(Index*3)+2);
    hImage->pPalette[ Index ].peFlags = 0;
  }

  return TRUE;
}
