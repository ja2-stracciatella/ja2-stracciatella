#include "Animation_Cache.h"

#include "Animation_Control.h"
#include "Animation_Data.h"
#include "Logger.h"
#include "Overhead.h"
#include <climits>


void AnimationSurfaceCacheType::init(UINT8 const pid)
{
	mPid = pid;

	// Zero surface database history for this soldier
	ClearAnimationSurfacesUsageHistory( pid );
}


void AnimationSurfaceCacheType::cache(UINT16 const usSurfaceIndex, UINT16 const usCurrentAnimation)
{
	// This function assumes a cache size of at least 3. If you choose a smaller
	// cache size (why?) you must adjust this function.
	static_assert(ANIM_CACHE_SIZE >= 3);

	// Check to see if surface exists already
	int freeSlot = -1;
	for ( int cnt = 0; cnt < ANIM_CACHE_SIZE; cnt++ )
	{
		if ( usCachedSurfaces[ cnt ] == usSurfaceIndex )
		{
			// Found surface, return
			SLOGD("Anim Cache: Hit {} ( Soldier {} )", usSurfaceIndex, mPid);
			sCacheHits[cnt]++;
			return;
		}
		if ( freeSlot == -1 && usCachedSurfaces[ cnt ] == EMPTY_CACHE_ENTRY )
		{
			freeSlot = cnt;
		}
	}

	// Check if max size has been reached
	if ( freeSlot == -1 )
	{
		SLOGD("Anim Cache: Determining Bump Candidate ( Soldier {} )", mPid);

		// Determine existing surface used by merc
		SOLDIERTYPE* const pSoldier = &GetMan( mPid );
		UINT16 const usCurrentAnimSurface = DetermineSoldierAnimationSurface( pSoldier, usCurrentAnimation );
		// If the surface we are going to bump is our existing animation, reject it as a candidate

		// If we get here, we need to remove an animation, pick the best one
		// Loop through and pick one with lowest cache hits
		INT16 sLeastHits = INT16_MAX;
		for ( int cnt = 0; cnt < ANIM_CACHE_SIZE; cnt++ )
		{
			SLOGD("Anim Cache: Slot {} Hits {} ( Soldier {} )", cnt, sCacheHits[ cnt ], mPid);

			if ( usCachedSurfaces[ cnt ] == usCurrentAnimSurface )
			{
				SLOGD("Anim Cache: REJECTING Slot {} EXISTING ANIM SURFACE ( Soldier {}, Surface {} )", cnt, mPid, usCurrentAnimSurface);
			}
			else if ( usCachedSurfaces[cnt] == pSoldier->usAnimSurface )
			{
				// The result of DetermineSoldierAnimationSurface may be inconsistent
				// with the actual usAnimSurface, for example when player is re-assigning
				// mercs in strategic view.
				SLOGD("Anim Cache: REJECTING Slot {} IN-USE ANIM SURFACE ( Soldier {}, Surface {} )", cnt, mPid, pSoldier->usAnimSurface);
			}
			else
			{
				if ( sCacheHits[ cnt ] < sLeastHits )
				{
					sLeastHits = sCacheHits[ cnt ];
					freeSlot = cnt;
				}
			}
		}

		// Bump off lowest index
		SLOGD("Anim Cache: Bumping {} ( Soldier {} )", freeSlot, mPid);
		UnLoadAnimationSurface( mPid, usCachedSurfaces[ freeSlot ] );
		usCachedSurfaces[ freeSlot ] = EMPTY_CACHE_ENTRY;
	}

	// If here, Insert at an empty slot
	SLOGD("Anim Cache: Loading Surface {} ( Soldier {} )", usSurfaceIndex, mPid);

	// Insert here
	LoadAnimationSurface( mPid, usSurfaceIndex, usCurrentAnimation );
	sCacheHits[ freeSlot ] = 0;
	usCachedSurfaces[ freeSlot ] = usSurfaceIndex;
}


void AnimationSurfaceCacheType::free()
{
	// Check to see if surface exists already
	for (const UINT16 surface : usCachedSurfaces)
	{
		if ( surface != EMPTY_CACHE_ENTRY )
		{
			UnLoadAnimationSurface( mPid, surface );
		}
	}
}
