#ifdef JA2_PRECOMPILED_HEADERS
	#include "JA2 SGP ALL.H"
#elif defined( WIZ8_PRECOMPILED_HEADERS )
	#include "WIZ8 SGP ALL.H"
#else
	#include "DirectDraw Calls.h"
	#include <stdio.h>
	#include "debug.h"
	#if defined( JA2 ) || defined( UTIL )
		#include "video.h"
	#else
		#include "video2.h"
	#endif
	#include "himage.h"
	#include "vobject.h"
	#include "vobject_private.h"
	#include "video_private.h"
	#include "wcheck.h"
	#include "vobject_blitters.h"
	#include "sgp.h"
#endif

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


// *******************************************************************************
// Defines
// *******************************************************************************

// This define is sent to CreateList SGP function. It dynamically re-sizes if
// the list gets larger
#define DEFAULT_VIDEO_OBJECT_LIST_SIZE	10

#define COMPRESS_TRANSPARENT				0x80
#define COMPRESS_RUN_MASK						0x7F

// *******************************************************************************
// External Functions and variables
// *******************************************************************************

// *******************************************************************************
// LOCAL functions
// *******************************************************************************


// *******************************************************************************
// LOCAL global variables
// *******************************************************************************



HLIST		ghVideoObjects = NULL;
BOOLEAN	gfVideoObjectsInit=FALSE;

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

VOBJECT_NODE  *gpVObjectHead = NULL;
VOBJECT_NODE  *gpVObjectTail = NULL;
UINT32				guiVObjectIndex = 1;
UINT32				guiVObjectSize = 0;
UINT32				guiVObjectTotalAdded = 0;

#ifdef _DEBUG
enum
{
	DEBUGSTR_NONE,
	DEBUGSTR_SETVIDEOOBJECTTRANSPARENCY,
	DEBUGSTR_BLTVIDEOOBJECTFROMINDEX,
	DEBUGSTR_SETOBJECTHANDLESHADE,
	DEBUGSTR_GETVIDEOOBJECTETRLESUBREGIONPROPERTIES,
	DEBUGSTR_GETVIDEOOBJECTETRLEPROPERTIESFROMINDEX,
	DEBUGSTR_SETVIDEOOBJECTPALETTE8BPP,
	DEBUGSTR_GETVIDEOOBJECTPALETTE16BPP,
	DEBUGSTR_COPYVIDEOOBJECTPALETTE16BPP,
	DEBUGSTR_BLTVIDEOOBJECTOUTLINEFROMINDEX,
	DEBUGSTR_BLTVIDEOOBJECTOUTLINESHADOWFROMINDEX,
	DEBUGSTR_DELETEVIDEOOBJECTFROMINDEX
};

UINT8 gubVODebugCode = 0;

void CheckValidVObjectIndex( UINT32 uiIndex );
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
	gfVideoObjectsInit=TRUE;
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
	guiVObjectTotalAdded = 0;
	UnRegisterDebugTopic(TOPIC_VIDEOOBJECT, "Video Objects");
	gfVideoObjectsInit=FALSE;
	return TRUE;
}

UINT32 CountVideoObjectNodes()
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

BOOLEAN AddStandardVideoObject( VOBJECT_DESC *pVObjectDesc, UINT32 *puiIndex )
{

	HVOBJECT hVObject;

	// Assertions
	Assert( puiIndex );
	Assert( pVObjectDesc );

	// Create video object
	hVObject = CreateVideoObject( pVObjectDesc );

	if( !hVObject )
	{
		// Video Object will set error condition.
		return FALSE ;
	}

	// Set transparency to default
	SetVideoObjectTransparencyColor( hVObject, FROMRGB( 0, 0, 0 ) );

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
	guiVObjectTotalAdded++;

	#ifdef JA2TESTVERSION
		if( CountVideoObjectNodes() != guiVObjectSize )
		{
			guiVObjectSize = guiVObjectSize;
		}
	#endif

	return TRUE ;
}


BOOLEAN SetVideoObjectTransparency( UINT32 uiIndex, COLORVAL TransColor )
{
	HVOBJECT hVObject;

	// Get video object
	#ifdef _DEBUG
		gubVODebugCode = DEBUGSTR_SETVIDEOOBJECTTRANSPARENCY;
	#endif
	CHECKF( GetVideoObject( &hVObject, uiIndex ) );

	// Set transparency
	SetVideoObjectTransparencyColor( hVObject, TransColor );

	return( TRUE );
}

BOOLEAN GetVideoObject( HVOBJECT *hVObject, UINT32 uiIndex )
{
	VOBJECT_NODE *curr;

	#ifdef _DEBUG
		CheckValidVObjectIndex( uiIndex );
	#endif

	curr = gpVObjectHead;
	while( curr )
	{
		if( curr->uiIndex == uiIndex )
		{
			*hVObject = curr->hVObject;
			return TRUE;
		}
		curr = curr->next;
	}
	return FALSE;
}

BOOLEAN BltVideoObjectFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject, UINT16 usRegionIndex, INT32 iDestX, INT32 iDestY, UINT32 fBltFlags, blt_fx *pBltFx )
{
	UINT16               *pBuffer;
	UINT32								uiPitch;
	HVOBJECT							hSrcVObject;

	// Lock video surface
	pBuffer = (UINT16*)LockVideoSurface( uiDestVSurface, &uiPitch );

	if ( pBuffer == NULL )
	{
		return( FALSE );
	}

	// Get video object
	#ifdef _DEBUG
		gubVODebugCode = DEBUGSTR_BLTVIDEOOBJECTFROMINDEX;
	#endif
	if( !GetVideoObject( &hSrcVObject, uiSrcVObject ) )
	{
		UnLockVideoSurface( uiDestVSurface );
		return FALSE;
	}

	// Now we have the video object and surface, call the VO blitter function
	if ( !BltVideoObjectToBuffer( pBuffer, uiPitch, hSrcVObject, usRegionIndex, iDestX, iDestY, fBltFlags, pBltFx ) )
	{
    UnLockVideoSurface( uiDestVSurface );
		// VO Blitter will set debug messages for error conditions
		return FALSE;
	}

	UnLockVideoSurface( uiDestVSurface );
	return( TRUE );
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



// Given an index to the dest and src vobject contained in ghVideoObjects
// Based on flags, blit accordingly
// There are two types, a BltFast and a Blt. BltFast is 10% faster, uses no
// clipping lists
BOOLEAN BltVideoObject(  UINT32	uiDestVSurface,
												 HVOBJECT hSrcVObject,
												 UINT16 usRegionIndex,
												 INT32  iDestX,
												 INT32  iDestY,
												 UINT32 fBltFlags,
												 blt_fx *pBltFx )
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
	if ( !BltVideoObjectToBuffer( pBuffer, uiPitch, hSrcVObject, usRegionIndex, iDestX, iDestY, fBltFlags, pBltFx ) )
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


HVOBJECT CreateVideoObject( VOBJECT_DESC *VObjectDesc )
{
	HVOBJECT						hVObject;
	HIMAGE							hImage;
	ETRLEData						TempETRLEData;
//	UINT32							count;

	// Allocate memory for video object data and initialize
	hVObject = MemAlloc( sizeof( SGPVObject ) );
	CHECKF( hVObject != NULL );
	memset( hVObject, 0, sizeof( SGPVObject ) );

	// default of all members of the vobject is 0

	// Check creation options
//	do
//	{
		if ( VObjectDesc->fCreateFlags & VOBJECT_CREATE_FROMFILE || VObjectDesc->fCreateFlags & VOBJECT_CREATE_FROMHIMAGE )
		{
			if ( VObjectDesc->fCreateFlags & VOBJECT_CREATE_FROMFILE )
			{
				// Create himage object from file
				hImage = CreateImage( VObjectDesc->ImageFile, IMAGE_ALLIMAGEDATA );

				if ( hImage == NULL )
				{
						MemFree( hVObject );
						DbgMessage( TOPIC_VIDEOOBJECT, DBG_LEVEL_2, "Invalid Image Filename given" );
						return( NULL );
				}
			}
			else
			{ // create video object from provided hImage
				hImage = VObjectDesc->hImage;
				if ( hImage == NULL )
				{
						MemFree( hVObject );
						DbgMessage( TOPIC_VIDEOOBJECT, DBG_LEVEL_2, "Invalid hImage pointer given" );
						return( NULL );
				}
			}

			// Check if returned himage is TRLE compressed - return error if not
			if ( ! (hImage->fFlags & IMAGE_TRLECOMPRESSED ) )
			{
					MemFree( hVObject );
					DbgMessage( TOPIC_VIDEOOBJECT, DBG_LEVEL_2, "Invalid Image format given." );
					DestroyImage( hImage );
					return( NULL );
			}

			// Set values from himage
			hVObject->ubBitDepth				= hImage->ubBitDepth;

			// Get TRLE data
			CHECKF( GetETRLEImageData( hImage, &TempETRLEData ) );

			// Set values
			hVObject->usNumberOfObjects	= TempETRLEData.usNumberOfObjects;
			hVObject->pETRLEObject			= TempETRLEData.pETRLEObject;
			hVObject->pPixData					= TempETRLEData.pPixData;
			hVObject->uiSizePixData			= TempETRLEData.uiSizePixData;

			// Set palette from himage
			if ( hImage->ubBitDepth == 8 )
			{
				hVObject->pShade8=ubColorTables[DEFAULT_SHADE_LEVEL];
				hVObject->pGlow8=ubColorTables[0];

				SetVideoObjectPalette( hVObject, hImage->pPalette );

			}

			if ( VObjectDesc->fCreateFlags & VOBJECT_CREATE_FROMFILE )
			{
				// Delete himage object
				DestroyImage( hImage );
			}
	//		break;
		}
		else
		{
			MemFree( hVObject );
			DbgMessage( TOPIC_VIDEOOBJECT, DBG_LEVEL_2, "Invalid VObject creation flags given." );
			return( NULL );
		}

		// If here, no special options given, use structure given in paraneters
		// TO DO:

//	}
//	while( FALSE );

	// All is well
//  DbgMessage( TOPIC_VIDEOOBJECT, DBG_LEVEL_3, String("Success in Creating Video Object" ) );

	return( hVObject );
}


// Palette setting is expensive, need to set both DDPalette and create 16BPP palette
BOOLEAN SetVideoObjectPalette( HVOBJECT hVObject, SGPPaletteEntry *pSrcPalette )
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

//  DbgMessage(TOPIC_VIDEOOBJECT, DBG_LEVEL_3, String("Video Object Palette change successfull" ));
	return( TRUE );
}

// Transparency needs to take RGB value and find best fit and place it into DD Surface
// colorkey value.
BOOLEAN SetVideoObjectTransparencyColor( HVOBJECT hVObject, COLORVAL TransColor )
{

	// Assertions
	Assert( hVObject != NULL );

	//Set trans color into video object
	hVObject->TransparentColor = TransColor;

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

	if ( hVObject->usNumberOf16BPPObjects > 0)
	{
		for( usLoop = 0; usLoop < hVObject->usNumberOf16BPPObjects; usLoop++)
		{
			MemFree( hVObject->p16BPPObject[usLoop].p16BPPData );
		}
		MemFree( hVObject->p16BPPObject );
	}

	// Release object
	MemFree( hVObject );

	return( TRUE );
}

/**********************************************************************************************
 CreateObjectPaletteTables

		Creates the shading tables for 8-bit brushes. One highlight table is created, based on
	the object-type, 3 brightening tables, 1 normal, and 11 darkening tables. The entries are
	created iteratively, rather than in a loop to allow hand-tweaking of the values. If you
	change the HVOBJECT_SHADE_TABLES symbol, remember to add/delete entries here, it won't
	adjust automagically.

**********************************************************************************************/

UINT16 CreateObjectPaletteTables(HVOBJECT pObj, UINT32 uiType)
{
UINT32 count;

		// this creates the highlight table. Specify the glow-type when creating the tables
		// through uiType, symbols are from VOBJECT.H
	for( count = 0; count < 16; count++ )
	{
		if ( (count == 4) && (pObj->p16BPPPalette == pObj->pShades[ count ]) )
			pObj->pShades[ count ] = NULL;
		else if ( pObj->pShades[ count ] != NULL )
		{
			MemFree( pObj->pShades[ count ] );
			pObj->pShades[ count ] = NULL;
		}
	}

		switch(uiType)
		{
			case HVOBJECT_GLOW_GREEN:	// green glow
				pObj->pShades[0]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 0, 255, 0, TRUE);
				break;
			case HVOBJECT_GLOW_BLUE:	// blue glow
				pObj->pShades[0]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 0, 0, 255, TRUE);
				break;
			case HVOBJECT_GLOW_YELLOW:	// yellow glow
				pObj->pShades[0]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 255, 255, 0, TRUE);
				break;
			case HVOBJECT_GLOW_RED:	// red glow
				pObj->pShades[0]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 255, 0, 0, TRUE);
				break;
		}

		// these are the brightening tables, 115%-150% brighter than original
		pObj->pShades[1]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 293, 293, 293, FALSE);
		pObj->pShades[2]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 281, 281, 281, FALSE);
		pObj->pShades[3]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 268, 268, 268, FALSE);

		// palette 4 is the non-modified palette.
		// if the standard one has already been made, we'll use it
		if(pObj->p16BPPPalette!=NULL)
			pObj->pShades[4]=pObj->p16BPPPalette;
		else
		{
			// or create our own, and assign it to the standard one
			pObj->pShades[4]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 255, 255, 255, FALSE);
			pObj->p16BPPPalette=pObj->pShades[4];
		}

		// the rest are darkening tables, right down to all-black.
		pObj->pShades[5]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 195, 195, 195, FALSE);
		pObj->pShades[6]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 165, 165, 165, FALSE);
		pObj->pShades[7]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 135, 135, 135, FALSE);
		pObj->pShades[8]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 105, 105, 105, FALSE);
		pObj->pShades[9]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 75, 75, 75, FALSE);
		pObj->pShades[10]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 45, 45, 45, FALSE);
		pObj->pShades[11]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 36, 36, 36, FALSE);
		pObj->pShades[12]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 27, 27, 27, FALSE);
		pObj->pShades[13]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 18, 18, 18, FALSE);
		pObj->pShades[14]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 9, 9, 9, FALSE);
		pObj->pShades[15]=Create16BPPPaletteShaded( pObj->pPaletteEntry, 0, 0, 0, FALSE);

		// Set current shade table to neutral color
		pObj->pShadeCurrent=pObj->pShades[4];

		// check to make sure every table got a palette
		for(count=0; (count < HVOBJECT_SHADE_TABLES) && (pObj->pShades[count]!=NULL); count++);


		// return the result of the check
		return(count==HVOBJECT_SHADE_TABLES);
}

// *******************************************************************
//
// Blitting Functions
//
// *******************************************************************

// High level blit function encapsolates ALL effects and BPP
BOOLEAN BltVideoObjectToBuffer( UINT16 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY, INT32 fBltFlags, blt_fx *pBltFx )
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
				do
				{
					if(gbPixelDepth==16)
					{
#ifndef JA2
						if ( fBltFlags & VO_BLT_MIRROR_Y)
						{
							if(!BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect))
								Blt8BPPDataTo16BPPBufferTransMirror( pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX, iDestY, usIndex );
// CLipping version not done -- DB
//								Blt8BPPDataTo16BPPBufferTransMirrorClip( pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect);
							break;
						}
						else
#endif
						if ( fBltFlags & VO_BLT_SRCTRANSPARENCY  )
						{
							if(BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect))
								Blt8BPPDataTo16BPPBufferTransparentClip( pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect);
							else
								Blt8BPPDataTo16BPPBufferTransparent( pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX, iDestY, usIndex );
							break;
						}
						else if ( fBltFlags & VO_BLT_SHADOW  )
						{
							if(BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect))
								Blt8BPPDataTo16BPPBufferShadowClip( pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect);
							else
								Blt8BPPDataTo16BPPBufferShadow( pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX, iDestY, usIndex );
							break;
						}

					}
					else if(gbPixelDepth==8)
					{
						if ( fBltFlags & VO_BLT_SRCTRANSPARENCY  )
						{
							if(BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect))
								Blt8BPPDataTo8BPPBufferTransparentClip( pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect);
							else
								Blt8BPPDataTo8BPPBufferTransparent( pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX, iDestY, usIndex );
							break;
						}
						else if ( fBltFlags & VO_BLT_SHADOW  )
						{
							if(BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect))
								Blt8BPPDataTo8BPPBufferShadowClip( pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect);
							else
								Blt8BPPDataTo8BPPBufferShadow( pBuffer, uiDestPitchBYTES, hSrcVObject, iDestX, iDestY, usIndex );
							break;
						}
					}
					// Use default blitter here
					//Blt8BPPDataTo16BPPBuffer( hDestVObject, hSrcVObject, (UINT16)iDestX, (UINT16)iDestY, (SGPRect*)&SrcRect );

				} while( FALSE );


				break;
	}

	return( TRUE );

}

BOOLEAN PixelateVideoObjectRect(  UINT32	uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2)
{
	UINT16 *pBuffer;
	UINT32 uiPitch;
	SGPRect   area;
	UINT8 uiPattern[8][8]={	{0,1,0,1,0,1,0,1},
													{1,0,1,0,1,0,1,0},
													{0,1,0,1,0,1,0,1},
													{1,0,1,0,1,0,1,0},
													{0,1,0,1,0,1,0,1},
													{1,0,1,0,1,0,1,0},
													{0,1,0,1,0,1,0,1},
													{1,0,1,0,1,0,1,0}};

	// Lock video surface
	pBuffer = (UINT16*)LockVideoSurface( uiDestVSurface, &uiPitch );

	if ( pBuffer == NULL )
	{
		return( FALSE );
	}

	area.iTop=Y1;
	area.iBottom=Y2;
	area.iLeft=X1;
	area.iRight=X2;

	// Now we have the video object and surface, call the shadow function
	if(!Blt16BPPBufferPixelateRect(pBuffer, uiPitch, &area, uiPattern))
	{
		UnLockVideoSurface( uiDestVSurface );
		// Blit has failed if false returned
		return( FALSE );
	}

	// Mark as dirty if it's the backbuffer
	//if ( uiDestVSurface == BACKBUFFER )
	//{
	//	InvalidateBackbuffer( );
	//}

	UnLockVideoSurface( uiDestVSurface );
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
	hVObject->pGlow=NULL;

	return(TRUE);

}



UINT16 SetObjectShade(HVOBJECT pObj, UINT32 uiShade)
{
	Assert(pObj!=NULL);
	Assert(uiShade >= 0);
	Assert(uiShade < HVOBJECT_SHADE_TABLES);

	if(pObj->pShades[uiShade]==NULL)
	{
	  DbgMessage(TOPIC_VIDEOOBJECT, DBG_LEVEL_2, String("Attempt to set shade level to NULL table"));
		return(FALSE);
	}

	pObj->pShadeCurrent=pObj->pShades[uiShade];
	return(TRUE);
}

UINT16 SetObjectHandleShade(UINT32 uiHandle, UINT32 uiShade)
{
	HVOBJECT hObj;

	#ifdef _DEBUG
		gubVODebugCode = DEBUGSTR_SETOBJECTHANDLESHADE;
	#endif
	if(!GetVideoObject(&hObj, uiHandle))
	{
	  DbgMessage(TOPIC_VIDEOOBJECT, DBG_LEVEL_2, String("Invalid object handle for setting shade level"));
		return(FALSE);
	}
	return(SetObjectShade(hObj, uiShade));
}

/*
UINT16 FillObjectRect(UINT32 iObj, INT32 x1, INT32 y1, INT32 x2, INT32 y2, COLORVAL color32)
{
UINT16	*pBuffer;
UINT32	uiPitch;
//HVSURFACE pSurface;

	// Lock video surface
	pBuffer = (UINT16*)LockVideoSurface(iObj, &uiPitch );
	//UnLockVideoSurface(iObj);


	if (pBuffer == NULL)
		return( FALSE );

	FillRect16BPP(pBuffer, uiPitch, x1, y1, x2, y2, Get16BPPColor(color32));

	// Mark as dirty if it's the backbuffer
	if(iObj == BACKBUFFER)
		InvalidateBackbuffer();

	UnLockVideoSurface(iObj);
}

*/


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

BOOLEAN GetVideoObjectETRLEProperties( HVOBJECT hVObject, ETRLEObject *pETRLEObject, UINT16 usIndex )
{
	CHECKF( usIndex >= 0 );
	CHECKF( usIndex < hVObject->usNumberOfObjects );

	memcpy( pETRLEObject, &( hVObject->pETRLEObject[ usIndex ] ), sizeof( ETRLEObject ) );

	return( TRUE );

}

BOOLEAN GetVideoObjectETRLESubregionProperties( UINT32 uiVideoObject, UINT16 usIndex, UINT16 *pusWidth, UINT16 *pusHeight )
{
	HVOBJECT							hVObject;
	ETRLEObject						ETRLEObject;

	// Get video object
	#ifdef _DEBUG
		gubVODebugCode = DEBUGSTR_GETVIDEOOBJECTETRLESUBREGIONPROPERTIES;
	#endif
	CHECKF( GetVideoObject( &hVObject, uiVideoObject ) );

	CHECKF( GetVideoObjectETRLEProperties( hVObject, &ETRLEObject, usIndex ) );

	*pusWidth = ETRLEObject.usWidth;
	*pusHeight = ETRLEObject.usHeight;

	return( TRUE );
}


BOOLEAN GetVideoObjectETRLEPropertiesFromIndex( UINT32 uiVideoObject, ETRLEObject *pETRLEObject, UINT16 usIndex )
{
	HVOBJECT							hVObject;

	// Get video object
	#ifdef _DEBUG
		gubVODebugCode = DEBUGSTR_GETVIDEOOBJECTETRLEPROPERTIESFROMINDEX;
	#endif
	CHECKF( GetVideoObject( &hVObject, uiVideoObject ) );

	CHECKF( GetVideoObjectETRLEProperties( hVObject, pETRLEObject, usIndex ) );

	return( TRUE );
}

BOOLEAN SetVideoObjectPalette8BPP(INT32 uiVideoObject, SGPPaletteEntry *pPal8)
{
	HVOBJECT							hVObject;

	// Get video object
	#ifdef _DEBUG
		gubVODebugCode = DEBUGSTR_SETVIDEOOBJECTPALETTE8BPP;
	#endif
	CHECKF( GetVideoObject( &hVObject, uiVideoObject ) );

	return( SetVideoObjectPalette( hVObject, pPal8 ) );
}


BOOLEAN GetVideoObjectPalette16BPP(INT32 uiVideoObject, UINT16 **ppPal16)
{
	HVOBJECT							hVObject;

	// Get video object
	#ifdef _DEBUG
		gubVODebugCode = DEBUGSTR_GETVIDEOOBJECTPALETTE16BPP;
	#endif
	CHECKF( GetVideoObject( &hVObject, uiVideoObject ) );

	*ppPal16 = hVObject->p16BPPPalette;

	return( TRUE );
}

BOOLEAN CopyVideoObjectPalette16BPP(INT32 uiVideoObject, UINT16 *ppPal16)
{
	HVOBJECT							hVObject;

	// Get video object
	#ifdef _DEBUG
		gubVODebugCode = DEBUGSTR_COPYVIDEOOBJECTPALETTE16BPP;
	#endif
	CHECKF( GetVideoObject( &hVObject, uiVideoObject ) );

	memcpy(ppPal16, hVObject->p16BPPPalette, 256*2);

	return( TRUE );
}

BOOLEAN CheckFor16BPPRegion( HVOBJECT hVObject, UINT16 usRegionIndex, UINT8 ubShadeLevel, UINT16 * pusIndex )
{
	UINT16					usLoop;
	SixteenBPPObjectInfo *	p16BPPObject;

	if (hVObject->usNumberOf16BPPObjects > 0)
	{
		for (usLoop = 0; usLoop < hVObject->usNumberOf16BPPObjects; usLoop++)
		{
			p16BPPObject = &(hVObject->p16BPPObject[usLoop]);
			if (p16BPPObject->usRegionIndex == usRegionIndex && p16BPPObject->ubShadeLevel == ubShadeLevel)
			{
				if (pusIndex != NULL)
				{
					*pusIndex = usLoop;
				}
				return( TRUE );
			}
		}
	}
	return( FALSE );
}

BOOLEAN ConvertVObjectRegionTo16BPP( HVOBJECT hVObject, UINT16 usRegionIndex, UINT8 ubShadeLevel )
{
	SixteenBPPObjectInfo *	p16BPPObject;
	UINT8 *					pInput;
	UINT8 *					pOutput;
	UINT16 *				p16BPPPalette;
	UINT32					uiDataLoop;
	UINT32					uiDataLength;
	UINT8					ubRunLoop;
	//UINT8					ubRunLength;
	INT8					bData;
	UINT32					uiLen;

	// check for existing 16BPP region and then allocate memory
	if (usRegionIndex >= hVObject->usNumberOfObjects || ubShadeLevel >= HVOBJECT_SHADE_TABLES)
	{
		return( FALSE );
	}
	if (CheckFor16BPPRegion( hVObject, usRegionIndex, ubShadeLevel, NULL) == TRUE)
	{
		// it already exists; no need to do anything!
		return( TRUE );
	}

	if (hVObject->usNumberOf16BPPObjects > 0)
	{
		// have to reallocate memory
		hVObject->p16BPPObject = MemRealloc( hVObject->p16BPPObject, sizeof( SixteenBPPObjectInfo ) * (hVObject->usNumberOf16BPPObjects + 1) );
	}
	else
	{
		// allocate memory for the first 16BPPObject
		hVObject->p16BPPObject = MemAlloc( sizeof( SixteenBPPObjectInfo ) );
	}
	if (hVObject->p16BPPObject == NULL)
	{
		hVObject->usNumberOf16BPPObjects = 0;
		return( FALSE );
	}

	// the new object is the last one in the array
	p16BPPObject = &(hVObject->p16BPPObject[hVObject->usNumberOf16BPPObjects]);

	// need twice as much memory because of going from 8 to 16 bits
	p16BPPObject->p16BPPData = MemAlloc( hVObject->pETRLEObject[usRegionIndex].uiDataLength * 2 );
	if (p16BPPObject->p16BPPData == NULL)
	{
		return( FALSE );
	}

	p16BPPObject->usRegionIndex = usRegionIndex;
	p16BPPObject->ubShadeLevel = ubShadeLevel;
	p16BPPObject->usHeight = hVObject->pETRLEObject[ usRegionIndex ].usHeight;
	p16BPPObject->usWidth = hVObject->pETRLEObject[ usRegionIndex ].usWidth;
	p16BPPObject->sOffsetX=hVObject->pETRLEObject[ usRegionIndex ].sOffsetX;
	p16BPPObject->sOffsetY=hVObject->pETRLEObject[ usRegionIndex ].sOffsetY;

	// get the palette
	p16BPPPalette = hVObject->pShades[ubShadeLevel];
	pInput = (UINT8 *) hVObject->pPixData + hVObject->pETRLEObject[ usRegionIndex ].uiDataOffset;

	uiDataLength=hVObject->pETRLEObject[usRegionIndex].uiDataLength;

	// now actually do the conversion

	uiLen=0;
	pOutput = (UINT8 *)p16BPPObject->p16BPPData;
	for (uiDataLoop = 0; uiDataLoop < uiDataLength; uiDataLoop++)
	{
		bData= *pInput;
		if(bData&0x80)
		{
			// transparent
			*pOutput = *pInput;
			pOutput++;
			pInput++;
			//uiDataLoop++;
			uiLen+=(UINT32)(bData&0x7f);
		}
		else if(bData > 0)
		{
			// nontransparent
			*pOutput = *pInput;
			pOutput++;
			pInput++;
			//uiDataLoop++;
			for(ubRunLoop=0; ubRunLoop < bData; ubRunLoop++)
			{
				*((UINT16 *)pOutput) = p16BPPPalette[*pInput];
				pOutput++;
				pOutput++;
				pInput++;
				uiDataLoop++;
			}
			uiLen+=(UINT32)bData;
		}
		else
		{
			// eol
			*pOutput = *pInput;
			pOutput++;
			pInput++;
			//uiDataLoop++;
			if(uiLen!=p16BPPObject->usWidth)
		    DbgMessage(TOPIC_VIDEOOBJECT, DBG_LEVEL_1, String( "Actual pixel width different from header width" ));
			uiLen=0;
		}

		// copy the run-length byte
	/*	*pOutput = *pInput;
		pOutput++;
		if (((*pInput) & COMPRESS_TRANSPARENT) == 0 && *pInput > 0)
		{
			// non-transparent run; deal with the pixel data
			ubRunLoop = 0;
			ubRunLength = ((*pInput) & COMPRESS_RUN_LIMIT);
			// skip to the next input byte
			pInput++;
			for (ubRunLoop = 0; ubRunLoop < ubRunLength; ubRunLoop++)
			{
				*((UINT16 *)pOutput) = p16BPPPalette[*pInput];
				// advance two bytes in output, one in input
				pOutput++;
				pOutput++;
				pInput++;
				uiDataLoop++;
			}
		}
		else
		{
			// transparent run or end of scanline; skip to the next input byte
			pInput++;
		} */
	}
	hVObject->usNumberOf16BPPObjects++;
	return( TRUE );
}


BOOLEAN BltVideoObjectOutlineFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY, INT16 s16BPPColor, BOOLEAN fDoOutline )
{
	UINT16               *pBuffer;
	UINT32								uiPitch;
	HVOBJECT							hSrcVObject;

	// Lock video surface
	pBuffer = (UINT16*)LockVideoSurface( uiDestVSurface, &uiPitch );

	if ( pBuffer == NULL )
	{
		return( FALSE );
	}

	// Get video object
	#ifdef _DEBUG
		gubVODebugCode = DEBUGSTR_BLTVIDEOOBJECTOUTLINEFROMINDEX;
	#endif
	CHECKF( GetVideoObject( &hSrcVObject, uiSrcVObject ) );

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



BOOLEAN BltVideoObjectOutlineShadowFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY )
{
	UINT16               *pBuffer;
	UINT32								uiPitch;
	HVOBJECT							hSrcVObject;

	// Lock video surface
	pBuffer = (UINT16*)LockVideoSurface( uiDestVSurface, &uiPitch );

	if ( pBuffer == NULL )
	{
		return( FALSE );
	}

	// Get video object
	#ifdef _DEBUG
		gubVODebugCode = DEBUGSTR_BLTVIDEOOBJECTOUTLINESHADOWFROMINDEX;
	#endif
	CHECKF( GetVideoObject( &hSrcVObject, uiSrcVObject ) );

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

BOOLEAN BltVideoObjectOutlineShadow(UINT32 uiDestVSurface, HVOBJECT hSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY )
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
void CheckValidVObjectIndex( UINT32 uiIndex )
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
		UINT8 str[60];
		switch( gubVODebugCode )
		{
			case DEBUGSTR_SETVIDEOOBJECTTRANSPARENCY:
				sprintf( str, "SetVideoObjectTransparency" );
				break;
			case DEBUGSTR_BLTVIDEOOBJECTFROMINDEX:
				sprintf( str, "BltVideoObjectFromIndex" );
				break;
			case DEBUGSTR_SETOBJECTHANDLESHADE:
				sprintf( str, "SetObjectHandleShade" );
				break;
			case DEBUGSTR_GETVIDEOOBJECTETRLESUBREGIONPROPERTIES:
				sprintf( str, "GetVideoObjectETRLESubRegionProperties" );
				break;
			case DEBUGSTR_GETVIDEOOBJECTETRLEPROPERTIESFROMINDEX:
				sprintf( str, "GetVideoObjectETRLEPropertiesFromIndex" );
				break;
			case DEBUGSTR_SETVIDEOOBJECTPALETTE8BPP:
				sprintf( str, "SetVideoObjectPalette8BPP" );
				break;
			case DEBUGSTR_GETVIDEOOBJECTPALETTE16BPP:
				sprintf( str, "GetVideoObjectPalette16BPP" );
				break;
			case DEBUGSTR_COPYVIDEOOBJECTPALETTE16BPP:
				sprintf( str, "CopyVideoObjectPalette16BPP" );
				break;
			case DEBUGSTR_BLTVIDEOOBJECTOUTLINEFROMINDEX:
				sprintf( str, "BltVideoObjectOutlineFromIndex" );
				break;
			case DEBUGSTR_BLTVIDEOOBJECTOUTLINESHADOWFROMINDEX:
				sprintf( str, "BltVideoObjectOutlineShadowFromIndex" );
				break;
			case DEBUGSTR_DELETEVIDEOOBJECTFROMINDEX:
				sprintf( str, "DeleteVideoObjectFromIndex" );
				break;
			case DEBUGSTR_NONE:
			default:
				sprintf( str, "GetVideoObject" );
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

void DumpVObjectInfoIntoFile( UINT8 *filename, BOOLEAN fAppend )
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
			if( !_stricmp( tempName, pName[i].str ) && !_stricmp( tempCode, pCode[i].str ) )
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
BOOLEAN _AddAndRecordVObject( VOBJECT_DESC *VObjectDesc, UINT32 *uiIndex, UINT32 uiLineNum, UINT8 *pSourceFile )
{
	UINT16 usLength;
	UINT8 str[256];
	if( !AddStandardVideoObject( VObjectDesc, uiIndex ) )
	{
		return FALSE;
	}

	//record the filename of the vObject (some are created via memory though)
	usLength = strlen( VObjectDesc->ImageFile ) + 1;
	gpVObjectTail->pName = (UINT8*)MemAlloc( usLength );
	memset( gpVObjectTail->pName, 0, usLength );
	strcpy( gpVObjectTail->pName, VObjectDesc->ImageFile );

	//record the code location of the calling creating function.
	sprintf( str, "%s -- line(%d)", pSourceFile, uiLineNum );
	usLength = strlen( str ) + 1;
	gpVObjectTail->pCode = (UINT8*)MemAlloc( usLength );
	memset( gpVObjectTail->pCode, 0, usLength );
	strcpy( gpVObjectTail->pCode, str );

	return TRUE;
}

void PerformVideoInfoDumpIntoFile( UINT8 *filename, BOOLEAN fAppend )
{
	DumpVObjectInfoIntoFile( filename, fAppend );
	DumpVSurfaceInfoIntoFile( filename, TRUE );
}

#endif

