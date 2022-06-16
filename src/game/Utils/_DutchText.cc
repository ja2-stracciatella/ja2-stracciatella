#include "Text.h"

#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"
#endif

#include <string_theory/string>


// ******************************************************************************************************
// **                                  IMPORTANT TRANSLATION NOTES                                     **
// ******************************************************************************************************
//
// GENERAL INSTRUCTIONS
// - Always be aware that foreign strings should be of equal or shorter length than the English equivalent.
//   I know that this is difficult to do on many occasions due to the nature of foreign languages when
//   compared to English.  By doing so, this will greatly reduce the amount of work on both sides.  In
//   most cases (but not all), JA2 interfaces were designed with just enough space to fit the English word.
//   The general rule is if the string is very short (less than 10 characters), then it's short because of
//   interface limitations.  On the other hand, full sentences commonly have little limitations for length.
//   Strings in between are a little dicey.
// - Never translate a string to appear on multiple lines.  All strings "This is a really long string...",
//   must fit on a single line no matter how long the string is.  All strings start with " and end with ",
// - Never remove any extra spaces in strings.  In addition, all strings containing multiple sentences only
//   have one space after a period, which is different than standard typing convention.  Never modify sections
//   of strings contain combinations of % characters.  These are special format characters and are always
//   used in conjunction with other characters.  For example, %s means string, and is commonly used for names,
//   locations, items, etc.  %d is used for numbers.  %c%d is a character and a number (such as A9).
//   %% is how a single % character is built.  There are countless types, but strings containing these
//   special characters are usually commented to explain what they mean.  If it isn't commented, then
//   if you can't figure out the context, then feel free to ask SirTech.
// - Comments are always started with // Anything following these two characters on the same line are
//   considered to be comments.  Do not translate comments.  Comments are always applied to the following
//   string(s) on the next line(s), unless the comment is on the same line as a string.
// - All new comments made by SirTech will use "//@@@ comment" (without the quotes) notation.  By searching
//   for @@@ everytime you recieve a new version, it will simplify your task and identify special instructions.
//   Commonly, these types of comments will be used to ask you to abbreviate a string.  Please leave the
//   comments intact, and SirTech will remove them once the translation for that particular area is resolved.
// - If you have a problem or question with translating certain strings, please use "//!!! comment"
//   (without the quotes).  The syntax is important, and should be identical to the comments used with @@@
//   symbols.  SirTech will search for !!! to look for your problems and questions.  This is a more
//   efficient method than detailing questions in email, so try to do this whenever possible.
//
//
//
// FAST HELP TEXT -- Explains how the syntax of fast help text works.
// **************
//
// 1) BOLDED LETTERS
//   The popup help text system supports special characters to specify the hot key(s) for a button.
//   Anytime you see a '|' symbol within the help text string, that means the following key is assigned
//   to activate the action which is usually a button.
//
//   EX:  "|Map Screen"
//
//   This means the 'M' is the hotkey.  In the game, when somebody hits the 'M' key, it activates that
//   button.  When translating the text to another language, it is best to attempt to choose a word that
//   uses 'M'.  If you can't always find a match, then the best thing to do is append the 'M' at the end
//   of the string in this format:
//
//   EX:  "Ecran De Carte (|M)"  (this is the French translation)
//
//   Other examples are used multiple times, like the Esc key  or "|E|s|c" or Space -> (|S|p|a|c|e)
//
// 2) NEWLINE
//   Any place you see a \n within the string, you are looking at another string that is part of the fast help
//   text system.  \n notation doesn't need to be precisely placed within that string, but whereever you wish
//   to start a new line.
//
//   EX:  "Clears all the mercs' positions,\nand allows you to re-enter them manually."
//
//   Would appear as:
//
//     Clears all the mercs' positions,
//     and allows you to re-enter them manually.
//
//   NOTE:  It is important that you don't pad the characters adjacent to the \n with spaces.  If we did this
//     in the above example, we would see
//
//   WRONG WAY -- spaces before and after the \n
//   EX:  "Clears all the mercs' positions, \n and allows you to re-enter them manually."
//
//   Would appear as: (the second line is moved in a character)
//
//     Clears all the mercs' positions,
//     and allows you to re-enter them manually.
//
//
// @@@ NOTATION
// ************
//
//   Throughout the text files, you'll find an assortment of comments.  Comments are used to describe the
//   text to make translation easier, but comments don't need to be translated.  A good thing is to search for
//   "@@@" after receiving new version of the text file, and address the special notes in this manner.
//
// !!! NOTATION
// ************
//
//   As described above, the "!!!" notation should be used by you to ask questions and address problems as
//   SirTech uses the "@@@" notation.



static ST::string s_dut_WeaponType[WeaponType_SIZE] =
{
	"Other",
	"Pistol",
	"Machine pistol",
	"Machine Gun",
	"Rifle",
	"Sniper Rifle",
	"Attack weapon",
	"Light machine gun",
	"Shotgun",
};

static ST::string s_dut_TeamTurnString[TeamTurnString_SIZE] =
{
	"Beurt speler",
	"Beurt opponent",
	"Beurt beest",
	"Beurt militie",
	"Beurt burgers",
	// planning turn
};

static ST::string s_dut_Message[Message_SIZE] =
{
	// In the following 8 strings, the %s is the merc's name, and the %d (if any) is a number.

	"%s geraakt in hoofd en verliest een intelligentiepunt!",
	"%s geraakt in de schouder en verliest een handigheidspunt!",
	"%s geraakt in de borst en verliest een krachtspunt!",
	"%s geraakt in het benen en verliest een beweeglijkspunt!",
	"%s geraakt in het hoofd en verliest %d wijsheidspunten!",
	"%s geraakt in de schouder en verliest %d handigheidspunten!",
	"%s geraakt in de borst en verliest %d krachtspunten!",
	"%s geraakt in de benen en verliest %d beweeglijkheidspunten!",
	"Storing!",

	"Je versterkingen zijn gearriveerd!",

	// In the following four lines, all %s's are merc names

	"%s herlaad.",
	"%s heeft niet genoeg actiepunten!",
	// the following 17 strings are used to create lists of gun advantages and disadvantages
	// (separated by commas)
	"reliable",
	"unreliable",
	"easy to repair",
	"hard to repair",
	"much damage",
	"low damage",
	"quick fire",
	"slow fire",
	"long range",
	"short range",
	"light",
	"heavy",
	"small",
	"quick salvo",
	"no salvo",
	"large magazine",
	"small magazine",

	// In the following two lines, all %s's are merc names

	"%s's camouflage is verdwenen.",
	"%s's camouflage is afgespoelt.",

	// The first %s is a merc name and the second %s is an item name

	"Tweede wapen is leeg!",
	"%s heeft %s gestolen.",

	// The %s is a merc name

	"%s's wapen vuurt geen salvo.",

	"Je hebt er al één van die vastgemaakt.",
	"Samen voegen?",

	// Both %s's are item names

	"Je verbindt %s niet met %s.",
	"Geen",
	"Eject ammo",
	"Toebehoren",

	//You cannot use "item(s)" and your "other item" at the same time.
	//Ex:  You cannot use sun goggles and you gas mask at the same time.
	"%s en %s zijn niet tegelijk te gebruiken.",

	"Het item dat je aanwijst, kan vastgemaakt worden aan een bepaald item door het in een van de vier uitbreidingssloten te plaatsen.",
	"Het item dat je aanwijst, kan vastgemaakt worden aan een bepaald item door het in een van de vier uitbreidingssloten te plaatsen. (Echter, het item is niet compatibel.)",
	"Er zijn nog vijanden in de sector!",
	"Je moet %s %s nog geven",
	"kogel doorboorde %s in zijn hoofd!",
	"Gevecht verlaten?",
	"Dit samenvoegen is permanent. Verdergaan?",
	"%s heeft meer energie!",
	"%s is uitgegleden!",
	"%s heeft %s niet gepakt!",
	"%s repareert de %s",
	"Stoppen voor ",
	"Overgeven?",
	"Deze persoon weigert je hulp.",
	"Ik denk het NIET!",
	"Chopper van Skyrider gebruiken? Eerst huurlingen TOEWIJZEN aan VOERTUIG/HELIKOPTER.",
	"%s had tijd maar EEN geweer te herladen",
	"Beurt bloodcats",
};


// the types of time compression. For example: is the timer paused? at normal speed, 5 minutes per second, etc.
// min is an abbreviation for minutes

static const ST::string s_dut_sTimeStrings[sTimeStrings_SIZE] =
{
	"Pause",
	"Normal",
	"5 min",
	"30 min",
	"60 min",
	"6 uur",
};


// Assignment Strings: what assignment does the merc  have right now? For example, are they on a squad, training,
// administering medical aid (doctor) or training a town. All are abbreviated. 8 letters is the longest it can be.

static const ST::string s_dut_pAssignmentStrings[pAssignmentStrings_SIZE] =
{
	"Team 1",
	"Team 2",
	"Team 3",
	"Team 4",
	"Team 5",
	"Team 6",
	"Team 7",
	"Team 8",
	"Team 9",
	"Team 10",
	"Team 11",
	"Team 12",
	"Team 13",
	"Team 14",
	"Team 15",
	"Team 16",
	"Team 17",
	"Team 18",
	"Team 19",
	"Team 20",
	"Dienst", // on active duty
	"Dokter", // administering medical aid
	"Patiënt", // getting medical aid
	"Voertuig", // in a vehicle
	"Onderweg", // in transit - abbreviated form
	"Repareer", // repairing
	"Oefenen", // training themselves
	"Militie", // training a town to revolt
	"Trainer", // training a teammate
	"Student", // being trained by someone else
	"Dood", // dead
	"Uitgesc.", // abbreviation for incapacitated
	"POW", // Prisoner of war - captured
	"Kliniek", // patient in a hospital
	"Leeg",	// Vehicle is empty
};


static const ST::string s_dut_pMilitiaString[pMilitiaString_SIZE] =
{
	"Militie", // the title of the militia box
	"Unassigned", //the number of unassigned militia troops
	"Milities kunnen niet herplaatst worden als er nog vijanden in de buurt zijn!",
};


static const ST::string s_dut_pMilitiaButtonString[pMilitiaButtonString_SIZE] =
{
	"Auto", // auto place the militia troops for the player
	"OK", // done placing militia troops
};

static const ST::string s_dut_pConditionStrings[pConditionStrings_SIZE] =
{
	"Excellent", //the state of a soldier .. excellent health
	"Good", // good health
	"Fair", // fair health
	"Wounded", // wounded health
	"Tired", // tired
	"Bleeding", // bleeding to death
	"Knocked out", // knocked out
	"Dying", // near death
	"Dead", // dead
};

static const ST::string s_dut_pEpcMenuStrings[pEpcMenuStrings_SIZE] =
{
	"On duty", // set merc on active duty
	"Patient", // set as a patient to receive medical aid
	"Vehicle", // tell merc to enter vehicle
	"Alone", // let the escorted character go off on their own
	"Close", // close this menu
};


// look at pAssignmentString above for comments

static const ST::string s_dut_pLongAssignmentStrings[pLongAssignmentStrings_SIZE] =
{
	"Team 1",
	"Team 2",
	"Team 3",
	"Team 4",
	"Team 5",
	"Team 6",
	"Team 7",
	"Team 8",
	"Team 9",
	"Team 10",
	"Team 11",
	"Team 12",
	"Team 13",
	"Team 14",
	"Team 15",
	"Team 16",
	"Team 17",
	"Team 18",
	"Team 19",
	"Team 20",
	"Dienst", // on active duty
	"Dokter", // administering medical aid
	"Patiënt", // getting medical aid
	"Voertuig", // in a vehicle
	"Onderweg", // in transit - abbreviated form
	"Repareer", // repairing
	"Oefenen", // training themselves
	"Militie", // training a town to revolt
	"Trainer", // training a teammate
	"Student", // being trained by someone else
	"Dood", // dead
	"Uitgesc.", // abbreviation for incapacitated
	"POW", // Prisoner of war - captured
	"Kliniek", // patient in a hospital
	"Leeg",	// Vehicle is empty
};


// the contract options

static const ST::string s_dut_pContractStrings[pContractStrings_SIZE] =
{
	"Contract Opties:",
	"", // a blank line, required
	"Voor een dag", // offer merc a one day contract extension
	"Voor een week", // 1 week
	"Voor twee weken", // 2 week
	"Ontslag", // end merc's contract
	"Stop", // stop showing this menu
};

static const ST::string s_dut_pPOWStrings[pPOWStrings_SIZE] =
{
	"POW",  //an acronym for Prisoner of War
	"??",
};

static const ST::string s_dut_pInvPanelTitleStrings[pInvPanelTitleStrings_SIZE] =
{
	"Wapen", // the armor rating of the merc
	"Gewicht", // the weight the merc is carrying
	"Camo", // the merc's camouflage rating
};

static const ST::string s_dut_pShortAttributeStrings[pShortAttributeStrings_SIZE] =
{
	"Bew", // the abbreviated version of : agility
	"Han", // dexterity
	"Kra", // strength
	"Ldr", // leadership
	"Wij", // wisdom
	"Niv", // experience level
	"Tre", // marksmanship skill
	"Exp", // explosive skill
	"Tec", // mechanical skill
	"Med", // medical skill
};


static const ST::string s_dut_pUpperLeftMapScreenStrings[pUpperLeftMapScreenStrings_SIZE] =
{
	"Opdracht", // the mercs current assignment
	"Gezond", // the health level of the current merc
	"Moraal", // the morale of the current merc
	"Conditie",	// the condition of the current vehicle
};

static const ST::string s_dut_pTrainingStrings[pTrainingStrings_SIZE] =
{
	"Oefen", // tell merc to train self
	"Militie", // tell merc to train town
	"Trainer", // tell merc to act as trainer
	"Student", // tell merc to be train by other
};

static const ST::string s_dut_pAssignMenuStrings[pAssignMenuStrings_SIZE] =
{
	"On duty", // merc is on active duty
	"Doctor", // the merc is acting as a doctor
	"Patient", // the merc is receiving medical attention
	"Vehicle", // the merc is in a vehicle
	"Repair", // the merc is repairing items
	"Train", // the merc is training
	"Stop", // cancel this menu
};

static const ST::string s_dut_pRemoveMercStrings[pRemoveMercStrings_SIZE] =
{
	"Verw.Huurl.", // remove dead merc from current team
	"Stop",
};

static const ST::string s_dut_pAttributeMenuStrings[pAttributeMenuStrings_SIZE] =
{
	"Kracht",
	"Behendigheid",
	"Lenigheid",
	"Gezondheid",
	"Scherpschutterskunst",
	"Medisch",
	"Mechanisch",
	"Leiderschap",
	"Explosief",
	"Annuleren",
};

/*
static const ST::string s_dut_pAttributeMenuStrings[pAttributeMenuStrings_SIZE] =
{
	"Kracht",
	"Behendig",
	"Beweging",
	"Schietkunst",
	"Trefzekerheid",
	"Medisch",
	"Technisch",
	"Leaderschip",
	"Precisie",
	"Stop",
};
*/

static const ST::string s_dut_pTrainingMenuStrings[pTrainingMenuStrings_SIZE] =
{
	"Oefenen", // train yourself
	"Militie", // train the town
	"Trainer", // train your teammates
	"Student",  // be trained by an instructor
	"Stop", // cancel this menu
};


static const ST::string s_dut_pSquadMenuStrings[pSquadMenuStrings_SIZE] =
{
	"Team  1",
	"Team  2",
	"Team  3",
	"Team  4",
	"Team  5",
	"Team  6",
	"Team  7",
	"Team  8",
	"Team  9",
	"Team 10",
	"Team 11",
	"Team 12",
	"Team 13",
	"Team 14",
	"Team 15",
	"Team 16",
	"Team 17",
	"Team 18",
	"Team 19",
	"Team 20",
	"Stop",
};


static const ST::string s_dut_pPersonnelScreenStrings[pPersonnelScreenStrings_SIZE] =
{
	"Med. Kosten:", // amount of medical deposit put down on the merc
	"Rest Contract:", // cost of current contract
	"Doden", // number of kills by merc
	"Hulp", // number of assists on kills by merc
	"Dag. Kosten:", // daily cost of merc
	"Huidige Tot. Kosten:", // total cost of merc
	"Huidige Tot. Service:", // total service rendered by merc
	"Salaris Tegoed:", // amount left on MERC merc to be paid
	"Trefzekerheid:", // percentage of shots that hit target
	"Gevechten", // number of battles fought
	"Keren Gewond", // number of times merc has been wounded
	"Vaardigheden:",
	"Vaardigheden:",
};


//These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
static const ST::string s_dut_gzMercSkillText[gzMercSkillText_SIZE] =
{
	"No Skill",
	"Forceer slot",
	"Man-tot-man",
	"Elektronica",
	"Nachtops",
	"Werpen",
	"Lesgeven",
	"Zware Wapens",
	"Auto Wapens",
	"Sluipen",
	"Handig",
	"Dief",
	"Vechtkunsten",
	"Mesworp",
	"Raak op dak! Bonus",
	"Camouflaged",
	"(Expert)",
};


// This is pop up help text for the options that are available to the merc

static const ST::string s_dut_pTacticalPopupButtonStrings[pTacticalPopupButtonStrings_SIZE] =
{
	"|Staan/Lopen",
	"Hurken/Gehurkt lopen (|C)",
	"Staan/|Rennen",
	"Liggen/Kruipen (|P)",
	"Kijk (|L)",
	"Actie",
	"Praat",
	"Bekijk (|C|t|r|l)",

	// Pop up door menu
	"Handm. openen",
	"Zoek boobytraps",
	"Forceer",
	"Met geweld",
	"Verwijder boobytrap",
	"Sluiten",
	"Maak open",
	"Gebruik explosief",
	"Gebruik breekijzer",
	"Stoppen (|E|s|c)",
	"Stop",
};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.

static const ST::string s_dut_pDoorTrapStrings[pDoorTrapStrings_SIZE] =
{
	"geen val",
	"een explosie",
	"een elektrische val",
	"alarm",
	"stil alarm",
};

// On the map screen, there are four columns. This text is popup help text that identifies the individual columns.

static const ST::string s_dut_pMapScreenMouseRegionHelpText[pMapScreenMouseRegionHelpText_SIZE] =
{
	"Selecteer Karakter",
	"Contracteer huurling",
	"Plan Route",
	"Huurling |Contract",
	"Verwijder Huurling",
	"Slaap",
};

// volumes of noises

static const ST::string s_dut_pNoiseVolStr[pNoiseVolStr_SIZE] =
{
	"VAAG",
	"ZEKER",
	"HARD",
	"ERG HARD",
};

// types of noises

static const ST::string s_dut_pNoiseTypeStr[pNoiseTypeStr_SIZE] = // OBSOLETE
{
	"ONBEKEND",
	"geluid van BEWEGING",
	"GEKRAAK",
	"PLONZEN",
	"INSLAG",
	"SCHOT",
	"EXPLOSIE",
	"GEGIL",
	"INSLAG",
	"INSLAG",
	"BARSTEN",
	"DREUN",
};

// Directions that are used to report noises

static const ST::string s_dut_pDirectionStr[pDirectionStr_SIZE] =
{
	"het NOORDOOSTEN",
	"het OOSTEN",
	"het ZUIDOOSTEN",
	"het ZUIDEN",
	"het ZUIDWESTEN",
	"het WESTEN",
	"het NOORDWESTEN",
	"het NOORDEN",
};

static const ST::string s_dut_gpStrategicString[gpStrategicString_SIZE] =
{
	"%s zijn ontdekt in sector %s en een ander team arriveert binnenkort.",	//STR_DETECTED_SINGULAR
	"%s zijn ontdekt in sector %s en andere teams arriveren binnenkort.",	//STR_DETECTED_PLURAL
	"Wil je een gezamenlijke aankomst coördineren?",					//STR_COORDINATE

	//Dialog strings for enemies.

	"De vijand geeft je de kans om je over te geven.",			//STR_ENEMY_SURRENDER_OFFER
	"De vijand heeft je overgebleven bewusteloze huurlingen gevangen.",	//STR_ENEMY_CAPTURED

	//The text that goes on the autoresolve buttons

	"Vluchten", 	//The retreat button			//STR_AR_RETREAT_BUTTON
	"OK",		//The done button				//STR_AR_DONE_BUTTON

	//The headers are for the autoresolve type (MUST BE UPPERCASE)

	"VERDEDIGEN",								//STR_AR_DEFEND_HEADER
	"AANVALLEN",								//STR_AR_ATTACK_HEADER
	"ONTDEKKEN",								//STR_AR_ENCOUNTER_HEADER
	"Sector",		//The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

	//The battle ending conditions

	"VICTORIE!",								//STR_AR_OVER_VICTORY
	"NEDERLAAG!",								//STR_AR_OVER_DEFEAT
	"OVERGEGEVEN!",							//STR_AR_OVER_SURRENDERED
	"GEVANGEN!",								//STR_AR_OVER_CAPTURED
	"GEVLUCHT!",								//STR_AR_OVER_RETREATED

	//These are the labels for the different types of enemies we fight in autoresolve.

	"Militie",							//STR_AR_MILITIA_NAME,
	"Elite",							//STR_AR_ELITE_NAME,
	"Troep",							//STR_AR_TROOP_NAME,
	"Admin",							//STR_AR_ADMINISTRATOR_NAME,
	"Wezen",							//STR_AR_CREATURE_NAME,

	//Label for the length of time the battle took

	"Tijd verstreken",							//STR_AR_TIME_ELAPSED,

	//Labels for status of merc if retreating.  (UPPERCASE)

	"GEVLUCHT",							//STR_AR_MERC_RETREATED,
	"VLUCHTEN",							//STR_AR_MERC_RETREATING,
	"VLUCHT",								//STR_AR_MERC_RETREAT,

	//PRE BATTLE INTERFACE STRINGS
	//Goes on the three buttons in the prebattle interface.  The Auto resolve button represents
	//a system that automatically resolves the combat for the player without having to do anything.
	//These strings must be short (two lines -- 6-8 chars per line)

	"Autom. Opl.",			//!!! 1			//STR_PB_AUTORESOLVE_BTN,
	"Naar Sector",						//STR_PB_GOTOSECTOR_BTN,
	"Terug- trekken",			//!!! 2		//STR_PB_RETREATMERCS_BTN,

	//The different headers(titles) for the prebattle interface.
	"VIJAND ONTDEKT",						//STR_PB_ENEMYENCOUNTER_HEADER,
	"INVASIE VIJAND",						//STR_PB_ENEMYINVASION_HEADER, // 30
	"HINDERLAAG VIJAND",						//STR_PB_ENEMYAMBUSH_HEADER
	"BINNENGAAN VIJANDIGE SECTOR",				//STR_PB_ENTERINGENEMYSECTOR_HEADER
	"AANVAL BEEST",							//STR_PB_CREATUREATTACK_HEADER
	"BLOODCAT VAL",							//STR_PB_BLOODCATAMBUSH_HEADER
	"BINNENGAAN HOL BLOODCAT",			//STR_PB_ENTERINGBLOODCATLAIR_HEADER

	//Various single words for direct translation.  The Civilians represent the civilian
	//militia occupying the sector being attacked.  Limited to 9-10 chars

	"Locatie",
	"Vijanden",
	"Huurlingen",
	"Milities",
	"Beesten",
	"Bloodcats",
	"Sector",
	"Geen",		//If there are no uninvolved mercs in this fight.
	"NVT",			//Acronym of Not Applicable
	"d",			//One letter abbreviation of day
	"u",			//One letter abbreviation of hour

	//TACTICAL PLACEMENT USER INTERFACE STRINGS
	//The four buttons

	"Weggaan",
	"Verspreid",
	"Groeperen",
	"OK",

	//The help text for the four buttons.  Use \n to denote new line (just like enter).

	"Maakt posities van huurlingen vrij en\nmaakt handmatig herinvoer mogelijk. (|C)",
	"Ver|spreidt willekeurig je huurlingen\nelke keer als je de toets indrukt.",
	"Hiermee is het mogelijk de huurlingen te |groeperen.",
	"Druk op deze toets als je klaar bent met\nhet positioneren van je huurlingen. (|E|n|t|e|r)",
	"Je moet al je huurlingen positioneren\nvoor je het gevecht kunt starten.",

	//Various strings (translate word for word)

	"Sector",
	"Kies posities binnenkomst",

	//Strings used for various popup message boxes.  Can be as long as desired.

	"Ziet er hier niet goed uit. Het is onbegaanbaar. Probeer een andere locatie.",
	"Plaats je huurlingen in de gemarkeerde sectie van de kaart.",

	//These entries are for button popup help text for the prebattle interface.  All popup help
	//text supports the use of \n to denote new line.  Do not use spaces before or after the \n.
	"Lost het gevecht |Automatisch\nop zonder de kaart te laden.",
	"Automatisch oplossen niet\nmogelijk als de speler aanvalt.",
	"Ga sector binnen om tegen\nde vijand te strijden. (|E)",
	"T|rek groep terug en ga naar de vorige sector.",			//singular version
	"T|rek alle groepen terug en\nga naar hun vorige sectors.", //multiple groups with same previous sector

	//various popup messages for battle conditions.

	//%s -- ex:  A9
	"Vijanden vallen je militie aan in sector %s.",
	//%s -- ex:  A9
	"Beesten vallen je militie aan in sector %s.",
	//1st %d refers to the number of civilians eaten by monsters,  %s -- ex:  A9
	//Note:  the minimum number of civilians eaten will be two.
	"Beesten vallen aan en doden %d burgers in sector %s.",
	//%s is the sector location -- ex:  A9: Omerta
	"Vijand valt je huurlingen aan in sector %s. Geen enkele huurling kan vechten!",
	//%s is the sector location -- ex:  A9: Omerta
	"Beesten vallen je huurlingen aan in sector %s. Geen enkele huurling kan vechten!",

};

//This is the day represented in the game clock.  Must be very short, 4 characters max.
static const ST::string s_dut_gpGameClockString = "Dag";

//When the merc finds a key, they can get a description of it which
//tells them where and when they found it.
static const ST::string s_dut_sKeyDescriptionStrings[sKeyDescriptionStrings_SIZE] =
{
	"Sector gevonden:",
	"Dag gevonden:",
};

//The headers used to describe various weapon statistics.

static ST::string s_dut_gWeaponStatsDesc[ gWeaponStatsDesc_SIZE] =
{
	"Gewicht (%s):",
	"Status:",
	"Munitie:", 		// Number of bullets left in a magazine
	"Afst:",		// Range
	"Sch:",		// Damage
	"AP:",			// abbreviation for Action Points
	"="
};

//The headers used for the merc's money.

static const ST::string s_dut_gMoneyStatsDesc[gMoneyStatsDesc_SIZE] =
{
	"Bedrag",
	"Restbedrag:", //this is the overall balance
	"Bedrag",
	"Splitsen:", // the amount he wants to separate from the overall balance to get two piles of money

	"Huidig",
	"Saldo",
	"Bedrag naar",
	"Opnemen",
};

//The health of various creatures, enemies, characters in the game. The numbers following each are for comment
//only, but represent the precentage of points remaining.

static const ST::string s_dut_zHealthStr[zHealthStr_SIZE] =
{
	"STERVEND",	//	>= 0
	"KRITIEK", 	//	>= 15
	"SLECHT",		//	>= 30
	"GEWOND",    	//	>= 45
	"GEZOND",    	//	>= 60
	"STERK",     	// 	>= 75
	"EXCELLENT",	// 	>= 90
};

static const ST::string s_dut_gzMoneyAmounts[gzMoneyAmounts_SIZE] =
{
	"$1000",
	"$100",
	"$10",
	"OK",
	"Splitsen",
	"Opnemen",
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
static const ST::string s_dut_gzProsLabel = "Voor:";
static const ST::string s_dut_gzConsLabel = "Tegen:";

//Conversation options a player has when encountering an NPC
static ST::string s_dut_zTalkMenuStrings[zTalkMenuStrings_SIZE] =
{
	"Wat?", 		//meaning "Repeat yourself"
	"Aardig",		//approach in a friendly
	"Direct",		//approach directly - let's get down to business
	"Dreigen",		//approach threateningly - talk now, or I'll blow your face off
	"Geef",
	"Rekruut",		//recruit
};

//Some NPCs buy, sell or repair items. These different options are available for those NPCs as well.
static ST::string s_dut_zDealerStrings[zDealerStrings_SIZE] =
{
	"Koop/Verkoop",	//Buy/Sell
	"Koop",		//Buy
	"Verkoop",		//Sell
	"Repareer",		//Repair
};

static const ST::string s_dut_zDialogActions = "OK";


//These are vehicles in the game.

static const ST::string s_dut_pVehicleStrings[pVehicleStrings_SIZE] =
{
	"Eldorado",
	"Hummer",			// a hummer jeep/truck -- military vehicle
	"Koeltruck",		// Icecream Truck
	"Jeep",
	"Tank",
	"Helikopter",
};

static const ST::string s_dut_pShortVehicleStrings[pVehicleStrings_SIZE] =
{
	"Eldor.",
	"Hummer",			// the HMVV
	"Truck",
	"Jeep",
	"Tank",
	"Heli", 				// the helicopter
};

static const ST::string s_dut_zVehicleName[pVehicleStrings_SIZE] =
{
	"Eldorado",
	"Hummer",		//a military jeep. This is a brand name.
	"Truck",			// Ice cream truck
	"Jeep",
	"Tank",
	"Heli", 		//an abbreviation for Helicopter
};


//These are messages Used in the Tactical Screen

static ST::string s_dut_TacticalStr[TacticalStr_SIZE] =
{
	"Luchtaanval",
	"Automatisch EHBO toepassen?",

	// CAMFIELD NUKE THIS and add quote #66.

	"%s ziet dat er items missen van de lading.",

	// The %s is a string from pDoorTrapStrings

	"Het slot heeft %s.",
	"Er is geen slot.",
	"Geen boobytrap op het slot.",
	// The %s is a merc name
	"%s heeft niet de juiste sleutel.",
	"Slot heeft geen boobytrap.",
	"Op slot.",
	"DEUR",
	"VAL",
	"OP SLOT",
	"OPEN",
	"KAPOT",
	"Hier zit een schakelaar. Activeren?",
	"Boobytrap ontmantelen?",
	"Meer...",

	// In the next 2 strings, %s is an item name

	"%s is op de grond geplaatst.",
	"%s is gegeven aan %s.",

	// In the next 2 strings, %s is a name

	"%s is helemaal betaald.",
	"%s heeft tegoed nog %d.",
	"Kies detonatie frequentie:",  	//in this case, frequency refers to a radio signal
	"Aantal beurten tot ontploffing:",	//how much time, in turns, until the bomb blows
	"Stel frequentie in van ontsteking:", 	//in this case, frequency refers to a radio signal
	"Boobytrap ontmantelen?",
	"Blauwe vlag weghalen?",
	"Blauwe vlag hier neerzetten?",
	"Laatste beurt",

	// In the next string, %s is a name. Stance refers to way they are standing.

	"Zeker weten dat je %s wil aanvallen?",
	"Ah, voertuigen kunnen plaats niet veranderen.",
	"De robot kan niet van plaats veranderen.",

	// In the next 3 strings, %s is a name

	"%s kan niet naar die plaats gaan.",
	"%s kan hier geen EHBO krijgen.",
	"%s heeft geen EHBO nodig.",
	"Kan daar niet heen.",
	"Je team is vol. Geen ruimte voor rekruut.",	//there's no room for a recruit on the player's team

	// In the next string, %s is a name

	"%s is gerekruteerd.",

	// Here %s is a name and %d is a number

	"%s ontvangt $%d.",

	// In the next string, %s is a name

	"%s begeleiden?",

	// In the next string, the first %s is a name and the second %s is an amount of money (including $ sign)

	"%s inhuren voor %s per dag?",

	// This line is used repeatedly to ask player if they wish to participate in a boxing match.

	"Wil je vechten?",

	// In the next string, the first %s is an item name and the
	// second %s is an amount of money (including $ sign)

	"%s kopen voor %s?",

	// In the next string, %s is a name

	"%s wordt begeleid door team %d.",

	// These messages are displayed during play to alert the player to a particular situation

	"GEBLOKKEERD",					//weapon is jammed.
	"Robot heeft %s kal. munitie nodig.",		//Robot is out of ammo
	"Hier gooien? Kan niet.",		//Merc can't throw to the destination he selected

	// These are different buttons that the player can turn on and off.

	"Sluipmodus (|Z)",			// "Stealth Mode (|Z)",
	"Landkaart (|M)",			// "|Map Screen",
	"OK (Ein|de)",				// "|Done (End Turn)",
	"Praat",					// "Talk",
	"Stil",					// "Mute",
	"Omhoog (|P|g|U|p)",			// "Stance Up (|P|g|U|p)",
	"Cursor Niveau (|T|a|b)",		// "Cursor Level (|T|a|b)",
	"Klim / Spring",				// "Climb / Jump",
	"Omlaag (|P|g|D|n)",			// "Stance Down (|P|g|D|n)",
	"Bekijk (|C|t|r|l)",			// "Examine (|C|t|r|l)",
	"Vorige huurling",			// "Previous Merc",
	"Volgende huurling (|S|p|a|c|e)",		// "Next Merc (|S|p|a|c|e)",
	"|Opties",					// "|Options",
	"Salvo's (|B)",				// "|Burst Mode",
	"Kijk/draai (|L)",			// "|Look/Turn",
	"Gezond: %d/%d\nKracht: %d/%d\nMoraal: %s",		// "Health: %d/%d\nEnergy: %d/%d\nMorale: %s",
	"Hé?",					//this means "what?"
	"Door",					//an abbrieviation for "Continued"
	"%s is praat weer.",			// "Mute off for %s.",
	"%s is stil.",				// "Mute on for %s.",
	"Gezond: %d/%d\nBrandst: %d/%d",	// "Health: %d/%d\nFuel: %d/%d",
	"Stap uit voertuig",			// "Exit Vehicle" ,
	"Wissel Team ( |S|h|i|f|t |S|p|a|c|e )",			// "Change Squad ( |S|h|i|f|t |S|p|a|c|e )",
	"Rijden",					// "Drive",
	"Nvt",						//this is an acronym for "Not Applicable."
	"Actie ( Man-tot-man )",		// "Use ( Hand To Hand )",
	"Actie ( Firearm )",			// "Use ( Firearm )",
	"Actie ( Mes )",				// "Use ( Blade )",
	"Actie ( Explosieven )",		// "Use ( Explosive )",
	"Actie ( EHBO )",			// "Use ( Medkit )",
	"(Vang)",					// "(Catch)",
	"(Herlaad)",				// "(Reload)",
	"(Geef)",					// "(Give)",
	"%s is afgezet.",			// "%s has been set off.",
	"%s is gearriveerd.",			// "%s has arrived.",
	"%s heeft geen Actie Punten.",	// "%s ran out of Action Points.",
	"%s is niet beschikbaar.",		// "%s isn't available.",
	"%s zit onder het verband.",		// "%s is all bandaged.",
	"Verband van %s is op.",		// "%s is out of bandages.",
	"Vijand in de sector!",		// "Enemy in sector!",
	"Geen vijanden in zicht.",		// "No enemies in sight.",
	"Niet genoeg Actie Punten.",		// "Not enough Action Points.",
	"Niemand gebruikt afstandb.",	// "Nobody's using the remote.",
	"Magazijn leeg door salvovuur!",	// "Burst fire emptied the clip!",
	"SOLDAAT",					// "SOLDIER",
	"CREPITUS",				// "CREPITUS",
	"MILITIE",					// "MILITIA",
	"BURGER",					// "CIVILIAN",
	"Verlaten Sector",			// "Exiting Sector",
	"OK",
	"Stoppen",					// "Cancel",
	"Huurling gesel.",			// "Selected Merc",
	"Alle huurl. in team",			// "All Mercs in Squad",
	"Naar Sector",				// "Go to Sector",
	"Naar Landk.",				// "Go to Map",
	"Vanaf deze kant kun je de sector niet verlaten.",	// "You can't leave the sector from this side.",
	"%s is te ver weg.",			// "%s is too far away.",
	"Verwijder Boomtoppen",		// "Removing Treetops",
	"Tonen Boomtoppen",			// "Showing Treetops",
	"KRAAI",				//Crow, as in the large black bird
	"NEK",
	"HOOFD",
	"TORSO",
	"BENEN",
	"De Koningin vertellen wat ze wil weten?",		// "Tell the Queen what she wants to know?",
	"Vingerafdruk-ID nodig",					// "Fingerprint ID aquired",
	"Vingerafdruk-ID ongeldig. Wapen funct. niet",		// "Invalid fingerprint ID. Weapon non-functional",
	"Doelwit nodig",					// "Target aquired",
	"Pad geblokkeerd",				// "Path Blocked",
	"Geld Storten/Opnemen",		//Help text over the $ button on the Single Merc Panel ("Deposit/Withdraw Money")
	"Niemand heeft EHBO nodig.",			// "No one needs first aid.",
	"Vast.",						// Short form of JAMMED, for small inv slots
	"Kan daar niet heen.",				// used ( now ) for when we click on a cliff
	"Persoon weigert weg te gaan.",
	// In the following message, '%s' would be replaced with a quantity of money (e.g. $200)
	"Ben je het eens met %s?",					// "Do you agree to pay %s?",
	"Wil je kostenloze medische hulp?",			// "Accept free medical treatment?",
	"Wil je trouwen met Daryl?",					// "Agree to marry Daryl?",
	"Slot Ring Paneel",						// "Key Ring Panel",
	"Dat kan niet met een EPC.",					// "You cannot do that with an EPC.",
	"Krott sparen?",							// "Spare Krott?",
	"Buiten wapenbereik",						// "Out of weapon range",
	"Mijnwerker",							// "Miner",
	"Voertuig kan alleen tussen sectors reizen",		// "Vehicle can only travel between sectors",
	"Nu geen Auto-EHBO mogelijk",				// "Can't autobandage right now",
	"Pad Geblokkeerd voor %s",					// "Path Blocked for %s",
	"Je huurlingen, gevangen door Deidranna's leger, zitten hier opgesloten!",
	"Slot geraakt",							// "Lock hit",
	"Slot vernielt",							// "Lock destroyed",
	"Iemand anders probeert deze deur te gebruiken.",		// "Somebody else is trying to use this door.",
	"Gezondheid: %d/%d\nBrandstof: %d/%d",				// "Health: %d/%d\nFuel: %d/%d",
	"%s kan %s niet zien.",					// Cannot see person trying to talk to
};

//Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different circumstances in the "exiting sector" interface.
static const ST::string s_dut_pExitingSectorHelpText[pExitingSectorHelpText_SIZE] =
{
	//Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box is checked.
	"Als aangekruist, dan wordt de aanliggende sector meteen geladen.",
	"Als aangekruist, dan worden de huurlingen automatisch op de\nkaart geplaatst rekening houdend met reistijden.",

	//If you attempt to leave a sector when you have multiple squads in a hostile sector.
	"Deze sector is door de vijand bezet en huurlingen kun je niet achterlaten.\nJe moet deze situatie oplossen voor het laden van andere sectors.",

	//Because you only have one squad in the sector, and the "move all" option is checked, the "go to sector" option is locked to on.
	//The helptext explains why it is locked.
	"Als de overgebleven huurlingen uit deze sector trekken,\nwordt de aanliggende sector onmiddellijk geladen.",
	"Als de overgebleven huurlingen uit deze sector trekken,\nwordt je automatisch in het landkaartscherm geplaatst,\nrekening houdend met de reistijd van je huurlingen.",

	//If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being escorted.  The "single" button is disabled.
	"%s moet geëscorteerd worden door jouw huurlingen\nen kan de sector niet alleen verlaten.",

	//If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc will be prohibited from leaving alone.
	//There are several strings depending on the gender of the merc and how many EPCs are in the squad.
	//DO NOT USE THE NEWLINE HERE AS IT IS USED FOR BOTH HELPTEXT AND SCREEN MESSAGES!
	"%s kan de sector niet alleen verlaten omdat hij %s escorteert.",	//male singular
	"%s kan de sector niet alleen verlaten omdat zij %s escorteert.",	//female singular
	"%s kan de sector niet alleen verlaten omdat hij meerdere karakters escorteert.",	//male plural
	"%s kan de sector niet alleen verlaten omdat zij meerdere karakters escorteert.",	//female plural

	//If one or more of your mercs in the selected squad aren't in range of the traversal area, then the  "move all" option is disabled,
	//and this helptext explains why.
	"Al je huurlingen moeten in de buurt zijn om het team te laten reizen.",

	//Standard helptext for single movement.  Explains what will happen (splitting the squad)
	"Als aangekruist, dan zal %s alleen verder reizen\nen automatisch bij een uniek team gevoegd worden.",

	//Standard helptext for all movement.  Explains what will happen (moving the squad)
	"Als aangekruist, dan zal je geselecteerde\nteam verder reizen, de sector verlatend.",

	//This strings is used BEFORE the "exiting sector" interface is created.  If you have an EPC selected and you attempt to tactically
	//traverse the EPC while the escorting mercs aren't near enough (or dead, dying, or unconscious), this message will appear and the
	//"exiting sector" interface will not appear.  This is just like the situation where
	//This string is special, as it is not used as helptext.  Do not use the special newline character (\n) for this string.
	"%s wordt geëscorteerd door jouw huurlingen en kan de sector niet alleen verlaten. Je huurlingen moeten eerst in de buurt zijn.",
};



static const ST::string s_dut_pRepairStrings[pRepairStrings_SIZE] =
{
	"Items", 		// tell merc to repair items in inventory
	"SAM-Stelling", 		// tell merc to repair SAM site - SAM is an acronym for Surface to Air Missile
	"Stop", 		// cancel this menu
	"Robot", 		// repair the robot
};


// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."

static const ST::string s_dut_sPreStatBuildString[sPreStatBuildString_SIZE] =
{
	"verliest", 	// the merc has lost a statistic
	"krijgt", 		// the merc has gained a statistic
	"punt voor",	// singular
	"punten voor",	// plural
	"niveau voor",	// singular
	"niveaus voor",	// plural
};

static const ST::string s_dut_sStatGainStrings[sStatGainStrings_SIZE] =
{
	"gezondheid.",
	"beweeglijkheid.",
	"handigheid.",
	"wijsheid.",
	"medisch kunnen.",
	"explosieven.",
	"technisch kunnen.",
	"trefzekerheid.",
	"ervaring.",
	"kracht.",
	"leiderschap.",
};


static const ST::string s_dut_pHelicopterEtaStrings[pHelicopterEtaStrings_SIZE] =
{
	"Totale Afstand: ", 			// total distance for helicopter to travel
	" Veilig: ", 			// distance to travel to destination
	" Onveilig:", 			// distance to return from destination to airport
	"Totale Kosten: ", 		// total cost of trip by helicopter
	"Aank: ", 				// ETA is an acronym for "estimated time of arrival"
	"Helikopter heeft weinig brandstof en moet landen in vijandelijk gebied!",	// warning that the sector the helicopter is going to use for refueling is under enemy control ->
	"Passagiers: ",
	"Selecteer Skyrider of Aanvoer Drop-plaats?",			// "Select Skyrider or the Arrivals Drop-off?",
	"Skyrider",
	"Aanvoer",						// "Arrivals",
};

static const ST::string s_dut_sMapLevelString = "Subniv.:"; // what level below the ground is the player viewing in mapscreen ("Sublevel:")

static const ST::string s_dut_gsLoyalString = "%d%% Loyaal"; // the loyalty rating of a town ie : Loyal 53%


// error message for when player is trying to give a merc a travel order while he's underground.
static const ST::string s_dut_gsUndergroundString = "kan geen reisorders ondergronds ontvangen.";

static const ST::string s_dut_gsTimeStrings[gsTimeStrings_SIZE] =
{
	"u",				// hours abbreviation
	"m",				// minutes abbreviation
	"s",				// seconds abbreviation
	"d",				// days abbreviation
};

// text for the various facilities in the sector

static const ST::string s_dut_sFacilitiesStrings[sFacilitiesStrings_SIZE] =
{
	"Geen",
	"Ziekenhuis",
	"Industrie",
	"Gevangenis",
	"Krijgsmacht",
	"Vliegveld",
	"Schietterrein",		// a field for soldiers to practise their shooting skills
};

// text for inventory pop up button

static const ST::string s_dut_pMapPopUpInventoryText[pMapPopUpInventoryText_SIZE] =
{
	"Inventaris",
	"OK",
};

// town strings

static const ST::string s_dut_pwTownInfoStrings[pwTownInfoStrings_SIZE] =
{
	"Grootte",					// size of the town in sectors
	"Gezag", 					// how much of town is controlled
	"Verboden Mijn", 				// mine associated with this town
	"Loyaliteit",					// the loyalty level of this town
	"Voorzieningen", 				// main facilities in this town
	"Training Burgers",				// state of civilian training in town
	"Militie", 					// the state of the trained civilians in the town
};

// Mine strings

static const ST::string s_dut_pwMineStrings[pwMineStrings_SIZE] =
{
	"Mijn",					// 0
	"Zilver",
	"Goud",
	"Dagelijkse prod.",
	"Mogelijke prod.",
	"Verlaten",				// 5
	"Gesloten",
	"Raakt Op",
	"Produceert",
	"Status",
	"Prod. Tempo",
	"Ertstype",				// 10
	"Gezag Dorp",
	"Loyaliteit Dorp",
};

// blank sector strings

static const ST::string s_dut_pwMiscSectorStrings[pwMiscSectorStrings_SIZE] =
{
	"Vijandelijke troepen",
	"Sector",
	"# Items",
	"Onbekend",
	"Gecontrolleerd",
	"Ja",
	"Nee",
};

// error strings for inventory

static const ST::string s_dut_pMapInventoryErrorString[pMapInventoryErrorString_SIZE] =
{
	"Kan huurling niet selecteren.",		//MARK CARTER
	"%s is niet in de sector om dat item te pakken.",
	"Tijdens gevechten moet je items handmatig oppakken.",
	"Tijdens gevechten moet je items handmatig neerleggen.",
	"%s is niet in de sector om dat item neer te leggen.",
};

static const ST::string s_dut_pMapInventoryStrings[pMapInventoryStrings_SIZE] =
{
	"Locatie", 		// sector these items are in
	"Aantal Items", 		// total number of items in sector
};


// movement menu text

static const ST::string s_dut_pMovementMenuStrings[pMovementMenuStrings_SIZE] =
{
	"Huurlingen in Sector %s", 	// title for movement box
	"Teken Reisroute", 		// done with movement menu, start plotting movement
	"Stop", 		// cancel this menu
	"Anders",		// title for group of mercs not on squads nor in vehicles
};


static const ST::string s_dut_pUpdateMercStrings[pUpdateMercStrings_SIZE] =
{
	"Oeps:", 			// an error has occured
	"Contract Huurling verlopen:", 	// this pop up came up due to a merc contract ending
	"Huurling Taak Volbracht:", // this pop up....due to more than one merc finishing assignments
	"Huurling weer aan het Werk:", // this pop up ....due to more than one merc waking up and returing to work
	"Huurling zegt Zzzzzzz:", // this pop up ....due to more than one merc being tired and going to sleep
	"Contract Loopt Bijna Af:", 	// this pop up came up due to a merc contract ending
};

// map screen map border buttons help text

static const ST::string s_dut_pMapScreenBorderButtonHelpText[pMapScreenBorderButtonHelpText_SIZE] =
{
	"Toon Dorpen (|W)",
	"Toon |Mijnen",
	"Toon |Teams & Vijanden",
	"Toon Luchtruim (|A)",
	"Toon |Items",
	"Toon Milities & Vijanden (|Z)",
};


static const ST::string s_dut_pMapScreenBottomFastHelp[pMapScreenBottomFastHelp_SIZE] =
{
	"|Laptop",
	"Tactisch (|E|s|c)",
	"|Opties",
	"TijdVersneller (|+)", 	// time compress more
	"TijdVersneller (|-)", 	// time compress less
	"Vorig Bericht (|U|p)\nVorige Pagina (|P|g|U|p)", 	// previous message in scrollable list
	"Volgend Bericht (|D|o|w|n)\nVolgende pagina (|P|g|D|n)", 	// next message in the scrollable list
	"Start/Stop Tijd (|S|p|a|c|e)",	// start/stop time compression
};

static const ST::string s_dut_pMapScreenBottomText = "Huidig Saldo"; // current balance in player bank account

static const ST::string s_dut_pMercDeadString = "%s is dood.";


static const ST::string s_dut_pDayStrings = "Dag";

// the list of email sender names

static const ST::string s_dut_pSenderNameList[pSenderNameList_SIZE] =
{
	"Enrico",
	"Psych Pro Inc",
	"Help Desk",
	"Psych Pro Inc",
	"Speck",
	"R.I.S.",		//5
	"Barry",
	"Blood",
	"Lynx",
	"Grizzly",
	"Vicki",		//10
	"Trevor",
	"Grunty",
	"Ivan",
	"Steroid",
	"Igor",		//15
	"Shadow",
	"Red",
	"Reaper",
	"Fidel",
	"Fox",		//20
	"Sidney",
	"Gus",
	"Buns",
	"Ice",
	"Spider",		//25
	"Cliff",
	"Bull",
	"Hitman",
	"Buzz",
	"Raider",		//30
	"Raven",
	"Static",
	"Len",
	"Danny",
	"Magic",
	"Stephan",
	"Scully",
	"Malice",
	"Dr.Q",
	"Nails",
	"Thor",
	"Scope",
	"Wolf",
	"MD",
	"Meltdown",
	//----------
	"M.I.S. Verzekeringen",
	"Bobby Rays",
	"Kingpin",
	"John Kulba",
	"A.I.M.",
};


// new mail notify string
static const ST::string s_dut_pNewMailStrings = "Je hebt nieuwe berichten...";


// confirm player's intent to delete messages

static const ST::string s_dut_pDeleteMailStrings[pDeleteMailStrings_SIZE] =
{
	"Bericht verwijderen?",
	"ONGELEZEN bericht(en) verwijderen?",
};


// the sort header strings

static const ST::string s_dut_pEmailHeaders[pEmailHeaders_SIZE] =
{
	"Van:",
	"Subject:",
	"Dag:",
};

// email titlebar text
static const ST::string s_dut_pEmailTitleText = "Postvak";


// the financial screen strings
static const ST::string s_dut_pFinanceTitle = "Account Plus"; // the name we made up for the financial program in the game

static const ST::string s_dut_pFinanceSummary[pFinanceSummary_SIZE] =
{
	"Credit:", 				// credit (subtract from) to player's account
	"Debet:", 				// debit (add to) to player's account
	"Saldo Gisteren:",
	"Stortingen Gisteren:",
	"Uitgaven Gisteren:",
	"Saldo Eind van de Dag:",
	"Saldo Vandaag:",
	"Stortingen Vandaag:",
	"Uitgaven Vandaag:",
	"Huidig Saldo:",
	"Voorspelde Inkomen:",
	"Geschat Saldo:", 	// projected balance for player for tommorow
};


// headers to each list in financial screen

static const ST::string s_dut_pFinanceHeaders[pFinanceHeaders_SIZE] =
{
	"Dag", 			// the day column
	"Credit", 			// the credits column (to ADD money to your account)
	"Debet",			// the debits column (to SUBTRACT money from your account)
	"Transactie", 		// transaction type - see TransactionText below
	"Saldo", 			// balance at this point in time
	"Pag.", 			// page number
	"Dag(en)", 		// the day(s) of transactions this page displays
};


static const ST::string s_dut_pTransactionText[pTransactionText_SIZE] =
{
	"Toegenomen Interest",			// interest the player has accumulated so far
	"Anonieme Storting",
	"Transactiekosten",
	"%s ingehuurd van AIM", // Merc was hired
	"Bobby Ray's Wapenhandel", 		// Bobby Ray is the name of an arms dealer
	"Rekeningen Voldaan bij M.E.R.C.",
	"Medische Storting voor %s", 		// medical deposit for merc
	"IMP Profiel Analyse", 		// IMP is the acronym for International Mercenary Profiling
	"Verzekering Afgesloten voor %s",
	"Verzekering Verminderd voor %s",
	"Verzekering Verlengd voor %s", 				// johnny contract extended
	"Verzekering Afgebroken voor %s",
	"Verzekeringsclaim voor %s", 		// insurance claim for merc
	"Contract %s verl. met 1 dag.", 		// entend mercs contract by a day
	"Contract %s verl. met 1 week.",
	"Contract %s verl. met 2 weken.",
	"Inkomen Mijn",
	"",						//String nuked
	"Gekochte Bloemen",
	"Volledige Medische Vergoeding voor %s",
	"Gedeeltelijke Medische Vergoeding voor %s",
	"Geen Medische Vergoeding voor %s",
	"Betaling aan %s",			// %s is the name of the npc being paid
	"Maak Geld over aan %s", 		// transfer funds to a merc
	"Maak Geld over van %s", 		// transfer funds from a merc
	"Rust militie uit in %s",		// initial cost to equip a town's militia
	"Items gekocht van %s.",		//is used for the Shop keeper interface.  The dealers name will be appended to the end of the string.
	"%s heeft geld gestort.",
};

// helicopter pilot payment

static const ST::string s_dut_pSkyriderText[pSkyriderText_SIZE] =
{
	"Skyrider is $%d betaald.", 			// skyrider was paid an amount of money
	"Skyrider heeft $%d tegoed.", 		// skyrider is still owed an amount of money
	"Skyrider heeft geen passagiers. Als je huurlingen in deze sector wil vervoeren, wijs ze dan eerst toe aan Voertuig/Helikopter.",
};


// strings for different levels of merc morale

static const ST::string s_dut_pMoralStrings[pMoralStrings_SIZE] =
{
	"Super",
	"Goed",
	"Stabiel",
	"Mager",
	"Paniek",
	"Slecht",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.
static const ST::string s_dut_str_left_equipment   = "%s's uitrusting is nu beschikbaar in %s (%s).";

// Status that appears on the Map Screen

static const ST::string s_dut_pMapScreenStatusStrings[pMapScreenStatusStrings_SIZE] =
{
	"Gezondheid",
	"Energie",
	"Moraal",
	"Conditie",	// the condition of the current vehicle (its "health")
	"Brandstof",	// the fuel level of the current vehicle (its "energy")
};


static const ST::string s_dut_pMapScreenPrevNextCharButtonHelpText[pMapScreenPrevNextCharButtonHelpText_SIZE] =
{
	"Vorige Huurling (|L|e|f|t)", 			// previous merc in the list
	"Volgende Huurling (|R|i|g|h|t)", 				// next merc in the list
};


static const ST::string s_dut_pEtaString = "aank:"; // eta is an acronym for Estimated Time of Arrival

static const ST::string s_dut_pTrashItemText[pTrashItemText_SIZE] =
{
	"Je bent het voor altijd kwijt. Zeker weten?", 	// do you want to continue and lose the item forever
	"Dit item ziet er HEEL belangrijk uit. Weet je HEEL, HEEL zeker dat je het wil weggooien?", // does the user REALLY want to trash this item

};


static const ST::string s_dut_pMapErrorString[pMapErrorString_SIZE] =
{
	"Team kan niet verder reizen met een slapende huurling.",

//1-5
	"Verplaats het team eerst bovengronds.",
	"Reisorders? Het is vijandig gebied!",
	"Om te verplaatsen moeten huurlingen eerst toegewezen worden aan een team of voertuig.",
	"Je hebt nog geen team-leden.", // you have no members, can't do anything
	"Huurling kan order niet opvolgen.", // merc can't comply with your order
//6-10
	"%s heeft een escorte nodig. Plaats hem in een team.", // merc can't move unescorted .. for a male
	"%s heeft een escorte nodig. Plaats haar in een team.", // for a female
	"Huurling is nog niet in Arulco aangekomen!",
	"Het lijkt erop dat er eerst nog contractbesprekingen gehouden moeten worden.",
	"",
//11-15
	"Reisorders? Er is daar een gevecht gaande!",
	"Je bent in een hinderlaag gelokt van Bloodcats in sector %s!",
	"Je bent in sector I16 iets binnengelopen dat lijkt op het hol van een bloodcat!",
	"",
	"De SAM-stelling in %s is overgenomen.",
//16-20
	"De mijn in %s is overgenomen. Je dagelijkse inkomen is gereduceerd tot %s per dag.",
	"De vijand heeft sector %s onbetwist overgenomen.",
	"Tenminste een van je huurlingen kan niet meedoen met deze opdracht.",
	"%s kon niet meedoen met %s omdat het al vol is",
	"%s kon niet meedoen met %s omdat het te ver weg is.",
//21-25
	"De mijn in %s is buitgemaakt door Deidranna's troepen!",
	"Deidranna's troepen zijn net de SAM-stelling in %s binnengevallen",
	"Deidranna's troepen zijn net %s binnengevallen",
	"Deidranna's troepen zijn gezien in %s.",
	"Deidranna's troepen hebben zojuist %s overgenomen.",
//26-30
	"Tenminste één huurling kon niet tot slapen gebracht worden.",
	"Tenminste één huurling kon niet wakker gemaakt worden.",
	"De Militie verschijnt niet totdat hun training voorbij is.",
	"%s kan geen reisorders gegeven worden op dit moment.",
	"Milities niet binnen de stadsgrenzen kunnen niet verplaatst worden naar een andere sector.",
//31-35
	"Je kunt geen militie in %s hebben.",
	"Een voertuig kan niet leeg rijden!",
	"%s is te gewond om te reizen!",
	"Je moet het museum eerst verlaten!",
	"%s is dood!",
//36-40
	"%s kan niet wisselen naar %s omdat het onderweg is",
	"%s kan het voertuig op die manier niet in",
	"%s kan zich niet aansluiten bij %s",
	"Totdat je nieuwe huurlingen in dienst neemt, kan de tijd niet versneld worden!",
	"Dit voertuig kan alleen over wegen rijden!",
//41-45
	"Je kunt geen reizende huurlingen opnieuw toewijzen",
	"Voertuig zit zonder brandstof!",
	"%s is te moe om te reizen.",
	"Niemand aan boord is in staat om het voertuig te besturen.",
	"Eén of meer teamleden kunnen zich op dit moment niet verplaatsen.",
//46-50
	"Eén of meer leden van de ANDERE huurlingen kunnen zich op dit moment niet verplaatsen.",
	"Voertuig is te beschadigd!",
	"Let op dat maar twee huurlingen milities in een sector mogen trainen.",
	"De robot kan zich zonder bediening niet verplaatsen. Plaats ze in hetzelfde team.",
};


// help text used during strategic route plotting
static const ST::string s_dut_pMapPlotStrings[pMapPlotStrings_SIZE] =
{
	"Klik nogmaals op de bestemming om de route te bevestigen, of klik op een andere sector om meer routepunten te plaatsen.",
	"Route bevestigd.",
	"Bestemming onveranderd.",
	"Reis afgebroken.",
	"Reis verkort.",
};


// help text used when moving the merc arrival sector
static const ST::string s_dut_pBullseyeStrings[pBullseyeStrings_SIZE] =
{
	"Klik op de sector waar de huurlingen in plaats daarvan moeten arriveren.",
	"OK. Arriverende huurlingen worden afgezet in %s",
	"Huurlingen kunnen hier niet ingevlogen worden, het luchtruim is onveilig!",
	"Afgebroken. Aankomst-sector onveranderd",
	"Luchtruim boven %s is niet langer veilig! Aankomst-sector is verplaatst naar %s.",
};


// help text for mouse regions

static const ST::string s_dut_pMiscMapScreenMouseRegionHelpText[pMiscMapScreenMouseRegionHelpText_SIZE] =
{
	"Naar Inventaris (|E|n|t|e|r)",
	"Gooi Item Weg",
	"Verlaat Inventaris (|E|n|t|e|r)",
};


static const ST::string s_dut_str_he_leaves_where_drop_equipment  = "Laat %s zijn uitrusting achterlaten waar hij nu is (%s) of later in %s (%s) bij het nemen van de vlucht uit Arulco?";
static const ST::string s_dut_str_she_leaves_where_drop_equipment = "Laat %s haar uitrusting achterlaten waar ze nu is (%s) of later in %s (%s) bij het nemen van de vlucht uit Arulco?";
static const ST::string s_dut_str_he_leaves_drops_equipment       = "%s gaat binnenkort weg en laat zijn uitrusting achter in %s.";
static const ST::string s_dut_str_she_leaves_drops_equipment      = "%s gaat binnenkort weg en laat haar uitrusting achter in %s.";


// Text used on IMP Web Pages

static const ST::string s_dut_pImpPopUpStrings[pImpPopUpStrings_SIZE] =
{
	"Ongeldige Autorisatiecode",
	"Je wil het gehele persoonlijkheidsonderzoek te herstarten. Zeker weten?",
	"Vul alsjeblieft de volledige naam en geslacht in",
	"Voortijdig onderzoek van je financiële status wijst uit dat je een persoonlijksheidsonderzoek niet kunt betalen.",
	"Geen geldige optie op dit moment.",
	"Om een nauwkeurig profiel te maken, moet je ruimte hebben voor tenminste één teamlid.",
	"Profiel is al gemaakt.",
};


// button labels used on the IMP site

static const ST::string s_dut_pImpButtonText[pImpButtonText_SIZE] =
{
	"Info", 			// about the IMP site ("About Us")
	"BEGIN", 			// begin profiling ("BEGIN")
	"Persoonlijkheid", 		// personality section ("Personality")
	"Eigenschappen", 		// personal stats/attributes section ("Attributes")
	"Portret", 			// the personal portrait selection ("Portrait")
	"Stem %d", 			// the voice selection ("Voice %d")
	"OK", 			// done profiling ("Done")
	"Opnieuw", 		// start over profiling ("Start Over")
	"Ja, ik kies het geselecteerde antwoord.",		// ("Yes, I choose the highlighted answer.")
	"Ja",
	"Nee",
	"OK", 			// finished answering questions
	"Vor.", 			// previous question..abbreviated form
	"Vol.", 			// next question
	"JA ZEKER.", 		// yes, I am certain ("YES, I AM.")
	"NEE, IK WIL OPNIEUW BEGINNEN.",	// no, I want to start over the profiling process ("NO, I WANT TO START OVER.")
	"JA, ZEKER.",		// ("YES, I DO.")
	"NEE",
	"Terug", 					// back one page
	"Stop", 					// cancel selection
	"Ja, zeker weten.",			// ("Yes, I am certain.")
	"Nee, laat me nog eens kijken.",	// ("No, let me have another look.")
	"Registratie", 			// the IMP site registry..when name and gender is selected
	"Analyseren", 			// analyzing your profile results
	"OK",
	"Stem",				// "Voice"
	"Specialties",			// "Specialties" - the skills selection screen
};

static const ST::string s_dut_pExtraIMPStrings[pExtraIMPStrings_SIZE] =
{
	"Selecteer Persoonlijkheid om eigenlijke onderzoek te starten.",
	"Nu het onderzoek compleet is, selecteer je eigenschappen.",
	"Nu de eigenschappen gekozen zijn, kun je verder gaan met de portretselectie.",
	"Selecteer de stem die het best bij je past om het proces te voltooien.",
};

static const ST::string s_dut_gzIMPSkillTraitsText[gzIMPSkillTraitsText_SIZE] =
{
	s_dut_gzMercSkillText[1],
	s_dut_gzMercSkillText[2],
	s_dut_gzMercSkillText[3],
	s_dut_gzMercSkillText[4],
	s_dut_gzMercSkillText[5],
	s_dut_gzMercSkillText[6],
	s_dut_gzMercSkillText[7],
	s_dut_gzMercSkillText[8],
	s_dut_gzMercSkillText[9],
	s_dut_gzMercSkillText[10],
	s_dut_gzMercSkillText[13],
	s_dut_gzMercSkillText[14],
	s_dut_gzMercSkillText[15],
	s_dut_gzMercSkillText[12],

	s_dut_gzMercSkillText[0],
	"I.M.P. Specialties",
};

static const ST::string s_dut_pFilesTitle = "Bestanden Bekijken"; // ("File Viewer")

static const ST::string s_dut_pFilesSenderList[pFilesSenderList_SIZE] =
{
	"Int. Verslag",		// the recon report sent to the player. Recon is an abbreviation for reconissance
	"Intercept.#1",		// first intercept file .. Intercept is the title of the organization sending the file...similar in function to INTERPOL/CIA/KGB..refer to fist record in files.txt for the translated title
	"Intercept.#2",		// second intercept file
	"Intercept.#3",		// third intercept file
	"Intercept.#4",		// fourth intercept file ("Intercept #4")
	"Intercept.#5",		// fifth intercept file
	"Intercept.#6",		// sixth intercept file
};

// Text having to do with the History Log
static const ST::string s_dut_pHistoryTitle = "Geschiedenis";

static const ST::string s_dut_pHistoryHeaders[pHistoryHeaders_SIZE] =
{
	"Dag", 			// the day the history event occurred
	"Pag.", 			// the current page in the history report we are in
	"Dag", 			// the days the history report occurs over
	"Locatie", 		// location (in sector) the event occurred
	"Geb.", 			// the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
static const ST::string s_dut_pHistoryStrings[pHistoryStrings_SIZE] =
{
	"",										// leave this line blank
	//1-5
	"%s ingehuurd via A.I.M.",						// merc was hired from the aim site
	"%s ingehuurd via M.E.R.C.",						// merc was hired from the merc site
	"%s gedood.", 								// merc was killed
	"Facturen betaald bij M.E.R.C.",					// paid outstanding bills at MERC
	"Opdracht van Enrico Chivaldori geaccepteerd.",		// ("Accepted Assignment From Enrico Chivaldori")
	//6-10
	"IMP Profiel Klaar",							// ("IMP Profile Generated")
	"Verzekeringspolis gekocht voor %s.",				// insurance contract purchased
	"Verzekeringspolis afgebroken van %s.",				// insurance contract canceled
	"Uitbetaling Verzekeringspolis %s.",				// insurance claim payout for merc
	"%s's contract verlengd met 1 dag.",				// Extented "mercs name"'s for a day
	//11-15
	"%s's contract verlengd met 1 week.",				// Extented "mercs name"'s for a week
	"%s's contract verlengd met 2 weken.",				// Extented "mercs name"'s 2 weeks
	"%s is ontslagen.",							// "merc's name" was dismissed.
	"%s gestopt.",								// "merc's name" quit.
	"zoektocht gestart.",							// a particular quest started
	//16-20
	"zoektocht afgesloten.",						// ("quest completed.")
	"Gepraat met hoofdmijnwerker van %s",				// talked to head miner of town
	"%s bevrijd",								// ("Liberated %s")
	"Vals gespeeld",								// ("Cheat Used")
	"Voedsel zou morgen in Omerta moeten zijn",			// ("Food should be in Omerta by tomorrow")
	//21-25
	"%s weggegaan, wordt Daryl Hick's vrouw",			// ("%s left team to become Daryl Hick's wife")
	"%s's contract afgelopen.",						// ("%s's contract expired.")
	"%s aangenomen.",							// ("%s was recruited.")
	"Enrico klaagde over de voortgang",				// ("Enrico complained about lack of progress")
	"Strijd gewonnen",							// ("Battle won")
	//26-30
	"%s mijn raakt uitgeput",						// ("%s mine started running out of ore")
	"%s mijn is uitgeput",							// ("%s mine ran out of ore")
	"%s mijn is gesloten",							// ("%s mine was shut down")
	"%s mijn heropend",							// ("%s mine was reopened")
	"Info verkregen over gevangenis Tixa.",				// ("Found out about a prison called Tixa.")
	//31-35
	"Van geheime wapenfabriek gehoord, Orta genaamd.",		// ("Heard about a secret weapons plant called Orta.")
	"Onderzoeker in Orta geeft wat raketwerpers.",			// ("Scientist in Orta donated a slew of rocket rifles.")
	"Koningin Deidranna kickt op lijken.",				// ("Queen Deidranna has a use for dead bodies.")
	"Frank vertelde over knokwedstrijden in San Mona.",		// ("Frank talked about fighting matches in San Mona.")
	"Een patiënt dacht dat ie iets in de mijnen zag.",		// ("A patient thinks he saw something in the mines.")
	//36-40
	"Pers. ontmoet; Devin - verkoopt explosieven.",		// ("Met someone named Devin - he sells explosives.")
	"Beroemde ex-AIM huurling Mike ontmoet!",			// ("Ran into the famous ex-AIM merc Mike!")
	"Tony ontmoet - handelt in wapens.",				// ("Met Tony - he deals in arms.")
	"Raketwerper gekregen van Serg. Krott.",				// ("Got a rocket rifle from Sergeant Krott.")
	"Kyle akte gegeven van Angel's leerwinkel.",			// ("Gave Kyle the deed to Angel's leather shop.")
	//41-45
	"Madlab bood aan robot te bouwen.",				// ("Madlab offered to build a robot.")
	"Gabby maakt superbrouwsel tegen beesten.",			// ("Gabby can make stealth concoction for bugs.")
	"Keith is er mee opgehouden.",					// ("Keith is out of business.")
	"Howard geeft Koningin Deidranna cyanide.",			// ("Howard provided cyanide to Queen Deidranna.")
	"Keith ontmoet - handelaar in Cambria.",				// ("Met Keith - all purpose dealer in Cambria.")
	//46-50
	"Howard ontmoet - medicijnendealer in Balime",			// ("Met Howard - deals pharmaceuticals in Balime")
	"Perko ontmoet - heeft reparatiebedrijfje.",			// ("Met Perko - runs a small repair business.")
	"Sam van Balime ontmoet - verkoopt ijzerwaren.",		// ("Met Sam of Balime - runs a hardware shop.")
	"Franz verkoopt elektronica en andere dingen.",		// ("Franz deals in electronics and other goods.")
	"Arnold runt reparatiezaak in Grumm.",				// ("Arnold runs a repair shop in Grumm.")
	//51-55
	"Fredo repareert elektronica in Grumm.",				// ("Fredo repairs electronics in Grumm.")
	"Van rijke vent in Balime donatie gekregen.",			// ("Received donation from rich guy in Balime.")
	"Schroothandelaar Jake ontmoet.",					// ("Met a junkyard dealer named Jake.")
	"Vaag iemand gaf ons elektronische sleutelkaart.",		// ("Some bum gave us an electronic keycard.")
	"Walter omgekocht om kelderdeur open te maken.",		// ("Bribed Walter to unlock the door to the basement.")
	//56-60
	"Als Dave gas heeft, geeft hij deze weg.",			// ("If Dave has gas, he'll provide free fillups.")
	"Geslijmd met Pablo.",							// ("Greased Pablo's palms.")
	"Kingpin bewaard geld in San Mona mine.",			// ("Kingpin keeps money in San Mona mine.")
	"%s heeft Extreme Fighting gewonnen",				// ("%s won Extreme Fighting match")
	"%s heeft Extreme Fighting verloren",				// ("%s lost Extreme Fighting match")
	//61-65
	"%s gediskwalificeerd v. Extreme Fighting",			// ("%s was disqualified in Extreme Fighting")
	"Veel geld gevonden in een verlaten mijn.",			// ("Found a lot of money stashed in the abandoned mine.")
	"Huurmoordenaar van Kingpin ontdekt.",				// ("Encountered assassin sent by Kingpin.")
	"Controle over sector verloren",					//ENEMY_INVASION_CODE ("Lost control of sector")
	"Sector verdedigd",							// ("Defended sector")
	//66-70
	"Strijd verloren",					//ENEMY_ENCOUNTER_CODE ("Lost battle")
	"Fatale val",						//ENEMY_AMBUSH_CODE ("Fatal ambush")
	"Vijandige val weggevaagd",				// ("Wiped out enemy ambush")
	"Aanval niet gelukt",					//ENTERING_ENEMY_SECTOR_CODE ("Unsuccessful attack")
	"Aanval gelukt!",					// ("Successful attack!")
	//71-75
	"Beesten vielen aan",					//CREATURE_ATTACK_CODE ("Creatures attacked")
	"Gedood door bloodcats",				//BLOODCAT_AMBUSH_CODE ("Killed by bloodcats")
	"Afgeslacht door bloodcats",				// ("Slaughtered bloodcats")
	"%s was gedood",						// ("%s was killed")
	"Carmen kop v.e. terrorist gegeven",		// ("Gave Carmen a terrorist's head")
	"Slay vertrok",						// ("Slay left")
	"%s vermoord",						// ("Killed %s")
};

static const ST::string s_dut_pHistoryLocations = "Nvt"; // N/A is an acronym for Not Applicable

// icon text strings that appear on the laptop

static const ST::string s_dut_pLaptopIcons[pLaptopIcons_SIZE] =
{
	"E-mail",
	"Web",
	"Financieel",
	"Dossiers",
	"Historie",
	"Bestanden",
	"Afsluiten",
	"sir-FER 4.0",			// our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added

static const ST::string s_dut_pBookMarkStrings[pBookMarkStrings_SIZE] =
{
	"A.I.M.",
	"Bobby Ray's",
	"I.M.P",
	"M.E.R.C.",
	"Mortuarium",
	"Bloemist",
	"Verzekering",
	"Stop",
};

// When loading or download a web page

static const ST::string s_dut_pDownloadString[pDownloadString_SIZE] =
{
	"Laden",
	"Herladen",
};

//This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

static const ST::string s_dut_gsAtmStartButtonText[gsAtmStartButtonText_SIZE] =
{
	"Info", 			// view stats of the merc
	"Inventaris", 			// view the inventory of the merc
	"Werk",
};

// Web error messages. Please use foreign language equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator

static const ST::string s_dut_pErrorStrings = "Periodieke verbinding met host. Houdt rekening met lange wachttijden.";


static const ST::string s_dut_pPersonnelString = "Huurlingen:"; // mercs we have


static const ST::string s_dut_pWebTitle = "sir-FER 4.0"; // our name for the version of the browser, play on company name


// The titles for the web program title bar, for each page loaded

static const ST::string s_dut_pWebPagesTitles[pWebPagesTitles_SIZE] =
{
	"A.I.M.",
	"A.I.M. Leden",
	"A.I.M. Portretten",		// a mug shot is another name for a portrait
	"A.I.M. Sorteer",
	"A.I.M.",
	"A.I.M. Veteranen",
	"A.I.M. Regelement",
	"A.I.M. Geschiedenis",
	"A.I.M. Links",
	"M.E.R.C.",
	"M.E.R.C. Rekeningen",
	"M.E.R.C. Registratie",
	"M.E.R.C. Index",
	"Bobby Ray's",
	"Bobby Ray's - Wapens",
	"Bobby Ray's - Munitie",
	"Bobby Ray's - Pantsering",
	"Bobby Ray's - Diversen",					//misc is an abbreviation for miscellaneous
	"Bobby Ray's - Gebruikt",
	"Bobby Ray's - Mail Order",
	"I.M.P.",
	"I.M.P.",
	"United Floral Service",
	"United Floral Service - Etalage",
	"United Floral Service - Bestelformulier",
	"United Floral Service - Kaart Etalage",
	"Malleus, Incus & Stapes Verzekeringen",
	"Informatie",
	"Contract",
	"Opmerkingen",
	"McGillicutty's Mortuarium",
	"URL niet gevonden.",
	"Bobby Ray's - Recentelijke Zendingen",
	"",
	"",
};

static const ST::string s_dut_pShowBookmarkString[pShowBookmarkString_SIZE] =
{
	"Sir-Help",
	"Klik opnieuw voor Bookmarks.",
};

static const ST::string s_dut_pLaptopTitles[pLaptopTitles_SIZE] =
{
	"E-Mail",
	"Bestanden bekijken",
	"Persoonlijk",
	"Boekhouder Plus",
	"Geschiedenis",
};

static const ST::string s_dut_pPersonnelDepartedStateStrings[pPersonnelDepartedStateStrings_SIZE] =
{
	//reasons why a merc has left.
	"Omgekomen tijdens gevechten",
	"Weggestuurd",
	"Getrouwd",
	"Contract Afgelopen",
	"Gestopt",
};
// personnel strings appearing in the Personnel Manager on the laptop

static const ST::string s_dut_pPersonelTeamStrings[pPersonelTeamStrings_SIZE] =
{
	"Huidig Team",
	"Vertrekken",
	"Dag. Kosten:",
	"Hoogste Kosten:",
	"Laagste Kosten:",
	"Omgekomen tijdens gevechten:",
	"Weggestuurd:",
	"Anders:",
};


static const ST::string s_dut_pPersonnelCurrentTeamStatsStrings[pPersonnelCurrentTeamStatsStrings_SIZE] =
{
	"Laagste",
	"Gemiddeld",
	"Hoogste",
};


static const ST::string s_dut_pPersonnelTeamStatsStrings[pPersonnelTeamStatsStrings_SIZE] =
{
	"GZND",
	"BEW",
	"HAN",
	"KRA",
	"LDR",
	"WIJ",
	"NIV",
	"TREF",
	"MECH",
	"EXPL",
	"MED",
};


// horizontal and vertical indices on the map screen

static const ST::string s_dut_pMapVertIndex[pMapVertIndex_SIZE] =
{
	"X",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
};

static const ST::string s_dut_pMapHortIndex[pMapHortIndex_SIZE] =
{
	"X",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"10",
	"11",
	"12",
	"13",
	"14",
	"15",
	"16",
};

static const ST::string s_dut_pMapDepthIndex[pMapDepthIndex_SIZE] =
{
	"",
	"-1",
	"-2",
	"-3",
};

// text that appears on the contract button

static const ST::string s_dut_pContractButtonString = "Contract";

// text that appears on the update panel buttons

static const ST::string s_dut_pUpdatePanelButtons[pUpdatePanelButtons_SIZE] =
{
	"Doorgaan",
	"Stop",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle

static ST::string s_dut_LargeTacticalStr[LargeTacticalStr_SIZE] =
{
	"Je bent verslagen in deze sector!",
	"De vijand, geen genade kennende, slacht ieder teamlid af!",
	"Je bewusteloze teamleden zijn gevangen genomen!",
	"Je teamleden zijn gevangen genomen door de vijand.",
};


//Insurance Contract.c
//The text on the buttons at the bottom of the screen.

static const ST::string s_dut_InsContractText[InsContractText_SIZE] =
{
	"Vorige",
	"Volgende",
	"OK",
	"Leeg",
};



//Insurance Info
// Text on the buttons on the bottom of the screen

static const ST::string s_dut_InsInfoText[InsInfoText_SIZE] =
{
	"Vorige",
	"Volgende",
};



//For use at the M.E.R.C. web site. Text relating to the player's account with MERC

static const ST::string s_dut_MercAccountText[MercAccountText_SIZE] =
{
	// Text on the buttons on the bottom of the screen
	"Autoriseer",
	"Thuis",
	"Rekening#:",
	"Huurl.",
	"Dagen",
	"Tarief",	//5
	"Prijs",
	"Totaal:",
	"Weet je zeker de betaling van %s te autoriseren?",		//the %s is a string that contains the dollar amount ( ex. "$150" )
};



//For use at the M.E.R.C. web site. Text relating a MERC mercenary


static const ST::string s_dut_MercInfo[MercInfo_SIZE] =
{
	"Vorige",
	"Huur",
	"Volgende",
	"Extra Info",
	"Thuis",
	"Ingehuurd",
	"Salaris:",
	"Per Dag",
	"Overleden",

	"Lijkt erop dat je teveel huurlingen wil recruteren. Limiet is 18.",
	"Niet beschikbaar",
};



// For use at the M.E.R.C. web site. Text relating to opening an account with MERC

static const ST::string s_dut_MercNoAccountText[MercNoAccountText_SIZE] =
{
	//Text on the buttons at the bottom of the screen
	"Open Rekening",
	"Afbreken",
	"Je hebt geen rekening. Wil je er één openen?",
};



// For use at the M.E.R.C. web site. MERC Homepage

static const ST::string s_dut_MercHomePageText[MercHomePageText_SIZE] =
{
	//Description of various parts on the MERC page
	"Speck T. Kline, oprichter en bezitter",
	"Om een rekening te open, klik hier",
	"Klik hier om rekening te bekijken",
	"Klik hier om bestanden in te zien",
	// The version number on the video conferencing system that pops up when Speck is talking
	"Speck Com v3.2",
};

// For use at MiGillicutty's Web Page.

static const ST::string s_dut_sFuneralString[sFuneralString_SIZE] =
{
	"McGillicutty's Mortuarium: Helpt families rouwen sinds 1983.",
	"Begrafenisondernemer en voormalig A.I.M. huurling Murray \"Pops\" McGillicutty is een kundig en ervaren begrafenisondernemer.",
	"Pops weet hoe moeilijk de dood kan zijn, in heel zijn leven heeft hij te maken gehad met de dood en sterfgevallen.",
	"McGillicutty's Mortuarium biedt een breed scala aan stervensbegeleiding, van een schouder om uit te huilen tot recontructie van misvormde overblijfselen.",
	"Laat McGillicutty's Mortuarium u helpen en laat uw dierbaren zacht rusten.",

	// Text for the various links available at the bottom of the page
	"STUUR BLOEMEN",
	"DOODSKIST & URN COLLECTIE",
	"CREMATIE SERVICE",
	"SERVICES",
	"ETIQUETTE",

	// The text that comes up when you click on any of the links ( except for send flowers ).
	"Helaas is deze pagina nog niet voltooid door een sterfgeval in de familie. Afhankelijk van de laatste wil en uitbetaling van de beschikbare activa wordt de pagina zo snel mogelijk voltooid.",
	"Ons medeleven gaat uit naar jou, tijdens deze probeerperiode. Kom nog eens langs.",
};

// Text for the florist Home page

static const ST::string s_dut_sFloristText[sFloristText_SIZE] =
{
	//Text on the button on the bottom of the page

	"Etalage",

	//Address of United Florist

	"\"We brengen overal langs\"",
	"1-555-SCENT-ME",
	"333 NoseGay Dr, Seedy City, CA USA 90210",
	"http://www.scent-me.com",

	// detail of the florist page

	"We zijn snel en efficiënt!",
	"Volgende dag gebracht, wereldwijd, gegarandeerd. Enkele beperkingen zijn van toepassing.",
	"Laagste prijs in de wereld, gegarandeerd!",
	"Toon ons een lagere geadverteerde prijs voor een regeling en ontvang gratis een dozijn rozen.",
	"Flora, Fauna & Bloemen sinds 1981.",
	"Onze onderscheiden ex-bommenwerperpiloten droppen je boeket binnen een tien kilometer radius van de gevraagde locatie. Altijd!",
	"Laat ons al je bloemenfantasieën waarmaken.",
	"Laat Bruce, onze wereldberoemde bloemist, de verste bloemen met de hoogste kwaliteit uit onze eigen kassen uitzoeken.",
	"En onthoudt, als we het niet hebben, kunnen we het kweken - Snel!",
};



//Florist OrderForm

static const ST::string s_dut_sOrderFormText[sOrderFormText_SIZE] =
{
	//Text on the buttons

	"Terug",
	"Verstuur",
	"Leeg",
	"Etalage",

	"Naam vh Boeket:",
	"Prijs:",			//5
	"Ordernummer:",
	"Bezorgingsdatum",
	"volgende dag",
	"komt wanneer het komt",
	"Locatie Bezorging",			//10
	"Extra Service",
	"Geplet Boeket($10)",
	"Zwarte Rozen ($20)",
	"Verlept Boeket($10)",
	"Fruitcake (indien beschikbaar)($10)",		//15
	"Persoonlijk Bericht:",
	"Wegens de grootte kaarten, mogen je berichten niet langer zijn dan 75 karakters.",
	"...of selecteer er één van de onze",

	"STANDAARDKAARTEN",
	"Factuurinformatie",	//20

	//The text that goes beside the area where the user can enter their name

	"Naam:",
};




//Florist Gallery.c

static const ST::string s_dut_sFloristGalleryText[sFloristGalleryText_SIZE] =
{
	//text on the buttons

	"Back",	//abbreviation for previous
	"Next",	//abbreviation for next

	"Klik op de selectie die je wil bestellen.",
	"Let op: er geldt een extra tarief van $10 voor geplette en verlepte boeketten.",

	//text on the button

	"Home",
};

//Florist Cards

static const ST::string s_dut_sFloristCards[sFloristCards_SIZE] =
{
	"Klik op je selectie",
	"Terug",
};



// Text for Bobby Ray's Mail Order Site

static const ST::string s_dut_BobbyROrderFormText[BobbyROrderFormText_SIZE] =
{
	"Bestelformulier",				//Title of the page
	"Hvl",					// The number of items ordered
	"Gewicht(%s)",			// The weight of the item
	"Itemnaam",				// The name of the item
	"Prijs unit",				// the item's weight
	"Totaal",				//5	// The total price of all of items of the same type
	"Sub-Totaal",				// The sub total of all the item totals added
	"Porto (Zie Bezorgloc.)",		// S&H is an acronym for Shipping and Handling
	"Eindtotaal",			// The grand total of all item totals + the shipping and handling
	"Bezorglocatie",
	"Verzendingssnelheid",			//10	// See below
	"Kosten (per %s.)",			// The cost to ship the items
	"Nacht-Express",			// Gets deliverd the next day
	"2 Werkdagen",			// Gets delivered in 2 days
	"Standaard Service",			// Gets delivered in 3 days
	"Order Leegmaken",//15			// Clears the order page
	"Accept. Order",			// Accept the order
	"Terug",				// text on the button that returns to the previous page
	"Home",				// Text on the button that returns to the home page
	"* Duidt op Gebruikte Items",		// Disclaimer stating that the item is used
	"Je kunt dit niet betalen.",		//20	// A popup message that to warn of not enough money
	"<GEEN>",				// Gets displayed when there is no valid city selected
	"Weet je zeker dat je de bestelling wil sturen naar %s?",		// A popup that asks if the city selected is the correct one
	"Gewicht Pakket**",			// Displays the weight of the package
	"** Min. Gew.",				// Disclaimer states that there is a minimum weight for the package
	"Zendingen",
};


// This text is used when on the various Bobby Ray Web site pages that sell items

static const ST::string s_dut_BobbyRText[BobbyRText_SIZE] =
{
	"Bestelling",				// Title
	// instructions on how to order
	"Klik op de item(s). Voor meer dan één, blijf dan klikken. Rechtsklikken voor minder. Als je alles geselecteerd hebt, dat je wil bestellen, ga dan naar het bestelformulier.",

	//Text on the buttons to go the various links

	"Vorige Items",		//
	"Wapens", 			//3
	"Munitie",			//4
	"Pantser",			//5
	"Diversen",			//6	//misc is an abbreviation for miscellaneous
	"Gebruikt",			//7
	"Meer Items",
	"BESTELFORMULIER",
	"Home",			//10

	//The following lines provide information on the items

	"Gewicht:",		// Weight of all the items of the same type
	"Kal:",			// the caliber of the gun
	"Mag:",			// number of rounds of ammo the Magazine can hold
	"Afs:",			// The range of the gun
	"Sch:",			// Damage of the weapon
	"ROF:",			// Weapon's Rate Of Fire, acronym ROF
	"Kost:",			// Cost of the item
	"Aanwezig:",			// The number of items still in the store's inventory
	"# Besteld:",		// The number of items on order
	"Beschadigd",			// If the item is damaged
	"SubTotaal:",			// The total cost of all items on order
	"* % Functioneel",		// if the item is damaged, displays the percent function of the item

	//Popup that tells the player that they can only order 10 items at a time

	"Verdraaid! Dit on-line bestelformulier accepteert maar 10 items per keer. Als je meer wil bestellen (en dat hopen we), plaats dan afzonderlijke orders en accepteer onze excuses.",

	// A popup that tells the user that they are trying to order more items then the store has in stock

	"Sorry. We hebben niet meer van die zaken in het magazijn. Probeer het later nog eens.",

	//A popup that tells the user that the store is temporarily sold out

	"Sorry, alle items van dat type zijn nu uitverkocht.",
};


// The following line is used on the Ammunition page.  It is used for help text
// to display how many items the player's merc has that can use this type of
// ammo.
static const ST::string s_dut_str_bobbyr_guns_num_guns_that_use_ammo = "Je team heeft %d wapen(s) gebruik makende van deze munitie";


// Text for Bobby Ray's Home Page

static const ST::string s_dut_BobbyRaysFrontText[BobbyRaysFrontText_SIZE] =
{
	//Details on the web site

	"Hier moet je zijn voor de nieuwste en beste wapens en militaire goederen",
	"We kunnen de perfecte oplossing vinden voor elke explosiebehoefte",
	"Gebruikte en opgeknapte items",

	//Text for the various links to the sub pages

	"Diversen",
	"WAPENS",
	"MUNITIE",		//5
	"PANTSER",

	//Details on the web site

	"Als wij het niet verkopen, dan kun je het nergens krijgen!",
	"Under construction",
};



// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug shot page

static const ST::string s_dut_AimSortText[AimSortText_SIZE] =
{
	"A.I.M. Leden",				// Title
	// Title for the way to sort
	"Sort. op:",

	//Text of the links to other AIM pages

	"Bekijk portretfotoindex van huurlingen",
	"Bekijk het huurlingendossier",
	"Bekijk de A.I.M. Veteranen"
};


// text to display how the entries will be sorted
static const ST::string s_dut_str_aim_sort_price        = "Prijs";
static const ST::string s_dut_str_aim_sort_experience   = "Ervaring";
static const ST::string s_dut_str_aim_sort_marksmanship = "Trefzekerheid";
static const ST::string s_dut_str_aim_sort_medical      = "Medisch";
static const ST::string s_dut_str_aim_sort_explosives   = "Explosieven";
static const ST::string s_dut_str_aim_sort_mechanical   = "Technisch";
static const ST::string s_dut_str_aim_sort_ascending    = "Oplopend";
static const ST::string s_dut_str_aim_sort_descending   = "Aflopend";


//Aim Policies.c
//The page in which the AIM policies and regulations are displayed

static const ST::string s_dut_AimPolicyText[AimPolicyText_SIZE] =
{
	// The text on the buttons at the bottom of the page

	"Previous",
	"AIM HomePage",
	"Index Regels",
	"Next",
	"Oneens",
	"Mee eens",
};



//Aim Member.c
//The page in which the players hires AIM mercenaries

// Instructions to the user to either start video conferencing with the merc, or to go the mug shot index

static const ST::string s_dut_AimMemberText[AimMemberText_SIZE] =
{
	"Klik Links",
	"voor Verbinding met Huurl.",
	"Klik Rechts",
	"voor Portretfotoindex.",
};

//Aim Member.c
//The page in which the players hires AIM mercenaries

static const ST::string s_dut_CharacterInfo[CharacterInfo_SIZE] =
{
	// the contract expenses' area

	"Tarief",
	"Contract",
	"een dag",
	"een week",
	"twee weken",

	// text for the buttons that either go to the previous merc,
	// start talking to the merc, or go to the next merc

	"Previous",
	"Contact",
	"Next",

	"Extra Info",				// Title for the additional info for the merc's bio
	"Actieve Leden", // Title of the page
	"Aanv. Uitrusting:",				// Displays the optional gear cost
	"MEDISCHE aanbetaling nodig",			// If the merc required a medical deposit, this is displayed
};


//Aim Member.c
//The page in which the player's hires AIM mercenaries

//The following text is used with the video conference popup

static const ST::string s_dut_VideoConfercingText[VideoConfercingText_SIZE] =
{
	"Contractkosten:",				//Title beside the cost of hiring the merc

	//Text on the buttons to select the length of time the merc can be hired

	"Een Dag",
	"Een Week",
	"Twee Weken",

	//Text on the buttons to determine if you want the merc to come with the equipment

	"Geen Uitrusting",
	"Koop Uitrusting",

	// Text on the Buttons

	"HUUR IN",			// to actually hire the merc
	"STOP",				// go back to the previous menu
	"VOORWAARDEN",				// go to menu in which you can hire the merc
	"OPHANGEN",			// stops talking with the merc
	"OK",
	"STUUR BERICHT",			// if the merc is not there, you can leave a message

	//Text on the top of the video conference popup

	"Video Conference met",
	"Verbinding maken. . .",

	"+ med. depo",			// Displays if you are hiring the merc with the medical deposit
};



//Aim Member.c
//The page in which the player hires AIM mercenaries

// The text that pops up when you select the TRANSFER FUNDS button

static const ST::string s_dut_AimPopUpText[AimPopUpText_SIZE] =
{
	"BEDRAG OVERGEBOEKT",	// You hired the merc
	"OVERMAKEN NIET MOGELIJK",			// Player doesn't have enough money, message 1
	"ONVOLDOENDE GELD",				// Player doesn't have enough money, message 2

	// if the merc is not available, one of the following is displayed over the merc's face

	"Op missie",
	"Laat a.u.b. bericht achter",
	"Overleden",

	//If you try to hire more mercs than game can support

	"Je team bestaat al uit 18 huurlingen.",

	"Opgenomen bericht",
	"Bericht opgenomen",
};


//AIM Link.c

static const ST::string s_dut_AimLinkText = "A.I.M. Links"; // The title of the AIM links page



//Aim History

// This page displays the history of AIM

static const ST::string s_dut_AimHistoryText[AimHistoryText_SIZE] =
{
	"A.I.M. Geschiedenis",					//Title

	// Text on the buttons at the bottom of the page

	"Previous",
	"Home",
	"A.I.M. Veteranen",
	"Next",
};


//Aim Mug Shot Index

//The page in which all the AIM members' portraits are displayed in the order selected by the AIM sort page.

static const ST::string s_dut_AimFiText[AimFiText_SIZE] =
{
	// displays the way in which the mercs were sorted

	"Prijs",
	"Ervaring",
	"Trefzekerheid",
	"Medisch",
	"Explosieven",
	"Technisch",

	// The title of the page, the above text gets added at the end of this text

	"A.I.M. Leden Oplopend Gesorteerd op %s",
	"A.I.M. Leden Aflopend Gesorteerd op %s",

	// Instructions to the players on what to do

	"Klik Links",
	"om Huurling te Selecteren",			//10
	"Klik Rechts",
	"voor Sorteeropties",

	// Gets displayed on top of the merc's portrait if they are...

	"Overleden",						//14
	"Op missie",
};



//AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer with AIM

static const ST::string s_dut_AimAlumniText[AimAlumniText_SIZE] =
{
	// Text of the buttons

	"PAG. 1",
	"PAG. 2",
	"PAG. 3",

	"A.I.M. Veteranen",	// Title of the page

	"OK",			// Stops displaying information on selected merc
};






//AIM Home Page

static const ST::string s_dut_AimScreenText[AimScreenText_SIZE] =
{
	// AIM disclaimers

	"A.I.M. en A.I.M.-logo zijn geregistreerde handelsmerken in de meeste landen.",
	"Dus denk er niet aan om ons te kopiëren.",
	"Copyright 1998-1999 A.I.M., Ltd.  All rights reserved.",

	//Text for an advertisement that gets displayed on the AIM page

	"United Floral Service",
	"\"We droppen overal\"",				//10
	"Doe het goed",
	"... de eerste keer",
	"Wapens en zo, als we het niet hebben, dan heb je het ook niet nodig.",
};


//Aim Home Page

static const ST::string s_dut_AimBottomMenuText[AimBottomMenuText_SIZE] =
{
	//Text for the links at the bottom of all AIM pages
	"Home",
	"Leden",
	"Veteranen",
	"Regels",
	"Geschiedenis",
	"Links",
};



//ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.

static const ST::string s_dut_SKI_Text[SKI_SIZE ] =
{
	"HANDELSWAAR OP VOORRAAD",		//Header for the merchandise available
	"PAG.",				//The current store inventory page being displayed
	"TOTALE KOSTEN",				//The total cost of the the items in the Dealer inventory area
	"TOTALE WAARDE",			//The total value of items player wishes to sell
	"EVALUEER",				//Button text for dealer to evaluate items the player wants to sell
	"TRANSACTIE",			//Button text which completes the deal. Makes the transaction.
	"OK",				//Text for the button which will leave the shopkeeper interface.
	"REP. KOSTEN",			//The amount the dealer will charge to repair the merc's goods
	"1 UUR",			// SINGULAR! The text underneath the inventory slot when an item is given to the dealer to be repaired
	"%d UREN",		// PLURAL!   The text underneath the inventory slot when an item is given to the dealer to be repaired
	"GEREPAREERD",		// Text appearing over an item that has just been repaired by a NPC repairman dealer
	"Er is geen ruimte meer.",	//Message box that tells the user there is no more room to put there stuff
	"%d MINUTEN",		// The text underneath the inventory slot when an item is given to the dealer to be repaired
	"Drop Item op Grond.",
};


static const ST::string s_dut_SkiMessageBoxText[SkiMessageBoxText_SIZE] =
{
	"Wil je %s aftrekken van je hoofdrekening om het verschil op te vangen?",
	"Niet genoeg geld. Je komt %s tekort",
	"Wil je %s aftrekken van je hoofdrekening om de kosten te dekken?",
	"Vraag de dealer om de transactie te starten",
	"Vraag de dealer om de gesel. items te repareren",
	"Einde conversatie",
	"Huidige Saldo",
};


//OptionScreen.c

static const ST::string s_dut_zOptionsText[zOptionsText_SIZE] =
{
	//button Text
	"Spel Bewaren",
	"Spel Laden",
	"Stop",
	"OK",

	//Text above the slider bars
	"Effecten",
	"Spraak",
	"Muziek",

	//Confirmation pop when the user selects..
	"Spel verlaten en terugkeren naar hoofdmenu?",

	"Je hebt of de Spraakoptie nodig of de ondertiteling.",
};


//SaveLoadScreen
static const ST::string s_dut_zSaveLoadText[zSaveLoadText_SIZE] =
{
	"Spel Bewaren",
	"Spel Laden",
	"Stop",
	"Bewaren Gesel.",
	"Laden Gesel.",

	"Spel Bewaren voltooid",
	"FOUT bij bewaren spel!",
	"Spel laden succesvol",
	"FOUT bij laden spel: \"%s\"",

	"De spelversie van het bewaarde spel verschilt van de huidige versie. Waarschijnlijk is het veilig om door te gaan.",
	"Weet je zeker dat je het opgeslagen spel met de naam \"%s\" wilt verwijderen?",

	"Aandacht:",
	"Poging om de opslag van een oudere versie te laden. Als u doorgaat, wordt de opslag automatisch bijgewerkt.",
	"Je hebt andere mods ingeschakeld dan waarmee de savegame is opgeslagen. De mods werken mogelijk niet correct.",
	"Doorgaan?",

	"Weet je zeker dat je het spel met de naam \"%s\" wil overschrijven?",

	"Bewaren...",			//When saving a game, a message box with this string appears on the screen

	"Normale Wapens",
	"Stapels Wapens",
	"Realistische stijl",
	"SF stijl",
	"Moeilijkheid",

	"Geen mod ingeschakeld",
	"Mods:"
};



//MapScreen
static const ST::string s_dut_zMarksMapScreenText[zMarksMapScreenText_SIZE] =
{
	"Kaartniveau",
	"Je hebt geen militie. Je moet stadsburgers trainen om een stadsmilitie te krijgen.",
	"Dagelijks Inkomen",
	"Huurling heeft levensverzekering",
	"%s is niet moe.",
	"%s is bezig en kan niet slapen",
	"%s is te moe, probeer het later nog eens.",
	"%s is aan het rijden.",
	"Team kan niet reizen met een slapende huurling.",

	// stuff for contracts
	"Je kunt wel het contract betalen, maar je hebt geen geld meer om de levensverzekering van de huurling te betalen.",
	"%s verzekeringspremie kost %s voor %d extra dag(en). Wil je betalen?",
	"Inventaris Sector",
	"Huurling heeft medische kosten.",

	// other items
	"Medici",		// people acting a field medics and bandaging wounded mercs
	"Patiënten", // people who are being bandaged by a medic
	"OK", // Continue on with the game after autobandage is complete
	"Stop", // Stop autobandaging of patients by medics now
	"%s heeft geen reparatie-kit.",
	"%s heeft geen medische kit.",
	"Er zijn nu niet genoeg mensen die getraind willen worden.",
	"%s is vol met milities.",
	"Huurling heeft eindig contract.",
	"Contract Huurling is niet verzekerd",
};


static const ST::string s_dut_pLandMarkInSectorString = "Team %d is heeft iemand ontdekt in sector %s";

// confirm the player wants to pay X dollars to build a militia force in town
static const ST::string s_dut_pMilitiaConfirmStrings[pMilitiaConfirmStrings_SIZE] =
{
	"Een stadsmilitie trainen kost $",		// telling player how much it will cost
	"Uitgave goedkeuren?", // asking player if they wish to pay the amount requested
	"je kunt dit niet betalen.", // telling the player they can't afford to train this town
	"Doorgaan met militie trainen %s (%s %d)?", // continue training this town?
	"Kosten $", // the cost in dollars to train militia
	"( J/N )",   // abbreviated yes/no
	"Stadsmilities trainen in %d sectors kost $ %d. %s", // cost to train sveral sectors at once
	"Je kunt de $%d niet betalen om de stadsmilitie hier te trainen.",
	"%s heeft een loyaliteit nodig van %d procent om door te gaan met milities trainen.",
	"Je kunt de militie in %s niet meer trainen.",
};


//Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom of the single merc panel
static const ST::string s_dut_gzMoneyWithdrawMessageText[gzMoneyWithdrawMessageText_SIZE] =
{
	"Je kunt maximaal $20.000 in één keer opnemen.",
	"Weet je zeker dat je %s wil storten op je rekening?",
};

static const ST::string s_dut_gzCopyrightText = "Copyright (C) 1999 Sir-tech Canada Ltd.  All rights reserved.";

//option Text
static const ST::string s_dut_zOptionsToggleText[zOptionsToggleText_SIZE] =
{
	"Spraak",
	"Bevestigingen uit",
	"Ondertitels",
	"Wacht bij tekst-dialogen",
	"Rook Animeren",
	"Bloedsporen Tonen",
	"Cursor Niet Bewegen",
	"Oude Selectiemethode",
	"Toon reisroute",
	"Toon Missers",
	"Bevestiging Real-Time",
	"Slaap/wakker-berichten",
	"Metrieke Stelsel",
	"Huurling Oplichten",
	"Auto-Cursor naar Huurling",
	"Auto-Cursor naar Deuren",
	"Items Oplichten",
	"Toon Boomtoppen",
	"Toon Draadmodellen",
	"Toon 3D Cursor",
};

//This is the help text associated with the above toggles.
static const ST::string s_dut_zOptionsScreenHelpText[zOptionsToggleText_SIZE] =
{
	//speech
	"Schakel deze optie IN als je de karakter-dialogen wil horen.",

	//Mute Confirmation
	"Schakelt verbale bevestigingen v.d. karakters in of uit.",

	//Subtitles
	"Stelt in of dialoogteksten op het scherm worden getoond.",

	//Key to advance speech
	"Als ondertitels AANstaan, schakel dit ook in om tijd te hebben de NPC-dialogen te lezen.",

	//Toggle smoke animation
	"Schakel deze optie uit als rookanimaties het spel vertragen.",

	//Blood n Gore
	"Schakel deze optie UIT als je bloed aanstootgevend vindt.",

	//Never move my mouse
	"Schakel deze optie UIT als je wil dat de muis automatisch gepositioneerd wordt bij bevestigingsdialogen.",

	//Old selection method
	"Schakel deze optie IN als je karakters wil selecteren zoals in de vorige JAGGED ALLIANCE (methode is tegengesteld dus).",

	//Show movement path
	"Schakel deze optie IN om bewegingspaden te tonen in real-time (schakel het uit en gebruik dan de SHIFT-toets om paden te tonen).",

	//show misses
	"Schakel IN om het spel de plaats van inslag van je kogels te tonen wanneer je \"mist\".",

	//Real Time Confirmation
	"Als INGESCHAKELD, een extra \"veiligheids\"-klik is nodig om in real-time te bewegen.",

	//Sleep/Wake notification
	"INGESCHAKELD zorgt voor berichten of huurlingen op een \"missie\" slapen of werken.",

	//Use the metric system
	"Wanneer INGESCHAKELD wordt het metrieke stelsel gebruikt, anders het Imperiale stelsel.",

	//Merc Lighted movement
	"Wanneer INGESCHAKELD, de huurling verlicht de grond tijdens het lopen. Schakel UIT voor sneller spelen.",

	//Smart cursor
	"Wanneer INGESCHAKELD zullen huurlingen dichtbij de cursor automatisch oplichten.",

	//snap cursor to the door
	"Wanneer INGESCHAKELD zal de cursor dichtbij een deur automatisch boven de deur gepositioneerd worden.",

	//glow items
	"Wanneer INGESCHAKELD lichten |Items altijd op",

	//toggle tree tops
	"Wanneer INGESCHAKELD worden Boom|toppen getoond.",

	//toggle wireframe
	"Wanneer INGESCHAKELD worden Draadmodellen (|W) van niet-zichtbare muren getoond.",

	"Wanneer INGESCHAKELD wordt de cursor in 3D getoond. (|H|o|m|e)",

};


static const ST::string s_dut_gzGIOScreenText[gzGIOScreenText_SIZE] =
{
	"SPEL-INSTELLINGEN",
	"Speelstijl",
	"Realistisch",
	"SF",
	"Wapenopties",
	"Extra wapens",
	"Normaal",
	"Moeilijksheidsgraad",
	"Beginneling",
	"Ervaren",
	"Expert",
	"Ok",
	"Stop",
	"Extra Moeilijk",
	"Ongelimiteerde Tijd",
	"Getimede Beurten",
	"Dead is Dead"
};

// This string is used in the IMP character generation.  It is possible to
// select 0 ability in a skill meaning you can't use it.  This text is
// confirmation to the player.
static const ST::string s_dut_pSkillAtZeroWarning    = "Are you sure? A value of zero means NO ability in this skill.";
static const ST::string s_dut_pIMPBeginScreenStrings = "( 8 Karakters Max )";
static const ST::string s_dut_pIMPFinishButtonText   = "Analiseren";
static const ST::string s_dut_pIMPFinishStrings      = "Bedankt, %s"; //%s is the name of the merc
static const ST::string s_dut_pIMPVoicesStrings      = "Stem"; // the strings for imp voices screen

// title for program
static const ST::string s_dut_pPersTitleText = "Personeelsmanager";

// paused game strings
static const ST::string s_dut_pPausedGameText[pPausedGameText_SIZE] =
{
	"Spel Gepauzeerd",
	"Doorgaan (|P|a|u|s|e)",
	"Pauze Spel (|P|a|u|s|e)",
};


static const ST::string s_dut_pMessageStrings[pMessageStrings_SIZE] =
{
	"Spel verlaten?",
	"OK",
	"JA",
	"NEE",
	"STOPPEN",
	"WEER AANNEMEN",
	"LEUGEN",
	"Geen beschrijving", //Save slots that don't have a description.
	"Spel opgeslagen.",
	"Dag",
	"Huurl",
	"Leeg Slot",		//An empty save game slot
	"rpm",				//Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
	"min",				//Abbreviation for minute.
	"m",					//One character abbreviation for meter (metric distance measurement unit).
	"rnds",				//Abbreviation for rounds (# of bullets)
	"kg",					//Abbreviation for kilogram (metric weight measurement unit)
	"lb",					//Abbreviation for pounds (Imperial weight measurement unit)
	"Home",				//Home as in homepage on the internet.
	"USD",					//Abbreviation to US dollars
	"nvt",					//Lowercase acronym for not applicable.
	"Intussen",		//Meanwhile
	"%s is gearriveerd in sector %s%s", //Name/Squad has arrived in sector A9.  Order must not change without notifying
																		//SirTech
	"Versie",
	"Nieuwe savegame maken",
	"Dit slot is gereserveerd voor SnelBewaren tijdens tactische en kaartoverzichten m.b.v. ALT+S.",
	"Geopend",
	"Gesloten",
	"Schijfruimte raakt op. Er is slects %s MB vrij en Jagged Alliance 2 heeft %s MB nodig.",
	"%s heeft %s gevangen.",		//'Merc name' has caught 'item' -- let SirTech know if name comes after item.
	"%s heeft de drug genomen.",		//'Merc name' has taken the drug
	"%s heeft geen medische kennis",	//'Merc name' has no medical skill.

	//CDRom errors (such as ejecting CD while attempting to read the CD)
	"De integriteit van het spel is aangetast.",
	"FOUT: CD-ROM geopend",

	//When firing heavier weapons in close quarters, you may not have enough room to do so.
	"Er is geen plaats om vanaf hier te schieten.",

	//Can't change stance due to objects in the way...
	"Kan op dit moment geen standpunt wisselen.",

	//Simple text indications that appear in the game, when the merc can do one of these things.
	"Drop",
	"Gooi",
	"Geef",

	"%s gegeven aan %s.",	//"Item" passed to "merc".  Please try to keep the item %s before the merc %s,
					//otherwise, must notify SirTech.
	"Geen plaats om %s aan %s te geven.",	//pass "item" to "merc".  Same instructions as above.

	//A list of attachments appear after the items.  Ex:  Kevlar vest ( Ceramic Plate 'Attached )'
	" eraan vastgemaakt)",

	//Cheat modes
	"Vals spel niveau EEN",
	"Vals spel niveau TWEE",

	//Toggling various stealth modes
	"Team op sluipmodus.",
	"Team niet op sluipmodus.",
	"%s op sluipmodus.",
	"%s niet op sluipmodus.",

	//Wireframes are shown through buildings to reveal doors and windows that can't otherwise be seen in
	//an isometric engine.  You can toggle this mode freely in the game.
	"Extra Draadmodellen Aan",
	"Extra Draadmodellen Uit",

	//These are used in the cheat modes for changing levels in the game.  Going from a basement level to
	//an upper level, etc.
	"Kan niet naar boven vanaf dit niveau...",
	"Er zijn geen lagere niveaus...",
	"Betreden basisniveau %d...",
	"Verlaten basisniveau...",

	"'s",		// used in the shop keeper inteface to mark the ownership of the item eg Red's gun
	"Volgmodus UIT.",
	"Volgmodus AAN.",
	"3D Cursor UIT.",
	"3D Cursor AAN.",
	"Team %d actief.",
	"Je kunt %s's dagelijkse salaris van %s niet betalen",	//first %s is the mercs name, the seconds is a string containing the salary
	"Overslaan",
	"%s kan niet alleen weggaan.",
	"Een spel is bewaard onder de naam SaveGame99.sav. Indien nodig, hernoem het naar SaveGame10 zodat je het kan aanroepen in het Laden-scherm.",
	"%s dronk wat %s",
	"Een pakket is in Drassen gearriveerd.",
	"%s zou moeten arriveren op het aangewezen punt (sector %s) op dag %d, om ongeveer %s.",
	"Geschiedenisverslag bijgewerkt.",
};


static const ST::string s_dut_ItemPickupHelpPopup[ItemPickupHelpPopup_SIZE] =
{
	"OK",
	"Scroll Omhoog",
	"Selecteer Alles",
	"Scroll Omlaag",
	"Stop",
};

static const ST::string s_dut_pDoctorWarningString[pDoctorWarningString_SIZE] =
{
	"%s is niet dichtbij genoeg om te worden genezen.",
	"Je medici waren niet in staat om iedereen te verbinden.",
};

static const ST::string s_dut_pMilitiaButtonsHelpText[pMilitiaButtonsHelpText_SIZE] =
{
	"Raap op(Klik Rechts)/drop(Klik links) Groene Troepen",	// button help text informing player they can pick up or drop militia with this button
	"Raap op(Klik Rechts)/drop(Klik links) Normale Troepen",
	"Raap op(Klik Rechts)/drop(Klik links) Veteranentroepen",
	"Verspreidt beschikbare milities evenredig over alle sectoren",
};

// to inform the player to hire some mercs to get things going
static const ST::string s_dut_pMapScreenJustStartedHelpText = "Ga naar AIM en huur wat huurlingen in ( *Hint* dat kan bij Laptop )";

static const ST::string s_dut_pAntiHackerString = "Fout. Bestanden missen of zijn beschadigd. Spel wordt beëindigd.";


static const ST::string s_dut_gzLaptopHelpText[gzLaptopHelpText_SIZE] =
{
	//Buttons:
	"Lees E-mail",
	"Bekijk web-pagina's",
	"Bekijk bestanden en e-mail attachments",
	"Lees verslag van gebeurtenissen",
	"Bekijk team-info",
	"Bekijk financieel overzicht",
	"Sluit laptop",

	//Bottom task bar icons (if they exist):
	"Je hebt nieuwe berichten",
	"Je hebt nieuwe bestanden",

	//Bookmarks:
	"Association of International Mercenaries",
	"Bobby Ray's online weapon mail order",
	"Institute of Mercenary Profiling",
	"More Economic Recruiting Center",
	"McGillicutty's Mortuarium",
	"United Floral Service",
	"Verzekeringsagenten voor A.I.M. contracten",
};


static const ST::string s_dut_gzHelpScreenText = "Verlaat help-scherm";

static const ST::string s_dut_gzNonPersistantPBIText[gzNonPersistantPBIText_SIZE] =
{
	"Er is een gevecht gaande. Je kan alleen terugtrekken m.b.v. het tactische scherm.",
	"B|etreedt sector om door te gaan met het huidige gevecht.",
	"Los huidige gevecht |automatisch op.",
	"Gevecht kan niet automatisch opgelost worden als je de aanvaller bent.",
	"Gevecht kan niet automatisch opgelost worden als je in een hinderlaag ligt.",
	"Gevecht kan niet automatisch opgelost worden als je vecht met beesten in de mijnen.",
	"Gevecht kan niet automatisch opgelost worden als er vijandige burgers zijn.",
	"Gevecht kan niet automatisch opgelost worden als er nog bloodcats zijn.",
	"GEVECHT GAANDE",
	"je kan je op dit moment niet terugtrekken.",
};

static const ST::string s_dut_gzMiscString[gzMiscString_SIZE] =
{
	"Je militie vecht door zonder hulp van je huurlingen...",
	"Het voertuig heeft geen brandstof meer nodig.",
	"De brandstoftank is voor %d%% gevuld.",
	"Het leger van Deidranna heeft totale controle verkregen over %s.",
	"Je hebt een tankplaats verloren.",
};

static const ST::string s_dut_gzIntroScreen = "Kan intro video niet vinden";

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
static const ST::string s_dut_pNewNoiseStr[pNewNoiseStr_SIZE] =
{
	"%s hoort een %s geluid uit %s.",
	"%s hoort een %s geluid van BEWEGING uit %s.",
	"%s hoort een %s KRAKEND geluid uit %s.",
	"%s hoort een %s SPETTEREND geluid uit %s.",
	"%s hoort een %s INSLAG uit %s.",
	"%s hoort een %s EXPLOSIE naar %s.",
	"%s hoort een %s SCHREEUW naar %s.",
	"%s hoort een %s INSLAG naar %s.",
	"%s hoort een %s INSLAG naar %s.",
	"%s hoort een %s VERSPLINTEREN uit %s.",
	"%s hoort een %s KLAP uit %s.",
};

static const ST::string s_dut_wMapScreenSortButtonHelpText[wMapScreenSortButtonHelpText_SIZE] =
{
	"Sorteer op Naam (|F|1)",
	"Sorteer op Taak (|F|2)",
	"Sorteer op Slaapstatus (|F|3)",
	"Sorteer op locatie (|F|4)",
	"Sorteer op Bestemming (|F|5)",
	"Sorteer op Vertrektijd (|F|6)",
};



static const ST::string s_dut_BrokenLinkText[BrokenLinkText_SIZE] =
{
	"Fout 404",
	"Site niet gevonden.",
};


static const ST::string s_dut_gzBobbyRShipmentText[gzBobbyRShipmentText_SIZE] =
{
	"Recentelijke ladingen",
	"Order #",
	"Aantal Items",
	"Besteld op",
};


static const ST::string s_dut_gzCreditNames[gzCreditNames_SIZE]=
{
	"Chris Camfield",
	"Shaun Lyng",
	"Kris Märnes",
	"Ian Currie",
	"Linda Currie",
	"Eric \"WTF\" Cheng",
	"Lynn Holowka",
	"Norman \"NRG\" Olsen",
	"George Brooks",
	"Andrew Stacey",
	"Scot Loving",
	"Andrew \"Big Cheese\" Emmons",
	"Dave \"The Feral\" French",
	"Alex Meduna",
	"Joey \"Joeker\" Whelan",
};


static const ST::string s_dut_gzCreditNameTitle[gzCreditNameTitle_SIZE]=
{
	"Spel Programmeur", 					// Chris Camfield "Game Internals Programmer"
	"Co-ontwerper/Schrijver",				// Shaun Lyng "Co-designer/Writer"
	"Strategische Systemen & Programmeur",		//Kris Marnes "Strategic Systems & Editor Programmer"
	"Producer/Co-ontwerper",				// Ian Currie "Producer/Co-designer"
	"Co-ontwerper/Kaartontwerp",				// Linda Currie "Co-designer/Map Designer"
	"Artiest",							// Eric \"WTF\" Cheng "Artist"
	"Beta Coördinator, Ondersteuning",			// Lynn Holowka
	"Artiest Extraordinaire",				// Norman \"NRG\" Olsen
	"Geluidsgoeroe",						// George Brooks
	"Schermontwerp/Artiest",				// Andrew Stacey
	"Hoofd-Artiest/Animator",				// Scot Loving
	"Hoofd-Programmeur",					// Andrew \"Big Cheese Doddle\" Emmons
	"Programmeur",							// Dave French
	"Strategische Systemen & Spelbalans Programmeur",	// Alex Meduna
	"Portret-Artiest",						// Joey \"Joeker\" Whelan",
};

static const ST::string s_dut_gzCreditNameFunny[gzCreditNameFunny_SIZE]=
{
	"", 																			// Chris Camfield
	"(leert nog steeds interpunctie)",			// Shaun Lyng
	"(\"Het is klaar. Ben er mee bezig\")",		//Kris \"The Cow Rape Man\" Marnes
	"(wordt veel te oud voor dit)",			// Ian Currie
	"(en werkt aan Wizardry 8)",				// Linda Currie
	"(moets onder bedreiging ook QA doen)",		// Eric \"WTF\" Cheng
	"(Verliet ons voor CFSA - dus...)",		// Lynn Holowka
	"",								// Norman \"NRG\" Olsen
	"",								// George Brooks
	"(Dead Head en jazz liefhebber)",			// Andrew Stacey
	"(in het echt heet hij Robert)",			// Scot Loving
	"(de enige verantwoordelijke persoon)",		// Andrew \"Big Cheese Doddle\" Emmons
	"(kan nu weer motorcrossen)",			// Dave French
	"(gestolen van Wizardry 8)",				// Alex Meduna
	"(deed items en schermen-laden ook!)",		// Joey \"Joeker\" Whelan",
};

static const ST::string s_dut_sRepairsDoneString[sRepairsDoneString_SIZE] =
{
	"%s is klaar met reparatie van eigen items",
	"%s is klaar met reparatie van ieders wapens en bepantering",
	"%s is klaar met reparatie van ieders uitrusting",
	"%s is klaar met reparatie van ieders vervoerde items",
};

static const ST::string s_dut_zGioDifConfirmText[zGioDifConfirmText_SIZE]=
{
	"Je hebt de NOVICE-modus geselecteerd. Deze instelling is geschikt voor diegenen die Jagged Alliance voor de eerste keer spelen, voor diegenen die nog niet zo bekend zijn met strategy games, of voor diegenen die kortere gevechten in de game willen hebben.", //Je keuze beïnvloedt dingen in het hele verloop van de game, dus weet wat je doet. Weet je zeker dat je in de Novice-modus wilt spelen?",
	"Je hebt de EXPERIENCED-modus geselecteerd. Deze instelling is geschikt voor diegenen die al bekend zijn met Jagged Alliance of dergelijke games. Je keuze beïnvloedt dingen in het hele verloop van de game, dus weet wat je doet. Weet je zeker dat je in de Experienced-modus wilt spelen ?",
	"Je hebt de EXPERT-modus geselecteerd. We hebben je gewaarschuwd. Geef ons niet de schuld als je in een kist terugkomt. Je keuze beïnvloedt dingen in het hele verloop van de game, dus weet wat je doet. Weet je zeker dat je in de Expert-modus wilt spelen?",
};

static const ST::string s_dut_gzLateLocalizedString[gzLateLocalizedString_SIZE] =
{
	//1-5
	"De robot kan de sector niet verlaten als niemand de besturing gebruikt.",

	//This message comes up if you have pending bombs waiting to explode in tactical.
	"Je kan de tijd niet versnellen, Wacht op het vuurwerk!",

	//'Name' refuses to move.
	"%s weigert zich te verplaatsen.",

	//%s a merc name
	"%s heeft niet genoeg energie om standpunt te wisselen.",

	//A message that pops up when a vehicle runs out of gas.
	"{} heeft geen brandstof en is gestrand in {}.",

	//6-10

	// the following two strings are combined with the pNewNoise[] strings above to report noises
	// heard above or below the merc
	"boven",
	"onder",

	//The following strings are used in autoresolve for autobandaging related feedback.
	"Niemand van je huurlingen heeft medische kennis.",
	"Er zijn geen medische hulpmiddelen om mensen te verbinden.",
	"Er waren niet genoeg medische hulpmiddelen om iedereen te verbinden.",
	"Geen enkele huurling heeft medische hulp nodig.",
	"Verbindt huurlingen automatisch.",
	"Al je huurlingen zijn verbonden.",

	//14
	"Arulco",

	"(dak)",

	"Gezondheid: %d/%d",

	//In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
	//"vs." is the abbreviation of versus.
	"%d vs. %d",

	"%s is vol!",  //(ex "The ice cream truck is full")

	"%s heeft geen eerste hulp nodig, maar échte medische hulp of iets dergelijks.",

	//20
	//Happens when you get shot in the legs, and you fall down.
	"%s is geraakt in het been en valt om!",
	//Name can't speak right now.
	"%s kan nu niet praten.",

	//22-24 plural versions
	"%d groene milities zijn gepromoveerd tot veteranenmilitie.",
	"%d groene milities zijn gepromoveerd tot reguliere militie.",
	"%d reguliere milities zijn gepromoveerd tot veteranenmilitie.",

	//25
	"Schakelaar",

	//26
	//Name has gone psycho -- when the game forces the player into burstmode (certain unstable characters)
	"%s wordt gek!",

	//27-28
	//Messages why a player can't time compress.
	"Het is nu onveilig om de tijd te versnellen omdat je huurlingen hebt in sector %s.",
	"Het is nu onveilig om de tijd te versnellen als er huurlingen zijn in de mijnen met beesten.",

	//29-31 singular versions
	"1 groene militie is gepromoveerd tot veteranenmilitie.",
	"1 groene militie is gepromoveerd tot reguliere militie.",
	"1 reguliere militie is gepromoveerd tot veteranenmilitie.",

	//32-34
	"%s zegt helemaal niets.",
	"Naar oppervlakte reizen?",
	"(Team %d)",

	//35
	//Ex: "Red has repaired Scope's MP5K".  Careful to maintain the proper order (Red before Scope, Scope before MP5K)
	"%s heeft %s's %s gerepareerd",

	//36
	"BLOODCAT",

	//37-38 "Name trips and falls"
	"%s ups en downs",
	"Dit item kan vanaf hier niet opgepakt worden.",

	//39
	"Geen enkele huurling van je is in staat om te vechten. De militie zal zelf tegen de beesten vechten.",

	//40-43
	//%s is the name of merc.
	"%s heeft geen medische kits meer!",
	"%s heeft geen medische kennis om iemand te verzorgen!",
	"%s heeft geen gereedschapkits meer!",
	"%s heeft geen technische kennis om iets te repareren!",

	//44-45
	"Reparatietijd",
	"%s kan deze persoon niet zien.",

	//46-48
	"%s's pistoolloopverlenger valt eraf!",
	"Niet meer dan %d militietrainers zijn toegelaten per sector.",
	"Zeker weten?",

	//49-50
	"Tijdversneller",
	"De tank van het voertuig is nu vol.",

	//51-52 Fast help text in mapscreen.
	"Doorgaan met Tijdversnelling (|S|p|a|c|e)",
	"Stop Tijdversnelling (|E|s|c)",

	//53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
	"%s heeft de %s gedeblokkeerd",
	"%s heeft %s's %s gedeblokkeerd",

	//55
	"Kan tijd niet versneller tijdens bekijken van sector inventaris.",

	//56
	//Displayed with the version information when cheats are enabled.
	"Huidig/Max Voortgang: %d%%/%d%%",

	//57
	"John en Mary escorteren?",

	"Schakelaar geactiveerd.",
};

static const ST::string s_dut_str_ceramic_plates_smashed = "%s's ceramic plates have been smashed!"; // TODO translate

static const ST::string s_dut_str_arrival_rerouted = "Arrival of new recruits is being rerouted to sector %s, as scheduled drop-off point of sector %s is enemy occupied."; // TODO translate


static const ST::string s_dut_str_stat_health       = "Gezondheid";
static const ST::string s_dut_str_stat_agility      = "Beweeglijkheid";
static const ST::string s_dut_str_stat_dexterity    = "Handigheid";
static const ST::string s_dut_str_stat_strength     = "Kracht";
static const ST::string s_dut_str_stat_leadership   = "Leiderschap";
static const ST::string s_dut_str_stat_wisdom       = "Wijsheid";
static const ST::string s_dut_str_stat_exp_level    = "Ervaringsniveau";
static const ST::string s_dut_str_stat_marksmanship = "Trefzekerheid";
static const ST::string s_dut_str_stat_mechanical   = "Technisch";
static const ST::string s_dut_str_stat_explosive    = "Explosieven";
static const ST::string s_dut_str_stat_medical      = "Medisch";

static const ST::string s_dut_str_stat_list[str_stat_list_SIZE] =
{
	s_dut_str_stat_health,
	s_dut_str_stat_agility,
	s_dut_str_stat_dexterity,
	s_dut_str_stat_strength,
	s_dut_str_stat_leadership,
	s_dut_str_stat_wisdom,
	s_dut_str_stat_exp_level,
	s_dut_str_stat_marksmanship,
	s_dut_str_stat_mechanical,
	s_dut_str_stat_explosive,
	s_dut_str_stat_medical
};

static const ST::string s_dut_str_aim_sort_list[str_aim_sort_list_SIZE] =
{
	s_dut_str_aim_sort_price,
	s_dut_str_aim_sort_experience,
	s_dut_str_aim_sort_marksmanship,
	s_dut_str_aim_sort_medical,
	s_dut_str_aim_sort_explosives,
	s_dut_str_aim_sort_mechanical,
	s_dut_str_aim_sort_ascending,
	s_dut_str_aim_sort_descending,
};

static const ST::string s_dut_gs_dead_is_dead_mode_tab_name[gs_dead_is_dead_mode_tab_name_SIZE] =
{
	"Normal", 			// Normal Tab
	"DiD", 			// Dead is Dead Tab
};

// Dutch language resources.
LanguageRes g_LanguageResDutch = {

	s_dut_WeaponType,

	s_dut_Message,
	s_dut_TeamTurnString,
	s_dut_pAssignMenuStrings,
	s_dut_pTrainingStrings,
	s_dut_pTrainingMenuStrings,
	s_dut_pAttributeMenuStrings,
	s_dut_pVehicleStrings,
	s_dut_pShortAttributeStrings,
	s_dut_pContractStrings,
	s_dut_pAssignmentStrings,
	s_dut_pConditionStrings,
	s_dut_pPersonnelScreenStrings,
	s_dut_pUpperLeftMapScreenStrings,
	s_dut_pTacticalPopupButtonStrings,
	s_dut_pSquadMenuStrings,
	s_dut_pDoorTrapStrings,
	s_dut_pLongAssignmentStrings,
	s_dut_pMapScreenMouseRegionHelpText,
	s_dut_pNoiseVolStr,
	s_dut_pNoiseTypeStr,
	s_dut_pDirectionStr,
	s_dut_pRemoveMercStrings,
	s_dut_sTimeStrings,
	s_dut_pInvPanelTitleStrings,
	s_dut_pPOWStrings,
	s_dut_pMilitiaString,
	s_dut_pMilitiaButtonString,
	s_dut_pEpcMenuStrings,
	s_dut_pRepairStrings,
	s_dut_sPreStatBuildString,
	s_dut_sStatGainStrings,
	s_dut_pHelicopterEtaStrings,
	s_dut_sMapLevelString,
	s_dut_gsLoyalString,
	s_dut_gsUndergroundString,
	s_dut_gsTimeStrings,
	s_dut_sFacilitiesStrings,
	s_dut_pMapPopUpInventoryText,
	s_dut_pwTownInfoStrings,
	s_dut_pwMineStrings,
	s_dut_pwMiscSectorStrings,
	s_dut_pMapInventoryErrorString,
	s_dut_pMapInventoryStrings,
	s_dut_pMovementMenuStrings,
	s_dut_pUpdateMercStrings,
	s_dut_pMapScreenBorderButtonHelpText,
	s_dut_pMapScreenBottomFastHelp,
	s_dut_pMapScreenBottomText,
	s_dut_pMercDeadString,
	s_dut_pSenderNameList,
	s_dut_pNewMailStrings,
	s_dut_pDeleteMailStrings,
	s_dut_pEmailHeaders,
	s_dut_pEmailTitleText,
	s_dut_pFinanceTitle,
	s_dut_pFinanceSummary,
	s_dut_pFinanceHeaders,
	s_dut_pTransactionText,
	s_dut_pMoralStrings,
	s_dut_pSkyriderText,
	s_dut_str_left_equipment,
	s_dut_pMapScreenStatusStrings,
	s_dut_pMapScreenPrevNextCharButtonHelpText,
	s_dut_pEtaString,
	s_dut_pShortVehicleStrings,
	s_dut_pTrashItemText,
	s_dut_pMapErrorString,
	s_dut_pMapPlotStrings,
	s_dut_pBullseyeStrings,
	s_dut_pMiscMapScreenMouseRegionHelpText,
	s_dut_str_he_leaves_where_drop_equipment,
	s_dut_str_she_leaves_where_drop_equipment,
	s_dut_str_he_leaves_drops_equipment,
	s_dut_str_she_leaves_drops_equipment,
	s_dut_pImpPopUpStrings,
	s_dut_pImpButtonText,
	s_dut_pExtraIMPStrings,
	s_dut_pFilesTitle,
	s_dut_pFilesSenderList,
	s_dut_pHistoryLocations,
	s_dut_pHistoryStrings,
	s_dut_pHistoryHeaders,
	s_dut_pHistoryTitle,
	s_dut_pShowBookmarkString,
	s_dut_pWebPagesTitles,
	s_dut_pWebTitle,
	s_dut_pPersonnelString,
	s_dut_pErrorStrings,
	s_dut_pDownloadString,
	s_dut_pBookMarkStrings,
	s_dut_pLaptopIcons,
	s_dut_gsAtmStartButtonText,
	s_dut_pPersonnelTeamStatsStrings,
	s_dut_pPersonnelCurrentTeamStatsStrings,
	s_dut_pPersonelTeamStrings,
	s_dut_pPersonnelDepartedStateStrings,
	s_dut_pMapHortIndex,
	s_dut_pMapVertIndex,
	s_dut_pMapDepthIndex,
	s_dut_pLaptopTitles,
	s_dut_pDayStrings,
	s_dut_pMilitiaConfirmStrings,
	s_dut_pSkillAtZeroWarning,
	s_dut_pIMPBeginScreenStrings,
	s_dut_pIMPFinishButtonText,
	s_dut_pIMPFinishStrings,
	s_dut_pIMPVoicesStrings,
	s_dut_pPersTitleText,
	s_dut_pPausedGameText,
	s_dut_zOptionsToggleText,
	s_dut_zOptionsScreenHelpText,
	s_dut_pDoctorWarningString,
	s_dut_pMilitiaButtonsHelpText,
	s_dut_pMapScreenJustStartedHelpText,
	s_dut_pLandMarkInSectorString,
	s_dut_gzMercSkillText,
	s_dut_gzNonPersistantPBIText,
	s_dut_gzMiscString,
	s_dut_wMapScreenSortButtonHelpText,
	s_dut_pNewNoiseStr,
	s_dut_gzLateLocalizedString,
	s_dut_pAntiHackerString,
	s_dut_pMessageStrings,
	s_dut_ItemPickupHelpPopup,
	s_dut_TacticalStr,
	s_dut_LargeTacticalStr,
	s_dut_zDialogActions,
	s_dut_zDealerStrings,
	s_dut_zTalkMenuStrings,
	s_dut_gzMoneyAmounts,
	s_dut_gzProsLabel,
	s_dut_gzConsLabel,
	s_dut_gMoneyStatsDesc,
	s_dut_gWeaponStatsDesc,
	s_dut_sKeyDescriptionStrings,
	s_dut_zHealthStr,
	s_dut_zVehicleName,
	s_dut_pExitingSectorHelpText,
	s_dut_InsContractText,
	s_dut_InsInfoText,
	s_dut_MercAccountText,
	s_dut_MercInfo,
	s_dut_MercNoAccountText,
	s_dut_MercHomePageText,
	s_dut_sFuneralString,
	s_dut_sFloristText,
	s_dut_sOrderFormText,
	s_dut_sFloristGalleryText,
	s_dut_sFloristCards,
	s_dut_BobbyROrderFormText,
	s_dut_BobbyRText,
	s_dut_str_bobbyr_guns_num_guns_that_use_ammo,
	s_dut_BobbyRaysFrontText,
	s_dut_AimSortText,
	s_dut_str_aim_sort_price,
	s_dut_str_aim_sort_experience,
	s_dut_str_aim_sort_marksmanship,
	s_dut_str_aim_sort_medical,
	s_dut_str_aim_sort_explosives,
	s_dut_str_aim_sort_mechanical,
	s_dut_str_aim_sort_ascending,
	s_dut_str_aim_sort_descending,
	s_dut_AimPolicyText,
	s_dut_AimMemberText,
	s_dut_CharacterInfo,
	s_dut_VideoConfercingText,
	s_dut_AimPopUpText,
	s_dut_AimLinkText,
	s_dut_AimHistoryText,
	s_dut_AimFiText,
	s_dut_AimAlumniText,
	s_dut_AimScreenText,
	s_dut_AimBottomMenuText,
	s_dut_zMarksMapScreenText,
	s_dut_gpStrategicString,
	s_dut_gpGameClockString,
	s_dut_SKI_Text,
	s_dut_SkiMessageBoxText,
	s_dut_zSaveLoadText,
	s_dut_zOptionsText,
	s_dut_gzGIOScreenText,
	s_dut_gzHelpScreenText,
	s_dut_gzLaptopHelpText,
	s_dut_gzMoneyWithdrawMessageText,
	s_dut_gzCopyrightText,
	s_dut_BrokenLinkText,
	s_dut_gzBobbyRShipmentText,
	s_dut_zGioDifConfirmText,
	s_dut_gzCreditNames,
	s_dut_gzCreditNameTitle,
	s_dut_gzCreditNameFunny,
	s_dut_pContractButtonString,
	s_dut_gzIntroScreen,
	s_dut_pUpdatePanelButtons,
	s_dut_sRepairsDoneString,
	s_dut_str_ceramic_plates_smashed,
	s_dut_str_arrival_rerouted,

	s_dut_str_stat_health,
	s_dut_str_stat_agility,
	s_dut_str_stat_dexterity,
	s_dut_str_stat_strength,
	s_dut_str_stat_leadership,
	s_dut_str_stat_wisdom,
	s_dut_str_stat_exp_level,
	s_dut_str_stat_marksmanship,
	s_dut_str_stat_mechanical,
	s_dut_str_stat_explosive,
	s_dut_str_stat_medical,

	s_dut_str_stat_list,
	s_dut_str_aim_sort_list,

	g_eng_zNewTacticalMessages,
	g_eng_str_iron_man_mode_warning,
	g_eng_str_dead_is_dead_mode_warning,
	g_eng_str_dead_is_dead_mode_enter_name,
	s_dut_gs_dead_is_dead_mode_tab_name,

	s_dut_gzIMPSkillTraitsText,
};

#ifdef WITH_UNITTESTS
TEST(StringEncodingTest, DutchTextFile)
{
	// This test checks that the utf8 string literals in this file are correctly
	// interpreted by the compiler.  Failed test means that the compiler
	// cannot correctly interpret the string literals.
	const ST::string str = "тест";
	ASSERT_EQ(str.size(), 8u) << "Compiler cannot correctly interpret utf8 string literals";
	EXPECT_EQ(str[0], '\xD1');
	EXPECT_EQ(str[1], '\x82');
	EXPECT_EQ(str[2], '\xD0');
	EXPECT_EQ(str[3], '\xB5');
	EXPECT_EQ(str[4], '\xD1');
	EXPECT_EQ(str[5], '\x81');
	EXPECT_EQ(str[6], '\xD1');
	EXPECT_EQ(str[7], '\x82');
	EXPECT_EQ(str[8], '\0');
}
#endif
