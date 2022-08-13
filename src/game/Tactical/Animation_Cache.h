#ifndef __ANIMATION_CACHE_H
#define __ANIMATION_CACHE_H

#include "JA2Types.h"

constexpr int ANIM_CACHE_SIZE = 3;
constexpr UINT16 EMPTY_CACHE_ENTRY = 65000;

class AnimationSurfaceCacheType
{
	// Ensure cache state is valid even if init is never called
	// (the special soldiers created by Auto Resolve)
	UINT16    usCachedSurfaces[ANIM_CACHE_SIZE]{ EMPTY_CACHE_ENTRY, EMPTY_CACHE_ENTRY, EMPTY_CACHE_ENTRY };
	INT16     sCacheHits[ANIM_CACHE_SIZE]{ 0, 0, 0 };
	UINT8     mPid{ 0 };

public:
	// Load an animation surface if it is not already cached.
	void cache(UINT16 usSurfaceIndex, UINT16 usCurrentAnimation);
	// Init the animation cache for the specified soldier.
	void init(UINT8 usSoldierID);
	// Unload all cached animation surfaces.
	void free();
};

#endif
