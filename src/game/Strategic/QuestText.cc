#include "sgp/Types.h"
#include "QuestText.h"
#include "Quests.h"


const wchar_t* const QuestDescText[] =
{
	L"Deliver Letter",
	L"Food Route",
	L"Terrorists",
	L"Kingpin Chalice",
	L"Kingpin Money",
	L"Runaway Joey",
	L"Rescue Maria",
	L"Chitzena Chalice",
	L"Held in Alma",
	L"Interogation",

	L"Hillbilly Problem",								//10
	L"Find Scientist",
	L"Deliver Video Camera",
	L"Blood Cats",
	L"Find Hermit",
	L"Creatures",
	L"Find Chopper Pilot",
	L"Escort SkyRider",
	L"Free Dynamo",
	L"Escort Tourists",


	L"Doreen",									//20
	L"Leather Shop Dream",
	L"Escort Shank",
	L"No 23 Yet",
	L"No 24 Yet",
	L"Kill Deidranna",
	L"No 26 Yet",
	L"No 27 Yet",
	L"No 28 Yet",
	L"No 29 Yet",

};


const wchar_t* const FactDescText[] =
{
	L"Omerta Liberated",
	L"Drassen Liberated",
	L"Sanmona Liberated",
	L"Cambria Liberated",
	L"Alma Liberated",
	L"Grumm Liberated",
	L"Tixa Liberated",
	L"Chitzena Liberated",
	L"Estoni Liberated",
	L"Balime Liberated",

	L"Orta Liberated",					//10
	L"Meduna Liberated",
	L"Pacos approched",
	L"Fatima Read note",
	L"Fatima Walked away from player",
	L"Dimitri (#60) is dead",
	L"Fatima responded to Dimitri's supprise",
	L"Carlo's exclaimed 'no one moves'",
	L"Fatima described note",
	L"Fatima arrives at final dest",

	L"Dimitri said Fatima has proof",		//20
	L"Miguel overheard conversation",
	L"Miguel asked for letter",
	L"Miguel read note",
	L"Ira comment on Miguel reading note",
	L"Rebels are enemies",
	L"Fatima spoken to before given note",
	L"Start Drassen quest",
	L"Miguel offered Ira",
	L"Pacos hurt/Killed",

	L"Pacos is in A10",		//30
	L"Current Sector is safe",
	L"Bobby R Shpmnt in transit",
	L"Bobby R Shpmnt in Drassen",
	L"33 is TRUE and it arrived within 2 hours",
	L"33 is TRUE 34 is false more then 2 hours",
	L"Player has realized part of shipment is missing",
	L"36 is TRUE and Pablo was injured by player",
	L"Pablo admitted theft",
	L"Pablo returned goods, set 37 false",

	L"Miguel will join team",		//40
	L"Gave some cash to Pablo",
	L"Skyrider is currently under escort",
	L"Skyrider is close to his chopper in Drassen",
	L"Skyrider explained deal",
	L"Player has clicked on Heli in Mapscreen at least once",
	L"NPC is owed money",
	L"Npc is wounded",
	L"Npc was wounded by Player",
	L"Father J.Walker was told of food shortage",

	L"Ira is not in sector",		//50
	L"Ira is doing the talking",
	L"Food quest over",
	L"Pablo stole something from last shpmnt",
	L"Last shipment crashed",
	L"Last shipment went to wrong airport",
	L"24 hours elapsed since notified that shpment went to wrong airport",
	L"Lost package arrived with damaged goods. 56 to False",
	L"Lost package is lost permanently. Turn 56 False",
	L"Next package can (random) be lost",

	L"Next package can(random) be delayed",		//60
	L"Package is medium sized",
	L"Package is largesized",
	L"Doreen has conscience",
	L"Player Spoke to Gordon",
	L"Ira is still npc and in A10-2(hasnt joined)",
	L"Dynamo asked for first aid",
	L"Dynamo can be recruited",
	L"Npc is bleeding",
	L"Shank wnts to join",

	L"NPC is bleeding",		//70
	L"Player Team has head & Carmen in San Mona",
	L"Player Team has head & Carmen in Cambria",
	L"Player Team has head & Carmen in Drassen",
	L"Father is drunk",
	L"Player has wounded mercs within 8 tiles of NPC",
	L"1 & only 1 merc wounded within 8 tiles of NPC",
	L"More then 1 wounded merc within 8 tiles of NPC",
	L"Brenda is in the store ",
	L"Brenda is Dead",

	L"Brenda is at home",		//80
	L"NPC is an enemy",
	L"Speaker Strength >= 84 and < 3 males present",
	L"Speaker Strength >= 84 and at least 3 males present",
	L"Hans lets ou see Tony",
	L"Hans is standing on 13523",
	L"Tony isnt available Today",
	L"Female is speaking to NPC",
	L"Player has enjoyed the Brothel",
	L"Carla is available",

	L"Cindy is available",		//90
	L"Bambi is available",
	L"No girls is available",
	L"Player waited for girls",
	L"Player paid right amount of money",
	L"Mercs walked by goon",
	L"More thean 1 merc present within 3 tiles of NPC",
	L"At least 1 merc present withing 3 tiles of NPC",
	L"Kingping expectingh visit from player",
	L"Darren expecting money from player",

	L"Player within 5 tiles and NPC is visible",			//	100
	L"Carmen is in San Mona",
	L"Player Spoke to Carmen",
	L"KingPin knows about stolen money",
	L"Player gave money back to KingPin",
	L"Frank was given the money ( not to buy booze )",
	L"Player was told about KingPin watching fights",
	L"Past club closing time and Darren warned Player. (reset every day)",
	L"Joey is EPC",
	L"Joey is in C5",

	L"Joey is within 5 tiles of Martha(109) in sector G8",		//110
	L"Joey is Dead!",
	L"At least one player merc within 5 tiles of Martha",
	L"Spike is occuping tile 9817",
	L"Angel offered vest",
	L"Angel sold vest",
	L"Maria is EPC",
	L"Maria is EPC and inside leather Shop",
	L"Player wants to buy vest",
	L"Maria rescue was noticed by KingPin goons and Kingpin now enemy",

	L"Angel left deed on counter",		//120
	L"Maria quest over",
	L"Player bandaged NPC today",
	L"Doreen revealed allegiance to Queen",
	L"Pablo should not steal from player",
	L"Player shipment arrived but loyalty to low, so it left",
	L"Helicopter is in working condition",
	L"Player is giving amount of money >= $1000",
	L"Player is giving amount less than $1000",
	L"Waldo agreed to fix helicopter( heli is damaged )",

	L"Helicopter was destroyed",		//130
	L"Waldo told us about heli pilot",
	L"Father told us about Deidranna killing sick people",
	L"Father told us about Chivaldori family",
	L"Father told us about creatures",
	L"Loyalty is OK",
	L"Loyalty is Low",
	L"Loyalty is High",
	L"Player doing poorly",
	L"Player gave valid head to Carmen",

	L"Current sector is G9(Cambria)",		//140
	L"Current sector is C5(SanMona)",
	L"Current sector is C13(Drassen",
	L"Carmen has at least $10,000 on him",
	L"Player has Slay on team for over 48 hours",
	L"Carmen is suspicous about slay",
	L"Slay is in current sector",
	L"Carmen gave us final warning",
	L"Vince has explained that he has to charge",
	L"Vince is expecting cash (reset everyday)",

	L"Player stole some medical supplies once",		//150
	L"Player stole some medical supplies again",
	L"Vince can be recruited",
	L"Vince is currently doctoring",
	L"Vince was recruited",
	L"Slay offered deal",
	L"All terrorists killed",
	L"",
	L"Maria left in wrong sector",
	L"Skyrider left in wrong sector",

	L"Joey left in wrong sector",		//160
	L"John left in wrong sector",
	L"Mary left in wrong sector",
	L"Walter was bribed",
	L"Shank(67) is part of squad but not speaker",
	L"Maddog spoken to",
	L"Jake told us about shank",
	L"Shank(67) is not in secotr",
	L"Bloodcat quest on for more than 2 days",
	L"Effective threat made to Armand",

	L"Queen is DEAD!",												//170
	L"Speaker is with Aim or Aim person on squad within 10 tiles",
	L"Current mine is empty",
	L"Current mine is running out",
	L"Loyalty low in affiliated town (low mine production)",
	L"Creatures invaded current mine",
	L"Player LOST current mine",
	L"Current mine is at FULL production",
	L"Dynamo(66) is Speaker or within 10 tiles of speaker",
	L"Fred told us about creatures",

	L"Matt told us about creatures",				//180
	L"Oswald told us about creatures",
	L"Calvin told us about creatures",
	L"Carl told us about creatures",
	L"Chalice stolen from museam",
	L"John(118) is EPC",
	L"Mary(119) and John (118) are EPC's",
	L"Mary(119) is alive",
	L"Mary(119)is EPC",
	L"Mary(119) is bleeding",

	L"John(118) is alive",							//190
	L"John(118) is bleeding",
	L"John or Mary close to airport in Drassen(B13)",
	L"Mary is Dead",
	L"Miners placed",
	L"Krott planning to shoot player",
	L"Madlab explained his situation",
	L"Madlab expecting a firearm",
	L"Madlab expecting a video camera.",
	L"Item condition is < 70 ",

	L"Madlab complained about bad firearm.",					//200
	L"Madlab complained about bad video camera.",
	L"Robot is ready to go!",
	L"First robot destroyed.",
	L"Madlab given a good camera.",
	L"Robot is ready to go a second time!",
	L"Second robot destroyed.",
	L"Mines explained to player.",
	L"Dynamo (#66) is in sector J9.",
	L"Dynamo (#66) is alive.",

	L"One PC hasn't fought, but is able, and less than 3 fights have occured",							//210
	L"Player receiving mine income from Drassen, Cambria, Alma & Chitzena",
	L"Player has been to K4_b1",
	L"Brewster got to talk while Warden was alive",
	L"Warden (#103) is dead.",
	L"Ernest gave us the guns",
	L"This is the first bartender",
	L"This is the second bartender",
	L"This is the third bartender",
	L"This is the fourth bartender",


	L"Manny is a bartender.",									//220
	L"Nothing is repaired yet (some stuff being worked on, nothing to give player right now)",
	L"Player made purchase from Howard (#125)",
	L"Dave sold vehicle",
	L"Dave's vehicle ready",
	L"Dave expecting cash for car",
	L"Dave has gas. (randomized daily)",
	L"Vehicle is present",
	L"First battle won by player",
	L"Robot recruited and moved",

	L"No club fighting allowed",							//230
	L"Player already fought 3 fights today",
	L"Hans mentioned Joey",
	L"Player is doing better than 50% (Alex's function)",
	L"Player is doing very well (better than 80%)",
	L"Father is drunk and sci-fi option is on",
	L"Micky (#96) is drunk",
	L"Player has attempted to force their way into brothel",
	L"Rat effectively threatened 3 times",
	L"Player paid for two people to enter brothel",

	L"",						//240
	L"",
	L"Player owns 2 towns including omerta",
	L"Player owns 3 towns including omerta",//							243
	L"Player owns 4 towns including omerta",//							244
	L"",
	L"",
	L"",
	L"Fact male speaking female present",
	L"Fact hicks married player merc",//													249

	L"Fact museum open",//																				250
	L"Fact brothel open",//																			251
	L"Fact club open",//																					252
	L"Fact first battle fought",//																253
	L"Fact first battle being fought",//													254
	L"Fact kingpin introduced self",//														255
	L"Fact kingpin not in office",//															256
	L"Fact dont owe kingpin money",//														257
	L"Fact pc marrying daryl is flo",//													258
	L"",

	L"",					//260
	L"Fact npc cowering",																			//	261,
	L"",
	L"",
	L"Fact top and bottom levels cleared",
	L"Fact top level cleared",//																	265
	L"Fact bottom level cleared",//															266
	L"Fact need to speak nicely",//															267
	L"Fact attached item before",//															268
	L"Fact skyrider ever escorted",//														269

	L"Fact npc not under fire",//																270
	L"Fact willis heard about joey rescue",//										271
	L"Fact willis gives discount",//															272
	L"Fact hillbillies killed",//																273
	L"Fact keith out of business",	//														274
	L"Fact mike available to army",//														275
	L"Fact kingpin can send assassins",//												276
	L"Fact estoni refuelling possible",//                        277
	L"Fact museum alarm went off",//															278
	L"",

	L"Fact maddog is speaker", 																	//280,
	L"",
	L"Fact angel mentioned deed", 															// 282,
	L"Fact iggy available to army",//														283
	L"Fact pc has conrads recruit opinion",//										284
	L"",
	L"",
	L"",
	L"",
	L"Fact npc hostile or pissed off",													//289,

	L"",					//290
	L"Fact tony in building",																		//291,
	L"Fact shank speaking",	//																		292,
	L"Fact doreen alive",//																			293
	L"Fact waldo alive",//																				294
	L"Fact perko alive",//																				295
	L"Fact tony alive",//																				296
	L"",
	L"Fact vince alive",// 																				298,
	L"Fact jenny alive",//																				299

	L"",					//300
	L"",
	L"Fact arnold alive",//																				302,
	L"",
	L"Fact rocket rifle exists",// 																304,
	L"",
	L"",
	L"",
	L"",
	L"",

	L"",					//310
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//320

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//330

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//340

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//350

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//360

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//370

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//380

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//390

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//400

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//410

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//420

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//430

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//440

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//450

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//460

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//470

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//480

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//490

	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",					//500

};
