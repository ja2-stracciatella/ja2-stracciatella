#ifndef SOUND_CONTROL_H
#define SOUND_CONTROL_H

#include "JA2Types.h"
#include "Random.h"

#include <string_theory/string>


#define FARLEFT	0
#define LEFTSIDE	48
#define MIDDLE		64
#define MIDDLEPAN	64
#define RIGHTSIDE	80
#define FARRIGHT	127


#define LOWVOLUME	25
#define BTNVOLUME	40
#define MIDVOLUME	65
#define HIGHVOLUME	127


#define LOOPING	0


// SOUNDS ENUMERATION
enum SoundID
{
	NO_SOUND = -1,
	MISS_1 = 0,                   // 0
	MISS_2,                       // 1
	MISS_3,                       // 2
	MISS_4,                       // 3
	MISS_5,                       // 4
	MISS_6,                       // 5
	MISS_7,                       // 6
	MISS_8,                       // 7
	MISS_G1,                      // 8
	MISS_G2,                      // 9
	MISS_KNIFE,                   // 10
	FALL_1,                       // 11
	FALL_2,                       // 12
	FALL_TO_GROUND_1,             // 13
	FALL_TO_GROUND_2,             // 14
	FALL_TO_GROUND_3,             // 15
	HEAVY_FALL_1,                 // 16
	BODY_SPLAT_1,                 // 17
	GLASS_SHATTER1,               // 18
	GLASS_SHATTER2,               // 19
	DROPEN_1,                     // 20
	DROPEN_2,                     // 21
	DROPEN_3,                     // 22
	DRCLOSE_1,                    // 23
	DRCLOSE_2,                    // 24
	UNLOCK_DOOR_1,                // 25
	KICKIN_DOOR,                  // 26
	BREAK_LOCK,                   // 27
	PICKING_LOCK,                 // 28
	GARAGE_DOOR_OPEN,             // 29
	GARAGE_DOOR_CLOSE,            // 30
	S_UNUSED_31,                  // 31
	S_UNUSED_32,                  // 32
	S_UNUSED_33,                  // 33
	S_UNUSED_34,                  // 34
	CURTAINS_OPEN,                // 35
	CURTAINS_CLOSE,               // 36
	METAL_DOOR_OPEN,              // 37
	METAL_DOOR_CLOSE,             // 38
	WALK_LEFT_OUT,                // 39
	WALK_RIGHT_OUT,               // 40
	WALK_LEFT_OUT2,               // 41
	WALK_RIGHT_OUT2,              // 42
	WALK_LEFT_IN,                 // 43
	WALK_RIGHT_IN,                // 44
	WALK_LEFT_IN2,                // 45
	WALK_RIGHT_IN2,               // 46
	WALK_LEFT_ROAD,               // 47
	WALK_RIGHT_ROAD,              // 48
	WALK_LEFT_ROAD2,              // 49
	WALK_RIGHT_ROAD2,             // 50
	CRAWL_1,                      // 51
	CRAWL_2,                      // 52
	CRAWL_3,                      // 53
	CRAWL_4,                      // 54
	TARG_REFINE_BEEP,             // 55
	ENDTURN_1,                    // 56
	HEADCR_1,                     // 57
	DOORCR_1,                     // 58
	HEADSPLAT_1,                  // 59
	BODY_EXPLODE_1,               // 60
	EXPLOSION_1,                  // 61
	CROW_EXPLODE_1,               // 62
	SMALL_EXPLODE_1,              // 63
	HELI_1,                       // 64
	BULLET_IMPACT_1,              // 65
	BULLET_IMPACT_2,              // 66
	BULLET_IMPACT_3,              // 67
	CREATURE_BATTLECRY_1,         // 68
	ENTER_WATER_1,                // 69
	ENTER_DEEP_WATER_1,           // 70
	COW_HIT_SND,                  // 71
	COW_DIE_SND,                  // 72
	RG_ID_IMPRINTED,              // 73
	RG_ID_INVALID,                // 74
	RG_TARGET_SELECTED,           // 75
	CAVE_COLLAPSE,                // 76
	S_RAID_WHISTLE,               // 77
	S_RAID_AMBIENT,               // 78
	S_RAID_DIVE,                  // 79
	S_RAID_TB_DIVE,               // 80
	S_RAID_TB_BOMB,               // 81
	S_VECH1_MOVE,                 // 82
	S_VECH1_ON,                   // 83
	S_VECH1_OFF,                  // 84
	S_VECH1_INTO,                 // 85
	S_DRYFIRE1,                   // 86
	S_WOOD_IMPACT1,               // 87
	S_WOOD_IMPACT2,               // 88
	S_WOOD_IMPACT3,               // 89
	S_PORCELAIN_IMPACT1,          // 90
	S_RUBBER_IMPACT1,             // 91
	S_STONE_IMPACT1,              // 92
	S_WATER_IMPACT1,              // 93
	S_VEG_IMPACT1,                // 94
	S_METAL_IMPACT1,              // 95
	S_METAL_IMPACT2,              // 96
	S_METAL_IMPACT3,              // 97
	S_SLAP_IMPACT,                // 98
	S_RELOAD_REVOLVER,            // 99
	S_RELOAD_PISTOL,              // 100
	S_RELOAD_SMG,                 // 101
	S_RELOAD_RIFLE,               // 102
	S_RELOAD_SHOTGUN,             // 103
	S_RELOAD_LMG,                 // 104
	S_LNL_REVOLVER,               // 105
	S_LNL_PISTOL,                 // 106
	S_LNL_SMG,                    // 107
	S_LNL_RIFLE,                  // 108
	S_LNL_SHOTGUN,                // 109
	S_LNL_LMG,                    // 110
	S_UNUSED_111,                 // 111
	S_UNUSED_112,                 // 112
	S_UNUSED_113,                 // 113
	S_UNUSED_114,                 // 114
	S_UNUSED_115,                 // 115
	S_UNUSED_116,                 // 116
	S_UNUSED_117,                 // 117
	S_UNUSED_118,                 // 118
	S_UNUSED_119,                 // 119
	S_UNUSED_120,                 // 120
	S_UNUSED_121,                 // 121
	S_UNUSED_122,                 // 122
	S_UNUSED_123,                 // 123
	S_UNUSED_124,                 // 124
	S_UNUSED_125,                 // 125
	S_UNUSED_126,                 // 126
	S_UNUSED_127,                 // 127
	S_UNUSED_128,                 // 128
	S_UNUSED_129,                 // 129
	S_UNUSED_130,                 // 130
	S_UNUSED_131,                 // 131
	S_UNUSED_132,                 // 132
	S_UNUSED_133,                 // 133
	S_UNUSED_134,                 // 134
	S_UNUSED_135,                 // 135
	S_UNUSED_136,                 // 136
	S_UNUSED_137,                 // 137
	S_UNUSED_138,                 // 138
	S_UNUSED_139,                 // 139
	S_UNUSED_140,                 // 140
	S_UNUSED_141,                 // 141
	S_UNUSED_142,                 // 142
	S_UNUSED_143,                 // 143
	S_UNUSED_144,                 // 144
	S_UNUSED_145,                 // 145
	S_UNUSED_146,                 // 146
	S_UNUSED_147,                 // 147
	S_UNUSED_148,                 // 148
	S_UNUSED_149,                 // 149
	S_UNUSED_150,                 // 150
	S_UNUSED_151,                 // 151
	S_UNUSED_152,                 // 152
	S_UNUSED_153,                 // 153
	S_UNUSED_154,                 // 154
	S_UNUSED_155,                 // 155
	S_SILENCER_1,                 // 156
	S_SILENCER_2,                 // 157
	SWOOSH_1,                     // 158
	SWOOSH_2,                     // 159
	SWOOSH_3,                     // 160
	SWOOSH_4,                     // 161
	SWOOSH_5,                     // 162
	SWOOSH_6,                     // 163
	ACR_FALL_1,                   // 164
	ACR_STEP_1,                   // 165
	ACR_STEP_2,                   // 166
	ACR_SWIPE,                    // 167
	ACR_EATFLESH,                 // 168
	ACR_CRIPPLED,                 // 169
	ACR_DIE_PART1,                // 170
	ACR_DIE_PART2,                // 171
	ACR_LUNGE,                    // 172
	ACR_SMELL_THREAT,             // 173
	ACR_SMELL_PREY,               // 174
	ACR_SPIT,                     // 175
	BCR_DYING,                    // 176
	BCR_DRAGGING,                 // 177
	BCR_SHRIEK,                   // 178
	S_UNUSED_179,                 // 179
	LCR_MOVEMENT,                 // 180
	LCR_RUPTURE,                  // 181
	LQ_SHRIEK,                    // 182
	LQ_DYING,                     // 183
	LQ_ENRAGED_ATTACK,            // 184
	LQ_RUPTURING,                 // 185
	LQ_CRIPPLED,                  // 186
	LQ_SMELLS_THREAT,             // 187
	LQ_WHIP_ATTACK,               // 188
	THROW_IMPACT_1,               // 189
	THROW_IMPACT_2,               // 190
	IDLE_SCRATCH,                 // 191
	IDLE_ARMPIT,                  // 192
	IDLE_BACKCRACK,               // 193
	AUTORESOLVE_FINISHFX,         // 194
	EMAIL_ALERT,                  // 195
	ENTERING_TEXT,                // 196
	REMOVING_TEXT,                // 197
	COMPUTER_BEEP2_IN,            // 198
	COMPUTER_BEEP2_OUT,           // 199
	COMPUTER_SWITCH1_IN,          // 200
	COMPUTER_SWITCH1_OUT,         // 201
	VSM_SWITCH1_IN,               // 202
	VSM_SWITCH1_OUT,              // 203
	VSM_SWITCH2_IN,               // 204
	VSM_SWITCH2_OUT,              // 205
	SM_SWITCH1_IN,                // 206
	SM_SWITCH1_OUT,               // 207
	SM_SWITCH2_IN,                // 208
	SM_SWITCH2_OUT,               // 209
	SM_SWITCH3_IN,                // 210
	SM_SWITCH3_OUT,               // 211
	BIG_SWITCH3_IN,               // 212
	BIG_SWITCH3_OUT,              // 213
	KLAXON_ALARM,                 // 214
	BOXING_BELL,                  // 215
	S_UNUSED_216,                 // 216
	ATTACH_TO_GUN,                // 217
	ATTACH_CERAMIC_PLATES,        // 218
	ATTACH_DETONATOR,             // 219
	GRAB_ROOF,                    // 220
	LAND_ON_ROOF,                 // 221
	S_UNUSED_222,                 // 222
	S_UNUSED_223,                 // 223
	S_UNUSED_224,                 // 224
	OPEN_DEFAULT_OPENABLE,        // 225
	CLOSE_DEFAULT_OPENABLE,       // 226
	FIRE_ON_MERC,                 // 227
	GLASS_CRACK,                  // 228
	SPIT_RICOCHET,                // 229
	BLOODCAT_HIT_1,               // 230
	BLOODCAT_DIE_1,               // 231
	SLAP_1,                       // 232
	ROBOT_BEEP,                   // 233
	DOOR_ELECTRICITY,             // 234
	SWIM_1,                       // 235
	SWIM_2,                       // 236
	KEY_FAILURE,                  // 237
	TARGET_OUT_OF_RANGE,          // 238
	OPEN_STATUE,                  // 239
	USE_STATUE_REMOTE,            // 240
	USE_WIRE_CUTTERS,             // 241
	DRINK_CANTEEN_FEMALE,         // 242
	BLOODCAT_ATTACK,              // 243
	BLOODCAT_ROAR,                // 244
	ROBOT_GREETING,               // 245
	ROBOT_DEATH,                  // 246
	GAS_EXPLODE_1,                // 247
	AIR_ESCAPING_1,               // 248
	OPEN_DRAWER,                  // 249
	CLOSE_DRAWER,                 // 250
	OPEN_LOCKER,                  // 251
	CLOSE_LOCKER,                 // 252
	OPEN_WOODEN_BOX,              // 253
	CLOSE_WOODEN_BOX,             // 254
	ROBOT_STOP,                   // 255
	WATER_WALK1_IN,               // 256
	WATER_WALK1_OUT,              // 257
	WATER_WALK2_IN,               // 258
	WATER_WALK2_OUT,              // 259
	PRONE_UP_SOUND,               // 260
	PRONE_DOWN_SOUND,             // 261
	KNEEL_UP_SOUND,               // 262
	KNEEL_DOWN_SOUND,             // 263
	PICKING_SOMETHING_UP,         // 264
	COW_FALL,                     // 265
	BLOODCAT_GROWL_1,             // 266
	BLOODCAT_GROWL_2,             // 267
	BLOODCAT_GROWL_3,             // 268
	BLOODCAT_GROWL_4,             // 269
	CREATURE_GAS_NOISE,           // 270
	CREATURE_FALL_PART_2,         // 271
	CREATURE_DISSOLVE_1,          // 272
	QUEEN_AMBIENT_NOISE,          // 273
	CREATURE_FALL,                // 274
	CROW_PECKING_AT_FLESH,        // 275
	CROW_FLYING_AWAY,             // 276
	SLAP_2,                       // 277
	MORTAR_START,                 // 278
	MORTAR_WHISTLE,               // 279
	MORTAR_LOAD,                  // 280
	TURRET_MOVE,                  // 281
	TURRET_STOP,                  // 282
	COW_FALL_2,                   // 283
	KNIFE_IMPACT,                 // 284
	EXPLOSION_ALT_BLAST_1,        // 285
	EXPLOSION_BLAST_2,            // 286
	DRINK_CANTEEN_MALE,           // 287
	USE_X_RAY_MACHINE,            // 288
	CATCH_OBJECT,                 // 289
	FENCE_OPEN,                   // 290

	NUM_SAMPLES
};


template<SoundID first, SoundID last> static inline SoundID SoundRange()
{
	static_assert(first < last);
	return static_cast<SoundID>(first + Random(last - first + 1));
};


enum AmbientSoundID
{
	LIGHTNING_1 = 0,
	LIGHTNING_2,
	RAIN_1,
	BIRD_1,
	BIRD_2,
	CRICKETS_1,
	CRICKETS_2,
	CRICKET_1,
	CRICKET_2,
	OWL_1,
	OWL_2,
	OWL_3,
	NIGHT_BIRD_1,
	NIGHT_BIRD_2,

	NUM_AMBIENTS
};

typedef void (*SOUND_STOP_CALLBACK)( void *pData );


void   ShutdownJA2Sound(void);
UINT32 PlayJA2Sample(const char *sample, UINT32 const ubVolume, UINT32 const ubLoops, UINT32 const uiPan);
UINT32 PlayJA2Sample(SoundID, UINT32 ubVolume, UINT32 ubLoops, UINT32 uiPan);

UINT32 PlayJA2SampleFromFile(const char* szFileName, UINT32 ubVolume, UINT32 ubLoops, UINT32 uiPan);
UINT32 PlayJA2StreamingSampleFromFile(const char* szFileName, UINT32 ubVolume, UINT32 ubLoops, UINT32 uiPan, SOUND_STOP_CALLBACK EndsCallback);

UINT32 PlayJA2Ambient(AmbientSoundID, UINT32 ubVolume, UINT32 ubLoops);

UINT32 PlayLocationJA2SampleFromFile(UINT16 grid_no, const char* filename, UINT32 base_vol, UINT32 loops);

UINT32 PlayLocationJA2Sample(UINT16 grid_no, SoundID, UINT32 base_vol, UINT32 loops);
UINT32 PlayLocationJA2Sample(UINT16 grid_no, const ST::string &sample, UINT32 base_vol, UINT32 loops);
UINT32 PlayLocationJA2StreamingSample(UINT16 grid_no, SoundID, UINT32 base_vol, UINT32 loops);
UINT32 PlaySoldierJA2Sample(SOLDIERTYPE const* s, SoundID, UINT32 base_vol, UINT32 ubLoops, BOOLEAN fCheck);


UINT32 GetSoundEffectsVolume(void);
void   SetSoundEffectsVolume( UINT32 uiNewVolume );

UINT32 GetSpeechVolume(void);
void   SetSpeechVolume( UINT32 uiNewVolume );


//Calculates a volume based on the current Speech Volume level
UINT32 CalculateSpeechVolume( UINT32 uiVolume );


//Calculates a volume based on the current Sound Effects Volume level
UINT32 CalculateSoundEffectsVolume( UINT32 uiVolume );

INT8 SoundDir( INT16 sGridNo );
INT8 SoundVolume( INT8 bInitialVolume, INT16 sGridNo );


INT32 NewPositionSnd(INT16 sGridNo, SOLDIERTYPE const* SoundSource, SoundID);
void DeletePositionSnd( INT32 iPositionSndIndex );
void SetPositionSndsActive(void);
void SetPositionSndsInActive(void);
void SetPositionSndsVolumeAndPanning(void);
void SetPositionSndGridNo( INT32 iPositionSndIndex, INT16 sGridNo );

#endif
