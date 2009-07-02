#include <stdexcept>

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
#include "Debug_Control.h"
#include "Inventory_Choosing.h"
#include "Campaign_Types.h"
#include "AI.h"
#include "NPC.h"
#include "Scheduling.h"
#include "MemMan.h"
#include "FileMan.h"

#ifdef JA2BETAVERSION
#	include "MessageBoxScreen.h"
#	include "SaveLoadScreen.h"
#	include "ScreenIDs.h"
#endif


BOOLEAN gfOriginalList = TRUE;

SOLDIERINITNODE *gSoldierInitHead = NULL;
SOLDIERINITNODE *gSoldierInitTail = NULL;

SOLDIERINITNODE *gOriginalSoldierInitListHead = NULL;
SOLDIERINITNODE *gAlternateSoldierInitListHead = NULL;

#ifdef JA2BETAVERSION
	BOOLEAN gfDoDialogOnceGameScreenFadesIn = FALSE;
#endif


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

SOLDIERINITNODE* AddBasicPlacementToSoldierInitList( BASIC_SOLDIERCREATE_STRUCT *pBasicPlacement )
{
	//Allocate memory for node
	SOLDIERINITNODE* const curr = MALLOCZ(SOLDIERINITNODE);

	//Allocate memory for basic placement
	curr->pBasicPlacement = MALLOC(BASIC_SOLDIERCREATE_STRUCT);

	//Copy memory for basic placement
	*curr->pBasicPlacement = *pBasicPlacement;

	//It is impossible to set up detailed placement stuff now.
	//If there is any detailed placement information during map load, it will be added
	//immediately after this function call.
	curr->pDetailedPlacement = NULL;
	curr->pSoldier = NULL;

	//Insert the new node in the list in its proper place.
	if( !gSoldierInitHead )
	{
		gSoldierInitHead = curr;
		if( gfOriginalList )
			gOriginalSoldierInitListHead = curr;
		else
			gAlternateSoldierInitListHead = curr;
		gSoldierInitTail = curr;
		gSoldierInitHead->next = NULL;
		gSoldierInitHead->prev = NULL;
	}
	else
	{
		//TEMP:  no sorting, just enemies
		curr->prev = gSoldierInitTail;
		curr->next = NULL;
		gSoldierInitTail->next = curr;
		gSoldierInitTail = gSoldierInitTail->next;
	}
	if( gfOriginalList )
		gMapInformation.ubNumIndividuals++;
	return curr;
}


void RemoveSoldierNodeFromInitList( SOLDIERINITNODE *pNode )
{
	if( !pNode )
		return;
	if( gfOriginalList )
		gMapInformation.ubNumIndividuals--;
	if( pNode->pBasicPlacement )
	{
		MemFree( pNode->pBasicPlacement );
		pNode->pBasicPlacement = NULL;
	}
	if( pNode->pDetailedPlacement )
	{
		MemFree( pNode->pDetailedPlacement );
		pNode->pDetailedPlacement = NULL;
	}
	if( pNode->pSoldier )
	{
		if( pNode->pSoldier->ubID >= 20 )
		{
			TacticalRemoveSoldier(pNode->pSoldier);
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
	MemFree( pNode );
}


#ifdef JA2EDITOR

//These serialization functions are assuming the passing of a valid file
//pointer to the beginning of the save/load area, which is not necessarily at
//the beginning of the file.  This is just a part of the whole map serialization.
BOOLEAN SaveSoldiersToMap( HWFILE fp )
{
	UINT32 i;
	SOLDIERINITNODE *curr;

	if( !fp )
		return FALSE;

	if( gMapInformation.ubNumIndividuals > MAX_INDIVIDUALS )
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
		FileWrite(fp, curr->pBasicPlacement, sizeof(BASIC_SOLDIERCREATE_STRUCT));

		if( curr->pBasicPlacement->fDetailedPlacement )
		{
			if( !curr->pDetailedPlacement )
				return FALSE;
			InjectSoldierCreateIntoFileUTF16(fp, curr->pDetailedPlacement);
		}
		curr = curr->next;
	}
	return TRUE;
}

#endif


void LoadSoldiersFromMap(INT8** const hBuffer)
{
	UINT32 i;
	UINT8 ubNumIndividuals;
	BASIC_SOLDIERCREATE_STRUCT tempBasicPlacement;
	SOLDIERINITNODE *pNode;
	BOOLEAN fCowInSector = FALSE;

	ubNumIndividuals = gMapInformation.ubNumIndividuals;

	UseEditorAlternateList();
	KillSoldierInitList();
	UseEditorOriginalList();
	KillSoldierInitList();

	InitSoldierInitList();

	if (ubNumIndividuals > MAX_INDIVIDUALS)
	{
		throw std::runtime_error("Corrupt map check failed.  ubNumIndividuals is greater than MAX_INDIVIDUALS.");
	}
	if (ubNumIndividuals == 0) return; // no mercs

	//Because we are loading the map, we needed to know how many
	//guys are being loaded, but when we add them to the list here, it
	//automatically increments that number, effectively doubling it, which
	//would be a problem.  Now that we know the number, we clear it here, so
	//it gets built again.
	gMapInformation.ubNumIndividuals = 0;		//MUST BE CLEARED HERE!!!

	for( i=0; i < ubNumIndividuals; i++ )
	{
		LOADDATA( &tempBasicPlacement, *hBuffer, sizeof( BASIC_SOLDIERCREATE_STRUCT ) );
		pNode = AddBasicPlacementToSoldierInitList( &tempBasicPlacement );
		pNode->ubNodeID = (UINT8)i;
		if( tempBasicPlacement.fDetailedPlacement )
		{ //Add the static detailed placement information in the same newly created node as the basic placement.
			//read static detailed placement from file
			SOLDIERCREATE_STRUCT* const Soldier = MALLOC(SOLDIERCREATE_STRUCT);

			BYTE Data[1040];
			LOADDATA(Data, *hBuffer, sizeof(Data));
			ExtractSoldierCreateUTF16(Data, Soldier);

			if (Soldier->ubProfile != NO_PROFILE)
			{
				UINT8 CivilianGroup = gMercProfiles[Soldier->ubProfile].ubCivilianGroup;
				Soldier->ubCivilianGroup                = CivilianGroup;
				pNode->pBasicPlacement->ubCivilianGroup = CivilianGroup;
			}

			pNode->pDetailedPlacement = Soldier;
		}
		if( tempBasicPlacement.bBodyType == COW )
		{
			fCowInSector = TRUE;
		}
	}
	if( fCowInSector )
	{
		char str[40];
		sprintf( str, "Sounds/cowmoo%d.wav", Random( 3 ) + 1 );
		PlayJA2SampleFromFile(str, MIDVOLUME, 1, MIDDLEPAN);
	}
}


//Because soldiers, creatures, etc., maybe added to the game at anytime theoretically, the
//list will need to be sorted to reflect this.  It is quite likely that this won't be needed,
//but the flexibility is there just incase.  Now the list is sorted in the following manner:
//-1st priority:  Any nodes containing valid pointers to soldiers are moved to the end of the list.
//								We don't ever want to use two identical placements.
//-2nd priority:  Any nodes that have priority existance and detailed placement information are
//								put first in the list.
//-3rd priority:	Any nodes that have priority existance and no detailed placement information are used next.
//-4th priority:	Any nodes that have detailed placement and no priority existance information are used next.
//-5th priority:  The rest of the nodes are basic placements and are placed in the center of the list.  Of
//								these, they are randomly filled based on the number needed.
//NOTE:  This function is called by AddSoldierInitListTeamToWorld().  There is no other place it needs to
//			 be called.
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
		{	//This node has an existing soldier, so move to end of list.
			//copy node
			temp = curr;
			if( temp == gSoldierInitHead )
			{ //If we dealing with the head, we need to move it now.
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
		{	//Priority existance nodes without detailed placement info are moved to beginning of list
			//copy node
			temp = curr;
			if( temp == gSoldierInitTail )
			{ //If we dealing with the tail, we need to move it now.
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
		{	//Priority existance nodes without detailed placement info are moved to beginning of list
			//copy node
			temp = curr;
			if( temp == gSoldierInitTail )
			{ //If we dealing with the tail, we need to move it now.
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
			{ //If we dealing with the tail, we need to move it now.
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
	//			 priority placements are at the beginning of the list, then the
	//			 basic placements are in the middle.
}

BOOLEAN AddPlacementToWorld( SOLDIERINITNODE *curr )
{
	UINT8 ubProfile;
	SOLDIERCREATE_STRUCT tempDetailedPlacement;
	// First check if this guy has a profile and if so check his location such that it matches!
	// Get profile from placement info
	memset( &tempDetailedPlacement, 0, sizeof( SOLDIERCREATE_STRUCT ) );

	if( curr->pDetailedPlacement )
	{
		ubProfile = curr->pDetailedPlacement->ubProfile;

		if( ubProfile != NO_PROFILE && !gfEditMode )
		{
			if( gMercProfiles[ ubProfile ].sSectorX != gWorldSectorX ||
				  gMercProfiles[ ubProfile ].sSectorY != gWorldSectorY ||
					gMercProfiles[ ubProfile ].bSectorZ != gbWorldSectorZ ||
					gMercProfiles[ ubProfile ].ubMiscFlags & (PROFILE_MISC_FLAG_RECRUITED | PROFILE_MISC_FLAG_EPCACTIVE) ||
//				gMercProfiles[ ubProfile ].ubMiscFlags2 & PROFILE_MISC_FLAG2_DONT_ADD_TO_SECTOR ||
					!gMercProfiles[ ubProfile ].bLife	||
					gMercProfiles[ ubProfile ].fUseProfileInsertionInfo
				 )
			{
				return FALSE;
			}
		}
		//Special case code when adding icecream truck.
		if( !gfEditMode )
		{
			// CJC, August 18, 1999: don't do this code unless the ice cream truck is on our team
			if (FindSoldierByProfileIDOnPlayerTeam(ICECREAMTRUCK) != NULL)
			{
				if( curr->pDetailedPlacement->bBodyType == ICECREAMTRUCK )
				{ //Check to see if Hamous is here and not recruited.  If so, add truck
					if( gMercProfiles[ HAMOUS ].sSectorX != gWorldSectorX ||
							gMercProfiles[ HAMOUS ].sSectorY != gWorldSectorY ||
							gMercProfiles[ HAMOUS ].bSectorZ )
					{ //not here, so don't add
						return TRUE;
					}
					//Hamous is here.  Check to make sure he isn't recruited.
					if( gMercProfiles[ HAMOUS ].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED )
					{
						return TRUE;
					}
				}
			}
		}
		CreateDetailedPlacementGivenStaticDetailedPlacementAndBasicPlacementInfo(
			&tempDetailedPlacement, curr->pDetailedPlacement, curr->pBasicPlacement );
	}
	else
	{
		CreateDetailedPlacementGivenBasicPlacementInfo( &tempDetailedPlacement, curr->pBasicPlacement );
	}

	if ( !gfEditMode )
	{
		if( tempDetailedPlacement.bTeam == CIV_TEAM )
		{
			// quest-related overrides
			if ( gWorldSectorX == 5 && gWorldSectorY == MAP_ROW_C )
			{
				// Kinpin guys might be guarding Tony
				if ( tempDetailedPlacement.ubCivilianGroup == KINGPIN_CIV_GROUP && ( gTacticalStatus.fCivGroupHostile[ KINGPIN_CIV_GROUP ] == CIV_GROUP_WILL_BECOME_HOSTILE || ( (gubQuest[ QUEST_KINGPIN_MONEY ] == QUESTINPROGRESS) && (CheckFact( FACT_KINGPIN_CAN_SEND_ASSASSINS, KINGPIN )) ) ) )
				{
					if (tempDetailedPlacement.ubProfile == NO_PROFILE)
					{
						// these guys should be guarding Tony!
						tempDetailedPlacement.sInsertionGridNo = 13531 +
							(INT16) ( PreRandom( 8 ) * ( PreRandom( 1 ) ? -1 : 1)
							+ PreRandom( 8 ) * ( PreRandom( 1 ) ? -1 : 1) * WORLD_ROWS );

						switch( PreRandom( 3 ) )
						{
							case 0:
								tempDetailedPlacement.bOrders = ONGUARD;
								break;
							case 1:
								tempDetailedPlacement.bOrders = CLOSEPATROL;
								break;
							case 2:
								tempDetailedPlacement.bOrders = ONCALL;
								break;

						}
					}
					else if (tempDetailedPlacement.ubProfile == BILLY )
					{
						// billy should now be able to roam around
						tempDetailedPlacement.sInsertionGridNo = 13531 +
							(INT16) ( PreRandom( 30 ) * ( PreRandom( 1 ) ? -1 : 1)
							+ PreRandom( 30 ) * ( PreRandom( 1 ) ? -1 : 1) * WORLD_ROWS );
						tempDetailedPlacement.bOrders = SEEKENEMY;
					}
					else if ( tempDetailedPlacement.ubProfile == MADAME )
					{
						// she shouldn't be here!
						return( TRUE );
					}
					else if (tempDetailedPlacement.ubProfile == NO_PROFILE)
					{
						UINT8 const room = GetRoom(tempDetailedPlacement.sInsertionGridNo);
						if (IN_BROTHEL(room))
						{
							// must be a hooker, shouldn't be there
							return TRUE;
						}
					}
				}
			}
			else if ( !gfInMeanwhile && gWorldSectorX == 3 && gWorldSectorY == 16 && !gbWorldSectorZ )
			{ //Special civilian setup for queen's palace.
				if( gubFact[ FACT_QUEEN_DEAD ] )
				{
					if( tempDetailedPlacement.ubCivilianGroup == QUEENS_CIV_GROUP )
					{ //The queen's civs aren't added if queen is dead
						return TRUE;
					}
				}
				else
				{
					if( gfUseAlternateQueenPosition && tempDetailedPlacement.ubProfile == QUEEN )
					{
						tempDetailedPlacement.sInsertionGridNo = 11448;
					}
					if( tempDetailedPlacement.ubCivilianGroup != QUEENS_CIV_GROUP )
					{ //The free civilians aren't added if queen is alive
						return TRUE;
					}
				}
			}
			else if ( gWorldSectorX == TIXA_SECTOR_X && gWorldSectorY == TIXA_SECTOR_Y  && gbWorldSectorZ == 0 )
			{
				// Tixa prison, once liberated, should not have any civs without profiles unless
				// they are kids
				if( !StrategicMap[ TIXA_SECTOR_X + TIXA_SECTOR_Y * MAP_WORLD_X ].fEnemyControlled && tempDetailedPlacement.ubProfile == NO_PROFILE && tempDetailedPlacement.bBodyType != HATKIDCIV && tempDetailedPlacement.bBodyType != KIDCIV )
				{
					// not there
					return( TRUE );
				}
			}
			else if ( gWorldSectorX == 13 && gWorldSectorY == MAP_ROW_C && gbWorldSectorZ == 0 )
			{
				if ( CheckFact( FACT_KIDS_ARE_FREE, 0 ) )
				{
					if ( tempDetailedPlacement.bBodyType == HATKIDCIV || tempDetailedPlacement.bBodyType == KIDCIV )
					{
						// not there any more!  kids have been freeeeed!
						return( TRUE );
					}
				}
			}
		}

		//SPECIAL!  Certain events in the game can cause profiled NPCs to become enemies.  The two cases are
		//adding Mike and Iggy.  We will only add one NPC in any given combat and the conditions for setting
		//the associated facts are done elsewhere.  There is also another place where NPCs can get added, which
		//is in TacticalCreateElite() used for inserting offensive enemies.
		if( tempDetailedPlacement.bTeam == ENEMY_TEAM && tempDetailedPlacement.ubSoldierClass == SOLDIER_CLASS_ELITE )
		{
			OkayToUpgradeEliteToSpecialProfiledEnemy( &tempDetailedPlacement );
		}
	}

	SOLDIERTYPE* pSoldier = TacticalCreateSoldier(tempDetailedPlacement);
	if (pSoldier != NULL)
	{
		curr->pSoldier = pSoldier;
		curr->ubSoldierID = pSoldier->ubID;
		AddSoldierToSectorNoCalculateDirection(pSoldier);

		if (pSoldier->bInSector && pSoldier->bTeam == ENEMY_TEAM && !pSoldier->inv[HANDPOS].usItem)
		{
			pSoldier = pSoldier;
		}

		return TRUE;
	}
	else
	{
		LiveMessage( "Failed to create soldier using TacticalCreateSoldier within AddPlacementToWorld" );
	}
	return FALSE;
}


UINT8 AddSoldierInitListTeamToWorld( INT8 bTeam, UINT8 ubMaxNum )
{
	UINT8 ubNumAdded = 0;
	SOLDIERINITNODE *mark;
	UINT8 ubSlotsToFill;
	UINT8 ubSlotsAvailable;

	//Sort the list in the following manner:
	//-Priority placements first
	//-Basic placements next
	//-Any placements with existing soldiers last (overrides others)
	SortSoldierInitList();

	if (giCurrentTilesetID == CAVES_1) //cave/mine tileset only
	{ //convert all civilians to miners which use uniforms and more masculine body types.
		CFOR_ALL_SOLDIERINITNODES(curr)
		{
			if( curr->pBasicPlacement->bTeam == CIV_TEAM && !curr->pDetailedPlacement )
			{
				curr->pBasicPlacement->ubSoldierClass = SOLDIER_CLASS_MINER;
				curr->pBasicPlacement->bBodyType = -1;
			}
		}
	}

	//Count the current number of soldiers of the specified team
	CFOR_ALL_SOLDIERINITNODES(curr)
	{
		if( curr->pBasicPlacement->bTeam == bTeam && curr->pSoldier )
			ubNumAdded++;  //already one here!
	}

	SOLDIERINITNODE* curr = gSoldierInitHead;

	//First fill up all of the priority existance slots...
	while( curr && curr->pBasicPlacement->fPriorityExistance && ubNumAdded < ubMaxNum )
	{
		if( curr->pBasicPlacement->bTeam == bTeam )
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
		return ubNumAdded;

	//Now count the number of nodes that are basic placements of desired team
	//This information will be used to randomly determine which of these placements
	//will be added based on the number of slots we can still add.
	mark = curr;
	ubSlotsAvailable = 0;
	ubSlotsToFill = ubMaxNum - ubNumAdded;
	while( curr && !curr->pSoldier && ubNumAdded < ubMaxNum )
	{
		if( curr->pBasicPlacement->bTeam == bTeam )
			ubSlotsAvailable++;
		curr = curr->next;
	}

	//we now have the number, so compared it to the num we can add, and determine how we will
	//randomly determine which nodes to add.
	if( !ubSlotsAvailable )
	{	//There aren't any basic placements of desired team, so exit.
		return ubNumAdded;
	}
	curr = mark;
	//while we have a list, with no active soldiers, the num added is less than the max num requested, and
	//we have slots available, process the list to add new soldiers.
	while( curr && !curr->pSoldier && ubNumAdded < ubMaxNum && ubSlotsAvailable )
	{
		if( curr->pBasicPlacement->bTeam == bTeam )
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
					return ubNumAdded;
				}
				ubSlotsToFill--;
			}
			ubSlotsAvailable--;
			//With the decrementing of the slot vars in this manner, the chances increase so that all slots
			//will be full by the time the end of the list comes up.
		}
		curr = curr->next;
	}
	return ubNumAdded;
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
	CFOR_ALL_SOLDIERINITNODES(curr)
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
		FOR_ALL_SOLDIERINITNODES(curr)
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
				Assert(0);
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
	CFOR_ALL_SOLDIERINITNODES(curr)
	{
		if( !curr->pSoldier && curr->pBasicPlacement->bTeam == ENEMY_TEAM )
			ubFreeSlots++;
	}

	//Now, loop through the entire list again, but for the last time.  All enemies will be inserted now ignoring
	//detailed placements and classes.
	FOR_ALL_SOLDIERINITNODES(curr)
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
					Assert(0);
				/* DISABLE THE OVERRIDE FOR NOW...
				if( curr->pDetailedPlacement )
				{ //delete the detailed placement information.
					MemFree( curr->pDetailedPlacement );
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
				{ //delete the detailed placement information.
					MemFree( curr->pDetailedPlacement );
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
				case SOLDIER_CLASS_ELITE:						ubEliteSlots++;	break;
				case SOLDIER_CLASS_ADMINISTRATOR:		ubGreenSlots++; break;
				case SOLDIER_CLASS_ARMY:						ubRegSlots++; break;
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
	CFOR_ALL_SOLDIERINITNODES(curr)
	{
		if( !curr->pSoldier && (curr->pBasicPlacement->bTeam == ENEMY_TEAM || curr->pBasicPlacement->bTeam == MILITIA_TEAM) )
			ubFreeSlots++;
	}

	//Now, loop through the entire list again, but for the last time.  All enemies will be inserted now ignoring
	//detailed placements and classes.
	FOR_ALL_SOLDIERINITNODES(curr)
	{
		if (ubFreeSlots == 0 || ubMaxNum == 0) break;
		if( !curr->pSoldier && (curr->pBasicPlacement->bTeam == ENEMY_TEAM || curr->pBasicPlacement->bTeam == MILITIA_TEAM) )
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
					Assert(0);
				curr->pBasicPlacement->bTeam = MILITIA_TEAM;
				curr->pBasicPlacement->bOrders = STATIONARY;
				curr->pBasicPlacement->bAttitude = (INT8) Random( MAXATTITUDES );
				if( curr->pDetailedPlacement )
				{ //delete the detailed placement information.
					MemFree( curr->pDetailedPlacement );
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

void AddSoldierInitListCreatures( BOOLEAN fQueen, UINT8 ubNumLarvae, UINT8 ubNumInfants,
																	UINT8 ubNumYoungMales, UINT8 ubNumYoungFemales, UINT8 ubNumAdultMales,
																	UINT8 ubNumAdultFemales )
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
		FOR_ALL_SOLDIERINITNODES(curr)
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
			#ifdef JA2BETAVERSION
				ScreenMsg( FONT_RED, MSG_ERROR, L"Couldn't place the queen." );
			#endif
		}
	}

	//First fill up only the priority existance slots (as long as the availability and bodytypes match)
	FOR_ALL_SOLDIERINITNODES(curr)
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
	CFOR_ALL_SOLDIERINITNODES(curr)
	{
		if( !curr->pSoldier && curr->pBasicPlacement->bTeam == CREATURE_TEAM )
			ubFreeSlots++;
	}
	//Now, if we still have creatures to place, do so completely randomly, overriding priority
	//placements, etc.
	FOR_ALL_SOLDIERINITNODES(curr)
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
					Assert(0);
				if( curr->pDetailedPlacement )
				{ //delete the detailed placement information.
					MemFree( curr->pDetailedPlacement );
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
	FOR_ALL_SOLDIERINITNODES(curr)
	{
		if( curr->pSoldier->ubID == usID )
			return curr;
	}
	return NULL;
}


SOLDIERINITNODE* FindSoldierInitNodeBySoldier(const SOLDIERTYPE* const s)
{
	FOR_ALL_SOLDIERINITNODES(i)
	{
		if (i->pSoldier == s) return i;
	}
	return NULL;
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

//Any killed people that used detailed placement information must prevent that from occurring
//again in the future.  Otherwise, the sniper guy with 99 marksmanship could appear again
//if the map was loaded again!
void EvaluateDeathEffectsToSoldierInitList(const SOLDIERTYPE* const pSoldier)
{
	if( pSoldier->bTeam == MILITIA_TEAM )
		return;
	SOLDIERINITNODE* const curr = FindSoldierInitNodeBySoldier(pSoldier);
	if (curr != NULL)
	{ //Matching soldier found
		if (curr->pDetailedPlacement)
		{ //This soldier used detailed placement information, so we must save the
			//node ID into the temp file which signifies that the

			//RECORD UBNODEID IN TEMP FILE.

			curr->pSoldier = NULL;
			MemFree(curr->pDetailedPlacement);
			curr->pDetailedPlacement = NULL;
		}
	}
}


//For the purpose of keeping track of which soldier belongs to which placement within the game,
//the only way we can do this properly is to save the soldier ID from the list and reconnect the
//soldier pointer whenever we load the game.
void SaveSoldierInitListLinks(HWFILE const hfile)
{
	UINT8 ubSlots = 0;

	//count the number of soldier init nodes...
	CFOR_ALL_SOLDIERINITNODES(curr) ++ubSlots;
	//...and save it.
	FileWrite(hfile, &ubSlots, 1);
	//Now, go through each node, and save just the ubSoldierID, if that soldier is alive.
	FOR_ALL_SOLDIERINITNODES(curr)
	{
		if( curr->pSoldier && !curr->pSoldier->bActive )
		{
			curr->ubSoldierID = 0;
		}
		FileWrite(hfile, &curr->ubNodeID,    1);
		FileWrite(hfile, &curr->ubSoldierID, 1);
	}
}


void LoadSoldierInitListLinks(HWFILE const f)
{
	UINT8 slots;
	FileRead(f, &slots, 1);
	for (UINT8 n = slots; n != 0; --n)
	{
		UINT8 node_id;
		UINT8 soldier_id;
		FileRead(f, &node_id,    1);
		FileRead(f, &soldier_id, 1);

		if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) continue;

		FOR_ALL_SOLDIERINITNODES(curr)
		{
			if (curr->ubNodeID != node_id) continue;

			curr->ubSoldierID = soldier_id;
			TacticalTeamType const* const team = gTacticalStatus.Team;
			if ((team[ENEMY_TEAM].bFirstID <= soldier_id && soldier_id <= team[CREATURE_TEAM].bLastID) ||
					(team[CIV_TEAM  ].bFirstID <= soldier_id && soldier_id <= team[CIV_TEAM     ].bLastID))
			{ // only enemies, creatures and civilians
				curr->pSoldier = &GetMan(soldier_id);
			}
		}
	}
}


void AddSoldierInitListBloodcats()
{
	SECTORINFO *pSector;
	UINT8 ubSectorID;

	if( gbWorldSectorZ )
	{
		return; //no bloodcats underground.
	}

	ubSectorID = (UINT8)SECTOR( gWorldSectorX, gWorldSectorY );
	pSector = &SectorInfo[ ubSectorID ];

	if( !pSector->bBloodCatPlacements )
	{ //This map has no bloodcat placements, so don't waste CPU time.
		return;
	}

	if( pSector->bBloodCatPlacements )
	{ //We don't yet know the number of bloodcat placements in this sector so
		//count them now, and permanently record it.
		INT8 bBloodCatPlacements = 0;
		CFOR_ALL_SOLDIERINITNODES(curr)
		{
			if( curr->pBasicPlacement->bBodyType == BLOODCAT )
			{
				bBloodCatPlacements++;
			}
		}
		if( bBloodCatPlacements != pSector->bBloodCatPlacements &&
				ubSectorID != SEC_I16 && ubSectorID != SEC_N5 )
		{
			#ifdef JA2BETAVERSION
				wchar_t str[200];
				swprintf(str, lengthof(str), L"Table specifies that there are %d bloodcat placements in sector %c%d, but the map actually has %d bloodcat placements. Map value takes precedence. KM,LC:1",
									pSector->bBloodCatPlacements, gWorldSectorY + 'A' - 1, gWorldSectorX, bBloodCatPlacements );
				DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, NULL );
			#endif
			pSector->bBloodCatPlacements = bBloodCatPlacements;
			pSector->bBloodCats = -1;
			if( !bBloodCatPlacements )
			{
				return;
			}
		}
	}
	if( pSector->bBloodCats > 0 )
	{ //Add them to the world now...
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
		CFOR_ALL_SOLDIERINITNODES(curr)
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
	FOR_ALL_SOLDIERINITNODES(curr)
	{
		if ( curr->pDetailedPlacement && curr->pDetailedPlacement->ubProfile == ubProfile )
		{
			return( curr );
		}
	}
	return( NULL );
}

//This is the code that loops through the profiles starting at the RPCs, and adds them using strategic insertion
//information, and not editor placements.  The key flag involved for doing it this way is the gMercProfiles[i].fUseProfileInsertionInfo.
void AddProfilesUsingProfileInsertionData()
{
	INT32 i;
	SOLDIERINITNODE * curr;

	for( i = FIRST_RPC; i < ( PROF_HUMMER ); i++ )
	{
		//Perform various checks to make sure the soldier is actually in the same sector, alive, and so on.
		//More importantly, the flag to use profile insertion data must be set.
		if( gMercProfiles[ i ].sSectorX != gWorldSectorX ||
				gMercProfiles[ i ].sSectorY != gWorldSectorY ||
				gMercProfiles[ i ].bSectorZ != gbWorldSectorZ ||
				gMercProfiles[ i ].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED ||
				gMercProfiles[ i ].ubMiscFlags & PROFILE_MISC_FLAG_EPCACTIVE ||
//			gMercProfiles[ i ].ubMiscFlags2 & PROFILE_MISC_FLAG2_DONT_ADD_TO_SECTOR ||
				!gMercProfiles[ i ].bLife ||
				!gMercProfiles[ i ].fUseProfileInsertionInfo
			)
		{ //Don't add, so skip to the next soldier.
			continue;
		}
		SOLDIERTYPE* pSoldier = FindSoldierByProfileID((UINT8)i);
		if( !pSoldier )
		{ //Create a new soldier, as this one doesn't exist
			SOLDIERCREATE_STRUCT		MercCreateStruct;

			//Set up the create struct so that we can properly create the profile soldier.
			memset( &MercCreateStruct, 0, sizeof( MercCreateStruct ) );
			MercCreateStruct.bTeam						= CIV_TEAM;
			MercCreateStruct.ubProfile				= (UINT8)i;
			MercCreateStruct.sSectorX					= gWorldSectorX;
			MercCreateStruct.sSectorY					= gWorldSectorY;
			MercCreateStruct.bSectorZ					= gbWorldSectorZ;

			pSoldier = TacticalCreateSoldier(MercCreateStruct);
		}
		if ( pSoldier )
		{ //Now, insert the soldier.
			pSoldier->ubStrategicInsertionCode = gMercProfiles[ i ].ubStrategicInsertionCode;
			pSoldier->usStrategicInsertionData = gMercProfiles[ i ].usStrategicInsertionData;
			UpdateMercInSector( pSoldier, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
			// CJC: Note well that unless an error occurs, UpdateMercInSector calls
			// AddSoldierToSector
			// AddSoldierToSector(pSoldier);

			// check action ID values
			if ( gMercProfiles[ i ].ubQuoteRecord )
			{
				pSoldier->ubQuoteRecord = gMercProfiles[ i ].ubQuoteRecord;
				pSoldier->ubQuoteActionID = gMercProfiles[ i ].ubQuoteActionID;
				if ( pSoldier->ubQuoteActionID == QUOTE_ACTION_ID_CHECKFORDEST )
				{
					// gridno will have been changed to destination... so we're there...
					NPCReachedDestination( pSoldier, FALSE );
				}
			}

			// make sure this person's pointer is set properly in the init list
			curr = FindSoldierInitListNodeByProfile( pSoldier->ubProfile );
			if ( curr )
			{
				curr->pSoldier = pSoldier;
				curr->ubSoldierID = pSoldier->ubID;
				// also connect schedules here
				if ( curr->pDetailedPlacement->ubScheduleID != 0 )
				{
					SCHEDULENODE * pSchedule = GetSchedule( curr->pDetailedPlacement->ubScheduleID );
					if ( pSchedule )
					{
						pSchedule->soldier     = pSoldier;
						pSoldier->ubScheduleID = curr->pDetailedPlacement->ubScheduleID;
					}
				}
			}
		}
	}
}

void AddProfilesNotUsingProfileInsertionData()
{
	FOR_ALL_SOLDIERINITNODES(curr)
	{
		if( !curr->pSoldier &&
				curr->pBasicPlacement->bTeam == CIV_TEAM &&
				curr->pDetailedPlacement &&
				curr->pDetailedPlacement->ubProfile != NO_PROFILE &&
				!gMercProfiles[ curr->pDetailedPlacement->ubProfile ].fUseProfileInsertionInfo &&
				gMercProfiles[ curr->pDetailedPlacement->ubProfile ].bLife )
		{
			AddPlacementToWorld( curr );
		}
	}
}


#ifdef JA2BETAVERSION
BOOLEAN ValidateSoldierInitLinks(UINT8 ubCode)
{
	UINT32 uiNumInvalids = 0;
	wchar_t str[512];
	CFOR_ALL_SOLDIERINITNODES(curr)
	{
		if( curr->pSoldier )
		{
			if (curr->pSoldier->ubID < 20 && !curr->pSoldier->bActive)
			{
				uiNumInvalids++;
			}
		}
	}
	if( uiNumInvalids || ubCode == 4)
	{
		switch( ubCode )
		{
			case 1: //loading save
				swprintf(str, lengthof(str), L"Error detected in save file WHILE LOADING.  Please send save and text files associated with save to Kris and Dave."
											 L"  After doing so, go back into the game and immediately resave the game which will fix the problem."
											 L"  This is the bug responsible for mercs disappearing.  Be prepared to answer lots of questions..." );
				DoSaveLoadMessageBox(str, SAVE_LOAD_SCREEN, MSG_BOX_FLAG_OK, ErrorDetectedInSaveCallback);
				break;
			case 2: //saving game
				//swprintf( str, L"Error detected WHILE SAVING file.  Please send save and text files associated with save to Kris and Dave."
				//							 L"  After doing so, go back into the game and try reloading the new save and saving it again which *could* fix the problem."
				//							 L"  This is the bug responsible for mercs disappearing.  Be prepared to answer lots of questions..." );
				//if( guiPreviousOptionScreen == MAP_SCREEN )
				//	DoMapMessageBox( MSG_BOX_BASIC_STYLE, str, MAP_SCREEN, MSG_BOX_FLAG_OK, NULL );
				//else
				//	DoMessageBox( MSG_BOX_BASIC_STYLE, str, GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL );
				break;
			case 3: //entering sector using temp files (before fade in)
				gfDoDialogOnceGameScreenFadesIn = TRUE;
				break;
			case 4: //after fade in
				gfDoDialogOnceGameScreenFadesIn = FALSE;
				swprintf(str, lengthof(str), L"Error detected while entering sector USING TEMP FILES.  Please send previous save and text files associated with save to Kris and Dave."
											 L"  After doing so, go back into the game and saving the game, reloading it, and saving it again *could* fix it."
											 L"  This is the bug responsible for mercs disappearing.  Be prepared to answer lots of questions..." );
				DoMessageBox( MSG_BOX_BASIC_STYLE, str, GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL );
				break;
		}
		return FALSE;
	}
	return TRUE;
}
#endif //betaversion error checking functions


void NewWayOfLoadingEnemySoldierInitListLinks(HWFILE const f)
{
	UINT8 slots;
	FileRead(f, &slots, 1);
	for (UINT8 n = slots; n != 0; --n)
	{
		UINT8 node_id;
		UINT8 soldier_id;
		FileRead(f, &node_id,    1);
		FileRead(f, &soldier_id, 1);

		if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) continue;

		FOR_ALL_SOLDIERINITNODES(curr)
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
	FileRead(f, &slots, 1);
	for (UINT8 n = slots; n != 0; --n)
	{
		UINT8 node_id;
		UINT8 soldier_id;
		FileRead(f, &node_id,    1);
		FileRead(f, &soldier_id, 1);

		if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) continue;

		FOR_ALL_SOLDIERINITNODES(curr)
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


void StripEnemyDetailedPlacementsIfSectorWasPlayerLiberated(void)
{
	if (!gfWorldLoaded || gbWorldSectorZ != 0)
	{ /* No world loaded or underground.  Underground sectors don't matter seeing
		 * enemies (not creatures) never rejuvenate underground. */
		return;
	}

	SECTORINFO* const sector = &SectorInfo[SECTOR(gWorldSectorX, gWorldSectorY)];
	if (sector->uiTimeLastPlayerLiberated == 0)
	{ // The player has never owned the sector.
		return;
	}

	/* The player has owned the sector at one point.  By stripping all of the
	 * detailed placements, only basic placements will remain.  This prevents
	 * tanks and "specially detailed" enemies from coming back. */
	FOR_ALL_SOLDIERINITNODES(curr)
	{
		if (!curr->pDetailedPlacement) continue;

		BASIC_SOLDIERCREATE_STRUCT* const bp = curr->pBasicPlacement;
		if (bp->bTeam != ENEMY_TEAM) continue;

		MemFree(curr->pDetailedPlacement);
		curr->pDetailedPlacement = NULL;
		bp->fDetailedPlacement = FALSE;
		bp->fPriorityExistance = FALSE;
		bp->bBodyType          = -1;
		RandomizeRelativeLevel(&bp->bRelativeAttributeLevel, bp->ubSoldierClass);
		RandomizeRelativeLevel(&bp->bRelativeEquipmentLevel, bp->ubSoldierClass);
	}
}
