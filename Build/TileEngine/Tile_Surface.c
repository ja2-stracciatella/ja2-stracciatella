#include "HImage.h"
#include "VObject.h"
#include "WorldDef.h"
#include "WorldDat.h"
#include "WCheck.h"
#include "Debug.h"
#include "Smooth.h"
#include "MouseSystem.h"
#include "Sys_Globals.h"
#include "TileDat.h"
#include "FileMan.h"
#include "MemMan.h"
#include "Tile_Cache.h"


TILE_IMAGERY				*gTileSurfaceArray[ NUMBEROFTILETYPES ];


TILE_IMAGERY* LoadTileSurface(const char* cFilename)
{
	// Add tile surface
	const HIMAGE hImage = CreateImage(cFilename, IMAGE_ALLDATA);
	if (hImage == NULL)
	{
		SET_ERROR( "Could not load tile file: %s", cFilename );
		goto fail;
	}

	const HVOBJECT hVObject = AddVideoObjectFromHImage(hImage);
	if (hVObject == NULL)
	{
		SET_ERROR( "Could not load tile file: %s", cFilename );
		// Video Object will set error conition.]
		goto fail_image;
	}

	// Load structure data, if any.
	// Start by hacking the image filename into that for the structure data
	SGPFILENAME cStructureFilename;
	strcpy( cStructureFilename, cFilename );
	char* cEndOfName = strchr( cStructureFilename, '.' );
	if (cEndOfName != NULL)
	{
		cEndOfName++;
		*cEndOfName = '\0';
	}
	else
	{
		strcat( cStructureFilename, "." );
	}
	strcat( cStructureFilename, STRUCTURE_FILE_EXTENSION );
	STRUCTURE_FILE_REF* pStructureFileRef;
	if (FileExists( cStructureFilename ))
	{
		pStructureFileRef = LoadStructureFile( cStructureFilename );
		if (pStructureFileRef == NULL)
		{
			SET_ERROR("Structure file error: %s", cStructureFilename);
			goto fail_vobj;
		}

		if (hVObject->usNumberOfObjects != pStructureFileRef->usNumberOfStructures)
		{
			SET_ERROR("Structure file error: %s", cStructureFilename);
			goto fail_structure;
		}

		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, cStructureFilename );

		if (!AddZStripInfoToVObject(hVObject, pStructureFileRef, FALSE, 0))
		{
			SET_ERROR(  "ZStrip creation error: %s", cStructureFilename );
			goto fail_structure;
		}
	}
	else
	{
		pStructureFileRef = NULL;
	}

	TILE_IMAGERY* const pTileSurf = MemAlloc(sizeof(*pTileSurf));

	// Set all values to zero
	memset( pTileSurf, 0, sizeof( TILE_IMAGERY ) );

	pTileSurf->vo									= hVObject;
	pTileSurf->pStructureFileRef	= pStructureFileRef;

	if (pStructureFileRef && pStructureFileRef->pAuxData != NULL)
	{
		pTileSurf->pAuxData = pStructureFileRef->pAuxData;
		pTileSurf->pTileLocData = pStructureFileRef->pTileLocData;
	}
	else if (hImage->uiAppDataSize == hVObject->usNumberOfObjects * sizeof( AuxObjectData ))
	{
		// Valid auxiliary data, so make a copy of it for TileSurf
		pTileSurf->pAuxData = MemAlloc( hImage->uiAppDataSize );
		if (pTileSurf->pAuxData == NULL) goto fail_tile_imagery;
		memcpy( pTileSurf->pAuxData, hImage->pAppData, hImage->uiAppDataSize );
	}
	else
	{
		pTileSurf->pAuxData = NULL;
	}
	// the hImage is no longer needed
	DestroyImage( hImage );

  return( pTileSurf );

fail_tile_imagery:
	MemFree(pTileSurf);
fail_structure:
	FreeStructureFile(pStructureFileRef);
fail_vobj:
	DeleteVideoObjectFromIndex(hVObject);
fail_image:
	DestroyImage(hImage);
fail:
	return NULL;
}


void DeleteTileSurface( PTILE_IMAGERY	pTileSurf )
{
	if ( pTileSurf->pStructureFileRef != NULL )
	{
		FreeStructureFile( pTileSurf->pStructureFileRef );
	}
	else
	{
		// If a structure file exists, it will free the auxdata.
		// Since there is no structure file in this instance, we
		// free it ourselves.
		if (pTileSurf->pAuxData != NULL)
		{
			MemFree( pTileSurf->pAuxData );
		}
	}

	DeleteVideoObjectFromIndex(pTileSurf->vo);
	MemFree( pTileSurf );
}


void SetRaisedObjectFlag(const char* cFilename, TILE_IMAGERY* pTileSurf)
{
	static const char RaisedObjectFiles[][9] =
	{
		"bones",
		"bones2",
		"grass2",
		"grass3",
		"l_weed3",
		"litter",
		"miniweed",
		"sblast",
		"sweeds",
		"twigs",
		"wing"
	};

	// Loop through array of RAISED objecttype imagery and set global value...
	if ((pTileSurf->fType >= DEBRISWOOD && pTileSurf->fType <= DEBRISWEEDS) || pTileSurf->fType == DEBRIS2MISC || pTileSurf->fType == ANOTHERDEBRIS)
	{
		char cRootFile[128];
		GetRootName(cRootFile, cFilename);
		for (UINT32 i = 0; i != lengthof(RaisedObjectFiles); i++)
		{
			if (strcasecmp(RaisedObjectFiles[i], cRootFile) == 0)
			{
				pTileSurf->bRaisedObjectType = TRUE;
				return;
			}
		}
	}
}
