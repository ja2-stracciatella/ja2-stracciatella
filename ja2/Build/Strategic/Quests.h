#ifndef __QUESTS_H
#define __QUESTS_H

#include "FileMan.h"
#include "StrategicMap.h"

//  Quest.ubStatus values
#define QUESTNOTSTARTED					0
#define	QUESTINPROGRESS					1
#define QUESTDONE								2

#define MAX_QUESTS							30
#define MAX_FACTS								65536
#define	NUM_FACTS								500			//If you increase this number, add entries to the fact text list in QuestText.c


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

enum
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
} Quests;
/*
// The first quest is mandatory. Player must find Miguel and deliver letter to him. The tough
// part is finding him. Miguel is UNDERGROUND in EAST OMERTA. Only by talking to Miguel's right
// hand man will the player be delivered to this hideout. This hideout will also serve as the
// player's HQ and inventory stash.
#define	QUEST_DELIVER_LETTER		0


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
#define	QUEST_KILL_TERRORIST6		7

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

#define	QUEST_KILL_DEIDRANNA		29	// Meduna



// --------------------------------------------------------------------------------------
*/


// FACTS

enum
{
	// city liberations
	FACT_OMERTA_LIBERATED	=																		0,
	FACT_DRASSEN_LIBERATED,	//																1
	FACT_SANMONA_LIBERATED,	//																2
	FACT_CAMBRIA_LIBERATED,	//																3
	FACT_ALMA_LIBERATED,		//																4
	FACT_GRUMM_LIBERATED,		//																5
	FACT_TIXA_LIBERATED,		//																6
	FACT_CHITZENA_LIBERATED,//																7
	FACT_ESTONI_LIBERATED,	//																8
	FACT_BALIME_LIBERATED,	//																9
	FACT_ORTA_LIBERATED,		//																10
	FACT_MEDUNA_LIBERATED,	//																11

	// quest stuff
	FACT_MIGUEL_FOUND,			//																12
	FACT_LETTER_DELIVERED,	//																13
	FACT_FOOD_ROUTE_EXISTS,	//																14
	FACT_DIMITRI_DEAD,		  //																15

	FACT_MIGUEL_READ_LETTER =																	23,

// rebels do not trust player
	FACT_REBELS_HATE_PLAYER =																	25,

	FACT_PACOS_KILLED =																				29,

	FACT_CURRENT_SECTOR_IS_SAFE =															31,
	FACT_BOBBYRAY_SHIPMENT_IN_TRANSIT,	//										32
	FACT_NEW_BOBBYRAY_SHIPMENT_WAITING, //										33
	FACT_REALLY_NEW_BOBBYRAY_SHIPMENT_WAITING,//							34
	FACT_LARGE_SIZED_OLD_SHIPMENT_WAITING,//									35
	FACT_PLAYER_FOUND_ITEMS_MISSING,//												36
	FACT_PABLO_PUNISHED_BY_PLAYER,//													37

	FACT_PABLO_RETURNED_GOODS =																39,

	FACT_PABLOS_BRIBED =																			41,
	FACT_ESCORTING_SKYRIDER,//																42
	FACT_SKYRIDER_CLOSE_TO_CHOPPER,//													43

	FACT_SKYRIDER_USED_IN_MAPSCREEN =													45,
	FACT_NPC_OWED_MONEY,//																		46
	FACT_NPC_WOUNDED,//																				47
	FACT_NPC_WOUNDED_BY_PLAYER,//															48

	FACT_IRA_NOT_PRESENT = 																		50,
	FACT_IRA_TALKING,//																				51
	FACT_FOOD_QUEST_OVER,//																		52
	FACT_PABLOS_STOLE_FROM_LATEST_SHIPMENT,//									53
	FACT_LAST_SHIPMENT_CRASHED,//															54
	FACT_LAST_SHIPMENT_WENT_TO_WRONG_AIRPORT,//								55
	FACT_SHIPMENT_DELAYED_24_HOURS,//													56
	FACT_PACKAGE_DAMAGED,//																		57
	FACT_PACKAGE_LOST_PERMANENTLY,//													58
	FACT_NEXT_PACKAGE_CAN_BE_LOST,//													59
	FACT_NEXT_PACKAGE_CAN_BE_DELAYED,//												60
	FACT_MEDIUM_SIZED_SHIPMENT_WAITING,//											61
	FACT_LARGE_SIZED_SHIPMENT_WAITING,//											62
	FACT_DOREEN_HAD_CHANGE_OF_HEART,//												63

	FACT_IRA_UNHIRED_AND_ALIVE =															65,

	FACT_NPC_BLEEDING =																				68,

	FACT_NPC_BLEEDING_BUT_OKAY =															70,
	FACT_PLAYER_HAS_HEAD_AND_CARMEN_IN_SAN_MONA,//						71
	FACT_PLAYER_HAS_HEAD_AND_CARMEN_IN_CAMBRIA,//							72
	FACT_PLAYER_HAS_HEAD_AND_CARMEN_IN_DRASSEN,//							73
	FACT_FATHER_DRUNK,//																			74
	FACT_WOUNDED_MERCS_NEARBY,//															75
	FACT_ONE_WOUNDED_MERC_NEARBY,//														76
	FACT_MULTIPLE_WOUNDED_MERCS_NEARBY,//											77
	FACT_BRENDA_IN_STORE_AND_ALIVE,//													78
	FACT_BRENDA_DEAD,//																				79

	FACT_NPC_IS_ENEMY =																				81,
	FACT_PC_STRONG_AND_LESS_THAN_3_MALES_PRESENT,//						82
	FACT_PC_STRONG_AND_3_PLUS_MALES_PRESENT,//								83

	FACT_HANS_AT_SPOT =																				85,
	FACT_TONY_NOT_AVAILABLE,//																86
	FACT_FEMALE_SPEAKING_TO_NPC,//														87
	FACT_PLAYER_USED_BROTHEL,//																88
	FACT_CARLA_AVAILABLE,//																		89
	FACT_CINDY_AVAILABLE,//																		90
	FACT_BAMBI_AVAILABLE,//																		91
	FACT_NO_GIRLS_AVAILABLE,//																92
	FACT_PLAYER_WAITED_FOR_GIRL,//														93
	FACT_PLAYER_PAID_RIGHT_AMOUNT,//													94
	FACT_PLAYER_PASSED_GOON,//																95
	FACT_MULTIPLE_MERCS_CLOSE ,//															96,
	FACT_SOME_MERCS_CLOSE,//																	97

	FACT_DARREN_EXPECTING_MONEY =															99,
	FACT_PC_NEAR,//																						100
	FACT_CARMEN_IN_C5,//																			101
	FACT_CARMEN_EXPLAINED_DEAL,//															102
	FACT_KINGPIN_KNOWS_MONEY_GONE,//													103
	FACT_PLAYER_REPAID_KINGPIN,//															104
	FACT_FRANK_HAS_BEEN_BRIBED,//															105

	FACT_PAST_CLUB_CLOSING_AND_PLAYER_WARNED =								107,
	FACT_JOEY_ESCORTED,//																			108
	FACT_JOEY_IN_C5,//																				109
	FACT_JOEY_NEAR_MARTHA,//																	110
	FACT_JOEY_DEAD,//																					111
	FACT_MERC_NEAR_MARTHA,//																	112
	FACT_SPIKE_AT_DOOR,//																			113

	FACT_ANGEL_SOLD_VEST =																		115,
	FACT_MARIA_ESCORTED,//																		116
	FACT_MARIA_ESCORTED_AT_LEATHER_SHOP,//										117
	FACT_PLAYER_WANTS_TO_BUY_LEATHER_VEST,//									118
	FACT_MARIA_ESCAPE_NOTICED,//															119
	FACT_ANGEL_LEFT_DEED,//																		120

	FACT_NPC_BANDAGED_TODAY =																	122,

	FACT_PABLO_WONT_STEAL = 																	124,
	FACT_AGENTS_PREVENTED_SHIPMENT,//													125

	FACT_LARGE_AMOUNT_OF_MONEY =															127,
	FACT_SMALL_AMOUNT_OF_MONEY,//															128

	FACT_LOYALTY_OKAY =																				135,
	FACT_LOYALTY_LOW,//																				136
	FACT_LOYALTY_HIGH,//																			137
	FACT_PLAYER_DOING_POORLY,//																138

	FACT_CURRENT_SECTOR_G9 =																	140,
	FACT_CURRENT_SECTOR_C5,//																	141
	FACT_CURRENT_SECTOR_C13,//																142
	FACT_CARMEN_HAS_TEN_THOUSAND,//														143
	FACT_SLAY_HIRED_AND_WORKED_FOR_48_HOURS,//								144

	FACT_SLAY_IN_SECTOR =																			146,

	FACT_VINCE_EXPLAINED_HAS_TO_CHARGE =											148,
	FACT_VINCE_EXPECTING_MONEY,//															149
	FACT_PLAYER_STOLE_MEDICAL_SUPPLIES,//											150
	FACT_PLAYER_STOLE_MEDICAL_SUPPLIES_AGAIN,//								151
	FACT_VINCE_RECRUITABLE,//																	152

	FACT_ALL_TERRORISTS_KILLED =															156,
	FACT_ELGIN_ALIVE,//																				157

	FACT_SHANK_IN_SQUAD_BUT_NOT_SPEAKING =										164,

	FACT_SHANK_NOT_IN_SECTOR =																167,
	FACT_BLOODCAT_QUEST_STARTED_TWO_DAYS_AGO,//								168

	FACT_QUEEN_DEAD =																					170,

	FACT_SPEAKER_AIM_OR_AIM_NEARBY =													171,
	FACT_MINE_EMPTY,//																				172
	FACT_MINE_RUNNING_OUT,//																	173
	FACT_MINE_PRODUCING_BUT_LOYALTY_LOW,//										174
	FACT_CREATURES_IN_MINE,//																	175
	FACT_PLAYER_LOST_MINE,//																	176
	FACT_MINE_AT_FULL_PRODUCTION,//														177
	FACT_DYNAMO_SPEAKING_OR_NEARBY,//													178

	FACT_CHALICE_STOLEN =																			184,
	FACT_JOHN_EPC,//																					185
	FACT_JOHN_AND_MARY_EPCS,//																186
	FACT_MARY_ALIVE,//																				187
	FACT_MARY_EPC,//																					188
	FACT_MARY_BLEEDING,//																			189
	FACT_JOHN_ALIVE,//																				190
	FACT_JOHN_BLEEDING,//																			191
	FACT_MARY_OR_JOHN_ARRIVED,//															192
	FACT_MARY_DEAD,//																					193
	FACT_MINERS_PLACED,//																			194
	FACT_KROTT_GOT_ANSWER_NO,//																195
	FACT_MADLAB_EXPECTING_FIREARM =														197,
	FACT_MADLAB_EXPECTING_VIDEO_CAMERA,//											198
	FACT_ITEM_POOR_CONDITION,//																199

	FACT_ROBOT_READY =																				202,
	FACT_FIRST_ROBOT_DESTROYED,//															203
	FACT_MADLAB_HAS_GOOD_CAMERA,//														204
	FACT_ROBOT_READY_SECOND_TIME,//														205
	FACT_SECOND_ROBOT_DESTROYED,//														206

	FACT_DYNAMO_IN_J9 = 																			208,
	FACT_DYNAMO_ALIVE,//																			209
	FACT_ANOTHER_FIGHT_POSSIBLE,//														210
	FACT_RECEIVING_INCOME_FROM_DCAC,//												211
	FACT_PLAYER_BEEN_TO_K4,//																	212

	FACT_WARDEN_DEAD =																				214,

	FACT_FIRST_BARTENDER =																		216,
	FACT_SECOND_BARTENDER,//																	217
	FACT_THIRD_BARTENDER,//																		218
	FACT_FOURTH_BARTENDER,//																	219
	FACT_MANNY_IS_BARTENDER,//																220
	FACT_NOTHING_REPAIRED_YET,//															221,

	FACT_OK_USE_HUMMER =																			224,

	FACT_DAVE_HAS_GAS =																				226,
	FACT_VEHICLE_PRESENT,//																		227
	FACT_FIRST_BATTLE_WON,//																	228
	FACT_ROBOT_RECRUITED_AND_MOVED,//													229
	FACT_NO_CLUB_FIGHTING_ALLOWED,//													230
	FACT_PLAYER_FOUGHT_THREE_TIMES_TODAY,//										231
	FACT_PLAYER_SPOKE_TO_DRASSEN_MINER,//											232
	FACT_PLAYER_DOING_WELL,//																	233
	FACT_PLAYER_DOING_VERY_WELL,//														234
	FACT_FATHER_DRUNK_AND_SCIFI_OPTION_ON,//									235
	FACT_MICKY_DRUNK,//																				236
	FACT_PLAYER_FORCED_WAY_INTO_BROTHEL,//										237

	FACT_PLAYER_PAID_FOR_TWO_IN_BROTHEL =											239,

	FACT_PLAYER_OWNS_2_TOWNS_INCLUDING_OMERTA =								242,
	FACT_PLAYER_OWNS_3_TOWNS_INCLUDING_OMERTA,//							243
	FACT_PLAYER_OWNS_4_TOWNS_INCLUDING_OMERTA,//							244

	FACT_MALE_SPEAKING_FEMALE_PRESENT =												248,
	FACT_HICKS_MARRIED_PLAYER_MERC,//													249
	FACT_MUSEUM_OPEN,//																				250
	FACT_BROTHEL_OPEN,//																			251
	FACT_CLUB_OPEN,//																					252
	FACT_FIRST_BATTLE_FOUGHT,//																253
	FACT_FIRST_BATTLE_BEING_FOUGHT,//													254
	FACT_KINGPIN_INTRODUCED_SELF,//														255
	FACT_KINGPIN_NOT_IN_OFFICE,//															256
	FACT_DONT_OWE_KINGPIN_MONEY,//														257
	FACT_PC_MARRYING_DARYL_IS_FLO,//													258
	FACT_I16_BLOODCATS_KILLED,//															259

	FACT_NPC_COWERING	=																				261,

	FACT_TOP_AND_BOTTOM_LEVELS_CLEARED =											264,
	FACT_TOP_LEVEL_CLEARED,//																	265
	FACT_BOTTOM_LEVEL_CLEARED,//															266
	FACT_NEED_TO_SAY_SOMETHING,//															267
	FACT_ATTACHED_ITEM_BEFORE,//															268
	FACT_SKYRIDER_EVER_ESCORTED,//														269
	FACT_NPC_NOT_UNDER_FIRE,//																270
	FACT_WILLIS_HEARD_ABOUT_JOEY_RESCUE,//										271
	FACT_WILLIS_GIVES_DISCOUNT,//															272
	FACT_HILLBILLIES_KILLED,//																273
	FACT_KEITH_OUT_OF_BUSINESS,	//														274
	FACT_MIKE_AVAILABLE_TO_ARMY,//														275
	FACT_KINGPIN_CAN_SEND_ASSASSINS,//												276
	FACT_ESTONI_REFUELLING_POSSIBLE,//                        277
	FACT_MUSEUM_ALARM_WENT_OFF,//															278

	FACT_MADDOG_IS_SPEAKER =																	280,

	FACT_ANGEL_MENTIONED_DEED =																282,
	FACT_IGGY_AVAILABLE_TO_ARMY,//														283
	FACT_PC_HAS_CONRADS_RECRUIT_OPINION,//										284

	FACT_NPC_HOSTILE_OR_PISSED_OFF =													289,

	FACT_TONY_IN_BUILDING =																		291,
	FACT_SHANK_SPEAKING =																			292,
	FACT_PABLO_ALIVE,//																				293
	FACT_DOREEN_ALIVE,//																			294
	FACT_WALDO_ALIVE,//																				295
	FACT_PERKO_ALIVE,//																				296
	FACT_TONY_ALIVE,//																				297

	FACT_VINCE_ALIVE = 																				299,
	FACT_JENNY_ALIVE,//																				300

	FACT_ARNOLD_ALIVE =																				303,
	FACT_ROCKET_RIFLE_EXISTS,//																304,
	FACT_24_HOURS_SINCE_JOEY_RESCUED,//												305
	FACT_24_HOURS_SINCE_DOCTOR_TALKED_TO,									//	306
	FACT_OK_USE_ICECREAM,																	//	307
	FACT_KINGPIN_DEAD,//																			308

	FACT_KIDS_ARE_FREE =																			318,
	FACT_PLAYER_IN_SAME_ROOM,//																319

	FACT_PLAYER_IN_CONTROLLED_DRASSEN_MINE =									324,
	FACT_PLAYER_SPOKE_TO_CAMBRIA_MINER,//											325
	FACT_PLAYER_IN_CONTROLLED_CAMBRIA_MINE,//									326
	FACT_PLAYER_SPOKE_TO_CHITZENA_MINER,//										327
	FACT_PLAYER_IN_CONTROLLED_CHITZENA_MINE,//								328
	FACT_PLAYER_SPOKE_TO_ALMA_MINER,//												329
	FACT_PLAYER_IN_CONTROLLED_ALMA_MINE,//										330
	FACT_PLAYER_SPOKE_TO_GRUMM_MINER,//												331
	FACT_PLAYER_IN_CONTROLLED_GRUMM_MINE,//										332

	FACT_LARRY_CHANGED =																			334,
	FACT_PLAYER_KNOWS_ABOUT_BLOODCAT_LAIR,//									335
	FACT_HOSPITAL_FREEBIE_DECISION_MADE,//										336
	FACT_ENOUGH_LOYALTY_TO_TRAIN_MILITIA,//										337
	FACT_WALKER_AT_BAR,//																			338

	FACT_JOEY_ALIVE =																					340,
	FACT_UNPROPOSITIONED_FEMALE_SPEAKING_TO_NPC,//						341
	FACT_84_AND_85_TRUE,//																		342

	FACT_KINGPIN_WILL_LEARN_OF_MONEY_GONE = 									350,

	FACT_SKYRIDER_IN_B15 =																		354,
	FACT_SKYRIDER_IN_C16,//																		355
	FACT_SKYRIDER_IN_E14,//																		356
	FACT_SKYRIDER_IN_D12,//																		357
	FACT_SKYRIDER_HINT_GIVEN,//																358
	FACT_KINGPIN_IS_ENEMY,//																	359
	FACT_BRENDA_PATIENCE_TIMER_EXPIRED,//											360

	FACT_DYNAMO_NOT_SPEAKER =																	362,

	FACT_PABLO_BRIBED =																				365,

	FACT_CONRAD_SHOULD_GO =																		367,
	FACT_PLAYER_KILLED_BOXERS =																368,
} Facts;
//**  END FACTS *************************

#define		BOBBYR_SHIPPING_DEST_SECTOR_X				13
#define		BOBBYR_SHIPPING_DEST_SECTOR_Y				2
#define		BOBBYR_SHIPPING_DEST_SECTOR_Z				0
#define		BOBBYR_SHIPPING_DEST_GRIDNO					10112
#define		PABLOS_STOLEN_DEST_GRIDNO						1
#define		LOST_SHIPMENT_GRIDNO								2

// omerta positions
#define OMERTA_LEAVE_EQUIP_SECTOR_X		9
#define OMERTA_LEAVE_EQUIP_SECTOR_Y		1
#define OMERTA_LEAVE_EQUIP_SECTOR_Z		0
#define OMERTA_LEAVE_EQUIP_GRIDNO			4868

// NB brothel rooms 88-90 removed because they are the antechamber
#define IN_BROTHEL( room ) (gWorldSectorX == 5 && gWorldSectorY == MAP_ROW_C && (room) >= 91 && (room) <= 119)
#define IN_BROTHEL_GUARD_ROOM( room ) ( room == 110 )

#define IN_KINGPIN_HOUSE( room ) ( gWorldSectorX == 5 && gWorldSectorY == MAP_ROW_D && (room) >= 30 && (room) <= 39 )

#define LOYALTY_LOW_THRESHOLD		30
#define LOYALTY_OK_THRESHOLD		50
#define LOYALTY_HIGH_THRESHOLD	80

extern UINT8 gubQuest[MAX_QUESTS];
extern UINT8 gubFact[NUM_FACTS];

extern void SetFactTrue( UINT16 usFact );
extern void SetFactFalse( UINT16 usFact );
extern BOOLEAN CheckFact( UINT16 usFact, UINT8 ubProfileID );

extern void StartQuest( UINT8 ubQuest, INT16 sSectorX, INT16 sSectorY );
extern void EndQuest( UINT8 ubQuest, INT16 sSectorX, INT16 sSectorY );

extern void InternalStartQuest( UINT8 ubQuest, INT16 sSectorX, INT16 sSectorY, BOOLEAN fUpdateHistory );
extern void InternalEndQuest( UINT8 ubQuest, INT16 sSectorX, INT16 sSectorY, BOOLEAN fUpdateHistory );


extern void CheckForQuests( UINT32 uiDay );

void InitQuestEngine();

BOOLEAN LoadQuestInfoFromSavedGameFile( HWFILE hFile );
BOOLEAN SaveQuestInfoToSavedGameFile( HWFILE hFile );


#endif
