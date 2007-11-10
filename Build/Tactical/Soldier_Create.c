#include "Soldier_Create.h"
#include "Overhead.h"
#include "WCheck.h"
#include "Soldier_Profile.h"
#include "Animation_Control.h"
#include "Animation_Data.h"
#include "Sound_Control.h"
#include "Overhead_Types.h"
#include "Faces.h"
#include "Isometric_Utils.h"
#include "Items.h"
#include "Weapons.h"
#include "OppList.h"
#include "Random.h"
#include "Assignments.h"
#include "Soldier_Init_List.h"
#include "EditorMercs.h"
#include "Smell.h"
#include "Squads.h"
#include "Interface_Panels.h"
#include "StrategicMap.h"
#include "Inventory_Choosing.h"
#include "Queen_Command.h"
#include "Soldier_Add.h"
#include "Quests.h"
#include "Sys_Globals.h"
#include "Scheduling.h"
#include "Rotting_Corpses.h"
#include "Vehicles.h"
#include "Handle_UI.h"
#include "Text.h"
#include "Campaign.h"
#include "GameSettings.h"
#include "PreBattle_Interface.h"
#include "Auto_Resolve.h"
#include "Morale.h"
#include "AI.h"
#include "Strategic_Mines.h"
#include "MapScreen.h"
#include <math.h>
#include "JAScreens.h"
#include "SoundMan.h"
#include "MemMan.h"
#include "Debug.h"
#include "ScreenIDs.h"


// THESE 3 DIFFICULTY FACTORS MUST ALWAYS ADD UP TO 100% EXACTLY!!!
#define DIFF_FACTOR_PLAYER_PROGRESS			50
#define DIFF_FACTOR_PALACE_DISTANCE			30
#define DIFF_FACTOR_GAME_DIFFICULTY			20

// additional difficulty modifiers
#define DIFF_MODIFIER_SOME_PROGRESS			+5
#define DIFF_MODIFIER_NO_INCOME					-5
#define DIFF_MODIFIER_DRASSEN_MILITIA		+10


#define PALACE_SECTOR_X 3
#define PALACE_SECTOR_Y 16

#define MAX_PALACE_DISTANCE		20


BOOLEAN gfProfiledEnemyAdded = FALSE;

UINT32 guiCurrentUniqueSoldierId = 1;

// CJC note: trust me, it's easiest just to put this here; this is the only
// place it should need to be used
UINT8 gubItemDroppableFlag[NUM_INV_SLOTS] =
{
	0x01,
	0x02,
	0x04,
	0,
	0,
	0x08,
	0,
	0x10,
	0x20,
	0x40,
	0x80,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};


void RandomizeNewSoldierStats( SOLDIERCREATE_STRUCT *pCreateStruct )
{
	pCreateStruct->bLifeMax  							= (UINT8)Random(50)+50;
	pCreateStruct->bLife	  							= pCreateStruct->bLifeMax;
	pCreateStruct->bAgility								= (UINT8)Random(50)+50;
	pCreateStruct->bDexterity							= (UINT8)Random(50)+50;
	pCreateStruct->bExpLevel							= 1 + (UINT8)Random(4);

	// Randomize skills (for now)
	pCreateStruct->bMarksmanship					= (UINT8)Random(50)+50;
	pCreateStruct->bMedical								= (UINT8)Random(50)+50;
	pCreateStruct->bMechanical						= (UINT8)Random(50)+50;
	pCreateStruct->bExplosive							= (UINT8)Random(50)+50;
	pCreateStruct->bLeadership						= (UINT8)Random(50)+50;
	pCreateStruct->bStrength							= (UINT8)Random(50)+50;
	pCreateStruct->bWisdom								= (UINT8)Random(50)+50;
	pCreateStruct->bAttitude							= (INT8) Random( MAXATTITUDES );
	pCreateStruct->bOrders								= FARPATROL;
	pCreateStruct->bMorale								= 50;
	pCreateStruct->bAIMorale							= MORALE_FEARLESS;
}


static void CopyProfileItems(SOLDIERTYPE* pSoldier, const SOLDIERCREATE_STRUCT* pCreateStruct);
static void InitSoldierStruct(SOLDIERTYPE* pSoldier);
static BOOLEAN TacticalCopySoldierFromCreateStruct(SOLDIERTYPE* pSoldier, const SOLDIERCREATE_STRUCT* pCreateStruct);
static BOOLEAN TacticalCopySoldierFromProfile(SOLDIERTYPE* pSoldier, const SOLDIERCREATE_STRUCT* pCreateStruct);


SOLDIERTYPE* TacticalCreateSoldier(const SOLDIERCREATE_STRUCT* const pCreateStruct)
{
	SOLDIERTYPE			Soldier;
	INT32						cnt;
	SOLDIERTYPE			*pTeamSoldier;
	BOOLEAN					fGuyAvail = FALSE;
	UINT8						bLastTeamID;
	UINT8						ubVehicleID = 0;

	//Kris:
	//Huge no no!  See the header file for description of static detailed placements.
	//If this expression ever evaluates to true, then it will expose serious problems.
	//Simply returning won't help.
	if( pCreateStruct->fStatic )
	{
		Assert( 0 );
	}

	// Some values initialized here but could be changed before going to the common one
	InitSoldierStruct( &Soldier );

	Soldier.uiUniqueSoldierIdValue = guiCurrentUniqueSoldierId;

	guiCurrentUniqueSoldierId++;

	// OK, CHECK IF WE HAVE A VALID PROFILE ID!
	if ( pCreateStruct->ubProfile != NO_PROFILE )
	{
		// We have a merc created by profile, do this!
		TacticalCopySoldierFromProfile( &Soldier, pCreateStruct );
	}
	else
	{
		TacticalCopySoldierFromCreateStruct( &Soldier, pCreateStruct );
	}

	// If we are NOT creating an existing soldier ( ie, this is not from a save game ), create soldier normally
	if( !pCreateStruct->fUseExistingSoldier )
	{
		// We want to determine what team to place these guys in...

		// First off, force player team if they are a player guy! ( do some other stuff for only our guys!
		if ( pCreateStruct->fPlayerMerc )
		{
			Soldier.uiStatusFlags |= SOLDIER_PC;
			Soldier.bTeam = gbPlayerNum;
			Soldier.bVisible = 1;
		}
		else if ( pCreateStruct->fPlayerPlan )
		{
			Soldier.uiStatusFlags |= SOLDIER_PC;
			Soldier.bVisible = 1;
		}
		else
		{
			Soldier.uiStatusFlags |= SOLDIER_ENEMY;
		}


		// Check for auto team
		if ( pCreateStruct->bTeam == SOLDIER_CREATE_AUTO_TEAM )
		{
			// Auto determine!
			// OK, if this is our guy, set team as ours!
			if ( pCreateStruct->fPlayerMerc )
			{
				Soldier.bTeam = OUR_TEAM;
				Soldier.bNormalSmell = NORMAL_HUMAN_SMELL_STRENGTH;
			}
			else if ( pCreateStruct->fPlayerPlan )
			{
				Soldier.bTeam = PLAYER_PLAN;
			}
			else
			{
				// LOOK AT BODY TYPE!
				switch ( pCreateStruct->bBodyType )
				{
					case REGMALE:
					case BIGMALE:
					case STOCKYMALE:
					case REGFEMALE:

						Soldier.bTeam = ENEMY_TEAM;
						break;

					case ADULTFEMALEMONSTER:
					case AM_MONSTER:
					case YAF_MONSTER:
					case YAM_MONSTER:
					case LARVAE_MONSTER:
					case INFANT_MONSTER:
					case QUEENMONSTER:

						Soldier.bTeam = CREATURE_TEAM;
						break;

					case FATCIV:
					case MANCIV:
					case MINICIV:
					case DRESSCIV:
					case HATKIDCIV:
					case KIDCIV:
					case COW:
					case CROW:
					case ROBOTNOWEAPON:

						Soldier.bTeam = CIV_TEAM;
						break;

				}
			}
		}
		else
		{
			Soldier.bTeam = pCreateStruct->bTeam;
			// if WE_SEE_WHAT_MILITIA_SEES
			if ( Soldier.bTeam == MILITIA_TEAM )
			{
				Soldier.bVisible = 1;
			}
		}

		// Copy the items over for thew soldier, only if we have a valid profile id!
		if ( pCreateStruct->ubProfile != NO_PROFILE )
			CopyProfileItems( &Soldier, pCreateStruct );

		// Given team, get an ID for this guy!

		if( guiCurrentScreen != AUTORESOLVE_SCREEN )
		{
			cnt = gTacticalStatus.Team[ Soldier.bTeam ].bFirstID;

			// ATE: If we are a vehicle, and a player, start at a different slot ( 2 - max )
			if( Soldier.ubBodyType == HUMVEE ||
					Soldier.ubBodyType == ELDORADO ||
					Soldier.ubBodyType == ICECREAMTRUCK ||
					Soldier.ubBodyType == JEEP )
			{
				if( Soldier.bTeam == gbPlayerNum )
				{
					cnt = gTacticalStatus.Team[ Soldier.bTeam ].bLastID - 1;
				}
			}

			bLastTeamID = gTacticalStatus.Team[ Soldier.bTeam ].bLastID;

			// look for all mercs on the same team,
			for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= bLastTeamID; cnt++,pTeamSoldier++)
			{
				if ( !pTeamSoldier->bActive )
				{
					fGuyAvail = TRUE;
					break;
				}
			}

			// Check if there was space!
			if ( !fGuyAvail )
			{ //No space, so can't create the soldier.
				return NULL;
			}

			// OK, set ID
			Soldier.ubID = (UINT8)cnt;
		}


		// LOAD MERC's FACE!
		if ( pCreateStruct->ubProfile != NO_PROFILE && Soldier.bTeam == OUR_TEAM )
		{
			Soldier.iFaceIndex = InitSoldierFace( &Soldier );
		}


		Soldier.bActionPoints					= CalcActionPoints( &Soldier );
		Soldier.bInitialActionPoints	= Soldier.bActionPoints;
		Soldier.bSide									= gTacticalStatus.Team[ Soldier.bTeam ].bSide;
		Soldier.bActive								= TRUE;
		Soldier.sSectorX							= pCreateStruct->sSectorX;
		Soldier.sSectorY							= pCreateStruct->sSectorY;
		Soldier.bSectorZ							= pCreateStruct->bSectorZ;
		Soldier.ubInsertionDirection	= pCreateStruct->bDirection;
		Soldier.bDesiredDirection			= pCreateStruct->bDirection;
		Soldier.bDominantDir					= pCreateStruct->bDirection;
		Soldier.bDirection						= pCreateStruct->bDirection;

		Soldier.sInsertionGridNo			= pCreateStruct->sInsertionGridNo;
		Soldier.bOldLife							= Soldier.bLifeMax;

		// If a civvy, set neutral
		if ( Soldier.bTeam == CIV_TEAM )
		{
			if (Soldier.ubProfile == WARDEN )
			{
				Soldier.bNeutral = FALSE;
			}
			else if (Soldier.ubCivilianGroup != NON_CIV_GROUP)
			{
				if ( gTacticalStatus.fCivGroupHostile[ Soldier.ubCivilianGroup ] == CIV_GROUP_HOSTILE )
				{
					Soldier.bNeutral = FALSE;
				}
				else
				{
					Soldier.bNeutral = TRUE;
				}
			}
			else
			{
				Soldier.bNeutral = TRUE;
			}

			//Weaken stats based on the bodytype of the civilian.
			if( Soldier.ubProfile == NO_PROFILE )
			{
				switch( Soldier.ubBodyType )
				{
					case REGMALE:
					case BIGMALE:
					case STOCKYMALE:
					case REGFEMALE:
						//no adjustments necessary for these "healthy" bodytypes.
						break;
					case FATCIV:
						//fat, so slower
						Soldier.bAgility = (INT8)( 30 + Random( 26 ) ); //30 - 55
						break;
					case MANCIV:
						Soldier.bLife = Soldier.bLifeMax = (INT8)( 35 + Random( 26 ) ); //35 - 60
						break;
					case MINICIV:
					case DRESSCIV:
						Soldier.bLife = Soldier.bLifeMax = (INT8)( 30 + Random( 16 ) ); //30 - 45
						break;
					case HATKIDCIV:
					case KIDCIV:
						Soldier.bLife = Soldier.bLifeMax = (INT8)( 20 + Random( 16 ) ); //20 - 35
						break;
					case CRIPPLECIV:
						Soldier.bLife = Soldier.bLifeMax = (INT8)( 20 + Random( 26 ) ); //20 - 45
						Soldier.bAgility = (INT8)( 30 + Random( 16 ) ); // 30 - 45
						break;
				}
			}
		}
		else if ( Soldier.bTeam == CREATURE_TEAM )
		{
			// bloodcats are neutral to start out
			if ( Soldier.ubBodyType == BLOODCAT )
			{
				Soldier.bNeutral = TRUE;
			} // otherwise (creatures) false
		}

		// OK, If not given a profile num, set a randomized defualt battle sound set
		// and then adjust it according to body type!
		if ( Soldier.ubProfile == NO_PROFILE )
		{
			Soldier.ubBattleSoundID = (UINT8)Random( 3 );
		}

		// ATE: TEMP : No enemy women mercs (unless elite)!
		if( Soldier.ubProfile == NO_PROFILE && Soldier.bTeam == ENEMY_TEAM &&
				Soldier.ubBodyType == REGFEMALE && Soldier.ubSoldierClass != SOLDIER_CLASS_ELITE )
		{
			Soldier.ubBodyType = (UINT8)( REGMALE + Random( 3 ) );
		}

		// ATE
		// Set some values for variation in anims...
		if ( Soldier.ubBodyType == BIGMALE )
		{
			Soldier.uiAnimSubFlags |= SUB_ANIM_BIGGUYTHREATENSTANCE;
		}

		//For inventory, look for any face class items that may be located in the big pockets and if found, move
		//that item to a face slot and clear the pocket!
		if( Soldier.bTeam != OUR_TEAM )
		{
			INT32 i;
			BOOLEAN fSecondFaceItem = FALSE;
			for( i = BIGPOCK1POS; i <= BIGPOCK4POS; i++ )
			{
				if( Item[ Soldier.inv[ i ].usItem ].usItemClass & IC_FACE )
				{
					if( !fSecondFaceItem )
					{ //Don't check for compatibility...  automatically assume there are no head positions filled.
						fSecondFaceItem = TRUE;
						Soldier.inv[HEAD1POS] = Soldier.inv[i];
						memset( &Soldier.inv[ i ], 0, sizeof( OBJECTTYPE ) );
					}
					else
					{ //if there is a second item, compare it to the first one we already added.
						if( CompatibleFaceItem( Soldier.inv[ HEAD1POS ].usItem, Soldier.inv[ i ].usItem ) )
						{
							Soldier.inv[HEAD2POS] = Soldier.inv[i];
							memset( &Soldier.inv[ i ], 0, sizeof( OBJECTTYPE ) );
							break;
						}
					}
				}
			}

			if( guiCurrentScreen != AUTORESOLVE_SCREEN )
			{
				// also, if an army guy has camouflage, roll to determine whether they start camouflaged
				if ( Soldier.bTeam == ENEMY_TEAM )
				{
					i = FindObj( &Soldier, CAMOUFLAGEKIT );

					if ( i != NO_SLOT && Random( 5 ) < SoldierDifficultyLevel( &Soldier ))
					{
						// start camouflaged
						Soldier.bCamo = 100;
					}
				}
			}
		}

		//Set some flags, actions based on what body type we are
		//NOTE:  BE VERY CAREFUL WHAT YOU DO IN THIS SECTION!
		//  It is very possible to override editor settings, especially orders and attitude.
		//  In those cases, you can check for !gfEditMode (not in editor).
		switch ( Soldier.ubBodyType )
		{
			case HATKIDCIV:
			case KIDCIV:

				Soldier.ubBattleSoundID = (UINT8)Random( 2 );
				break;

			case REGFEMALE:
			case MINICIV:
			case DRESSCIV:

				Soldier.ubBattleSoundID = 7 + (UINT8) Random( 2 );
				Soldier.bNormalSmell = NORMAL_HUMAN_SMELL_STRENGTH;
				break;

			case BLOODCAT:
				AssignCreatureInventory( &Soldier );
				Soldier.bNormalSmell = NORMAL_HUMAN_SMELL_STRENGTH;
				Soldier.uiStatusFlags |= SOLDIER_ANIMAL;
				break;

			case ADULTFEMALEMONSTER:
			case AM_MONSTER:
			case YAF_MONSTER:
			case YAM_MONSTER:
			case LARVAE_MONSTER:
			case INFANT_MONSTER:
			case QUEENMONSTER:

				AssignCreatureInventory( &Soldier );
				Soldier.ubCaller = NOBODY;
				if( !gfEditMode )
				{
					Soldier.bOrders = FARPATROL;
					Soldier.bAttitude = AGGRESSIVE;
				}
				Soldier.uiStatusFlags |= SOLDIER_MONSTER;
				Soldier.bMonsterSmell = NORMAL_CREATURE_SMELL_STRENGTH;
				break;

			case COW:
				Soldier.uiStatusFlags |= SOLDIER_ANIMAL;
				Soldier.bNormalSmell = COW_SMELL_STRENGTH;
				break;
			case CROW:

				Soldier.uiStatusFlags |= SOLDIER_ANIMAL;
				break;

			case ROBOTNOWEAPON:

				Soldier.uiStatusFlags |= SOLDIER_ROBOT;
				break;

			case HUMVEE:
			case ELDORADO:
			case ICECREAMTRUCK:
			case JEEP:
				case TANK_NW:
				case TANK_NE:

				Soldier.uiStatusFlags |= SOLDIER_VEHICLE;

				switch( Soldier.ubBodyType )
				{
					case HUMVEE:

						ubVehicleID = HUMMER;
            Soldier.bNeutral = TRUE;
						break;

					case ELDORADO:

						ubVehicleID = ELDORADO_CAR;
            Soldier.bNeutral = TRUE;
						break;

					case ICECREAMTRUCK:

						ubVehicleID = ICE_CREAM_TRUCK;
            Soldier.bNeutral = TRUE;
						break;

					case JEEP:

						ubVehicleID = JEEP_CAR;
						break;

					case TANK_NW:
					case TANK_NE:

						ubVehicleID = TANK_CAR;
						break;

				}

				if ( pCreateStruct->fUseGivenVehicle )
				{
					Soldier.bVehicleID = pCreateStruct->bUseGivenVehicleID;
				}
				else
				{
					// Add vehicle to list....
					Soldier.bVehicleID = (INT8)AddVehicleToList( Soldier.sSectorX, Soldier.sSectorY, Soldier.bSectorZ, ubVehicleID );
				}
				SetVehicleValuesIntoSoldierType( &Soldier );
				break;

			default:
				Soldier.bNormalSmell = NORMAL_HUMAN_SMELL_STRENGTH;
				break;

		}

		if( guiCurrentScreen != AUTORESOLVE_SCREEN )
		{
			SOLDIERTYPE* const s = GetMan(Soldier.ubID);
			// Copy into merc struct
			*s = Soldier;
			// Alrighty then, we are set to create the merc, stuff after here can fail!
			CHECKF(CreateSoldierCommon(Soldier.ubBodyType, s, Soldier.ubID, STANDING));
		}
	}
	else
	{
		//Copy the data from the existing soldier struct to the new soldier struct
		Soldier = *pCreateStruct->pExistingSoldier;

		//Reset the face index
		Soldier.iFaceIndex = -1;
		Soldier.iFaceIndex = InitSoldierFace( &Soldier );

    // ATE: Reset soldier's light value to -1....
    Soldier.iLight = -1;

		if ( Soldier.ubBodyType == HUMVEE || Soldier.ubBodyType == ICECREAMTRUCK )
    {
      Soldier.bNeutral = TRUE;
    }

		SOLDIERTYPE* const s = GetMan(Soldier.ubID);

		// Copy into merc struct
		*s = Soldier;

		// Alrighty then, we are set to create the merc, stuff after here can fail!
		CHECKF(CreateSoldierCommon(Soldier.ubBodyType, s, Soldier.ubID, s->usAnimState));

		// The soldiers animation frame gets reset, set
//		s->usAniCode   = pCreateStruct->pExistingSoldier->usAniCode;
//		s->usAnimState = Soldier.usAnimState;
//		s->usAniFrame  = Soldier.usAniFrame;
	}


	if( guiCurrentScreen != AUTORESOLVE_SCREEN )
	{
		SOLDIERTYPE* const s = GetMan(Soldier.ubID);

		if( pCreateStruct->fOnRoof && FlatRoofAboveGridNo( pCreateStruct->sInsertionGridNo ) )
		{
			SetSoldierHeight(s, 58.0);
		}

		//if we are loading DONT add men to team, because the number is restored in gTacticalStatus
		if( !( gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
		{
			// Increment men in sector number!
			AddManToTeam( Soldier.bTeam );
		}

		return s;
	}
	else
	{ //We are creating a dynamically allocated soldier for autoresolve.
		SOLDIERTYPE *pSoldier;
		UINT8 ubSectorID;
		ubSectorID = GetAutoResolveSectorID();
		pSoldier = (SOLDIERTYPE*)MemAlloc( sizeof( SOLDIERTYPE ) );
		if( !pSoldier )
			return NULL;
		*pSoldier = Soldier;
		pSoldier->ubID = 255;
		pSoldier->sSectorX = (INT16)SECTORX( ubSectorID );
		pSoldier->sSectorY = (INT16)SECTORY( ubSectorID );
		pSoldier->bSectorZ = 0;
		return pSoldier;
	}
}


static BOOLEAN TacticalCopySoldierFromProfile(SOLDIERTYPE* const pSoldier, const SOLDIERCREATE_STRUCT* const pCreateStruct)
{
	UINT8						ubProfileIndex;
	MERCPROFILESTRUCT * pProfile;

	ubProfileIndex = pCreateStruct->ubProfile;
	pProfile = &(gMercProfiles[ubProfileIndex]);

	SET_PALETTEREP_ID ( pSoldier->HeadPal,		pProfile->HAIR );
	SET_PALETTEREP_ID ( pSoldier->VestPal,		pProfile->VEST );
	SET_PALETTEREP_ID ( pSoldier->SkinPal,		pProfile->SKIN );
	SET_PALETTEREP_ID ( pSoldier->PantsPal,   pProfile->PANTS );

	// Set profile index!
	pSoldier->ubProfile									= ubProfileIndex;
	pSoldier->ubScheduleID							= pCreateStruct->ubScheduleID;
	pSoldier->bHasKeys									= pCreateStruct->fHasKeys;

	wcscpy( pSoldier->name, pProfile->zNickname );

	pSoldier->bLife 										= pProfile->bLife;
	pSoldier->bLifeMax									= pProfile->bLifeMax;
	pSoldier->bAgility									= pProfile->bAgility;
	pSoldier->bLeadership								= pProfile->bLeadership;
	pSoldier->bDexterity								= pProfile->bDexterity;
	pSoldier->bStrength									= pProfile->bStrength;
	pSoldier->bWisdom										= pProfile->bWisdom;
	pSoldier->bExpLevel									= pProfile->bExpLevel;
	pSoldier->bMarksmanship							= pProfile->bMarksmanship;
	pSoldier->bMedical									= pProfile->bMedical;
	pSoldier->bMechanical								= pProfile->bMechanical;
	pSoldier->bExplosive								= pProfile->bExplosive;
	pSoldier->bScientific								= pProfile->bScientific;

	pSoldier->bVocalVolume							= MIDVOLUME;
	pSoldier->uiAnimSubFlags						= pProfile->uiBodyTypeSubFlags;
	pSoldier->ubBodyType								= pProfile->ubBodyType;
	pSoldier->ubCivilianGroup						= pProfile->ubCivilianGroup;
	//OK set initial duty
//  pSoldier->bAssignment=ON_DUTY;

	pSoldier->bOldAssignment = NO_ASSIGNMENT;
	pSoldier->ubSkillTrait1 = pProfile->bSkillTrait;
	pSoldier->ubSkillTrait2 = pProfile->bSkillTrait2;

	pSoldier->bOrders								= pCreateStruct->bOrders;
	pSoldier->bAttitude							= pCreateStruct->bAttitude;
	pSoldier->bDirection						= pCreateStruct->bDirection;
	pSoldier->bPatrolCnt						= pCreateStruct->bPatrolCnt;
	memcpy( pSoldier->usPatrolGrid, pCreateStruct->sPatrolGrid, sizeof( INT16 ) * MAXPATROLGRIDS );

	if ( HAS_SKILL_TRAIT( pSoldier, CAMOUFLAGED ) )
	{
		// set camouflaged to 100 automatically
		pSoldier->bCamo = 100;
	}
	return( TRUE );
}

enum { PINKSKIN, TANSKIN, DARKSKIN, BLACKSKIN, NUMSKINS };
enum {
	WHITEHEAD, BLACKHEAD, //black skin (only this line )
	BROWNHEAD,						//dark skin (this line plus all above)
	BLONDEHEAD, REDHEAD,   //pink/tan skin (this line plus all above )
	NUMHEADS
};


static INT32 ChooseHairColor(SOLDIERTYPE* pSoldier, INT32 skin)
{
	INT32 iRandom;
	INT32 hair = 0;
	iRandom = Random( 100 );
	switch( skin )
	{
		case PINKSKIN:
		case TANSKIN:
			if( iRandom < 12 )
			{
				hair = REDHEAD;
			}
			else if( iRandom < 34 )
			{
				hair = BLONDEHEAD;
			}
			else if( iRandom < 60 )
			{
				hair = BROWNHEAD;
			}
			else if( iRandom < 92 )
			{
				hair = BLACKHEAD;
			}
			else
			{
				hair = WHITEHEAD;
				if( pSoldier->ubBodyType == REGFEMALE ||
					  pSoldier->ubBodyType == MINICIV ||
						pSoldier->ubBodyType == DRESSCIV ||
						pSoldier->ubBodyType == HATKIDCIV ||
						pSoldier->ubBodyType == KIDCIV )
				{
					hair = Random( NUMHEADS - 1 ) + 1;
				}
			}
			hair = Random( NUMHEADS );
			break;
		case DARKSKIN:
			if( iRandom < 35 )
			{
				hair = BROWNHEAD;
			}
			else if( iRandom < 84 )
			{
				hair = BLACKHEAD;
			}
			else
			{
				hair = WHITEHEAD;
				if( pSoldier->ubBodyType == REGFEMALE ||
					  pSoldier->ubBodyType == MINICIV ||
						pSoldier->ubBodyType == DRESSCIV ||
						pSoldier->ubBodyType == HATKIDCIV ||
						pSoldier->ubBodyType == KIDCIV )
				{
					hair = Random( 2 ) + 1;
				}
			}
			break;
		case BLACKSKIN:
			if( iRandom < 84 )
			{
				hair = BLACKHEAD;
			}
			else
			{
				hair = WHITEHEAD;
				if( pSoldier->ubBodyType == REGFEMALE ||
					  pSoldier->ubBodyType == MINICIV ||
						pSoldier->ubBodyType == DRESSCIV ||
						pSoldier->ubBodyType == HATKIDCIV ||
						pSoldier->ubBodyType == KIDCIV )
				{
					hair = BLACKHEAD;
				}
			}
			break;
		default:
			AssertMsg( 0, "Skin type not accounted for." );
			break;
	}
	if( pSoldier->ubBodyType == CRIPPLECIV )
	{
		if( Chance( 50 ) )
		{
			hair = WHITEHEAD;
		}
	}
	return hair;
}


static void GeneratePaletteForSoldier(SOLDIERTYPE* pSoldier, UINT8 ubSoldierClass)
{
	INT32 skin, hair;
	BOOLEAN fMercClothingScheme;
	hair = -1;

	//choose random skin tone which will limit the choice of hair colors.
	skin = (INT8)Random( NUMSKINS );
	switch( skin )
	{
		case PINKSKIN:
			SET_PALETTEREP_ID( pSoldier->SkinPal,  "PINKSKIN" );
			break;
		case TANSKIN:
			SET_PALETTEREP_ID( pSoldier->SkinPal,  "TANSKIN" );
			break;
		case DARKSKIN:
			SET_PALETTEREP_ID( pSoldier->SkinPal,  "DARKSKIN" );
			break;
		case BLACKSKIN:
			SET_PALETTEREP_ID( pSoldier->SkinPal,  "BLACKSKIN" );
			break;
		default:
			AssertMsg( 0, "Skin type not accounted for." );
			break;
	}

	//Choose hair color which uses the skin color to limit choices
	hair = ChooseHairColor( pSoldier, skin );
	switch( hair )
	{
		case BROWNHEAD: SET_PALETTEREP_ID( pSoldier->HeadPal, "BROWNHEAD" ); break;
		case BLACKHEAD: SET_PALETTEREP_ID( pSoldier->HeadPal, "BLACKHEAD" ); break;
		case WHITEHEAD: SET_PALETTEREP_ID( pSoldier->HeadPal, "WHITEHEAD" ); break;
		case BLONDEHEAD:SET_PALETTEREP_ID( pSoldier->HeadPal, "BLONDHEAD" ); break;
		case REDHEAD:   SET_PALETTEREP_ID( pSoldier->HeadPal, "REDHEAD"   ); break;
		default:  AssertMsg( 0, "Hair type not accounted for.");						 break;
	}

	// OK, After skin, hair we could have a forced color scheme.. use here if so
	switch( ubSoldierClass )
	{
		case SOLDIER_CLASS_ADMINISTRATOR:
			SET_PALETTEREP_ID( pSoldier->VestPal, "YELLOWVEST"  );
			SET_PALETTEREP_ID( pSoldier->PantsPal, "GREENPANTS"   );
			pSoldier->ubSoldierClass = ubSoldierClass;
			return;
		case SOLDIER_CLASS_ELITE:
			SET_PALETTEREP_ID( pSoldier->VestPal, "BLACKSHIRT"  );
			SET_PALETTEREP_ID( pSoldier->PantsPal, "BLACKPANTS"   );
			pSoldier->ubSoldierClass = ubSoldierClass;
			return;
		case SOLDIER_CLASS_ARMY:
			SET_PALETTEREP_ID( pSoldier->VestPal, "REDVEST"  );
			SET_PALETTEREP_ID( pSoldier->PantsPal, "GREENPANTS"   );
			pSoldier->ubSoldierClass = ubSoldierClass;
			return;
		case SOLDIER_CLASS_GREEN_MILITIA:
			SET_PALETTEREP_ID( pSoldier->VestPal, "GREENVEST"  );
			SET_PALETTEREP_ID( pSoldier->PantsPal, "BEIGEPANTS"   );
			pSoldier->ubSoldierClass = ubSoldierClass;
			return;
		case SOLDIER_CLASS_REG_MILITIA:
			SET_PALETTEREP_ID( pSoldier->VestPal, "JEANVEST"  );
			SET_PALETTEREP_ID( pSoldier->PantsPal, "BEIGEPANTS"   );
			pSoldier->ubSoldierClass = ubSoldierClass;
			return;
		case SOLDIER_CLASS_ELITE_MILITIA:
			SET_PALETTEREP_ID( pSoldier->VestPal, "BLUEVEST"  );
			SET_PALETTEREP_ID( pSoldier->PantsPal, "BEIGEPANTS"   );
			pSoldier->ubSoldierClass = ubSoldierClass;
			return;
		case SOLDIER_CLASS_MINER:
			SET_PALETTEREP_ID( pSoldier->VestPal, "greyVEST"  );
			SET_PALETTEREP_ID( pSoldier->PantsPal, "BEIGEPANTS"   );
			pSoldier->ubSoldierClass = ubSoldierClass;
			return;
	}

	//there are 2 clothing schemes, 1 for mercs and 1 for civilians.  The
	//merc clothing scheme is much larger and general and is an exclusive superset
	//of the civilian clothing scheme which means the civilians will choose the
	//merc clothing scheme often ( actually 60% of the time ).
	if( !pSoldier->PantsPal[0] || !pSoldier->VestPal[0] )
	{
		fMercClothingScheme = TRUE;
		if( pSoldier->bTeam == CIV_TEAM && Random( 100 ) < 40 )
		{ //40% chance of using cheezy civilian colors
			fMercClothingScheme = FALSE;
		}
		if( !fMercClothingScheme ) //CHEEZY CIVILIAN COLORS
		{
			if( Random( 100 ) < 30 )
			{ //30% chance that the civilian will choose a gaudy yellow shirt with pants.
				SET_PALETTEREP_ID( pSoldier->VestPal, "GYELLOWSHIRT" );
				switch( Random( 3 ) )
				{
					case 0:	SET_PALETTEREP_ID( pSoldier->PantsPal, "TANPANTS"   ); break;
					case 1: SET_PALETTEREP_ID( pSoldier->PantsPal, "BEIGEPANTS" ); break;
					case 2: SET_PALETTEREP_ID( pSoldier->PantsPal, "GREENPANTS" ); break;
				}
			}
			else
			{ //70% chance that the civilian will choose jeans with a shirt.
				SET_PALETTEREP_ID( pSoldier->PantsPal, "JEANPANTS" );
				switch( Random( 7 ) )
				{
					case 0:	SET_PALETTEREP_ID( pSoldier->VestPal, "WHITEVEST"   ); break;
					case 1: SET_PALETTEREP_ID( pSoldier->VestPal, "BLACKSHIRT"  ); break;
					case 2: SET_PALETTEREP_ID( pSoldier->VestPal, "PURPLESHIRT" ); break;
					case 3: SET_PALETTEREP_ID( pSoldier->VestPal, "BLUEVEST"    ); break;
					case 4: SET_PALETTEREP_ID( pSoldier->VestPal, "BROWNVEST"   ); break;
					case 5: SET_PALETTEREP_ID( pSoldier->VestPal, "JEANVEST"    ); break;
					case 6: SET_PALETTEREP_ID( pSoldier->VestPal, "REDVEST"     ); break;
				}
			}
			return;
		}
		//MERC COLORS
		switch( Random( 3 ) )
		{
			case 0:
				SET_PALETTEREP_ID( pSoldier->PantsPal, "GREENPANTS" );
				switch( Random( 4 ) )
				{
					case 0: SET_PALETTEREP_ID( pSoldier->VestPal, "YELLOWVEST" ); break;
					case 1: SET_PALETTEREP_ID( pSoldier->VestPal, "WHITEVEST"  ); break;
					case 2: SET_PALETTEREP_ID( pSoldier->VestPal, "BROWNVEST"  ); break;
					case 3: SET_PALETTEREP_ID( pSoldier->VestPal, "GREENVEST"  ); break;
				}
				break;
			case 1:
				SET_PALETTEREP_ID( pSoldier->PantsPal, "TANPANTS" );
				switch( Random( 8 ) )
				{
					case 0: SET_PALETTEREP_ID( pSoldier->VestPal, "YELLOWVEST" ); break;
					case 1: SET_PALETTEREP_ID( pSoldier->VestPal, "WHITEVEST"  ); break;
					case 2: SET_PALETTEREP_ID( pSoldier->VestPal, "BLACKSHIRT" ); break;
					case 3: SET_PALETTEREP_ID( pSoldier->VestPal, "BLUEVEST"   ); break;
					case 4: SET_PALETTEREP_ID( pSoldier->VestPal, "BROWNVEST"  ); break;
					case 5: SET_PALETTEREP_ID( pSoldier->VestPal, "GREENVEST"  ); break;
					case 6: SET_PALETTEREP_ID( pSoldier->VestPal, "JEANVEST"   ); break;
					case 7: SET_PALETTEREP_ID( pSoldier->VestPal, "REDVEST"    ); break;
				}
				break;
			case 2:
				SET_PALETTEREP_ID( pSoldier->PantsPal, "BLUEPANTS" );
				switch( Random( 4 ) )
				{
					case 0: SET_PALETTEREP_ID( pSoldier->VestPal, "YELLOWVEST" ); break;
					case 1: SET_PALETTEREP_ID( pSoldier->VestPal, "WHITEVEST"  ); break;
					case 2: SET_PALETTEREP_ID( pSoldier->VestPal, "REDVEST"    ); break;
					case 3: SET_PALETTEREP_ID( pSoldier->VestPal, "BLACKSHIRT" ); break;
				}
				break;
		}
	}
}


static BOOLEAN TacticalCopySoldierFromCreateStruct(SOLDIERTYPE* const pSoldier, const SOLDIERCREATE_STRUCT* const pCreateStruct)
{
	pSoldier->ubProfile							= NO_PROFILE;

	// Randomize attributes
	pSoldier->bLife	  							= pCreateStruct->bLife;
	pSoldier->bLifeMax  						= pCreateStruct->bLifeMax;
	pSoldier->bAgility							= pCreateStruct->bAgility;
	pSoldier->bDexterity						= pCreateStruct->bDexterity;
	pSoldier->bExpLevel							= pCreateStruct->bExpLevel;

	pSoldier->bMarksmanship					= pCreateStruct->bMarksmanship;
	pSoldier->bMedical							= pCreateStruct->bMedical;
	pSoldier->bMechanical						= pCreateStruct->bMechanical;
	pSoldier->bExplosive						= pCreateStruct->bExplosive;
	pSoldier->bLeadership						= pCreateStruct->bLeadership;
	pSoldier->bStrength							= pCreateStruct->bStrength;
	pSoldier->bWisdom								= pCreateStruct->bWisdom;

	pSoldier->bAttitude							= pCreateStruct->bAttitude;
	pSoldier->bOrders								= pCreateStruct->bOrders;
	pSoldier->bMorale								= pCreateStruct->bMorale;
	pSoldier->bAIMorale							= pCreateStruct->bAIMorale;
	pSoldier->bVocalVolume					= MIDVOLUME;
	pSoldier->ubBodyType						= pCreateStruct->bBodyType;
	pSoldier->ubCivilianGroup				= pCreateStruct->ubCivilianGroup;

	pSoldier->ubScheduleID					= pCreateStruct->ubScheduleID;
	pSoldier->bHasKeys							= pCreateStruct->fHasKeys;
	pSoldier->ubSoldierClass				= pCreateStruct->ubSoldierClass;

	if( pCreateStruct->fVisible )
	{
		strcpy(pSoldier->HeadPal,  pCreateStruct->HeadPal);
		strcpy(pSoldier->PantsPal, pCreateStruct->PantsPal);
		strcpy(pSoldier->VestPal,  pCreateStruct->VestPal);
		strcpy(pSoldier->SkinPal,  pCreateStruct->SkinPal);
	}

	//KM:  March 25, 1999
	//Assign nightops traits to enemies/militia
	if( pSoldier->ubSoldierClass == SOLDIER_CLASS_ELITE || pSoldier->ubSoldierClass == SOLDIER_CLASS_ELITE_MILITIA )
	{
		INT32 iChance;
		UINT8	ubProgress;

		ubProgress = HighestPlayerProgressPercentage();

		if ( ubProgress < 60 )
		{
			// ramp chance from 40 to 80% over the course of 60% progress
			// 60 * 2/3 = 40, and 40+40 = 80
			iChance = 40 + (ubProgress * 2) / 3;
		}
		else
		{
			iChance = 80;
		}

		if ( Chance( iChance ) )
		{
			pSoldier->ubSkillTrait1 = NIGHTOPS;
			if ( ubProgress >= 40 && Chance( 30 ) )
			{
				pSoldier->ubSkillTrait2 = NIGHTOPS;
			}
		}
	}
	else if( pSoldier->ubSoldierClass == SOLDIER_CLASS_ARMY || pSoldier->ubSoldierClass == SOLDIER_CLASS_REG_MILITIA )
	{
		INT32 iChance;
		UINT8	ubProgress;

		ubProgress = HighestPlayerProgressPercentage();

		if ( ubProgress < 60 )
		{
			// ramp chance from 0 to 40% over the course of 60% progress
			// 60 * 2/3 = 40
			iChance = (ubProgress * 2) / 3;
		}
		else
		{
			iChance = 40;
		}

		if ( Chance( iChance ) )
		{
			pSoldier->ubSkillTrait1 = NIGHTOPS;
			if ( ubProgress >= 50 && Chance( 20 ) )
			{
				pSoldier->ubSkillTrait2 = NIGHTOPS;
			}
		}
	}

	//KM:  November 10, 1997
	//Adding patrol points
	//CAUTION:  CONVERTING SIGNED TO UNSIGNED though the values should never be negative.
	pSoldier->bPatrolCnt						= pCreateStruct->bPatrolCnt;
	memcpy( pSoldier->usPatrolGrid, pCreateStruct->sPatrolGrid, sizeof( INT16 ) * MAXPATROLGRIDS );

	//Kris:  November 10, 1997
	//Expanded the default names based on team.
	const wchar_t* Name;
	switch( pCreateStruct->bTeam )
	{
		case ENEMY_TEAM:    Name = TacticalStr[ENEMY_TEAM_MERC_NAME];   break;
		case MILITIA_TEAM:  Name = TacticalStr[MILITIA_TEAM_MERC_NAME]; break;

		case CIV_TEAM:
			if( pSoldier->ubSoldierClass == SOLDIER_CLASS_MINER )
			{
				Name = TacticalStr[CIV_TEAM_MINER_NAME];
			}
			else
			{
				Name = TacticalStr[CIV_TEAM_MERC_NAME];
			}
			break;

		case CREATURE_TEAM:
			if( pSoldier->ubBodyType == BLOODCAT )
			{
				Name = gzLateLocalizedString[36];
			}
			else
			{
				Name = TacticalStr[CREATURE_TEAM_MERC_NAME];
			}
			break;

		default: goto no_name; // XXX fishy
	}
	wcslcpy(pSoldier->name, Name, lengthof(pSoldier->name));
no_name:

	//Generate colors for soldier based on the body type.
	GeneratePaletteForSoldier( pSoldier, pCreateStruct->ubSoldierClass );

	// Copy item info over
	memcpy( pSoldier->inv, pCreateStruct->Inv, sizeof( OBJECTTYPE ) * NUM_INV_SLOTS );

	return( TRUE );
}


static void InitSoldierStruct(SOLDIERTYPE* pSoldier)
{
	// Memset values
	memset( pSoldier, 0, sizeof( SOLDIERTYPE ) );

	// Set default values
	pSoldier->bVisible							= -1;
	pSoldier->iFaceIndex						= -1;

	// Set morale default
	pSoldier->bMorale								= DEFAULT_MORALE;

	pSoldier->ubAttackerID					= NOBODY;
	pSoldier->ubPreviousAttackerID	= NOBODY;
	pSoldier->ubNextToPreviousAttackerID	= NOBODY;

	//Set AI Delay!
	pSoldier->uiAIDelay							= 100;

	pSoldier->iLight								= -1;
	pSoldier->iFaceIndex						= -1;

	// Set update time to new speed
	pSoldier->ubDesiredHeight				= NO_DESIRED_HEIGHT;
	pSoldier->bViewRange						= NORMAL_VIEW_RANGE;
	pSoldier->bInSector							= FALSE;
	pSoldier->sGridNo								= NO_MAP_POS;
	pSoldier->iMuzFlash							= -1;
	pSoldier->usPendingAnimation		= NO_PENDING_ANIMATION;
	pSoldier->usPendingAnimation2		= NO_PENDING_ANIMATION;
	pSoldier->ubPendingStanceChange	= NO_PENDING_STANCE;
	pSoldier->ubPendingDirection		= NO_PENDING_DIRECTION;
	pSoldier->ubPendingAction				= NO_PENDING_ACTION;
	pSoldier->bLastRenderVisibleValue	= -1;
	pSoldier->bBreath								= 99;
	pSoldier->bBreathMax						= 100;
	pSoldier->bActive									= TRUE;
	pSoldier->fShowLocator						= FALSE;
	pSoldier->sLastTarget							= NOWHERE;
	pSoldier->sAbsoluteFinalDestination = NOWHERE;
	pSoldier->sZLevelOverride					= -1;
	pSoldier->ubServicePartner				= NOBODY;
	pSoldier->ubAttackingHand					= HANDPOS;
	pSoldier->usAnimState							= STANDING;
	pSoldier->bInterruptDuelPts				= NO_INTERRUPT;
	pSoldier->bMoved									= FALSE;
	pSoldier->ubRobotRemoteHolderID		= NOBODY;
	pSoldier->sNoiseGridno						= NOWHERE;
	pSoldier->ubPrevSectorID					= 255;
	pSoldier->bNextPatrolPnt					= 1;
	pSoldier->bCurrentCivQuote				= -1;
	pSoldier->bCurrentCivQuoteDelta		= 0;
	pSoldier->uiBattleSoundID					= NO_SAMPLE;
	pSoldier->ubXRayedBy							= NOBODY;
	pSoldier->uiXRayActivatedTime			= 0;
	pSoldier->bBulletsLeft						= 0;
	pSoldier->bVehicleUnderRepairID		= -1;
}


BOOLEAN InternalTacticalRemoveSoldier(SOLDIERTYPE* const s, const BOOLEAN fRemoveVehicle)
{
	// ATE: If this guy is our global selected dude, take selection off...
	if (gUIFullTarget == s) gUIFullTarget = NULL;

	return TacticalRemoveSoldierPointer(s, fRemoveVehicle);
}

BOOLEAN TacticalRemoveSoldierPointer( SOLDIERTYPE *pSoldier, BOOLEAN fRemoveVehicle )
{
	if( !pSoldier->bActive )
		return FALSE;

	if( pSoldier->ubScheduleID )
	{
		DeleteSchedule( pSoldier->ubScheduleID );
	}

	if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE && fRemoveVehicle )
	{
		// remove this vehicle from the list
		RemoveVehicleFromList( pSoldier->bVehicleID );
	}

	// Handle crow leave....
	if ( pSoldier->ubBodyType == CROW )
	{
		HandleCrowLeave( pSoldier );
	}

	if( guiCurrentScreen != AUTORESOLVE_SCREEN )
	{

		// remove character from squad list.. if they are on one
		RemoveCharacterFromSquads( pSoldier );

		//remove the soldier from the interface panel
		RemovePlayerFromTeamSlot(pSoldier);

		// Check if a guy exists here
		// Does another soldier exist here?
		if ( pSoldier->bActive  )
		{
			RemoveSoldierFromGridNo( pSoldier );

      // Delete shadow of crow....
			if ( pSoldier->pAniTile != NULL )
			{
				DeleteAniTile( pSoldier->pAniTile );
				pSoldier->pAniTile = NULL;
			}

			if ( ! (pSoldier->uiStatusFlags & SOLDIER_OFF_MAP) )
			{
				// Decrement men in sector number!
				RemoveManFromTeam( pSoldier->bTeam );
			} // people specified off-map have already been removed from their team count

			pSoldier->bActive = FALSE;

			// Delete!
			DeleteSoldier( pSoldier );
		}
	}
	else
	{
		if( gfPersistantPBI )
		{
			DeleteSoldier( pSoldier );
		}
		MemFree( pSoldier );
	}

	return( TRUE );
}


BOOLEAN TacticalRemoveSoldier(SOLDIERTYPE* const s)
{
	return InternalTacticalRemoveSoldier(s, TRUE);
}


static UINT8 GetLocationModifier(UINT8 ubSoldierClass);


// returns a soldier difficulty modifier from 0 to 100 based on player's progress, distance from the Palace, mining income, and
// playing difficulty level.  Used for generating soldier stats, equipment, and AI skill level.
INT8 CalcDifficultyModifier( UINT8 ubSoldierClass )
{
	INT8 bDiffModifier = 0;
	UINT8 ubProgress;
	UINT8 ubProgressModifier;


	if( gfEditMode )
	{
		// return an average rating for editor purposes
		return( 50 );
	}


	// THESE 3 DIFFICULTY FACTORS MUST ALWAYS ADD UP TO 100% EXACTLY!!!
	Assert( ( DIFF_FACTOR_PLAYER_PROGRESS	+ DIFF_FACTOR_PALACE_DISTANCE	+ DIFF_FACTOR_GAME_DIFFICULTY ) == 100 );


	// adjust for game difficulty level
	switch( gGameOptions.ubDifficultyLevel )
	{
		case DIF_LEVEL_EASY:
			// very strong militia, very weak enemies/cratures/bloodcats
			if ( SOLDIER_CLASS_MILITIA( ubSoldierClass ) )
			{
				// +20
				bDiffModifier += DIFF_FACTOR_GAME_DIFFICULTY;
			}
			break;

		case DIF_LEVEL_MEDIUM:
			// equally strong militia, enemies, creatures, bloodcats (+10)
			bDiffModifier += ( DIFF_FACTOR_GAME_DIFFICULTY / 2 );
			break;

		case DIF_LEVEL_HARD:
			// equally stronger militia/enemies/creatures/bloodcats (+20)
			bDiffModifier += DIFF_FACTOR_GAME_DIFFICULTY;
			break;
	}


	// the progress returned ranges from 0 to 100
	ubProgress = HighestPlayerProgressPercentage();

	// bump it a bit once we've accomplished something (killed some enemies or taken an important sector)
	if ( ubProgress > 0 )
	{
		// +5
		bDiffModifier += DIFF_MODIFIER_SOME_PROGRESS;
	}

	// drop it down a bit if we still don't have any mine income
	if ( PredictIncomeFromPlayerMines() == 0 )
	{
		// -5
		bDiffModifier += DIFF_MODIFIER_NO_INCOME;
	}


	// adjust for progress level (0 to +50)
	ubProgressModifier = ( ubProgress * DIFF_FACTOR_PLAYER_PROGRESS ) / 100;
	bDiffModifier += ubProgressModifier;


	// adjust for map location
	bDiffModifier += GetLocationModifier( ubSoldierClass );


	// should be no way to go over 100, although it's possible to go below 0 when just starting on easy
	// Assert( bDiffModifier <= 100 );

	// limit the range of the combined factors to between 0 and 100
	bDiffModifier = __max(   0, bDiffModifier );
	bDiffModifier = __min( 100, bDiffModifier );

	// DON'T change this function without carefully considering the impact on GenerateRandomEquipment(),
	// CreateDetailedPlacementGivenBasicPlacementInfo(), and SoldierDifficultyLevel().

	return(bDiffModifier);
}


static void ReduceHighExpLevels(INT8* pbExpLevel);


//When the editor modifies the soldier's relative attribute level,
//this function is called to update that information.
//Used to generate a detailed placement from a basic placement.  This assumes that the detailed placement
//doesn't exist, meaning there are no static attributes.  This is called when you wish to convert a basic
//placement into a detailed placement just before creating a soldier.
void CreateDetailedPlacementGivenBasicPlacementInfo( SOLDIERCREATE_STRUCT *pp, BASIC_SOLDIERCREATE_STRUCT *bp )
{
	INT8 bBaseAttribute;
	UINT8 ubSoldierClass;
	UINT8 ubDiffFactor;
	INT8 bExpLevelModifier;
	INT8 bStatsModifier;
	UINT8 ubStatsLevel;


	if( !pp || !bp )
		return;
	pp->fStatic								= FALSE;
	pp->ubProfile							= NO_PROFILE;
	pp->sInsertionGridNo			= bp->usStartingGridNo;
	pp->fPlayerMerc						= FALSE;
	pp->fPlayerPlan						= FALSE;
	pp->fCopyProfileItemsOver = FALSE;
	pp->bTeam									= bp->bTeam;
	pp->ubSoldierClass				= bp->ubSoldierClass;
	pp->ubCivilianGroup				= bp->ubCivilianGroup;
	pp->ubScheduleID					= 0;
	pp->sSectorX							= gWorldSectorX;
	pp->sSectorY							= gWorldSectorY;
	pp->bSectorZ							= gbWorldSectorZ;
	pp->fHasKeys							= bp->fHasKeys;

	//Choose a body type randomly if none specified.
	if( bp->bBodyType < 0 )
	{
		switch ( bp->bTeam )
		{
			case OUR_TEAM:
			case ENEMY_TEAM:
			case MILITIA_TEAM:
				switch( Random( 4 ) )
				{
					case 0:	pp->bBodyType = REGMALE;		break;
					case 1:	pp->bBodyType = BIGMALE;		break;
					case 2:	pp->bBodyType = STOCKYMALE;	break;
					case 3:	pp->bBodyType = REGFEMALE;	break;
				}
				break;
			case CIV_TEAM:
				if( pp->ubSoldierClass == SOLDIER_CLASS_MINER )
				{
					switch( Random( 3 ) )
					{ //only strong and fit men can be miners.
						case 0:	pp->bBodyType = REGMALE;	break;
						case 1:	pp->bBodyType = BIGMALE;	break;
						case 2: pp->bBodyType = MANCIV;		break;
					}
				}
				else
				{
					INT32 iRandom;
					iRandom = Random( 100 );
					if( iRandom < 8 )
					{ //8% chance FATCIV
						pp->bBodyType = FATCIV;
					}
					else if( iRandom < 38 )
					{ //30% chance MANCIV
						pp->bBodyType = MANCIV;
					}
					else if( iRandom < 57 )
					{ //19% chance MINICIV
						pp->bBodyType = MINICIV;
					}
					else if( iRandom < 76 )
					{ //19% chance DRESSCIV
						pp->bBodyType = DRESSCIV;
					}
					else if( iRandom < 88 )
					{ //12% chance HATKIDCIV
						pp->bBodyType = HATKIDCIV;
					}
					else
					{ //12% chance KIDCIV
						pp->bBodyType = KIDCIV;
					}
				}
				break;
		}
	}
	else
	{
		pp->bBodyType = bp->bBodyType;
	}

	//Pass over mandatory information specified from the basic placement
	pp->bOrders = bp->bOrders;
	pp->bAttitude = bp->bAttitude;
	pp->bDirection = bp->bDirection;


	// determine this soldier's soldier class
	if( bp->bTeam == CREATURE_TEAM )
	{
		ubSoldierClass = SOLDIER_CLASS_CREATURE;
	}
	else
	{
		ubSoldierClass = bp->ubSoldierClass;
	}

	ubDiffFactor = CalcDifficultyModifier( ubSoldierClass );


	// experience level is modified by game difficulty, player's progress, & proximity to Queen's Palace, etc.
	// This formula gives the following results:
	//	DIFFICULTY FACTOR			EXP. LEVEL  MODIFIER		LEVEL OF AVG REGULAR TROOP
	//			   0 to 19									-2													2
	//			  20 to 39									-1													3
	//			  41 to 59									-0													4
	//				60 to 79									+1													5
	//				80 to 99									+2													6
	//				  100											+3													7		(can happen in P3 Meduna itself on HARD only!)
	bExpLevelModifier = ( ubDiffFactor / 20 ) - 2;

	// if in the upper half of this difficulty rating (10-19, 30-39, 50-59, 70-79, and 90-99)
	if ( ( ubDiffFactor % 20 ) >= 10 )
	{
		// increase stats only by one level's worth
		bStatsModifier = +1;
	}
	else
	{
		// normal stats for this level
		bStatsModifier = 0;
	}

	// Adjust level and statistics for Linda's prespecified relative attribute level
	switch ( bp->bRelativeAttributeLevel )
	{
		// NOTE: bStatsModifier already includes the bExpLevelModifier since it's based on the level itself!
		case 0:	bExpLevelModifier += -1; bStatsModifier += -1; break;
		case 1:	bExpLevelModifier += -1; bStatsModifier +=  0; break;
		case 2:	bExpLevelModifier +=  0; bStatsModifier +=  0; break;
		case 3:	bExpLevelModifier += +1; bStatsModifier +=  0; break;
		case 4:	bExpLevelModifier += +1; bStatsModifier += +1; break;

		default:
			AssertMsg( FALSE, String( "Invalid bRelativeAttributeLevel = %d", bp->bRelativeAttributeLevel ) );
			break;
	}


	//Set the experience level based on the soldier class and exp level modifier or relative attribute level
	switch( ubSoldierClass )
	{
		case SOLDIER_CLASS_ADMINISTRATOR:
			pp->bExpLevel = (INT8) 2 + bExpLevelModifier;
			break;
		case SOLDIER_CLASS_ARMY:
			pp->bExpLevel = (INT8) 4 + bExpLevelModifier;
			break;
		case SOLDIER_CLASS_ELITE:
			pp->bExpLevel = (INT8) 6 + bExpLevelModifier;
			break;
		case SOLDIER_CLASS_GREEN_MILITIA:
			pp->bExpLevel = (INT8) 2 + bExpLevelModifier;
			break;
		case SOLDIER_CLASS_REG_MILITIA:
			pp->bExpLevel = (INT8) 4 + bExpLevelModifier;
			break;
		case SOLDIER_CLASS_ELITE_MILITIA:
			pp->bExpLevel = (INT8) 6 + bExpLevelModifier;
			break;
		case SOLDIER_CLASS_MINER:
			pp->bExpLevel = bp->bRelativeAttributeLevel;					 //avg 2 (1-4)
			break;

		case SOLDIER_CLASS_CREATURE:
			switch( bp->bBodyType )
			{
				case LARVAE_MONSTER:
					pp->bExpLevel = 1 + bExpLevelModifier;
					break;
				case INFANT_MONSTER:
					pp->bExpLevel = 2 + bExpLevelModifier;
					break;
				case YAF_MONSTER:
				case YAM_MONSTER:
					pp->bExpLevel = (INT8)(3 + Random( 2 ) + bExpLevelModifier ); //3-4
					break;
				case ADULTFEMALEMONSTER:
				case AM_MONSTER:
					pp->bExpLevel = (INT8)(5 + Random( 2 ) + bExpLevelModifier ); //5-6
					break;
				case QUEENMONSTER:
					pp->bExpLevel = 7 + bExpLevelModifier;
					break;

				case BLOODCAT:
					pp->bExpLevel = 5 + bExpLevelModifier;
					if( SECTOR( gWorldSectorX, gWorldSectorY ) == SEC_I16 )
					{
						pp->bExpLevel += gGameOptions.ubDifficultyLevel;
					}
					break;
			}
			break;

		default:
			pp->bExpLevel = bp->bRelativeAttributeLevel;					 //avg 2 (1-4)
			ubSoldierClass = SOLDIER_CLASS_NONE;
			break;
	}


	pp->bExpLevel = max( 1, pp->bExpLevel ); //minimum exp. level of 1
	pp->bExpLevel = min( 9, pp->bExpLevel ); //maximum exp. level of 9

	ubStatsLevel = pp->bExpLevel + bStatsModifier;
	#if 0 /* unsigned < 0 ? */
	ubStatsLevel = max( 0, ubStatsLevel );	//minimum stats level of 0
	#else
	ubStatsLevel = ubStatsLevel;	//minimum stats level of 0
	#endif
	ubStatsLevel = min( 9, ubStatsLevel );	//maximum stats level of 9

	//Set the minimum base attribute
	bBaseAttribute = 49 + ( 4 * ubStatsLevel );

	//Roll soldier's statistics and skills
	//Stat range is currently 49-99, bell-curved around a range of 16 values dependent on the stats level
	pp->bLifeMax			= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bLife	  			= pp->bLifeMax;
	pp->bAgility			= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bDexterity		= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));

	pp->bMarksmanship	= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bMedical			= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bMechanical		= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bExplosive		= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bLeadership		= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bStrength			= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bWisdom				= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bMorale				= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));

	// CJC: now calculate the REAL experience level if in the really upper end
	ReduceHighExpLevels( &( pp->bExpLevel ) );

	pp->fVisible			= 0;

	pp->fOnRoof				= bp->fOnRoof;

	pp->ubSoldierClass = ubSoldierClass;

	//Transfer over the patrol points.
	pp->bPatrolCnt		= bp->bPatrolCnt;
	memcpy( pp->sPatrolGrid, bp->sPatrolGrid, sizeof( INT16 ) * MAXPATROLGRIDS );

	//If it is a detailed placement, don't do this yet, as detailed placements may have their
	//own equipment.
	if( !bp->fDetailedPlacement && ubSoldierClass != SOLDIER_CLASS_NONE && ubSoldierClass != SOLDIER_CLASS_CREATURE && ubSoldierClass != SOLDIER_CLASS_MINER )
		GenerateRandomEquipment( pp, ubSoldierClass, bp->bRelativeEquipmentLevel);
}


//Used exclusively by the editor when the user wishes to change a basic placement into a detailed placement.
//Because the intention is to make some of the attributes static, all of the information that can be generated
//are defaulted to -1.  When an attribute is made to be static, that value in replaced by the new static value.
//This information is NOT compatible with TacticalCreateSoldier.  Before doing so, you must first convert the
//static detailed placement to a regular detailed placement.
void CreateStaticDetailedPlacementGivenBasicPlacementInfo( SOLDIERCREATE_STRUCT *spp, BASIC_SOLDIERCREATE_STRUCT *bp )
{
	INT32 i;
	if( !spp || !bp )
		return;
	memset( spp, 0, sizeof( SOLDIERCREATE_STRUCT ) );
	spp->fStatic								= TRUE;
	spp->ubProfile							= NO_PROFILE;
	spp->sInsertionGridNo				= bp->usStartingGridNo;
	spp->fPlayerMerc						= FALSE;
	spp->fPlayerPlan						= FALSE;
	spp->fCopyProfileItemsOver	= FALSE;
	spp->bTeam									= bp->bTeam;
	spp->ubSoldierClass					= bp->ubSoldierClass;
	spp->ubCivilianGroup				= bp->ubCivilianGroup;
	spp->ubScheduleID						= 0;
	spp->sSectorX							= gWorldSectorX;
	spp->sSectorY							= gWorldSectorY;
	spp->bSectorZ							= gbWorldSectorZ;
	spp->fHasKeys							= bp->fHasKeys;

	//Pass over mandatory information specified from the basic placement
	spp->bOrders = bp->bOrders;
	spp->bAttitude = bp->bAttitude;
	spp->bDirection = bp->bDirection;

	//Only creatures have mandatory body types specified.
	if( spp->bTeam == CREATURE_TEAM )
		spp->bBodyType = bp->bBodyType;
	else
		spp->bBodyType = -1;

	//Set up all possible static values.
	//By setting them all to -1, that signifies that the attribute isn't static.
	//The static placement needs to be later *regenerated* in order to create a valid soldier.
	spp->bExpLevel			= -1;
	spp->bLifeMax				= -1;
	spp->bLife	  			= -1;
	spp->bAgility				= -1;
	spp->bDexterity			= -1;
	spp->bMarksmanship	= -1;
	spp->bMedical				= -1;
	spp->bMechanical		= -1;
	spp->bExplosive			= -1;
	spp->bLeadership		= -1;
	spp->bStrength			= -1;
	spp->bWisdom				= -1;
	spp->bMorale				= -1;

	spp->fVisible				=  0;

	spp->fOnRoof				= bp->fOnRoof;

	//Transfer over the patrol points.
	spp->bPatrolCnt			= bp->bPatrolCnt;
	memcpy( spp->sPatrolGrid, bp->sPatrolGrid, sizeof( INT16 ) * MAXPATROLGRIDS );

	//Starts with nothing
	for( i = 0; i < NUM_INV_SLOTS; i++ )
	{
		memset( &(spp->Inv[ i ]), 0, sizeof( OBJECTTYPE ) );
		spp->Inv[ i ].usItem = NOTHING;
		spp->Inv[ i ].fFlags |= OBJECT_UNDROPPABLE;
	}
}

//When you are ready to generate a soldier with a static detailed placement slot, this function will generate
//the proper detailed placement slot given the static detailed placement and it's accompanying basic placement.
//For the purposes of merc editing, the static detailed placement is preserved.
void CreateDetailedPlacementGivenStaticDetailedPlacementAndBasicPlacementInfo(
		   SOLDIERCREATE_STRUCT *pp, SOLDIERCREATE_STRUCT *spp, BASIC_SOLDIERCREATE_STRUCT *bp )
{
	INT32 i;

	memset( pp, 0, sizeof( SOLDIERCREATE_STRUCT ) );
	pp->fOnRoof = spp->fOnRoof = bp->fOnRoof;
	pp->fStatic = FALSE;
	pp->ubSoldierClass = bp->ubSoldierClass;
	//Generate the new placement
	pp->ubProfile = spp->ubProfile;
	if( pp->ubProfile != NO_PROFILE )
	{
		// Copy over team
		pp->bTeam = bp->bTeam;

		pp->bDirection						= bp->bDirection;
		pp->sInsertionGridNo			= bp->usStartingGridNo;

		//ATE: Copy over sector coordinates from profile to create struct
		pp->sSectorX							= gMercProfiles[ pp->ubProfile ].sSectorX;
		pp->sSectorY							= gMercProfiles[ pp->ubProfile ].sSectorY;
		pp->bSectorZ							= gMercProfiles[ pp->ubProfile ].bSectorZ;

		pp->ubScheduleID					= spp->ubScheduleID;

		pp->bOrders								= bp->bOrders;
		pp->bAttitude							= bp->bAttitude;
		pp->bDirection						= bp->bDirection;
		pp->bPatrolCnt						= bp->bPatrolCnt;
		memcpy( pp->sPatrolGrid, bp->sPatrolGrid, sizeof( INT16 ) * MAXPATROLGRIDS );
		pp->fHasKeys							= bp->fHasKeys;
		pp->ubCivilianGroup				= bp->ubCivilianGroup;

		return; //done
	}
	CreateDetailedPlacementGivenBasicPlacementInfo( pp, bp );
	pp->ubScheduleID			= spp->ubScheduleID;
	//Replace any of the new placement's attributes with static attributes.
	if( spp->bExpLevel		!= -1 )			pp->bExpLevel			=	spp->bExpLevel;
	if( spp->bLife				!= -1 )			pp->bLife					= spp->bLife;
	if( spp->bLifeMax			!= -1 )			pp->bLifeMax			= spp->bLifeMax;
	if( spp->bMarksmanship!= -1 )			pp->bMarksmanship	= spp->bMarksmanship;
	if( spp->bStrength		!= -1 )			pp->bStrength			= spp->bStrength;
	if( spp->bAgility			!= -1 )			pp->bAgility			= spp->bAgility;
	if( spp->bDexterity		!= -1 )			pp->bDexterity		= spp->bDexterity;
	if( spp->bWisdom			!= -1 )			pp->bWisdom				= spp->bWisdom;
	if( spp->bLeadership	!= -1 )			pp->bLeadership		= spp->bLeadership;
	if( spp->bExplosive		!= -1 )			pp->bExplosive		= spp->bExplosive;
	if( spp->bMedical			!= -1 )			pp->bMedical			= spp->bMedical;
	if( spp->bMechanical	!= -1 )			pp->bMechanical		= spp->bMechanical;
	if( spp->bMorale			!= -1 )			pp->bMorale				= spp->bMorale;

	pp->fVisible = spp->fVisible;
	if( spp->fVisible )
	{
		strcpy(pp->HeadPal,  spp->HeadPal);
		strcpy(pp->PantsPal, spp->PantsPal);
		strcpy(pp->VestPal,  spp->VestPal);
		strcpy(pp->SkinPal,  spp->SkinPal);
	}

	//This isn't perfect, however, it blindly brings over the items from the static
	//detailed placement.  Due to the order of things, other items would
	for( i = 0; i < NUM_INV_SLOTS; i++ )
	{
		//copy over static items and empty slots that are droppable (signifies a forced empty slot)
		if( spp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
		{
			pp->Inv[i] = spp->Inv[i];
			//memcpy( pp->Inv, spp->Inv, sizeof( OBJECTTYPE ) * NUM_INV_SLOTS );
			//return;
		}
	}
	if ( !gGameOptions.fGunNut )
	{
		ReplaceExtendedGuns( pp, bp->ubSoldierClass );
	}
	if( bp->ubSoldierClass != SOLDIER_CLASS_NONE && bp->ubSoldierClass != SOLDIER_CLASS_CREATURE && bp->ubSoldierClass != SOLDIER_CLASS_MINER )
	{
		GenerateRandomEquipment( pp, bp->ubSoldierClass, bp->bRelativeEquipmentLevel);
	}
}


static void UpdateStaticDetailedPlacementWithProfileInformation(SOLDIERCREATE_STRUCT* spp, UINT8 ubProfile);


//Used to update a existing soldier's attributes with the new static detailed placement info.  This is used
//by the editor upon exiting the editor into the game, to update the existing soldiers with new information.
//This gives flexibility of testing mercs.  Upon entering the editor again, this call will reset all the
//mercs to their original states.
void UpdateSoldierWithStaticDetailedInformation( SOLDIERTYPE *s, SOLDIERCREATE_STRUCT *spp )
{
	//First, check to see if the soldier has a profile.  If so, then it'll extract the information
	//and update the soldier with the profile information instead.  This has complete override
	//authority.
	if( spp->ubProfile != NO_PROFILE )
	{
		TacticalCopySoldierFromProfile( s, spp );
		UpdateStaticDetailedPlacementWithProfileInformation( spp, spp->ubProfile );
		SetSoldierAnimationSurface( s, s->usAnimState );
		CreateSoldierPalettes( s );
		return;
	}

	switch( spp->ubSoldierClass )
	{ //If the soldier is an administrator, then
		case SOLDIER_CLASS_ADMINISTRATOR:
		case SOLDIER_CLASS_ARMY:
		case SOLDIER_CLASS_ELITE:
			GeneratePaletteForSoldier( s, spp->ubSoldierClass );
			break;
	}

	if( spp->bExpLevel != -1 )
	{ //We have a static experience level, so generate all of the soldier's attributes.
		INT8 bBaseAttribute;
		s->bExpLevel = spp->bExpLevel;
		//Set the minimum base attribute
		bBaseAttribute = 49 + ( 4 * s->bExpLevel );

		//Roll enemy's combat statistics, taking bExpLevel into account.
		//Stat range is currently 40-99, slightly bell-curved around the bExpLevel
		s->bLifeMax				= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bLife	  			= s->bLifeMax;
		s->bAgility				= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bDexterity			= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bMarksmanship	= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bMedical				= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bMechanical		= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bExplosive			= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bLeadership		= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bStrength			= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bWisdom				= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bMorale				= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	}
	//Replace any soldier attributes with any static values in the detailed placement.
	if( spp->bLife				!= -1 )			s->bLife					= spp->bLife;
	if( spp->bLifeMax			!= -1 )			s->bLifeMax				= spp->bLifeMax;
	if( spp->bMarksmanship!= -1 )			s->bMarksmanship	= spp->bMarksmanship;
	if( spp->bStrength		!= -1 )			s->bStrength			= spp->bStrength;
	if( spp->bAgility			!= -1 )			s->bAgility				= spp->bAgility;
	if( spp->bDexterity		!= -1 )			s->bDexterity			= spp->bDexterity;
	if( spp->bWisdom			!= -1 )			s->bWisdom				= spp->bWisdom;
	if( spp->bLeadership	!= -1 )			s->bLeadership		= spp->bLeadership;
	if( spp->bExplosive		!= -1 )			s->bExplosive			= spp->bExplosive;
	if( spp->bMedical			!= -1 )			s->bMedical				= spp->bMedical;
	if( spp->bMechanical	!= -1 )			s->bMechanical		= spp->bMechanical;
	if( spp->bMorale			!= -1 )			s->bMorale				= spp->bMorale;

	//life can't exceed the life max.
	if( s->bLife > s->bLifeMax )
		s->bLife = s->bLifeMax;

	s->ubScheduleID		=	spp->ubScheduleID;

	//Copy over the current inventory list.
	memcpy( s->inv, spp->Inv, sizeof( OBJECTTYPE ) * NUM_INV_SLOTS );
}


//In the case of setting a profile ID in order to extract a soldier from the profile array, we
//also want to copy that information to the static detailed placement, for editor viewing purposes.
static void UpdateStaticDetailedPlacementWithProfileInformation(SOLDIERCREATE_STRUCT* spp, UINT8 ubProfile)
{
	UINT32					cnt;
	MERCPROFILESTRUCT * pProfile;

	spp->ubProfile = ubProfile;

	pProfile = &(gMercProfiles[ ubProfile ]);

	SET_PALETTEREP_ID ( spp->HeadPal,		pProfile->HAIR );
	SET_PALETTEREP_ID ( spp->VestPal,		pProfile->VEST );
	SET_PALETTEREP_ID ( spp->SkinPal,		pProfile->SKIN );
	SET_PALETTEREP_ID ( spp->PantsPal,  pProfile->PANTS );

	wcscpy( spp->name, pProfile->zNickname );

	spp->bLife										= pProfile->bLife;
	spp->bLifeMax									= pProfile->bLifeMax;
	spp->bAgility									= pProfile->bAgility;
	spp->bLeadership							= pProfile->bLeadership;
	spp->bDexterity								= pProfile->bDexterity;
	spp->bStrength								= pProfile->bStrength;
	spp->bWisdom									= pProfile->bWisdom;
	spp->bExpLevel								= pProfile->bExpLevel;
	spp->bMarksmanship						= pProfile->bMarksmanship;
	spp->bMedical									= pProfile->bMedical;
	spp->bMechanical							= pProfile->bMechanical;
	spp->bExplosive								= pProfile->bExplosive;

	spp->bBodyType								= pProfile->ubBodyType;

	// Copy over inv if we want to
	for ( cnt = 0; cnt < NUM_INV_SLOTS; cnt++ )
	{
		CreateItems( pProfile->inv[ cnt ], pProfile->bInvStatus[ cnt ], pProfile->bInvNumber[ cnt ], &(spp->Inv[ cnt ]) );
	}
}

//When the editor modifies the soldier's relative attribute level,
//this function is called to update that information.
void ModifySoldierAttributesWithNewRelativeLevel( SOLDIERTYPE *s, INT8 bRelativeAttributeLevel )
{
	INT8 bBaseAttribute;
	//Set the experience level based on the relative attribute level
	// NOTE OF WARNING: THIS CURRENTLY IGNORES THE ENEMY CLASS (ADMIN/REG/ELITE) FOR CALCULATING LEVEL & ATTRIBUTES

	// Rel level 0: Lvl 1, 1: Lvl 2-3, 2: Lvl 4-5, 3: Lvl 6-7, 4: Lvl 8-9
	s->bExpLevel = (INT8)(2 * bRelativeAttributeLevel + Random(2));

	s->bExpLevel = max( 1, s->bExpLevel ); //minimum level of 1
	s->bExpLevel = min( 9, s->bExpLevel ); //maximum level of 9

	//Set the minimum base attribute
	bBaseAttribute = 49 + ( 4 * s->bExpLevel );

	//Roll enemy's combat statistics, taking bExpLevel into account.
	//Stat range is currently 40-99, slightly bell-curved around the bExpLevel
	s->bLifeMax				= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bLife	  			= s->bLifeMax;
	s->bAgility				= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bDexterity			= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bMarksmanship	= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bMedical				= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bMechanical		= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bExplosive			= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bLeadership		= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bStrength			= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bWisdom				= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bMorale				= (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
}


void ForceSoldierProfileID( SOLDIERTYPE *pSoldier, UINT8 ubProfileID )
{
	SOLDIERCREATE_STRUCT CreateStruct;

	memset( &CreateStruct, 0, sizeof( CreateStruct ) );
	CreateStruct.ubProfile = ubProfileID;


	TacticalCopySoldierFromProfile( pSoldier, &CreateStruct );

	// Delete face and re-create
	DeleteSoldierFace( pSoldier );

	// Init face
	pSoldier->iFaceIndex = InitSoldierFace( pSoldier );

	// Update animation, palettes
	SetSoldierAnimationSurface( pSoldier, pSoldier->usAnimState );

	// Re-Create palettes
	CreateSoldierPalettes( pSoldier );

}

#define CENTRAL_GRIDNO 13202
#define CENTRAL_RADIUS 30


static SOLDIERTYPE* ReserveTacticalSoldierForAutoresolve(UINT8 ubSoldierClass)
{
	INT32 i, iStart, iEnd;
	SOLDIERTYPE *pSoldier;
	//This code looks for a soldier of specified type that currently exists in tactical and
	//returns the pointer to that soldier.  This is used when copying the exact status of
	//all remaining enemy troops (or creatures) to finish the battle in autoresolve.  To
	//signify that the troop has already been reserved, we simply set their gridno to NOWHERE.
	if( ubSoldierClass != SOLDIER_CLASS_CREATURE )
	{ //use the enemy team
		iStart = gTacticalStatus.Team[ ENEMY_TEAM ].bFirstID;
		iEnd = gTacticalStatus.Team[ ENEMY_TEAM ].bLastID;
	}
	else
	{ //use the creature team
		iStart = gTacticalStatus.Team[ CREATURE_TEAM ].bFirstID;
		iEnd = gTacticalStatus.Team[ CREATURE_TEAM ].bLastID;
	}
	for( i = iStart; i <= iEnd; i++ )
	{
		SOLDIERTYPE* const s = GetMan(i);
		if (s->bActive && s->bInSector && s->bLife != 0 && s->sGridNo != NOWHERE)
		{
			if (s->ubSoldierClass == ubSoldierClass)
			{
				//reserve this soldier
				s->sGridNo = NOWHERE;

				//Allocate and copy the soldier
				pSoldier = (SOLDIERTYPE*)MemAlloc( sizeof( SOLDIERTYPE ) );
				if( !pSoldier )
					return NULL;
				*pSoldier = *s;

				//Assign a bogus ID, then return it
				pSoldier->ubID = 255;
				return pSoldier;
			}
		}
	}
	return NULL;
}

//USED BY STRATEGIC AI and AUTORESOLVE
SOLDIERTYPE* TacticalCreateAdministrator()
{
	BASIC_SOLDIERCREATE_STRUCT bp;
	SOLDIERCREATE_STRUCT pp;

	if( guiCurrentScreen == AUTORESOLVE_SCREEN && !gfPersistantPBI )
	{
		return ReserveTacticalSoldierForAutoresolve( SOLDIER_CLASS_ADMINISTRATOR );
	}

	memset( &bp, 0, sizeof( BASIC_SOLDIERCREATE_STRUCT ) );
	memset( &pp, 0, sizeof( SOLDIERCREATE_STRUCT ) );
	RandomizeRelativeLevel( &( bp.bRelativeAttributeLevel ), SOLDIER_CLASS_ADMINISTRATOR );
	RandomizeRelativeLevel( &( bp.bRelativeEquipmentLevel ), SOLDIER_CLASS_ADMINISTRATOR );
	bp.bTeam = ENEMY_TEAM;
	bp.bOrders = SEEKENEMY;
	bp.bAttitude = (INT8) Random( MAXATTITUDES );
	bp.bBodyType = -1;
	bp.ubSoldierClass = SOLDIER_CLASS_ADMINISTRATOR;
	CreateDetailedPlacementGivenBasicPlacementInfo( &pp, &bp );
	SOLDIERTYPE* const pSoldier = TacticalCreateSoldier(&pp);
	if ( pSoldier )
	{
		// send soldier to centre of map, roughly
		pSoldier->sNoiseGridno = (INT16) (CENTRAL_GRIDNO + ( Random( CENTRAL_RADIUS * 2 + 1 ) - CENTRAL_RADIUS ) + ( Random( CENTRAL_RADIUS * 2 + 1 ) - CENTRAL_RADIUS ) * WORLD_COLS);
		pSoldier->ubNoiseVolume = MAX_MISC_NOISE_DURATION;
	}
	return( pSoldier );
}

//USED BY STRATEGIC AI and AUTORESOLVE
SOLDIERTYPE* TacticalCreateArmyTroop()
{
	BASIC_SOLDIERCREATE_STRUCT bp;
	SOLDIERCREATE_STRUCT pp;

	if( guiCurrentScreen == AUTORESOLVE_SCREEN && !gfPersistantPBI )
	{
		return ReserveTacticalSoldierForAutoresolve( SOLDIER_CLASS_ARMY );
	}

	memset( &bp, 0, sizeof( BASIC_SOLDIERCREATE_STRUCT ) );
	memset( &pp, 0, sizeof( SOLDIERCREATE_STRUCT ) );
	RandomizeRelativeLevel( &( bp.bRelativeAttributeLevel ), SOLDIER_CLASS_ARMY );
	RandomizeRelativeLevel( &( bp.bRelativeEquipmentLevel ), SOLDIER_CLASS_ARMY );
	bp.bTeam = ENEMY_TEAM;
	bp.bOrders	= SEEKENEMY;
	bp.bAttitude = (INT8) Random( MAXATTITUDES );
	bp.bBodyType = -1;
	bp.ubSoldierClass = SOLDIER_CLASS_ARMY;
	CreateDetailedPlacementGivenBasicPlacementInfo( &pp, &bp );
	SOLDIERTYPE* const pSoldier = TacticalCreateSoldier(&pp);
	if ( pSoldier )
	{
		// send soldier to centre of map, roughly
		pSoldier->sNoiseGridno = (INT16) (CENTRAL_GRIDNO + ( Random( CENTRAL_RADIUS * 2 + 1 ) - CENTRAL_RADIUS ) + ( Random( CENTRAL_RADIUS * 2 + 1 ) - CENTRAL_RADIUS ) * WORLD_COLS);
		pSoldier->ubNoiseVolume = MAX_MISC_NOISE_DURATION;
	}
	return( pSoldier );
}

//USED BY STRATEGIC AI and AUTORESOLVE
SOLDIERTYPE* TacticalCreateEliteEnemy()
{
	BASIC_SOLDIERCREATE_STRUCT bp;
	SOLDIERCREATE_STRUCT pp;

	if( guiCurrentScreen == AUTORESOLVE_SCREEN && !gfPersistantPBI )
	{
		return ReserveTacticalSoldierForAutoresolve( SOLDIER_CLASS_ELITE );
	}

	memset( &bp, 0, sizeof( BASIC_SOLDIERCREATE_STRUCT ) );
	memset( &pp, 0, sizeof( SOLDIERCREATE_STRUCT ) );

	RandomizeRelativeLevel( &( bp.bRelativeAttributeLevel ), SOLDIER_CLASS_ELITE );
	RandomizeRelativeLevel( &( bp.bRelativeEquipmentLevel ), SOLDIER_CLASS_ELITE );
	bp.bTeam = ENEMY_TEAM;
	bp.bOrders	= SEEKENEMY;
	bp.bAttitude = (INT8) Random( MAXATTITUDES );
	bp.bBodyType = -1;
	bp.ubSoldierClass = SOLDIER_CLASS_ELITE;
	CreateDetailedPlacementGivenBasicPlacementInfo( &pp, &bp );

	//SPECIAL!  Certain events in the game can cause profiled NPCs to become enemies.  The two cases are
	//adding Mike and Iggy.  We will only add one NPC in any given combat and the conditions for setting
	//the associated facts are done elsewhere.  There is also another place where NPCs can get added, which
	//is in AddPlacementToWorld() used for inserting defensive enemies.
	//NOTE:  We don't want to add Mike or Iggy if this is being called from autoresolve!
	OkayToUpgradeEliteToSpecialProfiledEnemy( &pp );

	SOLDIERTYPE* const pSoldier = TacticalCreateSoldier(&pp);
	if ( pSoldier )
	{
		// send soldier to centre of map, roughly
		pSoldier->sNoiseGridno = (INT16)(CENTRAL_GRIDNO + ( Random( CENTRAL_RADIUS * 2 + 1 ) - CENTRAL_RADIUS ) + ( Random( CENTRAL_RADIUS * 2 + 1 ) - CENTRAL_RADIUS ) * WORLD_COLS);
		pSoldier->ubNoiseVolume = MAX_MISC_NOISE_DURATION;
	}
	return( pSoldier );
}

SOLDIERTYPE* TacticalCreateMilitia( UINT8 ubMilitiaClass )
{
	BASIC_SOLDIERCREATE_STRUCT bp;
	SOLDIERCREATE_STRUCT pp;

	memset( &bp, 0, sizeof( BASIC_SOLDIERCREATE_STRUCT ) );
	memset( &pp, 0, sizeof( SOLDIERCREATE_STRUCT ) );
	RandomizeRelativeLevel( &( bp.bRelativeAttributeLevel ), ubMilitiaClass );
	RandomizeRelativeLevel( &( bp.bRelativeEquipmentLevel ), ubMilitiaClass );
	bp.bTeam = MILITIA_TEAM;
	bp.ubSoldierClass = ubMilitiaClass;
	bp.bOrders = STATIONARY;
	bp.bAttitude = (INT8) Random( MAXATTITUDES );
	//bp.bAttitude = AGGRESSIVE;
	bp.bBodyType = -1;
	CreateDetailedPlacementGivenBasicPlacementInfo( &pp, &bp );
	return TacticalCreateSoldier(&pp);
}

SOLDIERTYPE* TacticalCreateCreature( INT8 bCreatureBodyType )
{
	BASIC_SOLDIERCREATE_STRUCT bp;
	SOLDIERCREATE_STRUCT pp;

	if( guiCurrentScreen == AUTORESOLVE_SCREEN && !gfPersistantPBI )
	{
		return ReserveTacticalSoldierForAutoresolve( SOLDIER_CLASS_CREATURE );
	}

	memset( &bp, 0, sizeof( BASIC_SOLDIERCREATE_STRUCT ) );
	memset( &pp, 0, sizeof( SOLDIERCREATE_STRUCT ) );
	RandomizeRelativeLevel( &( bp.bRelativeAttributeLevel ), SOLDIER_CLASS_CREATURE );
	RandomizeRelativeLevel( &( bp.bRelativeEquipmentLevel ), SOLDIER_CLASS_CREATURE );
	bp.bTeam = CREATURE_TEAM;
	bp.ubSoldierClass = SOLDIER_CLASS_CREATURE;
	bp.bOrders	= SEEKENEMY;
	bp.bAttitude = AGGRESSIVE;
	bp.bBodyType = bCreatureBodyType;
	CreateDetailedPlacementGivenBasicPlacementInfo( &pp, &bp );
	return TacticalCreateSoldier(&pp);
}


void RandomizeRelativeLevel( INT8 *pbRelLevel, UINT8 ubSoldierClass )
{
	UINT8 ubLocationModifier;
	INT8 bRollModifier;
	INT8 bRoll, bAdjustedRoll;


	// We now adjust the relative level by location on the map, so enemies in NE corner will be generally very crappy (lots
	// of bad and poor, with avg about best), while enemies in the SW will have lots of great and good, with avg about as
	// lousy as it gets.  Militia are generally unmodified by distance, and never get bad or great at all.

	// this returns 0 to DIFF_FACTOR_PALACE_DISTANCE (0 to +30)
	ubLocationModifier = GetLocationModifier( ubSoldierClass );

	// convert to 0 to 10 (divide by 3), the subtract 5 to get a range of -5 to +5
	bRollModifier = ( INT8 ) ( ubLocationModifier / ( DIFF_FACTOR_PALACE_DISTANCE / 10 ) ) - 5;

	// roll a number from 0 to 9
	bRoll = ( INT8 ) Random( 10 );

	// adjust by the modifier (giving -5 to +14)
	bAdjustedRoll = bRoll + bRollModifier;

	if ( SOLDIER_CLASS_MILITIA( ubSoldierClass ) )
	{
		// Militia never get to roll bad/great results at all (to avoid great equipment drops from them if killed)
		bAdjustedRoll = __max( 1, bAdjustedRoll );
		bAdjustedRoll = __min( 8, bAdjustedRoll );
		if( IsAutoResolveActive() )
		{ //Artificially strengthen militia strength for sake of gameplay
			bAdjustedRoll++;
		}
	}
	else
	{
		// max-min this to a range of 0-9
		bAdjustedRoll = __max( 0, bAdjustedRoll );
		bAdjustedRoll = __min( 9, bAdjustedRoll );
		if( IsAutoResolveActive() )
		{ //Artificially weaken enemy/creature strength for sake of gameplay
			if( bAdjustedRoll > 0 )
			{
				bAdjustedRoll--;
			}
		}
	}

	switch ( bAdjustedRoll )
	{
		case 0:
			// bad
			*pbRelLevel = 0;
			break;
		case 1:
		case 2:
			// poor
			*pbRelLevel = 1;
			break;
		case 3:
		case 4:
		case 5:
		case 6:
			// average
			*pbRelLevel = 2;
			break;
		case 7:
		case 8:
			// good
			*pbRelLevel = 3;
			break;
		case 9:
			// great
			*pbRelLevel = 4;
			break;

		default:
			Assert( FALSE );
			*pbRelLevel = 2;
			break;
	}

/*
	if( IsAutoResolveActive() )
	{ //Artificially strengthen militia strength for sake of gameplay
		if ( SOLDIER_CLASS_MILITIA( ubSoldierClass ) )
		{
			*pbRelLevel = 4;
		}
		else
		{
			*pbRelLevel = 0;
		}
	}
*/
}


//This function shouldn't be called outside of tactical
void QuickCreateProfileMerc( INT8 bTeam, UINT8 ubProfileID )
{
	// Create guy # X
	SOLDIERCREATE_STRUCT		MercCreateStruct;
	INT16										sWorldX, sWorldY, sSectorX, sSectorY, sGridX, sGridY;
	UINT16 usMapPos;

	if ( GetMouseXY( &sGridX, &sGridY ) )
	{
		usMapPos = MAPROWCOLTOPOS( sGridY, sGridX );
		// Get Grid Coordinates of mouse
		if ( GetMouseWorldCoordsInCenter( &sWorldX, &sWorldY ) )
		{
			GetCurrentWorldSector( &sSectorX, &sSectorY );


			memset( &MercCreateStruct, 0, sizeof( MercCreateStruct ) );
			MercCreateStruct.bTeam				= bTeam;
			MercCreateStruct.ubProfile		= ubProfileID;
			MercCreateStruct.sSectorX			= sSectorX;
			MercCreateStruct.sSectorY			= sSectorY;
			MercCreateStruct.bSectorZ			= gbWorldSectorZ;
			MercCreateStruct.sInsertionGridNo		= usMapPos;

			RandomizeNewSoldierStats( &MercCreateStruct );

			SOLDIERTYPE* const s = TacticalCreateSoldier(&MercCreateStruct);
			if (s != NULL)
			{
				AddSoldierToSector(s);

				// So we can see them!
				AllTeamsLookForAll(NO_INTERRUPTS);
			}
		}
	}
}


static void CopyProfileItems(SOLDIERTYPE* const pSoldier, const SOLDIERCREATE_STRUCT* const pCreateStruct)
{
	UINT32								cnt, cnt2;
	MERCPROFILESTRUCT *		pProfile;
	OBJECTTYPE 						Obj;
	UINT32								uiMoneyLeft, uiMoneyLimitInSlot;
	INT8									bSlot;

	pProfile = &(gMercProfiles[pCreateStruct->ubProfile]);

	// Copy over inv if we want to
	if ( pCreateStruct->fCopyProfileItemsOver || pSoldier->bTeam != OUR_TEAM )
	{
		if (pCreateStruct->fPlayerMerc)
		{
			// do some special coding to put stuff in the profile in better-looking
			// spots
			memset( pSoldier->inv, 0, NUM_INV_SLOTS * sizeof( OBJECTTYPE ) );
			for ( cnt = 0; cnt < NUM_INV_SLOTS; cnt++ )
			{
				if ( pProfile->inv[ cnt ] != NOTHING )
				{
					CreateItems( pProfile->inv[ cnt ], pProfile->bInvStatus[ cnt ], pProfile->bInvNumber[ cnt ], &Obj );
					if (Item[Obj.usItem].fFlags & ITEM_ATTACHMENT)
					{
						// try to find the appropriate item to attach to!
						for ( cnt2 = 0; cnt2 < NUM_INV_SLOTS; cnt2++ )
						{
							if ( pSoldier->inv[ cnt2 ].usItem != NOTHING && ValidAttachment( Obj.usItem, pSoldier->inv[ cnt2 ].usItem ) )
							{
								AttachObject( NULL, &(pSoldier->inv[ cnt2 ]), &Obj );
								break;
							}
						}
						if (cnt2 == NUM_INV_SLOTS)
						{
							// oh well, couldn't find anything to attach to!
							AutoPlaceObject( pSoldier, &Obj, FALSE );
						}
					}
					else
					{
						AutoPlaceObject( pSoldier, &Obj, FALSE );
					}

				}
			}
			pProfile->usOptionalGearCost = 0;
		}
		else
		{
			for ( cnt = 0; cnt < NUM_INV_SLOTS; cnt++ )
			{
				if ( pProfile->inv[ cnt ] != NOTHING )
				{
					if ( Item[ pProfile->inv[ cnt ] ].usItemClass == IC_KEY )
					{
						// since keys depend on 2 values, they pretty much have to be hardcoded.
						// and if a case isn't handled here it's better to not give any key than
						// to provide one which doesn't work and would confuse everything.
						switch( pCreateStruct->ubProfile )
						{
							case BREWSTER:
								if ( pProfile->inv[ cnt ] >= KEY_1 && pProfile->inv[ cnt ] <= KEY_32)
								{
									CreateKeyObject( &(pSoldier->inv[ cnt ] ), pProfile->bInvNumber[ cnt ], 19 );
								}
								else
								{
									memset( &(pSoldier->inv[cnt]), 0, sizeof( OBJECTTYPE ) );
								}
								break;
							case SKIPPER:
								if ( pProfile->inv[ cnt ] >= KEY_1 && pProfile->inv[ cnt ] <= KEY_32)
								{
									CreateKeyObject( &(pSoldier->inv[ cnt ] ), pProfile->bInvNumber[ cnt ], 11 );
								}
								else
								{
									memset( &(pSoldier->inv[cnt]), 0, sizeof( OBJECTTYPE ) );
								}
								break;
							case DOREEN:
								if ( pProfile->inv[ cnt ] >= KEY_1 && pProfile->inv[ cnt ] <= KEY_32)
								{
									CreateKeyObject( &(pSoldier->inv[ cnt ] ), pProfile->bInvNumber[ cnt ], 32 );
								}
								else
								{
									memset( &(pSoldier->inv[cnt]), 0, sizeof( OBJECTTYPE ) );
								}
								break;
							default:
								memset( &(pSoldier->inv[cnt]), 0, sizeof( OBJECTTYPE ) );
								break;
						}
					}
					else
					{
						CreateItems( pProfile->inv[ cnt ], pProfile->bInvStatus[ cnt ], pProfile->bInvNumber[ cnt ], &(pSoldier->inv[ cnt ] ) );
					}
					if ( pProfile->inv[ cnt ] == ROCKET_RIFLE || pProfile->inv[ cnt ] == AUTO_ROCKET_RIFLE )
					{
						pSoldier->inv[ cnt ].ubImprintID = pSoldier->ubProfile;
					}
					if (gubItemDroppableFlag[cnt])
					{
						if (pProfile->ubInvUndroppable & gubItemDroppableFlag[cnt])
						{
							pSoldier->inv[cnt].fFlags |= OBJECT_UNDROPPABLE;
						}
					}
				}
				else
				{
					memset( &(pSoldier->inv[cnt]), 0, sizeof( OBJECTTYPE ) );
				}
			}
			if (pProfile->uiMoney > 0)
			{
				uiMoneyLeft = pProfile->uiMoney;
				bSlot = FindEmptySlotWithin( pSoldier, BIGPOCK1POS, SMALLPOCK8POS );

				// add in increments of
				while ( bSlot != NO_SLOT )
				{
					uiMoneyLimitInSlot = MAX_MONEY_PER_SLOT;
					if ( bSlot >= SMALLPOCK1POS )
					{
						uiMoneyLimitInSlot /= 2;
					}

					CreateItem( MONEY, 100, &( pSoldier->inv[ bSlot ] ) );
					if ( uiMoneyLeft > uiMoneyLimitInSlot )
					{
						// fill pocket with money
						pSoldier->inv[ bSlot ].uiMoneyAmount = uiMoneyLimitInSlot;
						uiMoneyLeft -= uiMoneyLimitInSlot;
					}
					else
					{
						pSoldier->inv[ bSlot ].uiMoneyAmount = uiMoneyLeft;
						// done!
						break;
					}

					bSlot = FindEmptySlotWithin( pSoldier, BIGPOCK1POS, SMALLPOCK8POS );
				}
			}
		}
	}
}

//SPECIAL!  Certain events in the game can cause profiled NPCs to become enemies.  The two cases are
//adding Mike and Iggy.  We will only add one NPC in any given combat and the conditions for setting
//the associated facts are done elsewhere.  The function will set the profile for the SOLDIERCREATE_STRUCT
//and the rest will be handled automatically so long the ubProfile field doesn't get changed.
//NOTE:  We don't want to add Mike or Iggy if this is being called from autoresolve!
void OkayToUpgradeEliteToSpecialProfiledEnemy( SOLDIERCREATE_STRUCT *pp )
{
	if( !gfProfiledEnemyAdded && gubEnemyEncounterCode != ENEMY_ENCOUNTER_CODE && gubEnemyEncounterCode != ENEMY_INVASION_CODE )
	{
		if( gubFact[ FACT_MIKE_AVAILABLE_TO_ARMY ] == 1 && !pp->fOnRoof )
		{
			gubFact[ FACT_MIKE_AVAILABLE_TO_ARMY ] = 2; //so it fails all subsequent checks
			pp->ubProfile = MIKE;
			gfProfiledEnemyAdded = TRUE;
		}
		else if( gubFact[ FACT_IGGY_AVAILABLE_TO_ARMY ] == 1 && !pp->fOnRoof )
		{
			gubFact[ FACT_IGGY_AVAILABLE_TO_ARMY ] = 2; //so it fails all subsequent checks
			pp->ubProfile = IGGY;
			gfProfiledEnemyAdded = TRUE;
		}
	}
}


void TrashAllSoldiers( )
{
  INT32       cnt;
  SOLDIERTYPE *pSoldier;

	cnt = 0;

	for ( pSoldier = MercPtrs[ cnt ]; cnt < MAX_NUM_SOLDIERS; pSoldier++, cnt++ )
	{
		if (pSoldier->bActive) TacticalRemoveSoldier(pSoldier);
	}
}


static UINT8 GetLocationModifier(UINT8 ubSoldierClass)
{
	UINT8 ubLocationModifier;
	UINT8 ubPalaceDistance;
	INT16 sSectorX, sSectorY, sSectorZ;
	INT8 bTownId;
	BOOLEAN fSuccess;


	// where is all this taking place?
	fSuccess = GetCurrentBattleSectorXYZ( &sSectorX, &sSectorY, &sSectorZ );
	Assert( fSuccess );

	// ignore sSectorZ - treat any underground enemies as if they were on the surface!
	bTownId = GetTownIdForSector( sSectorX, sSectorY );


	switch ( bTownId )
	{
		case ORTA:
		case TIXA:
			// enemy troops in these special places are stronger than geography would indicate
			ubPalaceDistance = 4;
			break;

		case ALMA:
			// enemy troops in these special places are stronger than geography would indicate
			ubPalaceDistance = 10;
			break;

		default:
			// how far is this sector from the palace ?
			// the distance returned is in sectors, and the possible range is about 0-20
			ubPalaceDistance = GetPythDistanceFromPalace( sSectorX, sSectorY );
			if ( ubPalaceDistance > MAX_PALACE_DISTANCE )
			{
				ubPalaceDistance = MAX_PALACE_DISTANCE;
			}
	}

	// adjust for distance from Queen's palace (P3) (0 to +30)
	ubLocationModifier = ( ( MAX_PALACE_DISTANCE - ubPalaceDistance ) * DIFF_FACTOR_PALACE_DISTANCE ) / MAX_PALACE_DISTANCE;

	return( ubLocationModifier );
}



// grab the distance from the palace
UINT8 GetPythDistanceFromPalace( INT16 sSectorX, INT16 sSectorY )
{
	UINT8 ubDistance = 0;
	INT16 sRows = 0, sCols = 0;
	float fValue = 0.0;

	// grab number of rows and cols
	sRows = (INT16)(abs((sSectorX) - ( PALACE_SECTOR_X )));
	sCols = (INT16)(abs((sSectorY) - ( PALACE_SECTOR_Y )));


	// apply Pythagoras's theorem for right-handed triangle:
	// dist^2 = rows^2 + cols^2, so use the square root to get the distance
	fValue = ( float )sqrt(( float )(sRows * sRows) + ( float )(sCols * sCols));

	if(  fmod( fValue, 1.0 ) >= 0.50 )
	{
		ubDistance = (UINT8)( 1 + fValue );
	}
	else
	{
		ubDistance = ( UINT8 )fValue;
	}

	return( ubDistance );
}


static void ReduceHighExpLevels(INT8* pbExpLevel)
{
	UINT8 ubRoll;
	// important: must reset these to 0 by default for logic to work!
	UINT8 ubChanceLvl8 = 0;
	UINT8 ubChanceLvl7 = 0;
	UINT8 ubChanceLvl6 = 0;
	UINT8 ubChanceLvl5 = 0;


	// this function reduces the experience levels of very high level enemies to something that player can compete with
	// for interrupts.  It doesn't affect attributes and skills, those are rolled prior to this reduction!


	// adjust for game difficulty level
	switch( gGameOptions.ubDifficultyLevel )
	{
		case DIF_LEVEL_EASY:
			// max level: 6
			switch ( *pbExpLevel )
			{
				case 6:
					ubChanceLvl6 = 25;
					ubChanceLvl5 = 100;
					break;
				case 7:
					ubChanceLvl6 = 50;
					ubChanceLvl5 = 100;
					break;
				case 8:
					ubChanceLvl6 = 75;
					ubChanceLvl5 = 100;
					break;
				case 9:
					ubChanceLvl6 = 100;
					break;
			}
			break;

		case DIF_LEVEL_MEDIUM:
			// max level: 7
			switch ( *pbExpLevel )
			{
				case 7:
					ubChanceLvl7 = 25;
					ubChanceLvl6 = 100;
					break;
				case 8:
					ubChanceLvl7 = 50;
					ubChanceLvl6 = 100;
					break;
				case 9:
					ubChanceLvl7 = 75;
					ubChanceLvl6 = 100;
					break;
			}
			break;

		case DIF_LEVEL_HARD:
			// max level: 8
			switch ( *pbExpLevel )
			{
				case 8:
					ubChanceLvl8 = 25;
					ubChanceLvl7 = 100;
					break;
				case 9:
					ubChanceLvl8 = 50;
					ubChanceLvl7 = 100;
					break;
			}
			break;
	}


	ubRoll = ( UINT8 ) Random( 100 );

	if			( ubRoll < ubChanceLvl8 )
		*pbExpLevel	= 8;
	else if ( ubRoll < ubChanceLvl7 )
		*pbExpLevel	= 7;
	else if ( ubRoll < ubChanceLvl6 )
		*pbExpLevel	= 6;
	else if ( ubRoll < ubChanceLvl5 )
		*pbExpLevel	= 5;
	// else leave it alone

}
