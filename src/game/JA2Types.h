#ifndef JA2TYPES_H
#define JA2TYPES_H

#include "Types.h"
#include <chrono>


typedef INT16 GridNo;
typedef UINT8 ProfileID;
using TIMECOUNTER = std::chrono::time_point<std::chrono::steady_clock>;

struct ANIM_PROF;
struct ANITILE;
struct BACKGROUND_SAVE;
struct BASIC_SOLDIERCREATE_STRUCT;
struct BULLET;
struct DB_STRUCTURE_REF;
struct EXPLOSIONTYPE;
struct FACETYPE;
struct GROUP;
struct ITEM_POOL;
struct LEVELNODE;
struct LIGHT_SPRITE;
struct MERCPROFILESTRUCT;
struct MercPopUpBox;
struct OBJECTTYPE;
struct PathSt;
struct PopUpBox;
struct REAL_OBJECT;
struct SMOKEEFFECT;
struct SOLDIERTYPE;
struct STRUCTURE;
struct STRUCTURE_FILE_REF;
struct TILE_IMAGERY;
struct VEHICLETYPE;
struct VIDEO_OVERLAY;

#endif
