#ifndef __QUESTS_H
#define __QUESTS_H

#include "Campaign_Types.h"
#include "Facts.h"
#include "StrategicMap.h"


//  Quest.ubStatus values
#define QUESTNOTSTARTED		0
#define QUESTINPROGRESS		1
#define QUESTDONE		2

#define MAX_QUESTS		30
#define MAX_FACTS		65536
#define NUM_FACTS		500 //If you increase this number, add entries to the fact text list in QuestText.c


#define MAP_ROW_A		1
#define MAP_ROW_B		2
#define MAP_ROW_C		3
#define MAP_ROW_D		4
#define MAP_ROW_E		5
#define MAP_ROW_F		6
#define MAP_ROW_G		7
#define MAP_ROW_H		8
#define MAP_ROW_I		9
#define MAP_ROW_J		10
#define MAP_ROW_K		11
#define MAP_ROW_L		12
#define MAP_ROW_M		13
#define MAP_ROW_N		14
#define MAP_ROW_O		15
#define MAP_ROW_P		16






// --------------------------------------------------------------------------------------
//  Q U E S T     D E F I N E S
// --------------------------------------------------------------------------------------

enum Quests
{
	QUEST_DELIVER_LETTER = 0,
	QUEST_FOOD_ROUTE,
	QUEST_KILL_TERRORISTS,
	QUEST_KINGPIN_IDOL,
	QUEST_KINGPIN_MONEY,
	QUEST_RUNAWAY_JOEY,
	QUEST_RESCUE_MARIA,
	QUEST_CHITZENA_IDOL,
	QUEST_HELD_IN_ALMA,
	QUEST_INTERROGATION,

	QUEST_ARMY_FARM, // 10
	QUEST_FIND_SCIENTIST,
	QUEST_DELIVER_VIDEO_CAMERA,
	QUEST_BLOODCATS,
	QUEST_FIND_HERMIT,
	QUEST_CREATURES,
	QUEST_CHOPPER_PILOT,
	QUEST_ESCORT_SKYRIDER,
	QUEST_FREE_DYNAMO,
	QUEST_ESCORT_TOURISTS,

	QUEST_FREE_CHILDREN,	// 20
	QUEST_LEATHER_SHOP_DREAM,

	QUEST_KILL_DEIDRANNA = 25
};
/*
// The first quest is mandatory. Player must find Miguel and deliver letter to him. The tough
// part is finding him. Miguel is UNDERGROUND in EAST OMERTA. Only by talking to Miguel's right
// hand man will the player be delivered to this hideout. This hideout will also serve as the
// player's HQ and inventory stash.
#define QUEST_DELIVER_LETTER		0


// This quest is triggered by Miguel. Player can liberate route from either SAN MONA or
// Drassen. Result allows player to hire on 2 of Miguel's guys.
#define QUEST_FOOD_ROUTE				1

// terrorist quests: 3 out of possible 6 per game - randomized which 3 and where. The
// Bounty Hunter will either appear in CAMBRIA or ALMA. He'll give a diskette with files
// on three of these terrorists. (we won't give files for all six since it may confuse
// player if ony 3 of them exist). The files will contain photographs. The terrorists will go
// by aliases and the photographs will be hard to tell if the NPC is really a terrorist.
// Bounty Hunter will give reward ($$$ CASH $$$) for proof of each death.
#define QUEST_KILL_TERRORISTS				2
#define QUEST_KILL_IMPOSTER			3
#define QUEST_KILL_TERRORIST3		4
#define QUEST_KILL_TERRORIST4		5
#define QUEST_KILL_TERRORIST5		6
#define QUEST_KILL_TERRORIST6		7

// While there'll always be one arms dealer in SAN MONA, there'll be a second one
// randomized in the outskirts of either: GRUMM, BARLINETTA or ESTONI. Only way player
// will know where to look will be to talk to certain town residents.
#define QUEST_FIND_ARMSDEALER		8


// CAMBRIA has 4 medical doctors, supplies and facilities
#define QUEST_MEDICAL_FACILITY	9		// Cambria


// A hermit will exist in each game, but in a different location. The player can try to
// talk to him, but he won't admit much. If the player threatens him, he gets his pet blood
// cats to attack the player. However, if the player talks to the hermit's brother, the brother
// will give valuable info (a code phrase) to the player that will allow the hermit to talk
// to the player, telling him about using creature blood when in the mines and giving him a
// vile - good enough for 3 mercs.
#define QUEST_FIND_HERMIT				10


// SAN MONA is a tough town. Player will find it tough to get any info out of any residents
// because they won't trust the player's mercs. SAN MONA doesn't have it as tough as any of
// the other towns - the local authorities (Queen's police) is as corrupt as the residents
// and to protect themselves they don't deal with outsiders. All residents will more or less
// let the player know that unless the word comes down from the top dude, no one will speak
// or deal with them. So, the player must seek out the top dude who'll want the player to
// fight his strongest bodyguard (hand to hand or knife).
#define QUEST_SAN_MONA_FIGHT		11



// Need a way to bring a kid giving us something into the picture.


// Micky returns in JA2 but with a new occupation.
#define QUEST_FIND_MICKY				12



#define QUEST_FREE_PRISONERS		13		// Tixa

#define QUEST_FIND_SCIENTIST		14


#define QUEST_KILL_BUGS					15	// Queen bug - randomized location?

//
#define QUEST_ESCAPE_MEDUNA			16

// Arulco farmer in F11 (Juan Erizia) is suspected of harboring a weapons cache in support
// of the Queen's forces. Player finds out about him in Cambria via conversation with a non
// family member (many of Cambria's residents are related/Erizias). So, player can go
// investigate. F11 must be liberated and Juan Erizia must still be alive. We randomize
// between QUEST_WEAPON_FARM and QUEST_SICK_WIFE. Player goes to talk to Juan. If he arrives
// during nighttime, there won't be too many cousins hanging around, but player has to be
// careful
//any attempt to
// investigate will awaken everyone, a battle will ensue.


// Player finds out through conversation in Cambria that Juan suspected of harboring weapons
// cache.

// Juan is very protective of his farm.
// Juan has 3 sons, 4 nephews and a sick wife.
// Juan refuses attempts to inspect his farm unless his brother has arranged it.
// Brother Emile is in Cambria.
// Player can investigate at night as long as he doesn't wake anyone.
// If player has talked to Emile, Juan will give him permission to investigate everywhere
// except his bedroom (citing a sick wife).




#define QUEST_WEAPON_FARM				17

#define QUEST_SICK_WIFE					18


// Queen has removed a national & historic treasure from the ruins of Chitzena. It is
// believed a wealthy supporter of the Queen purchased the item and that it is located in
// his home in Balime. Returning the treasure to Chitzena boosts loyalty points.
#define QUEST_RUIN_ARTIFACT			19


// In Cambria, there is a serial killer on the loose, attacking miners. This will only
// start to happen once player has control over the city & mine. It affects the number
// of residents willing to work in the mines (loyalty basically).
#define QUEST_MURDERER					20

// Find the chopper pilot near Drassen(?) and get him to work for you.
#define QUEST_CHOPPER_PILOT			21

#define QUEST_KILL_DEIDRANNA		29	// Meduna



// --------------------------------------------------------------------------------------
*/


#define PABLOS_STOLEN_DEST_GRIDNO	1
#define LOST_SHIPMENT_GRIDNO		2

#define START_SECTOR_LEAVE_EQUIP_GRIDNO	4868

// NB brothel rooms 88-90 removed because they are the antechamber
#define IN_BROTHEL(room)	(gWorldSector.x == 5 && gWorldSector.y == MAP_ROW_C && (room) >= 91 && (room) <= 119)
#define IN_BROTHEL_GUARD_ROOM( room )	( room == 110 )

#define IN_KINGPIN_HOUSE(room)	(gWorldSector.x == 5 && gWorldSector.y == MAP_ROW_D && (room) >= 30 && (room) <= 39)

#define LOYALTY_LOW_THRESHOLD		30
#define LOYALTY_OK_THRESHOLD		50
#define LOYALTY_HIGH_THRESHOLD		80

extern UINT8 gubQuest[MAX_QUESTS];
extern UINT8 gubFact[NUM_FACTS];

extern void    SetFactTrue(Fact);
extern void    SetFactFalse(Fact);
extern BOOLEAN CheckFact(Fact, UINT8 ubProfileID);

extern void StartQuest(UINT8 ubQuest, const SGPSector& sector);
extern void EndQuest(UINT8 ubQuest, const SGPSector& sector);

extern void InternalStartQuest(UINT8 ubQuest, const SGPSector& sector, BOOLEAN fUpdateHistory);
extern void InternalEndQuest(UINT8 ubQuest, const SGPSector& sector, BOOLEAN fUpdateHistory);


extern void CheckForQuests( UINT32 uiDay );

void InitQuestEngine(void);

void LoadQuestInfoFromSavedGameFile(HWFILE);
void SaveQuestInfoToSavedGameFile(HWFILE);

BOOLEAN NPCInRoom(UINT8 ubProfileID, UINT8 ubRoomID);

#endif
