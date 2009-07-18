#ifdef GERMAN

#include "Text.h"


/*
******************************************************************************************************
**																	IMPORTANT TRANSLATION NOTES																			**
******************************************************************************************************

GENERAL TOPWARE INSTRUCTIONS
- Always be aware that German strings should be of equal or shorter length than the English equivalent.
	I know that this is difficult to do on many occasions due to the nature of the German language when
	compared to English. By doing so, this will greatly reduce the amount of work on both sides. In
	most cases (but not all), JA2 interfaces were designed with just enough space to fit the English word.
	The general rule is if the string is very short (less than 10 characters), then it's short because of
	interface limitations. On the other hand, full sentences commonly have little limitations for length.
	Strings in between are a little dicey.
- Never translate a string to appear on multiple lines. All strings L"This is a really long string...",
	must fit on a single line no matter how long the string is. All strings start with L" and end with ",
- Never remove any extra spaces in strings. In addition, all strings containing multiple sentences only
	have one space after a period, which is different than standard typing convention. Never modify sections
	of strings contain combinations of % characters. These are special format characters and are always
	used in conjunction with other characters. For example, %s means string, and is commonly used for names,
	locations, items, etc. %d is used for numbers. %c%d is a character and a number (such as A9).
	%% is how a single % character is built. There are countless types, but strings containing these
	special characters are usually commented to explain what they mean. If it isn't commented, then
	if you can't figure out the context, then feel free to ask SirTech.
- Comments are always started with // Anything following these two characters on the same line are
	considered to be comments. Do not translate comments. Comments are always applied to the following
	string(s) on the next line(s), unless the comment is on the same line as a string.
- All new comments made by SirTech will use "//@@@ comment" (without the quotes) notation. By searching
	for @@@ everytime you recieve a new version, it will simplify your task and identify special instructions.
	Commonly, these types of comments will be used to ask you to abbreviate a string. Please leave the
	comments intact, and SirTech will remove them once the translation for that particular area is resolved.
- If you have a problem or question with translating certain strings, please use "//!!! comment"
	(without the quotes). The syntax is important, and should be identical to the comments used with @@@
	symbols. SirTech will search for !!! to look for Topware problems and questions. This is a more
	efficient method than detailing questions in email, so try to do this whenever possible.



FAST HELP TEXT -- Explains how the syntax of fast help text works.
**************

1) BOLDED LETTERS
	The popup help text system supports special characters to specify the hot key(s) for a button.
	Anytime you see a '|' symbol within the help text string, that means the following key is assigned
	to activate the action which is usually a button.

	EX: L"|Map Screen"

	This means the 'M' is the hotkey. In the game, when somebody hits the 'M' key, it activates that
	button. When translating the text to another language, it is best to attempt to choose a word that
	uses 'M'. If you can't always find a match, then the best thing to do is append the 'M' at the end
	of the string in this format:

	EX: L"Ecran De Carte (|M)" (this is the French translation)

	Other examples are used multiple times, like the Esc key or "|E|s|c" or Space -> (|S|p|a|c|e)

2) NEWLINE
	Any place you see a \n within the string, you are looking at another string that is part of the fast help
	text system. \n notation doesn't need to be precisely placed within that string, but whereever you wish
	to start a new line.

	EX: L"Clears all the mercs' positions,\nand allows you to re-enter them manually."

	Would appear as:

				Clears all the mercs' positions,
				and allows you to re-enter them manually.

	NOTE: It is important that you don't pad the characters adjacent to the \n with spaces. If we did this
				in the above example, we would see

	WRONG WAY -- spaces before and after the \n
	EX: L"Clears all the mercs' positions, \n and allows you to re-enter them manually."

	Would appear as: (the second line is moved in a character)

				Clears all the mercs' positions,
 				 and allows you to re-enter them manually.


@@@ NOTATION
************

	Throughout the text files, you'll find an assortment of comments. Comments are used to describe the
	text to make translation easier, but comments don't need to be translated. A good thing is to search for
	"@@@" after receiving new version of the text file, and address the special notes in this manner.

!!! NOTATION
************

	As described above, the "!!!" notation should be used by Topware to ask questions and address problems as
	SirTech uses the "@@@" notation.

*/

// Different weapon calibres
// CAWS is Close Assault Weapon System and should probably be left as it is
// NATO is the North Atlantic Treaty Organization
// WP is Warsaw Pact
// cal is an abbreviation for calibre
const wchar_t AmmoCaliber[][20] =
{
	L"0",
	L".38 Kal",
	L"9mm",
	L".45 Kal",
	L".357 Kal",
	L"12 Kal",
	L"CAWS",
	L"5.45mm",
	L"5.56mm",
	L"7.62mm NATO",
	L"7.62mm WP",
	L"4.7mm",
	L"5.7mm",
	L"Monster",
	L"Rakete",
	L"",
	L"",
};

// This BobbyRayAmmoCaliber is virtually the same as AmmoCaliber however the bobby version doesnt have as much room for the words.
//
// Different weapon calibres
// CAWS is Close Assault Weapon System and should probably be left as it is
// NATO is the North Atlantic Treaty Organization
// WP is Warsaw Pact
// cal is an abbreviation for calibre
const wchar_t BobbyRayAmmoCaliber[][20] =
{
	L"0",
	L".38 Kal",
	L"9mm",
	L".45 Kal",
	L".357 Kal",
	L"12 Kal",
	L"CAWS",
	L"5.45mm",
	L"5.56mm",
	L"7.62mm N.",
	L"7.62mm WP",
	L"4.7mm",
	L"5.7mm",
	L"Monster",
	L"Rakete",
	L"", // dart
};

const wchar_t WeaponType[][30] =
{
	L"Andere",
	L"Pistole",
	L"Maschinenpistole",
	L"Schwere Maschinenpistole",
	L"Gewehr",
	L"Scharfschützengewehr",
	L"Sturmgewehr",
	L"Leichtes Maschinengewehr",
	L"Schrotflinte",
};

const wchar_t* const TeamTurnString[] =
{
	L"Spielzug Spieler",
	L"Spielzug Gegner",
	L"Spielzug Monster",
	L"Spielzug Miliz",
	L"Spielzug Zivilisten",
	// planning turn
};

const wchar_t* const Message[] =
{
	// In the following 8 strings, the %s is the merc's name, and the %d (if any) is a number.

	L"%ls am Kopf getroffen, verliert einen Weisheitspunkt!",
	L"%ls an der Schulter getroffen, verliert Geschicklichkeitspunkt!",
	L"%ls an der Brust getroffen, verliert einen Kraftpunkt!",
	L"%ls an den Beinen getroffen, verliert einen Beweglichkeitspunkt!",
	L"%ls am Kopf getroffen, verliert %d Weisheitspunkte!",
	L"%ls an der Schulter getroffen, verliert %d Geschicklichkeitspunkte!",
	L"%ls an der Brust getroffen, verliert %d Kraftpunkte!",
	L"%ls an den Beinen getroffen, verliert %d Beweglichkeitspunkte!",
	L"Unterbrechung!",

	L"Verstärkung ist angekommen!",

	// In the following four lines, all %s's are merc names

	L"%ls lädt nach.",
	L"%ls hat nicht genug Action-Punkte!",
	// the following 17 strings are used to create lists of gun advantages and disadvantages
	// (separated by commas)
	L"zuverlässig",
	L"unzuverlässig",
	L"Reparatur leicht",
	L"Reparatur schwer",
	L"große Durchschlagskraft",
	L"kleine Durchschlagskraft",
	L"feuert schnell",
	L"feuert langsam",
	L"große Reichweite",
	L"kurze Reichweite",
	L"leicht",
	L"schwer",
	L"klein",
	L"schneller Feuerstoß",
	L"kein Feuerstoß",
	L"großes Magazin",
	L"kleines Magazin",

	// In the following two lines, all %s's are merc names

	L"%lss Tarnung hat sich abgenutzt.",
	L"%lss Tarnung ist weggewaschen.",

	// The first %s is a merc name and the second %s is an item name

	L"Zweite Waffe hat keine Munition!",
	L"%ls hat %ls gestohlen.",

	// The %s is a merc name

	L"%lss Waffe kann keinen Feuerstoß abgeben.",

	L"Sie haben schon eines davon angebracht.",
	L"Gegenstände zusammenfügen?",

	// Both %s's are item names

	L"Sie können %ls mit %ls nicht zusammenfügen",

	L"Keine",
	L"Munition entfernen",
	L"Modifikationen",

	//You cannot use "item(s)" and your "other item" at the same time.
	//Ex: You cannot use sun goggles and you gas mask at the same time.
	L"Sie können %ls nicht zusammen mit %ls benutzen.",	//

	L"Der Gegenstand in Ihrem Cursor kann mit anderen Gegenständen verbunden werden, indem Sie ihn in einem der vier Slots platzieren",
	L"Der Gegenstand in Ihrem Cursor kann mit anderen Gegenständen verbunden werden, indem Sie ihn in einem der vier Attachment-Slots platzieren. (Aber in diesem Fall sind die Gegenstände nicht kompatibel.)",
	L"Es sind noch Feinde im Sektor!",
	L"Geben Sie %ls %ls",
	L"%ls am Kopf getroffen!",
	L"Kampf abbrechen?",
	L"Die Modifikation ist permanent. Weitermachen?",
	L"%ls fühlt sich frischer!",
	L"%ls ist auf Murmeln ausgerutscht!",
	L"%ls konnte %ls nicht greifen!",
	L"%ls hat %ls repariert",
	L"Unterbrechung für ",
	L"Ergeben?",
	L"Diese Person will keine Hilfe.",
	L"Lieber NICHT!",
	L"Wenn Sie zu Skyriders Heli wollen, müssen Sie Söldner einem FAHRZEUG/HELIKOPTER ZUWEISEN.",
	L"%ls hat nur Zeit, EINE Waffe zu laden",
	L"Spielzug Bloodcats",
};

const wchar_t *pTownNames[] =
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

wchar_t const* const g_towns_locative[] =
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
const wchar_t *sTimeStrings[] =
{
	L"Pause",
	L"Normal",
	L"5 Min",
	L"30 Min",
	L"60 Min",
	L"6 Std",
};

// Assignment Strings: what assignment does the merc have right now? For example, are they on a squad, training,
// administering medical aid (doctor) or training a town. All are abbreviated. 8 letters is the longest it can be.
const wchar_t *pAssignmentStrings[] =
{
	L"Trupp 1",
	L"Trupp 2",
	L"Trupp 3",
	L"Trupp 4",
	L"Trupp 5",
	L"Trupp 6",
	L"Trupp 7",
	L"Trupp 8",
	L"Trupp 9",
	L"Trupp 10",
	L"Trupp 11",
	L"Trupp 12",
	L"Trupp 13",
	L"Trupp 14",
	L"Trupp 15",
	L"Trupp 16",
	L"Trupp 17",
	L"Trupp 18",
	L"Trupp 19",
	L"Trupp 20",
	L"Dienst", // on active duty
	L"Doktor", // administering medical aid
	L"Patient", // getting medical aid
	L"Fahrzeug", // in a vehicle
	L"Transit", // in transit - abbreviated form
	L"Repar.", // repairing
	L"Üben", // training themselves
	L"Miliz", // training a town to revolt
	L"Trainer", // training a teammate
	L"Rekrut", // being trained by someone else
	L"Tot", // dead
	L"Unfähig", // abbreviation for incapacitated
	L"Gefangen", // Prisoner of war - captured
	L"Hospital", // patient in a hospital
	L"Leer",	//Vehicle is empty
};

const wchar_t *pMilitiaString[] =
{
	L"Miliz", // the title of the militia box
	L"Ohne Aufgabe", //the number of unassigned militia troops
	L"Mit Feinden im Sektor können Sie keine Miliz einsetzen!",
};

const wchar_t *pMilitiaButtonString[] =
{
	L"Autom.", // auto place the militia troops for the player
	L"Fertig", // done placing militia troops
};

const wchar_t *pConditionStrings[] =
{
	L"Sehr gut", //the state of a soldier .. excellent health
	L"Gut", // good health
	L"Mittel", // fair health
	L"Verwundet", // wounded health
	L"Erschöpft", // tired
	L"Verblutend", // bleeding to death
	L"Bewusstlos", // knocked out
	L"Stirbt", // near death
	L"Tot", // dead
};

const wchar_t *pEpcMenuStrings[] =
{
	L"Dienst", // set merc on active duty
	L"Patient", // set as a patient to receive medical aid
	L"Fahrzeug", // tell merc to enter vehicle
	L"Unbewacht", // let the escorted character go off on their own
	L"Abbrechen", // close this menu
};

// look at pAssignmentString above for comments
const wchar_t *pPersonnelAssignmentStrings[] =
{
	L"Trupp 1",
	L"Trupp 2",
	L"Trupp 3",
	L"Trupp 4",
	L"Trupp 5",
	L"Trupp 6",
	L"Trupp 7",
	L"Trupp 8",
	L"Trupp 9",
	L"Trupp 10",
	L"Trupp 11",
	L"Trupp 12",
	L"Trupp 13",
	L"Trupp 14",
	L"Trupp 15",
	L"Trupp 16",
	L"Trupp 17",
	L"Trupp 18",
	L"Trupp 19",
	L"Trupp 20",
	L"Dienst",
	L"Doktor",
	L"Patient",
	L"Fahrzeug",
	L"Transit",
	L"Repar.",
	L"Üben",
	L"Miliz",
	L"Trainer",
	L"Rekrut",
	L"Tot",
	L"Unfähig",
	L"Gefangen",
	L"Hospital",
	L"Leer",	// Vehicle is empty
};

// refer to above for comments
const wchar_t *pLongAssignmentStrings[] =
{
	L"Trupp 1",
	L"Trupp 2",
	L"Trupp 3",
	L"Trupp 4",
	L"Trupp 5",
	L"Trupp 6",
	L"Trupp 7",
	L"Trupp 8",
	L"Trupp 9",
	L"Trupp 10",
	L"Trupp 11",
	L"Trupp 12",
	L"Trupp 13",
	L"Trupp 14",
	L"Trupp 15",
	L"Trupp 16",
	L"Trupp 17",
	L"Trupp 18",
	L"Trupp 19",
	L"Trupp 20",
	L"Dienst",
	L"Doktor",
	L"Patient",
	L"Fahrzeug",
	L"Transit",
	L"Repar.",
	L"Üben",
	L"Miliz",
	L"Trainer",
	L"Rekrut",
	L"Tot",
	L"Unfähig",
	L"Gefangen",
	L"Hospital", // patient in a hospital
	L"Leer",	// Vehicle is empty
};

// the contract options
const wchar_t *pContractStrings[] =
{
	L"Vertragsoptionen:",
	L"", // a blank line, required
	L"Einen Tag anbieten", // offer merc a one day contract extension
	L"Eine Woche anbieten", // 1 week
	L"Zwei Wochen anbieten", // 2 week
	L"Entlassen", //end merc's contract (used to be "Terminate")
	L"Abbrechen", // stop showing this menu
};

const wchar_t *pPOWStrings[] =
{
	L"gefangen", //an acronym for Prisoner of War
	L"??",
};

const wchar_t *pInvPanelTitleStrings[] =
{
	L"Rüstung", // the armor rating of the merc
	L"Gewicht", // the weight the merc is carrying
	L"Tarnung", // the merc's camouflage rating
};

const wchar_t *pShortAttributeStrings[] =
{
	L"Bew", // the abbreviated version of : agility
	L"Ges", // dexterity
	L"Krf", // strength
	L"Fhr", // leadership
	L"Wsh", // wisdom
	L"Erf", // experience level
	L"Trf", // marksmanship skill
	L"Spr", // explosive skill
	L"Tec", // mechanical skill
	L"Med", // medical skill
};

const wchar_t *pUpperLeftMapScreenStrings[] =
{
	L"Aufgabe", // the mercs current assignment
	L"Gesundh.", // the health level of the current merc
	L"Moral", // the morale of the current merc
	L"Zustand",	// the condition of the current vehicle
};

const wchar_t *pTrainingStrings[] =
{
	L"Üben", // tell merc to train self
	L"Miliz", // tell merc to train town //
	L"Trainer", // tell merc to act as trainer
	L"Rekrut", // tell merc to be train by other
};

const wchar_t *pAssignMenuStrings[] =
{
	L"Dienst", // merc is on active duty
	L"Doktor", // the merc is acting as a doctor
	L"Patient", // the merc is receiving medical attention
	L"Fahrzeug", // the merc is in a vehicle
	L"Reparieren", // the merc is repairing items
	L"Training", // the merc is training
	L"Abbrechen", // cancel this menu
};

const wchar_t *pRemoveMercStrings[] ={
	L"Söldner entfernen", // remove dead merc from current team
	L"Abbrechen",
};

const wchar_t *pAttributeMenuStrings[] =
{
	L"Kraft",
	L"Geschicklichkeit",
	L"Beweglichkeit",
	L"Gesundheit",
	L"Treffsicherheit",
	L"Medizin",
	L"Technik",
	L"Führungsqualität",
	L"Sprengstoffe",
	L"Abbrechen",
};

const wchar_t *pTrainingMenuStrings[] =
{
 L"Üben", // train yourself
 L"Miliz", // train the town
 L"Trainer", // train your teammates
 L"Rekrut", // be trained by an instructor
 L"Abbrechen", // cancel this menu
};

const wchar_t *pSquadMenuStrings[] =
{
	L"Trupp  1",
	L"Trupp  2",
	L"Trupp  3",
	L"Trupp  4",
	L"Trupp  5",
	L"Trupp  6",
	L"Trupp  7",
	L"Trupp  8",
	L"Trupp  9",
	L"Trupp 10",
	L"Trupp 11",
	L"Trupp 12",
	L"Trupp 13",
	L"Trupp 14",
	L"Trupp 15",
	L"Trupp 16",
	L"Trupp 17",
	L"Trupp 18",
	L"Trupp 19",
	L"Trupp 20",
	L"Abbrechen",
};


const wchar_t *pPersonnelScreenStrings[] =
{
	L"Med. Versorgung:", // amount of medical deposit put down on the merc
	L"Laufzeit:", // time remaining on current contract
	L"Getötet", // number of kills by merc
	L"Mithilfe", // number of assists on kills by merc
	L"Tägliche Kosten:", // daily cost of merc
	L"Gesamtkosten:", // total cost of merc
	L"Vertrag:", // cost of current contract
	L"Diensttage:", // total service rendered by merc
	L"Schulden:", // amount left on MERC merc to be paid
	L"Prozentzahl Treffer:", // percentage of shots that hit target
	L"Einsätze", // number of battles fought
	L"Verwundungen", // number of times merc has been wounded
	L"Fähigkeiten:",
	L"Keine Fähigkeiten"
};


//These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
const wchar_t *gzMercSkillText[] =
{
	L"Keine Fähigkeiten",
	L"Schlösser knacken",
	L"Nahkampf",
	L"Elektronik",
	L"Nachteinsatz",
	L"Werfen",
	L"Lehren",
	L"Schwere Waffen",
	L"Autom. Waffen",
	L"Schleichen",
	L"Geschickt",
	L"Dieb",
	L"Kampfsport",
	L"Messer",
	L"Dach-Treffer-Bonus",
	L"Getarnt",
	L"(Experte)",
};

// This is pop up help text for the options that are available to the merc
const wchar_t *pTacticalPopupButtonStrings[] =
{
	L"|Stehen/Gehen",
	L"Kauern/Kauernd bewegen (|C)",
	L"Stehen/|Rennen",
	L"Hinlegen/Kriechen (|P)",
	L"Schauen (|L)",
	L"Action",
	L"Reden",
	L"Untersuchen (|S|t|r|g)",

	//Pop up door menu
	L"Manuell öffnen",
	L"Auf Fallen untersuchen",
	L"Dietrich",
	L"Mit Gewalt öffnen",
	L"Falle entschärfen",
	L"Abschließen",
	L"Aufschließen",
	L"Sprengstoff an Tür benutzen",
	L"Brecheisen benutzen",
	L"Rückgängig (|E|s|c)",
	L"Schließen",
};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.
const wchar_t *pDoorTrapStrings[] =
{
	L"keine Falle",
	L"eine Sprengstofffalle",
	L"eine elektrische Falle",
	L"eine Falle mit Sirene",
	L"eine Falle mit stummem Alarm",
};

// On the map screen, there are four columns. This text is popup help text that identifies the individual columns.
const wchar_t *pMapScreenMouseRegionHelpText[] =
{
	L"Charakter auswählen",
	L"Söldner einteilen",
	L"Marschroute",

	//The new 'c' key activates this option. Either reword this string to include a 'c' in it, or leave as is.
	L"Vertrag für Söldner (|c)",

	L"Söldner entfernen",
	L"Schlafen",
};

// volumes of noises
const wchar_t *pNoiseVolStr[] =
{
	L"LEISE",
	L"DEUTLICH",
	L"LAUT",
	L"SEHR LAUT",
};

// types of noises
const wchar_t *pNoiseTypeStr[] =
{
	L"EIN UNBEKANNTES GERÄUSCH",
	L"EINE BEWEGUNG",
	L"EIN KNARREN",
	L"EIN KLATSCHEN",
	L"EINEN AUFSCHLAG",
	L"EINEN SCHUSS",
	L"EINE EXPLOSION",
	L"EINEN SCHREI",
	L"EINEN AUFSCHLAG",
	L"EINEN AUFSCHLAG",
	L"EIN ZERBRECHEN",
	L"EIN ZERSCHMETTERN",
};

// Directions that are used throughout the code for identification.
const wchar_t *pDirectionStr[] =
{
	L"NORDOSTEN",
	L"OSTEN",
	L"SÜDOSTEN",
	L"SÜDEN",
	L"SÜDWESTEN",
	L"WESTEN",
	L"NORDWESTEN",
	L"NORDEN",
};

// These are the different terrain types.
const wchar_t *pLandTypeStrings[] =
{
	L"Stadt",
	L"Straße",
	L"Ebene",
	L"Wüste",
	L"Lichter Wald",
	L"Dichter Wald",
	L"Sumpf",
	L"See/Ozean",
	L"Hügel",
	L"Unpassierbar",
	L"Fluss",		//river from north to south
	L"Fluss",		//river from east to west
	L"Fremdes Land",
	//NONE of the following are used for directional travel, just for the sector description.
	L"Tropen",
	L"Farmland",
	L"Ebene, Straße",
	L"Wald, Straße",
	L"Farm, Straße",
	L"Tropen, Straße",
	L"Wald, Straße",
	L"Küste",
	L"Berge, Straße",
	L"Küste, Straße",
	L"Wüste, Straße",
	L"Sumpf, Straße",
	L"Wald, Raketen",
	L"Wüste, Raketen",
	L"Tropen, Raketen",
	L"Meduna, Raketen",

	//These are descriptions for special sectors
	L"Cambria Hospital",
	L"Drassen Flugplatz",
	L"Meduna Flugplatz",
	L"Raketen",
	L"Rebellenlager", //The rebel base underground in sector A10
	L"Tixa, Keller",	//The basement of the Tixa Prison (J9)
	L"Monsterhöhle",	//Any mine sector with creatures in it
	L"Orta, Keller",	//The basement of Orta (K4)
	L"Tunnel",				//The tunnel access from the maze garden in Meduna
										//leading to the secret shelter underneath the palace
	L"Bunker",				//The shelter underneath the queen's palace
	L"",							//Unused
};

const wchar_t *gpStrategicString[] =
{
	L"%ls wurden entdeckt in Sektor %c%d und ein weiterer Trupp wird gleich ankommen.",	//STR_DETECTED_SINGULAR
	L"%ls wurden entdeckt in Sektor %c%d und weitere Trupps werden gleich ankommen.",	//STR_DETECTED_PLURAL
	L"Gleichzeitige Ankunft koordinieren?",													//STR_COORDINATE

	//Dialog strings for enemies.

	L"Feind bietet die Chance zum Aufgeben an.",			//STR_ENEMY_SURRENDER_OFFER
	L"Feind hat restliche bewusstlose Söldner gefangen genommen.",	//STR_ENEMY_CAPTURED

	//The text that goes on the autoresolve buttons

	L"Rückzug", 		//The retreat button				//STR_AR_RETREAT_BUTTON
	L"Fertig",		//The done button				//STR_AR_DONE_BUTTON

	//The headers are for the autoresolve type (MUST BE UPPERCASE)

	L"VERTEIDIGUNG",								//STR_AR_DEFEND_HEADER
	L"ANGRIFF",									//STR_AR_ATTACK_HEADER
	L"BEGEGNUNG",									//STR_AR_ENCOUNTER_HEADER
	L"Sektor",		//The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

	//The battle ending conditions

	L"SIEG!",									//STR_AR_OVER_VICTORY
	L"NIEDERLAGE!",								//STR_AR_OVER_DEFEAT
	L"AUFGEGEBEN!",									//STR_AR_OVER_SURRENDERED
	L"GEFANGENGENOMMEN!",							//STR_AR_OVER_CAPTURED
	L"ZURÜCKGEZOGEN!",									//STR_AR_OVER_RETREATED

	//These are the labels for the different types of enemies we fight in autoresolve.

	L"Miliz",							//STR_AR_MILITIA_NAME,
	L"Elite",								//STR_AR_ELITE_NAME,
	L"Trupp",								//STR_AR_TROOP_NAME,
	L"Verwal",								//STR_AR_ADMINISTRATOR_NAME,
	L"Monster",								//STR_AR_CREATURE_NAME,

	//Label for the length of time the battle took

	L"Zeit verstrichen",							//STR_AR_TIME_ELAPSED,

	//Labels for status of merc if retreating. (UPPERCASE)

	L"HAT SICH ZURÜCKGEZOGEN",								//STR_AR_MERC_RETREATED,
	L"ZIEHT SICH ZURÜCK",								//STR_AR_MERC_RETREATING,
	L"RÜCKZUG",								//STR_AR_MERC_RETREAT,

	//PRE BATTLE INTERFACE STRINGS
	//Goes on the three buttons in the prebattle interface. The Auto resolve button represents
	//a system that automatically resolves the combat for the player without having to do anything.
	//These strings must be short (two lines -- 6-8 chars per line)

	L"PC Kampf",							//STR_PB_AUTORESOLVE_BTN,
	L"Gehe zu Sektor",							//STR_PB_GOTOSECTOR_BTN,
	L"Rückzug",							//STR_PB_RETREATMERCS_BTN,

	//The different headers(titles) for the prebattle interface.
	L"FEINDBEGEGNUNG",
	L"FEINDLICHE INVASION",
	L"FEINDLICHER HINTERHALT",
	L"FEINDLICHEN SEKTOR BETRETEN",
	L"MONSTERANGRIFF",
	L"BLOODCAT-HINTERHALT",
	L"BLOODCAT-HÖHLE BETRETEN",

	//Various single words for direct translation. The Civilians represent the civilian
	//militia occupying the sector being attacked. Limited to 9-10 chars

	L"Ort",
	L"Feinde",
	L"Söldner",
	L"Miliz",
	L"Monster",
	L"Bloodcats",
	L"Sektor",
	L"Keine",		//If there are no uninvolved mercs in this fight.
	L"n.a.",			//Acronym of Not Applicable
	L"T",			//One letter abbreviation of day
	L"h",			//One letter abbreviation of hour

	//TACTICAL PLACEMENT USER INTERFACE STRINGS
	//The four buttons

	L"Räumen",
	L"Verteilen",
	L"Gruppieren",
	L"Fertig",

	//The help text for the four buttons. Use \n to denote new line (just like enter).

	L"Söldner räumen ihre Positionen und\nkönnen manuell neu platziert werden. (|C)",
	L"Söldner |schwärmen in alle Richtungen\naus wenn der Button gedrückt wird.",
	L"Mit diesem Button können Sie wählen, wo die Söldner |gruppiert werden sollen.",
	L"Klicken Sie auf diesen Button, wenn Sie die\nPositionen der Söldner gewählt haben. (|E|i|n|g|a|b|e)",
	L"Sie müssen alle Söldner positionieren\nbevor die Schlacht beginnt.",

	//Various strings (translate word for word)

	L"Sektor",
	L"Eintrittspunkte wählen",

	//Strings used for various popup message boxes. Can be as long as desired.

	L"Das sieht nicht gut aus. Gelände ist unzugänglich. Versuchen Sie es an einer anderen Stelle.",
	L"Platzieren Sie Ihre Söldner in den markierten Sektor auf der Karte.",

	//These entries are for button popup help text for the prebattle interface. All popup help
	//text supports the use of \n to denote new line. Do not use spaces before or after the \n.
	L"Entscheidet Schlacht |automatisch für Sie\nohne Karte zu laden.",
	L"Sie können den PC-Kampf-Modus nicht benutzen, während Sie\neinen vom Feind verteidigten Ort angreifen.",
	L"Sektor b|etreten und Feind in Kampf verwickeln.",
	L"Gruppe zum vorigen Sektor zu|rückziehen.",				//singular version
	L"Alle Gruppen zum vorigen Sektor zu|rückziehen.", //multiple groups with same previous sector

	//various popup messages for battle conditions.

	//%c%d is the sector -- ex: A9
	L"Feinde attackieren Ihre Miliz im Sektor %c%d.",
	//%c%d is the sector -- ex: A9
	L"Monster attackieren Ihre Miliz im Sektor %c%d.",
	//1st %d refers to the number of civilians eaten by monsters, %c%d is the sector -- ex: A9
	//Note: the minimum number of civilians eaten will be two.
	L"Monster attackieren und töten %d Zivilisten im Sektor %ls.",
	//%s is the sector -- ex: A9
	L"Feinde attackieren Ihre Söldner im Sektor %ls. Alle Söldner sind bewusstlos!",
	//%s is the sector -- ex: A9
	L"Monster attackieren Ihre Söldner im Sektor %ls. Alle Söldner sind bewusstlos!",

};

//This is the day represented in the game clock. Must be very short, 4 characters max.
const wchar_t gpGameClockString[] = L"Tag";

//When the merc finds a key, they can get a description of it which
//tells them where and when they found it.
const wchar_t *sKeyDescriptionStrings[2]=
{
	L"gefunden im Sektor:",
	L"gefunden am:",
};

//The headers used to describe various weapon statistics.
const wchar_t* gWeaponStatsDesc[] =
{
	L"Gewicht (%ls):", //weight
	L"Status:",
	L"Anzahl:", 		// Number of bullets left in a magazine
	L"Reichw.:",		// Range
	L"Schaden:",
	L"AP:",			// abbreviation for Action Points
	L"="
};

//The headers used for the merc's money.
const wchar_t gMoneyStatsDesc[][ 13 ] =
{
	L"Betrag",
	L"Verbleibend:", //this is the overall balance
	L"Betrag",
	L"Abteilen:", // the amount he wants to separate from the overall balance to get two piles of money
	L"Konto",
	L"Saldo",
	L"Betrag",
	L"abheben",
};

//The health of various creatures, enemies, characters in the game. The numbers following each are for comment
//only, but represent the precentage of points remaining.
const wchar_t zHealthStr[][13] =	//used to be 10
{
	L"STIRBT",		//	>= 0
	L"KRITISCH", 	//	>= 15
	L"SCHLECHT",	//	>= 30
	L"VERWUNDET",	//	>= 45
	L"GESUND",		//	>= 60
	L"STARK",			// 	>= 75
	L"SEHR GUT",	// 	>= 90
};

const wchar_t *gzMoneyAmounts[6] =
{
	L"$1000",
	L"$100",
	L"$10",
	L"OK",
	L"Abteilen",
	L"Abheben",
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
const wchar_t gzProsLabel[] = L"Pro:";
const wchar_t gzConsLabel[] = L"Kontra:";

//Conversation options a player has when encountering an NPC
const wchar_t zTalkMenuStrings[6][ SMALL_STRING_LENGTH ] =
{
	L"Wie bitte?", 	//meaning "Repeat yourself"
	L"Freundlich",		//approach in a friendly
	L"Direkt",		//approach directly - let's get down to business
	L"Drohen",		//approach threateningly - talk now, or I'll blow your face off
	L"Geben",
	L"Rekrutieren",
};

//Some NPCs buy, sell or repair items. These different options are available for those NPCs as well.
const wchar_t zDealerStrings[4][ SMALL_STRING_LENGTH ]=
{
	L"Handeln",
	L"Kaufen",
	L"Verkaufen",
	L"Reparieren",
};

const wchar_t zDialogActions[] = L"Fertig";

const wchar_t *pVehicleStrings[] =
{
	L"Eldorado",
	L"Hummer", // a hummer jeep/truck -- military vehicle
	L"Ice Cream Truck",
	L"Jeep",
	L"Tank",
	L"Helikopter",
};

const wchar_t *pShortVehicleStrings[] =
{
	L"Eldor.",
	L"Hummer",			// the HMVV
	L"Truck",
	L"Jeep",
	L"Tank",
	L"Heli", 				// the helicopter
};

const wchar_t *zVehicleName[] =
{
	L"Eldorado",
	L"Hummer",		//a military jeep. This is a brand name.
	L"Truck",			// Ice cream truck
	L"Jeep",
	L"Tank",
	L"Heli", 		//an abbreviation for Helicopter
};

//These are messages Used in the Tactical Screen
const wchar_t* const TacticalStr[] =
{
	L"Luftangriff",
	L"Automatisch Erste Hilfe leisten?",

	// CAMFIELD NUKE THIS and add quote #66.

	L"%ls bemerkt, dass Teile aus der Lieferung fehlen.",

	// The %s is a string from pDoorTrapStrings

	L"Das Schloss hat %ls.",
	L"Es gibt kein Schloss.",
	L"Erfolg!",
	L"Fehlschlag.",
	L"Das Schloss hat keine Falle.",
	L"Erfolg!",
	// The %s is a merc name
	L"%ls hat nicht den richtigen Schlüssel.",
	L"Die Falle am Schloss ist entschärft.",
	L"Das Schloss hat keine Falle.",
	L"Geschl.",
	L"TÜR",
	L"FALLE AN",
	L"Geschl.",
	L"GEÖFFNET",
	L"EINGETRETEN",
	L"Hier ist ein Schalter. Betätigen?",
	L"Falle entschärfen?",
	L"Mehr...",

	// In the next 2 strings, %s is an item name

	L"%ls liegt jetzt auf dem Boden.",
	L"%ls ist jetzt bei %ls.",

	// In the next 2 strings, %s is a name

	L"%ls hat den vollen Betrag erhalten.",
	L"%ls bekommt noch %d.",
	L"Detonationsfrequenz auswählen:",		//in this case, frequency refers to a radio signal
	L"Wie viele Züge bis zur Explosion:",	//how much time, in turns, until the bomb blows
	L"Ferngesteuerte Zündung einstellen:",//in this case, frequency refers to a radio signal
	L"Falle entschärfen?",
	L"Blaue Flagge wegnehmen?",
	L"Blaue Flagge hier aufstellen?",
	L"Zug beenden",

	// In the next string, %s is a name. Stance refers to way they are standing.

	L"Wollen Sie %ls wirklich angreifen?",
	L"Fahrzeuge können ihre Position nicht ändern.",
	L"Der Roboter kann seine Position nicht ändern.",

	// In the next 3 strings, %s is a name

	//%s can't change to that stance here
	L"%ls kann die Haltung hier nicht ändern.",

	L"%ls kann hier nicht versorgt werden.",
	L"%ls braucht keine Erste Hilfe.",
	L"Kann nicht dorthin gehen.",
	L"Ihr Team ist komplett. Kein Platz mehr für Rekruten.",	//there's no room for a recruit on the player's team

	// In the next string, %s is a name

	L"%ls wurde rekrutiert.",

	// Here %s is a name and %d is a number

	L"%ls bekommt noch %d $.",

	// In the next string, %s is a name

	L"%ls eskortieren?",

	// In the next string, the first %s is a name and the second %s is an amount of money (including $ sign)

	L"%ls für %ls pro Tag anheuern?",

	// This line is used repeatedly to ask player if they wish to participate in a boxing match.

	L"Kämpfen?",

	// In the next string, the first %s is an item name and the
	// second %s is an amount of money (including $ sign)

	L"%ls für %ls kaufen?",

	// In the next string, %s is a name

	L"%ls wird von Trupp %d eskortiert.",

	// These messages are displayed during play to alert the player to a particular situation

	L"KLEMMT",					//weapon is jammed.
	L"Roboter braucht %ls Munition.",		//Robot is out of ammo
	L"Dorthin werfen? Unmöglich.",		//Merc can't throw to the destination he selected

	// These are different buttons that the player can turn on and off.

	L"Stealth Mode (|Z)",
	L"Kartenbildschir|m",
	L"Spielzug been|den",
	L"Sprechen",
	L"Stumm",
	L"Aufrichten (|B|i|l|d |h|o|c|h)",
	L"Cursor Level (|T|a|b|u|l|a|t|o|r)",
	L"Klettern / Springen",
	L"Ducken (|B|i|l|d |r|u|n|t|e|r)",
	L"Untersuchen (|S|t|r|g)",
	L"Voriger Söldner",
	L"Nächster Söldner (|L|e|e|r|t|a|s|t|e)",
	L"|Optionen",
	L"Feuerstoß (|B)",
	L"B|lickrichtung",
	L"Gesundheit: %d/%d\nEnergie: %d/%d\nMoral: %ls",
	L"Was?",					//this means "what?"
	L"Forts.",					//an abbrieviation for "Continued"
	L"Stumm aus für %ls.",
	L"Stumm für %ls.", //
	L"Fahrer",
	L"Fahrzeug verlassen",
	L"Trupp wechseln",
	L"Fahren",
	L"n.a.",						//this is an acronym for "Not Applicable."
	L"Benutzen ( Faustkampf )",
	L"Benutzen ( Feuerwaffe )",
	L"Benutzen ( Hieb-/Stichwaffe )",
	L"Benutzen ( Sprengstoff )",
	L"Benutzen ( Verbandskasten )",
	L"(Fangen)",
	L"(Nachladen)",
	L"(Geben)",
	L"%ls Falle wurde ausgelöst.",
	L"%ls ist angekommen.",
	L"%ls hat keine Action-Punkte mehr.",
	L"%ls ist nicht verfügbar.",
	L"%ls ist fertig verbunden.",
	L"%ls sind die Verbände ausgegangen.",
	L"Feind im Sektor!",
	L"Keine Feinde in Sicht.",
	L"Nicht genug Action-Punkte.",
	L"Niemand bedient die Fernbedienung.",
	L"Feuerstoß hat Magazin geleert!",
	L"SOLDAT",
	L"MONSTER",
	L"MILIZ",
	L"ZIVILIST",
	L"Sektor verlassen",
	L"OK",
	L"Abbruch",
	L"Gewählter Söldner",
	L"Ganzer Trupp",
	L"Gehe zu Sektor",

	L"Gehe zu Karte",

	L"Sie können den Sektor von dieser Seite aus nicht verlassen.",
	L"%ls ist zu weit weg.",
	L"Baumkronen entfernen",
	L"Baumkronen zeigen",
	L"KRÄHE",				//Crow, as in the large black bird
	L"NACKEN",
	L"KOPF",
	L"TORSO",
	L"BEINE",
	L"Der Herrin sagen, was sie wissen will?",
	L"Fingerabdruck-ID gespeichert",
	L"Falsche Fingerabdruck-ID. Waffe außer Betrieb",
	L"Ziel erfasst",
	L"Weg blockiert",
	L"Geld einzahlen/abheben",		//Help text over the $ button on the Single Merc Panel
	L"Niemand braucht Erste Hilfe.",
	L"Klemmt.",			//Short form of JAMMED, for small inv slots
	L"Kann da nicht hin.",					// used ( now ) for when we click on a cliff
	L"Person will sich nicht bewegen",
	// In the following message, '%s' would be replaced with a quantity of money (e.g. $200)
	L"Mit der Zahlung von %ls einverstanden?",
	L"Gratisbehandlung akzeptieren?",
	L"Daryl heiraten?",
	L"Schlüsselring",
	L"Das ist mit einem EPC nicht möglich.",
	L"Krott verschonen?",
	L"Außer Reichweite",
	L"Arbeiter",	//People that work in mines to extract precious metals
	L"Fahrzeug kann nur zwischen Sektoren fahren",
	L"Automatische Erste Hilfe nicht möglich",
	L"Weg blockiert für %ls",
	L"Ihre von Deidrannas Truppe gefangenen Soldaten sind hier inhaftiert",
	L"Schloss getroffen",
	L"Schloss zerstört",
	L"Noch jemand an der Tür.",
	L"Gesundh.: %d/%d\nTank: %d/%d",
	L"%ls kann %ls nicht sehen.", // Cannot see person trying to talk to
};

//Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different circumstances in the "exiting sector" interface.
const wchar_t *pExitingSectorHelpText[] =
{
	//Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box is checked.
	L"Der nächste Sektor wird sofort geladen, wenn Sie das Kästchen aktivieren.",
	L"Sie kommen sofort zum Kartenbildschirm, wenn Sie das Kästchen aktivieren\nweil die Reise Zeit braucht.",

	//If you attempt to leave a sector when you have multiple squads in a hostile sector.
	L"Der Sektor ist von Feinden besetzt. Sie können keine Söldner hier lassen.\nRegeln Sie das, bevor Sie neue Sektoren laden.",

	//Because you only have one squad in the sector, and the "move all" option is checked, the "go to sector" option is locked to on.
	//The helptext explains why it is locked.
	L"Wenn die restlichen Söldner den Sektor verlassen,\nwird sofort der nächste Sektor geladen.",
	L"Wenn die restlichen Söldner den Sektor verlassen,\nkommen Sie sofort zum Kartenbildschirm\nweil die Reise Zeit braucht.",

	//If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being escorted. The "single" button is disabled.
	L"%ls kann den Sektor nicht ohne Eskorte verlassen.",

	//If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc will be prohibited from leaving alone.
	//There are several strings depending on the gender of the merc and how many EPCs are in the squad.
	//DO NOT USE THE NEWLINE HERE AS IT IS USED FOR BOTH HELPTEXT AND SCREEN MESSAGES!
	L"%ls kann den Sektor nicht verlassen, weil er %ls eskortiert.", //male singular
	L"%ls kann den Sektor nicht verlassen, weil sie %ls eskortiert.", //female singular
	L"%ls kann den Sektor nicht verlassen, weil er mehrere Personen eskortiert.", //male plural
	L"%ls kann den Sektor nicht verlassen, weil sie mehrere Personen eskortiert.", //female plural

	//If one or more of your mercs in the selected squad aren't in range of the traversal area, then the "move all" option is disabled,
	//and this helptext explains why.
	L"Alle Söldner müssen in der Nähe sein,\ndamit der Trupp weiterreisen kann.",

	//Standard helptext for single movement. Explains what will happen (splitting the squad)
	L"Bei aktiviertem Kästchen reist %ls alleine und\nbildet automatisch wieder einen Trupp.",

	//Standard helptext for all movement. Explains what will happen (moving the squad)
	L"Bei aktiviertem Kästchen reist der ausgewählte Trupp\nweiter und verlässt den Sektor.",

	//This strings is used BEFORE the "exiting sector" interface is created. If you have an EPC selected and you attempt to tactically
	//traverse the EPC while the escorting mercs aren't near enough (or dead, dying, or unconscious), this message will appear and the
	//"exiting sector" interface will not appear. This is just like the situation where
	//This string is special, as it is not used as helptext. Do not use the special newline character (\n) for this string.
	L"%ls wird von Söldnern eskortiert und kann den Sektor nicht alleine verlassen. Die anderen Söldner müssen in der Nähe sein.",
};

const wchar_t *pRepairStrings[] =
{
	L"Gegenstände", 		// tell merc to repair items in inventory
	L"Raketenstützpunkt", 		// tell merc to repair SAM site - SAM is an acronym for Surface to Air Missile
	L"Abbruch", 		// cancel this menu
	L"Roboter", 		// repair the robot
};

// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."
const wchar_t *sPreStatBuildString[] =
{
	L"verliert", 		// the merc has lost a statistic
	L"gewinnt", 		// the merc has gained a statistic
	L"Punkt",	// singular
	L"Punkte",	// plural
	L"Level",	// singular
	L"Level",	// plural
};

const wchar_t *sStatGainStrings[] =
{
	L"Gesundheit.",
	L"Beweglichkeit",
	L"Geschicklichkeit",
	L"Weisheit.",
	L"an Medizin.",
	L"an Sprengstoff.",
	L"an Technik.",
	L"an Treffsicherheit.",
	L"Erfahrungsstufe(n).",
	L"Kraft.",
	L"Führungsqualität.",
};

const wchar_t *pHelicopterEtaStrings[] =
{
	L"Gesamt: ", 			// total distance for helicopter to travel
	L" Sicher: ", 		// Number of safe sectors
	L" Unsicher:", 		// Number of unsafe sectors
	L"Gesamtkosten: ",// total cost of trip by helicopter
	L"Ank.: ", 				// ETA is an acronym for "estimated time of arrival"

	// warning that the sector the helicopter is going to use for refueling is under enemy control
	L"Helikopter hat fast keinen Sprit mehr und muss im feindlichen Gebiet landen.",
	L"Passagiere: ",
	L"Skyrider oder Absprungsort auswählen?",
	L"Skyrider",
	L"Absprung", //make sure length doesn't exceed 8 characters (used to be "Absprungsort")
};

const wchar_t sMapLevelString[] = L"Ebene:"; // what level below the ground is the player viewing in mapscreen

const wchar_t gsLoyalString[] = L"%d%% Loyalität"; // the loyalty rating of a town ie : Loyal 53%


// error message for when player is trying to give a merc a travel order while he's underground.
const wchar_t gsUndergroundString[] = L"Ich kann unter der Erde keinen Marschbefehl empfangen.";

const wchar_t *gsTimeStrings[] =
{
	L"h",				// hours abbreviation
	L"m",				// minutes abbreviation
	L"s",				// seconds abbreviation
	L"T",				// days abbreviation
};

// text for the various facilities in the sector
const wchar_t *sFacilitiesStrings[] =
{
	L"Keine",
	L"Hospital",
	L"Fabrik",
	L"Gefängnis",
	L"Militär",
	L"Flughafen",
	L"Schießstand", // a field for soldiers to practise their shooting skills
};

// text for inventory pop up button
const wchar_t *pMapPopUpInventoryText[] =
{
	L"Inventar",
	L"Exit",
};

// town strings
const wchar_t *pwTownInfoStrings[] =
{
	L"Größe",					// 0 // size of the town in sectors
	L"", 						// blank line, required
	L"unter Kontrolle", 					// how much of town is controlled
	L"Keine", 					// none of this town
	L"Mine", 				// mine associated with this town
	L"Loyalität",					// 5 // the loyalty level of this town
	L"Trainiert", 					// the forces in the town trained by the player
	L"",
	L"Wichtigste Gebäude", 				// main facilities in this town
	L"Level", 					// the training level of civilians in this town
	L"Zivilistentraining",				// 10 // state of civilian training in town
	L"Miliz", 					// the state of the trained civilians in the town
};

// Mine strings
const wchar_t *pwMineStrings[] =
{
	L"Mine",						// 0
	L"Silber",
	L"Gold",
	L"Tagesproduktion",
	L"Maximale Produktion",
	L"Aufgegeben",				// 5
	L"Geschlossen",
	L"Fast erschöpft",
	L"Produziert",
	L"Status",
	L"Produktionsrate",
	L"Erzart",				// 10
	L"Kontrolle über Stadt",
	L"Loyalität der Stadt",
//	L"Minenarbeiter",
};

// blank sector strings
const wchar_t *pwMiscSectorStrings[] =
{
	L"Feindliche Verbände",
	L"Sektor",
	L"# der Gegenstände",
	L"Unbekannt",
	L"Kontrolliert",
	L"Ja",
	L"Nein",
};

// error strings for inventory
const wchar_t *pMapInventoryErrorString[] =
{
	L"Diesen Söldner können Sie nicht auswählen.",
	L"%ls ist nicht im Sektor.",
	L"Während einer Schlacht müssen Sie Gegenstände manuell nehmen.",
	L"Während einer Schlacht müssen Sie Gegenstände manuell fallenlassen.",
	L"%ls ist nicht im Sektor und kann Gegenstand nicht fallen lassen.",
};

const wchar_t *pMapInventoryStrings[] =
{
	L"Ort", 			// sector these items are in
	L"Zahl der Gegenstände", 		// total number of items in sector
};

// movement menu text
const wchar_t *pMovementMenuStrings[] =
{
	L"Söldner in Sektor %ls bewegen", 	// title for movement box
	L"Route planen", 		// done with movement menu, start plotting movement
	L"Abbruch", 	// cancel this menu
	L"Andere",			// title for group of mercs not on squads nor in vehicles
};

const wchar_t *pUpdateMercStrings[] =
{
	L"Ups:", 			// an error has occured
	L"Vertrag ist abgelaufen:", 	// this pop up came up due to a merc contract ending
	L"Auftrag wurde ausgeführt:", // this pop up....due to more than one merc finishing assignments
	L"Diese Söldner arbeiten wieder:", // this pop up ....due to more than one merc waking up and returing to work
	L"Diese Söldner schlafen:", // this pop up ....due to more than one merc being tired and going to sleep
	L"Vertrag bald abgelaufen:", 	//this pop up came up due to a merc contract ending
};

// map screen map border buttons help text
const wchar_t *pMapScreenBorderButtonHelpText[] =
{
	L"Städte zeigen (|W)",
	L"|Minen zeigen",
	L"|Teams & Feinde zeigen",
	L"Luftr|aum zeigen",
	L"Gegenstände zeigen (|I)",
	L"Miliz & Feinde zeigen (|Z)",
};

const wchar_t *pMapScreenBottomFastHelp[] =
{
	L"|Laptop",
	L"Taktik (|E|s|c)",
	L"|Optionen",
	L"Zeitraffer (|+)", 	// time compress more
	L"Zeitraffer (|-)", 	// time compress less
	L"Vorige Nachricht (|P|f|e|i|l |h|o|c|h)\nSeite zurück (|B|i|l|d |h|o|c|h)", 	// previous message in scrollable list
	L"Nächste Nachricht (|P|f|e|i|l |r|u|n|t|e|r)\nNächste Seite (|B|i|l|d |r|u|n|t|e|r)", 	// next message in the scrollable list
	L"Zeit Start/Stop (|L|e|e|r|t|a|s|t|e)",	// start/stop time compression
};

const wchar_t pMapScreenBottomText[] = L"Kontostand"; // current balance in player bank account

const wchar_t pMercDeadString[] = L"%ls ist tot.";


const wchar_t pDayStrings[] = L"Tag";

// the list of email sender names
const wchar_t *pSenderNameList[] =
{
	L"Enrico",
	L"Psych Pro Inc.",
	L"Online-Hilfe",
	L"Psych Pro Inc.",
	L"Speck",
	L"R.I.S.",
	L"Barry",
	L"Blood",
	L"Lynx",
	L"Grizzly",
	L"Vicki",
	L"Trevor",
	L"Grunty",
	L"Ivan",
	L"Steroid",
	L"Igor",
	L"Shadow",
	L"Red",
	L"Reaper",
	L"Fidel",
	L"Fox",
	L"Sidney",
	L"Gus",
	L"Buns",
	L"Ice",
	L"Spider",
	L"Cliff",
	L"Bull",
	L"Hitman",
	L"Buzz",
	L"Raider",
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
	L"H, A & S Versicherung",
	L"Bobby Rays",
	L"Kingpin",
	L"John Kulba",
	L"A.I.M.",
};


// new mail notify string
const wchar_t pNewMailStrings[] = L"Sie haben neue Mails...";


// confirm player's intent to delete messages
const wchar_t *pDeleteMailStrings[] =
{
	L"Mail löschen?",
	L"UNGELESENE Mail löschen?",
};

// the sort header strings
const wchar_t *pEmailHeaders[] =
{
	L"Absender:",
	L"Betreff:",
	L"Datum:",
};

// email titlebar text
const wchar_t pEmailTitleText[] = L"Mailbox";

// the financial screen strings
const wchar_t pFinanceTitle[] = L"Buchhalter Plus"; // the name we made up for the financial program in the game

const wchar_t *pFinanceSummary[] =
{
	L"Haben:", 				//the credits column (to ADD money to your account)
	L"Soll:", 				//the debits column (to SUBTRACT money from your account)
	L"Einkünfte vom Vortag:",
	L"Sonstige Einzahlungen vom Vortag:",
	L"Haben vom Vortag:",
	L"Kontostand Ende des Tages:",
	L"Tagessatz:",
	L"Sonstige Einzahlungen von heute:",
	L"Haben von heute:",
	L"Kontostand:",
	L"Voraussichtliche Einkünfte:",
	L"Prognostizierter Kontostand:", 		// projected balance for player for tommorow
};

// headers to each list in financial screen
const wchar_t *pFinanceHeaders[] =
{
	L"Tag", 					// the day column
	L"Haben", 				//the credits column (to ADD money to your account)
	L"Soll", 					//the debits column (to SUBTRACT money from your account)
	L"Kontobewegungen", 			// transaction type - see TransactionText below
	L"Kontostand", 				// balance at this point in time
	L"Seite", 				// page number
	L"Tag(e)", 				// the day(s) of transactions this page displays
};

const wchar_t *pTransactionText[] =
{
	L"Aufgelaufene Zinsen",			// interest the player has accumulated so far
	L"Anonyme Einzahlung",
	L"Bearbeitungsgebühr",
	L"%ls von AIM angeheuert", // Merc was hired
	L"Kauf bei Bobby Rays",	 		// Bobby Ray is the name of an arms dealer
	L"Ausgeglichene Konten bei M.E.R.C.",
	L"Krankenversicherung für %ls", 		// medical deposit for merc
	L"BSE-Profilanalyse", 		// IMP is the acronym for International Mercenary Profiling
	L"Versicherung für %ls abgeschlossen",
	L"Versicherung für %ls verringert",
	L"Versicherung für %ls verlängert", 				// johnny contract extended
	L"Versicherung für %ls gekündigt",
	L"Versicherungsanspruch für %ls", 		// insurance claim for merc
	L"%lss Vertrag verl. um 1 Tag", 				// entend mercs contract by a day
	L"%lss Vertrag verl. um 1 Woche",
	L"%lss Vertrag verl. um 2 Wochen",
	L"Minenertrag",
	L"",
	L"Blumen kaufen",
	L"Volle Rückzahlung für %ls",
	L"Teilw. Rückzahlung für %ls",
	L"Keine Rückzahlung für %ls",
	L"Zahlung an %ls",		// %s is the name of the npc being paid
	L"Überweisen an %ls", 			// transfer funds to a merc
	L"Überweisen von %ls", 		// transfer funds from a merc
	L"Miliz in %ls ausbilden", // initial cost to equip a town's militia
	L"Gegenstände von %ls gekauft.",	//is used for the Shop keeper interface. The dealers name will be appended to the end of the string.
	L"%ls hat Geld angelegt.",
};

// helicopter pilot payment
const wchar_t *pSkyriderText[] =
{
	L"Skyrider wurden $%d gezahlt", 			// skyrider was paid an amount of money
	L"Skyrider bekommt noch $%d", 		// skyrider is still owed an amount of money
	L"Skyrider hat keine Passagiere. Wenn Sie Söldner in den Sektor transportieren wollen, weisen Sie sie einem Fahrzeug/Helikopter zu.",
};

// strings for different levels of merc morale
const wchar_t *pMoralStrings[] =
{
	L"Super",
	L"Gut",
	L"Stabil",
	L"Schlecht",
	L"Panik",
	L"Mies",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.
const wchar_t str_left_equipment[]   = L"%lss Ausrüstung ist in %ls (%c%d) angekommen.";

// Status that appears on the Map Screen
const wchar_t *pMapScreenStatusStrings[] =
{
	L"Gesundheit",
	L"Energie",
	L"Moral",
	L"Zustand",	// the condition of the current vehicle (its "health")
	L"Tank",	// the fuel level of the current vehicle (its "energy")
};

const wchar_t *pMapScreenPrevNextCharButtonHelpText[] =
{
	L"Voriger Söldner (|P|f|e|i|l |l|i|n|k|s)", 			// previous merc in the list
	L"Nächster Söldner (|P|f|e|i|l |r|e|c|h|t|s)", 				// next merc in the list
};

const wchar_t pEtaString[] = L"Ank.:"; // eta is an acronym for Estimated Time of Arrival

const wchar_t *pTrashItemText[] =
{
	L"Sie werden das Ding nie wiedersehen. Trotzdem wegwerfen?", 	// do you want to continue and lose the item forever
	L"Dieser Gegenstand sieht SEHR wichtig aus. Sie sie GANZ SICHER, dass Sie ihn wegwerfen wollen?", // does the user REALLY want to trash this item
};

const wchar_t *pMapErrorString[] =
{
	L"Trupp kann nicht reisen, wenn einer schläft.",

//1-5
	L"Wir müssen erst an die Oberfläche.",
	L"Marschbefehl? Wir sind in einem feindlichen Sektor!",
	L"Wenn Söldner reisen sollen, müssen sie einem Trupp oder Fahrzeug zugewiesen werden.",
	L"Sie haben noch keine Teammitglieder.", 		// you have no members, can't do anything
	L"Söldner kann nicht gehorchen.",			 		// merc can't comply with your order
//6-10
	L"%ls braucht eine Eskorte. Platzieren Sie ihn in einem Trupp mit Eskorte.", // merc can't move unescorted .. for a male
	L"%ls braucht eine Eskorte. Platzieren Sie sie in einem Trupp mit Eskorte.", // for a female
	L"Söldner ist noch nicht in Arulco!",
	L"Erst mal Vertrag aushandeln!",
	L"",
//11-15
	L"Marschbefehl? Hier tobt ein Kampf!",
	L"Sie sind von Bloodcats umstellt in Sektor %ls!",
	L"Sie haben gerade eine Bloodcat-Höhle betreten in Sektor I16!",
	L"",
	L"Raketenstützpunkt in %ls wurde erobert.",
//16-20
	L"Mine in %ls wurde erobert. Ihre Tageseinnahmen wurden reduziert auf %ls.",
	L"Gegner hat Sektor %ls ohne Gegenwehr erobert.",
	L"Mindestens ein Söldner konnte nicht eingeteilt werden.",
	L"%ls konnte sich nicht anschließen, weil %ls voll ist",
	L"%ls konnte sich %ls nicht anschließen, weil er zu weit weg ist.",
//21-25
	L"Die Mine in %ls ist von Deidrannas Truppen erobert worden!",
	L"Deidrannas Truppen sind gerade in den Raketenstützpunkt in %ls eingedrungen",
	L"Deidrannas Truppen sind gerade in %ls eingedrungen",
	L"Deidrannas Truppen wurden gerade in %ls gesichtet.",
	L"Deidrannas Truppen haben gerade %ls erobert.",
//26-30
	L"Mindestens ein Söldner kann nicht schlafen.",
	L"Mindestens ein Söldner ist noch nicht wach.",
	L"Die Miliz kommt erst, wenn das Training beendet ist.",
	L"%ls kann im Moment keine Marschbefehle erhalten.",
	L"Milizen außerhalb der Stadtgrenzen können nicht in andere Sektoren reisen.",
//31-35
	L"Sie können keine Milizen in %ls haben.",
	L"Leere Fahrzeuge fahren nicht!",
	L"%ls ist nicht transportfähig!",
	L"Sie müssen erst das Museum verlassen!",
	L"%ls ist tot!",
//36-40
	L"%ls kann nicht zu %ls wechseln, weil der sich bewegt",
	L"%ls kann so nicht einsteigen",
	L"%ls kann sich nicht %ls anschließen",
	L"Sie können den Zeitraffer erst mit neuen Söldner benutzen!",
	L"Dieses Fahrzeug kann nur auf Straßen fahren!",
//41-45
	L"Reisenden Söldnern können Sie keine Aufträge erteilen.",
	L"Kein Benzin mehr!",
	L"%ls ist zu müde.",
	L"Keiner kann das Fahrzeug steuern.",
	L"Ein oder mehrere Söldner dieses Trupps können sich jetzt nicht bewegen.",
//46-50
	L"Ein oder mehrere Söldner des ANDEREN Trupps kann sich gerade nicht bewegen.",
	L"Fahrzeug zu stark beschädigt!",
	L"Nur zwei Söldner pro Sektor können Milizen trainieren.",
	L"Roboter muss von jemandem bedient werden. Beide im selben Trupp platzieren.",

};

// help text used during strategic route plotting
const wchar_t *pMapPlotStrings[] =
{
	L"Klicken Sie noch einmal auf das Ziel, um die Route zu bestätigen. Klicken Sie auf andere Sektoren, um die Route zu ändern.",
	L"Route bestätigt.",
	L"Ziel unverändert.",
	L"Route geändert.",
	L"Route verkürzt.",
};

// help text used when moving the merc arrival sector
const wchar_t *pBullseyeStrings[] =
{
	L"Klicken Sie auf den Sektor, in dem die Söldner statt dessen ankommen sollen.",
	L"OK. Söldner werden in %ls abgesetzt",
	L"Söldner können nicht dorthin fliegen. Luftraum nicht gesichert!",
	L"Abbruch. Ankunftssektor unverändert,",
	L"Luftraum über %ls ist nicht mehr sicher! Ankunftssektor jetzt in %ls.",
};

// help text for mouse regions
const wchar_t *pMiscMapScreenMouseRegionHelpText[] =
{
	L"Ins Inventar gehen (|E|i|n|g|a|b|e)",
	L"Gegenstand wegwerfen",
	L"Inventar verlassen (|E|i|n|g|a|b|e)",
};

const wchar_t str_he_leaves_where_drop_equipment[]  = L"Soll %ls seine Ausrüstung hier lassen (%ls) oder in %ls (%ls), bevor er Arulco verlässt?";
const wchar_t str_she_leaves_where_drop_equipment[] = L"Soll %ls ihre Ausrüstung hier lassen (%ls) oder in %ls (%ls), bevor sie Arulco verlässt?";
const wchar_t str_he_leaves_drops_equipment[]       = L"%ls geht bald und lässt seine Ausrüstung in %ls.";
const wchar_t str_she_leaves_drops_equipment[]      = L"%ls geht bald und lässt ihre Ausrüstung in %ls.";


// Text used on IMP Web Pages
const wchar_t *pImpPopUpStrings[] =
{
	L"Ungültiger Code",
	L"Sie wollen gerade den ganzen Evaluierungsprozess von vorn beginnen. Sind Sie sicher?",
	L"Bitte Ihren vollen Namen und Ihr Geschlecht eingeben",
	L"Die Überprüfung Ihrer finanziellen Mittel hat ergeben, dass Sie sich keine Evaluierung leisten können.",
	L"Option zur Zeit nicht gültig.",
	L"Um eine genaue Evaluierung durchzuführen, müssen Sie mindestens noch ein Teammitglied aufnehmen können.",
	L"Evaluierung bereits durchgeführt.",
};

// button labels used on the IMP site
const wchar_t *pImpButtonText[] =
{
	L"Wir über uns", 			// about the IMP site
	L"BEGINNEN", 			// begin profiling
	L"Persönlichkeit", 		// personality section
	L"Eigenschaften", 		// personal stats/attributes section
	L"Porträt", 			// the personal portrait selection
	L"Stimme %d", 			// the voice selection
	L"Fertig", 			// done profiling
	L"Von vorne anfangen",	 		// start over profiling
	L"Ja, die Antwort passt!",
	L"Ja",
	L"Nein",
	L"Fertig", 			// finished answering questions
	L"Zurück", 			// previous question..abbreviated form
	L"Weiter", 			// next question
	L"JA",		 		// yes, I am certain
	L"NEIN, ICH MÖCHTE VON VORNE ANFANGEN.", // no, I want to start over the profiling process
	L"JA",
	L"NEIN",
	L"Zurück", 			// back one page
	L"Abbruch", 			// cancel selection
	L"Ja",
	L"Nein, ich möchte es mir nochmal ansehen.",
	L"Registrieren", 			// the IMP site registry..when name and gender is selected
	L"Analyse wird durchgeführt",	// analyzing your profile results
	L"OK",
	L"Stimme",
};

const wchar_t *pExtraIMPStrings[] =
{
	L"Um mit der Evaluierung zu beginnen, Persönlichkeit auswählen.",
	L"Da Sie nun mit der Persönlichkeit fertig sind, wählen Sie Ihre Eigenschaften aus.",
	L"Nach Festlegung der Eigenschaften können Sie nun mit der Porträtauswahl fortfahren.",
	L"Wählen Sie abschließend die Stimmprobe aus, die Ihrer eigenen Stimme am nächsten kommt.",
};

const wchar_t pFilesTitle[] = L"Akten einsehen";

const wchar_t *pFilesSenderList[] =
{
	L"Aufklärungsbericht", 		// the recon report sent to the player. Recon is an abbreviation for reconissance
	L"Intercept #1", 		// first intercept file .. Intercept is the title of the organization sending the file...similar in function to INTERPOL/CIA/KGB..refer to fist record in files.txt for the translated title
	L"Intercept #2",		// second intercept file
	L"Intercept #3",		// third intercept file
	L"Intercept #4",		// fourth intercept file
	L"Intercept #5",		// fifth intercept file
	L"Intercept #6",		// sixth intercept file
};

// Text having to do with the History Log
const wchar_t pHistoryTitle[] = L"Logbuch";

const wchar_t *pHistoryHeaders[] =
{
	L"Tag",	 			// the day the history event occurred
	L"Seite", 			// the current page in the history report we are in
	L"Tag", 			// the days the history report occurs over
	L"Ort",	 			// location (in sector) the event occurred
	L"Ereignis", 			// the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
const wchar_t *pHistoryStrings[] =
{
	L"",																						// leave this line blank
	//1-5
	L"%ls von A.I.M. angeheuert.", 										// merc was hired from the aim site
	L"%ls von M.E.R.C. angeheuert.", 									// merc was hired from the aim site
	L"%ls ist tot.", 															// merc was killed
	L"Rechnung an M.E.R.C. bezahlt",								// paid outstanding bills at MERC
	L"Enrico Chivaldoris Auftrag akzeptiert",
	//6-10
	L"BSE-Profil erstellt",
	L"Versicherung abgeschlossen für %ls.", 				// insurance contract purchased
	L"Versicherung gekündigt für %ls.", 				// insurance contract canceled
	L"Versicherung ausgezahlt für %ls.", 							// insurance claim payout for merc
	L"%lss Vertrag um 1 Tag verlängert.", 						// Extented "mercs name"'s for a day
	//11-15
	L"%lss Vertrag um 1 Woche verlängert.", 					// Extented "mercs name"'s for a week
	L"%lss Vertrag um 2 Wochen verlängert.", 					// Extented "mercs name"'s 2 weeks
	L"%ls entlassen.", 													// "merc's name" was dismissed.
	L"%ls geht.", 																		// "merc's name" quit.
	L"Quest begonnen.", 															// a particular quest started
	//16-20
	L"Quest gelöst.",
	L"Mit Vorarbeiter in %ls geredet",									// talked to head miner of town
	L"%ls befreit",
	L"Cheat benutzt",
	L"Essen ist morgen in Omerta",
	//21-25
	L"%ls heiratet Daryl Hick",
	L"%lss Vertrag abgelaufen.",
	L"%ls rekrutiert.",
	L"Enrico sieht kaum Fortschritte",
	L"Schlacht gewonnen",
	//26-30
	L"Mine in %ls produziert weniger",
	L"Mine in %ls leer",
	L"Mine in %ls geschlossen",
	L"Mine in %ls wieder offen",
	L"Etwas über Gefängnis in Tixa erfahren.",
	//31-35
	L"Von Waffenfabrik in Orta gehört.",
	L"Forscher in Orta gab uns viele Raketengewehre.",
	L"Deidranna verfüttert Leichen.",
	L"Frank erzählte von Kämpfen in San Mona.",
	L"Patient denkt, er hat in den Minen etwas gesehen.",
	//36-40
	L"Devin getroffen - verkauft Sprengstoff",
	L"Berühmten Ex-AIM-Mann Mike getroffen!",
	L"Tony getroffen - verkauft Waffen.",
	L"Sergeant Krott gab mir Raketengewehr.",
	L"Kyle die Urkunde für Angels Laden gegeben.",
	//41-45
	L"Madlab will Roboter bauen.",
	L"Gabby kann Tinktur gegen Käfer machen.",
	L"Keith nicht mehr im Geschäft.",
	L"Howard lieferte Gift an Deidranna.",
	L"Keith getroffen - verkauft alles in Cambria.",
	//46-50
	L"Howard getroffen - Apotheker in Balime",
	L"Perko getroffen - hat kleinen Reparaturladen.",
	L"Sam aus Balime getroffen - hat Computerladen.",
	L"Franz hat Elektronik und andere Sachen.",
	L"Arnold repariert Sachen in Grumm.",
	//51-55
	L"Fredo repariert Elektronik in Grumm.",
	L"Spende von Reichem aus Balime bekommen.",
	L"Schrotthändler Jake getroffen.",
	L"Ein Depp hat uns eine Codekarte gegeben.",
	L"Walter bestochen, damit er Keller öffnet.",
	//56-60
	L"Wenn Dave Sprit hat, bekommen wir's gratis.",
	L"Pablo bestochen.",
	L"Kingpin hat Geld in San Mona-Mine.",
	L"%ls gewinnt Extremkampf",
	L"%ls verliert Extremkampf",
	//61-65
	L"%ls beim Extremkampf disqualifiziert",
	L"Viel Geld in verlassener Mine gefunden.",
	L"Von Kingpin geschickten Mörder getroffen",
	L"Kontrolle über Sektor verloren",
	L"Sektor verteidigt",
	//66-70
	L"Schlacht verloren",							//ENEMY_ENCOUNTER_CODE
	L"Tödlicher Hinterhalt",						//ENEMY_AMBUSH_CODE
	L"Hinterhalt ausgehoben",
	L"Angriff fehlgeschlagen",			//ENTERING_ENEMY_SECTOR_CODE
	L"Angriff erfolgreich",
	//71-75
	L"Monster angegriffen",			//CREATURE_ATTACK_CODE
	L"Von Bloodcats getötet",			//BLOODCAT_AMBUSH_CODE
	L"Bloodcats getötet",
	L"%ls wurde getötet",
	L"Carmen den Kopf eines Terroristen gegeben",
	L"Slay ist gegangen", //Slay is a merc and has left the team
	L"%ls gekillt",		//History log for when a merc kills an NPC or PC
};

const wchar_t pHistoryLocations[] = L"n.a"; // N/A is an acronym for Not Applicable

// icon text strings that appear on the laptop
const wchar_t *pLaptopIcons[] =
{
	L"E-mail",
	L"Web",
	L"Finanzen",
	L"Personal",
	L"Logbuch",
	L"Dateien",
	L"Schließen",
	L"sir-FER 4.0",			// our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added
const wchar_t *pBookMarkStrings[] =
{
	L"A.I.M.",
	L"Bobby Rays",
	L"B.S.E",
	L"M.E.R.C.",
	L"Bestattungsinst.",
	L"Florist",
	L"Versicherung",
	L"Abbruch",
};

// When loading or download a web page
const wchar_t *pDownloadString[] = {
	L"Download läuft",
	L"Neuladen läuft",
};

//This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

const wchar_t *gsAtmStartButtonText[] =
{
	L"Statistik", 			// view stats of the merc
	L"Inventar", 			// view the inventory of the merc
	L"Anstellung",
};

// Web error messages. Please use German equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator
const wchar_t pErrorStrings[] = L"Verbindung zum Host wird dauernd unterbrochen. Mit längeren Übertragungszeiten ist zu rechnen.";

const wchar_t pPersonnelString[] = L"Söldner:"; // mercs we have

const wchar_t pWebTitle[] = L"sir-FER 4.0"; // our name for the version of the browser, play on company name

// The titles for the web program title bar, for each page loaded
const wchar_t *pWebPagesTitles[] =
{
	L"A.I.M.",
	L"A.I.M. Mitglieder",
	L"A.I.M. Bilder",		// a mug shot is another name for a portrait
	L"A.I.M. Sortierfunktion",
	L"A.I.M.",
	L"A.I.M. Veteranen",
	L"A.I.M. Politik",
	L"A.I.M. Geschichte",
	L"A.I.M. Links",
	L"M.E.R.C.",
	L"M.E.R.C. Konten",
	L"M.E.R.C. Registrierung",
	L"M.E.R.C. Index",
	L"Bobby Rays",
	L"Bobby Rays - Waffen",
	L"Bobby Rays - Munition",
	L"Bobby Rays - Rüstungen",
	L"Bobby Rays - Sonstiges",							//misc is an abbreviation for miscellaneous
	L"Bobby Rays - Gebraucht",
	L"Bobby Rays - Versandauftrag",
	L"B.S.E",
	L"B.S.E",
	L"Fleuropa",
	L"Fleuropa - Gestecke",
	L"Fleuropa - Bestellformular",
	L"Fleuropa - Karten",
	L"Hammer, Amboss & Steigbügel Versicherungsmakler",
	L"Information",
	L"Vertrag",
	L"Bemerkungen",
	L"McGillicuttys Bestattungen",
	L"URL nicht gefunden.",
	L"Bobby Rays - Letzte Lieferungen",
	L"",
	L"",
};

const wchar_t *pShowBookmarkString[] =
{
	L"Sir-Help",
	L"Erneut auf Web klicken für Lesezeichen.",
};

const wchar_t *pLaptopTitles[] =
{
	L"E-Mail",
	L"Dateien",
	L"Söldner-Manager",
	L"Buchhalter Plus",
	L"Logbuch",
};

const wchar_t *pPersonnelDepartedStateStrings[] =
{ //(careful not to exceed 18 characters total including spaces)
	//reasons why a merc has left.
	L"Getötet",
	L"Entlassen",
	L"Heirat",
	L"Vertrag zu Ende",
	L"Quit",
};

// personnel strings appearing in the Personnel Manager on the laptop
const wchar_t *pPersonelTeamStrings[] =
{
	L"Aktuelles Team",
	L"Ausgeschieden",
	L"Tägliche Kosten:",
	L"Höchste Kosten:",
	L"Niedrigste Kosten:",
	L"Im Kampf getötet:",
	L"Entlassen:",
	L"Sonstiges:",
};

const wchar_t *pPersonnelCurrentTeamStatsStrings[] =
{
	L"Schlechteste",
	L"Durchschn.",
	L"Beste",
};

const wchar_t *pPersonnelTeamStatsStrings[] =
{
	L"GSND",
	L"BEW",
	L"GES",
	L"KRF",
	L"FHR",
	L"WSH",
	L"ERF",
	L"TRF",
	L"TEC",
	L"SPR",
	L"MED",
};

// horizontal and vertical indices on the map screen
const wchar_t *pMapVertIndex[] =
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

const wchar_t *pMapHortIndex[] =
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

const wchar_t *pMapDepthIndex[] =
{
	L"",
	L"-1",
	L"-2",
	L"-3",
};

// text that appears on the contract button
const wchar_t pContractButtonString[] = L"Vertrag";

// text that appears on the update panel buttons
const wchar_t *pUpdatePanelButtons[] =
{
	L"Weiter",
	L"Stop",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle
const wchar_t* const LargeTacticalStr[] =
{
	L"Sie sind in diesem Sektor geschlagen worden!",
	L"Der Feind hat kein Erbarmen mit den Seelen Ihrer Teammitglieder und verschlingt jeden einzelnen.",
	L"Ihre bewusstlosen Teammitglieder wurden gefangen genommen!",
	L"Ihre Teammitglieder wurden vom Feind gefangen genommen.",
};

//Insurance Contract.c
//The text on the buttons at the bottom of the screen.
const wchar_t *InsContractText[] =
{
	L"Zurück",
	L"Vor",
	L"OK",
	L"Löschen",
};

//Insurance Info
// Text on the buttons on the bottom of the screen
const wchar_t *InsInfoText[] =
{
	L"Zurück",
	L"Vor",
};

//For use at the M.E.R.C. web site. Text relating to the player's account with MERC
const wchar_t *MercAccountText[] =
{
	// Text on the buttons on the bottom of the screen
	L"Befugnis erteilen",
	L"Home",
	L"Konto #:",
	L"Söldner",
	L"Tage",
	L"Rate",		//5
	L"Belasten",
	L"Gesamt:",
	L"Zahlung von %ls wirklich genehmigen?",		//the %s is a string that contains the dollar amount ( ex. "$150" )
};

//For use at the M.E.R.C. web site. Text relating a MERC mercenary
const wchar_t *MercInfo[] =
{
	L"Zurück",
	L"Anheuern",
	L"Weiter",
	L"Zusatzinfo",
	L"Home",
	L"Angestellt",
	L"Sold:",
	L"Pro Tag",
	L"Verstorben",

	L"Sie versuchen, zu viele Söldner anzuheuern. 18 ist Maximum.",
	L"nicht da",
};

// For use at the M.E.R.C. web site. Text relating to opening an account with MERC
const wchar_t *MercNoAccountText[] =
{
	//Text on the buttons at the bottom of the screen
	L"Konto eröffnen",
	L"Rückgängig",
	L"Sie haben kein Konto. Möchten Sie eins eröffnen?",
};

// For use at the M.E.R.C. web site. MERC Homepage
const wchar_t *MercHomePageText[] =
{
	//Description of various parts on the MERC page
	L"Speck T. Kline, Gründer und Besitzer",
	L"Hier klicken, um ein Konto zu eröffnen",
	L"Hier klicken, um das Konto einzusehen",
	L"Hier klicken, um Dateien einzusehen.",
	// The version number on the video conferencing system that pops up when Speck is talking
	L"Speck Com v3.2",
};

// For use at MiGillicutty's Web Page.
const wchar_t *sFuneralString[] =
{
	L"McGillicuttys Bestattungen: Wir trösten trauernde Familien seit 1983.",
	L"Der Bestattungsunternehmer und frühere A.I.M.-Söldner Murray \"Pops\" McGillicutty ist ein ebenso versierter wie erfahrener Bestatter.",
	L"Pops hat sein ganzes Leben mit Todes- und Trauerfällen verbracht. Deshalb weiß er aus erster Hand, wie schwierig das sein kann.",
	L"Das Bestattungsunternehmen McGillicutty bietet Ihnen einen umfassenden Service, angefangen bei der Schulter zum Ausweinen bis hin zur kosmetischen Aufbereitung entstellter Körperteile.",
	L"McGillicuttys Bestattungen - und Ihre Lieben ruhen in Frieden.",

	// Text for the various links available at the bottom of the page
	L"BLUMEN",
	L"SÄRGE UND URNEN",
	L"FEUERBEST.",
	L"GRÄBER",
	L"PIETÄT",

	// The text that comes up when you click on any of the links ( except for send flowers ).
	L"Leider ist diese Site aufgrund eines Todesfalles in der Familie noch nicht fertiggestellt. Sobald das Testament eröffnet worden und die Verteilung des Erbes geklärt ist, wird diese Site fertiggestellt.",
	L"Unser Mitgefühl gilt trotzdem all jenen, die es diesmal versucht haben. Bis später.",
};

// Text for the florist Home page
const wchar_t *sFloristText[] =
{
	//Text on the button on the bottom of the page

	L"Galerie",

	//Address of United Florist

	L"\"Wir werfen überall per Fallschirm ab\"",
	L"1-555-SCHNUPPER-MAL",
	L"333 Duftmarke Dr, Aroma City, CA USA 90210",
	L"http://www.schnupper-mal.com",

	// detail of the florist page

	L"Wir arbeiten schnell und effizient",
	L"Lieferung am darauf folgenden Tag, in fast jedes Land der Welt. Ausnahmen sind möglich.",
	L"Wir haben die garantiert niedrigsten Preise weltweit!",
	L"Wenn Sie anderswo einen niedrigeren Preis für irgend ein Arrangement sehen, bekommen Sie von uns ein Dutzend Rosen umsonst!",
	L"Fliegende Flora, Fauna & Blumen seit 1981.",
	L"Unsere hochdekorierten Ex-Bomber-Piloten werfen das Bouquet in einem Radius von zehn Meilen rund um den Bestimmungsort ab. Jederzeit!",
	L"Mit uns werden Ihre blumigsten Fantasien wahr",
	L"Bruce, unser weltberühmter Designer-Florist, verwendet nur die frischesten handverlesenen Blumen aus unserem eigenen Gewächshaus.",
	L"Und denken Sie daran: Was wir nicht haben, pflanzen wir für Sie - und zwar schnell!",
};

//Florist OrderForm
const wchar_t *sOrderFormText[] =
{

	//Text on the buttons

	L"Zurück",
	L"Senden",
	L"Löschen",
	L"Galerie",

	L"Name des Gestecks:",
	L"Preis:",			//5
	L"Bestellnr.:",
	L"Liefertermin",
	L"Morgen",
	L"Egal",
	L"Bestimmungsort",			//10
	L"Extraservice",
	L"Kaputtes Gesteck($10)",
	L"Schwarze Rosen($20)",
	L"Welkes Gesteck($10)",
	L"Früchtekuchen (falls vorrätig)($10)",		//15
	L"Persönliche Worte:",
	L"Aufgrund der Kartengröße darf Ihre Botschaft nicht länger sein als 75 Zeichen.",
	L"...oder wählen Sie eine unserer",

	L"STANDARD-KARTEN",
	L"Rechnung für",//20

	//The text that goes beside the area where the user can enter their name

	L"Name:",
};

//Florist Gallery.c
const wchar_t *sFloristGalleryText[] =
{
	//text on the buttons
	L"Zurück",	//abbreviation for previous
	L"Weiter",	//abbreviation for next
	L"Klicken Sie auf das Gesteck Ihrer Wahl",
	L"Bitte beachten Sie, dass wir für jedes kaputte oder verwelkte Gesteck einen Aufpreis von $10 berechnen.",
	L"Home",
};

const wchar_t *sFloristCards[] =
{
	L"Klicken Sie auf das Gesteck Ihrer Wahl",
	L"Zurück",
};

// Text for Bobby Ray's Mail Order Site
const wchar_t *BobbyROrderFormText[] =
{
	L"Bestellformular",				//Title of the page
	L"St.",					// The number of items ordered
	L"Gew. (%ls)",			// The weight of the item
	L"Artikel",				// The name of the item
	L"Preis",				// the item's weight
	L"Summe",				//5	// The total price of all of items of the same type
	L"Zwischensumme",				// The sub total of all the item totals added
	L"Versandkosten (vgl. Bestimmungsort)",		// S&H is an acronym for Shipping and Handling
	L"Endbetrag",			// The grand total of all item totals + the shipping and handling
	L"Bestimmungsort",
	L"Liefergeschwindigkeit",			//10	// See below
	L"$ (pro %ls)",			// The cost to ship the items
	L"Übernacht Express",			// Gets deliverd the next day
	L"2 Arbeitstage",			// Gets delivered in 2 days
	L"Standard-Service",			// Gets delivered in 3 days
	L"Löschen",//15			// Clears the order page
	L"Bestellen",			// Accept the order
	L"Zurück",				// text on the button that returns to the previous page
	L"Home",				// Text on the button that returns to the home page
	L"* Gebrauchter Gegenstand",		// Disclaimer stating that the item is used
	L"Sie haben nicht genug Geld.",		//20	// A popup message that to warn of not enough money
	L"<KEINER>",				// Gets displayed when there is no valid city selected
	L"Wollen Sie Ihre Bestellung wirklich nach %ls schicken?",		// A popup that asks if the city selected is the correct one
	L"Packungsgewicht**",			// Displays the weight of the package
	L"** Mindestgewicht",				// Disclaimer states that there is a minimum weight for the package
	L"Lieferungen",
};

// This text is used when on the various Bobby Ray Web site pages that sell items
const wchar_t *BobbyRText[] =
{
	L"Bestellen",				// Title
	L"Klicken Sie auf den gewünschten Gegenstand. Weiteres Klicken erhöht die Stückzahl. Rechte Maustaste verringert Stückzahl. Wenn Sie fertig sind, weiter mit dem Bestellformular.",			// instructions on how to order

	//Text on the buttons to go the various links

	L"Zurück",		//
	L"Feuerwfn.", 			//3
	L"Munition",			//4
	L"Rüstung",			//5
	L"Sonstiges",			//6	//misc is an abbreviation for miscellaneous
	L"Gebraucht",			//7
	L"Vor",
	L"BESTELLEN",
	L"Home",			//10

	//The following lines provide information on the items

	L"Gewicht:",			// Weight of all the items of the same type
	L"Kal:",			// the caliber of the gun
	L"Mag:",			// number of rounds of ammo the Magazine can hold
	L"Reichw.:",				// The range of the gun
	L"Schaden:",				// Damage of the weapon
	L"Freq.:",			// Weapon's Rate Of Fire, acroymn ROF
	L"Preis:",			// Cost of the item
	L"Vorrätig:",			// The number of items still in the store's inventory
	L"Bestellt:",		// The number of items on order
	L"Beschädigt",			// If the item is damaged
	L"Summe:",			// The total cost of all items on order
	L"* % funktionstüchtig",		// if the item is damaged, displays the percent function of the item

	//Popup that tells the player that they can only order 10 items at a time
	L"Mist! Mit diesem Formular können Sie nur 10 Sachen bestellen. Wenn Sie mehr wollen (was wir sehr hoffen), füllen Sie bitte noch ein Formular aus.",

	// A popup that tells the user that they are trying to order more items then the store has in stock

	L"Sorry. Davon haben wir leider im Moment nichts mehr auf Lager. Versuchen Sie es später noch einmal.",

	//A popup that tells the user that the store is temporarily sold out

	L"Es tut uns sehr leid, aber im Moment sind diese Sachen total ausverkauft.",
};

/* The following line is used on the Ammunition page.  It is used for help text
 * to display how many items the player's merc has that can use this type of
 * ammo. */
const wchar_t str_bobbyr_guns_num_guns_that_use_ammo[] = L"Ihr Team hat %d Waffe(n), die dieses Kaliber benutzen";

// Text for Bobby Ray's Home Page
const wchar_t *BobbyRaysFrontText[] =
{
	//Details on the web site

	L"Dies ist die heißeste Site für Waffen und militärische Ausrüstung aller Art",
	L"Welchen Sprengstoff Sie auch immer brauchen - wir haben ihn.",
	L"SECOND HAND",

	//Text for the various links to the sub pages

	L"SONSTIGES",
	L"FEUERWAFFEN",
	L"MUNITION",		//5
	L"RÜSTUNG",

	//Details on the web site

	L"Was wir nicht haben, das hat auch kein anderer",
	L"In Arbeit",
};

// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug shot page
const wchar_t *AimSortText[] =
{
	L"A.I.M. Mitglieder",				// Title
	L"Sortieren:",					// Title for the way to sort

	//Text of the links to other AIM pages

	L"Den Söldner-Kurzindex ansehen",
	L"Personalakte der Söldner ansehen",
	L"Die AIM-Veteranengalerie ansehen"
};

// text to display how the entries will be sorted
const wchar_t str_aim_sort_price[]        = L"Preis";
const wchar_t str_aim_sort_experience[]   = L"Erfahrung";
const wchar_t str_aim_sort_marksmanship[] = L"Treffsicherheit";
const wchar_t str_aim_sort_medical[]      = L"Medizin";
const wchar_t str_aim_sort_explosives[]   = L"Sprengstoff";
const wchar_t str_aim_sort_mechanical[]   = L"Technik";
const wchar_t str_aim_sort_ascending[]    = L"Aufsteigend";
const wchar_t str_aim_sort_descending[]   = L"Absteigend";

//Aim Policies.c
//The page in which the AIM policies and regulations are displayed
const wchar_t *AimPolicyText[] =
{
	// The text on the buttons at the bottom of the page

	L"Zurück",
	L"AIM HomePage",
	L"Regel-Index",
	L"Nächste Seite",
	L"Ablehnen",
	L"Zustimmen",
};

//Aim Member.c
//The page in which the players hires AIM mercenaries
// Instructions to the user to either start video conferencing with the merc, or to go the mug shot index
const wchar_t *AimMemberText[] =
{
	L"Linksklick",
	L"zum Kontaktieren.",
	L"Rechtsklick",
	L"zum Foto-Index.",
//	L"Linksklick zum Kontaktieren. \nRechtsklick zum Foto-Index.",
};

//Aim Member.c
//The page in which the players hires AIM mercenaries
const wchar_t *CharacterInfo[] =
{
	// the contract expenses' area

	L"Preis",
	L"Vertrag",
	L"1 Tag",
	L"1 Woche",
	L"2 Wochen",

	// text for the buttons that either go to the previous merc,
	// start talking to the merc, or go to the next merc

	L"Zurück",
	L"Kontakt",
	L"Weiter",
	L"Zusatzinfo",				// Title for the additional info for the merc's bio
	L"Aktive Mitglieder", // Title of the page
	L"Zusätzliche Ausrüstung:",				// Displays the optional gear cost
	L"VERSICHERUNG erforderlich",			// If the merc required a medical deposit, this is displayed
};

//Aim Member.c
//The page in which the player's hires AIM mercenaries
//The following text is used with the video conference popup
const wchar_t *VideoConfercingText[] =
{
	L"Vertragskosten:",				//Title beside the cost of hiring the merc

	//Text on the buttons to select the length of time the merc can be hired

	L"1 Tag",
	L"1 Woche",
	L"2 Wochen",

	//Text on the buttons to determine if you want the merc to come with the equipment

	L"Keine Ausrüstung",
	L"Ausrüstung kaufen",

	// Text on the Buttons

	L"GELD ÜBERWEISEN",			// to actually hire the merc
	L"ABBRECHEN",				// go back to the previous menu
	L"ANHEUERN",				// go to menu in which you can hire the merc
	L"AUFLEGEN",				// stops talking with the merc
	L"OK",
	L"NACHRICHT",			// if the merc is not there, you can leave a message

	//Text on the top of the video conference popup

	L"Videokonferenz mit",
	L"Verbinde. . .",

	L"versichert",			// Displays if you are hiring the merc with the medical deposit

};

//Aim Member.c
//The page in which the player hires AIM mercenaries
// The text that pops up when you select the TRANSFER FUNDS button
const wchar_t *AimPopUpText[] =
{
	L"ELEKTRONISCHE ÜBERWEISUNG AUSGEFÜHRT",	// You hired the merc
	L"ÜBERWEISUNG KANN NICHT BEARBEITET WERDEN",		// Player doesn't have enough money, message 1
	L"NICHT GENUG GELD",				// Player doesn't have enough money, message 2

	// if the merc is not available, one of the following is displayed over the merc's face

	L"Im Einsatz",
	L"Bitte Nachricht hinterlassen",
	L"Verstorben",

	//If you try to hire more mercs than game can support

	L"Sie haben bereits 18 Söldner in Ihrem Team.",

	L"Mailbox",
	L"Nachricht aufgenommen",
};

//AIM Link.c
const wchar_t AimLinkText[] = L"A.I.M. Links"; // The title of the AIM links page

//Aim History
// This page displays the history of AIM
const wchar_t *AimHistoryText[] =
{
	L"Die Geschichte von A.I.M.",					//Title

	// Text on the buttons at the bottom of the page

	L"Zurück",
	L"Home",
	L"Veteranen",
	L"Weiter",
};

//Aim Mug Shot Index
//The page in which all the AIM members' portraits are displayed in the order selected by the AIM sort page.
const wchar_t *AimFiText[] =
{
	// displays the way in which the mercs were sorted

	L"Preis",
	L"Erfahrung",
	L"Treffsicherheit",
	L"Medizin",
	L"Sprengstoff",
	L"Technik",

	// The title of the page, the above text gets added at the end of this text
	L"A.I.M.-Mitglieder ansteigend sortiert nach %ls",
	L"A.I.M. Mitglieder absteigend sortiert nach %ls",

	// Instructions to the players on what to do

	L"Linke Maustaste",
	L"um Söldner auszuwählen",			//10
	L"Rechte Maustaste",
	L"um Optionen einzustellen",

	// Gets displayed on top of the merc's portrait if they are...

	//Please be careful not to increase the size of strings for following three
	L"Verstorben",						//14
	L"Im Dienst",
};

//AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer with AIM
const wchar_t *AimAlumniText[] =
{
	// Text of the buttons
	L"SEITE 1",
	L"SEITE 2",
	L"SEITE 3",
	L"A.I.M.-Veteranen",	// Title of the page
	L"ENDE",		// Stops displaying information on selected merc
};

//AIM Home Page
const wchar_t *AimScreenText[] =
{
	// AIM disclaimers

	L"A.I.M. und das A.I.M.-Logo sind in den meisten Ländern eingetragene Warenzeichen.",
	L"Also denken Sie nicht mal daran, uns nachzumachen.",
	L"Copyright 1998-1999 A.I.M., Ltd. Alle Rechte vorbehalten.",

	//Text for an advertisement that gets displayed on the AIM page

	L"Fleuropa",
	L"\"Wir werfen überall per Fallschirm ab\"",				//10
	L"Treffen Sie gleich zu Anfang",
	L"... die richtige Wahl.",
	L"Was wir nicht haben, das brauchen Sie auch nicht.",
};

//Aim Home Page
const wchar_t *AimBottomMenuText[] =
{
	//Text for the links at the bottom of all AIM pages

	L"Home",
	L"Mitglieder",
	L"Veteranen",
	L"Regeln",
	L"Geschichte",
	L"Links",
};

//ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.
const wchar_t *SKI_Text[] =
{
	L"WAREN VORRÄTIG",	//Header for the merchandise available
	L"SEITE",						//The current store inventory page being displayed
	L"KOSTEN",	//The total cost of the the items in the Dealer inventory area
	L"WERT",		//The total value of items player wishes to sell
	L"SCHÄTZUNG",				//Button text for dealer to evaluate items the player wants to sell
	L"TRANSAKTION",			//Button text which completes the deal. Makes the transaction.
	L"FERTIG",						//Text for the button which will leave the shopkeeper interface.
	L"KOSTEN",		//The amount the dealer will charge to repair the merc's goods
	L"1 STUNDE",			// SINGULAR! The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"%d STUNDEN",		// PLURAL! The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"REPARIERT",		// Text appearing over an item that has just been repaired by a NPC repairman dealer
	L"Es ist kein Platz mehr, um Sachen anzubieten.",	//Message box that tells the user there is no more room to put there stuff
	L"%d MINUTEN",		// The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"Gegenstand fallenlassen.",
};

const wchar_t *SkiMessageBoxText[] =
{
	L"Möchten Sie %ls von Ihrem Konto abbuchen, um die Differenz zu begleichen?",
	L"Nicht genug Geld. Ihnen fehlen %ls",
	L"Möchten Sie %ls von Ihrem Konto abbuchen, um die Kosten zu decken?",
	L"Händler bitten, mit der Überweisung zu beginnen.",
	L"Händler bitten, Gegenstände zu reparieren",
	L"Unterhaltung beenden",
	L"Kontostand",
};

//OptionScreen.c
const wchar_t *zOptionsText[] =
{
	//button Text
	L"Spiel sichern",
	L"Spiel laden",
	L"Spiel beenden",
	L"Fertig",
	//Text above the slider bars
	L"Effekte",
	L"Sprache",
	L"Musik",
	//Confirmation pop when the user selects..
	L"Spiel verlassen und zurück zum Hauptmenü?",
	L"Sprachoption oder Untertitel müssen aktiviert sein.",
};

//SaveLoadScreen
const wchar_t *zSaveLoadText[] =
{
	L"Spiel sichern",
	L"Spiel laden",
	L"Abbrechen",
	L"Auswahl speichern",
	L"Auswahl laden",

	L"Spiel erfolgreich gespeichert",
	L"FEHLER beim Speichern des Spiels!",
	L"Spiel erfolgreich geladen",
	L"FEHLER beim Laden des Spiels: \"%hs\"",


	L"Der gespeicherte Spielstand unterscheidet sich vom aktuellen Spielstand. Es kann wahrscheinlich nichts passieren. Weiter?",
	L"Die gespeicherten Spielstände sind evtl. beschädigt Wollen Sie sie alle löschen?",


	//Translators, the next two strings are for the same thing. The first one is for beta version releases and the second one
	//is used for the final version. Please don't modify the "#ifdef JA2BETAVERSION" or the "#else" or the "#endif" as they are
	//used by the compiler and will cause program errors if modified/removed. It's okay to translate the strings though.
#ifdef JA2BETAVERSION
	L"Gespeicherte Version wurde geändert. Bitte melden Sie etwaige Probleme. Weiter?",
#else
	L"Versuche, älteren Spielstand zu laden. Laden und automatisch aktualisieren?",
#endif

	//Translators, the next two strings are for the same thing. The first one is for beta version releases and the second one
	//is used for the final version. Please don't modify the "#ifdef JA2BETAVERSION" or the "#else" or the "#endif" as they are
	//used by the compiler and will cause program errors if modified/removed. It's okay to translate the strings though.
#ifdef JA2BETAVERSION
	L"Spielstand und Spieleversion wurden geändert. Bitte melden Sie etwaige Probleme. Weiter?",
#else
	L"Versuche, älteren Spielstand zu laden. Laden und automatisch aktualisieren?",
#endif

	L"Gespeichertes Spiel in Slot #%d wirklich überschreiben?",

	//The first %d is a number that contains the amount of free space on the users hard drive,
	//the second is the recommended amount of free space.
	//
	L"Sie haben zu wenig Festplattenspeicher. Sie haben nur %d MB frei und JA2 benötigt mindestens %d MB.",


	L"Speichere...",	//While the game is saving this message appears.

	L"Normale Waffen",
	L"Zusatzwaffen",
	L"Real-Stil",
	L"SciFi-Stil",
	L"Schwierigkeit",
};

//MapScreen
const wchar_t *zMarksMapScreenText[] =
{
	L"Map-Level",
	L"Sie haben gar keine Miliz. Sie müssen Bewohner der Stadt trainieren, wenn Sie dort eine Miliz aufstellen wollen.",
	L"Tägliches Einkommen",
	L"Söldner hat Lebensversicherung",
	L"%ls ist nicht müde.",
	L"%ls ist unterwegs und kann nicht schlafen.",
	L"%ls ist zu müde. Versuchen Sie es ein bisschen später nochmal.",
	L"%ls fährt.",
	L"Der Trupp kann nicht weiter, wenn einer der Söldner pennt.",


	// stuff for contracts
	L"Sie können zwar den Vertrag bezahlen, haben aber kein Geld für die Lebensversicherung.",
	L"%ls Lebensversicherungsprämien kosten %ls pro %d Zusatztag(en). Wollen Sie das bezahlen?",
	L"Sektorinventar",

	L"Söldner hat Krankenversicherung.",


	// other items
	L"Sanitäter", // people acting a field medics and bandaging wounded mercs
	L"Patienten", // people who are being bandaged by a medic
	L"Fertig", // Continue on with the game after autobandage is complete
	L"Stop", // Stop autobandaging of patients by medics now

	L"%ls hat kein Werkzeug.",
	L"%ls hat kein Verbandszeug.",
	L"Es sind nicht genug Leute zum Training bereit.",
	L"%ls ist voller Milizen.",
	L"Söldner hat begrenzten Vertrag.",
	L"Vertrag des Söldners ist nicht versichert",
};

const wchar_t pLandMarkInSectorString[] = L"Trupp %d hat in Sektor %ls jemanden bemerkt";

// confirm the player wants to pay X dollars to build a militia force in town
const wchar_t *pMilitiaConfirmStrings[] =
{
	L"Eine Milizeinheit für diese Stadt zu trainieren kostet $", // telling player how much it will cost
	L"Ausgabe genehmigen?", // asking player if they wish to pay the amount requested
	L"Sie haben nicht genug Geld.", // telling the player they can't afford to train this town
	L"Miliz in %ls (%ls %d) weitertrainieren?", // continue training this town?
	L"Preis $", // the cost in dollars to train militia
	L"( J/N )", // abbreviated yes/no
	L"Milizen in %d Sektoren zu trainieren kostet $ %d. %ls", // cost to train sveral sectors at once
	L"Sie können sich keine $%d für die Miliz hier leisten.",
	L"%ls benötigt eine Loyalität von %d Prozent, um mit dem Milizen-Training fortzufahren.",
	L"Sie können die Miliz in %ls nicht mehr trainieren.",
};

#ifdef JA2DEMOADS
const wchar_t *gpDemoString[] =
{
	//0-9
	L"UNÜBERTROFFENE STRATEGIE.",
	L"UNÜBERTROFFENES ROLLENSPIEL.",
	L"Nicht-lineare Spielhandlung",
	L"Umfassende neue Story",
	L"Heiße Gefechte",
	L"Dynamisches Lighting",
	L"NPC-Interaktion",
	L"Geheimstrategien und Nachtmanöver",
	L"Kreieren Sie Ihre eigenen Söldner",
	L"Herausfordernde und kreative Einsätze",
	L"Steuerbare Fahrzeuge",
	//10-19
	L"Über 150 Charaktere",
	L"Hunderte von abgefahrenen Gegenständen",
	L"Über 250 einzigartige Locations",
	L"Furchterregende Explosionen",
	L"Brandneue taktische Manöver",
	L"Tonnenweise Animationen",
	L"Über 9000 Zeilen Dialoge",
	L"Fantastische Musik von Kevin Manthei",
	L"Globale Strategie",
	L"Kämpfe unter Luftangriffen",
	//20-29
	L"Bilden Sie Ihre eigene Miliz aus",
	L"Individuell einstellbares Gameplay",
	L"Realistische Line-of-sight",
	L"Spektakuläre High-color Grafiken",
	L"Kaufen, verkaufen, tauschen...",
	L"Gegner mit gnadenloser AI",
	L"Modernste Kampftechniken",
	L"Optionales Sci-Fi-Element",
	L"Realistische Tagesabläufe",
	L"Über 60 Stunden Spiel unter Hochspannung",
	//30-34
	L"Auswählbare Schwierigkeitsgrade",
	L"Realistische Ballistik",
	L"Einzigartige physikalische Engine",
	L"Komplett interaktives Spielumfeld",
	L"Und vieles mehr...",
	//35 on are BOLD
	L"",
	L"",
	L"\"Eines der besten Spiele des Jahres\"",
	L"PC Gamer",
	L"Bestellen Sie noch heute unter WWW.JA2.DE!",
};

const wchar_t *gpDemoIntroString[] =
{
	L"Gleich werden Sie noch nie dagewesene Strategien, unübertroffenes Rollenspiel und überwältigende Kampftechniken erleben:",
	//Point 1 (uses one string)
	L"Führen Sie ein Team von Söldnern mit einzigartigen Persönlichkeiten (hier gibt's keine langweiligen stereotypen Einheiten).",
	//Point 2 (uses one string)
	L"Testen Sie die Vielfalt der taktischen Manöver, rennen Sie, kriechen Sie, klettern Sie, springen Sie, führen Sie Nahkämpfe und, und, und...",
	//Point 3 (uses one string)
	L"Spielen Sie mit dem Feuer! Cooles Spielzeug, tödliche Waffen, atemberaubende Explosionen.",
	//Additional comment
	L"(und das ist nur der Anfang)",
	//Introduction/instructions
	L"Willkommen in Demoville... (weiter mit beliebiger Taste)",
};
#endif

//Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom of the single merc panel
const wchar_t *gzMoneyWithdrawMessageText[] =
{
	L"Sie können nur maximal 20.000$ abheben.",
	L"Wollen Sie wirklich %ls auf Ihr Konto einzahlen?",
};

const wchar_t gzCopyrightText[] = L"Copyright (C) 1999 Sir-tech Canada Ltd. Alle Rechte vorbehalten.";

//option Text
const wchar_t *zOptionsToggleText[] =
{
	L"Sprache",
	L"Stumme Bestätigungen",
	L"Untertitel",
	L"Dialoge Pause",
	L"Rauch animieren",
	L"Sichtbare Verletzungen",
	L"Cursor nicht bewegen!",
	L"Alte Auswahlmethode",
	L"Weg vorzeichnen",
	L"Fehlschüsse anzeigen",
	L"Bestätigung bei Echtzeit",
	L"Schlaf-/Wachmeldung anzeigen",
	L"Metrisches System benutzen",
	L"Boden beleuchten",
	L"Cursor automatisch auf Söldner",
	L"Cursor automatisch auf Türen",
	L"Gegenstände leuchten",
	L"Baumkronen zeigen",
	L"Drahtgitter zeigen",
	L"3D Cursor zeigen",
};

//This is the help text associated with the above toggles.
const wchar_t *zOptionsScreenHelpText[] =
{
	//speech
	L"Mit dieser Option hören Sie die Dialoge.",

	//Mute Confirmation
	L"Schaltet die gesprochenen Bestätigungen an oder aus.",

	//Subtitles
	L"Schaltet Untertitel für Dialoge ein oder aus.",

	//Key to advance speech
	L"Wenn Untertitel AN sind, hat man durch diese Option Zeit, Dialoge von NPCs zu lesen.",

	//Toggle smoke animation
	L"Schalten Sie diese Option ab, wenn animierter Rauch Ihre Bildwiederholrate verlangsamt.",

	//Blood n Gore
	L"Diese Option abschalten, wenn Sie kein Blut sehen können.",

	//Never move my mouse
	L"Wenn Sie diese Option abstellen, wird der Mauszeiger nicht mehr von den Popup-Fenstern verdeckt.",

	//Old selection method
	L"Mit dieser Option funktioniert die Auswahl der Söldner so wie in früheren JAGGED ALLIANCE-Spielen (also genau andersherum als jetzt).",

	//Show movement path
	L"Diese Funktion ANschalten, um die geplanten Wege der Söldner in Echtzeit anzuzeigen\n(oder abgeschaltet lassen und bei gewünschter Anzeige die SHIFT-Taste drücken).",

	//show misses
	L"Mit dieser Option zeigt Ihnen das Spiel, wo Ihre Kugeln hinfliegen, wenn Sie \"nicht treffen\".",

	//Real Time Confirmation
	L"Durch diese Option wird vor der Rückkehr in den Echtzeit-Modus ein zusätzlicher \"Sicherheits\"-Klick verlangt.",

	//Sleep/Wake notification
  L"Wenn ANgeschaltet werden Sie informiert, wann Ihre Söldner, die sich im \"Dienst\" befinden schlafen oder die Arbeit wieder aufnehmen.",

	//Use the metric system
	L"Mit dieser Option wird im Spiel das metrische System verwendet.",

	//Merc Lighted movement
	L"Diese Funktion ANschalten, wenn der Söldner beim Gehen den Boden beleuchten soll. AUSgeschaltet erhöht sich die Bildwiederholrate.",

	//Smart cursor
	L"Wenn diese Funktion aktiviert ist, werden Söldner automatisch hervorgehoben, sobald der Cursor in ihrer Nähe ist.",

	//snap cursor to the door
	L"Wenn diese Funktion aktiviert ist, springt der Cursor automatisch auf eine Tür, sobald eine in der Nähe ist.",

	//glow items
	L"Angeschaltet, bekommen Gegenstände einen pulsierenden Rahmen (|I).",

	//toggle tree tops
	L"ANgeschaltet, werden die Baumkronen gezeigt (|T).",

	//toggle wireframe
	L"ANgeschaltet, werden Drahtgitter für verborgene Wände gezeigt (|W).",

	L"ANgeschaltet, wird der Bewegungs-Cursor in 3D angezeigt (|P|o|s|1).",

};

const wchar_t *gzGIOScreenText[] =
{
	L"GRUNDEINSTELLUNGEN",
	L"Spielmodus",
	L"Realistisch",
	L"SciFi",
	L"Waffen",
	L"Zusätzliche Waffen",
	L"Normal",
	L"Schwierigkeitsgrad",
	L"Einsteiger",
	L"Profi",
	L"Alter Hase",
	L"Ok",
	L"Abbrechen",
	L"Extraschwer",
	L"Speichern jederzeit möglich",
	L"Iron Man"
};

const wchar_t *pDeliveryLocationStrings[] =
{
	L"Austin",			//Austin, Texas, USA
	L"Bagdad",			//Baghdad, Iraq (Suddam Hussein's home)
	L"Drassen",			//The main place in JA2 that you can receive items. The other towns are dummy names...
	L"Hong Kong",		//Hong Kong, Hong Kong
	L"Beirut",			//Beirut, Lebanon	(Middle East)
	L"London",			//London, England
	L"Los Angeles",	//Los Angeles, California, USA (SW corner of USA)
	L"Meduna",			//Meduna -- the other airport in JA2 that you can receive items.
	L"Metavira",		//The island of Metavira was the fictional location used by JA1
	L"Miami",				//Miami, Florida, USA (SE corner of USA)
	L"Moskau",			//Moscow, USSR
	L"New York",		//New York, New York, USA
	L"Ottawa",			//Ottawa, Ontario, Canada -- where JA2 was made!
	L"Paris",				//Paris, France
	L"Tripolis",		//Tripoli, Libya (eastern Mediterranean)
	L"Tokio",				//Tokyo, Japan
	L"Vancouver",		//Vancouver, British Columbia, Canada (west coast near US border)
};

/* This string is used in the IMP character generation.  It is possible to
 * select 0 ability in a skill meaning you can't use it.  This text is
 * confirmation to the player. */
const wchar_t pSkillAtZeroWarning[]    = L"Sind Sie sicher? Ein Wert von 0 bedeutet, dass der Charakter diese Fähigkeit nicht nutzen kann.";
const wchar_t pIMPBeginScreenStrings[] = L"(max. 8 Buchstaben)";
const wchar_t pIMPFinishButtonText[]   = L"Analyse wird durchgeführt";
const wchar_t pIMPFinishStrings[]      = L"Danke, %ls"; //%s is the name of the merc
const wchar_t pIMPVoicesStrings[]      = L"Stimme"; // the strings for imp voices screen

// title for program
const wchar_t pPersTitleText[] = L"Söldner-Manager";

// paused game strings
const wchar_t *pPausedGameText[] =
{
	L"Pause",
	L"Zurück zum Spiel (|P|a|u|s|e)",
	L"Pause (|P|a|u|s|e)",
};

const wchar_t *pMessageStrings[] =
{
	L"Spiel beenden?",
	L"OK",
	L"JA",
	L"NEIN",
	L"ABBRECHEN",
	L"ZURÜCK",
	L"LÜGEN",
	L"Keine Beschreibung", //Save slots that don't have a description.
	L"Spiel gespeichert",
	L"Tag",
	L"Söldner",
	L"Leerer Slot", //An empty save game slot
	L"KpM",					//Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
	L"min",					//Abbreviation for minute.
	L"m",						//One character abbreviation for meter (metric distance measurement unit).
	L"Kgln",				//Abbreviation for rounds (# of bullets)
	L"kg",					//Abbreviation for kilogram (metric weight measurement unit)
	L"Pfd",					//Abbreviation for pounds (Imperial weight measurement unit)
	L"Home",				//Home as in homepage on the internet.
	L"US$",					//Abbreviation for US Dollars
	L"n.a",					//Lowercase acronym for not applicable.
	L"Inzwischen",		//Meanwhile
	L"%ls ist im Sektor %ls%ls angekommen", //Name/Squad has arrived in sector A9. Order must not change without notifying SirTech
	L"Version",
	L"Leerer Quick-Save-Slot",
	L"Dieser Slot ist nur für Quick-Saves aus den Map Screens und dem Taktik-Bildschirm. Speichern mit ALT+S",
	L"offen",
	L"zu",
	L"Ihr Festplattenspeicher ist knapp. Sie haben lediglich %lsMB frei und Jagged Alliance 2 benötigt %lsMB.",
	L"%ls hat %ls gefangen.",		//'Merc name' has caught 'item' -- let SirTech know if name comes after item.

	L"%ls hat die Droge genommen.", //'Merc name' has taken the drug
	L"%ls hat keine medizinischen Fähigkeiten",//'Merc name' has no medical skill.

	//CDRom errors (such as ejecting CD while attempting to read the CD)
	L"Die Integrität des Spieles wurde beschädigt.", //The integrity of the game has been compromised
	L"FEHLER: CD-ROM-Laufwerk schließen",

	//When firing heavier weapons in close quarters, you may not have enough room to do so.
	L"Kein Platz, um von hier aus zu feuern.",

	//Can't change stance due to objects in the way...
	L"Kann seine Position jetzt nicht ändern.",

	//Simple text indications that appear in the game, when the merc can do one of these things.
	L"Ablegen",
	L"Werfen",
	L"Weitergeben",

	L"%ls weitergegeben an %ls.", //"Item" passed to "merc". Please try to keep the item %s before the merc %s, otherwise,
											 //must notify SirTech.
	L"Kein Platz, um %ls an %ls weiterzugeben.", //pass "item" to "merc". Same instructions as above.

	//A list of attachments appear after the items. Ex: Kevlar vest ( Ceramic Plate 'Attached )'
	L" angebracht)",

	//Cheat modes
	L"Cheat-Level EINS erreicht",
	L"Cheat-Level ZWEI erreicht",

	//Toggling various stealth modes
	L"Stealth Mode für Trupp ein.",
	L"Stealth Mode für Trupp aus.",
	L"Stealth Mode für %ls ein.",
	L"Stealth Mode für %ls aus.",

	//Wireframes are shown through buildings to reveal doors and windows that can't otherwise be seen in
	//an isometric engine. You can toggle this mode freely in the game.
	L"Drahtgitter ein",
	L"Drahtgitter aus",

	//These are used in the cheat modes for changing levels in the game. Going from a basement level to
	//an upper level, etc.
	L"Von dieser Ebene geht es nicht nach oben...",
	L"Noch tiefere Ebenen gibt es nicht...",
	L"Gewölbeebene %d betreten...",
	L"Gewölbe verlassen...",

	L"s",		// used in the shop keeper inteface to mark the ownership of the item eg Red's gun
	L"Autoscrolling AUS.",
	L"Autoscrolling AN.",
	L"3D-Cursor AUS.",
	L"3D-Cursor AN.",
	L"Trupp %d aktiv.",
	L"Sie können %lss Tagessold von %ls nicht zahlen",	//first %s is the mercs name, the second is a string containing the salary
	L"Abbruch",
	L"%ls kann alleine nicht gehen.",
	L"Spielstand namens Spielstand99.sav kreiert. Wenn nötig, in Spielstand01 - Spielstand10 umbennen und über die Option 'Laden' aufrufen.",
	L"%ls hat %ls getrunken.",
	L"Paket in Drassen angekommen.",
	L"%ls kommt am %d. um ca. %ls am Zielort an (Sektor %ls).",		//first %s is mercs name(OK), next is the sector location and name where they will be arriving in, lastely is the day an the time of arrival       !!!7 It should be like this: first one is merc (OK), next is day of arrival (OK) , next is time of the day for ex. 07:00 (not OK, now it is still sector), next should be sector (not OK, now it is still time of the day)
	L"Logbuch aktualisiert.",
#ifdef JA2BETAVERSION
	L"Spiel erfolgreich in Slot End Turn Auto Save gespeichert.",
#endif
};

const wchar_t ItemPickupHelpPopup[][40] =
{
	L"OK",
	L"Hochscrollen",
	L"Alle auswählen",
	L"Runterscrollen",
	L"Abbrechen",
};

const wchar_t *pDoctorWarningString[] =
{
	L"%ls ist nicht nahe genug, um geheilt zu werden",
	L"Ihre Mediziner haben noch nicht alle verbinden können.",
};

const wchar_t *pMilitiaButtonsHelpText[] =
{
	L"Grüne Miliz aufnehmen(Rechtsklick)/absetzen(Linksklick)", // button help text informing player they can pick up or drop militia with this button
	L"Reguläre Milizen aufnehmen(Rechtsklick)/absetzen(Linksklick)",
	L"Elitemilizen aufnehmen(Rechtsklick)/absetzen(Linksklick)",
	L"Milizen gleichmäßig über alle Sektoren verteilen",
};

// to inform the player to hire some mercs to get things going
const wchar_t pMapScreenJustStartedHelpText[] = L"Zu AIM gehen und Söldner anheuern ( *Tip*: Befindet sich im Laptop )";

const wchar_t pAntiHackerString[] = L"Fehler. Fehlende oder fehlerhafte Datei(en). Spiel wird beendet.";

const wchar_t *gzLaptopHelpText[] =
{
	//Buttons:
	L"E-Mail einsehen",
	L"Websites durchblättern",
	L"Dateien und Anlagen einsehen",
	L"Logbuch lesen",
	L"Team-Info einsehen",
	L"Finanzen und Notizen einsehen",

	L"Laptop schließen",

	//Bottom task bar icons (if they exist):
	L"Sie haben neue Mail",
	L"Sie haben neue Dateien",

	//Bookmarks:
	L"Association of International Mercenaries",
	L"Bobby Rays Online-Waffenversand",
	L"Bundesinstitut für Söldnerevaluierung",
	L"More Economic Recruiting Center",
	L"McGillicuttys Bestattungen",
	L"Fleuropa",
	L"Versicherungsmakler für A.I.M.-Verträge",
};

const wchar_t gzHelpScreenText[] = L"Helpscreen verlassen";

const wchar_t *gzNonPersistantPBIText[] =
{
	L"Es tobt eine Schlacht. Sie können sich nur im Taktikbildschirm zurückziehen.",
	L"Sektor betreten und Kampf fortsetzen (|E).",
	L"Kampf durch PC entscheiden (|A).",
	L"Sie können den Kampf nicht vom PC entscheiden lassen, wenn Sie angreifen.",
	L"Sie können den Kampf nicht vom PC entscheiden lassen, wenn Sie in einem Hinterhalt sind.",
	L"Sie können den Kampf nicht vom PC entscheiden lassen, wenn Sie gegen Monster kämpfen.",
	L"Sie können den Kampf nicht vom PC entscheiden lassen, wenn feindliche Zivilisten da sind.",
	L"Sie können einen Kampf nicht vom PC entscheiden lassen, wenn Bloodcats da sind.",
	L"KAMPF IM GANGE",
	L"Sie können sich nicht zurückziehen, wenn Sie in einem Hinterhalt sind.",
};

const wchar_t *gzMiscString[] =
{
	L"Ihre Milizen kämpfen ohne die Hilfe der Söldner weiter...",
	L"Das Fahrzeug muss nicht mehr aufgetankt werden.",
	L"Der Tank ist %d%% voll.",
	L"Deidrannas Armee hat wieder volle Kontrolle über %ls.",
	L"Sie haben ein Tanklager verloren.",
};


const wchar_t gzIntroScreen[] = L"Kann Introvideo nicht finden";

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
const wchar_t *pNewNoiseStr[] =
{
	//There really isn't any difference between using "coming from" or "to".
	//For the explosion case the string in English could be either:
	//	L"Gus hears a loud EXPLOSION 'to' the north.",
	//	L"Gus hears a loud EXPLOSION 'coming from' the north.",
	//For certain idioms, it sounds better to use one over the other. It is a matter of preference.
	L"%ls hört %ls aus dem %ls.",
	L"%ls hört eine BEWEGUNG (%ls) von %ls.",
	L"%ls hört ein KNARREN (%ls) von %ls.",
	L"%ls hört ein KLATSCHEN (%ls) von %ls.",
	L"%ls hört einen AUFSCHLAG (%ls) von %ls.",
	L"%ls hört eine EXPLOSION (%ls) von %ls.",
	L"%ls hört einen SCHREI (%ls) von %ls.",
	L"%ls hört einen AUFSCHLAG (%ls) von %ls.",
	L"%ls hört einen AUFSCHLAG (%ls) von %ls.",
	L"%ls hört ein ZERBRECHEN (%ls) von %ls.",
	L"%ls hört ein ZERSCHMETTERN (%ls) von %ls.",
};

const wchar_t *wMapScreenSortButtonHelpText[] =
{
	L"Sortieren nach Name (|F|1)",
	L"Sortieren nach Auftrag (|F|2)",
	L"Sortieren nach wach/schlafend (|F|3)",
	L"Sortieren nach Ort (|F|4)",
	L"Sortieren nach Ziel (|F|5)",
	L"Sortieren nach Vertragsende (|F|6)",
};

const wchar_t *BrokenLinkText[] =
{
	L"Error 404",
	L"Site nicht gefunden.",
};

const wchar_t *gzBobbyRShipmentText[] =
{
	L"Letzte Lieferungen",
	L"Bestellung #",
	L"Artikelanzahl",
	L"Bestellt am",
};

const wchar_t *gzCreditNames[]=
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

const wchar_t *gzCreditNameTitle[]=
{
	L"Game Internals Programmer", 			// Chris Camfield
	L"Co-designer/Writer",							// Shaun Lyng
	L"Strategic Systems & Editor Programmer",					//Kris \"The Cow Rape Man\" Marnes
	L"Producer/Co-designer",						// Ian Currie
	L"Co-designer/Map Designer",				// Linda Currie
	L"Artist",													// Eric \"WTF\" Cheng
	L"Beta Coordinator, Support",				// Lynn Holowka
	L"Artist Extraordinaire",						// Norman \"NRG\" Olsen
	L"Sound Guru",											// George Brooks
	L"Screen Designer/Artist",					// Andrew Stacey
	L"Lead Artist/Animator",						// Scot Loving
	L"Lead Programmer",									// Andrew \"Big Cheese Doddle\" Emmons
	L"Programmer",											// Dave French
	L"Strategic Systems & Game Balance Programmer",					// Alex Meduna
	L"Portraits Artist",								// Joey \"Joeker\" Whelan",
};

const wchar_t *gzCreditNameFunny[]=
{
	L"", 																			// Chris Camfield
	L"(still learning punctuation)",					// Shaun Lyng
	L"(\"It's done. I'm just fixing it\")",	//Kris \"The Cow Rape Man\" Marnes
	L"(getting much too old for this)",				// Ian Currie
	L"(and working on Wizardry 8)",						// Linda Currie
	L"(forced at gunpoint to also do QA)",			// Eric \"WTF\" Cheng
	L"(Left us for the CFSA - go figure...)",	// Lynn Holowka
	L"",																			// Norman \"NRG\" Olsen
	L"",																			// George Brooks
	L"(Dead Head and jazz lover)",						// Andrew Stacey
	L"(his real name is Robert)",							// Scot Loving
	L"(the only responsible person)",					// Andrew \"Big Cheese Doddle\" Emmons
	L"(can now get back to motocrossing)",	// Dave French
	L"(stolen from Wizardry 8)",							// Alex Meduna
	L"(did items and loading screens too!)",	// Joey \"Joeker\" Whelan",
};

const wchar_t *sRepairsDoneString[] =
{
	L"%ls hat  seine eigenen Gegenstände repariert",
	L"%ls hat die Waffen und Rüstungen aller Teammitglieder repariert",
	L"%ls hat die aktivierten Gegenstände aller Teammitglieder repariert",
	L"%ls hat die mitgeführten Gegenstände aller Teammitglieder repariert",
};

const wchar_t *zGioDifConfirmText[]=
{
	L"Sie haben sich für den EINSTEIGER-Modus entschieden. Dies ist die passende Einstellung für Spieler, die noch nie zuvor Jagged Alliance oder ähnliche Spiele gespielt haben oder für Spieler, die sich ganz einfach kürzere Schlachten wünschen. Ihre Wahl wird den Verlauf des ganzen Spiels beeinflussen. Treffen Sie also eine sorgfältige Wahl. Sind Sie ganz sicher, dass Sie im Einsteiger-Modus spielen wollen?",
	L"Sie haben sich für den FORTGESCHRITTENEN-Modus entschieden. Dies ist die passende Einstellung für Spieler, die bereits Erfahrung mit Jagged Alliance oder ähnlichen Spielen haben. Ihre Wahl wird den Verlauf des ganzen Spiels beeinflussen. Treffen Sie also eine sorgfältige Wahl. Sind Sie ganz sicher, dass Sie im Fortgeschrittenen-Modus spielen wollen?",
	L"Sie haben sich für den PROFI-Modus entschieden. Na gut, wir haben Sie gewarnt. Machen Sie hinterher bloß nicht uns dafür verantwortlich, wenn Sie im Sarg nach Hause kommen. Ihre Wahl wird den Verlauf des ganzen Spiels beeinflussen. Treffen Sie also eine sorgfältige Wahl. Sind Sie ganz sicher, dass Sie im Profi-Modus spielen wollen?",
};

const wchar_t *gzLateLocalizedString[] =
{
	L"%ls Loadscreen-Daten nicht gefunden...",

	//1-5
	L"Der Roboter kann diesen Sektor nicht verlassen, wenn niemand die Fernbedienung benutzt.",

	L"Sie können den Zeitraffer jetzt nicht benutzen. Warten Sie das Feuerwerk ab!",
	L"%ls will sich nicht bewegen.",
	L"%ls hat nicht genug Energie, um die Position zu ändern.",
	L"%ls hat kein Benzin mehr und steckt in %c%d fest.",

	//6-10

	// the following two strings are combined with the strings below to report noises
	// heard above or below the merc
	L"oben",
	L"unten",

	//The following strings are used in autoresolve for autobandaging related feedback.
	L"Keiner der Söldner hat medizinische Fähigkeiten.",
	L"Sie haben kein Verbandszeug.",
	L"Sie haben nicht genug Verbandszeug, um alle zu verarzten.",
	L"Keiner der Söldner muss verbunden werden.",
	L"Söldner automatisch verbinden.",
	L"Alle Söldner verarztet.",

	//14-16
	L"Arulco",
	L"(Dach)",
	L"Gesundheit: %d/%d",

	//17
	//In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
	//"vs." is the abbreviation of versus.
	L"%d gegen %d",

	//18-19
	L"%ls ist voll!", //(ex "The ice cream truck is full")
	L"%ls braucht nicht eine schnelle Erste Hilfe, sondern eine richtige medizinische Betreuung und/oder Erholung.",

	//20
	//Happens when you get shot in the legs, and you fall down.
	L"%ls ist am Bein getroffen und hingefallen!",
	//Name can't speak right now.
	L"%ls kann gerade nicht sprechen.",

	//22-24 plural versions
	L"%d grüne Milizen wurden zu Elitemilizen befördert.",
	L"%d grüne Milizen wurden zu regulären Milizen befördert.",
	L"%d reguläre Milizen wurden zu Elitemilizen befördert.",

	//25
	L"Schalter",

	//26
	//Name has gone psycho -- when the game forces the player into burstmode (certain unstable characters)
	L"%ls dreht durch!",

	//27-28
	//Messages why a player can't time compress.
	L"Es ist momentan gefährlich den Zeitraffer zu betätigen, da Sie noch Söldner in Sektor %ls haben.",
	L"Es ist gefährlich den Zeitraffer zu betätigen, wenn Sie noch Söldner in den von Monstern verseuchten Minen haben.",

	//29-31 singular versions
	L"1 grüne Miliz wurde zur Elitemiliz befördert.",
	L"1 grüne Miliz wurde zur regulären Miliz befördert.",
	L"1 reguläre Miliz wurde zur Elitemiliz befördert.",

	//32-34
	L"%ls sagt überhaupt nichts.",
	L"Zur Oberfläche gehen?",
	L"(Trupp %d)",

	//35
	L"%ls reparierte %lss %ls",

	//36
	L"BLOODCAT",

	//37-38 "Name trips and falls"
	L"%ls stolpert und stürzt",
	L"Dieser Gegenstand kann von hier aus nicht aufgehoben werden.",

	//39
	L"Keiner Ihrer übrigen Söldner ist in der Lage zu kämpfen. Die Miliz wird die Monster alleine bekämpfen",

	//40-43
	//%s is the name of merc.
	L"%ls hat keinen Erste-Hilfe-Kasten mehr!",
	L"%ls hat nicht das geringste Talent, jemanden zu verarzten!",
	L"%ls hat keinen Werkzeugkasten mehr!",
	L"%ls ist absolut unfähig dazu, irgend etwas zu reparieren!",

	//44
	L"Repar. Zeit",
	L"%ls kann diese Person nicht sehen.",

	//46-48
	L"%lss Gewehrlauf-Verlängerung fällt ab!",
	L"Pro Sektor sind nicht mehr als %d Milizausbilder erlaubt.",
  L"Sind Sie sicher?", //

	//49-50
	L"Zeitraffer", //time compression
	L"Der Fahrzeugtank ist jetzt voll.",

	//51-52 Fast help text in mapscreen.
	L"Zeitraffer fortsetzen (|L|e|e|r|t|a|s|t|e)",
	L"Zeitraffer anhalten (|E|s|c)",

	//53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
	L"%ls hat die Ladehemmung der %ls behoben",
	L"%ls hat die Ladehemmung von %lss %ls behoben",

	//55
	L"Die Zeit kann nicht komprimiert werden, während das Sektorinventar eingesehen wird.",

	//L"Im Sektor sind Feinde entdeckt worden",		//STR_DETECTED_SIMULTANEOUS_ARRIVAL

	//56
	//Displayed with the version information when cheats are enabled.
	L"Aktueller/Maximaler Fortschritt: %d%%/%d%%",

	//57
	L"John und Mary eskortieren?",

  L"Schalter betätigt.",
};

const wchar_t str_ceramic_plates_smashed[] = L"%lss Keramikplatten wurden zertrümmert!";

const wchar_t str_arrival_rerouted[] = L"Die Ankunft neuer Rekruten wurde nach Sektor %ls umgeleitet, da der geplante Ankunftsort in Sektor %ls vom Feind besetzt ist.";

#ifdef JA2DEMO
const wchar_t str_disabled_for_the_demo[]    = L"Option nicht verfügbar";
/* informs player this option/button has been disabled in the demo */
const wchar_t str_disabled_in_demo[]         = L"Sorry. Diese Option gibt es in der Demo nicht.";
const wchar_t str_go_see_gabby[]             = L"Hier unten sind Sie fertig. Vielleicht sollten Sie mit Gabby reden.";
const wchar_t str_too_bad_you_killed_gabby[] = L"Vielleicht hätten Sie ihn besser nicht getötet.";
/* For the demo, the sector exit interface, you'll be able to split your teams
 * up, but the demo has this feature disabled. This string is fast help text
 * that appears over "single" button. */
const wchar_t str_strategic_exit_gui_demo[]  = L"In der Vollversion können Sie Ihr Team\naufteilen, aber nicht in der Demo.";
/* The overhead map is a map of the entire sector, which you can go into
 * anytime, except in the demo. */
const wchar_t str_overhead_map_disabled[]    = L"Overhead-Karte steht in der Demo nicht zur Verfügung.";
#endif

const wchar_t str_stat_health[]       = L"Gesundheit";
const wchar_t str_stat_agility[]      = L"Beweglichkeit";
const wchar_t str_stat_dexterity[]    = L"Geschicklichkeit";
const wchar_t str_stat_strength[]     = L"Kraft";
const wchar_t str_stat_leadership[]   = L"Führungsqualität";
const wchar_t str_stat_wisdom[]       = L"Weisheit";
const wchar_t str_stat_exp_level[]    = L"Erfahrungsstufe";
const wchar_t str_stat_marksmanship[] = L"Treffsicherheit";
const wchar_t str_stat_mechanical[]   = L"Technik";
const wchar_t str_stat_explosive[]    = L"Sprengstoffe";
const wchar_t str_stat_medical[]      = L"Medizin";

#endif
