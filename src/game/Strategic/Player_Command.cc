#include "MapScreen.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Player_Command.h"
#include "Queen_Command.h"
#include "Campaign_Types.h"
#include "Strategic_Town_Loyalty.h"
#include "Strategic_Mines.h"
#include "Text.h"
#include "LaptopSave.h"
#include "Quests.h"
#include "Morale.h"
#include "Overhead.h"
#include "StrategicMap.h"
#include "Map_Screen_Helicopter.h"
#include "Game_Clock.h"
#include "Meanwhile.h"
#include "EMail.h"
#include "GameSettings.h"
#include "Strategic_Status.h"
#include "Strategic_AI.h"
#include "PreBattle_Interface.h"
#include "Map_Screen_Interface.h"
#include "Tactical_Save.h"
#include "GameInstance.h"
#include "ContentManager.h"
#include "ShippingDestinationModel.h"


#include <string_theory/format>
#include <string_theory/string>


ST::string GetSectorFacilitiesFlags(const SGPSector& sector)
{
	// Build a string stating current facilities present in sector
	UINT32 const facilities = SectorInfo[sector.AsByte()].uiFacilitiesFlags;
	if (facilities == 0)
	{
		return sFacilitiesStrings[0];
	}

	ST::string buf;
	const char* fmt = "{}";
	for (size_t i = 0;; ++i)
	{
		UINT32 const bit = 1 << i;
		if (!(facilities & bit)) continue;
		buf += ST::format(fmt, sFacilitiesStrings[i + 1]);
		fmt = ",{}";
		if ((facilities & ~(bit - 1)) == bit) break;
	}
	return buf;
}


// ALL changes of control to player must be funneled through here!
BOOLEAN SetThisSectorAsPlayerControlled(const SGPSector& sMap, BOOLEAN fContested)
{
	// NOTE: MapSector must be 16-bit, cause MAX_WORLD_X is actually 18, so the sector numbers exceed 256 although we use only 16x16
	UINT16 usMapSector = 0;
	BOOLEAN fWasEnemyControlled = FALSE;
	INT8 bTownId = 0;

	if( AreInMeanwhile( ) )
	{
		return FALSE;
	}

	UINT8 const sector = sMap.AsByte();
	if (sMap.z == 0)
	{
		usMapSector = sMap.AsStrategicIndex();

/*
		// if enemies formerly controlled this sector
		if (StrategicMap[ usMapSector ].fEnemyControlled)
		{
			// remember that the enemies have lost it
			StrategicMap[ usMapSector ].fLostControlAtSomeTime = TRUE;
		}
*/
		if (NumHostilesInSector(sMap))
		{ //too premature:  enemies still in sector.
			return FALSE;
		}

		// check if we ever grabbed drassen airport, if so, set fact we can go to BR's
		auto shippingDest = GCM->getPrimaryShippingDestination();
		if(sector == shippingDest->getDeliverySector())
		{
			LaptopSaveInfo.fBobbyRSiteCanBeAccessed = TRUE;

			//If the player has been to Bobbyr when it was down, and we havent already sent email, send him an email
			if( LaptopSaveInfo.ubHaveBeenToBobbyRaysAtLeastOnceWhileUnderConstruction == BOBBYR_BEEN_TO_SITE_ONCE &&  LaptopSaveInfo.ubHaveBeenToBobbyRaysAtLeastOnceWhileUnderConstruction != BOBBYR_ALREADY_SENT_EMAIL )
			{
				AddEmail( BOBBYR_NOW_OPEN, BOBBYR_NOW_OPEN_LENGTH, BOBBY_R, GetWorldTotalMin());
				LaptopSaveInfo.ubHaveBeenToBobbyRaysAtLeastOnceWhileUnderConstruction = BOBBYR_ALREADY_SENT_EMAIL;
			}
		}

		fWasEnemyControlled = StrategicMap[ usMapSector ].fEnemyControlled;

		StrategicMap[ usMapSector ].fEnemyControlled = FALSE;

		bTownId = StrategicMap[ usMapSector ].bNameId;

		// check if there's a town in the sector
		if ((bTownId >= FIRST_TOWN) && (bTownId < NUM_TOWNS))
		{
			// yes, start tracking (& displaying) this town's loyalty if not already doing so
			StartTownLoyaltyIfFirstTime( bTownId );
		}


		// if player took control away from enemy
		if( fWasEnemyControlled && fContested )
		{
			// and it's a town
			if ((bTownId >= FIRST_TOWN) && (bTownId < NUM_TOWNS))
			{
				// don't do these for takeovers of Omerta sectors at the beginning of the game
				if ((bTownId != OMERTA) || (GetWorldDay() != 1))
				{
					if (sMap.z == 0 && sector != SEC_J9 && sector != SEC_K4)
					{
						HandleMoraleEvent(nullptr, MORALE_TOWN_LIBERATED, sMap);
						HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_GAIN_TOWN_SECTOR, sMap);

						// liberation by definition requires that the place was enemy controlled in the first place
						CheckIfEntireTownHasBeenLiberated(bTownId, sMap);
					}
				}
			}

			// if it's a mine that's still worth something
			INT8 const mine_id = GetMineIndexForSector(sector);
			if (mine_id != -1 && GetTotalLeftInMine(mine_id) > 0)
			{
				HandleMoraleEvent(NULL, MORALE_MINE_LIBERATED, sMap);
				HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_GAIN_MINE, sMap);
			}

			// if it's a SAM site sector
			INT8 const sam_id = GetSAMIdFromSector(sMap);
			if (sam_id != -1)
			{
				if ( 1 /*!GetSectorFlagStatus( sMapX, sMapY, bMapZ, SF_SECTOR_HAS_BEEN_LIBERATED_ONCE ) */)
				{
					// SAM site liberated for first time, schedule meanwhile
					HandleMeanWhileEventPostingForSAMLiberation(sam_id);
				}

				HandleMoraleEvent(nullptr, MORALE_SAM_SITE_LIBERATED, sMap);
				HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_GAIN_SAM, sMap);

				// if Skyrider has been delivered to chopper, and already mentioned Drassen SAM site, but not used this quote yet
				if ( IsHelicopterPilotAvailable() && ( guiHelicopterSkyriderTalkState >= 1 ) && ( !gfSkyriderSaidCongratsOnTakingSAM ) )
				{
					SkyRiderTalk( SAM_SITE_TAKEN );
					gfSkyriderSaidCongratsOnTakingSAM = TRUE;
				}

				if (!SectorInfo[sector].fSurfaceWasEverPlayerControlled)
				{
					// grant grace period
					if ( gGameOptions.ubDifficultyLevel >= DIF_LEVEL_HARD )
					{
						UpdateLastDayOfPlayerActivity( ( UINT16 ) ( GetWorldDay() + 2 ) );
					}
					else
					{
						UpdateLastDayOfPlayerActivity( ( UINT16 ) ( GetWorldDay() + 1 ) );
					}
				}
			}

			// if it's a helicopter refueling site sector
			if (IsRefuelSiteInSector(usMapSector))
			{
				UpdateRefuelSiteAvailability( );
			}

//			SetSectorFlag(sMap, SF_SECTOR_HAS_BEEN_LIBERATED_ONCE);
			if (sMap.z == 0 && ((sMap.y == MAP_ROW_M && (sMap.x >= 2 && sMap.x <= 6)) || (sMap.y == MAP_ROW_N && sMap.x == 6)))
			{
				HandleOutskirtsOfMedunaMeanwhileScene();
			}
		}

		if( fContested )
		{
			StrategicHandleQueenLosingControlOfSector( (UINT8) sMap.x, (UINT8) sMap.y, (UINT8) sMap.z);
		}
	}
	else
	{
		if (sector == SEC_P3 && sMap.z == 1)
		{ //Basement sector (P3_b1)
			gfUseAlternateQueenPosition = TRUE;
		}
	}

	if (sMap.z == 0)
	{
		SectorInfo[sector].fSurfaceWasEverPlayerControlled = TRUE;
	}

	//KM : Aug 11, 1999 -- Patch fix:  Relocated this check so it gets called everytime a sector changes hands,
	//     even if the sector isn't a SAM site.  There is a bug _somewhere_ that fails to update the airspace,
	//     even though the player controls it.
	UpdateAirspaceControl( );

	// redraw map/income if in mapscreen
	fMapPanelDirty = TRUE;
	fMapScreenBottomDirty = TRUE;

	return fWasEnemyControlled;
}


// ALL changes of control to enemy must be funneled through here!
BOOLEAN SetThisSectorAsEnemyControlled(const SGPSector& sec)
{
	BOOLEAN fWasPlayerControlled = FALSE;
	INT8 bTownId = 0;
	UINT8 ubTheftChance;

	//KM : August 6, 1999 Patch fix
	//     This check was added because this function gets called when player mercs retreat from an unresolved
	//     battle between militia and enemies.  It will get called again AFTER autoresolve is finished.
	if( gfAutomaticallyStartAutoResolve )
	{
		return( FALSE );
	}

	if (sec.z == 0)
	{
		UINT16 usMapSector = sec.AsStrategicIndex();

		fWasPlayerControlled = !StrategicMap[ usMapSector ].fEnemyControlled;

		StrategicMap[ usMapSector ].fEnemyControlled = TRUE;

		// if player lost control to the enemy
		if ( fWasPlayerControlled )
		{
			if (PlayerMercsInSector(sec))
			{ //too premature:  Player mercs still in sector.
				return FALSE;
			}

			UINT8 const sector = sec.AsByte();
			// check if there's a town in the sector
			bTownId = StrategicMap[ usMapSector ].bNameId;

			// and it's a town
			if ((bTownId >= FIRST_TOWN) && (bTownId < NUM_TOWNS))
			{
				if (sec.z == 0 && sector != SEC_J9 && sector != SEC_K4)
				{
					HandleMoraleEvent(nullptr, MORALE_TOWN_LOST, sec);
					HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_LOSE_TOWN_SECTOR, sec);

					CheckIfEntireTownHasBeenLost(bTownId, sec.x, sec.y);
				}
			}

			// if the sector has a mine which is still worth something
			INT8 const mine_id = GetMineIndexForSector(sector);
			if (mine_id != -1 && GetTotalLeftInMine(mine_id) > 0)
			{
				QueenHasRegainedMineSector(mine_id);
				HandleMoraleEvent(nullptr, MORALE_MINE_LOST, sec);
				HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_LOSE_MINE, sec);
			}

			// if it's a SAM site sector
			if (IsThisSectorASAMSector(sec))
			{
				HandleMoraleEvent(nullptr, MORALE_SAM_SITE_LOST, sec);
				HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_LOSE_SAM, sec);
			}

			// if it's a helicopter refueling site sector
			if (IsRefuelSiteInSector(usMapSector))
			{
				UpdateRefuelSiteAvailability( );
			}

			// ARM: this must be AFTER all resulting loyalty effects are resolved, or reduced mine income shown won't be accurate
			NotifyPlayerWhenEnemyTakesControlOfImportantSector(sec);
		}

		// NOTE: Stealing is intentionally OUTSIDE the fWasPlayerControlled branch.  This function gets called if new
		// enemy reinforcements arrive, and they deserve another crack at stealing what the first group missed! :-)

		// stealing should fail anyway 'cause there shouldn't be a temp file for unvisited sectors, but let's check anyway
		if (GetSectorFlagStatus(sec, SF_ALREADY_VISITED))
		{
			// enemies can steal items left lying about (random chance).  The more there are, the more they take!
			ubTheftChance = 5 * NumEnemiesInAnySector(sec);
			// max 90%, some stuff may just simply not get found
			if (ubTheftChance > 90 )
			{
				ubTheftChance = 90;
			}
			RemoveRandomItemsInSector(sec, ubTheftChance);
		}

		// don't touch fPlayer flag for a surface sector lost to the enemies!
		// just because player has lost the sector doesn't mean he realizes it - that's up to our caller to decide!
	}

	//KM : Aug 11, 1999 -- Patch fix:  Relocated this check so it gets called everytime a sector changes hands,
	//     even if the sector isn't a SAM site.  There is a bug _somewhere_ that fails to update the airspace,
	//     even though the player controls it.
	UpdateAirspaceControl( );

	// redraw map/income if in mapscreen
	fMapPanelDirty = TRUE;
	fMapScreenBottomDirty = TRUE;

	return fWasPlayerControlled;
}
