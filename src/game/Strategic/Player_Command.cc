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


ST::string GetSectorFacilitiesFlags(INT16 x, INT16 y)
{
	// Build a string stating current facilities present in sector
	UINT32 const facilities = SectorInfo[SECTOR(x, y)].uiFacilitiesFlags;
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
BOOLEAN SetThisSectorAsPlayerControlled( INT16 sMapX, INT16 sMapY, INT8 bMapZ, BOOLEAN fContested )
{
	// NOTE: MapSector must be 16-bit, cause MAX_WORLD_X is actually 18, so the sector numbers exceed 256 although we use only 16x16
	UINT16 usMapSector = 0;
	BOOLEAN fWasEnemyControlled = FALSE;
	INT8 bTownId = 0;

	if( AreInMeanwhile( ) )
	{
		return FALSE;
	}

	UINT8 const sector = SECTOR(sMapX, sMapY);
	if( bMapZ == 0 )
	{
		usMapSector = sMapX + ( sMapY * MAP_WORLD_X );

/*
		// if enemies formerly controlled this sector
		if (StrategicMap[ usMapSector ].fEnemyControlled)
		{
			// remember that the enemies have lost it
			StrategicMap[ usMapSector ].fLostControlAtSomeTime = TRUE;
		}
*/
		if (NumHostilesInSector(SGPSector(sMapX, sMapY, bMapZ)))
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
					if (bMapZ == 0 && sector != SEC_J9 && sector != SEC_K4)
					{
						HandleMoraleEvent( NULL, MORALE_TOWN_LIBERATED, sMapX, sMapY, bMapZ );
						HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_GAIN_TOWN_SECTOR, sMapX, sMapY, bMapZ );

						// liberation by definition requires that the place was enemy controlled in the first place
						CheckIfEntireTownHasBeenLiberated( bTownId, sMapX, sMapY );
					}
				}
			}

			// if it's a mine that's still worth something
			INT8 const mine_id = GetMineIndexForSector(sector);
			if (mine_id != -1 && GetTotalLeftInMine(mine_id) > 0)
			{
				HandleMoraleEvent(NULL, MORALE_MINE_LIBERATED, sMapX, sMapY, bMapZ);
				HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_GAIN_MINE, sMapX, sMapY, bMapZ);
			}

			// if it's a SAM site sector
			INT8 const sam_id = GetSAMIdFromSector(sMapX, sMapY, bMapZ);
			if (sam_id != -1)
			{
				if ( 1 /*!GetSectorFlagStatus( sMapX, sMapY, bMapZ, SF_SECTOR_HAS_BEEN_LIBERATED_ONCE ) */)
				{
					// SAM site liberated for first time, schedule meanwhile
					HandleMeanWhileEventPostingForSAMLiberation(sam_id);
				}

				HandleMoraleEvent( NULL, MORALE_SAM_SITE_LIBERATED, sMapX, sMapY, bMapZ );
				HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_GAIN_SAM, sMapX, sMapY, bMapZ );

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

//			SetSectorFlag( sMapX, sMapY, bMapZ, SF_SECTOR_HAS_BEEN_LIBERATED_ONCE );
			if ( bMapZ == 0 && ( ( sMapY == MAP_ROW_M && (sMapX >= 2 && sMapX <= 6) ) || (sMapY == MAP_ROW_N && sMapX == 6)) )
			{
				HandleOutskirtsOfMedunaMeanwhileScene();
			}
		}

		if( fContested )
		{
			StrategicHandleQueenLosingControlOfSector( (UINT8)sMapX, (UINT8)sMapY, (UINT8)bMapZ );
		}
	}
	else
	{
		if (sector == SEC_P3 && bMapZ == 1)
		{ //Basement sector (P3_b1)
			gfUseAlternateQueenPosition = TRUE;
		}
	}

	if ( bMapZ == 0 )
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
BOOLEAN SetThisSectorAsEnemyControlled(INT16 const sMapX, INT16 const sMapY, INT8 const bMapZ)
{
	UINT16 usMapSector = 0;
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

	if( bMapZ == 0 )
	{
		usMapSector = sMapX + ( sMapY * MAP_WORLD_X );

		fWasPlayerControlled = !StrategicMap[ usMapSector ].fEnemyControlled;

		StrategicMap[ usMapSector ].fEnemyControlled = TRUE;

		// if player lost control to the enemy
		if ( fWasPlayerControlled )
		{
			if( PlayerMercsInSector( (UINT8)sMapX, (UINT8)sMapY, (UINT8)bMapZ ) )
			{ //too premature:  Player mercs still in sector.
				return FALSE;
			}

			UINT8 const sector = SECTOR(sMapX, sMapY);
			// check if there's a town in the sector
			bTownId = StrategicMap[ usMapSector ].bNameId;

			// and it's a town
			if ((bTownId >= FIRST_TOWN) && (bTownId < NUM_TOWNS))
			{
				if (bMapZ == 0 && sector != SEC_J9 && sector != SEC_K4)
				{
					HandleMoraleEvent( NULL, MORALE_TOWN_LOST, sMapX, sMapY, bMapZ );
					HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_LOSE_TOWN_SECTOR, sMapX, sMapY, bMapZ );

					CheckIfEntireTownHasBeenLost( bTownId, sMapX, sMapY );
				}
			}

			// if the sector has a mine which is still worth something
			INT8 const mine_id = GetMineIndexForSector(sector);
			if (mine_id != -1 && GetTotalLeftInMine(mine_id) > 0)
			{
				QueenHasRegainedMineSector(mine_id);
				HandleMoraleEvent(NULL, MORALE_MINE_LOST, sMapX, sMapY, bMapZ);
				HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_LOSE_MINE, sMapX, sMapY, bMapZ);
			}

			// if it's a SAM site sector
			if( IsThisSectorASAMSector( sMapX, sMapY, bMapZ ) )
			{
				HandleMoraleEvent( NULL, MORALE_SAM_SITE_LOST, sMapX, sMapY, bMapZ );
				HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_LOSE_SAM, sMapX, sMapY, bMapZ );
			}

			// if it's a helicopter refueling site sector
			if (IsRefuelSiteInSector(usMapSector))
			{
				UpdateRefuelSiteAvailability( );
			}

			// ARM: this must be AFTER all resulting loyalty effects are resolved, or reduced mine income shown won't be accurate
			NotifyPlayerWhenEnemyTakesControlOfImportantSector(sMapX, sMapY, 0);
		}

		// NOTE: Stealing is intentionally OUTSIDE the fWasPlayerControlled branch.  This function gets called if new
		// enemy reinforcements arrive, and they deserve another crack at stealing what the first group missed! :-)

		// stealing should fail anyway 'cause there shouldn't be a temp file for unvisited sectors, but let's check anyway
		if (GetSectorFlagStatus(sMapX, sMapY, bMapZ, SF_ALREADY_VISITED))
		{
			// enemies can steal items left lying about (random chance).  The more there are, the more they take!
			ubTheftChance = 5 * NumEnemiesInAnySector( sMapX, sMapY, bMapZ );
			// max 90%, some stuff may just simply not get found
			if (ubTheftChance > 90 )
			{
				ubTheftChance = 90;
			}
			RemoveRandomItemsInSector( sMapX, sMapY, bMapZ, ubTheftChance );
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
