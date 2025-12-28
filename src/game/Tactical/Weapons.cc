#include "ArmourModel.h"
#include "Directories.h"
#include "Font_Control.h"
#include "Handle_Items.h"
#include "Overhead_Types.h"
#include "SGPStrings.h"
#include "Soldier_Control.h"
#include "Overhead.h"
#include "Event_Pump.h"
#include "Soldier_Functions.h"
#include "Structure.h"
#include "TileDef.h"
#include "Timer_Control.h"
#include "Weapons.h"
#include "Animation_Control.h"
#include "Handle_UI.h"
#include "Isometric_Utils.h"
#include "WorldMan.h"
#include "Points.h"
#include "AI.h"
#include "LOS.h"
#include "RenderWorld.h"
#include "OppList.h"
#include "Interface.h"
#include "Message.h"
#include "Campaign.h"
#include "Items.h"
#include "Text.h"
#include "Soldier_Profile.h"
#include "Tile_Animation.h"
#include "Dialogue_Control.h"
#include "SkillCheck.h"
#include "Explosion_Control.h"
#include "Physics.h"
#include "Random.h"
#include "Vehicles.h"
#include "Bullets.h"
#include "Morale.h"
#include "Meanwhile.h"
#include "GameSettings.h"
#include "SaveLoadMap.h"
#include "Soldier_Macros.h"
#include "SmokeEffects.h"
#include "Auto_Resolve.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "WeaponModels.h"
#include "Logger.h"
#include "GamePolicy.h"

// NB this is arbitrary, chances in DG ranged from 1 in 6 to 1 in 20
#define BASIC_DEPRECIATE_CHANCE	15

#define NORMAL_RANGE		90 // # world units considered an 'avg' shot
#define MIN_SCOPE_RANGE	60 // # world units after which scope's useful

#define MIN_TANK_RANGE		120 // range at which tank starts really having trouble aiming

// percent reduction in sight range per point of aiming
#define SNIPERSCOPE_AIM_BONUS	gamepolicy(aim_bonus_sniperscope)
// bonus to hit with working laser scope
#define LASERSCOPE_BONUS	gamepolicy(aim_bonus_laserscope)

#define CRITICAL_HIT_THRESHOLD	30

#define HTH_MODE_PUNCH		1
#define HTH_MODE_STAB		2
#define HTH_MODE_STEAL		3

// JA2 GOLD: for weapons and attachments, give penalties only for status values below 85
#define WEAPON_STATUS_MOD( x )	( (x) >= 85 ? 100 : (((x) * 100) / 85) )

BOOLEAN gfNextFireJam      = FALSE;
BOOLEAN gfNextShotKills    = FALSE;

//GLOBALS

// the amount of momentum reduction for the head, torso, and legs
// used to determine whether the bullet will go through someone
static const UINT8 BodyImpactReduction[4] = { 0, 15, 30, 23 };


UINT16 GunRange(OBJECTTYPE const& o)
{
	// return a minimal value of 1 tile
	if (!(GCM->getItem(o.usItem)->isWeapon())) return CELL_X_SIZE;

	UINT16 range = GCM->getWeapon(o.usItem)->usRange;
	INT8 attach_pos = FindAttachment(&o, GUN_BARREL_EXTENDER);
	if (attach_pos != ITEM_NOT_FOUND)
	{
		range += GUN_BARREL_RANGE_BONUS * WEAPON_STATUS_MOD(o.bAttachStatus[attach_pos]) / 100;
	}
	attach_pos = FindAttachment(&o, SILENCER);
	if (attach_pos != ITEM_NOT_FOUND)
	{
		range -= SILENCER_RANGE_PENALTY;
	}
	return range;
}


INT8 EffectiveArmour(OBJECTTYPE const* const o)
{
	if (!o) return 0;

	const ItemModel * item = GCM->getItem(o->usItem);
	if (!item->isArmour()) return 0;

	INT32 armour_val = item->asArmour()->getProtection() * o->bStatus[0] / 100;
	INT8  const plate_pos  = FindPlatesAttachment(o);
	if (plate_pos != ITEM_NOT_FOUND)
	{
		armour_val += GCM->getItem(o->usAttachItem[plate_pos])->asArmour()->getProtection() * o->bAttachStatus[plate_pos] / 100;
	}
	return armour_val;
}


INT8 ArmourPercent(const SOLDIERTYPE* pSoldier)
{
	INT32 iVest = 0, iHelmet = 0, iLeg = 0;

	if (pSoldier->inv[VESTPOS].usItem)
	{
		iVest = EffectiveArmour( &(pSoldier->inv[VESTPOS]) );
		// convert to % of best; ignoring bug-treated stuff
		iVest = 65 * iVest / GCM->getMaxArmourPerClass(ARMOURCLASS_VEST);
	}

	if (pSoldier->inv[HELMETPOS].usItem)
	{
		iHelmet = EffectiveArmour( &(pSoldier->inv[HELMETPOS]) );
		// convert to % of best; ignoring bug-treated stuff
		iHelmet = 15 * iHelmet / GCM->getMaxArmourPerClass(ARMOURCLASS_HELMET);
	}

	if (pSoldier->inv[LEGPOS].usItem)
	{
		iLeg = EffectiveArmour( &(pSoldier->inv[LEGPOS]) );
		// convert to % of best; ignoring bug-treated stuff
		iLeg = 25 * iLeg / GCM->getMaxArmourPerClass(ARMOURCLASS_LEGGINGS);
	}

	return( (INT8) (iHelmet + iVest + iLeg) );
}


INT8 ExplosiveEffectiveArmour(const OBJECTTYPE* pObj)
{
	INT32 iValue;
	INT8  bPlate;

	if (pObj == NULL || !GCM->getItem(pObj->usItem)->isArmour())
	{
		return( 0 );
	}
	iValue = GCM->getItem(pObj->usItem)->asArmour()->getExplosivesProtection();
	iValue = iValue * pObj->bStatus[0] / 100;

	bPlate = FindPlatesAttachment(pObj);
	if ( bPlate != ITEM_NOT_FOUND )
	{
		INT32 iValue2;

		iValue2 = GCM->getItem(pObj->usAttachItem[bPlate])->asArmour()->getProtection();
		iValue2 = iValue2 * pObj->bAttachStatus[ bPlate ] / 100;

		iValue += iValue2;
	}
	return( (INT8) iValue );
}

INT8 ArmourVersusExplosivesPercent( SOLDIERTYPE * pSoldier )
{
	// returns the % damage reduction from grenades
	INT32 iVest = 0, iHelmet = 0, iLeg = 0;

	if (pSoldier->inv[VESTPOS].usItem)
	{
		iVest = ExplosiveEffectiveArmour( &(pSoldier->inv[VESTPOS]) );
		// convert to % of best; ignoring bug-treated stuff
		iVest = std::min(65, 65 * iVest / GCM->getMaxArmourPerClass(ARMOURCLASS_VEST));
	}

	if (pSoldier->inv[HELMETPOS].usItem)
	{
		iHelmet = ExplosiveEffectiveArmour( &(pSoldier->inv[HELMETPOS]) );
		// convert to % of best; ignoring bug-treated stuff
		iHelmet = std::min(15, 15 * iHelmet / GCM->getMaxArmourPerClass(ARMOURCLASS_HELMET));
	}

	if (pSoldier->inv[LEGPOS].usItem)
	{
		iLeg = ExplosiveEffectiveArmour( &(pSoldier->inv[LEGPOS]) );
		// convert to % of best; ignoring bug-treated stuff
		iLeg = std::min(25, 25 * iLeg / GCM->getMaxArmourPerClass(ARMOURCLASS_LEGGINGS));
	}

	return( (INT8) (iHelmet + iVest + iLeg) );
}


static void AdjustImpactByHitLocation(INT32 iImpact, UINT8 ubHitLocation, INT32* piNewImpact, INT32* piImpactForCrits)
{
	switch( ubHitLocation )
	{
		case AIM_SHOT_HEAD:
		{
			// vanilla was: 1.5x damage from successful hits to the head!
			float critical_damage_to_head = gamepolicy(critical_damage_head_multiplier);
			float impactForCrits = floorf(critical_damage_to_head * (float)iImpact);
			*piImpactForCrits = (INT32)impactForCrits;
			*piNewImpact = *piImpactForCrits;
			break;
		}
		case AIM_SHOT_LEGS:
		{
			// for vanilla:
			// half damage for determining critical hits
			// quarter actual damage
			float critical_damage_to_legs = gamepolicy(critical_damage_legs_multiplier);
			float impactForCrits = floorf(critical_damage_to_legs * (float)iImpact);
			float newImpact = floorf(critical_damage_to_legs * impactForCrits);
			// NOTE: to calculate new impact, multiplying by crit damage multiplier twice might be overkill
			// might be better to halve the multiplier:
			// float newImpact = floor(0.5f * critical_damage_to_legs * (float)iImpact);
			*piImpactForCrits = (INT32)impactForCrits;
			*piNewImpact = (INT32)newImpact;
			break;
		}
		default:
			*piImpactForCrits = iImpact;
			*piNewImpact = iImpact;
			break;
	}
}


// #define TESTGUNJAM

FireWeaponResult CheckForGunJam(SOLDIERTYPE * const pSoldier)
{
	// should jams apply to enemies?
	if (pSoldier->uiStatusFlags & SOLDIER_PC)
	{
		if ( GCM->getItem(pSoldier->usAttackingWeapon)->getItemClass() == IC_GUN && !EXPLOSIVE_GUN( pSoldier->usAttackingWeapon ) )
		{
			auto & obj = pSoldier->inv[pSoldier->ubAttackingHand];
			if (obj.bGunAmmoStatus > 0)
			{
				// gun might jam, figure out the chance
				INT32 iChance = 80 - obj.bGunStatus;

				// CJC: removed reliability from formula...

				// jams can happen to unreliable guns "earlier" than normal or reliable ones.
				//iChance = iChance - GCM->getItem(pObj->usItem)->getReliability() * 2;

				// decrease the chance of a jam by 20% per point of reliability;
				// increased by 20% per negative point...
				//iChance = iChance * (10 - GCM->getItem(pObj->usItem)->getReliability() * 2) / 10;

				if (pSoldier->bDoBurst > 1)
				{
					// if at bullet in a burst after the first, higher chance
					iChance -= PreRandom( 80 );
				}
				else
				{
					iChance -= PreRandom( 100 );
				}

#ifdef TESTGUNJAM
				if ( 1 )
#else
				if ((INT32) PreRandom( 100 ) < iChance || gfNextFireJam )
#endif
				{
					gfNextFireJam = FALSE;

					// jam! negate the gun ammo status.
					obj.bGunAmmoStatus *= -1;

					// Deduct AMMO!
					DeductAmmo( pSoldier, pSoldier->ubAttackingHand );

					TacticalCharacterDialogue( pSoldier, QUOTE_JAMMED_GUN );
					return FireWeaponResult::JAMMED;
				}
			}
			else if (obj.bGunAmmoStatus < 0)
			{
				// try to unjam gun
				if (SkillCheck(pSoldier, UNJAM_GUN_CHECK,
					GCM->getItem(obj.usItem)->getReliability() * 4) > 0)
				{
					// yay! unjammed the gun
					obj.bGunAmmoStatus *= -1;

					// MECHANICAL/DEXTERITY GAIN: Unjammed a gun
					StatChange(*pSoldier, MECHANAMT, 5, FROM_SUCCESS);
					StatChange(*pSoldier, DEXTAMT,   5, FROM_SUCCESS);

					DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );

					// We unjammed gun, return appropriate value!
					return FireWeaponResult::UNJAMMED;
				}
				else
				{
					return FireWeaponResult::JAMMED;
				}
			}
		}
	}
	return FireWeaponResult::FIREABLE;
}


FireWeaponResult OKFireWeapon(SOLDIERTYPE * const pSoldier)
{
	// 1) Are we attacking with our second hand?
	if ( pSoldier->ubAttackingHand == SECONDHANDPOS )
	{
		if ( !EnoughAmmo( pSoldier, FALSE, pSoldier->ubAttackingHand ) )
		{
			if (pSoldier->bTeam == OUR_TEAM)
			{
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, g_langRes->Message[ STR_2ND_CLIP_DEPLETED ] );
				return FireWeaponResult::FAILED;
			}
		}
	}

	return CheckForGunJam(pSoldier);
}


static void UseGun(     SOLDIERTYPE * pSoldier, GridNo sTargetGridNo);
static void UseBlade(   SOLDIERTYPE * pSoldier, GridNo sTargetGridNo);
static void UseThrown(  SOLDIERTYPE * pSoldier, GridNo sTargetGridNo);
static void UseLauncher(SOLDIERTYPE * pSoldier, GridNo sTargetGridNo);


FireWeaponResult FireWeapon(SOLDIERTYPE * const pSoldier, GridNo const sTargetGridNo)
{
	// if target gridno is the same as ours, do not fire!
	if (sTargetGridNo == pSoldier->sGridNo)
	{
		// FREE UP NPC!
		SLOGD("Freeing up attacker - attack on own gridno!");
		FreeUpAttacker(pSoldier);
		return FireWeaponResult::FAILED;
	}

	// SET ATTACKER TO NOBODY, WILL GET SET EVENTUALLY
	pSoldier->opponent = NULL;

	switch( GCM->getItem(pSoldier->usAttackingWeapon)->getItemClass() )
	{
		case IC_THROWING_KNIFE:
		case IC_GUN:

			// ATE: PAtch up - bookkeeping for spreading done out of whak
			if ( pSoldier->fDoSpread && !pSoldier->bDoBurst )
			{
				pSoldier->fDoSpread = FALSE;
			}

			if ( pSoldier->fDoSpread > 6 )
			{
				pSoldier->fDoSpread = FALSE;
			}


			if ( pSoldier->fDoSpread )
			{
				if ( pSoldier->sSpreadLocations[ pSoldier->fDoSpread - 1 ] != 0 )
				{
					UseGun( pSoldier, pSoldier->sSpreadLocations[ pSoldier->fDoSpread - 1 ] );
				}
				else
				{
					UseGun( pSoldier, sTargetGridNo );
				}
				pSoldier->fDoSpread++;
			}
			else
			{
				UseGun( pSoldier, sTargetGridNo );
			}
			break;
		case IC_BLADE:

			UseBlade( pSoldier, sTargetGridNo );
			break;
		case IC_PUNCH:
			UseHandToHand( pSoldier, sTargetGridNo, FALSE );
			break;

		case IC_LAUNCHER:
			UseLauncher( pSoldier, sTargetGridNo );
			break;

		default:
			// attempt to throw
			UseThrown( pSoldier, sTargetGridNo );
			break;
	}
	return FireWeaponResult::FIRED;
}


void GetTargetWorldPositions( SOLDIERTYPE *pSoldier, INT16 sTargetGridNo, FLOAT *pdXPos, FLOAT *pdYPos, FLOAT *pdZPos )
{
	FLOAT  dTargetX;
	FLOAT  dTargetY;
	FLOAT  dTargetZ;
	INT16  sXMapPos, sYMapPos;
	UINT32 uiRoll;

	SOLDIERTYPE* const pTargetSoldier = WhoIsThere2(sTargetGridNo, pSoldier->bTargetLevel);
	if ( pTargetSoldier )
	{
		pSoldier->opponent = pTargetSoldier;
		dTargetX = (FLOAT) CenterX( pTargetSoldier->sGridNo );
		dTargetY = (FLOAT) CenterY( pTargetSoldier->sGridNo );

		INT8 const bAimShotLocation = pSoldier->bAimShotLocation;

		if (pSoldier->bAimShotLocation == AIM_SHOT_RANDOM)
		{
			uiRoll = PreRandom( 100 );
			if (uiRoll < 15)
			{
				pSoldier->bAimShotLocation = AIM_SHOT_LEGS;
			}
			else if (uiRoll > 94)
			{
				pSoldier->bAimShotLocation = AIM_SHOT_HEAD;
			}
			else
			{
				pSoldier->bAimShotLocation = AIM_SHOT_TORSO;
			}
			if ( pSoldier->bAimShotLocation != AIM_SHOT_HEAD )
			{
				UINT32 uiChanceToGetThrough = SoldierToSoldierBodyPartChanceToGetThrough( pSoldier, pTargetSoldier, pSoldier->bAimShotLocation );

				if ( uiChanceToGetThrough < 25 )
				{
					if ( SoldierToSoldierBodyPartChanceToGetThrough( pSoldier, pTargetSoldier, AIM_SHOT_HEAD ) > uiChanceToGetThrough * 2 )
					{
						// try for a head shot then
						pSoldier->bAimShotLocation = AIM_SHOT_HEAD;
					}
				}
			}

		}

		if (gamepolicy(ai_better_aiming_choice) && bAimShotLocation == AIM_SHOT_RANDOM)
		{
			UINT32 const threshold_cth_head = gamepolicy(threshold_cth_head);
			UINT32 const threshold_cth_legs = gamepolicy(threshold_cth_legs);
			UINT32 const cth_aim_shot_head = SoldierToSoldierBodyPartChanceToGetThrough( pSoldier, pTargetSoldier, AIM_SHOT_HEAD );
			UINT32 const cth_aim_shot_torso = SoldierToSoldierBodyPartChanceToGetThrough( pSoldier, pTargetSoldier, AIM_SHOT_TORSO );
			UINT32 const cth_aim_shot_legs = SoldierToSoldierBodyPartChanceToGetThrough( pSoldier, pTargetSoldier, AIM_SHOT_LEGS );

			UINT32 cth_choice = cth_aim_shot_torso;
			pSoldier->bAimShotLocation = AIM_SHOT_TORSO; // default

			if (cth_aim_shot_legs >= threshold_cth_legs || (cth_aim_shot_legs + 5) > cth_choice)
			{
				pSoldier->bAimShotLocation = AIM_SHOT_LEGS;
				cth_choice = cth_aim_shot_legs;
			}

			if (cth_aim_shot_head >= threshold_cth_head ||   // good enough, override
				cth_aim_shot_head >= cth_choice) // close enough, better if extra damage
			{
				pSoldier->bAimShotLocation = AIM_SHOT_HEAD;
			}
		}

		switch( pSoldier->bAimShotLocation )
		{
			case AIM_SHOT_HEAD:
				CalculateSoldierZPos( pTargetSoldier, HEAD_TARGET_POS, &dTargetZ );
				break;
			case AIM_SHOT_TORSO:
				CalculateSoldierZPos( pTargetSoldier, TORSO_TARGET_POS, &dTargetZ );
				break;
			case AIM_SHOT_LEGS:
				CalculateSoldierZPos( pTargetSoldier, LEGS_TARGET_POS, &dTargetZ );
				break;
			default:
				// %)@#&(%?
				CalculateSoldierZPos( pTargetSoldier, TARGET_POS, &dTargetZ );
				break;
		}
	}
	else
	{

		// GET TARGET XY VALUES
		ConvertGridNoToCenterCellXY( sTargetGridNo, &sXMapPos, &sYMapPos );

		// fire at centre of tile
		dTargetX = (FLOAT) sXMapPos;
		dTargetY = (FLOAT) sYMapPos;
		if (pSoldier->bTargetCubeLevel)
		{
			// fire at the centre of the cube specified
			dTargetZ = ( (FLOAT) (pSoldier->bTargetCubeLevel + pSoldier->bTargetLevel * PROFILE_Z_SIZE) - 0.5f) * HEIGHT_UNITS_PER_INDEX;
		}
		else
		{
			INT8 bStructHeight = GetStructureTargetHeight(sTargetGridNo, pSoldier->bTargetLevel == 1);
			if (bStructHeight > 0)
			{
				// fire at the centre of the cube *one below* the tallest of the tallest structure
				if (bStructHeight > 1)
				{
					// reduce target level by 1
					bStructHeight--;
				}
				dTargetZ = ((FLOAT) (bStructHeight + pSoldier->bTargetLevel * PROFILE_Z_SIZE) - 0.5f) * HEIGHT_UNITS_PER_INDEX;
			}
			else
			{
				// fire at 1 unit above the level of the ground
				dTargetZ = (FLOAT) (pSoldier->bTargetLevel * PROFILE_Z_SIZE) * HEIGHT_UNITS_PER_INDEX + 1;
			}
		}
		// adjust for terrain height
		dTargetZ += CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[sTargetGridNo].sHeight );
	}

	*pdXPos = dTargetX;
	*pdYPos = dTargetY;
	*pdZPos = dTargetZ;
}


static UINT16 ModifyExpGainByTarget(const UINT16 exp_gain, const SOLDIERTYPE* const tgt)
{
	if (tgt->ubBodyType == COW || tgt->ubBodyType == CROW)
	{
		return exp_gain / 2;
	}
	else if (IsMechanical(*tgt) || TANK(tgt))
	{
		// no exp from shooting a vehicle that you can't damage and can't move!
		return 0;
	}
	return exp_gain;
}


static BOOLEAN WillExplosiveWeaponFail(const SOLDIERTYPE* pSoldier, const OBJECTTYPE* pObj);


static ST::string GetBurstSoundName(SOLDIERTYPE const& soldier)
{
	auto * const weapon = GCM->getWeapon(soldier.usAttackingWeapon);
	bool isSilenced = FindAttachment(&soldier.inv[soldier.ubAttackingHand], SILENCER) != NO_SLOT;
	auto const& burstSound = isSilenced ? weapon->silencedBurstSound : weapon->burstSound;

	if (!burstSound.empty())
	{
		// Was a burst sound specified and do we have the matching sound file?
		auto tryBurstSound = st_format_printf(burstSound, soldier.bBulletsLeft);
		if (GCM->doesGameResExists(tryBurstSound))
		{
			return tryBurstSound;
		}
	}

	if (soldier.bBulletsLeft == 1)
	{
		// No burst sound found for 1-shot bursts, see if we can find a
		// file for a single shot.
		auto const& singleSound = isSilenced ? weapon->silencedSound : weapon->sound;
		if (!singleSound.empty() && GCM->doesGameResExists(singleSound))
		{
			return singleSound;
		}
	}

	// Everything else failed, use the generic burst sound.
	return "sounds/weapons/bursttype1.wav";
}

static void UseGun(SOLDIERTYPE * const pSoldier, GridNo const sTargetGridNo)
{
	UINT32  uiHitChance, uiDiceRoll;
	INT16   sAPCost;
	FLOAT   dTargetX;
	FLOAT   dTargetY;
	FLOAT   dTargetZ;
	UINT16  usItemNum;
	BOOLEAN fBuckshot;
	UINT8   ubVolume;
	INT8    bSilencerPos;
	UINT8   ubDirection;
	INT16   sNewGridNo;
	UINT16  usExpGain = 0;
	UINT32  uiDepreciateTest;

	// Deduct points!
	sAPCost = CalcTotalAPsToAttack( pSoldier, sTargetGridNo, FALSE, pSoldier->bAimTime );

	usItemNum = pSoldier->usAttackingWeapon;

	if ( pSoldier->bDoBurst )
	{
		// ONly deduct points once
		if ( pSoldier->bDoBurst == 1 )
		{
			auto burstSound = GetBurstSoundName(*pSoldier);
			SLOGD("Using burst sound {}", burstSound);
			pSoldier->uiBurstSoundID = PlayLocationJA2SampleFromFile(pSoldier->sGridNo,
				burstSound.c_str(), HIGHVOLUME, 1);

			DeductPoints( pSoldier, sAPCost, 0 );
		}

	}
	else
	{
		// ONLY DEDUCT FOR THE FIRST HAND when doing two-pistol attacks
		if ( IsValidSecondHandShot( pSoldier ) && pSoldier->inv[ HANDPOS ].bGunStatus >= USABLE && pSoldier->inv[HANDPOS].bGunAmmoStatus > 0 )
		{
			// only deduct APs when the main gun fires
			if ( pSoldier->ubAttackingHand == HANDPOS )
			{
				DeductPoints( pSoldier, sAPCost, 0 );
			}
		}
		else
		{
			DeductPoints( pSoldier, sAPCost, 0 );
		}

		auto weapon = GCM->getWeapon( usItemNum );
		//PLAY SOUND
		// ( For throwing knife.. it's earlier in the animation
		if ( GCM->getItem(usItemNum)->getItemClass() != IC_THROWING_KNIFE )
		{
			// Switch on silencer...
			if( FindAttachment( &( pSoldier->inv[ pSoldier->ubAttackingHand ] ), SILENCER ) != NO_SLOT )
			{
				if (!weapon->silencedSound.empty()) {
					PlayLocationJA2Sample(pSoldier->sGridNo, weapon->silencedSound, HIGHVOLUME, 1);
				}

			}
			else
			{
				if (!weapon->sound.empty()) {
					PlayLocationJA2Sample(pSoldier->sGridNo, weapon->sound, HIGHVOLUME, 1);
				}
			}
		}
	}


	// CALC CHANCE TO HIT
	if ( GCM->getItem(usItemNum)->getItemClass() == IC_THROWING_KNIFE )
	{
		uiHitChance = CalcThrownChanceToHit( pSoldier, sTargetGridNo, pSoldier->bAimTime, pSoldier->bAimShotLocation );
	}
	else
	{
		uiHitChance = CalcChanceToHitGun( pSoldier, sTargetGridNo, pSoldier->bAimTime, pSoldier->bAimShotLocation, true );
	}

	//ATE: Added if we are in meanwhile, we always hit...
	if ( AreInMeanwhile( ) )
	{
		uiHitChance = 100;
	}

	// ROLL DICE
	uiDiceRoll = PreRandom( 100 );

	bool const fGonnaHit = uiDiceRoll <= uiHitChance;

	// ATE; Moved a whole blotch if logic code for finding target positions to a function
	// so other places can use it
	GetTargetWorldPositions( pSoldier, sTargetGridNo, &dTargetX, &dTargetY, &dTargetZ );

	// Some things we don't do for knives...
	if ( GCM->getItem(usItemNum)->getItemClass() != IC_THROWING_KNIFE )
	{
		// Deduct AMMO!
		DeductAmmo( pSoldier, pSoldier->ubAttackingHand );

		// ATE: Check if we should say quote...
		if ( pSoldier->inv[ pSoldier->ubAttackingHand ].ubGunShotsLeft == 0 && pSoldier->usAttackingWeapon != ROCKET_LAUNCHER )
		{
			if ( pSoldier->bTeam == OUR_TEAM )
			{
				pSoldier->fSayAmmoQuotePending = TRUE;
			}
		}

		// NB bDoBurst will be 2 at this point for the first shot since it was incremented
		// above
		if (IsOnOurTeam(*pSoldier) &&
			pSoldier->target != NULL  &&
			(!pSoldier->bDoBurst || pSoldier->bDoBurst == 2) &&
			gTacticalStatus.uiFlags & INCOMBAT)
		{
			const SOLDIERTYPE* const tgt = pSoldier->target;
			if (SoldierToSoldierBodyPartChanceToGetThrough(pSoldier, tgt, pSoldier->bAimShotLocation) > 0)
			{
				if ( fGonnaHit )
				{
					// grant extra exp for hitting a difficult target
					usExpGain += (UINT8) (100 - uiHitChance) / 25;

					if ( pSoldier->bAimTime && !pSoldier->bDoBurst )
					{
						// gain extra exp for aiming, up to the amount from
						// the difficulty of the shot
						usExpGain += std::min(int(pSoldier->bAimTime), int(usExpGain));
					}

					// base pts extra for hitting
					usExpGain	+= 3;
				}

				// add base pts for taking a shot, whether it hits or misses
				usExpGain += 3;

				if ( IsValidSecondHandShot( pSoldier ) && pSoldier->inv[ HANDPOS ].bGunStatus >= USABLE && pSoldier->inv[HANDPOS].bGunAmmoStatus > 0 )
				{
					// reduce exp gain for two pistol shooting since both shots give xp
					usExpGain = (usExpGain * 2) / 3;
				}

				usExpGain = ModifyExpGainByTarget(usExpGain, tgt);

				// MARKSMANSHIP GAIN: gun attack
				StatChange(*pSoldier, MARKAMT, usExpGain, fGonnaHit ? FROM_SUCCESS : FROM_FAILURE);
			}
		}

		// set buckshot and muzzle flash
		fBuckshot = FALSE;
		if (!CREATURE_OR_BLOODCAT( pSoldier ) )
		{
			pSoldier->fMuzzleFlash = TRUE;
			switch ( pSoldier->inv[ pSoldier->ubAttackingHand ].ubGunAmmoType )
			{
				case AMMO_BUCKSHOT:
					fBuckshot = TRUE;
					break;
				case AMMO_SLEEP_DART:
					pSoldier->fMuzzleFlash = FALSE;
					break;
				default:
					break;
			}
		}
	}
	else //  throwing knife
	{
		fBuckshot = FALSE;
		pSoldier->fMuzzleFlash = FALSE;

		// Deduct knife from inv! (not here, later?)

		// Improve for using a throwing knife....
		if (IsOnOurTeam(*pSoldier) && pSoldier->target != NULL)
		{
			if ( fGonnaHit )
			{
				// grant extra exp for hitting a difficult target
				usExpGain += (UINT8) (100 - uiHitChance) / 10;

				if (pSoldier->bAimTime)
				{
					// gain extra exp for aiming, up to the amount from
					// the difficulty of the throw
					usExpGain += 2 * std::min(int(pSoldier->bAimTime), int(usExpGain));
				}

				// base pts extra for hitting
				usExpGain	+= 10;
			}

			// add base pts for taking a shot, whether it hits or misses
			usExpGain += 10;

			usExpGain = ModifyExpGainByTarget(usExpGain, pSoldier->target);

			// MARKSMANSHIP/DEXTERITY GAIN: throwing knife attack
			StatChangeCause const cause = fGonnaHit ? FROM_SUCCESS : FROM_FAILURE;
			StatChange(*pSoldier, MARKAMT, usExpGain / 2, cause);
			StatChange(*pSoldier, DEXTAMT, usExpGain / 2, cause);
		}
	}

	if ( usItemNum == ROCKET_LAUNCHER)
	{
		if ( WillExplosiveWeaponFail( pSoldier, &( pSoldier->inv[ HANDPOS ] ) ) )
		{
			CreateItem( DISCARDED_LAW, pSoldier->inv[ HANDPOS ].bStatus[ 0 ], &(pSoldier->inv[ HANDPOS ] ) );
			DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );

			IgniteExplosion(pSoldier, 0, pSoldier->sGridNo, C1, pSoldier->bLevel);

			// Reduce again for attack end 'cause it has been incremented for a normal attack
			//
			SLOGD("Freeing up attacker - ATTACK ANIMATION {} ENDED BY BAD EXPLOSIVE CHECK, Now {}",
				gAnimControl[pSoldier->usAnimState].zAnimStr, gTacticalStatus.ubAttackBusyCount);
			ReduceAttackBusyCount(pSoldier, FALSE);
		}
	}

	FireBulletGivenTarget(pSoldier, dTargetX, dTargetY, dTargetZ, pSoldier->usAttackingWeapon,
				(INT16) (uiHitChance - uiDiceRoll), fBuckshot, FALSE);

	ubVolume = GCM->getWeapon( pSoldier->usAttackingWeapon )->ubAttackVolume;

	if ( GCM->getItem(usItemNum)->getItemClass() == IC_THROWING_KNIFE )
	{
		// Here, remove the knife...	or (for now) rocket launcher
		RemoveObjs( &(pSoldier->inv[ HANDPOS ] ), 1 );
		if(pSoldier->inv[HANDPOS].usItem == NOTHING)
		{
			INT8 slot=FindObj(pSoldier, BLOODY_THROWING_KNIFE);
			if(slot==NO_SLOT) slot=FindObj(pSoldier, THROWING_KNIFE);
			if(slot!=NO_SLOT)
			{
				OBJECTTYPE *item=&pSoldier->inv[slot];
				CreateItem(item->usItem, item->bStatus[0], &pSoldier->inv[HANDPOS]);
				RemoveObjs(item, 1);
			}
		}
		DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
	}
	else if ( usItemNum == ROCKET_LAUNCHER)
	{
		CreateItem( DISCARDED_LAW, pSoldier->inv[ HANDPOS ].bStatus[ 0 ], &(pSoldier->inv[ HANDPOS ] ) );
		DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );

		// Direction to center of explosion
		ubDirection = OppositeDirection(pSoldier->bDirection);
		sNewGridNo  = NewGridNo( (UINT16)pSoldier->sGridNo, DirectionInc( ubDirection ) );

		// Check if a person exists here and is not prone....
		SOLDIERTYPE* const tgt = WhoIsThere2(sNewGridNo, pSoldier->bLevel);
		if (tgt != NULL)
		{
			if (gAnimControl[tgt->usAnimState].ubHeight != ANIM_PRONE)
			{
				// Increment attack counter...
				gTacticalStatus.ubAttackBusyCount++;
				SLOGD("Incrementing Attack: Exhaust from LAW ({})", gTacticalStatus.ubAttackBusyCount);
				EVENT_SoldierGotHit(tgt, MINI_GRENADE, 10, 200, pSoldier->bDirection, 0, pSoldier, 0, ANIM_CROUCH, sNewGridNo);
			}
		}
	}
	else
	{
		// if the weapon has a silencer attached
		bSilencerPos = FindAttachment( &(pSoldier->inv[HANDPOS]), SILENCER );
		if (bSilencerPos != -1)
		{
			// reduce volume by a percentage equal to silencer's work %age (min 1)
			ubVolume = 1 + ((100 - WEAPON_STATUS_MOD(pSoldier->inv[HANDPOS].bAttachStatus[bSilencerPos])) / (100 / (ubVolume - 1)));
		}
	}

	MakeNoise(pSoldier, pSoldier->sGridNo, pSoldier->bLevel, ubVolume, NOISE_GUNFIRE);

	if ( pSoldier->bDoBurst )
	{
		// done, if bursting, increment
		pSoldier->bDoBurst++;
	}

	// CJC: since jamming is no longer affected by reliability, increase chance of status going down for really unreliabile guns
	uiDepreciateTest = BASIC_DEPRECIATE_CHANCE + 3 * GCM->getItem(usItemNum)->getReliability();

	if ( !PreRandom( uiDepreciateTest ) && ( pSoldier->inv[ pSoldier->ubAttackingHand ].bStatus[0] > 1) )
	{
		pSoldier->inv[ pSoldier->ubAttackingHand ].bStatus[ 0 ]--;
	}

	// reduce monster smell (gunpowder smell)
	if ( pSoldier->bMonsterSmell > 0 && Random( 2 ) == 0 )
	{
		pSoldier->bMonsterSmell--;
	}
}


static void AgilityForEnemyMissingPlayer(const SOLDIERTYPE* const attacker, SOLDIERTYPE* const target, const UINT agil_amt)
{
	// if it was another team attacking someone under our control
	if (target->bTeam != attacker->bTeam &&
			target->bTeam == OUR_TEAM && target->bLife >= OKLIFE)
	{
		StatChange(*target, AGILAMT, agil_amt, FROM_SUCCESS);
	}
}


static void UseBlade(SOLDIERTYPE * const pSoldier, GridNo const sTargetGridNo)
{
	INT32          iHitChance, iDiceRoll;
	INT16          sAPCost;
	INT32          iImpact, iImpactForCrits;
	BOOLEAN        fGonnaHit = FALSE;
	UINT16         usExpGain = 0;
	int            bMaxDrop;
	BOOLEAN        fSurpriseAttack;

	// Deduct points!
	sAPCost = CalcTotalAPsToAttack( pSoldier, sTargetGridNo, FALSE, pSoldier->bAimTime );

	DeductPoints( pSoldier, sAPCost, 0 );

	// See if a guy is here!
	SOLDIERTYPE* const pTargetSoldier = WhoIsThere2(sTargetGridNo, pSoldier->bTargetLevel);
	if ( pTargetSoldier )
	{
		// set target as noticed attack
		pSoldier->uiStatusFlags |= SOLDIER_ATTACK_NOTICED;
		pTargetSoldier->fIntendedTarget = TRUE;

		pSoldier->opponent = pTargetSoldier;

		// CHECK IF BUDDY KNOWS ABOUT US
		if ( pTargetSoldier->bOppList[ pSoldier->ubID ] == NOT_HEARD_OR_SEEN ||
			pTargetSoldier->bLife < OKLIFE || pTargetSoldier->bCollapsed )
		{
			iHitChance = 100;
			fSurpriseAttack = TRUE;
		}
		else
		{
			iHitChance = CalcChanceToStab( pSoldier, pTargetSoldier, pSoldier->bAimTime );
			fSurpriseAttack = FALSE;
		}

		// ROLL DICE
		iDiceRoll = (INT32) PreRandom( 100 );

		if ( iDiceRoll <= iHitChance )
		{
			fGonnaHit = TRUE;

			// CALCULATE DAMAGE!
			// attack HITS, calculate damage (base damage is 1-maximum knife sImpact)
			iImpact = HTHImpact( pSoldier, pTargetSoldier, (iHitChance - iDiceRoll), TRUE );

			// modify this by the knife's condition (if it's dull, not much good)
			iImpact = ( iImpact * WEAPON_STATUS_MOD(pSoldier->inv[pSoldier->ubAttackingHand].bStatus[0]) ) / 100;

			// modify by hit location
			AdjustImpactByHitLocation( iImpact, pSoldier->bAimShotLocation, &iImpact, &iImpactForCrits );

			// bonus for surprise
			if ( fSurpriseAttack )
			{
				iImpact = (iImpact * 3) / 2;
			}

			// any successful hit does at LEAST 1 pt minimum damage
			if (iImpact < 1)
			{
				iImpact = 1;
			}

			if ( pSoldier->inv[ pSoldier->ubAttackingHand ].bStatus[ 0 ] > USABLE )
			{
				bMaxDrop = (iImpact / 20);
				int adjStatus = pSoldier->inv[pSoldier->ubAttackingHand].bStatus[0] / 10;

				// the duller they get, the slower they get any worse...
				// as long as its still > USABLE, it drops another point 1/2 the time
				bMaxDrop = adjStatus > 2 ? std::clamp(bMaxDrop, 2, adjStatus) : 2;

				pSoldier->inv[ pSoldier->ubAttackingHand ].bStatus[ 0 ] -= (INT8) Random( bMaxDrop );     // 0 to (maxDrop - 1)
			}

			// Send event for getting hit
			EV_S_WEAPONHIT SWeaponHit{};
			SWeaponHit.usSoldierID = pTargetSoldier->ubID;
			SWeaponHit.usWeaponIndex = pSoldier->usAttackingWeapon;
			SWeaponHit.sDamage = (INT16) iImpact;
			SWeaponHit.usDirection = GetDirectionFromGridNo( pSoldier->sGridNo, pTargetSoldier );
			SWeaponHit.sXPos = (INT16)pTargetSoldier->dXPos;
			SWeaponHit.sYPos = (INT16)pTargetSoldier->dYPos;
			SWeaponHit.sZPos = 20;
			SWeaponHit.sRange = 1;
			SWeaponHit.ubAttackerID = pSoldier->ubID;
			SWeaponHit.ubSpecial = FIRE_WEAPON_NO_SPECIAL;
			AddGameEvent(SWeaponHit, 20);
		}
		else
		{
			// AGILITY GAIN (10):  Target avoids a knife attack
			AgilityForEnemyMissingPlayer(pSoldier, pTargetSoldier, 10);
			SLOGD("Freeing up attacker - missed in knife attack");
			FreeUpAttacker(pSoldier);
		}

		if (IsOnOurTeam(*pSoldier) && pSoldier->target != NULL)
		{
			if ( fGonnaHit )
			{
				// grant extra exp for hitting a difficult target
				usExpGain += (UINT8) (100 - iHitChance) / 10;

				if (pSoldier->bAimTime)
				{
					// gain extra exp for aiming, up to the amount from
					// the difficulty of the attack
					usExpGain += 2 * std::min(int(pSoldier->bAimTime), int(usExpGain));
				}

				// base pts extra for hitting
				usExpGain += 10;
			}

			// add base pts for taking a shot, whether it hits or misses
			usExpGain += 10;

			usExpGain = ModifyExpGainByTarget(usExpGain, pSoldier->target);

			// DEXTERITY GAIN:  Made a knife attack, successful or not
			StatChange(*pSoldier, DEXTAMT, usExpGain, fGonnaHit ? FROM_SUCCESS : FROM_FAILURE);
		}
	}
	else
	{
		SLOGD("Freeing up attacker - missed in knife attack");
		FreeUpAttacker(pSoldier);
	}

	// possibly reduce monster smell
	if ( pSoldier->bMonsterSmell > 0 && Random( 5 ) == 0 )
	{
		pSoldier->bMonsterSmell--;
	}
}


static UINT32 CalcChanceToSteal(SOLDIERTYPE* pAttacker, SOLDIERTYPE* pDefender, UINT8 ubAimTime);


void UseHandToHand(SOLDIERTYPE* const pSoldier, INT16 const sTargetGridNo, BOOLEAN const fStealing)
{
	INT32          iHitChance, iDiceRoll;
	INT16          sAPCost;
	INT32          iImpact;
	UINT16         usOldItem;

	// Deduct points!
	// August 13 2002: unless stealing - APs already deducted elsewhere
	if (!fStealing)
	{
		sAPCost = CalcTotalAPsToAttack( pSoldier, sTargetGridNo, FALSE, pSoldier->bAimTime );

		DeductPoints( pSoldier, sAPCost, 0 );
	}

	// See if a guy is here!
	SOLDIERTYPE* const pTargetSoldier = WhoIsThere2(sTargetGridNo, pSoldier->bTargetLevel);
	if ( pTargetSoldier )
	{
		// set target as noticed attack
		pSoldier->uiStatusFlags |= SOLDIER_ATTACK_NOTICED;
		pTargetSoldier->fIntendedTarget = TRUE;

		pSoldier->opponent = pTargetSoldier;

		if (fStealing)
		{
			if (AM_A_ROBOT(pTargetSoldier) || TANK(pTargetSoldier) || CREATURE_OR_BLOODCAT(pTargetSoldier))
			{
				iHitChance = 0;
			}
			else if ( pTargetSoldier->bOppList[ pSoldier->ubID ] == NOT_HEARD_OR_SEEN )
			{
				// give bonus for surprise, but not so much as struggle would still occur
				iHitChance = CalcChanceToSteal( pSoldier, pTargetSoldier, pSoldier->bAimTime ) + 20;
			}
			else if ( pTargetSoldier->bLife < OKLIFE || pTargetSoldier->bCollapsed )
			{
				iHitChance = 100;
			}
			else
			{
				iHitChance = CalcChanceToSteal( pSoldier, pTargetSoldier, pSoldier->bAimTime );
			}
		}
		else
		{
			if ( pTargetSoldier->bOppList[ pSoldier->ubID ] == NOT_HEARD_OR_SEEN || pTargetSoldier->bLife < OKLIFE || pTargetSoldier->bCollapsed )
			{
				iHitChance = 100;
			}
			else
			{
				iHitChance = CalcChanceToPunch( pSoldier, pTargetSoldier, pSoldier->bAimTime );
			}
		}

		// ROLL DICE
		iDiceRoll = (INT32) PreRandom( 100 );

		if (fStealing )
		{
			if ( pTargetSoldier->inv[HANDPOS].usItem != NOTHING )
			{

				if ( iDiceRoll <= iHitChance )
				{
					// Was a good steal!
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(g_langRes->Message[ STR_STOLE_SOMETHING ], pSoldier->name, GCM->getItem(pTargetSoldier->inv[HANDPOS].usItem)->getShortName()) );

					usOldItem = pTargetSoldier->inv[HANDPOS].usItem;

					if (pSoldier->bTeam == OUR_TEAM && pTargetSoldier->bTeam != OUR_TEAM && !IsMechanical(*pTargetSoldier) && !TANK(pTargetSoldier))
					{
						// made a steal; give experience
						StatChange(*pSoldier, STRAMT, 8, FROM_SUCCESS);
					}

					if ( iDiceRoll <= iHitChance * 2 / 3)
					{
						// Grabbed item
						if (AutoPlaceObject( pSoldier, &(pTargetSoldier->inv[HANDPOS]), TRUE ))
						{
							// Item transferred; remove it from the target's inventory
							DeleteObj( &(pTargetSoldier->inv[HANDPOS]) );
						}
						else
						{
							// No room to hold it so the item should drop in our tile again
							AddItemToPool(pSoldier->sGridNo, &pTargetSoldier->inv[HANDPOS], VISIBLE, pSoldier->bLevel, 0, -1);
							DeleteObj( &(pTargetSoldier->inv[HANDPOS]) );
						}
					}
					else
					{

						if ( pSoldier->bTeam == OUR_TEAM )
						{
							DoMercBattleSound( pSoldier, BATTLE_SOUND_CURSE1 );
						}

						// Item dropped somewhere... roll based on the same chance to determine where!
						iDiceRoll = (INT32) PreRandom( 100 );
						if (iDiceRoll < iHitChance)
						{
							// Drop item in the our tile
							AddItemToPool(pSoldier->sGridNo, &(pTargetSoldier->inv[HANDPOS]), VISIBLE, pSoldier->bLevel, 0, -1);
						}
						else
						{
							// Drop item in the target's tile
							AddItemToPool(pTargetSoldier->sGridNo, &pTargetSoldier->inv[HANDPOS], VISIBLE, pSoldier->bLevel, 0, -1);
						}
						DeleteObj( &(pTargetSoldier->inv[HANDPOS]) );
					}

					// Reload buddy's animation...
					ReLoadSoldierAnimationDueToHandItemChange( pTargetSoldier, usOldItem, NOTHING );

				}
				else
				{
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE,
						st_format_printf(g_langRes->Message[ STR_FAILED_TO_STEAL_SOMETHING ],
						pSoldier->name, GCM->getItem(pTargetSoldier->inv[HANDPOS].usItem)->getShortName()) );
					if ( pSoldier->bTeam == OUR_TEAM )
					{
						DoMercBattleSound( pSoldier, BATTLE_SOUND_CURSE1 );
					}

					if (iHitChance > 0 && pSoldier->bTeam == OUR_TEAM && pTargetSoldier->bTeam != OUR_TEAM && !IsMechanical(*pTargetSoldier) && !TANK(pTargetSoldier))
					{
						// failed a steal; give some experience
						StatChange(*pSoldier, STRAMT, 4, FROM_FAILURE);
					}

				}
			}
			SLOGD("Freeing up attacker - steal");
			FreeUpAttacker(pSoldier);
		}
		else
		{

			// ATE/CC: if doing ninja spin kick (only), automatically make it a hit
			if ( pSoldier->usAnimState == NINJA_SPINKICK)
			{
				// Let him to succeed by a random amount
				iDiceRoll = PreRandom( iHitChance );
			}

			if ( pSoldier->bTeam == OUR_TEAM && pTargetSoldier->bTeam != OUR_TEAM )
			{
				// made an HTH attack; give experience
				UINT8           ubExpGain;
				StatChangeCause reason;
				if (iDiceRoll <= iHitChance)
				{
					ubExpGain = 8;
					reason    = FROM_SUCCESS;
				}
				else
				{
					ubExpGain = 4;
					reason    = FROM_FAILURE;
				}

				ubExpGain = ModifyExpGainByTarget(ubExpGain, pTargetSoldier);

				StatChange(*pSoldier, STRAMT,  ubExpGain, reason);
				StatChange(*pSoldier, DEXTAMT, ubExpGain, reason);
			}
			else if (iDiceRoll > iHitChance)
			{
				// being attacked... successfully dodged, give experience
				AgilityForEnemyMissingPlayer(pSoldier, pTargetSoldier, 8);
			}

			if ( iDiceRoll <= iHitChance || AreInMeanwhile( ) )
			{
				// CALCULATE DAMAGE!
				iImpact = HTHImpact( pSoldier, pTargetSoldier, (iHitChance - iDiceRoll), FALSE );

				// Send event for getting hit
				EV_S_WEAPONHIT SWeaponHit{};
				SWeaponHit.usSoldierID = pTargetSoldier->ubID;
				SWeaponHit.usWeaponIndex = pSoldier->usAttackingWeapon;
				SWeaponHit.sDamage = (INT16) iImpact;
				SWeaponHit.usDirection = GetDirectionFromGridNo( pSoldier->sGridNo, pTargetSoldier );
				SWeaponHit.sXPos = (INT16)pTargetSoldier->dXPos;
				SWeaponHit.sYPos = (INT16)pTargetSoldier->dYPos;
				SWeaponHit.sZPos = 20;
				SWeaponHit.sRange = 1;
				SWeaponHit.ubAttackerID = pSoldier->ubID;
				SWeaponHit.ubSpecial = FIRE_WEAPON_NO_SPECIAL;
				AddGameEvent(SWeaponHit, 20);
			}
			else
			{
				SLOGD("Freeing up attacker - missed in HTH attack");
				FreeUpAttacker(pSoldier);
			}
		}
	}

	// possibly reduce monster smell (gunpowder smell)
	if ( pSoldier->bMonsterSmell > 0 && Random( 5 ) == 0 )
	{
		pSoldier->bMonsterSmell--;
	}
}


static void UseThrown(SOLDIERTYPE * const pSoldier, GridNo const sTargetGridNo)
{
	UINT32 uiHitChance, uiDiceRoll;
	INT8   bLoop;

	uiHitChance = CalcThrownChanceToHit( pSoldier, sTargetGridNo, pSoldier->bAimTime, AIM_SHOT_TORSO );

	uiDiceRoll = PreRandom( 100 );

	if ( pSoldier->bTeam == OUR_TEAM && gTacticalStatus.uiFlags & INCOMBAT )
	{
		// check target gridno
		const SOLDIERTYPE* pTargetSoldier = WhoIsThere2(pSoldier->sTargetGridNo, pSoldier->bTargetLevel);
		if ( pTargetSoldier && pTargetSoldier->bTeam == pSoldier->bTeam )
		{
			// ignore!
			pTargetSoldier = NULL;
		}

		if ( pTargetSoldier == NULL )
		{
			// search for an opponent near the target gridno
			for ( bLoop = 0; bLoop < NUM_WORLD_DIRECTIONS; bLoop++ )
			{
				pTargetSoldier = WhoIsThere2(NewGridNo(pSoldier->sTargetGridNo, DirectionInc(bLoop)), pSoldier->bTargetLevel);
				if (pTargetSoldier != NULL && pTargetSoldier->bTeam != pSoldier->bTeam) break;
			}
		}

		if ( pTargetSoldier )
		{
			// ok this is a real attack on someone, grant experience
			StatChange(*pSoldier, STRAMT, 5, FROM_SUCCESS);
			if ( uiDiceRoll < uiHitChance )
			{
				StatChange(*pSoldier, DEXTAMT, 5, FROM_SUCCESS);
				StatChange(*pSoldier, MARKAMT, 5, FROM_SUCCESS);
			}
			else
			{
				StatChange(*pSoldier, DEXTAMT, 2, FROM_FAILURE);
				StatChange(*pSoldier, MARKAMT, 2, FROM_FAILURE);
			}
		}
	}


	CalculateLaunchItemParamsForThrow( pSoldier, sTargetGridNo, pSoldier->bTargetLevel, (INT16)(pSoldier->bTargetLevel * 256 ), &(pSoldier->inv[ HANDPOS ] ), (INT8)(uiDiceRoll - uiHitChance), THROW_ARM_ITEM, 0 );

	// OK, goto throw animation
	HandleSoldierThrowItem( pSoldier, pSoldier->sTargetGridNo );

	UINT16 const thrown_item=pSoldier->inv[HANDPOS].usItem;
	RemoveObjs( &(pSoldier->inv[ HANDPOS ] ), 1 );
	if(pSoldier->inv[HANDPOS].usItem == NOTHING)
	{
		INT8 const slot=FindObj(pSoldier, thrown_item);
		if(slot!=NO_SLOT)
		{
			OBJECTTYPE *item=&pSoldier->inv[slot];
			CreateItem(item->usItem, item->bStatus[0], &pSoldier->inv[HANDPOS]);
			RemoveObjs(item, 1);
		}
		DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
	}
}


static void UseLauncher(SOLDIERTYPE * const pSoldier, GridNo const sTargetGridNo)
{
	UINT32     uiHitChance, uiDiceRoll;
	INT16      sAPCost = 0;
	INT8       bAttachPos;
	OBJECTTYPE Launchable;
	OBJECTTYPE *pObj;
	UINT16     usItemNum;

	usItemNum = pSoldier->usAttackingWeapon;

	if ( !EnoughAmmo( pSoldier, TRUE, pSoldier->ubAttackingHand ) )
	{
		return;
	}

	pObj = &(pSoldier->inv[HANDPOS]);
	for (bAttachPos = 0; bAttachPos < MAX_ATTACHMENTS; bAttachPos++)
	{
		if (pObj->usAttachItem[ bAttachPos ] != NOTHING)
		{
			if ( GCM->getItem(pObj->usAttachItem[ bAttachPos ])->isExplosive() )
			{
				break;
			}
		}
	}
	if (bAttachPos == MAX_ATTACHMENTS)
	{
		// this should not happen!!
		return;
	}

	CreateItem( pObj->usAttachItem[ bAttachPos ],	pObj->bAttachStatus[ bAttachPos ], &Launchable );

	if ( pSoldier->usAttackingWeapon == pObj->usItem)
	{
		DeductAmmo( pSoldier, HANDPOS );
	}
	else
	{
		// Firing an attached grenade launcher... the attachment we found above
		// is the one to remove!
		RemoveAttachment( pObj, bAttachPos, NULL );
	}

	// ATE: Check here if the launcher should fail 'cause of bad status.....
	if ( WillExplosiveWeaponFail( pSoldier, pObj ) )
	{
		// Explode dude!

		// So we still should have ABC > 0
		// Begin explosion due to failure...
		IgniteExplosion(pSoldier, 0, pSoldier->sGridNo, Launchable.usItem, pSoldier->bLevel);

		// Reduce again for attack end 'cause it has been incremented for a normal attack
		SLOGD("Freeing up attacker - ATTACK ANIMATION {} ENDED BY BAD EXPLOSIVE CHECK, Now {}",
			gAnimControl[pSoldier->usAnimState].zAnimStr, gTacticalStatus.ubAttackBusyCount);
		ReduceAttackBusyCount(pSoldier, FALSE);

		// So all's well, should be good from here....
		return;
	}

	if ( !GCM->getWeapon( usItemNum )->sound.empty()  )
	{
		PlayLocationJA2Sample(pSoldier->sGridNo, GCM->getWeapon(usItemNum)->sound, HIGHVOLUME, 1);
	}

	uiHitChance = CalcThrownChanceToHit( pSoldier, sTargetGridNo, pSoldier->bAimTime, AIM_SHOT_TORSO );

	uiDiceRoll = PreRandom( 100 );

	if ( GCM->getItem(usItemNum)->getItemClass() == IC_LAUNCHER )
	{
		// Preserve gridno!
		//pSoldier->sLastTarget = sTargetGridNo;

		sAPCost = MinAPsToAttack( pSoldier, sTargetGridNo, TRUE );
	}
	else
	{
		// Throw....
		sAPCost = MinAPsToThrow(*pSoldier, sTargetGridNo, FALSE);
	}

	DeductPoints( pSoldier, sAPCost, 0 );

	CalculateLaunchItemParamsForThrow( pSoldier, pSoldier->sTargetGridNo, pSoldier->bTargetLevel, 0, &Launchable, (INT8)(uiDiceRoll - uiHitChance), THROW_ARM_ITEM, 0 );

	const THROW_PARAMS* const t = pSoldier->pThrowParams;
	CreatePhysicalObject(pSoldier->pTempObject, t->dLifeSpan, t->dX, t->dY, t->dZ, t->dForceX, t->dForceY, t->dForceZ, pSoldier, t->ubActionCode, t->target);

	ClearTempObject(pSoldier);

	delete pSoldier->pThrowParams;
	pSoldier->pThrowParams = NULL;
}


static BOOLEAN DoSpecialEffectAmmoMiss(SOLDIERTYPE* const attacker, const INT16 sGridNo, const INT16 sXPos, const INT16 sYPos, const INT16 sZPos, const BOOLEAN fSoundOnly, const BOOLEAN fFreeupAttacker, BULLET* const bullet)
{
	ANITILE_PARAMS AniParams;
	UINT8          ubAmmoType;
	UINT16         usItem;

	ubAmmoType = attacker->inv[attacker->ubAttackingHand].ubGunAmmoType;
	usItem     = attacker->inv[attacker->ubAttackingHand].usItem;

	AniParams = ANITILE_PARAMS{};

	if ( ubAmmoType == AMMO_HE || ubAmmoType == AMMO_HEAT )
	{
		if ( !fSoundOnly )
		{
			AniParams.sGridNo = sGridNo;
			AniParams.ubLevelID = ANI_TOPMOST_LEVEL;
			AniParams.sDelay = (INT16)( 100 );
			AniParams.sStartFrame = 0;
			AniParams.uiFlags = ANITILE_FORWARD | ANITILE_ALWAYS_TRANSLUCENT;
			AniParams.sX = sXPos;
			AniParams.sY = sYPos;
			AniParams.sZ = sZPos;
			AniParams.zCachedFile = TILECACHEDIR "/miniboom.sti";
			CreateAnimationTile( &AniParams );

			if ( fFreeupAttacker )
			{
				if (bullet) RemoveBullet(bullet);
				SLOGD("Freeing up attacker - bullet hit structure - explosive ammo");
				FreeUpAttacker(attacker);
			}
		}

		if ( sGridNo != NOWHERE )
		{
			PlayLocationJA2Sample(sGridNo, SMALL_EXPLODE_1, HIGHVOLUME, 1);
		}
		else
		{
			PlayJA2Sample(SMALL_EXPLODE_1, MIDVOLUME, 1, MIDDLE);
		}

		return( TRUE );
	}
	else if (ubAmmoType == AMMO_MONSTER)
	{
		UINT16 gas = GCM->getWeapon(usItem)->usSmokeEffect;
		if (gas == NONE)
		{
			return FALSE;
		}

		// Increment attack busy...
		// gTacticalStatus.ubAttackBusyCount++;
		// SLOGD("Incrementing Attack: Explosion gone off, COunt now {}", gTacticalStatus.ubAttackBusyCount);

		PlayLocationJA2Sample(sGridNo, CREATURE_GAS_NOISE, HIGHVOLUME, 1);

		NewSmokeEffect(sGridNo, gas, 0, attacker);
	}

	return( FALSE );
}


void WeaponHit(SOLDIERTYPE* const pTargetSoldier, const UINT16 usWeaponIndex, const INT16 sDamage,
		const INT16 sBreathLoss, const UINT16 usDirection, const INT16 sXPos, const INT16 sYPos,
		const INT16 sZPos, const INT16 sRange, SOLDIERTYPE* const attacker, const UINT8 ubSpecial,
		const UINT8 ubHitLocation)
{
	MakeNoise(attacker, pTargetSoldier->sGridNo, pTargetSoldier->bLevel,
			GCM->getWeapon(usWeaponIndex)->ubHitVolume, NOISE_BULLET_IMPACT);

	if ( EXPLOSIVE_GUN( usWeaponIndex ) )
	{
		// Reduce attacker count!
		const UINT16 item = (usWeaponIndex == ROCKET_LAUNCHER ? C1 : TANK_SHELL);
		IgniteExplosionXY(attacker, sXPos, sYPos, 0, GETWORLDINDEXFROMWORLDCOORDS(sYPos, sXPos), item, pTargetSoldier->bLevel);

		SLOGD("Freeing up attacker - end of LAW fire");
		FreeUpAttacker(attacker);
		return;
	}

	DoSpecialEffectAmmoMiss(attacker, pTargetSoldier->sGridNo, sXPos, sYPos, sZPos, FALSE, FALSE, NULL);

	// OK, SHOT HAS HIT, DO THINGS APPROPRIATELY
	// ATE: This is 'cause of that darn smoke effect that could potnetially kill
	// the poor bastard .. so check
	if ( !pTargetSoldier->fDoingExternalDeath )
	{
		EVENT_SoldierGotHit(pTargetSoldier, usWeaponIndex, sDamage, sBreathLoss,
					usDirection, sRange, attacker, ubSpecial, ubHitLocation, NOWHERE);
	}
	else
	{
		// Buddy had died from additional dammage - free up attacker here...
		ReduceAttackBusyCount(pTargetSoldier->attacker, FALSE);
		SLOGD("Special effect killed before bullet impact, attack count now {}",
			gTacticalStatus.ubAttackBusyCount);
	}
}


void StructureHit(BULLET* const pBullet, const UINT16 usStructureID, const INT32 iImpact, const BOOLEAN fStopped)
{
	const INT16 sXPos = FIXEDPT_TO_INT32(pBullet->qCurrX);
	const INT16 sYPos = FIXEDPT_TO_INT32(pBullet->qCurrY);
	const INT16 sZPos = CONVERT_HEIGHTUNITS_TO_PIXELS(FIXEDPT_TO_INT32(pBullet->qCurrZ));

	BOOLEAN        fDoMissForGun = FALSE;
	ANITILE        *pNode;
	ANITILE_PARAMS AniParams;
	UINT32         uiMissVolume = MIDVOLUME;

	SOLDIERTYPE* const attacker      = pBullet->pFirer;
	const UINT16       usWeaponIndex = attacker->usAttackingWeapon;
	const INT8         bWeaponStatus = pBullet->ubItemStatus;

	if (fStopped)
	{
		// AGILITY GAIN: Opponent "dodged" a bullet shot at him (it missed)
		SOLDIERTYPE* const opp = attacker->opponent;
		if (opp != NULL) AgilityForEnemyMissingPlayer(attacker, opp, 5);
	}

	const BOOLEAN fHitSameStructureAsBefore = (usStructureID == pBullet->usLastStructureHit);

	// Tile's height is here to account for cliff-elevated sectors, e.g. Drassen mine. Everywhere else it's zero
	const int8_t level = sZPos > ( WALL_HEIGHT - ROOF_HIT_ADJUSTMENT ) + gpWorldLevelData[attacker->sGridNo].sHeight;

	const GridNo sGridNo = pBullet->sGridNo;
	if ( !fHitSameStructureAsBefore )
	{
		MakeNoise(attacker, sGridNo, level, GCM->getWeapon(usWeaponIndex)->ubHitVolume, NOISE_BULLET_IMPACT);
	}

	if (fStopped)
	{
		if ( usWeaponIndex == ROCKET_LAUNCHER )
		{
			RemoveBullet(pBullet);

			// Reduce attacker count!
			SLOGD("Freeing up attacker - end of LAW fire");
			FreeUpAttacker(attacker);

			IgniteExplosion(attacker, 0, sGridNo, C1, level);
			//FreeUpAttacker(attacker);

			return;
		}

		if ( usWeaponIndex == TANK_CANNON )
		{
			RemoveBullet(pBullet);

			// Reduce attacker count!
			SLOGD("Freeing up attacker - end of TANK fire");
			FreeUpAttacker(attacker);

			IgniteExplosion(attacker, 0, sGridNo, TANK_SHELL, sZPos >= WALL_HEIGHT);
			//FreeUpAttacker(attacker);

			return;
		}
	}

	STRUCTURE* pStructure = nullptr;
	// Get Structure pointer and damage it!
	if ( usStructureID != INVALID_STRUCTURE_ID )
	{
		pStructure = FindStructureByID(sGridNo, usStructureID);
		DamageStructure(pStructure, iImpact, STRUCTURE_DAMAGE_GUNFIRE, sGridNo, sXPos, sYPos, attacker);
	}

	GridNo adjustedGridNo = sGridNo;
	GridNo oppositeSideGridNo = sGridNo;
	switch(  GCM->getWeapon( usWeaponIndex )->ubWeaponClass )
	{
		case HANDGUNCLASS:
		case RIFLECLASS:
		case SHOTGUNCLASS:
		case SMGCLASS:
		case MGCLASS:
			// Guy has missed, play random sound
			if (attacker->bTeam == OUR_TEAM &&
				!attacker->bDoBurst &&
				Random(40) == 0)
			{
				DoMercBattleSound(attacker, BATTLE_SOUND_CURSE1);
			}
			//fDoMissForGun = TRUE;
			//break;
			fDoMissForGun = TRUE;
			break;

		case MONSTERCLASS:
			// If the structure is wall-oriented determine which side of it monster spit arrives at
			pStructure = FindStructureByID(sGridNo, usStructureID);
			if (pStructure && pStructure->ubWallOrientation)
			{
				if (pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT || pStructure->ubWallOrientation == INSIDE_TOP_RIGHT)
				{
					oppositeSideGridNo += DirectionInc(EAST);
				}
				else
				{
					oppositeSideGridNo += DirectionInc(SOUTH);
				}
				if (PythSpacesAway(attacker->sGridNo, sGridNo) > PythSpacesAway(attacker->sGridNo, oppositeSideGridNo))
				{
					adjustedGridNo = oppositeSideGridNo;
				}
			}
			DoSpecialEffectAmmoMiss(attacker, adjustedGridNo, sXPos, sYPos, sZPos, FALSE, TRUE, pBullet);

			RemoveBullet(pBullet);
			SLOGD("Freeing up attacker - monster attack hit structure");
			FreeUpAttacker(attacker);

			//PlayJA2Sample(SPIT_RICOCHET, uiMissVolume, 1, SoundDir(sGridNo));
			break;

		case KNIFECLASS:

			// When it hits the ground, leave on map...
			if ( GCM->getItem(usWeaponIndex)->getItemClass() == IC_THROWING_KNIFE )
			{
				OBJECTTYPE Object;

				// OK, have we hit ground?
				if ( usStructureID == INVALID_STRUCTURE_ID )
				{
					// Add item
					CreateItem( THROWING_KNIFE, bWeaponStatus, &Object );

					AddItemToPool(sGridNo, &Object, INVISIBLE, 0, 0, -1);

					// Make team look for items
					NotifySoldiersToLookforItems( );
				}

				if ( !fHitSameStructureAsBefore )
				{
					PlayJA2Sample(MISS_KNIFE, uiMissVolume, 1, SoundDir(sGridNo));
				}

				RemoveBullet(pBullet);
				SLOGD("Freeing up attacker - knife attack hit structure");
				FreeUpAttacker(attacker);
			}
	}

	if ( fDoMissForGun )
	{
		// OK, are we a shotgun, if so , make sounds lower...
		if ( GCM->getWeapon( usWeaponIndex )->ubWeaponClass == SHOTGUNCLASS )
		{
			uiMissVolume = LOWVOLUME;
		}

		// Free guy!
		//SLOGD("Freeing up attacker - bullet hit structure");
		//FreeUpAttacker(attacker);


		// PLAY SOUND AND FLING DEBRIS
		// RANDOMIZE SOUND SYSTEM

		// IF WE HIT THE GROUND

		if ( fHitSameStructureAsBefore )
		{
			if ( fStopped )
			{
				RemoveBullet(pBullet);
				SLOGD("Freeing up attacker - bullet hit same structure twice");
				FreeUpAttacker(attacker);
			}
		}
		else
		{
			if (!fStopped || !DoSpecialEffectAmmoMiss(attacker, sGridNo, sXPos, sYPos, sZPos, FALSE, TRUE, pBullet))
			{
				if ( sZPos == 0 )
				{
					PlayJA2Sample(MISS_G2, uiMissVolume, 1, SoundDir(sGridNo));
				}
				else
				{
					PlayJA2Sample(SoundRange<MISS_1, MISS_8>(), uiMissVolume, 1, SoundDir(sGridNo));
				}

				// Default hit is the ground
				UINT16 usMissTileIndex = FIRSTMISS1;

				// Check if we are in water...
				if ( gpWorldLevelData[ sGridNo ].ubTerrainID == LOW_WATER ||  gpWorldLevelData[ sGridNo ].ubTerrainID == DEEP_WATER )
				{
					usMissTileIndex = SECONDMISS1;

					// Add ripple
					AniParams = ANITILE_PARAMS{};
					AniParams.sGridNo = sGridNo;
					AniParams.ubLevelID = ANI_STRUCT_LEVEL;
					AniParams.usTileIndex = THIRDMISS1;
					AniParams.sDelay = 50;
					AniParams.sStartFrame = 0;
					AniParams.uiFlags = ANITILE_FORWARD;

					pNode = CreateAnimationTile( &AniParams );

					// Adjust for absolute positioning
					pNode->pLevelNode->uiFlags |= LEVELNODE_USEABSOLUTEPOS;
					pNode->pLevelNode->sRelativeX	= sXPos;
					pNode->pLevelNode->sRelativeY	= sYPos;
					pNode->pLevelNode->sRelativeZ = sZPos;

				}

				AniParams = ANITILE_PARAMS{};
				AniParams.sGridNo = sGridNo;
				AniParams.ubLevelID = ANI_STRUCT_LEVEL;
				AniParams.usTileIndex = usMissTileIndex;
				AniParams.sDelay = 80;
				AniParams.sStartFrame = 0;
				if (fStopped)
				{
					AniParams.uiFlags = ANITILE_FORWARD | ANITILE_RELEASE_ATTACKER_WHEN_DONE;
					AniParams.v.bullet = pBullet;
				}
				else
				{
					AniParams.uiFlags = ANITILE_FORWARD;
				}

				pNode = CreateAnimationTile( &AniParams );

				// Adjust for absolute positioning
				pNode->pLevelNode->uiFlags |= LEVELNODE_USEABSOLUTEPOS;
				pNode->pLevelNode->sRelativeX = sXPos;
				pNode->pLevelNode->sRelativeY = sYPos;
				pNode->pLevelNode->sRelativeZ = sZPos;

				// ATE: Show misses...( if our team )
				if (gGameSettings.fOptions[TOPTION_SHOW_MISSES] &&
					attacker->bTeam == OUR_TEAM)
				{
					LocateGridNo(sGridNo);
				}
			}

			pBullet->usLastStructureHit = usStructureID;

		}
	}
}

void WindowHit( INT16 sGridNo, UINT16 usStructureID, BOOLEAN fBlowWindowSouth, BOOLEAN fLargeForce )
{
	STRUCTURE      *pWallAndWindow;
	DB_STRUCTURE   *pWallAndWindowInDB;
	INT16          sShatterGridNo;
	UINT16         usTileIndex;
	ANITILE_PARAMS AniParams;


	// ATE: Make large force always for now ( feel thing )
	fLargeForce = TRUE;

	// we have to do two things here: swap the window structure
	// (right now just using the partner stuff in a chain from
	// intact to cracked to shattered) and display the
	// animation if we've reached shattered

	// find the wall structure, and go one length along the chain
	pWallAndWindow = FindStructureByID( sGridNo, usStructureID );
	if (pWallAndWindow == NULL)
	{
		return;
	}

	pWallAndWindow = SwapStructureForPartner(pWallAndWindow);
	if (pWallAndWindow == NULL)
	{
		return;
	}

	// record window smash
	AddWindowHitToMapTempFile( sGridNo );

	pWallAndWindowInDB = pWallAndWindow->pDBStructureRef->pDBStructure;

	if ( fLargeForce )
	{
		// Force to destruction animation!
		if (pWallAndWindowInDB->bPartnerDelta != NO_PARTNER_STRUCTURE  )
		{
			pWallAndWindow = SwapStructureForPartner(pWallAndWindow);
			if ( pWallAndWindow )
			{
				// record 2nd window smash
				AddWindowHitToMapTempFile( sGridNo );

				pWallAndWindowInDB = pWallAndWindow->pDBStructureRef->pDBStructure;
			}
		}
	}

	SetRenderFlags( RENDER_FLAG_FULL );

	if (pWallAndWindowInDB->ubArmour == MATERIAL_THICKER_METAL_WITH_SCREEN_WINDOWS)
	{
		// don't play any sort of animation or sound
		return;
	}

	if (pWallAndWindowInDB->bPartnerDelta != NO_PARTNER_STRUCTURE  )
	{ // just cracked; don't display the animation
		MakeNoise(NULL, sGridNo, 0, WINDOW_CRACK_VOLUME, NOISE_BULLET_IMPACT);
		return;
	}
	MakeNoise(NULL, sGridNo, 0, WINDOW_SMASH_VOLUME, NOISE_BULLET_IMPACT);
	if (pWallAndWindowInDB->ubWallOrientation == INSIDE_TOP_RIGHT || pWallAndWindowInDB->ubWallOrientation == OUTSIDE_TOP_RIGHT)
	{
		/*
		sShatterGridNo = sGridNo + 1;
		// check for wrapping around edge of map
		if (sShatterGridNo % WORLD_COLS == 0)
		{
			// in which case we don't play the animation!
			return;
		}*/
		if (fBlowWindowSouth)
		{
			usTileIndex = WINDOWSHATTER1;
			sShatterGridNo = sGridNo + 1;
		}
		else
		{
			usTileIndex = WINDOWSHATTER11;
			sShatterGridNo = sGridNo;
		}

	}
	else
	{
		/*
		sShatterGridNo = sGridNo + WORLD_COLS;
		// check for wrapping around edge of map
		if (sShatterGridNo % WORLD_ROWS == 0)
		{
			// in which case we don't play the animation!
			return;
		}*/
		if (fBlowWindowSouth)
		{
			usTileIndex = WINDOWSHATTER6;
			sShatterGridNo = sGridNo + WORLD_COLS;
		}
		else
		{
			usTileIndex = WINDOWSHATTER16;
			sShatterGridNo = sGridNo;
		}
	}

	AniParams = ANITILE_PARAMS{};
	AniParams.sGridNo = sShatterGridNo;
	AniParams.ubLevelID = ANI_STRUCT_LEVEL;
	AniParams.usTileIndex = usTileIndex;
	AniParams.sDelay = 50;
	AniParams.sStartFrame = 0;
	AniParams.uiFlags = ANITILE_FORWARD;

	CreateAnimationTile( &AniParams );

	PlayJA2Sample(SoundRange<GLASS_SHATTER1, GLASS_SHATTER2>(), MIDVOLUME, 1, SoundDir(sGridNo));
}


BOOLEAN InRange(const SOLDIERTYPE* pSoldier, INT16 sGridNo)
{
	INT16  sRange;
	UINT16 usInHand;

	usInHand = pSoldier->inv[HANDPOS].usItem;

	if ( GCM->getItem(usInHand)->getItemClass() == IC_GUN ||
		GCM->getItem(usInHand)->getItemClass() == IC_THROWING_KNIFE  )
	{
		// Determine range
		sRange = (INT16)GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, sGridNo );

		if ( GCM->getItem(usInHand)->getItemClass() == IC_THROWING_KNIFE )
		{
			// NB CalcMaxTossRange returns range in tiles, not in world units
			if ( sRange <= CalcMaxTossRange( pSoldier, THROWING_KNIFE, TRUE ) * CELL_X_SIZE )
			{
				return( TRUE );
			}
		}
		else
		{
			// For given weapon, check range
			if (sRange <= GunRange(pSoldier->inv[HANDPOS]))
			{
				return( TRUE );
			}
		}
	}
	return( FALSE );
}

UINT32 CalcChanceToHitGun(SOLDIERTYPE *pSoldier, UINT16 sGridNo, UINT8 ubAimTime, UINT8 ubAimPos, BOOLEAN fModify )
{
	INT32 iChance, iRange, iSightRange, iMaxRange, iScopeBonus, iBonus; //, minRange;
	INT32 iGunCondition, iMarksmanship;
	INT32 iPenalty;
	UINT16 usInHand;
	OBJECTTYPE *pInHand;
	INT8 bAttachPos;
	INT8 bBandaged;
	INT16 sDistVis;
	UINT8 ubAdjAimPos;

	if ( pSoldier->bMarksmanship == 0 )
	{
		// always min chance
		return( MINCHANCETOHIT );
	}

	// make sure the guy's actually got a weapon in his hand!
	pInHand = &(pSoldier->inv[pSoldier->ubAttackingHand]);
	usInHand = pSoldier->usAttackingWeapon;

	// DETERMINE BASE CHANCE OF HITTING
	iGunCondition = WEAPON_STATUS_MOD( pInHand->bGunStatus );

	if (usInHand == ROCKET_LAUNCHER)
	{
		// use the same calculation as for mechanical thrown weapons
		iMarksmanship = ( EffectiveDexterity( pSoldier ) + EffectiveMarksmanship( pSoldier ) + EffectiveWisdom( pSoldier ) + (10 * EffectiveExpLevel( pSoldier ) )) / 4;
		// heavy weapons trait helps out
		iMarksmanship += gbSkillTraitBonus[HEAVY_WEAPS] * NUM_SKILL_TRAITS(pSoldier, HEAVY_WEAPS);
	}
	else
	{
		iMarksmanship = EffectiveMarksmanship( pSoldier );

		if ( AM_A_ROBOT( pSoldier ) )
		{
			const SOLDIERTYPE * pSoldier2;

			pSoldier2 = GetRobotController( pSoldier );
			if ( pSoldier2 )
			{
				iMarksmanship = std::max(iMarksmanship, int(EffectiveMarksmanship(pSoldier2)));
			}
		}
	}

	// modify chance to hit by morale
	iMarksmanship += GetMoraleModifier( pSoldier );

	// penalize marksmanship for fatigue
	iMarksmanship -= GetSkillCheckPenaltyForFatigue( pSoldier, iMarksmanship );

	if (iGunCondition >= iMarksmanship)
		// base chance is equal to the shooter's marksmanship skill
		iChance = iMarksmanship;
	else
		// base chance is equal to the average of marksmanship & gun's condition!
		iChance = (iMarksmanship + iGunCondition) / 2;

	// if shooting same target as the last shot
	if (sGridNo == pSoldier->sLastTarget )
		iChance += AIM_BONUS_SAME_TARGET; // give a bonus to hit

	if ( pSoldier->ubProfile != NO_PROFILE && gMercProfiles[ pSoldier->ubProfile ].bPersonalityTrait == PSYCHO )
	{
		iChance += AIM_BONUS_PSYCHO;
	}

	// calculate actual range (in units, 10 units = 1 tile)
	iRange = GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, sGridNo );

	// if shooter is crouched, he aims slightly better (to max of AIM_BONUS_CROUCHING)
	if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_CROUCH )
	{
		iBonus = iRange / 10;
		if (iBonus > AIM_BONUS_CROUCHING)
		{
			iBonus = AIM_BONUS_CROUCHING;
		}
		iChance += iBonus;
	}
	// if shooter is prone, he aims even better, except at really close range
	else if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_PRONE )
	{
		if (iRange > MIN_PRONE_RANGE)
		{
			iBonus = iRange / 10;
			if (iBonus > AIM_BONUS_PRONE)
			{
				iBonus = AIM_BONUS_PRONE;
			}
			bAttachPos = FindAttachment( pInHand, BIPOD );
			if (bAttachPos != ITEM_NOT_FOUND)
			{
				// extra bonus to hit for a bipod, up to half the prone bonus itself
				iBonus += (iBonus * WEAPON_STATUS_MOD(pInHand->bAttachStatus[bAttachPos]) / 100) / 2;
			}
			iChance += iBonus;
		}
	}

	if (GCM->getItem(usInHand)->isWeapon() && !(GCM->getItem(usInHand)->isTwoHanded()))
	{
		// SMGs are treated as pistols for these purpose except there is a -5 penalty;
		if (GCM->getWeapon(usInHand)->ubWeaponClass == SMGCLASS)
		{
			iChance -= AIM_PENALTY_SMG; // TODO0007
		}

		/*
		if (pSoldier->inv[SECONDHANDPOS].usItem == NOTHING)
		{
			// firing with pistol in right hand, and second hand empty.
			iChance += AIM_BONUS_TWO_HANDED_PISTOL;
		}
		else */
		if ( !HAS_SKILL_TRAIT( pSoldier, AMBIDEXT ) )
		{
			if ( IsValidSecondHandShot( pSoldier ) )
			{
				// penalty to aim when firing two pistols
				iChance -= AIM_PENALTY_DUAL_PISTOLS;
			}
			/*
			else
			{
				// penalty to aim with pistol being fired one-handed
				iChance -= AIM_PENALTY_ONE_HANDED_PISTOL;
			}
			*/
		}
	}

	// If in burst mode, deduct points for change to hit for each shot after the first
	if ( pSoldier->bDoBurst )
	{
		iPenalty = GCM->getWeapon(usInHand)->ubBurstPenalty * (pSoldier->bDoBurst - 1);

		// halve the penalty for people with the autofire trait
		UINT AutoWeaponsSkill = NUM_SKILL_TRAITS(pSoldier, AUTO_WEAPS);
		if (AutoWeaponsSkill != 0)
		{
			iPenalty /= 2 * AutoWeaponsSkill;
		}
		iChance -= iPenalty;
	}

	sDistVis = DistanceVisible( pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sGridNo, 0 );

	// give some leeway to allow people to spot for each other...
	// use distance limitation for LOS routine of 2 x maximum distance EVER visible, so that we get accurate
	// calculations out to around 50 tiles.  Because we multiply max distance by 2, we must divide by 2 later

	// CJC August 13 2002:  Wow, this has been wrong the whole time.  bTargetCubeLevel seems to be generally set to 2 -
	// but if a character is shooting at an enemy in a particular spot, then we should be using the target position on the body.

	// CJC August 13, 2002
	// If the start soldier has a body part they are aiming at, and know about the person in the tile, then use that height instead
	iSightRange = -1;

	// best to use team knowledge as well, in case of spotting for someone else
	const SOLDIERTYPE* const tgt = WhoIsThere2(sGridNo, pSoldier->bTargetLevel);
	if (tgt != NULL && (pSoldier->bOppList[tgt->ubID] == SEEN_CURRENTLY || gbPublicOpplist[pSoldier->bTeam][tgt->ubID] == SEEN_CURRENTLY))
	{
		iSightRange = SoldierToBodyPartLineOfSightTest(pSoldier, sGridNo, pSoldier->bTargetLevel,
								pSoldier->bAimShotLocation,
								(UINT8) (MaxDistanceVisible() * 2), TRUE);
	}

	if (iSightRange == -1) // didn't do a bodypart-based test
	{
		iSightRange = SoldierTo3DLocationLineOfSightTest(pSoldier, sGridNo, pSoldier->bTargetLevel,
									pSoldier->bTargetCubeLevel,
									(UINT8) (MaxDistanceVisible() * 2), TRUE);
	}

	iSightRange *= 2;

	if ( iSightRange > (sDistVis * CELL_X_SIZE) )
	{
		// shooting beyond max normal vision... penalize such distance at double (also later we halve the remaining chance)
		iSightRange += (iSightRange - sDistVis * CELL_X_SIZE);
	}

	// if shooter spent some extra time aiming and can see the target
	if (iSightRange > 0 && ubAimTime && !pSoldier->bDoBurst)
		iChance += (AIM_BONUS_PER_AP * ubAimTime); // bonus for every pt of aiming

	if ( !(pSoldier->uiStatusFlags & SOLDIER_PC ) ) // if this is a computer AI controlled enemy
	{
		if ( gGameOptions.ubDifficultyLevel == DIF_LEVEL_EASY )
		{
			// On easy, penalize all enemies by 5%
			iChance -= 5;
		}
		else
		{
			// max with 0 to prevent this being a bonus, for JA2 it's just a penalty to make early enemies easy
			// CJC note: IDIOT!  This should have been a min.  It's kind of too late now...
			// CJC 2002-05-17: changed the max to a min to make this work.
			iChance += std::min(INT8(0), gbDiff[DIFF_ENEMY_TO_HIT_MOD][SoldierDifficultyLevel(pSoldier)]);
		}
	}

	// if shooter is being affected by gas
	if ( pSoldier->uiStatusFlags & SOLDIER_GASSED )
	{
		iChance -= AIM_PENALTY_GASSED;
	}

	// if shooter is being bandaged at the same time, his concentration is off
	if (pSoldier->ubServiceCount > 0)
		iChance -= AIM_PENALTY_GETTINGAID;

	// if shooter is still in shock
	if (pSoldier->bShock)
		iChance -= (pSoldier->bShock * AIM_PENALTY_PER_SHOCK);

	if ( GCM->getItem(usInHand)->getItemClass() == IC_GUN )
	{
		bAttachPos = FindAttachment( pInHand, GUN_BARREL_EXTENDER );
		if ( bAttachPos != ITEM_NOT_FOUND && fModify)
		{
			// reduce status and see if it falls off
			pInHand->bAttachStatus[ bAttachPos ] -= (INT8) Random( 2 );

			if ( pInHand->bAttachStatus[ bAttachPos ] - Random( 35 ) - Random( 35 ) < USABLE )
			{
				// barrel extender falls off!
				OBJECTTYPE Temp;

				// since barrel extenders are not removable we cannot call RemoveAttachment here
				// and must create the item by hand
				CreateItem( GUN_BARREL_EXTENDER, pInHand->bAttachStatus[ bAttachPos ], &Temp );
				pInHand->usAttachItem[ bAttachPos ] = NOTHING;
				pInHand->bAttachStatus[ bAttachPos ] = 0;

				// drop it to ground
				AddItemToPool(pSoldier->sGridNo, &Temp, VISIBLE, pSoldier->bLevel, 0, -1);

				// big penalty to hit
				iChance -= 30;

				// curse!
				if ( pSoldier->bTeam == OUR_TEAM )
				{
					DoMercBattleSound( pSoldier, BATTLE_SOUND_CURSE1 );

					ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(gzLateLocalizedString[STR_LATE_46], pSoldier->name));
				}
			}
		}

		iMaxRange = GunRange(*pInHand);
	}
	else
	{
		iMaxRange = CELL_X_SIZE; // one tile
	}

	if ( iSightRange > 0 )
	{

		if (IsWearingHeadGear(*pSoldier, SUNGOGGLES))
		{
			// decrease effective range by 10% when using sungoggles (w or w/o scope)
			iSightRange -= iRange / 10; //basically, +1% to hit per every 2 squares
		}

		bAttachPos = FindAttachment( pInHand, SNIPERSCOPE );

		// does gun have scope, long range recommends its use, and shooter's aiming?
		if (bAttachPos != NO_SLOT && (iRange > MIN_SCOPE_RANGE) && (ubAimTime > 0))
		{
			// reduce effective sight range by 20% per extra aiming time AP of the distance
			// beyond MIN_SCOPE_RANGE.  Max reduction is 80% of the range beyond.
			iScopeBonus = ((SNIPERSCOPE_AIM_BONUS * ubAimTime) * (iRange - MIN_SCOPE_RANGE)) / 100;

			// adjust for scope condition, only has full affect at 100%
			iScopeBonus = (iScopeBonus * WEAPON_STATUS_MOD(pInHand->bAttachStatus[bAttachPos])) / 100;

			// reduce effective range by the bonus obtained from the scope
			iSightRange = std::max(iSightRange - iScopeBonus, 1);
		}

		bAttachPos = FindAttachment( pInHand, LASERSCOPE );
		if (usInHand == ROCKET_RIFLE || usInHand == AUTO_ROCKET_RIFLE ||
			bAttachPos != NO_SLOT) // rocket rifle has one built in
		{
			INT8 bLaserStatus;

			if ( usInHand == ROCKET_RIFLE || usInHand == AUTO_ROCKET_RIFLE )
			{
				bLaserStatus = WEAPON_STATUS_MOD(pInHand->bGunStatus);
			}
			else
			{
				bLaserStatus = WEAPON_STATUS_MOD(pInHand->bAttachStatus[ bAttachPos ]);
			}

			// laser scope isn't of much use in high light levels; add something for that
			if (bLaserStatus > 50)
			{
				iScopeBonus = LASERSCOPE_BONUS * (bLaserStatus - 50) / 50;
			}
			else
			{
				// laser scope in bad condition creates aim penalty!
				iScopeBonus = - LASERSCOPE_BONUS * (50 - bLaserStatus) / 50;
			}

			iChance += iScopeBonus;

		}

	}

	// if aiming at the head, reduce chance to hit
	if (ubAimPos == AIM_SHOT_HEAD)
	{
		// penalty of 3% per tile
		iPenalty = 3 * iSightRange / 10;
		iChance -= iPenalty;
	}
	else if (ubAimPos == AIM_SHOT_LEGS)
	{
		// penalty of 1% per tile
		iPenalty = iSightRange / 10;
		iChance -= iPenalty;
	}

	// ADJUST FOR RANGE
	// bonus if range is less than normal range, penalty if it's more
	//iChance += (NORMAL_RANGE - iRange) / (CELL_X_SIZE / 5);	// 5% per tile

	// Effects of actual gun max range... the numbers are based on wanting -40%
	// at range 26for a pistol with range 13, and -0 for a sniper rifle with range 80
	iPenalty = ((iMaxRange - iRange * 3) * 10) / (17 * CELL_X_SIZE);
	if ( iPenalty < 0 )
	{
		iChance += iPenalty;
	}
	//iChance -= 20 * iRange / iMaxRange;

	if ( TANK( pSoldier ) && ( iRange / CELL_X_SIZE < MaxDistanceVisible() ) )
	{
		// tank; penalize at close range!
		// 2 percent per tile closer than max visible distance
		iChance -= 2 * ( MaxDistanceVisible() - (iRange / CELL_X_SIZE) );
	}

	if (iSightRange == 0)
	{
		// firing blind!
		iChance -= AIM_PENALTY_BLIND;
	}
	else
	{
		// Effects based on aiming & sight
		// From for JA2.5:  3% bonus/penalty for each tile different from range NORMAL_RANGE.
		// This doesn't provide a bigger bonus at close range, but stretches it out, making medium
		// range less penalized, and longer range more penalized
		iChance += 3 * ( NORMAL_RANGE - iSightRange ) / CELL_X_SIZE;
		/*
		if (iSightRange < NORMAL_RANGE)
		{
			// bonus to hit of 20% at point blank (would be 25% at range 0);
			//at NORMAL_RANGE, bonus is 0
			iChance += 25 * (NORMAL_RANGE - iSightRange) / NORMAL_RANGE;
		}
		else
		{
			// penalty of 2% / tile
			iChance -= (iSightRange - NORMAL_RANGE) / 5;
		}
		*/
	}

	// adjust for roof/not on roof
	if ( pSoldier->bLevel == 0 )
	{
		if ( pSoldier->bTargetLevel > 0 )
		{
			// penalty for firing up
			iChance -= AIM_PENALTY_FIRING_UP;
		}
	}
	else // pSoldier->bLevel > 0 )
	{
		if ( pSoldier->bTargetLevel == 0 )
		{
			iChance += AIM_BONUS_FIRING_DOWN;
		}
		// if have roof trait, give bonus
		iChance += gbSkillTraitBonus[ONROOF] * NUM_SKILL_TRAITS(pSoldier, ONROOF);
	}


	const SOLDIERTYPE* const pTarget = WhoIsThere2(sGridNo, pSoldier->bTargetLevel);
	if (pTarget != NULL)
	{
		// targeting a merc
		// adjust for crouched/prone target
		switch( gAnimControl[ pTarget->usAnimState ].ubHeight )
		{
			case ANIM_CROUCH:
				if ( TANK( pSoldier ) && iRange < MIN_TANK_RANGE )
				{
					// 13% penalty per tile closer than min range
					iChance -= 13 * ( ( MIN_TANK_RANGE - iRange ) / CELL_X_SIZE );
				}
				else
				{
					// at anything other than point-blank range
					if (iRange > POINT_BLANK_RANGE + 10 * (AIM_PENALTY_TARGET_CROUCHED / 3) )
					{
						iChance -= AIM_PENALTY_TARGET_CROUCHED;
					}
					else if (iRange > POINT_BLANK_RANGE)
					{
						// at close range give same bonus as prone, up to maximum of AIM_PENALTY_TARGET_CROUCHED
						iChance -= 3 * ((iRange - POINT_BLANK_RANGE) / CELL_X_SIZE); // penalty -3%/tile
					}
				}
				break;
			case ANIM_PRONE:
				if ( TANK( pSoldier ) && iRange < MIN_TANK_RANGE )
				{
					// 25% penalty per tile closer than min range
					iChance -= 25 * ( ( MIN_TANK_RANGE - iRange ) / CELL_X_SIZE );
				}
				else
				{
					// at anything other than point-blank range
					if (iRange > POINT_BLANK_RANGE)
					{
						// reduce chance to hit with distance to the prone/immersed target
						iPenalty = 3 * ((iRange - POINT_BLANK_RANGE) / CELL_X_SIZE); // penalty -3%/tile
						iPenalty = std::min(iPenalty, AIM_PENALTY_TARGET_PRONE);

						iChance -= iPenalty;
					}
				}
				break;
			case ANIM_STAND:
				// if we are prone and at close range, then penalize shots to the torso or head!
				if ( iRange <= MIN_PRONE_RANGE && gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_PRONE )
				{
					if ( ubAimPos == AIM_SHOT_RANDOM || ubAimPos == AIM_SHOT_GLAND )
					{
						ubAdjAimPos = AIM_SHOT_TORSO;
					}
					else
					{
						ubAdjAimPos = ubAimPos;
					}
					// lose 10% per height difference, lessened by distance
					// e.g. 30% to aim at head at range 1, only 10% at range 3
					// or 20% to aim at torso at range 1, no penalty at range 3
					// NB torso aim position is 2, so (5-aimpos) is 3, for legs it's 2, for head 4
					iChance -= (5 - ubAdjAimPos - iRange / CELL_X_SIZE) * 10;
				}
				break;
			default:
				break;
		}

		// penalty for amount that enemy has moved
		iPenalty = std::min(((pTarget->bTilesMoved * 3) / 2), 30);
		iChance -= iPenalty;

		// if target sees us, he may have a chance to dodge before the gun goes off
		// but ability to dodge is reduced if crouched or prone!
		if (pTarget->bOppList[pSoldier->ubID] == SEEN_CURRENTLY && !TANK( pTarget ) && !(pSoldier->ubBodyType != QUEENMONSTER) )
		{
			iPenalty = ( EffectiveAgility( pTarget ) / 5 + EffectiveExpLevel( pTarget ) * 2);
			switch( gAnimControl[ pTarget->usAnimState ].ubHeight )
			{
				case ANIM_CROUCH:
					iPenalty = iPenalty * 2 / 3;
					break;
				case ANIM_PRONE:
					iPenalty /= 3;
					break;
			}

			// reduce dodge ability by the attacker's stats
			iBonus = ( EffectiveDexterity( pSoldier ) / 5 + EffectiveExpLevel( pSoldier ) * 2);
			if ( TANK( pTarget ) || (pSoldier->ubBodyType != QUEENMONSTER) )
			{
				// reduce ability to track shots
				iBonus = iBonus / 2;
			}

			if ( iPenalty > iBonus )
			{
				iChance -= (iPenalty - iBonus);
			}
		}
	}
	else if ( TANK( pSoldier ) && iRange < MIN_TANK_RANGE )
	{
		// 25% penalty per tile closer than min range
		iChance -= 25 * ( ( MIN_TANK_RANGE - iRange ) / CELL_X_SIZE );
	}

	// IF CHANCE EXISTS, BUT SHOOTER IS INJURED
	if ((iChance > 0) && (pSoldier->bLife < pSoldier->bLifeMax))
	{
		// if bandaged, give 1/2 of the bandaged life points back into equation
		bBandaged = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;

		// injury penalty is based on % damage taken (max 2/3rds chance)
		iPenalty = (iChance * 2 * (pSoldier->bLifeMax - pSoldier->bLife + (bBandaged / 2))) /
						(3 * pSoldier->bLifeMax);

		// reduce injury penalty due to merc's experience level (he can take it!)
		iChance -= (iPenalty * (100 - (10 * ( EffectiveExpLevel( pSoldier ) - 1)))) / 100;
	}

	// IF CHANCE EXISTS, BUT SHOOTER IS LOW ON BREATH
	if ((iChance > 0) && (pSoldier->bBreath < 100))
	{
		// breath penalty is based on % breath missing (max 1/2 chance)
		iPenalty = (iChance * (100 - pSoldier->bBreath)) / 200;
		// reduce breath penalty due to merc's dexterity (he can compensate!)
		iChance -= (iPenalty * (100 - ( EffectiveDexterity( pSoldier ) - 10))) / 100;
	}


	// CHECK IF TARGET IS WITHIN GUN'S EFFECTIVE MAXIMUM RANGE
	if ( iRange > iMaxRange )
	{
		// a bullet WILL travel that far if not blocked, but it's NOT accurate,
		// because beyond maximum range, the bullet drops rapidly

		// This won't cause the bullet to be off to the left or right, only make it
		// drop in flight.
		iChance /= 2;
	}
	if ( iSightRange > (sDistVis * CELL_X_SIZE) )
	{
		// penalize out of sight shots, cumulative to effective range penalty
		iChance /= 2;
	}

	// MAKE SURE CHANCE TO HIT IS WITHIN DEFINED LIMITS
	if (iChance < MINCHANCETOHIT)
	{
		if ( TANK( pSoldier ) )
		{
			// allow absolute minimums
			iChance = 0;
		}
		else
		{
			iChance = MINCHANCETOHIT;
		}
	}
	else
	{
		if (iChance > MAXCHANCETOHIT)
			iChance = MAXCHANCETOHIT;
	}

	return (iChance);
}


UINT32 AICalcChanceToHitGun(SOLDIERTYPE *pSoldier, UINT16 sGridNo, UINT8 ubAimTime, UINT8 ubAimPos )
{
	UINT16 usTrueState;
	UINT32 uiChance;

	// same as CCTHG but fakes the attacker always standing
	usTrueState = pSoldier->usAnimState;
	pSoldier->usAnimState = STANDING;
	uiChance = CalcChanceToHitGun( pSoldier, sGridNo, ubAimTime, ubAimPos, false );
	pSoldier->usAnimState = usTrueState;
	return( uiChance );
}

INT32 CalcBodyImpactReduction( UINT8 ubAmmoType, UINT8 ubHitLocation )
{
	// calculate how much bullets are slowed by passing through someone
	INT32 iReduction = BodyImpactReduction[ubHitLocation];

	switch (ubAmmoType)
	{
		case AMMO_HP:
			iReduction = AMMO_ARMOUR_ADJUSTMENT_HP( iReduction );
			break;
		case AMMO_AP:
		case AMMO_HEAT:
			iReduction = AMMO_ARMOUR_ADJUSTMENT_AP( iReduction );
			break;
		case AMMO_SUPER_AP:
			iReduction = AMMO_ARMOUR_ADJUSTMENT_SAP( iReduction );
			break;
		default:
			break;
	}
	return( iReduction );
}


static INT32 ArmourProtection(SOLDIERTYPE const& pTarget, UINT16 const usItemIndex, INT8* const pbStatus, INT32 const iImpact, UINT8 const ubAmmoType)
{
	INT32 iProtection, iAppliedProtection, iFailure;

	auto armour = GCM->getArmour(usItemIndex);
	if (!armour) {
		return 0;
	}
	iProtection = armour->getProtection();

	if (!AM_A_ROBOT(&pTarget))
	{
		// check for the bullet hitting a weak spot in the armour
		iFailure = PreRandom( 100 ) + 1 - *pbStatus;
		if (iFailure > 0)
		{
			iProtection -= iFailure;
			if (iProtection < 0)
			{
				return( 0 );
			}
		}
	}

	// adjust protection of armour due to different ammo types
	switch (ubAmmoType)
	{
		case AMMO_HP:
			iProtection = AMMO_ARMOUR_ADJUSTMENT_HP( iProtection );
			break;
		case AMMO_AP:
		case AMMO_HEAT:
			iProtection = AMMO_ARMOUR_ADJUSTMENT_AP( iProtection );
			break;
		case AMMO_SUPER_AP:
			iProtection = AMMO_ARMOUR_ADJUSTMENT_SAP( iProtection );
			break;
		default:
			break;
	}

	// figure out how much of the armour's protection value is necessary
	// in defending against this bullet
	if (iProtection > iImpact )
	{
		iAppliedProtection = iImpact;
	}
	else
	{
		// applied protection is the full strength of the armour, before AP/HP changes
		iAppliedProtection = armour->getProtection();
	}

	// reduce armour condition

	if (ubAmmoType == AMMO_KNIFE || ubAmmoType == AMMO_SLEEP_DART)
	{
		// knives and darts damage armour but are not stopped by kevlar
		if (armour->getArmourClass() == ARMOURCLASS_VEST ||
			armour->getArmourClass() == ARMOURCLASS_LEGGINGS)
		{
			iProtection = 0;
		}
	}
	else if (ubAmmoType == AMMO_MONSTER)
	{
		// creature spit damages armour a lot! an extra 3x for a total of 4x normal
		*pbStatus -= 3 * (iAppliedProtection * armour->getDegradePercentage()) / 100;

		// reduce amount of protection from armour
		iProtection /= 2;
	}

	if (!AM_A_ROBOT(&pTarget))
	{
		*pbStatus -= (iAppliedProtection * armour->getDegradePercentage()) / 100;
	}

	// return armour protection
	return( iProtection );
}


INT32 TotalArmourProtection(SOLDIERTYPE& pTarget, const UINT8 ubHitLocation, const INT32 iImpact, const UINT8 ubAmmoType)
{
	INT32      iTotalProtection = 0, iSlot;
	OBJECTTYPE *pArmour;
	INT8       bPlatePos = -1;

	if (pTarget.uiStatusFlags & SOLDIER_VEHICLE)
	{
		INT8 bDummyStatus = 100;
		iTotalProtection += ArmourProtection(pTarget, GetVehicleArmour(pTarget.bVehicleID), &bDummyStatus, iImpact, ubAmmoType);
	}
	else
	{
		switch( ubHitLocation )
		{
			case AIM_SHOT_GLAND:
				// creature hit in the glands!!! no armour there!
				return( 0 );
			case AIM_SHOT_HEAD:
				iSlot = HELMETPOS;
				break;
			case AIM_SHOT_LEGS:
				iSlot = LEGPOS;
				break;
			case AIM_SHOT_TORSO:
			default:
				iSlot = VESTPOS;
				break;

		}

		pArmour = &pTarget.inv[iSlot];
		if (pArmour->usItem != NOTHING)
		{
			// check plates first
			if ( iSlot == VESTPOS )
			{
				bPlatePos = FindPlatesAttachment(pArmour);
				if (bPlatePos != -1)
				{
					// bullet got through jacket; apply ceramic plate armour
					iTotalProtection += ArmourProtection(pTarget, pArmour->usAttachItem[bPlatePos], &(pArmour->bAttachStatus[bPlatePos]), iImpact, ubAmmoType);
					if ( pArmour->bAttachStatus[bPlatePos] < USABLE )
					{
						// destroy plates!
						pArmour->usAttachItem[ bPlatePos ] = NOTHING;
						pArmour->bAttachStatus[ bPlatePos ] = 0;
						DirtyMercPanelInterface(&pTarget, DIRTYLEVEL2);
						if (pTarget.bTeam == OUR_TEAM)
						{
							// report plates destroyed!
							ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(str_ceramic_plates_smashed, pTarget.name));
						}
					}
				}
			}

			// if the plate didn't stop the bullet...
			if ( iImpact > iTotalProtection )
			{
				iTotalProtection += ArmourProtection( pTarget, pArmour->usItem, &(pArmour->bStatus[0]), iImpact, ubAmmoType );
				if ( pArmour->bStatus[ 0 ] < USABLE )
				{
					DeleteObj( pArmour );
					DirtyMercPanelInterface(&pTarget, DIRTYLEVEL2);
				}
			}
		}
	}
	return( iTotalProtection );
}

INT32 BulletImpact( SOLDIERTYPE *pFirer, SOLDIERTYPE * pTarget, UINT8 ubHitLocation, INT32 iOrigImpact, INT16 sHitBy, UINT8 * pubSpecial )
{
	INT32 iImpact, iFluke, iBonus, iImpactForCrits = 0;
	INT8  bStatLoss;
	UINT8 ubAmmoType;

	// NOTE: reduction of bullet impact due to range and obstacles is handled
	// in MoveBullet.

	// Set a few things up:
	if ( GCM->getItem(pFirer->usAttackingWeapon)->getItemClass() == IC_THROWING_KNIFE )
	{
		ubAmmoType = AMMO_KNIFE;
	}
	else
	{
		ubAmmoType = pFirer->inv[pFirer->ubAttackingHand].ubGunAmmoType;
	}

	if ( TANK( pTarget ) )
	{
		if ( ubAmmoType != AMMO_HEAT )
		{
			// ping!
			return( 0 );
		}
	}

	// plus/minus up to 25% due to "random" factors (major organs hit or missed,
	// lucky lighter in breast pocket, divine intervention on behalf of "Rev"...)
	iFluke = PreRandom(51) - 25; // gives (0 to 50 -25) -> -25% to +25%

	// up to 50% extra impact for making particularly accurate successful shots
	iBonus = sHitBy / 2;

	iOrigImpact = iOrigImpact * (100 + iFluke + iBonus) / 100;

	// at very long ranges (1.5x maxRange and beyond) impact could go negative
	if (iOrigImpact < 1)
	{
		iOrigImpact = 1; // raise impact to a minimum of 1 for any hit
	}

	// adjust for HE rounds
	if (ubAmmoType == AMMO_HE || ubAmmoType == AMMO_HEAT)
	{
		iOrigImpact = AMMO_DAMAGE_ADJUSTMENT_HE( iOrigImpact );

		if ( TANK( pTarget ) )
		{
			// HEAT round on tank, divide by 3 for damage
			iOrigImpact /= 2;
		}
	}

	if (pubSpecial && *pubSpecial == FIRE_WEAPON_BLINDED_BY_SPIT_SPECIAL)
	{
		iImpact = iOrigImpact;
	}
	else
	{
		iImpact = iOrigImpact - TotalArmourProtection(*pTarget, ubHitLocation, iOrigImpact, ubAmmoType);
	}

	// calc minimum damage
	if (ubAmmoType == AMMO_HP || ubAmmoType == AMMO_SLEEP_DART)
	{
		if (iImpact < 0)
		{
			iImpact = 0;
		}
	}
	else
	{
		if (iImpact < ((iOrigImpact + 5) / 10) )
		{
			iImpact = (iOrigImpact + 5) / 10;
		}

		if ( (ubAmmoType == AMMO_BUCKSHOT) && (pTarget->bNumPelletsHitBy > 0) )
		{
			iImpact += (pTarget->bNumPelletsHitBy - 1)  / 2;
		}

	}

	if (gfNextShotKills)
	{
		// big time cheat key effect!
		iImpact = 100;
		gfNextShotKills = FALSE;
	}

	if ( iImpact > 0 && !TANK( pTarget ) )
	{

		if ( ubAmmoType == AMMO_SLEEP_DART && sHitBy > 20 )
		{
			if (pubSpecial)
			{
				*pubSpecial = FIRE_WEAPON_SLEEP_DART_SPECIAL;
			}
			return( iImpact );
		}

		if (ubAmmoType == AMMO_HP)
		{ // good solid hit with a hollow-point bullet, which got through armour!
			iImpact = AMMO_DAMAGE_ADJUSTMENT_HP( iImpact );
		}

		AdjustImpactByHitLocation( iImpact, ubHitLocation, &iImpact, &iImpactForCrits );

		switch( ubHitLocation )
		{
			case AIM_SHOT_HEAD:
				// is the blow deadly enough for an instant kill?
				if ( PythSpacesAway( pFirer->sGridNo, pTarget->sGridNo ) <= MAX_DISTANCE_FOR_MESSY_DEATH )
				{
					if (iImpactForCrits > MIN_DAMAGE_FOR_INSTANT_KILL && iImpactForCrits < pTarget->bLife)
					{
						// blow to the head is so deadly that it causes instant death;
						// the target has more life than iImpact so we increase it
						iImpact = pTarget->bLife + Random( 10 );
						iImpactForCrits = iImpact;
					}

					if (pubSpecial)
					{
						// is the blow deadly enough to cause a head explosion?
						if ( iImpactForCrits >= pTarget->bLife )
						{
							if (iImpactForCrits > MIN_DAMAGE_FOR_HEAD_EXPLOSION )
							{
								*pubSpecial = FIRE_WEAPON_HEAD_EXPLODE_SPECIAL;
							}
							else if ( iImpactForCrits > (MIN_DAMAGE_FOR_HEAD_EXPLOSION / 2) && ( PreRandom( MIN_DAMAGE_FOR_HEAD_EXPLOSION / 2 ) < (UINT32)(iImpactForCrits - MIN_DAMAGE_FOR_HEAD_EXPLOSION / 2) ) )
							{
								*pubSpecial = FIRE_WEAPON_HEAD_EXPLODE_SPECIAL;
							}
						}

					}
				}
				break;
			case AIM_SHOT_LEGS:
				// is the damage enough to make us fall over?
				if (pubSpecial && IS_MERC_BODY_TYPE(pTarget) && gAnimControl[pTarget->usAnimState].ubEndHeight == ANIM_STAND && !MercInWater(pTarget))
				{
					if (iImpactForCrits > MIN_DAMAGE_FOR_AUTO_FALL_OVER )
					{
						*pubSpecial = FIRE_WEAPON_LEG_FALLDOWN_SPECIAL;
					}
					// else ramping up chance from 1/2 the automatic value onwards
					else if ( iImpactForCrits > (MIN_DAMAGE_FOR_AUTO_FALL_OVER / 2) && ( PreRandom( MIN_DAMAGE_FOR_AUTO_FALL_OVER / 2 ) < (UINT32)(iImpactForCrits - MIN_DAMAGE_FOR_AUTO_FALL_OVER / 2) ) )
					{
						*pubSpecial = FIRE_WEAPON_LEG_FALLDOWN_SPECIAL;
					}
				}
				break;
			case AIM_SHOT_TORSO:
				// normal damage to torso
				// is the blow deadly enough for an instant kill?
				// since this value is much lower than the others, it only applies at short range...
				if ( PythSpacesAway( pFirer->sGridNo, pTarget->sGridNo ) <= MAX_DISTANCE_FOR_MESSY_DEATH )
				{
					if (iImpact > MIN_DAMAGE_FOR_INSTANT_KILL && iImpact < pTarget->bLife)
					{
						// blow to the chest is so deadly that it causes instant death;
						// the target has more life than iImpact so we increase it
						iImpact = pTarget->bLife + Random( 10 );
						iImpactForCrits = iImpact;
					}
					// special thing for hitting chest - allow cumulative damage to count
					else if ( (iImpact + pTarget->sDamage) > (MIN_DAMAGE_FOR_BLOWN_AWAY + MIN_DAMAGE_FOR_INSTANT_KILL) )
					{
						iImpact = pTarget->bLife + Random( 10 );
						iImpactForCrits = iImpact;
					}

					// is the blow deadly enough to cause a chest explosion?
					if (pubSpecial)
					{
						if (iImpact > MIN_DAMAGE_FOR_BLOWN_AWAY && iImpact >= pTarget->bLife)
						{
							*pubSpecial = FIRE_WEAPON_CHEST_EXPLODE_SPECIAL;
						}
					}
				}
				break;
		}
	}

	if ( AM_A_ROBOT( pTarget ) )
	{
		iImpactForCrits = 0;
	}

	// don't do critical hits against people who are gonna die!
	if( !IsAutoResolveActive() )
	{

		if ( ubAmmoType == AMMO_KNIFE && pFirer->bOppList[ pTarget->ubID ] == SEEN_CURRENTLY )
		{
			// is this a stealth attack?
			if ( pTarget->bOppList[ pFirer->ubID ] == NOT_HEARD_OR_SEEN && !CREATURE_OR_BLOODCAT( pTarget ) && (ubHitLocation == AIM_SHOT_HEAD || ubHitLocation == AIM_SHOT_TORSO ) )
			{
				if ( PreRandom( 100 ) < (UINT32)(sHitBy + 10 * NUM_SKILL_TRAITS( pFirer, THROWING )) )
				{
					// instant death!
					iImpact = pTarget->bLife + Random( 10 );
					iImpactForCrits = iImpact;
				}
			}
		}

		if (iImpactForCrits > 0 && iImpactForCrits < pTarget->bLife )
		{
			if (PreRandom( iImpactForCrits / 2 + pFirer->bAimTime * 5) + 1 > CRITICAL_HIT_THRESHOLD)
			{
				bStatLoss = (INT8) PreRandom( iImpactForCrits / 2 ) + 1;
				switch( ubHitLocation )
				{
					case AIM_SHOT_HEAD:
						if (bStatLoss >= pTarget->bWisdom)
						{
							bStatLoss = pTarget->bWisdom - 1;
						}
						if ( bStatLoss > 0 )
						{
							pTarget->bWisdom -= bStatLoss;

							if (pTarget->ubProfile != NO_PROFILE)
							{
								gMercProfiles[ pTarget->ubProfile ].bWisdom = pTarget->bWisdom;
								gMercProfiles[ pTarget->ubProfile ].bWisdomDelta -= bStatLoss;
							}


							if (pTarget->name[0] && pTarget->bVisible == TRUE)
							{
								// make stat RED for a while...
								pTarget->uiChangeWisdomTime = GetJA2Clock();
								pTarget->usValueGoneUp &= ~( WIS_INCREASE );

								if (bStatLoss == 1)
								{
									ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(g_langRes->Message[STR_LOSES_1_WISDOM], pTarget->name) );
								}
								else
								{
									ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(g_langRes->Message[STR_LOSES_WISDOM], pTarget->name, bStatLoss) );
								}
							}
						}
						else if ( pTarget->bNumPelletsHitBy == 0 )
						{
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(g_langRes->Message[STR_HEAD_HIT], pTarget->name) );
						}
						break;
					case AIM_SHOT_TORSO:
						if (PreChance(50) && !(pTarget->uiStatusFlags & SOLDIER_MONSTER))
						{
							if (bStatLoss >= pTarget->bDexterity)
							{
								bStatLoss = pTarget->bDexterity - 1;
							}
							if ( bStatLoss > 0 )
							{
								pTarget->bDexterity -= bStatLoss;

								if (pTarget->ubProfile != NO_PROFILE)
								{
									gMercProfiles[ pTarget->ubProfile ].bDexterity = pTarget->bDexterity;
									gMercProfiles[ pTarget->ubProfile ].bDexterityDelta -= bStatLoss;
								}

								if (pTarget->name[0] && pTarget->bVisible == TRUE)
								{
									// make stat RED for a while...
									pTarget->uiChangeDexterityTime = GetJA2Clock();
									pTarget->usValueGoneUp &= ~( DEX_INCREASE );

									if (bStatLoss == 1)
									{
										ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(g_langRes->Message[STR_LOSES_1_DEX], pTarget->name) );
									}
									else
									{
										ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(g_langRes->Message[STR_LOSES_DEX], pTarget->name, bStatLoss) );
									}
								}
							}
						}
						else
						{
							if (bStatLoss >= pTarget->bStrength)
							{
								bStatLoss = pTarget->bStrength - 1;
							}
							if ( bStatLoss > 0 )
							{
								pTarget->bStrength -= bStatLoss;

								if (pTarget->ubProfile != NO_PROFILE)
								{
									gMercProfiles[ pTarget->ubProfile ].bStrength = pTarget->bStrength;
									gMercProfiles[ pTarget->ubProfile ].bStrengthDelta -= bStatLoss;
								}

								if (pTarget->name[0] && pTarget->bVisible == TRUE)
								{
									// make stat RED for a while...
									pTarget->uiChangeStrengthTime = GetJA2Clock();
									pTarget->usValueGoneUp &= ~( STRENGTH_INCREASE );

									if (bStatLoss == 1)
									{
										ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(g_langRes->Message[STR_LOSES_1_STRENGTH], pTarget->name) );
									}
									else
									{
										ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(g_langRes->Message[STR_LOSES_STRENGTH], pTarget->name, bStatLoss) );
									}
								}
							}
						}
						break;
					case AIM_SHOT_LEGS:
						if (bStatLoss >= pTarget->bAgility)
						{
							bStatLoss = pTarget->bAgility - 1;
						}
						if ( bStatLoss > 0 )
						{
							pTarget->bAgility -= bStatLoss;

							if (pTarget->ubProfile != NO_PROFILE)
							{
								gMercProfiles[ pTarget->ubProfile ].bAgility = pTarget->bAgility;
								gMercProfiles[ pTarget->ubProfile ].bAgilityDelta -= bStatLoss;
							}

							if (pTarget->name[0] && pTarget->bVisible == TRUE)
							{
								// make stat RED for a while...
								pTarget->uiChangeAgilityTime = GetJA2Clock();
								pTarget->usValueGoneUp &= ~( AGIL_INCREASE );

								if (bStatLoss == 1)
								{
									ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(g_langRes->Message[STR_LOSES_1_AGIL], pTarget->name) );
								}
								else
								{
									ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(g_langRes->Message[STR_LOSES_AGIL], pTarget->name, bStatLoss) );
								}
							}
						}
						break;
				}
			}
			else if ( ubHitLocation == AIM_SHOT_HEAD && pTarget->bNumPelletsHitBy == 0 )
			{
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(g_langRes->Message[STR_HEAD_HIT], pTarget->name) );
			}
		}
	}

	return( iImpact );
}


INT32 HTHImpact(const SOLDIERTYPE* const att, const SOLDIERTYPE* const tgt, const INT32 iHitBy, const BOOLEAN fBladeAttack)
{
	INT32        impact   = EffectiveExpLevel(att) / 2; // 0 to 4 for level
	const INT8   strength = EffectiveStrength(att);
	const UINT16 weapon   = att->usAttackingWeapon;
	if (fBladeAttack)
	{
		impact += strength / 20; // 0 to 5 for strength, adjusted by damage taken
		impact += GCM->getWeapon(weapon)->ubImpact;

		if (AM_A_ROBOT(tgt)) impact /= 4;
	}
	else
	{
		impact += strength / 5; // 0 to 20 for strength, adjusted by damage taken

		// NB martial artists don't get a bonus for using brass knuckles!
		if (weapon && !HAS_SKILL_TRAIT(att, MARTIALARTS))
		{
			impact += GCM->getWeapon(weapon)->ubImpact;
			if (AM_A_ROBOT(tgt)) impact /= 2;
		}
		else
		{
			// base HTH damage
			impact += 5;
			if (AM_A_ROBOT(tgt)) impact = 0;
		}
	}

	const INT32 fluke = PreRandom(51) - 25; // +/-25% bonus due to random factors
	const INT32 bonus = iHitBy / 2;         // up to 50% extra impact for accurate attacks
	impact = impact * (100 + fluke + bonus) / 100;

	if (!fBladeAttack)
	{
		// add bonuses for hand-to-hand and martial arts
		if (HAS_SKILL_TRAIT(att, MARTIALARTS))
		{
			impact = impact * (100 + gbSkillTraitBonus[MARTIALARTS] * NUM_SKILL_TRAITS(att, MARTIALARTS)) / 100;
			if (att->usAnimState == NINJA_SPINKICK) impact *= 2;
		}
		// SPECIAL  - give TRIPLE bonus for damage for hand-to-hand trait
		// because the HTH bonus is half that of martial arts, and gets only 1x for to-hit bonus
		impact = impact * (100 + 3 * gbSkillTraitBonus[HANDTOHAND] * NUM_SKILL_TRAITS(att, HANDTOHAND)) / 100;
	}

	return impact;
}


void ShotMiss(const BULLET* const b)
{
	SOLDIERTYPE* const pAttacker = b->pFirer;
	SOLDIERTYPE* const opponent = pAttacker->opponent;
	// AGILITY GAIN: Opponent "dodged" a bullet shot at him (it missed)
	if (opponent != NULL) AgilityForEnemyMissingPlayer(pAttacker, opponent, 5);

	switch (GCM->getWeapon(pAttacker->usAttackingWeapon)->ubWeaponClass)
	{
		case HANDGUNCLASS:
		case RIFLECLASS:
		case SHOTGUNCLASS:
		case SMGCLASS:
		case MGCLASS:
			// Guy has missed, play random sound
			if (pAttacker->bTeam == OUR_TEAM && Random(40) == 0)
			{
				DoMercBattleSound(pAttacker, BATTLE_SOUND_CURSE1);
			}

			// PLAY SOUND AND FLING DEBRIS
			// RANDOMIZE SOUND SYSTEM
			if (!DoSpecialEffectAmmoMiss(pAttacker, NOWHERE, 0, 0, 0, TRUE, TRUE, NULL))
			{
				PlayJA2Sample(SoundRange<MISS_1, MISS_8>(), HIGHVOLUME, 1, MIDDLEPAN);
			}

			// ATE: Show misses...( if our team )
			if (gGameSettings.fOptions[TOPTION_SHOW_MISSES] &&
				pAttacker->bTeam == OUR_TEAM)
			{
				LocateGridNo(b->sGridNo);
			}
			break;

		case MONSTERCLASS:
			PlayJA2Sample(SPIT_RICOCHET, HIGHVOLUME, 1, MIDDLEPAN);
			break;
	}

	SLOGD("Freeing up attacker - bullet missed");
	FreeUpAttacker(pAttacker);
}


static UINT32 CalcChanceHTH(SOLDIERTYPE* pAttacker, SOLDIERTYPE* pDefender, UINT8 ubAimTime, UINT8 ubMode, bool skipSafetyCheck = false)
{
	UINT16 usInHand;
	UINT8  ubBandaged;
	INT32  iAttRating, iDefRating;
	INT32  iChance;

	usInHand = pAttacker->usAttackingWeapon;

	if ( (usInHand != CREATURE_QUEEN_TENTACLES ) && (pDefender->bLife < OKLIFE || pDefender->bBreath < OKBREATH) )
	{
		// there is NO way to miss
		return( 100 );
	}

	if (!skipSafetyCheck) // Added this check so we can get punch calculation without "punch item" equipped
	{
		if (ubMode == HTH_MODE_STAB)
		{
			// safety check
			if (GCM->getWeapon(usInHand)->ubWeaponClass != KNIFECLASS)
			{
				return(0);
			}
		}
		else
		{
			if ( GCM->getItem(usInHand)->getItemClass() != IC_PUNCH )
			{
				return(0);
			}
		}
	}

	// CALCULATE ATTACKER'S CLOSE COMBAT RATING (1-100)
	if (ubMode == HTH_MODE_STEAL)
	{
		// this is more of a brute force strength-vs-strength check
		iAttRating = (EffectiveDexterity(pAttacker) + // coordination, accuracy
				EffectiveAgility(pAttacker) +    // speed & reflexes
				3 * pAttacker->bStrength +    // physical strength (TRIPLED!)
				(10 * EffectiveExpLevel(pAttacker)));  // experience, knowledge
	}
	else
	{
		iAttRating = (3 * EffectiveDexterity(pAttacker) + // coordination, accuracy (TRIPLED!)
				EffectiveAgility(pAttacker) +    // speed & reflexes
				pAttacker->bStrength +    // physical strength
				(10 * EffectiveExpLevel(pAttacker)));  // experience, knowledge
	}

	iAttRating /= 6;  // convert from 6-600 to 1-100

	// psycho bonus
	if ( pAttacker->ubProfile != NO_PROFILE && gMercProfiles[ pAttacker->ubProfile ].bPersonalityTrait == PSYCHO )
	{
		iAttRating += AIM_BONUS_PSYCHO;
	}

	// modify chance to hit by morale
	iAttRating += GetMoraleModifier( pAttacker );

	// modify for fatigue
	iAttRating -= GetSkillCheckPenaltyForFatigue( pAttacker, iAttRating );

	// if attacker spent some extra time aiming
	if (ubAimTime)
	{
		// use only HALF of the normal aiming bonus for knife aiming.
		// since there's no range penalty, the bonus is otherwise too generous
		iAttRating += ((AIM_BONUS_PER_AP * ubAimTime) / 2);    //bonus for aiming
	}

	if (! (pAttacker->uiStatusFlags & SOLDIER_PC) )   // if attacker is a computer AI controlled enemy
	{
		iAttRating += gbDiff[ DIFF_ENEMY_TO_HIT_MOD ][ SoldierDifficultyLevel( pAttacker ) ];
	}

	// if attacker is being affected by gas
	if ( pAttacker->uiStatusFlags & SOLDIER_GASSED )
		iAttRating -= AIM_PENALTY_GASSED;

	// if attacker is being bandaged at the same time, his concentration is off
	if (pAttacker->ubServiceCount > 0)
		iAttRating -= AIM_PENALTY_GETTINGAID;

	// if attacker is still in shock
	if (pAttacker->bShock)
		iAttRating -= (pAttacker->bShock * AIM_PENALTY_PER_SHOCK);

	// If attacker injured, reduce chance accordingly (by up to 2/3rds)
	if ((iAttRating > 0) && (pAttacker->bLife < pAttacker->bLifeMax))
	{
		// if bandaged, give 1/2 of the bandaged life points back into equation
		ubBandaged = pAttacker->bLifeMax - pAttacker->bLife - pAttacker->bBleeding;

		iAttRating -= (2 * iAttRating * (pAttacker->bLifeMax - pAttacker->bLife + (ubBandaged / 2))) /
				(3 * pAttacker->bLifeMax);
	}

	// If attacker tired, reduce chance accordingly (by up to 1/2)
	if ((iAttRating > 0) && (pAttacker->bBreath < 100))
		iAttRating -= (iAttRating * (100 - pAttacker->bBreath)) / 200;

	if (pAttacker->ubProfile != NO_PROFILE)
	{
		if (ubMode == HTH_MODE_STAB)
		{
			iAttRating += gbSkillTraitBonus[KNIFING] * NUM_SKILL_TRAITS(pAttacker, KNIFING);
		}
		else
		{
			// add bonuses for hand-to-hand and martial arts
			iAttRating += gbSkillTraitBonus[MARTIALARTS] * NUM_SKILL_TRAITS(pAttacker, MARTIALARTS);
			iAttRating += gbSkillTraitBonus[HANDTOHAND]  * NUM_SKILL_TRAITS(pAttacker, HANDTOHAND);
		}
	}


	if (iAttRating < 1)
		iAttRating = 1;


	// CALCULATE DEFENDER'S CLOSE COMBAT RATING (0-100)
	if (ubMode == HTH_MODE_STEAL)
	{
		iDefRating = (EffectiveAgility( pDefender )) +   // speed & reflexes
			EffectiveDexterity( pDefender ) +  // coordination, accuracy
			3 * pDefender->bStrength +    // physical strength (TRIPLED!)
			(10 * EffectiveExpLevel( pDefender ) );  // experience, knowledge
	}
	else
	{
		iDefRating = (3 * EffectiveAgility( pDefender ) ) +   // speed & reflexes (TRIPLED!)
			EffectiveDexterity( pDefender ) +  // coordination, accuracy
			pDefender->bStrength +    // physical strength
			(10 * EffectiveExpLevel( pDefender ) );  // experience, knowledge
	}

	iDefRating /= 6;  // convert from 6-600 to 1-100

	// modify chance to dodge by morale
	iDefRating += GetMoraleModifier( pDefender );

	// modify for fatigue
	iDefRating -= GetSkillCheckPenaltyForFatigue( pDefender, iDefRating );

	// if attacker is being affected by gas
	if ( pDefender->uiStatusFlags & SOLDIER_GASSED )
		iDefRating -= AIM_PENALTY_GASSED;

	// if defender is being bandaged at the same time, his concentration is off
	if (pDefender->ubServiceCount > 0)
		iDefRating -= AIM_PENALTY_GETTINGAID;

	// if defender is still in shock
	if (pDefender->bShock)
		iDefRating -= (pDefender->bShock * AIM_PENALTY_PER_SHOCK);

	// If defender injured, reduce chance accordingly (by up to 2/3rds)
	if ((iDefRating > 0) && (pDefender->bLife < pDefender->bLifeMax))
	{
		// if bandaged, give 1/2 of the bandaged life points back into equation
		ubBandaged = pDefender->bLifeMax - pDefender->bLife - pDefender->bBleeding;

		iDefRating -= (2 * iDefRating * (pDefender->bLifeMax - pDefender->bLife + (ubBandaged / 2))) /
		(3 * pDefender->bLifeMax);

	}

	// If defender tired, reduce chance accordingly (by up to 1/2)
	if ((iDefRating > 0) && (pDefender->bBreath < 100))
		iDefRating -= (iDefRating * (100 - pDefender->bBreath)) / 200;

	if ((usInHand == CREATURE_QUEEN_TENTACLES && pDefender->ubBodyType == LARVAE_MONSTER) ||
		pDefender->ubBodyType == INFANT_MONSTER)
	{
		// try to prevent queen from killing the kids, ever!
		iDefRating += 10000;
	}

	if (gAnimControl[ pDefender->usAnimState ].ubEndHeight < ANIM_STAND)
	{
		if (usInHand == CREATURE_QUEEN_TENTACLES)
		{
			if ( gAnimControl[ pDefender->usAnimState ].ubEndHeight == ANIM_PRONE )
			{
				// make it well-nigh impossible to hit someone who is prone!
				iDefRating += 1000;
			}
			else
			{
				iDefRating += BAD_DODGE_POSITION_PENALTY * 2;
			}
		}
		else
		{
			// if defender crouched, reduce chance accordingly (harder to dodge)
			iDefRating -= BAD_DODGE_POSITION_PENALTY;
			// If our target is prone, double the penalty!
			if ( gAnimControl[ pDefender->usAnimState ].ubEndHeight == ANIM_PRONE )
			{
				iDefRating -= BAD_DODGE_POSITION_PENALTY;
			}
		}
	}


	if (pDefender->ubProfile != NO_PROFILE)
	{
		if (ubMode == HTH_MODE_STAB)
		{
			if (GCM->getItem(pDefender->inv[HANDPOS].usItem)->getItemClass() == IC_BLADE)
			{
				// good with knives, got one, so we're good at parrying
				iDefRating += gbSkillTraitBonus[KNIFING] * NUM_SKILL_TRAITS(pDefender, KNIFING);
				// the knife gets in the way but we're still better than nobody
				iDefRating += gbSkillTraitBonus[MARTIALARTS] * NUM_SKILL_TRAITS(pDefender, MARTIALARTS) / 3;
			}
			else
			{
				// good with knives, don't have one, but we know a bit about dodging
				iDefRating += gbSkillTraitBonus[KNIFING]     * NUM_SKILL_TRAITS(pDefender, KNIFING)     / 3;
				// bonus for dodging knives
				iDefRating += gbSkillTraitBonus[MARTIALARTS] * NUM_SKILL_TRAITS(pDefender, MARTIALARTS) / 2;
			}
		}
		else
		{	// punch/hand-to-hand/martial arts attack/steal
			if (GCM->getItem(pDefender->inv[HANDPOS].usItem)->getItemClass() == IC_BLADE && ubMode != HTH_MODE_STEAL)
			{
				// with our knife, we get some bonus at defending from HTH attacks
				iDefRating += gbSkillTraitBonus[KNIFING] * NUM_SKILL_TRAITS(pDefender, KNIFING) / 2;
			}
			else
			{
				iDefRating += gbSkillTraitBonus[MARTIALARTS] * NUM_SKILL_TRAITS(pDefender, MARTIALARTS);
				iDefRating += gbSkillTraitBonus[HANDTOHAND]  * NUM_SKILL_TRAITS(pDefender, HANDTOHAND);
			}
		}
	}

	if (iDefRating < 1)
		iDefRating = 1;
	// calculate chance to hit by comparing the 2 opponent's ratings
	//  iChance = (100 * iAttRating) / (iAttRating + iDefRating);


	if (ubMode == HTH_MODE_STEAL)
	{
		// make this more extreme so that weak people have a harder time stealing from
		// the stronger
		iChance = 50 * iAttRating / iDefRating;
	}
	else
	{
		// Changed from DG by CJC to give higher chances of hitting with a stab or punch
		iChance = 67 + (iAttRating - iDefRating) / 3;

		if ( pAttacker->bAimShotLocation == AIM_SHOT_HEAD )
		{
			// make this harder!
			iChance -= 20;
		}

	}


	// MAKE SURE CHANCE TO HIT IS WITHIN DEFINED LIMITS
	if (iChance < MINCHANCETOHIT)
	{
		iChance = MINCHANCETOHIT;
	}
	else
	{
		if (iChance > MAXCHANCETOHIT)
			iChance = MAXCHANCETOHIT;
	}
	return (iChance);
}

UINT32 CalcChanceToStab(SOLDIERTYPE * pAttacker,SOLDIERTYPE *pDefender, UINT8 ubAimTime)
{
	return( CalcChanceHTH( pAttacker, pDefender, ubAimTime, HTH_MODE_STAB ) );
}

UINT32 CalcChanceToPunch(SOLDIERTYPE *pAttacker, SOLDIERTYPE * pDefender, UINT8 ubAimTime, bool skipSafetyCheck)
{
	return( CalcChanceHTH( pAttacker, pDefender, ubAimTime, HTH_MODE_PUNCH, skipSafetyCheck) );
}


static UINT32 CalcChanceToSteal(SOLDIERTYPE* pAttacker, SOLDIERTYPE* pDefender, UINT8 ubAimTime)
{
	return( CalcChanceHTH( pAttacker, pDefender, ubAimTime, HTH_MODE_STEAL ) );
}


void ReloadWeapon(SOLDIERTYPE* const s, UINT8 const inv_pos)
{
	// NB this is a cheat function, don't award experience
	OBJECTTYPE& o = s->inv[inv_pos];
	if (o.usItem == NOTHING) return;

	o.ubGunShotsLeft = GCM->getWeapon(o.usItem)->ubMagSize;
	DirtyMercPanelInterface(s, DIRTYLEVEL1);
}


bool IsGunBurstCapable(SOLDIERTYPE const* const s, UINT8 const inv_pos)
{
	UINT16 const item = s->inv[inv_pos].usItem;
	return GCM->getItem(item)->isWeapon() &&
		GCM->getWeapon(item)->ubShotsPerBurst > 0;
}


INT32 CalcMaxTossRange(const SOLDIERTYPE* pSoldier, UINT16 usItem, BOOLEAN fArmed)
{
	INT32  iRange;
	UINT16 usSubItem;

	if ( EXPLOSIVE_GUN( usItem ) )
	{
		// oops! return value in weapons table
		return( GCM->getWeapon( usItem )->usRange / CELL_X_SIZE );
	}

	// if item's fired mechanically
	// ATE: If we are sent in a LAUNCHABLE, get the LAUCNHER, and sub ONLY if we are armed...
	usSubItem = GetLauncherFromLaunchable( usItem );

	if ( fArmed && usSubItem != NOTHING )
	{
		usItem = usSubItem;
	}

	if ( GCM->getItem(usItem)->getItemClass() == IC_LAUNCHER && fArmed )
	{
		// this function returns range in tiles so, stupidly, we have to divide by 10 here
		iRange = GCM->getWeapon(usItem)->usRange / CELL_X_SIZE;
	}
	else
	{
		if ( GCM->getItem(usItem)->getFlags() & ITEM_UNAERODYNAMIC )
		{
			iRange = 1;
		}
		else if ( GCM->getItem(usItem)->getItemClass() == IC_GRENADE )
		{
			// start with the range based on the soldier's strength and the item's weight
			INT32 iThrowingStrength = ( EffectiveStrength( pSoldier ) * 2 + 100 ) / 3;
			iRange = 2 + ( iThrowingStrength / std::min(( 3 + (GCM->getItem(usItem)->getWeight()) / 3 ), 4 ));
		}
		else
		{	// not as aerodynamic!

			// start with the range based on the soldier's strength and the item's weight
			iRange = 2 + ( ( EffectiveStrength( pSoldier ) / ( 5 + GCM->getItem(usItem)->getWeight()) ) );
		}

		// adjust for thrower's remaining breath (lose up to 1/2 of range)
		iRange -= (iRange * (100 - pSoldier->bBreath)) / 200;

		// better max range due to expertise
		iRange = iRange * (100 + gbSkillTraitBonus[THROWING] * NUM_SKILL_TRAITS(pSoldier, THROWING)) / 100;
	}

	if (iRange < 1)
	{
		iRange = 1;
	}

	return( iRange );
}


UINT32 CalcThrownChanceToHit(SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubAimTime, UINT8 ubAimPos )
{
	INT32  iChance, iMaxRange, iRange;
	UINT16 usHandItem;
	INT8   bPenalty, bBandaged;

	if ( pSoldier->bWeaponMode == WM_ATTACHED)
	{
		usHandItem = UNDER_GLAUNCHER;
	}
	else
	{
		usHandItem = pSoldier->inv[HANDPOS].usItem;
	}

	if ( GCM->getItem(usHandItem)->getItemClass() != IC_LAUNCHER && pSoldier->bWeaponMode != WM_ATTACHED )
	{
		// PHYSICALLY THROWN arced projectile (ie. grenade)
		// for lack of anything better, base throwing accuracy on dex & marskmanship
		iChance = ( EffectiveDexterity( pSoldier ) + EffectiveMarksmanship( pSoldier ) ) / 2;
		// throwing trait helps out
		iChance += gbSkillTraitBonus[THROWING] * NUM_SKILL_TRAITS(pSoldier, THROWING);
	}
	else
	{
		// MECHANICALLY FIRED arced projectile (ie. mortar), need brains & know-how
		iChance = ( EffectiveDexterity( pSoldier ) + EffectiveMarksmanship( pSoldier ) + EffectiveWisdom( pSoldier ) + (10 * EffectiveExpLevel( pSoldier ) )) / 4;

		// heavy weapons trait helps out
		iChance += gbSkillTraitBonus[HEAVY_WEAPS] * NUM_SKILL_TRAITS(pSoldier, HEAVY_WEAPS);
	}

	// modify based on morale
	iChance += GetMoraleModifier( pSoldier );

	// modify by fatigue
	iChance -= GetSkillCheckPenaltyForFatigue( pSoldier, iChance );

	// if shooting same target from same position as the last shot
	if (sGridNo == pSoldier->sLastTarget)
	{
		iChance += AIM_BONUS_SAME_TARGET; // give a bonus to hit
	}

	// ADJUST FOR EXTRA AIMING TIME
	if (ubAimTime)
	{
		iChance += (AIM_BONUS_PER_AP * ubAimTime); // bonus for every pt of aiming
	}

	// if shooter is being affected by gas
	if ( pSoldier->uiStatusFlags & SOLDIER_GASSED )
	{
		iChance -= AIM_PENALTY_GASSED;
	}

	// if shooter is being bandaged at the same time, his concentration is off
	if (pSoldier->ubServiceCount > 0)
	{
		iChance -= AIM_PENALTY_GETTINGAID;
	}

	// if shooter is still in shock
	if (pSoldier->bShock)
	{
		iChance -= (pSoldier->bShock * AIM_PENALTY_PER_SHOCK);
	}

	// calculate actual range (in world units)
	iRange = (INT16)GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, sGridNo );

	if (IsWearingHeadGear(*pSoldier, SUNGOGGLES))
	{
		// decrease effective range by 10% when using sungoggles (w or w/o scope)
		iRange -= iRange / 10;	//basically, +1% to hit per every 2 squares
	}

	// ADJUST FOR RANGE

	if ( usHandItem == MORTAR && iRange < MIN_MORTAR_RANGE)
	{
		return(0);
	}
	else
	{
		iMaxRange = CalcMaxTossRange( pSoldier, usHandItem , TRUE ) * CELL_X_SIZE;

		// bonus if range is less than 1/2 maximum range, penalty if it's more

		// bonus is 50% at range 0, -50% at maximum range

		iChance += 50 * 2 * ( (iMaxRange / 2) - iRange ) / iMaxRange;
		//iChance += ((iMaxRange / 2) - iRange); // increments of 1% per pixel

		// IF TARGET IS BEYOND MAXIMUM THROWING RANGE
		if (iRange > iMaxRange)
		{
			// the object CAN travel that far if not blocked, but it's NOT accurate!
			iChance /= 2;
		}
	}

	// IF CHANCE EXISTS, BUT ATTACKER IS INJURED
	if ((iChance > 0) && (pSoldier->bLife < pSoldier->bLifeMax))
	{
		// if bandaged, give 1/2 of the bandaged life points back into equation
		bBandaged = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;

		// injury penalty is based on % damage taken (max 2/3rds iChance)
		bPenalty = (2 * iChance * (pSoldier->bLifeMax - pSoldier->bLife + (bBandaged / 2))) /
				(3 * pSoldier->bLifeMax);

		// for mechanically-fired projectiles, reduce penalty in half
		if ( GCM->getItem(usHandItem)->getItemClass() == IC_LAUNCHER )
		{
			bPenalty /= 2;
		}

		// reduce injury penalty due to merc's experience level (he can take it!)
		iChance -= (bPenalty * (100 - (10 * ( EffectiveExpLevel( pSoldier ) - 1)))) / 100;
	}

	// IF CHANCE EXISTS, BUT ATTACKER IS LOW ON BREATH
	if ((iChance > 0) && (pSoldier->bBreath < 100))
	{
		// breath penalty is based on % breath missing (max 1/2 iChance)
		bPenalty = (iChance * (100 - pSoldier->bBreath)) / 200;

		// for mechanically-fired projectiles, reduce penalty in half
		if ( GCM->getItem(usHandItem)->getItemClass() == IC_LAUNCHER )
			bPenalty /= 2;

		// reduce breath penalty due to merc's dexterity (he can compensate!)
		iChance -= (bPenalty * (100 - ( EffectiveDexterity( pSoldier ) - 10))) / 100;
	}

	// if iChance exists, but it's a mechanical item being used
	if ((iChance > 0) && (GCM->getItem(usHandItem)->getItemClass() == IC_LAUNCHER ))
		// reduce iChance to hit DIRECTLY by the item's working condition
		iChance = (iChance * WEAPON_STATUS_MOD(pSoldier->inv[HANDPOS].bStatus[0])) / 100;

	// MAKE SURE CHANCE TO HIT IS WITHIN DEFINED LIMITS
	if (iChance < MINCHANCETOHIT)
		iChance = MINCHANCETOHIT;
	else
	{
		if (iChance > MAXCHANCETOHIT)
			iChance = MAXCHANCETOHIT;
	}
	return (iChance);
}


bool HasLauncher(const SOLDIERTYPE* const s)
{
	OBJECTTYPE const& o = s->inv[HANDPOS];
	return FindAttachment(&o, UNDER_GLAUNCHER) != ITEM_NOT_FOUND &&
		FindLaunchableAttachment(&o, UNDER_GLAUNCHER) != ITEM_NOT_FOUND;
}


// Ensure the main hand item is in sync with the soldier's weapon mode.
void EnsureConsistentWeaponMode(SOLDIERTYPE* const s)
{
	if (s->bWeaponMode == WM_BURST)
	{
		if (!IsGunBurstCapable(s, HANDPOS))
		{
			s->bWeaponMode = WM_NORMAL;
		}
	}
	else if (s->bWeaponMode == WM_ATTACHED)
	{
		if (!HasLauncher(s))
		{
			s->bWeaponMode = WM_NORMAL;
		}
	}

	s->bDoBurst = s->bWeaponMode == WM_BURST;
}


// Change the soldier's weapon mode to the next possible in
// the cycle NORMAL ➔ BURST ➔ ATTACHED
void ChangeWeaponMode(SOLDIERTYPE* const s)
{
	// ATE: Don't do this if in a fire amimation.....
	if (gAnimControl[s->usAnimState].uiFlags & ANIM_FIRE) return;

	WeaponModes& mode = s->bWeaponMode;
	switch (mode)
	{
		case WM_NORMAL:
			if (IsGunBurstCapable(s, HANDPOS))
			{
				mode = WM_BURST;
			}
			else if (HasLauncher(s))
			{
				mode = WM_ATTACHED;
			}
			else
			{
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(g_langRes->Message[STR_NOT_BURST_CAPABLE], s->name));
			}
			break;

		case WM_BURST:
			mode = (HasLauncher(s) ? WM_ATTACHED : WM_NORMAL);
			break;

		case WM_ATTACHED:
		default:
			mode = WM_NORMAL;
			break;
	}

	EnsureConsistentWeaponMode(s);

	DirtyMercPanelInterface(s, DIRTYLEVEL2);
	gfUIForceReExamineCursorData = TRUE;
}


void DishoutQueenSwipeDamage( SOLDIERTYPE *pQueenSoldier )
{
	static const INT8 bValidDishoutDirs[3][3] =
	{
		{ NORTH, NORTHEAST, -1 },
		{ EAST,  SOUTHEAST, -1 },
		{ SOUTH, -1,        -1 }
	};

	INT8  bDir;
	INT32 iChance;
	INT32 iImpact;
	INT32 iHitBy;

	// Loop through all mercs and make go
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const pSoldier = *i;
		if (pSoldier == pQueenSoldier) continue;

		// ATE: Ok, lets check for some basic things here!
		if (pSoldier->bLife >= OKLIFE && pSoldier->sGridNo != NOWHERE && pSoldier->bInSector)
		{
			// Get Pyth spaces away....
			if ( GetRangeInCellCoordsFromGridNoDiff( pQueenSoldier->sGridNo, pSoldier->sGridNo ) <= GCM->getWeapon( CREATURE_QUEEN_TENTACLES)->usRange )
			{
				// get direction
				bDir = (INT8)GetDirectionFromGridNo( pSoldier->sGridNo, pQueenSoldier );

				//
				for (UINT32 cnt2 = 0; cnt2 < 2; ++cnt2)
				{
					if ( bValidDishoutDirs[ pQueenSoldier->uiPendingActionData1 ][ cnt2 ] == bDir )
					{
						iChance = CalcChanceToStab( pQueenSoldier, pSoldier, 0 );

						// CC: Look here for chance to hit, damage, etc...
						// May want to not hit if target is prone, etc....
						iHitBy = iChance - (INT32) PreRandom( 100 );
						if ( iHitBy > 0 )
						{
							// Hit!
							iImpact = HTHImpact( pQueenSoldier, pSoldier, iHitBy, TRUE );
							EVENT_SoldierGotHit(pSoldier, CREATURE_QUEEN_TENTACLES, iImpact, iImpact, OppositeDirection(bDir), 50, pQueenSoldier, 0, ANIM_CROUCH, 0);
						}
					}
				}
			}
		}
	}

	pQueenSoldier->uiPendingActionData1++;
}


static BOOLEAN WillExplosiveWeaponFail(const SOLDIERTYPE* pSoldier, const OBJECTTYPE* pObj)
{
	if ( pSoldier->bTeam == OUR_TEAM || pSoldier->bVisible == 1 )
	{
		if ( (INT8)(PreRandom( 40 ) + PreRandom( 40 ) ) > pObj->bStatus[0] )
		{
			// Do second dice roll
			if ( PreRandom( 2 ) == 1 )
			{
				// Fail
				return( TRUE );
			}
		}
	}

	return( FALSE );
}


// #ifdef WITH_UNITTESTS
// #include "gtest/gtest.h"

// TEST(Weapons, asserts)
// {
//   EXPECT_EQ(lengthof(OLD_Weapon), MAX_WEAPONS);
// }

// #endif
