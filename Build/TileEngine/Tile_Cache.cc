#include <stdexcept>

#include "Directories.h"
#include "HImage.h"
#include "Structure.h"
#include "TileDef.h"
#include "Animation_Cache.h"
#include "Animation_Data.h"
#include "Debug_Control.h"
#include "Tile_Surface.h"
#include "Tile_Cache.h"
#include "FileMan.h"
#include "MemMan.h"
#include "Vector.h"


struct TILE_CACHE_STRUCT
{
	char zRootName[30];
	STRUCTURE_FILE_REF* pStructureFileRef;
};


static const UINT32 guiMaxTileCacheSize = 50;
static UINT32 guiCurTileCacheSize       = 0;
static INT32  giDefaultStructIndex      = -1;


TILE_CACHE_ELEMENT*                   gpTileCache;
static SGP::Vector<TILE_CACHE_STRUCT> gpTileCacheStructInfo;


void InitTileCache(void)
{
	gpTileCache         = MALLOCN(TILE_CACHE_ELEMENT, guiMaxTileCacheSize);
	guiCurTileCacheSize = 0;

	// Zero entries
	for (UINT32 i = 0; i < guiMaxTileCacheSize; ++i)
	{
		gpTileCache[i].pImagery        = 0;
		gpTileCache[i].struct_file_ref = 0;
	}

	// Look for JSD files in the tile cache directory and load any we find
  std::string jsd_file_pattern = FileMan::joinPaths(FileMan::getTilecacheDirPath(), "*.jsd");

	// Loop through and set filenames
	SGP::FindFiles find(jsd_file_pattern.c_str());
	for (;;)
	{
		char const* const find_filename = find.Next();
		if (find_filename == NULL) break;

    std::string filename = FileMan::joinPaths(FileMan::getTilecacheDirPath(), find_filename);

		TILE_CACHE_STRUCT tc;
		GetRootName(tc.zRootName, lengthof(tc.zRootName), filename.c_str());
		tc.pStructureFileRef = LoadStructureFile(filename.c_str());

		if (strcasecmp(tc.zRootName, "l_dead1") == 0)
		{
			giDefaultStructIndex = (INT32)gpTileCacheStructInfo.Size();
		}

		gpTileCacheStructInfo.PushBack(tc);
	}
}


void DeleteTileCache( )
{
	UINT32 cnt;

	// Allocate entries
	if ( gpTileCache != NULL )
	{
		// Loop through and delete any entries
		for ( cnt = 0; cnt < guiMaxTileCacheSize; cnt++ )
		{
			if ( gpTileCache[ cnt ].pImagery != NULL )
			{
				DeleteTileSurface( gpTileCache[ cnt ].pImagery );
			}
		}
		MemFree( gpTileCache );
	}

	gpTileCacheStructInfo.Reserve(0);

	guiCurTileCacheSize = 0;
}


INT32 GetCachedTile(const char* const filename)
{
	INT32 idx = -1;

	// Check to see if surface exists already
	for (UINT32 cnt = 0; cnt < guiCurTileCacheSize; ++cnt)
	{
		TILE_CACHE_ELEMENT* const i = &gpTileCache[cnt];
		if (i->pImagery == NULL)
		{
			if (idx == -1) idx = cnt;
			continue;
		}

		if (strcasecmp(i->zName, filename) != 0) continue;

		// Found surface, return
		++i->sHits;
		return (INT32)cnt;
	}

	if (idx == -1)
	{
		if (guiCurTileCacheSize < guiMaxTileCacheSize)
		{
			idx = guiCurTileCacheSize++;
		}
		else
		{
			// cache out least used file
			idx = 0;
			INT16 sMostHits = gpTileCache[idx].sHits;
			for (UINT32 cnt = 1; cnt < guiCurTileCacheSize; ++cnt)
			{
				const TILE_CACHE_ELEMENT* const i = &gpTileCache[cnt];
				if (i->sHits < sMostHits)
				{
					sMostHits = i->sHits;
					idx       = cnt;
				}
			}

			// Bump off lowest index
			TILE_CACHE_ELEMENT* const del = &gpTileCache[idx];
			DeleteTileSurface(del->pImagery);
			del->sHits           = 0;
			del->pImagery        = 0;
			del->struct_file_ref = 0;
		}
	}

	TILE_CACHE_ELEMENT* const tce = &gpTileCache[idx];

	tce->pImagery = LoadTileSurface(filename);

	strcpy(tce->zName, filename);
	tce->sHits = 1;

	char root_name[30];
	GetRootName(root_name, lengthof(root_name), filename);
	STRUCTURE_FILE_REF* const sfr = GetCachedTileStructureRefFromFilename(root_name);
	tce->struct_file_ref = sfr;
	if (sfr) AddZStripInfoToVObject(tce->pImagery->vo, sfr, TRUE, 0);

	const AuxObjectData* const aux = tce->pImagery->pAuxData;
	tce->ubNumFrames = (aux != NULL ? aux->ubNumberOfFrames : 1);

	return idx;
}


void RemoveCachedTile(INT32 const cached_tile)
{
	if ((UINT32)cached_tile < guiCurTileCacheSize)
	{
		TILE_CACHE_ELEMENT& e = gpTileCache[cached_tile];
		if (e.pImagery)
		{
			if (--e.sHits != 0) return;

			DeleteTileSurface(e.pImagery);
			e.pImagery        = 0;
			e.struct_file_ref = 0;
			return;
		}
	}
	throw std::logic_error("Trying to remove invalid cached tile");
}


static STRUCTURE_FILE_REF* GetCachedTileStructureRef(INT32 const idx)
{
	return idx != -1 ? gpTileCache[idx].struct_file_ref : 0;
}


STRUCTURE_FILE_REF* GetCachedTileStructureRefFromFilename(char const* const filename)
{
	size_t const n = gpTileCacheStructInfo.Size();
	for (size_t i = 0; i != n; ++i)
	{
		TILE_CACHE_STRUCT& t = gpTileCacheStructInfo[i];
		if (strcasecmp(t.zRootName, filename) == 0) return t.pStructureFileRef;
	}
	return 0;
}


void CheckForAndAddTileCacheStructInfo(LEVELNODE* const pNode, INT16 const sGridNo, UINT16 const usIndex, UINT16 const usSubIndex)
{
	STRUCTURE_FILE_REF* const sfr = GetCachedTileStructureRef(usIndex);
	if (!sfr)  return;

	if (AddStructureToWorld(sGridNo, 0, &sfr->pDBStructureRef[usSubIndex], pNode)) return;

	if (giDefaultStructIndex == -1) return;

	STRUCTURE_FILE_REF* const def_sfr = gpTileCacheStructInfo[giDefaultStructIndex].pStructureFileRef;
	if (!def_sfr) return;

	AddStructureToWorld(sGridNo, 0, &def_sfr->pDBStructureRef[usSubIndex], pNode);
}


void CheckForAndDeleteTileCacheStructInfo( LEVELNODE *pNode, UINT16 usIndex )
{
	STRUCTURE_FILE_REF *pStructureFileRef;

	if ( usIndex >= TILE_CACHE_START_INDEX )
	{
		pStructureFileRef = GetCachedTileStructureRef( ( usIndex - TILE_CACHE_START_INDEX ) );

		if ( pStructureFileRef != NULL)
		{
			DeleteStructureFromWorld( pNode->pStructureData );
		}
	}
}


void GetRootName(char* const pDestStr, size_t const n, char const* const pSrcStr)
{
	// Remove path and extension
	ReplacePath(pDestStr, n, "", pSrcStr, "");
}
