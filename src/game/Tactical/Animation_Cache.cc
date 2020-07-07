#include "Animation_Cache.h"

#include "Animation_Control.h"
#include "Animation_Data.h"
#include "Logger.h"
#include "MemMan.h"
#include "Overhead.h"
#include "Types.h"
#include <climits>

#define EMPTY_CACHE_ENTRY 65000

static const UINT32 guiCacheSize = DEFAULT_ANIM_CACHE_SIZE;


void InitAnimationCache(UINT16 const usSoldierID, AnimationSurfaceCacheType* const pAnimCache)
{
	UINT32 cnt;

	// Allocate entries
	SLOGD("Initializing anim cache surface for soldier %d", usSoldierID);
	pAnimCache->usCachedSurfaces = new UINT16[guiCacheSize]{};

	SLOGD("Initializing anim cache hit counter for soldier %d", usSoldierID);
	pAnimCache->sCacheHits = new INT16[guiCacheSize]{};

	// Zero entries
	for ( cnt = 0; cnt < guiCacheSize; cnt++ )
	{
		pAnimCache->usCachedSurfaces[ cnt ] = EMPTY_CACHE_ENTRY;
		pAnimCache->sCacheHits[ cnt ] = 0;
	}
	pAnimCache->ubCacheSize = 0;

	// Zero surface databse history for this soldeir
	ClearAnimationSurfacesUsageHistory( usSoldierID );
}


void DeleteAnimationCache( UINT16 usSoldierID, AnimationSurfaceCacheType *pAnimCache )
{
	// Allocate entries
	if ( pAnimCache->usCachedSurfaces!= NULL )
	{
		SLOGD("Removing Anim Cache surface for soldier %d", usSoldierID);
		delete[] pAnimCache->usCachedSurfaces;
	}

	if ( pAnimCache->sCacheHits != NULL )
	{
		SLOGD("Removing Anim Cache hit counter for soldier %d", usSoldierID);
		delete[] pAnimCache->sCacheHits;
	}
}


void GetCachedAnimationSurface(UINT16 const usSoldierID, AnimationSurfaceCacheType* const pAnimCache, UINT16 const usSurfaceIndex, UINT16 const usCurrentAnimation)
{
	UINT8  cnt;
	UINT8  ubLowestIndex = -1;
	INT16  sMostHits = INT16_MAX;
	UINT16 usCurrentAnimSurface;

	// Check to see if surface exists already
	for ( cnt = 0; cnt < pAnimCache->ubCacheSize; cnt++ )
	{
		if ( pAnimCache->usCachedSurfaces[ cnt ] == usSurfaceIndex )
		{
			// Found surface, return
			SLOGD("Anim Cache: Hit %d ( Soldier %d )", usSurfaceIndex, usSoldierID);
			pAnimCache->sCacheHits[cnt]++;
			return;
		}
	}

	// Check if max size has been reached
	if ( pAnimCache->ubCacheSize == guiCacheSize )
	{
		SLOGD("Anim Cache: Determining Bump Candidate ( Soldier %d )", usSoldierID);

		// Determine existing surface used by merc
		SOLDIERTYPE* pSoldier = &GetMan(usSoldierID);
		usCurrentAnimSurface = DetermineSoldierAnimationSurface(pSoldier, usCurrentAnimation);
		// If the surface we are going to bump is our existing animation, reject it as a candidate

		// If we get here, we need to remove an animation, pick the best one
		// Loop through and pick one with lowest cache hits
		for ( cnt = 0; cnt < pAnimCache->ubCacheSize; cnt++ )
		{
			SLOGD("Anim Cache: Slot %d Hits %d ( Soldier %d )", cnt, pAnimCache->sCacheHits[ cnt ], usSoldierID);

			if ( pAnimCache->usCachedSurfaces[ cnt ] == usCurrentAnimSurface )
			{
				SLOGD(ST::format("Anim Cache: REJECTING Slot {} EXISTING ANIM SURFACE ( Soldier {}, Surface {} )", cnt, usSoldierID, usCurrentAnimSurface));
			}
			else if (pAnimCache->usCachedSurfaces[cnt] == pSoldier->usAnimSurface)
			{
				// The result of DetermineSoldierAnimationSurface may be inconsistent
				// with the actual usAnimSurface, for example when player is re-assigning
				// mercs in strategic view.
				SLOGD(ST::format("Anim Cache: REJECTING Slot {} IN-USE ANIM SURFACE ( Soldier {}, Surface {} )", cnt, usSoldierID, pSoldier->usAnimSurface));
			}
			else
			{
				if ( pAnimCache->sCacheHits[ cnt ] < sMostHits )
				{
					sMostHits = pAnimCache->sCacheHits[ cnt ];
					ubLowestIndex = cnt;
				}
			}
		}

		if (ubLowestIndex != -1)
		{
			// Bump off lowest index
			SLOGD("Anim Cache: Bumping %d ( Soldier %d )", ubLowestIndex, usSoldierID);
			UnLoadAnimationSurface( usSoldierID, pAnimCache->usCachedSurfaces[ ubLowestIndex ] );

			// Decrement
			pAnimCache->sCacheHits[ ubLowestIndex ] = 0;
			pAnimCache->usCachedSurfaces[ ubLowestIndex ] = EMPTY_CACHE_ENTRY;
			pAnimCache->ubCacheSize--;
		}
		else
		{
			SLOGW("Anim Cache: No slots can be evicted");
		}
	}

	// If here, Insert at an empty slot
	// Find an empty slot
	for ( cnt = 0; cnt < guiCacheSize; cnt++ )
	{
		if ( pAnimCache->usCachedSurfaces[ cnt ] == EMPTY_CACHE_ENTRY )
		{
			SLOGD("Anim Cache: Loading Surface %d ( Soldier %d )", usSurfaceIndex, usSoldierID);

			// Insert here
			LoadAnimationSurface(usSoldierID, usSurfaceIndex, usCurrentAnimation);
			pAnimCache->sCacheHits[ cnt ] = 0;
			pAnimCache->usCachedSurfaces[ cnt ] = usSurfaceIndex;
			pAnimCache->ubCacheSize++;

			return;
		}
	}

	SLOGW(ST::format("Anim Cache: Failed to find slot to load surface ( Soldier {} )", usSoldierID));
}


void UnLoadCachedAnimationSurfaces( UINT16 usSoldierID, AnimationSurfaceCacheType *pAnimCache )
{
	UINT8 cnt;

	// Check to see if surface exists already
	for ( cnt = 0; cnt < pAnimCache->ubCacheSize; cnt++ )
	{
		if ( pAnimCache->usCachedSurfaces[ cnt ] != EMPTY_CACHE_ENTRY )
		{
			UnLoadAnimationSurface( usSoldierID, pAnimCache->usCachedSurfaces[ cnt ] );
		}
	}

}
