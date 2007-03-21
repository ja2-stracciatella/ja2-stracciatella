#include "WorldDef.h"
#include <stdio.h>
#include <string.h>
#include "WCheck.h"
#include "stdlib.h"
#include "time.h"
#include "Debug.h"
#include "Smooth.h"
#include "WorldMan.h"
#include "Lighting.h"
#include "RenderWorld.h"
#include "Overhead.h"
#include "AI.h"
#include "Sound_Control.h"
#include "Animation_Control.h"
#include "Isometric_Utils.h"
#include "Font_Control.h"
#include "Message.h"
#include "Tile_Animation.h"
#include "Tile_Cache.h"
#include "Explosion_Control.h"
#include "Weapons.h"
#include "Keys.h"
#include "Bullets.h"
#include "Rotting_Corpses.h"
#include "SmokeEffects.h"
#include "MemMan.h"


ANITILE					*pAniTileHead = NULL;


ANITILE *CreateAnimationTile( ANITILE_PARAMS *pAniParams )
{
	ANITILE		*pAniNode;
	ANITILE		*pNewAniNode;
	LEVELNODE	*pNode;
	INT32			iCachedTile=-1;
	INT16			sGridNo;
	UINT8			ubLevel;
	INT16			usTileType;
	INT16			usTileIndex;
	INT16			sDelay;
	INT16			sStartFrame=-1;
	UINT32		uiFlags;
	LEVELNODE	*pGivenNode;
	INT16			sX, sY, sZ;
	UINT8			ubTempDir;

	// Get some parameters from structure sent in...
	sGridNo			= pAniParams->sGridNo;
	ubLevel			= pAniParams->ubLevelID;
	usTileType	= pAniParams->usTileType;
	usTileIndex	= pAniParams->usTileIndex;
	sDelay			= pAniParams->sDelay;
	sStartFrame	= pAniParams->sStartFrame;
	uiFlags			= pAniParams->uiFlags;
	pGivenNode	= pAniParams->pGivenLevelNode;
	sX					= pAniParams->sX;
	sY					= pAniParams->sY;
	sZ					= pAniParams->sZ;


	pAniNode = pAniTileHead;

	// Allocate head
	pNewAniNode = MemAlloc( sizeof( ANITILE ) );

	if ( (uiFlags & ANITILE_EXISTINGTILE  ) )
	{
		pNewAniNode->pLevelNode						= pGivenNode;
		pNewAniNode->pLevelNode->pAniTile = pNewAniNode;
	}
	else
	{
		if ( ( uiFlags & ANITILE_CACHEDTILE ) )
		{
			iCachedTile = GetCachedTile( pAniParams->zCachedFile );

			if ( iCachedTile == -1 )
			{
				return( NULL );
			}

			usTileIndex = iCachedTile + TILE_CACHE_START_INDEX;
		}

		// ALLOCATE NEW TILE
		switch( ubLevel )
		{
			case ANI_STRUCT_LEVEL:

				pNode = ForceStructToTail( sGridNo, usTileIndex );
				break;

			case ANI_SHADOW_LEVEL:

				AddShadowToHead( sGridNo, usTileIndex );
				pNode = gpWorldLevelData[ sGridNo ].pShadowHead;
				break;

			case ANI_OBJECT_LEVEL:

				AddObjectToHead( sGridNo, usTileIndex );
				pNode = gpWorldLevelData[ sGridNo ].pObjectHead;
				break;

			case ANI_ROOF_LEVEL:

				AddRoofToHead( sGridNo, usTileIndex );
				pNode = gpWorldLevelData[ sGridNo ].pRoofHead;
				break;

			case ANI_ONROOF_LEVEL:

				AddOnRoofToHead( sGridNo, usTileIndex );
				pNode = gpWorldLevelData[ sGridNo ].pOnRoofHead;
				break;

			case ANI_TOPMOST_LEVEL:

				AddTopmostToHead( sGridNo, usTileIndex );
				pNode = gpWorldLevelData[ sGridNo ].pTopmostHead;
				break;

			default:

				return( NULL );
		}

		// SET NEW TILE VALUES
		pNode->ubShadeLevel=DEFAULT_SHADE_LEVEL;
		pNode->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;

		pNewAniNode->pLevelNode								= pNode;

		if ( ( uiFlags & ANITILE_CACHEDTILE ) )
		{
			pNewAniNode->pLevelNode->uiFlags |=	( LEVELNODE_CACHEDANITILE );
			pNewAniNode->sCachedTileID = (INT16)iCachedTile;
			pNewAniNode->usCachedTileSubIndex = usTileType;
			pNewAniNode->pLevelNode->pAniTile = pNewAniNode;
			pNewAniNode->sRelativeX		= sX;
			pNewAniNode->sRelativeY		= sY;
			pNewAniNode->pLevelNode->sRelativeZ		= sZ;

		}
		// Can't set relative X,Y,Z IF FLAGS ANITILE_CACHEDTILE set!
		else if ( (uiFlags & ANITILE_USEABSOLUTEPOS  ) )
		{
			pNewAniNode->pLevelNode->sRelativeX		= sX;
			pNewAniNode->pLevelNode->sRelativeY		= sY;
			pNewAniNode->pLevelNode->sRelativeZ		= sZ;
			pNewAniNode->pLevelNode->uiFlags |=	( LEVELNODE_USEABSOLUTEPOS );
		}

	}


	switch( ubLevel )
	{
		case ANI_STRUCT_LEVEL:

			ResetSpecificLayerOptimizing( TILES_DYNAMIC_STRUCTURES );
			break;

		case ANI_SHADOW_LEVEL:

			ResetSpecificLayerOptimizing( TILES_DYNAMIC_SHADOWS );
			break;

		case ANI_OBJECT_LEVEL:

			ResetSpecificLayerOptimizing( TILES_DYNAMIC_OBJECTS );
			break;

		case ANI_ROOF_LEVEL:

			ResetSpecificLayerOptimizing( TILES_DYNAMIC_ROOF );
			break;

		case ANI_ONROOF_LEVEL:

			ResetSpecificLayerOptimizing( TILES_DYNAMIC_ONROOF );
			break;

		case ANI_TOPMOST_LEVEL:

			ResetSpecificLayerOptimizing( TILES_DYNAMIC_TOPMOST );
			break;

	}

	// SET FLAGS FOR LEVELNODE
	pNewAniNode->pLevelNode->uiFlags |=	( LEVELNODE_ANIMATION | LEVELNODE_USEZ | LEVELNODE_DYNAMIC );

	if ( ( uiFlags & ANITILE_NOZBLITTER ) )
	{
		pNewAniNode->pLevelNode->uiFlags |= LEVELNODE_NOZBLITTER;
	}

	if ( ( uiFlags & ANITILE_ALWAYS_TRANSLUCENT ) )
	{
		pNewAniNode->pLevelNode->uiFlags |= LEVELNODE_REVEAL;
	}

	if ( ( uiFlags & ANITILE_USEBEST_TRANSLUCENT ) )
	{
		pNewAniNode->pLevelNode->uiFlags |= LEVELNODE_USEBESTTRANSTYPE;
	}

	if ( ( uiFlags & ANITILE_ANIMATE_Z ) )
	{
		pNewAniNode->pLevelNode->uiFlags |= LEVELNODE_DYNAMICZ;
	}

	if ( ( uiFlags & ANITILE_PAUSED ) )
	{
		pNewAniNode->pLevelNode->uiFlags |= ( LEVELNODE_LASTDYNAMIC | LEVELNODE_UPDATESAVEBUFFERONCE );
		pNewAniNode->pLevelNode->uiFlags &= (~LEVELNODE_DYNAMIC );
	}

	if ( ( uiFlags & ANITILE_OPTIMIZEFORSMOKEEFFECT ) )
	{
		pNewAniNode->pLevelNode->uiFlags |= LEVELNODE_NOWRITEZ;
	}


	// SET ANITILE VALUES
	pNewAniNode->ubLevelID				= ubLevel;
	pNewAniNode->usTileIndex			= usTileIndex;

	if ( ( uiFlags & ANITILE_CACHEDTILE ) )
	{
		pNewAniNode->usNumFrames			= gpTileCache[ iCachedTile ].ubNumFrames;
	}
	else
	{
		Assert( gTileDatabase[ usTileIndex ].pAnimData != NULL );
		pNewAniNode->usNumFrames			= gTileDatabase[ usTileIndex ].pAnimData->ubNumFrames;
	}

	if ( ( uiFlags & ANITILE_USE_DIRECTION_FOR_START_FRAME ) )
	{
		// Our start frame is actually a direction indicator
		ubTempDir = gOneCDirection[ pAniParams->uiUserData3 ];
		sStartFrame = (UINT16)sStartFrame + ( pNewAniNode->usNumFrames * ubTempDir );
	}

	if ( ( uiFlags & ANITILE_USE_4DIRECTION_FOR_START_FRAME ) )
	{
		// Our start frame is actually a direction indicator
		ubTempDir = gb4DirectionsFrom8[ pAniParams->uiUserData3 ];
		sStartFrame = (UINT16)sStartFrame + ( pNewAniNode->usNumFrames * ubTempDir );
	}

	pNewAniNode->usTileType				= usTileType;
	pNewAniNode->pNext						= pAniNode;
	pNewAniNode->uiFlags					= uiFlags;
	pNewAniNode->sDelay						= sDelay;
	pNewAniNode->sCurrentFrame		= sStartFrame;
	pNewAniNode->uiTimeLastUpdate = GetJA2Clock( );
	pNewAniNode->sGridNo					= sGridNo;

	pNewAniNode->sStartFrame      = sStartFrame;

	pNewAniNode->ubKeyFrame1			= pAniParams->ubKeyFrame1;
	pNewAniNode->uiKeyFrame1Code	= pAniParams->uiKeyFrame1Code;
	pNewAniNode->ubKeyFrame2			= pAniParams->ubKeyFrame2;
	pNewAniNode->uiKeyFrame2Code	= pAniParams->uiKeyFrame2Code;
	pNewAniNode->uiUserData				= pAniParams->uiUserData;
	pNewAniNode->ubUserData2			= pAniParams->ubUserData2;
	pNewAniNode->uiUserData3			= pAniParams->uiUserData3;


	//Set head
	pAniTileHead = pNewAniNode;

	// Set some special stuff
	return( pNewAniNode );
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


void DeleteAniTile( ANITILE *pAniTile )
{
	ANITILE				*pAniNode				= NULL;
	ANITILE				*pOldAniNode		= NULL;
	TILE_ELEMENT	*TileElem;

	pAniNode = pAniTileHead;

	while( pAniNode!= NULL )
	{
		if ( pAniNode == pAniTile )
		{
			// OK, set links
			// Check for head or tail
			if ( pOldAniNode == NULL )
			{
				// It's the head
				pAniTileHead = pAniTile->pNext;
			}
			else
			{
				pOldAniNode->pNext = pAniNode->pNext;
			}

			if ( !(pAniNode->uiFlags & ANITILE_EXISTINGTILE  ) )
			{

				// Delete memory assosiated with item
				switch( pAniNode->ubLevelID )
				{
					case ANI_STRUCT_LEVEL:

						RemoveStructFromLevelNode( pAniNode->sGridNo, pAniNode->pLevelNode );
						break;

					case ANI_SHADOW_LEVEL:

						RemoveShadowFromLevelNode( pAniNode->sGridNo, pAniNode->pLevelNode );
						break;

					case ANI_OBJECT_LEVEL:

						RemoveObject( pAniNode->sGridNo, pAniNode->usTileIndex );
						break;

					case ANI_ROOF_LEVEL:

						RemoveRoof( pAniNode->sGridNo, pAniNode->usTileIndex );
						break;

					case ANI_ONROOF_LEVEL:

						RemoveOnRoof( pAniNode->sGridNo, pAniNode->usTileIndex );
						break;

					case ANI_TOPMOST_LEVEL:

						RemoveTopmostFromLevelNode( pAniNode->sGridNo, pAniNode->pLevelNode );
						break;

				}

				if ( ( pAniNode->uiFlags & ANITILE_CACHEDTILE ) )
				{
					RemoveCachedTile( pAniNode->sCachedTileID );
				}

				if ( pAniNode->uiFlags & ANITILE_EXPLOSION )
				{
					// Talk to the explosion data...
					RemoveExplosionData( pAniNode->uiUserData3 );

					if ( !gfExplosionQueueActive )
					{
						// turn on sighting again
						// the explosion queue handles all this at the end of the queue
						gTacticalStatus.uiFlags &= (~DISALLOW_SIGHT);
					}

          // Freeup attacker from explosion
		      DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("@@@@@@@ Reducing attacker busy count..., EXPLOSION effect gone off") );
		      ReduceAttackBusyCount( (UINT8)pAniNode->ubUserData2, FALSE );

				}


				if ( pAniNode->uiFlags & ANITILE_RELEASE_ATTACKER_WHEN_DONE )
				{
					// First delete the bullet!
					RemoveBullet( pAniNode->uiUserData3 );

					DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("@@@@@@@ Freeing up attacker - miss finished animation") );
					FreeUpAttacker( (UINT8) pAniNode->ubAttackerMissed );
				}

			}
			else
			{
				TileElem = &( gTileDatabase[ pAniNode->usTileIndex ] );

				// OK, update existing tile usIndex....
				Assert( TileElem->pAnimData != NULL );
				pAniNode->pLevelNode->usIndex = TileElem->pAnimData->pusFrames[ pAniNode->pLevelNode->sCurrentFrame ];

				// OK, set our frame data back to zero....
				pAniNode->pLevelNode->sCurrentFrame = 0;

				// Set some flags to write to Z / update save buffer
				// pAniNode->pLevelNode->uiFlags |=( LEVELNODE_LASTDYNAMIC | LEVELNODE_UPDATESAVEBUFFERONCE );
				pAniNode->pLevelNode->uiFlags &= ~( LEVELNODE_DYNAMIC | LEVELNODE_USEZ | LEVELNODE_ANIMATION );

				if (pAniNode->uiFlags & ANITILE_DOOR)
				{
					// unset door busy!
					DOOR_STATUS * pDoorStatus;

					pDoorStatus = GetDoorStatus( pAniNode->sGridNo );
					if (pDoorStatus)
					{
						pDoorStatus->ubFlags &= ~(DOOR_BUSY);
					}

					if ( GridNoOnScreen( pAniNode->sGridNo ) )
					{
						SetRenderFlags(RENDER_FLAG_FULL);
					}

				}
			}

			MemFree( pAniNode );
			return;
		}

		pOldAniNode = pAniNode;
		pAniNode		= pAniNode->pNext;

	}


}



void UpdateAniTiles( )
{
	ANITILE *pAniNode			= NULL;
	ANITILE *pNode				= NULL;
	UINT32	uiClock				= GetJA2Clock( );
	UINT16	usMaxFrames, usMinFrames;
	UINT8		ubTempDir;

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
			else if ( pNode->uiFlags & ( ANITILE_OPTIMIZEFORSMOKEEFFECT ) )
			{
			//	pNode->pLevelNode->uiFlags |= LEVELNODE_DYNAMICZ;
				ResetSpecificLayerOptimizing( TILES_DYNAMIC_STRUCTURES );
				pNode->pLevelNode->uiFlags &= (~LEVELNODE_LASTDYNAMIC);
				pNode->pLevelNode->uiFlags |= (LEVELNODE_DYNAMIC );
			}

			if ( pNode->uiFlags & ANITILE_FORWARD )
			{
				usMaxFrames = pNode->usNumFrames;

				if ( pNode->uiFlags & ANITILE_USE_DIRECTION_FOR_START_FRAME )
				{
					ubTempDir = gOneCDirection[ pNode->uiUserData3 ];
					usMaxFrames = (UINT16)usMaxFrames + ( pNode->usNumFrames * ubTempDir );
				}

				if ( pNode->uiFlags & ANITILE_USE_4DIRECTION_FOR_START_FRAME )
				{
					ubTempDir = gb4DirectionsFrom8[ pNode->uiUserData3 ];
					usMaxFrames = (UINT16)usMaxFrames + ( pNode->usNumFrames * ubTempDir );
				}

				if ( ( pNode->sCurrentFrame + 1 ) < usMaxFrames )
				{
					pNode->sCurrentFrame++;
					pNode->pLevelNode->sCurrentFrame = pNode->sCurrentFrame;

					if ( pNode->uiFlags & ANITILE_EXPLOSION )
					{
						// Talk to the explosion data...
						UpdateExplosionFrame( pNode->uiUserData3, pNode->sCurrentFrame );
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

								IgniteExplosion( pNode->ubUserData2, pNode->pLevelNode->sRelativeX, pNode->pLevelNode->sRelativeY, 0, pNode->sGridNo, (UINT16)( pNode->uiUserData ), 0 );
								break;

              case ANI_KEYFRAME_DO_SOUND:

                PlayJA2Sample(pNode->uiUserData, SoundVolume(MIDVOLUME, (INT16)pNode->uiUserData3), 1, SoundDir((INT16)pNode->uiUserData3));
                break;
						}

					}

					// CHECK IF WE SHOULD BE DISPLAYING TRANSLUCENTLY!
					if ( pNode->sCurrentFrame == pNode->ubKeyFrame2 )
					{
            UINT8     ubExpType;

           	switch( pNode->uiKeyFrame2Code )
						{
							case ANI_KEYFRAME_BEGIN_DAMAGE:

                ubExpType = Explosive[ Item[ (UINT16)pNode->uiUserData ].ubClassIndex ].ubType;

                if ( ubExpType == EXPLOSV_TEARGAS || ubExpType == EXPLOSV_MUSTGAS ||
                     ubExpType == EXPLOSV_SMOKE )
                {
                  // Do sound....
                  // PlayJA2Sample(AIR_ESCAPING_1, SoundVolume(HIGHVOLUME, pNode->sGridNo), 1, SoundDir(pNode->sGridNo));
		              NewSmokeEffect( pNode->sGridNo, (UINT16)pNode->uiUserData, gExplosionData[ pNode->uiUserData3 ].Params.bLevel, (UINT8)pNode->ubUserData2 );
                }
                else
                {
    							SpreadEffect( pNode->sGridNo, Explosive[ Item[ (UINT16)pNode->uiUserData ].ubClassIndex ].ubRadius, (UINT16)pNode->uiUserData, (UINT8)pNode->ubUserData2, FALSE, gExplosionData[ pNode->uiUserData3 ].Params.bLevel, -1 );
                }
								// Forfait any other animations this frame....
								return;
						}

					}

				}
				else
				{
					// We are done!
					if ( pNode->uiFlags & ANITILE_LOOPING )
					{
						pNode->sCurrentFrame = pNode->sStartFrame;

						if ( ( pNode->uiFlags & ANITILE_USE_DIRECTION_FOR_START_FRAME ) )
						{
							// Our start frame is actually a direction indicator
							ubTempDir = gOneCDirection[ pNode->uiUserData3 ];
							pNode->sCurrentFrame = (UINT16)( pNode->usNumFrames * ubTempDir );
						}

						if ( ( pNode->uiFlags & ANITILE_USE_4DIRECTION_FOR_START_FRAME ) )
						{
							// Our start frame is actually a direction indicator
							ubTempDir = gb4DirectionsFrom8[ pNode->uiUserData3 ];
							pNode->sCurrentFrame = (UINT16)( pNode->usNumFrames * ubTempDir );
						}

					}
					else if ( pNode->uiFlags & ANITILE_REVERSE_LOOPING )
					{
						// Turn off backwards flag
						pNode->uiFlags &= (~ANITILE_FORWARD );

						// Turn onn forwards flag
						pNode->uiFlags |= ANITILE_BACKWARD;
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

				usMinFrames = 0;

				if ( pNode->uiFlags & ANITILE_USE_DIRECTION_FOR_START_FRAME )
				{
					ubTempDir = gOneCDirection[ pNode->uiUserData3 ];
					usMinFrames = ( pNode->usNumFrames * ubTempDir );
				}

				if ( pNode->uiFlags & ANITILE_USE_4DIRECTION_FOR_START_FRAME )
				{
					ubTempDir = gb4DirectionsFrom8[ pNode->uiUserData3 ];
					usMinFrames = ( pNode->usNumFrames * ubTempDir );
				}

				if ( ( pNode->sCurrentFrame - 1 ) >= usMinFrames )
				{
					pNode->sCurrentFrame--;
					pNode->pLevelNode->sCurrentFrame = pNode->sCurrentFrame;

					if ( pNode->uiFlags & ANITILE_EXPLOSION )
					{
						// Talk to the explosion data...
						UpdateExplosionFrame( pNode->uiUserData3, pNode->sCurrentFrame );
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
						pNode->sCurrentFrame = pNode->sStartFrame;

						if ( ( pNode->uiFlags & ANITILE_USE_DIRECTION_FOR_START_FRAME ) )
						{
							// Our start frame is actually a direction indicator
							ubTempDir = gOneCDirection[ pNode->uiUserData3 ];
							pNode->sCurrentFrame = (UINT16)( pNode->usNumFrames * ubTempDir );
						}
						if ( ( pNode->uiFlags & ANITILE_USE_4DIRECTION_FOR_START_FRAME ) )
						{
							// Our start frame is actually a direction indicator
							ubTempDir = gb4DirectionsFrom8[ pNode->uiUserData3 ];
							pNode->sCurrentFrame = (UINT16)( pNode->usNumFrames * ubTempDir );
						}

					}
					else if ( pNode->uiFlags & ANITILE_REVERSE_LOOPING )
					{
						// Turn off backwards flag
						pNode->uiFlags &= (~ANITILE_BACKWARD );

						// Turn onn forwards flag
						pNode->uiFlags |= ANITILE_FORWARD;
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
			else if ( pNode->uiFlags & ( ANITILE_OPTIMIZEFORSMOKEEFFECT ) )
			{
				pNode->pLevelNode->uiFlags |= (LEVELNODE_LASTDYNAMIC);
				pNode->pLevelNode->uiFlags &= (~LEVELNODE_DYNAMIC );
			}
		}
	}
}


static void SetAniTileFrame(ANITILE* pAniTile, INT16 sFrame)
{
	UINT8 ubTempDir;
	INT16	sStartFrame = 0;

	if ( (pAniTile->uiFlags & ANITILE_USE_DIRECTION_FOR_START_FRAME ) )
	{
		// Our start frame is actually a direction indicator
		ubTempDir = gOneCDirection[ pAniTile->uiUserData3 ];
		sStartFrame = (UINT16)sFrame + ( pAniTile->usNumFrames * ubTempDir );
	}

	if ( (pAniTile->uiFlags & ANITILE_USE_4DIRECTION_FOR_START_FRAME ) )
	{
		// Our start frame is actually a direction indicator
		ubTempDir = gb4DirectionsFrom8[ pAniTile->uiUserData3 ];
		sStartFrame = (UINT16)sFrame + ( pAniTile->usNumFrames * ubTempDir );
	}

	pAniTile->sCurrentFrame = sStartFrame;

}


ANITILE *GetCachedAniTileOfType( INT16 sGridNo, UINT8 ubLevelID, UINT32 uiFlags )
{
	LEVELNODE *pNode = NULL;

	switch( ubLevelID )
	{
		case ANI_STRUCT_LEVEL:

			pNode = gpWorldLevelData[ sGridNo ].pStructHead;
			break;

		case ANI_SHADOW_LEVEL:

			pNode = gpWorldLevelData[ sGridNo ].pShadowHead;
			break;

		case ANI_OBJECT_LEVEL:

			pNode = gpWorldLevelData[ sGridNo ].pObjectHead;
			break;

		case ANI_ROOF_LEVEL:

			pNode = gpWorldLevelData[ sGridNo ].pRoofHead;
			break;

		case ANI_ONROOF_LEVEL:

			pNode = gpWorldLevelData[ sGridNo ].pOnRoofHead;
			break;

		case ANI_TOPMOST_LEVEL:

			pNode = gpWorldLevelData[ sGridNo ].pTopmostHead;
			break;

		default:

			return( NULL );
	}

	while( pNode != NULL )
	{
		if ( pNode->uiFlags & LEVELNODE_CACHEDANITILE )
		{
			if ( pNode->pAniTile->uiFlags & uiFlags )
			{
				return( pNode->pAniTile );
			}

		}

		pNode = pNode->pNext;
	}

	return( NULL );
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


static void PauseAniTile(ANITILE* pAniTile, BOOLEAN fPause)
{
	if ( fPause )
	{
		pAniTile->uiFlags |= ANITILE_PAUSED;
	}
	else
	{
		pAniTile->uiFlags &= (~ANITILE_PAUSED );
	}
}


static void PauseAllAniTilesOfType(UINT32 uiType, BOOLEAN fPause)
{
	ANITILE *pAniNode			= NULL;
	ANITILE *pNode				= NULL;

	// LOOP THROUGH EACH NODE
	pAniNode = pAniTileHead;

	while( pAniNode != NULL )
	{
		pNode = pAniNode;
		pAniNode = pAniNode->pNext;

		if ( pNode->uiFlags & uiType )
		{
      PauseAniTile( pNode, fPause );
		}
	}
}
