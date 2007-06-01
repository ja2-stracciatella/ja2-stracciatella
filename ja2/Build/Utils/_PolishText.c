#include "Language_Defines.h"


#ifdef POLISH

#include "Text.h"

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

	Other examples are used multiple times, like the Esc key  or "|E|s|c" or Space -> (|S|p|a|c|j|a)

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

wchar_t ItemNames[MAXITEMS][80] =
{
	L"",
};


wchar_t ShortItemNames[MAXITEMS][80] =
{
	L"",
};

// Different weapon calibres
// CAWS is Close Assault Weapon System and should probably be left as it is
// NATO is the North Atlantic Treaty Organization
// WP is Warsaw Pact
// cal is an abbreviation for calibre
const wchar_t AmmoCaliber[][20] =
{
	L"0",
	L".38 cal",
	L"9mm",
	L".45 cal",
	L".357 cal",
	L"12 gauge",
	L"CAWS",
	L"5.45mm",
	L"5.56mm",
	L"7.62mm NATO",
	L"7.62mm WP",
	L"4.7mm",
	L"5.7mm",
	L"Monstrum",
	L"Rakiety",
	L"", // dart
	L"", // flame
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
	L".38 cal",
	L"9mm",
	L".45 cal",
	L".357 cal",
	L"12 gauge",
	L"CAWS",
	L"5.45mm",
	L"5.56mm",
	L"7.62mm N.",
	L"7.62mm WP",
	L"4.7mm",
	L"5.7mm",
	L"Monstrum",
	L"Rakiety",
	L"", // dart
};


const wchar_t WeaponType[][30] =
{
	L"Inny",
	L"Pistolet",
	L"Pistolet maszynowy",
	L"Karabin maszynowy",
	L"Karabin",
	L"Karabin snajperski",
	L"Karabin bojowy",
	L"Lekki karabin maszynowy",
	L"Strzelba"
};

const wchar_t TeamTurnString[][STRING_LENGTH] =
{
	L"Tura gracza", // player's turn
	L"Tura przeciwnika",
	L"Tura stworzeÒ",
	L"Tura samoobrony",
	L"Tura cywili"
	// planning turn
};

const wchar_t Message[][STRING_LENGTH] =
{
	L"",

	// In the following 8 strings, the %ls is the merc's name, and the %d (if any) is a number.

	L"%ls dosta≥(a) w g≥owÍ i traci 1 punkt inteligencji!",
	L"%ls dosta≥(a) w ramiÍ i traci 1 punkt zrÍcznoúci!",
	L"%ls dosta≥(a) w klatkÍ piersiowπ i traci 1 punkt si≥y!",
	L"%ls dosta≥(a) w nogi i traci 1 punkt zwinnoúci!",
	L"%ls dosta≥(a) w g≥owÍ i traci %d pkt. inteligencji!",
	L"%ls dosta≥(a) w ramiÍ i traci %d pkt. zrÍcznoúci!",
	L"%ls dosta≥(a) w klatkÍ piersiowπ i traci %d pkt. si≥y!",
	L"%ls dosta≥(a) w nogi i traci %d pkt. zwinnoúci!",
	L"Przerwanie!",

	// The first %ls is a merc's name, the second is a string from pNoiseVolStr,
	// the third is a string from pNoiseTypeStr, and the last is a string from pDirectionStr

	L"", //OBSOLETE
	L"Dotar≥y twoje posi≥ki!",

	// In the following four lines, all %ls's are merc names

	L"%ls prze≥adowuje.",
	L"%ls posiada za ma≥o PunktÛw Akcji!",
	L"%ls udziela pierwszej pomocy. (Naciúnij dowolny klawisz aby przerwaÊ.)",
	L"%ls i %ls udzielajπ pierwszej pomocy. (Naciúnij dowolny klawisz aby przerwaÊ.)",
	// the following 17 strings are used to create lists of gun advantages and disadvantages
	// (separated by commas)
	L"niezawodna",
	L"zawodna",
	L"≥atwa w naprawie",
	L"trudna do naprawy",
	L"solidna",
	L"niesolidna",
	L"szybkostrzelna",
	L"wolno strzelajπca",
	L"daleki zasiÍg",
	L"krÛtki zasiÍg",
	L"ma≥a waga",
	L"duøa waga",
	L"niewielkie rozmiary",
	L"szybki ciπg≥y ogieÒ",
	L"brak ciπg≥ego ognia",
	L"duøy magazynek",
	L"ma≥y magazynek",

	// In the following two lines, all %ls's are merc names

	L"%ls: kamuflaø siÍ star≥.",
	L"%ls: kamuflaø siÍ zmy≥.",

	// The first %ls is a merc name and the second %ls is an item name

	L"Brak amunicji w dodatkowej broni!",
	L"%ls ukrad≥(a): %ls.",

	// The %ls is a merc name

	L"%ls ma broÒ bez funkcji ciπg≥ego ognia.",

	L"Juø masz coú takiego do≥πczone.",
	L"Po≥πczyÊ przedmioty?",

	// Both %ls's are item names

	L"%ls i %ls nie pasujπ do siebie.",

	L"Brak",
	L"Wyjmij amunicjÍ",
	L"Dodatki",

	//You cannot use "item(s)" and your "other item" at the same time.
	//Ex:  You cannot use sun goggles and you gas mask at the same time.
	L" %ls i %ls nie mogπ byÊ uøywane jednoczeúnie.",

	L"Element, ktÛry masz na kursorze myszy moøe byÊ do≥πczony do pewnych przedmiotÛw, poprzez umieszczenie go w jednym z czterech slotÛw.",
	L"Element, ktÛry masz na kursorze myszy moøe byÊ do≥πczony do pewnych przedmiotÛw, poprzez umieszczenie go w jednym z czterech slotÛw. (Jednak w tym przypadku, przedmioty do siebie nie pasujπ.)",
	L"Ten sektor nie zosta≥ oczyszczony z wrogÛw!",
	L"Wciπø musisz daÊ %ls %ls",
	L"%ls dosta≥(a) w g≥owÍ!",
	L"PrzerwaÊ walkÍ?",
	L"Ta zmiana bÍdzie trwa≥a. KontynuowaÊ?",
	L"%ls ma wiÍcej energii!",
	L"%ls poúlizgnπ≥(nÍ≥a) siÍ na kulkach!",
	L"%ls nie chwyci≥(a) - %ls!",
	L"%ls naprawi≥(a) %ls",
	L"Przerwanie dla: ",
	L"PoddaÊ siÍ?",
	L"Ta osoba nie chce twojej pomocy.",
	L"NIE S•DZ !",
  	L"Aby podrÛøowaÊ helikopterem Skyridera, musisz najpierw zmieniÊ przydzia≥ najemnikÛw na POJAZD/HELIKOPTER.",
	L"%ls mia≥(a) czas by prze≥adowaÊ tylko jednπ broÒ",
	L"Tura dzikich kotÛw",
};


// the names of the towns in the game

const wchar_t* pTownNames[] =
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

// the types of time compression. For example: is the timer paused? at normal speed, 5 minutes per second, etc.
// min is an abbreviation for minutes

const wchar_t* sTimeStrings[] =
{
	L"Pauza",
	L"Normalna",
	L"5 min.",
	L"30 min.",
	L"60 min.",
	L"6 godz.", //NEW
};


// Assignment Strings: what assignment does the merc  have right now? For example, are they on a squad, training,
// administering medical aid (doctor) or training a town. All are abbreviated. 8 letters is the longest it can be.

const wchar_t* pAssignmentStrings[] =
{
	L"Oddz. 1",
	L"Oddz. 2",
	L"Oddz. 3",
	L"Oddz. 4",
	L"Oddz. 5",
	L"Oddz. 6",
	L"Oddz. 7",
	L"Oddz. 8",
	L"Oddz. 9",
	L"Oddz. 10",
	L"Oddz. 11",
	L"Oddz. 12",
	L"Oddz. 13",
	L"Oddz. 14",
	L"Oddz. 15",
	L"Oddz. 16",
	L"Oddz. 17",
	L"Oddz. 18",
	L"Oddz. 19",
	L"Oddz. 20",
	L"S≥uøba", // on active duty
	L"Lekarz", // administering medical aid
	L"Pacjent", // getting medical aid
	L"Pojazd", // sitting around resting
	L"PodrÛø", // in transit - abbreviated form
	L"Naprawa", // repairing
	L"Praktyka", // training themselves  // ***************NEW******************** as of June 24. 1998
	L"Samoobr.", // training a town to revolt // *************NEW******************** as of June 24, 1998
	L"Instruk.", // training a teammate
	L"UczeÒ", // being trained by someone else // *******************NEW************** as of June 24, 1998
	L"Nie øyje", // dead
	L"Obezw≥.", // abbreviation for incapacitated
	L"Jeniec", // Prisoner of war - captured
	L"Szpital", // patient in a hospital
	L"Pusty",	// Vehicle is empty
};


const wchar_t* pMilitiaString[] =
{
	L"Samoobrona", // the title of the militia box
	L"Bez przydzia≥u", //the number of unassigned militia troops
	L"Nie moøesz przemieszczaÊ oddzia≥Ûw samoobrony gdy nieprzyjaciel jest w sektorze!",
};


const wchar_t* pMilitiaButtonString[] =
{
	L"Auto", // auto place the militia troops for the player
	L"OK", // done placing militia troops
};

const wchar_t* pConditionStrings[] =
{
	L"Doskona≥y", //the state of a soldier .. excellent health
	L"Dobry", // good health
	L"DoúÊ dobry", // fair health
	L"Ranny", // wounded health
	L"ZmÍczony",//L"Wyczerpany", // tired
	L"Krwawi", // bleeding to death
	L"Nieprzyt.", // knocked out
	L"Umierajπcy", // near death
	L"Nie øyje", // dead
};

const wchar_t* pEpcMenuStrings[] =
{
	L"S≥uøba", // set merc on active duty
	L"Pacjent", // set as a patient to receive medical aid
	L"Pojazd", // tell merc to enter vehicle
	L"WypuúÊ", // let the escorted character go off on their own
	L"Anuluj", // close this menu
};


// look at pAssignmentString above for comments

const wchar_t* pPersonnelAssignmentStrings[] =
{
	L"Oddz. 1",
	L"Oddz. 2",
	L"Oddz. 3",
	L"Oddz. 4",
	L"Oddz. 5",
	L"Oddz. 6",
	L"Oddz. 7",
	L"Oddz. 8",
	L"Oddz. 9",
	L"Oddz. 10",
	L"Oddz. 11",
	L"Oddz. 12",
	L"Oddz. 13",
	L"Oddz. 14",
	L"Oddz. 15",
	L"Oddz. 16",
	L"Oddz. 17",
	L"Oddz. 18",
	L"Oddz. 19",
	L"Oddz. 20",
	L"S≥uøba",
	L"Lekarz",
	L"Pacjent",
	L"Pojazd",
	L"PodrÛø",
	L"Naprawa",
	L"Praktyka",
	L"Trenuje samoobronÍ",
	L"Instruktor",
	L"UczeÒ",
	L"Nie øyje",
	L"Obezw≥adniony",
	L"Jeniec",
	L"Szpital",
	L"Pusty",	// Vehicle is empty
};


// refer to above for comments

const wchar_t* pLongAssignmentStrings[] =
{
	L"Oddzia≥ 1",
	L"Oddzia≥ 2",
	L"Oddzia≥ 3",
	L"Oddzia≥ 4",
	L"Oddzia≥ 5",
	L"Oddzia≥ 6",
	L"Oddzia≥ 7",
	L"Oddzia≥ 8",
	L"Oddzia≥ 9",
	L"Oddzia≥ 10",
	L"Oddzia≥ 11",
	L"Oddzia≥ 12",
	L"Oddzia≥ 13",
	L"Oddzia≥ 14",
	L"Oddzia≥ 15",
	L"Oddzia≥ 16",
	L"Oddzia≥ 17",
	L"Oddzia≥ 18",
	L"Oddzia≥ 19",
	L"Oddzia≥ 20",
	L"S≥uøba",
	L"Lekarz",
	L"Pacjent",
	L"Pojazd",
	L"W podrÛøy",
	L"Naprawa",
	L"Praktyka",
	L"Trenuj samoobronÍ",
	L"Trenuj oddzia≥",
	L"UczeÒ",
	L"Nie øyje",
	L"Obezw≥adniony",
	L"Jeniec",
	L"W szpitalu",
	L"Pusty",	// Vehicle is empty
};


// the contract options

const wchar_t* pContractStrings[] =
{
	L"Opcje kontraktu:",
	L"", // a blank line, required
	L"Zaproponuj 1 dzieÒ", // offer merc a one day contract extension
	L"Zaproponuj 1 tydzieÒ", // 1 week
	L"Zaproponuj 2 tygodnie", // 2 week
	L"Zwolnij", // end merc's contract
	L"Anuluj", // stop showing this menu
};

const wchar_t* pPOWStrings[] =
{
	L"Jeniec",  //an acronym for Prisoner of War
	L"??",
};

const wchar_t* pLongAttributeStrings[] =
{
	L"SI£A", //The merc's strength attribute. Others below represent the other attributes.
	L"ZR CZNOå∆",
	L"ZWINNOå∆",
	L"INTELIGENCJA",
	L"UMIEJ TNOåCI STRZELECKIE",
	L"WIEDZA MEDYCZNA",
	L"ZNAJOMOå∆ MECHANIKI",
	L"UMIEJ TNOå∆ DOWODZENIA",
	L"ZNAJOMOå∆ MATERIA£”W WYBUCHOWYCH",
	L"POZIOM DOåWIADCZENIA",
};

const wchar_t* pInvPanelTitleStrings[] =
{
	L"Os≥ona", // the armor rating of the merc
	L"Ekwip.", // the weight the merc is carrying
	L"Kamuf.", // the merc's camouflage rating
};

const wchar_t* pShortAttributeStrings[] =
{
	L"Zwn", // the abbreviated version of : agility
	L"Zrc", // dexterity
	L"Si≥", // strength
	L"Dow", // leadership
	L"Int", // wisdom
	L"Doú", // experience level
	L"Str", // marksmanship skill
	L"Wyb", // explosive skill
	L"Mec", // mechanical skill
	L"Med", // medical skill
};


const wchar_t* pUpperLeftMapScreenStrings[] =
{
	L"Przydzia≥", // the mercs current assignment // *********************NEW****************** as of June 24, 1998
	L"Kontrakt", // the contract info about the merc
	L"Zdrowie", // the health level of the current merc
	L"Morale", // the morale of the current merc
	L"Stan",	// the condition of the current vehicle
	L"Paliwo",	// the fuel level of the current vehicle
};

const wchar_t* pTrainingStrings[] =
{
	L"Praktyka", // tell merc to train self // ****************************NEW******************* as of June 24, 1998
	L"Samoobrona", // tell merc to train town // *****************************NEW ****************** as of June 24, 1998
	L"Instruktor", // tell merc to act as trainer
	L"UczeÒ", // tell merc to be train by other // **********************NEW******************* as of June 24, 1998
};

const wchar_t* pGuardMenuStrings[] =
{
	L"Limit ognia:", // the allowable rate of fire for a merc who is guarding
	L" Agresywny ogieÒ", // the merc can be aggressive in their choice of fire rates
	L" OszczÍdzaj amunicjÍ", // conserve ammo
	L" Strzelaj w ostatecznoúci", // fire only when the merc needs to
	L"Inne opcje:", // other options available to merc
	L" Moøe siÍ wycofaÊ", // merc can retreat
	L" Moøe szukaÊ schronienia",  // merc is allowed to seek cover
	L" Moøe pomagaÊ partnerom", // merc can assist teammates
	L"OK", // done with this menu
	L"Anuluj", // cancel this menu
};

// This string has the same comments as above, however the * denotes the option has been selected by the player

const wchar_t* pOtherGuardMenuStrings[] =
{
	L"Limit ognia:",
	L" *Agresywny ogieÒ*",
	L" *OszczÍdzaj amunicjÍ*",
	L" *Strzelaj w ostatecznoúci*",
	L"Inne opcje:",
	L" *Moøe siÍ wycofaÊ*",
	L" *Moøe szukaÊ schronienia*",
	L" *Moøe pomagaÊ partnerom*",
	L"OK",
	L"Anuluj",
};

const wchar_t* pAssignMenuStrings[] =
{
	L"S≥uøba", // merc is on active duty
	L"Lekarz", // the merc is acting as a doctor
	L"Pacjent", // the merc is receiving medical attention
	L"Pojazd", // the merc is in a vehicle
	L"Naprawa", // the merc is repairing items
	L"Szkolenie", // the merc is training
	L"Anuluj", // cancel this menu
};

const wchar_t* pRemoveMercStrings[] =
{
	L"UsuÒ najemnika", // remove dead merc from current team
	L"Anuluj",
};

const wchar_t* pAttributeMenuStrings[] =
{
	L"Si≥a",
	L"ZrÍcznoúÊ",
	L"ZwinnoúÊ",
	L"Zdrowie",
	L"Um. strzeleckie",
	L"Wiedza med.",
	L"Zn. mechaniki",
	L"Um. dowodzenia",
	L"Zn. mat. wyb.",
	L"Anuluj",
};

const wchar_t* pTrainingMenuStrings[] =
{
 L"Praktyka", // train yourself //****************************NEW************************** as of June 24, 1998
 L"Samoobrona", // train the town // ****************************NEW ************************* as of June 24, 1998
 L"Instruktor", // train your teammates // *******************NEW************************** as of June 24, 1998
 L"UczeÒ",  // be trained by an instructor //***************NEW************************** as of June 24, 1998
 L"Anuluj", // cancel this menu
};


const wchar_t* pSquadMenuStrings[] =
{
	L"Oddzia≥  1",
	L"Oddzia≥  2",
	L"Oddzia≥  3",
	L"Oddzia≥  4",
	L"Oddzia≥  5",
	L"Oddzia≥  6",
	L"Oddzia≥  7",
	L"Oddzia≥  8",
	L"Oddzia≥  9",
	L"Oddzia≥ 10",
	L"Oddzia≥ 11",
	L"Oddzia≥ 12",
	L"Oddzia≥ 13",
	L"Oddzia≥ 14",
	L"Oddzia≥ 15",
	L"Oddzia≥ 16",
	L"Oddzia≥ 17",
	L"Oddzia≥ 18",
	L"Oddzia≥ 19",
	L"Oddzia≥ 20",
	L"Anuluj",
};


const wchar_t* pPersonnelScreenStrings[] =
{
	L"Zdrowie: ", // health of merc
	L"ZwinnoúÊ: ",
	L"ZrÍcznoúÊ: ",
 	L"Si≥a: ",
 	L"Um. dowodzenia: ",
 	L"Inteligencja: ",
 	L"Poziom doúw.: ", // experience level
 	L"Um. strzeleckie: ",
 	L"Zn. mechaniki: ",
 	L"Zn. mat. wybuchowych: ",
 	L"Wiedza medyczna: ",
 	L"Zastaw na øycie: ", // amount of medical deposit put down on the merc
 	L"Bieøπcy kontrakt: ", // cost of current contract
 	L"Liczba zabÛjstw: ", // number of kills by merc
 	L"Liczba asyst: ", // number of assists on kills by merc
 	L"Dzienny koszt:", // daily cost of merc
 	L"OgÛlny koszt:", // total cost of merc
 	L"WartoúÊ kontraktu:", // cost of current contract
 	L"Us≥ugi ogÛ≥em", // total service rendered by merc
 	L"Zaleg≥a kwota", // amount left on MERC merc to be paid
 	L"CelnoúÊ:", // percentage of shots that hit target
 	L"IloúÊ walk:", // number of battles fought
 	L"Ranny(a):", // number of times merc has been wounded
 	L"UmiejÍtnoúci:",
 	L"Brak umiÍjÍtnoúci",
};


//These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
const wchar_t* gzMercSkillText[] =
{
	L"Brak umiejÍtnoúci",
	L"Otwieranie zamkÛw",
	L"Walka wrÍcz",
	L"Elektronika",
	L"Nocne operacje",
	L"Rzucanie",
	L"Szkolenie",
	L"CiÍøka broÒ",
	L"BroÒ automatyczna",
	L"Skradanie siÍ",
	L"OburÍcznoúÊ",
	L"Kradzieøe",
	L"Sztuki walki",
	L"BroÒ bia≥a",
	L"Snajper",
	L"Kamuflaø",
	L"(Eksp.)",
};


// This is pop up help text for the options that are available to the merc

const wchar_t* pTacticalPopupButtonStrings[] =
{
	L"W|staÒ/Idü",
	L"S|chyl siÍ/Idü",
	L"WstaÒ/Biegnij (|R)",
	L"|Padnij/Czo≥gaj siÍ",
	L"Patrz (|L)",
	L"Akcja",
	L"Rozmawiaj",
	L"Zbadaj (|C|t|r|l)",

	// Pop up door menu
	L"OtwÛrz",
	L"Poszukaj pu≥apek",
	L"Uøyj wytrychÛw",
	L"Wywaø",
	L"UsuÒ pu≥apki",
	L"Zamknij na klucz",
	L"OtwÛrz kluczem",
	L"Uøyj ≥adunku wybuchowego",
	L"Uøyj ≥omu",
	L"Anuluj (|E|s|c)",
	L"Zamknij"
};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.

const wchar_t* pDoorTrapStrings[] =
{
	L"nie posiada øadnych pu≥apek",
	L"ma za≥oøony ≥adunek wybuchowy",
	L"jest pod napiÍciem",
	L"posiada syrenÍ alarmowπ",
	L"posiada dyskretny alarm"
};

// On the map screen, there are four columns. This text is popup help text that identifies the individual columns.

const wchar_t* pMapScreenMouseRegionHelpText[] =
{
	L"WybÛr postaci",
	L"Przydzia≥ najemnika",
	L"Nanieú trasÍ podrÛøy",
	L"Kontrakt najemnika",
	L"UsuÒ najemnika",
	L"åpij", // *****************************NEW********************* as of June 29, 1998
};

// volumes of noises

const wchar_t* pNoiseVolStr[] =
{
	L"CICHY",
	L"WYRAèNY",
	L"G£OåNY",
	L"BARDZO G£OåNY"
};

// types of noises

const wchar_t* pNoiseTypeStr[] = // OBSOLETE
{
	L"NIEOKREåLONY DèWI K",
	L"ODG£OS RUCHU",
	L"ODG£OS SKRZYPNI CIA",
	L"PLUSK",
	L"ODG£OS UDERZENIA",
	L"STRZA£",
	L"WYBUCH",
	L"KRZYK",
	L"ODG£OS UDERZENIA",
	L"ODG£OS UDERZENIA",
	L"£OMOT",
	L"TRZASK"
};

// Directions that are used to report noises

const wchar_t* pDirectionStr[] =
{
	L"P£N-WSCH",
	L"WSCH",
	L"P£D-WSCH",
	L"P£D",
	L"P£D-ZACH",
	L"ZACH",
	L"P£N-ZACH",
	L"P£N"
};

// These are the different terrain types.

const wchar_t* pLandTypeStrings[] =
{
	L"Miasto",
	L"Droga",
	L"Otwarty teren",
	L"Pustynia",
	L"Las",
	L"Las",
	L"Bagno",
	L"Woda",
	L"WzgÛrza",
	L"Teren nieprzejezdny",
	L"Rzeka",	//river from north to south
	L"Rzeka",	//river from east to west
	L"Terytorium innego kraju",
	//NONE of the following are used for directional travel, just for the sector description.
	L"Tropiki",
	L"Pola uprawne",
	L"Otwarty teren, droga",
	L"Las, droga",
	L"Las, droga",
	L"Tropiki, droga",
	L"Las, droga",
	L"Wybrzeøe",
	L"GÛry, droga",
	L"Wybrzeøe, droga",
	L"Pustynia, droga",
	L"Bagno, droga",
	L"Las, Rakiety Z-P",
	L"Pustynia, Rakiety Z-P",
	L"Tropiki, Rakiety Z-P",
	L"Meduna, Rakiety Z-P",

	//These are descriptions for special sectors
	L"Szpital w Cambrii",
	L"Lotnisko w Drassen",
	L"Lotnisko w Medunie",
	L"Rakiety Z-P",
	L"KryjÛwka rebeliantÛw", //The rebel base underground in sector A10
	L"Tixa - Lochy",	//The basement of the Tixa Prison (J9)
	L"Gniazdo stworzeÒ",	//Any mine sector with creatures in it
	L"Orta - Piwnica",	//The basement of Orta (K4)
	L"Tunel",				//The tunnel access from the maze garden in Meduna
						//leading to the secret shelter underneath the palace
	L"Schron",				//The shelter underneath the queen's palace
	L"",							//Unused
};

const wchar_t* gpStrategicString[] =
{
	L"",	//Unused
	L"%ls wykryto w sektorze %c%d, a inny oddzia≥ jest w drodze.",	//STR_DETECTED_SINGULAR
	L"%ls wykryto w sektorze %c%d, a inne oddzia≥y sπ w drodze.",	//STR_DETECTED_PLURAL
	L"Chcesz skoordynowaÊ jednoczesne przybycie?",			//STR_COORDINATE

	//Dialog strings for enemies.

	L"WrÛg daje ci szansÍ siÍ poddaÊ.",			//STR_ENEMY_SURRENDER_OFFER
	L"WrÛg schwyta≥ resztÍ twoich nieprzytomnych najemnikÛw.",	//STR_ENEMY_CAPTURED

	//The text that goes on the autoresolve buttons

	L"OdwrÛt", 		//The retreat button				//STR_AR_RETREAT_BUTTON
	L"OK",		//The done button				//STR_AR_DONE_BUTTON

	//The headers are for the autoresolve type (MUST BE UPPERCASE)

	L"OBRONA",								//STR_AR_DEFEND_HEADER
	L"ATAK",								//STR_AR_ATTACK_HEADER
	L"STARCIE",								//STR_AR_ENCOUNTER_HEADER
	L"Sektor",		//The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

	//The battle ending conditions

	L"ZWYCI STWO!",								//STR_AR_OVER_VICTORY
	L"PORAØKA!",								//STR_AR_OVER_DEFEAT
	L"KAPITULACJA!",							//STR_AR_OVER_SURRENDERED
	L"NIEWOLA!",								//STR_AR_OVER_CAPTURED
	L"ODWR”T!",								//STR_AR_OVER_RETREATED

	//These are the labels for the different types of enemies we fight in autoresolve.

	L"Samoobrona",							//STR_AR_MILITIA_NAME,
	L"Elity",								//STR_AR_ELITE_NAME,
	L"Øo≥nierze",								//STR_AR_TROOP_NAME,
	L"Administrator",							//STR_AR_ADMINISTRATOR_NAME,
	L"Stworzenie",								//STR_AR_CREATURE_NAME,

	//Label for the length of time the battle took

	L"Czas trwania",							//STR_AR_TIME_ELAPSED,

	//Labels for status of merc if retreating.  (UPPERCASE)

	L"WYCOFA£(A) SI ",								//STR_AR_MERC_RETREATED,
	L"WYCOFUJE SI ",								//STR_AR_MERC_RETREATING,
	L"WYCOFAJ SI ",								//STR_AR_MERC_RETREAT,

	//PRE BATTLE INTERFACE STRINGS
	//Goes on the three buttons in the prebattle interface.  The Auto resolve button represents
	//a system that automatically resolves the combat for the player without having to do anything.
	//These strings must be short (two lines -- 6-8 chars per line)

	L"Rozst. autom.",							//STR_PB_AUTORESOLVE_BTN,
	L"Idü do sektora",							//STR_PB_GOTOSECTOR_BTN,
	L"Wycof. ludzi",							//STR_PB_RETREATMERCS_BTN,

	//The different headers(titles) for the prebattle interface.
	L"STARCIE Z WROGIEM",							//STR_PB_ENEMYENCOUNTER_HEADER,
	L"INWAZJA WROGA",							//STR_PB_ENEMYINVASION_HEADER, // 30
	L"ZASADZKA WROGA",
	L"WEJåCIE DO WROGIEGO SEKTORA",
	L"ATAK STWOR”W",
	L"ATAK DZIKICH KOT”W",							//STR_PB_BLOODCATAMBUSH_HEADER
	L"WEJåCIE DO LEGOWISKA DZIKICH KOT”W",			//STR_PB_ENTERINGBLOODCATLAIR_HEADER

	//Various single words for direct translation.  The Civilians represent the civilian
	//militia occupying the sector being attacked.  Limited to 9-10 chars

	L"Po≥oøenie",
	L"Wrogowie",
	L"Najemnicy",
	L"Samoobrona",
	L"Stwory",
	L"Dzikie koty",
	L"Sektor",
	L"Brak",		//If there are no uninvolved mercs in this fight.
	L"N/D",			//Acronym of Not Applicable N/A
	L"d",			//One letter abbreviation of day
	L"g",			//One letter abbreviation of hour

	//TACTICAL PLACEMENT USER INTERFACE STRINGS
	//The four buttons

	L"WyczyúÊ",
	L"Rozprosz",
	L"Zgrupuj",
	L"OK",

	//The help text for the four buttons.  Use \n to denote new line (just like enter).

	L"Kasuje wszystkie pozy|cje najemnikÛw, \ni pozwala ponownie je wprowadziÊ.",
	L"Po kaødym naciúniÍciu rozmie|szcza\nlosowo twoich najemnikÛw.",
	L"|Grupuje najemnikÛw w wybranym miejscu.",
	L"Kliknij ten klawisz gdy juø rozmieúcisz \nswoich najemnikÛw. (|E|n|t|e|r)",
	L"Musisz rozmieúciÊ wszystkich najemnikÛw \nzanim rozpoczniesz walkÍ.",

	//Various strings (translate word for word)

	L"Sektor",
	L"Wybierz poczπtkowe pozycje",

	//Strings used for various popup message boxes.  Can be as long as desired.

	L"To miejsce nie jest zbyt dobre. Jest niedostÍpne. SprÛbuj gdzie indziej.",
	L"RozmieúÊ swoich najemnikÛw na podúwietlonej czÍúci mapy.",

	//This message is for mercs arriving in sectors.  Ex:  Red has arrived in sector A9.
	//Don't uppercase first character, or add spaces on either end.

	L"przyby≥(a) do sektora",

	//These entries are for button popup help text for the prebattle interface.  All popup help
	//text supports the use of \n to denote new line.  Do not use spaces before or after the \n.
	L"|Automatycznie prowadzi walkÍ za ciebie,\nnie ≥adujπc planszy.",
	L"Atakujπc sektor wroga nie moøna automatycznie rozstrzygnπÊ walki.",
	L"Wejúcie do s|ektora by nawiπzaÊ walkÍ z wrogiem.",
	L"Wycofuje oddzia≥ do sπsiedniego sekto|ra.",				//singular version
	L"Wycofuje wszystkie oddzia≥y do sπsiedniego sekto|ra.", //multiple groups with same previous sector

	//various popup messages for battle conditions.

	//%c%d is the sector -- ex:  A9
	L"Nieprzyjaciel zatakowa≥ oddzia≥y samoobrony w sektorze %c%d.",
	//%c%d is the sector -- ex:  A9
	L"Stworzenia zaatakowa≥y oddzia≥y samoobrony w sektorze %c%d.",
	//1st %d refers to the number of civilians eaten by monsters,  %c%d is the sector -- ex:  A9
	//Note:  the minimum number of civilians eaten will be two.
	L"Stworzenia zatakowa≥y i zabi≥y %d cywili w sektorze %ls.",
	//%c%d is the sector -- ex:  A9
	L"Nieprzyjaciel zatakowa≥ twoich najemnikÛw w sektorze %ls.  Øaden z twoich najemnikÛw nie moøe walczyÊ!",
	//%c%d is the sector -- ex:  A9
	L"Stworzenia zatakowa≥y twoich najemnikÛw w sektorze %ls.  Øaden z twoich najemnikÛw nie moøe walczyÊ!",

};

const wchar_t* gpGameClockString[] =
{
	//This is the day represented in the game clock.  Must be very short, 4 characters max.
	L"DzieÒ",
};

//When the merc finds a key, they can get a description of it which
//tells them where and when they found it.
const wchar_t* sKeyDescriptionStrings[2] =
{
	L"Zn. w sektorze:",
	L"Zn. w dniu:",
};

//The headers used to describe various weapon statistics.

const wchar_t gWeaponStatsDesc[][ 14 ] =
{
	L"Waga (%ls):", // change kg to another weight unit if your standard is not kilograms, and TELL SIR-TECH!
	L"Stan:",
	L"IloúÊ:", 		// Number of bullets left in a magazine
	L"Zas.:",		// Range
	L"Si≥a:",		// Damage
    L"PA:",                 // abbreviation for Action Points
	L"",
	L"=",
	L"=",
};

//The headers used for the merc's money.

const wchar_t gMoneyStatsDesc[][ 13 ] =
{
	L"Kwota",
	L"Pozosta≥o:", //this is the overall balance
	L"Kwota",
	L"WydzieliÊ:", // the amount he wants to separate from the overall balance to get two piles of money

	L"Bieøπce",
	L"Saldo",
	L"Kwota do",
	L"podjÍcia",
};

//The health of various creatures, enemies, characters in the game. The numbers following each are for comment
//only, but represent the precentage of points remaining.

const wchar_t zHealthStr[][13] =
{
	L"UMIERAJ•CY",		//	>= 0
	L"KRYTYCZNY", 		//	>= 15
	L"KIEPSKI",		//	>= 30
	L"RANNY",    	//	>= 45
	L"ZDROWY",    	//	>= 60
	L"SILNY",     	// 	>= 75
	L"DOSKONA£Y",		// 	>= 90
};

const wchar_t* gzMoneyAmounts[6] =
{
	L"$1000",
	L"$100",
	L"$10",
	L"OK",
	L"Wydziel",
	L"Podejmij",
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
const wchar_t gzProsLabel[10] =
{
	L"Zalety:",
};

const wchar_t gzConsLabel[10] =
{
	L"Wady:",
};

//Conversation options a player has when encountering an NPC
const wchar_t zTalkMenuStrings[6][ SMALL_STRING_LENGTH ] =
{
	L"PowtÛrz", 	//meaning "Repeat yourself"
	L"Przyjaünie",		//approach in a friendly
	L"Bezpoúrednio",		//approach directly - let's get down to business
	L"Groünie",		//approach threateningly - talk now, or I'll blow your face off
	L"Daj",
	L"Rekrutuj",
};

//Some NPCs buy, sell or repair items. These different options are available for those NPCs as well.
const wchar_t zDealerStrings[4][ SMALL_STRING_LENGTH ]=
{
	L"Kup/Sprzedaj",
	L"Kup",
	L"Sprzedaj",
	L"Napraw",
};

const wchar_t zDialogActions[1][ SMALL_STRING_LENGTH ] =
{
	L"OK",
};


//These are vehicles in the game.

const wchar_t* pVehicleStrings[] =
{
 L"Eldorado",
 L"Hummer", // a hummer jeep/truck -- military vehicle
 L"Furgonetka z lodami",
 L"Jeep",
 L"Czo≥g",
 L"Helikopter",
};

const wchar_t* pShortVehicleStrings[] =
{
	L"Eldor.",
	L"Hummer",			// the HMVV
	L"Furg.",
	L"Jeep",
	L"Czo≥g",
	L"Heli.", 				// the helicopter
};

const wchar_t* zVehicleName[] =
{
	L"Eldorado",
	L"Hummer",		//a military jeep. This is a brand name.
	L"Furg.",			// Ice cream truck
	L"Jeep",
	L"Czo≥g",
	L"Heli.", 		//an abbreviation for Helicopter
};


//These are messages Used in the Tactical Screen

const wchar_t TacticalStr[][ MED_STRING_LENGTH ] =
{
	L"Nalot",
	L"UdzieliÊ automatycznie pierwszej pomocy?",

	// CAMFIELD NUKE THIS and add quote #66.

	L"%ls zauwaøy≥(a) øe dostawa jest niekompletna.",

	// The %ls is a string from pDoorTrapStrings

	L"Zamek %ls.",
	L"Brak zamka.",
	L"Sukces!",
	L"Niepowodzenie.",
	L"Sukces!",
	L"Niepowodzenie.",
	L"Zamek nie ma pu≥apek.",
	L"Sukces!",
	// The %ls is a merc name
	L"%ls nie posiada odpowiedniego klucza.",
	L"Zamek zosta≥ rozbrojony.",
	L"Zamek nie ma pu≥apek.",
	L"ZamkniÍte.",
	L"DRZWI",
	L"ZABEZP.",
	L"ZAMKNI TE",
	L"OTWARTE",
	L"ROZWALONE",
	L"Tu jest prze≥πcznik. W≥πczyÊ go?",
	L"RozbroiÊ pu≥apkÍ?",
	L"Poprz...",
	L"Nast...",
	L"WiÍcej...",

	// In the next 2 strings, %ls is an item name

	L"%ls - po≥oøono na ziemi.",
	L"%ls - przekazano do - %ls.",

	// In the next 2 strings, %ls is a name

	L"%ls otrzyma≥(a) ca≥π zap≥atÍ.",
	L"%ls - naleønoúÊ wobec niej/niego wynosi jeszcze %d.",
	L"Wybierz czÍstotliwoúÊ sygna≥u detonujπcego:",  	//in this case, frequency refers to a radio signal
	L"Ile tur do eksplozji:",	//how much time, in turns, until the bomb blows
	L"Ustaw czÍstotliwoúÊ zdalnego detonatora:", 	//in this case, frequency refers to a radio signal
	L"RozbroiÊ pu≥apkÍ?",
	L"UsunπÊ niebieskπ flagÍ?",
	L"UmieúciÊ tutaj niebieskπ flagÍ?",
	L"KoÒczπca tura",

	// In the next string, %ls is a name. Stance refers to way they are standing.

	L"Na pewno chcesz zaatakowaÊ - %ls?",
	L"Pojazdy nie mogπ zmieniaÊ pozycji.",
	L"Robot nie moøe zmieniaÊ pozycji.",

	// In the next 3 strings, %ls is a name

	L"%ls nie moøe zmieniÊ pozycji w tym miejscu.",
	L"%ls nie moøe tu otrzymaÊ pierwszej pomocy.",
	L"%ls nie potrzebuje pierwszej pomocy.",
	L"Nie moøna ruszyÊ w to miejsce.",
	L"Oddzia≥ jest juø kompletny. Nie ma miejsca dla nowych rekrutÛw.",	//there's no room for a recruit on the player's team

	// In the next string, %ls is a name

	L"%ls pracuje juø dla ciebie.",

	// Here %ls is a name and %d is a number

	L"%ls - naleønoúÊ wobec niej/niego wynosi %d$.",

	// In the next string, %ls is a name

	L"%ls - EskortowaÊ tπ osobÍ?",

	// In the next string, the first %ls is a name and the second %ls is an amount of money (including $ sign)

	L"%ls - ZatrudniÊ tπ osobÍ za %ls dziennie?",

	// This line is used repeatedly to ask player if they wish to participate in a boxing match.

	L"Chcesz walczyÊ?",

	// In the next string, the first %ls is an item name and the
	// second %ls is an amount of money (including $ sign)

	L"%ls - KupiÊ to za %ls?",

	// In the next string, %ls is a name

	L"%ls jest pod eskortπ oddzia≥u %d.",

	// These messages are displayed during play to alert the player to a particular situation

	L"ZACI TA",					//weapon is jammed.
	L"Robot potrzebuje amunicji kaliber %ls.",		//Robot is out of ammo
	L"RzuciÊ tam? To niemoøliwe.",		//Merc can't throw to the destination he selected

	// These are different buttons that the player can turn on and off.

	L"Skradanie siÍ (|Z)",
	L"Ekran |Mapy",
	L"Koniec tury (|D)",
	L"Rozmowa",
	L"Wycisz",
	L"Pozycja do gÛry (|P|g|U|p)",
	L"Poziom kursora (|T|a|b)",
	L"Wspinaj siÍ / Zeskocz",
	L"Pozycja w dÛ≥ (|P|g|D|n)",
	L"BadaÊ (|C|t|r|l)",
	L"Poprzedni najemnik",
	L"NastÍpny najemnik (|S|p|a|c|j|a)",
	L"|Opcje",
	L"Ciπg≥y ogieÒ (|B)",
	L"SpÛjrz/ObrÛÊ siÍ (|L)",
	L"Zdrowie: %d/%d\nEnergia: %d/%d\nMorale: %ls",
	L"Co?",					//this means "what?"
	L"Kont",					//an abbrieviation for "Continued"
	L"%ls ma w≥πczone potwierdzenia g≥osowe.",
	L"%ls ma wy≥πczone potwierdzenia g≥osowe.",
	L"Stan: %d/%d\nPaliwo: %d/%d",
	L"Wysiπdü z pojazdu" ,
	L"ZmieÒ oddzia≥ ( |S|h|i|f|t |S|p|a|c|j|a )",
	L"Prowadü",
	L"N/D",						//this is an acronym for "Not Applicable."
	L"Uøyj ( Walka wrÍcz )",
	L"Uøyj ( Broni palnej )",
	L"Uøyj ( Broni bia≥ej )",
	L"Uøyj ( Mat. wybuchowych )",
	L"Uøyj ( Apteczki )",
	L"(£ap)",
	L"(Prze≥aduj)",
	L"(Daj)",
	L"%ls - pu≥apka zosta≥a uruchomiona.",
	L"%ls przyby≥(a) na miejsce.",
	L"%ls straci≥(a) wszystkie Punkty Akcji.",
	L"%ls jest nieosiπgalny(na).",
	L"%ls ma juø za≥oøone opatrunki.",
	L"%ls nie ma bandaøy.",
	L"WrÛg w sektorze!",
	L"Nie ma wroga w zasiÍgu wzroku.",
	L"Zbyt ma≥o PunktÛw Akcji.",
	L"Nikt nie uøywa zdalnego sterowania.",
	L"Ciπg≥y ogieÒ oprÛøni≥ magazynek!",
	L"ØO£NIERZ",
	L"STW”R",
	L"SAMOOBRONA",
	L"CYWIL",
	L"Wyjúcie z sektora",
	L"OK",
	L"Anuluj",
	L"Wybrany najemnik",
	L"Wszyscy najemnicy w oddziale",
	L"Idü do sektora",
	L"OtwÛrz mapÍ",
	L"Nie moøna opuúciÊ sektora z tej strony.",
	L"%ls jest zbyt daleko.",
	L"UsuÒ korony drzew",
	L"Pokaø korony drzew",
	L"WRONA",				//Crow, as in the large black bird
	L"SZYJA",
	L"G£OWA",
	L"TU£”W",
	L"NOGI",
	L"PowiedzieÊ krÛlowej to, co chce wiedzieÊ?",
	L"WzÛr odcisku palca pobrany",
	L"Niew≥aúciwy wzÛr odcisku palca. BroÒ bezuøyteczna.",
	L"Cel osiπgniÍty",
	L"Droga zablokowana",
	L"Wp≥ata/PodjÍcie pieniÍdzy",		//Help text over the $ button on the Single Merc Panel
	L"Nikt nie potrzebuje pierwszej pomocy.",
	L"Zac.",						// Short form of JAMMED, for small inv slots
	L"Nie moøna siÍ tam dostaÊ.",					// used ( now ) for when we click on a cliff
	L"Przejúcie zablokowane. Czy chcesz zamieniÊ siÍ miejscami z tπ osobπ?",
	L"Osoba nie chce siÍ przesunπÊ.",
	// In the following message, '%ls' would be replaced with a quantity of money (e.g. $200)
	L"Zgadzasz siÍ zap≥aciÊ %ls?",
	L"Zgadzasz siÍ na darmowe leczenie?",
	L"Zgadasz siÍ na ma≥øeÒstwo z Darylem?",
	L"KÛ≥ko na klucze",
	L"Nie moøesz tego zrobiÊ z eskortowanπ osobπ.",
	L"OszczÍdziÊ Krotta?",
	L"Poza zasiÍgiem broni",
	L"GÛrnik",
	L"Pojazdem moøna podrÛøowaÊ tylko pomiÍdzy sektorami",
	L"Teraz nie moøna automatycznie udzieliÊ pierwszej pomocy",
	L"Przejúcie zablokowane dla - %ls",
	L"Twoi najemnicy, schwytani przez øo≥nierzy Deidranny, sπ tutaj uwiÍzieni!",
	L"Zamek zosta≥ trafiony",
	L"Zamek zosta≥ zniszczony",
	L"Ktoú inny majstruje przy tych drzwiach.",
	L"Stan: %d/%d\nPaliwo: %d/%d",
  L"%ls nie widzi - %ls.",  // Cannot see person trying to talk to
};

//Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different circumstances in the "exiting sector" interface.
const wchar_t* pExitingSectorHelpText[] =
{
	//Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box is checked.
	L"Jeúli zaznaczysz tÍ opcjÍ, to sπsiedni sektor zostanie natychmiast za≥adowany.",
	L"Jeúli zaznaczysz tÍ opcjÍ, to na czas podrÛøy pojawi siÍ automatycznie ekran mapy.",

	//If you attempt to leave a sector when you have multiple squads in a hostile sector.
	L"Ten sektor jest okupowany przez wroga i nie moøesz tu zostawiÊ najemnikÛw.\nMusisz uporaÊ siÍ z tπ sytuacjπ zanim za≥adujesz inny sektor.",

	//Because you only have one squad in the sector, and the "move all" option is checked, the "go to sector" option is locked to on.
	//The helptext explains why it is locked.
	L"Gdy wyprowadzisz swoich pozosta≥ych najemnikÛw z tego sektora,\nsπsiedni sektor zostanie automatycznie za≥adowany.",
	L"Gdy wyprowadzisz swoich pozosta≥ych najemnikÛw z tego sektora,\nto na czas podrÛøy pojawi siÍ automatycznie ekran mapy.",

	//If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being escorted.  The "single" button is disabled.
	L"%ls jest pod eskortπ twoich najemnikÛw i nie moøe bez nich opuúciÊ tego sektora.",

	//If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc will be prohibited from leaving alone.
	//There are several strings depending on the gender of the merc and how many EPCs are in the squad.
	//DO NOT USE THE NEWLINE HERE AS IT IS USED FOR BOTH HELPTEXT AND SCREEN MESSAGES!
	L"%ls nie moøe sam opuúciÊ tego sektora, gdyø %ls jest pod jego eskortπ.", //male singular
	L"%ls nie moøe sama opuúciÊ tego sektora, gdyø %ls jest pod jej eskortπ.", //female singular
	L"%ls nie moøe sam opuúciÊ tego sektora, gdyø eskortuje inne osoby.", //male plural
	L"%ls nie moøe sama opuúciÊ tego sektora, gdyø eskortuje inne osoby.", //female plural

	//If one or more of your mercs in the selected squad aren't in range of the traversal area, then the  "move all" option is disabled,
	//and this helptext explains why.
	L"Wszyscy twoi najemnicy muszπ byÊ w pobliøu,\naby oddzia≥ mÛg≥ siÍ przemieszczaÊ.",

	L"", //UNUSED

	//Standard helptext for single movement.  Explains what will happen (splitting the squad)
	L"Jeúli zaznaczysz tÍ opcjÍ, %ls bÍdzie podrÛøowaÊ w pojedynkÍ\ni automatycznie znajdzie siÍ w osobnym oddziale.",

	//Standard helptext for all movement.  Explains what will happen (moving the squad)
	L"Jeúli zaznaczysz tÍ opcjÍ, aktualnie\nwybrany oddzia≥ opuúci ten sektor.",

	//This strings is used BEFORE the "exiting sector" interface is created.  If you have an EPC selected and you attempt to tactically
	//traverse the EPC while the escorting mercs aren't near enough (or dead, dying, or unconscious), this message will appear and the
	//"exiting sector" interface will not appear.  This is just like the situation where
	//This string is special, as it is not used as helptext.  Do not use the special newline character (\n) for this string.
	L"%ls jest pod eskortπ twoich najemnikÛw i nie moøe bez nich opuúciÊ tego sektora. Aby opuúciÊ sektor twoi najemnicy muszπ byÊ w pobliøu.",
};



const wchar_t* pRepairStrings[] =
{
	L"Wyposaøenie", 		// tell merc to repair items in inventory
	L"Baza rakiet Z-P", // tell merc to repair SAM site - SAM is an acronym for Surface to Air Missile
	L"Anuluj", 		// cancel this menu
	L"Robot", 		// repair the robot
};


// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."

const wchar_t* sPreStatBuildString[] =
{
	L"traci", 		// the merc has lost a statistic
	L"zyskuje", 		// the merc has gained a statistic
	L"pkt.",	// singular
	L"pkt.",	// plural
	L"pkt.",	// singular
	L"pkt.",	// plural
};

const wchar_t* sStatGainStrings[] =
{
	L"zdrowia.",
	L"zwinnoúci.",
	L"zrÍcznoúci.",
	L"inteligencji.",
	L"umiejÍtnoúci medycznych.",
	L"umiejÍtnoúci w dziedzinie materia≥Ûw wybuchowych.",
	L"umiejÍtnoúci w dziedzinie mechaniki.",
	L"umiejÍtnoúci strzeleckich.",
	L"doúwiadczenia.",
	L"si≥y.",
	L"umiejÍtnoúci dowodzenia.",
};


const wchar_t* pHelicopterEtaStrings[] =
{
	L"Ca≥kowita trasa:  ",// total distance for helicopter to travel
	L" Bezp.:   ", 			// distance to travel to destination
	L" Niebezp.:", 			// distance to return from destination to airport
	L"Ca≥kowity koszt: ", 		// total cost of trip by helicopter
	L"PCP:  ", 			// ETA is an acronym for "estimated time of arrival"
	L"Helikopter ma ma≥o paliwa i musi wylπdowaÊ na terenie wroga.",	// warning that the sector the helicopter is going to use for refueling is under enemy control ->
  L"Pasaøerowie: ",
  L"WybÛr Skyridera lub pasaøerÛw?",
  L"Skyrider",
  L"Pasaøerowie",
};

const wchar_t* sMapLevelString[] =
{
	L"Poziom:", 			// what level below the ground is the player viewing in mapscreen
};

const wchar_t* gsLoyalString[] =
{
	L"Lojalnoúci", 			// the loyalty rating of a town ie : Loyal 53%
};


// error message for when player is trying to give a merc a travel order while he's underground.

const wchar_t* gsUndergroundString[] =
{
	L"nie moøna wydawaÊ rozkazÛw podrÛøy pod ziemiπ.",
};

const wchar_t* gsTimeStrings[] =
{
	L"g",				// hours abbreviation
	L"m",				// minutes abbreviation
	L"s",				// seconds abbreviation
	L"d",				// days abbreviation
};

// text for the various facilities in the sector

const wchar_t* sFacilitiesStrings[] =
{
	L"Brak",
	L"Szpital",
	L"Przemys≥",
	L"WiÍzienie",
	L"Baza wojskowa",
	L"Lotnisko",
	L"Strzelnica",		// a field for soldiers to practise their shooting skills
};

// text for inventory pop up button

const wchar_t* pMapPopUpInventoryText[] =
{
	L"Inwentarz",
	L"Zamknij",
};

// town strings

const wchar_t* pwTownInfoStrings[] =
{
	L"Rozmiar",					// 0 // size of the town in sectors
	L"", 						// blank line, required
	L"Pod kontrolπ", 					// how much of town is controlled
	L"Brak", 					// none of this town
	L"Przynaleøna kopalnia", 				// mine associated with this town
	L"LojalnoúÊ",					// 5 // the loyalty level of this town
	L"Wyszkolonych", 					// the forces in the town trained by the player
	L"",
	L"G≥Ûwne obiekty", 				// main facilities in this town
	L"Poziom", 					// the training level of civilians in this town
	L"Szkolenie cywili",				// 10 // state of civilian training in town
	L"Samoobrona", 					// the state of the trained civilians in the town
};

// Mine strings

const wchar_t* pwMineStrings[] =
{
	L"Kopalnia",						// 0
	L"Srebro",
	L"Z≥oto",
	L"Dzienna produkcja",
	L"Moøliwa produkcja",
	L"Opuszczona",				// 5
	L"ZamkniÍta",
	L"Na wyczerpaniu",
	L"Produkuje",
	L"Stan",
	L"Tempo produkcji",
	L"Typ z≥oøa",				// 10
	L"Kontrola miasta",
	L"LojalnoúÊ miasta",
//	L"GÛrnicy",
};

// blank sector strings

const wchar_t* pwMiscSectorStrings[] =
{
	L"Si≥y wroga",
	L"Sektor",
	L"PrzedmiotÛw",
	L"Nieznane",
	L"Pod kontrolπ",
	L"Tak",
	L"Nie",
};

// error strings for inventory

const wchar_t* pMapInventoryErrorString[] =
{
	L"%ls jest zbyt daleko.",	//Merc is in sector with item but not close enough
	L"Nie moøna wybraÊ tego najemnika.",  //MARK CARTER
	L"%ls nie moøe stπd zabraÊ tego przedmiotu, gdyø nie jest w tym sektorze.",
	L"Podczas walki nie moøna korzystaÊ z tego panelu.",
	L"Podczas walki nie moøna korzystaÊ z tego panelu.",
	L"%ls nie moøe tu zostawiÊ tego przedmiotu, gdyø nie jest w tym sektorze.",
};

const wchar_t* pMapInventoryStrings[] =
{
	L"Po≥oøenie", 			// sector these items are in
	L"Razem przedmiotÛw", 		// total number of items in sector
};


// help text for the user

const wchar_t* pMapScreenFastHelpTextList[] =
{
	L"Kliknij w kolumnie 'Przydz.', aby przydzieliÊ najemnika do innego oddzia≥u lub wybranego zadania.",
	L"Aby wyznaczyÊ najemnikowi cel w innym sektorze, kliknij pole w kolumnie 'Cel'.",
	L"Gdy najemnicy otrzymajπ juø rozkaz przemieszczenia siÍ, kompresja czasu pozwala im szybciej dotrzeÊ na miejsce.",
	L"Kliknij lewym klawiszem aby wybraÊ sektor. Kliknij ponownie aby wydaÊ najemnikom rozkazy przemieszczenia, lub kliknij prawym klawiszem by uzyskaÊ informacje o sektorze.",
	L"Naciúnij w dowolnym momencie klawisz 'H' by wyúwietliÊ okienko pomocy.",
	L"PrÛbny tekst",
	L"PrÛbny tekst",
	L"PrÛbny tekst",
	L"PrÛbny tekst",
	L"Niewiele moøesz tu zrobiÊ, dopÛki najemnicy nie przylecπ do Arulco. Gdy juø zbierzesz swÛj oddzia≥, kliknij przycisk Kompresji Czasu, w prawym dolnym rogu. W ten sposÛb twoi najemnicy szybciej dotrπ na miejsce.",
};

// movement menu text

const wchar_t* pMovementMenuStrings[] =
{
	L"PrzemieúÊ najemnikÛw", 	// title for movement box
	L"Nanieú trasÍ podrÛøy", 		// done with movement menu, start plotting movement
	L"Anuluj", 		// cancel this menu
	L"Inni",		// title for group of mercs not on squads nor in vehicles
};


const wchar_t* pUpdateMercStrings[] =
{
	L"Oj:", 			// an error has occured
	L"Wygas≥ kontrakt najemnikÛw:", 	// this pop up came up due to a merc contract ending
	L"Najemnicy wype≥nili zadanie:", // this pop up....due to more than one merc finishing assignments
	L"Najemnicy wrÛcili do pracy:", // this pop up ....due to more than one merc waking up and returing to work
	L"Odpoczywajπcy najemnicy:", // this pop up ....due to more than one merc being tired and going to sleep
	L"WkrÛtce wygasnπ kontrakty:", 	// this pop up came up due to a merc contract ending
};

// map screen map border buttons help text

const wchar_t* pMapScreenBorderButtonHelpText[] =
{
	L"Pokaø miasta (|W)",
	L"Pokaø kopalnie (|M)",
	L"Pokaø oddzia≥y i wrogÛw (|T)",
	L"Pokaø przestrzeÒ powietrznπ (|A)",
	L"Pokaø przedmioty (|I)",
	L"Pokaø samoobronÍ i wrogÛw (|Z)",
};


const wchar_t* pMapScreenBottomFastHelp[] =
{
	L"|Laptop",
	L"Ekran taktyczny (|E|s|c)",
	L"|Opcje",
	L"Kompresja czasu (|+)", 	// time compress more
	L"Kompresja czasu (|-)", 	// time compress less
	L"Poprzedni komunikat (|S|t|r|z|a|≥|k|a |w |g|Û|r|Í)\nPoprzednia strona (|P|g|U|p)", 	// previous message in scrollable list
	L"NastÍpny komunikat (|S|t|r|z|a|≥|k|a |w |d|Û|≥)\nNastÍpna strona (|P|g|D|n)", 	// next message in the scrollable list
	L"W≥πcz/Wy≥πcz kompresjÍ czasu (|S|p|a|c|j|a)",	// start/stop time compression
};

const wchar_t* pMapScreenBottomText[] =
{
	L"Saldo dostÍpne", 	// current balance in player bank account
};

const wchar_t* pMercDeadString[] =
{
	L"%ls nie øyje.",
};


const wchar_t* pDayStrings[] =
{
	L"DzieÒ",
};

// the list of email sender names

const wchar_t* pSenderNameList[] =
{
	L"Enrico",
	L"Psych Pro Inc",
	L"Pomoc",
	L"Psych Pro Inc",
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
	L"M.I.S. Ubezpieczenia",
	L"Bobby Ray",
	L"Kingpin",
	L"John Kulba",
	L"A.I.M.",
};


// new mail notify string

const wchar_t* pNewMailStrings[] =
{
 L"Masz nowπ pocztÍ...",
};


// confirm player's intent to delete messages

const wchar_t* pDeleteMailStrings[] =
{
 L"UsunπÊ wiadomoúÊ?",
 L"UsunπÊ wiadomoúÊ?",
};


// the sort header strings

const wchar_t* pEmailHeaders[] =
{
 	L"Od:",
 	L"Temat:",
 	L"DzieÒ:",
};

// email titlebar text

const wchar_t* pEmailTitleText[] =
{
 	L"Skrzynka odbiorcza",
};


// the financial screen strings
const wchar_t* pFinanceTitle[] =
{
	L"KsiÍgowy Plus",		//the name we made up for the financial program in the game
};

const wchar_t* pFinanceSummary[] =
{
	L"Wyp≥ata:", 				// credit (subtract from) to player's account
	L"Wp≥ata:", 				// debit (add to) to player's account
	L"Wczorajsze wp≥ywy:",
	L"Wczorajsze dodatkowe wp≥ywy:",
	L"Wczorajsze wydatki:",
	L"Saldo na koniec dnia:",
	L"Dzisiejsze wp≥ywy:",
	L"Dzisiejsze dodatkowe wp≥ywy:",
	L"Dzisiejsze wydatki:",
	L"Saldo dostÍpne:",
	L"Przewidywane wp≥ywy:",
	L"Przewidywane saldo:", 		// projected balance for player for tommorow
};


// headers to each list in financial screen

const wchar_t* pFinanceHeaders[] =
{
	L"DzieÒ", 					// the day column
	L"Ma", 				// the credits column
	L"Winien",				// the debits column
	L"Transakcja", 			// transaction type - see TransactionText below
	L"Saldo", 				// balance at this point in time
	L"Strona", 				// page number
	L"DzieÒ (dni)", 		// the day(s) of transactions this page displays
};


const wchar_t* pTransactionText[] =
{
	L"Naros≥e odsetki",			// interest the player has accumulated so far
	L"Anonimowa wp≥ata",
	L"Koszt transakcji",
	L"WynajÍto -", 				// Merc was hired
	L"Zakupy u Bobby'ego Ray'a", 		// Bobby Ray is the name of an arms dealer
	L"Uregulowanie rachunkÛw w M.E.R.C.",
	L"Zastaw na øycie dla - %ls", 		// medical deposit for merc
	L"Analiza profilu w IMP", 		// IMP is the acronym for International Mercenary Profiling
	L"Ubezpieczneie dla - %ls",
	L"Redukcja ubezp. dla - %ls",
	L"Przed≥. ubezp. dla - %ls", 				// johnny contract extended
	L"Anulowano ubezp. dla - %ls",
	L"Odszkodowanie za - %ls", 		// insurance claim for merc
	L"1 dzieÒ", 				// merc's contract extended for a day
	L"1 tydzieÒ", 				// merc's contract extended for a week
	L"2 tygodnie", 				// ... for 2 weeks
	L"PrzychÛd z kopalni",
	L"", //String nuked
	L"Zakup kwiatÛw",
	L"Pe≥ny zwrot zastawu za - %ls",
	L"CzÍúciowy zwrot zastawu za - %ls",
	L"Brak zwrotu zastawu za - %ls",
	L"Zap≥ata dla - %ls",		// %ls is the name of the npc being paid
	L"Transfer funduszy do - %ls", 			// transfer funds to a merc
	L"Transfer funduszy od - %ls", 		// transfer funds from a merc
	L"Samoobrona w - %ls", // initial cost to equip a town's militia
	L"Zakupy u - %ls.",	//is used for the Shop keeper interface.  The dealers name will be appended to the end of the string.
	L"%ls wp≥aci≥(a) pieniπdze.",
};

const wchar_t* pTransactionAlternateText[] =
{
	L"Ubezpieczenie dla -", 				// insurance for a merc
	L"Przed≥. kontrakt z - %ls o 1 dzieÒ.", 				// entend mercs contract by a day
	L"Przed≥. kontrakt z - %ls o 1 tydzieÒ.",
	L"Przed≥. kontrakt z - %ls o 2 tygodnie.",
};

// helicopter pilot payment

const wchar_t* pSkyriderText[] =
{
	L"Skyriderowi zap≥acono %d$", 			// skyrider was paid an amount of money
	L"Skyriderowi trzeba jeszcze zap≥aciÊ %d$", 		// skyrider is still owed an amount of money
	L"Skyrider zatankowa≥",	// skyrider has finished refueling
	L"",//unused
	L"",//unused
	L"Skyrider jest gotÛw do kolejnego lotu.", // Skyrider was grounded but has been freed
	L"Skyrider nie ma pasaøerÛw. Jeúli chcesz przetransportowaÊ najemnikÛw, zmieÒ ich przydzia≥ na POJAZD/HELIKOPTER.",
};


// strings for different levels of merc morale

const wchar_t* pMoralStrings[] =
{
 L"åwietne",
 L"Dobre",
 L"Stabilne",
 L"S≥abe",
 L"Panika",
 L"Z≥e",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.

const wchar_t* pLeftEquipmentString[] =
{
	L"%ls - jego/jej sprzÍt jest juø w Omercie( A9 ).",
	L"%ls - jego/jej sprzÍt jest juø w Drassen( B13 ).",
};

// Status that appears on the Map Screen

const wchar_t* pMapScreenStatusStrings[] =
{
	L"Zdrowie",
	L"Energia",
	L"Morale",
	L"Stan",	// the condition of the current vehicle (its "health")
	L"Paliwo",	// the fuel level of the current vehicle (its "energy")
};


const wchar_t* pMapScreenPrevNextCharButtonHelpText[] =
{
	L"Poprzedni najemnik (|S|t|r|z|a|≥|k|a |w |l|e|w|o)", 			// previous merc in the list
	L"NastÍpny najemnik (|S|t|r|z|a|≥|k|a |w |p|r|a|w|o)", 				// next merc in the list
};


const wchar_t* pEtaString[] =
{
	L"PCP:", 				// eta is an acronym for Estimated Time of Arrival
};

const wchar_t* pTrashItemText[] =
{
	L"WiÍcej tego nie zobaczysz. Czy na pewno chcesz to zrobiÊ?", 	// do you want to continue and lose the item forever
	L"To wyglπda na coú NAPRAWD  waønego. Czy NA PEWNO chcesz to zniszczyÊ?", // does the user REALLY want to trash this item
};


const wchar_t* pMapErrorString[] =
{
	L"Oddzia≥ nie moøe siÍ przemieszczaÊ, jeúli ktÛryú z najemnikÛw úpi.",

//1-5
	L"Najpierw wyprowadü oddzia≥ na powierzchniÍ.",
	L"Rozkazy przemieszczenia? To jest sektor wroga!",
	L"Aby podrÛøowaÊ najemnicy muszπ byÊ przydzieleni do oddzia≥u lub pojazdu.",
	L"Nie masz jeszcze ludzi.", 		// you have no members, can't do anything
	L"Najemnik nie moøe wype≥niÊ tego rozkazu.",			 		// merc can't comply with your order
//6-10
	L"musi mieÊ eskortÍ, aby siÍ przemieszczaÊ. UmieúÊ go w oddziale z eskortπ.", // merc can't move unescorted .. for a male
	L"musi mieÊ eskortÍ, aby siÍ przemieszczaÊ. UmieúÊ jπ w oddziale z eskortπ.", // for a female
	L"Najemnik nie przyby≥ jeszcze do Arulco!",
	L"Wyglπda na to, øe trzeba wpierw uregulowaÊ sprawy kontraktu.",
	L"",
//11-15
	L"Rozkazy przemieszczenia? Trwa walka!",
	L"Zaatakowa≥y ciÍ dzikie koty, w sektorze %ls!",
	L"W sektorze I16 znajduje siÍ coú, co wyglπda na legowisko dzikich kotÛw!",
	L"",
	L"Baza rakiet Ziemia-Powietrze zosta≥a przejÍta.",
//16-20
	L"%ls - kopalnia zosta≥a przejÍta. TwÛj dzienny przychÛd zosta≥ zredukowany do %ls.",
	L"Nieprzyjaciel bezkonfliktowo przejπ≥ sektor %ls.",
	L"Przynajmniej jeden z twoich najemnikÛw nie zosta≥ do tego przydzielony.",
	L"%ls nie moøe siÍ przy≥πczyÊ, poniewaø %ls jest pe≥ny",
	L"%ls nie moøe siÍ przy≥πczyÊ, poniewaø %ls jest zbyt daleko.",
//21-25
	L"%ls - kopalnia zosta≥a przejÍta przez si≥y Deidranny!",
	L"Si≥y Deidranny w≥aúnie zaatakowa≥y bazÍ rakiet Ziemia-Powietrze w - %ls.",
	L"Si≥y Deidranny w≥aúnie zaatakowa≥y - %ls.",
	L"W≥aúnie zauwaøono si≥y Deidranny w - %ls.",
	L"Si≥y Deidranny w≥aúnie przejÍ≥y - %ls.",
//26-30
	L"Przynajmniej jeden z twoich najemnikÛw nie mÛg≥ siÍ po≥oøyÊ spaÊ.",
	L"Przynajmniej jeden z twoich najemnikÛw nie mÛg≥ wstaÊ.",
	L"Oddzia≥y samoobrony nie pojawiπ siÍ dopÛki nie zostanπ wyszkolone.",
	L"%ls nie moøe siÍ w tej chwili przemieszczaÊ.",
	L"Øo≥nierze samoobrony, ktÛrzy znajdujπ siÍ poza granicami miasta, nie mogπ byÊ przeniesieni do innego sektora.",
//31-35
	L"Nie moøesz trenowaÊ samoobrony w - %ls.",
	L"Pusty pojazd nie moøe siÍ poruszaÊ!",
	L"%ls ma zbyt wiele ran by podrÛøowaÊ!",
	L"Musisz wpierw opuúciÊ muzeum!",
	L"%ls nie øyje!",
//36-40
	L"%ls nie moøe siÍ zamieniÊ z - %ls, poniewaø siÍ porusza",
	L"%ls nie moøe w ten sposÛb wejúc do pojazdu",
	L"%ls nie moøe siÍ do≥πczyÊ do - %ls",
	L"Nie moøesz kompresowaÊ czasu dopÛki nie zatrudnisz sobie kilku nowych najemnikÛw!",
	L"Ten pojazd moøe siÍ poruszaÊ tylko po drodze!",
//41-45
	L"Nie moøna zmieniaÊ przydzia≥u najemnikÛw, ktÛrzy sπ w drodze",
	L"Pojazd nie ma paliwa!",
	L"%ls jest zbyt zmÍczony(na) by podrÛøowaÊ.",
	L"Øaden z pasaøerÛw nie jest w stanie kierowaÊ tym pojazdem.",
	L"Jeden lub wiÍcej cz≥onkÛw tego oddzia≥u nie moøe siÍ w tej chwili przemieszczaÊ.",
//46-50
	L"Jeden lub wiÍcej INNYCH cz≥onkÛw tego oddzia≥u nie moøe siÍ w tej chwili przemieszczaÊ.",
	L"Pojazd jest uszkodzony!",
	L"PamiÍtaj, øe w jednym sektorze tylko dwÛch najemnikÛw moøe trenowaÊ øo≥nierzy samoobrony.",
	L"Robot nie moøe siÍ poruszaÊ bez operatora. UmieúÊ ich razem w jednym oddziale.",
};


// help text used during strategic route plotting
const wchar_t* pMapPlotStrings[] =
{
	L"Kliknij ponownie sektor docelowy, aby zatwierdziÊ trasÍ podrÛøy, lub kliknij inny sektor, aby jπ wyd≥uøyÊ.",
	L"Trasa podrÛøy zatwierdzona.",
	L"Cel podrÛøy nie zosta≥ zmieniony.",
	L"Trasa podrÛøy zosta≥a anulowana.",
	L"Trasa podrÛøy zosta≥a skrÛcona.",
};


// help text used when moving the merc arrival sector
const wchar_t* pBullseyeStrings[] =
{
	L"Kliknij sektor, do ktÛrego majπ przylatywaÊ najemnicy.",
	L"Dobrze. Przylatujπcy najemnicy bÍdπ zrzucani w %ls",
	L"Najemnicy nie mogπ tu przylatywaÊ. PrzestrzeÒ powietrzna nie jest zabezpieczona!",
	L"Anulowano. Sektor zrzutu nie zosta≥ zmieniony.",
	L"PrzestrzeÒ powietrzna nad %ls nie jest juø bezpieczna! Sektor zrzutu zosta≥ przesuniÍty do %ls.",
};


// help text for mouse regions

const wchar_t* pMiscMapScreenMouseRegionHelpText[] =
{
	L"OtwÛrz wyposaøenie (|E|n|t|e|r)",
	L"Zniszcz przedmiot",
	L"Zamknij wyposaøenie (|E|n|t|e|r)",
};



// male version of where equipment is left
const wchar_t* pMercHeLeaveString[] =
{
	L"Czy %ls ma zostawiÊ swÛj sprzÍt w sektorze, w ktÛrym siÍ obecnie znajduje (%ls), czy w Dressen (B13), skπd odlatuje? ",
	L"Czy %ls ma zostawiÊ swÛj sprzÍt w sektorze, w ktÛrym siÍ obecnie znajduje (%ls), czy w Omercie (A9), skπd odlatuje?",
	L"wkrÛtce odchodzi i zostawi swÛj sprzÍt w Omercie (A9).",
	L"wkrÛtce odchodzi i zostawi swÛj sprzÍt w Drassen (B13).",
	L"%ls wkrÛtce odchodzi i zostawi swÛj sprzÍt w %ls.",
};


// female version
const wchar_t* pMercSheLeaveString[] =
{
	L"Czy %ls ma zostawiÊ swÛj sprzÍt w sektorze, w ktÛrym siÍ obecnie znajduje (%ls), czy w Dressen (B13), skπd odlatuje? ",
	L"Czy %ls ma zostawiÊ swÛj sprzÍt w sektorze, w ktÛrym siÍ obecnie znajduje (%ls), czy w Omercie (A9), skπd odlatuje?",
	L"wkrÛtce odchodzi i zostawi swÛj sprzÍt w Omercie (A9).",
	L"wkrÛtce odchodzi i zostawi swÛj sprzÍt w Drassen (B13).",
	L"%ls wkrÛtce odchodzi i zostawi swÛj sprzÍt w %ls.",
};


const wchar_t* pMercContractOverStrings[] =
{
	L" zakoÒczy≥ kontrakt wiÍc wyjecha≥.", 		// merc's contract is over and has departed
	L" zakoÒczy≥a kontrakt wiÍc wyjecha≥a.", 		// merc's contract is over and has departed
	L" - jego kontrakt zosta≥ zerwany wiÍc odszed≥.", 		// merc's contract has been terminated
	L" - jej kontrakt zosta≥ zerwany wiÍc odesz≥a.",		// merc's contract has been terminated
	L"Masz za duøy d≥ug wobec M.E.R.C. wiÍc %ls odchodzi.", // Your M.E.R.C. account is invalid so merc left
};

// Text used on IMP Web Pages

const wchar_t* pImpPopUpStrings[] =
{
	L"Nieprawid≥owy kod dostÍpu",
	L"Czy na pewno chcesz wznowiÊ proces okreúlenia profilu?",
	L"Wprowadü nazwisko oraz p≥eÊ",
	L"WstÍpna kontrola stanu twoich finansÛw wykaza≥a, øe nie staÊ ciÍ na analizÍ profilu.",
	L"Opcja tym razem nieaktywna.",
	L"Aby wykonaÊ profil, musisz mieÊ miejsce dla przynajmniej jednego cz≥onka za≥ogi.",
	L"Profil zosta≥ juø wykonany.",
};


// button labels used on the IMP site

const wchar_t* pImpButtonText[] =
{
	L"O Nas", 			// about the IMP site
	L"ZACZNIJ", 			// begin profiling
	L"OsobowoúÊ", 		// personality section
	L"Atrybuty", 		// personal stats/attributes section
	L"Portret", 			// the personal portrait selection
	L"G≥os %d", 			// the voice selection
	L"Gotowe", 			// done profiling
	L"Zacznij od poczπtku", 		// start over profiling
	L"Tak, wybieram tπ odpowiedü.",
	L"Tak",
	L"Nie",
	L"SkoÒczone", 			// finished answering questions
	L"Poprz.", 			// previous question..abbreviated form
	L"Nast.", 			// next question
	L"TAK, JESTEM.", 		// yes, I am certain
	L"NIE, CHC  ZACZ•∆ OD NOWA.", // no, I want to start over the profiling process
	L"TAK",
	L"NIE",
	L"Wstecz", 			// back one page
	L"Anuluj", 			// cancel selection
	L"Tak.",
	L"Nie, ChcÍ spojrzeÊ jeszcze raz.",
	L"Rejestr", 			// the IMP site registry..when name and gender is selected
	L"AnalizujÍ...", 			// analyzing your profile results
	L"OK",
	L"G≥os",
};

const wchar_t* pExtraIMPStrings[] =
{
	L"Aby zaczπÊ analizÍ profilu, wybierz osobowoúÊ.",
	L"Teraz okreúl swoje atrybuty.",
	L"Teraz moøesz przystπpiÊ do wyboru portretu.",
	L"Aby zakoÒczyÊ proces, wybierz prÛbkÍ g≥osu, ktÛra ci najbardziej odpowiada."
};

const wchar_t* pFilesTitle[] =
{
	L"Przeglπdarka plikÛw",
};

const wchar_t* pFilesSenderList[] =
{
	L"Raport Rozp.", 		// the recon report sent to the player. Recon is an abbreviation for reconissance
	L"Intercept #1", 		// first intercept file .. Intercept is the title of the organization sending the file...similar in function to INTERPOL/CIA/KGB..refer to fist record in files.txt for the translated title
	L"Intercept #2",	   // second intercept file
	L"Intercept #3",			 // third intercept file
	L"Intercept #4", // fourth intercept file
	L"Intercept #5", // fifth intercept file
	L"Intercept #6", // sixth intercept file
};

// Text having to do with the History Log

const wchar_t* pHistoryTitle[] =
{
	L"Historia",
};

const wchar_t* pHistoryHeaders[] =
{
	L"DzieÒ", 			// the day the history event occurred
	L"Strona", 			// the current page in the history report we are in
	L"DzieÒ", 			// the days the history report occurs over
	L"Po≥oøenie", 			// location (in sector) the event occurred
	L"Zdarzenie", 			// the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
const wchar_t* pHistoryStrings[] =
{
	L"",																						// leave this line blank
	//1-5
	L"%ls najÍty(ta) w A.I.M.", 										// merc was hired from the aim site
	L"%ls najÍty(ta) w M.E.R.C.", 									// merc was hired from the aim site
	L"%ls ginie.", 															// merc was killed
	L"Uregulowano rachunki w M.E.R.C.",								// paid outstanding bills at MERC
	L"PrzyjÍto zlecenie od Enrico Chivaldori",
	//6-10
	L"Profil IMP wygenerowany",
	L"Podpisano umowÍ ubezpieczeniowπ dla %ls.", 				// insurance contract purchased
	L"Anulowano umowÍ ubezpieczeniowπ dla %ls.", 				// insurance contract canceled
	L"Wyp≥ata ubezpieczenia za %ls.", 							// insurance claim payout for merc
	L"Przed≥uøono kontrakt z: %ls o 1 dzieÒ.", 						// Extented "mercs name"'s for a day
	//11-15
	L"Przed≥uøono kontrakt z: %ls o 1 tydzieÒ.", 					// Extented "mercs name"'s for a week
	L"Przed≥uøono kontrakt z: %ls o 2 tygodnie.", 					// Extented "mercs name"'s 2 weeks
	L"%ls zwolniony(na).", 													// "merc's name" was dismissed.
	L"%ls odchodzi.", 																		// "merc's name" quit.
	L"przyjÍto zadanie.", 															// a particular quest started
	//16-20
	L"zadanie wykonane.",
	L"Rozmawiano szefem kopalni %ls",									// talked to head miner of town
	L"Wyzwolono - %ls",
	L"Uøyto kodu Cheat",
	L"ØywnoúÊ powinna byÊ jutro w Omercie",
	//21-25
	L"%ls odchodzi, aby wziπÊ úlub z Darylem Hickiem",
	L"Wygas≥ kontrakt z - %ls.",
	L"%ls zrekrutowany(na).",
	L"Enrico narzeka na brak postÍpÛw",
	L"Walka wygrana",
	//26-30
	L"%ls - w kopalni koÒczy siÍ ruda",
	L"%ls - w kopalni skoÒczy≥a siÍ ruda",
	L"%ls - kopalnia zosta≥a zamkniÍta",
	L"%ls - kopalnia zosta≥a otwarta",
	L"Informacja o wiÍzieniu zwanym Tixa.",
	//31-35
	L"Informacja o tajnej fabryce broni zwanej Orta.",
	L"Naukowiec w Orcie ofiarowa≥ kilka karabinÛw rakietowych.",
	L"KrÛlowa Deidranna robi uøytek ze zw≥ok.",
	L"Frank opowiedzia≥ o walkach w San Monie.",
	L"Pewien pacjent twierdzi, øe widzia≥ coú w kopalni.",
	//36-40
	L"GoúÊ o imieniu Devin sprzedaje materia≥y wybuchowe.",
	L"Spotkanie ze s≥awynm eks-najemnikiem A.I.M. - Mike'iem!",
	L"Tony handluje broniπ.",
	L"Otrzymano karabin rakietowy od sierøanta Krotta.",
	L"Dano Kyle'owi akt w≥asnoúci sklepu Angela.",
	//41-45
	L"Madlab zaoferowa≥ siÍ zbudowaÊ robota.",
	L"Gabby potrafi zrobiÊ miksturÍ chroniπcπ przed robakami.",
	L"Keith wypad≥ z interesu.",
	L"Howard dostarcza≥ cyjanek krÛlowej Deidrannie.",
	L"Spotkanie z handlarzem Keithem w Cambrii.",
	//46-50
	L"Spotkanie z aptekarzem Howardem w Balime",
	L"Spotkanie z Perko, prowadzπcym ma≥y warsztat.",
	L"Spotkanie z Samem z Balime - prowadzi sklep z narzÍdziami.",
	L"Franz handluje sprzÍtem elektronicznym.",
	L"Arnold prowadzi warsztat w Grumm.",
	//51-55
	L"Fredo naprawia sprzÍt elektroniczny w Grumm.",
	L"Otrzymano darowiznÍ od bogatego goúcia w Balime.",
	L"Spotkano Jake'a, ktÛry prowadzi z≥omowisko.",
	L"Jakiú w≥ÛczÍga da≥ nam elektronicznπ kartÍ dostÍpu.",
	L"Przekupiono Waltera, aby otworzy≥ drzwi do piwnicy.",
	//56-60
	L"Dave oferuje darmowe tankowania, jeúli bÍdzie mia≥ paliwo.",
	L"Greased Pablo's palms.",
	L"Kingpin trzyma pieniπdze w kopalni w San Mona.",
	L"%ls wygra≥(a) walkÍ",
	L"%ls przegra≥(a) walkÍ",
	//61-65
	L"%ls zdyskwalifikowany(na) podczas walki",
	L"Znaleziono duøo pieniÍdzy w opuszczonej kopalni.",
	L"Spotkano zabÛjcÍ nas≥anego przez Kingpina.",
	L"Utrata kontroli nad sektorem",				//ENEMY_INVASION_CODE
	L"Sektor obroniony",
	//66-70
	L"Przegrana bitwa",							//ENEMY_ENCOUNTER_CODE
	L"Fatalna zasadzka",						//ENEMY_AMBUSH_CODE
	L"Usunieto zasadzkÍ wroga",
	L"Nieudany atak",			//ENTERING_ENEMY_SECTOR_CODE
	L"Udany atak!",
	//71-75
	L"Stworzenia zaatakowa≥y",			//CREATURE_ATTACK_CODE
	L"Zabity(ta) przez dzikie koty",			//BLOODCAT_AMBUSH_CODE
	L"WyrøniÍto dzikie koty",
	L"%ls zabity(ta)",
	L"Przekazano Carmenowi g≥owÍ terrorysty",
	L"Slay odszed≥",
	L"Zabito: %ls",
};

const wchar_t* pHistoryLocations[] =
{
	L"N/D",						// N/A is an acronym for Not Applicable
};

// icon text strings that appear on the laptop

const wchar_t* pLaptopIcons[] =
{
	L"E-mail",
	L"SieÊ",
	L"Finanse",
	L"Personel",
	L"Historia",
	L"Pliki",
	L"Zamknij",
	L"sir-FER 4.0",			// our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added

const wchar_t* pBookMarkStrings[] =
{
	L"A.I.M.",
	L"Bobby Ray's",
	L"I.M.P",
	L"M.E.R.C.",
	L"Pogrzeby",
	L"Kwiaty",
	L"Ubezpieczenia",
	L"Anuluj",
};

// When loading or download a web page

const wchar_t* pDownloadString[] =
{
	L"£adowanie strony...",
	L"Otwieranie strony...",
};

//This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

const wchar_t* gsAtmStartButtonText[] =
{
	L"Atrybuty", 			// view stats of the merc
	L"Wyposaøenie", 			// view the inventory of the merc
	L"Zatrudnienie",
};

// Web error messages. Please use foreign language equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator

const wchar_t* pErrorStrings[] =
{
	L"Niestabilne po≥πczenie z Hostem. Transfer moøe trwaÊ d≥uøej.",
};


const wchar_t* pPersonnelString[] =
{
	L"Najemnicy:", 			// mercs we have
};


const wchar_t* pWebTitle[ ]=
{
	L"sir-FER 4.0",		// our name for thL"sir-FER 4.0",		// our name for the version of the browser, play on company name
};


// The titles for the web program title bar, for each page loaded

const wchar_t* pWebPagesTitles[] =
{
	L"A.I.M.",
	L"A.I.M. Cz≥onkowie",
	L"A.I.M. Portrety",		// a mug shot is another name for a portrait
	L"A.I.M. Lista",
	L"A.I.M.",
	L"A.I.M. Weterani",
	L"A.I.M. Polisy",
	L"A.I.M. Historia",
	L"A.I.M. Linki",
	L"M.E.R.C.",
	L"M.E.R.C. Konta",
	L"M.E.R.C. Rejestracja",
	L"M.E.R.C. Indeks",
	L"Bobby Ray's",
	L"Bobby Ray's - BroÒ",
	L"Bobby Ray's - Amunicja",
	L"Bobby Ray's - Pancerz",
	L"Bobby Ray's - RÛøne",							//misc is an abbreviation for miscellaneous
	L"Bobby Ray's - Uøywane",
	L"Bobby Ray's - ZamÛwienie pocztowe",
	L"I.M.P.",
	L"I.M.P.",
	L"United Floral Service",
	L"United Floral Service - Galeria",
	L"United Floral Service - ZamÛwienie",
	L"United Floral Service - Galeria kartek",
	L"Malleus, Incus & Stapes - Brokerzy ubezpieczeniowi",
	L"Informacja",
	L"Kontrakt",
	L"Uwagi",
	L"McGillicutty - Zak≥ad pogrzebowy",
	L"",
	L"Nie odnaleziono URL.",
	L"Bobby Ray's - Ostatnie dostawy",
	L"",
	L"",
};

const wchar_t* pShowBookmarkString[] =
{
	L"Sir-Pomoc",
	L"Kliknij ponownie SieÊ by otworzyÊ menu Ulubione.",
};

const wchar_t* pLaptopTitles[] =
{
	L"Poczta",
	L"Przeglπdarka plikÛw",
	L"Personel",
	L"KsiÍgowy Plus",
	L"Historia",
};

const wchar_t* pPersonnelDepartedStateStrings[] =
{
	//reasons why a merc has left.
	L"åmierÊ w akcji",
	L"Zwolnienie",
	L"Inny",
	L"Ma≥øeÒstwo",
	L"Koniec kontraktu",
	L"Rezygnacja",
};
// personnel strings appearing in the Personnel Manager on the laptop

const wchar_t* pPersonelTeamStrings[] =
{
	L"Bieøπcy oddzia≥",
	L"Wyjazdy",
	L"Koszt dzienny:",
	L"Najwyøszy koszt:",
	L"Najniøszy koszt:",
	L"åmierÊ w akcji:",
	L"Zwolnienie:",
	L"Inny:",
};


const wchar_t* pPersonnelCurrentTeamStatsStrings[] =
{
	L"Najniøszy",
	L"åredni",
	L"Najwyøszy",
};


const wchar_t* pPersonnelTeamStatsStrings[] =
{
	L"ZDR",
	L"ZWN",
	L"ZRCZ",
	L"SI£A",
	L"DOW",
	L"INT",
	L"DOåW",
	L"STRZ",
	L"MECH",
	L"WYB",
	L"MED",
};


// horizontal and vertical indices on the map screen

const wchar_t* pMapVertIndex[] =
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

const wchar_t* pMapHortIndex[] =
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

const wchar_t* pMapDepthIndex[] =
{
	L"",
	L"-1",
	L"-2",
	L"-3",
};

// text that appears on the contract button

const wchar_t* pContractButtonString[] =
{
	L"Kontrakt",
};

// text that appears on the update panel buttons

const wchar_t* pUpdatePanelButtons[] =
{
	L"Dalej",
	L"Stop",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle

const wchar_t LargeTacticalStr[][ LARGE_STRING_LENGTH ] =
{
	L"Pokonano ciÍ w tym sektorze!",
	L"WrÛg nie zna litoúci i poøera was wszystkich!",
	L"Nieprzytomni cz≥onkowie twojego oddzia≥u zostali pojmani!",
	L"Cz≥onkowie twojego oddzia≥u zostali uwiÍzieni.",
};


//Insurance Contract.c
//The text on the buttons at the bottom of the screen.

const wchar_t* InsContractText[] =
{
	L"Wstecz",
	L"Dalej",
	//L"AkceptujÍ",
	L"OK",
	L"Skasuj",
};



//Insurance Info
// Text on the buttons on the bottom of the screen

const wchar_t* InsInfoText[] =
{
	L"Wstecz",
	L"Dalej"
};



//For use at the M.E.R.C. web site. Text relating to the player's account with MERC

const wchar_t* MercAccountText[] =
{
	// Text on the buttons on the bottom of the screen
	L"Autoryzacja",
	L"Strona g≥Ûwna",
	L"Konto #:",
	L"Najemnik",
	L"Dni",
	L"Stawka",	//5
	L"Op≥ata",
	L"Razem:",
	L"Czy na pewno chcesz zatwierdziÊ p≥atnoúÊ: %ls?",		//the %ls is a string that contains the dollar amount ( ex. "$150" )
};



//For use at the M.E.R.C. web site. Text relating a MERC mercenary


const wchar_t* MercInfo[] =
{
	L"Zdrowie",
	L"ZwinnoúÊ",
	L"SprawnoúÊ",
	L"Si≥a",
	L"Um. dowodz.",
	L"Inteligencja",
	L"Poz. doúwiadczenia",
	L"Um. strzeleckie",
	L"Zn. mechaniki",
	L"Mat. wybuchowe",
	L"Wiedza medyczna",

	L"Poprzedni",
	L"Najmij",
	L"NastÍpny",
	L"Dodatkowe informacje",
	L"Strona g≥Ûwna",
	L"NajÍty",
	L"Koszt:",
	L"Dziennie",
	L"Nie øyje",

	L"Wyglπda na to, øe chcesz wynajπÊ zbyt wielu najemnikÛw. Limit wynosi 18.",
	L"NiedostÍpny",
};



// For use at the M.E.R.C. web site. Text relating to opening an account with MERC

const wchar_t* MercNoAccountText[] =
{
	//Text on the buttons at the bottom of the screen
	L"OtwÛrz konto",
	L"Anuluj",
	L"Nie posiadasz konta. Czy chcesz sobie za≥oøyÊ?"
};



// For use at the M.E.R.C. web site. MERC Homepage

const wchar_t* MercHomePageText[] =
{
	//Description of various parts on the MERC page
	L"Speck T. Kline, za≥oøyciel i w≥aúciciel",
	L"Aby otworzyÊ konto naciúnij tu",
	L"Aby zobaczyÊ konto naciúnij tu",
	L"Aby obejrzeÊ akta naciúnij tu",
	// The version number on the video conferencing system that pops up when Speck is talking
	L"Speck Com v3.2",
};

// For use at MiGillicutty's Web Page.

const wchar_t* sFuneralString[] =
{
	L"Zak≥ad pogrzebowy McGillicutty, pomaga rodzinom pogrπøonym w smutku od 1983.",
	L"Kierownik, by≥y najemnik A.I.M. Murray \'Pops\' McGillicutty jest doúwiadczonym pracownikiem zak≥adu pogrzebowego.",
	L"Przez ca≥e øycie obcowa≥ ze úmierciπ, 'Pops' wie jak trudne sπ te chwile.",
	L"Zak≥ad pogrzebowy McGillicutty oferuje szeroki zakres us≥ug, od duchowego wsparcia po rekonstrukcjÍ silnie zniekszta≥conych zw≥ok.",
	L"PozwÛl by McGillicutty ci pomÛg≥ a twÛj ukochany bÍdzie spoczywa≥ w pokoju.",

	// Text for the various links available at the bottom of the page
	L"WYåLIJ KWIATY",
	L"KOLEKCJA TRUMIEN I URN",
	L"US£UGI KREMA- CYJNE",
	L"US£UGI PLANOWANIA POGRZEBU",
	L"KARTKI POGRZE- BOWE",

	// The text that comes up when you click on any of the links ( except for send flowers ).
	L"Niestety, z powodu úmierci w rodzinie, nie dzia≥ajπ jeszcze wszystkie elementy tej strony.",
	L"Przepraszamy za powyøsze uniedogodnienie."
};

// Text for the florist Home page

const wchar_t* sFloristText[] =
{
	//Text on the button on the bottom of the page

	L"Galeria",

	//Address of United Florist

	L"\"Zrzucamy z samolotu w dowolnym miejscu\"",
	L"1-555-POCZUJ-MNIE",
	L"Ul. Nosowska 333, Zapuszczone miasto, CA USA 90210",
	L"http://www.poczuj-mnie.com",

	// detail of the florist page

	L"Dzia≥amy szybko i sprawnie!",
	L"Gwarantujemy dostawÍ w dowolny punkt na Ziemi, nastÍpnego dnia po z≥oøeniu zamÛwienia!",
	L"Oferujemy najniøsze ceny na úwiecie!",
	L"Pokaø nam ofertÍ z niøszπ cenπ, a dostaniesz w nagrodÍ tuzin rÛø, za darmo!",
	L"Latajπca flora, fauna i kwiaty od 1981.",
	L"Nasz ozdobiony bombowiec zrzuci twÛj bukiet w promieniu co najwyøej dziesiÍciu mil od øπdanego miejsca. Kiedy tylko zechcesz!",
	L"PozwÛl nam zaspokoiÊ twoje kwieciste fantazje.",
	L"Bruce, nasz úwiatowej renomy projektant bukietÛw, zerwie dla ciebie najúwieøsze i najwspanialsze kwiaty z naszej szklarni.",
	L"I pamiÍtaj, jeúli czegoú nie mamy, moøemy to szybko zasadziÊ!"
};



//Florist OrderForm

const wchar_t* sOrderFormText[] =
{
	//Text on the buttons

	L"PowrÛt",
	L"Wyúlij",
	L"Skasuj",
	L"Galeria",

	L"Nazwa bukietu:",
	L"Cena:",			//5
	L"ZamÛwienie numer:",
	L"Czas dostawy",
	L"nast. dnia",
	L"dostawa gdy to bÍdzie moøliwe",
	L"Miejsce dostawy",			//10
	L"Dodatkowe us≥ugi",
	L"Zgnieciony bukiet($10)",
	L"Czarne RÛøe($20)",
	L"ZwiÍdniÍty bukiet($10)",
	L"Ciasto owocowe (jeøeli bÍdzie)($10)",		//15
	L"Osobiste kondolencje:",
	L"Ze wzglÍdu na rozmiar karteczek, tekst nie moøe zawieraÊ wiÍcej niø 75 znakÛw.",
	L"...moøesz teø przejrzeÊ nasze",

	L"STANDARDOWE KARTKI",
	L"Informacja o rachunku",//20

	//The text that goes beside the area where the user can enter their name

	L"Nazwisko:",
};




//Florist Gallery.c

const wchar_t* sFloristGalleryText[] =
{
	//text on the buttons

	L"Poprz.",	//abbreviation for previous
	L"Nast.",	//abbreviation for next

	L"Kliknij wybranπ pozycjÍ aby z≥oøyÊ zamÛwienie.",
	L"Uwaga: $10 dodatkowej op≥aty za zwiÍdniÍty lub zgnieciony bukiet.",

	//text on the button

	L"G≥Ûwna",
};

//Florist Cards

const wchar_t* sFloristCards[] =
{
	L"Kliknij swÛj wybÛr",
	L"Wstecz"
};



// Text for Bobby Ray's Mail Order Site

const wchar_t* BobbyROrderFormText[] =
{
	L"Formularz zamÛwienia",				//Title of the page
	L"IloúÊ",					// The number of items ordered
	L"Waga (%ls)",			// The weight of the item
	L"Nazwa",				// The name of the item
	L"Cena",				// the item's weight
	L"WartoúÊ",				//5	// The total price of all of items of the same type
	L"W sumie",				// The sub total of all the item totals added
	L"Transport",		// S&H is an acronym for Shipping and Handling
	L"Razem",			// The grand total of all item totals + the shipping and handling
	L"Miejsce dostawy",
	L"Czas dostawy",			//10	// See below
	L"Koszt (za %ls.)",			// The cost to ship the items
	L"Ekspres - 24h",			// Gets deliverd the next day
	L"2 dni robocze",			// Gets delivered in 2 days
	L"Standardowa dostawa",			// Gets delivered in 3 days
	L" WyczyúÊ",//15			// Clears the order page
	L" AkceptujÍ",			// Accept the order
	L"Wstecz",				// text on the button that returns to the previous page
	L"Strona g≥Ûwna",				// Text on the button that returns to the home page
	L"* oznacza uøywane rzeczy",		// Disclaimer stating that the item is used
	L"Nie staÊ ciÍ na to.",		//20	// A popup message that to warn of not enough money
	L"<BRAK>",				// Gets displayed when there is no valid city selected
	L"Miejsce docelowe przesy≥ki: %ls. Potwierdzasz?",		// A popup that asks if the city selected is the correct one
	L"Waga przesy≥ki*",			// Displays the weight of the package
	L"* Min. Waga",				// Disclaimer states that there is a minimum weight for the package
	L"Dostawy",
};


// This text is used when on the various Bobby Ray Web site pages that sell items

const wchar_t* BobbyRText[] =
{
	L"ZamÛw",				// Title

	L"Kliknij wybrane towary. Lewym klawiszem zwiÍkszasz iloúÊ towaru, a prawym zmniejszasz. Gdy juø skompletujesz swoje zakupy przejdü do formularza zamÛwienia.",			// instructions on how to order

	//Text on the buttons to go the various links

	L"Poprzednia",		//
	L"BroÒ", 			//3
	L"Amunicja",			//4
	L"Ochraniacze",			//5
	L"RÛøne",			//6	//misc is an abbreviation for miscellaneous
	L"Uøywane",			//7
	L"NastÍpna",
	L"FORMULARZ",
	L"Strona g≥Ûwna",			//10

	//The following 2 lines are used on the Ammunition page.
	//They are used for help text to display how many items the player's merc has
	//that can use this type of ammo

	L"TwÛj zespÛ≥ posiada",//11
	L"szt. broni do ktÛrej pasuje amunicja tego typu", //12

	//The following lines provide information on the items

	L"Waga:",			// Weight of all the items of the same type
	L"Kal:",			// the caliber of the gun
	L"Mag:",			// number of rounds of ammo the Magazine can hold
	L"Zas:",				// The range of the gun
	L"Si≥a:",				// Damage of the weapon
	L"CS:",			// Weapon's Rate Of Fire, acroymn ROF
	L"Koszt:",			// Cost of the item
	L"Na stanie:",			// The number of items still in the store's inventory
	L"IloúÊ na zamÛw.:",		// The number of items on order
	L"Uszkodz.",			// If the item is damaged
	L"Waga:",			// the Weight of the item
	L"Razem:",			// The total cost of all items on order
	L"* Stan: %%",		// if the item is damaged, displays the percent function of the item

	//Popup that tells the player that they can only order 10 items at a time

	L"Przepraszamy za to utrudnienie, ale na jednym zamÛwieniu moøe siÍ znajdowaÊ tylko 10 pozycji! Jeúli potrzebujesz wiÍcej, z≥Ûø kolejne zamÛwienie.",

	// A popup that tells the user that they are trying to order more items then the store has in stock

	L"Przykro nam. Chwilowo nie mamy tego wiÍcej na magazynie. ProszÍ sprÛbowaÊ pÛüniej.",

	//A popup that tells the user that the store is temporarily sold out

	L"Przykro nam, ale chwilowo nie mamy tego towaru na magazynie",

};


// Text for Bobby Ray's Home Page

const wchar_t* BobbyRaysFrontText[] =
{
	//Details on the web site

	L"Tu znajdziesz nowoúci z dziedziny broni i osprzÍtu wojskowego",
	L"Zaspokoimy wszystkie twoje potrzeby w dziedzinie materia≥Ûw wybuchowych",
	L"UØYWANE RZECZY",

	//Text for the various links to the sub pages

	L"R”ØNE",
	L"BRO—",
	L"AMUNICJA",		//5
	L"OCHRANIACZE",

	//Details on the web site

	L"Jeúli MY tego nie mamy, to znaczy, øe nigdzie tego nie dostaniesz!",
	L"W trakcie budowy",
};



// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug shot page

const wchar_t* AimSortText[] =
{
	L"Cz≥onkowie A.I.M.",				// Title

	L"Sortuj wg:",					// Title for the way to sort

	// sort by...

	L"Ceny",
	L"Doúwiadczenia",
	L"Um. strzeleckich",
	L"Um. med.",
	L"Zn. mat. wyb.",
	L"Zn. mechaniki",

	//Text of the links to other AIM pages

	L"Portrety najemnikÛw",
	L"Akta najemnika",
	L"Pokaø galeriÍ by≥ych cz≥onkÛw A.I.M.",

	// text to display how the entries will be sorted

	L"Rosnπco",
	L"Malejπco",
};


//Aim Policies.c
//The page in which the AIM policies and regulations are displayed

const wchar_t* AimPolicyText[] =
{
	// The text on the buttons at the bottom of the page

	L"Poprzednia str.",
	L"Strona g≥Ûwna",
	L"Przepisy",
	L"NastÍpna str.",
	L"RezygnujÍ",
	L"AkceptujÍ",
};



//Aim Member.c
//The page in which the players hires AIM mercenaries

// Instructions to the user to either start video conferencing with the merc, or to go the mug shot index

const wchar_t* AimMemberText[] =
{
	L"Lewy klawisz myszy",
	L"kontakt z najemnikiem",
	L"Prawy klawisz myszy",
	L"lista portretÛw",
};

//Aim Member.c
//The page in which the players hires AIM mercenaries

const wchar_t* CharacterInfo[] =
{
	// The various attributes of the merc

	L"Zdrowie",
	L"ZwinnoúÊ",
	L"SprawnoúÊ",
	L"Si≥a",
	L"Um. dowodzenia",
	L"Inteligencja",
	L"Poziom doúw.",
	L"Um. strzeleckie",
	L"Zn. mechaniki",
	L"Zn. mat. wyb.",
	L"Wiedza med.",				//10

	// the contract expenses' area

	L"Zap≥ata",
	L"Czas",
	L"1 dzieÒ",
	L"1 tydzieÒ",
	L"2 tygodnie",

	// text for the buttons that either go to the previous merc,
	// start talking to the merc, or go to the next merc

	L"Poprzedni",
	L"Kontakt",
	L"NastÍpny",

	L"Dodatkowe informacje",				// Title for the additional info for the merc's bio
	L"Aktywni cz≥onkowie",		//20		// Title of the page
	L"Opcjonalne wyposaøenie:",				// Displays the optional gear cost
	L"Wymagany jest zastaw na øycie",			// If the merc required a medical deposit, this is displayed
};


//Aim Member.c
//The page in which the player's hires AIM mercenaries

//The following text is used with the video conference popup

const wchar_t* VideoConfercingText[] =
{
	L"WartoúÊ kontraktu:",				//Title beside the cost of hiring the merc

	//Text on the buttons to select the length of time the merc can be hired

	L"Jeden dzieÒ",
	L"Jeden tydzieÒ",
	L"Dwa tygodnie",

	//Text on the buttons to determine if you want the merc to come with the equipment

	L"Bez sprzÍtu",
	L"Weü sprzÍt",

	// Text on the Buttons

	L"TRANSFER",			// to actually hire the merc
	L"ANULUJ",				// go back to the previous menu
	L"WYNAJMIJ",				// go to menu in which you can hire the merc
	L"ROZ£•CZ",				// stops talking with the merc
	L"OK",
	L"NAGRAJ SI ",			// if the merc is not there, you can leave a message

	//Text on the top of the video conference popup

	L"Wideo konferencja z - ",
	L"£πczÍ. . .",

	L"z zastawem"			// Displays if you are hiring the merc with the medical deposit
};



//Aim Member.c
//The page in which the player hires AIM mercenaries

// The text that pops up when you select the TRANSFER FUNDS button

const wchar_t* AimPopUpText[] =
{
	L"TRANSFER ZAKO—CZONY POMYåLNIE",	// You hired the merc
	L"PRZEPROWADZENIE TRANSFERU NIE MOØLIWE",		// Player doesn't have enough money, message 1
	L"BRAK åRODK”W",				// Player doesn't have enough money, message 2

	// if the merc is not available, one of the following is displayed over the merc's face

	L"WynajÍto",
	L"ProszÍ zostaw wiadomoúÊ",
	L"Nie øyje",

	//If you try to hire more mercs than game can support

	L"Masz juø pe≥ny zespÛ≥ 18 najemnikÛw.",

	L"Nagrana wiadomoúÊ",
	L"WiadomoúÊ zapisana",
};


//AIM Link.c

const wchar_t* AimLinkText[] =
{
	L"A.I.M. Linki",	//The title of the AIM links page
};



//Aim History

// This page displays the history of AIM

const wchar_t* AimHistoryText[] =
{
	L"A.I.M. Historia",					//Title

	// Text on the buttons at the bottom of the page

	L"Poprzednia str.",
	L"Strona g≥Ûwna",
	L"Byli cz≥onkowie",
	L"NastÍpna str."
};


//Aim Mug Shot Index

//The page in which all the AIM members' portraits are displayed in the order selected by the AIM sort page.

const wchar_t* AimFiText[] =
{
	// displays the way in which the mercs were sorted

	L"ceny",
	L"doúwiadczenia",
	L"um. strzeleckich",
	L"um. medycznych",
	L"zn. materia≥Ûw wyb.",
	L"zn. mechaniki",

	// The title of the page, the above text gets added at the end of this text

	L"Cz≥onkowie A.I.M. posortowani rosnπco wg %ls",
	L"Cz≥onkowie A.I.M. posortowani malejπco wg %ls",

	// Instructions to the players on what to do

	L"Lewy klawisz",
	L"WybÛr najemnika",			//10
	L"Prawy klawisz",
	L"Opcje sortowania",

	// Gets displayed on top of the merc's portrait if they are...

	L"Wyjecha≥(a)",
	L"Nie øyje",						//14
	L"WynajÍto",
};



//AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer with AIM

const wchar_t* AimAlumniText[] =
{

	L"STRONA 1",
	L"STRONA 2",
	L"STRONA 3",

	L"Byli cz≥onkowie A.I.M.",	// Title of the page


	L"OK"			// Stops displaying information on selected merc
};






//AIM Home Page

const wchar_t* AimScreenText[] =
{
	// AIM disclaimers

	L"Znaki A.I.M. i logo A.I.M. sπ prawnie chronione w wiÍkszoúci krajÛw.",
	L"WiÍc nawet nie myúl o prÛbie ich podrobienia.",
	L"Copyright 1998-1999 A.I.M., Ltd. All rights reserved.",

	//Text for an advertisement that gets displayed on the AIM page

	L"United Floral Service",
	L"\"Zrzucamy gdziekolwiek\"",				//10
	L"ZrÛb to jak naleøy...",
	L"...za pierwszym razem",
	L"BroÒ i akcesoria, jeúli czegoú nie mamy, to tego nie potrzebujesz.",
};


//Aim Home Page

const wchar_t* AimBottomMenuText[] =
{
	//Text for the links at the bottom of all AIM pages
	L"Strona g≥Ûwna",
	L"Cz≥onkowie",
	L"Byli cz≥onkowie",
	L"Przepisy",
	L"Historia",
	L"Linki",
};



//ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.

const wchar_t* SKI_Text[ ] =
{
	L"TOWARY NA STANIE",		//Header for the merchandise available
	L"STRONA",				//The current store inventory page being displayed
	L"KOSZT OG”£EM",				//The total cost of the the items in the Dealer inventory area
	L"WARTOå∆ OG”£EM",			//The total value of items player wishes to sell
	L"WYCENA",				//Button text for dealer to evaluate items the player wants to sell
	L"TRANSAKCJA",			//Button text which completes the deal. Makes the transaction.
	L"OK",				//Text for the button which will leave the shopkeeper interface.
	L"KOSZT NAPRAWY",			//The amount the dealer will charge to repair the merc's goods
	L"1 GODZINA",			// SINGULAR! The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"%d GODZIN(Y)",		// PLURAL!   The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"NAPRAWIONO",		// Text appearing over an item that has just been repaired by a NPC repairman dealer
	L"Brak miejsca by zaoferowaÊ wiÍcej rzeczy.",	//Message box that tells the user there is no more room to put there stuff
	L"%d MINUT(Y)",		// The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"UpuúÊ przedmiot na ziemiÍ.",
};


const wchar_t* SkiMessageBoxText[] =
{
	L"Czy chcesz do≥oøyÊ %ls ze swojego konta, aby pokryÊ rÛønicÍ?",
	L"Brak úrodkÛw. Brakuje ci %ls",
	L"Czy chcesz przeznaczyÊ %ls ze swojego konta, aby pokryÊ koszty?",
	L"Poproú o rozpoczÍcie transakscji",
	L"Poproú o naprawÍ wybranych przedmiotÛw",
	L"ZakoÒcz rozmowÍ",
	L"Saldo dostÍpne",
};


//OptionScreen.c

const wchar_t* zOptionsText[] =
{
	//button Text
	L"Zapisz grÍ",
	L"Odczytaj grÍ",
	L"Wyjúcie",
	L"OK",

	//Text above the slider bars
	L"Efekty",
	L"Dialogi",
	L"Muzyka",

	//Confirmation pop when the user selects..
	L"ZakoÒczyÊ grÍ i wrÛciÊ do g≥Ûwnego menu?",

	L"Musisz w≥πczyÊ opcjÍ dialogÛw lub napisÛw.",
};


//SaveLoadScreen
const wchar_t* zSaveLoadText[] =
{
	L"Zapisz grÍ",
	L"Odczytaj grÍ",
	L"Anuluj",
	L"Zapisz wybranπ",
	L"Odczytaj wybranπ",

	L"Gra zosta≥a pomyúlnie zapisana",
	L"B£•D podczas zapisu gry!",
	L"Gra zosta≥a pomyúlnie odczytana",
	L"B£•D podczas odczytu gry!",

	L"Wersja gry w zapisanym pliku rÛøni siÍ od bieøπcej. Prawdopodobnie moøna bezpiecznie kontynuowaÊ. KontynuowaÊ?",
	L"Zapisane pliki gier mogπ byÊ uszkodzone. Czy chcesz je usunπÊ?",

	//Translators, the next two strings are for the same thing.  The first one is for beta version releases and the second one
	//is used for the final version.  Please don't modify the "#ifdef JA2BETAVERSION" or the "#else" or the "#endif" as they are
	//used by the compiler and will cause program errors if modified/removed.  It's okay to translate the strings though.
#ifdef JA2BETAVERSION
	L"Nieprawid≥owa wersja zapisu gry.  W razie problemÛw prosimy o raport.  KontynuowaÊ?",
#else
	L"PrÛba odczytu starszej wersji zapisu gry.  ZaktualizowaÊ ten zapis i odczytaÊ grÍ?",
#endif

	//Translators, the next two strings are for the same thing.  The first one is for beta version releases and the second one
	//is used for the final version.  Please don't modify the "#ifdef JA2BETAVERSION" or the "#else" or the "#endif" as they are
	//used by the compiler and will cause program errors if modified/removed.  It's okay to translate the strings though.
#ifdef JA2BETAVERSION
	L"Nieprawid≥owa wersja zapisu gry.  W razie problemÛw prosimy o raport.  KontynuowaÊ?",
#else
	L"PrÛba odczytu starszej wersji zapisu gry.  ZaktualizowaÊ ten zapis i odczytaÊ grÍ?",
#endif

	L"Czy na pewno chcesz nadpisaÊ grÍ na pozycji %d?",
	L"Chcesz odczytaÊ grÍ z pozycji",


	//The first %d is a number that contains the amount of free space on the users hard drive,
	//the second is the recommended amount of free space.
	L"Brak miejsca na dysku twardym.  Na dysku wolne jest %d MB, a wymagane jest przynajmniej %d MB.",

	L"ZapisujÍ...",			//When saving a game, a message box with this string appears on the screen

	L"Standardowe uzbrojenie",
	L"Ca≥e mnÛstwo broni",
	L"Realistyczna gra",
	L"Elementy S-F",

	L"StopieÒ trudnoúci",
};



//MapScreen
const wchar_t* zMarksMapScreenText[] =
{
	L"Poziom mapy",
	L"Nie masz jeszcze øo≥nierzy samoobrony.  Musisz najpierw wytrenowaÊ mieszkaÒcÛw miast.",
	L"Dzienny przychÛd",
	L"Najmemnik ma polisÍ ubezpieczeniowπ",
    L"%ls nie potrzebuje snu.",
	L"%ls jest w drodze i nie moøe spaÊ",
	L"%ls jest zbyt zmÍczony(na), sprÛbuj trochÍ pÛüniej.",
	L"%ls prowadzi.",
	L"Oddzia≥ nie moøe siÍ poruszaÊ jeøeli jeden z najemnikÛw úpi.",

	// stuff for contracts
	L"Mimo, øe moøesz op≥aciÊ kontrakt, to jednak nie masz gotÛwki by op≥aciÊ sk≥adkÍ ubezpieczeniowπ za najemnika.",
	L"%ls - sk≥adka ubezpieczeniowa najemnika bÍdzie kosztowaÊ %ls za %d dzieÒ(dni). Czy chcesz jπ op≥aciÊ?",
	L"Inwentarz sektora",
	L"Najemnik posiada zastaw na øycie.",

	// other items
	L"Lekarze", // people acting a field medics and bandaging wounded mercs // **************************************NEW******** as of July 09, 1998
	L"Pacjenci", // people who are being bandaged by a medic // ****************************************************NEW******** as of July 10, 1998
	L"Gotowe", // Continue on with the game after autobandage is complete
	L"Przerwij", // Stop autobandaging of patients by medics now
	L"Przykro nam, ale ta opcja jest wy≥πczona w wersji demo.", // informs player this option/button has been disabled in the demo
	L"%ls nie ma zestawu narzÍdzi.",
	L"%ls nie ma apteczki.",
	L"Brak chÍtnych ludzi do szkolenia, w tej chwili.",
	L"%ls posiada juø maksymalnπ liczbÍ oddzia≥Ûw samoobrony.",
	L"Najemnik ma kontrakt na okreúlony czas.",
  L"Kontrakt najemnika nie jest ubezpieczony",
};


const wchar_t* pLandMarkInSectorString[] =
{
	L"Oddzia≥ %d zauwaøy≥ kogoú w sektorze %ls",
};

// confirm the player wants to pay X dollars to build a militia force in town
const wchar_t* pMilitiaConfirmStrings[] =
{
	L"Szkolenie oddzia≥u samoobrony bÍdzie kosztowa≥o $", // telling player how much it will cost
	L"Zatwierdzasz wydatek?", // asking player if they wish to pay the amount requested
	L"Nie staÊ ciÍ na to.", // telling the player they can't afford to train this town
	L"KontynuowaÊ szkolenie samoobrony w - %ls (%ls %d)?", // continue training this town?
	L"Koszt $", // the cost in dollars to train militia
	L"( T/N )",   // abbreviated yes/no
	L"",	// unused
	L"Szkolenie samoobrony w %d sektorach bÍdzie kosztowa≥o $ %d. %ls", // cost to train sveral sectors at once
	L"Nie masz %d$, aby wyszkoliÊ samoobronÍ w tym mieúcie.",
	L"%ls musi mieÊ %d% lojalnoúci, aby moøna by≥o kontynuowaÊ szkolenie samoobrony.",
	L"Nie moøesz juø d≥uøej szkoliÊ samoobrony w mieúcie %ls.",
};

#ifdef JA2DEMOADS
const wchar_t* gpDemoString[] =
{
	//0-9
	L"NAJLEPSZA GRA STRATEGICZNA.",
	L"NAJLEPSZA GRA ROLE-PLAYING.",
	L"Nieliniowy scenariusz gry",
	L"Absolutnie nowy pomys≥",
	L"Wspania≥e strzelaniny",
	L"Dynamiczne oúwietlenie",
	L"Rozmowy z postaciami",
	L"Nocne akcje",
	L"Kreowanie w≥asnego najemnika",
	L"Ambitne i wciπgajπce misje",
	L"Sterowanie pojazdami",
	//10-19
	L"Ponad 150 rÛønych postaci",
	L"Setki ciekawych przedmiotÛw",
	L"Ponad 250 rÛønych plenerÛw",
	L"Efektowne eksplozje",
	L"Nowe taktyczne manewry",
	L"Tony animacji",
	L"Ponad 9000 kwestii dialogowych",
	L"Niesamowity podk≥ad muzyczny Kevina Manthei",
	L"Globalna strategia",
	L"Naloty",
	//20-29
	L"Szkolenie oddzia≥Ûw samoobrony",
	L"MoøliwoúÊ dostosowania scenariusza gry",
	L"Realistyczne tereny",
	L"Efektowna grafika high-color",
	L"Kupno, sprzedaø i handel",
	L"BezwzglÍdny przeciwnik AI",
	L"åwietna walka taktyczna",
	L"Opcjonalny element S-F",
	L"24 godzinny zegar",
	L"Minimum 60 godzin gry",
	//30-34
	L"RÛøne poziomy trudnoúci",
	L"Realistycznie opracowana balistyka",
	L"Wierne odwzorowanie zasad fizyki",
	L"W pe≥ni interaktywny interfejs",
	L"I wiele, wiele wiÍcej...",
	//35 on are BOLD
	L"",
	L"",
	L"Jedna z najbardziej pomys≥owych gier 1998 roku",
	L"PC Gamer",
	L"ZamÛw juø dzisiaj na stronie WWW.TOPWARE.PL !",
};

const wchar_t* gpDemoIntroString[] =
{
	L"Za chwilÍ sprÛbujesz najlepszej strategii, role-playing i najwspanialszej walki taktycznej:",
	//Point 1 (uses one string)
	L"Kontrola nad grupπ najemnikÛw o bogatej osobowoúci   (øadnych nudnych i zbÍdnych postaci).",
	//Point 2 (uses one string)
	L"Sprawdü wspania≥e manewry taktyczne, poczπwszy od biegu do czo≥gania siÍ, wspinania siÍ, skakania, walki wrÍcz i wielu innych.",
	//Point 3 (uses one string)
	L"Igraj z ogniem! Fajne zabawki, úmiercionoúna broÒ, efektowne eksplozje.",
	//Additional comment
	L"(a to dopiero poczπtek...)",
	//Introduction/instructions
	L"Witamy w Demoville... (naciúnij dowolny klawisz)",
};
#endif

//Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom of the single merc panel
const wchar_t* gzMoneyWithdrawMessageText[] =
{
	L"Jednorazowo moøesz wyp≥aciÊ do 20,000$.",
	L"Czy na pewno chcesz wp≥aciÊ %ls na swoje konto?",
};

const wchar_t* gzCopyrightText[] =
{
	L"Copyright (C) 1999 Sir-tech Canada Ltd.  All rights reserved.",
};

//option Text
const wchar_t* zOptionsToggleText[] =
{
	L"Dialogi",
	L"Wycisz potwierdzenia",
	L"Napisy",
	L"Wstrzymuj napisy",
	L"Animowany dym",
	L"Drastyczne sceny",
	L"Nigdy nie ruszaj mojej myszki!",
	L"Stara metoda wyboru",
	L"Pokazuj trasÍ ruchu",
	L"Pokazuj chybione strza≥y",
	L"Potwierdzenia w trybie Real-Time",
	L"Informacja, øe najemnik úpi/budzi siÍ",
	L"Uøywaj systemu metrycznego",
	L"åwiat≥o wokÛ≥ najemnikÛw podczas ruchu",
	L"Przyciπgaj kursor do najemnikÛw",
	L"Przyciπgaj kursor do drzwi",
	L"Pulsujπce przedmioty",
	L"Pokazuj korony drzew",
	L"Pokazuj siatkÍ",
	L"Pokazuj kursor 3D",
};

//This is the help text associated with the above toggles.
const wchar_t* zOptionsScreenHelpText[] =
{
	//speech
	L"W≥πcz tÍ opcjÍ, jeúli chcesz s≥uchaÊ dialogÛw.",

	//Mute Confirmation
	L"W≥πcza lub wy≥πcza g≥osowe potwierzenia postaci.",

		//Subtitles
	L"W≥πcza lub wy≥πcza napisy podczas dialogÛw.",

	//Key to advance speech
	L"Jeúli napisy sπ w≥πczone, opcja ta pozwoli ci spokojnie je przeczytaÊ podczas dialogu.",

	//Toggle smoke animation
	L"Wy≥πcz tÍ opcjÍ, aby poprawiÊ p≥ynnoúÊ dzia≥ania gry.",

	//Blood n Gore
	L"Wy≥πcz tÍ opcjÍ, jeúli nie lubisz widoku krwi.",

	//Never move my mouse
	L"Wy≥πcz tÍ opcjÍ, aby kursor myszki automatycznie ustawia≥ siÍ nad pojawiajπcymi siÍ okienkami dialogowymi.",

	//Old selection method
	L"W≥πcz tÍ opcjÍ, aby wybÛr postaci dzia≥a≥ tak jak w poprzedniej wersji gry.",

	//Show movement path
	L"W≥πcz tÍ opcjÍ jeúli chcesz widzieÊ trasÍ ruchu w trybie Real-Time.",

	//show misses
	L"W≥πcz tÍ opcjÍ, aby zobaczyÊ w co trafiajπ twoje kule gdy pud≥ujesz.",

	//Real Time Confirmation
	L"Gdy opcja ta jest w≥πczona, kaødy ruch najemnika w trybie Real-Time bÍdzie wymaga≥ dodatkowego, potwierdzajπcego klikniÍcia.",

	//Sleep/Wake notification
  	L"Gdy opcja ta jest w≥πczona, wyúwietlana bÍdzie informacja, øe najemnik po≥oøy≥ siÍ spaÊ lub wsta≥ i wrÛci≥ do pracy.",

	//Use the metric system
	L"Gdy opcja ta jest w≥πczona, gra uøywa systemu metrycznego.",

	//Merc Lighted movement
	L"Gdy opcja ta jest w≥πczona, teren wokÛ≥ najemnika bÍdzie oúwietlony podczas ruchu. Wy≥πcz tÍ opcjÍ, jeúli obniøa p≥ynnoúÊ gry.",

	//Smart cursor
	L"Gdy opcja ta jest w≥πczona, kursor automatycznie ustawia siÍ na najemnikach gdy znajdzie siÍ w ich pobliøu.",

	//snap cursor to the door
	L"Gdy opcja ta jest w≥πczona, kursor automatycznie ustawi siÍ na drzwiach gdy znajdzie siÍ w ich pobliøu.",

	//glow items
	L"Gdy opcja ta jest w≥πczona, przedmioty pulsujπ. ( |I )",

	//toggle tree tops
	L"Gdy opcja ta jest w≥πczona, wyúwietlane sπ korony drzew. ( |T )",

	//toggle wireframe
	L"Gdy opcja ta jest w≥πczona, wyúwietlane sπ zarysy niewidocznych úcian. ( |W )",

	L"Gdy opcja ta jest w≥πczona, kursor ruchu wyúwietlany jest w 3D. ( |Home )",

};


const wchar_t* gzGIOScreenText[] =
{
	L"POCZ•TKOWE USTAWIENIA GRY",
	L"Styl gry",
	L"Realistyczny",
	L"S-F",
	L"Opcje broni",
	L"MnÛstwo broni",
	L"Standardowe uzbrojenie",
	L"StopieÒ trudnoúci",
	L"Nowicjusz",
	L"Doúwiadczony",
	L"Ekspert",
	L"Ok",
	L"Anuluj",
	L"Dodatkowe opcje",
	L"Nielimitowany czas",
	L"Tury limitowane czasowo",
	L"Nie dzia≥a w wersji demo",
};

const wchar_t* pDeliveryLocationStrings[] =
{
	L"Austin",			//Austin, Texas, USA
	L"Bagdad",			//Baghdad, Iraq (Suddam Hussein's home)
	L"Drassen",			//The main place in JA2 that you can receive items.  The other towns are dummy names...
	L"Hong Kong",		//Hong Kong, Hong Kong
	L"Bejrut",			//Beirut, Lebanon	(Middle East)
	L"Londyn",			//London, England
	L"Los Angeles",	//Los Angeles, California, USA (SW corner of USA)
	L"Meduna",			//Meduna -- the other airport in JA2 that you can receive items.
	L"Metavira",		//The island of Metavira was the fictional location used by JA1
	L"Miami",				//Miami, Florida, USA (SE corner of USA)
	L"Moskwa",			//Moscow, USSR
	L"Nowy Jork",		//New York, New York, USA
	L"Ottawa",			//Ottawa, Ontario, Canada -- where JA2 was made!
	L"Paryø",				//Paris, France
	L"Trypolis",			//Tripoli, Libya (eastern Mediterranean)
	L"Tokio",				//Tokyo, Japan
	L"Vancouver",		//Vancouver, British Columbia, Canada (west coast near US border)
};

const wchar_t* pSkillAtZeroWarning[] =
{ //This string is used in the IMP character generation.  It is possible to select 0 ability
	//in a skill meaning you can't use it.  This text is confirmation to the player.
	L"Na pewno? WartoúÊ zero oznacza brak jakichkolwiek umiejÍtnoúci w tej dziedzinie.",
};

const wchar_t* pIMPBeginScreenStrings[] =
{
	L"( Maks. 8 znakÛw )",
};

const wchar_t* pIMPFinishButtonText[ 1 ]=
{
	L"AnalizujÍ",
};

const wchar_t* pIMPFinishStrings[ ]=
{
	L"DziÍkujemy, %ls", //%ls is the name of the merc
};

// the strings for imp voices screen
const wchar_t* pIMPVoicesStrings[] =
{
	L"G≥os",
};

const wchar_t* pDepartedMercPortraitStrings[ ]=
{
	L"åmierÊ w akcji",
	L"Zwolnienie",
	L"Inny",
};

// title for program
const wchar_t* pPersTitleText[] =
{
	L"Personel",
};

// paused game strings
const wchar_t* pPausedGameText[] =
{
	L"Gra wstrzymana",
	L"WznÛw grÍ (|P|a|u|s|e)",
	L"Wstrzymaj grÍ (|P|a|u|s|e)",
};


const wchar_t* pMessageStrings[] =
{
	L"ZakoÒczyÊ grÍ?",
	L"OK",
	L"TAK",
	L"NIE",
	L"ANULUJ",
	L"NAJMIJ",
	L"LIE",
	L"Brak opisu", //Save slots that don't have a description.
	L"Gra zapisana.",
	L"Gra zapisana.",
	L"QuickSave", //The name of the quicksave file (filename, text reference)
	L"SaveGame",	//The name of the normal savegame file, such as SaveGame01, SaveGame02, etc.
	L"sav",				//The 3 character dos extension (represents sav)
	L"../SavedGames", //The name of the directory where games are saved.
	L"DzieÒ",
	L"Najemn.",
	L"Wolna pozycja", //An empty save game slot
	L"Demo",				//Demo of JA2
	L"Debug",				//State of development of a project (JA2) that is a debug build
	L"",			//Release build for JA2
	L"strz/min",					//Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
	L"min",					//Abbreviation for minute.
	L"m",						//One character abbreviation for meter (metric distance measurement unit).
	L"kul",				//Abbreviation for rounds (# of bullets)
	L"kg",					//Abbreviation for kilogram (metric weight measurement unit)
	L"lb",					//Abbreviation for pounds (Imperial weight measurement unit)
	L"Strona g≥Ûwna",				//Home as in homepage on the internet.
	L"USD",					//Abbreviation to US dollars
	L"N/D",					//Lowercase acronym for not applicable.
	L"Tymczasem",		//Meanwhile
	L"%ls przyby≥(a) do sektora %ls%ls", //Name/Squad has arrived in sector A9.  Order must not change without notifying
																		//SirTech
	L"Wersja",
	L"Wolna pozycja na szybki zapis",
	L"Ta pozycja zarezerwowana jest na szybkie zapisy wykonywane podczas gry kombinacjπ klawiszy ALT+S.",
	L"Otw.",
	L"Zamkn.",
#ifdef JA2DEMO
	L"Tu juø wszystko za≥atwione. Moøe czas spotkaÊ siÍ z Gabby'm.",
	L"Nie trzeba by≥o go zabijaÊ.",
#endif
	L"Brak miejsca na dysku twardym.  Na dysku wolne jest %ls MB, a wymagane jest przynajmniej %ls MB.",
	L"NajÍto - %ls z A.I.M.",
	L"%ls z≥apa≥(a) %ls",		//'Merc name' has caught 'item' -- let SirTech know if name comes after item.
	L"%ls zaaplikowa≥(a) sobie lekarstwo", //'Merc name' has taken the drug
	L"%ls nie posiada wiedzy medycznej",//'Merc name' has no medical skill.

	//CDRom errors (such as ejecting CD while attempting to read the CD)
	L"IntegralnoúÊ gry zosta≥a naraøona na szwank.",
	L"B£•D: WyjÍto p≥ytÍ CD",

	//When firing heavier weapons in close quarters, you may not have enough room to do so.
	L"Nie ma miejsca, øeby stπd oddaÊ strza≥.",

	//Can't change stance due to objects in the way...
	L"Nie moøna zmieniÊ pozycji w tej chwili.",

	//Simple text indications that appear in the game, when the merc can do one of these things.
	L"UpuúÊ",
	L"RzuÊ",
	L"Podaj",

	L"%ls przekazano do - %ls.", //"Item" passed to "merc".  Please try to keep the item %ls before the merc %ls, otherwise,
											 //must notify SirTech.
	L"Brak wolnego miejsca, by przekazaÊ %ls do - %ls.", //pass "item" to "merc".  Same instructions as above.

	//A list of attachments appear after the items.  Ex:  Kevlar vest ( Ceramic Plate 'Attached )'
	L" do≥πczono )",

	//Cheat modes
	L"Pierwszy poziom lamerskich zagrywek osiπgniÍty",
	L"Drugi poziom lamerskich zagrywek osiπgniÍty",

	//Toggling various stealth modes
	L"Oddzia≥ ma w≥πczony tryb skradania siÍ.",
	L"Oddzia≥ ma wy≥πczony tryb skradania siÍ.",
	L"%ls ma w≥πczony tryb skradania siÍ.",
	L"%ls ma wy≥πczony tryb skradania siÍ.",

	//Wireframes are shown through buildings to reveal doors and windows that can't otherwise be seen in
	//an isometric engine.  You can toggle this mode freely in the game.
	L"Dodatkowe siatki w≥πczone.",
	L"Dodatkowe siatki wy≥πczone.",

	//These are used in the cheat modes for changing levels in the game.  Going from a basement level to
	//an upper level, etc.
	L"Nie moøna wyjúÊ do gÛry z tego poziomu...",
	L"Nie ma juø niøszych poziomÛw...",
	L"Wejúcie na %d poziom pod ziemiπ...",
	L"Wyjúcie z podziemii...",

	#ifdef JA2DEMO

	//For the demo, the sector exit interface, you'll be able to split your teams up, but the demo
	//has this feature disabled.  This string is fast help text that appears over "single" button.
	L"W pe≥nej wersji gry bÍdzie moøna dzieliÊ oddzia≥y,\nale w wersji demo jest to niemoøliwe.",

	//The overhead map is a map of the entire sector, which you can go into anytime, except in the demo.
	L"OgÛlna mapa sektora jest niedostÍpna w wersji demo.",

	#endif

	L" - ",		// used in the shop keeper inteface to mark the ownership of the item eg Red's gun
	L"Automatyczne centrowanie ekranu wy≥πczone.",
	L"Automatyczne centrowanie ekranu w≥πczone.",
	L"Kursor 3D wy≥πczony.",
	L"Kursor 3D w≥πczony.",
	L"Oddzia≥ %d aktywny.",
	L"%ls - Nie staÊ ciÍ by wyp≥aciÊ jej/jemu dziennπ pensjÍ w wysokoúci %ls.",	//first %ls is the mercs name, the seconds is a string containing the salary
	L"PomiÒ",
	L"%ls nie moøe odejúÊ sam(a).",
	L"Utworzono zapis gry o nazwie SaveGame99.sav. W razie potrzeby zmieÒ jego nazwÍ na SaveGame01..10. Wtedy bÍdzie moøna go odczytaÊ ze standardowego okna odczytu gry.",
	L"%ls wypi≥(a) trochÍ - %ls",
	L"Przesy≥ka dotar≥a do Drassen.",
 	L"%ls przybÍdzie do wyznaczonego punktu zrzutu (sektor %ls) w dniu %d, oko≥o godziny %ls.",		//first %ls is mercs name, next is the sector location and name where they will be arriving in, lastely is the day an the time of arrival
	L"Lista historii zaktualizowana.",
#ifdef JA2BETAVERSION
	L"Automatyczny zapis zosta≥ pomyúlnie wykonany.",
#endif
};


const wchar_t ItemPickupHelpPopup[][40] =
{
	L"OK",
	L"W gÛrÍ",
	L"Wybierz wszystko",
	L"W dÛ≥",
	L"Anuluj",
};

const wchar_t* pDoctorWarningString[] =
{
	L"%ls jest za daleko, aby poddaÊ siÍ leczeniu.",
	L"Lekarze nie mogli opatrzyÊ wszystkich rannych.",
};

const wchar_t* pMilitiaButtonsHelpText[] =
{
	L"Podnieú(Prawy klawisz myszy)/upuúÊ(Lewy klawisz myszy) Zielonych øo≥nierzy", // button help text informing player they can pick up or drop militia with this button
	L"Podnieú(Prawy klawisz myszy)/upuúÊ(Lewy klawisz myszy) Doúwiadczonych øo≥nierzy",
	L"Podnieú(Prawy klawisz myszy)/upuúÊ(Lewy klawisz myszy) WeteranÛw",
	L"Umieszcza jednakowπ iloúÊ øo≥nierzy samoobrony w kaødym sektorze.",
};

const wchar_t* pMapScreenJustStartedHelpText[] =
{
	L"Zajrzyj do A.I.M. i zatrudnij kilku najemnikÛw (*WskazÛwka* musisz otworzyÊ laptopa)", // to inform the player to hired some mercs to get things going
	L"Jeúli chcesz juø udaÊ siÍ do Arulco, kliknij przycisk kompresji czasu, w prawym dolnym rogu ekranu.", // to inform the player to hit time compression to get the game underway
};

const wchar_t* pAntiHackerString[] =
{
	L"B≥πd. Brakuje pliku, lub jest on uszkodzony. Gra zostanie przerwana.",
};


const wchar_t* gzLaptopHelpText[] =
{
	//Buttons:
	L"Przeglπdanie poczty",
	L"Przeglπdanie stron internetowych",
	L"Przeglπdanie plikÛw i za≥πcznikÛw pocztowych",
	L"Rejestr zdarzeÒ",
	L"Informacje o cz≥onkach oddzia≥u",
	L"Finanse i rejestr transakcji",
	L"Koniec pracy z laptopem",

	//Bottom task bar icons (if they exist):
	L"Masz nowπ pocztÍ",
	L"Masz nowe pliki",

	//Bookmarks:
	L"MiÍdzynarodowe Stowarzyszenie NajemnikÛw",
	L"Bobby Ray's - Internetowy sklep z broniπ",
	L"Instytut BadaÒ NajemnikÛw",
	L"Bardziej Ekonomiczne Centrum Rekrutacyjne",
	L"McGillicutty's - Zak≥ad pogrzebowy",
	L"United Floral Service",
	L"Brokerzy ubezpieczeniowi",
};


const wchar_t* gzHelpScreenText[] =
{
	L"Zamknij okno pomocy",
};

const wchar_t* gzNonPersistantPBIText[] =
{
	L"Trwa walka. NajemnikÛw moøna wycofaÊ tylko na ekranie taktycznym.",
	L"W|ejdü do sektora, aby kontynuowaÊ walkÍ.",
	L"|Automatycznie rozstrzyga walkÍ.",
	L"Nie moøna automatycznie rozstrzygnπÊ walki, gdy atakujesz.",
	L"Nie moøna automatycznie rozstrzygnπÊ walki, gdy wpadasz w pu≥apkÍ.",
	L"Nie moøna automatycznie rozstrzygnπÊ walki, gdy walczysz ze stworzeniami w kopalni.",
	L"Nie moøna automatycznie rozstrzygnπÊ walki, gdy w sektorze sπ wrodzy cywile.",
	L"Nie moøna automatycznie rozstrzygnπÊ walki, gdy w sektorze sπ dzikie koty.",
	L"TRWA WALKA",
	L"W tym momencie nie moøesz siÍ wycofaÊ.",
};

const wchar_t* gzMiscString[] =
{
	L"Øo≥nierze samoobrony kontynuujπ walkÍ bez pomocy twoich najemnikÛw...",
	L"W tym momencie tankowanie nie jest konieczne.",
	L"W baku jest %d%% paliwa.",
	L"Øo≥nierze Deidranny przejÍli ca≥kowitπ kontrolÍ nad - %ls.",
	L"Nie masz juø gdzie zatankowaÊ.",
};

const wchar_t* gzIntroScreen[] =
{
	L"Nie odnaleziono filmu wprowadzajπcego",
};

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
const wchar_t* pNewNoiseStr[] =
{
	L"%ls s≥yszy %ls DèWI K dochodzπcy z %ls.",
	L"%ls s≥yszy %ls ODG£OS RUCHU dochodzπcy z %ls.",
	L"%ls s≥yszy %ls ODG£OS SKRZYPNI CIA dochodzπcy z %ls.",
	L"%ls s≥yszy %ls PLUSK dochodzπcy z %ls.",
	L"%ls s≥yszy %ls ODG£OS UDERZENIA dochodzπcy z %ls.",
	L"%ls s≥yszy %ls WYBUCH dochodzπcy z %ls.",
	L"%ls s≥yszy %ls KRZYK dochodzπcy z %ls.",
	L"%ls s≥yszy %ls ODG£OS UDERZENIA dochodzπcy z %ls.",
	L"%ls s≥yszy %ls ODG£OS UDERZENIA dochodzπcy z %ls.",
	L"%ls s≥yszy %ls £OMOT dochodzπcy z %ls.",
	L"%ls s≥yszy %ls TRZASK dochodzπcy z %ls.",
};

const wchar_t* wMapScreenSortButtonHelpText[] =
{
	L"Sortuj wed≥ug kolumny ImiÍ (|F|1)",
	L"Sortuj wed≥ug kolumny Przydzia≥ (|F|2)",
	L"Sortuj wed≥ug kolumny Sen (|F|3)",
	L"Sortuj wed≥ug kolumny Lokalizacja (|F|4)",
	L"Sortuj wed≥ug kolumny Cel podrÛøy (|F|5)",
	L"Sortuj wed≥ug kolumny Wyjazd (|F|6)",
};



const wchar_t* BrokenLinkText[] =
{
	L"B≥πd 404",
	L"Nie odnaleziono strony.",
};


const wchar_t* gzBobbyRShipmentText[] =
{
	L"Ostatnie dostawy",
	L"ZamÛwienie nr ",
	L"IloúÊ przedmiotÛw",
	L"ZamÛwiono:",
};


const wchar_t* gzCreditNames[]=
{
	L"Chris Camfield",
	L"Shaun Lyng",
	L"Kris M‰rnes",
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


const wchar_t* gzCreditNameTitle[]=
{
	L"Game Internals Programmer", 			// Chris Camfield
	L"Co-designer/Writer",							// Shaun Lyng
	L"Strategic Systems & Editor Programmer",					//Kris Marnes
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

const wchar_t* gzCreditNameFunny[]=
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

const wchar_t* sRepairsDoneString[] =
{
	L"%ls skoÒczy≥(a) naprawiaÊ w≥asne wyposaøenie",
	L"%ls skoÒczy≥(a) naprawiaÊ broÒ i ochraniacze wszystkich cz≥onkÛw oddzia≥u",
	L"%ls skoÒczy≥(a) naprawiaÊ wyposaøenie wszystkich cz≥onkÛw oddzia≥u",
	L"%ls skoÒczy≥(a) naprawiaÊ ekwipunek wszystkich cz≥onkÛw oddzia≥u",
};


const wchar_t* zGioDifConfirmText[]=
{
	L"Wybrano opcjÍ Nowicjusz. Opcja ta jest przeznaczona dla niedoúwiadczonych graczy, lub dla tych, ktÛrzy nie majπ ochoty na d≥ugie i ciÍøkie walki. PamiÍtaj, øe opcja ta ma wp≥yw na przebieg ca≥ej gry. Czy na pewno chcesz graÊ w trybie Nowicjusz?",
	L"Wybrano opcjÍ Doúwiadczony. Opcja ta jest przenaczona dla graczy posiadajπcych juø pewne doúwiadczenie w grach tego typu. PamiÍtaj, øe opcja ta ma wp≥yw na przebieg ca≥ej gry. Czy na pewno chcesz graÊ w trybie Doúwiadczony?",
	L"Wybrano opcjÍ Ekspert. Jakby co, to ostrzegaliúmy ciÍ. Nie obwiniaj nas, jeúli wrÛcisz w plastikowym worku. PamiÍtaj, øe opcja ta ma wp≥yw na przebieg ca≥ej gry. Czy na pewno chcesz graÊ w trybie Ekspert?",
};


const wchar_t* gzLateLocalizedString[] =
{
	L"%ls - nie odnaleziono pliku...",

	//1-5
	L"Robot nie moøe opuúciÊ sektora bez operatora.",

	//This message comes up if you have pending bombs waiting to explode in tactical.
	L"Nie moøna teraz kompresowaÊ czasu.  Poczekaj na fajerwerki!",

	//'Name' refuses to move.
	L"%ls nie chce siÍ przesunπÊ.",

	//%ls a merc name
	L"%ls ma zbyt ma≥o energii, aby zmieniÊ pozycjÍ.",

	//A message that pops up when a vehicle runs out of gas.
	L"%ls nie ma paliwa i stoi w sektorze %c%d.",

	//6-10

	// the following two strings are combined with the pNewNoise[] strings above to report noises
	// heard above or below the merc
	L"G”RY",
	L"DO£U",

	//The following strings are used in autoresolve for autobandaging related feedback.
	L"Øaden z twoich najemnikÛw nie posiada wiedzy medycznej.",
	L"Brak úrodkÛw medycznych, aby za≥oøyÊ rannym opatrunki.",
	L"Zabrak≥o úrodkÛw medycznych, aby za≥oøyÊ wszystkim rannym opatrunki.",
	L"Øaden z twoich najemnikÛw nie potrzebuje pomocy medycznej.",
	L"Automatyczne zak≥adanie opatrunkÛw rannym najemnikom.",
	L"Wszystkim twoim najemnikom za≥oøono opatrunki.",

	//14
	L"Arulco",

  L"(dach)",

	L"Zdrowie: %d/%d",

	//In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
	//"vs." is the abbreviation of versus.
	L"%d vs. %d",

	L"%ls - brak wolnych miejsc!",  //(ex "The ice cream truck is full")

  L"%ls nie potrzebuje pierwszej pomocy lecz opieki lekarza lub d≥uøszego odpoczynku.",

	//20
	//Happens when you get shot in the legs, and you fall down.
	L"%ls dosta≥(a) w nogi i upad≥(a)!",
	//Name can't speak right now.
	L"%ls nie moøe teraz mÛwiÊ.",

	//22-24 plural versions
	L"%d zielonych øo≥nierzy samoobrony awansowa≥o na weteranÛw.",
	L"%d zielonych øo≥nierzy samoobrony awansowa≥o na regularnych øo≥nierzy.",
	L"%d regularnych øo≥nierzy samoobrony awansowa≥o na weteranÛw.",

	//25
	L"Prze≥πcznik",

	//26
	//Name has gone psycho -- when the game forces the player into burstmode (certain unstable characters)
	L"%ls dostaje úwira!",

	//27-28
	//Messages why a player can't time compress.
	L"Niebezpiecznie jest kompresowaÊ teraz czas, gdyø masz najemnikÛw w sektorze %ls.",
	L"Niebezpiecznie jest kompresowaÊ teraz czas, gdyø masz najemnikÛw w kopalni zaatakowanej przez robale.",

	//29-31 singular versions
	L"1 zielony øo≥nierz samoobrony awansowa≥ na weterana.",
	L"1 zielony øo≥nierz samoobrony awansowa≥ na regularnego øo≥nierza.",
	L"1 regularny øo≥nierz samoobrony awansowa≥ na weterana.",

	//32-34
	L"%ls nic nie mÛwi.",
	L"WyjúÊ na powierzchniÍ?",
	L"(Oddzia≥ %d)",

	//35
	//Ex: "Red has repaired Scope's MP5K".  Careful to maintain the proper order (Red before Scope, Scope before MP5K)
	L"%ls naprawi≥(a) najemnikowi - %ls, jego/jej - %ls",

	//36
	L"DZIKI KOT",

	//37-38 "Name trips and falls"
	L"%ls potyka siÍ i upada",
	L"Nie moøna stπd podnieúÊ tego przedmiotu.",

	//39
	L"Øaden z twoich najemnikÛw nie jest w stanie walczyÊ.  Øo≥nierze samoobrony sami bÍdπ walczyÊ z robalami.",

	//40-43
	//%ls is the name of merc.
	L"%ls nie ma úrodkÛw medycznych!",
	L"%ls nie posiada odpowiedniej wiedzy, aby kogokolwiek wyleczyÊ!",
	L"%ls nie ma narzÍdzi!",
	L"%ls nie posiada odpowiedniej wiedzy, aby cokolwiek naprawiÊ!",

	//44-45
	L"Czas naprawy",
	L"%ls nie widzi tej osoby.",

	//46-48
	L"%ls - przed≥uøka lufy jego/jej broni odpada!",
	L"W jednym sektorze, szkolenie samoobrony moøe prowadziÊ tylko %d instruktor(Ûw).",
  	L"Na pewno?",

	//49-50
	L"Kompresja czasu",
	L"Pojazd ma pe≥ny zbiornik paliwa.",

	//51-52 Fast help text in mapscreen.
	L"Kontynuuj kompresjÍ czasu (|S|p|a|c|j|a)",
	L"Zatrzymaj kompresjÍ czasu (|E|s|c)",

	//53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
	L"%ls odblokowa≥(a) - %ls",
	L"%ls odblokowa≥(a) najemnikowi - %ls, jego/jej - %ls",

	//55
	L"Nie moøna kompresowaÊ czasu, gdy otwarty jest inwentarz sektora.",

	L"Nie odnaleziono p≥yty nr 2 Jagged Alliance 2.",

	L"Przedmioty zosta≥y pomyúlnie po≥πczone.",

	//58
	//Displayed with the version information when cheats are enabled.
	L"Bieøπcy/Maks. postÍp: %d%%/%d%%",

	//59
	L"EskortowaÊ Johna i Mary?",

  L"Prze≥πcznik aktywowany.",
};




#endif //POLISH
