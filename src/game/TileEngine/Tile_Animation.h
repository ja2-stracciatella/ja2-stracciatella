#ifndef _TILE_ANIMATION_H
#define _TILE_ANIMATION_H

#include "JA2Types.h"
#include "Sound_Control.h"
#include "WorldDef.h"


enum AnimationFlags
{
	ANITILE_DOOR                           = 0x00000001,
	ANITILE_BACKWARD                       = 0x00000020,
	ANITILE_FORWARD                        = 0x00000040,
	ANITILE_PAUSED                         = 0x00000200,
	ANITILE_EXISTINGTILE                   = 0x00000400,
	ANITILE_LOOPING                        = 0x00020000,
	ANITILE_NOZBLITTER                     = 0x00040000,
	// 0x80000 was the unused ANITILE_REVERSE_LOOPING
	ANITILE_ALWAYS_TRANSLUCENT             = 0x00100000,
	ANITILE_OPTIMIZEFORSLOWMOVING          = 0x00400000,
	ANITILE_ANIMATE_Z                      = 0x00800000,
	ANITILE_USE_DIRECTION_FOR_START_FRAME  = 0x01000000,
	ANITILE_PAUSE_AFTER_LOOP               = 0x02000000, // XXX same value as ANITILE_USE_4DIRECTION_FOR_START_FRAME
	ANITILE_ERASEITEMFROMSAVEBUFFFER       = 0x04000000,
	ANITILE_SMOKE_EFFECT                   = 0x10000000,
	ANITILE_EXPLOSION                      = 0x20000000,
	ANITILE_RELEASE_ATTACKER_WHEN_DONE     = 0x40000000,
	ANITILE_USE_4DIRECTION_FOR_START_FRAME = 0x02000000  // XXX same value as ANITILE_PAUSE_AFTER_LOOP
};
ENUM_BITSET(AnimationFlags)


enum AnimationLevel
{
	ANI_LAND_LEVEL    = 1,
	ANI_SHADOW_LEVEL  = 2,
	ANI_OBJECT_LEVEL  = 3,
	ANI_STRUCT_LEVEL  = 4,
	ANI_ROOF_LEVEL    = 5,
	ANI_ONROOF_LEVEL  = 6,
	ANI_TOPMOST_LEVEL = 7
};


union AniUserData
{
	struct
	{
		UINT32 uiData;
		UINT32 uiData3;
	} user;
	EXPLOSIONTYPE*     explosion;
	BULLET*            bullet;
	const REAL_OBJECT* object;
	SoundID            sound;
};


struct ANITILE
{
	ANITILE*       pNext;
	AnimationFlags uiFlags; // flags struct
	UINT32         uiTimeLastUpdate; // Stuff for animated tiles

	LEVELNODE      *pLevelNode;
	UINT8          ubLevelID;
	INT16          sCurrentFrame;
	INT16          sStartFrame;
	INT16          sDelay;
	UINT16         usNumFrames;

	INT16          sRelativeX;
	INT16          sRelativeY;
	INT16          sRelativeZ;
	INT16          sGridNo;
	UINT16         usTileIndex;

	INT16          sCachedTileID; // Index into cached tile ID

	UINT8          ubKeyFrame1;
	UINT32         uiKeyFrame1Code;
	UINT8          ubKeyFrame2;
	UINT32         uiKeyFrame2Code;

	AniUserData v;

	INT8           bFrameCountAfterStart;

};


struct ANITILE_PARAMS
{
	AnimationFlags uiFlags; // flags struct
	AnimationLevel ubLevelID; // Level ID for rendering layer
	INT16          sStartFrame; // Start frame
	INT16          sDelay; // Delay time
	UINT16         usTileIndex; // Tile database index ( optional )
	INT16          sX; // World X ( optional )
	INT16          sY; // World Y ( optional )
	INT16          sZ; // World Z ( optional )
	INT16          sGridNo; // World GridNo

	LEVELNODE      *pGivenLevelNode; // Levelnode for existing tile ( optional )
	const char     *zCachedFile; // Filename for cached tile name ( optional )

	UINT8          ubKeyFrame1; // Key frame 1
	UINT32         uiKeyFrame1Code; // Key frame code
	UINT8          ubKeyFrame2; // Key frame 2
	UINT32         uiKeyFrame2Code; // Key frame code

	AniUserData v;
};


enum KeyFrameEnums
{
	ANI_KEYFRAME_NO_CODE,
	ANI_KEYFRAME_BEGIN_TRANSLUCENCY,
	ANI_KEYFRAME_BEGIN_DAMAGE,
	ANI_KEYFRAME_CHAIN_WATER_EXPLOSION,
	ANI_KEYFRAME_DO_SOUND
};


ANITILE* CreateAnimationTile(const ANITILE_PARAMS*);


void DeleteAniTile( ANITILE *pAniTile );
void UpdateAniTiles(void);
void DeleteAniTiles(void);

void HideAniTile( ANITILE *pAniTile, BOOLEAN fHide );

ANITILE* GetCachedAniTileOfType(INT16 sGridNo, UINT8 ubLevelID, AnimationFlags);

#endif
