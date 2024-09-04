#include "Soldier_Control.h"
#include "AI.h"
#include "Animation_Cache.h"
#include "Animation_Control.h"
#include "Animation_Data.h"
#include "Boxing.h"
#include "Campaign.h"
#include "Campaign_Types.h"
#include "Civ_Quotes.h"
#include "ContentManager.h"
#include "Debug.h"
#include "Dialogue_Control.h"
#include "Directories.h"
#include "Drugs_And_Alcohol.h"
#include "Event_Pump.h"
#include "Explosion_Control.h"
#include "Faces.h"
#include "Font_Control.h"
#include "GameInstance.h"
#include "GameSettings.h"
#include "Game_Clock.h"
#include "HImage.h"
#include "Handle_Doors.h"
#include "Handle_Items.h"
#include "Handle_UI.h"
#include "Interface.h"
#include "Interface_Dialogue.h"
#include "Isometric_Utils.h"
#include "Items.h"
#include "JAScreens.h"
#include "Keys.h"
#include "Lighting.h"
#include "Logger.h"
#include "MapScreen.h"
#include "Map_Information.h"
#include "Meanwhile.h"
#include "MercProfile.h"
#include "Message.h"
#include "Morale.h"
#include "NPC.h"
#include "OppList.h"
#include "Overhead.h"
#include "Overhead_Map.h"
#include "Overhead_Types.h"
#include "PathAI.h"
#include "Points.h"
#include "Quests.h"
#include "RT_Time_Defines.h"
#include "Random.h"
#include "RenderWorld.h"
#include "Rotting_Corpses.h"
#include "ScreenIDs.h"
#include "SkillCheck.h"
#include "Smell.h"
#include "SmokeEffects.h"
#include "Soldier.h"
#include "Soldier_Ani.h"
#include "Soldier_Find.h"
#include "Soldier_Functions.h"
#include "Soldier_Macros.h"
#include "Soldier_Profile.h"
#include "Soldier_Tile.h"
#include "SoundMan.h"
#include "Sound_Control.h"
#include "Squads.h"
#include "Strategic.h"
#include "StrategicMap.h"
#include "Strategic_Merc_Handler.h"
#include "Strategic_Status.h"
#include "Structure.h"
#include "Structure_Wrap.h"
#include "Text.h"
#include "TileDef.h"
#include "Tile_Animation.h"
#include "Timer_Control.h"
#include "Utilities.h"
#include "VObject.h"
#include "Vehicles.h"
#include "WCheck.h"
#include "WeaponModels.h"
#include "Weapons.h"
#include "WorldMan.h"
#include "enums.h"
#include <array>
#include <cmath>
#include <stdexcept>
#include <string_theory/string>

#define PALETTEFILENAME			BINARYDATADIR "/ja2pal.dat"


#define TURNING_FROM_PRONE_OFF			0
#define TURNING_FROM_PRONE_ON			1
#define TURNING_FROM_PRONE_START_UP_FROM_MOVE	2
#define TURNING_FROM_PRONE_ENDING_UP_FROM_MOVE	3

#define MIN_SUBSEQUENT_SNDS_DELAY		2000

// Enumerate extended directions
enum
{
	EX_NORTH = 0,
	EX_NORTHEAST = 4,
	EX_EAST = 8,
	EX_SOUTHEAST = 12,
	EX_SOUTH = 16,
	EX_SOUTHWEST = 20,
	EX_WEST = 24,
	EX_NORTHWEST = 28,
	EX_NUM_WORLD_DIRECTIONS = 32,
	EX_DIRECTION_IRRELEVANT
};

static void SetSoldierPersonalLightLevel(SOLDIERTYPE*);

static UINT8 Dir2ExtDir(const UINT8 dir)
{
	return dir * 4;
}


static UINT8 ExtOneCDirection(const UINT8 exdir)
{
	return (exdir + 4) % EX_NUM_WORLD_DIRECTIONS;
}


struct BATTLESNDS_STRUCT
{
	CHAR8   zName[20];
	UINT8   ubRandomVal;
	BOOLEAN fBadGuy;
	BOOLEAN fDontAllowTwoInRow;
	INT8    stopDialogue;
};


static const BATTLESNDS_STRUCT gBattleSndsData[] =
{
	{ "ok1",    2, 1, 1, 2 },
	{ "ok2",    0, 1, 1, 2 },
	{ "cool",   0, 0, 1, 0 },
	{ "curse",  0, 1, 1, 0 },
	{ "hit1",   2, 1, 1, 1 },
	{ "hit2",   0, 1, 1, 1 },
	{ "laugh",  0, 1, 1, 0 },
	{ "attn",   0, 0, 1, 0 },
	{ "die",    0, 1, 1, 1 },
	{ "humm",   0, 0, 1, 1 },
	{ "noth",   0, 0, 1, 1 },
	{ "gotit",  0, 0, 1, 1 },
	{ "lmok1",  2, 0, 1, 2 },
	{ "lmok2",  0, 0, 1, 2 },
	{ "lmattn", 0, 0, 1, 0 },
	{ "locked", 0, 0, 1, 0 },
	{ "enem",   0, 1, 1, 0 }
};


UINT8	bHealthStrRanges[] =
{
	15,
	30,
	45,
	60,
	75,
	90,
	101
};



struct PaletteSubRangeType
{
	UINT8 ubStart;
	UINT8 ubEnd;
};


// Palette ranges
static UINT32 guiNumPaletteSubRanges;
static PaletteSubRangeType* gpPaletteSubRanges;
// Palette replacements
static UINT32 guiNumReplacements;

BOOLEAN	gfGetNewPathThroughPeople = FALSE;


UINT8*                  gubpNumReplacementsPerRange;
PaletteReplacementType* gpPalRep;


void AdjustNoAPToFinishMove( SOLDIERTYPE *pSoldier, BOOLEAN fSet )
{
	if ( pSoldier->ubBodyType == CROW )
	{
		return;
	}

	// Check if we are a vehicle first....
	if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		// Turn off sound effects....
		if ( fSet )
		{
			HandleVehicleMovementSound( pSoldier, FALSE );
		}
	}

	pSoldier->fNoAPToFinishMove = fSet;

	if ( !fSet )
	{
		// return reason to default value
		pSoldier->ubReasonCantFinishMove = REASON_STOPPED_NO_APS;
	}
}


static bool IsCrowWithShadow(SOLDIERTYPE const& s)
{
	return s.ubBodyType  == CROW &&
		s.usAnimState == CROW_FLY &&
		s.pAniTile;
}


void HandleCrowShadowVisibility(SOLDIERTYPE& s)
{
	if (!IsCrowWithShadow(s)) return;
	HideAniTile(s.pAniTile, s.bLastRenderVisibleValue == -1);
}


static void HandleCrowShadowNewGridNo(SOLDIERTYPE& s)
{
	if (s.ubBodyType != CROW) return;

	if (s.pAniTile)
	{
		DeleteAniTile(s.pAniTile);
		s.pAniTile = 0;
	}

	if (s.sGridNo     == NOWHERE)  return;
	if (s.usAnimState != CROW_FLY) return;

	ANITILE_PARAMS a;
	a = ANITILE_PARAMS{};
	a.sGridNo        = s.sGridNo;
	a.ubLevelID      = ANI_SHADOW_LEVEL;
	a.sDelay         = s.sAniDelay;
	a.sStartFrame    = 0;
	a.uiFlags        = ANITILE_FORWARD | ANITILE_LOOPING | ANITILE_USE_DIRECTION_FOR_START_FRAME;
	a.sX             = s.sX;
	a.sY             = s.sY;
	a.sZ             = 0;
	a.zCachedFile    = TILECACHEDIR "/fly_shdw.sti";
	a.v.user.uiData3 = s.bDirection;
	s.pAniTile = CreateAnimationTile(&a);

	HandleCrowShadowVisibility(s);
}


static void HandleCrowShadowRemoveGridNo(SOLDIERTYPE& s)
{
	if (!IsCrowWithShadow(s)) return;
	DeleteAniTile(s.pAniTile);
	s.pAniTile = 0;
}


static void HandleCrowShadowNewDirection(SOLDIERTYPE* const s)
{
	if (!IsCrowWithShadow(*s)) return;
	s->pAniTile->v.user.uiData3 = s->bDirection;
}


static void HandleCrowShadowNewPosition(SOLDIERTYPE* const s)
{
	if (!IsCrowWithShadow(*s)) return;
	s->pAniTile->sRelativeX = s->sX;
	s->pAniTile->sRelativeY = s->sY;
}


static const UINT8 gubMaxActionPoints[] =
{
	AP_MAXIMUM,         // REGMALE
	AP_MAXIMUM,         // BIGMALE
	AP_MAXIMUM,         // STOCKYMALE
	AP_MAXIMUM,         // REGFEMALE
	AP_MONSTER_MAXIMUM, // ADULTMONSTER
	AP_MONSTER_MAXIMUM, // ADULTMONSTER
	AP_MONSTER_MAXIMUM, // ADULTMONSTER
	AP_MONSTER_MAXIMUM, // ADULTMONSTER
	AP_MONSTER_MAXIMUM, // ADULTMONSTER
	AP_MONSTER_MAXIMUM, // INFANT
	AP_MONSTER_MAXIMUM, // QUEEN MONSTER
	AP_MAXIMUM,         // FATCIV
	AP_MAXIMUM,         // MANCIV
	AP_MAXIMUM,         // MINICIV
	AP_MAXIMUM,         // DRESSCIV
	AP_MAXIMUM,         // HAT KID
	AP_MAXIMUM,         // NOHAT KID
	AP_MAXIMUM,         // CRIPPLE
	AP_MAXIMUM,         // COW
	AP_MAXIMUM,         // CROW
	AP_MAXIMUM,         // BLOOD CAT
	AP_MAXIMUM,         // ROBOT1
	AP_VEHICLE_MAXIMUM, // HUMVEE
	AP_VEHICLE_MAXIMUM, // TANK1
	AP_VEHICLE_MAXIMUM, // TANK2
	AP_VEHICLE_MAXIMUM, // ELDORADO
	AP_VEHICLE_MAXIMUM, // ICECREAMTRUCK
	AP_VEHICLE_MAXIMUM  // JEEP
};


INT8 CalcActionPoints(const SOLDIERTYPE* const pSold)
{
	UINT8 ubPoints,ubMaxAPs;
	INT8  bBandage;

	// dead guys don't get any APs (they shouldn't be here asking for them!)
	if (!pSold->bLife)
		return(0);

	// people with sleep dart drug who have collapsed get no APs
	if ( (pSold->bSleepDrugCounter > 0) && pSold->bCollapsed )
		return( 0 );

	// Calculate merc's action points at 100% capability (range is 10 - 25)
	// round fractions of .5 up (that's why the +20 before the division!
	ubPoints = 5 + (((10 * EffectiveExpLevel( pSold ) +
			3 * EffectiveAgility( pSold )   +
			2 * pSold->bLifeMax   +
			2 * EffectiveDexterity( pSold ) ) + 20) / 40);

	// Calculate bandage
	bBandage = pSold->bLifeMax - pSold->bLife - pSold->bBleeding;

	// If injured, reduce action points accordingly (by up to 2/3rds)
	if (pSold->bLife < pSold->bLifeMax)
	{
		ubPoints -= (2 * ubPoints * (pSold->bLifeMax - pSold->bLife + (bBandage / 2))) /
				(3 * pSold->bLifeMax);
	}

	// If tired, reduce action points accordingly (by up to 1/2)
	if (pSold->bBreath < 100)
		ubPoints -= (ubPoints * (100 - pSold->bBreath)) / 200;

	if (pSold->sWeightCarriedAtTurnStart > 100)
	{
		ubPoints = (UINT8) ( ((UINT32)ubPoints) * 100 / pSold->sWeightCarriedAtTurnStart );
	}

	// If resulting APs are below our permitted minimum, raise them to it!
	if (ubPoints < AP_MINIMUM)
		ubPoints = AP_MINIMUM;

	// make sure action points doesn't exceed the permitted maximum
	ubMaxAPs = gubMaxActionPoints[ pSold->ubBodyType ];

	// If resulting APs are below our permitted minimum, raise them to it!
	if (ubPoints > ubMaxAPs)
		ubPoints = ubMaxAPs;

	if ( pSold->ubBodyType == BLOODCAT )
	{
		// use same as young monsters
		ubPoints = (ubPoints * AP_YOUNG_MONST_FACTOR) / 10;
	}
	else if (pSold->uiStatusFlags & SOLDIER_MONSTER)
	{
		// young monsters get extra APs
		if ( pSold->ubBodyType == YAF_MONSTER || pSold->ubBodyType == YAM_MONSTER ||
			pSold->ubBodyType == INFANT_MONSTER )
		{
			ubPoints = (ubPoints * AP_YOUNG_MONST_FACTOR) / 10;
		}

		// if frenzied, female monsters get more APs! (for young females, cumulative!)
		if (pSold->bFrenzied)
		{

			ubPoints = (ubPoints * AP_MONST_FRENZY_FACTOR) / 10;
		}
	}

	// adjust APs for phobia situations
	if ( pSold->ubProfile != NO_PROFILE )
	{
		if ((gMercProfiles[ pSold->ubProfile ].bPersonalityTrait == CLAUSTROPHOBIC) &&
			gWorldSector.z > 0)
		{
			ubPoints = (ubPoints * AP_CLAUSTROPHOBE) / 10;
		}
		else if (gMercProfiles[pSold->ubProfile].bPersonalityTrait == FEAR_OF_INSECTS &&
			MercSeesCreature(*pSold))
		{
			ubPoints = (ubPoints * AP_AFRAID_OF_INSECTS) / 10;
		}
	}

	// Adjusat APs due to drugs...
	HandleAPEffectDueToDrugs( pSold, &ubPoints );

	// If we are a vehicle, adjust APS...
	if (pSold->uiStatusFlags & SOLDIER_VEHICLE) ubPoints += 35;

	// if we are in boxing mode, adjust APs... THIS MUST BE LAST!
	if ( gTacticalStatus.bBoxingState == BOXING || gTacticalStatus.bBoxingState == PRE_BOXING )
	{
		ubPoints /= 2;
	}

	return (ubPoints);
}

void CalcNewActionPoints( SOLDIERTYPE *pSoldier )
{
	if ( gTacticalStatus.bBoxingState == BOXING || gTacticalStatus.bBoxingState == PRE_BOXING )
	{
		// if we are in boxing mode, carry 1/2 as many points
		if (pSoldier->bActionPoints > MAX_AP_CARRIED / 2)
		{
			pSoldier->bActionPoints = MAX_AP_CARRIED / 2;
		}
	}
	else
	{
		if (pSoldier->bActionPoints > MAX_AP_CARRIED)
		{
			pSoldier->bActionPoints = MAX_AP_CARRIED;
		}
	}

	pSoldier->bActionPoints += CalcActionPoints( pSoldier);

	// Don't max out if we are drugged....
	if ( !GetDrugEffect( pSoldier, DRUG_TYPE_ADRENALINE ) )
	{
		pSoldier->bActionPoints = std::min(int(pSoldier->bActionPoints), int(gubMaxActionPoints[pSoldier->ubBodyType]));
	}

	pSoldier->bInitialActionPoints	= pSoldier->bActionPoints;
}


void	DoNinjaAttack( SOLDIERTYPE *pSoldier )
{
	//UINT32 uiMercFlags;
	UINT8  ubTDirection;
	UINT8  ubTargetStance;

	const SOLDIERTYPE* const pTSoldier = WhoIsThere2(pSoldier->sTargetGridNo, pSoldier->bLevel);
	if (pTSoldier != NULL)
	{
		// Look at stance of target
		ubTargetStance = gAnimControl[ pTSoldier->usAnimState ].ubEndHeight;

		// Get his life...if < certain value, do finish!
		if ( (pTSoldier->bLife <= 30 || pTSoldier->bBreath <= 30) && ubTargetStance != ANIM_PRONE )
		{
			// Do finish!
			ChangeSoldierState( pSoldier, NINJA_SPINKICK, 0 , FALSE );
		}
		else
		{
			if ( ubTargetStance != ANIM_PRONE )
			{
				const UINT16 state = (Random(2) == 0 ? NINJA_LOWKICK : NINJA_PUNCH);
				ChangeSoldierState(pSoldier, state, 0, FALSE);

				// CHECK IF HE CAN SEE US, IF SO CHANGE DIRECTION
				if ( pTSoldier->bOppList[ pSoldier->ubID ] == 0 && pTSoldier->bTeam != pSoldier->bTeam )
				{
					if ( !( pTSoldier->uiStatusFlags & ( SOLDIER_MONSTER | SOLDIER_ANIMAL | SOLDIER_VEHICLE ) ) )
					{
						ubTDirection = (UINT8)GetDirectionFromGridNo( pSoldier->sGridNo, pTSoldier );
						SendSoldierSetDesiredDirectionEvent( pTSoldier, ubTDirection );
					}
				}
			}
			else
			{
				// CHECK OUR STANCE
				if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_CROUCH )
				{
					// SET DESIRED STANCE AND SET PENDING ANIMATION
					ChangeSoldierStance(pSoldier, ANIM_CROUCH);
					pSoldier->usPendingAnimation = PUNCH_LOW;
				}
				else
				{
					// USE crouched one
					// NEED TO CHANGE STANCE IF NOT CROUCHD!
					EVENT_InitNewSoldierAnim( pSoldier, PUNCH_LOW, 0 , FALSE );
				}
			}
		}
	}

	if (pSoldier->ubProfile == DR_Q)
	{
		// Play sound!

		UINT32 volume = CalculateSpeechVolume(HIGHVOLUME);

		// If we are an enemy.....reduce due to volume
		if ( pSoldier->bTeam != OUR_TEAM )
		{
			volume = SoundVolume(volume, pSoldier->sGridNo);
		}

		const UINT32 pan = SoundDir(pSoldier->sGridNo);

		const char* filename;
		if ( pSoldier->usAnimState == NINJA_SPINKICK )
		{
			filename = BATTLESNDSDIR "/033_chop2.wav";
		}
		else
		{
			if ( Random( 2 ) == 0 )
			{
				filename = BATTLESNDSDIR "/033_chop3.wav";
			}
			else
			{
				filename = BATTLESNDSDIR "/033_chop1.wav";
			}
		}
		const UINT32 uiSoundID = SoundPlay(filename, volume, pan, 1, NULL, NULL);

		if ( uiSoundID != SOUND_ERROR )
		{
			pSoldier->uiBattleSoundID = uiSoundID;

			if ( pSoldier->ubProfile != NO_PROFILE )
			{
				FACETYPE* const face = pSoldier->face;
				if (face != NULL) ExternSetFaceTalking(*face, uiSoundID);
			}
		}
	}
}


void CreateSoldierCommon(SOLDIERTYPE& s)
try
{
	//if we are loading a saved game, we DO NOT want to reset the opplist,
	//look for enemies, or say a dying commnet
	if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME))
	{
		// Set initial values for opplist!
		InitSoldierOppList(s);
		HandleSight(s, SIGHT_LOOK);

		// Set some quote flags
		s.fDyingComment = s.bLife < OKLIFE;
	}

	// ATE: Reset some timer flags...
	s.uiTimeSameBattleSndDone   = 0;
	// ATE: Reset every time.....
	s.fSoldierWasMoving         = TRUE;
	s.uiTuringSoundID           = NO_SAMPLE;
	s.uiTimeSinceLastBleedGrunt = 0;

	if (s.ubBodyType == QUEENMONSTER)
	{
		s.iPositionSndID = NewPositionSnd(NOWHERE, &s, QUEEN_AMBIENT_NOISE);
	}

	// ANYTHING AFTER HERE CAN FAIL
	if (IsOnOurTeam(s))
	{
		s.pKeyRing = new KEY_ON_RING[NUM_KEYS]{};
	}
	else
	{
		s.pKeyRing = NULL;
	}

	// Create frame cache
	s.AnimCache.init(s.ubID);

	UINT16 ani_state = s.usAnimState;
	if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME))
	{
		// Init new soldier state
		// OFFSET FIRST ANIMATION FRAME FOR NEW MERCS
		EVENT_InitNewSoldierAnim(&s, ani_state, ani_state == STANDING ? Random(10) : 0, TRUE);
	}
	else
	{
		/// if we don't have a world loaded, and are in a bad anim, goto standing.
		UINT16 ani_code = s.usAniCode;
		if (!gfWorldLoaded)
		{
			switch (ani_state)
			{
				case HOPFENCE:
				case CLIMBDOWNROOF:
				case FALLFORWARD_ROOF:
				case FALLOFF:
				case CLIMBUPROOF:
					ani_state = STANDING;
					ani_code  = 0;
					break;
			}
		}
		EVENT_InitNewSoldierAnim(&s, ani_state, ani_code, TRUE);
	}

	CreateSoldierPalettes(s);
}
catch (...)
{
	DeleteSoldier(s);
	throw;
}


void DeleteSoldier(SOLDIERTYPE& s)
{
	if (s.sGridNo != NOWHERE)
	{
		// Remove adjacency records
		for (INT8 bDir = 0; bDir < NUM_WORLD_DIRECTIONS; ++bDir)
		{
			INT32 const iGridNo = s.sGridNo + DirIncrementer[bDir];
			if (0 <= iGridNo && iGridNo < WORLD_MAX)
			{
				--gpWorldLevelData[iGridNo].ubAdjacentSoldierCnt;
			}
		}
	}

	if (s.pKeyRing)
	{
		delete[] s.pKeyRing;
		s.pKeyRing = 0;
	}

	DeleteSoldierFace(&s);

	FOR_EACH(UINT16*, i, s.pShades)
	{
		if (*i == NULL) continue;
		delete[] *i;
		*i = NULL;
	}

	if (s.effect_shade)
	{
		delete[] s.effect_shade;
		s.effect_shade = 0;
	}

	FOR_EACH(UINT16*, i, s.pGlowShades)
	{
		if (*i == NULL) continue;
		delete[] *i;
		*i = NULL;
	}

	if (s.ubBodyType == QUEENMONSTER)
	{
		DeletePositionSnd(s.iPositionSndID);
	}

	// Free any animations we may have locked...
	s.AnimCache.free();

	DeleteSoldierLight(&s);
	UnMarkMovementReserved(s);
	if (!RemoveMercSlot(&s)) RemoveAwaySlot(&s);

	s.bActive = FALSE;
}


static BOOLEAN CreateSoldierLight(SOLDIERTYPE* pSoldier)
{
	if ( pSoldier->bTeam != OUR_TEAM )
	{
		return( FALSE );
	}

	// DO ONLY IF WE'RE AT A GOOD LEVEL
	if (pSoldier->light == NULL)
	{
		// ATE: Check for goggles in headpos....
		ST::string light_file = IsWearingHeadGear(*pSoldier, UVGOGGLES) ? "Light4" :
						IsWearingHeadGear(*pSoldier, NIGHTGOGGLES) ? "Light3" :
						"Light2";

		LIGHT_SPRITE* const l = LightSpriteCreate(light_file);
		pSoldier->light = l;
		if (l == NULL)
		{
			SLOGD("Soldier: Failed loading light");
			return FALSE;
		}

		l->uiFlags |= MERC_LIGHT;

		if (pSoldier->bLevel != 0) LightSpriteRoofStatus(l, TRUE);
	}

	return( TRUE );
}


void ReCreateSoldierLight(SOLDIERTYPE* const s)
{
	if (s->bTeam != OUR_TEAM) return;
	if (!s->bActive)             return;
	if (!s->bInSector)           return;

	DeleteSoldierLight(s);
	CreateSoldierLight(s);
}


void DeleteSoldierLight(SOLDIERTYPE* const s)
{
	if (s->light == NULL) return;

	LightSpriteDestroy(s->light);
	s->light = NULL;
}


// FUNCTIONS CALLED BY EVENT PUMP
/////////////////////////////////

void ChangeSoldierState(SOLDIERTYPE* pSoldier, UINT16 usNewState, UINT16 usStartingAniCode, BOOLEAN fForce)
{
	EVENT_InitNewSoldierAnim(pSoldier, usNewState, usStartingAniCode, fForce);
}


// This function reevaluates the stance if the guy sees us!
BOOLEAN ReevaluateEnemyStance( SOLDIERTYPE *pSoldier, UINT16 usAnimState )
{
	// make the chosen one not turn to face us
	if (OK_ENEMY_MERC(pSoldier) &&
		pSoldier != gTacticalStatus.the_chosen_one &&
		gAnimControl[usAnimState].ubEndHeight == ANIM_STAND &&
		!(pSoldier->uiStatusFlags & SOLDIER_UNDERAICONTROL))
	{
		if ( pSoldier->fTurningFromPronePosition == TURNING_FROM_PRONE_OFF )
		{
			// If we are a queen and see enemies, goto ready
			if ( pSoldier->ubBodyType == QUEENMONSTER )
			{
				if ( gAnimControl[ usAnimState ].uiFlags & ( ANIM_BREATH ) )
				{
					if ( pSoldier->bOppCnt > 0 )
					{
						EVENT_InitNewSoldierAnim( pSoldier, QUEEN_INTO_READY, 0 , TRUE );
						return( TRUE );
					}
				}
			}

			// ATE: Don't do this if we're not a merc.....
			if ( !IS_MERC_BODY_TYPE( pSoldier ) )
			{
				return( FALSE );
			}

			if ( gAnimControl[ usAnimState ].uiFlags & ( ANIM_MERCIDLE | ANIM_BREATH ) )
			{
				GridNo closest = NOWHERE;
				if (pSoldier->bOppCnt > 0)
				{
					// Pick a guy this buddy sees and turn towards them!
					INT16 sClosestDist = 10000;
					CFOR_EACH_IN_TEAM(opp, OUR_TEAM)
					{
						if (pSoldier->bOppList[opp->ubID] == SEEN_CURRENTLY)
						{
							const GridNo gridno = opp->sGridNo;
							const INT16 sDist   = PythSpacesAway(pSoldier->sGridNo, gridno);
							if (sDist < sClosestDist)
							{
								sClosestDist = sDist;
								closest      = gridno;
							}
						}
					}

					if (closest != NOWHERE)
					{
						// Change to fire ready animation
						pSoldier->fDontChargeReadyAPs = TRUE;
						return SoldierReadyWeapon(pSoldier, closest, FALSE);
					}
				}
			}
		}
	}
	return( FALSE );

}


static void CheckForFreeupFromHit(SOLDIERTYPE* pSoldier, UINT32 uiOldAnimFlags, UINT32 uiNewAnimFlags, UINT16 usOldAniState, UINT16 usNewState)
{
	// THIS COULD POTENTIALLY CALL EVENT_INITNEWAnim() if the GUY was SUPPRESSED
	// CHECK IF THE OLD ANIMATION WAS A HIT START THAT WAS NOT FOLLOWED BY A HIT FINISH
	// IF SO, RELEASE ATTACKER FROM ATTACKING

	// If old and new animations are the same, do nothing!
	if ( usOldAniState == QUEEN_HIT && usNewState == QUEEN_HIT )
	{
		return;
	}

	if ( usOldAniState != usNewState && ( uiOldAnimFlags & ANIM_HITSTART ) && !( uiNewAnimFlags & ANIM_HITFINISH ) && !( uiNewAnimFlags & ANIM_IGNOREHITFINISH ) && !(pSoldier->uiStatusFlags & SOLDIER_TURNINGFROMHIT ) )
	{
		// Release attacker
		SLOGD("Releasesoldierattacker, normal hit animation ended\n\
			NEW: {} ( {} ) OLD: {} ( {} )",
			gAnimControl[usNewState].zAnimStr, usNewState,
			gAnimControl[usOldAniState].zAnimStr, pSoldier->usOldAniState);
		ReleaseSoldiersAttacker( pSoldier );

		//FREEUP GETTING HIT FLAG
		pSoldier->fGettingHit = FALSE;

		// ATE: if our guy, have 10% change of say damn, if still conscious...
		if ( pSoldier->bTeam == OUR_TEAM && pSoldier->bLife >= OKLIFE )
		{
			if ( Random( 10 ) == 0 )
			{
				DoMercBattleSound(pSoldier, BATTLE_SOUND_CURSE1);
			}
		}
	}

	// CHECK IF WE HAVE FINSIHED A HIT WHILE DOWN
	// OBLY DO THIS IF 1 ) We are dead already or 2 ) We are alive still
	if ((uiOldAnimFlags & ANIM_HITWHENDOWN) && ((pSoldier->uiStatusFlags & SOLDIER_DEAD)
		|| pSoldier->bLife != 0))
	{
		// Release attacker
		SLOGD("Releasesoldierattacker, animation of kill on the ground ended");
		ReleaseSoldiersAttacker( pSoldier );

		//FREEUP GETTING HIT FLAG
		pSoldier->fGettingHit = FALSE;

		if ( pSoldier->bLife == 0 )
		{
			//ATE: Set previous attacker's value!
			// This is so that the killer can say their killed quote....
			pSoldier->attacker = pSoldier->previous_attacker;
		}
	}
}


static bool IsRifle(UINT16 const item_id)
{
	return item_id != NOTHING &&
		item_id != ROCKET_LAUNCHER &&
		GCM->getItem(item_id)->getItemClass() == IC_GUN &&
		GCM->getItem(item_id)->isTwoHanded();
}


static void HandleAnimationProfile(SOLDIERTYPE&, UINT16 usAnimState, BOOLEAN fRemove);
static void SetSoldierLocatorOffsets(SOLDIERTYPE* pSoldier);


void EVENT_InitNewSoldierAnim(SOLDIERTYPE* const pSoldier, UINT16 usNewState, UINT16 const usStartingAniCode, BOOLEAN const fForce)
{
	INT16   sAPCost = 0;
	INT16   sBPCost = 0;
	UINT32  uiOldAnimFlags;
	UINT32  uiNewAnimFlags;
	BOOLEAN fTryingToRestart = FALSE;

	CHECKV(usNewState < NUMANIMATIONSTATES);

	///////////////////////////////////////////////////////////////////////
	//			DO SOME CHECKS ON OUR NEW ANIMATION!
	/////////////////////////////////////////////////////////////////////

	// If we are NOT loading a game, continue normally
	if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		// CHECK IF WE ARE TRYING TO INTURRUPT A SCRIPT WHICH WE DO NOT WANT INTERRUPTED!
		if ( pSoldier->fInNonintAnim )
		{
			return;
		}

		if ( pSoldier->fRTInNonintAnim )
		{
			if ( !(gTacticalStatus.uiFlags & INCOMBAT) )
			{
				return;
			}
			else
			{
				pSoldier->fRTInNonintAnim = FALSE;
			}
		}


		// Check if we can restart this animation if it's the same as our current!
		if ( usNewState == pSoldier->usAnimState )
		{
			if ( ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_NORESTART ) && !fForce )
			{
				fTryingToRestart = TRUE;
			}
		}

		// Check state, if we are not at the same height, set this ani as the pending one and
		// change stance accordingly
		// ATE: ONLY IF WE ARE STARTING AT START OF ANIMATION!
		if ( usStartingAniCode == 0 )
		{
			if (gAnimControl[usNewState].ubHeight != gAnimControl[pSoldier->usAnimState].ubEndHeight &&
				!(gAnimControl[usNewState].uiFlags & (ANIM_STANCECHANGEANIM | ANIM_IGNORE_AUTOSTANCE)))
			{

				// Check if we are going from crouched height to prone height, and adjust fast turning
				// accordingly. Make guy turn while crouched THEN go into prone
				if ((gAnimControl[usNewState].ubEndHeight == ANIM_PRONE &&
					gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_CROUCH ) &&
					!(gTacticalStatus.uiFlags & INCOMBAT))
				{
					pSoldier->fTurningUntilDone = TRUE;
					pSoldier->ubPendingStanceChange = gAnimControl[ usNewState ].ubEndHeight;
					pSoldier->usPendingAnimation = usNewState;
					return;
				}
				// Check if we are in realtime and we are going from stand to crouch
				else if (gAnimControl[usNewState].ubEndHeight == ANIM_CROUCH &&
					gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_STAND &&
					(gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_MOVING) &&
					!(gTacticalStatus.uiFlags & INCOMBAT))
				{
					pSoldier->ubDesiredHeight = gAnimControl[ usNewState ].ubEndHeight;
					// Continue with this course of action IE: Do animation
					// and skip from stand to crouch
				}
				// Check if we are in realtime and we are going from crouch to stand
				else if (gAnimControl[usNewState].ubEndHeight == ANIM_STAND &&
					gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_CROUCH &&
					(gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_MOVING) &&
					!(gTacticalStatus.uiFlags & INCOMBAT) &&
					pSoldier->usAnimState != HELIDROP)
				{
					pSoldier->ubDesiredHeight = gAnimControl[ usNewState ].ubEndHeight;
					// Continue with this course of action IE: Do animation and
					// skip from stand to crouch
				}
				else
				{
					// Set our next moving animation to be pending, after
					pSoldier->usPendingAnimation = usNewState;
					// Set new state to be animation to move to new stance
					ChangeSoldierStance(pSoldier, gAnimControl[usNewState].ubHeight);
					return;
				}
			}
		}

		if ( usNewState == ADJACENT_GET_ITEM )
		{
			if ( pSoldier->ubPendingDirection != NO_PENDING_DIRECTION )
			{
				EVENT_SetSoldierDesiredDirectionForward(pSoldier, pSoldier->ubPendingDirection);
				pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
				pSoldier->usPendingAnimation = ADJACENT_GET_ITEM;
				pSoldier->fTurningUntilDone = TRUE;
				SoldierGotoStationaryStance( pSoldier );
				return;
			}
		}


		if ( usNewState == CLIMBUPROOF )
		{
			if ( pSoldier->ubPendingDirection != NO_PENDING_DIRECTION )
			{
				EVENT_SetSoldierDesiredDirectionForward(pSoldier, pSoldier->ubPendingDirection);
				pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
				pSoldier->usPendingAnimation = CLIMBUPROOF;
				pSoldier->fTurningUntilDone = TRUE;
				SoldierGotoStationaryStance( pSoldier );
				return;
			}
		}

		if ( usNewState == CLIMBDOWNROOF )
		{
			if ( pSoldier->ubPendingDirection != NO_PENDING_DIRECTION )
			{
				EVENT_SetSoldierDesiredDirectionForward(pSoldier, pSoldier->ubPendingDirection);
				pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
				pSoldier->usPendingAnimation = CLIMBDOWNROOF;
				pSoldier->fTurningFromPronePosition = FALSE;
				pSoldier->fTurningUntilDone = TRUE;
				SoldierGotoStationaryStance( pSoldier );
				return;
			}
		}

		// ATE: Don't raise/lower automatically if we are low on health,
		// as our gun looks lowered anyway....
		//if ( pSoldier->bLife > INJURED_CHANGE_THREASHOLD )
		{
			// Don't do some of this if we are a monster!
			// ATE: LOWER AIMATION IS GOOD, RAISE ONE HOWEVER MAY CAUSE PROBLEMS FOR AI....
			if (!(pSoldier->uiStatusFlags & SOLDIER_MONSTER) &&
				pSoldier->ubBodyType != ROBOTNOWEAPON && pSoldier->bTeam == OUR_TEAM)
			{
				// If this animation is a raise_weapon animation
				if ((gAnimControl[usNewState].uiFlags & ANIM_RAISE_WEAPON) &&
					!(gAnimControl[pSoldier->usAnimState].uiFlags & (ANIM_RAISE_WEAPON | ANIM_NOCHANGE_WEAPON)))
				{
					// We are told that we need to raise weapon
					// Do so only if
					// 1) We have a rifle in hand...
					if (IsRifle(pSoldier->inv[HANDPOS].usItem))
					{
						// Switch on height!
						switch( gAnimControl[ pSoldier->usAnimState ].ubEndHeight )
						{
							case ANIM_STAND:

								// 2) OK, all's fine... lower weapon first....
								pSoldier->usPendingAnimation = usNewState;
								// Set new state to be animation to move to new stance
								usNewState = RAISE_RIFLE;
						}
					}
				}

				// If this animation is a lower_weapon animation
				if ((gAnimControl[usNewState].uiFlags & ANIM_LOWER_WEAPON) &&
					!( gAnimControl[pSoldier->usAnimState].uiFlags & (ANIM_LOWER_WEAPON | ANIM_NOCHANGE_WEAPON)))
				{
					// We are told that we need to lower weapon
					// Do so only if
					// 1) We have a rifle in hand...
					if (IsRifle(pSoldier->inv[HANDPOS].usItem))
					{
						// Switch on height!
						switch( gAnimControl[ pSoldier->usAnimState ].ubEndHeight )
						{
							case ANIM_STAND:

								// 2) OK, all's fine... lower weapon first....
								pSoldier->usPendingAnimation = usNewState;
								// Set new state to be animation to move to new stance
								usNewState = LOWER_RIFLE;
						}
					}
				}
			}
		}

		// Are we cowering and are tyring to move, getup first...
		if (gAnimControl[usNewState].uiFlags & ANIM_MOVING &&
			pSoldier->usAnimState == COWERING &&
			gAnimControl[usNewState].ubEndHeight == ANIM_STAND)
		{
			pSoldier->usPendingAnimation = usNewState;
			// Set new state to be animation to move to new stance
			usNewState = END_COWER;
		}

		// If we want to start swatting, put a pending animation
		if( pSoldier->usAnimState != START_SWAT && usNewState == SWATTING )
		{
			// Set new state to be animation to move to new stance
			usNewState = START_SWAT;
		}

		if( pSoldier->usAnimState == SWATTING && usNewState == CROUCHING )
		{
			// Set new state to be animation to move to new stance
			usNewState = END_SWAT;
		}

		if(pSoldier->usAnimState == WALKING && usNewState == STANDING &&
			pSoldier->bLife < INJURED_CHANGE_THREASHOLD &&
			pSoldier->ubBodyType <= REGFEMALE &&
			!MercInWater(pSoldier))
		{
			// Set new state to be animation to move to new stance
			usNewState = END_HURT_WALKING;
		}

		// Check if we are an enemy, and we are in an animation what should be overriden
		// by if he sees us or not.
		if ( ReevaluateEnemyStance( pSoldier, usNewState ) )
		{
			return;
		}

		// Alrighty, check if we should free buddy up!
		if ( usNewState == GIVING_AID )
		{
			UnSetUIBusy(pSoldier);
		}

		// SUBSTITUDE VARIOUS REG ANIMATIONS WITH ODD BODY TYPES
		usNewState = SubstituteBodyTypeAnimation(pSoldier, usNewState);

		// CHECK IF WE CAN DO THIS ANIMATION!
		if (!IsAnimationValidForBodyType(*pSoldier, usNewState)) return;

		// OK, make guy transition if a big merc...
		if ( pSoldier->uiAnimSubFlags & SUB_ANIM_BIGGUYTHREATENSTANCE )
		{
			if (usNewState == KNEEL_DOWN &&
				pSoldier->usAnimState != BIGMERC_CROUCH_TRANS_INTO &&
				IsRifle(pSoldier->inv[HANDPOS].usItem))
			{
				usNewState = BIGMERC_CROUCH_TRANS_INTO;
			}

			if (usNewState == KNEEL_UP &&
				pSoldier->usAnimState != BIGMERC_CROUCH_TRANS_OUTOF &&
				IsRifle(pSoldier->inv[HANDPOS].usItem))
			{
				usNewState = BIGMERC_CROUCH_TRANS_OUTOF;
			}
		}

		// OK, if we have reverse set, do the side step!
		if ( pSoldier->bReverse )
		{
			if ( usNewState == WALKING || usNewState == RUNNING || usNewState == SWATTING )
			{
				// CHECK FOR SIDEWAYS!
				if ( pSoldier->bDirection == gPurpendicularDirection[ pSoldier->bDirection ][ pSoldier->ubPathingData[ pSoldier->ubPathIndex ] ] )
				{
					// We are perpendicular!
					usNewState = SIDE_STEP;
				}
				else
				{
					if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_CROUCH )
					{
						usNewState = SWAT_BACKWARDS;
					}
					else
					{
						// Here, change to  opposite direction
						usNewState = WALK_BACKWARDS;
					}
				}
			}
		}

		// ATE: Patch hole for breath collapse for roofs, fences
		if ( usNewState == CLIMBUPROOF || usNewState == CLIMBDOWNROOF || usNewState == HOPFENCE )
		{
			// Check for breath collapse if a given animation like
			if (CheckForBreathCollapse(*pSoldier) || pSoldier->bCollapsed)
			{
				// UNset UI
				UnSetUIBusy(pSoldier);

				SoldierCollapse( pSoldier );

				pSoldier->bBreathCollapsed = FALSE;

				return;

			}
		}

		// If we are in water.....and trying to run, change to run
		if ( pSoldier->bOverTerrainType == LOW_WATER || pSoldier->bOverTerrainType == MED_WATER )
		{
			// Check animation
			// Change to walking
			if ( usNewState == RUNNING )
			{
				usNewState = WALKING;
			}
		}

		// Turn off anipause flag for any anim!
		pSoldier->uiStatusFlags &= (~SOLDIER_PAUSEANIMOVE);

		// Unset paused for no APs.....
		AdjustNoAPToFinishMove( pSoldier, FALSE );

		if ( usNewState == CRAWLING && pSoldier->usDontUpdateNewGridNoOnMoveAnimChange == 1 )
		{
			if ( pSoldier->fTurningFromPronePosition != TURNING_FROM_PRONE_ENDING_UP_FROM_MOVE )
			{
				pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_START_UP_FROM_MOVE;
			}

			// ATE: IF we are starting to crawl, but have to getup to turn first......
			if ( pSoldier->fTurningFromPronePosition == TURNING_FROM_PRONE_START_UP_FROM_MOVE )
			{
				usNewState = PRONE_UP;
				pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_ENDING_UP_FROM_MOVE;
			}
		}

		// We are about to start moving
		// Handle buddy beginning to move...
		// check new gridno, etc
		// ATE: Added: Make check that old anim is not a moving one as well
		if ((gAnimControl[usNewState].uiFlags & ANIM_MOVING &&
			!(gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_MOVING)) ||
			(gAnimControl[usNewState].uiFlags & ANIM_MOVING && fForce))
		{
			BOOLEAN fKeepMoving;

			if ( usNewState == CRAWLING && pSoldier->usDontUpdateNewGridNoOnMoveAnimChange == LOCKED_NO_NEWGRIDNO )
			{
				// Turn off lock once we are crawling once...
				pSoldier->usDontUpdateNewGridNoOnMoveAnimChange = 1;
			}

			// ATE: Additional check here if we have just been told to update animation ONLY, not goto gridno stuff...
			if ( !pSoldier->usDontUpdateNewGridNoOnMoveAnimChange  )
			{
				if ( usNewState != SWATTING  )
				{
					SLOGD("Handling New gridNo for {}: Old {}, New {}",
						pSoldier->ubID, gAnimControl[pSoldier->usAnimState].zAnimStr,
						gAnimControl[usNewState].zAnimStr);

					if ( !( gAnimControl[ usNewState ].uiFlags & ANIM_SPECIALMOVE ) )
					{
						// Handle goto new tile...
						if ( HandleGotoNewGridNo( pSoldier, &fKeepMoving, TRUE, usNewState ) )
						{
							if ( !fKeepMoving )
							{
								return;
							}

							// Make sure desy = zeroed out...
							// pSoldier->fPastXDest = pSoldier->fPastYDest = FALSE;
						}
						else
						{
							if ( pSoldier->bBreathCollapsed )
							{
								// UNset UI
								UnSetUIBusy(pSoldier);

								SoldierCollapse( pSoldier );

								pSoldier->bBreathCollapsed = FALSE;
							}
							return;
						}
					}
					else
					{
						// Change desired direction
						// Just change direction
						EVENT_InternalSetSoldierDestination( pSoldier, pSoldier->ubPathingData[ pSoldier->ubPathIndex ], FALSE, pSoldier->usAnimState );
					}

					//check for services
					ReceivingSoldierCancelServices( pSoldier );
					GivingSoldierCancelServices( pSoldier );


					// Check if we are a vehicle, and start playing noise sound....
					if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
					{
						HandleVehicleMovementSound( pSoldier, TRUE );
					}
				}
			}
		}
		else
		{
			// Check for stopping movement noise...
			if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
			{
				HandleVehicleMovementSound( pSoldier, FALSE );

				// If a vehicle, set hewight to 0
				SetSoldierHeight( pSoldier, (FLOAT)( 0 ) );
			}

		}

		// Reset to false always.....
		// ( Unless locked )
		if ( gAnimControl[ usNewState ].uiFlags & ANIM_MOVING )
		{
			if ( pSoldier->usDontUpdateNewGridNoOnMoveAnimChange != LOCKED_NO_NEWGRIDNO )
			{
				pSoldier->usDontUpdateNewGridNoOnMoveAnimChange = FALSE;
			}
		}

		if ( fTryingToRestart )
		{
			return;
		}

	}


	// ATE: If this is an AI guy.. unlock him!
	if ( gTacticalStatus.fEnemySightingOnTheirTurn )
	{
		if (gTacticalStatus.enemy_sighting_on_their_turn_enemy == pSoldier)
		{
			pSoldier->fPauseAllAnimation = FALSE;
			gTacticalStatus.fEnemySightingOnTheirTurn = FALSE;
		}
	}

	///////////////////////////////////////////////////////////////////////
	//			HERE DOWN - WE HAVE MADE A DESCISION!
	/////////////////////////////////////////////////////////////////////

	uiOldAnimFlags = gAnimControl[ pSoldier->usAnimState ].uiFlags;
	uiNewAnimFlags = gAnimControl[ usNewState ].uiFlags;


	// CHECKING IF WE HAVE A HIT FINISH BUT NO DEATH IS DONE WITH A SPECIAL ANI CODE
	// IN THE HIT FINSIH ANI SCRIPTS

	// CHECKING IF WE HAVE FINISHED A DEATH ANIMATION IS DONE WITH A SPECIAL ANI CODE
	// IN THE DEATH SCRIPTS


	// CHECK IF THIS NEW STATE IS NON-INTERRUPTABLE
	// IF SO - SET NON-INT FLAG
	if ( uiNewAnimFlags & ANIM_NONINTERRUPT )
	{
		pSoldier->fInNonintAnim = TRUE;
	}

	if ( uiNewAnimFlags & ANIM_RT_NONINTERRUPT )
	{
		pSoldier->fRTInNonintAnim = TRUE;
	}

	// CHECK IF WE ARE NOT AIMING, IF NOT, RESET LAST TAGRET!
	if (!(gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_FIREREADY) &&
		!(gAnimControl[usNewState].uiFlags & ANIM_FIREREADY))
	{
		// ATE: Also check for the transition anims to not reset this
		// this should have used a flag but we're out of them....
		if ( usNewState != READY_RIFLE_STAND && usNewState != READY_RIFLE_PRONE &&
			usNewState != READY_RIFLE_CROUCH && usNewState != ROBOT_SHOOT )
		{
			pSoldier->sLastTarget = NOWHERE;
		}
	}

	// If a special move state, release np aps
	if ( ( gAnimControl[ usNewState ].uiFlags & ANIM_SPECIALMOVE ) )
	{
		AdjustNoAPToFinishMove( pSoldier, FALSE );
	}

	// ATE: If not a moving animation - turn off reverse....
	if ( !( gAnimControl[ usNewState ].uiFlags & ANIM_MOVING ) )
	{
		pSoldier->bReverse = FALSE;
	}

	// Do special things based on new state
	switch( usNewState )
	{
		case STANDING:

			// Update desired height
			pSoldier->ubDesiredHeight = ANIM_STAND;
			break;

		case CROUCHING:

			// Update desired height
			pSoldier->ubDesiredHeight = ANIM_CROUCH;
			break;

		case PRONE:

			// Update desired height
			pSoldier->ubDesiredHeight = ANIM_PRONE;
			break;

		case READY_RIFLE_STAND:
		case READY_RIFLE_PRONE:
		case READY_RIFLE_CROUCH:
		case READY_DUAL_STAND:
		case READY_DUAL_CROUCH:
		case READY_DUAL_PRONE:

			// OK, get points to ready weapon....
			if ( !pSoldier->fDontChargeReadyAPs )
			{
				sAPCost = GetAPsToReadyWeapon( pSoldier, usNewState );
				DeductPoints( pSoldier, sAPCost, sBPCost );
			}
			else
			{
				pSoldier->fDontChargeReadyAPs = FALSE;
			}
			break;

		case WALKING:

			pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
			pSoldier->ubPendingActionAnimCount = 0;
			break;

		case SWATTING:

			pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
			pSoldier->ubPendingActionAnimCount = 0;
			break;

		case CRAWLING:

			// Turn off flag...
			pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_OFF;
			pSoldier->ubPendingActionAnimCount = 0;
			pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
			break;

		case RUNNING:

			// Only if our previous is not running
			if ( pSoldier->usAnimState != RUNNING )
			{
				sAPCost = AP_START_RUN_COST;
				DeductPoints( pSoldier, sAPCost, sBPCost );
			}
			// Set pending action count to 0
			pSoldier->ubPendingActionAnimCount = 0;
			pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
			break;

		case ADULTMONSTER_WALKING:
			pSoldier->ubPendingActionAnimCount = 0;
			break;

		case ROBOT_WALK:
			pSoldier->ubPendingActionAnimCount = 0;
			break;

		case KNEEL_UP:
		case KNEEL_DOWN:
		case BIGMERC_CROUCH_TRANS_INTO:
		case BIGMERC_CROUCH_TRANS_OUTOF:

			if ( !pSoldier->fDontChargeAPsForStanceChange )
			{
				DeductPoints( pSoldier, AP_CROUCH, BP_CROUCH );
			}
			pSoldier->fDontChargeAPsForStanceChange = FALSE;
			break;

		case PRONE_UP:
		case PRONE_DOWN:

			// ATE: If we are NOT waiting for prone down...
			if ( pSoldier->fTurningFromPronePosition < TURNING_FROM_PRONE_START_UP_FROM_MOVE && !pSoldier->fDontChargeAPsForStanceChange )
			{
				// ATE: Don't do this if we are still 'moving'....
				if ( pSoldier->sGridNo == pSoldier->sFinalDestination || pSoldier->ubPathIndex == 0 )
				{
					DeductPoints( pSoldier, AP_PRONE, BP_PRONE );
				}
			}
			pSoldier->fDontChargeAPsForStanceChange = FALSE;
			break;

			//Deduct points for stance change
			//sAPCost = GetAPsToChangeStance( pSoldier, gAnimControl[ usNewState ].ubEndHeight );
			//DeductPoints( pSoldier, sAPCost, 0 );
			//break;

		case START_AID:

			DeductPoints( pSoldier, AP_START_FIRST_AID, BP_START_FIRST_AID );
			break;

		case CUTTING_FENCE:
			DeductPoints( pSoldier, AP_USEWIRECUTTERS, BP_USEWIRECUTTERS );
			break;

		case PLANT_BOMB:

			DeductPoints( pSoldier, AP_DROP_BOMB, 0 );
			break;

		case STEAL_ITEM:

			DeductPoints( pSoldier, AP_STEAL_ITEM, 0 );
			break;

		case CROW_DIE:

			// Delete shadow of crow....
			if ( pSoldier->pAniTile != NULL )
			{
				DeleteAniTile( pSoldier->pAniTile );
				pSoldier->pAniTile = NULL;
			}
			break;

		case CROW_FLY:
			// Ate: startup a shadow ( if gridno is set )
			HandleCrowShadowNewGridNo(*pSoldier);
			break;

		case CROW_EAT:
			// ATE: Make sure height level is 0....
			SetSoldierHeight( pSoldier, (FLOAT)(0) );
			HandleCrowShadowRemoveGridNo(*pSoldier);
			break;

		case USE_REMOTE:

			DeductPoints( pSoldier, AP_USE_REMOTE, 0 );
			break;

		//case PUNCH:

			//Deduct points for punching
			//sAPCost = MinAPsToAttack( pSoldier, pSoldier->sGridNo, FALSE );
			//DeductPoints( pSoldier, sAPCost, 0 );
			//break;

		case HOPFENCE:

			DeductPoints( pSoldier, AP_JUMPFENCE, BP_JUMPFENCE );
			break;

		// Deduct aps for falling down....
		case FALLBACK_HIT_STAND:
		case FALLFORWARD_FROMHIT_STAND:

			DeductPoints( pSoldier, AP_FALL_DOWN, BP_FALL_DOWN );
			break;

		case FALLFORWARD_FROMHIT_CROUCH:

			DeductPoints( pSoldier, (AP_FALL_DOWN/2), (BP_FALL_DOWN/2) );
			break;

		case QUEEN_SWIPE:

			// ATE: set damage counter...
			pSoldier->uiPendingActionData1 = 0;
			break;

		case CLIMBDOWNROOF:

			// disable sight
			gTacticalStatus.uiFlags |= DISALLOW_SIGHT;

			DeductPoints( pSoldier, AP_CLIMBOFFROOF, BP_CLIMBOFFROOF );
			break;

		case CLIMBUPROOF:

			// disable sight
			gTacticalStatus.uiFlags |= DISALLOW_SIGHT;

			DeductPoints( pSoldier, AP_CLIMBROOF, BP_CLIMBROOF );
			break;

		case JUMP_OVER_BLOCKING_PERSON:

		// Set path....
		{
			UINT16 usNewGridNo;

			DeductPoints( pSoldier, AP_JUMP_OVER, BP_JUMP_OVER );

			usNewGridNo = NewGridNo( (UINT16)pSoldier->sGridNo, DirectionInc( pSoldier->bDirection ) );
			usNewGridNo = NewGridNo( (UINT16)usNewGridNo, DirectionInc( pSoldier->bDirection ) );

			pSoldier->ubPathDataSize = 0;
			pSoldier->ubPathIndex    = 0;
			pSoldier->ubPathingData[ pSoldier->ubPathDataSize ] = pSoldier->bDirection;
			pSoldier->ubPathDataSize++;
			pSoldier->ubPathingData[ pSoldier->ubPathDataSize ] = pSoldier->bDirection;
			pSoldier->ubPathDataSize++;
			pSoldier->sFinalDestination = usNewGridNo;
			// Set direction
			EVENT_InternalSetSoldierDestination( pSoldier, pSoldier->ubPathingData[ pSoldier->ubPathIndex ], FALSE, JUMP_OVER_BLOCKING_PERSON );
		}
			break;


		case GENERIC_HIT_STAND:
		case GENERIC_HIT_CROUCH:
		case STANDING_BURST_HIT:
		case ADULTMONSTER_HIT:
		case ADULTMONSTER_DYING:
		case COW_HIT:
		case COW_DYING:
		case BLOODCAT_HIT:
		case BLOODCAT_DYING:
		case WATER_HIT:
		case WATER_DIE:
		case DEEP_WATER_HIT:
		case DEEP_WATER_DIE:
		case RIFLE_STAND_HIT:
		case LARVAE_HIT:
		case LARVAE_DIE:
		case QUEEN_HIT:
		case QUEEN_DIE:
		case INFANT_HIT:
		case INFANT_DIE:
		case CRIPPLE_HIT:
		case CRIPPLE_DIE:
		case CRIPPLE_DIE_FLYBACK:
		case ROBOTNW_HIT:
		case ROBOTNW_DIE:

			// Set getting hit flag to TRUE
			pSoldier->fGettingHit = TRUE;
			break;

		case CHARIOTS_OF_FIRE:
		case BODYEXPLODING:

			// Merc on fire!
			pSoldier->uiPendingActionData1 = PlaySoldierJA2Sample(pSoldier, FIRE_ON_MERC, HIGHVOLUME, 5, TRUE);
			break;
	}

	// Remove old animation profile
	HandleAnimationProfile(*pSoldier, pSoldier->usAnimState, TRUE);

	// From animation control, set surface
	if (!SetSoldierAnimationSurface(pSoldier, usNewState))
	{
		return;
	}


	// Set state
	pSoldier->usOldAniState = pSoldier->usAnimState;
	pSoldier->sOldAniCode = pSoldier->usAniCode;

	// Change state value!
	pSoldier->usAnimState = usNewState;

	pSoldier->sZLevelOverride = -1;

	if ( !( pSoldier->uiStatusFlags & SOLDIER_LOCKPENDINGACTIONCOUNTER ) )
	{
		//ATE Cancel ANY pending action...
		if ( pSoldier->ubPendingActionAnimCount > 0 && ( gAnimControl[ pSoldier->usOldAniState ].uiFlags & ANIM_MOVING ) )
		{
			// Do some special things for some actions
			switch( pSoldier->ubPendingAction )
			{
				case MERC_GIVEITEM:
					// Unset target as enaged
					GetMan(pSoldier->uiPendingActionData4).uiStatusFlags &= ~SOLDIER_ENGAGEDINACTION;
					break;
			}
			Soldier{pSoldier}.removePendingAction();
		}
		else
		{
			// Increment this for almost all animations except some movement ones...
			// That's because this represents ANY animation other than the one we began
			// when the pending action was started
			// ATE: Added to ignore this count if we are waiting for someone to move out of our way...
			if (usNewState != START_SWAT && usNewState != END_SWAT &&
				!(gAnimControl[usNewState].uiFlags & ANIM_NOCHANGE_PENDINGCOUNT) &&
				!pSoldier->fDelayedMovement &&
				!(pSoldier->uiStatusFlags & SOLDIER_ENGAGEDINACTION))
			{
				pSoldier->ubPendingActionAnimCount++;
			}
		}
	}

	// Set new animation profile
	HandleAnimationProfile(*pSoldier, usNewState, FALSE);

	// Reset some animation values
	pSoldier->fForceShade = FALSE;

	// CHECK IF WE ARE AT AN IDLE ACTION
#if 0
	if ( gAnimControl[ usNewState ].uiFlags & ANIM_IDLE )
	{
		pSoldier->bAction = ACTION_DONE;
	}
	else
	{
		pSoldier->bAction = ACTION_BUSY;
	}
#endif

	// Set current frame
	pSoldier->usAniCode  = usStartingAniCode;

	// ATE; For some animations that could use some variations, do so....
	if (usNewState == CHARIOTS_OF_FIRE || usNewState == BODYEXPLODING )
	{
		pSoldier->usAniCode = (UINT16)( Random( 10 ) );
	}

	// ATE: Default to first frame....
	// Will get changed ( probably ) by AdjustToNextAnimationFrame()
	ConvertAniCodeToAniFrame( pSoldier, (INT16)( 0 ) );

	// Set delay speed
	SetSoldierAniSpeed( pSoldier );

	// Reset counters
	RESETTIMECOUNTER( pSoldier->UpdateCounter, pSoldier->sAniDelay );

	// Adjust to new animation frame ( the first one )
	AdjustToNextAnimationFrame( pSoldier );

	// Setup offset information for UI above guy
	SetSoldierLocatorOffsets( pSoldier );

	// If we are certain animations, reload palette
	if ( usNewState == VEHICLE_DIE || usNewState == CHARIOTS_OF_FIRE || usNewState == BODYEXPLODING )
	{
		CreateSoldierPalettes(*pSoldier);
	}

	// ATE: if the old animation was a movement, and new is not, play sound...
	// OK, play final footstep sound...
	if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		if ( ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_STATIONARY ) &&
			( gAnimControl[ pSoldier->usOldAniState ].uiFlags & ANIM_MOVING ) )
		{
			PlaySoldierFootstepSound( pSoldier );
		}
	}

	// Free up from stance change
	FreeUpNPCFromStanceChange( pSoldier );

	CheckForFreeupFromHit(pSoldier, uiOldAnimFlags, uiNewAnimFlags, pSoldier->usOldAniState, usNewState);
}


static void InternalRemoveSoldierFromGridNo(SOLDIERTYPE& s, BOOLEAN const force)
{
	if (s.sGridNo == NOWHERE) return;

	if (!s.bInSector && !force) return;

	// Remove from world (old pos)
	RemoveMerc(s.sGridNo, s, false);
	HandleAnimationProfile(s, s.usAnimState, TRUE);

	// Remove records of this guy being adjacent
	for (INT8 dir = 0; dir < NUM_WORLD_DIRECTIONS; ++dir)
	{
		INT32 const grid_no = s.sGridNo + DirIncrementer[dir];
		if (grid_no < 0 || WORLD_MAX <= grid_no) continue;

		--gpWorldLevelData[grid_no].ubAdjacentSoldierCnt;
	}

	HandlePlacingRoofMarker(s, false, false);

	UnMarkMovementReserved(s);
	HandleCrowShadowRemoveGridNo(s);
	s.sGridNo = NOWHERE;
}


void RemoveSoldierFromGridNo(SOLDIERTYPE& s)
{
	InternalRemoveSoldierFromGridNo(s, FALSE);
}


static void SetSoldierGridNo(SOLDIERTYPE&, INT16 sNewGridNo, BOOLEAN fForceRemove);


static void EVENT_InternalSetSoldierPosition(SOLDIERTYPE* pSoldier, GridNo pos, FLOAT dNewXPos, FLOAT dNewYPos, SetSoldierPosFlags flags)
{
	// Not if we're dead!
	if ( ( pSoldier->uiStatusFlags & SOLDIER_DEAD ) )
	{
		return;
	}

	if (!(flags & SSP_NO_DEST))       pSoldier->sDestination      = pos;
	if (!(flags & SSP_NO_FINAL_DEST)) pSoldier->sFinalDestination = pos;

	// Set New pos
	pSoldier->dXPos = dNewXPos;
	pSoldier->dYPos = dNewYPos;

	pSoldier->sX = (INT16)dNewXPos;
	pSoldier->sY = (INT16)dNewYPos;

	HandleCrowShadowNewPosition( pSoldier );

	SetSoldierGridNo(*pSoldier, pos, (flags & SSP_FORCE_DELETE) != 0);

	if ( !( pSoldier->uiStatusFlags & ( SOLDIER_DRIVER | SOLDIER_PASSENGER ) ) )
	{
		if ( gGameSettings.fOptions[ TOPTION_MERC_ALWAYS_LIGHT_UP ] )
		{
			PositionSoldierLight(pSoldier);
		}
	}

	// ATE: Mirror calls if we are a vehicle ( for all our passengers )
	UpdateAllVehiclePassengersGridNo( pSoldier );
}


void EVENT_SetSoldierPosition(SOLDIERTYPE* const s, const GridNo pos, const SetSoldierPosFlags flags)
{
	INT16 x;
	INT16 y;
	ConvertGridNoToCenterCellXY(pos, &x, &y);
	EVENT_InternalSetSoldierPosition(s, pos, x, y, flags);
}


void EVENT_SetSoldierPositionNoCenter(SOLDIERTYPE* const s, const GridNo pos, const SetSoldierPosFlags flags)
{
	INT16 x;
	INT16 y;
	ConvertGridNoToCellXY(pos, &x, &y);
	EVENT_InternalSetSoldierPosition(s, pos, x, y, flags);
}


void EVENT_SetSoldierPositionXY(SOLDIERTYPE* const s, const FLOAT x, const FLOAT y, const SetSoldierPosFlags flags)
{
	EVENT_InternalSetSoldierPosition(s, GETWORLDINDEXFROMWORLDCOORDS(y, x), x, y, flags);
}


void SetSoldierHeight(SOLDIERTYPE* const s, FLOAT const new_height)
{
	s->dHeightAdjustment = new_height;
	s->sHeightAdjustment = (INT16)new_height;

	if (s->sHeightAdjustment > 0)
	{
		ApplyTranslucencyToWalls((INT16)(s->dXPos / CELL_X_SIZE), (INT16)(s->dYPos / CELL_Y_SIZE));
		s->bLevel = SECOND_LEVEL;
	}
	else
	{
		s->bLevel = FIRST_LEVEL;
	}
}


static void SetSoldierGridNo(SOLDIERTYPE& s, GridNo new_grid_no, BOOLEAN const fForceRemove)
{
	// Not if we're dead!
	if (s.uiStatusFlags & SOLDIER_DEAD) return;

	if (new_grid_no == s.sGridNo && s.pLevelNode) return;

	// Check if we are moving AND this is our next dest gridno....
	if (gAnimControl[s.usAnimState].uiFlags & (ANIM_MOVING | ANIM_SPECIALMOVE))
	{
		if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME))
		{
			if (new_grid_no != s.sDestination)
			{
				// This must be our new one, make it so
				new_grid_no = s.sDestination;
			}

			// Now check this baby
			if (new_grid_no == s.sGridNo) return;
		}
	}

	s.sOldGridNo = s.sGridNo;

	if (s.ubBodyType == QUEENMONSTER)
	{
		SetPositionSndGridNo(s.iPositionSndID, new_grid_no);
	}

	bool const in_vehicle = s.uiStatusFlags & (SOLDIER_DRIVER | SOLDIER_PASSENGER);
	if (!in_vehicle)
	{
		InternalRemoveSoldierFromGridNo(s, fForceRemove);
	}

	s.sGridNo = new_grid_no;

	// Check if our new gridno is valid, if not do not set!
	if (!GridNoOnVisibleWorldTile(new_grid_no)) return;

	// Alrighty, update UI for this guy, if he's the selected guy
	if (GetSelectedMan() == &s && guiCurrentEvent == C_WAIT_FOR_CONFIRM)
	{
		// Update path!
		gfPlotNewMovement = TRUE;
	}

	// Reset some flags for optimizations
	s.sWalkToAttackGridNo = NOWHERE;

	// check for special code to close door
	if (s.bEndDoorOpenCode == 2)
	{
		s.bEndDoorOpenCode = 0;
		HandleDoorChangeFromGridNo(&s, s.sEndDoorOpenCodeData, FALSE);
	}

	// Update buddy's strategic insertion code
	s.ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
	s.usStrategicInsertionData = new_grid_no;

	// Remove this gridno as a reserved place!
	if (!in_vehicle) UnMarkMovementReserved(s);

	if (s.sInitialGridNo == 0)
	{
		s.sInitialGridNo  = new_grid_no;
		s.usPatrolGrid[0] = new_grid_no;
	}

	// Add records of this guy being adjacent
	for (INT8 dir = 0; dir < NUM_WORLD_DIRECTIONS; ++dir)
	{
		++gpWorldLevelData[new_grid_no + DirIncrementer[dir]].ubAdjacentSoldierCnt;
	}

	if (!in_vehicle) DropSmell(s);

	// Handle any special rendering situations
	s.sZLevelOverride = -1;

	// If we are over a fence (hopping), make us higher!
	if (IsJumpableFencePresentAtGridno(new_grid_no))
	{
		s.sZLevelOverride = TOPMOST_Z_LEVEL;
	}

	// Add merc at new pos
	if (!in_vehicle)
	{
		LEVELNODE* const n = AddMercToHead(new_grid_no, s, TRUE);

		// If we are in the middle of climbing the roof!
		if (s.usAnimState == CLIMBUPROOF)
		{
			if (s.light) LightSpriteRoofStatus(s.light, TRUE);
		}
		else if (s.usAnimState == CLIMBDOWNROOF)
		{
			if (s.light) LightSpriteRoofStatus(s.light, FALSE);
		}

		//JA2Gold: If the player wants the merc to cast the fake light AND it is night
		if (s.bTeam != OUR_TEAM || (gGameSettings.fOptions[TOPTION_MERC_CASTS_LIGHT] && NightTime()))
		{
			MAP_ELEMENT const& m     = gpWorldLevelData[new_grid_no];
			LEVELNODE   const& other = s.bLevel > 0 && m.pRoofHead ? *m.pRoofHead : *m.pLandHead;
			n->ubShadeLevel        = other.ubShadeLevel;
			n->ubSumLights         = other.ubSumLights;
			n->ubMaxLights         = other.ubMaxLights;
			n->ubNaturalShadeLevel = other.ubNaturalShadeLevel;
		}
		else    //The player DOESNT want the mercs to cast the fake lights
		{
			//Only light the soldier
			SetSoldierPersonalLightLevel(&s);
		}

		HandleAnimationProfile(s, s.usAnimState, FALSE);
		HandleCrowShadowNewGridNo(s);
	}

	INT8 const old_over_terrain_type = s.bOverTerrainType;
	s.bOverTerrainType = GetTerrainType(new_grid_no);

	// Check that our animation is up to date!
	if (!in_vehicle)
	{
		BOOLEAN const in_water = MercInWater(&s);

		// ATE: If ever in water make sure we walk afterwoods!
		if (in_water) s.usUIMovementMode = WALKING;

		if (in_water != s.fPrevInWater)
		{
			// Update Animation data
			SetSoldierAnimationSurface(&s, s.usAnimState);

			s.fPrevInWater = in_water;

			// Update sound
			if (in_water)
			{
				PlaySoldierJA2Sample(&s, ENTER_WATER_1, MIDVOLUME, 1, TRUE);
			}
			else
			{
				// ATE: Check if we are going from water to land - if so, resume with
				// regular movement mode
				EVENT_InitNewSoldierAnim(&s, s.usUIMovementMode, 0, FALSE);
			}
		}

		// OK, If we were not in deep water but we are now, handle deep animations!
		if (s.bOverTerrainType == DEEP_WATER && old_over_terrain_type != DEEP_WATER)
		{
			// Based on our current animation, change!
			switch (s.usAnimState)
			{
				case WALKING:
				case RUNNING:
					// In deep water, swim!
					// Make transition from low to deep
					EVENT_InitNewSoldierAnim(&s, LOW_TO_DEEP_WATER, 0, FALSE);
					s.usPendingAnimation = DEEP_WATER_SWIM;
					PlayLocationJA2Sample(new_grid_no, ENTER_DEEP_WATER_1, MIDVOLUME, 1);
			}
		}

		// Damage water if in deep water
		if (s.bOverTerrainType == MED_WATER || s.bOverTerrainType == DEEP_WATER)
		{
			WaterDamage(s);
		}

		// OK, If we were in deep water but we are NOT now, handle mid animations!
		if (s.bOverTerrainType != DEEP_WATER && old_over_terrain_type == DEEP_WATER)
		{
			// Make transition from low to deep
			EVENT_InitNewSoldierAnim(&s, DEEP_TO_LOW_WATER, 0, FALSE);
			s.usPendingAnimation = s.usUIMovementMode;
		}
	}

	// Are we now standing in tear gas without a decently working gas mask?
	if (GetSmokeEffectOnTile(new_grid_no, s.bLevel) != SmokeEffectID::NOTHING &&
		(s.inv[HEAD1POS].usItem != GASMASK || s.inv[HEAD1POS].bStatus[0] < GASMASK_MIN_STATUS) &&
		(s.inv[HEAD2POS].usItem != GASMASK || s.inv[HEAD2POS].bStatus[0] < GASMASK_MIN_STATUS))
	{
		s.uiStatusFlags |= SOLDIER_GASSED;
	}

	// Merc got to a new tile by "sneaking". Did we theoretically sneak past an enemy?
	if (s.bTeam == OUR_TEAM &&
		s.bStealthMode &&
		s.bOppCnt > 0) // opponents in sight
	{
		// Check each possible enemy
		CFOR_EACH_SOLDIER(enemy)
		{
			// If this guy is here and alive enough to be looking for us
			if (!enemy->bInSector)     continue;
			if (enemy->bLife < OKLIFE) continue;

			// No points for sneaking by the neutrals & friendlies
			if (enemy->bNeutral)           continue;
			if (s.bSide == enemy->bSide)   continue;
			if (enemy->ubBodyType == COW)  continue;
			if (enemy->ubBodyType == CROW) continue;

			// if we SEE this particular oppponent, and he DOESN'T see us and he COULD
			// see us
			if (s.bOppList[enemy->ubID] != SEEN_CURRENTLY) continue;
			if (enemy->bOppList[s.ubID] == SEEN_CURRENTLY) continue;
			if (PythSpacesAway(new_grid_no, enemy->sGridNo) >= DistanceVisible(enemy, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, new_grid_no, s.bLevel)) continue;

			// AGILITY (5):  Soldier snuck 1 square past unaware enemy
			StatChange(s, AGILAMT, 5, FROM_SUCCESS);
			// Keep looping, we'll give'em 1 point for EACH such enemy!
		}
	}

	// Adjust speed based on terrain, etc
	SetSoldierAniSpeed(&s);

	if (s.bTeam == ENEMY_TEAM &&
		(!gpWorldLevelData[s.sGridNo].pMercHead ||
	     !gpWorldLevelData[s.sGridNo].pMercHead->pStructureData))
	{
		// Debug help for issue #1681: set a break point here and check who
		// is calling this function without calling InternalIsValidStance first.
		SLOGW("SetSoldierGridNo: ID {} ({}) has no structure", s.ubID, s.name);
	}
}


static UINT16 SelectFireAnimation(SOLDIERTYPE* pSoldier, UINT8 ubHeight);


void EVENT_FireSoldierWeapon( SOLDIERTYPE *pSoldier, INT16 sTargetGridNo )
{
	BOOLEAN fDoFireRightAway = FALSE;

	// CANNOT BE SAME GRIDNO!
	if ( pSoldier->sGridNo == sTargetGridNo )
	{
		return;
	}

	// Increment the number of people busy doing stuff because of an attack
	//if (gTacticalStatus.uiFlags & INCOMBAT)
	//{
		gTacticalStatus.ubAttackBusyCount++;
		SLOGD("Starting attack, attack count now {}",
			gTacticalStatus.ubAttackBusyCount);
	//}

	// Set soldier's target gridno
	// This assignment was redundent because it's already set in
	// the actual event call
	pSoldier->sTargetGridNo = sTargetGridNo;
	//pSoldier->sLastTarget = sTargetGridNo;
	pSoldier->target = WhoIsThere2(sTargetGridNo, pSoldier->bTargetLevel);

	auto item = GCM->getItem(pSoldier->inv[HANDPOS].usItem);
	if (item->isGun())
	{
		if (pSoldier->bDoBurst)
		{
			// Set the TOTAL number of bullets to be fired
			// Can't shoot more bullets than we have in our magazine!
			pSoldier->bBulletsLeft = std::min(GCM->getWeapon(pSoldier->inv[ pSoldier->ubAttackingHand].usItem)->ubShotsPerBurst, pSoldier->inv[ pSoldier->ubAttackingHand ].ubGunShotsLeft);
		}
		else if ( IsValidSecondHandShot( pSoldier ) )
		{
			// two-pistol attack - two bullets!
			pSoldier->bBulletsLeft = 2;
		}
		else
		{
			pSoldier->bBulletsLeft = 1;
		}
		if (pSoldier->inv[ pSoldier->ubAttackingHand ].ubGunAmmoType == AMMO_BUCKSHOT)
		{
			pSoldier->bBulletsLeft *= NUM_BUCKSHOT_PELLETS;
		}
	}
	SLOGD("Starting attack, bullets left {}", pSoldier->bBulletsLeft);

	// Change to fire animation
	SoldierReadyWeapon(pSoldier, sTargetGridNo, FALSE);

	// IF WE ARE AN NPC, SLIDE VIEW TO SHOW WHO IS SHOOTING
	{
		//if ( pSoldier->fDoSpread )
		//{
			// If we are spreading burst, goto right away!
			//EVENT_InitNewSoldierAnim( pSoldier, SelectFireAnimation( pSoldier, gAnimControl[ pSoldier->usAnimState ].ubEndHeight ), 0, FALSE );

		//}

		// else
		{
			if (pSoldier->uiStatusFlags & SOLDIER_MONSTER )
			{
				// Force our direction!
				EVENT_SetSoldierDirection( pSoldier, pSoldier->bDesiredDirection );
				EVENT_InitNewSoldierAnim( pSoldier, SelectFireAnimation( pSoldier, gAnimControl[ pSoldier->usAnimState ].ubEndHeight ), 0, FALSE );
			}
			else
			{
				// IF WE ARE IN REAl-TIME, FIRE IMMEDIATELY!
				if (!(gTacticalStatus.uiFlags & INCOMBAT))
				{
					//fDoFireRightAway = TRUE;
				}

				// Check if our weapon has no intermediate anim...
				if (item->isLauncher() || item->getItemIndex() == ROCKET_LAUNCHER) {
					fDoFireRightAway = TRUE;
				}

				if ( fDoFireRightAway )
				{
					// Set to true so we don't get toasted twice for APs..
					pSoldier->fDontUnsetLastTargetFromTurn = TRUE;

					// Make sure we don't try and do fancy prone turning.....
					pSoldier->fTurningFromPronePosition = FALSE;

					// Force our direction!
					EVENT_SetSoldierDirection( pSoldier, pSoldier->bDesiredDirection );

					EVENT_InitNewSoldierAnim( pSoldier, SelectFireAnimation( pSoldier, gAnimControl[ pSoldier->usAnimState ].ubEndHeight ), 0, FALSE );
				}
				else
				{
					// Set flag indicating we are about to shoot once destination direction is hit
					pSoldier->fTurningToShoot = TRUE;

					if ( pSoldier->bTeam != OUR_TEAM  && pSoldier->bVisible != -1)
					{
						LocateSoldier(pSoldier, DONTSETLOCATOR);
					}
				}
			}
		}
	}
}


static UINT16 SelectFireAnimation(SOLDIERTYPE* pSoldier, UINT8 ubHeight)
{
	INT16   sDist;
	FLOAT   dTargetX;
	FLOAT   dTargetY;
	FLOAT   dTargetZ;
	BOOLEAN fDoLowShot = FALSE;


	//Do different things if we are a monster
	if (pSoldier->uiStatusFlags & SOLDIER_MONSTER)
	{
		switch( pSoldier->ubBodyType )
		{
			case ADULTFEMALEMONSTER:
			case AM_MONSTER:
			case YAF_MONSTER:
			case YAM_MONSTER:
				return( MONSTER_SPIT_ATTACK );

			case LARVAE_MONSTER:
				break;

			case INFANT_MONSTER:
				return( INFANT_ATTACK );

			case QUEENMONSTER:
				return( QUEEN_SPIT );
		}
		return( TRUE );
	}

	if ( pSoldier->ubBodyType == ROBOTNOWEAPON )
	{
		if ( pSoldier->bDoBurst > 0 )
		{
			return( ROBOT_BURST_SHOOT );
		}
		else
		{
			return( ROBOT_SHOOT );
		}
	}

	// Check for rocket laucncher....
	if ( pSoldier->inv[ HANDPOS ].usItem == ROCKET_LAUNCHER )
	{
		return( SHOOT_ROCKET );
	}

	// Check for rocket laucncher....
	if ( pSoldier->inv[ HANDPOS ].usItem == MORTAR )
	{
		return( SHOOT_MORTAR );
	}

	// Check for tank cannon
	if ( pSoldier->inv[ HANDPOS ].usItem == TANK_CANNON )
	{
		return( TANK_SHOOT );
	}

	if ( pSoldier->ubBodyType == TANK_NW || pSoldier->ubBodyType == TANK_NE )
	{
		return( TANK_BURST );
	}

	// Determine which animation to do...depending on stance and gun in hand...
	switch ( ubHeight )
	{
		case ANIM_STAND:
			// CHECK 2ND HAND!
			if ( IsValidSecondHandShot( pSoldier ) )
			{
				// Increment the number of people busy doing stuff because of an attack
				//gTacticalStatus.ubAttackBusyCount++;
				//SLOGD("Starting attack with 2 guns, attack count now {}", gTacticalStatus.ubAttackBusyCount);

				return( SHOOT_DUAL_STAND );
			}
			else
			{
				// OK, while standing check distance away from target, and shoot low if we should!
				sDist = PythSpacesAway( pSoldier->sGridNo, pSoldier->sTargetGridNo );

				//ATE: OK, SEE WERE WE ARE TARGETING....
				GetTargetWorldPositions( pSoldier, pSoldier->sTargetGridNo, &dTargetX, &dTargetY, &dTargetZ );

				//CalculateSoldierZPos( pSoldier, FIRING_POS, &dFirerZ );

				if ( sDist <= 2 && dTargetZ <= 100 )
				{
					fDoLowShot = TRUE;
				}

				// ATE: Made distence away long for psitols such that they never use this....
				//if ( !(GCM->getItem(usItem)->isTwoHanded()) )
				//{
				//	fDoLowShot = FALSE;
				//}

				// Don't do any low shots if in water
				if ( MercInWater( pSoldier ) )
				{
					fDoLowShot = FALSE;
				}


				if ( pSoldier->bDoBurst > 0 )
				{
					if ( fDoLowShot )
					{
						return( FIRE_BURST_LOW_STAND );
					}
					else
					{
						return( STANDING_BURST );
					}
				}
				else
				{
					if ( fDoLowShot )
					{
						return( FIRE_LOW_STAND );
					}
					else
					{
						return( SHOOT_RIFLE_STAND );
					}
				}
			}

		case ANIM_PRONE:

			if ( pSoldier->bDoBurst > 0 )
			{
				//pSoldier->fBurstCompleted = FALSE;
				return( PRONE_BURST );
			}
			else
			{
				if ( IsValidSecondHandShot( pSoldier ) )
				{
					return( SHOOT_DUAL_PRONE );
				}
				else
				{
					return( SHOOT_RIFLE_PRONE );
				}
			}

		case ANIM_CROUCH:

			if ( IsValidSecondHandShot( pSoldier ) )
			{
				// Increment the number of people busy doing stuff because of an attack
				//gTacticalStatus.ubAttackBusyCount++;
				//SLOGD("Starting attack with 2 guns, attack count now {}", gTacticalStatus.ubAttackBusyCount);

				return( SHOOT_DUAL_CROUCH );
			}
			else
			{
				if ( pSoldier->bDoBurst > 0 )
				{
					//pSoldier->fBurstCompleted = FALSE;
					return( CROUCHED_BURST );
				}
				else
				{
					return( SHOOT_RIFLE_CROUCH );
				}
			}

		default:
			AssertMsg(FALSE, ST::format("SelectFireAnimation: ERROR - Invalid height {}", ubHeight));
			break;
	}


	// If here, an internal error has occured!
	Assert( FALSE );
	return ( 0 );
}


UINT16 GetMoveStateBasedOnStance(const SOLDIERTYPE* const s, const UINT8 ubStanceHeight)
{
	switch (ubStanceHeight)
	{
		case ANIM_STAND:
			return s->fUIMovementFast && !(s->uiStatusFlags & SOLDIER_VEHICLE) ?
				RUNNING : WALKING;

		case ANIM_PRONE:  return CRAWLING;
		case ANIM_CROUCH: return SWATTING;

		default:
			AssertMsg(FALSE, ST::format("GetMoveStateBasedOnStance: ERROR - Invalid height {}",
					ubStanceHeight));
			return 0;
	}
}


BOOLEAN SoldierReadyWeapon(SOLDIERTYPE* const pSoldier, const GridNo tgt_pos, const BOOLEAN fEndReady)
{
	const INT16 sFacingDir = GetDirectionFromGridNo(tgt_pos, pSoldier);
	return InternalSoldierReadyWeapon(pSoldier, sFacingDir, fEndReady);
}


static void EVENT_InternalSetSoldierDesiredDirection(SOLDIERTYPE* pSoldier, UINT16 usNewDirection, BOOLEAN fInitalMove, UINT16 usAnimState);


BOOLEAN InternalSoldierReadyWeapon( SOLDIERTYPE *pSoldier, UINT8 sFacingDir, BOOLEAN fEndReady )
{
	UINT16 usAnimState;
	BOOLEAN	fReturnVal = FALSE;

	// Handle monsters differently
	if (pSoldier->uiStatusFlags & SOLDIER_MONSTER)
	{
		if ( !fEndReady )
		{
			EVENT_SetSoldierDesiredDirection( pSoldier, sFacingDir );
		}
		return( FALSE );
	}

	usAnimState = PickSoldierReadyAnimation( pSoldier, fEndReady );

	if ( usAnimState != INVALID_ANIMATION )
	{
		EVENT_InitNewSoldierAnim( pSoldier, usAnimState, 0 , FALSE );
		fReturnVal = TRUE;
	}

	if ( !fEndReady )
	{
		// Ready direction for new facing direction
		if ( usAnimState == INVALID_ANIMATION )
		{
			usAnimState = pSoldier->usAnimState;
		}

		EVENT_InternalSetSoldierDesiredDirection( pSoldier, sFacingDir, FALSE, usAnimState );

		// Check if facing dir is different from ours and change direction if so!
		//if ( sFacingDir != pSoldier->bDirection )
		//{
		//	DeductPoints( pSoldier, AP_CHANGE_FACING, 0 );
		//}//

	}

	return( fReturnVal );
}


UINT16 PickSoldierReadyAnimation(SOLDIERTYPE* pSoldier, BOOLEAN fEndReady)
{

	// Invalid animation if nothing in our hands
	if ( pSoldier->inv[ HANDPOS ].usItem == NOTHING )
	{
		return( INVALID_ANIMATION );
	}

	if ( pSoldier->bOverTerrainType == DEEP_WATER )
	{
		return( INVALID_ANIMATION );
	}

	if ( pSoldier->ubBodyType == ROBOTNOWEAPON )
	{
		return( INVALID_ANIMATION );
	}

	// Check if we have a gun.....
	auto item = GCM->getItem(pSoldier->inv[ HANDPOS ].usItem);
	if (item->getItemClass() != IC_GUN && !item->isLauncher())
	{
		return( INVALID_ANIMATION );
	}

	if ( pSoldier->inv[ HANDPOS ].usItem == ROCKET_LAUNCHER )
	{
		return( INVALID_ANIMATION );
	}

	if ( pSoldier->ubBodyType == TANK_NW || pSoldier->ubBodyType == TANK_NE )
	{
		return( INVALID_ANIMATION );
	}

	if ( fEndReady )
	{
		// IF our gun is already drawn, do not change animation, just direction
		if ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ( ANIM_FIREREADY | ANIM_FIRE ))
		{

			switch ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight )
			{
				case ANIM_STAND:

					// CHECK 2ND HAND!
					if ( IsValidSecondHandShot( pSoldier ) )
					{
						return( END_DUAL_STAND );
					}
					else
					{
						return( END_RIFLE_STAND );
					}

				case ANIM_PRONE:

					if ( IsValidSecondHandShot( pSoldier ) )
					{
						return( END_DUAL_PRONE );
					}
					else
					{
						return( END_RIFLE_PRONE );
					}

				case ANIM_CROUCH:

					// CHECK 2ND HAND!
					if ( IsValidSecondHandShot( pSoldier ) )
					{
						return( END_DUAL_CROUCH );
					}
					else
					{
						return( END_RIFLE_CROUCH );
					}

			}

		}
	}
	else
	{
		// IF our gun is already drawn, do not change animation, just direction
		if ( !(gAnimControl[ pSoldier->usAnimState ].uiFlags & ( ANIM_FIREREADY | ANIM_FIRE ) ) )
		{
			switch ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight )
			{
				case ANIM_STAND:

					// CHECK 2ND HAND!
					if ( IsValidSecondHandShot( pSoldier ) )
					{
						return( READY_DUAL_STAND );
					}
					else
					{
						return( READY_RIFLE_STAND );
					}

				case ANIM_PRONE:
					// Go into crouch, turn, then go into prone again
					//ChangeSoldierStance( pSoldier, ANIM_CROUCH );
					//pSoldier->ubDesiredHeight = ANIM_PRONE;
					//ChangeSoldierState( pSoldier, PRONE_UP );
					if ( IsValidSecondHandShot( pSoldier ) )
					{
						return( READY_DUAL_PRONE );
					}
					else
					{
						return( READY_RIFLE_PRONE );
					}

				case ANIM_CROUCH:

					// CHECK 2ND HAND!
					if ( IsValidSecondHandShot( pSoldier ) )
					{
						return( READY_DUAL_CROUCH );
					}
					else
					{
						return( READY_RIFLE_CROUCH );
					}
			}
		}
	}

	return( INVALID_ANIMATION );
}


static UINT8 CalcScreamVolume(SOLDIERTYPE* pSoldier, UINT8 ubCombinedLoss);
static UINT32 SleepDartSuccumbChance(const SOLDIERTYPE* pSoldier);
static void SoldierGotHitBlade(SOLDIERTYPE* pSoldier);
static void SoldierGotHitExplosion(SOLDIERTYPE* pSoldier, UINT16 usWeaponIndex, UINT16 bDirection, UINT16 sRange);
static void SoldierGotHitGunFire(SOLDIERTYPE* pSoldier, UINT16 bDirection, SOLDIERTYPE* att, UINT8 ubSpecial);
static void SoldierGotHitPunch(SOLDIERTYPE* pSoldier);


// ATE: THIS FUNCTION IS USED FOR ALL SOLDIER TAKE DAMAGE FUNCTIONS!
void EVENT_SoldierGotHit(SOLDIERTYPE* pSoldier, const UINT16 usWeaponIndex, INT16 sDamage, INT16 sBreathLoss, const UINT16 bDirection, const UINT16 sRange, SOLDIERTYPE* const att, const UINT8 ubSpecial, const UINT8 ubHitLocation, const INT16 sLocationGrid)
{
	UINT8 ubCombinedLoss, ubVolume, ubReason;
	SOLDIERTYPE *pNewSoldier;

	ubReason = 0;

	// ATE: If we have gotten hit, but are still in our attack animation, reduce count!
	switch ( pSoldier->usAnimState )
	{
		case SHOOT_ROCKET:
		case SHOOT_MORTAR:
		case THROW_ITEM:
		case LOB_ITEM:
			SLOGD("Freeing up attacker - ATTACK ANIMATION {} ENDED BY HIT ANIMATION, Now {}",
				gAnimControl[pSoldier->usAnimState].zAnimStr, gTacticalStatus.ubAttackBusyCount);
			ReduceAttackBusyCount(pSoldier, FALSE);
			break;
	}

	// DO STUFF COMMON FOR ALL TYPES
	if (att != NULL) att->bLastAttackHit = TRUE;

	pSoldier->attacker = att;

	if ( !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
	{
		// Increment  being attacked count
		pSoldier->bBeingAttackedCount++;
	}

	// if defender is a vehicle, there will be no hit animation played!
	if ( !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
	{
		// Increment the number of people busy doing stuff because of an attack (busy doing hit anim!)
		gTacticalStatus.ubAttackBusyCount++;
		SLOGD("Person got hit, attack count now {}", gTacticalStatus.ubAttackBusyCount);
	}

	// ATE; Save hit location info...( for later anim determination stuff )
	pSoldier->ubHitLocation = ubHitLocation;

	// handle morale for heavy damage attacks
	if ( sDamage > 25 )
	{
		if (att != NULL)
		{
			if (att->bTeam == OUR_TEAM)
			{
				HandleMoraleEvent(att, MORALE_DID_LOTS_OF_DAMAGE, att->sSector);
			}
		}
		if (pSoldier->bTeam == OUR_TEAM)
		{
			HandleMoraleEvent(pSoldier, MORALE_TOOK_LOTS_OF_DAMAGE, pSoldier->sSector);
		}
	}

	// SWITCH IN TYPE OF WEAPON
	if ( ubSpecial == FIRE_WEAPON_TOSSED_OBJECT_SPECIAL )
	{
		ubReason = TAKE_DAMAGE_OBJECT;
	}
	else if ( GCM->getItem(usWeaponIndex)->isTentacles() )
	{
		ubReason = TAKE_DAMAGE_TENTACLES;
	}
	else if ( GCM->getItem(usWeaponIndex)->getItemClass() & ( IC_GUN | IC_THROWING_KNIFE ) )
	{
		if ( ubSpecial == FIRE_WEAPON_SLEEP_DART_SPECIAL )
		{
			UINT32	uiChance;

			// put the drug in!
			pSoldier->bSleepDrugCounter = 10;

			uiChance = SleepDartSuccumbChance( pSoldier );

			if ( PreRandom( 100 ) < uiChance )
			{
				// succumb to the drug!
				sBreathLoss = (INT16)( pSoldier->bBreathMax * 100 );
			}

		}
		else if ( ubSpecial == FIRE_WEAPON_BLINDED_BY_SPIT_SPECIAL )
		{
			// blinded!!
			if ( pSoldier->bBlindedCounter == 0 )
			{
				// say quote
				if (pSoldier->uiStatusFlags & SOLDIER_PC)
				{
					TacticalCharacterDialogue( pSoldier, QUOTE_BLINDED );
				}
				DecayIndividualOpplist( pSoldier );
			}
			// will always increase counter by at least 1
			pSoldier->bBlindedCounter += (sDamage / 8) + 1;

			// Dirty panel
			fInterfacePanelDirty = DIRTYLEVEL2;
		}
		sBreathLoss += BP_GET_HIT;
		ubReason = TAKE_DAMAGE_GUNFIRE;
	}
	else if ( GCM->getItem(usWeaponIndex)->isBlade() )
	{
		sBreathLoss = BP_GET_HIT;
		ubReason = TAKE_DAMAGE_BLADE;
	}
	else if ( GCM->getItem(usWeaponIndex)->isPunch() )
	{
		// damage from hand-to-hand is 1/4 normal, 3/4 breath.. the sDamage value
		// is actually how much breath we'll take away
		sBreathLoss = sDamage * 100;
		sDamage = sDamage / PUNCH_REAL_DAMAGE_PORTION;
		if ( AreInMeanwhile() && gCurrentMeanwhileDef.ubMeanwhileID == INTERROGATION )
		{
			sBreathLoss = 0;
			sDamage /= 2;
		}
		ubReason = TAKE_DAMAGE_HANDTOHAND;
	}
	else if ( GCM->getItem(usWeaponIndex)->isExplosive() )
	{
		if ( usWeaponIndex == STRUCTURE_EXPLOSION )
		{
			ubReason = TAKE_DAMAGE_STRUCTURE_EXPLOSION;
		}
		else
		{
			ubReason = TAKE_DAMAGE_EXPLOSION;
		}
	}
	else
	{
		SLOGW("Soldier Control: Weapon class not handled in SoldierGotHit( ) {}",
			usWeaponIndex);
	}


	// CJC: moved to after SoldierTakeDamage so that any quotes from the defender
	// will not be said if they are knocked out or killed
	if ( ubReason != TAKE_DAMAGE_TENTACLES && ubReason != TAKE_DAMAGE_OBJECT )
	{
		// OK, OK: THis is hairy, however, it's ness. because the normal freeup call uses the
		// attckers intended target, and here we want to use thier actual target....

		// ATE: If it's from GUNFIRE damage, keep in mind bullets...
		if ( GCM->getItem(usWeaponIndex)->isGun())
		{
			pNewSoldier = FreeUpAttackerGivenTarget(pSoldier);
		}
		else
		{
			pNewSoldier = ReduceAttackBusyGivenTarget(pSoldier);
		}

		if (pNewSoldier != NULL)
		{
			pSoldier = pNewSoldier;
		}
		SLOGD("Tried to free up attacker, attack count now {}",
			gTacticalStatus.ubAttackBusyCount);
	}


	// OK, If we are a vehicle.... damage vehicle...( people inside... )
	if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		SoldierTakeDamage(pSoldier, sDamage, sBreathLoss, ubReason, att);
		return;
	}

	// DEDUCT LIFE
	ubCombinedLoss = SoldierTakeDamage(pSoldier, sDamage, sBreathLoss, ubReason, att);

	// ATE: OK, Let's check our ASSIGNMENT state,
	// If anything other than on a squad or guard, make them guard....
	if ( pSoldier->bTeam == OUR_TEAM )
	{
		if ( pSoldier->bAssignment >= ON_DUTY && pSoldier->bAssignment != ASSIGNMENT_POW )
		{
			if( pSoldier->fMercAsleep )
			{
				pSoldier->fMercAsleep = FALSE;
				pSoldier -> fForcedToStayAwake = FALSE;

				// refresh map screen
				fCharacterInfoPanelDirty = TRUE;
				fTeamPanelDirty = TRUE;
			}

			AddCharacterToAnySquad( pSoldier );
		}
	}


	// SCREAM!!!!
	ubVolume = CalcScreamVolume( pSoldier, ubCombinedLoss );

	// IF WE ARE AT A HIT_STOP ANIMATION
	// DO APPROPRIATE HITWHILE DOWN ANIMATION
	if ( !( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_HITSTOP )
		|| pSoldier->usAnimState != JFK_HITDEATH_STOP )
	{
		MakeNoise(pSoldier, pSoldier->sGridNo, pSoldier->bLevel, ubVolume, NOISE_SCREAM);
	}

	// IAN ADDED THIS SAT JUNE 14th : HAVE TO SHOW VICTIM!
	if ((gTacticalStatus.uiFlags & INCOMBAT) &&
		pSoldier->bVisible != -1 && pSoldier->bTeam == OUR_TEAM)
	{
		LocateSoldier(pSoldier, DONTSETLOCATOR);
	}


	if ( GCM->getItem(usWeaponIndex)->isBlade() )
	{
		PlayLocationJA2Sample(pSoldier->sGridNo, KNIFE_IMPACT, MIDVOLUME, 1);
	}
	else
	{
		PlayLocationJA2Sample(pSoldier->sGridNo, SoundRange<BULLET_IMPACT_1, BULLET_IMPACT_3>(), MIDVOLUME, 1);
	}

	// PLAY RANDOM GETTING HIT SOUND
	// ONLY IF WE ARE CONSCIOUS!
	if ( pSoldier->bLife >= CONSCIOUSNESS )
	{
		if ( pSoldier->ubBodyType == CROW )
		{
			// Exploding crow...
			PlayLocationJA2Sample(pSoldier->sGridNo, CROW_EXPLODE_1, HIGHVOLUME, 1);
		}
		else
		{
			// ATE: This is to disallow large amounts of smaples being played which is load!
			if ( pSoldier->fGettingHit && pSoldier->usAniCode != STANDING_BURST_HIT )
			{

			}
			else
			{
				DoMercBattleSound(pSoldier, BATTLE_SOUND_HIT1);
			}
		}
	}

	// CHECK FOR DOING HIT WHILE DOWN
	if ( ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_HITSTOP ) )
	{
		UINT16 state;
		switch (pSoldier->usAnimState)
		{
			case FLYBACKHIT_STOP:            state = FALLBACK_DEATHTWICH;        break;
			case STAND_FALLFORWARD_STOP:     state = GENERIC_HIT_DEATHTWITCHNB;  break;
			case JFK_HITDEATH_STOP:          state = JFK_HITDEATH_TWITCHB;       break;
			case FALLBACKHIT_STOP:           state = FALLBACK_HIT_DEATHTWITCHNB; break;
			case PRONE_LAYFROMHIT_STOP:      state = PRONE_HIT_DEATHTWITCHNB;    break;
			case PRONE_HITDEATH_STOP:        state = PRONE_HIT_DEATHTWITCHB;     break;
			case FALLFORWARD_HITDEATH_STOP:  state = GENERIC_HIT_DEATHTWITCHB;   break;
			case FALLBACK_HITDEATH_STOP:     state = FALLBACK_HIT_DEATHTWITCHB;  break;
			case FALLOFF_DEATH_STOP:         state = FALLOFF_TWITCHB;            break;
			case FALLOFF_STOP:               state = FALLOFF_TWITCHNB;           break;
			case FALLOFF_FORWARD_DEATH_STOP: state = FALLOFF_FORWARD_TWITCHB;    break;
			case FALLOFF_FORWARD_STOP:       state = FALLOFF_FORWARD_TWITCHNB;   break;

			default:
				SLOGD("Death state {} has no death hit", pSoldier->usAnimState);
				return;
		}
		ChangeSoldierState(pSoldier, state, 0, FALSE);
		return;
	}

	// Set goback to aim after hit flag!
	// Only if we were aiming!
	if ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_FIREREADY )
	{
		pSoldier->fGoBackToAimAfterHit = TRUE;
	}

	// IF COWERING, PLAY SPECIFIC GENERIC HIT STAND...
	if ( pSoldier->uiStatusFlags & SOLDIER_COWERING )
	{
		if ( pSoldier->bLife == 0 || IS_MERC_BODY_TYPE( pSoldier ) )
		{
			EVENT_InitNewSoldierAnim( pSoldier, GENERIC_HIT_STAND, 0 , FALSE );
		}
		else
		{
			EVENT_InitNewSoldierAnim( pSoldier, CIV_COWER_HIT, 0 , FALSE );
		}
		return;
	}

	// Change based on body type
	switch( pSoldier->ubBodyType )
	{
		case COW:
			EVENT_InitNewSoldierAnim( pSoldier, COW_HIT, 0 , FALSE );
			return;

		case BLOODCAT:
			EVENT_InitNewSoldierAnim( pSoldier, BLOODCAT_HIT, 0 , FALSE );
			return;

		case ADULTFEMALEMONSTER:
		case AM_MONSTER:
		case YAF_MONSTER:
		case YAM_MONSTER:

			EVENT_InitNewSoldierAnim( pSoldier, ADULTMONSTER_HIT, 0 , FALSE );
			return;

		case LARVAE_MONSTER:
			EVENT_InitNewSoldierAnim( pSoldier, LARVAE_HIT, 0 , FALSE );
			return;

		case QUEENMONSTER:
			EVENT_InitNewSoldierAnim( pSoldier, QUEEN_HIT, 0 , FALSE );
			return;

		case CRIPPLECIV:

		{
			// OK, do some code here to allow the fact that poor buddy can be
			// thrown back if it's a big enough hit...
			EVENT_InitNewSoldierAnim( pSoldier, CRIPPLE_HIT, 0 , FALSE );

			//pSoldier->bLife = 0;
			//EVENT_InitNewSoldierAnim( pSoldier, CRIPPLE_DIE_FLYBACK, 0 , FALSE );


		}
			return;

		case ROBOTNOWEAPON:
			EVENT_InitNewSoldierAnim( pSoldier, ROBOTNW_HIT, 0 , FALSE );
			return;


		case INFANT_MONSTER:
			EVENT_InitNewSoldierAnim( pSoldier, INFANT_HIT, 0 , FALSE );
			return;

		case CROW:

			EVENT_InitNewSoldierAnim( pSoldier, CROW_DIE, 0 , FALSE );
			return;

		//case FATCIV:
		case MANCIV:
		case MINICIV:
		case DRESSCIV:
		case HATKIDCIV:
		case KIDCIV:

			// OK, if life is 0 and not set as dead ( this is a death hit... )
			if ( !( pSoldier->uiStatusFlags & SOLDIER_DEAD ) && pSoldier->bLife == 0 )
			{
				// Randomize death!
				if ( Random( 2 ) )
				{
					EVENT_InitNewSoldierAnim( pSoldier, CIV_DIE2, 0 , FALSE );
					return;
				}
			}

			// IF here, go generic hit ALWAYS.....
			EVENT_InitNewSoldierAnim( pSoldier, GENERIC_HIT_STAND, 0 , FALSE );
			return;
	}

	// If here, we are a merc, check if we are in water
	if ( pSoldier->bOverTerrainType == LOW_WATER )
	{
			EVENT_InitNewSoldierAnim( pSoldier, WATER_HIT, 0 , FALSE );
			return;
	}
	if ( pSoldier->bOverTerrainType == DEEP_WATER )
	{
			EVENT_InitNewSoldierAnim( pSoldier, DEEP_WATER_HIT, 0 , FALSE );
			return;
	}


	// SWITCH IN TYPE OF WEAPON
	if ( GCM->getItem(usWeaponIndex)->getItemClass() & ( IC_GUN | IC_THROWING_KNIFE ) )
	{
		SoldierGotHitGunFire(pSoldier, bDirection, att, ubSpecial);
	}
	if ( GCM->getItem(usWeaponIndex)->isBlade() )
	{
		SoldierGotHitBlade(pSoldier);
	}
	if ( GCM->getItem(usWeaponIndex)->isExplosive() || GCM->getItem(usWeaponIndex)->isTentacles() )
	{
		SoldierGotHitExplosion(pSoldier, usWeaponIndex, bDirection, sRange);
	}
	if ( GCM->getItem(usWeaponIndex)->isPunch() )
	{
		SoldierGotHitPunch(pSoldier);
	}
}


static UINT8 CalcScreamVolume(SOLDIERTYPE* pSoldier, UINT8 ubCombinedLoss)
{
	// NB explosions are so loud they should drown out screams
	UINT8 ubVolume;

	if (ubCombinedLoss < 1)
	{
		ubVolume = 1;
	}
	else
	{
		ubVolume = ubCombinedLoss;
	}

	// Victim yells out in pain, making noise.  Yelps are louder from greater
	// wounds, but softer for more experienced soldiers.

	if (ubVolume > (10 - EffectiveExpLevel( pSoldier ) ))
	{
		ubVolume = 10 - EffectiveExpLevel( pSoldier );
	}

	/*
	// the "Speck factor"...  He's a whiner, and extra-sensitive to pain!
	if (ptr->trait == NERVOUS)
		ubVolume += 2;*/

	#if 0 /* XXX unsigned < 0 ? */
	if (ubVolume < 0)
	{
		ubVolume = 0;
	}
	#endif

	return( ubVolume );
}


static BOOLEAN SoldierCarriesTwoHandedWeapon(SOLDIERTYPE* pSoldier);


static void DoGenericHit(SOLDIERTYPE* pSoldier, UINT8 ubSpecial, INT16 bDirection)
{
	// Based on stance, select generic hit animation
	switch ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight )
	{
		case ANIM_STAND:
			// For now, check if we are affected by a burst
			// For now, if the weapon was a gun, special 1 == burst
			// ATE: Only do this for mercs!
			if ( ubSpecial == FIRE_WEAPON_BURST_SPECIAL && pSoldier->ubBodyType <= REGFEMALE )
			{
				//SetSoldierDesiredDirection( pSoldier, bDirection );
				EVENT_SetSoldierDirection( pSoldier, (INT8)bDirection );
				EVENT_SetSoldierDesiredDirection( pSoldier, pSoldier->bDirection );

				EVENT_InitNewSoldierAnim( pSoldier, STANDING_BURST_HIT, 0 , FALSE );
			}
			else
			{
				// Check in hand for rifle
				if ( SoldierCarriesTwoHandedWeapon( pSoldier ) )
				{
					EVENT_InitNewSoldierAnim( pSoldier, RIFLE_STAND_HIT, 0 , FALSE );
				}
				else
				{
					EVENT_InitNewSoldierAnim( pSoldier, GENERIC_HIT_STAND, 0 , FALSE );
				}
			}
			break;

		case ANIM_PRONE:

			EVENT_InitNewSoldierAnim( pSoldier, GENERIC_HIT_PRONE, 0 , FALSE );
			break;

		case ANIM_CROUCH:
			EVENT_InitNewSoldierAnim( pSoldier, GENERIC_HIT_CROUCH, 0 , FALSE );
			break;

	}
}


static void ChangeToFlybackAnimation(SOLDIERTYPE* pSoldier, INT8 bDirection);


static void SoldierGotHitGunFire(SOLDIERTYPE* const pSoldier, const UINT16 bDirection, SOLDIERTYPE* const att, const UINT8 ubSpecial)
{
	UINT16  usNewGridNo;
	BOOLEAN fBlownAway = FALSE;
	BOOLEAN fHeadHit = FALSE;
	BOOLEAN fFallenOver = FALSE;

	// MAYBE CHANGE TO SPECIAL ANIMATION BASED ON VALUE SET BY DAMAGE CALCULATION CODE
	// ALL THESE ONLY WORK ON STANDING PEOPLE
	if (!(pSoldier->uiStatusFlags & SOLDIER_MONSTER) &&
		gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_STAND)
	{
		if (gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_STAND )
		{
			if (ubSpecial == FIRE_WEAPON_HEAD_EXPLODE_SPECIAL)
			{
				if ( gGameSettings.fOptions[ TOPTION_BLOOD_N_GORE ] )
				{
					if (SpacesAway(pSoldier->sGridNo, att->sGridNo) <= MAX_DISTANCE_FOR_MESSY_DEATH)
					{
						usNewGridNo = NewGridNo( (UINT16)pSoldier->sGridNo, DirectionInc( pSoldier->bDirection ) );

						// CHECK OK DESTINATION!
						if ( OKFallDirection( pSoldier, usNewGridNo, pSoldier->bLevel, pSoldier->bDirection, JFK_HITDEATH ) )
						{
							usNewGridNo = NewGridNo( (UINT16)usNewGridNo, DirectionInc( pSoldier->bDirection ) );

							if ( OKFallDirection( pSoldier, usNewGridNo, pSoldier->bLevel, pSoldier->bDirection, pSoldier->usAnimState ) )
							{
								fHeadHit = TRUE;
							}
						}
					}
				}
			}
			else if (ubSpecial == FIRE_WEAPON_CHEST_EXPLODE_SPECIAL)
			{
				if ( gGameSettings.fOptions[ TOPTION_BLOOD_N_GORE ] )
				{
					if (SpacesAway(pSoldier->sGridNo, att->sGridNo) <= MAX_DISTANCE_FOR_MESSY_DEATH)
					{

						// possibly play torso explosion anim!
						if (pSoldier->bDirection == bDirection)
						{
							const UINT8 opp_dir = OppositeDirection(bDirection);
							usNewGridNo = NewGridNo(pSoldier->sGridNo, DirectionInc(opp_dir));

							if (OKFallDirection(pSoldier, usNewGridNo, pSoldier->bLevel, opp_dir, FLYBACK_HIT))
							{
								usNewGridNo = NewGridNo(usNewGridNo, DirectionInc(opp_dir));

								if (OKFallDirection(pSoldier, usNewGridNo, pSoldier->bLevel, opp_dir, pSoldier->usAnimState))
								{
									fBlownAway = TRUE;
								}
							}
						}
					}
				}
			}
			else if (ubSpecial == FIRE_WEAPON_LEG_FALLDOWN_SPECIAL)
			{
				// possibly play fall over anim!
				// this one is NOT restricted by distance
				if (IsValidStance( pSoldier, ANIM_PRONE ) )
				{
					// Can't be in water, or not standing
					if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_STAND && !MercInWater( pSoldier ) )
					{
						fFallenOver = TRUE;
						ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(gzLateLocalizedString[STR_LATE_20], pSoldier->name));
					}
				}
			}
		}
	}

	// IF HERE AND GUY IS DEAD, RETURN!
	if ( pSoldier->uiStatusFlags & SOLDIER_DEAD )
	{
		SLOGD("Releasesoldierattacker, Dead soldier hit");
		ReleaseSoldiersAttacker( pSoldier );
		return;
	}

	if ( fFallenOver )
	{
		SoldierCollapse( pSoldier );
		return;
	}

	if ( fBlownAway )
	{
		// Only for mercs...
		if (pSoldier->ubBodyType <= REGFEMALE)
		{
			ChangeToFlybackAnimation( pSoldier, (INT8)bDirection );
			return;
		}
	}

	if ( fHeadHit )
	{
		// Only for mercs ( or KIDS! )
		if (pSoldier->ubBodyType <= REGFEMALE || pSoldier->ubBodyType == HATKIDCIV || pSoldier->ubBodyType == KIDCIV)
		{
			EVENT_InitNewSoldierAnim( pSoldier, JFK_HITDEATH, 0 , FALSE );
			return;
		}
	}

	DoGenericHit( pSoldier, ubSpecial, bDirection );
}


static void SoldierGotHitExplosion(SOLDIERTYPE* const pSoldier, const UINT16 usWeaponIndex, const UINT16 bDirection, const UINT16 sRange)
{
	INT16 sNewGridNo;

	// IF HERE AND GUY IS DEAD, RETURN!
	if ( pSoldier->uiStatusFlags & SOLDIER_DEAD )
	{
		return;
	}

	//check for services
	ReceivingSoldierCancelServices( pSoldier );
	GivingSoldierCancelServices( pSoldier );


	if ( gGameSettings.fOptions[ TOPTION_BLOOD_N_GORE ] )
	{
		if ( GCM->getExplosive(usWeaponIndex)->getRadius() >= 3 &&
			pSoldier->bLife == 0 && gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_PRONE )
		{
			if ( sRange >= 2 && sRange <= 4 )
			{
				DoMercBattleSound(pSoldier, BATTLE_SOUND_HIT1);

				EVENT_InitNewSoldierAnim( pSoldier, CHARIOTS_OF_FIRE, 0 , FALSE );
				return;
			}
			else if ( sRange <= 1 )
			{
				DoMercBattleSound(pSoldier, BATTLE_SOUND_HIT1);

				EVENT_InitNewSoldierAnim( pSoldier, BODYEXPLODING, 0 , FALSE );
				return;
			}
		}
	}

	// If we can't fal back or such, so generic hit...
	if (pSoldier->ubBodyType > REGFEMALE)
	{
		DoGenericHit( pSoldier, 0, bDirection );
		return;
	}

	// Based on stance, select generic hit animation
	switch ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight )
	{
		case ANIM_STAND:
		case ANIM_CROUCH:

			EVENT_SetSoldierDirection( pSoldier, (INT8)bDirection );
			EVENT_SetSoldierDesiredDirection( pSoldier, pSoldier->bDirection );

			// Check behind us!
			sNewGridNo = NewGridNo(pSoldier->sGridNo, DirectionInc(OppositeDirection(bDirection)));

			if (OKFallDirection(pSoldier, sNewGridNo, pSoldier->bLevel, OppositeDirection(bDirection), FLYBACK_HIT))
			{
				ChangeToFallbackAnimation( pSoldier, (INT8)bDirection );
			}
			else
			{
				if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_STAND )
				{
					BeginTyingToFall( pSoldier );
					EVENT_InitNewSoldierAnim( pSoldier, FALLFORWARD_FROMHIT_STAND, 0, FALSE );
				}
				else
				{
					SoldierCollapse( pSoldier );
				}
			}
			break;

		case ANIM_PRONE:

			SoldierCollapse( pSoldier );
			break;
	}
}


static void SoldierGotHitBlade(SOLDIERTYPE* const pSoldier)
{
	// IF HERE AND GUY IS DEAD, RETURN!
	if ( pSoldier->uiStatusFlags & SOLDIER_DEAD )
	{
		return;
	}


	// Based on stance, select generic hit animation
	switch ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight )
	{
		case ANIM_STAND:

			// Check in hand for rifle
			if ( SoldierCarriesTwoHandedWeapon( pSoldier ) )
			{
				EVENT_InitNewSoldierAnim( pSoldier, RIFLE_STAND_HIT, 0 , FALSE );
			}
			else
			{
				EVENT_InitNewSoldierAnim( pSoldier, GENERIC_HIT_STAND, 0 , FALSE );
			}
			break;

		case ANIM_CROUCH:
			EVENT_InitNewSoldierAnim( pSoldier, GENERIC_HIT_CROUCH, 0 , FALSE );
			break;

		case ANIM_PRONE:
			EVENT_InitNewSoldierAnim( pSoldier, GENERIC_HIT_PRONE, 0 , FALSE );
			break;
	}
}


static void SoldierGotHitPunch(SOLDIERTYPE* const pSoldier)
{

	// IF HERE AND GUY IS DEAD, RETURN!
	if ( pSoldier->uiStatusFlags & SOLDIER_DEAD )
	{
		return;
	}

	// Based on stance, select generic hit animation
	switch ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight )
	{
		case ANIM_STAND:
			// Check in hand for rifle
			if ( SoldierCarriesTwoHandedWeapon( pSoldier ) )
			{
				EVENT_InitNewSoldierAnim( pSoldier, RIFLE_STAND_HIT, 0 , FALSE );
			}
			else
			{
				EVENT_InitNewSoldierAnim( pSoldier, GENERIC_HIT_STAND, 0 , FALSE );
			}
			break;

		case ANIM_CROUCH:
			EVENT_InitNewSoldierAnim( pSoldier, GENERIC_HIT_CROUCH, 0 , FALSE );
			break;

		case ANIM_PRONE:
			EVENT_InitNewSoldierAnim( pSoldier, GENERIC_HIT_PRONE, 0 , FALSE );
			break;

	}

}


BOOLEAN EVENT_InternalGetNewSoldierPath( SOLDIERTYPE *pSoldier, UINT16 sDestGridNo, UINT16 usMovementAnim, BOOLEAN fFromUI, BOOLEAN fForceRestartAnim )
{
	INT32 iDest;
	BOOLEAN fContinue;
	UINT32 uiDist;
	UINT16 usAnimState;
	UINT16 usMoveAnimState = usMovementAnim;
	INT16 sMercGridNo;
	UINT8 ubPathingData[MAX_PATH_LIST_SIZE];
	//UINT8 ubPathingMaxDirection;
	BOOLEAN fAdvancePath = TRUE;
	UINT8 fFlags = 0;

	// Ifd this code, make true if a player
	if ( fFromUI == 3 )
	{
		if ( pSoldier->bTeam == OUR_TEAM )
		{
			fFromUI = 1;
		}
		else
		{
			fFromUI = 0;
		}
	}

	// ATE: if a civ, and from UI, and were cowering, remove from cowering
	if ( AM_AN_EPC( pSoldier ) && fFromUI )
	{
		if ( pSoldier->uiStatusFlags & SOLDIER_COWERING )
		{
			SetSoldierCowerState( pSoldier, FALSE );
			usMoveAnimState = WALKING;
		}
	}


	pSoldier->bGoodContPath = FALSE;

	if ( pSoldier->fDelayedMovement )
	{
		if ( pSoldier->ubDelayedMovementFlags & DELAYED_MOVEMENT_FLAG_PATH_THROUGH_PEOPLE )
		{
			fFlags = PATH_THROUGH_PEOPLE;
		}
		else
		{
			fFlags = PATH_IGNORE_PERSON_AT_DEST;
		}
		pSoldier->fDelayedMovement = FALSE;
	}

	if ( gfGetNewPathThroughPeople )
	{
		fFlags = PATH_THROUGH_PEOPLE;
	}

	// ATE: Some stuff here for realtime, going through interface....
	if ((!( gTacticalStatus.uiFlags & INCOMBAT) &&
		( gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_MOVING) && fFromUI == 1)
		|| fFromUI == 2)
	{
		if ( pSoldier->bCollapsed )
		{
			return( FALSE );
		}

		sMercGridNo = pSoldier->sGridNo;
		pSoldier->sGridNo = pSoldier->sDestination;

		// Check if path is good before copying it into guy's path...
		if ( FindBestPath( pSoldier, sDestGridNo, pSoldier->bLevel, pSoldier->usUIMovementMode, NO_COPYROUTE, fFlags ) == 0 )
		{
			// Set to old....
			pSoldier->sGridNo = sMercGridNo;

			return( FALSE );
		}

		uiDist =  FindBestPath( pSoldier, sDestGridNo, pSoldier->bLevel, pSoldier->usUIMovementMode, COPYROUTE, fFlags );

		pSoldier->sGridNo = sMercGridNo;
		pSoldier->sFinalDestination = sDestGridNo;

		if ( uiDist > 0 )
		{
			// Add one to path data size....
			if ( fAdvancePath )
			{
				memcpy( ubPathingData, pSoldier->ubPathingData, sizeof( ubPathingData ) );
				//ubPathingMaxDirection = (UINT8)ubPathingData[ MAX_PATH_LIST_SIZE -1 ];
				memcpy( &(pSoldier->ubPathingData[1]), ubPathingData, sizeof( ubPathingData ) - sizeof( ubPathingData[0] ) );

				// If we have reach the max, go back one sFinalDest....
				if ( pSoldier->ubPathDataSize == MAX_PATH_LIST_SIZE )
				{
					//pSoldier->sFinalDestination = NewGridNo(pSoldier->sFinalDestination, DirectionInc(OppositeDirection(ubPathingMaxDirection)));
				}
				else
				{
					pSoldier->ubPathDataSize++;
				}
			}

			usMoveAnimState = pSoldier->usUIMovementMode;

			if ( pSoldier->bOverTerrainType == DEEP_WATER )
			{
				usMoveAnimState = DEEP_WATER_SWIM;
			}

			// Change animation only.... set value to NOT call any goto new gridno stuff.....
			if ( usMoveAnimState != pSoldier->usAnimState )
			{
				//
				pSoldier->usDontUpdateNewGridNoOnMoveAnimChange = TRUE;

				EVENT_InitNewSoldierAnim( pSoldier, usMoveAnimState, 0, FALSE );
			}

			return( TRUE );
		}

		return( FALSE );
	}

	// we can use the soldier's level here because we don't have pathing across levels right now...
	if (pSoldier->bPathStored)
	{
		fContinue = TRUE;
	}
	else
	{
		iDest = FindBestPath( pSoldier, sDestGridNo, pSoldier->bLevel, usMovementAnim, COPYROUTE, fFlags );
		fContinue = (iDest != 0);
	}

	// Only if we can get a path here
	if ( fContinue )
	{
		// Debug messages
		SLOGD("Soldier {}: Get new path", pSoldier->ubID);

		// Set final destination
		pSoldier->sFinalDestination = sDestGridNo;
		pSoldier->fPastXDest = 0;
		pSoldier->fPastYDest = 0;

		// If true, we're OK, if not, WAIT for a guy to pass!
		// If we are in deep water, we can only swim!
		if ( pSoldier->bOverTerrainType == DEEP_WATER )
		{
			usMoveAnimState = DEEP_WATER_SWIM;
		}

		// If we were aiming, end aim!
		usAnimState = PickSoldierReadyAnimation( pSoldier, TRUE );

		// Add a pending animation first!
		// Only if we were standing!
		if ( usAnimState != INVALID_ANIMATION && gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_STAND )
		{
			EVENT_InitNewSoldierAnim( pSoldier, usAnimState, 0, FALSE );
			pSoldier->usPendingAnimation = usMoveAnimState;
		}
		else
		{
			// Call local copy for change soldier state!
			EVENT_InitNewSoldierAnim( pSoldier, usMoveAnimState, 0, fForceRestartAnim );

		}

		// Change desired direction
		// ATE: Here we have a situation where in RT, we may have
		// gotten a new path, but we are alreayd moving.. so
		// at leasty change new dest. This will be redundent if the ANI is a totaly new one

		return( TRUE );
	}

	return( FALSE );
}

void EVENT_GetNewSoldierPath( SOLDIERTYPE *pSoldier, UINT16 sDestGridNo, UINT16 usMovementAnim )
{
	// ATE: Default restart of animation to TRUE
	EVENT_InternalGetNewSoldierPath( pSoldier, sDestGridNo, usMovementAnim, FALSE, TRUE );
}

// Change our state based on stance, to stop!
void StopSoldier( SOLDIERTYPE *pSoldier )
{
	ReceivingSoldierCancelServices( pSoldier );
	GivingSoldierCancelServices( pSoldier );

	if ( !( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_STATIONARY ) )
	{
		//SoldierGotoStationaryStance( pSoldier );
		EVENT_StopMerc(pSoldier);
	}

	// Set desination
	pSoldier->sFinalDestination = pSoldier->sGridNo;

}

void SoldierGotoStationaryStance( SOLDIERTYPE *pSoldier )
{
	// ATE: This is to turn off fast movement, that us used to change movement mode
	// for ui display on stance changes....
	if ( pSoldier->bTeam == OUR_TEAM )
	{
		//pSoldier->fUIMovementFast = FALSE;
	}

	// The queen, if she sees anybody, goes to ready, not normal breath....
	if ( pSoldier->ubBodyType == QUEENMONSTER )
	{
		if ( pSoldier->bOppCnt > 0 || pSoldier->bTeam == OUR_TEAM )
		{
			EVENT_InitNewSoldierAnim( pSoldier, QUEEN_READY, 0 , TRUE );
			return;
		}
	}

	// Check if we are in deep water!
	if ( pSoldier->bOverTerrainType == DEEP_WATER )
	{
		// IN deep water, tred!
		EVENT_InitNewSoldierAnim( pSoldier, DEEP_WATER_TRED, 0 , FALSE );
	}
	else if (pSoldier->service_partner != NULL && pSoldier->bLife >= OKLIFE && pSoldier->bBreath > 0)
	{
		EVENT_InitNewSoldierAnim( pSoldier, GIVING_AID, 0 , FALSE );
	}
	else
	{
		// Change state back to stationary state for given height
		switch( gAnimControl[ pSoldier->usAnimState ].ubEndHeight )
		{
			case ANIM_STAND:

				// If we are cowering....goto cower state
				if ( pSoldier->uiStatusFlags & SOLDIER_COWERING )
				{
					EVENT_InitNewSoldierAnim( pSoldier, START_COWER, 0 , FALSE );
				}
				else
				{
					EVENT_InitNewSoldierAnim( pSoldier, STANDING, 0 , FALSE );
				}
				break;

			case ANIM_CROUCH:

				// If we are cowering....goto cower state
				if ( pSoldier->uiStatusFlags & SOLDIER_COWERING )
				{
					EVENT_InitNewSoldierAnim( pSoldier, COWERING, 0 , FALSE );
				}
				else
				{
					EVENT_InitNewSoldierAnim( pSoldier, CROUCHING, 0 , FALSE );
				}
				break;

			case ANIM_PRONE:
				EVENT_InitNewSoldierAnim( pSoldier, PRONE, 0 , FALSE );
				break;
		}

	}

}


static UINT16 GetNewSoldierStateFromNewStance(SOLDIERTYPE* pSoldier, UINT8 ubDesiredStance);


void ChangeSoldierStance( SOLDIERTYPE *pSoldier, UINT8 ubDesiredStance )
{
	UINT16 usNewState;

	// Check if they are the same!
	if ( ubDesiredStance == gAnimControl[ pSoldier->usAnimState ].ubEndHeight )
	{
		// Free up from stance change
		FreeUpNPCFromStanceChange( pSoldier );
		return;
	}

	// Set UI Busy
	SetUIBusy(pSoldier);

	// ATE: If we are an NPC, cower....
	if ( pSoldier->ubBodyType >= FATCIV && pSoldier->ubBodyType <= KIDCIV )
	{
		if ( ubDesiredStance == ANIM_STAND )
		{
			SetSoldierCowerState( pSoldier, FALSE );
		}
		else
		{
			SetSoldierCowerState( pSoldier, TRUE );
		}
	}
	else
	{
		usNewState = GetNewSoldierStateFromNewStance( pSoldier, ubDesiredStance );

		// Set desired stance
		pSoldier->ubDesiredHeight = ubDesiredStance;

		// Now change to appropriate animation
		EVENT_InitNewSoldierAnim( pSoldier, usNewState, 0 , FALSE );
	}
}

void EVENT_InternalSetSoldierDestination( SOLDIERTYPE *pSoldier, UINT16	usNewDirection, BOOLEAN fFromMove, UINT16 usAnimState )
{
	UINT16 usNewGridNo;
	INT16  sXPos, sYPos;

	// Get dest gridno, convert to center coords
	usNewGridNo = NewGridNo( (UINT16)pSoldier->sGridNo, DirectionInc( usNewDirection ) );

	ConvertGridNoToCenterCellXY(usNewGridNo, &sXPos, &sYPos);

	// Save new dest gridno, x, y
	pSoldier->sDestination = usNewGridNo;
	pSoldier->sDestXPos = sXPos;
	pSoldier->sDestYPos = sYPos;

	pSoldier->bMovementDirection = (INT8)usNewDirection;


	// OK, ATE: If we are side_stepping, calculate a NEW desired direction....
	if ( pSoldier->bReverse && usAnimState == SIDE_STEP )
	{
		UINT8 ubPerpDirection;

		// Get a new desired direction,
		ubPerpDirection = gPurpendicularDirection[ pSoldier->bDirection ][ usNewDirection ];

		// CHange actual and desired direction....
		EVENT_SetSoldierDirection( pSoldier, ubPerpDirection );
		pSoldier->bDesiredDirection = pSoldier->bDirection;
	}
	else
	{
		if ( !( gAnimControl[ usAnimState ].uiFlags & ANIM_SPECIALMOVE ) )
		{
			EVENT_InternalSetSoldierDesiredDirection( pSoldier, usNewDirection, fFromMove, usAnimState );
		}
	}
}


// function to determine which direction a creature can turn in
static INT8 MultiTiledTurnDirection(SOLDIERTYPE* pSoldier, INT8 bStartDirection, INT8 bDesiredDirection)
{
	INT8 bTurningIncrement;
	INT8 bCurrentDirection;
	INT8 bLoop;
	BOOLEAN fOk = FALSE;

	// start by trying to turn in quickest direction
	bTurningIncrement = QuickestDirection( bStartDirection, bDesiredDirection );

	UINT16 const usAnimSurface = DetermineSoldierAnimationSurface(pSoldier, pSoldier->usUIMovementMode);

	const STRUCTURE_FILE_REF* const pStructureFileRef = GetAnimationStructureRef(pSoldier, usAnimSurface, pSoldier->usUIMovementMode);
	if ( !pStructureFileRef )
	{
		// without structure data, well, assume quickest direction
		return( bTurningIncrement );
	}

	// ATE: Only if we have a levelnode...
	UINT16 const usStructureID = GetStructureID(pSoldier);

	bLoop = 0;
	bCurrentDirection = bStartDirection;

	while( bLoop < 2 )
	{
		while( bCurrentDirection != bDesiredDirection )
		{
			bCurrentDirection += bTurningIncrement;

			// did we wrap directions?
			if ( bCurrentDirection < 0 )
			{
				bCurrentDirection = (MAXDIR - 1);
			}
			else if ( bCurrentDirection >= MAXDIR )
			{
				bCurrentDirection = 0;
			}

			// check to see if we can add creature in that direction
			fOk = OkayToAddStructureToWorld(pSoldier->sGridNo, pSoldier->bLevel, &pStructureFileRef->pDBStructureRef[OneCDirection(bCurrentDirection)], usStructureID);
			if (!fOk)
			{
				break;
			}
		}

		if ( (bCurrentDirection == bDesiredDirection) && fOk )
		{
			// success!!
			return( bTurningIncrement );
		}

		bLoop++;
		if ( bLoop < 2 )
		{
			// change direction of loop etc
			bCurrentDirection = bStartDirection;
			bTurningIncrement *= -1;
		}
	}
	// nothing found... doesn't matter much what we return
	return( bTurningIncrement );
}


static void EVENT_InternalSetSoldierDesiredDirection(SOLDIERTYPE* const pSoldier, UINT16 usNewDirection, const BOOLEAN fInitalMove, const UINT16 usAnimState)
{
	//if ( usAnimState == WALK_BACKWARDS )
	if (pSoldier->bReverse && usAnimState != SIDE_STEP) // XXX TODO0014
	{
		// OK, check if we are going to go in the exact opposite than our facing....
		usNewDirection = OppositeDirection(usNewDirection);
	}


	pSoldier->bDesiredDirection = (INT8)usNewDirection;

	// If we are prone, goto crouched first!
	// ONly if we are stationary, and only if directions are differnet!

	// ATE: If we are fNoAPsToFinnishMove, stop what we were doing and
	// reset flag.....
	if ( pSoldier->fNoAPToFinishMove && ( gAnimControl[ usAnimState ].uiFlags & ANIM_MOVING ) )
	{
		// ATE; Commented this out: NEVER, EVER, start a new anim from this function, as
		// an eternal loop will result....
		//SoldierGotoStationaryStance( pSoldier );
		// Reset flag!
		AdjustNoAPToFinishMove( pSoldier, FALSE );
	}

	if ( pSoldier->bDesiredDirection != pSoldier->bDirection )
	{
		if (gAnimControl[usAnimState].uiFlags & (ANIM_BREATH | ANIM_OK_CHARGE_AP_FOR_TURN | ANIM_FIREREADY) &&
			!fInitalMove && !pSoldier->fDontChargeTurningAPs)
		{
			// Deduct points for initial turn!
			switch( gAnimControl[ usAnimState ].ubEndHeight )
			{
				// Now change to appropriate animation
				case ANIM_STAND:
					DeductPoints( pSoldier, AP_LOOK_STANDING, 0 );
					break;

				case ANIM_CROUCH:
					DeductPoints( pSoldier, AP_LOOK_CROUCHED, 0 );
					break;

				case ANIM_PRONE:
					DeductPoints( pSoldier, AP_LOOK_PRONE, 0 );
					break;
			}

		}

		pSoldier->fDontChargeTurningAPs = FALSE;

		if ( fInitalMove )
		{
			if ( gAnimControl[ usAnimState ].ubHeight == ANIM_PRONE  )
			{
				if ( pSoldier->fTurningFromPronePosition != TURNING_FROM_PRONE_ENDING_UP_FROM_MOVE )
				{
					pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_START_UP_FROM_MOVE;
				}
			}
		}

		if ( gAnimControl[ usAnimState ].uiFlags & ANIM_STATIONARY || pSoldier->fNoAPToFinishMove || fInitalMove )
		{
			if ( gAnimControl[ usAnimState ].ubHeight == ANIM_PRONE )
			{
				// Set this beasty of a flag to allow us to go back down to prone if we choose!
				// ATE: Alrighty, set flag to go back down only if we are not moving anywhere
				//if ( pSoldier->sDestination == pSoldier->sGridNo )
				if ( !fInitalMove )
				{
					pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_ON;

					// Set a pending animation to change stance first...
					ChangeSoldierStance(pSoldier, ANIM_CROUCH);
				}
			}
		}
	}

	if ( pSoldier->bDesiredDirection != pSoldier->bDirection )
	{
		if ( pSoldier->uiStatusFlags & ( SOLDIER_VEHICLE ) || CREATURE_OR_BLOODCAT( pSoldier ) )
		{
			pSoldier->uiStatusFlags |= SOLDIER_PAUSEANIMOVE;
		}
	}


	if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		const UINT8 hires_desired_dir = Dir2ExtDir(pSoldier->bDesiredDirection);
		pSoldier->bTurningIncrement = QuickestDirection(pSoldier->ubHiResDirection, hires_desired_dir, 16);
	}
	else
	{
		if ( pSoldier->uiStatusFlags & SOLDIER_MULTITILE )
		{
			pSoldier->bTurningIncrement = (INT8) MultiTiledTurnDirection( pSoldier, pSoldier->bDirection, pSoldier->bDesiredDirection );
		}
		else
		{
			pSoldier->bTurningIncrement = QuickestDirection( pSoldier->bDirection, pSoldier->bDesiredDirection );
		}
	}

}


void EVENT_SetSoldierDesiredDirection( SOLDIERTYPE *pSoldier, UINT16	usNewDirection )
{
	EVENT_InternalSetSoldierDesiredDirection( pSoldier, usNewDirection, FALSE, pSoldier->usAnimState );
}


void EVENT_SetSoldierDesiredDirectionForward(SOLDIERTYPE* const s, const UINT16 new_direction)
{
	s->bReverse = FALSE; // XXX TODO0014
	EVENT_SetSoldierDesiredDirection(s, new_direction);
}


static void AdjustForFastTurnAnimation(SOLDIERTYPE* pSoldier);


void EVENT_SetSoldierDirection( SOLDIERTYPE *pSoldier, UINT16	usNewDirection )
{
	// Remove old location data
	HandleAnimationProfile(*pSoldier, pSoldier->usAnimState, TRUE);

	pSoldier->bDirection = (INT8)usNewDirection;

	// Updated extended direction.....
	pSoldier->ubHiResDirection = Dir2ExtDir(pSoldier->bDirection);

	// Add new stuff
	HandleAnimationProfile(*pSoldier, pSoldier->usAnimState, FALSE);

	// If we are turning, we have chaanged our aim!
	if ( !pSoldier->fDontUnsetLastTargetFromTurn )
	{
		pSoldier->sLastTarget = NOWHERE;
	}

	AdjustForFastTurnAnimation( pSoldier );

	// Update structure info!
	//if ( pSoldier->uiStatusFlags & SOLDIER_MULTITILE )
	{
		UpdateMercStructureInfo( pSoldier );
	}

	// Handle Profile data for hit locations
	HandleAnimationProfile(*pSoldier, pSoldier->usAnimState, TRUE);

	HandleCrowShadowNewDirection( pSoldier );

	// Change values!
	SetSoldierLocatorOffsets( pSoldier );

}


static INT32 CheckBleeding(SOLDIERTYPE* pSoldier);


void EVENT_BeginMercTurn(SOLDIERTYPE& s)
{
	// UnderFire now starts at 2 for "under fire this turn", down to 1 for "under
	// fire last turn", to 0
	if (s.bUnderFire != 0) --s.bUnderFire;

	// ATE: Add decay effect for drugs
	HandleEndTurnDrugAdjustments(&s);

	// ATE: Don't bleed if in auto bandage!
	if (!gTacticalStatus.fAutoBandageMode)
	{
		// Blood is not for the weak of heart, or mechanical
		if (!(s.uiStatusFlags & (SOLDIER_VEHICLE | SOLDIER_ROBOT)))
		{
			if (s.bBleeding != 0 || s.bLife < OKLIFE) // is he bleeding or dying?
			{
				INT32 const blood = CheckBleeding(&s); // check if he might lose another life point
				// ATE: Only if in sector
				if (blood != NOBLOOD && s.bInSector)
				{
					DropBlood(s, blood);
				}
			}
		}
	}

	if (s.bLife == 0) return;
	// He is still alive (didn't bleed to death)

	// Reduce the effects of any residual shock from past injuries by half
	s.bShock /= 2;

	// If this person has heard a noise that hasn't been investigated
	if (s.sNoiseGridno != NOWHERE && s.ubNoiseVolume != 0)
	{
		// The volume of the noise "decays" by 1 point
		if (--s.ubNoiseVolume == 0)
		{
			// The volume has reached zero, forget about the noise
			s.sNoiseGridno = NOWHERE;
		}
	}

	if (s.uiStatusFlags & SOLDIER_GASSED)
	{
		// Must get a gas mask or leave the gassed area to get over it
		if (IsWearingHeadGear(s, GASMASK) ||
			GetSmokeEffectOnTile(s.sGridNo, s.bLevel) == SmokeEffectID::NOTHING)
		{
			// Turn off gassed flag
			s.uiStatusFlags &= ~SOLDIER_GASSED;
		}
	}

	if (s.bBlindedCounter > 0 && --s.bBlindedCounter == 0)
	{
		HandleSight(s, SIGHT_LOOK);         // We can see
		fInterfacePanelDirty = DIRTYLEVEL2; // Dirty panel
	}

	s.sWeightCarriedAtTurnStart = CalculateCarriedWeight(&s);

	UnusedAPsToBreath(&s);

	// Set flag back to normal, after reaching a certain statge
	if (s.bBreath > 80) s.usQuoteSaidFlags &= ~SOLDIER_QUOTE_SAID_LOW_BREATH;
	if (s.bBreath > 50) s.usQuoteSaidFlags &= ~SOLDIER_QUOTE_SAID_DROWNING;

	if (s.ubTurnsUntilCanSayHeardNoise > 0) --s.ubTurnsUntilCanSayHeardNoise;

	if (s.bInSector) CheckForBreathCollapse(s);

	CalcNewActionPoints(&s);

	s.bTilesMoved = 0;

	if (s.bInSector)
	{
		BeginSoldierGetup(&s);

		// CJC Nov 30: handle RT opplist decaying in another function which operates less often
		if (gTacticalStatus.uiFlags & INCOMBAT)
		{
			VerifyAndDecayOpplist(&s);
			if (s.uiXRayActivatedTime != 0) TurnOffXRayEffects(&s);
		}

		if (s.bTeam == OUR_TEAM && s.ubProfile != NO_PROFILE)
		{
			switch (GetProfile(s.ubProfile).bPersonalityTrait)
			{
				case FEAR_OF_INSECTS:
					if (MercSeesCreature(s))
					{
						HandleMoraleEvent(&s, MORALE_INSECT_PHOBIC_SEES_CREATURE, s.sSector);
						goto say_personality_quote;
					}
					break;

				case CLAUSTROPHOBIC:
					if (gWorldSector.z > 0 && Random(6 - gWorldSector.z) == 0)
					{
						HandleMoraleEvent(&s, MORALE_CLAUSTROPHOBE_UNDERGROUND, s.sSector);
						goto say_personality_quote;
					}
					break;

				case NERVOUS:
					if (DistanceToClosestFriend(&s) > NERVOUS_RADIUS)
					{
						if (s.bMorale < 50)
						{
							HandleMoraleEvent(&s, MORALE_NERVOUS_ALONE, s.sSector);
							goto say_personality_quote;
						}
					}
					else
					{
						if (s.bMorale > 45)
						{ // Turn flag off, so that we say it every two turns
							s.usQuoteSaidFlags &= ~SOLDIER_QUOTE_SAID_PERSONALITY;
						}
					}
					break;

say_personality_quote:
					if (!(s.usQuoteSaidFlags & SOLDIER_QUOTE_SAID_PERSONALITY))
					{
						TacticalCharacterDialogue(&s, QUOTE_PERSONALITY_TRAIT);
						s.usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_PERSONALITY;
					}
					break;
			}
		}
	}

	// Reset quote flags for under heavy fire and close call
	s.usQuoteSaidFlags    &= ~SOLDIER_QUOTE_SAID_BEING_PUMMELED;
	s.usQuoteSaidExtFlags &= ~SOLDIER_QUOTE_SAID_EXT_CLOSE_CALL;
	s.bNumHitsThisTurn     = 0;
	s.ubSuppressionPoints  = 0;
	s.fCloseCall           = FALSE;
	s.ubMovementNoiseHeard = 0;

	// If soldier has new APs, reset flags
	if (s.bActionPoints > 0)
	{
		s.bMoved               = FALSE;
		s.bPassedLastInterrupt = FALSE;
	}
}


BOOLEAN ConvertAniCodeToAniFrame(SOLDIERTYPE* const s, UINT16 ani_frame)
{
	static UINT8 const gDirectionFrom8to2[] = { 0, 0, 1, 1, 0, 1, 1, 0 };

	// Given ani code, adjust for facing direction

	// get anim surface and determine # of frames
	UINT16 const anim_surface = GetSoldierAnimationSurface(s);
	CHECKF(anim_surface != INVALID_ANIMATION_SURFACE);
	AnimationSurfaceType const& as = gAnimSurfaceDatabase[anim_surface];

	// Convert world direction into sprite direction
	UINT8 temp_dir = OneCDirection(s->bDirection);

	// Check # of directions/surface, adjust if ness.
	switch (as.uiNumDirections)
	{
		case  1: temp_dir  = 0;                                     break;
		case  4: temp_dir /= 2;                                     break;
		case 32: temp_dir  = ExtOneCDirection(s->ubHiResDirection); break;

		case  2:
			temp_dir = gDirectionFrom8to2[s->bDirection];
			break;

		case  3:
			switch (s->bDirection)
			{
				case NORTHWEST: temp_dir = 1; break;
				case WEST:      temp_dir = 0; break;
				case EAST:      temp_dir = 2; break;
			}
			break;
	}

	// If we are only one frame, ignore what the script is telling us!
	if (as.ubFlags & ANIM_DATA_FLAG_NOFRAMES) ani_frame = 0;

	if (!as.hVideoObject)
	{
		ani_frame = 0;
	}
	else
	{
		ani_frame += as.uiNumFramesPerDir * temp_dir;
		if (ani_frame >= as.hVideoObject->SubregionCount())
		{
			// Debug msg here....
			SLOGW("Wrong Number of frames per number of objects: {} vs {}, {}",
				as.uiNumFramesPerDir, as.hVideoObject->SubregionCount(),
				gAnimControl[s->usAnimState].zAnimStr);
			ani_frame = 0;
		}
	}

	s->usAniFrame = ani_frame;
	return TRUE;
}


void TurnSoldier( SOLDIERTYPE *pSoldier)
{
	INT16 sDirection;

	// If we are a vehicle... DON'T TURN!
	if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		if ( pSoldier->ubBodyType != TANK_NW && pSoldier->ubBodyType != TANK_NE )
		{
			return;
		}
	}

	// We handle sight now....
	if ( pSoldier->uiStatusFlags & SOLDIER_LOOK_NEXT_TURNSOLDIER )
	{
		if ((gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_STATIONARY &&
			pSoldier->usAnimState != CLIMBUPROOF && pSoldier->usAnimState != CLIMBDOWNROOF))
		{
			// HANDLE SIGHT!
			HandleSight(*pSoldier, SIGHT_LOOK | SIGHT_RADIO);
		}
		// Turn off!
		pSoldier->uiStatusFlags &= (~SOLDIER_LOOK_NEXT_TURNSOLDIER );

		HandleSystemNewAISituation(pSoldier);
	}


	if ( pSoldier->fTurningToShoot )
	{
		if ( pSoldier->bDirection == pSoldier->bDesiredDirection )
		{
			if (((gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_FIREREADY ) &&
				!pSoldier->fTurningFromPronePosition ) || pSoldier->ubBodyType == ROBOTNOWEAPON ||
				pSoldier->ubBodyType == TANK_NW || pSoldier->ubBodyType == TANK_NE)
			{
				EVENT_InitNewSoldierAnim( pSoldier, SelectFireAnimation( pSoldier, gAnimControl[ pSoldier->usAnimState ].ubEndHeight ), 0, FALSE );
				pSoldier->fTurningToShoot = FALSE;

				// Save last target gridno!
				//pSoldier->sLastTarget = pSoldier->sTargetGridNo;

			}
			// Else check if we are trying to shoot and once was prone, but am now crouched
			// because we needed to turn...
			else if ( pSoldier->fTurningFromPronePosition )
			{
				if ( IsValidStance( pSoldier, ANIM_PRONE ) )
				{
					ChangeSoldierStance(pSoldier, ANIM_PRONE);
					pSoldier->usPendingAnimation = SelectFireAnimation( pSoldier, ANIM_PRONE );
				}
				else
				{
					EVENT_InitNewSoldierAnim( pSoldier, SelectFireAnimation( pSoldier, ANIM_CROUCH ), 0, FALSE );
				}
				pSoldier->fTurningToShoot = FALSE;
				pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_OFF;
			}
		}
	}

	if ( pSoldier->fTurningUntilDone && ( pSoldier->ubPendingStanceChange != NO_PENDING_STANCE ) )
	{
		if ( pSoldier->bDirection == pSoldier->bDesiredDirection )
		{
			ChangeSoldierStance(pSoldier, pSoldier->ubPendingStanceChange);
			pSoldier->ubPendingStanceChange = NO_PENDING_STANCE;
			pSoldier->fTurningUntilDone = FALSE;
		}
	}

	if ( pSoldier->fTurningUntilDone && ( pSoldier->usPendingAnimation != NO_PENDING_ANIMATION ) )
	{
		if ( pSoldier->bDirection == pSoldier->bDesiredDirection )
		{
			UINT16 usPendingAnimation;

			usPendingAnimation = pSoldier->usPendingAnimation;
			pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;

			EVENT_InitNewSoldierAnim( pSoldier, usPendingAnimation, 0 , FALSE );
			pSoldier->fTurningUntilDone = FALSE;
		}
	}

	// Don't do anything if we are at dest direction!
	if ( pSoldier->bDirection == pSoldier->bDesiredDirection )
	{
		if ( pSoldier->ubBodyType == TANK_NW || pSoldier->ubBodyType == TANK_NE )
		{
			if ( pSoldier->uiTuringSoundID != NO_SAMPLE )
			{
				SoundStop( pSoldier->uiTuringSoundID );
				pSoldier->uiTuringSoundID = NO_SAMPLE;

				PlaySoldierJA2Sample(pSoldier, TURRET_STOP, HIGHVOLUME, 1, TRUE);
			}
		}

		// Turn off!
		pSoldier->uiStatusFlags &= (~SOLDIER_LOOK_NEXT_TURNSOLDIER );
		pSoldier->fDontUnsetLastTargetFromTurn = FALSE;

		// Unset ui busy if from ui
		if ( pSoldier->bTurningFromUI && ( pSoldier->fTurningFromPronePosition != 3 ) && ( pSoldier->fTurningFromPronePosition != 1 ) )
		{
			UnSetUIBusy(pSoldier);
			pSoldier->bTurningFromUI = FALSE;
		}

		if ( pSoldier->uiStatusFlags & ( SOLDIER_VEHICLE ) || CREATURE_OR_BLOODCAT( pSoldier ) )
		{
			pSoldier->uiStatusFlags &= (~SOLDIER_PAUSEANIMOVE);
		}

		FreeUpNPCFromTurning(pSoldier);

		// Undo our flag for prone turning...
		// Else check if we are trying to shoot and once was prone, but am now crouched
		// because we needed to turn...
		if ( pSoldier->fTurningFromPronePosition == TURNING_FROM_PRONE_ON )
		{
			// ATE: Don't do this if we have something in our hands we are going to throw!
			if ( IsValidStance( pSoldier, ANIM_PRONE ) && pSoldier->pTempObject == NULL )
			{
				ChangeSoldierStance(pSoldier, ANIM_PRONE);
			}
			pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_OFF;
		}

		// If a special code, make guy crawl after stance change!
		if ( pSoldier->fTurningFromPronePosition == TURNING_FROM_PRONE_ENDING_UP_FROM_MOVE &&
			pSoldier->usAnimState != PRONE_UP && pSoldier->usAnimState != PRONE_DOWN )
		{
			if ( IsValidStance( pSoldier, ANIM_PRONE ) )
			{
				EVENT_InitNewSoldierAnim( pSoldier, CRAWLING, 0, FALSE );
			}
		}

		if ( pSoldier->uiStatusFlags & SOLDIER_TURNINGFROMHIT )
		{
			if ( pSoldier->fGettingHit == 1 )
			{
				if (pSoldier->usPendingAnimation != FALLFORWARD_ROOF &&
					pSoldier->usPendingAnimation != FALLOFF &&
					pSoldier->usAnimState != FALLFORWARD_ROOF &&
					pSoldier->usAnimState != FALLOFF)
				{
					// Go back to original direction
					EVENT_SetSoldierDesiredDirection( pSoldier, (INT8)pSoldier->uiPendingActionData1 );

					//SETUP GETTING HIT FLAG TO 2
					pSoldier->fGettingHit = 2;
				}
				else
				{
					pSoldier->uiStatusFlags &= (~SOLDIER_TURNINGFROMHIT );
				}
			}
			else if ( pSoldier->fGettingHit == 2 )
			{
				// Turn off
				pSoldier->uiStatusFlags &= (~SOLDIER_TURNINGFROMHIT );

				// Release attacker
				SLOGD("Releasesoldierattacker, turning from hit animation ended");
				ReleaseSoldiersAttacker( pSoldier );

				//FREEUP GETTING HIT FLAG
				pSoldier->fGettingHit = FALSE;
			}
		}

		return;
	}

	// IF WE ARE HERE, WE ARE IN THE PROCESS OF TURNING

	// DOUBLE CHECK TO UNSET fNOAPs...
	if ( pSoldier->fNoAPToFinishMove )
	{
		AdjustNoAPToFinishMove( pSoldier, FALSE );
	}

	// Do something different for vehicles....
	if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		// Get new direction
		sDirection = pSoldier->ubHiResDirection + pSoldier->bTurningIncrement;
		if (sDirection > 31)
		{
			sDirection = 0;
		}
		else
		{
			if ( sDirection < 0 )
			{
				sDirection = 31;
			}
		}
		pSoldier->ubHiResDirection = (UINT8)sDirection;

		if ( pSoldier->ubBodyType == TANK_NW || pSoldier->ubBodyType == TANK_NE )
		{
			if ( pSoldier->uiTuringSoundID == NO_SAMPLE )
			{
			pSoldier->uiTuringSoundID = PlaySoldierJA2Sample(pSoldier, TURRET_MOVE, HIGHVOLUME, 100, TRUE);
			}
		}

		if (sDirection % 4 != 0)
		{
			// We are not at the multiple of a 'cardinal' direction
			return;
		}

		sDirection /= 4;
	}
	else
	{
		// Get new direction
		//sDirection = pSoldier->bDirection + QuickestDirection( pSoldier->bDirection, pSoldier->bDesiredDirection );
		sDirection = pSoldier->bDirection + pSoldier->bTurningIncrement;
		if (sDirection > 7)
		{
			sDirection = 0;
		}
		else
		{
			if ( sDirection < 0 )
			{
				sDirection = 7;
			}
		}
	}


	// CHECK FOR A VALID TURN DIRECTION
	// This is needed for prone animations as well as any multi-tiled structs
	if (OKToAddMercToWorld(pSoldier, (INT8)sDirection))
	{
		// Don't do this if we are walkoing off screen...
		if (gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALKOFF_SCREEN || gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO)
		{

		}
		else
		{
			// ATE: We should only do this if we are STATIONARY!
			if (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_STATIONARY)
			{
				pSoldier->uiStatusFlags |= SOLDIER_LOOK_NEXT_TURNSOLDIER;
			}
			// otherwise, it's handled next tile...
		}

		EVENT_SetSoldierDirection(pSoldier, sDirection);

		if (pSoldier->ubBodyType != LARVAE_MONSTER && !MercInWater(pSoldier) && pSoldier->bOverTerrainType != DIRT_ROAD && pSoldier->bOverTerrainType != PAVED_ROAD)
		{
			PlaySoldierFootstepSound(pSoldier);
		}
	}
	else
	{
		// Are we prone crawling?
		if (pSoldier->usAnimState == CRAWLING)
		{
			// OK, we want to getup, turn and go prone again....
			ChangeSoldierStance(pSoldier, ANIM_CROUCH);
			pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_ENDING_UP_FROM_MOVE;
		}
		// If we are a creature, or multi-tiled, cancel AI action.....?
		else if (pSoldier->uiStatusFlags & SOLDIER_MULTITILE)
		{
			pSoldier->bDesiredDirection = pSoldier->bDirection;
		}
	}
}


static const UINT8 gRedGlowR[]=
{
	0, // Normal shades
	25,
	50,
	75,
	100,
	125,
	150,
	175,
	200,
	225,

	0, // For gray palettes
	25,
	50,
	75,
	100,
	125,
	150,
	175,
	200,
	225,

};


#if 0
static const UINT8 gOrangeGlowR[]=
{
	0, // Normal shades
	20,
	40,
	60,
	80,
	100,
	120,
	140,
	160,
	180,

	0, // For gray palettes
	20,
	40,
	60,
	80,
	100,
	120,
	140,
	160,
	180,
};
#endif

static const UINT8 gOrangeGlowR[]=
{
	0, // Normal shades
	25,
	50,
	75,
	100,
	125,
	150,
	175,
	200,
	225,

	0, // For gray palettes
	25,
	50,
	75,
	100,
	125,
	150,
	175,
	200,
	225,

};


#if 0
static const UINT8 gOrangeGlowG[]=
{
	0, // Normal shades
	5,
	10,
	25,
	30,
	35,
	40,
	45,
	50,
	55,

	0, // For gray palettes
	5,
	10,
	25,
	30,
	35,
	40,
	45,
	50,
	55,
};
#endif

static const UINT8 gOrangeGlowG[]=
{
	0, // Normal shades
	20,
	40,
	60,
	80,
	100,
	120,
	140,
	160,
	180,

	0, // For gray palettes
	20,
	40,
	60,
	80,
	100,
	120,
	140,
	160,
	180,

};


static UINT16* CreateEnemyGlow16BPPPalette(const SGPPaletteEntry* pPalette, UINT32 rscale, UINT32 gscale);
static UINT16* CreateEnemyGreyGlow16BPPPalette(const SGPPaletteEntry* pPalette, UINT32 rscale, UINT32 gscale);


void CreateSoldierPalettes(SOLDIERTYPE& s)
{
	// --- TAKE FROM CURRENT ANIMATION HVOBJECT!
	UINT16 const anim_surface = GetSoldierAnimationSurface(&s);
	if (anim_surface == INVALID_ANIMATION_SURFACE)
	{
		throw std::runtime_error("Palette creation failed, soldier has invalid animation");
	}

	SGPPaletteEntry tmp_pal[256];
	std::fill_n(tmp_pal, 256, SGPPaletteEntry{});

	SGPPaletteEntry const*       pal;
	char            const* const substitution = GetBodyTypePaletteSubstitution(&s, s.ubBodyType);
	if (!substitution)
	{
		// ATE: here we want to use the breath cycle for the palette.....
		UINT16 const palette_anim_surface = LoadSoldierAnimationSurface(s, STANDING);
		if (palette_anim_surface != INVALID_ANIMATION_SURFACE)
		{
			// Use palette from HVOBJECT, then use substitution for pants, etc
			memcpy(tmp_pal, gAnimSurfaceDatabase[palette_anim_surface].hVideoObject->Palette(), sizeof(*tmp_pal) * 256);

			// Substitute based on head, etc
			SetPaletteReplacement(tmp_pal, s.HeadPal);
			SetPaletteReplacement(tmp_pal, s.VestPal);
			SetPaletteReplacement(tmp_pal, s.PantsPal);
			SetPaletteReplacement(tmp_pal, s.SkinPal);
		}
		pal = tmp_pal;
	}
	else if (substitution[0] != '\0' && CreateSGPPaletteFromCOLFile(tmp_pal, substitution))
	{
		pal = tmp_pal;
	}
	else
	{
		// Use palette from hvobject
		pal = gAnimSurfaceDatabase[anim_surface].hVideoObject->Palette();
	}


	for (INT32 i = 0; i < NUM_SOLDIER_SHADES; ++i)
	{
		if (s.pShades[i])
		{
			delete[] s.pShades[i];
			s.pShades[i] = 0;
		}
	}

	if (s.effect_shade)
	{
		delete[] s.effect_shade;
		s.effect_shade = 0;
	}

	for (INT32 i = 0; i < 20; ++i)
	{
		if (s.pGlowShades[i])
		{
			delete[] s.pGlowShades[i];
			s.pGlowShades[i] = 0;
		}
	}


	CreateBiasedShadedPalettes(s.pShades, pal);

	s.effect_shade = Create16BPPPaletteShaded(pal, 100, 100, 100, TRUE);

	// Build shades for glowing visible bad guy

	// First do visible guy
	s.pGlowShades[0] = Create16BPPPaletteShaded(pal, 255, 255, 255, FALSE);
	for (INT32 i = 1; i < 10; ++i)
	{
		s.pGlowShades[i] = CreateEnemyGlow16BPPPalette(pal, gRedGlowR[i], 0);
	}

	// Now for gray guy...
	s.pGlowShades[10] = Create16BPPPaletteShaded(pal, 100, 100, 100, TRUE);
	for (INT32 i = 11; i < 19; ++i)
	{
		s.pGlowShades[i] = CreateEnemyGreyGlow16BPPPalette(pal, gRedGlowR[i], 0);
	}
	s.pGlowShades[19] = CreateEnemyGreyGlow16BPPPalette(pal, gRedGlowR[18], 0);

	// ATE: OK, piggyback on the shades we are not using for 2 colored lighting....
	// ORANGE, VISIBLE GUY
	s.pShades[20] = Create16BPPPaletteShaded(pal, 255, 255, 255, FALSE);
	for (INT32 i = 21; i < 30; ++i)
	{
		s.pShades[i] = CreateEnemyGlow16BPPPalette(pal, gOrangeGlowR[i - 20], gOrangeGlowG[i - 20]);
	}

	// ORANGE, GREY GUY
	s.pShades[30] = Create16BPPPaletteShaded(pal, 100, 100, 100, TRUE);
	for (INT32 i = 31; i < 39; ++i)
	{
		s.pShades[i] = CreateEnemyGreyGlow16BPPPalette(pal, gOrangeGlowR[i - 20], gOrangeGlowG[i - 20]);
	}
	s.pShades[39] = CreateEnemyGreyGlow16BPPPalette(pal, gOrangeGlowR[18], gOrangeGlowG[18]);
}


static void AdjustAniSpeed(SOLDIERTYPE* pSoldier)
{
	if ( ( gTacticalStatus.uiFlags & SLOW_ANIMATION ) )
	{
		if ( gTacticalStatus.bRealtimeSpeed == -1 )
		{
			pSoldier->sAniDelay = 10000;
		}
		else
		{
			pSoldier->sAniDelay = pSoldier->sAniDelay * ( 1 * gTacticalStatus.bRealtimeSpeed / 2 );
		}
	}


	RESETTIMECOUNTER( pSoldier->UpdateCounter, pSoldier->sAniDelay );
}


static void CalculateSoldierAniSpeed(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pStatsSoldier)
{
	UINT32 uiTerrainDelay;

	INT8 bBreathDef, bLifeDef, bAgilDef;
	INT8 bAdditional = 0;

	// for those animations which have a speed of zero, we have to calculate it
	// here. Some animation, such as water-movement, have an ADDITIONAL speed
	switch( pSoldier->usAnimState )
	{
		case PRONE:
		case STANDING:

			pSoldier->sAniDelay = ( pStatsSoldier->bBreath * 2 ) + (100 - pStatsSoldier->bLife );

			// Limit it!
			if ( pSoldier->sAniDelay < 40 )
			{
				pSoldier->sAniDelay = 40;
			}
			AdjustAniSpeed( pSoldier );
			return;

		case CROUCHING:

			pSoldier->sAniDelay = ( pStatsSoldier->bBreath * 2 ) + ( (100 - pStatsSoldier->bLife ) );

			// Limit it!
			if ( pSoldier->sAniDelay < 40 )
			{
				pSoldier->sAniDelay = 40;
			}
			AdjustAniSpeed( pSoldier );
			return;

		case WALKING:

			// Adjust based on body type
			bAdditional = (UINT8)( gubAnimWalkSpeeds[ pStatsSoldier->ubBodyType ].sSpeed );
			if ( bAdditional < 0 )
				bAdditional = 0;
			break;

		case RUNNING:

			// Adjust based on body type
			bAdditional = (UINT8)gubAnimRunSpeeds[ pStatsSoldier->ubBodyType ].sSpeed;
			if ( bAdditional < 0 )
				bAdditional = 0;
			break;

		case SWATTING:

			// Adjust based on body type
			if ( pStatsSoldier->ubBodyType <= REGFEMALE )
			{
				bAdditional = (UINT8)gubAnimSwatSpeeds[ pStatsSoldier->ubBodyType ].sSpeed;
				if ( bAdditional < 0 )
					bAdditional = 0;
			}
			break;

		case CRAWLING:

			// Adjust based on body type
			if ( pStatsSoldier->ubBodyType <= REGFEMALE )
			{
				bAdditional = (UINT8)gubAnimCrawlSpeeds[ pStatsSoldier->ubBodyType ].sSpeed;
				if ( bAdditional < 0 )
					bAdditional = 0;
			}
			break;

		case READY_RIFLE_STAND:

			// Raise rifle based on aim vs non-aim.
			if ( pSoldier->bAimTime == 0 )
			{
				// Quick shot
				pSoldier->sAniDelay = 70;
			}
			else
			{
				pSoldier->sAniDelay = 150;
			}
			AdjustAniSpeed( pSoldier );
			return;
	}


	// figure out movement speed (terrspeed)
	if ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_MOVING )
	{
		static std::array<uint8_t, NUM_TERRAIN_TYPES> const terrainTypeSpeedModifiers
		{
			5, // Nothing,
			5, // Flat ground
			5, // Floor
			5, // Paved road
			5, // Dirt road
			10, // LOW GRASS
			15, // HIGH GRASS
			20, // TRAIN TRACKS
			20, // LOW WATER
			25, // MID WATER
			30 // DEEP WATER
		};

		uiTerrainDelay = terrainTypeSpeedModifiers[pStatsSoldier->bOverTerrainType];
	}
	else
	{
		uiTerrainDelay = 40; // standing still
	}

	bBreathDef = 50 - ( pStatsSoldier->bBreath / 2 );

	if ( bBreathDef > 30 )
		bBreathDef = 30;

	bAgilDef = 50 - ( EffectiveAgility( pStatsSoldier ) / 4 );
	bLifeDef = 50 - ( pStatsSoldier->bLife / 2 );

	uiTerrainDelay += ( bLifeDef + bBreathDef + bAgilDef + bAdditional );

	pSoldier->sAniDelay = (INT16)uiTerrainDelay;

	// If a moving animation and w/re on drugs, increase speed....
	if ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_MOVING )
	{
		if ( GetDrugEffect( pSoldier, DRUG_TYPE_ADRENALINE ) )
		{
			pSoldier->sAniDelay = pSoldier->sAniDelay / 2;
		}
	}

	// MODIFTY NOW BASED ON REAL-TIME, ETC
	// Adjust speed, make twice as fast if in turn-based!
	if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		pSoldier->sAniDelay = pSoldier->sAniDelay / 2;
	}

	// MODIFY IF REALTIME COMBAT
	if ( !( gTacticalStatus.uiFlags & INCOMBAT ) )
	{
		// ATE: If realtime, and stealth mode...
		if ( pStatsSoldier->bStealthMode )
		{
			pSoldier->sAniDelay = (INT16)( pSoldier->sAniDelay * 2 );
		}

		//pSoldier->sAniDelay = pSoldier->sAniDelay * ( 1 * gTacticalStatus.bRealtimeSpeed / 2 );
	}
}


void SetSoldierAniSpeed(SOLDIERTYPE* pSoldier)
{
	SOLDIERTYPE *pStatsSoldier;


	// ATE: If we are an enemy and are not visible......
	// Set speed to 0
	if ((gTacticalStatus.uiFlags & INCOMBAT) || gTacticalStatus.fAutoBandageMode)
	{
		if ( ( ( pSoldier->bVisible == -1 && pSoldier->bVisible == pSoldier->bLastRenderVisibleValue ) ||
			gTacticalStatus.fAutoBandageMode ) && pSoldier->usAnimState != MONSTER_UP )
		{
			pSoldier->sAniDelay = 0;
			RESETTIMECOUNTER( pSoldier->UpdateCounter, pSoldier->sAniDelay );
			return;
		}
	}

	// Default stats soldier to same as normal soldier.....
	pStatsSoldier = pSoldier;

	if ( pSoldier->fUseMoverrideMoveSpeed )
	{
		pStatsSoldier = &GetMan(pSoldier->bOverrideMoveSpeed);
	}

	// Only calculate if set to zero
	if ( ( pSoldier->sAniDelay = gAnimControl[ pSoldier->usAnimState ].sSpeed ) == 0 )
	{
		CalculateSoldierAniSpeed( pSoldier, pStatsSoldier );
	}

	AdjustAniSpeed( pSoldier );

	if (_KeyDown(SDLK_SPACE))
	{
		//pSoldier->sAniDelay = 1000;
	}

}


///////////////////////////////////////////////////////
//PALETTE REPLACEMENT FUNCTIONS
///////////////////////////////////////////////////////
void LoadPaletteData()
{
	UINT32 cnt, cnt2;

	AutoSGPFile hFile(GCM->openGameResForReading(PALETTEFILENAME));

	// Read # of types
	hFile->read(&guiNumPaletteSubRanges, sizeof(guiNumPaletteSubRanges));

	// Malloc!
	gpPaletteSubRanges          = new PaletteSubRangeType[guiNumPaletteSubRanges]{};
	gubpNumReplacementsPerRange = new UINT8[guiNumPaletteSubRanges]{};

	// Read # of types for each!
	for ( cnt = 0; cnt < guiNumPaletteSubRanges; cnt++ )
	{
		hFile->read(&gubpNumReplacementsPerRange[cnt], sizeof(UINT8));
	}

	// Loop for each one, read in data
	for ( cnt = 0; cnt < guiNumPaletteSubRanges; cnt++ )
	{
		hFile->read(&gpPaletteSubRanges[cnt].ubStart, sizeof(UINT8));
		hFile->read(&gpPaletteSubRanges[cnt].ubEnd,   sizeof(UINT8));
	}


	// Read # of palettes
	hFile->read(&guiNumReplacements, sizeof(guiNumReplacements));

	// Malloc!
	gpPalRep = new PaletteReplacementType[guiNumReplacements]{};

	// Read!
	for ( cnt = 0; cnt < guiNumReplacements; cnt++ )
	{
		// type
		hFile->read(&gpPalRep[cnt].ubType, sizeof(gpPalRep[cnt].ubType));

		ST::char_buffer buf{PaletteRepID_LENGTH, '\0'};
		hFile->read(buf.data(), buf.size() * sizeof(char));
		gpPalRep[cnt].ID = ST::string(buf.c_str(), ST_AUTO_SIZE, ST::substitute_invalid);

		// # entries
		hFile->read(&gpPalRep[cnt].ubPaletteSize, sizeof(gpPalRep[cnt].ubPaletteSize));

		SGPPaletteEntry* const Pal = new SGPPaletteEntry[gpPalRep[cnt].ubPaletteSize]{};
		gpPalRep[cnt].rgb = Pal;

		for( cnt2 = 0; cnt2 < gpPalRep[ cnt ].ubPaletteSize; cnt2++ )
		{
			hFile->read(&Pal[cnt2].r, sizeof(Pal[cnt2].r));
			hFile->read(&Pal[cnt2].g, sizeof(Pal[cnt2].g));
			hFile->read(&Pal[cnt2].b, sizeof(Pal[cnt2].b));
		}

	}
}


void SetPaletteReplacement(SGPPaletteEntry* p8BPPPalette, const ST::string& aPalRep)
{
	UINT32 cnt2;

	auto const ubPalIndex = GetPaletteRepIndexFromID(aPalRep);
	if (!ubPalIndex)
	{
		return;
	}

	// Get range type
	auto const ubType = gpPalRep[*ubPalIndex].ubType;

	for ( cnt2 = gpPaletteSubRanges[ ubType ].ubStart; cnt2 <= gpPaletteSubRanges[ ubType ].ubEnd; cnt2++ )
	{
		p8BPPPalette[cnt2] = gpPalRep[*ubPalIndex].rgb[cnt2 - gpPaletteSubRanges[ubType].ubStart];
	}
}


void DeletePaletteData()
{
	UINT32 cnt;

	// Free!
	if ( gpPaletteSubRanges != NULL )
	{
		delete[] gpPaletteSubRanges;
		gpPaletteSubRanges = NULL;
	}

	if ( gubpNumReplacementsPerRange != NULL )
	{
		delete[] gubpNumReplacementsPerRange;
		gubpNumReplacementsPerRange = NULL;
	}


	for ( cnt = 0; cnt < guiNumReplacements; cnt++ )
	{
		if (gpPalRep[cnt].rgb != NULL) delete[] gpPalRep[cnt].rgb;
	}

	// Free
	if ( gpPalRep != NULL )
	{
		delete[] gpPalRep;
		gpPalRep = NULL;
	}
}


std::optional<UINT8> GetPaletteRepIndexFromID(const ST::string& pal_rep)
{
	// Check if type exists
	for (UINT32 i = 0; i < guiNumReplacements; ++i)
	{
		if (pal_rep.compare(gpPalRep[i].ID) == 0) return i;
	}

	return {};
}


static UINT16 GetNewSoldierStateFromNewStance(SOLDIERTYPE* pSoldier, UINT8 ubDesiredStance)
{
	UINT16 usNewState;
	INT8   bCurrentHeight;

	bCurrentHeight = ( ubDesiredStance - gAnimControl[ pSoldier->usAnimState ].ubEndHeight );

	// Now change to appropriate animation

	switch( bCurrentHeight )
	{
		case ANIM_STAND - ANIM_CROUCH:
			usNewState = KNEEL_UP;
			break;
		case ANIM_CROUCH - ANIM_STAND:
			usNewState = KNEEL_DOWN;
			break;

		case ANIM_STAND - ANIM_PRONE:
			usNewState = PRONE_UP;
			break;
		case ANIM_PRONE - ANIM_STAND:
			usNewState = KNEEL_DOWN;
			break;

		case ANIM_CROUCH - ANIM_PRONE:
			usNewState = PRONE_UP;
			break;
		case ANIM_PRONE - ANIM_CROUCH:
			usNewState = PRONE_DOWN;
			break;

		default:

			// Cannot get here unless ub desired stance is bogus
			SLOGD("GetNewSoldierStateFromNewStance bogus ubDesiredStance value {}",
				ubDesiredStance);
			usNewState = pSoldier->usAnimState;
	}

	return( usNewState );
}


void MoveMercFacingDirection( SOLDIERTYPE *pSoldier, BOOLEAN fReverse, FLOAT dMovementDist )
{
	FLOAT dAngle = (FLOAT)0;

	// Determine which direction we are in
	switch( pSoldier->bDirection )
	{
		case NORTH:
			dAngle = (FLOAT)( -1 * PI );
			break;

		case NORTHEAST:
			dAngle = (FLOAT)( PI * .75 );
			break;

		case EAST:
			dAngle = (FLOAT)( PI / 2 );
			break;

		case SOUTHEAST:
			dAngle = (FLOAT)( PI / 4 );
			break;

		case SOUTH:
			dAngle = (FLOAT)0;
			break;

		case SOUTHWEST:
			//dAngle = (FLOAT)(  PI * -.25 );
			dAngle = (FLOAT)-0.786;
			break;

		case WEST:
			dAngle = (FLOAT) ( PI *-.5 );
			break;

		case NORTHWEST:
			dAngle = (FLOAT) ( PI * -.75 );
			break;

	}

	if ( fReverse )
	{
		dMovementDist = dMovementDist * -1;
	}

	MoveMerc( pSoldier, dMovementDist, dAngle, FALSE );
}


static void InternalReceivingSoldierCancelServices(SOLDIERTYPE* pSoldier, BOOLEAN fPlayEndAnim);


void BeginSoldierClimbUpRoof(SOLDIERTYPE* const s)
{
	UINT8 direction;
	if (!FindHigherLevel(s, &direction)) return;

	if (!EnoughPoints(s, GetAPsToClimbRoof(s, FALSE), 0, TRUE)) return;

	if (s->bTeam == OUR_TEAM) SetUIBusy(s);

	s->sTempNewGridNo     = NewGridNo(s->sGridNo, DirectionInc(direction));
	s->ubPendingDirection = direction;
	EVENT_InitNewSoldierAnim(s, CLIMBUPROOF, 0, FALSE);
	InternalReceivingSoldierCancelServices(s, FALSE);
	InternalGivingSoldierCancelServices(s, FALSE);
}


void BeginSoldierClimbWindow(SOLDIERTYPE* const s)
{
	if(!IsFacingClimableWindow(s)) return;

	s->sTempNewGridNo            = NewGridNo(s->sGridNo, DirectionInc(s->bDirection));
	s->fDontChargeTurningAPs     = TRUE;
	//EVENT_SetSoldierDesiredDirectionForward(s, direction);
	s->fTurningUntilDone         = TRUE;
	// ATE: Reset flag to go back to prone
	s->fTurningFromPronePosition = TURNING_FROM_PRONE_OFF;
	//s->usPendingAnimation        = HOPFENCE;
	DeductPoints( s, AP_JUMPFENCE, BP_JUMPFENCE );
	TeleportSoldier( *s, s->sTempNewGridNo, TRUE );
}

void BeginSoldierClimbFence(SOLDIERTYPE* const s)
{
	UINT8 direction;
	if (!FindFenceJumpDirection(s, &direction)) return;

	s->sTempNewGridNo            = NewGridNo(s->sGridNo, DirectionInc(direction));
	s->fDontChargeTurningAPs     = TRUE;
	EVENT_SetSoldierDesiredDirectionForward(s, direction);
	s->fTurningUntilDone         = TRUE;
	// ATE: Reset flag to go back to prone
	s->fTurningFromPronePosition = TURNING_FROM_PRONE_OFF;
	s->usPendingAnimation        = HOPFENCE;
}


static UINT32 SleepDartSuccumbChance(const SOLDIERTYPE* pSoldier)
{
	UINT32 uiChance;
	INT8   bEffectiveStrength;

	// figure out base chance of succumbing,
	bEffectiveStrength = EffectiveStrength( pSoldier );

	if (bEffectiveStrength > 90)
	{
		uiChance = 110 - bEffectiveStrength;
	}
	else if (bEffectiveStrength > 80)
	{
		uiChance = 120 - bEffectiveStrength;
	}
	else if (bEffectiveStrength > 70)
	{
		uiChance = 130 - bEffectiveStrength;
	}
	else
	{
		uiChance = 140 - bEffectiveStrength;
	}

	// add in a bonus based on how long it's been since shot... highest chance at the beginning
	uiChance += (10 - pSoldier->bSleepDrugCounter);

	return( uiChance );
}

void BeginSoldierGetup( SOLDIERTYPE *pSoldier )
{
	// RETURN IF WE ARE BEING SERVICED
	if ( pSoldier->ubServiceCount > 0 )
	{
		return;
	}

	// ATE: Don't getup if we are in a meanwhile
	if ( AreInMeanwhile( ) )
	{
		return;
	}

	if ( pSoldier->bCollapsed )
	{
		if ( pSoldier->bLife >= OKLIFE && pSoldier->bBreath >= OKBREATH && (pSoldier->bSleepDrugCounter == 0) )
		{
			// get up you hoser!

			pSoldier->bCollapsed = FALSE;
			pSoldier->bTurnsCollapsed = 0;

			if ( IS_MERC_BODY_TYPE( pSoldier ) )
			{
				switch( pSoldier->usAnimState )
				{
					case FALLOFF_FORWARD_STOP:
					case PRONE_LAYFROMHIT_STOP:
					case STAND_FALLFORWARD_STOP:
						ChangeSoldierStance( pSoldier, ANIM_CROUCH );
						break;

					case FALLBACKHIT_STOP:
					case FALLOFF_STOP:
					case FLYBACKHIT_STOP:
					case FALLBACK_HIT_STAND:
					case FALLOFF:
					case FLYBACK_HIT:

						// ROLL OVER
						EVENT_InitNewSoldierAnim( pSoldier, ROLLOVER, 0 , FALSE );
						break;

					default:

						ChangeSoldierStance( pSoldier, ANIM_CROUCH );
						break;
				}
			}
			else
			{
				EVENT_InitNewSoldierAnim( pSoldier, END_COWER, 0 , FALSE );
			}
		}
		else
		{
			pSoldier->bTurnsCollapsed++;
			if ( (gTacticalStatus.bBoxingState == BOXING) && (pSoldier->uiStatusFlags & SOLDIER_BOXER) )
			{
				if (pSoldier->bTurnsCollapsed > 1)
				{
					// We have a winnah!  But it isn't this boxer!
					EndBoxingMatch( pSoldier );
				}
			}
		}
	}
	else if ( pSoldier->bSleepDrugCounter > 0 )
	{
		UINT32 uiChance;

		uiChance = SleepDartSuccumbChance( pSoldier );

		if ( PreRandom( 100 ) < uiChance )
		{
			// succumb to the drug!
			DeductPoints( pSoldier, 0, (INT16)( pSoldier->bBreathMax * 100 ) );
			SoldierCollapse( pSoldier );
		}
	}

	if ( pSoldier->bSleepDrugCounter > 0 )
	{
		pSoldier->bSleepDrugCounter--;
	}
}


static void HandleSoldierTakeDamageFeedback(SOLDIERTYPE* pSoldier);


static void HandleTakeDamageDeath(SOLDIERTYPE* pSoldier, UINT8 bOldLife, UINT8 ubReason)
{
	switch( ubReason )
	{
		case TAKE_DAMAGE_BLOODLOSS:
		case TAKE_DAMAGE_ELECTRICITY:
		case TAKE_DAMAGE_GAS:

			if ( pSoldier->bInSector )
			{
				if ( pSoldier->bVisible != -1 )
				{
					if ( ubReason != TAKE_DAMAGE_BLOODLOSS )
					{
						DoMercBattleSound( pSoldier, BATTLE_SOUND_DIE1 );
						pSoldier->fDeadSoundPlayed = TRUE;
					}
				}

				if ( ( ubReason == TAKE_DAMAGE_ELECTRICITY ) && pSoldier->bLife < OKLIFE )
				{
					pSoldier->fInNonintAnim = FALSE;
				}

				// Check for < OKLIFE
				if ( pSoldier->bLife < OKLIFE && pSoldier->bLife != 0 && !pSoldier->bCollapsed)
				{
					SoldierCollapse( pSoldier );
				}

				// THis is for the die animation that will be happening....
				if ( pSoldier->bLife == 0 )
				{
					pSoldier->fDoingExternalDeath = TRUE;
				}

				// Check if he is dead....
				CheckForAndHandleSoldierDyingNotFromHit( pSoldier );

			}

			HandleSoldierTakeDamageFeedback(pSoldier);

			if (fInMapMode || !pSoldier->bInSector)
			{
				if ( pSoldier->bLife == 0 && !( pSoldier->uiStatusFlags & SOLDIER_DEAD ) )
				{
					StrategicHandlePlayerTeamMercDeath(*pSoldier);

					DoMercBattleSound( pSoldier, BATTLE_SOUND_DIE1 );
					pSoldier->fDeadSoundPlayed = TRUE;

					// ATE: DO death sound
					PlayJA2Sample(DOORCR_1, HIGHVOLUME, 1, MIDDLEPAN);
					PlayJA2Sample(HEADCR_1, HIGHVOLUME, 1, MIDDLEPAN);
				}
			}
			break;
	}

	if ( ubReason == TAKE_DAMAGE_ELECTRICITY )
	{
		if ( pSoldier->bLife >= OKLIFE )
		{
			SLOGD("Freeing up attacker from electricity damage");
			ReleaseSoldiersAttacker( pSoldier );
		}
	}
}


static FLOAT CalcSoldierNextBleed(SOLDIERTYPE* pSoldier);


UINT8 SoldierTakeDamage(SOLDIERTYPE* const pSoldier, INT16 sLifeDeduct, INT16 sBreathLoss, const UINT8 ubReason, SOLDIERTYPE* const attacker)
{
	INT8  bOldLife;
	UINT8 ubCombinedLoss;
	INT8  bBandage;
	INT16 sAPCost;
	UINT8 ubBlood;


	pSoldier->ubLastDamageReason = ubReason;


	// CJC Jan 21 99: add check to see if we are hurting an enemy in an enemy-controlled
	// sector; if so, this is a sign of player activity
	switch ( pSoldier->bTeam )
	{
		case ENEMY_TEAM:
			// if we're in the wilderness this always counts
			if (StrategicMap[gWorldSector.AsStrategicIndex()].fEnemyControlled ||
				SectorInfo[gWorldSector.AsByte()].ubTraversability[THROUGH_STRATEGIC_MOVE] != TOWN)
			{
				// update current day of activity!
				UpdateLastDayOfPlayerActivity( (UINT16) GetWorldDay() );
			}
			break;
		case CREATURE_TEAM:
			// always a sign of activity?
			UpdateLastDayOfPlayerActivity( (UINT16) GetWorldDay() );
			break;
		case CIV_TEAM:
			if (pSoldier->ubCivilianGroup == KINGPIN_CIV_GROUP &&
				gubQuest[QUEST_RESCUE_MARIA] == QUESTINPROGRESS && gTacticalStatus.bBoxingState == NOT_BOXING)
			{
				const SOLDIERTYPE* const pMaria = FindSoldierByProfileID(MARIA);
				if (pMaria && pMaria->bInSector)
				{
					SetFactTrue( FACT_MARIA_ESCAPE_NOTICED );
				}
			}
			break;
		default:
			break;
	}

	// Deduct life!, Show damage if we want!
	bOldLife = pSoldier->bLife;

	// OK, If we are a vehicle.... damage vehicle...( people inside... )
	if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		if ( TANK( pSoldier ) )
		{
			//sLifeDeduct = (sLifeDeduct * 2) / 3;
		}
		else
		{
			if ( ubReason == TAKE_DAMAGE_GUNFIRE )
			{
				sLifeDeduct /= 3;
			}
			else if ( ubReason == TAKE_DAMAGE_EXPLOSION && sLifeDeduct > 50 )
			{
				// boom!
				sLifeDeduct *= 2;
			}
		}

		VehicleTakeDamage(pSoldier->bVehicleID, ubReason, sLifeDeduct, pSoldier->sGridNo, attacker);
		HandleTakeDamageDeath( pSoldier, bOldLife, ubReason );
		return( 0 );
	}

	// ATE: If we are elloit being attacked in a meanwhile...
	if ( pSoldier->uiStatusFlags & SOLDIER_NPC_SHOOTING )
	{
		// Almost kill but not quite.....
		sLifeDeduct = ( pSoldier->bLife - 1 );
		// Turn off
		pSoldier->uiStatusFlags &= ( ~SOLDIER_NPC_SHOOTING );
	}

	// CJC: make sure Elliot doesn't bleed to death!
	if ( ubReason == TAKE_DAMAGE_BLOODLOSS && AreInMeanwhile() )
	{
		return( 0 );
	}


	// Calculate bandage
	bBandage = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;

	if( guiCurrentScreen == MAP_SCREEN )
	{
		fReDrawFace = TRUE;
	}

	if ( CREATURE_OR_BLOODCAT( pSoldier ) )
	{
		int sReductionFactor = 0;

		if ( pSoldier->ubBodyType == BLOODCAT )
		{
			sReductionFactor = 2;
		}
		else if (pSoldier->uiStatusFlags & SOLDIER_MONSTER)
		{
			switch( pSoldier->ubBodyType )
			{
				case LARVAE_MONSTER:
				case INFANT_MONSTER:
					sReductionFactor = 1;
					break;
				case YAF_MONSTER:
				case YAM_MONSTER:
					sReductionFactor = 4;
					break;
				case ADULTFEMALEMONSTER:
				case AM_MONSTER:
					sReductionFactor = 6;
					break;
				case QUEENMONSTER:
					// increase with range!
					if (attacker == NULL)
					{
						sReductionFactor = 8;
					}
					else
					{
						sReductionFactor = 4 + PythSpacesAway(attacker->sGridNo, pSoldier->sGridNo) / 2;
					}
					break;
			}
		}

		if ( ubReason == TAKE_DAMAGE_EXPLOSION )
		{
			sReductionFactor /= 4;
		}
		if ( sReductionFactor > 1 )
		{
			sLifeDeduct = (sLifeDeduct + (sReductionFactor / 2 ) ) / sReductionFactor;
		}
		else if (	ubReason == TAKE_DAMAGE_EXPLOSION )
		{
			// take at most 2/3rds
			sLifeDeduct = (sLifeDeduct * 2) / 3;
		}

		// reduce breath loss to a smaller degree, except for the queen...
		if ( pSoldier->ubBodyType == QUEENMONSTER )
		{
			// in fact, reduce breath loss by MORE!
			sReductionFactor = std::min(sReductionFactor, 8);
			sReductionFactor *= 2;
		}
		else
		{
			sReductionFactor /= 2;
		}
		if ( sReductionFactor > 1 )
		{
			sBreathLoss = (sBreathLoss + (sReductionFactor / 2 ) ) / sReductionFactor;
		}
	}

	if (sLifeDeduct > pSoldier->bLife)
	{
		pSoldier->bLife = 0;
	}
	else
	{
		// Decrease Health
		pSoldier->bLife -= sLifeDeduct;
	}

	// ATE: Put some logic in here to allow enemies to die quicker.....
	// Are we an enemy?
	if ( pSoldier->bSide != OUR_TEAM && !pSoldier->bNeutral && pSoldier->ubProfile == NO_PROFILE )
	{
		// ATE: Give them a chance to fall down...
		if ( pSoldier->bLife > 0 && pSoldier->bLife < ( OKLIFE - 1 ) )
		{
			// Are we taking damage from bleeding?
			if ( ubReason == TAKE_DAMAGE_BLOODLOSS )
			{
				// Fifty-fifty chance to die now!
				if ( Random( 2 ) == 0 || gTacticalStatus.Team[ pSoldier->bTeam ].bMenInSector == 1 )
				{
					// Kill!
					pSoldier->bLife = 0;
				}
			}
			else
			{
				// OK, see how far we are..
				if ( pSoldier->bLife < ( OKLIFE - 3 ) )
				{
					// Kill!
					pSoldier->bLife = 0;
				}
			}
		}
	}

	pSoldier->sDamage += sLifeDeduct;

	// Truncate life
	if ( pSoldier->bLife < 0 )
	{
		pSoldier->bLife = 0;
	}


	// Calculate damage to our items if from an explosion!
	if ( ubReason == TAKE_DAMAGE_EXPLOSION || ubReason == TAKE_DAMAGE_STRUCTURE_EXPLOSION)
	{
		CheckEquipmentForDamage( pSoldier, sLifeDeduct );
	}



	// Calculate bleeding
	if ( ubReason != TAKE_DAMAGE_GAS && !AM_A_ROBOT( pSoldier ) )
	{
		if ( ubReason == TAKE_DAMAGE_HANDTOHAND  )
		{
			if ( sLifeDeduct > 0 )
			{
				// HTH does 1 pt bleeding per hit
				pSoldier->bBleeding = pSoldier->bBleeding + 1;
			}
		}
		else
		{
			pSoldier->bBleeding = pSoldier->bLifeMax - ( pSoldier->bLife + bBandage );
		}

	}

	// Deduct breath AND APs!
	sAPCost = (sLifeDeduct / AP_GET_WOUNDED_DIVISOR); // + fallCost;

	// ATE: if the robot, do not deduct
	if ( !AM_A_ROBOT( pSoldier ) )
	{
		DeductPoints( pSoldier, sAPCost, sBreathLoss );
	}

	ubCombinedLoss = (UINT8) sLifeDeduct / 10 + sBreathLoss / 2000;

	// Add shock
	if ( !AM_A_ROBOT( pSoldier ) )
	{
		pSoldier->bShock += ubCombinedLoss;
	}

	// start the stopwatch - the blood is gushing!
	pSoldier->dNextBleed = CalcSoldierNextBleed( pSoldier );

	if ( pSoldier->bInSector && pSoldier->bVisible != -1 )
	{
		// If we are already dead, don't show damage!
		if (bOldLife != 0 && sLifeDeduct != 0 && sLifeDeduct < 1000)
		{
			// Display damage

			// Set Damage display counter
			pSoldier->fDisplayDamage = TRUE;
			pSoldier->bDisplayDamageCount = 0;

			if ( pSoldier->ubBodyType == QUEENMONSTER )
			{
				pSoldier->sDamageX = 0;
				pSoldier->sDamageY = 0;
			}
			else
			{
				pSoldier->sDamageX = pSoldier->sBoundingBoxOffsetX;
				pSoldier->sDamageY = pSoldier->sBoundingBoxOffsetY;
			}
		}
	}

	// OK, if here, let's see if we should drop our weapon....
	if ( ubReason != TAKE_DAMAGE_BLOODLOSS && !(AM_A_ROBOT( pSoldier )) )
	{
		INT16 sTestOne, sTestTwo, sChanceToDrop;

		sTestOne = EffectiveStrength( pSoldier );
		sTestTwo = 2 * std::max(int(sLifeDeduct), sBreathLoss / 100);

		const SOLDIERTYPE* const attacker = pSoldier->attacker;
		if (attacker != NULL && attacker->ubBodyType == BLOODCAT)
		{
			// bloodcat boost, let them make people drop items more
			sTestTwo += 20;
		}

		// If damage > effective strength....
		sChanceToDrop = ( std::max(0, ( sTestTwo - sTestOne ) ));

		// ATE: Increase odds of NOT dropping an UNDROPPABLE OBJECT
		if ( ( pSoldier->inv[ HANDPOS ].fFlags & OBJECT_UNDROPPABLE ) )
		{
			sChanceToDrop -= 30;
		}
		SLOGD("Chance To Drop Weapon: str: {} Dam: {} Chance: {}",
			sTestOne, sTestTwo, sChanceToDrop );

		if ( Random( 100 ) < (UINT16) sChanceToDrop )
		{
			// OK, drop item in main hand...
			if ( pSoldier->inv[ HANDPOS ].usItem != NOTHING )
			{
				if ( !( pSoldier->inv[ HANDPOS ].fFlags & OBJECT_UNDROPPABLE ) )
				{
					// ATE: if our guy, make visible....
					Visibility  const bVisible = pSoldier->bTeam == OUR_TEAM ?
									VISIBLE : INVISIBLE;
					AddItemToPool( pSoldier->sGridNo, &(pSoldier->inv[ HANDPOS ]), bVisible, pSoldier->bLevel, 0, -1 );
					DeleteObj( &(pSoldier->inv[HANDPOS]) );
				}
			}
		}
	}

	// Drop some blood!
	// decide blood amt, if any
	ubBlood = ( sLifeDeduct / BLOODDIVISOR);
	if ( ubBlood > MAXBLOODQUANTITY )
	{
		ubBlood = MAXBLOODQUANTITY;
	}

	if ( !( pSoldier->uiStatusFlags & ( SOLDIER_VEHICLE | SOLDIER_ROBOT ) ) )
	{
		if ( ubBlood != 0 )
		{
			if ( pSoldier->bInSector )
			{
				DropBlood(*pSoldier, ubBlood);
			}
		}
	}

	//Set UI Flag for unconscious, if it's our own guy!
	if ( pSoldier->bTeam == OUR_TEAM  )
	{
		if ( pSoldier->bLife < OKLIFE && pSoldier->bLife > 0 && bOldLife >= OKLIFE )
		{
			fInterfacePanelDirty = DIRTYLEVEL2;
		}
	}

	if ( pSoldier->bInSector )
	{
		CheckForBreathCollapse(*pSoldier);
	}

	// EXPERIENCE CLASS GAIN (combLoss): Getting wounded in battle

	DirtyMercPanelInterface( pSoldier, DIRTYLEVEL1 );


	if (attacker != NULL)
	{
		// don't give exp for hitting friends!
		if (attacker->bTeam == OUR_TEAM && pSoldier->bTeam != OUR_TEAM)
		{
			if ( ubReason == TAKE_DAMAGE_EXPLOSION )
			{
				// EXPLOSIVES GAIN (combLoss):  Causing wounds in battle
				StatChange(*attacker, EXPLODEAMT, 10 * ubCombinedLoss, FROM_FAILURE);
			}
			/*
			else if ( ubReason == TAKE_DAMAGE_GUNFIRE )
			{
				// MARKSMANSHIP GAIN (combLoss):  Causing wounds in battle
				StatChange(*attacker, MARKAMT, 5 * ubCombinedLoss, FALSE);
			}*/
		}
	}

	if (IsOnOurTeam(*pSoldier))
	{
		// EXPERIENCE GAIN: Took some damage
		StatChange(*pSoldier, EXPERAMT, 5 * ubCombinedLoss, FROM_FAILURE);

		// Check for quote
		if ( !(pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_BEING_PUMMELED ) )
		{
			// Check attacker!
			if (attacker != NULL && attacker != pSoldier)
			{
				pSoldier->bNumHitsThisTurn++;

				if ( (pSoldier->bNumHitsThisTurn >= 3) && ( pSoldier->bLife - pSoldier->bOldLife > 20 ) )
				{
					if ( Random(100) < (UINT16)((40 * ( pSoldier->bNumHitsThisTurn - 2))))
					{
						DelayedTacticalCharacterDialogue( pSoldier, QUOTE_TAKEN_A_BREATING );
						pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_BEING_PUMMELED;
						pSoldier->bNumHitsThisTurn = 0;
					}
				}
			}
		}
	}

	if (attacker != NULL && attacker->bTeam == OUR_TEAM && pSoldier->ubProfile != NO_PROFILE &&
		MercProfile(pSoldier->ubProfile).isNPCorRPC())
	{
		gMercProfiles[pSoldier->ubProfile].ubMiscFlags |= PROFILE_MISC_FLAG_WOUNDEDBYPLAYER;
		if (pSoldier->ubProfile == PACOS)
		{
			SetFactTrue( FACT_PACOS_KILLED );
		}
	}

	HandleTakeDamageDeath( pSoldier, bOldLife, ubReason );

	// Check if we are < unconscious, and shutup if so! also wipe sight
	if (pSoldier->bLife < CONSCIOUSNESS) ShutupaYoFace(pSoldier->face);

	if ( pSoldier->bLife < OKLIFE )
	{
		DecayIndividualOpplist( pSoldier );
	}


	return( ubCombinedLoss );
}


BOOLEAN InternalDoMercBattleSound(SOLDIERTYPE* s, BattleSound battle_snd_id, bool const lowerVolume)
{
	constexpr INT8 LOW_MORALE_BATTLE_SND_THRESHOLD = 35;

	CHECKF (battle_snd_id < NUM_MERC_BATTLE_SOUNDS);

	if (s->uiStatusFlags & SOLDIER_VEHICLE)
	{
		// Pick a passenger from vehicle....
		s = PickRandomPassengerFromVehicle(s);
		if (!s) return FALSE;
	}

	// Are we mute?
	if (s->uiStatusFlags & SOLDIER_MUTE) return FALSE;

	// If we are a creature, etc, pick a better sound...
	SoundID sub_snd;
	switch (battle_snd_id)
	{
		case BATTLE_SOUND_HIT1:
		case BATTLE_SOUND_HIT2:
			switch (s->ubBodyType)
			{
				case COW:                sub_snd = COW_HIT_SND;                                    break;
				case YAF_MONSTER:
				case YAM_MONSTER:
				case ADULTFEMALEMONSTER:
				case AM_MONSTER:         sub_snd = Random(2) == 0 ? ACR_DIE_PART1 : ACR_LUNGE;     break;
				case INFANT_MONSTER:     sub_snd = BCR_SHRIEK;                                     break;
				case QUEENMONSTER:       sub_snd = LQ_SHRIEK;                                      break;
				case LARVAE_MONSTER:     sub_snd = BCR_SHRIEK;                                     break;
				case BLOODCAT:           sub_snd = BLOODCAT_HIT_1;                                 break;
				case ROBOTNOWEAPON:      sub_snd = SoundRange<S_METAL_IMPACT1, S_METAL_IMPACT2>(); break;

				default: goto no_sub;
			}
			break;

		case BATTLE_SOUND_DIE1:
			switch (s->ubBodyType)
			{
				case COW:                sub_snd = COW_DIE_SND;          break;
				case YAF_MONSTER:
				case YAM_MONSTER:
				case ADULTFEMALEMONSTER:
				case AM_MONSTER:         sub_snd = CREATURE_FALL_PART_2; break;
				case INFANT_MONSTER:     sub_snd = BCR_DYING;            break;
				case LARVAE_MONSTER:     sub_snd = LCR_RUPTURE;          break;
				case QUEENMONSTER:       sub_snd = LQ_DYING;             break;
				case BLOODCAT:           sub_snd = BLOODCAT_DIE_1;       break;

				case ROBOTNOWEAPON:
					sub_snd = EXPLOSION_1;
					PlayJA2Sample(ROBOT_DEATH, HIGHVOLUME, 1, MIDDLEPAN);
					break;

				default: goto no_sub;
			}
			break;

		default:
			// OK. any other sound, not hits, robot makes a beep
			switch (s->ubBodyType)
			{
				case ROBOTNOWEAPON:
					sub_snd = battle_snd_id == BATTLE_SOUND_ATTN1 ?
						ROBOT_GREETING : ROBOT_BEEP;
					break;

				default: goto no_sub;
			}
			break;
	}

	if (guiCurrentScreen != GAME_SCREEN)
	{
		PlayJA2Sample(sub_snd, HIGHVOLUME, 1, MIDDLEPAN);
	}
	else
	{
		PlayLocationJA2Sample(s->sGridNo, sub_snd, CalculateSpeechVolume(HIGHVOLUME), 1);
	}
	return TRUE;

no_sub:
	auto const& battle_snd = gBattleSndsData[battle_snd_id];

	// Check if this is the same one we just played and we are below the min delay
	if (s->bOldBattleSnd == battle_snd_id &&
		battle_snd.fDontAllowTwoInRow &&
		GetJA2Clock() - s->uiTimeSameBattleSndDone < MIN_SUBSEQUENT_SNDS_DELAY)
	{
		return TRUE;
	}

	// If a battle snd is STILL playing....
	if (SoundIsPlaying(s->uiBattleSoundID))
	{
		// If this is not crucial, skip it
		if (battle_snd.stopDialogue != 1) return TRUE;

		// Stop playing original
		SoundStop(s->uiBattleSoundID);
	}

	// If we are talking now....
	if (IsMercSayingDialogue(s->ubProfile))
	{
		switch (battle_snd.stopDialogue)
		{
			case 1: DialogueAdvanceSpeech(); break; // Stop dialogue
			case 2: return TRUE;                    // Skip battle snd
		}
	}

	// Save this one we're doing...
	s->bOldBattleSnd           = battle_snd_id;
	s->uiTimeSameBattleSndDone = GetJA2Clock();

	//if the sound to be played is a confirmation, check to see if we are to play it
	if (battle_snd_id == BATTLE_SOUND_OK1 &&
		gGameSettings.fOptions[TOPTION_MUTE_CONFIRMATIONS])
	{
		return TRUE;
	}

	// Adjust based on morale...
	if (s->bMorale < LOW_MORALE_BATTLE_SND_THRESHOLD)
	{
		switch (battle_snd_id)
		{
			case BATTLE_SOUND_OK1:   battle_snd_id = BATTLE_SOUND_LOWMORALE_OK1;   break;
			case BATTLE_SOUND_ATTN1: battle_snd_id = BATTLE_SOUND_LOWMORALE_ATTN1; break;
			default:
				break;
		}
	}

	// Randomize between sounds, if applicable (Random(0) returns 0).
	battle_snd_id = static_cast<BattleSound>(battle_snd_id + Random(battle_snd.ubRandomVal));
	auto const& adjusted_battle_snd = gBattleSndsData[battle_snd_id];

	ST::string basename;
	if (s->ubProfile != NO_PROFILE)
	{
		basename = ST::format("{03d}", s->ubProfile);
	}
	else
	{
		// Check if we can play this!
		if (!adjusted_battle_snd.fBadGuy) return FALSE;

		char const* const prefix = s->ubBodyType == HATKIDCIV ||
						s->ubBodyType == KIDCIV ? "kid" : "bad";
		basename = ST::format("{}{d}", prefix, s->ubBattleSoundID);
	}

	ST::string filename = ST::format(BATTLESNDSDIR "/{}_{}.wav", basename, adjusted_battle_snd.zName);
	if (!GCM->doesGameResExists(filename))
	{
		if (battle_snd_id == BATTLE_SOUND_DIE1)
		{
			// The "die" sound filenames differs between profiles and languages
			filename = ST::format(BATTLESNDSDIR "/{}_dying.wav", basename);
			if (GCM->doesGameResExists(filename)) goto file_exists;
		}

		if (s->ubProfile == NO_PROFILE) return FALSE;

		// Generic replacement voices
		char const prefix = s->ubBodyType == REGFEMALE ? 'f' : 'm';
		filename = ST::format(BATTLESNDSDIR "/{c}_{}.wav", prefix, adjusted_battle_snd.zName);
	}
file_exists:;

	// ATE: Reduce volume for OK sounds...
	// (Only for all-moves or multi-selection cases)
	UINT32 const base_volume = lowerVolume == BATTLE_SND_LOWER_VOLUME ? MIDVOLUME : HIGHVOLUME;
	UINT32       volume      = CalculateSpeechVolume(base_volume);

	// If we are an enemy.....reduce due to volume
	if (s->bTeam != OUR_TEAM)
	{
		volume = SoundVolume(volume, s->sGridNo);
	}

	UINT32 const pan       = SoundDir(s->sGridNo);
	UINT32 const uiSoundID = SoundPlay(filename.c_str(), volume, pan, 1, NULL, NULL);
	if (uiSoundID == SOUND_ERROR) return FALSE;
	s->uiBattleSoundID = uiSoundID;

	if (s->ubProfile != NO_PROFILE)
	{
		FACETYPE* const face = s->face;
		if (face) ExternSetFaceTalking(*face, uiSoundID);
	}

	return TRUE;
}


void MakeCharacterDialogueEventDoBattleSound(SOLDIERTYPE& s, BattleSound const sound, UINT32 const delay)
{
	class CharacterDialogueEventDoBattleSound : public CharacterDialogueEvent
	{
		public:
			CharacterDialogueEventDoBattleSound(SOLDIERTYPE& s, BattleSound const sound, UINT32 const delay) :
				CharacterDialogueEvent(s),
				sound_(sound),
				time_stamp_(GetJA2Clock()),
				delay_(delay)
			{}

			bool Execute()
			{
				// ATE: If a battle sound, and delay value was given, set time stamp now
				if (delay_ != 0 && GetJA2Clock() - time_stamp_ < delay_) return true;

				if (!MayExecute()) return true;

				InternalDoMercBattleSound(&soldier_, sound_, 0);
				return false;
			}

		private:
			BattleSound const sound_;
			UINT32      const time_stamp_;
			UINT32      const delay_;
	};

	DialogueEvent::Add(new CharacterDialogueEventDoBattleSound(s, sound, delay));
}


BOOLEAN DoMercBattleSound(SOLDIERTYPE* const s, BattleSound const battle_snd_id)
{
	// We WANT to play some RIGHT AWAY or merc is not saying anything right now
	if (gBattleSndsData[battle_snd_id].stopDialogue == 1 ||
		s->ubProfile == NO_PROFILE ||
		InOverheadMap() ||
		!IsMercSayingDialogue(s->ubProfile))
	{
		return InternalDoMercBattleSound(s, battle_snd_id, 0);
	}

	// OK, queue it up otherwise!
	MakeCharacterDialogueEventDoBattleSound(*s, battle_snd_id, 0);
	return TRUE;
}


BOOLEAN CheckSoldierHitRoof( SOLDIERTYPE *pSoldier )
{
	// Check if we are near a lower level
	UINT8   bNewDirection;
	BOOLEAN fReturnVal = FALSE;
	INT16   sNewGridNo;
	// Default to true
	BOOLEAN fDoForwards = TRUE;

	if ( pSoldier->bLife >= OKLIFE )
	{
		return( FALSE );
	}

	if (FindLowerLevel(pSoldier, &bNewDirection))
	{
		// ONly if standing!
		if ( gAnimControl[ pSoldier->usAnimState ].ubHeight == ANIM_STAND )
		{
			// We are near a lower level.
			// Use opposite direction
			bNewDirection = OppositeDirection(bNewDirection);

			// Alrighty, let's not blindly change here, look at whether the dest gridno is good!
			sNewGridNo = NewGridNo(pSoldier->sGridNo, DirectionInc(OppositeDirection(bNewDirection)));
			if ( !NewOKDestination( pSoldier, sNewGridNo, TRUE, 0 ) )
			{
				return( FALSE );
			}
			sNewGridNo = NewGridNo(sNewGridNo, DirectionInc(OppositeDirection(bNewDirection)));
			if ( !NewOKDestination( pSoldier, sNewGridNo, TRUE, 0 ) )
			{
				return( FALSE );
			}

			// Are wee near enough to fall forwards....
			if (pSoldier->bDirection == OneCDirection(bNewDirection) ||
				pSoldier->bDirection == TwoCDirection(bNewDirection) ||
				pSoldier->bDirection == bNewDirection ||
				pSoldier->bDirection == OneCCDirection(bNewDirection) ||
				pSoldier->bDirection == TwoCCDirection(bNewDirection))
			{
				// Do backwards...
				fDoForwards = FALSE;
			}

			// If we are facing the opposite direction, fall backwards
			// ATE: Make this more usefull...
			if ( fDoForwards )
			{
				pSoldier->sTempNewGridNo = NewGridNo( (UINT16)pSoldier->sGridNo, DirectionInc( OppositeDirection( bNewDirection ) ) );
				pSoldier->sTempNewGridNo = NewGridNo( (UINT16)pSoldier->sTempNewGridNo, DirectionInc( OppositeDirection( bNewDirection ) ) );
				EVENT_SetSoldierDesiredDirection(pSoldier, OppositeDirection(bNewDirection));
				pSoldier->fTurningUntilDone = TRUE;
				pSoldier->usPendingAnimation = FALLFORWARD_ROOF;
				//EVENT_InitNewSoldierAnim( pSoldier, FALLFORWARD_ROOF, 0 , FALSE );

				// Deduct hitpoints/breath for falling!
				SoldierTakeDamage(pSoldier, 100, 5000, TAKE_DAMAGE_FALLROOF, NULL);

				fReturnVal = TRUE;

			}
			else
			{

				pSoldier->sTempNewGridNo = NewGridNo( (UINT16)pSoldier->sGridNo, DirectionInc( OppositeDirection( bNewDirection ) ) );
				pSoldier->sTempNewGridNo = NewGridNo( (UINT16)pSoldier->sTempNewGridNo, DirectionInc( OppositeDirection( bNewDirection ) ) );
				EVENT_SetSoldierDesiredDirection( pSoldier, bNewDirection );
				pSoldier->fTurningUntilDone = TRUE;
				pSoldier->usPendingAnimation = FALLOFF;

				// Deduct hitpoints/breath for falling!
				SoldierTakeDamage(pSoldier, 100, 5000, TAKE_DAMAGE_FALLROOF, NULL);

				fReturnVal = TRUE;
			}
		}
	}

	return( fReturnVal );
}


void BeginSoldierClimbDownRoof(SOLDIERTYPE* const s)
{
	UINT8 direction;
	if (!FindLowerLevel(s, &direction)) return;

	if (!EnoughPoints(s, GetAPsToClimbRoof(s, TRUE), 0, TRUE)) return;

	if (s->bTeam == OUR_TEAM) SetUIBusy(s);

	s->sTempNewGridNo     = NewGridNo(s->sGridNo, DirectionInc(direction));
	s->ubPendingDirection = TwoCDirection(direction);
	EVENT_InitNewSoldierAnim(s, CLIMBDOWNROOF, 0, FALSE);
	InternalReceivingSoldierCancelServices(s, FALSE);
	InternalGivingSoldierCancelServices(s, FALSE);
}


void MoveMerc( SOLDIERTYPE *pSoldier, FLOAT dMovementChange, FLOAT dAngle, BOOLEAN fCheckRange )
{
	FLOAT   dDeltaPos;
	FLOAT   dXPos , dYPos;
	BOOLEAN fStop = FALSE;

	// Find delta Movement for X pos
	dDeltaPos = (FLOAT) (dMovementChange * sin( dAngle ));

	// Find new position
	dXPos = pSoldier->dXPos + dDeltaPos;

	if ( fCheckRange )
	{
		fStop = FALSE;

		switch( pSoldier->bMovementDirection )
		{
			case NORTHEAST:
			case EAST:
			case SOUTHEAST:

				if ( dXPos >= pSoldier->sDestXPos )
				{
					fStop = TRUE;
				}
				break;

			case NORTHWEST:
			case WEST:
			case SOUTHWEST:

				if ( dXPos <= pSoldier->sDestXPos )
				{
					fStop = TRUE;
				}
				break;

			case NORTH:
			case SOUTH:

				fStop = TRUE;
				break;

		}

		if ( fStop )
		{
			//dXPos = pSoldier->sDestXPos;
			pSoldier->fPastXDest = TRUE;

			if ( pSoldier->sGridNo == pSoldier->sFinalDestination )
			{
				dXPos = pSoldier->sDestXPos;
			}
		}
	}

	// Find delta Movement for Y pos
	dDeltaPos = (FLOAT) (dMovementChange * cos( dAngle ));

	// Find new pos
	dYPos = pSoldier->dYPos + dDeltaPos;

	if ( fCheckRange )
	{
		fStop = FALSE;

		switch( pSoldier->bMovementDirection )
		{
			case NORTH:
			case NORTHEAST:
			case NORTHWEST:

				if ( dYPos <= pSoldier->sDestYPos )
				{
					fStop = TRUE;
				}
				break;

			case SOUTH:
			case SOUTHWEST:
			case SOUTHEAST:

				if ( dYPos >= pSoldier->sDestYPos )
				{
					fStop = TRUE;
				}
				break;

			case EAST:
			case WEST:

				fStop = TRUE;
				break;

		}

		if ( fStop )
		{
			//dYPos = pSoldier->sDestYPos;
			pSoldier->fPastYDest = TRUE;

			if ( pSoldier->sGridNo == pSoldier->sFinalDestination )
			{
				dYPos = pSoldier->sDestYPos;
			}
		}
	}

	// OK, set new position
	EVENT_SetSoldierPositionXY(pSoldier, dXPos, dYPos, SSP_NO_DEST | SSP_NO_FINAL_DEST);
}


INT16 GetDirectionFromGridNo(const INT16 sGridNo, const SOLDIERTYPE* const s)
{
	return GetDirectionToGridNoFromGridNo(s->sGridNo, sGridNo);
}


INT16 GetDirectionToGridNoFromGridNo( INT16 sGridNoDest, INT16 sGridNoSrc )
{
	INT16 sXPos2, sYPos2;
	INT16 sXPos, sYPos;

	ConvertGridNoToXY( sGridNoSrc, &sXPos, &sYPos );
	ConvertGridNoToXY( sGridNoDest, &sXPos2, &sYPos2 );

	return( atan8( sXPos2, sYPos2, sXPos, sYPos ) );

}


// Returns the direction (in enum WorldDirections terms) from point 1 to point 2.
UINT8 atan8( INT16 sXPos, INT16 sYPos, INT16 sXPos2, INT16 sYPos2 )
{
	const double x = sXPos2 - sXPos;

	// Negate y because in screen coordinates smaller y values are to the north
	// but in the coordinate systen used by atan2 larger values are to the "north".
	const double y = -(sYPos2 - sYPos);

	// The result of atan2(0, 0) is implementation dependant, ensure we always
	// return the same result for this case.
	if (x == 0.0 && y == 0.0) return EAST;

	double theta = std::atan2(y, x);  // theta now ∈ [-π, +π] = [-180°, 180°]
	if (theta < 0) theta += 2 * M_PI; // theta now ∈ [0, +2π] = [0°, 360°]

	const int directionIndex = static_cast<int>((8 * theta + M_PI) / (2 * M_PI)) % 8;
	static const UINT8 directionTable[8]
	{
		EAST, NORTHEAST, NORTH, NORTHWEST, WEST, SOUTHWEST, SOUTH, SOUTHEAST
	};

	return directionTable[directionIndex];
}


static void AdjustForFastTurnAnimation(SOLDIERTYPE* pSoldier)
{

	// CHECK FOR FASTTURN ANIMATIONS
	// ATE: Mod: Only fastturn for OUR guys!
	if ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_FASTTURN &&
		pSoldier->bTeam == OUR_TEAM && !( pSoldier->uiStatusFlags & SOLDIER_TURNINGFROMHIT ) )
	{
		if ( pSoldier->bDirection != pSoldier->bDesiredDirection )
		{
			pSoldier->sAniDelay = FAST_TURN_ANIM_SPEED;
		}
		else
		{
			SetSoldierAniSpeed( pSoldier );
		}
	}
}


void SendSoldierSetDesiredDirectionEvent(const SOLDIERTYPE* pSoldier, UINT16 usDesiredDirection)
{
	// Sent event for position update
	EV_S_SETDESIREDDIRECTION SSetDesiredDirection;

	SSetDesiredDirection.usSoldierID = pSoldier->ubID;
	SSetDesiredDirection.usDesiredDirection = usDesiredDirection;
	SSetDesiredDirection.uiUniqueId = pSoldier -> uiUniqueSoldierIdValue;

	AddGameEvent(SSetDesiredDirection, 0);
}


void SendGetNewSoldierPathEvent(SOLDIERTYPE* const pSoldier, UINT16 const sDestGridNo)
{
	EV_S_GETNEWPATH	SGetNewPath;

	SGetNewPath.usSoldierID = pSoldier->ubID;
	SGetNewPath.sDestGridNo = sDestGridNo;
	SGetNewPath.usMovementAnim = pSoldier->usUIMovementMode;
	SGetNewPath.uiUniqueId = pSoldier -> uiUniqueSoldierIdValue;

	AddGameEvent(SGetNewPath, 0);
}


void SendBeginFireWeaponEvent( SOLDIERTYPE *pSoldier, INT16 sTargetGridNo )
{
	EV_S_BEGINFIREWEAPON SBeginFireWeapon;

	SBeginFireWeapon.usSoldierID = pSoldier->ubID;
	SBeginFireWeapon.sTargetGridNo = sTargetGridNo;
	SBeginFireWeapon.bTargetLevel = pSoldier->bTargetLevel;
	SBeginFireWeapon.bTargetCubeLevel = pSoldier->bTargetCubeLevel;
	SBeginFireWeapon.uiUniqueId = pSoldier -> uiUniqueSoldierIdValue;

	AddGameEvent(SBeginFireWeapon, 0);
}

// This function just encapolates the check for turnbased and having an attacker in the first place
void ReleaseSoldiersAttacker( SOLDIERTYPE *pSoldier )
{
	INT32 cnt;
	UINT8 ubNumToFree;

	//if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		// ATE: Removed...
		//if (pSoldier->attacker != NULL)
		{
			// JA2 Gold
			// set next-to-previous attacker, so long as this isn't a repeat attack
			if (pSoldier->previous_attacker != pSoldier->attacker)
			{
				pSoldier->next_to_previous_attacker = pSoldier->previous_attacker;
			}

			// get previous attacker id
			pSoldier->previous_attacker = pSoldier->attacker;

			// Copy BeingAttackedCount here....
			ubNumToFree = pSoldier->bBeingAttackedCount;
			// Zero it out BEFORE, as supression may increase it again...
			pSoldier->bBeingAttackedCount = 0;

			for ( cnt = 0; cnt < ubNumToFree; cnt++ )
			{
				SLOGD("Freeing up attacker of {} (attacker is {})\n\
					releasesoldierattacker num to free is {}",
					pSoldier->ubID, SOLDIER2ID(pSoldier->attacker), ubNumToFree);
				ReduceAttackBusyCount(pSoldier->attacker, FALSE);
			}

			// ATE: Set to NOBODY if this person is NOT dead
			// otherise, we keep it so the kill can be awarded!
			if ( pSoldier->bLife != 0 && pSoldier->ubBodyType != QUEENMONSTER )
			{
				pSoldier->attacker = NULL;
			}
		}
	}
}


BOOLEAN MercInWater(const SOLDIERTYPE* pSoldier)
{
	// Our water texture , for now is of a given type
	if ( pSoldier->bOverTerrainType == LOW_WATER || pSoldier->bOverTerrainType == MED_WATER ||
		pSoldier->bOverTerrainType == DEEP_WATER )
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


void ReviveSoldier( SOLDIERTYPE *pSoldier )
{
	if ( pSoldier->bLife < OKLIFE  && pSoldier->bActive )
	{
		// If dead or unconscious, revive!
		pSoldier->uiStatusFlags &= ( ~SOLDIER_DEAD );

		pSoldier->bLife = pSoldier->bLifeMax;
		pSoldier->bBleeding = 0;
		pSoldier->ubDesiredHeight = ANIM_STAND;

		AddManToTeam( pSoldier->bTeam );

		// Set to standing
		pSoldier->fInNonintAnim = FALSE;
		pSoldier->fRTInNonintAnim = FALSE;

		// Change to standing,unless we can getup with an animation
		EVENT_InitNewSoldierAnim( pSoldier, STANDING, 0, TRUE );
		BeginSoldierGetup( pSoldier );

		EVENT_SetSoldierPosition(pSoldier, pSoldier->sGridNo, SSP_NONE);

		// Dirty INterface
		fInterfacePanelDirty = DIRTYLEVEL2;
	}
}


static void HandleAnimationProfile(SOLDIERTYPE& s, UINT16 const usAnimState, BOOLEAN const fRemove)
{
	UINT16 const anim_surface = DetermineSoldierAnimationSurface(&s, usAnimState);
	CHECKV(anim_surface != INVALID_ANIMATION_SURFACE);

	INT8 const profile_id = gAnimSurfaceDatabase[anim_surface].bProfile;
	if (profile_id == -1) return;

	ANIM_PROF     const& profile     = gpAnimProfiles[profile_id];
	ANIM_PROF_DIR const& profile_dir = profile.Dirs[s.bDirection];

	// Loop tiles and set accordingly into world
	for (UINT32 tile_count = 0; tile_count != profile_dir.ubNumTiles; ++tile_count)
	{
		ANIM_PROF_TILE const& profile_tile = profile_dir.pTiles[tile_count];
		GridNo         const  grid_no      = s.sGridNo + WORLD_COLS * profile_tile.bTileY + profile_tile.bTileX;

		// Check if in bounds
		if (OutOfBounds(s.sGridNo, grid_no)) continue;

		if (fRemove)
		{
			// Remove from world
			RemoveMerc(grid_no, s, true);
		}
		else
		{
			// Place into world
			LEVELNODE* const n = AddMercToHead(grid_no, s, FALSE);
			n->uiFlags                |= LEVELNODE_MERCPLACEHOLDER;
			n->uiAnimHitLocationFlags  = profile_tile.usTileFlags;
		}
	}
}


void EVENT_SoldierBeginGiveItem( SOLDIERTYPE *pSoldier )
{
	if (VerifyGiveItem(pSoldier) != NULL)
	{
		// CHANGE DIRECTION AND GOTO ANIMATION NOW
		pSoldier->bDesiredDirection = pSoldier->bPendingActionData3;
		pSoldier->bDirection = pSoldier->bPendingActionData3;

		// begin animation
		EVENT_InitNewSoldierAnim( pSoldier, GIVE_ITEM, 0 , FALSE );

	}
	else
	{
		UnSetEngagedInConvFromPCAction( pSoldier );

		delete pSoldier->pTempObject;
		pSoldier->pTempObject = nullptr;
	}
}


void EVENT_SoldierBeginBladeAttack( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection )
{
	//UINT32 uiMercFlags;
	UINT8 ubTDirection;

	// Increment the number of people busy doing stuff because of an attack
	//if (gTacticalStatus.uiFlags & INCOMBAT)
	//{
		gTacticalStatus.ubAttackBusyCount++;
		SLOGD("Begin blade attack: ATB  {}", gTacticalStatus.ubAttackBusyCount);

	//}

	// CHANGE DIRECTION AND GOTO ANIMATION NOW
	EVENT_SetSoldierDesiredDirection( pSoldier, ubDirection );
	EVENT_SetSoldierDirection( pSoldier, ubDirection );
	// CHANGE TO ANIMATION

	// DETERMINE ANIMATION TO PLAY
	// LATER BASED ON IF TAREGT KNOWS OF US, STANCE, ETC
	// GET POINTER TO TAREGT
	if (pSoldier->uiStatusFlags & SOLDIER_MONSTER)
	{
		// Is there an unconscious guy at gridno......
		const SOLDIERTYPE* const tgt = WhoIsThere2(sGridNo, pSoldier->bTargetLevel);
		if (tgt != NULL && ((tgt->bLife < OKLIFE && tgt->bLife > 0) || (tgt->bBreath < OKBREATH && tgt->bCollapsed)))
		{
			pSoldier->uiPendingActionData4 = tgt->ubID;
			// add regen bonus
			pSoldier->bRegenerationCounter++;
			EVENT_InitNewSoldierAnim( pSoldier, MONSTER_BEGIN_EATTING_FLESH, 0, FALSE );
		}
		else
		{
			if ( PythSpacesAway( pSoldier->sGridNo, sGridNo ) <= 1 )
			{
				EVENT_InitNewSoldierAnim( pSoldier, MONSTER_CLOSE_ATTACK, 0, FALSE );
			}
			else
			{
				EVENT_InitNewSoldierAnim( pSoldier, ADULTMONSTER_ATTACKING, 0, FALSE );
			}
		}
	}
	else if (pSoldier->ubBodyType == BLOODCAT)
	{
		// Check if it's a claws or teeth...
		if ( pSoldier->inv[ HANDPOS ].usItem == BLOODCAT_CLAW_ATTACK )
		{
			EVENT_InitNewSoldierAnim( pSoldier, BLOODCAT_SWIPE, 0, FALSE );
		}
		else
		{
			EVENT_InitNewSoldierAnim( pSoldier, BLOODCAT_BITE_ANIM, 0, FALSE );
		}
	}
	else
	{
		SOLDIERTYPE* const pTSoldier = WhoIsThere2(sGridNo, pSoldier->bTargetLevel);
		if (pTSoldier != NULL)
		{
			// Look at stance of target
			switch( gAnimControl[ pTSoldier->usAnimState ].ubEndHeight	)
			{
				case ANIM_STAND:
				case ANIM_CROUCH:

					// CHECK IF HE CAN SEE US, IF SO RANDOMIZE
					if ( pTSoldier->bOppList[ pSoldier->ubID ] == 0 && pTSoldier->bTeam != pSoldier->bTeam )
					{
						// WE ARE NOT SEEN
						EVENT_InitNewSoldierAnim( pSoldier, STAB, 0 , FALSE );
					}
					else
					{
						// WE ARE SEEN
						if (CoinToss())
						{
							EVENT_InitNewSoldierAnim( pSoldier, STAB, 0 , FALSE );
						}
						else
						{
							EVENT_InitNewSoldierAnim( pSoldier, SLICE, 0 , FALSE );
						}

						// IF WE ARE SEEN, MAKE SURE GUY TURNS!
						// Get direction to target
						// IF WE ARE AN ANIMAL, CAR, MONSTER, DONT'T TURN
						if ( !( pTSoldier->uiStatusFlags & ( SOLDIER_MONSTER | SOLDIER_ANIMAL | SOLDIER_VEHICLE ) ) )
						{
							// OK, stop merc....
							EVENT_StopMerc(pTSoldier);

							if ( pTSoldier->bTeam != OUR_TEAM )
							{
								CancelAIAction(pTSoldier);
							}

							ubTDirection = (UINT8)GetDirectionFromGridNo( pSoldier->sGridNo, pTSoldier );
							SendSoldierSetDesiredDirectionEvent( pTSoldier, ubTDirection );
						}
					}

					break;

				case ANIM_PRONE:

					// CHECK OUR STANCE
					if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_CROUCH )
					{
						// SET DESIRED STANCE AND SET PENDING ANIMATION
						ChangeSoldierStance(pSoldier, ANIM_CROUCH);
						pSoldier->usPendingAnimation = CROUCH_STAB;
					}
					else
					{
						// USE crouched one
						// NEED TO CHANGE STANCE IF NOT CROUCHD!
						EVENT_InitNewSoldierAnim( pSoldier, CROUCH_STAB, 0 , FALSE );
					}
					break;
			}
		}
		else
		{
			// OK, SEE IF THERE IS AN OBSTACLE HERE...
			if ( !NewOKDestination( pSoldier, sGridNo, FALSE, pSoldier->bLevel ) )
			{
				EVENT_InitNewSoldierAnim( pSoldier, STAB, 0 , FALSE );
			}
			else
			{
				const ROTTING_CORPSE* const c = GetCorpseAtGridNo(sGridNo, pSoldier->bLevel);
				const UINT16 state = (c != NULL && IsValidDecapitationCorpse(c) ? DECAPITATE : CROUCH_STAB);
				EVENT_InitNewSoldierAnim(pSoldier, state, 0, FALSE);
			}
		}
	}

	// SET TARGET GRIDNO
	pSoldier->sTargetGridNo = sGridNo;
	pSoldier->bTargetLevel = pSoldier->bLevel;
	pSoldier->target = WhoIsThere2(sGridNo, pSoldier->bTargetLevel);
}


void EVENT_SoldierBeginPunchAttack( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection )
{
	//UINT32 uiMercFlags;
	UINT8 ubTDirection;
	BOOLEAN fChangeDirection = FALSE;
	UINT16	usItem;

	// Get item in hand...
	usItem = pSoldier->inv[ HANDPOS ].usItem;


	// Increment the number of people busy doing stuff because of an attack
	//if (gTacticalStatus.uiFlags & INCOMBAT)
	//{
		gTacticalStatus.ubAttackBusyCount++;
		SLOGD("Begin HTH attack: ATB  {}", gTacticalStatus.ubAttackBusyCount);

	//}

	// get target.....
	SOLDIERTYPE* const pTSoldier = WhoIsThere2(pSoldier->sTargetGridNo, pSoldier->bLevel);
	if (pTSoldier == NULL) return;

	fChangeDirection = TRUE;


	if ( fChangeDirection )
	{
			// CHANGE DIRECTION AND GOTO ANIMATION NOW
		EVENT_SetSoldierDesiredDirection( pSoldier, ubDirection );
		EVENT_SetSoldierDirection( pSoldier, ubDirection );
	}


	if (HAS_SKILL_TRAIT(pSoldier, MARTIALARTS) && !AreInMeanwhile() && usItem != CROWBAR)
	{
		// Are we in attack mode yet?
		if ( pSoldier->usAnimState != NINJA_BREATH && gAnimControl[ pSoldier->usAnimState ].ubHeight == ANIM_STAND && gAnimControl[ pTSoldier->usAnimState ].ubHeight != ANIM_PRONE )
		{
			EVENT_InitNewSoldierAnim( pSoldier, NINJA_GOTOBREATH, 0 , FALSE );
		}
		else
		{
			DoNinjaAttack( pSoldier );
		}
	}
	else
	{
		// Look at stance of target
		switch( gAnimControl[ pTSoldier->usAnimState ].ubEndHeight	)
		{
			case ANIM_STAND:
			case ANIM_CROUCH:

				if ( usItem != CROWBAR )
				{
					EVENT_InitNewSoldierAnim( pSoldier, PUNCH, 0 , FALSE );
				}
				else
				{
					EVENT_InitNewSoldierAnim( pSoldier, CROWBAR_ATTACK, 0 , FALSE );
				}

				// CHECK IF HE CAN SEE US, IF SO CHANGE DIR
				if ( pTSoldier->bOppList[ pSoldier->ubID ] == 0 && pTSoldier->bTeam != pSoldier->bTeam )
				{
					// Get direction to target
					// IF WE ARE AN ANIMAL, CAR, MONSTER, DONT'T TURN
					if ( !( pTSoldier->uiStatusFlags & ( SOLDIER_MONSTER | SOLDIER_ANIMAL | SOLDIER_VEHICLE ) ) )
					{
						// OK, stop merc....
						EVENT_StopMerc(pTSoldier);

						if ( pTSoldier->bTeam != OUR_TEAM )
						{
							CancelAIAction(pTSoldier);
						}

						ubTDirection = (UINT8)GetDirectionFromGridNo( pSoldier->sGridNo, pTSoldier );
						SendSoldierSetDesiredDirectionEvent( pTSoldier, ubTDirection );
					}
				}
				break;

			case ANIM_PRONE:

				// CHECK OUR STANCE
				// ATE: Added this for CIV body types 'cause of elliot
				if ( !IS_MERC_BODY_TYPE( pSoldier ) )
				{
					EVENT_InitNewSoldierAnim( pSoldier, PUNCH, 0 , FALSE );
				}
				else
				{
					if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_CROUCH )
					{
						// SET DESIRED STANCE AND SET PENDING ANIMATION
						ChangeSoldierStance(pSoldier, ANIM_CROUCH);
						pSoldier->usPendingAnimation = PUNCH_LOW;
					}
					else
					{
						// USE crouched one
						// NEED TO CHANGE STANCE IF NOT CROUCHD!
						EVENT_InitNewSoldierAnim( pSoldier, PUNCH_LOW, 0 , FALSE );
					}
				}
				break;
		}
	}

	// SET TARGET GRIDNO
	pSoldier->sTargetGridNo = sGridNo;
	pSoldier->bTargetLevel = pSoldier->bLevel;
	pSoldier->sLastTarget = sGridNo;
	pSoldier->target = WhoIsThere2(sGridNo, pSoldier->bTargetLevel);
}


void EVENT_SoldierBeginKnifeThrowAttack( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection )
{
	// Increment the number of people busy doing stuff because of an attack
	//if (gTacticalStatus.uiFlags & INCOMBAT)
	//{
		gTacticalStatus.ubAttackBusyCount++;
	//}
	pSoldier->bBulletsLeft = 1;
	SLOGD("Starting knifethrow attack, bullets left {}", pSoldier->bBulletsLeft);

	EVENT_InitNewSoldierAnim( pSoldier, THROW_KNIFE, 0 , FALSE );

	// CHANGE DIRECTION AND GOTO ANIMATION NOW
	EVENT_SetSoldierDesiredDirection( pSoldier, ubDirection );
	EVENT_SetSoldierDirection( pSoldier, ubDirection );


	// SET TARGET GRIDNO
	pSoldier->sTargetGridNo = sGridNo;
	pSoldier->fTurningFromPronePosition = 0;
	// NB target level must be set by functions outside of here... but I think it
	// is already set in HandleItem or in the AI code - CJC
	pSoldier->target = WhoIsThere2(sGridNo, pSoldier->bTargetLevel);
}


void EVENT_SoldierBeginDropBomb( SOLDIERTYPE *pSoldier )
{
	// Increment the number of people busy doing stuff because of an attack
	switch( gAnimControl[ pSoldier->usAnimState ].ubHeight )
	{
		case ANIM_STAND:

			EVENT_InitNewSoldierAnim( pSoldier, PLANT_BOMB, 0 , FALSE );
			break;

		default:

			// Call hander for planting bomb...
			HandleSoldierDropBomb( pSoldier, pSoldier->sPendingActionData2 );
			SoldierGotoStationaryStance( pSoldier );
			break;
	}

}


void EVENT_SoldierBeginUseDetonator( SOLDIERTYPE *pSoldier )
{
	// Increment the number of people busy doing stuff because of an attack
	switch( gAnimControl[ pSoldier->usAnimState ].ubHeight )
	{
		case ANIM_STAND:

			EVENT_InitNewSoldierAnim( pSoldier, USE_REMOTE, 0 , FALSE );
			break;

		default:

			// Call hander for planting bomb...
			HandleSoldierUseRemote( pSoldier, pSoldier->sPendingActionData2 );
			break;
	}
}

void EVENT_SoldierBeginFirstAid( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection )
{
	//UINT32 uiMercFlags;
	BOOLEAN fRefused = FALSE;

	SOLDIERTYPE* const pTSoldier = WhoIsThere2(sGridNo, pSoldier->bLevel);
	if (pTSoldier != NULL)
	{
		// OK, check if we should play quote...
		if ( pTSoldier->bTeam != OUR_TEAM )
		{
			if (pTSoldier->ubProfile != NO_PROFILE && MercProfile(pTSoldier->ubProfile).isNPCorRPC() && !RPC_RECRUITED(pTSoldier))
			{
				fRefused = PCDoesFirstAidOnNPC( pTSoldier->ubProfile );
			}

			if ( !fRefused )
			{
				if ( CREATURE_OR_BLOODCAT( pTSoldier ) )
				{
					// nope!!
					fRefused = TRUE;
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, g_langRes->Message[ STR_REFUSE_FIRSTAID_FOR_CREATURE ] );
				}
				else if ( !pTSoldier->bNeutral && pTSoldier->bLife >= OKLIFE && pTSoldier->bSide != pSoldier->bSide )
				{
					fRefused = TRUE;
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, g_langRes->Message[ STR_REFUSE_FIRSTAID ] );
				}

			}
		}

		if ( fRefused )
		{
			UnSetUIBusy(pSoldier);
			return;
		}

		// ATE: We can only give firsty aid to one perosn at a time... cancel
		// any now...
		InternalGivingSoldierCancelServices( pSoldier, FALSE );

		// CHANGE DIRECTION AND GOTO ANIMATION NOW
		EVENT_SetSoldierDesiredDirectionForward(pSoldier, ubDirection);
		EVENT_SetSoldierDirection( pSoldier, ubDirection );

		// CHECK OUR STANCE AND GOTO CROUCH IF NEEDED
		//if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_CROUCH )
		//{
		//	// SET DESIRED STANCE AND SET PENDING ANIMATION
		//	ChangeSoldierStance(pSoldier, ANIM_CROUCH);
		//	pSoldier->usPendingAnimation = START_AID;
		//}
		//else
		{
			// CHANGE TO ANIMATION
			EVENT_InitNewSoldierAnim( pSoldier, START_AID, 0 , FALSE );
		}

		// SET TARGET GRIDNO
		pSoldier->sTargetGridNo = sGridNo;

		// SET PARTNER ID
		pSoldier->service_partner = pTSoldier;

		// SET PARTNER'S COUNT REFERENCE
		pTSoldier->ubServiceCount++;

		// If target and doer are no the same guy...
		if (pTSoldier != pSoldier && !pTSoldier->bCollapsed)
		{
			SoldierGotoStationaryStance( pTSoldier );
		}
	}
}


void EVENT_SoldierEnterVehicle(SOLDIERTYPE& s, GridNo const gridno)
{
	SOLDIERTYPE const* const tgt = FindSoldier(gridno, FIND_SOLDIER_GRIDNO);
	if (tgt && tgt->uiStatusFlags & SOLDIER_VEHICLE)
	{
		VEHICLETYPE& v = GetVehicle(tgt->bVehicleID);
		PutSoldierInVehicle(s, v);
	}
	UnSetUIBusy(&s);
}


UINT32 SoldierDressWound( SOLDIERTYPE *pSoldier, SOLDIERTYPE *pVictim, INT16 sKitPts, INT16 sStatus )
{
	UINT32 uiDressSkill, uiPossible, uiActual, uiMedcost, uiDeficiency, uiAvailAPs, uiUsedAPs;
	UINT8 ubBelowOKlife, ubPtsLeft;

	if (pVictim->bBleeding < 1 && pVictim->bLife >= OKLIFE )
	{
		return(0); // nothing to do, shouldn't have even been called!
	}

	if ( pVictim->bLife == 0 )
	{
		return(0);
	}

	// in case he has multiple kits in hand, limit influence of kit status to 100%!
	if (sStatus >= 100)
	{
		sStatus = 100;
	}

	// calculate wound-dressing skill (3x medical, 2x equip, 10x level, 1x dex)
	uiDressSkill = ((3 * EffectiveMedical(pSoldier)) + // medical knowledge
			(2 * sStatus) + // state of medical kit
			(10 * EffectiveExpLevel(pSoldier)) + // battle injury experience
			EffectiveDexterity(pSoldier)) / 7; // general "handiness"

	// try to use every AP that the merc has left
	uiAvailAPs = pSoldier->bActionPoints;

	// OK, If we are in real-time, use another value...
	if (!(gTacticalStatus.uiFlags & INCOMBAT))
	{
		// Set to a value which looks good based on our tactical turns duration
		uiAvailAPs = RT_FIRST_AID_GAIN_MODIFIER;
	}

	// calculate how much bandaging CAN be done this turn
	uiPossible = ( uiAvailAPs * uiDressSkill ) / 50; // max rate is 2 * fullAPs

	// if no healing is possible (insufficient APs or insufficient dressSkill)
	if (!uiPossible)
		return(0);


	if (pSoldier->inv[ HANDPOS ].usItem == MEDICKIT ) // using the GOOD medic stuff
	{
		uiPossible += ( uiPossible / 2); // add extra 50 %
	}

	uiActual = uiPossible; // start by assuming maximum possible


	// figure out how far below OKLIFE the victim is
	if (pVictim->bLife >= OKLIFE)
	{
		ubBelowOKlife = 0;
	}
	else
	{
		ubBelowOKlife = OKLIFE - pVictim->bLife;
	}

	// figure out how many healing pts we need to stop dying (2x cost)
	uiDeficiency = (2 * ubBelowOKlife );

	// if, after that, the patient will still be bleeding
	if ( (pVictim->bBleeding - ubBelowOKlife ) > 0)
	{
		// then add how many healing pts we need to stop bleeding (1x cost)
		uiDeficiency += ( pVictim->bBleeding - ubBelowOKlife );
	}

	// now, make sure we weren't going to give too much
	if ( uiActual > uiDeficiency) // if we were about to apply too much
		uiActual = uiDeficiency; // reduce actual not to waste anything


	// now make sure we HAVE that much
	if (pSoldier->inv[ HANDPOS ].usItem == MEDICKIT)
	{
		uiMedcost = (uiActual + 1) / 2; // cost is only half, rounded up

		if ( uiMedcost > (UINT32)sKitPts ) // if we can't afford this
		{
			uiMedcost = sKitPts; // what CAN we afford?
			uiActual = uiMedcost * 2; // give double this as aid
		}
	}
	else
	{
		uiMedcost = uiActual;

		if ( uiMedcost > (UINT32)sKitPts) // can't afford it
		{
			uiMedcost = uiActual = sKitPts; // recalc cost AND aid
		}
	}

	ubPtsLeft = (UINT8)uiActual;


	// heal real life points first (if below OKLIFE) because we don't want the
	// patient still DYING if bandages run out, or medic is disabled/distracted!
	// NOTE: Dressing wounds for life below OKLIFE now costs 2 pts/life point!
	if ( ubPtsLeft && pVictim->bLife < OKLIFE)
	{
		// if we have enough points to bring him all the way to OKLIFE this turn
		if ( ubPtsLeft >= (2 * ubBelowOKlife ) )
		{
			// raise life to OKLIFE
			pVictim->bLife = OKLIFE;

			// reduce bleeding by the same number of life points healed up
			pVictim->bBleeding -= ubBelowOKlife;

			// use up appropriate # of actual healing points
			ubPtsLeft -= (2 * ubBelowOKlife);
		}
		else
		{
			pVictim->bLife += ( ubPtsLeft / 2);
			pVictim->bBleeding -= ( ubPtsLeft / 2);

			ubPtsLeft = ubPtsLeft % 2; // if ptsLeft was odd, ptsLeft = 1
		}

		// this should never happen any more, but make sure bleeding not negative
		if (pVictim->bBleeding < 0)
		{
			pVictim->bBleeding = 0;
		}

		// if this healing brought the patient out of the worst of it, cancel dying
		if (pVictim->bLife >= OKLIFE )
		{
			//pVictim->dying = pVictim->dyingComment = FALSE;
			//pVictim->shootOn = TRUE;

			// turn off merc QUOTE flags
			pVictim->fDyingComment = FALSE;

		}

		// update patient's entire panel (could have regained consciousness, etc.)
	}


	// if any healing points remain, apply that to any remaining bleeding (1/1)
	// DON'T spend any APs/kit pts to cure bleeding until merc is no longer dying
	//if ( ubPtsLeft && pVictim->bBleeding && !pVictim->dying)
	if ( ubPtsLeft && pVictim->bBleeding )
	{
		// if we have enough points to bandage all remaining bleeding this turn
		if (ubPtsLeft >= pVictim->bBleeding )
		{
			ubPtsLeft -= pVictim->bBleeding;
			pVictim->bBleeding = 0;
		}
		else // bandage what we can
		{
			pVictim->bBleeding -= ubPtsLeft;
			ubPtsLeft = 0;
		}

		// update patient's life bar only
	}


	// if wound has been dressed enough so that bleeding won't occur, turn off
	// the "warned about bleeding" flag so merc tells us about the next bleeding
	if ( pVictim->bBleeding <= MIN_BLEEDING_THRESHOLD )
	{
		pVictim->fWarnedAboutBleeding = FALSE;
	}


	// if there are any ptsLeft now, then we didn't actually get to use them
	uiActual -= ubPtsLeft;

	// usedAPs equals (actionPts) * (%of possible points actually used)
	uiUsedAPs = ( uiActual * uiAvailAPs ) / uiPossible;

	if (pSoldier->inv[ HANDPOS ].usItem == MEDICKIT) // using the GOOD medic stuff
	{
		uiUsedAPs = ( uiUsedAPs * 2) / 3; // reverse 50% bonus by taking 2/3rds
	}

	DeductPoints( pSoldier, (INT16)uiUsedAPs, (INT16)( ( uiUsedAPs * BP_PER_AP_LT_EFFORT) ) );

	if (IsOnOurTeam(*pSoldier))
	{
		// MEDICAL GAIN   (actual / 2):  Helped someone by giving first aid
		StatChange(*pSoldier, MEDICALAMT, uiActual / 2, FROM_SUCCESS);

		// DEXTERITY GAIN (actual / 6):  Helped someone by giving first aid
		StatChange(*pSoldier, DEXTAMT, uiActual / 6, FROM_SUCCESS);
	}

	return( uiMedcost );
}


static void InternalReceivingSoldierCancelServices(SOLDIERTYPE* pSoldier, BOOLEAN fPlayEndAnim)
{
	if (pSoldier->ubServiceCount <= 0) return;

	// Loop through guys who have us as servicing
	FOR_EACH_SOLDIER(pTSoldier)
	{
		if (pTSoldier->service_partner == pSoldier)
		{
			// END SERVICE!
			pSoldier->ubServiceCount--;

			pTSoldier->service_partner = NULL;

			if (gTacticalStatus.fAutoBandageMode)
			{
				pSoldier->auto_bandaging_medic = NULL;
				ActionDone(pTSoldier);
			}
			else
			{
				// don't use end aid animation in autobandage
				if (pTSoldier->bLife >= OKLIFE && pTSoldier->bBreath > 0 && fPlayEndAnim)
				{
					EVENT_InitNewSoldierAnim(pTSoldier, END_AID, 0, FALSE);
				}
			}
		}
	}
}


void ReceivingSoldierCancelServices( SOLDIERTYPE *pSoldier )
{
	InternalReceivingSoldierCancelServices( pSoldier, TRUE );
}


void InternalGivingSoldierCancelServices( SOLDIERTYPE *pSoldier, BOOLEAN fPlayEndAnim )
{
	// GET TARGET SOLDIER
	SOLDIERTYPE* const pTSoldier = pSoldier->service_partner;
	if (pTSoldier != NULL)
	{
		// END SERVICE!
		pTSoldier->ubServiceCount--;

		pSoldier->service_partner = NULL;

		if ( gTacticalStatus.fAutoBandageMode )
		{
			pTSoldier->auto_bandaging_medic = NULL;

			ActionDone( pSoldier );
		}
		else
		{
			if ( pSoldier->bLife >= OKLIFE && pSoldier->bBreath > 0 && fPlayEndAnim )
			{
				// don't use end aid animation in autobandage
				EVENT_InitNewSoldierAnim( pSoldier, END_AID, 0 , FALSE );
			}
		}
	}

}

void GivingSoldierCancelServices( SOLDIERTYPE *pSoldier )
{
	InternalGivingSoldierCancelServices( pSoldier, TRUE );
}


void HaultSoldierFromSighting( SOLDIERTYPE *pSoldier, BOOLEAN fFromSightingEnemy )
{
	// If we are a 'specialmove... ignore...
	if ( ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_SPECIALMOVE ) )
	{
		return;
	}

	// Whether or not we are about to throw an object
	bool fIsThrowing = (pSoldier->pTempObject != NULL);

	// Below are the animations for attacks which do not involve an object.
	// We use fTurningToShoot and pTempObject to check for other attacks.
	bool fIsAttacking = pSoldier->usPendingAnimation == THROW_KNIFE ||
			 pSoldier->usPendingAnimation == SLICE ||
			 pSoldier->usPendingAnimation == STAB ||
			 pSoldier->usPendingAnimation == CROUCH_STAB ||
			 pSoldier->usPendingAnimation == PUNCH ||
			 pSoldier->usPendingAnimation == PUNCH_LOW ||
			 pSoldier->usPendingAnimation == CROWBAR_ATTACK;

	// OK, check if we were going to throw something, and give it back if so!
	if (fIsThrowing && fFromSightingEnemy)
	{
		// Place it back into inv....
		AutoPlaceObject( pSoldier, pSoldier->pTempObject, FALSE );
		delete pSoldier->pTempObject;
		pSoldier->pTempObject        = NULL;
		pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
	}

	// Here, we need to handle the situation when we're in the middle of an attack animation we see somebody
	if ((fIsThrowing && fFromSightingEnemy) || fIsAttacking)
	{
		// Decrement attack counter...
		SLOGD("Reducing attacker busy count..., ending attack ({}) because saw something", Internals::getAnimationName(pSoldier->usPendingAnimation));
		ReduceAttackBusyCount(pSoldier, FALSE);

		// ATE: Goto stationary stance......
		SoldierGotoStationaryStance( pSoldier );

		DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
	}

	if ( !( gTacticalStatus.uiFlags & INCOMBAT ) )
	{
		EVENT_StopMerc(pSoldier);
	}
	else
	{
		// Pause this guy from no APS
		AdjustNoAPToFinishMove( pSoldier, TRUE );

		pSoldier->ubReasonCantFinishMove = REASON_STOPPED_SIGHT;

		// ATE; IF turning to shoot, stop!
		// ATE: We want to do this only for enemies, not items....
		if ( pSoldier->fTurningToShoot && fFromSightingEnemy )
		{
			pSoldier->fTurningToShoot = FALSE;
			// Release attacker

			// OK - this is hightly annoying , but due to the huge combinations of
			// things that can happen - 1 of them is that sLastTarget will get unset
			// after turn is done - so set flag here to tell it not to...
			pSoldier->fDontUnsetLastTargetFromTurn = TRUE;

			SLOGD("Reducing attacker busy count..., ending fire because saw something");
			ReduceAttackBusyCount(pSoldier, FALSE);
		}

		// OK, if we are stopped at our destination, cancel pending action...
		if ( fFromSightingEnemy )
		{
			Soldier soldier{pSoldier};
			if (soldier.hasPendingAction() && pSoldier->sGridNo == pSoldier->sFinalDestination)
			{
				soldier.removePendingAction();
			}

			// Stop pending animation....
			pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
		}

		if ( !pSoldier->fTurningToShoot )
		{
			pSoldier->fTurningFromPronePosition = FALSE;
		}
	}

	// Unset UI!
	if ( fFromSightingEnemy || ( !fIsThrowing && !pSoldier->fTurningToShoot ) )
	{
		UnSetUIBusy(pSoldier);
	}

	pSoldier->bTurningFromUI = FALSE;

	UnSetEngagedInConvFromPCAction( pSoldier );
}


void EVENT_StopMerc(SOLDIERTYPE* const s)
{
	EVENT_StopMerc(s, s->sGridNo, s->bDirection);
}


// Halt event is used to stop a merc - networking should check / adjust to gridno?
void EVENT_StopMerc(SOLDIERTYPE* const s, GridNo const grid_no, INT8 const direction)
{
	if (!s->fDelayedMovement)
	{
		Soldier{s}.removePendingAnimation();
	}

	s->bEndDoorOpenCode          = 0;
	s->fTurningFromPronePosition = 0;
	s->ubPathIndex               = 0;     // Cancel path data!
	s->ubPathDataSize            = 0;
	s->fDelayedMovement          = FALSE; // Set ext tile waiting flag off
	s->bReverse                  = FALSE; // Turn off reverse

	// Move guy to gridno - should be the same unless in multiplayer
	EVENT_SetSoldierPosition(s, grid_no, SSP_NONE);
	s->sDestXPos = (INT16)s->dXPos;
	s->sDestYPos = (INT16)s->dYPos;
	EVENT_SetSoldierDirection(s, direction);

	if (gAnimControl[s->usAnimState].uiFlags & ANIM_MOVING)
	{
		SoldierGotoStationaryStance(s);
	}

	// ATE: If turning to shoot, stop!
	if (s->fTurningToShoot)
	{
		s->fTurningToShoot = FALSE;
		// Release attacker
		SLOGD("Reducing attacker busy count..., ending fire because saw something");
		ReduceAttackBusyCount(s, FALSE);
	}

	// Turn off multi-move speed override
	if (s->sGridNo == s->sFinalDestination)
	{
		s->fUseMoverrideMoveSpeed = FALSE;
	}

	UnSetUIBusy(s);
	UnMarkMovementReserved(*s);
}


void ReLoadSoldierAnimationDueToHandItemChange(SOLDIERTYPE* const s, UINT16 const usOldItem, UINT16 const usNewItem)
{
	// DON'T continue aiming!
	// GOTO STANCE
	// CHECK FOR AIMING ANIMATIONS

	// Shutoff burst....
	// ( we could be on, then change gun that does not have burst )
	if (GCM->getItem(usNewItem)->isWeapon() && GCM->getWeapon(usNewItem)->ubShotsPerBurst == 0)
	{
		s->bDoBurst    = FALSE;
		s->bWeaponMode = WM_NORMAL;
	}

	if (gAnimControl[s->usAnimState].uiFlags & ANIM_FIREREADY)
	{
		// Stop aiming!
		SoldierGotoStationaryStance(s);
	}

	GivingSoldierCancelServices(s);

	// Switch on stance!
	switch (gAnimControl[s->usAnimState].ubEndHeight)
	{
		case ANIM_STAND:
		{
			// Did we have a rifle and do we now not have one?
			bool const old_rifle = IsRifle(usOldItem);
			bool const new_rifle = IsRifle(usNewItem);
			if (old_rifle && !new_rifle)
			{
				// Put it away!
				EVENT_InitNewSoldierAnim(s, LOWER_RIFLE, 0, FALSE);
				break;
			}
			else if (!old_rifle && new_rifle)
			{
				// Bring it up!
				EVENT_InitNewSoldierAnim(s, RAISE_RIFLE, 0, FALSE);
				break;
			}
		}
			// fallthrough

		case ANIM_CROUCH:
		case ANIM_PRONE:
			SetSoldierAnimationSurface(s, s->usAnimState);
			break;
	}
}


static UINT16* CreateEnemyGlow16BPPPalette(const SGPPaletteEntry* pPalette, UINT32 rscale, UINT32 gscale)
{
	Assert(pPalette != NULL);

	UINT16* const p16BPPPalette = new UINT16[256]{};

	for (UINT32 cnt = 0; cnt < 256; cnt++)
	{
		UINT8 r = std::max(rscale, static_cast<UINT32>(pPalette[cnt].r));
		UINT8 g = std::max(gscale, static_cast<UINT32>(pPalette[cnt].g));
		UINT8 b = pPalette[cnt].b;
		p16BPPPalette[cnt] = Get16BPPColor(FROMRGB(r, g, b));
	}
	return p16BPPPalette;
}


static UINT16* CreateEnemyGreyGlow16BPPPalette(const SGPPaletteEntry* pPalette, UINT32 rscale, UINT32 gscale)
{
	Assert(pPalette != NULL);

	UINT16* const p16BPPPalette = new UINT16[256]{};

	for (UINT32 cnt = 0; cnt < 256; cnt++)
	{
		UINT32 lumin = (pPalette[cnt].r * 299 + pPalette[cnt].g * 587 + pPalette[cnt].b * 114) / 1000;
		UINT32 rmod = 100 * lumin / 256;
		UINT32 gmod = 100 * lumin / 256;
		UINT32 bmod = 100 * lumin / 256;

		rmod = std::max(rscale, rmod);
		gmod = std::max(gscale, gmod);

		UINT8 r = std::min(rmod, 255U);
		UINT8 g = std::min(gmod, 255U);
		UINT8 b = std::min(bmod, 255U);
		p16BPPPalette[cnt] = Get16BPPColor(FROMRGB(r, g, b));
	}
	return p16BPPPalette;
}


void ContinueMercMovement( SOLDIERTYPE *pSoldier )
{
	INT16 sAPCost;
	INT16 sGridNo;

	sGridNo = pSoldier->sFinalDestination;

	// Can we afford this?
	if ( pSoldier->bGoodContPath )
	{
		sGridNo = pSoldier->sContPathLocation;
	}
	else
	{
		// ATE: OK, don't cancel count, so pending actions are still valid...
		pSoldier->ubPendingActionAnimCount = 0;
	}

	// get a path to dest...
	if ( FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, pSoldier->usUIMovementMode, NO_COPYROUTE, 0 ) )
	{
		sAPCost = PtsToMoveDirection( pSoldier, guiPathingData[ 0 ] );

		if (EnoughPoints(pSoldier, sAPCost, 0, pSoldier->bTeam == OUR_TEAM))
		{
			// Acknowledge
			if ( pSoldier->bTeam == OUR_TEAM )
			{
				DoMercBattleSound( pSoldier, BATTLE_SOUND_OK1 );

				// If we have a face, tell text in it to go away!
				FACETYPE* const face = pSoldier->face;
				if (face != NULL) face->fDisplayTextOver = FACE_ERASE_TEXT_OVER;
			}

			AdjustNoAPToFinishMove( pSoldier, FALSE );

			SetUIBusy(pSoldier);

			// OK, try and get a path to out dest!
			EVENT_InternalGetNewSoldierPath( pSoldier, sGridNo, pSoldier->usUIMovementMode, FALSE, TRUE );
		}
	}
}


bool CheckForBreathCollapse(SOLDIERTYPE& s)
{
	// Check if we are out of breath
	if (s.bBreathMax > 70 &&
		s.bBreath < 20 &&
		!(s.usQuoteSaidFlags & SOLDIER_QUOTE_SAID_LOW_BREATH) &&
		gAnimControl[s.usAnimState].ubEndHeight == ANIM_STAND)
	{
		// Warn
		TacticalCharacterDialogue(&s, QUOTE_OUT_OF_BREATH);
		// Set flag indicating we were warned
		s.usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_LOW_BREATH;
	}

#if 0 // XXX was commented out
	// Check for drowning
	if (s.bBreath < 10 &&
		!(s.usQuoteSaidFlags & SOLDIER_QUOTE_SAID_DROWNING) &&
		s.bOverTerrainType == DEEP_WATER)
	{
		// Warn
		TacticalCharacterDialogue(&s, QUOTE_DROWNING);
		// Set flag indicating we were warned
		s.usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_DROWNING;
		// WISDOM GAIN (25): Starting to drown
		StatChange(s, WISDOMAMT, 25, FALSE);
	}
#endif

	if (s.bBreath != 0) return false;
	if (s.bCollapsed)   return false;
	if (s.uiStatusFlags & (SOLDIER_VEHICLE | SOLDIER_ANIMAL | SOLDIER_MONSTER)) return false;

	// Collapse
	// Set a flag, because we may still be in the middle of an animation what is
	// not interruptable
	s.bBreathCollapsed = TRUE;
	return true;
}


BOOLEAN InternalIsValidStance(const SOLDIERTYPE* pSoldier, INT8 bDirection, INT8 bNewStance)
{
	UINT16 usAnimSurface=0;
	UINT16 usAnimState;

	// Check, if dest is prone, we can actually do this!

	// If we are a vehicle, we can only 'stand'
	if ( ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) && bNewStance != ANIM_STAND )
	{
		return( FALSE );
	}

	// Check if we are in water?
	if ( MercInWater( pSoldier ) )
	{
		if ( bNewStance == ANIM_PRONE || bNewStance == ANIM_CROUCH )
		{
			return( FALSE );
		}
	}

	if ( pSoldier->ubBodyType == ROBOTNOWEAPON && bNewStance != ANIM_STAND )
	{
		return( FALSE );
	}

	// Check if we are in water?
	if ( AM_AN_EPC( pSoldier ) )
	{
		if ( bNewStance == ANIM_PRONE )
		{
			return( FALSE );
		}
		else
		{
			return( TRUE );
		}
	}


	if ( pSoldier->bCollapsed )
	{
		if ((bNewStance == ANIM_STAND || bNewStance == ANIM_CROUCH) && pSoldier->bBreath < OKBREATH)
		{
			return( FALSE );
		}
	}

	switch( bNewStance )
	{
		case ANIM_STAND:

			usAnimState = STANDING;
			break;

		case ANIM_CROUCH:

			usAnimState = CROUCHING;
			break;


		case ANIM_PRONE:

			usAnimState = PRONE;
			break;

		default:

			// Something gone funny here....
			usAnimState = pSoldier->usAnimState;
			SLOGW("Wrong desired stance given: {}, {}.", bNewStance, pSoldier->usAnimState);
	}

	usAnimSurface = DetermineSoldierAnimationSurface( pSoldier,  usAnimState );

	// Get structure ref........
	const STRUCTURE_FILE_REF* const pStructureFileRef = GetAnimationStructureRef(pSoldier, usAnimSurface, usAnimState);
	if ( pStructureFileRef != NULL )
	{
		// Check if we can do this....
		UINT16 const usOKToAddStructID = GetStructureID(pSoldier);

		// Can we add structure data for this stance...?
		if (!OkayToAddStructureToWorld(pSoldier->sGridNo, pSoldier->bLevel, &pStructureFileRef->pDBStructureRef[OneCDirection(bDirection)], usOKToAddStructID))
		{
			return( FALSE );
		}
	}

	return( TRUE );
}


BOOLEAN IsValidStance(const SOLDIERTYPE* pSoldier, INT8 bNewStance)
{
	return( InternalIsValidStance( pSoldier, pSoldier->bDirection, bNewStance ) );
}


BOOLEAN IsValidMovementMode(const SOLDIERTYPE* pSoldier, INT16 usMovementMode)
{
	// Check, if dest is prone, we can actually do this!

	// Check if we are in water?
	if ( MercInWater( pSoldier ) )
	{
		if ( usMovementMode == RUNNING || usMovementMode == SWATTING || usMovementMode == CRAWLING )
		{
			return( FALSE );
		}
	}

	return( TRUE );
}


void SelectMoveAnimationFromStance( SOLDIERTYPE *pSoldier )
{
	// Determine which animation to do...depending on stance and gun in hand...
	switch ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight )
	{
		case ANIM_STAND:
			EVENT_InitNewSoldierAnim( pSoldier, WALKING, 0 , FALSE );
			break;

		case ANIM_PRONE:
			EVENT_InitNewSoldierAnim( pSoldier, CRAWLING, 0 , FALSE );
			break;

		case ANIM_CROUCH:
			EVENT_InitNewSoldierAnim( pSoldier, SWATTING, 0 , FALSE );
			break;

	}

}


static ETRLEObject const& GetActualSoldierAnimDims(SOLDIERTYPE const* const s)
{
	static ETRLEObject const fallback = { 0, 0, 0, 0, 5, 5 };

	UINT16 const anim_surface = GetSoldierAnimationSurface(s);
	if (anim_surface == INVALID_ANIMATION_SURFACE) return fallback;

	SGPVObject const* vo = gAnimSurfaceDatabase[anim_surface].hVideoObject;
	if (!vo) return fallback;

	// XXX comment seems wrong
	// OK, noodle here on what we should do... If we take each frame, it will be
	// different slightly depending on the frame and the value returned here will
	// vary thusly. However, for the uses of this function, we should be able to
	// use just the first frame...
	return vo->SubregionProperties(s->usAniFrame);
}


UINT16 GetStructureID(SOLDIERTYPE const * const pSoldier)
{
	if (pSoldier->pLevelNode && pSoldier->pLevelNode->pStructureData)
	{
		return pSoldier->pLevelNode->pStructureData->usStructureID;
	}

	return INVALID_STRUCTURE_ID;
}


static void SetSoldierLocatorOffsets(SOLDIERTYPE* const s)
{
	// OK, from our animation, get height, width
	ETRLEObject const& dims = GetActualSoldierAnimDims(s);
	s->sBoundingBoxWidth   = dims.usWidth;
	s->sBoundingBoxHeight  = dims.usHeight;
	s->sBoundingBoxOffsetX = dims.sOffsetX;
	s->sBoundingBoxOffsetY = dims.sOffsetY;
}


static BOOLEAN SoldierCarriesTwoHandedWeapon(SOLDIERTYPE* pSoldier)
{
	UINT16 usItem;

	usItem = pSoldier->inv[ HANDPOS ].usItem;

	if ( usItem != NOTHING && (GCM->getItem(usItem)->isTwoHanded()) )
	{
		return( TRUE );
	}

	return( FALSE );
}


static void SoldierBleed(SOLDIERTYPE* pSoldier, BOOLEAN fBandagedBleed);


static INT32 CheckBleeding(SOLDIERTYPE* pSoldier)
{
	INT8  bBandaged; //,savedOurTurn;
	INT32 iBlood = NOBLOOD;

	if ( pSoldier->bLife != 0 )
	{
		// if merc is hurt beyond the minimum required to bleed, or he's dying
		if ( ( pSoldier->bBleeding > MIN_BLEEDING_THRESHOLD) || pSoldier->bLife < OKLIFE )
		{
			// if he's NOT in the process of being bandaged or DOCTORed
			if ( ( pSoldier->ubServiceCount == 0 ) && ( AnyDoctorWhoCanHealThisPatient( pSoldier, HEALABLE_EVER ) == NULL ) )
			{
				// may drop blood whether or not any bleeding takes place this turn
				if ( pSoldier->bTilesMoved < 1 )
				{
					iBlood = ( ( pSoldier->bBleeding - MIN_BLEEDING_THRESHOLD ) / BLOODDIVISOR ); // + pSoldier->dying;
					if ( iBlood > MAXBLOODQUANTITY )
					{
						iBlood = MAXBLOODQUANTITY;
					}
				}
				else
				{
					iBlood = NOBLOOD;
				}

				// Are we in a different mode?
				if (!(gTacticalStatus.uiFlags & INCOMBAT))
				{
					pSoldier->dNextBleed -= (FLOAT)RT_NEXT_BLEED_MODIFIER;
				}
				else
				{
					// Do a single step descrease
					pSoldier->dNextBleed--;
				}

				// if it's time to lose some blood
				if ( pSoldier->dNextBleed <= 0)
				{
					// first, calculate if soldier is bandaged
					bBandaged = pSoldier->bLifeMax - pSoldier->bBleeding - pSoldier->bLife;

					// as long as he's bandaged and not "dying"
					if ( bBandaged && pSoldier->bLife >= OKLIFE )
					{
						// just bleeding through existing bandages
						pSoldier->bBleeding++;

						SoldierBleed( pSoldier, TRUE );
					}
					else // soldier is either not bandaged at all or is dying
					{
						if ( pSoldier->bLife < OKLIFE ) // if he's dying
						{
							// if he's conscious, and he hasn't already, say his "dying quote"
							if ( ( pSoldier->bLife >= CONSCIOUSNESS ) && !pSoldier->fDyingComment )
							{
								TacticalCharacterDialogue( pSoldier, QUOTE_SERIOUSLY_WOUNDED );

								pSoldier->fDyingComment = TRUE;
							}

							// can't permit lifemax to ever bleed beneath OKLIFE, or that
							// soldier might as well be dead!
							if (pSoldier->bLifeMax >= OKLIFE)
							{
								// bleeding while "dying" costs a PERMANENT point of life each time!
								pSoldier->bLifeMax--;
								pSoldier->bBleeding--;
							}
						}
					}

					// either way, a point of life (health) is lost because of bleeding
					// This will also update the life bar

					SoldierBleed( pSoldier, FALSE );


					// if he's not dying (which includes him saying the dying quote just
					// now), and he hasn't warned us that he's bleeding yet, he does so
					// Also, not if they are being bandaged....
					if ( ( pSoldier->bLife >= OKLIFE ) && !pSoldier->fDyingComment && !pSoldier->fWarnedAboutBleeding && !gTacticalStatus.fAutoBandageMode && pSoldier->ubServiceCount == 0 )
					{
						TacticalCharacterDialogue( pSoldier, QUOTE_STARTING_TO_BLEED );

						// "starting to bleed" quote
						pSoldier->fWarnedAboutBleeding = TRUE;
					}

					pSoldier->dNextBleed = CalcSoldierNextBleed( pSoldier );

				}
			}
		}
	}
	return( iBlood );
}


void FlashSoldierPortrait(SOLDIERTYPE* const s)
{
	s->fFlashPortrait      = FLASH_PORTRAIT_START;
	s->bFlashPortraitFrame = FLASH_PORTRAIT_STARTSHADE;
	RESETTIMECOUNTER(s->PortraitFlashCounter, FLASH_PORTRAIT_DELAY);
}


static void SoldierBleed(SOLDIERTYPE* const s, const BOOLEAN fBandagedBleed)
{
	// OK, here make some stuff happen for bleeding
	// A banaged bleed does not show damage taken, just through existing bandages

	if (guiCurrentScreen != GAME_SCREEN || s->bInSector)
	{
		FlashSoldierPortrait(s);
		if (guiCurrentScreen == MAP_SCREEN) SetInfoChar(s);
	}

	if (!fBandagedBleed)
	{
		SoldierTakeDamage(s, 1, 100, TAKE_DAMAGE_BLOODLOSS, NULL);
	}
}


void SoldierCollapse( SOLDIERTYPE *pSoldier )
{
	BOOLEAN fMerc = FALSE;

	if ( pSoldier->ubBodyType <= REGFEMALE )
	{
		fMerc = TRUE;
	}

	// If we are an animal, etc, don't do anything....
	switch( pSoldier->ubBodyType )
	{
		case ADULTFEMALEMONSTER:
		case AM_MONSTER:
		case YAF_MONSTER:
		case YAM_MONSTER:
		case LARVAE_MONSTER:
		case INFANT_MONSTER:
		case QUEENMONSTER:

			// Give breath back....
			DeductPoints( pSoldier,0, (INT16)-5000 );
			return;
	}

	pSoldier->bCollapsed = TRUE;

	ReceivingSoldierCancelServices( pSoldier );

	// CC has requested - handle sight here...
	HandleSight(*pSoldier, SIGHT_LOOK);

	// Check height
	switch( gAnimControl[ pSoldier->usAnimState ].ubEndHeight )
	{
		case ANIM_STAND:

			if ( pSoldier->bOverTerrainType == DEEP_WATER )
			{
				EVENT_InitNewSoldierAnim( pSoldier, DEEP_WATER_DIE, 0, FALSE );
			}
			else if ( pSoldier->bOverTerrainType == LOW_WATER )
			{
				EVENT_InitNewSoldierAnim( pSoldier, WATER_DIE, 0, FALSE );
			}
			else
			{
				BeginTyingToFall( pSoldier );
				EVENT_InitNewSoldierAnim( pSoldier, FALLFORWARD_FROMHIT_STAND, 0, FALSE );
			}
			break;

		case ANIM_CROUCH:

			// Crouched or prone, only for mercs!
			BeginTyingToFall( pSoldier );

			if ( fMerc )
			{
				EVENT_InitNewSoldierAnim( pSoldier, FALLFORWARD_FROMHIT_CROUCH, 0 , FALSE);
			}
			else
			{
				// For civs... use fall from stand...
				EVENT_InitNewSoldierAnim( pSoldier, FALLFORWARD_FROMHIT_STAND, 0 , FALSE);
			}
			break;

		case ANIM_PRONE:

			switch( pSoldier->usAnimState )
			{
				case FALLFORWARD_FROMHIT_STAND:
				case ENDFALLFORWARD_FROMHIT_CROUCH:

					ChangeSoldierState( pSoldier, STAND_FALLFORWARD_STOP, 0, FALSE );
					break;

				case FALLBACK_HIT_STAND:
					ChangeSoldierState( pSoldier, FALLBACKHIT_STOP, 0, FALSE );
					break;

				default:
					EVENT_InitNewSoldierAnim( pSoldier, PRONE_LAY_FROMHIT, 0 , FALSE );
					break;
			}
			break;
	}

	if (pSoldier->uiStatusFlags & SOLDIER_ENEMY)
	{
		INT8 bPanicTrigger = ClosestPanicTrigger(pSoldier);
		if (bPanicTrigger != -1 && !gTacticalStatus.bPanicTriggerIsAlarm[bPanicTrigger] && gTacticalStatus.the_chosen_one == pSoldier)
		{
			// replace this guy as the chosen one!
			gTacticalStatus.the_chosen_one = NULL;
			MakeClosestEnemyChosenOne();
		}

		if ((gTacticalStatus.uiFlags & INCOMBAT) && (pSoldier->uiStatusFlags & SOLDIER_UNDERAICONTROL))
		{
			SLOGD("Ending turn for {} because of error from HandleItem", pSoldier->ubID);
			EndAIGuysTurn(*pSoldier);
		}
	}
}


static FLOAT CalcSoldierNextBleed(SOLDIERTYPE* pSoldier)
{
	INT8 bBandaged;

	// calculate how many turns before he bleeds again
	// bleeding faster the lower life gets, and if merc is running around
	//pSoldier->nextbleed = 2 + (pSoldier->life / (10 + pSoldier->tilesMoved));  // min = 2

	// if bandaged, give 1/2 of the bandaged life points back into equation
	bBandaged = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;

	return( (FLOAT)1 + (FLOAT)( (pSoldier->bLife + bBandaged / 2) / (10 + pSoldier->bTilesMoved) ) );  // min = 1
}


void HandlePlacingRoofMarker(SOLDIERTYPE& s, bool const set, bool const force)
{
	if (set && s.bVisible == -1) return;

	if (s.bLevel != SECOND_LEVEL) return;
	// We are on the roof, add roof UI piece

	// Return if we are still climbing roof
	if (!force && s.usAnimState == CLIMBUPROOF) return;

	GridNo const gridno = s.sGridNo;
	if (!gpWorldLevelData[gridno].pRoofHead) return;

	if (!set)
	{
		RemoveRoof(gridno, FIRSTPOINTERS11);
	}
	else if (!IndexExistsInRoofLayer(gridno, FIRSTPOINTERS11))
	{
		// It does not exist already
		LEVELNODE* const l = AddRoofToTail(gridno, FIRSTPOINTERS11);
		l->ubShadeLevel        = DEFAULT_SHADE_LEVEL;
		l->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
	}
}


void PositionSoldierLight( SOLDIERTYPE *pSoldier )
{
	// DO ONLY IF WE'RE AT A GOOD LEVEL
	if ( ubAmbientLightLevel < MIN_AMB_LEVEL_FOR_MERC_LIGHTS )
	{
		return;
	}

	if ( !pSoldier->bInSector )
	{
		return;
	}

	if ( pSoldier->bTeam != OUR_TEAM )
	{
		return;
	}

	if ( pSoldier->bLife < OKLIFE )
	{
		return;
	}

	//if the player DOESNT want the merc to cast light
	if( !gGameSettings.fOptions[ TOPTION_MERC_CASTS_LIGHT ] )
	{
		return;
	}

	if (pSoldier->light == NULL) CreateSoldierLight(pSoldier);

	LIGHT_SPRITE* const l = pSoldier->light;
	LightSpritePower(l, TRUE);
	LightSpriteFake(l);
	LightSpritePosition(l, pSoldier->sX / CELL_X_SIZE, pSoldier->sY / CELL_Y_SIZE);
}


void PickPickupAnimation( SOLDIERTYPE *pSoldier, INT32 iItemIndex, INT16 sGridNo, INT8 bZLevel )
{
	INT8      bDirection;
	STRUCTURE *pStructure;
	BOOLEAN   fDoNormalPickup = TRUE;


	// OK, Given the gridno, determine if it's the same one or different....
	if ( sGridNo != pSoldier->sGridNo )
	{
		// Get direction to face....
		bDirection = (INT8)GetDirectionFromGridNo( sGridNo, pSoldier );
		pSoldier->ubPendingDirection = bDirection;

		// Change to pickup animation
		EVENT_InitNewSoldierAnim( pSoldier, ADJACENT_GET_ITEM, 0 , FALSE );

		if (!(pSoldier->uiStatusFlags & SOLDIER_PC))
		{
			// set "pending action" value for AI so it will wait
			pSoldier->bAction = AI_ACTION_PENDING_ACTION;
		}

	}
	else
	{
		// If in water....
		if ( MercInWater( pSoldier ) )
		{
			UnSetUIBusy(pSoldier);
			HandleSoldierPickupItem( pSoldier, iItemIndex, sGridNo, bZLevel );
			SoldierGotoStationaryStance( pSoldier );
			if (!(pSoldier->uiStatusFlags & SOLDIER_PC))
			{
				// reset action value for AI because we're done!
				ActionDone( pSoldier );
			}

		}
		else
		{
			// Don't show animation of getting item, if we are not standing
			switch ( gAnimControl[ pSoldier->usAnimState ].ubHeight )
			{
				case ANIM_STAND:

					// OK, if we are looking at z-level >0, AND
					// we have a strucxture with items in it
					// look for orientation and use angle accordingly....
					if ( bZLevel > 0 )
					{
//#if 0
						// Get direction to face....
						if ( ( pStructure = FindStructure( (INT16)sGridNo, ( STRUCTURE_HASITEMONTOP | STRUCTURE_OPENABLE ) ) ) != NULL )
						{
							fDoNormalPickup = FALSE;

							// OK, look at orientation
							switch( pStructure->ubWallOrientation )
							{
								case OUTSIDE_TOP_LEFT:
								case INSIDE_TOP_LEFT:

									bDirection = (INT8)NORTH;
									break;

								case OUTSIDE_TOP_RIGHT:
								case INSIDE_TOP_RIGHT:

									bDirection = (INT8)WEST;
									break;

								default:

									bDirection = pSoldier->bDirection;
									break;
							}

							//pSoldier->ubPendingDirection = bDirection;
							EVENT_SetSoldierDesiredDirection( pSoldier, bDirection );
							EVENT_SetSoldierDirection( pSoldier, bDirection );

							// Change to pickup animation
							EVENT_InitNewSoldierAnim( pSoldier, ADJACENT_GET_ITEM, 0 , FALSE );
						}
//#endif
					}

					if ( fDoNormalPickup )
					{
						EVENT_InitNewSoldierAnim( pSoldier, PICKUP_ITEM, 0 , FALSE );
					}

					if (!(pSoldier->uiStatusFlags & SOLDIER_PC))
					{
						// set "pending action" value for AI so it will wait
						pSoldier->bAction = AI_ACTION_PENDING_ACTION;
					}
					break;

				case ANIM_CROUCH:
				case ANIM_PRONE:
					UnSetUIBusy(pSoldier);
					HandleSoldierPickupItem( pSoldier, iItemIndex, sGridNo, bZLevel );
					SoldierGotoStationaryStance( pSoldier );
					if (!(pSoldier->uiStatusFlags & SOLDIER_PC))
					{
						// reset action value for AI because we're done!
						ActionDone( pSoldier );
					}
					break;
			}
		}
	}
}

void PickDropItemAnimation( SOLDIERTYPE *pSoldier )
{
	// Don't show animation of getting item, if we are not standing
	switch ( gAnimControl[ pSoldier->usAnimState ].ubHeight )
	{
		case ANIM_STAND:

			EVENT_InitNewSoldierAnim( pSoldier, DROP_ITEM, 0 , FALSE );
			break;

		case ANIM_CROUCH:
		case ANIM_PRONE:

			SoldierHandleDropItem( pSoldier );
			SoldierGotoStationaryStance( pSoldier );
			break;
	}
}


void EVENT_SoldierBeginCutFence( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection )
{
	// Make sure we have a structure here....
	if ( IsCuttableWireFenceAtGridNo( sGridNo ) )
	{
		// CHANGE DIRECTION AND GOTO ANIMATION NOW
		EVENT_SetSoldierDesiredDirectionForward(pSoldier, ubDirection);
		EVENT_SetSoldierDirection( pSoldier, ubDirection );

		//BOOLEAN CutWireFence( INT16 sGridNo )

		// SET TARGET GRIDNO
		pSoldier->sTargetGridNo = sGridNo;

		// CHANGE TO ANIMATION
		EVENT_InitNewSoldierAnim( pSoldier, CUTTING_FENCE, 0 , FALSE );
	}
}


void EVENT_SoldierBeginRepair(SOLDIERTYPE& s, GridNo const gridno, UINT8 const direction)
{
	SOLDIERTYPE* tgt;
	INT8 const repair_item = IsRepairableStructAtGridNo(gridno, &tgt);
	if (repair_item == 0) return;

	// Change direction and goto animation now
	EVENT_SetSoldierDesiredDirection(&s, direction);
	EVENT_SetSoldierDirection(&s, direction);
	EVENT_InitNewSoldierAnim(&s, GOTO_REPAIRMAN, 0, FALSE);

	switch (repair_item) // Set mercs's assignment to repair
	{
		case 2: /* Vehicle */ SetSoldierAssignmentRepair(s, FALSE, tgt->ubID); break;
		default:              throw std::runtime_error(ST::string("unknown repair assignment: {}", repair_item).to_std_string());
	}
}


void EVENT_SoldierBeginRefuel( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection )
{
	// Make sure we have a structure here....
	if (GetRefuelableStructAtGridNo(sGridNo) != NULL)
	{
		// CHANGE DIRECTION AND GOTO ANIMATION NOW
		EVENT_SetSoldierDesiredDirection( pSoldier, ubDirection );
		EVENT_SetSoldierDirection( pSoldier, ubDirection );

		//BOOLEAN CutWireFence( INT16 sGridNo )

		// SET TARGET GRIDNO
		//pSoldier->sTargetGridNo = sGridNo;

		// CHANGE TO ANIMATION
		EVENT_InitNewSoldierAnim( pSoldier, REFUEL_VEHICLE, 0 , FALSE );
		// SET BUDDY'S ASSIGNMENT TO REPAIR...
	}
}


void EVENT_SoldierBeginTakeBlood( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection )
{
	// See if these is a corpse here....
	ROTTING_CORPSE const * const pCorpse = GetCorpseAtGridNo(sGridNo , pSoldier->bLevel);

	if ( pCorpse != NULL )
	{
		pSoldier->uiPendingActionData4 = pCorpse->ID();

		// CHANGE DIRECTION AND GOTO ANIMATION NOW
		EVENT_SetSoldierDesiredDirection( pSoldier, ubDirection );
		EVENT_SetSoldierDirection( pSoldier, ubDirection );

		EVENT_InitNewSoldierAnim( pSoldier, TAKE_BLOOD_FROM_CORPSE, 0 , FALSE );
	}
	else
	{
		// Say NOTHING quote...
		DoMercBattleSound( pSoldier, BATTLE_SOUND_NOTHING );
	}
}


void EVENT_SoldierBeginAttachCan( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection )
{
	STRUCTURE   *pStructure;
	DOOR_STATUS *pDoorStatus;

	// OK, find door, attach to door, do animation...., remove item....

	// First make sure we still have item in hand....
	if ( pSoldier->inv[ HANDPOS ].usItem != STRING_TIED_TO_TIN_CAN )
	{
		return;
	}

	pStructure = FindStructure( sGridNo, STRUCTURE_ANYDOOR );

	if ( pStructure == NULL )
	{
		return;
	}

	// Modify door status to make sure one is created for this door
	// Use the current door state for this
	if ( !(pStructure->fFlags & STRUCTURE_OPEN) )
	{
		ModifyDoorStatus( sGridNo, FALSE, FALSE );
	}
	else
	{
		ModifyDoorStatus( sGridNo, TRUE, TRUE );
	}

	// Now get door status...
	pDoorStatus = GetDoorStatus( sGridNo );
	if ( pDoorStatus == NULL )
	{
		// SOmething wrong here...
		return;
	}

	// OK set flag!
	pDoorStatus->ubFlags |= DOOR_HAS_TIN_CAN;

	// Do animation
	EVENT_SetSoldierDesiredDirection( pSoldier, ubDirection );
	EVENT_SetSoldierDirection( pSoldier, ubDirection );

	EVENT_InitNewSoldierAnim( pSoldier, ATTACH_CAN_TO_STRING, 0 , FALSE );

	// Remove item...
	DeleteObj( &(pSoldier->inv[ HANDPOS ] ) );
	fInterfacePanelDirty = DIRTYLEVEL2;

}


void EVENT_SoldierBeginReloadRobot( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection, UINT8 ubMercSlot )
{
	// Make sure we have a robot here....
	const SOLDIERTYPE* const tgt = WhoIsThere2(sGridNo, pSoldier->bLevel);
	if (tgt != NULL && tgt->uiStatusFlags & SOLDIER_ROBOT)
	{
		// CHANGE DIRECTION AND GOTO ANIMATION NOW
		EVENT_SetSoldierDesiredDirection( pSoldier, ubDirection );
		EVENT_SetSoldierDirection( pSoldier, ubDirection );

		// CHANGE TO ANIMATION
		EVENT_InitNewSoldierAnim( pSoldier, RELOAD_ROBOT, 0 , FALSE );

	}
}


static void ChangeToFlybackAnimation(SOLDIERTYPE* pSoldier, INT8 bDirection)
{
	UINT16 usNewGridNo;

	// Get dest gridno, convert to center coords
	usNewGridNo = NewGridNo(pSoldier->sGridNo, DirectionInc(OppositeDirection(bDirection)));
	usNewGridNo = NewGridNo(usNewGridNo,       DirectionInc(OppositeDirection(bDirection)));

	Soldier{pSoldier}.removePendingAction();

	// Set path....
	pSoldier->ubPathDataSize = 0;
	pSoldier->ubPathIndex    = 0;
	pSoldier->ubPathingData[pSoldier->ubPathDataSize] = OppositeDirection(pSoldier->bDirection);
	pSoldier->ubPathDataSize++;
	pSoldier->ubPathingData[pSoldier->ubPathDataSize] = OppositeDirection(pSoldier->bDirection);
	pSoldier->ubPathDataSize++;
	pSoldier->sFinalDestination = usNewGridNo;
	EVENT_InternalSetSoldierDestination( pSoldier, pSoldier->ubPathingData[ pSoldier->ubPathIndex ], FALSE, FLYBACK_HIT );

	// Get a new direction based on direction
	EVENT_InitNewSoldierAnim( pSoldier, FLYBACK_HIT, 0 , FALSE );
}

void ChangeToFallbackAnimation( SOLDIERTYPE *pSoldier, INT8 bDirection )
{
	UINT16 usNewGridNo;

	// Get dest gridno, convert to center coords
	usNewGridNo = NewGridNo(pSoldier->sGridNo, DirectionInc(OppositeDirection(bDirection)));
	//usNewGridNo = NewGridNo( (UINT16)usNewGridNo, (UINT16)(-1 * DirectionInc( bDirection ) ) );

	Soldier{pSoldier}.removePendingAction();

	// Set path....
	pSoldier->ubPathDataSize = 0;
	pSoldier->ubPathIndex    = 0;
	pSoldier->ubPathingData[pSoldier->ubPathDataSize] = OppositeDirection(pSoldier->bDirection);
	pSoldier->ubPathDataSize++;
	pSoldier->sFinalDestination = usNewGridNo;
	EVENT_InternalSetSoldierDestination( pSoldier, pSoldier->ubPathingData[ pSoldier->ubPathIndex ], FALSE, FALLBACK_HIT_STAND );

	// Get a new direction based on direction
	EVENT_InitNewSoldierAnim( pSoldier, FALLBACK_HIT_STAND, 0 , FALSE );
}


void SetSoldierCowerState( SOLDIERTYPE *pSoldier, BOOLEAN fOn )
{
	// Robot's don't cower!
	if ( pSoldier->ubBodyType == ROBOTNOWEAPON )
	{
		SLOGW("Robot was told to cower!");
		return;
	}

	// OK< set flag and do anim...
	if ( fOn )
	{
		if ( !( pSoldier->uiStatusFlags & SOLDIER_COWERING ) )
		{
			EVENT_InitNewSoldierAnim( pSoldier, START_COWER, 0, FALSE );

			pSoldier->uiStatusFlags |= SOLDIER_COWERING;

			pSoldier->ubDesiredHeight = ANIM_CROUCH;
		}
	}
	else
	{
		if ( (pSoldier->uiStatusFlags & SOLDIER_COWERING) )
		{
			EVENT_InitNewSoldierAnim( pSoldier, END_COWER, 0, FALSE );

			pSoldier->uiStatusFlags &= (~SOLDIER_COWERING );

			pSoldier->ubDesiredHeight = ANIM_STAND;
		}
	}
}


void MercStealFromMerc(SOLDIERTYPE* const pSoldier, const SOLDIERTYPE* const pTarget)
{
	INT16 sActionGridNo, sGridNo, sAdjustedGridNo;
	UINT8	ubDirection;

	// OK, find an adjacent gridno....
	sGridNo = pTarget->sGridNo;

	// See if we can get there to punch
	sActionGridNo =  FindAdjacentGridEx( pSoldier, sGridNo, &ubDirection, &sAdjustedGridNo, TRUE, FALSE );
	if ( sActionGridNo != -1 )
	{
		Soldier{pSoldier}.setPendingAction(MERC_STEAL, pTarget->sGridNo, ubDirection);
		pSoldier->bTargetLevel = pTarget->bLevel;

		// CHECK IF WE ARE AT THIS GRIDNO NOW
		if ( pSoldier->sGridNo != sActionGridNo )
		{
			// WALK UP TO DEST FIRST
			SendGetNewSoldierPathEvent(pSoldier, sActionGridNo);
		}
		else
		{
			EVENT_SetSoldierDesiredDirection( pSoldier, ubDirection );
			EVENT_InitNewSoldierAnim( pSoldier, STEAL_ITEM, 0 , FALSE );
		}

		// OK, set UI
		gTacticalStatus.ubAttackBusyCount++;
		// reset attacking item (hand)
		pSoldier->usAttackingWeapon = 0;
		SLOGD("Starting STEAL attack, attack count now {}",
			gTacticalStatus.ubAttackBusyCount);

		SetUIBusy(pSoldier);
	}
}

BOOLEAN PlayerSoldierStartTalking( SOLDIERTYPE *pSoldier, UINT8 ubTargetID, BOOLEAN fValidate )
{
	UINT32 uiRange;

	if ( ubTargetID == NOBODY )
	{
		return( FALSE );
	}

	SOLDIERTYPE& tgt = GetMan(ubTargetID);

	// Check distance again, to be sure
	if ( fValidate )
	{
		// OK, since we locked this guy from moving
		// we should be close enough, so talk ( unless he is now dead )
		if (!IsValidTalkableNPC(&tgt, FALSE, FALSE, FALSE)) return FALSE;

		uiRange = GetRangeFromGridNoDiff(pSoldier->sGridNo, tgt.sGridNo);

		if ( uiRange > ( NPC_TALK_RADIUS * 2 ) )
		{
			// Todo here - should we follow dude?
			return( FALSE );
		}


	}

	// Get APs...
	const INT16 sAPCost = AP_TALK;

	// Deduct points from our guy....
	DeductPoints( pSoldier, sAPCost, 0 );

	INT16 const sFacingDir = GetDirectionFromGridNo(tgt.sGridNo, pSoldier);

	// Set our guy facing
	SendSoldierSetDesiredDirectionEvent( pSoldier, sFacingDir );

	// Set NPC facing
	SendSoldierSetDesiredDirectionEvent(&tgt, OppositeDirection(sFacingDir));

	// Stop our guys...
	EVENT_StopMerc(pSoldier);

	// ATE; Check for normal civs...
	if (GetCivType(&tgt) != CIV_TYPE_NA)
	{
		StartCivQuote(&tgt);
		return( FALSE );
	}


	// Are we an EPC that is being escorted?
	if (tgt.ubProfile != NO_PROFILE && tgt.ubWhatKindOfMercAmI == MERC_TYPE__EPC)
	{
		return InitiateConversation(&tgt, pSoldier, APPROACH_EPC_WHO_IS_RECRUITED);
	}
	else if (tgt.bNeutral)
	{
		switch (tgt.ubProfile)
		{
			case JIM:
			case JACK:
			case OLAF:
			case RAY:
			case OLGA:
			case TYRONE:
				// Start combat etc
				DeleteTalkingMenu();
				CancelAIAction(&tgt);
				AddToShouldBecomeHostileOrSayQuoteList(&tgt);
				break;
			default:
				// Start talking!
				return InitiateConversation(&tgt, pSoldier, NPC_INITIAL_QUOTE);
		}
	}
	else
	{
		// Start talking with hostile NPC
		return InitiateConversation(&tgt, pSoldier, APPROACH_ENEMY_NPC_QUOTE);
	}

	return( TRUE );
}


bool IsValidSecondHandShot(SOLDIERTYPE const* const s)
{
	OBJECTTYPE const& o = s->inv[SECONDHANDPOS];
	return GCM->getItem(o.usItem)->getItemClass() == IC_GUN &&
		!(GCM->getItem(o.usItem)->isTwoHanded()) &&
		!s->bDoBurst &&
		GCM->getItem(s->inv[HANDPOS].usItem)->getItemClass() == IC_GUN &&
		o.bGunStatus >= USABLE &&
		o.ubGunShotsLeft > 0;
}


bool IsValidSecondHandShotForReloadingPurposes(SOLDIERTYPE const* const s)
{
	// Should be maintained as same as function above with line about ammo taken
	// out!
	OBJECTTYPE const& o = s->inv[SECONDHANDPOS];
	return GCM->getItem(o.usItem)->getItemClass() == IC_GUN &&
		!s->bDoBurst &&
		GCM->getItem(s->inv[HANDPOS].usItem)->getItemClass() == IC_GUN &&
		o.bGunStatus >= USABLE;
}


BOOLEAN CanRobotBeControlled(const SOLDIERTYPE* const robot)
{
	Assert(robot->uiStatusFlags & SOLDIER_ROBOT);
	const SOLDIERTYPE* const controller = robot->robot_remote_holder;
	if (controller == NULL) return FALSE;
	return ControllingRobot(controller);
}


BOOLEAN ControllingRobot(const SOLDIERTYPE* s)
{
	if (!s->bActive) return FALSE;

	// EPCs can't control the robot (no inventory to hold remote, for one)
	if (AM_AN_EPC(s)) return FALSE;

	// Don't require s->bInSector here, it must work from mapscreen!

	// are we in ok shape?
	if (s->bLife < OKLIFE || s->bTeam != OUR_TEAM)
	{
		return( FALSE );
	}

	// allow control from within vehicles - allows strategic travel in a vehicle with robot!
	if (s->bAssignment >= ON_DUTY && s->bAssignment != VEHICLE)
	{
		return( FALSE );
	}

	// is the soldier wearing a robot remote control?
	if (!IsWearingHeadGear(*s, ROBOT_REMOTE_CONTROL))
	{
		return( FALSE );
	}

	// Find the robot
	const SOLDIERTYPE* const pRobot = FindSoldierByProfileIDOnPlayerTeam(ROBOT);
	if ( !pRobot )
	{
		return( FALSE );
	}

	// Are we in the same sector....?
	// ARM: CHANGED TO WORK IN MAPSCREEN, DON'T USE WorldSector HERE
	if (pRobot->sSector == s->sSector)
	{
		// they have to be either both in sector, or both on the road
		if (pRobot->fBetweenSectors == s->fBetweenSectors)
		{
			// if they're on the road...
			if ( pRobot->fBetweenSectors )
			{
				// they have to be in the same squad or vehicle
				if (pRobot->bAssignment != s->bAssignment) return FALSE;

				// if in a vehicle, must be the same vehicle
				if (pRobot->bAssignment == VEHICLE && pRobot->iVehicleId != s->iVehicleId)
				{
					return( FALSE );
				}
			}

			// all OK!
			return( TRUE );
		}
	}

	return( FALSE );
}


const SOLDIERTYPE *GetRobotController( const SOLDIERTYPE *pSoldier )
{
	return pSoldier->robot_remote_holder;
}


void UpdateRobotControllerGivenRobot( SOLDIERTYPE *pRobot )
{
	// Loop through guys and look for a controller!
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (ControllingRobot(s))
		{
			pRobot->robot_remote_holder = s;
			return;
		}
	}

	pRobot->robot_remote_holder = NULL;
}


void UpdateRobotControllerGivenController( SOLDIERTYPE *pSoldier )
{
	// First see if are still controlling the robot
	if ( !ControllingRobot( pSoldier ) )
	{
		return;
	}

	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s->uiStatusFlags & SOLDIER_ROBOT)
		{
			s->robot_remote_holder = pSoldier;
		}
	}
}


static void HandleSoldierTakeDamageFeedback(SOLDIERTYPE* const s)
{
	// Do sound.....
	// ATE: Limit how often we grunt...
	if (GetJA2Clock() - s->uiTimeSinceLastBleedGrunt > 1000)
	{
		s->uiTimeSinceLastBleedGrunt = GetJA2Clock();
		DoMercBattleSound(s, BATTLE_SOUND_HIT1);
	}

	FlashSoldierPortrait(s);
}


void HandleSystemNewAISituation(SOLDIERTYPE* const pSoldier)
{
	// Are we an AI guy?
	if ( gTacticalStatus.ubCurrentTeam != OUR_TEAM && pSoldier->bTeam != OUR_TEAM )
	{
		if ( pSoldier->bNewSituation == IS_NEW_SITUATION )
		{
			// Cancel what they were doing....
			pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
			pSoldier->fTurningFromPronePosition = FALSE;
			pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
			Soldier{pSoldier}.removePendingAction();
			pSoldier->bEndDoorOpenCode = 0;

			// if this guy isn't under direct AI control, WHO GIVES A FLYING FLICK?
			if ( pSoldier->uiStatusFlags & SOLDIER_UNDERAICONTROL )
			{
				if ( pSoldier->fTurningToShoot )
				{
					pSoldier->fTurningToShoot = FALSE;
					// Release attacker
					// OK - this is hightly annoying , but due to the huge combinations of
					// things that can happen - 1 of them is that sLastTarget will get unset
					// after turn is done - so set flag here to tell it not to...
					pSoldier->fDontUnsetLastTargetFromTurn = TRUE;
					SLOGD("Reducing attacker busy count..., ending fire because saw something: DONE IN SYSTEM NEW SITUATION");
						ReduceAttackBusyCount(pSoldier, FALSE);
				}

				if ( pSoldier->pTempObject != NULL )
				{
					// Place it back into inv....
					AutoPlaceObject( pSoldier, pSoldier->pTempObject, FALSE );
					delete pSoldier->pTempObject;
					pSoldier->pTempObject        = NULL;
					pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;

					// Decrement attack counter...
					SLOGD("Reducing attacker busy count..., ending throw because saw something: DONE IN SYSTEM NEW SITUATION");
						ReduceAttackBusyCount(pSoldier, FALSE);
				}
			}
		}
	}
}


static void InternalPlaySoldierFootstepSound(SOLDIERTYPE* const s)
{
	// Determine if we are on the floor
	if (s->uiStatusFlags & SOLDIER_VEHICLE) return;

	INT8 volume = MIDVOLUME;
	if (s->usAnimState == HOPFENCE) volume = HIGHVOLUME;

	if (s->uiStatusFlags & SOLDIER_ROBOT)
	{
		PlaySoldierJA2Sample(s, ROBOT_BEEP, volume, 1, TRUE);
		return;
	}

	// Assume outside
	SoundID sound_base = WALK_LEFT_OUT;
	UINT8   random_max = 4;
	if (s->usAnimState == CRAWLING)
	{
		sound_base = CRAWL_1;
	}
	else switch (s->bOverTerrainType) // Pick base based on terrain over
	{
		case FLAT_FLOOR: sound_base = WALK_LEFT_IN; break;
		case DIRT_ROAD:
		case PAVED_ROAD: sound_base = WALK_LEFT_ROAD; break;
		case LOW_WATER:
		case MED_WATER:  sound_base = WATER_WALK1_IN; random_max = 2; break;
		case DEEP_WATER: sound_base = SWIM_1;         random_max = 2; break;
	}

	// Pick a random sound
	UINT8 random_snd;
	do
	{
		random_snd = Random(random_max);
	}
	while (random_snd == s->ubLastFootPrintSound);

	s->ubLastFootPrintSound = random_snd;

	// If in realtime, don't play at full volume, because too many people walking
	// around sounds don't sound good (unless we are the selected guy, then always
	// play at reg volume)
	if (!(gTacticalStatus.uiFlags & INCOMBAT) && s != GetSelectedMan())
	{
		volume = LOWVOLUME;
	}

	PlaySoldierJA2Sample(s, static_cast<SoundID>(sound_base + random_snd), volume, 1, TRUE);
}


void PlaySoldierFootstepSound(SOLDIERTYPE* pSoldier)
{
	// normally, not in stealth mode
	if ( !pSoldier->bStealthMode )
	{
		InternalPlaySoldierFootstepSound( pSoldier );
	}
}


void PlayStealthySoldierFootstepSound(SOLDIERTYPE* pSoldier)
{
	// even if in stealth mode
	InternalPlaySoldierFootstepSound( pSoldier );
}


void CrowsFlyAway(const UINT8 ubTeam)
{
	FOR_EACH_IN_TEAM(s, ubTeam)
	{
		if (s->bInSector &&
			s->ubBodyType  == CROW &&
			s->usAnimState != CROW_FLY)
		{
			// fly away even if not seen!
			HandleCrowFlyAway(s);
		}
	}
}


void BeginTyingToFall( SOLDIERTYPE *pSoldier )
{
	pSoldier->bStartFallDir = pSoldier->bDirection;
	pSoldier->fTryingToFall = TRUE;

	// Randomize direction
	if ( Random( 50 ) < 25 )
	{
		pSoldier->fFallClockwise = TRUE;
	}
	else
	{
		pSoldier->fFallClockwise = FALSE;
	}
}

void SetSoldierAsUnderAiControl( SOLDIERTYPE *pSoldierToSet )
{
	if ( pSoldierToSet == NULL )
	{
		return;
	}

	FOR_EACH_SOLDIER(s)
	{
		s->uiStatusFlags &= ~SOLDIER_UNDERAICONTROL;
	}

	pSoldierToSet->uiStatusFlags |= SOLDIER_UNDERAICONTROL;
}


static void EnableDisableSoldierLightEffects(BOOLEAN fEnableLights);


void HandlePlayerTogglingLightEffects( BOOLEAN fToggleValue )
{
	if( fToggleValue )
	{
		//Toggle light status
		gGameSettings.fOptions[ TOPTION_MERC_CASTS_LIGHT ] ^= TRUE;
	}

	//Update all the mercs in the sector
	EnableDisableSoldierLightEffects( gGameSettings.fOptions[ TOPTION_MERC_CASTS_LIGHT ] );

	SetRenderFlags(RENDER_FLAG_FULL);
}


static void EnableDisableSoldierLightEffects(BOOLEAN const enable_lights)
{
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (!s->bInSector)     continue;
		if (s->bLife < OKLIFE) continue;
		// att: NO Soldier Lights if in a VEHICLE
		if (s->bAssignment == VEHICLE) continue;

		if (enable_lights)
		{
			// Add the light around the merc
			PositionSoldierLight(s);
		}
		else
		{
			// Delete the fake light the merc casts
			DeleteSoldierLight(s);
			// Light up the merc though
			SetSoldierPersonalLightLevel(s);
		}
	}
}


static void SetSoldierPersonalLightLevel(SOLDIERTYPE* const s)
{
	if (!s || s->sGridNo == NOWHERE) return;
	// The light level for the soldier
	LEVELNODE& n = *gpWorldLevelData[s->sGridNo].pMercHead;
	n.ubShadeLevel        = 3;
	n.ubSumLights         = 5;
	n.ubMaxLights         = 5;
	n.ubNaturalShadeLevel = 5;
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(SoldierControl, asserts)
{
	EXPECT_EQ(lengthof(gubMaxActionPoints), static_cast<size_t>(TOTALBODYTYPES));
	EXPECT_EQ(sizeof(KEY_ON_RING), 2u);
}

UINT8 oldatan8( INT16 sXPos, INT16 sYPos, INT16 sXPos2, INT16 sYPos2 )
{
	DOUBLE  test_x =  sXPos2 - sXPos;
	DOUBLE  test_y =  sYPos2 - sYPos;
	UINT8	  mFacing = WEST;
	DOUBLE angle;

	if ( test_x == 0 )
	{
		test_x = 0.04;
	}

	angle = atan2( test_x, test_y );

	do
	{
		if ( angle >=-PI*.375 && angle <= -PI*.125 )
		{
			mFacing = SOUTHWEST;
			break;
		}

		if ( angle <= PI*.375 && angle >= PI*.125 )
		{
			mFacing = SOUTHEAST;
			break;
		}

		if ( angle >=PI*.623 && angle <= PI*.875 )
		{
			mFacing = NORTHEAST;
			break;
		}

		if ( angle <=-PI*.623 && angle >= -PI*.875 )
		{
			mFacing = NORTHWEST;
			break;
		}

		if ( angle >-PI*0.125 && angle < PI*0.125 )
		{
			mFacing = SOUTH;
		}
		if ( angle > PI*0.375 && angle < PI*0.623 )
		{
			mFacing = EAST;
		}
		if ( ( angle > PI*0.875 && angle <= PI ) || ( angle > -PI && angle < -PI*0.875 ) )
		{
			mFacing = NORTH;
		}
		if ( angle > -PI*0.623 && angle < -PI*0.375 )
		{
			mFacing = WEST;
		}

	} while( FALSE );

	return( mFacing );
}

TEST(SoldierControl, atan8)
{
	struct {
		INT16 x2, y2;
		UINT8 expectedResult;
	} TestTable[]
	{
		{ 50, 0, EAST },
		{ 50, 50, SOUTHEAST },
		{ 0, 50, SOUTH },
		{ -50, 50, SOUTHWEST },
		{ -50, 0, WEST },
		{ -50, -50, NORTHWEST },
		{ 0, -50, NORTH },
		{ 50, -50, NORTHEAST},
	};

	for (auto t : TestTable)
	{
		EXPECT_EQ(atan8(0, 0, t.x2, t.y2), t.expectedResult);
		// Shifting both points equally must not make a difference
		EXPECT_EQ(atan8(5000, 5000, 5000 + t.x2, 5000 + t.y2), t.expectedResult);
		EXPECT_EQ(atan8(-5000, -5000, -5000 + t.x2, -5000 + t.y2), t.expectedResult);
	}

	// Calling atan8 with point 1 and point 2 must always return the opposite direction
	for (INT16 x1 = -500; x1 < 500; x1 += 11)
		for (INT16 y1 = -500; y1 < 500; y1 += 3)
			EXPECT_EQ(std::abs((int)atan8(0, 0, x1, y1) - (int)atan8(x1, y1, 0, 0)), 4);

	// Verify that both versions produce the same result when the points are identical
	EXPECT_EQ(atan8(10, 10, 10, 10), oldatan8(10, 10, 10, 10));

	// Old and new must produce the same result for these values
	for (INT16 x1 = -500; x1 < 500; x1++)
		EXPECT_EQ(atan8(10, 10, x1, 5), oldatan8(10, 10, x1, 5));

	// This is one point where the old and new versions differ
	EXPECT_NE(atan8(0, 0, 500, -205), oldatan8(0, 0, 500, -205));
}

#endif
