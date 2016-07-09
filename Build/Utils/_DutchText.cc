#include "Text.h"

#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"
#endif

/*

******************************************************************************************************
**                                  IMPORTANT TRANSLATION NOTES                                     **
******************************************************************************************************

GENERAL INSTRUCTIONS
- Always be aware that foreign strings should be of equal or shorter length than the English equivalent.
	I know that this is difficult to do on many occasions due to the nature of foreign languages when
	compared to English.  By doing so, this will greatly reduce the amount of work on both sides.  In
	most cases (but not all), JA2 interfaces were designed with just enough space to fit the English word.
	The general rule is if the string is very short (less than 10 characters), then it's short because of
	interface limitations.  On the other hand, full sentences commonly have little limitations for length.
	Strings in between are a little dicey.
- Never translate a string to appear on multiple lines.  All strings L"This is a really long string...",
	must fit on a single line no matter how long the string is.  All strings start with L" and end with ",
- Never remove any extra spaces in strings.  In addition, all strings containing multiple sentences only
	have one space after a period, which is different than standard typing convention.  Never modify sections
	of strings contain combinations of % characters.  These are special format characters and are always
	used in conjunction with other characters.  For example, %ls means string, and is commonly used for names,
	locations, items, etc.  %d is used for numbers.  %c%d is a character and a number (such as A9).
	%% is how a single % character is built.  There are countless types, but strings containing these
	special characters are usually commented to explain what they mean.  If it isn't commented, then
	if you can't figure out the context, then feel free to ask SirTech.
- Comments are always started with // Anything following these two characters on the same line are
	considered to be comments.  Do not translate comments.  Comments are always applied to the following
	string(s) on the next line(s), unless the comment is on the same line as a string.
- All new comments made by SirTech will use "//@@@ comment" (without the quotes) notation.  By searching
	for @@@ everytime you recieve a new version, it will simplify your task and identify special instructions.
  Commonly, these types of comments will be used to ask you to abbreviate a string.  Please leave the
	comments intact, and SirTech will remove them once the translation for that particular area is resolved.
- If you have a problem or question with translating certain strings, please use "//!!! comment"
	(without the quotes).  The syntax is important, and should be identical to the comments used with @@@
	symbols.  SirTech will search for !!! to look for your problems and questions.  This is a more
	efficient method than detailing questions in email, so try to do this whenever possible.



FAST HELP TEXT -- Explains how the syntax of fast help text works.
**************

1) BOLDED LETTERS
	The popup help text system supports special characters to specify the hot key(s) for a button.
	Anytime you see a '|' symbol within the help text string, that means the following key is assigned
	to activate the action which is usually a button.

	EX:  L"|Map Screen"

	This means the 'M' is the hotkey.  In the game, when somebody hits the 'M' key, it activates that
	button.  When translating the text to another language, it is best to attempt to choose a word that
	uses 'M'.  If you can't always find a match, then the best thing to do is append the 'M' at the end
	of the string in this format:

	EX:  L"Ecran De Carte (|M)"  (this is the French translation)

	Other examples are used multiple times, like the Esc key  or "|E|s|c" or Space -> (|S|p|a|c|e)

2) NEWLINE
  Any place you see a \n within the string, you are looking at another string that is part of the fast help
	text system.  \n notation doesn't need to be precisely placed within that string, but whereever you wish
	to start a new line.

	EX:  L"Clears all the mercs' positions,\nand allows you to re-enter them manually."

	Would appear as:

				Clears all the mercs' positions,
				and allows you to re-enter them manually.

	NOTE:  It is important that you don't pad the characters adjacent to the \n with spaces.  If we did this
	       in the above example, we would see

	WRONG WAY -- spaces before and after the \n
	EX:  L"Clears all the mercs' positions, \n and allows you to re-enter them manually."

	Would appear as: (the second line is moved in a character)

				Clears all the mercs' positions,
 				 and allows you to re-enter them manually.


@@@ NOTATION
************

	Throughout the text files, you'll find an assortment of comments.  Comments are used to describe the
	text to make translation easier, but comments don't need to be translated.  A good thing is to search for
	"@@@" after receiving new version of the text file, and address the special notes in this manner.

!!! NOTATION
************

	As described above, the "!!!" notation should be used by you to ask questions and address problems as
	SirTech uses the "@@@" notation.

*/

static StrPointer s_dut_WeaponType[WeaponType_SIZE] =
{
	L"Other",
	L"Pistol",
	L"Machine pistol",
	L"Machine Gun",
	L"Rifle",
	L"Sniper Rifle",
	L"Attack weapon",
	L"Light machine gun",
	L"Shotgun",
};

static StrPointer s_dut_TeamTurnString[TeamTurnString_SIZE] =
{
	L"Beurt speler",
	L"Beurt opponent",
	L"Beurt beest",
	L"Beurt militie",
	L"Beurt burgers",
	// planning turn
};

static StrPointer s_dut_Message[Message_SIZE] =
{
	// In the following 8 strings, the %ls is the merc's name, and the %d (if any) is a number.

	L"%ls geraakt in hoofd en verliest een intelligentiepunt!",
	L"%ls geraakt in de schouder en verliest een handigheidspunt!",
	L"%ls geraakt in de borst en verliest een krachtspunt!",
	L"%ls geraakt in het benen en verliest een beweeglijkspunt!",
	L"%ls geraakt in het hoofd en verliest %d wijsheidspunten!",
	L"%ls geraakt in de schouder en verliest %d handigheidspunten!",
	L"%ls geraakt in de borst en verliest %d krachtspunten!",
	L"%ls geraakt in de benen en verliest %d beweeglijkheidspunten!",
	L"Storing!",

	L"Je versterkingen zijn gearriveerd!",

	// In the following four lines, all %ls's are merc names

	L"%ls herlaad.",
	L"%ls heeft niet genoeg actiepunten!",
	// the following 17 strings are used to create lists of gun advantages and disadvantages
	// (separated by commas)
	L"reliable",
	L"unreliable",
	L"easy to repair",
	L"hard to repair",
	L"much damage",
	L"low damage",
	L"quick fire",
	L"slow fire",
	L"long range",
	L"short range",
	L"light",
	L"heavy",
	L"small",
	L"quick salvo",
	L"no salvo",
	L"large magazine",
	L"small magazine",

	// In the following two lines, all %ls's are merc names

	L"%ls's camouflage is verdwenen.",
	L"%ls's camouflage is afgespoelt.",

	// The first %ls is a merc name and the second %ls is an item name

	L"Tweede wapen is leeg!",
	L"%ls heeft %ls gestolen.",

	// The %ls is a merc name

	L"%ls's wapen vuurt geen salvo.",

	L"Je hebt er al één van die vastgemaakt.",
	L"Samen voegen?",

	// Both %ls's are item names

	L"Je verbindt %ls niet met %ls.",
	L"Geen",
	L"Eject ammo",
	L"Toebehoren",

	//You cannot use "item(s)" and your "other item" at the same time.
	//Ex:  You cannot use sun goggles and you gas mask at the same time.
	L"%ls en %ls zijn niet tegelijk te gebruiken.",

	L"Het item dat je aanwijst, kan vastgemaakt worden aan een bepaald item door het in een van de vier uitbreidingssloten te plaatsen.",
	L"Het item dat je aanwijst, kan vastgemaakt worden aan een bepaald item door het in een van de vier uitbreidingssloten te plaatsen. (Echter, het item is niet compatibel.)",
	L"Er zijn nog vijanden in de sector!",
	L"Je moet %ls %ls nog geven",
	L"kogel doorboorde %ls in zijn hoofd!",
	L"Gevecht verlaten?",
	L"Dit samenvoegen is permanent. Verdergaan?",
	L"%ls heeft meer energie!",
	L"%ls is uitgegleden!",
	L"%ls heeft %ls niet gepakt!",
	L"%ls repareert de %ls",
	L"Stoppen voor ",
	L"Overgeven?",
	L"Deze persoon weigert je hulp.",
	L"Ik denk het NIET!",
	L"Chopper van Skyrider gebruiken? Eerst huurlingen TOEWIJZEN aan VOERTUIG/HELIKOPTER.",
	L"%ls had tijd maar EEN geweer te herladen",
	L"Beurt bloodcats",
};


// the names of the towns in the game

static const wchar_t *s_dut_pTownNames[pTownNames_SIZE] =
{
	L"",
	L"Omerta",
	L"Drassen",
	L"Alma",
	L"Grumm",
	L"Tixa",
	L"Cambria",
	L"San Mona",
	L"Estoni",
	L"Orta",
	L"Balime",
	L"Meduna",
	L"Chitzena",
};

static const wchar_t *s_dut_g_towns_locative[g_towns_locative_SIZE] =
{
	L"",
	L"Omerta",
	L"Drassen",
	L"Alma",
	L"Grumm",
	L"Tixa",
	L"Cambria",
	L"San Mona",
	L"Estoni",
	L"Orta",
	L"Balime",
	L"Meduna",
	L"Chitzena"
};

// the types of time compression. For example: is the timer paused? at normal speed, 5 minutes per second, etc.
// min is an abbreviation for minutes

static const wchar_t *s_dut_sTimeStrings[sTimeStrings_SIZE] =
{
	L"Pause",
	L"Normal",
	L"5 min",
	L"30 min",
	L"60 min",
	L"6 uur",
};


// Assignment Strings: what assignment does the merc  have right now? For example, are they on a squad, training,
// administering medical aid (doctor) or training a town. All are abbreviated. 8 letters is the longest it can be.

static const wchar_t *s_dut_pAssignmentStrings[pAssignmentStrings_SIZE] =
{
	L"Team 1",
	L"Team 2",
	L"Team 3",
	L"Team 4",
	L"Team 5",
	L"Team 6",
	L"Team 7",
	L"Team 8",
	L"Team 9",
	L"Team 10",
	L"Team 11",
	L"Team 12",
	L"Team 13",
	L"Team 14",
	L"Team 15",
	L"Team 16",
	L"Team 17",
	L"Team 18",
	L"Team 19",
	L"Team 20",
	L"Dienst", // on active duty
	L"Dokter", // administering medical aid
	L"Patiënt", // getting medical aid
	L"Voertuig", // in a vehicle
	L"Onderweg", // in transit - abbreviated form
	L"Repareer", // repairing
	L"Oefenen", // training themselves
	L"Militie", // training a town to revolt
	L"Trainer", // training a teammate
	L"Student", // being trained by someone else
	L"Dood", // dead
	L"Uitgesc.", // abbreviation for incapacitated
	L"POW", // Prisoner of war - captured
	L"Kliniek", // patient in a hospital
	L"Leeg",	// Vehicle is empty
};


static const wchar_t *s_dut_pMilitiaString[pMilitiaString_SIZE] =
{
	L"Militie", // the title of the militia box
	L"Unassigned", //the number of unassigned militia troops
	L"Milities kunnen niet herplaatst worden als er nog vijanden in de buurt zijn!",
};


static const wchar_t *s_dut_pMilitiaButtonString[pMilitiaButtonString_SIZE] =
{
	L"Auto", // auto place the militia troops for the player
	L"OK", // done placing militia troops
};

static const wchar_t *s_dut_pConditionStrings[pConditionStrings_SIZE] =
{
	L"Excellent", //the state of a soldier .. excellent health
	L"Good", // good health
	L"Fair", // fair health
	L"Wounded", // wounded health
	L"Tired", // tired
	L"Bleeding", // bleeding to death
	L"Knocked out", // knocked out
	L"Dying", // near death
	L"Dead", // dead
};

static const wchar_t *s_dut_pEpcMenuStrings[pEpcMenuStrings_SIZE] =
{
	L"On duty", // set merc on active duty
	L"Patient", // set as a patient to receive medical aid
	L"Vehicle", // tell merc to enter vehicle
	L"Alone", // let the escorted character go off on their own
	L"Close", // close this menu
};


// look at pAssignmentString above for comments

static const wchar_t *s_dut_pLongAssignmentStrings[pLongAssignmentStrings_SIZE] =
{
	L"Team 1",
	L"Team 2",
	L"Team 3",
	L"Team 4",
	L"Team 5",
	L"Team 6",
	L"Team 7",
	L"Team 8",
	L"Team 9",
	L"Team 10",
	L"Team 11",
	L"Team 12",
	L"Team 13",
	L"Team 14",
	L"Team 15",
	L"Team 16",
	L"Team 17",
	L"Team 18",
	L"Team 19",
	L"Team 20",
	L"Dienst", // on active duty
	L"Dokter", // administering medical aid
	L"Patiënt", // getting medical aid
	L"Voertuig", // in a vehicle
	L"Onderweg", // in transit - abbreviated form
	L"Repareer", // repairing
	L"Oefenen", // training themselves
	L"Militie", // training a town to revolt
	L"Trainer", // training a teammate
	L"Student", // being trained by someone else
	L"Dood", // dead
	L"Uitgesc.", // abbreviation for incapacitated
	L"POW", // Prisoner of war - captured
	L"Kliniek", // patient in a hospital
	L"Leeg",	// Vehicle is empty
};


// the contract options

static const wchar_t *s_dut_pContractStrings[pContractStrings_SIZE] =
{
	L"Contract Opties:",
	L"", // a blank line, required
	L"Voor een dag", // offer merc a one day contract extension
	L"Voor een week", // 1 week
	L"Voor twee weken", // 2 week
	L"Ontslag", // end merc's contract
	L"Stop", // stop showing this menu
};

static const wchar_t *s_dut_pPOWStrings[pPOWStrings_SIZE] =
{
	L"POW",  //an acronym for Prisoner of War
	L"??",
};

static const wchar_t *s_dut_pInvPanelTitleStrings[pInvPanelTitleStrings_SIZE] =
{
	L"Wapen", // the armor rating of the merc
	L"Gewicht", // the weight the merc is carrying
	L"Camo", // the merc's camouflage rating
};

static const wchar_t *s_dut_pShortAttributeStrings[pShortAttributeStrings_SIZE] =
{
	L"Bew", // the abbreviated version of : agility
	L"Han", // dexterity
	L"Kra", // strength
	L"Ldr", // leadership
	L"Wij", // wisdom
	L"Niv", // experience level
	L"Tre", // marksmanship skill
	L"Exp", // explosive skill
	L"Tec", // mechanical skill
	L"Med", // medical skill
};


static const wchar_t *s_dut_pUpperLeftMapScreenStrings[pUpperLeftMapScreenStrings_SIZE] =
{
	L"Opdracht", // the mercs current assignment
	L"Gezond", // the health level of the current merc
	L"Moraal", // the morale of the current merc
	L"Conditie",	// the condition of the current vehicle
};

static const wchar_t *s_dut_pTrainingStrings[pTrainingStrings_SIZE] =
{
	L"Oefen", // tell merc to train self
	L"Militie", // tell merc to train town
	L"Trainer", // tell merc to act as trainer
	L"Student", // tell merc to be train by other
};

static const wchar_t *s_dut_pAssignMenuStrings[pAssignMenuStrings_SIZE] =
{
	L"On duty", // merc is on active duty
	L"Doctor", // the merc is acting as a doctor
	L"Patient", // the merc is receiving medical attention
	L"Vehicle", // the merc is in a vehicle
	L"Repair", // the merc is repairing items
	L"Train", // the merc is training
	L"Stop", // cancel this menu
};

static const wchar_t *s_dut_pRemoveMercStrings[pRemoveMercStrings_SIZE] =
{
	L"Verw.Huurl.", // remove dead merc from current team
	L"Stop",
};

static const wchar_t *s_dut_pAttributeMenuStrings[pAttributeMenuStrings_SIZE] =
{
	L"Kracht",
	L"Behendigheid",
	L"Lenigheid",
	L"Gezondheid",
	L"Scherpschutterskunst",
	L"Medisch",
	L"Mechanisch",
	L"Leiderschap",
	L"Explosief",
	L"Annuleren",
};

/*
static const wchar_t *s_dut_pAttributeMenuStrings[pAttributeMenuStrings_SIZE] =
{
	L"Kracht",
	L"Behendig",
	L"Beweging",
	L"Schietkunst",
	L"Trefzekerheid",
	L"Medisch",
	L"Technisch",
	L"Leaderschip",
	L"Precisie",
	L"Stop",
};
*/

static const wchar_t *s_dut_pTrainingMenuStrings[pTrainingMenuStrings_SIZE] =
{
 L"Oefenen", // train yourself
 L"Militie", // train the town
 L"Trainer", // train your teammates
 L"Student",  // be trained by an instructor
 L"Stop", // cancel this menu
};


static const wchar_t *s_dut_pSquadMenuStrings[pSquadMenuStrings_SIZE] =
{
	L"Team  1",
	L"Team  2",
	L"Team  3",
	L"Team  4",
	L"Team  5",
	L"Team  6",
	L"Team  7",
	L"Team  8",
	L"Team  9",
	L"Team 10",
	L"Team 11",
	L"Team 12",
	L"Team 13",
	L"Team 14",
	L"Team 15",
	L"Team 16",
	L"Team 17",
	L"Team 18",
	L"Team 19",
	L"Team 20",
	L"Stop",
};


static const wchar_t *s_dut_pPersonnelScreenStrings[pPersonnelScreenStrings_SIZE] =
{
	L"Med. Kosten:", // amount of medical deposit put down on the merc
	L"Rest Contract:", // cost of current contract
	L"Doden", // number of kills by merc
	L"Hulp", // number of assists on kills by merc
	L"Dag. Kosten:", // daily cost of merc
	L"Huidige Tot. Kosten:", // total cost of merc
	L"Huidige Tot. Service:", // total service rendered by merc
	L"Salaris Tegoed:", // amount left on MERC merc to be paid
	L"Trefzekerheid:", // percentage of shots that hit target
	L"Gevechten", // number of battles fought
	L"Keren Gewond", // number of times merc has been wounded
	L"Vaardigheden:",
	L"Vaardigheden:",
};


//These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
static const wchar_t *s_dut_gzMercSkillText[gzMercSkillText_SIZE] =
{
	L"No Skill",
	L"Forceer slot",
	L"Man-tot-man",
	L"Elektronica",
	L"Nachtops",
	L"Werpen",
	L"Lesgeven",
	L"Zware Wapens",
	L"Auto Wapens",
	L"Sluipen",
	L"Handig",
	L"Dief",
	L"Vechtkunsten",
	L"Mesworp",
	L"Raak op dak! Bonus",
	L"Camouflaged",
	L"(Expert)",
};


// This is pop up help text for the options that are available to the merc

static const wchar_t *s_dut_pTacticalPopupButtonStrings[pTacticalPopupButtonStrings_SIZE] =
{
	L"|Staan/Lopen",
	L"Hurken/Gehurkt lopen (|C)",
	L"Staan/|Rennen",
	L"Liggen/Kruipen (|P)",
	L"Kijk (|L)",
	L"Actie",
	L"Praat",
	L"Bekijk (|C|t|r|l)",

	// Pop up door menu
	L"Handm. openen",
	L"Zoek boobytraps",
	L"Forceer",
	L"Met geweld",
	L"Verwijder boobytrap",
	L"Sluiten",
	L"Maak open",
	L"Gebruik explosief",
	L"Gebruik breekijzer",
	L"Stoppen (|E|s|c)",
	L"Stop",
};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.

static const wchar_t *s_dut_pDoorTrapStrings[pDoorTrapStrings_SIZE] =
{
	L"geen val",
	L"een explosie",
	L"een elektrische val",
	L"alarm",
	L"stil alarm",
};

// On the map screen, there are four columns. This text is popup help text that identifies the individual columns.

static const wchar_t *s_dut_pMapScreenMouseRegionHelpText[pMapScreenMouseRegionHelpText_SIZE] =
{
	L"Selecteer Karakter",
	L"Contracteer huurling",
	L"Plan Route",
	L"Huurling |Contract",
	L"Verwijder Huurling",
	L"Slaap",
};

// volumes of noises

static const wchar_t *s_dut_pNoiseVolStr[pNoiseVolStr_SIZE] =
{
	L"VAAG",
	L"ZEKER",
	L"HARD",
	L"ERG HARD",
};

// types of noises

static const wchar_t *s_dut_pNoiseTypeStr[pNoiseTypeStr_SIZE] = // OBSOLETE
{
	L"ONBEKEND",
	L"geluid van BEWEGING",
	L"GEKRAAK",
	L"PLONZEN",
	L"INSLAG",
	L"SCHOT",
	L"EXPLOSIE",
	L"GEGIL",
	L"INSLAG",
	L"INSLAG",
	L"BARSTEN",
	L"DREUN",
};

// Directions that are used to report noises

static const wchar_t *s_dut_pDirectionStr[pDirectionStr_SIZE] =
{
	L"het NOORDOOSTEN",
	L"het OOSTEN",
	L"het ZUIDOOSTEN",
	L"het ZUIDEN",
	L"het ZUIDWESTEN",
	L"het WESTEN",
	L"het NOORDWESTEN",
	L"het NOORDEN",
};

// These are the different terrain types.

static const wchar_t *s_dut_pLandTypeStrings[pLandTypeStrings_SIZE] =
{
	L"Stad",
	L"Weg",
	L"Vlaktes",
	L"Woestijn",
	L"Bossen",
	L"Woud",
	L"Moeras",
	L"Water",
	L"Heuvels",
	L"Onbegaanbaar",
	L"Rivier",	//river from north to south
	L"Rivier",	//river from east to west
	L"Buitenland",
	//NONE of the following are used for directional travel, just for the sector description.
	L"Tropisch",
	L"Landbouwgrond",
	L"Vlaktes, weg",
	L"Bossen, weg",
	L"Boerderij, weg",
	L"Tropisch, weg",
	L"Woud, weg",
	L"Kustlijn",
	L"Bergen, weg",
	L"Kust-, weg",
	L"Woestijn, weg",
	L"Moeras, weg",
	L"Bossen, SAM-stelling",
	L"Woestijn, SAM-stelling",
	L"Tropisch, SAM-stelling",
	L"Meduna, SAM-stelling",

	//These are descriptions for special sectors
	L"Cambria Ziekenhuis",
	L"Drassen Vliegveld",
	L"Meduna Vliegveld",
	L"SAM-stelling",
	L"Schuilplaats Rebellen",	//The rebel base underground in sector A10
	L"Tixa Kerker",			//The basement of the Tixa Prison (J9)
	L"Hol Beest",			//Any mine sector with creatures in it
	L"Orta Basis",			//The basement of Orta (K4)
	L"Tunnel",				//The tunnel access from the maze garden in Meduna
										//leading to the secret shelter underneath the palace
	L"Schuilplaats",			//The shelter underneath the queen's palace
	L"",					//Unused
};

static const wchar_t *s_dut_gpStrategicString[gpStrategicString_SIZE] =
{
	L"%ls zijn ontdekt in sector %c%d en een ander team arriveert binnenkort.",	//STR_DETECTED_SINGULAR
	L"%ls zijn ontdekt in sector %c%d en andere teams arriveren binnenkort.",	//STR_DETECTED_PLURAL
	L"Wil je een gezamenlijke aankomst coördineren?",					//STR_COORDINATE

	//Dialog strings for enemies.

	L"De vijand geeft je de kans om je over te geven.",			//STR_ENEMY_SURRENDER_OFFER
	L"De vijand heeft je overgebleven bewusteloze huurlingen gevangen.",	//STR_ENEMY_CAPTURED

	//The text that goes on the autoresolve buttons

	L"Vluchten", 	//The retreat button			//STR_AR_RETREAT_BUTTON
	L"OK",		//The done button				//STR_AR_DONE_BUTTON

	//The headers are for the autoresolve type (MUST BE UPPERCASE)

	L"VERDEDIGEN",								//STR_AR_DEFEND_HEADER
	L"AANVALLEN",								//STR_AR_ATTACK_HEADER
	L"ONTDEKKEN",								//STR_AR_ENCOUNTER_HEADER
	L"Sector",		//The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

	//The battle ending conditions

	L"VICTORIE!",								//STR_AR_OVER_VICTORY
	L"NEDERLAAG!",								//STR_AR_OVER_DEFEAT
	L"OVERGEGEVEN!",							//STR_AR_OVER_SURRENDERED
	L"GEVANGEN!",								//STR_AR_OVER_CAPTURED
	L"GEVLUCHT!",								//STR_AR_OVER_RETREATED

	//These are the labels for the different types of enemies we fight in autoresolve.

	L"Militie",							//STR_AR_MILITIA_NAME,
	L"Elite",							//STR_AR_ELITE_NAME,
	L"Troep",							//STR_AR_TROOP_NAME,
	L"Admin",							//STR_AR_ADMINISTRATOR_NAME,
	L"Wezen",							//STR_AR_CREATURE_NAME,

	//Label for the length of time the battle took

	L"Tijd verstreken",							//STR_AR_TIME_ELAPSED,

	//Labels for status of merc if retreating.  (UPPERCASE)

	L"GEVLUCHT",							//STR_AR_MERC_RETREATED,
	L"VLUCHTEN",							//STR_AR_MERC_RETREATING,
	L"VLUCHT",								//STR_AR_MERC_RETREAT,

	//PRE BATTLE INTERFACE STRINGS
	//Goes on the three buttons in the prebattle interface.  The Auto resolve button represents
	//a system that automatically resolves the combat for the player without having to do anything.
	//These strings must be short (two lines -- 6-8 chars per line)

	L"Autom. Opl.",			//!!! 1			//STR_PB_AUTORESOLVE_BTN,
	L"Naar Sector",						//STR_PB_GOTOSECTOR_BTN,
	L"Terug- trekken",			//!!! 2		//STR_PB_RETREATMERCS_BTN,

	//The different headers(titles) for the prebattle interface.
	L"VIJAND ONTDEKT",						//STR_PB_ENEMYENCOUNTER_HEADER,
	L"INVASIE VIJAND",						//STR_PB_ENEMYINVASION_HEADER, // 30
	L"HINDERLAAG VIJAND",						//STR_PB_ENEMYAMBUSH_HEADER
	L"BINNENGAAN VIJANDIGE SECTOR",				//STR_PB_ENTERINGENEMYSECTOR_HEADER
	L"AANVAL BEEST",							//STR_PB_CREATUREATTACK_HEADER
	L"BLOODCAT VAL",							//STR_PB_BLOODCATAMBUSH_HEADER
	L"BINNENGAAN HOL BLOODCAT",			//STR_PB_ENTERINGBLOODCATLAIR_HEADER

	//Various single words for direct translation.  The Civilians represent the civilian
	//militia occupying the sector being attacked.  Limited to 9-10 chars

	L"Locatie",
	L"Vijanden",
	L"Huurlingen",
	L"Milities",
	L"Beesten",
	L"Bloodcats",
	L"Sector",
	L"Geen",		//If there are no uninvolved mercs in this fight.
	L"NVT",			//Acronym of Not Applicable
	L"d",			//One letter abbreviation of day
	L"u",			//One letter abbreviation of hour

	//TACTICAL PLACEMENT USER INTERFACE STRINGS
	//The four buttons

	L"Weggaan",
	L"Verspreid",
	L"Groeperen",
	L"OK",

	//The help text for the four buttons.  Use \n to denote new line (just like enter).

	L"Maakt posities van huurlingen vrij en\nmaakt handmatig herinvoer mogelijk. (|C)",
	L"Ver|spreidt willekeurig je huurlingen\nelke keer als je de toets indrukt.",
	L"Hiermee is het mogelijk de huurlingen te |groeperen.",
	L"Druk op deze toets als je klaar bent met\nhet positioneren van je huurlingen. (|E|n|t|e|r)",
	L"Je moet al je huurlingen positioneren\nvoor je het gevecht kunt starten.",

	//Various strings (translate word for word)

	L"Sector",
	L"Kies posities binnenkomst",

	//Strings used for various popup message boxes.  Can be as long as desired.

	L"Ziet er hier niet goed uit. Het is onbegaanbaar. Probeer een andere locatie.",
	L"Plaats je huurlingen in de gemarkeerde sectie van de kaart.",

	//These entries are for button popup help text for the prebattle interface.  All popup help
	//text supports the use of \n to denote new line.  Do not use spaces before or after the \n.
	L"Lost het gevecht |Automatisch\nop zonder de kaart te laden.",
	L"Automatisch oplossen niet\nmogelijk als de speler aanvalt.",
	L"Ga sector binnen om tegen\nde vijand te strijden. (|E)",
	L"T|rek groep terug en ga naar de vorige sector.",			//singular version
	L"T|rek alle groepen terug en\nga naar hun vorige sectors.", //multiple groups with same previous sector

	//various popup messages for battle conditions.

	//%c%d is the sector -- ex:  A9
	L"Vijanden vallen je militie aan in sector %c%d.",
	//%c%d is the sector -- ex:  A9
	L"Beesten vallen je militie aan in sector %c%d.",
	//1st %d refers to the number of civilians eaten by monsters,  %c%d is the sector -- ex:  A9
	//Note:  the minimum number of civilians eaten will be two.
	L"Beesten vallen aan en doden %d burgers in sector %ls.",
	//%ls is the sector location -- ex:  A9: Omerta
	L"Vijand valt je huurlingen aan in sector %ls. Geen enkele huurling kan vechten!",
	//%ls is the sector location -- ex:  A9: Omerta
	L"Beesten vallen je huurlingen aan in sector %ls. Geen enkele huurling kan vechten!",

};

//This is the day represented in the game clock.  Must be very short, 4 characters max.
static const wchar_t s_dut_gpGameClockString[] = L"Dag";

//When the merc finds a key, they can get a description of it which
//tells them where and when they found it.
static const wchar_t *s_dut_sKeyDescriptionStrings[sKeyDescriptionStrings_SIZE] =
{
	L"Sector gevonden:",
	L"Dag gevonden:",
};

//The headers used to describe various weapon statistics.

static StrPointer s_dut_gWeaponStatsDesc[ gWeaponStatsDesc_SIZE] =
{
	L"Gewicht (%ls):",
	L"Status:",
	L"Munitie:", 		// Number of bullets left in a magazine
	L"Afst:",		// Range
	L"Sch:",		// Damage
	L"AP:",			// abbreviation for Action Points
	L"="
};

//The headers used for the merc's money.

static const wchar_t *s_dut_gMoneyStatsDesc[gMoneyStatsDesc_SIZE] =
{
	L"Bedrag",
	L"Restbedrag:", //this is the overall balance
	L"Bedrag",
	L"Splitsen:", // the amount he wants to separate from the overall balance to get two piles of money

	L"Huidig",
	L"Saldo",
	L"Bedrag naar",
	L"Opnemen",
};

//The health of various creatures, enemies, characters in the game. The numbers following each are for comment
//only, but represent the precentage of points remaining.

static const wchar_t *s_dut_zHealthStr[zHealthStr_SIZE] =
{
	L"STERVEND",	//	>= 0
	L"KRITIEK", 	//	>= 15
	L"SLECHT",		//	>= 30
	L"GEWOND",    	//	>= 45
	L"GEZOND",    	//	>= 60
	L"STERK",     	// 	>= 75
	L"EXCELLENT",	// 	>= 90
};

static const wchar_t *s_dut_gzMoneyAmounts[gzMoneyAmounts_SIZE] =
{
	L"$1000",
	L"$100",
	L"$10",
	L"OK",
	L"Splitsen",
	L"Opnemen",
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
static const wchar_t s_dut_gzProsLabel[] = L"Voor:";
static const wchar_t s_dut_gzConsLabel[] = L"Tegen:";

//Conversation options a player has when encountering an NPC
static StrPointer s_dut_zTalkMenuStrings[zTalkMenuStrings_SIZE] =
{
	L"Wat?", 		//meaning "Repeat yourself"
	L"Aardig",		//approach in a friendly
	L"Direct",		//approach directly - let's get down to business
	L"Dreigen",		//approach threateningly - talk now, or I'll blow your face off
	L"Geef",
	L"Rekruut",		//recruit
};

//Some NPCs buy, sell or repair items. These different options are available for those NPCs as well.
static StrPointer s_dut_zDealerStrings[zDealerStrings_SIZE] =
{
	L"Koop/Verkoop",	//Buy/Sell
	L"Koop",		//Buy
	L"Verkoop",		//Sell
	L"Repareer",		//Repair
};

static const wchar_t s_dut_zDialogActions[] = L"OK";


//These are vehicles in the game.

static const wchar_t *s_dut_pVehicleStrings[pVehicleStrings_SIZE] =
{
 L"Eldorado",
 L"Hummer",			// a hummer jeep/truck -- military vehicle
 L"Koeltruck",		// Icecream Truck
 L"Jeep",
 L"Tank",
 L"Helikopter",
};

static const wchar_t *s_dut_pShortVehicleStrings[pVehicleStrings_SIZE] =
{
	L"Eldor.",
	L"Hummer",			// the HMVV
	L"Truck",
	L"Jeep",
	L"Tank",
	L"Heli", 				// the helicopter
};

static const wchar_t *s_dut_zVehicleName[pVehicleStrings_SIZE] =
{
	L"Eldorado",
	L"Hummer",		//a military jeep. This is a brand name.
	L"Truck",			// Ice cream truck
	L"Jeep",
	L"Tank",
	L"Heli", 		//an abbreviation for Helicopter
};


//These are messages Used in the Tactical Screen

static StrPointer s_dut_TacticalStr[TacticalStr_SIZE] =
{
	L"Luchtaanval",
	L"Automatisch EHBO toepassen?",

	// CAMFIELD NUKE THIS and add quote #66.

	L"%ls ziet dat er items missen van de lading.",

	// The %ls is a string from pDoorTrapStrings

	L"Het slot heeft %ls.",
	L"Er is geen slot.",
	L"Geen boobytrap op het slot.",
	// The %ls is a merc name
	L"%ls heeft niet de juiste sleutel.",
	L"Slot heeft geen boobytrap.",
	L"Op slot.",
	L"DEUR",
	L"VAL",
	L"OP SLOT",
	L"OPEN",
	L"KAPOT",
	L"Hier zit een schakelaar. Activeren?",
	L"Boobytrap ontmantelen?",
	L"Meer...",

	// In the next 2 strings, %ls is an item name

	L"%ls is op de grond geplaatst.",
	L"%ls is gegeven aan %ls.",

	// In the next 2 strings, %ls is a name

	L"%ls is helemaal betaald.",
	L"%ls heeft tegoed nog %d.",
	L"Kies detonatie frequentie:",  	//in this case, frequency refers to a radio signal
	L"Aantal beurten tot ontploffing:",	//how much time, in turns, until the bomb blows
	L"Stel frequentie in van ontsteking:", 	//in this case, frequency refers to a radio signal
	L"Boobytrap ontmantelen?",
	L"Blauwe vlag weghalen?",
	L"Blauwe vlag hier neerzetten?",
	L"Laatste beurt",

	// In the next string, %ls is a name. Stance refers to way they are standing.

	L"Zeker weten dat je %ls wil aanvallen?",
	L"Ah, voertuigen kunnen plaats niet veranderen.",
	L"De robot kan niet van plaats veranderen.",

	// In the next 3 strings, %ls is a name

	L"%ls kan niet naar die plaats gaan.",
	L"%ls kan hier geen EHBO krijgen.",
	L"%ls heeft geen EHBO nodig.",
	L"Kan daar niet heen.",
	L"Je team is vol. Geen ruimte voor rekruut.",	//there's no room for a recruit on the player's team

	// In the next string, %ls is a name

	L"%ls is gerekruteerd.",

	// Here %ls is a name and %d is a number

	L"%ls ontvangt $%d.",

	// In the next string, %ls is a name

	L"%ls begeleiden?",

	// In the next string, the first %ls is a name and the second %ls is an amount of money (including $ sign)

	L"%ls inhuren voor %ls per dag?",

	// This line is used repeatedly to ask player if they wish to participate in a boxing match.

	L"Wil je vechten?",

	// In the next string, the first %ls is an item name and the
	// second %ls is an amount of money (including $ sign)

	L"%ls kopen voor %ls?",

	// In the next string, %ls is a name

	L"%ls wordt begeleid door team %d.",

	// These messages are displayed during play to alert the player to a particular situation

	L"GEBLOKKEERD",					//weapon is jammed.
	L"Robot heeft %ls kal. munitie nodig.",		//Robot is out of ammo
	L"Hier gooien? Kan niet.",		//Merc can't throw to the destination he selected

	// These are different buttons that the player can turn on and off.

	L"Sluipmodus (|Z)",			// L"Stealth Mode (|Z)",
	L"Landkaart (|M)",			// L"|Map Screen",
	L"OK (Ein|de)",				// L"|Done (End Turn)",
	L"Praat",					// L"Talk",
	L"Stil",					// L"Mute",
	L"Omhoog (|P|g|U|p)",			// L"Stance Up (|P|g|U|p)",
	L"Cursor Niveau (|T|a|b)",		// L"Cursor Level (|T|a|b)",
	L"Klim / Spring",				// L"Climb / Jump",
	L"Omlaag (|P|g|D|n)",			// L"Stance Down (|P|g|D|n)",
	L"Bekijk (|C|t|r|l)",			// L"Examine (|C|t|r|l)",
	L"Vorige huurling",			// L"Previous Merc",
	L"Volgende huurling (|S|p|a|c|e)",		// L"Next Merc (|S|p|a|c|e)",
	L"|Opties",					// L"|Options",
	L"Salvo's (|B)",				// L"|Burst Mode",
	L"Kijk/draai (|L)",			// L"|Look/Turn",
	L"Gezond: %d/%d\nKracht: %d/%d\nMoraal: %ls",		// L"Health: %d/%d\nEnergy: %d/%d\nMorale: %ls",
	L"Hé?",					//this means "what?"
	L"Door",					//an abbrieviation for "Continued"
	L"%ls is praat weer.",			// L"Mute off for %ls.",
	L"%ls is stil.",				// L"Mute on for %ls.",
	L"Gezond: %d/%d\nBrandst: %d/%d",	// L"Health: %d/%d\nFuel: %d/%d",
	L"Stap uit voertuig",			// L"Exit Vehicle" ,
	L"Wissel Team ( |S|h|i|f|t |S|p|a|c|e )",			// L"Change Squad ( |S|h|i|f|t |S|p|a|c|e )",
	L"Rijden",					// L"Drive",
	L"Nvt",						//this is an acronym for "Not Applicable."
	L"Actie ( Man-tot-man )",		// L"Use ( Hand To Hand )",
	L"Actie ( Firearm )",			// L"Use ( Firearm )",
	L"Actie ( Mes )",				// L"Use ( Blade )",
	L"Actie ( Explosieven )",		// L"Use ( Explosive )",
	L"Actie ( EHBO )",			// L"Use ( Medkit )",
	L"(Vang)",					// L"(Catch)",
	L"(Herlaad)",				// L"(Reload)",
	L"(Geef)",					// L"(Give)",
	L"%ls is afgezet.",			// L"%ls has been set off.",
	L"%ls is gearriveerd.",			// L"%ls has arrived.",
	L"%ls heeft geen Actie Punten.",	// L"%ls ran out of Action Points.",
	L"%ls is niet beschikbaar.",		// L"%ls isn't available.",
	L"%ls zit onder het verband.",		// L"%ls is all bandaged.",
	L"Verband van %ls is op.",		// L"%ls is out of bandages.",
	L"Vijand in de sector!",		// L"Enemy in sector!",
	L"Geen vijanden in zicht.",		// L"No enemies in sight.",
	L"Niet genoeg Actie Punten.",		// L"Not enough Action Points.",
	L"Niemand gebruikt afstandb.",	// L"Nobody's using the remote.",
	L"Magazijn leeg door salvovuur!",	// L"Burst fire emptied the clip!",
	L"SOLDAAT",					// L"SOLDIER",
	L"CREPITUS",				// L"CREPITUS",
	L"MILITIE",					// L"MILITIA",
	L"BURGER",					// L"CIVILIAN",
	L"Verlaten Sector",			// L"Exiting Sector",
	L"OK",
	L"Stoppen",					// L"Cancel",
	L"Huurling gesel.",			// L"Selected Merc",
	L"Alle huurl. in team",			// L"All Mercs in Squad",
	L"Naar Sector",				// L"Go to Sector",
	L"Naar Landk.",				// L"Go to Map",
	L"Vanaf deze kant kun je de sector niet verlaten.",	// L"You can't leave the sector from this side.",
	L"%ls is te ver weg.",			// L"%ls is too far away.",
	L"Verwijder Boomtoppen",		// L"Removing Treetops",
	L"Tonen Boomtoppen",			// L"Showing Treetops",
	L"KRAAI",				//Crow, as in the large black bird
	L"NEK",
	L"HOOFD",
	L"TORSO",
	L"BENEN",
	L"De Koningin vertellen wat ze wil weten?",		// L"Tell the Queen what she wants to know?",
	L"Vingerafdruk-ID nodig",					// L"Fingerprint ID aquired",
	L"Vingerafdruk-ID ongeldig. Wapen funct. niet",		// L"Invalid fingerprint ID. Weapon non-functional",
	L"Doelwit nodig",					// L"Target aquired",
	L"Pad geblokkeerd",				// L"Path Blocked",
	L"Geld Storten/Opnemen",		//Help text over the $ button on the Single Merc Panel ("Deposit/Withdraw Money")
	L"Niemand heeft EHBO nodig.",			// L"No one needs first aid.",
	L"Vast.",						// Short form of JAMMED, for small inv slots
	L"Kan daar niet heen.",				// used ( now ) for when we click on a cliff
	L"Persoon weigert weg te gaan.",
	// In the following message, '%ls' would be replaced with a quantity of money (e.g. $200)
	L"Ben je het eens met %ls?",					// L"Do you agree to pay %ls?",
	L"Wil je kostenloze medische hulp?",			// L"Accept free medical treatment?",
	L"Wil je trouwen met Daryl?",					// L"Agree to marry Daryl?",
	L"Slot Ring Paneel",						// L"Key Ring Panel",
	L"Dat kan niet met een EPC.",					// L"You cannot do that with an EPC.",
	L"Krott sparen?",							// L"Spare Krott?",
	L"Buiten wapenbereik",						// L"Out of weapon range",
	L"Mijnwerker",							// L"Miner",
	L"Voertuig kan alleen tussen sectors reizen",		// L"Vehicle can only travel between sectors",
	L"Nu geen Auto-EHBO mogelijk",				// L"Can't autobandage right now",
	L"Pad Geblokkeerd voor %ls",					// L"Path Blocked for %ls",
	L"Je huurlingen, gevangen door Deidranna's leger, zitten hier opgesloten!",
	L"Slot geraakt",							// L"Lock hit",
	L"Slot vernielt",							// L"Lock destroyed",
	L"Iemand anders probeert deze deur te gebruiken.",		// L"Somebody else is trying to use this door.",
	L"Gezondheid: %d/%d\nBrandstof: %d/%d",				//L"Health: %d/%d\nFuel: %d/%d",
	L"%ls kan %ls niet zien.",					// Cannot see person trying to talk to
};

//Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different circumstances in the "exiting sector" interface.
static const wchar_t *s_dut_pExitingSectorHelpText[pExitingSectorHelpText_SIZE] =
{
	//Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box is checked.
	L"Als aangekruist, dan wordt de aanliggende sector meteen geladen.",
	L"Als aangekruist, dan worden de huurlingen automatisch op de\nkaart geplaatst rekening houdend met reistijden.",

	//If you attempt to leave a sector when you have multiple squads in a hostile sector.
	L"Deze sector is door de vijand bezet en huurlingen kun je niet achterlaten.\nJe moet deze situatie oplossen voor het laden van andere sectors.",

	//Because you only have one squad in the sector, and the "move all" option is checked, the "go to sector" option is locked to on.
	//The helptext explains why it is locked.
	L"Als de overgebleven huurlingen uit deze sector trekken,\nwordt de aanliggende sector onmiddellijk geladen.",
	L"Als de overgebleven huurlingen uit deze sector trekken,\nwordt je automatisch in het landkaartscherm geplaatst,\nrekening houdend met de reistijd van je huurlingen.",

	//If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being escorted.  The "single" button is disabled.
	L"%ls moet geëscorteerd worden door jouw huurlingen\nen kan de sector niet alleen verlaten.",

	//If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc will be prohibited from leaving alone.
	//There are several strings depending on the gender of the merc and how many EPCs are in the squad.
	//DO NOT USE THE NEWLINE HERE AS IT IS USED FOR BOTH HELPTEXT AND SCREEN MESSAGES!
	L"%ls kan de sector niet alleen verlaten omdat hij %ls escorteert.",	//male singular
	L"%ls kan de sector niet alleen verlaten omdat zij %ls escorteert.",	//female singular
	L"%ls kan de sector niet alleen verlaten omdat hij meerdere karakters escorteert.",	//male plural
	L"%ls kan de sector niet alleen verlaten omdat zij meerdere karakters escorteert.",	//female plural

	//If one or more of your mercs in the selected squad aren't in range of the traversal area, then the  "move all" option is disabled,
	//and this helptext explains why.
	L"Al je huurlingen moeten in de buurt zijn om het team te laten reizen.",

	//Standard helptext for single movement.  Explains what will happen (splitting the squad)
	L"Als aangekruist, dan zal %ls alleen verder reizen\nen automatisch bij een uniek team gevoegd worden.",

	//Standard helptext for all movement.  Explains what will happen (moving the squad)
	L"Als aangekruist, dan zal je geselecteerde\nteam verder reizen, de sector verlatend.",

	//This strings is used BEFORE the "exiting sector" interface is created.  If you have an EPC selected and you attempt to tactically
	//traverse the EPC while the escorting mercs aren't near enough (or dead, dying, or unconscious), this message will appear and the
	//"exiting sector" interface will not appear.  This is just like the situation where
	//This string is special, as it is not used as helptext.  Do not use the special newline character (\n) for this string.
	L"%ls wordt geëscorteerd door jouw huurlingen en kan de sector niet alleen verlaten. Je huurlingen moeten eerst in de buurt zijn.",
};



static const wchar_t *s_dut_pRepairStrings[pRepairStrings_SIZE] =
{
	L"Items", 		// tell merc to repair items in inventory
	L"SAM-Stelling", 		// tell merc to repair SAM site - SAM is an acronym for Surface to Air Missile
	L"Stop", 		// cancel this menu
	L"Robot", 		// repair the robot
};


// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."

static const wchar_t *s_dut_sPreStatBuildString[sPreStatBuildString_SIZE] =
{
	L"verliest", 	// the merc has lost a statistic
	L"krijgt", 		// the merc has gained a statistic
	L"punt voor",	// singular
	L"punten voor",	// plural
	L"niveau voor",	// singular
	L"niveaus voor",	// plural
};

static const wchar_t *s_dut_sStatGainStrings[sStatGainStrings_SIZE] =
{
	L"gezondheid.",
	L"beweeglijkheid.",
	L"handigheid.",
	L"wijsheid.",
	L"medisch kunnen.",
	L"explosieven.",
	L"technisch kunnen.",
	L"trefzekerheid.",
	L"ervaring.",
	L"kracht.",
	L"leiderschap.",
};


static const wchar_t *s_dut_pHelicopterEtaStrings[pHelicopterEtaStrings_SIZE] =
{
	L"Totale Afstand: ", 			// total distance for helicopter to travel
	L" Veilig: ", 			// distance to travel to destination
	L" Onveilig:", 			// distance to return from destination to airport
	L"Totale Kosten: ", 		// total cost of trip by helicopter
	L"Aank: ", 				// ETA is an acronym for "estimated time of arrival"
	L"Helikopter heeft weinig brandstof en moet landen in vijandelijk gebied!",	// warning that the sector the helicopter is going to use for refueling is under enemy control ->
	L"Passagiers: ",
	L"Selecteer Skyrider of Aanvoer Drop-plaats?",			// L"Select Skyrider or the Arrivals Drop-off?",
	L"Skyrider",
	L"Aanvoer",						// L"Arrivals",
};

static const wchar_t s_dut_sMapLevelString[] = L"Subniv.:"; // what level below the ground is the player viewing in mapscreen ("Sublevel:")

static const wchar_t s_dut_gsLoyalString[] = L"%d%% Loyaal"; // the loyalty rating of a town ie : Loyal 53%


// error message for when player is trying to give a merc a travel order while he's underground.
static const wchar_t s_dut_gsUndergroundString[] = L"kan geen reisorders ondergronds ontvangen.";

static const wchar_t *s_dut_gsTimeStrings[gsTimeStrings_SIZE] =
{
	L"u",				// hours abbreviation
	L"m",				// minutes abbreviation
	L"s",				// seconds abbreviation
	L"d",				// days abbreviation
};

// text for the various facilities in the sector

static const wchar_t *s_dut_sFacilitiesStrings[sFacilitiesStrings_SIZE] =
{
	L"Geen",
	L"Ziekenhuis",
	L"Industrie",
	L"Gevangenis",
	L"Krijgsmacht",
	L"Vliegveld",
	L"Schietterrein",		// a field for soldiers to practise their shooting skills
};

// text for inventory pop up button

static const wchar_t *s_dut_pMapPopUpInventoryText[pMapPopUpInventoryText_SIZE] =
{
	L"Inventaris",
	L"OK",
};

// town strings

static const wchar_t *s_dut_pwTownInfoStrings[pwTownInfoStrings_SIZE] =
{
	L"Grootte",					// size of the town in sectors
	L"Gezag", 					// how much of town is controlled
	L"Verboden Mijn", 				// mine associated with this town
	L"Loyaliteit",					// the loyalty level of this town
	L"Voorzieningen", 				// main facilities in this town
	L"Training Burgers",				// state of civilian training in town
	L"Militie", 					// the state of the trained civilians in the town
};

// Mine strings

static const wchar_t *s_dut_pwMineStrings[pwMineStrings_SIZE] =
{
	L"Mijn",					// 0
	L"Zilver",
	L"Goud",
	L"Dagelijkse prod.",
	L"Mogelijke prod.",
	L"Verlaten",				// 5
	L"Gesloten",
	L"Raakt Op",
	L"Produceert",
	L"Status",
	L"Prod. Tempo",
	L"Ertstype",				// 10
	L"Gezag Dorp",
	L"Loyaliteit Dorp",
};

// blank sector strings

static const wchar_t *s_dut_pwMiscSectorStrings[pwMiscSectorStrings_SIZE] =
{
	L"Vijandelijke troepen",
	L"Sector",
	L"# Items",
	L"Onbekend",
	L"Gecontrolleerd",
	L"Ja",
	L"Nee",
};

// error strings for inventory

static const wchar_t *s_dut_pMapInventoryErrorString[pMapInventoryErrorString_SIZE] =
{
	L"Kan huurling niet selecteren.",		//MARK CARTER
	L"%ls is niet in de sector om dat item te pakken.",
	L"Tijdens gevechten moet je items handmatig oppakken.",
	L"Tijdens gevechten moet je items handmatig neerleggen.",
	L"%ls is niet in de sector om dat item neer te leggen.",
};

static const wchar_t *s_dut_pMapInventoryStrings[pMapInventoryStrings_SIZE] =
{
	L"Locatie", 		// sector these items are in
	L"Aantal Items", 		// total number of items in sector
};


// movement menu text

static const wchar_t *s_dut_pMovementMenuStrings[pMovementMenuStrings_SIZE] =
{
	L"Huurlingen in Sector %ls", 	// title for movement box
	L"Teken Reisroute", 		// done with movement menu, start plotting movement
	L"Stop", 		// cancel this menu
	L"Anders",		// title for group of mercs not on squads nor in vehicles
};


static const wchar_t *s_dut_pUpdateMercStrings[pUpdateMercStrings_SIZE] =
{
	L"Oeps:", 			// an error has occured
	L"Contract Huurling verlopen:", 	// this pop up came up due to a merc contract ending
	L"Huurling Taak Volbracht:", // this pop up....due to more than one merc finishing assignments
	L"Huurling weer aan het Werk:", // this pop up ....due to more than one merc waking up and returing to work
	L"Huurling zegt Zzzzzzz:", // this pop up ....due to more than one merc being tired and going to sleep
	L"Contract Loopt Bijna Af:", 	// this pop up came up due to a merc contract ending
};

// map screen map border buttons help text

static const wchar_t *s_dut_pMapScreenBorderButtonHelpText[pMapScreenBorderButtonHelpText_SIZE] =
{
	L"Toon Dorpen (|W)",
	L"Toon |Mijnen",
	L"Toon |Teams & Vijanden",
	L"Toon Luchtruim (|A)",
	L"Toon |Items",
	L"Toon Milities & Vijanden (|Z)",
};


static const wchar_t *s_dut_pMapScreenBottomFastHelp[pMapScreenBottomFastHelp_SIZE] =
{
	L"|Laptop",
	L"Tactisch (|E|s|c)",
	L"|Opties",
	L"TijdVersneller (|+)", 	// time compress more
	L"TijdVersneller (|-)", 	// time compress less
	L"Vorig Bericht (|U|p)\nVorige Pagina (|P|g|U|p)", 	// previous message in scrollable list
	L"Volgend Bericht (|D|o|w|n)\nVolgende pagina (|P|g|D|n)", 	// next message in the scrollable list
	L"Start/Stop Tijd (|S|p|a|c|e)",	// start/stop time compression
};

static const wchar_t s_dut_pMapScreenBottomText[] = L"Huidig Saldo"; // current balance in player bank account

static const wchar_t s_dut_pMercDeadString[] = L"%ls is dood.";


static const wchar_t s_dut_pDayStrings[] = L"Dag";

// the list of email sender names

static const wchar_t *s_dut_pSenderNameList[pSenderNameList_SIZE] =
{
	L"Enrico",
	L"Psych Pro Inc",
	L"Help Desk",
	L"Psych Pro Inc",
	L"Speck",
	L"R.I.S.",		//5
	L"Barry",
	L"Blood",
	L"Lynx",
	L"Grizzly",
	L"Vicki",		//10
	L"Trevor",
	L"Grunty",
	L"Ivan",
	L"Steroid",
	L"Igor",		//15
	L"Shadow",
	L"Red",
	L"Reaper",
	L"Fidel",
	L"Fox",		//20
	L"Sidney",
	L"Gus",
	L"Buns",
	L"Ice",
	L"Spider",		//25
	L"Cliff",
	L"Bull",
	L"Hitman",
	L"Buzz",
	L"Raider",		//30
	L"Raven",
	L"Static",
	L"Len",
	L"Danny",
	L"Magic",
	L"Stephan",
	L"Scully",
	L"Malice",
	L"Dr.Q",
	L"Nails",
	L"Thor",
	L"Scope",
	L"Wolf",
	L"MD",
	L"Meltdown",
	//----------
	L"M.I.S. Verzekeringen",
	L"Bobby Rays",
	L"Kingpin",
	L"John Kulba",
	L"A.I.M.",
};


// new mail notify string
static const wchar_t s_dut_pNewMailStrings[] = L"Je hebt nieuwe berichten...";


// confirm player's intent to delete messages

static const wchar_t *s_dut_pDeleteMailStrings[pDeleteMailStrings_SIZE] =
{
 L"Bericht verwijderen?",
 L"ONGELEZEN bericht(en) verwijderen?",
};


// the sort header strings

static const wchar_t *s_dut_pEmailHeaders[pEmailHeaders_SIZE] =
{
	L"Van:",
	L"Subject:",
	L"Dag:",
};

// email titlebar text
static const wchar_t s_dut_pEmailTitleText[] = L"Postvak";


// the financial screen strings
static const wchar_t s_dut_pFinanceTitle[] = L"Account Plus"; // the name we made up for the financial program in the game

static const wchar_t *s_dut_pFinanceSummary[pFinanceSummary_SIZE] =
{
	L"Credit:", 				// credit (subtract from) to player's account
	L"Debet:", 				// debit (add to) to player's account
	L"Saldo Gisteren:",
	L"Stortingen Gisteren:",
	L"Uitgaven Gisteren:",
	L"Saldo Eind van de Dag:",
	L"Saldo Vandaag:",
	L"Stortingen Vandaag:",
	L"Uitgaven Vandaag:",
	L"Huidig Saldo:",
	L"Voorspelde Inkomen:",
	L"Geschat Saldo:", 	// projected balance for player for tommorow
};


// headers to each list in financial screen

static const wchar_t *s_dut_pFinanceHeaders[pFinanceHeaders_SIZE] =
{
	L"Dag", 			// the day column
	L"Credit", 			// the credits column (to ADD money to your account)
	L"Debet",			// the debits column (to SUBTRACT money from your account)
	L"Transactie", 		// transaction type - see TransactionText below
	L"Saldo", 			// balance at this point in time
	L"Pag.", 			// page number
	L"Dag(en)", 		// the day(s) of transactions this page displays
};


static const wchar_t *s_dut_pTransactionText[pTransactionText_SIZE] =
{
	L"Toegenomen Interest",			// interest the player has accumulated so far
	L"Anonieme Storting",
	L"Transactiekosten",
	L"%ls ingehuurd van AIM", // Merc was hired
	L"Bobby Ray's Wapenhandel", 		// Bobby Ray is the name of an arms dealer
	L"Rekeningen Voldaan bij M.E.R.C.",
	L"Medische Storting voor %ls", 		// medical deposit for merc
	L"IMP Profiel Analyse", 		// IMP is the acronym for International Mercenary Profiling
	L"Verzekering Afgesloten voor %ls",
	L"Verzekering Verminderd voor %ls",
	L"Verzekering Verlengd voor %ls", 				// johnny contract extended
	L"Verzekering Afgebroken voor %ls",
	L"Verzekeringsclaim voor %ls", 		// insurance claim for merc
	L"Contract %ls verl. met 1 dag.", 		// entend mercs contract by a day
	L"Contract %ls verl. met 1 week.",
	L"Contract %ls verl. met 2 weken.",
	L"Inkomen Mijn",
	L"",						 //String nuked
	L"Gekochte Bloemen",
	L"Volledige Medische Vergoeding voor %ls",
	L"Gedeeltelijke Medische Vergoeding voor %ls",
	L"Geen Medische Vergoeding voor %ls",
	L"Betaling aan %ls",				// %ls is the name of the npc being paid
	L"Maak Geld over aan %ls", 		// transfer funds to a merc
	L"Maak Geld over van %ls", 		// transfer funds from a merc
	L"Rust militie uit in %ls",			// initial cost to equip a town's militia
	L"Items gekocht van %ls.",		//is used for the Shop keeper interface.  The dealers name will be appended to the end of the string.
	L"%ls heeft geld gestort.",
};

// helicopter pilot payment

static const wchar_t *s_dut_pSkyriderText[pSkyriderText_SIZE] =
{
	L"Skyrider is $%d betaald.", 			// skyrider was paid an amount of money
	L"Skyrider heeft $%d tegoed.", 		// skyrider is still owed an amount of money
	L"Skyrider heeft geen passagiers. Als je huurlingen in deze sector wil vervoeren, wijs ze dan eerst toe aan Voertuig/Helikopter.",
};


// strings for different levels of merc morale

static const wchar_t *s_dut_pMoralStrings[pMoralStrings_SIZE] =
{
	L"Super",
	L"Goed",
	L"Stabiel",
	L"Mager",
	L"Paniek",
	L"Slecht",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.
static const wchar_t s_dut_str_left_equipment[]   = L"%ls's uitrusting is nu beschikbaar in %ls (%c%d).";

// Status that appears on the Map Screen

static const wchar_t *s_dut_pMapScreenStatusStrings[pMapScreenStatusStrings_SIZE] =
{
	L"Gezondheid",
	L"Energie",
	L"Moraal",
	L"Conditie",	// the condition of the current vehicle (its "health")
	L"Brandstof",	// the fuel level of the current vehicle (its "energy")
};


static const wchar_t *s_dut_pMapScreenPrevNextCharButtonHelpText[pMapScreenPrevNextCharButtonHelpText_SIZE] =
{
	L"Vorige Huurling (|L|e|f|t)", 			// previous merc in the list
	L"Volgende Huurling (|R|i|g|h|t)", 				// next merc in the list
};


static const wchar_t s_dut_pEtaString[] = L"aank:"; // eta is an acronym for Estimated Time of Arrival

static const wchar_t *s_dut_pTrashItemText[pTrashItemText_SIZE] =
{
	L"Je bent het voor altijd kwijt. Zeker weten?", 	// do you want to continue and lose the item forever
	L"Dit item ziet er HEEL belangrijk uit. Weet je HEEL, HEEL zeker dat je het wil weggooien?", // does the user REALLY want to trash this item

};


static const wchar_t *s_dut_pMapErrorString[pMapErrorString_SIZE] =
{
	L"Team kan niet verder reizen met een slapende huurling.",

//1-5
	L"Verplaats het team eerst bovengronds.",
	L"Reisorders? Het is vijandig gebied!",
	L"Om te verplaatsen moeten huurlingen eerst toegewezen worden aan een team of voertuig.",
	L"Je hebt nog geen team-leden.", 			// you have no members, can't do anything
	L"Huurling kan order niet opvolgen.",			 		// merc can't comply with your order
//6-10
	L"%ls heeft een escorte nodig. Plaats hem in een team.", // merc can't move unescorted .. for a male
	L"%ls heeft een escorte nodig. Plaats haar in een team.", // for a female
	L"Huurling is nog niet in Arulco aangekomen!",
	L"Het lijkt erop dat er eerst nog contractbesprekingen gehouden moeten worden.",
	L"",
//11-15
	L"Reisorders? Er is daar een gevecht gaande!",
	L"Je bent in een hinderlaag gelokt van Bloodcats in sector %ls!",
	L"Je bent in sector I16 iets binnengelopen dat lijkt op het hol van een bloodcat!",
	L"",
	L"De SAM-stelling in %ls is overgenomen.",
//16-20
	L"De mijn in %ls is overgenomen. Je dagelijkse inkomen is gereduceerd tot %ls per dag.",
	L"De vijand heeft sector %ls onbetwist overgenomen.",
	L"Tenminste een van je huurlingen kan niet meedoen met deze opdracht.",
	L"%ls kon niet meedoen met %ls omdat het al vol is",
	L"%ls kon niet meedoen met %ls omdat het te ver weg is.",
//21-25
	L"De mijn in %ls is buitgemaakt door Deidranna's troepen!",
	L"Deidranna's troepen zijn net de SAM-stelling in %ls binnengevallen",
	L"Deidranna's troepen zijn net %ls binnengevallen",
	L"Deidranna's troepen zijn gezien in %ls.",
	L"Deidranna's troepen hebben zojuist %ls overgenomen.",
//26-30
	L"Tenminste één huurling kon niet tot slapen gebracht worden.",
	L"Tenminste één huurling kon niet wakker gemaakt worden.",
	L"De Militie verschijnt niet totdat hun training voorbij is.",
	L"%ls kan geen reisorders gegeven worden op dit moment.",
	L"Milities niet binnen de stadsgrenzen kunnen niet verplaatst worden naar een andere sector.",
//31-35
	L"Je kunt geen militie in %ls hebben.",
	L"Een voertuig kan niet leeg rijden!",
	L"%ls is te gewond om te reizen!",
	L"Je moet het museum eerst verlaten!",
	L"%ls is dood!",
//36-40
	L"%ls kan niet wisselen naar %ls omdat het onderweg is",
	L"%ls kan het voertuig op die manier niet in",
	L"%ls kan zich niet aansluiten bij %ls",
	L"Totdat je nieuwe huurlingen in dienst neemt, kan de tijd niet versneld worden!",
	L"Dit voertuig kan alleen over wegen rijden!",
//41-45
	L"Je kunt geen reizende huurlingen opnieuw toewijzen",
	L"Voertuig zit zonder brandstof!",
	L"%ls is te moe om te reizen.",
	L"Niemand aan boord is in staat om het voertuig te besturen.",
	L"Eén of meer teamleden kunnen zich op dit moment niet verplaatsen.",
//46-50
	L"Eén of meer leden van de ANDERE huurlingen kunnen zich op dit moment niet verplaatsen.",
	L"Voertuig is te beschadigd!",
	L"Let op dat maar twee huurlingen milities in een sector mogen trainen.",
	L"De robot kan zich zonder bediening niet verplaatsen. Plaats ze in hetzelfde team.",
};


// help text used during strategic route plotting
static const wchar_t *s_dut_pMapPlotStrings[pMapPlotStrings_SIZE] =
{
	L"Klik nogmaals op de bestemming om de route te bevestigen, of klik op een andere sector om meer routepunten te plaatsen.",
	L"Route bevestigd.",
	L"Bestemming onveranderd.",
	L"Reis afgebroken.",
	L"Reis verkort.",
};


// help text used when moving the merc arrival sector
static const wchar_t *s_dut_pBullseyeStrings[pBullseyeStrings_SIZE] =
{
	L"Klik op de sector waar de huurlingen in plaats daarvan moeten arriveren.",
	L"OK. Arriverende huurlingen worden afgezet in %ls",
	L"Huurlingen kunnen hier niet ingevlogen worden, het luchtruim is onveilig!",
	L"Afgebroken. Aankomst-sector onveranderd",
	L"Luchtruim boven %ls is niet langer veilig! Aankomst-sector is verplaatst naar %ls.",
};


// help text for mouse regions

static const wchar_t *s_dut_pMiscMapScreenMouseRegionHelpText[pMiscMapScreenMouseRegionHelpText_SIZE] =
{
	L"Naar Inventaris (|E|n|t|e|r)",
	L"Gooi Item Weg",
	L"Verlaat Inventaris (|E|n|t|e|r)",
};


static const wchar_t s_dut_str_he_leaves_where_drop_equipment[]  = L"Laat %ls zijn uitrusting achterlaten waar hij nu is (%ls) of later in %ls (%ls) bij het nemen van de vlucht uit Arulco?";
static const wchar_t s_dut_str_she_leaves_where_drop_equipment[] = L"Laat %ls haar uitrusting achterlaten waar ze nu is (%ls) of later in %ls (%ls) bij het nemen van de vlucht uit Arulco?";
static const wchar_t s_dut_str_he_leaves_drops_equipment[]       = L"%ls gaat binnenkort weg en laat zijn uitrusting achter in %ls.";
static const wchar_t s_dut_str_she_leaves_drops_equipment[]      = L"%ls gaat binnenkort weg en laat haar uitrusting achter in %ls.";


// Text used on IMP Web Pages

static const wchar_t *s_dut_pImpPopUpStrings[pImpPopUpStrings_SIZE] =
{
	L"Ongeldige Autorisatiecode",
	L"Je wil het gehele persoonlijkheidsonderzoek te herstarten. Zeker weten?",
	L"Vul alsjeblieft de volledige naam en geslacht in",
	L"Voortijdig onderzoek van je financiële status wijst uit dat je een persoonlijksheidsonderzoek niet kunt betalen.",
	L"Geen geldige optie op dit moment.",
	L"Om een nauwkeurig profiel te maken, moet je ruimte hebben voor tenminste één teamlid.",
	L"Profiel is al gemaakt.",
};


// button labels used on the IMP site

static const wchar_t *s_dut_pImpButtonText[pImpButtonText_SIZE] =
{
	L"Info", 			// about the IMP site ("About Us")
	L"BEGIN", 			// begin profiling ("BEGIN")
	L"Persoonlijkheid", 		// personality section ("Personality")
	L"Eigenschappen", 		// personal stats/attributes section ("Attributes")
	L"Portret", 			// the personal portrait selection ("Portrait")
	L"Stem %d", 			// the voice selection ("Voice %d")
	L"OK", 			// done profiling ("Done")
	L"Opnieuw", 		// start over profiling ("Start Over")
	L"Ja, ik kies het geselecteerde antwoord.",		// ("Yes, I choose the highlighted answer.")
	L"Ja",
	L"Nee",
	L"OK", 			// finished answering questions
	L"Vor.", 			// previous question..abbreviated form
	L"Vol.", 			// next question
	L"JA ZEKER.", 		// yes, I am certain ("YES, I AM.")
	L"NEE, IK WIL OPNIEUW BEGINNEN.",	// no, I want to start over the profiling process ("NO, I WANT TO START OVER.")
	L"JA, ZEKER.",		// ("YES, I DO.")
	L"NEE",
	L"Terug", 					// back one page
	L"Stop", 					// cancel selection
	L"Ja, zeker weten.",			// ("Yes, I am certain.")
	L"Nee, laat me nog eens kijken.",	// ("No, let me have another look.")
	L"Registratie", 			// the IMP site registry..when name and gender is selected
	L"Analyseren", 			// analyzing your profile results
	L"OK",
	L"Stem",				// "Voice"
};

static const wchar_t *s_dut_pExtraIMPStrings[pExtraIMPStrings_SIZE] =
{
	L"Selecteer Persoonlijkheid om eigenlijke onderzoek te starten.",
	L"Nu het onderzoek compleet is, selecteer je eigenschappen.",
	L"Nu de eigenschappen gekozen zijn, kun je verder gaan met de portretselectie.",
	L"Selecteer de stem die het best bij je past om het proces te voltooien.",
};

static const wchar_t s_dut_pFilesTitle[] = L"Bestanden Bekijken"; // ("File Viewer")

static const wchar_t *s_dut_pFilesSenderList[pFilesSenderList_SIZE] =
{
	L"Int. Verslag",		// the recon report sent to the player. Recon is an abbreviation for reconissance
	L"Intercept.#1",		// first intercept file .. Intercept is the title of the organization sending the file...similar in function to INTERPOL/CIA/KGB..refer to fist record in files.txt for the translated title
	L"Intercept.#2",		// second intercept file
	L"Intercept.#3",		// third intercept file
	L"Intercept.#4",		// fourth intercept file ("Intercept #4")
	L"Intercept.#5",		// fifth intercept file
	L"Intercept.#6",		// sixth intercept file
};

// Text having to do with the History Log
static const wchar_t s_dut_pHistoryTitle[] = L"Geschiedenis";

static const wchar_t *s_dut_pHistoryHeaders[pHistoryHeaders_SIZE] =
{
	L"Dag", 			// the day the history event occurred
	L"Pag.", 			// the current page in the history report we are in
	L"Dag", 			// the days the history report occurs over
	L"Locatie", 		// location (in sector) the event occurred
	L"Geb.", 			// the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
static const wchar_t *s_dut_pHistoryStrings[pHistoryStrings_SIZE] =
{
	L"",										// leave this line blank
	//1-5
	L"%ls ingehuurd via A.I.M.",						// merc was hired from the aim site
	L"%ls ingehuurd via M.E.R.C.",						// merc was hired from the merc site
	L"%ls gedood.", 								// merc was killed
	L"Facturen betaald bij M.E.R.C.",					// paid outstanding bills at MERC
	L"Opdracht van Enrico Chivaldori geaccepteerd.",		// ("Accepted Assignment From Enrico Chivaldori")
	//6-10
	L"IMP Profiel Klaar",							// ("IMP Profile Generated")
	L"Verzekeringspolis gekocht voor %ls.",				// insurance contract purchased
	L"Verzekeringspolis afgebroken van %ls.",				// insurance contract canceled
	L"Uitbetaling Verzekeringspolis %ls.",				// insurance claim payout for merc
	L"%ls's contract verlengd met 1 dag.",				// Extented "mercs name"'s for a day
	//11-15
	L"%ls's contract verlengd met 1 week.",				// Extented "mercs name"'s for a week
	L"%ls's contract verlengd met 2 weken.",				// Extented "mercs name"'s 2 weeks
	L"%ls is ontslagen.",							// "merc's name" was dismissed.
	L"%ls gestopt.",								// "merc's name" quit.
	L"zoektocht gestart.",							// a particular quest started
	//16-20
	L"zoektocht afgesloten.",						// ("quest completed.")
	L"Gepraat met hoofdmijnwerker van %ls",				// talked to head miner of town
	L"%ls bevrijd",								// ("Liberated %ls")
	L"Vals gespeeld",								// ("Cheat Used")
	L"Voedsel zou morgen in Omerta moeten zijn",			// ("Food should be in Omerta by tomorrow")
	//21-25
	L"%ls weggegaan, wordt Daryl Hick's vrouw",			// ("%ls left team to become Daryl Hick's wife")
	L"%ls's contract afgelopen.",						// ("%ls's contract expired.")
	L"%ls aangenomen.",							// ("%ls was recruited.")
	L"Enrico klaagde over de voortgang",				// ("Enrico complained about lack of progress")
	L"Strijd gewonnen",							// ("Battle won")
	//26-30
	L"%ls mijn raakt uitgeput",						// ("%ls mine started running out of ore")
	L"%ls mijn is uitgeput",							// ("%ls mine ran out of ore")
	L"%ls mijn is gesloten",							// ("%ls mine was shut down")
	L"%ls mijn heropend",							// ("%ls mine was reopened")
	L"Info verkregen over gevangenis Tixa.",				// ("Found out about a prison called Tixa.")
	//31-35
	L"Van geheime wapenfabriek gehoord, Orta genaamd.",		// ("Heard about a secret weapons plant called Orta.")
	L"Onderzoeker in Orta geeft wat raketwerpers.",			// ("Scientist in Orta donated a slew of rocket rifles.")
	L"Koningin Deidranna kickt op lijken.",				// ("Queen Deidranna has a use for dead bodies.")
	L"Frank vertelde over knokwedstrijden in San Mona.",		// ("Frank talked about fighting matches in San Mona.")
	L"Een patiënt dacht dat ie iets in de mijnen zag.",		// ("A patient thinks he saw something in the mines.")
	//36-40
	L"Pers. ontmoet; Devin - verkoopt explosieven.",		// ("Met someone named Devin - he sells explosives.")
	L"Beroemde ex-AIM huurling Mike ontmoet!",			// ("Ran into the famous ex-AIM merc Mike!")
	L"Tony ontmoet - handelt in wapens.",				// ("Met Tony - he deals in arms.")
	L"Raketwerper gekregen van Serg. Krott.",				// ("Got a rocket rifle from Sergeant Krott.")
	L"Kyle akte gegeven van Angel's leerwinkel.",			// ("Gave Kyle the deed to Angel's leather shop.")
	//41-45
	L"Madlab bood aan robot te bouwen.",				// ("Madlab offered to build a robot.")
	L"Gabby maakt superbrouwsel tegen beesten.",			// ("Gabby can make stealth concoction for bugs.")
	L"Keith is er mee opgehouden.",					// ("Keith is out of business.")
	L"Howard geeft Koningin Deidranna cyanide.",			// ("Howard provided cyanide to Queen Deidranna.")
	L"Keith ontmoet - handelaar in Cambria.",				// ("Met Keith - all purpose dealer in Cambria.")
	//46-50
	L"Howard ontmoet - medicijnendealer in Balime",			// ("Met Howard - deals pharmaceuticals in Balime")
	L"Perko ontmoet - heeft reparatiebedrijfje.",			// ("Met Perko - runs a small repair business.")
	L"Sam van Balime ontmoet - verkoopt ijzerwaren.",		// ("Met Sam of Balime - runs a hardware shop.")
	L"Franz verkoopt elektronica en andere dingen.",		// ("Franz deals in electronics and other goods.")
	L"Arnold runt reparatiezaak in Grumm.",				// ("Arnold runs a repair shop in Grumm.")
	//51-55
	L"Fredo repareert elektronica in Grumm.",				// ("Fredo repairs electronics in Grumm.")
	L"Van rijke vent in Balime donatie gekregen.",			// ("Received donation from rich guy in Balime.")
	L"Schroothandelaar Jake ontmoet.",					// ("Met a junkyard dealer named Jake.")
	L"Vaag iemand gaf ons elektronische sleutelkaart.",		// ("Some bum gave us an electronic keycard.")
	L"Walter omgekocht om kelderdeur open te maken.",		// ("Bribed Walter to unlock the door to the basement.")
	//56-60
	L"Als Dave gas heeft, geeft hij deze weg.",			// ("If Dave has gas, he'll provide free fillups.")
	L"Geslijmd met Pablo.",							// ("Greased Pablo's palms.")
	L"Kingpin bewaard geld in San Mona mine.",			// ("Kingpin keeps money in San Mona mine.")
	L"%ls heeft Extreme Fighting gewonnen",				// ("%ls won Extreme Fighting match")
	L"%ls heeft Extreme Fighting verloren",				// ("%ls lost Extreme Fighting match")
	//61-65
	L"%ls gediskwalificeerd v. Extreme Fighting",			// ("%ls was disqualified in Extreme Fighting")
	L"Veel geld gevonden in een verlaten mijn.",			// ("Found a lot of money stashed in the abandoned mine.")
	L"Huurmoordenaar van Kingpin ontdekt.",				// ("Encountered assassin sent by Kingpin.")
	L"Controle over sector verloren",					//ENEMY_INVASION_CODE ("Lost control of sector")
	L"Sector verdedigd",							// ("Defended sector")
	//66-70
	L"Strijd verloren",					//ENEMY_ENCOUNTER_CODE ("Lost battle")
	L"Fatale val",						//ENEMY_AMBUSH_CODE ("Fatal ambush")
	L"Vijandige val weggevaagd",				// ("Wiped out enemy ambush")
	L"Aanval niet gelukt",					//ENTERING_ENEMY_SECTOR_CODE ("Unsuccessful attack")
	L"Aanval gelukt!",					// ("Successful attack!")
	//71-75
	L"Beesten vielen aan",					//CREATURE_ATTACK_CODE ("Creatures attacked")
	L"Gedood door bloodcats",				//BLOODCAT_AMBUSH_CODE ("Killed by bloodcats")
	L"Afgeslacht door bloodcats",				// ("Slaughtered bloodcats")
	L"%ls was gedood",						// ("%ls was killed")
	L"Carmen kop v.e. terrorist gegeven",		// ("Gave Carmen a terrorist's head")
	L"Slay vertrok",						// ("Slay left")
	L"%ls vermoord",						// ("Killed %ls")
};

static const wchar_t s_dut_pHistoryLocations[] = L"Nvt"; // N/A is an acronym for Not Applicable

// icon text strings that appear on the laptop

static const wchar_t *s_dut_pLaptopIcons[pLaptopIcons_SIZE] =
{
	L"E-mail",
	L"Web",
	L"Financieel",
	L"Dossiers",
	L"Historie",
	L"Bestanden",
	L"Afsluiten",
	L"sir-FER 4.0",			// our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added

static const wchar_t *s_dut_pBookMarkStrings[pBookMarkStrings_SIZE] =
{
	L"A.I.M.",
	L"Bobby Ray's",
	L"I.M.P",
	L"M.E.R.C.",
	L"Mortuarium",
	L"Bloemist",
	L"Verzekering",
	L"Stop",
};

// When loading or download a web page

static const wchar_t *s_dut_pDownloadString[pDownloadString_SIZE] =
{
	L"Laden",
	L"Herladen",
};

//This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

static const wchar_t *s_dut_gsAtmStartButtonText[gsAtmStartButtonText_SIZE] =
{
	L"Info", 			// view stats of the merc
	L"Inventaris", 			// view the inventory of the merc
	L"Werk",
};

// Web error messages. Please use foreign language equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator

static const wchar_t s_dut_pErrorStrings[] = L"Periodieke verbinding met host. Houdt rekening met lange wachttijden.";


static const wchar_t s_dut_pPersonnelString[] = L"Huurlingen:"; // mercs we have


static const wchar_t s_dut_pWebTitle[] = L"sir-FER 4.0"; // our name for the version of the browser, play on company name


// The titles for the web program title bar, for each page loaded

static const wchar_t *s_dut_pWebPagesTitles[pWebPagesTitles_SIZE] =
{
	L"A.I.M.",
	L"A.I.M. Leden",
	L"A.I.M. Portretten",		// a mug shot is another name for a portrait
	L"A.I.M. Sorteer",
	L"A.I.M.",
	L"A.I.M. Veteranen",
	L"A.I.M. Regelement",
	L"A.I.M. Geschiedenis",
	L"A.I.M. Links",
	L"M.E.R.C.",
	L"M.E.R.C. Rekeningen",
	L"M.E.R.C. Registratie",
	L"M.E.R.C. Index",
	L"Bobby Ray's",
	L"Bobby Ray's - Wapens",
	L"Bobby Ray's - Munitie",
	L"Bobby Ray's - Pantsering",
	L"Bobby Ray's - Diversen",					//misc is an abbreviation for miscellaneous
	L"Bobby Ray's - Gebruikt",
	L"Bobby Ray's - Mail Order",
	L"I.M.P.",
	L"I.M.P.",
	L"United Floral Service",
	L"United Floral Service - Etalage",
	L"United Floral Service - Bestelformulier",
	L"United Floral Service - Kaart Etalage",
	L"Malleus, Incus & Stapes Verzekeringen",
	L"Informatie",
	L"Contract",
	L"Opmerkingen",
	L"McGillicutty's Mortuarium",
	L"URL niet gevonden.",
	L"Bobby Ray's - Recentelijke Zendingen",
	L"",
	L"",
};

static const wchar_t *s_dut_pShowBookmarkString[pShowBookmarkString_SIZE] =
{
	L"Sir-Help",
	L"Klik opnieuw voor Bookmarks.",
};

static const wchar_t *s_dut_pLaptopTitles[pLaptopTitles_SIZE] =
{
	L"E-Mail",
	L"Bestanden bekijken",
	L"Persoonlijk",
	L"Boekhouder Plus",
	L"Geschiedenis",
};

static const wchar_t *s_dut_pPersonnelDepartedStateStrings[pPersonnelDepartedStateStrings_SIZE] =
{
	//reasons why a merc has left.
	L"Omgekomen tijdens gevechten",
	L"Weggestuurd",
	L"Getrouwd",
	L"Contract Afgelopen",
	L"Gestopt",
};
// personnel strings appearing in the Personnel Manager on the laptop

static const wchar_t *s_dut_pPersonelTeamStrings[pPersonelTeamStrings_SIZE] =
{
	L"Huidig Team",
	L"Vertrekken",
	L"Dag. Kosten:",
	L"Hoogste Kosten:",
	L"Laagste Kosten:",
	L"Omgekomen tijdens gevechten:",
	L"Weggestuurd:",
	L"Anders:",
};


static const wchar_t *s_dut_pPersonnelCurrentTeamStatsStrings[pPersonnelCurrentTeamStatsStrings_SIZE] =
{
	L"Laagste",
	L"Gemiddeld",
	L"Hoogste",
};


static const wchar_t *s_dut_pPersonnelTeamStatsStrings[pPersonnelTeamStatsStrings_SIZE] =
{
	L"GZND",
	L"BEW",
	L"HAN",
	L"KRA",
	L"LDR",
	L"WIJ",
	L"NIV",
	L"TREF",
	L"MECH",
	L"EXPL",
	L"MED",
};


// horizontal and vertical indices on the map screen

static const wchar_t *s_dut_pMapVertIndex[pMapVertIndex_SIZE] =
{
	L"X",
	L"A",
	L"B",
	L"C",
	L"D",
	L"E",
	L"F",
	L"G",
	L"H",
	L"I",
	L"J",
	L"K",
	L"L",
	L"M",
	L"N",
	L"O",
	L"P",
};

static const wchar_t *s_dut_pMapHortIndex[pMapHortIndex_SIZE] =
{
	L"X",
	L"1",
	L"2",
	L"3",
	L"4",
	L"5",
	L"6",
	L"7",
	L"8",
	L"9",
	L"10",
	L"11",
	L"12",
	L"13",
	L"14",
	L"15",
	L"16",
};

static const wchar_t *s_dut_pMapDepthIndex[pMapDepthIndex_SIZE] =
{
	L"",
	L"-1",
	L"-2",
	L"-3",
};

// text that appears on the contract button

static const wchar_t s_dut_pContractButtonString[] = L"Contract";

// text that appears on the update panel buttons

static const wchar_t *s_dut_pUpdatePanelButtons[pUpdatePanelButtons_SIZE] =
{
	L"Doorgaan",
	L"Stop",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle

static StrPointer s_dut_LargeTacticalStr[LargeTacticalStr_SIZE] =
{
	L"Je bent verslagen in deze sector!",
	L"De vijand, geen genade kennende, slacht ieder teamlid af!",
	L"Je bewusteloze teamleden zijn gevangen genomen!",
	L"Je teamleden zijn gevangen genomen door de vijand.",
};


//Insurance Contract.c
//The text on the buttons at the bottom of the screen.

static const wchar_t *s_dut_InsContractText[InsContractText_SIZE] =
{
	L"Vorige",
	L"Volgende",
	L"OK",
	L"Leeg",
};



//Insurance Info
// Text on the buttons on the bottom of the screen

static const wchar_t *s_dut_InsInfoText[InsInfoText_SIZE] =
{
	L"Vorige",
	L"Volgende",
};



//For use at the M.E.R.C. web site. Text relating to the player's account with MERC

static const wchar_t *s_dut_MercAccountText[MercAccountText_SIZE] =
{
	// Text on the buttons on the bottom of the screen
	L"Autoriseer",
	L"Thuis",
	L"Rekening#:",
	L"Huurl.",
	L"Dagen",
	L"Tarief",	//5
	L"Prijs",
	L"Totaal:",
	L"Weet je zeker de betaling van %ls te autoriseren?",		//the %ls is a string that contains the dollar amount ( ex. "$150" )
};



//For use at the M.E.R.C. web site. Text relating a MERC mercenary


static const wchar_t *s_dut_MercInfo[MercInfo_SIZE] =
{
	L"Vorige",
	L"Huur",
	L"Volgende",
	L"Extra Info",
	L"Thuis",
	L"Ingehuurd",
	L"Salaris:",
	L"Per Dag",
	L"Overleden",

	L"Lijkt erop dat je teveel huurlingen wil recruteren. Limiet is 18.",
	L"Niet beschikbaar",
};



// For use at the M.E.R.C. web site. Text relating to opening an account with MERC

static const wchar_t *s_dut_MercNoAccountText[MercNoAccountText_SIZE] =
{
	//Text on the buttons at the bottom of the screen
	L"Open Rekening",
	L"Afbreken",
	L"Je hebt geen rekening. Wil je er één openen?",
};



// For use at the M.E.R.C. web site. MERC Homepage

static const wchar_t *s_dut_MercHomePageText[MercHomePageText_SIZE] =
{
	//Description of various parts on the MERC page
	L"Speck T. Kline, oprichter en bezitter",
	L"Om een rekening te open, klik hier",
	L"Klik hier om rekening te bekijken",
	L"Klik hier om bestanden in te zien",
	// The version number on the video conferencing system that pops up when Speck is talking
	L"Speck Com v3.2",
};

// For use at MiGillicutty's Web Page.

static const wchar_t *s_dut_sFuneralString[sFuneralString_SIZE] =
{
	L"McGillicutty's Mortuarium: Helpt families rouwen sinds 1983.",
	L"Begrafenisondernemer en voormalig A.I.M. huurling Murray \"Pops\" McGillicutty is een kundig en ervaren begrafenisondernemer.",
	L"Pops weet hoe moeilijk de dood kan zijn, in heel zijn leven heeft hij te maken gehad met de dood en sterfgevallen.",
	L"McGillicutty's Mortuarium biedt een breed scala aan stervensbegeleiding, van een schouder om uit te huilen tot recontructie van misvormde overblijfselen.",
	L"Laat McGillicutty's Mortuarium u helpen en laat uw dierbaren zacht rusten.",

	// Text for the various links available at the bottom of the page
	L"STUUR BLOEMEN",
	L"DOODSKIST & URN COLLECTIE",
	L"CREMATIE SERVICE",
	L"SERVICES",
	L"ETIQUETTE",

	// The text that comes up when you click on any of the links ( except for send flowers ).
	L"Helaas is deze pagina nog niet voltooid door een sterfgeval in de familie. Afhankelijk van de laatste wil en uitbetaling van de beschikbare activa wordt de pagina zo snel mogelijk voltooid.",
	L"Ons medeleven gaat uit naar jou, tijdens deze probeerperiode. Kom nog eens langs.",
};

// Text for the florist Home page

static const wchar_t *s_dut_sFloristText[sFloristText_SIZE] =
{
	//Text on the button on the bottom of the page

	L"Etalage",

	//Address of United Florist

	L"\"We brengen overal langs\"",
	L"1-555-SCENT-ME",
	L"333 NoseGay Dr, Seedy City, CA USA 90210",
	L"http://www.scent-me.com",

	// detail of the florist page

	L"We zijn snel en efficiënt!",
	L"Volgende dag gebracht, wereldwijd, gegarandeerd. Enkele beperkingen zijn van toepassing.",
	L"Laagste prijs in de wereld, gegarandeerd!",
	L"Toon ons een lagere geadverteerde prijs voor een regeling en ontvang gratis een dozijn rozen.",
	L"Flora, Fauna & Bloemen sinds 1981.",
	L"Onze onderscheiden ex-bommenwerperpiloten droppen je boeket binnen een tien kilometer radius van de gevraagde locatie. Altijd!",
	L"Laat ons al je bloemenfantasieën waarmaken.",
	L"Laat Bruce, onze wereldberoemde bloemist, de verste bloemen met de hoogste kwaliteit uit onze eigen kassen uitzoeken.",
	L"En onthoudt, als we het niet hebben, kunnen we het kweken - Snel!",
};



//Florist OrderForm

static const wchar_t *s_dut_sOrderFormText[sOrderFormText_SIZE] =
{
	//Text on the buttons

	L"Terug",
	L"Verstuur",
	L"Leeg",
	L"Etalage",

	L"Naam vh Boeket:",
	L"Prijs:",			//5
	L"Ordernummer:",
	L"Bezorgingsdatum",
	L"volgende dag",
	L"komt wanneer het komt",
	L"Locatie Bezorging",			//10
	L"Extra Service",
	L"Geplet Boeket($10)",
	L"Zwarte Rozen ($20)",
	L"Verlept Boeket($10)",
	L"Fruitcake (indien beschikbaar)($10)",		//15
	L"Persoonlijk Bericht:",
	L"Wegens de grootte kaarten, mogen je berichten niet langer zijn dan 75 karakters.",
	L"...of selecteer er één van de onze",

	L"STANDAARDKAARTEN",
	L"Factuurinformatie",	//20

	//The text that goes beside the area where the user can enter their name

	L"Naam:",
};




//Florist Gallery.c

static const wchar_t *s_dut_sFloristGalleryText[sFloristGalleryText_SIZE] =
{
	//text on the buttons

	L"Back",	//abbreviation for previous
	L"Next",	//abbreviation for next

	L"Klik op de selectie die je wil bestellen.",
	L"Let op: er geldt een extra tarief van $10 voor geplette en verlepte boeketten.",

	//text on the button

	L"Home",
};

//Florist Cards

static const wchar_t *s_dut_sFloristCards[sFloristCards_SIZE] =
{
	L"Klik op je selectie",
	L"Terug",
};



// Text for Bobby Ray's Mail Order Site

static const wchar_t *s_dut_BobbyROrderFormText[BobbyROrderFormText_SIZE] =
{
	L"Bestelformulier",				//Title of the page
	L"Hvl",					// The number of items ordered
	L"Gewicht(%ls)",			// The weight of the item
	L"Itemnaam",				// The name of the item
	L"Prijs unit",				// the item's weight
	L"Totaal",				//5	// The total price of all of items of the same type
	L"Sub-Totaal",				// The sub total of all the item totals added
	L"Porto (Zie Bezorgloc.)",		// S&H is an acronym for Shipping and Handling
	L"Eindtotaal",			// The grand total of all item totals + the shipping and handling
	L"Bezorglocatie",
	L"Verzendingssnelheid",			//10	// See below
	L"Kosten (per %ls.)",			// The cost to ship the items
	L"Nacht-Express",			// Gets deliverd the next day
	L"2 Werkdagen",			// Gets delivered in 2 days
	L"Standaard Service",			// Gets delivered in 3 days
	L"Order Leegmaken",//15			// Clears the order page
	L"Accept. Order",			// Accept the order
	L"Terug",				// text on the button that returns to the previous page
	L"Home",				// Text on the button that returns to the home page
	L"* Duidt op Gebruikte Items",		// Disclaimer stating that the item is used
	L"Je kunt dit niet betalen.",		//20	// A popup message that to warn of not enough money
	L"<GEEN>",				// Gets displayed when there is no valid city selected
	L"Weet je zeker dat je de bestelling wil sturen naar %ls?",		// A popup that asks if the city selected is the correct one
	L"Gewicht Pakket**",			// Displays the weight of the package
	L"** Min. Gew.",				// Disclaimer states that there is a minimum weight for the package
	L"Zendingen",
};


// This text is used when on the various Bobby Ray Web site pages that sell items

static const wchar_t *s_dut_BobbyRText[BobbyRText_SIZE] =
{
	L"Bestelling",				// Title
	// instructions on how to order
	L"Klik op de item(s). Voor meer dan één, blijf dan klikken. Rechtsklikken voor minder. Als je alles geselecteerd hebt, dat je wil bestellen, ga dan naar het bestelformulier.",

	//Text on the buttons to go the various links

	L"Vorige Items",		//
	L"Wapens", 			//3
	L"Munitie",			//4
	L"Pantser",			//5
	L"Diversen",			//6	//misc is an abbreviation for miscellaneous
	L"Gebruikt",			//7
	L"Meer Items",
	L"BESTELFORMULIER",
	L"Home",			//10

	//The following lines provide information on the items

	L"Gewicht:",		// Weight of all the items of the same type
	L"Kal:",			// the caliber of the gun
	L"Mag:",			// number of rounds of ammo the Magazine can hold
	L"Afs:",			// The range of the gun
	L"Sch:",			// Damage of the weapon
	L"ROF:",			// Weapon's Rate Of Fire, acronym ROF
	L"Kost:",			// Cost of the item
	L"Aanwezig:",			// The number of items still in the store's inventory
	L"# Besteld:",		// The number of items on order
	L"Beschadigd",			// If the item is damaged
	L"SubTotaal:",			// The total cost of all items on order
	L"* % Functioneel",		// if the item is damaged, displays the percent function of the item

	//Popup that tells the player that they can only order 10 items at a time

	L"Verdraaid! Dit on-line bestelformulier accepteert maar 10 items per keer. Als je meer wil bestellen (en dat hopen we), plaats dan afzonderlijke orders en accepteer onze excuses.",

	// A popup that tells the user that they are trying to order more items then the store has in stock

	L"Sorry. We hebben niet meer van die zaken in het magazijn. Probeer het later nog eens.",

	//A popup that tells the user that the store is temporarily sold out

	L"Sorry, alle items van dat type zijn nu uitverkocht.",
};


/* The following line is used on the Ammunition page.  It is used for help text
 * to display how many items the player's merc has that can use this type of
 * ammo. */
static const wchar_t s_dut_str_bobbyr_guns_num_guns_that_use_ammo[] = L"Je team heeft %d wapen(s) gebruik makende van deze munitie";


// Text for Bobby Ray's Home Page

static const wchar_t *s_dut_BobbyRaysFrontText[BobbyRaysFrontText_SIZE] =
{
	//Details on the web site

	L"Hier moet je zijn voor de nieuwste en beste wapens en militaire goederen",
	L"We kunnen de perfecte oplossing vinden voor elke explosiebehoefte",
	L"Gebruikte en opgeknapte items",

	//Text for the various links to the sub pages

	L"Diversen",
	L"WAPENS",
	L"MUNITIE",		//5
	L"PANTSER",

	//Details on the web site

	L"Als wij het niet verkopen, dan kun je het nergens krijgen!",
	L"Under construction",
};



// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug shot page

static const wchar_t *s_dut_AimSortText[AimSortText_SIZE] =
{
	L"A.I.M. Leden",				// Title
	// Title for the way to sort
	L"Sort. op:",

	//Text of the links to other AIM pages

	L"Bekijk portretfotoindex van huurlingen",
	L"Bekijk het huurlingendossier",
	L"Bekijk de A.I.M. Veteranen"
};


// text to display how the entries will be sorted
static const wchar_t s_dut_str_aim_sort_price[]        = L"Prijs";
static const wchar_t s_dut_str_aim_sort_experience[]   = L"Ervaring";
static const wchar_t s_dut_str_aim_sort_marksmanship[] = L"Trefzekerheid";
static const wchar_t s_dut_str_aim_sort_medical[]      = L"Medisch";
static const wchar_t s_dut_str_aim_sort_explosives[]   = L"Explosieven";
static const wchar_t s_dut_str_aim_sort_mechanical[]   = L"Technisch";
static const wchar_t s_dut_str_aim_sort_ascending[]    = L"Oplopend";
static const wchar_t s_dut_str_aim_sort_descending[]   = L"Aflopend";


//Aim Policies.c
//The page in which the AIM policies and regulations are displayed

static const wchar_t *s_dut_AimPolicyText[AimPolicyText_SIZE] =
{
	// The text on the buttons at the bottom of the page

	L"Previous",
	L"AIM HomePage",
	L"Index Regels",
	L"Next",
	L"Oneens",
	L"Mee eens",
};



//Aim Member.c
//The page in which the players hires AIM mercenaries

// Instructions to the user to either start video conferencing with the merc, or to go the mug shot index

static const wchar_t *s_dut_AimMemberText[AimMemberText_SIZE] =
{
	L"Klik Links",
	L"voor Verbinding met Huurl.",
	L"Klik Rechts",
	L"voor Portretfotoindex.",
};

//Aim Member.c
//The page in which the players hires AIM mercenaries

static const wchar_t *s_dut_CharacterInfo[CharacterInfo_SIZE] =
{
	// the contract expenses' area

	L"Tarief",
	L"Contract",
	L"een dag",
	L"een week",
	L"twee weken",

	// text for the buttons that either go to the previous merc,
	// start talking to the merc, or go to the next merc

	L"Previous",
	L"Contact",
	L"Next",

	L"Extra Info",				// Title for the additional info for the merc's bio
	L"Actieve Leden", // Title of the page
	L"Aanv. Uitrusting:",				// Displays the optional gear cost
	L"MEDISCHE aanbetaling nodig",			// If the merc required a medical deposit, this is displayed
};


//Aim Member.c
//The page in which the player's hires AIM mercenaries

//The following text is used with the video conference popup

static const wchar_t *s_dut_VideoConfercingText[VideoConfercingText_SIZE] =
{
	L"Contractkosten:",				//Title beside the cost of hiring the merc

	//Text on the buttons to select the length of time the merc can be hired

	L"Een Dag",
	L"Een Week",
	L"Twee Weken",

	//Text on the buttons to determine if you want the merc to come with the equipment

	L"Geen Uitrusting",
	L"Koop Uitrusting",

	// Text on the Buttons

	L"HUUR IN",			// to actually hire the merc
	L"STOP",				// go back to the previous menu
	L"VOORWAARDEN",				// go to menu in which you can hire the merc
	L"OPHANGEN",			// stops talking with the merc
	L"OK",
	L"STUUR BERICHT",			// if the merc is not there, you can leave a message

	//Text on the top of the video conference popup

	L"Video Conference met",
	L"Verbinding maken. . .",

	L"+ med. depo",			// Displays if you are hiring the merc with the medical deposit
};



//Aim Member.c
//The page in which the player hires AIM mercenaries

// The text that pops up when you select the TRANSFER FUNDS button

static const wchar_t *s_dut_AimPopUpText[AimPopUpText_SIZE] =
{
	L"BEDRAG OVERGEBOEKT",	// You hired the merc
	L"OVERMAKEN NIET MOGELIJK",			// Player doesn't have enough money, message 1
	L"ONVOLDOENDE GELD",				// Player doesn't have enough money, message 2

	// if the merc is not available, one of the following is displayed over the merc's face

	L"Op missie",
	L"Laat a.u.b. bericht achter",
	L"Overleden",

	//If you try to hire more mercs than game can support

	L"Je team bestaat al uit 18 huurlingen.",

	L"Opgenomen bericht",
	L"Bericht opgenomen",
};


//AIM Link.c

static const wchar_t s_dut_AimLinkText[] = L"A.I.M. Links"; // The title of the AIM links page



//Aim History

// This page displays the history of AIM

static const wchar_t *s_dut_AimHistoryText[AimHistoryText_SIZE] =
{
	L"A.I.M. Geschiedenis",					//Title

	// Text on the buttons at the bottom of the page

	L"Previous",
	L"Home",
	L"A.I.M. Veteranen",
	L"Next",
};


//Aim Mug Shot Index

//The page in which all the AIM members' portraits are displayed in the order selected by the AIM sort page.

static const wchar_t *s_dut_AimFiText[AimFiText_SIZE] =
{
	// displays the way in which the mercs were sorted

	L"Prijs",
	L"Ervaring",
	L"Trefzekerheid",
	L"Medisch",
	L"Explosieven",
	L"Technisch",

	// The title of the page, the above text gets added at the end of this text

	L"A.I.M. Leden Oplopend Gesorteerd op %ls",
	L"A.I.M. Leden Aflopend Gesorteerd op %ls",

	// Instructions to the players on what to do

	L"Klik Links",
	L"om Huurling te Selecteren",			//10
	L"Klik Rechts",
	L"voor Sorteeropties",

	// Gets displayed on top of the merc's portrait if they are...

	L"Overleden",						//14
	L"Op missie",
};



//AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer with AIM

static const wchar_t *s_dut_AimAlumniText[AimAlumniText_SIZE] =
{
	// Text of the buttons

	L"PAG. 1",
	L"PAG. 2",
	L"PAG. 3",

	L"A.I.M. Veteranen",	// Title of the page

	L"OK",			// Stops displaying information on selected merc
};






//AIM Home Page

static const wchar_t *s_dut_AimScreenText[AimScreenText_SIZE] =
{
	// AIM disclaimers

	L"A.I.M. en A.I.M.-logo zijn geregistreerde handelsmerken in de meeste landen.",
	L"Dus denk er niet aan om ons te kopiëren.",
	L"Copyright 1998-1999 A.I.M., Ltd.  All rights reserved.",

	//Text for an advertisement that gets displayed on the AIM page

	L"United Floral Service",
	L"\"We droppen overal\"",				//10
	L"Doe het goed",
	L"... de eerste keer",
	L"Wapens en zo, als we het niet hebben, dan heb je het ook niet nodig.",
};


//Aim Home Page

static const wchar_t *s_dut_AimBottomMenuText[AimBottomMenuText_SIZE] =
{
	//Text for the links at the bottom of all AIM pages
	L"Home",
	L"Leden",
	L"Veteranen",
	L"Regels",
	L"Geschiedenis",
	L"Links",
};



//ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.

static const wchar_t *s_dut_SKI_Text[SKI_SIZE ] =
{
	L"HANDELSWAAR OP VOORRAAD",		//Header for the merchandise available
	L"PAG.",				//The current store inventory page being displayed
	L"TOTALE KOSTEN",				//The total cost of the the items in the Dealer inventory area
	L"TOTALE WAARDE",			//The total value of items player wishes to sell
	L"EVALUEER",				//Button text for dealer to evaluate items the player wants to sell
	L"TRANSACTIE",			//Button text which completes the deal. Makes the transaction.
	L"OK",				//Text for the button which will leave the shopkeeper interface.
	L"REP. KOSTEN",			//The amount the dealer will charge to repair the merc's goods
	L"1 UUR",			// SINGULAR! The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"%d UREN",		// PLURAL!   The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"GEREPAREERD",		// Text appearing over an item that has just been repaired by a NPC repairman dealer
	L"Er is geen ruimte meer.",	//Message box that tells the user there is no more room to put there stuff
	L"%d MINUTEN",		// The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"Drop Item op Grond.",
};


static const wchar_t *s_dut_SkiMessageBoxText[SkiMessageBoxText_SIZE] =
{
	L"Wil je %ls aftrekken van je hoofdrekening om het verschil op te vangen?",
	L"Niet genoeg geld. Je komt %ls tekort",
	L"Wil je %ls aftrekken van je hoofdrekening om de kosten te dekken?",
	L"Vraag de dealer om de transactie te starten",
	L"Vraag de dealer om de gesel. items te repareren",
	L"Einde conversatie",
	L"Huidige Saldo",
};


//OptionScreen.c

static const wchar_t *s_dut_zOptionsText[zOptionsText_SIZE] =
{
	//button Text
	L"Spel Bewaren",
	L"Spel Laden",
	L"Stop",
	L"OK",

	//Text above the slider bars
	L"Effecten",
	L"Spraak",
	L"Muziek",

	//Confirmation pop when the user selects..
	L"Spel verlaten en terugkeren naar hoofdmenu?",

	L"Je hebt of de Spraakoptie nodig of de ondertiteling.",
};


//SaveLoadScreen
static const wchar_t *s_dut_zSaveLoadText[zSaveLoadText_SIZE] =
{
	L"Spel Bewaren",
	L"Spel Laden",
	L"Stop",
	L"Bewaren Gesel.",
	L"Laden Gesel.",

	L"Spel Bewaren voltooid",
	L"FOUT bij bewaren spel!",
	L"Spel laden succesvol",
	L"FOUT bij laden spel: \"%hs\"",

	L"De spelversie van het bewaarde spel verschilt van de huidige versie. Waarschijnlijk is het veilig om door te gaan. Doorgaan?",
	L"De bewaarde spelen zijn waarschijnlijk ongeldig. Deze verwijderen?",

	//Translators, the next two strings are for the same thing.  The first one is for beta version releases and the second one
	//is used for the final version.  Please don't modify the "#ifdef JA2BETAVERSION" or the "#else" or the "#endif" as they are
	//used by the compiler and will cause program errors if modified/removed.  It's okay to translate the strings though.
#ifdef JA2BETAVERSION
	L"Save version has changed.  Please report if there any problems.  Continue?",
#else
	L"Attempting to load an older version save.  Automatically update and load the save?",
#endif

	//Translators, the next two strings are for the same thing.  The first one is for beta version releases and the second one
	//is used for the final version.  Please don't modify the "#ifdef JA2BETAVERSION" or the "#else" or the "#endif" as they are
	//used by the compiler and will cause program errors if modified/removed.  It's okay to translate the strings though.
#ifdef JA2BETAVERSION
	L"Save version and game version have changed.  Please report if there are any problems.  Continue?",
#else
	L"Attempting to load an older version save.  Automatically update and load the save?",
#endif

	L"Weet je zeker dat je het spel in slot #%d wil overschrijven?",


	//The first %d is a number that contains the amount of free space on the users hard drive,
	//the second is the recommended amount of free space.
	L"Er is te weinig ruimte op de harde schijf. Er is maar %d MB vrij en Jagged heeft tenminste %d MB nodig.",

	L"Bewaren...",			//When saving a game, a message box with this string appears on the screen

	L"Normale Wapens",
	L"Stapels Wapens",
	L"Realistische stijl",
	L"SF stijl",

	L"Moeilijkheid",
};



//MapScreen
static const wchar_t *s_dut_zMarksMapScreenText[zMarksMapScreenText_SIZE] =
{
	L"Kaartniveau",
	L"Je hebt geen militie. Je moet stadsburgers trainen om een stadsmilitie te krijgen.",
	L"Dagelijks Inkomen",
	L"Huurling heeft levensverzekering",
	L"%ls is niet moe.",
	L"%ls is bezig en kan niet slapen",
	L"%ls is te moe, probeer het later nog eens.",
	L"%ls is aan het rijden.",
	L"Team kan niet reizen met een slapende huurling.",

	// stuff for contracts
	L"Je kunt wel het contract betalen, maar je hebt geen geld meer om de levensverzekering van de huurling te betalen.",
	L"%ls verzekeringspremie kost %ls voor %d extra dag(en). Wil je betalen?",
	L"Inventaris Sector",
	L"Huurling heeft medische kosten.",

	// other items
	L"Medici",		// people acting a field medics and bandaging wounded mercs
	L"Patiënten", // people who are being bandaged by a medic
	L"OK", // Continue on with the game after autobandage is complete
	L"Stop", // Stop autobandaging of patients by medics now
	L"%ls heeft geen reparatie-kit.",
	L"%ls heeft geen medische kit.",
	L"Er zijn nu niet genoeg mensen die getraind willen worden.",
	L"%ls is vol met milities.",
	L"Huurling heeft eindig contract.",
	L"Contract Huurling is niet verzekerd",
};


static const wchar_t s_dut_pLandMarkInSectorString[] = L"Team %d is heeft iemand ontdekt in sector %ls";

// confirm the player wants to pay X dollars to build a militia force in town
static const wchar_t *s_dut_pMilitiaConfirmStrings[pMilitiaConfirmStrings_SIZE] =
{
	L"Een stadsmilitie trainen kost $",		// telling player how much it will cost
	L"Uitgave goedkeuren?", // asking player if they wish to pay the amount requested
	L"je kunt dit niet betalen.", // telling the player they can't afford to train this town
	L"Doorgaan met militie trainen %ls (%ls %d)?", // continue training this town?
	L"Kosten $", // the cost in dollars to train militia
	L"( J/N )",   // abbreviated yes/no
	L"Stadsmilities trainen in %d sectors kost $ %d. %ls", // cost to train sveral sectors at once
	L"Je kunt de $%d niet betalen om de stadsmilitie hier te trainen.",
	L"%ls heeft een loyaliteit nodig van %d procent om door te gaan met milities trainen.",
	L"Je kunt de militie in %ls niet meer trainen.",
};


//Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom of the single merc panel
static const wchar_t *s_dut_gzMoneyWithdrawMessageText[gzMoneyWithdrawMessageText_SIZE] =
{
	L"Je kunt maximaal $20.000 in één keer opnemen.",
	L"Weet je zeker dat je %ls wil storten op je rekening?",
};

static const wchar_t s_dut_gzCopyrightText[] = L"Copyright (C) 1999 Sir-tech Canada Ltd.  All rights reserved.";

//option Text
static const wchar_t *s_dut_zOptionsToggleText[zOptionsToggleText_SIZE] =
{
	L"Spraak",
	L"Bevestigingen uit",
	L"Ondertitels",
	L"Wacht bij tekst-dialogen",
	L"Rook Animeren",
	L"Bloedsporen Tonen",
	L"Cursor Niet Bewegen",
	L"Oude Selectiemethode",
	L"Toon reisroute",
	L"Toon Missers",
	L"Bevestiging Real-Time",
	L"Slaap/wakker-berichten",
	L"Metrieke Stelsel",
	L"Huurling Oplichten",
	L"Auto-Cursor naar Huurling",
	L"Auto-Cursor naar Deuren",
	L"Items Oplichten",
	L"Toon Boomtoppen",
	L"Toon Draadmodellen",
	L"Toon 3D Cursor",
};

//This is the help text associated with the above toggles.
static const wchar_t *s_dut_zOptionsScreenHelpText[zOptionsToggleText_SIZE] =
{
	//speech
	L"Schakel deze optie IN als je de karakter-dialogen wil horen.",

	//Mute Confirmation
	L"Schakelt verbale bevestigingen v.d. karakters in of uit.",

	//Subtitles
	L"Stelt in of dialoogteksten op het scherm worden getoond.",

	//Key to advance speech
	L"Als ondertitels AANstaan, schakel dit ook in om tijd te hebben de NPC-dialogen te lezen.",

	//Toggle smoke animation
	L"Schakel deze optie uit als rookanimaties het spel vertragen.",

	//Blood n Gore
	L"Schakel deze optie UIT als je bloed aanstootgevend vindt.",

	//Never move my mouse
	L"Schakel deze optie UIT als je wil dat de muis automatisch gepositioneerd wordt bij bevestigingsdialogen.",

	//Old selection method
	L"Schakel deze optie IN als je karakters wil selecteren zoals in de vorige JAGGED ALLIANCE (methode is tegengesteld dus).",

	//Show movement path
	L"Schakel deze optie IN om bewegingspaden te tonen in real-time (schakel het uit en gebruik dan de SHIFT-toets om paden te tonen).",

	//show misses
	L"Schakel IN om het spel de plaats van inslag van je kogels te tonen wanneer je \"mist\".",

	//Real Time Confirmation
	L"Als INGESCHAKELD, een extra \"veiligheids\"-klik is nodig om in real-time te bewegen.",

	//Sleep/Wake notification
	L"INGESCHAKELD zorgt voor berichten of huurlingen op een \"missie\" slapen of werken.",

	//Use the metric system
	L"Wanneer INGESCHAKELD wordt het metrieke stelsel gebruikt, anders het Imperiale stelsel.",

	//Merc Lighted movement
	L"Wanneer INGESCHAKELD, de huurling verlicht de grond tijdens het lopen. Schakel UIT voor sneller spelen.",

	//Smart cursor
	L"Wanneer INGESCHAKELD zullen huurlingen dichtbij de cursor automatisch oplichten.",

	//snap cursor to the door
	L"Wanneer INGESCHAKELD zal de cursor dichtbij een deur automatisch boven de deur gepositioneerd worden.",

	//glow items
	L"Wanneer INGESCHAKELD lichten |Items altijd op",

	//toggle tree tops
	L"Wanneer INGESCHAKELD worden Boom|toppen getoond.",

	//toggle wireframe
	L"Wanneer INGESCHAKELD worden Draadmodellen (|W) van niet-zichtbare muren getoond.",

	L"Wanneer INGESCHAKELD wordt de cursor in 3D getoond. (|H|o|m|e)",

};


static const wchar_t *s_dut_gzGIOScreenText[gzGIOScreenText_SIZE] =
{
	L"SPEL-INSTELLINGEN",
	L"Speelstijl",
	L"Realistisch",
	L"SF",
	L"Wapenopties",
	L"Extra wapens",
	L"Normaal",
	L"Moeilijksheidsgraad",
	L"Beginneling",
	L"Ervaren",
	L"Expert",
	L"Ok",
	L"Stop",
	L"Extra Moeilijk",
	L"Ongelimiteerde Tijd",
	L"Getimede Beurten",
	L"Dead is Dead"
};

static const wchar_t *s_dut_pDeliveryLocationStrings[pDeliveryLocationStrings_SIZE] =
{
	L"Austin",			//Austin, Texas, USA
	L"Baghdad",			//Baghdad, Iraq (Suddam Hussein's home)
	L"Drassen",			//The main place in JA2 that you can receive items.  The other towns are dummy names...
	L"Hong Kong",		//Hong Kong, Hong Kong
	L"Beirut",			//Beirut, Lebanon	(Middle East)
	L"London",			//London, England
	L"Los Angeles",	//Los Angeles, California, USA (SW corner of USA)
	L"Meduna",			//Meduna -- the other airport in JA2 that you can receive items.
	L"Metavira",		//The island of Metavira was the fictional location used by JA1
	L"Miami",				//Miami, Florida, USA (SE corner of USA)
	L"Moscow",			//Moscow, USSR
	L"New York",		//New York, New York, USA
	L"Ottawa",			//Ottawa, Ontario, Canada -- where JA2 was made!
	L"Paris",				//Paris, France
	L"Tripoli",			//Tripoli, Libya (eastern Mediterranean)
	L"Tokyo",				//Tokyo, Japan
	L"Vancouver",		//Vancouver, British Columbia, Canada (west coast near US border)
};

/* This string is used in the IMP character generation.  It is possible to
 * select 0 ability in a skill meaning you can't use it.  This text is
 * confirmation to the player. */
static const wchar_t s_dut_pSkillAtZeroWarning[]    = L"Are you sure? A value of zero means NO ability in this skill.";
static const wchar_t s_dut_pIMPBeginScreenStrings[] = L"( 8 Karakters Max )";
static const wchar_t s_dut_pIMPFinishButtonText[]   = L"Analiseren";
static const wchar_t s_dut_pIMPFinishStrings[]      = L"Bedankt, %ls"; //%ls is the name of the merc
static const wchar_t s_dut_pIMPVoicesStrings[]      = L"Stem"; // the strings for imp voices screen

// title for program
static const wchar_t s_dut_pPersTitleText[] = L"Personeelsmanager";

// paused game strings
static const wchar_t *s_dut_pPausedGameText[pPausedGameText_SIZE] =
{
	L"Spel Gepauzeerd",
	L"Doorgaan (|P|a|u|s|e)",
	L"Pauze Spel (|P|a|u|s|e)",
};


static const wchar_t *s_dut_pMessageStrings[pMessageStrings_SIZE] =
{
	L"Spel verlaten?",
	L"OK",
	L"JA",
	L"NEE",
	L"STOPPEN",
	L"WEER AANNEMEN",
	L"LEUGEN",
	L"Geen beschrijving", //Save slots that don't have a description.
	L"Spel opgeslagen.",
	L"Dag",
	L"Huurl",
	L"Leeg Slot",		//An empty save game slot
	L"rpm",				//Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
	L"min",				//Abbreviation for minute.
	L"m",					//One character abbreviation for meter (metric distance measurement unit).
	L"rnds",				//Abbreviation for rounds (# of bullets)
	L"kg",					//Abbreviation for kilogram (metric weight measurement unit)
	L"lb",					//Abbreviation for pounds (Imperial weight measurement unit)
	L"Home",				//Home as in homepage on the internet.
	L"USD",					//Abbreviation to US dollars
	L"nvt",					//Lowercase acronym for not applicable.
	L"Intussen",		//Meanwhile
	L"%ls is gearriveerd in sector %ls%ls", //Name/Squad has arrived in sector A9.  Order must not change without notifying
																		//SirTech
	L"Versie",
	L"Leeg SnelBewaarSlot",
	L"Dit slot is gereserveerd voor SnelBewaren tijdens tactische en kaartoverzichten m.b.v. ALT+S.",
	L"Geopend",
	L"Gesloten",
	L"Schijfruimte raakt op. Er is slects %ls MB vrij en Jagged Alliance 2 heeft %ls MB nodig.",
	L"%ls heeft %ls gevangen.",		//'Merc name' has caught 'item' -- let SirTech know if name comes after item.
	L"%ls heeft de drug genomen.",		//'Merc name' has taken the drug
	L"%ls heeft geen medische kennis",	//'Merc name' has no medical skill.

	//CDRom errors (such as ejecting CD while attempting to read the CD)
	L"De integriteit van het spel is aangetast.",
	L"FOUT: CD-ROM geopend",

	//When firing heavier weapons in close quarters, you may not have enough room to do so.
	L"Er is geen plaats om vanaf hier te schieten.",

	//Can't change stance due to objects in the way...
	L"Kan op dit moment geen standpunt wisselen.",

	//Simple text indications that appear in the game, when the merc can do one of these things.
	L"Drop",
	L"Gooi",
	L"Geef",

	L"%ls gegeven aan %ls.",	//"Item" passed to "merc".  Please try to keep the item %ls before the merc %ls, otherwise,
											 //must notify SirTech.
	L"Geen plaats om %ls aan %ls te geven.",	//pass "item" to "merc".  Same instructions as above.

	//A list of attachments appear after the items.  Ex:  Kevlar vest ( Ceramic Plate 'Attached )'
	L" eraan vastgemaakt)",

	//Cheat modes
	L"Vals spel niveau EEN",
	L"Vals spel niveau TWEE",

	//Toggling various stealth modes
	L"Team op sluipmodus.",
	L"Team niet op sluipmodus.",
	L"%ls op sluipmodus.",
	L"%ls niet op sluipmodus.",

	//Wireframes are shown through buildings to reveal doors and windows that can't otherwise be seen in
	//an isometric engine.  You can toggle this mode freely in the game.
	L"Extra Draadmodellen Aan",
	L"Extra Draadmodellen Uit",

	//These are used in the cheat modes for changing levels in the game.  Going from a basement level to
	//an upper level, etc.
	L"Kan niet naar boven vanaf dit niveau...",
	L"Er zijn geen lagere niveaus...",
	L"Betreden basisniveau %d...",
	L"Verlaten basisniveau...",

	L"'s",		// used in the shop keeper inteface to mark the ownership of the item eg Red's gun
	L"Volgmodus UIT.",
	L"Volgmodus AAN.",
	L"3D Cursor UIT.",
	L"3D Cursor AAN.",
	L"Team %d actief.",
	L"Je kunt %ls's dagelijkse salaris van %ls niet betalen",	//first %ls is the mercs name, the seconds is a string containing the salary
	L"Overslaan",
	L"%ls kan niet alleen weggaan.",
	L"Een spel is bewaard onder de naam SaveGame99.sav. Indien nodig, hernoem het naar SaveGame10 zodat je het kan aanroepen in het Laden-scherm.",
	L"%ls dronk wat %ls",
	L"Een pakket is in Drassen gearriveerd.",
	L"%ls zou moeten arriveren op het aangewezen punt (sector %ls) op dag %d, om ongeveer %ls.",
	L"Geschiedenisverslag bijgewerkt.",
#ifdef JA2BETAVERSION
	L"Spel succesvol bewaard in de Einde Beurt Auto Bewaar Slot.",
#endif
};


static const wchar_t *s_dut_ItemPickupHelpPopup[ItemPickupHelpPopup_SIZE] =
{
	L"OK",
	L"Scroll Omhoog",
	L"Selecteer Alles",
	L"Scroll Omlaag",
	L"Stop",
};

static const wchar_t *s_dut_pDoctorWarningString[pDoctorWarningString_SIZE] =
{
	L"%ls is niet dichtbij genoeg om te worden genezen.",
	L"Je medici waren niet in staat om iedereen te verbinden.",
};

static const wchar_t *s_dut_pMilitiaButtonsHelpText[pMilitiaButtonsHelpText_SIZE] =
{
	L"Raap op(Klik Rechts)/drop(Klik links) Groene Troepen",	// button help text informing player they can pick up or drop militia with this button
	L"Raap op(Klik Rechts)/drop(Klik links) Normale Troepen",
	L"Raap op(Klik Rechts)/drop(Klik links) Veteranentroepen",
	L"Verspreidt beschikbare milities evenredig over alle sectoren",
};

// to inform the player to hire some mercs to get things going
static const wchar_t s_dut_pMapScreenJustStartedHelpText[] = L"Ga naar AIM en huur wat huurlingen in ( *Hint* dat kan bij Laptop )";

static const wchar_t s_dut_pAntiHackerString[] = L"Fout. Bestanden missen of zijn beschadigd. Spel wordt beëindigd.";


static const wchar_t *s_dut_gzLaptopHelpText[gzLaptopHelpText_SIZE] =
{
	//Buttons:
	L"Lees E-mail",
	L"Bekijk web-pagina's",
	L"Bekijk bestanden en e-mail attachments",
	L"Lees verslag van gebeurtenissen",
	L"Bekijk team-info",
	L"Bekijk financieel overzicht",
	L"Sluit laptop",

	//Bottom task bar icons (if they exist):
	L"Je hebt nieuwe berichten",
	L"Je hebt nieuwe bestanden",

	//Bookmarks:
	L"Association of International Mercenaries",
	L"Bobby Ray's online weapon mail order",
	L"Institute of Mercenary Profiling",
	L"More Economic Recruiting Center",
	L"McGillicutty's Mortuarium",
	L"United Floral Service",
	L"Verzekeringsagenten voor A.I.M. contracten",
};


static const wchar_t s_dut_gzHelpScreenText[] = L"Verlaat help-scherm";

static const wchar_t *s_dut_gzNonPersistantPBIText[gzNonPersistantPBIText_SIZE] =
{
	L"Er is een gevecht gaande. Je kan alleen terugtrekken m.b.v. het tactische scherm.",
	L"B|etreedt sector om door te gaan met het huidige gevecht.",
	L"Los huidige gevecht |automatisch op.",
	L"Gevecht kan niet automatisch opgelost worden als je de aanvaller bent.",
	L"Gevecht kan niet automatisch opgelost worden als je in een hinderlaag ligt.",
	L"Gevecht kan niet automatisch opgelost worden als je vecht met beesten in de mijnen.",
	L"Gevecht kan niet automatisch opgelost worden als er vijandige burgers zijn.",
	L"Gevecht kan niet automatisch opgelost worden als er nog bloodcats zijn.",
	L"GEVECHT GAANDE",
	L"je kan je op dit moment niet terugtrekken.",
};

static const wchar_t *s_dut_gzMiscString[gzMiscString_SIZE] =
{
	L"Je militie vecht door zonder hulp van je huurlingen...",
	L"Het voertuig heeft geen brandstof meer nodig.",
	L"De brandstoftank is voor %d%% gevuld.",
	L"Het leger van Deidranna heeft totale controle verkregen over %ls.",
	L"Je hebt een tankplaats verloren.",
};

static const wchar_t s_dut_gzIntroScreen[] = L"Kan intro video niet vinden";

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
static const wchar_t *s_dut_pNewNoiseStr[pNewNoiseStr_SIZE] =
{
	L"%ls hoort een %ls geluid uit %ls.",
	L"%ls hoort een %ls geluid van BEWEGING uit %ls.",
	L"%ls hoort een %ls KRAKEND geluid uit %ls.",
	L"%ls hoort een %ls SPETTEREND geluid uit %ls.",
	L"%ls hoort een %ls INSLAG uit %ls.",
	L"%ls hoort een %ls EXPLOSIE naar %ls.",
	L"%ls hoort een %ls SCHREEUW naar %ls.",
	L"%ls hoort een %ls INSLAG naar %ls.",
	L"%ls hoort een %ls INSLAG naar %ls.",
	L"%ls hoort een %ls VERSPLINTEREN uit %ls.",
	L"%ls hoort een %ls KLAP uit %ls.",
};

static const wchar_t *s_dut_wMapScreenSortButtonHelpText[wMapScreenSortButtonHelpText_SIZE] =
{
	L"Sorteer op Naam (|F|1)",
	L"Sorteer op Taak (|F|2)",
	L"Sorteer op Slaapstatus (|F|3)",
	L"Sorteer op locatie (|F|4)",
	L"Sorteer op Bestemming (|F|5)",
	L"Sorteer op Vertrektijd (|F|6)",
};



static const wchar_t *s_dut_BrokenLinkText[BrokenLinkText_SIZE] =
{
	L"Fout 404",
	L"Site niet gevonden.",
};


static const wchar_t *s_dut_gzBobbyRShipmentText[gzBobbyRShipmentText_SIZE] =
{
	L"Recentelijke ladingen",
	L"Order #",
	L"Aantal Items",
	L"Besteld op",
};


static const wchar_t *s_dut_gzCreditNames[gzCreditNames_SIZE]=
{
	L"Chris Camfield",
	L"Shaun Lyng",
	L"Kris Märnes",
	L"Ian Currie",
	L"Linda Currie",
	L"Eric \"WTF\" Cheng",
	L"Lynn Holowka",
	L"Norman \"NRG\" Olsen",
	L"George Brooks",
	L"Andrew Stacey",
	L"Scot Loving",
	L"Andrew \"Big Cheese\" Emmons",
	L"Dave \"The Feral\" French",
	L"Alex Meduna",
	L"Joey \"Joeker\" Whelan",
};


static const wchar_t *s_dut_gzCreditNameTitle[gzCreditNameTitle_SIZE]=
{
	L"Spel Programmeur", 					// Chris Camfield "Game Internals Programmer"
	L"Co-ontwerper/Schrijver",				// Shaun Lyng "Co-designer/Writer"
	L"Strategische Systemen & Programmeur",		//Kris Marnes "Strategic Systems & Editor Programmer"
	L"Producer/Co-ontwerper",				// Ian Currie "Producer/Co-designer"
	L"Co-ontwerper/Kaartontwerp",				// Linda Currie "Co-designer/Map Designer"
	L"Artiest",							// Eric \"WTF\" Cheng "Artist"
	L"Beta Coördinator, Ondersteuning",			// Lynn Holowka
	L"Artiest Extraordinaire",				// Norman \"NRG\" Olsen
	L"Geluidsgoeroe",						// George Brooks
	L"Schermontwerp/Artiest",				// Andrew Stacey
	L"Hoofd-Artiest/Animator",				// Scot Loving
	L"Hoofd-Programmeur",					// Andrew \"Big Cheese Doddle\" Emmons
	L"Programmeur",							// Dave French
	L"Strategische Systemen & Spelbalans Programmeur",	// Alex Meduna
	L"Portret-Artiest",						// Joey \"Joeker\" Whelan",
};

static const wchar_t *s_dut_gzCreditNameFunny[gzCreditNameFunny_SIZE]=
{
	L"", 																			// Chris Camfield
	L"(leert nog steeds interpunctie)",			// Shaun Lyng
	L"(\"Het is klaar. Ben er mee bezig\")",		//Kris \"The Cow Rape Man\" Marnes
	L"(wordt veel te oud voor dit)",			// Ian Currie
	L"(en werkt aan Wizardry 8)",				// Linda Currie
	L"(moets onder bedreiging ook QA doen)",		// Eric \"WTF\" Cheng
	L"(Verliet ons voor CFSA - dus...)",		// Lynn Holowka
	L"",								// Norman \"NRG\" Olsen
	L"",								// George Brooks
	L"(Dead Head en jazz liefhebber)",			// Andrew Stacey
	L"(in het echt heet hij Robert)",			// Scot Loving
	L"(de enige verantwoordelijke persoon)",		// Andrew \"Big Cheese Doddle\" Emmons
	L"(kan nu weer motorcrossen)",			// Dave French
	L"(gestolen van Wizardry 8)",				// Alex Meduna
	L"(deed items en schermen-laden ook!)",		// Joey \"Joeker\" Whelan",
};

static const wchar_t *s_dut_sRepairsDoneString[sRepairsDoneString_SIZE] =
{
	L"%ls is klaar met reparatie van eigen items",
	L"%ls is klaar met reparatie van ieders wapens en bepantering",
	L"%ls is klaar met reparatie van ieders uitrusting",
	L"%ls is klaar met reparatie van ieders vervoerde items",
};

static const wchar_t *s_dut_zGioDifConfirmText[zGioDifConfirmText_SIZE]=
{
	L"Je hebt de NOVICE-modus geselecteerd. Deze instelling is geschikt voor diegenen die Jagged Alliance voor de eerste keer spelen, voor diegenen die nog niet zo bekend zijn met strategy games, of voor diegenen die kortere gevechten in de game willen hebben.", //Je keuze beïnvloedt dingen in het hele verloop van de game, dus weet wat je doet. Weet je zeker dat je in de Novice-modus wilt spelen?",
	L"Je hebt de EXPERIENCED-modus geselecteerd. Deze instelling is geschikt voor diegenen die al bekend zijn met Jagged Alliance of dergelijke games. Je keuze beïnvloedt dingen in het hele verloop van de game, dus weet wat je doet. Weet je zeker dat je in de Experienced-modus wilt spelen ?",
	L"Je hebt de EXPERT-modus geselecteerd. We hebben je gewaarschuwd. Geef ons niet de schuld als je in een kist terugkomt. Je keuze beïnvloedt dingen in het hele verloop van de game, dus weet wat je doet. Weet je zeker dat je in de Expert-modus wilt spelen?",
};

static const wchar_t *s_dut_gzLateLocalizedString[gzLateLocalizedString_SIZE] =
{
	//1-5
	L"De robot kan de sector niet verlaten als niemand de besturing gebruikt.",

	//This message comes up if you have pending bombs waiting to explode in tactical.
	L"Je kan de tijd niet versnellen, Wacht op het vuurwerk!",

	//'Name' refuses to move.
	L"%ls weigert zich te verplaatsen.",

	//%ls a merc name
	L"%ls heeft niet genoeg energie om standpunt te wisselen.",

	//A message that pops up when a vehicle runs out of gas.
	L"%ls heeft geen brandstof en is gestrand in %c%d.",

	//6-10

	// the following two strings are combined with the pNewNoise[] strings above to report noises
	// heard above or below the merc
	L"boven",
	L"onder",

	//The following strings are used in autoresolve for autobandaging related feedback.
	L"Niemand van je huurlingen heeft medische kennis.",
	L"Er zijn geen medische hulpmiddelen om mensen te verbinden.",
	L"Er waren niet genoeg medische hulpmiddelen om iedereen te verbinden.",
	L"Geen enkele huurling heeft medische hulp nodig.",
	L"Verbindt huurlingen automatisch.",
	L"Al je huurlingen zijn verbonden.",

	//14
	L"Arulco",

	L"(dak)",

	L"Gezondheid: %d/%d",

	//In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
	//"vs." is the abbreviation of versus.
	L"%d vs. %d",

	L"%ls is vol!",  //(ex "The ice cream truck is full")

	L"%ls heeft geen eerste hulp nodig, maar échte medische hulp of iets dergelijks.",

	//20
	//Happens when you get shot in the legs, and you fall down.
	L"%ls is geraakt in het been en valt om!",
	//Name can't speak right now.
	L"%ls kan nu niet praten.",

	//22-24 plural versions
	L"%d groene milities zijn gepromoveerd tot veteranenmilitie.",
	L"%d groene milities zijn gepromoveerd tot reguliere militie.",
	L"%d reguliere milities zijn gepromoveerd tot veteranenmilitie.",

	//25
	L"Schakelaar",

	//26
	//Name has gone psycho -- when the game forces the player into burstmode (certain unstable characters)
	L"%ls wordt gek!",

	//27-28
	//Messages why a player can't time compress.
	L"Het is nu onveilig om de tijd te versnellen omdat je huurlingen hebt in sector %ls.",
	L"Het is nu onveilig om de tijd te versnellen als er huurlingen zijn in de mijnen met beesten.",

	//29-31 singular versions
	L"1 groene militie is gepromoveerd tot veteranenmilitie.",
	L"1 groene militie is gepromoveerd tot reguliere militie.",
	L"1 reguliere militie is gepromoveerd tot veteranenmilitie.",

	//32-34
	L"%ls zegt helemaal niets.",
	L"Naar oppervlakte reizen?",
	L"(Team %d)",

	//35
	//Ex: "Red has repaired Scope's MP5K".  Careful to maintain the proper order (Red before Scope, Scope before MP5K)
	L"%ls heeft %ls's %ls gerepareerd",

	//36
	L"BLOODCAT",

	//37-38 "Name trips and falls"
	L"%ls ups en downs",
	L"Dit item kan vanaf hier niet opgepakt worden.",

	//39
	L"Geen enkele huurling van je is in staat om te vechten. De militie zal zelf tegen de beesten vechten.",

	//40-43
	//%ls is the name of merc.
	L"%ls heeft geen medische kits meer!",
	L"%ls heeft geen medische kennis om iemand te verzorgen!",
	L"%ls heeft geen gereedschapkits meer!",
	L"%ls heeft geen technische kennis om iets te repareren!",

	//44-45
	L"Reparatietijd",
	L"%ls kan deze persoon niet zien.",

	//46-48
	L"%ls's pistoolloopverlenger valt eraf!",
	L"Niet meer dan %d militietrainers zijn toegelaten per sector.",
	L"Zeker weten?",

	//49-50
	L"Tijdversneller",
	L"De tank van het voertuig is nu vol.",

	//51-52 Fast help text in mapscreen.
	L"Doorgaan met Tijdversnelling (|S|p|a|c|e)",
	L"Stop Tijdversnelling (|E|s|c)",

	//53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
	L"%ls heeft de %ls gedeblokkeerd",
	L"%ls heeft %ls's %ls gedeblokkeerd",

	//55
	L"Kan tijd niet versneller tijdens bekijken van sector inventaris.",

	//56
	//Displayed with the version information when cheats are enabled.
	L"Huidig/Max Voortgang: %d%%/%d%%",

	//57
	L"John en Mary escorteren?",

	L"Schakelaar geactiveerd.",
};

static const wchar_t s_dut_str_ceramic_plates_smashed[] = L"%ls's ceramic plates have been smashed!"; // TODO translate

static const wchar_t s_dut_str_arrival_rerouted[] = L"Arrival of new recruits is being rerouted to sector %ls, as scheduled drop-off point of sector %ls is enemy occupied."; // TODO translate


static const wchar_t s_dut_str_stat_health[]       = L"Gezondheid";
static const wchar_t s_dut_str_stat_agility[]      = L"Beweeglijkheid";
static const wchar_t s_dut_str_stat_dexterity[]    = L"Handigheid";
static const wchar_t s_dut_str_stat_strength[]     = L"Kracht";
static const wchar_t s_dut_str_stat_leadership[]   = L"Leiderschap";
static const wchar_t s_dut_str_stat_wisdom[]       = L"Wijsheid";
static const wchar_t s_dut_str_stat_exp_level[]    = L"Ervaringsniveau";
static const wchar_t s_dut_str_stat_marksmanship[] = L"Trefzekerheid";
static const wchar_t s_dut_str_stat_mechanical[]   = L"Technisch";
static const wchar_t s_dut_str_stat_explosive[]    = L"Explosieven";
static const wchar_t s_dut_str_stat_medical[]      = L"Medisch";

static const wchar_t *s_dut_str_stat_list[str_stat_list_SIZE] =
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

static const wchar_t *s_dut_str_aim_sort_list[str_aim_sort_list_SIZE] =
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

extern const wchar_t *g_eng_zNewTacticalMessages[];
extern const wchar_t *g_eng_str_iron_man_mode_warning;
extern const wchar_t *g_eng_str_dead_is_dead_mode_warning;
extern const wchar_t *g_eng_str_dead_is_dead_mode_enter_name;

static const wchar_t *s_dut_gs_dead_is_dead_mode_tab_name[gs_dead_is_dead_mode_tab_name_SIZE] =
{
	L"Normal", 			// Normal Tab
	L"DiD", 			// Dead is Dead Tab
};

/** Dutch language resources. */
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
  s_dut_pTownNames,
  s_dut_g_towns_locative,
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
  s_dut_pLandTypeStrings,
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
  s_dut_pDeliveryLocationStrings,
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
};

#ifdef WITH_UNITTESTS
#define ARR_SIZE(x) (sizeof(x)/sizeof(x[0]))
TEST(WideStringEncodingTest, DutchTextFile)
{
  // This test checks that the wide string literals in this file are correctly
  // interpreted by the compiler.  Visual Studio requires BOM (byte-order mark)
  // to correctly identify file encoding.  Failed test means that the compiler
  // cannot correctly interpret the string literals.
  const wchar_t str[] = L"тест";
  ASSERT_EQ(ARR_SIZE(str), 5) << "Compiler cannot correctly interpret wide string literals";
  EXPECT_EQ(str[0], 0x0442);
  EXPECT_EQ(str[1], 0x0435);
  EXPECT_EQ(str[2], 0x0441);
  EXPECT_EQ(str[3], 0x0442);
  EXPECT_EQ(str[4], 0x00);
}
#endif
