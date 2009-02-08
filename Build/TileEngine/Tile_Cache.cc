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
		gpTileCache[i].pImagery     = NULL;
		gpTileCache[i].sStructRefID = -1;
	}

	// Look for JSD files in the tile cache directory and load any we find
	const char* const data_path = GetBinDataPath();
	char              jsd_file_pattern[512];
	snprintf(jsd_file_pattern, lengthof(jsd_file_pattern), "%s/Data/TILECACHE/*.jsd", data_path);

	// Loop through and set filenames
	SGP::FindFiles find(jsd_file_pattern);
	for (;;)
	{
		char const* const find_filename = find.Next();
		if (find_filename == NULL) break;

		char filename[150];
		sprintf(filename, "%s/Data/TILECACHE/%s", data_path, find_filename);

		TILE_CACHE_STRUCT tc;
		GetRootName(tc.zRootName, filename);
		tc.pStructureFileRef = LoadStructureFile(filename);

		if (strcasecmp(tc.zRootName, "l_dead1") == 0)
		{
			giDefaultStructIndex = gpTileCacheStructInfo.Size();
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


static INT16 FindCacheStructDataIndex(const char* cFilename)
{
	size_t const n = gpTileCacheStructInfo.Size();
	for (size_t i = 0; i != n; ++i)
	{
		if (strcasecmp(gpTileCacheStructInfo[i].zRootName, cFilename) == 0)
		{
			return (INT16)i;
		}
	}

	return( -1 );
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
			del->sHits        = 0;
			del->pImagery     = NULL;
			del->sStructRefID = -1;
		}
	}

	TILE_CACHE_ELEMENT* const tce = &gpTileCache[idx];

	tce->pImagery = LoadTileSurface(filename);

	strcpy(tce->zName, filename);
	tce->sHits = 1;

	char root_name[30];
	GetRootName(root_name, filename);
	tce->sStructRefID = FindCacheStructDataIndex(root_name);
	if (tce->sStructRefID != -1) // ATE: Add z-strip info
	{
		AddZStripInfoToVObject(tce->pImagery->vo, gpTileCacheStructInfo[tce->sStructRefID].pStructureFileRef, TRUE, 0);
	}

	const AuxObjectData* const aux = tce->pImagery->pAuxData;
	tce->ubNumFrames = (aux != NULL ? aux->ubNumberOfFrames : 1);

	return idx;
}


BOOLEAN RemoveCachedTile( INT32 iCachedTile )
{
	UINT32			cnt;

	// Find tile
	for ( cnt = 0; cnt < guiCurTileCacheSize; cnt++ )
	{
		if ( gpTileCache[ cnt ].pImagery != NULL )
		{
			if ( cnt == (UINT32)iCachedTile )
			{
				 // Found surface, decrement hits
				 gpTileCache[ cnt ].sHits--;

				 // Are we at zero?
				 if ( gpTileCache[ cnt ].sHits == 0 )
				 {
						DeleteTileSurface( gpTileCache[ cnt ].pImagery );
						gpTileCache[ cnt ].pImagery = NULL;
						gpTileCache[ cnt ].sStructRefID = -1;
						return TRUE;
				 }
			}
		}
	}

	return( FALSE );
}


static STRUCTURE_FILE_REF* GetCachedTileStructureRef(INT32 iIndex)
{
	if ( iIndex == -1 )
	{
		return( NULL );
	}

	if ( gpTileCache[ iIndex ].sStructRefID == -1 )
	{
		return( NULL );
	}

	return( gpTileCacheStructInfo[ gpTileCache[ iIndex ].sStructRefID ].pStructureFileRef );
}


STRUCTURE_FILE_REF* GetCachedTileStructureRefFromFilename(const char* cFilename)
{
	INT16 sStructDataIndex;

	// Given filename, look for index
	sStructDataIndex = FindCacheStructDataIndex( cFilename );

	if ( sStructDataIndex == -1 )
	{
		return( NULL );
	}

	return( gpTileCacheStructInfo[ sStructDataIndex ].pStructureFileRef );
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


void GetRootName(char* pDestStr, const char* pSrcStr)
{
	// Remove path and extension
	ReplacePath(pDestStr, (size_t)-1, "", pSrcStr, "");
}
