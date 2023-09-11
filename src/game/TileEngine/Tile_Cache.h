#ifndef __TILE_CACHE_H
#define __TILE_CACHE_H

#include "JA2Types.h"
#include <string_theory/string>


#define TILE_CACHE_START_INDEX 36000

struct TILE_CACHE_ELEMENT
{
	ST::string          zName; // Name of tile (filename and directory here)
	TILE_IMAGERY*       pImagery;   // Tile imagery
	INT16               sHits;
	UINT8               ubNumFrames;
	STRUCTURE_FILE_REF* struct_file_ref;
};


extern TILE_CACHE_ELEMENT* gpTileCache;


void InitTileCache(void);
void DeleteTileCache(void);

INT32 GetCachedTile(ST::string const& filename);
void  RemoveCachedTile(INT32 cached_tile);

STRUCTURE_FILE_REF* GetCachedTileStructureRefFromFilename(ST::string const& filename);

void CheckForAndAddTileCacheStructInfo( LEVELNODE *pNode, INT16 sGridNo, UINT16 usIndex, UINT16 usSubIndex );
void CheckForAndDeleteTileCacheStructInfo( LEVELNODE *pNode, UINT16 usIndex );


// OF COURSE, FOR SPEED, WE EXPORT OUR ARRAY
// ACCESS FUNCTIONS IN RENDERER IS NOT TOO NICE
// ATE



#endif
