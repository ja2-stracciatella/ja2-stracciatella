#include "ItemModel.h"
#include "Physics.h"
#include "Soldier_Find.h"
#include "TileDef.h"
#include "Timer_Control.h"
#include "WorldDef.h"
#include "Debug.h"
#include "WorldMan.h"
#include "RenderWorld.h"
#include "Overhead.h"
#include "Isometric_Utils.h"
#include "Tile_Animation.h"
#include "Tile_Cache.h"
#include "Explosion_Control.h"
#include "Weapons.h"
#include "Keys.h"
#include "Bullets.h"
#include "Rotting_Corpses.h"
#include "SmokeEffects.h"

#include "ContentManager.h"
#include "GameInstance.h"

static ANITILE* pAniTileHead = NULL;


static UINT16 SetFrameByDir(UINT16 frame, const ANITILE* const a)
{
	if (a->uiFlags & ANITILE_USE_DIRECTION_FOR_START_FRAME)
	{
		// Our start frame is actually a direction indicator
		const UINT8 ubTempDir = OneCDirection(a->v.user.uiData3);
		frame = a->usNumFrames * ubTempDir;
	}
	else if (a->uiFlags & ANITILE_USE_4DIRECTION_FOR_START_FRAME)
	{
		// Our start frame is actually a direction indicator
		const UINT8 ubTempDir = gb4DirectionsFrom8[a->v.user.uiData3];
		frame = a->usNumFrames * ubTempDir;
	}
	return frame;
}


ANITILE* CreateAnimationTile(const ANITILE_PARAMS* const parms)
{
	INT32                cached_tile = -1;
	INT16          const gridno      = parms->sGridNo;
	AnimationLevel const ubLevel     = parms->ubLevelID;
	UINT16               tile_index  = parms->usTileIndex;
	AnimationFlags const flags       = parms->uiFlags;
	LEVELNODE*           l           = parms->pGivenLevelNode;

	// Early validation: for non-cached tiles, verify animation data exists
	// before modifying any level node state. Cached tiles use gpTileCache
	// and do not need pAnimData.
	if (parms->zCachedFile == nullptr &&
		(tile_index >= NUMBEROFTILES || gTileDatabase[tile_index].pAnimData == nullptr))
	{
		SLOGW("CreateAnimationTile: tile {} (tileset {}) at gridno {} has no animation data",
			tile_index, giCurrentTilesetID, gridno);
		return nullptr;
	}

	ANITILE* const a = new ANITILE{};

	if (flags & ANITILE_EXISTINGTILE)
	{
		Assert(parms->zCachedFile == NULL);
		l->pAniTile = a;
	}
	else
	{
		if (parms->zCachedFile != NULL)
		{
			cached_tile = GetCachedTile(parms->zCachedFile);
			tile_index  = cached_tile + TILE_CACHE_START_INDEX;
		}

		// allocate new tile
		switch (ubLevel)
		{
			case ANI_STRUCT_LEVEL:  l = ForceStructToTail(gridno, tile_index); break;
			case ANI_SHADOW_LEVEL:  l = AddShadowToHead(  gridno, tile_index); break;
			case ANI_OBJECT_LEVEL:  l = AddObjectToHead(  gridno, tile_index); break;
			case ANI_ROOF_LEVEL:    l = AddRoofToHead(    gridno, tile_index); break;
			case ANI_ONROOF_LEVEL:  l = AddOnRoofToHead(  gridno, tile_index); break;
			case ANI_TOPMOST_LEVEL: l = AddTopmostToHead( gridno, tile_index); break;
			default: throw std::logic_error("Tried to create animation tile at invalid level");
		}

		// set new tile values
		l->ubShadeLevel        = DEFAULT_SHADE_LEVEL;
		l->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;

		if (cached_tile != -1)
		{
			l->uiFlags    |= LEVELNODE_CACHEDANITILE;
			l->pAniTile    = a;
			a->sRelativeX  = parms->sX;
			a->sRelativeY  = parms->sY;
			l->sRelativeZ  = parms->sZ;
		}
	}
	a->pLevelNode    = l;
	a->sCachedTileID = cached_tile;

	switch (ubLevel)
	{
		case ANI_STRUCT_LEVEL:  ResetSpecificLayerOptimizing(TILES_DYNAMIC_STRUCTURES); break;
		case ANI_SHADOW_LEVEL:  ResetSpecificLayerOptimizing(TILES_DYNAMIC_SHADOWS);    break;
		case ANI_OBJECT_LEVEL:  ResetSpecificLayerOptimizing(TILES_DYNAMIC_OBJECTS);    break;
		case ANI_ROOF_LEVEL:    ResetSpecificLayerOptimizing(TILES_DYNAMIC_ROOF);       break;
		case ANI_ONROOF_LEVEL:  ResetSpecificLayerOptimizing(TILES_DYNAMIC_ONROOF);     break;
		case ANI_TOPMOST_LEVEL: ResetSpecificLayerOptimizing(TILES_DYNAMIC_TOPMOST);    break;
		default:
			break;
	}

	// set flags for levelnode
	LevelnodeFlags lflags = l->uiFlags | LEVELNODE_ANIMATION | LEVELNODE_USEZ;
	lflags |= (flags & ANITILE_PAUSED ? LEVELNODE_LASTDYNAMIC | LEVELNODE_UPDATESAVEBUFFERONCE : LEVELNODE_DYNAMIC);
	if (flags & ANITILE_NOZBLITTER)         lflags |= LEVELNODE_NOZBLITTER;
	if (flags & ANITILE_ALWAYS_TRANSLUCENT) lflags |= LEVELNODE_REVEAL;
	if (flags & ANITILE_ANIMATE_Z)          lflags |= LEVELNODE_DYNAMICZ;
	l->uiFlags = lflags;

	// set anitile values
	if (cached_tile != -1)
	{
		a->usNumFrames = gpTileCache[cached_tile].ubNumFrames;
	}
	else
	{
		a->usNumFrames = gTileDatabase[tile_index].pAnimData->ubNumFrames;
	}
	a->ubLevelID        = ubLevel;
	a->usTileIndex      = tile_index;
	a->uiFlags          = flags;
	a->sDelay           = parms->sDelay;
	a->uiTimeLastUpdate = GetJA2Clock();
	a->sGridNo          = gridno;
	a->ubKeyFrame1      = parms->ubKeyFrame1;
	a->uiKeyFrame1Code  = parms->uiKeyFrame1Code;
	a->ubKeyFrame2      = parms->ubKeyFrame2;
	a->uiKeyFrame2Code  = parms->uiKeyFrame2Code;
	a->v                = parms->v;
	const INT16 start_frame = parms->sStartFrame + SetFrameByDir(0, a);
	a->sCurrentFrame    = start_frame;
	a->sStartFrame      = start_frame;
	a->pNext            = pAniTileHead;
	pAniTileHead = a;
	return a;
}


// Loop throug all ani tiles and remove...
void DeleteAniTiles( )
{
	ANITILE *pAniNode			= NULL;
	ANITILE *pNode				= NULL;

	// LOOP THROUGH EACH NODE
	// And call delete function...
	pAniNode = pAniTileHead;

	while( pAniNode != NULL )
	{
		pNode = pAniNode;
		pAniNode = pAniNode->pNext;

		DeleteAniTile( pNode );
	}
}


void DeleteAniTile(ANITILE* const a)
{
	for (ANITILE** anchor = &pAniTileHead;; anchor = &(*anchor)->pNext)
	{
		if (!*anchor)     return; // XXX exception?
		if (*anchor != a) continue;

		// Remove node from list
		*anchor = a->pNext;
		break;
	}

	if (a->uiFlags & ANITILE_EXISTINGTILE)
	{
		// update existing tile usIndex
		LEVELNODE& l = *a->pLevelNode;
		l.usIndex        = gTileDatabase[a->usTileIndex].pAnimData->pusFrames[l.sCurrentFrame];
		l.sCurrentFrame  = 0; // Set frame data back to zero
		l.uiFlags       &= ~(LEVELNODE_DYNAMIC | LEVELNODE_USEZ | LEVELNODE_ANIMATION);

		if (a->uiFlags & ANITILE_DOOR)
		{
			// Unset door busy
			DOOR_STATUS* const d = GetDoorStatus(a->sGridNo);
			if (d) d->ubFlags &= ~DOOR_BUSY;

			if (GridNoOnScreen(a->sGridNo)) SetRenderFlags(RENDER_FLAG_FULL);
		}
	}
	else
	{
		// Delete memory associated with item
		switch (a->ubLevelID)
		{
			case ANI_STRUCT_LEVEL:  RemoveStructFromLevelNode( a->sGridNo, a->pLevelNode);  break;
			case ANI_SHADOW_LEVEL:  RemoveShadowFromLevelNode( a->sGridNo, a->pLevelNode);  break;
			case ANI_OBJECT_LEVEL:  RemoveObject(              a->sGridNo, a->usTileIndex); break;
			case ANI_ROOF_LEVEL:    RemoveRoof(                a->sGridNo, a->usTileIndex); break;
			case ANI_ONROOF_LEVEL:  RemoveOnRoof(              a->sGridNo, a->usTileIndex); break;
			case ANI_TOPMOST_LEVEL: RemoveTopmostFromLevelNode(a->sGridNo, a->pLevelNode);  break;
		}

		if (a->sCachedTileID != -1) RemoveCachedTile(a->sCachedTileID);

		if (a->uiFlags & ANITILE_EXPLOSION)
		{
			// Talk to the explosion data
			EXPLOSIONTYPE* const e     = a->v.explosion;
			SOLDIERTYPE*   const owner = e->owner;
			RemoveExplosionData(e);

			if (!gfExplosionQueueActive)
			{ /* Turn on sighting again. The explosion queue handles all this at the
				 * end of the queue */
				gTacticalStatus.uiFlags &= ~DISALLOW_SIGHT;
			}

			// Freeup attacker from explosion
			ReduceAttackBusyCount(owner, FALSE);
		}


		if (a->uiFlags & ANITILE_RELEASE_ATTACKER_WHEN_DONE)
		{
			BULLET*      const bullet   = a->v.bullet;
			SOLDIERTYPE* const attacker = bullet->pFirer;
			// First delete the bullet!
			RemoveBullet(bullet);

			FreeUpAttacker(attacker);
		}
	}

	delete a;
}


void UpdateAniTiles( )
{
	ANITILE *pAniNode			= NULL;
	ANITILE *pNode				= NULL;
	UINT32	uiClock				= GetJA2Clock( );

	// LOOP THROUGH EACH NODE
	pAniNode = pAniTileHead;

	while( pAniNode != NULL )
	{
		pNode = pAniNode;
		pAniNode = pAniNode->pNext;

		if ( (uiClock - pNode->uiTimeLastUpdate ) > (UINT32)pNode->sDelay && !( pNode->uiFlags & ANITILE_PAUSED ) )
		{
			pNode->uiTimeLastUpdate = GetJA2Clock( );

			if ( pNode->uiFlags & ( ANITILE_OPTIMIZEFORSLOWMOVING ) )
			{
				pNode->pLevelNode->uiFlags |= (LEVELNODE_DYNAMIC );
				pNode->pLevelNode->uiFlags &= (~LEVELNODE_LASTDYNAMIC);
			}

			if ( pNode->uiFlags & ANITILE_FORWARD )
			{
				const UINT16 usMaxFrames = pNode->usNumFrames + SetFrameByDir(0, pNode);
				if ( ( pNode->sCurrentFrame + 1 ) < usMaxFrames )
				{
					pNode->sCurrentFrame++;
					pNode->pLevelNode->sCurrentFrame = pNode->sCurrentFrame;

					if ( pNode->uiFlags & ANITILE_EXPLOSION )
					{
						// Talk to the explosion data...
						UpdateExplosionFrame(pNode->v.explosion, pNode->sCurrentFrame);
					}

					// CHECK IF WE SHOULD BE DISPLAYING TRANSLUCENTLY!
					if ( pNode->sCurrentFrame == pNode->ubKeyFrame1 )
					{
						switch( pNode->uiKeyFrame1Code )
						{
							case ANI_KEYFRAME_BEGIN_TRANSLUCENCY:

								pNode->pLevelNode->uiFlags |= LEVELNODE_REVEAL;
								break;

							case ANI_KEYFRAME_CHAIN_WATER_EXPLOSION:
							{
								const REAL_OBJECT* const o = pNode->v.object;
								IgniteExplosionXY(o->owner, pNode->pLevelNode->sRelativeX, pNode->pLevelNode->sRelativeY, 0, pNode->sGridNo, o->Obj.usItem, 0);
								break;
							}

							case ANI_KEYFRAME_DO_SOUND:
								PlayLocationJA2Sample(pNode->sGridNo, pNode->v.sound, MIDVOLUME, 1);
								break;
						}

					}

					// CHECK IF WE SHOULD BE DISPLAYING TRANSLUCENTLY!
					if ( pNode->sCurrentFrame == pNode->ubKeyFrame2 )
					{
						switch( pNode->uiKeyFrame2Code )
						{
							case ANI_KEYFRAME_BEGIN_DAMAGE:
							{
								Assert(pNode->uiFlags & ANITILE_EXPLOSION);
								const EXPLOSIONTYPE* const e    = pNode->v.explosion;
								const UINT16               item = e->usItem;
								auto explosive = GCM->getExplosive(item);
								auto smokeEffect = explosive->getSmokeEffect();
								auto blastEffect = explosive->getBlastEffect();
								auto stunEffect = explosive->getStunEffect();

								if ( smokeEffect )
								{
									// Do sound....
									// PlayLocationJA2Sample(pNode->sGridNo, AIR_ESCAPING_1, HIGHVOLUME, 1);
									NewSmokeEffect(pNode->sGridNo, item, e->bLevel, e->owner);
								}
								if (blastEffect || stunEffect)
								{
									auto radius = 0;
									if (blastEffect) {
										radius = blastEffect->radius;
									}
									if (stunEffect && radius < stunEffect->radius) {
										radius = stunEffect->radius;
									}
									SpreadEffect(pNode->sGridNo, radius, item, e->owner, FALSE, e->bLevel, NULL);
								}
								// Forfait any other animations this frame....
								return;
							}
						}

					}

				}
				else
				{
					// We are done!
					if ( pNode->uiFlags & ANITILE_LOOPING )
					{
						pNode->sCurrentFrame = SetFrameByDir(pNode->sStartFrame, pNode);
					}
					else
					{
						// Delete from world!
						DeleteAniTile( pNode );

						// Turn back on redunency checks!
						gTacticalStatus.uiFlags &= (~NOHIDE_REDUNDENCY);

						return;
					}
				}
			}

			if ( pNode->uiFlags & ANITILE_BACKWARD )
			{
				if ( pNode->uiFlags & ANITILE_ERASEITEMFROMSAVEBUFFFER )
				{
					// ATE: Check if bounding box is on the screen...
					if ( pNode->bFrameCountAfterStart == 0 )
					{
						pNode->bFrameCountAfterStart = 1;
						pNode->pLevelNode->uiFlags |= (LEVELNODE_DYNAMIC );

						// Dangerous here, since we may not even be on the screen...
						SetRenderFlags( RENDER_FLAG_FULL );

						continue;
					}
				}

				const UINT16 usMinFrames = SetFrameByDir(0, pNode);
				if ( ( pNode->sCurrentFrame - 1 ) >= usMinFrames )
				{
					pNode->sCurrentFrame--;
					pNode->pLevelNode->sCurrentFrame = pNode->sCurrentFrame;

					if ( pNode->uiFlags & ANITILE_EXPLOSION )
					{
						// Talk to the explosion data...
						UpdateExplosionFrame(pNode->v.explosion, pNode->sCurrentFrame);
					}

				}
				else
				{
					// We are done!
					if ( pNode->uiFlags & ANITILE_PAUSE_AFTER_LOOP )
					{
						// Turn off backwards flag
						pNode->uiFlags &= (~ANITILE_BACKWARD );

						// Pause
						pNode->uiFlags |= ANITILE_PAUSED;

					}
					else if ( pNode->uiFlags & ANITILE_LOOPING )
					{
						pNode->sCurrentFrame = SetFrameByDir(pNode->sStartFrame, pNode);
					}
					else
					{
						// Delete from world!
						DeleteAniTile( pNode );

						return;
					}

					if ( pNode->uiFlags & ANITILE_ERASEITEMFROMSAVEBUFFFER )
					{
						// ATE: Check if bounding box is on the screen...
						pNode->bFrameCountAfterStart = 0;
						//pNode->pLevelNode->uiFlags |= LEVELNODE_UPDATESAVEBUFFERONCE;

						// Dangerous here, since we may not even be on the screen...
						SetRenderFlags( RENDER_FLAG_FULL );

					}

				}

			}

		}
		else
		{
			if ( pNode->uiFlags & ( ANITILE_OPTIMIZEFORSLOWMOVING ) )
			{
				// ONLY TURN OFF IF PAUSED...
				if ( ( pNode->uiFlags & ANITILE_ERASEITEMFROMSAVEBUFFFER ) )
				{
					if ( pNode->uiFlags & ANITILE_PAUSED )
					{
						if ( pNode->pLevelNode->uiFlags & LEVELNODE_DYNAMIC )
						{
							pNode->pLevelNode->uiFlags &= (~LEVELNODE_DYNAMIC );
							pNode->pLevelNode->uiFlags |= (LEVELNODE_LASTDYNAMIC);
							SetRenderFlags( RENDER_FLAG_FULL );
						}
					}
				}
				else
				{
					pNode->pLevelNode->uiFlags &= (~LEVELNODE_DYNAMIC );
					pNode->pLevelNode->uiFlags |= (LEVELNODE_LASTDYNAMIC);
				}
			}
		}
	}
}


ANITILE* GetCachedAniTileOfType(INT16 const sGridNo, UINT8 const ubLevelID, AnimationFlags const uiFlags)
{
	LEVELNODE*         n;
	MAP_ELEMENT const& me = gpWorldLevelData[sGridNo];
	switch (ubLevelID)
	{
		case ANI_STRUCT_LEVEL:  n = me.pStructHead;  break;
		case ANI_SHADOW_LEVEL:  n = me.pShadowHead;  break;
		case ANI_OBJECT_LEVEL:  n = me.pObjectHead;  break;
		case ANI_ROOF_LEVEL:    n = me.pRoofHead;    break;
		case ANI_ONROOF_LEVEL:  n = me.pOnRoofHead;  break;
		case ANI_TOPMOST_LEVEL: n = me.pTopmostHead; break;
		default: throw std::logic_error("Invalid level ID");
	}

	for (; n; n = n->pNext)
	{
		if (!(n->uiFlags & LEVELNODE_CACHEDANITILE)) continue;
		ANITILE* const a = n->pAniTile;
		if (a->uiFlags & uiFlags) return a;
	}

	return 0;
}


void HideAniTile( ANITILE *pAniTile, BOOLEAN fHide )
{
	if ( fHide )
	{
		pAniTile->pLevelNode->uiFlags |= LEVELNODE_HIDDEN;
	}
	else
	{
		pAniTile->pLevelNode->uiFlags &= (~LEVELNODE_HIDDEN );
	}
}
