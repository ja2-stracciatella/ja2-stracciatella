#include "QuestText.h"


const ST::string QuestDescText[] =
{
	"Deliver Letter",
	"Food Route",
	"Terrorists",
	"Kingpin Chalice",
	"Kingpin Money",
	"Runaway Joey",
	"Rescue Maria",
	"Chitzena Chalice",
	"Held in Alma",
	"Interogation",

	"Hillbilly Problem",								//10
	"Find Scientist",
	"Deliver Video Camera",
	"Blood Cats",
	"Find Hermit",
	"Creatures",
	"Find Chopper Pilot",
	"Escort SkyRider",
	"Free Dynamo",
	"Escort Tourists",


	"Doreen",									//20
	"Leather Shop Dream",
	"Escort Shank",
	"No 23 Yet",
	"No 24 Yet",
	"Kill Deidranna",
	"No 26 Yet",
	"No 27 Yet",
	"No 28 Yet",
	"No 29 Yet",

};


const ST::string FactDescText[] =
{
	"Omerta Liberated",
	"Drassen Liberated",
	"Sanmona Liberated",
	"Cambria Liberated",
	"Alma Liberated",
	"Grumm Liberated",
	"Tixa Liberated",
	"Chitzena Liberated",
	"Estoni Liberated",
	"Balime Liberated",

	"Orta Liberated",					//10
	"Meduna Liberated",
	"Pacos approched",
	"Fatima Read note",
	"Fatima Walked away from player",
	"Dimitri (#60) is dead",
	"Fatima responded to Dimitri's supprise",
	"Carlo's exclaimed 'no one moves'",
	"Fatima described note",
	"Fatima arrives at final dest",

	"Dimitri said Fatima has proof",		//20
	"Miguel overheard conversation",
	"Miguel asked for letter",
	"Miguel read note",
	"Ira comment on Miguel reading note",
	"Rebels are enemies",
	"Fatima spoken to before given note",
	"Start Drassen quest",
	"Miguel offered Ira",
	"Pacos hurt/Killed",

	"Pacos is in A10",		//30
	"Current Sector is safe",
	"Bobby R Shpmnt in transit",
	"Bobby R Shpmnt in Drassen",
	"33 is TRUE and it arrived within 2 hours",
	"33 is TRUE 34 is false more then 2 hours",
	"Player has realized part of shipment is missing",
	"36 is TRUE and Pablo was injured by player",
	"Pablo admitted theft",
	"Pablo returned goods, set 37 false",

	"Miguel will join team",		//40
	"Gave some cash to Pablo",
	"Skyrider is currently under escort",
	"Skyrider is close to his chopper in Drassen",
	"Skyrider explained deal",
	"Player has clicked on Heli in Mapscreen at least once",
	"NPC is owed money",
	"Npc is wounded",
	"Npc was wounded by Player",
	"Father J.Walker was told of food shortage",

	"Ira is not in sector",		//50
	"Ira is doing the talking",
	"Food quest over",
	"Pablo stole something from last shpmnt",
	"Last shipment crashed",
	"Last shipment went to wrong airport",
	"24 hours elapsed since notified that shpment went to wrong airport",
	"Lost package arrived with damaged goods. 56 to False",
	"Lost package is lost permanently. Turn 56 False",
	"Next package can (random) be lost",

	"Next package can(random) be delayed",		//60
	"Package is medium sized",
	"Package is largesized",
	"Doreen has conscience",
	"Player Spoke to Gordon",
	"Ira is still npc and in A10-2(hasnt joined)",
	"Dynamo asked for first aid",
	"Dynamo can be recruited",
	"Npc is bleeding",
	"Shank wnts to join",

	"NPC is bleeding",		//70
	"Player Team has head & Carmen in San Mona",
	"Player Team has head & Carmen in Cambria",
	"Player Team has head & Carmen in Drassen",
	"Father is drunk",
	"Player has wounded mercs within 8 tiles of NPC",
	"1 & only 1 merc wounded within 8 tiles of NPC",
	"More then 1 wounded merc within 8 tiles of NPC",
	"Brenda is in the store ",
	"Brenda is Dead",

	"Brenda is at home",		//80
	"NPC is an enemy",
	"Speaker Strength >= 84 and < 3 males present",
	"Speaker Strength >= 84 and at least 3 males present",
	"Hans lets you see Tony",
	"Hans is standing on 13523",
	"Tony isnt available Today",
	"Female is speaking to NPC",
	"Player has enjoyed the Brothel",
	"Carla is available",

	"Cindy is available",		//90
	"Bambi is available",
	"No girls is available",
	"Player waited for girls",
	"Player paid right amount of money",
	"Mercs walked by goon",
	"More thean 1 merc present within 3 tiles of NPC",
	"At least 1 merc present withing 3 tiles of NPC",
	"Kingping expectingh visit from player",
	"Darren expecting money from player",

	"Player within 5 tiles and NPC is visible",			//	100
	"Carmen is in San Mona",
	"Player Spoke to Carmen",
	"KingPin knows about stolen money",
	"Player gave money back to KingPin",
	"Frank was given the money ( not to buy booze )",
	"Player was told about KingPin watching fights",
	"Past club closing time and Darren warned Player. (reset every day)",
	"Joey is EPC",
	"Joey is in C5",

	"Joey is within 5 tiles of Martha(109) in sector G8",		//110
	"Joey is Dead!",
	"At least one player merc within 5 tiles of Martha",
	"Spike is occuping tile 9817",
	"Angel offered vest",
	"Angel sold vest",
	"Maria is EPC",
	"Maria is EPC and inside leather Shop",
	"Player wants to buy vest",
	"Maria rescue was noticed by KingPin goons and Kingpin now enemy",

	"Angel left deed on counter",		//120
	"Maria quest over",
	"Player bandaged NPC today",
	"Doreen revealed allegiance to Queen",
	"Pablo should not steal from player",
	"Player shipment arrived but loyalty to low, so it left",
	"Helicopter is in working condition",
	"Player is giving amount of money >= $1000",
	"Player is giving amount less than $1000",
	"Waldo agreed to fix helicopter( heli is damaged )",

	"Helicopter was destroyed",		//130
	"Waldo told us about heli pilot",
	"Father told us about Deidranna killing sick people",
	"Father told us about Chivaldori family",
	"Father told us about creatures",
	"Loyalty is OK",
	"Loyalty is Low",
	"Loyalty is High",
	"Player doing poorly",
	"Player gave valid head to Carmen",

	"Current sector is G9(Cambria)",		//140
	"Current sector is C5(SanMona)",
	"Current sector is C13(Drassen",
	"Carmen has at least $10,000 on him",
	"Player has Slay on team for over 48 hours",
	"Carmen is suspicous about slay",
	"Slay is in current sector",
	"Carmen gave us final warning",
	"Vince has explained that he has to charge",
	"Vince is expecting cash (reset everyday)",

	"Player stole some medical supplies once",		//150
	"Player stole some medical supplies again",
	"Vince can be recruited",
	"Vince is currently doctoring",
	"Vince was recruited",
	"Slay offered deal",
	"All terrorists killed",
	"",
	"Maria left in wrong sector",
	"Skyrider left in wrong sector",

	"Joey left in wrong sector",		//160
	"John left in wrong sector",
	"Mary left in wrong sector",
	"Walter was bribed",
	"Shank(67) is part of squad but not speaker",
	"Maddog spoken to",
	"Jake told us about shank",
	"Shank(67) is not in secotr",
	"Bloodcat quest on for more than 2 days",
	"Effective threat made to Armand",

	"Queen is DEAD!",												//170
	"Speaker is with Aim or Aim person on squad within 10 tiles",
	"Current mine is empty",
	"Current mine is running out",
	"Loyalty low in affiliated town (low mine production)",
	"Creatures invaded current mine",
	"Player LOST current mine",
	"Current mine is at FULL production",
	"Dynamo(66) is Speaker or within 10 tiles of speaker",
	"Fred told us about creatures",

	"Matt told us about creatures",				//180
	"Oswald told us about creatures",
	"Calvin told us about creatures",
	"Carl told us about creatures",
	"Chalice stolen from museam",
	"John(118) is EPC",
	"Mary(119) and John (118) are EPC's",
	"Mary(119) is alive",
	"Mary(119)is EPC",
	"Mary(119) is bleeding",

	"John(118) is alive",							//190
	"John(118) is bleeding",
	"John or Mary close to airport in Drassen(B13)",
	"Mary is Dead",
	"Miners placed",
	"Krott planning to shoot player",
	"Madlab explained his situation",
	"Madlab expecting a firearm",
	"Madlab expecting a video camera.",
	"Item condition is < 70 ",

	"Madlab complained about bad firearm.",					//200
	"Madlab complained about bad video camera.",
	"Robot is ready to go!",
	"First robot destroyed.",
	"Madlab given a good camera.",
	"Robot is ready to go a second time!",
	"Second robot destroyed.",
	"Mines explained to player.",
	"Dynamo (#66) is in sector J9.",
	"Dynamo (#66) is alive.",

	"One PC hasn't fought, but is able, and less than 3 fights have occured",							//210
	"Player receiving mine income from Drassen, Cambria, Alma & Chitzena",
	"Player has been to K4_b1",
	"Brewster got to talk while Warden was alive",
	"Warden (#103) is dead.",
	"Ernest gave us the guns",
	"This is the first bartender",
	"This is the second bartender",
	"This is the third bartender",
	"This is the fourth bartender",


	"Manny is a bartender.",									//220
	"Nothing is repaired yet (some stuff being worked on, nothing to give player right now)",
	"Player made purchase from Howard (#125)",
	"Dave sold vehicle",
	"Dave's vehicle ready",
	"Dave expecting cash for car",
	"Dave has gas. (randomized daily)",
	"Vehicle is present",
	"First battle won by player",
	"Robot recruited and moved",

	"No club fighting allowed",							//230
	"Player already fought 3 fights today",
	"Hans mentioned Joey",
	"Player is doing better than 50% (Alex's function)",
	"Player is doing very well (better than 80%)",
	"Father is drunk and sci-fi option is on",
	"Micky (#96) is drunk",
	"Player has attempted to force their way into brothel",
	"Rat effectively threatened 3 times",
	"Player paid for two people to enter brothel",

	"",						//240
	"",
	"Player owns 2 towns including omerta",
	"Player owns 3 towns including omerta",//							243
	"Player owns 4 towns including omerta",//							244
	"",
	"",
	"",
	"Fact male speaking female present",
	"Fact hicks married player merc",//													249

	"Fact museum open",//																				250
	"Fact brothel open",//																			251
	"Fact club open",//																					252
	"Fact first battle fought",//																253
	"Fact first battle being fought",//													254
	"Fact kingpin introduced self",//														255
	"Fact kingpin not in office",//															256
	"Fact dont owe kingpin money",//														257
	"Fact pc marrying daryl is flo",//													258
	"",

	"",					//260
	"Fact npc cowering",																			//	261,
	"",
	"",
	"Fact top and bottom levels cleared",
	"Fact top level cleared",//																	265
	"Fact bottom level cleared",//															266
	"Fact need to speak nicely",//															267
	"Fact attached item before",//															268
	"Fact skyrider ever escorted",//														269

	"Fact npc not under fire",//																270
	"Fact willis heard about joey rescue",//										271
	"Fact willis gives discount",//															272
	"Fact hillbillies killed",//																273
	"Fact keith out of business",	//														274
	"Fact mike available to army",//														275
	"Fact kingpin can send assassins",//												276
	"Fact estoni refuelling possible",//                        277
	"Fact museum alarm went off",//															278
	"",

	"Fact maddog is speaker", 																	//280,
	"",
	"Fact angel mentioned deed", 															// 282,
	"Fact iggy available to army",//														283
	"Fact pc has conrads recruit opinion",//										284
	"",
	"",
	"",
	"",
	"Fact npc hostile or pissed off",													//289,

	"",					//290
	"Fact tony in building",																		//291,
	"Fact shank speaking",	//																		292,
	"Fact doreen alive",//																			293
	"Fact waldo alive",//																				294
	"Fact perko alive",//																				295
	"Fact tony alive",//																				296
	"",
	"Fact vince alive",// 																				298,
	"Fact jenny alive",//																				299

	"",					//300
	"",
	"Fact arnold alive",//																				302,
	"",
	"Fact rocket rifle exists",// 																304,
	"",
	"",
	"",
	"",
	"",

	"",					//310
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//320

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//330

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//340

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//350

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//360

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//370

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//380

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//390

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//400

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//410

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//420

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//430

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//440

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//450

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//460

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//470

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//480

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//490

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",					//500

};
