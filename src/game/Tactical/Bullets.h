#ifndef __BULLETS_H
#define __BULLETS_H

#include "JA2Types.h"
#include "Types.h"
#include "LOS.h"
#include "Tile_Animation.h"


#define BULLET_FLAG_CREATURE_SPIT      0x0001
#define BULLET_FLAG_KNIFE              0x0002
#define BULLET_FLAG_MISSILE            0x0004
#define BULLET_FLAG_SMALL_MISSILE      0x0008
#define BULLET_STOPPED                 0x0010
#define BULLET_FLAG_TANK_CANNON        0x0020
#define BULLET_FLAG_BUCKSHOT           0x0040
#define BULLET_FLAG_FLAME              0x0080
#define BULLET_FLAG_BLOODY             0x0100 // bloody knives have both bloody and knife flags

struct BULLET
{
	const SOLDIERTYPE* target;
	INT8    bStartCubesAboveLevelZ;
	INT8    bEndCubesAboveLevelZ;
	UINT16  usLastStructureHit;
	UINT32  sGridNo;
	FIXEDPT qCurrX;
	FIXEDPT qCurrY;
	FIXEDPT qCurrZ;
	FIXEDPT qIncrX;
	FIXEDPT qIncrY;
	FIXEDPT qIncrZ;
	double  ddHorizAngle;
	INT32   iCurrTileX;
	INT32   iCurrTileY;
	INT8    bLOSIndexX;
	INT8    bLOSIndexY;
	BOOLEAN fCheckForRoof;
	INT32   iCurrCubesZ;
	INT32   iLoop;
	BOOLEAN fAllocated;
	BOOLEAN fToDelete;
	BOOLEAN fLocated;
	BOOLEAN fReal;
	BOOLEAN fAimed;
	UINT32  uiLastUpdate;
	UINT8   ubTilesPerUpdate;
	UINT16  usClockTicksPerUpdate;
	SOLDIERTYPE *pFirer;
	UINT32  sTargetGridNo;
	INT16   sHitBy;
	INT32   iImpact;
	INT32   iImpactReduction;
	INT32   iRange;
	INT32   iDistanceLimit;
	UINT16  usFlags;
	ANITILE *pAniTile;
	ANITILE *pShadowAniTile;
	UINT8   ubItemStatus;
};

extern UINT32 guiNumBullets;

BULLET* CreateBullet(SOLDIERTYPE* firer, BOOLEAN fFake, UINT16 usFlags);
void    RemoveBullet(BULLET* b);
void    StopBullet(BULLET* b);
void    UpdateBullets(void);

void DeleteAllBullets(void);

void LocateBullet(BULLET* b);

void HandleBulletSpecialFlags(BULLET* b);

void AddMissileTrail( BULLET *pBullet, FIXEDPT qCurrX, FIXEDPT qCurrY, FIXEDPT qCurrZ );

//Save the bullet table to the saved game file
void SaveBulletStructureToSaveGameFile(HWFILE);

//Load the bullet table from the saved game file
void LoadBulletStructureFromSavedGameFile(HWFILE);

#endif
