#include "Text.h"

#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"
#endif

#include <string_theory/string>


// ******************************************************************************************************
// **                                  IMPORTANT TRANSLATION NOTES                                     **
// ******************************************************************************************************
//
// GENERAL TOPWARE INSTRUCTIONS
// - Always be aware that German strings should be of equal or shorter length than the English equivalent.
//   I know that this is difficult to do on many occasions due to the nature of the German language when
//   compared to English. By doing so, this will greatly reduce the amount of work on both sides. In
//   most cases (but not all), JA2 interfaces were designed with just enough space to fit the English word.
//   The general rule is if the string is very short (less than 10 characters), then it's short because of
//   interface limitations. On the other hand, full sentences commonly have little limitations for length.
//   Strings in between are a little dicey.
// - Never translate a string to appear on multiple lines. All strings "This is a really long string...",
//   must fit on a single line no matter how long the string is. All strings start with " and end with ",
// - Never remove any extra spaces in strings. In addition, all strings containing multiple sentences only
//   have one space after a period, which is different than standard typing convention. Never modify sections
//   of strings contain combinations of % characters. These are special format characters and are always
//   used in conjunction with other characters. For example, %s means string, and is commonly used for names,
//   locations, items, etc. %d is used for numbers. %c%d is a character and a number (such as A9).
//   %% is how a single % character is built. There are countless types, but strings containing these
//   special characters are usually commented to explain what they mean. If it isn't commented, then
//   if you can't figure out the context, then feel free to ask SirTech.
// - Comments are always started with // Anything following these two characters on the same line are
//   considered to be comments. Do not translate comments. Comments are always applied to the following
//   string(s) on the next line(s), unless the comment is on the same line as a string.
// - All new comments made by SirTech will use "//@@@ comment" (without the quotes) notation. By searching
//   for @@@ everytime you recieve a new version, it will simplify your task and identify special instructions.
//   Commonly, these types of comments will be used to ask you to abbreviate a string. Please leave the
//   comments intact, and SirTech will remove them once the translation for that particular area is resolved.
// - If you have a problem or question with translating certain strings, please use "//!!! comment"
//   (without the quotes). The syntax is important, and should be identical to the comments used with @@@
//   symbols. SirTech will search for !!! to look for Topware problems and questions. This is a more
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
//   EX: "|Map Screen"
//
//   This means the 'M' is the hotkey. In the game, when somebody hits the 'M' key, it activates that
//   button. When translating the text to another language, it is best to attempt to choose a word that
//   uses 'M'. If you can't always find a match, then the best thing to do is append the 'M' at the end
//   of the string in this format:
//
//   EX: "Ecran De Carte (|M)" (this is the French translation)
//
//   Other examples are used multiple times, like the Esc key or "|E|s|c" or Space -> (|S|p|a|c|e)
//
// 2) NEWLINE
//   Any place you see a \n within the string, you are looking at another string that is part of the fast help
//   text system. \n notation doesn't need to be precisely placed within that string, but whereever you wish
//   to start a new line.
//
//   EX: "Clears all the mercs' positions,\nand allows you to re-enter them manually."
//
//   Would appear as:
//
//     Clears all the mercs' positions,
//     and allows you to re-enter them manually.
//
//   NOTE: It is important that you don't pad the characters adjacent to the \n with spaces. If we did this
//     in the above example, we would see
//
//   WRONG WAY -- spaces before and after the \n
//   EX: "Clears all the mercs' positions, \n and allows you to re-enter them manually."
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
//   Throughout the text files, you'll find an assortment of comments. Comments are used to describe the
//   text to make translation easier, but comments don't need to be translated. A good thing is to search for
//   "@@@" after receiving new version of the text file, and address the special notes in this manner.
//
// !!! NOTATION
// ************
//
//   As described above, the "!!!" notation should be used by Topware to ask questions and address problems as
//   SirTech uses the "@@@" notation.



static ST::string s_ger_WeaponType[WeaponType_SIZE] =
{
	"Andere",
	"Pistole",
	"Maschinenpistole",
	"Schwere Maschinenpistole",
	"Gewehr",
	"Scharfschützengewehr",
	"Sturmgewehr",
	"Leichtes Maschinengewehr",
	"Schrotflinte",
};

static ST::string s_ger_TeamTurnString[TeamTurnString_SIZE] =
{
	"Spielzug Spieler",
	"Spielzug Gegner",
	"Spielzug Monster",
	"Spielzug Miliz",
	"Spielzug Zivilisten",
	// planning turn
};

static ST::string s_ger_Message[Message_SIZE] =
{
	// In the following 8 strings, the %s is the merc's name, and the %d (if any) is a number.

	"%s am Kopf getroffen, verliert einen Weisheitspunkt!",
	"%s an der Schulter getroffen, verliert Geschicklichkeitspunkt!",
	"%s an der Brust getroffen, verliert einen Kraftpunkt!",
	"%s an den Beinen getroffen, verliert einen Beweglichkeitspunkt!",
	"%s am Kopf getroffen, verliert %d Weisheitspunkte!",
	"%s an der Schulter getroffen, verliert %d Geschicklichkeitspunkte!",
	"%s an der Brust getroffen, verliert %d Kraftpunkte!",
	"%s an den Beinen getroffen, verliert %d Beweglichkeitspunkte!",
	"Unterbrechung!",

	"Verstärkung ist angekommen!",

	// In the following four lines, all %s's are merc names

	"%s lädt nach.",
	"%s hat nicht genug Action-Punkte!",
	// the following 17 strings are used to create lists of gun advantages and disadvantages
	// (separated by commas)
	"zuverlässig",
	"unzuverlässig",
	"Reparatur leicht",
	"Reparatur schwer",
	"große Durchschlagskraft",
	"kleine Durchschlagskraft",
	"feuert schnell",
	"feuert langsam",
	"große Reichweite",
	"kurze Reichweite",
	"leicht",
	"schwer",
	"klein",
	"schneller Feuerstoß",
	"kein Feuerstoß",
	"großes Magazin",
	"kleines Magazin",

	// In the following two lines, all %s's are merc names

	"%ss Tarnung hat sich abgenutzt.",
	"%ss Tarnung ist weggewaschen.",

	// The first %s is a merc name and the second %s is an item name

	"Zweite Waffe hat keine Munition!",
	"%s hat %s gestohlen.",

	// The %s is a merc name

	"%ss Waffe kann keinen Feuerstoß abgeben.",

	"Sie haben schon eines davon angebracht.",
	"Gegenstände zusammenfügen?",

	// Both %s's are item names

	"Sie können %s mit %s nicht zusammenfügen",

	"Keine",
	"Munition entfernen",
	"Modifikationen",

	//You cannot use "item(s)" and your "other item" at the same time.
	//Ex: You cannot use sun goggles and you gas mask at the same time.
	"Sie können %s nicht zusammen mit %s benutzen.",	//

	"Der Gegenstand in Ihrem Cursor kann mit anderen Gegenständen verbunden werden, indem Sie ihn in einem der vier Slots platzieren",
	"Der Gegenstand in Ihrem Cursor kann mit anderen Gegenständen verbunden werden, indem Sie ihn in einem der vier Attachment-Slots platzieren. (Aber in diesem Fall sind die Gegenstände nicht kompatibel.)",
	"Es sind noch Feinde im Sektor!",
	"Geben Sie %s %s",
	"%s am Kopf getroffen!",
	"Kampf abbrechen?",
	"Die Modifikation ist permanent. Weitermachen?",
	"%s fühlt sich frischer!",
	"%s ist auf Murmeln ausgerutscht!",
	"%s konnte %s nicht greifen!",
	"%s hat %s repariert",
	"Unterbrechung für ",
	"Ergeben?",
	"Diese Person will keine Hilfe.",
	"Lieber NICHT!",
	"Wenn Sie zu Skyriders Heli wollen, müssen Sie Söldner einem FAHRZEUG/HELIKOPTER ZUWEISEN.",
	"%s hat nur Zeit, EINE Waffe zu laden",
	"Spielzug Bloodcats",
};


// the types of time compression. For example: is the timer paused? at normal speed, 5 minutes per second, etc.
// min is an abbreviation for minutes
static const ST::string s_ger_sTimeStrings[sTimeStrings_SIZE] =
{
	"Pause",
	"Normal",
	"5 Min",
	"30 Min",
	"60 Min",
	"6 Std",
};

// Assignment Strings: what assignment does the merc have right now? For example, are they on a squad, training,
// administering medical aid (doctor) or training a town. All are abbreviated. 8 letters is the longest it can be.
static const ST::string s_ger_pAssignmentStrings[pAssignmentStrings_SIZE] =
{
	"Trupp 1",
	"Trupp 2",
	"Trupp 3",
	"Trupp 4",
	"Trupp 5",
	"Trupp 6",
	"Trupp 7",
	"Trupp 8",
	"Trupp 9",
	"Trupp 10",
	"Trupp 11",
	"Trupp 12",
	"Trupp 13",
	"Trupp 14",
	"Trupp 15",
	"Trupp 16",
	"Trupp 17",
	"Trupp 18",
	"Trupp 19",
	"Trupp 20",
	"Dienst", // on active duty
	"Doktor", // administering medical aid
	"Patient", // getting medical aid
	"Fahrzeug", // in a vehicle
	"Transit", // in transit - abbreviated form
	"Repar.", // repairing
	"Üben", // training themselves
	"Miliz", // training a town to revolt
	"Trainer", // training a teammate
	"Rekrut", // being trained by someone else
	"Tot", // dead
	"Unfähig", // abbreviation for incapacitated
	"Gefangen", // Prisoner of war - captured
	"Hospital", // patient in a hospital
	"Leer",	//Vehicle is empty
};

static const ST::string s_ger_pMilitiaString[pMilitiaString_SIZE] =
{
	"Miliz", // the title of the militia box
	"Ohne Aufgabe", //the number of unassigned militia troops
	"Mit Feinden im Sektor können Sie keine Miliz einsetzen!",
};

static const ST::string s_ger_pMilitiaButtonString[pMilitiaButtonString_SIZE] =
{
	"Autom.", // auto place the militia troops for the player
	"Fertig", // done placing militia troops
};

static const ST::string s_ger_pConditionStrings[pConditionStrings_SIZE] =
{
	"Sehr gut", //the state of a soldier .. excellent health
	"Gut", // good health
	"Mittel", // fair health
	"Verwundet", // wounded health
	"Erschöpft", // tired
	"Verblutend", // bleeding to death
	"Bewusstlos", // knocked out
	"Stirbt", // near death
	"Tot", // dead
};

static const ST::string s_ger_pEpcMenuStrings[pEpcMenuStrings_SIZE] =
{
	"Dienst", // set merc on active duty
	"Patient", // set as a patient to receive medical aid
	"Fahrzeug", // tell merc to enter vehicle
	"Unbewacht", // let the escorted character go off on their own
	"Abbrechen", // close this menu
};

// look at pAssignmentString above for comments
static const ST::string s_ger_pLongAssignmentStrings[pLongAssignmentStrings_SIZE] =
{
	"Trupp 1",
	"Trupp 2",
	"Trupp 3",
	"Trupp 4",
	"Trupp 5",
	"Trupp 6",
	"Trupp 7",
	"Trupp 8",
	"Trupp 9",
	"Trupp 10",
	"Trupp 11",
	"Trupp 12",
	"Trupp 13",
	"Trupp 14",
	"Trupp 15",
	"Trupp 16",
	"Trupp 17",
	"Trupp 18",
	"Trupp 19",
	"Trupp 20",
	"Dienst",
	"Doktor",
	"Patient",
	"Fahrzeug",
	"Transit",
	"Reparieren",
	"Üben",
	"Miliz",
	"Trainer",
	"Rekrut",
	"Tot",
	"Unfähig",
	"Gefangen",
	"Hospital", // patient in a hospital
	"Leer",	// Vehicle is empty
};

// the contract options
static const ST::string s_ger_pContractStrings[pContractStrings_SIZE] =
{
	"Vertragsoptionen:",
	"", // a blank line, required
	"Einen Tag anbieten", // offer merc a one day contract extension
	"Eine Woche anbieten", // 1 week
	"Zwei Wochen anbieten", // 2 week
	"Entlassen", //end merc's contract (used to be "Terminate")
	"Abbrechen", // stop showing this menu
};

static const ST::string s_ger_pPOWStrings[pPOWStrings_SIZE] =
{
	"gefangen", //an acronym for Prisoner of War
	"??",
};

static const ST::string s_ger_pInvPanelTitleStrings[pInvPanelTitleStrings_SIZE] =
{
	"Rüstung", // the armor rating of the merc
	"Gewicht", // the weight the merc is carrying
	"Tarnung", // the merc's camouflage rating
};

static const ST::string s_ger_pShortAttributeStrings[pShortAttributeStrings_SIZE] =
{
	"Bew", // the abbreviated version of : agility
	"Ges", // dexterity
	"Krf", // strength
	"Fhr", // leadership
	"Wsh", // wisdom
	"Erf", // experience level
	"Trf", // marksmanship skill
	"Spr", // explosive skill
	"Tec", // mechanical skill
	"Med", // medical skill
};

static const ST::string s_ger_pUpperLeftMapScreenStrings[pUpperLeftMapScreenStrings_SIZE] =
{
	"Aufgabe", // the mercs current assignment
	"Gesundh.", // the health level of the current merc
	"Moral", // the morale of the current merc
	"Zustand",	// the condition of the current vehicle
};

static const ST::string s_ger_pTrainingStrings[pTrainingStrings_SIZE] =
{
	"Üben", // tell merc to train self
	"Miliz", // tell merc to train town //
	"Trainer", // tell merc to act as trainer
	"Rekrut", // tell merc to be train by other
};

static const ST::string s_ger_pAssignMenuStrings[pAssignMenuStrings_SIZE] =
{
	"Dienst", // merc is on active duty
	"Doktor", // the merc is acting as a doctor
	"Patient", // the merc is receiving medical attention
	"Fahrzeug", // the merc is in a vehicle
	"Reparieren", // the merc is repairing items
	"Training", // the merc is training
	"Abbrechen", // cancel this menu
};

static const ST::string s_ger_pRemoveMercStrings[pRemoveMercStrings_SIZE] ={
	"Söldner entfernen", // remove dead merc from current team
	"Abbrechen",
};

static const ST::string s_ger_pAttributeMenuStrings[pAttributeMenuStrings_SIZE] =
{
	"Kraft",
	"Geschicklichkeit",
	"Beweglichkeit",
	"Gesundheit",
	"Treffsicherheit",
	"Medizin",
	"Technik",
	"Führungsqualität",
	"Sprengstoffe",
	"Abbrechen",
};

static const ST::string s_ger_pTrainingMenuStrings[pTrainingMenuStrings_SIZE] =
{
	"Üben", // train yourself
	"Miliz", // train the town
	"Trainer", // train your teammates
	"Rekrut", // be trained by an instructor
	"Abbrechen", // cancel this menu
};

static const ST::string s_ger_pSquadMenuStrings[pSquadMenuStrings_SIZE] =
{
	"Trupp  1",
	"Trupp  2",
	"Trupp  3",
	"Trupp  4",
	"Trupp  5",
	"Trupp  6",
	"Trupp  7",
	"Trupp  8",
	"Trupp  9",
	"Trupp 10",
	"Trupp 11",
	"Trupp 12",
	"Trupp 13",
	"Trupp 14",
	"Trupp 15",
	"Trupp 16",
	"Trupp 17",
	"Trupp 18",
	"Trupp 19",
	"Trupp 20",
	"Abbrechen",
};


static const ST::string s_ger_pPersonnelScreenStrings[pPersonnelScreenStrings_SIZE] =
{
	"Med. Versorgung:", // amount of medical deposit put down on the merc
	"Laufzeit:", // time remaining on current contract
	"Getötet", // number of kills by merc
	"Mithilfe", // number of assists on kills by merc
	"Tägliche Kosten:", // daily cost of merc
	"Gesamtkosten:", // total cost of merc
	"Vertrag:", // cost of current contract
	"Diensttage:", // total service rendered by merc
	"Schulden:", // amount left on MERC merc to be paid
	"Prozentzahl Treffer:", // percentage of shots that hit target
	"Einsätze", // number of battles fought
	"Verwundungen", // number of times merc has been wounded
	"Fähigkeiten:",
	"Keine Fähigkeiten"
};


//These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
static const ST::string s_ger_gzMercSkillText[gzMercSkillText_SIZE] =
{
	"Keine Fähigkeiten",
	"Schlösser knacken",
	"Nahkampf",
	"Elektronik",
	"Nachteinsatz",
	"Werfen",
	"Lehren",
	"Schwere Waffen",
	"Autom. Waffen",
	"Schleichen",
	"Geschickt",
	"Dieb",
	"Kampfsport",
	"Messer",
	"Dach-Treffer-Bonus",
	"Getarnt",
	"(Experte)",
};

// This is pop up help text for the options that are available to the merc
static const ST::string s_ger_pTacticalPopupButtonStrings[pTacticalPopupButtonStrings_SIZE] =
{
	"|Stehen/Gehen",
	"Kauern/Kauernd bewegen (|C)",
	"Stehen/|Rennen",
	"Hinlegen/Kriechen (|P)",
	"Schauen (|L)",
	"Action",
	"Reden",
	"Untersuchen (|S|t|r|g)",

	//Pop up door menu
	"Manuell öffnen",
	"Auf Fallen untersuchen",
	"Dietrich",
	"Mit Gewalt öffnen",
	"Falle entschärfen",
	"Abschließen",
	"Aufschließen",
	"Sprengstoff an Tür benutzen",
	"Brecheisen benutzen",
	"Rückgängig (|E|s|c)",
	"Schließen",
};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.
static const ST::string s_ger_pDoorTrapStrings[pDoorTrapStrings_SIZE] =
{
	"keine Falle",
	"eine Sprengstofffalle",
	"eine elektrische Falle",
	"eine Falle mit Sirene",
	"eine Falle mit stummem Alarm",
};

// On the map screen, there are four columns. This text is popup help text that identifies the individual columns.
static const ST::string s_ger_pMapScreenMouseRegionHelpText[pMapScreenMouseRegionHelpText_SIZE] =
{
	"Charakter auswählen",
	"Söldner einteilen",
	"Marschroute",

	//The new 'c' key activates this option. Either reword this string to include a 'c' in it, or leave as is.
	"Vertrag für Söldner (|c)",

	"Söldner entfernen",
	"Schlafen",
};

// volumes of noises
static const ST::string s_ger_pNoiseVolStr[pNoiseVolStr_SIZE] =
{
	"LEISE",
	"DEUTLICH",
	"LAUT",
	"SEHR LAUT",
};

// types of noises
static const ST::string s_ger_pNoiseTypeStr[pNoiseTypeStr_SIZE] =
{
	"EIN UNBEKANNTES GERÄUSCH",
	"EINE BEWEGUNG",
	"EIN KNARREN",
	"EIN KLATSCHEN",
	"EINEN AUFSCHLAG",
	"EINEN SCHUSS",
	"EINE EXPLOSION",
	"EINEN SCHREI",
	"EINEN AUFSCHLAG",
	"EINEN AUFSCHLAG",
	"EIN ZERBRECHEN",
	"EIN ZERSCHMETTERN",
};

// Directions that are used throughout the code for identification.
static const ST::string s_ger_pDirectionStr[pDirectionStr_SIZE] =
{
	"NORDOSTEN",
	"OSTEN",
	"SÜDOSTEN",
	"SÜDEN",
	"SÜDWESTEN",
	"WESTEN",
	"NORDWESTEN",
	"NORDEN",
};

static const ST::string s_ger_gpStrategicString[gpStrategicString_SIZE] =
{
	"%s wurden entdeckt in Sektor %s und ein weiterer Trupp wird gleich ankommen.",	//STR_DETECTED_SINGULAR
	"%s wurden entdeckt in Sektor %s und weitere Trupps werden gleich ankommen.",	//STR_DETECTED_PLURAL
	"Gleichzeitige Ankunft koordinieren?",													//STR_COORDINATE

	//Dialog strings for enemies.

	"Feind bietet die Chance zum Aufgeben an.",			//STR_ENEMY_SURRENDER_OFFER
	"Feind hat restliche bewusstlose Söldner gefangen genommen.",	//STR_ENEMY_CAPTURED

	//The text that goes on the autoresolve buttons

	"Rückzug", 		//The retreat button				//STR_AR_RETREAT_BUTTON
	"Fertig",		//The done button				//STR_AR_DONE_BUTTON

	//The headers are for the autoresolve type (MUST BE UPPERCASE)

	"VERTEIDIGUNG",								//STR_AR_DEFEND_HEADER
	"ANGRIFF",									//STR_AR_ATTACK_HEADER
	"BEGEGNUNG",									//STR_AR_ENCOUNTER_HEADER
	"Sektor",		//The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

	//The battle ending conditions

	"SIEG!",									//STR_AR_OVER_VICTORY
	"NIEDERLAGE!",								//STR_AR_OVER_DEFEAT
	"AUFGEGEBEN!",									//STR_AR_OVER_SURRENDERED
	"GEFANGENGENOMMEN!",							//STR_AR_OVER_CAPTURED
	"ZURÜCKGEZOGEN!",									//STR_AR_OVER_RETREATED

	//These are the labels for the different types of enemies we fight in autoresolve.

	"Miliz",							//STR_AR_MILITIA_NAME,
	"Elite",								//STR_AR_ELITE_NAME,
	"Trupp",								//STR_AR_TROOP_NAME,
	"Verwal",								//STR_AR_ADMINISTRATOR_NAME,
	"Monster",								//STR_AR_CREATURE_NAME,

	//Label for the length of time the battle took

	"Zeit verstrichen",							//STR_AR_TIME_ELAPSED,

	//Labels for status of merc if retreating. (UPPERCASE)

	"HAT SICH ZURÜCKGEZOGEN",								//STR_AR_MERC_RETREATED,
	"ZIEHT SICH ZURÜCK",								//STR_AR_MERC_RETREATING,
	"RÜCKZUG",								//STR_AR_MERC_RETREAT,

	//PRE BATTLE INTERFACE STRINGS
	//Goes on the three buttons in the prebattle interface. The Auto resolve button represents
	//a system that automatically resolves the combat for the player without having to do anything.
	//These strings must be short (two lines -- 6-8 chars per line)

	"PC Kampf",							//STR_PB_AUTORESOLVE_BTN,
	"Gehe zu Sektor",							//STR_PB_GOTOSECTOR_BTN,
	"Rückzug",							//STR_PB_RETREATMERCS_BTN,

	//The different headers(titles) for the prebattle interface.
	"FEINDBEGEGNUNG",
	"FEINDLICHE INVASION",
	"FEINDLICHER HINTERHALT",
	"FEINDLICHEN SEKTOR BETRETEN",
	"MONSTERANGRIFF",
	"BLOODCAT-HINTERHALT",
	"BLOODCAT-HÖHLE BETRETEN",

	//Various single words for direct translation. The Civilians represent the civilian
	//militia occupying the sector being attacked. Limited to 9-10 chars

	"Ort",
	"Feinde",
	"Söldner",
	"Miliz",
	"Monster",
	"Bloodcats",
	"Sektor",
	"Keine",		//If there are no uninvolved mercs in this fight.
	"n.a.",			//Acronym of Not Applicable
	"T",			//One letter abbreviation of day
	"h",			//One letter abbreviation of hour

	//TACTICAL PLACEMENT USER INTERFACE STRINGS
	//The four buttons

	"Räumen",
	"Verteilen",
	"Gruppieren",
	"Fertig",

	//The help text for the four buttons. Use \n to denote new line (just like enter).

	"Söldner räumen ihre Positionen und\nkönnen manuell neu platziert werden. (|C)",
	"Söldner |schwärmen in alle Richtungen\naus wenn der Button gedrückt wird.",
	"Mit diesem Button können Sie wählen, wo die Söldner |gruppiert werden sollen.",
	"Klicken Sie auf diesen Button, wenn Sie die\nPositionen der Söldner gewählt haben. (|E|i|n|g|a|b|e)",
	"Sie müssen alle Söldner positionieren\nbevor die Schlacht beginnt.",

	//Various strings (translate word for word)

	"Sektor",
	"Eintrittspunkte wählen",

	//Strings used for various popup message boxes. Can be as long as desired.

	"Das sieht nicht gut aus. Gelände ist unzugänglich. Versuchen Sie es an einer anderen Stelle.",
	"Platzieren Sie Ihre Söldner in den markierten Sektor auf der Karte.",

	//These entries are for button popup help text for the prebattle interface. All popup help
	//text supports the use of \n to denote new line. Do not use spaces before or after the \n.
	"Entscheidet Schlacht |automatisch für Sie\nohne Karte zu laden.",
	"Sie können den PC-Kampf-Modus nicht benutzen, während Sie\neinen vom Feind verteidigten Ort angreifen.",
	"Sektor b|etreten und Feind in Kampf verwickeln.",
	"Gruppe zum vorigen Sektor zu|rückziehen.",				//singular version
	"Alle Gruppen zum vorigen Sektor zu|rückziehen.", //multiple groups with same previous sector

	//various popup messages for battle conditions.

	//%s -- ex: A9
	"Feinde attackieren Ihre Miliz im Sektor %s.",
	//%s -- ex: A9
	"Monster attackieren Ihre Miliz im Sektor %s.",
	//1st %d refers to the number of civilians eaten by monsters, %s -- ex: A9
	//Note: the minimum number of civilians eaten will be two.
	"Monster attackieren und töten %d Zivilisten im Sektor %s.",
	//%s is the sector -- ex: A9
	"Feinde attackieren Ihre Söldner im Sektor %s. Alle Söldner sind bewusstlos!",
	//%s is the sector -- ex: A9
	"Monster attackieren Ihre Söldner im Sektor %s. Alle Söldner sind bewusstlos!",

};

//This is the day represented in the game clock. Must be very short, 4 characters max.
static const ST::string s_ger_gpGameClockString = "Tag";

//When the merc finds a key, they can get a description of it which
//tells them where and when they found it.
static const ST::string s_ger_sKeyDescriptionStrings[sKeyDescriptionStrings_SIZE] =
{
	"gefunden im Sektor:",
	"gefunden am:",
};

//The headers used to describe various weapon statistics.
static ST::string s_ger_gWeaponStatsDesc[ gWeaponStatsDesc_SIZE] =
{
	"Gewicht (%s):", //weight
	"Status:",
	"Anzahl:", 		// Number of bullets left in a magazine
	"Reichw.:",		// Range
	"Schaden:",
	"AP:",			// abbreviation for Action Points
	"="
};

//The headers used for the merc's money.
static const ST::string s_ger_gMoneyStatsDesc[gMoneyStatsDesc_SIZE] =
{
	"Betrag",
	"Verbleibend:", //this is the overall balance
	"Betrag",
	"Abteilen:", // the amount he wants to separate from the overall balance to get two piles of money
	"Konto",
	"Saldo",
	"Betrag",
	"abheben",
};

//The health of various creatures, enemies, characters in the game. The numbers following each are for comment
//only, but represent the precentage of points remaining.
static const ST::string s_ger_zHealthStr[zHealthStr_SIZE] =	//used to be 10
{
	"STIRBT",		//	>= 0
	"KRITISCH", 	//	>= 15
	"SCHLECHT",	//	>= 30
	"VERWUNDET",	//	>= 45
	"GESUND",		//	>= 60
	"STARK",			// 	>= 75
	"SEHR GUT",	// 	>= 90
};

static const ST::string s_ger_gzMoneyAmounts[gzMoneyAmounts_SIZE] =
{
	"$1000",
	"$100",
	"$10",
	"OK",
	"Abteilen",
	"Abheben",
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
static const ST::string s_ger_gzProsLabel = "Pro:";
static const ST::string s_ger_gzConsLabel = "Kontra:";

//Conversation options a player has when encountering an NPC
static ST::string s_ger_zTalkMenuStrings[zTalkMenuStrings_SIZE] =
{
	"Wie bitte?", 	//meaning "Repeat yourself"
	"Freundlich",		//approach in a friendly
	"Direkt",		//approach directly - let's get down to business
	"Drohen",		//approach threateningly - talk now, or I'll blow your face off
	"Geben",
	"Rekrutieren",
};

//Some NPCs buy, sell or repair items. These different options are available for those NPCs as well.
static ST::string s_ger_zDealerStrings[zDealerStrings_SIZE] =
{
	"Handeln",
	"Kaufen",
	"Verkaufen",
	"Reparieren",
};

static const ST::string s_ger_zDialogActions = "Fertig";

static const ST::string s_ger_pVehicleStrings[pVehicleStrings_SIZE] =
{
	"Eldorado",
	"Hummer", // a hummer jeep/truck -- military vehicle
	"Ice Cream Truck",
	"Jeep",
	"Tank",
	"Helikopter",
};

static const ST::string s_ger_pShortVehicleStrings[pVehicleStrings_SIZE] =
{
	"Eldor.",
	"Hummer",			// the HMVV
	"Truck",
	"Jeep",
	"Tank",
	"Heli", 				// the helicopter
};

static const ST::string s_ger_zVehicleName[pVehicleStrings_SIZE] =
{
	"Eldorado",
	"Hummer",		//a military jeep. This is a brand name.
	"Truck",			// Ice cream truck
	"Jeep",
	"Tank",
	"Heli", 		//an abbreviation for Helicopter
};

//These are messages Used in the Tactical Screen
static ST::string s_ger_TacticalStr[TacticalStr_SIZE] =
{
	"Luftangriff",
	"Automatisch Erste Hilfe leisten?",

	// CAMFIELD NUKE THIS and add quote #66.

	"%s bemerkt, dass Teile aus der Lieferung fehlen.",

	// The %s is a string from pDoorTrapStrings

	"Das Schloss hat %s.",
	"Es gibt kein Schloss.",
	"Das Schloss hat keine Falle.",
	// The %s is a merc name
	"%s hat nicht den richtigen Schlüssel.",
	"Das Schloss hat keine Falle.",
	"Geschl.",
	"TÜR",
	"FALLE AN",
	"Geschl.",
	"GEÖFFNET",
	"EINGETRETEN",
	"Hier ist ein Schalter. Betätigen?",
	"Falle entschärfen?",
	"Mehr...",

	// In the next 2 strings, %s is an item name

	"%s liegt jetzt auf dem Boden.",
	"%s ist jetzt bei %s.",

	// In the next 2 strings, %s is a name

	"%s hat den vollen Betrag erhalten.",
	"%s bekommt noch %d.",
	"Detonationsfrequenz auswählen:",		//in this case, frequency refers to a radio signal
	"Wie viele Züge bis zur Explosion:",	//how much time, in turns, until the bomb blows
	"Ferngesteuerte Zündung einstellen:",//in this case, frequency refers to a radio signal
	"Falle entschärfen?",
	"Blaue Flagge wegnehmen?",
	"Blaue Flagge hier aufstellen?",
	"Zug beenden",

	// In the next string, %s is a name. Stance refers to way they are standing.

	"Wollen Sie %s wirklich angreifen?",
	"Fahrzeuge können ihre Position nicht ändern.",
	"Der Roboter kann seine Position nicht ändern.",

	// In the next 3 strings, %s is a name

	//%s can't change to that stance here
	"%s kann die Haltung hier nicht ändern.",

	"%s kann hier nicht versorgt werden.",
	"%s braucht keine Erste Hilfe.",
	"Kann nicht dorthin gehen.",
	"Ihr Team ist komplett. Kein Platz mehr für Rekruten.",	//there's no room for a recruit on the player's team

	// In the next string, %s is a name

	"%s wurde rekrutiert.",

	// Here %s is a name and %d is a number

	"%s bekommt noch %d $.",

	// In the next string, %s is a name

	"%s eskortieren?",

	// In the next string, the first %s is a name and the second %s is an amount of money (including $ sign)

	"%s für %s pro Tag anheuern?",

	// This line is used repeatedly to ask player if they wish to participate in a boxing match.

	"Kämpfen?",

	// In the next string, the first %s is an item name and the
	// second %s is an amount of money (including $ sign)

	"%s für %s kaufen?",

	// In the next string, %s is a name

	"%s wird von Trupp %d eskortiert.",

	// These messages are displayed during play to alert the player to a particular situation

	"KLEMMT",					//weapon is jammed.
	"Roboter braucht %s Munition.",		//Robot is out of ammo
	"Dorthin werfen? Unmöglich.",		//Merc can't throw to the destination he selected

	// These are different buttons that the player can turn on and off.

	"Stealth Mode (|Z)",
	"Kartenbildschir|m",
	"Spielzug been|den",
	"Sprechen",
	"Stumm",
	"Aufrichten (|B|i|l|d |h|o|c|h)",
	"Cursor Level (|T|a|b|u|l|a|t|o|r)",
	"Klettern / Springen",
	"Ducken (|B|i|l|d |r|u|n|t|e|r)",
	"Untersuchen (|S|t|r|g)",
	"Voriger Söldner",
	"Nächster Söldner (|L|e|e|r|t|a|s|t|e)",
	"|Optionen",
	"Feuerstoß (|B)",
	"B|lickrichtung",
	"Gesundheit: %d/%d\nEnergie: %d/%d\nMoral: %s",
	"Was?",					//this means "what?"
	"Forts.",					//an abbrieviation for "Continued"
	"Stumm aus für %s.",
	"Stumm für %s.", //
	"Fahrer",
	"Fahrzeug verlassen",
	"Trupp wechseln",
	"Fahren",
	"n.a.",						//this is an acronym for "Not Applicable."
	"Benutzen ( Faustkampf )",
	"Benutzen ( Feuerwaffe )",
	"Benutzen ( Hieb-/Stichwaffe )",
	"Benutzen ( Sprengstoff )",
	"Benutzen ( Verbandskasten )",
	"(Fangen)",
	"(Nachladen)",
	"(Geben)",
	"%s Falle wurde ausgelöst.",
	"%s ist angekommen.",
	"%s hat keine Action-Punkte mehr.",
	"%s ist nicht verfügbar.",
	"%s ist fertig verbunden.",
	"%s sind die Verbände ausgegangen.",
	"Feind im Sektor!",
	"Keine Feinde in Sicht.",
	"Nicht genug Action-Punkte.",
	"Niemand bedient die Fernbedienung.",
	"Feuerstoß hat Magazin geleert!",
	"SOLDAT",
	"MONSTER",
	"MILIZ",
	"ZIVILIST",
	"Sektor verlassen",
	"OK",
	"Abbruch",
	"Gewählter Söldner",
	"Ganzer Trupp",
	"Gehe zu Sektor",

	"Gehe zu Karte",

	"Sie können den Sektor von dieser Seite aus nicht verlassen.",
	"%s ist zu weit weg.",
	"Baumkronen entfernen",
	"Baumkronen zeigen",
	"KRÄHE",				//Crow, as in the large black bird
	"NACKEN",
	"KOPF",
	"TORSO",
	"BEINE",
	"Der Herrin sagen, was sie wissen will?",
	"Fingerabdruck-ID gespeichert",
	"Falsche Fingerabdruck-ID. Waffe außer Betrieb",
	"Ziel erfasst",
	"Weg blockiert",
	"Geld einzahlen/abheben",		//Help text over the $ button on the Single Merc Panel
	"Niemand braucht Erste Hilfe.",
	"Klemmt.",			//Short form of JAMMED, for small inv slots
	"Kann da nicht hin.",					// used ( now ) for when we click on a cliff
	"Person will sich nicht bewegen",
	// In the following message, '%s' would be replaced with a quantity of money (e.g. $200)
	"Mit der Zahlung von %s einverstanden?",
	"Gratisbehandlung akzeptieren?",
	"Daryl heiraten?",
	"Schlüsselring",
	"Das ist mit einem EPC nicht möglich.",
	"Krott verschonen?",
	"Außer Reichweite",
	"Arbeiter",	//People that work in mines to extract precious metals
	"Fahrzeug kann nur zwischen Sektoren fahren",
	"Automatische Erste Hilfe nicht möglich",
	"Weg blockiert für %s",
	"Ihre von Deidrannas Truppe gefangenen Soldaten sind hier inhaftiert",
	"Schloss getroffen",
	"Schloss zerstört",
	"Noch jemand an der Tür.",
	"Gesundh.: %d/%d\nTank: %d/%d",
	"%s kann %s nicht sehen.", // Cannot see person trying to talk to
};

//Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different circumstances in the "exiting sector" interface.
static const ST::string s_ger_pExitingSectorHelpText[pExitingSectorHelpText_SIZE] =
{
	//Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box is checked.
	"Der nächste Sektor wird sofort geladen, wenn Sie das Kästchen aktivieren.",
	"Sie kommen sofort zum Kartenbildschirm, wenn Sie das Kästchen aktivieren\nweil die Reise Zeit braucht.",

	//If you attempt to leave a sector when you have multiple squads in a hostile sector.
	"Der Sektor ist von Feinden besetzt. Sie können keine Söldner hier lassen.\nRegeln Sie das, bevor Sie neue Sektoren laden.",

	//Because you only have one squad in the sector, and the "move all" option is checked, the "go to sector" option is locked to on.
	//The helptext explains why it is locked.
	"Wenn die restlichen Söldner den Sektor verlassen,\nwird sofort der nächste Sektor geladen.",
	"Wenn die restlichen Söldner den Sektor verlassen,\nkommen Sie sofort zum Kartenbildschirm\nweil die Reise Zeit braucht.",

	//If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being escorted. The "single" button is disabled.
	"%s kann den Sektor nicht ohne Eskorte verlassen.",

	//If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc will be prohibited from leaving alone.
	//There are several strings depending on the gender of the merc and how many EPCs are in the squad.
	//DO NOT USE THE NEWLINE HERE AS IT IS USED FOR BOTH HELPTEXT AND SCREEN MESSAGES!
	"%s kann den Sektor nicht verlassen, weil er %s eskortiert.", //male singular
	"%s kann den Sektor nicht verlassen, weil sie %s eskortiert.", //female singular
	"%s kann den Sektor nicht verlassen, weil er mehrere Personen eskortiert.", //male plural
	"%s kann den Sektor nicht verlassen, weil sie mehrere Personen eskortiert.", //female plural

	//If one or more of your mercs in the selected squad aren't in range of the traversal area, then the "move all" option is disabled,
	//and this helptext explains why.
	"Alle Söldner müssen in der Nähe sein,\ndamit der Trupp weiterreisen kann.",

	//Standard helptext for single movement. Explains what will happen (splitting the squad)
	"Bei aktiviertem Kästchen reist %s alleine und\nbildet automatisch wieder einen Trupp.",

	//Standard helptext for all movement. Explains what will happen (moving the squad)
	"Bei aktiviertem Kästchen reist der ausgewählte Trupp\nweiter und verlässt den Sektor.",

	//This strings is used BEFORE the "exiting sector" interface is created. If you have an EPC selected and you attempt to tactically
	//traverse the EPC while the escorting mercs aren't near enough (or dead, dying, or unconscious), this message will appear and the
	//"exiting sector" interface will not appear. This is just like the situation where
	//This string is special, as it is not used as helptext. Do not use the special newline character (\n) for this string.
	"%s wird von Söldnern eskortiert und kann den Sektor nicht alleine verlassen. Die anderen Söldner müssen in der Nähe sein.",
};

static const ST::string s_ger_pRepairStrings[pRepairStrings_SIZE] =
{
	"Gegenstände", 		// tell merc to repair items in inventory
	"Raketenstützpunkt", 		// tell merc to repair SAM site - SAM is an acronym for Surface to Air Missile
	"Abbruch", 		// cancel this menu
	"Roboter", 		// repair the robot
};

// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."
static const ST::string s_ger_sPreStatBuildString[sPreStatBuildString_SIZE] =
{
	"verliert", 		// the merc has lost a statistic
	"gewinnt", 		// the merc has gained a statistic
	"Punkt",	// singular
	"Punkte",	// plural
	"Level",	// singular
	"Level",	// plural
};

static const ST::string s_ger_sStatGainStrings[sStatGainStrings_SIZE] =
{
	"Gesundheit.",
	"Beweglichkeit",
	"Geschicklichkeit",
	"Weisheit.",
	"an Medizin.",
	"an Sprengstoff.",
	"an Technik.",
	"an Treffsicherheit.",
	"Erfahrungsstufe(n).",
	"Kraft.",
	"Führungsqualität.",
};

static const ST::string s_ger_pHelicopterEtaStrings[pHelicopterEtaStrings_SIZE] =
{
	"Gesamt: ", 			// total distance for helicopter to travel
	" Sicher: ", 		// Number of safe sectors
	" Unsicher:", 		// Number of unsafe sectors
	"Gesamtkosten: ",// total cost of trip by helicopter
	"Ank.: ", 				// ETA is an acronym for "estimated time of arrival"

	// warning that the sector the helicopter is going to use for refueling is under enemy control
	"Helikopter hat fast keinen Sprit mehr und muss im feindlichen Gebiet landen.",
	"Passagiere: ",
	"Skyrider oder Absprungsort auswählen?",
	"Skyrider",
	"Absprung", //make sure length doesn't exceed 8 characters (used to be "Absprungsort")
};

static const ST::string s_ger_sMapLevelString = "Ebene:"; // what level below the ground is the player viewing in mapscreen

static const ST::string s_ger_gsLoyalString = "%d%% Loyalität"; // the loyalty rating of a town ie : Loyal 53%


// error message for when player is trying to give a merc a travel order while he's underground.
static const ST::string s_ger_gsUndergroundString = "Ich kann unter der Erde keinen Marschbefehl empfangen.";

static const ST::string s_ger_gsTimeStrings[gsTimeStrings_SIZE] =
{
	"h",				// hours abbreviation
	"m",				// minutes abbreviation
	"s",				// seconds abbreviation
	"T",				// days abbreviation
};

// text for the various facilities in the sector
static const ST::string s_ger_sFacilitiesStrings[sFacilitiesStrings_SIZE] =
{
	"Keine",
	"Hospital",
	"Fabrik",
	"Gefängnis",
	"Militär",
	"Flughafen",
	"Schießstand", // a field for soldiers to practise their shooting skills
};

// text for inventory pop up button
static const ST::string s_ger_pMapPopUpInventoryText[pMapPopUpInventoryText_SIZE] =
{
	"Inventar",
	"Exit",
};

// town strings
static const ST::string s_ger_pwTownInfoStrings[pwTownInfoStrings_SIZE] =
{
	"Größe",					// size of the town in sectors
	"unter Kontrolle", 					// how much of town is controlled
	"Mine", 				// mine associated with this town
	"Loyalität",					// the loyalty level of this town
	"Wichtigste Gebäude", 				// main facilities in this town
	"Zivilistentraining",				// state of civilian training in town
	"Miliz", 					// the state of the trained civilians in the town
};

// Mine strings
static const ST::string s_ger_pwMineStrings[pwMineStrings_SIZE] =
{
	"Mine",						// 0
	"Silber",
	"Gold",
	"Tagesproduktion",
	"Maximale Produktion",
	"Aufgegeben",				// 5
	"Geschlossen",
	"Fast erschöpft",
	"Produziert",
	"Status",
	"Produktionsrate",
	"Erzart",				// 10
	"Kontrolle über Stadt",
	"Loyalität der Stadt",
};

// blank sector strings
static const ST::string s_ger_pwMiscSectorStrings[pwMiscSectorStrings_SIZE] =
{
	"Feindliche Verbände",
	"Sektor",
	"# der Gegenstände",
	"Unbekannt",
	"Kontrolliert",
	"Ja",
	"Nein",
};

// error strings for inventory
static const ST::string s_ger_pMapInventoryErrorString[pMapInventoryErrorString_SIZE] =
{
	"Diesen Söldner können Sie nicht auswählen.",
	"%s ist nicht im Sektor.",
	"Während einer Schlacht müssen Sie Gegenstände manuell nehmen.",
	"Während einer Schlacht müssen Sie Gegenstände manuell fallenlassen.",
	"%s ist nicht im Sektor und kann Gegenstand nicht fallen lassen.",
};

static const ST::string s_ger_pMapInventoryStrings[pMapInventoryStrings_SIZE] =
{
	"Ort", 			// sector these items are in
	"Zahl der Gegenstände", 		// total number of items in sector
};

// movement menu text
static const ST::string s_ger_pMovementMenuStrings[pMovementMenuStrings_SIZE] =
{
	"Söldner in Sektor %s bewegen", 	// title for movement box
	"Route planen", 		// done with movement menu, start plotting movement
	"Abbruch", 	// cancel this menu
	"Andere",			// title for group of mercs not on squads nor in vehicles
};

static const ST::string s_ger_pUpdateMercStrings[pUpdateMercStrings_SIZE] =
{
	"Ups:", 			// an error has occured
	"Vertrag ist abgelaufen:", 	// this pop up came up due to a merc contract ending
	"Auftrag wurde ausgeführt:", // this pop up....due to more than one merc finishing assignments
	"Diese Söldner arbeiten wieder:", // this pop up ....due to more than one merc waking up and returing to work
	"Diese Söldner schlafen:", // this pop up ....due to more than one merc being tired and going to sleep
	"Vertrag bald abgelaufen:", 	//this pop up came up due to a merc contract ending
};

// map screen map border buttons help text
static const ST::string s_ger_pMapScreenBorderButtonHelpText[pMapScreenBorderButtonHelpText_SIZE] =
{
	"Städte zeigen (|W)",
	"|Minen zeigen",
	"|Teams & Feinde zeigen",
	"Luftr|aum zeigen",
	"Gegenstände zeigen (|I)",
	"Miliz & Feinde zeigen (|Z)",
};

static const ST::string s_ger_pMapScreenBottomFastHelp[pMapScreenBottomFastHelp_SIZE] =
{
	"|Laptop",
	"Taktik (|E|s|c)",
	"|Optionen",
	"Zeitraffer (|+)", 	// time compress more
	"Zeitraffer (|-)", 	// time compress less
	"Vorige Nachricht (|P|f|e|i|l |h|o|c|h)\nSeite zurück (|B|i|l|d |h|o|c|h)", 	// previous message in scrollable list
	"Nächste Nachricht (|P|f|e|i|l |r|u|n|t|e|r)\nNächste Seite (|B|i|l|d |r|u|n|t|e|r)", 	// next message in the scrollable list
	"Zeit Start/Stop (|L|e|e|r|t|a|s|t|e)",	// start/stop time compression
};

static const ST::string s_ger_pMapScreenBottomText = "Kontostand"; // current balance in player bank account

static const ST::string s_ger_pMercDeadString = "%s ist tot.";


static const ST::string s_ger_pDayStrings = "Tag";

// the list of email sender names
static const ST::string s_ger_pSenderNameList[pSenderNameList_SIZE] =
{
	"Enrico",
	"Psych Pro Inc.",
	"Online-Hilfe",
	"Psych Pro Inc.",
	"Speck",
	"R.I.S.",
	"Barry",
	"Blood",
	"Lynx",
	"Grizzly",
	"Vicki",
	"Trevor",
	"Grunty",
	"Ivan",
	"Steroid",
	"Igor",
	"Shadow",
	"Red",
	"Reaper",
	"Fidel",
	"Fox",
	"Sidney",
	"Gus",
	"Buns",
	"Ice",
	"Spider",
	"Cliff",
	"Bull",
	"Hitman",
	"Buzz",
	"Raider",
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
	"H, A & S Versicherung",
	"Bobby Rays",
	"Kingpin",
	"John Kulba",
	"A.I.M.",
};


// new mail notify string
static const ST::string s_ger_pNewMailStrings = "Sie haben neue Mails...";


// confirm player's intent to delete messages
static const ST::string s_ger_pDeleteMailStrings[pDeleteMailStrings_SIZE] =
{
	"Mail löschen?",
	"UNGELESENE Mail löschen?",
};

// the sort header strings
static const ST::string s_ger_pEmailHeaders[pEmailHeaders_SIZE] =
{
	"Absender:",
	"Betreff:",
	"Datum:",
};

// email titlebar text
static const ST::string s_ger_pEmailTitleText = "Mailbox";

// the financial screen strings
static const ST::string s_ger_pFinanceTitle = "Buchhalter Plus"; // the name we made up for the financial program in the game

static const ST::string s_ger_pFinanceSummary[pFinanceSummary_SIZE] =
{
	"Haben:", 				//the credits column (to ADD money to your account)
	"Soll:", 				//the debits column (to SUBTRACT money from your account)
	"Einkünfte vom Vortag:",
	"Sonstige Einzahlungen vom Vortag:",
	"Haben vom Vortag:",
	"Kontostand Ende des Tages:",
	"Tagessatz:",
	"Sonstige Einzahlungen von heute:",
	"Haben von heute:",
	"Kontostand:",
	"Voraussichtliche Einkünfte:",
	"Prognostizierter Kontostand:", 		// projected balance for player for tommorow
};

// headers to each list in financial screen
static const ST::string s_ger_pFinanceHeaders[pFinanceHeaders_SIZE] =
{
	"Tag", 					// the day column
	"Haben", 				//the credits column (to ADD money to your account)
	"Soll", 					//the debits column (to SUBTRACT money from your account)
	"Kontobewegungen", 			// transaction type - see TransactionText below
	"Kontostand", 				// balance at this point in time
	"Seite", 				// page number
	"Tag(e)", 				// the day(s) of transactions this page displays
};

static const ST::string s_ger_pTransactionText[pTransactionText_SIZE] =
{
	"Aufgelaufene Zinsen",			// interest the player has accumulated so far
	"Anonyme Einzahlung",
	"Bearbeitungsgebühr",
	"%s von AIM angeheuert", // Merc was hired
	"Kauf bei Bobby Rays",		// Bobby Ray is the name of an arms dealer
	"Ausgeglichene Konten bei M.E.R.C.",
	"Krankenversicherung für %s", 		// medical deposit for merc
	"BSE-Profilanalyse", 		// IMP is the acronym for International Mercenary Profiling
	"Versicherung für %s abgeschlossen",
	"Versicherung für %s verringert",
	"Versicherung für %s verlängert", 				// johnny contract extended
	"Versicherung für %s gekündigt",
	"Versicherungsanspruch für %s", 		// insurance claim for merc
	"%ss Vertrag verl. um 1 Tag", 				// entend mercs contract by a day
	"%ss Vertrag verl. um 1 Woche",
	"%ss Vertrag verl. um 2 Wochen",
	"Minenertrag",
	"",
	"Blumen kaufen",
	"Volle Rückzahlung für %s",
	"Teilw. Rückzahlung für %s",
	"Keine Rückzahlung für %s",
	"Zahlung an %s",		// %s is the name of the npc being paid
	"Überweisen an %s", 			// transfer funds to a merc
	"Überweisen von %s", 		// transfer funds from a merc
	"Miliz in %s ausbilden", // initial cost to equip a town's militia
	"Gegenstände von %s gekauft.",	//is used for the Shop keeper interface. The dealers name will be appended to the end of the string.
	"%s hat Geld angelegt.",
};

// helicopter pilot payment
static const ST::string s_ger_pSkyriderText[pSkyriderText_SIZE] =
{
	"Skyrider wurden $%d gezahlt", 			// skyrider was paid an amount of money
	"Skyrider bekommt noch $%d", 		// skyrider is still owed an amount of money
	"Skyrider hat keine Passagiere. Wenn Sie Söldner in den Sektor transportieren wollen, weisen Sie sie einem Fahrzeug/Helikopter zu.",
};

// strings for different levels of merc morale
static const ST::string s_ger_pMoralStrings[pMoralStrings_SIZE] =
{
	"Super",
	"Gut",
	"Stabil",
	"Schlecht",
	"Panik",
	"Mies",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.
static const ST::string s_ger_str_left_equipment   = "%ss Ausrüstung ist in %s (%s) angekommen.";

// Status that appears on the Map Screen
static const ST::string s_ger_pMapScreenStatusStrings[pMapScreenStatusStrings_SIZE] =
{
	"Gesundheit",
	"Energie",
	"Moral",
	"Zustand",	// the condition of the current vehicle (its "health")
	"Tank",	// the fuel level of the current vehicle (its "energy")
};

static const ST::string s_ger_pMapScreenPrevNextCharButtonHelpText[pMapScreenPrevNextCharButtonHelpText_SIZE] =
{
	"Voriger Söldner (|P|f|e|i|l |l|i|n|k|s)", 			// previous merc in the list
	"Nächster Söldner (|P|f|e|i|l |r|e|c|h|t|s)", 				// next merc in the list
};

static const ST::string s_ger_pEtaString = "Ank.:"; // eta is an acronym for Estimated Time of Arrival

static const ST::string s_ger_pTrashItemText[pTrashItemText_SIZE] =
{
	"Sie werden das Ding nie wiedersehen. Trotzdem wegwerfen?", 	// do you want to continue and lose the item forever
	"Dieser Gegenstand sieht SEHR wichtig aus. Sie sie GANZ SICHER, dass Sie ihn wegwerfen wollen?", // does the user REALLY want to trash this item
};

static const ST::string s_ger_pMapErrorString[pMapErrorString_SIZE] =
{
	"Trupp kann nicht reisen, wenn einer schläft.",

//1-5
	"Wir müssen erst an die Oberfläche.",
	"Marschbefehl? Wir sind in einem feindlichen Sektor!",
	"Wenn Söldner reisen sollen, müssen sie einem Trupp oder Fahrzeug zugewiesen werden.",
	"Sie haben noch keine Teammitglieder.",		// you have no members, can't do anything
	"Söldner kann nicht gehorchen.",			// merc can't comply with your order
//6-10
	"%s braucht eine Eskorte. Platzieren Sie ihn in einem Trupp mit Eskorte.", // merc can't move unescorted .. for a male
	"%s braucht eine Eskorte. Platzieren Sie sie in einem Trupp mit Eskorte.", // for a female
	"Söldner ist noch nicht in Arulco!",
	"Erst mal Vertrag aushandeln!",
	"",
//11-15
	"Marschbefehl? Hier tobt ein Kampf!",
	"Sie sind von Bloodcats umstellt in Sektor %s!",
	"Sie haben gerade eine Bloodcat-Höhle betreten in Sektor I16!",
	"",
	"Raketenstützpunkt in %s wurde erobert.",
//16-20
	"Mine in %s wurde erobert. Ihre Tageseinnahmen wurden reduziert auf %s.",
	"Gegner hat Sektor %s ohne Gegenwehr erobert.",
	"Mindestens ein Söldner konnte nicht eingeteilt werden.",
	"%s konnte sich nicht anschließen, weil %s voll ist",
	"%s konnte sich %s nicht anschließen, weil er zu weit weg ist.",
//21-25
	"Die Mine in %s ist von Deidrannas Truppen erobert worden!",
	"Deidrannas Truppen sind gerade in den Raketenstützpunkt in %s eingedrungen",
	"Deidrannas Truppen sind gerade in %s eingedrungen",
	"Deidrannas Truppen wurden gerade in %s gesichtet.",
	"Deidrannas Truppen haben gerade %s erobert.",
//26-30
	"Mindestens ein Söldner kann nicht schlafen.",
	"Mindestens ein Söldner ist noch nicht wach.",
	"Die Miliz kommt erst, wenn das Training beendet ist.",
	"%s kann im Moment keine Marschbefehle erhalten.",
	"Milizen außerhalb der Stadtgrenzen können nicht in andere Sektoren reisen.",
//31-35
	"Sie können keine Milizen in %s haben.",
	"Leere Fahrzeuge fahren nicht!",
	"%s ist nicht transportfähig!",
	"Sie müssen erst das Museum verlassen!",
	"%s ist tot!",
//36-40
	"%s kann nicht zu %s wechseln, weil der sich bewegt",
	"%s kann so nicht einsteigen",
	"%s kann sich nicht %s anschließen",
	"Sie können den Zeitraffer erst mit neuen Söldner benutzen!",
	"Dieses Fahrzeug kann nur auf Straßen fahren!",
//41-45
	"Reisenden Söldnern können Sie keine Aufträge erteilen.",
	"Kein Benzin mehr!",
	"%s ist zu müde.",
	"Keiner kann das Fahrzeug steuern.",
	"Ein oder mehrere Söldner dieses Trupps können sich jetzt nicht bewegen.",
//46-50
	"Ein oder mehrere Söldner des ANDEREN Trupps kann sich gerade nicht bewegen.",
	"Fahrzeug zu stark beschädigt!",
	"Nur zwei Söldner pro Sektor können Milizen trainieren.",
	"Roboter muss von jemandem bedient werden. Beide im selben Trupp platzieren.",

};

// help text used during strategic route plotting
static const ST::string s_ger_pMapPlotStrings[pMapPlotStrings_SIZE] =
{
	"Klicken Sie noch einmal auf das Ziel, um die Route zu bestätigen. Klicken Sie auf andere Sektoren, um die Route zu ändern.",
	"Route bestätigt.",
	"Ziel unverändert.",
	"Route geändert.",
	"Route verkürzt.",
};

// help text used when moving the merc arrival sector
static const ST::string s_ger_pBullseyeStrings[pBullseyeStrings_SIZE] =
{
	"Klicken Sie auf den Sektor, in dem die Söldner statt dessen ankommen sollen.",
	"OK. Söldner werden in %s abgesetzt",
	"Söldner können nicht dorthin fliegen. Luftraum nicht gesichert!",
	"Abbruch. Ankunftssektor unverändert,",
	"Luftraum über %s ist nicht mehr sicher! Ankunftssektor jetzt in %s.",
};

// help text for mouse regions
static const ST::string s_ger_pMiscMapScreenMouseRegionHelpText[pMiscMapScreenMouseRegionHelpText_SIZE] =
{
	"Ins Inventar gehen (|E|i|n|g|a|b|e)",
	"Gegenstand wegwerfen",
	"Inventar verlassen (|E|i|n|g|a|b|e)",
};

static const ST::string s_ger_str_he_leaves_where_drop_equipment  = "Soll %s seine Ausrüstung hier lassen (%s) oder in %s (%s), bevor er Arulco verlässt?";
static const ST::string s_ger_str_she_leaves_where_drop_equipment = "Soll %s ihre Ausrüstung hier lassen (%s) oder in %s (%s), bevor sie Arulco verlässt?";
static const ST::string s_ger_str_he_leaves_drops_equipment       = "%s geht bald und lässt seine Ausrüstung in %s.";
static const ST::string s_ger_str_she_leaves_drops_equipment      = "%s geht bald und lässt ihre Ausrüstung in %s.";


// Text used on IMP Web Pages
static const ST::string s_ger_pImpPopUpStrings[pImpPopUpStrings_SIZE] =
{
	"Ungültiger Code",
	"Sie wollen gerade den ganzen Evaluierungsprozess von vorn beginnen. Sind Sie sicher?",
	"Bitte Ihren vollen Namen und Ihr Geschlecht eingeben",
	"Die Überprüfung Ihrer finanziellen Mittel hat ergeben, dass Sie sich keine Evaluierung leisten können.",
	"Option zur Zeit nicht gültig.",
	"Um eine genaue Evaluierung durchzuführen, müssen Sie mindestens noch ein Teammitglied aufnehmen können.",
	"Evaluierung bereits durchgeführt.",
};

// button labels used on the IMP site
static const ST::string s_ger_pImpButtonText[pImpButtonText_SIZE] =
{
	"Wir über uns", 			// about the IMP site
	"BEGINNEN", 			// begin profiling
	"Persönlichkeit", 		// personality section
	"Eigenschaften", 		// personal stats/attributes section
	"Porträt", 			// the personal portrait selection
	"Stimme %d", 			// the voice selection
	"Fertig", 			// done profiling
	"Von vorne anfangen",		// start over profiling
	"Ja, die Antwort passt!",
	"Ja",
	"Nein",
	"Fertig", 			// finished answering questions
	"Zurück", 			// previous question..abbreviated form
	"Weiter", 			// next question
	"JA",				// yes, I am certain
	"NEIN, ICH MÖCHTE VON VORNE ANFANGEN.", // no, I want to start over the profiling process
	"JA",
	"NEIN",
	"Zurück", 			// back one page
	"Abbruch", 			// cancel selection
	"Ja",
	"Nein, ich möchte es mir nochmal ansehen.",
	"Registrieren", 			// the IMP site registry..when name and gender is selected
	"Analyse wird durchgeführt",	// analyzing your profile results
	"OK",
	"Stimme",
	"Spezialitäten",			// "Specialties" - the skills selection screen
};

static const ST::string s_ger_pExtraIMPStrings[pExtraIMPStrings_SIZE] =
{
	"Um mit der Evaluierung zu beginnen, Persönlichkeit auswählen.",
	"Da Sie nun mit der Persönlichkeit fertig sind, wählen Sie Ihre Eigenschaften aus.",
	"Nach Festlegung der Eigenschaften können Sie nun mit der Porträtauswahl fortfahren.",
	"Wählen Sie abschließend die Stimmprobe aus, die Ihrer eigenen Stimme am nächsten kommt.",
};

static const ST::string s_ger_gzIMPSkillTraitsText[gzIMPSkillTraitsText_SIZE] =
{
	s_ger_gzMercSkillText[1],
	"Mann gegen Mann",
	s_ger_gzMercSkillText[3],
	L"Nacht-Operationen",
	s_ger_gzMercSkillText[5],
	L"Unterrichten",
	s_ger_gzMercSkillText[7],
	s_ger_gzMercSkillText[8],
	s_ger_gzMercSkillText[9],
	L"Beidhändigkeit",
	L"Messer",
	L"Dachtreffer-Bonus",
	L"Tarnkunst",
	L"Martial Arts",

	L"Kein",
	L"B.S.E. Spezialitäten",
};

static const ST::string s_ger_pFilesTitle = "Akten einsehen";

static const ST::string s_ger_pFilesSenderList[pFilesSenderList_SIZE] =
{
	"Aufklärungsbericht",	// the recon report sent to the player. Recon is an abbreviation for reconissance
	"Intercept #1",	// first intercept file .. Intercept is the title of the organization sending the file...similar in function to INTERPOL/CIA/KGB..refer to fist record in files.txt for the translated title
	"Intercept #2",	// second intercept file
	"Intercept #3",	// third intercept file
	"Intercept #4",	// fourth intercept file
	"Intercept #5",	// fifth intercept file
	"Intercept #6",	// sixth intercept file
};

// Text having to do with the History Log
static const ST::string s_ger_pHistoryTitle = "Logbuch";

static const ST::string s_ger_pHistoryHeaders[pHistoryHeaders_SIZE] =
{
	"Tag",		// the day the history event occurred
	"Seite",	// the current page in the history report we are in
	"Tag",		// the days the history report occurs over
	"Ort",		// location (in sector) the event occurred
	"Ereignis",	// the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
static const ST::string s_ger_pHistoryStrings[pHistoryStrings_SIZE] =
{
	"",																						// leave this line blank
	//1-5
	"%s von A.I.M. angeheuert.", 										// merc was hired from the aim site
	"%s von M.E.R.C. angeheuert.", 									// merc was hired from the aim site
	"%s ist tot.", 															// merc was killed
	"Rechnung an M.E.R.C. bezahlt",								// paid outstanding bills at MERC
	"Enrico Chivaldoris Auftrag akzeptiert",
	//6-10
	"BSE-Profil erstellt",
	"Versicherung abgeschlossen für %s.", 				// insurance contract purchased
	"Versicherung gekündigt für %s.", 				// insurance contract canceled
	"Versicherung ausgezahlt für %s.", 							// insurance claim payout for merc
	"%ss Vertrag um 1 Tag verlängert.", 						// Extented "mercs name"'s for a day
	//11-15
	"%ss Vertrag um 1 Woche verlängert.", 					// Extented "mercs name"'s for a week
	"%ss Vertrag um 2 Wochen verlängert.", 					// Extented "mercs name"'s 2 weeks
	"%s entlassen.", 													// "merc's name" was dismissed.
	"%s geht.", 																		// "merc's name" quit.
	"Quest begonnen.", 															// a particular quest started
	//16-20
	"Quest gelöst.",
	"Mit Vorarbeiter in %s geredet",									// talked to head miner of town
	"%s befreit",
	"Cheat benutzt",
	"Essen ist morgen in Omerta",
	//21-25
	"%s heiratet Daryl Hick",
	"%ss Vertrag abgelaufen.",
	"%s rekrutiert.",
	"Enrico sieht kaum Fortschritte",
	"Schlacht gewonnen",
	//26-30
	"Mine in %s produziert weniger",
	"Mine in %s leer",
	"Mine in %s geschlossen",
	"Mine in %s wieder offen",
	"Etwas über Gefängnis in Tixa erfahren.",
	//31-35
	"Von Waffenfabrik in Orta gehört.",
	"Forscher in Orta gab uns viele Raketengewehre.",
	"Deidranna verfüttert Leichen.",
	"Frank erzählte von Kämpfen in San Mona.",
	"Patient denkt, er hat in den Minen etwas gesehen.",
	//36-40
	"Devin getroffen - verkauft Sprengstoff",
	"Berühmten Ex-AIM-Mann Mike getroffen!",
	"Tony getroffen - verkauft Waffen.",
	"Sergeant Krott gab mir Raketengewehr.",
	"Kyle die Urkunde für Angels Laden gegeben.",
	//41-45
	"Madlab will Roboter bauen.",
	"Gabby kann Tinktur gegen Käfer machen.",
	"Keith nicht mehr im Geschäft.",
	"Howard lieferte Gift an Deidranna.",
	"Keith getroffen - verkauft alles in Cambria.",
	//46-50
	"Howard getroffen - Apotheker in Balime",
	"Perko getroffen - hat kleinen Reparaturladen.",
	"Sam aus Balime getroffen - hat Computerladen.",
	"Franz hat Elektronik und andere Sachen.",
	"Arnold repariert Sachen in Grumm.",
	//51-55
	"Fredo repariert Elektronik in Grumm.",
	"Spende von Reichem aus Balime bekommen.",
	"Schrotthändler Jake getroffen.",
	"Ein Depp hat uns eine Codekarte gegeben.",
	"Walter bestochen, damit er Keller öffnet.",
	//56-60
	"Wenn Dave Sprit hat, bekommen wir's gratis.",
	"Pablo bestochen.",
	"Kingpin hat Geld in San Mona-Mine.",
	"%s gewinnt Extremkampf",
	"%s verliert Extremkampf",
	//61-65
	"%s beim Extremkampf disqualifiziert",
	"Viel Geld in verlassener Mine gefunden.",
	"Von Kingpin geschickten Mörder getroffen",
	"Kontrolle über Sektor verloren",
	"Sektor verteidigt",
	//66-70
	"Schlacht verloren",							//ENEMY_ENCOUNTER_CODE
	"Tödlicher Hinterhalt",						//ENEMY_AMBUSH_CODE
	"Hinterhalt ausgehoben",
	"Angriff fehlgeschlagen",			//ENTERING_ENEMY_SECTOR_CODE
	"Angriff erfolgreich",
	//71-75
	"Monster angegriffen",			//CREATURE_ATTACK_CODE
	"Von Bloodcats getötet",			//BLOODCAT_AMBUSH_CODE
	"Bloodcats getötet",
	"%s wurde getötet",
	"Carmen den Kopf eines Terroristen gegeben",
	"Slay ist gegangen", //Slay is a merc and has left the team
	"%s gekillt",		//History log for when a merc kills an NPC or PC
};

static const ST::string s_ger_pHistoryLocations = "n.a"; // N/A is an acronym for Not Applicable

// icon text strings that appear on the laptop
static const ST::string s_ger_pLaptopIcons[pLaptopIcons_SIZE] =
{
	"E-mail",
	"Web",
	"Finanzen",
	"Personal",
	"Logbuch",
	"Dateien",
	"Schließen",
	"sir-FER 4.0",			// our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added
static const ST::string s_ger_pBookMarkStrings[pBookMarkStrings_SIZE] =
{
	"A.I.M.",
	"Bobby Rays",
	"B.S.E",
	"M.E.R.C.",
	"Bestattungsinst.",
	"Florist",
	"Versicherung",
	"Abbruch",
};

// When loading or download a web page
static const ST::string s_ger_pDownloadString[pDownloadString_SIZE] = {
	"Download läuft",
	"Neuladen läuft",
};

//This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

static const ST::string s_ger_gsAtmStartButtonText[gsAtmStartButtonText_SIZE] =
{
	"Statistik", 			// view stats of the merc
	"Inventar", 			// view the inventory of the merc
	"Anstellung",
};

// Web error messages. Please use German equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator
static const ST::string s_ger_pErrorStrings = "Verbindung zum Host wird dauernd unterbrochen. Mit längeren Übertragungszeiten ist zu rechnen.";

static const ST::string s_ger_pPersonnelString = "Söldner:"; // mercs we have

static const ST::string s_ger_pWebTitle = "sir-FER 4.0"; // our name for the version of the browser, play on company name

// The titles for the web program title bar, for each page loaded
static const ST::string s_ger_pWebPagesTitles[pWebPagesTitles_SIZE] =
{
	"A.I.M.",
	"A.I.M. Mitglieder",
	"A.I.M. Bilder",		// a mug shot is another name for a portrait
	"A.I.M. Sortierfunktion",
	"A.I.M.",
	"A.I.M. Veteranen",
	"A.I.M. Politik",
	"A.I.M. Geschichte",
	"A.I.M. Links",
	"M.E.R.C.",
	"M.E.R.C. Konten",
	"M.E.R.C. Registrierung",
	"M.E.R.C. Index",
	"Bobby Rays",
	"Bobby Rays - Waffen",
	"Bobby Rays - Munition",
	"Bobby Rays - Rüstungen",
	"Bobby Rays - Sonstiges",							//misc is an abbreviation for miscellaneous
	"Bobby Rays - Gebraucht",
	"Bobby Rays - Versandauftrag",
	"B.S.E",
	"B.S.E",
	"Fleuropa",
	"Fleuropa - Gestecke",
	"Fleuropa - Bestellformular",
	"Fleuropa - Karten",
	"Hammer, Amboss & Steigbügel Versicherungsmakler",
	"Information",
	"Vertrag",
	"Bemerkungen",
	"McGillicuttys Bestattungen",
	"URL nicht gefunden.",
	"Bobby Rays - Letzte Lieferungen",
	"",
	"",
};

static const ST::string s_ger_pShowBookmarkString[pShowBookmarkString_SIZE] =
{
	"Sir-Help",
	"Erneut auf Web klicken für Lesezeichen.",
};

static const ST::string s_ger_pLaptopTitles[pLaptopTitles_SIZE] =
{
	"E-Mail",
	"Dateien",
	"Söldner-Manager",
	"Buchhalter Plus",
	"Logbuch",
};

static const ST::string s_ger_pPersonnelDepartedStateStrings[pPersonnelDepartedStateStrings_SIZE] =
{ //(careful not to exceed 18 characters total including spaces)
	//reasons why a merc has left.
	"Getötet",
	"Entlassen",
	"Heirat",
	"Vertrag zu Ende",
	"Quit",
};

// personnel strings appearing in the Personnel Manager on the laptop
static const ST::string s_ger_pPersonelTeamStrings[pPersonelTeamStrings_SIZE] =
{
	"Aktuelles Team",
	"Ausgeschieden",
	"Tägliche Kosten:",
	"Höchste Kosten:",
	"Niedrigste Kosten:",
	"Im Kampf getötet:",
	"Entlassen:",
	"Sonstiges:",
};

static const ST::string s_ger_pPersonnelCurrentTeamStatsStrings[pPersonnelCurrentTeamStatsStrings_SIZE] =
{
	"Schlechteste",
	"Durchschn.",
	"Beste",
};

static const ST::string s_ger_pPersonnelTeamStatsStrings[pPersonnelTeamStatsStrings_SIZE] =
{
	"GSND",
	"BEW",
	"GES",
	"KRF",
	"FHR",
	"WSH",
	"ERF",
	"TRF",
	"TEC",
	"SPR",
	"MED",
};

// horizontal and vertical indices on the map screen
static const ST::string s_ger_pMapVertIndex[pMapVertIndex_SIZE] =
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

static const ST::string s_ger_pMapHortIndex[pMapHortIndex_SIZE] =
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

static const ST::string s_ger_pMapDepthIndex[pMapDepthIndex_SIZE] =
{
	"",
	"-1",
	"-2",
	"-3",
};

// text that appears on the contract button
static const ST::string s_ger_pContractButtonString = "Vertrag";

// text that appears on the update panel buttons
static const ST::string s_ger_pUpdatePanelButtons[pUpdatePanelButtons_SIZE] =
{
	"Weiter",
	"Stop",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle
static ST::string s_ger_LargeTacticalStr[LargeTacticalStr_SIZE] =
{
	"Sie sind in diesem Sektor geschlagen worden!",
	"Der Feind hat kein Erbarmen mit den Seelen Ihrer Teammitglieder und verschlingt jeden einzelnen.",
	"Ihre bewusstlosen Teammitglieder wurden gefangen genommen!",
	"Ihre Teammitglieder wurden vom Feind gefangen genommen.",
};

//Insurance Contract.c
//The text on the buttons at the bottom of the screen.
static const ST::string s_ger_InsContractText[InsContractText_SIZE] =
{
	"Zurück",
	"Vor",
	"OK",
	"Löschen",
};

//Insurance Info
// Text on the buttons on the bottom of the screen
static const ST::string s_ger_InsInfoText[InsInfoText_SIZE] =
{
	"Zurück",
	"Vor",
};

//For use at the M.E.R.C. web site. Text relating to the player's account with MERC
static const ST::string s_ger_MercAccountText[MercAccountText_SIZE] =
{
	// Text on the buttons on the bottom of the screen
	"Befugnis erteilen",
	"Home",
	"Konto #:",
	"Söldner",
	"Tage",
	"Rate",		//5
	"Belasten",
	"Gesamt:",
	"Zahlung von %s wirklich genehmigen?",		//the %s is a string that contains the dollar amount ( ex. "$150" )
};

//For use at the M.E.R.C. web site. Text relating a MERC mercenary
static const ST::string s_ger_MercInfo[MercInfo_SIZE] =
{
	"Zurück",
	"Anheuern",
	"Weiter",
	"Zusatzinfo",
	"Home",
	"Angestellt",
	"Sold:",
	"Pro Tag",
	"Verstorben",

	"Sie versuchen, zu viele Söldner anzuheuern. 18 ist Maximum.",
	"nicht da",
};

// For use at the M.E.R.C. web site. Text relating to opening an account with MERC
static const ST::string s_ger_MercNoAccountText[MercNoAccountText_SIZE] =
{
	//Text on the buttons at the bottom of the screen
	"Konto eröffnen",
	"Rückgängig",
	"Sie haben kein Konto. Möchten Sie eins eröffnen?",
};

// For use at the M.E.R.C. web site. MERC Homepage
static const ST::string s_ger_MercHomePageText[MercHomePageText_SIZE] =
{
	//Description of various parts on the MERC page
	"Speck T. Kline, Gründer und Besitzer",
	"Hier klicken, um ein Konto zu eröffnen",
	"Hier klicken, um das Konto einzusehen",
	"Hier klicken, um Dateien einzusehen.",
	// The version number on the video conferencing system that pops up when Speck is talking
	"Speck Com v3.2",
};

// For use at MiGillicutty's Web Page.
static const ST::string s_ger_sFuneralString[sFuneralString_SIZE] =
{
	"McGillicuttys Bestattungen: Wir trösten trauernde Familien seit 1983.",
	"Der Bestattungsunternehmer und frühere A.I.M.-Söldner Murray \"Pops\" McGillicutty ist ein ebenso versierter wie erfahrener Bestatter.",
	"Pops hat sein ganzes Leben mit Todes- und Trauerfällen verbracht. Deshalb weiß er aus erster Hand, wie schwierig das sein kann.",
	"Das Bestattungsunternehmen McGillicutty bietet Ihnen einen umfassenden Service, angefangen bei der Schulter zum Ausweinen bis hin zur kosmetischen Aufbereitung entstellter Körperteile.",
	"McGillicuttys Bestattungen - und Ihre Lieben ruhen in Frieden.",

	// Text for the various links available at the bottom of the page
	"BLUMEN",
	"SÄRGE UND URNEN",
	"FEUERBEST.",
	"GRÄBER",
	"PIETÄT",

	// The text that comes up when you click on any of the links ( except for send flowers ).
	"Leider ist diese Site aufgrund eines Todesfalles in der Familie noch nicht fertiggestellt. Sobald das Testament eröffnet worden und die Verteilung des Erbes geklärt ist, wird diese Site fertiggestellt.",
	"Unser Mitgefühl gilt trotzdem all jenen, die es diesmal versucht haben. Bis später.",
};

// Text for the florist Home page
static const ST::string s_ger_sFloristText[sFloristText_SIZE] =
{
	//Text on the button on the bottom of the page

	"Galerie",

	//Address of United Florist

	"\"Wir werfen überall per Fallschirm ab\"",
	"1-555-SCHNUPPER-MAL",
	"333 Duftmarke Dr, Aroma City, CA USA 90210",
	"http://www.schnupper-mal.com",

	// detail of the florist page

	"Wir arbeiten schnell und effizient",
	"Lieferung am darauf folgenden Tag, in fast jedes Land der Welt. Ausnahmen sind möglich.",
	"Wir haben die garantiert niedrigsten Preise weltweit!",
	"Wenn Sie anderswo einen niedrigeren Preis für irgend ein Arrangement sehen, bekommen Sie von uns ein Dutzend Rosen umsonst!",
	"Fliegende Flora, Fauna & Blumen seit 1981.",
	"Unsere hochdekorierten Ex-Bomber-Piloten werfen das Bouquet in einem Radius von zehn Meilen rund um den Bestimmungsort ab. Jederzeit!",
	"Mit uns werden Ihre blumigsten Fantasien wahr",
	"Bruce, unser weltberühmter Designer-Florist, verwendet nur die frischesten handverlesenen Blumen aus unserem eigenen Gewächshaus.",
	"Und denken Sie daran: Was wir nicht haben, pflanzen wir für Sie - und zwar schnell!",
};

//Florist OrderForm
static const ST::string s_ger_sOrderFormText[sOrderFormText_SIZE] =
{

	//Text on the buttons

	"Zurück",
	"Senden",
	"Löschen",
	"Galerie",

	"Name des Gestecks:",
	"Preis:",			//5
	"Bestellnr.:",
	"Liefertermin",
	"Morgen",
	"Egal",
	"Bestimmungsort",			//10
	"Extraservice",
	"Kaputtes Gesteck($10)",
	"Schwarze Rosen($20)",
	"Welkes Gesteck($10)",
	"Früchtekuchen (falls vorrätig)($10)",		//15
	"Persönliche Worte:",
	"Aufgrund der Kartengröße darf Ihre Botschaft nicht länger sein als 75 Zeichen.",
	"...oder wählen Sie eine unserer",

	"STANDARD-KARTEN",
	"Rechnung für",//20

	//The text that goes beside the area where the user can enter their name

	"Name:",
};

//Florist Gallery.c
static const ST::string s_ger_sFloristGalleryText[sFloristGalleryText_SIZE] =
{
	//text on the buttons
	"Zurück",	//abbreviation for previous
	"Weiter",	//abbreviation for next
	"Klicken Sie auf das Gesteck Ihrer Wahl",
	"Bitte beachten Sie, dass wir für jedes kaputte oder verwelkte Gesteck einen Aufpreis von $10 berechnen.",
	"Home",
};

static const ST::string s_ger_sFloristCards[sFloristCards_SIZE] =
{
	"Klicken Sie auf das Gesteck Ihrer Wahl",
	"Zurück",
};

// Text for Bobby Ray's Mail Order Site
static const ST::string s_ger_BobbyROrderFormText[BobbyROrderFormText_SIZE] =
{
	"Bestellformular",				//Title of the page
	"St.",					// The number of items ordered
	"Gew. (%s)",			// The weight of the item
	"Artikel",				// The name of the item
	"Preis",				// the item's weight
	"Summe",				//5	// The total price of all of items of the same type
	"Zwischensumme",				// The sub total of all the item totals added
	"Versandkosten (vgl. Bestimmungsort)",		// S&H is an acronym for Shipping and Handling
	"Endbetrag",			// The grand total of all item totals + the shipping and handling
	"Bestimmungsort",
	"Liefergeschwindigkeit",			//10	// See below
	"$ (pro %s)",			// The cost to ship the items
	"Übernacht Express",			// Gets deliverd the next day
	"2 Arbeitstage",			// Gets delivered in 2 days
	"Standard-Service",			// Gets delivered in 3 days
	"Löschen",//15			// Clears the order page
	"Bestellen",			// Accept the order
	"Zurück",				// text on the button that returns to the previous page
	"Home",				// Text on the button that returns to the home page
	"* Gebrauchter Gegenstand",		// Disclaimer stating that the item is used
	"Sie haben nicht genug Geld.",		//20	// A popup message that to warn of not enough money
	"<KEINER>",				// Gets displayed when there is no valid city selected
	"Wollen Sie Ihre Bestellung wirklich nach %s schicken?",		// A popup that asks if the city selected is the correct one
	"Packungsgewicht**",			// Displays the weight of the package
	"** Mindestgewicht",				// Disclaimer states that there is a minimum weight for the package
	"Lieferungen",
};

// This text is used when on the various Bobby Ray Web site pages that sell items
static const ST::string s_ger_BobbyRText[BobbyRText_SIZE] =
{
	"Bestellen",				// Title
	"Klicken Sie auf den gewünschten Gegenstand. Weiteres Klicken erhöht die Stückzahl. Rechte Maustaste verringert Stückzahl. Wenn Sie fertig sind, weiter mit dem Bestellformular.",			// instructions on how to order

	//Text on the buttons to go the various links

	"Zurück",		//
	"Feuerwfn.", 			//3
	"Munition",			//4
	"Rüstung",			//5
	"Sonstiges",			//6	//misc is an abbreviation for miscellaneous
	"Gebraucht",			//7
	"Vor",
	"BESTELLEN",
	"Home",			//10

	//The following lines provide information on the items

	"Gewicht:",			// Weight of all the items of the same type
	"Kal:",			// the caliber of the gun
	"Mag:",			// number of rounds of ammo the Magazine can hold
	"Reichw.:",				// The range of the gun
	"Schaden:",				// Damage of the weapon
	"Freq.:",			// Weapon's Rate Of Fire, acroymn ROF
	"Preis:",			// Cost of the item
	"Vorrätig:",			// The number of items still in the store's inventory
	"Bestellt:",		// The number of items on order
	"Beschädigt",			// If the item is damaged
	"Summe:",			// The total cost of all items on order
	"* % funktionstüchtig",		// if the item is damaged, displays the percent function of the item

	//Popup that tells the player that they can only order 10 items at a time
	"Mist! Mit diesem Formular können Sie nur 10 Sachen bestellen. Wenn Sie mehr wollen (was wir sehr hoffen), füllen Sie bitte noch ein Formular aus.",

	// A popup that tells the user that they are trying to order more items then the store has in stock

	"Sorry. Davon haben wir leider im Moment nichts mehr auf Lager. Versuchen Sie es später noch einmal.",

	//A popup that tells the user that the store is temporarily sold out

	"Es tut uns sehr leid, aber im Moment sind diese Sachen total ausverkauft.",
};

// The following line is used on the Ammunition page.  It is used for help text
// to display how many items the player's merc has that can use this type of
// ammo.
static const ST::string s_ger_str_bobbyr_guns_num_guns_that_use_ammo = "Ihr Team hat %d Waffe(n), die dieses Kaliber benutzen";

// Text for Bobby Ray's Home Page
static const ST::string s_ger_BobbyRaysFrontText[BobbyRaysFrontText_SIZE] =
{
	//Details on the web site

	"Dies ist die heißeste Site für Waffen und militärische Ausrüstung aller Art",
	"Welchen Sprengstoff Sie auch immer brauchen - wir haben ihn.",
	"SECOND HAND",

	//Text for the various links to the sub pages

	"SONSTIGES",
	"FEUERWAFFEN",
	"MUNITION",		//5
	"RÜSTUNG",

	//Details on the web site

	"Was wir nicht haben, das hat auch kein anderer",
	"In Arbeit",
};

// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug shot page
static const ST::string s_ger_AimSortText[AimSortText_SIZE] =
{
	"A.I.M. Mitglieder",				// Title
	"Sortieren:",					// Title for the way to sort

	//Text of the links to other AIM pages

	"Den Söldner-Kurzindex ansehen",
	"Personalakte der Söldner ansehen",
	"Die AIM-Veteranengalerie ansehen"
};

// text to display how the entries will be sorted
static const ST::string s_ger_str_aim_sort_price        = "Preis";
static const ST::string s_ger_str_aim_sort_experience   = "Erfahrung";
static const ST::string s_ger_str_aim_sort_marksmanship = "Treffsicherheit";
static const ST::string s_ger_str_aim_sort_medical      = "Medizin";
static const ST::string s_ger_str_aim_sort_explosives   = "Sprengstoff";
static const ST::string s_ger_str_aim_sort_mechanical   = "Technik";
static const ST::string s_ger_str_aim_sort_ascending    = "Aufsteigend";
static const ST::string s_ger_str_aim_sort_descending   = "Absteigend";

//Aim Policies.c
//The page in which the AIM policies and regulations are displayed
static const ST::string s_ger_AimPolicyText[AimPolicyText_SIZE] =
{
	// The text on the buttons at the bottom of the page

	"Zurück",
	"AIM HomePage",
	"Regel-Index",
	"Nächste Seite",
	"Ablehnen",
	"Zustimmen",
};

//Aim Member.c
//The page in which the players hires AIM mercenaries
// Instructions to the user to either start video conferencing with the merc, or to go the mug shot index
static const ST::string s_ger_AimMemberText[AimMemberText_SIZE] =
{
	"Linksklick",
	"zum Kontaktieren.",
	"Rechtsklick",
	"zum Foto-Index.",
//	"Linksklick zum Kontaktieren. \nRechtsklick zum Foto-Index.",
};

//Aim Member.c
//The page in which the players hires AIM mercenaries
static const ST::string s_ger_CharacterInfo[CharacterInfo_SIZE] =
{
	// the contract expenses' area

	"Preis",
	"Vertrag",
	"1 Tag",
	"1 Woche",
	"2 Wochen",

	// text for the buttons that either go to the previous merc,
	// start talking to the merc, or go to the next merc

	"Zurück",
	"Kontakt",
	"Weiter",
	"Zusatzinfo",				// Title for the additional info for the merc's bio
	"Aktive Mitglieder", // Title of the page
	"Zusätzliche Ausrüstung:",				// Displays the optional gear cost
	"VERSICHERUNG erforderlich",			// If the merc required a medical deposit, this is displayed
};

//Aim Member.c
//The page in which the player's hires AIM mercenaries
//The following text is used with the video conference popup
static const ST::string s_ger_VideoConfercingText[VideoConfercingText_SIZE] =
{
	"Vertragskosten:",				//Title beside the cost of hiring the merc

	//Text on the buttons to select the length of time the merc can be hired

	"1 Tag",
	"1 Woche",
	"2 Wochen",

	//Text on the buttons to determine if you want the merc to come with the equipment

	"Keine Ausrüstung",
	"Ausrüstung kaufen",

	// Text on the Buttons

	"GELD ÜBERWEISEN",			// to actually hire the merc
	"ABBRECHEN",				// go back to the previous menu
	"ANHEUERN",				// go to menu in which you can hire the merc
	"AUFLEGEN",				// stops talking with the merc
	"OK",
	"NACHRICHT",			// if the merc is not there, you can leave a message

	//Text on the top of the video conference popup

	"Videokonferenz mit",
	"Verbinde. . .",

	"versichert",			// Displays if you are hiring the merc with the medical deposit

};

//Aim Member.c
//The page in which the player hires AIM mercenaries
// The text that pops up when you select the TRANSFER FUNDS button
static const ST::string s_ger_AimPopUpText[AimPopUpText_SIZE] =
{
	"ELEKTRONISCHE ÜBERWEISUNG AUSGEFÜHRT",	// You hired the merc
	"ÜBERWEISUNG KANN NICHT BEARBEITET WERDEN",		// Player doesn't have enough money, message 1
	"NICHT GENUG GELD",				// Player doesn't have enough money, message 2

	// if the merc is not available, one of the following is displayed over the merc's face

	"Im Einsatz",
	"Bitte Nachricht hinterlassen",
	"Verstorben",

	//If you try to hire more mercs than game can support

	"Sie haben bereits 18 Söldner in Ihrem Team.",

	"Mailbox",
	"Nachricht aufgenommen",
};

//AIM Link.c
static const ST::string s_ger_AimLinkText = "A.I.M. Links"; // The title of the AIM links page

//Aim History
// This page displays the history of AIM
static const ST::string s_ger_AimHistoryText[AimHistoryText_SIZE] =
{
	"Die Geschichte von A.I.M.",					//Title

	// Text on the buttons at the bottom of the page

	"Zurück",
	"Home",
	"Veteranen",
	"Weiter",
};

//Aim Mug Shot Index
//The page in which all the AIM members' portraits are displayed in the order selected by the AIM sort page.
static const ST::string s_ger_AimFiText[AimFiText_SIZE] =
{
	// displays the way in which the mercs were sorted

	"Preis",
	"Erfahrung",
	"Treffsicherheit",
	"Medizin",
	"Sprengstoff",
	"Technik",

	// The title of the page, the above text gets added at the end of this text
	"A.I.M.-Mitglieder ansteigend sortiert nach %s",
	"A.I.M. Mitglieder absteigend sortiert nach %s",

	// Instructions to the players on what to do

	"Linke Maustaste",
	"um Söldner auszuwählen",			//10
	"Rechte Maustaste",
	"um Optionen einzustellen",

	// Gets displayed on top of the merc's portrait if they are...

	//Please be careful not to increase the size of strings for following three
	"Verstorben",						//14
	"Im Dienst",
};

//AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer with AIM
static const ST::string s_ger_AimAlumniText[AimAlumniText_SIZE] =
{
	// Text of the buttons
	"SEITE 1",
	"SEITE 2",
	"SEITE 3",
	"A.I.M.-Veteranen",	// Title of the page
	"ENDE",		// Stops displaying information on selected merc
};

//AIM Home Page
static const ST::string s_ger_AimScreenText[AimScreenText_SIZE] =
{
	// AIM disclaimers

	"A.I.M. und das A.I.M.-Logo sind in den meisten Ländern eingetragene Warenzeichen.",
	"Also denken Sie nicht mal daran, uns nachzumachen.",
	"Copyright 1998-1999 A.I.M., Ltd. Alle Rechte vorbehalten.",

	//Text for an advertisement that gets displayed on the AIM page

	"Fleuropa",
	"\"Wir werfen überall per Fallschirm ab\"",				//10
	"Treffen Sie gleich zu Anfang",
	"... die richtige Wahl.",
	"Was wir nicht haben, das brauchen Sie auch nicht.",
};

//Aim Home Page
static const ST::string s_ger_AimBottomMenuText[AimBottomMenuText_SIZE] =
{
	//Text for the links at the bottom of all AIM pages

	"Home",
	"Mitglieder",
	"Veteranen",
	"Regeln",
	"Geschichte",
	"Links",
};

//ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.
static const ST::string s_ger_SKI_Text[SKI_SIZE ] =
{
	"WAREN VORRÄTIG",	//Header for the merchandise available
	"SEITE",						//The current store inventory page being displayed
	"KOSTEN",	//The total cost of the the items in the Dealer inventory area
	"WERT",		//The total value of items player wishes to sell
	"SCHÄTZUNG",				//Button text for dealer to evaluate items the player wants to sell
	"TRANSAKTION",			//Button text which completes the deal. Makes the transaction.
	"FERTIG",						//Text for the button which will leave the shopkeeper interface.
	"KOSTEN",		//The amount the dealer will charge to repair the merc's goods
	"1 STUNDE",			// SINGULAR! The text underneath the inventory slot when an item is given to the dealer to be repaired
	"%d STUNDEN",		// PLURAL! The text underneath the inventory slot when an item is given to the dealer to be repaired
	"REPARIERT",		// Text appearing over an item that has just been repaired by a NPC repairman dealer
	"Es ist kein Platz mehr, um Sachen anzubieten.",	//Message box that tells the user there is no more room to put there stuff
	"%d MINUTEN",		// The text underneath the inventory slot when an item is given to the dealer to be repaired
	"Gegenstand fallenlassen.",
};

static const ST::string s_ger_SkiMessageBoxText[SkiMessageBoxText_SIZE] =
{
	"Möchten Sie %s von Ihrem Konto abbuchen, um die Differenz zu begleichen?",
	"Nicht genug Geld. Ihnen fehlen %s",
	"Möchten Sie %s von Ihrem Konto abbuchen, um die Kosten zu decken?",
	"Händler bitten, mit der Überweisung zu beginnen.",
	"Händler bitten, Gegenstände zu reparieren",
	"Unterhaltung beenden",
	"Kontostand",
};

//OptionScreen.c
static const ST::string s_ger_zOptionsText[zOptionsText_SIZE] =
{
	//button Text
	"Spiel sichern",
	"Spiel laden",
	"Spiel beenden",
	"Fertig",
	//Text above the slider bars
	"Effekte",
	"Sprache",
	"Musik",
	//Confirmation pop when the user selects..
	"Spiel verlassen und zurück zum Hauptmenü?",
	"Sprachoption oder Untertitel müssen aktiviert sein.",
};

//SaveLoadScreen
static const ST::string s_ger_zSaveLoadText[zSaveLoadText_SIZE] =
{
	"Spiel sichern",
	"Spiel laden",
	"Abbrechen",
	"Auswahl speichern",
	"Auswahl laden",

	"Spiel erfolgreich gespeichert",
	"Fehler beim Speichern des Spiels!",
	"Spiel erfolgreich geladen",
	"Fehler beim Laden des Spiels: \"%s\"",

	"Der Version des Spielstands unterscheidet sich vom aktuellen Spielstand. Es kann wahrscheinlich nichts passieren.",
	"Spielstand mit Namen \"%s\" wirklich löschen?",

	"Achtung:",
	"Versuche, älteren Spielstand zu laden. Das Laden wird den Spielstand automatisch aktualisieren.",
	"Der Spielstand wurde mit anderen Mods geschrieben as aktuell aktiviert sind.",
	"Möchten Sie fortfahren?",

	"Spielstand mit Namen \"%s\" wirklich überschreiben?",

	"Speichere...",	//While the game is saving this message appears.

	"Normale Waffen",
	"Zusatzwaffen",
	"Real-Stil",
	"SciFi-Stil",
	"Schwierigkeit",

	"Keine Mods aktiviert",
	"Mods:"
};

//MapScreen
static const ST::string s_ger_zMarksMapScreenText[zMarksMapScreenText_SIZE] =
{
	"Map-Level",
	"Sie haben gar keine Miliz. Sie müssen Bewohner der Stadt trainieren, wenn Sie dort eine Miliz aufstellen wollen.",
	"Tägliches Einkommen",
	"Söldner hat Lebensversicherung",
	"%s ist nicht müde.",
	"%s ist unterwegs und kann nicht schlafen.",
	"%s ist zu müde. Versuchen Sie es ein bisschen später nochmal.",
	"%s fährt.",
	"Der Trupp kann nicht weiter, wenn einer der Söldner pennt.",


	// stuff for contracts
	"Sie können zwar den Vertrag bezahlen, haben aber kein Geld für die Lebensversicherung.",
	"%s Lebensversicherungsprämien kosten %s pro %d Zusatztag(en). Wollen Sie das bezahlen?",
	"Sektorinventar",

	"Söldner hat Krankenversicherung.",


	// other items
	"Sanitäter", // people acting a field medics and bandaging wounded mercs
	"Patienten", // people who are being bandaged by a medic
	"Fertig", // Continue on with the game after autobandage is complete
	"Stop", // Stop autobandaging of patients by medics now

	"%s hat kein Werkzeug.",
	"%s hat kein Verbandszeug.",
	"Es sind nicht genug Leute zum Training bereit.",
	"%s ist voller Milizen.",
	"Söldner hat begrenzten Vertrag.",
	"Vertrag des Söldners ist nicht versichert",
};

static const ST::string s_ger_pLandMarkInSectorString = "Trupp %d hat in Sektor %s jemanden bemerkt";

// confirm the player wants to pay X dollars to build a militia force in town
static const ST::string s_ger_pMilitiaConfirmStrings[pMilitiaConfirmStrings_SIZE] =
{
	"Eine Milizeinheit für diese Stadt zu trainieren kostet $", // telling player how much it will cost
	"Ausgabe genehmigen?", // asking player if they wish to pay the amount requested
	"Sie haben nicht genug Geld.", // telling the player they can't afford to train this town
	"Miliz in %s (%s %d) weitertrainieren?", // continue training this town?
	"Preis $", // the cost in dollars to train militia
	"( J/N )", // abbreviated yes/no
	"Milizen in %d Sektoren zu trainieren kostet $ %d. %s", // cost to train sveral sectors at once
	"Sie können sich keine $%d für die Miliz hier leisten.",
	"%s benötigt eine Loyalität von %d Prozent, um mit dem Milizen-Training fortzufahren.",
	"Sie können die Miliz in %s nicht mehr trainieren.",
};


//Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom of the single merc panel
static const ST::string s_ger_gzMoneyWithdrawMessageText[gzMoneyWithdrawMessageText_SIZE] =
{
	"Sie können nur maximal 20.000$ abheben.",
	"Wollen Sie wirklich %s auf Ihr Konto einzahlen?",
};

static const ST::string s_ger_gzCopyrightText = "Copyright (C) 1999 Sir-tech Canada Ltd. Alle Rechte vorbehalten.";

//option Text
static const ST::string s_ger_zOptionsToggleText[zOptionsToggleText_SIZE] =
{
	"Sprache",
	"Stumme Bestätigungen",
	"Untertitel",
	"Dialoge Pause",
	"Rauch animieren",
	"Sichtbare Verletzungen",
	"Cursor nicht bewegen!",
	"Alte Auswahlmethode",
	"Weg vorzeichnen",
	"Fehlschüsse anzeigen",
	"Bestätigung bei Echtzeit",
	"Schlaf-/Wachmeldung anzeigen",
	"Metrisches System benutzen",
	"Boden beleuchten",
	"Cursor automatisch auf Söldner",
	"Cursor automatisch auf Türen",
	"Gegenstände leuchten",
	"Baumkronen zeigen",
	"Drahtgitter zeigen",
	"3D Cursor zeigen",
};

//This is the help text associated with the above toggles.
static const ST::string s_ger_zOptionsScreenHelpText[zOptionsToggleText_SIZE] =
{
	//speech
	"Mit dieser Option hören Sie die Dialoge.",

	//Mute Confirmation
	"Schaltet die gesprochenen Bestätigungen an oder aus.",

	//Subtitles
	"Schaltet Untertitel für Dialoge ein oder aus.",

	//Key to advance speech
	"Wenn Untertitel AN sind, hat man durch diese Option Zeit, Dialoge von NPCs zu lesen.",

	//Toggle smoke animation
	"Schalten Sie diese Option ab, wenn animierter Rauch Ihre Bildwiederholrate verlangsamt.",

	//Blood n Gore
	"Diese Option abschalten, wenn Sie kein Blut sehen können.",

	//Never move my mouse
	"Wenn Sie diese Option abstellen, wird der Mauszeiger nicht mehr von den Popup-Fenstern verdeckt.",

	//Old selection method
	"Mit dieser Option funktioniert die Auswahl der Söldner so wie in früheren JAGGED ALLIANCE-Spielen (also genau andersherum als jetzt).",

	//Show movement path
	"Diese Funktion ANschalten, um die geplanten Wege der Söldner in Echtzeit anzuzeigen\n(oder abgeschaltet lassen und bei gewünschter Anzeige die SHIFT-Taste drücken).",

	//show misses
	"Mit dieser Option zeigt Ihnen das Spiel, wo Ihre Kugeln hinfliegen, wenn Sie \"nicht treffen\".",

	//Real Time Confirmation
	"Durch diese Option wird vor der Rückkehr in den Echtzeit-Modus ein zusätzlicher \"Sicherheits\"-Klick verlangt.",

	//Sleep/Wake notification
	"Wenn ANgeschaltet werden Sie informiert, wann Ihre Söldner, die sich im \"Dienst\" befinden schlafen oder die Arbeit wieder aufnehmen.",

	//Use the metric system
	"Mit dieser Option wird im Spiel das metrische System verwendet.",

	//Merc Lighted movement
	"Diese Funktion ANschalten, wenn der Söldner beim Gehen den Boden beleuchten soll. AUSgeschaltet erhöht sich die Bildwiederholrate.",

	//Smart cursor
	"Wenn diese Funktion aktiviert ist, werden Söldner automatisch hervorgehoben, sobald der Cursor in ihrer Nähe ist.",

	//snap cursor to the door
	"Wenn diese Funktion aktiviert ist, springt der Cursor automatisch auf eine Tür, sobald eine in der Nähe ist.",

	//glow items
	"Angeschaltet, bekommen Gegenstände einen pulsierenden Rahmen (|I).",

	//toggle tree tops
	"ANgeschaltet, werden die Baumkronen gezeigt (|T).",

	//toggle wireframe
	"ANgeschaltet, werden Drahtgitter für verborgene Wände gezeigt (|W).",

	"ANgeschaltet, wird der Bewegungs-Cursor in 3D angezeigt (|P|o|s|1).",

};

static const ST::string s_ger_gzGIOScreenText[gzGIOScreenText_SIZE] =
{
	"GRUNDEINSTELLUNGEN",
	"Spielmodus",
	"Realistisch",
	"SciFi",
	"Waffen",
	"Zusätzliche Waffen",
	"Normal",
	"Schwierigkeitsgrad",
	"Einsteiger",
	"Profi",
	"Alter Hase",
	"Ok",
	"Abbrechen",
	"Extraschwer",
	"Speichern jederzeit möglich",
	"Iron Man",
	"Tot ist tot"
};

// This string is used in the IMP character generation.  It is possible to
// select 0 ability in a skill meaning you can't use it.  This text is
// confirmation to the player.
static const ST::string s_ger_pSkillAtZeroWarning    = "Sind Sie sicher? Ein Wert von 0 bedeutet, dass der Charakter diese Fähigkeit nicht nutzen kann.";
static const ST::string s_ger_pIMPBeginScreenStrings = "(max. 8 Buchstaben)";
static const ST::string s_ger_pIMPFinishButtonText   = "Analyse wird durchgeführt";
static const ST::string s_ger_pIMPFinishStrings      = "Danke, %s"; //%s is the name of the merc
static const ST::string s_ger_pIMPVoicesStrings      = "Stimme"; // the strings for imp voices screen

// title for program
static const ST::string s_ger_pPersTitleText = "Söldner-Manager";

// paused game strings
static const ST::string s_ger_pPausedGameText[pPausedGameText_SIZE] =
{
	"Pause",
	"Zurück zum Spiel (|P|a|u|s|e)",
	"Pause (|P|a|u|s|e)",
};

static const ST::string s_ger_pMessageStrings[pMessageStrings_SIZE] =
{
	"Spiel beenden?",
	"OK",
	"JA",
	"NEIN",
	"ABBRECHEN",
	"ZURÜCK",
	"LÜGEN",
	"Keine Beschreibung", //Save slots that don't have a description.
	"Spiel gespeichert",
	"Tag",
	"Söldner",
	"Leerer Slot", //An empty save game slot
	"KpM",					//Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
	"min",					//Abbreviation for minute.
	"m",						//One character abbreviation for meter (metric distance measurement unit).
	"Kgln",				//Abbreviation for rounds (# of bullets)
	"kg",					//Abbreviation for kilogram (metric weight measurement unit)
	"Pfd",					//Abbreviation for pounds (Imperial weight measurement unit)
	"Home",				//Home as in homepage on the internet.
	"US$",					//Abbreviation for US Dollars
	"n.a",					//Lowercase acronym for not applicable.
	"Inzwischen",		//Meanwhile
	"%s ist im Sektor %s%s angekommen", //Name/Squad has arrived in sector A9. Order must not change without notifying SirTech
	"Version",
	"Neuen Spielstand anlegen",
	"Dieser Slot ist nur für Quick-Saves aus den Map Screens und dem Taktik-Bildschirm. Speichern mit ALT+S",
	"offen",
	"zu",
	"Ihr Festplattenspeicher ist knapp. Sie haben lediglich %sMB frei und Jagged Alliance 2 benötigt %sMB.",
	"%s hat %s gefangen.",		//'Merc name' has caught 'item' -- let SirTech know if name comes after item.

	"%s hat die Droge genommen.", //'Merc name' has taken the drug
	"%s hat keine medizinischen Fähigkeiten",//'Merc name' has no medical skill.

	//CDRom errors (such as ejecting CD while attempting to read the CD)
	"Die Integrität des Spieles wurde beschädigt.", //The integrity of the game has been compromised
	"FEHLER: CD-ROM-Laufwerk schließen",

	//When firing heavier weapons in close quarters, you may not have enough room to do so.
	"Kein Platz, um von hier aus zu feuern.",

	//Can't change stance due to objects in the way...
	"Kann seine Position jetzt nicht ändern.",

	//Simple text indications that appear in the game, when the merc can do one of these things.
	"Ablegen",
	"Werfen",
	"Weitergeben",

	"%s weitergegeben an %s.",	//"Item" passed to "merc". Please try to keep the item %s before the merc %s,
					//otherwise, must notify SirTech.
	"Kein Platz, um %s an %s weiterzugeben.", //pass "item" to "merc". Same instructions as above.

	//A list of attachments appear after the items. Ex: Kevlar vest ( Ceramic Plate 'Attached )'
	" angebracht)",

	//Cheat modes
	"Cheat-Level EINS erreicht",
	"Cheat-Level ZWEI erreicht",

	//Toggling various stealth modes
	"Stealth Mode für Trupp ein.",
	"Stealth Mode für Trupp aus.",
	"Stealth Mode für %s ein.",
	"Stealth Mode für %s aus.",

	//Wireframes are shown through buildings to reveal doors and windows that can't otherwise be seen in
	//an isometric engine. You can toggle this mode freely in the game.
	"Drahtgitter ein",
	"Drahtgitter aus",

	//These are used in the cheat modes for changing levels in the game. Going from a basement level to
	//an upper level, etc.
	"Von dieser Ebene geht es nicht nach oben...",
	"Noch tiefere Ebenen gibt es nicht...",
	"Gewölbeebene %d betreten...",
	"Gewölbe verlassen...",

	"s",		// used in the shop keeper inteface to mark the ownership of the item eg Red's gun
	"Autoscrolling AUS.",
	"Autoscrolling AN.",
	"3D-Cursor AUS.",
	"3D-Cursor AN.",
	"Trupp %d aktiv.",
	"Sie können %ss Tagessold von %s nicht zahlen",	//first %s is the mercs name, the second is a string containing the salary
	"Abbruch",
	"%s kann alleine nicht gehen.",
	"Spielstand namens Spielstand99.sav kreiert. Wenn nötig, in Spielstand01 - Spielstand10 umbennen und über die Option 'Laden' aufrufen.",
	"%s hat %s getrunken.",
	"Paket in Drassen angekommen.",
	"%s kommt am %d. um ca. %s am Zielort an (Sektor %s).",		//first %s is mercs name(OK), next is the sector location and name where they will be arriving in, lastely is the day an the time of arrival       !!!7 It should be like this: first one is merc (OK), next is day of arrival (OK) , next is time of the day for ex. 07:00 (not OK, now it is still sector), next should be sector (not OK, now it is still time of the day)
	"Logbuch aktualisiert.",
};

static const ST::string s_ger_ItemPickupHelpPopup[ItemPickupHelpPopup_SIZE] =
{
	"OK",
	"Hochscrollen",
	"Alle auswählen",
	"Runterscrollen",
	"Abbrechen",
};

static const ST::string s_ger_pDoctorWarningString[pDoctorWarningString_SIZE] =
{
	"%s ist nicht nahe genug, um geheilt zu werden",
	"Ihre Mediziner haben noch nicht alle verbinden können.",
};

static const ST::string s_ger_pMilitiaButtonsHelpText[pMilitiaButtonsHelpText_SIZE] =
{
	"Grüne Miliz aufnehmen(Rechtsklick)/absetzen(Linksklick)", // button help text informing player they can pick up or drop militia with this button
	"Reguläre Milizen aufnehmen(Rechtsklick)/absetzen(Linksklick)",
	"Elitemilizen aufnehmen(Rechtsklick)/absetzen(Linksklick)",
	"Milizen gleichmäßig über alle Sektoren verteilen",
};

// to inform the player to hire some mercs to get things going
static const ST::string s_ger_pMapScreenJustStartedHelpText = "Zu AIM gehen und Söldner anheuern ( *Tip*: Befindet sich im Laptop )";

static const ST::string s_ger_pAntiHackerString = "Fehler. Fehlende oder fehlerhafte Datei(en). Spiel wird beendet.";

static const ST::string s_ger_gzLaptopHelpText[gzLaptopHelpText_SIZE] =
{
	//Buttons:
	"E-Mail einsehen",
	"Websites durchblättern",
	"Dateien und Anlagen einsehen",
	"Logbuch lesen",
	"Team-Info einsehen",
	"Finanzen und Notizen einsehen",

	"Laptop schließen",

	//Bottom task bar icons (if they exist):
	"Sie haben neue Mail",
	"Sie haben neue Dateien",

	//Bookmarks:
	"Association of International Mercenaries",
	"Bobby Rays Online-Waffenversand",
	"Bundesinstitut für Söldnerevaluierung",
	"More Economic Recruiting Center",
	"McGillicuttys Bestattungen",
	"Fleuropa",
	"Versicherungsmakler für A.I.M.-Verträge",
};

static const ST::string s_ger_gzHelpScreenText = "Helpscreen verlassen";

static const ST::string s_ger_gzNonPersistantPBIText[gzNonPersistantPBIText_SIZE] =
{
	"Es tobt eine Schlacht. Sie können sich nur im Taktikbildschirm zurückziehen.",
	"Sektor betreten und Kampf fortsetzen (|E).",
	"Kampf durch PC entscheiden (|A).",
	"Sie können den Kampf nicht vom PC entscheiden lassen, wenn Sie angreifen.",
	"Sie können den Kampf nicht vom PC entscheiden lassen, wenn Sie in einem Hinterhalt sind.",
	"Sie können den Kampf nicht vom PC entscheiden lassen, wenn Sie gegen Monster kämpfen.",
	"Sie können den Kampf nicht vom PC entscheiden lassen, wenn feindliche Zivilisten da sind.",
	"Sie können einen Kampf nicht vom PC entscheiden lassen, wenn Bloodcats da sind.",
	"KAMPF IM GANGE",
	"Sie können sich nicht zurückziehen, wenn Sie in einem Hinterhalt sind.",
};

static const ST::string s_ger_gzMiscString[gzMiscString_SIZE] =
{
	"Ihre Milizen kämpfen ohne die Hilfe der Söldner weiter...",
	"Das Fahrzeug muss nicht mehr aufgetankt werden.",
	"Der Tank ist %d%% voll.",
	"Deidrannas Armee hat wieder volle Kontrolle über %s.",
	"Sie haben ein Tanklager verloren.",
};


static const ST::string s_ger_gzIntroScreen = "Kann Introvideo nicht finden";

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
static const ST::string s_ger_pNewNoiseStr[pNewNoiseStr_SIZE] =
{
	//There really isn't any difference between using "coming from" or "to".
	//For the explosion case the string in English could be either:
	//	"Gus hears a loud EXPLOSION 'to' the north.",
	//	"Gus hears a loud EXPLOSION 'coming from' the north.",
	//For certain idioms, it sounds better to use one over the other. It is a matter of preference.
	"%s hört %s aus dem %s.",
	"%s hört eine BEWEGUNG (%s) von %s.",
	"%s hört ein KNARREN (%s) von %s.",
	"%s hört ein KLATSCHEN (%s) von %s.",
	"%s hört einen AUFSCHLAG (%s) von %s.",
	"%s hört eine EXPLOSION (%s) von %s.",
	"%s hört einen SCHREI (%s) von %s.",
	"%s hört einen AUFSCHLAG (%s) von %s.",
	"%s hört einen AUFSCHLAG (%s) von %s.",
	"%s hört ein ZERBRECHEN (%s) von %s.",
	"%s hört ein ZERSCHMETTERN (%s) von %s.",
};

static const ST::string s_ger_wMapScreenSortButtonHelpText[wMapScreenSortButtonHelpText_SIZE] =
{
	"Sortieren nach Name (|F|1)",
	"Sortieren nach Auftrag (|F|2)",
	"Sortieren nach wach/schlafend (|F|3)",
	"Sortieren nach Ort (|F|4)",
	"Sortieren nach Ziel (|F|5)",
	"Sortieren nach Vertragsende (|F|6)",
};

static const ST::string s_ger_BrokenLinkText[BrokenLinkText_SIZE] =
{
	"Error 404",
	"Site nicht gefunden.",
};

static const ST::string s_ger_gzBobbyRShipmentText[gzBobbyRShipmentText_SIZE] =
{
	"Letzte Lieferungen",
	"Bestellung #",
	"Artikelanzahl",
	"Bestellt am",
};

static const ST::string s_ger_gzCreditNames[gzCreditNames_SIZE]=
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

static const ST::string s_ger_gzCreditNameTitle[gzCreditNameTitle_SIZE]=
{
	"Game Internals Programmer", 			// Chris Camfield
	"Co-designer/Writer",							// Shaun Lyng
	"Strategic Systems & Editor Programmer",					//Kris \"The Cow Rape Man\" Marnes
	"Producer/Co-designer",						// Ian Currie
	"Co-designer/Map Designer",				// Linda Currie
	"Artist",													// Eric \"WTF\" Cheng
	"Beta Coordinator, Support",				// Lynn Holowka
	"Artist Extraordinaire",						// Norman \"NRG\" Olsen
	"Sound Guru",											// George Brooks
	"Screen Designer/Artist",					// Andrew Stacey
	"Lead Artist/Animator",						// Scot Loving
	"Lead Programmer",									// Andrew \"Big Cheese Doddle\" Emmons
	"Programmer",											// Dave French
	"Strategic Systems & Game Balance Programmer",					// Alex Meduna
	"Portraits Artist",								// Joey \"Joeker\" Whelan",
};

static const ST::string s_ger_gzCreditNameFunny[gzCreditNameFunny_SIZE]=
{
	"", 																			// Chris Camfield
	"(still learning punctuation)",					// Shaun Lyng
	"(\"It's done. I'm just fixing it\")",	//Kris \"The Cow Rape Man\" Marnes
	"(getting much too old for this)",				// Ian Currie
	"(and working on Wizardry 8)",						// Linda Currie
	"(forced at gunpoint to also do QA)",			// Eric \"WTF\" Cheng
	"(Left us for the CFSA - go figure...)",	// Lynn Holowka
	"",																			// Norman \"NRG\" Olsen
	"",																			// George Brooks
	"(Dead Head and jazz lover)",						// Andrew Stacey
	"(his real name is Robert)",							// Scot Loving
	"(the only responsible person)",					// Andrew \"Big Cheese Doddle\" Emmons
	"(can now get back to motocrossing)",	// Dave French
	"(stolen from Wizardry 8)",							// Alex Meduna
	"(did items and loading screens too!)",	// Joey \"Joeker\" Whelan",
};

static const ST::string s_ger_sRepairsDoneString[sRepairsDoneString_SIZE] =
{
	"%s hat  seine eigenen Gegenstände repariert",
	"%s hat die Waffen und Rüstungen aller Teammitglieder repariert",
	"%s hat die aktivierten Gegenstände aller Teammitglieder repariert",
	"%s hat die mitgeführten Gegenstände aller Teammitglieder repariert",
};

static const ST::string s_ger_zGioDifConfirmText[zGioDifConfirmText_SIZE]=
{
	"Sie haben sich für den EINSTEIGER-Modus entschieden. Dies ist die passende Einstellung für Spieler, die noch nie zuvor Jagged Alliance oder ähnliche Spiele gespielt haben oder für Spieler, die sich ganz einfach kürzere Schlachten wünschen. Ihre Wahl wird den Verlauf des ganzen Spiels beeinflussen. Treffen Sie also eine sorgfältige Wahl. Sind Sie ganz sicher, dass Sie im Einsteiger-Modus spielen wollen?",
	"Sie haben sich für den FORTGESCHRITTENEN-Modus entschieden. Dies ist die passende Einstellung für Spieler, die bereits Erfahrung mit Jagged Alliance oder ähnlichen Spielen haben. Ihre Wahl wird den Verlauf des ganzen Spiels beeinflussen. Treffen Sie also eine sorgfältige Wahl. Sind Sie ganz sicher, dass Sie im Fortgeschrittenen-Modus spielen wollen?",
	"Sie haben sich für den PROFI-Modus entschieden. Na gut, wir haben Sie gewarnt. Machen Sie hinterher bloß nicht uns dafür verantwortlich, wenn Sie im Sarg nach Hause kommen. Ihre Wahl wird den Verlauf des ganzen Spiels beeinflussen. Treffen Sie also eine sorgfältige Wahl. Sind Sie ganz sicher, dass Sie im Profi-Modus spielen wollen?",
};

static const ST::string s_ger_gzLateLocalizedString[gzLateLocalizedString_SIZE] =
{
	//1-5
	"Der Roboter kann diesen Sektor nicht verlassen, wenn niemand die Fernbedienung benutzt.",

	"Sie können den Zeitraffer jetzt nicht benutzen. Warten Sie das Feuerwerk ab!",
	"%s will sich nicht bewegen.",
	"%s hat nicht genug Energie, um die Position zu ändern.",
	"{} hat kein Benzin mehr und steckt in {} fest.",

	//6-10

	// the following two strings are combined with the strings below to report noises
	// heard above or below the merc
	"oben",
	"unten",

	//The following strings are used in autoresolve for autobandaging related feedback.
	"Keiner der Söldner hat medizinische Fähigkeiten.",
	"Sie haben kein Verbandszeug.",
	"Sie haben nicht genug Verbandszeug, um alle zu verarzten.",
	"Keiner der Söldner muss verbunden werden.",
	"Söldner automatisch verbinden.",
	"Alle Söldner verarztet.",

	//14-16
	"Arulco",
	"(Dach)",
	"Gesundheit: %d/%d",

	//17
	//In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
	//"vs." is the abbreviation of versus.
	"%d gegen %d",

	//18-19
	"%s ist voll!", //(ex "The ice cream truck is full")
	"%s braucht nicht eine schnelle Erste Hilfe, sondern eine richtige medizinische Betreuung und/oder Erholung.",

	//20
	//Happens when you get shot in the legs, and you fall down.
	"%s ist am Bein getroffen und hingefallen!",
	//Name can't speak right now.
	"%s kann gerade nicht sprechen.",

	//22-24 plural versions
	"%d grüne Milizen wurden zu Elitemilizen befördert.",
	"%d grüne Milizen wurden zu regulären Milizen befördert.",
	"%d reguläre Milizen wurden zu Elitemilizen befördert.",

	//25
	"Schalter",

	//26
	//Name has gone psycho -- when the game forces the player into burstmode (certain unstable characters)
	"%s dreht durch!",

	//27-28
	//Messages why a player can't time compress.
	"Es ist momentan gefährlich den Zeitraffer zu betätigen, da Sie noch Söldner in Sektor %s haben.",
	"Es ist gefährlich den Zeitraffer zu betätigen, wenn Sie noch Söldner in den von Monstern verseuchten Minen haben.",

	//29-31 singular versions
	"1 grüne Miliz wurde zur Elitemiliz befördert.",
	"1 grüne Miliz wurde zur regulären Miliz befördert.",
	"1 reguläre Miliz wurde zur Elitemiliz befördert.",

	//32-34
	"%s sagt überhaupt nichts.",
	"Zur Oberfläche gehen?",
	"(Trupp %d)",

	//35
	"%s reparierte %ss %s",

	//36
	"BLOODCAT",

	//37-38 "Name trips and falls"
	"%s stolpert und stürzt",
	"Dieser Gegenstand kann von hier aus nicht aufgehoben werden.",

	//39
	"Keiner Ihrer übrigen Söldner ist in der Lage zu kämpfen. Die Miliz wird die Monster alleine bekämpfen",

	//40-43
	//%s is the name of merc.
	"%s hat keinen Erste-Hilfe-Kasten mehr!",
	"%s hat nicht das geringste Talent, jemanden zu verarzten!",
	"%s hat keinen Werkzeugkasten mehr!",
	"%s ist absolut unfähig dazu, irgend etwas zu reparieren!",

	//44
	"Repar. Zeit",
	"%s kann diese Person nicht sehen.",

	//46-48
	"%ss Gewehrlauf-Verlängerung fällt ab!",
	"Pro Sektor sind nicht mehr als %d Milizausbilder erlaubt.",
	"Sind Sie sicher?", //

	//49-50
	"Zeitraffer", //time compression
	"Der Fahrzeugtank ist jetzt voll.",

	//51-52 Fast help text in mapscreen.
	"Zeitraffer fortsetzen (|L|e|e|r|t|a|s|t|e)",
	"Zeitraffer anhalten (|E|s|c)",

	//53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
	"%s hat die Ladehemmung der %s behoben",
	"%s hat die Ladehemmung von %ss %s behoben",

	//55
	"Die Zeit kann nicht komprimiert werden, während das Sektorinventar eingesehen wird.",

	//"Im Sektor sind Feinde entdeckt worden",		//STR_DETECTED_SIMULTANEOUS_ARRIVAL

	//56
	//Displayed with the version information when cheats are enabled.
	"Aktueller/Maximaler Fortschritt: %d%%/%d%%",

	//57
	"John und Mary eskortieren?",

	"Schalter betätigt.",
};

static const ST::string s_ger_str_ceramic_plates_smashed = "%ss Keramikplatten wurden zertrümmert!";

static const ST::string s_ger_str_arrival_rerouted = "Die Ankunft neuer Rekruten wurde nach Sektor %s umgeleitet, da der geplante Ankunftsort in Sektor %s vom Feind besetzt ist.";


static const ST::string s_ger_str_stat_health       = "Gesundheit";
static const ST::string s_ger_str_stat_agility      = "Beweglichkeit";
static const ST::string s_ger_str_stat_dexterity    = "Geschicklichkeit";
static const ST::string s_ger_str_stat_strength     = "Kraft";
static const ST::string s_ger_str_stat_leadership   = "Führungsqualität";
static const ST::string s_ger_str_stat_wisdom       = "Weisheit";
static const ST::string s_ger_str_stat_exp_level    = "Erfahrungsstufe";
static const ST::string s_ger_str_stat_marksmanship = "Treffsicherheit";
static const ST::string s_ger_str_stat_mechanical   = "Technik";
static const ST::string s_ger_str_stat_explosive    = "Sprengstoffe";
static const ST::string s_ger_str_stat_medical      = "Medizin";

static const ST::string s_ger_str_stat_list[str_stat_list_SIZE] =
{
	s_ger_str_stat_health,
	s_ger_str_stat_agility,
	s_ger_str_stat_dexterity,
	s_ger_str_stat_strength,
	s_ger_str_stat_leadership,
	s_ger_str_stat_wisdom,
	s_ger_str_stat_exp_level,
	s_ger_str_stat_marksmanship,
	s_ger_str_stat_mechanical,
	s_ger_str_stat_explosive,
	s_ger_str_stat_medical
};

static const ST::string s_ger_str_aim_sort_list[str_aim_sort_list_SIZE] =
{
	s_ger_str_aim_sort_price,
	s_ger_str_aim_sort_experience,
	s_ger_str_aim_sort_marksmanship,
	s_ger_str_aim_sort_medical,
	s_ger_str_aim_sort_explosives,
	s_ger_str_aim_sort_mechanical,
	s_ger_str_aim_sort_ascending,
	s_ger_str_aim_sort_descending,
};

static const ST::string s_ger_zNewTacticalMessages[] =
{
	"Entfernung zum Ziel: %d Felder",
	"Waffen-Rchwt.: %d Felder, Entf. zum Ziel: %d Felder",
	"Deckung anzeigen",
	"Sichtfeld",
	"Dies ist ein IRON MAN-Spiel, und es kann nicht gespeichert werden, wenn sich Gegner in der Nähe befinden.",
	"(Kann während Kampf nicht speichern)",
	"(Kann keine vorigen Spielstände laden)",
	"Dies ist ein Tot ist Tot Spiel und Sie können nicht über den Spiel Speichern Dialog speichern.",
	"Trefferchance: %d%%, mit Deckung: %d%%", // Stracciatella: chance to hit
};

//@@@:  New string as of March 3, 2000.
static const ST::string s_ger_str_iron_man_mode_warning = "You have chosen IRON MAN mode. This setting makes the game considerably more challenging as you will not be able to save your game when in a sector occupied by enemies. This setting will affect the entire course of the game.  Are you sure want to play in IRON MAN mode?";

// Stracciatella: New Dead is Dead game mode warning
static const ST::string s_ger_str_dead_is_dead_mode_warning = "Sie haben den TOT IST TOT Modus ausgewählt. Diese Einstellung macht das Spiel bedeutend schwieriger, da Sie nicht in der Lage sein werden einen alten Spielstand zu laden falls Ihnen ein Fehler unterläuft. Das Spiel speichert den aktuellen Stand beim verlassen automatisch ab. Diese Einstellung wird den gesamten Spielverlauf verändern.  Sind Sie sicher dass Sie den TOT IST TOT Modus spielen wollen?";
static const ST::string s_ger_str_dead_is_dead_mode_enter_name = "Sie werden nun zum Spiel Speichern Menü geleitet. Wählen Sie dort einen Slot und einen Namen für ihren Speicherstand. Dieser Slot wird für den Verlauf des Spiels genutzt und kann nicht geändert werden!";

static const ST::string s_ger_gs_dead_is_dead_mode_tab_name[gs_dead_is_dead_mode_tab_name_SIZE] =
{
	"Normal", 			// Normal Tab
	"TIT", 			// Dead is Dead Tab
};

// German language resources.
LanguageRes g_LanguageResGerman = {

	s_ger_WeaponType,

	s_ger_Message,
	s_ger_TeamTurnString,
	s_ger_pAssignMenuStrings,
	s_ger_pTrainingStrings,
	s_ger_pTrainingMenuStrings,
	s_ger_pAttributeMenuStrings,
	s_ger_pVehicleStrings,
	s_ger_pShortAttributeStrings,
	s_ger_pContractStrings,
	s_ger_pAssignmentStrings,
	s_ger_pConditionStrings,
	s_ger_pPersonnelScreenStrings,
	s_ger_pUpperLeftMapScreenStrings,
	s_ger_pTacticalPopupButtonStrings,
	s_ger_pSquadMenuStrings,
	s_ger_pDoorTrapStrings,
	s_ger_pLongAssignmentStrings,
	s_ger_pMapScreenMouseRegionHelpText,
	s_ger_pNoiseVolStr,
	s_ger_pNoiseTypeStr,
	s_ger_pDirectionStr,
	s_ger_pRemoveMercStrings,
	s_ger_sTimeStrings,
	s_ger_pInvPanelTitleStrings,
	s_ger_pPOWStrings,
	s_ger_pMilitiaString,
	s_ger_pMilitiaButtonString,
	s_ger_pEpcMenuStrings,
	s_ger_pRepairStrings,
	s_ger_sPreStatBuildString,
	s_ger_sStatGainStrings,
	s_ger_pHelicopterEtaStrings,
	s_ger_sMapLevelString,
	s_ger_gsLoyalString,
	s_ger_gsUndergroundString,
	s_ger_gsTimeStrings,
	s_ger_sFacilitiesStrings,
	s_ger_pMapPopUpInventoryText,
	s_ger_pwTownInfoStrings,
	s_ger_pwMineStrings,
	s_ger_pwMiscSectorStrings,
	s_ger_pMapInventoryErrorString,
	s_ger_pMapInventoryStrings,
	s_ger_pMovementMenuStrings,
	s_ger_pUpdateMercStrings,
	s_ger_pMapScreenBorderButtonHelpText,
	s_ger_pMapScreenBottomFastHelp,
	s_ger_pMapScreenBottomText,
	s_ger_pMercDeadString,
	s_ger_pSenderNameList,
	s_ger_pNewMailStrings,
	s_ger_pDeleteMailStrings,
	s_ger_pEmailHeaders,
	s_ger_pEmailTitleText,
	s_ger_pFinanceTitle,
	s_ger_pFinanceSummary,
	s_ger_pFinanceHeaders,
	s_ger_pTransactionText,
	s_ger_pMoralStrings,
	s_ger_pSkyriderText,
	s_ger_str_left_equipment,
	s_ger_pMapScreenStatusStrings,
	s_ger_pMapScreenPrevNextCharButtonHelpText,
	s_ger_pEtaString,
	s_ger_pShortVehicleStrings,
	s_ger_pTrashItemText,
	s_ger_pMapErrorString,
	s_ger_pMapPlotStrings,
	s_ger_pBullseyeStrings,
	s_ger_pMiscMapScreenMouseRegionHelpText,
	s_ger_str_he_leaves_where_drop_equipment,
	s_ger_str_she_leaves_where_drop_equipment,
	s_ger_str_he_leaves_drops_equipment,
	s_ger_str_she_leaves_drops_equipment,
	s_ger_pImpPopUpStrings,
	s_ger_pImpButtonText,
	s_ger_pExtraIMPStrings,
	s_ger_pFilesTitle,
	s_ger_pFilesSenderList,
	s_ger_pHistoryLocations,
	s_ger_pHistoryStrings,
	s_ger_pHistoryHeaders,
	s_ger_pHistoryTitle,
	s_ger_pShowBookmarkString,
	s_ger_pWebPagesTitles,
	s_ger_pWebTitle,
	s_ger_pPersonnelString,
	s_ger_pErrorStrings,
	s_ger_pDownloadString,
	s_ger_pBookMarkStrings,
	s_ger_pLaptopIcons,
	s_ger_gsAtmStartButtonText,
	s_ger_pPersonnelTeamStatsStrings,
	s_ger_pPersonnelCurrentTeamStatsStrings,
	s_ger_pPersonelTeamStrings,
	s_ger_pPersonnelDepartedStateStrings,
	s_ger_pMapHortIndex,
	s_ger_pMapVertIndex,
	s_ger_pMapDepthIndex,
	s_ger_pLaptopTitles,
	s_ger_pDayStrings,
	s_ger_pMilitiaConfirmStrings,
	s_ger_pSkillAtZeroWarning,
	s_ger_pIMPBeginScreenStrings,
	s_ger_pIMPFinishButtonText,
	s_ger_pIMPFinishStrings,
	s_ger_pIMPVoicesStrings,
	s_ger_pPersTitleText,
	s_ger_pPausedGameText,
	s_ger_zOptionsToggleText,
	s_ger_zOptionsScreenHelpText,
	s_ger_pDoctorWarningString,
	s_ger_pMilitiaButtonsHelpText,
	s_ger_pMapScreenJustStartedHelpText,
	s_ger_pLandMarkInSectorString,
	s_ger_gzMercSkillText,
	s_ger_gzNonPersistantPBIText,
	s_ger_gzMiscString,
	s_ger_wMapScreenSortButtonHelpText,
	s_ger_pNewNoiseStr,
	s_ger_gzLateLocalizedString,
	s_ger_pAntiHackerString,
	s_ger_pMessageStrings,
	s_ger_ItemPickupHelpPopup,
	s_ger_TacticalStr,
	s_ger_LargeTacticalStr,
	s_ger_zDialogActions,
	s_ger_zDealerStrings,
	s_ger_zTalkMenuStrings,
	s_ger_gzMoneyAmounts,
	s_ger_gzProsLabel,
	s_ger_gzConsLabel,
	s_ger_gMoneyStatsDesc,
	s_ger_gWeaponStatsDesc,
	s_ger_sKeyDescriptionStrings,
	s_ger_zHealthStr,
	s_ger_zVehicleName,
	s_ger_pExitingSectorHelpText,
	s_ger_InsContractText,
	s_ger_InsInfoText,
	s_ger_MercAccountText,
	s_ger_MercInfo,
	s_ger_MercNoAccountText,
	s_ger_MercHomePageText,
	s_ger_sFuneralString,
	s_ger_sFloristText,
	s_ger_sOrderFormText,
	s_ger_sFloristGalleryText,
	s_ger_sFloristCards,
	s_ger_BobbyROrderFormText,
	s_ger_BobbyRText,
	s_ger_str_bobbyr_guns_num_guns_that_use_ammo,
	s_ger_BobbyRaysFrontText,
	s_ger_AimSortText,
	s_ger_str_aim_sort_price,
	s_ger_str_aim_sort_experience,
	s_ger_str_aim_sort_marksmanship,
	s_ger_str_aim_sort_medical,
	s_ger_str_aim_sort_explosives,
	s_ger_str_aim_sort_mechanical,
	s_ger_str_aim_sort_ascending,
	s_ger_str_aim_sort_descending,
	s_ger_AimPolicyText,
	s_ger_AimMemberText,
	s_ger_CharacterInfo,
	s_ger_VideoConfercingText,
	s_ger_AimPopUpText,
	s_ger_AimLinkText,
	s_ger_AimHistoryText,
	s_ger_AimFiText,
	s_ger_AimAlumniText,
	s_ger_AimScreenText,
	s_ger_AimBottomMenuText,
	s_ger_zMarksMapScreenText,
	s_ger_gpStrategicString,
	s_ger_gpGameClockString,
	s_ger_SKI_Text,
	s_ger_SkiMessageBoxText,
	s_ger_zSaveLoadText,
	s_ger_zOptionsText,
	s_ger_gzGIOScreenText,
	s_ger_gzHelpScreenText,
	s_ger_gzLaptopHelpText,
	s_ger_gzMoneyWithdrawMessageText,
	s_ger_gzCopyrightText,
	s_ger_BrokenLinkText,
	s_ger_gzBobbyRShipmentText,
	s_ger_zGioDifConfirmText,
	s_ger_gzCreditNames,
	s_ger_gzCreditNameTitle,
	s_ger_gzCreditNameFunny,
	s_ger_pContractButtonString,
	s_ger_gzIntroScreen,
	s_ger_pUpdatePanelButtons,
	s_ger_sRepairsDoneString,
	s_ger_str_ceramic_plates_smashed,
	s_ger_str_arrival_rerouted,

	s_ger_str_stat_health,
	s_ger_str_stat_agility,
	s_ger_str_stat_dexterity,
	s_ger_str_stat_strength,
	s_ger_str_stat_leadership,
	s_ger_str_stat_wisdom,
	s_ger_str_stat_exp_level,
	s_ger_str_stat_marksmanship,
	s_ger_str_stat_mechanical,
	s_ger_str_stat_explosive,
	s_ger_str_stat_medical,

	s_ger_str_stat_list,
	s_ger_str_aim_sort_list,

	s_ger_zNewTacticalMessages,
	s_ger_str_iron_man_mode_warning,
	s_ger_str_dead_is_dead_mode_warning,
	s_ger_str_dead_is_dead_mode_enter_name,

	s_ger_gs_dead_is_dead_mode_tab_name,

	s_ger_gzIMPSkillTraitsText,
};

#ifdef WITH_UNITTESTS
TEST(StringEncodingTest, GermanTextFile)
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
