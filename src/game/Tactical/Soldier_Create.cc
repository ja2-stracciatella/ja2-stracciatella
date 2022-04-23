#include "Soldier_Create.h"
#include "Overhead.h"
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
#include "JAScreens.h"
#include "SoundMan.h"
#include "MemMan.h"
#include "Debug.h"
#include "ScreenIDs.h"
#include "WorldDef.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "policy/GamePolicy.h"
#include "externalized/strategic/BloodCatSpawnsModel.h"

#include <string_theory/string>

#include <algorithm>
#include <iterator>
#include <math.h>

// THESE 3 DIFFICULTY FACTORS MUST ALWAYS ADD UP TO 100% EXACTLY!!!
#define DIFF_FACTOR_PLAYER_PROGRESS		50
#define DIFF_FACTOR_PALACE_DISTANCE		30
#define DIFF_FACTOR_GAME_DIFFICULTY		20

// additional difficulty modifiers
#define DIFF_MODIFIER_SOME_PROGRESS		+5
#define DIFF_MODIFIER_NO_INCOME		-5
#define DIFF_MODIFIER_DRASSEN_MILITIA		+10


#define PALACE_SECTOR_X			3
#define PALACE_SECTOR_Y			16

#define MAX_PALACE_DISTANCE			20


Observable<SOLDIERTYPE*> OnSoldierCreated = {};

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
	pCreateStruct->bLifeMax = (UINT8)Random(50)+50;
	pCreateStruct->bLife = pCreateStruct->bLifeMax;
	pCreateStruct->bAgility = (UINT8)Random(50)+50;
	pCreateStruct->bDexterity = (UINT8)Random(50)+50;
	pCreateStruct->bExpLevel = 1 + (UINT8)Random(4);

	// Randomize skills (for now)
	pCreateStruct->bMarksmanship = (UINT8)Random(50)+50;
	pCreateStruct->bMedical = (UINT8)Random(50)+50;
	pCreateStruct->bMechanical = (UINT8)Random(50)+50;
	pCreateStruct->bExplosive = (UINT8)Random(50)+50;
	pCreateStruct->bLeadership = (UINT8)Random(50)+50;
	pCreateStruct->bStrength = (UINT8)Random(50)+50;
	pCreateStruct->bWisdom = (UINT8)Random(50)+50;
	pCreateStruct->bAttitude = (INT8) Random( MAXATTITUDES );
	pCreateStruct->bOrders = FARPATROL;
	pCreateStruct->bMorale = 50;
	pCreateStruct->bAIMorale = MORALE_FEARLESS;
}


static void CopyProfileItems(SOLDIERTYPE&, SOLDIERCREATE_STRUCT const&);
static void InitSoldierStruct(SOLDIERTYPE&);
static void TacticalCopySoldierFromCreateStruct(SOLDIERTYPE&, SOLDIERCREATE_STRUCT const&);
static void TacticalCopySoldierFromProfile(SOLDIERTYPE&, SOLDIERCREATE_STRUCT const&);


SOLDIERTYPE* TacticalCreateSoldier(SOLDIERCREATE_STRUCT const& c)
try
{
	// Kris: Huge no no! See the header file for description of static detailed
	// placements. If this expression ever evaluates to true, then it will expose
	// serious problems. Simply returning won't help.
	Assert(!c.fStatic);

	ProfileID const profile = c.ubProfile;
	INT8      const team_id = c.bTeam;

	// Given team, get an ID for this guy!
	SOLDIERTYPE* s;
	SoldierID    id;
	if (guiCurrentScreen == AUTORESOLVE_SCREEN)
	{
		// We are creating a dynamically allocated soldier for autoresolve.
		s  = new SOLDIERTYPE{};
		id = 255;
	}
	else
	{
		TacticalTeamType const& team = gTacticalStatus.Team[team_id];
		id = team.bFirstID;
		// ATE: If we are a vehicle, and a player, start at a different slot (2 - max)
		if (team_id == OUR_TEAM)
		{
			switch (profile != NO_PROFILE ? GetProfile(profile).ubBodyType : c.bBodyType)
			{
				case ELDORADO:
				case HUMVEE:
				case ICECREAMTRUCK:
				case JEEP:
					id = team.bLastID - 1;
					break;
			}
		}

		UINT8 const last_id = team.bLastID;
		for (;;)
		{
			s = &GetMan(id);
			if (!s->bActive) break;
			if (++id > last_id) return 0;
		}
	}

	// Some values initialized here but could be changed before going to the common one
	InitSoldierStruct(*s);
	s->ubID                   = id;
	s->uiUniqueSoldierIdValue = guiCurrentUniqueSoldierId++;

	if (profile != NO_PROFILE)
	{
		TacticalCopySoldierFromProfile(*s, c);
	}
	else
	{
		TacticalCopySoldierFromCreateStruct(*s, c);
	}

	if (team_id == OUR_TEAM)
	{
		s->uiStatusFlags |= SOLDIER_PC;
		s->bVisible       = 1;
	}
	else
	{
		s->uiStatusFlags |= SOLDIER_ENEMY;
	}

	s->bTeam = team_id;

	// if WE_SEE_WHAT_MILITIA_SEES
	if (team_id == MILITIA_TEAM) s->bVisible = 1;

	if (profile != NO_PROFILE)
	{
		CopyProfileItems(*s, c);
		if (team_id == OUR_TEAM) InitSoldierFace(*s);
	}

	s->bActionPoints        = CalcActionPoints(s);
	s->bInitialActionPoints = s->bActionPoints;
	s->bSide                = gTacticalStatus.Team[team_id].bSide;
	s->sSector              = c.sSector;
	s->ubInsertionDirection = c.bDirection;
	s->bDesiredDirection    = c.bDirection;
	s->bDominantDir         = c.bDirection;
	s->bDirection           = c.bDirection;
	s->sInsertionGridNo     = c.sInsertionGridNo;
	s->bOldLife             = s->bLifeMax;

	if (team_id == CIV_TEAM)
	{
		s->bNeutral =
			profile            == WARDEN        ? FALSE :
			s->ubCivilianGroup == NON_CIV_GROUP ? TRUE  :
			gTacticalStatus.fCivGroupHostile[s->ubCivilianGroup] != CIV_GROUP_HOSTILE;

		// Weaken stats based on the bodytype of the civilian.
		if (profile == NO_PROFILE)
		{
			switch (s->ubBodyType)
			{
				case REGMALE:
				case BIGMALE:
				case STOCKYMALE:
				case REGFEMALE:
					// No adjustments necessary for these "healthy" bodytypes.
					break;
				case FATCIV:
					//fat, so slower
					s->bAgility = 30 + Random(26); //30 - 55
					break;
				case MANCIV:
					s->bLife = s->bLifeMax = 35 + Random(26); //35 - 60
					break;
				case MINICIV:
				case DRESSCIV:
					s->bLife = s->bLifeMax = 30 + Random(16); //30 - 45
					break;
				case HATKIDCIV:
				case KIDCIV:
					s->bLife = s->bLifeMax = 20 + Random(16); //20 - 35
					break;
				case CRIPPLECIV:
					s->bLife = s->bLifeMax = 20 + Random(26); //20 - 45
					s->bAgility = 30 + Random(16); // 30 - 45
					break;
			}
		}
	}
	else if (team_id == CREATURE_TEAM)
	{
		// bloodcats are neutral to start out
		if (s->ubBodyType == BLOODCAT)
		{
			s->bNeutral = TRUE;
		}
		// otherwise (creatures) false
	}

	// If not given a profile num, set a randomized default battle sound set and
	// then adjust it according to body type
	if (profile == NO_PROFILE)
	{
		s->ubBattleSoundID = Random(3);
	}

	// ATE: TEMP: No enemy women mercs (unless elite)!
	if (profile == NO_PROFILE &&
		team_id == ENEMY_TEAM &&
		s->ubBodyType == REGFEMALE &&
		s->ubSoldierClass != SOLDIER_CLASS_ELITE)
	{
		s->ubBodyType = REGMALE + Random(3);
	}

	// ATE: Set some values for variation in anims
	if (s->ubBodyType == BIGMALE)
	{
		s->uiAnimSubFlags |= SUB_ANIM_BIGGUYTHREATENSTANCE;
	}

	// For inventory, look for any face class items that may be located in the big
	// pockets and if found, move that item to a face slot and clear the pocket!
	if (team_id != OUR_TEAM)
	{
		bool second_face_item = false;
		for (INT32 i = BIGPOCK1POS; i <= BIGPOCK4POS; ++i)
		{
			OBJECTTYPE& o = s->inv[i];
			if (!(GCM->getItem(o.usItem)->isFace())) continue;

			if (!second_face_item)
			{
				// Don't check for compatibility, automatically assume there are no head
				// positions filled.
				second_face_item = true;
				s->inv[HEAD1POS] = o;
				o = OBJECTTYPE{};
			}
			else if (CompatibleFaceItem(s->inv[HEAD1POS].usItem, o.usItem))
			{
				s->inv[HEAD2POS] = o;
				o = OBJECTTYPE{};
				break;
			}
		}

		// If an army guy has camouflage, roll to determine whether they start
		// camouflaged
		if (guiCurrentScreen != AUTORESOLVE_SCREEN &&
				team_id == ENEMY_TEAM                  &&
				FindObj(s, CAMOUFLAGEKIT) != NO_SLOT   &&
				Random(5) < SoldierDifficultyLevel(s))
		{ // Start camouflaged
			s->bCamo = 100;
		}
	}

	// Set some flags, actions based on what body type we are.
	// NOTE:  Be very careful what you do in this section!
	// It is possible to override editor settings, especially orders and attitude.
	// In those cases, you can check for !gfEditMode (not in editor).
	switch (s->ubBodyType)
	{
		case HATKIDCIV:
		case KIDCIV:
			s->ubBattleSoundID = Random(2);
			break;

		case REGFEMALE:
		case MINICIV:
		case DRESSCIV:
			s->ubBattleSoundID = 7 + Random(2);
			s->bNormalSmell    = NORMAL_HUMAN_SMELL_STRENGTH;
			break;

		case BLOODCAT:
			AssignCreatureInventory(s);
			s->bNormalSmell   = NORMAL_HUMAN_SMELL_STRENGTH;
			s->uiStatusFlags |= SOLDIER_ANIMAL;
			break;

		case ADULTFEMALEMONSTER:
		case AM_MONSTER:
		case YAF_MONSTER:
		case YAM_MONSTER:
		case LARVAE_MONSTER:
		case INFANT_MONSTER:
		case QUEENMONSTER:
			AssignCreatureInventory(s);
			s->ubCaller = NOBODY;
			if (!gfEditMode)
			{
				s->bOrders   = FARPATROL;
				s->bAttitude = AGGRESSIVE;
			}
			s->uiStatusFlags |= SOLDIER_MONSTER;
			s->bMonsterSmell  = NORMAL_CREATURE_SMELL_STRENGTH;
			break;

		case COW:
			s->uiStatusFlags |= SOLDIER_ANIMAL;
			s->bNormalSmell   = COW_SMELL_STRENGTH;
			break;

		case CROW:
			s->uiStatusFlags |= SOLDIER_ANIMAL;
			break;

		case ROBOTNOWEAPON:
			s->uiStatusFlags |= SOLDIER_ROBOT;
			break;

		case HUMVEE:
		case ELDORADO:
		case ICECREAMTRUCK:
		case JEEP:
		case TANK_NW:
		case TANK_NE:
		{
			s->uiStatusFlags |= SOLDIER_VEHICLE;

			UINT8 ubVehicleID = 0;
			switch (s->ubBodyType)
			{
				case HUMVEE:
					ubVehicleID = HUMMER;
					s->bNeutral = TRUE;
					break;

				case ELDORADO:
					ubVehicleID = ELDORADO_CAR;
					s->bNeutral = TRUE;
					break;

				case ICECREAMTRUCK:
					ubVehicleID = ICE_CREAM_TRUCK;
					s->bNeutral = TRUE;
					break;

				case JEEP:
					ubVehicleID = JEEP_CAR;
					break;

				case TANK_NW:
				case TANK_NE:
					ubVehicleID = TANK_CAR;
					break;
			}

			s->bVehicleID =
				c.fUseGivenVehicle ? c.bUseGivenVehicleID :
				// TODO: verify, it's suspicious we pass a z coord instead of gridno
				(INT8) AddVehicleToList(s->sSector, s->sSector.z, ubVehicleID);
			SetVehicleValuesIntoSoldierType(s);
			break;
		}

		default:
			s->bNormalSmell = NORMAL_HUMAN_SMELL_STRENGTH;
			break;
	}

	if (guiCurrentScreen == AUTORESOLVE_SCREEN)
	{
		s->sSector = SGPSector(GetAutoResolveSectorID());
		return s;
	}

	OnSoldierCreated(s);

	Assert(s->usAnimState == STANDING);

	// We are set to create the merc, stuff after here can fail
	CreateSoldierCommon(*s);

	if (c.fOnRoof && FlatRoofAboveGridNo(c.sInsertionGridNo))
	{
		SetSoldierHeight(s, SECOND_LEVEL_Z_OFFSET);
	}

	AddManToTeam(team_id);
	return s;
}
catch (...) { return 0; }


SOLDIERTYPE* TacticalCreateSoldierFromExisting(const SOLDIERTYPE* const existing)
{
	Assert(guiCurrentScreen != AUTORESOLVE_SCREEN);
	Assert(existing->face  == NULL);
	Assert(existing->light == NULL);

	SOLDIERTYPE& s = GetMan(existing->ubID);
	s = *existing;

	if (s.ubProfile != NO_PROFILE &&
		s.bTeam == OUR_TEAM &&
		!(s.uiStatusFlags & SOLDIER_VEHICLE))
	{
		InitSoldierFace(s);
	}

	if (s.ubBodyType == HUMVEE || s.ubBodyType == ICECREAMTRUCK)
	{
		s.bNeutral = TRUE;
	}

	CreateSoldierCommon(s);
	return &s;
}


static void TacticalCopySoldierFromProfile(SOLDIERTYPE& s, SOLDIERCREATE_STRUCT const& c)
{
	ProfileID         const  pid = c.ubProfile;
	MERCPROFILESTRUCT const& p   = GetProfile(pid);

	s.HeadPal  = p.HAIR;
	s.VestPal  = p.VEST;
	s.SkinPal  = p.SKIN;
	s.PantsPal = p.PANTS;

	s.ubProfile       = pid;
	s.ubScheduleID    = c.ubScheduleID;
	s.bHasKeys        = c.fHasKeys;

	s.name = p.zNickname;

	s.bLife           = p.bLife;
	s.bLifeMax        = p.bLifeMax;
	s.bAgility        = p.bAgility;
	s.bLeadership     = p.bLeadership;
	s.bDexterity      = p.bDexterity;
	s.bStrength       = p.bStrength;
	s.bWisdom         = p.bWisdom;
	s.bExpLevel       = p.bExpLevel;
	s.bMarksmanship   = p.bMarksmanship;
	s.bMedical        = p.bMedical;
	s.bMechanical     = p.bMechanical;
	s.bExplosive      = p.bExplosive;

	s.uiAnimSubFlags  = p.uiBodyTypeSubFlags;
	s.ubBodyType      = p.ubBodyType;
	s.ubCivilianGroup = p.ubCivilianGroup;

	s.ubSkillTrait1   = p.bSkillTrait;
	s.ubSkillTrait2   = p.bSkillTrait2;

	s.bOrders         = c.bOrders;
	s.bAttitude       = c.bAttitude;
	s.bDirection      = c.bDirection;
	s.bPatrolCnt      = c.bPatrolCnt;
	memcpy(s.usPatrolGrid, c.sPatrolGrid, sizeof(s.usPatrolGrid));

	if (HAS_SKILL_TRAIT(&s, CAMOUFLAGED))
	{
		// Set camouflaged to 100 automatically
		s.bCamo = 100;
	}
}


enum { PINKSKIN, TANSKIN, DARKSKIN, BLACKSKIN, NUMSKINS };
enum {
	WHITEHEAD, BLACKHEAD, //black skin (only this line )
	BROWNHEAD, //dark skin (this line plus all above)
	BLONDEHEAD, REDHEAD, //pink/tan skin (this line plus all above )
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
				if(pSoldier->ubBodyType == REGFEMALE ||
					pSoldier->ubBodyType == MINICIV ||
					pSoldier->ubBodyType == DRESSCIV ||
					pSoldier->ubBodyType == HATKIDCIV ||
					pSoldier->ubBodyType == KIDCIV)
				{
					hair = Random( NUMHEADS - 1 ) + 1;
				}
			}
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
				if(pSoldier->ubBodyType == REGFEMALE ||
					pSoldier->ubBodyType == MINICIV ||
					pSoldier->ubBodyType == DRESSCIV ||
					pSoldier->ubBodyType == HATKIDCIV ||
					pSoldier->ubBodyType == KIDCIV)
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
				if(pSoldier->ubBodyType == REGFEMALE ||
					pSoldier->ubBodyType == MINICIV ||
					pSoldier->ubBodyType == DRESSCIV ||
					pSoldier->ubBodyType == HATKIDCIV ||
					pSoldier->ubBodyType == KIDCIV)
				{
					hair = BLACKHEAD;
				}
			}
			break;
		default:
			SLOGA("Skin type not accounted for." );
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
			pSoldier->SkinPal = "PINKSKIN";
			break;
		case TANSKIN:
			pSoldier->SkinPal = "TANSKIN";
			break;
		case DARKSKIN:
			pSoldier->SkinPal = "DARKSKIN";
			break;
		case BLACKSKIN:
			pSoldier->SkinPal = "BLACKSKIN";
			break;
		default:
			SLOGA("Skin type not accounted for." );
			break;
	}

	//Choose hair color which uses the skin color to limit choices
	hair = ChooseHairColor( pSoldier, skin );
	switch( hair )
	{
		case BROWNHEAD:  pSoldier->HeadPal = "BROWNHEAD"; break;
		case BLACKHEAD:  pSoldier->HeadPal = "BLACKHEAD"; break;
		case WHITEHEAD:  pSoldier->HeadPal = "WHITEHEAD"; break;
		case BLONDEHEAD: pSoldier->HeadPal = "BLONDHEAD"; break;
		case REDHEAD:    pSoldier->HeadPal = "REDHEAD"  ; break;
		default:
			SLOGA("Hair type not accounted for.");
			break;
	}

	// OK, After skin, hair we could have a forced color scheme.. use here if so
	switch( ubSoldierClass )
	{
		case SOLDIER_CLASS_ADMINISTRATOR:
			pSoldier->VestPal  = "YELLOWVEST";
			pSoldier->PantsPal = "GREENPANTS";
			pSoldier->ubSoldierClass = ubSoldierClass;
			return;
		case SOLDIER_CLASS_ELITE:
			pSoldier->VestPal  = "BLACKSHIRT";
			pSoldier->PantsPal = "BLACKPANTS";
			pSoldier->ubSoldierClass = ubSoldierClass;
			return;
		case SOLDIER_CLASS_ARMY:
			pSoldier->VestPal  = "REDVEST";
			pSoldier->PantsPal = "GREENPANTS";
			pSoldier->ubSoldierClass = ubSoldierClass;
			return;
		case SOLDIER_CLASS_GREEN_MILITIA:
			pSoldier->VestPal  = "GREENVEST";
			pSoldier->PantsPal = "BEIGEPANTS";
			pSoldier->ubSoldierClass = ubSoldierClass;
			return;
		case SOLDIER_CLASS_REG_MILITIA:
			pSoldier->VestPal  = "JEANVEST";
			pSoldier->PantsPal = "BEIGEPANTS";
			pSoldier->ubSoldierClass = ubSoldierClass;
			return;
		case SOLDIER_CLASS_ELITE_MILITIA:
			pSoldier->VestPal  = "BLUEVEST";
			pSoldier->PantsPal = "BEIGEPANTS";
			pSoldier->ubSoldierClass = ubSoldierClass;
			return;
		case SOLDIER_CLASS_MINER:
			pSoldier->VestPal  = "greyVEST";
			pSoldier->PantsPal = "BEIGEPANTS";
			pSoldier->ubSoldierClass = ubSoldierClass;
			return;
	}

	//there are 2 clothing schemes, 1 for mercs and 1 for civilians.  The
	//merc clothing scheme is much larger and general and is an exclusive superset
	//of the civilian clothing scheme which means the civilians will choose the
	//merc clothing scheme often ( actually 60% of the time ).
	if (pSoldier->PantsPal.empty() || pSoldier->VestPal.empty())
	{
		fMercClothingScheme = TRUE;
		if( pSoldier->bTeam == CIV_TEAM && Random( 100 ) < 40 )
		{
			//40% chance of using cheezy civilian colors
			fMercClothingScheme = FALSE;
		}
		if( !fMercClothingScheme ) //CHEEZY CIVILIAN COLORS
		{
			if( Random( 100 ) < 30 )
			{
				//30% chance that the civilian will choose a gaudy yellow shirt with pants.
				pSoldier->VestPal = "GYELLOWSHIRT";
				switch( Random( 3 ) )
				{
					case 0:	pSoldier->PantsPal = "TANPANTS"  ; break;
					case 1:	pSoldier->PantsPal = "BEIGEPANTS"; break;
					case 2:	pSoldier->PantsPal = "GREENPANTS"; break;
				}
			}
			else
			{
				//70% chance that the civilian will choose jeans with a shirt.
				pSoldier->PantsPal = "JEANPANTS";
				switch( Random( 7 ) )
				{
					case 0:	pSoldier->VestPal = "WHITEVEST"  ; break;
					case 1:	pSoldier->VestPal = "BLACKSHIRT" ; break;
					case 2:	pSoldier->VestPal = "PURPLESHIRT"; break;
					case 3:	pSoldier->VestPal = "BLUEVEST"   ; break;
					case 4:	pSoldier->VestPal = "BROWNVEST"  ; break;
					case 5:	pSoldier->VestPal = "JEANVEST"   ; break;
					case 6:	pSoldier->VestPal = "REDVEST"    ; break;
				}
			}
			return;
		}
		//MERC COLORS
		switch( Random( 3 ) )
		{
			case 0:
				pSoldier->PantsPal = "GREENPANTS";
				switch( Random( 4 ) )
				{
					case 0: pSoldier->VestPal = "YELLOWVEST"; break;
					case 1: pSoldier->VestPal = "WHITEVEST" ; break;
					case 2: pSoldier->VestPal = "BROWNVEST" ; break;
					case 3: pSoldier->VestPal = "GREENVEST" ; break;
				}
				break;
			case 1:
				pSoldier->PantsPal = "TANPANTS";
				switch( Random( 8 ) )
				{
					case 0: pSoldier->VestPal = "YELLOWVEST"; break;
					case 1: pSoldier->VestPal = "WHITEVEST" ; break;
					case 2: pSoldier->VestPal = "BLACKSHIRT"; break;
					case 3: pSoldier->VestPal = "BLUEVEST"  ; break;
					case 4: pSoldier->VestPal = "BROWNVEST" ; break;
					case 5: pSoldier->VestPal = "GREENVEST" ; break;
					case 6: pSoldier->VestPal = "JEANVEST"  ; break;
					case 7: pSoldier->VestPal = "REDVEST"   ; break;
				}
				break;
			case 2:
				pSoldier->PantsPal = "BLUEPANTS";
				switch( Random( 4 ) )
				{
					case 0: pSoldier->VestPal = "YELLOWVEST"; break;
					case 1: pSoldier->VestPal = "WHITEVEST" ; break;
					case 2: pSoldier->VestPal = "REDVEST"   ; break;
					case 3: pSoldier->VestPal = "BLACKSHIRT"; break;
				}
				break;
		}
	}
}


static void TacticalCopySoldierFromCreateStruct(SOLDIERTYPE& s, SOLDIERCREATE_STRUCT const& c)
{
	s.ubProfile       = NO_PROFILE;

	s.bLife           = c.bLife;
	s.bLifeMax        = c.bLifeMax;
	s.bAgility        = c.bAgility;
	s.bDexterity      = c.bDexterity;
	s.bExpLevel       = c.bExpLevel;

	s.bMarksmanship   = c.bMarksmanship;
	s.bMedical        = c.bMedical;
	s.bMechanical     = c.bMechanical;
	s.bExplosive      = c.bExplosive;
	s.bLeadership     = c.bLeadership;
	s.bStrength       = c.bStrength;
	s.bWisdom         = c.bWisdom;

	s.bAttitude       = c.bAttitude;
	s.bOrders         = c.bOrders;
	s.bMorale         = c.bMorale;
	s.bAIMorale       = c.bAIMorale;
	s.ubBodyType      = c.bBodyType;
	s.ubCivilianGroup = c.ubCivilianGroup;

	s.ubScheduleID    = c.ubScheduleID;
	s.bHasKeys        = c.fHasKeys;
	s.ubSoldierClass  = c.ubSoldierClass;

	if (c.fVisible)
	{
		s.HeadPal = c.HeadPal;
		s.PantsPal = c.PantsPal;
		s.VestPal = c.VestPal;
		s.SkinPal = c.SkinPal;
	}

	// KM:  March 25, 1999
	// Assign nightops traits to enemies/militia
	switch (s.ubSoldierClass)
	{
		INT32 chance;
		UINT8 expert_progess;
		INT32 expert_chance;

		case SOLDIER_CLASS_ELITE:
		case SOLDIER_CLASS_ELITE_MILITIA:
			chance         = 40;
			expert_progess = 40;
			expert_chance  = 30;
			goto maybe_nightops;

		case SOLDIER_CLASS_ARMY:
		case SOLDIER_CLASS_REG_MILITIA:
			chance         =  0;
			expert_progess = 50;
			expert_chance  = 20;
maybe_nightops:
			UINT8 const progress = HighestPlayerProgressPercentage();
			// ramp chance from 0 to 40% over the course of 60% progress
			chance += progress < 60 ? progress * 2 / 3 : 40;
			if (Chance(chance))
			{
				s.ubSkillTrait1 = NIGHTOPS;
				if (progress >= expert_progess && Chance(expert_chance))
				{
					s.ubSkillTrait2 = NIGHTOPS;
				}
			}
			break;
	}

	// KM:  November 10, 1997
	// Adding patrol points
	// CAUTION:  CONVERTING SIGNED TO UNSIGNED though the values should never be
	// negative.
	s.bPatrolCnt = c.bPatrolCnt;
	memcpy(s.usPatrolGrid, c.sPatrolGrid, sizeof(s.usPatrolGrid));

	// Kris:  November 10, 1997
	// Expanded the default names based on team.
	ST::string name;
	switch (c.bTeam)
	{
		case ENEMY_TEAM:   name = TacticalStr[ENEMY_TEAM_MERC_NAME];   break;
		case MILITIA_TEAM: name = TacticalStr[MILITIA_TEAM_MERC_NAME]; break;

		case CIV_TEAM:
			name =
				s.ubSoldierClass == SOLDIER_CLASS_MINER ? TacticalStr[CIV_TEAM_MINER_NAME]   :
				s.ubBodyType     == CROW                ? TacticalStr[CROW_HIT_LOCATION_STR] :
				TacticalStr[CIV_TEAM_MERC_NAME];
			break;

		case CREATURE_TEAM:
			name =
				s.ubBodyType == BLOODCAT ? gzLateLocalizedString[STR_LATE_36] :
				TacticalStr[CREATURE_TEAM_MERC_NAME];
			break;

		default: goto no_name; // XXX fishy
	}
	s.name = name;
no_name:

	GeneratePaletteForSoldier(&s, c.ubSoldierClass);

	memcpy(s.inv, c.Inv, sizeof(s.inv));
}


static void InitSoldierStruct(SOLDIERTYPE& s)
{
	s = SOLDIERTYPE{};
	s.bVisible                  = -1;
	s.face                      = 0;
	s.bMorale                   = DEFAULT_MORALE;
	s.attacker                  = 0;
	s.previous_attacker         = 0;
	s.next_to_previous_attacker = 0;
	s.light                     = 0;
	s.ubDesiredHeight           = NO_DESIRED_HEIGHT;
	s.bInSector                 = FALSE;
	s.sGridNo                   = NOWHERE;
	s.muzzle_flash              = 0;
	s.usPendingAnimation        = NO_PENDING_ANIMATION;
	s.ubPendingStanceChange     = NO_PENDING_STANCE;
	s.ubPendingDirection        = NO_PENDING_DIRECTION;
	s.ubPendingAction           = NO_PENDING_ACTION;
	s.bLastRenderVisibleValue   = -1;
	s.bBreath                   = 99;
	s.bBreathMax                = 100;
	s.bActive                   = TRUE;
	s.fShowLocator              = FALSE;
	s.sLastTarget               = NOWHERE;
	s.sAbsoluteFinalDestination = NOWHERE;
	s.sZLevelOverride           = -1;
	s.service_partner           = 0;
	s.ubAttackingHand           = HANDPOS;
	s.usAnimState               = STANDING;
	s.bInterruptDuelPts         = NO_INTERRUPT;
	s.bMoved                    = FALSE;
	s.robot_remote_holder       = 0;
	s.sNoiseGridno              = NOWHERE;
	s.ubPrevSectorID            = 255;
	s.bNextPatrolPnt            = 1;
	s.bCurrentCivQuote          = -1;
	s.bCurrentCivQuoteDelta     = 0;
	s.uiBattleSoundID           = NO_SAMPLE;
	s.xrayed_by                 = 0;
	s.uiXRayActivatedTime       = 0;
	s.bBulletsLeft              = 0;
	s.bVehicleUnderRepairID     = -1;
}


void InternalTacticalRemoveSoldier(SOLDIERTYPE& s, BOOLEAN const fRemoveVehicle)
{
	if (GetSelectedMan() == &s) SetSelectedMan(0);
	if (gUIFullTarget    == &s) gUIFullTarget   = 0;
	if (gpSMCurrentMerc  == &s) gpSMCurrentMerc = 0;

	if (!s.bActive) return;

	if (s.ubScheduleID) DeleteSchedule(s.ubScheduleID);

	if (s.uiStatusFlags & SOLDIER_VEHICLE && fRemoveVehicle)
	{
		RemoveVehicleFromList(GetVehicle(s.bVehicleID));
	}

	if (s.ubBodyType == CROW) HandleCrowLeave(&s);

	if (guiCurrentScreen != AUTORESOLVE_SCREEN)
	{
		RemoveCharacterFromSquads(&s);
		RemovePlayerFromTeamSlot(&s);
		RemoveSoldierFromGridNo(s);

		// Delete shadow of crow....
		if (s.pAniTile)
		{
			DeleteAniTile(s.pAniTile);
			s.pAniTile = 0;
		}

		if (!(s.uiStatusFlags & SOLDIER_OFF_MAP))
		{
			// Decrement men in sector number!
			RemoveManFromTeam(s.bTeam);
		}
		// people specified off-map have already been removed from their team count

		DeleteSoldier(s);
	}
	else
	{
		if (gfPersistantPBI) DeleteSoldier(s);
		delete &s;
	}
}


void TacticalRemoveSoldier(SOLDIERTYPE& s)
{
	InternalTacticalRemoveSoldier(s, TRUE);
}


static UINT8 GetLocationModifier(UINT8 ubSoldierClass);


// returns a soldier difficulty modifier from 0 to 100 based on player's progress, distance from the Palace, mining
// income, and playing difficulty level.  Used for generating soldier stats, equipment, and AI skill level.
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
	Assert((DIFF_FACTOR_PLAYER_PROGRESS + DIFF_FACTOR_PALACE_DISTANCE + DIFF_FACTOR_GAME_DIFFICULTY ) == 100 );


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
	pp->fStatic = FALSE;
	pp->ubProfile = NO_PROFILE;
	pp->sInsertionGridNo = bp->usStartingGridNo;
	pp->fCopyProfileItemsOver = FALSE;
	pp->bTeam = bp->bTeam;
	pp->ubSoldierClass = bp->ubSoldierClass;
	pp->ubCivilianGroup = bp->ubCivilianGroup;
	pp->ubScheduleID = 0;
	pp->sSector = gWorldSector;
	pp->fHasKeys = bp->fHasKeys;

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
					case 0:	pp->bBodyType = REGMALE; break;
					case 1:	pp->bBodyType = BIGMALE; break;
					case 2:	pp->bBodyType = STOCKYMALE; break;
					case 3:	pp->bBodyType = REGFEMALE; break;
				}
				break;
			case CIV_TEAM:
				if( pp->ubSoldierClass == SOLDIER_CLASS_MINER )
				{
					switch( Random( 3 ) )
					{
						//only strong and fit men can be miners.
						case 0:	pp->bBodyType = REGMALE; break;
						case 1:	pp->bBodyType = BIGMALE; break;
						case 2: pp->bBodyType = MANCIV; break;
					}
				}
				else
				{
					INT32 iRandom;
					iRandom = Random( 100 );
					if( iRandom < 8 )
					{
						//8% chance FATCIV
						pp->bBodyType = FATCIV;
					}
					else if( iRandom < 38 )
					{
						//30% chance MANCIV
						pp->bBodyType = MANCIV;
					}
					else if( iRandom < 57 )
					{
						//19% chance MINICIV
						pp->bBodyType = MINICIV;
					}
					else if( iRandom < 76 )
					{
						//19% chance DRESSCIV
						pp->bBodyType = DRESSCIV;
					}
					else if( iRandom < 88 )
					{
						//12% chance HATKIDCIV
						pp->bBodyType = HATKIDCIV;
					}
					else
					{
						//12% chance KIDCIV
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
	// DIFFICULTY FACTOR    EXP. LEVEL MODIFIER    LEVEL OF AVG REGULAR TROOP
	//   0 to 19            -2                     2
	//  20 to 39            -1                     3
	//  41 to 59            -0                     4
	//  60 to 79            +1                     5
	//  80 to 99            +2                     6
	// 100                  +3                     7    (can happen in P3 Meduna itself on HARD only!)
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
			AssertMsg(FALSE, String("Invalid bRelativeAttributeLevel = %d", bp->bRelativeAttributeLevel));
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
			pp->bExpLevel = bp->bRelativeAttributeLevel; //avg 2 (1-4)
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
					auto spawns = GCM->getBloodCatSpawnsOfSector(gWorldSector.AsByte());
					if (spawns && spawns->isLair)
					{
						pp->bExpLevel += gGameOptions.ubDifficultyLevel;
					}
					break;
			}
			break;

		default:
			pp->bExpLevel = bp->bRelativeAttributeLevel; //avg 2 (1-4)
			ubSoldierClass = SOLDIER_CLASS_NONE;
			break;
	}


	// clamp experience level to 1-9 or the externalised values (elites only)
	if (ubSoldierClass == SOLDIER_CLASS_ELITE)
	{
		pp->bExpLevel = MAX(gamepolicy(enemy_elite_minimum_level), pp->bExpLevel);
		pp->bExpLevel = MIN(gamepolicy(enemy_elite_maximum_level), pp->bExpLevel);
	} else {
		pp->bExpLevel = MAX(1, pp->bExpLevel); //minimum exp. level of 1
		pp->bExpLevel = MIN(9, pp->bExpLevel); //maximum exp. level of 9
	}

	ubStatsLevel = pp->bExpLevel + bStatsModifier;
	ubStatsLevel = MIN( 9, ubStatsLevel );	//maximum stats level of 9

	//Set the minimum base attribute
	bBaseAttribute = 49 + ( 4 * ubStatsLevel );

	//Roll soldier's statistics and skills
	//Stat range is currently 49-99, bell-curved around a range of 16 values dependent on the stats level
	pp->bLifeMax = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bLife = pp->bLifeMax;
	pp->bAgility = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bDexterity = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));

	pp->bMarksmanship = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bMedical = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bMechanical = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bExplosive = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bLeadership = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bStrength = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bWisdom = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	pp->bMorale = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));

	if (ubSoldierClass != SOLDIER_CLASS_ELITE)
	{
		// CJC: now calculate the REAL experience level if in the really upper end
		// we're skipping elites, since they have externalised level bounds
		ReduceHighExpLevels( &( pp->bExpLevel ) );
	}

	pp->fVisible = 0;

	pp->fOnRoof = bp->fOnRoof;

	pp->ubSoldierClass = ubSoldierClass;

	//Transfer over the patrol points.
	pp->bPatrolCnt = bp->bPatrolCnt;
	memcpy( pp->sPatrolGrid, bp->sPatrolGrid, sizeof( INT16 ) * MAXPATROLGRIDS );

	//If it is a detailed placement, don't do this yet, as detailed placements may have their
	//own equipment.
	if(!bp->fDetailedPlacement &&
		ubSoldierClass != SOLDIER_CLASS_NONE &&
		ubSoldierClass != SOLDIER_CLASS_CREATURE &&
		ubSoldierClass != SOLDIER_CLASS_MINER)
	{
		GenerateRandomEquipment(pp, ubSoldierClass, bp->bRelativeEquipmentLevel);
	}
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
	*spp = SOLDIERCREATE_STRUCT{};
	spp->fStatic = TRUE;
	spp->ubProfile = NO_PROFILE;
	spp->sInsertionGridNo = bp->usStartingGridNo;
	spp->fCopyProfileItemsOver = FALSE;
	spp->bTeam = bp->bTeam;
	spp->ubSoldierClass = bp->ubSoldierClass;
	spp->ubCivilianGroup = bp->ubCivilianGroup;
	spp->ubScheduleID = 0;
	spp->sSector = gWorldSector;
	spp->fHasKeys = bp->fHasKeys;

	//Pass over mandatory information specified from the basic placement
	spp->bOrders = bp->bOrders;
	spp->bAttitude = bp->bAttitude;
	spp->bDirection = bp->bDirection;

	//Only creatures have mandatory body types specified.
	spp->bBodyType = spp->bTeam == CREATURE_TEAM ? bp->bBodyType : BODY_RANDOM;

	//Set up all possible static values.
	//By setting them all to -1, that signifies that the attribute isn't static.
	//The static placement needs to be later *regenerated* in order to create a valid soldier.
	spp->bExpLevel = -1;
	spp->bLifeMax = -1;
	spp->bLife    = -1;
	spp->bAgility = -1;
	spp->bDexterity = -1;
	spp->bMarksmanship = -1;
	spp->bMedical = -1;
	spp->bMechanical = -1;
	spp->bExplosive = -1;
	spp->bLeadership = -1;
	spp->bStrength = -1;
	spp->bWisdom = -1;
	spp->bMorale = -1;

	spp->fVisible =  0;

	spp->fOnRoof = bp->fOnRoof;

	//Transfer over the patrol points.
	spp->bPatrolCnt = bp->bPatrolCnt;
	memcpy( spp->sPatrolGrid, bp->sPatrolGrid, sizeof( INT16 ) * MAXPATROLGRIDS );

	//Starts with nothing
	for( i = 0; i < NUM_INV_SLOTS; i++ )
	{
		spp->Inv[ i ] = OBJECTTYPE{};
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

	*pp = SOLDIERCREATE_STRUCT{};
	pp->fOnRoof = spp->fOnRoof = bp->fOnRoof;
	pp->fStatic = FALSE;
	pp->ubSoldierClass = bp->ubSoldierClass;
	//Generate the new placement
	pp->ubProfile = spp->ubProfile;
	if( pp->ubProfile != NO_PROFILE )
	{
		// Copy over team
		pp->bTeam = bp->bTeam;

		pp->bDirection = bp->bDirection;
		pp->sInsertionGridNo = bp->usStartingGridNo;

		//ATE: Copy over sector coordinates from profile to create struct
		pp->sSector = gMercProfiles[ pp->ubProfile ].sSector;

		pp->ubScheduleID = spp->ubScheduleID;

		pp->bOrders = bp->bOrders;
		pp->bAttitude = bp->bAttitude;
		pp->bDirection = bp->bDirection;
		pp->bPatrolCnt = bp->bPatrolCnt;
		memcpy( pp->sPatrolGrid, bp->sPatrolGrid, sizeof( INT16 ) * MAXPATROLGRIDS );
		pp->fHasKeys = bp->fHasKeys;
		pp->ubCivilianGroup = bp->ubCivilianGroup;

		return; //done
	}
	CreateDetailedPlacementGivenBasicPlacementInfo( pp, bp );
	pp->ubScheduleID = spp->ubScheduleID;
	//Replace any of the new placement's attributes with static attributes.
	if( spp->bExpLevel != -1 ) pp->bExpLevel = spp->bExpLevel;
	if( spp->bLife != -1 ) pp->bLife = spp->bLife;
	if( spp->bLifeMax != -1 ) pp->bLifeMax = spp->bLifeMax;
	if( spp->bMarksmanship!= -1 ) pp->bMarksmanship = spp->bMarksmanship;
	if( spp->bStrength != -1 ) pp->bStrength = spp->bStrength;
	if( spp->bAgility != -1 ) pp->bAgility = spp->bAgility;
	if( spp->bDexterity != -1 ) pp->bDexterity = spp->bDexterity;
	if( spp->bWisdom != -1 ) pp->bWisdom = spp->bWisdom;
	if( spp->bLeadership != -1 ) pp->bLeadership = spp->bLeadership;
	if( spp->bExplosive != -1 ) pp->bExplosive = spp->bExplosive;
	if( spp->bMedical != -1 ) pp->bMedical = spp->bMedical;
	if( spp->bMechanical != -1 ) pp->bMechanical = spp->bMechanical;
	if( spp->bMorale != -1 ) pp->bMorale = spp->bMorale;

	pp->fVisible = spp->fVisible;
	if( spp->fVisible )
	{
		pp->HeadPal = spp->HeadPal;
		pp->PantsPal = spp->PantsPal;
		pp->VestPal = spp->VestPal;
		pp->SkinPal = spp->SkinPal;
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
		TacticalCopySoldierFromProfile(*s, *spp);
		UpdateStaticDetailedPlacementWithProfileInformation( spp, spp->ubProfile );
		SetSoldierAnimationSurface( s, s->usAnimState );
		CreateSoldierPalettes(*s);
		return;
	}

	switch( spp->ubSoldierClass )
	{
		//If the soldier is an administrator, then
		case SOLDIER_CLASS_ADMINISTRATOR:
		case SOLDIER_CLASS_ARMY:
		case SOLDIER_CLASS_ELITE:
			GeneratePaletteForSoldier( s, spp->ubSoldierClass );
			break;
	}

	if( spp->bExpLevel != -1 )
	{
		//We have a static experience level, so generate all of the soldier's attributes.
		INT8 bBaseAttribute;
		s->bExpLevel = spp->bExpLevel;
		//Set the minimum base attribute
		bBaseAttribute = 49 + ( 4 * s->bExpLevel );

		//Roll enemy's combat statistics, taking bExpLevel into account.
		//Stat range is currently 40-99, slightly bell-curved around the bExpLevel
		s->bLifeMax = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bLife    = s->bLifeMax;
		s->bAgility = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bDexterity = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bMarksmanship = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bMedical = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bMechanical = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bExplosive = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bLeadership = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bStrength = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bWisdom = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
		s->bMorale = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	}
	//Replace any soldier attributes with any static values in the detailed placement.
	if( spp->bLife != -1 ) s->bLife = spp->bLife;
	if( spp->bLifeMax != -1 ) s->bLifeMax = spp->bLifeMax;
	if( spp->bMarksmanship!= -1 ) s->bMarksmanship = spp->bMarksmanship;
	if( spp->bStrength != -1 ) s->bStrength = spp->bStrength;
	if( spp->bAgility != -1 ) s->bAgility = spp->bAgility;
	if( spp->bDexterity != -1 ) s->bDexterity = spp->bDexterity;
	if( spp->bWisdom != -1 ) s->bWisdom = spp->bWisdom;
	if( spp->bLeadership != -1 ) s->bLeadership = spp->bLeadership;
	if( spp->bExplosive != -1 ) s->bExplosive = spp->bExplosive;
	if( spp->bMedical != -1 ) s->bMedical = spp->bMedical;
	if( spp->bMechanical != -1 ) s->bMechanical = spp->bMechanical;
	if( spp->bMorale != -1 ) s->bMorale = spp->bMorale;

	//life can't exceed the life max.
	if( s->bLife > s->bLifeMax )
		s->bLife = s->bLifeMax;

	s->ubScheduleID = spp->ubScheduleID;

	//Copy over the current inventory list.
	memcpy( s->inv, spp->Inv, sizeof( OBJECTTYPE ) * NUM_INV_SLOTS );
}


//In the case of setting a profile ID in order to extract a soldier from the profile array, we
//also want to copy that information to the static detailed placement, for editor viewing purposes.
static void UpdateStaticDetailedPlacementWithProfileInformation(SOLDIERCREATE_STRUCT* spp, UINT8 ubProfile)
{
	UINT32 cnt;

	spp->ubProfile = ubProfile;

	MERCPROFILESTRUCT& p = GetProfile(ubProfile);

	spp->HeadPal  = p.HAIR;
	spp->VestPal  = p.VEST;
	spp->SkinPal  = p.SKIN;
	spp->PantsPal = p.PANTS;

	spp->name = p.zNickname;

	spp->bLife         = p.bLife;
	spp->bLifeMax      = p.bLifeMax;
	spp->bAgility      = p.bAgility;
	spp->bLeadership   = p.bLeadership;
	spp->bDexterity    = p.bDexterity;
	spp->bStrength     = p.bStrength;
	spp->bWisdom       = p.bWisdom;
	spp->bExpLevel     = p.bExpLevel;
	spp->bMarksmanship = p.bMarksmanship;
	spp->bMedical      = p.bMedical;
	spp->bMechanical   = p.bMechanical;
	spp->bExplosive    = p.bExplosive;

	spp->bBodyType     = p.ubBodyType;

	// Copy over inv if we want to
	for ( cnt = 0; cnt < NUM_INV_SLOTS; cnt++ )
	{
		CreateItems(p.inv[cnt], p.bInvStatus[cnt], p.bInvNumber[cnt], &spp->Inv[cnt]);
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

	s->bExpLevel = MAX( 1, s->bExpLevel ); //minimum level of 1
	s->bExpLevel = MIN( 9, s->bExpLevel ); //maximum level of 9

	//Set the minimum base attribute
	bBaseAttribute = 49 + ( 4 * s->bExpLevel );

	//Roll enemy's combat statistics, taking bExpLevel into account.
	//Stat range is currently 40-99, slightly bell-curved around the bExpLevel
	s->bLifeMax = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bLife = s->bLifeMax;
	s->bAgility = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bDexterity = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bMarksmanship = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bMedical = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bMechanical = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bExplosive = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bLeadership = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bStrength = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bWisdom = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
	s->bMorale = (INT8)(bBaseAttribute + Random( 9 ) + Random( 8 ));
}


void ForceSoldierProfileID( SOLDIERTYPE *pSoldier, UINT8 ubProfileID )
{
	SOLDIERCREATE_STRUCT CreateStruct;

	CreateStruct = SOLDIERCREATE_STRUCT{};
	CreateStruct.ubProfile = ubProfileID;
	TacticalCopySoldierFromProfile(*pSoldier, CreateStruct);

	// Delete face and re-create
	DeleteSoldierFace( pSoldier );

	// Init face
	InitSoldierFace(*pSoldier);

	// Update animation, palettes
	SetSoldierAnimationSurface( pSoldier, pSoldier->usAnimState );

	// Re-Create palettes
	CreateSoldierPalettes(*pSoldier);
}

#define CENTRAL_GRIDNO				13202
#define CENTRAL_RADIUS				30


static SOLDIERTYPE* ReserveTacticalSoldierForAutoresolve(UINT8 ubSoldierClass)
try
{
	//This code looks for a soldier of specified type that currently exists in tactical and
	//returns the pointer to that soldier.  This is used when copying the exact status of
	//all remaining enemy troops (or creatures) to finish the battle in autoresolve.  To
	//signify that the troop has already been reserved, we simply set their gridno to NOWHERE.
	const INT8 team = (ubSoldierClass == SOLDIER_CLASS_CREATURE ? CREATURE_TEAM : ENEMY_TEAM);
	FOR_EACH_IN_TEAM(s, team)
	{
		if (s->bInSector && s->bLife != 0 && s->sGridNo != NOWHERE)
		{
			if (s->ubSoldierClass == ubSoldierClass)
			{
				//reserve this soldier
				s->sGridNo = NOWHERE;

				//Allocate and copy the soldier
				SOLDIERTYPE* const pSoldier = new SOLDIERTYPE{};
				*pSoldier = *s;

				//Assign a bogus ID, then return it
				pSoldier->ubID = 255;
				return pSoldier;
			}
		}
	}
	return NULL;
}
catch (...) { return 0; }


//USED BY STRATEGIC AI and AUTORESOLVE
SOLDIERTYPE* TacticalCreateEnemySoldier(SoldierClass const sc)
{
	if (guiCurrentScreen == AUTORESOLVE_SCREEN && !gfPersistantPBI)
	{
		return ReserveTacticalSoldierForAutoresolve(sc);
	}

	BASIC_SOLDIERCREATE_STRUCT bp;
	bp = BASIC_SOLDIERCREATE_STRUCT{};
	RandomizeRelativeLevel(&bp.bRelativeAttributeLevel, sc);
	RandomizeRelativeLevel(&bp.bRelativeEquipmentLevel, sc);
	bp.bTeam          = ENEMY_TEAM;
	bp.bOrders        = SEEKENEMY;
	bp.bAttitude      = Random(MAXATTITUDES);
	bp.bBodyType      = BODY_RANDOM;
	bp.ubSoldierClass = sc;

	SOLDIERCREATE_STRUCT pp;
	pp = SOLDIERCREATE_STRUCT{};
	CreateDetailedPlacementGivenBasicPlacementInfo(&pp, &bp);

	if (sc == SOLDIER_CLASS_ELITE)
	{
		// SPECIAL! Certain events in the game can cause profiled NPCs to become
		// enemies. The two cases are adding Mike and Iggy. We will only add one NPC
		// in any given combat and the conditions for setting the associated facts
		// are done elsewhere. There is also another place where NPCs can get
		// added, which is in AddPlacementToWorld() used for inserting defensive
		// enemies.
		// NOTE: We don't want to add Mike or Iggy if this is being called from
		// autoresolve!
		OkayToUpgradeEliteToSpecialProfiledEnemy(&pp);
	}

	SOLDIERTYPE* const s = TacticalCreateSoldier(pp);
	if (s)
	{
		// Send soldier to centre of map, roughly
		s->sNoiseGridno  = CENTRAL_GRIDNO + (Random(CENTRAL_RADIUS * 2 + 1) - CENTRAL_RADIUS) + (Random(CENTRAL_RADIUS * 2 + 1) - CENTRAL_RADIUS) * WORLD_COLS;
		s->ubNoiseVolume = MAX_MISC_NOISE_DURATION;
	}
	return s;
}


SOLDIERTYPE* TacticalCreateMilitia( UINT8 ubMilitiaClass )
{
	BASIC_SOLDIERCREATE_STRUCT bp;
	SOLDIERCREATE_STRUCT pp;

	bp = BASIC_SOLDIERCREATE_STRUCT{};
	pp = SOLDIERCREATE_STRUCT{};
	RandomizeRelativeLevel( &( bp.bRelativeAttributeLevel ), ubMilitiaClass );
	RandomizeRelativeLevel( &( bp.bRelativeEquipmentLevel ), ubMilitiaClass );
	bp.bTeam = MILITIA_TEAM;
	bp.ubSoldierClass = ubMilitiaClass;
	bp.bOrders = STATIONARY;
	bp.bAttitude = (INT8) Random( MAXATTITUDES );
	//bp.bAttitude = AGGRESSIVE;
	bp.bBodyType = BODY_RANDOM;
	CreateDetailedPlacementGivenBasicPlacementInfo( &pp, &bp );
	return TacticalCreateSoldier(pp);
}

SOLDIERTYPE* TacticalCreateCreature( INT8 bCreatureBodyType )
{
	BASIC_SOLDIERCREATE_STRUCT bp;
	SOLDIERCREATE_STRUCT pp;

	if( guiCurrentScreen == AUTORESOLVE_SCREEN && !gfPersistantPBI )
	{
		return ReserveTacticalSoldierForAutoresolve( SOLDIER_CLASS_CREATURE );
	}

	bp = BASIC_SOLDIERCREATE_STRUCT{};
	pp = SOLDIERCREATE_STRUCT{};
	RandomizeRelativeLevel( &( bp.bRelativeAttributeLevel ), SOLDIER_CLASS_CREATURE );
	RandomizeRelativeLevel( &( bp.bRelativeEquipmentLevel ), SOLDIER_CLASS_CREATURE );
	bp.bTeam = CREATURE_TEAM;
	bp.ubSoldierClass = SOLDIER_CLASS_CREATURE;
	bp.bOrders = SEEKENEMY;
	bp.bAttitude = AGGRESSIVE;
	bp.bBodyType = bCreatureBodyType;
	CreateDetailedPlacementGivenBasicPlacementInfo( &pp, &bp );
	return TacticalCreateSoldier(pp);
}


void RandomizeRelativeLevel( INT8 *pbRelLevel, UINT8 ubSoldierClass )
{
	UINT8 ubLocationModifier;
	INT8 bRollModifier;
	INT8 bRoll, bAdjustedRoll;


	// We now adjust the relative level by location on the map, so enemies in NE corner will be generally very
	// crappy (lots of bad and poor, with avg about best), while enemies in the SW will have lots of great and
	// good, with avg about as lousy as it gets.  Militia are generally unmodified by distance, and never get
	// bad or great at all.

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
		{
			//Artificially strengthen militia strength for sake of gameplay
			bAdjustedRoll++;
		}
	}
	else
	{
		// max-min this to a range of 0-9
		bAdjustedRoll = __max( 0, bAdjustedRoll );
		bAdjustedRoll = __min( 9, bAdjustedRoll );
		if( IsAutoResolveActive() )
		{
			//Artificially weaken enemy/creature strength for sake of gameplay
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
	}*/
}


//This function shouldn't be called outside of tactical
void QuickCreateProfileMerc( INT8 bTeam, UINT8 ubProfileID )
{
	const GridNo pos = GetMouseMapPos();
	if (pos == NOWHERE) return;

	SOLDIERCREATE_STRUCT MercCreateStruct;
	MercCreateStruct = SOLDIERCREATE_STRUCT{};
	MercCreateStruct.bTeam            = bTeam;
	MercCreateStruct.ubProfile        = ubProfileID;
	MercCreateStruct.sSector          = gWorldSector;
	MercCreateStruct.sInsertionGridNo = pos;

	RandomizeNewSoldierStats(&MercCreateStruct);

	SOLDIERTYPE* const s = TacticalCreateSoldier(MercCreateStruct);
	if (s != NULL)
	{
		AddSoldierToSector(s);

		// So we can see them!
		AllTeamsLookForAll(NO_INTERRUPTS);
	}
}


static BOOLEAN TryToAttach(SOLDIERTYPE* const s, OBJECTTYPE* const o)
{
	if (!(GCM->getItem(o->usItem)->getFlags() & ITEM_ATTACHMENT)) return FALSE;

	// try to find the appropriate item to attach to!
	for (UINT32 i = 0; i < NUM_INV_SLOTS; ++i)
	{
		OBJECTTYPE& tgt_o = s->inv[i];
		if (tgt_o.usItem != NOTHING && ValidAttachment(o->usItem, tgt_o.usItem))
		{
			AttachObject(NULL, &tgt_o, o);
			return TRUE;
		}
	}
	return FALSE;
}


static void CopyProfileItems(SOLDIERTYPE& s, SOLDIERCREATE_STRUCT const& c)
{
	MERCPROFILESTRUCT& p = GetProfile(c.ubProfile);
	if (s.bTeam != OUR_TEAM)
	{
		for (UINT32 i = 0; i != NUM_INV_SLOTS; ++i)
		{
			UINT16      const item = p.inv[i];
			OBJECTTYPE* const slot = &s.inv[i];
			if (item != NOTHING)
			{
				UINT8 const count = p.bInvNumber[i];
				if (GCM->getItem(item)->getItemClass() == IC_KEY)
				{
					// Since keys depend on 2 values, they pretty much have to be
					// hardcoded.  if a case isn't handled here it's better to not give
					// any key than to provide one which doesn't work and would confuse
					// everything.
					if (KEY_1 <= item && item <= KEY_32)
					{
						switch (c.ubProfile)
						{
							case BREWSTER: CreateKeyObject(slot, count, 19); break;
							case SKIPPER:  CreateKeyObject(slot, count, 11); break;
							case DOREEN:   CreateKeyObject(slot, count, 32); break;
							default:       *slot = OBJECTTYPE{};             break;
						}
					}
					else
					{
						*slot = OBJECTTYPE{};
					}
				}
				else
				{
					CreateItems(item, p.bInvStatus[i], count, slot);
				}
				if (item == ROCKET_RIFLE || item == AUTO_ROCKET_RIFLE)
				{
					slot->ubImprintID = s.ubProfile;
				}
				if (p.ubInvUndroppable & gubItemDroppableFlag[i])
				{
					slot->fFlags |= OBJECT_UNDROPPABLE;
				}
			}
			else
			{
				*slot = OBJECTTYPE{};
			}
		}

		for (UINT32 money_left = p.uiMoney; money_left > 0;)
		{
			INT8 const slot_id = FindEmptySlotWithin(&s, BIGPOCK1POS, SMALLPOCK8POS);
			if (slot_id == NO_SLOT) break;
			OBJECTTYPE* const slot = &s.inv[slot_id];

			UINT32 const slot_limit  = MoneySlotLimit(slot_id);
			UINT32 const slot_amount = MIN(money_left, slot_limit);
			CreateMoney(slot_amount, slot);
			money_left -= slot_amount;
		}
	}
	else if (c.fCopyProfileItemsOver) // Copy over inv if we want to
	{
		// do some special coding to put stuff in the profile in better-looking
		// spots
		std::fill(std::begin(s.inv), std::end(s.inv), OBJECTTYPE{});
		for (UINT32 i = 0; i != NUM_INV_SLOTS; ++i)
		{
			if (p.inv[i] == NOTHING) continue;

			OBJECTTYPE o;
			CreateItems(p.inv[i], p.bInvStatus[i], p.bInvNumber[i], &o);
			if (!TryToAttach(&s, &o)) AutoPlaceObject(&s, &o, FALSE);
		}
		p.usOptionalGearCost = 0;
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
	FOR_EACH_SOLDIER(i) TacticalRemoveSoldier(*i);
}


static UINT8 GetLocationModifier(UINT8 ubSoldierClass)
{
	UINT8 ubLocationModifier;
	UINT8 ubPalaceDistance;

	// where is all this taking place?
	SGPSector sSector;
	BOOLEAN fSuccess = GetCurrentBattleSectorXYZ(sSector);
	Assert( fSuccess );

	// ignore sSector.z - treat any underground enemies as if they were on the surface!
	switch (GetTownIdForSector(sSector))
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
			ubPalaceDistance = GetPythDistanceFromPalace(sSector);
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
UINT8 GetPythDistanceFromPalace(const SGPSector& sSector)
{
	UINT8 ubDistance = 0;
	INT16 sRows = 0, sCols = 0;
	float fValue = 0.0;

	// grab number of rows and cols
	sRows = (INT16) (ABS(sSector.x - PALACE_SECTOR_X));
	sCols = (INT16) (ABS(sSector.y - PALACE_SECTOR_Y));

	// apply Pythagoras's theorem for right-handed triangle:
	// dist^2 = rows^2 + cols^2, so use the square root to get the distance
	fValue = ( float )sqrt(( float )(sRows * sRows) + ( float )(sCols * sCols));

	if(  fmod( fValue, 1.0f ) >= 0.50 )
	{
		ubDistance = (UINT8)( 1 + fValue );
	}
	else
	{
		ubDistance = ( UINT8 )fValue;
	}

	return( ubDistance );
}


// Reduce the experience levels of very high level enemies to something that
// player can compete with for interrupts. It doesn't affect attributes and
// skills, those are rolled prior to this reduction!
static void ReduceHighExpLevels(INT8* const exp_level)
{
	INT8 max_level;
	switch (gGameOptions.ubDifficultyLevel)
	{
		case DIF_LEVEL_EASY:   max_level = 6; break;
		case DIF_LEVEL_MEDIUM: max_level = 7; break;
		case DIF_LEVEL_HARD:   max_level = 8; break;
		default: return;
	}
	if (*exp_level < max_level) return;
	UINT32 const chance_max_lvl = (*exp_level - max_level + 1) * 25;
	*exp_level = Chance(chance_max_lvl) ? max_level : max_level - 1;
}
