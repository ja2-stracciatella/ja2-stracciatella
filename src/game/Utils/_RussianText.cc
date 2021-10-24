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
//   symbols.  SirTech will search for !!! to look for Topware problems and questions.  This is a more
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
//         Clears all the mercs' positions,
//         and allows you to re-enter them manually.
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
//   As described above, the "!!!" notation should be used by Topware to ask questions and address problems as
//   SirTech uses the "@@@" notation.



static const ST::string s_rus_WeaponType[WeaponType_SIZE] =
{
	"Другое",
	"Пистолет",
	"Автом. пистолет",
	"Пистолет-пулемет",
	"Винтовка",
	"Снайп. винтовка",
	"Автомат",
	"Легкий пулемет",
	"Дробовик"
};

static const ST::string s_rus_TeamTurnString[TeamTurnString_SIZE] =
{
	"Ход игрока", // player's turn
	"Ход противника",
	"Ход существ",
	"Ход ополчения",
	"Ход жителей"
	// planning turn
};

static const ST::string s_rus_Message[Message_SIZE] =
{
	// In the following 8 strings, the %s is the merc's name, and the %d (if any) is a number.

	"%s получает ранение в голову и теряет в мудрости!",
	"%s получает ранение в плечо и теряет в ловкости!",
	"%s получает ранение в грудь и теряет в силе!",
	"%s получает ранение ног и теряет в проворности!",
	"%s получает ранение в голову и теряет %d очков мудрости!",
	"%s получает ранение в плечо и теряет %d очков ловкости!",
	"%s получает ранение в грудь и теряет %d очков силы!",
	"%s получает ранение ног и теряет %d очков проворности!",
	"Перехват инициативы!",

	"К вам на помощь прибыло подкрепление!",

	// In the following four lines, all %s's are merc names

	"%s перезаряжает оружие.",
	"%s: недостаточно очков действия!",
	// the following 17 strings are used to create lists of gun advantages and disadvantages
	// (separated by commas)
	"надежность",
	"ненадежность",
	"легко починить",
	"трудно починить",
	"сильный урон",
	"слабый урон",
	"скорострельность",
	"медленный огонь",
	"дальний бой",
	"ближний бой",
	"малый вес",
	"большой вес",
	"малые размеры",
	"авто-огонь",
	"только одиночными",
	"большой магазин",
	"маленький магазин",

	// In the following two lines, all %s's are merc names

	"%s: камуфляж изношен.",
	"%s: окраска камуфляжа смыта.",

	// The first %s is a merc name and the second %s is an item name

	"У второго оружия закончились патроны!",
	"%s крадет %s.",

	// The %s is a merc name

	"%s: оружие не стреляет очередями.",

	"Это уже присоединено!",
	"Соединить?",

	// Both %s's are item names

	"Нельзя присоединить %s к %s.",

	"Ничего",
	"Разрядить",
	"Приспособления",

	//You cannot use "item(s)" and your "other item" at the same time.
	//Ex:  You cannot use sun goggles and you gas mask at the same time.
	"Нельзя использовать %s и %s одновременно.",

	"Этот предмет можно присоединить к другим предметам, поместив его в одну из ячеек приспособлений.",
	"Этот предмет можно присоединить к другим предметам, поместив его в одну из ячеек приспособлений. (Однако данные предметы несовместимы.)",
	"В секторе еще остались враги!",
	"%s должен получить еще %s", ///TRNSL_ASSUME all doctors who can get money are male
	"%s: попадание в голову!",
	"Выйти из боя?",
	"Соединение будет неразборно. Соединить?",
	"%s чувствует прилив энергии!",
	"%s поскальзывается на шариках!",
	"%s не удается взять %s!",
	"%s чинит %s",
	"На перехвате ",
	"Сдаться?",
	"Человек отвергает вашу помощь.",
	"Ни за что!",//refuse to heal monster creature
	"Чтобы воспользоваться вертолетом Всадника, нужно дать бойцам ЗАДАНИЕ сесть в ТРАНСПОРТ.",
	"%s успевает зарядить только одно оружие",
	"Ход кошек-убийц",
};


// the types of time compression. For example: is the timer paused? at normal speed, 5 minutes per second, etc.
// min is an abbreviation for minutes

static const ST::string s_rus_sTimeStrings[sTimeStrings_SIZE] =
{
	"Пауза",
	"Норма",
	"5 мин",
	"30 мин",
	"60 мин",
	"6 час",
};


// Assignment Strings: what assignment does the merc  have right now? For example, are they on a squad, training,
// administering medical aid (doctor) or training a town. All are abbreviated. 8 letters is the longest it can be.

static const ST::string s_rus_pAssignmentStrings[pAssignmentStrings_SIZE] =
{
	"Отряд 1",
	"Отряд 2",
	"Отряд 3",
	"Отряд 4",
	"Отряд 5",
	"Отряд 6",
	"Отряд 7",
	"Отряд 8",
	"Отряд 9",
	"Отряд 10",
	"Отряд 11",
	"Отряд 12",
	"Отряд 13",
	"Отряд 14",
	"Отряд 15",
	"Отряд 16",
	"Отряд 17",
	"Отряд 18",
	"Отряд 19",
	"Отряд 20",
	"На службе", // on active duty
	"Доктор", // оказывает медпомощь
	"Пациент", //принимает медпомощь
	"Транспорт", // in a vehicle
	"В пути", //транзитом - сокращение
	"Ремонт", // ремонтируются
	"Практика", // тренируются
	"Ополчение", //готовят восстание среди горожан
	"Тренер", // training a teammate
	"Ученик", // being trained by someone else
	"Труп", // мертв
	"Без созн.", // abbreviation for incapacitated
	"В плену", // Prisoner of war - captured
	"Госпиталь", // patient in a hospital
	"Пуст",	// Vehicle is empty
};


static const ST::string s_rus_pMilitiaString[pMilitiaString_SIZE] =
{
	"Ополчение", // the title of the militia box
	"не распределено:", //the number of unassigned militia troops
	"Нельзя перераспределять ополчение, когда кругом враги!",
};


static const ST::string s_rus_pMilitiaButtonString[pMilitiaButtonString_SIZE] =
{
	"Авто", // auto place the militia troops for the player
	"Готово", // done placing militia troops
};

static const ST::string s_rus_pConditionStrings[pConditionStrings_SIZE] =
{
	"Отлично", //состояние солдата..отличное здоровье
	"Хорошо", //хорошее здоровье
	"Норма", //нормальное здоровье
	"Ранение", //раны
	"Без сил", // усталый
	"Кровоточит", // истекает кровью
	"Без созн.", // в обмороке
	"Умирает", //умирает
	"Труп", // мертв
};

static const ST::string s_rus_pEpcMenuStrings[pEpcMenuStrings_SIZE] =
{
	"На службе", // set merc on active duty
	"Пациент", // set as a patient to receive medical aid
	"Транспорт", // tell merc to enter vehicle
	"Без эскорта", // охрана покидает героя
	"Отмена", // выход из этого меню
};


// look at pAssignmentString above for comments

static const ST::string s_rus_pLongAssignmentStrings[pLongAssignmentStrings_SIZE] =
{
	"Отряд 1",
	"Отряд 2",
	"Отряд 3",
	"Отряд 4",
	"Отряд 5",
	"Отряд 6",
	"Отряд 7",
	"Отряд 8",
	"Отряд 9",
	"Отряд 10",
	"Отряд 11",
	"Отряд 12",
	"Отряд 13",
	"Отряд 14",
	"Отряд 15",
	"Отряд 16",
	"Отряд 17",
	"Отряд 18",
	"Отряд 19",
	"Отряд 20",
	"На службе",
	"Доктор",
	"Пациент",
	"Транспорт",
	"В пути",
	"Ремонт",
	"Практика",
	"Ополчение",
	"Тренер",
	"Ученик",
	"Труп",
	"Без сознания",
	"В плену",
	"Госпиталь", // patient in a hospital
	"Пуст",	// Vehicle is empty
};


// the contract options

static const ST::string s_rus_pContractStrings[pContractStrings_SIZE] =
{
	"Действия по контракту:",
	"", // a blank line, required
	"Продлить на 1 день", // offer merc a one day contract extension
	"Продлить на 7 дней", // 1 week
	"Продлить на 14 дней", // 2 week
	"Уволить", // end merc's contract
	"Отмена", // stop showing this menu
};

static const ST::string s_rus_pPOWStrings[pPOWStrings_SIZE] =
{
	"В плену",  //an acronym for Prisoner of War
	"??",
};

static const ST::string s_rus_pInvPanelTitleStrings[pInvPanelTitleStrings_SIZE] =
{
	"Броня", // the armor rating of the merc
	"Груз", // the weight the merc is carrying
	"Камуфляж", // the merc's camouflage rating
};

static const ST::string s_rus_pShortAttributeStrings[pShortAttributeStrings_SIZE] =
{
	"Прв", // the abbreviated version of : agility
	"Лов", // dexterity
	"Сил", // strength
	"Лид", // leadership
	"Мдр", // wisdom
	"Опт", // experience level
	"Мтк", // marksmanship skill
	"Взр", // explosive skill
	"Мех", // mechanical skill
	"Мед", // medical skill};
};


static const ST::string s_rus_pUpperLeftMapScreenStrings[pUpperLeftMapScreenStrings_SIZE] =
{
	"Задание", // the mercs current assignment
	"Здоровье", // the health level of the current merc
	"Настрой", // the morale of the current merc
	"Сост.",	// the condition of the current vehicle
};

static const ST::string s_rus_pTrainingStrings[pTrainingStrings_SIZE] =
{
	"Практика", // tell merc to train self
	"Ополчение", // tell merc to train town
	"Тренер", // tell merc to act as trainer
	"Ученик", // tell merc to be train by other
};

static const ST::string s_rus_pAssignMenuStrings[pAssignMenuStrings_SIZE] =
{
	"На службе", // merc is on active duty
	"Доктор", // the merc is acting as a doctor
	"Пациент", // the merc is receiving medical attention
	"Транспорт", // the merc is in a vehicle
	"Ремонт", // the merc is repairing items
	"Тренинг", // the merc is training
	"Отмена", // cancel this menu
};

static const ST::string s_rus_pRemoveMercStrings[pRemoveMercStrings_SIZE] =
{
	"Исключить из команды", // remove dead or captured by enemy merc from team
	"Отмена",
};

static const ST::string s_rus_pAttributeMenuStrings[pAttributeMenuStrings_SIZE] =
{
	"Сила",
	"Ловкость",
	"Проворность",
	"Здоровье",
	"Меткость",
	"Медицина",
	"Механика",
	"Лидерство",
	"Взрывник",
	"Отмена",
};

static const ST::string s_rus_pTrainingMenuStrings[pTrainingMenuStrings_SIZE] =
{
	"Практика", // train yourself
	"Ополчение", // train the town
	"Тренер", // train your teammates
	"Ученик",  // be trained by an instructor
	"Отмена", // cancel this menu
};


static const ST::string s_rus_pSquadMenuStrings[pSquadMenuStrings_SIZE] =
{
	"Отряд  1",
	"Отряд  2",
	"Отряд  3",
	"Отряд  4",
	"Отряд  5",
	"Отряд  6",
	"Отряд  7",
	"Отряд  8",
	"Отряд  9",
	"Отряд 10",
	"Отряд 11",
	"Отряд 12",
	"Отряд 13",
	"Отряд 14",
	"Отряд 15",
	"Отряд 16",
	"Отряд 17",
	"Отряд 18",
	"Отряд 19",
	"Отряд 20",
	"Отмена",
};


static const ST::string s_rus_pPersonnelScreenStrings[pPersonnelScreenStrings_SIZE] =
{
	"Медицинский депозит:", // amount of medical deposit put down on the merc
	"Срок контракта:", // time of remaining contract
	"Убийства", // number of kills by merc
	"Помощь в бою", // number of assists on kills by merc
	"Стоимость в день:", // daily cost of merc
	"Гонорар:", // total cost of merc
	"Контракт:", // cost of current contract
	"На службе:", // total service rendered by merc
	"Долг по оплате:", // amount left on MERC merc to be paid
	"Процент попаданий:", // percentage of shots that hit target
	"Битвы", // number of battles fought
	"Ранения", // number of times merc has been wounded
	"Навыки:",
	"Нет навыков",
};


//These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
static const ST::string s_rus_gzMercSkillText[gzMercSkillText_SIZE] =
{
	"Нет навыков",
	"Взлом",
	"Рукопашная",
	"Электроника",
	"Ночной бой",
	"Броски",
	"Учитель",
	"Тяжелое вооружение",
	"Автоматы",
	"Скрытность",
	"Амбидекстрия",
	"Вор",
	"Боевые искусства",
	"Бой с ножом",
	"Стрельба с крыш",
	"Маскировка",
	"(Эксперт)",
};


// This is pop up help text for the options that are available to the merc

static const ST::string s_rus_pTacticalPopupButtonStrings[pTacticalPopupButtonStrings_SIZE] =
{
	"Стоять/Идти (|S)",
	"Присядью (|C)",
	"Стоять/Бегом (|R)",
	"Лечь/Ползти (|P)",
	"Поворот (|L)",
	"Действие",
	"Поговорить",
	"Осмотреть (|C|t|r|l)",

	// Pop up door menu
	"Открыть",
	"Искать ловушки",
	"Вскрыть отмычками",
	"Взломать",
	"Обезвредить",
	"Запереть",
	"Отпереть",
	"Взорвать замок",
	"Взломать ломом",
	"Отмена (|E|s|c)",
	"Закрыть",
};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.

static const ST::string s_rus_pDoorTrapStrings[pDoorTrapStrings_SIZE] =
{
	"нет ловушки",
	"мина",
	"электроловушка",
	"сирена",
	"сигнализация"
};

// On the map screen, there are four columns. This text is popup help text that identifies the individual columns.

static const ST::string s_rus_pMapScreenMouseRegionHelpText[pMapScreenMouseRegionHelpText_SIZE] =
{
	"Выбрать наемника",
	"Дать задание",
	"Проложить маршрут",
	"Управление контрактом (|C)",
	"Убрать бойца",
	"Спать",
};

// volumes of noises

static const ST::string s_rus_pNoiseVolStr[pNoiseVolStr_SIZE] =
{
	"ТИХИЙ",
	"ЧЕТКИЙ",
	"ГРОМКИЙ",
	"ОГЛУШИТЕЛЬНЫЙ"
};

// types of noises

static const ST::string s_rus_pNoiseTypeStr[pNoiseTypeStr_SIZE] = // OBSOLETE
{
	"НЕЗНАКОМЫЙ",
	"звук ДВИЖЕНИЯ",
	"СКРИП",
	"ПЛЕСК",
	"УДАР",
	"ВЫСТРЕЛ",
	"ВЗРЫВ",
	"КРИК",
	"УДАР",
	"УДАР",
	"ЗВОН",
	"ГРОХОТ"
};

// Directions that are used to report noises

static const ST::string s_rus_pDirectionStr[pDirectionStr_SIZE] =
{
	"с СЕВ-ВОСТОКА",
	"с ВОСТОКА",
	"с ЮГО-ВОСТОКА",
	"с ЮГА",
	"с ЮГО-ЗАПАДА",
	"с ЗАПАДА",
	"с СЕВ-ЗАПАДА",
	"с СЕВЕРА"
};

static const ST::string s_rus_gpStrategicString[gpStrategicString_SIZE] =
{
	"%s обнаружены в секторе %c%d и вот-вот прибудет еще один отряд.", //STR_DETECTED_SINGULAR
	"%s обнаружены в секторе %c%d и вот-вот прибудут еще отряды.",	//STR_DETECTED_PLURAL
	"Хотите координировать одновременное прибытие?",													//STR_COORDINATE

	//Dialog strings for enemies.

	"Враг предлагает сдаться.",			//STR_ENEMY_SURRENDER_OFFER
	"Враг взял в плен наемников без сознания.",	//STR_ENEMY_CAPTURED

	//The text that goes on the autoresolve buttons

	"Отступить", 		//The retreat button		//STR_AR_RETREAT_BUTTON
	"Готово",		//The done button				//STR_AR_DONE_BUTTON

	//The headers are for the autoresolve type (MUST BE UPPERCASE)

	"ОБОРОНА",								//STR_AR_DEFEND_HEADER
	"АТАКА",								//STR_AR_ATTACK_HEADER
	"СТОЛКНОВЕНИЕ",						//STR_AR_ENCOUNTER_HEADER
	"Сектор",		//The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

	//The battle ending conditions

	"ПОБЕДА!",								//STR_AR_OVER_VICTORY
	"ПОРАЖЕНИЕ!",							//STR_AR_OVER_DEFEAT  ///TRNSL_CHECK_INGAME LENGTH
	"СДАЧА!",								//STR_AR_OVER_SURRENDERED
	"В ПЛЕНУ!",							//STR_AR_OVER_CAPTURED
	"ОТСТУПЛЕНИЕ!",						//STR_AR_OVER_RETREATED

	//These are the labels for the different types of enemies we fight in autoresolve.

	"Ополчение",							//STR_AR_MILITIA_NAME,
	"Элита",								//STR_AR_ELITE_NAME,
	"Солдат",								//STR_AR_TROOP_NAME,
	"Админ",								//STR_AR_ADMINISTRATOR_NAME,
	"Существо",								//STR_AR_CREATURE_NAME,

	//Label for the length of time the battle took

	"Бой шел",						//STR_AR_TIME_ELAPSED,

	//Labels for status of merc if retreating.  (UPPERCASE)

	"ОТСТУПИЛ",							//STR_AR_MERC_RETREATED,
	"ОТСТУПАЕТ",							//STR_AR_MERC_RETREATING,
	"ОТСТУПЛЕНИЕ",							//STR_AR_MERC_RETREAT,

	//PRE BATTLE INTERFACE STRINGS
	//Goes on the three buttons in the prebattle interface.  The Auto resolve button represents
	//a system that automatically resolves the combat for the player without having to do anything.
	//These strings must be short (two lines -- 6-8 chars per line)

	"Авто битва",							//STR_PB_AUTORESOLVE_BTN,
	"Идти в сектор",						//STR_PB_GOTOSECTOR_BTN,
	"Уйти",								//STR_PB_RETREATMERCS_BTN,

	//The different headers(titles) for the prebattle interface.
	"СТОЛКНОВЕНИЕ",						//STR_PB_ENEMYENCOUNTER_HEADER,
	"НАПАДЕНИЕ",							//STR_PB_ENEMYINVASION_HEADER, // 30
	"ЗАСАДА",								//STR_PB_ENEMYAMBUSH_HEADER
	"ВРАЖЕСКИЙ СЕКТОР",					//STR_PB_ENTERINGENEMYSECTOR_HEADER
	"АТАКА СУЩЕСТВ",						//STR_PB_CREATUREATTACK_HEADER
	"ЗАСАДА КОШЕК-УБИЙЦ",					//STR_PB_BLOODCATAMBUSH_HEADER
	"ЛОГОВО КОШЕК-УБИЙЦ",					//STR_PB_ENTERINGBLOODCATLAIR_HEADER

	//Various single words for direct translation.  The Civilians represent the civilian
	//militia occupying the sector being attacked.  Limited to 9-10 chars

	"Место",
	"Враги",
	"Наемники",
	"Ополченцы",
	"Существа",
	"Кошки",
	"Сектор",
	"Никого",		//If there are no uninvolved mercs in this fight.
	"Н/П",			//Acronym of Not Applicable
	"д",			//One letter abbreviation of day
	"ч",			//One letter abbreviation of hour

	//TACTICAL PLACEMENT USER INTERFACE STRINGS
	//The four buttons

	"Очистить",
	"Порознь",
	"Группой",
	"Готово",

	//The help text for the four buttons.  Use \n to denote new line (just like enter).

	"Отменить размещение и начать заново (|C)",
	"Равномерно распределить бойцов (|S)",
	"Сгруппировать бойцов в заданной точке (|G)",
	"Нажмите эту кнопку, когда закончите\nвыбор позиций для бойцов (|E|n|t|e|r).",
	"Нужно разместить ВСЕХ наемников\nперед началом битвы.",

	//Various strings (translate word for word)

	"Сектор",
	"выберите исходные позиции",

	//Strings used for various popup message boxes.  Can be as long as desired.

	"Не очень-то хорошее место. Туда не пройти. Выберите другую позицию.",
	"Разместите своих наемников в выделенной области.",

	//These entries are for button popup help text for the prebattle interface.  All popup help
	//text supports the use of \n to denote new line.  Do not use spaces before or after the \n.
	"Битва разрешается автоматически\nбез вашего участия (|A)",
	"Нельзя использовать авто битву\nкогда вы нападаете",
	"Войти в сектор: стычка с врагом (|E)",
	"Отступить в исходный сектор (|R)",				//singular version
	"Отступить всем отрядам на исходные позиции (|R)", //multiple groups with same previous sector
//!!!What about repeated "R" as hotkey?
	//various popup messages for battle conditions.

	//%c%d is the sector -- ex:  A9
	"Враги атакуют ваше ополчение в секторе %c%d.",
	//%c%d сектор -- напр:  A9
	"Существа напали на ополчение в секторе %c%d.",
	//1st %d refers to the number of civilians eaten by monsters,  %c%d is the sector -- ex:  A9
	//Note:  the minimum number of civilians eaten will be two.
	"Существа напали на гражданских, убито %d в секторе %s.",
	//%s is the sector location -- ex:  A9: Omerta
	"Враги атакуют ваш отряд секторе %s. Никто из наемников не может сражаться!",
	//%s is the sector location -- ex:  A9: Omerta
	"Существа атакуют ваш отряд в секторе %s. Никто из наемников не может сражаться!",

};

//This is the day represented in the game clock.  Must be very short, 4 characters max.
static const ST::string s_rus_gpGameClockString = "День";

//When the merc finds a key, they can get a description of it which
//tells them where and when they found it.
static const ST::string s_rus_sKeyDescriptionStrings[sKeyDescriptionStrings_SIZE] =
{
	"Найден в:",
	"День находки:",
};

//The headers used to describe various weapon statistics.

static const ST::string s_rus_gWeaponStatsDesc[ gWeaponStatsDesc_SIZE] =
{
	"Вес (%s):",
	"Состояние:",
	"Патроны:", 	// Number of bullets left in a magazine
	"Дист:",		// Range
	"Урон:",		// Damage
	"ОД:",			// abbreviation for Action Points
	"="
};

//The headers used for the merc's money.

static const ST::string s_rus_gMoneyStatsDesc[gMoneyStatsDesc_SIZE] =
{
	"Денег в",
	"пачке:", //this is the overall balance
	"Отделить",
	"сумму:", // the amount he wants to separate from the overall balance to get two piles of money

	"Текущий",
	"баланс",
	"Взять со",
	"счета",
};

//The health of various creatures, enemies, characters in the game. The numbers following each are for comment
//only, but represent the precentage of points remaining.

static const ST::string s_rus_zHealthStr[zHealthStr_SIZE] = ///TRNSL_BAD (GENDER)
{
	"УМИРАЕТ",  // >=  0
	"КРИТИЧНО", // >= 15
	"ПЛОХ",     // >= 30
	"РАНЕН",    // >= 45
	"ЗДОРОВ",   // >= 60
	"СИЛЕН",    // >= 75
	"ОТЛИЧНО",  // >= 90
};

static const ST::string s_rus_gzMoneyAmounts[gzMoneyAmounts_SIZE] =
{
	"1000$",
	"100$",
	"10$",
	"OK",
	"Отделить",
	"Взять"
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
static const ST::string s_rus_gzProsLabel = "Плюсы:";
static const ST::string s_rus_gzConsLabel = "Минусы:";

//Conversation options a player has when encountering an NPC
static const ST::string s_rus_zTalkMenuStrings[zTalkMenuStrings_SIZE] =
{
	"Еще раз?", 	//meaning "Repeat yourself"
	"Дружески",		//approach in a friendly
	"Прямо",		//approach directly - let's get down to business
	"Угрожать",		//approach threateningly - talk now, or I'll blow your face off
	"Дать",
	"Нанять"
};

//Some NPCs buy, sell or repair items. These different options are available for those NPCs as well.
static const ST::string s_rus_zDealerStrings[zDealerStrings_SIZE] =
{
	"Торговля",
	"Купить",
	"Продать",
	"Ремонт",
};

static const ST::string s_rus_zDialogActions = "Готово";


//These are vehicles in the game.

static const ST::string s_rus_pVehicleStrings[pVehicleStrings_SIZE] =
{
	"Эльдорадо",
	"Хаммер", // a hummer jeep/truck -- military vehicle
	"Минивэн",
	"Джип",
	"Танк",
	"Вертолет",
};

static const ST::string s_rus_pShortVehicleStrings[pVehicleStrings_SIZE] =
{
	"Эльдор",
	"Хаммер",			// the HMVV
	"Вэн",
	"Джип",
	"Танк",
	"Верт.", 				// the helicopter
};

static const ST::string s_rus_zVehicleName[pVehicleStrings_SIZE] =
{
	"Эльдорадо",
	"Хаммер",		//a military jeep. This is a brand name.
	"Вэн",			// Ice cream truck
	"Джип",
	"Танк",
	"Верт.", 		//an abbreviation for Helicopter
};


//These are messages Used in the Tactical Screen

static const ST::string s_rus_TacticalStr[TacticalStr_SIZE] =
{
	"Воздушный налет",
	"Оказать первую помощь?",

	// CAMFIELD NUKE THIS and add quote #66.

	"%s замечает недостачу товара в ящике.",

	// The %s is a string from pDoorTrapStrings

	"К замку присоединена %s.", ///TRNSL_ASSUME all traps are female gender words
	"Тут нет замка.",
	"Замок без ловушки.",
	// The %s is a merc name
	"%s не имеет нужного ключа.",
	"Замок без ловушки.",
	"Заперто.",
	"ДВЕРЬ",
	"С ЛОВУШКОЙ",
	"ЗАПЕРТАЯ",
	"НЕЗАПЕРТАЯ",
	"СЛОМАНАЯ",
	"Тут есть выключатель. Нажать?",
	"Обезвредить ловушку?",
	"Еще предметы...",

	// In the next 2 strings, %s is an item name

	"%s теперь на земле.",
	"Предмет \'%s\' получает %s.",

	// In the next 2 strings, %s is a name

	"%s получил(а) всю сумму.",
	"%s ожидает от вас уплаты еще %d.",
	"Выберите частоту детонатора:",  	//in this case, frequency refers to a radio signal
	"Количество ходов перед взрывом:",	//how much time, in turns, until the bomb blows
	"Выберите частоту подрыва:", 	//in this case, frequency refers to a radio signal
	"Разрядить ловушку?",
	"Убрать флажок?",
	"Установить флажок?",
	"Завершение хода",

	// In the next string, %s is a name. Stance refers to way they are standing.

	"%s на вашей стороне! Атаковать?",
	"Ох, транспорт не может менять положения.",
	"Робот не может менять положения.",

	// In the next 3 strings, %s is a name

	"%s не может здесь поменять положение.",
	"%s не может быть перевязан.",
	"%s не нуждается в перевязке.",
	"Туда идти нельзя.",
	"Команда набрана. Нет места для новобранца.",	//there's no room for a recruit on the player's team

	// In the next string, %s is a name

	"%s теперь в команде.",

	// Here %s is a name and %d is a number

	"%s ожидает уплаты еще $%d.",

	// In the next string, %s is a name

	"%s будет сопровождаться в составе вашего отряда. Согласны?",

	// In the next string, the first %s is a name and the second %s is an amount of money (including $ sign)

	"%s может вступить в ваш отряд за %s в день. Нанять?",

	// This line is used repeatedly to ask player if they wish to participate in a boxing match.

	"Хотите драться?",

	// In the next string, the first %s is an item name and the
	// second %s is an amount of money (including $ sign)

	"Купить %s за %s?",

	// In the next string, %s is a name

	"%s теперь сопровождается отрядом %d.",

	// These messages are displayed during play to alert the player to a particular situation

	"ЗАКЛИНИЛО",					//weapon is jammed.
	"Роботу нужны патроны %s.",		//Robot is out of ammo
	"Бросить туда? Не получится.",		//Merc can't throw to the destination he selected

	// These are different buttons that the player can turn on and off.

	"Режим скрытности (|Z)",
	"Окно карты (|M)",
	"Завершить ход (|D)",
	"Говорить",
	"Запретить отзывы",
	"Подняться (|P|g|U|p)",
	"Уровень курсора (|T|a|b)",
	"Залезть/Слезть",
	"Опуститься (|P|g|D|n)",
	"Осмотреть (|C|t|r|l)",
	"Предыдущий боец",
	"Следующий боец (|S|p|a|c|e)",
	"Настройки (|O)",
	"Стрелять очередью (|B)",
	"Смотреть/Повернуться (|L)",
	"Здоровье: %d/%d\nЭнергия.: %d/%d\nНастрой: %s",
	"Чего?",					//this means "what?"
	"Продолж.",					//an abbrieviation for "Continued"
	"%s будет говорить.",
	"%s будет молчать.",
	"Состояние: %d/%d\nТопливо: %d/%d",
	"Выйти из машины",
	"Поменять отряд (|S|h|i|f|t |S|p|a|c|e)",
	"Вести машину",
	"Н/П",						//this is an acronym for "Not Applicable."
	"Применить (Рукопашная)",
	"Применить (Оружие)",
	"Применить (Нож)",
	"Применить (Взрывчатка)",
	"Применить (Аптечка)",
	"(Ловит)",
	"(Перезарядка)", ///TRNSL_CHECK_INGAME LENGTH
	"(Дать)",
	"Сработала %s.", // The %s here is a string from pDoorTrapStrings  ASSUME all traps are female gender
	"%s прибыл(a).",
	"%s: нет очков действия.",
	"%s: наемник недоступен.",
	"%s: успешная перевязка.",
	"%s: нет бинтов.",
	"Враг в секторе!",
	"Нет врагов в поле зрения.",
	"Не хватает очков действия.",
	"Никто не использует дистанционное управление.",
	"Обойма опустела!",
	"СОЛДАТ",
	"РЕПТИОН",
	"ОПОЛЧЕНЕЦ",
	"ЖИТЕЛЬ",
	"Выход из сектора",
	"OK",
	"ОТМЕНА",
	"Выбранный боец",
	"Весь отряд",
	"Идти в сектор",
	"Идти на карту",
	"Этот сектор нельзя покинуть здесь.",
	"%s слишком далеко.",
	"Вершины деревьев скрыты",
	"Вершины деревьев отображены",
	"ВОРОНА",				//Crow, as in the large black bird
	"ШЕЯ",
	"ГОЛОВА",
	"ГРУДЬ",
	"НОГИ",
	"Сказать королеве то, что она хочет знать?",
	"Отпечатки пальцев получены",
	"Отпечатки неверны. Оружие заблокировано",
	"Цель захвачена",
	"Путь блокирован",
	"Положить/Взять деньги",		//Help text over the $ button on the Single Merc Panel
	"Некого лечить.",
	"Слом.",											// Short form of JAMMED, for small inv slots
	"Туда не добраться.",					// used ( now ) for when we click on a cliff
	"Человек отказывается двигаться.",
	// In the following message, '%s' would be replaced with a quantity of money (e.g. $200)
	"Заплатить %s?",
	"Согласиться на бесплатное лечение?",
	"Согласны женить Дэррела?",
	"Ключи",
	"С эскортируемыми этого сделать нельзя.",
	"Пощадить Кротта?",
	"Цель вне зоны эффективного огня.",
	"Шахтер",
	"Транспорт передвигается только между секторами.",
	"Автоперевязку сделать сейчас нельзя",
	"%s не может пройти, путь блокирован.",
	"Здесь томятся ваши бойцы, захваченные армией Дейдраны!",
	"Попадание в замок",
	"Замок разрушен",
	"Кто-то еще пытается воспользоваться этой дверью.",
	"Состояние: %d/%d\nТопливо: %d/%d",
	"%s и %s не видят друг друга.", // Cannot see person trying to talk to
};

//Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different circumstances in the "exiting sector" interface.
static const ST::string s_rus_pExitingSectorHelpText[pExitingSectorHelpText_SIZE] =
{
	//Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box is checked.
	"Если выбрана эта опция, вы сразу перейдете в смежный сектор.",
	"Если выбрана эта опция, вы выйдете\nна карту, пока ваши бойцы в пути.",

	//If you attempt to leave a sector when you have multiple squads in a hostile sector.
	"Сектор занят врагами и оставлять здесь бойцов нельзя.\nРазберитесь с противником прежде чем идти в другие сектора.",

	//Because you only have one squad in the sector, and the "move all" option is checked, the "go to sector" option is locked to on.
	//The helptext explains why it is locked.
	"Выведя всех наемников из сектора,\nвы автоматически перейдете в смежный сектор.",
	"Выведя всех наемников из сектора,\nвы автоматически выйдете на карту,\nпока ваши бойцы в пути.",

	//If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being escorted.  The "single" button is disabled.
	"%s не может покинуть этот сектор без сопровождения.",

	//If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc will be prohibited from leaving alone.
	//There are several strings depending on the gender of the merc and how many EPCs are in the squad.
	//DO NOT USE THE NEWLINE HERE AS IT IS USED FOR BOTH HELPTEXT AND SCREEN MESSAGES!
	"%s не может покинуть сектор один - он сопровождает %s.", //male singular
	"%s не может покинуть сектор одна - она сопровождает %s.", //female singular
	"%s не может покинуть сектор один - он сопровождает группу.", //male plural
	"%s не может покинуть сектор одна - она сопровождает группу.", //female plural

	//If one or more of your mercs in the selected squad aren't in range of the traversal area, then the  "move all" option is disabled,
	//and this helptext explains why.
	"Для перемещения отряда\nвсе ваши наемники дожны быть рядом.",

	//Standard helptext for single movement.  Explains what will happen (splitting the squad)
	"Если выбрана эта опция, %s пойдет в одиночку и\nавтоматически попадет в отдельный отряд.",

	//Standard helptext for all movement.  Explains what will happen (moving the squad)
	"Если выбрана эта опция, текущий \nотряд покинет этот сектор.",

	//This strings is used BEFORE the "exiting sector" interface is created.  If you have an EPC selected and you attempt to tactically
	//traverse the EPC while the escorting mercs aren't near enough (or dead, dying, or unconscious), this message will appear and the
	//"exiting sector" interface will not appear.  This is just like the situation where
	//This string is special, as it is not used as helptext.  Do not use the special newline character (\n) for this string.
	"%s не может покинуть этот сектор без сопровождения. Для этого ваш отряд должен быть рядом.",
};



static const ST::string s_rus_pRepairStrings[pRepairStrings_SIZE] =
{
	"Вещи", 		// tell merc to repair items in inventory
	"ПВО", 		// tell merc to repair SAM site - SAM is an acronym for Surface to Air Missile
	"Отмена", 		// cancel this menu
	"Робот", 		// repair the robot
};


// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."

static const ST::string s_rus_sPreStatBuildString[sPreStatBuildString_SIZE] =
{
	"теряет", 			// the merc has lost a statistic
	"получает", 		// the merc has gained a statistic
	"очко",	// singular
	"очка",	// plural
	"уровень",	// singular
	"уровня",	// plural
};

static const ST::string s_rus_sStatGainStrings[sStatGainStrings_SIZE] =
{
	"здоровья.",
	"подвижности.",
	"проворности.",
	"мудрости.",
	"медицины.",
	"умения взрывника.",
	"умения механика.",
	"меткости.",
	"опыта.",
	"силы.",
	"лидерства.",
};


static const ST::string s_rus_pHelicopterEtaStrings[pHelicopterEtaStrings_SIZE] =
{
	"Общее расстояние:  ", 			// total distance for helicopter to travel
	" Безопасно:  ", 			// distance to travel to destination
	" Опасно:", 			// distance to return from destination to airport
	"Цена полета: ", 		// total cost of trip by helicopter
	"ОВП:  ", 			// ETA is an acronym for "estimated time of arrival"
	"У вертолета мало топлива, придется сесть на территории врага!",	// warning that the sector the helicopter is going to use for refueling is under enemy control ->
	"Пассажиры: ",
	"Высадить Всадника или прибывающих?",
	"Всадник",
	"Прибывающие",
};

static const ST::string s_rus_sMapLevelString = "Подуровень "; // what level below the ground is the player viewing in mapscreen

static const ST::string s_rus_gsLoyalString = "Отношение %d%%"; // the loyalty rating of a town ie : Loyal 53%


// error message for when player is trying to give a merc a travel order while he's underground.
static const ST::string s_rus_gsUndergroundString = "не может двигаться по карте, находясь под землей.";

static const ST::string s_rus_gsTimeStrings[gsTimeStrings_SIZE] =
{
	"ч",				// hours abbreviation
	"м",				// minutes abbreviation
	"с",				// seconds abbreviation
	"д",				// days abbreviation
};

// text for the various facilities in the sector

static const ST::string s_rus_sFacilitiesStrings[sFacilitiesStrings_SIZE] =
{
	"Нет",
	"Госпиталь",
	"Заводы",
	"Тюрьма",
	"База армии",
	"Аэропорт",
	"Стрельбище",		// a field for soldiers to practise their shooting skills
};

// text for inventory pop up button

static const ST::string s_rus_pMapPopUpInventoryText[pMapPopUpInventoryText_SIZE] =
{
	"Предметы",
	"Выход",
};

// town strings

static const ST::string s_rus_pwTownInfoStrings[pwTownInfoStrings_SIZE] =
{
	"Размер",					// size of the town in sectors
	"Контроль над городом", 					// how much of town is controlled
	"Шахта города", 				// mine associated with this town
	"Отношение",					// the loyalty level of this town
	"Сооружения", 				// main facilities in this town
	"Подготовка жителей",				// state of civilian training in town
	"Ополчение", 					// the state of the trained civilians in the town
};

// Mine strings

static const ST::string s_rus_pwMineStrings[pwMineStrings_SIZE] =
{
	"Шахта",						// 0
	"Серебро",
	"Золото",
	"Выработка в день",
	"Максимум выработки",
	"Заброшена",				// 5
	"Закрыта",
	"Истощена",
	"Работает",
	"Состояние",
	"Производительность",
	"Тип руды",				// 10
	"Контроль над городом",
	"Отношение города",
};

// blank sector strings

static const ST::string s_rus_pwMiscSectorStrings[pwMiscSectorStrings_SIZE] =
{
	"Силы врага",
	"Сектор",
	"Количество предметов",
	"?",
	"Сектор захвачен",
	"Да",
	"Нет",
};

// error strings for inventory

static const ST::string s_rus_pMapInventoryErrorString[pMapInventoryErrorString_SIZE] =
{
	"Нельзя выбрать этого бойца.",  //MARK CARTER
	"%s не в этом секторе, и не может взять этот предмет.",
	"Во время битвы надо подбирать предметы вручную",
	"Во время битвы надо бросать предметы вручную.",
	"%s не в этом секторе, и не может бросить этот предмет.",
};

static const ST::string s_rus_pMapInventoryStrings[pMapInventoryStrings_SIZE] =
{
	"Сектор", 			// sector these items are in
	"Всего предметов", 		// total number of items in sector
};


// movement menu text

static const ST::string s_rus_pMovementMenuStrings[pMovementMenuStrings_SIZE] =
{
	"Отправить бойцов из сектора %s", 	// title for movement box
	"Проложить маршрут", 		// done with movement menu, start plotting movement
	"Отмена", 		// cancel this menu
	"Другие",		// title for group of mercs not on squads nor in vehicles
};


static const ST::string s_rus_pUpdateMercStrings[pUpdateMercStrings_SIZE] =
{
	"Ой!:", 			// an error has occured
	"Контракты закончились:", 	// this pop up came up due to a merc contract ending
	"Бойцы выполнили задание:", // this pop up....due to more than one merc finishing assignments
	"Бойцы снова в строю:", // this pop up ....due to more than one merc waking up and returing to work
	"Бойцы идут спать:", // this pop up ....due to more than one merc being tired and going to sleep
	"Контракты скоро кончатся:", // this pop up came up due to a merc contract ending
};

// map screen map border buttons help text

static const ST::string s_rus_pMapScreenBorderButtonHelpText[pMapScreenBorderButtonHelpText_SIZE] =
{
	"Показать города (|W)",
	"Показать шахты (|M)",
	"Показать отряды и врагов (|T)",
	"Показать воздушное пространство (|A)",
	"Показать предметы (|I)",
	"Показать ополчение и врагов (|Z)",
};


static const ST::string s_rus_pMapScreenBottomFastHelp[pMapScreenBottomFastHelp_SIZE] =
{
	"Лэптоп (|L)",
	"Тактика (|E|s|c)",
	"Настройки (|O)",
	"Ускорение времени (|+)", 	// time compress more
	"Замедление времени (|-)", 	// time compress less
	"Предыдущее сообщение (|U|p)\nПредыдущая страница (|P|g|U|p)", 	// previous message in scrollable list
	"Следующее сообщение (|D|o|w|n)\nСледующая страница (|P|g|D|n)", 	// next message in the scrollable list
	"Пауза/Снять с паузы (|S|p|a|c|e)",	// start/stop time compression
};

static const ST::string s_rus_pMapScreenBottomText = "Текущий баланс"; // current balance in player bank account

static const ST::string s_rus_pMercDeadString = "%s мертв(а).";


static const ST::string s_rus_pDayStrings = "День";

// the list of email sender names

static const ST::string s_rus_pSenderNameList[pSenderNameList_SIZE] = // GOLD and BUKA use slightly different names
{
	"Энрико",
	"Псих Про Инк.",
	"Помощь",
	"Псих Про Инк.",
	"Спек",
	"R.I.S.",		//5
	"Барри",
	"Блад",
	"Рысь",
	"Гризли",
	"Вики",			//10
	"Тревер",
	"Хряп",
	"Иван",
	"Анаболик",
	"Игорь",			//15
	"Тень",
	"Рыжий",
	"Потрошитель",
	"Фидель",
	"Лиска",				//20
	"Сидней",
	"Гас",
	"Сдоба",
	"Айс",
	"Паук",		//25
	"Клифф",
	"Бык",
	"Стрелок",
	"Тоска",
	"Рейдер",		//30
	"Сова",
	"Статик",
	"Лен",
	"Данни",
	"Маг",
	"Стэфен",
	"Лысый",
	"Злобный",
	"Доктор Кью",
	"Гвоздь",
	"Тор",
	"Стрелка",
	"Волк",
	"ЭмДи",
	"Лава",
	//----------
	"M.I.S. Страх",
	"Бобби Рэй",
	"Босс",
	"Джон Калба",
	"А.I.М.",
};


static const ST::string s_rusGold_pSenderNameList[pSenderNameList_SIZE] = // GOLD and BUKA use slightly different names
{
	"Энрико",
	"Псих Про Инк.",
	"Помощь",
	"Псих Про Инк.",
	"Спек",
	"R.I.S.",		//5
	"Барри",
	"Блад",
	"Рысь",
	"Гризли",
	"Вики",			//10
	"Тревор",
	"Хряп",
	"Иван",
	"Анаболик",
	"Игорь",			//15
	"Тень",
	"Рыжий",
	"Жнец",
	"Фидель",
	"Лиска",				//20
	"Сидней",
	"Гас",
	"Сдоба",
	"Айс",
	"Паук",		//25
	"Скала",
	"Бык",
	"Стрелок",
	"Тоска",
	"Рейдер",		//30
	"Сова",
	"Статик",
	"Лен",
	"Данни",
	"Маг",
	"Стефан",
	"Лысый",
	"Злобный",
	"Доктор Кью",
	"Гвоздь",
	"Тор",
	"Стрелка",
	"Волк",
	"ЭмДи",
	"Лава",
	//----------
	"M.I.S. Страх",
	"Бобби Рэй",
	"Босс",
	"Джон Калба",
	"А.I.М.",
};


// new mail notify string
static const ST::string s_rus_pNewMailStrings = "Получена новая почта...";


// confirm player's intent to delete messages

static const ST::string s_rus_pDeleteMailStrings[pDeleteMailStrings_SIZE] =
{
	"Стереть сообщение?",
	"Стереть НЕПРОЧТЕННЫЕ?",
};


// the sort header strings

static const ST::string s_rus_pEmailHeaders[pEmailHeaders_SIZE] =
{
	"От:",
	"Тема:",
	"Дата:",
};

// email titlebar text
static const ST::string s_rus_pEmailTitleText = "Почтовый ящик";


// the financial screen strings
static const ST::string s_rus_pFinanceTitle = "Бухгалтер Плюс"; // the name we made up for the financial program in the game

static const ST::string s_rus_pFinanceSummary[pFinanceSummary_SIZE] =
{
	"Расход:", 				// credit (subtract from) to player's account
	"Приход:", 				// debit (add to) to player's account
	"Прибыль за вчерашний день:",
	"Другие вклады за вчерашний день:",
	"Приход за вчерашний день:",
	"Баланс на конец дня:",
	"Приход за сегодня:",
	"Другие вклады за сегодня:",
	"Приход на сегодня:",
	"Текущий баланс:",
	"Предполагаемый приход:",
	"Предполагаемый баланс:", 		// projected balance for player for tommorow
};


// headers to each list in financial screen

static const ST::string s_rus_pFinanceHeaders[pFinanceHeaders_SIZE] =
{
	"День", 					// the day column
	"Приход", 				// the credits column
	"Расход",				// the debits column
	"Операция", 			// transaction type - see TransactionText below
	"Баланс", 				// balance at this point in time
	"Стр.", 				// page number
	"Дн.", 				// the day(s) of transactions this page displays
};


static const ST::string s_rus_pTransactionText[pTransactionText_SIZE] =
{
	"Проценты",			// interest the player has accumulated so far
	"Анонимный вклад",
	"Плата за перевод",
	"%s нанят(а) в AIM", // Merc was hired
	"Покупки у Бобби Рэя", 		// Bobby Ray is the name of an arms dealer
	"Оплата по счетам M.E.R.C.",
	"%s: оплачен мед. депозит", 		// medical deposit for merc
	"Профилирование в IMP", 		// IMP is the acronym for International Mercenary Profiling
	"Застрахован(а) %s",
	"%s: cнижена страховка",
	"%s: продлена страховка", 				// johnny contract extended
	"%s: отмена страховки",
	"%s: выплата cтраховки", 		// insurance claim for merc
	"%s: контракт продлен на 1 день", 				// entend mercs contract by a day
	"%s: контракт продлен на 7 дней",
	"%s: контракт продлен на 14 дней",
	"Доход с шахт",
	"", //String nuked
	"Приобретение цветов",
	"%s: возврат депозита",
	"%s: возврат части депозита",
	"%s: удержание депозита",
	"Выплаты %s",		// %s is the name of the npc being paid
	"Перевод средств на имя %s", 			// transfer funds to a merc
	"Перевод средств от %s", 		// transfer funds from a merc
	"Ополчение в %s", // initial cost to equip a town's militia
	"%s: оплата покупок.",	//is used for the Shop keeper interface.  The dealers name will be appended to the end of the string.
	"%s: вклад наличными.",
};

// helicopter pilot payment

static const ST::string s_rus_pSkyriderText[pSkyriderText_SIZE] =
{
	"Всаднику выплачено $%d", 			// skyrider was paid an amount of money
	"Всаднику недоплачено $%d", 		// skyrider is still owed an amount of money
	"У Всадника нет пассажиров. Если вы собирались перевозить бойцов - назначьте им ЗАДАНИЕ ТРАНСПОРТ."
};


// strings for different levels of merc morale

static const ST::string s_rus_pMoralStrings[pMoralStrings_SIZE] =
{
	"Боевой",
	"Бодрый",
	"Норма",
	"Уныние",
	"Паника",
	"Разгром",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.
static const ST::string s_rus_str_left_equipment   = "%s оставляет снаряжение в %s (%c%d).";

// Status that appears on the Map Screen

static const ST::string s_rus_pMapScreenStatusStrings[pMapScreenStatusStrings_SIZE] =
{
	"Здоровье",
	"Энергия",
	"Настрой",
	"Состояние",	// the condition of the current vehicle (its "health")
	"Бензин",	// the fuel level of the current vehicle (its "energy")
};


static const ST::string s_rus_pMapScreenPrevNextCharButtonHelpText[pMapScreenPrevNextCharButtonHelpText_SIZE] =
{
	"Предыдущий наемник (|L|e|f|t)", 			// previous merc in the list
	"Следующий наемник (|R|i|g|h|t)", 				// next merc in the list
};


static const ST::string s_rus_pEtaString = "ОВП:"; // eta is an acronym for Estimated Time of Arrival

static const ST::string s_rus_pTrashItemText[pTrashItemText_SIZE] =
{
	"Вы потеряете этот предмет навсегда. Выбросить?", 	// do you want to continue and lose the item forever
	"Похоже, это действительно ВАЖНАЯ вещь. Вы ТОЧНО уверены, что хотите ее выбросить?", // does the user REALLY want to trash this item
};


static const ST::string s_rus_pMapErrorString[pMapErrorString_SIZE] =
{
	"Отряд не может двигаться, пока бойцы спят.",

//1-5
	"Сперва выведите отряд на поверхность.",
	"Приказ двигаться? Тут же кругом враги!",
	"Боец должен быть членом отряда или пассажиром, чтобы путешествовать.",
	"У вас в команде пока никого нет",	// you have no members, can't do anything
	"Боец не может выполнить приказ.",		// merc can't comply with your order
//6-10
	"%s нуждается в сопровождении. Назначьте его в отряд.", // merc can't move unescorted .. for a male
	"%s нуждается в сопровождении. Назначьте ее в отряд.", // for a female
	"Наемник еще не прибыл в Арулько!",
	"Кажется, сначала нужно уладить некоторые проблемы с контрактом.",
	"",
//11-15
	"Приказ двигаться? Тут же битва идет!",
	"Вы попали в засаду кошек-убийц в секторе %s!",
	"Вы попали в логово кошек-убийц в секторе I16!",
	"",
	"База ПВО в %s захвачена врагом.",
//16-20
	"%s: шахта захвачена. Ваш ежедневный доход упал до %s в день.",
	"Противник взял сектор %s, не встретив сопротивления.",
	"Как минимум одного из ваших бойцов нельзя назначить на это задание.",
	"%s не может вступить в %s. Отряд полон.",
	"%s не может вступить в %s. Слишком далеко.",
//21-25
	"%s: шахта захвачена войсками Дейдраны!",
	"Войска Дейдраны захватили базу ПВО в %s",
	"%s: в город вошли войска Дейдраны!",
	"Войска Дейдраны были замечены в %s.",
	"Войска Дейдраны захватили %s.",
//26-30
	"Как минимум одного из ваших бойцов невозможно уложить спать.",
	"Как минимум одного из ваших бойцов невозможно разбудить.",
	"Ополчение небоеспособно, пока не закончится его обучение.",
	"%s не может передвигаться прямо сейчас.",
	"Ополчение, которое находится вне города, нельзя переместить в другой сектор.",
//31-35
	"%s не может иметь ополчения.",
	"Машине нужен водитель!",
	"%s слишком изранен(а), чтобы идти!",
	"Сперва надо покинуть музей!",
	"%s мертв(а)!",
//36-40
	"%s не может перейти к %s: он в движении", ///TRNSL_CHECK MEANING
	"%s не может сесть в машину так",///TRNSL_CHECK MEANING
	"%s не может присоединиться к %s",///TRNSL_CHECK MEANING
	"Нельзя ускорять время, пока у вас нет бойцов!",
	"Эта машина может ездить только по дорогам!",
//41-45
	"Нельзя переназначать наемников в пути",///TRNSL_CHECK MEANING
	"У транспорта кончился бензин!",
	"%s не может передвигаться из-за усталости.",
	"Никто из сидящих в машине не может управлять ею.",
	"Один или более бойцов этого отряда сейчас не могут перемещаться.",
//46-50
	"Один или более ДРУГИХ бойцов сейчас не могут перемещаться.",///TRNSL_CHECK MEANING
	"Транспорт слишком сильно поврежден!",
	"Тренировать ополчение может не больше 2 бойцов на сектор.",
	"Робот не может двигаться без оператора. Поместите их в один отряд.",
};


// help text used during strategic route plotting
static const ST::string s_rus_pMapPlotStrings[pMapPlotStrings_SIZE] =
{
	"Кликните на секторе еще раз чтобы направить туда отряд, или щелкните по другому сектору для прокладки маршрута.",
	"Маршрут движения проложен.",
	"Маршрут не изменился.",
	"Маршрут отменен.",
	"Маршрут укорочен.",
};


// help text used when moving the merc arrival sector
static const ST::string s_rus_pBullseyeStrings[pBullseyeStrings_SIZE] =
{
	"Выберите сектор высадки прибывающих наемников.",
	"Теперь прибывающие наемники будут высаживаться в %s",
	"Сюда нельзя лететь, воздух контролирует враг!",
	"Отмена. Сектор прибытия тот же.",
	"Полет над %s опасен! Сектор прибытия перемещен в %s.",
};


// help text for mouse regions

static const ST::string s_rus_pMiscMapScreenMouseRegionHelpText[pMiscMapScreenMouseRegionHelpText_SIZE] =
{
	"Просмотр инвентаря (|E|n|t|e|r)",
	"Выкинуть предмет",
	"Выйти из инвентаря (|E|n|t|e|r)",
};


static const ST::string s_rus_str_he_leaves_where_drop_equipment  = "%s должен оставить снаряжение в (%s) или потом, в %s (%s), перед вылетом из Арулько?";
static const ST::string s_rus_str_she_leaves_where_drop_equipment = "%s должна оставить свое снаряжение в (%s) или потом, в %s (%s), перед вылетом из Арулько?";
static const ST::string s_rus_str_he_leaves_drops_equipment       = "%s собрался уйти, оставив свое снаряжение в %s.";
static const ST::string s_rus_str_she_leaves_drops_equipment      = "%s собралась уйти, оставив свое снаряжение в %s.";


// Text used on IMP Web Pages

static const ST::string s_rus_pImpPopUpStrings[pImpPopUpStrings_SIZE] =
{
	"Неверный код авторизации!",
	"Вы уверены, что хотите начать процесс профилирования заново?",
	"Укажите полное имя и пол",
	"Изучение вашего счета показало, что у вас нет средств для оплаты профилирования.",
	"Сейчас вы не можете это выбрать.",
	"Чтобы закончить анализ, нужно иметь место еще хотя бы для одного члена команды.",
	"Анализ уже пройден.",
};


// button labels used on the IMP site

static const ST::string s_rus_pImpButtonText[pImpButtonText_SIZE] =
{
	"Подробнее", 			// about the IMP site
	"НАЧАТЬ", 			// begin profiling
	"Личность", 		// personality section
	"Свойства", 		// personal stats/attributes section
	"Портрет", 			// the personal portrait selection
	"Голос %d", 			// the voice selection
	"Готово", 			// done profiling
	"Заново", 		// start over profiling
	"Да, выбрать выделенный ответ.",
	"Да",
	"Нет",
	"Закончить", 			// finished answering questions
	"Назад", 			// previous question..abbreviated form
	"Дальше", 			// next question
	"ДА.", 		// yes, I am certain
	"НЕТ, ХОЧУ НАЧАТЬ СНАЧАЛА.", // no, I want to start over the profiling process
	"ДА.",
	"НЕТ",
	"Назад", 			// back one page
	"Отмена", 			// cancel selection
	"Да, так и есть.",
	"Нет, просмотреть еще раз.",
	"Регистрация", 			// the IMP site registry..when name and gender is selected
	"Анализ", 			// analyzing your profile results
	"OK",
	"Голос",
	"Specialties",			// "Specialties" - the skills selection screen
};

static const ST::string s_rus_pExtraIMPStrings[pExtraIMPStrings_SIZE] =
{
	"Чтобы начать профилирование, протестируйте свои личные качества.",
	"Личные качества оценены, переходите к свойствам.",
	"Свойства зафиксированы, выберите наиболее подходящее вам лицо.",
	"Чтобы завершить процесс, выберите подходящий голос."
};

static const ST::string s_rus_gzIMPSkillTraitsText[gzIMPSkillTraitsText_SIZE] =
{
	s_rus_gzMercSkillText[1],
	s_rus_gzMercSkillText[2],
	s_rus_gzMercSkillText[3],
	s_rus_gzMercSkillText[4],
	s_rus_gzMercSkillText[5],
	s_rus_gzMercSkillText[6],
	s_rus_gzMercSkillText[7],
	s_rus_gzMercSkillText[8],
	s_rus_gzMercSkillText[9],
	s_rus_gzMercSkillText[10],
	s_rus_gzMercSkillText[13],
	s_rus_gzMercSkillText[14],
	s_rus_gzMercSkillText[15],
	s_rus_gzMercSkillText[12],

	s_rus_gzMercSkillText[0],
	"I.M.P. Specialties"
};

static const ST::string s_rus_pFilesTitle = "Просмотр файлов";

static const ST::string s_rus_pFilesSenderList[pFilesSenderList_SIZE] =
{
	"Отчет разведки",	// the recon report sent to the player. Recon is an abbreviation for reconissance
	"Перехват #1",		// first intercept file .. Intercept is the title of the organization sending the file...similar in function to INTERPOL/CIA/KGB..refer to fist record in files.txt for the translated title
	"Перехват #2",		// second intercept file
	"Перехват #3",		// third intercept file
	"Перехват #4",		// fourth intercept file
	"Перехват #5",		// fifth intercept file
	"Перехват #6",		// sixth intercept file
};

// Text having to do with the History Log
static const ST::string s_rus_pHistoryTitle = "События";

static const ST::string s_rus_pHistoryHeaders[pHistoryHeaders_SIZE] =
{
	"День", 			// the day the history event occurred
	"Стр.", 			// the current page in the history report we are in
	"День", 			// the days the history report occurs over
	"Место", 			// location (in sector) the event occurred
	"Событие", 			// the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
static const ST::string s_rus_pHistoryStrings[pHistoryStrings_SIZE] =
{
	"",																						// leave this line blank
	//1-5
	"%s нанят(а) в A.I.M.", 										// merc was hired from the aim site
	"%s нанят(а) в M.E.R.C.", 									// merc was hired from the aim site
	"%s мертв(а).", 															// merc was killed
	"Оплата счетов M.E.R.C.",								// paid outstanding bills at MERC
	"Принято задание от Энрико Чивалдори",
	//6-10
	"Прошел профилирование в IMP",
	"%s: жизнь застрахована", 				// insurance contract purchased
	"%s: страховка отменена", 				// insurance contract canceled
	"%s: страховка выплачена", 							// insurance claim payout for merc
	"%s: контракт продлен на день.", 						// Extented "mercs name"'s for a day
	//11-15
	"%s: контракт продлен на 7 дней", 					// Extented "mercs name"'s for a week
	"%s: контракт продлен на 14 дней", 					// Extented "mercs name"'s 2 weeks
	"%s уволен(а).", // "merc's name" was dismissed.
	"%s ушел.", // "merc's name" quit.
	"начат.", // a particular quest started
	//16-20
	"завершен.",
	"%s: состоялся разговор с начальником шахты.",	// talked to head miner of town
	"%s освобожден(а).",
	"Читы включены.",
	"Провизия завтра будет в Омерте.",
	//21-25
	"%s ушла из команды и вышла замуж за Хика",
	"%s: контракт окончен.",
	"%s в команде.",
	"Энрико недоволен отсутствием прогресса.",
	"Битва выиграна.",
	//26-30
	"%s: в шахте кончается руда",
	"%s: шахта выработана",
	"%s: шахта закрыта",
	"%s: шахта вновь открыта",
	"Получил сведения о тюрьме Тикса.",
	//31-35
	"Услышал о секретном военном заводе Орта.",
	"Ученый из Орты помог с ракетным ружьем.",
	"Дейдранна что-то делает с трупами.",
	"Фрэнк рассказал о поединках в Сан Моне.",
	"Пациент думает, что видел в шахтах что-то.",
	//36-40
	"Встретил какого-то Девина - торгует взрывчаткой.",
	"Столкнулся со знаменитым Майком!",
	"Встретил Тони - он занимается оружием.",
	"Получил ракетное ружье от сержанта Кротта.",
	"Передал Кайлу закладную на магазин Энжела.",
	//41-45
	"Шиз предлагает сделать робота.",
	"Болтун может сделать состав против жуков.",
	"Кейт больше не при делах.",
	"Говард обеспечивает Дейдрану цианидом.",
	"Встретил Кейта - барыжит всем подряд в Камбрии.",
	//46-50
	"Встретил Говарда - фармацевта из Балайма.",
	"Встретил Перко - у него маленький ремонтный бизнес.",
	"Встретил Сэма из Балайма, у него компьютерный магазин.",
	"Франц торгует электроникой и другими вещами.",
	"У Арнольда ремонтная мастерская в Граме.",
	//51-55
	"Фредо ремонтирует электронику в Граме.",
	"Получил пожертвование от богача из Балайма.",
	"Встретил старьевщика по имени Джейк.",
	"Бродяга дал нам электронный ключ.",
	"Подкупил Вальтера, чтобы он открыл дверь в подвал.",
	//56-60
	"Дэвид заправит нас даром, если у него есть бензин.",
	"Дал на лапу Пабло.",
	"Босс хранит деньги в шахте Сан Моны.",
	"%s выиграл кулачный бой",
	"%s проиграл кулачный бой",
	//61-65
	"%s: дисквалификация в кулачном бою",
	"Нашел много денег в заброшенной шахте.",
	"Столкнулся с убийцей, посланным Боссом.",
	"Утрачен контроль над сектором",				//ENEMY_INVASION_CODE
	"Успешная оборона сектора",
	//66-70
	"Поражение в бою",							//ENEMY_ENCOUNTER_CODE
	"Засада",						//ENEMY_AMBUSH_CODE
	"Победа над засадой врага",
	"Безуспешная атака",			//ENTERING_ENEMY_SECTOR_CODE
	"Успешная атака!",
	//71-75
	"Нападение существ!",			//CREATURE_ATTACK_CODE
	"Атака кошек-убийц",			//BLOODCAT_AMBUSH_CODE
	"Перебил кошек-убийц",
	"%s убит(а)",
	"Отдал голову террориста Кармену",
	"Убийца ушел",
	"Убил %s",
};

static const ST::string s_rus_pHistoryLocations = "Н/П"; // N/A is an acronym for Not Applicable

// icon text strings that appear on the laptop

static const ST::string s_rus_pLaptopIcons[pLaptopIcons_SIZE] =
{
	"Почта",
	"Сеть",
	"Финансы",
	"Команда",
	"Журнал",
	"Файлы",
	"Выключить",
	"сир-ФЕР 4.0",			// our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added

static const ST::string s_rus_pBookMarkStrings[pBookMarkStrings_SIZE] =
{
	"А.I.M.",
	"Бобби Рэй",
	"I.M.P.",
	"М.Е.R.С.",
	"Похороны",
	"Цветы",
	"Страховка",
	"Отмена",
};

// When loading or download a web page

static const ST::string s_rus_pDownloadString[pDownloadString_SIZE] =
{
	"Загрузка...",
	"Перегрузка...",
};

//This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

static const ST::string s_rus_gsAtmStartButtonText[gsAtmStartButtonText_SIZE] =
{
	"Параметры", 			// view stats of the merc
	"Снаряжение", 			// view the inventory of the merc
	"Контракт",
};

// Web error messages. Please use German equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator

static const ST::string s_rus_pErrorStrings = "Плохое соединение. Попробуйте позднее.";


static const ST::string s_rus_pPersonnelString = "Бойцов:"; // mercs we have


static const ST::string s_rus_pWebTitle = "сир-ФЕР 4.0"; // our name for the version of the browser, play on company name


// The titles for the web program title bar, for each page loaded

static const ST::string s_rus_pWebPagesTitles[pWebPagesTitles_SIZE] =
{
	"А.I.M.",
	"A.I.M. - Наемники",
	"A.I.M. - Фотографии ",		// a mug shot is another name for a portrait
	"A.I.M. - Сортировка",
	"A.I.M.",
	"A.I.M. - Бывшие члены",
	"A.I.M. - Правила",
	"A.I.M. - История",
	"A.I.M. - Ссылки",
	"M.E.R.C.",
	"M.E.R.C. - Счета",
	"M.E.R.C. - Регистрация",
	"M.E.R.C. - Индекс",
	"Бобби Рэй",
	"Бобби Рэй - Оружие",
	"Бобби Рэй - Боеприпасы",
	"Бобби Рэй - Броня",
	"Бобби Рэй - Разное",							//misc is an abbreviation for miscellaneous
	"Бобби Рэй - Бывшее в Употреблении",
	"Бобби Рэй - Бланк заказа",
	"I.M.P.",
	"I.M.P.",
	"Цветы по Всему Миру",
	"Цветы по Всему Миру - Галерея",
	"Цветы по Всему Миру - Бланк Заказа",
	"Цветы по Всему Миру - Открытки",
	"Страховые агенты Малеус, Инкус и Стэйпс",
	"Информация",
	"Контракт",
	"Комментарии",
	"Похоронное агенство Макгилликати",
	"URL не найден.",
	"Бобби Рэй - Последние поставки",//@@@3 Translate new text
	"",
	"",
};

static const ST::string s_rus_pShowBookmarkString[pShowBookmarkString_SIZE] =
{
	"Sir-Помощь",
	"Закладки: кликните на кнопке Сеть еще раз.",
};

static const ST::string s_rus_pLaptopTitles[pLaptopTitles_SIZE] =
{
	"Почтовый ящик",
	"Просмотр файлов",
	"Команда",
	"Бухгалтер Плюс",
	"Журнал",
};

static const ST::string s_rus_pPersonnelDepartedStateStrings[pPersonnelDepartedStateStrings_SIZE] =
{
	//reasons why a merc has left.
	"Убит в бою",
	"Уволен",
	"Замужем",///TRNSL ASSUME only female merc can depart from team by marrying somebody
	"Контракт истек",
	"Ушел",
};
// personnel strings appearing in the Personnel Manager on the laptop

static const ST::string s_rus_pPersonelTeamStrings[pPersonelTeamStrings_SIZE] =
{
	"В команде",
	"Убывшие",
	"Гонорар в день:",
	"Макс. гонорар:",
	"Мин. гонорар:",
	"Погибли в бою:",
	"Уволены:",
	"Другое:",
};


static const ST::string s_rus_pPersonnelCurrentTeamStatsStrings[pPersonnelCurrentTeamStatsStrings_SIZE] =
{
	"Минимум",
	"Среднее",
	"Максимум",
};


static const ST::string s_rus_pPersonnelTeamStatsStrings[pPersonnelTeamStatsStrings_SIZE] =
{
	"ЗДОР",
	"ПРВ",
	"ЛОВ",
	"СИЛ",
	"ЛДР",
	"МДР",
	"ОПЫТ",
	"МТК",
	"МЕХ",
	"ВЗРВ",
	"МЕД",
};


// horizontal and vertical indices on the map screen

static const ST::string s_rus_pMapVertIndex[pMapVertIndex_SIZE] =
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

static const ST::string s_rus_pMapHortIndex[pMapHortIndex_SIZE] =
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

static const ST::string s_rus_pMapDepthIndex[pMapDepthIndex_SIZE] =
{
	"",
	"-1",
	"-2",
	"-3",
};

// text that appears on the contract button

static const ST::string s_rus_pContractButtonString = "Контракт";

// text that appears on the update panel buttons

static const ST::string s_rus_pUpdatePanelButtons[pUpdatePanelButtons_SIZE] =
{
	"Дальше",
	"Стоп",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle

static const ST::string s_rus_LargeTacticalStr[LargeTacticalStr_SIZE] =
{
	"В этом секторе вам нанесли поражение!",
	"Враг, без тени сожаления, пожрет всех до единого!",
	"Члены вашей команды захвачены (без сознания)!",
	"Члены вашей команды захвачены в плен врагом.",
};


//Insurance Contract.c
//The text on the buttons at the bottom of the screen.

static const ST::string s_rus_InsContractText[InsContractText_SIZE] =
{
	"Назад",
	"Далее",
	"ОК",
	"Очистить",
};



//Insurance Info
// Text on the buttons on the bottom of the screen

static const ST::string s_rus_InsInfoText[InsInfoText_SIZE] =
{
	"Назад",
	"Далее"
};



//For use at the M.E.R.C. web site. Text relating to the player's account with MERC

static const ST::string s_rus_MercAccountText[MercAccountText_SIZE] =
{
	// Text on the buttons on the bottom of the screen
	"Оплатить",
	"Главная",
	"Счет #:",
	"Наемник",
	"Дни",
	"Ставка",	//5
	"Плата",
	"Всего:",
	"Подтверждаете выплату %s?",		//the %s is a string that contains the dollar amount ( ex. "$150" )
};



//For use at the M.E.R.C. web site. Text relating a MERC mercenary


static const ST::string s_rus_MercInfo[MercInfo_SIZE] =
{
	"Назад",
	"Нанять",
	"Дальше",
	"Дополнительная информация",
	"На главную",
	"В команде",
	"Зарплата:",
	"/день",
	"Мертв(а)",

	"Похоже, вы увлеклись набором наемников. Не более 18 бойцов в команде.",
	"Отсутствует",
};



// For use at the M.E.R.C. web site. Text relating to opening an account with MERC

static const ST::string s_rus_MercNoAccountText[MercNoAccountText_SIZE] =
{
	//Text on the buttons at the bottom of the screen
	"Открыть счет",
	"Отмена",
	"У вас нет счета. Хотите открыть?"
};



// For use at the M.E.R.C. web site. MERC Homepage

static const ST::string s_rus_MercHomePageText[MercHomePageText_SIZE] =
{
	//Description of various parts on the MERC page
	"Спек Т. Клайн, основатель",
	"Открытие счета",
	"Просмотр счета",
	"Личные дела",
	// The version number on the video conferencing system that pops up when Speck is talking
	"Спек Ком v3.2"
};

// For use at MiGillicutty's Web Page.

static const ST::string s_rus_sFuneralString[sFuneralString_SIZE] =
{
	"Похоронное агенство Макгилликати: скорбим вместе с семьями усопших с 1983 года.",
	"Директор по похоронам и бывший наемник А.I.М Мюррэй \"Папаша\" Макгилликати - специалист по части похорон.",
	"Всю жизнь Папашу сопровождали смерть и утраты, поэтому он, как никто, познал их тяжесть.",
	"Агенство Макгилликати предлагает широкий спектр похоронных услуг - от жилетки, в которую можно поплакать, до восстановления сильно поврежденных останков.",
	"Доверьтесь похоронному агенству Макгилликати, и ваши родственники почиют в мире.",

	// Text for the various links available at the bottom of the page
	"ДОСТАВКА ЦВЕТОВ",
	"КОЛЛЕКЦИЯ УРН И ГРОБОВ",
	"УСЛУГИ ПО КРЕМАЦИИ",
	"ПОМОЩЬ В ПРОВЕДЕНИИ ПОХОРОН",
	"ПОХОРОННЫЕ РИТУАЛЫ",

	// The text that comes up when you click on any of the links ( except for send flowers ).
	"К сожалению, наш сайт не закончен, в связи с утратой в семье. Мы постараемся продолжить работу после прочтения завещания и выплат долгов умершего.",
	"Однако, мы все равно скорбим вместе с вами. Пожалуйста, приходите еще."
};

// Text for the florist Home page

static const ST::string s_rus_sFloristText[sFloristText_SIZE] =
{
	//Text on the button on the bottom of the page

	"Галерея",

	//Address of United Florist

	"\"Авиадоставка по всему миру\"",
	"1-555-SCENT-ME",
	"333 Др. Носич, Семябург, КА США 90210",
	"http://www.scent-me.com",

	// detail of the florist page

	"Работаем быстро и эффективно!",
	"Гарантированная доставка в течение одного дня в любую точку земного шара. Есть ограничения.",
	"Самые низкие в мире цены!",
	"Покажите нам рекламу подобных услуг, которые стоят дешевле и получите букет роз бесплатно.",
	"Летающая Флора, Фауна & Цветы с 1981.",
	"Наши летчики, в прошлом военные бомбардировщики, сбросят ваш букет в радиусе 10 км от цели. В любое время! Всегда!",
	"Позвольте нам воплотить ваши цветочные фантазии в жизнь.",
	"Брюс, наш всемирно известный флорист, собственноручно соберет вам букет свежайших цветов из наших оранжерей.",
	"И помните, если у нас чего-то нет - мы это вырастим! И быстро!"
};



//Florist OrderForm

static const ST::string s_rus_sOrderFormText[sOrderFormText_SIZE] =
{
	//Text on the buttons

	"Назад",
	"Послать",
	"Очистить",
	"Галерея",

	"Букет:",
	"Цена:",			//5
	"Номер заказа:",
	"Срочность",
	"завтра",
	"дойдет, когда дойдет",
	"Место доставки",			//10
	"Доп. услуги",
	"Сломанные цветы($10)",
	"Черные розы($20)",
	"Увядший букет($10)",
	"Фруктовый пирог(если есть)($10)",		//15
	"Ваше пожелание:",
	"Ввиду небольшого размера открыток - не более 75 символов.",
	"...или выберите из наших",

	"СТАНДАРТНЫХ ПОЖЕЛАНИЙ",
	"Информация о дарителе",//20

	//The text that goes beside the area where the user can enter their name

	"Имя:",
};




//Florist Gallery.c

static const ST::string s_rus_sFloristGalleryText[sFloristGalleryText_SIZE] =
{
	//text on the buttons

	"Назад",	//abbreviation for previous
	"Далее",	//abbreviation for next

	"Кликните по желаемому букету.",
	"Примечание: можно заказать увядший или сломанный букет за дополнительные $10.",

	//text on the button

	"Главная",
};

//Florist Cards

static const ST::string s_rus_sFloristCards[sFloristCards_SIZE] =
{
	"Кликните для выбора",
	"Назад"
};



// Text for Bobby Ray's Mail Order Site

static const ST::string s_rus_BobbyROrderFormText[BobbyROrderFormText_SIZE] =
{
	"Бланк заказа",				//Title of the page
	"Шт.",					// The number of items ordered
	"Вес (%s)",			// The weight of the item
	"Наименование",				// The name of the item
	"Цена",				// the item's price
	"Всего",				//5	// The total price of all of items of the same type
	"Цена товаров",				// The sub total of all the item totals added
	"Доставка и упаковка",		// S&H is an acronym for Shipping and Handling
	"Цена с доставкой",			// The grand total of all item totals + the shipping and handling
	"Место доставки",
	"Тип доставки",			//10	// See below
	"Цена (за %s.)",			// The cost to ship the items
	"Доставка наутро",			// Gets deliverd the next day
	"2 рабочих дня",			// Gets delivered in 2 days
	"Стандартный срок",			// Gets delivered in 3 days
	"Очистить",//15			// Clears the order page
	"Принять заказ",			// Accept the order
	"Назад",				// text on the button that returns to the previous page
	"Главная",				// Text on the button that returns to the home page
	"* обозначает товары Б/У",		// Disclaimer stating that the item is used
	"У вас нет на это средств.",		//20	// A popup message that to warn of not enough money
	"<ПУСТО>",				// Gets displayed when there is no valid city selected
	"Место доставки заказа %s. Уверены?",		// A popup that asks if the city selected is the correct one
	"Вес товаров**",			// Displays the weight of the package
	"** минимальный вес заказа",				// Disclaimer states that there is a minimum weight for the package
	"Заказы",
};


// This text is used when on the various Bobby Ray Web site pages that sell items

static const ST::string s_rus_BobbyRText[BobbyRText_SIZE] =
{
	"Правила:",				// Title
	// instructions on how to order
	"Кликните на предмет. Хотите больше - кликайте еще. Убрать предмет из заказа - правый клик. Закончив выбор покупок, заполните бланк заказа.",

	//Text on the buttons to go the various links

	"Назад",		//
	"Оружие", 			//3
	"Амуниция",			//4
	"Броня",			//5
	"Разное",			//6	//misc is an abbreviation for miscellaneous
	"Б/У",			//7
	"Дальше",
	"ЗАКАЗАТЬ",
	"Главная",			//10

	//The following lines provide information on the items

	"Вес:",		// Weight of all the items of the same type
	"Калибр:",			// the caliber of the gun
	"Магаз.:",			// number of rounds of ammo the Magazine can hold
	"Дальноб.:",			// The range of the gun
	"Урон:",			// Damage of the weapon
	"Скор.:",			// Weapon's Rate Of Fire, acronym ROF
	"Цена:",			// Cost of the item
	"Склад:",		// The number of items still in the store's inventory
	"Заказано, шт:",		// The number of items on order
	"Повреждение",			// If the item is damaged
	"Итого:",			// The total cost of all items on order
	"* процент исправности",		// if the item is damaged, displays the percent function of the item

	//Popup that tells the player that they can only order 10 items at a time

	"Дорогие клиенты! Онлайновая форма позволяет заказать не более 10 позиций за раз. Если вы хотите заказать больше, (а мы надемся, что так и есть), заполните еще один бланк и примите наши извинения.",

	// A popup that tells the user that they are trying to order more items then the store has in stock

	"Извините. Этот товар закончился на складе. Попробуйте заказать его позже.",

	//A popup that tells the user that the store is temporarily sold out

	"Извините, но все товары этого типа закончились на складе.",
};


// The following line is used on the Ammunition page.  It is used for help text
// to display how many items the player's merc has that can use this type of
// ammo.
static const ST::string s_rus_str_bobbyr_guns_num_guns_that_use_ammo = "Количество единиц оружия у вашей команды, использующих этот боеприпас: %d";


// Text for Bobby Ray's Home Page

static const ST::string s_rus_BobbyRaysFrontText[BobbyRaysFrontText_SIZE] =
{
	//Details on the web site

	"Только у нас можно приобрести последние новинки в оружейном деле",
	"Прекрасный выбор взрывчатки и аксессуаров на любой вкус",
	"Б/У",

	//Text for the various links to the sub pages

	"РАЗНОЕ",
	"ОРУЖИЕ",
	"АМУНИЦИЯ",		//5
	"БРОНЯ",

	//Details on the web site

	"Нет у нас - нет нигде!",
	"В разработке",
};



// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug shot page

static const ST::string s_rus_AimSortText[AimSortText_SIZE] =
{
	"Наемники А.I.M.",				// Title
	// Title for the way to sort
	"Сортировка:",

	//Text of the links to other AIM pages

	"Просмотреть фото наемников",
	"Просмотреть характеристики наемников",
	"Просмотреть галерею бывших членов А.I.M."
};


// text to display how the entries will be sorted
static const ST::string s_rus_str_aim_sort_price        = "Цена";
static const ST::string s_rus_str_aim_sort_experience   = "Опытность";
static const ST::string s_rus_str_aim_sort_marksmanship = "Меткость";
static const ST::string s_rus_str_aim_sort_medical      = "Медицина";
static const ST::string s_rus_str_aim_sort_explosives   = "Взрывчатка";
static const ST::string s_rus_str_aim_sort_mechanical   = "Механика";
static const ST::string s_rus_str_aim_sort_ascending    = "по возрастанию";
static const ST::string s_rus_str_aim_sort_descending   = "по убыванию   ";


//Aim Policies.c
//The page in which the AIM policies and regulations are displayed

static const ST::string s_rus_AimPolicyText[AimPolicyText_SIZE] =
{
	// The text on the buttons at the bottom of the page

	"Назад",
	"Главная",
	"Правила",
	"Дальше",
	"Не согласен",
	"Согласен"
};



//Aim Member.c
//The page in which the players hires AIM mercenaries

// Instructions to the user to either start video conferencing with the merc, or to go the mug shot index

static const ST::string s_rus_AimMemberText[AimMemberText_SIZE] =
{
	"Левый щелчок",
	"связаться",
	"Правый щелчок",
	"назад к фото",
};

//Aim Member.c
//The page in which the players hires AIM mercenaries

static const ST::string s_rus_CharacterInfo[CharacterInfo_SIZE] =
{
	// the contract expenses' area

	"Плата",
	"Срок",
	"1 день",
	"7 дней",
	"14 дней",

	// text for the buttons that either go to the previous merc,
	// start talking to the merc, or go to the next merc

	"<<<<",
	"Связь",
	">>>>",

	"Дополнительная информация",				// Title for the additional info for the merc's bio
	"Действующие наемники A.I.M.", // Title of the page
	"Цена снаряжения:",				// Displays the optional gear cost
	"медицинский депозит",			// If the merc required a medical deposit, this is displayed
};


//Aim Member.c
//The page in which the player's hires AIM mercenaries

//The following text is used with the video conference popup

static const ST::string s_rus_VideoConfercingText[VideoConfercingText_SIZE] =
{
	"Цена контракта:",				//Title beside the cost of hiring the merc

	//Text on the buttons to select the length of time the merc can be hired

	"1 день",
	"7 дней",
	"14 дней",

	//Text on the buttons to determine if you want the merc to come with the equipment

	"Без снаряжения",
	"Со снаряжением",

	// Text on the Buttons

	"ОПЛАТИТЬ",			// to actually hire the merc
	"ОТМЕНА",				// go back to the previous menu
	"НАНЯТЬ",				// go to menu in which you can hire the merc
	"ОТБОЙ",				// stops talking with the merc
	"OK",
	"СООБЩЕНИЕ",			// if the merc is not there, you can leave a message

	//Text on the top of the video conference popup

	"Видеоконференция, абонент",
	"Соединение. . .",

	"с депозитом"			// Displays if you are hiring the merc with the medical deposit
};



//Aim Member.c
//The page in which the player hires AIM mercenaries

// The text that pops up when you select the TRANSFER FUNDS button

static const ST::string s_rus_AimPopUpText[AimPopUpText_SIZE] =
{
	"ЭЛЕКТРОННЫЙ ПЛАТЕЖ ПРОШЕЛ УСПЕШНО",	// You hired the merc
	"ОШИБКА! НЕТ",		// Player doesn't have enough money, message 1
	"СРЕДСТВ НА СЧЕТУ!",				// Player doesn't have enough money, message 2

	// if the merc is not available, one of the following is displayed over the merc's face

	"На задании",
	"Оставьте сообщение",
	"Скончался",

	//If you try to hire more mercs than game can support

	"У вас уже набрано 18 наемников - полная команда.",

	"АВТО ОТВЕТЧИК",
	"Сообщение записано",
};


//AIM Link.c

static const ST::string s_rus_AimLinkText = "Ссылки A.I.M.";	// The title of the AIM links page



//Aim History

// This page displays the history of AIM

static const ST::string s_rus_AimHistoryText[AimHistoryText_SIZE] =
{
	"История A.I.M.",					//Title

	// Text on the buttons at the bottom of the page

	"Назад",
	"Главная",
	"Галерея",
	"Дальше"
};


//Aim Mug Shot Index

//The page in which all the AIM members' portraits are displayed in the order selected by the AIM sort page.

static const ST::string s_rus_AimFiText[AimFiText_SIZE] =
{
	// displays the way in which the mercs were sorted

	"цены",
	"опытности",
	"меткости",
	"медицины",
	"саперных навыков",
	"навыков механика",

	// The title of the page, the above text gets added at the end of this text

	"Список наемников по возрастанию %s",
	"Список наемников по убыванию %s",

	// Instructions to the players on what to do

	"Левый щелчок",
	"выбор наемника",			//10
	"Правый щелчок",
	"настройка сортировки",

	// Gets displayed on top of the merc's portrait if they are...

	"Скончался",						//14
	"На задании",
};



//AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer with AIM

static const ST::string s_rus_AimAlumniText[AimAlumniText_SIZE] =
{
	// Text of the buttons

	"СТР 1",
	"СТР 2",
	"СТР 3",

	"Галерея бывших членов A.I.M.",	// Title of the page //$$

	"ОК"			// Stops displaying information on selected merc
};






//AIM Home Page

static const ST::string s_rus_AimScreenText[AimScreenText_SIZE] =
{
	// AIM disclaimers

	"A.I.M. и логотип A.I.M. - зарегистрированные во многих странах торговые марки.",
	"Поэтому даже и не думайте нас копировать.",
	"Копирайт 1998-1999 A.I.M.,Ltd. Все права защищены.",

	//Text for an advertisement that gets displayed on the AIM page

	"Цветы по Всему Миру",
	"\"Авиадоставка куда угодно\"",				//10
	"Сделай как надо",
	"... первый раз",
	"Если у нас чего-то нет - оно вам и не нужно.",
};


//Aim Home Page

static const ST::string s_rus_AimBottomMenuText[AimBottomMenuText_SIZE] =
{
	//Text for the links at the bottom of all AIM pages
	"Главная",
	"Наемники",
	"Галерея", //$$
	"Правила",
	"История",
	"Ссылки"
};



//ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.

static const ST::string s_rus_SKI_Text[SKI_SIZE ] =
{
	"ТОВАРЫ В ПРОДАЖЕ",		//Header for the merchandise available
	"СТР",				//The current store inventory page being displayed
	"ОБЩАЯ ЦЕНА",				//The total cost of the the items in the Dealer inventory area
	"ОБЩАЯ ЦЕНА",			//The total value of items player wishes to sell
	"ОЦЕНКА",				//Button text for dealer to evaluate items the player wants to sell
	"ПО РУКАМ",			//Button text which completes the deal. Makes the transaction.
	"ГОТОВО",				//Text for the button which will leave the shopkeeper interface.
	"ЦЕНА РЕМОНТА",			//The amount the dealer will charge to repair the merc's goods
	"1 ЧАС",			// SINGULAR! The text underneath the inventory slot when an item is given to the dealer to be repaired
	"%d ЧАСОВ",		// PLURAL!   The text underneath the inventory slot when an item is given to the dealer to be repaired
	"ОТРЕМОНТИРОВАНО",		// Text appearing over an item that has just been repaired by a NPC repairman dealer
	"Вам уже некуда класть вещи.",	//Message box that tells the user there is no more room to put there stuff
	"%d МИНУТ",		// The text underneath the inventory slot when an item is given to the dealer to be repaired
	"Бросить на землю", //hint about dropping item on the ground from ShopKeeper screen
};


static const ST::string s_rus_SkiMessageBoxText[SkiMessageBoxText_SIZE] =
{
	"Cнять %s со своего основного счета, чтобы покрыть разницу?",
	"Недостаточно денег. Не хватает %s",
	"Cнять %s со своего основного счета, чтобы оплатить покупку?",
	"Предложить торговцу совершить сделку",
	"Попросить торговца починить выбранные вещи",
	"Закончить разговор",
	"Текущий баланс",
};


//OptionScreen.c

static const ST::string s_rus_zOptionsText[zOptionsText_SIZE] =
{
	//button Text
	"Сохранить",
	"Загрузить",
	"Выход",
	"Готово",

	//Text above the slider bars
	"Звуки",
	"Речь",
	"Музыка",

	//Confirmation pop when the user selects..
	"Выйти из игры и вернуться в главное меню?",

	"Нужно выбрать либо РЕЧЬ, либо СУБТИТРЫ.",
};


//SaveLoadScreen
static const ST::string s_rus_zSaveLoadText[zSaveLoadText_SIZE] =
{
	"Сохранить",
	"Загрузить",
	"Отмена",
	"Сохранить выбранную.", //string arent used in game
	"Загрузить выбранную", //string arent used in game

	"Игра сохранена",
	"Ошибка при сохранении!",
	"Игра загружена",
	"Ошибка при загрузке: \"%s\"",

	"Версия сохраненной игры отличается от текущей. Скорее всего, игра загрузится нормально. Загрузить?",
	"Файлы сохраненной игры возможно испорчены. Стереть все?",

	"Попытка загрузки старой версии сохранения. Обновить автоматически и загрузить?",

	"Попытка загрузки старой версии сохранения. Обновить автоматически и загрузить?",

	"Вы уверены, что хотите перезаписать сохраненную игру #%d?",


	//The first %d is a number that contains the amount of free space on the users hard drive,
	//the second is the recommended amount of free space.
	"У вас кончается дисковое пространство. Осталось всего %d Mбайт. Нужно как минимум %d свободных Mбайт.",

	"Сохранение...",			//When saving a game, a message box with this string appears on the screen

	"Оружие: реализм",
	"Оружие: тонны",
	"Стиль: реализм",
	"Стиль: фантаст.",

	"",
};



//MapScreen
static const ST::string s_rus_zMarksMapScreenText[zMarksMapScreenText_SIZE] =
{
	"Уровень карты",
	"У вас нет ополчения. Подготовьте его из гражданского населения.",
	"Доход в день",
	"Жизнь наемника застрахована",
	"%s не устал(а).",
	"%s движется и спать не может",
	"%s слишком устал(а), попробуйте позже.",
	"%s за рулем.",
	"Отряд не может двигаться, пока кто-то из бойцов спит.",

	// stuff for contracts
	"Вы можете платить по контракту, но у вас нет денег на оплату страхования жизни.",
	"%s застрахован(а) на случай гибели. Для продления страховки надо доплатить %s за %d дней. Согласны?",
	"Предметы в секторе",
	"У наемника есть медицинский депозит",

	// other items
	"Медики", // people acting a field medics and bandaging wounded mercs
	"Пациенты", // people who are being bandaged by a medic
	"Готово", // Continue on with the game after autobandage is complete
	"Стоп", // Stop autobandaging of patients by medics now
	"%s не может этого сделать, нет инструментов для ремонта.",
	"%s не может этого сделать, нет медицинских принадлежностей.",
	"Недостаточно желающих пройти подготовку.",
	"%s: слишком много ополченцев.",
	"У наемника ограниченный по времени контракт",
	"У наемника нет страховки",
};


static const ST::string s_rus_pLandMarkInSectorString = "Отряд %d заметил кого-то в секторе %s";

// confirm the player wants to pay X dollars to build a militia force in town
static const ST::string s_rus_pMilitiaConfirmStrings[pMilitiaConfirmStrings_SIZE] =
{
	"Тренировка отряда ополчения будет стоить $", // telling player how much it will cost
	"Оплатить расходы?", // asking player if they wish to pay the amount requested
	"У вас нет такой суммы.", // telling the player they can't afford to train this town
	"Продолжить тренировку ополчения в %s (%s %d)?", // continue training this town?
	"Стоимость $", // the cost in dollars to train militia
	"(Д/Н)",   // abbreviated yes/no
	"Тренировка ополчения в %d секторах будет стоить $ %d. %s", // cost to train sveral sectors at once
	"У вас нет $%d на тренировку здесь ополчения.",
	"%s: нужно %d%% отношения к вам, чтобы продолжить тренировку ополчения.",
	"%s: невозможно продолжить подготовку ополчения.",
};


//Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom of the single merc panel
static const ST::string s_rus_gzMoneyWithdrawMessageText[gzMoneyWithdrawMessageText_SIZE] =
{
	"За один раз можно снять со счета не более $20,000.",
	"Вы уверены, что хотите положить %s на свой счет?",
};

static const ST::string s_rus_gzCopyrightText =
	"Авторские права(C) 1999 Sir-Tech Canada Ltd. Все права защищены. Распространение на территории стран СНГ компания БУКА.";

static const ST::string s_rusGold_gzCopyrightText =
	"(C) 2002 Strategy First и компания Акелла. Все права защищены.";

//option Text
static const ST::string s_rus_zOptionsToggleText[zOptionsToggleText_SIZE] =
{
	"Речь",
	"Заглушить отзывы",
	"Субтитры",
	"Диалоги с паузами",
	"Анимированный дым",
	"Кровища",
	"Не трожь мою мышь!",
	"Старый способ выбора",
	"Показывать путь",
	"Показывать промахи",
	"Подтверждение движения",
	"Оповещение о сне/бодрствовании",
	"Метрическая система мер",
	"Подсветка земли",
	"Переводить курсор на наемников",
	"Переводить курсор на двери",
	"Подсветка вещей",
	"Показывать верхушки деревьев",
	"Показывать каркасы",
	"Трехмерный курсор",
};

//This is the help text associated with the above toggles.
static const ST::string s_rus_zOptionsScreenHelpText[zOptionsToggleText_SIZE] =
{
	//speech
	"Включите эту опцию, если хотите слышать диалоги персонажей.",

	//Mute Confirmation
	"Включите эту опцию, чтобы заглушить отзывы на команды.",
	//Subtitles
	"Показывать тексты диалогов.",

	//Key to advance speech
	"Включите эту опцию, чтобы субтитры переходили к следующей фразе по нажатию клавиши.",

	//Toggle smoke animation
	"Отключите эту опцию, если анимированный дым тормозит игру.",

	//Blood n Gore
	"Отключите эту опцию, если вам не нравится вид крови.",

	//Never move my mouse
	"Отключите эту опцию, чтобы курсор автоматически перемещался к всплывающим окнам.",

	//Old selection method
	"Включает выбор персонажей, как в Jagged Alliance 1 (иначе выбор происходит наоборот).",

	//Show movement path
	"Включает отображение пути бойца в реальном времени (или отключите эту опцию и используйте SHIFT).",

	//show misses
	"Включите эту опцию, чтобы видеть, куда попадают пули при \"промахе\".",

	//Sleep/Wake notification
	"При включенной опции для движения в реальном времени необходим дополнительный клик мышью.",

	//Display the enemy indicator
	"Включает отображение оповещений о том, что ваш боец засыпает или просыпается.",

	//Use the metric system
	"Когда опция включена, игра использует метрическую систему мер, иначе - британскую.",

	//Merc Lighted movement
	"Когда опция включена, боец \"освещает\" землю при движении. Отключите для улучшения производительности.",

	//Smart cursor
	"Когда опция включена, бойцы выбираются, если курсор рядом с ними.",

	//snap cursor to the door
	"Когда опция включена, курсор перескакивает на дверь, если он рядом с ней.",

	//glow items
	"Включает постоянную подсветку предметов на земле (|I).",

	//toggle tree tops
	"Включает отображение верхушек деревьев (|Е).",

	//toggle wireframe
	"Включает отображение \"каркасов\" домов (|W).",

	"Включает \"трехмерный\" курсор движения (|H|o|m|e).",

};


static const ST::string s_rus_gzGIOScreenText[gzGIOScreenText_SIZE] =
{
	"ИГРОВЫЕ НАСТРОЙКИ",
	"Стиль игры",
	"Реалистичный",
	"Фантастичный",
	"Количество оружия",
	"Сотни стволов",
	"Как в жизни",
	"Уровень сложности",
	"НОВОБРАНЕЦ",
	"СОЛДАТ",
	"ВЕТЕРАН",
	"Ok",
	"Отмена",
	"Дополнительная сложность",
	"Сохранение где угодно",
	"СТАЛЬНАЯ ВОЛЯ",
	"Dead is Dead"
};

// This string is used in the IMP character generation.  It is possible to
// select 0 ability in a skill meaning you can't use it.  This text is
// confirmation to the player.
static const ST::string s_rus_pSkillAtZeroWarning    = "Уверены? Ноль означает отсутствие навыка.";
static const ST::string s_rus_pIMPBeginScreenStrings = "(не более 8 букв)";
static const ST::string s_rus_pIMPFinishButtonText   = "Анализ";
static const ST::string s_rus_pIMPFinishStrings      = "Спасибо, %s"; //%s is the name of the merc
static const ST::string s_rus_pIMPVoicesStrings      = "Голос"; // the strings for imp voices screen

// title for program
static const ST::string s_rus_pPersTitleText = "Кадры";

// paused game strings
static const ST::string s_rus_pPausedGameText[pPausedGameText_SIZE] =
{
	"Пауза",
	"Снять с паузы (|P|a|u|s|e)",
	"Поставить на паузу (|P|a|u|s|e)",
};


static const ST::string s_rus_pMessageStrings[pMessageStrings_SIZE] =
{
	"Выйти из игры?",
	"OK",
	"ДА",
	"НЕТ",
	"ОТМЕНА",
	"НАНЯТЬ",
	"ЛОЖЬ",
	"Нет описания", //Save slots that don't have a description.
	"Игра сохранена",
	"День",
	"Наемников",
	"Пустая ячейка сохранения", 		//An empty save game slot
	"в/м",					//Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
	"мин",					//Abbreviation for minute.
	"м",						//One character abbreviation for meter (metric distance measurement unit).
	"пуль",				//Abbreviation for rounds (# of bullets)
	"кг",					//Abbreviation for kilogram (metric weight measurement unit)
	"ф",					//Abbreviation for pounds (Imperial weight measurement unit)
	"Главная страница",	//Home as in homepage on the internet. Used on M.I.S. insurance web page
	"USD",					//Abbreviation to US dollars
	"н/п",					//Lowercase acronym for not applicable.
	"Тем временем",		//Meanwhile
	"%s теперь в секторе %s%s", //Name/Squad has arrived in sector A9.  Order must not change without notifying
																		//SirTech
	"Версия",
	"Создать новое сохранение",
	"Эта ячейка предназначена для быстрого сохранения игры (с помощью ALT+S).",
	"Открытая", //doors definitions
	"Закрытая", //doors definitions
	"У вас кончается дисковое пространство. У вас осталось %sМБ свободных, а для игры требуется %sMБ.",
	"%s ловит %s.",		//'Merc name' has caught 'item' -- let SirTech know if name comes after item.
	"%s принимает препарат.", //'Merc name' has taken the drug
	"%s не имеет медицинских навыков",//'Merc name' has no medical skill.

	//CDRom errors (such as ejecting CD while attempting to read the CD)
	"Нарушена целостность программы.",
	"ОШИБКА: Вынут CD-ROM",

	//When firing heavier weapons in close quarters, you may not have enough room to do so.
	"Мало места для стрельбы.",

	//Can't change stance due to objects in the way...
	"Сейчас изменить положение нельзя.",

	//Simple text indications that appear in the game, when the merc can do one of these things.
	"Уронить",
	"Бросить",
	"Передать",

	"%s передан %s.",	//"Item" passed to "merc".  Please try to keep the item %s before the merc %s,
				//otherwise, must notify SirTech.
	"Нельзя передать %s %s.", //pass "item" to "merc".  Same instructions as above.

	//A list of attachments appear after the items.  Ex:  Kevlar vest ( Ceramic Plate 'Attached )'
	" присоединен)",

	//Cheat modes
	"Достигнут чит-уровень один",
	"Достигнут чит-уровень два",

	//Toggling various stealth modes
	"Отряд в режиме скрытности.",
	"Отряд в обычном режиме.",
	"%s в режиме скрытности.",
	"%s в обычном режиме.",

	//Wireframes are shown through buildings to reveal doors and windows that can't otherwise be seen in
	//an isometric engine.  You can toggle this mode freely in the game.
	"Отображение каркасов зданий включено",
	"Отображение каркасов зданий выключено",

	//These are used in the cheat modes for changing levels in the game.  Going from a basement level to
	//an upper level, etc.
	"Нельзя подняться с этого уровня...",
	"Ниже уровней нет...",
	"Входим в подвальный уровень %d...",
	"Уходим из подвала...",

	".",		// used in the shop keeper inteface to mark the ownership of the item eg Red's gun
	"Режим слежения выключен.",
	"Режим слежения включен.",
	"Курсор трехмерный.",
	"Курсор обычный.",
	"Отряд %d действует.",
	"У вас нет денег, чтобы ежедневно выплачивать %s %s",	//first %s is the mercs name, the seconds is a string containing the salary
	"Пропуск",
	"%s не может идти в одиночку.",
	"Игра была сохранена под именем SaveGame99.sav. Для загрузки переименуйте ее в SaveGame01-SaveGame10.",
	"%s пъет %s",
	"Груз снаряжения прибыл в Драссен.",
	"%s прибудет в указанное место высадки (сектор %s) в день %d, примерно в %s.",		//first %s is mercs name, next is the sector location and name where they will be arriving in, lastely is the day an the time of arrival
	"Журнал событий обновлен.",
};


static const ST::string s_rus_ItemPickupHelpPopup[ItemPickupHelpPopup_SIZE] =
{
	"Взять выделенные",
	"Листать вверх",
	"Выделить все",
	"Листать вниз",
	"Отмена"
};

static const ST::string s_rus_pDoctorWarningString[pDoctorWarningString_SIZE] =
{
	"%s слишком далеко для лечения.",
	"Медики не смогли перевязать всех.",
};

static const ST::string s_rus_pMilitiaButtonsHelpText[pMilitiaButtonsHelpText_SIZE] =
{
	"Взять новобранцев (Правый клик)/Распределить (Левый клик)", // button help text informing player they can pick up or drop militia with this button
	"Взять солдат (Правый клик)/Распределить (Левый клик)",
	"Взять ветеранов (Правый клик)/Распределить (Левый клик)",
	"Поровну распределить доступное ополчение по всем секторам",
};

// to inform the player to hire some mercs to get things going
static const ST::string s_rus_pMapScreenJustStartedHelpText = "Сходите на сайт АIM и наймите наемников (*Подсказка* через ноутбук)";

static const ST::string s_rus_pAntiHackerString = "Ошибка. Испорченные или отсутствующие файлы. Игра завершает работу.";


static const ST::string s_rus_gzLaptopHelpText[gzLaptopHelpText_SIZE] =
{
	//Buttons:
	"Просмотреть электроные письма",
	"Посетить Интернет сайты",
	"Просмотреть файлы",
	"Просмотреть журнал событий",
	"Информация о команде",
	"Просмотреть финансовые отчеты",
	"Выключить лэптоп",

	//Bottom task bar icons (if they exist):
	"Получена новая почта",
	"Получены новые файлы",

	//Bookmarks:
	"Международная Ассоциация Наемников",
	"Бобби Рэй - заказ оружия в сети",
	"Институт Психологии Наемников",
	"M.E.R.C - Экономичный Рекрутинговый Центр",
	"Похоронное агенство Макгилликати",
	"Цветы по Всему Миру",
	"Страховые агенты по контрактам A.I.M.",
};


static const ST::string s_rus_gzHelpScreenText = "Закрыть помощь";

static const ST::string s_rus_gzNonPersistantPBIText[gzNonPersistantPBIText_SIZE] =
{
	"Идет бой. Отступить можно только с тактической карты",
	"Войти в сектор, чтобы продолжить бой (|E)",
	"Продолжить бой в автоматическом режиме (|A)",
	"Нельзя провести авто-бой, когда вы нападаете",
	"Нельзя провести авто-бой, когда вы попали в засаду",
	"Нельзя провести авто-бой, когда вы сражаетесь с существами в шахтах",
	"Нельзя провести авто-бой, если рядом враждебные жители",
	"Нельзя провести авто-бой, если рядом кошки-убийцы",
	"ИДЕТ БОЙ",
	"Сейчас отступить нельзя",
};

static const ST::string s_rus_gzMiscString[gzMiscString_SIZE] =
{
	"Ваше ополчение сражается без помощи наемников...",
	"Транспорт полностью заправлен.",
	"Бензобак заправлен на %d%%.",
	"Армия Дейдраны полностью захватила %s.",
	"Вы потеряли заправку.",
};

static const ST::string s_rus_gzIntroScreen = "Невозможно найти вступительный ролик";

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
static const ST::string s_rus_pNewNoiseStr[pNewNoiseStr_SIZE] =
{
	"%s слышит %s звук, идущий %s.",
	"%s слышит %s звук ДВИЖЕНИЯ, идущий %s.",
	"%s слышит %s СКРИП, идущий %s.",
	"%s слышит %s ПЛЕСК, идущий %s.",
	"%s слышит %s УДАР, идущий %s.", //$$
	"%s слышит %s ВЗРЫВ %s.",
	"%s слышит %s КРИК %s.",
	"%s слышит %s УДАР %s.",
	"%s слышит %s УДАР %s.",
	"%s слышит %s ЗВОН, идущий %s.",
	"%s слышит %s ГРОХОТ, идущий %s.",
};

static const ST::string s_rus_wMapScreenSortButtonHelpText[wMapScreenSortButtonHelpText_SIZE] =
{
	"Сортировка по имени (|F|1)",
	"Сортировка по заданию (|F|2)",
	"Сортировка по сну/бодрствованию (|F|3)",
	"Сортировка по месту (|F|4)",
	"Сортировка по цели (|F|5)",
	"Сортировка по времени контракта (|F|6)",
};



static const ST::string s_rus_BrokenLinkText[BrokenLinkText_SIZE] =
{
	"Ошибка 404",
	"URL не найден.",
};


static const ST::string s_rus_gzBobbyRShipmentText[gzBobbyRShipmentText_SIZE] =
{
	"Последние заказы",
	"Заказ #",
	"Предметов",
	"Дата",
};


static const ST::string s_rus_gzCreditNames[gzCreditNames_SIZE]=
{
	"Chris Camfield",
	"Shaun Lyng",
	"Kris Mornes",
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


static const ST::string s_rus_gzCreditNameTitle[gzCreditNameTitle_SIZE]=
{
	"Ведущий программист игры", 			// Chris Camfield !!!
	"Дизайн/Сценарий",							// Shaun Lyng
	"Программист стратегической части и редактора",					//Kris Marnes
	"Продюсер/Дизайн",						// Ian Currie
	"Дизайн/Дизайн карт",				// Linda Currie
	"Художник",													// Eric \"WTF\" Cheng
	"Бета-координатор, техподдержка",				// Lynn Holowka
	"Художник от Бога",						// Norman \"NRG\" Olsen
	"Гуру звука",											// George Brooks
	"Дизайн экранов/Художник",					// Andrew Stacey
	"Главный художник/Аниматор",						// Scot Loving
	"Главный программист",									// Andrew \"Big Cheese Doddle\" Emmons
	"Программист",											// Dave French
	"Программист стратегической части и баланса игры",					// Alex Meduna
	"Художник по портретам",								// Joey \"Joeker\" Whelan",
};

static const ST::string s_rus_gzCreditNameFunny[gzCreditNameFunny_SIZE]=
{
	"", 																			// Chris Camfield
	"(все еще зубрит правила пунктуации)",					// Shaun Lyng
	"(\"Оно работает. Я просто чиню\")",	//Kris \"The Cow Rape Man\" Marnes
	"(слишком стар для всего этого)",				// Ian Currie
	"(работает и над Wizardry 8)",						// Linda Currie
	"(принужденный заниматься тестированием)",			// Eric \"WTF\" Cheng
	"(Ушла от нас в CFSA - вот тебе на...)",	// Lynn Holowka
	"",																			// Norman \"NRG\" Olsen
	"",																			// George Brooks
	"(Мертвая Голова и любитель джаза)",						// Andrew Stacey
	"(его настоящее имя Роберт)",							// Scot Loving
	"(единственное ответственное лицо)",					// Andrew \"Big Cheese Doddle\" Emmons
	"(может опять заняться мотогонками)",	// Dave French
	"(украден с работы над Wizardry 8)",							// Alex Meduna
	"(делал предметы и загрузочные экраны!)",	// Joey \"Joeker\" Whelan",
};

static const ST::string s_rus_sRepairsDoneString[sRepairsDoneString_SIZE] =
{
	"%s отремонтировал(а) свои вещи",
	"%s отремонтировал(а) оружие и броню команды",
	"%s отремонтировал(а) снаряжение команды",
	"%s отремонтировал(а) транспортируемые командой вещи",
};

static const ST::string s_rus_zGioDifConfirmText[zGioDifConfirmText_SIZE]=
{
	"Вы выбрали сложность НОВОБРАНЕЦ. Это подходит для новичков в Jagged Alliance, для новичков в жанре стратегий вообще, или для тех, кто желает сократить битвы в игре. Ваш выбор скажется на игре в целом, так что выбирайте с умом. Уверены, что хотите играть новобранцем?",
	"Вы выбрали сложность СОЛДАТ. Это подходит для игроков, уже знакомых с играми Jagged Alliance или с подобными им. Ваш выбор скажется на игре в целом, так что выбирайте с умом. Уверены, что хотите играть солдатом?",
	"Вы выбрали сложность ВЕТЕРАН. Мы Вас предупредили. Нечего на нас пенять, если вас доставят назад в цинковом гробу. Ваш выбор скажется на игре в целом, так что выбирайте с умом. Уверены, что хотите играть ветераном?",
};

static const ST::string s_rus_gzLateLocalizedString[gzLateLocalizedString_SIZE] =
{
	//1-5
	"Робот не может покинуть сектор, если им никто не управляет.",

	//This message comes up if you have pending bombs waiting to explode in tactical.
	"Сейчас ускорять время нельзя. Подождите фейерверка!",

	//'Name' refuses to move.
	"%s отказывается двигаться.",

	//%s a merc name
	"%s без сил и не может поменять положение.",

	//A message that pops up when a vehicle runs out of gas.
	"У %sа кончилось топливо и он остается в %c%d.",///TRNSL Assumes that all vehicle names are male gender words

	//6-10

	// the following two strings are combined with the pNewNoise[] strings above to report noises
	// heard above or below the merc
	"сверху",
	"снизу",

	//The following strings are used in autoresolve for autobandaging related feedback.
	"Никто из ваших бойцов не умеет оказывать первую помощь.",
	"Для оказания первой помощи нет аптечек.",
	"Не хватает медикаментов и бинтов, чтобы перевязать всех.",
	"Ваши бойцы не нуждаются в первой помощи.",
	"Перевязать наемников автоматически",
	"Всем бойцам оказана первая помощь.",

	//14
	"Арулько",

	"(на крыше)",

	"Здоровье: %d/%d",

	//In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
	//"vs." is the abbreviation of versus.
	"%d против %d",

	"В %sе нет места!",  //(ex "The ice cream truck is full")///TRNSL_ASSUME vehicle gender is male

	"%s нуждается не в перевязке и первой помощи, а в основательном медицинском обследовании и/или отдыхе.",

	//20
	//Happens when you get shot in the legs, and you fall down.
	"%s получает ранение в ногу и падает!",
	//Name can't speak right now.
	"%s не может сейчас говорить.",

	//22-24 plural versions @@@2 elite to veteran
	"Несколько (%d) ополченцев-новобранцев стали ветеранами.",
	"Несколько (%d) ополченцев-новобранецев стали солдатами.",
	"Несколько (%d) ополченцев-солдат стали ветеранами.",

	//25
	"Перекл.", ///TRNSL_CHECK_INGAME LENGTH

	//26
	//Name has gone psycho -- when the game forces the player into burstmode (certain unstable characters)
	"%s впадает в безумие!",

	//27-28
	//Messages why a player can't time compress.
	"Нельзя ускорять время, поскольку у вас есть бойцы в секторе %s.", //
	"Нельзя ускорять время, когда бойцы находятся в шахтах с монстрами.", //

	//29-31 singular versions @@@2 elite to veteran
	"Один ополченец-новобранец стал ветераном.",
	"Один ополченец-новобранец стал солдатом.",
	"Один ополченец-солдат стал ветераном.",

	//32-34
	"%s ничего не говорит.",
	"Выбраться на поверхность?",
	"(Отряд %d)",

	//35
	//Ex: "Red has repaired Scope's MP5K".  Careful to maintain the proper order (Red before Scope, Scope before MP5K)
	"%s починил для %s %s",

	//36
	"КОШКА-УБИЙЦА",

	//37-38 "Name trips and falls"
	"%s спотыкается и падает",
	"Эту вещь нельзя взять отсюда.",

	//39
	"Ваши бойцы не в состоянии сражаться. Ополчение ведет бой с существами само.",

	//40-43
	//%s is the name of merc.
	"%s истратил(а) все медикаменты!",
	"%s не обладает навыками, чтобы лечить кого-либо!",
	"%s без инструментов!",
	"%s не обладает навыками, чтобы ремонтировать что-либо!",

	//44-45
	"Время ремонта",
	"%s не видит этого человека.",

	//46-48
	"%s ломает удлинитель ствола!",
	"Не разрешается больше %d тренеров ополчения на сектор.",
	"Вы уверены?",

	//49-50
	"Ускорение времени",
	"Бак теперь заправлен.",

	//51-52 Fast help text in mapscreen.
	"Продолжить ускорение времени (|S|p|a|c|e)",
	"Прекратить ускорение времени (|E|s|c)",

	//53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
	"%s расклинил %s",
	"%s расклинил для %s %s",

	//55
	"Невозможно сжимать время при просмотре предметов сектора.",

	//56
	//Displayed with the version information when cheats are enabled.
	"Текущий/Максимальный: %d%%/%d%%",

	//57
	"Сопроводить Джона и Мэри?",

	"Выключатель нажат."
};

static const ST::string s_rus_str_ceramic_plates_smashed = "%s: в бронежилете разбились пластины!";

static const ST::string s_rus_str_arrival_rerouted = "Место высадки новоприбывших наемников перенесено в сектор %s; ранее заданный сектор %s захвачен врагом.";


static const ST::string s_rus_str_stat_health       = "Здоровье";
static const ST::string s_rus_str_stat_agility      = "Проворность";
static const ST::string s_rus_str_stat_dexterity    = "Ловкость";
static const ST::string s_rus_str_stat_strength     = "Сила";
static const ST::string s_rus_str_stat_leadership   = "Лидерство";
static const ST::string s_rus_str_stat_wisdom       = "Мудрость";
static const ST::string s_rus_str_stat_exp_level    = "Опытность"; // Уровень опыта
static const ST::string s_rus_str_stat_marksmanship = "Меткость";
static const ST::string s_rus_str_stat_mechanical   = "Механика";
static const ST::string s_rus_str_stat_explosive    = "Взрывник";
static const ST::string s_rus_str_stat_medical      = "Медицина";

static const ST::string s_rus_str_stat_list[str_stat_list_SIZE] =
{
	s_rus_str_stat_health,
	s_rus_str_stat_agility,
	s_rus_str_stat_dexterity,
	s_rus_str_stat_strength,
	s_rus_str_stat_leadership,
	s_rus_str_stat_wisdom,
	s_rus_str_stat_exp_level,
	s_rus_str_stat_marksmanship,
	s_rus_str_stat_mechanical,
	s_rus_str_stat_explosive,
	s_rus_str_stat_medical
};

static const ST::string s_rus_str_aim_sort_list[str_aim_sort_list_SIZE] =
{
	s_rus_str_aim_sort_price,
	s_rus_str_aim_sort_experience,
	s_rus_str_aim_sort_marksmanship,
	s_rus_str_aim_sort_medical,
	s_rus_str_aim_sort_explosives,
	s_rus_str_aim_sort_mechanical,
	s_rus_str_aim_sort_ascending,
	s_rus_str_aim_sort_descending,
};

static const ST::string s_rus_zNewTacticalMessages[] =
{
	"Расстояние до цели: %d клеток",
	"Дальнобойность оружия: %d клеток, расстояние до цели: %d клеток",
	"Отобразить укрытия",
	"Зона обзора",
	"Это игра для человека со СТАЛЬНОЙ ВОЛЕЙ. Нельзя сохраняться, когда рядом враги!",
	"(Нельзя сохранять игру в бою)",
	"(Cannot load previous savegames)",
	"This is a Dead is Dead game and you cannot save through the save screen.",
	"точности удара: %d%%, c укрытия: %d%%", // Stracciatella: chance to hit
};

static const ST::string s_rus_str_iron_man_mode_warning = "Вы выбрали режим СТАЛЬНОЙ ВОЛИ. Эта настройка сильно усложняет игру, так как вы не сможете сохраняться, когда в секторе есть враги. ЖЕЛЕЗНАЯ ВОЛЯ действует в течение всей игры. Уверены, что хотите включить этот режим?";

extern const ST::string g_eng_str_dead_is_dead_mode_warning;
extern const ST::string g_eng_str_dead_is_dead_mode_enter_name;

static const ST::string s_rus_gs_dead_is_dead_mode_tab_name[gs_dead_is_dead_mode_tab_name_SIZE] =
{
	"Normal", 			// Normal Tab
	"DiD", 			// Dead is Dead Tab
};

// Russian language resources.
LanguageRes g_LanguageResRussian = {

	s_rus_WeaponType,

	s_rus_Message,
	s_rus_TeamTurnString,
	s_rus_pAssignMenuStrings,
	s_rus_pTrainingStrings,
	s_rus_pTrainingMenuStrings,
	s_rus_pAttributeMenuStrings,
	s_rus_pVehicleStrings,
	s_rus_pShortAttributeStrings,
	s_rus_pContractStrings,
	s_rus_pAssignmentStrings,
	s_rus_pConditionStrings,
	s_rus_pPersonnelScreenStrings,
	s_rus_pUpperLeftMapScreenStrings,
	s_rus_pTacticalPopupButtonStrings,
	s_rus_pSquadMenuStrings,
	s_rus_pDoorTrapStrings,
	s_rus_pLongAssignmentStrings,
	s_rus_pMapScreenMouseRegionHelpText,
	s_rus_pNoiseVolStr,
	s_rus_pNoiseTypeStr,
	s_rus_pDirectionStr,
	s_rus_pRemoveMercStrings,
	s_rus_sTimeStrings,
	s_rus_pInvPanelTitleStrings,
	s_rus_pPOWStrings,
	s_rus_pMilitiaString,
	s_rus_pMilitiaButtonString,
	s_rus_pEpcMenuStrings,
	s_rus_pRepairStrings,
	s_rus_sPreStatBuildString,
	s_rus_sStatGainStrings,
	s_rus_pHelicopterEtaStrings,
	s_rus_sMapLevelString,
	s_rus_gsLoyalString,
	s_rus_gsUndergroundString,
	s_rus_gsTimeStrings,
	s_rus_sFacilitiesStrings,
	s_rus_pMapPopUpInventoryText,
	s_rus_pwTownInfoStrings,
	s_rus_pwMineStrings,
	s_rus_pwMiscSectorStrings,
	s_rus_pMapInventoryErrorString,
	s_rus_pMapInventoryStrings,
	s_rus_pMovementMenuStrings,
	s_rus_pUpdateMercStrings,
	s_rus_pMapScreenBorderButtonHelpText,
	s_rus_pMapScreenBottomFastHelp,
	s_rus_pMapScreenBottomText,
	s_rus_pMercDeadString,
	s_rus_pSenderNameList,
	s_rus_pNewMailStrings,
	s_rus_pDeleteMailStrings,
	s_rus_pEmailHeaders,
	s_rus_pEmailTitleText,
	s_rus_pFinanceTitle,
	s_rus_pFinanceSummary,
	s_rus_pFinanceHeaders,
	s_rus_pTransactionText,
	s_rus_pMoralStrings,
	s_rus_pSkyriderText,
	s_rus_str_left_equipment,
	s_rus_pMapScreenStatusStrings,
	s_rus_pMapScreenPrevNextCharButtonHelpText,
	s_rus_pEtaString,
	s_rus_pShortVehicleStrings,
	s_rus_pTrashItemText,
	s_rus_pMapErrorString,
	s_rus_pMapPlotStrings,
	s_rus_pBullseyeStrings,
	s_rus_pMiscMapScreenMouseRegionHelpText,
	s_rus_str_he_leaves_where_drop_equipment,
	s_rus_str_she_leaves_where_drop_equipment,
	s_rus_str_he_leaves_drops_equipment,
	s_rus_str_she_leaves_drops_equipment,
	s_rus_pImpPopUpStrings,
	s_rus_pImpButtonText,
	s_rus_pExtraIMPStrings,
	s_rus_pFilesTitle,
	s_rus_pFilesSenderList,
	s_rus_pHistoryLocations,
	s_rus_pHistoryStrings,
	s_rus_pHistoryHeaders,
	s_rus_pHistoryTitle,
	s_rus_pShowBookmarkString,
	s_rus_pWebPagesTitles,
	s_rus_pWebTitle,
	s_rus_pPersonnelString,
	s_rus_pErrorStrings,
	s_rus_pDownloadString,
	s_rus_pBookMarkStrings,
	s_rus_pLaptopIcons,
	s_rus_gsAtmStartButtonText,
	s_rus_pPersonnelTeamStatsStrings,
	s_rus_pPersonnelCurrentTeamStatsStrings,
	s_rus_pPersonelTeamStrings,
	s_rus_pPersonnelDepartedStateStrings,
	s_rus_pMapHortIndex,
	s_rus_pMapVertIndex,
	s_rus_pMapDepthIndex,
	s_rus_pLaptopTitles,
	s_rus_pDayStrings,
	s_rus_pMilitiaConfirmStrings,
	s_rus_pSkillAtZeroWarning,
	s_rus_pIMPBeginScreenStrings,
	s_rus_pIMPFinishButtonText,
	s_rus_pIMPFinishStrings,
	s_rus_pIMPVoicesStrings,
	s_rus_pPersTitleText,
	s_rus_pPausedGameText,
	s_rus_zOptionsToggleText,
	s_rus_zOptionsScreenHelpText,
	s_rus_pDoctorWarningString,
	s_rus_pMilitiaButtonsHelpText,
	s_rus_pMapScreenJustStartedHelpText,
	s_rus_pLandMarkInSectorString,
	s_rus_gzMercSkillText,
	s_rus_gzNonPersistantPBIText,
	s_rus_gzMiscString,
	s_rus_wMapScreenSortButtonHelpText,
	s_rus_pNewNoiseStr,
	s_rus_gzLateLocalizedString,
	s_rus_pAntiHackerString,
	s_rus_pMessageStrings,
	s_rus_ItemPickupHelpPopup,
	s_rus_TacticalStr,
	s_rus_LargeTacticalStr,
	s_rus_zDialogActions,
	s_rus_zDealerStrings,
	s_rus_zTalkMenuStrings,
	s_rus_gzMoneyAmounts,
	s_rus_gzProsLabel,
	s_rus_gzConsLabel,
	s_rus_gMoneyStatsDesc,
	s_rus_gWeaponStatsDesc,
	s_rus_sKeyDescriptionStrings,
	s_rus_zHealthStr,
	s_rus_zVehicleName,
	s_rus_pExitingSectorHelpText,
	s_rus_InsContractText,
	s_rus_InsInfoText,
	s_rus_MercAccountText,
	s_rus_MercInfo,
	s_rus_MercNoAccountText,
	s_rus_MercHomePageText,
	s_rus_sFuneralString,
	s_rus_sFloristText,
	s_rus_sOrderFormText,
	s_rus_sFloristGalleryText,
	s_rus_sFloristCards,
	s_rus_BobbyROrderFormText,
	s_rus_BobbyRText,
	s_rus_str_bobbyr_guns_num_guns_that_use_ammo,
	s_rus_BobbyRaysFrontText,
	s_rus_AimSortText,
	s_rus_str_aim_sort_price,
	s_rus_str_aim_sort_experience,
	s_rus_str_aim_sort_marksmanship,
	s_rus_str_aim_sort_medical,
	s_rus_str_aim_sort_explosives,
	s_rus_str_aim_sort_mechanical,
	s_rus_str_aim_sort_ascending,
	s_rus_str_aim_sort_descending,
	s_rus_AimPolicyText,
	s_rus_AimMemberText,
	s_rus_CharacterInfo,
	s_rus_VideoConfercingText,
	s_rus_AimPopUpText,
	s_rus_AimLinkText,
	s_rus_AimHistoryText,
	s_rus_AimFiText,
	s_rus_AimAlumniText,
	s_rus_AimScreenText,
	s_rus_AimBottomMenuText,
	s_rus_zMarksMapScreenText,
	s_rus_gpStrategicString,
	s_rus_gpGameClockString,
	s_rus_SKI_Text,
	s_rus_SkiMessageBoxText,
	s_rus_zSaveLoadText,
	s_rus_zOptionsText,
	s_rus_gzGIOScreenText,
	s_rus_gzHelpScreenText,
	s_rus_gzLaptopHelpText,
	s_rus_gzMoneyWithdrawMessageText,
	s_rus_gzCopyrightText,
	s_rus_BrokenLinkText,
	s_rus_gzBobbyRShipmentText,
	s_rus_zGioDifConfirmText,
	s_rus_gzCreditNames,
	s_rus_gzCreditNameTitle,
	s_rus_gzCreditNameFunny,
	s_rus_pContractButtonString,
	s_rus_gzIntroScreen,
	s_rus_pUpdatePanelButtons,
	s_rus_sRepairsDoneString,
	s_rus_str_ceramic_plates_smashed,
	s_rus_str_arrival_rerouted,

	s_rus_str_stat_health,
	s_rus_str_stat_agility,
	s_rus_str_stat_dexterity,
	s_rus_str_stat_strength,
	s_rus_str_stat_leadership,
	s_rus_str_stat_wisdom,
	s_rus_str_stat_exp_level,
	s_rus_str_stat_marksmanship,
	s_rus_str_stat_mechanical,
	s_rus_str_stat_explosive,
	s_rus_str_stat_medical,

	s_rus_str_stat_list,
	s_rus_str_aim_sort_list,

	s_rus_zNewTacticalMessages,
	s_rus_str_iron_man_mode_warning,
	g_eng_str_dead_is_dead_mode_warning,
	g_eng_str_dead_is_dead_mode_enter_name,

	s_rus_gs_dead_is_dead_mode_tab_name,
	s_rus_gzIMPSkillTraitsText
};


// Russian Gold language resources.
LanguageRes g_LanguageResRussianGold = {

	s_rus_WeaponType,

	s_rus_Message,
	s_rus_TeamTurnString,
	s_rus_pAssignMenuStrings,
	s_rus_pTrainingStrings,
	s_rus_pTrainingMenuStrings,
	s_rus_pAttributeMenuStrings,
	s_rus_pVehicleStrings,
	s_rus_pShortAttributeStrings,
	s_rus_pContractStrings,
	s_rus_pAssignmentStrings,
	s_rus_pConditionStrings,
	s_rus_pPersonnelScreenStrings,
	s_rus_pUpperLeftMapScreenStrings,
	s_rus_pTacticalPopupButtonStrings,
	s_rus_pSquadMenuStrings,
	s_rus_pDoorTrapStrings,
	s_rus_pLongAssignmentStrings,
	s_rus_pMapScreenMouseRegionHelpText,
	s_rus_pNoiseVolStr,
	s_rus_pNoiseTypeStr,
	s_rus_pDirectionStr,
	s_rus_pRemoveMercStrings,
	s_rus_sTimeStrings,
	s_rus_pInvPanelTitleStrings,
	s_rus_pPOWStrings,
	s_rus_pMilitiaString,
	s_rus_pMilitiaButtonString,
	s_rus_pEpcMenuStrings,
	s_rus_pRepairStrings,
	s_rus_sPreStatBuildString,
	s_rus_sStatGainStrings,
	s_rus_pHelicopterEtaStrings,
	s_rus_sMapLevelString,
	s_rus_gsLoyalString,
	s_rus_gsUndergroundString,
	s_rus_gsTimeStrings,
	s_rus_sFacilitiesStrings,
	s_rus_pMapPopUpInventoryText,
	s_rus_pwTownInfoStrings,
	s_rus_pwMineStrings,
	s_rus_pwMiscSectorStrings,
	s_rus_pMapInventoryErrorString,
	s_rus_pMapInventoryStrings,
	s_rus_pMovementMenuStrings,
	s_rus_pUpdateMercStrings,
	s_rus_pMapScreenBorderButtonHelpText,
	s_rus_pMapScreenBottomFastHelp,
	s_rus_pMapScreenBottomText,
	s_rus_pMercDeadString,
	s_rusGold_pSenderNameList,
	s_rus_pNewMailStrings,
	s_rus_pDeleteMailStrings,
	s_rus_pEmailHeaders,
	s_rus_pEmailTitleText,
	s_rus_pFinanceTitle,
	s_rus_pFinanceSummary,
	s_rus_pFinanceHeaders,
	s_rus_pTransactionText,
	s_rus_pMoralStrings,
	s_rus_pSkyriderText,
	s_rus_str_left_equipment,
	s_rus_pMapScreenStatusStrings,
	s_rus_pMapScreenPrevNextCharButtonHelpText,
	s_rus_pEtaString,
	s_rus_pShortVehicleStrings,
	s_rus_pTrashItemText,
	s_rus_pMapErrorString,
	s_rus_pMapPlotStrings,
	s_rus_pBullseyeStrings,
	s_rus_pMiscMapScreenMouseRegionHelpText,
	s_rus_str_he_leaves_where_drop_equipment,
	s_rus_str_she_leaves_where_drop_equipment,
	s_rus_str_he_leaves_drops_equipment,
	s_rus_str_she_leaves_drops_equipment,
	s_rus_pImpPopUpStrings,
	s_rus_pImpButtonText,
	s_rus_pExtraIMPStrings,
	s_rus_pFilesTitle,
	s_rus_pFilesSenderList,
	s_rus_pHistoryLocations,
	s_rus_pHistoryStrings,
	s_rus_pHistoryHeaders,
	s_rus_pHistoryTitle,
	s_rus_pShowBookmarkString,
	s_rus_pWebPagesTitles,
	s_rus_pWebTitle,
	s_rus_pPersonnelString,
	s_rus_pErrorStrings,
	s_rus_pDownloadString,
	s_rus_pBookMarkStrings,
	s_rus_pLaptopIcons,
	s_rus_gsAtmStartButtonText,
	s_rus_pPersonnelTeamStatsStrings,
	s_rus_pPersonnelCurrentTeamStatsStrings,
	s_rus_pPersonelTeamStrings,
	s_rus_pPersonnelDepartedStateStrings,
	s_rus_pMapHortIndex,
	s_rus_pMapVertIndex,
	s_rus_pMapDepthIndex,
	s_rus_pLaptopTitles,
	s_rus_pDayStrings,
	s_rus_pMilitiaConfirmStrings,
	s_rus_pSkillAtZeroWarning,
	s_rus_pIMPBeginScreenStrings,
	s_rus_pIMPFinishButtonText,
	s_rus_pIMPFinishStrings,
	s_rus_pIMPVoicesStrings,
	s_rus_pPersTitleText,
	s_rus_pPausedGameText,
	s_rus_zOptionsToggleText,
	s_rus_zOptionsScreenHelpText,
	s_rus_pDoctorWarningString,
	s_rus_pMilitiaButtonsHelpText,
	s_rus_pMapScreenJustStartedHelpText,
	s_rus_pLandMarkInSectorString,
	s_rus_gzMercSkillText,
	s_rus_gzNonPersistantPBIText,
	s_rus_gzMiscString,
	s_rus_wMapScreenSortButtonHelpText,
	s_rus_pNewNoiseStr,
	s_rus_gzLateLocalizedString,
	s_rus_pAntiHackerString,
	s_rus_pMessageStrings,
	s_rus_ItemPickupHelpPopup,
	s_rus_TacticalStr,
	s_rus_LargeTacticalStr,
	s_rus_zDialogActions,
	s_rus_zDealerStrings,
	s_rus_zTalkMenuStrings,
	s_rus_gzMoneyAmounts,
	s_rus_gzProsLabel,
	s_rus_gzConsLabel,
	s_rus_gMoneyStatsDesc,
	s_rus_gWeaponStatsDesc,
	s_rus_sKeyDescriptionStrings,
	s_rus_zHealthStr,
	s_rus_zVehicleName,
	s_rus_pExitingSectorHelpText,
	s_rus_InsContractText,
	s_rus_InsInfoText,
	s_rus_MercAccountText,
	s_rus_MercInfo,
	s_rus_MercNoAccountText,
	s_rus_MercHomePageText,
	s_rus_sFuneralString,
	s_rus_sFloristText,
	s_rus_sOrderFormText,
	s_rus_sFloristGalleryText,
	s_rus_sFloristCards,
	s_rus_BobbyROrderFormText,
	s_rus_BobbyRText,
	s_rus_str_bobbyr_guns_num_guns_that_use_ammo,
	s_rus_BobbyRaysFrontText,
	s_rus_AimSortText,
	s_rus_str_aim_sort_price,
	s_rus_str_aim_sort_experience,
	s_rus_str_aim_sort_marksmanship,
	s_rus_str_aim_sort_medical,
	s_rus_str_aim_sort_explosives,
	s_rus_str_aim_sort_mechanical,
	s_rus_str_aim_sort_ascending,
	s_rus_str_aim_sort_descending,
	s_rus_AimPolicyText,
	s_rus_AimMemberText,
	s_rus_CharacterInfo,
	s_rus_VideoConfercingText,
	s_rus_AimPopUpText,
	s_rus_AimLinkText,
	s_rus_AimHistoryText,
	s_rus_AimFiText,
	s_rus_AimAlumniText,
	s_rus_AimScreenText,
	s_rus_AimBottomMenuText,
	s_rus_zMarksMapScreenText,
	s_rus_gpStrategicString,
	s_rus_gpGameClockString,
	s_rus_SKI_Text,
	s_rus_SkiMessageBoxText,
	s_rus_zSaveLoadText,
	s_rus_zOptionsText,
	s_rus_gzGIOScreenText,
	s_rus_gzHelpScreenText,
	s_rus_gzLaptopHelpText,
	s_rus_gzMoneyWithdrawMessageText,
	s_rusGold_gzCopyrightText,
	s_rus_BrokenLinkText,
	s_rus_gzBobbyRShipmentText,
	s_rus_zGioDifConfirmText,
	s_rus_gzCreditNames,
	s_rus_gzCreditNameTitle,
	s_rus_gzCreditNameFunny,
	s_rus_pContractButtonString,
	s_rus_gzIntroScreen,
	s_rus_pUpdatePanelButtons,
	s_rus_sRepairsDoneString,
	s_rus_str_ceramic_plates_smashed,
	s_rus_str_arrival_rerouted,

	s_rus_str_stat_health,
	s_rus_str_stat_agility,
	s_rus_str_stat_dexterity,
	s_rus_str_stat_strength,
	s_rus_str_stat_leadership,
	s_rus_str_stat_wisdom,
	s_rus_str_stat_exp_level,
	s_rus_str_stat_marksmanship,
	s_rus_str_stat_mechanical,
	s_rus_str_stat_explosive,
	s_rus_str_stat_medical,

	s_rus_str_stat_list,
	s_rus_str_aim_sort_list,

	s_rus_zNewTacticalMessages,
	s_rus_str_iron_man_mode_warning,
	g_eng_str_dead_is_dead_mode_warning,
	g_eng_str_dead_is_dead_mode_enter_name,

	s_rus_gs_dead_is_dead_mode_tab_name,

	s_rus_gzIMPSkillTraitsText,
};

#ifdef WITH_UNITTESTS
#define ARR_SIZE(x) (sizeof(x)/sizeof(x[0]))
TEST(StringEncodingTest, RusianTextFile)
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
