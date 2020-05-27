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
//   must fit on a single line non matter how long the string is.  All strings start with " and end with ",
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



static ST::string s_fr_WeaponType[WeaponType_SIZE] =
{
	"Divers",
	"Pistolet",
	"Pistolet-mitrailleur",
	"Mitraillette",
	"Fusil",
	"Fusil de précision",
	"Fusil d'assaut",
	"Mitrailleuse légère",
	"Fusil à pompe",
};

static ST::string s_fr_TeamTurnString[TeamTurnString_SIZE] =
{
	"Tour du joueur", // player's turn
	"Tour de l'adversaire",
	"Tour des créatures",
	"Tour de la milice",
	"Tour des civils",
	// planning turn
};

static ST::string s_fr_Message[Message_SIZE] =
{
	// In the following 8 strings, the %s is the merc's name, and the %d (if any) is a number.

	"%s est touché à la tête et perd un point de sagesse !",
	"%s est touché à l'épaule et perd un point de dextérité !",
	"%s est touché à la poitrine et perd un point de force !",
	"%s est touché à la jambe et perd un point d'agilité !",
	"%s est touché à la tête et perd %d points de sagesse !",
	"%s est touché à l'épaule et perd %d points de dextérité !",
	"%s est touché à la poitrine et perd %d points de force !",
	"%s est touché à la jambe et perd %d points d'agilité !",
	"Interruption !",

	"Les renforts sont arrivés !",

	// In the following four lines, all %s's are merc names

	"%s recharge.",
	"%s n'a pas assez de Points d'Action !",
	// the following 17 strings are used to create lists of gun advantages and disadvantages
	// (separated by commas)
	"fiable",
	"peu fiable",
	"facile à entretenir",
	"difficile à entretenir",
	"puissant",
	"peu puissant",
	"cadence de tir élevée",
	"faible cadence de tir",
	"longue portée",
	"courte portée",
	"léger",
	"encombrant",
	"petit",
	"tir en rafale",
	"pas de tir en rafale",
	"grand chargeur",
	"petit chargeur",

	// In the following two lines, all %s's are merc names

	"Le camouflage de %s s'est effacé.",
	"Le camouflage de %s est parti.",

	// The first %s is a merc name and the second %s is an item name

	"La deuxième arme est vide !",
	"%s a volé le/la %s.",

	// The %s is a merc name

	"L'arme de %s ne peut pas tirer en rafale.",

	"Vous avez déjà ajouté cet accessoire.",
	"Combiner les objets ?",

	// Both %s's are item names

	"Vous ne pouvez combiner un(e) %s avec un(e) %s.",

	"Aucun",
	"Ejecter chargeur",
	"Accessoire",

	//You cannot use "item(s)" and your "other item" at the same time.
	//Ex:  You cannot use sun goggles and you gas mask at the same time.
	"Vous ne pouvez utiliser votre %s et votre %s simultanément.",

	"Vous pouvez combiner cet accessoire avec certains objets en le mettant dans l'un des quatre emplacements disponibles.",
	"Vous pouvez combiner cet accessoire avec certains objets en le mettant dans l'un des quatre emplacements disponibles (Ici, cet accessoire n'est pas compatible avec cet objet).",
	"Ce secteur n'a pas été sécurisé !",
	"Vous devez donner %s à %s",//inverted !! you still need to give the letter to X
	"%s a été touché à la tête !",
	"Rompre le combat ?",
	"Cet accessoire ne pourra plus être enlevé. Désirez-vous toujours le mettre ?",
	"%s se sent beaucoup mieux !",
	"%s a glissé sur des billes !",
	"%s n'est pas parvenu à ramasser le/la %s !",
	"%s a réparé le %s",
	"Interruption pour ",
	"Voulez-vous vous rendre ?",
	"Cette personne refuse votre aide.",
	"JE NE CROIS PAS !",
	"Pour utiliser l'hélicoptère de Skyrider, vous devez ASSIGNER vos mercenaires au VEHICULE.",
	"%s ne peut recharger qu'UNE arme",
	"Tour des chats sauvages",
};


// the types of time compression. For example: is the timer paused? at normal speed, 5 minutes per second, etc.
// min is an abbreviation for minutes

static const ST::string s_fr_sTimeStrings[sTimeStrings_SIZE] =
{
	"Pause",
	"Normal",
	"5 min",
	"30 min",
	"60 min",
	"6 hrs",
};


// Assignment Strings: what assignment does the merc  have right now? For example, are they on a squad, training,
// administering medical aid (doctor) or training a town. All are abbreviated. 8 letters is the longest it can be.

static const ST::string s_fr_pAssignmentStrings[pAssignmentStrings_SIZE] =
{
	"Esc. 1",
	"Esc. 2",
	"Esc. 3",
	"Esc. 4",
	"Esc. 5",
	"Esc. 6",
	"Esc. 7",
	"Esc. 8",
	"Esc. 9",
	"Esc. 10",
	"Esc. 11",
	"Esc. 12",
	"Esc. 13",
	"Esc. 14",
	"Esc. 15",
	"Esc. 16",
	"Esc. 17",
	"Esc. 18",
	"Esc. 19",
	"Esc. 20",
	"Service", // on active duty
	"Docteur", // administering medical aid
	"Patient", // getting medical aid
	"Transport", // in a vehicle
	"Transit", // in transit - abbreviated form
	"Réparation", // repairing
	"Formation", // training themselves
	"Milice", // training a town to revolt
	"Entraîneur", // training a teammate
	"Elève", // being trained by someone else
	"Mort", // dead
	"Incap.", // abbreviation for incapacitated
	"Capturé", // Prisoner of war - captured
	"Hôpital", // patient in a hospital
	"Vide",	// Vehicle is empty
};


static const ST::string s_fr_pMilitiaString[pMilitiaString_SIZE] =
{
	"Milice", // the title of the militia box
	"Disponibles", //the number of unassigned militia troops
	"Vous ne pouvez réorganiser la milice lors d'un combat !",
};


static const ST::string s_fr_pMilitiaButtonString[pMilitiaButtonString_SIZE] =
{
	"Auto", // auto place the militia troops for the player
	"OK", // done placing militia troops
};

static const ST::string s_fr_pConditionStrings[pConditionStrings_SIZE] =
{
	"Excellent", //the state of a soldier .. excellent health
	"Bon", // good health
	"Satisfaisant", // fair health
	"Blessé", // wounded health
	"Fatigué", // tired
	"Epuisé", // bleeding to death
	"Inconscient", // knocked out
	"Mourant", // near death
	"Mort", // dead
};

static const ST::string s_fr_pEpcMenuStrings[pEpcMenuStrings_SIZE] =
{
	"Service", // set merc on active duty
	"Patient", // set as a patient to receive medical aid
	"Transport", // tell merc to enter vehicle
	"Laisser", // let the escorted character go off on their own
	"Annuler", // close this menu
};


// look at pAssignmentString above for comments

static const ST::string s_fr_pLongAssignmentStrings[pLongAssignmentStrings_SIZE] =
{
	"Escouade 1",
	"Escouade 2",
	"Escouade 3",
	"Escouade 4",
	"Escouade 5",
	"Escouade 6",
	"Escouade 7",
	"Escouade 8",
	"Escouade 9",
	"Escouade 10",
	"Escouade 11",
	"Escouade 12",
	"Escouade 13",
	"Escouade 14",
	"Escouade 15",
	"Escouade 16",
	"Escouade 17",
	"Escouade 18",
	"Escouade 19",
	"Escouade 20",
	"Service",
	"Docteur",
	"Patient",
	"Transport",
	"Transit",
	"Réparation",
	"Formation",
	"Milice",
	"Entraîneur",
	"Elève",
	"Mort",
	"Incap.",
	"Capturé",
	"Hôpital", // patient in a hospital
	"Vide",	// Vehicle is empty
};


// the contract options

static const ST::string s_fr_pContractStrings[pContractStrings_SIZE] =
{
	"Options du contrat :",
	"", // a blank line, required
	"Extension 1 jour", // offer merc a one day contract extension
	"Extension 1 semaine", // 1 week
	"Extension 2 semaines", // 2 week
	"Renvoyer", // end merc's contract
	"Annuler", // stop showing this menu
};

static const ST::string s_fr_pPOWStrings[pPOWStrings_SIZE] =
{
	"Capturé",  //an acronym for Prisoner of War
	"??",
};

static const ST::string s_fr_pInvPanelTitleStrings[pInvPanelTitleStrings_SIZE] =
{
	"Armure", // the armor rating of the merc
	"Poids", // the weight the merc is carrying
	"Cam.", // the merc's camouflage rating
};

static const ST::string s_fr_pShortAttributeStrings[pShortAttributeStrings_SIZE] =
{
	"Agi", // the abbreviated version of : agilité
	"Dex", // dextérité
	"For", // strength
	"Com", // leadership
	"Sag", // sagesse
	"Niv", // experience level
	"Tir", // marksmanship skill
	"Exp", // explosive skill
	"Tec", // mechanical skill
	"Méd", // medical skill
};


static const ST::string s_fr_pUpperLeftMapScreenStrings[pUpperLeftMapScreenStrings_SIZE] =
{
	"Affectation", // the mercs current assignment
	"Santé", // the health level of the current merc
	"Moral", // the morale of the current merc
	"Cond.",	// the condition of the current vehicle
};

static const ST::string s_fr_pTrainingStrings[pTrainingStrings_SIZE] =
{
	"Formation", // tell merc to train self
	"Milice", // tell merc to train town
	"Entraîneur", // tell merc to act as trainer
	"Elève", // tell merc to be train by other
};

static const ST::string s_fr_pAssignMenuStrings[pAssignMenuStrings_SIZE] =
{
	"Service", // merc is on active duty
	"Docteur", // the merc is acting as a doctor
	"Patient", // the merc is receiving medical attention
	"Transport", // the merc is in a vehicle
	"Réparation", // the merc is repairing items
	"Formation", // the merc is training
	"Annuler", // cancel this menu
};

static const ST::string s_fr_pRemoveMercStrings[pRemoveMercStrings_SIZE] =
{
	"Enlever Merc", // remove dead merc from current team
	"Annuler",
};

static const ST::string s_fr_pAttributeMenuStrings[pAttributeMenuStrings_SIZE] =
{
	"Force",
	"Dextérité",
	"Agilité",
	"Santé",
	"Tir",
	"Médecine",
	"Technique",
	"Commandement",
	"Explosifs",
	"Annuler",
};

static const ST::string s_fr_pTrainingMenuStrings[pTrainingMenuStrings_SIZE] =
{
	"Formation", // train yourself
	"Milice", // train the town
	"Entraîneur", // train your teammates
	"Elève",  // be trained by an instructor
	"Annuler", // cancel this menu
};


static const ST::string s_fr_pSquadMenuStrings[pSquadMenuStrings_SIZE] =
{
	"Esc. 1",
	"Esc. 2",
	"Esc. 3",
	"Esc. 4",
	"Esc. 5",
	"Esc. 6",
	"Esc. 7",
	"Esc. 8",
	"Esc. 9",
	"Esc. 10",
	"Esc. 11",
	"Esc. 12",
	"Esc. 13",
	"Esc. 14",
	"Esc. 15",
	"Esc. 16",
	"Esc. 17",
	"Esc. 18",
	"Esc. 19",
	"Esc. 20",
	"Annuler",
};


static const ST::string s_fr_pPersonnelScreenStrings[pPersonnelScreenStrings_SIZE] =
{
	"Acompte méd. :", // amount of medical deposit put down on the merc
	"Contrat :", // cost of current contract
	"Tués", // number of kills by merc
	"Participation", // number of assists on kills by merc
	"Coût/jour :", // daily cost of merc
	"Coût total :", // total cost of merc
	"Contrat :", // cost of current contract
	"Services rendus :", // total service rendered by merc
	"Salaires dus :", // amount left on MERC merc to be paid
	"Précision :", // percentage of shots that hit target
	"Combats", // number of battles fought
	"Blessures", // number of times merc has been wounded
	"Spécialités :",
	"Aucune spécialité",
};


//These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
static const ST::string s_fr_gzMercSkillText[gzMercSkillText_SIZE] =
{
	"Aucune spécialité",
	"Crochetage",
	"Combat à mains nues",
	"Electronique",
	"Opérations de nuit",
	"Lancer",
	"Enseigner",
	"Armes lourdes",
	"Armes automatiques",
	"Furtivité",
	"Ambidextre",
	"Voleur",
	"Arts martiaux",
	"Couteau",
	"Bonus toucher (sur le toit)",
	"Camouflage",
	"(Expert)",
};


// This is pop up help text for the options that are available to the merc

static const ST::string s_fr_pTacticalPopupButtonStrings[pTacticalPopupButtonStrings_SIZE] =
{
	"Debout/Marcher (|S)",
	"Accroupi/Avancer (|C)",
	"Debout/|Courir",
	"A terre/Ramper (|P)",
	"Regarder (|L)",
	"Action",
	"Parler",
	"Examiner (|C|t|r|l)",

	// Pop up door menu
	"Ouvrir à la main",
	"Examen poussé",
	"Crocheter",
	"Enfoncer",
	"Désamorcer",
	"Verrouiller",
	"Déverrouiller",
	"Utiliser explosif",
	"Utiliser pied de biche",
	"Annuler (|E|c|h|a|p)",
	"Fermer",
};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.

static const ST::string s_fr_pDoorTrapStrings[pDoorTrapStrings_SIZE] =
{
	"aucun piège",
	"un piège explosif",
	"un piège électrique",
	"une alarme sonore",
	"une alarme silencieuse",
};

// On the map screen, there are four columns. This text is popup help text that identifies the individual columns.

static const ST::string s_fr_pMapScreenMouseRegionHelpText[pMapScreenMouseRegionHelpText_SIZE] =
{
	"Choix personnage",
	"Affectation",
	"Destination",
	"Merc |Contrat",
	"Enlever Merc",
	"Repos",
};

// volumes of noises

static const ST::string s_fr_pNoiseVolStr[pNoiseVolStr_SIZE] =
{
	"FAIBLE",
	"MOYEN",
	"FORT",
	"TRES FORT",
};

// types of noises

static const ST::string s_fr_pNoiseTypeStr[pNoiseTypeStr_SIZE] = // OBSOLETE
{
	"INCONNU",
	"MOUVEMENT",
	"GRINCEMENT",
	"CLAPOTEMENT",
	"IMPACT",
	"COUP DE FEU",
	"EXPLOSION",
	"CRI",
	"IMPACT",
	"IMPACT",
	"BRUIT",
	"COLLISION",
};

// Directions that are used to report noises

static const ST::string s_fr_pDirectionStr[pDirectionStr_SIZE] =
{
	"au NORD-EST",
	"à l'EST",
	"au SUD-EST",
	"au SUD",
	"au SUD-OUEST",
	"à l'OUEST",
	"au NORD-OUEST",
	"au NORD",
};

// These are the different terrain types.

static const ST::string s_fr_pLandTypeStrings[pLandTypeStrings_SIZE] =
{
	"Ville",
	"Route",
	"Plaine",
	"Désert",
	"Bois",
	"Forêt",
	"Marais",
	"Eau",
	"Collines",
	"Infranchissable",
	"Rivière",	//river from north to south
	"Rivière",	//river from east to west
	"Pays étranger",
	//NONE of the following are used for directional travel, just for the sector description.
	"Tropical",
	"Cultures",
	"Plaines, route",
	"Bois, route",
	"Ferme, route",
	"Tropical, route",
	"Forêt, route",
	"Route côtière",
	"Montagne, route",
	"Côte, route",
	"Désert, route",
	"Marais, route",
	"Bois, site SAM",
	"Désert, site SAM",
	"Tropical, site SAM",
	"Meduna, site SAM",

	//These are descriptions for special sectors
	"Hôpital de Cambria",
	"Aéroport de Drassen",
	"Aéroport de Meduna",
	"Site SAM",
	"Base rebelle", //The rebel base underground in sector A10
	"Prison de Tixa",	//The basement of the Tixa Prison (J9)
	"Repaire de créatures",	//Any mine sector with creatures in it
	"Sous-sols d'Orta",	//The basement of Orta (K4)
	"Tunnel",				//The tunnel access from the maze garden in Meduna
										//leading to the secret shelter underneath the palace
	"Abri",				//The shelter underneath the queen's palace
	"",							//Unused
};

static const ST::string s_fr_gpStrategicString[gpStrategicString_SIZE] =
{
	"%s détecté dans le secteur %c%d et une autre escouade est en route.",	//STR_DETECTED_SINGULAR
	"%s détecté dans le secteur %c%d et d'autres escouades sont en route.",	//STR_DETECTED_PLURAL
	"Voulez-vous coordonner vos mouvements de troupe ?",													//STR_COORDINATE

	//Dialog strings for enemies.

	"L'ennemi vous propose de vous rendre.",			//STR_ENEMY_SURRENDER_OFFER
	"L'ennemi a capturé vos mercenaires inconscients.",	//STR_ENEMY_CAPTURED

	//The text that goes on the autoresolve buttons

	"Retraite", 		//The retreat button				//STR_AR_RETREAT_BUTTON
	"OK",		//The done button				//STR_AR_DONE_BUTTON

	//The headers are for the autoresolve type (MUST BE UPPERCASE)

	"DEFENSEURS",								//STR_AR_DEFEND_HEADER
	"ATTAQUANTS",								//STR_AR_ATTACK_HEADER
	"RENCONTRE",								//STR_AR_ENCOUNTER_HEADER
	"Secteur",		//The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

	//The battle ending conditions

	"VICTOIRE !",								//STR_AR_OVER_VICTORY
	"DEFAITE !",								//STR_AR_OVER_DEFEAT
	"REDDITION !",							//STR_AR_OVER_SURRENDERED
	"CAPTURE !",								//STR_AR_OVER_CAPTURED
	"RETRAITE !",								//STR_AR_OVER_RETREATED

	//These are the labels for the different types of enemies we fight in autoresolve.

	"Milicien",							//STR_AR_MILITIA_NAME,
	"Élite",								//STR_AR_ELITE_NAME,
	"Soldat",								//STR_AR_TROOP_NAME,
	"Admin",								//STR_AR_ADMINISTRATOR_NAME,
	"Créature",								//STR_AR_CREATURE_NAME,

	//Label for the length of time the battle took

	"Temps écoulé",							//STR_AR_TIME_ELAPSED,

	//Labels for status of merc if retreating.  (UPPERCASE)

	"SE RETIRE",								//STR_AR_MERC_RETREATED,
	"EN RETRAITE",								//STR_AR_MERC_RETREATING,
	"RETRAITE",								//STR_AR_MERC_RETREAT,

	//PRE BATTLE INTERFACE STRINGS
	//Goes on the three buttons in the prebattle interface.  The Auto resolve button represents
	//a system that automatically resolves the combat for the player without having to do anything.
	//These strings must be short (two lines -- 6-8 chars per line)

	"Auto.",							//STR_PB_AUTORESOLVE_BTN,
	"Combat",							//STR_PB_GOTOSECTOR_BTN,
	"Retraite",							//STR_PB_RETREATMERCS_BTN,

	//The different headers(titles) for the prebattle interface.
	"ENNEMI REPERE",							//STR_PB_ENEMYENCOUNTER_HEADER,
	"ATTAQUE ENNEMIE",							//STR_PB_ENEMYINVASION_HEADER, // 30
	"EMBUSCADE !",								//STR_PB_ENEMYAMBUSH_HEADER
	"VOUS PENETREZ EN SECTEUR ENNEMI",				//STR_PB_ENTERINGENEMYSECTOR_HEADER
	"ATTAQUE DE CREATURES",							//STR_PB_CREATUREATTACK_HEADER
	"ATTAQUE DE CHATS SAUVAGES",							//STR_PB_BLOODCATAMBUSH_HEADER
	"VOUS ENTREZ DANS LE REPAIRE DES CHATS SAUVAGES",			//STR_PB_ENTERINGBLOODCATLAIR_HEADER

	//Various single words for direct translation.  The Civilians represent the civilian
	//militia occupying the sector being attacked.  Limited to 9-10 chars

	"Lieu",
	"Ennemis",
	"Mercs",
	"Milice",
	"Créatures",
	"Chats",
	"Secteur",
	"Aucun",		//If there are non uninvolved mercs in this fight.
	"N/A",			//Acronym of Not Applicable
	"j",			//One letter abbreviation of day
	"h",			//One letter abbreviation of hour

	//TACTICAL PLACEMENT USER INTERFACE STRINGS
	//The four buttons

	"Annuler",
	"Dispersé",
	"Groupé",
	"OK",

	//The help text for the four buttons.  Use \n to denote new line (just like enter).

	"Annule le déploiement des mercenaires\net vous permet de les déployer vous-même. (|C)",
	"Disperse aléatoirement vos mercenaires\nà chaque fois. (|s)",
	"Vous permet de placer votre |groupe de mercenaires.",
	"Cliquez sur ce bouton lorsque vous avez déployé\nvos mercenaires. (|E|n|t|r|é|e)",
	"Vous devez déployer vos mercenaires\navant d'engager le combat.",

	//Various strings (translate word for word)

	"Secteur",
	"Définissez les points d'entrée",

	//Strings used for various popup message boxes.  Can be as long as desired.

	"Il semblerait que l'endroit soit inaccessible...",
	"Déployez vos mercenaires dans la zone en surbrillance.",

	//These entries are for button popup help text for the prebattle interface.  All popup help
	//text supports the use of \n to denote new line.  Do not use spaces before or after the \n.
	"Résolution |automatique du combat\nsans charger la carte.",
	"Résolution automatique impossible lorsque\nvous attaquez.",
	"Pénétrez dans le secteur pour engager le combat. (|E)",
	"Faire |retraite vers le secteur précédent.",				//singular version
	"Faire |retraite vers les secteurs précédents.", //multiple groups with same previous sector

	//various popup messages for battle conditions.

	//%c%d is the sector -- ex:  A9
	"L'ennemi attaque votre milice dans le secteur %c%d.",
	//%c%d is the sector -- ex:  A9
	"Les créatures attaquent votre milice dans le secteur %c%d.",
	//1st %d refers to the number of civilians eaten by monsters,  %c%d is the sector -- ex:  A9
	//Note:  the minimum number of civilians eaten will be two.
	"Les créatures ont tué %d civils dans le secteur %s.",
	//%s is the sector location -- ex:  A9: Omerta
	"L'ennemi attaque vos mercenaires dans le secteur %s. Aucun de vos hommes ne peut combattre !",
	//%s is the sector location -- ex:  A9: Omerta
	"Les créatures attaquent vos mercenaires dans le secteur %s. Aucun de vos hommes ne peut combattre !",

};

//This is the day represented in the game clock.  Must be very short, 4 characters max.
static const ST::string s_fr_gpGameClockString = "Jour";

//When the merc finds a key, they can get a description of it which
//tells them where and when they found it.
static const ST::string s_fr_sKeyDescriptionStrings[sKeyDescriptionStrings_SIZE] =
{
	"Secteur :",
	"Jour :",
};

//The headers used to describe various weapon statistics.

static ST::string s_fr_gWeaponStatsDesc[ gWeaponStatsDesc_SIZE] =
{
	"Poids (%s):",
	"Etat :",
	"Munitions :", 		// Number of bullets left in a magazine
	"Por. :",		// Range
	"Dég. :",		// Damage
	"PA :",			// abbreviation for Action Points
	"="
};

//The headers used for the merc's money.

static const ST::string s_fr_gMoneyStatsDesc[gMoneyStatsDesc_SIZE] =
{
	"Montant",
	"Restant :", //this is the overall balance
	"Montant",
	"Partager :", // the amount he wants to separate from the overall balance to get two piles of money

	"Actuel",
	"Solde",
	"Montant à",
	"Retirer",
};

//The health of various creatures, enemies, characters in the game. The numbers following each are for comment
//only, but represent the precentage of points remaining.

static const ST::string s_fr_zHealthStr[zHealthStr_SIZE] =
{
	"MOURANT",		//	>= 0
	"CRITIQUE", 		//	>= 15
	"FAIBLE",		//	>= 30
	"BLESSE",    	//	>= 45
	"SATISFAISANT",    	//	>= 60
	"BON",     	// 	>= 75
	"EXCELLENT",		// 	>= 90
};

static const ST::string s_fr_gzMoneyAmounts[gzMoneyAmounts_SIZE] =
{
	"1000",
	"100",
	"10",
	"OK",
	"Partager",
	"Retirer",
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
static const ST::string s_fr_gzProsLabel = "Plus :";
static const ST::string s_fr_gzConsLabel = "Moins :";

//Conversation options a player has when encountering an NPC
static ST::string s_fr_zTalkMenuStrings[zTalkMenuStrings_SIZE] =
{
	"Pardon ?", 	//meaning "Repeat yourself"
	"Amical",		//approach in a friendly
	"Direct",		//approach directly - let's get down to business
	"Menaçant",		//approach threateningly - talk now, or I'll blow your face off
	"Donner",
	"Recruter",
};

//Some NPCs buy, sell or repair items. These different options are available for those NPCs as well.
static ST::string s_fr_zDealerStrings[zDealerStrings_SIZE] =
{
	"Acheter/Vendre",
	"Acheter",
	"Vendre",
	"Réparer",
};

static const ST::string s_fr_zDialogActions = "OK";


//These are vehicles in the game.

static const ST::string s_fr_pVehicleStrings[pVehicleStrings_SIZE] =
{
	"Eldorado",
	"Hummer", // a hummer jeep/truck -- military vehicle
	"Camion de glaces",
	"Jeep",
	"Char",
	"Hélicoptère",
};

static const ST::string s_fr_pShortVehicleStrings[pVehicleStrings_SIZE] =
{
	"Eldor.",
	"Hummer",			// the HMVV
	"Camion",
	"Jeep",
	"Char",
	"Hélico", 				// the helicopter
};

static const ST::string s_fr_zVehicleName[pVehicleStrings_SIZE] =
{
	"Eldorado",
	"Hummer",		//a military jeep. This is a brand name.
	"Camion",			// Ice cream truck
	"Jeep",
	"Char",
	"Hélico", 		//an abbreviation for Helicopter
};


//These are messages Used in the Tactical Screen

static ST::string s_fr_TacticalStr[TacticalStr_SIZE] =
{
	"Raid aérien",
	"Appliquer les premiers soins ?",

	// CAMFIELD NUKE THIS and add quote #66.

	"%s a remarqué qu'il manque des objets dans cet envoi.",

	// The %s is a string from pDoorTrapStrings

	"La serrure est piégée par %s.",
	"Pas de serrure.",
	"La serrure n'est pas piégée.",
	// The %s is a merc name
	"%s ne possède pas la bonne clé.",
	"La serrure n'est pas piégée.",
	"Verrouillée.",
	"PORTE",
	"PIEGEE",
	"VERROUILLEE",
	"OUVERTE",
	"ENFONCEE",
	"Un interrupteur. Voulez-vous l'actionner ?",
	"Désamorcer le piège ?",
	"Plus...",

	// In the next 2 strings, %s is an item name

	"%s posé(e) à terre.",
	"%s donné(e) à %s.",

	// In the next 2 strings, %s is a name

	"%s a été payé.",
	"%d dus à %s.",
	"Choisissez la fréquence :",  	//in this case, frequency refers to a radio signal
	"Nombre de tours avant explosion :",	//how much time, in turns, until the bomb blows
	"Définir fréquence :", 	//in this case, frequency refers to a radio signal
	"Désamorcer le piège ?",
	"Enlever le drapeau bleu ?",
	"Poser un drapeau bleu ?",
	"Fin du tour",

	// In the next string, %s is a name. Stance refers to way they are standing.

	"Voulez-vous vraiment attaquer %s ?",
	"Les véhicules ne peuvent changer de position.",
	"Le robot ne peut changer de position.",

	// In the next 3 strings, %s is a name

	"%s ne peut adopter cette position ici.",
	"%s ne peut recevoir de premiers soins ici.",
	"%s n'a pas besoin de premiers soins.",
	"Impossible d'aller ici.",
	"Votre escouade est au complet. Vous ne pouvez pas ajouter quelqu'un.",	//there's non room for a recruit on the player's team

	// In the next string, %s is a name

	"%s a été recruté(e).",

	// Here %s is a name and %d is a number

	"Vous devez %d $ à %s.",

	// In the next string, %s is a name

	"Escorter %s?",

	// In the next string, the first %s is a name and the second %s is an amount of money (including $ sign)

	"Engager %s à %s la journée ?",

	// This line is used repeatedly to ask player if they wish to participate in a boxing match.

	"Voulez-vous engager le combat ?",

	// In the next string, the first %s is an item name and the
	// second %s is an amount of money (including $ sign)

	"Acheter %s pour %s ?",

	// In the next string, %s is a name

	"%s est escorté(e) par l'escouade %d.",

	// These messages are displayed during play to alert the player to a particular situation

	"ENRAYE",					//weapon is jammed.
	"Le robot a besoin de munitions calibre %s.",		//Robot is out of ammo
	"Lancer ici ? Aucune chance.",		//Merc can't throw to the destination he selected

	// These are different buttons that the player can turn on and off.

	"Furtivité (|Z)",
	"Carte (|M)",
	"OK (Fin du tour) (|D)",
	"Parler à",
	"Muet",
	"Position haute (|P|g|U|p)",
	"Niveau du curseur (|T|a|b)",
	"Escalader / Sauter",
	"Position basse (|P|g|D|n)",
	"Examiner (|C|t|r|l)",
	"Mercenaire précédent",
	"Mercenaire suivant (E|s|p|a|c|e)",
	"|Options",
	"Rafale (|B)",
	"Regarder/Pivoter (|L)",
	"Santé : %d/%d\nEnergie : %d/%d\nMoral : %s",
	"Pardon ?",					//this means "what?"
	"Suite",					//an abbrieviation for "Continued"
	"Sourdine désactivée pour %s.",
	"Sourdine activée pour %s.",
	"Etat : %d/%d\nCarburant : %d/%d",
	"Sortir du véhicule" ,
	"Changer d'escouade ( |M|a|j| |E|s|p|a|c|e )",
	"Conduire",
	"N/A",						//this is an acronym for "Not Applicable."
	"Utiliser (Mains nues)",
	"Utiliser (Arme à feu)",
	"Utiliser (Couteau)",
	"Utiliser (Explosifs)",
	"Utiliser (Trousse de soins)",
	"(Prendre)",
	"(Recharger)",
	"(Donner)",
	"%s part.",
	"%s arrive.",
	"%s n'a plus de Points d'Action.",
	"%s n'est pas disponible.",
	"%s est couvert de bandages.",
	"%s n'a plus de bandages.",
	"Ennemi dans le secteur !",
	"Pas d'ennemi en vue.",
	"Pas assez de Points d'Action.",
	"Télécommande inutilisée.",
	"La rafale a vidé le chargeur !",
	"SOLDAT",
	"CREPITUS",
	"Milice",
	"CIVIL",
	"Quitter Secteur",
	"OK",
	"Annuler",
	"Mercenaire",
	"Tous",
	"GO",
	"Carte",
	"Vous ne pouvez pas quitter ce secteur par ce côté.",
	"%s est trop loin.",
	"Effacer cime des arbres",
	"Afficher cime des arbres",
	"CORBEAU",				//Crow, as in the large black bird
	"COU",
	"TETE",
	"TORSE",
	"JAMBES",
	"Donner informations à la Reine ?",
	"Acquisition de l'ID digitale",
	"ID digitale refusée. Arme désactivée.",
	"Cible acquise",
	"Chemin bloqué",
	"Dépôt/Retrait",		//Help text over the $ button on the Single Merc Panel
	"Personne n'a besoin de premiers soins.",
	"Enr.",											// Short form of JAMMED, for small inv slots
	"Impossible d'aller ici.",					// used ( now ) for when we click on a cliff
	"La personne refuse de bouger.",
	// In the following message, '%s' would be replaced with a quantity of money (e.g. $200)
	"Etes-vous d'accord pour payer %s ?",
	"Acceptez-vous le traitement médical gratuit ?",
	"Voulez-vous épouser Daryl ?",
	"Trousseau de Clés",
	"Vous ne pouvez pas faire ça avec ce personnage.",
	"Epargner Krott ?",
	"Hors de portée",
	"Mineur",
	"Un véhicule ne peut rouler qu'entre des secteurs",
	"Impossible d'apposer des bandages maintenant",
	"Chemin bloqué pour %s",
	"Vos mercenaires capturés par l'armée de Deidranna sont emprisonnés ici !",
	"Verrou touché",
	"Verrou détruit",
	"Quelqu'un d'autre veut essayer sur cette porte.",
	"Etat : %d/%d\nCarburant : %d/%d",
	"%s ne peut pas voir %s.",  // Cannot see person trying to talk to
};

//Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different circumstances in the "exiting sector" interface.
static const ST::string s_fr_pExitingSectorHelpText[pExitingSectorHelpText_SIZE] =
{
	//Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box is checked.
	"Si vous cochez ce bouton, le secteur adjacent sera immédiatement chargé.",
	"Si vous cochez ce bouton, vous arriverez directement dans l'écran de carte\nle temps que vos mercenaires arrivent.",

	//If you attempt to leave a sector when you have multiple squads in a hostile sector.
	"Vous ne pouvez laisser vos mercenaires ici.\nVous devez d'abord nettoyer ce secteur.",

	//Because you only have one squad in the sector, and the "move all" option is checked, the "go to sector" option is locked to on.
	//The helptext explains why it is locked.
	"Faites sortir vos derniers mercenaires du secteur\npour charger le secteur adjacent.",
	"Faites sortir vos derniers mercenaires du secteur\npour aller dans l'écran de carte le temps que vos mercenaires fassent le voyage.",

	//If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being escorted.  The "single" button is disabled.
	"%s doit être escorté(e) par vos mercenaires et ne peut quitter ce secteur tout seul.",

	//If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc will be prohibited from leaving alone.
	//There are several strings depending on the gender of the merc and how many EPCs are in the squad.
	//DO NOT USE THE NEWLINE HERE AS IT IS USED FOR BOTH HELPTEXT AND SCREEN MESSAGES!
	"%s escorte %s, il ne peut quitter ce secteur seul.", //male singular
	"%s escorte %s, elle ne peut quitter ce secteur seule.", //female singular
	"%s escorte plusieurs personnages, il ne peut quitter ce secteur seul.", //male plural
	"%s escorte plusieurs personnages, elle ne peut quitter ce secteur seule.", //female plural

	//If one or more of your mercs in the selected squad aren't in range of the traversal area, then the  "move all" option is disabled,
	//and this helptext explains why.
	"Tous vos mercenaires doivent être dans les environs\npour que l'escouade avance.",

	//Standard helptext for single movement.  Explains what will happen (splitting the squad)
	"Si vous cochez ce bouton, %s voyagera seul et sera\nautomatiquement assigné à une nouvelle escouade.",

	//Standard helptext for all movement.  Explains what will happen (moving the squad)
	"Si vous cochez ce bouton, votre escouade\nquittera le secteur.",

	//This strings is used BEFORE the "exiting sector" interface is created.  If you have an EPC selected and you attempt to tactically
	//traverse the EPC while the escorting mercs aren't near enough (or dead, dying, or unconscious), this message will appear and the
	//"exiting sector" interface will not appear.  This is just like the situation where
	//This string is special, as it is not used as helptext.  Do not use the special newline character (\n) for this string.
	"%s est escorté par vos mercenaires et ne peut quitter ce secteur seul. Vos mercenaires doivent être à proximité.",
};



static const ST::string s_fr_pRepairStrings[pRepairStrings_SIZE] =
{
	"Objets", 		// tell merc to repair items in inventory
	"Site SAM", 		// tell merc to repair SAM site - SAM is an acronym for Surface to Air Missile
	"Annuler", 		// cancel this menu
	"Robot", 		// repair the robot
};


// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."

static const ST::string s_fr_sPreStatBuildString[sPreStatBuildString_SIZE] =
{
	"perd", 			// the merc has lost a statistic
	"gagne", 		// the merc has gained a statistic
	"point de",	// singular
	"points de",	// plural
	"niveau d'",	// singular
	"niveaux d'",	// plural
};

static const ST::string s_fr_sStatGainStrings[sStatGainStrings_SIZE] =
{
	"santé.",
	"agilité.",
	"dextérité.",
	"sagesse.",
	"compétence médicale.",
	"compétence en explosifs.",
	"compétence technique.",
	"tir",
	"expérience.",
	"force.",
	"commandement.",
};


static const ST::string s_fr_pHelicopterEtaStrings[pHelicopterEtaStrings_SIZE] =
{
	"Distance totale :  ", 			// total distance for helicopter to travel
	" Aller :  ", 			// distance to travel to destination
	" Retour : ", 			// distance to return from destination to airport
	"Coût : ", 		// total cost of trip by helicopter
	"AHP :  ", 			// ETA is an acronym for "estimated time of arrival"
	"L'hélicoptère n'a plus de carburant et doit se poser en terrain ennemi !",	// warning that the sector the helicopter is going to use for refueling is under enemy control ->
	"Passagers : ",
	"Sélectionner Skyrider ou l'aire d'atterrissage ?",
	"Skyrider",
	"Arrivée",
};

static const ST::string s_fr_sMapLevelString = "Niveau souterrain :"; // what level below the ground is the player viewing in mapscreen

static const ST::string s_fr_gsLoyalString = "%d%% Loyauté"; // the loyalty rating of a town ie : Loyal 53%


// error message for when player is trying to give a merc a travel order while he's underground.
static const ST::string s_fr_gsUndergroundString = "Impossible de donner des ordres.";

static const ST::string s_fr_gsTimeStrings[gsTimeStrings_SIZE] =
{
	"h",				// hours abbreviation
	"m",				// minutes abbreviation
	"s",				// seconds abbreviation
	"j",				// days abbreviation
};

// text for the various facilities in the sector

static const ST::string s_fr_sFacilitiesStrings[sFacilitiesStrings_SIZE] =
{
	"Aucun",
	"Hôpital",
	"Industrie",
	"Prison",
	"Militaire",
	"Aéroport",
	"Champ de tir",		// a field for soldiers to practise their shooting skills
};

// text for inventory pop up button

static const ST::string s_fr_pMapPopUpInventoryText[pMapPopUpInventoryText_SIZE] =
{
	"Inventaire",
	"Quitter",
};

// town strings

static const ST::string s_fr_pwTownInfoStrings[pwTownInfoStrings_SIZE] =
{
	"Taille",					// size of the town in sectors
	"Contrôle", 					// how much of town is controlled
	"Mine associée", 				// mine associated with this town
	"Loyauté",					// the loyalty level of this town
	"Principales installations", 				// main facilities in this town
	"Formation",				// state of civilian training in town
	"Milice", 					// the state of the trained civilians in the town
};

// Mine strings

static const ST::string s_fr_pwMineStrings[pwMineStrings_SIZE] =
{
	"Mine",						// 0
	"Argent",
	"Or",
	"Production quotidienne",
	"Production estimée",
	"Abandonnée",				// 5
	"Fermée",
	"Epuisée",
	"Production",
	"Etat",
	"Productivité",
	"Type de minerai",				// 10
	"Contrôle de la ville",
	"Loyauté de la ville",
};

// blank sector strings

static const ST::string s_fr_pwMiscSectorStrings[pwMiscSectorStrings_SIZE] =
{
	"Forces ennemies",
	"Secteur",
	"# d'objets",
	"Inconnu",
	"Contrôlé",
	"Oui",
	"Non",
};

// error strings for inventory

static const ST::string s_fr_pMapInventoryErrorString[pMapInventoryErrorString_SIZE] =
{
	"Sélection impossible.",  //MARK CARTER
	"%s n'est pas dans le bon secteur.",
	"En combat, vous devez prendre les objets vous-même.",
	"En combat, vous devez abandonner les objets vous-même.",
	"%s n'est pas dans le bon secteur.",
};

static const ST::string s_fr_pMapInventoryStrings[pMapInventoryStrings_SIZE] =
{
	"Lieu", 			// sector these items are in
	"Objets", 		// total number of items in sector
};


// movement menu text

static const ST::string s_fr_pMovementMenuStrings[pMovementMenuStrings_SIZE] =
{
	"Déplacement %s", 	// title for movement box
	"Route", 		// done with movement menu, start plotting movement
	"Annuler", 		// cancel this menu
	"Autre",		// title for group of mercs not on squads nor in vehicles
};


static const ST::string s_fr_pUpdateMercStrings[pUpdateMercStrings_SIZE] =
{
	"Oups :", 			// an error has occured
	"Expiration du contrat :", 	// this pop up came up due to a merc contract ending
	"Tâches accomplies :", // this pop up....due to more than one merc finishing assignments
	"Mercenaires disponibles :", // this pop up ....due to more than one merc waking up and returing to work
	"Mercenaires au repos :", // this pop up ....due to more than one merc being tired and going to sleep
	"Contrats arrivant à échéance :", 	// this pop up came up due to a merc contract ending
};

// map screen map border buttons help text

static const ST::string s_fr_pMapScreenBorderButtonHelpText[pMapScreenBorderButtonHelpText_SIZE] =
{
	"Afficher Villes (|w)",
	"Afficher |Mines",
	"Afficher Escouades & Ennemis (|T)",
	"Afficher Espace |aérien",
	"Afficher Objets (|I)",
	"Afficher Milice & Ennemis (|Z)",
};


static const ST::string s_fr_pMapScreenBottomFastHelp[pMapScreenBottomFastHelp_SIZE] =
{
	"Poste de Travail (|L)",
	"Tactique (|E|c|h|a|p)",
	"|Options",
	"Compression du temps (|+)", 	// time compress more
	"Compression du temps (|-)", 	// time compress less
	"Message précédent (|U|p)\nPage précédente (|P|g|U|p)", 	// previous message in scrollable list
	"Message suivant (|D|o|w|n)\nPage suivante (|P|g|D|n)", 	// next message in the scrollable list
	"Interrompre/Reprendre (|S|p|a|c|e)",	// start/stop time compression
};

static const ST::string s_fr_pMapScreenBottomText = "Solde actuel"; // current balance in player bank account

static const ST::string s_fr_pMercDeadString = "%s est mort.";


static const ST::string s_fr_pDayStrings = "Jour";

// the list of email sender names

static const ST::string s_fr_pSenderNameList[pSenderNameList_SIZE] =
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
	"M.I.S. Assurance",
	"Bobby Ray",
	"Kingpin",
	"John Kulba",
	"A.I.M.",
};


// new mail notify string
static const ST::string s_fr_pNewMailStrings = "Vous avez des messages...";


// confirm player's intent to delete messages

static const ST::string s_fr_pDeleteMailStrings[pDeleteMailStrings_SIZE] =
{
	"Effacer message ?",
	"Effacer message NON CONSULTE ?",
};


// the sort header strings

static const ST::string s_fr_pEmailHeaders[pEmailHeaders_SIZE] =
{
	"De :",
	"Sujet :",
	"Date :",
};

// email titlebar text
static const ST::string s_fr_pEmailTitleText = "Boîte aux lettres";


// the financial screen strings
static const ST::string s_fr_pFinanceTitle = "Bookkeeper Plus"; // the name we made up for the financial program in the game

static const ST::string s_fr_pFinanceSummary[pFinanceSummary_SIZE] =
{
	"Crédit :", 				// credit (subtract from) to player's account
	"Débit :", 				// debit (add to) to player's account
	"Revenus (hier) :",
	"Dépôts (hier) :",
	"Dépenses (hier) :",
	"Solde (fin de journée) :",
	"Revenus (aujourd'hui) :",
	"Dépôts (aujourd'hui) :",
	"Dépenses (aujourd'hui) :",
	"Solde actuel :",
	"Revenus (prévision) :",
	"Solde (prévision) :", 		// projected balance for player for tommorow
};


// headers to each list in financial screen

static const ST::string s_fr_pFinanceHeaders[pFinanceHeaders_SIZE] =
{
	"Jour", 				// the day column
	"Crédit", 			// the credits column (to ADD money to your account)
	"Débit",				// the debits column (to SUBTRACT money from your account)
	"Transaction", // transaction type - see TransactionText below
	"Solde", 		// balance at this point in time
	"Page", 				// page number
	"Jour(s)", 			// the day(s) of transactions this page displays
};


static const ST::string s_fr_pTransactionText[pTransactionText_SIZE] =
{
	"Intérêts cumulés",			// interest the player has accumulated so far
	"Dépôt anonyme",
	"Commission",
	"%s embauché(e) sur le site AIM", // Merc was hired
	"Achats Bobby Ray", 		// Bobby Ray is the name of an arms dealer
	"Règlement M.E.R.C.",
	"Acompte médical pour %s", 		// medical deposit for merc
	"Analyse IMP", 		// IMP is the acronym for International Mercenary Profiling
	"Assurance pour %s",
	"Réduction d'assurance pour %s",
	"Extension d'assurance pour %s", 				// johnny contract extended
	"Annulation d'assurance pour %s",
	"Indemnisation pour %s", 		// insurance claim for merc
	"Ext. contrat de %s (1 jour).", 				// entend mercs contract by a day
	"Ext. contrat de %s (1 semaine).",
	"Ext. contrat de %s (2 semaines).",
	"Revenus des mines",
	"", //String nuked
	"Achat de fleurs",
	"Remboursement médical pour %s",
	"Remb. médical partiel pour %s",
	"Pas de remb. médical pour %s",
	"Paiement à %s",		// %s is the name of the npc being paid
	"Transfert de fonds pour %s", 			// transfer funds to a merc
	"Transfert de fonds de %s", 		// transfer funds from a merc
	"Coût milice de %s", // initial cost to equip a town's militia
	"Achats à %s.",	//is used for the Shop keeper interface.  The dealers name will be appended to the en d of the string.
	"Montant déposé par %s.",
};

// helicopter pilot payment

static const ST::string s_fr_pSkyriderText[pSkyriderText_SIZE] =
{
	"Skyrider a reçu %d $", 			// skyrider was paid an amount of money
	"Skyrider attend toujours ses %d $", 		// skyrider is still owed an amount of money
	"Skyrider n'a pas de passagers. Si vous voulez envoyer des mercenaires dans ce secteur, n'oubliez pas de les assigner à l'hélicoptère.",
};


// strings for different levels of merc morale

static const ST::string s_fr_pMoralStrings[pMoralStrings_SIZE] =
{
	"Superbe",
	"Bon",
	"Stable",
	"Bas",
	"Paniqué",
	"Mauvais",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.
static const ST::string s_fr_str_left_equipment   = "L'équipement de %s est maintenant disponible à %s (%c%d).";

// Status that appears on the Map Screen

static const ST::string s_fr_pMapScreenStatusStrings[pMapScreenStatusStrings_SIZE] =
{
	"Santé",
	"Energie",
	"Moral",
	"Etat",	// the condition of the current vehicle (its "Santé")
	"Carburant",	// the fuel level of the current vehicle (its "energy")
};


static const ST::string s_fr_pMapScreenPrevNextCharButtonHelpText[pMapScreenPrevNextCharButtonHelpText_SIZE] =
{
	"Mercenaire précédent (|G|a|u|c|h|e)", 			// previous merc in the list
	"Mercenaire suivant (|D|r|o|i|t|e)", 				// next merc in the list
};


static const ST::string s_fr_pEtaString = "HPA :"; // eta is an acronym for Estimated Time of Arrival

static const ST::string s_fr_pTrashItemText[pTrashItemText_SIZE] =
{
	"Vous ne le reverrez jamais. Vous êtes sûr de vous ?", 	// do you want to continue and lose the item forever
	"Cet objet a l'air VRAIMENT important. Vous êtes bien sûr (mais alors BIEN SUR) de vouloir l'abandonner ?", // does the user REALLY want to trash this item
};


static const ST::string s_fr_pMapErrorString[pMapErrorString_SIZE] =
{
	"L'escouade ne peut se déplacer si l'un de ses membres se repose.",

//1-5
	"Déplacez d'abord votre escouade.",
	"Des ordres de mouvement ? C'est un secteur hostile !",
	"Les mercenaires doivent d'abord être assignés à un véhicule.",
	"Vous n'avez plus aucun membre dans votre escouade.", // you have non members, can't do anything
	"Le mercenaire ne peut obéir.", // merc can't comply with your order
//6-10
	"%s doit être escorté. Mettez-le dans une escouade.", // merc can't move unescorted .. for a male
	"%s doit être escortée. Mettez-la dans une escouade.", // for a female
	"Ce mercenaire n'est pas encore arrivé !",
	"Il faudrait d'abord revoir les termes du contrat...",
	"",
//11-15
	"Des ordres de mouvement ? Vous êtes en plein combat !",
	"Vous êtes tombé dans une embuscade de chats sauvages dans le secteur %s !",
	"Vous venez d'entrer dans le repaire des chats sauvages (secteur I16) !",
	"",
	"Le site SAM en %s est sous contrôle ennemi.",
//16-20
	"La mine en %s est sous contrôle ennemi. Votre revenu journalier est réduit de %s.",
	"L'ennemi vient de prendre le contrôle du secteur %s.",
	"L'un au moins de vos mercenaires ne peut effectuer cette tâche.",
	"%s ne peut rejoindre %s (plein).",
	"%s ne peut rejoindre %s (éloignement).",
//21-25
	"La mine en %s a été reprise par les forces de Deidranna !",
	"Les forces de Deidranna viennent d'envahir le site SAM en %s",
	"Les forces de Deidranna viennent d'envahir %s",
	"Les forces de Deidranna ont été aperçues en %s.",
	"Les forces de Deidranna viennent de prendre %s.",
//26-30
	"L'un au moins de vos mercenaires ne peut se reposer.",
	"L'un au moins de vos mercenaires ne peut être réveillé.",
	"La milice n'apparaît sur l'écran qu'une fois son entraînement achevé.",
	"%s ne peut recevoir d'ordre de mouvement pour le moment.",
	"Les miliciens qui ne se trouvent pas dans les limites d'une ville ne peuvent être déplacés.",
//31-35
	"Vous ne pouvez pas entraîner de milice en %s.",
	"Un véhicule ne peut se déplacer s'il est vide !",
	"L'état de santé de %s ne lui permet pas de voyager !",
	"Vous devez d'abord quitter le musée !",
	"%s est mort !",
//36-40
	"%s ne peut passer à %s (en mouvement)",
	"%s ne peut pas pénétrer dans le véhicule de cette façon",
	"%s ne peut rejoindre %s",
	"Vous devez d'abord engager des mercenaires !",
	"Ce véhicule ne peut circuler que sur les routes !",
//41-45
	"Vous ne pouvez réaffecter des mercenaires qui sont en déplacement",
	"Plus d'essence !",
	"%s est trop fatigué(e) pour entreprendre ce voyage.",
	"Personne n'est capable de conduire ce véhicule.",
	"L'un au moins des membres de cette escouade ne peut se déplacer.",
//46-50
	"L'un au moins des AUTRES mercenaires ne peut se déplacer.",
	"Le véhicule est trop endommagé !",
	"Deux mercenaires au plus peuvent être assignés à l'entraînement de la milice dans chaque secteur.",
	"Le robot ne peut se déplacer sans son contrôleur. Mettez-les ensemble dans la même escouade.",
};


// help text used during strategic route plotting
static const ST::string s_fr_pMapPlotStrings[pMapPlotStrings_SIZE] =
{
	"Cliquez à nouveau sur votre destination pour la confirmer ou cliquez sur d'autres secteurs pour définir de nouvelles étapes.",
	"Route confirmée.",
	"Destination inchangée.",
	"Route annulée.",
	"Route raccourcie.",
};


// help text used when moving the merc arrival sector
static const ST::string s_fr_pBullseyeStrings[pBullseyeStrings_SIZE] =
{
	"Cliquez sur la nouvelle destination de vos mercenaires.",
	"OK. Les mercenaires arriveront en %s",
	"Les mercenaires ne peuvent être déployés ici, l'espace aérien n'est pas sécurisé !",
	"Annulé. Secteur d'arrivée inchangé.",
	"L'espace aérien en %s n'est plus sûr ! Le secteur d'arrivée est maintenant %s.",
};


// help text for mouse regions

static const ST::string s_fr_pMiscMapScreenMouseRegionHelpText[pMiscMapScreenMouseRegionHelpText_SIZE] =
{
	"Inventaire (|E|n|t|r|é|e)",
	"Lancer objet",
	"Quitter Inventaire (|E|n|t|r|é|e)",
};


static const ST::string s_fr_str_he_leaves_where_drop_equipment  = "%s doit-il abandonner son équipement sur place (%s) ou à %s (%s) avant de quitter Arulco?";
static const ST::string s_fr_str_she_leaves_where_drop_equipment = "%s doit-elle abandonner son équipement sur place (%s) ou à %s (%s) avant de quitter Arulco?";
static const ST::string s_fr_str_he_leaves_drops_equipment       = "%s est sur le point de partir et laissera son équipement à %s.";
static const ST::string s_fr_str_she_leaves_drops_equipment      = "%s est sur le point de partir et laissera son équipement à %s.";


// Text used on IMP Web Pages

static const ST::string s_fr_pImpPopUpStrings[pImpPopUpStrings_SIZE] =
{
	"Code Incorrect",
	"Vous allez établir un nouveau profil. Etes-vous sûr de vouloir recommencer ?",
	"Veuillez entrer votre nom et votre sexe.",
	"Vous n'avez pas les moyens de vous offrir une analyse de profil.",
	"Option inaccessible pour le moment.",
	"Pour que cette analyse soit efficace, il doit vous rester au moins une place dans votre escouade.",
	"Profil déjà établi.",
};


// button labels used on the IMP site

static const ST::string s_fr_pImpButtonText[pImpButtonText_SIZE] =
{
	"Nous", 			// about the IMP site
	"COMMENCER", 			// begin profiling
	"Personnalité", 		// personality section
	"Caractéristiques", 		// personal stats/attributes section
	"Portrait", 			// the personal portrait selection
	"Voix %d", 			// the voice selection
	"OK", 			// done profiling
	"Recommencer", 		// start over profiling
	"Oui, la réponse en surbrillance me convient.",
	"Oui",
	"Non",
	"Terminé", 			// finished answering questions
	"Préc.", 			// previous question..abbreviated form
	"Suiv.", 			// next question
	"OUI, JE SUIS SUR.", 		// oui, I am certain
	"NON, JE VEUX RECOMMENCER.", // non, I want to start over the profiling process
	"OUI",
	"NON",
	"Retour", 			// back one page
	"Annuler", 			// cancel selection
	"Oui, je suis sûr.",
	"Non, je ne suis pas sûr.",
	"Registre", 			// the IMP site registry..when name and gender is selected
	"Analyse", 			// analyzing your profile results
	"OK",
	"Voix",
};

static const ST::string s_fr_pExtraIMPStrings[pExtraIMPStrings_SIZE] =
{
	"Pour lancer l'analyse, cliquez sur Personnalité.",
	"Cliquez maintenant sur Caractéristiques.",
	"Passons maintenant à la galerie de portraits.",
	"Pour que l'analyse soit complète, choisissez une voix.",
};

static const ST::string s_fr_pFilesTitle = "Fichiers";

static const ST::string s_fr_pFilesSenderList[pFilesSenderList_SIZE] =
{
	"Rapport Arulco", // the recon report sent to the player. Recon is an abbreviation for reconissance
	"Intercept #1",   // first intercept file .. Intercept is the title of the organization sending the file...similar in function to INTERPOL/CIA/KGB..refer to fist record in files.txt for the translated title
	"Intercept #2",   // second intercept file
	"Intercept #3",   // third intercept file
	"Intercept #4",   // fourth intercept file
	"Intercept #5",   // fifth intercept file
	"Intercept #6",   // sixth intercept file
};

// Text having to do with the History Log
static const ST::string s_fr_pHistoryTitle = "Historique";

static const ST::string s_fr_pHistoryHeaders[pHistoryHeaders_SIZE] =
{
	"Jour", 			// the day the history event occurred
	"Page", 			// the current page in the history report we are in
	"Jour", 			// the days the history report occurs over
	"Lieu", 			// location (in sector) the event occurred
	"Evénement", 			// the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
static const ST::string s_fr_pHistoryStrings[pHistoryStrings_SIZE] =
{
	"",																						// leave this line blank
	//1-5
	"%s engagé(e) sur le site A.I.M.", 										// merc was hired from the aim site
	"%s engagé(e) sur le site M.E.R.C.", 									// merc was hired from the aim site
	"%s meurt.", 															// merc was killed
	"Versements M.E.R.C.",								// paid outstanding bills at MERC
	"Ordre de mission d'Enrico Chivaldori accepté",
	//6-10
	"Profil IMP",
	"Souscription d'un contrat d'assurance pour %s.", 				// insurance contract purchased
	"Annulation du contrat d'assurance de %s.", 				// insurance contract canceled
	"Indemnité pour %s.", 							// insurance claim payout for merc
	"Extension du contrat de %s (1 jour).", 						// Extented "mercs name"'s for a day
	//11-15
	"Extension du contrat de %s (1 semaine).", 					// Extented "mercs name"'s for a week
	"Extension du contrat de %s (2 semaines).", 					// Extented "mercs name"'s 2 weeks
	"%s a été renvoyé(e).", 													// "merc's name" was dismissed.
	"%s a démissionné.", 																		// "merc's name" quit.
	"quête commencée.", 															// a particular quest started
	//16-20
	"quête achevée.",
	"Entretien avec le chef des mineurs de %s",									// talked to head miner of town
	"Libération de %s",
	"Activation du mode triche",
	"Le ravitaillement devrait arriver demain à Omerta",
	//21-25
	"%s a quitté l'escouade pour épouser Daryl Hick",
	"Expiration du contrat de %s.",
	"Recrutement de %s.",
	"Plainte d'Enrico pour manque de résultats",
	"Victoire",
	//26-30
	"La mine de %s commence à s'épuiser",
	"La mine de %s est épuisée",
	"La mine de %s a été fermée",
	"La mine de %s a été réouverte",
	"Une prison du nom de Tixa a été découverte.",
	//31-35
	"Rumeurs sur une usine d'armes secrètes : Orta.",
	"Les chercheurs d'Orta vous donnent des fusils à roquettes.",
	"Deidranna fait des expériences sur les cadavres.",
	"Frank parle de combats organisés à San Mona.",
	"Un témoin pense avoir aperçu quelque chose dans les mines.",
	//36-40
	"Rencontre avec Devin - vend des explosifs.",
	"Rencontre avec Mike, le fameux ex-mercenaire de l'AIM !",
	"Rencontre avec Tony - vend des armes.",
	"Fusil à roquettes récupéré auprès du Sergent Krott.",
	"Acte de propriété du magasin d'Angel donné à Kyle.",
	//41-45
	"Madlab propose de construire un robot.",
	"Gabby fait des décoctions rendant invisible aux créatures.",
	"Keith est hors-jeu.",
	"Howard fournit du cyanure à la Reine Deidranna.",
	"Rencontre avec Keith - vendeur à Cambria.",
	//46-50
	"Rencontre avec Howard - pharmacien à Balime",
	"Rencontre avec Perko - réparateur en tous genres.",
	"Rencontre avec Sam de Balime - vendeur de matériel.",
	"Franz vend du matériel électronique.",
	"Arnold tient un magasin de réparations à Grumm.",
	//51-55
	"Fredo répare le matériel électronique à Grumm.",
	"Don provenant d'un homme influent de Balime.",
	"Rencontre avec Jake, vendeur de pièces détachées.",
	"Clé électronique reçue.",
	"Corruption de Walter pour ouvrir l'accès aux sous-sols.",
	//56-60
	"Dave refait gratuitement le plein s'il a du carburant.",
	"Pot-de-vin donné à Pablo.",
	"Kingpin cache un trésor dans la mine de San Mona.",
	"Victoire de %s dans l'Extreme Fighting",
	"Défaite de %s dans l'Extreme Fighting",
	//61-65
	"Disqualification de %s dans l'Extreme Fighting",
	"Importante somme découverte dans la mine abandonnée.",
	"Rencontre avec un tueur engagé par Kingpin.",
	"Perte du secteur",				//ENEMY_INVASION_CODE
	"Secteur défendu",
	//66-70
	"Défaite",							//ENEMY_ENCOUNTER_CODE
	"Embuscade",						//ENEMY_AMBUSH_CODE
	"Embuscade ennemie déjouée",
	"Echec de l'attaque",			//ENTERING_ENEMY_SECTOR_CODE
	"Réussite de l'attaque !",
	//71-75
	"Attaque de créatures",			//CREATURE_ATTACK_CODE
	"Attaque de chats sauvages",			//BLOODCAT_AMBUSH_CODE
	"Elimination des chats sauvages",
	"%s a été tué(e)",
	"Tête de terroriste donnée à Carmen",
	"Reste Slay",
	"%s a été tué(e)",
};

static const ST::string s_fr_pHistoryLocations = "N/A"; // N/A is an acronym for Not Applicable

// icon text strings that appear on the laptop

static const ST::string s_fr_pLaptopIcons[pLaptopIcons_SIZE] =
{
	"E-mail",
	"Internet",
	"Finances",
	"Personnel",
	"Historique",
	"Fichiers",
	"Eteindre",
	"sir-FER 4.0",			// our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added

static const ST::string s_fr_pBookMarkStrings[pBookMarkStrings_SIZE] =
{
	"A.I.M.",
	"Bobby Ray",
	"I.M.P",
	"M.E.R.C.",
	"Morgue",
	"Fleuriste",
	"Assurance",
	"Annuler",
};

// When loading or download a web page

static const ST::string s_fr_pDownloadString[pDownloadString_SIZE] =
{
	"Téléchargement",
	"Chargement",
};

//This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

static const ST::string s_fr_gsAtmStartButtonText[gsAtmStartButtonText_SIZE] =
{
	"Stats", 			// view stats of the merc
	"Inventaire", 			// view the inventory of the merc
	"Tâche",
};

// Web error messages. Please use foreign language equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator

static const ST::string s_fr_pErrorStrings = "Connexion à l'hôte.";


static const ST::string s_fr_pPersonnelString = "Mercs :"; // mercs we have


static const ST::string s_fr_pWebTitle = "sir-FER 4.0"; // our name for the version of the browser, play on company name


// The titles for the web program title bar, for each page loaded

static const ST::string s_fr_pWebPagesTitles[pWebPagesTitles_SIZE] =
{
	"A.I.M.",
	"Membres A.I.M.",
	"Galerie A.I.M.",		// a mug shot is another name for a portrait
	"Tri A.I.M.",
	"A.I.M.",
	"Anciens A.I.M.",
	"Règlement A.I.M.",
	"Historique A.I.M.",
	"Liens A.I.M.",
	"M.E.R.C.",
	"Comptes M.E.R.C.",
	"Enregistrement M.E.R.C.",
	"Index M.E.R.C.",
	"Bobby Ray",
	"Bobby Ray - Armes",
	"Bobby Ray - Munitions",
	"Bobby Ray - Armures",
	"Bobby Ray - Divers",							//misc is an abbreviation for miscellaneous
	"Bobby Ray - Occasions",
	"Bobby Ray - Commande",
	"I.M.P.",
	"I.M.P.",
	"Service des Fleuristes Associés",
	"Service des Fleuristes Associés - Exposition",
	"Service des Fleuristes Associés - Bon de commande",
	"Service des Fleuristes Associés - Cartes",
	"Malleus, Incus & Stapes Courtiers",
	"Information",
	"Contrat",
	"Commentaires",
	"Morgue McGillicutty",
	"URL introuvable.",
	"Bobby Ray - Dernières commandes",
	"",
	"",
};

static const ST::string s_fr_pShowBookmarkString[pShowBookmarkString_SIZE] =
{
	"Sir-Help",
	"Cliquez à nouveau pour accéder aux Favoris.",
};

static const ST::string s_fr_pLaptopTitles[pLaptopTitles_SIZE] =
{
	"Boîte aux lettres",
	"Fichiers",
	"Personnel",
	"Bookkeeper Plus",
	"Historique",
};

static const ST::string s_fr_pPersonnelDepartedStateStrings[pPersonnelDepartedStateStrings_SIZE] =
{
	//reasons why a merc has left.
	"Mort en mission",
	"Parti(e)",
	"Mariage",
	"Contrat terminé",
	"Quitter",
};
// personnel strings appearing in the Personnel Manager on the laptop

static const ST::string s_fr_pPersonelTeamStrings[pPersonelTeamStrings_SIZE] =
{
	"Equipe actuelle",
	"Départs",
	"Coût quotidien :",
	"Coût maximum :",
	"Coût minimum :",
	"Morts en mission :",
	"Partis :",
	"Autres :",
};


static const ST::string s_fr_pPersonnelCurrentTeamStatsStrings[pPersonnelCurrentTeamStatsStrings_SIZE] =
{
	"Minimum",
	"Moyenne",
	"Maximum",
};


static const ST::string s_fr_pPersonnelTeamStatsStrings[pPersonnelTeamStatsStrings_SIZE] =
{
	"SAN",
	"AGI",
	"DEX",
	"FOR",
	"COM",
	"SAG",
	"NIV",
	"TIR",
	"TECH",
	"EXPL",
	"MED",
};


// horizontal and vertical indices on the map screen

static const ST::string s_fr_pMapVertIndex[pMapVertIndex_SIZE] =
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

static const ST::string s_fr_pMapHortIndex[pMapHortIndex_SIZE] =
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

static const ST::string s_fr_pMapDepthIndex[pMapDepthIndex_SIZE] =
{
	"",
	"-1",
	"-2",
	"-3",
};

// text that appears on the contract button

static const ST::string s_fr_pContractButtonString = "Contrat";

// text that appears on the update panel buttons

static const ST::string s_fr_pUpdatePanelButtons[pUpdatePanelButtons_SIZE] =
{
	"Continuer",
	"Stop",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle

static ST::string s_fr_LargeTacticalStr[LargeTacticalStr_SIZE] =
{
	"Vous avez été vaincu dans ce secteur !",
	"L'ennemi, sans aucune compassion, ne fait pas de quartier !",
	"Vos mercenaires inconscients ont été capturés !",
	"Vos mercenaires ont été faits prisonniers.",
};


//Insurance Contract.c
//The text on the buttons at the bottom of the screen.

static const ST::string s_fr_InsContractText[InsContractText_SIZE] =
{
	"Précédent",
	"Suivant",
	"Accepter",
	"Annuler",
};



//Insurance Info
// Text on the buttons on the bottom of the screen

static const ST::string s_fr_InsInfoText[InsInfoText_SIZE] =
{
	"Précédent",
	"Suivant",
};



//For use at the M.E.R.C. web site. Text relating to the player's account with MERC

static const ST::string s_fr_MercAccountText[MercAccountText_SIZE] =
{
	// Text on the buttons on the bottom of the screen
	"Autoriser",
	"Home",
	"Compte # :",
	"Merc",
	"Jours",
	"Taux",	//5
	"Montant",
	"Total :",
	"Désirez-vous autoriser le versement de %s ?",		//the %s is a string that contains the dollar amount ( ex. "$150" )
};



//For use at the M.E.R.C. web site. Text relating a MERC mercenary


static const ST::string s_fr_MercInfo[MercInfo_SIZE] =
{
	"Précédent",
	"Engager",
	"Suivant",
	"Infos complémentaires",
	"Home",
	"Engagé",
	"Salaire :",
	"Par jour",
	"Décédé(e)",

	"Vous ne pouvez engager plus de 18 mercenaires.",
	"Indisponible",
};



// For use at the M.E.R.C. web site. Text relating to opening an account with MERC

static const ST::string s_fr_MercNoAccountText[MercNoAccountText_SIZE] =
{
	//Text on the buttons at the bottom of the screen
	"Ouvrir compte",
	"Annuler",
	"Vous ne possédez pas de compte. Désirez-vous en ouvrir un ?",
};



// For use at the M.E.R.C. web site. MERC Homepage

static const ST::string s_fr_MercHomePageText[MercHomePageText_SIZE] =
{
	//Description of various parts on the MERC page
	"Speck T. Kline, fondateur",
	"Cliquez ici pour ouvrir un compte",
	"Cliquez ici pour voir votre compte",
	"Cliquez ici pour consulter les fichiers",
	// The version number on the video conferencing system that pops up when Speck is talking
	"Speck Com v3.2",
};

// For use at MiGillicutty's Web Page.

static const ST::string s_fr_sFuneralString[sFuneralString_SIZE] =
{
	"Morgue McGillicutty : A votre écoute depuis 1983.",
	"Murray \"Pops\" McGillicutty, notre directeur bien aimé, est un ancien mercenaire de l'AIM. Sa spécialité : la mort des autres.",
	"Pops l'a côtoyée pendant si longtemps qu'il est un expert de la mort, à tous points de vue.",
	"La morgue McGillicutty vous offre un large éventail de services funéraires, depuis une écoute compréhensive jusqu'à la reconstitution des corps... dispersés.",
	"Laissez donc la morgue McGillicutty vous aider, pour que votre compagnon repose enfin en paix.",

	// Text for the various links available at the bottom of the page
	"ENVOYER FLEURS",
	"CERCUEILS & URNES",
	"CREMATION",
	"SERVICES FUNERAIRES",
	"NOTRE ETIQUETTE",

	// The text that comes up when you click on any of the links ( except for send flowers ).
	"Le concepteur de ce site s'est malheureusement absenté pour cause de décès familial. Il reviendra dès que possible pour rendre ce service encore plus efficace.",
	"Veuillez croire en nos sentiments les plus respectueux dans cette période qui doit vous être douloureuse.",
};

// Text for the florist Home page

static const ST::string s_fr_sFloristText[sFloristText_SIZE] =
{
	//Text on the button on the bottom of the page

	"Vitrine",

	//Address of United Florist

	"\"Nous livrons partout dans le monde\"",
	"0-800-SENTMOI",
	"333 NoseGay Dr, Seedy City, CA USA 90210",
	"http://www.sentmoi.com",

	// detail of the florist page

	"Rapides et efficaces !",
	"Livraison en 24 heures partout dans le monde (ou presque).",
	"Les prix les plus bas (ou presque) !",
	"Si vous trouvez moins cher, nous vous livrons gratuitement une douzaine de roses !",
	"Flore, Faune & Fleurs depuis 1981.",
	"Nos bombardiers (recyclés) vous livrent votre bouquet dans un rayon de 20 km (ou presque). N'importe quand - N'importe où !",
	"Nous répondons à tous vos besoins (ou presque) !",
	"Bruce, notre expert fleuriste-conseil, trouvera pour vous les plus belles fleurs et vous composera le plus beau bouquet que vous ayez vu !",
	"Et n'oubliez pas que si nous ne l'avons pas, nous pouvons le faire pousser - et vite !",
};



//Florist OrderForm

static const ST::string s_fr_sOrderFormText[sOrderFormText_SIZE] =
{
	//Text on the buttons

	"Retour",
	"Envoi",
	"Annuler",
	"Galerie",

	"Nom du bouquet :",
	"Prix :",			//5
	"Référence :",
	"Date de livraison",
	"jour suivant",
	"dès que possible",
	"Lieu de livraison",			//10
	"Autres services",
	"Pot Pourri (10$)",
	"Roses Noires (20$)",
	"Nature Morte (10$)",
	"Gâteau (si dispo)(10$)",		//15
	"Carte personnelle :",
	"Veuillez écrire votre message en 75 caractères maximum...",
	"...ou utiliser l'une de nos",

	"CARTES STANDARDS",
	"Informations",//20

	//The text that goes beside the area where the user can enter their name

	"Nom :",
};




//Florist Gallery.c

static const ST::string s_fr_sFloristGalleryText[sFloristGalleryText_SIZE] =
{
	//text on the buttons

	"Préc.",	//abbreviation for previous
	"Suiv.",	//abbreviation for next

	"Cliquez sur le bouquet que vous désirez commander.",
	"Note : les bouquets \"pot pourri\" et \"nature morte\" vous seront facturés 10$ supplémentaires.",

	//text on the button

	"Home",
};

//Florist Cards

static const ST::string s_fr_sFloristCards[sFloristCards_SIZE] =
{
	"Faites votre choix",
	"Retour",
};



// Text for Bobby Ray's Mail Order Site

static const ST::string s_fr_BobbyROrderFormText[BobbyROrderFormText_SIZE] =
{
	"Commande",				//Title of the page
	"Qté",					// The number of items ordered
	"Poids (%s)",			// The weight of the item
	"Description",				// The name of the item
	"Prix unitaire",				// the item's weight
	"Total",				//5	// The total price of all of items of the same type
	"Sous-total",				// The sub total of all the item totals added
	"Transport",		// S&H is an acronym for Shipping and Handling
	"Total",			// The grand total of all item totals + the shipping and handling
	"Lieu de livraison",
	"Type d'envoi",			//10	// See below
	"Coût (par %s.)",			// The cost to ship the items
	"Du jour au lendemain",			// Gets deliverd the next day
	"2 c'est mieux qu'un",			// Gets delivered in 2 days
	"Jamais 2 sans 3",			// Gets delivered in 3 days
	"Effacer commande",//15			// Clears the order page
	"Confirmer commande",			// Accept the order
	"Retour",				// text on the button that returns to the previous page
	"Home",				// Text on the button that returns to the home page
	"* Matériel d'occasion",		// Disclaimer stating that the item is used
	"Vous n'avez pas les moyens.",		//20	// A popup message that to warn of not enough money
	"<AUCUNE>",				// Gets displayed when there is non valid city selected
	"Etes-vous sûr de vouloir envoyer cette commande à %s ?",		// A popup that asks if the city selected is the correct one
	"Poids total **",			// Displays the weight of the package
	"** Pds Min.",				// Disclaimer states that there is a minimum weight for the package
	"Envois",
};


// This text is used when on the various Bobby Ray Web site pages that sell items

static const ST::string s_fr_BobbyRText[BobbyRText_SIZE] =
{
	"Pour commander",				// Title
	// instructions on how to order
	"Cliquez sur les objets désirés. Cliquez à nouveau pour sélectionner plusieurs exemplaires d'un même objet. Effectuez un clic droit pour désélectionner un objet. Il ne vous reste plus qu'à passer commande.",

	//Text on the buttons to go the various links

	"Objets précédents",		//
	"Armes", 			//3
	"Munitions",			//4
	"Armures",			//5
	"Divers",			//6	//misc is an abbreviation for miscellaneous
	"Occasion",			//7
	"Autres objets",
	"BON DE COMMANDE",
	"Home",			//10

	//The following lines provide information on the items

	"Poids :",		// Weight of all the items of the same type
	"Cal :",			// the caliber of the gun
	"Chrg :",			// number of rounds of ammo the Magazine can hold
	"Por :",			// The range of the gun
	"Dgt :",			// Damage of the weapon
	"CDT :",			// Weapon's Rate Of Fire, acronym ROF
	"Prix :",			// Cost of the item
	"En réserve :",			// The number of items still in the store's inventory
	"Qté commandée :",		// The number of items on order
	"Endommagé",			// If the item is damaged
	"Sous-total :",			// The total cost of all items on order
	"* % efficacité",		// if the item is damaged, displays the percent function of the item

	//Popup that tells the player that they can only order 10 items at a time

	"Pas de chance ! Vous ne pouvez commander que 10 objets à la fois. Si vous désirez passer une commande plus importante, il vous faudra remplir un nouveau bon de commande.",

	// A popup that tells the user that they are trying to order more items then the store has in stock

	"Nous sommes navrés, mais nos stocks sont vides. N'hésitez pas à revenir plus tard !",

	//A popup that tells the user that the store is temporarily sold out

	"Nous sommes navrés, mais nous n'en avons plus en rayon.",
};


// The following line is used on the Ammunition page.  It is used for help text
// to display how many items the player's merc has that can use this type of
// ammo.
static const ST::string s_fr_str_bobbyr_guns_num_guns_that_use_ammo = "Votre équipe possède %d arme(s) qui utilise(nt) ce type de munitions";


// Text for Bobby Ray's Home Page

static const ST::string s_fr_BobbyRaysFrontText[BobbyRaysFrontText_SIZE] =
{
	//Details on the web site

	"Vous cherchez des armes et du matériel militaire ? Vous avez frappé à la bonne porte",
	"Un seul credo : force de frappe !",
	"Occasions et secondes mains",

	//Text for the various links to the sub pages

	"Divers",
	"ARMES",
	"MUNITIONS",		//5
	"ARMURES",

	//Details on the web site

	"Si nous n'en vendons pas, c'est que ça n'existe pas !",
	"En Construction",
};



// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug shot page

static const ST::string s_fr_AimSortText[AimSortText_SIZE] =
{
	"Membres A.I.M.",				// Title
	// Title for the way to sort
	"Tri par :",

	//Text of the links to other AIM pages

	"Afficher l'index de la galerie de portraits",
	"Consulter le fichier individuel",
	"Consulter la galerie des anciens de l'A.I.M."
};


// text to display how the entries will be sorted
static const ST::string s_fr_str_aim_sort_price        = "Prix";
static const ST::string s_fr_str_aim_sort_experience   = "Expérience";
static const ST::string s_fr_str_aim_sort_marksmanship = "Tir";
static const ST::string s_fr_str_aim_sort_medical      = "Médecine";
static const ST::string s_fr_str_aim_sort_explosives   = "Explosifs";
static const ST::string s_fr_str_aim_sort_mechanical   = "Technique";
static const ST::string s_fr_str_aim_sort_ascending    = "Ascendant";
static const ST::string s_fr_str_aim_sort_descending   = "Descendant";


//Aim Policies.c
//The page in which the AIM policies and regulations are displayed

static const ST::string s_fr_AimPolicyText[AimPolicyText_SIZE] =
{
	// The text on the buttons at the bottom of the page

	"Précédent",
	"Home AIM",
	"Index",
	"Suivant",
	"Je refuse",
	"J'accepte",
};



//Aim Member.c
//The page in which the players hires AIM mercenaries

// Instructions to the user to either start video conferencing with the merc, or to go the mug shot index

static const ST::string s_fr_AimMemberText[AimMemberText_SIZE] =
{
	"Cliquez pour",
	"contacter le mercenaire.",
	"Clic droit pour",
	"afficher l'index.",
};

//Aim Member.c
//The page in which the players hires AIM mercenaries

static const ST::string s_fr_CharacterInfo[CharacterInfo_SIZE] =
{
	// the contract expenses' area

	"Paie",
	"Contrat",
	"1 jour",
	"1 semaine",
	"2 semaines",

	// text for the buttons that either go to the previous merc,
	// start talking to the merc, or go to the next merc

	"Précédent",
	"Contacter",
	"Suivant",

	"Info. complémentaires",				// Title for the additional info for the merc's bio
	"Membres actifs", // Title of the page
	"Matériel optionnel :",				// Displays the optional gear cost
	"Dépôt Médical",			// If the merc required a medical deposit, this is displayed
};


//Aim Member.c
//The page in which the player's hires AIM mercenaries

//The following text is used with the video conference popup

static const ST::string s_fr_VideoConfercingText[VideoConfercingText_SIZE] =
{
	"Contrat :",				//Title beside the cost of hiring the merc

	//Text on the buttons to select the length of time the merc can be hired

	"1 jour",
	"1 semaine",
	"2 semaines",

	//Text on the buttons to determine if you want the merc to come with the equipment

	"Pas d'équipement",
	"Acheter équipement",

	// Text on the Buttons

	"TRANSFERT",			// to actually hire the merc
	"Annuler",				// go back to the previous menu
	"ENGAGER",				// go to menu in which you can hire the merc
	"RACCROCHER",				// stops talking with the merc
	"OK",
	"MESSAGE",			// if the merc is not there, you can leave a message

	//Text on the top of the video conference popup

	"Conférence vidéo avec",
	"Connexion. . .",

	"dépôt compris"			// Displays if you are hiring the merc with the medical deposit
};



//Aim Member.c
//The page in which the player hires AIM mercenaries

// The text that pops up when you select the TRANSFER FUNDS button

static const ST::string s_fr_AimPopUpText[AimPopUpText_SIZE] =
{
	"TRANSFERT ACCEPTE",	// You hired the merc
	"TRANSFERT REFUSE",		// Player doesn't have enough money, message 1
	"FONDS INSUFFISANTS",				// Player doesn't have enough money, message 2

	// if the merc is not available, one of the following is displayed over the merc's face

	"En mission",
	"Veuillez laisser un message",
	"Décédé",

	//If you try to hire more mercs than game can support

	"Votre équipe contient déjà 18 mercenaires.",

	"Message pré-enregistré",
	"Message enregistré",
};


//AIM Link.c

static const ST::string s_fr_AimLinkText = "Liens A.I.M."; // The title of the AIM links page



//Aim History

// This page displays the history of AIM

static const ST::string s_fr_AimHistoryText[AimHistoryText_SIZE] =
{
	"Historique A.I.M.",					//Title

	// Text on the buttons at the bottom of the page

	"Précédent",
	"Home",
	"Anciens",
	"Suivant",
};


//Aim Mug Shot Index

//The page in which all the AIM members' portraits are displayed in the order selected by the AIM sort page.

static const ST::string s_fr_AimFiText[AimFiText_SIZE] =
{
	// displays the way in which the mercs were sorted

	"Prix",
	"Expérience",
	"Tir",
	"Médecine",
	"Explosifs",
	"Technique",

	// The title of the page, the above text gets added at the end of this text

	"Tri ascendant des membres de l'A.I.M. par %s",
	"Tri descendant des membres de l'A.I.M. par %s",

	// Instructions to the players on what to do

	"Cliquez pour",
	"sélectionner le mercenaire",			//10
	"Clic droit pour",
	"les options de tri",

	// Gets displayed on top of the merc's portrait if they are...

	"Décédé",						//14
	"En mission",
};



//AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are non longer with AIM

static const ST::string s_fr_AimAlumniText[AimAlumniText_SIZE] =
{
	// Text of the buttons

	"PAGE 1",
	"PAGE 2",
	"PAGE 3",

	"Anciens",	// Title of the page

	"OK"			// Stops displaying information on selected merc
};






//AIM Home Page

static const ST::string s_fr_AimScreenText[AimScreenText_SIZE] =
{
	// AIM disclaimers

	"A.I.M. et le logo A.I.M. sont des marques déposées dans la plupart des pays.",
	"N'espérez même pas nous copier !",
	"Copyright 1998-1999 A.I.M., Ltd. Tous droits réservés.",

	//Text for an advertisement that gets displayed on the AIM page

	"Service des Fleuristes Associés",
	"\"Nous livrons partout dans le monde\"",				//10
	"Faites-le dans les règles de l'art",
	"... la première fois",
	"Si nous ne l'avons pas, c'est que vous n'en avez pas besoin.",
};


//Aim Home Page

static const ST::string s_fr_AimBottomMenuText[AimBottomMenuText_SIZE] =
{
	//Text for the links at the bottom of all AIM pages
	"Home",
	"Membres",
	"Anciens",
	"Règlement",
	"Historique",
	"Liens",
};



//ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.

static const ST::string s_fr_SKI_Text[SKI_SIZE ] =
{
	"MARCHANDISE EN STOCK",		//Header for the merchandise available
	"PAGE",				//The current store inventory page being displayed
	"COUT TOTAL",				//The total cost of the the items in the Dealer inventory area
	"VALEUR TOTALE",			//The total value of items player wishes to sell
	"EVALUATION",				//Button text for dealer to evaluate items the player wants to sell
	"TRANSACTION",			//Button text which completes the deal. Makes the transaction.
	"OK",				//Text for the button which will leave the shopkeeper interface.
	"COUT REPARATION",			//The amount the dealer will charge to repair the merc's goods
	"1 HEURE",			// SINGULAR! The text underneath the inventory slot when an item is given to the dealer to be repaired
	"%d HEURES",		// PLURAL!   The text underneath the inventory slot when an item is given to the dealer to be repaired
	"REPARE",		// Text appearing over an item that has just been repaired by a NPC repairman dealer
	"Plus d'emplacements libres.",	//Message box that tells the user there is non more room to put there stuff
	"%d MINUTES",		// The text underneath the inventory slot when an item is given to the dealer to be repaired
	"Objet lâché à terre.",
};


static const ST::string s_fr_SkiMessageBoxText[SkiMessageBoxText_SIZE] =
{
	"Voulez-vous déduire %s de votre compte pour combler la différence ?",
	"Pas assez d'argent. Il vous manque %s",
	"Voulez-vous déduire %s de votre compte pour couvrir le coût ?",
	"Demander au vendeur de lancer la transaction",
	"Demander au vendeur de réparer les objets sélectionnés",
	"Terminer l'entretien",
	"Solde actuel",
};


//OptionScreen.c

static const ST::string s_fr_zOptionsText[zOptionsText_SIZE] =
{
	//button Text
	"Enregistrer",
	"Charger partie",
	"Quitter",
	"OK",

	//Text above the slider bars
	"Effets",
	"Dialogue",
	"Musique",

	//Confirmation pop when the user selects..
	"Quitter la partie et revenir au menu principal ?",

	"Activez le mode Dialogue ou Sous-titre.",
};


//SaveLoadScreen
static const ST::string s_fr_zSaveLoadText[zSaveLoadText_SIZE] =
{
	"Enregistrer",
	"Charger partie",
	"Annuler",
	"Enregistrement",
	"Chargement",

	"Enregistrement réussi",
	"ERREUR lors de la sauvegarde !",
	"Chargement réussi",
	"ERREUR lors du chargement: \"%s\"",

	"La version de la sauvegarde est différente de celle du jeu. Désirez-vous continuer ?",
	"Les fichiers de sauvegarde sont peut-être altérés. Voulez-vous les effacer ?",

	"Tentative de chargement d'une sauvegarde de version précédente. Voulez-vous effectuer une mise à jour ?",

	"Tentative de chargement d'une sauvegarde de version précédente. Voulez-vous effectuer une mise à jour ?",

	"Etes-vous sûr de vouloir écraser la sauvegarde #%d ?",


	//The first %d is a number that contains the amount of free space on the users hard drive,
	//the second is the recommended amount of free space.
	"Votre risquez de manquer d'espace disque. Il ne vous reste que %d Mo de libre alors que le jeu nécessite %d Mo d'espace libre.",

	"Enregistrement...",			//When saving a game, a message box with this string appears on the screen

	"Peu d'armes",
	"Beaucoup d'armes",
	"Style réaliste",
	"Style SF",

	"Difficulté",
};



//MapScreen
static const ST::string s_fr_zMarksMapScreenText[zMarksMapScreenText_SIZE] =
{
	"Niveau carte",
	"Vous n'avez pas de milice : vous devez entraîner les habitants de la ville.",
	"Revenu quotidien",
	"Assurance vie",
	"%s n'est pas fatigué.",
	"%s est en mouvement et ne peut dormir.",
	"%s est trop fatigué pour obéir.",
	"%s conduit.",
	"L'escouade ne peut progresser si l'un de ses membres se repose.",

	// stuff for contracts
	"Vous pouvez payer les honoraires de ce mercenaire, mais vous ne pouvez pas vous offrir son assurance.",
	"La prime d'assurance de %s coûte %s pour %d jour(s) supplémentaire(s). Voulez-vous les payer ?",
	"Inventaire du Secteur",
	"Le mercenaire a un dépôt médical.",

	// other items
	"Medics", // people acting a field medics and bandaging wounded mercs
	"Patients", // people who are being bandaged by a medic
	"OK", // Continue on with the game after autobandage is complete
	"Stop", // Stop autobandaging of patients by medics now
	"%s n'a pas de trousse à outil.",
	"%s n'a pas de trousse de soins.",
	"Il y a trop peu de volontaires pour l'entraînement.",
	"%s ne peut pas former plus de miliciens.",
	"Le mercenaire a un contrat déterminé.",
	"Ce mercenaire n'est pas assuré.",
};


static const ST::string s_fr_pLandMarkInSectorString = "L'escouade %d a remarqué quelque chose dans le secteur %s";

// confirm the player wants to pay X dollars to build a militia force in town
static const ST::string s_fr_pMilitiaConfirmStrings[pMilitiaConfirmStrings_SIZE] =
{
	"L'entraînement de la milice vous coûtera $", // telling player how much it will cost
	"Etes-vous d'accord ?", // asking player if they wish to pay the amount requested
	"Vous n'en avez pas les moyens.", // telling the player they can't afford to train this town
	"Voulez-vous poursuivre l'entraînement de la milice à %s (%s %d) ?", // continue training this town?
	"Coût $", // the cost in dollars to train militia
	"(O/N)",   // abbreviated oui/non
	"L'entraînement des milices dans %d secteurs vous coûtera %d $. %s", // cost to train sveral sectors at once
	"Vous ne pouvez pas payer les %d $ nécessaires à l'entraînement.",
	"Vous ne pouvez poursuivre l'entraînement de la milice à %s que si cette ville est à niveau de loyauté de %d pour-cent.",
	"Vous ne pouvez plus entraîner de milice à %s.",
};


//Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom of the single merc panel
static const ST::string s_fr_gzMoneyWithdrawMessageText[gzMoneyWithdrawMessageText_SIZE] =
{
	"Vous ne pouvez retirer que 20 000 $ à la fois.",
	"Etes-vous sûr de vouloir déposer %s sur votre compte ?",
};

static const ST::string s_fr_gzCopyrightText = "Copyright (C) 1999 Sir-tech Canada Ltd. Tous droits réservés.";

//option Text
static const ST::string s_fr_zOptionsToggleText[zOptionsToggleText_SIZE] =
{
	"Dialogue",
	"Confirmations muettes",
	"Sous-titres",
	"Pause des dialogues",
	"Animation fumée",
	"Du sang et des tripes",
	"Ne pas toucher à ma souris !",
	"Ancienne méthode de sélection",
	"Afficher chemin",
	"Afficher tirs manqués",
	"Confirmation temps réel",
	"Afficher notifications sommeil/réveil",
	"Système métrique",
	"Mercenaire éclairé lors des mouvements",
	"Figer curseur sur les mercenaires",
	"Figer curseur sur les portes",
	"Objets en surbrillance",
	"Afficher cimes",
	"Affichage fil de fer",
	"Curseur 3D",
};

//This is the help text associated with the above toggles.
static const ST::string s_fr_zOptionsScreenHelpText[zOptionsToggleText_SIZE] =
{
	//speech
	"Activez cette option pour entendre vos mercenaires lorsqu'ils parlent.",

	//Mute Confirmation
	"Active/désactive les confirmations des mercenaires.",

		//Subtitles
	"Affichage des sous-titres à l'écran.",

	//Key to advance speech
	"Si les sous-titres s'affichent à l'écran, cette option vous permet de prendre le temps de les lire.",

	//Toggle smoke animation
	"Désactivez cette option si votre machine n'est pas suffisamment puissante.",

	//Blood n Gore
	"Désactivez cette option si le jeu vous paraît trop violent.",

	//Never move my mouse
	"Activez cette option pour que le curseur ne se place pas automatiquement sur les boutons qui s'affichent.",

	//Old selection method
	"Activez cette option pour retrouver vos automatismes de la version précédente.",

	//Show movement path
	"Activez cette option pour afficher le chemin suivi par les mercenaires. Vous pouvez la désactiver et utiliser la touche MAJ en cours de jeu.",

	//show misses
	"Activez cette option pour voir où atterrissent tous vos tirs.",

	//Real Time Confirmation
	"Activez cette option pour afficher une confirmation de mouvement en temps réel.",

	//Sleep/Wake notification
	"Activez cette option pour être mis au courant de l'état de veille de vos mercenaires.",

	//Use the metric system
	"Activez cette option pour que le jeu utilise le système métrique.",

	//Merc Lighted movement
	"Activez cette option pour éclairer les environs des mercenaires. Désactivez-le si votre machine n'est pas suffisamment puissante.",

	//Smart cursor
	"Activez cette option pour que le curseur se positionne directement sur un mercenaire quand il est à proximité.",

	//snap cursor to the door
	"Activez cette option pour que le curseur se positionne directement sur une porte quand il est à proximité.",

	//glow items
	"Activez cette option pour mettre les objets en évidence (|I)",

	//toggle tree tops
	"Activez cette option pour afficher le cime des arbres. (|T)",

	//toggle wireframe
	"Activez cette option pour afficher les murs en fil de fer. (|W)",

	"Activez cette option pour afficher le curseur en 3D. (|H|o|m|e)",

};


static const ST::string s_fr_gzGIOScreenText[gzGIOScreenText_SIZE] =
{
	"CONFIGURATION DU JEU",
	"Style de jeu",
	"Réaliste",
	"SF",
	"Armes",
	"Beaucoup",
	"Peu",
	"Difficulté",
	"Novice",
	"Expérimenté",
	"Expert",
	"Ok",
	"Annuler",
	"En combat",
	"Temps illimité",
	"Temps limité",
	"Mort est mort"
};

// This string is used in the IMP character generation.  It is possible to
// select 0 ability in a skill meaning you can't use it.  This text is
// confirmation to the player.
static const ST::string s_fr_pSkillAtZeroWarning    = "Etes-vous sûr de vous ? Une valeur de ZERO signifie que vous serez INCAPABLE d'utiliser cette compétence.";
static const ST::string s_fr_pIMPBeginScreenStrings = "( 8 Caractères Max )";
static const ST::string s_fr_pIMPFinishButtonText   = "Analyse";
static const ST::string s_fr_pIMPFinishStrings      = "Nous vous remercions, %s"; //%s is the name of the merc
static const ST::string s_fr_pIMPVoicesStrings      = "Voix"; // the strings for imp voices screen

// title for program
static const ST::string s_fr_pPersTitleText = "Personnel";

// paused game strings
static const ST::string s_fr_pPausedGameText[pPausedGameText_SIZE] =
{
	"Pause",
	"Reprendre (|P|a|u|s|e)",
	"Pause (|P|a|u|s|e)",
};


static const ST::string s_fr_pMessageStrings[pMessageStrings_SIZE] =
{
	"Quitter la partie ?",
	"OK",
	"OUI",
	"NON",
	"Annuler",
	"CONTRAT",
	"MENT",
	"Sans description", //Save slots that don't have a description.
	"Partie sauvegardée.",
	"Jour",
	"Mercs",
	"Vide", //An empty save game slot
	"bpm",					//Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
	"min",					//Abbreviation for minute.
	"m",						//One character abbreviation for meter (metric distance measurement unit).
	"balles",				//Abbreviation for rounds (# of bullets)
	"kg",					//Abbreviation for kilogram (metric weight measurement unit)
	"lb",					//Abbreviation for pounds (Imperial weight measurement unit)
	"Home",				//Home as in homepage on the internet.
	"USD",					//Abbreviation to US dollars
	"n/a",					//Lowercase acronym for not applicable.
	"Entre-temps",		//Meanwhile
	"%s est arrivé dans le secteur %s%s", //Name/Squad has arrived in sector A9.  Order must not change without notifying
																		//SirTech
	"Version",
	"Emplacement de sauvegarde rapide vide",
	"Cet emplacement est réservé aux sauvegardes rapides effectuées depuis l'écran tactique (ALT+S).",
	"Ouverte",
	"Fermée",
	"Espace disque insuffisant. Il ne vous reste que %s Mo de libre et Jagged Alliance 2 nécessite %s Mo.",
	"%s prend %s.",		//'Merc name' has caught 'item' -- let SirTech know if name comes after item.
	"%s a pris la drogue.", //'Merc name' has taken the drug
	"%s n'a aucune compétence médicale.",//'Merc name' has non medical skill.

	//CDRom errors (such as ejecting CD while attempting to read the CD)
	"L'intégrité du jeu n'est plus assurée.",
	"ERREUR : CD-ROM manquant",

	//When firing heavier weapons in close quarters, you may not have enough room to do so.
	"Pas assez de place !",

	//Can't change stance due to objects in the way...
	"Impossible de changer de position ici.",

	//Simple text indications that appear in the game, when the merc can do one of these things.
	"Lâcher",
	"Lancer",
	"Donner",

	"%s donné à %s.", 	//"Item" passed to "merc".  Please try to keep the item %s before the merc %s,
				//otherwise, must notify SirTech.
	"Impossible de donner %s à %s.", //pass "item" to "merc".  Same instructions as above.

	//A list of attachments appear after the items.  Ex:  Kevlar vest ( Ceramic Plate 'Attached )'
	" combiné)",

	//Cheat modes
	"Triche niveau 1",
	"Triche niveau 2",

	//Toggling various stealth modes
	"Escouade en mode furtif.",
	"Escouade en mode normal.",
	"%s en mode furtif.",
	"%s en mode normal.",

	//Wireframes are shown through buildings to reveal doors and windows that can't otherwise be seen in
	//an isometric engine.  You can toggle this mode freely in the game.
	"Fil de fer activé",
	"Fil de fer désactivé",

	//These are used in the cheat modes for changing levels in the game.  Going from a basement level to
	//an upper level, etc.
	"Impossible de remonter...",
	"Pas de niveau inférieur...",
	"Entrée dans le sous-sol %d...",
	"Sortie du sous-sol...",

	"'s",		// used in the shop keeper inteface to mark the ownership of the item eg Red's gun
	"Mode poursuite désactivé.",
	"Mode poursuite activé.",
	"Curseur 3D désactivé.",
	"Curseur 3D activé.",
	"Escouade %d active.",
	"Vous ne pouvez pas payer le salaire de %s qui se monte à %s",	//first %s is the mercs name, the seconds is a string containing the salary
	"Passer",
	"%s ne peut sortir seul.",
	"Une sauvegarde a été crée (Partie99.sav). Renommez-la (Partie01 - Partie10) pour pouvoir la charger ultérieurement.",
	"%s a bu %s",
	"Un colis vient d'arriver à Drassen.",
	"%s devrait arriver au point d'entrée (secteur %s) en jour %d vers %s.",		//first %s is mercs name, next is the sector location and name where they will be arriving in, lastely is the day an the time of arrival
	"Historique mis à jour.",
};


static const ST::string s_fr_ItemPickupHelpPopup[ItemPickupHelpPopup_SIZE] =
{
	"OK",
	"Défilement haut",
	"Tout sélectionner",
	"Défilement bas",
	"Annuler",
};

static const ST::string s_fr_pDoctorWarningString[pDoctorWarningString_SIZE] =
{
	"%s est trop loin pour être soigné.",
	"Impossible de soigner tout le monde.",
};

static const ST::string s_fr_pMilitiaButtonsHelpText[pMilitiaButtonsHelpText_SIZE] =
{
	"Prendre (Clic droit)/poser (Clic gauche) Miliciens", // button help text informing player they can pick up or drop militia with this button
	"Prendre (Clic droit)/poser (Clic gauche) Soldats",
	"Prendre (Clic droit)/poser (Clic gauche) Vétérans",
	"Répartition automatique",
};

// to inform the player to hire some mercs to get things going
static const ST::string s_fr_pMapScreenJustStartedHelpText = "Allez sur le site de l'AIM et engagez des mercenaires ( *Truc* allez voir dans le Poste de travail)";

static const ST::string s_fr_pAntiHackerString = "Erreur. Fichier manquant ou corrompu. L'application va s'arrêter.";


static const ST::string s_fr_gzLaptopHelpText[gzLaptopHelpText_SIZE] =
{
	//Buttons:
	"Voir messages",
	"Consulter les sites Internet",
	"Consulter les documents attachés",
	"Lire le compte-rendu",
	"Afficher les infos de l'équipe",
	"Afficher les états financiers",
	"Fermer le Poste de travail",

	//Bottom task bar icons (if they exist):
	"Vous avez de nouveaux messages",
	"Vous avez reçu de nouveaux fichiers",

	//Bookmarks:
	"Association Internationale des Mercenaires",
	"Bobby Ray : Petits et Gros Calibres",
	"Institut des Mercenaires Professionnels",
	"Mouvement pour l'Entraînement et le Recrutement des Commandos",
	"Morgue McGillicutty",
	"Service des Fleuristes Associés",
	"Courtiers d'Assurance des Mercenaires de l'A.I.M.",
};


static const ST::string s_fr_gzHelpScreenText = "Quitter l'écran d'aide";

static const ST::string s_fr_gzNonPersistantPBIText[gzNonPersistantPBIText_SIZE] =
{
	"Vous êtes en plein combat. Vous pouvez donner l'ordre de retraite depuis l'écran tactique.",
	"Pénétrez dans le secteur pour reprendre le cours du combat. (|E)",
	"Résolution |automatique du combat.",
	"Résolution automatique impossible lorsque vous êtes l'attaquant.",
	"Résolution automatique impossible lorsque vous êtes pris en embuscade.",
	"Résolution automatique impossible lorsque vous combattez des créatures dans les mines.",
	"Résolution automatique impossible en présence de civils hostiles.",
	"Résolution automatique impossible en présence de chats sauvages.",
	"COMBAT EN COURS",
	"Retraite impossible.",
};

static const ST::string s_fr_gzMiscString[gzMiscString_SIZE] =
{
	"Votre milice continue le combat sans vos mercenaires...",
	"Ce véhicule n'a plus besoin de carburant pour le moment.",
	"Le réservoir est plein à %d%%.",
	"L'armée de Deidranna a repris le contrôle de %s.",
	"Vous avez perdu un site de ravitaillement.",
};

static const ST::string s_fr_gzIntroScreen = "Vidéo d'introduction introuvable";

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
static const ST::string s_fr_pNewNoiseStr[pNewNoiseStr_SIZE] =
{
	"%s entend un bruit de %s %s.",
	"%s entend un bruit %s de MOUVEMENT %s.",
	"%s entend un GRINCEMENT %s %s.",
	"%s entend un CLAPOTIS %s %s.",
	"%s entend un IMPACT %s %s.",
	"%s entend une EXPLOSION %s %s.",
	"%s entend un CRI %s %s.",
	"%s entend un IMPACT %s %s.",
	"%s entend un IMPACT %s %s.",
	"%s entend un BRUIT %s %s.",
	"%s entend un BRUIT %s %s.",
};

static const ST::string s_fr_wMapScreenSortButtonHelpText[wMapScreenSortButtonHelpText_SIZE] =
{
	"Tri par nom (|F|1)",
	"Tri par affectation (|F|2)",
	"Tri par état de veille (|F|3)",
	"Tri par lieu (|F|4)",
	"Tri par destination (|F|5)",
	"Tri par date de départ (|F|6)",
};



static const ST::string s_fr_BrokenLinkText[BrokenLinkText_SIZE] =
{
	"Erreur 404",
	"Site introuvable.",
};


static const ST::string s_fr_gzBobbyRShipmentText[gzBobbyRShipmentText_SIZE] =
{
	"Derniers envois",
	"Commande #",
	"Quantité d'objets",
	"Commandé",
};


static const ST::string s_fr_gzCreditNames[gzCreditNames_SIZE]=
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


static const ST::string s_fr_gzCreditNameTitle[gzCreditNameTitle_SIZE]=
{
	"Programmeur", 			// Chris Camfield
	"Co-designer/Ecrivain",							// Shaun Lyng
	"Systèmes stratégiques & Programmeur",					//Kris Marnes
	"Producteur/Co-designer",						// Ian Currie
	"Co-designer/Conception des cartes",				// Linda Currie
	"Artiste",													// Eric \"WTF\" Cheng
	"Coordination, Assistance",				// Lynn Holowka
	"Artiste Extraordinaire",						// Norman \"NRG\" Olsen
	"Gourou du son",											// George Brooks
	"Conception écrans/Artiste",					// Andrew Stacey
	"Artiste en chef/Animateur",						// Scot Loving
	"Programmeur en chef",									// Andrew \"Big Cheese Doddle\" Emmons
	"Programmeur",											// Dave French
	"Systèmes stratégiques & Programmeur",					// Alex Meduna
	"Portraits",								// Joey \"Joeker\" Whelan",
};

static const ST::string s_fr_gzCreditNameFunny[gzCreditNameFunny_SIZE]=
{
	"", 																			// Chris Camfield
	"(ah, la ponctuation...)",					// Shaun Lyng
	"(\"C'est bon, trois fois rien\")",	//Kris \"The Cow Rape Man\" Marnes
	"(j'ai passé l'âge)",				// Ian Currie
	"(et en plus je bosse sur Wizardry 8)",						// Linda Currie
	"(on m'a forcé !)",			// Eric \"WTF\" Cheng
	"(partie en cours de route...)",	// Lynn Holowka
	"",																			// Norman \"NRG\" Olsen
	"",																			// George Brooks
	"(Tête de mort et fou de jazz)",						// Andrew Stacey
	"(en fait il s'appelle Robert)",							// Scot Loving
	"(la seule personne un peu responsable de l'équipe)",					// Andrew \"Big Cheese Doddle\" Emmons
	"(bon, je vais pouvoir réparer ma moto)",	// Dave French
	"(piqué à l'équipe de Wizardry 8)",							// Alex Meduna
	"(conception des objets et des écrans de chargement !)",	// Joey \"Joeker\" Whelan",
};

static const ST::string s_fr_sRepairsDoneString[sRepairsDoneString_SIZE] =
{
	"%s a terminé la réparation de ses objets",
	"%s a terminé la réparation des armes & armures",
	"%s a terminé la réparation des objets portés",
	"%s a terminé la réparation des objets transportés",
};

static const ST::string s_fr_zGioDifConfirmText[zGioDifConfirmText_SIZE]=
{
	"Vous avez choisi le mode de difficulté NOVICE. Ce mode de jeu est conseillé pour les joueurs qui découvrent Jagged Alliance, qui n'ont pas l'habitude de jouer à des jeux de stratégie ou qui souhaitent que les combats ne durent pas trop longtemps. Ce choix influe sur de nombreux paramètres du jeu. Etes-vous certain de vouloir jouer en mode Novice ?",
	"Vous avez choisi le mode de difficulté EXPERIMENTE. Ce mode de jeu est conseillé pour les joueurs qui ont déjà joué à Jagged Alliance ou des jeux de stratégie. Ce choix influe sur de nombreux paramètres du jeu. Etes-vous certain de vouloir jouer en mode Expérimenté ?",
	"Vous avez choisi le mode de difficulté EXPERT. Vous aurez été prévenu. Ne venez pas vous plaindre si vos mercenaires quittent Arulco dans un cerceuil. Ce choix influe sur de nombreux paramètres du jeu. Etes-vous certain de vouloir jouer en mode Expert ?",
};

static const ST::string s_fr_gzLateLocalizedString[gzLateLocalizedString_SIZE] =
{
	//1-5
	"Le robot ne peut quitter ce secteur par lui-même.",

	//This message comes up if you have pending bombs waiting to explode in tactical.
	"Compression du temps impossible. C'est bientôt le feu d'artifice !",

	//'Name' refuses to move.
	"%s refuse d'avancer.",

	//%s a merc name
	"%s n'a pas assez d'énergie pour changer de position.",

	//A message that pops up when a vehicle runs out of gas.
	"Le %s n'a plus de carburant ; le véhicule est bloqué à %c%d.",

	//6-10

	// the following two strings are combined with the pNewNoise[] strings above to report noises
	// heard above or below the merc
	"au-dessus",
	"en-dessous",

	//The following strings are used in autoresolve for autobandaging related feedback.
	"Aucun de vos mercenaires n'a de compétence médicale.",
	"Plus de bandages !",
	"Pas assez de bandages pour soigner tout le monde.",
	"Aucun de vos mercenaires n'a besoin de soins.",
	"Soins automatiques.",
	"Tous vos mercenaires ont été soignés.",

	//14
	"Arulco",

	"(roof)",

	"Santé : %d/%d",

	//In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
	//"vs." is the abbreviation of versus.
	"%d contre %d",

	"Plus de place dans le %s !",  //(ex "The ice cream truck is full")

	"%s requiert des soins bien plus importants et/ou du repos.",

	//20
	//Happens when you get shot in the legs, and you fall down.
	"%s a été touché aux jambes ! Il ne peut plus tenir debout !",
	//Name can't speak right now.
	"%s ne peut pas parler pour le moment.",

	//22-24 plural versions
	"%d miliciens ont été promus vétérans.",
	"%d miliciens ont été promus soldats.",
	"%d soldats ont été promus vétérans.",

	//25
	"Echanger",

	//26
	//Name has gone psycho -- when the game forces the player into burstmode (certain unstable characters)
	"%s est devenu fou !",

	//27-28
	//Messages why a player can't time compress.
	"Nous vous déconseillons d'utiliser la Compression du temps ; vous avez des mercenaires dans le secteur %s.",
	"Nous vous déconseillons d'utiliser la Compression du temps lorsque vos mercenaires se trouvent dans des mines infestées de créatures.",

	//29-31 singular versions
	"1 milicien a été promu vétéran.",
	"1 milicien a été promu soldat.",
	"1 soldat a été promu vétéran.",

	//32-34
	"%s ne dit rien.",
	"Revenir à la surface ?",
	"(Escouade %d)",

	//35
	//Ex: "Red has repaired Scope's MP5K".  Careful to maintain the proper order (Red before Scope, Scope before MP5K)
	"%s a réparé pour %s : %s",//inverted order !!! Red has repaired the MP5 of Scope

	//36
	"Chat Sauvage",

	//37-38 "Name trips and falls"
	"%s trébuche et tombe",
	"Cet objet ne peut être pris d'ici.",

	//39
	"Il ne vous reste aucun mercenaire en état de se battre. La milice combattra les créatures seule.",

	//40-43
	//%s is the name of merc.
	"%s n'a plus de trousse de soins !",
	"%s n'a aucune compétence médicale !",
	"%s n'a plus de trousse à outils !",
	"%s n'a aucune compétence technique !",

	//44-45
	"Temps de réparation",
	"%s ne peut pas voir cette personne.",

	//46-48
	"Le prolongateur de %s est tombé !",
	"Seuls %d instructeurs de milice peuvent travailler par secteur.",
	"Etes-vous sûr ?",

	//49-50
	"Compression du temps",
	"Le réservoir est plein.",

	//51-52 Fast help text in mapscreen.
	"Compression du temps (|E|s|p|a|c|e)",
	"Arrêt de la Compression du temps (|E|c|h|a|p)",

	//53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
	"%s a désenrayé le %s",
	"%s a désenrayé le %s de %s",//inverted !!! magic has unjammed the g11 of raven

	//55
	"Compression du temps impossible dans l'écran d'inventaire.",

	//56
	//Displayed with the version information when cheats are enabled.
	"Actuel/Maximum : %d%%/%d%%",

	//57
	"Escorter John et Mary ?",

	"Interrupteur activé.",
};

static const ST::string s_fr_str_ceramic_plates_smashed = "%s's ceramic plates have been smashed!"; // TODO translate

static const ST::string s_fr_str_arrival_rerouted = "Arrival of new recruits is being rerouted to sector %s, as scheduled drop-off point of sector %s is enemy occupied."; // TODO translate


static const ST::string s_fr_str_stat_health       = "Santé";
static const ST::string s_fr_str_stat_agility      = "Agilité";
static const ST::string s_fr_str_stat_dexterity    = "Dextérité";
static const ST::string s_fr_str_stat_strength     = "Force";
static const ST::string s_fr_str_stat_leadership   = "Commandement";
static const ST::string s_fr_str_stat_wisdom       = "Sagesse";
static const ST::string s_fr_str_stat_exp_level    = "Niveau"; // Niveau expérience
static const ST::string s_fr_str_stat_marksmanship = "Tir";
static const ST::string s_fr_str_stat_mechanical   = "Technique";
static const ST::string s_fr_str_stat_explosive    = "Explosifs";
static const ST::string s_fr_str_stat_medical      = "Médecine";

static const ST::string s_fr_str_stat_list[str_stat_list_SIZE] =
{
	s_fr_str_stat_health,
	s_fr_str_stat_agility,
	s_fr_str_stat_dexterity,
	s_fr_str_stat_strength,
	s_fr_str_stat_leadership,
	s_fr_str_stat_wisdom,
	s_fr_str_stat_exp_level,
	s_fr_str_stat_marksmanship,
	s_fr_str_stat_mechanical,
	s_fr_str_stat_explosive,
	s_fr_str_stat_medical
};

static const ST::string s_fr_str_aim_sort_list[str_aim_sort_list_SIZE] =
{
	s_fr_str_aim_sort_price,
	s_fr_str_aim_sort_experience,
	s_fr_str_aim_sort_marksmanship,
	s_fr_str_aim_sort_medical,
	s_fr_str_aim_sort_explosives,
	s_fr_str_aim_sort_mechanical,
	s_fr_str_aim_sort_ascending,
	s_fr_str_aim_sort_descending,
};

static const ST::string s_fr_gs_dead_is_dead_mode_tab_name[gs_dead_is_dead_mode_tab_name_SIZE] =
{
	"Normal", 			// Normal Tab
	"DiD", 			// Dead is Dead Tab
};

// French language resources.
LanguageRes g_LanguageResFrench = {

	s_fr_WeaponType,

	s_fr_Message,
	s_fr_TeamTurnString,
	s_fr_pAssignMenuStrings,
	s_fr_pTrainingStrings,
	s_fr_pTrainingMenuStrings,
	s_fr_pAttributeMenuStrings,
	s_fr_pVehicleStrings,
	s_fr_pShortAttributeStrings,
	s_fr_pContractStrings,
	s_fr_pAssignmentStrings,
	s_fr_pConditionStrings,
	s_fr_pPersonnelScreenStrings,
	s_fr_pUpperLeftMapScreenStrings,
	s_fr_pTacticalPopupButtonStrings,
	s_fr_pSquadMenuStrings,
	s_fr_pDoorTrapStrings,
	s_fr_pLongAssignmentStrings,
	s_fr_pMapScreenMouseRegionHelpText,
	s_fr_pNoiseVolStr,
	s_fr_pNoiseTypeStr,
	s_fr_pDirectionStr,
	s_fr_pRemoveMercStrings,
	s_fr_sTimeStrings,
	s_fr_pLandTypeStrings,
	s_fr_pInvPanelTitleStrings,
	s_fr_pPOWStrings,
	s_fr_pMilitiaString,
	s_fr_pMilitiaButtonString,
	s_fr_pEpcMenuStrings,
	s_fr_pRepairStrings,
	s_fr_sPreStatBuildString,
	s_fr_sStatGainStrings,
	s_fr_pHelicopterEtaStrings,
	s_fr_sMapLevelString,
	s_fr_gsLoyalString,
	s_fr_gsUndergroundString,
	s_fr_gsTimeStrings,
	s_fr_sFacilitiesStrings,
	s_fr_pMapPopUpInventoryText,
	s_fr_pwTownInfoStrings,
	s_fr_pwMineStrings,
	s_fr_pwMiscSectorStrings,
	s_fr_pMapInventoryErrorString,
	s_fr_pMapInventoryStrings,
	s_fr_pMovementMenuStrings,
	s_fr_pUpdateMercStrings,
	s_fr_pMapScreenBorderButtonHelpText,
	s_fr_pMapScreenBottomFastHelp,
	s_fr_pMapScreenBottomText,
	s_fr_pMercDeadString,
	s_fr_pSenderNameList,
	s_fr_pNewMailStrings,
	s_fr_pDeleteMailStrings,
	s_fr_pEmailHeaders,
	s_fr_pEmailTitleText,
	s_fr_pFinanceTitle,
	s_fr_pFinanceSummary,
	s_fr_pFinanceHeaders,
	s_fr_pTransactionText,
	s_fr_pMoralStrings,
	s_fr_pSkyriderText,
	s_fr_str_left_equipment,
	s_fr_pMapScreenStatusStrings,
	s_fr_pMapScreenPrevNextCharButtonHelpText,
	s_fr_pEtaString,
	s_fr_pShortVehicleStrings,
	s_fr_pTrashItemText,
	s_fr_pMapErrorString,
	s_fr_pMapPlotStrings,
	s_fr_pBullseyeStrings,
	s_fr_pMiscMapScreenMouseRegionHelpText,
	s_fr_str_he_leaves_where_drop_equipment,
	s_fr_str_she_leaves_where_drop_equipment,
	s_fr_str_he_leaves_drops_equipment,
	s_fr_str_she_leaves_drops_equipment,
	s_fr_pImpPopUpStrings,
	s_fr_pImpButtonText,
	s_fr_pExtraIMPStrings,
	s_fr_pFilesTitle,
	s_fr_pFilesSenderList,
	s_fr_pHistoryLocations,
	s_fr_pHistoryStrings,
	s_fr_pHistoryHeaders,
	s_fr_pHistoryTitle,
	s_fr_pShowBookmarkString,
	s_fr_pWebPagesTitles,
	s_fr_pWebTitle,
	s_fr_pPersonnelString,
	s_fr_pErrorStrings,
	s_fr_pDownloadString,
	s_fr_pBookMarkStrings,
	s_fr_pLaptopIcons,
	s_fr_gsAtmStartButtonText,
	s_fr_pPersonnelTeamStatsStrings,
	s_fr_pPersonnelCurrentTeamStatsStrings,
	s_fr_pPersonelTeamStrings,
	s_fr_pPersonnelDepartedStateStrings,
	s_fr_pMapHortIndex,
	s_fr_pMapVertIndex,
	s_fr_pMapDepthIndex,
	s_fr_pLaptopTitles,
	s_fr_pDayStrings,
	s_fr_pMilitiaConfirmStrings,
	s_fr_pSkillAtZeroWarning,
	s_fr_pIMPBeginScreenStrings,
	s_fr_pIMPFinishButtonText,
	s_fr_pIMPFinishStrings,
	s_fr_pIMPVoicesStrings,
	s_fr_pPersTitleText,
	s_fr_pPausedGameText,
	s_fr_zOptionsToggleText,
	s_fr_zOptionsScreenHelpText,
	s_fr_pDoctorWarningString,
	s_fr_pMilitiaButtonsHelpText,
	s_fr_pMapScreenJustStartedHelpText,
	s_fr_pLandMarkInSectorString,
	s_fr_gzMercSkillText,
	s_fr_gzNonPersistantPBIText,
	s_fr_gzMiscString,
	s_fr_wMapScreenSortButtonHelpText,
	s_fr_pNewNoiseStr,
	s_fr_gzLateLocalizedString,
	s_fr_pAntiHackerString,
	s_fr_pMessageStrings,
	s_fr_ItemPickupHelpPopup,
	s_fr_TacticalStr,
	s_fr_LargeTacticalStr,
	s_fr_zDialogActions,
	s_fr_zDealerStrings,
	s_fr_zTalkMenuStrings,
	s_fr_gzMoneyAmounts,
	s_fr_gzProsLabel,
	s_fr_gzConsLabel,
	s_fr_gMoneyStatsDesc,
	s_fr_gWeaponStatsDesc,
	s_fr_sKeyDescriptionStrings,
	s_fr_zHealthStr,
	s_fr_zVehicleName,
	s_fr_pExitingSectorHelpText,
	s_fr_InsContractText,
	s_fr_InsInfoText,
	s_fr_MercAccountText,
	s_fr_MercInfo,
	s_fr_MercNoAccountText,
	s_fr_MercHomePageText,
	s_fr_sFuneralString,
	s_fr_sFloristText,
	s_fr_sOrderFormText,
	s_fr_sFloristGalleryText,
	s_fr_sFloristCards,
	s_fr_BobbyROrderFormText,
	s_fr_BobbyRText,
	s_fr_str_bobbyr_guns_num_guns_that_use_ammo,
	s_fr_BobbyRaysFrontText,
	s_fr_AimSortText,
	s_fr_str_aim_sort_price,
	s_fr_str_aim_sort_experience,
	s_fr_str_aim_sort_marksmanship,
	s_fr_str_aim_sort_medical,
	s_fr_str_aim_sort_explosives,
	s_fr_str_aim_sort_mechanical,
	s_fr_str_aim_sort_ascending,
	s_fr_str_aim_sort_descending,
	s_fr_AimPolicyText,
	s_fr_AimMemberText,
	s_fr_CharacterInfo,
	s_fr_VideoConfercingText,
	s_fr_AimPopUpText,
	s_fr_AimLinkText,
	s_fr_AimHistoryText,
	s_fr_AimFiText,
	s_fr_AimAlumniText,
	s_fr_AimScreenText,
	s_fr_AimBottomMenuText,
	s_fr_zMarksMapScreenText,
	s_fr_gpStrategicString,
	s_fr_gpGameClockString,
	s_fr_SKI_Text,
	s_fr_SkiMessageBoxText,
	s_fr_zSaveLoadText,
	s_fr_zOptionsText,
	s_fr_gzGIOScreenText,
	s_fr_gzHelpScreenText,
	s_fr_gzLaptopHelpText,
	s_fr_gzMoneyWithdrawMessageText,
	s_fr_gzCopyrightText,
	s_fr_BrokenLinkText,
	s_fr_gzBobbyRShipmentText,
	s_fr_zGioDifConfirmText,
	s_fr_gzCreditNames,
	s_fr_gzCreditNameTitle,
	s_fr_gzCreditNameFunny,
	s_fr_pContractButtonString,
	s_fr_gzIntroScreen,
	s_fr_pUpdatePanelButtons,
	s_fr_sRepairsDoneString,
	s_fr_str_ceramic_plates_smashed,
	s_fr_str_arrival_rerouted,

	s_fr_str_stat_health,
	s_fr_str_stat_agility,
	s_fr_str_stat_dexterity,
	s_fr_str_stat_strength,
	s_fr_str_stat_leadership,
	s_fr_str_stat_wisdom,
	s_fr_str_stat_exp_level,
	s_fr_str_stat_marksmanship,
	s_fr_str_stat_mechanical,
	s_fr_str_stat_explosive,
	s_fr_str_stat_medical,

	s_fr_str_stat_list,
	s_fr_str_aim_sort_list,

	g_eng_zNewTacticalMessages,
	g_eng_str_iron_man_mode_warning,
	g_eng_str_dead_is_dead_mode_warning,
	g_eng_str_dead_is_dead_mode_enter_name,

	s_fr_gs_dead_is_dead_mode_tab_name,
};

#ifdef WITH_UNITTESTS
TEST(StringEncodingTest, FrenchTextFile)
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
