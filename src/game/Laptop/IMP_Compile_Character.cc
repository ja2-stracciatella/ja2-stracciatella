#include "Directories.h"
#include "Laptop.h"
#include "CharProfile.h"
#include "Debug.h"
#include "GameInstance.h"
#include "GamePolicy.h"
#include "IMPPolicy.h"
#include "ContentManager.h"
#include "Render_Dirty.h"
#include "IMP_Portraits.h"
#include "IMP_SkillTraits.h"
#include "IMP_Compile_Character.h"
#include "Soldier_Profile_Type.h"
#include "Soldier_Profile.h"
#include "Animation_Data.h"
#include "Random.h"
#include "LaptopSave.h"


#define ATTITUDE_LIST_SIZE 20


static INT32 AttitudeList[ATTITUDE_LIST_SIZE];
static INT32 iLastElementInAttitudeList = 0;

static INT32 SkillsList[ATTITUDE_LIST_SIZE];
static INT32 iLastElementInSkillsList = 0;

static INT32 PersonalityList[ATTITUDE_LIST_SIZE];
static INT32 iLastElementInPersonalityList = 0;

static void SelectMercFace(void);


void CreateACharacterFromPlayerEnteredStats(void)
{
	MERCPROFILESTRUCT& p = GetProfile(PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId);

	p.zName = pFullName;
	p.zNickname = pNickName;

	p.bSex = fCharacterIsMale ? MALE : FEMALE;

	p.bLifeMax    = iHealth;
	p.bLife       = iHealth;
	p.bAgility    = iAgility;
	p.bStrength   = iStrength;
	p.bDexterity  = iDexterity;
	p.bWisdom     = iWisdom;
	p.bLeadership = iLeadership;

	p.bMarksmanship = iMarksmanship;
	p.bMedical      = iMedical;
	p.bMechanical   = iMechanical;
	p.bExplosive    = iExplosives;

	p.bPersonalityTrait = iPersonality;

	p.bAttitude = iAttitude;

	p.bExpLevel = GCM->getIMPPolicy()->getStartingLevel();

	// set time away
	p.bMercStatus = 0;

	SelectMercFace();
}

static void CreatePlayerAttitude(void)
{
	// this function will 'roll a die' and decide if any attitude does exists
	INT32	iAttitudeHits[NUM_ATTITUDES] = { 0 };

	iAttitude = ATT_NORMAL;

	if (iLastElementInAttitudeList == 0)
	{
		return;
	}

	// count # of hits for each attitude
	for (INT32 i = 0; i < iLastElementInAttitudeList; i++)
	{
		iAttitudeHits[AttitudeList[i]]++;
	}

	// find highest # of hits for any attitude
	INT32 iHighestHits = 0;
	INT32 iNumAttitudesWithHighestHits = 0;
	for (INT32 i = 0; i < NUM_ATTITUDES; i++)
	{
		if (iAttitudeHits[i])
		{
			if (iAttitudeHits[i] > iHighestHits)
			{
				iHighestHits = iAttitudeHits[i];
				iNumAttitudesWithHighestHits = 1;
			}
			else if (iAttitudeHits[i] == iHighestHits)
			{
				iNumAttitudesWithHighestHits++;
			}
		}
	}

	INT32 iDiceValue = Random(iNumAttitudesWithHighestHits);

	// find attitude
	INT32 iCounter2 = 0;
	for (INT32 i = 0; i < NUM_ATTITUDES; i++)
	{
		if (iAttitudeHits[i] == iHighestHits)
		{
			if (iCounter2 == iDiceValue)
			{
				// this is it!
				iAttitude = i;
				break;
			}
			else
			{
				// one of the next attitudes...
				iCounter2++;
			}
		}
	}
}


void AddAnAttitudeToAttitudeList( INT8 bAttitude )
{
	// adds an attitude to attitude list
	if (iLastElementInAttitudeList < ATTITUDE_LIST_SIZE)
	{
		AttitudeList[iLastElementInAttitudeList++] = bAttitude;
	}
}


void AddSkillToSkillList( INT8 bSkill )
{
	// adds a skill to skills list
	if (iLastElementInSkillsList < ATTITUDE_LIST_SIZE)
	{
		SkillsList[iLastElementInSkillsList++] = bSkill;
	}
}


static void RemoveSkillFromSkillsList(INT32 const Skill)
{
	for (INT32 i = 0; i != iLastElementInSkillsList;)
	{
		if (SkillsList[i] == Skill)
		{
			SkillsList[i] = SkillsList[--iLastElementInSkillsList];
		}
		else
		{
			++i;
		}
	}
}


static void ValidateSkillsList(void)
{
	// remove from the generated traits list any traits that don't match
	// the character's skills
	MERCPROFILESTRUCT& p = GetProfile(PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId);

	if (p.bMechanical == 0)
	{
		// without mechanical, electronics is useless
		RemoveSkillFromSkillsList(ELECTRONICS);
	}

	// special check for lockpicking
	INT32 iValue = p.bMechanical;
	iValue = iValue * p.bWisdom    / 100;
	iValue = iValue * p.bDexterity / 100;
	if (iValue + gbSkillTraitBonus[LOCKPICKING] < 50)
	{
		// not good enough for lockpicking!
		RemoveSkillFromSkillsList(LOCKPICKING);
	}

	if (p.bMarksmanship == 0)
	{
		// without marksmanship, the following traits are useless:
		RemoveSkillFromSkillsList(AUTO_WEAPS);
		RemoveSkillFromSkillsList(HEAVY_WEAPS);
	}
}


static void CreatePlayerSkills(void)
{
	ValidateSkillsList();

	if (gamepolicy(imp_pick_skills_directly))
	{
		// the skills selected are distinct and there are at most 2
		iSkillA = NO_SKILLTRAIT;
		iSkillB = NO_SKILLTRAIT;

		if (iLastElementInSkillsList == 1)
		{
			// grant expert level if only 1 skill is chosen, unless the skill as no expert level
			iSkillA = SkillsList[0];
			if (iSkillA != ELECTRONICS && iSkillA != AMBIDEXT && iSkillA != CAMOUFLAGED)
			{
				iSkillB = iSkillA;
			}
		}
		else if (iLastElementInSkillsList == 2)
		{
			iSkillA = SkillsList[0];
			iSkillB = SkillsList[1];
		}
		else if (iLastElementInSkillsList > 2)
		{
			// This should be impossible
			SLOGA("Invalid number ({}) of skills selected", iLastElementInSkillsList);
		}
	
		return;
	}

	iSkillA = SkillsList[Random(iLastElementInSkillsList)];

	// there is no expert level these skills
	if (iSkillA == ELECTRONICS || iSkillA == AMBIDEXT) RemoveSkillFromSkillsList(iSkillA);

	if (iLastElementInSkillsList == 0)
	{
		iSkillB = NO_SKILLTRAIT;
	}
	else
	{
		iSkillB = SkillsList[Random(iLastElementInSkillsList)];
	}
}


void AddAPersonalityToPersonalityList(INT8 bPersonality)
{
	// CJC, Oct 26 98: prevent personality list from being generated
	// because no dialogue was written to support PC personality quotes

	// BUT we can manage this for PSYCHO okay
	if (bPersonality != PSYCHO) return;

	// will add a persoanlity to persoanlity list
	if (iLastElementInPersonalityList < ATTITUDE_LIST_SIZE)
	{
		PersonalityList[iLastElementInPersonalityList++] = bPersonality;
	}
}


static void CreatePlayerPersonality(void)
{
	// only psycho is available since we have no quotes
	// SO if the array is not empty, give them psycho!
	if (iLastElementInPersonalityList == 0)
	{
		iPersonality = NO_PERSONALITYTRAIT;
	}
	else
	{
		iPersonality = PSYCHO;
	}

	/*
	// this function will 'roll a die' and decide if any Personality does exists
	INT32 iDiceValue = 0;
	INT32 iCounter = 0;
	INT32 iSecondAttempt = -1;

	// roll dice
	iDiceValue = Random( iLastElementInPersonalityList + 1 );

	iPersonality = NO_PERSONALITYTRAIT;
	if( PersonalityList[ iDiceValue ] !=  NO_PERSONALITYTRAIT )
	{
		for( iCounter = 0; iCounter < iLastElementInPersonalityList; iCounter++ )
		{
			if( iCounter != iDiceValue )
			{
				if( PersonalityList[ iCounter ] == PersonalityList[ iDiceValue ] )
				{
					if( PersonalityList[ iDiceValue ] != PSYCHO )
					{
						iPersonality = PersonalityList[ iDiceValue ];
					}
					else
					{
						iSecondAttempt = iCounter;
					}
					if( iSecondAttempt != iCounter )
					{
						iPersonality = PersonalityList[ iDiceValue ];
					}

				}
			}
		}
	}*/
}


void CreatePlayersPersonalitySkillsAndAttitude( void )
{
	// creates personality and attitudes from curretly built list
	CreatePlayerPersonality();
	CreatePlayerAttitude();
}


void ResetSkillsAttributesAndPersonality( void )
{
	// reset count of skills attributes and personality
	iLastElementInPersonalityList = 0;
	iLastElementInSkillsList      = 0;
	iLastElementInAttitudeList    = 0;
}


static void SetMercSkinAndHairColors(void);


static void SelectMercFace(void)
{
	// this procedure will select the approriate face for the merc and save offsets
	MERCPROFILESTRUCT& p = GetProfile(PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId);

	// now the offsets
	p.ubFaceIndex = 200 + iPortraitNumber;

	// eyes
	p.usEyesX = 0;
	p.usEyesY = 0;

	// mouth
	p.usMouthX = 0;
	p.usMouthY = 0;

	// set merc skins and hair color
	SetMercSkinAndHairColors();
}


static void SetMercSkinAndHairColors(void)
{
#define PINKSKIN  "PINKSKIN"
#define TANSKIN   "TANSKIN"
#define DARKSKIN  "DARKSKIN"
#define BLACKSKIN "BLACKSKIN"

#define BROWNHEAD "BROWNHEAD"
#define BLACKHEAD "BLACKHEAD" // black skin till here
#define WHITEHEAD "WHITEHEAD" // dark skin till here
#define BLONDHEAD "BLONDHEAD"
#define REDHEAD   "REDHEAD"   // pink/tan skin till here

	static const struct
	{
		const char* Skin;
		const char* Hair;
	} Colors[] =
	{
		{ BLACKSKIN, BROWNHEAD },
		{ TANSKIN,   BROWNHEAD },
		{ TANSKIN,   BROWNHEAD },
		{ DARKSKIN,  BROWNHEAD },
		{ TANSKIN,   BROWNHEAD },
		{ DARKSKIN,  BLACKHEAD },
		{ TANSKIN,   BROWNHEAD },
		{ TANSKIN,   BROWNHEAD },
		{ TANSKIN,   BROWNHEAD },
		{ PINKSKIN,  BROWNHEAD },
		{ TANSKIN,   BLACKHEAD },
		{ TANSKIN,   BLACKHEAD },
		{ PINKSKIN,  BROWNHEAD },
		{ BLACKSKIN, BROWNHEAD },
		{ TANSKIN,   REDHEAD   },
		{ TANSKIN,   BLONDHEAD }
	};

	Assert(iPortraitNumber < static_cast<INT32>(lengthof(Colors)));
	MERCPROFILESTRUCT& p = GetProfile(PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId);
	p.HAIR = Colors[iPortraitNumber].Hair;
	p.SKIN = Colors[iPortraitNumber].Skin;
}


static BOOLEAN ShouldThisMercHaveABigBody(void);


void HandleMercStatsForChangesInFace(void)
{
	AddSelectedSkillsToSkillsList();

	CreatePlayerSkills();

	MERCPROFILESTRUCT& p = GetProfile(PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId);

	// body type
	if (fCharacterIsMale)
	{
		if (ShouldThisMercHaveABigBody())
		{
			p.ubBodyType = BIGMALE;
			if (iSkillA == MARTIALARTS) iSkillA = HANDTOHAND;
			if (iSkillB == MARTIALARTS) iSkillB = HANDTOHAND;
		}
		else
		{
			p.ubBodyType = REGMALE;
		}
	}
	else
	{
		p.ubBodyType = REGFEMALE;
		if (iSkillA == MARTIALARTS) iSkillA = HANDTOHAND;
		if (iSkillB == MARTIALARTS) iSkillB = HANDTOHAND;
	}

	// skill trait
	p.bSkillTrait  = iSkillA;
	p.bSkillTrait2 = iSkillB;
}


static BOOLEAN ShouldThisMercHaveABigBody(void)
{
	// should this merc be a big body typ
	return
		(iPortraitNumber == 0 || iPortraitNumber == 6 || iPortraitNumber == 7) &&
		gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId].bStrength >= 75;
}
