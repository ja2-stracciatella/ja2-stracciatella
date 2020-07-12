#include "Directories.h"
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
#include "GameRes.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "content/ContentMercs.h"
#include "WeaponModels.h"

extern BOOLEAN gfProfileDataLoaded;


BOOLEAN	gfPotentialTeamChangeDuringDeath = FALSE;


MERCPROFILESTRUCT gMercProfiles[ NUM_PROFILES ];

INT8 gbSkillTraitBonus[NUM_SKILLTRAITS] =
{
	0, //NO_SKILLTRAIT
	25, //LOCKPICKING
	15, //HANDTOHAND
	15, //ELECTRONICS
	15, //NIGHTOPS
	12, //THROWING
	15, //TEACHING
	15, //HEAVY_WEAPS
	0, //AUTO_WEAPS
	15, //STEALTHY
	0, //AMBIDEXT
	0, //THIEF // UNUSED!
	30, //MARTIALARTS
	30, //KNIFING
	15, //ONROOF
	0, //CAMOUFLAGED
};


UINT8 gubNumTerrorists = 0;

struct TerroristInfo
{
	ProfileID profile;
	UINT8     sectors[5];
};

static TerroristInfo const g_terrorist_infos[] =
{
	{DRUGGIST, { 0,       0,       0,       0,       0      }}, // Elgin, preplaced
	{SLAY,     { SEC_F9,  SEC_I14, SEC_G1,  SEC_G2,  SEC_G8 }}, // Slay
	{ANNIE,    { SEC_I14, SEC_C6,  SEC_B2,  SEC_L11, SEC_G8 }}, // Matron
	{CHRIS,    { SEC_G1,  SEC_F9,  SEC_L11, SEC_G8,  SEC_G2 }}, // Imposter
	{TIFFANY,  { SEC_I14, SEC_G2,  SEC_H14, SEC_C6,  SEC_B2 }}, // Tiffany
	{T_REX,    { SEC_F9,  SEC_H14, SEC_H2,  SEC_G1,  SEC_B2 }}  // Rexall
};

INT16 gsRobotGridNo;


struct AssassinInfo
{
	ProfileID profile;
	UINT8     towns[5];
};

static AssassinInfo const g_assassin_info[] =
{
	{ JIM,    { CAMBRIA,  DRASSEN, ALMA,    BALIME,  GRUMM   } },
	{ JACK,   { CHITZENA, ESTONI,  ALMA,    BALIME,  GRUMM   } },
	{ OLAF,   { DRASSEN,  ESTONI,  ALMA,    CAMBRIA, BALIME  } },
	{ RAY,    { CAMBRIA,  OMERTA,  BALIME,  GRUMM,   DRASSEN } },
	{ OLGA,   { CHITZENA, OMERTA,  CAMBRIA, ALMA,    GRUMM   } },
	{ TYRONE, { CAMBRIA,  BALIME,  ALMA,    GRUMM,   DRASSEN } }
};


static INT16 CalcMedicalDeposit(MERCPROFILESTRUCT const&);
static void DecideActiveTerrorists();
static void StartSomeMercsOnAssignment(void);


void LoadMercProfiles()
{
	{ AutoSGPFile f(GCM->openGameResForReading(BINARYDATADIR "/prof.dat"));
		LoadRawMercProfiles(f, NUM_PROFILES, gMercProfiles, getDataFilesEncodingCorrector());
		for (UINT32 i = 0; i != NUM_PROFILES; ++i)
		{
			MERCPROFILESTRUCT& p = gMercProfiles[i];

			// // dumping std inventory
			// printf("%03d/%s\n", i, p.zNickname.c_str());
			// FOR_EACH(UINT16, k, p.inv)
			// {
			//   const ItemModel *item = GCM->getItem(*k);
			//   printf("  %s\n", item->getInternalName().c_str());
			// }

			// If the dialogue exists for the merc, allow the merc to be hired
			p.bMercStatus = Content::canMercBeHired(GCM, i) ? 0 : MERC_HAS_NO_TEXT_FILE;

			p.sMedicalDepositAmount = p.bMedicalDeposit ? CalcMedicalDeposit(p) : 0;

			// ATE: New, face display independent of ID num now, default is the
			// profile ID
			p.ubFaceIndex = i;

			if (!gGameOptions.fGunNut)
			{
				// CJC: replace guns in profile if they aren't available
				FOR_EACH(UINT16, k, p.inv)
				{
					const ItemModel *item = GCM->getItem(*k);
					if (!item->isGun() || !item->isInBigGunList()) continue;

					const WeaponModel *oldWeapon = item->asWeapon();
					const WeaponModel *newWeapon = GCM->getWeaponByName(oldWeapon->getStandardReplacement());

					*k = newWeapon->getItemIndex();

					// Search through inventory and replace ammo accordingly
					FOR_EACH(UINT16, l, p.inv)
					{
						UINT16 const ammo = *l;
						if (!(GCM->getItem(ammo)->isAmmo())) continue;
						UINT16 const new_ammo = FindReplacementMagazineIfNecessary(oldWeapon, ammo, newWeapon);
						if (new_ammo == NOTHING) continue;
						// Found a new magazine, replace
						*l = new_ammo;
					}
				}
			}

			// Calculate inital attractiveness for the merc's initial gun and armour.
			// Calculate the optional gear cost.
			p.bMainGunAttractiveness = -1;
			p.bArmourAttractiveness  = -1;
			p.usOptionalGearCost     =  0;
			FOR_EACH(UINT16 const, k, p.inv)
			{
				UINT16 const item_id = *k;
				if (item_id == NOTHING) continue;
				const ItemModel * item = GCM->getItem(item_id);

				if (item->isGun())    p.bMainGunAttractiveness = GCM->getWeapon(item_id)->ubDeadliness;
				if (item->isArmour()) p.bArmourAttractiveness  = Armour[item->getClassIndex()].ubProtection;

				p.usOptionalGearCost += item->getPrice();
			}

			// These variables to get loaded in
			p.fUseProfileInsertionInfo = FALSE;
			p.sGridNo                  = 0;

			// ARM: this is also being done inside the profile editor, but put it here
			// too, so this project's code makes sense
			p.bHatedCount[0]    = p.bHatedTime[0];
			p.bHatedCount[1]    = p.bHatedTime[1];
			p.bLearnToHateCount = p.bLearnToHateTime;
			p.bLearnToLikeCount = p.bLearnToLikeTime;
		}
	}

	DecideActiveTerrorists();

	// Initialize mercs' status
	StartSomeMercsOnAssignment();

	gfProfileDataLoaded = TRUE;

	// no better place..heh?.. will load faces for profiles that are 'extern'.....won't have soldiertype instances
	InitalizeStaticExternalNPCFaces();

	LoadCarPortraitValues();
}


#define MAX_ADDITIONAL_TERRORISTS 4


// One terrorist is always Elgin. Determine how many more terrorists - 2 to 4
// more.
static void DecideActiveTerrorists()
{
	// Using this stochastic process(!), the chances for terrorists are:
	// EASY:    3,  9%    4, 42%    5, 49%
	// MEDIUM:  3, 25%    4, 50%    5, 25%
	// HARD:    3, 49%    4, 42%    5,  9%
	UINT32 chance;
	switch (gGameOptions.ubDifficultyLevel)
	{
		case DIF_LEVEL_EASY: chance = 70; break;
		default:             chance = 50; break;
		case DIF_LEVEL_HARD: chance = 30; break;
	}
	UINT8 n_additional_terrorists = 2; // Add at least 2 more.
	for (UINT8 n = MAX_ADDITIONAL_TERRORISTS - n_additional_terrorists; n != 0; --n)
	{
		if (Chance(chance)) ++n_additional_terrorists;
	}

	UINT8 terrorist_placement[MAX_ADDITIONAL_TERRORISTS];
	for (UINT8 n_terrorists_added = 0; n_terrorists_added != n_additional_terrorists;)
	{
		FOR_EACH(TerroristInfo const, i, g_terrorist_infos)
		{
			if (n_terrorists_added == n_additional_terrorists) break;

			TerroristInfo const& t = *i;
			MERCPROFILESTRUCT&   p = GetProfile(t.profile);
			// Random 40% chance of adding this terrorist if not yet placed.
			if (p.sSectorX != 0)   continue;
			if (Random(100) >= 40) continue;

			// Since there are 5 spots per terrorist and a maximum of 5 terrorists, we
			// are guaranteed to be able to find a spot for each terrorist since there
			// aren't enough other terrorists to use up all the spots for any one
			// terrorist
pick_sector:
			// Pick a random spot, see if it's already been used by another terrorist.
			UINT8 const sector = t.sectors[Random(lengthof(t.sectors))];
			for (UINT8 k = 0; k != n_terrorists_added; ++k)
			{
				if (terrorist_placement[k] == sector) goto pick_sector;
			}

			// Place terrorist.
			p.sSectorX = SECTORX(sector);
			p.sSectorY = SECTORY(sector);
			p.bSectorZ = 0;
			terrorist_placement[n_terrorists_added++] = sector;
		}
	}

	// Set total terrorists outstanding in Carmen's info byte.
	GetProfile(CARMEN).bNPCData = 1 + n_additional_terrorists;
	// Store total terrorists.
	gubNumTerrorists = 1 + n_additional_terrorists;
}


void MakeRemainingTerroristsTougher()
{
	UINT8 n_remaining_terrorists = 0;
	FOR_EACH(TerroristInfo const, i, g_terrorist_infos)
	{
		ProfileID         const  pid = i->profile;
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
	{
		// nothing
		return;
	}
	else if (remaining_difficulty < 28)
	{
		// mini grenade
		old_item = NOTHING;
		new_item = MINI_GRENADE;
	}
	else if (remaining_difficulty < 42)
	{
		// hand grenade
		old_item = MINI_GRENADE;
		new_item = HAND_GRENADE;
	}
	else if (remaining_difficulty < 56)
	{
		// mustard
		old_item = HAND_GRENADE;
		new_item = MUSTARD_GRENADE;
	}
	else if (remaining_difficulty < 70)
	{
		// LAW
		old_item = MUSTARD_GRENADE;
		new_item = ROCKET_LAUNCHER;
	}
	else
	{
		// LAW and hand grenade
		old_item = NOTHING;
		new_item = HAND_GRENADE;
	}

	OBJECTTYPE Object;
	DeleteObj(&Object);
	Object.usItem     = new_item;
	Object.bStatus[0] = 100;

	FOR_EACH(TerroristInfo const, i, g_terrorist_infos)
	{
		ProfileID         const  pid = i->profile;
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


void DecideOnAssassin()
{
	ProfileID   assassins[lengthof(g_assassin_info)];
	UINT8       n    = 0;
	UINT8 const town = GetTownIdForSector(SECTOR(gWorldSectorX, gWorldSectorY));
	FOR_EACH(AssassinInfo const, i, g_assassin_info)
	{
		AssassinInfo      const  a = *i;
		MERCPROFILESTRUCT const& p = GetProfile(a.profile);
		// Make sure alive and not placed already.
		if (p.bMercStatus == MERC_IS_DEAD)      continue;
		if (p.sSectorX != 0 || p.sSectorY != 0) continue;
		// Check this merc to see if the town is a possibility.
		FOR_EACH(UINT8 const, k, a.towns)
		{
			if (*k != town) continue;
			assassins[n++] = a.profile;
			break;
		}
	}

	if (n == 0) return;
	ProfileID const    pid = assassins[Random(n)];
	MERCPROFILESTRUCT& p   = GetProfile(pid);
	p.sSectorX             = gWorldSectorX;
	p.sSectorY             = gWorldSectorY;
	AddStrategicEvent(EVENT_REMOVE_ASSASSIN, GetWorldTotalMin() + 60 * (3 + Random(3)), pid);
}


void MakeRemainingAssassinsTougher()
{
	UINT8 n_remaining_assassins = 0;
	FOR_EACH(AssassinInfo const, i, g_assassin_info)
	{
		if (GetProfile(i->profile).bMercStatus == MERC_IS_DEAD) continue;
		++n_remaining_assassins;
	}

	size_t const n_assassins = lengthof(g_assassin_info);
	UINT8        difficulty  = 60 / n_assassins * (n_assassins - n_remaining_assassins);
	switch (gGameOptions.ubDifficultyLevel)
	{
		case DIF_LEVEL_MEDIUM: difficulty = difficulty * 13 / 10; break;
		case DIF_LEVEL_HARD:   difficulty = difficulty * 16 / 10; break;
		default: break;
	}

	UINT16 new_item;
	UINT16 old_item;
	if (difficulty < 14)
	{
		// Nothing
		return;
	}
	else if (difficulty < 28)
	{
		// Mini grenade
		old_item = NOTHING;
		new_item = MINI_GRENADE;
	}
	else if (difficulty < 42)
	{
		// Hand grenade
		old_item = MINI_GRENADE;
		new_item = HAND_GRENADE;
	}
	else if (difficulty < 56)
	{
		// Mustard grenade
		old_item = HAND_GRENADE;
		new_item = MUSTARD_GRENADE;
	}
	else if (difficulty < 70)
	{
		// LAW
		old_item = MUSTARD_GRENADE;
		new_item = ROCKET_LAUNCHER;
	}
	else
	{
		// LAW and hand grenade
		old_item = NOTHING;
		new_item = HAND_GRENADE;
	}

	OBJECTTYPE o;
	CreateItem(new_item, 100, &o);
	FOR_EACH(AssassinInfo const, i, g_assassin_info)
	{
		ProfileID const pid = i->profile;
		if (GetProfile(pid).bMercStatus == MERC_IS_DEAD) continue;
		if (old_item != NOTHING)
		{
			RemoveObjectFromSoldierProfile(pid, old_item);
		}
		PlaceObjectInSoldierProfile(pid, &o);
	}
}


static void StartSomeMercsOnAssignment(void)
{
	UINT32 uiCnt;
	UINT32 uiChance;

	// some randomly picked A.I.M. mercs will start off "on assignment" at the beginning of each new game
	for (uiCnt = 0; uiCnt <= AIM_AND_MERC_MERCS; uiCnt++)
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


void SetProfileFaceData(ProfileID const pid, UINT8 const face_idx, UINT16 const eyes_x, UINT16 const eyes_y, UINT16 const mouth_x, UINT16 const mouth_y)
{
	MERCPROFILESTRUCT& p = GetProfile(pid);
	p.ubFaceIndex = face_idx;
	p.usEyesX     = eyes_x;
	p.usEyesY     = eyes_y;
	p.usMouthX    = mouth_x;
	p.usMouthY    = mouth_y;
}


static UINT16 CalcCompetence(MERCPROFILESTRUCT const& p)
{
	UINT32 uiStats, uiSkills, uiActionPoints, uiSpecialSkills;
	UINT16 usCompetence;


	// count life twice 'cause it's also hit points
	// mental skills are halved 'cause they're actually not that important within the game
	uiStats = ((2 * p.bLifeMax) + p.bStrength + p.bAgility + p.bDexterity + ((p.bLeadership + p.bWisdom) / 2)) / 3;

	// marksmanship is very important, count it double
	uiSkills = (UINT32) ((2 * (pow((double)p.bMarksmanship, 3) / 10000)) +
				1.5 *	(pow((double)p.bMedical, 3) / 10000) +
				(pow((double)p.bMechanical, 3) / 10000) +
				(pow((double)p.bExplosive, 3) / 10000));

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
	FOR_EACH_SOLDIER(s)
	{
		if (s->ubProfile == pid) return s;
	}
	return NULL;
}


SOLDIERTYPE* FindSoldierByProfileIDOnPlayerTeam(const ProfileID pid)
{
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s->ubProfile == pid) return s;
	}
	return NULL;
}


SOLDIERTYPE* ChangeSoldierTeam(SOLDIERTYPE* const old_s, UINT8 const team)
{
	if (gfInTalkPanel) DeleteTalkingMenu();

	GridNo const old_gridno = old_s->sGridNo;

	// At the low level check if this guy is in inv panel, else remove.
	if (gsCurInterfacePanel == SM_PANEL && gpSMCurrentMerc == old_s)
	{
		SetCurrentInterfacePanel(TEAM_PANEL);
	}

	// Remove him from the game.
	InternalTacticalRemoveSoldier(*old_s, FALSE);

	// Create a new one.
	SOLDIERCREATE_STRUCT c;
	c = SOLDIERCREATE_STRUCT{};
	c.bTeam            = team;
	c.ubProfile        = old_s->ubProfile;
	c.bBodyType        = old_s->ubBodyType;
	c.sSectorX         = old_s->sSectorX;
	c.sSectorY         = old_s->sSectorY;
	c.bSectorZ         = old_s->bSectorZ;
	c.sInsertionGridNo = old_s->sGridNo; // XXX always NOWHERE due to InternalTacticalRemoveSoldier() above
	c.bDirection       = old_s->bDirection;
	if (old_s->uiStatusFlags & SOLDIER_VEHICLE)
	{
		c.ubProfile          = NO_PROFILE;
		c.fUseGivenVehicle   = TRUE;
		c.bUseGivenVehicleID = old_s->bVehicleID;
	}
	SOLDIERTYPE* const new_s = TacticalCreateSoldier(c);
	if (!new_s) return 0;

	// Copy vital stats back.
	new_s->bLife                   = old_s->bLife;
	new_s->bLifeMax                = old_s->bLifeMax;
	new_s->bAgility                = old_s->bAgility;
	new_s->bLeadership             = old_s->bLeadership;
	new_s->bDexterity              = old_s->bDexterity;
	new_s->bStrength               = old_s->bStrength;
	new_s->bWisdom                 = old_s->bWisdom;
	new_s->bExpLevel               = old_s->bExpLevel;
	new_s->bMarksmanship           = old_s->bMarksmanship;
	new_s->bMedical                = old_s->bMedical;
	new_s->bMechanical             = old_s->bMechanical;
	new_s->bExplosive              = old_s->bExplosive;
	new_s->bLastRenderVisibleValue = old_s->bLastRenderVisibleValue;
	new_s->bVisible                = old_s->bVisible;
	new_s->bCamo                   = old_s->bCamo;
	new_s->bLevel                  = old_s->bLevel;
	if(old_s->bLevel == SECOND_LEVEL) SetSoldierHeight(new_s, SECOND_LEVEL_Z_OFFSET);
	if (new_s->bCamo != 0) CreateSoldierPalettes(*new_s);

	if (team == OUR_TEAM) new_s->bVisible = 1;

	// Copy over any items.
	for (UINT32 i = 0; i != NUM_INV_SLOTS; ++i)
	{
		new_s->inv[i] = old_s->inv[i];
	}

	// Loop through all active merc slots, change any attacker's target if they
	// were once on this guy.
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE& s = **i;
		if (s.target == old_s) s.target = new_s;
	}

	new_s->sInsertionGridNo = old_gridno;

	if (gfPotentialTeamChangeDuringDeath)
	{
		HandleCheckForDeathCommonCode(old_s);
	}

	if (gfWorldLoaded && old_s->bInSector)
	{
		AddSoldierToSectorNoCalculateDirectionUseAnimation(new_s, old_s->usAnimState, old_s->usAniCode);
		HandleSight(*new_s, SIGHT_LOOK | SIGHT_RADIO);
	}

	if (new_s->ubProfile != NO_PROFILE)
	{
		UINT8& misc_flags = GetProfile(new_s->ubProfile).ubMiscFlags;
		misc_flags = team == OUR_TEAM ?
			misc_flags |  PROFILE_MISC_FLAG_RECRUITED :
			misc_flags & ~PROFILE_MISC_FLAG_RECRUITED;
	}

	return new_s;
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
	SOLDIERTYPE* const pNewSoldier = ChangeSoldierTeam(pSoldier, OUR_TEAM);

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
			if ( GCM->getItem(pNewSoldier->inv[ bSlot ].usItem)->isTwoHanded() )
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
	SOLDIERTYPE* const pNewSoldier = ChangeSoldierTeam( pSoldier, OUR_TEAM );
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
	if (ubCharNum == NO_PROFILE)
	{
		return -1;
	}

	MERCPROFILESTRUCT const& p = GetProfile(ubCharNum);
	for (INT8 bLoop = 0; bLoop < 3; bLoop++)
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


bool IsProfileATerrorist(ProfileID const pid)
{
	FOR_EACH(TerroristInfo const, i, g_terrorist_infos)
	{
		if (i->profile == pid) return true;
	}
	return false;
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


void UpdateSoldierPointerDataIntoProfile()
{
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE const& s = **i;
		if (s.ubProfile == NO_PROFILE) continue;
		// If we are above player mercs
		if (s.ubProfile < FIRST_RPC)   continue;

		MERCPROFILESTRUCT& p = GetProfile(s.ubProfile);
		p.bLife         = s.bLife;
		p.bLifeMax      = s.bLifeMax;
		p.bAgility      = s.bAgility;
		p.bLeadership   = s.bLeadership;
		p.bDexterity    = s.bDexterity;
		p.bStrength     = s.bStrength;
		p.bWisdom       = s.bWisdom;
		p.bExpLevel     = s.bExpLevel;
		p.bMarksmanship = s.bMarksmanship;
		p.bMedical      = s.bMedical;
		p.bMechanical   = s.bMechanical;
		p.bExplosive    = s.bExplosive;
	}
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

BOOLEAN IsProfileIdAnAimOrMERCMerc(UINT8 ubProfileID)
{
	// AIM: ubProfileID < BIFF
	// MERC: ubProfileID >= BIFF && ubProfileID <= BUBBA
	return ubProfileID <= AIM_AND_MERC_MERCS;
}
