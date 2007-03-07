#ifdef PRECOMPILEDHEADERS
	#include "Tactical All.h"
#else
	#include <stdio.h>
	#include <string.h>
	#include "WCheck.h"
	#include "stdlib.h"
	#include "Debug.h"
	#include "math.h"
	#include "WorldDef.h"
	#include "WorldMan.h"

	#include "Soldier_Control.h"
	#include "Animation_Control.h"
	#include "Animation_Data.h"
	#include "Isometric_Utils.h"
	#include "Event_Pump.h"
	#include "Timer_Control.h"
	#include "Render_Fun.h"
	#include "Render_Dirty.h"
	#include "MouseSystem.h"
	#include "Interface.h"
	#include "SysUtil.h"
	#include "FileMan.h"
	#include "Points.h"
	#include "Random.h"
	#include "AI.h"
	#include "Interactive_Tiles.h"
	#include "Soldier_Ani.h"
	#include "English.h"
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
	#include "Strategic_Town_Reputation.h"
	#include "Interface_Utils.h"
	#include "StrategicMap.h"
	#include "Game_Event_Hook.h"
	#include "Map_Information.h"
	#include "History.h"
	#include "Personnel.h"
	#include "Environment.h"
	#include "Player_Command.h"
	#include "Items.h"
#endif

#ifdef JA2EDITOR
	extern BOOLEAN gfProfileDataLoaded;
#endif


BOOLEAN	gfPotentialTeamChangeDuringDeath = FALSE;


#define		MIN_BLINK_FREQ					3000
#define		MIN_EXPRESSION_FREQ			2000

#define		SET_PROFILE_GAINS2			500, 500, 500, 500, 500, 500, 500, 500, 500

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


static const UINT8 gubBasicInventoryPositions[] = {
							HELMETPOS,
							VESTPOS,
							LEGPOS,
							HANDPOS,
							BIGPOCK1POS,
							BIGPOCK2POS,
							BIGPOCK3POS,
							BIGPOCK4POS
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

UINT16 CalcCompetence( MERCPROFILESTRUCT * pProfile );
INT16 CalcMedicalDeposit( MERCPROFILESTRUCT * pProfile );
extern void HandleEndDemoInCreatureLevel( );
void DecideActiveTerrorists( void );

extern SOLDIERTYPE			*gpSMCurrentMerc;
extern BOOLEAN	gfRerenderInterfaceFromHelpText;


typedef struct
{
	UINT16	zName[NAME_LENGTH]; // XXX 16bit chars
	UINT16	zNickname[NICKNAME_LENGTH]; // XXX 16bit chars
	UINT32	uiAttnSound;
	UINT32	uiCurseSound;
	UINT32	uiDieSound;
	UINT32	uiGoodSound;
	UINT32	uiGruntSound;
	UINT32	uiGrunt2Sound;
	UINT32  uiOkSound;
	UINT8		ubFaceIndex;
	PaletteRepID		PANTS;
	PaletteRepID		VEST;
	PaletteRepID		SKIN;
	PaletteRepID		HAIR;
	INT8		bSex;
	INT8		bArmourAttractiveness;
	UINT8		ubMiscFlags2;
	INT8		bEvolution;
	UINT8		ubMiscFlags;
	UINT8		bSexist;
	INT8		bLearnToHate;

	// skills
	INT8		bStealRate;
	INT8		bVocalVolume;
	UINT8		ubQuoteRecord;
	INT8		bDeathRate;
	INT8		bScientific;

	INT16		sExpLevelGain;
	INT16		sLifeGain;
	INT16		sAgilityGain;
	INT16		sDexterityGain;
	INT16		sWisdomGain;
	INT16		sMarksmanshipGain;
	INT16		sMedicalGain;
	INT16		sMechanicGain;
	INT16		sExplosivesGain;

	UINT8		ubBodyType;
	INT8		bMedical;

	UINT16	usEyesX;
	UINT16	usEyesY;
	UINT16	usMouthX;
	UINT16	usMouthY;
	UINT32	uiEyeDelay;
	UINT32	uiMouthDelay;
	UINT32	uiBlinkFrequency;
	UINT32	uiExpressionFrequency;
	UINT16	sSectorX;
	UINT16	sSectorY;

	UINT32	uiDayBecomesAvailable;			//day the merc will be available.  used with the bMercStatus

	INT8		bStrength;

	INT8		bLifeMax;
	INT8		bExpLevelDelta;
	INT8		bLifeDelta;
	INT8		bAgilityDelta;
	INT8		bDexterityDelta;
	INT8		bWisdomDelta;
	INT8		bMarksmanshipDelta;
	INT8		bMedicalDelta;
	INT8		bMechanicDelta;
	INT8		bExplosivesDelta;
	INT8    bStrengthDelta;
	INT8    bLeadershipDelta;
	UINT16  usKills;
	UINT16  usAssists;
	UINT16  usShotsFired;
	UINT16  usShotsHit;
	UINT16  usBattlesFought;
	UINT16  usTimesWounded;
	UINT16  usTotalDaysServed;

	INT16		sLeadershipGain;
	INT16		sStrengthGain;



	// BODY TYPE SUBSITUTIONS
	UINT32	uiBodyTypeSubFlags;

	INT16	sSalary;
	INT8	bLife;
	INT8		bDexterity;		// dexterity (hand coord) value
	INT8	bPersonalityTrait;
	INT8	bSkillTrait;

	INT8	bReputationTolerance;
	INT8	bExplosive;
	INT8	bSkillTrait2;
	INT8	bLeadership;

	INT8	bBuddy[5];
	INT8	bHated[5];
	INT8	bExpLevel;		// general experience level

	INT8	bMarksmanship;
	UINT8	bMinService;
	INT8	bWisdom;
	UINT8	bResigned;
	UINT8	bActive;

	UINT8	bInvStatus[19];
	UINT8 bInvNumber[19];
	UINT16 usApproachFactor[4];

	INT8	bMainGunAttractiveness;
	INT8		bAgility;			// agility (speed) value

	BOOLEAN	fUseProfileInsertionInfo;				// Set to various flags, ( contained in TacticalSave.h )
	INT16		sGridNo;												// The Gridno the NPC was in before leaving the sector
	UINT8		ubQuoteActionID;
	INT8		bMechanical;

	UINT8	ubInvUndroppable;
	UINT8	ubRoomRangeStart[2];
	UINT16 inv[19];
	INT8 bMercTownReputation[ 20 ];

	UINT16 usStatChangeChances[ 12 ];		// used strictly for balancing, never shown!
	UINT16 usStatChangeSuccesses[ 12 ];	// used strictly for balancing, never shown!

	UINT8	ubStrategicInsertionCode;

	UINT8	ubRoomRangeEnd[2];

	INT8 bPadding[ 4 ];

	UINT8 ubLastQuoteSaid;

	INT8 bRace;
	INT8 bNationality;
	INT8 bAppearance;
	INT8 bAppearanceCareLevel;
	INT8 bRefinement;
	INT8 bRefinementCareLevel;
	INT8 bHatedNationality;
	INT8 bHatedNationalityCareLevel;
	INT8 bRacist;
	UINT32 uiWeeklySalary;
	UINT32 uiBiWeeklySalary;
	INT8 bMedicalDeposit;
	INT8 bAttitude;
	INT8 bBaseMorale;
	UINT16 sMedicalDepositAmount;

	INT8 bLearnToLike;
	UINT8 ubApproachVal[4];
	UINT8 ubApproachMod[3][4];
	INT8 bTown;
	INT8 bTownAttachment;
	UINT16 usOptionalGearCost;
	INT8 bMercOpinion[75];
	INT8 bApproached;
	INT8 bMercStatus;								//The status of the merc.  If negative, see flags at the top of this file.  Positive:  The number of days the merc is away for.  0:  Not hired but ready to be.
	INT8 bHatedTime[5];
	INT8 bLearnToLikeTime;
	INT8 bLearnToHateTime;
	INT8 bHatedCount[5];
	INT8 bLearnToLikeCount;
	INT8 bLearnToHateCount;
	UINT8 ubLastDateSpokenTo;
	UINT8 bLastQuoteSaidWasSpecial;
	INT8	bSectorZ;
	UINT16 usStrategicInsertionData;
	INT8 bFriendlyOrDirectDefaultResponseUsedRecently;
	INT8 bRecruitDefaultResponseUsedRecently;
	INT8 bThreatenDefaultResponseUsedRecently;
	INT8 bNPCData;			// NPC specific
	INT32	iBalance;
	INT16 sTrueSalary; // for use when the person is working for us for free but has a positive salary value
	UINT8	ubCivilianGroup;
	UINT8	ubNeedForSleep;
	UINT32	uiMoney;
	INT8	bNPCData2;		// NPC specific

	UINT8	ubMiscFlags3;

	UINT8 ubDaysOfMoraleHangover;		// used only when merc leaves team while having poor morale
	UINT8	ubNumTimesDrugUseInLifetime;		// The # times a drug has been used in the player's lifetime...

	// Flags used for the precedent to repeating oneself in Contract negotiations.  Used for quote 80 -  ~107.  Gets reset every day
	UINT32	uiPrecedentQuoteSaid;
	UINT32	uiProfileChecksum;
	INT16		sPreCombatGridNo;
	UINT8		ubTimeTillNextHatedComplaint;
	UINT8		ubSuspiciousDeath;

	INT32	iMercMercContractLength;		//Used for MERC mercs, specifies how many days the merc has gone since last page

	UINT32	uiTotalCostToDate;			// The total amount of money that has been paid to the merc for their salary
	UINT8		ubBuffer[4];
} MERCPROFILESTRUCT_ON_DISK;
extern int ASSERT_COMPILE[(sizeof(MERCPROFILESTRUCT) == sizeof(MERCPROFILESTRUCT_ON_DISK) + (NAME_LENGTH + NICKNAME_LENGTH) * (sizeof(wchar_t) - sizeof(UINT16))) * 2 - 1];


BOOLEAN LoadMercProfiles(void)
{
//	FILE *fptr;
	HWFILE fptr;
	const char *pFileName = "BINARYDATA/Prof.dat";
	UINT32 uiLoop, uiLoop2, uiLoop3;
	UINT16 usItem, usNewGun, usAmmo, usNewAmmo;
	UINT32	uiNumBytesRead;


	fptr = FileOpen(pFileName, FILE_ACCESS_READ);
	if( !fptr )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("FAILED to LoadMercProfiles from file %s", pFileName) );
		return(FALSE);
	}

	for(uiLoop=0; uiLoop< NUM_PROFILES; uiLoop++)
	{
#if 1 // XXX HACK0003 16bit strings -> wchar_t
		MERCPROFILESTRUCT_ON_DISK tempprof;
		UINT32 i;

		if (JA2EncryptedFileRead(fptr, &tempprof, sizeof(tempprof), &uiNumBytesRead) != 1)
		{
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("FAILED to Read Merc Profiles from File %d %s",uiLoop, pFileName) );
			FileClose( fptr );
fprintf(stderr, "!!!> uiLoop = %d (%d)\n", uiLoop, NUM_PROFILES);
UNIMPLEMENTED();
			return(FALSE);
		}
		for (i = 0; i < lengthof(gMercProfiles[uiLoop].zName); i++)
		{
			gMercProfiles[uiLoop].zName[i] = tempprof.zName[i];
		}
		for (i = 0; i < lengthof(gMercProfiles[uiLoop].zNickname); i++)
		{
			gMercProfiles[uiLoop].zNickname[i] = tempprof.zNickname[i];
		}
		memcpy(
			(UINT8*)&gMercProfiles[uiLoop] + sizeof(gMercProfiles[uiLoop].zName) + sizeof(gMercProfiles[uiLoop].zNickname),
			(UINT8*)&tempprof              + sizeof(tempprof.zName)              + sizeof(tempprof.zNickname),
			sizeof(gMercProfiles[uiLoop]) - sizeof(gMercProfiles[uiLoop].zName) - sizeof(gMercProfiles[uiLoop].zNickname)
		);
#else
		if( JA2EncryptedFileRead( fptr, &gMercProfiles[uiLoop], sizeof( MERCPROFILESTRUCT ), &uiNumBytesRead )  != 1)
		{
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("FAILED to Read Merc Profiles from File %d %s",uiLoop, pFileName) );
			FileClose( fptr );
			return(FALSE);
		}
#endif


		//if the Dialogue exists for the merc, allow the merc to be hired
		if( DialogueDataFileExistsForProfile( (UINT8)uiLoop, 0, FALSE, NULL ) )
		{
			gMercProfiles[uiLoop].bMercStatus = 0;
		}
		else
			gMercProfiles[uiLoop].bMercStatus = MERC_HAS_NO_TEXT_FILE;

		// if the merc has a medical deposit
		if( gMercProfiles[ uiLoop ].bMedicalDeposit )
		{
			gMercProfiles[uiLoop].sMedicalDepositAmount = CalcMedicalDeposit( &gMercProfiles[uiLoop]);
		}
		else
			gMercProfiles[uiLoop].sMedicalDepositAmount = 0;

		// ATE: New, face display indipendent of ID num now
		// Setup face index value
		// Default is the ubCharNum
		gMercProfiles[uiLoop].ubFaceIndex = (UINT8)uiLoop;

#ifndef JA2DEMO
		if ( !gGameOptions.fGunNut )
		{

			// CJC: replace guns in profile if they aren't available
			for ( uiLoop2 = 0; uiLoop2 < NUM_INV_SLOTS; uiLoop2++ )
			{
				usItem = gMercProfiles[uiLoop].inv[ uiLoop2 ];

				if ( ( Item[ usItem ].usItemClass & IC_GUN ) && ExtendedGunListGun( usItem ) )
				{
					usNewGun = StandardGunListReplacement( usItem );
					if ( usNewGun != NOTHING )
					{
						gMercProfiles[uiLoop].inv[ uiLoop2 ] = usNewGun;

						// must search through inventory and replace ammo accordingly
						for ( uiLoop3 = 0; uiLoop3 < NUM_INV_SLOTS; uiLoop3++ )
						{
							usAmmo = gMercProfiles[ uiLoop ].inv[ uiLoop3 ];
							if ( (Item[ usAmmo ].usItemClass & IC_AMMO) )
							{
								usNewAmmo = FindReplacementMagazineIfNecessary( usItem, usAmmo, usNewGun );
								if (usNewAmmo != NOTHING );
								{
									// found a new magazine, replace...
									gMercProfiles[ uiLoop ].inv[ uiLoop3 ] = usNewAmmo;
								}
							}
						}
					}
				}

			}

		} // end of if not gun nut
#endif

		//ATE: Calculate some inital attractiveness values for buddy's inital equipment...
		// Look for gun and armour
		gMercProfiles[uiLoop].bMainGunAttractiveness		= -1;
		gMercProfiles[uiLoop].bArmourAttractiveness			= -1;

		for ( uiLoop2 = 0; uiLoop2 < NUM_INV_SLOTS; uiLoop2++ )
		{
			usItem = gMercProfiles[uiLoop].inv[ uiLoop2 ];

			if ( usItem != NOTHING )
			{
				// Check if it's a gun
				if ( Item[ usItem ].usItemClass & IC_GUN )
				{
					gMercProfiles[uiLoop].bMainGunAttractiveness = Weapon[ usItem ].ubDeadliness;
				}

				// If it's armour
				if ( Item[ usItem ].usItemClass & IC_ARMOUR )
				{
					gMercProfiles[uiLoop].bArmourAttractiveness = Armour[ Item[ usItem ].ubClassIndex ].ubProtection;
				}
			}
		}


		// OK, if we are a created slot, this will get overriden at some time..

		//add up the items the merc has for the usOptionalGearCost
		gMercProfiles[ uiLoop ].usOptionalGearCost = 0;
		for ( uiLoop2 = 0; uiLoop2< NUM_INV_SLOTS; uiLoop2++ )
		{
			if ( gMercProfiles[ uiLoop ].inv[ uiLoop2 ] != NOTHING )
			{
				//get the item
				usItem = gMercProfiles[ uiLoop ].inv[ uiLoop2 ];

				//add the cost
				gMercProfiles[ uiLoop ].usOptionalGearCost += Item[ usItem ].usPrice;
			}
		}

		//These variables to get loaded in
		gMercProfiles[ uiLoop ].fUseProfileInsertionInfo = FALSE;
		gMercProfiles[ uiLoop ].sGridNo = 0;

		// ARM: this is also being done inside the profile editor, but put it here too, so this project's code makes sense
		gMercProfiles[ uiLoop ].bHatedCount[0]    = gMercProfiles[ uiLoop ].bHatedTime[0];
		gMercProfiles[ uiLoop ].bHatedCount[1]    = gMercProfiles[ uiLoop ].bHatedTime[1];
		gMercProfiles[ uiLoop ].bLearnToHateCount = gMercProfiles[ uiLoop ].bLearnToHateTime;
		gMercProfiles[ uiLoop ].bLearnToLikeCount = gMercProfiles[ uiLoop ].bLearnToLikeTime;
	}

	// SET SOME DEFAULT LOCATIONS FOR STARTING NPCS

	FileClose( fptr );

	// decide which terrorists are active
	#ifndef JA2DEMO
	DecideActiveTerrorists();
	#endif

	// initialize mercs' status
	StartSomeMercsOnAssignment( );

	// initial recruitable mercs' reputation in each town
	InitializeProfilesForTownReputation( );

	#ifdef JA2EDITOR
	gfProfileDataLoaded = TRUE;
	#endif

	// no better place..heh?.. will load faces for profiles that are 'extern'.....won't have soldiertype instances
	InitalizeStaticExternalNPCFaces( );

	// car portrait values
	LoadCarPortraitValues( );


	return(TRUE);
}

#define MAX_ADDITIONAL_TERRORISTS 4

void DecideActiveTerrorists( void )
{
	UINT8		ubLoop, ubLoop2;
	UINT8		ubTerrorist;
	UINT8		ubNumAdditionalTerrorists, ubNumTerroristsAdded = 0;
	UINT32	uiChance, uiLocationChoice;
	BOOLEAN	fFoundSpot;
	INT16		sTerroristPlacement[MAX_ADDITIONAL_TERRORISTS][2] = { {0, 0}, {0, 0}, {0, 0}, {0, 0} };

	#ifdef CRIPPLED_VERSION
		return;
	#endif

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
	gMercProfiles[ 78 ].bNPCData = 1 + ubNumAdditionalTerrorists;

	// store total terrorists
	gubNumTerrorists = 1 + ubNumAdditionalTerrorists;
}

void MakeRemainingTerroristsTougher( void )
{
	UINT8					ubRemainingTerrorists = 0, ubLoop;
	UINT16				usNewItem, usOldItem;
	OBJECTTYPE		Object;
	UINT8					ubRemainingDifficulty;

	for ( ubLoop = 0; ubLoop < NUM_TERRORISTS; ubLoop++ )
	{
		if ( gMercProfiles[ gubTerrorists[ ubLoop ] ].bMercStatus != MERC_IS_DEAD && gMercProfiles[ gubTerrorists[ ubLoop ] ].sSectorX != 0 && gMercProfiles[ gubTerrorists[ ubLoop ] ].sSectorY != 0 )
		{
			if ( gubTerrorists[ ubLoop ] == SLAY )
			{
				if ( FindSoldierByProfileID( SLAY, TRUE ) != NULL )
				{
					// Slay on player's team, doesn't count towards remaining terrorists
					continue;
				}
			}
			ubRemainingTerrorists++;
		}
	}

	ubRemainingDifficulty = (60 / gubNumTerrorists) * (gubNumTerrorists - ubRemainingTerrorists);

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

	for ( ubLoop = 0; ubLoop < NUM_TERRORISTS; ubLoop++ )
	{
		if ( gMercProfiles[ gubTerrorists[ ubLoop ] ].bMercStatus != MERC_IS_DEAD && gMercProfiles[ gubTerrorists[ ubLoop ] ].sSectorX != 0 && gMercProfiles[ gubTerrorists[ ubLoop ] ].sSectorY != 0 )
		{
			if ( gubTerrorists[ ubLoop ] == SLAY )
			{
				if ( FindSoldierByProfileID( SLAY, TRUE ) != NULL )
				{
					// Slay on player's team, doesn't count towards remaining terrorists
					continue;
				}
			}

			if ( usOldItem != NOTHING )
			{
				RemoveObjectFromSoldierProfile( gubTerrorists[ ubLoop ], usOldItem );
			}
			PlaceObjectInSoldierProfile( gubTerrorists[ ubLoop ], &Object );
		}
	}
}

void DecideOnAssassin( void )
{
	UINT8		ubAssassinPossibility[NUM_ASSASSINS] = { NO_PROFILE, NO_PROFILE, NO_PROFILE, NO_PROFILE, NO_PROFILE, NO_PROFILE };
	UINT8		ubAssassinsPossible = 0;
	UINT8		ubLoop, ubLoop2;
	UINT8		ubTown;

	#ifdef CRIPPLED_VERSION
		return;
	#endif

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

void StartSomeMercsOnAssignment(void)
{
	UINT32 uiCnt;
	MERCPROFILESTRUCT *pProfile;
	UINT32 uiChance;

	// some randomly picked A.I.M. mercs will start off "on assignment" at the beginning of each new game
	for( uiCnt = 0; uiCnt < AIM_AND_MERC_MERCS; uiCnt++)
	{
		pProfile = &(gMercProfiles[ uiCnt ]);

		// calc chance to start on assignment
		uiChance = 5 * pProfile->bExpLevel;

		if (Random(100) < uiChance)
		{
			pProfile->bMercStatus = MERC_WORKING_ELSEWHERE;
			pProfile->uiDayBecomesAvailable = 1 + Random(6 + (pProfile->bExpLevel / 2) );		// 1-(6 to 11) days
		}
		else
		{
			pProfile->bMercStatus = MERC_OK;
			pProfile->uiDayBecomesAvailable = 0;
		}

		pProfile->uiPrecedentQuoteSaid = 0;
		pProfile->ubDaysOfMoraleHangover = 0;
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

UINT16 CalcCompetence( MERCPROFILESTRUCT * pProfile )
{
	UINT32 uiStats, uiSkills, uiActionPoints, uiSpecialSkills;
	UINT16 usCompetence;


	// count life twice 'cause it's also hit points
	// mental skills are halved 'cause they're actually not that important within the game
	uiStats = ((2 * pProfile->bLifeMax) + pProfile->bStrength + pProfile->bAgility + pProfile->bDexterity + ((pProfile->bLeadership + pProfile->bWisdom) / 2)) / 3;

	// marksmanship is very important, count it double
	uiSkills = (UINT32) ((2   * (pow(pProfile->bMarksmanship, 3) / 10000)) +
												1.5 *	(pow(pProfile->bMedical, 3) / 10000) +
															(pow(pProfile->bMechanical, 3) / 10000) +
															(pow(pProfile->bExplosive, 3) / 10000));

	// action points
	uiActionPoints = 5 + (((10 * pProfile->bExpLevel +
													 3 * pProfile->bAgility  +
													 2 * pProfile->bLifeMax  +
													 2 * pProfile->bDexterity) + 20) / 40);


	// count how many he has, don't care what they are
	uiSpecialSkills = ((pProfile->bSkillTrait != 0) ? 1 : 0) + ((pProfile->bSkillTrait2 != 0) ? 1 : 0);

	usCompetence = (UINT16) ((pow(pProfile->bExpLevel, 0.2) * uiStats * uiSkills * (uiActionPoints - 6) * (1 + (0.05 * (FLOAT)uiSpecialSkills))) / 1000);

	// this currently varies from about 10 (Flo) to 1200 (Gus)
	return(usCompetence);
}

INT16 CalcMedicalDeposit( MERCPROFILESTRUCT * pProfile )
{
	UINT16 usDeposit;

	// this rounds off to the nearest hundred
	usDeposit = (((5 * CalcCompetence(pProfile)) + 50) / 100) * 100;

	return(usDeposit);
}

SOLDIERTYPE * FindSoldierByProfileID( UINT8 ubProfileID, BOOLEAN fPlayerMercsOnly )
{
	UINT8 ubLoop, ubLoopLimit;
	SOLDIERTYPE * pSoldier;

	if (fPlayerMercsOnly)
	{
		ubLoopLimit = gTacticalStatus.Team[0].bLastID;
	}
	else
	{
		ubLoopLimit = MAX_NUM_SOLDIERS;
	}

	for (ubLoop = 0, pSoldier = MercPtrs[0]; ubLoop < ubLoopLimit; ubLoop++, pSoldier++)
	{
		if (pSoldier->bActive && pSoldier->ubProfile == ubProfileID)
		{
			return( pSoldier );
		}
	}
	return( NULL );
}



SOLDIERTYPE *ChangeSoldierTeam( SOLDIERTYPE *pSoldier, UINT8 ubTeam )
{
	UINT8										ubID;
	SOLDIERTYPE							*pNewSoldier = NULL;
	SOLDIERCREATE_STRUCT		MercCreateStruct;
	UINT32									cnt;
	INT16										sOldGridNo;

	UINT8										ubOldID;
	UINT32									uiOldUniqueId;

	UINT32									uiSlot;
	SOLDIERTYPE							*pGroupMember;

	if (gfInTalkPanel)
	{
		DeleteTalkingMenu();
	}

	// Save merc id for this guy...
	ubID = pSoldier->ubID;

	ubOldID = ubID;
	uiOldUniqueId = pSoldier->uiUniqueSoldierIdValue;

	sOldGridNo = pSoldier->sGridNo;

	// Remove him from the game!
	InternalTacticalRemoveSoldier( ubID, FALSE );

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

	if ( ubTeam == gbPlayerNum )
	{
		MercCreateStruct.fPlayerMerc = TRUE;
	}

	if ( TacticalCreateSoldier( &MercCreateStruct, &ubID ) )
	{
		pNewSoldier = MercPtrs[ ubID ];

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
		pNewSoldier->bScientific										= pSoldier->bScientific;
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
		for ( uiSlot = 0; uiSlot < guiNumMercSlots; uiSlot++ )
		{
			pGroupMember = MercSlots[ uiSlot ];

			if ( pGroupMember != NULL )
			{
				if ( pGroupMember->ubTargetID == pSoldier->ubID )
				{
					pGroupMember->ubTargetID = pNewSoldier->ubID;
				}
			}
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
			AddSoldierToSectorNoCalculateDirectionUseAnimation( ubID, pSoldier->usAnimState, pSoldier->usAniCode );
			HandleSight(pNewSoldier, SIGHT_LOOK | SIGHT_RADIO);
		}

		// fix up the event queue...
	//	ChangeSoldierIDInQueuedEvents( ubOldID, uiOldUniqueId, pNewSoldier->ubID, pNewSoldier->uiUniqueSoldierIdValue );

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
	SOLDIERTYPE *pSoldier, *pNewSoldier;

	// Get soldier pointer
	pSoldier = FindSoldierByProfileID( ubCharNum, FALSE );

	if (!pSoldier)
	{
		return( FALSE );
	}

	// OK, set recruit flag..
	gMercProfiles[ ubCharNum ].ubMiscFlags |= PROFILE_MISC_FLAG_RECRUITED;

	// Add this guy to our team!
	pNewSoldier = ChangeSoldierTeam( pSoldier, gbPlayerNum );

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
	if ( AddCharacterToSquad( pNewSoldier, (INT8)CurrentSquad( ) ) == FALSE )
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
	SOLDIERTYPE *pSoldier, *pNewSoldier;

	// Get soldier pointer
	pSoldier = FindSoldierByProfileID( ubCharNum, FALSE );

	if (!pSoldier)
	{
		return( FALSE );
	}

	// OK, set recruit flag..
	gMercProfiles[ ubCharNum ].ubMiscFlags |= PROFILE_MISC_FLAG_EPCACTIVE;

	gMercProfiles[ ubCharNum ].ubMiscFlags3 &= ~PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE;

	// Add this guy to our team!
	pNewSoldier = ChangeSoldierTeam( pSoldier, gbPlayerNum );
	pNewSoldier->ubWhatKindOfMercAmI = MERC_TYPE__EPC;

	// Try putting them into the current squad
	if ( AddCharacterToSquad( pNewSoldier, (INT8)CurrentSquad( ) ) == FALSE )
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


BOOLEAN UnRecruitEPC( UINT8 ubCharNum )
{
	SOLDIERTYPE *pSoldier, *pNewSoldier;

	// Get soldier pointer
	pSoldier = FindSoldierByProfileID( ubCharNum, FALSE );

	if (!pSoldier)
	{
		return( FALSE );
	}

	if (pSoldier->ubWhatKindOfMercAmI != MERC_TYPE__EPC)
	{
		return( FALSE );
	}

  if ( pSoldier->bAssignment < ON_DUTY )
  {
    ResetDeadSquadMemberList( pSoldier->bAssignment );
  }

	// Rmeove from squad....
	RemoveCharacterFromSquads( pSoldier );

	// O< check if this is the only guy in the sector....
	if ( gusSelectedSoldier == pSoldier->ubID )
	{
		gusSelectedSoldier = NOBODY;
	}

	// OK, UN set recruit flag..
	gMercProfiles[ ubCharNum ].ubMiscFlags &= (~PROFILE_MISC_FLAG_EPCACTIVE);

	// update sector values to current

	// check to see if this person should disappear from the map after this
	if ( (ubCharNum == JOHN || ubCharNum == MARY) && pSoldier->sSectorX == 13 && pSoldier->sSectorY == MAP_ROW_B && pSoldier->bSectorZ == 0 )
	{
		gMercProfiles[ ubCharNum ].sSectorX = 0;
		gMercProfiles[ ubCharNum ].sSectorY = 0;
		gMercProfiles[ ubCharNum ].bSectorZ = 0;
	}
	else
	{
		gMercProfiles[ ubCharNum ].sSectorX = pSoldier->sSectorX;
		gMercProfiles[ ubCharNum ].sSectorY = pSoldier->sSectorY;
		gMercProfiles[ ubCharNum ].bSectorZ = pSoldier->bSectorZ;
	}

	// how do we decide whether or not to set this?
	gMercProfiles[ ubCharNum ].fUseProfileInsertionInfo = TRUE;
	gMercProfiles[ ubCharNum ].ubMiscFlags3 |= PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE;

	// Add this guy to CIV team!
	pNewSoldier = ChangeSoldierTeam( pSoldier, CIV_TEAM );

  UpdateTeamPanelAssignments( );

	return( TRUE );
}



INT8 WhichBuddy( UINT8 ubCharNum, UINT8 ubBuddy )
{
	MERCPROFILESTRUCT *	pProfile;
	INT8								bLoop;

	pProfile = &( gMercProfiles[ ubCharNum ] );

	for (bLoop = 0; bLoop < 3; bLoop++)
	{
		if ( pProfile->bBuddy[bLoop] == ubBuddy )
		{
			return( bLoop );
		}
	}
	return( -1 );
}

INT8 WhichHated( UINT8 ubCharNum, UINT8 ubHated )
{
	MERCPROFILESTRUCT *	pProfile;
	INT8								bLoop;

	pProfile = &( gMercProfiles[ ubCharNum ] );

	for (bLoop = 0; bLoop < 3; bLoop++)
	{
		if ( pProfile->bHated[bLoop] == ubHated )
		{
			return( bLoop );
		}
	}
	return( -1 );
}


BOOLEAN IsProfileATerrorist( UINT8 ubProfile )
{
	if ( ubProfile == 83 || ubProfile == 111 ||
			 ubProfile == 64 || ubProfile == 112 ||
			 ubProfile == 82 || ubProfile == 110 )
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}

BOOLEAN IsProfileAHeadMiner( UINT8 ubProfile )
{
	if ( ubProfile == 106 || ubProfile == 148 ||
			 ubProfile == 156 || ubProfile == 157 ||
			 ubProfile == 158 )
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


void UpdateSoldierPointerDataIntoProfile( BOOLEAN fPlayerMercs )
{
	UINT32 uiCount;
	SOLDIERTYPE *pSoldier = NULL;
	MERCPROFILESTRUCT * pProfile;
	BOOLEAN				fDoCopy = FALSE;

	for( uiCount=0; uiCount < guiNumMercSlots; uiCount++)
	{
		pSoldier = MercSlots[ uiCount ];

		if ( pSoldier != NULL )
		{
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
					// get profile...
					pProfile = &( gMercProfiles[ pSoldier->ubProfile ] );

					// Copy....
					pProfile->bLife 										= pSoldier->bLife;
					pProfile->bLifeMax									= pSoldier->bLifeMax;
					pProfile->bAgility									= pSoldier->bAgility;
					pProfile->bLeadership								= pSoldier->bLeadership;
					pProfile->bDexterity								= pSoldier->bDexterity;
					pProfile->bStrength									= pSoldier->bStrength;
					pProfile->bWisdom										= pSoldier->bWisdom;
					pProfile->bExpLevel									= pSoldier->bExpLevel;
					pProfile->bMarksmanship							= pSoldier->bMarksmanship;
					pProfile->bMedical									= pSoldier->bMedical;
					pProfile->bMechanical								= pSoldier->bMechanical;
					pProfile->bExplosive								= pSoldier->bExplosive;
					pProfile->bScientific								= pSoldier->bScientific;
				}
			}
		}
	}
}



BOOLEAN DoesMercHaveABuddyOnTheTeam( UINT8 ubMercID )
{
	UINT8	ubCnt;
	INT8	bBuddyID;

	// loop through the list of people the merc is buddies with
	for(ubCnt=0; ubCnt< 3; ubCnt++)
	{
		//see if the merc has a buddy on the team
		bBuddyID = gMercProfiles[ ubMercID ].bBuddy[ubCnt];

		//If its not a valid 'buddy'
		if( bBuddyID < 0 )
			continue;

		if( IsMercOnTeam( bBuddyID ) )
		{
			if( !IsMercDead( bBuddyID ) )
			{
				return( TRUE );
			}
		}
	}

	return( FALSE );
}

BOOLEAN MercIsHot( SOLDIERTYPE * pSoldier )
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

SOLDIERTYPE * SwapLarrysProfiles( SOLDIERTYPE * pSoldier )
{
	UINT8	ubSrcProfile;
	UINT8	ubDestProfile;
	MERCPROFILESTRUCT * pNewProfile;

	ubSrcProfile = pSoldier->ubProfile;
	if ( ubSrcProfile == LARRY_NORMAL)
	{
		ubDestProfile = LARRY_DRUNK;
	}
	else if ( ubSrcProfile == LARRY_DRUNK )
	{
		ubDestProfile = LARRY_NORMAL;
	}
	else
	{
		// I don't think so!
		return( pSoldier );
	}

	pNewProfile = &gMercProfiles[ ubDestProfile ];
	pNewProfile->ubMiscFlags2 = gMercProfiles[ ubSrcProfile ].ubMiscFlags2;
	pNewProfile->ubMiscFlags = gMercProfiles[ ubSrcProfile ].ubMiscFlags;
	pNewProfile->sSectorX = gMercProfiles[ ubSrcProfile ].sSectorX;
	pNewProfile->sSectorY = gMercProfiles[ ubSrcProfile ].sSectorY;
	pNewProfile->uiDayBecomesAvailable = gMercProfiles[ ubSrcProfile ].uiDayBecomesAvailable;
	pNewProfile->usKills = gMercProfiles[ ubSrcProfile ].usKills;
	pNewProfile->usAssists = gMercProfiles[ ubSrcProfile ].usAssists;
	pNewProfile->usShotsFired = gMercProfiles[ ubSrcProfile ].usShotsFired;
	pNewProfile->usShotsHit = gMercProfiles[ ubSrcProfile ].usShotsHit;
	pNewProfile->usBattlesFought = gMercProfiles[ ubSrcProfile ].usBattlesFought;
	pNewProfile->usTimesWounded = gMercProfiles[ ubSrcProfile ].usTimesWounded;
	pNewProfile->usTotalDaysServed = gMercProfiles[ ubSrcProfile ].usTotalDaysServed;
	pNewProfile->bResigned = gMercProfiles[ ubSrcProfile ].bResigned;
	pNewProfile->bActive = gMercProfiles[ ubSrcProfile ].bActive;
	pNewProfile->fUseProfileInsertionInfo = gMercProfiles[ ubSrcProfile ].fUseProfileInsertionInfo;
	pNewProfile->sGridNo = gMercProfiles[ ubSrcProfile ].sGridNo;
	pNewProfile->ubQuoteActionID = gMercProfiles[ ubSrcProfile ].ubQuoteActionID;
	pNewProfile->ubLastQuoteSaid = gMercProfiles[ ubSrcProfile ].ubLastQuoteSaid;
	pNewProfile->ubStrategicInsertionCode = gMercProfiles[ ubSrcProfile ].ubStrategicInsertionCode;
	pNewProfile->bMercStatus = gMercProfiles[ ubSrcProfile ].bMercStatus;
	pNewProfile->bSectorZ = gMercProfiles[ ubSrcProfile ].bSectorZ;
	pNewProfile->usStrategicInsertionData = gMercProfiles[ ubSrcProfile ].usStrategicInsertionData;
	pNewProfile->sTrueSalary = gMercProfiles[ ubSrcProfile ].sTrueSalary;
	pNewProfile->ubMiscFlags3 = gMercProfiles[ ubSrcProfile ].ubMiscFlags3;
	pNewProfile->ubDaysOfMoraleHangover = gMercProfiles[ ubSrcProfile ].ubDaysOfMoraleHangover;
	pNewProfile->ubNumTimesDrugUseInLifetime = gMercProfiles[ ubSrcProfile ].ubNumTimesDrugUseInLifetime;
	pNewProfile->uiPrecedentQuoteSaid = gMercProfiles[ ubSrcProfile ].uiPrecedentQuoteSaid;
	pNewProfile->sPreCombatGridNo = gMercProfiles[ ubSrcProfile ].sPreCombatGridNo;

// CJC: this is causing problems so just skip the transfer of exp...
/*
	pNewProfile->sLifeGain = gMercProfiles[ ubSrcProfile ].sLifeGain;
	pNewProfile->sAgilityGain = gMercProfiles[ ubSrcProfile ].sAgilityGain;
	pNewProfile->sDexterityGain = gMercProfiles[ ubSrcProfile ].sDexterityGain;
	pNewProfile->sStrengthGain = gMercProfiles[ ubSrcProfile ].sStrengthGain;
	pNewProfile->sLeadershipGain = gMercProfiles[ ubSrcProfile ].sLeadershipGain;
	pNewProfile->sWisdomGain = gMercProfiles[ ubSrcProfile ].sWisdomGain;
	pNewProfile->sExpLevelGain = gMercProfiles[ ubSrcProfile ].sExpLevelGain;
	pNewProfile->sMarksmanshipGain = gMercProfiles[ ubSrcProfile ].sMarksmanshipGain;
	pNewProfile->sMedicalGain = gMercProfiles[ ubSrcProfile ].sMedicalGain;
	pNewProfile->sMechanicGain = gMercProfiles[ ubSrcProfile ].sMechanicGain;
	pNewProfile->sExplosivesGain = gMercProfiles[ ubSrcProfile ].sExplosivesGain;

	pNewProfile->bLifeDelta = gMercProfiles[ ubSrcProfile ].bLifeDelta;
	pNewProfile->bAgilityDelta = gMercProfiles[ ubSrcProfile ].bAgilityDelta;
	pNewProfile->bDexterityDelta = gMercProfiles[ ubSrcProfile ].bDexterityDelta;
	pNewProfile->bStrengthDelta = gMercProfiles[ ubSrcProfile ].bStrengthDelta;
	pNewProfile->bLeadershipDelta = gMercProfiles[ ubSrcProfile ].bLeadershipDelta;
	pNewProfile->bWisdomDelta = gMercProfiles[ ubSrcProfile ].bWisdomDelta;
	pNewProfile->bExpLevelDelta = gMercProfiles[ ubSrcProfile ].bExpLevelDelta;
	pNewProfile->bMarksmanshipDelta = gMercProfiles[ ubSrcProfile ].bMarksmanshipDelta;
	pNewProfile->bMedicalDelta = gMercProfiles[ ubSrcProfile ].bMedicalDelta;
	pNewProfile->bMechanicDelta = gMercProfiles[ ubSrcProfile ].bMechanicDelta;
	pNewProfile->bExplosivesDelta = gMercProfiles[ ubSrcProfile ].bExplosivesDelta;
	*/

	memcpy( pNewProfile->bInvStatus, gMercProfiles[ ubSrcProfile ].bInvStatus , sizeof( UINT8) * 19 );
	memcpy( pNewProfile->bInvStatus, gMercProfiles[ ubSrcProfile ].bInvStatus , sizeof( UINT8) * 19 );
	memcpy( pNewProfile->inv , gMercProfiles[ ubSrcProfile ].inv , sizeof( UINT16 ) * 19 );
	memcpy( pNewProfile->bMercTownReputation , gMercProfiles[ ubSrcProfile ].bMercTownReputation , sizeof( UINT8 ) * 20 );

	// remove face
	DeleteSoldierFace( pSoldier );

	pSoldier->ubProfile = ubDestProfile;

	// create new face
	pSoldier->iFaceIndex = InitSoldierFace( pSoldier );

	// replace profile in group
	ReplaceSoldierProfileInPlayerGroup( pSoldier->ubGroupID, ubSrcProfile, ubDestProfile );

	pSoldier->bStrength =			pNewProfile->bStrength + pNewProfile->bStrengthDelta;
	pSoldier->bDexterity =		pNewProfile->bDexterity + pNewProfile->bDexterityDelta;
	pSoldier->bAgility =			pNewProfile->bAgility + pNewProfile->bAgilityDelta;
	pSoldier->bWisdom =				pNewProfile->bWisdom + pNewProfile->bWisdomDelta;
	pSoldier->bExpLevel =			pNewProfile->bExpLevel + pNewProfile->bExpLevelDelta;
	pSoldier->bLeadership =		pNewProfile->bLeadership + pNewProfile->bLeadershipDelta;

	pSoldier->bMarksmanship =	pNewProfile->bMarksmanship + pNewProfile->bMarksmanshipDelta;
	pSoldier->bMechanical =		pNewProfile->bMechanical + pNewProfile->bMechanicDelta;
	pSoldier->bMedical =			pNewProfile->bMedical + pNewProfile->bMedicalDelta;
	pSoldier->bExplosive =		pNewProfile->bExplosive + pNewProfile->bExplosivesDelta;

	if ( pSoldier->ubProfile == LARRY_DRUNK )
	{
		SetFactTrue( FACT_LARRY_CHANGED );
	}
	else
	{
		SetFactFalse( FACT_LARRY_CHANGED );
	}

	DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );

	return( pSoldier );
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
