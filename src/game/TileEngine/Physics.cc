#include "Directories.h"
#include "Font_Control.h"
#include "Handle_Items.h"
#include "ItemModel.h"
#include "LoadSaveRealObject.h"
#include "Physics.h"
#include "Structure.h"
#include "TileDat.h"
#include "WCheck.h"
#include "Timer_Control.h"
#include "Isometric_Utils.h"
#include "LOS.h"
#include "WorldMan.h"
#include "Sound_Control.h"
#include "Soldier_Control.h"
#include "Interface.h"
#include "Interface_Items.h"
#include "Explosion_Control.h"
#include "Tile_Animation.h"
#include "Message.h"
#include "Weapons.h"
#include "Structure_Wrap.h"
#include "Overhead.h"
#include "Animation_Control.h"
#include "Text.h"
#include "Random.h"
#include "LightEffects.h"
#include "OppList.h"
#include "World_Items.h"
#include "Environment.h"
#include "SoundMan.h"
#include "Items.h"
#include "SkillCheck.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "Logger.h"

#include <algorithm>
#include <math.h>
#include <stdexcept>

#define NO_TEST_OBJECT				0
#define TEST_OBJECT_NO_COLLISIONS		1
#define TEST_OBJECT_ANY_COLLISION		2
#define TEST_OBJECT_NOTWALLROOF_COLLISIONS	3

#define OUTDOORS_START_ANGLE			(FLOAT)( PI/4 )
#define INDOORS_START_ANGLE			(FLOAT)( PI/30 )
//#define INDOORS_START_ANGLE			(FLOAT)( 0 )
#define GLAUNCHER_START_ANGLE			(FLOAT)( PI/8 )
#define GLAUNCHER_HIGHER_LEVEL_START_ANGLE	(FLOAT)( PI/6 )

#define GET_THROW_HEIGHT( l )			(INT16)( ( l * 256 ) )
#define GET_SOLDIER_THROW_HEIGHT( l )		(INT16)( ( l * 256 ) + STANDING_HEIGHT )

#define GET_OBJECT_LEVEL( z )			( (INT8)( ( z + 10 ) / HEIGHT_UNITS ) )
#define OBJECT_DETONATE_ON_IMPACT( o )		( ( o->Obj.usItem == MORTAR_SHELL ) ) // && ( o->ubActionCode == THROW_ARM_ITEM || pObject->fTestObject ) )


#define MAX_INTEGRATIONS			8

#define TIME_MULTI				1.8

//#define TIME_MULTI				2.2


#define DELTA_T					( 1.0 * TIME_MULTI )


#define GRAVITY					( 9.8 * 2.5 )
//#define GRAVITY				( 9.8 * 2.8 )


#define NUM_OBJECT_SLOTS			50
static REAL_OBJECT ObjectSlots[NUM_OBJECT_SLOTS];
UINT32  guiNumObjectSlots = 0;
BOOLEAN fDampingActive = FALSE;
//real   Kdl = (float)0.5; // LINEAR DAMPENING ( WIND RESISTANCE )
float   Kdl = (float)( 0.1 * TIME_MULTI ); // LINEAR DAMPENING ( WIND RESISTANCE )

#define EPSILONV				0.5
#define EPSILONP				(float)0.01
#define EPSILONPZ				3

#define CALCULATE_OBJECT_MASS( m )		( (float)( m * 2 ) )
#define SCALE_VERT_VAL_TO_HORZ( f )		( ( f / HEIGHT_UNITS ) * CELL_X_SIZE )
#define SCALE_HORZ_VAL_TO_VERT( f )		( ( f / CELL_X_SIZE ) * HEIGHT_UNITS )


#define REALOBJ2ID(o) 				((o) - ObjectSlots)


/// OBJECT POOL FUNCTIONS
static REAL_OBJECT* GetFreeObjectSlot(void)
{
	REAL_OBJECT*             i   = ObjectSlots;
	REAL_OBJECT const* const end = i + guiNumObjectSlots;
	for (; i != end; ++i)
	{
		if (!i->fAllocated) return i;
	}
	if (i != endof(ObjectSlots))
	{
		++guiNumObjectSlots;
		return i;
	}
	throw std::runtime_error("Out of physics object slots");
}


static void RecountObjectSlots(void)
{
	INT32 uiCount;

	for(uiCount=guiNumObjectSlots-1; (uiCount >=0) ; uiCount--)
	{
		if( ( ObjectSlots[uiCount].fAllocated ) )
		{
			guiNumObjectSlots=(UINT32)(uiCount+1);
			return;
		}
	}

	guiNumObjectSlots = 0;
}


static GridNo vector_3ToGridNo(vector_3 const& v)
{

	int16_t const row    = static_cast<int16_t>(v.y) / CELL_Y_SIZE;
	int16_t const column = static_cast<int16_t>(v.x) / CELL_X_SIZE;

	if (row >= 0 && row < WORLD_ROWS && column >= 0 && column < WORLD_COLS)
	{
		return row * WORLD_COLS + column;
	}

	return NOWHERE;
}


REAL_OBJECT* CreatePhysicalObject(OBJECTTYPE const* const pGameObj, float const dLifeLength, float const xPos, float const yPos, float const zPos, float const xForce, float const yForce, float const zForce, SOLDIERTYPE* const owner, UINT8 const ubActionCode, SOLDIERTYPE* const target)
{
	REAL_OBJECT* const o = GetFreeObjectSlot();
	*o = REAL_OBJECT{};

	o->Obj = *pGameObj;

	o->dLifeLength             = dLifeLength;
	o->fAllocated              = TRUE;
	o->fAlive                  = TRUE;
	o->fApplyFriction          = FALSE;
	o->uiSoundID               = NO_SAMPLE;
	o->Position.x              = xPos;
	o->Position.y              = yPos;
	o->Position.z              = zPos;
	o->fVisible                = TRUE;
	o->owner                   = owner;
	o->ubActionCode            = ubActionCode;
	o->target                  = target;
	o->fDropItem               = TRUE;
	o->ubLastTargetTakenDamage = NOBODY;
	o->fFirstTimeMoved         = TRUE;
	o->InitialForce.x          = SCALE_VERT_VAL_TO_HORZ(xForce);
	o->InitialForce.y          = SCALE_VERT_VAL_TO_HORZ(yForce);
	o->InitialForce.z          = zForce;
	o->InitialForce            = VMultScalar(&o->InitialForce, (float)(1.5 / TIME_MULTI));
	o->sGridNo                 = vector_3ToGridNo(o->Position);
	o->pNode                   = 0;
	o->pShadow                 = 0;

	// If gridno not equal to NOWHERE, use sHeight of land
	if (o->sGridNo != NOWHERE)
	{
		float const h = CONVERT_PIXELS_TO_HEIGHTUNITS(gpWorldLevelData[o->sGridNo].sHeight);
		o->Position.z                   += h;
		o->EndedWithCollisionPosition.z += h;
	}
	else
	{
		SLOGW("Physics object created at invalid gridno");
	}

	return o;
}


static BOOLEAN RemoveRealObject(REAL_OBJECT* const o)
{
	CHECKF(ObjectSlots <= o && o < endof(ObjectSlots));

	o->fAllocated = FALSE;

	RecountObjectSlots();

	return( TRUE );
}


static void SimulateObject(REAL_OBJECT* pObject, float deltaT);


void SimulateWorld(  )
{
	UINT32					cnt;
	REAL_OBJECT		*pObject;


	if ( COUNTERDONE( PHYSICSUPDATE ) )
	{
		for( cnt = 0;cnt < guiNumObjectSlots; cnt++)
		{
			// CHECK FOR ALLOCATED
			if( ObjectSlots[ cnt ].fAllocated )
			{
				// Get object
				pObject = &( ObjectSlots[ cnt ] );

				SimulateObject( pObject, (float)DELTA_T );
			}
		}
	}
}


static void PhysicsDeleteObject(REAL_OBJECT* pObject);


void RemoveAllPhysicsObjects( )
{
	UINT32					cnt;

	for( cnt = 0;cnt < guiNumObjectSlots; cnt++)
	{
		// CHECK FOR ALLOCATED
		if( ObjectSlots[ cnt ].fAllocated )
		{
			PhysicsDeleteObject( &(ObjectSlots[ cnt ]) );
		}
	}
}


static BOOLEAN PhysicsComputeForces(REAL_OBJECT* pObject);
static BOOLEAN PhysicsHandleCollisions(REAL_OBJECT* pObject, INT32* piCollisionID, float DeltaTime);
static BOOLEAN PhysicsIntegrate(REAL_OBJECT* pObject, float DeltaTime);
static BOOLEAN PhysicsMoveObject(REAL_OBJECT* pObject);
static BOOLEAN PhysicsUpdateLife(REAL_OBJECT* pObject, float DeltaTime);


static void SimulateObject(REAL_OBJECT* pObject, float deltaT)
{
	float   DeltaTime = 0;
	float   CurrentTime = 0;
	float   TargetTime = DeltaTime;
	INT32   iCollisionID;
	BOOLEAN fEndThisObject = FALSE;

	if ( !PhysicsUpdateLife( pObject, (float)deltaT ) )
	{
		return;
	}

	if ( pObject->fAlive )
	{
		CurrentTime = 0;
		TargetTime = (float)deltaT;

		// Do subtime here....
		DeltaTime = (float)deltaT / (float)10;

		if ( !PhysicsComputeForces( pObject ) )
		{
			return;
		}

		while( CurrentTime < TargetTime )
		{
			if ( !PhysicsIntegrate( pObject, DeltaTime ) )
			{
				fEndThisObject = TRUE;
				break;
			}

			if ( !PhysicsHandleCollisions( pObject, &iCollisionID, DeltaTime  ) )
			{
				fEndThisObject = TRUE;
				break;
			}

			if ( iCollisionID != COLLISION_NONE )
			{
				break;
			}

			CurrentTime += DeltaTime;
		}

		if ( fEndThisObject )
		{
			return;
		}

		if ( !PhysicsMoveObject( pObject ) )
		{
			return;
		}

	}
}


static BOOLEAN PhysicsComputeForces(REAL_OBJECT* pObject)
{
	vector_3			vTemp;

	// Calculate forces
	pObject->Force = pObject->InitialForce;

	pObject->Force.z -= (float)GRAVITY;

	// Set intial force to zero
	pObject->InitialForce = VMultScalar( &(pObject->InitialForce ), 0 );

	if ( pObject->fApplyFriction )
	{
		vTemp = VMultScalar( &(pObject->Velocity), -pObject->AppliedMu );
		pObject->Force = VAdd( &(vTemp), &(pObject->Force) );

		pObject->fApplyFriction = FALSE;
	}

	if( fDampingActive )
	{
		vTemp = VMultScalar( &(pObject->Velocity), -Kdl );
		pObject->Force = VAdd( &(vTemp), &(pObject->Force) );

	}

	return( TRUE );
}


static void HandleArmedObjectImpact(REAL_OBJECT* pObject);


static BOOLEAN PhysicsUpdateLife(REAL_OBJECT* pObject, float DeltaTime)
{
	UINT8 bLevel = 0;

	pObject->dLifeSpan += DeltaTime;

	// End life if time has ran out or we are stationary
	if ( pObject->dLifeLength != -1 )
	{
		if ( pObject->dLifeSpan > pObject->dLifeLength )
		{
			pObject->fAlive = FALSE;
		}

	}

	// End life if we are out of bounds....
	if ( !GridNoOnVisibleWorldTile( pObject->sGridNo ) )
	{
		pObject->fAlive = FALSE;
	}

	if ( !pObject->fAlive )
	{
		pObject->fAlive = FALSE;

		if ( !pObject->fTestObject )
		{
			if ( pObject->uiSoundID != NO_SAMPLE )
			{
				SoundStop( pObject->uiSoundID );
			}

			if ( pObject->ubActionCode == THROW_ARM_ITEM && !pObject->fInWater )
			{
				HandleArmedObjectImpact( pObject );
			}
			else
			{
				// If we are in water, and we are a sinkable item...
				if ( !pObject->fInWater || !( GCM->getItem(pObject->Obj.usItem)->getFlags() & ITEM_SINKS ) )
				{
					if ( pObject->fDropItem )
					{
						// ATE: If we have collided with roof last...
						if ( pObject->iOldCollisionCode == COLLISION_ROOF )
						{
							bLevel = 1;
						}

						// ATE; If an armed object, don't add....
						if ( pObject->ubActionCode != THROW_ARM_ITEM )
						{
							AddItemToPool(pObject->sGridNo, &pObject->Obj, VISIBLE, bLevel, 0, -1);
						}
					}
				}
			}

			// Make impact noise....
			if ( pObject->Obj.usItem == ROCK || pObject->Obj.usItem == ROCK2 )
			{
				MakeNoise(pObject->owner, pObject->sGridNo, 0, 9 + PreRandom(9), NOISE_ROCK_IMPACT);
			}
			else if ( GCM->getItem(pObject->Obj.usItem)->isGrenade() )
			{
				MakeNoise(pObject->owner, pObject->sGridNo, 0, 9 + PreRandom(9), NOISE_GRENADE_IMPACT);
			}

			if ( !pObject->fTestObject && pObject->iOldCollisionCode == COLLISION_GROUND )
			{
				PlayLocationJA2Sample(pObject->sGridNo, THROW_IMPACT_2, MIDVOLUME, 1);
			}

			ReduceAttackBusyCount(pObject->owner, FALSE);

			// ATE: Handle end of animation...
			if ( pObject->fCatchAnimOn )
			{
				pObject->fCatchAnimOn = FALSE;

				// Get intended target
				SOLDIERTYPE* const pSoldier = pObject->target;

				// Catch anim.....
				switch( gAnimControl[ pSoldier->usAnimState ].ubHeight )
				{
					case ANIM_STAND:

						pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
						EVENT_InitNewSoldierAnim( pSoldier, END_CATCH, 0 , FALSE );
						break;

					case ANIM_CROUCH:

						pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
						EVENT_InitNewSoldierAnim( pSoldier, END_CROUCH_CATCH, 0 , FALSE );
						break;
				}

				PlayLocationJA2Sample(pSoldier->sGridNo, CATCH_OBJECT, MIDVOLUME, 1);
			}
		}

		PhysicsDeleteObject( pObject );
		return( FALSE );
	}

	return( TRUE );
}


static BOOLEAN PhysicsIntegrate(REAL_OBJECT* pObject, float DeltaTime)
{
	vector_3			vTemp;

	// Save old position
	pObject->OldPosition = pObject->Position;
	pObject->OldVelocity = pObject->Velocity;

	vTemp = VMultScalar( &(pObject->Velocity), DeltaTime );
	pObject->Position = VAdd( &(pObject->Position), &vTemp );

	// Save test TargetPosition
	if ( pObject->fTestPositionNotSet )
	{
		pObject->TestTargetPosition = pObject->Position;
	}

	vTemp = VMultScalar( &(pObject->Force), ( DeltaTime / 60.0f ) );
	pObject->Velocity = VAdd( &(pObject->Velocity), &vTemp );

	if ( pObject->fPotentialForDebug )
	{
		SLOGD("Object {}: Force     {} {} {}", REALOBJ2ID(pObject),
			pObject->Force.x, pObject->Force.y, pObject->Force.z);
		SLOGD("Object {}: Velocity  {} {} {}", REALOBJ2ID(pObject),
			pObject->Velocity.x, pObject->Velocity.y, pObject->Velocity.z);
		SLOGD("Object {}: Position  {} {} {}", REALOBJ2ID(pObject),
			pObject->Position.x, pObject->Position.y, pObject->Position.z);
		SLOGD("Object {}: Delta Pos {} {} {}", REALOBJ2ID(pObject),
			pObject->OldPosition.x - pObject->Position.x, pObject->OldPosition.y - pObject->Position.y,
			pObject->OldPosition.z - pObject->Position.z);
	}

	if ( pObject->Obj.usItem == MORTAR_SHELL && !pObject->fTestObject && pObject->ubActionCode == THROW_ARM_ITEM )
	{
		// Start soud if we have reached our max height
		if ( pObject->OldVelocity.z >= 0 && pObject->Velocity.z < 0 )
		{
			if ( pObject->uiSoundID == NO_SAMPLE )
			{
				pObject->uiSoundID =	PlayJA2Sample(MORTAR_WHISTLE, HIGHVOLUME, 1, MIDDLEPAN);
			}
		}
	}

	return( TRUE );
}


static BOOLEAN PhysicsCheckForCollisions(REAL_OBJECT* pObject, INT32* piCollisionID);
static void PhysicsResolveCollision(REAL_OBJECT* pObject, vector_3* pVelocity, vector_3* pNormal, float CoefficientOfRestitution);


static BOOLEAN PhysicsHandleCollisions(REAL_OBJECT* pObject, INT32* piCollisionID, float DeltaTime)
{
	FLOAT dDeltaX, dDeltaY, dDeltaZ;


	if ( PhysicsCheckForCollisions( pObject, piCollisionID ) )
	{

		dDeltaX = pObject->Position.x - pObject->OldPosition.x;
		dDeltaY = pObject->Position.y - pObject->OldPosition.y;
		dDeltaZ = pObject->Position.z - pObject->OldPosition.z;

		if ( dDeltaX <= EPSILONV && dDeltaX >= -EPSILONV &&
			dDeltaY <= EPSILONV && dDeltaY >= -EPSILONV )
		{
			pObject->sConsecutiveZeroVelocityCollisions++;
		}

		if ( pObject->sConsecutiveZeroVelocityCollisions > 3 )
		{
			// We will continue with our Z velocity
			pObject->Velocity.x = 0;
			pObject->Velocity.y = 0;

			// Check that we are not colliding with structure z
			//if ( *piCollisionID == COLLISION_STRUCTURE_Z || *piCollisionID == COLLISION_ROOF )
			if ( *piCollisionID == COLLISION_STRUCTURE_Z || *piCollisionID == COLLISION_ROOF || *piCollisionID == COLLISION_GROUND )
			{
				pObject->Velocity.z = 0;

				// Set us not alive!
				pObject->fAlive = FALSE;
			}

			*piCollisionID = COLLISION_NONE;
		}
		else
		{
			// Set position back to before collision
			pObject->Position = pObject->OldPosition;
			// Set old position!
			pObject->OldPosition.x = pObject->Position.y - dDeltaX;
			pObject->OldPosition.y = pObject->Position.x - dDeltaY;
			pObject->OldPosition.z = pObject->Position.z - dDeltaZ;

			PhysicsResolveCollision( pObject, &(pObject->CollisionVelocity), &(pObject->CollisionNormal), pObject->CollisionElasticity );
		}

		if ( pObject->Position.z < 0 )
		{
			pObject->Position.z = 0;
		}
		//otherwise, continue falling downwards!

		// TO STOP?

		// Check for delta position values
		if (dDeltaZ <= EPSILONP && dDeltaZ >= -EPSILONP &&
			dDeltaY <= EPSILONP && dDeltaY >= -EPSILONP &&
			dDeltaX <= EPSILONP && dDeltaX >= -EPSILONP)
		{
			//pObject->fAlive = FALSE;
			//return( FALSE );
		}

		// Check for repeated collisions...
		//if ( pObject->iOldCollisionCode == COLLISION_ROOF || pObject->iOldCollisionCode == COLLISION_GROUND || pObject->iOldCollisionCode == COLLISION_WATER )
		{
			// ATE: This is a safeguard
			if (pObject->sConsecutiveCollisions > 30)
			{
				pObject->fAlive = FALSE;
				return( FALSE );
			}
		}


		// Check for -ve velocity still...
		//if ( pObject->Velocity.z <= EPSILONV && pObject->Velocity.z >= -EPSILONV &&
		//		pObject->Velocity.y <= EPSILONV && pObject->Velocity.y >= -EPSILONV &&
		//		pObject->Velocity.x <= EPSILONV && pObject->Velocity.x >= -EPSILONV )
		//{
			//PhysicsDeleteObject( pObject );
		//	pObject->fAlive = FALSE;
		//	return( FALSE );
		//}
	}

	return( TRUE );
}


static void PhysicsDeleteObject(REAL_OBJECT* pObject)
{
	if ( pObject->fAllocated )
	{
		if ( pObject->pNode != NULL )
		{
			RemoveStructFromLevelNode( pObject->sLevelNodeGridNo, pObject->pNode );
		}

		if ( pObject->pShadow != NULL )
		{
			RemoveShadowFromLevelNode( pObject->sLevelNodeGridNo, pObject->pShadow );
		}

		RemoveRealObject(pObject);
	}
}


static BOOLEAN CheckForCatcher(REAL_OBJECT* pObject, UINT16 usStructureID);
static void CheckForObjectHittingMerc(REAL_OBJECT* pObject, UINT16 usStructureID);


static BOOLEAN PhysicsCheckForCollisions(REAL_OBJECT* pObject, INT32* piCollisionID)
{
	vector_3 vTemp;
	FLOAT    dDeltaX, dDeltaY, dDeltaZ, dX, dY, dZ;
	INT32    iCollisionCode = COLLISION_NONE;
	BOOLEAN  fDoCollision = FALSE;
	FLOAT    dElasity = 1;
	UINT16   usStructureID;
	FLOAT    dNormalX, dNormalY, dNormalZ;
	INT16    sGridNo;

	// Checkf for collisions
	dX = pObject->Position.x;
	dY = pObject->Position.y;
	dZ = pObject->Position.z;

	vTemp.x = 0;
	vTemp.y = 0;
	vTemp.z = 0;

	dDeltaX = dX - pObject->OldPosition.x;
	dDeltaY = dY - pObject->OldPosition.y;
	dDeltaZ = dZ - pObject->OldPosition.z;

	//Round delta pos to nearest 0.01
	//dDeltaX = (float)( (int)dDeltaX * 100 ) / 100;
	//dDeltaY = (float)( (int)dDeltaY * 100 ) / 100;
	//dDeltaZ = (float)( (int)dDeltaZ * 100 ) / 100;

	// SKIP FIRST GRIDNO, WE'LL COLLIDE WITH OURSELVES....
	if ( pObject->fTestObject != TEST_OBJECT_NO_COLLISIONS )
	{
		iCollisionCode = CheckForCollision( dX, dY, dZ, dDeltaX, dDeltaY, dDeltaZ, &usStructureID, &dNormalX, &dNormalY, &dNormalZ );
	}
	else if ( pObject->fTestObject == TEST_OBJECT_NO_COLLISIONS )
	{
		iCollisionCode = COLLISION_NONE;

		// Are we on a downward slope?
		if ( dZ < pObject->TestZTarget && dDeltaZ < 0 )
		{
			if (pObject->fTestPositionNotSet )
			{
				if ( pObject->TestZTarget > 32 )
				{
					pObject->fTestPositionNotSet = FALSE;
					pObject->TestZTarget         = 0;
				}
				else
				{
					iCollisionCode = COLLISION_GROUND;
				}
			}
			else
			{
				iCollisionCode = COLLISION_GROUND;
			}
		}
	}


	// If a test object and we have collided with something ( should only be ground ( or roof? ) )
	// Or destination?
	if ( pObject->fTestObject == TEST_OBJECT_ANY_COLLISION )
	{
		if ( iCollisionCode != COLLISION_GROUND && iCollisionCode != COLLISION_ROOF && iCollisionCode != COLLISION_WATER && iCollisionCode != COLLISION_NONE )
		{
			pObject->fTestEndedWithCollision = TRUE;
			pObject->fAlive = FALSE;
			return( FALSE );
		}
	}

	if ( pObject->fTestObject == TEST_OBJECT_NOTWALLROOF_COLLISIONS )
	{
		// So we don't collide with ourselves.....
		if ( iCollisionCode != COLLISION_WATER && iCollisionCode != COLLISION_GROUND && iCollisionCode != COLLISION_NONE &&
			iCollisionCode != COLLISION_ROOF && iCollisionCode != COLLISION_INTERIOR_ROOF &&
			iCollisionCode != COLLISION_WALL_SOUTHEAST && iCollisionCode != COLLISION_WALL_SOUTHWEST &&
			iCollisionCode != COLLISION_WALL_NORTHEAST && iCollisionCode != COLLISION_WALL_NORTHWEST )
		{
			if ( pObject->fFirstTimeMoved || pObject->sFirstGridNo == pObject->sGridNo )
			{
				iCollisionCode = COLLISION_NONE;
			}

			// If we are NOT a wall or window, ignore....
			if ( pObject->uiNumTilesMoved < 4 )
			{
				switch( iCollisionCode )
				{
					case COLLISION_MERC:
					case COLLISION_STRUCTURE:
					case COLLISION_STRUCTURE_Z:
						// Set to no collision ( we shot past )
						iCollisionCode = COLLISION_NONE;
						break;
				}
			}
		}


		switch( iCollisionCode )
		{
			// End test with any collision NOT a wall, roof...
			case COLLISION_STRUCTURE:
			case COLLISION_STRUCTURE_Z:
				// OK, if it's mercs... don't stop
				if ( usStructureID >= INVALID_STRUCTURE_ID )
				{
					pObject->fTestEndedWithCollision = TRUE;

					if ( !pObject->fEndedWithCollisionPositionSet )
					{
						pObject->fEndedWithCollisionPositionSet = TRUE;
						pObject->EndedWithCollisionPosition = pObject->Position;
					}
					iCollisionCode = COLLISION_NONE;
				}
				else
				{
					if ( !pObject->fEndedWithCollisionPositionSet )
					{
						pObject->fEndedWithCollisionPositionSet = TRUE;
						pObject->EndedWithCollisionPosition = pObject->Position;
					}
				}
				break;

			case COLLISION_ROOF:

				if ( !pObject->fEndedWithCollisionPositionSet )
				{
					pObject->fEndedWithCollisionPositionSet = TRUE;
					pObject->EndedWithCollisionPosition = pObject->Position;
				}
				break;

			case COLLISION_WATER:
			case COLLISION_GROUND:
			case COLLISION_MERC:
			case COLLISION_INTERIOR_ROOF:
			case COLLISION_NONE:
			case COLLISION_WINDOW_SOUTHEAST:
			case COLLISION_WINDOW_SOUTHWEST:
			case COLLISION_WINDOW_NORTHEAST:
			case COLLISION_WINDOW_NORTHWEST:
				// Here we just keep going..
				break;

			default:
				// THis is for walls, windows, etc
				// here, we set test ended with collision, but keep going...
				pObject->fTestEndedWithCollision = TRUE;
				break;
		}
	}


	if ( pObject->fTestObject != TEST_OBJECT_NOTWALLROOF_COLLISIONS )
	{
		if ( iCollisionCode != COLLISION_WATER && iCollisionCode != COLLISION_GROUND && iCollisionCode != COLLISION_NONE &&
			iCollisionCode != COLLISION_ROOF && iCollisionCode != COLLISION_INTERIOR_ROOF &&
			iCollisionCode != COLLISION_WALL_SOUTHEAST && iCollisionCode != COLLISION_WALL_SOUTHWEST &&
			iCollisionCode != COLLISION_WALL_NORTHEAST && iCollisionCode != COLLISION_WALL_NORTHWEST )
		{
			// So we don't collide with ourselves.....
			if ( pObject->fFirstTimeMoved || pObject->sFirstGridNo == pObject->sGridNo )
			{
				iCollisionCode = COLLISION_NONE;
			}

			// If we are NOT a wall or window, ignore....
			if ( pObject->uiNumTilesMoved < 4 )
			{
				switch( iCollisionCode )
				{
				case COLLISION_MERC:
				case COLLISION_STRUCTURE:
				case COLLISION_STRUCTURE_Z:

					// Set to no collision ( we shot past )
					iCollisionCode = COLLISION_NONE;
					break;
				}
			}

		}
	}

	*piCollisionID = iCollisionCode;


	// If We hit the ground
	if ( iCollisionCode > COLLISION_NONE )
	{
		if ( pObject->iOldCollisionCode == iCollisionCode )
		{
			pObject->sConsecutiveCollisions++;
		}
		else
		{
			pObject->sConsecutiveCollisions = 1;
		}

		if ( iCollisionCode == COLLISION_WINDOW_NORTHWEST || iCollisionCode == COLLISION_WINDOW_NORTHEAST || iCollisionCode == COLLISION_WINDOW_SOUTHWEST || iCollisionCode == COLLISION_WINDOW_SOUTHEAST )
		{
			if ( !pObject->fTestObject )
			{
				// Break window!
				SLOGD("Object {}: Collision Window", REALOBJ2ID(pObject));

				sGridNo = vector_3ToGridNo(pObject->Position);

				WindowHit(sGridNo, usStructureID, FALSE, TRUE);
			}
			*piCollisionID = COLLISION_NONE;
			return( FALSE );
		}

		// ATE: IF detonate on impact, stop now!
		if ( OBJECT_DETONATE_ON_IMPACT( pObject ) )
		{
			pObject->fAlive = FALSE;
			return( TRUE );
		}

		if ( iCollisionCode == COLLISION_GROUND )
		{
			vTemp.x = 0;
			vTemp.y = 0;
			vTemp.z = -1;

			pObject->fApplyFriction = TRUE;
			//pObject->AppliedMu = (float)(0.54 * TIME_MULTI );
			pObject->AppliedMu = (float)(0.34 * TIME_MULTI );

			//dElasity = (float)1.5;
			dElasity = (float)1.3;

			fDoCollision = TRUE;

			if ( !pObject->fTestObject && !pObject->fHaveHitGround )
			{
				PlayLocationJA2Sample(pObject->sGridNo, THROW_IMPACT_2, MIDVOLUME, 1);
			}

			pObject->fHaveHitGround = TRUE;
		}
		else if ( iCollisionCode == COLLISION_WATER )
		{
			ANITILE_PARAMS	AniParams;
			ANITILE						*pNode;

			// Continue going...
			pObject->fApplyFriction = TRUE;
			pObject->AppliedMu = (float)(1.54 * TIME_MULTI );

			sGridNo = vector_3ToGridNo(pObject->Position);

			// Make thing unalive...
			pObject->fAlive = FALSE;

			// If first time...
			if ( pObject->fVisible )
			{
				if ( pObject->fTestObject == NO_TEST_OBJECT )
				{
					// Make invisible
					pObject->fVisible = FALSE;

					// JA25 CJC Oct 13 1999 - if node pointer is null don't try to set flags inside it!
					if( pObject->pNode )
					{
						pObject->pNode->uiFlags |= LEVELNODE_HIDDEN;
					}

					pObject->fInWater = TRUE;

					// Make ripple
					AniParams = ANITILE_PARAMS{};
					AniParams.sGridNo = sGridNo;
					AniParams.ubLevelID = ANI_STRUCT_LEVEL;
					AniParams.usTileIndex = THIRDMISS1;
					AniParams.sDelay = 50;
					AniParams.sStartFrame = 0;
					AniParams.uiFlags = ANITILE_FORWARD;


					if ( pObject->ubActionCode == THROW_ARM_ITEM )
					{
						AniParams.ubKeyFrame1 = 11;
						AniParams.uiKeyFrame1Code = ANI_KEYFRAME_CHAIN_WATER_EXPLOSION;
						AniParams.v.object        = pObject;
					}

					pNode = CreateAnimationTile( &AniParams );

					// Adjust for absolute positioning
					pNode->pLevelNode->uiFlags |= LEVELNODE_USEABSOLUTEPOS;

					pNode->pLevelNode->sRelativeX = (INT16)pObject->Position.x;
					pNode->pLevelNode->sRelativeY = (INT16)pObject->Position.y;
					pNode->pLevelNode->sRelativeZ = (INT16)CONVERT_HEIGHTUNITS_TO_PIXELS( (INT16)pObject->Position.z );
				}
			}

		}
		else if ( iCollisionCode == COLLISION_ROOF || iCollisionCode == COLLISION_INTERIOR_ROOF )
		{
			vTemp.x = 0;
			vTemp.y = 0;
			vTemp.z = -1;

			pObject->fApplyFriction = TRUE;
			pObject->AppliedMu = (float)(0.54 * TIME_MULTI );

			dElasity = (float)1.4;

			fDoCollision = TRUE;

		}
		//else if ( iCollisionCode == COLLISION_INTERIOR_ROOF )
		//{
		//	vTemp.x = 0;
		//	vTemp.y = 0;
		//	vTemp.z = 1;

		//	pObject->fApplyFriction = TRUE;
		//	pObject->AppliedMu = (float)(0.54 * TIME_MULTI );

		//	dElasity = (float)1.4;

		//	fDoCollision = TRUE;

		//}
		else if ( iCollisionCode == COLLISION_STRUCTURE_Z )
		{
			if ( CheckForCatcher( pObject, usStructureID ) )
			{
				return( FALSE );
			}

			CheckForObjectHittingMerc( pObject, usStructureID );

			vTemp.x = 0;
			vTemp.y = 0;
			vTemp.z = -1;

			pObject->fApplyFriction = TRUE;
			pObject->AppliedMu = (float)(0.54 * TIME_MULTI );

			dElasity = (float)1.2;

			fDoCollision = TRUE;

		}
		else if (iCollisionCode == COLLISION_WALL_SOUTHEAST || iCollisionCode == COLLISION_WALL_SOUTHWEST ||
				iCollisionCode == COLLISION_WALL_NORTHEAST || iCollisionCode == COLLISION_WALL_NORTHWEST )
		{
			// A wall, do stuff
			vTemp.x = dNormalX;
			vTemp.y = dNormalY;
			vTemp.z = dNormalZ;

			fDoCollision = TRUE;

			dElasity = (float)1.1;
		}
		else
		{
			vector_3 vIncident;

			if ( CheckForCatcher( pObject, usStructureID ) )
			{
				return( FALSE );
			}

			CheckForObjectHittingMerc( pObject, usStructureID );

			vIncident.x = dDeltaX;
			vIncident.y = dDeltaY;
			vIncident.z = 0;
			// Nomralize

			vIncident = VGetNormal( &vIncident );

			//vTemp.x = -1;
			//vTemp.y = 0;
			//vTemp.z = 0;
			vTemp.x = -1 * vIncident.x;
			vTemp.y = -1 * vIncident.y;
			vTemp.z = 0;

			fDoCollision = TRUE;

			dElasity = (float)1.1;
		}

		if ( fDoCollision )
		{
			pObject->CollisionNormal.x		= vTemp.x;
			pObject->CollisionNormal.y		= vTemp.y;
			pObject->CollisionNormal.z		= vTemp.z;
			pObject->CollisionElasticity  = dElasity;
			pObject->iOldCollisionCode  = iCollisionCode;

			// Save collision velocity
			pObject->CollisionVelocity = pObject->OldVelocity;

			if ( pObject->fPotentialForDebug )
			{
				SLOGD("Object {}: Collision {}", REALOBJ2ID(pObject), iCollisionCode);
				SLOGD("Object {}: Collision Normal {} {} {}", REALOBJ2ID(pObject),
					vTemp.x, vTemp.y, vTemp.z);
				SLOGD("Object {}: Collision OldPos {} {} {}", REALOBJ2ID(pObject),
					pObject->Position.x, pObject->Position.y, pObject->Position.z);
				SLOGD("Object {}: Collision Velocity {} {} {}", REALOBJ2ID(pObject),
					pObject->CollisionVelocity.x, pObject->CollisionVelocity.y, pObject->CollisionVelocity.z);
			}
		}
		else
		{
			pObject->sConsecutiveCollisions = 0;
			pObject->sConsecutiveZeroVelocityCollisions = 0;
			pObject->fHaveHitGround = FALSE;
		}
	}

	return( fDoCollision );
}


static void PhysicsResolveCollision(REAL_OBJECT* pObject, vector_3* pVelocity, vector_3* pNormal, float CoefficientOfRestitution)
{
	float ImpulseNumerator, Impulse;
	vector_3 vTemp;

	ImpulseNumerator = -1 * CoefficientOfRestitution * VDotProduct( pVelocity , pNormal );

	Impulse = ImpulseNumerator;

	vTemp = VMultScalar( pNormal, Impulse );

	pObject->Velocity = VAdd( &(pObject->Velocity), &vTemp );

}


static BOOLEAN CheckForCatchObject(REAL_OBJECT* pObject);


static BOOLEAN PhysicsMoveObject(REAL_OBJECT* pObject)
{
	LEVELNODE *pNode;

	//Determine new gridno
	GridNo const sNewGridNo = vector_3ToGridNo(pObject->Position);

	if ( pObject->fFirstTimeMoved )
	{
		pObject->fFirstTimeMoved = FALSE;
		pObject->sFirstGridNo    = sNewGridNo;
	}

	// CHECK FOR RANGE< IF INVALID, REMOVE!
	if (sNewGridNo == NOWHERE)
	{
		PhysicsDeleteObject( pObject );
		return( FALSE );
	}

	// Look at old gridno
	if ( sNewGridNo != pObject->sGridNo || pObject->pNode == NULL )
	{
		if ( pObject->fVisible )
		{
			if ( CheckForCatchObject( pObject ) )
			{
				pObject->fVisible = FALSE;
			}
		}

		if ( pObject->fVisible )
		{
			// Add smoke trails...
			if ( pObject->Obj.usItem == MORTAR_SHELL && pObject->uiNumTilesMoved > 2 && pObject->ubActionCode == THROW_ARM_ITEM )
			{
				if ( sNewGridNo != pObject->sGridNo )
				{
					ANITILE_PARAMS	AniParams{};
					AniParams.sGridNo = (INT16)sNewGridNo;
					AniParams.ubLevelID = ANI_STRUCT_LEVEL;
					AniParams.sDelay = (INT16)( 100 + PreRandom( 100 ) );
					AniParams.sStartFrame = 0;
					AniParams.uiFlags = ANITILE_FORWARD | ANITILE_ALWAYS_TRANSLUCENT;
					AniParams.sX = (INT16)pObject->Position.x;
					AniParams.sY = (INT16)pObject->Position.y;
					AniParams.sZ = (INT16)CONVERT_HEIGHTUNITS_TO_PIXELS( (INT16)pObject->Position.z );
					AniParams.zCachedFile = TILECACHEDIR "/msle_smk.sti";
					CreateAnimationTile( &AniParams );
				}
			}
			else if ( pObject->uiNumTilesMoved > 0 )
			{
				if ( sNewGridNo != pObject->sGridNo )
				{
					// We're at a new gridno!
					if ( pObject->pNode != NULL )
					{
						RemoveStructFromLevelNode( pObject->sLevelNodeGridNo, pObject->pNode );
					}

					// We're at a new gridno!
					if ( pObject->pShadow != NULL )
					{
						RemoveShadowFromLevelNode( pObject->sLevelNodeGridNo, pObject->pShadow );
					}

					// Now get graphic index
					INT16 const sTileIndex = GetTileGraphicForItem(GCM->getItem(pObject->Obj.usItem));
					//sTileIndex = BULLETTILE1;

					// Set new gridno, add
					pNode = AddStructToTail( sNewGridNo, sTileIndex );
					pNode->ubShadeLevel=DEFAULT_SHADE_LEVEL;
					pNode->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
					pNode->uiFlags |= ( LEVELNODE_USEABSOLUTEPOS | LEVELNODE_IGNOREHEIGHT | LEVELNODE_PHYSICSOBJECT | LEVELNODE_DYNAMIC );

					// Set levelnode
					pObject->pNode = pNode;

					// Add shadow
					pNode = AddShadowToHead(sNewGridNo, sTileIndex);
					pNode->ubShadeLevel=DEFAULT_SHADE_LEVEL;
					pNode->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
					pNode->uiFlags |= ( LEVELNODE_USEABSOLUTEPOS | LEVELNODE_IGNOREHEIGHT | LEVELNODE_PHYSICSOBJECT | LEVELNODE_DYNAMIC );

					// Set levelnode
					pObject->pShadow = pNode;

					pObject->sLevelNodeGridNo = sNewGridNo;
				}
			}
		}
		else
		{
			// Remove!
			if ( pObject->pNode != NULL )
			{
				RemoveStructFromLevelNode( pObject->sLevelNodeGridNo, pObject->pNode );
			}

			// We're at a new gridno!
			if ( pObject->pShadow != NULL )
			{
				RemoveShadowFromLevelNode( pObject->sLevelNodeGridNo, pObject->pShadow );
			}

			pObject->pNode = NULL;
			pObject->pShadow  = NULL;
		}

		if ( sNewGridNo != pObject->sGridNo )
		{
			pObject->uiNumTilesMoved++;
		}

		pObject->sGridNo = sNewGridNo;

		if ( pObject->fPotentialForDebug )
		{
			SLOGD("Object {}d: uiNumTilesMoved: {}", REALOBJ2ID(pObject), pObject->uiNumTilesMoved);
		}
	}

	if ( pObject->fVisible )
	{
		if ( pObject->Obj.usItem != MORTAR_SHELL || pObject->ubActionCode != THROW_ARM_ITEM )
		{
			if ( pObject->pNode != NULL )
			{
				// Add new object / update position
				// Update position data
				pObject->pNode->sRelativeX = (INT16)pObject->Position.x;
				pObject->pNode->sRelativeY = (INT16)pObject->Position.y;
				pObject->pNode->sRelativeZ = (INT16)CONVERT_HEIGHTUNITS_TO_PIXELS( (INT16)pObject->Position.z );

				// Update position data
				pObject->pShadow->sRelativeX = (INT16)pObject->Position.x;
				pObject->pShadow->sRelativeY = (INT16)pObject->Position.y;
				pObject->pShadow->sRelativeZ = (INT16)gpWorldLevelData[ pObject->sGridNo ].sHeight;
			}
		}
	}

	return( TRUE );
}


static FLOAT CalculateObjectTrajectory(INT16 sTargetZ, const OBJECTTYPE* pItem, vector_3* vPosition, vector_3* vForce, INT16* psFinalGridNo);


static vector_3 FindBestForceForTrajectory(INT16 sSrcGridNo, INT16 sGridNo, INT16 sStartZ, INT16 sEndZ, float dzDegrees, const OBJECTTYPE* pItem, INT16* psGridNo, float* pdMagForce)
{
	vector_3 vDirNormal, vPosition, vForce;
	INT16    sDestX, sDestY, sSrcX, sSrcY;
	float    dForce = 20;
	float    dRange;
	float    dPercentDiff = 0;
	float    dTestRange, dTestDiff;
	INT32    iNumChecks = 0;

	// Get XY from gridno
	ConvertGridNoToCenterCellXY( sGridNo, &sDestX, &sDestY );
	ConvertGridNoToCenterCellXY( sSrcGridNo, &sSrcX, &sSrcY );

	// Set position
	vPosition.x = sSrcX;
	vPosition.y = sSrcY;
	vPosition.z = sStartZ;

	// OK, get direction normal
	vDirNormal.x = (float)(sDestX - sSrcX);
	vDirNormal.y = (float)(sDestY - sSrcY);
	vDirNormal.z = 0;

	// NOmralize
	vDirNormal = VGetNormal( &vDirNormal );

	// From degrees, calculate Z portion of normal
	vDirNormal.z = (float)sin( dzDegrees );

	// Get range
	dRange = (float)GetRangeInCellCoordsFromGridNoDiff( sGridNo, sSrcGridNo );

	//calculate force needed
	{
		dForce = (float)( 12 * ( sqrt( ( GRAVITY * dRange ) / sin( 2 * dzDegrees ) ) ) );
	}

	do
	{
		// This first force is just an estimate...
		// now di a binary search to find best value....
		iNumChecks++;


		// Now use a force
		vForce.x = dForce * vDirNormal.x;
		vForce.y = dForce * vDirNormal.y;
		vForce.z = dForce * vDirNormal.z;

		dTestRange = CalculateObjectTrajectory( sEndZ, pItem, &vPosition, &vForce, psGridNo );

		// What's the diff?
		dTestDiff = dTestRange - dRange;

		// How have we done?
		// < 5% off...
		if ( fabs( ( dTestDiff / dRange ) ) < .01 )
		{
			break;
		}

		if ( iNumChecks > MAX_INTEGRATIONS )
		{
			break;
		}

		// What is the Percentage difference?
		dPercentDiff = dForce * ( dTestDiff / dRange );

		// Adjust force accordingly
		dForce = dForce - ( ( dPercentDiff ) / 2 );

	} while( TRUE );

	// OK, we have our force, calculate change to get through without collide
	//if ( ChanceToGetThroughObjectTrajectory( sEndZ, pItem, &vPosition, &vForce, NULL ) == 0 )
	{
		//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Chance to get through throw is 0." );
	}

	if ( pdMagForce )
	{
		(*pdMagForce) = dForce;
	}
	SLOGD("Number of integration: {}", iNumChecks);

	return( vForce );
}


static float FindBestAngleForTrajectory(INT16 sSrcGridNo, INT16 sGridNo, INT16 sStartZ, INT16 sEndZ, float dForce, const OBJECTTYPE* pItem, INT16* psGridNo)
{
	vector_3 vDirNormal, vPosition, vForce;
	INT16    sDestX, sDestY, sSrcX, sSrcY;
	float    dRange;
	float    dzDegrees = ( (float)PI/8 );
	float    dPercentDiff = 0;
	float    dTestRange, dTestDiff;
	INT32    iNumChecks = 0;


	// Get XY from gridno
	ConvertGridNoToCenterCellXY( sGridNo, &sDestX, &sDestY );
	ConvertGridNoToCenterCellXY( sSrcGridNo, &sSrcX, &sSrcY );

	// Set position
	vPosition.x = sSrcX;
	vPosition.y = sSrcY;
	vPosition.z = sStartZ;

	// OK, get direction normal
	vDirNormal.x = (float)(sDestX - sSrcX);
	vDirNormal.y = (float)(sDestY - sSrcY);
	vDirNormal.z = 0;

	// NOmralize
	vDirNormal = VGetNormal( &vDirNormal );

	// From degrees, calculate Z portion of normal
	vDirNormal.z = (float)sin( dzDegrees );

	// Get range
	dRange = (float)GetRangeInCellCoordsFromGridNoDiff( sGridNo, sSrcGridNo );

	do
	{
		// This first direction is just an estimate...
		// now do a binary search to find best value....
		iNumChecks++;

		// Now use a force
		vForce.x = dForce * vDirNormal.x;
		vForce.y = dForce * vDirNormal.y;
		vForce.z = dForce * vDirNormal.z;

		dTestRange = CalculateObjectTrajectory( sEndZ, pItem, &vPosition, &vForce, psGridNo );

		// What's the diff?
		dTestDiff = dTestRange - dRange;

		// How have we done?
		// < 5% off...
		if ( fabs( (FLOAT)( dTestDiff / dRange ) ) < .05 )
		{
			break;
		}

		if ( iNumChecks > MAX_INTEGRATIONS )
		{
			break;
		}

		// What is the Percentage difference?
		dPercentDiff = dzDegrees * ( dTestDiff / dRange );

		// Adjust degrees accordingly
		dzDegrees = dzDegrees - ( dPercentDiff / 2 );

		// OK, If our angle is too far either way, giveup!
		if ( fabs( dzDegrees ) >= ( PI / 2 ) || fabs( dzDegrees ) <= 0.005 )
		{
			// Use 0.....
			dzDegrees = 0;
			// From degrees, calculate Z portion of normal
			vDirNormal.z	= (float)sin( dzDegrees );
			// Now use a force
			vForce.x = dForce * vDirNormal.x;
			vForce.y = dForce * vDirNormal.y;
			vForce.z = dForce * vDirNormal.z;
			dTestRange = CalculateObjectTrajectory( sEndZ, pItem, &vPosition, &vForce, psGridNo );
			return( (FLOAT)( dzDegrees ) );
		}


		// From degrees, calculate Z portion of normal
		vDirNormal.z = (float)sin( dzDegrees );

	} while( TRUE );

	// OK, we have our force, calculate change to get through without collide
	//if ( ChanceToGetThroughObjectTrajectory( sEndZ, pItem, &vPosition, &vForce ) == 0 )
	//{
	//	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Chance to get through throw is 0." );
	//}

	return( dzDegrees );
}


// OK, this will, given a target Z, INVTYPE, source, target gridnos, initial force vector, will
// return range
static FLOAT CalculateObjectTrajectory(INT16 sTargetZ, const OBJECTTYPE* pItem, vector_3* vPosition, vector_3* vForce, INT16* psFinalGridNo)
{
	FLOAT dDiffX, dDiffY;

	if ( psFinalGridNo )
	{
		(*psFinalGridNo) = NOWHERE;
	}

	REAL_OBJECT* const pObject = CreatePhysicalObject(pItem, -1, vPosition->x, vPosition->y, vPosition->z, vForce->x, vForce->y, vForce->z, NULL, NO_THROW_ACTION, 0);

	// Set some special values...
	pObject->fTestObject = TEST_OBJECT_NO_COLLISIONS;
	pObject->TestZTarget = sTargetZ;
	pObject->fTestPositionNotSet = TRUE;
	pObject->fVisible = FALSE;

	// Alrighty, move this beast until it dies....
	while( pObject->fAlive )
	{
		SimulateObject( pObject, (float)DELTA_T );
	}

	// Calculate gridno from last position
	GridNo const sGridNo = vector_3ToGridNo(pObject->Position);

	PhysicsDeleteObject( pObject );

	// get new x, y, z values
	dDiffX = ( pObject->TestTargetPosition.x - vPosition->x );
	dDiffY = ( pObject->TestTargetPosition.y - vPosition->y );

	if ( psFinalGridNo )
	{
		(*psFinalGridNo) = sGridNo;
	}

	return std::hypotf(dDiffX, dDiffY);
}


static INT32 ChanceToGetThroughObjectTrajectory(INT16 sTargetZ, const OBJECTTYPE* pItem, vector_3* vPosition, vector_3* vForce, INT16* psNewGridNo, INT8* pbLevel, BOOLEAN fFromUI)
{
	REAL_OBJECT* const pObject = CreatePhysicalObject(pItem, -1, vPosition->x, vPosition->y, vPosition->z, vForce->x, vForce->y, vForce->z, NULL, NO_THROW_ACTION, 0);

	// Set some special values...
	pObject->fTestObject = TEST_OBJECT_NOTWALLROOF_COLLISIONS;
	pObject->fTestPositionNotSet = TRUE;
	pObject->TestZTarget = sTargetZ;
	pObject->fVisible = FALSE;
	//pObject->fPotentialForDebug = TRUE;

	// Alrighty, move this beast until it dies....
	while( pObject->fAlive )
	{
		SimulateObject( pObject, (float)DELTA_T );
	}


	if ( psNewGridNo != NULL )
	{
		// Calculate gridno from last position

		// If NOT from UI, use exact collision position
		*psNewGridNo = vector_3ToGridNo(fFromUI ? pObject->Position : pObject->EndedWithCollisionPosition);
		(*pbLevel) = GET_OBJECT_LEVEL( pObject->EndedWithCollisionPosition.z - CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[ (*psNewGridNo) ].sHeight ) );
	}

	PhysicsDeleteObject( pObject );

	// See If we collided
	if ( pObject->fTestEndedWithCollision )
	{
		return( 0 );
	}
	return( 100 );
}


static FLOAT CalculateForceFromRange(INT16 sRange, FLOAT dDegrees);
static FLOAT CalculateSoldierMaxForce(const SOLDIERTYPE* pSoldier, FLOAT dDegrees, const OBJECTTYPE* pItem, BOOLEAN fArmed);


static void CalculateLaunchItemBasicParams(const SOLDIERTYPE* pSoldier, const OBJECTTYPE* pItem, INT16 sGridNo, UINT8 ubLevel, INT16 sEndZ,  FLOAT* pdMagForce, FLOAT* pdDegrees, INT16* psFinalGridNo, BOOLEAN fArmed)
{
	INT16   sInterGridNo;
	FLOAT   dMagForce, dMaxForce, dMinForce;
	BOOLEAN fThroughIntermediateGridNo = FALSE;
	BOOLEAN fIndoors = FALSE;
	BOOLEAN fMortar = FALSE;
	BOOLEAN fGLauncher = FALSE;
	INT16   sMinRange = 0;

	// Start with default degrees/ force
	float dDegrees = OUTDOORS_START_ANGLE;
	INT16 sStartZ  = GET_SOLDIER_THROW_HEIGHT( pSoldier->bLevel );

	// Are we armed, and are we throwing a LAUNCHABLE?

	UINT16 const usLauncher = GetLauncherFromLaunchable(pItem->usItem);

	if ( fArmed && ( usLauncher == MORTAR || pItem->usItem == MORTAR ) )
	{
		// Start at 0....
		sStartZ = ( pSoldier->bLevel * 256 );
		fMortar = TRUE;
		sMinRange = MIN_MORTAR_RANGE;
	}

	if ( fArmed && ( usLauncher == GLAUNCHER || usLauncher == UNDER_GLAUNCHER || pItem->usItem == GLAUNCHER || pItem->usItem == UNDER_GLAUNCHER ) )
	{
		// OK, look at target level and decide angle to use...
		if ( ubLevel == 1 )
		{
			//dDegrees  = GLAUNCHER_START_ANGLE;
			dDegrees  = GLAUNCHER_HIGHER_LEVEL_START_ANGLE;
		}
		else
		{
			dDegrees  = GLAUNCHER_START_ANGLE;
		}
		fGLauncher = TRUE;
		sMinRange  = MIN_MORTAR_RANGE;
	}

	// CHANGE DEGREE VALUES BASED ON IF WE ARE INSIDE, ETC
	// ARE WE INSIDE?

	if ( gfCaves || gfBasement  )
	{
		// Adjust angle....
		dDegrees = INDOORS_START_ANGLE;
		fIndoors = TRUE;
	}

	if ( ( IsRoofPresentAtGridno( pSoldier->sGridNo ) ) && pSoldier->bLevel == 0 )
	{
		// Adjust angle....
		dDegrees = INDOORS_START_ANGLE;
		fIndoors = TRUE;
	}

	// IS OUR TARGET INSIDE?
	if ( IsRoofPresentAtGridno( sGridNo ) && ubLevel == 0 )
	{
		// Adjust angle....
		dDegrees = INDOORS_START_ANGLE;
		fIndoors = TRUE;
	}


	// OK, look if we can go through a windows here...
	if ( ubLevel == 0 )
	{
		sInterGridNo = SoldierToLocationWindowTest( pSoldier, sGridNo );
	}
	else
	{
		sInterGridNo = NOWHERE;
	}

	if ( sInterGridNo != NOWHERE )
	{
		// IF so, adjust target height, gridno....
		SLOGD("Through a window!" );

		fThroughIntermediateGridNo = TRUE;
	}

	// Find force for basic
	FindBestForceForTrajectory( pSoldier->sGridNo, sGridNo, sStartZ, sEndZ, dDegrees, pItem, psFinalGridNo, &dMagForce );

	// Adjust due to max range....
	dMaxForce   = CalculateSoldierMaxForce( pSoldier, dDegrees, pItem, fArmed );

	if ( fIndoors )
	{
		dMaxForce = dMaxForce * 2;
	}

	if ( dMagForce > dMaxForce )
	{
		dMagForce = dMaxForce;
	}

	// ATE: If we are a mortar, make sure we are at min.
	if ( fMortar || fGLauncher )
	{
		// find min force
		dMinForce = CalculateForceFromRange(sMinRange / 10, float(PI / 4));

		if ( dMagForce < dMinForce )
		{
			dMagForce = dMinForce;
		}
	}

	if ( fThroughIntermediateGridNo )
	{
		// Given this power, now try and go through this window....
		dDegrees = FindBestAngleForTrajectory( pSoldier->sGridNo, sInterGridNo, GET_SOLDIER_THROW_HEIGHT( pSoldier->bLevel ), 150, dMagForce, pItem, psFinalGridNo );
	}

	(*pdMagForce) = dMagForce;
	(*pdDegrees ) = dDegrees;
}


BOOLEAN CalculateLaunchItemChanceToGetThrough(const SOLDIERTYPE* pSoldier, const OBJECTTYPE* pItem, INT16 sGridNo, UINT8 ubLevel, INT16 sEndZ, INT16* psFinalGridNo, BOOLEAN fArmed, INT8* pbLevel, BOOLEAN fFromUI)
{
	FLOAT    dForce, dDegrees;
	INT16    sDestX, sDestY, sSrcX, sSrcY;
	vector_3 vForce, vPosition, vDirNormal;

	/* Prevent throwing to the same tile the thrower is standing on and only the
	 * target level differs.  This would lead to an endless loop when calculation
	 * the trajectory. */
	if (pSoldier->sGridNo == sGridNo)
	{
		*psFinalGridNo = sGridNo;
		*pbLevel       = pSoldier->bLevel;
		return FALSE;
	}

	// Ge7t basic launch params...
	CalculateLaunchItemBasicParams( pSoldier, pItem, sGridNo, ubLevel, sEndZ, &dForce, &dDegrees, psFinalGridNo, fArmed );

	// Get XY from gridno
	ConvertGridNoToCenterCellXY( sGridNo, &sDestX, &sDestY );
	ConvertGridNoToCenterCellXY( pSoldier->sGridNo, &sSrcX, &sSrcY );

	// Set position
	vPosition.x = sSrcX;
	vPosition.y = sSrcY;
	vPosition.z = GET_SOLDIER_THROW_HEIGHT( pSoldier->bLevel );

	// OK, get direction normal
	vDirNormal.x = (float)(sDestX - sSrcX);
	vDirNormal.y = (float)(sDestY - sSrcY);
	vDirNormal.z = 0;

	// NOmralize
	vDirNormal = VGetNormal( &vDirNormal );

	// From degrees, calculate Z portion of normal
	vDirNormal.z = (float)sin( dDegrees );

	// Do force....
	vForce.x = dForce * vDirNormal.x;
	vForce.y = dForce * vDirNormal.y;
	vForce.z = dForce * vDirNormal.z;

	// OK, we have our force, calculate change to get through without collide
	if ( ChanceToGetThroughObjectTrajectory( sEndZ, pItem, &vPosition, &vForce, psFinalGridNo, pbLevel, fFromUI ) == 0 )
	{
		return( FALSE );
	}

	if ( (*pbLevel) != ubLevel )
	{
		return( FALSE );
	}

	if ( !fFromUI && (*psFinalGridNo) != sGridNo )
	{
		return( FALSE );
	}

	return( TRUE );
}


static FLOAT CalculateForceFromRange(INT16 sRange, FLOAT dDegrees)
{
	FLOAT      dMagForce;
	INT16      sSrcGridNo, sDestGridNo;
	OBJECTTYPE Object;
	INT16      sFinalGridNo;

	// OK, use a fake gridno, find the new gridno based on range, use height of merc, end height of ground,
	// 45 degrees
	sSrcGridNo  = 4408;
	sDestGridNo = 4408 + ( sRange * WORLD_COLS );

	// Use a grenade objecttype
	CreateItem( HAND_GRENADE, 100, &Object );

	FindBestForceForTrajectory( sSrcGridNo, sDestGridNo, GET_SOLDIER_THROW_HEIGHT( 0 ), 0, dDegrees, &Object, &sFinalGridNo, &dMagForce );

	return( dMagForce );
}


static FLOAT CalculateSoldierMaxForce(const SOLDIERTYPE* pSoldier, FLOAT dDegrees, const OBJECTTYPE* pItem, BOOLEAN fArmed)
{
	INT32 uiMaxRange;
	FLOAT dMagForce;

	dDegrees = (FLOAT)( PI/4 );

	uiMaxRange = CalcMaxTossRange( pSoldier, pItem->usItem, fArmed );

	dMagForce = CalculateForceFromRange( (INT16) uiMaxRange, dDegrees );

	return( dMagForce );
}


static UINT16 RandomGridFromRadius(INT16 sSweetGridNo, INT8 ubMinRadius, INT8 ubMaxRadius);


void CalculateLaunchItemParamsForThrow(SOLDIERTYPE* const pSoldier, INT16 sGridNo, const UINT8 ubLevel, const INT16 sEndZ, OBJECTTYPE* const pItem, INT8 bMissBy, const UINT8 ubActionCode, SOLDIERTYPE* const target)
{
	constexpr INT8 MAX_MISS_BY = 30;
	constexpr INT8 MIN_MISS_BY = 1;

	FLOAT    dForce, dDegrees;
	INT16    sDestX, sDestY, sSrcX, sSrcY;
	vector_3 vForce, vDirNormal;
	INT16    sFinalGridNo;
	BOOLEAN  fArmed = FALSE;
	UINT16   usLauncher;
	INT16    sStartZ;
	INT8     bMinMissRadius, bMaxMissRadius, bMaxRadius;
	FLOAT    fScale;

	// Set target if anyone
	pSoldier->target = WhoIsThere2(sGridNo, ubLevel);

	if ( ubActionCode == THROW_ARM_ITEM )
	{
		fArmed = TRUE;
	}

	if ( bMissBy < 0 )
	{
		// then we hit!
		bMissBy = 0;
	}

	if ( bMissBy > 0 )
	{
		// Min/Max the miss variance
		bMissBy = std::clamp(bMissBy, MIN_MISS_BY, MAX_MISS_BY);

		// Adjust position, force, angle
		SLOGD("Throw miss by: {}", bMissBy);

		// Default to max radius...
		bMaxRadius = 5;

		// scale if pyth spaces away is too far
		if ( PythSpacesAway( sGridNo, pSoldier->sGridNo ) < ( (float)bMaxRadius / (float)1.5 ) )
		{
			bMaxRadius = PythSpacesAway( sGridNo, pSoldier->sGridNo ) / 2;
		}


		// Get radius
		fScale = ( (float)bMissBy / (float) MAX_MISS_BY );

		bMaxMissRadius = (INT8)( bMaxRadius * fScale );

		// Limit max radius...
		if ( bMaxMissRadius > 4 )
		{
			bMaxMissRadius = 4;
		}


		bMinMissRadius = bMaxMissRadius - 1;

		if ( bMinMissRadius < 2 )
		{
			bMinMissRadius = 2;
		}

		if ( bMaxMissRadius < bMinMissRadius )
		{
			bMaxMissRadius = bMinMissRadius;
		}

		sGridNo = RandomGridFromRadius( sGridNo, bMinMissRadius, bMaxMissRadius );
	}

	// Get basic launch params...
	CalculateLaunchItemBasicParams( pSoldier, pItem, sGridNo, ubLevel, sEndZ, &dForce, &dDegrees, &sFinalGridNo, fArmed );

	// Get XY from gridno
	ConvertGridNoToCenterCellXY( sGridNo, &sDestX, &sDestY );
	ConvertGridNoToCenterCellXY( pSoldier->sGridNo, &sSrcX, &sSrcY );

	// OK, get direction normal
	vDirNormal.x = (float)(sDestX - sSrcX);
	vDirNormal.y = (float)(sDestY - sSrcY);
	vDirNormal.z = 0;

	// NOmralize
	vDirNormal = VGetNormal( &vDirNormal );

	// From degrees, calculate Z portion of normal
	vDirNormal.z = (float)sin( dDegrees );

	// Do force....
	vForce.x = dForce * vDirNormal.x;
	vForce.y = dForce * vDirNormal.y;
	vForce.z = dForce * vDirNormal.z;


	// Allocate Throw Parameters
	pSoldier->pThrowParams = new THROW_PARAMS{};

	pSoldier->pTempObject  = new OBJECTTYPE{};

	*pSoldier->pTempObject = *pItem;
	pSoldier->pThrowParams->dX = (float)sSrcX;
	pSoldier->pThrowParams->dY = (float)sSrcY;


	sStartZ = GET_SOLDIER_THROW_HEIGHT( pSoldier->bLevel );
	usLauncher = GetLauncherFromLaunchable( pItem->usItem );
	if ( fArmed && usLauncher == MORTAR )
	{
		// Start at 0....
		sStartZ = ( pSoldier->bLevel * 256 ) + 50;
	}

	pSoldier->pThrowParams->dZ = (float)sStartZ;
	pSoldier->pThrowParams->dForceX = vForce.x;
	pSoldier->pThrowParams->dForceY = vForce.y;
	pSoldier->pThrowParams->dForceZ = vForce.z;
	pSoldier->pThrowParams->dLifeSpan = -1;
	pSoldier->pThrowParams->ubActionCode = ubActionCode;
	pSoldier->pThrowParams->target       = target;

	// Dirty interface
	DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );

}


static BOOLEAN DoCatchObject(REAL_OBJECT* pObject);


static BOOLEAN CheckForCatcher(REAL_OBJECT* const o, UINT16 const structure_id)
{
	// Do we want to catch?
	if (o->fTestObject  != NO_TEST_OBJECT)          return FALSE;
	if (o->ubActionCode != THROW_TARGET_MERC_CATCH) return FALSE;
	// Is it a guy?
	if (structure_id    >= INVALID_STRUCTURE_ID)    return FALSE;
	// Is it the same guy?
	if (o->target       != &GetMan(structure_id))   return FALSE;
	if (!DoCatchObject(o))                          return FALSE;

	o->fAlive = FALSE;
	return TRUE;
}


static void CheckForObjectHittingMerc(REAL_OBJECT* const o, UINT16 const structure_id)
{
	// Do we want to catch?
	if (o->fTestObject != NO_TEST_OBJECT)             return;
	// Is it a guy?
	if (structure_id   >= INVALID_STRUCTURE_ID)       return;
	if (structure_id   == o->ubLastTargetTakenDamage) return;

	SOLDIERTYPE& s      = GetMan(structure_id);
	INT16 const  damage = 1;
	INT16 const  breath = 0;
	EVENT_SoldierGotHit(&s, NOTHING, damage, breath, s.bDirection, 0, o->owner, FIRE_WEAPON_TOSSED_OBJECT_SPECIAL, 0, NOWHERE);

	o->ubLastTargetTakenDamage = structure_id;
}


static BOOLEAN AttemptToCatchObject(REAL_OBJECT* pObject);


static BOOLEAN CheckForCatchObject(REAL_OBJECT* pObject)
{
	UINT32 uiSpacesAway;

	// Do we want to catch?
	if ( pObject->fTestObject ==  NO_TEST_OBJECT )
	{
		if ( pObject->ubActionCode == THROW_TARGET_MERC_CATCH )
		{
			SOLDIERTYPE* const pSoldier = pObject->target;

			// Is it a guy?
			// Are we close to this guy?
			uiSpacesAway = PythSpacesAway( pObject->sGridNo, pSoldier->sGridNo );

			if ( uiSpacesAway < 4 && !pObject->fAttemptedCatch )
			{
				if ( pSoldier->usAnimState != CATCH_STANDING &&
					pSoldier->usAnimState != CATCH_CROUCHED &&
					pSoldier->usAnimState != LOWER_RIFLE )
				{
					if ( gAnimControl[ pSoldier->usAnimState ].ubHeight == ANIM_STAND )
					{
						EVENT_InitNewSoldierAnim( pSoldier, CATCH_STANDING, 0 , FALSE );
					}
					else if ( gAnimControl[ pSoldier->usAnimState ].ubHeight == ANIM_CROUCH )
					{
						EVENT_InitNewSoldierAnim( pSoldier, CATCH_CROUCHED, 0 , FALSE );
					}

					pObject->fCatchAnimOn = TRUE;
				}
			}

			pObject->fAttemptedCatch = TRUE;

			if ( uiSpacesAway <= 1 && !pObject->fCatchCheckDone )
			{
				if ( AttemptToCatchObject( pObject ) )
				{
					return( TRUE );
				}
			}
		}
	}
	return( FALSE );
}


static BOOLEAN AttemptToCatchObject(REAL_OBJECT* pObject)
{
	UINT8 ubChanceToCatch;

	// OK, get chance to catch
	// base it on...? CC? Dexterity?
	ubChanceToCatch = 50 + EffectiveDexterity(pObject->target) / 2;

	SLOGD("Chance To Catch: {}", ubChanceToCatch);

	pObject->fCatchCheckDone = TRUE;

	if ( PreRandom( 100 ) > ubChanceToCatch )
	{
		return( FALSE );
	}

	pObject->fCatchGood = TRUE;

	return( TRUE );
}


static BOOLEAN DoCatchObject(REAL_OBJECT* pObject)
{
	BOOLEAN fGoodCatch = FALSE;
	UINT16  usItem;

	// Get intended target
	SOLDIERTYPE* const pSoldier = pObject->target;

	// Catch anim.....
	switch( gAnimControl[ pSoldier->usAnimState ].ubHeight )
	{
		case ANIM_STAND:

			pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
			EVENT_InitNewSoldierAnim( pSoldier, END_CATCH, 0 , FALSE );
			break;

		case ANIM_CROUCH:

			pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
			EVENT_InitNewSoldierAnim( pSoldier, END_CROUCH_CATCH, 0 , FALSE );
			break;
	}

	PlayLocationJA2Sample(pSoldier->sGridNo, CATCH_OBJECT, MIDVOLUME, 1);

	pObject->fCatchAnimOn = FALSE;

	if ( !pObject->fCatchGood )
	{
		return( FALSE );
	}

	// Get item
	usItem = pObject->Obj.usItem;

	// Transfer object
	fGoodCatch = AutoPlaceObject( pSoldier, &(pObject->Obj), TRUE );

	// Report success....
	if ( fGoodCatch )
	{
		pObject->fDropItem = FALSE;

		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(pMessageStrings[ MSG_MERC_CAUGHT_ITEM ], pSoldier->name, GCM->getItem(usItem)->getShortName()) );
	}

	return( TRUE );
}


//#define TESTDUDEXPLOSIVES


static void HandleArmedObjectImpact(REAL_OBJECT* pObject)
{
	INT16      sZ;
	BOOLEAN    fDoImpact = FALSE;
	BOOLEAN    fCheckForDuds = FALSE;
	OBJECTTYPE *pObj;
	INT32      iTrapped = 0;
	UINT16     usFlags = 0;
	INT8       bLevel = 0;

	// Calculate pixel position of z
	sZ = (INT16)CONVERT_HEIGHTUNITS_TO_PIXELS( (INT16)( pObject->Position.z ) ) - gpWorldLevelData[ pObject->sGridNo ].sHeight;

	// get OBJECTTYPE
	pObj = &(pObject->Obj);

	// ATE: Make sure number of objects is 1...
	pObj->ubNumberOfObjects = 1;

	if ( GCM->getItem(pObj->usItem)->isGrenade()  )
	{
		fCheckForDuds = TRUE;
	}

	if ( pObj->usItem == MORTAR_SHELL )
	{
		fCheckForDuds = TRUE;
	}

	if ( GCM->getItem(pObj->usItem)->isThrown()  )
	{
		AddItemToPool( pObject->sGridNo, pObj, INVISIBLE, bLevel, usFlags, 0 );
	}

	if ( fCheckForDuds )
	{
		// If we landed on anything other than the floor, always! go off...
	#ifdef TESTDUDEXPLOSIVES
		if ( sZ != 0 || pObject->fInWater )
	#else
		if ( sZ != 0 || pObject->fInWater || ( pObj->bStatus[0] >= USABLE && ( PreRandom( 100 ) < (UINT32) pObj->bStatus[0] + PreRandom( 50 ) ) ) )
	#endif
		{
			fDoImpact = TRUE;
		}
		else	// didn't go off!
		{
	#ifdef TESTDUDEXPLOSIVES
			if ( 1 )
	#else
			if ( pObj->bStatus[0] >= USABLE && PreRandom(100) < (UINT32) pObj->bStatus[0] + PreRandom( 50 ) )
	#endif
			{
				iTrapped = PreRandom( 4 ) + 2;
			}

			if ( iTrapped )
			{
				// Start timed bomb...
				usFlags |= WORLD_ITEM_ARMED_BOMB;

				pObj->bDetonatorType = BOMB_TIMED;
				pObj->bDelay = (INT8)( 1 + PreRandom( 2 ) );
			}

			// ATE: If we have collided with roof last...
			if ( pObject->iOldCollisionCode == COLLISION_ROOF )
			{
				bLevel = 1;
			}

			// Add item to pool....
			AddItemToPool( pObject->sGridNo, pObj, INVISIBLE, bLevel, usFlags, 0 );

			// All teams lok for this...
			NotifySoldiersToLookforItems( );

			if (pObject->owner != NULL)
			{
				DoMercBattleSound(pObject->owner, BATTLE_SOUND_CURSE1);
			}
		}
	}
	else
	{
		fDoImpact = TRUE;
	}

	if ( fDoImpact )
	{
		if ( pObject->Obj.usItem == BREAK_LIGHT )
		{
			//if the light object will be created OFF the ground
			if (pObject->Position.z > 0)
			{
				//we cannot create the light source above the ground, or on a roof.  The system doesnt support it.
				AddItemToPool(pObject->sGridNo, &(pObject->Obj), VISIBLE, 1, 0, -1);
			}
			else
			{
				// Add a light effect...
				NewLightEffect(pObject->sGridNo, LIGHT_FLARE_MARK_1);
			}
		}
		else if ( GCM->getItem(pObject->Obj.usItem)->isGrenade()  )
		{
/* ARM: Removed.  Rewards even missed throws, and pulling a pin doesn't really teach anything about explosives
			if (pObject->owner->bTeam == OUR_TEAM && gTacticalStatus.uiFlags & INCOMBAT)
			{
				// tossed grenade, not a dud, so grant xp
				// EXPLOSIVES GAIN (10):  Tossing grenade
				if (pObject->owner != NULL)
				{
					StatChange(*pObject->owner, EXPLODEAMT, 10, FALSE);
				}
			}
*/

			IgniteExplosionXY(pObject->owner, pObject->Position.x, pObject->Position.y, sZ, pObject->sGridNo, pObject->Obj.usItem, GET_OBJECT_LEVEL(pObject->Position.z - CONVERT_PIXELS_TO_HEIGHTUNITS(gpWorldLevelData[pObject->sGridNo].sHeight)));
		}
		else if ( pObject->Obj.usItem == MORTAR_SHELL )
		{
			sZ = (INT16)CONVERT_HEIGHTUNITS_TO_PIXELS( (INT16)pObject->Position.z );

			IgniteExplosionXY(pObject->owner, pObject->Position.x, pObject->Position.y, sZ, pObject->sGridNo, pObject->Obj.usItem, GET_OBJECT_LEVEL(pObject->Position.z - CONVERT_PIXELS_TO_HEIGHTUNITS(gpWorldLevelData[pObject->sGridNo].sHeight)));
		}
	}
}


void SavePhysicsTableToSaveGameFile(HWFILE const hFile)
{
	UINT16 usCnt=0;
	UINT32 usPhysicsCount=0;

	for( usCnt=0; usCnt<NUM_OBJECT_SLOTS; usCnt++ )
	{
		//if the REAL_OBJECT is active, save it
		if( ObjectSlots[ usCnt ].fAllocated )
		{
			usPhysicsCount++;
		}
	}


	//Save the number of REAL_OBJECTs in the array
	hFile->write(&usPhysicsCount, sizeof(UINT32));

	if( usPhysicsCount != 0 )
	{
		for( usCnt=0; usCnt<NUM_OBJECT_SLOTS; usCnt++ )
		{
			const REAL_OBJECT* const o = &ObjectSlots[usCnt];
			if (o->fAllocated) InjectRealObjectIntoFile(hFile, o);
		}
	}
}


void LoadPhysicsTableFromSavedGameFile(HWFILE const hFile)
{
	UINT16 usCnt=0;

	//make sure the objects are not allocated
	std::fill_n(ObjectSlots, NUM_OBJECT_SLOTS, REAL_OBJECT{});

	//Load the number of REAL_OBJECTs in the array
	hFile->read(&guiNumObjectSlots, sizeof(UINT32));

	//loop through and add the objects
	for( usCnt=0; usCnt<guiNumObjectSlots; usCnt++ )
	{
		REAL_OBJECT* const o = &ObjectSlots[usCnt];
		ExtractRealObjectFromFile(hFile, o);
	}
}


static UINT16 RandomGridFromRadius(INT16 sSweetGridNo, INT8 ubMinRadius, INT8 ubMaxRadius)
{
	INT16  sX, sY;
	INT16  sGridNo;
	INT32  leftmost;
	UINT32 cnt = 0;

	if ( ubMaxRadius == 0 || ubMinRadius == 0 )
	{
		return( sSweetGridNo );
	}

	for (;;)
	{
		sX = (UINT16)PreRandom( ubMaxRadius );
		sY = (UINT16)PreRandom( ubMaxRadius );

		if ( ( sX < ubMinRadius || sY < ubMinRadius ) && ubMaxRadius != ubMinRadius )
		{
			continue;
		}

		if ( PreRandom( 2 ) == 0 )
		{
			sX = sX * -1;
		}

		if ( PreRandom( 2 ) == 0 )
		{
			sY = sY * -1;
		}

		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * sY ) )/ WORLD_COLS ) * WORLD_COLS;

		sGridNo = sSweetGridNo + ( WORLD_COLS * sY ) + sX;

		if ( sGridNo == sSweetGridNo )
		{
			continue;
		}

		if (++cnt > 50) return NOWHERE;

		if ( sGridNo >=0 && sGridNo < WORLD_MAX &&
			sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS ) )
		{
			return sGridNo;
		}
	}
}
