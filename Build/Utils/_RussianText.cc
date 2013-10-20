#include "Text.h"

#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"
#endif

/*

******************************************************************************************************
**                                  IMPORTANT TRANSLATION NOTES                                     **
******************************************************************************************************

GENERAL TOPWARE INSTRUCTIONS
- Always be aware that German strings should be of equal or shorter length than the English equivalent.
	I know that this is difficult to do on many occasions due to the nature of the German language when
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
	symbols.  SirTech will search for !!! to look for Topware problems and questions.  This is a more
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

	As described above, the "!!!" notation should be used by Topware to ask questions and address problems as
	SirTech uses the "@@@" notation.

*/

static StrPointer s_rus_WeaponType[WeaponType_SIZE] =
{
	L"Другое",
	L"Пистолет",
	L"Автом. пистолет",
	L"Пистолет-пулемет",
	L"Винтовка",
	L"Снайп. винтовка",
	L"Автомат",
	L"Легкий пулемет",
	L"Дробовик"
};

static StrPointer s_rus_TeamTurnString[TeamTurnString_SIZE] =
{
	L"Ход игрока", // player's turn
	L"Ход противника",
	L"Ход существ",
	L"Ход ополчения",
	L"Ход жителей"
	// planning turn
};

static StrPointer s_rus_Message[Message_SIZE] =
{
	// In the following 8 strings, the %ls is the merc's name, and the %d (if any) is a number.

	L"%ls получает ранение в голову и теряет в мудрости!",
	L"%ls получает ранение в плечо и теряет в ловкости!",
	L"%ls получает ранение в грудь и теряет в силе!",
	L"%ls получает ранение ног и теряет в проворности!",
	L"%ls получает ранение в голову и теряет %d очков мудрости!",
	L"%ls получает ранение в плечо и теряет %d очков ловкости!",
	L"%ls получает ранение в грудь и теряет %d очков силы!",
	L"%ls получает ранение ног и теряет %d очков проворности!",
	L"Перехват инициативы!",

	L"К вам на помощь прибыло подкрепление!",

	// In the following four lines, all %ls's are merc names

	L"%ls перезаряжает оружие.",
	L"%ls: недостаточно очков действия!",
	// the following 17 strings are used to create lists of gun advantages and disadvantages
	// (separated by commas)
	L"надежность",
	L"ненадежность",
	L"легко починить",
	L"трудно починить",
	L"сильный урон",
	L"слабый урон",
	L"скорострельность",
	L"медленный огонь",
	L"дальний бой",
	L"ближний бой",
	L"малый вес",
	L"большой вес",
	L"малые размеры",
	L"авто-огонь",
	L"только одиночными",
	L"большой магазин",
	L"маленький магазин",

	// In the following two lines, all %ls's are merc names

	L"%ls: камуфляж изношен.",
	L"%ls: окраска камуфляжа смыта.",

	// The first %ls is a merc name and the second %ls is an item name

	L"У второго оружия закончились патроны!",
	L"%ls крадет %ls.",

	// The %ls is a merc name

	L"%ls: оружие не стреляет очередями.",

	L"Это уже присоединено!",
	L"Соединить?",

	// Both %ls's are item names

	L"Нельзя присоединить %ls к %ls.",

	L"Ничего",
	L"Разрядить",
	L"Приспособления",

	//You cannot use "item(s)" and your "other item" at the same time.
	//Ex:  You cannot use sun goggles and you gas mask at the same time.
	L"Нельзя использовать %ls и %ls одновременно.",

	L"Этот предмет можно присоединить к другим предметам, поместив его в одну из ячеек приспособлений.",
	L"Этот предмет можно присоединить к другим предметам, поместив его в одну из ячеек приспособлений. (Однако данные предметы несовместимы.)",
	L"В секторе еще остались враги!",
	L"%ls должен получить еще %ls", ///TRNSL_ASSUME all doctors who can get money are male
	L"%ls: попадание в голову!",
	L"Выйти из боя?",
	L"Соединение будет неразборно. Соединить?",
	L"%ls чувствует прилив энергии!",
	L"%ls поскальзывается на шариках!",
	L"%ls не удается взять %ls!",
	L"%ls чинит %ls",
	L"На перехвате ",
	L"Сдаться?",
	L"Человек отвергает вашу помощь.",
	L"Ни за что!",//refuse to heal monster creature
	L"Чтобы воспользоваться вертолетом Всадника, нужно дать бойцам ЗАДАНИЕ сесть в ТРАНСПОРТ.",
	L"%ls успевает зарядить только одно оружие",
	L"Ход кошек-убийц",
};


// the names of the towns in the game

static const wchar_t *s_rus_pTownNames[pTownNames_SIZE] =
{
	L"",
	L"Омерта",
	L"Драссен",
	L"Альма",
	L"Грам",
	L"Тикса",
	L"Камбрия",
	L"Сан Мона",
	L"Эстони",
	L"Орта",
	L"Балайм",
	L"Медуна",
	L"Читзена",
};

static const wchar_t *s_rus_g_towns_locative[g_towns_locative_SIZE] =
{
	L"",
	L"Омерте",
	L"Драссене",
	L"Альма",     // TODO fill in correct forms
	L"Грам",
	L"Тикса",
	L"Камбрия",
	L"Сан Мона",
	L"Эстони",
	L"Орта",
	L"Балайм",
	L"Медуна",
	L"Читзена",
};

// the types of time compression. For example: is the timer paused? at normal speed, 5 minutes per second, etc.
// min is an abbreviation for minutes

static const wchar_t *s_rus_sTimeStrings[sTimeStrings_SIZE] =
{
	L"Пауза",
	L"Норма",
	L"5 мин",
	L"30 мин",
	L"60 мин",
	L"6 час",
};


// Assignment Strings: what assignment does the merc  have right now? For example, are they on a squad, training,
// administering medical aid (doctor) or training a town. All are abbreviated. 8 letters is the longest it can be.

static const wchar_t *s_rus_pAssignmentStrings[pAssignmentStrings_SIZE] =
{
	L"Отряд 1",
	L"Отряд 2",
	L"Отряд 3",
	L"Отряд 4",
	L"Отряд 5",
	L"Отряд 6",
	L"Отряд 7",
	L"Отряд 8",
	L"Отряд 9",
	L"Отряд 10",
	L"Отряд 11",
	L"Отряд 12",
	L"Отряд 13",
	L"Отряд 14",
	L"Отряд 15",
	L"Отряд 16",
	L"Отряд 17",
	L"Отряд 18",
	L"Отряд 19",
	L"Отряд 20",
	L"На службе", // on active duty
	L"Доктор", // оказывает медпомощь
	L"Пациент", //принимает медпомощь
	L"Транспорт", // in a vehicle
	L"В пути", //транзитом - сокращение
	L"Ремонт", // ремонтируются
	L"Практика", // тренируются
	L"Ополчение", //готовят восстание среди горожан
	L"Тренер", // training a teammate
	L"Ученик", // being trained by someone else
	L"Труп", // мертв
	L"Без созн.", // abbreviation for incapacitated
	L"В плену", // Prisoner of war - captured
	L"Госпиталь", // patient in a hospital
	L"Пуст",	// Vehicle is empty
};


static const wchar_t *s_rus_pMilitiaString[pMilitiaString_SIZE] =
{
	L"Ополчение", // the title of the militia box
	L"не распределено:", //the number of unassigned militia troops
	L"Нельзя перераспределять ополчение, когда кругом враги!",
};


static const wchar_t *s_rus_pMilitiaButtonString[pMilitiaButtonString_SIZE] =
{
	L"Авто", // auto place the militia troops for the player
	L"Готово", // done placing militia troops
};

static const wchar_t *s_rus_pConditionStrings[pConditionStrings_SIZE] =
{
	L"Отлично", //состояние солдата..отличное здоровье
	L"Хорошо", //хорошее здоровье
	L"Норма", //нормальное здоровье
	L"Ранение", //раны
	L"Без сил", // усталый
	L"Кровоточит", // истекает кровью
	L"Без созн.", // в обмороке
	L"Умирает", //умирает
	L"Труп", // мертв
};

static const wchar_t *s_rus_pEpcMenuStrings[pEpcMenuStrings_SIZE] =
{
	L"На службе", // set merc on active duty
	L"Пациент", // set as a patient to receive medical aid
	L"Транспорт", // tell merc to enter vehicle
	L"Без эскорта", // охрана покидает героя
	L"Отмена", // выход из этого меню
};


// look at pAssignmentString above for comments

static const wchar_t *s_rus_pLongAssignmentStrings[pLongAssignmentStrings_SIZE] =
{
	L"Отряд 1",
	L"Отряд 2",
	L"Отряд 3",
	L"Отряд 4",
	L"Отряд 5",
	L"Отряд 6",
	L"Отряд 7",
	L"Отряд 8",
	L"Отряд 9",
	L"Отряд 10",
	L"Отряд 11",
	L"Отряд 12",
	L"Отряд 13",
	L"Отряд 14",
	L"Отряд 15",
	L"Отряд 16",
	L"Отряд 17",
	L"Отряд 18",
	L"Отряд 19",
	L"Отряд 20",
	L"На службе",
	L"Доктор",
	L"Пациент",
	L"Транспорт",
	L"В пути",
	L"Ремонт",
	L"Практика",
	L"Ополчение",
	L"Тренер",
	L"Ученик",
	L"Труп",
	L"Без сознания",
	L"В плену",
	L"Госпиталь", // patient in a hospital
	L"Пуст",	// Vehicle is empty
};


// the contract options

static const wchar_t *s_rus_pContractStrings[pContractStrings_SIZE] =
{
	L"Действия по контракту:",
	L"", // a blank line, required
	L"Продлить на 1 день", // offer merc a one day contract extension
	L"Продлить на 7 дней", // 1 week
	L"Продлить на 14 дней", // 2 week
	L"Уволить", // end merc's contract
	L"Отмена", // stop showing this menu
};

static const wchar_t *s_rus_pPOWStrings[pPOWStrings_SIZE] =
{
	L"В плену",  //an acronym for Prisoner of War
	L"??",
};

static const wchar_t *s_rus_pInvPanelTitleStrings[pInvPanelTitleStrings_SIZE] =
{
	L"Броня", // the armor rating of the merc
	L"Груз", // the weight the merc is carrying
	L"Камуфляж", // the merc's camouflage rating
};

static const wchar_t *s_rus_pShortAttributeStrings[pShortAttributeStrings_SIZE] =
{
	L"Прв", // the abbreviated version of : agility
	L"Лов", // dexterity
	L"Сил", // strength
	L"Лид", // leadership
	L"Мдр", // wisdom
	L"Опт", // experience level
	L"Мтк", // marksmanship skill
	L"Взр", // explosive skill
	L"Мех", // mechanical skill
	L"Мед", // medical skill};
};


static const wchar_t *s_rus_pUpperLeftMapScreenStrings[pUpperLeftMapScreenStrings_SIZE] =
{
	L"Задание", // the mercs current assignment
	L"Здоровье", // the health level of the current merc
	L"Настрой", // the morale of the current merc
	L"Сост.",	// the condition of the current vehicle
};

static const wchar_t *s_rus_pTrainingStrings[pTrainingStrings_SIZE] =
{
	L"Практика", // tell merc to train self
	L"Ополчение", // tell merc to train town
	L"Тренер", // tell merc to act as trainer
	L"Ученик", // tell merc to be train by other
};

static const wchar_t *s_rus_pAssignMenuStrings[pAssignMenuStrings_SIZE] =
{
	L"На службе", // merc is on active duty
	L"Доктор", // the merc is acting as a doctor
	L"Пациент", // the merc is receiving medical attention
	L"Транспорт", // the merc is in a vehicle
	L"Ремонт", // the merc is repairing items
	L"Тренинг", // the merc is training
	L"Отмена", // cancel this menu
};

static const wchar_t *s_rus_pRemoveMercStrings[pRemoveMercStrings_SIZE] =
{
	L"Исключить из команды", // remove dead or captured by enemy merc from team
	L"Отмена",
};

static const wchar_t *s_rus_pAttributeMenuStrings[pAttributeMenuStrings_SIZE] =
{
	L"Сила",
	L"Ловкость",
	L"Проворность",
	L"Здоровье",
	L"Меткость",
	L"Медицина",
	L"Механика",
	L"Лидерство",
	L"Взрывник",
	L"Отмена",
};

static const wchar_t *s_rus_pTrainingMenuStrings[pTrainingMenuStrings_SIZE] =
{
	L"Практика", // train yourself
	L"Ополчение", // train the town
	L"Тренер", // train your teammates
	L"Ученик",  // be trained by an instructor
	L"Отмена", // cancel this menu
};


static const wchar_t *s_rus_pSquadMenuStrings[pSquadMenuStrings_SIZE] =
{
	L"Отряд  1",
	L"Отряд  2",
	L"Отряд  3",
	L"Отряд  4",
	L"Отряд  5",
	L"Отряд  6",
	L"Отряд  7",
	L"Отряд  8",
	L"Отряд  9",
	L"Отряд 10",
	L"Отряд 11",
	L"Отряд 12",
	L"Отряд 13",
	L"Отряд 14",
	L"Отряд 15",
	L"Отряд 16",
	L"Отряд 17",
	L"Отряд 18",
	L"Отряд 19",
	L"Отряд 20",
	L"Отмена",
};


static const wchar_t *s_rus_pPersonnelScreenStrings[pPersonnelScreenStrings_SIZE] =
{
	L"Медицинский депозит:", // amount of medical deposit put down on the merc
	L"Срок контракта:", // time of remaining contract
	L"Убийства", // number of kills by merc
	L"Помощь в бою", // number of assists on kills by merc
	L"Стоимость в день:", // daily cost of merc
	L"Гонорар:", // total cost of merc
	L"Контракт:", // cost of current contract
	L"На службе:", // total service rendered by merc
	L"Долг по оплате:", // amount left on MERC merc to be paid
	L"Процент попаданий:", // percentage of shots that hit target
	L"Битвы", // number of battles fought
	L"Ранения", // number of times merc has been wounded
	L"Навыки:",
	L"Нет навыков",
};


//These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
static const wchar_t *s_rus_gzMercSkillText[gzMercSkillText_SIZE] =
{
	L"Нет навыков",
	L"Взлом",
	L"Рукопашная",
	L"Электроника",
	L"Ночной бой",
	L"Броски",
	L"Учитель",
	L"Тяжелое вооружение",
	L"Автоматы",
	L"Скрытность",
	L"Амбидекстрия",
	L"Вор",
	L"Боевые искусства",
	L"Бой с ножом",
	L"Стрельба с крыш",
	L"Маскировка",
	L"(Эксперт)",
};


// This is pop up help text for the options that are available to the merc

static const wchar_t *s_rus_pTacticalPopupButtonStrings[pTacticalPopupButtonStrings_SIZE] =
{
	L"Стоять/Идти (|S)",
	L"Присядью (|C)",
	L"Стоять/Бегом (|R)",
	L"Лечь/Ползти (|P)",
	L"Поворот (|L)",
	L"Действие",
	L"Поговорить",
	L"Осмотреть (|C|t|r|l)",

	// Pop up door menu
	L"Открыть",
	L"Искать ловушки",
	L"Вскрыть отмычками",
	L"Взломать",
	L"Обезвредить",
	L"Запереть",
	L"Отпереть",
	L"Взорвать замок",
	L"Взломать ломом",
	L"Отмена (|E|s|c)",
	L"Закрыть",
};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.

static const wchar_t *s_rus_pDoorTrapStrings[pDoorTrapStrings_SIZE] =
{
	L"нет ловушки",
	L"мина",
	L"электроловушка",
	L"сирена",
	L"сигнализация"
};

// On the map screen, there are four columns. This text is popup help text that identifies the individual columns.

static const wchar_t *s_rus_pMapScreenMouseRegionHelpText[pMapScreenMouseRegionHelpText_SIZE] =
{
	L"Выбрать наемника",
	L"Дать задание",
	L"Проложить маршрут",
	L"Управление контрактом (|C)",
	L"Убрать бойца",
	L"Спать",
};

// volumes of noises

static const wchar_t *s_rus_pNoiseVolStr[pNoiseVolStr_SIZE] =
{
	L"ТИХИЙ",
	L"ЧЕТКИЙ",
	L"ГРОМКИЙ",
	L"ОГЛУШИТЕЛЬНЫЙ"
};

// types of noises

static const wchar_t *s_rus_pNoiseTypeStr[pNoiseTypeStr_SIZE] = // OBSOLETE
{
	L"НЕЗНАКОМЫЙ",
	L"звук ДВИЖЕНИЯ",
	L"СКРИП",
	L"ПЛЕСК",
	L"УДАР",
	L"ВЫСТРЕЛ",
	L"ВЗРЫВ",
	L"КРИК",
	L"УДАР",
	L"УДАР",
	L"ЗВОН",
	L"ГРОХОТ"
};

// Directions that are used to report noises

static const wchar_t *s_rus_pDirectionStr[pDirectionStr_SIZE] =
{
	L"с СЕВ-ВОСТОКА",
	L"с ВОСТОКА",
	L"с ЮГО-ВОСТОКА",
	L"с ЮГА",
	L"с ЮГО-ЗАПАДА",
	L"с ЗАПАДА",
	L"с СЕВ-ЗАПАДА",
	L"с СЕВЕРА"
};

// These are the different terrain types.

static const wchar_t *s_rus_pLandTypeStrings[pLandTypeStrings_SIZE] =
{
	L"Город",
	L"Дорога",
	L"Равнина",
	L"Пустыня",
	L"Леса",
	L"Роща",
	L"Болото",
	L"Вода",
	L"Холмы",
	L"Непроходимо",
	L"Река",	//river from north to south
	L"Река",	//river from east to west
	L"Чужая страна",
	//NONE of the following are used for directional travel, just for the sector description.
	L"Тропики",
	L"Фермы",
	L"Поля, дорога",
	L"Леса, дорога",
	L"Фермы, дорога",
	L"Тропики, дорога",
	L"Роща, дорога",
	L"Берег",
	L"Горы, дорога",
	L"Побережье, дорога",
	L"Пустыня, дорога",
	L"Болото, дорога",
	L"Леса, ПВО",
	L"Пустыня, ПВО",
	L"Тропики, ПВО",
	L"Медуна, ПВО",

	//These are descriptions for special sectors
	L"Госпиталь Камбрии",
	L"Аэропорт Драссена",
	L"Аэропорт Медуны",
	L"База ПВО",
	L"База повстанцев", //The rebel base underground in sector A10
	L"Подземелья Тиксы",	//The basement of the Tixa Prison (J9)
	L"Логово существ",	//Any mine sector with creatures in it
	L"Подвалы Орты",	//The basement of Orta (K4)
	L"Туннель",				//The tunnel access from the maze garden in Meduna
										//leading to the secret shelter underneath the palace
	L"Убежище",				//The shelter underneath the queen's palace
	L"",							//Unused
};

static const wchar_t *s_rus_gpStrategicString[gpStrategicString_SIZE] =
{
	L"%ls обнаружены в секторе %c%d и вот-вот прибудет еще один отряд.", //STR_DETECTED_SINGULAR
	L"%ls обнаружены в секторе %c%d и вот-вот прибудут еще отряды.",	//STR_DETECTED_PLURAL
	L"Хотите координировать одновременное прибытие?",													//STR_COORDINATE

	//Dialog strings for enemies.

	L"Враг предлагает сдаться.",			//STR_ENEMY_SURRENDER_OFFER
	L"Враг взял в плен наемников без сознания.",	//STR_ENEMY_CAPTURED

	//The text that goes on the autoresolve buttons

	L"Отступить", 		//The retreat button		//STR_AR_RETREAT_BUTTON
	L"Готово",		//The done button				//STR_AR_DONE_BUTTON

	//The headers are for the autoresolve type (MUST BE UPPERCASE)

	L"ОБОРОНА",								//STR_AR_DEFEND_HEADER
	L"АТАКА",								//STR_AR_ATTACK_HEADER
	L"СТОЛКНОВЕНИЕ",						//STR_AR_ENCOUNTER_HEADER
	L"Сектор",		//The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

	//The battle ending conditions

	L"ПОБЕДА!",								//STR_AR_OVER_VICTORY
	L"ПОРАЖЕНИЕ!",							//STR_AR_OVER_DEFEAT  ///TRNSL_CHECK_INGAME LENGTH
	L"СДАЧА!",								//STR_AR_OVER_SURRENDERED
	L"В ПЛЕНУ!",							//STR_AR_OVER_CAPTURED
	L"ОТСТУПЛЕНИЕ!",						//STR_AR_OVER_RETREATED

	//These are the labels for the different types of enemies we fight in autoresolve.

	L"Ополчение",							//STR_AR_MILITIA_NAME,
	L"Элита",								//STR_AR_ELITE_NAME,
	L"Солдат",								//STR_AR_TROOP_NAME,
	L"Админ",								//STR_AR_ADMINISTRATOR_NAME,
	L"Существо",								//STR_AR_CREATURE_NAME,

	//Label for the length of time the battle took

	L"Бой шел",						//STR_AR_TIME_ELAPSED,

	//Labels for status of merc if retreating.  (UPPERCASE)

	L"ОТСТУПИЛ",							//STR_AR_MERC_RETREATED,
	L"ОТСТУПАЕТ",							//STR_AR_MERC_RETREATING,
	L"ОТСТУПЛЕНИЕ",							//STR_AR_MERC_RETREAT,

	//PRE BATTLE INTERFACE STRINGS
	//Goes on the three buttons in the prebattle interface.  The Auto resolve button represents
	//a system that automatically resolves the combat for the player without having to do anything.
	//These strings must be short (two lines -- 6-8 chars per line)

	L"Авто битва",							//STR_PB_AUTORESOLVE_BTN,
	L"Идти в сектор",						//STR_PB_GOTOSECTOR_BTN,
	L"Уйти",								//STR_PB_RETREATMERCS_BTN,

	//The different headers(titles) for the prebattle interface.
	L"СТОЛКНОВЕНИЕ",						//STR_PB_ENEMYENCOUNTER_HEADER,
	L"НАПАДЕНИЕ",							//STR_PB_ENEMYINVASION_HEADER, // 30
	L"ЗАСАДА",								//STR_PB_ENEMYAMBUSH_HEADER
	L"ВРАЖЕСКИЙ СЕКТОР",					//STR_PB_ENTERINGENEMYSECTOR_HEADER
	L"АТАКА СУЩЕСТВ",						//STR_PB_CREATUREATTACK_HEADER
	L"ЗАСАДА КОШЕК-УБИЙЦ",					//STR_PB_BLOODCATAMBUSH_HEADER
	L"ЛОГОВО КОШЕК-УБИЙЦ",					//STR_PB_ENTERINGBLOODCATLAIR_HEADER

	//Various single words for direct translation.  The Civilians represent the civilian
	//militia occupying the sector being attacked.  Limited to 9-10 chars

	L"Место",
	L"Враги",
	L"Наемники",
	L"Ополченцы",
	L"Существа",
	L"Кошки",
	L"Сектор",
	L"Никого",		//If there are no uninvolved mercs in this fight.
	L"Н/П",			//Acronym of Not Applicable
	L"д",			//One letter abbreviation of day
	L"ч",			//One letter abbreviation of hour

	//TACTICAL PLACEMENT USER INTERFACE STRINGS
	//The four buttons

	L"Очистить",
	L"Порознь",
	L"Группой",
	L"Готово",

	//The help text for the four buttons.  Use \n to denote new line (just like enter).

	L"Отменить размещение и начать заново (|C)",
	L"Равномерно распределить бойцов (|S)",
	L"Сгруппировать бойцов в заданной точке (|G)",
	L"Нажмите эту кнопку, когда закончите\nвыбор позиций для бойцов (|E|n|t|e|r).",
	L"Нужно разместить ВСЕХ наемников\nперед началом битвы.",

	//Various strings (translate word for word)

	L"Сектор",
	L"выберите исходные позиции",

	//Strings used for various popup message boxes.  Can be as long as desired.

	L"Не очень-то хорошее место. Туда не пройти. Выберите другую позицию.",
	L"Разместите своих наемников в выделенной области.",

	//These entries are for button popup help text for the prebattle interface.  All popup help
	//text supports the use of \n to denote new line.  Do not use spaces before or after the \n.
	L"Битва разрешается автоматически\nбез вашего участия (|A)",
	L"Нельзя использовать авто битву\nкогда вы нападаете",
	L"Войти в сектор: стычка с врагом (|E)",
	L"Отступить в исходный сектор (|R)",				//singular version
	L"Отступить всем отрядам на исходные позиции (|R)", //multiple groups with same previous sector
//!!!What about repeated "R" as hotkey?
	//various popup messages for battle conditions.

	//%c%d is the sector -- ex:  A9
	L"Враги атакуют ваше ополчение в секторе %c%d.",
	//%c%d сектор -- напр:  A9
	L"Существа напали на ополчение в секторе %c%d.",
	//1st %d refers to the number of civilians eaten by monsters,  %c%d is the sector -- ex:  A9
	//Note:  the minimum number of civilians eaten will be two.
	L"Существа напали на гражданских, убито %d в секторе %ls.",
	//%ls is the sector location -- ex:  A9: Omerta
	L"Враги атакуют ваш отряд секторе %ls. Никто из наемников не может сражаться!",
	//%ls is the sector location -- ex:  A9: Omerta
	L"Существа атакуют ваш отряд в секторе %ls. Никто из наемников не может сражаться!",

};

//This is the day represented in the game clock.  Must be very short, 4 characters max.
static const wchar_t s_rus_gpGameClockString[] = L"День";

//When the merc finds a key, they can get a description of it which
//tells them where and when they found it.
static const wchar_t *s_rus_sKeyDescriptionStrings[sKeyDescriptionStrings_SIZE] =
{
	L"Найден в:",
	L"День находки:",
};

//The headers used to describe various weapon statistics.

static StrPointer s_rus_gWeaponStatsDesc[ gWeaponStatsDesc_SIZE] =
{
	L"Вес (%ls):",
	L"Состояние:",
	L"Патроны:", 	// Number of bullets left in a magazine
	L"Дист:",		// Range
	L"Урон:",		// Damage
	L"ОД:",			// abbreviation for Action Points
	L"="
};

//The headers used for the merc's money.

static const wchar_t *s_rus_gMoneyStatsDesc[gMoneyStatsDesc_SIZE] =
{
	L"Денег в",
	L"пачке:", //this is the overall balance
	L"Отделить",
	L"сумму:", // the amount he wants to separate from the overall balance to get two piles of money

	L"Текущий",
	L"баланс",
	L"Взять со",
	L"счета",
};

//The health of various creatures, enemies, characters in the game. The numbers following each are for comment
//only, but represent the precentage of points remaining.

static const wchar_t *s_rus_zHealthStr[zHealthStr_SIZE] = ///TRNSL_BAD (GENDER)
{
	L"УМИРАЕТ",  // >=  0
	L"КРИТИЧНО", // >= 15
	L"ПЛОХ",     // >= 30
	L"РАНЕН",    // >= 45
	L"ЗДОРОВ",   // >= 60
	L"СИЛЕН",    // >= 75
	L"ОТЛИЧНО",  // >= 90
};

static const wchar_t *s_rus_gzMoneyAmounts[gzMoneyAmounts_SIZE] =
{
	L"1000$",
	L"100$",
	L"10$",
	L"OK",
	L"Отделить",
	L"Взять"
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
static const wchar_t s_rus_gzProsLabel[] = L"Плюсы:";
static const wchar_t s_rus_gzConsLabel[] = L"Минусы:";

//Conversation options a player has when encountering an NPC
static StrPointer s_rus_zTalkMenuStrings[zTalkMenuStrings_SIZE] =
{
	L"Еще раз?", 	//meaning "Repeat yourself"
	L"Дружески",		//approach in a friendly
	L"Прямо",		//approach directly - let's get down to business
	L"Угрожать",		//approach threateningly - talk now, or I'll blow your face off
	L"Дать",
	L"Нанять"
};

//Some NPCs buy, sell or repair items. These different options are available for those NPCs as well.
static StrPointer s_rus_zDealerStrings[zDealerStrings_SIZE] =
{
	L"Торговля",
	L"Купить",
	L"Продать",
	L"Ремонт",
};

static const wchar_t s_rus_zDialogActions[] = L"Готово";


//These are vehicles in the game.

static const wchar_t *s_rus_pVehicleStrings[pVehicleStrings_SIZE] =
{
	L"Эльдорадо",
	L"Хаммер", // a hummer jeep/truck -- military vehicle
	L"Минивэн",
	L"Джип",
	L"Танк",
	L"Вертолет",
};

static const wchar_t *s_rus_pShortVehicleStrings[pVehicleStrings_SIZE] =
{
	L"Эльдор",
	L"Хаммер",			// the HMVV
	L"Вэн",
	L"Джип",
	L"Танк",
	L"Верт.", 				// the helicopter
};

static const wchar_t *s_rus_zVehicleName[pVehicleStrings_SIZE] =
{
	L"Эльдорадо",
	L"Хаммер",		//a military jeep. This is a brand name.
	L"Вэн",			// Ice cream truck
	L"Джип",
	L"Танк",
	L"Верт.", 		//an abbreviation for Helicopter
};


//These are messages Used in the Tactical Screen

static StrPointer s_rus_TacticalStr[TacticalStr_SIZE] =
{
	L"Воздушный налет",
	L"Оказать первую помощь?",

	// CAMFIELD NUKE THIS and add quote #66.

	L"%ls замечает недостачу товара в ящике.",

	// The %ls is a string from pDoorTrapStrings

	L"К замку присоединена %ls.", ///TRNSL_ASSUME all traps are female gender words
	L"Тут нет замка.",
	L"Замок без ловушки.",
	// The %ls is a merc name
	L"%ls не имеет нужного ключа.",
	L"Замок без ловушки.",
	L"Заперто.",
	L"ДВЕРЬ",
	L"С ЛОВУШКОЙ",
	L"ЗАПЕРТАЯ",
	L"НЕЗАПЕРТАЯ",
	L"СЛОМАНАЯ",
	L"Тут есть выключатель. Нажать?",
	L"Обезвредить ловушку?",
	L"Еще предметы...",

	// In the next 2 strings, %ls is an item name

	L"%ls теперь на земле.",
	L"Предмет \'%ls\' получает %ls.",

	// In the next 2 strings, %ls is a name

	L"%ls получил(а) всю сумму.",
	L"%ls ожидает от вас уплаты еще %d.",
	L"Выберите частоту детонатора:",  	//in this case, frequency refers to a radio signal
	L"Количество ходов перед взрывом:",	//how much time, in turns, until the bomb blows
	L"Выберите частоту подрыва:", 	//in this case, frequency refers to a radio signal
	L"Разрядить ловушку?",
	L"Убрать флажок?",
	L"Установить флажок?",
	L"Завершение хода",

	// In the next string, %ls is a name. Stance refers to way they are standing.

	L"%ls на вашей стороне! Атаковать?",
	L"Ох, транспорт не может менять положения.",
	L"Робот не может менять положения.",

	// In the next 3 strings, %ls is a name

	L"%ls не может здесь поменять положение.",
	L"%ls не может быть перевязан.",
	L"%ls не нуждается в перевязке.",
	L"Туда идти нельзя.",
	L"Команда набрана. Нет места для новобранца.",	//there's no room for a recruit on the player's team

	// In the next string, %ls is a name

	L"%ls теперь в команде.",

	// Here %ls is a name and %d is a number

	L"%ls ожидает уплаты еще $%d.",

	// In the next string, %ls is a name

	L"%ls будет сопровождаться в составе вашего отряда. Согласны?",

	// In the next string, the first %ls is a name and the second %ls is an amount of money (including $ sign)

	L"%ls может вступить в ваш отряд за %ls в день. Нанять?",

	// This line is used repeatedly to ask player if they wish to participate in a boxing match.

	L"Хотите драться?",

	// In the next string, the first %ls is an item name and the
	// second %ls is an amount of money (including $ sign)

	L"Купить %ls за %ls?",

	// In the next string, %ls is a name

	L"%ls теперь сопровождается отрядом %d.",

	// These messages are displayed during play to alert the player to a particular situation

	L"ЗАКЛИНИЛО",					//weapon is jammed.
	L"Роботу нужны патроны %ls.",		//Robot is out of ammo
	L"Бросить туда? Не получится.",		//Merc can't throw to the destination he selected

	// These are different buttons that the player can turn on and off.

	L"Режим скрытности (|Z)",
	L"Окно карты (|M)",
	L"Завершить ход (|D)",
	L"Говорить",
	L"Запретить отзывы",
	L"Подняться (|P|g|U|p)",
	L"Уровень курсора (|T|a|b)",
	L"Залезть/Слезть",
	L"Опуститься (|P|g|D|n)",
	L"Осмотреть (|C|t|r|l)",
	L"Предыдущий боец",
	L"Следующий боец (|S|p|a|c|e)",
	L"Настройки (|O)",
	L"Стрелять очередью (|B)",
	L"Смотреть/Повернуться (|L)",
	L"Здоровье: %d/%d\nЭнергия.: %d/%d\nНастрой: %ls",
	L"Чего?",					//this means "what?"
	L"Продолж.",					//an abbrieviation for "Continued"
	L"%ls будет говорить.",
	L"%ls будет молчать.",
	L"Состояние: %d/%d\nТопливо: %d/%d",
	L"Выйти из машины",
	L"Поменять отряд (|S|h|i|f|t |S|p|a|c|e)",
	L"Вести машину",
	L"Н/П",						//this is an acronym for "Not Applicable."
	L"Применить (Рукопашная)",
	L"Применить (Оружие)",
	L"Применить (Нож)",
	L"Применить (Взрывчатка)",
	L"Применить (Аптечка)",
	L"(Ловит)",
	L"(Перезарядка)", ///TRNSL_CHECK_INGAME LENGTH
	L"(Дать)",
	L"Сработала %ls.", // The %ls here is a string from pDoorTrapStrings  ASSUME all traps are female gender
	L"%ls прибыл(a).",
	L"%ls: нет очков действия.",
	L"%ls: наемник недоступен.",
	L"%ls: успешная перевязка.",
	L"%ls: нет бинтов.",
	L"Враг в секторе!",
	L"Нет врагов в поле зрения.",
	L"Не хватает очков действия.",
	L"Никто не использует дистанционное управление.",
	L"Обойма опустела!",
	L"СОЛДАТ",
	L"РЕПТИОН",
	L"ОПОЛЧЕНЕЦ",
	L"ЖИТЕЛЬ",
	L"Выход из сектора",
	L"OK",
	L"ОТМЕНА",
	L"Выбранный боец",
	L"Весь отряд",
	L"Идти в сектор",
	L"Идти на карту",
	L"Этот сектор нельзя покинуть здесь.",
	L"%ls слишком далеко.",
	L"Вершины деревьев скрыты",
	L"Вершины деревьев отображены",
	L"ВОРОНА",				//Crow, as in the large black bird
	L"ШЕЯ",
	L"ГОЛОВА",
	L"ГРУДЬ",
	L"НОГИ",
	L"Сказать королеве то, что она хочет знать?",
	L"Отпечатки пальцев получены",
	L"Отпечатки неверны. Оружие заблокировано",
	L"Цель захвачена",
	L"Путь блокирован",
	L"Положить/Взять деньги",		//Help text over the $ button on the Single Merc Panel
	L"Некого лечить.",
	L"Слом.",											// Short form of JAMMED, for small inv slots
	L"Туда не добраться.",					// used ( now ) for when we click on a cliff
	L"Человек отказывается двигаться.",
	// In the following message, '%ls' would be replaced with a quantity of money (e.g. $200)
	L"Заплатить %ls?",
	L"Согласиться на бесплатное лечение?",
	L"Согласны женить Дэррела?",
	L"Ключи",
	L"С эскортируемыми этого сделать нельзя.",
	L"Пощадить Кротта?",
	L"Цель вне зоны эффективного огня.",
	L"Шахтер",
	L"Транспорт передвигается только между секторами.",
	L"Автоперевязку сделать сейчас нельзя",
	L"%ls не может пройти, путь блокирован.",
	L"Здесь томятся ваши бойцы, захваченные армией Дейдраны!",
	L"Попадание в замок",
	L"Замок разрушен",
	L"Кто-то еще пытается воспользоваться этой дверью.",
	L"Состояние: %d/%d\nТопливо: %d/%d",
	L"%ls и %ls не видят друг друга.", // Cannot see person trying to talk to
};

//Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different circumstances in the "exiting sector" interface.
static const wchar_t *s_rus_pExitingSectorHelpText[pExitingSectorHelpText_SIZE] =
{
	//Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box is checked.
	L"Если выбрана эта опция, вы сразу перейдете в смежный сектор.",
	L"Если выбрана эта опция, вы выйдете\nна карту, пока ваши бойцы в пути.",

	//If you attempt to leave a sector when you have multiple squads in a hostile sector.
	L"Сектор занят врагами и оставлять здесь бойцов нельзя.\nРазберитесь с противником прежде чем идти в другие сектора.",

	//Because you only have one squad in the sector, and the "move all" option is checked, the "go to sector" option is locked to on.
	//The helptext explains why it is locked.
	L"Выведя всех наемников из сектора,\nвы автоматически перейдете в смежный сектор.",
	L"Выведя всех наемников из сектора,\nвы автоматически выйдете на карту,\nпока ваши бойцы в пути.",

	//If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being escorted.  The "single" button is disabled.
	L"%ls не может покинуть этот сектор без сопровождения.",

	//If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc will be prohibited from leaving alone.
	//There are several strings depending on the gender of the merc and how many EPCs are in the squad.
	//DO NOT USE THE NEWLINE HERE AS IT IS USED FOR BOTH HELPTEXT AND SCREEN MESSAGES!
	L"%ls не может покинуть сектор один - он сопровождает %ls.", //male singular
	L"%ls не может покинуть сектор одна - она сопровождает %ls.", //female singular
	L"%ls не может покинуть сектор один - он сопровождает группу.", //male plural
	L"%ls не может покинуть сектор одна - она сопровождает группу.", //female plural

	//If one or more of your mercs in the selected squad aren't in range of the traversal area, then the  "move all" option is disabled,
	//and this helptext explains why.
	L"Для перемещения отряда\nвсе ваши наемники дожны быть рядом.",

	//Standard helptext for single movement.  Explains what will happen (splitting the squad)
	L"Если выбрана эта опция, %ls пойдет в одиночку и\nавтоматически попадет в отдельный отряд.",

	//Standard helptext for all movement.  Explains what will happen (moving the squad)
	L"Если выбрана эта опция, текущий \nотряд покинет этот сектор.",

	//This strings is used BEFORE the "exiting sector" interface is created.  If you have an EPC selected and you attempt to tactically
	//traverse the EPC while the escorting mercs aren't near enough (or dead, dying, or unconscious), this message will appear and the
	//"exiting sector" interface will not appear.  This is just like the situation where
	//This string is special, as it is not used as helptext.  Do not use the special newline character (\n) for this string.
	L"%ls не может покинуть этот сектор без сопровождения. Для этого ваш отряд должен быть рядом.",
};



static const wchar_t *s_rus_pRepairStrings[pRepairStrings_SIZE] =
{
	L"Вещи", 		// tell merc to repair items in inventory
	L"ПВО", 		// tell merc to repair SAM site - SAM is an acronym for Surface to Air Missile
	L"Отмена", 		// cancel this menu
	L"Робот", 		// repair the robot
};


// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."

static const wchar_t *s_rus_sPreStatBuildString[sPreStatBuildString_SIZE] =
{
	L"теряет", 			// the merc has lost a statistic
	L"получает", 		// the merc has gained a statistic
	L"очко",	// singular
	L"очка",	// plural
	L"уровень",	// singular
	L"уровня",	// plural
};

static const wchar_t *s_rus_sStatGainStrings[sStatGainStrings_SIZE] =
{
	L"здоровья.",
	L"подвижности.",
	L"проворности.",
	L"мудрости.",
	L"медицины.",
	L"умения взрывника.",
	L"умения механика.",
	L"меткости.",
	L"опыта.",
	L"силы.",
	L"лидерства.",
};


static const wchar_t *s_rus_pHelicopterEtaStrings[pHelicopterEtaStrings_SIZE] =
{
	L"Общее расстояние:  ", 			// total distance for helicopter to travel
	L" Безопасно:  ", 			// distance to travel to destination
	L" Опасно:", 			// distance to return from destination to airport
	L"Цена полета: ", 		// total cost of trip by helicopter
	L"ОВП:  ", 			// ETA is an acronym for "estimated time of arrival"
	L"У вертолета мало топлива, придется сесть на территории врага!",	// warning that the sector the helicopter is going to use for refueling is under enemy control ->
	L"Пассажиры: ",
	L"Высадить Всадника или прибывающих?",
	L"Всадник",
	L"Прибывающие",
};

static const wchar_t s_rus_sMapLevelString[] = L"Подуровень "; // what level below the ground is the player viewing in mapscreen

static const wchar_t s_rus_gsLoyalString[] = L"Отношение %d%%"; // the loyalty rating of a town ie : Loyal 53%


// error message for when player is trying to give a merc a travel order while he's underground.
static const wchar_t s_rus_gsUndergroundString[] = L"не может двигаться по карте, находясь под землей.";

static const wchar_t *s_rus_gsTimeStrings[gsTimeStrings_SIZE] =
{
	L"ч",				// hours abbreviation
	L"м",				// minutes abbreviation
	L"с",				// seconds abbreviation
	L"д",				// days abbreviation
};

// text for the various facilities in the sector

static const wchar_t *s_rus_sFacilitiesStrings[sFacilitiesStrings_SIZE] =
{
	L"Нет",
	L"Госпиталь",
	L"Заводы",
	L"Тюрьма",
	L"База армии",
	L"Аэропорт",
	L"Стрельбище",		// a field for soldiers to practise their shooting skills
};

// text for inventory pop up button

static const wchar_t *s_rus_pMapPopUpInventoryText[pMapPopUpInventoryText_SIZE] =
{
	L"Предметы",
	L"Выход",
};

// town strings

static const wchar_t *s_rus_pwTownInfoStrings[pwTownInfoStrings_SIZE] =
{
	L"Размер",					// size of the town in sectors
	L"Контроль над городом", 					// how much of town is controlled
	L"Шахта города", 				// mine associated with this town
	L"Отношение",					// the loyalty level of this town
	L"Сооружения", 				// main facilities in this town
	L"Подготовка жителей",				// state of civilian training in town
	L"Ополчение", 					// the state of the trained civilians in the town
};

// Mine strings

static const wchar_t *s_rus_pwMineStrings[pwMineStrings_SIZE] =
{
	L"Шахта",						// 0
	L"Серебро",
	L"Золото",
	L"Выработка в день",
	L"Максимум выработки",
	L"Заброшена",				// 5
	L"Закрыта",
	L"Истощена",
	L"Работает",
	L"Состояние",
	L"Производительность",
	L"Тип руды",				// 10
	L"Контроль над городом",
	L"Отношение города",
};

// blank sector strings

static const wchar_t *s_rus_pwMiscSectorStrings[pwMiscSectorStrings_SIZE] =
{
	L"Силы врага",
	L"Сектор",
	L"Количество предметов",
	L"?",
	L"Сектор захвачен",
	L"Да",
	L"Нет",
};

// error strings for inventory

static const wchar_t *s_rus_pMapInventoryErrorString[pMapInventoryErrorString_SIZE] =
{
	L"Нельзя выбрать этого бойца.",  //MARK CARTER
	L"%ls не в этом секторе, и не может взять этот предмет.",
	L"Во время битвы надо подбирать предметы вручную",
	L"Во время битвы надо бросать предметы вручную.",
	L"%ls не в этом секторе, и не может бросить этот предмет.",
};

static const wchar_t *s_rus_pMapInventoryStrings[pMapInventoryStrings_SIZE] =
{
	L"Сектор", 			// sector these items are in
	L"Всего предметов", 		// total number of items in sector
};


// movement menu text

static const wchar_t *s_rus_pMovementMenuStrings[pMovementMenuStrings_SIZE] =
{
	L"Отправить бойцов из сектора %ls", 	// title for movement box
	L"Проложить маршрут", 		// done with movement menu, start plotting movement
	L"Отмена", 		// cancel this menu
	L"Другие",		// title for group of mercs not on squads nor in vehicles
};


static const wchar_t *s_rus_pUpdateMercStrings[pUpdateMercStrings_SIZE] =
{
	L"Ой!:", 			// an error has occured
	L"Контракты закончились:", 	// this pop up came up due to a merc contract ending
	L"Бойцы выполнили задание:", // this pop up....due to more than one merc finishing assignments
	L"Бойцы снова в строю:", // this pop up ....due to more than one merc waking up and returing to work
	L"Бойцы идут спать:", // this pop up ....due to more than one merc being tired and going to sleep
	L"Контракты скоро кончатся:", // this pop up came up due to a merc contract ending
};

// map screen map border buttons help text

static const wchar_t *s_rus_pMapScreenBorderButtonHelpText[pMapScreenBorderButtonHelpText_SIZE] =
{
	L"Показать города (|W)",
	L"Показать шахты (|M)",
	L"Показать отряды и врагов (|T)",
	L"Показать воздушное пространство (|A)",
	L"Показать предметы (|I)",
	L"Показать ополчение и врагов (|Z)",
};


static const wchar_t *s_rus_pMapScreenBottomFastHelp[pMapScreenBottomFastHelp_SIZE] =
{
	L"Лэптоп (|L)",
	L"Тактика (|E|s|c)",
	L"Настройки (|O)",
	L"Ускорение времени (|+)", 	// time compress more
	L"Замедление времени (|-)", 	// time compress less
	L"Предыдущее сообщение (|U|p)\nПредыдущая страница (|P|g|U|p)", 	// previous message in scrollable list
	L"Следующее сообщение (|D|o|w|n)\nСледующая страница (|P|g|D|n)", 	// next message in the scrollable list
	L"Пауза/Снять с паузы (|S|p|a|c|e)",	// start/stop time compression
};

static const wchar_t s_rus_pMapScreenBottomText[] = L"Текущий баланс"; // current balance in player bank account

static const wchar_t s_rus_pMercDeadString[] = L"%ls мертв(а).";


static const wchar_t s_rus_pDayStrings[] = L"День";

// the list of email sender names

static const wchar_t *s_rus_pSenderNameList[pSenderNameList_SIZE] = // GOLD and BUKA use slightly different names
{
	L"Энрико",
	L"Псих Про Инк.",
	L"Помощь",
	L"Псих Про Инк.",
	L"Спек",
	L"R.I.S.",		//5
	L"Барри",
	L"Блад",
	L"Рысь",
	L"Гризли",
	L"Вики",			//10
	L"Тревер",
	L"Хряп",
	L"Иван",
	L"Анаболик",
	L"Игорь",			//15
	L"Тень",
	L"Рыжий",
	L"Потрошитель",
	L"Фидель",
	L"Лиска",				//20
	L"Сидней",
	L"Гас",
	L"Сдоба",
	L"Айс",
	L"Паук",		//25
	L"Клифф",
	L"Бык",
	L"Стрелок",
	L"Тоска",
	L"Рейдер",		//30
	L"Сова",
	L"Статик",
	L"Лен",
	L"Данни",
	L"Маг",
	L"Стэфен",
	L"Лысый",
	L"Злобный",
	L"Доктор Кью",
	L"Гвоздь",
	L"Тор",
	L"Стрелка",
	L"Волк",
	L"ЭмДи",
	L"Лава",
	//----------
	L"M.I.S. Страх",
	L"Бобби Рэй",
	L"Босс",
	L"Джон Калба",
	L"А.I.М.",
};


static const wchar_t *s_rusGold_pSenderNameList[pSenderNameList_SIZE] = // GOLD and BUKA use slightly different names
{
	L"Энрико",
	L"Псих Про Инк.",
	L"Помощь",
	L"Псих Про Инк.",
	L"Спек",
	L"R.I.S.",		//5
	L"Барри",
	L"Блад",
	L"Рысь",
	L"Гризли",
	L"Вики",			//10
	L"Тревор",
	L"Хряп",
	L"Иван",
	L"Анаболик",
	L"Игорь",			//15
	L"Тень",
	L"Рыжий",
	L"Жнец",
	L"Фидель",
	L"Лиска",				//20
	L"Сидней",
	L"Гас",
	L"Сдоба",
	L"Айс",
	L"Паук",		//25
	L"Скала",
	L"Бык",
	L"Стрелок",
	L"Тоска",
	L"Рейдер",		//30
	L"Сова",
	L"Статик",
	L"Лен",
	L"Данни",
	L"Маг",
	L"Стефан",
	L"Лысый",
	L"Злобный",
	L"Доктор Кью",
	L"Гвоздь",
	L"Тор",
	L"Стрелка",
	L"Волк",
	L"ЭмДи",
	L"Лава",
	//----------
	L"M.I.S. Страх",
	L"Бобби Рэй",
	L"Босс",
	L"Джон Калба",
	L"А.I.М.",
};


// new mail notify string
static const wchar_t s_rus_pNewMailStrings[] = L"Получена новая почта...";


// confirm player's intent to delete messages

static const wchar_t *s_rus_pDeleteMailStrings[pDeleteMailStrings_SIZE] =
{
	L"Стереть сообщение?",
	L"Стереть НЕПРОЧТЕННЫЕ?",
};


// the sort header strings

static const wchar_t *s_rus_pEmailHeaders[pEmailHeaders_SIZE] =
{
	L"От:",
	L"Тема:",
	L"Дата:",
};

// email titlebar text
static const wchar_t s_rus_pEmailTitleText[] = L"Почтовый ящик";


// the financial screen strings
static const wchar_t s_rus_pFinanceTitle[] = L"Бухгалтер Плюс"; // the name we made up for the financial program in the game

static const wchar_t *s_rus_pFinanceSummary[pFinanceSummary_SIZE] =
{
	L"Расход:", 				// credit (subtract from) to player's account
	L"Приход:", 				// debit (add to) to player's account
	L"Прибыль за вчерашний день:",
	L"Другие вклады за вчерашний день:",
	L"Приход за вчерашний день:",
	L"Баланс на конец дня:",
	L"Приход за сегодня:",
	L"Другие вклады за сегодня:",
	L"Приход на сегодня:",
	L"Текущий баланс:",
	L"Предполагаемый приход:",
	L"Предполагаемый баланс:", 		// projected balance for player for tommorow
};


// headers to each list in financial screen

static const wchar_t *s_rus_pFinanceHeaders[pFinanceHeaders_SIZE] =
{
	L"День", 					// the day column
	L"Приход", 				// the credits column
	L"Расход",				// the debits column
	L"Операция", 			// transaction type - see TransactionText below
	L"Баланс", 				// balance at this point in time
	L"Стр.", 				// page number
	L"Дн.", 				// the day(s) of transactions this page displays
};


static const wchar_t *s_rus_pTransactionText[pTransactionText_SIZE] =
{
	L"Проценты",			// interest the player has accumulated so far
	L"Анонимный вклад",
	L"Плата за перевод",
	L"%ls нанят(а) в AIM", // Merc was hired
	L"Покупки у Бобби Рэя", 		// Bobby Ray is the name of an arms dealer
	L"Оплата по счетам M.E.R.C.",
	L"%ls: оплачен мед. депозит", 		// medical deposit for merc
	L"Профилирование в IMP", 		// IMP is the acronym for International Mercenary Profiling
	L"Застрахован(а) %ls",
	L"%ls: cнижена страховка",
	L"%ls: продлена страховка", 				// johnny contract extended
	L"%ls: отмена страховки",
	L"%ls: выплата cтраховки", 		// insurance claim for merc
	L"%ls: контракт продлен на 1 день", 				// entend mercs contract by a day
	L"%ls: контракт продлен на 7 дней",
	L"%ls: контракт продлен на 14 дней",
	L"Доход с шахт",
	L"", //String nuked
	L"Приобретение цветов",
	L"%ls: возврат депозита",
	L"%ls: возврат части депозита",
	L"%ls: удержание депозита",
	L"Выплаты %ls",		// %ls is the name of the npc being paid
	L"Перевод средств на имя %ls", 			// transfer funds to a merc
	L"Перевод средств от %ls", 		// transfer funds from a merc
	L"Ополчение в %ls", // initial cost to equip a town's militia
	L"%ls: оплата покупок.",	//is used for the Shop keeper interface.  The dealers name will be appended to the end of the string.
	L"%ls: вклад наличными.",
};

// helicopter pilot payment

static const wchar_t *s_rus_pSkyriderText[pSkyriderText_SIZE] =
{
	L"Всаднику выплачено $%d", 			// skyrider was paid an amount of money
	L"Всаднику недоплачено $%d", 		// skyrider is still owed an amount of money
	L"У Всадника нет пассажиров. Если вы собирались перевозить бойцов - назначьте им ЗАДАНИЕ ТРАНСПОРТ."
};


// strings for different levels of merc morale

static const wchar_t *s_rus_pMoralStrings[pMoralStrings_SIZE] =
{
	L"Боевой",
	L"Бодрый",
	L"Норма",
	L"Уныние",
	L"Паника",
	L"Разгром",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.
static const wchar_t s_rus_str_left_equipment[]   = L"%ls оставляет снаряжение в %ls (%c%d).";

// Status that appears on the Map Screen

static const wchar_t *s_rus_pMapScreenStatusStrings[pMapScreenStatusStrings_SIZE] =
{
	L"Здоровье",
	L"Энергия",
	L"Настрой",
	L"Состояние",	// the condition of the current vehicle (its "health")
	L"Бензин",	// the fuel level of the current vehicle (its "energy")
};


static const wchar_t *s_rus_pMapScreenPrevNextCharButtonHelpText[pMapScreenPrevNextCharButtonHelpText_SIZE] =
{
	L"Предыдущий наемник (|L|e|f|t)", 			// previous merc in the list
	L"Следующий наемник (|R|i|g|h|t)", 				// next merc in the list
};


static const wchar_t s_rus_pEtaString[] = L"ОВП:"; // eta is an acronym for Estimated Time of Arrival

static const wchar_t *s_rus_pTrashItemText[pTrashItemText_SIZE] =
{
	L"Вы потеряете этот предмет навсегда. Выбросить?", 	// do you want to continue and lose the item forever
	L"Похоже, это действительно ВАЖНАЯ вещь. Вы ТОЧНО уверены, что хотите ее выбросить?", // does the user REALLY want to trash this item
};


static const wchar_t *s_rus_pMapErrorString[pMapErrorString_SIZE] =
{
	L"Отряд не может двигаться, пока бойцы спят.",

//1-5
	L"Сперва выведите отряд на поверхность.",
	L"Приказ двигаться? Тут же кругом враги!",
	L"Боец должен быть членом отряда или пассажиром, чтобы путешествовать.",
	L"У вас в команде пока никого нет",	// you have no members, can't do anything
	L"Боец не может выполнить приказ.",		// merc can't comply with your order
//6-10
	L"%ls нуждается в сопровождении. Назначьте его в отряд.", // merc can't move unescorted .. for a male
	L"%ls нуждается в сопровождении. Назначьте ее в отряд.", // for a female
	L"Наемник еще не прибыл в Арулько!",
	L"Кажется, сначала нужно уладить некоторые проблемы с контрактом.",
	L"",
//11-15
	L"Приказ двигаться? Тут же битва идет!",
	L"Вы попали в засаду кошек-убийц в секторе %ls!",
	L"Вы попали в логово кошек-убийц в секторе I16!",
	L"",
	L"База ПВО в %ls захвачена врагом.",
//16-20
	L"%ls: шахта захвачена. Ваш ежедневный доход упал до %ls в день.",
	L"Противник взял сектор %ls, не встретив сопротивления.",
	L"Как минимум одного из ваших бойцов нельзя назначить на это задание.",
	L"%ls не может вступить в %ls. Отряд полон.",
	L"%ls не может вступить в %ls. Слишком далеко.",
//21-25
	L"%ls: шахта захвачена войсками Дейдраны!",
	L"Войска Дейдраны захватили базу ПВО в %ls",
	L"%ls: в город вошли войска Дейдраны!",
	L"Войска Дейдраны были замечены в %ls.",
	L"Войска Дейдраны захватили %ls.",
//26-30
	L"Как минимум одного из ваших бойцов невозможно уложить спать.",
	L"Как минимум одного из ваших бойцов невозможно разбудить.",
	L"Ополчение небоеспособно, пока не закончится его обучение.",
	L"%ls не может передвигаться прямо сейчас.",
	L"Ополчение, которое находится вне города, нельзя переместить в другой сектор.",
//31-35
	L"%ls не может иметь ополчения.",
	L"Машине нужен водитель!",
	L"%ls слишком изранен(а), чтобы идти!",
	L"Сперва надо покинуть музей!",
	L"%ls мертв(а)!",
//36-40
	L"%ls не может перейти к %ls: он в движении", ///TRNSL_CHECK MEANING
	L"%ls не может сесть в машину так",///TRNSL_CHECK MEANING
	L"%ls не может присоединиться к %ls",///TRNSL_CHECK MEANING
	L"Нельзя ускорять время, пока у вас нет бойцов!",
	L"Эта машина может ездить только по дорогам!",
//41-45
	L"Нельзя переназначать наемников в пути",///TRNSL_CHECK MEANING
	L"У транспорта кончился бензин!",
	L"%ls не может передвигаться из-за усталости.",
	L"Никто из сидящих в машине не может управлять ею.",
	L"Один или более бойцов этого отряда сейчас не могут перемещаться.",
//46-50
	L"Один или более ДРУГИХ бойцов сейчас не могут перемещаться.",///TRNSL_CHECK MEANING
	L"Транспорт слишком сильно поврежден!",
	L"Тренировать ополчение может не больше 2 бойцов на сектор.",
	L"Робот не может двигаться без оператора. Поместите их в один отряд.",
};


// help text used during strategic route plotting
static const wchar_t *s_rus_pMapPlotStrings[pMapPlotStrings_SIZE] =
{
	L"Кликните на секторе еще раз чтобы направить туда отряд, или щелкните по другому сектору для прокладки маршрута.",
	L"Маршрут движения проложен.",
	L"Маршрут не изменился.",
	L"Маршрут отменен.",
	L"Маршрут укорочен.",
};


// help text used when moving the merc arrival sector
static const wchar_t *s_rus_pBullseyeStrings[pBullseyeStrings_SIZE] =
{
	L"Выберите сектор высадки прибывающих наемников.",
	L"Теперь прибывающие наемники будут высаживаться в %ls",
	L"Сюда нельзя лететь, воздух контролирует враг!",
	L"Отмена. Сектор прибытия тот же.",
	L"Полет над %ls опасен! Сектор прибытия перемещен в %ls.",
};


// help text for mouse regions

static const wchar_t *s_rus_pMiscMapScreenMouseRegionHelpText[pMiscMapScreenMouseRegionHelpText_SIZE] =
{
	L"Просмотр инвентаря (|E|n|t|e|r)",
	L"Выкинуть предмет",
	L"Выйти из инвентаря (|E|n|t|e|r)",
};


static const wchar_t s_rus_str_he_leaves_where_drop_equipment[]  = L"%ls должен оставить снаряжение в (%ls) или потом, в %ls (%ls), перед вылетом из Арулько?";
static const wchar_t s_rus_str_she_leaves_where_drop_equipment[] = L"%ls должна оставить свое снаряжение в (%ls) или потом, в %ls (%ls), перед вылетом из Арулько?";
static const wchar_t s_rus_str_he_leaves_drops_equipment[]       = L"%ls собрался уйти, оставив свое снаряжение в %ls.";
static const wchar_t s_rus_str_she_leaves_drops_equipment[]      = L"%ls собралась уйти, оставив свое снаряжение в %ls.";


// Text used on IMP Web Pages

static const wchar_t *s_rus_pImpPopUpStrings[pImpPopUpStrings_SIZE] =
{
	L"Неверный код авторизации!",
	L"Вы уверены, что хотите начать процесс профилирования заново?",
	L"Укажите полное имя и пол",
	L"Изучение вашего счета показало, что у вас нет средств для оплаты профилирования.",
	L"Сейчас вы не можете это выбрать.",
	L"Чтобы закончить анализ, нужно иметь место еще хотя бы для одного члена команды.",
	L"Анализ уже пройден.",
};


// button labels used on the IMP site

static const wchar_t *s_rus_pImpButtonText[pImpButtonText_SIZE] =
{
	L"Подробнее", 			// about the IMP site
	L"НАЧАТЬ", 			// begin profiling
	L"Личность", 		// personality section
	L"Свойства", 		// personal stats/attributes section
	L"Портрет", 			// the personal portrait selection
	L"Голос %d", 			// the voice selection
	L"Готово", 			// done profiling
	L"Заново", 		// start over profiling
	L"Да, выбрать выделенный ответ.",
	L"Да",
	L"Нет",
	L"Закончить", 			// finished answering questions
	L"Назад", 			// previous question..abbreviated form
	L"Дальше", 			// next question
	L"ДА.", 		// yes, I am certain
	L"НЕТ, ХОЧУ НАЧАТЬ СНАЧАЛА.", // no, I want to start over the profiling process
	L"ДА.",
	L"НЕТ",
	L"Назад", 			// back one page
	L"Отмена", 			// cancel selection
	L"Да, так и есть.",
	L"Нет, просмотреть еще раз.",
	L"Регистрация", 			// the IMP site registry..when name and gender is selected
	L"Анализ", 			// analyzing your profile results
	L"OK",
	L"Голос",
};

static const wchar_t *s_rus_pExtraIMPStrings[pExtraIMPStrings_SIZE] =
{
	L"Чтобы начать профилирование, протестируйте свои личные качества.",
	L"Личные качества оценены, переходите к свойствам.",
	L"Свойства зафиксированы, выберите наиболее подходящее вам лицо.",
	L"Чтобы завершить процесс, выберите подходящий голос."
};

static const wchar_t s_rus_pFilesTitle[] = L"Просмотр файлов";

static const wchar_t *s_rus_pFilesSenderList[pFilesSenderList_SIZE] =
{
	L"Отчет разведки", 		// the recon report sent to the player. Recon is an abbreviation for reconissance
	L"Перехват #1", 		// first intercept file .. Intercept is the title of the organization sending the file...similar in function to INTERPOL/CIA/KGB..refer to fist record in files.txt for the translated title
	L"Перехват #2",	   // second intercept file
	L"Перехват #3",			 // third intercept file
	L"Перехват #4", // fourth intercept file
	L"Перехват #5", // fifth intercept file
	L"Перехват #6", // sixth intercept file
};

// Text having to do with the History Log
static const wchar_t s_rus_pHistoryTitle[] = L"События";

static const wchar_t *s_rus_pHistoryHeaders[pHistoryHeaders_SIZE] =
{
	L"День", 			// the day the history event occurred
	L"Стр.", 			// the current page in the history report we are in
	L"День", 			// the days the history report occurs over
	L"Место", 			// location (in sector) the event occurred
	L"Событие", 			// the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
static const wchar_t *s_rus_pHistoryStrings[pHistoryStrings_SIZE] =
{
	L"",																						// leave this line blank
	//1-5
	L"%ls нанят(а) в A.I.M.", 										// merc was hired from the aim site
	L"%ls нанят(а) в M.E.R.C.", 									// merc was hired from the aim site
	L"%ls мертв(а).", 															// merc was killed
	L"Оплата счетов M.E.R.C.",								// paid outstanding bills at MERC
	L"Принято задание от Энрико Чивалдори",
	//6-10
	L"Прошел профилирование в IMP",
	L"%ls: жизнь застрахована", 				// insurance contract purchased
	L"%ls: страховка отменена", 				// insurance contract canceled
	L"%ls: страховка выплачена", 							// insurance claim payout for merc
	L"%ls: контракт продлен на день.", 						// Extented "mercs name"'s for a day
	//11-15
	L"%ls: контракт продлен на 7 дней", 					// Extented "mercs name"'s for a week
	L"%ls: контракт продлен на 14 дней", 					// Extented "mercs name"'s 2 weeks
	L"%ls уволен(а).", // "merc's name" was dismissed.
	L"%ls ушел.", // "merc's name" quit.
	L"начат.", // a particular quest started
	//16-20
	L"завершен.",
	L"%ls: состоялся разговор с начальником шахты.",	// talked to head miner of town
	L"%ls освобожден(а).",
	L"Читы включены.",
	L"Провизия завтра будет в Омерте.",
	//21-25
	L"%ls ушла из команды и вышла замуж за Хика",
	L"%ls: контракт окончен.",
	L"%ls в команде.",
	L"Энрико недоволен отсутствием прогресса.",
	L"Битва выиграна.",
	//26-30
	L"%ls: в шахте кончается руда",
	L"%ls: шахта выработана",
	L"%ls: шахта закрыта",
	L"%ls: шахта вновь открыта",
	L"Получил сведения о тюрьме Тикса.",
	//31-35
	L"Услышал о секретном военном заводе Орта.",
	L"Ученый из Орты помог с ракетным ружьем.",
	L"Дейдранна что-то делает с трупами.",
	L"Фрэнк рассказал о поединках в Сан Моне.",
	L"Пациент думает, что видел в шахтах что-то.",
	//36-40
	L"Встретил какого-то Девина - торгует взрывчаткой.",
	L"Столкнулся со знаменитым Майком!",
	L"Встретил Тони - он занимается оружием.",
	L"Получил ракетное ружье от сержанта Кротта.",
	L"Передал Кайлу закладную на магазин Энжела.",
	//41-45
	L"Шиз предлагает сделать робота.",
	L"Болтун может сделать состав против жуков.",
	L"Кейт больше не при делах.",
	L"Говард обеспечивает Дейдрану цианидом.",
	L"Встретил Кейта - барыжит всем подряд в Камбрии.",
	//46-50
	L"Встретил Говарда - фармацевта из Балайма.",
	L"Встретил Перко - у него маленький ремонтный бизнес.",
	L"Встретил Сэма из Балайма, у него компьютерный магазин.",
	L"Франц торгует электроникой и другими вещами.",
	L"У Арнольда ремонтная мастерская в Граме.",
	//51-55
	L"Фредо ремонтирует электронику в Граме.",
	L"Получил пожертвование от богача из Балайма.",
	L"Встретил старьевщика по имени Джейк.",
	L"Бродяга дал нам электронный ключ.",
	L"Подкупил Вальтера, чтобы он открыл дверь в подвал.",
	//56-60
	L"Дэвид заправит нас даром, если у него есть бензин.",
	L"Дал на лапу Пабло.",
	L"Босс хранит деньги в шахте Сан Моны.",
	L"%ls выиграл кулачный бой",
	L"%ls проиграл кулачный бой",
	//61-65
	L"%ls: дисквалификация в кулачном бою",
	L"Нашел много денег в заброшенной шахте.",
	L"Столкнулся с убийцей, посланным Боссом.",
	L"Утрачен контроль над сектором",				//ENEMY_INVASION_CODE
	L"Успешная оборона сектора",
	//66-70
	L"Поражение в бою",							//ENEMY_ENCOUNTER_CODE
	L"Засада",						//ENEMY_AMBUSH_CODE
	L"Победа над засадой врага",
	L"Безуспешная атака",			//ENTERING_ENEMY_SECTOR_CODE
	L"Успешная атака!",
	//71-75
	L"Нападение существ!",			//CREATURE_ATTACK_CODE
	L"Атака кошек-убийц",			//BLOODCAT_AMBUSH_CODE
	L"Перебил кошек-убийц",
	L"%ls убит(а)",
	L"Отдал голову террориста Кармену",
	L"Убийца ушел",
	L"Убил %ls",
};

static const wchar_t s_rus_pHistoryLocations[] = L"Н/П"; // N/A is an acronym for Not Applicable

// icon text strings that appear on the laptop

static const wchar_t *s_rus_pLaptopIcons[pLaptopIcons_SIZE] =
{
	L"Почта",
	L"Сеть",
	L"Финансы",
	L"Команда",
	L"Журнал",
	L"Файлы",
	L"Выключить",
	L"сир-ФЕР 4.0",			// our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added

static const wchar_t *s_rus_pBookMarkStrings[pBookMarkStrings_SIZE] =
{
	L"А.I.M.",
	L"Бобби Рэй",
	L"I.M.P.",
	L"М.Е.R.С.",
	L"Похороны",
	L"Цветы",
	L"Страховка",
	L"Отмена",
};

// When loading or download a web page

static const wchar_t *s_rus_pDownloadString[pDownloadString_SIZE] =
{
	L"Загрузка...",
	L"Перегрузка...",
};

//This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

static const wchar_t *s_rus_gsAtmStartButtonText[gsAtmStartButtonText_SIZE] =
{
	L"Параметры", 			// view stats of the merc
	L"Снаряжение", 			// view the inventory of the merc
	L"Контракт",
};

// Web error messages. Please use German equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator

static const wchar_t s_rus_pErrorStrings[] = L"Плохое соединение. Попробуйте позднее.";


static const wchar_t s_rus_pPersonnelString[] = L"Бойцов:"; // mercs we have


static const wchar_t s_rus_pWebTitle[] = L"сир-ФЕР 4.0"; // our name for the version of the browser, play on company name


// The titles for the web program title bar, for each page loaded

static const wchar_t *s_rus_pWebPagesTitles[pWebPagesTitles_SIZE] =
{
	L"А.I.M.",
	L"A.I.M. - Наемники",
	L"A.I.M. - Фотографии ",		// a mug shot is another name for a portrait
	L"A.I.M. - Сортировка",
	L"A.I.M.",
	L"A.I.M. - Бывшие члены",
	L"A.I.M. - Правила",
	L"A.I.M. - История",
	L"A.I.M. - Ссылки",
	L"M.E.R.C.",
	L"M.E.R.C. - Счета",
	L"M.E.R.C. - Регистрация",
	L"M.E.R.C. - Индекс",
	L"Бобби Рэй",
	L"Бобби Рэй - Оружие",
	L"Бобби Рэй - Боеприпасы",
	L"Бобби Рэй - Броня",
	L"Бобби Рэй - Разное",							//misc is an abbreviation for miscellaneous
	L"Бобби Рэй - Бывшее в Употреблении",
	L"Бобби Рэй - Бланк заказа",
	L"I.M.P.",
	L"I.M.P.",
	L"Цветы по Всему Миру",
	L"Цветы по Всему Миру - Галерея",
	L"Цветы по Всему Миру - Бланк Заказа",
	L"Цветы по Всему Миру - Открытки",
	L"Страховые агенты Малеус, Инкус и Стэйпс",
	L"Информация",
	L"Контракт",
	L"Комментарии",
	L"Похоронное агенство Макгилликати",
	L"URL не найден.",
	L"Бобби Рэй - Последние поставки",//@@@3 Translate new text
	L"",
	L"",
};

static const wchar_t *s_rus_pShowBookmarkString[pShowBookmarkString_SIZE] =
{
	L"Sir-Помощь",
	L"Закладки: кликните на кнопке Сеть еще раз.",
};

static const wchar_t *s_rus_pLaptopTitles[pLaptopTitles_SIZE] =
{
	L"Почтовый ящик",
	L"Просмотр файлов",
	L"Команда",
	L"Бухгалтер Плюс",
	L"Журнал",
};

static const wchar_t *s_rus_pPersonnelDepartedStateStrings[pPersonnelDepartedStateStrings_SIZE] =
{
	//reasons why a merc has left.
	L"Убит в бою",
	L"Уволен",
	L"Замужем",///TRNSL ASSUME only female merc can depart from team by marrying somebody
	L"Контракт истек",
	L"Ушел",
};
// personnel strings appearing in the Personnel Manager on the laptop

static const wchar_t *s_rus_pPersonelTeamStrings[pPersonelTeamStrings_SIZE] =
{
	L"В команде",
	L"Убывшие",
	L"Гонорар в день:",
	L"Макс. гонорар:",
	L"Мин. гонорар:",
	L"Погибли в бою:",
	L"Уволены:",
	L"Другое:",
};


static const wchar_t *s_rus_pPersonnelCurrentTeamStatsStrings[pPersonnelCurrentTeamStatsStrings_SIZE] =
{
	L"Минимум",
	L"Среднее",
	L"Максимум",
};


static const wchar_t *s_rus_pPersonnelTeamStatsStrings[pPersonnelTeamStatsStrings_SIZE] =
{
	L"ЗДОР",
	L"ПРВ",
	L"ЛОВ",
	L"СИЛ",
	L"ЛДР",
	L"МДР",
	L"ОПЫТ",
	L"МТК",
	L"МЕХ",
	L"ВЗРВ",
	L"МЕД",
};


// horizontal and vertical indices on the map screen

static const wchar_t *s_rus_pMapVertIndex[pMapVertIndex_SIZE] =
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

static const wchar_t *s_rus_pMapHortIndex[pMapHortIndex_SIZE] =
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

static const wchar_t *s_rus_pMapDepthIndex[pMapDepthIndex_SIZE] =
{
	L"",
	L"-1",
	L"-2",
	L"-3",
};

// text that appears on the contract button

static const wchar_t s_rus_pContractButtonString[] = L"Контракт";

// text that appears on the update panel buttons

static const wchar_t *s_rus_pUpdatePanelButtons[pUpdatePanelButtons_SIZE] =
{
	L"Дальше",
	L"Стоп",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle

static StrPointer s_rus_LargeTacticalStr[LargeTacticalStr_SIZE] =
{
	L"В этом секторе вам нанесли поражение!",
	L"Враг, без тени сожаления, пожрет всех до единого!",
	L"Члены вашей команды захвачены (без сознания)!",
	L"Члены вашей команды захвачены в плен врагом.",
};


//Insurance Contract.c
//The text on the buttons at the bottom of the screen.

static const wchar_t *s_rus_InsContractText[InsContractText_SIZE] =
{
	L"Назад",
	L"Далее",
	L"ОК",
	L"Очистить",
};



//Insurance Info
// Text on the buttons on the bottom of the screen

static const wchar_t *s_rus_InsInfoText[InsInfoText_SIZE] =
{
	L"Назад",
	L"Далее"
};



//For use at the M.E.R.C. web site. Text relating to the player's account with MERC

static const wchar_t *s_rus_MercAccountText[MercAccountText_SIZE] =
{
	// Text on the buttons on the bottom of the screen
	L"Оплатить",
	L"Главная",
	L"Счет #:",
	L"Наемник",
	L"Дни",
	L"Ставка",	//5
	L"Плата",
	L"Всего:",
	L"Подтверждаете выплату %ls?",		//the %ls is a string that contains the dollar amount ( ex. "$150" )
};



//For use at the M.E.R.C. web site. Text relating a MERC mercenary


static const wchar_t *s_rus_MercInfo[MercInfo_SIZE] =
{
	L"Назад",
	L"Нанять",
	L"Дальше",
	L"Дополнительная информация",
	L"На главную",
	L"В команде",
	L"Зарплата:",
	L"/день",
	L"Мертв(а)",

	L"Похоже, вы увлеклись набором наемников. Не более 18 бойцов в команде.",
	L"Отсутствует",
};



// For use at the M.E.R.C. web site. Text relating to opening an account with MERC

static const wchar_t *s_rus_MercNoAccountText[MercNoAccountText_SIZE] =
{
	//Text on the buttons at the bottom of the screen
	L"Открыть счет",
	L"Отмена",
	L"У вас нет счета. Хотите открыть?"
};



// For use at the M.E.R.C. web site. MERC Homepage

static const wchar_t *s_rus_MercHomePageText[MercHomePageText_SIZE] =
{
	//Description of various parts on the MERC page
	L"Спек Т. Клайн, основатель",
	L"Открытие счета",
	L"Просмотр счета",
	L"Личные дела",
	// The version number on the video conferencing system that pops up when Speck is talking
	L"Спек Ком v3.2"
};

// For use at MiGillicutty's Web Page.

static const wchar_t *s_rus_sFuneralString[sFuneralString_SIZE] =
{
	L"Похоронное агенство Макгилликати: скорбим вместе с семьями усопших с 1983 года.",
	L"Директор по похоронам и бывший наемник А.I.М Мюррэй \"Папаша\" Макгилликати - специалист по части похорон.",
	L"Всю жизнь Папашу сопровождали смерть и утраты, поэтому он, как никто, познал их тяжесть.",
	L"Агенство Макгилликати предлагает широкий спектр похоронных услуг - от жилетки, в которую можно поплакать, до восстановления сильно поврежденных останков.",
	L"Доверьтесь похоронному агенству Макгилликати, и ваши родственники почиют в мире.",

	// Text for the various links available at the bottom of the page
	L"ДОСТАВКА ЦВЕТОВ",
	L"КОЛЛЕКЦИЯ УРН И ГРОБОВ",
	L"УСЛУГИ ПО КРЕМАЦИИ",
	L"ПОМОЩЬ В ПРОВЕДЕНИИ ПОХОРОН",
	L"ПОХОРОННЫЕ РИТУАЛЫ",

	// The text that comes up when you click on any of the links ( except for send flowers ).
	L"К сожалению, наш сайт не закончен, в связи с утратой в семье. Мы постараемся продолжить работу после прочтения завещания и выплат долгов умершего.",
	L"Однако, мы все равно скорбим вместе с вами. Пожалуйста, приходите еще."
};

// Text for the florist Home page

static const wchar_t *s_rus_sFloristText[sFloristText_SIZE] =
{
	//Text on the button on the bottom of the page

	L"Галерея",

	//Address of United Florist

	L"\"Авиадоставка по всему миру\"",
	L"1-555-SCENT-ME",
	L"333 Др. Носич, Семябург, КА США 90210",
	L"http://www.scent-me.com",

	// detail of the florist page

	L"Работаем быстро и эффективно!",
	L"Гарантированная доставка в течение одного дня в любую точку земного шара. Есть ограничения.",
	L"Самые низкие в мире цены!",
	L"Покажите нам рекламу подобных услуг, которые стоят дешевле и получите букет роз бесплатно.",
	L"Летающая Флора, Фауна & Цветы с 1981.",
	L"Наши летчики, в прошлом военные бомбардировщики, сбросят ваш букет в радиусе 10 км от цели. В любое время! Всегда!",
	L"Позвольте нам воплотить ваши цветочные фантазии в жизнь.",
	L"Брюс, наш всемирно известный флорист, собственноручно соберет вам букет свежайших цветов из наших оранжерей.",
	L"И помните, если у нас чего-то нет - мы это вырастим! И быстро!"
};



//Florist OrderForm

static const wchar_t *s_rus_sOrderFormText[sOrderFormText_SIZE] =
{
	//Text on the buttons

	L"Назад",
	L"Послать",
	L"Очистить",
	L"Галерея",

	L"Букет:",
	L"Цена:",			//5
	L"Номер заказа:",
	L"Срочность",
	L"завтра",
	L"дойдет, когда дойдет",
	L"Место доставки",			//10
	L"Доп. услуги",
	L"Сломанные цветы($10)",
	L"Черные розы($20)",
	L"Увядший букет($10)",
	L"Фруктовый пирог(если есть)($10)",		//15
	L"Ваше пожелание:",
	L"Ввиду небольшого размера открыток - не более 75 символов.",
	L"...или выберите из наших",

	L"СТАНДАРТНЫХ ПОЖЕЛАНИЙ",
	L"Информация о дарителе",//20

	//The text that goes beside the area where the user can enter their name

	L"Имя:",
};




//Florist Gallery.c

static const wchar_t *s_rus_sFloristGalleryText[sFloristGalleryText_SIZE] =
{
	//text on the buttons

	L"Назад",	//abbreviation for previous
	L"Далее",	//abbreviation for next

	L"Кликните по желаемому букету.",
	L"Примечание: можно заказать увядший или сломанный букет за дополнительные $10.",

	//text on the button

	L"Главная",
};

//Florist Cards

static const wchar_t *s_rus_sFloristCards[sFloristCards_SIZE] =
{
	L"Кликните для выбора",
	L"Назад"
};



// Text for Bobby Ray's Mail Order Site

static const wchar_t *s_rus_BobbyROrderFormText[BobbyROrderFormText_SIZE] =
{
	L"Бланк заказа",				//Title of the page
	L"Шт.",					// The number of items ordered
	L"Вес (%ls)",			// The weight of the item
	L"Наименование",				// The name of the item
	L"Цена",				// the item's price
	L"Всего",				//5	// The total price of all of items of the same type
	L"Цена товаров",				// The sub total of all the item totals added
	L"Доставка и упаковка",		// S&H is an acronym for Shipping and Handling
	L"Цена с доставкой",			// The grand total of all item totals + the shipping and handling
	L"Место доставки",
	L"Тип доставки",			//10	// See below
	L"Цена (за %ls.)",			// The cost to ship the items
	L"Доставка наутро",			// Gets deliverd the next day
	L"2 рабочих дня",			// Gets delivered in 2 days
	L"Стандартный срок",			// Gets delivered in 3 days
	L"Очистить",//15			// Clears the order page
	L"Принять заказ",			// Accept the order
	L"Назад",				// text on the button that returns to the previous page
	L"Главная",				// Text on the button that returns to the home page
	L"* обозначает товары Б/У",		// Disclaimer stating that the item is used
	L"У вас нет на это средств.",		//20	// A popup message that to warn of not enough money
	L"<ПУСТО>",				// Gets displayed when there is no valid city selected
	L"Место доставки заказа %ls. Уверены?",		// A popup that asks if the city selected is the correct one
	L"Вес товаров**",			// Displays the weight of the package
	L"** минимальный вес заказа",				// Disclaimer states that there is a minimum weight for the package
	L"Заказы",
};


// This text is used when on the various Bobby Ray Web site pages that sell items

static const wchar_t *s_rus_BobbyRText[BobbyRText_SIZE] =
{
	L"Правила:",				// Title
	// instructions on how to order
	L"Кликните на предмет. Хотите больше - кликайте еще. Убрать предмет из заказа - правый клик. Закончив выбор покупок, заполните бланк заказа.",

	//Text on the buttons to go the various links

	L"Назад",		//
	L"Оружие", 			//3
	L"Амуниция",			//4
	L"Броня",			//5
	L"Разное",			//6	//misc is an abbreviation for miscellaneous
	L"Б/У",			//7
	L"Дальше",
	L"ЗАКАЗАТЬ",
	L"Главная",			//10

	//The following lines provide information on the items

	L"Вес:",		// Weight of all the items of the same type
	L"Калибр:",			// the caliber of the gun
	L"Магаз.:",			// number of rounds of ammo the Magazine can hold
	L"Дальноб.:",			// The range of the gun
	L"Урон:",			// Damage of the weapon
	L"Скор.:",			// Weapon's Rate Of Fire, acronym ROF
	L"Цена:",			// Cost of the item
	L"Склад:",		// The number of items still in the store's inventory
	L"Заказано, шт:",		// The number of items on order
	L"Повреждение",			// If the item is damaged
	L"Итого:",			// The total cost of all items on order
	L"* процент исправности",		// if the item is damaged, displays the percent function of the item

	//Popup that tells the player that they can only order 10 items at a time

	L"Дорогие клиенты! Онлайновая форма позволяет заказать не более 10 позиций за раз. Если вы хотите заказать больше, (а мы надемся, что так и есть), заполните еще один бланк и примите наши извинения.",

	// A popup that tells the user that they are trying to order more items then the store has in stock

	L"Извините. Этот товар закончился на складе. Попробуйте заказать его позже.",

	//A popup that tells the user that the store is temporarily sold out

	L"Извините, но все товары этого типа закончились на складе.",
};


/* The following line is used on the Ammunition page.  It is used for help text
 * to display how many items the player's merc has that can use this type of
 * ammo. */
static const wchar_t s_rus_str_bobbyr_guns_num_guns_that_use_ammo[] = L"Количество единиц оружия у вашей команды, использующих этот боеприпас: %d";


// Text for Bobby Ray's Home Page

static const wchar_t *s_rus_BobbyRaysFrontText[BobbyRaysFrontText_SIZE] =
{
	//Details on the web site

	L"Только у нас можно приобрести последние новинки в оружейном деле",
	L"Прекрасный выбор взрывчатки и аксессуаров на любой вкус",
	L"Б/У",

	//Text for the various links to the sub pages

	L"РАЗНОЕ",
	L"ОРУЖИЕ",
	L"АМУНИЦИЯ",		//5
	L"БРОНЯ",

	//Details on the web site

	L"Нет у нас - нет нигде!",
	L"В разработке",
};



// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug shot page

static const wchar_t *s_rus_AimSortText[AimSortText_SIZE] =
{
	L"Наемники А.I.M.",				// Title
	// Title for the way to sort
	L"Сортировка:",

	//Text of the links to other AIM pages

	L"Просмотреть фото наемников",
	L"Просмотреть характеристики наемников",
	L"Просмотреть галерею бывших членов А.I.M."
};


// text to display how the entries will be sorted
static const wchar_t s_rus_str_aim_sort_price[]        = L"Цена";
static const wchar_t s_rus_str_aim_sort_experience[]   = L"Опытность";
static const wchar_t s_rus_str_aim_sort_marksmanship[] = L"Меткость";
static const wchar_t s_rus_str_aim_sort_medical[]      = L"Медицина";
static const wchar_t s_rus_str_aim_sort_explosives[]   = L"Взрывчатка";
static const wchar_t s_rus_str_aim_sort_mechanical[]   = L"Механика";
static const wchar_t s_rus_str_aim_sort_ascending[]    = L"по возрастанию";
static const wchar_t s_rus_str_aim_sort_descending[]   = L"по убыванию   ";


//Aim Policies.c
//The page in which the AIM policies and regulations are displayed

static const wchar_t *s_rus_AimPolicyText[AimPolicyText_SIZE] =
{
	// The text on the buttons at the bottom of the page

	L"Назад",
	L"Главная",
	L"Правила",
	L"Дальше",
	L"Не согласен",
	L"Согласен"
};



//Aim Member.c
//The page in which the players hires AIM mercenaries

// Instructions to the user to either start video conferencing with the merc, or to go the mug shot index

static const wchar_t *s_rus_AimMemberText[AimMemberText_SIZE] =
{
	L"Левый щелчок",
	L"связаться",
	L"Правый щелчок",
	L"назад к фото",
};

//Aim Member.c
//The page in which the players hires AIM mercenaries

static const wchar_t *s_rus_CharacterInfo[CharacterInfo_SIZE] =
{
	// the contract expenses' area

	L"Плата",
	L"Срок",
	L"1 день",
	L"7 дней",
	L"14 дней",

	// text for the buttons that either go to the previous merc,
	// start talking to the merc, or go to the next merc

	L"<<<<",
	L"Связь",
	L">>>>",

	L"Дополнительная информация",				// Title for the additional info for the merc's bio
	L"Действующие наемники A.I.M.", // Title of the page
	L"Цена снаряжения:",				// Displays the optional gear cost
	L"медицинский депозит",			// If the merc required a medical deposit, this is displayed
};


//Aim Member.c
//The page in which the player's hires AIM mercenaries

//The following text is used with the video conference popup

static const wchar_t *s_rus_VideoConfercingText[VideoConfercingText_SIZE] =
{
	L"Цена контракта:",				//Title beside the cost of hiring the merc

	//Text on the buttons to select the length of time the merc can be hired

	L"1 день",
	L"7 дней",
	L"14 дней",

	//Text on the buttons to determine if you want the merc to come with the equipment

	L"Без снаряжения",
	L"Со снаряжением",

	// Text on the Buttons

	L"ОПЛАТИТЬ",			// to actually hire the merc
	L"ОТМЕНА",				// go back to the previous menu
	L"НАНЯТЬ",				// go to menu in which you can hire the merc
	L"ОТБОЙ",				// stops talking with the merc
	L"OK",
	L"СООБЩЕНИЕ",			// if the merc is not there, you can leave a message

	//Text on the top of the video conference popup

	L"Видеоконференция, абонент",
	L"Соединение. . .",

	L"с депозитом"			// Displays if you are hiring the merc with the medical deposit
};



//Aim Member.c
//The page in which the player hires AIM mercenaries

// The text that pops up when you select the TRANSFER FUNDS button

static const wchar_t *s_rus_AimPopUpText[AimPopUpText_SIZE] =
{
	L"ЭЛЕКТРОННЫЙ ПЛАТЕЖ ПРОШЕЛ УСПЕШНО",	// You hired the merc
	L"ОШИБКА! НЕТ",		// Player doesn't have enough money, message 1
	L"СРЕДСТВ НА СЧЕТУ!",				// Player doesn't have enough money, message 2

	// if the merc is not available, one of the following is displayed over the merc's face

	L"На задании",
	L"Оставьте сообщение",
	L"Скончался",

	//If you try to hire more mercs than game can support

	L"У вас уже набрано 18 наемников - полная команда.",

	L"АВТО ОТВЕТЧИК",
	L"Сообщение записано",
};


//AIM Link.c

static const wchar_t s_rus_AimLinkText[] = L"Ссылки A.I.M.";	// The title of the AIM links page



//Aim History

// This page displays the history of AIM

static const wchar_t *s_rus_AimHistoryText[AimHistoryText_SIZE] =
{
	L"История A.I.M.",					//Title

	// Text on the buttons at the bottom of the page

	L"Назад",
	L"Главная",
	L"Галерея",
	L"Дальше"
};


//Aim Mug Shot Index

//The page in which all the AIM members' portraits are displayed in the order selected by the AIM sort page.

static const wchar_t *s_rus_AimFiText[AimFiText_SIZE] =
{
	// displays the way in which the mercs were sorted

	L"цены",
	L"опытности",
	L"меткости",
	L"медицины",
	L"саперных навыков",
	L"навыков механика",

	// The title of the page, the above text gets added at the end of this text

	L"Список наемников по возрастанию %ls",
	L"Список наемников по убыванию %ls",

	// Instructions to the players on what to do

	L"Левый щелчок",
	L"выбор наемника",			//10
	L"Правый щелчок",
	L"настройка сортировки",

	// Gets displayed on top of the merc's portrait if they are...

	L"Скончался",						//14
	L"На задании",
};



//AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer with AIM

static const wchar_t *s_rus_AimAlumniText[AimAlumniText_SIZE] =
{
	// Text of the buttons

	L"СТР 1",
	L"СТР 2",
	L"СТР 3",

	L"Галерея бывших членов A.I.M.",	// Title of the page //$$

	L"ОК"			// Stops displaying information on selected merc
};






//AIM Home Page

static const wchar_t *s_rus_AimScreenText[AimScreenText_SIZE] =
{
	// AIM disclaimers

	L"A.I.M. и логотип A.I.M. - зарегистрированные во многих странах торговые марки.",
	L"Поэтому даже и не думайте нас копировать.",
	L"Копирайт 1998-1999 A.I.M.,Ltd. Все права защищены.",

	//Text for an advertisement that gets displayed on the AIM page

	L"Цветы по Всему Миру",
	L"\"Авиадоставка куда угодно\"",				//10
	L"Сделай как надо",
	L"... первый раз",
	L"Если у нас чего-то нет - оно вам и не нужно.",
};


//Aim Home Page

static const wchar_t *s_rus_AimBottomMenuText[AimBottomMenuText_SIZE] =
{
	//Text for the links at the bottom of all AIM pages
	L"Главная",
	L"Наемники",
	L"Галерея", //$$
	L"Правила",
	L"История",
	L"Ссылки"
};



//ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.

static const wchar_t *s_rus_SKI_Text[SKI_SIZE ] =
{
	L"ТОВАРЫ В ПРОДАЖЕ",		//Header for the merchandise available
	L"СТР",				//The current store inventory page being displayed
	L"ОБЩАЯ ЦЕНА",				//The total cost of the the items in the Dealer inventory area
	L"ОБЩАЯ ЦЕНА",			//The total value of items player wishes to sell
	L"ОЦЕНКА",				//Button text for dealer to evaluate items the player wants to sell
	L"ПО РУКАМ",			//Button text which completes the deal. Makes the transaction.
	L"ГОТОВО",				//Text for the button which will leave the shopkeeper interface.
	L"ЦЕНА РЕМОНТА",			//The amount the dealer will charge to repair the merc's goods
	L"1 ЧАС",			// SINGULAR! The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"%d ЧАСОВ",		// PLURAL!   The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"ОТРЕМОНТИРОВАНО",		// Text appearing over an item that has just been repaired by a NPC repairman dealer
	L"Вам уже некуда класть вещи.",	//Message box that tells the user there is no more room to put there stuff
	L"%d МИНУТ",		// The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"Бросить на землю", //hint about dropping item on the ground from ShopKeeper screen
};


static const wchar_t *s_rus_SkiMessageBoxText[SkiMessageBoxText_SIZE] =
{
	L"Cнять %ls со своего основного счета, чтобы покрыть разницу?",
	L"Недостаточно денег. Не хватает %ls",
	L"Cнять %ls со своего основного счета, чтобы оплатить покупку?",
	L"Предложить торговцу совершить сделку",
	L"Попросить торговца починить выбранные вещи",
	L"Закончить разговор",
	L"Текущий баланс",
};


//OptionScreen.c

static const wchar_t *s_rus_zOptionsText[zOptionsText_SIZE] =
{
	//button Text
	L"Сохранить",
	L"Загрузить",
	L"Выход",
	L"Готово",

	//Text above the slider bars
	L"Звуки",
	L"Речь",
	L"Музыка",

	//Confirmation pop when the user selects..
	L"Выйти из игры и вернуться в главное меню?",

	L"Нужно выбрать либо РЕЧЬ, либо СУБТИТРЫ.",
};


//SaveLoadScreen
static const wchar_t *s_rus_zSaveLoadText[zSaveLoadText_SIZE] =
{
	L"Сохранить",
	L"Загрузить",
	L"Отмена",
	L"Сохранить выбранную.", //string arent used in game
	L"Загрузить выбранную", //string arent used in game

	L"Игра сохранена",
	L"Ошибка при сохранении!",
	L"Игра загружена",
	L"Ошибка при загрузке: \"%hs\"",

	L"Версия сохраненной игры отличается от текущей. Скорее всего, игра загрузится нормально. Загрузить?",
	L"Файлы сохраненной игры возможно испорчены. Стереть все?",

	//Translators, the next two strings are for the same thing.  The first one is for beta version releases and the second one
	//is used for the final version.  Please don't modify the "#ifdef JA2BETAVERSION" or the "#else" or the "#endif" as they are
	//used by the compiler and will cause program errors if modified/removed.  It's okay to translate the strings though.
#ifdef JA2BETAVERSION
	L"Версия сохранения изменилась. Сообщите, если будут проблемы. Продолжить?",
#else
	L"Попытка загрузки старой версии сохранения. Обновить автоматически и загрузить?",
#endif

	//Translators, the next two strings are for the same thing.  The first one is for beta version releases and the second one
	//is used for the final version.  Please don't modify the "#ifdef JA2BETAVERSION" or the "#else" or the "#endif" as they are
	//used by the compiler and will cause program errors if modified/removed.  It's okay to translate the strings though.
#ifdef JA2BETAVERSION
	L"Версия сохранения изменилась. Сообщите, если будут проблемы. Продолжить?",
#else
	L"Попытка загрузки старой версии сохранения. Обновить автоматически и загрузить?",
#endif

	L"Вы уверены, что хотите перезаписать сохраненную игру #%d?",


	//The first %d is a number that contains the amount of free space on the users hard drive,
	//the second is the recommended amount of free space.
	L"У вас кончается дисковое пространство. Осталось всего %d Mбайт. Нужно как минимум %d свободных Mбайт.",

	L"Сохранение...",			//When saving a game, a message box with this string appears on the screen

	L"Оружие: реализм",
	L"Оружие: тонны",
	L"Стиль: реализм",
	L"Стиль: фантаст.",

	L"",
};



//MapScreen
static const wchar_t *s_rus_zMarksMapScreenText[zMarksMapScreenText_SIZE] =
{
	L"Уровень карты",
	L"У вас нет ополчения. Подготовьте его из гражданского населения.",
	L"Доход в день",
	L"Жизнь наемника застрахована",
	L"%ls не устал(а).",
	L"%ls движется и спать не может",
	L"%ls слишком устал(а), попробуйте позже.",
	L"%ls за рулем.",
	L"Отряд не может двигаться, пока кто-то из бойцов спит.",

	// stuff for contracts
	L"Вы можете платить по контракту, но у вас нет денег на оплату страхования жизни.",
	L"%ls застрахован(а) на случай гибели. Для продления страховки надо доплатить %ls за %d дней. Согласны?",
	L"Предметы в секторе",
	L"У наемника есть медицинский депозит",

	// other items
	L"Медики", // people acting a field medics and bandaging wounded mercs
	L"Пациенты", // people who are being bandaged by a medic
	L"Готово", // Continue on with the game after autobandage is complete
	L"Стоп", // Stop autobandaging of patients by medics now
	L"%ls не может этого сделать, нет инструментов для ремонта.",
	L"%ls не может этого сделать, нет медицинских принадлежностей.",
	L"Недостаточно желающих пройти подготовку.",
	L"%ls: слишком много ополченцев.",
	L"У наемника ограниченный по времени контракт",
	L"У наемника нет страховки",
};


static const wchar_t s_rus_pLandMarkInSectorString[] = L"Отряд %d заметил кого-то в секторе %ls";

// confirm the player wants to pay X dollars to build a militia force in town
static const wchar_t *s_rus_pMilitiaConfirmStrings[pMilitiaConfirmStrings_SIZE] =
{
	L"Тренировка отряда ополчения будет стоить $", // telling player how much it will cost
	L"Оплатить расходы?", // asking player if they wish to pay the amount requested
	L"У вас нет такой суммы.", // telling the player they can't afford to train this town
	L"Продолжить тренировку ополчения в %ls (%ls %d)?", // continue training this town?
	L"Стоимость $", // the cost in dollars to train militia
	L"(Д/Н)",   // abbreviated yes/no
	L"Тренировка ополчения в %d секторах будет стоить $ %d. %ls", // cost to train sveral sectors at once
	L"У вас нет $%d на тренировку здесь ополчения.",
	L"%ls: нужно %d%% отношения к вам, чтобы продолжить тренировку ополчения.",
	L"%ls: невозможно продолжить подготовку ополчения.",
};


//Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom of the single merc panel
static const wchar_t *s_rus_gzMoneyWithdrawMessageText[gzMoneyWithdrawMessageText_SIZE] =
{
	L"За один раз можно снять со счета не более $20,000.",
	L"Вы уверены, что хотите положить %ls на свой счет?",
};

static const wchar_t s_rus_gzCopyrightText[] =
	L"Авторские права(C) 1999 Sir-Tech Canada Ltd. Все права защищены. Распространение на территории стран СНГ компания БУКА.";

static const wchar_t s_rusGold_gzCopyrightText[] =
	L"(C) 2002 Strategy First и компания Акелла. Все права защищены.";

//option Text
static const wchar_t *s_rus_zOptionsToggleText[zOptionsToggleText_SIZE] =
{
	L"Речь",
	L"Заглушить отзывы",
	L"Субтитры",
	L"Диалоги с паузами",
	L"Анимированный дым",
	L"Кровища",
	L"Не трожь мою мышь!",
	L"Старый способ выбора",
	L"Показывать путь",
	L"Показывать промахи",
	L"Подтверждение движения",
	L"Оповещение о сне/бодрствовании",
	L"Метрическая система мер",
	L"Подсветка земли",
	L"Переводить курсор на наемников",
	L"Переводить курсор на двери",
	L"Подсветка вещей",
	L"Показывать верхушки деревьев",
	L"Показывать каркасы",
	L"Трехмерный курсор",
};

//This is the help text associated with the above toggles.
static const wchar_t *s_rus_zOptionsScreenHelpText[zOptionsToggleText_SIZE] =
{
	//speech
	L"Включите эту опцию, если хотите слышать диалоги персонажей.",

	//Mute Confirmation
	L"Включите эту опцию, чтобы заглушить отзывы на команды.",
	//Subtitles
	L"Показывать тексты диалогов.",

	//Key to advance speech
	L"Включите эту опцию, чтобы субтитры переходили к следующей фразе по нажатию клавиши.",

	//Toggle smoke animation
	L"Отключите эту опцию, если анимированный дым тормозит игру.",

	//Blood n Gore
	L"Отключите эту опцию, если вам не нравится вид крови.",

	//Never move my mouse
	L"Отключите эту опцию, чтобы курсор автоматически перемещался к всплывающим окнам.",

	//Old selection method
	L"Включает выбор персонажей, как в Jagged Alliance 1 (иначе выбор происходит наоборот).",

	//Show movement path
	L"Включает отображение пути бойца в реальном времени (или отключите эту опцию и используйте SHIFT).",

	//show misses
	L"Включите эту опцию, чтобы видеть, куда попадают пули при \"промахе\".",

	//Sleep/Wake notification
	L"При включенной опции для движения в реальном времени необходим дополнительный клик мышью.",

	//Display the enemy indicator
	L"Включает отображение оповещений о том, что ваш боец засыпает или просыпается.",

	//Use the metric system
	L"Когда опция включена, игра использует метрическую систему мер, иначе - британскую.",

	//Merc Lighted movement
	L"Когда опция включена, боец \"освещает\" землю при движении. Отключите для улучшения производительности.",

	//Smart cursor
	L"Когда опция включена, бойцы выбираются, если курсор рядом с ними.",

	//snap cursor to the door
	L"Когда опция включена, курсор перескакивает на дверь, если он рядом с ней.",

	//glow items
	L"Включает постоянную подсветку предметов на земле (|I).",

	//toggle tree tops
	L"Включает отображение верхушек деревьев (|Е).",

	//toggle wireframe
	L"Включает отображение \"каркасов\" домов (|W).",

	L"Включает \"трехмерный\" курсор движения (|H|o|m|e).",

};


static const wchar_t *s_rus_gzGIOScreenText[gzGIOScreenText_SIZE] =
{
	L"ИГРОВЫЕ НАСТРОЙКИ",
	L"Стиль игры",
	L"Реалистичный",
	L"Фантастичный",
	L"Количество оружия",
	L"Сотни стволов",
	L"Как в жизни",
	L"Уровень сложности",
	L"НОВОБРАНЕЦ",
	L"СОЛДАТ",
	L"ВЕТЕРАН",
	L"Ok",
	L"Отмена",
	L"Дополнительная сложность",
	L"Сохранение где угодно",
	L"СТАЛЬНАЯ ВОЛЯ"
};

static const wchar_t *s_rus_pDeliveryLocationStrings[pDeliveryLocationStrings_SIZE] =
{
	L"Остин",			//Austin, Texas, USA
	L"Багдад",			//Baghdad, Iraq (Suddam Hussein's home)
	L"Драссен",			//The main place in JA2 that you can receive items.  The other towns are dummy names...
	L"Гонконг",		   //Hong Kong, Hong Kong
	L"Бейрут",			//Beirut, Lebanon	(Middle East)
	L"Лондон",			//London, England
	L"Лос Анджелес",	//Los Angeles, California, USA (SW corner of USA)
	L"Медуна",			//Meduna -- the other airport in JA2 that you can receive items.
	L"Метавира",		//The island of Metavira was the fictional location used by JA1
	L"Майами",				//Miami, Florida, USA (SE corner of USA)
	L"Москва",			//Moscow, USSR
	L"Нью-Йорк",		//New York, New York, USA
	L"Оттава",			//Ottawa, Ontario, Canada -- where JA2 was made!
	L"Париж",				//Paris, France
	L"Триполи",			//Tripoli, Libya (eastern Mediterranean)
	L"Токио",				//Tokyo, Japan
	L"Ванкувер",		//Vancouver, British Columbia, Canada (west coast near US border)
};

/* This string is used in the IMP character generation.  It is possible to
 * select 0 ability in a skill meaning you can't use it.  This text is
 * confirmation to the player. */
static const wchar_t s_rus_pSkillAtZeroWarning[]    = L"Уверены? Ноль означает отсутствие навыка.";
static const wchar_t s_rus_pIMPBeginScreenStrings[] = L"(не более 8 букв)";
static const wchar_t s_rus_pIMPFinishButtonText[]   = L"Анализ";
static const wchar_t s_rus_pIMPFinishStrings[]      = L"Спасибо, %ls"; //%ls is the name of the merc
static const wchar_t s_rus_pIMPVoicesStrings[]      = L"Голос"; // the strings for imp voices screen

// title for program
static const wchar_t s_rus_pPersTitleText[] = L"Кадры";

// paused game strings
static const wchar_t *s_rus_pPausedGameText[pPausedGameText_SIZE] =
{
	L"Пауза",
	L"Снять с паузы (|P|a|u|s|e)",
	L"Поставить на паузу (|P|a|u|s|e)",
};


static const wchar_t *s_rus_pMessageStrings[pMessageStrings_SIZE] =
{
	L"Выйти из игры?",
	L"OK",
	L"ДА",
	L"НЕТ",
	L"ОТМЕНА",
	L"НАНЯТЬ",
	L"ЛОЖЬ",
	L"Нет описания", //Save slots that don't have a description.
	L"Игра сохранена",
	L"День",
	L"Наемников",
	L"Пустая ячейка сохранения", 		//An empty save game slot
	L"в/м",					//Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
	L"мин",					//Abbreviation for minute.
	L"м",						//One character abbreviation for meter (metric distance measurement unit).
	L"пуль",				//Abbreviation for rounds (# of bullets)
	L"кг",					//Abbreviation for kilogram (metric weight measurement unit)
	L"ф",					//Abbreviation for pounds (Imperial weight measurement unit)
	L"Главная страница",	//Home as in homepage on the internet. Used on M.I.S. insurance web page
	L"USD",					//Abbreviation to US dollars
	L"н/п",					//Lowercase acronym for not applicable.
	L"Тем временем",		//Meanwhile
	L"%ls теперь в секторе %ls%ls", //Name/Squad has arrived in sector A9.  Order must not change without notifying
																		//SirTech
	L"Версия",
	L"Пустая ячейка быстрого сохр",
	L"Эта ячейка предназначена для быстрого сохранения игры (с помощью ALT+S).",
	L"Открытая", //doors definitions
	L"Закрытая", //doors definitions
	L"У вас кончается дисковое пространство. У вас осталось %lsМБ свободных, а для игры требуется %lsMБ.",
	L"%ls ловит %ls.",		//'Merc name' has caught 'item' -- let SirTech know if name comes after item.
	L"%ls принимает препарат.", //'Merc name' has taken the drug
	L"%ls не имеет медицинских навыков",//'Merc name' has no medical skill.

	//CDRom errors (such as ejecting CD while attempting to read the CD)
	L"Нарушена целостность программы.",
	L"ОШИБКА: Вынут CD-ROM",

	//When firing heavier weapons in close quarters, you may not have enough room to do so.
	L"Мало места для стрельбы.",

	//Can't change stance due to objects in the way...
	L"Сейчас изменить положение нельзя.",

	//Simple text indications that appear in the game, when the merc can do one of these things.
	L"Уронить",
	L"Бросить",
	L"Передать",

	L"%ls передан %ls.", //"Item" passed to "merc".  Please try to keep the item %ls before the merc %ls, otherwise,
											 //must notify SirTech.
	L"Нельзя передать %ls %ls.", //pass "item" to "merc".  Same instructions as above.

	//A list of attachments appear after the items.  Ex:  Kevlar vest ( Ceramic Plate 'Attached )'
	L" присоединен)",

	//Cheat modes
	L"Достигнут чит-уровень один",
	L"Достигнут чит-уровень два",

	//Toggling various stealth modes
	L"Отряд в режиме скрытности.",
	L"Отряд в обычном режиме.",
	L"%ls в режиме скрытности.",
	L"%ls в обычном режиме.",

	//Wireframes are shown through buildings to reveal doors and windows that can't otherwise be seen in
	//an isometric engine.  You can toggle this mode freely in the game.
	L"Отображение каркасов зданий включено",
	L"Отображение каркасов зданий выключено",

	//These are used in the cheat modes for changing levels in the game.  Going from a basement level to
	//an upper level, etc.
	L"Нельзя подняться с этого уровня...",
	L"Ниже уровней нет...",
	L"Входим в подвальный уровень %d...",
	L"Уходим из подвала...",

	L".",		// used in the shop keeper inteface to mark the ownership of the item eg Red's gun
	L"Режим слежения выключен.",
	L"Режим слежения включен.",
	L"Курсор трехмерный.",
	L"Курсор обычный.",
	L"Отряд %d действует.",
	L"У вас нет денег, чтобы ежедневно выплачивать %ls %ls",	//first %ls is the mercs name, the seconds is a string containing the salary
	L"Пропуск",
	L"%ls не может идти в одиночку.",
	L"Игра была сохранена под именем SaveGame99.sav. Для загрузки переименуйте ее в SaveGame01-SaveGame10.",
	L"%ls пъет %ls",
	L"Груз снаряжения прибыл в Драссен.",
	L"%ls прибудет в указанное место высадки (сектор %ls) в день %d, примерно в %ls.",		//first %ls is mercs name, next is the sector location and name where they will be arriving in, lastely is the day an the time of arrival
	L"Журнал событий обновлен.",
#ifdef JA2BETAVERSION
	L"Игра сохранена в ячейку авто-сохранения.",
#endif
};


static const wchar_t *s_rus_ItemPickupHelpPopup[ItemPickupHelpPopup_SIZE] =
{
	L"Взять выделенные",
	L"Листать вверх",
	L"Выделить все",
	L"Листать вниз",
	L"Отмена"
};

static const wchar_t *s_rus_pDoctorWarningString[pDoctorWarningString_SIZE] =
{
	L"%ls слишком далеко для лечения.",
	L"Медики не смогли перевязать всех.",
};

static const wchar_t *s_rus_pMilitiaButtonsHelpText[pMilitiaButtonsHelpText_SIZE] =
{
	L"Взять новобранцев (Правый клик)/Распределить (Левый клик)", // button help text informing player they can pick up or drop militia with this button
	L"Взять солдат (Правый клик)/Распределить (Левый клик)",
	L"Взять ветеранов (Правый клик)/Распределить (Левый клик)",
	L"Поровну распределить доступное ополчение по всем секторам",
};

// to inform the player to hire some mercs to get things going
static const wchar_t s_rus_pMapScreenJustStartedHelpText[] = L"Сходите на сайт АIM и наймите наемников (*Подсказка* через ноутбук)";

static const wchar_t s_rus_pAntiHackerString[] = L"Ошибка. Испорченные или отсутствующие файлы. Игра завершает работу.";


static const wchar_t *s_rus_gzLaptopHelpText[gzLaptopHelpText_SIZE] =
{
	//Buttons:
	L"Просмотреть электроные письма",
	L"Посетить Интернет сайты",
	L"Просмотреть файлы",
	L"Просмотреть журнал событий",
	L"Информация о команде",
	L"Просмотреть финансовые отчеты",
	L"Выключить лэптоп",

	//Bottom task bar icons (if they exist):
	L"Получена новая почта",
	L"Получены новые файлы",

	//Bookmarks:
	L"Международная Ассоциация Наемников",
	L"Бобби Рэй - заказ оружия в сети",
	L"Институт Психологии Наемников",
	L"M.E.R.C - Экономичный Рекрутинговый Центр",
	L"Похоронное агенство Макгилликати",
	L"Цветы по Всему Миру",
	L"Страховые агенты по контрактам A.I.M.",
};


static const wchar_t s_rus_gzHelpScreenText[] = L"Закрыть помощь";

static const wchar_t *s_rus_gzNonPersistantPBIText[gzNonPersistantPBIText_SIZE] =
{
	L"Идет бой. Отступить можно только с тактической карты",
	L"Войти в сектор, чтобы продолжить бой (|E)",
	L"Продолжить бой в автоматическом режиме (|A)",
	L"Нельзя провести авто-бой, когда вы нападаете",
	L"Нельзя провести авто-бой, когда вы попали в засаду",
	L"Нельзя провести авто-бой, когда вы сражаетесь с существами в шахтах",
	L"Нельзя провести авто-бой, если рядом враждебные жители",
	L"Нельзя провести авто-бой, если рядом кошки-убийцы",
	L"ИДЕТ БОЙ",
	L"Сейчас отступить нельзя",
};

static const wchar_t *s_rus_gzMiscString[gzMiscString_SIZE] =
{
	L"Ваше ополчение сражается без помощи наемников...",
	L"Транспорт полностью заправлен.",
	L"Бензобак заправлен на %d%%.",
	L"Армия Дейдраны полностью захватила %ls.",
	L"Вы потеряли заправку.",
};

static const wchar_t s_rus_gzIntroScreen[] = L"Невозможно найти вступительный ролик";

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
static const wchar_t *s_rus_pNewNoiseStr[pNewNoiseStr_SIZE] =
{
	L"%ls слышит %ls звук, идущий %ls.",
	L"%ls слышит %ls звук ДВИЖЕНИЯ, идущий %ls.",
	L"%ls слышит %ls СКРИП, идущий %ls.",
	L"%ls слышит %ls ПЛЕСК, идущий %ls.",
	L"%ls слышит %ls УДАР, идущий %ls.", //$$
	L"%ls слышит %ls ВЗРЫВ %ls.",
	L"%ls слышит %ls КРИК %ls.",
	L"%ls слышит %ls УДАР %ls.",
	L"%ls слышит %ls УДАР %ls.",
	L"%ls слышит %ls ЗВОН, идущий %ls.",
	L"%ls слышит %ls ГРОХОТ, идущий %ls.",
};

static const wchar_t *s_rus_wMapScreenSortButtonHelpText[wMapScreenSortButtonHelpText_SIZE] =
{
	L"Сортировка по имени (|F|1)",
	L"Сортировка по заданию (|F|2)",
	L"Сортировка по сну/бодрствованию (|F|3)",
	L"Сортировка по месту (|F|4)",
	L"Сортировка по цели (|F|5)",
	L"Сортировка по времени контракта (|F|6)",
};



static const wchar_t *s_rus_BrokenLinkText[BrokenLinkText_SIZE] =
{
	L"Ошибка 404",
	L"URL не найден.",
};


static const wchar_t *s_rus_gzBobbyRShipmentText[gzBobbyRShipmentText_SIZE] =
{
	L"Последние заказы",
	L"Заказ #",
	L"Предметов",
	L"Дата",
};


static const wchar_t *s_rus_gzCreditNames[gzCreditNames_SIZE]=
{
	L"Chris Camfield",
	L"Shaun Lyng",
	L"Kris Mornes",
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


static const wchar_t *s_rus_gzCreditNameTitle[gzCreditNameTitle_SIZE]=
{
	L"Ведущий программист игры", 			// Chris Camfield !!!
	L"Дизайн/Сценарий",							// Shaun Lyng
	L"Программист стратегической части и редактора",					//Kris Marnes
	L"Продюсер/Дизайн",						// Ian Currie
	L"Дизайн/Дизайн карт",				// Linda Currie
	L"Художник",													// Eric \"WTF\" Cheng
	L"Бета-координатор, техподдержка",				// Lynn Holowka
	L"Художник от Бога",						// Norman \"NRG\" Olsen
	L"Гуру звука",											// George Brooks
	L"Дизайн экранов/Художник",					// Andrew Stacey
	L"Главный художник/Аниматор",						// Scot Loving
	L"Главный программист",									// Andrew \"Big Cheese Doddle\" Emmons
	L"Программист",											// Dave French
	L"Программист стратегической части и баланса игры",					// Alex Meduna
	L"Художник по портретам",								// Joey \"Joeker\" Whelan",
};

static const wchar_t *s_rus_gzCreditNameFunny[gzCreditNameFunny_SIZE]=
{
	L"", 																			// Chris Camfield
	L"(все еще зубрит правила пунктуации)",					// Shaun Lyng
	L"(\"Оно работает. Я просто чиню\")",	//Kris \"The Cow Rape Man\" Marnes
	L"(слишком стар для всего этого)",				// Ian Currie
	L"(работает и над Wizardry 8)",						// Linda Currie
	L"(принужденный заниматься тестированием)",			// Eric \"WTF\" Cheng
	L"(Ушла от нас в CFSA - вот тебе на...)",	// Lynn Holowka
	L"",																			// Norman \"NRG\" Olsen
	L"",																			// George Brooks
	L"(Мертвая Голова и любитель джаза)",						// Andrew Stacey
	L"(его настоящее имя Роберт)",							// Scot Loving
	L"(единственное ответственное лицо)",					// Andrew \"Big Cheese Doddle\" Emmons
	L"(может опять заняться мотогонками)",	// Dave French
	L"(украден с работы над Wizardry 8)",							// Alex Meduna
	L"(делал предметы и загрузочные экраны!)",	// Joey \"Joeker\" Whelan",
};

static const wchar_t *s_rus_sRepairsDoneString[sRepairsDoneString_SIZE] =
{
	L"%ls отремонтировал(а) свои вещи",
	L"%ls отремонтировал(а) оружие и броню команды",
	L"%ls отремонтировал(а) снаряжение команды",
	L"%ls отремонтировал(а) транспортируемые командой вещи",
};

static const wchar_t *s_rus_zGioDifConfirmText[zGioDifConfirmText_SIZE]=
{
	L"Вы выбрали сложность НОВОБРАНЕЦ. Это подходит для новичков в Jagged Alliance, для новичков в жанре стратегий вообще, или для тех, кто желает сократить битвы в игре. Ваш выбор скажется на игре в целом, так что выбирайте с умом. Уверены, что хотите играть новобранцем?",
	L"Вы выбрали сложность СОЛДАТ. Это подходит для игроков, уже знакомых с играми Jagged Alliance или с подобными им. Ваш выбор скажется на игре в целом, так что выбирайте с умом. Уверены, что хотите играть солдатом?",
	L"Вы выбрали сложность ВЕТЕРАН. Мы Вас предупредили. Нечего на нас пенять, если вас доставят назад в цинковом гробу. Ваш выбор скажется на игре в целом, так что выбирайте с умом. Уверены, что хотите играть ветераном?",
};

static const wchar_t *s_rus_gzLateLocalizedString[gzLateLocalizedString_SIZE] =
{
	//1-5
	L"Робот не может покинуть сектор, если им никто не управляет.",

	//This message comes up if you have pending bombs waiting to explode in tactical.
	L"Сейчас ускорять время нельзя. Подождите фейерверка!",

	//'Name' refuses to move.
	L"%ls отказывается двигаться.",

	//%ls a merc name
	L"%ls без сил и не может поменять положение.",

	//A message that pops up when a vehicle runs out of gas.
	L"У %lsа кончилось топливо и он остается в %c%d.",///TRNSL Assumes that all vehicle names are male gender words

	//6-10

	// the following two strings are combined with the pNewNoise[] strings above to report noises
	// heard above or below the merc
	L"сверху",
	L"снизу",

	//The following strings are used in autoresolve for autobandaging related feedback.
	L"Никто из ваших бойцов не умеет оказывать первую помощь.",
	L"Для оказания первой помощи нет аптечек.",
	L"Не хватает медикаментов и бинтов, чтобы перевязать всех.",
	L"Ваши бойцы не нуждаются в первой помощи.",
	L"Перевязать наемников автоматически",
	L"Всем бойцам оказана первая помощь.",

	//14
	L"Арулько",

	L"(на крыше)",

	L"Здоровье: %d/%d",

	//In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
	//"vs." is the abbreviation of versus.
	L"%d против %d",

	L"В %lsе нет места!",  //(ex "The ice cream truck is full")///TRNSL_ASSUME vehicle gender is male

	L"%ls нуждается не в перевязке и первой помощи, а в основательном медицинском обследовании и/или отдыхе.",

	//20
	//Happens when you get shot in the legs, and you fall down.
	L"%ls получает ранение в ногу и падает!",
	//Name can't speak right now.
	L"%ls не может сейчас говорить.",

	//22-24 plural versions @@@2 elite to veteran
	L"Несколько (%d) ополченцев-новобранцев стали ветеранами.",
	L"Несколько (%d) ополченцев-новобранецев стали солдатами.",
	L"Несколько (%d) ополченцев-солдат стали ветеранами.",

	//25
	L"Перекл.", ///TRNSL_CHECK_INGAME LENGTH

	//26
	//Name has gone psycho -- when the game forces the player into burstmode (certain unstable characters)
	L"%ls впадает в безумие!",

	//27-28
	//Messages why a player can't time compress.
	L"Нельзя ускорять время, поскольку у вас есть бойцы в секторе %ls.", //
	L"Нельзя ускорять время, когда бойцы находятся в шахтах с монстрами.", //

	//29-31 singular versions @@@2 elite to veteran
	L"Один ополченец-новобранец стал ветераном.",
	L"Один ополченец-новобранец стал солдатом.",
	L"Один ополченец-солдат стал ветераном.",

	//32-34
	L"%ls ничего не говорит.",
	L"Выбраться на поверхность?",
	L"(Отряд %d)",

	//35
	//Ex: "Red has repaired Scope's MP5K".  Careful to maintain the proper order (Red before Scope, Scope before MP5K)
	L"%ls починил для %ls %ls",

	//36
	L"КОШКА-УБИЙЦА",

	//37-38 "Name trips and falls"
	L"%ls спотыкается и падает",
	L"Эту вещь нельзя взять отсюда.",

	//39
	L"Ваши бойцы не в состоянии сражаться. Ополчение ведет бой с существами само.",

	//40-43
	//%ls is the name of merc.
	L"%ls истратил(а) все медикаменты!",
	L"%ls не обладает навыками, чтобы лечить кого-либо!",
	L"%ls без инструментов!",
	L"%ls не обладает навыками, чтобы ремонтировать что-либо!",

	//44-45
	L"Время ремонта",
	L"%ls не видит этого человека.",

	//46-48
	L"%ls ломает удлинитель ствола!",
	L"Не разрешается больше %d тренеров ополчения на сектор.",
	L"Вы уверены?",

	//49-50
	L"Ускорение времени",
	L"Бак теперь заправлен.",

	//51-52 Fast help text in mapscreen.
	L"Продолжить ускорение времени (|S|p|a|c|e)",
	L"Прекратить ускорение времени (|E|s|c)",

	//53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
	L"%ls расклинил %ls",
	L"%ls расклинил для %ls %ls",

	//55
	L"Невозможно сжимать время при просмотре предметов сектора.",

	//56
	//Displayed with the version information when cheats are enabled.
	L"Текущий/Максимальный: %d%%/%d%%",

	//57
	L"Сопроводить Джона и Мэри?",

	L"Выключатель нажат."
};

static const wchar_t s_rus_str_ceramic_plates_smashed[] = L"%ls: в бронежилете разбились пластины!";

static const wchar_t s_rus_str_arrival_rerouted[] = L"Место высадки новоприбывших наемников перенесено в сектор %ls; ранее заданный сектор %ls захвачен врагом.";


static const wchar_t s_rus_str_stat_health[]       = L"Здоровье";
static const wchar_t s_rus_str_stat_agility[]      = L"Проворность";
static const wchar_t s_rus_str_stat_dexterity[]    = L"Ловкость";
static const wchar_t s_rus_str_stat_strength[]     = L"Сила";
static const wchar_t s_rus_str_stat_leadership[]   = L"Лидерство";
static const wchar_t s_rus_str_stat_wisdom[]       = L"Мудрость";
static const wchar_t s_rus_str_stat_exp_level[]    = L"Опытность"; // Уровень опыта
static const wchar_t s_rus_str_stat_marksmanship[] = L"Меткость";
static const wchar_t s_rus_str_stat_mechanical[]   = L"Механика";
static const wchar_t s_rus_str_stat_explosive[]    = L"Взрывник";
static const wchar_t s_rus_str_stat_medical[]      = L"Медицина";

static const wchar_t *s_rus_str_stat_list[str_stat_list_SIZE] =
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

static const wchar_t *s_rus_str_aim_sort_list[str_aim_sort_list_SIZE] =
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

static const wchar_t * s_rus_zNewTacticalMessages[] =
{
	L"Расстояние до цели: %d клеток",
	L"Дальнобойность оружия: %d клеток, расстояние до цели: %d клеток",
	L"Отобразить укрытия",
	L"Зона обзора",
	L"Это игра для человека со СТАЛЬНОЙ ВОЛЕЙ. Нельзя сохраняться, когда рядом враги!",
	L"(Нельзя сохранять игру в бою)",
};

static const wchar_t s_rus_str_iron_man_mode_warning[] = L"Вы выбрали режим СТАЛЬНОЙ ВОЛИ. Эта настройка сильно усложняет игру, так как вы не сможете сохраняться, когда в секторе есть враги. ЖЕЛЕЗНАЯ ВОЛЯ действует в течение всей игры. Уверены, что хотите включить этот режим?";

/** Russian language resources. */
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
  s_rus_pTownNames,
  s_rus_g_towns_locative,
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
  s_rus_pLandTypeStrings,
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
  s_rus_pDeliveryLocationStrings,
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
};


/** Russian Gold language resources. */
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
  s_rus_pTownNames,
  s_rus_g_towns_locative,
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
  s_rus_pLandTypeStrings,
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
  s_rus_pDeliveryLocationStrings,
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
};

#ifdef WITH_UNITTESTS
#define ARR_SIZE(x) (sizeof(x)/sizeof(x[0]))
TEST(WideStringEncodingTest, RusianTextFile)
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
