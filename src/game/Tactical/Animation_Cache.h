#ifndef __ANIMATION_CACHE_H
#define __ANIMATION_CACHE_H

#include "Types.h"

#define DEFAULT_ANIM_CACHE_SIZE	3


struct AnimationSurfaceCacheType
{
	UINT16 *usCachedSurfaces;
	INT16  *sCacheHits;
	UINT8  ubCacheSize;
};

void GetCachedAnimationSurface(UINT16 usSoldierID, AnimationSurfaceCacheType* pAnimCache, UINT16 usSurfaceIndex, UINT16 usCurrentAnimation);
void InitAnimationCache(UINT16 usSoldierID, AnimationSurfaceCacheType*);
void DeleteAnimationCache( UINT16 usSoldierID, AnimationSurfaceCacheType *pAnimCache );
void UnLoadCachedAnimationSurfaces( UINT16 usSoldierID, AnimationSurfaceCacheType *pAnimCache );

#endif
