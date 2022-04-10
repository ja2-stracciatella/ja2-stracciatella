#include "Inventory_Choosing.h"
#include "Animation_Data.h"
#include "Items.h"
#include "Random.h"
#include "Weapons.h"
#include "Strategic_Status.h"
#include "Campaign.h"
#include "GameSettings.h"
#include "StrategicMap.h"
#include "Auto_Resolve.h"
#include "Map_Screen_Interface_Map.h"
#include "Debug.h"

#include "AmmoTypeModel.h"
#include "ContentManager.h"
#include "GameInstance.h"
#include "MagazineModel.h"
#include "WeaponModels.h"
#include "policy/GamePolicy.h"

#define ENEMYAMMODROPRATE       50 // % of time enemies drop ammunition
#define ENEMYGRENADEDROPRATE    25 // % of time enemies drop grenades
#define ENEMYEQUIPDROPRATE      15 // % of stuff enemies drop equipment

// only 1/10th of what enemies drop...
#define MILITIAAMMODROPRATE      5 // % of time enemies drop ammunition
#define MILITIAGRENADEDROPRATE   3 // % of time enemies drop grenades
#define MILITIAEQUIPDROPRATE     2 // % of stuff enemies drop equipment

#define MAX_MORTARS_PER_TEAM     1 // one team can't randomly roll more than this many mortars per sector


UINT32 guiMortarsRolledByTeam = 0;
static const SGPSector tixa(TIXA_SECTOR_X, TIXA_SECTOR_Y);

static void MarkAllWeaponsOfSameGunClassAsDropped(UINT16 usWeapon);


void InitArmyGunTypes(void)
{
	UINT8 ubWeapon;

	// set all flags that track whether this weapon type has been dropped before to FALSE
	for (ubWeapon = 0; ubWeapon < MAX_WEAPONS; ubWeapon++)
	{
		gStrategicStatus.fWeaponDroppedAlready[ubWeapon] = FALSE;
	}

	// avoid auto-drops for the gun class with the crappiest guns in it
	MarkAllWeaponsOfSameGunClassAsDropped( SW38 );
}


static INT8 GetWeaponClass(UINT16 usGun)
{
	const std::vector<std::vector<const WeaponModel*> > & gunChoice = GCM->getExtendedGunChoice();
	UINT32 uiGunLevel, uiLoop;

	// always use the extended list since it contains all guns...
	for (uiGunLevel = 0; uiGunLevel < gunChoice.size(); uiGunLevel++)
	{
		for ( uiLoop = 0; uiLoop < gunChoice[uiGunLevel].size(); uiLoop++ )
		{
			if ( gunChoice[uiGunLevel][uiLoop]->getItemIndex() == usGun )
			{
				return( (INT8) uiGunLevel );
			}
		}
	}
	return( -1 );
}


static void MarkAllWeaponsOfSameGunClassAsDropped(UINT16 usWeapon)
{
	INT8 bGunClass;
	UINT32 uiLoop;


	// mark that item itself as dropped, whether or not it's part of a gun class
	gStrategicStatus.fWeaponDroppedAlready[ usWeapon ] = TRUE;

	bGunClass = GetWeaponClass( usWeapon );

	// if the gun belongs to a gun class (mortars, GLs, LAWs, etc. do not and are handled independently)
	const std::vector<std::vector<const WeaponModel*> > & gunChoice = GCM->getExtendedGunChoice();

	if ( bGunClass != -1 )
	{
		// then mark EVERY gun in that class as dropped
		for ( uiLoop = 0; uiLoop < gunChoice[bGunClass].size(); uiLoop++ )
		{
			gStrategicStatus.fWeaponDroppedAlready[gunChoice[bGunClass][uiLoop]->getItemIndex()] = TRUE;
		}
	}
}


static void ChooseArmourForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp, INT8 bHelmetClass, INT8 bVestClass, INT8 bLeggingsClass);
static void ChooseBombsForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp, INT8 bBombClass);
static void ChooseFaceGearForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp);
static void ChooseGrenadesForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp, INT8 bGrenades, INT8 bGrenadeClass, BOOLEAN fGrenadeLauncher);
static void ChooseKitsForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp, INT8 bKitClass);
static void ChooseLocationSpecificGearForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp);
static void ChooseMiscGearForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp, INT8 bMiscClass);
static void ChooseSpecialWeaponsForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp, INT8 bKnifeClass, BOOLEAN fGrenadeLauncher, BOOLEAN fLAW, BOOLEAN fMortar);
static void ChooseWeaponForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp, INT8 bWeaponClass, INT8 bAmmoClips, INT8 bAttachClass, BOOLEAN fAttachment);
static void EquipTank(SOLDIERCREATE_STRUCT* pp);
static void RandomlyChooseWhichItemsAreDroppable(SOLDIERCREATE_STRUCT* pp, INT8 bSoldierClass);


//Chooses equipment based on the relative equipment level (0-4) with best being 4.  It allocates a range
//of equipment to choose from.
//NOTE:  I'm just winging it for the decisions on which items that different groups can have.  Basically,
// there are variations, so a guy at a certain level may get a better gun and worse armour or vice versa.
void GenerateRandomEquipment( SOLDIERCREATE_STRUCT *pp, INT8 bSoldierClass, INT8 bEquipmentRating )
{
	OBJECTTYPE *pItem;
	//general rating information
	int bRating = 0;
	//numbers of items
	INT8 bAmmoClips = 0;
	INT8 bGrenades = 0;
	BOOLEAN fAttachment = FALSE;
	//item levels
	INT8 bWeaponClass = 0;
	INT8 bHelmetClass = 0;
	INT8 bVestClass = 0;
	INT8 bLeggingClass = 0;
	INT8 bAttachClass = 0;
	INT8 bGrenadeClass = 0;
	INT8 bKnifeClass = 0;
	INT8 bKitClass = 0;
	INT8 bMiscClass = 0;
	INT8 bBombClass = 0;
	//special weapons
	BOOLEAN fMortar = FALSE;
	BOOLEAN fGrenadeLauncher = FALSE;
	BOOLEAN fLAW = FALSE;
	INT32 i;
	INT8 bEquipmentModifier;
	UINT8 ubMaxSpecialWeaponRoll;


	Assert( pp );

	// kids don't get anything 'cause they don't have any weapon animations and the rest is inappropriate
	if ( ( pp->bBodyType == HATKIDCIV ) || ( pp->bBodyType == KIDCIV ) )
	{
		return;
	}


	if ( ( pp->bBodyType == TANK_NE ) || ( pp->bBodyType == TANK_NW ) )
	{
		EquipTank( pp );
		return;
	}


	Assert( ( bSoldierClass >= SOLDIER_CLASS_NONE ) && ( bSoldierClass <= SOLDIER_CLASS_ELITE_MILITIA ) );
	Assert( ( bEquipmentRating >= 0 ) && ( bEquipmentRating <= 4 ) );


	// equipment level is modified by 1/10 of the difficulty percentage, -5, so it's between -5 to +5
	// (on normal, this is actually -4 to +4, easy is -5 to +3, and hard is -3 to +5)
	bEquipmentModifier = bEquipmentRating + ( ( CalcDifficultyModifier( bSoldierClass ) / 10 ) - 5 );

	switch( bSoldierClass )
	{
		case SOLDIER_CLASS_NONE:
			// ammo is here only so that civilians with pre-assigned ammo will get some clips for it!
			bAmmoClips = (INT8)(1 + Random( 2 ));

			// civilians get nothing, anyone who should get something should be preassigned by Linda
			break;

		case SOLDIER_CLASS_ADMINISTRATOR:
		case SOLDIER_CLASS_GREEN_MILITIA:
			bRating = BAD_ADMINISTRATOR_EQUIPMENT_RATING + bEquipmentModifier;
			bRating = (INT8) std::clamp(bRating, MIN_EQUIPMENT_CLASS, MAX_EQUIPMENT_CLASS);

			bWeaponClass = bRating;

			//Note:  in some cases the class of armour and/or helmet won't be high enough to make
			//       the lowest level.
			bVestClass = bRating;
			bHelmetClass = bRating;
			// no leggings

			if( Random( 2 ) )
				bKnifeClass = bRating;

			bAmmoClips = (INT8)(1 + Random( 2 ));

			if( bRating >= GOOD_ADMINISTRATOR_EQUIPMENT_RATING )
			{
				bAmmoClips++;

				bKitClass = bRating;
				bMiscClass = bRating;
			}

			if( bRating >= GREAT_ADMINISTRATOR_EQUIPMENT_RATING )
			{
				bGrenades = 1, bGrenadeClass = bRating;
			}

			if( ( bRating > MIN_EQUIPMENT_CLASS ) && bRating < MAX_EQUIPMENT_CLASS )
			{
				//Randomly decide if there is to be an upgrade of guns vs armour (one goes up, the other down)
				switch( Random( 5 ) )
				{
					case 0:
						bWeaponClass++, bVestClass--;
						break; //better gun, worse armour
					case 1:
						bWeaponClass--, bVestClass++;
						break; //worse gun, better armour
				}
			}
			break;

		case SOLDIER_CLASS_ARMY:
		case SOLDIER_CLASS_REG_MILITIA:
			//army guys tend to have a broad range of equipment
			bRating = BAD_ARMY_EQUIPMENT_RATING + bEquipmentModifier;
			bRating = (INT8) std::clamp(bRating, MIN_EQUIPMENT_CLASS, MAX_EQUIPMENT_CLASS);

			bWeaponClass = bRating;
			bVestClass = bRating;
			bHelmetClass = bRating;
			bGrenadeClass = bRating;

			if( ( bRating >= GOOD_ARMY_EQUIPMENT_RATING ) && ( Random( 100 ) < 33 ) )
			{
				fAttachment = TRUE;
				bAttachClass = bRating;
			}

			bAmmoClips = (INT8)(2 + Random( 2 ));

			if( bRating >= AVERAGE_ARMY_EQUIPMENT_RATING )
			{
				bGrenades = (INT8)Random( 2 );
				bKitClass = bRating;
				bMiscClass = bRating;
			}

			if( bRating >= GOOD_ARMY_EQUIPMENT_RATING )
			{
				bGrenades++;
			}

			if( bRating >= GREAT_ARMY_EQUIPMENT_RATING )
			{
				bGrenades++;

				bLeggingClass = bRating;

				if ( Chance( 25 ) )
				{
					bBombClass = bRating;
				}
			}

			if( Random( 2 ) )
				bKnifeClass = bRating;

			if( ( bRating > MIN_EQUIPMENT_CLASS ) && bRating < MAX_EQUIPMENT_CLASS )
			{
				switch( Random( 7 ) )
				{
					case 3:
						bWeaponClass++, bVestClass--;
						break; //better gun, worse armour
					case 4:
						bWeaponClass--, bVestClass++;
						break; //worse gun, better armour
					case 5:
						bVestClass++, bHelmetClass--;
						break; //better armour, worse helmet
					case 6:
						bVestClass--, bHelmetClass++;
						break; //worse armour, better helmet
				}
			}

			// if well-enough equipped, 1/5 chance of something really special
			if( ( bRating >= GREAT_ARMY_EQUIPMENT_RATING ) && ( Random( 100 ) < 20 ) )
			{
				//give this man a special weapon!  No mortars if underground, however
				ubMaxSpecialWeaponRoll = (!IsAutoResolveActive() && gWorldSector.z != 0) ? 6 : 7;
				switch ( Random ( ubMaxSpecialWeaponRoll ) )
				{
					case 0:
					case 1:
					case 2:
						if ( pp->bExpLevel >= 3 )
						{
							//grenade launcher
							fGrenadeLauncher = TRUE;
							bGrenades = 3 + (INT8)(Random( 3 )); //3-5
						}
						break;

					case 3:
					case 4:
					case 5:
						if ( pp->bExpLevel >= 4 )
						{
							// LAW rocket launcher
							fLAW = TRUE;
						}
						break;

					case 6:
						// one per team maximum!
						if ( ( pp->bExpLevel >= 5 ) && ( guiMortarsRolledByTeam < MAX_MORTARS_PER_TEAM ) )
						{
							//mortar
							fMortar = TRUE;
							guiMortarsRolledByTeam++;

							// the grenades will actually represent mortar shells in this case
							bGrenades = 2 + (INT8)(Random( 3 )); //2-4
							bGrenadeClass = MORTAR_GRENADE_CLASS;
						}
						break;
				}
			}
			break;

		case SOLDIER_CLASS_ELITE:
		case SOLDIER_CLASS_ELITE_MILITIA:
			bRating = BAD_ELITE_EQUIPMENT_RATING + bEquipmentModifier;
			bRating = (INT8) std::clamp(bRating, MIN_EQUIPMENT_CLASS, MAX_EQUIPMENT_CLASS);

			bWeaponClass = bRating;
			bHelmetClass = bRating;
			bVestClass = bRating;
			bLeggingClass = bRating;
			bAttachClass = bRating;
			bGrenadeClass = bRating;
			bKitClass = bRating;
			bMiscClass = bRating;

			if ( Chance( 25 ) )
			{
				bBombClass = bRating;
			}

			bAmmoClips = (INT8)(3 + Random( 2 ));
			bGrenades = (INT8)(1 + Random( 3 ));

			if( ( bRating >= AVERAGE_ELITE_EQUIPMENT_RATING ) && ( Random( 100 ) < 75 ) )
			{
				fAttachment = TRUE;
				bAttachClass = bRating;
			}

			if( Random( 2 ) )
				bKnifeClass = bRating;

			if( ( bRating > MIN_EQUIPMENT_CLASS ) && bRating < MAX_EQUIPMENT_CLASS )
			{
				switch( Random( 11 ) )
				{
					case 4:
						bWeaponClass++, bVestClass--;
						break;
					case 5:
						bWeaponClass--, bVestClass--;
						break;
					case 6:
						bVestClass++, bHelmetClass--;
						break;
					case 7:
						bGrenades += 2;
						break;
					case 8:
						bHelmetClass++;
						break;
					case 9:
						bVestClass++;
						break;
					case 10:
						bWeaponClass++;
						break;
				}
			}

			// if well-enough equipped, 1/3 chance of something really special
			if( ( bRating >= GOOD_ELITE_EQUIPMENT_RATING ) && ( Random( 100 ) < 33 ) )
			{
				//give this man a special weapon!  No mortars if underground, however
				ubMaxSpecialWeaponRoll = (!IsAutoResolveActive() && gWorldSector.z != 0) ? 6 : 7;
				switch ( Random ( ubMaxSpecialWeaponRoll ) )
				{
					case 0:
					case 1:
					case 2:
						//grenade launcher
						fGrenadeLauncher = TRUE;
						bGrenades = 4 + (INT8)(Random( 4 )); //4-7
						break;
					case 3:
					case 4:
					case 5:
						// LAW rocket launcher
						fLAW = TRUE;
						break;
					case 6:
						// one per team maximum!
						if ( guiMortarsRolledByTeam < MAX_MORTARS_PER_TEAM )
						{
							//mortar
							fMortar = TRUE;
							guiMortarsRolledByTeam++;

							// the grenades will actually represent mortar shells in this case
							bGrenades = 3 + (INT8)(Random( 5 )); //3-7
							bGrenadeClass = MORTAR_GRENADE_CLASS;
						}
						break;
				}
			}
			break;
	}

	for ( i = 0; i < NUM_INV_SLOTS; i++ )
	{
		//clear items, but only if they have write status.
		if( !(pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE) )
		{
			pp->Inv[ i ] = OBJECTTYPE{};
			pp->Inv[ i ].fFlags |= OBJECT_UNDROPPABLE;
		}
		else
		{
			//check to see what kind of item is here.  If we find a gun, for example, it'll make the
			//bWeaponClass negative to signify that a gun has already been specified, and later
			//code will use that to determine that and to pick ammo for it.
			pItem = &pp->Inv[ i ];
			if( !pItem )
				continue;
			switch( GCM->getItem(pItem->usItem)->getItemClass() )
			{
				case IC_GUN:
					if ( pItem->usItem != ROCKET_LAUNCHER )
					{
						bWeaponClass *= -1;
					}
					else	// rocket launcher!
					{
						fLAW = FALSE;
					}
					break;
				case IC_AMMO:
					bAmmoClips = 0;
					break;
				case IC_BLADE:
				case IC_THROWING_KNIFE:
					bKnifeClass = 0;
					break;
				case IC_LAUNCHER:
					fGrenadeLauncher = FALSE;
					fMortar = FALSE;
					break;
				case IC_ARMOUR:
					if( i == HELMETPOS )
						bHelmetClass = 0;
					else if( i == VESTPOS )
						bVestClass = 0;
					else if( i == LEGPOS )
						bLeggingClass = 0;
					break;
				case IC_GRENADE:
					bGrenades = 0;
					bGrenadeClass = 0;
					break;
				case IC_MEDKIT:
				case IC_KIT:
					bKitClass = 0;
					break;
				case IC_MISC:
					bMiscClass = 0;
					break;
				case IC_BOMB:
					bBombClass = 0;
					break;
			}
		}
	}


	// special: militia shouldn't drop bombs
	if ( !( SOLDIER_CLASS_ENEMY( bSoldierClass ) ) )
	{
		bBombClass = 0;
	}


	//Now actually choose the equipment!
	ChooseWeaponForSoldierCreateStruct( pp, bWeaponClass, bAmmoClips, bAttachClass, fAttachment );
	ChooseGrenadesForSoldierCreateStruct( pp, bGrenades, bGrenadeClass, fGrenadeLauncher );
	ChooseArmourForSoldierCreateStruct( pp, bHelmetClass, bVestClass, bLeggingClass );
	ChooseSpecialWeaponsForSoldierCreateStruct( pp, bKnifeClass, fGrenadeLauncher, fLAW, fMortar );
	ChooseFaceGearForSoldierCreateStruct( pp );
	ChooseKitsForSoldierCreateStruct( pp, bKitClass );
	ChooseMiscGearForSoldierCreateStruct( pp, bMiscClass );
	ChooseBombsForSoldierCreateStruct( pp, bBombClass );
	ChooseLocationSpecificGearForSoldierCreateStruct( pp );
	RandomlyChooseWhichItemsAreDroppable( pp, bSoldierClass );
}


static BOOLEAN PlaceObjectInSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp, OBJECTTYPE* pObject);
static UINT16 SelectStandardArmyGun(UINT8 uiGunLevel);


//When using the class values, they should all range from 0-11, 0 meaning that there will be no
//selection for that particular type of item, and 1-11 means to choose an item if possible.  1 is
//the worst class of item, while 11 is the best.
static void ChooseWeaponForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp, INT8 bWeaponClass, INT8 bAmmoClips, INT8 bAttachClass, BOOLEAN fAttachment)
{
	OBJECTTYPE Object;
	UINT16 i;
	UINT16 usRandom;
	UINT16 usNumMatches = 0;
	UINT16 usGunIndex = 0;
	UINT16 usAmmoIndex = 0;
	UINT16 usAttachIndex = 0;
	UINT8 ubChanceStandardAmmo;
	int bStatus;

	// Choose weapon:
	// WEAPONS are very important, and are therefore handled differently using special pre-generated tables.
	// It was requested that enemies use only a small subset of guns with a lot duplication of the same gun type.

	// if gun was pre-selected (rcvd negative weapon class) and needs ammo
	if( bWeaponClass < 0 && bAmmoClips )
	{
		//Linda has added a specific gun to the merc's inventory, but no ammo.  So, we
		//will choose ammunition that works with the gun.
		for( i = 0; i < NUM_INV_SLOTS; i++ )
		{
			if( GCM->getItem(pp->Inv[ i ].usItem)->getItemClass() == IC_GUN )
			{
				usGunIndex = pp->Inv[ i ].usItem;
				ubChanceStandardAmmo = 100 - (bWeaponClass * -9);		// weapon class is negative!
				usAmmoIndex = RandomMagazine( usGunIndex, ubChanceStandardAmmo );

				if ( usGunIndex == ROCKET_RIFLE )
				{
					pp->Inv[ i ].ubImprintID = (NO_PROFILE + 1);
				}

				break;
			}
		}
		if( bAmmoClips && usAmmoIndex )
		{
			CreateItems( usAmmoIndex, 100, bAmmoClips, &Object );
			Object.fFlags |= OBJECT_UNDROPPABLE;
			PlaceObjectInSoldierCreateStruct( pp, &Object );
		}

		return;
	}

	if (bWeaponClass < 1)
		return; //empty handed / pre-selected

	// reduce anything over 11 to 11
	if (bWeaponClass > ARMY_GUN_LEVELS)
	{
		bWeaponClass = ARMY_GUN_LEVELS;
	}


	// the weapon class here ranges from 1 to 11, so subtract 1 to get a gun level
	usGunIndex = SelectStandardArmyGun( (UINT8) (bWeaponClass - 1));


	if( bAmmoClips )
	{
		//We have a gun, so choose ammo clips

		// check default ammo first...
		usAmmoIndex = DefaultMagazine( usGunIndex );
		switch( GCM->getItem(usAmmoIndex)->asAmmo()->ammoType->index )
		{
			case AMMO_AP:
			case AMMO_SUPER_AP:
				// assault rifle, rocket rifle (etc) - high chance of having AP ammo
				ubChanceStandardAmmo = 80;
				break;
			default:
				ubChanceStandardAmmo = 100 - (bWeaponClass * 9);
				break;
		}
		usAmmoIndex = RandomMagazine( usGunIndex, ubChanceStandardAmmo );
	}

	//Choose attachment
	if( bAttachClass && fAttachment )
	{
		usNumMatches = 0;
		while( bAttachClass && !usNumMatches )
		{
			//Count the number of valid attachments.
			for( i = 0; i < MAXITEMS; i++ )
			{
				const ItemModel * pItem = GCM->getItem(i);
				if( pItem->getItemClass() == IC_MISC && pItem->getCoolness() == bAttachClass && ValidAttachment( i, usGunIndex ) )
					usNumMatches++;
			}
			if( !usNumMatches )
			{
				bAttachClass--;
			}
		}
		if( usNumMatches )
		{
			usRandom = (UINT16)Random( usNumMatches );
			for( i = 0; i < MAXITEMS; i++ )
			{
				const ItemModel * pItem = GCM->getItem(i);
				if( pItem->getItemClass() == IC_MISC && pItem->getCoolness() == bAttachClass && ValidAttachment( i, usGunIndex ) )
				{
					if( usRandom )
						usRandom--;
					else
					{
						usAttachIndex = i;
						break;
					}
				}
			}
		}
	}
	//Now, we have chosen all of the correct items.  Now, we will assign them into the slots.
	//Because we are dealing with enemies, automatically give them full ammo in their weapon.
	if( !(pp->Inv[ HANDPOS ].fFlags & OBJECT_NO_OVERWRITE) )
	{
		switch( pp->ubSoldierClass )
		{
			case SOLDIER_CLASS_ADMINISTRATOR:
			case SOLDIER_CLASS_ARMY:
			case SOLDIER_CLASS_GREEN_MILITIA:
			case SOLDIER_CLASS_REG_MILITIA:
				//Admins/Troops: 60-75% + 1% every 4% progress
				bStatus = 60 + Random(16);
				bStatus += HighestPlayerProgressPercentage() / 4;
				bStatus = std::min(100, bStatus);
				break;
			case SOLDIER_CLASS_ELITE:
			case SOLDIER_CLASS_ELITE_MILITIA:
				//85-90% +  1% every 10% progress
				bStatus = 85 + Random(6);
				bStatus += HighestPlayerProgressPercentage() / 10;
				bStatus = std::min(100, bStatus);
				break;
			default:
				bStatus = 50 + Random(51);
				break;
		}
		// don't allow it to be lower than marksmanship, we don't want it to affect their chances of hitting
		bStatus = std::max(int(pp->bMarksmanship), bStatus);
		bStatus = std::max(bStatus, (int) gamepolicy(enemy_weapon_minimal_status));

		CreateItem( usGunIndex, bStatus, &(pp->Inv[ HANDPOS ]) );
		pp->Inv[ HANDPOS ].fFlags |= OBJECT_UNDROPPABLE;

		// Rocket Rifles must come pre-imprinted, in case carrier gets killed without getting a shot off
		if ( usGunIndex == ROCKET_RIFLE )
		{
			pp->Inv[ HANDPOS ].ubImprintID = (NO_PROFILE + 1);
		}
	}
	else
	{
		//slot locked, so don't add any attachments to it!
		usAttachIndex = 0;
	}
	//Ammo
	if( bAmmoClips && usAmmoIndex )
	{
		CreateItems( usAmmoIndex, 100, bAmmoClips, &Object );
		Object.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &Object );
	}
	if( usAttachIndex )
	{
		CreateItem( usAttachIndex, 100, &Object );
		Object.fFlags |= OBJECT_UNDROPPABLE;
		AttachObject( NULL, &(pp->Inv[ HANDPOS ]), &Object );
	}
}


static void ChooseGrenadesForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp, INT8 bGrenades, INT8 bGrenadeClass, BOOLEAN fGrenadeLauncher)
{
	OBJECTTYPE Object;
	INT16 sNumPoints;
	UINT16 usItem;
	UINT8 ubBaseQuality;
	UINT8 ubQualityVariation;
	//numbers of each type the player will get!
	UINT8 ubNumStun = 0;
	UINT8 ubNumTear = 0;
	UINT8 ubNumMustard = 0;
	UINT8 ubNumMini = 0;
	UINT8 ubNumReg = 0;
	UINT8 ubNumSmoke = 0;
	UINT8 ubNumFlare = 0;

	//determine how many *points* the enemy will get to spend on grenades...
	sNumPoints = bGrenades * bGrenadeClass;

	//no points, no grenades!
	if( !sNumPoints )
		return;

	// special mortar shell handling
	if (bGrenadeClass == MORTAR_GRENADE_CLASS)
	{
		CreateItems( MORTAR_SHELL, (INT8) (80 + Random(21)), bGrenades, &Object );
		Object.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &Object );
		return;
	}

	Assert( bGrenadeClass <= 11 );

	//determine the quality of grenades.  The elite guys get the best quality, while the others
	//get progressively worse.
	ubBaseQuality = (UINT8) std::min(45 + bGrenadeClass * 5, 90);
	ubQualityVariation = 101 - ubBaseQuality;


	//now, purchase the grenades.
	while( sNumPoints > 0 )
	{
		if( sNumPoints >= 20 )
		{ //Choose randomly between mustard and regular
			if( Random( 2 ) && !fGrenadeLauncher )
				ubNumMustard++, sNumPoints -= 10;
			else
				ubNumReg++, sNumPoints -= 9;
		}

		if( sNumPoints >= 10 )
		{ //Choose randomly between any
			switch( Random( 7 ) )
			{
				case 0:
					if ( !fGrenadeLauncher )
					{
						ubNumMustard++;
						sNumPoints -= 10;
						break;
					}
					// if grenade launcher, pick regular instead
					// fallthrough
				case 1:
					ubNumReg++;
					sNumPoints -= 9;
					break;
				case 2:
					if ( !fGrenadeLauncher )
					{
						ubNumMini++;
						sNumPoints -= 7;
						break;
					}
					// if grenade launcher, pick tear instead
					// fallthrough
				case 3:
					ubNumTear++;
					sNumPoints -= 6;
					break;
				case 4:
					ubNumStun++;
					sNumPoints -= 5;
					break;
				case 5:
					ubNumSmoke++;
					sNumPoints -= 4;
					break;
				case 6:
					if (!fGrenadeLauncher )
					{
						ubNumFlare++;
						sNumPoints -= 3;
					}
					break;
			}
		}

		// JA2 Gold: don't make mini-grenades take all points available, and add chance of break lights
		if( sNumPoints >= 1 && sNumPoints < 10 )
		{
			switch( Random( 10 ) )
			{
				case 0:
				case 1:
				case 2:
					ubNumSmoke++;
					sNumPoints -= 4;
					break;
				case 3:
				case 4:
					ubNumTear++;
					sNumPoints -= 6;
					break;
				case 5:
				case 6:
					if (!fGrenadeLauncher)
					{
						ubNumFlare++;
						sNumPoints -= 3;
					}
					break;
				case 7:
				case 8:
					ubNumStun++;
					sNumPoints -= 5;
					break;
				case 9:
					if (!fGrenadeLauncher)
					{
						ubNumMini++;
						sNumPoints -= 7;
					}
					break;
			}
		}
		/*
		if( usNumPoints >= 1 && usNumPoints < 10 )
		{ //choose randomly between either stun or tear, (normal with rare chance)
			switch( Random( 10 ) )
			{
				case 0:
				case 1:
				case 2:
				case 3:
					ubNumSmoke++;
					if( usNumPoints > 4 )
						usNumPoints -= 4;
					else
						usNumPoints = 0;
					break;
				case 4:
				case 5:
				case 6:
					ubNumTear++;
					if( usNumPoints > 6 )
						usNumPoints -= 6;
					else
						usNumPoints = 0;
					break;
				case 7:
				case 8:
					ubNumStun++;
					if( usNumPoints > 5 )
						usNumPoints -= 5;
					else
						usNumPoints = 0;
					break;
				case 9:
					ubNumMini++;
					usNumPoints = 0;
					break;
			}
		}*/
	}


	//Create the grenades and add them to the soldier

	if( ubNumSmoke )
	{
		if ( fGrenadeLauncher )
		{
			usItem = GL_SMOKE_GRENADE;
		}
		else
		{
			usItem = SMOKE_GRENADE;
		}
		CreateItems( usItem, (INT8)(ubBaseQuality + Random( ubQualityVariation )), ubNumSmoke, &Object );
		Object.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &Object );
	}
	if( ubNumTear )
	{
		if ( fGrenadeLauncher )
		{
			usItem = GL_TEARGAS_GRENADE;
		}
		else
		{
			usItem = TEARGAS_GRENADE;
		}
		CreateItems( usItem, (INT8)(ubBaseQuality + Random( ubQualityVariation )), ubNumTear, &Object );
		Object.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &Object );
	}
	if( ubNumStun )
	{
		if ( fGrenadeLauncher )
		{
			usItem = GL_STUN_GRENADE;
		}
		else
		{
			usItem = STUN_GRENADE;
		}
		CreateItems( usItem, (INT8)(ubBaseQuality + Random( ubQualityVariation )), ubNumStun, &Object );
		Object.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &Object );
	}
	if( ubNumReg )
	{
		if ( fGrenadeLauncher )
		{
			usItem = GL_HE_GRENADE;
		}
		else
		{
			usItem = HAND_GRENADE;
		}
		CreateItems( usItem, (INT8)(ubBaseQuality + Random( ubQualityVariation )), ubNumReg, &Object );
		Object.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &Object );
	}

	if( ubNumMini )
	{
		CreateItems( MINI_GRENADE, (INT8)(ubBaseQuality + Random( ubQualityVariation )), ubNumMini, &Object );
		Object.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &Object );
	}
	if( ubNumMustard )
	{
		CreateItems( MUSTARD_GRENADE, (INT8)(ubBaseQuality + Random( ubQualityVariation )), ubNumMustard, &Object );
		Object.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &Object );
	}
	if( ubNumFlare )
	{
		CreateItems( BREAK_LIGHT, (INT8)(ubBaseQuality + Random( ubQualityVariation )), ubNumFlare, &Object );
		Object.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &Object );
	}

}


static void ChooseArmourForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp, INT8 bHelmetClass, INT8 bVestClass, INT8 bLeggingsClass)
{
	UINT16 i;
	UINT16 usRandom;
	UINT16 usNumMatches;
	INT8 bOrigVestClass = bVestClass;
	OBJECTTYPE Object;

	//Choose helmet
	if( bHelmetClass )
	{
		usNumMatches = 0;
		while( bHelmetClass && !usNumMatches )
		{ //First step is to count the number of helmets in the helmet class range.  If we
			//don't find one, we keep lowering the class until we do.
			for( i = 0; i < MAXITEMS; i++ )
			{
				const ItemModel * pItem = GCM->getItem(i);
				// NOTE: This relies on treated armor to have a coolness of 0 in order for enemies not to be equipped with it
				if( pItem->getItemClass() == IC_ARMOUR && pItem->getCoolness() == bHelmetClass )
				{
					if( Armour[ pItem->getClassIndex() ].ubArmourClass == ARMOURCLASS_HELMET )
						usNumMatches++;
				}
			}
			if( !usNumMatches )
				bHelmetClass--;
		}
		if( usNumMatches )
		{ //There is a helmet that we can choose.
			usRandom = (UINT16)Random( usNumMatches );
			for( i = 0; i < MAXITEMS; i++ )
			{
				const ItemModel * pItem = GCM->getItem(i);
				if( pItem->getItemClass() == IC_ARMOUR && pItem->getCoolness() == bHelmetClass )
				{
					if( Armour[ pItem->getClassIndex() ].ubArmourClass == ARMOURCLASS_HELMET )
					{
						if( usRandom )
							usRandom--;
						else
						{
							if( !(pp->Inv[ HELMETPOS ].fFlags & OBJECT_NO_OVERWRITE) )
							{
								CreateItem( i, (INT8)(70+Random(31)), &(pp->Inv[ HELMETPOS ]) );
								pp->Inv[ HELMETPOS ].fFlags |= OBJECT_UNDROPPABLE;
							}
							break;
						}
					}
				}
			}
		}
	}
	//Choose vest
	if( bVestClass )
	{
		usNumMatches = 0;
		while( bVestClass && !usNumMatches )
		{
			//First step is to count the number of armours in the armour class range.  If we
			//don't find one, we keep lowering the class until we do.
			for( i = 0; i < MAXITEMS; i++ )
			{
				// these 3 have a non-zero coolness, and would otherwise be selected, so skip them
				if ( ( i == TSHIRT ) || ( i == LEATHER_JACKET ) || ( i == TSHIRT_DEIDRANNA ) )
					continue;

				const ItemModel * pItem = GCM->getItem(i);
				// NOTE: This relies on treated armor to have a coolness of 0 in order for enemies not to be equipped with it
				if( pItem->getItemClass() == IC_ARMOUR && pItem->getCoolness() == bVestClass )
				{
					if( Armour[ pItem->getClassIndex() ].ubArmourClass == ARMOURCLASS_VEST )
						usNumMatches++;
				}
			}
			if( !usNumMatches )
				bVestClass--;
		}
		if( usNumMatches )
		{
			//There is an armour that we can choose.
			usRandom = (UINT16)Random( usNumMatches );
			for( i = 0; i < MAXITEMS; i++ )
			{
				// these 3 have a non-zero coolness, and would otherwise be selected, so skip them
				if ( ( i == TSHIRT ) || ( i == LEATHER_JACKET ) || ( i == TSHIRT_DEIDRANNA ) )
					continue;

				const ItemModel * pItem = GCM->getItem(i);
				if( pItem->getItemClass() == IC_ARMOUR && pItem->getCoolness() == bVestClass )
				{
					if( Armour[ pItem->getClassIndex() ].ubArmourClass == ARMOURCLASS_VEST )
					{
						if( usRandom )
							usRandom--;
						else
						{
							if( !(pp->Inv[ VESTPOS ].fFlags & OBJECT_NO_OVERWRITE) )
							{
								CreateItem( i, (INT8)(70+Random(31)), &(pp->Inv[ VESTPOS ]) );
								pp->Inv[ VESTPOS ].fFlags |= OBJECT_UNDROPPABLE;

								if ( ( i == KEVLAR_VEST ) || ( i == SPECTRA_VEST ) )
								{
									// roll to see if he gets a CERAMIC PLATES, too.
									// Higher chance the higher his entitled vest class is
									if ((INT8) Random(100) < (15 * (bOrigVestClass - pItem->getCoolness())))
									{
										CreateItem( CERAMIC_PLATES, (INT8)(70+Random(31)), &Object );
										Object.fFlags |= OBJECT_UNDROPPABLE;
										AttachObject( NULL, &(pp->Inv[ VESTPOS ]), &Object );
									}
								}
							}
							break;
						}
					}
				}
			}
		}
	}
	//Choose Leggings
	if( bLeggingsClass )
	{
		usNumMatches = 0;
		while( bLeggingsClass && !usNumMatches )
		{ //First step is to count the number of Armours in the Armour class range.  If we
			//don't find one, we keep lowering the class until we do.
			for( i = 0; i < MAXITEMS; i++ )
			{
				const ItemModel * pItem = GCM->getItem(i);
				// NOTE: This relies on treated armor to have a coolness of 0 in order for enemies not to be equipped with it
				if( pItem->getItemClass() == IC_ARMOUR && pItem->getCoolness() == bLeggingsClass )
				{
					if( Armour[ pItem->getClassIndex() ].ubArmourClass == ARMOURCLASS_LEGGINGS )
						usNumMatches++;
				}
			}
			if( !usNumMatches )
				bLeggingsClass--;
		}
		if( usNumMatches )
		{
			//There is a legging that we can choose.
			usRandom = (UINT16)Random( usNumMatches );
			for( i = 0; i < MAXITEMS; i++ )
			{
				const ItemModel * pItem = GCM->getItem(i);
				if( pItem->getItemClass() == IC_ARMOUR && pItem->getCoolness() == bLeggingsClass )
				{
					if( Armour[ pItem->getClassIndex() ].ubArmourClass == ARMOURCLASS_LEGGINGS )
					{
						if( usRandom )
							usRandom--;
						else
						{
							if( !(pp->Inv[ LEGPOS ].fFlags & OBJECT_NO_OVERWRITE) )
							{
								CreateItem( i, (INT8)(70+Random(31)), &(pp->Inv[ LEGPOS ]) );
								pp->Inv[ LEGPOS ].fFlags |= OBJECT_UNDROPPABLE;
								break;
							}
						}
					}
				}
			}
		}
	}
}


static void ChooseSpecialWeaponsForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp, INT8 bKnifeClass, BOOLEAN fGrenadeLauncher, BOOLEAN fLAW, BOOLEAN fMortar)
{
	UINT16 i;
	UINT16 usRandom;
	UINT16 usNumMatches = 0;
	UINT16 usKnifeIndex = 0;
	OBJECTTYPE Object;

	//Choose knife
	while( bKnifeClass && !usNumMatches )
	{
		//First step is to count the number of weapons in the weapon class range.  If we
		//don't find one, we keep lowering the class until we do.
		for( i = 0; i < MAXITEMS; i++ )
		{
			const ItemModel * pItem = GCM->getItem(i);
			if( ( pItem->getItemClass() == IC_BLADE || pItem->getItemClass() == IC_THROWING_KNIFE ) && pItem->getCoolness() == bKnifeClass )
			{
				usNumMatches++;
			}
		}
		if( !usNumMatches )
			bKnifeClass--;
	}
	if( usNumMatches )
	{
		//There is a knife that we can choose.
		usRandom = (UINT16)Random( usNumMatches );
		for( i = 0; i < MAXITEMS; i++ )
		{
			const ItemModel * pItem = GCM->getItem(i);
			if( ( pItem->getItemClass() == IC_BLADE || pItem->getItemClass() == IC_THROWING_KNIFE ) && pItem->getCoolness() == bKnifeClass )
			{
				if( usRandom )
					usRandom--;
				else
				{
					usKnifeIndex = i;
					break;
				}
			}
		}
	}

	if( usKnifeIndex )
	{
		CreateItem( usKnifeIndex, (INT8)(70 + Random( 31 )), &Object );
		Object.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &Object );
	}

	if (fGrenadeLauncher)
	{
		// give grenade launcher
		CreateItem( GLAUNCHER, (INT8)(50 + Random( 51 )), &Object );
		Object.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &Object );
	}

	if (fLAW)
	{
		// give rocket launcher
		CreateItem( ROCKET_LAUNCHER, (INT8)(50 + Random( 51 )), &Object );
		Object.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &Object );
	}

	if (fMortar)
	{
		// make sure we're not distributing them underground!
		Assert(IsAutoResolveActive() || gWorldSector.z == 0);

		// give mortar
		CreateItem( MORTAR, (INT8)(50 + Random( 51 )), &Object );
		Object.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &Object );
	}
}


static void ChooseFaceGearForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp)
{
	INT32 i;
	INT8 bDifficultyRating = CalcDifficultyModifier( pp->ubSoldierClass );

	if (gWorldSector == tixa && StrategicMap[tixa.AsStrategicIndex()].fEnemyControlled)
	{
		//Tixa is a special case that is handled differently.
		return;
	}

	//Look for any face item in the big pocket positions (the only place they can be added in the editor)
	//If any are found, then don't assign any
	for( i = BIGPOCK1POS; i < BIGPOCK4POS; i++ )
	{
		if( GCM->getItem(pp->Inv[ i ].usItem)->getItemClass() == IC_FACE )
		{
			return;
		}
	}

	//KM: (NEW)
	//Note the lack of overwritable item checking here.  This is because faceitems are not
	//supported in the editor, hence they can't have this status.
	switch( pp->ubSoldierClass )
	{
		case SOLDIER_CLASS_ELITE:
		case SOLDIER_CLASS_ELITE_MILITIA:
			//All elites get a gasmask and either night goggles or uv goggles.
			if( Chance( 75 ) )
			{
				CreateItem( GASMASK, (INT8)(70+Random(31)), &(pp->Inv[ HEAD1POS ]) );
				pp->Inv[ HEAD1POS ].fFlags |= OBJECT_UNDROPPABLE;
			}
			else
			{
				CreateItem( EXTENDEDEAR, (INT8)(70+Random(31)), &(pp->Inv[ HEAD1POS ]) );
				pp->Inv[ HEAD1POS ].fFlags |= OBJECT_UNDROPPABLE;
			}
			if( Chance( 75 ) )
			{
				CreateItem( NIGHTGOGGLES, (INT8)(70+Random(31)), &(pp->Inv[ HEAD2POS ]) );
				pp->Inv[ HEAD2POS ].fFlags |= OBJECT_UNDROPPABLE;
			}
			else
			{
				CreateItem( UVGOGGLES, (INT8)(70+Random(31)), &(pp->Inv[ HEAD2POS ]) );
				pp->Inv[ HEAD2POS ].fFlags |= OBJECT_UNDROPPABLE;
			}
			break;
		case SOLDIER_CLASS_ARMY:
		case SOLDIER_CLASS_REG_MILITIA:
			if( Chance( bDifficultyRating / 2 ) )
			{
				//chance of getting a face item
				if( Chance( 50 ) )
				{
					CreateItem( GASMASK, (INT8)(70+Random(31)), &(pp->Inv[ HEAD1POS ]) );
					pp->Inv[ HEAD1POS ].fFlags |= OBJECT_UNDROPPABLE;
				}
				else
				{
					CreateItem( NIGHTGOGGLES, (INT8)(70+Random(31)), &(pp->Inv[ HEAD1POS ]) );
					pp->Inv[ HEAD1POS ].fFlags |= OBJECT_UNDROPPABLE;
				}
			}
			if( Chance( bDifficultyRating / 3 ) )
			{
				//chance of getting a extended ear
				CreateItem( EXTENDEDEAR, (INT8)(70+Random(31)), &(pp->Inv[ HEAD2POS ]) );
				pp->Inv[ HEAD2POS ].fFlags |= OBJECT_UNDROPPABLE;
			}
			break;
		case SOLDIER_CLASS_ADMINISTRATOR:
		case SOLDIER_CLASS_GREEN_MILITIA:
			break;
	}
}


static void ChooseKitsForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp, INT8 bKitClass)
{
	UINT16 i;
	UINT16 usRandom;
	UINT16 usNumMatches = 0;
	OBJECTTYPE Object;
	UINT16 usKitItem = 0;


	// we want these mostly to be first aid and medical kits, and for those kit class doesn't matter, they're always useful
	if ( Chance( 50 ) )
	{
		usKitItem = FIRSTAIDKIT;
	}
	else
	if ( Chance( 25 ) )
	{
		usKitItem = MEDICKIT;
	}
	else
	{
		// count how many non-medical KITS are eligible ( of sufficient or lower coolness)
		for( i = 0; i < MAXITEMS; i++ )
		{
			const ItemModel * pItem = GCM->getItem(i);
			// skip toolkits
			if( ( pItem->getItemClass() == IC_KIT ) && ( pItem->getCoolness() > 0 ) && pItem->getCoolness() <= bKitClass && ( i != TOOLKIT ) )
			{
				usNumMatches++;
			}
		}

		// if any are eligible, pick one of them at random
		if( usNumMatches )
		{
			usRandom = (UINT16)Random( usNumMatches );
			for( i = 0; i < MAXITEMS; i++ )
			{
				const ItemModel * pItem = GCM->getItem(i);
				// skip toolkits
				if((pItem->getItemClass() == IC_KIT) && (pItem->getCoolness() > 0) &&
					pItem->getCoolness() <= bKitClass && (i != TOOLKIT))
				{
					if( usRandom )
						usRandom--;
					else
					{
						usKitItem = i;
						break;
					}
				}
			}
		}
	}


	if ( usKitItem != 0 )
	{
		CreateItem( usKitItem, (INT8)(80 + Random( 21 )), &Object );
		Object.fFlags |= OBJECT_UNDROPPABLE;
		PlaceObjectInSoldierCreateStruct( pp, &Object );
	}
}


static void ChooseMiscGearForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp, INT8 bMiscClass)
{
	// not all of these are IC_MISC, some are IC_PUNCH (not covered anywhere else)
	static const INT32 iMiscItemsList[] =
	{
		CANTEEN,
		CANTEEN,
		CANTEEN,
		CANTEEN,
		ALCOHOL,
		ALCOHOL,
		ADRENALINE_BOOSTER,
		ADRENALINE_BOOSTER,
		REGEN_BOOSTER,
		BRASS_KNUCKLES,
		CHEWING_GUM,
		CIGARS,
		GOLDWATCH,
		NOTHING
	};

	// count how many are eligible
	UINT16 usNumMatches = 0;
	for (const INT32* i = iMiscItemsList; *i != NOTHING; ++i)
	{
		const ItemModel * pItem = GCM->getItem(*i);
		if (pItem->getCoolness() > 0 &&
			pItem->getCoolness() <= bMiscClass &&
			(*i != REGEN_BOOSTER || gGameOptions.fSciFi)) // exclude REGEN_BOOSTERs unless Sci-Fi flag is on
		{
			++usNumMatches;
		}
	}

	// if any are eligible, pick one of them at random
	if (usNumMatches == 0) return;

	UINT16 usRandom = Random(usNumMatches);
	for (const INT32* i = iMiscItemsList; *i != NOTHING; ++i)
	{
		const ItemModel * pItem = GCM->getItem(*i);
		if (pItem->getCoolness() > 0 &&
			pItem->getCoolness() <= bMiscClass &&
			(*i != REGEN_BOOSTER || gGameOptions.fSciFi)) // exclude REGEN_BOOSTERs unless Sci-Fi flag is on
		{
			if (usRandom)
			{
				--usRandom;
			}
			else
			{
				OBJECTTYPE Object;
				CreateItem(*i, 80 + Random(21), &Object);
				Object.fFlags |= OBJECT_UNDROPPABLE;
				PlaceObjectInSoldierCreateStruct(pp, &Object);
				break;
			}
		}
	}
}


static void ChooseBombsForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp, INT8 bBombClass)
{
	UINT16 i;
	UINT16 usRandom;
	UINT16 usNumMatches = 0;
	OBJECTTYPE Object;

	// count how many are eligible
	for( i = 0; i < MAXITEMS; i++ )
	{
		const ItemModel * pItem = GCM->getItem(i);
		if( ( pItem->getItemClass() == IC_BOMB ) && ( pItem->getCoolness() > 0 ) && ( pItem->getCoolness() <= bBombClass ) )
		{
			usNumMatches++;
		}
	}


	// if any are eligible, pick one of them at random
	if( usNumMatches )
	{
		usRandom = (UINT16)Random( usNumMatches );
		for( i = 0; i < MAXITEMS; i++ )
		{
			const ItemModel * pItem = GCM->getItem(i);
			if( ( pItem->getItemClass() == IC_BOMB ) && ( pItem->getCoolness() > 0 ) && ( pItem->getCoolness() <= bBombClass ) )
			{
				if( usRandom )
					usRandom--;
				else
				{
					CreateItem( i, (INT8)(80 + Random( 21 )), &Object );
					Object.fFlags |= OBJECT_UNDROPPABLE;
					PlaceObjectInSoldierCreateStruct( pp, &Object );
					break;
				}
			}
		}
	}
}


static void ChooseLocationSpecificGearForSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp)
{
	OBJECTTYPE Object;

	// If this is Tixa and the player doesn't control Tixa then give all enemies gas masks,
	// but somewhere on their person, not in their face positions
	if (gWorldSector == tixa && StrategicMap[tixa.AsStrategicIndex()].fEnemyControlled)
	{
		CreateItem( GASMASK, (INT8) (95+Random(6)), &Object );
		PlaceObjectInSoldierCreateStruct( pp, &Object );
	}
}


static BOOLEAN PlaceObjectInSoldierCreateStruct(SOLDIERCREATE_STRUCT* pp, OBJECTTYPE* pObject)
{
	INT8 i;
	if( !GCM->getItem(pObject->usItem )->getPerPocket() )
	{
		//ubPerPocket == 0 will only fit in large pockets.
		pObject->ubNumberOfObjects = 1;
		for( i = BIGPOCK1POS; i <= BIGPOCK4POS; i++ )
		{
			if( !(pp->Inv[ i ].usItem) && !(pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE) )
			{
				pp->Inv[i] = *pObject;
				return TRUE;
			}
		}
		return FALSE;
	}
	else
	{
		pObject->ubNumberOfObjects = (UINT8) std::min(GCM->getItem(pObject->usItem )->getPerPocket(), pObject->ubNumberOfObjects);
		//try to get it into a small pocket first
		for( i = SMALLPOCK1POS; i <= SMALLPOCK8POS; i++ )
		{
			if( !(pp->Inv[ i ].usItem) && !(pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE) )
			{
				pp->Inv[i] = *pObject;
				return TRUE;
			}
		}
		for( i = BIGPOCK1POS; i <= BIGPOCK4POS; i++ )
		{
			//no space free in small pockets, so put it into a large pocket.
			if( !(pp->Inv[ i ].usItem) && !(pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE) )
			{
				pp->Inv[i] = *pObject;
				return TRUE;
			}
		}
	}
	return FALSE;
}


static void MakeOneItemOfClassDroppable(SOLDIERCREATE_STRUCT* const sc, UINT32 const item_class)
{
	// XXX TODO001B: OBJECT_NO_OVERWRITE test should probably continue instead of
	// break, but it was this way in the original code.  This is even more
	// plausible, because the OBJECT_NO_OVERWRITE condition in the second loop
	// never can be true in the current configuration.  A comment below says that
	// no object of that class should be dropped if any has this flag set, but the
	// code did not do this.
	UINT8 n_matches = 0;
	for (INT32 i = 0; i != NUM_INV_SLOTS; ++i)
	{
		OBJECTTYPE const& o = sc->Inv[i];
		if (!(GCM->getItem(o.usItem)->getItemClass() & item_class))
			continue;
		if (o.fFlags & OBJECT_NO_OVERWRITE)
			break;
		++n_matches;
	}
	if (n_matches == 0) return;

	for (INT32 i = 0; i != NUM_INV_SLOTS; ++i)
	{
		OBJECTTYPE& o = sc->Inv[i];
		if (!(GCM->getItem(o.usItem)->getItemClass() & item_class))
			continue;
		if (o.fFlags & OBJECT_NO_OVERWRITE)
			break;
		if (Random(n_matches--) != 0)
			continue;
		o.fFlags &= ~OBJECT_UNDROPPABLE;
		break;
	}
}


static void RandomlyChooseWhichItemsAreDroppable(SOLDIERCREATE_STRUCT* pp, INT8 bSoldierClass)
{
	INT32 i;
	//UINT16 usRandomNum;
	UINT32 uiItemClass;
	UINT16 usItem;
	UINT8 ubAmmoDropRate;
	UINT8 ubGrenadeDropRate;
	UINT8 ubOtherDropRate;
	BOOLEAN fWeapon = FALSE;
	BOOLEAN fGrenades = FALSE; // this includes all grenades!
	BOOLEAN fAmmo = FALSE;
	BOOLEAN fArmour = FALSE;
	BOOLEAN fKnife = FALSE;
	BOOLEAN fKit = FALSE;
	BOOLEAN fFace = FALSE;
	BOOLEAN fMisc = FALSE;


	/*
	//40% of soldiers will have droppable items.
	usRandomNum = (UINT16)Random( 1000 );
	if( usRandomNum >= 400 )
		return;
	//so now the number is 0-399.  This is kind of like a D&D die roll where
	//various numbers drop different items, or even more than one item.  At this
	//point, we don't care if the enemy has anything in the slot that is made droppable.
	//Any items containing the OBJECT_NO_OVERWRITE slot is rejected for droppable
	//consideration.

	if( usRandomNum < 32 ) //3.2% of dead bodies present the possibility of several items (0-5 items : avg 3).
	{
		//31 is the magic number that allows all 5 item classes to be dropped!
		if( usRandomNum & 16 )
			fWeapon = TRUE;
		if( usRandomNum & 8 )
			fAmmo = TRUE;
		if( usRandomNum & 4 )
			fGrenades = TRUE;
		if( usRandomNum & 2 )
			fArmour = TRUE;
		if( usRandomNum & 1 )
			fMisc = TRUE;
	}
	else if( usRandomNum < 100 ) //6.8% chance of getting 2-3 different items.
	{
		//do a more generalized approach to dropping items.
		switch( usRandomNum / 10 )
		{
			case 3:
				fWeapon = TRUE;
				fAmmo = TRUE;
				break;
			case 4:
				fWeapon = TRUE;
				fGrenades = TRUE;
				break;
			case 5:
				fGrenades = TRUE;
				fMisc = TRUE;
				break;
			case 6:
				fGrenades = TRUE;
				fArmour = TRUE;
				break;
			case 7:
				fAmmo = TRUE;
				fArmour = TRUE;
				break;
			case 8:
				fAmmo = TRUE;
				fArmour = TRUE;
				fMisc = TRUE;
				break;
			case 9:
				fGrenades = TRUE;
				fAmmo = TRUE;
				fMisc = TRUE;
				break;
		}
	}
	else
	{
		switch( usRandomNum / 50 ) //30% chance of getting 1-2 items (no weapons)
		{
			case 2:
				fGrenades = TRUE;
				break;
			case 3:
				fAmmo = TRUE;
				break;
			case 4:
				fArmour = TRUE;
				break;
			case 5:
				fMisc = TRUE;
				break;
			case 6:
				fAmmo = TRUE;
				fMisc = TRUE;
				break;
			case 7:
				fGrenades = TRUE;
				fAmmo = TRUE;
				break;
		}
	}

	fKnife = (Random(3)) ? FALSE : TRUE;*/


	// only enemy soldiers use auto-drop system!
	// don't use the auto-drop system in auto-resolve: player won't see what's being used & enemies will often win & keep'em
	if ( SOLDIER_CLASS_ENEMY( bSoldierClass ) && !IsAutoResolveActive() )
	{
		// SPECIAL handling for weapons: we'll always drop a weapon type that has never been dropped before
		for( i = 0; i < NUM_INV_SLOTS; i++ )
		{
			usItem = pp->Inv[ i ].usItem;
			// if it's a weapon (monster parts included - they won't drop due to checks elsewhere!)
			if ((usItem > NONE) && (usItem < MAX_WEAPONS))
			{
				// and we're allowed to change its flags
				if(! (pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE ))
				{
					// and it's never been dropped before in this game
					if (!gStrategicStatus.fWeaponDroppedAlready[usItem])
					{
						// mark it as droppable, and remember we did so.  If the player never kills this particular dude,
						// oh well, tough luck, he missed his chance for an easy reward, he'll have to wait til next
						// time and need some luck...
						pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;

						MarkAllWeaponsOfSameGunClassAsDropped( usItem );
					}
				}
			}
		}
	}


	if ( SOLDIER_CLASS_MILITIA( bSoldierClass ) )
	{
		// militia (they drop much less stuff)
		ubAmmoDropRate = MILITIAAMMODROPRATE;
		ubGrenadeDropRate = MILITIAGRENADEDROPRATE;
		ubOtherDropRate = MILITIAEQUIPDROPRATE;
	}
	else
	{
		// enemy army
		ubAmmoDropRate = ENEMYAMMODROPRATE;
		ubGrenadeDropRate = ENEMYGRENADEDROPRATE;
		ubOtherDropRate = ENEMYEQUIPDROPRATE;
	}



	if( Random(100) < ubAmmoDropRate )
		fAmmo = TRUE;

	if( Random(100) < ubOtherDropRate )
		fWeapon = TRUE;

	if( Random(100) < ubOtherDropRate )
		fArmour = TRUE;

	if( Random(100) < ubOtherDropRate )
		fKnife = TRUE;

	if( Random(100) < ubGrenadeDropRate )
		fGrenades = TRUE;

	if( Random(100) < ubOtherDropRate )
		fKit = TRUE;

	if( Random(100) < (UINT32)(ubOtherDropRate / 3) )
		fFace = TRUE;

	if( Random(100) < ubOtherDropRate )
		fMisc = TRUE;


	//Now, that the flags are set for each item, we now have to search through the item slots to
	//see if we can find a matching item, however, if we find any items in a particular class that
	//have the OBJECT_NO_OVERWRITE flag set, we will not make any items droppable for that class
	//because the editor would have specified it already.
	if( fAmmo )
	{
		// now drops ALL ammo found, not just the first slot
		for( i = 0; i < NUM_INV_SLOTS; i++ )
		{
			uiItemClass = GCM->getItem(pp->Inv[ i ].usItem)->getItemClass();
			if( uiItemClass == IC_AMMO )
			{
				if( pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
					continue;
				else
				{
					pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
				}
			}
		}
	}

	if (fWeapon) MakeOneItemOfClassDroppable(pp, IC_LAUNCHER | IC_GUN);
	if (fArmour) MakeOneItemOfClassDroppable(pp, IC_ARMOUR);

	if( fKnife)
	{
		for( i = 0; i < NUM_INV_SLOTS; i++ )
		{
			// drops FIRST knife found
			uiItemClass = GCM->getItem(pp->Inv[ i ].usItem)->getItemClass();
			if( uiItemClass == IC_BLADE || uiItemClass == IC_THROWING_KNIFE )
			{
				if( pp->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
					break;
				else
				{
					pp->Inv[ i ].fFlags &= ~OBJECT_UNDROPPABLE;
					break;
				}
			}
		}
	}

	// note that they'll only drop ONE TYPE of grenade if they have multiple types (very common)
	if (fGrenades) MakeOneItemOfClassDroppable(pp, IC_GRENADE);
	if (fKit)      MakeOneItemOfClassDroppable(pp, IC_KIT | IC_MEDKIT);
	if (fFace)     MakeOneItemOfClassDroppable(pp, IC_FACE);
	if (fMisc)     MakeOneItemOfClassDroppable(pp, IC_MISC);
}


void AssignCreatureInventory( SOLDIERTYPE *pSoldier )
{
	UINT32 uiChanceToDrop = 0;
	BOOLEAN fMaleCreature = FALSE;
	BOOLEAN fBloodcat = FALSE;

	// all creature items in this first section are only offensive/defensive placeholders, and
	// never get dropped, because they're not real items!
	switch(pSoldier->ubBodyType)
	{
		case ADULTFEMALEMONSTER:
			CreateItem(CREATURE_OLD_FEMALE_CLAWS, 100, &(pSoldier->inv[HANDPOS]));
			CreateItem(CREATURE_OLD_FEMALE_HIDE, 100, &(pSoldier->inv[HELMETPOS]));
			CreateItem(CREATURE_OLD_FEMALE_HIDE, 100, &(pSoldier->inv[VESTPOS]));
			CreateItem(CREATURE_OLD_FEMALE_HIDE, 100, &(pSoldier->inv[LEGPOS]));
			uiChanceToDrop = 30;
			break;
		case AM_MONSTER:
			CreateItem(CREATURE_OLD_MALE_CLAWS, 100, &(pSoldier->inv[HANDPOS]));
			CreateItem(CREATURE_OLD_MALE_SPIT, 100, &(pSoldier->inv[SECONDHANDPOS]));
			CreateItem(CREATURE_OLD_MALE_HIDE, 100, &(pSoldier->inv[HELMETPOS]));
			CreateItem(CREATURE_OLD_MALE_HIDE, 100, &(pSoldier->inv[VESTPOS]));
			CreateItem(CREATURE_OLD_MALE_HIDE, 100, &(pSoldier->inv[LEGPOS]));
			uiChanceToDrop = 30;
			fMaleCreature = TRUE;
			break;
		case YAF_MONSTER:
			CreateItem(CREATURE_YOUNG_FEMALE_CLAWS, 100, &(pSoldier->inv[HANDPOS]));
			CreateItem(CREATURE_YOUNG_FEMALE_HIDE, 100, &(pSoldier->inv[HELMETPOS]));
			CreateItem(CREATURE_YOUNG_FEMALE_HIDE, 100, &(pSoldier->inv[VESTPOS]));
			CreateItem(CREATURE_YOUNG_FEMALE_HIDE, 100, &(pSoldier->inv[LEGPOS]));
			uiChanceToDrop = 15;
			break;
		case YAM_MONSTER:
			CreateItem(CREATURE_YOUNG_MALE_CLAWS, 100, &(pSoldier->inv[HANDPOS]));
			CreateItem(CREATURE_YOUNG_MALE_SPIT, 100, &(pSoldier->inv[SECONDHANDPOS]));
			CreateItem(CREATURE_YOUNG_MALE_HIDE, 100, &(pSoldier->inv[HELMETPOS]));
			CreateItem(CREATURE_YOUNG_MALE_HIDE, 100, &(pSoldier->inv[VESTPOS]));
			CreateItem(CREATURE_YOUNG_MALE_HIDE, 100, &(pSoldier->inv[LEGPOS]));
			uiChanceToDrop = 15;
			fMaleCreature = TRUE;
			break;
		case INFANT_MONSTER:
			CreateItem(CREATURE_INFANT_SPIT, 100, &(pSoldier->inv[HANDPOS]));
			CreateItem(CREATURE_INFANT_HIDE, 100, &(pSoldier->inv[HELMETPOS]));
			CreateItem(CREATURE_INFANT_HIDE, 100, &(pSoldier->inv[VESTPOS]));
			CreateItem(CREATURE_INFANT_HIDE, 100, &(pSoldier->inv[LEGPOS]));
			uiChanceToDrop = 5;
			break;
		case LARVAE_MONSTER:
			uiChanceToDrop = 0;
			break;
		case QUEENMONSTER:
			CreateItem(CREATURE_QUEEN_SPIT, 100, &(pSoldier->inv[HANDPOS]));
			CreateItem(CREATURE_QUEEN_TENTACLES, 100, &(pSoldier->inv[SECONDHANDPOS]));
			CreateItem(CREATURE_QUEEN_HIDE, 100, &(pSoldier->inv[HELMETPOS]));
			CreateItem(CREATURE_QUEEN_HIDE, 100, &(pSoldier->inv[VESTPOS]));
			CreateItem(CREATURE_QUEEN_HIDE, 100, &(pSoldier->inv[LEGPOS]));
			// she can't drop anything, because the items are unreachable anyways (she's too big!)
			uiChanceToDrop = 0;
			break;
		case BLOODCAT:
			CreateItem(BLOODCAT_CLAW_ATTACK, 100, &(pSoldier->inv[HANDPOS]));
			CreateItem(BLOODCAT_BITE, 100, &(pSoldier->inv[SECONDHANDPOS]));
			fBloodcat = TRUE;
			uiChanceToDrop = 30;
			break;

		default:
			AssertMsg(FALSE, String("Invalid creature bodytype %d", pSoldier->ubBodyType));
			return;
	}

	// decide if the creature will drop any REAL bodyparts
	if (Random(100) < uiChanceToDrop)
	{
		CreateItem( (UINT16)(fBloodcat ? BLOODCAT_CLAWS : CREATURE_PART_CLAWS), (INT8) (80 + Random(21)), &(pSoldier->inv[BIGPOCK1POS]) );
	}

	if (Random(100) < uiChanceToDrop)
	{
		CreateItem( (UINT16)(fBloodcat ? BLOODCAT_TEETH : CREATURE_PART_FLESH), (INT8) (80 + Random(21)), &(pSoldier->inv[BIGPOCK2POS]) );
	}

	// as requested by ATE, males are more likely to drop their "organs" (he actually suggested this, I'm serious!)
	if (fMaleCreature)
	{
		// increase chance by 50%
		uiChanceToDrop += (uiChanceToDrop / 2);
	}

	if (Random(100) < uiChanceToDrop)
	{
		CreateItem( (UINT16)(fBloodcat ? BLOODCAT_PELT : CREATURE_PART_ORGAN), (INT8) (80 + Random(21)), &(pSoldier->inv[BIGPOCK3POS]) );
	}
}

void ReplaceExtendedGuns( SOLDIERCREATE_STRUCT *pp, INT8 bSoldierClass )
{
	UINT32 uiLoop, uiLoop2, uiAttachDestIndex;
	INT8   bWeaponClass;
	OBJECTTYPE OldObj;
	UINT16 usItem, usNewGun, usAmmo, usNewAmmo;

	for ( uiLoop = 0; uiLoop < NUM_INV_SLOTS; uiLoop++ )
	{
		usItem = pp->Inv[ uiLoop ].usItem;
		const ItemModel *item = GCM->getItem(usItem);
		const WeaponModel *weapon = item->asWeapon();

		if (weapon && weapon->isInBigGunList())
		{
			if ( bSoldierClass == SOLDIER_CLASS_NONE )
			{
				usNewGun = GCM->getWeaponByName(weapon->getStandardReplacement())->getItemIndex();
			}
			else
			{
				bWeaponClass = GetWeaponClass( usItem );
				AssertMsg( bWeaponClass != -1, String( "Gun %d does not have a match in the extended gun array", usItem ) );
				usNewGun = SelectStandardArmyGun( bWeaponClass );
			}

			if ( usNewGun != NOTHING )
			{
				// have to replace!  but first (ugh) must store backup (b/c of attachments)
				OldObj = pp->Inv[uiLoop];
				CreateItem( usNewGun, OldObj.bGunStatus, &(pp->Inv[ uiLoop ]) );
				pp->Inv[ uiLoop ].fFlags = OldObj.fFlags;

				// copy any valid attachments; for others, just drop them...
				if (ItemHasAttachments(OldObj))
				{
					// we're going to copy into the first attachment position first :-)
					uiAttachDestIndex = 0;
					// loop!
					for ( uiLoop2 = 0; uiLoop2 < MAX_ATTACHMENTS; uiLoop2++ )
					{
						if ( ( OldObj.usAttachItem[ uiLoop2 ] != NOTHING ) && ValidAttachment( OldObj.usAttachItem[ uiLoop2 ], usNewGun ) )
						{
							pp->Inv[ uiLoop ].usAttachItem[ uiAttachDestIndex ] = OldObj.usAttachItem[ uiLoop2 ];
							pp->Inv[ uiLoop ].bAttachStatus[ uiAttachDestIndex ] = OldObj.bAttachStatus[ uiLoop2 ];
							uiAttachDestIndex++;
						}
					}
				}

				// must search through inventory and replace ammo accordingly
				for ( uiLoop2 = 0; uiLoop2 < NUM_INV_SLOTS; uiLoop2++ )
				{
					usAmmo = pp->Inv[ uiLoop2 ].usItem;
					if ( (GCM->getItem(usAmmo)->isAmmo()) )
					{
						usNewAmmo = FindReplacementMagazineIfNecessary(item->asWeapon(), usAmmo, GCM->getWeapon(usNewGun) );
						if (usNewAmmo != NOTHING )
						{
							// found a new magazine, replace...
							CreateItems( usNewAmmo, 100, pp->Inv[ uiLoop2 ].ubNumberOfObjects, &( pp->Inv[ uiLoop2 ] ) );
						}
					}
				}
			}
		}
	}
}


static UINT16 SelectStandardArmyGun(UINT8 uiGunLevel)
{
	UINT32 uiChoice;
	UINT16 usGunIndex;

	const std::vector<std::vector<const WeaponModel*> > * gunChoice = NULL;

	if (gGameOptions.fGunNut)
	{
		gunChoice = &GCM->getExtendedGunChoice();
	}
	else
	{
		gunChoice = &GCM->getNormalGunChoice();
	}

	// choose one the of the possible gun choices
	uiChoice = Random(static_cast<UINT32>(gunChoice->at(uiGunLevel).size()));
	usGunIndex = (gunChoice->at(uiGunLevel)[uiChoice])->getItemIndex();

	Assert(usGunIndex);

	return(usGunIndex);
}


static void EquipTank(SOLDIERCREATE_STRUCT* pp)
{
	OBJECTTYPE Object;

	// tanks get special equipment, and they drop nothing (MGs are hard-mounted & non-removable)

	// main cannon
	CreateItem( TANK_CANNON, ( INT8 )( 80 + Random( 21 ) ), &( pp->Inv[ HANDPOS ]) );
	pp->Inv[ HANDPOS ].fFlags |= OBJECT_UNDROPPABLE;

	// machine gun
	CreateItems( MINIMI, ( INT8 )( 80 + Random( 21 ) ), 1, &Object );
	Object.fFlags |= OBJECT_UNDROPPABLE;
	PlaceObjectInSoldierCreateStruct( pp, &Object );

	// tanks don't deplete shells or ammo...
	CreateItems( TANK_SHELL, 100, 1, &Object );
	Object.fFlags |= OBJECT_UNDROPPABLE;
	PlaceObjectInSoldierCreateStruct( pp, &Object );

	// armour equal to spectra all over (for vs explosives)
	CreateItem( SPECTRA_VEST, 100, &(pp->Inv[ VESTPOS ]) );
	pp->Inv[ VESTPOS ].fFlags |= OBJECT_UNDROPPABLE;
	CreateItem( SPECTRA_HELMET, 100, &(pp->Inv[ HELMETPOS ]) );
	pp->Inv[ HELMETPOS ].fFlags |= OBJECT_UNDROPPABLE;
	CreateItem( SPECTRA_LEGGINGS, 100, &(pp->Inv[ LEGPOS ]) );
	pp->Inv[ LEGPOS ].fFlags |= OBJECT_UNDROPPABLE;

}



void ResetMortarsOnTeamCount( void )
{
	guiMortarsRolledByTeam = 0;
}
