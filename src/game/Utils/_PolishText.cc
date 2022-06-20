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
//   Other examples are used multiple times, like the Esc key  or "|E|s|c" or Space -> (|S|p|a|c|j|a)
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



static const ST::string s_pl_WeaponType[WeaponType_SIZE] =
{
	"Inny",
	"Pistolet",
	"Pistolet maszynowy",
	"Karabin maszynowy",
	"Karabin",
	"Karabin snajperski",
	"Karabin bojowy",
	"Lekki karabin maszynowy",
	"Strzelba"
};

static const ST::string s_pl_TeamTurnString[TeamTurnString_SIZE] =
{
	"Tura gracza", // player's turn
	"Tura przeciwnika",
	"Tura stworzeń",
	"Tura samoobrony",
	"Tura cywili"
	// planning turn
};

static const ST::string s_pl_Message[Message_SIZE] =
{
	// In the following 8 strings, the %s is the merc's name, and the %d (if any) is a number.

	"%s dostał(a) w głowę i traci 1 punkt inteligencji!",
	"%s dostał(a) w ramię i traci 1 punkt zręczności!",
	"%s dostał(a) w klatkę piersiową i traci 1 punkt siły!",
	"%s dostał(a) w nogi i traci 1 punkt zwinności!",
	"%s dostał(a) w głowę i traci %d pkt. inteligencji!",
	"%s dostał(a) w ramię i traci %d pkt. zręczności!",
	"%s dostał(a) w klatkę piersiową i traci %d pkt. siły!",
	"%s dostał(a) w nogi i traci %d pkt. zwinności!",
	"Przerwanie!",

	"Dotarły twoje posiłki!",

	// In the following four lines, all %s's are merc names

	"%s przeładowuje.",
	"%s posiada za mało Punktów Akcji!",
	// the following 17 strings are used to create lists of gun advantages and disadvantages
	// (separated by commas)
	"niezawodna",
	"zawodna",
	"łatwa w naprawie",
	"trudna do naprawy",
	"solidna",
	"niesolidna",
	"szybkostrzelna",
	"wolno strzelająca",
	"daleki zasięg",
	"krótki zasięg",
	"mała waga",
	"duża waga",
	"niewielkie rozmiary",
	"szybki ciągły ogień",
	"brak ciągłego ognia",
	"duży magazynek",
	"mały magazynek",

	// In the following two lines, all %s's are merc names

	"%s: kamuflaż się starł.",
	"%s: kamuflaż się zmył.",

	// The first %s is a merc name and the second %s is an item name

	"Brak amunicji w dodatkowej broni!",
	"%s ukradł(a): %s.",

	// The %s is a merc name

	"%s ma broń bez funkcji ciągłego ognia.",

	"Już masz coś takiego dołączone.",
	"Połączyć przedmioty?",

	// Both %s's are item names

	"%s i %s nie pasują do siebie.",

	"Brak",
	"Wyjmij amunicję",
	"Dodatki",

	//You cannot use "item(s)" and your "other item" at the same time.
	//Ex:  You cannot use sun goggles and you gas mask at the same time.
	" %s i %s nie mogą być używane jednocześnie.",

	"Element, który masz na kursorze myszy może być dołączony do pewnych przedmiotów, poprzez umieszczenie go w jednym z czterech slotów.",
	"Element, który masz na kursorze myszy może być dołączony do pewnych przedmiotów, poprzez umieszczenie go w jednym z czterech slotów. (Jednak w tym przypadku, przedmioty do siebie nie pasują.)",
	"Ten sektor nie został oczyszczony z wrogów!",
	"Wciąż musisz dać %s %s",
	"%s dostał(a) w głowę!",
	"Przerwać walkę?",
	"Ta zmiana będzie trwała. Kontynuować?",
	"%s ma więcej energii!",
	"%s poślizgnął(nęła) się na kulkach!",
	"%s nie chwycił(a) - %s!",
	"%s naprawił(a) %s",
	"Przerwanie dla: ",
	"Poddać się?",
	"Ta osoba nie chce twojej pomocy.",
	"NIE SĄDZĘ!",
	"Aby podróżować helikopterem Skyridera, musisz najpierw zmienić przydział najemników na POJAZD/HELIKOPTER.",
	"%s miał(a) czas by przeładować tylko jedną broń",
	"Tura dzikich kotów",
};


// the types of time compression. For example: is the timer paused? at normal speed, 5 minutes per second, etc.
// min is an abbreviation for minutes

static const ST::string s_pl_sTimeStrings[sTimeStrings_SIZE] =
{
	"Pauza",
	"Normalna",
	"5 min.",
	"30 min.",
	"60 min.",
	"6 godz.", //NEW
};


// Assignment Strings: what assignment does the merc  have right now? For example, are they on a squad, training,
// administering medical aid (doctor) or training a town. All are abbreviated. 8 letters is the longest it can be.

static const ST::string s_pl_pAssignmentStrings[pAssignmentStrings_SIZE] =
{
	"Oddz. 1",
	"Oddz. 2",
	"Oddz. 3",
	"Oddz. 4",
	"Oddz. 5",
	"Oddz. 6",
	"Oddz. 7",
	"Oddz. 8",
	"Oddz. 9",
	"Oddz. 10",
	"Oddz. 11",
	"Oddz. 12",
	"Oddz. 13",
	"Oddz. 14",
	"Oddz. 15",
	"Oddz. 16",
	"Oddz. 17",
	"Oddz. 18",
	"Oddz. 19",
	"Oddz. 20",
	"Służba", // on active duty
	"Lekarz", // administering medical aid
	"Pacjent", // getting medical aid
	"Pojazd", // sitting around resting
	"Podróż", // in transit - abbreviated form
	"Naprawa", // repairing
	"Praktyka", // training themselves  // ***************NEW******************** as of June 24. 1998
	"Samoobr.", // training a town to revolt // *************NEW******************** as of June 24, 1998
	"Instruk.", // training a teammate
	"Uczeń", // being trained by someone else // *******************NEW************** as of June 24, 1998
	"Nie żyje", // dead
	"Obezwł.", // abbreviation for incapacitated
	"Jeniec", // Prisoner of war - captured
	"Szpital", // patient in a hospital
	"Pusty",	// Vehicle is empty
};


static const ST::string s_pl_pMilitiaString[pMilitiaString_SIZE] =
{
	"Samoobrona", // the title of the militia box
	"Bez przydziału", //the number of unassigned militia troops
	"Nie możesz przemieszczać oddziałów samoobrony gdy nieprzyjaciel jest w sektorze!",
};


static const ST::string s_pl_pMilitiaButtonString[pMilitiaButtonString_SIZE] =
{
	"Auto", // auto place the militia troops for the player
	"OK", // done placing militia troops
};

static const ST::string s_pl_pConditionStrings[pConditionStrings_SIZE] =
{
	"Doskonały", //the state of a soldier .. excellent health
	"Dobry", // good health
	"Dość dobry", // fair health
	"Ranny", // wounded health
	"Zmęczony",// "Wyczerpany", // tired
	"Krwawi", // bleeding to death
	"Nieprzyt.", // knocked out
	"Umierający", // near death
	"Nie żyje", // dead
};

static const ST::string s_pl_pEpcMenuStrings[pEpcMenuStrings_SIZE] =
{
	"Służba", // set merc on active duty
	"Pacjent", // set as a patient to receive medical aid
	"Pojazd", // tell merc to enter vehicle
	"Wypuść", // let the escorted character go off on their own
	"Anuluj", // close this menu
};


// look at pAssignmentString above for comments

static const ST::string s_pl_pLongAssignmentStrings[pLongAssignmentStrings_SIZE] =
{
	"Oddział 1",
	"Oddział 2",
	"Oddział 3",
	"Oddział 4",
	"Oddział 5",
	"Oddział 6",
	"Oddział 7",
	"Oddział 8",
	"Oddział 9",
	"Oddział 10",
	"Oddział 11",
	"Oddział 12",
	"Oddział 13",
	"Oddział 14",
	"Oddział 15",
	"Oddział 16",
	"Oddział 17",
	"Oddział 18",
	"Oddział 19",
	"Oddział 20",
	"Służba",
	"Lekarz",
	"Pacjent",
	"Pojazd",
	"W podróży",
	"Naprawa",
	"Praktyka",
	"Trenuj samoobronę",
	"Trenuj oddział",
	"Uczeń",
	"Nie żyje",
	"Obezwładniony",
	"Jeniec",
	"W szpitalu",
	"Pusty",	// Vehicle is empty
};


// the contract options

static const ST::string s_pl_pContractStrings[pContractStrings_SIZE] =
{
	"Opcje kontraktu:",
	"", // a blank line, required
	"Zaproponuj 1 dzień", // offer merc a one day contract extension
	"Zaproponuj 1 tydzień", // 1 week
	"Zaproponuj 2 tygodnie", // 2 week
	"Zwolnij", // end merc's contract
	"Anuluj", // stop showing this menu
};

static const ST::string s_pl_pPOWStrings[pPOWStrings_SIZE] =
{
	"Jeniec",  //an acronym for Prisoner of War
	"??",
};

static const ST::string s_pl_pInvPanelTitleStrings[pInvPanelTitleStrings_SIZE] =
{
	"Osłona", // the armor rating of the merc
	"Ekwip.", // the weight the merc is carrying
	"Kamuf.", // the merc's camouflage rating
};

static const ST::string s_pl_pShortAttributeStrings[pShortAttributeStrings_SIZE] =
{
	"Zwn", // the abbreviated version of : agility
	"Zrc", // dexterity
	"Sił", // strength
	"Dow", // leadership
	"Int", // wisdom
	"Doś", // experience level
	"Str", // marksmanship skill
	"Wyb", // explosive skill
	"Mec", // mechanical skill
	"Med", // medical skill
};


static const ST::string s_pl_pUpperLeftMapScreenStrings[pUpperLeftMapScreenStrings_SIZE] =
{
	"Przydział", // the mercs current assignment // *********************NEW****************** as of June 24, 1998
	"Zdrowie", // the health level of the current merc
	"Morale", // the morale of the current merc
	"Stan",	// the condition of the current vehicle
};

static const ST::string s_pl_pTrainingStrings[pTrainingStrings_SIZE] =
{
	"Praktyka", // tell merc to train self // ****************************NEW******************* as of June 24, 1998
	"Samoobrona", // tell merc to train town // *****************************NEW ****************** as of June 24, 1998
	"Instruktor", // tell merc to act as trainer
	"Uczeń", // tell merc to be train by other // **********************NEW******************* as of June 24, 1998
};

static const ST::string s_pl_pAssignMenuStrings[pAssignMenuStrings_SIZE] =
{
	"Służba", // merc is on active duty
	"Lekarz", // the merc is acting as a doctor
	"Pacjent", // the merc is receiving medical attention
	"Pojazd", // the merc is in a vehicle
	"Naprawa", // the merc is repairing items
	"Szkolenie", // the merc is training
	"Anuluj", // cancel this menu
};

static const ST::string s_pl_pRemoveMercStrings[pRemoveMercStrings_SIZE] =
{
	"Usuń najemnika", // remove dead merc from current team
	"Anuluj",
};

static const ST::string s_pl_pAttributeMenuStrings[pAttributeMenuStrings_SIZE] =
{
	"Siła",
	"Zręczność",
	"Zwinność",
	"Zdrowie",
	"Um. strzeleckie",
	"Wiedza med.",
	"Zn. mechaniki",
	"Um. dowodzenia",
	"Zn. mat. wyb.",
	"Anuluj",
};

static const ST::string s_pl_pTrainingMenuStrings[pTrainingMenuStrings_SIZE] =
{
	"Praktyka", // train yourself //****************************NEW************************** as of June 24, 1998
	"Samoobrona", // train the town // ****************************NEW ************************* as of June 24, 1998
	"Instruktor", // train your teammates // *******************NEW************************** as of June 24, 1998
	"Uczeń",  // be trained by an instructor //***************NEW************************** as of June 24, 1998
	"Anuluj", // cancel this menu
};


static const ST::string s_pl_pSquadMenuStrings[pSquadMenuStrings_SIZE] =
{
	"Oddział  1",
	"Oddział  2",
	"Oddział  3",
	"Oddział  4",
	"Oddział  5",
	"Oddział  6",
	"Oddział  7",
	"Oddział  8",
	"Oddział  9",
	"Oddział 10",
	"Oddział 11",
	"Oddział 12",
	"Oddział 13",
	"Oddział 14",
	"Oddział 15",
	"Oddział 16",
	"Oddział 17",
	"Oddział 18",
	"Oddział 19",
	"Oddział 20",
	"Anuluj",
};


static const ST::string s_pl_pPersonnelScreenStrings[pPersonnelScreenStrings_SIZE] =
{
	"Zastaw na życie:", // amount of medical deposit put down on the merc
	"Bieżący kontrakt:", // cost of current contract
	"Liczba zabójstw", // number of kills by merc
	"Liczba asyst", // number of assists on kills by merc
	"Dzienny koszt:", // daily cost of merc
	"Ogólny koszt:", // total cost of merc
	"Wartość kontraktu:", // cost of current contract
	"Usługi ogółem", // total service rendered by merc
	"Zaległa kwota", // amount left on MERC merc to be paid
	"Celność:", // percentage of shots that hit target
	"Ilość walk", // number of battles fought
	"Ranny(a)", // number of times merc has been wounded
	"Umiejętności:",
	"Brak umięjętności",
};


//These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
static const ST::string s_pl_gzMercSkillText[gzMercSkillText_SIZE] =
{
	"Brak umiejętności",
	"Otwieranie zamków",
	"Walka wręcz",
	"Elektronika",
	"Nocne operacje",
	"Rzucanie",
	"Szkolenie",
	"Ciężka broń",
	"Broń automatyczna",
	"Skradanie się",
	"Oburęczność",
	"Kradzieże",
	"Sztuki walki",
	"Broń biała",
	"Snajper",
	"Kamuflaż",
	"(Eksp.)",
};


// This is pop up help text for the options that are available to the merc

static const ST::string s_pl_pTacticalPopupButtonStrings[pTacticalPopupButtonStrings_SIZE] =
{
	"W|stań/Idź",
	"S|chyl się/Idź",
	"Wstań/Biegnij (|R)",
	"|Padnij/Czołgaj się",
	"Patrz (|L)",
	"Akcja",
	"Rozmawiaj",
	"Zbadaj (|C|t|r|l)",

	// Pop up door menu
	"Otwórz",
	"Poszukaj pułapek",
	"Użyj wytrychów",
	"Wyważ",
	"Usuń pułapki",
	"Zamknij na klucz",
	"Otwórz kluczem",
	"Użyj ładunku wybuchowego",
	"Użyj łomu",
	"Anuluj (|E|s|c)",
	"Zamknij"
};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.

static const ST::string s_pl_pDoorTrapStrings[pDoorTrapStrings_SIZE] =
{
	"nie posiada żadnych pułapek",
	"ma założony ładunek wybuchowy",
	"jest pod napięciem",
	"posiada syrenę alarmową",
	"posiada dyskretny alarm"
};

// On the map screen, there are four columns. This text is popup help text that identifies the individual columns.

static const ST::string s_pl_pMapScreenMouseRegionHelpText[pMapScreenMouseRegionHelpText_SIZE] =
{
	"Wybór postaci",
	"Przydział najemnika",
	"Nanieś trasę podróży",
	"Kontrakt najemnika",
	"Usuń najemnika",
	"Śpij", // *****************************NEW********************* as of June 29, 1998
};

// volumes of noises

static const ST::string s_pl_pNoiseVolStr[pNoiseVolStr_SIZE] =
{
	"CICHY",
	"WYRAŹNY",
	"GŁOŚNY",
	"BARDZO GŁOŚNY"
};

// types of noises

static const ST::string s_pl_pNoiseTypeStr[pNoiseTypeStr_SIZE] = // OBSOLETE
{
	"NIEOKREŚLONY DŹWIĘK",
	"ODGŁOS RUCHU",
	"ODGŁOS SKRZYPNIĘCIA",
	"PLUSK",
	"ODGŁOS UDERZENIA",
	"STRZAŁ",
	"WYBUCH",
	"KRZYK",
	"ODGŁOS UDERZENIA",
	"ODGŁOS UDERZENIA",
	"ŁOMOT",
	"TRZASK"
};

// Directions that are used to report noises

static const ST::string s_pl_pDirectionStr[pDirectionStr_SIZE] =
{
	"PŁN-WSCH",
	"WSCH",
	"PŁD-WSCH",
	"PŁD",
	"PŁD-ZACH",
	"ZACH",
	"PŁN-ZACH",
	"PŁN"
};

static const ST::string s_pl_gpStrategicString[gpStrategicString_SIZE] =
{
	"%s wykryto w sektorze %s, a inny oddział jest w drodze.",	//STR_DETECTED_SINGULAR
	"%s wykryto w sektorze %s, a inne oddziały są w drodze.",	//STR_DETECTED_PLURAL
	"Chcesz skoordynować jednoczesne przybycie?",			//STR_COORDINATE

	//Dialog strings for enemies.

	"Wróg daje ci szansę się poddać.",			//STR_ENEMY_SURRENDER_OFFER
	"Wróg schwytał resztę twoich nieprzytomnych najemników.",	//STR_ENEMY_CAPTURED

	//The text that goes on the autoresolve buttons

	"Odwrót", 		//The retreat button				//STR_AR_RETREAT_BUTTON
	"OK",		//The done button				//STR_AR_DONE_BUTTON

	//The headers are for the autoresolve type (MUST BE UPPERCASE)

	"OBRONA",								//STR_AR_DEFEND_HEADER
	"ATAK",								//STR_AR_ATTACK_HEADER
	"STARCIE",								//STR_AR_ENCOUNTER_HEADER
	"Sektor",		//The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

	//The battle ending conditions

	"ZWYCIĘSTWO!",								//STR_AR_OVER_VICTORY
	"PORAŻKA!",								//STR_AR_OVER_DEFEAT
	"KAPITULACJA!",							//STR_AR_OVER_SURRENDERED
	"NIEWOLA!",								//STR_AR_OVER_CAPTURED
	"ODWRÓT!",								//STR_AR_OVER_RETREATED

	//These are the labels for the different types of enemies we fight in autoresolve.

	"Samoobrona",							//STR_AR_MILITIA_NAME,
	"Elity",								//STR_AR_ELITE_NAME,
	"Żołnierze",								//STR_AR_TROOP_NAME,
	"Administrator",							//STR_AR_ADMINISTRATOR_NAME,
	"Stworzenie",								//STR_AR_CREATURE_NAME,

	//Label for the length of time the battle took

	"Czas trwania",							//STR_AR_TIME_ELAPSED,

	//Labels for status of merc if retreating.  (UPPERCASE)

	"WYCOFAŁ(A) SIĘ",								//STR_AR_MERC_RETREATED,
	"WYCOFUJE SIĘ",								//STR_AR_MERC_RETREATING,
	"WYCOFAJ SIĘ",								//STR_AR_MERC_RETREAT,

	//PRE BATTLE INTERFACE STRINGS
	//Goes on the three buttons in the prebattle interface.  The Auto resolve button represents
	//a system that automatically resolves the combat for the player without having to do anything.
	//These strings must be short (two lines -- 6-8 chars per line)

	"Rozst. autom.",							//STR_PB_AUTORESOLVE_BTN,
	"Idź do sektora",							//STR_PB_GOTOSECTOR_BTN,
	"Wycof. ludzi",							//STR_PB_RETREATMERCS_BTN,

	//The different headers(titles) for the prebattle interface.
	"STARCIE Z WROGIEM",							//STR_PB_ENEMYENCOUNTER_HEADER,
	"INWAZJA WROGA",							//STR_PB_ENEMYINVASION_HEADER, // 30
	"ZASADZKA WROGA",
	"WEJŚCIE DO WROGIEGO SEKTORA",
	"ATAK STWORÓW",
	"ATAK DZIKICH KOTÓW",							//STR_PB_BLOODCATAMBUSH_HEADER
	"WEJŚCIE DO LEGOWISKA DZIKICH KOTÓW",			//STR_PB_ENTERINGBLOODCATLAIR_HEADER

	//Various single words for direct translation.  The Civilians represent the civilian
	//militia occupying the sector being attacked.  Limited to 9-10 chars

	"Położenie",
	"Wrogowie",
	"Najemnicy",
	"Samoobrona",
	"Stwory",
	"Dzikie koty",
	"Sektor",
	"Brak",		//If there are no uninvolved mercs in this fight.
	"N/D",			//Acronym of Not Applicable N/A
	"d",			//One letter abbreviation of day
	"g",			//One letter abbreviation of hour

	//TACTICAL PLACEMENT USER INTERFACE STRINGS
	//The four buttons

	"Wyczyść",
	"Rozprosz",
	"Zgrupuj",
	"OK",

	//The help text for the four buttons.  Use \n to denote new line (just like enter).

	"Kasuje wszystkie pozy|cje najemników,\ni pozwala ponownie je wprowadzić.",
	"Po każdym naciśnięciu rozmie|szcza\nlosowo twoich najemników.",
	"|Grupuje najemników w wybranym miejscu.",
	"Kliknij ten klawisz gdy już rozmieścisz\nswoich najemników. (|E|n|t|e|r)",
	"Musisz rozmieścić wszystkich najemników\nzanim rozpoczniesz walkę.",

	//Various strings (translate word for word)

	"Sektor",
	"Wybierz początkowe pozycje",

	//Strings used for various popup message boxes.  Can be as long as desired.

	"To miejsce nie jest zbyt dobre. Jest niedostępne. Spróbuj gdzie indziej.",
	"Rozmieść swoich najemników na podświetlonej części mapy.",

	//These entries are for button popup help text for the prebattle interface.  All popup help
	//text supports the use of \n to denote new line.  Do not use spaces before or after the \n.
	"|Automatycznie prowadzi walkę za ciebie,\nnie ładując planszy.",
	"Atakując sektor wroga nie można automatycznie rozstrzygnąć walki.",
	"Wejście do s|ektora by nawiązać walkę z wrogiem.",
	"Wycofuje oddział do sąsiedniego sekto|ra.",				//singular version
	"Wycofuje wszystkie oddziały do sąsiedniego sekto|ra.", //multiple groups with same previous sector

	//various popup messages for battle conditions.

	//%s -- ex:  A9
	"Nieprzyjaciel zatakował oddziały samoobrony w sektorze %s.",
	//%s -- ex:  A9
	"Stworzenia zaatakowały oddziały samoobrony w sektorze %s.",
	//1st %d refers to the number of civilians eaten by monsters,  %s -- ex:  A9
	//Note:  the minimum number of civilians eaten will be two.
	"Stworzenia zatakowały i zabiły %d cywili w sektorze %s.",
	//%s -- ex:  A9
	"Nieprzyjaciel zatakował twoich najemników w sektorze %s.  Żaden z twoich najemników nie może walczyć!",
	//%s -- ex:  A9
	"Stworzenia zatakowały twoich najemników w sektorze %s.  Żaden z twoich najemników nie może walczyć!",

};

//This is the day represented in the game clock.  Must be very short, 4 characters max.
static const ST::string s_pl_gpGameClockString = "Dzień";

//When the merc finds a key, they can get a description of it which
//tells them where and when they found it.
static const ST::string s_pl_sKeyDescriptionStrings[sKeyDescriptionStrings_SIZE] =
{
	"Zn. w sektorze:",
	"Zn. w dniu:",
};

//The headers used to describe various weapon statistics.

static const ST::string s_pl_gWeaponStatsDesc[ gWeaponStatsDesc_SIZE] =
{
	"Waga (%s):", // change kg to another weight unit if your standard is not kilograms, and TELL SIR-TECH!
	"Stan:",
	"Ilość:", 		// Number of bullets left in a magazine
	"Zas.:",		// Range
	"Siła:",		// Damage
	"PA:",                 // abbreviation for Action Points
	"="
};

//The headers used for the merc's money.

static const ST::string s_pl_gMoneyStatsDesc[gMoneyStatsDesc_SIZE] =
{
	"Kwota",
	"Pozostało:", //this is the overall balance
	"Kwota",
	"Wydzielić:", // the amount he wants to separate from the overall balance to get two piles of money

	"Bieżące",
	"Saldo",
	"Kwota do",
	"podjęcia",
};

//The health of various creatures, enemies, characters in the game. The numbers following each are for comment
//only, but represent the precentage of points remaining.

static const ST::string s_pl_zHealthStr[zHealthStr_SIZE] =
{
	"UMIERAJĄCY",		//	>= 0
	"KRYTYCZNY", 		//	>= 15
	"KIEPSKI",		//	>= 30
	"RANNY",    	//	>= 45
	"ZDROWY",    	//	>= 60
	"SILNY",     	// 	>= 75
	"DOSKONAŁY",		// 	>= 90
};

static const ST::string s_pl_gzMoneyAmounts[gzMoneyAmounts_SIZE] =
{
	"$1000",
	"$100",
	"$10",
	"OK",
	"Wydziel",
	"Podejmij",
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
static const ST::string s_pl_gzProsLabel = "Zalety:";
static const ST::string s_pl_gzConsLabel = "Wady:";

//Conversation options a player has when encountering an NPC
static const ST::string s_pl_zTalkMenuStrings[zTalkMenuStrings_SIZE] =
{
	"Powtórz", 	//meaning "Repeat yourself"
	"Przyjaźnie",		//approach in a friendly
	"Bezpośrednio",		//approach directly - let's get down to business
	"Groźnie",		//approach threateningly - talk now, or I'll blow your face off
	"Daj",
	"Rekrutuj",
};

//Some NPCs buy, sell or repair items. These different options are available for those NPCs as well.
static const ST::string s_pl_zDealerStrings[zDealerStrings_SIZE] =
{
	"Kup/Sprzedaj",
	"Kup",
	"Sprzedaj",
	"Napraw",
};

static const ST::string s_pl_zDialogActions = "OK";


//These are vehicles in the game.

static const ST::string s_pl_pVehicleStrings[pVehicleStrings_SIZE] =
{
	"Eldorado",
	"Hummer", // a hummer jeep/truck -- military vehicle
	"Furgonetka z lodami",
	"Jeep",
	"Czołg",
	"Helikopter",
};

static const ST::string s_pl_pShortVehicleStrings[pVehicleStrings_SIZE] =
{
	"Eldor.",
	"Hummer",			// the HMVV
	"Furg.",
	"Jeep",
	"Czołg",
	"Heli.", 				// the helicopter
};

static const ST::string s_pl_zVehicleName[pVehicleStrings_SIZE] =
{
	"Eldorado",
	"Hummer",		//a military jeep. This is a brand name.
	"Furg.",			// Ice cream truck
	"Jeep",
	"Czołg",
	"Heli.", 		//an abbreviation for Helicopter
};


//These are messages Used in the Tactical Screen

static const ST::string s_pl_TacticalStr[TacticalStr_SIZE] =
{
	"Nalot",
	"Udzielić automatycznie pierwszej pomocy?",

	// CAMFIELD NUKE THIS and add quote #66.

	"%s zauważył(a) że dostawa jest niekompletna.",

	// The %s is a string from pDoorTrapStrings

	"Zamek %s.",
	"Brak zamka.",
	"Zamek nie ma pułapek.",
	// The %s is a merc name
	"%s nie posiada odpowiedniego klucza.",
	"Zamek nie ma pułapek.",
	"Zamknięte.",
	"DRZWI",
	"ZABEZP.",
	"ZAMKNIĘTE",
	"OTWARTE",
	"ROZWALONE",
	"Tu jest przełącznik. Włączyć go?",
	"Rozbroić pułapkę?",
	"Więcej...",

	// In the next 2 strings, %s is an item name

	"%s - położono na ziemi.",
	"%s - przekazano do - %s.",

	// In the next 2 strings, %s is a name

	"%s otrzymał(a) całą zapłatę.",
	"%s - należność wobec niej/niego wynosi jeszcze %d.",
	"Wybierz częstotliwość sygnału detonującego:",  	//in this case, frequency refers to a radio signal
	"Ile tur do eksplozji:",	//how much time, in turns, until the bomb blows
	"Ustaw częstotliwość zdalnego detonatora:", 	//in this case, frequency refers to a radio signal
	"Rozbroić pułapkę?",
	"Usunąć niebieską flagę?",
	"Umieścić tutaj niebieską flagę?",
	"Kończąca tura",

	// In the next string, %s is a name. Stance refers to way they are standing.

	"Na pewno chcesz zaatakować - %s?",
	"Pojazdy nie mogą zmieniać pozycji.",
	"Robot nie może zmieniać pozycji.",

	// In the next 3 strings, %s is a name

	"%s nie może zmienić pozycji w tym miejscu.",
	"%s nie może tu otrzymać pierwszej pomocy.",
	"%s nie potrzebuje pierwszej pomocy.",
	"Nie można ruszyć w to miejsce.",
	"Oddział jest już kompletny. Nie ma miejsca dla nowych rekrutów.",	//there's no room for a recruit on the player's team

	// In the next string, %s is a name

	"%s pracuje już dla ciebie.",

	// Here %s is a name and %d is a number

	"%s - należność wobec niej/niego wynosi %d$.",

	// In the next string, %s is a name

	"%s - Eskortować tą osobę?",

	// In the next string, the first %s is a name and the second %s is an amount of money (including $ sign)

	"%s - Zatrudnić tą osobę za %s dziennie?",

	// This line is used repeatedly to ask player if they wish to participate in a boxing match.

	"Chcesz walczyć?",

	// In the next string, the first %s is an item name and the
	// second %s is an amount of money (including $ sign)

	"%s - Kupić to za %s?",

	// In the next string, %s is a name

	"%s jest pod eskortą oddziału %d.",

	// These messages are displayed during play to alert the player to a particular situation

	"ZACIĘTA",					//weapon is jammed.
	"Robot potrzebuje amunicji kaliber %s.",		//Robot is out of ammo
	"Rzucić tam? To niemożliwe.",		//Merc can't throw to the destination he selected

	// These are different buttons that the player can turn on and off.

	"Skradanie się (|Z)",
	"Ekran |Mapy",
	"Koniec tury (|D)",
	"Rozmowa",
	"Wycisz",
	"Pozycja do góry (|P|g|U|p)",
	"Poziom kursora (|T|a|b)",
	"Wspinaj się / Zeskocz",
	"Pozycja w dół (|P|g|D|n)",
	"Badać (|C|t|r|l)",
	"Poprzedni najemnik",
	"Następny najemnik (|S|p|a|c|j|a)",
	"|Opcje",
	"Ciągły ogień (|B)",
	"Spójrz/Obróć się (|L)",
	"Zdrowie: %d/%d\nEnergia: %d/%d\nMorale: %s",
	"Co?",					//this means "what?"
	"Kont",					//an abbrieviation for "Continued"
	"%s ma włączone potwierdzenia głosowe.",
	"%s ma wyłączone potwierdzenia głosowe.",
	"Stan: %d/%d\nPaliwo: %d/%d",
	"Wysiądź z pojazdu" ,
	"Zmień oddział ( |S|h|i|f|t |S|p|a|c|j|a )",
	"Prowadź",
	"N/D",						//this is an acronym for "Not Applicable."
	"Użyj ( Walka wręcz )",
	"Użyj ( Broni palnej )",
	"Użyj ( Broni białej )",
	"Użyj ( Mat. wybuchowych )",
	"Użyj ( Apteczki )",
	"(Łap)",
	"(Przeładuj)",
	"(Daj)",
	"%s - pułapka została uruchomiona.",
	"%s przybył(a) na miejsce.",
	"%s stracił(a) wszystkie Punkty Akcji.",
	"%s jest nieosiągalny(na).",
	"%s ma już założone opatrunki.",
	"%s nie ma bandaży.",
	"Wróg w sektorze!",
	"Nie ma wroga w zasięgu wzroku.",
	"Zbyt mało Punktów Akcji.",
	"Nikt nie używa zdalnego sterowania.",
	"Ciągły ogień opróżnił magazynek!",
	"ŻOŁNIERZ",
	"STWÓR",
	"SAMOOBRONA",
	"CYWIL",
	"Wyjście z sektora",
	"OK",
	"Anuluj",
	"Wybrany najemnik",
	"Wszyscy najemnicy w oddziale",
	"Idź do sektora",
	"Otwórz mapę",
	"Nie można opuścić sektora z tej strony.",
	"%s jest zbyt daleko.",
	"Usuń korony drzew",
	"Pokaż korony drzew",
	"WRONA",				//Crow, as in the large black bird
	"SZYJA",
	"GŁOWA",
	"TUŁÓW",
	"NOGI",
	"Powiedzieć królowej to, co chce wiedzieć?",
	"Wzór odcisku palca pobrany",
	"Niewłaściwy wzór odcisku palca. Broń bezużyteczna.",
	"Cel osiągnięty",
	"Droga zablokowana",
	"Wpłata/Podjęcie pieniędzy",		//Help text over the $ button on the Single Merc Panel
	"Nikt nie potrzebuje pierwszej pomocy.",
	"Zac.",						// Short form of JAMMED, for small inv slots
	"Nie można się tam dostać.",					// used ( now ) for when we click on a cliff
	"Osoba nie chce się przesunąć.",
	// In the following message, '%s' would be replaced with a quantity of money (e.g. $200)
	"Zgadzasz się zapłacić %s?",
	"Zgadzasz się na darmowe leczenie?",
	"Zgadasz się na małżeństwo z Darylem?",
	"Kółko na klucze",
	"Nie możesz tego zrobić z eskortowaną osobą.",
	"Oszczędzić Krotta?",
	"Poza zasięgiem broni",
	"Górnik",
	"Pojazdem można podróżować tylko pomiędzy sektorami",
	"Teraz nie można automatycznie udzielić pierwszej pomocy",
	"Przejście zablokowane dla - %s",
	"Twoi najemnicy, schwytani przez żołnierzy Deidranny, są tutaj uwięzieni!",
	"Zamek został trafiony",
	"Zamek został zniszczony",
	"Ktoś inny majstruje przy tych drzwiach.",
	"Stan: %d/%d\nPaliwo: %d/%d",
	"%s nie widzi - %s.",  // Cannot see person trying to talk to
};

//Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different circumstances in the "exiting sector" interface.
static const ST::string s_pl_pExitingSectorHelpText[pExitingSectorHelpText_SIZE] =
{
	//Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box is checked.
	"Jeśli zaznaczysz tę opcję, to sąsiedni sektor zostanie natychmiast załadowany.",
	"Jeśli zaznaczysz tę opcję, to na czas podróży pojawi się automatycznie ekran mapy.",

	//If you attempt to leave a sector when you have multiple squads in a hostile sector.
	"Ten sektor jest okupowany przez wroga i nie możesz tu zostawić najemników.\nMusisz uporać się z tą sytuacją zanim załadujesz inny sektor.",

	//Because you only have one squad in the sector, and the "move all" option is checked, the "go to sector" option is locked to on.
	//The helptext explains why it is locked.
	"Gdy wyprowadzisz swoich pozostałych najemników z tego sektora,\nsąsiedni sektor zostanie automatycznie załadowany.",
	"Gdy wyprowadzisz swoich pozostałych najemników z tego sektora,\nto na czas podróży pojawi się automatycznie ekran mapy.",

	//If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being escorted.  The "single" button is disabled.
	"%s jest pod eskortą twoich najemników i nie może bez nich opuścić tego sektora.",

	//If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc will be prohibited from leaving alone.
	//There are several strings depending on the gender of the merc and how many EPCs are in the squad.
	//DO NOT USE THE NEWLINE HERE AS IT IS USED FOR BOTH HELPTEXT AND SCREEN MESSAGES!
	"%s nie może sam opuścić tego sektora, gdyż %s jest pod jego eskortą.", //male singular
	"%s nie może sama opuścić tego sektora, gdyż %s jest pod jej eskortą.", //female singular
	"%s nie może sam opuścić tego sektora, gdyż eskortuje inne osoby.", //male plural
	"%s nie może sama opuścić tego sektora, gdyż eskortuje inne osoby.", //female plural

	//If one or more of your mercs in the selected squad aren't in range of the traversal area, then the  "move all" option is disabled,
	//and this helptext explains why.
	"Wszyscy twoi najemnicy muszą być w pobliżu,\naby oddział mógł się przemieszczać.",

	//Standard helptext for single movement.  Explains what will happen (splitting the squad)
	"Jeśli zaznaczysz tę opcję, %s będzie podróżować w pojedynkę\ni automatycznie znajdzie się w osobnym oddziale.",

	//Standard helptext for all movement.  Explains what will happen (moving the squad)
	"Jeśli zaznaczysz tę opcję, aktualnie\nwybrany oddział opuści ten sektor.",

	//This strings is used BEFORE the "exiting sector" interface is created.  If you have an EPC selected and you attempt to tactically
	//traverse the EPC while the escorting mercs aren't near enough (or dead, dying, or unconscious), this message will appear and the
	//"exiting sector" interface will not appear.  This is just like the situation where
	//This string is special, as it is not used as helptext.  Do not use the special newline character (\n) for this string.
	"%s jest pod eskortą twoich najemników i nie może bez nich opuścić tego sektora. Aby opuścić sektor twoi najemnicy muszą być w pobliżu.",
};



static const ST::string s_pl_pRepairStrings[pRepairStrings_SIZE] =
{
	"Wyposażenie", 		// tell merc to repair items in inventory
	"Baza rakiet Z-P", // tell merc to repair SAM site - SAM is an acronym for Surface to Air Missile
	"Anuluj", 		// cancel this menu
	"Robot", 		// repair the robot
};


// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."

static const ST::string s_pl_sPreStatBuildString[sPreStatBuildString_SIZE] =
{
	"traci", 		// the merc has lost a statistic
	"zyskuje", 		// the merc has gained a statistic
	"pkt.",	// singular
	"pkt.",	// plural
	"pkt.",	// singular
	"pkt.",	// plural
};

static const ST::string s_pl_sStatGainStrings[sStatGainStrings_SIZE] =
{
	"zdrowia.",
	"zwinności.",
	"zręczności.",
	"inteligencji.",
	"umiejętności medycznych.",
	"umiejętności w dziedzinie materiałów wybuchowych.",
	"umiejętności w dziedzinie mechaniki.",
	"umiejętności strzeleckich.",
	"doświadczenia.",
	"siły.",
	"umiejętności dowodzenia.",
};


static const ST::string s_pl_pHelicopterEtaStrings[pHelicopterEtaStrings_SIZE] =
{
	"Całkowita trasa:  ",// total distance for helicopter to travel
	" Bezp.:   ", 			// distance to travel to destination
	" Niebezp.:", 			// distance to return from destination to airport
	"Całkowity koszt: ", 		// total cost of trip by helicopter
	"PCP:  ", 			// ETA is an acronym for "estimated time of arrival"
	"Helikopter ma mało paliwa i musi wylądować na terenie wroga.",	// warning that the sector the helicopter is going to use for refueling is under enemy control ->
	"Pasażerowie: ",
	"Wybór Skyridera lub pasażerów?",
	"Skyrider",
	"Pasażerowie",
};

static const ST::string s_pl_sMapLevelString = "Poziom:"; // what level below the ground is the player viewing in mapscreen

static const ST::string s_pl_gsLoyalString = "%d%% Lojalności"; // the loyalty rating of a town ie : Loyal 53%


// error message for when player is trying to give a merc a travel order while he's underground.
static const ST::string s_pl_gsUndergroundString = "nie można wydawać rozkazów podróży pod ziemią.";

static const ST::string s_pl_gsTimeStrings[gsTimeStrings_SIZE] =
{
	"g",				// hours abbreviation
	"m",				// minutes abbreviation
	"s",				// seconds abbreviation
	"d",				// days abbreviation
};

// text for the various facilities in the sector

static const ST::string s_pl_sFacilitiesStrings[sFacilitiesStrings_SIZE] =
{
	"Brak",
	"Szpital",
	"Przemysł",
	"Więzienie",
	"Baza wojskowa",
	"Lotnisko",
	"Strzelnica",		// a field for soldiers to practise their shooting skills
};

// text for inventory pop up button

static const ST::string s_pl_pMapPopUpInventoryText[pMapPopUpInventoryText_SIZE] =
{
	"Inwentarz",
	"Zamknij",
};

// town strings

static const ST::string s_pl_pwTownInfoStrings[pwTownInfoStrings_SIZE] =
{
	"Rozmiar",					// size of the town in sectors
	"Pod kontrolą", 					// how much of town is controlled
	"Przynależna kopalnia", 				// mine associated with this town
	"Lojalność",					// the loyalty level of this town
	"Główne obiekty", 				// main facilities in this town
	"Szkolenie cywili",				// state of civilian training in town
	"Samoobrona", 					// the state of the trained civilians in the town
};

// Mine strings

static const ST::string s_pl_pwMineStrings[pwMineStrings_SIZE] =
{
	"Kopalnia",						// 0
	"Srebro",
	"Złoto",
	"Dzienna produkcja",
	"Możliwa produkcja",
	"Opuszczona",				// 5
	"Zamknięta",
	"Na wyczerpaniu",
	"Produkuje",
	"Stan",
	"Tempo produkcji",
	"Typ złoża",				// 10
	"Kontrola miasta",
	"Lojalność miasta",
};

// blank sector strings

static const ST::string s_pl_pwMiscSectorStrings[pwMiscSectorStrings_SIZE] =
{
	"Siły wroga",
	"Sektor",
	"Przedmiotów",
	"Nieznane",
	"Pod kontrolą",
	"Tak",
	"Nie",
};

// error strings for inventory

static const ST::string s_pl_pMapInventoryErrorString[pMapInventoryErrorString_SIZE] =
{
	"Nie można wybrać tego najemnika.",  //MARK CARTER
	"%s nie może stąd zabrać tego przedmiotu, gdyż nie jest w tym sektorze.",
	"Podczas walki nie można korzystać z tego panelu.",
	"Podczas walki nie można korzystać z tego panelu.",
	"%s nie może tu zostawić tego przedmiotu, gdyż nie jest w tym sektorze.",
};

static const ST::string s_pl_pMapInventoryStrings[pMapInventoryStrings_SIZE] =
{
	"Położenie", 			// sector these items are in
	"Razem przedmiotów", 		// total number of items in sector
};


// movement menu text

static const ST::string s_pl_pMovementMenuStrings[pMovementMenuStrings_SIZE] =
{
	"Przemieść najemników %s", 	// title for movement box
	"Nanieś trasę podróży", 		// done with movement menu, start plotting movement
	"Anuluj", 		// cancel this menu
	"Inni",		// title for group of mercs not on squads nor in vehicles
};


static const ST::string s_pl_pUpdateMercStrings[pUpdateMercStrings_SIZE] =
{
	"Oj:", 			// an error has occured
	"Wygasł kontrakt najemników:", 	// this pop up came up due to a merc contract ending
	"Najemnicy wypełnili zadanie:", // this pop up....due to more than one merc finishing assignments
	"Najemnicy wrócili do pracy:", // this pop up ....due to more than one merc waking up and returing to work
	"Odpoczywający najemnicy:", // this pop up ....due to more than one merc being tired and going to sleep
	"Wkrótce wygasną kontrakty:", 	// this pop up came up due to a merc contract ending
};

// map screen map border buttons help text

static const ST::string s_pl_pMapScreenBorderButtonHelpText[pMapScreenBorderButtonHelpText_SIZE] =
{
	"Pokaż miasta (|W)",
	"Pokaż kopalnie (|M)",
	"Pokaż oddziały i wrogów (|T)",
	"Pokaż przestrzeń powietrzną (|A)",
	"Pokaż przedmioty (|I)",
	"Pokaż samoobronę i wrogów (|Z)",
};


static const ST::string s_pl_pMapScreenBottomFastHelp[pMapScreenBottomFastHelp_SIZE] =
{
	"|Laptop",
	"Ekran taktyczny (|E|s|c)",
	"|Opcje",
	"Kompresja czasu (|+)", 	// time compress more
	"Kompresja czasu (|-)", 	// time compress less
	"Poprzedni komunikat (|S|t|r|z|a|ł|k|a |w |g|ó|r|ę)\nPoprzednia strona (|P|g|U|p)", 	// previous message in scrollable list
	"Następny komunikat (|S|t|r|z|a|ł|k|a |w |d|ó|ł)\nNastępna strona (|P|g|D|n)", 	// next message in the scrollable list
	"Włącz/Wyłącz kompresję czasu (|S|p|a|c|j|a)",	// start/stop time compression
};

static const ST::string s_pl_pMapScreenBottomText = "Saldo dostępne"; // current balance in player bank account

static const ST::string s_pl_pMercDeadString = "%s nie żyje.";


static const ST::string s_pl_pDayStrings = "Dzień";

// the list of email sender names

static const ST::string s_pl_pSenderNameList[pSenderNameList_SIZE] =
{
	"Enrico",
	"Psych Pro Inc",
	"Pomoc",
	"Psych Pro Inc",
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
	"M.I.S. Ubezpieczenia",
	"Bobby Ray",
	"Kingpin",
	"John Kulba",
	"A.I.M.",
};


// new mail notify string
static const ST::string s_pl_pNewMailStrings = "Masz nową pocztę...";


// confirm player's intent to delete messages

static const ST::string s_pl_pDeleteMailStrings[pDeleteMailStrings_SIZE] =
{
	"Usunąć wiadomość?",
	"Usunąć wiadomość?",
};


// the sort header strings

static const ST::string s_pl_pEmailHeaders[pEmailHeaders_SIZE] =
{
	"Od:",
	"Temat:",
	"Dzień:",
};

// email titlebar text
static const ST::string s_pl_pEmailTitleText = "Skrzynka odbiorcza";


// the financial screen strings
static const ST::string s_pl_pFinanceTitle = "Księgowy Plus"; // the name we made up for the financial program in the game

static const ST::string s_pl_pFinanceSummary[pFinanceSummary_SIZE] =
{
	"Wypłata:", 				// credit (subtract from) to player's account
	"Wpłata:", 				// debit (add to) to player's account
	"Wczorajsze wpływy:",
	"Wczorajsze dodatkowe wpływy:",
	"Wczorajsze wydatki:",
	"Saldo na koniec dnia:",
	"Dzisiejsze wpływy:",
	"Dzisiejsze dodatkowe wpływy:",
	"Dzisiejsze wydatki:",
	"Saldo dostępne:",
	"Przewidywane wpływy:",
	"Przewidywane saldo:", 		// projected balance for player for tommorow
};


// headers to each list in financial screen

static const ST::string s_pl_pFinanceHeaders[pFinanceHeaders_SIZE] =
{
	"Dzień", 					// the day column
	"Ma", 				// the credits column
	"Winien",				// the debits column
	"Transakcja", 			// transaction type - see TransactionText below
	"Saldo", 				// balance at this point in time
	"Strona", 				// page number
	"Dzień (dni)", 		// the day(s) of transactions this page displays
};


static const ST::string s_pl_pTransactionText[pTransactionText_SIZE] =
{
	"Narosłe odsetki",			// interest the player has accumulated so far
	"Anonimowa wpłata",
	"Koszt transakcji",
	"Najęto - %s z A.I.M.", // Merc was hired
	"Zakupy u Bobby'ego Ray'a", 		// Bobby Ray is the name of an arms dealer
	"Uregulowanie rachunków w M.E.R.C.",
	"Zastaw na życie dla - %s", 		// medical deposit for merc
	"Analiza profilu w IMP", 		// IMP is the acronym for International Mercenary Profiling
	"Ubezpieczneie dla - %s",
	"Redukcja ubezp. dla - %s",
	"Przedł. ubezp. dla - %s", 				// johnny contract extended
	"Anulowano ubezp. dla - %s",
	"Odszkodowanie za - %s", 		// insurance claim for merc
	"Przedł. kontrakt z - %s o 1 dzień.", 				// entend mercs contract by a day
	"Przedł. kontrakt z - %s o 1 tydzień.",
	"Przedł. kontrakt z - %s o 2 tygodnie.",
	"Przychód z kopalni",
	"", //String nuked
	"Zakup kwiatów",
	"Pełny zwrot zastawu za - %s",
	"Częściowy zwrot zastawu za - %s",
	"Brak zwrotu zastawu za - %s",
	"Zapłata dla - %s",		// %s is the name of the npc being paid
	"Transfer funduszy do - %s", 			// transfer funds to a merc
	"Transfer funduszy od - %s", 		// transfer funds from a merc
	"Samoobrona w - %s", // initial cost to equip a town's militia
	"Zakupy u - %s.",	//is used for the Shop keeper interface.  The dealers name will be appended to the end of the string.
	"%s wpłacił(a) pieniądze.",
};

// helicopter pilot payment

static const ST::string s_pl_pSkyriderText[pSkyriderText_SIZE] =
{
	"Skyriderowi zapłacono %d$", 			// skyrider was paid an amount of money
	"Skyriderowi trzeba jeszcze zapłacić %d$", 		// skyrider is still owed an amount of money
	"Skyrider nie ma pasażerów. Jeśli chcesz przetransportować najemników, zmień ich przydział na POJAZD/HELIKOPTER.",
};


// strings for different levels of merc morale

static const ST::string s_pl_pMoralStrings[pMoralStrings_SIZE] =
{
	"Świetne",
	"Dobre",
	"Stabilne",
	"Słabe",
	"Panika",
	"Złe",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.
static const ST::string s_pl_str_left_equipment   = "%s - jego/jej sprzęt jest już w %s (%s).";

// Status that appears on the Map Screen

static const ST::string s_pl_pMapScreenStatusStrings[pMapScreenStatusStrings_SIZE] =
{
	"Zdrowie",
	"Energia",
	"Morale",
	"Stan",	// the condition of the current vehicle (its "health")
	"Paliwo",	// the fuel level of the current vehicle (its "energy")
};


static const ST::string s_pl_pMapScreenPrevNextCharButtonHelpText[pMapScreenPrevNextCharButtonHelpText_SIZE] =
{
	"Poprzedni najemnik (|S|t|r|z|a|ł|k|a |w |l|e|w|o)", 			// previous merc in the list
	"Następny najemnik (|S|t|r|z|a|ł|k|a |w |p|r|a|w|o)", 				// next merc in the list
};


static const ST::string s_pl_pEtaString = "PCP:"; // eta is an acronym for Estimated Time of Arrival

static const ST::string s_pl_pTrashItemText[pTrashItemText_SIZE] =
{
	"Więcej tego nie zobaczysz. Czy na pewno chcesz to zrobić?", 	// do you want to continue and lose the item forever
	"To wygląda na coś NAPRAWDĘ ważnego. Czy NA PEWNO chcesz to zniszczyć?", // does the user REALLY want to trash this item
};


static const ST::string s_pl_pMapErrorString[pMapErrorString_SIZE] =
{
	"Oddział nie może się przemieszczać, jeśli któryś z najemników śpi.",

//1-5
	"Najpierw wyprowadź oddział na powierzchnię.",
	"Rozkazy przemieszczenia? To jest sektor wroga!",
	"Aby podróżować najemnicy muszą być przydzieleni do oddziału lub pojazdu.",
	"Nie masz jeszcze ludzi.", // you have no members, can't do anything
	"Najemnik nie może wypełnić tego rozkazu.", // merc can't comply with your order
//6-10
	"%s musi mieć eskortę, aby się przemieszczać. Umieść go w oddziale z eskortą.", // merc can't move unescorted .. for a male
	"%s musi mieć eskortę, aby się przemieszczać. Umieść ją w oddziale z eskortą.", // for a female
	"Najemnik nie przybył jeszcze do Arulco!",
	"Wygląda na to, że trzeba wpierw uregulować sprawy kontraktu.",
	"",
//11-15
	"Rozkazy przemieszczenia? Trwa walka!",
	"Zaatakowały cię dzikie koty, w sektorze %s!",
	"W sektorze I16 znajduje się coś, co wygląda na legowisko dzikich kotów!",
	"",
	"Baza rakiet Ziemia-Powietrze została przejęta.",
//16-20
	"%s - kopalnia została przejęta. Twój dzienny przychód został zredukowany do %s.",
	"Nieprzyjaciel bezkonfliktowo przejął sektor %s.",
	"Przynajmniej jeden z twoich najemników nie został do tego przydzielony.",
	"%s nie może się przyłączyć, ponieważ %s jest pełny",
	"%s nie może się przyłączyć, ponieważ %s jest zbyt daleko.",
//21-25
	"%s - kopalnia została przejęta przez siły Deidranny!",
	"Siły Deidranny właśnie zaatakowały bazę rakiet Ziemia-Powietrze w - %s.",
	"Siły Deidranny właśnie zaatakowały - %s.",
	"Właśnie zauważono siły Deidranny w - %s.",
	"Siły Deidranny właśnie przejęły - %s.",
//26-30
	"Przynajmniej jeden z twoich najemników nie mógł się położyć spać.",
	"Przynajmniej jeden z twoich najemników nie mógł wstać.",
	"Oddziały samoobrony nie pojawią się dopóki nie zostaną wyszkolone.",
	"%s nie może się w tej chwili przemieszczać.",
	"Żołnierze samoobrony, którzy znajdują się poza granicami miasta, nie mogą być przeniesieni do innego sektora.",
//31-35
	"Nie możesz trenować samoobrony w - %s.",
	"Pusty pojazd nie może się poruszać!",
	"%s ma zbyt wiele ran by podróżować!",
	"Musisz wpierw opuścić muzeum!",
	"%s nie żyje!",
//36-40
	"%s nie może się zamienić z - %s, ponieważ się porusza",
	"%s nie może w ten sposób wejśc do pojazdu",
	"%s nie może się dołączyć do - %s",
	"Nie możesz kompresować czasu dopóki nie zatrudnisz sobie kilku nowych najemników!",
	"Ten pojazd może się poruszać tylko po drodze!",
//41-45
	"Nie można zmieniać przydziału najemników, którzy są w drodze",
	"Pojazd nie ma paliwa!",
	"%s jest zbyt zmęczony(na) by podróżować.",
	"Żaden z pasażerów nie jest w stanie kierować tym pojazdem.",
	"Jeden lub więcej członków tego oddziału nie może się w tej chwili przemieszczać.",
//46-50
	"Jeden lub więcej INNYCH członków tego oddziału nie może się w tej chwili przemieszczać.",
	"Pojazd jest uszkodzony!",
	"Pamiętaj, że w jednym sektorze tylko dwóch najemników może trenować żołnierzy samoobrony.",
	"Robot nie może się poruszać bez operatora. Umieść ich razem w jednym oddziale.",
};


// help text used during strategic route plotting
static const ST::string s_pl_pMapPlotStrings[pMapPlotStrings_SIZE] =
{
	"Kliknij ponownie sektor docelowy, aby zatwierdzić trasę podróży, lub kliknij inny sektor, aby ją wydłużyć.",
	"Trasa podróży zatwierdzona.",
	"Cel podróży nie został zmieniony.",
	"Trasa podróży została anulowana.",
	"Trasa podróży została skrócona.",
};


// help text used when moving the merc arrival sector
static const ST::string s_pl_pBullseyeStrings[pBullseyeStrings_SIZE] =
{
	"Kliknij sektor, do którego mają przylatywać najemnicy.",
	"Dobrze. Przylatujący najemnicy będą zrzucani w %s",
	"Najemnicy nie mogą tu przylatywać. Przestrzeń powietrzna nie jest zabezpieczona!",
	"Anulowano. Sektor zrzutu nie został zmieniony.",
	"Przestrzeń powietrzna nad %s nie jest już bezpieczna! Sektor zrzutu został przesunięty do %s.",
};


// help text for mouse regions

static const ST::string s_pl_pMiscMapScreenMouseRegionHelpText[pMiscMapScreenMouseRegionHelpText_SIZE] =
{
	"Otwórz wyposażenie (|E|n|t|e|r)",
	"Zniszcz przedmiot",
	"Zamknij wyposażenie (|E|n|t|e|r)",
};


static const ST::string s_pl_str_he_leaves_where_drop_equipment  = "Czy %s ma zostawić swój sprzęt w sektorze, w którym się obecnie znajduje (%s), czy w %s (%s), skąd odlatuje?";
static const ST::string s_pl_str_she_leaves_where_drop_equipment = "Czy %s ma zostawić swój sprzęt w sektorze, w którym się obecnie znajduje (%s), czy w %s (%s), skąd odlatuje?";
static const ST::string s_pl_str_he_leaves_drops_equipment       = "%s wkrótce odchodzi i zostawi swój sprzęt w %s.";
static const ST::string s_pl_str_she_leaves_drops_equipment      = "%s wkrótce odchodzi i zostawi swój sprzęt w %s.";


// Text used on IMP Web Pages

static const ST::string s_pl_pImpPopUpStrings[pImpPopUpStrings_SIZE] =
{
	"Nieprawidłowy kod dostępu",
	"Czy na pewno chcesz wznowić proces określenia profilu?",
	"Wprowadź nazwisko oraz płeć",
	"Wstępna kontrola stanu twoich finansów wykazała, że nie stać cię na analizę profilu.",
	"Opcja tym razem nieaktywna.",
	"Aby wykonać profil, musisz mieć miejsce dla przynajmniej jednego członka załogi.",
	"Profil został już wykonany.",
};


// button labels used on the IMP site

static const ST::string s_pl_pImpButtonText[pImpButtonText_SIZE] =
{
	"O Nas", 			// about the IMP site
	"ZACZNIJ", 			// begin profiling
	"Osobowość", 		// personality section
	"Atrybuty", 		// personal stats/attributes section
	"Portret", 			// the personal portrait selection
	"Głos %d", 			// the voice selection
	"Gotowe", 			// done profiling
	"Zacznij od początku", 		// start over profiling
	"Tak, wybieram tą odpowiedź.",
	"Tak",
	"Nie",
	"Skończone", 			// finished answering questions
	"Poprz.", 			// previous question..abbreviated form
	"Nast.", 			// next question
	"TAK, JESTEM.", 		// yes, I am certain
	"NIE, CHCĘ ZACZĄĆ OD NOWA.", // no, I want to start over the profiling process
	"TAK",
	"NIE",
	"Wstecz", 			// back one page
	"Anuluj", 			// cancel selection
	"Tak.",
	"Nie, Chcę spojrzeć jeszcze raz.",
	"Rejestr", 			// the IMP site registry..when name and gender is selected
	"Analizuję...", 			// analyzing your profile results
	"OK",
	"Głos",
	"Specialties",			// "Specialties" - the skills selection screen
};

static const ST::string s_pl_pExtraIMPStrings[pExtraIMPStrings_SIZE] =
{
	"Aby zacząć analizę profilu, wybierz osobowość.",
	"Teraz określ swoje atrybuty.",
	"Teraz możesz przystąpić do wyboru portretu.",
	"Aby zakończyć proces, wybierz próbkę głosu, która ci najbardziej odpowiada."
};

static const ST::string s_pl_gzIMPSkillTraitsText[gzIMPSkillTraitsText_SIZE] =
{
	s_pl_gzMercSkillText[1],
	s_pl_gzMercSkillText[2],
	s_pl_gzMercSkillText[3],
	s_pl_gzMercSkillText[4],
	s_pl_gzMercSkillText[5],
	s_pl_gzMercSkillText[6],
	s_pl_gzMercSkillText[7],
	s_pl_gzMercSkillText[8],
	s_pl_gzMercSkillText[9],
	s_pl_gzMercSkillText[10],
	s_pl_gzMercSkillText[13],
	s_pl_gzMercSkillText[14],
	s_pl_gzMercSkillText[15],
	s_pl_gzMercSkillText[12],

	s_pl_gzMercSkillText[0],
	"I.M.P. Specialties"
};

static const ST::string s_pl_pFilesTitle = "Przeglądarka plików";

static const ST::string s_pl_pFilesSenderList[pFilesSenderList_SIZE] =
{
	"Raport Rozp.", // the recon report sent to the player. Recon is an abbreviation for reconissance
	"Intercept #1", // first intercept file .. Intercept is the title of the organization sending the file...similar in function to INTERPOL/CIA/KGB..refer to fist record in files.txt for the translated title
	"Intercept #2", // second intercept file
	"Intercept #3", // third intercept file
	"Intercept #4", // fourth intercept file
	"Intercept #5", // fifth intercept file
	"Intercept #6", // sixth intercept file
};

// Text having to do with the History Log
static const ST::string s_pl_pHistoryTitle = "Historia";

static const ST::string s_pl_pHistoryHeaders[pHistoryHeaders_SIZE] =
{
	"Dzień", 			// the day the history event occurred
	"Strona", 			// the current page in the history report we are in
	"Dzień", 			// the days the history report occurs over
	"Położenie", 			// location (in sector) the event occurred
	"Zdarzenie", 			// the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
static const ST::string s_pl_pHistoryStrings[pHistoryStrings_SIZE] =
{
	"",																						// leave this line blank
	//1-5
	"%s najęty(ta) w A.I.M.", 										// merc was hired from the aim site
	"%s najęty(ta) w M.E.R.C.", 									// merc was hired from the aim site
	"%s ginie.", 															// merc was killed
	"Uregulowano rachunki w M.E.R.C.",								// paid outstanding bills at MERC
	"Przyjęto zlecenie od Enrico Chivaldori",
	//6-10
	"Profil IMP wygenerowany",
	"Podpisano umowę ubezpieczeniową dla %s.", 				// insurance contract purchased
	"Anulowano umowę ubezpieczeniową dla %s.", 				// insurance contract canceled
	"Wypłata ubezpieczenia za %s.", 							// insurance claim payout for merc
	"Przedłużono kontrakt z: %s o 1 dzień.", 						// Extented "mercs name"'s for a day
	//11-15
	"Przedłużono kontrakt z: %s o 1 tydzień.", 					// Extented "mercs name"'s for a week
	"Przedłużono kontrakt z: %s o 2 tygodnie.", 					// Extented "mercs name"'s 2 weeks
	"%s zwolniony(na).", 													// "merc's name" was dismissed.
	"%s odchodzi.", 																		// "merc's name" quit.
	"przyjęto zadanie.", 															// a particular quest started
	//16-20
	"zadanie wykonane.",
	"Rozmawiano szefem kopalni %s",									// talked to head miner of town
	"Wyzwolono - %s",
	"Użyto kodu Cheat",
	"Żywność powinna być jutro w Omercie",
	//21-25
	"%s odchodzi, aby wziąć ślub z Darylem Hickiem",
	"Wygasł kontrakt z - %s.",
	"%s zrekrutowany(na).",
	"Enrico narzeka na brak postępów",
	"Walka wygrana",
	//26-30
	"%s - w kopalni kończy się ruda",
	"%s - w kopalni skończyła się ruda",
	"%s - kopalnia została zamknięta",
	"%s - kopalnia została otwarta",
	"Informacja o więzieniu zwanym Tixa.",
	//31-35
	"Informacja o tajnej fabryce broni zwanej Orta.",
	"Naukowiec w Orcie ofiarował kilka karabinów rakietowych.",
	"Królowa Deidranna robi użytek ze zwłok.",
	"Frank opowiedział o walkach w San Monie.",
	"Pewien pacjent twierdzi, że widział coś w kopalni.",
	//36-40
	"Gość o imieniu Devin sprzedaje materiały wybuchowe.",
	"Spotkanie ze sławynm eks-najemnikiem A.I.M. - Mike'iem!",
	"Tony handluje bronią.",
	"Otrzymano karabin rakietowy od sierżanta Krotta.",
	"Dano Kyle'owi akt własności sklepu Angela.",
	//41-45
	"Madlab zaoferował się zbudować robota.",
	"Gabby potrafi zrobić miksturę chroniącą przed robakami.",
	"Keith wypadł z interesu.",
	"Howard dostarczał cyjanek królowej Deidrannie.",
	"Spotkanie z handlarzem Keithem w Cambrii.",
	//46-50
	"Spotkanie z aptekarzem Howardem w Balime",
	"Spotkanie z Perko, prowadzącym mały warsztat.",
	"Spotkanie z Samem z Balime - prowadzi sklep z narzędziami.",
	"Franz handluje sprzętem elektronicznym.",
	"Arnold prowadzi warsztat w Grumm.",
	//51-55
	"Fredo naprawia sprzęt elektroniczny w Grumm.",
	"Otrzymano darowiznę od bogatego gościa w Balime.",
	"Spotkano Jake'a, który prowadzi złomowisko.",
	"Jakiś włóczęga dał nam elektroniczną kartę dostępu.",
	"Przekupiono Waltera, aby otworzył drzwi do piwnicy.",
	//56-60
	"Dave oferuje darmowe tankowania, jeśli będzie miał paliwo.",
	"Greased Pablo's palms.",
	"Kingpin trzyma pieniądze w kopalni w San Mona.",
	"%s wygrał(a) walkę",
	"%s przegrał(a) walkę",
	//61-65
	"%s zdyskwalifikowany(na) podczas walki",
	"Znaleziono dużo pieniędzy w opuszczonej kopalni.",
	"Spotkano zabójcę nasłanego przez Kingpina.",
	"Utrata kontroli nad sektorem",				//ENEMY_INVASION_CODE
	"Sektor obroniony",
	//66-70
	"Przegrana bitwa",							//ENEMY_ENCOUNTER_CODE
	"Fatalna zasadzka",						//ENEMY_AMBUSH_CODE
	"Usunieto zasadzkę wroga",
	"Nieudany atak",			//ENTERING_ENEMY_SECTOR_CODE
	"Udany atak!",
	//71-75
	"Stworzenia zaatakowały",			//CREATURE_ATTACK_CODE
	"Zabity(ta) przez dzikie koty",			//BLOODCAT_AMBUSH_CODE
	"Wyrżnięto dzikie koty",
	"%s zabity(ta)",
	"Przekazano Carmenowi głowę terrorysty",
	"Slay odszedł",
	"Zabito: %s",
};

static const ST::string s_pl_pHistoryLocations = "N/D"; // N/A is an acronym for Not Applicable

// icon text strings that appear on the laptop

static const ST::string s_pl_pLaptopIcons[pLaptopIcons_SIZE] =
{
	"E-mail",
	"Sieć",
	"Finanse",
	"Personel",
	"Historia",
	"Pliki",
	"Zamknij",
	"sir-FER 4.0",			// our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added

static const ST::string s_pl_pBookMarkStrings[pBookMarkStrings_SIZE] =
{
	"A.I.M.",
	"Bobby Ray's",
	"I.M.P",
	"M.E.R.C.",
	"Pogrzeby",
	"Kwiaty",
	"Ubezpieczenia",
	"Anuluj",
};

// When loading or download a web page

static const ST::string s_pl_pDownloadString[pDownloadString_SIZE] =
{
	"Ładowanie strony...",
	"Otwieranie strony...",
};

//This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

static const ST::string s_pl_gsAtmStartButtonText[gsAtmStartButtonText_SIZE] =
{
	"Atrybuty", 			// view stats of the merc
	"Wyposażenie", 			// view the inventory of the merc
	"Zatrudnienie",
};

// Web error messages. Please use foreign language equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator

static const ST::string s_pl_pErrorStrings = "Niestabilne połączenie z Hostem. Transfer może trwać dłużej.";


static const ST::string s_pl_pPersonnelString = "Najemnicy:"; // mercs we have


static const ST::string s_pl_pWebTitle = "sir-FER 4.0"; // our name for the version of the browser, play on company name


// The titles for the web program title bar, for each page loaded

static const ST::string s_pl_pWebPagesTitles[pWebPagesTitles_SIZE] =
{
	"A.I.M.",
	"A.I.M. Członkowie",
	"A.I.M. Portrety",		// a mug shot is another name for a portrait
	"A.I.M. Lista",
	"A.I.M.",
	"A.I.M. Weterani",
	"A.I.M. Polisy",
	"A.I.M. Historia",
	"A.I.M. Linki",
	"M.E.R.C.",
	"M.E.R.C. Konta",
	"M.E.R.C. Rejestracja",
	"M.E.R.C. Indeks",
	"Bobby Ray's",
	"Bobby Ray's - Broń",
	"Bobby Ray's - Amunicja",
	"Bobby Ray's - Pancerz",
	"Bobby Ray's - Różne",							//misc is an abbreviation for miscellaneous
	"Bobby Ray's - Używane",
	"Bobby Ray's - Zamówienie pocztowe",
	"I.M.P.",
	"I.M.P.",
	"United Floral Service",
	"United Floral Service - Galeria",
	"United Floral Service - Zamówienie",
	"United Floral Service - Galeria kartek",
	"Malleus, Incus & Stapes - Brokerzy ubezpieczeniowi",
	"Informacja",
	"Kontrakt",
	"Uwagi",
	"McGillicutty - Zakład pogrzebowy",
	"Nie odnaleziono URL.",
	"Bobby Ray's - Ostatnie dostawy",
	"",
	"",
};

static const ST::string s_pl_pShowBookmarkString[pShowBookmarkString_SIZE] =
{
	"Sir-Pomoc",
	"Kliknij ponownie Sieć by otworzyć menu Ulubione.",
};

static const ST::string s_pl_pLaptopTitles[pLaptopTitles_SIZE] =
{
	"Poczta",
	"Przeglądarka plików",
	"Personel",
	"Księgowy Plus",
	"Historia",
};

static const ST::string s_pl_pPersonnelDepartedStateStrings[pPersonnelDepartedStateStrings_SIZE] =
{
	//reasons why a merc has left.
	"Śmierć w akcji",
	"Zwolnienie",
	"Małżeństwo",
	"Koniec kontraktu",
	"Rezygnacja",
};
// personnel strings appearing in the Personnel Manager on the laptop

static const ST::string s_pl_pPersonelTeamStrings[pPersonelTeamStrings_SIZE] =
{
	"Bieżący oddział",
	"Wyjazdy",
	"Koszt dzienny:",
	"Najwyższy koszt:",
	"Najniższy koszt:",
	"Śmierć w akcji:",
	"Zwolnienie:",
	"Inny:",
};


static const ST::string s_pl_pPersonnelCurrentTeamStatsStrings[pPersonnelCurrentTeamStatsStrings_SIZE] =
{
	"Najniższy",
	"Średni",
	"Najwyższy",
};


static const ST::string s_pl_pPersonnelTeamStatsStrings[pPersonnelTeamStatsStrings_SIZE] =
{
	"ZDR",
	"ZWN",
	"ZRCZ",
	"SIŁA",
	"DOW",
	"INT",
	"DOŚW",
	"STRZ",
	"MECH",
	"WYB",
	"MED",
};


// horizontal and vertical indices on the map screen

static const ST::string s_pl_pMapVertIndex[pMapVertIndex_SIZE] =
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

static const ST::string s_pl_pMapHortIndex[pMapHortIndex_SIZE] =
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

static const ST::string s_pl_pMapDepthIndex[pMapDepthIndex_SIZE] =
{
	"",
	"-1",
	"-2",
	"-3",
};

// text that appears on the contract button

static const ST::string s_pl_pContractButtonString = "Kontrakt";

// text that appears on the update panel buttons

static const ST::string s_pl_pUpdatePanelButtons[pUpdatePanelButtons_SIZE] =
{
	"Dalej",
	"Stop",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle

static const ST::string s_pl_LargeTacticalStr[LargeTacticalStr_SIZE] =
{
	"Pokonano cię w tym sektorze!",
	"Wróg nie zna litości i pożera was wszystkich!",
	"Nieprzytomni członkowie twojego oddziału zostali pojmani!",
	"Członkowie twojego oddziału zostali uwięzieni.",
};


//Insurance Contract.c
//The text on the buttons at the bottom of the screen.

static const ST::string s_pl_InsContractText[InsContractText_SIZE] =
{
	"Wstecz",
	"Dalej",
	// "Akceptuję",
	"OK",
	"Skasuj",
};



//Insurance Info
// Text on the buttons on the bottom of the screen

static const ST::string s_pl_InsInfoText[InsInfoText_SIZE] =
{
	"Wstecz",
	"Dalej"
};



//For use at the M.E.R.C. web site. Text relating to the player's account with MERC

static const ST::string s_pl_MercAccountText[MercAccountText_SIZE] =
{
	// Text on the buttons on the bottom of the screen
	"Autoryzacja",
	"Strona główna",
	"Konto #:",
	"Najemnik",
	"Dni",
	"Stawka",	//5
	"Opłata",
	"Razem:",
	"Czy na pewno chcesz zatwierdzić płatność: %s?",		//the %s is a string that contains the dollar amount ( ex. "$150" )
};



//For use at the M.E.R.C. web site. Text relating a MERC mercenary


static const ST::string s_pl_MercInfo[MercInfo_SIZE] =
{
	"Poprzedni",
	"Najmij",
	"Następny",
	"Dodatkowe informacje",
	"Strona główna",
	"Najęty",
	"Koszt:",
	"Dziennie",
	"Nie żyje",

	"Wygląda na to, że chcesz wynająć zbyt wielu najemników. Limit wynosi 18.",
	"Niedostępny",
};



// For use at the M.E.R.C. web site. Text relating to opening an account with MERC

static const ST::string s_pl_MercNoAccountText[MercNoAccountText_SIZE] =
{
	//Text on the buttons at the bottom of the screen
	"Otwórz konto",
	"Anuluj",
	"Nie posiadasz konta. Czy chcesz sobie założyć?"
};



// For use at the M.E.R.C. web site. MERC Homepage

static const ST::string s_pl_MercHomePageText[MercHomePageText_SIZE] =
{
	//Description of various parts on the MERC page
	"Speck T. Kline, założyciel i właściciel",
	"Aby otworzyć konto naciśnij tu",
	"Aby zobaczyć konto naciśnij tu",
	"Aby obejrzeć akta naciśnij tu",
	// The version number on the video conferencing system that pops up when Speck is talking
	"Speck Com v3.2",
};

// For use at MiGillicutty's Web Page.

static const ST::string s_pl_sFuneralString[sFuneralString_SIZE] =
{
	"Zakład pogrzebowy McGillicutty, pomaga rodzinom pogrążonym w smutku od 1983.",
	"Kierownik, były najemnik A.I.M. Murray \'Pops\' McGillicutty jest doświadczonym pracownikiem zakładu pogrzebowego.",
	"Przez całe życie obcował ze śmiercią, 'Pops' wie jak trudne są te chwile.",
	"Zakład pogrzebowy McGillicutty oferuje szeroki zakres usług, od duchowego wsparcia po rekonstrukcję silnie zniekształconych zwłok.",
	"Pozwól by McGillicutty ci pomógł a twój ukochany będzie spoczywał w pokoju.",

	// Text for the various links available at the bottom of the page
	"WYŚLIJ KWIATY",
	"KOLEKCJA TRUMIEN I URN",
	"USŁUGI KREMA- CYJNE",
	"USŁUGI PLANOWANIA POGRZEBU",
	"KARTKI POGRZE- BOWE",

	// The text that comes up when you click on any of the links ( except for send flowers ).
	"Niestety, z powodu śmierci w rodzinie, nie działają jeszcze wszystkie elementy tej strony.",
	"Przepraszamy za powyższe uniedogodnienie."
};

// Text for the florist Home page

static const ST::string s_pl_sFloristText[sFloristText_SIZE] =
{
	//Text on the button on the bottom of the page

	"Galeria",

	//Address of United Florist

	"\"Zrzucamy z samolotu w dowolnym miejscu\"",
	"1-555-POCZUJ-MNIE",
	"Ul. Nosowska 333, Zapuszczone miasto, CA USA 90210",
	"http://www.poczuj-mnie.com",

	// detail of the florist page

	"Działamy szybko i sprawnie!",
	"Gwarantujemy dostawę w dowolny punkt na Ziemi, następnego dnia po złożeniu zamówienia!",
	"Oferujemy najniższe ceny na świecie!",
	"Pokaż nam ofertę z niższą ceną, a dostaniesz w nagrodę tuzin róż, za darmo!",
	"Latająca flora, fauna i kwiaty od 1981.",
	"Nasz ozdobiony bombowiec zrzuci twój bukiet w promieniu co najwyżej dziesięciu mil od żądanego miejsca. Kiedy tylko zechcesz!",
	"Pozwól nam zaspokoić twoje kwieciste fantazje.",
	"Bruce, nasz światowej renomy projektant bukietów, zerwie dla ciebie najświeższe i najwspanialsze kwiaty z naszej szklarni.",
	"I pamiętaj, jeśli czegoś nie mamy, możemy to szybko zasadzić!"
};



//Florist OrderForm

static const ST::string s_pl_sOrderFormText[sOrderFormText_SIZE] =
{
	//Text on the buttons

	"Powrót",
	"Wyślij",
	"Skasuj",
	"Galeria",

	"Nazwa bukietu:",
	"Cena:",			//5
	"Zamówienie numer:",
	"Czas dostawy",
	"nast. dnia",
	"dostawa gdy to będzie możliwe",
	"Miejsce dostawy",			//10
	"Dodatkowe usługi",
	"Zgnieciony bukiet($10)",
	"Czarne Róże($20)",
	"Zwiędnięty bukiet($10)",
	"Ciasto owocowe (jeżeli będzie)($10)",		//15
	"Osobiste kondolencje:",
	"Ze względu na rozmiar karteczek, tekst nie może zawierać więcej niż 75 znaków.",
	"...możesz też przejrzeć nasze",

	"STANDARDOWE KARTKI",
	"Informacja o rachunku",//20

	//The text that goes beside the area where the user can enter their name

	"Nazwisko:",
};




//Florist Gallery.c

static const ST::string s_pl_sFloristGalleryText[sFloristGalleryText_SIZE] =
{
	//text on the buttons

	"Poprz.",	//abbreviation for previous
	"Nast.",	//abbreviation for next

	"Kliknij wybraną pozycję aby złożyć zamówienie.",
	"Uwaga: $10 dodatkowej opłaty za zwiędnięty lub zgnieciony bukiet.",

	//text on the button

	"Główna",
};

//Florist Cards

static const ST::string s_pl_sFloristCards[sFloristCards_SIZE] =
{
	"Kliknij swój wybór",
	"Wstecz"
};



// Text for Bobby Ray's Mail Order Site

static const ST::string s_pl_BobbyROrderFormText[BobbyROrderFormText_SIZE] =
{
	"Formularz zamówienia",				//Title of the page
	"Ilość",					// The number of items ordered
	"Waga (%s)",			// The weight of the item
	"Nazwa",				// The name of the item
	"Cena",				// the item's weight
	"Wartość",				//5	// The total price of all of items of the same type
	"W sumie",				// The sub total of all the item totals added
	"Transport",		// S&H is an acronym for Shipping and Handling
	"Razem",			// The grand total of all item totals + the shipping and handling
	"Miejsce dostawy",
	"Czas dostawy",			//10	// See below
	"Koszt (za %s.)",			// The cost to ship the items
	"Ekspres - 24h",			// Gets deliverd the next day
	"2 dni robocze",			// Gets delivered in 2 days
	"Standardowa dostawa",			// Gets delivered in 3 days
	" Wyczyść",//15			// Clears the order page
	" Akceptuję",			// Accept the order
	"Wstecz",				// text on the button that returns to the previous page
	"Strona główna",				// Text on the button that returns to the home page
	"* oznacza używane rzeczy",		// Disclaimer stating that the item is used
	"Nie stać cię na to.",		//20	// A popup message that to warn of not enough money
	"<BRAK>",				// Gets displayed when there is no valid city selected
	"Miejsce docelowe przesyłki: %s. Potwierdzasz?",		// A popup that asks if the city selected is the correct one
	"Waga przesyłki*",			// Displays the weight of the package
	"* Min. Waga",				// Disclaimer states that there is a minimum weight for the package
	"Dostawy",
};


// This text is used when on the various Bobby Ray Web site pages that sell items

static const ST::string s_pl_BobbyRText[BobbyRText_SIZE] =
{
	"Zamów",				// Title

	"Kliknij wybrane towary. Lewym klawiszem zwiększasz ilość towaru, a prawym zmniejszasz. Gdy już skompletujesz swoje zakupy przejdź do formularza zamówienia.",			// instructions on how to order

	//Text on the buttons to go the various links

	"Poprzednia",		//
	"Broń", 			//3
	"Amunicja",			//4
	"Ochraniacze",			//5
	"Różne",			//6	//misc is an abbreviation for miscellaneous
	"Używane",			//7
	"Następna",
	"FORMULARZ",
	"Strona główna",			//10

	//The following lines provide information on the items

	"Waga:",			// Weight of all the items of the same type
	"Kal:",			// the caliber of the gun
	"Mag:",			// number of rounds of ammo the Magazine can hold
	"Zas:",				// The range of the gun
	"Siła:",				// Damage of the weapon
	"CS:",			// Weapon's Rate Of Fire, acroymn ROF
	"Koszt:",			// Cost of the item
	"Na stanie:",			// The number of items still in the store's inventory
	"Ilość na zamów.:",		// The number of items on order
	"Uszkodz.",			// If the item is damaged
	"Razem:",			// The total cost of all items on order
	"* Stan: %",		// if the item is damaged, displays the percent function of the item

	//Popup that tells the player that they can only order 10 items at a time

	"Przepraszamy za to utrudnienie, ale na jednym zamówieniu może się znajdować tylko 10 pozycji! Jeśli potrzebujesz więcej, złóż kolejne zamówienie.",

	// A popup that tells the user that they are trying to order more items then the store has in stock

	"Przykro nam. Chwilowo nie mamy tego więcej na magazynie. Proszę spróbować później.",

	//A popup that tells the user that the store is temporarily sold out

	"Przykro nam, ale chwilowo nie mamy tego towaru na magazynie",
};


// The following line is used on the Ammunition page.  It is used for help text
// to display how many items the player's merc has that can use this type of
// ammo.
static const ST::string s_pl_str_bobbyr_guns_num_guns_that_use_ammo = "Twój zespół posiada %d szt. broni do której pasuje amunicja tego typu";


// Text for Bobby Ray's Home Page

static const ST::string s_pl_BobbyRaysFrontText[BobbyRaysFrontText_SIZE] =
{
	//Details on the web site

	"Tu znajdziesz nowości z dziedziny broni i osprzętu wojskowego",
	"Zaspokoimy wszystkie twoje potrzeby w dziedzinie materiałów wybuchowych",
	"UŻYWANE RZECZY",

	//Text for the various links to the sub pages

	"RÓŻNE",
	"BROŃ",
	"AMUNICJA",		//5
	"OCHRANIACZE",

	//Details on the web site

	"Jeśli MY tego nie mamy, to znaczy, że nigdzie tego nie dostaniesz!",
	"W trakcie budowy",
};



// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug shot page

static const ST::string s_pl_AimSortText[AimSortText_SIZE] =
{
	"Członkowie A.I.M.",				// Title

	"Sortuj wg:",					// Title for the way to sort

	//Text of the links to other AIM pages

	"Portrety najemników",
	"Akta najemnika",
	"Pokaż galerię byłych członków A.I.M."
};


// text to display how the entries will be sorted
static const ST::string s_pl_str_aim_sort_price        = "Ceny";
static const ST::string s_pl_str_aim_sort_experience   = "Doświadczenia";
static const ST::string s_pl_str_aim_sort_marksmanship = "Um. strzeleckich";
static const ST::string s_pl_str_aim_sort_medical      = "Um. med.";
static const ST::string s_pl_str_aim_sort_explosives   = "Zn. mat. wyb.";
static const ST::string s_pl_str_aim_sort_mechanical   = "Zn. mechaniki";
static const ST::string s_pl_str_aim_sort_ascending    = "Rosnąco";
static const ST::string s_pl_str_aim_sort_descending   = "Malejąco";


//Aim Policies.c
//The page in which the AIM policies and regulations are displayed

static const ST::string s_pl_AimPolicyText[AimPolicyText_SIZE] =
{
	// The text on the buttons at the bottom of the page

	"Poprzednia str.",
	"Strona główna",
	"Przepisy",
	"Następna str.",
	"Rezygnuję",
	"Akceptuję",
};



//Aim Member.c
//The page in which the players hires AIM mercenaries

// Instructions to the user to either start video conferencing with the merc, or to go the mug shot index

static const ST::string s_pl_AimMemberText[AimMemberText_SIZE] =
{
	"Lewy klawisz myszy",
	"kontakt z najemnikiem",
	"Prawy klawisz myszy",
	"lista portretów",
};

//Aim Member.c
//The page in which the players hires AIM mercenaries

static const ST::string s_pl_CharacterInfo[CharacterInfo_SIZE] =
{
	// the contract expenses' area

	"Zapłata",
	"Czas",
	"1 dzień",
	"1 tydzień",
	"2 tygodnie",

	// text for the buttons that either go to the previous merc,
	// start talking to the merc, or go to the next merc

	"Poprzedni",
	"Kontakt",
	"Następny",

	"Dodatkowe informacje",				// Title for the additional info for the merc's bio
	"Aktywni członkowie", // Title of the page
	"Opcjonalne wyposażenie:",				// Displays the optional gear cost
	"Wymagany jest zastaw na życie",			// If the merc required a medical deposit, this is displayed
};


//Aim Member.c
//The page in which the player's hires AIM mercenaries

//The following text is used with the video conference popup

static const ST::string s_pl_VideoConfercingText[VideoConfercingText_SIZE] =
{
	"Wartość kontraktu:",				//Title beside the cost of hiring the merc

	//Text on the buttons to select the length of time the merc can be hired

	"Jeden dzień",
	"Jeden tydzień",
	"Dwa tygodnie",

	//Text on the buttons to determine if you want the merc to come with the equipment

	"Bez sprzętu",
	"Weź sprzęt",

	// Text on the Buttons

	"TRANSFER",			// to actually hire the merc
	"ANULUJ",				// go back to the previous menu
	"WYNAJMIJ",				// go to menu in which you can hire the merc
	"ROZŁĄCZ",				// stops talking with the merc
	"OK",
	"NAGRAJ SIĘ",			// if the merc is not there, you can leave a message

	//Text on the top of the video conference popup

	"Wideo konferencja z - ",
	"Łączę. . .",

	"z zastawem"			// Displays if you are hiring the merc with the medical deposit
};



//Aim Member.c
//The page in which the player hires AIM mercenaries

// The text that pops up when you select the TRANSFER FUNDS button

static const ST::string s_pl_AimPopUpText[AimPopUpText_SIZE] =
{
	"TRANSFER ZAKOŃCZONY POMYŚLNIE",	// You hired the merc
	"PRZEPROWADZENIE TRANSFERU NIE MOŻLIWE",		// Player doesn't have enough money, message 1
	"BRAK ŚRODKÓW",				// Player doesn't have enough money, message 2

	// if the merc is not available, one of the following is displayed over the merc's face

	"Wynajęto",
	"Proszę zostaw wiadomość",
	"Nie żyje",

	//If you try to hire more mercs than game can support

	"Masz już pełny zespół 18 najemników.",

	"Nagrana wiadomość",
	"Wiadomość zapisana",
};


//AIM Link.c

static const ST::string s_pl_AimLinkText = "A.I.M. Linki"; // The title of the AIM links page



//Aim History

// This page displays the history of AIM

static const ST::string s_pl_AimHistoryText[AimHistoryText_SIZE] =
{
	"A.I.M. Historia",					//Title

	// Text on the buttons at the bottom of the page

	"Poprzednia str.",
	"Strona główna",
	"Byli członkowie",
	"Następna str."
};


//Aim Mug Shot Index

//The page in which all the AIM members' portraits are displayed in the order selected by the AIM sort page.

static const ST::string s_pl_AimFiText[AimFiText_SIZE] =
{
	// displays the way in which the mercs were sorted

	"ceny",
	"doświadczenia",
	"um. strzeleckich",
	"um. medycznych",
	"zn. materiałów wyb.",
	"zn. mechaniki",

	// The title of the page, the above text gets added at the end of this text

	"Członkowie A.I.M. posortowani rosnąco wg %s",
	"Członkowie A.I.M. posortowani malejąco wg %s",

	// Instructions to the players on what to do

	"Lewy klawisz",
	"Wybór najemnika",			//10
	"Prawy klawisz",
	"Opcje sortowania",

	// Gets displayed on top of the merc's portrait if they are...

	"Nie żyje",						//14
	"Wynajęto",
};



//AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer with AIM

static const ST::string s_pl_AimAlumniText[AimAlumniText_SIZE] =
{

	"STRONA 1",
	"STRONA 2",
	"STRONA 3",

	"Byli członkowie A.I.M.",	// Title of the page


	"OK"			// Stops displaying information on selected merc
};






//AIM Home Page

static const ST::string s_pl_AimScreenText[AimScreenText_SIZE] =
{
	// AIM disclaimers

	"Znaki A.I.M. i logo A.I.M. są prawnie chronione w większości krajów.",
	"Więc nawet nie myśl o próbie ich podrobienia.",
	"Copyright 1998-1999 A.I.M., Ltd. All rights reserved.",

	//Text for an advertisement that gets displayed on the AIM page

	"United Floral Service",
	"\"Zrzucamy gdziekolwiek\"",				//10
	"Zrób to jak należy...",
	"...za pierwszym razem",
	"Broń i akcesoria, jeśli czegoś nie mamy, to tego nie potrzebujesz.",
};


//Aim Home Page

static const ST::string s_pl_AimBottomMenuText[AimBottomMenuText_SIZE] =
{
	//Text for the links at the bottom of all AIM pages
	"Strona główna",
	"Członkowie",
	"Byli członkowie",
	"Przepisy",
	"Historia",
	"Linki",
};



//ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.

static const ST::string s_pl_SKI_Text[SKI_SIZE ] =
{
	"TOWARY NA STANIE",		//Header for the merchandise available
	"STRONA",				//The current store inventory page being displayed
	"KOSZT OGÓŁEM",				//The total cost of the the items in the Dealer inventory area
	"WARTOŚĆ OGÓŁEM",			//The total value of items player wishes to sell
	"WYCENA",				//Button text for dealer to evaluate items the player wants to sell
	"TRANSAKCJA",			//Button text which completes the deal. Makes the transaction.
	"OK",				//Text for the button which will leave the shopkeeper interface.
	"KOSZT NAPRAWY",			//The amount the dealer will charge to repair the merc's goods
	"1 GODZINA",			// SINGULAR! The text underneath the inventory slot when an item is given to the dealer to be repaired
	"%d GODZIN(Y)",		// PLURAL!   The text underneath the inventory slot when an item is given to the dealer to be repaired
	"NAPRAWIONO",		// Text appearing over an item that has just been repaired by a NPC repairman dealer
	"Brak miejsca by zaoferować więcej rzeczy.",	//Message box that tells the user there is no more room to put there stuff
	"%d MINUT(Y)",		// The text underneath the inventory slot when an item is given to the dealer to be repaired
	"Upuść przedmiot na ziemię.",
};


static const ST::string s_pl_SkiMessageBoxText[SkiMessageBoxText_SIZE] =
{
	"Czy chcesz dołożyć %s ze swojego konta, aby pokryć różnicę?",
	"Brak środków. Brakuje ci %s",
	"Czy chcesz przeznaczyć %s ze swojego konta, aby pokryć koszty?",
	"Poproś o rozpoczęcie transakscji",
	"Poproś o naprawę wybranych przedmiotów",
	"Zakończ rozmowę",
	"Saldo dostępne",
};


//OptionScreen.c

static const ST::string s_pl_zOptionsText[zOptionsText_SIZE] =
{
	//button Text
	"Zapisz grę",
	"Odczytaj grę",
	"Wyjście",
	"OK",

	//Text above the slider bars
	"Efekty",
	"Dialogi",
	"Muzyka",

	//Confirmation pop when the user selects..
	"Zakończyć grę i wrócić do głównego menu?",

	"Musisz włączyć opcję dialogów lub napisów.",
};


//SaveLoadScreen
static const ST::string s_pl_zSaveLoadText[zSaveLoadText_SIZE] =
{
	"Zapisz grę",
	"Odczytaj grę",
	"Anuluj",
	"Zapisz wybraną",
	"Odczytaj wybraną",

	"Gra została pomyślnie zapisana",
	"BŁĄD podczas zapisu gry!",
	"Gra została pomyślnie odczytana",
	"BŁĄD podczas odczytu gry: \"%s\"",

	"Wersja gry w zapisanym pliku różni się od bieżącej. Prawdopodobnie można bezpiecznie kontynuować.",
	"Czy na pewno chcesz usunąć zapisaną grę o nazwie \"%s\"?",

	"Uwaga:",
	"Próba wczytania zapisu ze starszej wersji. Kontynuacja spowoduje automatyczną aktualizację zapisu.",
	"Masz włączone inne mody, niż zapis gry. Mody mogą nie działać poprawnie.",
	"Kontynuować?",

	"Czy na pewno chcesz nadpisać zapisaną grę o nazwie \"%s\"?",

	"Zapisuję...",			//When saving a game, a message box with this string appears on the screen

	"Standardowe uzbrojenie",
	"Całe mnóstwo broni",
	"Realistyczna gra",
	"Elementy S-F",
	"Stopień trudności",

	"Brak włączonych modów",
	"Mody:"
};



//MapScreen
static const ST::string s_pl_zMarksMapScreenText[zMarksMapScreenText_SIZE] =
{
	"Poziom mapy",
	"Nie masz jeszcze żołnierzy samoobrony.  Musisz najpierw wytrenować mieszkańców miast.",
	"Dzienny przychód",
	"Najmemnik ma polisę ubezpieczeniową",
	"%s nie potrzebuje snu.",
	"%s jest w drodze i nie może spać",
	"%s jest zbyt zmęczony(na), spróbuj trochę później.",
	"%s prowadzi.",
	"Oddział nie może się poruszać jeżeli jeden z najemników śpi.",

	// stuff for contracts
	"Mimo, że możesz opłacić kontrakt, to jednak nie masz gotówki by opłacić składkę ubezpieczeniową za najemnika.",
	"%s - składka ubezpieczeniowa najemnika będzie kosztować %s za %d dzień(dni). Czy chcesz ją opłacić?",
	"Inwentarz sektora",
	"Najemnik posiada zastaw na życie.",

	// other items
	"Lekarze", // people acting a field medics and bandaging wounded mercs // **************************************NEW******** as of July 09, 1998
	"Pacjenci", // people who are being bandaged by a medic // ****************************************************NEW******** as of July 10, 1998
	"Gotowe", // Continue on with the game after autobandage is complete
	"Przerwij", // Stop autobandaging of patients by medics now
	"%s nie ma zestawu narzędzi.",
	"%s nie ma apteczki.",
	"Brak chętnych ludzi do szkolenia, w tej chwili.",
	"%s posiada już maksymalną liczbę oddziałów samoobrony.",
	"Najemnik ma kontrakt na określony czas.",
	"Kontrakt najemnika nie jest ubezpieczony",
};


static const ST::string s_pl_pLandMarkInSectorString = "Oddział %d zauważył kogoś w sektorze %s";

// confirm the player wants to pay X dollars to build a militia force in town
static const ST::string s_pl_pMilitiaConfirmStrings[pMilitiaConfirmStrings_SIZE] =
{
	"Szkolenie oddziału samoobrony będzie kosztowało $", // telling player how much it will cost
	"Zatwierdzasz wydatek?", // asking player if they wish to pay the amount requested
	"Nie stać cię na to.", // telling the player they can't afford to train this town
	"Kontynuować szkolenie samoobrony w - %s (%s %d)?", // continue training this town?
	"Koszt $", // the cost in dollars to train militia
	"( T/N )",   // abbreviated yes/no
	"Szkolenie samoobrony w %d sektorach będzie kosztowało $ %d. %s", // cost to train sveral sectors at once
	"Nie masz %d$, aby wyszkolić samoobronę w tym mieście.",
	"%s musi mieć %d%% lojalności, aby można było kontynuować szkolenie samoobrony.",
	"Nie możesz już dłużej szkolić samoobrony w mieście %s.",
};


//Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom of the single merc panel
static const ST::string s_pl_gzMoneyWithdrawMessageText[gzMoneyWithdrawMessageText_SIZE] =
{
	"Jednorazowo możesz wypłacić do 20,000$.",
	"Czy na pewno chcesz wpłacić %s na swoje konto?",
};

static const ST::string s_pl_gzCopyrightText = "Copyright (C) 1999 Sir-tech Canada Ltd.  All rights reserved.";

//option Text
static const ST::string s_pl_zOptionsToggleText[zOptionsToggleText_SIZE] =
{
	"Dialogi",
	"Wycisz potwierdzenia",
	"Napisy",
	"Wstrzymuj napisy",
	"Animowany dym",
	"Drastyczne sceny",
	"Nigdy nie ruszaj mojej myszki!",
	"Stara metoda wyboru",
	"Pokazuj trasę ruchu",
	"Pokazuj chybione strzały",
	"Potwierdzenia w trybie Real-Time",
	"Informacja, że najemnik śpi/budzi się",
	"Używaj systemu metrycznego",
	"Światło wokół najemników podczas ruchu",
	"Przyciągaj kursor do najemników",
	"Przyciągaj kursor do drzwi",
	"Pulsujące przedmioty",
	"Pokazuj korony drzew",
	"Pokazuj siatkę",
	"Pokazuj kursor 3D",
};

//This is the help text associated with the above toggles.
static const ST::string s_pl_zOptionsScreenHelpText[zOptionsToggleText_SIZE] =
{
	//speech
	"Włącz tę opcję, jeśli chcesz słuchać dialogów.",

	//Mute Confirmation
	"Włącza lub wyłącza głosowe potwierzenia postaci.",

		//Subtitles
	"Włącza lub wyłącza napisy podczas dialogów.",

	//Key to advance speech
	"Jeśli napisy są włączone, opcja ta pozwoli ci spokojnie je przeczytać podczas dialogu.",

	//Toggle smoke animation
	"Wyłącz tę opcję, aby poprawić płynność działania gry.",

	//Blood n Gore
	"Wyłącz tę opcję, jeśli nie lubisz widoku krwi.",

	//Never move my mouse
	"Wyłącz tę opcję, aby kursor myszki automatycznie ustawiał się nad pojawiającymi się okienkami dialogowymi.",

	//Old selection method
	"Włącz tę opcję, aby wybór postaci działał tak jak w poprzedniej wersji gry.",

	//Show movement path
	"Włącz tę opcję jeśli chcesz widzieć trasę ruchu w trybie Real-Time.",

	//show misses
	"Włącz tę opcję, aby zobaczyć w co trafiają twoje kule gdy pudłujesz.",

	//Real Time Confirmation
	"Gdy opcja ta jest włączona, każdy ruch najemnika w trybie Real-Time będzie wymagał dodatkowego, potwierdzającego kliknięcia.",

	//Sleep/Wake notification
	"Gdy opcja ta jest włączona, wyświetlana będzie informacja, że najemnik położył się spać lub wstał i wrócił do pracy.",

	//Use the metric system
	"Gdy opcja ta jest włączona, gra używa systemu metrycznego.",

	//Merc Lighted movement
	"Gdy opcja ta jest włączona, teren wokół najemnika będzie oświetlony podczas ruchu. Wyłącz tę opcję, jeśli obniża płynność gry.",

	//Smart cursor
	"Gdy opcja ta jest włączona, kursor automatycznie ustawia się na najemnikach gdy znajdzie się w ich pobliżu.",

	//snap cursor to the door
	"Gdy opcja ta jest włączona, kursor automatycznie ustawi się na drzwiach gdy znajdzie się w ich pobliżu.",

	//glow items
	"Gdy opcja ta jest włączona, przedmioty pulsują. ( |I )",

	//toggle tree tops
	"Gdy opcja ta jest włączona, wyświetlane są korony drzew. ( |T )",

	//toggle wireframe
	"Gdy opcja ta jest włączona, wyświetlane są zarysy niewidocznych ścian. ( |W )",

	"Gdy opcja ta jest włączona, kursor ruchu wyświetlany jest w 3D. (|H|o|m|e)",

};


static const ST::string s_pl_gzGIOScreenText[gzGIOScreenText_SIZE] =
{
	"POCZĄTKOWE USTAWIENIA GRY",
	"Styl gry",
	"Realistyczny",
	"S-F",
	"Opcje broni",
	"Mnóstwo broni",
	"Standardowe uzbrojenie",
	"Stopień trudności",
	"Nowicjusz",
	"Doświadczony",
	"Ekspert",
	"Ok",
	"Anuluj",
	"Dodatkowe opcje",
	"Nielimitowany czas",
	"Tury limitowane czasowo",
	"Dead is Dead"
};

// This string is used in the IMP character generation.  It is possible to
// select 0 ability in a skill meaning you can't use it.  This text is
// confirmation to the player.
static const ST::string s_pl_pSkillAtZeroWarning    = "Na pewno? Wartość zero oznacza brak jakichkolwiek umiejętności w tej dziedzinie.";
static const ST::string s_pl_pIMPBeginScreenStrings = "( Maks. 8 znaków )";
static const ST::string s_pl_pIMPFinishButtonText   = "Analizuję";
static const ST::string s_pl_pIMPFinishStrings      = "Dziękujemy, %s"; //%s is the name of the merc
static const ST::string s_pl_pIMPVoicesStrings      = "Głos"; // the strings for imp voices screen

// title for program
static const ST::string s_pl_pPersTitleText = "Personel";

// paused game strings
static const ST::string s_pl_pPausedGameText[pPausedGameText_SIZE] =
{
	"Gra wstrzymana",
	"Wznów grę (|P|a|u|s|e)",
	"Wstrzymaj grę (|P|a|u|s|e)",
};


static const ST::string s_pl_pMessageStrings[pMessageStrings_SIZE] =
{
	"Zakończyć grę?",
	"OK",
	"TAK",
	"NIE",
	"ANULUJ",
	"NAJMIJ",
	"LIE",
	"Brak opisu", //Save slots that don't have a description.
	"Gra zapisana.",
	"Dzień",
	"Najemn.",
	"Wolna pozycja", //An empty save game slot
	"strz/min",					//Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
	"min",					//Abbreviation for minute.
	"m",						//One character abbreviation for meter (metric distance measurement unit).
	"kul",				//Abbreviation for rounds (# of bullets)
	"kg",					//Abbreviation for kilogram (metric weight measurement unit)
	"lb",					//Abbreviation for pounds (Imperial weight measurement unit)
	"Strona główna",				//Home as in homepage on the internet.
	"USD",					//Abbreviation to US dollars
	"N/D",					//Lowercase acronym for not applicable.
	"Tymczasem",		//Meanwhile
	"%s przybył(a) do sektora %s%s", //Name/Squad has arrived in sector A9.  Order must not change without notifying
																		//SirTech
	"Wersja",
	"Utwórz nowy zapis gry",
	"Ta pozycja zarezerwowana jest na szybkie zapisy wykonywane podczas gry kombinacją klawiszy ALT+S.",
	"Otw.",
	"Zamkn.",
	"Brak miejsca na dysku twardym.  Na dysku wolne jest %s MB, a wymagane jest przynajmniej %s MB.",
	"%s złapał(a) %s",		//'Merc name' has caught 'item' -- let SirTech know if name comes after item.
	"%s zaaplikował(a) sobie lekarstwo", //'Merc name' has taken the drug
	"%s nie posiada wiedzy medycznej",//'Merc name' has no medical skill.

	//CDRom errors (such as ejecting CD while attempting to read the CD)
	"Integralność gry została narażona na szwank.",
	"BŁĄD: Wyjęto płytę CD",

	//When firing heavier weapons in close quarters, you may not have enough room to do so.
	"Nie ma miejsca, żeby stąd oddać strzał.",

	//Can't change stance due to objects in the way...
	"Nie można zmienić pozycji w tej chwili.",

	//Simple text indications that appear in the game, when the merc can do one of these things.
	"Upuść",
	"Rzuć",
	"Podaj",

	"%s przekazano do - %s.",	//"Item" passed to "merc".  Please try to keep the item %s before the merc %s,
					//otherwise, must notify SirTech.
	"Brak wolnego miejsca, by przekazać %s do - %s.", //pass "item" to "merc".  Same instructions as above.

	//A list of attachments appear after the items.  Ex:  Kevlar vest ( Ceramic Plate 'Attached )'
	" dołączono)",

	//Cheat modes
	"Pierwszy poziom lamerskich zagrywek osiągnięty",
	"Drugi poziom lamerskich zagrywek osiągnięty",

	//Toggling various stealth modes
	"Oddział ma włączony tryb skradania się.",
	"Oddział ma wyłączony tryb skradania się.",
	"%s ma włączony tryb skradania się.",
	"%s ma wyłączony tryb skradania się.",

	//Wireframes are shown through buildings to reveal doors and windows that can't otherwise be seen in
	//an isometric engine.  You can toggle this mode freely in the game.
	"Dodatkowe siatki włączone.",
	"Dodatkowe siatki wyłączone.",

	//These are used in the cheat modes for changing levels in the game.  Going from a basement level to
	//an upper level, etc.
	"Nie można wyjść do góry z tego poziomu...",
	"Nie ma już niższych poziomów...",
	"Wejście na %d poziom pod ziemią...",
	"Wyjście z podziemii...",

	" - ",		// used in the shop keeper inteface to mark the ownership of the item eg Red's gun
	"Automatyczne centrowanie ekranu wyłączone.",
	"Automatyczne centrowanie ekranu włączone.",
	"Kursor 3D wyłączony.",
	"Kursor 3D włączony.",
	"Oddział %d aktywny.",
	"%s - Nie stać cię by wypłacić jej/jemu dzienną pensję w wysokości %s.",	//first %s is the mercs name, the seconds is a string containing the salary
	"Pomiń",
	"%s nie może odejść sam(a).",
	"Utworzono zapis gry o nazwie SaveGame99.sav. W razie potrzeby zmień jego nazwę na SaveGame01..10. Wtedy będzie można go odczytać ze standardowego okna odczytu gry.",
	"%s wypił(a) trochę - %s",
	"Przesyłka dotarła do Drassen.",
	"%s przybędzie do wyznaczonego punktu zrzutu (sektor %s) w dniu %d, około godziny %s.",		//first %s is mercs name, next is the sector location and name where they will be arriving in, lastely is the day an the time of arrival
	"Lista historii zaktualizowana.",
};


static const ST::string s_pl_ItemPickupHelpPopup[ItemPickupHelpPopup_SIZE] =
{
	"OK",
	"W górę",
	"Wybierz wszystko",
	"W dół",
	"Anuluj",
};

static const ST::string s_pl_pDoctorWarningString[pDoctorWarningString_SIZE] =
{
	"%s jest za daleko, aby poddać się leczeniu.",
	"Lekarze nie mogli opatrzyć wszystkich rannych.",
};

static const ST::string s_pl_pMilitiaButtonsHelpText[pMilitiaButtonsHelpText_SIZE] =
{
	"Podnieś(Prawy klawisz myszy)/upuść(Lewy klawisz myszy) Zielonych żołnierzy", // button help text informing player they can pick up or drop militia with this button
	"Podnieś(Prawy klawisz myszy)/upuść(Lewy klawisz myszy) Doświadczonych żołnierzy",
	"Podnieś(Prawy klawisz myszy)/upuść(Lewy klawisz myszy) Weteranów",
	"Umieszcza jednakową ilość żołnierzy samoobrony w każdym sektorze.",
};

// to inform the player to hire some mercs to get things going
static const ST::string s_pl_pMapScreenJustStartedHelpText = "Zajrzyj do A.I.M. i zatrudnij kilku najemników (*Wskazówka* musisz otworzyć laptopa)";

static const ST::string s_pl_pAntiHackerString = "Błąd. Brakuje pliku, lub jest on uszkodzony. Gra zostanie przerwana.";


static const ST::string s_pl_gzLaptopHelpText[gzLaptopHelpText_SIZE] =
{
	//Buttons:
	"Przeglądanie poczty",
	"Przeglądanie stron internetowych",
	"Przeglądanie plików i załączników pocztowych",
	"Rejestr zdarzeń",
	"Informacje o członkach oddziału",
	"Finanse i rejestr transakcji",
	"Koniec pracy z laptopem",

	//Bottom task bar icons (if they exist):
	"Masz nową pocztę",
	"Masz nowe pliki",

	//Bookmarks:
	"Międzynarodowe Stowarzyszenie Najemników",
	"Bobby Ray's - Internetowy sklep z bronią",
	"Instytut Badań Najemników",
	"Bardziej Ekonomiczne Centrum Rekrutacyjne",
	"McGillicutty's - Zakład pogrzebowy",
	"United Floral Service",
	"Brokerzy ubezpieczeniowi",
};


static const ST::string s_pl_gzHelpScreenText = "Zamknij okno pomocy";

static const ST::string s_pl_gzNonPersistantPBIText[gzNonPersistantPBIText_SIZE] =
{
	"Trwa walka. Najemników można wycofać tylko na ekranie taktycznym.",
	"W|ejdź do sektora, aby kontynuować walkę.",
	"|Automatycznie rozstrzyga walkę.",
	"Nie można automatycznie rozstrzygnąć walki, gdy atakujesz.",
	"Nie można automatycznie rozstrzygnąć walki, gdy wpadasz w pułapkę.",
	"Nie można automatycznie rozstrzygnąć walki, gdy walczysz ze stworzeniami w kopalni.",
	"Nie można automatycznie rozstrzygnąć walki, gdy w sektorze są wrodzy cywile.",
	"Nie można automatycznie rozstrzygnąć walki, gdy w sektorze są dzikie koty.",
	"TRWA WALKA",
	"W tym momencie nie możesz się wycofać.",
};

static const ST::string s_pl_gzMiscString[gzMiscString_SIZE] =
{
	"Żołnierze samoobrony kontynuują walkę bez pomocy twoich najemników...",
	"W tym momencie tankowanie nie jest konieczne.",
	"W baku jest %d%% paliwa.",
	"Żołnierze Deidranny przejęli całkowitą kontrolę nad - %s.",
	"Nie masz już gdzie zatankować.",
};

static const ST::string s_pl_gzIntroScreen = "Nie odnaleziono filmu wprowadzającego";

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
static const ST::string s_pl_pNewNoiseStr[pNewNoiseStr_SIZE] =
{
	"%s słyszy %s DŹWIĘK dochodzący z %s.",
	"%s słyszy %s ODGŁOS RUCHU dochodzący z %s.",
	"%s słyszy %s ODGŁOS SKRZYPNIĘCIA dochodzący z %s.",
	"%s słyszy %s PLUSK dochodzący z %s.",
	"%s słyszy %s ODGŁOS UDERZENIA dochodzący z %s.",
	"%s słyszy %s WYBUCH dochodzący z %s.",
	"%s słyszy %s KRZYK dochodzący z %s.",
	"%s słyszy %s ODGŁOS UDERZENIA dochodzący z %s.",
	"%s słyszy %s ODGŁOS UDERZENIA dochodzący z %s.",
	"%s słyszy %s ŁOMOT dochodzący z %s.",
	"%s słyszy %s TRZASK dochodzący z %s.",
};

static const ST::string s_pl_wMapScreenSortButtonHelpText[wMapScreenSortButtonHelpText_SIZE] =
{
	"Sortuj według kolumny Imię (|F|1)",
	"Sortuj według kolumny Przydział (|F|2)",
	"Sortuj według kolumny Sen (|F|3)",
	"Sortuj według kolumny Lokalizacja (|F|4)",
	"Sortuj według kolumny Cel podróży (|F|5)",
	"Sortuj według kolumny Wyjazd (|F|6)",
};



static const ST::string s_pl_BrokenLinkText[BrokenLinkText_SIZE] =
{
	"Błąd 404",
	"Nie odnaleziono strony.",
};


static const ST::string s_pl_gzBobbyRShipmentText[gzBobbyRShipmentText_SIZE] =
{
	"Ostatnie dostawy",
	"Zamówienie nr ",
	"Ilość przedmiotów",
	"Zamówiono:",
};


static const ST::string s_pl_gzCreditNames[gzCreditNames_SIZE]=
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


static const ST::string s_pl_gzCreditNameTitle[gzCreditNameTitle_SIZE]=
{
	"Game Internals Programmer", 			// Chris Camfield
	"Co-designer/Writer",							// Shaun Lyng
	"Strategic Systems & Editor Programmer",					//Kris Marnes
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

static const ST::string s_pl_gzCreditNameFunny[gzCreditNameFunny_SIZE]=
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

static const ST::string s_pl_sRepairsDoneString[sRepairsDoneString_SIZE] =
{
	"%s skończył(a) naprawiać własne wyposażenie",
	"%s skończył(a) naprawiać broń i ochraniacze wszystkich członków oddziału",
	"%s skończył(a) naprawiać wyposażenie wszystkich członków oddziału",
	"%s skończył(a) naprawiać ekwipunek wszystkich członków oddziału",
};


static const ST::string s_pl_zGioDifConfirmText[zGioDifConfirmText_SIZE]=
{
	"Wybrano opcję Nowicjusz. Opcja ta jest przeznaczona dla niedoświadczonych graczy, lub dla tych, którzy nie mają ochoty na długie i ciężkie walki. Pamiętaj, że opcja ta ma wpływ na przebieg całej gry. Czy na pewno chcesz grać w trybie Nowicjusz?",
	"Wybrano opcję Doświadczony. Opcja ta jest przenaczona dla graczy posiadających już pewne doświadczenie w grach tego typu. Pamiętaj, że opcja ta ma wpływ na przebieg całej gry. Czy na pewno chcesz grać w trybie Doświadczony?",
	"Wybrano opcję Ekspert. Jakby co, to ostrzegaliśmy cię. Nie obwiniaj nas, jeśli wrócisz w plastikowym worku. Pamiętaj, że opcja ta ma wpływ na przebieg całej gry. Czy na pewno chcesz grać w trybie Ekspert?",
};


static const ST::string s_pl_gzLateLocalizedString[gzLateLocalizedString_SIZE] =
{
	//1-5
	"Robot nie może opuścić sektora bez operatora.",

	//This message comes up if you have pending bombs waiting to explode in tactical.
	"Nie można teraz kompresować czasu.  Poczekaj na fajerwerki!",

	//'Name' refuses to move.
	"%s nie chce się przesunąć.",

	//%s a merc name
	"%s ma zbyt mało energii, aby zmienić pozycję.",

	//A message that pops up when a vehicle runs out of gas.
	"{} nie ma paliwa i stoi w sektorze {}.",

	//6-10

	// the following two strings are combined with the pNewNoise[] strings above to report noises
	// heard above or below the merc
	"GÓRY",
	"DOŁU",

	//The following strings are used in autoresolve for autobandaging related feedback.
	"Żaden z twoich najemników nie posiada wiedzy medycznej.",
	"Brak środków medycznych, aby założyć rannym opatrunki.",
	"Zabrakło środków medycznych, aby założyć wszystkim rannym opatrunki.",
	"Żaden z twoich najemników nie potrzebuje pomocy medycznej.",
	"Automatyczne zakładanie opatrunków rannym najemnikom.",
	"Wszystkim twoim najemnikom założono opatrunki.",

	//14
	"Arulco",

	"(dach)",

	"Zdrowie: %d/%d",

	//In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
	//"vs." is the abbreviation of versus.
	"%d vs. %d",

	"%s - brak wolnych miejsc!",  //(ex "The ice cream truck is full")

	"%s nie potrzebuje pierwszej pomocy lecz opieki lekarza lub dłuższego odpoczynku.",

	//20
	//Happens when you get shot in the legs, and you fall down.
	"%s dostał(a) w nogi i upadł(a)!",
	//Name can't speak right now.
	"%s nie może teraz mówić.",

	//22-24 plural versions
	"%d zielonych żołnierzy samoobrony awansowało na weteranów.",
	"%d zielonych żołnierzy samoobrony awansowało na regularnych żołnierzy.",
	"%d regularnych żołnierzy samoobrony awansowało na weteranów.",

	//25
	"Przełącznik",

	//26
	//Name has gone psycho -- when the game forces the player into burstmode (certain unstable characters)
	"%s dostaje świra!",

	//27-28
	//Messages why a player can't time compress.
	"Niebezpiecznie jest kompresować teraz czas, gdyż masz najemników w sektorze %s.",
	"Niebezpiecznie jest kompresować teraz czas, gdyż masz najemników w kopalni zaatakowanej przez robale.",

	//29-31 singular versions
	"1 zielony żołnierz samoobrony awansował na weterana.",
	"1 zielony żołnierz samoobrony awansował na regularnego żołnierza.",
	"1 regularny żołnierz samoobrony awansował na weterana.",

	//32-34
	"%s nic nie mówi.",
	"Wyjść na powierzchnię?",
	"(Oddział %d)",

	//35
	//Ex: "Red has repaired Scope's MP5K".  Careful to maintain the proper order (Red before Scope, Scope before MP5K)
	"%s naprawił(a) najemnikowi - %s, jego/jej - %s",

	//36
	"DZIKI KOT",

	//37-38 "Name trips and falls"
	"%s potyka się i upada",
	"Nie można stąd podnieść tego przedmiotu.",

	//39
	"Żaden z twoich najemników nie jest w stanie walczyć.  Żołnierze samoobrony sami będą walczyć z robalami.",

	//40-43
	//%s is the name of merc.
	"%s nie ma środków medycznych!",
	"%s nie posiada odpowiedniej wiedzy, aby kogokolwiek wyleczyć!",
	"%s nie ma narzędzi!",
	"%s nie posiada odpowiedniej wiedzy, aby cokolwiek naprawić!",

	//44-45
	"Czas naprawy",
	"%s nie widzi tej osoby.",

	//46-48
	"%s - przedłużka lufy jego/jej broni odpada!",
	"W jednym sektorze, szkolenie samoobrony może prowadzić tylko %d instruktor(ów).",
	"Na pewno?",

	//49-50
	"Kompresja czasu",
	"Pojazd ma pełny zbiornik paliwa.",

	//51-52 Fast help text in mapscreen.
	"Kontynuuj kompresję czasu (|S|p|a|c|j|a)",
	"Zatrzymaj kompresję czasu (|E|s|c)",

	//53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
	"%s odblokował(a) - %s",
	"%s odblokował(a) najemnikowi - %s, jego/jej - %s",

	//55
	"Nie można kompresować czasu, gdy otwarty jest inwentarz sektora.",

	//56
	//Displayed with the version information when cheats are enabled.
	"Bieżący/Maks. postęp: %d%%/%d%%",

	//57
	"Eskortować Johna i Mary?",

	"Przełącznik aktywowany.",
};

static const ST::string s_pl_str_ceramic_plates_smashed = "%s's ceramic plates have been smashed!"; // TODO translate

static const ST::string s_pl_str_arrival_rerouted = "Arrival of new recruits is being rerouted to sector %s, as scheduled drop-off point of sector %s is enemy occupied."; // TODO translate


static const ST::string s_pl_str_stat_health       = "Zdrowie";
static const ST::string s_pl_str_stat_agility      = "Zwinność";
static const ST::string s_pl_str_stat_dexterity    = "Sprawność";
static const ST::string s_pl_str_stat_strength     = "Siła";
static const ST::string s_pl_str_stat_leadership   = "Um. dowodzenia";  // Umiejętność dowodzenia
static const ST::string s_pl_str_stat_wisdom       = "Inteligencja";
static const ST::string s_pl_str_stat_exp_level    = "Poziom dośw.";    // Poziom doświadczenia
static const ST::string s_pl_str_stat_marksmanship = "Um. strzeleckie"; // Umiejętności strzeleckie
static const ST::string s_pl_str_stat_mechanical   = "Zn. mechaniki";   // Znajomość mechaniki
static const ST::string s_pl_str_stat_explosive    = "Zn. mat. wyb.";   // Znajomość materiałów wybuchowych
static const ST::string s_pl_str_stat_medical      = "Wiedza medyczna";

static const ST::string s_pl_str_stat_list[str_stat_list_SIZE] =
{
	s_pl_str_stat_health,
	s_pl_str_stat_agility,
	s_pl_str_stat_dexterity,
	s_pl_str_stat_strength,
	s_pl_str_stat_leadership,
	s_pl_str_stat_wisdom,
	s_pl_str_stat_exp_level,
	s_pl_str_stat_marksmanship,
	s_pl_str_stat_mechanical,
	s_pl_str_stat_explosive,
	s_pl_str_stat_medical
};

static const ST::string s_pl_str_aim_sort_list[str_aim_sort_list_SIZE] =
{
	s_pl_str_aim_sort_price,
	s_pl_str_aim_sort_experience,
	s_pl_str_aim_sort_marksmanship,
	s_pl_str_aim_sort_medical,
	s_pl_str_aim_sort_explosives,
	s_pl_str_aim_sort_mechanical,
	s_pl_str_aim_sort_ascending,
	s_pl_str_aim_sort_descending,
};

static const ST::string s_pl_gs_dead_is_dead_mode_tab_name[gs_dead_is_dead_mode_tab_name_SIZE] =
{
	"Normal", 			// Normal Tab
	"DiD", 			// Dead is Dead Tab
};

// Polish language resources.
LanguageRes g_LanguageResPolish = {

	s_pl_WeaponType,

	s_pl_Message,
	s_pl_TeamTurnString,
	s_pl_pAssignMenuStrings,
	s_pl_pTrainingStrings,
	s_pl_pTrainingMenuStrings,
	s_pl_pAttributeMenuStrings,
	s_pl_pVehicleStrings,
	s_pl_pShortAttributeStrings,
	s_pl_pContractStrings,
	s_pl_pAssignmentStrings,
	s_pl_pConditionStrings,
	s_pl_pPersonnelScreenStrings,
	s_pl_pUpperLeftMapScreenStrings,
	s_pl_pTacticalPopupButtonStrings,
	s_pl_pSquadMenuStrings,
	s_pl_pDoorTrapStrings,
	s_pl_pLongAssignmentStrings,
	s_pl_pMapScreenMouseRegionHelpText,
	s_pl_pNoiseVolStr,
	s_pl_pNoiseTypeStr,
	s_pl_pDirectionStr,
	s_pl_pRemoveMercStrings,
	s_pl_sTimeStrings,
	s_pl_pInvPanelTitleStrings,
	s_pl_pPOWStrings,
	s_pl_pMilitiaString,
	s_pl_pMilitiaButtonString,
	s_pl_pEpcMenuStrings,
	s_pl_pRepairStrings,
	s_pl_sPreStatBuildString,
	s_pl_sStatGainStrings,
	s_pl_pHelicopterEtaStrings,
	s_pl_sMapLevelString,
	s_pl_gsLoyalString,
	s_pl_gsUndergroundString,
	s_pl_gsTimeStrings,
	s_pl_sFacilitiesStrings,
	s_pl_pMapPopUpInventoryText,
	s_pl_pwTownInfoStrings,
	s_pl_pwMineStrings,
	s_pl_pwMiscSectorStrings,
	s_pl_pMapInventoryErrorString,
	s_pl_pMapInventoryStrings,
	s_pl_pMovementMenuStrings,
	s_pl_pUpdateMercStrings,
	s_pl_pMapScreenBorderButtonHelpText,
	s_pl_pMapScreenBottomFastHelp,
	s_pl_pMapScreenBottomText,
	s_pl_pMercDeadString,
	s_pl_pSenderNameList,
	s_pl_pNewMailStrings,
	s_pl_pDeleteMailStrings,
	s_pl_pEmailHeaders,
	s_pl_pEmailTitleText,
	s_pl_pFinanceTitle,
	s_pl_pFinanceSummary,
	s_pl_pFinanceHeaders,
	s_pl_pTransactionText,
	s_pl_pMoralStrings,
	s_pl_pSkyriderText,
	s_pl_str_left_equipment,
	s_pl_pMapScreenStatusStrings,
	s_pl_pMapScreenPrevNextCharButtonHelpText,
	s_pl_pEtaString,
	s_pl_pShortVehicleStrings,
	s_pl_pTrashItemText,
	s_pl_pMapErrorString,
	s_pl_pMapPlotStrings,
	s_pl_pBullseyeStrings,
	s_pl_pMiscMapScreenMouseRegionHelpText,
	s_pl_str_he_leaves_where_drop_equipment,
	s_pl_str_she_leaves_where_drop_equipment,
	s_pl_str_he_leaves_drops_equipment,
	s_pl_str_she_leaves_drops_equipment,
	s_pl_pImpPopUpStrings,
	s_pl_pImpButtonText,
	s_pl_pExtraIMPStrings,
	s_pl_pFilesTitle,
	s_pl_pFilesSenderList,
	s_pl_pHistoryLocations,
	s_pl_pHistoryStrings,
	s_pl_pHistoryHeaders,
	s_pl_pHistoryTitle,
	s_pl_pShowBookmarkString,
	s_pl_pWebPagesTitles,
	s_pl_pWebTitle,
	s_pl_pPersonnelString,
	s_pl_pErrorStrings,
	s_pl_pDownloadString,
	s_pl_pBookMarkStrings,
	s_pl_pLaptopIcons,
	s_pl_gsAtmStartButtonText,
	s_pl_pPersonnelTeamStatsStrings,
	s_pl_pPersonnelCurrentTeamStatsStrings,
	s_pl_pPersonelTeamStrings,
	s_pl_pPersonnelDepartedStateStrings,
	s_pl_pMapHortIndex,
	s_pl_pMapVertIndex,
	s_pl_pMapDepthIndex,
	s_pl_pLaptopTitles,
	s_pl_pDayStrings,
	s_pl_pMilitiaConfirmStrings,
	s_pl_pSkillAtZeroWarning,
	s_pl_pIMPBeginScreenStrings,
	s_pl_pIMPFinishButtonText,
	s_pl_pIMPFinishStrings,
	s_pl_pIMPVoicesStrings,
	s_pl_pPersTitleText,
	s_pl_pPausedGameText,
	s_pl_zOptionsToggleText,
	s_pl_zOptionsScreenHelpText,
	s_pl_pDoctorWarningString,
	s_pl_pMilitiaButtonsHelpText,
	s_pl_pMapScreenJustStartedHelpText,
	s_pl_pLandMarkInSectorString,
	s_pl_gzMercSkillText,
	s_pl_gzNonPersistantPBIText,
	s_pl_gzMiscString,
	s_pl_wMapScreenSortButtonHelpText,
	s_pl_pNewNoiseStr,
	s_pl_gzLateLocalizedString,
	s_pl_pAntiHackerString,
	s_pl_pMessageStrings,
	s_pl_ItemPickupHelpPopup,
	s_pl_TacticalStr,
	s_pl_LargeTacticalStr,
	s_pl_zDialogActions,
	s_pl_zDealerStrings,
	s_pl_zTalkMenuStrings,
	s_pl_gzMoneyAmounts,
	s_pl_gzProsLabel,
	s_pl_gzConsLabel,
	s_pl_gMoneyStatsDesc,
	s_pl_gWeaponStatsDesc,
	s_pl_sKeyDescriptionStrings,
	s_pl_zHealthStr,
	s_pl_zVehicleName,
	s_pl_pExitingSectorHelpText,
	s_pl_InsContractText,
	s_pl_InsInfoText,
	s_pl_MercAccountText,
	s_pl_MercInfo,
	s_pl_MercNoAccountText,
	s_pl_MercHomePageText,
	s_pl_sFuneralString,
	s_pl_sFloristText,
	s_pl_sOrderFormText,
	s_pl_sFloristGalleryText,
	s_pl_sFloristCards,
	s_pl_BobbyROrderFormText,
	s_pl_BobbyRText,
	s_pl_str_bobbyr_guns_num_guns_that_use_ammo,
	s_pl_BobbyRaysFrontText,
	s_pl_AimSortText,
	s_pl_str_aim_sort_price,
	s_pl_str_aim_sort_experience,
	s_pl_str_aim_sort_marksmanship,
	s_pl_str_aim_sort_medical,
	s_pl_str_aim_sort_explosives,
	s_pl_str_aim_sort_mechanical,
	s_pl_str_aim_sort_ascending,
	s_pl_str_aim_sort_descending,
	s_pl_AimPolicyText,
	s_pl_AimMemberText,
	s_pl_CharacterInfo,
	s_pl_VideoConfercingText,
	s_pl_AimPopUpText,
	s_pl_AimLinkText,
	s_pl_AimHistoryText,
	s_pl_AimFiText,
	s_pl_AimAlumniText,
	s_pl_AimScreenText,
	s_pl_AimBottomMenuText,
	s_pl_zMarksMapScreenText,
	s_pl_gpStrategicString,
	s_pl_gpGameClockString,
	s_pl_SKI_Text,
	s_pl_SkiMessageBoxText,
	s_pl_zSaveLoadText,
	s_pl_zOptionsText,
	s_pl_gzGIOScreenText,
	s_pl_gzHelpScreenText,
	s_pl_gzLaptopHelpText,
	s_pl_gzMoneyWithdrawMessageText,
	s_pl_gzCopyrightText,
	s_pl_BrokenLinkText,
	s_pl_gzBobbyRShipmentText,
	s_pl_zGioDifConfirmText,
	s_pl_gzCreditNames,
	s_pl_gzCreditNameTitle,
	s_pl_gzCreditNameFunny,
	s_pl_pContractButtonString,
	s_pl_gzIntroScreen,
	s_pl_pUpdatePanelButtons,
	s_pl_sRepairsDoneString,
	s_pl_str_ceramic_plates_smashed,
	s_pl_str_arrival_rerouted,

	s_pl_str_stat_health,
	s_pl_str_stat_agility,
	s_pl_str_stat_dexterity,
	s_pl_str_stat_strength,
	s_pl_str_stat_leadership,
	s_pl_str_stat_wisdom,
	s_pl_str_stat_exp_level,
	s_pl_str_stat_marksmanship,
	s_pl_str_stat_mechanical,
	s_pl_str_stat_explosive,
	s_pl_str_stat_medical,

	s_pl_str_stat_list,
	s_pl_str_aim_sort_list,

	g_eng_zNewTacticalMessages,
	g_eng_str_iron_man_mode_warning,
	g_eng_str_dead_is_dead_mode_warning,
	g_eng_str_dead_is_dead_mode_enter_name,

	s_pl_gs_dead_is_dead_mode_tab_name,

	s_pl_gzIMPSkillTraitsText,
};

#ifdef WITH_UNITTESTS
TEST(StringEncodingTest, PolishTextFile)
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
