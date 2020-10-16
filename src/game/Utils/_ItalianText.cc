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



static const ST::string s_it_WeaponType[WeaponType_SIZE] =
{
	"Altro",
	"Arma",
	"Mitragliatrice",
	"Mitra",
	"Fucile",
	"Fucile del cecchino",
	"Fucile d'assalto",
	"Mitragliatrice leggera",
	"Fucile a canne mozze",
};

static const ST::string s_it_TeamTurnString[TeamTurnString_SIZE] =
{
	"Turno del giocatore", // player's turn
	"Turno degli avversari",
	"Turno delle creature",
	"Turno dell'esercito",
	"Turno dei civili",
	// planning turn
};

static const ST::string s_it_Message[Message_SIZE] =
{
	// In the following 8 strings, the %s is the merc's name, and the %d (if any) is a number.

	"%s è stato colpito alla testa e perde un punto di saggezza!",
	"%s è stato colpito alla spalla e perde un punto di destrezza!",
	"%s è stato colpito al torace e perde un punto di forza!",
	"%s è stato colpito alle gambe e perde un punto di agilità!",
	"%s è stato colpito alla testa e perde %d punti di saggezza!",
	"%s è stato colpito alle palle perde %d punti di destrezza!",
	"%s è stato colpito al torace e perde %d punti di forza!",
	"%s è stato colpito alle gambe e perde %d punti di agilità!",
	"Interrompete!",

	"I vostri rinforzi sono arrivati!",

	// In the following four lines, all %s's are merc names

	"%s ricarica.",
	"%s non ha abbastanza Punti Azione!",
	// the following 17 strings are used to create lists of gun advantages and disadvantages
	// (separated by commas)
	"affidabile",
	"non affidabile",
	"facile da riparare",
	"difficile da riparare",
	"danno grave",
	"danno lieve",
	"fuoco veloce",
	"fuoco",
	"raggio lungo",
	"raggio corto",
	"leggero",
	"pesante",
	"piccolo",
	"fuoco a raffica",
	"niente raffiche",
	"grande deposito d'armi",
	"piccolo deposito d'armi",

	// In the following two lines, all %s's are merc names

	"Il travestimento di %s è stato scoperto.",
	"Il travestimento di %s è stato scoperto.",

	// The first %s is a merc name and the second %s is an item name

	"La seconda arma è priva di munizioni!",
	"%s ha rubato il %s.",

	// The %s is a merc name

	"L'arma di %s non può più sparare a raffica.",

	"Ne avete appena ricevuto uno di quelli attaccati.",
	"Volete combinare gli oggetti?",

	// Both %s's are item names

	"Non potete attaccare %s a un %s.",

	"Nessuno",
	"Espelli munizioni",
	"Attaccare",

	//You cannot use "item(s)" and your "other item" at the same time.
	//Ex:  You cannot use sun goggles and you gas mask at the same time.
	"Non potete usare %s e il vostro %s contemporaneamente.",

	"L'oggetto puntato dal vostro cursore può essere combinato ad alcuni oggetti ponendolo in uno dei quattro slot predisposti.",
	"L'oggetto puntato dal vostro cursore può essere combinato ad alcuni oggetti ponendolo in uno dei quattro slot predisposti. (Comunque, in questo caso, l'oggetto non è compatibile.)",
	"Il settore non è libero da nemici!",
	"Vi dovete ancora dare %s %s",
	"%s è stato colpito alla testa!",
	"Abbandonate la battaglia?",
	"Questo attaco sarà definitivo. Andate avanti?",
	"%s si sente molto rinvigorito!",
	"%s ha dormito di sasso!",
	"%s non è riuscito a catturare il %s!",
	"%s ha riparato il %s",
	"Interrompete per ",
	"Vi arrendete?",
	"Questa persona rifiuta il vostro aiuto.",
	"NON sono d'accordo!",
	"Per viaggiare sull'elicottero di Skyrider, dovrete innanzitutto ASSEGNARE mercenari al VEICOLO/ELICOTTERO.",
	"solo %s aveva abbastanza tempo per ricaricare UNA pistola",
	"Turno dei Bloodcat",
};


// the types of time compression. For example: is the timer paused? at normal speed, 5 minutes per second, etc.
// min is an abbreviation for minutes

static const ST::string s_it_sTimeStrings[sTimeStrings_SIZE] =
{
	"Fermo",
	"Normale",
	"5 min",
	"30 min",
	"60 min",
	"6 ore",
};


// Assignment Strings: what assignment does the merc  have right now? For example, are they on a squad, training,
// administering medical aid (doctor) or training a town. All are abbreviated. 8 letters is the longest it can be.

static const ST::string s_it_pAssignmentStrings[pAssignmentStrings_SIZE] =
{
	"Squad. 1",
	"Squad. 2",
	"Squad. 3",
	"Squad. 4",
	"Squad. 5",
	"Squad. 6",
	"Squad. 7",
	"Squad. 8",
	"Squad. 9",
	"Squad. 10",
	"Squad. 11",
	"Squad. 12",
	"Squad. 13",
	"Squad. 14",
	"Squad. 15",
	"Squad. 16",
	"Squad. 17",
	"Squad. 18",
	"Squad. 19",
	"Squad. 20",
	"Servizio", // on active duty
	"Dottore", // administering medical aid
	"Paziente", // getting medical aid
	"Veicolo", // in a vehicle
	"Transito", // in transit - abbreviated form
	"Riparare", // repairing
	"Esercit.", // training themselves
	"Esercit.", // training a town to revolt
	"Istrutt.", // training a teammate
	"Studente", // being trained by someone else
	"Morto", // dead
	"Incap.", // abbreviation for incapacitated
	"PDG", // Prisoner of war - captured
	"Ospedale", // patient in a hospital
	"Vuoto",	// Vehicle is empty
};


static const ST::string s_it_pMilitiaString[pMilitiaString_SIZE] =
{
	"Esercito", // the title of the militia box
	"Non incaricato", //the number of unassigned militia troops
	"Non potete ridistribuire reclute, se ci sono nemici nei paraggi!",
};


static const ST::string s_it_pMilitiaButtonString[pMilitiaButtonString_SIZE] =
{
	"Auto", // auto place the militia troops for the player
	"Eseguito", // done placing militia troops
};

static const ST::string s_it_pConditionStrings[pConditionStrings_SIZE] =
{
	"Eccellente", //the state of a soldier .. excellent health
	"Buono", // good health
	"Discreto", // fair health
	"Ferito", // wounded health
	"Stanco", // tired
	"Grave", // bleeding to death
	"Svenuto", // knocked out
	"Morente", // near death
	"Morto", // dead
};

static const ST::string s_it_pEpcMenuStrings[pEpcMenuStrings_SIZE] =
{
	"In servizio", // set merc on active duty
	"Paziente", // set as a patient to receive medical aid
	"Veicolo", // tell merc to enter vehicle
	"Non scortato", // let the escorted character go off on their own
	"Cancella", // close this menu
};


// look at pAssignmentString above for comments

static const ST::string s_it_pLongAssignmentStrings[pLongAssignmentStrings_SIZE] =
{
	"Squadra 1",
	"Squadra 2",
	"Squadra 3",
	"Squadra 4",
	"Squadra 5",
	"Squadra 6",
	"Squadra 7",
	"Squadra 8",
	"Squadra 9",
	"Squadra 10",
	"Squadra 11",
	"Squadra 12",
	"Squadra 13",
	"Squadra 14",
	"Squadra 15",
	"Squadra 16",
	"Squadra 17",
	"Squadra 18",
	"Squadra 19",
	"Squadra 20",
	"In servizio",
	"Dottore",
	"Paziente",
	"Veicolo",
	"In transito",
	"Riparare",
	"Esercitarsi",
	"Allenatore esercito",
	"Allena squadra",
	"Studente",
	"Morto",
	"Incap.",
	"PDG",
	"Ospedale", // patient in a hospital
	"Vuoto",	// Vehicle is empty
};


// the contract options

static const ST::string s_it_pContractStrings[pContractStrings_SIZE] =
{
	"Opzioni del contratto:",
	"", // a blank line, required
	"Offri 1 giorno", // offer merc a one day contract extension
	"Offri 1 settimana", // 1 week
	"Offri 2 settimane", // 2 week
	"Termina contratto", // end merc's contract
	"Annulla", // stop showing this menu
};

static const ST::string s_it_pPOWStrings[pPOWStrings_SIZE] =
{
	"PDG",  //an acronym for Prisoner of War
	"??",
};

static const ST::string s_it_pInvPanelTitleStrings[pInvPanelTitleStrings_SIZE] =
{
	"Giubb. A-P", // the armor rating of the merc
	"Peso", // the weight the merc is carrying
	"Trav.", // the merc's camouflage rating
};

static const ST::string s_it_pShortAttributeStrings[pShortAttributeStrings_SIZE] =
{
	"Abi", // the abbreviated version of : agility
	"Des", // dexterity
	"For", // strength
	"Com", // leadership
	"Sag", // wisdom
	"Liv", // experience level
	"Tir", // marksmanship skill
	"Esp", // explosive skill
	"Mec", // mechanical skill
	"PS", // medical skill
};


static const ST::string s_it_pUpperLeftMapScreenStrings[pUpperLeftMapScreenStrings_SIZE] =
{
	"Compito", // the mercs current assignment
	"Salute", // the health level of the current merc
	"Morale", // the morale of the current merc
	"Cond.",	// the condition of the current vehicle
};

static const ST::string s_it_pTrainingStrings[pTrainingStrings_SIZE] =
{
	"Esercitarsi", // tell merc to train self
	"Esercito", // tell merc to train town
	"Allenatore", // tell merc to act as trainer
	"Studente", // tell merc to be train by other
};

static const ST::string s_it_pAssignMenuStrings[pAssignMenuStrings_SIZE] =
{
	"In servizio", // merc is on active duty
	"Dottore", // the merc is acting as a doctor
	"Paziente", // the merc is receiving medical attention
	"Veicolo", // the merc is in a vehicle
	"Ripara", // the merc is repairing items
	"Si esercita", // the merc is training
	"Annulla", // cancel this menu
};

static const ST::string s_it_pRemoveMercStrings[pRemoveMercStrings_SIZE] =
{
	"Rimuovi Mercenario", // remove dead merc from current team
	"Annulla",
};

static const ST::string s_it_pAttributeMenuStrings[pAttributeMenuStrings_SIZE] =
{
	"Forza",
	"Destrezza",
	"Agilità",
	"Salute",
	"Mira",
	"Pronto socc.",
	"Meccanica",
	"Comando",
	"Esplosivi",
	"Annulla",
};

static const ST::string s_it_pTrainingMenuStrings[pTrainingMenuStrings_SIZE] =
{
	"Allenati", // train yourself
	"Esercito", // train the town
	"Allenatore", // train your teammates
	"Studente",  // be trained by an instructor
	"Annulla", // cancel this menu
};


static const ST::string s_it_pSquadMenuStrings[pSquadMenuStrings_SIZE] =
{
	"Squadra  1",
	"Squadra  2",
	"Squadra  3",
	"Squadra  4",
	"Squadra  5",
	"Squadra  6",
	"Squadra  7",
	"Squadra  8",
	"Squadra  9",
	"Squadra 10",
	"Squadra 11",
	"Squadra 12",
	"Squadra 13",
	"Squadra 14",
	"Squadra 15",
	"Squadra 16",
	"Squadra 17",
	"Squadra 18",
	"Squadra 19",
	"Squadra 20",
	"Annulla",
};


static const ST::string s_it_pPersonnelScreenStrings[pPersonnelScreenStrings_SIZE] =
{
	"Deposito med.:", // amount of medical deposit put down on the merc
	"Contratto in corso:", // cost of current contract
	"Uccisi", // number of kills by merc
	"Assistiti", // number of assists on kills by merc
	"Costo giornaliero:", // daily cost of merc
	"Tot. costo fino a oggi:", // total cost of merc
	"Contratto:", // cost of current contract
	"Tot. servizio fino a oggi:", // total service rendered by merc
	"Salario arretrato:", // amount left on MERC merc to be paid
	"Percentuale di colpi:", // percentage of shots that hit target
	"Battaglie", // number of battles fought
	"Numero ferite", // number of times merc has been wounded
	"Destrezza:",
	"Nessuna abilità",
};


//These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
static const ST::string s_it_gzMercSkillText[gzMercSkillText_SIZE] =
{
	"Nessuna abilità",
	"Forzare serrature",
	"Corpo a corpo",
	"Elettronica",
	"Op. notturne",
	"Lanciare",
	"Istruire",
	"Armi pesanti",
	"Armi automatiche",
	"Clandestino",
	"Ambidestro",
	"Furtività",
	"Arti marziali",
	"Coltelli",
	"Bonus per altezza",
	"Camuffato",
	"(Esperto)",
};


// This is pop up help text for the options that are available to the merc

static const ST::string s_it_pTacticalPopupButtonStrings[pTacticalPopupButtonStrings_SIZE] =
{
	"|Stare fermi/Camminare",
	"|Accucciarsi/Muoversi accucciato",
	"Stare fermi/|Correre",
	"|Prono/Strisciare",
	"|Guardare",
	"Agire",
	"Parlare",
	"Esaminare (|C|t|r|l)",

	// Pop up door menu
	"Aprire manualmente",
	"Esaminare trappole",
	"Grimaldello",
	"Forzare",
	"Liberare da trappole",
	"Chiudere",
	"Aprire",
	"Usare esplosivo per porta",
	"Usare piede di porco",
	"Annulla (|E|s|c)",
	"Chiudere",
};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.

static const ST::string s_it_pDoorTrapStrings[pDoorTrapStrings_SIZE] =
{
	"Nessuna trappola",
	"una trappola esplosiva",
	"una trappola elettrica",
	"una trappola con sirena",
	"una trappola con allarme insonoro",
};

// On the map screen, there are four columns. This text is popup help text that identifies the individual columns.

static const ST::string s_it_pMapScreenMouseRegionHelpText[pMapScreenMouseRegionHelpText_SIZE] =
{
	"Selezionare postazioni",
	"Assegnare mercenario",
	"Tracciare percorso di viaggio",
	"Merc |Contratto",
	"Eliminare mercenario",
	"Dormire",
};

// volumes of noises

static const ST::string s_it_pNoiseVolStr[pNoiseVolStr_SIZE] =
{
	"DEBOLE",
	"DEFINITO",
	"FORTE",
	"MOLTO FORTE",
};

// types of noises

static const ST::string s_it_pNoiseTypeStr[pNoiseTypeStr_SIZE] = // OBSOLETE
{
	"SCONOSCIUTO",
	"rumore di MOVIMENTO",
	"SCRICCHIOLIO",
	"TONFO IN ACQUA",
	"IMPATTO",
	"SPARO",
	"ESPLOSIONE",
	"URLA",
	"IMPATTO",
	"IMPATTO",
	"FRASTUONO",
	"SCHIANTO",
};

// Directions that are used to report noises

static const ST::string s_it_pDirectionStr[pDirectionStr_SIZE] =
{
	"il NORD-EST",
	"il EST",
	"il SUD-EST",
	"il SUD",
	"il SUD-OVEST",
	"il OVEST",
	"il NORD-OVEST",
	"il NORD",
};

static const ST::string s_it_gpStrategicString[gpStrategicString_SIZE] =
{
	"%s sono stati individuati nel settore %c%d e un'altra squadra sta per arrivare.",	//STR_DETECTED_SINGULAR
	"%s sono stati individuati nel settore %c%d e un'altra squadra sta per arrivare.",	//STR_DETECTED_PLURAL
	"Volete coordinare un attacco simultaneo?",													//STR_COORDINATE

	//Dialog strings for enemies.

	"Il nemico offre la possibilità di arrendervi.",			//STR_ENEMY_SURRENDER_OFFER
	"Il nemico ha catturato i vostri mercenari sopravvissuti.",	//STR_ENEMY_CAPTURED

	//The text that goes on the autoresolve buttons

	"Ritirarsi", 		//The retreat button				//STR_AR_RETREAT_BUTTON
	"Fine",		//The done button				//STR_AR_DONE_BUTTON

	//The headers are for the autoresolve type (MUST BE UPPERCASE)

	"DIFENDERE",								//STR_AR_DEFEND_HEADER
	"ATTACCARE",								//STR_AR_ATTACK_HEADER
	"INCONTRARE",								//STR_AR_ENCOUNTER_HEADER
	"settore",		//The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

	//The battle ending conditions

	"VITTORIA!",								//STR_AR_OVER_VICTORY
	"SCONFITTA!",								//STR_AR_OVER_DEFEAT
	"ARRENDERSI!",							//STR_AR_OVER_SURRENDERED
	"CATTURATI!",								//STR_AR_OVER_CAPTURED
	"RITIRARSI!",								//STR_AR_OVER_RETREATED

	//These are the labels for the different types of enemies we fight in autoresolve.

	"Esercito",							//STR_AR_MILITIA_NAME,
	"Èlite",								//STR_AR_ELITE_NAME,
	"Truppa",								//STR_AR_TROOP_NAME,
	"Amministratore",								//STR_AR_ADMINISTRATOR_NAME,
	"Creatura",								//STR_AR_CREATURE_NAME,

	//Label for the length of time the battle took

	"Tempo trascorso",							//STR_AR_TIME_ELAPSED,

	//Labels for status of merc if retreating.  (UPPERCASE)

	"RITIRATOSI",								//STR_AR_MERC_RETREATED,
	"RITIRARSI",								//STR_AR_MERC_RETREATING,
	"RITIRATA",								//STR_AR_MERC_RETREAT,

	//PRE BATTLE INTERFACE STRINGS
	//Goes on the three buttons in the prebattle interface.  The Auto resolve button represents
	//a system that automatically resolves the combat for the player without having to do anything.
	//These strings must be short (two lines -- 6-8 chars per line)

	"Esito",							//STR_PB_AUTORESOLVE_BTN,
	"Vai al settore",							//STR_PB_GOTOSECTOR_BTN,
	"Ritira merc.",							//STR_PB_RETREATMERCS_BTN,

	//The different headers(titles) for the prebattle interface.
	"SCONTRO NEMICO",							//STR_PB_ENEMYENCOUNTER_HEADER,
	"INVASIONE NEMICA",							//STR_PB_ENEMYINVASION_HEADER, // 30
	"IMBOSCATA NEMICA",								//STR_PB_ENEMYAMBUSH_HEADER
	"INTRUSIONE NEMICA NEL SETTORE",				//STR_PB_ENTERINGENEMYSECTOR_HEADER
	"ATTACCO DELLE CREATURE",							//STR_PB_CREATUREATTACK_HEADER
	"IMBOSCATA DEI BLOODCAT",							//STR_PB_BLOODCATAMBUSH_HEADER
	"INTRUSIONE NELLA TANA BLOODCAT",			//STR_PB_ENTERINGBLOODCATLAIR_HEADER

	//Various single words for direct translation.  The Civilians represent the civilian
	//militia occupying the sector being attacked.  Limited to 9-10 chars

	"Postazione",
	"Nemici",
	"Mercenari",
	"Esercito",
	"Creature",
	"Bloodcat",
	"Settore",
	"Nessuno",		//If there are no uninvolved mercs in this fight.
	"N/A",			//Acronym of Not Applicable
	"g",			//One letter abbreviation of day
	"o",			//One letter abbreviation of hour

	//TACTICAL PLACEMENT USER INTERFACE STRINGS
	//The four buttons

	"Sgombro",
	"Sparsi",
	"In gruppo",
	"Fine",

	//The help text for the four buttons.  Use \n to denote new line (just like enter).

	"|Mostra chiaramente tutte le postazioni dei mercenari,\ne vi permette di rimetterli in gioco manualmente.",
	"A caso |sparge i vostri mercenari\nogni volta che lo premerete.",
	"Vi permette di scegliere dove vorreste |raggruppare i vostri mercenari.",
	"Cliccate su questo pulsante quando avrete\nscelto le postazioni dei vostri mercenari. (|I|n|v|i|o)",
	"Dovete posizionare tutti i vostri mercenari\nprima di iniziare la battaglia.",

	//Various strings (translate word for word)

	"Settore",
	"Scegliete le postazioni di intervento",

	//Strings used for various popup message boxes.  Can be as long as desired.

	"Non sembra così bello qui. È inacessibile. Provate con una diversa postazione.",
	"Posizionate i vostri mercenari nella sezione illuminata della mappa.",

	//These entries are for button popup help text for the prebattle interface.  All popup help
	//text supports the use of \n to denote new line.  Do not use spaces before or after the \n.
	"|Automaticamente svolge i combattimenti al vostro posto\nsenza caricare la mappa.",
	"Non è possibile utilizzare l'opzione di risoluzione automatica quando\nil giocatore sta attaccando.",
	"|Entrate nel settore per catturare il nemico.",
	"|Rimandate il gruppo al settore precedente.",				//singular version
	"|Rimandate tutti i gruppi ai loro settori precedenti.", //multiple groups with same previous sector

	//various popup messages for battle conditions.

	//%c%d is the sector -- ex:  A9
	"I nemici attaccano il vostro esercito nel settore %c%d.",
	//%c%d is the sector -- ex:  A9
	"Le creature attaccano il vostro esercito nel settore %c%d.",
	//1st %d refers to the number of civilians eaten by monsters,  %c%d is the sector -- ex:  A9
	//Note:  the minimum number of civilians eaten will be two.
	"Le creature attaccano e uccidono %d civili nel settore %s.",
	//%s is the sector location -- ex:  A9: Omerta
	"I nemici attaccano i vostri mercenari nel settore %s. Nessuno dei vostri mercenari è in grado di combattere!",
	//%s is the sector location -- ex:  A9: Omerta
	"I nemici attaccano i vostri mercenari nel settore %s. Nessuno dei vostri mercenari è in grado di combattere!",

};

//This is the day represented in the game clock.  Must be very short, 4 characters max.
static const ST::string s_it_gpGameClockString = "Gg";

//When the merc finds a key, they can get a description of it which
//tells them where and when they found it.
static const ST::string s_it_sKeyDescriptionStrings[sKeyDescriptionStrings_SIZE] =
{
	"Settore trovato:",
	"Giorno trovato:",
};

//The headers used to describe various weapon statistics.

static const ST::string s_it_gWeaponStatsDesc[ gWeaponStatsDesc_SIZE] =
{
	"Peso (%s):",
	"Stato:",
	"Ammontare:", 		// Number of bullets left in a magazine
	"Git:",		// Range
	"Dan:",		// Damage
	"PA:",			// abbreviation for Action Points
	"="
};

//The headers used for the merc's money.

static const ST::string s_it_gMoneyStatsDesc[gMoneyStatsDesc_SIZE] =
{
	"Ammontare",
	"Rimanenti:", //this is the overall balance
	"Ammontare",
	"Da separare:", // the amount he wants to separate from the overall balance to get two piles of money

	"Bilancio",
	"corrente",
	"Ammontare",
	"del prelievo",
};

//The health of various creatures, enemies, characters in the game. The numbers following each are for comment
//only, but represent the precentage of points remaining.

static const ST::string s_it_zHealthStr[zHealthStr_SIZE] =
{
	"MORENTE",		//	>= 0
	"CRITICO", 		//	>= 15
	"DEBOLE",		//	>= 30
	"FERITO",    		//	>= 45
	"SANO",    		//	>= 60
	"FORTE",     		// 	>= 75
	"ECCELLENTE",		// 	>= 90
};

static const ST::string s_it_gzMoneyAmounts[gzMoneyAmounts_SIZE] =
{
	"$1000",
	"$100",
	"$10",
	"Fine",
	"Separare",
	"Prelevare",
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
static const ST::string s_it_gzProsLabel = "Vant.:";
static const ST::string s_it_gzConsLabel = "Svant.:";

//Conversation options a player has when encountering an NPC
static const ST::string s_it_zTalkMenuStrings[zTalkMenuStrings_SIZE] =
{
	"Vuoi ripetere?", 	//meaning "Repeat yourself"
	"Amichevole",		//approach in a friendly
	"Diretto",		//approach directly - let's get down to business
	"Minaccioso",		//approach threateningly - talk now, or I'll blow your face off
	"Dai",
	"Recluta",
};

//Some NPCs buy, sell or repair items. These different options are available for those NPCs as well.
static const ST::string s_it_zDealerStrings[zDealerStrings_SIZE] =
{
	"Compra/Vendi",
	"Compra",
	"Vendi",
	"Ripara",
};

static const ST::string s_it_zDialogActions = "Fine";


//These are vehicles in the game.

static const ST::string s_it_pVehicleStrings[pVehicleStrings_SIZE] =
{
	"Eldorado",
	"Hummer", // a hummer jeep/truck -- military vehicle
	"Icecream Truck",
	"Jeep",
	"Carro armato",
	"Elicottero",
};

static const ST::string s_it_pShortVehicleStrings[pVehicleStrings_SIZE] =
{
	"Eldor.",
	"Hummer",			// the HMVV
	"Truck",
	"Jeep",
	"Carro",
	"Eli", 				// the helicopter
};

static const ST::string s_it_zVehicleName[pVehicleStrings_SIZE] =
{
	"Eldorado",
	"Hummer",		//a military jeep. This is a brand name.
	"Truck",			// Ice cream truck
	"Jeep",
	"Carro",
	"Eli", 		//an abbreviation for Helicopter
};


//These are messages Used in the Tactical Screen

static const ST::string s_it_TacticalStr[TacticalStr_SIZE] =
{
	"Attacco aereo",
	"Ricorrete al pronto soccorso automaticamente?",

	// CAMFIELD NUKE THIS and add quote #66.

	"%s nota ch egli oggetti mancano dall'equipaggiamento.",

	// The %s is a string from pDoorTrapStrings

	"La serratura ha %s",
	"Non ci sono serrature",
	"La serratura non presenta trappole",
	// The %s is a merc name
	"%s non ha la chiave giusta",
	"La serratura non presenta trappole",
	"Serrato",
	"",
	"TRAPPOLE",
	"SERRATO",
	"APERTO",
	"FRACASSATO",
	"C'è un interruttore qui. Lo volete attivare?",
	"Disattivate le trappole?",
	"Più...",

	// In the next 2 strings, %s is an item name

	"Il %s è stato posizionato sul terreno.",
	"Il %s è stato dato a %s.",

	// In the next 2 strings, %s is a name

	"%s è stato pagato completamente.",
	"Bisogna ancora dare %d a %s.",
	"Scegliete la frequenza di detonazione:",  	//in this case, frequency refers to a radio signal
	"Quante volte finché la bomba non esploderà:",	//how much time, in turns, until the bomb blows
	"Stabilite la frequenza remota di detonazione:", 	//in this case, frequency refers to a radio signal
	"Disattivate le trappole?",
	"Rimuovete la bandiera blu?",
	"Mettete qui la bandiera blu?",
	"Fine del turno",

	// In the next string, %s is a name. Stance refers to way they are standing.

	"Siete sicuri di volere attaccare %s ?",
	"Ah, i veicoli non possono cambiare posizione.",
	"Il robot non può cambiare posizione.",

	// In the next 3 strings, %s is a name

	"%s non può cambiare posizione.",
	"%s non sono ricorsi al pronto soccorso qui.",
	"%s non ha bisogno del pronto soccorso.",
	"Non può muoversi là.",
	"La vostra squadra è al completo. Non c'è spazio per una recluta.",	//there's no room for a recruit on the player's team

	// In the next string, %s is a name

	"%s è stato reclutato.",

	// Here %s is a name and %d is a number

	"Bisogna dare %d a $%s.",

	// In the next string, %s is a name

	"Scortate %s?",

	// In the next string, the first %s is a name and the second %s is an amount of money (including $ sign)

	"Il salario di %s ammonta a %s per giorno?",

	// This line is used repeatedly to ask player if they wish to participate in a boxing match.

	"Volete combattere?",

	// In the next string, the first %s is an item name and the
	// second %s is an amount of money (including $ sign)

	"Comprate %s per %s?",

	// In the next string, %s is a name

	"%s è scortato dalla squadra %d.",

	// These messages are displayed during play to alert the player to a particular situation

	"INCEPPATA",					//weapon is jammed.
	"Il robot ha bisogno di munizioni calibro %s.",		//Robot is out of ammo
	"Cosa? Impossibile.",		//Merc can't throw to the destination he selected

	// These are different buttons that the player can turn on and off.

	"Modalità furtiva (|Z)",
	"Schermata della |mappa",
	"Fine del turno (|D)",
	"Parlato",
	"Muto",
	"Alza (|P|a|g|S|ù)",
	"Livello della vista (|T|a|b)",
	"Scala / Salta",
	"Abbassa (|P|a|g|G|i|ù)",
	"Esamina (|C|t|r|l)",
	"Mercenario precedente",
	"Prossimo mercenario (|S|p|a|z|i|o)",
	"|Opzioni",
	"Modalità a raffica (|B)",
	"Guarda/Gira (|L)",
	"Salute: %d/%d\nEnergia: %d/%d\nMorale: %s",
	"Eh?",					//this means "what?"
	"Fermo",					//an abbrieviation for "Continued"
	"Audio on per %s.",
	"Audio off per %s.",
	"Salute: %d/%d\nCarburante: %d/%d",
	"Uscita veicoli" ,
	"Cambia squadra (|M|a|i|u|s|c |S|p|a|z|i|o)",
	"Guida",
	"N/A",						//this is an acronym for "Not Applicable."
	"Usa (Corpo a corpo)",
	"Usa (Arma da fuoco)",
	"Usa (Lama)",
	"Usa (Esplosivo)",
	"Usa (Kit medico)",
	"Afferra",
	"Ricarica",
	"Dai",
	"%s è partito.",
	"%s è arrivato.",
	"%s ha esaurito i Punti Azione.",
	"%s non è disponibile.",
	"%s è tutto bendato.",
	"%s non è provvisto di bende.",
	"Nemico nel settore!",
	"Nessun nemico in vista.",
	"Punti Azione insufficienti.",
	"Nessuno sta utilizzando il comando a distanza.",
	"Il fuoco a raffica ha svuotato il caricatore!",
	"SOLDATO",
	"CREPITUS",
	"ESERCITO",
	"CIVILE",
	"Settore di uscita",
	"OK",
	"Annulla",
	"Merc. selezionato",
	"Tutta la squadra",
	"Vai nel settore",
	"Vai alla mappa",
	"Non puoi uscire dal settore da questa parte.",
	"%s è troppo lontano.",
	"Rimuovi le fronde degli alberi",
	"Mostra le fronde degli alberi",
	"CORVO",				//Crow, as in the large black bird
	"COLLO",
	"TESTA",
	"TORSO",
	"GAMBE",
	"Vuoi dire alla Regina cosa vuole sapere?",
	"Impronta digitale ID ottenuta",
	"Impronta digitale ID non valida. Arma non funzionante",
	"Raggiunto scopo",
	"Sentiero bloccato",
	"Deposita/Preleva soldi",		//Help text over the $ button on the Single Merc Panel
	"Nessuno ha bisogno del pronto soccorso.",
	"Bloccato.",											// Short form of JAMMED, for small inv slots
	"Non può andare là.",					// used ( now ) for when we click on a cliff
	"La persona rifiuta di muoversi.",
	// In the following message, '%s' would be replaced with a quantity of money (e.g. $200)
	"Sei d'accordo a pagare %s?",
	"Accetti il trattamento medico gratuito?",
	"Vuoi sposare Daryl?",
	"Quadro delle chiavi",
	"Non puoi farlo con un EPC.",
	"Risparmi Krott?",
	"Fuori dalla gittata dell'arma",
	"Minatore",
	"Il veicolo può viaggiare solo tra i settori",
	"Non è in grado di fasciarsi da solo ora",
	"Sentiero bloccato per %s",
//	"I tuoi mercenari, che erano stati catturati dall'esercito di Deidranna, sono stati imprigionati qui!",
	"I mercenari catturati dall'esercito di Deidranna, sono stati imprigionati qui!",
	"Serratura manomessa",
	"Serratura distrutta",
	"Qualcun altro sta provando a utilizzare questa porta.",
	"Salute: %d/%d\nCarburante: %d/%d",
	"%s non riesce a vedere %s.",  // Cannot see person trying to talk to
};

//Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different circumstances in the "exiting sector" interface.
static const ST::string s_it_pExitingSectorHelpText[pExitingSectorHelpText_SIZE] =
{
	//Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box is checked.
	"Se selezionato, il settore adiacente verrà immediatamente caricato.",
	"Se selezionato, sarete automaticamente posti nella schermata della mappa\nvisto che i vostri mercenari avranno bisogno di tempo per viaggiare.",

	//If you attempt to leave a sector when you have multiple squads in a hostile sector.
	"Questo settore è occupato da nemicie non potete lasciare mercenari qui.\nDovete risolvere questa situazione prima di caricare qualsiasi altro settore.",

	//Because you only have one squad in the sector, and the "move all" option is checked, the "go to sector" option is locked to on.
	//The helptext explains why it is locked.
	"Rimuovendo i vostri mercenari da questo settore,\nil settore adiacente verrà immediatamente caricato.",
	"Rimuovendo i vostri mercenari da questo settore,\nverrete automaticamente postinella schermata della mappa\nvisto che i vostri mercenari avranno bisogno di tempo per viaggiare.",

	//If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being escorted.  The "single" button is disabled.
	"%s ha bisogno di essere scortato dai vostri mercenari e non può lasciare questo settore da solo.",

	//If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc will be prohibited from leaving alone.
	//There are several strings depending on the gender of the merc and how many EPCs are in the squad.
	//DO NOT USE THE NEWLINE HERE AS IT IS USED FOR BOTH HELPTEXT AND SCREEN MESSAGES!
	"%s non può lasciare questo settore da solo, perché sta scortando %s.", //male singular
	"%s non può lasciare questo settore da solo, perché sta scortando %s.", //female singular
	"%s non può lasciare questo settore da solo, perché sta scortando altre persone.", //male plural
	"%s non può lasciare questo settore da solo, perché sta scortando altre persone.", //female plural

	//If one or more of your mercs in the selected squad aren't in range of the traversal area, then the  "move all" option is disabled,
	//and this helptext explains why.
	"Tutti i vostri personaggi devono trovarsi nei paraggi\nin modo da permettere alla squadra di attraversare.",

	//Standard helptext for single movement.  Explains what will happen (splitting the squad)
	"Se selezionato, %s viaggerà da solo, e\nautomaticamente verrà riassegnato a un'unica squadra.",

	//Standard helptext for all movement.  Explains what will happen (moving the squad)
	"Se selezionato, la vostra\nsquadra attualmente selezionata viaggerà, lasciando questo settore.",

	//This strings is used BEFORE the "exiting sector" interface is created.  If you have an EPC selected and you attempt to tactically
	//traverse the EPC while the escorting mercs aren't near enough (or dead, dying, or unconscious), this message will appear and the
	//"exiting sector" interface will not appear.  This is just like the situation where
	//This string is special, as it is not used as helptext.  Do not use the special newline character (\n) for this string.
	"%s è scortato dai vostri mercenari e non può lasciare questo settore da solo. Gli altri vostri mercenari devono trovarsi nelle vicinanze prima che possiate andarvene.",
};



static const ST::string s_it_pRepairStrings[pRepairStrings_SIZE] =
{
	"Oggetti", 		// tell merc to repair items in inventory
	"Sito SAM", 		// tell merc to repair SAM site - SAM is an acronym for Surface to Air Missile
	"Annulla", 		// cancel this menu
	"Robot", 		// repair the robot
};


// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."

static const ST::string s_it_sPreStatBuildString[sPreStatBuildString_SIZE] =
{
	"perduto", 			// the merc has lost a statistic
	"guadagnato", 		// the merc has gained a statistic
	"punto di",	// singular
	"punti di",	// plural
	"livello di",	// singular
	"livelli di",	// plural
};

static const ST::string s_it_sStatGainStrings[sStatGainStrings_SIZE] =
{
	"salute.",
	"agilità.",
	"destrezza.",
	"saggezza.",
	"pronto socc.",
	"abilità esplosivi.",
	"abilità meccanica.",
	"mira.",
	"esperienza.",
	"forza.",
	"comando.",
};


static const ST::string s_it_pHelicopterEtaStrings[pHelicopterEtaStrings_SIZE] =
{
	"Distanza totale: ", 			// total distance for helicopter to travel
	"Sicura: ", 			// distance to travel to destination
	"Insicura: ", 			// distance to return from destination to airport
	"Costo totale: ", 		// total cost of trip by helicopter
	"TPA: ", 			// ETA is an acronym for "estimated time of arrival"
	"L'elicottero ha poco carburante e deve atterrare in territorio nemico!",	// warning that the sector the helicopter is going to use for refueling is under enemy control ->
	"Passeggeri: ",
	"Seleziona Skyrider o gli Arrivi Drop-off?",
	"Skyrider",
	"Arrivi",
};

static const ST::string s_it_sMapLevelString = "Sottolivello:"; // what level below the ground is the player viewing in mapscreen

static const ST::string s_it_gsLoyalString = "%d%% Leale"; // the loyalty rating of a town ie : Loyal 53%


// error message for when player is trying to give a merc a travel order while he's underground.
static const ST::string s_it_gsUndergroundString = "non può portare ordini di viaggio sottoterra.";

static const ST::string s_it_gsTimeStrings[gsTimeStrings_SIZE] =
{
	"h",				// hours abbreviation
	"m",				// minutes abbreviation
	"s",				// seconds abbreviation
	"g",				// days abbreviation
};

// text for the various facilities in the sector

static const ST::string s_it_sFacilitiesStrings[sFacilitiesStrings_SIZE] =
{
	"Nessuno",
	"Ospedale",
	"Fabbrica",
	"Prigione",
	"Militare",
	"Aeroporto",
	"Frequenza di fuoco",		// a field for soldiers to practise their shooting skills
};

// text for inventory pop up button

static const ST::string s_it_pMapPopUpInventoryText[pMapPopUpInventoryText_SIZE] =
{
	"Inventario",
	"Uscita",
};

// town strings

static const ST::string s_it_pwTownInfoStrings[pwTownInfoStrings_SIZE] =
{
	"Dimensione",					// size of the town in sectors
	"Controllo", 					// how much of town is controlled
	"Miniera", 				// mine associated with this town
	"Lealtà",					// the loyalty level of this town
	"Servizi principali", 				// main facilities in this town
	"addestramento civili",				// state of civilian training in town
	"Esercito", 					// the state of the trained civilians in the town
};

// Mine strings

static const ST::string s_it_pwMineStrings[pwMineStrings_SIZE] =
{
	"Miniera",						// 0
	"Argento",
	"Oro",
	"Produzione giornaliera",
	"Produzione possibile",
	"Abbandonata",				// 5
	"Chiudi",
	"Esci",
	"Produci",
	"Stato",
	"Ammontare produzione",
	"Tipo di minerale",				// 10
	"Controllo della città",
	"Lealtà della città",
};

// blank sector strings

static const ST::string s_it_pwMiscSectorStrings[pwMiscSectorStrings_SIZE] =
{
	"Forze nemiche",
	"Settore",
	"# di oggetti",
	"Sconosciuto",
	"Controllato",
	"Sì",
	"No",
};

// error strings for inventory

static const ST::string s_it_pMapInventoryErrorString[pMapInventoryErrorString_SIZE] =
{
	"Non può selezionare quel mercenario.",  //MARK CARTER
	"%s non si trova nel settore per prendere quell'oggetto.",
	"Durante il combattimento, dovrete raccogliere gli oggetti manualmente.",
	"Durante il combattimento, dovrete rilasciare gli oggetti manualmente.",
	"%s non si trova nel settore per rilasciare quell'oggetto.",
};

static const ST::string s_it_pMapInventoryStrings[pMapInventoryStrings_SIZE] =
{
	"Posizione", 			// sector these items are in
	"Totale oggetti", 		// total number of items in sector
};


// movement menu text

static const ST::string s_it_pMovementMenuStrings[pMovementMenuStrings_SIZE] =
{
	"Muovere mercenari nel settore %s", 	// title for movement box
	"Rotta spostamento esercito", 		// done with movement menu, start plotting movement
	"Annulla", 		// cancel this menu
	"Altro",		// title for group of mercs not on squads nor in vehicles
};


static const ST::string s_it_pUpdateMercStrings[pUpdateMercStrings_SIZE] =
{
	"Oops:", 			// an error has occured
	"Scaduto contratto mercenari:", 	// this pop up came up due to a merc contract ending
	"Portato a termine incarico mercenari:", // this pop up....due to more than one merc finishing assignments
	"Mercenari di nuovo al lavoro:", // this pop up ....due to more than one merc waking up and returing to work
	"Mercenari a riposo:", // this pop up ....due to more than one merc being tired and going to sleep
	"Contratti in scadenza:", 	// this pop up came up due to a merc contract ending
};

// map screen map border buttons help text

static const ST::string s_it_pMapScreenBorderButtonHelpText[pMapScreenBorderButtonHelpText_SIZE] =
{
	"Mostra città (|w)",
	"Mostra |miniere",
	"Mos|tra squadre & nemici",
	"Mostra spazio |aereo",
	"Mostra oggett|i",
	"Mostra esercito & nemici (|Z)",
};


static const ST::string s_it_pMapScreenBottomFastHelp[pMapScreenBottomFastHelp_SIZE] =
{
	"Portati|le",
	"Tattico (|E|s|c)",
	"|Opzioni",
	"Dilata tempo (|+)", 	// time compress more
	"Comprime tempo (|-)", 	// time compress less
	"Messaggio precedente (|S|u)\nIndietro (|P|a|g|S|u)", 	// previous message in scrollable list
	"Messaggio successivo (|G|i|ù)\nAvanti (|P|a|g|G|i|ù)", 	// next message in the scrollable list
	"Inizia/Ferma tempo (|S|p|a|z|i|o)",	// start/stop time compression
};

static const ST::string s_it_pMapScreenBottomText = "Bilancio attuale"; // current balance in player bank account

static const ST::string s_it_pMercDeadString = "%s è morto.";


static const ST::string s_it_pDayStrings = "Giorno";

// the list of email sender names

static const ST::string s_it_pSenderNameList[pSenderNameList_SIZE] =
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
	"Vicki",			//10
	"Trevor",
	"Grunty",
	"Ivan",
	"Steroid",
	"Igor",			//15
	"Shadow",
	"Red",
	"Reaper",
	"Fidel",
	"Fox",				//20
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
	"Assicurazione M.I.S.",
	"Bobby Ray",
	"Capo",
	"John Kulba",
	"A.I.M.",
};


// new mail notify string
static const ST::string s_it_pNewMailStrings = "Avete una nuova E-mail...";


// confirm player's intent to delete messages

static const ST::string s_it_pDeleteMailStrings[pDeleteMailStrings_SIZE] =
{
	"Eliminate l'E-mail?",
	"Eliminate l'E-mail NON LETTA?",
};


// the sort header strings

static const ST::string s_it_pEmailHeaders[pEmailHeaders_SIZE] =
{
	"Da:",
	"Sogg.:",
	"Giorno:",
};

// email titlebar text
static const ST::string s_it_pEmailTitleText = "posta elettronica";


// the financial screen strings
static const ST::string s_it_pFinanceTitle = "Contabile aggiuntivo"; // the name we made up for the financial program in the game

static const ST::string s_it_pFinanceSummary[pFinanceSummary_SIZE] =
{
	"Crediti:", 				// credit (subtract from) to player's account
	"Debiti:", 				// debit (add to) to player's account
	"Entrate effettive di ieri:",
	"Altri depositi di ieri:",
	"Debiti di ieri:",
	"Bilancio di fine giornata:",
	"Entrate effettive di oggi:",
	"Altri depositi di oggi:",
	"Debiti di oggi:",
	"Bilancio attuale:",
	"Entrate previste:",
	"Bilancio previsto:", 		// projected balance for player for tommorow
};


// headers to each list in financial screen

static const ST::string s_it_pFinanceHeaders[pFinanceHeaders_SIZE] =
{
	"Giorno", 				// the day column
	"Crediti", 			// the credits column (to ADD money to your account)
	"Debiti",				// the debits column (to SUBTRACT money from your account)
	"Transazione", // transaction type - see TransactionText below
	"Bilancio", 		// balance at this point in time
	"Pagina", 				// page number
	"Giorno(i)", 			// the day(s) of transactions this page displays
};


static const ST::string s_it_pTransactionText[pTransactionText_SIZE] =
{
	"Interessi maturati",			// interest the player has accumulated so far
	"Deposito anonimo",
	"Tassa di transazione",
	"Arruolato %s dall'A.I.M.", // Merc was hired
	"Acquistato da Bobby Ray", 		// Bobby Ray is the name of an arms dealer
	"Acconti pagati al M.E.R.C.",
	"Deposito medico per %s", 		// medical deposit for merc
	"Analisi del profilo I.M.P.", 		// IMP is the acronym for International Mercenary Profiling
	"Assicurazione acquistata per %s",
	"Assicurazione ridotta per %s",
	"Assicurazione estesa per %s", 				// johnny contract extended
	"Assicurazione annullata %s",
	"Richiesta di assicurazione per %s", 		// insurance claim for merc
	"Est. contratto di %s per 1 giorno.", 				// entend mercs contract by a day
	"Est. %s contratto per 1 settimana.",
	"Est. %s contratto per 2 settimane.",
	"Entrata mineraria",
	"", //String nuked
	"Fiori acquistati",
	"Totale rimborso medico per %s",
	"Parziale rimborso medico per %s",
	"Nessun rimborso medico per %s",
	"Pagamento a %s",		// %s is the name of the npc being paid
	"Trasferimento fondi a %s", 			// transfer funds to a merc
	"Trasferimento fondi da %s", 		// transfer funds from a merc
	"Equipaggiamento esercito in %s", // initial cost to equip a town's militia
	"Oggetti acquistati da%s.",	//is used for the Shop keeper interface.  The dealers name will be appended to the end of the string.
	"%s soldi depositati.",
};

// helicopter pilot payment

static const ST::string s_it_pSkyriderText[pSkyriderText_SIZE] =
{
	"Skyrider è stato pagato $%d", 			// skyrider was paid an amount of money
	"A Skyrider bisogna ancora dare $%d", 		// skyrider is still owed an amount of money
	"Skyrider non ha passeggeri. Se avete intenzione di trasportare mercenari in questo settore, assegnateli prima al Veicolo/Elicottero.",
};


// strings for different levels of merc morale

static const ST::string s_it_pMoralStrings[pMoralStrings_SIZE] =
{
	"Ottimo",
	"Buono",
	"Medio",
	"Basso",
	"Panico",
	"Cattivo",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.
static const ST::string s_it_str_left_equipment   = "L'equipaggio di %s è ora disponibile a %s (%c%d).";

// Status that appears on the Map Screen

static const ST::string s_it_pMapScreenStatusStrings[pMapScreenStatusStrings_SIZE] =
{
	"Salute",
	"Energia",
	"Morale",
	"Condizione",	// the condition of the current vehicle (its "health")
	"Carburante",	// the fuel level of the current vehicle (its "energy")
};


static const ST::string s_it_pMapScreenPrevNextCharButtonHelpText[pMapScreenPrevNextCharButtonHelpText_SIZE] =
{
	"Mercenario precedente (|S|i|n)", 			// previous merc in the list
	"Mercenario successivo (|D|e|s)", 				// next merc in the list
};


static const ST::string s_it_pEtaString = "TAP"; // eta is an acronym for Estimated Time of Arrival

static const ST::string s_it_pTrashItemText[pTrashItemText_SIZE] =
{
	"Non lo vedrete mai più. Siete sicuri?", 	// do you want to continue and lose the item forever
	"Questo oggetto sembra DAVVERO importante. Siete DAVVERO SICURISSIMI di volerlo gettare via?", // does the user REALLY want to trash this item
};


static const ST::string s_it_pMapErrorString[pMapErrorString_SIZE] =
{
	"La squadra non può muoversi, se un mercenario dorme.",

//1-5
	"Muovete la squadra al primo piano.",
	"Ordini di movimento? È un settore nemico!",
	"I mercenari devono essere assegnati a una squadra o a un veicolo per potersi muovere.",
	"Non avete ancora membri nella squadra.", // you have no members, can't do anything
	"I mercenari non possono attenersi agli ordini.", // merc can't comply with your order
//6-10
	"%s ha bisogno di una scorta per muoversi. Inseritelo in una squadra che ne è provvista.", // merc can't move unescorted .. for a male
	"%s ha bisogno di una scorta per muoversi. Inseritela in una squadra che ne è provvista.", // for a female
	"Il mercenario non è ancora arrivato ad Arulco!",
	"Sembra che ci siano negoziazioni di contratto da stabilire.",
	"",
//11-15
	"Ordini di movimento? È in corso una battaglia!",
	"Siete stati vittima di un'imboscata da parte dai Bloodcat nel settore %s!",
	"Siete appena entrati in quella che sembra una tana di un Bloodcat nel settore I16!",
	"",
	"La zona SAM in %s è stata assediata.",
//16-20
	"La miniera di %s è stata assediata. La vostra entrata giornaliera è stata ridotta di %s per giorno.",
	"Il nemico ha assediato il settore %s senza incontrare resistenza.",
	"Almeno uno dei vostri mercenari non ha potuto essere affidato a questo incarico.",
	"%s non ha potuto unirsi alla %s visto che è completamente pieno",
	"%s non ha potuto unirsi alla %s visto che è troppo lontano.",
//21-25
	"La miniera di %s è stata invasa dalle forze armate di Deidranna!",
	"Le forze armate di Deidranna hanno appena invaso la zona SAM in %s",
	"Le forze armate di Deidranna hanno appena invaso %s",
	"Le forze armate di Deidranna sono appena state avvistate in %s.",
	"Le forze armate di Deidranna sono appena partite per %s.",
//26-30
	"Almeno uno dei vostri mercenari non può riposarsi.",
	"Almeno uno dei vostri mercenari non è stato svegliato.",
	"L'esercito non si farà vivo finché non avranno finito di esercitarsi.",
	"%s non possono ricevere ordini di movimento adesso.",
	"I militari che non si trovano entro i confini della città non possono essere spostati inquesto settore.",
//31-35
	"Non potete avere soldati in %s.",
	"Un veicolo non può muoversi se è vuoto!",
	"%s è troppo grave per muoversi!",
	"Prima dovete lasciare il museo!",
	"%s è morto!",
//36-40
	"%s non può andare a %s perché si sta muovendo",
	"%s non può salire sul veicolo in quel modo",
	"%s non può unirsi alla %s",
	"Non potete comprimere il tempo finché non arruolerete nuovi mercenari!",
	"Questo veicolo può muoversi solo lungo le strade!",
//41-45
	"Non potete riassegnare i mercenari che sono già in movimento",
	"Il veicolo è privo di benzina!",
	"%s è troppo stanco per muoversi.",
	"Nessuno a bordo è in grado di guidare il veicolo.",
	"Uno o più membri di questa squadra possono muoversi ora.",
//46-50
	"Uno o più degli altri mercenari non può muoversi ora.",
	"Il veicolo è troppo danneggiato!",
	"Osservate che solo due mercenari potrebbero addestrare i militari in questo settore.",
	"Il robot non può muoversi senza il suo controller. Metteteli nella stessa squadra.",
};


// help text used during strategic route plotting
static const ST::string s_it_pMapPlotStrings[pMapPlotStrings_SIZE] =
{
	"Cliccate di nuovo su una destinazione per confermare la vostra meta finale, oppure cliccate su un altro settore per fissare più tappe.",
	"Rotta di spostamento confermata.",
	"Destinazione immutata.",
	"Rotta di spostamento annullata.",
	"Rotta di spostamento accorciata.",
};


// help text used when moving the merc arrival sector
static const ST::string s_it_pBullseyeStrings[pBullseyeStrings_SIZE] =
{
	"Cliccate sul settore dove desiderate che i mercenari arrivino.",
	"OK. I mercenari che stavano arrivando si sono dileguati a %s",
	"I mercenari non possono essere trasportati, lo spazio aereo non è sicuro!",
	"Annullato. Il settore d'arrivo è immutato",
	"Lo spazio aereo sopra %s non è più sicuro! Il settore d'arrivo è stato spostato a %s.",
};


// help text for mouse regions

static const ST::string s_it_pMiscMapScreenMouseRegionHelpText[pMiscMapScreenMouseRegionHelpText_SIZE] =
{
	"Entra nell'inventario (|I|n|v|i|o)",
	"Getta via l'oggetto",
	"Esci dall'inventario (|I|n|v|i|o)",
};


static const ST::string s_it_str_he_leaves_where_drop_equipment  = "Volete che %s lasci il suo equipaggiamento dove si trova ora (%s) o in seguito a %s (%s) dopo aver preso il volo da Arulco?";
static const ST::string s_it_str_she_leaves_where_drop_equipment = "Volete che %s lasci il suo equipaggiamento dove si trova ora (%s) o in seguito a %s (%s) dopo aver preso il volo da Arulco?";
static const ST::string s_it_str_he_leaves_drops_equipment       = "%s sta per partire e spedirà il suo equipaggiamento a %s.";
static const ST::string s_it_str_she_leaves_drops_equipment      = "%s sta per partire e spedirà il suo equipaggiamento a %s.";


// Text used on IMP Web Pages

static const ST::string s_it_pImpPopUpStrings[pImpPopUpStrings_SIZE] =
{
	"Codice di autorizzazione non valido",
	"State per riiniziare l'intero processo di profilo. Ne siete certi?",
	"Inserite nome e cognome corretti oltre che al sesso",
	"L'analisi preliminare del vostro stato finanziario mostra che non potete offrire un'analisi di profilo.",
	"Opzione non valida questa volta.",
	"Per completare un profilo accurato, dovete aver spazio per almeno uno dei membri della squadra.",
	"Profilo già completato.",
};


// button labels used on the IMP site

static const ST::string s_it_pImpButtonText[pImpButtonText_SIZE] =
{
	"Cosa offriamo", 			// about the IMP site
	"INIZIO", 			// begin profiling
	"Personalità", 		// personality section
	"Attributi", 		// personal stats/attributes section
	"Ritratto", 			// the personal portrait selection
	"Voce %d", 			// the voice selection
	"Fine", 			// done profiling
	"Ricomincio", 		// start over profiling
	"Sì, scelgo la risposta evidenziata.",
	"Sì",
	"No",
	"Finito", 			// finished answering questions
	"Prec.", 			// previous question..abbreviated form
	"Avanti", 			// next question
	"SÌ, LO SONO.", 		// yes, I am certain
	"NO, VOGLIO RICOMINCIARE.", // no, I want to start over the profiling process
	"SÌ",
	"NO",
	"Indietro", 			// back one page
	"Annulla", 			// cancel selection
	"Sì, ne sono certo.",
	"No, lasciami dare un'altra occhiata.",
	"Immatricolazione", 			// the IMP site registry..when name and gender is selected
	"Analisi", 			// analyzing your profile results
	"OK",
	"Voce",
	"Specialties",			// "Specialties" - the skills selection screen
};

static const ST::string s_it_pExtraIMPStrings[pExtraIMPStrings_SIZE] =
{
	"Per completare il profilo attuale, seleziona 'Personalità'.",
	"Ora che hai completato la Personalità, seleziona i tuoi attributi.",
	"Con gli attributi ora assegnati, puoi procedere alla selezione del ritratto.",
	"Per completare il processo, seleziona il campione della voce che più ti piace.",
};

static const ST::string s_it_gzIMPSkillTraitsText[gzIMPSkillTraitsText_SIZE] =
{
	s_it_gzMercSkillText[1],
	s_it_gzMercSkillText[2],
	s_it_gzMercSkillText[3],
	s_it_gzMercSkillText[4],
	s_it_gzMercSkillText[5],
	s_it_gzMercSkillText[6],
	s_it_gzMercSkillText[7],
	s_it_gzMercSkillText[8],
	s_it_gzMercSkillText[9],
	s_it_gzMercSkillText[10],
	s_it_gzMercSkillText[13],
	s_it_gzMercSkillText[14],
	s_it_gzMercSkillText[15],
	s_it_gzMercSkillText[12],

	s_it_gzMercSkillText[0],
	"I.M.P. Specialties"
};

static const ST::string s_it_pFilesTitle = "Gestione risorse";

static const ST::string s_it_pFilesSenderList[pFilesSenderList_SIZE] =
{
	"Rapporto",      // the recon report sent to the player. Recon is an abbreviation for reconissance
	"Intercetta #1", // first intercept file .. Intercept is the title of the organization sending the file...similar in function to INTERPOL/CIA/KGB..refer to fist record in files.txt for the translated title
	"Intercetta #2", // second intercept file
	"Intercetta #3", // third intercept file
	"Intercetta #4", // fourth intercept file
	"Intercetta #5", // fifth intercept file
	"Intercetta #6", // sixth intercept file
};

// Text having to do with the History Log
static const ST::string s_it_pHistoryTitle = "Registro";

static const ST::string s_it_pHistoryHeaders[pHistoryHeaders_SIZE] =
{
	"Giorno", 			// the day the history event occurred
	"Pagina", 			// the current page in the history report we are in
	"Giorno", 			// the days the history report occurs over
	"Posizione", 			// location (in sector) the event occurred
	"Evento", 			// the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
static const ST::string s_it_pHistoryStrings[pHistoryStrings_SIZE] =
{
	"",																						// leave this line blank
	//1-5
	"%s è stato assunto dall'A.I.M.", 										// merc was hired from the aim site
	"%s è stato assunto dal M.E.R.C.", 									// merc was hired from the aim site
	"%s morì.", 															// merc was killed
	"Acconti stanziati al M.E.R.C.",								// paid outstanding bills at MERC
	"Assegno accettato da Enrico Chivaldori",
	//6-10
	"Profilo generato I.M.P.",
	"Acquistato contratto d'assicurazione per %s.", 				// insurance contract purchased
	"Annullato contratto d'assicurazione per %s.", 				// insurance contract canceled
	"Versamento per richiesta assicurazione per %s.", 							// insurance claim payout for merc
	"Esteso contratto di %s di 1 giorno.", 						// Extented "mercs name"'s for a day
	//11-15
	"Esteso contratto di %s di 1 settimana.", 					// Extented "mercs name"'s for a week
	"Esteso contratto di %s di 2 settimane.", 					// Extented "mercs name"'s 2 weeks
	"%s è stato congedato.", 													// "merc's name" was dismissed.
	"%s è partito.", 																		// "merc's name" quit.
	"avventura iniziata.", 															// a particular quest started
	//16-20
	"avventura completata.",
	"Parlato col capo minatore di %s",									// talked to head miner of town
	"Liberato %s",
	"Inganno utilizzato",
	"Il cibo dovrebbe arrivare a Omerta domani",
	//21-25
	"%s ha lasciato la squadra per diventare la moglie di Daryl Hick",
	"contratto di %s scaduto.",
	"%s è stato arruolato.",
	"Enrico si è lamentato della mancanza di progresso",
	"Vinta battaglia",
	//26-30
	"%s miniera ha iniziato a esaurire i minerali",
	"%s miniera ha esaurito i minerali",
	"%s miniera è stata chiusa",
	"%s miniera è stata riaperta",
	"Trovata una prigione chiamata Tixa.",
	//31-35
	"Sentito di una fabbrica segreta di armi chiamata Orta.",
	"Alcuni scienziati a Orta hanno donato una serie di lanciamissili.",
	"La regina Deidranna ha bisogno di cadaveri.",
	"Frank ha parlato di scontri a San Mona.",
	"Un paziente pensa che lui abbia visto qualcosa nella miniera.",
	//36-40
	"Incontrato qualcuno di nome Devin - vende esplosivi.",
	"Imbattutosi nel famoso ex-mercenario dell'A.I.M. Mike!",
	"Incontrato Tony - si occupa di armi.",
	"Preso un lanciamissili dal Sergente Krott.",
	"Concessa a Kyle la licenza del negozio di pelle di Angel.",
	//41-45
	"Madlab ha proposto di costruire un robot.",
	"Gabby può effettuare operazioni di sabotaggio contro sistemi d'allarme.",
	"Keith è fuori dall'affare.",
	"Howard ha fornito cianuro alla regina Deidranna.",
	"Incontrato Keith - si occupa di un po' di tutto a Cambria.",
	//46-50
	"Incontrato Howard - si occupa di farmaceutica a Balime",
	"Incontrato Perko - conduce una piccola impresa di riparazioni.",
	"Incontrato Sam di Balime - ha un negozio di hardware.",
	"Franz si occupa di elettronica e altro.",
	"Arnold possiede un'impresa di riparazioni a Grumm.",
	//51-55
	"Fredo si occupa di elettronica a Grumm.",
	"Donazione ricevuta da un ricco ragazzo a Balime.",
	"Incontrato un rivenditore di un deposito di robivecchi di nome Jake.",
	"Alcuni vagabondi ci hanno dato una scheda elettronica.",
	"Corrotto Walter per aprire la porta del seminterrato.",
	//56-60
	"Se Dave ha benzina, potrà fare il pieno gratis.",
	"Corrotto Pablo.",
	"Kingpin tiene i soldi nella miniera di San Mona.",
	"%s ha vinto il Combattimento Estremo",
	"%s ha perso il Combattimento Estremo",
	//61-65
	"%s è stato squalificato dal Combattimento Estremo",
	"trovati moltissimi soldi nascosti nella miniera abbandonata.",
	"Incontrato assassino ingaggiato da Kingpin.",
	"Perso il controllo del settore",				//ENEMY_INVASION_CODE
	"Difeso il settore",
	//66-70
	"Persa la battaglia",							//ENEMY_ENCOUNTER_CODE
	"Imboscata fatale",						//ENEMY_AMBUSH_CODE
	"Annientata imboscata nemica",
	"Attacco fallito",			//ENTERING_ENEMY_SECTOR_CODE
	"Attacco riuscito!",
	//71-75
	"Creature attaccate",			//CREATURE_ATTACK_CODE
	"Ucciso dai Bloodcat",			//BLOODCAT_AMBUSH_CODE
	"Massacrati dai Bloodcat",
	"%s è stato ucciso",
	"Data a Carmen la testa di un terrorista",
	"Massacro sinistro",
	"Ucciso %s",
};

static const ST::string s_it_pHistoryLocations = "N/A"; // N/A is an acronym for Not Applicable

// icon text strings that appear on the laptop

static const ST::string s_it_pLaptopIcons[pLaptopIcons_SIZE] =
{
	"E-mail",
	"Rete",
	"Finanza",
	"Personale",
	"Cronologia",
	"File",
	"Chiudi",
	"sir-FER 4.0",			// our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added

static const ST::string s_it_pBookMarkStrings[pBookMarkStrings_SIZE] =
{
	"A.I.M.",
	"Bobby Ray",
	"I.M.P",
	"M.E.R.C.",
	"Pompe funebri",
	"Fiorista",
	"Assicurazione",
	"Annulla",
};

// When loading or download a web page

static const ST::string s_it_pDownloadString[pDownloadString_SIZE] =
{
	"Caricamento",
	"Caricamento",
};

//This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

static const ST::string s_it_gsAtmStartButtonText[gsAtmStartButtonText_SIZE] =
{
	"Stato", 			// view stats of the merc
	"Inventario", 			// view the inventory of the merc
	"Impiego",
};

// Web error messages. Please use foreign language equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator

static const ST::string s_it_pErrorStrings = "Connessione intermittente all'host. Tempi d'attesa più lunghi per il trasferimento.";


static const ST::string s_it_pPersonnelString = "Mercenari:"; // mercs we have


static const ST::string s_it_pWebTitle = "sir-FER 4.0"; // our name for the version of the browser, play on company name


// The titles for the web program title bar, for each page loaded

static const ST::string s_it_pWebPagesTitles[pWebPagesTitles_SIZE] =
{
	"A.I.M.",
	"Membri dell'A.I.M.",
	"Ritratti A.I.M.",		// a mug shot is another name for a portrait
	"Categoria A.I.M.",
	"A.I.M.",
	"Membri dell'A.I.M.",
	"Tattiche A.I.M.",
	"Storia A.I.M.",
	"Collegamenti A.I.M.",
	"M.E.R.C.",
	"Conti M.E.R.C.",
	"Registrazione M.E.R.C.",
	"Indice M.E.R.C.",
	"Bobby Ray",
	"Bobby Ray - Armi",
	"Bobby Ray - Munizioni",
	"Bobby Ray - Giubb. A-P",
	"Bobby Ray - Varie",							//misc is an abbreviation for miscellaneous
	"Bobby Ray - Usato",
	"Bobby Ray - Ordine Mail",
	"I.M.P.",
	"I.M.P.",
	"Servizio Fioristi Riuniti",
	"Servizio Fioristi Riuniti - Galleria",
	"Servizio Fioristi Riuniti - Ordine",
	"Servizio Fioristi Riuniti - Card Gallery",
	"Agenti assicurativi Malleus, Incus & Stapes",
	"Informazione",
	"Contratto",
	"Commenti",
	"Servizio di pompe funebri di McGillicutty",
	"URL non ritrovato.",
	"Bobby Ray - Spedizioni recenti",
	"",
	"",
};

static const ST::string s_it_pShowBookmarkString[pShowBookmarkString_SIZE] =
{
	"Aiuto",
	"Cliccate su Rete un'altra volta per i segnalibri.",
};

static const ST::string s_it_pLaptopTitles[pLaptopTitles_SIZE] =
{
	"Cassetta della posta",
	"Gestione risorse",
	"Personale",
	"Contabile aggiuntivo",
	"Ceppo storico",
};

static const ST::string s_it_pPersonnelDepartedStateStrings[pPersonnelDepartedStateStrings_SIZE] =
{
	//reasons why a merc has left.
	"Ucciso in azione",
	"Licenziato",
	"Sposato",
	"Contratto Scaduto",
	"Liberato",
};
// personnel strings appearing in the Personnel Manager on the laptop

static const ST::string s_it_pPersonelTeamStrings[pPersonelTeamStrings_SIZE] =
{
	"Squadra attuale",
	"Partenze",
	"Costo giornaliero:",
	"Costo più alto:",
	"Costo più basso:",
	"Ucciso in azione:",
	"Licenziato:",
	"Altro:",
};


static const ST::string s_it_pPersonnelCurrentTeamStatsStrings[pPersonnelCurrentTeamStatsStrings_SIZE] =
{
	"Più basso",
	"Normale",
	"Più alto",
};


static const ST::string s_it_pPersonnelTeamStatsStrings[pPersonnelTeamStatsStrings_SIZE] =
{
	"SAL",
	"AGI",
	"DES",
	"FOR",
	"COM",
	"SAG",
	"LIV",
	"TIR",
	"MEC",
	"ESP",
	"PS",
};


// horizontal and vertical indices on the map screen

static const ST::string s_it_pMapVertIndex[pMapVertIndex_SIZE] =
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

static const ST::string s_it_pMapHortIndex[pMapHortIndex_SIZE] =
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

static const ST::string s_it_pMapDepthIndex[pMapDepthIndex_SIZE] =
{
	"",
	"-1",
	"-2",
	"-3",
};

// text that appears on the contract button

static const ST::string s_it_pContractButtonString = "Contratto";

// text that appears on the update panel buttons

static const ST::string s_it_pUpdatePanelButtons[pUpdatePanelButtons_SIZE] =
{
	"Continua",
	"Fermati",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle

static const ST::string s_it_LargeTacticalStr[LargeTacticalStr_SIZE] =
{
	"Siete stati sconfitti in questo settore!",
	"Il nemico, non avendo alcuna pietà delle anime della squadra, divorerà ognuno di voi!",
	"I membri inconscenti della vostra squadra sono stati catturati!",
	"I membri della vostra squadra sono stati fatti prigionieri dal nemico.",
};


//Insurance Contract.c
//The text on the buttons at the bottom of the screen.

static const ST::string s_it_InsContractText[InsContractText_SIZE] =
{
	"Indietro",
	"Avanti",
	"Accetta",
	"Pulisci",
};



//Insurance Info
// Text on the buttons on the bottom of the screen

static const ST::string s_it_InsInfoText[InsInfoText_SIZE] =
{
	"Indietro",
	"Avanti",
};



//For use at the M.E.R.C. web site. Text relating to the player's account with MERC

static const ST::string s_it_MercAccountText[MercAccountText_SIZE] =
{
	// Text on the buttons on the bottom of the screen
	"Autorizza",
	"Home Page",
	"Conto #:",
	"Merc",
	"Giorni",
	"Tasso",	//5
	"Costo",
	"Totale:",
	"Conferma il pagamento di %s?",		//the %s is a string that contains the dollar amount ( ex. "$150" )
};



//For use at the M.E.R.C. web site. Text relating a MERC mercenary


static const ST::string s_it_MercInfo[MercInfo_SIZE] =
{
	"Indietro",
	"Ricompensa",
	"Successivo",
	"Info. addizionali",
	"Home Page",
	"Assoldato",
	"Salario:",
	"Al giorno",
	"Deceduto",

	"Sembra che state arruolando troppi mercenari. Il vostro limite è di 18.",
	"Non disponibile",
};



// For use at the M.E.R.C. web site. Text relating to opening an account with MERC

static const ST::string s_it_MercNoAccountText[MercNoAccountText_SIZE] =
{
	//Text on the buttons at the bottom of the screen
	"Apri conto",
	"Annulla",
	"Non hai alcun conto. Vuoi aprirne uno?",
};



// For use at the M.E.R.C. web site. MERC Homepage

static const ST::string s_it_MercHomePageText[MercHomePageText_SIZE] =
{
	//Description of various parts on the MERC page
	"Speck T. Kline, fondatore e proprietario",
	"Per aprire un conto, cliccate qui",
	"Per visualizzare un conto, cliccate qui",
	"Per visualizzare i file, cliccate qui",
	// The version number on the video conferencing system that pops up when Speck is talking
	"Speck Com v3.2",
};

// For use at MiGillicutty's Web Page.

static const ST::string s_it_sFuneralString[sFuneralString_SIZE] =
{
	"Impresa di pompe funebri di McGillicutty: Il dolore delle famiglie che hanno fornito il loro aiuto dal 1983.",
	"Precedentemente mercenario dell'A.I.M. Murray \"Pops\" McGillicutty è un impresario di pompe funebri qualificato e con molta esperienza.",
	"Essendo coinvolto profondamente nella morte e nel lutto per tutta la sua vita, Pops sa quanto sia difficile affrontarli.",
	"L'impresa di pompe funebri di McGillicutty offre una vasta gamma di servizi funebri, da una spalla su cui piangere a ricostruzioni post-mortem per corpi mutilati o sfigurati.",
	"Lasciate che l'impresa di pompe funebri di McGillicutty vi aiuti e i vostri amati riposeranno in pace.",

	// Text for the various links available at the bottom of the page
	"SPEDISCI FIORI",
	"ESPOSIZIONE DI BARE & URNE",
	"SERVIZI DI CREMAZIONE",
	"SERVIZI PRE-FUNEBRI",
	"CERIMONIALE FUNEBRE",

	// The text that comes up when you click on any of the links ( except for send flowers ).
	"Purtroppo, il resto di questo sito non è stato completato a causa di una morte in famiglia. Aspettando la lettura del testamento e la riscossione dell'eredità, il sito verrà ultimato non appena possibile.",
	"Vi porgiamo, comunque, le nostre condolianze in questo momento di dolore. Contatteci ancora.",
};

// Text for the florist Home Page

static const ST::string s_it_sFloristText[sFloristText_SIZE] =
{
	//Text on the button on the bottom of the page

	"Galleria",

	//Address of United Florist

	"\"Ci lanciamo col paracadute ovunque\"",
	"1-555-SCENT-ME",
	"333 Dot. NoseGay, Seedy City, CA USA 90210",
	"http://www.scent-me.com",

	// detail of the florist page

	"Siamo veloci ed efficienti!",
	"Consegna il giorno successivo in quasi tutto il mondo, garantito. Applicate alcune restrizioni.",
	"I prezzi più bassi in tutto il mondo, garantito!",
	"Mostrateci un prezzo concorrente più basso per qualsiasi progetto, e riceverete una dozzina di rose, gratuitamente.",
	"Flora, fauna & fiori in volo dal 1981.",
	"I nostri paracadutisti decorati ex-bomber lanceranno il vostro bouquet entro un raggio di dieci miglia dalla locazione richiesta. Sempre e ovunque!",
	"Soddisfiamo la vostra fantasia floreale.",
	"Lasciate che Bruce, il nostro esperto in composizioni floreali, selezioni con cura i fiori più freschi e della migliore qualità dalle nostre serre più esclusive.",
	"E ricordate, se non l'abbiamo, possiamo coltivarlo - E subito!",
};



//Florist OrderForm

static const ST::string s_it_sOrderFormText[sOrderFormText_SIZE] =
{
	//Text on the buttons

	"Indietro",
	"Spedisci",
	"Home",
	"Galleria",

	"Nome del bouquet:",
	"Prezzo:",			//5
	"Numero ordine:",
	"Data consegna",
	"gior. succ.",
	"arriva quando arriva",
	"Luogo consegna",			//10
	"Servizi aggiuntivi",
	"Bouquet schiacciato ($10)",
	"Rose nere ($20)",
	"Bouquet appassito ($10)",
	"Torta di frutta (se disponibile 10$)",		//15
	"Sentimenti personali:",
	"Il vostro messaggio non può essere più lungo di 75 caratteri.",
	"... oppure sceglietene uno dai nostri",

	"BIGLIETTI STANDARD",
	"Informazioni sulla fatturazione",//20

	//The text that goes beside the area where the user can enter their name

	"Nome:",
};




//Florist Gallery.c

static const ST::string s_it_sFloristGalleryText[sFloristGalleryText_SIZE] =
{
	//text on the buttons

	"Prec.",	//abbreviation for previous
	"Succ.",	//abbreviation for next

	"Clicca sul modello che vuoi ordinare.",
	"Ricorda: c'è un supplemento di 10$ per tutti i bouquet appassiti o schiacciati.",

	//text on the button

	"Home",
};

//Florist Cards

static const ST::string s_it_sFloristCards[sFloristCards_SIZE] =
{
	"Cliccate sulla vostra selezione",
	"Indietro",
};



// Text for Bobby Ray's Mail Order Site

static const ST::string s_it_BobbyROrderFormText[BobbyROrderFormText_SIZE] =
{
	"Ordine",				//Title of the page
	"Qta",					// The number of items ordered
	"Peso (%s)",			// The weight of the item
	"Nome oggetto",				// The name of the item
	"Prezzo unit.",				// the item's weight
	"Totale",				//5	// The total price of all of items of the same type
	"Sotto-totale",				// The sub total of all the item totals added
	"S&C (Vedete luogo consegna)",		// S&H is an acronym for Shipping and Handling
	"Totale finale",			// The grand total of all item totals + the shipping and handling
	"Luogo consegna",
	"Spedizione veloce",			//10	// See below
	"Costo (per %s.)",			// The cost to ship the items
	"Espresso di notte",			// Gets deliverd the next day
	"2 giorni d'affari",			// Gets delivered in 2 days
	"Servizio standard",			// Gets delivered in 3 days
	"Annulla ordine",//15			// Clears the order page
	"Accetta ordine",			// Accept the order
	"Indietro",				// text on the button that returns to the previous page
	"Home Page",				// Text on the button that returns to the home Page
	"* Indica oggetti usati",		// Disclaimer stating that the item is used
	"Non potete permettervi di pagare questo.",		//20	// A popup message that to warn of not enough money
	"<NESSUNO>",				// Gets displayed when there is no valid city selected
	"Siete sicuri di volere spedire quest'ordine a %s?",		// A popup that asks if the city selected is the correct one
	"peso del pacco**",			// Displays the weight of the package
	"** Peso min.",				// Disclaimer states that there is a minimum weight for the package
	"Spedizioni",
};


// This text is used when on the various Bobby Ray Web site pages that sell items

static const ST::string s_it_BobbyRText[BobbyRText_SIZE] =
{
	"Ordini:",				// Title
	// instructions on how to order
	"Cliccate sull'oggetto. Sinistro per aggiungere pezzi, destro per toglierne. Una volta selezionata la quantità, procedete col nuovo ordine.",

	//Text on the buttons to go the various links

	"Oggetti prec.",		//
	"Armi", 			//3
	"Munizioni",			//4
	"Giubb. A-P",			//5
	"Varie",			//6	//misc is an abbreviation for miscellaneous
	"Usato",			//7
	"Oggetti succ.",
	"ORDINE",
	"Home Page",			//10

	//The following lines provide information on the items

	"Peso:",		// Weight of all the items of the same type
	"Cal.:",			// the caliber of the gun
	"Mag.:",			// number of rounds of ammo the Magazine can hold
	"Git.:",			// The range of the gun
	"Dan.:",			// Damage of the weapon
	"FFA:",			// Weapon's Rate Of Fire, acronym ROF
	"Costo:",			// Cost of the item
	"Inventario:",			// The number of items still in the store's inventory
	"Num. ordine:",		// The number of items on order
	"Danneggiato",			// If the item is damaged
	"Totale:",			// The total cost of all items on order
	"* funzionale al %",		// if the item is damaged, displays the percent function of the item

	//Popup that tells the player that they can only order 10 items at a time

	"Darn! Quest'ordine qui accetterà solo 10 oggetti. Se avete intenzione di ordinare più merce (ed è quello che speriamo), fate un ordine a parte e accettate le nostre scuse.",

	// A popup that tells the user that they are trying to order more items then the store has in stock

	"Ci dispiace. Non disponiamo più di questo articolo. Riprovate più tardi.",

	//A popup that tells the user that the store is temporarily sold out

	"Ci dispiace, ma siamo momentaneamente sprovvisti di oggetti di questo genere.",
};


// The following line is used on the Ammunition page.  It is used for help text
// to display how many items the player's merc has that can use this type of
// ammo.
static const ST::string s_it_str_bobbyr_guns_num_guns_that_use_ammo = "La vostra squadra ha %d arma(i) che usa(no) questo tipo di munizioni";


// Text for Bobby Ray's Home Page

static const ST::string s_it_BobbyRaysFrontText[BobbyRaysFrontText_SIZE] =
{
	//Details on the web site

	"Questo è il negozio con la fornitura militare e le armi più recenti e potenti!",
	"Possiamo trovare la soluzione perfetta per tutte le vostre esigenze riguardo agli esplosivi.",
	"Oggetti usati e riparati",

	//Text for the various links to the sub pages

	"Varie",
	"ARMI",
	"MUNIZIONI",		//5
	"GIUBB. A-P",

	//Details on the web site

	"Se non lo vendiamo, non potrete averlo!",
	"In costruzione",
};



// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug shot page

static const ST::string s_it_AimSortText[AimSortText_SIZE] =
{
	"Membri dell'A.I.M.",				// Title
	// Title for the way to sort
	"Ordine per:",

	//Text of the links to other AIM pages

	"Visualizza le facce dei mercenari disponibili",
	"Rivedi il file di ogni singolo mercenario",
	"Visualizza la galleria degli associati dell'A.I.M."
};


// text to display how the entries will be sorted
static const ST::string s_it_str_aim_sort_price        = "Prezzo";
static const ST::string s_it_str_aim_sort_experience   = "Esperienza";
static const ST::string s_it_str_aim_sort_marksmanship = "Mira";
static const ST::string s_it_str_aim_sort_medical      = "Pronto socc.";
static const ST::string s_it_str_aim_sort_explosives   = "Esplosivi";
static const ST::string s_it_str_aim_sort_mechanical   = "Meccanica";
static const ST::string s_it_str_aim_sort_ascending    = "Crescente";
static const ST::string s_it_str_aim_sort_descending   = "Decrescente";


//Aim Policies.c
//The page in which the AIM policies and regulations are displayed

static const ST::string s_it_AimPolicyText[AimPolicyText_SIZE] =
{
	// The text on the buttons at the bottom of the page

	"Indietro",
	"Home Page",
	"Indice",
	"Avanti",
	"Disaccordo",
	"Accordo",
};



//Aim Member.c
//The page in which the players hires AIM mercenaries

// Instructions to the user to either start video conferencing with the merc, or to go the mug shot index

static const ST::string s_it_AimMemberText[AimMemberText_SIZE] =
{
	"Clic sinistro",
	"per contattarlo",
	"Clic destro",
	"per i mercenari disponibili.",
};

//Aim Member.c
//The page in which the players hires AIM mercenaries

static const ST::string s_it_CharacterInfo[CharacterInfo_SIZE] =
{
	// the contract expenses' area

	"Paga",
	"Durata",
	"1 giorno",
	"1 settimana",
	"2 settimane",

	// text for the buttons that either go to the previous merc,
	// start talking to the merc, or go to the next merc

	"Indietro",
	"Contratto",
	"Avanti",

	"Ulteriori informazioni",				// Title for the additional info for the merc's bio
	"Membri attivi", // Title of the page
	"Dispositivo opzionale:",				// Displays the optional gear cost
	"Deposito MEDICO richiesto",			// If the merc required a medical deposit, this is displayed
};


//Aim Member.c
//The page in which the player's hires AIM mercenaries

//The following text is used with the video conference popup

static const ST::string s_it_VideoConfercingText[VideoConfercingText_SIZE] =
{
	"Costo del contratto:",				//Title beside the cost of hiring the merc

	//Text on the buttons to select the length of time the merc can be hired

	"1 giorno",
	"1 settimana",
	"2 settimane",

	//Text on the buttons to determine if you want the merc to come with the equipment

	"Nessun equip.",
	"Compra equip.",

	// Text on the Buttons

	"TRASFERISCI FONDI",			// to actually hire the merc
	"ANNULLA",				// go back to the previous menu
	"ARRUOLA",				// go to menu in which you can hire the merc
	"TACI",				// stops talking with the merc
	"OK",
	"LASCIA MESSAGGIO",			// if the merc is not there, you can leave a message

	//Text on the top of the video conference popup

	"Videoconferenza con",
	"Connessione...",

	"con deposito"			// Displays if you are hiring the merc with the medical deposit
};



//Aim Member.c
//The page in which the player hires AIM mercenaries

// The text that pops up when you select the TRANSFER FUNDS button

static const ST::string s_it_AimPopUpText[AimPopUpText_SIZE] =
{
	"TRASFERIMENTO ELETTRONICO FONDI RIUSCITO",	// You hired the merc
	"NON IN GRADO DI TRASFERIRE",		// Player doesn't have enough money, message 1
	"FONDI INSUFFICIENTI",				// Player doesn't have enough money, message 2

	// if the merc is not available, one of the following is displayed over the merc's face

	"In missione",
	"Lascia messaggio",
	"Deceduto",

	//If you try to hire more mercs than game can support

	"Avete già una squadra di 18 mercenari.",

	"Messaggio già registrato",
	"Messaggio registrato",
};


//AIM Link.c

static const ST::string s_it_AimLinkText = "Collegamenti dell'A.I.M."; // The title of the AIM links page



//Aim History

// This page displays the history of AIM

static const ST::string s_it_AimHistoryText[AimHistoryText_SIZE] =
{
	"Storia dell'A.I.M.",					//Title

	// Text on the buttons at the bottom of the page

	"Indietro",
	"Home Page",
	"Associati",
	"Avanti",
};


//Aim Mug Shot Index

//The page in which all the AIM members' portraits are displayed in the order selected by the AIM sort page.

static const ST::string s_it_AimFiText[AimFiText_SIZE] =
{
	// displays the way in which the mercs were sorted

	"Prezzo",
	"Esperienza",
	"Mira",
	"Pronto socc.",
	"Esplosivi",
	"Meccanica",

	// The title of the page, the above text gets added at the end of this text

	"Membri scelti dell'A.I.M. in ordine crescente secondo %s",
	"Membri scelti dell'A.I.M. in ordine decrescente secondo %s",

	// Instructions to the players on what to do

	"Clic sinistro",
	"Per scegliere un mercenario.",			//10
	"Clic destro",
	"Per selezionare opzioni",

	// Gets displayed on top of the merc's portrait if they are...

	"Deceduto",						//14
	"In missione",
};



//AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer with AIM

static const ST::string s_it_AimAlumniText[AimAlumniText_SIZE] =
{
	// Text of the buttons

	"PAGINA 1",
	"PAGINA 2",
	"PAGINA 3",

	"Membri dell'A.I.M.",	// Title of the page

	"FINE"			// Stops displaying information on selected merc
};






//AIM Home Page

static const ST::string s_it_AimScreenText[AimScreenText_SIZE] =
{
	// AIM disclaimers

	"A.I.M. e il logo A.I.M. sono marchi registrati in diversi paesi.",
	"Di conseguenza, non cercate di copiarci.",
	"Copyright 1998-1999 A.I.M., Ltd. Tutti i diritti riservati.",

	//Text for an advertisement that gets displayed on the AIM page

	"Servizi riuniti floreali",
	"\"Atterriamo col paracadute ovunque\"",				//10
	"Fallo bene",
	"... la prima volta",
	"Se non abbiamo armi e oggetti, non ne avrete bisogno.",
};


//Aim Home Page

static const ST::string s_it_AimBottomMenuText[AimBottomMenuText_SIZE] =
{
	//Text for the links at the bottom of all AIM pages
	"Home Page",
	"Membri",
	"Associati",
	"Assicurazioni",
	"Storia",
	"Collegamenti",
};



//ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.

static const ST::string s_it_SKI_Text[SKI_SIZE ] =
{
	"MERCANZIA IN STOCK",		//Header for the merchandise available
	"PAGINA",				//The current store inventory page being displayed
	"COSTO TOTALE",				//The total cost of the the items in the Dealer inventory area
	"VALORE TOTALE",			//The total value of items player wishes to sell
	"STIMATO",				//Button text for dealer to evaluate items the player wants to sell
	"TRANSAZIONE",			//Button text which completes the deal. Makes the transaction.
	"FINE",				//Text for the button which will leave the shopkeeper interface.
	"COSTO DI RIPARAZIONE",			//The amount the dealer will charge to repair the merc's goods
	"1 ORA",			// SINGULAR! The text underneath the inventory slot when an item is given to the dealer to be repaired
	"%d ORE",		// PLURAL!   The text underneath the inventory slot when an item is given to the dealer to be repaired
	"RIPARATO",		// Text appearing over an item that has just been repaired by a NPC repairman dealer
	"Non c'è abbastanza spazio nel vostro margine di ordine.",	//Message box that tells the user there is no more room to put there stuff
	"%d MINUTI",		// The text underneath the inventory slot when an item is given to the dealer to be repaired
	"Lascia oggetto a terra.",
};


static const ST::string s_it_SkiMessageBoxText[SkiMessageBoxText_SIZE] =
{
	"Volete sottrarre %s dal vostro conto principale per coprire la differenza?",
	"Fondi insufficienti. Avete pochi %s",
	"Volete sottrarre %s dal vostro conto principale per coprire la spesa?",
	"Rivolgetevi all'operatore per iniziare la transazione",
	"Rivolgetevi all'operatore per riparare gli oggetti selezionati",
	"Fine conversazione",
	"Bilancio attuale",
};


//OptionScreen.c

static const ST::string s_it_zOptionsText[zOptionsText_SIZE] =
{
	//button Text
	"Salva partita",
	"Carica partita",
	"Abbandona",
	"Fine",

	//Text above the slider bars
	"Effetti",
	"Parlato",
	"Musica",

	//Confirmation pop when the user selects..
	"Volete terminare la partita e tornare al menu principale?",

	"Avete bisogno dell'opzione 'Parlato' o di quella 'Sottotitoli' per poter giocare.",
};


//SaveLoadScreen
static const ST::string s_it_zSaveLoadText[zSaveLoadText_SIZE] =
{
	"Salva partita",
	"Carica partita",
	"Annulla",
	"Salvamento selezionato",
	"Caricamento selezionato",

	"Partita salvata con successo",
	"ERRORE durante il salvataggio!",
	"Partita caricata con successo",
	"ERRORE durante il caricamento: \"%s\"",

	"La versione del gioco nel file della partita salvata è diverso dalla versione attuale. È abbastanza sicuro proseguire. Continuate?",
	"I file della partita salvata potrebbero essere annullati. Volete cancellarli tutti?",

	"Tentativo di caricare una versione salvata più vecchia. Aggiornate e caricate automaticamente quella salvata?",

	"Tentativo di caricare una vecchia versione salvata. Aggiornate e caricate automaticamente quella salvata?",

	"Siete sicuri di volere sovrascrivere la partita salvata nello slot #%d?",


	//The first %d is a number that contains the amount of free space on the users hard drive,
	//the second is the recommended amount of free space.
	"Lo spazio su disco si sta esaurendo. Sono disponibili solo %d MB, mentre per giocare a Jagged dovrebbero esserci almeno %d MB liberi .",

	"Salvataggio in corso...",			//When saving a game, a message box with this string appears on the screen

	"Armi normali",
	"Tonn. di armi",
	"Stile realistico",
	"Stile fantascientifico",

	"Difficoltà",
};



//MapScreen
static const ST::string s_it_zMarksMapScreenText[zMarksMapScreenText_SIZE] =
{
	"Livello mappa",
	"Non avete soldati. Avete bisogno di addestrare gli abitanti della città per poter disporre di un esercito cittadino.",
	"Entrata giornaliera",
	"Il mercenario ha l'assicurazione sulla vita",
	"%s non è stanco.",
	"%s si sta muovendo e non può riposare",
	"%s è troppo stanco, prova un po' più tardi.",
	"%s sta guidando.",
	"La squadra non può muoversi, se un mercenario dorme.",

	// stuff for contracts
	"Visto che non potete pagare il contratto, non avete neanche i soldi per coprire il premio dell'assicurazione sulla vita di questo nercenario.",
	"%s premio dell'assicurazione costerà %s per %d giorno(i) extra. Volete pagare?",
	"Settore inventario",
	"Il mercenario ha una copertura medica.",

	// other items
	"Medici", // people acting a field medics and bandaging wounded mercs
	"Pazienti", // people who are being bandaged by a medic
	"Fine", // Continue on with the game after autobandage is complete
	"Ferma", // Stop autobandaging of patients by medics now
	"%s non ha un kit di riparazione.",
	"%s non ha un kit di riparazione.",
	"Non ci sono abbastanza persone che vogliono essere addestrate ora.",
	"%s è pieno di soldati.",
	"Il mercenario ha un contratto a tempo determinato.",
	"Il contratto del mercenario non è assicurato",
};


static const ST::string s_it_pLandMarkInSectorString = "La squadra %d ha notato qualcuno nel settore %s";

// confirm the player wants to pay X dollars to build a militia force in town
static const ST::string s_it_pMilitiaConfirmStrings[pMilitiaConfirmStrings_SIZE] =
{
	"Addestrare una squadra dell'esercito cittadino costerà $", // telling player how much it will cost
	"Approvate la spesa?", // asking player if they wish to pay the amount requested
	"Non potete permettervelo.", // telling the player they can't afford to train this town
	"Continuate ad aeddestrare i soldati in %s (%s %d)?", // continue training this town?
	"Costo $", // the cost in dollars to train militia
	"(S/N)",   // abbreviated yes/no
	"Addestrare l'esrecito cittadino nei settori di %d costerà $ %d. %s", // cost to train sveral sectors at once
	"Non potete permettervi il $%d per addestrare l'esercito cittadino qui.",
	"%s ha bisogno di una percentuale di %d affinché possiate continuare ad addestrare i soldati.",
	"Non potete più addestrare i soldati a %s.",
};


//Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom of the single merc panel
static const ST::string s_it_gzMoneyWithdrawMessageText[gzMoneyWithdrawMessageText_SIZE] =
{
	"Potete prelevare solo fino a $20,000 alla volta.",
	"Sieti sicuri di voler depositare il %s sul vostro conto?",
};

static const ST::string s_it_gzCopyrightText = "Copyright (C) 1999 Sir-tech Canada Ltd. Tutti i diritti riservati.";

//option Text
static const ST::string s_it_zOptionsToggleText[zOptionsToggleText_SIZE] =
{
	"Parlato",
	"Conferme mute",
	"Sottotitoli",
	"Mettete in pausa il testo del dialogo",
	"Fumo dinamico",
	"Sangue e violenza",
	"Non è necessario usare il mouse!",
	"Vecchio metodo di selezione",
	"Mostra il percorso dei mercenari",
	"Mostra traiettoria colpi sbagliati",
	"Conferma in tempo reale",
	"Visualizza gli avvertimenti sveglio/addormentato",
	"Utilizza il sistema metrico",
	"Tragitto illuminato durante gli spostamenti",
	"Sposta il cursore sui mercenari",
	"Sposta il cursore sulle porte",
	"Evidenzia gli oggetti",
	"Mostra le fronde degli alberi",
	"Mostra strutture",
	"Mostra il cursore 3D",
};

//This is the help text associated with the above toggles.
static const ST::string s_it_zOptionsScreenHelpText[zOptionsToggleText_SIZE] =
{
	//speech
	"Attivate questa opzione, se volete ascoltare il dialogo dei personaggi.",

	//Mute Confirmation
	"Attivate o disattivate le conferme verbali dei personaggi.",

		//Subtitles
	"Controllate se il testo su schermo viene visualizzato per il dialogo.",

	//Key to advance speech
	"Se i sottotitoli sono attivati, utilizzate questa opzione per leggere tranquillamente i dialoghi NPC.",

	//Toggle smoke animation
	"Disattivate questa opzione, se il fumo dinamico diminuisce la frequenza d'aggiornamento.",

	//Blood n Gore
	"Disattivate questa opzione, se il sangue vi disturba.",

	//Never move my mouse
	"Disattivate questa opzione per muovere automaticamente il mouse sulle finestre a comparsa di conferma al loro apparire.",

	//Old selection method
	"Attivate questa opzione per selezionare i personaggi e muoverli come nel vecchio JA (dato che la funzione è stata invertita).",

	//Show movement path
	"Attivate questa opzione per visualizzare i sentieri di movimento in tempo reale (oppure disattivatela utilizzando il tasto MAIUSC).",

	//show misses
	"Attivate per far sì che la partita vi mostri dove finiscono i proiettili quando \"sbagliate\".",

	//Real Time Confirmation
	"Se attivata, sarà richiesto un altro clic su \"salva\" per il movimento in tempo reale.",

	//Sleep/Wake notification
	"Se attivata, verrete avvisati quando i mercenari in \"servizio\" vanno a riposare e quando rientrano in servizio.",

	//Use the metric system
	"Se attivata, utilizza il sistema metrico di misurazione; altrimenti ricorre al sistema britannico.",

	//Merc Lighted movement
	"Se attivata, il mercenario mostrerà il terreno su cui cammina. Disattivatela per un aggiornamento più veloce.",

	//Smart cursor
	"Se attivata, muovendo il cursore vicino ai vostri mercenari li evidenzierà automaticamente.",

	//snap cursor to the door
	"Se attivata, muovendo il cursore vicino a una porta farà posizionare automaticamente il cursore sopra di questa.",

	//glow items
	"Se attivata, l'opzione evidenzierà gli |Oggetti automaticamente.",

	//toggle tree tops
	"Se attivata, mostra le |fronde degli alberi.",

	//toggle wireframe
	"Se attivata, visualizza le |Strutture dei muri nascosti.",

	"Se attivata, il cursore di movimento verrà mostrato in 3D (|H|o|m|e).",

};


static const ST::string s_it_gzGIOScreenText[gzGIOScreenText_SIZE] =
{
	"INSTALLAZIONE INIZIALE DEL GIOCO",
	"Versione del gioco",
	"Realistica",
	"Fantascientifica",
	"Opzioni delle armi",
	"Varietà di armi",
	"Normale",
	"Livello di difficoltà",
	"Principiante",
	"Esperto",
	"Professionista",
	"Ok",
	"Annulla",
	"Difficoltà extra",
	"Tempo illimitato",
	"Turni a tempo",
	"Dead is Dead",
};

// This string is used in the IMP character generation.  It is possible to
// select 0 ability in a skill meaning you can't use it.  This text is
// confirmation to the player.
static const ST::string s_it_pSkillAtZeroWarning    = "Siete sicuri? Un valore di zero significa NESSUNA abilità.";
static const ST::string s_it_pIMPBeginScreenStrings = "(max 8 personaggi)";
static const ST::string s_it_pIMPFinishButtonText   = "Analisi";
static const ST::string s_it_pIMPFinishStrings      = "Grazie, %s"; //%s is the name of the merc
static const ST::string s_it_pIMPVoicesStrings      = "Voce"; // the strings for imp voices screen

// title for program
static const ST::string s_it_pPersTitleText = "Manager del personale";

// paused game strings
static const ST::string s_it_pPausedGameText[pPausedGameText_SIZE] =
{
	"Partita in pausa",
	"Riprendi la partita (|P|a|u|s|a)",
	"Metti in pausa la partita (|P|a|u|s|a)",
};


static const ST::string s_it_pMessageStrings[pMessageStrings_SIZE] =
{
	"Vuoi uscire dalla partita?",
	"OK",
	"SÌ",
	"NO",
	"ANNULLA",
	"RIASSUMI",
	"MENTI",
	"Nessuna descrizione", //Save slots that don't have a description.
	"Partita salvata.",
	"Giorno",
	"Mercenari",
	"Slot vuoto", //An empty save game slot
	"ppm",					//Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
	"dm",					//Abbreviation for minute.
	"m",						//One character abbreviation for meter (metric distance measurement unit).
	"colpi",				//Abbreviation for rounds (# of bullets)
	"kg",					//Abbreviation for kilogram (metric weight measurement unit)
	"lb",					//Abbreviation for pounds (Imperial weight measurement unit)
	"Home Page",				//Home as in homepage on the internet.
	"USD",					//Abbreviation to US dollars
	"n/a",					//Lowercase acronym for not applicable.
	"In corso",		//Meanwhile
	"%s si trova ora nel settore %s%s", //Name/Squad has arrived in sector A9.  Order must not change without notifying
																		//SirTech
	"Versione",
	"Slot di salvataggio rapido vuoto",
	"Questo slot è riservato ai salvataggi rapidi fatti dalle schermate tattiche e dalla mappa utilizzando ALT+S.",
	"Aperto",
	"Chiuso",
	"Lo spazio su disco si sta esaurendo. Avete liberi solo %s MB e Jagged Alliance 2 ne richiede %s.",
	"%s ha preso %s.",		//'Merc name' has caught 'item' -- let SirTech know if name comes after item.
	"%s ha assunto della droga.", //'Merc name' has taken the drug
	"%s non ha alcuna abilità medica",//'Merc name' has no medical skill.

	//CDRom errors (such as ejecting CD while attempting to read the CD)
	"L'integrità del gioco è stata compromessa.",
	"ERRORE: CD-ROM non valido",

	//When firing heavier weapons in close quarters, you may not have enough room to do so.
	"Non c'è spazio per sparare da qui.",

	//Can't change stance due to objects in the way...
	"Non potete cambiare posizione questa volta.",

	//Simple text indications that appear in the game, when the merc can do one of these things.
	"Fai cadere",
	"Getta",
	"Passa",

	"%s è passato a %s.",	//"Item" passed to "merc".  Please try to keep the item %s before the merc %s,
					//otherwise, must notify SirTech.
	"Nessun spazio per passare %s a %s.", //pass "item" to "merc".  Same instructions as above.

	//A list of attachments appear after the items.  Ex:  Kevlar vest (Ceramic Plate 'Attached)'
	" Compreso)",

	//Cheat modes
	"Raggiunto il livello Cheat UNO",
	"Raggiunto il livello Cheat DUE",

	//Toggling various stealth modes
	"Squadra in modalità furtiva.",
	"Squadra non in modalità furtiva.",
	"%s in modalità furtiva.",
	"%s non in modalità furtiva.",

	//Wireframes are shown through buildings to reveal doors and windows that can't otherwise be seen in
	//an isometric engine.  You can toggle this mode freely in the game.
	"Strutture visibili",
	"Strutture nascoste",

	//These are used in the cheat modes for changing levels in the game.  Going from a basement level to
	//an upper level, etc.
	"Non potete passare al livello superiore...",
	"Non esiste nessun livello inferiore...",
	"Entra nel seminterrato %d...",
	"Abbandona il seminterrato...",

	"di",		// used in the shop keeper inteface to mark the ownership of the item eg Red's gun
	"Modalità segui disattiva.",
	"Modalità segui attiva.",
	"Cursore 3D disattivo.",
	"Cursore 3D attivo.",
	"Squadra %d attiva.",
	"Non potete permettervi di pagare a %s un salario giornaliero di %s",	//first %s is the mercs name, the seconds is a string containing the salary
	"Salta",
	"%s non può andarsene da solo.",
	"Un salvataggio è stato chiamato SaveGame99.sav. Se necessario, rinominatelo da SaveGame01 a SaveGame10 e così potrete accedervi nella schermata di caricamento.",
	"%s ha bevuto del %s",
	"Un pacco è arivato a Drassen.",
	"%s dovrebbe arrivare al punto designato di partenza (settore %s) nel giorno %d, approssimativamente alle ore %s.",		//first %s is mercs name, next is the sector location and name where they will be arriving in, lastely is the day an the time of arrival
	"Registro aggiornato.",
};


static const ST::string s_it_ItemPickupHelpPopup[ItemPickupHelpPopup_SIZE] =
{
	"OK",
	"Scorrimento su",
	"Seleziona tutto",
	"Scorrimento giù",
	"Annulla",
};

static const ST::string s_it_pDoctorWarningString[pDoctorWarningString_SIZE] =
{
	"%s non è abbstanza vicina per poter esser riparata.",
	"I vostri medici non sono riusciti a bendare completamente tutti.",
};

static const ST::string s_it_pMilitiaButtonsHelpText[pMilitiaButtonsHelpText_SIZE] =
{
	"Raccogli (Clicca di destro)/lascia (Clicca di sinistro) le truppe verdi", // button help text informing player they can pick up or drop militia with this button
	"Raccogli (Clicca di destro)/lascia (Clicca di sinistro) le truppe regolari",
	"Raccogli (Clicca di destro)/lascia (Clicca di sinistro) le truppe veterane",
	"Distribuisci equamente i soldati disponibili tra i vari settori",
};

// to inform the player to hire some mercs to get things going
static const ST::string s_it_pMapScreenJustStartedHelpText = "Andate all'A.I.M. e arruolate alcuni mercenari (*Hint* è nel Laptop)";

static const ST::string s_it_pAntiHackerString = "Errore. File mancanti o corrotti. Il gioco verrà completato ora.";


static const ST::string s_it_gzLaptopHelpText[gzLaptopHelpText_SIZE] =
{
	//Buttons:
	"Visualizza E-mail",
	"Siti web",
	"Visualizza file e gli attach delle E-mail",
	"Legge il registro degli eventi",
	"Visualizza le informazioni inerenti la squadra",
	"Visualizza la situazione finanziaria e la storia",
	"Chiude laptop",

	//Bottom task bar icons (if they exist):
	"Avete nuove E-mail",
	"Avete nuovi file",

	//Bookmarks:
	"Associazione Internazionale Mercenari",
	"Ordinativi di armi online dal sito di Bobby Ray",
	"Istituto del Profilo del Mercenario",
	"Centro più economico di reclutamento",
	"Impresa di pompe funebri McGillicutty",
	"Servizio Fioristi Riuniti",
	"Contratti assicurativi per agenti A.I.M.",
};


static const ST::string s_it_gzHelpScreenText = "Esci dalla schermata di aiuto";

static const ST::string s_it_gzNonPersistantPBIText[gzNonPersistantPBIText_SIZE] =
{
	"È in corso una battaglia. Potete solo ritirarvi dalla schermata delle tattiche.",
	"|Entra nel settore per continuare l'attuale battaglia in corso.",
	"|Automaticamente decide l'esito della battaglia in corso.",
	"Non potete decidere l'esito della battaglia in corso automaticamente, se siete voi ad attaccare.",
	"Non potete decidere l'esito della battaglia in corso automaticamente, se subite un'imboscata.",
	"Non potete decidere l'esito della battaglia in corso automaticamente, se state combattendo contro le creature nelle miniere.",
	"Non potete decidere l'esito della battaglia in corso automaticamente, se ci sono civili nemici.",
	"Non potete decidere l'esito della battaglia in corso automaticamente, se ci sono dei Bloodcat.",
	"BATTAGLIA IN CORSO",
	"Non potete ritirarvi ora.",
};

static const ST::string s_it_gzMiscString[gzMiscString_SIZE] =
{
	"I vostri soldati continuano a combattere senza l'aiuto dei vostri mercenari...",
	"Il veicolo non ha più bisogno di carburante.",
	"La tanica della benzina è piena %d%%.",
	"L'esercito di Deidrannaha riguadagnato il controllo completo su %s.",
	"Avete perso una stazione di rifornimento.",
};

static const ST::string s_it_gzIntroScreen = "Video introduttivo non trovato";

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
static const ST::string s_it_pNewNoiseStr[pNewNoiseStr_SIZE] =
{
	"%s sente un %s rumore proveniente da %s.",
	"%s sente un %s rumore di MOVIMENTO proveniente da %s.",
	"%s sente uno %s SCRICCHIOLIO proveniente da %s.",
	"%s sente un %s TONFO NELL'ACQUA proveniente da %s.",
	"%s sente un %s URTO proveniente da %s.",
	"%s sente una %s ESPLOSIONE verso %s.",
	"%s sente un %s URLO verso %s.",
	"%s sente un %s IMPATTO verso %s.",
	"%s sente un %s IMPATTO a %s.",
	"%s sente un %s SCHIANTO proveniente da %s.",
	"%s sente un %s FRASTUONO proveniente da %s.",
};

static const ST::string s_it_wMapScreenSortButtonHelpText[wMapScreenSortButtonHelpText_SIZE] =
{
	"Nome (|F|1)",
	"Assegnato (|F|2)",
	"Tipo di riposo (|F|3)",
	"Postazione (|F|4)",
	"Destinazione (|F|5)",
	"Durata dell'incarico (|F|6)",
};



static const ST::string s_it_BrokenLinkText[BrokenLinkText_SIZE] =
{
	"Errore 404",
	"Luogo non trovato.",
};


static const ST::string s_it_gzBobbyRShipmentText[gzBobbyRShipmentText_SIZE] =
{
	"Spedizioni recenti",
	"Ordine #",
	"Numero di oggetti",
	"Ordinato per",
};


static const ST::string s_it_gzCreditNames[gzCreditNames_SIZE]=
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


static const ST::string s_it_gzCreditNameTitle[gzCreditNameTitle_SIZE]=
{
	"Programmatore del gioco", 			// Chris Camfield
	"Co-designer / Autore",							// Shaun Lyng
	"Programmatore sistemi strategici & Editor",					//Kris Marnes
	"Produttore / Co-designer",						// Ian Currie
	"Co-designer / Designer della mappa",				// Linda Currie
	"Grafico",													// Eric \"WTF\" Cheng
	"Coordinatore beta, supporto",				// Lynn Holowka
	"Grafico straordinario",						// Norman \"NRG\" Olsen
	"Guru dell'audio",											// George Brooks
	"Designer delle schermate / Grafico",					// Andrew Stacey
	"Capo grafico / Animatore",						// Scot Loving
	"Capo programmatore",									// Andrew \"Big Cheese Doddle\" Emmons
	"Programmatore",											// Dave French
	"Programmatore sistemi & bilancio di gioco",					// Alex Meduna
	"Grafico dei ritratti",								// Joey \"Joeker\" Whelan",
};

static const ST::string s_it_gzCreditNameFunny[gzCreditNameFunny_SIZE]=
{
	"", 																			// Chris Camfield
	"(deve ancora esercitarsi con la punteggiatura)",					// Shaun Lyng
	"(\"Fatto. Devo solo perfezionarmi\")",	//Kris \"The Cow Rape Man\" Marnes
	"(sta diventando troppo vecchio per questo)",				// Ian Currie
	"(sta lavorando a Wizardry 8)",						// Linda Currie
	"(obbligato a occuparsi anche del CQ)",			// Eric \"WTF\" Cheng
	"(ci ha lasciato per CFSA...)",	// Lynn Holowka
	"",																			// Norman \"NRG\" Olsen
	"",																			// George Brooks
	"(Testa matta e amante del jazz)",						// Andrew Stacey
	"(il suo nome vero è Robert)",							// Scot Loving
	"(l'unica persona responsabile)",					// Andrew \"Big Cheese Doddle\" Emmons
	"(può ora tornare al motocross)",	// Dave French
	"(rubato da Wizardry 8)",							// Alex Meduna
	"",	// Joey \"Joeker\" Whelan",
};

static const ST::string s_it_sRepairsDoneString[sRepairsDoneString_SIZE] =
{
	"%s ha finito di riparare gli oggetti",
	"%s ha finito di riparare le armi e i giubbotti antiproiettile di tutti",
	"%s ha finito di riparare gli oggetti dell'equipaggiamento di tutti",
	"%s ha finito di riparare gli oggetti trasportati di tutti",
};

static const ST::string s_it_zGioDifConfirmText[zGioDifConfirmText_SIZE]=
{
	//L"You have chosen NOVICE mode. This setting is appropriate for those new to Jagged Alliance, those new to strategy games in general, or those wishing shorter battles in the game. Your choice will affect things throughout the entire course of the game, so choose wisely. Are you sure you want to play in Novice mode?",
	"Avete selezionato la modalità PRINCIPIANTE. Questo scenario è adatto a chi gioca per la prima volta a Jagged Alliance, a chi prova a giocare per la prima volta in generale o a chi desidera combattere battaglie più brevi nel gioco. La vostra decisione influirà sull'intero corso della partita; scegliete, quindi, con attenzione. Siete sicuri di voler giocare nella modalità PRINCIPIANTE?",

	//L"You have chosen EXPERIENCED mode. This setting is suitable for those already familiar with Jagged Alliance or similar games. Your choice will affect things throughout the entire course of the game, so choose wisely. Are you sure you want to play in Experienced mode?",
	"Avete selezionato la modalità ESPERTO. Questo scenario è adatto a chi ha già una certa dimestichezza con Jagged Alliance o con giochi simili. La vostra decisione influirà sull'intero corso della partita; scegliete, quindi, con attenzione. Siete sicuri di voler giocare nella modalità ESPERTO?",

	//L"You have chosen EXPERT mode. We warned you. Don't blame us if you get shipped back in a body bag. Your choice will affect things throughout the entire course of the game, so choose wisely. Are you sure you want to play in Expert mode?",
	"Avete selezionato la modalità PROFESSIONISTA. Siete avvertiti. Non malediteci, se vi ritroverete a brandelli. La vostra decisione influirà sull'intero corso della partita; scegliete, quindi, con attenzione. Siete sicuri di voler giocare nella modalità PROFESSIONISTA?",

};

static const ST::string s_it_gzLateLocalizedString[gzLateLocalizedString_SIZE] =
{
	//1-5
	"Il robot non può lasciare questo settore, se nessuno sta usando il controller.",

	//This message comes up if you have pending bombs waiting to explode in tactical.
	"Non potete comprimere il tempo ora. Aspettate le esplosioni!",

	//'Name' refuses to move.
	"%s si rifiuta di muoversi.",

	//%s a merc name
	"%s non ha abbastanza energia per cambiare posizione.",

	//A message that pops up when a vehicle runs out of gas.
	"Il %s ha esaurito la benzina e ora è rimasto a piedi a %c%d.",

	//6-10

	// the following two strings are combined with the pNewNoise[] strings above to report noises
	// heard above or below the merc
	"sopra",
	"sotto",

	//The following strings are used in autoresolve for autobandaging related feedback.
	"Nessuno dei vostri mercenari non sa praticare il pronto soccorso.",
	"Non ci sono supporti medici per bendare.",
	"Non ci sono stati supporti medici sufficienti per bendare tutti.",
	"Nessuno dei vostri mercenari ha bisogno di fasciature.",
	"Fascia i mercenari automaticamento.",
	"Tutti i vostri mercenari sono stati bendati.",

	//14
	"Arulco",

	"(tetto)",

	"Salute: %d/%d",

	//In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
	//"vs." is the abbreviation of versus.
	"%d contro %d",

	"Il %s è pieno!",  //(ex "The ice cream truck is full")

	"%s non ha bisogno immediatamente di pronto soccorso o di fasciature, quanto piuttosto di cure mediche più serie e/o riposo.",

	//20
	//Happens when you get shot in the legs, and you fall down.
	"%s è stato colpito alla gamba e collassa!",
	//Name can't speak right now.
	"%s non può parlare ora.",

	//22-24 plural versions
	"%d l'esercito verde è stato promosso a veterano.",
	"%d l'esercito verde è stato promosso a regolare.",
	"%d l'esercito regolare è stato promosso a veterano.",

	//25
	"Interruttore",

	//26
	//Name has gone psycho -- when the game forces the player into burstmode (certain unstable characters)
	"%s è impazzito!",

	//27-28
	//Messages why a player can't time compress.
	"Non è al momento sicuro comprimere il tempo visto che avete dei mercenari nel settore %s.",
	"Non è al momento sicuro comprimere il tempo quando i mercenari sono nelle miniere infestate dalle creature.",

	//29-31 singular versions
	"1 esercito verde è stato promosso a veterano.",
	"1 esercito verde è stato promosso a regolare.",
	"1 eserciro regolare è stato promosso a veterano.",

	//32-34
	"%s non dice nulla.",
	"Andate in superficie?",
	"(Squadra %d)",

	//35
	//Ex: "Red has repaired Scope's MP5K".  Careful to maintain the proper order (Red before Scope, Scope before MP5K)
	"%s ha riparato %s's %s",

	//36
	"BLOODCAT",

	//37-38 "Name trips and falls"
	"%s trips and falls",
	"Questo oggetto non può essere raccolto qui.",

	//39
	"Nessuno dei vostri rimanenti mercenari è in grado di combattere. L'esercito combatterà contro le creature da solo.",

	//40-43
	//%s is the name of merc.
	"%s è rimasto sprovvisto di kit medici!",
	"%s non è in grado di curare nessuno!",
	"%s è rimasto sprovvisto di forniture mediche!",
	"%s non è in grado di riparare niente!",

	//44-45
	"Tempo di riparazione",
	"%s non può vedere questa persona.",

	//46-48
	"L'estensore della canna dell'arma di %s si è rotto!",
	"Non più di %d allenatori di soldati sono ammessi in questo settore.",
	"Siete sicuri?",

	//49-50
	"Compressione del tempo",
	"La tanica della benzina del veicolo è ora piena.",

	//51-52 Fast help text in mapscreen.
	"Continua la compressione del tempo (|S|p|a|z|i|o)",
	"Ferma la compressione del tempo (|E|s|c)",

	//53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
	"%s ha sbloccata il %s",
	"%s ha sbloccato il %s di %s",

	//55
	"Non potete comprimere il tempo mentre visualizzate l'inventario del settore.",

	//56
	//Displayed with the version information when cheats are enabled.
	"Attuale/Massimo Progresso: %d%%/%d%%",

	//57
	"Accompagnate John e Mary?",

	"Interruttore attivato.",
};

static const ST::string s_it_str_ceramic_plates_smashed = "%s's ceramic plates have been smashed!"; // TODO translate

static const ST::string s_it_str_arrival_rerouted = "Arrival of new recruits is being rerouted to sector %s, as scheduled drop-off point of sector %s is enemy occupied."; // TODO translate


static const ST::string s_it_str_stat_health       = "Salute";
static const ST::string s_it_str_stat_agility      = "Agilità";
static const ST::string s_it_str_stat_dexterity    = "Destrezza";
static const ST::string s_it_str_stat_strength     = "Forza";
static const ST::string s_it_str_stat_leadership   = "Comando";
static const ST::string s_it_str_stat_wisdom       = "Saggezza";
static const ST::string s_it_str_stat_exp_level    = "Esperienza"; // Livello esperienza
static const ST::string s_it_str_stat_marksmanship = "Mira";
static const ST::string s_it_str_stat_mechanical   = "Meccanica";
static const ST::string s_it_str_stat_explosive    = "Esplosivi";
static const ST::string s_it_str_stat_medical      = "Pronto socc.";

static const ST::string s_it_str_stat_list[str_stat_list_SIZE] =
{
	s_it_str_stat_health,
	s_it_str_stat_agility,
	s_it_str_stat_dexterity,
	s_it_str_stat_strength,
	s_it_str_stat_leadership,
	s_it_str_stat_wisdom,
	s_it_str_stat_exp_level,
	s_it_str_stat_marksmanship,
	s_it_str_stat_mechanical,
	s_it_str_stat_explosive,
	s_it_str_stat_medical
};

static const ST::string s_it_str_aim_sort_list[str_aim_sort_list_SIZE] =
{
	s_it_str_aim_sort_price,
	s_it_str_aim_sort_experience,
	s_it_str_aim_sort_marksmanship,
	s_it_str_aim_sort_medical,
	s_it_str_aim_sort_explosives,
	s_it_str_aim_sort_mechanical,
	s_it_str_aim_sort_ascending,
	s_it_str_aim_sort_descending,
};

static const ST::string s_it_gs_dead_is_dead_mode_tab_name[gs_dead_is_dead_mode_tab_name_SIZE] =
{
	"Normal", 			// Normal Tab
	"DiD", 			// Dead is Dead Tab
};

// Italian language resources.
LanguageRes g_LanguageResItalian = {

	s_it_WeaponType,

	s_it_Message,
	s_it_TeamTurnString,
	s_it_pAssignMenuStrings,
	s_it_pTrainingStrings,
	s_it_pTrainingMenuStrings,
	s_it_pAttributeMenuStrings,
	s_it_pVehicleStrings,
	s_it_pShortAttributeStrings,
	s_it_pContractStrings,
	s_it_pAssignmentStrings,
	s_it_pConditionStrings,
	s_it_pPersonnelScreenStrings,
	s_it_pUpperLeftMapScreenStrings,
	s_it_pTacticalPopupButtonStrings,
	s_it_pSquadMenuStrings,
	s_it_pDoorTrapStrings,
	s_it_pLongAssignmentStrings,
	s_it_pMapScreenMouseRegionHelpText,
	s_it_pNoiseVolStr,
	s_it_pNoiseTypeStr,
	s_it_pDirectionStr,
	s_it_pRemoveMercStrings,
	s_it_sTimeStrings,
	s_it_pInvPanelTitleStrings,
	s_it_pPOWStrings,
	s_it_pMilitiaString,
	s_it_pMilitiaButtonString,
	s_it_pEpcMenuStrings,
	s_it_pRepairStrings,
	s_it_sPreStatBuildString,
	s_it_sStatGainStrings,
	s_it_pHelicopterEtaStrings,
	s_it_sMapLevelString,
	s_it_gsLoyalString,
	s_it_gsUndergroundString,
	s_it_gsTimeStrings,
	s_it_sFacilitiesStrings,
	s_it_pMapPopUpInventoryText,
	s_it_pwTownInfoStrings,
	s_it_pwMineStrings,
	s_it_pwMiscSectorStrings,
	s_it_pMapInventoryErrorString,
	s_it_pMapInventoryStrings,
	s_it_pMovementMenuStrings,
	s_it_pUpdateMercStrings,
	s_it_pMapScreenBorderButtonHelpText,
	s_it_pMapScreenBottomFastHelp,
	s_it_pMapScreenBottomText,
	s_it_pMercDeadString,
	s_it_pSenderNameList,
	s_it_pNewMailStrings,
	s_it_pDeleteMailStrings,
	s_it_pEmailHeaders,
	s_it_pEmailTitleText,
	s_it_pFinanceTitle,
	s_it_pFinanceSummary,
	s_it_pFinanceHeaders,
	s_it_pTransactionText,
	s_it_pMoralStrings,
	s_it_pSkyriderText,
	s_it_str_left_equipment,
	s_it_pMapScreenStatusStrings,
	s_it_pMapScreenPrevNextCharButtonHelpText,
	s_it_pEtaString,
	s_it_pShortVehicleStrings,
	s_it_pTrashItemText,
	s_it_pMapErrorString,
	s_it_pMapPlotStrings,
	s_it_pBullseyeStrings,
	s_it_pMiscMapScreenMouseRegionHelpText,
	s_it_str_he_leaves_where_drop_equipment,
	s_it_str_she_leaves_where_drop_equipment,
	s_it_str_he_leaves_drops_equipment,
	s_it_str_she_leaves_drops_equipment,
	s_it_pImpPopUpStrings,
	s_it_pImpButtonText,
	s_it_pExtraIMPStrings,
	s_it_pFilesTitle,
	s_it_pFilesSenderList,
	s_it_pHistoryLocations,
	s_it_pHistoryStrings,
	s_it_pHistoryHeaders,
	s_it_pHistoryTitle,
	s_it_pShowBookmarkString,
	s_it_pWebPagesTitles,
	s_it_pWebTitle,
	s_it_pPersonnelString,
	s_it_pErrorStrings,
	s_it_pDownloadString,
	s_it_pBookMarkStrings,
	s_it_pLaptopIcons,
	s_it_gsAtmStartButtonText,
	s_it_pPersonnelTeamStatsStrings,
	s_it_pPersonnelCurrentTeamStatsStrings,
	s_it_pPersonelTeamStrings,
	s_it_pPersonnelDepartedStateStrings,
	s_it_pMapHortIndex,
	s_it_pMapVertIndex,
	s_it_pMapDepthIndex,
	s_it_pLaptopTitles,
	s_it_pDayStrings,
	s_it_pMilitiaConfirmStrings,
	s_it_pSkillAtZeroWarning,
	s_it_pIMPBeginScreenStrings,
	s_it_pIMPFinishButtonText,
	s_it_pIMPFinishStrings,
	s_it_pIMPVoicesStrings,
	s_it_pPersTitleText,
	s_it_pPausedGameText,
	s_it_zOptionsToggleText,
	s_it_zOptionsScreenHelpText,
	s_it_pDoctorWarningString,
	s_it_pMilitiaButtonsHelpText,
	s_it_pMapScreenJustStartedHelpText,
	s_it_pLandMarkInSectorString,
	s_it_gzMercSkillText,
	s_it_gzNonPersistantPBIText,
	s_it_gzMiscString,
	s_it_wMapScreenSortButtonHelpText,
	s_it_pNewNoiseStr,
	s_it_gzLateLocalizedString,
	s_it_pAntiHackerString,
	s_it_pMessageStrings,
	s_it_ItemPickupHelpPopup,
	s_it_TacticalStr,
	s_it_LargeTacticalStr,
	s_it_zDialogActions,
	s_it_zDealerStrings,
	s_it_zTalkMenuStrings,
	s_it_gzMoneyAmounts,
	s_it_gzProsLabel,
	s_it_gzConsLabel,
	s_it_gMoneyStatsDesc,
	s_it_gWeaponStatsDesc,
	s_it_sKeyDescriptionStrings,
	s_it_zHealthStr,
	s_it_zVehicleName,
	s_it_pExitingSectorHelpText,
	s_it_InsContractText,
	s_it_InsInfoText,
	s_it_MercAccountText,
	s_it_MercInfo,
	s_it_MercNoAccountText,
	s_it_MercHomePageText,
	s_it_sFuneralString,
	s_it_sFloristText,
	s_it_sOrderFormText,
	s_it_sFloristGalleryText,
	s_it_sFloristCards,
	s_it_BobbyROrderFormText,
	s_it_BobbyRText,
	s_it_str_bobbyr_guns_num_guns_that_use_ammo,
	s_it_BobbyRaysFrontText,
	s_it_AimSortText,
	s_it_str_aim_sort_price,
	s_it_str_aim_sort_experience,
	s_it_str_aim_sort_marksmanship,
	s_it_str_aim_sort_medical,
	s_it_str_aim_sort_explosives,
	s_it_str_aim_sort_mechanical,
	s_it_str_aim_sort_ascending,
	s_it_str_aim_sort_descending,
	s_it_AimPolicyText,
	s_it_AimMemberText,
	s_it_CharacterInfo,
	s_it_VideoConfercingText,
	s_it_AimPopUpText,
	s_it_AimLinkText,
	s_it_AimHistoryText,
	s_it_AimFiText,
	s_it_AimAlumniText,
	s_it_AimScreenText,
	s_it_AimBottomMenuText,
	s_it_zMarksMapScreenText,
	s_it_gpStrategicString,
	s_it_gpGameClockString,
	s_it_SKI_Text,
	s_it_SkiMessageBoxText,
	s_it_zSaveLoadText,
	s_it_zOptionsText,
	s_it_gzGIOScreenText,
	s_it_gzHelpScreenText,
	s_it_gzLaptopHelpText,
	s_it_gzMoneyWithdrawMessageText,
	s_it_gzCopyrightText,
	s_it_BrokenLinkText,
	s_it_gzBobbyRShipmentText,
	s_it_zGioDifConfirmText,
	s_it_gzCreditNames,
	s_it_gzCreditNameTitle,
	s_it_gzCreditNameFunny,
	s_it_pContractButtonString,
	s_it_gzIntroScreen,
	s_it_pUpdatePanelButtons,
	s_it_sRepairsDoneString,
	s_it_str_ceramic_plates_smashed,
	s_it_str_arrival_rerouted,

	s_it_str_stat_health,
	s_it_str_stat_agility,
	s_it_str_stat_dexterity,
	s_it_str_stat_strength,
	s_it_str_stat_leadership,
	s_it_str_stat_wisdom,
	s_it_str_stat_exp_level,
	s_it_str_stat_marksmanship,
	s_it_str_stat_mechanical,
	s_it_str_stat_explosive,
	s_it_str_stat_medical,

	s_it_str_stat_list,
	s_it_str_aim_sort_list,

	g_eng_zNewTacticalMessages,
	g_eng_str_iron_man_mode_warning,
	g_eng_str_dead_is_dead_mode_warning,
	g_eng_str_dead_is_dead_mode_enter_name,

	s_it_gs_dead_is_dead_mode_tab_name,

	s_it_gzIMPSkillTraitsText,
};

#ifdef WITH_UNITTESTS
TEST(StringEncodingTest, ItalianTextFile)
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
