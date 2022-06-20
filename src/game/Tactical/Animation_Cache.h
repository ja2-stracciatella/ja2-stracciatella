#ifndef __ANIMATION_CACHE_H
#define __ANIMATION_CACHE_H

#include "JA2Types.h"

constexpr int ANIM_CACHE_SIZE = 3;


class AnimationSurfaceCacheType
{
	UINT16    usCachedSurfaces[ANIM_CACHE_SIZE];
	INT16     sCacheHits[ANIM_CACHE_SIZE];
	ProfileID mPid;

public:
	// Load an animation surface if it is not already cached.
	void cache(UINT16 usSurfaceIndex, UINT16 usCurrentAnimation);
	// Init the animation cache for the specified soldier.
	void init(ProfileID usSoldierID);
	// Unload all cached animation surfaces.
	void free();
};

#endif
