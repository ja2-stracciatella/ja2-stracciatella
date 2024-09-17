#include "Tile_Surface.h"
#include "ContentManager.h"
#include "FileMan.h"
#include "GameInstance.h"
#include "HImage.h"
#include "Logger.h"
#include "PODObj.h"
#include "Structure.h"
#include "Structure_Internals.h"
#include "Sys_Globals.h"
#include "TileDef.h"
#include "Types.h"
#include "VObject.h"
#include <array>
#include <stdexcept>
#include <string_theory/format>



TILE_IMAGERY				*gTileSurfaceArray[ NUMBEROFTILETYPES ];


TILE_IMAGERY* LoadTileSurface(ST::string const& cFilename)
try
{
	// Add tile surface
	AutoSGPImage   hImage(CreateImage(cFilename, IMAGE_ALLDATA));
	AutoSGPVObject hVObject(AddVideoObjectFromHImage(hImage.get()));

	// Load structure data, if any.
	// Start by hacking the image filename into that for the structure data
	ST::string cStructureFilename(FileMan::replaceExtension(cFilename, "jsd"));

	std::unique_ptr<STRUCTURE_FILE_REF> pStructureFileRef;
	if (GCM->doesGameResExists( cStructureFilename ))
	{
		SLOGD("loading tile {}", cStructureFilename);

		pStructureFileRef.reset(LoadStructureFile(cStructureFilename));

		if (hVObject->SubregionCount() != pStructureFileRef->usNumberOfStructures)
		{
			throw std::runtime_error("Structure file error");
		}

		AddZStripInfoToVObject(hVObject.get(), pStructureFileRef.get(), FALSE, 0);
	}

	SGP::PODObj<TILE_IMAGERY> pTileSurf;

	if (pStructureFileRef && !pStructureFileRef->pAuxData.empty())
	{
		pTileSurf->pAuxData = pStructureFileRef->pAuxData.data();
		pTileSurf->pTileLocData = pStructureFileRef->pTileLocData.data();
	}
	else if (hImage->uiAppDataSize == hVObject->SubregionCount() * sizeof(AuxObjectData))
	{
		// Valid auxiliary data, so move it to TileSurf
		pTileSurf->pAuxData = reinterpret_cast<AuxObjectData *>(hImage->pAppData.Release());
	}
	else
	{
		pTileSurf->pAuxData = NULL;
	}

	pTileSurf->vo                = hVObject.release();
	pTileSurf->pStructureFileRef.swap(pStructureFileRef);
	return pTileSurf.Release();
}
catch (...)
{
	SET_ERROR(ST::format("Could not load tile file : {}", cFilename));
	throw;
}


void DeleteTileSurface(TILE_IMAGERY* const pTileSurf)
{
	if (!pTileSurf->pStructureFileRef)
	{
		// If a structure file exists, it will free the auxdata.
		// Since there is no structure file in this instance, we
		// free it ourselves.
		delete[] pTileSurf->pAuxData;
	}

	DeleteVideoObject(pTileSurf->vo);
	delete pTileSurf;
}


void SetRaisedObjectFlag(ST::string const& filename, TILE_IMAGERY* const t)
{
	static std::array<const ST::string, 11> const raisedObjectFiles = {
		ST_LITERAL("bones"),
		ST_LITERAL("bones2"),
		ST_LITERAL("grass2"),
		ST_LITERAL("grass3"),
		ST_LITERAL("l_weed3"),
		ST_LITERAL("litter"),
		ST_LITERAL("miniweed"),
		ST_LITERAL("sblast"),
		ST_LITERAL("sweeds"),
		ST_LITERAL("twigs"),
		ST_LITERAL("wing")
	};

	if (DEBRISWOOD != t->fType && t->fType != DEBRISWEEDS && t->fType != DEBRIS2MISC && t->fType != ANOTHERDEBRIS) return;

	// Loop through array of RAISED objecttype imagery and set global value
	ST::string const rootfile = FileMan::getFileNameWithoutExt(filename);
	for (ST::string const& i : raisedObjectFiles)
	{
		if (i.compare_i(rootfile) == 0)
		{
			t->bRaisedObjectType = TRUE;
			return;
		}
	}
}
