#ifndef _OPPLIST_H
#define _OPPLIST_H

#include "Overhead_Types.h"


// For RadioSightings() parameter about
#define EVERYBODY						NULL

#define MAX_MISC_NOISE_DURATION				12 // max dur for VERY loud NOBODY noises

#define DOOR_NOISE_VOLUME       				2
#define WINDOW_CRACK_VOLUME					4
#define WINDOW_SMASH_VOLUME					8
#define MACHETE_VOLUME						9
#define TRIMMER_VOLUME						18
#define CHAINSAW_VOLUME					30
#define SMASHING_DOOR_VOLUME					6
#define CROWBAR_DOOR_VOLUME					4
#define ITEM_THROWN_VOLUME					2

#define TIME_BETWEEN_RT_OPPLIST_DECAYS				20

// this is a fake "level" value (0 on ground, 1 on roof) for
// HearNoise to ignore the effects of lighting(?)
#define LIGHT_IRRELEVANT					127

#define AUTOMATIC_INTERRUPT					100
#define NO_INTERRUPT						127

#define MOVEINTERRUPT						0
#define SIGHTINTERRUPT						1
#define NOISEINTERRUPT						2


// noise type constants
enum NoiseKind
{
	NOISE_UNKNOWN = 0,
	NOISE_MOVEMENT,
	NOISE_CREAKING,
	NOISE_SPLASHING,
	NOISE_BULLET_IMPACT,
	NOISE_GUNFIRE,
	NOISE_EXPLOSION,
	NOISE_SCREAM,
	NOISE_ROCK_IMPACT,
	NOISE_GRENADE_IMPACT,
	NOISE_WINDOW_SMASHING,
	NOISE_DOOR_SMASHING,
	NOISE_SILENT_ALARM, // only heard by enemies
	MAX_NOISES
};


#define NUM_WATCHED_LOCS 3

extern INT8  gbPublicOpplist[MAXTEAMS][ TOTAL_SOLDIERS ];
extern INT8  gbSeenOpponents[TOTAL_SOLDIERS][TOTAL_SOLDIERS];
extern INT16 gsLastKnownOppLoc[TOTAL_SOLDIERS][TOTAL_SOLDIERS]; // merc vs. merc
extern INT8  gbLastKnownOppLevel[TOTAL_SOLDIERS][TOTAL_SOLDIERS];
extern INT16 gsPublicLastKnownOppLoc[MAXTEAMS][TOTAL_SOLDIERS]; // team vs. merc
extern INT8  gbPublicLastKnownOppLevel[MAXTEAMS][TOTAL_SOLDIERS];
extern UINT8 gubPublicNoiseVolume[MAXTEAMS];
extern INT16 gsPublicNoiseGridno[MAXTEAMS];
extern INT8  gbPublicNoiseLevel[MAXTEAMS];
extern UINT8 gubKnowledgeValue[10][10];
extern INT8  gfKnowAboutOpponents;

extern BOOLEAN gfPlayerTeamSawJoey;
extern BOOLEAN gfMikeShouldSayHi;


extern INT16   gsWatchedLoc[ TOTAL_SOLDIERS ][ NUM_WATCHED_LOCS ];
extern INT8    gbWatchedLocLevel[ TOTAL_SOLDIERS ][ NUM_WATCHED_LOCS ];
extern UINT8   gubWatchedLocPoints[ TOTAL_SOLDIERS ][ NUM_WATCHED_LOCS ];
extern BOOLEAN gfWatchedLocReset[ TOTAL_SOLDIERS ][ NUM_WATCHED_LOCS ];

#define BEST_SIGHTING_ARRAY_SIZE				6
#define BEST_SIGHTING_ARRAY_SIZE_ALL_TEAMS_LOOK_FOR_ALL	6
#define BEST_SIGHTING_ARRAY_SIZE_NONCOMBAT			3
#define BEST_SIGHTING_ARRAY_SIZE_INCOMBAT			0
extern UINT8 gubBestToMakeSightingSize;

INT16 AdjustMaxSightRangeForEnvEffects(INT8 bLightLevel, INT16 sDistVisible);
void HandleSight(SOLDIERTYPE&, SightFlags);
void AllTeamsLookForAll(UINT8 ubAllowInterrupts);
void GloballyDecideWhoSeesWho(void);
UINT16 GetClosestMerc( UINT16 usSoldierIndex );
INT16 MaxDistanceVisible( void );
INT16 DistanceVisible(const SOLDIERTYPE* pSoldier, INT8 bFacingDir, INT8 bSubjectDir, INT16 sSubjectGridNo, INT8 bLevel);
void RecalculateOppCntsDueToNoLongerNeutral( SOLDIERTYPE * pSoldier );


void InitOpponentKnowledgeSystem(void);
void InitSoldierOppList(SOLDIERTYPE&);
void BetweenTurnsVisibilityAdjustments();
void RemoveManAsTarget(SOLDIERTYPE *pSoldier);
void RadioSightings(SOLDIERTYPE* pSoldier, SOLDIERTYPE* about, UINT8 ubTeamToRadioTo);
void DebugSoldierPage1(void);
void DebugSoldierPage2(void);
void DebugSoldierPage3(void);
void DebugSoldierPage4(void);

UINT8 MovementNoise( SOLDIERTYPE const *pSoldier );
UINT8 DoorOpeningNoise( SOLDIERTYPE *pSoldier );
void MakeNoise(SOLDIERTYPE* noise_maker, INT16 sGridNo, INT8 bLevel, UINT8 ubVolume, NoiseKind);
void OurNoise(SOLDIERTYPE* noise_maker, INT16 sGridNo, INT8 bLevel, UINT8 ubVolume, NoiseKind);

void ResolveInterruptsVs( SOLDIERTYPE * pSoldier, UINT8 ubInterruptType);

void VerifyAndDecayOpplist(SOLDIERTYPE *pSoldier);
void DecayIndividualOpplist(SOLDIERTYPE *pSoldier);
void VerifyPublicOpplistDueToDeath( SOLDIERTYPE * pSoldier );
void NoticeUnseenAttacker( SOLDIERTYPE * pAttacker, SOLDIERTYPE * pDefender, INT8 bReason );

bool MercSeesCreature(SOLDIERTYPE const&);

INT8 GetWatchedLocPoints( UINT8 ubID, INT16 sGridNo, INT8 bLevel );
INT8 GetHighestVisibleWatchedLoc(const SOLDIERTYPE* s);
INT8 GetHighestWatchedLocPoints(const SOLDIERTYPE* s);

void TurnOffEveryonesMuzzleFlashes( void );
void TurnOffTeamsMuzzleFlashes( UINT8 ubTeam );
void EndMuzzleFlash( SOLDIERTYPE * pSoldier );
void NonCombatDecayPublicOpplist( UINT32 uiTime );

void CheckHostileOrSayQuoteList( void );
void InitOpplistForDoorOpening( void );

void AddToShouldBecomeHostileOrSayQuoteList(SOLDIERTYPE* s);

extern INT8 gbLightSighting[1][16];

void CheckForAlertWhenEnemyDies(SOLDIERTYPE* pDyingSoldier);

extern SOLDIERTYPE* gInterruptProvoker;

extern const SOLDIERTYPE* gWhoThrewRock;

void DecayPublicOpplist(INT8 bTeam);

void RecalculateOppCntsDueToBecomingNeutral(SOLDIERTYPE* pSoldier);

#endif
