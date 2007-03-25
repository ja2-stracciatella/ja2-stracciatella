#include <stdio.h>
#include "Debug.h"
#include "HImage.h"
#include "VObject.h"
#include "WCheck.h"
#include "VObject_Blitters.h"
#include "SGP.h"
#include <string.h>
#include "MemMan.h"


// ******************************************************************************
//
// Video Object SGP Module
//
// Video Objects are used to contain any imagery which requires blitting. The data
// is contained within a Direct Draw surface. Palette information is in both
// a Direct Draw Palette and a 16BPP palette structure for 8->16 BPP Blits.
// Blitting is done via Direct Draw as well as custum blitters. Regions are
// used to define local coordinates within the surface
//
// Second Revision: Dec 10, 1996, Andrew Emmons
//
// *******************************************************************************


#define COMPRESS_TRANSPARENT				0x80
#define COMPRESS_RUN_MASK						0x7F


typedef struct VOBJECT_NODE
{
	HVOBJECT hVObject;
	UINT32 uiIndex;
  struct VOBJECT_NODE *next, *prev;

	#ifdef SGP_VIDEO_DEBUGGING
		UINT8									*pName;
		UINT8									*pCode;
	#endif

}VOBJECT_NODE;

static VOBJECT_NODE* gpVObjectHead = NULL;
static VOBJECT_NODE* gpVObjectTail = NULL;
static UINT32 guiVObjectIndex = 1;
UINT32				guiVObjectSize = 0;

#ifdef _DEBUG
enum
{
	DEBUGSTR_NONE,
	DEBUGSTR_BLTVIDEOOBJECTFROMINDEX,
	DEBUGSTR_SETOBJECTHANDLESHADE,
	DEBUGSTR_GETVIDEOOBJECTETRLESUBREGIONPROPERTIES,
	DEBUGSTR_GETVIDEOOBJECTETRLEPROPERTIESFROMINDEX,
	DEBUGSTR_BLTVIDEOOBJECTOUTLINEFROMINDEX,
	DEBUGSTR_BLTVIDEOOBJECTOUTLINESHADOWFROMINDEX,
	DEBUGSTR_DELETEVIDEOOBJECTFROMINDEX
};

static UINT8 gubVODebugCode = 0;

static void CheckValidVObjectIndex(UINT32 uiIndex);
#endif

// **************************************************************
//
// Video Object Manager functions
//
// **************************************************************


BOOLEAN InitializeVideoObjectManager( )
{
	//Shouldn't be calling this if the video object manager already exists.
	//Call shutdown first...
	Assert( !gpVObjectHead );
	Assert( !gpVObjectTail );
	RegisterDebugTopic(TOPIC_VIDEOOBJECT, "Video Object Manager");
	gpVObjectHead = gpVObjectTail = NULL;
	return TRUE ;
}

BOOLEAN ShutdownVideoObjectManager( )
{
	VOBJECT_NODE *curr;
	while( gpVObjectHead )
	{
		curr = gpVObjectHead;
		gpVObjectHead = gpVObjectHead->next;
		DeleteVideoObject( curr->hVObject );
		#ifdef SGP_VIDEO_DEBUGGING
			if( curr->pName )
				MemFree( curr->pName );
			if( curr->pCode )
				MemFree( curr->pCode );
		#endif
		MemFree( curr );
	}
	gpVObjectHead = NULL;
	gpVObjectTail = NULL;
	guiVObjectIndex = 1;
	guiVObjectSize = 0;
	UnRegisterDebugTopic(TOPIC_VIDEOOBJECT, "Video Objects");
	return TRUE;
}


#ifdef JA2TESTVERSION
static UINT32 CountVideoObjectNodes(void)
{
	VOBJECT_NODE *curr;
	UINT32 i = 0;
	curr = gpVObjectHead;
	while( curr )
	{
		i++;
		curr = curr->next;
	}
	return i;
}
#endif


static BOOLEAN AddStandardVideoObject(HVOBJECT hVObject, UINT32* puiIndex)
{
	// Assertions
	Assert( puiIndex );

	if( !hVObject )
	{
		// Video Object will set error condition.
		return FALSE ;
	}

	// Set into video object list
	if( gpVObjectHead )
	{ //Add node after tail
		gpVObjectTail->next = (VOBJECT_NODE*)MemAlloc( sizeof( VOBJECT_NODE ) );
		Assert( gpVObjectTail->next ); //out of memory?
		gpVObjectTail->next->prev = gpVObjectTail;
		gpVObjectTail->next->next = NULL;
		gpVObjectTail = gpVObjectTail->next;
	}
	else
	{ //new list
		gpVObjectHead = (VOBJECT_NODE*)MemAlloc( sizeof( VOBJECT_NODE ) );
		Assert( gpVObjectHead ); //out of memory?
		gpVObjectHead->prev = gpVObjectHead->next = NULL;
		gpVObjectTail = gpVObjectHead;
	}
	#ifdef SGP_VIDEO_DEBUGGING
		gpVObjectTail->pName = NULL;
		gpVObjectTail->pCode = NULL;
	#endif
	//Set the hVObject into the node.
	gpVObjectTail->hVObject = hVObject;
	gpVObjectTail->uiIndex = guiVObjectIndex+=2;
	*puiIndex = gpVObjectTail->uiIndex;
	Assert( guiVObjectIndex < 0xfffffff0 ); //unlikely that we will ever use 2 billion vobjects!
	//We would have to create about 70 vobjects per second for 1 year straight to achieve this...
	guiVObjectSize++;

	#ifdef JA2TESTVERSION
		if( CountVideoObjectNodes() != guiVObjectSize )
		{
			guiVObjectSize = guiVObjectSize;
		}
	#endif

	return TRUE ;
}


BOOLEAN AddStandardVideoObjectFromHImage(HIMAGE hImage, UINT32* uiIndex)
{
	return AddStandardVideoObject(CreateVideoObject(hImage), uiIndex);
}


BOOLEAN AddStandardVideoObjectFromFile(const char* ImageFile, UINT32* uiIndex)
{
	return AddStandardVideoObject(CreateVideoObjectFromFile(ImageFile), uiIndex);
}


HVOBJECT GetVideoObject(UINT32 uiIndex)
{
	VOBJECT_NODE *curr;

	#ifdef _DEBUG
		CheckValidVObjectIndex( uiIndex );
	#endif

	curr = gpVObjectHead;
	while( curr )
	{
		if (curr->uiIndex == uiIndex) return curr->hVObject;
		curr = curr->next;
	}
	return NULL;
}


BOOLEAN BltVideoObjectFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject, UINT16 usRegionIndex, INT32 iDestX, INT32 iDestY)
{
#ifdef _DEBUG
	gubVODebugCode = DEBUGSTR_BLTVIDEOOBJECTFROMINDEX;
#endif
	HVOBJECT hSrcVObject = GetVideoObject(uiSrcVObject);
	if (hSrcVObject == NULL) return FALSE;
	return BltVideoObject(uiDestVSurface, hSrcVObject, usRegionIndex, iDestX, iDestY);
}


BOOLEAN DeleteVideoObjectFromIndex( UINT32 uiVObject  )
{
	VOBJECT_NODE *curr;

	#ifdef _DEBUG
		gubVODebugCode = DEBUGSTR_DELETEVIDEOOBJECTFROMINDEX;
		CheckValidVObjectIndex( uiVObject );
	#endif

	curr = gpVObjectHead;
	while( curr )
	{
		if( curr->uiIndex == uiVObject )
		{ //Found the node, so detach it and delete it.

			//Deallocate the memory for the video object
			DeleteVideoObject( curr->hVObject );

			if( curr == gpVObjectHead )
			{ //Advance the head, because we are going to remove the head node.
				gpVObjectHead = gpVObjectHead->next;
			}
			if( curr == gpVObjectTail )
			{ //Back up the tail, because we are going to remove the tail node.
				gpVObjectTail = gpVObjectTail->prev;
			}
			//Detach the node from the vobject list
			if( curr->next )
			{ //Make the prev node point to the next
				curr->next->prev = curr->prev;
			}
			if( curr->prev )
			{ //Make the next node point to the prev
				curr->prev->next = curr->next;
			}
			//The node is now detached.  Now deallocate it.
			#ifdef SGP_VIDEO_DEBUGGING
				if( curr->pName )
					MemFree( curr->pName );
				if( curr->pCode )
					MemFree( curr->pCode );
			#endif
			MemFree( curr );
			curr = NULL;
			guiVObjectSize--;
			#ifdef JA2TESTVERSION
				if( CountVideoObjectNodes() != guiVObjectSize )
				{
					guiVObjectSize = guiVObjectSize;
				}
			#endif
			return TRUE;
		}
		curr = curr->next;
	}
	return FALSE;
}


static BOOLEAN BltVideoObjectToBuffer(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY);


// Given an index to the dest and src vobject contained in ghVideoObjects
// Based on flags, blit accordingly
// There are two types, a BltFast and a Blt. BltFast is 10% faster, uses no
// clipping lists
BOOLEAN BltVideoObject(UINT32 uiDestVSurface, HVOBJECT hSrcVObject, UINT16 usRegionIndex, INT32 iDestX, INT32 iDestY)
{

	UINT16								*pBuffer;
	UINT32								uiPitch;

	// Lock video surface
	pBuffer = (UINT16*)LockVideoSurface( uiDestVSurface, &uiPitch );

	if ( pBuffer == NULL )
	{
		return( FALSE );
	}

	// Now we have the video object and surface, call the VO blitter function
	if (!BltVideoObjectToBuffer(pBuffer, uiPitch, hSrcVObject, usRegionIndex, iDestX, iDestY))
	{
		UnLockVideoSurface( uiDestVSurface );
		// VO Blitter will set debug messages for error conditions
		return( FALSE );
	}

	UnLockVideoSurface( uiDestVSurface );
	return( TRUE );
}

// *******************************************************************************
// Video Object Manipulation Functions
// *******************************************************************************


static BOOLEAN SetVideoObjectPalette(HVOBJECT hVObject, SGPPaletteEntry* pSrcPalette);


HVOBJECT CreateVideoObject(HIMAGE hImage)
{
	if (hImage == NULL)
	{
		DbgMessage(TOPIC_VIDEOOBJECT, DBG_LEVEL_2, "Invalid hImage pointer given");
		return NULL;
	}

	HVOBJECT hVObject = MemAlloc(sizeof(*hVObject));
	CHECKF(hVObject != NULL);
	memset(hVObject, 0, sizeof(*hVObject));

	// Check if himage is TRLE compressed - return error if not
	if (!(hImage->fFlags & IMAGE_TRLECOMPRESSED))
	{
		MemFree(hVObject);
		DbgMessage(TOPIC_VIDEOOBJECT, DBG_LEVEL_2, "Invalid Image format given.");
		DestroyImage(hImage);
		return NULL;
	}

	ETRLEData TempETRLEData;
	CHECKF(GetETRLEImageData(hImage, &TempETRLEData));

	hVObject->usNumberOfObjects = TempETRLEData.usNumberOfObjects;
	hVObject->pETRLEObject      = TempETRLEData.pETRLEObject;
	hVObject->pPixData          = TempETRLEData.pPixData;
	hVObject->uiSizePixData     = TempETRLEData.uiSizePixData;
	hVObject->ubBitDepth        = hImage->ubBitDepth;

	if (hImage->ubBitDepth == 8)
	{
		SetVideoObjectPalette(hVObject, hImage->pPalette);
	}

	return hVObject;
}


HVOBJECT CreateVideoObjectFromFile(const char* Filename)
{
	HIMAGE hImage = CreateImage(Filename, IMAGE_ALLIMAGEDATA);
	if (hImage == NULL)
	{
		DbgMessage(TOPIC_VIDEOOBJECT, DBG_LEVEL_2, String("Invalid Image Filename '%s' given", Filename));
		return NULL;
	}

	HVOBJECT vObject = CreateVideoObject(hImage);

	DestroyImage(hImage);
	return vObject;
}


// Palette setting is expensive, need to set both DDPalette and create 16BPP palette
static BOOLEAN SetVideoObjectPalette(HVOBJECT hVObject, SGPPaletteEntry* pSrcPalette)
{

	Assert( hVObject != NULL );
	Assert( pSrcPalette != NULL );

	// Create palette object if not already done so
	if ( hVObject->pPaletteEntry == NULL )
	{
		// Create palette
		hVObject->pPaletteEntry = MemAlloc( sizeof( SGPPaletteEntry ) * 256 );
		CHECKF( hVObject->pPaletteEntry != NULL );

		// Copy src into palette
		memcpy( hVObject->pPaletteEntry, pSrcPalette, sizeof( SGPPaletteEntry ) * 256 );

	}
	else
	{
		// Just Change entries
		memcpy( hVObject->pPaletteEntry, pSrcPalette, sizeof( SGPPaletteEntry ) * 256 );
	}

	// Delete 16BPP Palette if one exists
	if ( hVObject->p16BPPPalette != NULL )
	{
		MemFree( hVObject->p16BPPPalette );
		hVObject->p16BPPPalette = NULL;
	}

	// Create 16BPP Palette
	hVObject->p16BPPPalette = Create16BPPPalette( pSrcPalette );
	hVObject->pShadeCurrent = hVObject->p16BPPPalette;

//	DbgMessage(TOPIC_VIDEOOBJECT, DBG_LEVEL_3, "Video Object Palette change successfull");
	return( TRUE );
}


// Deletes all palettes, surfaces and region data
BOOLEAN DeleteVideoObject( HVOBJECT hVObject )
{
	UINT16			usLoop;

	// Assertions
	CHECKF( hVObject != NULL );

	DestroyObjectPaletteTables(hVObject);

	// Release palette
	if ( hVObject->pPaletteEntry != NULL )
	{
		MemFree( hVObject->pPaletteEntry );
//		hVObject->pPaletteEntry = NULL;
	}


	if ( hVObject->pPixData != NULL )
	{
		MemFree( hVObject->pPixData );
//		hVObject->pPixData = NULL;
	}

	if ( hVObject->pETRLEObject != NULL )
	{
		MemFree( hVObject->pETRLEObject );
//		hVObject->pETRLEObject = NULL;
	}

	if ( hVObject->ppZStripInfo != NULL )
	{
		for (usLoop = 0; usLoop < hVObject->usNumberOfObjects; usLoop++)
		{
			if (hVObject->ppZStripInfo[usLoop] != NULL)
			{
				MemFree( hVObject->ppZStripInfo[usLoop]->pbZChange );
				MemFree( hVObject->ppZStripInfo[usLoop] );
			}
		}
		MemFree( hVObject->ppZStripInfo );
//		hVObject->ppZStripInfo = NULL;
	}

	// Release object
	MemFree( hVObject );

	return( TRUE );
}


// *******************************************************************
//
// Blitting Functions
//
// *******************************************************************

// High level blit function encapsolates ALL effects and BPP
static BOOLEAN BltVideoObjectToBuffer(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY)
{

	// Assertions
	Assert( pBuffer != NULL );

	if ( hSrcVObject == NULL )
	{
		int i=0;
	}

	Assert( hSrcVObject != NULL );

	// Check For Flags and bit depths
	switch( hSrcVObject->ubBitDepth )
	{
			case 16:


				break;

			case 8:
				// Switch based on flags given
				if (BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect))
					Blt8BPPDataTo16BPPBufferTransparentClip(pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect);
				else
					Blt8BPPDataTo16BPPBufferTransparent(pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX, iDestY, usIndex);
				break;
	}

	return( TRUE );

}


/**********************************************************************************************
 DestroyObjectPaletteTables

	Destroys the palette tables of a video object. All memory is deallocated, and
	the pointers set to NULL. Be careful not to try and blit this object until new
	tables are calculated, or things WILL go boom.

**********************************************************************************************/
BOOLEAN DestroyObjectPaletteTables(HVOBJECT hVObject)
{
UINT32 x;
BOOLEAN f16BitPal;

	for ( x = 0; x < HVOBJECT_SHADE_TABLES; x++ )
	{
		if ( !( hVObject->fFlags & VOBJECT_FLAG_SHADETABLE_SHARED ) )
		{
			if ( hVObject->pShades[x] != NULL )
			{
				if ( hVObject->pShades[x] == hVObject->p16BPPPalette )
					f16BitPal = TRUE;
				else
					f16BitPal = FALSE;

				MemFree( hVObject->pShades[x] );
				hVObject->pShades[x] = NULL;

				if ( f16BitPal )
					hVObject->p16BPPPalette = NULL;
			}
		}
	}

	if ( hVObject->p16BPPPalette != NULL )
	{
		MemFree( hVObject->p16BPPPalette );
		hVObject->p16BPPPalette = NULL;
	}

	hVObject->pShadeCurrent=NULL;

	return(TRUE);

}



UINT16 SetObjectShade(HVOBJECT pObj, UINT32 uiShade)
{
	Assert(pObj!=NULL);
	Assert(uiShade >= 0);
	Assert(uiShade < HVOBJECT_SHADE_TABLES);

	if(pObj->pShades[uiShade]==NULL)
	{
		DbgMessage(TOPIC_VIDEOOBJECT, DBG_LEVEL_2, "Attempt to set shade level to NULL table");
		return(FALSE);
	}

	pObj->pShadeCurrent=pObj->pShades[uiShade];
	return(TRUE);
}

UINT16 SetObjectHandleShade(UINT32 uiHandle, UINT32 uiShade)
{
	#ifdef _DEBUG
		gubVODebugCode = DEBUGSTR_SETOBJECTHANDLESHADE;
	#endif
	HVOBJECT hObj = GetVideoObject(uiHandle);
	if (hObj == NULL)
	{
		DbgMessage(TOPIC_VIDEOOBJECT, DBG_LEVEL_2, "Invalid object handle for setting shade level");
		return(FALSE);
	}
	return(SetObjectShade(hObj, uiShade));
}


/********************************************************************************************
	GetETRLEPixelValue

	Given a VOBJECT and ETRLE image index, retrieves the value of the pixel located at the
	given image coordinates. The value returned is an 8-bit palette index
********************************************************************************************/
BOOLEAN GetETRLEPixelValue( UINT8 * pDest, HVOBJECT hVObject, UINT16 usETRLEIndex, UINT16 usX, UINT16 usY )
{
	UINT8 *					pCurrent;
	UINT16					usLoopX = 0;
	UINT16					usLoopY = 0;
	UINT16					ubRunLength;
	ETRLEObject *		pETRLEObject;

	// Do a bunch of checks
	CHECKF( hVObject != NULL );
	CHECKF( usETRLEIndex < hVObject->usNumberOfObjects );

	pETRLEObject = &(hVObject->pETRLEObject[usETRLEIndex]);

	CHECKF( usX < pETRLEObject->usWidth );
	CHECKF( usY < pETRLEObject->usHeight );

	// Assuming everything's okay, go ahead and look...
	pCurrent = &((UINT8 *)hVObject->pPixData)[pETRLEObject->uiDataOffset];

	// Skip past all uninteresting scanlines
	while( usLoopY < usY )
	{
		while( *pCurrent != 0 )
		{
			if (*pCurrent & COMPRESS_TRANSPARENT)
			{
				pCurrent++;
			}
			else
			{
				pCurrent += *pCurrent & COMPRESS_RUN_MASK;
			}
		}
		usLoopY++;
	}

	// Now look in this scanline for the appropriate byte
	do
	{
		ubRunLength = *pCurrent & COMPRESS_RUN_MASK;

		if (*pCurrent & COMPRESS_TRANSPARENT)
		{
			if (usLoopX + ubRunLength >= usX)
			{
				*pDest = 0;
				return( TRUE );
			}
			else
			{
				pCurrent++;
			}
		}
		else
		{
			if (usLoopX + ubRunLength >= usX)
			{
				// skip to the correct byte; skip at least 1 to get past the byte defining the run
				pCurrent += (usX - usLoopX) + 1;
				*pDest = *pCurrent;
				return( TRUE );
			}
			else
			{
				pCurrent += ubRunLength + 1;
			}
		}
		usLoopX += ubRunLength;
	}
	while( usLoopX < usX );
	// huh???
	return( FALSE );
}


static BOOLEAN GetVideoObjectETRLEProperties(HVOBJECT hVObject, ETRLEObject* pETRLEObject, UINT16 usIndex)
{
	//CHECKF( usIndex >= 0 ); /* XXX unsigned < 0 ? */
	CHECKF( usIndex < hVObject->usNumberOfObjects );

	memcpy( pETRLEObject, &( hVObject->pETRLEObject[ usIndex ] ), sizeof( ETRLEObject ) );

	return( TRUE );

}

BOOLEAN GetVideoObjectETRLESubregionProperties( UINT32 uiVideoObject, UINT16 usIndex, UINT16 *pusWidth, UINT16 *pusHeight )
{
	ETRLEObject						ETRLEObject;

	// Get video object
	#ifdef _DEBUG
		gubVODebugCode = DEBUGSTR_GETVIDEOOBJECTETRLESUBREGIONPROPERTIES;
	#endif
	HVOBJECT hVObject = GetVideoObject(uiVideoObject);
	CHECKF(hVObject != NULL);

	CHECKF( GetVideoObjectETRLEProperties( hVObject, &ETRLEObject, usIndex ) );

	*pusWidth = ETRLEObject.usWidth;
	*pusHeight = ETRLEObject.usHeight;

	return( TRUE );
}


BOOLEAN GetVideoObjectETRLEPropertiesFromIndex( UINT32 uiVideoObject, ETRLEObject *pETRLEObject, UINT16 usIndex )
{
	// Get video object
	#ifdef _DEBUG
		gubVODebugCode = DEBUGSTR_GETVIDEOOBJECTETRLEPROPERTIESFROMINDEX;
	#endif
	HVOBJECT hVObject = GetVideoObject(uiVideoObject);
	CHECKF(hVObject != NULL);

	CHECKF( GetVideoObjectETRLEProperties( hVObject, pETRLEObject, usIndex ) );

	return( TRUE );
}


BOOLEAN BltVideoObjectOutlineFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY, INT16 s16BPPColor, BOOLEAN fDoOutline )
{
#ifdef _DEBUG
	gubVODebugCode = DEBUGSTR_BLTVIDEOOBJECTOUTLINEFROMINDEX;
#endif
	HVOBJECT hSrcVObject = GetVideoObject(uiSrcVObject);
	CHECKF(hSrcVObject != NULL);
	return BltVideoObjectOutline(uiDestVSurface, hSrcVObject, usIndex, iDestX, iDestY, s16BPPColor, fDoOutline);
}


BOOLEAN BltVideoObjectOutline(UINT32 uiDestVSurface, HVOBJECT hSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY, INT16 s16BPPColor, BOOLEAN fDoOutline )
{
	UINT16               *pBuffer;
	UINT32								uiPitch;
	// Lock video surface
	pBuffer = (UINT16*)LockVideoSurface( uiDestVSurface, &uiPitch );

	if ( pBuffer == NULL )
	{
		return( FALSE );
	}

	if( BltIsClipped( hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect) )
	{
		 Blt8BPPDataTo16BPPBufferOutlineClip((UINT16*)pBuffer, uiPitch, hSrcVObject, iDestX, iDestY, usIndex, s16BPPColor, fDoOutline, &ClippingRect );
	}
	else
	{
		 Blt8BPPDataTo16BPPBufferOutline((UINT16*)pBuffer, uiPitch, hSrcVObject, iDestX, iDestY, usIndex, s16BPPColor, fDoOutline );
	}

	// Now we have the video object and surface, call the VO blitter function

	UnLockVideoSurface( uiDestVSurface );
	return( TRUE );
}


static BOOLEAN BltVideoObjectOutlineShadow(UINT32 uiDestVSurface, HVOBJECT hSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY);


BOOLEAN BltVideoObjectOutlineShadowFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY )
{
#ifdef _DEBUG
	gubVODebugCode = DEBUGSTR_BLTVIDEOOBJECTOUTLINESHADOWFROMINDEX;
#endif
	HVOBJECT hSrcVObject = GetVideoObject(uiSrcVObject);
	CHECKF(hSrcVObject != NULL);
	return BltVideoObjectOutlineShadow(uiDestVSurface, hSrcVObject, usIndex, iDestX, iDestY);
}


static BOOLEAN BltVideoObjectOutlineShadow(UINT32 uiDestVSurface, HVOBJECT hSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY)
{
	UINT16               *pBuffer;
	UINT32								uiPitch;
	// Lock video surface
	pBuffer = (UINT16*)LockVideoSurface( uiDestVSurface, &uiPitch );

	if ( pBuffer == NULL )
	{
		return( FALSE );
	}

	if( BltIsClipped( hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect) )
	{
		 Blt8BPPDataTo16BPPBufferOutlineShadowClip((UINT16*)pBuffer, uiPitch, hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect );
	}
	else
	{
		 Blt8BPPDataTo16BPPBufferOutlineShadow((UINT16*)pBuffer, uiPitch, hSrcVObject, iDestX, iDestY, usIndex );
	}

	// Now we have the video object and surface, call the VO blitter function

	UnLockVideoSurface( uiDestVSurface );
	return( TRUE );
}


#ifdef _DEBUG
static void CheckValidVObjectIndex(UINT32 uiIndex)
{
	BOOLEAN fAssertError = FALSE;
	if( uiIndex == 0xffffffff )
	{ //-1 index -- deleted
		fAssertError = TRUE;
	}
	if( !(uiIndex % 2) && uiIndex < 0xfffffff0 || uiIndex >= 0xfffffff0 )
	{ //even numbers are reserved for vsurfaces as well as the 0xfffffff0+ values
		fAssertError = TRUE;
	}

	if( fAssertError )
	{
		const char* str;
		switch( gubVODebugCode )
		{
			case DEBUGSTR_BLTVIDEOOBJECTFROMINDEX:
				str = "BltVideoObjectFromIndex";
				break;
			case DEBUGSTR_SETOBJECTHANDLESHADE:
				str = "SetObjectHandleShade";
				break;
			case DEBUGSTR_GETVIDEOOBJECTETRLESUBREGIONPROPERTIES:
				str = "GetVideoObjectETRLESubRegionProperties";
				break;
			case DEBUGSTR_GETVIDEOOBJECTETRLEPROPERTIESFROMINDEX:
				str = "GetVideoObjectETRLEPropertiesFromIndex";
				break;
			case DEBUGSTR_BLTVIDEOOBJECTOUTLINEFROMINDEX:
				str = "BltVideoObjectOutlineFromIndex";
				break;
			case DEBUGSTR_BLTVIDEOOBJECTOUTLINESHADOWFROMINDEX:
				str = "BltVideoObjectOutlineShadowFromIndex";
				break;
			case DEBUGSTR_DELETEVIDEOOBJECTFROMINDEX:
				str = "DeleteVideoObjectFromIndex";
				break;
			case DEBUGSTR_NONE:
			default:
				str = "GetVideoObject";
				break;
		}
		if( uiIndex == 0xffffffff )
		{
			AssertMsg( 0, String( "Trying to %s with deleted index -1." , str ) );
		}
		else
		{
			AssertMsg( 0, String( "Trying to %s using a VSURFACE ID %d!", str, uiIndex ) );
		}
	}
}
#endif

#ifdef SGP_VIDEO_DEBUGGING

typedef struct DUMPFILENAME
{
	UINT8 str[256];
}DUMPFILENAME;

static void DumpVObjectInfoIntoFile(const char *filename, BOOLEAN fAppend)
{
	VOBJECT_NODE *curr;
	FILE *fp;
	DUMPFILENAME *pName, *pCode;
	UINT32 *puiCounter;
	UINT8 tempName[ 256 ];
	UINT8 tempCode[ 256 ];
	UINT32 i, uiUniqueID;
	BOOLEAN fFound;
	if( !guiVObjectSize )
	{
		return;
	}

	if( fAppend )
	{
		fp = fopen( filename, "a" );
	}
	else
	{
		fp = fopen( filename, "w" );
	}
	Assert( fp );

	//Allocate enough strings and counters for each node.
	pName = (DUMPFILENAME*)MemAlloc( sizeof( DUMPFILENAME ) * guiVObjectSize );
	pCode = (DUMPFILENAME*)MemAlloc( sizeof( DUMPFILENAME ) * guiVObjectSize );
	memset( pName, 0, sizeof( DUMPFILENAME ) * guiVObjectSize );
	memset( pCode, 0, sizeof( DUMPFILENAME ) * guiVObjectSize );
	puiCounter = (UINT32*)MemAlloc( 4 * guiVObjectSize );
	memset( puiCounter, 0, 4 * guiVObjectSize );

	//Loop through the list and record every unique filename and count them
	uiUniqueID = 0;
	curr = gpVObjectHead;
	while( curr )
	{
		strcpy( tempName, curr->pName );
		strcpy( tempCode, curr->pCode );
		fFound = FALSE;
		for( i = 0; i < uiUniqueID; i++ )
		{
			if (strcasecmp(tempName, pName[i].str) == 0 && strcasecmp(tempCode, pCode[i].str) == 0)
			{ //same string
				fFound = TRUE;
				(puiCounter[ i ])++;
				break;
			}
		}
		if( !fFound )
		{
			strcpy( pName[i].str, tempName );
			strcpy( pCode[i].str, tempCode );
			(puiCounter[ i ])++;
			uiUniqueID++;
		}
		curr = curr->next;
	}

	//Now dump the info.
	fprintf( fp, "-----------------------------------------------\n" );
	fprintf( fp, "%d unique vObject names exist in %d VObjects\n", uiUniqueID, guiVObjectSize );
	fprintf( fp, "-----------------------------------------------\n\n" );
	for( i = 0; i < uiUniqueID; i++ )
	{
		fprintf( fp, "%d occurrences of %s\n", puiCounter[i], pName[i].str );
		fprintf( fp, "%s\n\n", pCode[i].str );
	}
	fprintf( fp, "\n-----------------------------------------------\n\n" );

	//Free all memory associated with this operation.
	MemFree( pName );
	MemFree( pCode );
	MemFree( puiCounter );
	fclose( fp );
}


//Debug wrapper for adding vObjects
static void RecordVObject(const char* Filename, UINT32 uiLineNum, const char* pSourceFile)
{
	UINT16 usLength;
	UINT8 str[256];

	//record the filename of the vObject (some are created via memory though)
	usLength = strlen(Filename) + 1;
	gpVObjectTail->pName = (UINT8*)MemAlloc( usLength );
	memset( gpVObjectTail->pName, 0, usLength );
	strcpy(gpVObjectTail->pName, Filename);

	//record the code location of the calling creating function.
	sprintf( str, "%s -- line(%d)", pSourceFile, uiLineNum );
	usLength = strlen( str ) + 1;
	gpVObjectTail->pCode = (UINT8*)MemAlloc( usLength );
	memset( gpVObjectTail->pCode, 0, usLength );
	strcpy( gpVObjectTail->pCode, str );
}


BOOLEAN AddAndRecordVObjectFromHImage(HIMAGE hImage, UINT32* uiIndex, UINT32 uiLineNum, const char* pSourceFile)
{
	BOOLEAN Res = AddStandardVideoObjectFromHImage(hImage, uiIndex);
	if (Res) RecordVObject("<IMAGE>", uiLineNum, pSourceFile);
	return Res;
}


BOOLEAN AddAndRecordVObjectFromFile(const char* ImageFile, UINT32* uiIndex, UINT32 uiLineNum, const char* pSourceFile)
{
	BOOLEAN Res = AddStandardVideoObjectFromFile(ImageFile, uiIndex);
	if (Res) RecordVObject(ImageFile, uiLineNum, pSourceFile);
	return Res;
}


void PerformVideoInfoDumpIntoFile(const char *filename, BOOLEAN fAppend)
{
	DumpVObjectInfoIntoFile( filename, fAppend );
	DumpVSurfaceInfoIntoFile( filename, TRUE );
}

#endif
