#include <stdexcept>

#include "Directories.h"
#include "LoadSaveBasicSoldierCreateStruct.h"
#include "LoadSaveSoldierCreate.h"
#include "Types.h"
#include "StrategicMap.h"
#include "Overhead.h"
#include "Soldier_Add.h"
#include "Soldier_Create.h"
#include "Soldier_Init_List.h"
#include "Debug.h"
#include "Random.h"
#include "Items.h"
#include "Map_Information.h"
#include "Soldier_Profile.h"
#include "Sys_Globals.h"
#include "EditorMercs.h"
#include "Animation_Data.h"
#include "Message.h"
#include "Font_Control.h"
#include "Sound_Control.h"
#include "Quests.h"
#include "Render_Fun.h"
#include "Meanwhile.h"
#include "Strategic_AI.h"
#include "Map_Screen_Interface_Map.h"
#include "Inventory_Choosing.h"
#include "Campaign_Types.h"
#include "AI.h"
#include "NPC.h"
#include "Scheduling.h"
#include "FileMan.h"
#include "Logger.h"
#include "MercProfile.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "externalized/strategic/BloodCatSpawnsModel.h"

BOOLEAN gfOriginalList = TRUE;

SOLDIERINITNODE *gSoldierInitHead = NULL;
SOLDIERINITNODE *gSoldierInitTail = NULL;

SOLDIERINITNODE *gOriginalSoldierInitListHead = NULL;
SOLDIERINITNODE *gAlternateSoldierInitListHead = NULL;

void InitSoldierInitList()
{
	if( gSoldierInitHead )
		KillSoldierInitList();
	gSoldierInitHead = NULL;
	gSoldierInitTail = NULL;
}

void KillSoldierInitList()
{
	while( gSoldierInitHead )
		RemoveSoldierNodeFromInitList( gSoldierInitTail );
	if( gfOriginalList )
		gOriginalSoldierInitListHead = NULL;
	else
		gAlternateSoldierInitListHead = NULL;
}


SOLDIERINITNODE* AddBasicPlacementToSoldierInitList(BASIC_SOLDIERCREATE_STRUCT const& bp)
{
	SOLDIERINITNODE* const si = new SOLDIERINITNODE{};

	si->pBasicPlacement  = new BASIC_SOLDIERCREATE_STRUCT{};
	*si->pBasicPlacement = bp;

	// It is impossible to set up detailed placement stuff now. If there is any
	// detailed placement information during map load, it will be added
	// immediately after this function call.
	si->pDetailedPlacement = 0;
	si->pSoldier           = 0;
	si->next               = 0;
	si->prev               = gSoldierInitTail;

	// Insert the new node in the list in its proper place
	if (!gSoldierInitTail)
	{
		gSoldierInitHead = si;
		if (gfOriginalList)
			gOriginalSoldierInitListHead = si;
		else
			gAlternateSoldierInitListHead = si;
	}
	else
	{
		// TEMP: no sorting, just enemies
		gSoldierInitTail->next = si;
	}
	gSoldierInitTail = si;

	if (gfOriginalList) ++gMapInformation.ubNumIndividuals;
	return si;
}


void RemoveSoldierNodeFromInitList( SOLDIERINITNODE *pNode )
{
	if( !pNode )
		return;
	if( gfOriginalList )
		gMapInformation.ubNumIndividuals--;
	if( pNode->pBasicPlacement )
	{
		delete pNode->pBasicPlacement;
		pNode->pBasicPlacement = NULL;
	}
	if( pNode->pDetailedPlacement )
	{
		delete pNode->pDetailedPlacement;
		pNode->pDetailedPlacement = NULL;
	}
	if( pNode->pSoldier )
	{
		if( pNode->pSoldier->ubID >= 20 )
		{
			TacticalRemoveSoldier(*pNode->pSoldier);
		}
	}
	if( pNode == gSoldierInitHead )
	{
		gSoldierInitHead = gSoldierInitHead->next;
		if( gSoldierInitHead )
			gSoldierInitHead->prev = NULL;
		if( gfOriginalList )
			gOriginalSoldierInitListHead = gSoldierInitHead;
		else
			gAlternateSoldierInitListHead = gSoldierInitHead;
	}
	else if( pNode == gSoldierInitTail )
	{
		gSoldierInitTail = gSoldierInitTail->prev;
		gSoldierInitTail->next = NULL;
	}
	else
	{
		pNode->prev->next = pNode->next;
		pNode->next->prev = pNode->prev;
	}
	delete pNode;
}


//These serialization functions are assuming the passing of a valid file
//pointer to the beginning of the save/load area, which is not necessarily at
//the beginning of the file.  This is just a part of the whole map serialization.
BOOLEAN SaveSoldiersToMap( HWFILE fp )
{
	UINT32 i;
	SOLDIERINITNODE *curr;

	if( !fp )
		return FALSE;

	if (gMapInformation.ubNumIndividuals > MAX_NUM_SOLDIERS)
		return FALSE;

	//If we are perhaps in the alternate version of the editor, we don't want bad things to
	//happen.  This is probably the only place I know where the user gets punished now.  If the
	//person was in the alternate editor mode, then decided to save the game, the current mercs may
	//not be there.  This would be bad.  What we do is override any merc editing done while in this
	//mode, and kill them all, while replacing them with the proper ones.  Not only that, the alternate
	//editing mode is turned off, and if intentions are to play the game, the user will be facing many
	//enemies!
	if (!gfOriginalList) ResetAllMercPositions();

	curr = gSoldierInitHead;
	for( i=0; i < gMapInformation.ubNumIndividuals; i++ )
	{
		if( !curr )
			return FALSE;
		curr->ubNodeID = (UINT8)i;
		InjectBasicSoldierCreateStructIntoFile(fp, *curr->pBasicPlacement);

		if( curr->pBasicPlacement->fDetailedPlacement )
		{
			if( !curr->pDetailedPlacement )
				return FALSE;
			InjectSoldierCreateIntoFile(fp, curr->pDetailedPlacement);
		}
		curr = curr->next;
	}
	return TRUE;
}


void LoadSoldiersFromMap(HWFILE const f, bool stracLinuxFormat)
{
	UINT8 const n_individuals = gMapInformation.ubNumIndividuals;

	UseEditorAlternateList();
	KillSoldierInitList();
	UseEditorOriginalList();
	KillSoldierInitList();
	InitSoldierInitList();

	if (n_individuals > MAX_NUM_SOLDIERS)
	{
		throw std::runtime_error("Corrupt map check failed. ubNumIndividuals is greater than MAX_NUM_SOLDIERS.");
	}

	// Because we are loading the map, we needed to know how many guys are being
	// loaded, but when we add them to the list here, it automatically increments
	// that number, effectively doubling it, which would be a problem. Now that we
	// know the number, we clear it here, so it gets built again.
	gMapInformation.ubNumIndividuals = 0; // Must be cleared here

	bool cow_in_sector = false;
	for (UINT32 i = 0; i != n_individuals; ++i)
	{
		BASIC_SOLDIERCREATE_STRUCT bp;
		ExtractBasicSoldierCreateStructFromFile(f, bp);
		SOLDIERINITNODE* const n = AddBasicPlacementToSoldierInitList(bp);
		n->ubNodeID = i;

		if (bp.fDetailedPlacement)
		{
			// Add the static detailed placement information in the same newly created
			// node as the basic placement.
			SOLDIERCREATE_STRUCT* const sc = new SOLDIERCREATE_STRUCT{};
			ExtractSoldierCreateFromFile(f, sc, stracLinuxFormat);

			if (sc->ubProfile != NO_PROFILE)
			{
				UINT8 const civ_group = GetProfile(sc->ubProfile).ubCivilianGroup;
				sc->ubCivilianGroup                 = civ_group;
				n->pBasicPlacement->ubCivilianGroup = civ_group;
			}

			n->pDetailedPlacement = sc;
		}

		if (bp.bBodyType == COW) cow_in_sector = true;
	}

	if (cow_in_sector)
	{
		ST::string str = ST::format(SOUNDSDIR "/cowmoo{}.wav", Random(3) + 1);
		PlayJA2SampleFromFile(str.c_str(), MIDVOLUME, 1, MIDDLEPAN);
	}
}


//Because soldiers, creatures, etc., maybe added to the game at anytime theoretically, the
//list will need to be sorted to reflect this.  It is quite likely that this won't be needed,
//but the flexibility is there just incase.  Now the list is sorted in the following manner:
//-1st priority:  Any nodes containing valid pointers to soldiers are moved to the end of the list.
//                We don't ever want to use two identical placements.
//-2nd priority:  Any nodes that have priority existance and detailed placement information are
//                put first in the list.
//-3rd priority:  Any nodes that have priority existance and no detailed placement information are used next.
//-4th priority:  Any nodes that have detailed placement and no priority existance information are used next.
//-5th priority:  The rest of the nodes are basic placements and are placed in the center of the list.  Of
//                these, they are randomly filled based on the number needed.
//NOTE:  This function is called by AddSoldierInitListTeamToWorld().  There is no other place it needs to
//       be called.
static void SortSoldierInitList(void)
{
	SOLDIERINITNODE *temp, *curr;

	if( !gSoldierInitHead )
		return;

	//1st priority sort
	curr = gSoldierInitTail;
	while( curr )
	{
		if( curr->pSoldier && curr != gSoldierInitTail )
		{
			//This node has an existing soldier, so move to end of list.
			//copy node
			temp = curr;
			if( temp == gSoldierInitHead )
			{
				//If we dealing with the head, we need to move it now.
				gSoldierInitHead = gSoldierInitHead->next;
				if( gfOriginalList )
					gOriginalSoldierInitListHead = gSoldierInitHead;
				else
					gAlternateSoldierInitListHead = gSoldierInitHead;
				gSoldierInitHead->prev = NULL;
				temp->next = NULL;
			}
			curr = curr->prev;
			//detach node from list
			if( temp->prev )
				temp->prev->next = temp->next;
			if( temp->next )
				temp->next->prev = temp->prev;
			//add node to end of list
			temp->prev = gSoldierInitTail;
			temp->next = NULL;
			gSoldierInitTail->next = temp;
			gSoldierInitTail = temp;
		}
		else
		{
			curr = curr->prev;
		}
	}
	//4th -- put to start
	curr = gSoldierInitHead;
	while( curr )
	{
		if( !curr->pSoldier && !curr->pBasicPlacement->fPriorityExistance && curr->pDetailedPlacement && curr != gSoldierInitHead )
		{
			//Priority existance nodes without detailed placement info are moved to beginning of list
			//copy node
			temp = curr;
			if( temp == gSoldierInitTail )
			{
				//If we dealing with the tail, we need to move it now.
				gSoldierInitTail = gSoldierInitTail->prev;
				gSoldierInitTail->next = NULL;
				temp->prev = NULL;
			}
			curr = curr->next;
			//detach node from list
			if( temp->prev )
				temp->prev->next = temp->next;
			if( temp->next )
				temp->next->prev = temp->prev;
			//add node to beginning of list
			temp->prev = NULL;
			temp->next = gSoldierInitHead;
			gSoldierInitHead->prev = temp;
			gSoldierInitHead = temp;
			if( gfOriginalList )
				gOriginalSoldierInitListHead = gSoldierInitHead;
			else
				gAlternateSoldierInitListHead = gSoldierInitHead;
		}
		else
		{
			curr = curr->next;
		}
	}
	//3rd priority sort (see below for reason why we do 2nd after 3rd)
	curr = gSoldierInitHead;
	while( curr )
	{
		if( !curr->pSoldier && curr->pBasicPlacement->fPriorityExistance && !curr->pDetailedPlacement && curr != gSoldierInitHead )
		{
			//Priority existance nodes without detailed placement info are moved
			//to beginning of list copy node
			temp = curr;
			if( temp == gSoldierInitTail )
			{
				//If we dealing with the tail, we need to move it now.
				gSoldierInitTail = gSoldierInitTail->prev;
				gSoldierInitTail->next = NULL;
				temp->prev = NULL;
			}
			curr = curr->next;
			//detach node from list
			if( temp->prev )
				temp->prev->next = temp->next;
			if( temp->next )
				temp->next->prev = temp->prev;
			//add node to beginning of list
			temp->prev = NULL;
			temp->next = gSoldierInitHead;
			gSoldierInitHead->prev = temp;
			gSoldierInitHead = temp;
			if( gfOriginalList )
				gOriginalSoldierInitListHead = gSoldierInitHead;
			else
				gAlternateSoldierInitListHead = gSoldierInitHead;
		}
		else
		{
			curr = curr->next;
		}
	}
	//2nd priority sort (by adding these to the front, it'll be before the
	//3rd priority sort.  This is why we do it after.
	curr = gSoldierInitHead;
	while( curr )
	{
		if( !curr->pSoldier && curr->pBasicPlacement->fPriorityExistance && curr->pDetailedPlacement && curr != gSoldierInitHead )
		{	//Priority existance nodes are moved to beginning of list
			//copy node
			temp = curr;
			if( temp == gSoldierInitTail )
			{
				//If we dealing with the tail, we need to move it now.
				gSoldierInitTail = gSoldierInitTail->prev;
				gSoldierInitTail->next = NULL;
				temp->prev = NULL;
			}
			curr = curr->next;
			//detach node from list
			if( temp->prev )
				temp->prev->next = temp->next;
			if( temp->next )
				temp->next->prev = temp->prev;
			//add node to beginning of list
			temp->prev = NULL;
			temp->next = gSoldierInitHead;
			gSoldierInitHead->prev = temp;
			gSoldierInitHead = temp;
			if( gfOriginalList )
				gOriginalSoldierInitListHead = gSoldierInitHead;
			else
				gAlternateSoldierInitListHead = gSoldierInitHead;
		}
		else
		{
			curr = curr->next;
		}
	}
	//4th priority sort
	//Done!  If the soldier existing slots are at the end of the list and the
	//       priority placements are at the beginning of the list, then the
	//       basic placements are in the middle.
}


bool AddPlacementToWorld(SOLDIERINITNODE* const init)
{
	SOLDIERCREATE_STRUCT dp{};

	// First check if this guy has a profile and if so check his location such that it matches
	if (SOLDIERCREATE_STRUCT* const init_dp = init->pDetailedPlacement)
	{
		if (!gfEditMode)
		{
			ProfileID const pid = init_dp->ubProfile;
			if (pid != NO_PROFILE)
			{
				MERCPROFILESTRUCT& p = GetProfile(pid);
				if (p.sSector != gWorldSector)  return false;
				if (p.ubMiscFlags & (PROFILE_MISC_FLAG_RECRUITED | PROFILE_MISC_FLAG_EPCACTIVE)) return false;
				if (p.bLife == 0)                 return false;
				if (p.fUseProfileInsertionInfo)   return false;
			}

			// Special case code when adding icecream truck.
			// CJC, August 18, 1999: don't do this code unless the ice cream truck is on our team
			if (init_dp->bBodyType == ICECREAMTRUCK &&
				FindSoldierByProfileIDOnPlayerTeam(ICECREAMTRUCK))
			{
				// Check to see if Hamous is here and not recruited. If so, add truck
				MERCPROFILESTRUCT& hamous = GetProfile(HAMOUS);
				// If not here, do not add
				if (hamous.sSector != gWorldSector) return true;
				// Check to make sure he isn't recruited.
				if (hamous.ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED) return true;
			}
		}
		CreateDetailedPlacementGivenStaticDetailedPlacementAndBasicPlacementInfo(&dp, init_dp, init->pBasicPlacement);
	}
	else
	{
		CreateDetailedPlacementGivenBasicPlacementInfo(&dp, init->pBasicPlacement);
	}

	if (!gfEditMode)
	{
		if (dp.bTeam == CIV_TEAM)
		{
			// Quest-related overrides
			static const SGPSector kingpin(5, MAP_ROW_C);
			static const SGPSector queen(3, MAP_ROW_P);
			static const SGPSector tixa(TIXA_SECTOR_X, TIXA_SECTOR_Y);
			static const SGPSector kids(13, MAP_ROW_C);
			if (gWorldSector == kingpin)
			{
				// Kinpin guys might be guarding Tony
				if (dp.ubCivilianGroup == KINGPIN_CIV_GROUP && (
					gTacticalStatus.fCivGroupHostile[KINGPIN_CIV_GROUP] == CIV_GROUP_WILL_BECOME_HOSTILE || (
					gubQuest[QUEST_KINGPIN_MONEY] == QUESTINPROGRESS &&
					CheckFact(FACT_KINGPIN_CAN_SEND_ASSASSINS, KINGPIN))))
				{
					// Pick a gridno in a square around the door to Hans' shop
					auto const PickGridNo = [](INT16 const apothem) -> GridNo
					{
						std::uniform_int_distribution<INT16> uid(-apothem, apothem);
						return 13531 + uid(gRandomEngine) + uid(gRandomEngine) * WORLD_COLS;
					};

					if (dp.ubProfile == NO_PROFILE)
					{
						// These guys should be guarding Tony
						dp.sInsertionGridNo = PickGridNo(7);

						switch (PreRandom(3))
						{
							case 0: dp.bOrders = ONGUARD;     break;
							case 1: dp.bOrders = CLOSEPATROL; break;
							case 2: dp.bOrders = ONCALL;      break;
						}
					}
					else if (dp.ubProfile == BILLY)
					{
						// Billy should now be able to roam around
						dp.sInsertionGridNo = PickGridNo(29);
						dp.bOrders = SEEKENEMY;
					}
					else if (dp.ubProfile == MADAME)
					{
						// She shouldn't be here
						return true;
					}
				}
			}
			else if (gWorldSector == queen && !gfInMeanwhile)
			{
				// Special civilian setup for queen's palace
				if (gubFact[FACT_QUEEN_DEAD])
				{
					// The queen's civs aren't added if queen is dead
					if (dp.ubCivilianGroup == QUEENS_CIV_GROUP) return true;
				}
				else
				{
					if (gfUseAlternateQueenPosition && dp.ubProfile == QUEEN)
					{
						dp.sInsertionGridNo = 11448;
					}
					if (dp.ubCivilianGroup != QUEENS_CIV_GROUP)
					{
						// The free civilians aren't added if queen is alive
						return true;
					}
				}
			}
			else if (gWorldSector == tixa)
			{
				// Tixa prison, once liberated, should not have any civs without
				// profiles unless they are kids
				if (!StrategicMap[tixa.AsStrategicIndex()].fEnemyControlled &&
					dp.ubProfile == NO_PROFILE &&
					dp.bBodyType != HATKIDCIV &&
					dp.bBodyType != KIDCIV)
				{
					// not there
					return true;
				}
			}
			else if (gWorldSector == kids)
			{
				if (CheckFact(FACT_KIDS_ARE_FREE, 0) &&
					(dp.bBodyType == HATKIDCIV || dp.bBodyType == KIDCIV))
				{
					// Not there any more, kids have been freed.
					return true;
				}
			}
		}
		else if (dp.bTeam == ENEMY_TEAM && dp.ubSoldierClass == SOLDIER_CLASS_ELITE)
		{
			// Special! Certain events in the game can cause profiled NPCs to become
			// enemies. The two cases are adding Mike and Iggy. We will only add one
			// NPC in any given combat and the conditions for setting the associated
			// facts are done elsewhere. There is also another place where NPCs can
			// get added, which is in TacticalCreateElite() used for inserting
			// offensive enemies.
			OkayToUpgradeEliteToSpecialProfiledEnemy(&dp);
		}
	}

	if (SOLDIERTYPE* const s = TacticalCreateSoldier(dp))
	{
		init->pSoldier    = s;
		init->ubSoldierID = s->ubID;
		AddSoldierToSectorNoCalculateDirection(s);
		return true;
	}
	else
	{
		SLOGD("Failed to create soldier using TacticalCreateSoldier within AddPlacementToWorld");
		return false;
	}
}


void AddSoldierInitListTeamToWorld(INT8 const team)
{
	// Sort the list in the following manner:
	// - Priority placements first
	// - Basic placements next
	// - Any placements with existing soldiers last (overrides others)
	SortSoldierInitList();

	if (giCurrentTilesetID == CAVES_1) // Cave/mine tileset only
	{
		// Convert all civilians to miners which use uniforms and more masculine
		// body types.
		CFOR_EACH_SOLDIERINITNODE(i)
		{
			BASIC_SOLDIERCREATE_STRUCT& bp = *i->pBasicPlacement;
			if (bp.bTeam != CIV_TEAM || i->pDetailedPlacement) continue;
			bp.ubSoldierClass = SOLDIER_CLASS_MINER;
			bp.bBodyType      = BODY_RANDOM;
		}
	}

	// While we have a list, with no active soldiers, process the list to add new
	// soldiers.
	for (SOLDIERINITNODE* i = gSoldierInitHead; i && !i->pSoldier; i = i->next)
	{
		if (i->pBasicPlacement->bTeam != team) continue;

		// mgl: Missing civilians Fix
		// AddPlacementToWorld() returns false for people (civilians) who have a profile
		// but are not currently in the sector of the loaded map. It doesn't mean that
		// there isn't any remaining slot for them in this case.
		if (!AddPlacementToWorld(i) && i->pBasicPlacement->bTeam != CIV_TEAM)
		{
			// If it fails to create the soldier, it is likely that it is because the
			// slots in the tactical engine are already full. Besides, the strategic
			// AI shouldn't be trying to fill a map with more than the maximum
			// allowable soldiers of team. All teams can have a max of 32 individuals,
			// except for the player which is 20. Players aren't processed in this
			// list anyway.
			break;
		}
	}
}


void AddSoldierInitListEnemyDefenceSoldiers( UINT8 ubTotalAdmin, UINT8 ubTotalTroops, UINT8 ubTotalElite )
{
	SOLDIERINITNODE *mark;
	INT32 iRandom;
	UINT8 ubMaxNum;
	UINT8 ubElitePDSlots = 0, ubEliteDSlots = 0, ubElitePSlots = 0, ubEliteBSlots = 0;
	UINT8 ubTroopPDSlots = 0, ubTroopDSlots = 0, ubTroopPSlots = 0, ubTroopBSlots = 0;
	UINT8 ubAdminPDSlots = 0, ubAdminDSlots = 0, ubAdminPSlots = 0, ubAdminBSlots = 0;
	UINT8 ubFreeSlots;
	UINT8 *pCurrSlots=NULL;
	UINT8 *pCurrTotal=NULL;
	UINT8 ubCurrClass;

	ResetMortarsOnTeamCount();

	//Specs call for only one profiled enemy can be in a sector at a time due to flavor reasons.
	gfProfiledEnemyAdded = FALSE;

	//Because the enemy defence forces work differently than the regular map placements, the numbers
	//of each type of enemy may not be the same.  Elites will choose the best placements, then army, then
	//administrators.

	ubMaxNum = ubTotalAdmin + ubTotalTroops + ubTotalElite;

	//Sort the list in the following manner:
	//-Priority placements first
	//-Basic placements next
	//-Any placements with existing soldiers last (overrides others)
	SortSoldierInitList();

	//Now count the number of nodes that are basic placements of desired team AND CLASS
	//This information will be used to randomly determine which of these placements
	//will be added based on the number of slots we can still add.
	CFOR_EACH_SOLDIERINITNODE(curr)
	{
		if (curr->pSoldier) break;
		if( curr->pBasicPlacement->bTeam == ENEMY_TEAM )
		{
			switch( curr->pBasicPlacement->ubSoldierClass )
			{
				case SOLDIER_CLASS_ELITE:
					if( curr->pBasicPlacement->fPriorityExistance && curr->pDetailedPlacement )
						ubElitePDSlots++;
					else if( curr->pBasicPlacement->fPriorityExistance )
						ubElitePSlots++;
					else if( curr->pDetailedPlacement )
						ubEliteDSlots++;
					else
						ubEliteBSlots++;
					break;
				case SOLDIER_CLASS_ADMINISTRATOR:
					if( curr->pBasicPlacement->fPriorityExistance && curr->pDetailedPlacement )
						ubAdminPDSlots++;
					else if( curr->pBasicPlacement->fPriorityExistance )
						ubAdminPSlots++;
					else if( curr->pDetailedPlacement )
						ubAdminDSlots++;
					else
						ubAdminBSlots++;
					break;
				case SOLDIER_CLASS_ARMY:
					if( curr->pBasicPlacement->fPriorityExistance && curr->pDetailedPlacement )
						ubTroopPDSlots++;
					else if( curr->pBasicPlacement->fPriorityExistance )
						ubTroopPSlots++;
					else if( curr->pDetailedPlacement )
						ubTroopDSlots++;
					else
						ubTroopBSlots++;
					break;
			}
		}
	}

	//ADD PLACEMENTS WITH PRIORITY EXISTANCE WITH DETAILED PLACEMENT INFORMATION FIRST
	//we now have the numbers of available slots for each soldier class, so loop through three times
	//and randomly choose some (or all) of the matching slots to fill.  This is done randomly.
	for( ubCurrClass = SOLDIER_CLASS_ADMINISTRATOR; ubCurrClass <= SOLDIER_CLASS_ARMY; ubCurrClass++ )
	{
		//First, prepare the counters.
		switch( ubCurrClass )
		{
			case SOLDIER_CLASS_ADMINISTRATOR:
				pCurrSlots = &ubAdminPDSlots;
				pCurrTotal = &ubTotalAdmin;
				break;
			case SOLDIER_CLASS_ELITE:
				pCurrSlots = &ubElitePDSlots;
				pCurrTotal = &ubTotalElite;
				break;
			case SOLDIER_CLASS_ARMY:
				pCurrSlots = &ubTroopPDSlots;
				pCurrTotal = &ubTotalTroops;
				break;
		}
		//Now, loop through the priority existance and detailed placement section of the list.
		FOR_EACH_SOLDIERINITNODE(curr)
		{
			if (ubMaxNum == 0 ||
				*pCurrTotal == 0 ||
				*pCurrSlots == 0 ||
				curr->pDetailedPlacement == NULL ||
				!curr->pBasicPlacement->fPriorityExistance)
			{
				break;
			}
			if( !curr->pSoldier && curr->pBasicPlacement->bTeam == ENEMY_TEAM )
			{
				if( curr->pBasicPlacement->ubSoldierClass == ubCurrClass )
				{
					if( *pCurrSlots <= *pCurrTotal || Random( *pCurrSlots ) < *pCurrTotal )
					{
						//found matching team, so add this soldier to the game.
						if( AddPlacementToWorld( curr ) )
						{
							(*pCurrTotal)--;
							ubMaxNum--;
						}
						else
							return;
					}
					(*pCurrSlots)--;
					//With the decrementing of the slot vars in this manner, the chances increase so that all slots
					//will be full by the time the end of the list comes up.
				}
			}
			curr = curr->next;
		}
	}
	if( !ubMaxNum )
		return;
	SOLDIERINITNODE* curr = gSoldierInitHead;
	while( curr && curr->pDetailedPlacement && curr->pBasicPlacement->fPriorityExistance )
		curr = curr->next;
	mark = curr;

	//ADD PLACEMENTS WITH PRIORITY EXISTANCE AND NO DETAILED PLACEMENT INFORMATION SECOND
	//we now have the numbers of available slots for each soldier class, so loop through three times
	//and randomly choose some (or all) of the matching slots to fill.  This is done randomly.
	for( ubCurrClass = SOLDIER_CLASS_ADMINISTRATOR; ubCurrClass <= SOLDIER_CLASS_ARMY; ubCurrClass++ )
	{
		//First, prepare the counters.
		switch( ubCurrClass )
		{
			case SOLDIER_CLASS_ADMINISTRATOR:
				pCurrSlots = &ubAdminPSlots;
				pCurrTotal = &ubTotalAdmin;
				break;
			case SOLDIER_CLASS_ELITE:
				pCurrSlots = &ubElitePSlots;
				pCurrTotal = &ubTotalElite;
				break;
			case SOLDIER_CLASS_ARMY:
				pCurrSlots = &ubTroopPSlots;
				pCurrTotal = &ubTotalTroops;
				break;
		}
		//Now, loop through the priority existance and non detailed placement section of the list.
		curr = mark;
		while( curr && ubMaxNum && *pCurrTotal && *pCurrSlots &&
			!curr->pDetailedPlacement && curr->pBasicPlacement->fPriorityExistance )
		{
			if( !curr->pSoldier && curr->pBasicPlacement->bTeam == ENEMY_TEAM )
			{
				if( curr->pBasicPlacement->ubSoldierClass == ubCurrClass )
				{
					if( *pCurrSlots <= *pCurrTotal || Random( *pCurrSlots ) < *pCurrTotal )
					{
						//found matching team, so add this soldier to the game.
						if( AddPlacementToWorld( curr ) )
						{
							(*pCurrTotal)--;
							ubMaxNum--;
						}
						else
							return;
					}
					(*pCurrSlots)--;
					//With the decrementing of the slot vars in this manner, the chances increase so that all slots
					//will be full by the time the end of the list comes up.
				}
			}
			curr = curr->next;
		}
	}
	if( !ubMaxNum )
		return;
	curr = mark;
	while( curr && !curr->pDetailedPlacement && curr->pBasicPlacement->fPriorityExistance )
		curr = curr->next;
	mark = curr;

	//ADD PLACEMENTS WITH NO DETAILED PLACEMENT AND PRIORITY EXISTANCE INFORMATION SECOND
	//we now have the numbers of available slots for each soldier class, so loop through three times
	//and randomly choose some (or all) of the matching slots to fill.  This is done randomly.
	for( ubCurrClass = SOLDIER_CLASS_ADMINISTRATOR; ubCurrClass <= SOLDIER_CLASS_ARMY; ubCurrClass++ )
	{
		//First, prepare the counters.
		switch( ubCurrClass )
		{
			case SOLDIER_CLASS_ADMINISTRATOR:
				pCurrSlots = &ubAdminDSlots;
				pCurrTotal = &ubTotalAdmin;
				break;
			case SOLDIER_CLASS_ELITE:
				pCurrSlots = &ubEliteDSlots;
				pCurrTotal = &ubTotalElite;
				break;
			case SOLDIER_CLASS_ARMY:
				pCurrSlots = &ubTroopDSlots;
				pCurrTotal = &ubTotalTroops;
				break;
		}
		//Now, loop through the priority existance and detailed placement section of the list.
		curr = mark;
		while( curr && ubMaxNum && *pCurrTotal && *pCurrSlots &&
			curr->pDetailedPlacement && !curr->pBasicPlacement->fPriorityExistance )
		{
			if( !curr->pSoldier && curr->pBasicPlacement->bTeam == ENEMY_TEAM )
			{
				if( curr->pBasicPlacement->ubSoldierClass == ubCurrClass )
				{
					if( *pCurrSlots <= *pCurrTotal || Random( *pCurrSlots ) < *pCurrTotal )
					{
						//found matching team, so add this soldier to the game.
						if( AddPlacementToWorld( curr ) )
						{
							(*pCurrTotal)--;
							ubMaxNum--;
						}
						else
							return;
					}
					(*pCurrSlots)--;
					//With the decrementing of the slot vars in this manner, the chances increase so that all slots
					//will be full by the time the end of the list comes up.
				}
			}
			curr = curr->next;
		}
	}
	if( !ubMaxNum )
		return;
	curr = mark;
	while( curr && curr->pDetailedPlacement && !curr->pBasicPlacement->fPriorityExistance )
		curr = curr->next;
	mark = curr;

	//Kris: January 11, 2000 -- NEW!!!
	//PRIORITY EXISTANT SLOTS MUST BE FILLED
	//This must be done to ensure all priority existant slots are filled before ANY other slots are filled,
	//even if that means changing the class of the slot.  Also, assume that there are no matching fits left
	//for priority existance slots.  All of the matches have been already assigned in the above passes.
	//We'll have to convert the soldier type of the slot to match.
	curr = gSoldierInitHead;
	while( curr && ubMaxNum && curr->pBasicPlacement->fPriorityExistance )
	{
		if( !curr->pSoldier && curr->pBasicPlacement->bTeam == ENEMY_TEAM )
		{
			//Choose which team to use.
			iRandom = Random( ubMaxNum );
			if( iRandom < ubTotalElite )
			{
				curr->pBasicPlacement->ubSoldierClass = SOLDIER_CLASS_ELITE;
				ubTotalElite--;
			}
			else if( iRandom < ubTotalElite + ubTotalTroops )
			{
				curr->pBasicPlacement->ubSoldierClass = SOLDIER_CLASS_ARMY;
				ubTotalTroops--;
			}
			else if( iRandom < ubTotalElite + ubTotalTroops + ubTotalAdmin )
			{
				curr->pBasicPlacement->ubSoldierClass = SOLDIER_CLASS_ADMINISTRATOR;
				ubTotalAdmin--;
			}
			else
				SLOGA("AddSoldierInitListEnemyDefenceSoldiers: something wrong with random");
			if( AddPlacementToWorld( curr ) )
			{
				ubMaxNum--;
			}
			else
				return;
		}
		curr = curr->next;
	}
	if( !ubMaxNum )
		return;

	//ADD REMAINING PLACEMENTS WITH BASIC PLACEMENT INFORMATION
	//we now have the numbers of available slots for each soldier class, so loop through three times
	//and randomly choose some (or all) of the matching slots to fill.  This is done randomly.
	for( ubCurrClass = SOLDIER_CLASS_ADMINISTRATOR; ubCurrClass <= SOLDIER_CLASS_ARMY; ubCurrClass++ )
	{
		//First, prepare the counters.
		switch( ubCurrClass )
		{
			case SOLDIER_CLASS_ADMINISTRATOR:
				pCurrSlots = &ubAdminBSlots;
				pCurrTotal = &ubTotalAdmin;
				break;
			case SOLDIER_CLASS_ELITE:
				pCurrSlots = &ubEliteBSlots;
				pCurrTotal = &ubTotalElite;
				break;
			case SOLDIER_CLASS_ARMY:
				pCurrSlots = &ubTroopBSlots;
				pCurrTotal = &ubTotalTroops;
				break;
		}
		//Now, loop through the regular basic placements section of the list.
		curr = mark;
		while( curr && ubMaxNum && *pCurrTotal && *pCurrSlots )
		{
			if( !curr->pSoldier && curr->pBasicPlacement->bTeam == ENEMY_TEAM )
			{
				if( curr->pBasicPlacement->ubSoldierClass == ubCurrClass )
				{
					if( *pCurrSlots <= *pCurrTotal || Random( *pCurrSlots ) < *pCurrTotal )
					{
						//found matching team, so add this soldier to the game.
						if( AddPlacementToWorld( curr ) )
						{
							(*pCurrTotal)--;
							ubMaxNum--;
						}
						else
							return;
					}
					(*pCurrSlots)--;
					//With the decrementing of the slot vars in this manner, the chances increase so that all slots
					//will be full by the time the end of the list comes up.
				}
			}
			curr = curr->next;
		}
	}
	if( !ubMaxNum )
		return;

	//If we are at this point, that means that there are some compatibility issues.  This is fine.  An example
	//would be a map containing 1 elite placement, and 31 troop placements.  If we had 3 elites move into this
	//sector, we would not have placements for two of them.  What we have to do is override the class information
	//contained in the list by choosing unused placements, and assign them to the elites.  This time, we will
	//use all free slots including priority placement slots (ignoring the priority placement information).

	//First, count up the total number of free slots.
	ubFreeSlots = 0;
	CFOR_EACH_SOLDIERINITNODE(curr)
	{
		if( !curr->pSoldier && curr->pBasicPlacement->bTeam == ENEMY_TEAM )
			ubFreeSlots++;
	}

	//Now, loop through the entire list again, but for the last time.  All enemies will be inserted now ignoring
	//detailed placements and classes.
	FOR_EACH_SOLDIERINITNODE(curr)
	{
		if (ubFreeSlots == 0 || ubMaxNum == 0) break;
		if( !curr->pSoldier && curr->pBasicPlacement->bTeam == ENEMY_TEAM )
		{
			//Randomly determine if we will use this slot; the more available slots in proportion to
			//the number of enemies, the lower the chance of accepting the slot.
			if( ubFreeSlots <= ubMaxNum || Random( ubFreeSlots ) < ubMaxNum )
			{
				//Choose which team to use.
				iRandom = Random( ubMaxNum );
				if( iRandom < ubTotalElite )
				{
					curr->pBasicPlacement->ubSoldierClass = SOLDIER_CLASS_ELITE;
					ubTotalElite--;
				}
				else if( iRandom < ubTotalElite + ubTotalTroops )
				{
					curr->pBasicPlacement->ubSoldierClass = SOLDIER_CLASS_ARMY;
					ubTotalTroops--;
				}
				else if( iRandom < ubTotalElite + ubTotalTroops + ubTotalAdmin )
				{
					curr->pBasicPlacement->ubSoldierClass = SOLDIER_CLASS_ADMINISTRATOR;
					ubTotalAdmin--;
				}
				else
					SLOGA("AddSoldierInitListEnemyDefenceSoldiers: something wrong with random");
				/* DISABLE THE OVERRIDE FOR NOW...
				if( curr->pDetailedPlacement )
				{ //delete the detailed placement information.
					delete curr->pDetailedPlacement;
					curr->pDetailedPlacement = NULL;
					curr->pBasicPlacement->fDetailedPlacement = FALSE;
				}
				*/
				if( AddPlacementToWorld( curr ) )
				{
					ubMaxNum--;
				}
				else
					return;
			}
			ubFreeSlots--;
			//With the decrementing of the slot vars in this manner, the chances increase so that all slots
			//will be full by the time the end of the list comes up.
		}
	}
}

//If we are adding militia to our map, then we do a few things differently.
//First of all, they exist exclusively to the enemy troops, so if the militia exists in the
//sector, then they get to use the enemy placements.  However, we remove any orders from
//placements containing RNDPTPATROL or POINTPATROL orders, as well as remove any detailed
//placement information.
void AddSoldierInitListMilitia( UINT8 ubNumGreen, UINT8 ubNumRegs, UINT8 ubNumElites )
{
	SOLDIERINITNODE *mark;
	SOLDIERINITNODE *curr;
	INT32 iRandom;
	UINT8 ubMaxNum;
	BOOLEAN fDoPlacement;
	UINT8 ubEliteSlots = 0;
	UINT8 ubRegSlots = 0;
	UINT8 ubGreenSlots = 0;
	UINT8 ubFreeSlots;
	UINT8 *pCurrSlots=NULL;
	UINT8 *pCurrTotal=NULL;
	UINT8 ubCurrClass;

	ubMaxNum = ubNumGreen + ubNumRegs + ubNumElites;

	//Sort the list in the following manner:
	//-Priority placements first
	//-Basic placements next
	//-Any placements with existing soldiers last (overrides others)
	SortSoldierInitList();

	curr = gSoldierInitHead;

	//First fill up only the priority existance slots (as long as the availability and class are okay)
	while( curr && curr->pBasicPlacement->fPriorityExistance && ubMaxNum )
	{
		fDoPlacement = TRUE;

		if( curr->pBasicPlacement->bTeam == ENEMY_TEAM || curr->pBasicPlacement->bTeam == MILITIA_TEAM )
		{
			//Matching team (kindof), now check the soldier class...
			if( ubNumElites && curr->pBasicPlacement->ubSoldierClass == SOLDIER_CLASS_ELITE )
			{
				curr->pBasicPlacement->ubSoldierClass = SOLDIER_CLASS_ELITE_MILITIA;
				ubNumElites--;
			}
			else if( ubNumRegs && curr->pBasicPlacement->ubSoldierClass == SOLDIER_CLASS_ARMY )
			{
				curr->pBasicPlacement->ubSoldierClass = SOLDIER_CLASS_REG_MILITIA;
				ubNumRegs--;
			}
			else if( ubNumGreen && curr->pBasicPlacement->ubSoldierClass == SOLDIER_CLASS_ADMINISTRATOR )
			{
				curr->pBasicPlacement->ubSoldierClass = SOLDIER_CLASS_GREEN_MILITIA;
				ubNumGreen--;
			}
			else
				fDoPlacement = FALSE;

			if ( fDoPlacement )
			{
				curr->pBasicPlacement->bTeam = MILITIA_TEAM;
				curr->pBasicPlacement->bOrders = STATIONARY;
				curr->pBasicPlacement->bAttitude = (INT8) Random( MAXATTITUDES );
				if( curr->pDetailedPlacement )
				{
					//delete the detailed placement information.
					delete curr->pDetailedPlacement;
					curr->pDetailedPlacement = NULL;
					curr->pBasicPlacement->fDetailedPlacement = FALSE;
					RandomizeRelativeLevel( &( curr->pBasicPlacement->bRelativeAttributeLevel ), curr->pBasicPlacement->ubSoldierClass );
					RandomizeRelativeLevel( &( curr->pBasicPlacement->bRelativeEquipmentLevel ), curr->pBasicPlacement->ubSoldierClass );
				}
				if( AddPlacementToWorld( curr ) )
				{
					ubMaxNum--;
				}
				else
					return;
			}
		}
		curr = curr->next;
	}
	if( !ubMaxNum )
		return;
	//Now count the number of nodes that are basic placements of desired team AND CLASS
	//This information will be used to randomly determine which of these placements
	//will be added based on the number of slots we can still add.
	mark = curr;
	while( curr && !curr->pSoldier )
	{
		if( curr->pBasicPlacement->bTeam == ENEMY_TEAM || curr->pBasicPlacement->bTeam == MILITIA_TEAM )
		{
			switch( curr->pBasicPlacement->ubSoldierClass )
			{
				case SOLDIER_CLASS_ELITE:         ubEliteSlots++; break;
				case SOLDIER_CLASS_ADMINISTRATOR: ubGreenSlots++; break;
				case SOLDIER_CLASS_ARMY:          ubRegSlots++;   break;
			}
		}
		curr = curr->next;
	}

	//we now have the numbers of available slots for each soldier class, so loop through three times
	//and randomly choose some (or all) of the matching slots to fill.  This is done randomly.
	for( ubCurrClass = SOLDIER_CLASS_ADMINISTRATOR; ubCurrClass <= SOLDIER_CLASS_ARMY; ubCurrClass++ )
	{
		//First, prepare the counters.
		switch( ubCurrClass )
		{
			case SOLDIER_CLASS_ADMINISTRATOR:
				pCurrSlots = &ubGreenSlots;
				pCurrTotal = &ubNumGreen;
				break;
			case SOLDIER_CLASS_ELITE:
				pCurrSlots = &ubEliteSlots;
				pCurrTotal = &ubNumElites;
				break;
			case SOLDIER_CLASS_ARMY:
				pCurrSlots = &ubRegSlots;
				pCurrTotal = &ubNumRegs;
				break;
		}
		//Now, loop through the basic placement of the list.
		curr = mark; //mark is the marker where the basic placements start.
		while( curr && !curr->pSoldier && ubMaxNum && *pCurrTotal && *pCurrSlots )
		{
			if( curr->pBasicPlacement->bTeam == ENEMY_TEAM || curr->pBasicPlacement->bTeam == MILITIA_TEAM )
			{
				if( curr->pBasicPlacement->ubSoldierClass == ubCurrClass )
				{
					if( *pCurrSlots <= *pCurrTotal || Random( *pCurrSlots ) < *pCurrTotal )
					{
						curr->pBasicPlacement->bTeam = MILITIA_TEAM;
						curr->pBasicPlacement->bOrders = STATIONARY;
						switch( ubCurrClass )
						{
							case SOLDIER_CLASS_ADMINISTRATOR:
								curr->pBasicPlacement->ubSoldierClass = SOLDIER_CLASS_GREEN_MILITIA;
								break;
							case SOLDIER_CLASS_ARMY:
								curr->pBasicPlacement->ubSoldierClass = SOLDIER_CLASS_REG_MILITIA;
								break;
							case SOLDIER_CLASS_ELITE:
								curr->pBasicPlacement->ubSoldierClass = SOLDIER_CLASS_ELITE_MILITIA;
								break;
						}
						//found matching team, so add this soldier to the game.
						if( AddPlacementToWorld( curr ) )
						{
							(*pCurrTotal)--;
							ubMaxNum--;
						}
						else
							return;
					}
					(*pCurrSlots)--;
					//With the decrementing of the slot vars in this manner, the chances increase so that all slots
					//will be full by the time the end of the list comes up.
				}
			}
			curr = curr->next;
		}
	}
	if( !ubMaxNum )
		return;
	//If we are at this point, that means that there are some compatibility issues.  This is fine.  An example
	//would be a map containing 1 elite placement, and 31 troop placements.  If we had 3 elites move into this
	//sector, we would not have placements for two of them.  What we have to do is override the class information
	//contained in the list by choosing unused placements, and assign them to the elites.  This time, we will
	//use all free slots including priority placement slots (ignoring the priority placement information).

	//First, count up the total number of free slots.
	ubFreeSlots = 0;
	CFOR_EACH_SOLDIERINITNODE(curr)
	{
		if(!curr->pSoldier &&
			(curr->pBasicPlacement->bTeam == ENEMY_TEAM ||
			curr->pBasicPlacement->bTeam == MILITIA_TEAM))
		{
			ubFreeSlots++;
		}
	}

	//Now, loop through the entire list again, but for the last time.  All enemies will be inserted now ignoring
	//detailed placements and classes.
	FOR_EACH_SOLDIERINITNODE(curr)
	{
		if (ubFreeSlots == 0 || ubMaxNum == 0) break;
		if(!curr->pSoldier &&
			(curr->pBasicPlacement->bTeam == ENEMY_TEAM ||
			curr->pBasicPlacement->bTeam == MILITIA_TEAM))
		{
			//Randomly determine if we will use this slot; the more available slots in proportion to
			//the number of enemies, the lower the chance of accepting the slot.
			if( ubFreeSlots <= ubMaxNum || Random( ubFreeSlots ) < ubMaxNum )
			{
				//Choose which team to use.
				iRandom = Random( ubMaxNum );
				if( iRandom < ubNumElites )
				{
					curr->pBasicPlacement->ubSoldierClass = SOLDIER_CLASS_ELITE_MILITIA;
					ubNumElites--;
				}
				else if( iRandom < ubNumElites + ubNumRegs )
				{
					curr->pBasicPlacement->ubSoldierClass = SOLDIER_CLASS_REG_MILITIA;
					ubNumRegs--;
				}
				else if( iRandom < ubNumElites + ubNumRegs + ubNumGreen )
				{
					curr->pBasicPlacement->ubSoldierClass = SOLDIER_CLASS_GREEN_MILITIA;
					ubNumGreen--;
				}
				else
					SLOGE("AddSoldierInitListMilitia: something wrong with random");
				curr->pBasicPlacement->bTeam = MILITIA_TEAM;
				curr->pBasicPlacement->bOrders = STATIONARY;
				curr->pBasicPlacement->bAttitude = (INT8) Random( MAXATTITUDES );
				if( curr->pDetailedPlacement )
				{
					//delete the detailed placement information.
					delete curr->pDetailedPlacement;
					curr->pDetailedPlacement = NULL;
					curr->pBasicPlacement->fDetailedPlacement = FALSE;
					RandomizeRelativeLevel( &( curr->pBasicPlacement->bRelativeAttributeLevel), curr->pBasicPlacement->ubSoldierClass );
					RandomizeRelativeLevel( &( curr->pBasicPlacement->bRelativeEquipmentLevel), curr->pBasicPlacement->ubSoldierClass );
				}
				if( AddPlacementToWorld( curr ) )
				{
					ubMaxNum--;
				}
				else
					return;
			}
			ubFreeSlots--;
			//With the decrementing of the slot vars in this manner, the chances increase so that all slots
			//will be full by the time the end of the list comes up.
		}
	}
}

void AddSoldierInitListCreatures(BOOLEAN fQueen, UINT8 ubNumLarvae, UINT8 ubNumInfants,
					UINT8 ubNumYoungMales, UINT8 ubNumYoungFemales,
					UINT8 ubNumAdultMales, UINT8 ubNumAdultFemales )
{
	INT32 iRandom;
	UINT8 ubFreeSlots;
	BOOLEAN fDoPlacement;
	UINT8 ubNumCreatures;

	SortSoldierInitList();

	//Okay, if we have a queen, place her first.  She MUST have a special placement, else
	//we can't use anything.
	ubNumCreatures = (UINT8)(ubNumLarvae + ubNumInfants + ubNumYoungMales + ubNumYoungFemales + ubNumAdultMales + ubNumAdultFemales);
	if( fQueen )
	{
		FOR_EACH_SOLDIERINITNODE(curr)
		{
			if( !curr->pSoldier && curr->pBasicPlacement->bTeam == CREATURE_TEAM && curr->pBasicPlacement->bBodyType == QUEENMONSTER )
			{
				if( !AddPlacementToWorld( curr ) )
				{
					fQueen = FALSE;
					break;
				}
			}
		}
		if( !fQueen )
		{
			SLOGE("Couldn't place the queen.");
		}
	}

	//First fill up only the priority existance slots (as long as the availability and bodytypes match)
	FOR_EACH_SOLDIERINITNODE(curr)
	{
		if (!curr->pBasicPlacement->fPriorityExistance || ubNumCreatures == 0) break;
		fDoPlacement = TRUE;

		if( curr->pBasicPlacement->bTeam == CREATURE_TEAM )
		{
			//Matching team, now check the soldier class...
			if( ubNumLarvae && curr->pBasicPlacement->bBodyType == LARVAE_MONSTER )
				ubNumLarvae--;
			else if( ubNumInfants && curr->pBasicPlacement->bBodyType == INFANT_MONSTER )
				ubNumInfants--;
			else if( ubNumYoungMales && curr->pBasicPlacement->bBodyType == YAM_MONSTER )
				ubNumYoungMales--;
			else if( ubNumYoungFemales && curr->pBasicPlacement->bBodyType == YAF_MONSTER )
				ubNumYoungFemales--;
			else if( ubNumAdultMales && curr->pBasicPlacement->bBodyType == AM_MONSTER )
				ubNumAdultMales--;
			else if( ubNumAdultFemales && curr->pBasicPlacement->bBodyType == ADULTFEMALEMONSTER )
				ubNumAdultFemales--;
			else
				fDoPlacement = FALSE;
			if ( fDoPlacement )
			{
				if( AddPlacementToWorld( curr ) )
				{
					ubNumCreatures--;
				}
				else
					return;
			}
		}
	}
	if( !ubNumCreatures )
		return;

	//Count how many free creature slots are left.
	ubFreeSlots = 0;
	CFOR_EACH_SOLDIERINITNODE(curr)
	{
		if( !curr->pSoldier && curr->pBasicPlacement->bTeam == CREATURE_TEAM )
			ubFreeSlots++;
	}
	//Now, if we still have creatures to place, do so completely randomly, overriding priority
	//placements, etc.
	FOR_EACH_SOLDIERINITNODE(curr)
	{
		if (ubFreeSlots == 0 || ubNumCreatures == 0) break;
		if( !curr->pSoldier && curr->pBasicPlacement->bTeam == CREATURE_TEAM )
		{
			//Randomly determine if we will use this slot; the more available slots in proportion to
			//the number of enemies, the lower the chance of accepting the slot.
			if( ubFreeSlots <= ubNumCreatures || Random( ubFreeSlots ) < ubNumCreatures )
			{
				//Choose which team to use.
				iRandom = Random( ubNumCreatures );

				if( ubNumLarvae && iRandom < ubNumLarvae )
				{
					ubNumLarvae--;
					curr->pBasicPlacement->bBodyType = LARVAE_MONSTER;
				}
				else if( ubNumInfants && iRandom < ubNumLarvae + ubNumInfants )
				{
					ubNumInfants--;
					curr->pBasicPlacement->bBodyType = INFANT_MONSTER;
				}
				else if( ubNumYoungMales && iRandom < ubNumLarvae + ubNumInfants + ubNumYoungMales )
				{
					ubNumYoungMales--;
					curr->pBasicPlacement->bBodyType = YAM_MONSTER;
				}
				else if( ubNumYoungFemales && iRandom < ubNumLarvae + ubNumInfants + ubNumYoungMales + ubNumYoungFemales )
				{
					ubNumYoungFemales--;
					curr->pBasicPlacement->bBodyType = YAF_MONSTER;
				}
				else if( ubNumAdultMales && iRandom < ubNumLarvae + ubNumInfants + ubNumYoungMales + ubNumYoungFemales + ubNumAdultMales )
				{
					ubNumAdultMales--;
					curr->pBasicPlacement->bBodyType = AM_MONSTER;
				}
				else if( ubNumAdultFemales && iRandom < ubNumLarvae + ubNumInfants + ubNumYoungMales + ubNumYoungFemales + ubNumAdultMales + ubNumAdultFemales )
				{
					ubNumAdultFemales--;
					curr->pBasicPlacement->bBodyType = ADULTFEMALEMONSTER;
				}
				else
					SLOGA("AddSoldierInitListCreatures: something wrong with random");
				if( curr->pDetailedPlacement )
				{ //delete the detailed placement information.
					delete curr->pDetailedPlacement;
					curr->pDetailedPlacement = NULL;
					curr->pBasicPlacement->fDetailedPlacement = FALSE;
				}
				if( AddPlacementToWorld( curr ) )
				{
					ubNumCreatures--;
				}
				else
				{
					return;
				}
			}
			ubFreeSlots--;
			//With the decrementing of the slot vars in this manner, the chances increase so that all slots
			//will be full by the time the end of the list comes up.
		}
	}
}


SOLDIERINITNODE* FindSoldierInitNodeWithID( UINT16 usID )
{
	FOR_EACH_SOLDIERINITNODE(curr)
	{
		if( curr->pSoldier->ubID == usID )
			return curr;
	}
	return NULL;
}


SOLDIERINITNODE* FindSoldierInitNodeBySoldier(SOLDIERTYPE const& s)
{
	FOR_EACH_SOLDIERINITNODE(i)
	{
		if (i->pSoldier == &s) return i;
	}
	return 0;
}


void UseEditorOriginalList()
{
	SOLDIERINITNODE *curr;
	gfOriginalList = TRUE;
	gSoldierInitHead = gOriginalSoldierInitListHead;
	curr = gSoldierInitHead;
	if( curr )
	{
		while( curr->next )
			curr = curr->next;
	}
	if( curr )
		gSoldierInitTail = curr;
}

void UseEditorAlternateList()
{
	SOLDIERINITNODE *curr;
	gfOriginalList = FALSE;
	gSoldierInitHead = gAlternateSoldierInitListHead;
	curr = gSoldierInitHead;
	if( curr )
	{
		while( curr->next )
			curr = curr->next;
	}
	if( curr )
		gSoldierInitTail = curr;
}


void EvaluateDeathEffectsToSoldierInitList(SOLDIERTYPE const& s)
{
	if (s.bTeam == MILITIA_TEAM) return;
	SOLDIERINITNODE* const curr = FindSoldierInitNodeBySoldier(s);
	if (!curr || !curr->pDetailedPlacement) return;
	delete curr->pDetailedPlacement;
	curr->pDetailedPlacement = 0;
	curr->pSoldier           = 0;
}


//For the purpose of keeping track of which soldier belongs to which placement within the game,
//the only way we can do this properly is to save the soldier ID from the list and reconnect the
//soldier pointer whenever we load the game.
void SaveSoldierInitListLinks(HWFILE const hfile)
{
	UINT8 ubSlots = 0;

	//count the number of soldier init nodes...
	CFOR_EACH_SOLDIERINITNODE(curr) ++ubSlots;
	//...and save it.
	hfile->write(&ubSlots, 1);
	//Now, go through each node, and save just the ubSoldierID, if that soldier is alive.
	FOR_EACH_SOLDIERINITNODE(curr)
	{
		if( curr->pSoldier && !curr->pSoldier->bActive )
		{
			curr->ubSoldierID = 0;
		}
		hfile->write(&curr->ubNodeID,    1);
		hfile->write(&curr->ubSoldierID, 1);
	}
}


void LoadSoldierInitListLinks(HWFILE const f)
{
	UINT8 slots;
	f->read(&slots, 1);
	for (UINT8 n = slots; n != 0; --n)
	{
		UINT8 node_id;
		UINT8 soldier_id;
		f->read(&node_id,    1);
		f->read(&soldier_id, 1);

		if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) continue;

		FOR_EACH_SOLDIERINITNODE(curr)
		{
			if (curr->ubNodeID != node_id) continue;

			curr->ubSoldierID = soldier_id;
			TacticalTeamType const* const team = gTacticalStatus.Team;
			if ((team[ENEMY_TEAM].bFirstID <= soldier_id &&
				soldier_id <= team[CREATURE_TEAM].bLastID) ||
				(team[CIV_TEAM].bFirstID <= soldier_id &&
				soldier_id <= team[CIV_TEAM].bLastID))
			{
				// only enemies, creatures and civilians
				curr->pSoldier = &GetMan(soldier_id);
			}
		}
	}
}


void AddSoldierInitListBloodcats()
{
	SECTORINFO *pSector;
	UINT8 ubSectorID;

	if (gWorldSector.z)
	{
		return; //no bloodcats underground.
	}

	ubSectorID = gWorldSector.AsByte();
	pSector = &SectorInfo[ ubSectorID ];

	if( !pSector->bBloodCatPlacements )
	{ //This map has no bloodcat placements, so don't waste CPU time.
		return;
	}

	if( pSector->bBloodCatPlacements )
	{ //We don't yet know the number of bloodcat placements in this sector so
		//count them now, and permanently record it.
		INT8 bBloodCatPlacements = 0;
		CFOR_EACH_SOLDIERINITNODE(curr)
		{
			if( curr->pBasicPlacement->bBodyType == BLOODCAT )
			{
				bBloodCatPlacements++;
			}
		}

		auto spawns = GCM->getBloodCatSpawnsOfSector( ubSectorID );
		if( bBloodCatPlacements != pSector->bBloodCatPlacements && spawns == NULL )
		{
			pSector->bBloodCatPlacements = bBloodCatPlacements;
			pSector->bBloodCats = -1;
			if( !bBloodCatPlacements )
			{
				return;
			}
		}
	}
	if( pSector->bBloodCats > 0 )
	{
		//Add them to the world now...
		UINT8 ubNumAdded = 0;
		UINT8 ubMaxNum = (UINT8)pSector->bBloodCats;
		SOLDIERINITNODE *mark;
		UINT8 ubSlotsToFill;
		UINT8 ubSlotsAvailable;
		SOLDIERINITNODE *curr;

		//Sort the list in the following manner:
		//-Priority placements first
		//-Basic placements next
		//-Any placements with existing soldiers last (overrides others)
		SortSoldierInitList();

		//Count the current number of soldiers of the specified team
		CFOR_EACH_SOLDIERINITNODE(curr)
		{
			if( curr->pBasicPlacement->bBodyType == BLOODCAT && curr->pSoldier )
				ubNumAdded++;  //already one here!
		}

		curr = gSoldierInitHead;

		//First fill up all of the priority existance slots...
		while( curr && curr->pBasicPlacement->fPriorityExistance && ubNumAdded < ubMaxNum )
		{
			if( curr->pBasicPlacement->bBodyType == BLOODCAT )
			{
				//Matching team, so add this placement.
				if( AddPlacementToWorld( curr ) )
				{
					ubNumAdded++;
				}
			}
			curr = curr->next;
		}
		if( ubNumAdded == ubMaxNum )
			return;

		//Now count the number of nodes that are basic placements of desired team
		//This information will be used to randomly determine which of these placements
		//will be added based on the number of slots we can still add.
		mark = curr;
		ubSlotsAvailable = 0;
		ubSlotsToFill = ubMaxNum - ubNumAdded;
		while( curr && !curr->pSoldier && ubNumAdded < ubMaxNum )
		{
			if( curr->pBasicPlacement->bBodyType == BLOODCAT )
				ubSlotsAvailable++;
			curr = curr->next;
		}

		//we now have the number, so compared it to the num we can add, and determine how we will
		//randomly determine which nodes to add.
		if( !ubSlotsAvailable )
		{	//There aren't any basic placements of desired team, so exit.
			return;
		}
		curr = mark;
		//while we have a list, with no active soldiers, the num added is less than the max num requested, and
		//we have slots available, process the list to add new soldiers.
		while( curr && !curr->pSoldier && ubNumAdded < ubMaxNum && ubSlotsAvailable )
		{
			if( curr->pBasicPlacement->bBodyType == BLOODCAT )
			{
				if( ubSlotsAvailable <= ubSlotsToFill || Random( ubSlotsAvailable ) < ubSlotsToFill )
				{
					//found matching team, so add this soldier to the game.
					if( AddPlacementToWorld( curr ) )
					{
						ubNumAdded++;
					}
					else
					{
						//if it fails to create the soldier, it is likely that it is because the slots in the tactical
						//engine are already full.  Besides, the strategic AI shouldn't be trying to fill a map with
						//more than the maximum allowable soldiers of team.  All teams can have a max of 32 individuals,
						//except for the player which is 20.  Players aren't processed in this list anyway.
						return;
					}
					ubSlotsToFill--;
				}
				ubSlotsAvailable--;
				//With the decrementing of the slot vars in this manner, the chances increase so that all slots
				//will be full by the time the end of the list comes up.
			}
			curr = curr->next;
		}
		return;
	}
}


static SOLDIERINITNODE* FindSoldierInitListNodeByProfile(UINT8 ubProfile)
{
	FOR_EACH_SOLDIERINITNODE(curr)
	{
		if ( curr->pDetailedPlacement && curr->pDetailedPlacement->ubProfile == ubProfile )
		{
			return( curr );
		}
	}
	return( NULL );
}


// Loop through the profiles starting at the RPCs, add them using strategic
// insertion information and not editor placements. The key flag involved for
// doing it this way is the GetProfile(i).fUseProfileInsertionInfo.
void AddProfilesUsingProfileInsertionData()
{
	for (const MercProfile* prof : GCM->listMercProfiles())
	{
		if (!prof->isNPCorRPC() && !prof->isVehicle())   continue;

		// Perform various checks to make sure the soldier is actually in the same
		// sector, alive and so on. More importantly, the flag to use profile
		// insertion data must be set.
		ProfileID                i = prof->getID();
		MERCPROFILESTRUCT const& p = prof->getStruct();
		if (p.sSector != gWorldSector)                   continue;
		if (p.ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED) continue;
		if (p.ubMiscFlags & PROFILE_MISC_FLAG_EPCACTIVE) continue;
		if (p.bLife == 0)                                continue;
		if (!p.fUseProfileInsertionInfo)                 continue;

		SOLDIERTYPE* ps = FindSoldierByProfileID(i);
		if (!ps)
		{
			// Create a new soldier, as this one doesn't exist
			SOLDIERCREATE_STRUCT c;
			c = SOLDIERCREATE_STRUCT{};
			c.bTeam     = CIV_TEAM;
			c.ubProfile = i;
			c.sSector  = gWorldSector;
			ps = TacticalCreateSoldier(c);
			if (!ps) continue; // XXX exception?
		}
		SOLDIERTYPE& s = *ps;

		// Insert the soldier
		s.ubStrategicInsertionCode = p.ubStrategicInsertionCode;
		s.usStrategicInsertionData = p.usStrategicInsertionData;
		UpdateMercInSector(s, gWorldSector);

		// check action ID values
		if (p.ubQuoteRecord != 0)
		{
			s.ubQuoteRecord   = p.ubQuoteRecord;
			s.ubQuoteActionID = p.ubQuoteActionID;
			if (s.ubQuoteActionID == QUOTE_ACTION_ID_CHECKFORDEST)
			{
				// Gridno will have been changed to destination, so we're there
				NPCReachedDestination(&s, FALSE);
			}
		}

		// Make sure this person's pointer is set properly in the init list
		if (SOLDIERINITNODE* const init = FindSoldierInitListNodeByProfile(s.ubProfile))
		{
			init->pSoldier    = &s;
			init->ubSoldierID = s.ubID;
			// Also connect schedules here
			SOLDIERCREATE_STRUCT& dp = *init->pDetailedPlacement;
			if (dp.ubScheduleID != 0)
			{
				if (SCHEDULENODE* const sched = GetSchedule(dp.ubScheduleID))
				{
					sched->soldier = &s;
					s.ubScheduleID = dp.ubScheduleID;
				}
			}
		}
	}
}


void AddProfilesNotUsingProfileInsertionData()
{
	FOR_EACH_SOLDIERINITNODE(i)
	{
		SOLDIERINITNODE& si = *i;
		if (si.pSoldier)                           continue;
		if (si.pBasicPlacement->bTeam != CIV_TEAM) continue;
		if (!si.pDetailedPlacement)                continue;
		ProfileID const pid = si.pDetailedPlacement->ubProfile;
		if (pid == NO_PROFILE)                     continue;
		MERCPROFILESTRUCT const& p = GetProfile(pid);
		if (p.fUseProfileInsertionInfo)            continue;
		if (p.bLife == 0)                          continue;
		AddPlacementToWorld(&si);
	}
}


void NewWayOfLoadingEnemySoldierInitListLinks(HWFILE const f)
{
	UINT8 slots;
	f->read(&slots, 1);
	for (UINT8 n = slots; n != 0; --n)
	{
		UINT8 node_id;
		UINT8 soldier_id;
		f->read(&node_id,    1);
		f->read(&soldier_id, 1);

		if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) continue;

		FOR_EACH_SOLDIERINITNODE(curr)
		{
			if (curr->ubNodeID != node_id) continue;

			curr->ubSoldierID = soldier_id;
			TacticalTeamType const* const team = gTacticalStatus.Team;
			if (soldier_id < team[ENEMY_TEAM].bFirstID || team[CREATURE_TEAM].bLastID < soldier_id) continue;
			// only enemies and creatures
			curr->pSoldier = &GetMan(soldier_id);
		}
	}
}


void NewWayOfLoadingCivilianInitListLinks(HWFILE const f)
{
	UINT8 slots;
	f->read(&slots, 1);
	for (UINT8 n = slots; n != 0; --n)
	{
		UINT8 node_id;
		UINT8 soldier_id;
		f->read(&node_id,    1);
		f->read(&soldier_id, 1);

		if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) continue;

		FOR_EACH_SOLDIERINITNODE(curr)
		{
			if (curr->ubNodeID != node_id) continue;

			curr->ubSoldierID = soldier_id;
			TacticalTeamType const* const team = gTacticalStatus.Team;
			if (soldier_id < team[CIV_TEAM].bFirstID || team[CIV_TEAM].bLastID < soldier_id) continue;
			// only civilians
			curr->pSoldier = &GetMan(soldier_id);
		}
	}
}


void StripEnemyDetailedPlacementsIfSectorWasPlayerLiberated()
{
	if (!gfWorldLoaded || gWorldSector.z != 0)
	{
		// No world loaded or underground.  Underground sectors don't matter seeing
		// enemies (not creatures) never rejuvenate underground.
		return;
	}

	SECTORINFO const& sector = SectorInfo[gWorldSector.AsByte()];
	if (sector.uiTimeLastPlayerLiberated == 0)
	{
		// The player has never owned the sector.
		return;
	}

	// The player has owned the sector at one point.  By stripping all of the
	// detailed placements, only basic placements will remain.  This prevents
	// tanks and "specially detailed" enemies from coming back.
	FOR_EACH_SOLDIERINITNODE(i)
	{
		SOLDIERINITNODE&            si = *i;
		BASIC_SOLDIERCREATE_STRUCT& bp = *si.pBasicPlacement;
		if (bp.bTeam != ENEMY_TEAM) continue;
		if (!si.pDetailedPlacement) continue;

		delete si.pDetailedPlacement;
		si.pDetailedPlacement = 0;
		bp.fDetailedPlacement = FALSE;
		bp.fPriorityExistance = FALSE;
		bp.bBodyType          = BODY_RANDOM;
		RandomizeRelativeLevel(&bp.bRelativeAttributeLevel, bp.ubSoldierClass);
		RandomizeRelativeLevel(&bp.bRelativeEquipmentLevel, bp.ubSoldierClass);
	}
}
