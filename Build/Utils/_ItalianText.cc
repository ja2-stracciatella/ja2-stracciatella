#ifdef ITALIAN

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

// Different weapon calibres
// CAWS is Close Assault Weapon System and should probably be left as it is
// NATO is the North Atlantic Treaty Organization
// WP is Warsaw Pact
// cal is an abbreviation for calibre
const wchar_t AmmoCaliber[][20] =
{
	L"0",
	L"cal .38",
	L"9 mm",
	L"cal .45",
	L"cal .357",
	L"cal fisso 12",
	L"CAW",
	L"5.45 mm",
	L"5.56 mm",
	L"7.62 mm NATO",
	L"7.62 mm WP",
	L"4.7 mm",
	L"5.7 mm",
	L"Mostro",
	L"Missile",
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
	L"cal .38",
	L"9 mm",
	L"cal .45",
	L"cal .357",
	L"cal fisso 12",
	L"CAWS",
	L"5.45 mm",
	L"5.56 mm",
	L"7.62 mm N.",
	L"7.62 mm WP",
	L"4.7 mm",
	L"5.7 mm",
	L"Mostro",
	L"Missile",
	L"", // dart
};


const wchar_t WeaponType[][30] =
{
	L"Altro",
	L"Arma",
	L"Mitragliatrice",
	L"Mitra",
	L"Fucile",
	L"Fucile del cecchino",
	L"Fucile d'assalto",
	L"Mitragliatrice leggera",
	L"Fucile a canne mozze",
};

const wchar_t* const TeamTurnString[] =
{
	L"Turno del giocatore", // player's turn
	L"Turno degli avversari",
	L"Turno delle creature",
	L"Turno dell'esercito",
	L"Turno dei civili",
	// planning turn
};

const wchar_t* const Message[] =
{
	// In the following 8 strings, the %ls is the merc's name, and the %d (if any) is a number.

	L"%ls è stato colpito alla testa e perde un punto di saggezza!",
	L"%ls è stato colpito alla spalla e perde un punto di destrezza!",
	L"%ls è stato colpito al torace e perde un punto di forza!",
	L"%ls è stato colpito alle gambe e perde un punto di agilità!",
	L"%ls è stato colpito alla testa e perde %d punti di saggezza!",
	L"%ls è stato colpito alle palle perde %d punti di destrezza!",
	L"%ls è stato colpito al torace e perde %d punti di forza!",
	L"%ls è stato colpito alle gambe e perde %d punti di agilità!",
	L"Interrompete!",

	L"I vostri rinforzi sono arrivati!",

	// In the following four lines, all %ls's are merc names

	L"%ls ricarica.",
	L"%ls non ha abbastanza Punti Azione!",
	// the following 17 strings are used to create lists of gun advantages and disadvantages
	// (separated by commas)
	L"affidabile",
	L"non affidabile",
	L"facile da riparare",
	L"difficile da riparare",
	L"danno grave",
	L"danno lieve",
	L"fuoco veloce",
	L"fuoco",
	L"raggio lungo",
	L"raggio corto",
	L"leggero",
	L"pesante",
	L"piccolo",
	L"fuoco a raffica",
	L"niente raffiche",
	L"grande deposito d'armi",
	L"piccolo deposito d'armi",

	// In the following two lines, all %ls's are merc names

	L"Il travestimento di %ls è stato scoperto.",
	L"Il travestimento di %ls è stato scoperto.",

	// The first %ls is a merc name and the second %ls is an item name

	L"La seconda arma è priva di munizioni!",
	L"%ls ha rubato il %ls.",

	// The %ls is a merc name

	L"L'arma di %ls non può più sparare a raffica.",

	L"Ne avete appena ricevuto uno di quelli attaccati.",
	L"Volete combinare gli oggetti?",

	// Both %ls's are item names

	L"Non potete attaccare %ls a un %ls.",

	L"Nessuno",
	L"Espelli munizioni",
	L"Attaccare",

	//You cannot use "item(s)" and your "other item" at the same time.
	//Ex:  You cannot use sun goggles and you gas mask at the same time.
	L"Non potete usare %ls e il vostro %ls contemporaneamente.",

	L"L'oggetto puntato dal vostro cursore può essere combinato ad alcuni oggetti ponendolo in uno dei quattro slot predisposti.",
	L"L'oggetto puntato dal vostro cursore può essere combinato ad alcuni oggetti ponendolo in uno dei quattro slot predisposti. (Comunque, in questo caso, l'oggetto non è compatibile.)",
	L"Il settore non è libero da nemici!",
	L"Vi dovete ancora dare %ls %ls",
	L"%ls è stato colpito alla testa!",
	L"Abbandonate la battaglia?",
	L"Questo attaco sarà definitivo. Andate avanti?",
	L"%ls si sente molto rinvigorito!",
	L"%ls ha dormito di sasso!",
	L"%ls non è riuscito a catturare il %ls!",
	L"%ls ha riparato il %ls",
	L"Interrompete per ",
	L"Vi arrendete?",
	L"Questa persona rifiuta il vostro aiuto.",
	L"NON sono d'accordo!",
    L"Per viaggiare sull'elicottero di Skyrider, dovrete innanzitutto ASSEGNARE mercenari al VEICOLO/ELICOTTERO.",
	L"solo %ls aveva abbastanza tempo per ricaricare UNA pistola",
	L"Turno dei Bloodcat",
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

const wchar_t* sTimeStrings[] =
{
	L"Fermo",
	L"Normale",
	L"5 min",
	L"30 min",
	L"60 min",
	L"6 ore",
};


// Assignment Strings: what assignment does the merc  have right now? For example, are they on a squad, training,
// administering medical aid (doctor) or training a town. All are abbreviated. 8 letters is the longest it can be.

const wchar_t* pAssignmentStrings[] =
{
	L"Squad. 1",
	L"Squad. 2",
	L"Squad. 3",
	L"Squad. 4",
	L"Squad. 5",
	L"Squad. 6",
	L"Squad. 7",
	L"Squad. 8",
	L"Squad. 9",
	L"Squad. 10",
	L"Squad. 11",
	L"Squad. 12",
	L"Squad. 13",
	L"Squad. 14",
	L"Squad. 15",
	L"Squad. 16",
	L"Squad. 17",
	L"Squad. 18",
	L"Squad. 19",
	L"Squad. 20",
	L"Servizio", // on active duty
	L"Dottore", // administering medical aid
	L"Paziente", // getting medical aid
	L"Veicolo", // in a vehicle
	L"Transito", // in transit - abbreviated form
	L"Riparare", // repairing
	L"Esercit.", // training themselves
  L"Esercit.", // training a town to revolt
	L"Istrutt.", // training a teammate
	L"Studente", // being trained by someone else
	L"Morto", // dead
	L"Incap.", // abbreviation for incapacitated
	L"PDG", // Prisoner of war - captured
	L"Ospedale", // patient in a hospital
	L"Vuoto",	// Vehicle is empty
};


const wchar_t* pMilitiaString[] =
{
	L"Esercito", // the title of the militia box
	L"Non incaricato", //the number of unassigned militia troops
	L"Non potete ridistribuire reclute, se ci sono nemici nei paraggi!",
};


const wchar_t* pMilitiaButtonString[] =
{
	L"Auto", // auto place the militia troops for the player
	L"Eseguito", // done placing militia troops
};

const wchar_t* pConditionStrings[] =
{
	L"Eccellente", //the state of a soldier .. excellent health
	L"Buono", // good health
	L"Discreto", // fair health
	L"Ferito", // wounded health
	L"Stanco", // tired
	L"Grave", // bleeding to death
	L"Svenuto", // knocked out
	L"Morente", // near death
	L"Morto", // dead
};

const wchar_t* pEpcMenuStrings[] =
{
	L"In servizio", // set merc on active duty
	L"Paziente", // set as a patient to receive medical aid
	L"Veicolo", // tell merc to enter vehicle
	L"Non scortato", // let the escorted character go off on their own
	L"Cancella", // close this menu
};


// look at pAssignmentString above for comments

const wchar_t* pLongAssignmentStrings[] =
{
	L"Squadra 1",
	L"Squadra 2",
	L"Squadra 3",
	L"Squadra 4",
	L"Squadra 5",
	L"Squadra 6",
	L"Squadra 7",
	L"Squadra 8",
	L"Squadra 9",
	L"Squadra 10",
	L"Squadra 11",
	L"Squadra 12",
	L"Squadra 13",
	L"Squadra 14",
	L"Squadra 15",
	L"Squadra 16",
	L"Squadra 17",
	L"Squadra 18",
	L"Squadra 19",
	L"Squadra 20",
	L"In servizio",
	L"Dottore",
	L"Paziente",
	L"Veicolo",
	L"In transito",
	L"Riparare",
	L"Esercitarsi",
  L"Allenatore esercito",
	L"Allena squadra",
	L"Studente",
	L"Morto",
	L"Incap.",
	L"PDG",
	L"Ospedale", // patient in a hospital
	L"Vuoto",	// Vehicle is empty
};


// the contract options

const wchar_t* pContractStrings[] =
{
	L"Opzioni del contratto:",
	L"", // a blank line, required
	L"Offri 1 giorno", // offer merc a one day contract extension
	L"Offri 1 settimana", // 1 week
	L"Offri 2 settimane", // 2 week
	L"Termina contratto", // end merc's contract
	L"Annulla", // stop showing this menu
};

const wchar_t* pPOWStrings[] =
{
	L"PDG",  //an acronym for Prisoner of War
	L"??",
};

const wchar_t* pInvPanelTitleStrings[] =
{
	L"Giubb. A-P", // the armor rating of the merc
	L"Peso", // the weight the merc is carrying
	L"Trav.", // the merc's camouflage rating
};

const wchar_t* pShortAttributeStrings[] =
{
	L"Abi", // the abbreviated version of : agility
	L"Des", // dexterity
	L"For", // strength
	L"Com", // leadership
	L"Sag", // wisdom
	L"Liv", // experience level
	L"Tir", // marksmanship skill
	L"Esp", // explosive skill
	L"Mec", // mechanical skill
	L"PS", // medical skill
};


const wchar_t* pUpperLeftMapScreenStrings[] =
{
	L"Compito", // the mercs current assignment
	L"Salute", // the health level of the current merc
	L"Morale", // the morale of the current merc
	L"Cond.",	// the condition of the current vehicle
};

const wchar_t* pTrainingStrings[] =
{
	L"Esercitarsi", // tell merc to train self
  L"Esercito", // tell merc to train town
	L"Allenatore", // tell merc to act as trainer
	L"Studente", // tell merc to be train by other
};

const wchar_t* pAssignMenuStrings[] =
{
	L"In servizio", // merc is on active duty
	L"Dottore", // the merc is acting as a doctor
	L"Paziente", // the merc is receiving medical attention
	L"Veicolo", // the merc is in a vehicle
	L"Ripara", // the merc is repairing items
	L"Si esercita", // the merc is training
	L"Annulla", // cancel this menu
};

const wchar_t* pRemoveMercStrings[] =
{
	L"Rimuovi Mercenario", // remove dead merc from current team
	L"Annulla",
};

const wchar_t* pAttributeMenuStrings[] =
{
	L"Forza",
	L"Destrezza",
	L"Agilità",
	L"Salute",
	L"Mira",
	L"Pronto socc.",
	L"Meccanica",
	L"Comando",
	L"Esplosivi",
	L"Annulla",
};

const wchar_t* pTrainingMenuStrings[] =
{
 L"Allenati", // train yourself
 L"Esercito", // train the town
 L"Allenatore", // train your teammates
 L"Studente",  // be trained by an instructor
 L"Annulla", // cancel this menu
};


const wchar_t* pSquadMenuStrings[] =
{
	L"Squadra  1",
	L"Squadra  2",
	L"Squadra  3",
	L"Squadra  4",
	L"Squadra  5",
	L"Squadra  6",
	L"Squadra  7",
	L"Squadra  8",
	L"Squadra  9",
	L"Squadra 10",
	L"Squadra 11",
	L"Squadra 12",
	L"Squadra 13",
	L"Squadra 14",
	L"Squadra 15",
	L"Squadra 16",
	L"Squadra 17",
	L"Squadra 18",
	L"Squadra 19",
	L"Squadra 20",
	L"Annulla",
};


const wchar_t* pPersonnelScreenStrings[] =
{
	L"Deposito med.:", // amount of medical deposit put down on the merc
	L"Contratto in corso:", // cost of current contract
	L"Uccisi", // number of kills by merc
	L"Assistiti", // number of assists on kills by merc
	L"Costo giornaliero:", // daily cost of merc
	L"Tot. costo fino a oggi:", // total cost of merc
	L"Contratto:", // cost of current contract
	L"Tot. servizio fino a oggi:", // total service rendered by merc
	L"Salario arretrato:", // amount left on MERC merc to be paid
	L"Percentuale di colpi:", // percentage of shots that hit target
	L"Battaglie", // number of battles fought
	L"Numero ferite", // number of times merc has been wounded
	L"Destrezza:",
	L"Nessuna abilità",
};


//These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
const wchar_t* gzMercSkillText[] =
{
	L"Nessuna abilità",
	L"Forzare serrature",
	L"Corpo a corpo",
	L"Elettronica",
	L"Op. notturne",
	L"Lanciare",
	L"Istruire",
	L"Armi pesanti",
	L"Armi automatiche",
	L"Clandestino",
	L"Ambidestro",
	L"Furtività",
	L"Arti marziali",
	L"Coltelli",
	L"Bonus per altezza",
	L"Camuffato",
	L"(Esperto)",
};


// This is pop up help text for the options that are available to the merc

const wchar_t* pTacticalPopupButtonStrings[] =
{
	L"|Stare fermi/Camminare",
	L"|Accucciarsi/Muoversi accucciato",
	L"Stare fermi/|Correre",
	L"|Prono/Strisciare",
	L"|Guardare",
	L"Agire",
	L"Parlare",
	L"Esaminare (|C|t|r|l)",

	// Pop up door menu
	L"Aprire manualmente",
	L"Esaminare trappole",
	L"Grimaldello",
	L"Forzare",
	L"Liberare da trappole",
	L"Chiudere",
	L"Aprire",
	L"Usare esplosivo per porta",
	L"Usare piede di porco",
	L"Annulla (|E|s|c)",
	L"Chiudere",
};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.

const wchar_t* pDoorTrapStrings[] =
{
	L"Nessuna trappola",
	L"una trappola esplosiva",
	L"una trappola elettrica",
	L"una trappola con sirena",
	L"una trappola con allarme insonoro",
};

// On the map screen, there are four columns. This text is popup help text that identifies the individual columns.

const wchar_t* pMapScreenMouseRegionHelpText[] =
{
	L"Selezionare postazioni",
	L"Assegnare mercenario",
	L"Tracciare percorso di viaggio",
	L"Merc |Contratto",
	L"Eliminare mercenario",
	L"Dormire",
};

// volumes of noises

const wchar_t* pNoiseVolStr[] =
{
	L"DEBOLE",
	L"DEFINITO",
	L"FORTE",
	L"MOLTO FORTE",
};

// types of noises

const wchar_t* pNoiseTypeStr[] = // OBSOLETE
{
	L"SCONOSCIUTO",
	L"rumore di MOVIMENTO",
	L"SCRICCHIOLIO",
	L"TONFO IN ACQUA",
	L"IMPATTO",
	L"SPARO",
	L"ESPLOSIONE",
	L"URLA",
	L"IMPATTO",
	L"IMPATTO",
	L"FRASTUONO",
	L"SCHIANTO",
};

// Directions that are used to report noises

const wchar_t* pDirectionStr[] =
{
	L"il NORD-EST",
	L"il EST",
	L"il SUD-EST",
	L"il SUD",
	L"il SUD-OVEST",
	L"il OVEST",
	L"il NORD-OVEST",
	L"il NORD",
};

// These are the different terrain types.

const wchar_t* pLandTypeStrings[] =
{
	L"Urbano",
	L"Strada",
	L"Pianure",
	L"Deserto",
	L"Boschi",
	L"Foresta",
	L"Palude",
	L"Acqua",
	L"Colline",
	L"Impervio",
	L"Fiume",	//river from north to south
	L"Fiume",	//river from east to west
	L"Paese straniero",
	//NONE of the following are used for directional travel, just for the sector description.
	L"Tropicale",
	L"Campi",
	L"Pianure, strada",
	L"Boschi, strada",
	L"Fattoria, strada",
	L"Tropicale, strada",
	L"Foresta, strada",
	L"Linea costiera",
	L"Montagna, strada",
	L"Litoraneo, strada",
	L"Deserto, strada",
	L"Palude, strada",
	L"Boschi, postazione SAM",
	L"Deserto, postazione SAM",
	L"Tropicale, postazione SAM",
	L"Meduna, postazione SAM",

	//These are descriptions for special sectors
	L"Ospedale di Cambria",
	L"Aeroporto di Drassen",
	L"Aeroporto di Meduna",
	L"Postazione SAM",
	L"Nascondiglio ribelli", //The rebel base underground in sector A10
	L"Prigione sotterranea di Tixa",	//The basement of the Tixa Prison (J9)
	L"Tana della creatura",	//Any mine sector with creatures in it
	L"Cantina di Orta",	//The basement of Orta (K4)
	L"Tunnel",				//The tunnel access from the maze garden in Meduna
										//leading to the secret shelter underneath the palace
	L"Rifugio",				//The shelter underneath the queen's palace
	L"",							//Unused
};

const wchar_t* gpStrategicString[] =
{
	L"%ls sono stati individuati nel settore %c%d e un'altra squadra sta per arrivare.",	//STR_DETECTED_SINGULAR
	L"%ls sono stati individuati nel settore %c%d e un'altra squadra sta per arrivare.",	//STR_DETECTED_PLURAL
	L"Volete coordinare un attacco simultaneo?",													//STR_COORDINATE

	//Dialog strings for enemies.

	L"Il nemico offre la possibilità di arrendervi.",			//STR_ENEMY_SURRENDER_OFFER
	L"Il nemico ha catturato i vostri mercenari sopravvissuti.",	//STR_ENEMY_CAPTURED

	//The text that goes on the autoresolve buttons

	L"Ritirarsi", 		//The retreat button				//STR_AR_RETREAT_BUTTON
	L"Fine",		//The done button				//STR_AR_DONE_BUTTON

	//The headers are for the autoresolve type (MUST BE UPPERCASE)

	L"DIFENDERE",								//STR_AR_DEFEND_HEADER
	L"ATTACCARE",								//STR_AR_ATTACK_HEADER
	L"INCONTRARE",								//STR_AR_ENCOUNTER_HEADER
	L"settore",		//The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

	//The battle ending conditions

	L"VITTORIA!",								//STR_AR_OVER_VICTORY
	L"SCONFITTA!",								//STR_AR_OVER_DEFEAT
	L"ARRENDERSI!",							//STR_AR_OVER_SURRENDERED
	L"CATTURATI!",								//STR_AR_OVER_CAPTURED
	L"RITIRARSI!",								//STR_AR_OVER_RETREATED

	//These are the labels for the different types of enemies we fight in autoresolve.

	L"Esercito",							//STR_AR_MILITIA_NAME,
	L"Èlite",								//STR_AR_ELITE_NAME,
	L"Truppa",								//STR_AR_TROOP_NAME,
	L"Amministratore",								//STR_AR_ADMINISTRATOR_NAME,
	L"Creatura",								//STR_AR_CREATURE_NAME,

	//Label for the length of time the battle took

	L"Tempo trascorso",							//STR_AR_TIME_ELAPSED,

	//Labels for status of merc if retreating.  (UPPERCASE)

	L"RITIRATOSI",								//STR_AR_MERC_RETREATED,
	L"RITIRARSI",								//STR_AR_MERC_RETREATING,
	L"RITIRATA",								//STR_AR_MERC_RETREAT,

	//PRE BATTLE INTERFACE STRINGS
	//Goes on the three buttons in the prebattle interface.  The Auto resolve button represents
	//a system that automatically resolves the combat for the player without having to do anything.
	//These strings must be short (two lines -- 6-8 chars per line)

	L"Esito",							//STR_PB_AUTORESOLVE_BTN,
	L"Vai al settore",							//STR_PB_GOTOSECTOR_BTN,
	L"Ritira merc.",							//STR_PB_RETREATMERCS_BTN,

	//The different headers(titles) for the prebattle interface.
	L"SCONTRO NEMICO",							//STR_PB_ENEMYENCOUNTER_HEADER,
	L"INVASIONE NEMICA",							//STR_PB_ENEMYINVASION_HEADER, // 30
	L"IMBOSCATA NEMICA",								//STR_PB_ENEMYAMBUSH_HEADER
	L"INTRUSIONE NEMICA NEL SETTORE",				//STR_PB_ENTERINGENEMYSECTOR_HEADER
	L"ATTACCO DELLE CREATURE",							//STR_PB_CREATUREATTACK_HEADER
	L"IMBOSCATA DEI BLOODCAT",							//STR_PB_BLOODCATAMBUSH_HEADER
	L"INTRUSIONE NELLA TANA BLOODCAT",			//STR_PB_ENTERINGBLOODCATLAIR_HEADER

	//Various single words for direct translation.  The Civilians represent the civilian
	//militia occupying the sector being attacked.  Limited to 9-10 chars

	L"Postazione",
	L"Nemici",
	L"Mercenari",
	L"Esercito",
	L"Creature",
	L"Bloodcat",
	L"Settore",
	L"Nessuno",		//If there are no uninvolved mercs in this fight.
	L"N/A",			//Acronym of Not Applicable
	L"g",			//One letter abbreviation of day
	L"o",			//One letter abbreviation of hour

	//TACTICAL PLACEMENT USER INTERFACE STRINGS
	//The four buttons

	L"Sgombro",
	L"Sparsi",
	L"In gruppo",
	L"Fine",

	//The help text for the four buttons.  Use \n to denote new line (just like enter).

	L"|Mostra chiaramente tutte le postazioni dei mercenari,\ne vi permette di rimetterli in gioco manualmente.",
	L"A caso |sparge i vostri mercenari\nogni volta che lo premerete.",
	L"Vi permette di scegliere dove vorreste |raggruppare i vostri mercenari.",
	L"Cliccate su questo pulsante quando avrete\nscelto le postazioni dei vostri mercenari. (|I|n|v|i|o)",
	L"Dovete posizionare tutti i vostri mercenari\nprima di iniziare la battaglia.",

	//Various strings (translate word for word)

	L"Settore",
	L"Scegliete le postazioni di intervento",

	//Strings used for various popup message boxes.  Can be as long as desired.

	L"Non sembra così bello qui. È inacessibile. Provate con una diversa postazione.",
	L"Posizionate i vostri mercenari nella sezione illuminata della mappa.",

	//These entries are for button popup help text for the prebattle interface.  All popup help
	//text supports the use of \n to denote new line.  Do not use spaces before or after the \n.
	L"|Automaticamente svolge i combattimenti al vostro posto\nsenza caricare la mappa.",
	L"Non è possibile utilizzare l'opzione di risoluzione automatica quando\nil giocatore sta attaccando.",
	L"|Entrate nel settore per catturare il nemico.",
	L"|Rimandate il gruppo al settore precedente.",				//singular version
	L"|Rimandate tutti i gruppi ai loro settori precedenti.", //multiple groups with same previous sector

	//various popup messages for battle conditions.

	//%c%d is the sector -- ex:  A9
	L"I nemici attaccano il vostro esercito nel settore %c%d.",
	//%c%d is the sector -- ex:  A9
	L"Le creature attaccano il vostro esercito nel settore %c%d.",
	//1st %d refers to the number of civilians eaten by monsters,  %c%d is the sector -- ex:  A9
	//Note:  the minimum number of civilians eaten will be two.
	L"Le creature attaccano e uccidono %d civili nel settore %ls.",
	//%ls is the sector location -- ex:  A9: Omerta
	L"I nemici attaccano i vostri mercenari nel settore %ls. Nessuno dei vostri mercenari è in grado di combattere!",
	//%ls is the sector location -- ex:  A9: Omerta
	L"I nemici attaccano i vostri mercenari nel settore %ls. Nessuno dei vostri mercenari è in grado di combattere!",

};

//This is the day represented in the game clock.  Must be very short, 4 characters max.
const wchar_t gpGameClockString[] = L"Gg";

//When the merc finds a key, they can get a description of it which
//tells them where and when they found it.
const wchar_t* sKeyDescriptionStrings[2] =
{
	L"Settore trovato:",
	L"Giorno trovato:",
};

//The headers used to describe various weapon statistics.

const wchar_t* gWeaponStatsDesc[] =
{
	L"Peso (%ls):",
	L"Stato:",
	L"Ammontare:", 		// Number of bullets left in a magazine
	L"Git:",		// Range
	L"Dan:",		// Damage
	L"PA:",			// abbreviation for Action Points
	L"="
};

//The headers used for the merc's money.

const wchar_t gMoneyStatsDesc[][ 13 ] =
{
	L"Ammontare",
	L"Rimanenti:", //this is the overall balance
	L"Ammontare",
	L"Da separare:", // the amount he wants to separate from the overall balance to get two piles of money

	L"Bilancio",
	L"corrente",
	L"Ammontare",
	L"del prelievo",
};

//The health of various creatures, enemies, characters in the game. The numbers following each are for comment
//only, but represent the precentage of points remaining.

const wchar_t zHealthStr[][13] =
{
	L"MORENTE",		//	>= 0
	L"CRITICO", 		//	>= 15
	L"DEBOLE",		//	>= 30
	L"FERITO",    	//	>= 45
	L"SANO",    	//	>= 60
	L"FORTE",     	// 	>= 75
  L"ECCELLENTE",		// 	>= 90
};

const wchar_t* gzMoneyAmounts[6] =
{
	L"$1000",
	L"$100",
	L"$10",
	L"Fine",
	L"Separare",
	L"Prelevare",
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
const wchar_t gzProsLabel[] = L"Vant.:";
const wchar_t gzConsLabel[] = L"Svant.:";

//Conversation options a player has when encountering an NPC
const wchar_t zTalkMenuStrings[6][ SMALL_STRING_LENGTH ] =
{
	L"Vuoi ripetere?", 	//meaning "Repeat yourself"
	L"Amichevole",		//approach in a friendly
	L"Diretto",		//approach directly - let's get down to business
	L"Minaccioso",		//approach threateningly - talk now, or I'll blow your face off
	L"Dai",
	L"Recluta",
};

//Some NPCs buy, sell or repair items. These different options are available for those NPCs as well.
const wchar_t zDealerStrings[4][ SMALL_STRING_LENGTH ]=
{
	L"Compra/Vendi",
	L"Compra",
	L"Vendi",
	L"Ripara",
};

const wchar_t zDialogActions[] = L"Fine";


//These are vehicles in the game.

const wchar_t* pVehicleStrings[] =
{
 L"Eldorado",
 L"Hummer", // a hummer jeep/truck -- military vehicle
 L"Icecream Truck",
 L"Jeep",
 L"Carro armato",
 L"Elicottero",
};

const wchar_t* pShortVehicleStrings[] =
{
	L"Eldor.",
	L"Hummer",			// the HMVV
	L"Truck",
	L"Jeep",
	L"Carro",
	L"Eli", 				// the helicopter
};

const wchar_t* zVehicleName[] =
{
	L"Eldorado",
	L"Hummer",		//a military jeep. This is a brand name.
	L"Truck",			// Ice cream truck
	L"Jeep",
	L"Carro",
	L"Eli", 		//an abbreviation for Helicopter
};


//These are messages Used in the Tactical Screen

const wchar_t* const TacticalStr[] =
{
	L"Attacco aereo",
	L"Ricorrete al pronto soccorso automaticamente?",

	// CAMFIELD NUKE THIS and add quote #66.

	L"%ls nota ch egli oggetti mancano dall'equipaggiamento.",

	// The %ls is a string from pDoorTrapStrings

	L"La serratura ha %ls",
	L"Non ci sono serrature",
	L"La serratura non presenta trappole",
	// The %ls is a merc name
	L"%ls non ha la chiave giusta",
	L"La serratura non presenta trappole",
	L"Serrato",
	L"",
	L"TRAPPOLE",
	L"SERRATO",
	L"APERTO",
	L"FRACASSATO",
	L"C'è un interruttore qui. Lo volete attivare?",
	L"Disattivate le trappole?",
	L"Più...",

	// In the next 2 strings, %ls is an item name

	L"Il %ls è stato posizionato sul terreno.",
	L"Il %ls è stato dato a %ls.",

	// In the next 2 strings, %ls is a name

	L"%ls è stato pagato completamente.",
	L"Bisogna ancora dare %d a %ls.",
	L"Scegliete la frequenza di detonazione:",  	//in this case, frequency refers to a radio signal
	L"Quante volte finché la bomba non esploderà:",	//how much time, in turns, until the bomb blows
	L"Stabilite la frequenza remota di detonazione:", 	//in this case, frequency refers to a radio signal
	L"Disattivate le trappole?",
	L"Rimuovete la bandiera blu?",
	L"Mettete qui la bandiera blu?",
	L"Fine del turno",

	// In the next string, %ls is a name. Stance refers to way they are standing.

	L"Siete sicuri di volere attaccare %ls ?",
	L"Ah, i veicoli non possono cambiare posizione.",
	L"Il robot non può cambiare posizione.",

	// In the next 3 strings, %ls is a name

	L"%ls non può cambiare posizione.",
	L"%ls non sono ricorsi al pronto soccorso qui.",
	L"%ls non ha bisogno del pronto soccorso.",
	L"Non può muoversi là.",
	L"La vostra squadra è al completo. Non c'è spazio per una recluta.",	//there's no room for a recruit on the player's team

	// In the next string, %ls is a name

	L"%ls è stato reclutato.",

	// Here %ls is a name and %d is a number

	L"Bisogna dare %d a $%ls.",

	// In the next string, %ls is a name

	L"Scortate %ls?",

	// In the next string, the first %ls is a name and the second %ls is an amount of money (including $ sign)

	L"Il salario di %ls ammonta a %ls per giorno?",

	// This line is used repeatedly to ask player if they wish to participate in a boxing match.

	L"Volete combattere?",

	// In the next string, the first %ls is an item name and the
	// second %ls is an amount of money (including $ sign)

	L"Comprate %ls per %ls?",

	// In the next string, %ls is a name

	L"%ls è scortato dalla squadra %d.",

	// These messages are displayed during play to alert the player to a particular situation

	L"INCEPPATA",					//weapon is jammed.
	L"Il robot ha bisogno di munizioni calibro %ls.",		//Robot is out of ammo
	L"Cosa? Impossibile.",		//Merc can't throw to the destination he selected

	// These are different buttons that the player can turn on and off.

	L"Modalità furtiva (|Z)",
	L"Schermata della |mappa",
	L"Fine del turno (|D)",
	L"Parlato",
	L"Muto",
	L"Alza (|P|a|g|S|ù)",
	L"Livello della vista (|T|a|b)",
	L"Scala / Salta",
	L"Abbassa (|P|a|g|G|i|ù)",
	L"Esamina (|C|t|r|l)",
	L"Mercenario precedente",
	L"Prossimo mercenario (|S|p|a|z|i|o)",
	L"|Opzioni",
	L"Modalità a raffica (|B)",
	L"Guarda/Gira (|L)",
	L"Salute: %d/%d\nEnergia: %d/%d\nMorale: %ls",
	L"Eh?",					//this means "what?"
	L"Fermo",					//an abbrieviation for "Continued"
	L"Audio on per %ls.",
	L"Audio off per %ls.",
	L"Salute: %d/%d\nCarburante: %d/%d",
	L"Uscita veicoli" ,
	L"Cambia squadra (|M|a|i|u|s|c |S|p|a|z|i|o)",
	L"Guida",
	L"N/A",						//this is an acronym for "Not Applicable."
	L"Usa (Corpo a corpo)",
	L"Usa (Arma da fuoco)",
	L"Usa (Lama)",
	L"Usa (Esplosivo)",
	L"Usa (Kit medico)",
	L"Afferra",
	L"Ricarica",
	L"Dai",
	L"%ls è partito.",
	L"%ls è arrivato.",
	L"%ls ha esaurito i Punti Azione.",
	L"%ls non è disponibile.",
	L"%ls è tutto bendato.",
	L"%ls non è provvisto di bende.",
	L"Nemico nel settore!",
	L"Nessun nemico in vista.",
	L"Punti Azione insufficienti.",
	L"Nessuno sta utilizzando il comando a distanza.",
	L"Il fuoco a raffica ha svuotato il caricatore!",
	L"SOLDATO",
	L"CREPITUS",
	L"ESERCITO",
	L"CIVILE",
	L"Settore di uscita",
	L"OK",
	L"Annulla",
	L"Merc. selezionato",
	L"Tutta la squadra",
	L"Vai nel settore",
	L"Vai alla mappa",
	L"Non puoi uscire dal settore da questa parte.",
	L"%ls è troppo lontano.",
	L"Rimuovi le fronde degli alberi",
	L"Mostra le fronde degli alberi",
	L"CORVO",				//Crow, as in the large black bird
	L"COLLO",
	L"TESTA",
	L"TORSO",
	L"GAMBE",
	L"Vuoi dire alla Regina cosa vuole sapere?",
	L"Impronta digitale ID ottenuta",
	L"Impronta digitale ID non valida. Arma non funzionante",
	L"Raggiunto scopo",
	L"Sentiero bloccato",
	L"Deposita/Preleva soldi",		//Help text over the $ button on the Single Merc Panel
	L"Nessuno ha bisogno del pronto soccorso.",
	L"Bloccato.",											// Short form of JAMMED, for small inv slots
	L"Non può andare là.",					// used ( now ) for when we click on a cliff
	L"La persona rifiuta di muoversi.",
	// In the following message, '%ls' would be replaced with a quantity of money (e.g. $200)
	L"Sei d'accordo a pagare %ls?",
	L"Accetti il trattamento medico gratuito?",
	L"Vuoi sposare Daryl?",
	L"Quadro delle chiavi",
	L"Non puoi farlo con un EPC.",
	L"Risparmi Krott?",
	L"Fuori dalla gittata dell'arma",
	L"Minatore",
	L"Il veicolo può viaggiare solo tra i settori",
	L"Non è in grado di fasciarsi da solo ora",
	L"Sentiero bloccato per %ls",
//	L"I tuoi mercenari, che erano stati catturati dall'esercito di Deidranna, sono stati imprigionati qui!",
	L"I mercenari catturati dall'esercito di Deidranna, sono stati imprigionati qui!",
	L"Serratura manomessa",
	L"Serratura distrutta",
	L"Qualcun altro sta provando a utilizzare questa porta.",
	L"Salute: %d/%d\nCarburante: %d/%d",
  L"%ls non riesce a vedere %ls.",  // Cannot see person trying to talk to
};

//Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different circumstances in the "exiting sector" interface.
const wchar_t* pExitingSectorHelpText[] =
{
	//Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box is checked.
	L"Se selezionato, il settore adiacente verrà immediatamente caricato.",
	L"Se selezionato, sarete automaticamente posti nella schermata della mappa\nvisto che i vostri mercenari avranno bisogno di tempo per viaggiare.",

	//If you attempt to leave a sector when you have multiple squads in a hostile sector.
	L"Questo settore è occupato da nemicie non potete lasciare mercenari qui.\nDovete risolvere questa situazione prima di caricare qualsiasi altro settore.",

	//Because you only have one squad in the sector, and the "move all" option is checked, the "go to sector" option is locked to on.
	//The helptext explains why it is locked.
	L"Rimuovendo i vostri mercenari da questo settore,\nil settore adiacente verrà immediatamente caricato.",
	L"Rimuovendo i vostri mercenari da questo settore,\nverrete automaticamente postinella schermata della mappa\nvisto che i vostri mercenari avranno bisogno di tempo per viaggiare.",

	//If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being escorted.  The "single" button is disabled.
	L"%ls ha bisogno di essere scortato dai vostri mercenari e non può lasciare questo settore da solo.",

	//If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc will be prohibited from leaving alone.
	//There are several strings depending on the gender of the merc and how many EPCs are in the squad.
	//DO NOT USE THE NEWLINE HERE AS IT IS USED FOR BOTH HELPTEXT AND SCREEN MESSAGES!
	L"%ls non può lasciare questo settore da solo, perché sta scortando %ls.", //male singular
	L"%ls non può lasciare questo settore da solo, perché sta scortando %ls.", //female singular
	L"%ls non può lasciare questo settore da solo, perché sta scortando altre persone.", //male plural
	L"%ls non può lasciare questo settore da solo, perché sta scortando altre persone.", //female plural

	//If one or more of your mercs in the selected squad aren't in range of the traversal area, then the  "move all" option is disabled,
	//and this helptext explains why.
	L"Tutti i vostri personaggi devono trovarsi nei paraggi\nin modo da permettere alla squadra di attraversare.",

	//Standard helptext for single movement.  Explains what will happen (splitting the squad)
	L"Se selezionato, %ls viaggerà da solo, e\nautomaticamente verrà riassegnato a un'unica squadra.",

	//Standard helptext for all movement.  Explains what will happen (moving the squad)
	L"Se selezionato, la vostra\nsquadra attualmente selezionata viaggerà, lasciando questo settore.",

	//This strings is used BEFORE the "exiting sector" interface is created.  If you have an EPC selected and you attempt to tactically
	//traverse the EPC while the escorting mercs aren't near enough (or dead, dying, or unconscious), this message will appear and the
	//"exiting sector" interface will not appear.  This is just like the situation where
	//This string is special, as it is not used as helptext.  Do not use the special newline character (\n) for this string.
	L"%ls è scortato dai vostri mercenari e non può lasciare questo settore da solo. Gli altri vostri mercenari devono trovarsi nelle vicinanze prima che possiate andarvene.",
};



const wchar_t* pRepairStrings[] =
{
	L"Oggetti", 		// tell merc to repair items in inventory
	L"Sito SAM", 		// tell merc to repair SAM site - SAM is an acronym for Surface to Air Missile
	L"Annulla", 		// cancel this menu
	L"Robot", 		// repair the robot
};


// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."

const wchar_t* sPreStatBuildString[] =
{
	L"perduto", 			// the merc has lost a statistic
	L"guadagnato", 		// the merc has gained a statistic
	L"punto di",	// singular
	L"punti di",	// plural
	L"livello di",	// singular
	L"livelli di",	// plural
};

const wchar_t* sStatGainStrings[] =
{
	L"salute.",
	L"agilità.",
	L"destrezza.",
	L"saggezza.",
	L"pronto socc.",
	L"abilità esplosivi.",
	L"abilità meccanica.",
	L"mira.",
	L"esperienza.",
	L"forza.",
	L"comando.",
};


const wchar_t* pHelicopterEtaStrings[] =
{
	L"Distanza totale: ", 			// total distance for helicopter to travel
	L"Sicura: ", 			// distance to travel to destination
	L"Insicura: ", 			// distance to return from destination to airport
	L"Costo totale: ", 		// total cost of trip by helicopter
	L"TPA: ", 			// ETA is an acronym for "estimated time of arrival"
	L"L'elicottero ha poco carburante e deve atterrare in territorio nemico!",	// warning that the sector the helicopter is going to use for refueling is under enemy control ->
  L"Passeggeri: ",
  L"Seleziona Skyrider o gli Arrivi Drop-off?",
  L"Skyrider",
  L"Arrivi",
};

const wchar_t sMapLevelString[] = L"Sottolivello:"; // what level below the ground is the player viewing in mapscreen

const wchar_t gsLoyalString[] = L"%d%% Leale"; // the loyalty rating of a town ie : Loyal 53%


// error message for when player is trying to give a merc a travel order while he's underground.
const wchar_t gsUndergroundString[] = L"non può portare ordini di viaggio sottoterra.";

const wchar_t* gsTimeStrings[] =
{
	L"h",				// hours abbreviation
	L"m",				// minutes abbreviation
	L"s",				// seconds abbreviation
	L"g",				// days abbreviation
};

// text for the various facilities in the sector

const wchar_t* sFacilitiesStrings[] =
{
	L"Nessuno",
	L"Ospedale",
	L"Fabbrica",
	L"Prigione",
	L"Militare",
	L"Aeroporto",
	L"Frequenza di fuoco",		// a field for soldiers to practise their shooting skills
};

// text for inventory pop up button

const wchar_t* pMapPopUpInventoryText[] =
{
	L"Inventario",
	L"Uscita",
};

// town strings

const wchar_t* pwTownInfoStrings[] =
{
	L"Dimensione",					// size of the town in sectors
	L"Controllo", 					// how much of town is controlled
	L"Miniera", 				// mine associated with this town
	L"Lealtà",					// the loyalty level of this town
	L"Servizi principali", 				// main facilities in this town
	L"addestramento civili",				// state of civilian training in town
	L"Esercito", 					// the state of the trained civilians in the town
};

// Mine strings

const wchar_t* pwMineStrings[] =
{
	L"Miniera",						// 0
	L"Argento",
	L"Oro",
	L"Produzione giornaliera",
	L"Produzione possibile",
	L"Abbandonata",				// 5
	L"Chiudi",
	L"Esci",
	L"Produci",
	L"Stato",
	L"Ammontare produzione",
	L"Tipo di minerale",				// 10
	L"Controllo della città",
	L"Lealtà della città",
};

// blank sector strings

const wchar_t* pwMiscSectorStrings[] =
{
	L"Forze nemiche",
	L"Settore",
	L"# di oggetti",
	L"Sconosciuto",
	L"Controllato",
	L"Sì",
	L"No",
};

// error strings for inventory

const wchar_t* pMapInventoryErrorString[] =
{
	L"Non può selezionare quel mercenario.",  //MARK CARTER
	L"%ls non si trova nel settore per prendere quell'oggetto.",
	L"Durante il combattimento, dovrete raccogliere gli oggetti manualmente.",
	L"Durante il combattimento, dovrete rilasciare gli oggetti manualmente.",
	L"%ls non si trova nel settore per rilasciare quell'oggetto.",
};

const wchar_t* pMapInventoryStrings[] =
{
	L"Posizione", 			// sector these items are in
	L"Totale oggetti", 		// total number of items in sector
};


// movement menu text

const wchar_t* pMovementMenuStrings[] =
{
	L"Muovere mercenari nel settore %ls", 	// title for movement box
	L"Rotta spostamento esercito", 		// done with movement menu, start plotting movement
	L"Annulla", 		// cancel this menu
	L"Altro",		// title for group of mercs not on squads nor in vehicles
};


const wchar_t* pUpdateMercStrings[] =
{
	L"Oops:", 			// an error has occured
	L"Scaduto contratto mercenari:", 	// this pop up came up due to a merc contract ending
	L"Portato a termine incarico mercenari:", // this pop up....due to more than one merc finishing assignments
	L"Mercenari di nuovo al lavoro:", // this pop up ....due to more than one merc waking up and returing to work
	L"Mercenari a riposo:", // this pop up ....due to more than one merc being tired and going to sleep
	L"Contratti in scadenza:", 	// this pop up came up due to a merc contract ending
};

// map screen map border buttons help text

const wchar_t* pMapScreenBorderButtonHelpText[] =
{
	L"Mostra città (|w)",
	L"Mostra |miniere",
	L"Mos|tra squadre & nemici",
	L"Mostra spazio |aereo",
	L"Mostra oggett|i",
	L"Mostra esercito & nemici (|Z)",
};


const wchar_t* pMapScreenBottomFastHelp[] =
{
	L"Portati|le",
	L"Tattico (|E|s|c)",
	L"|Opzioni",
	L"Dilata tempo (|+)", 	// time compress more
	L"Comprime tempo (|-)", 	// time compress less
	L"Messaggio precedente (|S|u)\nIndietro (|P|a|g|S|u)", 	// previous message in scrollable list
	L"Messaggio successivo (|G|i|ù)\nAvanti (|P|a|g|G|i|ù)", 	// next message in the scrollable list
	L"Inizia/Ferma tempo (|S|p|a|z|i|o)",	// start/stop time compression
};

const wchar_t pMapScreenBottomText[] = L"Bilancio attuale"; // current balance in player bank account

const wchar_t pMercDeadString[] = L"%ls è morto.";


const wchar_t pDayStrings[] = L"Giorno";

// the list of email sender names

const wchar_t* pSenderNameList[] =
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
	L"Vicki",			//10
	L"Trevor",
	L"Grunty",
	L"Ivan",
	L"Steroid",
	L"Igor",			//15
	L"Shadow",
	L"Red",
	L"Reaper",
	L"Fidel",
	L"Fox",				//20
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
	L"Assicurazione M.I.S.",
	L"Bobby Ray",
	L"Capo",
	L"John Kulba",
	L"A.I.M.",
};


// new mail notify string
const wchar_t pNewMailStrings[] = L"Avete una nuova E-mail...";


// confirm player's intent to delete messages

const wchar_t* pDeleteMailStrings[] =
{
 L"Eliminate l'E-mail?",
 L"Eliminate l'E-mail NON LETTA?",
};


// the sort header strings

const wchar_t* pEmailHeaders[] =
{
	L"Da:",
	L"Sogg.:",
	L"Giorno:",
};

// email titlebar text
const wchar_t pEmailTitleText[] = L"posta elettronica";


// the financial screen strings
const wchar_t pFinanceTitle[] = L"Contabile aggiuntivo"; // the name we made up for the financial program in the game

const wchar_t* pFinanceSummary[] =
{
	L"Crediti:", 				// credit (subtract from) to player's account
	L"Debiti:", 				// debit (add to) to player's account
	L"Entrate effettive di ieri:",
	L"Altri depositi di ieri:",
	L"Debiti di ieri:",
	L"Bilancio di fine giornata:",
	L"Entrate effettive di oggi:",
	L"Altri depositi di oggi:",
	L"Debiti di oggi:",
	L"Bilancio attuale:",
	L"Entrate previste:",
	L"Bilancio previsto:", 		// projected balance for player for tommorow
};


// headers to each list in financial screen

const wchar_t* pFinanceHeaders[] =
{
  L"Giorno", 				// the day column
	L"Crediti", 			// the credits column (to ADD money to your account)
	L"Debiti",				// the debits column (to SUBTRACT money from your account)
	L"Transazione", // transaction type - see TransactionText below
	L"Bilancio", 		// balance at this point in time
	L"Pagina", 				// page number
	L"Giorno(i)", 			// the day(s) of transactions this page displays
};


const wchar_t* pTransactionText[] =
{
	L"Interessi maturati",			// interest the player has accumulated so far
	L"Deposito anonimo",
	L"Tassa di transazione",
	L"Arruolato %ls dall'A.I.M.", // Merc was hired
	L"Acquistato da Bobby Ray", 		// Bobby Ray is the name of an arms dealer
	L"Acconti pagati al M.E.R.C.",
	L"Deposito medico per %ls", 		// medical deposit for merc
	L"Analisi del profilo I.M.P.", 		// IMP is the acronym for International Mercenary Profiling
	L"Assicurazione acquistata per %ls",
	L"Assicurazione ridotta per %ls",
	L"Assicurazione estesa per %ls", 				// johnny contract extended
	L"Assicurazione annullata %ls",
	L"Richiesta di assicurazione per %ls", 		// insurance claim for merc
	L"Est. contratto di %ls per 1 giorno.", 				// entend mercs contract by a day
	L"Est. %ls contratto per 1 settimana.",
	L"Est. %ls contratto per 2 settimane.",
	L"Entrata mineraria",
	L"", //String nuked
	L"Fiori acquistati",
	L"Totale rimborso medico per %ls",
	L"Parziale rimborso medico per %ls",
	L"Nessun rimborso medico per %ls",
	L"Pagamento a %ls",		// %ls is the name of the npc being paid
	L"Trasferimento fondi a %ls", 			// transfer funds to a merc
	L"Trasferimento fondi da %ls", 		// transfer funds from a merc
	L"Equipaggiamento esercito in %ls", // initial cost to equip a town's militia
	L"Oggetti acquistati da%ls.",	//is used for the Shop keeper interface.  The dealers name will be appended to the end of the string.
	L"%ls soldi depositati.",
};

// helicopter pilot payment

const wchar_t* pSkyriderText[] =
{
	L"Skyrider è stato pagato $%d", 			// skyrider was paid an amount of money
	L"A Skyrider bisogna ancora dare $%d", 		// skyrider is still owed an amount of money
	L"Skyrider non ha passeggeri. Se avete intenzione di trasportare mercenari in questo settore, assegnateli prima al Veicolo/Elicottero.",
};


// strings for different levels of merc morale

const wchar_t* pMoralStrings[] =
{
	L"Ottimo",
	L"Buono",
	L"Medio",
	L"Basso",
	L"Panico",
	L"Cattivo",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.
const wchar_t str_left_equipment[]   = L"L'equipaggio di %ls è ora disponibile a %ls (%c%d).";

// Status that appears on the Map Screen

const wchar_t* pMapScreenStatusStrings[] =
{
	L"Salute",
	L"Energia",
	L"Morale",
	L"Condizione",	// the condition of the current vehicle (its "health")
	L"Carburante",	// the fuel level of the current vehicle (its "energy")
};


const wchar_t* pMapScreenPrevNextCharButtonHelpText[] =
{
	L"Mercenario precedente (|S|i|n)", 			// previous merc in the list
	L"Mercenario successivo (|D|e|s)", 				// next merc in the list
};


const wchar_t pEtaString[] = L"TAP"; // eta is an acronym for Estimated Time of Arrival

const wchar_t* pTrashItemText[] =
{
	L"Non lo vedrete mai più. Siete sicuri?", 	// do you want to continue and lose the item forever
	L"Questo oggetto sembra DAVVERO importante. Siete DAVVERO SICURISSIMI di volerlo gettare via?", // does the user REALLY want to trash this item
};


const wchar_t* pMapErrorString[] =
{
	L"La squadra non può muoversi, se un mercenario dorme.",

//1-5
	L"Muovete la squadra al primo piano.",
	L"Ordini di movimento? È un settore nemico!",
	L"I mercenari devono essere assegnati a una squadra o a un veicolo per potersi muovere.",
	L"Non avete ancora membri nella squadra.", 		// you have no members, can't do anything
	L"I mercenari non possono attenersi agli ordini.",			 		// merc can't comply with your order
//6-10
	L"%ls ha bisogno di una scorta per muoversi. Inseritelo in una squadra che ne è provvista.", // merc can't move unescorted .. for a male
	L"%ls ha bisogno di una scorta per muoversi. Inseritela in una squadra che ne è provvista.", // for a female
	L"Il mercenario non è ancora arrivato ad Arulco!",
	L"Sembra che ci siano negoziazioni di contratto da stabilire.",
	L"",
//11-15
	L"Ordini di movimento? È in corso una battaglia!",
	L"Siete stati vittima di un'imboscata da parte dai Bloodcat nel settore %ls!",
	L"Siete appena entrati in quella che sembra una tana di un Bloodcat nel settore I16!",
	L"",
	L"La zona SAM in %ls è stata assediata.",
//16-20
	L"La miniera di %ls è stata assediata. La vostra entrata giornaliera è stata ridotta di %ls per giorno.",
	L"Il nemico ha assediato il settore %ls senza incontrare resistenza.",
	L"Almeno uno dei vostri mercenari non ha potuto essere affidato a questo incarico.",
	L"%ls non ha potuto unirsi alla %ls visto che è completamente pieno",
	L"%ls non ha potuto unirsi alla %ls visto che è troppo lontano.",
//21-25
	L"La miniera di %ls è stata invasa dalle forze armate di Deidranna!",
	L"Le forze armate di Deidranna hanno appena invaso la zona SAM in %ls",
	L"Le forze armate di Deidranna hanno appena invaso %ls",
	L"Le forze armate di Deidranna sono appena state avvistate in %ls.",
	L"Le forze armate di Deidranna sono appena partite per %ls.",
//26-30
	L"Almeno uno dei vostri mercenari non può riposarsi.",
	L"Almeno uno dei vostri mercenari non è stato svegliato.",
	L"L'esercito non si farà vivo finché non avranno finito di esercitarsi.",
	L"%ls non possono ricevere ordini di movimento adesso.",
	L"I militari che non si trovano entro i confini della città non possono essere spostati inquesto settore.",
//31-35
	L"Non potete avere soldati in %ls.",
	L"Un veicolo non può muoversi se è vuoto!",
	L"%ls è troppo grave per muoversi!",
	L"Prima dovete lasciare il museo!",
	L"%ls è morto!",
//36-40
	L"%ls non può andare a %ls perché si sta muovendo",
	L"%ls non può salire sul veicolo in quel modo",
	L"%ls non può unirsi alla %ls",
	L"Non potete comprimere il tempo finché non arruolerete nuovi mercenari!",
	L"Questo veicolo può muoversi solo lungo le strade!",
//41-45
	L"Non potete riassegnare i mercenari che sono già in movimento",
	L"Il veicolo è privo di benzina!",
	L"%ls è troppo stanco per muoversi.",
	L"Nessuno a bordo è in grado di guidare il veicolo.",
	L"Uno o più membri di questa squadra possono muoversi ora.",
//46-50
	L"Uno o più degli altri mercenari non può muoversi ora.",
	L"Il veicolo è troppo danneggiato!",
	L"Osservate che solo due mercenari potrebbero addestrare i militari in questo settore.",
	L"Il robot non può muoversi senza il suo controller. Metteteli nella stessa squadra.",
};


// help text used during strategic route plotting
const wchar_t* pMapPlotStrings[] =
{
	L"Cliccate di nuovo su una destinazione per confermare la vostra meta finale, oppure cliccate su un altro settore per fissare più tappe.",
	L"Rotta di spostamento confermata.",
	L"Destinazione immutata.",
	L"Rotta di spostamento annullata.",
	L"Rotta di spostamento accorciata.",
};


// help text used when moving the merc arrival sector
const wchar_t* pBullseyeStrings[] =
{
	L"Cliccate sul settore dove desiderate che i mercenari arrivino.",
	L"OK. I mercenari che stavano arrivando si sono dileguati a %ls",
	L"I mercenari non possono essere trasportati, lo spazio aereo non è sicuro!",
	L"Annullato. Il settore d'arrivo è immutato",
	L"Lo spazio aereo sopra %ls non è più sicuro! Il settore d'arrivo è stato spostato a %ls.",
};


// help text for mouse regions

const wchar_t* pMiscMapScreenMouseRegionHelpText[] =
{
	L"Entra nell'inventario (|I|n|v|i|o)",
	L"Getta via l'oggetto",
	L"Esci dall'inventario (|I|n|v|i|o)",
};


const wchar_t str_he_leaves_where_drop_equipment[]  = L"Volete che %ls lasci il suo equipaggiamento dove si trova ora (%ls) o in seguito a %ls (%ls) dopo aver preso il volo da Arulco?";
const wchar_t str_she_leaves_where_drop_equipment[] = L"Volete che %ls lasci il suo equipaggiamento dove si trova ora (%ls) o in seguito a %ls (%ls) dopo aver preso il volo da Arulco?";
const wchar_t str_he_leaves_drops_equipment[]       = L"%ls sta per partire e spedirà il suo equipaggiamento a %ls.";
const wchar_t str_she_leaves_drops_equipment[]      = L"%ls sta per partire e spedirà il suo equipaggiamento a %ls.";


// Text used on IMP Web Pages

const wchar_t* pImpPopUpStrings[] =
{
	L"Codice di autorizzazione non valido",
	L"State per riiniziare l'intero processo di profilo. Ne siete certi?",
	L"Inserite nome e cognome corretti oltre che al sesso",
	L"L'analisi preliminare del vostro stato finanziario mostra che non potete offrire un'analisi di profilo.",
  L"Opzione non valida questa volta.",
	L"Per completare un profilo accurato, dovete aver spazio per almeno uno dei membri della squadra.",
	L"Profilo già completato.",
};


// button labels used on the IMP site

const wchar_t* pImpButtonText[] =
{
	L"Cosa offriamo", 			// about the IMP site
	L"INIZIO", 			// begin profiling
	L"Personalità", 		// personality section
	L"Attributi", 		// personal stats/attributes section
	L"Ritratto", 			// the personal portrait selection
	L"Voce %d", 			// the voice selection
	L"Fine", 			// done profiling
	L"Ricomincio", 		// start over profiling
	L"Sì, scelgo la risposta evidenziata.",
	L"Sì",
	L"No",
	L"Finito", 			// finished answering questions
	L"Prec.", 			// previous question..abbreviated form
	L"Avanti", 			// next question
	L"SÌ, LO SONO.", 		// yes, I am certain
	L"NO, VOGLIO RICOMINCIARE.", // no, I want to start over the profiling process
	L"SÌ",
	L"NO",
	L"Indietro", 			// back one page
	L"Annulla", 			// cancel selection
	L"Sì, ne sono certo.",
	L"No, lasciami dare un'altra occhiata.",
	L"Immatricolazione", 			// the IMP site registry..when name and gender is selected
	L"Analisi", 			// analyzing your profile results
	L"OK",
	L"Voce",
};

const wchar_t* pExtraIMPStrings[] =
{
	L"Per completare il profilo attuale, seleziona 'Personalità'.",
	L"Ora che hai completato la Personalità, seleziona i tuoi attributi.",
	L"Con gli attributi ora assegnati, puoi procedere alla selezione del ritratto.",
	L"Per completare il processo, seleziona il campione della voce che più ti piace.",
};

const wchar_t pFilesTitle[] = L"Gestione risorse";

const wchar_t* pFilesSenderList[] =
{
  L"Rapporto", 		// the recon report sent to the player. Recon is an abbreviation for reconissance
	L"Intercetta #1", 		// first intercept file .. Intercept is the title of the organization sending the file...similar in function to INTERPOL/CIA/KGB..refer to fist record in files.txt for the translated title
	L"Intercetta #2",	   // second intercept file
	L"Intercetta #3",			 // third intercept file
	L"Intercetta #4", // fourth intercept file
	L"Intercetta #5", // fifth intercept file
	L"Intercetta #6", // sixth intercept file
};

// Text having to do with the History Log
const wchar_t pHistoryTitle[] = L"Registro";

const wchar_t* pHistoryHeaders[] =
{
	L"Giorno", 			// the day the history event occurred
	L"Pagina", 			// the current page in the history report we are in
	L"Giorno", 			// the days the history report occurs over
	L"Posizione", 			// location (in sector) the event occurred
	L"Evento", 			// the event label
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
	L"%ls è stato assunto dall'A.I.M.", 										// merc was hired from the aim site
	L"%ls è stato assunto dal M.E.R.C.", 									// merc was hired from the aim site
	L"%ls morì.", 															// merc was killed
	L"Acconti stanziati al M.E.R.C.",								// paid outstanding bills at MERC
	L"Assegno accettato da Enrico Chivaldori",
	//6-10
	L"Profilo generato I.M.P.",
	L"Acquistato contratto d'assicurazione per %ls.", 				// insurance contract purchased
	L"Annullato contratto d'assicurazione per %ls.", 				// insurance contract canceled
	L"Versamento per richiesta assicurazione per %ls.", 							// insurance claim payout for merc
	L"Esteso contratto di %ls di 1 giorno.", 						// Extented "mercs name"'s for a day
	//11-15
	L"Esteso contratto di %ls di 1 settimana.", 					// Extented "mercs name"'s for a week
	L"Esteso contratto di %ls di 2 settimane.", 					// Extented "mercs name"'s 2 weeks
	L"%ls è stato congedato.", 													// "merc's name" was dismissed.
	L"%ls è partito.", 																		// "merc's name" quit.
	L"avventura iniziata.", 															// a particular quest started
	//16-20
	L"avventura completata.",
	L"Parlato col capo minatore di %ls",									// talked to head miner of town
	L"Liberato %ls",
	L"Inganno utilizzato",
	L"Il cibo dovrebbe arrivare a Omerta domani",
	//21-25
	L"%ls ha lasciato la squadra per diventare la moglie di Daryl Hick",
	L"contratto di %ls scaduto.",
	L"%ls è stato arruolato.",
	L"Enrico si è lamentato della mancanza di progresso",
	L"Vinta battaglia",
	//26-30
	L"%ls miniera ha iniziato a esaurire i minerali",
	L"%ls miniera ha esaurito i minerali",
	L"%ls miniera è stata chiusa",
	L"%ls miniera è stata riaperta",
	L"Trovata una prigione chiamata Tixa.",
	//31-35
	L"Sentito di una fabbrica segreta di armi chiamata Orta.",
	L"Alcuni scienziati a Orta hanno donato una serie di lanciamissili.",
	L"La regina Deidranna ha bisogno di cadaveri.",
	L"Frank ha parlato di scontri a San Mona.",
	L"Un paziente pensa che lui abbia visto qualcosa nella miniera.",
	//36-40
	L"Incontrato qualcuno di nome Devin - vende esplosivi.",
	L"Imbattutosi nel famoso ex-mercenario dell'A.I.M. Mike!",
	L"Incontrato Tony - si occupa di armi.",
	L"Preso un lanciamissili dal Sergente Krott.",
	L"Concessa a Kyle la licenza del negozio di pelle di Angel.",
	//41-45
	L"Madlab ha proposto di costruire un robot.",
	L"Gabby può effettuare operazioni di sabotaggio contro sistemi d'allarme.",
	L"Keith è fuori dall'affare.",
	L"Howard ha fornito cianuro alla regina Deidranna.",
	L"Incontrato Keith - si occupa di un po' di tutto a Cambria.",
	//46-50
	L"Incontrato Howard - si occupa di farmaceutica a Balime",
	L"Incontrato Perko - conduce una piccola impresa di riparazioni.",
	L"Incontrato Sam di Balime - ha un negozio di hardware.",
	L"Franz si occupa di elettronica e altro.",
	L"Arnold possiede un'impresa di riparazioni a Grumm.",
	//51-55
	L"Fredo si occupa di elettronica a Grumm.",
	L"Donazione ricevuta da un ricco ragazzo a Balime.",
	L"Incontrato un rivenditore di un deposito di robivecchi di nome Jake.",
	L"Alcuni vagabondi ci hanno dato una scheda elettronica.",
	L"Corrotto Walter per aprire la porta del seminterrato.",
	//56-60
	L"Se Dave ha benzina, potrà fare il pieno gratis.",
	L"Corrotto Pablo.",
	L"Kingpin tiene i soldi nella miniera di San Mona.",
	L"%ls ha vinto il Combattimento Estremo",
	L"%ls ha perso il Combattimento Estremo",
	//61-65
	L"%ls è stato squalificato dal Combattimento Estremo",
	L"trovati moltissimi soldi nascosti nella miniera abbandonata.",
	L"Incontrato assassino ingaggiato da Kingpin.",
	L"Perso il controllo del settore",				//ENEMY_INVASION_CODE
	L"Difeso il settore",
	//66-70
	L"Persa la battaglia",							//ENEMY_ENCOUNTER_CODE
	L"Imboscata fatale",						//ENEMY_AMBUSH_CODE
	L"Annientata imboscata nemica",
	L"Attacco fallito",			//ENTERING_ENEMY_SECTOR_CODE
	L"Attacco riuscito!",
	//71-75
	L"Creature attaccate",			//CREATURE_ATTACK_CODE
	L"Ucciso dai Bloodcat",			//BLOODCAT_AMBUSH_CODE
	L"Massacrati dai Bloodcat",
	L"%ls è stato ucciso",
	L"Data a Carmen la testa di un terrorista",
	L"Massacro sinistro",
	L"Ucciso %ls",
};

const wchar_t pHistoryLocations[] = L"N/A"; // N/A is an acronym for Not Applicable

// icon text strings that appear on the laptop

const wchar_t* pLaptopIcons[] =
{
	L"E-mail",
	L"Rete",
	L"Finanza",
	L"Personale",
	L"Cronologia",
	L"File",
	L"Chiudi",
	L"sir-FER 4.0",			// our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added

const wchar_t* pBookMarkStrings[] =
{
	L"A.I.M.",
	L"Bobby Ray",
	L"I.M.P",
	L"M.E.R.C.",
	L"Pompe funebri",
	L"Fiorista",
	L"Assicurazione",
	L"Annulla",
};

// When loading or download a web page

const wchar_t* pDownloadString[] =
{
	L"Caricamento",
	L"Caricamento",
};

//This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

const wchar_t* gsAtmStartButtonText[] =
{
	L"Stato", 			// view stats of the merc
	L"Inventario", 			// view the inventory of the merc
	L"Impiego",
};

// Web error messages. Please use foreign language equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator

const wchar_t pErrorStrings[] = L"Connessione intermittente all'host. Tempi d'attesa più lunghi per il trasferimento.";


const wchar_t pPersonnelString[] = L"Mercenari:"; // mercs we have


const wchar_t pWebTitle[] = L"sir-FER 4.0"; // our name for the version of the browser, play on company name


// The titles for the web program title bar, for each page loaded

const wchar_t* pWebPagesTitles[] =
{
	L"A.I.M.",
	L"Membri dell'A.I.M.",
	L"Ritratti A.I.M.",		// a mug shot is another name for a portrait
	L"Categoria A.I.M.",
	L"A.I.M.",
	L"Membri dell'A.I.M.",
	L"Tattiche A.I.M.",
	L"Storia A.I.M.",
	L"Collegamenti A.I.M.",
	L"M.E.R.C.",
	L"Conti M.E.R.C.",
	L"Registrazione M.E.R.C.",
	L"Indice M.E.R.C.",
	L"Bobby Ray",
	L"Bobby Ray - Armi",
	L"Bobby Ray - Munizioni",
	L"Bobby Ray - Giubb. A-P",
	L"Bobby Ray - Varie",							//misc is an abbreviation for miscellaneous
	L"Bobby Ray - Usato",
	L"Bobby Ray - Ordine Mail",
	L"I.M.P.",
	L"I.M.P.",
	L"Servizio Fioristi Riuniti",
	L"Servizio Fioristi Riuniti - Galleria",
	L"Servizio Fioristi Riuniti - Ordine",
	L"Servizio Fioristi Riuniti - Card Gallery",
	L"Agenti assicurativi Malleus, Incus & Stapes",
	L"Informazione",
	L"Contratto",
	L"Commenti",
	L"Servizio di pompe funebri di McGillicutty",
	L"URL non ritrovato.",
	L"Bobby Ray - Spedizioni recenti",
	L"",
	L"",
};

const wchar_t* pShowBookmarkString[] =
{
	L"Aiuto",
	L"Cliccate su Rete un'altra volta per i segnalibri.",
};

const wchar_t* pLaptopTitles[] =
{
	L"Cassetta della posta",
	L"Gestione risorse",
	L"Personale",
	L"Contabile aggiuntivo",
	L"Ceppo storico",
};

const wchar_t* pPersonnelDepartedStateStrings[] =
{
	//reasons why a merc has left.
	L"Ucciso in azione",
	L"Licenziato",
	L"Sposato",
	L"Contratto Scaduto",
	L"Liberato",
};
// personnel strings appearing in the Personnel Manager on the laptop

const wchar_t* pPersonelTeamStrings[] =
{
	L"Squadra attuale",
	L"Partenze",
	L"Costo giornaliero:",
	L"Costo più alto:",
	L"Costo più basso:",
	L"Ucciso in azione:",
	L"Licenziato:",
	L"Altro:",
};


const wchar_t* pPersonnelCurrentTeamStatsStrings[] =
{
	L"Più basso",
	L"Normale",
	L"Più alto",
};


const wchar_t* pPersonnelTeamStatsStrings[] =
{
	L"SAL",
	L"AGI",
	L"DES",
	L"FOR",
	L"COM",
	L"SAG",
	L"LIV",
	L"TIR",
	L"MEC",
	L"ESP",
	L"PS",
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

const wchar_t pContractButtonString[] = L"Contratto";

// text that appears on the update panel buttons

const wchar_t* pUpdatePanelButtons[] =
{
	L"Continua",
	L"Fermati",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle

const wchar_t* const LargeTacticalStr[] =
{
	L"Siete stati sconfitti in questo settore!",
	L"Il nemico, non avendo alcuna pietà delle anime della squadra, divorerà ognuno di voi!",
	L"I membri inconscenti della vostra squadra sono stati catturati!",
	L"I membri della vostra squadra sono stati fatti prigionieri dal nemico.",
};


//Insurance Contract.c
//The text on the buttons at the bottom of the screen.

const wchar_t* InsContractText[] =
{
	L"Indietro",
	L"Avanti",
	L"Accetta",
	L"Pulisci",
};



//Insurance Info
// Text on the buttons on the bottom of the screen

const wchar_t* InsInfoText[] =
{
	L"Indietro",
	L"Avanti",
};



//For use at the M.E.R.C. web site. Text relating to the player's account with MERC

const wchar_t* MercAccountText[] =
{
	// Text on the buttons on the bottom of the screen
	L"Autorizza",
	L"Home Page",
	L"Conto #:",
	L"Merc",
	L"Giorni",
	L"Tasso",	//5
	L"Costo",
	L"Totale:",
	L"Conferma il pagamento di %ls?",		//the %ls is a string that contains the dollar amount ( ex. "$150" )
};



//For use at the M.E.R.C. web site. Text relating a MERC mercenary


const wchar_t* MercInfo[] =
{
	L"Indietro",
	L"Ricompensa",
	L"Successivo",
	L"Info. addizionali",
	L"Home Page",
	L"Assoldato",
	L"Salario:",
	L"Al giorno",
	L"Deceduto",

	L"Sembra che state arruolando troppi mercenari. Il vostro limite è di 18.",
	L"Non disponibile",
};



// For use at the M.E.R.C. web site. Text relating to opening an account with MERC

const wchar_t* MercNoAccountText[] =
{
	//Text on the buttons at the bottom of the screen
	L"Apri conto",
	L"Annulla",
	L"Non hai alcun conto. Vuoi aprirne uno?",
};



// For use at the M.E.R.C. web site. MERC Homepage

const wchar_t* MercHomePageText[] =
{
	//Description of various parts on the MERC page
	L"Speck T. Kline, fondatore e proprietario",
	L"Per aprire un conto, cliccate qui",
	L"Per visualizzare un conto, cliccate qui",
	L"Per visualizzare i file, cliccate qui",
	// The version number on the video conferencing system that pops up when Speck is talking
	L"Speck Com v3.2",
};

// For use at MiGillicutty's Web Page.

const wchar_t* sFuneralString[] =
{
	L"Impresa di pompe funebri di McGillicutty: Il dolore delle famiglie che hanno fornito il loro aiuto dal 1983.",
	L"Precedentemente mercenario dell'A.I.M. Murray \"Pops\" McGillicutty è un impresario di pompe funebri qualificato e con molta esperienza.",
	L"Essendo coinvolto profondamente nella morte e nel lutto per tutta la sua vita, Pops sa quanto sia difficile affrontarli.",
	L"L'impresa di pompe funebri di McGillicutty offre una vasta gamma di servizi funebri, da una spalla su cui piangere a ricostruzioni post-mortem per corpi mutilati o sfigurati.",
	L"Lasciate che l'impresa di pompe funebri di McGillicutty vi aiuti e i vostri amati riposeranno in pace.",

	// Text for the various links available at the bottom of the page
	L"SPEDISCI FIORI",
	L"ESPOSIZIONE DI BARE & URNE",
	L"SERVIZI DI CREMAZIONE",
	L"SERVIZI PRE-FUNEBRI",
	L"CERIMONIALE FUNEBRE",

	// The text that comes up when you click on any of the links ( except for send flowers ).
	L"Purtroppo, il resto di questo sito non è stato completato a causa di una morte in famiglia. Aspettando la lettura del testamento e la riscossione dell'eredità, il sito verrà ultimato non appena possibile.",
	L"Vi porgiamo, comunque, le nostre condolianze in questo momento di dolore. Contatteci ancora.",
};

// Text for the florist Home Page

const wchar_t* sFloristText[] =
{
	//Text on the button on the bottom of the page

	L"Galleria",

	//Address of United Florist

	L"\"Ci lanciamo col paracadute ovunque\"",
	L"1-555-SCENT-ME",
	L"333 Dot. NoseGay, Seedy City, CA USA 90210",
	L"http://www.scent-me.com",

	// detail of the florist page

	L"Siamo veloci ed efficienti!",
	L"Consegna il giorno successivo in quasi tutto il mondo, garantito. Applicate alcune restrizioni.",
	L"I prezzi più bassi in tutto il mondo, garantito!",
	L"Mostrateci un prezzo concorrente più basso per qualsiasi progetto, e riceverete una dozzina di rose, gratuitamente.",
	L"Flora, fauna & fiori in volo dal 1981.",
	L"I nostri paracadutisti decorati ex-bomber lanceranno il vostro bouquet entro un raggio di dieci miglia dalla locazione richiesta. Sempre e ovunque!",
	L"Soddisfiamo la vostra fantasia floreale.",
	L"Lasciate che Bruce, il nostro esperto in composizioni floreali, selezioni con cura i fiori più freschi e della migliore qualità dalle nostre serre più esclusive.",
	L"E ricordate, se non l'abbiamo, possiamo coltivarlo - E subito!",
};



//Florist OrderForm

const wchar_t* sOrderFormText[] =
{
	//Text on the buttons

	L"Indietro",
	L"Spedisci",
	L"Home",
	L"Galleria",

	L"Nome del bouquet:",
	L"Prezzo:",			//5
	L"Numero ordine:",
	L"Data consegna",
	L"gior. succ.",
	L"arriva quando arriva",
	L"Luogo consegna",			//10
	L"Servizi aggiuntivi",
	L"Bouquet schiacciato ($10)",
	L"Rose nere ($20)",
	L"Bouquet appassito ($10)",
	L"Torta di frutta (se disponibile 10$)",		//15
	L"Sentimenti personali:",
	L"Il vostro messaggio non può essere più lungo di 75 caratteri.",
	L"... oppure sceglietene uno dai nostri",

	L"BIGLIETTI STANDARD",
	L"Informazioni sulla fatturazione",//20

	//The text that goes beside the area where the user can enter their name

	L"Nome:",
};




//Florist Gallery.c

const wchar_t* sFloristGalleryText[] =
{
	//text on the buttons

	L"Prec.",	//abbreviation for previous
	L"Succ.",	//abbreviation for next

	L"Clicca sul modello che vuoi ordinare.",
	L"Ricorda: c'è un supplemento di 10$ per tutti i bouquet appassiti o schiacciati.",

	//text on the button

	L"Home",
};

//Florist Cards

const wchar_t* sFloristCards[] =
{
	L"Cliccate sulla vostra selezione",
	L"Indietro",
};



// Text for Bobby Ray's Mail Order Site

const wchar_t* BobbyROrderFormText[] =
{
	L"Ordine",				//Title of the page
	L"Qta",					// The number of items ordered
	L"Peso (%ls)",			// The weight of the item
	L"Nome oggetto",				// The name of the item
	L"Prezzo unit.",				// the item's weight
	L"Totale",				//5	// The total price of all of items of the same type
	L"Sotto-totale",				// The sub total of all the item totals added
	L"S&C (Vedete luogo consegna)",		// S&H is an acronym for Shipping and Handling
	L"Totale finale",			// The grand total of all item totals + the shipping and handling
	L"Luogo consegna",
	L"Spedizione veloce",			//10	// See below
	L"Costo (per %ls.)",			// The cost to ship the items
	L"Espresso di notte",			// Gets deliverd the next day
	L"2 giorni d'affari",			// Gets delivered in 2 days
	L"Servizio standard",			// Gets delivered in 3 days
	L"Annulla ordine",//15			// Clears the order page
	L"Accetta ordine",			// Accept the order
	L"Indietro",				// text on the button that returns to the previous page
	L"Home Page",				// Text on the button that returns to the home Page
	L"* Indica oggetti usati",		// Disclaimer stating that the item is used
	L"Non potete permettervi di pagare questo.",		//20	// A popup message that to warn of not enough money
	L"<NESSUNO>",				// Gets displayed when there is no valid city selected
	L"Siete sicuri di volere spedire quest'ordine a %ls?",		// A popup that asks if the city selected is the correct one
	L"peso del pacco**",			// Displays the weight of the package
	L"** Peso min.",				// Disclaimer states that there is a minimum weight for the package
	L"Spedizioni",
};


// This text is used when on the various Bobby Ray Web site pages that sell items

const wchar_t* BobbyRText[] =
{
	L"Ordini:",				// Title
	// instructions on how to order
	L"Cliccate sull'oggetto. Sinistro per aggiungere pezzi, destro per toglierne. Una volta selezionata la quantità, procedete col nuovo ordine.",

	//Text on the buttons to go the various links

	L"Oggetti prec.",		//
	L"Armi", 			//3
	L"Munizioni",			//4
	L"Giubb. A-P",			//5
	L"Varie",			//6	//misc is an abbreviation for miscellaneous
	L"Usato",			//7
	L"Oggetti succ.",
	L"ORDINE",
	L"Home Page",			//10

	//The following lines provide information on the items

	L"Peso:",		// Weight of all the items of the same type
	L"Cal.:",			// the caliber of the gun
	L"Mag.:",			// number of rounds of ammo the Magazine can hold
	L"Git.:",			// The range of the gun
	L"Dan.:",			// Damage of the weapon
	L"FFA:",			// Weapon's Rate Of Fire, acronym ROF
	L"Costo:",			// Cost of the item
	L"Inventario:",			// The number of items still in the store's inventory
	L"Num. ordine:",		// The number of items on order
	L"Danneggiato",			// If the item is damaged
	L"Totale:",			// The total cost of all items on order
	L"* funzionale al %",		// if the item is damaged, displays the percent function of the item

	//Popup that tells the player that they can only order 10 items at a time

	L"Darn! Quest'ordine qui accetterà solo 10 oggetti. Se avete intenzione di ordinare più merce (ed è quello che speriamo), fate un ordine a parte e accettate le nostre scuse.",

	// A popup that tells the user that they are trying to order more items then the store has in stock

	L"Ci dispiace. Non disponiamo più di questo articolo. Riprovate più tardi.",

	//A popup that tells the user that the store is temporarily sold out

	L"Ci dispiace, ma siamo momentaneamente sprovvisti di oggetti di questo genere.",
};


/* The following line is used on the Ammunition page.  It is used for help text
 * to display how many items the player's merc has that can use this type of
 * ammo. */
const wchar_t str_bobbyr_guns_num_guns_that_use_ammo[] = L"La vostra squadra ha %d arma(i) che usa(no) questo tipo di munizioni";


// Text for Bobby Ray's Home Page

const wchar_t* BobbyRaysFrontText[] =
{
	//Details on the web site

	L"Questo è il negozio con la fornitura militare e le armi più recenti e potenti!",
	L"Possiamo trovare la soluzione perfetta per tutte le vostre esigenze riguardo agli esplosivi.",
	L"Oggetti usati e riparati",

	//Text for the various links to the sub pages

	L"Varie",
	L"ARMI",
	L"MUNIZIONI",		//5
	L"GIUBB. A-P",

	//Details on the web site

	L"Se non lo vendiamo, non potrete averlo!",
	L"In costruzione",
};



// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug shot page

const wchar_t* AimSortText[] =
{
	L"Membri dell'A.I.M.",				// Title
	// Title for the way to sort
	L"Ordine per:",

	//Text of the links to other AIM pages

	L"Visualizza le facce dei mercenari disponibili",
	L"Rivedi il file di ogni singolo mercenario",
	L"Visualizza la galleria degli associati dell'A.I.M."
};


// text to display how the entries will be sorted
const wchar_t str_aim_sort_price[]        = L"Prezzo";
const wchar_t str_aim_sort_experience[]   = L"Esperienza";
const wchar_t str_aim_sort_marksmanship[] = L"Mira";
const wchar_t str_aim_sort_medical[]      = L"Pronto socc.";
const wchar_t str_aim_sort_explosives[]   = L"Esplosivi";
const wchar_t str_aim_sort_mechanical[]   = L"Meccanica";
const wchar_t str_aim_sort_ascending[]    = L"Crescente";
const wchar_t str_aim_sort_descending[]   = L"Decrescente";


//Aim Policies.c
//The page in which the AIM policies and regulations are displayed

const wchar_t* AimPolicyText[] =
{
	// The text on the buttons at the bottom of the page

	L"Indietro",
	L"Home Page",
	L"Indice",
	L"Avanti",
	L"Disaccordo",
	L"Accordo",
};



//Aim Member.c
//The page in which the players hires AIM mercenaries

// Instructions to the user to either start video conferencing with the merc, or to go the mug shot index

const wchar_t* AimMemberText[] =
{
	L"Clic sinistro",
	L"per contattarlo",
	L"Clic destro",
	L"per i mercenari disponibili.",
};

//Aim Member.c
//The page in which the players hires AIM mercenaries

const wchar_t* CharacterInfo[] =
{
	// the contract expenses' area

	L"Paga",
	L"Durata",
	L"1 giorno",
	L"1 settimana",
	L"2 settimane",

	// text for the buttons that either go to the previous merc,
	// start talking to the merc, or go to the next merc

	L"Indietro",
	L"Contratto",
	L"Avanti",

	L"Ulteriori informazioni",				// Title for the additional info for the merc's bio
	L"Membri attivi", // Title of the page
	L"Dispositivo opzionale:",				// Displays the optional gear cost
	L"Deposito MEDICO richiesto",			// If the merc required a medical deposit, this is displayed
};


//Aim Member.c
//The page in which the player's hires AIM mercenaries

//The following text is used with the video conference popup

const wchar_t* VideoConfercingText[] =
{
	L"Costo del contratto:",				//Title beside the cost of hiring the merc

	//Text on the buttons to select the length of time the merc can be hired

	L"1 giorno",
	L"1 settimana",
	L"2 settimane",

	//Text on the buttons to determine if you want the merc to come with the equipment

	L"Nessun equip.",
	L"Compra equip.",

	// Text on the Buttons

	L"TRASFERISCI FONDI",			// to actually hire the merc
	L"ANNULLA",				// go back to the previous menu
	L"ARRUOLA",				// go to menu in which you can hire the merc
	L"TACI",				// stops talking with the merc
	L"OK",
	L"LASCIA MESSAGGIO",			// if the merc is not there, you can leave a message

	//Text on the top of the video conference popup

	L"Videoconferenza con",
	L"Connessione...",

	L"con deposito"			// Displays if you are hiring the merc with the medical deposit
};



//Aim Member.c
//The page in which the player hires AIM mercenaries

// The text that pops up when you select the TRANSFER FUNDS button

const wchar_t* AimPopUpText[] =
{
	L"TRASFERIMENTO ELETTRONICO FONDI RIUSCITO",	// You hired the merc
	L"NON IN GRADO DI TRASFERIRE",		// Player doesn't have enough money, message 1
	L"FONDI INSUFFICIENTI",				// Player doesn't have enough money, message 2

	// if the merc is not available, one of the following is displayed over the merc's face

	L"In missione",
	L"Lascia messaggio",
	L"Deceduto",

	//If you try to hire more mercs than game can support

	L"Avete già una squadra di 18 mercenari.",

	L"Messaggio già registrato",
	L"Messaggio registrato",
};


//AIM Link.c

const wchar_t AimLinkText[] = L"Collegamenti dell'A.I.M."; // The title of the AIM links page



//Aim History

// This page displays the history of AIM

const wchar_t* AimHistoryText[] =
{
	L"Storia dell'A.I.M.",					//Title

	// Text on the buttons at the bottom of the page

	L"Indietro",
	L"Home Page",
	L"Associati",
	L"Avanti",
};


//Aim Mug Shot Index

//The page in which all the AIM members' portraits are displayed in the order selected by the AIM sort page.

const wchar_t* AimFiText[] =
{
	// displays the way in which the mercs were sorted

	L"Prezzo",
	L"Esperienza",
	L"Mira",
	L"Pronto socc.",
	L"Esplosivi",
	L"Meccanica",

	// The title of the page, the above text gets added at the end of this text

	L"Membri scelti dell'A.I.M. in ordine crescente secondo %ls",
	L"Membri scelti dell'A.I.M. in ordine decrescente secondo %ls",

	// Instructions to the players on what to do

	L"Clic sinistro",
	L"Per scegliere un mercenario.",			//10
	L"Clic destro",
	L"Per selezionare opzioni",

	// Gets displayed on top of the merc's portrait if they are...

	L"Deceduto",						//14
	L"In missione",
};



//AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer with AIM

const wchar_t* AimAlumniText[] =
{
	// Text of the buttons

	L"PAGINA 1",
	L"PAGINA 2",
	L"PAGINA 3",

	L"Membri dell'A.I.M.",	// Title of the page

	L"FINE"			// Stops displaying information on selected merc
};






//AIM Home Page

const wchar_t* AimScreenText[] =
{
	// AIM disclaimers

	L"A.I.M. e il logo A.I.M. sono marchi registrati in diversi paesi.",
	L"Di conseguenza, non cercate di copiarci.",
	L"Copyright 1998-1999 A.I.M., Ltd. Tutti i diritti riservati.",

	//Text for an advertisement that gets displayed on the AIM page

	L"Servizi riuniti floreali",
	L"\"Atterriamo col paracadute ovunque\"",				//10
	L"Fallo bene",
	L"... la prima volta",
	L"Se non abbiamo armi e oggetti, non ne avrete bisogno.",
};


//Aim Home Page

const wchar_t* AimBottomMenuText[] =
{
	//Text for the links at the bottom of all AIM pages
	L"Home Page",
	L"Membri",
	L"Associati",
	L"Assicurazioni",
	L"Storia",
	L"Collegamenti",
};



//ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.

const wchar_t* SKI_Text[ ] =
{
	L"MERCANZIA IN STOCK",		//Header for the merchandise available
	L"PAGINA",				//The current store inventory page being displayed
	L"COSTO TOTALE",				//The total cost of the the items in the Dealer inventory area
	L"VALORE TOTALE",			//The total value of items player wishes to sell
	L"STIMATO",				//Button text for dealer to evaluate items the player wants to sell
	L"TRANSAZIONE",			//Button text which completes the deal. Makes the transaction.
	L"FINE",				//Text for the button which will leave the shopkeeper interface.
	L"COSTO DI RIPARAZIONE",			//The amount the dealer will charge to repair the merc's goods
	L"1 ORA",			// SINGULAR! The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"%d ORE",		// PLURAL!   The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"RIPARATO",		// Text appearing over an item that has just been repaired by a NPC repairman dealer
	L"Non c'è abbastanza spazio nel vostro margine di ordine.",	//Message box that tells the user there is no more room to put there stuff
	L"%d MINUTI",		// The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"Lascia oggetto a terra.",
};


const wchar_t* SkiMessageBoxText[] =
{
	L"Volete sottrarre %ls dal vostro conto principale per coprire la differenza?",
	L"Fondi insufficienti. Avete pochi %ls",
	L"Volete sottrarre %ls dal vostro conto principale per coprire la spesa?",
	L"Rivolgetevi all'operatore per iniziare la transazione",
	L"Rivolgetevi all'operatore per riparare gli oggetti selezionati",
	L"Fine conversazione",
	L"Bilancio attuale",
};


//OptionScreen.c

const wchar_t* zOptionsText[] =
{
	//button Text
	L"Salva partita",
	L"Carica partita",
	L"Abbandona",
	L"Fine",

	//Text above the slider bars
	L"Effetti",
	L"Parlato",
	L"Musica",

	//Confirmation pop when the user selects..
	L"Volete terminare la partita e tornare al menu principale?",

	L"Avete bisogno dell'opzione 'Parlato' o di quella 'Sottotitoli' per poter giocare.",
};


//SaveLoadScreen
const wchar_t* zSaveLoadText[] =
{
	L"Salva partita",
	L"Carica partita",
	L"Annulla",
	L"Salvamento selezionato",
	L"Caricamento selezionato",

	L"Partita salvata con successo",
	L"ERRORE durante il salvataggio!",
	L"Partita caricata con successo",
	L"ERRORE durante il caricamento: \"%hs\"",

	L"La versione del gioco nel file della partita salvata è diverso dalla versione attuale. È abbastanza sicuro proseguire. Continuate?",
	L"I file della partita salvata potrebbero essere annullati. Volete cancellarli tutti?",

	//Translators, the next two strings are for the same thing.  The first one is for beta version releases and the second one
	//is used for the final version.  Please don't modify the "#ifdef JA2BETAVERSION" or the "#else" or the "#endif" as they are
	//used by the compiler and will cause program errors if modified/removed.  It's okay to translate the strings though.
#ifdef JA2BETAVERSION
	L"La versionbe salvata è cambiata. Fateci avere un report, se incontrate problemi. Continuate?",
#else
	L"Tentativo di caricare una versione salvata più vecchia. Aggiornate e caricate automaticamente quella salvata?",
#endif

	//Translators, the next two strings are for the same thing.  The first one is for beta version releases and the second one
	//is used for the final version.  Please don't modify the "#ifdef JA2BETAVERSION" or the "#else" or the "#endif" as they are
	//used by the compiler and will cause program errors if modified/removed.  It's okay to translate the strings though.
#ifdef JA2BETAVERSION
	L"La versione salvata e la versione della partita sono cambiate. Fateci avere un report, se incontrate problemi. Continuate?",
#else
	L"Tentativo di caricare una vecchia versione salvata. Aggiornate e caricate automaticamente quella salvata?",
#endif

	L"Siete sicuri di volere sovrascrivere la partita salvata nello slot #%d?",


	//The first %d is a number that contains the amount of free space on the users hard drive,
	//the second is the recommended amount of free space.
	L"Lo spazio su disco si sta esaurendo. Sono disponibili solo %d MB, mentre per giocare a Jagged dovrebbero esserci almeno %d MB liberi .",

	L"Salvataggio in corso...",			//When saving a game, a message box with this string appears on the screen

	L"Armi normali",
	L"Tonn. di armi",
	L"Stile realistico",
	L"Stile fantascientifico",

	L"Difficoltà",
};



//MapScreen
const wchar_t* zMarksMapScreenText[] =
{
	L"Livello mappa",
	L"Non avete soldati. Avete bisogno di addestrare gli abitanti della città per poter disporre di un esercito cittadino.",
	L"Entrata giornaliera",
	L"Il mercenario ha l'assicurazione sulla vita",
	L"%ls non è stanco.",
	L"%ls si sta muovendo e non può riposare",
	L"%ls è troppo stanco, prova un po' più tardi.",
	L"%ls sta guidando.",
	L"La squadra non può muoversi, se un mercenario dorme.",

	// stuff for contracts
	L"Visto che non potete pagare il contratto, non avete neanche i soldi per coprire il premio dell'assicurazione sulla vita di questo nercenario.",
	L"%ls premio dell'assicurazione costerà %ls per %d giorno(i) extra. Volete pagare?",
	L"Settore inventario",
	L"Il mercenario ha una copertura medica.",

	// other items
	L"Medici", // people acting a field medics and bandaging wounded mercs
	L"Pazienti", // people who are being bandaged by a medic
	L"Fine", // Continue on with the game after autobandage is complete
	L"Ferma", // Stop autobandaging of patients by medics now
	L"%ls non ha un kit di riparazione.",
	L"%ls non ha un kit di riparazione.",
	L"Non ci sono abbastanza persone che vogliono essere addestrate ora.",
	L"%ls è pieno di soldati.",
	L"Il mercenario ha un contratto a tempo determinato.",
  L"Il contratto del mercenario non è assicurato",
};


const wchar_t pLandMarkInSectorString[] = L"La squadra %d ha notato qualcuno nel settore %ls";

// confirm the player wants to pay X dollars to build a militia force in town
const wchar_t* pMilitiaConfirmStrings[] =
{
	L"Addestrare una squadra dell'esercito cittadino costerà $", // telling player how much it will cost
	L"Approvate la spesa?", // asking player if they wish to pay the amount requested
	L"Non potete permettervelo.", // telling the player they can't afford to train this town
	L"Continuate ad aeddestrare i soldati in %ls (%ls %d)?", // continue training this town?
	L"Costo $", // the cost in dollars to train militia
	L"(S/N)",   // abbreviated yes/no
	L"Addestrare l'esrecito cittadino nei settori di %d costerà $ %d. %ls", // cost to train sveral sectors at once
	L"Non potete permettervi il $%d per addestrare l'esercito cittadino qui.",
	L"%ls ha bisogno di una percentuale di %d affinché possiate continuare ad addestrare i soldati.",
	L"Non potete più addestrare i soldati a %ls.",
};

#ifdef JA2DEMOADS
const wchar_t* gpDemoString[] =
{
	//0-9
	L"IL MEGLIO DELLA STRATEGIA.",
	L"IL MEGLIO DEI GIOCHI DI RUOLO.",
	L"Gioco non completamente lineare",
	L"Nuova trama estrema",
	L"Incendi estremamente realistici",
	L"Luci dinamiche",
	L"Interazione NPC",
	L"Tattiche di furto e notturne",
	L"Personalizzate il vostro mercenario",
	L"Sfide e avventure creative",
	L"Veicoli controllabili",
	//10-19
	L"Più di 150 personalità",
	L"Centinaia di oggetti fantastici",
	L"Più di 250 locazioni uniche",
	L"Paurose esplosioni",
	L"Nuove manovre tattiche",
	L"Gran quantità di animazione",
	L"Più di 9000 righe di dialogo",
	L"Stupendo tema musicale di Kevin Manthei",
	L"Strategia globale",
	L"Battaglia e attacchi aerei",
	//20-29
	L"Allenate il vostro esercito",
	L"Gioco personalizzabile",
	L"Linea di vista realistica",
	L"Sorprendente grafica a colori",
	L"Comprare, vendere e commerciare",
	L"Nemico crudele AI",
	L"Tattiche di combattimento allo stato dell'arte",
	L"Elementi opzionali di fantascienza",
	L"24 ore",
	L"Ben più di 60 ore di gioco",
	//30-34
	L"Ambientazioni di varie difficoltà",
	L"Modelli balistici realistici",
	L"Accurato studio della fisica",
	L"Ambiente completamente interattivo",
	L"E altro ancora...",
	//35 on are BOLD
	L"",
	L"",
	L"\"Uno dei giochi più creativi del 1998\"",
	L"Giocatore PC",
	L"Fate subito il vostro ordine al sito WWW.JAGGEDALLIANCE2.COM!",
};

const wchar_t* gpDemoIntroString[] =
{
	L"State per sperimentare il meglio della strategia, dei giochi di ruolo e del combattimento tattico:",
	//Point 1 (uses one string)
	L"Controllate una squadra di mercenari con una spiccata personalità (nessuno di loro vi annoierà).",
	//Point 2 (uses one string)
	L"Controllate lo strabiliante schieramento di manovre tattiche: correre, strisciare, scalare, saltare, combattimenti corpo a corpo e altro ancora.",
	//Point 3 (uses one string)
	L"Scherzate col fuoco! Giochi fantastici, armi assassine, esplosioni paurose.",
	//Additional comment
	L"(e questo è solo l'inizio)",
	//Introduction/instructions
	L"Benvenuti a Demoville... (premete un tasto per continuare)",
};
#endif

//Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom of the single merc panel
const wchar_t* gzMoneyWithdrawMessageText[] =
{
	L"Potete prelevare solo fino a $20,000 alla volta.",
	L"Sieti sicuri di voler depositare il %ls sul vostro conto?",
};

const wchar_t gzCopyrightText[] = L"Copyright (C) 1999 Sir-tech Canada Ltd. Tutti i diritti riservati.";

//option Text
const wchar_t* zOptionsToggleText[] =
{
	L"Parlato",
	L"Conferme mute",
	L"Sottotitoli",
	L"Mettete in pausa il testo del dialogo",
	L"Fumo dinamico",
	L"Sangue e violenza",
	L"Non è necessario usare il mouse!",
	L"Vecchio metodo di selezione",
	L"Mostra il percorso dei mercenari",
	L"Mostra traiettoria colpi sbagliati",
	L"Conferma in tempo reale",
	L"Visualizza gli avvertimenti sveglio/addormentato",
	L"Utilizza il sistema metrico",
	L"Tragitto illuminato durante gli spostamenti",
	L"Sposta il cursore sui mercenari",
	L"Sposta il cursore sulle porte",
	L"Evidenzia gli oggetti",
	L"Mostra le fronde degli alberi",
	L"Mostra strutture",
	L"Mostra il cursore 3D",
};

//This is the help text associated with the above toggles.
const wchar_t* zOptionsScreenHelpText[] =
{
	//speech
	L"Attivate questa opzione, se volete ascoltare il dialogo dei personaggi.",

	//Mute Confirmation
	L"Attivate o disattivate le conferme verbali dei personaggi.",

		//Subtitles
	L"Controllate se il testo su schermo viene visualizzato per il dialogo.",

	//Key to advance speech
	L"Se i sottotitoli sono attivati, utilizzate questa opzione per leggere tranquillamente i dialoghi NPC.",

	//Toggle smoke animation
	L"Disattivate questa opzione, se il fumo dinamico diminuisce la frequenza d'aggiornamento.",

	//Blood n Gore
	L"Disattivate questa opzione, se il sangue vi disturba.",

	//Never move my mouse
	L"Disattivate questa opzione per muovere automaticamente il mouse sulle finestre a comparsa di conferma al loro apparire.",

	//Old selection method
	L"Attivate questa opzione per selezionare i personaggi e muoverli come nel vecchio JA (dato che la funzione è stata invertita).",

	//Show movement path
	L"Attivate questa opzione per visualizzare i sentieri di movimento in tempo reale (oppure disattivatela utilizzando il tasto MAIUSC).",

	//show misses
	L"Attivate per far sì che la partita vi mostri dove finiscono i proiettili quando \"sbagliate\".",

	//Real Time Confirmation
	L"Se attivata, sarà richiesto un altro clic su \"salva\" per il movimento in tempo reale.",

	//Sleep/Wake notification
  L"Se attivata, verrete avvisati quando i mercenari in \"servizio\" vanno a riposare e quando rientrano in servizio.",

	//Use the metric system
	L"Se attivata, utilizza il sistema metrico di misurazione; altrimenti ricorre al sistema britannico.",

	//Merc Lighted movement
	L"Se attivata, il mercenario mostrerà il terreno su cui cammina. Disattivatela per un aggiornamento più veloce.",

	//Smart cursor
	L"Se attivata, muovendo il cursore vicino ai vostri mercenari li evidenzierà automaticamente.",

	//snap cursor to the door
	L"Se attivata, muovendo il cursore vicino a una porta farà posizionare automaticamente il cursore sopra di questa.",

	//glow items
	L"Se attivata, l'opzione evidenzierà gli |Oggetti automaticamente.",

	//toggle tree tops
	L"Se attivata, mostra le |fronde degli alberi.",

	//toggle wireframe
	L"Se attivata, visualizza le |Strutture dei muri nascosti.",

	L"Se attivata, il cursore di movimento verrà mostrato in 3D (|H|o|m|e).",

};


const wchar_t* gzGIOScreenText[] =
{
	L"INSTALLAZIONE INIZIALE DEL GIOCO",
	L"Versione del gioco",
	L"Realistica",
	L"Fantascientifica",
	L"Opzioni delle armi",
	L"Varietà di armi",
	L"Normale",
	L"Livello di difficoltà",
	L"Principiante",
	L"Esperto",
	L"Professionista",
	L"Ok",
	L"Annulla",
	L"Difficoltà extra",
	L"Tempo illimitato",
	L"Turni a tempo"
};

const wchar_t* pDeliveryLocationStrings[] =
{
	L"Austin",			//Austin, Texas, USA
	L"Baghdad",			//Baghdad, Iraq (Suddam Hussein's home)
	L"Drassen",			//The main place in JA2 that you can receive items.  The other towns are dummy names...
	L"Hong Kong",		//Hong Kong, Hong Kong
	L"Beirut",			//Beirut, Lebanon	(Middle East)
	L"Londra",			//London, England
	L"Los Angeles",	//Los Angeles, California, USA (SW corner of USA)
	L"Meduna",			//Meduna -- the other airport in JA2 that you can receive items.
	L"Metavira",		//The island of Metavira was the fictional location used by JA1
	L"Miami",				//Miami, Florida, USA (SE corner of USA)
	L"Mosca",			//Moscow, USSR
	L"New York",		//New York, New York, USA
	L"Ottawa",			//Ottawa, Ontario, Canada -- where JA2 was made!
	L"Parigi",				//Paris, France
	L"Tripoli",			//Tripoli, Libya (eastern Mediterranean)
	L"Tokyo",				//Tokyo, Japan
	L"Vancouver",		//Vancouver, British Columbia, Canada (west coast near US border)
};

/* This string is used in the IMP character generation.  It is possible to
 * select 0 ability in a skill meaning you can't use it.  This text is
 * confirmation to the player. */
const wchar_t pSkillAtZeroWarning[]    = L"Siete sicuri? Un valore di zero significa NESSUNA abilità.";
const wchar_t pIMPBeginScreenStrings[] = L"(max 8 personaggi)";
const wchar_t pIMPFinishButtonText[]   = L"Analisi";
const wchar_t pIMPFinishStrings[]      = L"Grazie, %ls"; //%ls is the name of the merc
const wchar_t pIMPVoicesStrings[]      = L"Voce"; // the strings for imp voices screen

// title for program
const wchar_t pPersTitleText[] = L"Manager del personale";

// paused game strings
const wchar_t* pPausedGameText[] =
{
	L"Partita in pausa",
	L"Riprendi la partita (|P|a|u|s|a)",
	L"Metti in pausa la partita (|P|a|u|s|a)",
};


const wchar_t* pMessageStrings[] =
{
	L"Vuoi uscire dalla partita?",
	L"OK",
	L"SÌ",
	L"NO",
	L"ANNULLA",
	L"RIASSUMI",
	L"MENTI",
	L"Nessuna descrizione", //Save slots that don't have a description.
	L"Partita salvata.",
	L"Giorno",
	L"Mercenari",
	L"Slot vuoto", //An empty save game slot
	L"ppm",					//Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
	L"dm",					//Abbreviation for minute.
	L"m",						//One character abbreviation for meter (metric distance measurement unit).
	L"colpi",				//Abbreviation for rounds (# of bullets)
	L"kg",					//Abbreviation for kilogram (metric weight measurement unit)
	L"lb",					//Abbreviation for pounds (Imperial weight measurement unit)
	L"Home Page",				//Home as in homepage on the internet.
	L"USD",					//Abbreviation to US dollars
	L"n/a",					//Lowercase acronym for not applicable.
	L"In corso",		//Meanwhile
	L"%ls si trova ora nel settore %ls%ls", //Name/Squad has arrived in sector A9.  Order must not change without notifying
																		//SirTech
	L"Versione",
	L"Slot di salvataggio rapido vuoto",
	L"Questo slot è riservato ai salvataggi rapidi fatti dalle schermate tattiche e dalla mappa utilizzando ALT+S.",
	L"Aperto",
	L"Chiuso",
	L"Lo spazio su disco si sta esaurendo. Avete liberi solo %ls MB e Jagged Alliance 2 ne richiede %ls.",
	L"%ls ha preso %ls.",		//'Merc name' has caught 'item' -- let SirTech know if name comes after item.
	L"%ls ha assunto della droga.", //'Merc name' has taken the drug
	L"%ls non ha alcuna abilità medica",//'Merc name' has no medical skill.

	//CDRom errors (such as ejecting CD while attempting to read the CD)
	L"L'integrità del gioco è stata compromessa.",
	L"ERRORE: CD-ROM non valido",

	//When firing heavier weapons in close quarters, you may not have enough room to do so.
	L"Non c'è spazio per sparare da qui.",

	//Can't change stance due to objects in the way...
	L"Non potete cambiare posizione questa volta.",

	//Simple text indications that appear in the game, when the merc can do one of these things.
	L"Fai cadere",
	L"Getta",
	L"Passa",

	L"%ls è passato a %ls.", //"Item" passed to "merc".  Please try to keep the item %ls before the merc %ls, otherwise,
											 //must notify SirTech.
	L"Nessun spazio per passare %ls a %ls.", //pass "item" to "merc".  Same instructions as above.

	//A list of attachments appear after the items.  Ex:  Kevlar vest (Ceramic Plate 'Attached)'
	L" Compreso)",

	//Cheat modes
	L"Raggiunto il livello Cheat UNO",
	L"Raggiunto il livello Cheat DUE",

	//Toggling various stealth modes
	L"Squadra in modalità furtiva.",
	L"Squadra non in modalità furtiva.",
	L"%ls in modalità furtiva.",
	L"%ls non in modalità furtiva.",

	//Wireframes are shown through buildings to reveal doors and windows that can't otherwise be seen in
	//an isometric engine.  You can toggle this mode freely in the game.
	L"Strutture visibili",
	L"Strutture nascoste",

	//These are used in the cheat modes for changing levels in the game.  Going from a basement level to
	//an upper level, etc.
	L"Non potete passare al livello superiore...",
	L"Non esiste nessun livello inferiore...",
	L"Entra nel seminterrato %d...",
	L"Abbandona il seminterrato...",

	L"di",		// used in the shop keeper inteface to mark the ownership of the item eg Red's gun
	L"Modalità segui disattiva.",
	L"Modalità segui attiva.",
	L"Cursore 3D disattivo.",
	L"Cursore 3D attivo.",
	L"Squadra %d attiva.",
	L"Non potete permettervi di pagare a %ls un salario giornaliero di %ls",	//first %ls is the mercs name, the seconds is a string containing the salary
	L"Salta",
	L"%ls non può andarsene da solo.",
	L"Un salvataggio è stato chiamato SaveGame99.sav. Se necessario, rinominatelo da SaveGame01 a SaveGame10 e così potrete accedervi nella schermata di caricamento.",
	L"%ls ha bevuto del %ls",
	L"Un pacco è arivato a Drassen.",
 	L"%ls dovrebbe arrivare al punto designato di partenza (settore %ls) nel giorno %d, approssimativamente alle ore %ls.",		//first %ls is mercs name, next is the sector location and name where they will be arriving in, lastely is the day an the time of arrival
	L"Registro aggiornato.",
#ifdef JA2BETAVERSION
	L"Salvataggio riuscito della partita nello slot End Turn Auto Save.",
#endif
};


const wchar_t ItemPickupHelpPopup[][40] =
{
	L"OK",
	L"Scorrimento su",
	L"Seleziona tutto",
	L"Scorrimento giù",
	L"Annulla",
};

const wchar_t* pDoctorWarningString[] =
{
	L"%ls non è abbstanza vicina per poter esser riparata.",
	L"I vostri medici non sono riusciti a bendare completamente tutti.",
};

const wchar_t* pMilitiaButtonsHelpText[] =
{
	L"Raccogli (Clicca di destro)/lascia (Clicca di sinistro) le truppe verdi", // button help text informing player they can pick up or drop militia with this button
	L"Raccogli (Clicca di destro)/lascia (Clicca di sinistro) le truppe regolari",
	L"Raccogli (Clicca di destro)/lascia (Clicca di sinistro) le truppe veterane",
	L"Distribuisci equamente i soldati disponibili tra i vari settori",
};

// to inform the player to hire some mercs to get things going
const wchar_t pMapScreenJustStartedHelpText[] = L"Andate all'A.I.M. e arruolate alcuni mercenari (*Hint* è nel Laptop)";

const wchar_t pAntiHackerString[] = L"Errore. File mancanti o corrotti. Il gioco verrà completato ora.";


const wchar_t* gzLaptopHelpText[] =
{
	//Buttons:
	L"Visualizza E-mail",
	L"Siti web",
	L"Visualizza file e gli attach delle E-mail",
	L"Legge il registro degli eventi",
	L"Visualizza le informazioni inerenti la squadra",
	L"Visualizza la situazione finanziaria e la storia",
	L"Chiude laptop",

	//Bottom task bar icons (if they exist):
	L"Avete nuove E-mail",
	L"Avete nuovi file",

	//Bookmarks:
	L"Associazione Internazionale Mercenari",
	L"Ordinativi di armi online dal sito di Bobby Ray",
	L"Istituto del Profilo del Mercenario",
	L"Centro più economico di reclutamento",
	L"Impresa di pompe funebri McGillicutty",
	L"Servizio Fioristi Riuniti",
	L"Contratti assicurativi per agenti A.I.M.",
};


const wchar_t gzHelpScreenText[] = L"Esci dalla schermata di aiuto";

const wchar_t* gzNonPersistantPBIText[] =
{
	L"È in corso una battaglia. Potete solo ritirarvi dalla schermata delle tattiche.",
	L"|Entra nel settore per continuare l'attuale battaglia in corso.",
	L"|Automaticamente decide l'esito della battaglia in corso.",
	L"Non potete decidere l'esito della battaglia in corso automaticamente, se siete voi ad attaccare.",
	L"Non potete decidere l'esito della battaglia in corso automaticamente, se subite un'imboscata.",
	L"Non potete decidere l'esito della battaglia in corso automaticamente, se state combattendo contro le creature nelle miniere.",
	L"Non potete decidere l'esito della battaglia in corso automaticamente, se ci sono civili nemici.",
	L"Non potete decidere l'esito della battaglia in corso automaticamente, se ci sono dei Bloodcat.",
	L"BATTAGLIA IN CORSO",
	L"Non potete ritirarvi ora.",
};

const wchar_t* gzMiscString[] =
{
	L"I vostri soldati continuano a combattere senza l'aiuto dei vostri mercenari...",
	L"Il veicolo non ha più bisogno di carburante.",
	L"La tanica della benzina è piena %d%%.",
	L"L'esercito di Deidrannaha riguadagnato il controllo completo su %ls.",
	L"Avete perso una stazione di rifornimento.",
};

const wchar_t gzIntroScreen[] = L"Video introduttivo non trovato";

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
const wchar_t* pNewNoiseStr[] =
{
	L"%ls sente un %ls rumore proveniente da %ls.",
	L"%ls sente un %ls rumore di MOVIMENTO proveniente da %ls.",
	L"%ls sente uno %ls SCRICCHIOLIO proveniente da %ls.",
	L"%ls sente un %ls TONFO NELL'ACQUA proveniente da %ls.",
	L"%ls sente un %ls URTO proveniente da %ls.",
	L"%ls sente una %ls ESPLOSIONE verso %ls.",
	L"%ls sente un %ls URLO verso %ls.",
	L"%ls sente un %ls IMPATTO verso %ls.",
	L"%ls sente un %ls IMPATTO a %ls.",
	L"%ls sente un %ls SCHIANTO proveniente da %ls.",
	L"%ls sente un %ls FRASTUONO proveniente da %ls.",
};

const wchar_t* wMapScreenSortButtonHelpText[] =
{
	L"Nome (|F|1)",
	L"Assegnato (|F|2)",
	L"Tipo di riposo (|F|3)",
	L"Postazione (|F|4)",
	L"Destinazione (|F|5)",
	L"Durata dell'incarico (|F|6)",
};



const wchar_t* BrokenLinkText[] =
{
	L"Errore 404",
	L"Luogo non trovato.",
};


const wchar_t* gzBobbyRShipmentText[] =
{
	L"Spedizioni recenti",
	L"Ordine #",
	L"Numero di oggetti",
	L"Ordinato per",
};


const wchar_t* gzCreditNames[]=
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


const wchar_t* gzCreditNameTitle[]=
{
	L"Programmatore del gioco", 			// Chris Camfield
	L"Co-designer / Autore",							// Shaun Lyng
	L"Programmatore sistemi strategici & Editor",					//Kris Marnes
	L"Produttore / Co-designer",						// Ian Currie
	L"Co-designer / Designer della mappa",				// Linda Currie
	L"Grafico",													// Eric \"WTF\" Cheng
	L"Coordinatore beta, supporto",				// Lynn Holowka
	L"Grafico straordinario",						// Norman \"NRG\" Olsen
	L"Guru dell'audio",											// George Brooks
	L"Designer delle schermate / Grafico",					// Andrew Stacey
	L"Capo grafico / Animatore",						// Scot Loving
	L"Capo programmatore",									// Andrew \"Big Cheese Doddle\" Emmons
	L"Programmatore",											// Dave French
	L"Programmatore sistemi & bilancio di gioco",					// Alex Meduna
	L"Grafico dei ritratti",								// Joey \"Joeker\" Whelan",
};

const wchar_t* gzCreditNameFunny[]=
{
	L"", 																			// Chris Camfield
	L"(deve ancora esercitarsi con la punteggiatura)",					// Shaun Lyng
	L"(\"Fatto. Devo solo perfezionarmi\")",	//Kris \"The Cow Rape Man\" Marnes
	L"(sta diventando troppo vecchio per questo)",				// Ian Currie
	L"(sta lavorando a Wizardry 8)",						// Linda Currie
	L"(obbligato a occuparsi anche del CQ)",			// Eric \"WTF\" Cheng
	L"(ci ha lasciato per CFSA...)",	// Lynn Holowka
	L"",																			// Norman \"NRG\" Olsen
	L"",																			// George Brooks
	L"(Testa matta e amante del jazz)",						// Andrew Stacey
	L"(il suo nome vero è Robert)",							// Scot Loving
	L"(l'unica persona responsabile)",					// Andrew \"Big Cheese Doddle\" Emmons
	L"(può ora tornare al motocross)",	// Dave French
	L"(rubato da Wizardry 8)",							// Alex Meduna
	L"",	// Joey \"Joeker\" Whelan",
};

const wchar_t* sRepairsDoneString[] =
{
	L"%ls ha finito di riparare gli oggetti",
	L"%ls ha finito di riparare le armi e i giubbotti antiproiettile di tutti",
	L"%ls ha finito di riparare gli oggetti dell'equipaggiamento di tutti",
	L"%ls ha finito di riparare gli oggetti trasportati di tutti",
};

const wchar_t* zGioDifConfirmText[]=
{
	//L"You have chosen NOVICE mode. This setting is appropriate for those new to Jagged Alliance, those new to strategy games in general, or those wishing shorter battles in the game. Your choice will affect things throughout the entire course of the game, so choose wisely. Are you sure you want to play in Novice mode?",
	L"Avete selezionato la modalità PRINCIPIANTE. Questo scenario è adatto a chi gioca per la prima volta a Jagged Alliance, a chi prova a giocare per la prima volta in generale o a chi desidera combattere battaglie più brevi nel gioco. La vostra decisione influirà sull'intero corso della partita; scegliete, quindi, con attenzione. Siete sicuri di voler giocare nella modalità PRINCIPIANTE?",

	//L"You have chosen EXPERIENCED mode. This setting is suitable for those already familiar with Jagged Alliance or similar games. Your choice will affect things throughout the entire course of the game, so choose wisely. Are you sure you want to play in Experienced mode?",
	L"Avete selezionato la modalità ESPERTO. Questo scenario è adatto a chi ha già una certa dimestichezza con Jagged Alliance o con giochi simili. La vostra decisione influirà sull'intero corso della partita; scegliete, quindi, con attenzione. Siete sicuri di voler giocare nella modalità ESPERTO?",

	//L"You have chosen EXPERT mode. We warned you. Don't blame us if you get shipped back in a body bag. Your choice will affect things throughout the entire course of the game, so choose wisely. Are you sure you want to play in Expert mode?",
	L"Avete selezionato la modalità PROFESSIONISTA. Siete avvertiti. Non malediteci, se vi ritroverete a brandelli. La vostra decisione influirà sull'intero corso della partita; scegliete, quindi, con attenzione. Siete sicuri di voler giocare nella modalità PROFESSIONISTA?",

};

const wchar_t* gzLateLocalizedString[] =
{
	//1-5
	L"Il robot non può lasciare questo settore, se nessuno sta usando il controller.",

	//This message comes up if you have pending bombs waiting to explode in tactical.
	L"Non potete comprimere il tempo ora. Aspettate le esplosioni!",

	//'Name' refuses to move.
	L"%ls si rifiuta di muoversi.",

	//%ls a merc name
	L"%ls non ha abbastanza energia per cambiare posizione.",

	//A message that pops up when a vehicle runs out of gas.
	L"Il %ls ha esaurito la benzina e ora è rimasto a piedi a %c%d.",

	//6-10

	// the following two strings are combined with the pNewNoise[] strings above to report noises
	// heard above or below the merc
	L"sopra",
	L"sotto",

	//The following strings are used in autoresolve for autobandaging related feedback.
	L"Nessuno dei vostri mercenari non sa praticare il pronto soccorso.",
	L"Non ci sono supporti medici per bendare.",
	L"Non ci sono stati supporti medici sufficienti per bendare tutti.",
	L"Nessuno dei vostri mercenari ha bisogno di fasciature.",
	L"Fascia i mercenari automaticamento.",
	L"Tutti i vostri mercenari sono stati bendati.",

	//14
	L"Arulco",

  L"(tetto)",

	L"Salute: %d/%d",

	//In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
	//"vs." is the abbreviation of versus.
	L"%d contro %d",

	L"Il %ls è pieno!",  //(ex "The ice cream truck is full")

  L"%ls non ha bisogno immediatamente di pronto soccorso o di fasciature, quanto piuttosto di cure mediche più serie e/o riposo.",

	//20
	//Happens when you get shot in the legs, and you fall down.
	L"%ls è stato colpito alla gamba e collassa!",
	//Name can't speak right now.
	L"%ls non può parlare ora.",

	//22-24 plural versions
	L"%d l'esercito verde è stato promosso a veterano.",
	L"%d l'esercito verde è stato promosso a regolare.",
	L"%d l'esercito regolare è stato promosso a veterano.",

	//25
	L"Interruttore",

	//26
	//Name has gone psycho -- when the game forces the player into burstmode (certain unstable characters)
	L"%ls è impazzito!",

	//27-28
	//Messages why a player can't time compress.
	L"Non è al momento sicuro comprimere il tempo visto che avete dei mercenari nel settore %ls.",
	L"Non è al momento sicuro comprimere il tempo quando i mercenari sono nelle miniere infestate dalle creature.",

	//29-31 singular versions
	L"1 esercito verde è stato promosso a veterano.",
	L"1 esercito verde è stato promosso a regolare.",
	L"1 eserciro regolare è stato promosso a veterano.",

	//32-34
	L"%ls non dice nulla.",
	L"Andate in superficie?",
	L"(Squadra %d)",

	//35
	//Ex: "Red has repaired Scope's MP5K".  Careful to maintain the proper order (Red before Scope, Scope before MP5K)
	L"%ls ha riparato %ls's %ls",

	//36
	L"BLOODCAT",

	//37-38 "Name trips and falls"
	L"%ls trips and falls",
	L"Questo oggetto non può essere raccolto qui.",

	//39
	L"Nessuno dei vostri rimanenti mercenari è in grado di combattere. L'esercito combatterà contro le creature da solo.",

	//40-43
	//%ls is the name of merc.
	L"%ls è rimasto sprovvisto di kit medici!",
	L"%ls non è in grado di curare nessuno!",
	L"%ls è rimasto sprovvisto di forniture mediche!",
	L"%ls non è in grado di riparare niente!",

	//44-45
	L"Tempo di riparazione",
	L"%ls non può vedere questa persona.",

	//46-48
	L"L'estensore della canna dell'arma di %ls si è rotto!",
	L"Non più di %d allenatori di soldati sono ammessi in questo settore.",
  L"Siete sicuri?",

	//49-50
	L"Compressione del tempo",
	L"La tanica della benzina del veicolo è ora piena.",

	//51-52 Fast help text in mapscreen.
	L"Continua la compressione del tempo (|S|p|a|z|i|o)",
	L"Ferma la compressione del tempo (|E|s|c)",

	//53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
	L"%ls ha sbloccata il %ls",
	L"%ls ha sbloccato il %ls di %ls",

	//55
	L"Non potete comprimere il tempo mentre visualizzate l'inventario del settore.",

	//56
	//Displayed with the version information when cheats are enabled.
	L"Attuale/Massimo Progresso: %d%%/%d%%",

	//57
	L"Accompagnate John e Mary?",

  L"Interruttore attivato.",
};

const wchar_t str_ceramic_plates_smashed[] = L"%ls's ceramic plates have been smashed!"; // TODO translate

const wchar_t str_arrival_rerouted[] = L"Arrival of new recruits is being rerouted to sector %ls, as scheduled drop-off point of sector %ls is enemy occupied."; // TODO translate

#ifdef JA2DEMO
const wchar_t str_disabled_for_the_demo[]    = L"Disabilitato per Demo";
/* informs player this option/button has been disabled in the demo */
const wchar_t str_disabled_in_demo[]         = L"Siamo spiacenti. Questa opzione è stata disabilitata in questo demo.";
const wchar_t str_go_see_gabby[]             = L"Siete tutti finiti qui. Forse dovreste andare a trovare Gabby.";
const wchar_t str_too_bad_you_killed_gabby[] = L"Troppo crudele per averlo ucciso.";
/* For the demo, the sector exit interface, you'll be able to split your teams
 * up, but the demo has this feature disabled.  This string is fast help text
 * that appears over "single" button. */
const wchar_t str_strategic_exit_gui_demo[]  = L"In pieno gioco, potrete separare\nla vostra squadra, ma non in modalità demo.";
/* The overhead map is a map of the entire sector, which you can go into
 * anytime, except in the demo. */
const wchar_t str_overhead_map_disabled[]    = L"Mappa generale non attiva nella demo.";
#endif

const wchar_t str_stat_health[]       = L"Salute";
const wchar_t str_stat_agility[]      = L"Agilità";
const wchar_t str_stat_dexterity[]    = L"Destrezza";
const wchar_t str_stat_strength[]     = L"Forza";
const wchar_t str_stat_leadership[]   = L"Comando";
const wchar_t str_stat_wisdom[]       = L"Saggezza";
const wchar_t str_stat_exp_level[]    = L"Esperienza"; // Livello esperienza
const wchar_t str_stat_marksmanship[] = L"Mira";
const wchar_t str_stat_mechanical[]   = L"Meccanica";
const wchar_t str_stat_explosive[]    = L"Esplosivi";
const wchar_t str_stat_medical[]      = L"Pronto socc.";

#endif
