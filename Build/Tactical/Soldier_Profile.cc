#include "Interface_Control.h"
#include "Interface_Panels.h"
#include "LoadSaveMercProfile.h"
#include "MapScreen.h"
#include "Merc_Hiring.h"
#include "Debug.h"
#include "math.h"
#include "WorldDef.h"
#include "Soldier_Control.h"
#include "Animation_Data.h"
#include "Render_Fun.h"
#include "Render_Dirty.h"
#include "MouseSystem.h"
#include "Interface.h"
#include "SysUtil.h"
#include "FileMan.h"
#include "Points.h"
#include "Random.h"
#include "AI.h"
#include "Soldier_Ani.h"
#include "Overhead.h"
#include "Soldier_Profile.h"
#include "Game_Clock.h"
#include "Assignments.h"
#include "Dialogue_Control.h"
#include "Soldier_Create.h"
#include "Soldier_Add.h"
#include "OppList.h"
#include "Weapons.h"
#include "Strategic_Town_Loyalty.h"
#include "Squads.h"
#include "Tactical_Save.h"
#include "Quests.h"
#include "AIM.h"
#include "Interface_Dialogue.h"
#include "GameSettings.h"
#include "Interface_Utils.h"
#include "StrategicMap.h"
#include "Game_Event_Hook.h"
#include "Map_Information.h"
#include "History.h"
#include "Personnel.h"
#include "Environment.h"
#include "Items.h"

#ifdef JA2EDITOR
	extern BOOLEAN gfProfileDataLoaded;
#endif


BOOLEAN	gfPotentialTeamChangeDuringDeath = FALSE;


MERCPROFILESTRUCT gMercProfiles[ NUM_PROFILES ];

INT8 gbSkillTraitBonus[NUM_SKILLTRAITS] =
{
	 0,  //NO_SKILLTRAIT
	25,  //LOCKPICKING
	15,  //HANDTOHAND
	15,  //ELECTRONICS
	15,  //NIGHTOPS
	12,  //THROWING
	15,  //TEACHING
	15,  //HEAVY_WEAPS
	 0,  //AUTO_WEAPS
	15,  //STEALTHY
	 0,	 //AMBIDEXT
	 0,  //THIEF				// UNUSED!
	30,  //MARTIALARTS
	30,  //KNIFING
	15,	 //ONROOF
	 0,	 //CAMOUFLAGED
};


#define NUM_TERRORISTS 6

UINT8	gubTerrorists[NUM_TERRORISTS + 1] =
{
	DRUGGIST,
	SLAY,
	ANNIE,
	CHRIS,
	TIFFANY,
	T_REX,
	0
};

UINT8	gubNumTerrorists = 0;

#define NUM_TERRORIST_POSSIBLE_LOCATIONS 5

INT16	gsTerroristSector[NUM_TERRORISTS][NUM_TERRORIST_POSSIBLE_LOCATIONS][2] =
{
	// Elgin... preplaced
	{
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 }
	},
	// Slay
	{
		{ 9,	MAP_ROW_F },
		{ 14,	MAP_ROW_I },
		{ 1,	MAP_ROW_G },
		{ 2,	MAP_ROW_G },
		{ 8,	MAP_ROW_G }
	},
	// Matron
	{
		{ 14,	MAP_ROW_I },
		{ 6,	MAP_ROW_C },
		{ 2,	MAP_ROW_B },
		{ 11, MAP_ROW_L },
		{ 8,	MAP_ROW_G }
	},
	// Imposter
	{
		{ 1,	MAP_ROW_G },
		{ 9,	MAP_ROW_F },
		{	11,	MAP_ROW_L },
		{	8,	MAP_ROW_G },
		{ 2,	MAP_ROW_G }
	},
	// Tiffany
	{
		{ 14,	MAP_ROW_I },
		{ 2,	MAP_ROW_G },
		{ 14,	MAP_ROW_H },
		{	6,	MAP_ROW_C },
		{	2,	MAP_ROW_B }
	},
	// Rexall
	{
		{	9,	MAP_ROW_F },
		{ 14,	MAP_ROW_H },
		{ 2,	MAP_ROW_H },
		{ 1,	MAP_ROW_G },
		{ 2,	MAP_ROW_B }
	}
};

INT16 gsRobotGridNo;

#define NUM_ASSASSINS 6

UINT8 gubAssassins[NUM_ASSASSINS] =
{
	JIM,
	JACK,
	OLAF,
	RAY,
	OLGA,
	TYRONE
};

#define NUM_ASSASSIN_POSSIBLE_TOWNS 5

INT8 gbAssassinTown[NUM_ASSASSINS][NUM_ASSASSIN_POSSIBLE_TOWNS] =
{
	// Jim
	{ CAMBRIA, DRASSEN, ALMA, BALIME, GRUMM },
	// Jack
	{ CHITZENA,	ESTONI, ALMA, BALIME, GRUMM },
	// Olaf
	{ DRASSEN, ESTONI, ALMA, CAMBRIA, BALIME },
	// Ray
	{ CAMBRIA, OMERTA, BALIME, GRUMM, DRASSEN },
	// Olga
	{ CHITZENA, OMERTA, CAMBRIA, ALMA, GRUMM },
	// Tyrone
	{ CAMBRIA, BALIME, ALMA, GRUMM, DRASSEN },
};


static INT16 CalcMedicalDeposit(MERCPROFILESTRUCT const&);
static void DecideActiveTerrorists(void);
static void StartSomeMercsOnAssignment(void);


BOOLEAN LoadMercProfiles(void)
{
	const char* const pFileName = "BINARYDATA/Prof.dat";

	try
	{
		AutoSGPFile fptr(FileOpen(pFileName, FILE_ACCESS_READ));
		for (UINT32 uiLoop = 0; uiLoop < NUM_PROFILES; ++uiLoop)
		{
#ifdef JA2DEMO
			BYTE data[696];
			FileRead(fptr, data, sizeof(data));
#else
			BYTE data[716];
			JA2EncryptedFileRead(fptr, data, sizeof(data));
#endif

			MERCPROFILESTRUCT& p = GetProfile(uiLoop);
			ExtractMercProfileUTF16(data, p);

			// If the dialogue exists for the merc, allow the merc to be hired
			p.bMercStatus = (FileExists(GetDialogueDataFilename(uiLoop, 0, FALSE)) ? 0 : MERC_HAS_NO_TEXT_FILE);

			p.sMedicalDepositAmount = (p.bMedicalDeposit ? CalcMedicalDeposit(p) : 0);

			// ATE: New, face display independent of ID num now
			// Setup face index value
			// Default is the ubCharNum
			p.ubFaceIndex = (UINT8)uiLoop;

#ifndef JA2DEMO
			if (!gGameOptions.fGunNut)
			{
				// CJC: replace guns in profile if they aren't available
				for (UINT32 uiLoop2 = 0; uiLoop2 < NUM_INV_SLOTS; ++uiLoop2)
				{
					const UINT16 usItem = p.inv[uiLoop2];
					if (!(Item[usItem].usItemClass & IC_GUN) || !ExtendedGunListGun(usItem)) continue;

					const UINT16 usNewGun = StandardGunListReplacement(usItem);
					if (usNewGun == NOTHING) continue;

					p.inv[uiLoop2] = usNewGun;

					// must search through inventory and replace ammo accordingly
					for (UINT32 uiLoop3 = 0; uiLoop3 < NUM_INV_SLOTS; ++uiLoop3)
					{
						const UINT16 usAmmo = p.inv[uiLoop3];
						if (!(Item[usAmmo].usItemClass & IC_AMMO)) continue;

						const UINT16 usNewAmmo = FindReplacementMagazineIfNecessary(usItem, usAmmo, usNewGun);
						if (usNewAmmo == NOTHING) continue;

						// found a new magazine, replace...
						p.inv[uiLoop3] = usNewAmmo;
					}
				}
			}
#endif

			/* Calculate inital attractiveness for the merc's initial gun and armour.
			 * Calculate the optional gear cost. */
			p.bMainGunAttractiveness = -1;
			p.bArmourAttractiveness  = -1;
			p.usOptionalGearCost     =  0;
			for (UINT32 uiLoop2 = 0; uiLoop2 < NUM_INV_SLOTS; ++uiLoop2)
			{
				const UINT16 usItem = p.inv[uiLoop2];
				if (usItem == NOTHING) continue;
				const INVTYPE* const item = &Item[usItem];

				if (item->usItemClass & IC_GUN)    p.bMainGunAttractiveness = Weapon[usItem].ubDeadliness;
				if (item->usItemClass & IC_ARMOUR) p.bArmourAttractiveness  = Armour[item->ubClassIndex].ubProtection;

				p.usOptionalGearCost += item->usPrice;
			}

			//These variables to get loaded in
			p.fUseProfileInsertionInfo = FALSE;
			p.sGridNo                  = 0;

			// ARM: this is also being done inside the profile editor, but put it here too, so this project's code makes sense
			p.bHatedCount[0]    = p.bHatedTime[0];
			p.bHatedCount[1]    = p.bHatedTime[1];
			p.bLearnToHateCount = p.bLearnToHateTime;
			p.bLearnToLikeCount = p.bLearnToLikeTime;
		}
	}
	catch (...)
	{
		DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("FAILED to load merc profiles from file %s", pFileName));
		return FALSE;
	}

#ifndef JA2DEMO
	DecideActiveTerrorists();
#endif

	// initialize mercs' status
	StartSomeMercsOnAssignment();

#ifdef JA2EDITOR
	gfProfileDataLoaded = TRUE;
#endif

	// no better place..heh?.. will load faces for profiles that are 'extern'.....won't have soldiertype instances
	InitalizeStaticExternalNPCFaces();

	LoadCarPortraitValues();

	return TRUE;
}


#define MAX_ADDITIONAL_TERRORISTS 4


static void DecideActiveTerrorists(void)
{
	UINT8		ubLoop, ubLoop2;
	UINT8		ubTerrorist;
	UINT8		ubNumAdditionalTerrorists, ubNumTerroristsAdded = 0;
	UINT32	uiChance, uiLocationChoice;
	BOOLEAN	fFoundSpot;
	INT16		sTerroristPlacement[MAX_ADDITIONAL_TERRORISTS][2] = { {0, 0}, {0, 0}, {0, 0}, {0, 0} };

	// one terrorist will always be Elgin
	// determine how many more terrorists - 2 to 4 more

	// using this stochastic process(!), the chances for terrorists are:
	// EASY:		3, 9%			4, 42%		5, 49%
	// MEDIUM:	3, 25%		4, 50%		5, 25%
	// HARD:		3, 49%		4, 42%		5, 9%
	switch( gGameOptions.ubDifficultyLevel )
	{
		case DIF_LEVEL_EASY:
			uiChance = 70;
			break;
		case DIF_LEVEL_HARD:
			uiChance = 30;
			break;
		default:
			uiChance = 50;
			break;
	}
	// add at least 2 more
	ubNumAdditionalTerrorists = 2;
	for (ubLoop = 0; ubLoop < (MAX_ADDITIONAL_TERRORISTS - 2); ubLoop++)
	{
		if (Random( 100 ) < uiChance)
		{
			ubNumAdditionalTerrorists++;
		}
	}

	// ifdefs added by CJC
	#ifdef JA2TESTVERSION
		ubNumAdditionalTerrorists = 4;
	#endif

	while ( ubNumTerroristsAdded < ubNumAdditionalTerrorists )
	{

		ubLoop = 1; // start at beginning of array (well, after Elgin)

		// NB terrorist ID of 0 indicates end of array
		while ( ubNumTerroristsAdded < ubNumAdditionalTerrorists && gubTerrorists[ ubLoop ] != 0 )
		{

			ubTerrorist = gubTerrorists[ ubLoop ];

			// random 40% chance of adding this terrorist if not yet placed
			if ( ( gMercProfiles[ ubTerrorist ].sSectorX == 0 ) && ( Random( 100 ) < 40 ) )
			{
				fFoundSpot = FALSE;
				// Since there are 5 spots per terrorist and a maximum of 5 terrorists, we
				// are guaranteed to be able to find a spot for each terrorist since there
				// aren't enough other terrorists to use up all the spots for any one
				// terrorist
				do
				{
					// pick a random spot, see if it's already been used by another terrorist
					uiLocationChoice = Random( NUM_TERRORIST_POSSIBLE_LOCATIONS );
					for (ubLoop2 = 0; ubLoop2 < ubNumTerroristsAdded; ubLoop2++)
					{
						if (sTerroristPlacement[ubLoop2][0] == gsTerroristSector[ubLoop][uiLocationChoice][0] )
						{
							if (sTerroristPlacement[ubLoop2][1] == gsTerroristSector[ubLoop][uiLocationChoice][1] )
							{
								continue;
							}
						}
					}
					fFoundSpot = TRUE;
				} while( !fFoundSpot );

				// place terrorist!
				gMercProfiles[ ubTerrorist ].sSectorX = gsTerroristSector[ ubLoop ][ uiLocationChoice ][ 0 ];
				gMercProfiles[ ubTerrorist ].sSectorY = gsTerroristSector[ ubLoop ][ uiLocationChoice ][ 1 ];
				gMercProfiles[ ubTerrorist ].bSectorZ = 0;
				sTerroristPlacement[ ubNumTerroristsAdded ][ 0 ] = gMercProfiles[ ubTerrorist ].sSectorX;
				sTerroristPlacement[ ubNumTerroristsAdded ][ 1 ] = gMercProfiles[ ubTerrorist ].sSectorY;
				ubNumTerroristsAdded++;
			}
			ubLoop++;

		}

		// start over if necessary
	}

	// set total terrorists outstanding in Carmen's info byte
	GetProfile(CARMEN).bNPCData = 1 + ubNumAdditionalTerrorists;

	// store total terrorists
	gubNumTerrorists = 1 + ubNumAdditionalTerrorists;
}


void MakeRemainingTerroristsTougher()
{
	UINT8 n_remaining_terrorists = 0;
	for (UINT8 i = 0; i != NUM_TERRORISTS; ++i)
	{
		ProfileID         const  pid = gubTerrorists[i];
		MERCPROFILESTRUCT const& p   = GetProfile(pid);
		if (p.bMercStatus == MERC_IS_DEAD || p.sSectorX == 0 || p.sSectorY == 0) continue;
		// Slay on player's team, doesn't count towards remaining terrorists
		if (pid == SLAY && FindSoldierByProfileIDOnPlayerTeam(SLAY)) continue;
		++n_remaining_terrorists;
	}

	UINT8 remaining_difficulty = 60 / gubNumTerrorists * (gubNumTerrorists - n_remaining_terrorists);

	switch (gGameOptions.ubDifficultyLevel)
	{
		case DIF_LEVEL_MEDIUM: remaining_difficulty = remaining_difficulty * 13 / 10; break;
		case DIF_LEVEL_HARD:   remaining_difficulty = remaining_difficulty * 16 / 10; break;
		default: break;
	}

	UINT16 old_item;
	UINT16 new_item;
	if (remaining_difficulty < 14)
	{ // nothing
		return;
	}
	else if (remaining_difficulty < 28)
	{ // mini grenade
		old_item = NOTHING;
		new_item = MINI_GRENADE;
	}
	else if (remaining_difficulty < 42)
	{ // hand grenade
		old_item = MINI_GRENADE;
		new_item = HAND_GRENADE;
	}
	else if (remaining_difficulty < 56)
	{ // mustard
		old_item = HAND_GRENADE;
		new_item = MUSTARD_GRENADE;
	}
	else if (remaining_difficulty < 70)
	{ // LAW
		old_item = MUSTARD_GRENADE;
		new_item = ROCKET_LAUNCHER;
	}
	else
	{ // LAW and hand grenade
		old_item = NOTHING;
		new_item = HAND_GRENADE;
	}

	OBJECTTYPE Object;
	DeleteObj(&Object);
	Object.usItem     = new_item;
	Object.bStatus[0] = 100;

	for (UINT8 i = 0; i != NUM_TERRORISTS; ++i)
	{
		ProfileID         const  pid = gubTerrorists[i];
		MERCPROFILESTRUCT const& p   = GetProfile(pid);
		if (p.bMercStatus == MERC_IS_DEAD || p.sSectorX == 0 || p.sSectorY == 0) continue;
		// Slay on player's team, doesn't count towards remaining terrorists
		if (pid == SLAY && FindSoldierByProfileIDOnPlayerTeam(SLAY)) continue;

		if (old_item != NOTHING)
		{
			RemoveObjectFromSoldierProfile(pid, old_item);
		}
		PlaceObjectInSoldierProfile(pid, &Object);
	}
}


void DecideOnAssassin( void )
{
	UINT8		ubAssassinPossibility[NUM_ASSASSINS] = { NO_PROFILE, NO_PROFILE, NO_PROFILE, NO_PROFILE, NO_PROFILE, NO_PROFILE };
	UINT8		ubAssassinsPossible = 0;
	UINT8		ubLoop, ubLoop2;
	UINT8		ubTown;

	ubTown = GetTownIdForSector( gWorldSectorX, gWorldSectorY );

	for ( ubLoop = 0; ubLoop < NUM_ASSASSINS; ubLoop++ )
	{
		// make sure alive and not placed already
		if ( gMercProfiles[ gubAssassins[ ubLoop ] ].bMercStatus != MERC_IS_DEAD && gMercProfiles[ gubAssassins[ ubLoop ] ].sSectorX == 0 && gMercProfiles[ gubAssassins[ ubLoop ] ].sSectorY == 0 )
		{
			// check this merc to see if the town is a possibility
			for ( ubLoop2 = 0; ubLoop2 < NUM_ASSASSIN_POSSIBLE_TOWNS; ubLoop2++ )
			{
				if ( gbAssassinTown[ ubLoop ][ ubLoop2 ] == ubTown )
				{
					ubAssassinPossibility[ ubAssassinsPossible ] = gubAssassins[ ubLoop ];
					ubAssassinsPossible++;
				}
			}
		}
	}

	if ( ubAssassinsPossible != 0 )
	{
		ubLoop = ubAssassinPossibility[ Random( ubAssassinsPossible ) ];
		gMercProfiles[ ubLoop ].sSectorX = gWorldSectorX;
		gMercProfiles[ ubLoop ].sSectorY = gWorldSectorY;
		AddStrategicEvent( EVENT_REMOVE_ASSASSIN, GetWorldTotalMin() + 60 * ( 3 + Random( 3 ) ), ubLoop );
	}

}

void MakeRemainingAssassinsTougher( void )
{
	UINT8					ubRemainingAssassins = 0, ubLoop;
	UINT16				usNewItem, usOldItem;
	OBJECTTYPE		Object;
	UINT8					ubRemainingDifficulty;

	for ( ubLoop = 0; ubLoop < NUM_ASSASSINS; ubLoop++ )
	{
		if ( gMercProfiles[ gubAssassins[ ubLoop ] ].bMercStatus != MERC_IS_DEAD  )
		{
			ubRemainingAssassins++;
		}
	}

	ubRemainingDifficulty = (60 / NUM_ASSASSINS) * (NUM_ASSASSINS - ubRemainingAssassins);

	switch( gGameOptions.ubDifficultyLevel )
	{
		case DIF_LEVEL_MEDIUM:
			ubRemainingDifficulty = (ubRemainingDifficulty * 13) / 10;
			break;
		case DIF_LEVEL_HARD:
			ubRemainingDifficulty = (ubRemainingDifficulty * 16) / 10;
			break;
		default:
			break;
	}

	if ( ubRemainingDifficulty < 14 )
	{
		// nothing
		return;
	}
	else if ( ubRemainingDifficulty < 28 )
	{
		// mini grenade
		usOldItem = NOTHING;
		usNewItem = MINI_GRENADE;
	}
	else if ( ubRemainingDifficulty < 42)
	{
		// hand grenade
		usOldItem = MINI_GRENADE;
		usNewItem = HAND_GRENADE;
	}
	else if ( ubRemainingDifficulty < 56)
	{
		// mustard
		usOldItem = HAND_GRENADE;
		usNewItem = MUSTARD_GRENADE;
	}
	else if ( ubRemainingDifficulty < 70)
	{
		// LAW
		usOldItem = MUSTARD_GRENADE;
		usNewItem = ROCKET_LAUNCHER;
	}
	else
	{
		// LAW and hand grenade
		usOldItem = NOTHING;
		usNewItem = HAND_GRENADE;
	}

	DeleteObj( &Object );
	Object.usItem = usNewItem;
	Object.bStatus[ 0 ] = 100;

	for ( ubLoop = 0; ubLoop < NUM_ASSASSINS; ubLoop++ )
	{
		if ( gMercProfiles[ gubAssassins[ ubLoop ] ].bMercStatus != MERC_IS_DEAD )
		{
			if ( usOldItem != NOTHING )
			{
				RemoveObjectFromSoldierProfile( gubAssassins[ ubLoop ], usOldItem );
			}
			PlaceObjectInSoldierProfile( gubAssassins[ ubLoop ], &Object );
		}
	}
}


static void StartSomeMercsOnAssignment(void)
{
	UINT32 uiCnt;
	UINT32 uiChance;

	// some randomly picked A.I.M. mercs will start off "on assignment" at the beginning of each new game
	for( uiCnt = 0; uiCnt < AIM_AND_MERC_MERCS; uiCnt++)
	{
		MERCPROFILESTRUCT& p = GetProfile(uiCnt);

		// calc chance to start on assignment
		uiChance = 5 * p.bExpLevel;

		if (Random(100) < uiChance)
		{
			p.bMercStatus = MERC_WORKING_ELSEWHERE;
			p.uiDayBecomesAvailable = 1 + Random(6 + p.bExpLevel / 2); // 1-(6 to 11) days
		}
		else
		{
			p.bMercStatus           = MERC_OK;
			p.uiDayBecomesAvailable = 0;
		}

		p.uiPrecedentQuoteSaid   = 0;
		p.ubDaysOfMoraleHangover = 0;
	}
}


void SetProfileFaceData( UINT8 ubCharNum, UINT8 ubFaceIndex, UINT16 usEyesX, UINT16 usEyesY, UINT16 usMouthX, UINT16 usMouthY )
{
	 gMercProfiles[ ubCharNum ].ubFaceIndex = ubFaceIndex;
	 gMercProfiles[ ubCharNum ].usEyesX			= usEyesX;
	 gMercProfiles[ ubCharNum ].usEyesY			= usEyesY;
	 gMercProfiles[ ubCharNum ].usMouthX		= usMouthX;
	 gMercProfiles[ ubCharNum ].usMouthY		= usMouthY;
}


static UINT16 CalcCompetence(MERCPROFILESTRUCT const& p)
{
	UINT32 uiStats, uiSkills, uiActionPoints, uiSpecialSkills;
	UINT16 usCompetence;


	// count life twice 'cause it's also hit points
	// mental skills are halved 'cause they're actually not that important within the game
	uiStats = ((2 * p.bLifeMax) + p.bStrength + p.bAgility + p.bDexterity + ((p.bLeadership + p.bWisdom) / 2)) / 3;

	// marksmanship is very important, count it double
	uiSkills = (UINT32) ((2   * (pow(p.bMarksmanship, 3) / 10000)) +
												1.5 *	(pow(p.bMedical, 3) / 10000) +
															(pow(p.bMechanical, 3) / 10000) +
															(pow(p.bExplosive, 3) / 10000));

	// action points
	uiActionPoints = 5 + (((10 * p.bExpLevel +
													 3 * p.bAgility  +
													 2 * p.bLifeMax  +
													 2 * p.bDexterity) + 20) / 40);


	// count how many he has, don't care what they are
	uiSpecialSkills = ((p.bSkillTrait != 0) ? 1 : 0) + ((p.bSkillTrait2 != 0) ? 1 : 0);

	usCompetence = (UINT16) ((pow(p.bExpLevel, 0.2) * uiStats * uiSkills * (uiActionPoints - 6) * (1 + (0.05 * (FLOAT)uiSpecialSkills))) / 1000);

	// this currently varies from about 10 (Flo) to 1200 (Gus)
	return(usCompetence);
}


static INT16 CalcMedicalDeposit(MERCPROFILESTRUCT const& p)
{
	UINT16 usDeposit;

	// this rounds off to the nearest hundred
	usDeposit = (5 * CalcCompetence(p) + 50) / 100 * 100;

	return(usDeposit);
}


SOLDIERTYPE* FindSoldierByProfileID(const ProfileID pid)
{
	FOR_ALL_SOLDIERS(s)
	{
		if (s->ubProfile == pid) return s;
	}
	return NULL;
}


SOLDIERTYPE* FindSoldierByProfileIDOnPlayerTeam(const ProfileID pid)
{
	FOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		if (s->ubProfile == pid) return s;
	}
	return NULL;
}


SOLDIERTYPE *ChangeSoldierTeam( SOLDIERTYPE *pSoldier, UINT8 ubTeam )
{
	SOLDIERCREATE_STRUCT		MercCreateStruct;
	UINT32									cnt;
	INT16										sOldGridNo;

	if (gfInTalkPanel)
	{
		DeleteTalkingMenu();
	}

	sOldGridNo = pSoldier->sGridNo;

	// Remove him from the game!
	InternalTacticalRemoveSoldier(pSoldier, FALSE);

	// Create a new one!
	memset( &MercCreateStruct, 0, sizeof( MercCreateStruct ) );
	MercCreateStruct.bTeam							= ubTeam;
	MercCreateStruct.ubProfile					= pSoldier->ubProfile;
	MercCreateStruct.bBodyType					= pSoldier->ubBodyType;
	MercCreateStruct.sSectorX						= pSoldier->sSectorX;
	MercCreateStruct.sSectorY						= pSoldier->sSectorY;
	MercCreateStruct.bSectorZ						= pSoldier->bSectorZ;
	MercCreateStruct.sInsertionGridNo		= pSoldier->sGridNo;
	MercCreateStruct.bDirection					= pSoldier->bDirection;

	if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		MercCreateStruct.ubProfile					= NO_PROFILE;
		MercCreateStruct.fUseGivenVehicle		= TRUE;
		MercCreateStruct.bUseGivenVehicleID	= pSoldier->bVehicleID;
	}

	SOLDIERTYPE* const pNewSoldier = TacticalCreateSoldier(MercCreateStruct);
	if (pNewSoldier != NULL)
	{
		// Copy vital stats back!
		pNewSoldier->bLife													= pSoldier->bLife;
		pNewSoldier->bLifeMax												= pSoldier->bLifeMax;
		pNewSoldier->bAgility												= pSoldier->bAgility;
		pNewSoldier->bLeadership										= pSoldier->bLeadership;
		pNewSoldier->bDexterity											= pSoldier->bDexterity;
		pNewSoldier->bStrength											= pSoldier->bStrength;
		pNewSoldier->bWisdom												= pSoldier->bWisdom;
		pNewSoldier->bExpLevel											= pSoldier->bExpLevel;
		pNewSoldier->bMarksmanship									= pSoldier->bMarksmanship;
		pNewSoldier->bMedical												= pSoldier->bMedical;
		pNewSoldier->bMechanical										= pSoldier->bMechanical;
		pNewSoldier->bExplosive											= pSoldier->bExplosive;
		pNewSoldier->bLastRenderVisibleValue				= pSoldier->bLastRenderVisibleValue;
		pNewSoldier->bVisible												= pSoldier->bVisible;

		if ( ubTeam == gbPlayerNum )
		{
			pNewSoldier->bVisible											= 1;
		}

		// Copy over any items....
		for ( cnt = 0; cnt < NUM_INV_SLOTS; cnt++ )
		{
			pNewSoldier->inv[ cnt ] = pSoldier->inv[ cnt ];
		}

		// OK, loop through all active merc slots, change
		// Change ANY attacker's target if they were once on this guy.....
		FOR_ALL_MERCS(i)
		{
			SOLDIERTYPE* const s = *i;
			if (s->target == pSoldier) s->target = pNewSoldier;
		}

		// Set insertion gridNo
		pNewSoldier->sInsertionGridNo								= sOldGridNo;

		if ( gfPotentialTeamChangeDuringDeath )
		{
			HandleCheckForDeathCommonCode( pSoldier );
		}

		if ( gfWorldLoaded &&  pSoldier->bInSector
		//pSoldier->sSectorX == gWorldSectorX && pSoldier->sSectorY == gWorldSectorY && pSoldier->bSectorZ == gbWorldSectorZ
		 )
		{
			AddSoldierToSectorNoCalculateDirectionUseAnimation(pNewSoldier, pSoldier->usAnimState, pSoldier->usAniCode);
			HandleSight(pNewSoldier, SIGHT_LOOK | SIGHT_RADIO);
		}

		if ( pNewSoldier->ubProfile != NO_PROFILE )
		{
			if ( ubTeam == gbPlayerNum )
			{
				gMercProfiles[ pNewSoldier->ubProfile ].ubMiscFlags |= PROFILE_MISC_FLAG_RECRUITED;
			}
			else
			{
				gMercProfiles[ pNewSoldier->ubProfile ].ubMiscFlags &= (~PROFILE_MISC_FLAG_RECRUITED);
			}
		}

	}

	// AT the low level check if this poor guy is in inv panel, else
	// remove....
	if ( gsCurInterfacePanel == SM_PANEL && gpSMCurrentMerc == pSoldier )
	{
		// Switch....
		SetCurrentInterfacePanel( TEAM_PANEL );
	}

	return( pNewSoldier );
}


BOOLEAN RecruitRPC( UINT8 ubCharNum )
{
	SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubCharNum);
	if (!pSoldier)
	{
		return( FALSE );
	}

	// OK, set recruit flag..
	gMercProfiles[ ubCharNum ].ubMiscFlags |= PROFILE_MISC_FLAG_RECRUITED;

	// Add this guy to our team!
	SOLDIERTYPE* const pNewSoldier = ChangeSoldierTeam(pSoldier, gbPlayerNum);

	// handle set up any RPC's that will leave us in time
	if ( ubCharNum == SLAY )
	{
		// slay will leave in a week
		pNewSoldier->iEndofContractTime = GetWorldTotalMin() + ( 7 * 24 * 60 );

		KickOutWheelchair( pNewSoldier );
	}
	else if ( ubCharNum == DYNAMO && gubQuest[ QUEST_FREE_DYNAMO ] == QUESTINPROGRESS )
	{
		EndQuest( QUEST_FREE_DYNAMO, pSoldier->sSectorX, pSoldier->sSectorY );
	}
	// handle town loyalty adjustment
	HandleTownLoyaltyForNPCRecruitment( pNewSoldier );

	// Try putting them into the current squad
	if (!AddCharacterToSquad(pNewSoldier, CurrentSquad()))
	{
		AddCharacterToAnySquad( pNewSoldier );
	}

  ResetDeadSquadMemberList( pNewSoldier->bAssignment );

	DirtyMercPanelInterface( pNewSoldier, DIRTYLEVEL2 );

	if ( pNewSoldier->inv[ HANDPOS ].usItem == NOTHING )
	{
		// empty handed - swap in first available weapon
		INT8		bSlot;

		bSlot = FindObjClass( pNewSoldier, IC_WEAPON );
		if ( bSlot != NO_SLOT )
		{
			if ( Item[ pNewSoldier->inv[ bSlot ].usItem ].fFlags & ITEM_TWO_HANDED )
			{
				if ( bSlot != SECONDHANDPOS && pNewSoldier->inv[ SECONDHANDPOS ].usItem != NOTHING )
				{
					// need to move second hand item out first
					AutoPlaceObject( pNewSoldier, &(pNewSoldier->inv[ SECONDHANDPOS ]), FALSE );
				}
			}
			// swap item to hand
			SwapObjs( &(pNewSoldier->inv[ bSlot ]), &(pNewSoldier->inv[ HANDPOS ]) );
		}
	}

#ifdef JA2DEMO
	HandleEndDemoInCreatureLevel( );
#endif

	if ( ubCharNum == IRA )
	{
		// trigger 0th PCscript line
		TriggerNPCRecord( IRA, 0 );
	}

	// Set whatkind of merc am i
	pNewSoldier->ubWhatKindOfMercAmI = MERC_TYPE__NPC;


	//
	//add a history log that tells the user that a npc has joined the team
	//
	// ( pass in pNewSoldier->sSectorX cause if its invalid, -1, n/a will appear as the sector in the history log )
	AddHistoryToPlayersLog( HISTORY_RPC_JOINED_TEAM, pNewSoldier->ubProfile, GetWorldTotalMin(), pNewSoldier->sSectorX, pNewSoldier->sSectorY );


	//remove the merc from the Personnel screens departed list ( if they have never been hired before, its ok to call it )
	RemoveNewlyHiredMercFromPersonnelDepartedList( pSoldier->ubProfile );

	return( TRUE );
}

BOOLEAN RecruitEPC( UINT8 ubCharNum )
{
	SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubCharNum);
	if (!pSoldier)
	{
		return( FALSE );
	}

	// OK, set recruit flag..
	gMercProfiles[ ubCharNum ].ubMiscFlags |= PROFILE_MISC_FLAG_EPCACTIVE;

	gMercProfiles[ ubCharNum ].ubMiscFlags3 &= ~PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE;

	// Add this guy to our team!
	SOLDIERTYPE* const pNewSoldier = ChangeSoldierTeam( pSoldier, gbPlayerNum );
	pNewSoldier->ubWhatKindOfMercAmI = MERC_TYPE__EPC;

	// Try putting them into the current squad
	if (!AddCharacterToSquad(pNewSoldier, CurrentSquad()))
	{
		AddCharacterToAnySquad( pNewSoldier );
	}

  ResetDeadSquadMemberList( pNewSoldier->bAssignment );

	DirtyMercPanelInterface( pNewSoldier, DIRTYLEVEL2 );
	// Make the interface panel dirty..
	// This will dirty the panel next frame...
	gfRerenderInterfaceFromHelpText = TRUE;


	// If we are a robot, look to update controller....
	if ( pNewSoldier->uiStatusFlags & SOLDIER_ROBOT )
	{
		UpdateRobotControllerGivenRobot( pNewSoldier );
	}

	// Set whatkind of merc am i
	pNewSoldier->ubWhatKindOfMercAmI = MERC_TYPE__EPC;

  UpdateTeamPanelAssignments( );

	return( TRUE );
}


BOOLEAN UnRecruitEPC(ProfileID const pid)
{
	SOLDIERTYPE* const s = FindSoldierByProfileID(pid);
	if (!s) return FALSE;
	if (s->ubWhatKindOfMercAmI != MERC_TYPE__EPC) return FALSE;

	if (s->bAssignment < ON_DUTY) ResetDeadSquadMemberList(s->bAssignment);

	MERCPROFILESTRUCT& p = GetProfile(pid);

	// OK, UN set recruit flag..
	p.ubMiscFlags &= ~PROFILE_MISC_FLAG_EPCACTIVE;

	// update sector values to current

	// check to see if this person should disappear from the map after this
	if ((pid == JOHN || pid == MARY) &&
			s->sSectorX == 13            &&
			s->sSectorY == MAP_ROW_B     &&
			s->bSectorZ == 0)
	{
		p.sSectorX = 0;
		p.sSectorY = 0;
		p.bSectorZ = 0;
	}
	else
	{
		p.sSectorX = s->sSectorX;
		p.sSectorY = s->sSectorY;
		p.bSectorZ = s->bSectorZ;
	}

	// how do we decide whether or not to set this?
	p.fUseProfileInsertionInfo  = TRUE;
	p.ubMiscFlags3             |= PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE;

	ChangeSoldierTeam(s, CIV_TEAM);
	UpdateTeamPanelAssignments();
	return TRUE;
}


INT8 WhichBuddy( UINT8 ubCharNum, UINT8 ubBuddy )
{
	INT8								bLoop;

	MERCPROFILESTRUCT const& p = GetProfile(ubCharNum);

	for (bLoop = 0; bLoop < 3; bLoop++)
	{
		if (p.bBuddy[bLoop] == ubBuddy)
		{
			return( bLoop );
		}
	}
	return( -1 );
}

INT8 WhichHated( UINT8 ubCharNum, UINT8 ubHated )
{
	INT8								bLoop;

	MERCPROFILESTRUCT const& p = GetProfile(ubCharNum);

	for (bLoop = 0; bLoop < 3; bLoop++)
	{
		if (p.bHated[bLoop] == ubHated)
		{
			return( bLoop );
		}
	}
	return( -1 );
}


INT8 GetFirstBuddyOnTeam(MERCPROFILESTRUCT const& p)
{
	for (INT i = 0; i != 3; ++i)
	{
		INT8 const buddy = p.bBuddy[i];
		if (buddy < 0)                     continue;
		if (!IsMercOnTeam(buddy))          continue;
		if (IsMercDead(GetProfile(buddy))) continue;
		return buddy;
	}
	return -1;
}


BOOLEAN IsProfileATerrorist(UINT8 ubProfile)
{
	switch (ubProfile)
	{
		case SLAY:
		case ANNIE:
		case CHRIS:
		case TIFFANY:
		case T_REX:
		case DRUGGIST: return TRUE;
		default:       return FALSE;
	}
}


BOOLEAN IsProfileAHeadMiner(UINT8 ubProfile)
{
	switch (ubProfile)
	{
		case FRED:
		case MATT:
		case OSWALD:
		case CALVIN:
		case CARL:   return TRUE;
		default:     return FALSE;
	}
}


void UpdateSoldierPointerDataIntoProfile( BOOLEAN fPlayerMercs )
{
	BOOLEAN				fDoCopy = FALSE;

	FOR_ALL_MERCS(i)
	{
		const SOLDIERTYPE* const pSoldier = *i;
		if ( pSoldier->ubProfile != NO_PROFILE )
		{
			fDoCopy = FALSE;

			// If we are above player mercs
			if ( fPlayerMercs )
			{
				if ( pSoldier->ubProfile < FIRST_RPC )
				{
					fDoCopy = TRUE;
				}
			}
			else
			{
				if ( pSoldier->ubProfile >= FIRST_RPC )
				{
					fDoCopy = TRUE;
				}
			}

			if ( fDoCopy )
			{
				// Copy....
				MERCPROFILESTRUCT& p = GetProfile(pSoldier->ubProfile);
				p.bLife         = pSoldier->bLife;
				p.bLifeMax      = pSoldier->bLifeMax;
				p.bAgility      = pSoldier->bAgility;
				p.bLeadership   = pSoldier->bLeadership;
				p.bDexterity    = pSoldier->bDexterity;
				p.bStrength     = pSoldier->bStrength;
				p.bWisdom       = pSoldier->bWisdom;
				p.bExpLevel     = pSoldier->bExpLevel;
				p.bMarksmanship = pSoldier->bMarksmanship;
				p.bMedical      = pSoldier->bMedical;
				p.bMechanical   = pSoldier->bMechanical;
				p.bExplosive    = pSoldier->bExplosive;
			}
		}
	}
}


static BOOLEAN MercIsHot(SOLDIERTYPE* pSoldier)
{
	if ( pSoldier->ubProfile != NO_PROFILE && gMercProfiles[ pSoldier->ubProfile ].bPersonalityTrait == HEAT_INTOLERANT )
	{
		if ( SectorTemperature( GetWorldMinutesInDay(), pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ ) > 0 )
		{
			return( TRUE );
		}
	}
	return( FALSE );
}


SOLDIERTYPE* SwapLarrysProfiles(SOLDIERTYPE* const s)
{
	const ProfileID src_id = s->ubProfile;
	ProfileID       dst_id;
	switch (src_id)
	{
		case LARRY_NORMAL: dst_id = LARRY_DRUNK;  break;
		case LARRY_DRUNK:  dst_id = LARRY_NORMAL; break;
		default:           return s; // I don't think so!
	}

	MERCPROFILESTRUCT const& src = GetProfile(src_id);
	MERCPROFILESTRUCT&       dst = GetProfile(dst_id);

	dst.ubMiscFlags2                = src.ubMiscFlags2;
	dst.ubMiscFlags                 = src.ubMiscFlags;
	dst.sSectorX                    = src.sSectorX;
	dst.sSectorY                    = src.sSectorY;
	dst.uiDayBecomesAvailable       = src.uiDayBecomesAvailable;
	dst.usKills                     = src.usKills;
	dst.usAssists                   = src.usAssists;
	dst.usShotsFired                = src.usShotsFired;
	dst.usShotsHit                  = src.usShotsHit;
	dst.usBattlesFought             = src.usBattlesFought;
	dst.usTimesWounded              = src.usTimesWounded;
	dst.usTotalDaysServed           = src.usTotalDaysServed;
	dst.fUseProfileInsertionInfo    = src.fUseProfileInsertionInfo;
	dst.sGridNo                     = src.sGridNo;
	dst.ubQuoteActionID             = src.ubQuoteActionID;
	dst.ubLastQuoteSaid             = src.ubLastQuoteSaid;
	dst.ubStrategicInsertionCode    = src.ubStrategicInsertionCode;
	dst.bMercStatus                 = src.bMercStatus;
	dst.bSectorZ                    = src.bSectorZ;
	dst.usStrategicInsertionData    = src.usStrategicInsertionData;
	dst.ubMiscFlags3                = src.ubMiscFlags3;
	dst.ubDaysOfMoraleHangover      = src.ubDaysOfMoraleHangover;
	dst.ubNumTimesDrugUseInLifetime = src.ubNumTimesDrugUseInLifetime;
	dst.uiPrecedentQuoteSaid        = src.uiPrecedentQuoteSaid;
	dst.sPreCombatGridNo            = src.sPreCombatGridNo;

// CJC: this is causing problems so just skip the transfer of exp...
/*
	dst.sLifeGain         = src.sLifeGain;
	dst.sAgilityGain      = src.sAgilityGain;
	dst.sDexterityGain    = src.sDexterityGain;
	dst.sStrengthGain     = src.sStrengthGain;
	dst.sLeadershipGain   = src.sLeadershipGain;
	dst.sWisdomGain       = src.sWisdomGain;
	dst.sExpLevelGain     = src.sExpLevelGain;
	dst.sMarksmanshipGain = src.sMarksmanshipGain;
	dst.sMedicalGain      = src.sMedicalGain;
	dst.sMechanicGain     = src.sMechanicGain;
	dst.sExplosivesGain   = src.sExplosivesGain;

	dst.bLifeDelta         = src.bLifeDelta;
	dst.bAgilityDelta      = src.bAgilityDelta;
	dst.bDexterityDelta    = src.bDexterityDelta;
	dst.bStrengthDelta     = src.bStrengthDelta;
	dst.bLeadershipDelta   = src.bLeadershipDelta;
	dst.bWisdomDelta       = src.bWisdomDelta;
	dst.bExpLevelDelta     = src.bExpLevelDelta;
	dst.bMarksmanshipDelta = src.bMarksmanshipDelta;
	dst.bMedicalDelta      = src.bMedicalDelta;
	dst.bMechanicDelta     = src.bMechanicDelta;
	dst.bExplosivesDelta   = src.bExplosivesDelta;
*/

	memcpy(dst.bInvStatus, src.bInvStatus, sizeof(dst.bInvStatus));
	memcpy(dst.bInvNumber, src.bInvStatus, sizeof(dst.bInvNumber));
	memcpy(dst.inv,        src.inv,        sizeof(dst.inv));

	DeleteSoldierFace(s);
	s->ubProfile = dst_id;
	InitSoldierFace(*s);

	s->bStrength     = dst.bStrength     + dst.bStrengthDelta;
	s->bDexterity    = dst.bDexterity    + dst.bDexterityDelta;
	s->bAgility      = dst.bAgility      + dst.bAgilityDelta;
	s->bWisdom       = dst.bWisdom       + dst.bWisdomDelta;
	s->bExpLevel     = dst.bExpLevel     + dst.bExpLevelDelta;
	s->bLeadership   = dst.bLeadership   + dst.bLeadershipDelta;
	s->bMarksmanship = dst.bMarksmanship + dst.bMarksmanshipDelta;
	s->bMechanical   = dst.bMechanical   + dst.bMechanicDelta;
	s->bMedical      = dst.bMedical      + dst.bMedicalDelta;
	s->bExplosive    = dst.bExplosive    + dst.bExplosivesDelta;

	if (s->ubProfile == LARRY_DRUNK)
	{
		SetFactTrue(FACT_LARRY_CHANGED);
	}
	else
	{
		SetFactFalse(FACT_LARRY_CHANGED);
	}

	DirtyMercPanelInterface(s, DIRTYLEVEL2);

	return s;
}


BOOLEAN DoesNPCOwnBuilding( SOLDIERTYPE *pSoldier, INT16 sGridNo )
{
  UINT8 ubRoomInfo;

  // Get room info
  ubRoomInfo = gubWorldRoomInfo[ sGridNo ];

  if ( ubRoomInfo == NO_ROOM )
  {
    return( FALSE );
  }

  // Are we an NPC?
  if ( pSoldier->bTeam != CIV_TEAM )
  {
    return( FALSE );
  }

  // OK, check both ranges
  if ( ubRoomInfo >= gMercProfiles[ pSoldier->ubProfile ].ubRoomRangeStart[ 0 ] &&
       ubRoomInfo <= gMercProfiles[ pSoldier->ubProfile ].ubRoomRangeEnd[ 0 ] )
  {
     return( TRUE );
  }

  if ( ubRoomInfo >= gMercProfiles[ pSoldier->ubProfile ].ubRoomRangeStart[ 1 ] &&
       ubRoomInfo <= gMercProfiles[ pSoldier->ubProfile ].ubRoomRangeEnd[ 1 ] )
  {
     return( TRUE );
  }

  return( FALSE );
}
