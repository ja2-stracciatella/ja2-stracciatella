#include "SkillCheck.h"
#include "Soldier_Profile.h"
#include "Random.h"
#include "Items.h"
#include "Dialogue_Control.h"
#include "Overhead.h"
#include "Soldier_Macros.h"
#include "Isometric_Utils.h"
#include "Morale.h"
#include "Drugs_And_Alcohol.h"
#include "StrategicMap.h"


INT8 EffectiveStrength(const SOLDIERTYPE* s)
{
	// Effective strength is:
	// 1/2 full strength
	// plus 1/2 strength scaled according to how hurt we are
	const INT8 bBandaged    = s->bLifeMax - s->bLife - s->bBleeding;
	INT32      iEffStrength = s->bStrength / 2;
	iEffStrength += (s->bStrength / 2) * (s->bLife + bBandaged / 2) / (s->bLifeMax);

	// ATE: Make sure at least 2...
	iEffStrength = std::max(iEffStrength, 2);

	return( (INT8) iEffStrength );
}


INT8 EffectiveWisdom(const SOLDIERTYPE* s)
{
	return EffectStatForBeingDrunk(s, s->bWisdom);
}


INT8 EffectiveAgility(const SOLDIERTYPE* s)
{
	int32_t iEffAgility = EffectStatForBeingDrunk(s, s->bAgility);

	return s->sWeightCarriedAtTurnStart > 100 ? (iEffAgility * 100) / s->sWeightCarriedAtTurnStart : iEffAgility;
}


INT8 EffectiveMechanical(const SOLDIERTYPE* s)
{
	return EffectStatForBeingDrunk(s, s->bMechanical);
}


INT8 EffectiveExplosive(const SOLDIERTYPE* s)
{
	return EffectStatForBeingDrunk(s, s->bExplosive);
}


INT8 EffectiveMedical(const SOLDIERTYPE* s)
{
	return EffectStatForBeingDrunk(s, s->bMedical);
}


INT8 EffectiveLeadership(const SOLDIERTYPE* s)
{
	return GetDrunkLevel(s) == FEELING_GOOD ? s->bLeadership * 120 / 100 : s->bLeadership;
}


INT8 EffectiveExpLevel(const SOLDIERTYPE* s)
{
	INT32 iEffExpLevel;
	INT8  bDrunkLevel;
	INT32 iExpModifier[] = {
		0, // SOBER
		0, // Feeling good
		-1, // Borderline
		-2, // Drunk
		0, // Hung
	};

	iEffExpLevel = s->bExpLevel;

	bDrunkLevel = GetDrunkLevel(s);

	iEffExpLevel = iEffExpLevel + iExpModifier[ bDrunkLevel ];

	if (s->ubProfile != NO_PROFILE)
	{
		if ((gMercProfiles[s->ubProfile].bPersonalityTrait == CLAUSTROPHOBIC) && s->bActive && s->bInSector && gWorldSector.z > 0)
		{
			// claustrophobic!
			iEffExpLevel--;
		}
	}

	if (iEffExpLevel < 1)
	{
		// can't go below 1
		return( 1 );
	}
	else
	{
		return( (INT8) iEffExpLevel );
	}
}


INT8 EffectiveMarksmanship(const SOLDIERTYPE* s)
{
	return EffectStatForBeingDrunk(s, s->bMarksmanship);
}


INT8 EffectiveDexterity(const SOLDIERTYPE* s)
{
	return EffectStatForBeingDrunk(s, s->bDexterity);
}


static UINT8 GetPenaltyForFatigue(const SOLDIERTYPE* s)
{
	UINT8 ubPercentPenalty;

	if      (s->bBreathMax >= 85) ubPercentPenalty =   0;
	else if (s->bBreathMax >= 70) ubPercentPenalty =  10;
	else if (s->bBreathMax >= 50) ubPercentPenalty =  25;
	else if (s->bBreathMax >= 30) ubPercentPenalty =  50;
	else if (s->bBreathMax >= 15) ubPercentPenalty =  75;
	else if (s->bBreathMax >   0) ubPercentPenalty =  90;
	else                          ubPercentPenalty = 100;

	return( ubPercentPenalty );
}


void ReducePointsForFatigue(const SOLDIERTYPE* s, UINT16* pusPoints)
{
	*pusPoints -= *pusPoints * GetPenaltyForFatigue(s) / 100;
}


INT32 GetSkillCheckPenaltyForFatigue( const SOLDIERTYPE *pSoldier, INT32 iSkill )
{
	// use only half the full effect of fatigue for skill checks
	return( ( (iSkill * GetPenaltyForFatigue( pSoldier ) ) / 100) / 2 );
}

INT32 SkillCheck( SOLDIERTYPE * pSoldier, INT8 bReason, INT8 bChanceMod )
{
	INT32   iSkill;
	INT32   iChance;
	INT32   iRoll, iMadeItBy;
	INT8    bSlot;
	BOOLEAN fForceDamnSound = FALSE;

	switch (bReason)
	{
		case LOCKPICKING_CHECK:
		case ELECTRONIC_LOCKPICKING_CHECK:

			fForceDamnSound = TRUE;

			iSkill = EffectiveMechanical( pSoldier );
			if (iSkill == 0)
			{
				break;
			}
			// adjust skill based on wisdom (knowledge)
			iSkill = iSkill * (EffectiveWisdom( pSoldier ) + 100) / 200;
			// and dexterity (clumsy?)
			iSkill = iSkill * (EffectiveDexterity( pSoldier ) + 100) / 200;
			// factor in experience
			iSkill = iSkill + EffectiveExpLevel( pSoldier ) * 3;
			// if we specialize in picking locks...
			iSkill += gbSkillTraitBonus[LOCKPICKING] * NUM_SKILL_TRAITS(pSoldier, LOCKPICKING);
			if (bReason == ELECTRONIC_LOCKPICKING_CHECK && !(HAS_SKILL_TRAIT( pSoldier, ELECTRONICS)) )
			{
				// if we are unfamiliar with electronics...
				iSkill /= 2;
			}
			// adjust chance based on status of kit
			bSlot = FindObj( pSoldier, LOCKSMITHKIT );
			if (bSlot == NO_SLOT)
			{
				// this should never happen, but might as well check...
				iSkill = 0;
			}
			else
			{
				iSkill = iSkill * pSoldier->inv[bSlot].bStatus[0] / 100;
			}
			break;
		case ATTACHING_DETONATOR_CHECK:
		case ATTACHING_REMOTE_DETONATOR_CHECK:
			iSkill = EffectiveExplosive( pSoldier );
			if (iSkill == 0)
			{
				break;
			}
			iSkill = (iSkill * 3 + EffectiveDexterity( pSoldier ) ) / 4;
			if ( bReason == ATTACHING_REMOTE_DETONATOR_CHECK && !(HAS_SKILL_TRAIT( pSoldier, ELECTRONICS )) )
			{
				iSkill /= 2;
			}
			break;
		case PLANTING_BOMB_CHECK:
		case PLANTING_REMOTE_BOMB_CHECK:
			iSkill = EffectiveExplosive( pSoldier ) * 7;
			iSkill += EffectiveWisdom( pSoldier ) * 2;
			iSkill += EffectiveExpLevel( pSoldier ) * 10;
			iSkill = iSkill / 10; // bring the value down to a percentage

			if ( bReason == PLANTING_REMOTE_BOMB_CHECK && !(HAS_SKILL_TRAIT( pSoldier, ELECTRONICS)) )
			{
				// deduct only a bit...
				iSkill = (iSkill * 3) / 4;
			}

			// Ok, this is really damn easy, so skew the values...
			// e.g. if calculated skill is 84, skewed up to 96
			// 51 to 84
			// 22 stays as is
			iSkill = (iSkill + 100 * (iSkill / 25) ) / (iSkill / 25 + 1);
			break;

		case DISARM_TRAP_CHECK:

			fForceDamnSound = TRUE;

			iSkill = EffectiveExplosive( pSoldier ) * 7;
			if ( iSkill == 0 )
			{
				break;
			}
			iSkill += EffectiveDexterity( pSoldier ) * 2;
			iSkill += EffectiveExpLevel( pSoldier ) * 10;
			iSkill = iSkill / 10; // bring the value down to a percentage
			// penalty based on poor wisdom
			iSkill -= (100 - EffectiveWisdom( pSoldier ) ) / 5;
			break;

		case DISARM_ELECTRONIC_TRAP_CHECK:

			fForceDamnSound = TRUE;

			iSkill = std::max( EffectiveMechanical( pSoldier ) , EffectiveExplosive( pSoldier ) ) * 7;
			if ( iSkill == 0 )
			{
				break;
			}
			iSkill += EffectiveDexterity( pSoldier ) * 2;
			iSkill += EffectiveExpLevel( pSoldier ) * 10;
			iSkill = iSkill / 10; // bring the value down to a percentage
			// penalty based on poor wisdom
			iSkill -= (100 - EffectiveWisdom( pSoldier ) ) / 5;

			if ( !(HAS_SKILL_TRAIT( pSoldier, ELECTRONICS )) )
			{
				iSkill = (iSkill * 3) / 4;
			}
			break;

		case OPEN_WITH_CROWBAR:
			// Add for crowbar...
			iSkill = EffectiveStrength( pSoldier ) + 20;
			fForceDamnSound = TRUE;
			break;

		case SMASH_DOOR_CHECK:
			iSkill = EffectiveStrength( pSoldier );
			break;
		case UNJAM_GUN_CHECK:
			iSkill = 30 + EffectiveMechanical( pSoldier ) / 2;
			break;
		case NOTICE_DART_CHECK:
			// only a max of ~20% chance
			iSkill = EffectiveWisdom( pSoldier ) / 10 + EffectiveExpLevel( pSoldier );
			break;
		case LIE_TO_QUEEN_CHECK:
			// competitive check vs the queen's wisdom and leadership... poor guy!
			iSkill = 50 * ( EffectiveWisdom( pSoldier ) + EffectiveLeadership( pSoldier ) ) / ( gMercProfiles[ QUEEN ].bWisdom + gMercProfiles[ QUEEN ].bLeadership );
			break;
		case ATTACHING_SPECIAL_ITEM_CHECK:
		case ATTACHING_SPECIAL_ELECTRONIC_ITEM_CHECK:
			iSkill = EffectiveMechanical( pSoldier );
			if (iSkill == 0)
			{
				break;
			}
			// adjust skill based on wisdom (knowledge)
			iSkill = iSkill * (EffectiveWisdom( pSoldier ) + 100) / 200;
			// and dexterity (clumsy?)
			iSkill = iSkill * (EffectiveDexterity( pSoldier ) + 100) / 200;
			// factor in experience
			iSkill = iSkill + EffectiveExpLevel( pSoldier ) * 3;
			if (bReason == ATTACHING_SPECIAL_ELECTRONIC_ITEM_CHECK && !(HAS_SKILL_TRAIT( pSoldier, ELECTRONICS)) )
			{
				// if we are unfamiliar with electronics...
				iSkill /= 2;
			}
			break;
		default:
			iSkill = 0;
			break;
	}

	iSkill -= GetSkillCheckPenaltyForFatigue( pSoldier, iSkill );

	iChance = iSkill + bChanceMod;

	switch (bReason)
	{
		case LOCKPICKING_CHECK:
		case ELECTRONIC_LOCKPICKING_CHECK:
		case DISARM_TRAP_CHECK:
		case DISARM_ELECTRONIC_TRAP_CHECK:
		case OPEN_WITH_CROWBAR:
		case SMASH_DOOR_CHECK:
		case ATTACHING_SPECIAL_ITEM_CHECK:
		case ATTACHING_SPECIAL_ELECTRONIC_ITEM_CHECK:
			// for lockpicking and smashing locks, if the chance isn't reasonable
			// we set it to 0 so they can never get through the door if they aren't
			// good enough
			if (iChance < 30)
			{
				iChance = 0;
				break;
			}
			// else fall through
		default:
			iChance += GetMoraleModifier( pSoldier );
			break;
	}

	if (iChance > 99)
	{
		iChance = 99;
	}
	else if (iChance < 0)
	{
		iChance = 0;
	}

	iRoll = PreRandom( 100 );
	iMadeItBy = iChance - iRoll;
	if (iMadeItBy < 0)
	{
		if ( (pSoldier->bLastSkillCheck == bReason) && (pSoldier->sGridNo == pSoldier->sSkillCheckGridNo) )
		{
			pSoldier->ubSkillCheckAttempts++;
			if (pSoldier->ubSkillCheckAttempts > 2)
			{
				if (iChance == 0)
				{
					// do we realize that we just can't do this?
					if ( (100 - (pSoldier->ubSkillCheckAttempts - 2) * 20) < EffectiveWisdom( pSoldier ) )
					{
						// say "I can't do this" quote
						TacticalCharacterDialogue( pSoldier, QUOTE_DEFINITE_CANT_DO );
						return( iMadeItBy );
					}
				}
			}
		}
		else
		{
			pSoldier->bLastSkillCheck = bReason;
			pSoldier->ubSkillCheckAttempts = 1;
			pSoldier->sSkillCheckGridNo = pSoldier->sGridNo;
		}

		if ( fForceDamnSound || Random( 100 ) < 40 )
		{
			switch( bReason )
			{
				case UNJAM_GUN_CHECK:
				case NOTICE_DART_CHECK:
				case LIE_TO_QUEEN_CHECK:
					// silent check
					break;
				default:
					DoMercBattleSound( pSoldier, BATTLE_SOUND_CURSE1 );
					break;
			}
		}

	}
	else
	{
		// A buddy might make a positive comment based on our success;
		// Increase the chance for people with higher skill and for more difficult tasks
		iChance = 15 + iSkill / 20 + (-bChanceMod) / 20;
		if (iRoll < iChance)
		{
			// If a buddy of this merc is standing around nearby, they'll make a positive comment.
			FOR_EACH_IN_TEAM(s, OUR_TEAM)
			{
				if (!OkControllableMerc(s)) continue;

				const BuddySlot bBuddyIndex = WhichBuddy(s->ubProfile, pSoldier->ubProfile);
				if (bBuddyIndex != BUDDY_NOT_FOUND && SpacesAway(pSoldier->sGridNo, s->sGridNo) < 15)
				{
					switch (bBuddyIndex)
					{
						case BUDDY_SLOT1:
							// buddy #1 did something good!
							TacticalCharacterDialogue(s, QUOTE_BUDDY_1_GOOD);
							break;
						case BUDDY_SLOT2:
							// buddy #2 did something good!
							TacticalCharacterDialogue(s, QUOTE_BUDDY_2_GOOD);
							break;
						case LEARNED_TO_LIKE_SLOT:
							// learn to like buddy did something good!
							TacticalCharacterDialogue(s, QUOTE_LEARNED_TO_LIKE_WITNESSED);
							break;
						default:
							break;
					}
				}
			}
		}
	}
	return( iMadeItBy );
}


INT8 CalcTrapDetectLevel( const SOLDIERTYPE * pSoldier, BOOLEAN fExamining )
{
	// return the level of trap which the guy is able to detect

	INT8 bDetectLevel;

	// formula: 1 pt for every exp_level
	//     plus 1 pt for every 40 explosives
	//     less 1 pt for every 20 wisdom MISSING

	bDetectLevel = EffectiveExpLevel( pSoldier );
	bDetectLevel += (EffectiveExplosive( pSoldier ) / 40);
	bDetectLevel -= ((100 - EffectiveWisdom( pSoldier ) ) / 20);

	// if the examining flag is true, this isn't just a casual glance
	// and the merc should have a higher chance
	if (fExamining)
	{
		bDetectLevel += (INT8) PreRandom(bDetectLevel / 3 + 2);
	}

	// if substantially bleeding, or still in serious shock, randomly lower value
	if ((pSoldier->bBleeding > 20) || (pSoldier->bShock > 1))
	{
		bDetectLevel -= (INT8) PreRandom(3);
	}

	if (bDetectLevel < 1)
	{
		bDetectLevel = 1;
	}

	return( bDetectLevel );
}
