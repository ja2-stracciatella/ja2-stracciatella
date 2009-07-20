#include <stdexcept>

#include "HImage.h"
#include "PODObj.h"
#include "Structure.h"
#include "TileDef.h"
#include "Tile_Surface.h"
#include "VObject.h"
#include "WorldDef.h"
#include "WorldDat.h"
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
try
{
	// Add tile surface
	AutoSGPImage   hImage(CreateImage(cFilename, IMAGE_ALLDATA));
	AutoSGPVObject hVObject(AddVideoObjectFromHImage(hImage));

	// Load structure data, if any.
	// Start by hacking the image filename into that for the structure data
	SGPFILENAME cStructureFilename;
	ReplacePath(cStructureFilename, lengthof(cStructureFilename), 0, cFilename, "." STRUCTURE_FILE_EXTENSION);

	AutoStructureFileRef pStructureFileRef;
	if (FileExists( cStructureFilename ))
	{
		pStructureFileRef = LoadStructureFile( cStructureFilename );

		if (hVObject->SubregionCount() != pStructureFileRef->usNumberOfStructures)
		{
			throw std::runtime_error("Structure file error");
		}

		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, cStructureFilename );

		AddZStripInfoToVObject(hVObject, pStructureFileRef, FALSE, 0);
	}

	SGP::PODObj<TILE_IMAGERY> pTileSurf;

	if (pStructureFileRef && pStructureFileRef->pAuxData != NULL)
	{
		pTileSurf->pAuxData = pStructureFileRef->pAuxData;
		pTileSurf->pTileLocData = pStructureFileRef->pTileLocData;
	}
	else if (hImage->uiAppDataSize == hVObject->SubregionCount() * sizeof(AuxObjectData))
	{
		// Valid auxiliary data, so make a copy of it for TileSurf
		pTileSurf->pAuxData = MALLOCN(AuxObjectData, hVObject->SubregionCount());
		memcpy( pTileSurf->pAuxData, hImage->pAppData, hImage->uiAppDataSize );
	}
	else
	{
		pTileSurf->pAuxData = NULL;
	}

	pTileSurf->vo                = hVObject.Release();
	pTileSurf->pStructureFileRef = pStructureFileRef.Release();
	return pTileSurf.Release();
}
catch (...)
{
	SET_ERROR("Could not load tile file: %s", cFilename);
	throw;
}


void DeleteTileSurface(TILE_IMAGERY* const pTileSurf)
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

	DeleteVideoObject(pTileSurf->vo);
	MemFree( pTileSurf );
}


void SetRaisedObjectFlag(char const* const filename, TILE_IMAGERY* const t)
{
	static char const RaisedObjectFiles[][9] =
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

	if (DEBRISWOOD != t->fType && t->fType != DEBRISWEEDS && t->fType != DEBRIS2MISC && t->fType != ANOTHERDEBRIS) return;

	// Loop through array of RAISED objecttype imagery and set global value
	char rootfile[128];
	GetRootName(rootfile, lengthof(rootfile), filename);
	for (char const (*i)[9] = RaisedObjectFiles; i != endof(RaisedObjectFiles); ++i)
	{
		if (strcasecmp(*i, rootfile) != 0) continue;
		t->bRaisedObjectType = TRUE;
		return;
	}
}
