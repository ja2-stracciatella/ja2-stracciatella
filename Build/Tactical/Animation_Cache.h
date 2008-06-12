#ifndef __ANIMATION_CACHE_H
#define __ANIMATION_CACHE_H

#include "Types.h"

#define MAX_CACHE_SIZE		20
#define MIN_CACHE_SIZE		2


typedef struct
{
	UINT16	*usCachedSurfaces;
	INT16		*sCacheHits;
	UINT8		ubCacheSize;

} AnimationSurfaceCacheType;

BOOLEAN GetCachedAnimationSurface( UINT16 usSoldierID, AnimationSurfaceCacheType *pAnimCache, UINT16 usSurfaceIndex, UINT16 usCurrentAnimation );
void InitAnimationCache(UINT16 usSoldierID, AnimationSurfaceCacheType*);
void DeleteAnimationCache( UINT16 usSoldierID, AnimationSurfaceCacheType *pAnimCache );
void UnLoadCachedAnimationSurfaces( UINT16 usSoldierID, AnimationSurfaceCacheType *pAnimCache );

#endif
