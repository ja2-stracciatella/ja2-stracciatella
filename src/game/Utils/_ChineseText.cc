#include "Text.h"

#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"
#endif


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
// - Never translate a string to appear on multiple lines.  All strings L"This is a really long string...",
//   must fit on a single line no matter how long the string is.  All strings start with L" and end with ",
// - Never remove any extra spaces in strings.  In addition, all strings containing multiple sentences only
//   have one space after a period, which is different than standard typing convention.  Never modify sections
//   of strings contain combinations of % characters.  These are special format characters and are always
//   used in conjunction with other characters.  For example, %ls means string, and is commonly used for names,
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
//   EX:  L"|Map Screen"
//
//   This means the 'M' is the hotkey.  In the game, when somebody hits the 'M' key, it activates that
//   button.  When translating the text to another language, it is best to attempt to choose a word that
//   uses 'M'.  If you can't always find a match, then the best thing to do is append the 'M' at the end
//   of the string in this format:
//
//   EX:  L"Ecran De Carte (|M)"  (this is the French translation)
//
//   Other examples are used multiple times, like the Esc key  or "|E|s|c" or Space -> (|S|p|a|c|e)
//
// 2) NEWLINE
//   Any place you see a \n within the string, you are looking at another string that is part of the fast help
//   text system.  \n notation doesn't need to be precisely placed within that string, but whereever you wish
//   to start a new line.
//
//   EX:  L"Clears all the mercs' positions,\nand allows you to re-enter them manually."
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
//   EX:  L"Clears all the mercs' positions, \n and allows you to re-enter them manually."
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



static StrPointer s_chn_WeaponType[WeaponType_SIZE] =
{
	L"其他",
	L"手枪",
	L"自动手枪",
	L"冲锋枪",
	L"步枪",
	L"狙击步枪",
	L"突击步枪",
	L"轻机枪",
	L"霰弹枪",
};

static StrPointer s_chn_TeamTurnString[TeamTurnString_SIZE] =
{
	L"玩家的回合",
	L"敌军的回合",
	L"异形的回合",
	L"民兵的回合",
	L"平民的回合",
	// planning turn
};

static StrPointer s_chn_Message[Message_SIZE] =
{
	// In the following 8 strings, the %ls is the merc's name, and the %d (if any) is a number.

	L"%ls被射中了头部, 并且失去了1点智慧!",
	L"%ls被射中了肩部, 并且失去了1点灵巧!",
	L"%ls被射中了胸膛, 并且失去了1点力量!",
	L"%ls被射中了腿部, 并且失去了1点敏捷!",
	L"%ls被射中了头部, 并且失去了%d点智慧!",
	L"%ls被射中了肩部, 并且失去了%d点灵巧!",
	L"%ls被射中了胸膛, 并且失去了%d点力量!",
	L"%ls被射中了腿部, 并且失去了%d点敏捷!",
	L"中断!",

	L"您的援军到达了!",

	// In the following four lines, all %ls's are merc names

	L"%ls装填弹药.",
	L"%ls没有足够的行动点数.",
	// the following 17 strings are used to create lists of gun advantages and disadvantages
	// (separated by commas)
	L"耐用",
	L"不耐用",
	L"容易修复",
	L"不易修复",
	L"杀伤力高",
	L"杀伤力低",
	L"射击快",
	L"射击慢",
	L"射程远",
	L"射程近",
	L"轻盈",
	L"笨重",
	L"小巧",
	L"射速高",
	L"无点射限制",
	L"大容弹量",
	L"小容弹量",

	// In the following two lines, all %ls's are merc names

	L"%ls的伪装失效了.",
	L"%ls的伪装被洗掉了.",

	// The first %ls is a merc name and the second %ls is an item name

	L"副武器没弹药了!",//	L"Second weapon is out of ammo!",
	L"%ls偷到了%ls.",		//	L"%ls has stolen the %ls.",

	// The %ls is a merc name

	L"%ls的武器不能扫射.",	//	L"%ls's weapon can't burst fire.",

	L"您已经装上了该附件.",//	L"You've already got one of those attached.",
	L"组合物品?",			//	L"Merge items?",

	// Both %ls's are item names

	L"您不能把%ls和%ls组合在一起.",

	L"无",
	L"退出子弹",
	L"附件",
	
	//You cannot use "item(s)" and your "other item" at the same time.
	//Ex:  You cannot use sun goggles and you gas mask at the same time.
	L"您不能同时使用%ls和%ls.",

	L"请把光标选中的物品放到另一物品的任意附件格中, 这样就可能合成新物品.",
	L"请把光标选中的物品放到另一物品的任意附件格中, 这样就可能合成新物品.(但是这一次, 该物品不相容.)",
	L"该区域的敌军尚未被肃清",
	L"您还得给%ls%ls",
	L"%ls被射中了头部.",
	L"放弃战斗?",
	L"这个组合是永久性的.您确认要这样做吗?",
	L"%ls感觉精力充沛!",
	L"%ls踩到了大理石珠子, 滑倒了!",
	L"%ls没能从敌人手里抢到%ls!",
	L"%ls修复了%ls.",
	L"中断: ",
	L"投降?",
	L"此人拒绝您的包扎.",
	L"我不这么认为.",
	L"要搭乘Skyrider的直升机, 您得先把佣兵分配到交通工具/直升机.",
	L"%ls的时间只够给一支枪装填弹药",
	L"血猫的回合",
};


// the names of the towns in the game

static const wchar_t *s_chn_pTownNames[pTownNames_SIZE] =
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

static const wchar_t *s_chn_g_towns_locative[g_towns_locative_SIZE] =
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

static const wchar_t *s_chn_sTimeStrings[sTimeStrings_SIZE] =
{
	L"暂停",
	L"普通",
	L"5 mins",
	L"30 mins",
	L"60 mins",
	L"6 hrs",
};


// Assignment Strings: what assignment does the merc  have right now? For example, are they on a squad, training,
// administering medical aid (doctor) or training a town. All are abbreviated. 8 letters is the longest it can be.

static const wchar_t *s_chn_pAssignmentStrings[pAssignmentStrings_SIZE] =
{
	L"第1小队",
	L"第2小队",
	L"第3小队",
	L"第4小队",
	L"第5小队",
	L"第6小队",
	L"第7小队",
	L"第8小队",
	L"第9小队",
	L"第10小队",
	L"第11小队",
	L"第12小队",
	L"第13小队",
	L"第14小队",
	L"第15小队",
	L"第16小队",
	L"第17小队",
	L"第18小队",
	L"第19小队",
	L"第20小队",
	L"编队",// on active duty
	L"医生",// administering medical aid
	L"病人", // getting medical aid
	L"交通工具", // in a vehicle
	L"在途中",// in transit - abbreviated form
	L"修理", // repairing
	L"锻炼", // training themselves
	L"民兵", // training a town to revolt
	L"教练", // training a teammate
	L"学员", // being trained by someone else
	L"死亡", // dead
	L"无力中", // abbreviation for incapacitated
	L"战俘", // Prisoner of war - captured
	L"医院", // patient in a hospital
	L"空",	// Vehicle is empty
};


static const wchar_t *s_chn_pMilitiaString[pMilitiaString_SIZE] =
{
	L"民兵", // the title of the militia box
	L"未分配的民兵", //the number of unassigned militia troops
	L"本地区有敌军存在, 您无法重新分配民兵!",
};


static const wchar_t *s_chn_pMilitiaButtonString[pMilitiaButtonString_SIZE] =
{
	L"自动", // auto place the militia troops for the player
	L"完成", // done placing militia troops
};

static const wchar_t *s_chn_pConditionStrings[pConditionStrings_SIZE] =
{
	L"极好", //the state of a soldier .. excellent health
	L"良好", // good health
	L"普通", // fair health
	L"受伤", // wounded health
	L"疲劳", // tired
	L"失血", // bleeding to death
	L"昏迷", // knocked out
	L"垂死", // near death
	L"死亡", // dead
};

static const wchar_t *s_chn_pEpcMenuStrings[pEpcMenuStrings_SIZE] =
{
	L"编队", // set merc on active duty
	L"病人",// set as a patient to receive medical aid
	L"交通工具", // tell merc to enter vehicle
	L"无护送", // let the escorted character go off on their own
	L"取消", // close this menu
};


// look at pAssignmentString above for comments

static const wchar_t *s_chn_pLongAssignmentStrings[pLongAssignmentStrings_SIZE] =
{
	L"第1小队",
	L"第2小队",
	L"第3小队",
	L"第4小队",
	L"第5小队",
	L"第6小队",
	L"第7小队",
	L"第8小队",
	L"第9小队",
	L"第10小队",
	L"第11小队",
	L"第12小队",
	L"第13小队",
	L"第14小队",
	L"第15小队",
	L"第16小队",
	L"第17小队",
	L"第18小队",
	L"第19小队",
	L"第20小队",
	L"编队",// on active duty
	L"医生",// administering medical aid
	L"病人", // getting medical aid
	L"交通工具", // in a vehicle
	L"在途中",// in transit - abbreviated form
	L"修理", // repairing
	L"锻炼", // training themselves
	L"民兵", // training a town to revolt
	L"教练", // training a teammate
	L"学员", // being trained by someone else
	L"死亡", // dead
	L"无力中", // abbreviation for incapacitated
	L"战俘", // Prisoner of war - captured
	L"医院", // patient in a hospital
	L"空",	// Vehicle is empty
};


// the contract options

static const wchar_t *s_chn_pContractStrings[pContractStrings_SIZE] =
{
	L"合同选项: ",
	L"", // a blank line, required
	L"雇佣一日",// offer merc a one day contract extension
	L"雇佣一周", // 1 week
	L"雇佣两周", // 2 week
	L"解雇",// end merc's contract
	L"取消", // stop showing this menu
};

static const wchar_t *s_chn_pPOWStrings[pPOWStrings_SIZE] =
{
	L"囚禁",  //an acronym for Prisoner of War
	L"??",
};

static const wchar_t *s_chn_pInvPanelTitleStrings[pInvPanelTitleStrings_SIZE] =
{
	L"护甲", // the armor rating of the merc
	L"负重", // the weight the merc is carrying
	L"伪装", // the merc's camouflage rating
};

static const wchar_t *s_chn_pShortAttributeStrings[pShortAttributeStrings_SIZE] =
{
	L"敏捷", // the abbreviated version of : agility
	L"灵巧", // dexterity
	L"力量", // strength
	L"领导", // leadership
	L"智慧", // wisdom
	L"级别", // experience level
	L"枪法", // marksmanship skill
	L"爆破", // explosive skill
	L"机械", // mechanical skill
	L"医疗", // medical skill
};


static const wchar_t *s_chn_pUpperLeftMapScreenStrings[pUpperLeftMapScreenStrings_SIZE] =
{
	L"任务", // the mercs current assignment
	L"生命", // the health level of the current merc
	L"士气", // the morale of the current merc
	L"状态",	// the condition of the current vehicle
};

static const wchar_t *s_chn_pTrainingStrings[pTrainingStrings_SIZE] =
{
	L"锻炼", // tell merc to train self
	L"民兵",// tell merc to train town
	L"教练", // tell merc to act as trainer
	L"学员", // tell merc to be train by other
};

static const wchar_t *s_chn_pAssignMenuStrings[pAssignMenuStrings_SIZE] =
{
	L"编队",
	L"医生",
	L"病人",
	L"交通工具",
	L"修理",
	L"训练",
	L"取消", // cancel this menu
};

static const wchar_t *s_chn_pRemoveMercStrings[pRemoveMercStrings_SIZE] =
{
	L"移除佣兵", // remove dead merc from current team
	L"取消",
};

static const wchar_t *s_chn_pAttributeMenuStrings[pAttributeMenuStrings_SIZE] =
{
	L"力量",	//"Strength",
	L"灵巧",	//"Dexterity",
	L"敏捷",	//"Agility",
	L"生命",	//"Health",
	L"枪法",	//"Marksmanship",
	L"医疗",	//"Medical",
	L"机械",	//"Mechanical",
	L"领导",	//"Leadership",
	L"爆破",	//"Explosives",
	L"取消",	//"Cancel",
};

static const wchar_t *s_chn_pTrainingMenuStrings[pTrainingMenuStrings_SIZE] =
{
	L"锻炼", // tell merc to train self
	L"民兵",// tell merc to train town
	L"教练", // tell merc to act as trainer
	L"学员", // tell merc to be train by other
	L"取消", // cancel this menu
};


static const wchar_t *s_chn_pSquadMenuStrings[pSquadMenuStrings_SIZE] =
{
	L"第1小队",
	L"第2小队",
	L"第3小队",
	L"第4小队",
	L"第5小队",
	L"第6小队",
	L"第7小队",
	L"第8小队",
	L"第9小队",
	L"第10小队",
	L"第11小队",
	L"第12小队",
	L"第13小队",
	L"第14小队",
	L"第15小队",
	L"第16小队",
	L"第17小队",
	L"第18小队",
	L"第19小队",
	L"第20小队",
	L"取消",
};


static const wchar_t *s_chn_pPersonnelScreenStrings[pPersonnelScreenStrings_SIZE] =
{
	L"医疗保证金: ", // amount of medical deposit put down on the merc
	L"合同剩余时间: ", // cost of current contract
	L"击毙数: ", // number of kills by merc
	L"击伤数: ",// number of assists on kills by merc
	L"日薪: ", // daily cost of merc
	L"花费总数: ",// total cost of merc
	L"合同花费: ",
	L"总日数: ",// total service rendered by merc
	L"欠付佣金: ",// amount left on MERC merc to be paid
	L"命中率: ",// percentage of shots that hit target
	L"战斗次数: ", // number of battles fought
	L"受伤次数: ", // number of times merc has been wounded
	L"技能: ",
	L"无技能",
};


//These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
static const wchar_t *s_chn_gzMercSkillText[gzMercSkillText_SIZE] =
{
	L"无技能",
	L"开锁",
	L"徒手格斗",		//JA25: modified
	L"电子",
	L"夜战",			//JA25: modified
	L"投掷",
	L"教学",
	L"重武器",
	L"自动武器",
	L"潜行",
	L"双手武器",
	L"偷窃",
	L"武术",
	L"刀技",
	L"狙击手",
	L"伪装",					//JA25: modified
	L"(专家)",
};


// This is pop up help text for the options that are available to the merc

static const wchar_t *s_chn_pTacticalPopupButtonStrings[pTacticalPopupButtonStrings_SIZE] =
{
	L"站立/行走 (|S)",
	L"蹲伏/蹲伏前进(|C)",
	L"站立/奔跑 (|R)",
	L"匍匐/匍匐前进(|P)",
	L"查看(|L)",
	L"行动",
	L"交谈",
	L"检查 (|C|t|r|l)",

	// Pop up door menu
	L"用手开门",
	L"检查陷阱",
	L"开锁",
	L"踢门",
	L"解除陷阱",
	L"关门",
	L"开门",
	L"使用破门炸药",
	L"使用撬棍",
	L"取消 (|E|s|c)",
	L"关闭",
};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.

static const wchar_t *s_chn_pDoorTrapStrings[pDoorTrapStrings_SIZE] =
{
	L"无陷阱",
	L"一个爆炸陷阱",
	L"一个带电陷阱",
	L"一个警报陷阱",
	L"一个无声警报陷阱",
};

// On the map screen, there are four columns. This text is popup help text that identifies the individual columns.

static const wchar_t *s_chn_pMapScreenMouseRegionHelpText[pMapScreenMouseRegionHelpText_SIZE] =
{
	L"选择角色",
	L"分配任务",
	L"安排行军路线",
	L"签约 (|C)",
	L"移除佣兵",
	L"睡觉",
};

// volumes of noises

static const wchar_t *s_chn_pNoiseVolStr[pNoiseVolStr_SIZE] =
{
	L"微弱的",
	L"清晰的",
	L"大声的",
	L"非常大声的",
};

// types of noises

static const wchar_t *s_chn_pNoiseTypeStr[pNoiseTypeStr_SIZE] = // OBSOLETE
{
	L"未知",
	L"移动声",
	L"辗扎声",
	L"溅泼声",
	L"撞击声",
	L"枪声",
	L"爆炸声",
	L"尖叫声",
	L"撞击声",
	L"撞击声",
	L"粉碎声",
	L"破碎声",
};

// Directions that are used to report noises

static const wchar_t *s_chn_pDirectionStr[pDirectionStr_SIZE] =
{
	L"东北方",
	L"东方",
	L"东南方",
	L"南方",
	L"西南方",
	L"西方",
	L"西北方",
	L"北方"
};

// These are the different terrain types.

static const wchar_t *s_chn_pLandTypeStrings[pLandTypeStrings_SIZE] =
{
	L"城市",
	L"公路",
	L"平原",
	L"沙漠",
	L"灌木",
	L"森林",
	L"沼泽",
	L"湖泊",
	L"山地",
	L"不可通行",
	L"河流",	//river from north to south
	L"河流",	//river from east to west
	L"外国",
	//NONE of the following are used for directional travel, just for the sector description.
	L"热带",
	L"农田",
	L"平原, 公路",
	L"灌木, 公路",
	L"农庄, 公路",
	L"热带, 公路",
	L"森林, 公路",
	L"海滨",
	L"山地, 公路",
	L"海滨, 公路",
	L"沙漠, 公路",
	L"沼泽, 公路",
	L"灌木, SAM导弹基地",
	L"沙漠, SAM导弹基地",
	L"热带, SAM导弹基地",
	L"Meduna, SAM导弹基地",

	//These are descriptions for special sectors
	L"Cambria医院",
	L"Drassen机场",
	L"Meduna机场",
	L"SAM导弹基地",
	L"抵抗军隐蔽处",//The rebel base underground in sector A10
	L"Tixa地牢",//The basement of the Tixa Prison (J9)
	L"异形巢穴",//Any mine sector with creatures in it
	L"Orta地下室",	//The basement of Orta (K4)
	L"地道",		//The tunnel access from the maze garden in Meduna
										//leading to the secret shelter underneath the palace
	L"地下掩体",				//The shelter underneath the queen's palace
	L"",							//Unused
};

static const wchar_t *s_chn_gpStrategicString[gpStrategicString_SIZE] =
{
	L"%ls在%c%d区域被发现了, 另一小队即将到达.",	//STR_DETECTED_SINGULAR
	L"%ls在%c%d区域被发现了, 其它几个小队即将到达.",	//STR_DETECTED_PLURAL
	L"您想调整为同时到达吗?",												//STR_COORDINATE

	//Dialog strings for enemies.

	L"敌军给您一个投降的机会.",
	L"敌军俘虏了昏迷中的佣兵.",

	//The text that goes on the autoresolve buttons							

	L"撤退",		//The retreat button
	L"完成",		//The done button				//STR_AR_DONE_BUTTON

	//The headers are for the autoresolve type (MUST BE UPPERCASE)

	L"防守",							//STR_AR_DEFEND_HEADER
	L"攻击",						//STR_AR_ATTACK_HEADER
	L"遭遇战",						//STR_AR_ENCOUNTER_HEADER
	L"区域",	//The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

	//The battle ending conditions

	L"胜利!",								//STR_AR_OVER_VICTORY
	L"失败!",								//STR_AR_OVER_DEFEAT
	L"投降!",							//STR_AR_OVER_SURRENDERED
	L"被俘!",								//STR_AR_OVER_CAPTURED
	L"撤退!",								//STR_AR_OVER_RETREATED

	//These are the labels for the different types of enemies we fight in autoresolve.

	L"民兵",							//STR_AR_MILITIA_NAME,
	L"精兵",							//STR_AR_ELITE_NAME,
	L"部队",							//STR_AR_TROOP_NAME,
	L"行政人员",								//STR_AR_ADMINISTRATOR_NAME,
	L"异形",								//STR_AR_CREATURE_NAME,

	//Label for the length of time the battle took

	L"战斗用时",						//STR_AR_TIME_ELAPSED,

	//Labels for status of merc if retreating.  (UPPERCASE)

	L"已撤退",						//STR_AR_MERC_RETREATED,
	L"正在撤退",						//STR_AR_MERC_RETREATING,
	L"撤退",						//STR_AR_MERC_RETREAT,

	//PRE BATTLE INTERFACE STRINGS 
	//Goes on the three buttons in the prebattle interface.  The Auto resolve button represents
	//a system that automatically resolves the combat for the player without having to do anything.
	//These strings must be short (two lines -- 6-8 chars per line)

	L"自动战斗",							//STR_PB_AUTORESOLVE_BTN,
	L"进入战区",							//STR_PB_GOTOSECTOR_BTN,
	L"撤退佣兵",							//STR_PB_RETREATMERCS_BTN,

	//The different headers(titles) for the prebattle interface.
	L"遭遇敌军",						//STR_PB_ENEMYENCOUNTER_HEADER,
	L"敌军入侵",						//STR_PB_ENEMYINVASION_HEADER, // 30
	L"敌军伏击",
	L"进入敌占区",			//STR_PB_ENTERINGENEMYSECTOR_HEADER
	L"异形攻击",					//STR_PB_CREATUREATTACK_HEADER
	L"血猫伏击",					//STR_PB_BLOODCATAMBUSH_HEADER
	L"进入血猫巢穴",

	//Various single words for direct translation.  The Civilians represent the civilian
	//militia occupying the sector being attacked.  Limited to 9-10 chars

	L"地区",
	L"敌军",
	L"佣兵",
	L"民兵",
	L"异形",
	L"血猫",
	L"区域",
	L"无人",	//If there are no uninvolved mercs in this fight.
	L"N/A",			//Acronym of Not Applicable
	L"日",			//One letter abbreviation of day
	L"时",			//One letter abbreviation of hour

	//TACTICAL PLACEMENT USER INTERFACE STRINGS
	//The four buttons

	L"清除",
	L"分散",
	L"集中",
	L"完成",

	//The help text for the four buttons.  Use \n to denote new line (just like enter).

	L"清除所有佣兵的布阵, 然后您可以\n重新手动对他们进行安排.(|c)",
	L"每次按本按钮, 就会重新 \n随机分散地布阵佣兵.(|s)",
	L"您可以选择您想集中地布阵佣兵的地方.(|g)",
	L"当您完成对佣兵布阵后, \n请按本按钮. (|E|n|t|e|r)",
	L"开始战斗前, 您必须\n对所有佣兵完成布阵.",

	//Various strings (translate word for word)

	L"区域",
	L"选择进入的位置",

	//Strings used for various popup message boxes.  Can be as long as desired.

	L"看起来不太好.无法进入这里.换个不同的位置吧.",
	L"请把佣兵放在地图的高亮区域里.",

	//These entries are for button popup help text for the prebattle interface.  All popup help
	//text supports the use of \n to denote new line.  Do not use spaces before or after the \n.
	L"自动解决战斗, 不需要\n载入该区域地图.",
	L"当玩家在攻击时, 无法使用\n自动战斗功能.",
	L"进入该区域和敌军作战(|E)",
	L"将小队撤退到先前的区域.(|R)",				//singular version
	L"将所有小队撤退到先前的区域.(|R)", //multiple groups with same previous sector

	//various popup messages for battle conditions.  

	//%c%d is the sector -- ex:  A9
	L"敌军向您在%c%d区域的民兵发起了攻击.",
	//%c%d is the sector -- ex:  A9
	L"异形向您在%c%d区域的民兵发起了攻击.",
	//1st %d refers to the number of civilians eaten by monsters,  %c%d is the sector -- ex:  A9
	//Note:  the minimum number of civilians eaten will be two.
	L"异形攻击了区域%ls, 吃掉了%d名平民.",
	//%ls is the sector location -- ex:  A9: Omerta
	L"敌军向您在区域%ls的佣兵发起了攻击.您的佣兵中没人能进行战斗.",
	//%ls is the sector location -- ex:  A9: Omerta
	L"异形向您在区域%ls的佣兵发起了攻击.您的佣兵中没人能进行战斗.",

};

//This is the day represented in the game clock.  Must be very short, 4 characters max.
static const wchar_t s_chn_gpGameClockString[] = 
	L"日";

//When the merc finds a key, they can get a description of it which
//tells them where and when they found it.
static const wchar_t *s_chn_sKeyDescriptionStrings[sKeyDescriptionStrings_SIZE] =
{
	L"找到的区域: ",
	L"找到的日期: ",
};

//The headers used to describe various weapon statistics.

static StrPointer s_chn_gWeaponStatsDesc[ gWeaponStatsDesc_SIZE] =
{
	L"重量 (%ls):",
	L"状态:",
	L"弹药:", 		// Number of bullets left in a magazine
	L"射程",		// Range
	L"杀伤:",		// Damage
	L"AP:",			// abbreviation for Action Points
	L"=",
};

//The headers used for the merc's money.

static const wchar_t *s_chn_gMoneyStatsDesc[gMoneyStatsDesc_SIZE] =
{
	L"剩余",
	L"金额: ",//this is the overall balance
	L"分割",
	L"金额: ", // the amount he wants to separate from the overall balance to get two piles of money

	L"当前",
	L"余额",
	L"提取",
	L"金额",
};

//The health of various creatures, enemies, characters in the game. The numbers following each are for comment
//only, but represent the precentage of points remaining.

static const wchar_t *s_chn_zHealthStr[zHealthStr_SIZE] =
{
	L"垂死",	//"DYING",		//	>= 0
	L"濒危",	//"CRITICAL", 		//	>= 15
	L"虚弱",	//"POOR",		//	>= 30
	L"受伤",	//"WOUNDED",    	//	>= 45
	L"健康",	//"HEALTHY",    	//	>= 60
	L"强壮",	//"STRONG",     	// 	>= 75
	L"极好",	//"EXCELLENT",		// 	>= 90
};

static const wchar_t *s_chn_gzMoneyAmounts[gzMoneyAmounts_SIZE] =
{
	L"$1000",
	L"$100",
	L"$10",
	L"完成",
	L"分割",
	L"提取",
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
static const wchar_t s_chn_gzProsLabel[] = 
	L"优点:";
static const wchar_t s_chn_gzConsLabel[] = 
	L"缺点:";

//Conversation options a player has when encountering an NPC
static StrPointer s_chn_zTalkMenuStrings[zTalkMenuStrings_SIZE] =
{
	L"再说一次?",	//meaning "Repeat yourself"
	L"友好",		//approach in a friendly
	L"直率",		//approach directly - let's get down to business
	L"恐吓",		//approach threateningly - talk now, or I'll blow your face off
	L"给予",
	L"招募",
};

//Some NPCs buy, sell or repair items. These different options are available for those NPCs as well.
static StrPointer s_chn_zDealerStrings[zDealerStrings_SIZE] =
{
	L"买/卖",
	L"买",
	L"卖",
	L"修理",
};

static const wchar_t s_chn_zDialogActions[] = L"Done";


//These are vehicles in the game.

static const wchar_t *s_chn_pVehicleStrings[pVehicleStrings_SIZE] =
{
	L"Eldorado",
	L"悍马", // a hummer jeep/truck -- military vehicle
	L"冰淇淋车",
	L"吉普",
	L"坦克",
	L"直升机",
};

static const wchar_t *s_chn_pShortVehicleStrings[pVehicleStrings_SIZE] =
{
	L"Eldor.",
	L"悍马", // a hummer jeep/truck -- military vehicle
	L"冰淇淋",
	L"吉普",
	L"坦克",
	L"直升机", 				// the helicopter
};

static const wchar_t *s_chn_zVehicleName[pVehicleStrings_SIZE] =
{
	L"Eldor.",
	L"悍马", // a hummer jeep/truck -- military vehicle
	L"冰淇淋",
	L"吉普",
	L"坦克",
	L"直升机", 		//an abbreviation for Helicopter
};


//These are messages Used in the Tactical Screen

static StrPointer s_chn_TacticalStr[TacticalStr_SIZE] =
{
	L"空袭",
	L"自动包扎?",

	// CAMFIELD NUKE THIS and add quote #66.

	L"%ls发现运来的货品短缺了几件.",

	// The %ls is a string from pDoorTrapStrings

	L"锁上有%ls.",
	L"未上锁.",
	L"锁上未被设置陷阱.",
	// The %ls is a merc name
	L"%ls无对应的钥匙.",
	L"锁上未被设置陷阱.",
	L"锁住了.",
	L"门",
	L"有陷阱的",
	L"锁住的",
	L"没锁的",
	L"被打烂的",
	L"这里有一个开关.启动它吗?",
	L"解除陷阱?",
	L"更多...",

	// In the next 2 strings, %ls is an item name

	L"%ls被放在地上了.",
	L"%ls被交给%ls了.",

	// In the next 2 strings, %ls is a name

	L"%ls已经被完全支付.",
	L"%ls还拖欠%d.",
	L"选择引爆的频率", //in this case, frequency refers to a radio signal
	L"设定几个回合后爆炸: ", //how much time, in turns, until the bomb blows
	L"设定遥控雷管的频率: ",//in this case, frequency refers to a radio signal
	L"解除诡雷?",
	L"移掉蓝旗?",
	L"在这里插上蓝旗吗?",
	L"结束回合",

	// In the next string, %ls is a name. Stance refers to way they are standing.

	L"您确定要攻击%ls吗?",
	L"车辆无法变动姿势.",
	L"机器人无法变动姿势.",

	// In the next 3 strings, %ls is a name

	L"%ls无法在这里变为该姿势.",
	L"%ls无法在这里被包扎.",
	L"%ls不需要包扎.",
	L"不能移动到那儿.",
	L"您的队伍已经满员了.无空位雇佣新队员.",	//there's no room for a recruit on the player's team

	// In the next string, %ls is a name

	L"%ls已经被招募.",

	// Here %ls is a name and %d is a number

	L"尚拖欠%ls$%d.",

	// In the next string, %ls is a name

	L"护送%ls吗?",

	// In the next string, the first %ls is a name and the second %ls is an amount of money (including $ sign)

	L"要雇佣%ls吗(每日得支付%ls)?",

	// This line is used repeatedly to ask player if they wish to participate in a boxing match. 

	L"您要进行拳击比赛吗?",

	// In the next string, the first %ls is an item name and the 
	// second %ls is an amount of money (including $ sign)

	L"要买%ls吗(得支付%ls)?",

	// In the next string, %ls is a name

	L"%ls接受第%d小队的护送.",

	// These messages are displayed during play to alert the player to a particular situation

	L"卡壳",					//weapon is jammed.
	L"机器人需要%ls口径的子弹.",	//Robot is out of ammo
	L"扔到那儿?那不可能.",		//Merc can't throw to the destination he selected

	// These are different buttons that the player can turn on and off.

	L"潜行模式 (|Z)",
	L"地图屏幕 (|M)",
	L"结束回合 (|D)",
	L"谈话",
	L"禁音",
	L"起身 (|P|g|U|p)",
	L"光标层次 (|T|a|b)",
	L"攀爬/跳跃",
	L"伏下 (|P|g|D|n)",
	L"检查",
	L"上一佣兵",
	L"下一佣兵 (|S|p|a|c|e)",
	L"选项 (|O)",
	L"扫射模式 (|B)",
	L"查看/转向(|L)",
	L"生命: %d/%d\n精力: %d/%d\n士气: %ls",
	L"厄?",		//this means "what?"
	L"继续",		//an abbrieviation for "Continued"
	L"对%ls关闭禁音模式.",
	L"对%ls打开禁音模式.",
	L"耐久度: %d/%d\n油量: %d/%d",
	L"下车",
	L"切换小队 ( |S|h|i|f|t |S|p|a|c|e )",
	L"驾驶",
	L"N/A",		//this is an acronym for "Not Applicable."
	L"使用 (拳头)",
	L"使用 (武器)",
	L"使用 (刀具)",
	L"使用 (爆炸品)",
	L"使用 (医疗用品)",
	L"(抓住)",
	L"(装填弹药)",
	L"(给予)",
	L"%ls被触发了.",
	L"%ls已到达.",
	L"%ls用完了行动点数(AP).",
	L"%ls无法行动.",
	L"%ls包扎好了.",
	L"%ls用完了绷带.",
	L"这个区域中有敌军.",
	L"视野中无敌军.",
	L"没有足够的行动点数(AP).",
	L"没人使用遥控器.",
	L"扫射光了子弹!",
	L"敌兵",
	L"异形",
	L"民兵",
	L"平民",
	L"离开区域",
	L"确定",
	L"取消",
	L"选择佣兵",
	L"小队的所有佣兵",
	L"前往区域",
	L"前往地图",
	L"您不能从这边离开此区域.",
	L"%ls太远了.",
	L"不显示树冠",
	L"显示树冠",
	L"乌鸦"	,				//Crow, as in the large black bird
	L"颈部",
	L"头部",
	L"躯体",
	L"腿部",
	L"要告诉女王她想知道的情报吗?",
	L"获得指纹ID",
	L"指纹ID无效.无法使用该武器.",
	L"达成目标",
	L"路被堵住了",
	L"存钱/取钱",	//Help text over the $ button on the Single Merc Panel
	L"没人需要包扎.",
	L"卡壳",											// Short form of JAMMED, for small inv slots
	L"无法到达那里.", // used ( now ) for when we click on a cliff
	L"那人拒绝移动.",
	// In the following message, '%ls' would be replaced with a quantity of money (e.g. $200)
	L"您同意支付%ls吗?",
	L"您要接受免费治疗吗?",
	L"您同意让佣兵和Daryl结婚吗?",
	L"钥匙面板",
	L"您不能这样用EPC.",
	L"不杀Krott?",
	L"超出武器的有效射程.",
	L"矿工",
	L"车辆只能在区域间旅行",
	L"现在不能自动包扎",
	L"%ls被堵住了",
	L"被Deidranna的军队俘虏的佣兵, 被关押在这里!",
	L"锁被击中了",
	L"锁被破坏了",
	L"其他人在使用这扇门.",
	L"耐久度: %d/%d\n油量: %d/%d",
	L"%ls看不见%ls.", // Cannot see person trying to talk to
};

//Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different circumstances in the "exiting sector" interface.
static const wchar_t *s_chn_pExitingSectorHelpText[pExitingSectorHelpText_SIZE] =
{
	//Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box is checked.
	L"如果勾中, 将立即进入邻近的区域.",
	L"如果勾中, 您将被立即自动放置在地图屏幕, \n因为您的佣兵要花些时间来行军.",

	//If you attempt to leave a sector when you have multiple squads in a hostile sector.
	L"该区域被敌军占据.您不能将佣兵留在这里.\n在进入其它区域前, 您必须把这里的问题解决.",

	//Because you only have one squad in the sector, and the "move all" option is checked, the "go to sector" option is locked to on. 
	//The helptext explains why it is locked.
	L"让留下的佣兵离开本区域, \n将立即进入邻近的区域.",
	L"让留下的佣兵离开本区域, \n您将被立即自动放置在地图屏幕, \n因为您的佣兵要花些时间来行军.",

	//If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being escorted.  The "single" button is disabled.
	L"%ls需要被您的佣兵护送, 他 (她) 无法独自离开本区域.",

	//If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc will be prohibited from leaving alone.
	//There are several strings depending on the gender of the merc and how many EPCs are in the squad.
	//DO NOT USE THE NEWLINE HERE AS IT IS USED FOR BOTH HELPTEXT AND SCREEN MESSAGES!
	L"%ls无法独自离开本区域, 因为他得护送%ls.", //male singular
	L"%ls无法独自离开本区域, 因为她得护送%ls.", //female singular
	L"%ls无法独自离开本区域, 因为他得护送多人.", //male plural
	L"%ls无法独自离开本区域, 因为她得护送多人.", //female plural

	//If one or more of your mercs in the selected squad aren't in range of the traversal area, then the  "move all" option is disabled,
	//and this helptext explains why.
	L"如果要让小队在区域间移动的话, \n您的全部队员都必须在附近.",

	//Standard helptext for single movement.  Explains what will happen (splitting the squad)
	L"如果勾中,  %ls将独自行军, \n而且被自动重新分配到一个单独的小队中.",

	//Standard helptext for all movement.  Explains what will happen (moving the squad)
	L"如果勾中, 您当前选中的小队\n将会离开本区域, 开始行军.",

	//This strings is used BEFORE the "exiting sector" interface is created.  If you have an EPC selected and you attempt to tactically
	//traverse the EPC while the escorting mercs aren't near enough (or dead, dying, or unconscious), this message will appear and the
	//"exiting sector" interface will not appear.  This is just like the situation where
	//This string is special, as it is not used as helptext.  Do not use the special newline character (\n) for this string.
	L"%ls正在被您的佣兵护送, 无法独自离开本区域.您的佣兵必须在附近以护送%ls离开.",
};



static const wchar_t *s_chn_pRepairStrings[pRepairStrings_SIZE] =
{
	L"物品",	// tell merc to repair items in inventor
	L"SAM基地",		// tell merc to repair SAM site - SAM is an acronym for Surface to Air Missile
	L"取消",		// cancel this menu
	L"机器人",		// repair the robot
};


// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."

static const wchar_t *s_chn_sPreStatBuildString[sPreStatBuildString_SIZE] =
{
	L"丧失",// the merc has lost a statistic
	L"获得",// the merc has gained a statistic
	L"点",// singular
	L"点",// plural
	L"级",//singular
	L"级",//plural
};

static const wchar_t *s_chn_sStatGainStrings[sStatGainStrings_SIZE] =
{
	L"生命.",
	L"敏捷.",
	L"灵巧.",
	L"智慧.",
	L"医疗技能.",
	L"爆破技能.",
	L"机械技能.",
	L"枪法技能.",
	L"级别",
	L"力量",
	L"领导",
};


static const wchar_t *s_chn_pHelicopterEtaStrings[pHelicopterEtaStrings_SIZE] =
{
	L"总距离: ", 			// total distance for helicopter to travel
	L"安全: ", 			// distance to travel to destination
	L"不安全: ",		// distance to return from destination to airport
	L"总价: ", 		// total cost of trip by helicopter
	L"耗时: ", 			// ETA is an acronym for "estimated time of arrival"
	L"直升机油量不够, 必须在敌占区着陆.", 	// warning that the sector the helicopter is going to use for refueling is under enemy control ->
	L"乘客: ",
	L"选择Skyrider还是 '着陆点'?",
	L"Skyrider",
	L"着陆点",
};

static const wchar_t s_chn_sMapLevelString[] = 
	L"地层:"; // what level below the ground is the player viewing in mapscreen

static const wchar_t s_chn_gsLoyalString[] = 
	L"%d%% 忠诚"; // the loyalty rating of a town ie : Loyal 53%


// error message for when player is trying to give a merc a travel order while he's underground.
static const wchar_t s_chn_gsUndergroundString[] = 
	L"不能在地下行军.";

static const wchar_t *s_chn_gsTimeStrings[gsTimeStrings_SIZE] =
{
	L"h",				// hours abbreviation
	L"m",				// minutes abbreviation
	L"s",				// seconds abbreviation
	L"d",				// days abbreviation
};

// text for the various facilities in the sector

static const wchar_t *s_chn_sFacilitiesStrings[sFacilitiesStrings_SIZE] =
{
	L"无",
	L"医院",
	L"工厂",
	L"监狱",
	L"军事基地",
	L"机场",
	L"靶场",		// a field for soldiers to practise their shooting skills
};

// text for inventory pop up button

static const wchar_t *s_chn_pMapPopUpInventoryText[pMapPopUpInventoryText_SIZE] =
{
	L"存货",
	L"离开",
};

// town strings

static const wchar_t *s_chn_pwTownInfoStrings[pwTownInfoStrings_SIZE] =
{
	L"大小",				// 0 // size of the town in sectors
	L"占领度",				// how much of town is controlled
	L"矿区",			// mine associated with this town
	L"忠诚度",				// 5 // the loyalty level of this town
	L"主要设施",			// main facilities in this town
	L"民兵训练度",		// 10 // state of civilian training in town
	L"民兵",			// the state of the trained civilians in the town
};

// Mine strings

static const wchar_t *s_chn_pwMineStrings[pwMineStrings_SIZE] =
{
	L"矿井",			// 0
	L"银块",
	L"金块",
	L"当前日产量",
	L"最高产量",
	L"废弃",				// 5
	L"关闭",
	L"矿脉耗尽",
	L"生产",
	L"状态",
	L"生产率",
	L"矿石类型",			// 10
	L"占领度",
	L"忠诚度",
};

// blank sector strings

static const wchar_t *s_chn_pwMiscSectorStrings[pwMiscSectorStrings_SIZE] =
{
	L"敌军",
	L"区域",
	L"物品数量",
	L"未知",
	L"已占领",
	L"是",
	L"否",
};

// error strings for inventory

static const wchar_t *s_chn_pMapInventoryErrorString[pMapInventoryErrorString_SIZE] =
{
	L"无法选择该佣兵.",  //MARK CARTER
	L"%ls不在此区域, 不能拿到该物品.",
	L"在战斗时, 您只能动手捡起物品.",
	L"在战斗时, 您只能动手放下物品.",
	L"%ls不在该区域, 不能放下那个物品.",
};

static const wchar_t *s_chn_pMapInventoryStrings[pMapInventoryStrings_SIZE] =
{
	L"位置",		// sector these items are in
	L"物品总数", 		// total number of items in sector
};


// movement menu text

static const wchar_t *s_chn_pMovementMenuStrings[pMovementMenuStrings_SIZE] =
{
	L"在%ls区域调动佣兵", 	// title for movement box
	L"安排行军路线",	// done with movement menu, start plotting movement
	L"取消",		// cancel this menu
	L"其它",		// title for group of mercs not on squads nor in vehicles
};


static const wchar_t *s_chn_pUpdateMercStrings[pUpdateMercStrings_SIZE] =
{
	L"糟了: ", 			// an error has occured
	L"佣兵合同到期了: ", 	// this pop up came up due to a merc contract ending
	L"佣兵完成了分配的任务: ", // this pop up....due to more than one merc finishing assignments
	L"佣兵醒来了, 继续干活: ", // this pop up ....due to more than one merc waking up and returing to work
	L"佣兵困倦了: ", // this pop up ....due to more than one merc being tired and going to sleep
	L"合同快到期了: ", 	// this pop up came up due to a merc contract ending
};

// map screen map border buttons help text

static const wchar_t *s_chn_pMapScreenBorderButtonHelpText[pMapScreenBorderButtonHelpText_SIZE] =
{
	L"显示城镇 (|W)",
	L"显示矿井 (|M)",
	L"显示队伍和敌人 (|T)",
	L"显示领空 (|A)",
	L"显示物品 (|I)",
	L"显示民兵和敌人 (|Z)",
};


static const wchar_t *s_chn_pMapScreenBottomFastHelp[pMapScreenBottomFastHelp_SIZE] =
{
	L"笔记本电脑 (|L)",
	L"战术屏幕 (|E|s|c)",
	L"选项 (|O)",
	L"时间压缩 (|+)", 	// time compress more
	L"时间压缩 (|-)", 	// time compress less
	L"上一信息 (|U|p)\n上页 (|P|g|U|p)", // previous message in scrollable list
	L"下一信息 (|D|o|w|n)\n下页 (|P|g|D|n)", 	// next message in the scrollable list
	L"开始/停止时间压缩 (|S|p|a|c|e)",	// start/stop time compression
};

static const wchar_t s_chn_pMapScreenBottomText[] = 
	L"帐户余额"; // current balance in player bank account

static const wchar_t s_chn_pMercDeadString[] = 
	L"%ls阵亡.";


static const wchar_t s_chn_pDayStrings[] = 
	L"日";

// the list of email sender names

static const wchar_t *s_chn_pSenderNameList[pSenderNameList_SIZE] =
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
	L"Stephen",
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
	L"M.I.S. Insurance",
	L"Bobby Rays",
	L"Kingpin",
	L"John Kulba",
	L"A.I.M.",
};


// new mail notify string
static const wchar_t s_chn_pNewMailStrings[] = 
	L"您有新的邮件...";


// confirm player's intent to delete messages

static const wchar_t *s_chn_pDeleteMailStrings[pDeleteMailStrings_SIZE] =
{
	L"删除邮件?",
	L"删除未读的邮件?",
};


// the sort header strings

static const wchar_t *s_chn_pEmailHeaders[pEmailHeaders_SIZE] =
{
	L"来自: ",
	L"标题: ",
	L"日期: ",
};

// email titlebar text
static const wchar_t s_chn_pEmailTitleText[] = 
	L"邮箱";


// the financial screen strings
static const wchar_t s_chn_pFinanceTitle[] = 
	L"Bookkeeper Plus"; // the name we made up for the financial program in the game

static const wchar_t *s_chn_pFinanceSummary[pFinanceSummary_SIZE] =
{
	L"收入: ", 				// credit (subtract from) to player's account
	L"支出: ",				// debit (add to) to player's account
	L"昨日实际收入: ",
	L"昨日其它存款: ",
	L"昨日支出: ",
	L"昨日日终余额: ",
	L"今日实际收入: ",
	L"今日其它存款: ",
	L"今日支出: ",
	L"今日当前余额: ",
	L"预期收入: ",
	L"明日预计余额: ", 		// projected balance for player for tommorow
};


// headers to each list in financial screen

static const wchar_t *s_chn_pFinanceHeaders[pFinanceHeaders_SIZE] =
{
	L"天数",			// the day column
	L"收入", 			// the credits column (to ADD money to your account)
	L"支出",				// the debits column (to SUBTRACT money from your account)
	L"交易记录", // transaction type - see TransactionText below
	L"余额",		// balance at this point in time
	L"页数",				// page number
	L"日", 			// the day(s) of transactions this page displays
};


static const wchar_t *s_chn_pTransactionText[pTransactionText_SIZE] =
{
	L"自然增值利息",		// interest the player has accumulated so far
	L"匿名存款",
	L"交易费用",
	L"已雇佣",			// Merc was hired
	L"在Bobby Ray购买货品", // Bobby Ray is the name of an arms dealer
	L"在M.E.R.C开户.",
	L"%ls的医疗保证金",  	// medical deposit for merc
	L"IMP心理剖析分析", 		// IMP is the acronym for International Mercenary Profiling
	L"为%ls购买保险",
	L"缩短%ls的保险期限",
	L"延长%ls的保险期限",			// johnny contract extended
	L"取消%ls的保险",
	L"%ls的保险索赔",	// insurance claim for merc
	L"1日", 				// merc's contract extended for a day
	L"1周",				// merc's contract extended for a week
	L"2周",				// ... for 2 weeks
	L"矿井收入",
	L"", //String nuked
	L"买花",
	L"%ls的医疗保证金的全额退款",
	L"%ls的医疗保证金的部分退款",
	L"%ls的医疗保证金无退款",
	L"付给%ls金钱",// %ls is the name of the npc being paid
	L"支付给%ls的佣金", 	// transfer funds to a merc
	L"%ls退回的佣金",	 // transfer funds from a merc
	L"在%ls训练民兵",	 // initial cost to equip a town's militia
	L"向%ls购买了物品.", //is used for the Shop keeper interface.  The dealers name will be appended to the end of the string.
	L"%ls存款",
};

// helicopter pilot payment

static const wchar_t *s_chn_pSkyriderText[pSkyriderText_SIZE] =
{
	L"付给 Skyrider $%d",	// skyrider was paid an amount of money
	L"还欠 Skyrider $%d",	// skyrider is still owed an amount of money
	L"Skyrider没有乘客. 如果您试图运送此区域的佣兵, 首先要分配他们进入 '交通工具' －> '直升机'.",
};


// strings for different levels of merc morale

static const wchar_t *s_chn_pMoralStrings[pMoralStrings_SIZE] =
{
	L"高涨",
	L"良好",
	L"稳定",
	L"低下",
	L"惊慌",
	L"糟糕",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.
static const wchar_t s_chn_str_left_equipment[]   = 
	L"%ls的装备现在可以在%ls (%c%d)获得.";

// Status that appears on the Map Screen

static const wchar_t *s_chn_pMapScreenStatusStrings[pMapScreenStatusStrings_SIZE] =
{
	L"生命",
	L"精力",
	L"士气",
	L"状态",	// the condition of the current vehicle (its "health")
	L"油量",	// the fuel level of the current vehicle (its "energy")
};


static const wchar_t *s_chn_pMapScreenPrevNextCharButtonHelpText[pMapScreenPrevNextCharButtonHelpText_SIZE] =
{
	L"上一佣兵 (|L|e|f|t)",	//"Previous Merc (|L|e|f|t)", 			// previous merc in the list
	L"下一佣兵 (|R|i|g|h|t)", 				// next merc in the list
};


static const wchar_t s_chn_pEtaString[] = 
	L"耗时:"; // eta is an acronym for Estimated Time of Arrival

static const wchar_t *s_chn_pTrashItemText[pTrashItemText_SIZE] =
{
	L"您将不会再见到它了.您确定吗?",	 	// do you want to continue and lose the item forever
	L"此物品看起来非常重要.您真的非常非常确定要扔掉它吗?", // does the user REALLY want to trash this item
};


static const wchar_t *s_chn_pMapErrorString[pMapErrorString_SIZE] =
{
	L"小队不能行军, 因为有人在睡觉.",

//1-5
	L"首先要回到地面, 才能移动小队.",
	L"行军?那里是敌占区!",
	L"必须给佣兵分配小队或者交通工具才能开始行军.",
	L"您现在没有任何队员.", 	// you have no members, can't do anything
	L"佣兵无法遵从命令.",		// merc can't comply with your order
//6-10
	L"需要有人护送才能行军.请把他分进一个小队里.", // merc can't move unescorted .. for a male
	L"需要有人护送才能行军.请把她分进一个小队里.", // for a female
	L"佣兵尚未到达Arulco!",
	L"看来得先谈妥合同.",
	L"无法发出行军命令.目前有空袭.",
//11-15
	L"行军? 这里正在战斗中!",
	L"您在区域%ls被血猫伏击了!",
	L"您刚刚进入了 %ls 区域, 这里是血猫的巢穴!", // HEADROCK HAM 3.6: Added argument.
	L"",
	L"在%ls的SAM基地被敌军占领了.",
//16-20
	L"在%ls的矿井被敌军占领了.您的日收入下降为每日%ls.",
	L"敌军未遭到抵抗, 就占领了%ls",
	L"您至少有一名佣兵不能被分配此任务.",
	L"%ls无法加入%ls, 因为它已经满员了",
	L"%ls无法加入%ls, 因为它太远了.",
//21-25
	L"在%ls的矿井被敌军占领了!",
	L"敌军入侵了%ls处的SAM基地",
	L"敌军入侵了%ls",
	L"敌军在%ls出没.",
	L"敌军占领了%ls.",
//26-30
	L"您的佣兵中至少有一人不能睡眠.",
	L"您的佣兵中至少有一人不能醒来.",
	L"训练完毕, 才会出现民兵.",
	L"现在无法对%ls下达行军命令.",
	L"不在城镇边界的民兵无法行军到另一个区域.",
//31-35
	L"您不能在%ls拥有民兵.",
	L"车是空的, 无法移动!",
	L"%ls受伤太严重了, 无法行军!",
	L"您必须首先离开博物馆!",
	L"%ls死了!",
//36-40
	L"%ls无法转到%ls因为它在移动中",
	L"%ls无法那样进入交通工具",
	L"%ls无法加入%ls",
	L"在您雇佣新的佣兵前, 您不能压缩时间.",
	L"车辆只能在公路上开!",
//41-45
	L"在佣兵移动时, 您不能重新分配任务",
	L"车辆没油了!",
	L"%ls太累了, 以致不能行军.",
	L"车上无人能够驾驶.",
	L"该小队的佣兵现在不能移动.",
//46-50
	L"其他佣兵现在不能移动.",
	L"车辆被损坏得太严重了!",
	L"每个区域只能由两名佣兵来训练民兵.",
	L"没有遥控员, 机器人无法移动.请把他们分配在同一个小队.",
};


// help text used during strategic route plotting
static StrPointer s_chn_pMapPlotStrings[] =
{
	L"再点击一下目的地以确认您的最后路线, 或者点击下一个区域以设置更多的路点.",
	L"行军路线已确认.",
	L"目的地未改变.",
	L"行军路线已取消.",
	L"行军路线已缩短.",
};


// help text used when moving the merc arrival sector
static const wchar_t *s_chn_pBullseyeStrings[pBullseyeStrings_SIZE] =
{
	L"点击您想让佣兵着陆的区域.",
	L"好的.佣兵将在%ls着陆",
	L"佣兵不能飞往那里, 领空不安全!",
	L"取消.着陆区域未改变",
	L"%ls上的领空现在不安全了!着陆区域被改为%ls.",
};


// help text for mouse regions

static const wchar_t *s_chn_pMiscMapScreenMouseRegionHelpText[pMiscMapScreenMouseRegionHelpText_SIZE] =
{
	L"进入存货屏幕 (|E|n|t|e|r)",
	L"扔掉物品",	//"Throw Item Away",
	L"离开存货屏幕 (|E|n|t|e|r)",
};


static const wchar_t s_chn_str_he_leaves_where_drop_equipment[]  = 
	L"让%ls把装备留在他所在的(%ls), 离开Arulco, 或者留在%ls (%ls), 飞离Arulco?";
static const wchar_t s_chn_str_she_leaves_where_drop_equipment[] = 
	L"让%ls把装备留在她所在的(%ls), 离开Arulco, 或者留在%ls (%ls), 飞离Arulco?";
static const wchar_t s_chn_str_he_leaves_drops_equipment[]       = 
	L"%ls要离开了, 他的装备将被留在%ls.";
static const wchar_t s_chn_str_she_leaves_drops_equipment[]      = 
	L"%ls要离开了, 她的装备将被留在%ls.";


// Text used on IMP Web Pages

static const wchar_t *s_chn_pImpPopUpStrings[pImpPopUpStrings_SIZE] =
{
	L"无效的授权号",
	L"您试图重新开始整个测试.您确定吗?",
	L"请输入正确的全名和性别.",
	L"对您的财政状况的预先分析显示了您无法负担心理剖析的费用.",
	L"现在不是个有效的选择.",
	L"要进行心理剖析, 您的队伍中必须至少留一个空位.",
	L"测试完毕.",
};


// button labels used on the IMP site

static const wchar_t *s_chn_pImpButtonText[pImpButtonText_SIZE] =
{
	L"关于我们",			// about the IMP site
	L"开始",		// begin profiling
	L"性格",		// personality section
	L"属性", 		// personal stats/attributes section
	L"外表", 			// changed from portrait - SANDRO
	L"嗓音%d",		// the voice selection
	L"完成",		// done profiling
	L"重新开始",	// start over profiling
	L"是的, 我选择了高亮突出的回答.",
	L"是",
	L"否",
	L"完成",			// finished answering questions
	L"上一个",			// previous question..abbreviated form
	L"下一个", 			// next question
	L"是的, 我确定.",	// yes, I am certain
	L"不, 我想重新开始.",
	L"是",
	L"否",
	L"后退",		// back one page
	L"取消",		// cancel selection
	L"是的, 我确定.",
	L"不, 让我再看看.",
	L"注册",			// the IMP site registry..when name and gender is selected
	L"分析中", 		// analyzing your profile results
	L"完成",
	L"嗓音",
};

static const wchar_t *s_chn_pExtraIMPStrings[pExtraIMPStrings_SIZE] =
{
	L"选择性格, 开始实际分析.",
	L"既然您已经完成了性格选择, 下面选择您的能力.",
	L"能力确认完毕, 您可以继续选择头像.",
	L"选择最符合您的声音, 完成整个过程.",
};

static const wchar_t s_chn_pFilesTitle[] = 
	L"File Viewer";

static const wchar_t *s_chn_pFilesSenderList[pFilesSenderList_SIZE] =
{
	L"侦察报告",
	L"1号通缉令",
	L"2号通缉令",
	L"3号通缉令",
	L"4号通缉令",
	L"5号通缉令",
	L"6号通缉令", // sixth intercept file
};

// Text having to do with the History Log
static const wchar_t s_chn_pHistoryTitle[] = 
	L"历史记录";

static const wchar_t *s_chn_pHistoryHeaders[pHistoryHeaders_SIZE] =
{
	L"日",		// the day the history event occurred
	L"页",	// the current page in the history report we are in
	L"日",	// the days the history report occurs over
	L"位置",	// location (in sector) the event occurred
	L"事件",	// the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
static const wchar_t *s_chn_pHistoryStrings[pHistoryStrings_SIZE] =
{
	L"",																						// leave this line blank
	//1-5
	L"从A.I.M雇佣了%ls.",
	L"从M.E.R.C雇佣了%ls.",
	L"%ls死了.",	//"%ls died.", 															// merc was killed
	L"在M.E.R.C开户.",
	L"接受Enrico Chivaldori的委托",	//"Accepted Assignment From Enrico Chivaldori",
	//6-10
	L"IMP已生成",
	L"为%ls购买保险.",
	L"取消%ls的保险合同.",
	L"收到%ls的保险索赔.",
	L"延长一日%ls的合同.",
	//11-15
	L"延长一周%ls的合同.",
	L"延长两周%ls的合同.",
	L"%ls被解雇了.",
	L"%ls退出了.",
	L"开始任务.",
	//16-20
	L"完成任务.",
	L"和%ls的矿主交谈",
	L"解放了%ls",
	L"启用作弊",
	L"食物会在明天送达Omerta",
	//21-25
	L"%ls离队并成为了Daryl Hick的妻子",
	L"%ls的合同到期了.",
	L"招募了%ls.",
	L"Enrico抱怨进展缓慢",
	L"战斗胜利",
	//26-30
	L"%ls的矿井开始缺乏矿石",
	L"%ls的矿井采完了矿石",
	L"%ls的矿井关闭了",	//"%ls mine was shut down",
	L"%ls的矿井复工了",
	L"发现一个叫Tixa的监狱.",
	//31-35
	L"打听到一个叫Orta的秘密武器工厂.",
	L"在Orta的科学家捐赠了大量的火箭枪.",
	L"Deidranna女王在利用死尸做某些事情.",
	L"Frank谈到了在San Mona的拳击比赛.",
	L"一个病人说他在矿井里看到了一些东西.",
	//36-40
	L"遇到一个叫Devin的人, 他出售爆炸物.",
	L"偶遇Mike, 前AIM名人!",
	L"遇到Tony, 他做武器买卖.",
	L"从Krott中士那里得到一把火箭枪.",
	L"把Angel皮衣店的契约给了Kyle.",
	//41-45
	L"Madlab提议做一个机器人.",
	L"Gabby能制作对付虫子的隐形药.",
	L"Keith歇业了.",
	L"Howard给Deidranna女王提供氰化物.",
	L"遇到Keith -Cambria的杂货商.",
	//46-50
	L"遇到Howrd, 一个在Balime的医药商.",
	L"遇到Perko, 他开了一家小修理档口..",
	L"遇到在Balime的Sam, 他有一家五金店.",
	L"Franz做电子产品和其他货物的生意.",
	L"Arnold在Grumm开了一家修理店.",
	//51-55
	L"Fredo在Grumm修理电子产品.",
	L"收到在Balime的有钱人的捐款.",
	L"遇到一个叫Jake的废品商人.",
	L"一个流浪者给了我们一张电子钥匙卡.",
	L"贿赂了Walter, 让他打开地下室的门.",
	//56-60
	L"如果Dave有汽油, 他会免费进行加油.",
	L"贿赂Pablo.",
	L"Kingping拿回了San Mona矿井中的钱.",
	L"%ls赢了拳击赛",
	L"%ls输了拳击赛",
	//61-65
	L"%ls丧失了拳击赛的参赛资格",
	L"在废弃的矿井里找到一大笔钱.",
	L"遭遇Kingpin派出的杀手.",
	L"该区域失守",	//ENEMY_INVASION_CODE
	L"成功防御该区域",
	//66-70
	L"作战失败",			//ENEMY_ENCOUNTER_CODE
	L"致命的伏击",		//ENEMY_AMBUSH_CODE
	L"杀光了敌军的伏兵",
	L"攻击失败",		//ENTERING_ENEMY_SECTOR_CODE
	L"攻击成功!",
	//71-75
	L"异形攻击",		//CREATURE_ATTACK_CODE
	L"被血猫吃掉了",	//BLOODCAT_AMBUSH_CODE
	L"宰掉了血猫",
	L"%ls被干掉了",
	L"把一个恐怖分子的头颅给了Carmen.",
	L"Slay走了",
	L"干掉了%ls",
};

static const wchar_t s_chn_pHistoryLocations[] = 
	L"N/A"; // N/A is an acronym for Not Applicable

// icon text strings that appear on the laptop

static const wchar_t *s_chn_pLaptopIcons[pLaptopIcons_SIZE] =
{
	L"邮箱",
	L"网页",
	L"财务",
	L"人事",
	L"日志",
	L"文件",
	L"关闭",
	L"sir-FER 4.0",			// our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added

static const wchar_t *s_chn_pBookMarkStrings[pBookMarkStrings_SIZE] =
{
	L"A.I.M.",
	L"Bobby Ray's",
	L"I.M.P",
	L"M.E.R.C.",
	L"公墓",
	L"花店",
	L"M.I.S 保险公司",
	L"取消",
};

// When loading or download a web page

static const wchar_t *s_chn_pDownloadString[pDownloadString_SIZE] =
{
	L"下载中",
	L"重载中",
};

//This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

static const wchar_t *s_chn_gsAtmStartButtonText[gsAtmStartButtonText_SIZE] =
{
	L"状态",			// view stats of the merc
	L"物品清单", 			// view the inventory of the merc
	L"雇佣合同",
};

// Web error messages. Please use foreign language equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator

static const wchar_t s_chn_pErrorStrings[] = 
	L"主机连接时断时续.预计需要较长的传输时间.";


static const wchar_t s_chn_pPersonnelString[] = 
	L"佣兵:"; // mercs we have


static const wchar_t s_chn_pWebTitle[] = 
	L"sir-FER 4.0"; // our name for the version of the browser, play on company name


// The titles for the web program title bar, for each page loaded

static const wchar_t *s_chn_pWebPagesTitles[pWebPagesTitles_SIZE] =
{
	L"A.I.M.",
	L"A.I.M. 成员",
	L"A.I.M. 肖像",		// a mug shot is another name for a portrait
	L"A.I.M. 排序",
	L"A.I.M.",
	L"A.I.M. 前成员",
	L"A.I.M. 规则",
	L"A.I.M. 历史",
	L"A.I.M. 链接",
	L"M.E.R.C.",
	L"M.E.R.C. 账号",
	L"M.E.R.C. 注册",
	L"M.E.R.C. 索引",
	L"Bobby Ray's",
	L"Bobby Ray's - 枪械",
	L"Bobby Ray's - 弹药",
	L"Bobby Ray's - 护甲",
	L"Bobby Ray's - 杂货",	//"Bobby Ray's - Misc",							//misc is an abbreviation for miscellaneous
	L"Bobby Ray's - 二手货",
	L"Bobby Ray's - 邮购",
	L"I.M.P.",
	L"I.M.P.",
	L"联合花卉服务公司",
	L"联合花卉服务公司 - 花卉",
	L"联合花卉服务公司 - 订单",
	L"联合花卉服务公司 - 贺卡",
	L"Malleus, Incus & Stapes 保险公司",
	L"信息",
	L"合同",
	L"评论",
	L"McGillicutty's 公墓",
	L"无法找到URL",
	L"Bobby Ray's - 最近的运货",
	L"",
	L"",
};

static const wchar_t *s_chn_pShowBookmarkString[pShowBookmarkString_SIZE] =
{
	L"Sir-Help",
	L"再次点击页面以放进收藏夹.",
};

static const wchar_t *s_chn_pLaptopTitles[pLaptopTitles_SIZE] =
{
	L"邮箱",
	L"File Viewer",
	L"人事",
	L"Bookkeeper Plus",
	L"历史日志",
};

static const wchar_t *s_chn_pPersonnelDepartedStateStrings[pPersonnelDepartedStateStrings_SIZE] =
{
	//reasons why a merc has left.
	L"阵亡",
	L"解雇",
	L"结婚",
	L"合同到期",
	L"退出",
};
// personnel strings appearing in the Personnel Manager on the laptop

static const wchar_t *s_chn_pPersonelTeamStrings[pPersonelTeamStrings_SIZE] =
{
	L"当前成员: ",
	L"离队成员: ",
	L"每日花费: ",
	L"最高日薪: ",
	L"最低日薪: ",
	L"阵亡: ",
	L"解雇: ",
	L"其它: ",
};


static const wchar_t *s_chn_pPersonnelCurrentTeamStatsStrings[pPersonnelCurrentTeamStatsStrings_SIZE] =
{
	L"最低",
	L"平均",
	L"最高",
};


static const wchar_t *s_chn_pPersonnelTeamStatsStrings[pPersonnelTeamStatsStrings_SIZE] =
{
	L"生命",
	L"敏捷",
	L"灵巧",
	L"力量",
	L"领导",
	L"智慧",
	L"级别",
	L"枪法",
	L"机械",
	L"爆破",
	L"医疗",
};


// horizontal and vertical indices on the map screen

static const wchar_t *s_chn_pMapVertIndex[pMapVertIndex_SIZE] =
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

static const wchar_t *s_chn_pMapHortIndex[pMapHortIndex_SIZE] =
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

static const wchar_t *s_chn_pMapDepthIndex[pMapDepthIndex_SIZE] =
{
	L"",
	L"-1",
	L"-2",
	L"-3",
};

// text that appears on the contract button

static const wchar_t s_chn_pContractButtonString[] = 
	L"合同";

// text that appears on the update panel buttons

static const wchar_t *s_chn_pUpdatePanelButtons[pUpdatePanelButtons_SIZE] =
{
	L"继续",
	L"停止",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle

static StrPointer s_chn_LargeTacticalStr[LargeTacticalStr_SIZE] =
{
	L"您在这个地区战败了!",
	L"敌人冷酷无情地处死了您的队员!",
	L"您昏迷的队员被俘虏了!",
	L"您的队员成了敌军的俘虏.",
};


//Insurance Contract.c
//The text on the buttons at the bottom of the screen.

static const wchar_t *s_chn_InsContractText[InsContractText_SIZE] =
{
	L"上页",
	L"下页",
	L"接受",
	L"清除",
};



//Insurance Info
// Text on the buttons on the bottom of the screen

static const wchar_t *s_chn_InsInfoText[InsInfoText_SIZE] =
{
	L"上页",
	L"下页",
};



//For use at the M.E.R.C. web site. Text relating to the player's account with MERC

static const wchar_t *s_chn_MercAccountText[MercAccountText_SIZE] =
{
	// Text on the buttons on the bottom of the screen
	L"支付",
	L"主页",
	L"账号 #: ",
	L"佣兵",
	L"日数",
	L"日薪",	//5
	L"索价",
	L"合计: ",
	L"您确定要支付%ls吗?",		//the %ls is a string that contains the dollar amount ( ex. "$150" )
};



//For use at the M.E.R.C. web site. Text relating a MERC mercenary


static const wchar_t *s_chn_MercInfo[MercInfo_SIZE] =
{
	L"上一位",	//"Previous",
	L"雇佣",	//"Hire",
	L"下一位",	//"Next",
	L"附加信息",	//"Additional Info",
	L"主页",	//"Home",
	L"已雇佣",	//"Hired",
	L"日薪: ",	//"Salary:",
	L"每日",		//"Per Day",
	L"阵亡",	//"Deceased",

	L"看起来您雇佣了太多的佣兵.最多只能雇18人.",	//"Looks like you're trying to hire too many mercs. Your limit is 18.",
	L"不可雇佣",
};



// For use at the M.E.R.C. web site. Text relating to opening an account with MERC

static const wchar_t *s_chn_MercNoAccountText[MercNoAccountText_SIZE] =
{
	//Text on the buttons at the bottom of the screen
	L"开户",	//"Open Account",
	L"取消",	//"Cancel",
	L"您没有帐户, 注册一个吗?",
};



// For use at the M.E.R.C. web site. MERC Homepage

static const wchar_t *s_chn_MercHomePageText[MercHomePageText_SIZE] =
{
	//Description of various parts on the MERC page
	L"Speck T. Kline, 创办人和拥有者",
	L"开户点击这里",	//"To open an account press here",
	L"查看帐户点击这里",	//"To view account press here",
	L"查看文件点击这里",	//"To view files press here",
	// The version number on the video conferencing system that pops up when Speck is talking
	L"Speck Com v3.2",
};

// For use at MiGillicutty's Web Page
static const wchar_t *s_chn_sFuneralString[sFuneralString_SIZE] =
{
	L"McGillicutty公墓: 1983开业, 办理家庭悼念业务.",
	L"葬礼部经理兼A.I.M.前佣兵 Murray \"Pops\" McGillicutty是一名经验丰富, 业务熟练的殡仪业者.",
	L"Pops跟死亡和葬礼打了一辈子交道, 他非常熟悉该业务.",
	L"McGillicutty公墓提供各种各样的悼念服务: 从可以依靠着哭泣的肩膀到对严重变形的遗体做美容美体服务.",
	L"McGillicutty公墓是您所爱的人的安息地.",

	// Text for the various links available at the bottom of the page
	L"献花",
	L"骨灰盒",
	L"火葬服务",
	L"安排葬礼",
	L"葬礼规则",

	// The text that comes up when you click on any of the links ( except for send flowers ).
	L"很抱歉, 由于家里有人去世, 本网站的剩余部分尚未完成.一旦解决了宣读遗嘱和财产分配问题, 本网站会尽快建设好.",
	L"很抱歉, 但现在还是测试期间, 请以后再来访问.",
};

// Text for the florist Home page

static const wchar_t *s_chn_sFloristText[sFloristText_SIZE] =
{
	//Text on the button on the bottom of the page

	L"花廊",

	//Address of United Florist

	L"\"全球送货上门\"",
	L"1-555-SCENT-ME",
	L"333 NoseGay大夫, Seedy City, 美国加州 90210",
	L"http://www.scent-me.com",

	// detail of the florist page

	L"我们快速高效!",
	L"保证把鲜花在第二天送到世界上大部分地区, 但有少量限制.",
	L"保证价格是世界上最低廉的!",
	L"向我们反应比我们价格更低的送花服务广告, 我们会送您一打绝对免费的玫瑰.",
	L"自从1981年来, 我们送植物, 送动物, 送鲜花.",
	L"我们雇请了被颁发过勋章的前轰炸机飞行员, 他们能把您的鲜花空投在指定位置的十英里半径内. 总是这样 - 每次这样!",
	L"让我们满足您对鲜花的品位.",
	L"让Bruce - 我们的世界闻名的花卉设计师 - 从我们的花房里为您亲手摘取最新鲜, 最优质的花束.",
	L"还有请记住, 如果我们没有您要的花, 我们能种出来 - 很快!",
};



//Florist OrderForm

static const wchar_t *s_chn_sOrderFormText[sOrderFormText_SIZE] =
{
	//Text on the buttons

	L"后退",	//"Back",
	L"发送",	//"Send",
	L"清除",	//"Clear",
	L"花廊",	//"Gallery",

	L"花卉名称: ",			//"Name of Bouquet:",
	L"价格: ",			//"Price:",//5
	L"订单号: ",			//"Order Number:",
	L"送货日期",			//"Delivery Date",
	L"第二天",			//"next day",
	L"慢慢送去",			//"gets there when it gets there",
	L"送货目的地",			//"Delivery Location",//10
	L"额外服务",			//"Additional Services",
	L"变形的花卉($10)",		//"Crushed Bouquet($10)",
	L"黑玫瑰($20)",			//"Black Roses($20)",
	L"枯萎的花卉($10)",		//"Wilted Bouquet($10)",
	L"水果蛋糕(如果有的话)($10)",	//"Fruit Cake (if available)($10)",	//15
	L"私人密语: ",			//"Personal Sentiments:",
	L"您写的话不能多于75字.",
	L"...或者选择我们提供的",	//L"...or select from one of our",

	L"标准贺卡",			//"STANDARDIZED CARDS",
	L"传单信息",			//"Billing Information",	//20

	//The text that goes beside the area where the user can enter their name

	L"姓名: ",				//"Name:",
};




//Florist Gallery.c

static const wchar_t *s_chn_sFloristGalleryText[sFloristGalleryText_SIZE] =
{
	//text on the buttons

	L"上页",	//abbreviation for previous
	L"下页",	//abbreviation for next

	L"点击您想要订购的花卉.",
	L"请注意: 为了防止运输中的枯萎和变形, 每束花另收$10包装费.",

	//text on the button

	L"主页",	//"Home",
};

//Florist Cards

static const wchar_t *s_chn_sFloristCards[sFloristCards_SIZE] =
{
	L"请点击您想要订购的贺卡",	//"Click on your selection",
	L"后退",			//"Back",
};



// Text for Bobby Ray's Mail Order Site

static const wchar_t *s_chn_BobbyROrderFormText[BobbyROrderFormText_SIZE] =
{
	L"订单",		//"Order Form",		//Title of the page
	L"数量",			//"Qty",			// The number of items ordered
	L"重量 (%ls)",		//"Weight (%ls)",	// The weight of the item
	L"物品名称",		//"Item Name",		// The name of the item
	L"物品单价",		//"Unit Price",			// the item's weight
	L"总价",		//"Total",		//5	// The total price of all of items of the same type
	L"全部物品总价",		//"Sub-Total",		// The sub total of all the item totals added
	L"运费 (视目的地而定)",	//	"S&H (See Delivery Loc.)",	// S&H is an acronym for Shipping and Handling
	L"全部费用",		//"Grand Total",		// The grand total of all item totals + the shipping and handling
	L"送货目的地",	//"Delivery Location",
	L"运输速度",	//"Shipping Speed",		//10	// See below
	L"运费(每%ls)",	//"Cost (per %ls.)",		// The cost to ship the items
	L"连夜速递",	//"Overnight Express",		// Gets deliverd the next day
	L"2工作日",	//"2 Business Days",		// Gets delivered in 2 days
	L"标准服务",	//"Standard Service",		// Gets delivered in 3 days
	L"清除订单",		//"Clear Order",	//15// Clears the order page
	L"确认订单",	//"Accept Order",		// Accept the order
	L"后退",		//"Back",	// text on the button that returns to the previous page
	L"主页",		//"Home",	// Text on the button that returns to the home page
	L"*代表二手货",	//"* Denotes Used Items",	// Disclaimer stating that the item is used
	L"您无法支付所需费用.",	//"You can't afford to pay for this.",		//20	// A popup message that to warn of not enough money
	L"<无>",		//"<NONE>",		// Gets displayed when there is no valid city selected
	L"您确定要把该订单里订购的物品送往%ls吗?",	//"Are you sure you want to send this order to %ls?",	// A popup that asks if the city selected is the correct one
	L"包裹重量**",	//"Package Weight**",		// Displays the weight of the package
	L"** 最小重量: ",		//"** Min. Wt.",			// Disclaimer states that there is a minimum weight for the package
	L"运货",		//"Shipments",
};


// This text is used when on the various Bobby Ray Web site pages that sell items

static const wchar_t *s_chn_BobbyRText[BobbyRText_SIZE] =
{
	L"订购",				//"To Order",	// Title
	// instructions on how to order
	L"请点击该物品.如果要订购多件, 请连续点击.右击以减少要订购的数量.一旦选好了您要订购的, 请前往订单页面.",

	//Text on the buttons to go the various links

	L"上页",	//"Previous Items",		//
	L"枪械",	//"Guns", 			//3
	L"弹药",	//"Ammo",			//4
	L"护甲",	//"Armor",			//5
	L"其他",	//"Misc.",			//6	//misc is an abbreviation for miscellaneous
	L"二手货",	//"Used",			//7
	L"下页",	//"More Items",
	L"订货单",	//"ORDER FORM",
	L"主页",	//"Home",			//10

	//The following lines provide information on the items

	L"重量: ",	//"Weight:",		// Weight of all the items of the same type
	L"口径: ",	//"Cal:",			// the caliber of the gun
	L"载弹量: ",	//"Mag:",			// number of rounds of ammo the Magazine can hold
	L"射程: ",	//"Rng:",			// The range of the gun
	L"杀伤力: ",	//"Dam:",			// Damage of the weapon
	L"射速: ",	//"ROF:",			// Weapon's Rate Of Fire, acronym ROF
	L"单价: ",	//"Cost:",			// Cost of the item
	L"库存: ",	//"In stock:",			// The number of items still in the store's inventory
	L"购买量: ",	//"Qty on Order:",		// The number of items on order
	L"已损坏",	//"Damaged",			// If the item is damaged
	L"小计: ",	//"SubTotal:",			// The total cost of all items on order
	L"* %％ 可用",	//"* %% Functional",		// if the item is damaged, displays the percent function of the item

	//Popup that tells the player that they can only order 10 items at a time

	L"唉! 我们这里的在线订单一次只接受10件物品的订购.如果您想要订购更多商品 (我们希望如此), 请接受我们的歉意, 再开一份订单.",

	// A popup that tells the user that they are trying to order more items then the store has in stock

	L"抱歉, 我们只有这么多, 请稍后再来订购.",

	//A popup that tells the user that the store is temporarily sold out

	L"抱歉, 现在缺货.",

};


// The following line is used on the Ammunition page.  It is used for help text
// to display how many items the player's merc has that can use this type of
// ammo.
static const wchar_t s_chn_str_bobbyr_guns_num_guns_that_use_ammo[] = 
	L"您的队伍有%d件武器使用此类型弹药";


// Text for Bobby Ray's Home Page

static const wchar_t *s_chn_BobbyRaysFrontText[BobbyRaysFrontText_SIZE] =
{
	//Details on the web site

	L"这里有最新最火爆的军用物资",	//"This is the place to be for the newest and hottest in weaponry and military supplies",
	L"我们提供完美的方案满足您所有破坏欲望!",	//"We can find the perfect solution for all your explosives needs",
	L"二手货",	//"Used and refitted items",

	//Text for the various links to the sub pages

	L"杂货",	//"Miscellaneous",
	L"枪械",	//"GUNS",
	L"弹药",	//"AMMUNITION",		//5
	L"护甲",	//"ARMOR",

	//Details on the web site

	L"独此一家, 别无分店.",	//"If we don't sell it, you can't get it!",
	L"建设中",	//"Under Construction",
};



// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug shot page

static const wchar_t *s_chn_AimSortText[AimSortText_SIZE] =
{
	L"A.I.M成员",	//"A.I.M. Members",				// Title
	// Title for the way to sort
	L"排序: ",	//"Sort By:",

	//Text of the links to other AIM pages

	L"查看佣兵的肖像索引",	//"View the mercenary mug shot index",
	L"查看单独的佣兵档案",	//"Review the individual mercenary's file",
	L"浏览A.I.M前成员",	//"Browse the A.I.M. Alumni Gallery",
};


// text to display how the entries will be sorted
static const wchar_t s_chn_str_aim_sort_price[]        = 
	L"费用";
static const wchar_t s_chn_str_aim_sort_experience[]   = 
	L"级别";
static const wchar_t s_chn_str_aim_sort_marksmanship[] = 
	L"枪法";
static const wchar_t s_chn_str_aim_sort_medical[]      = 
	L"医疗";
static const wchar_t s_chn_str_aim_sort_explosives[]   = 
	L"爆破";
static const wchar_t s_chn_str_aim_sort_mechanical[]   = 
	L"机械";
static const wchar_t s_chn_str_aim_sort_ascending[]    = 
	L"升序";
static const wchar_t s_chn_str_aim_sort_descending[]   = 
	L"降序";


//Aim Policies.c
//The page in which the AIM policies and regulations are displayed

static const wchar_t *s_chn_AimPolicyText[AimPolicyText_SIZE] =
{
	// The text on the buttons at the bottom of the page

	L"上页",	//"Previous Page",
	L"AIM主页",	//"AIM HomePage",
	L"规则索引",	//"Policy Index",
	L"下页",	//"Next Page",
	L"不同意",	//Disagree",
	L"同意",	//"Agree",
};



//Aim Member.c
//The page in which the players hires AIM mercenaries

// Instructions to the user to either start video conferencing with the merc, or to go the mug shot index

static const wchar_t *s_chn_AimMemberText[AimMemberText_SIZE] =
{
	L"鼠标左击",	//"Left Click",
	L"联系佣兵.",	//"to Contact Merc.",
	L"鼠标右击",	//"Right Click",
	L"回到肖像索引.",	//"for Mug Shot Index.",
};

//Aim Member.c
//The page in which the players hires AIM mercenaries

static const wchar_t *s_chn_CharacterInfo[CharacterInfo_SIZE] =
{
	// the contract expenses' area

	L"费用",	//"Fee",
	L"合同",	//"Contract",
	L"一日",	//"one day",
	L"一周",	//"one week",
	L"两周",	//"two weeks",

	// text for the buttons that either go to the previous merc, 
	// start talking to the merc, or go to the next merc

	L"上一位",	//"Previous",
	L"联系",	//"Contact",
	L"下一位",	//"Next",

	L"附加信息",	//"Additional Info",				// Title for the additional info for the merc's bio
	L"现役成员",	//"Active Members",		//20		// Title of the page
	L"可选装备: ",				// Displays the optional gear cost
	L"所需医疗保证金",			// If the merc required a medical deposit, this is displayed
};


//Aim Member.c
//The page in which the player's hires AIM mercenaries

//The following text is used with the video conference popup

static const wchar_t *s_chn_VideoConfercingText[VideoConfercingText_SIZE] =
{
	L"合同总价: ",	//"Contract Charge:",				//Title beside the cost of hiring the merc

	//Text on the buttons to select the length of time the merc can be hired

	L"一日",	//"One Day",
	L"一周",	//"One Week",
	L"两周",	//"Two Weeks",

	//Text on the buttons to determine if you want the merc to come with the equipment 

	L"不买装备",	//"No Equipment",
	L"购买装备",	//"Buy Equipment",

	// Text on the Buttons

	L"转帐",	//"TRANSFER FUNDS",			// to actually hire the merc
	L"取消",	//"CANCEL",				// go back to the previous menu
	L"雇佣",	//"HIRE",				// go to menu in which you can hire the merc
	L"挂断",	//"HANG UP",				// stops talking with the merc
	L"完成",	//"OK",
	L"留言",	//"LEAVE MESSAGE",			// if the merc is not there, you can leave a message

	//Text on the top of the video conference popup

	L"视频通讯: ",	//"Video Conferencing with",
	L"建立连接...",	//"Connecting. . .",

	L"包括医保",	//"with medical"			//Displays if you are hiring the merc with the medical deposit
};



//Aim Member.c
//The page in which the player hires AIM mercenaries

// The text that pops up when you select the TRANSFER FUNDS button

static const wchar_t *s_chn_AimPopUpText[AimPopUpText_SIZE] =
{
	L"电子转帐成功",	//"ELECTRONIC FUNDS TRANSFER SUCCESSFUL",	// You hired the merc
	L"无法处理转帐",	//"UNABLE TO PROCESS TRANSFER",		// Player doesn't have enough money, message 1
	L"资金不足",	//"INSUFFICIENT FUNDS",				// Player doesn't have enough money, message 2

	// if the merc is not available, one of the following is displayed over the merc's face

	L"执行任务中",	//"On Assignment"
	L"请留言",	//"Please Leave Message",
	L"阵亡",	//"Deceased",

	//If you try to hire more mercs than game can support

	L"您的队伍现有18人, 已经满员了.",	//"You have a full team of 18 mercs already.",

	L"预录消息",	//"Pre-recorded message",
	L"留言已记录",	//"Message recorded",
};


//AIM Link.c

static const wchar_t s_chn_AimLinkText[] = 
	L"A.I.M 链接"; // The title of the AIM links page



//Aim History

// This page displays the history of AIM

static const wchar_t *s_chn_AimHistoryText[AimHistoryText_SIZE] =
{
	L"A.I.M 历史",	//"A.I.M. History",					//Title

	// Text on the buttons at the bottom of the page

	L"上页",	//"Previous Page",
	L"主页",	//"Home",
	L"A.I.M 前成员",	//"A.I.M. Alumni",
	L"下页",	//"Next Page",
};


//Aim Mug Shot Index

//The page in which all the AIM members' portraits are displayed in the order selected by the AIM sort page.

static const wchar_t *s_chn_AimFiText[AimFiText_SIZE] =
{
	// displays the way in which the mercs were sorted

	L"费用",	//"Price",
	L"级别",	//"Experience",
	L"枪法",	//"Marksmanship",
	L"医疗",	//"Medical",
	L"爆破",	//"Explosives",
	L"机械",	//"Mechanical",

	// The title of the page, the above text gets added at the end of this text

	L"根据%ls升序排列的A.I.M成员",	//"A.I.M. Members Sorted Ascending By %ls",
	L"根据%ls降序排列的A.I.M成员",	//"A.I.M. Members Sorted Descending By %ls",

	// Instructions to the players on what to do

	L"鼠标左击",	//"Left Click",
	L"选择佣兵.",	//"To Select Merc",			//10
	L"鼠标右击",	//"Right Click",
	L"回到排序选项.",	//"For Sorting Options",

	// Gets displayed on top of the merc's portrait if they are...

	L"阵亡",	//"Deceased",						//14
	L"任务中",	//"On Assign",
};



//AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer with AIM

static const wchar_t *s_chn_AimAlumniText[AimAlumniText_SIZE] =
{
	// Text of the buttons

	L"页 1",
	L"页 2",
	L"页 3",

	L"A.I.M. 前成员",	// Title of the page

	L"好"			// Stops displaying information on selected merc
};






//AIM Home Page

static const wchar_t *s_chn_AimScreenText[AimScreenText_SIZE] =
{
	// AIM disclaimers

	L"A.I.M. 和 A.I.M. 标志已在世界大多数国家注册.",
	L"版权所有, 仿冒必究.",
	L"版权 1998-1999 A.I.M., Ltd. 所有权利保留.",

	//Text for an advertisement that gets displayed on the AIM page

	L"联合花卉服务公司",
	L"\"我们将花空运到任何地方\"",				//10
	L"把活干好",
	L"... 第一次",
	L"枪械和杂货, 只此一家, 别无分店.",
};


//Aim Home Page

static const wchar_t *s_chn_AimBottomMenuText[AimBottomMenuText_SIZE] =
{
	//Text for the links at the bottom of all AIM pages
	L"主页",	//"Home",
	L"成员",	//"Members",
	L"前成员",	//"Alumni",
	L"规则",	//"Policies",
	L"历史",	//"History",
	L"链接",	//"Links",
};



//ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.

static const wchar_t *s_chn_SKI_Text[SKI_SIZE ] =
{
	L"库存商品",	//"MERCHANDISE IN STOCK",	//Header for the merchandise available
	L"页面",			//"PAGE",	//The current store inventory page being displayed
	L"总价格",			//"TOTAL COST",		//The total cost of the the items in the Dealer inventory area
	L"总价值",			//"TOTAL VALUE",	//The total value of items player wishes to sell
	L"估价",			//"EVALUATE",		//Button text for dealer to evaluate items the player wants to sell
	L"确认交易",			//"TRANSACTION",	//Button text which completes the deal. Makes the transaction.
	L"完成",			//"DONE",	//Text for the button which will leave the shopkeeper interface.
	L"修理费",			//"REPAIR COST",	//The amount the dealer will charge to repair the merc's goods
	L"1小时",			//"1 HOUR",// SINGULAR! The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"%d小时",		//"%d HOURS",// PLURAL!   The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"已经修好",		//"REPAIRED",// Text appearing over an item that has just been repaired by a NPC repairman dealer
	L"您没有空余的位置来放东西了.",	//"There is not enough room in your offer area.",//Message box that tells the user there is no more room to put there stuff
	L"%d分钟",		//"%d MINUTES",	// The text underneath the inventory slot when an item is given to the dealer to be repaired
	L"把物品放在地上.",	//"Drop Item To Ground.",
};


static const wchar_t *s_chn_SkiMessageBoxText[SkiMessageBoxText_SIZE] =
{
	L"您要从主帐户中提取%ls来支付吗?",
	L"资金不足.您缺少%ls.",
	L"您要从主帐户中提取%ls来支付吗?",
	L"请求商人开始交易",
	L"请求商人修理选定物品",
	L"结束对话",
	L"当前余额",
};


//OptionScreen.c

static const wchar_t *s_chn_zOptionsText[zOptionsText_SIZE] =
{
	//button Text
	L"保存游戏",	//"Save Game",
	L"载入游戏",	//"Load Game",
	L"退出",	//"Quit",
	L"完成",	//"Done",

	//Text above the slider bars
	L"效果音",	//"Effects",
	L"语音",	//"Speech",
	L"音乐",	//"Music",

	//Confirmation pop when the user selects..
	L"退出并回到游戏主菜单?",

	L"您必须选择 '语音' 和 '对话显示' 中至少一项.",
};


//SaveLoadScreen
static const wchar_t *s_chn_zSaveLoadText[zSaveLoadText_SIZE] =
{
	L"保存游戏",
	L"载入游戏",
	L"取消",
	L"保存选中",
	L"载入选中",

	L"保存游戏成功",
	L"保存游戏错误!",
	L"载入游戏成功",
	L"载入游戏错误!",
 
	L"存档的游戏版本不同于当前的游戏版本, 读取它游戏很可能可以正常进行.继续吗?",
	L"存档可能已经无效, 您要删除它们吗?",

	L"试图载入老版本的存档, 您要自动更新并载入存档吗?",

	L"试图载入老版本的存档, 您要自动更新并载入存档吗?",

	L"您确认要覆盖#%d位置的存档吗?",


	//The first %d is a number that contains the amount of free space on the users hard drive,
	//the second is the recommended amount of free space.
	L"您的硬盘空间不够, 您现在只有 %dM 可用空间, JA2需要至少 %dM 可用空间.",

	L"保存中...",			//When saving a game, a message box with this string appears on the screen

	L"正常的武器",
	L"大量的武器",
	L"真实风格",
	L"科幻风格",

	L"难度",
};



//MapScreen
static const wchar_t *s_chn_zMarksMapScreenText[zMarksMapScreenText_SIZE] =
{
	L"地图层次",	//"Map Level",
	L"您没有民兵.要拥有民兵, 您得先训练镇民.",	//"You have no militia.  You need to train town residents in order to have a town militia.",
	L"每日收入",	//"Daily Income",
	L"佣兵有人寿保险",	//"Merc has life insurance",
	L"%ls不疲劳.",	//"%ls isn't tired.",
	L"%ls行军中, 不能睡觉",	//"%ls is on the move and can't sleep",
	L"%ls太累了, 等会儿再试.",	//"%ls is too tired, try a little later.",
	L"%ls正在开车.",	//"%ls is driving.",
	L"有人在睡觉时, 整个队伍不能行动.",	//"Squad can't move with a sleeping merc on it.",

	// stuff for contracts
	L"您能支付合同所需费用, 但是您的钱不够给该佣兵购买人寿保险.",
	L"要给%ls花费保险金%ls 以延长保险合同%d天.您要付费吗?",
	L"区域存货",	//"Sector Inventory",
	L"佣兵有医疗保证金.",	//"Merc has a medical deposit.",

	// other items
	L"医生", 	//"Medics", // people acting a field medics and bandaging wounded mercs
	L"病人", 	//"Patients", // people who are being bandaged by a medic
	L"完成", 	//"Done", // Continue on with the game after autobandage is complete
	L"停止", 	//"Stop", // Stop autobandaging of patients by medics now
	L"%ls没工具箱.",	//"%ls doesn't have a repair kit.",
	L"%ls没医药箱.",	//"%ls doesn't have a medical kit.",
	L"现在没有足够的人愿意加入民兵.",
	L"%ls的民兵已经训练满了.",	//"%ls is full of militia.",
	L"佣兵有一份限时的合同.",	//"Merc has a finite contract.",
	L"佣兵的合同没投保", //"Merc's contract is not insured",
};


static const wchar_t s_chn_pLandMarkInSectorString[] = 
	L"%d小队在%ls地区发现有人";

// confirm the player wants to pay X dollars to build a militia force in town
static const wchar_t *s_chn_pMilitiaConfirmStrings[pMilitiaConfirmStrings_SIZE] =
{
	L"训练一队民兵要花费$",
	L"同意支付吗?",
	L"您付不起.",
	L"继续在%ls (%ls %d)训练民兵吗?",
	L"花费$",
	L"( Y/N )",   // abbreviated yes/no
	L"在%d地区训练民兵将花费$%d.%ls",
	L"您无法支付$%d以供在这里训练民兵.",
	L"%ls的忠诚度必须达到%d以上方可训练民兵.",
	L"您不能在%ls训练民兵了.",
};


//Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom of the single merc panel
static const wchar_t *s_chn_gzMoneyWithdrawMessageText[gzMoneyWithdrawMessageText_SIZE] =
{
	L"您每次最多能提取$20,000.",
	L"您确认要把%ls存入您的帐户吗?",
};

static const wchar_t s_chn_gzCopyrightText[] = 
	L"版权所有 (C) 1999 Sir-tech 加拿大 Ltd.  所有权利保留.";

//option Text
static const wchar_t *s_chn_zOptionsToggleText[zOptionsToggleText_SIZE] =
{
	L"语音",		//"Speech",
	L"确认静默",		//"Mute Confirmations",
	L"显示对话文字",	//"Subtitles",
	L"显示对话文字时暂停",		//"Pause Text Dialogue",
	L"烟火效果",		//"Animate Smoke",
	L"血腥效果",		//"Blood n Gore",
	L"不移动鼠标",		//"Never Move My Mouse!",
	L"旧的选择方式",	//"Old Selection Method",
	L"显示移动路径",	//"Show Movement Path",
	L"显示未击中",		//"Show Misses",
	L"实时确认",		//"Real Time Confirmation",
	L"显示睡觉/醒来时的提示",	//"Display sleep/wake notifications",
	L"使用公制系统",		//"Use Metric System",
	L"佣兵移动时高亮显示",	//"Merc Lights during Movement",
	L"锁定佣兵",	//"Snap Cursor to Mercs",
	L"锁定门",		//"Snap Cursor to Doors",
	L"物品闪亮",	//"Make Items Glow",
	L"显示树冠",	//"Show Tree Tops",
	L"显示轮廓",		//"Show Wireframes",
	L"显示3D光标",	//"Show 3D Cursor",
};

//This is the help text associated with the above toggles.
static const wchar_t *s_chn_zOptionsScreenHelpText[zOptionsToggleText_SIZE] =
{
	//speech
	L"如果您想听到人物对话, 打开此选项.",

	//Mute Confirmation
	L"打开或关闭人物的口头确认.",

	//Subtitles
	L"是否显示对话的文字.",

	//Key to advance speech
	L"如果 '显示对话文字' 已打开, 此选项会让您有足够的时间来阅读NPC的对话.",

	//Toggle smoke animation
	L"如果烟火效果使得游戏变慢, 关闭该选项.",

	//Blood n Gore
	L"如果鲜血使您觉得恶心, 关闭该选项.",

	//Never move my mouse
	L"关闭该选项会使您的光标自动移到弹出的确认对话框上.",

	//Old selection method
	L"打开此选项, 使用铁血联盟1代的佣兵选择方式.",

	//Show movement path
	L"打开此选项, 会实时显示移动路径(关闭此选项.如果您想要显示路径的话, 使用SHIFT键).",

	//show misses
	L"打开此选项时, 会显示未击中目标的子弹落点.",

	//Real Time Confirmation
	L"当打开时, 进入实时模式会有一个确认对话框.",

	//Sleep/Wake notification
	L"当打开时, 被分配任务的佣兵睡觉和醒来时会提示您.",

	//Use the metric system
	L"当打开时, 使用公制系统, 否则使用英制系统.",

	//Merc Lighted movement
	L"当打开时, 佣兵移动时会照亮地表.关闭该选项会使游戏的显示速度变快.",

	//Smart cursor
	L"当打开时, 光标移动到佣兵身上时会高亮显示佣兵.",

	//snap cursor to the door
	L"当打开时, 光标靠近门时会自动定位到门上.",

	//glow items
	L"当打开时, 物品会不断的闪烁.(|I)",

	//toggle tree tops
	L"当打开时, 显示树冠.(|T)",

	//toggle wireframe
	L"显示未探明的墙的轮廓.(|W)",

	L"打开时, 移动时的光标为3D式样.(|H)",

};


static const wchar_t *s_chn_gzGIOScreenText[gzGIOScreenText_SIZE] =
{
	L"游戏初始设置",
	L"游戏风格",
	L"真实",
	L"科幻",
	L"武器数量", // changed by SANDRO
	L"大量武器",
	L"少量武器", // changed by SANDRO
	L"难度",
	L"新手",
	L"老手",
	L"专家",
	L"确定",	// TODO.Translate
	L"取消",
	L"额外难度",
	L"可随时存盘",
	L"铁人模式",
	L"Dead is Dead"
};

static const wchar_t *s_chn_pDeliveryLocationStrings[pDeliveryLocationStrings_SIZE] =
{
	L"奥斯汀",	//"Austin",			//Austin, Texas, USA
	L"巴格达",	//"Baghdad",			//Baghdad, Iraq (Suddam Hussein's home)
	L"Drassen",			//The main place in JA2 that you can receive items.  The other towns are dummy names...
	L"庙街",	//"Hong Kong",		//Hong Kong, China
	L"贝鲁特",	//"Beirut",			//Beirut, Lebanon	(Middle East)
	L"伦敦",	//"London",			//London, England
	L"洛杉矶",	//"Los Angeles",	//Los Angeles, California, USA (SW corner of USA)
	L"Meduna",			//Meduna -- the other airport in JA2 that you can receive items.
	L"Metavira",		//The island of Metavira was the fictional location used by JA1
	L"迈阿密",	//"Miami",				//Miami, Florida, USA (SE corner of USA)
	L"莫斯科",	//"Moscow",			//Moscow, USSR
	L"纽约",	//"New York",		//New York, New York, USA
	L"渥太华",	//"Ottawa",			//Ottawa, Ontario, Canada -- where JA2 was made!
	L"巴黎",	//"Paris",				//Paris, France
	L"的黎波里",	//"Tripoli",			//Tripoli, Libya (eastern Mediterranean)
	L"东京",	//"Tokyo",				//Tokyo, Japan
	L"温哥华",		//Vancouver, British Columbia, Canada (west coast near US border)
};

// This string is used in the IMP character generation.  It is possible to
// select 0 ability in a skill meaning you can't use it.  This text is
// confirmation to the player.
static const wchar_t s_chn_pSkillAtZeroWarning[]    = 
	L"您确定吗? '0' 意味着您不能拥有这项技能.";
static const wchar_t s_chn_pIMPBeginScreenStrings[] = 
	L"( 最多8个字符 )";
static const wchar_t s_chn_pIMPFinishButtonText[]   = 
	L"分析中";
static const wchar_t s_chn_pIMPFinishStrings[]      = 
	L"谢谢您, %ls"; //%ls is the name of the merc
static const wchar_t s_chn_pIMPVoicesStrings[]      = 
	L"嗓音"; // the strings for imp voices screen

// title for program
static const wchar_t s_chn_pPersTitleText[] = 
	L"人事管理";

// paused game strings
static const wchar_t *s_chn_pPausedGameText[pPausedGameText_SIZE] =
{
	L"游戏暂停",	//"Game Paused",
	L"继续游戏 (|P|a|u|s|e)",	//"Resume Game (|P|a|u|s|e)",
	L"暂停游戏 (|P|a|u|s|e)",
};


static const wchar_t *s_chn_pMessageStrings[pMessageStrings_SIZE] =
{
	L"退出游戏",	//"Exit Game?",
	L"确定",	//"OK",
	L"是",	//"YES",
	L"否",	//"NO",
	L"取消",	//"CANCEL",
	L"再次雇佣",	//"REHIRE",
	L"撒谎",	//"LIE",		//
	L"无描述",	//"No description", //Save slots that don't have a description.
	L"游戏已保存.",	//"Game Saved.",
	L"日",	//"Day",
	L"位佣兵",	//"Mercs",
	L"空",	//"Empty Slot", //An empty save game slot
	L"rpm",					//Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
	L"分钟",	//"min",					//Abbreviation for minute.
	L"米",	//"m",						//One character abbreviation for meter (metric distance measurement unit).
	L"发",  //L"rnds",				//Abbreviation for rounds (# of bullets)
	L"公斤",	//"kg",					//Abbreviation for kilogram (metric weight measurement unit)
	L"磅",	//"lb",					//Abbreviation for pounds (Imperial weight measurement unit)
	L"主页",	//"Home",				//Home as in homepage on the internet.
	L"USD", //L"USD",					//Abbreviation to US dollars
	L"n/a",					//Lowercase acronym for not applicable.
	L"与此同时",	//"Meanwhile",	//Meanwhile
	L"%ls已到达%ls%ls区域",	//"%ls has arrived in sector %ls%ls", //Name/Squad has arrived in sector A9.  Order must not change without notifying																		//SirTech

	L"版本", //L"Version",
	L"无快速存档",	//"Empty Quick Save Slot",
	L"该位置用来放Quick Save (快速存档). 请在战术屏幕或者地图屏幕按Alt+S进行快速存档.",
	L"打开的",	//"Opened",
	L"关闭的",	//"Closed",
	L"磁盘空间不足. 只有%lsMB可用空间, 《铁血联盟2》需要%lsMB.",
	L"%ls抓住了%ls.",	//"%ls has caught %ls.",	//'Merc name' has caught 'item' -- let SirTech know if name comes after item.
	L"%ls打了针剂.", 	//"%ls has taken the drug.", //'Merc name' has taken the drug
	L"%ls无医疗技能",	//"%ls has no medical skill",//'Merc name' has no medical skill.

	//CDRom errors (such as ejecting CD while attempting to read the CD)
	L"The integrity of the game has been compromised.",
	L"ERROR: Ejected CD-ROM",

	//When firing heavier weapons in close quarters, you may not have enough room to do so.
	L"没有空间施展您的武器.",	//"There is no room to fire from here.",

	//Can't change stance due to objects in the way...
	L"现在无法改变姿势.",	//"Cannot change stance at this time.",

	//Simple text indications that appear in the game, when the merc can do one of these things.
	L"放下",	//"Drop",
	L"投掷",	//"Throw",
	L"交给",	//"Pass",

	L"把%ls交给了%ls.", //"%ls passed to %ls.", //"Item" passed to "merc".  Please try to keep the item %ls before the merc %ls, otherwise,
											 //must notify SirTech.
	L"没有足够空位以把%ls交给%ls",	//"No room to pass %ls to %ls.", //pass "item" to "merc".  Same instructions as above.

	//A list of attachments appear after the items.  Ex:  Kevlar vest ( Ceramic Plate 'Attached )'
	L"附加 )",	//" Attached )",

	//Cheat modes
	L"开启作弊等级1",	//"Cheat level ONE reached",
	L"开启作弊等级2",	//"Cheat level TWO reached",

	//Toggling various stealth modes
	L"小队进入潜行模式.",	//"Squad on stealth mode.",
	L"小队退出潜行模式.",	//"Squad off stealth mode.",
	L"%ls进入潜行模式.",	//"%ls on stealth mode.",
	L"%ls退出潜行模式.",	//"%ls off stealth mode.",

	//Wireframes are shown through buildings to reveal doors and windows that can't otherwise be seen in 
	//an isometric engine.  You can toggle this mode freely in the game.
	L"打开显示轮廓",	//"Extra Wireframes On",
	L"关闭显示轮廓",	//"Extra Wireframes Off",

	//These are used in the cheat modes for changing levels in the game.  Going from a basement level to
	//an upper level, etc.
	L"无法从这层上去...",	//"Can't go up from this level...",
	L"没有更低的层了...",	//"There are no lower levels...",
	L"进入地下室%d层...",	//"Entering basement level %d...",
	L"离开地下室...",			//"Leaving basement...",

	L"的",	//"'s",		// used in the shop keeper inteface to mark the ownership of the item eg Red's gun
	L"跟随模式关.",	//"Follow mode OFF.",
	L"跟随模式开.",	//"Follow mode ON.",
	L"3D光标关.",	//"3D Cursor OFF.",
	L"3D光标开.",	//"3D Cursor ON.",
	L"%d小队激活.",	//"Squad %d active.",
	L"您无法支付%ls的%ls日薪",	//"You cannot afford to pay for %ls's daily salary of %ls",	//first %ls is the mercs name, the seconds is a string containing the salary
	L"跳过",			//"Skip",
	L"%ls不能独自离开",	//"%ls cannot leave alone.",
	L"一个文件名为SaveGame99.sav的存档被创建了.如果需要的话, 将其更名为SaveGame01 - SaveGame10, 然后您就能载入此存档了.",	//"A save has been created called, SaveGame99.sav.  If needed, rename it to SaveGame01 - SaveGame10 and then you will have access to it in the Load screen.",
	L"%ls喝了点%ls",	//"%ls drank some %ls",
	L"一个包裹到达Drassen.",	//"A package has arrived in Drassen.",
 	L"%ls将于%d日%ls到达指定的着陆点(区域%ls).",	//"%ls should arrive at the designated drop-off point (sector %ls) on day %d, at approximately %ls.",		//first %ls is mercs name, next is the sector location and name where they will be arriving in, lastely is the day an the time of arrival
	L"日志已经更新.",	//"History log updated.",
};


static const wchar_t *s_chn_ItemPickupHelpPopup[ItemPickupHelpPopup_SIZE] =
{
	L"确认",		//"OK",
	L"向上滚动",		//"Scroll Up",
	L"选择全部",		//"Select All",
	L"向下滚动",		//"Scroll Down",
	L"取消",		//"Cancel",
};

static const wchar_t *s_chn_pDoctorWarningString[pDoctorWarningString_SIZE] =
{
	L"%ls不够近, 不能被治疗.",
	L"您的医生不能包扎完每个人.",
};

static const wchar_t *s_chn_pMilitiaButtonsHelpText[pMilitiaButtonsHelpText_SIZE] =
{
	L"调离(右击)/安置(左击)新兵",
	L"调离(右击)/安置(左击)老兵",
	L"调离(右击)/安置(左击)精兵",
	L"在各个区域平均分布民兵",
};

// to inform the player to hire some mercs to get things going
static const wchar_t s_chn_pMapScreenJustStartedHelpText[] = 
	L"去AIM雇几位佣兵 (*提示* 在笔记本电脑里)";

static const wchar_t s_chn_pAntiHackerString[] = 
	L"错误. 丢失或损坏文件, 游戏将退出.";


static const wchar_t *s_chn_gzLaptopHelpText[gzLaptopHelpText_SIZE] =
{
	//Buttons:
	L"查看邮件",
	L"浏览网页",
	L"查看文件和邮件的附件",
	L"阅读事件日志",
	L"查看队伍信息",
	L"查看财务简报和记录",
	L"关闭笔记本电脑",

	//Bottom task bar icons (if they exist):
	L"您有新的邮件",
	L"您有新的文件",

	//Bookmarks:
	L"国际佣兵联盟",
	L"Bobby Ray网上武器店",
	L"佣兵心理剖析研究所",
	L"廉价新兵中心",
	L"McGillicutty公墓",
	L"联合花卉服务公司",
	L"A.I.M指定保险代理人",
};


static const wchar_t s_chn_gzHelpScreenText[] = 
	L"退出帮助界面";

static const wchar_t *s_chn_gzNonPersistantPBIText[gzNonPersistantPBIText_SIZE] =
{
	L"战斗正在进行中, 您只能在战术屏幕进行撤退.",
	L"进入该区域, 继续战斗.(|E)",
	L"自动解决这次战斗.(|A)",
	L"当您进攻时, 不能自动解决战斗.",
	L"当您遭遇伏兵时, 不能自动解决战斗.",
	L"当在矿井里和异形作战时, 不能自动解决战斗.",
	L"还有敌对的平民时, 不能自动解决战斗.",
	L"有血猫时, 不能自动解决战斗.",
	L"战斗进行中",
	L"您不能在这时撤退.",
};

static const wchar_t *s_chn_gzMiscString[gzMiscString_SIZE] =
{
	L"在没有您的佣兵的支援下, 民兵继续战斗...",
	L"现在车辆不需要加油.",	//"The vehicle does not need anymore fuel right now.",
	L"油箱装了%d％的油.",	//"The fuel tank is %d%% full.",
	L"Deidranna女王的军队重新完全占领了%ls.",
	L"您丢失了加油点.",	//"You have lost a refueling site.",
};

static const wchar_t s_chn_gzIntroScreen[] = L"找不到介绍视频";

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
static const wchar_t *s_chn_pNewNoiseStr[pNewNoiseStr_SIZE] =
{
	L"%ls听到%ls声音来自%ls.",
	L"%ls听到%ls移动声来自%ls.",
	L"%ls听到%ls吱吱声来自%ls.",
	L"%ls听到%ls溅水声来自%ls.",
	L"%ls听到%ls撞击声来自%ls.",
	L"%ls听到%ls爆炸声发向%ls.",
	L"%ls听到%ls尖叫声发向%ls.",
	L"%ls听到%ls撞击声发向%ls.",
	L"%ls听到%ls撞击声发向%ls.",
	L"%ls听到%ls粉碎声来自%ls.",
	L"%ls听到%ls破碎声来自%ls.",
};

static const wchar_t *s_chn_wMapScreenSortButtonHelpText[wMapScreenSortButtonHelpText_SIZE] =
{
	L"按姓名排序 (|F|1)",
	L"按任务排序 (|F|2)",
	L"按睡眠状态排序 (|F|3)",
	L"按地点排序 (|F|4)",
	L"按目的地排序 (|F|5)",
	L"按预计离队时间排序 (|F|6)",
};



static const wchar_t *s_chn_BrokenLinkText[BrokenLinkText_SIZE] =
{
	L"404错误",	//"Error 404",
	L"网站未找到",	//"Site not found.",
};


static const wchar_t *s_chn_gzBobbyRShipmentText[gzBobbyRShipmentText_SIZE] =
{
	L"近期货物",	//"Recent Shipments",
	L"订单 #",		//"Order #",
	L"物品数量",	//"Number Of Items",
	L"订购于",		//"Ordered On",
};


static const wchar_t *s_chn_gzCreditNames[gzCreditNames_SIZE]=
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


static const wchar_t *s_chn_gzCreditNameTitle[gzCreditNameTitle_SIZE]=
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

static const wchar_t *s_chn_gzCreditNameFunny[gzCreditNameFunny_SIZE]=
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

static const wchar_t *s_chn_sRepairsDoneString[sRepairsDoneString_SIZE] =
{
	L"%ls 修复了自己的物品.",
	L"%ls 修复了所有人的枪和护甲.",
	L"%ls 修复了所有人装备的物品.",
	L"%ls 修复了所有人携带的物品",//L"%ls finished repairing everyone's large carried items",
};

static const wchar_t *s_chn_zGioDifConfirmText[zGioDifConfirmText_SIZE]=
{
	L"您选择了 '新手' 模式. 此设置是为那些刚玩铁血联盟的玩家准备的, 他们刚接触策略游戏, 或者他们希望快点结束战斗. 您的选择会在整个游戏中生效, 所以请作出明智的选择.您真的要玩 '新手' 模式吗?",
	L"您选择了 '老手' 模式. 此设置是为那些已经熟悉铁血联盟或类似游戏的玩家准备的. 您的选择会在整个游戏中生效, 所以请作出明智的选择. 您真的要玩 '老手' 模式吗?",
	L"您选择了 '专家' 模式. 我们警告您, 如果您被装在尸袋里运回来, 不要来向我们抱怨. 您的选择会在整个游戏中生效, 所以请作出明智的选择. 您真的要玩 '专家' 模式吗?",
};

static const wchar_t *s_chn_gzLateLocalizedString[gzLateLocalizedString_SIZE] =
{
	//1-5
	L"由于无人在用遥控器, 机器人无法离开本区域.",

	//This message comes up if you have pending bombs waiting to explode in tactical.
	L"您现在无法压缩时间.请等待炸弹爆炸!",

	//'Name' refuses to move.
	L"%ls拒绝移动.",

	//%ls a merc name
	L"%ls精力不足, 无法改变姿势.",	//"%ls does not have enough energy to change stance.",

	//A message that pops up when a vehicle runs out of gas.
	L"%ls汽油耗尽, 在%c%d抛锚了.",	//"The %ls has run out of gas and is now stranded in %c%d.",

	//6-10

	// the following two strings are combined with the pNewNoise[] strings above to report noises
	// heard above or below the merc
	L"上方",
	L"下方",

	//The following strings are used in autoresolve for autobandaging related feedback.
	L"佣兵中没人有医疗技能.",
	L"没有足够的医疗物品进行包扎.",
	L"没有足够的医疗物品给所有人进行包扎.",
	L"佣兵中没人需要包扎.",	//"None of your mercs need bandaging.",
	L"自动包扎佣兵.",	//"Bandages mercs automatically.",
	L"全部佣兵已被包扎完毕.",	//"All your mercs are bandaged.",

	//14
	L"Arulco",

	L"(屋顶)",

	L"生命: %d/%d",

	//In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
	//"vs." is the abbreviation of versus.
	L"%d vs. %d",

	L"%ls满了.",
 
	L"%ls现在不用包扎, 他 (她) 需要认真的治疗和休息.",

	//20
	//Happens when you get shot in the legs, and you fall down.
	L"%ls被击中腿部, 并且倒下了!",
	//Name can't speak right now.
	L"%ls现在不能说话.",

	//22-24 plural versions
	L"%d名新兵被提升为精兵.",
	L"%d名新兵被提升为老兵.",
	L"%d名老兵被提升为精兵.",

	//25
	L"开关",

	//26
	//Name has gone psycho -- when the game forces the player into burstmode (certain unstable characters)
	L"%疯狂了!",

	//27-28
	//Messages why a player can't time compress.
	L"现在压缩时间不安全, 因为您有佣兵在区域%ls.",
	L"现在压缩时间不安全, 因为您有佣兵在被异形感染的矿井.",

	//29-31 singular versions
	L"1名新兵晋升为精兵.",
	L"1名新兵晋升为老兵.",
	L"1名老兵晋升为精兵.",

	//32-34
	L"%ls无语.",	//"%ls doesn't say anything.",
	L"回到地面?",
	L"(%d小队)",	//"(Squad %d)",

	//35
	//Ex: "Red has repaired Scope's MP5K".  Careful to maintain the proper order (Red before Scope, Scope before MP5K)
	L"%ls修复了%ls的%ls.",

	//36
	L"血猫",

	//37-38 "Name trips and falls"
	L"%ls踩到陷阱, 跌倒了.",
	L"此物品不能从这里捡起.",

	//39
	L"您现有的佣兵中没人能进行战斗.民兵们将独自和异形作战.",

	//40-43
	//%ls is the name of merc.
	L"%ls用完了医药箱里的药品!",	//"%ls ran out of medical kits!",
	L"%ls无所需技能来医疗他人!",	//"%ls lacks the necessary skill to doctor anyone!",
	L"%ls用完工具箱里的工具!",	//"%ls ran out of tool kits!",
	L"%ls无所需技能来修理物品!",	//"%ls lacks the necessary skill to repair anything!",

	//44-45
	L"Repair Time",
	L"%ls看不到此人.",

	//46-48
	L"%ls的增程枪管掉下来了!",	//"%ls's gun barrel extender falls off!",
	L"只允许不多于%d名佣兵在此区域训练民兵.",	//"No more than %d militia trainers are permitted per sector.", //L"No more than %d militia trainers are permitted in this sector.",//ham3.6
	L"您确定吗?",			//"Are you sure?",

	//49-50
	L"时间压缩",
	L"车辆的油箱已经加满油了.",

	//51-52 Fast help text in mapscreen.
	L"继续时间压缩 (|S|p|a|c|e)",
	L"停止时间压缩 (|E|s|c)",

	//53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
	L"%ls修理好了卡壳的%ls",	//L"%ls has unjammed the %ls",
	L"%ls修理好了卡壳的%ls的%ls",	//L"%ls has unjammed %ls's %ls",

	//55
	L"查看区域存货时候无法压缩时间",	//L"Can't compress time while viewing sector inventory.",

	//56
	//Displayed with the version information when cheats are enabled.
	L"当前/最大进展: %d%%/%d%%",

	L"护送John和Mary?",

	// 58
	L"开关被激活.",
};

static const wchar_t s_chn_str_ceramic_plates_smashed[] = 
	L"%ls的陶瓷片已经粉碎了!";

static const wchar_t s_chn_str_arrival_rerouted[] = 
	L"援兵的会合地被挪至%ls, 因降落地点%ls目前由敌人占据.";

static const wchar_t s_chn_str_stat_health[]       = 
	L"生命";
static const wchar_t s_chn_str_stat_agility[]      = 
	L"敏捷";
static const wchar_t s_chn_str_stat_dexterity[]    = 
	L"灵巧";
static const wchar_t s_chn_str_stat_strength[]     = 
	L"力量";
static const wchar_t s_chn_str_stat_leadership[]   = 
	L"领导";
static const wchar_t s_chn_str_stat_wisdom[]       = 
	L"智慧";
static const wchar_t s_chn_str_stat_exp_level[]    = 
	L"级别";
static const wchar_t s_chn_str_stat_marksmanship[] = 
	L"枪法";
static const wchar_t s_chn_str_stat_mechanical[]   = 
	L"机械";
static const wchar_t s_chn_str_stat_explosive[]    = 
	L"爆破";
static const wchar_t s_chn_str_stat_medical[]      = 
	L"医疗";

static const wchar_t *s_chn_str_stat_list[str_stat_list_SIZE] =
{
	s_chn_str_stat_health,
	s_chn_str_stat_agility,
	s_chn_str_stat_dexterity,
	s_chn_str_stat_strength,
	s_chn_str_stat_leadership,
	s_chn_str_stat_wisdom,
	s_chn_str_stat_exp_level,
	s_chn_str_stat_marksmanship,
	s_chn_str_stat_mechanical,
	s_chn_str_stat_explosive,
	s_chn_str_stat_medical
};

static const wchar_t *s_chn_str_aim_sort_list[str_aim_sort_list_SIZE] =
{
	s_chn_str_aim_sort_price,
	s_chn_str_aim_sort_experience,
	s_chn_str_aim_sort_marksmanship,
	s_chn_str_aim_sort_medical,
	s_chn_str_aim_sort_explosives,
	s_chn_str_aim_sort_mechanical,
	s_chn_str_aim_sort_ascending,
	s_chn_str_aim_sort_descending,
};

// VERY TRUNCATED FILE COPIED FROM JA2.5 FOR ITS FEATURES FOR JA2 GOLD

const wchar_t *g_chn_zNewTacticalMessages[] =
{
	L"到目标的距离: %d格",
	L"射程: %d格, 到目标的距离: %d格",
	L"显示遮盖",
	L"视线",
	L"这是铁人模式的游戏, 敌人在周围时您不能保存.",	//	@@@  new text
	L"(战斗中不能保存)", //@@@@ new text
	L"(不能载入之前的存档)", // Stracciatella: New Dead is Dead game mode text
	L"这是Dead is Dead模式的游戏, 您不能通过保存界面保存.",
};

//@@@:  New string as of March 3, 2000.
const wchar_t *g_chn_str_iron_man_mode_warning = 
	L"您选择了铁人模式. 此模式使游戏更具挑战性, 例如进入敌占区时不能保存游戏. 设定将在整个游戏进程中生效.  您确定想玩铁人模式吗?";

// Stracciatella: New Dead is Dead game mode warning
const wchar_t *g_chn_str_dead_is_dead_mode_warning = 
	L"您选择了DEAD IS DEAD模式. 此模式使游戏更具挑战性, 例如您不能在犯错后载入之前的存档. 游戏将在安全时自动保存. 设定将在整个游戏进程中生效.  您确定想玩DEAD IS DEAD模式吗?";
const wchar_t *g_chn_str_dead_is_dead_mode_enter_name = 
	L"您现在将进入保存界面. 选择一个位置并为您的存档命名. 您的游戏将持续存于此位置, 且无法更改!";

static const wchar_t *s_chn_gs_dead_is_dead_mode_tab_name[gs_dead_is_dead_mode_tab_name_SIZE] =
{
	L"普通", 			// Normal Tab
	L"DiD", 			// Dead is Dead Tab
};

// English language resources.
LanguageRes g_LanguageResChinese = {

	s_chn_WeaponType,

	s_chn_Message,
	s_chn_TeamTurnString,
	s_chn_pAssignMenuStrings,
	s_chn_pTrainingStrings,
	s_chn_pTrainingMenuStrings,
	s_chn_pAttributeMenuStrings,
	s_chn_pVehicleStrings,
	s_chn_pShortAttributeStrings,
	s_chn_pContractStrings,
	s_chn_pAssignmentStrings,
	s_chn_pConditionStrings,
	s_chn_pTownNames,
	s_chn_g_towns_locative,
	s_chn_pPersonnelScreenStrings,
	s_chn_pUpperLeftMapScreenStrings,
	s_chn_pTacticalPopupButtonStrings,
	s_chn_pSquadMenuStrings,
	s_chn_pDoorTrapStrings,
	s_chn_pLongAssignmentStrings,
	s_chn_pMapScreenMouseRegionHelpText,
	s_chn_pNoiseVolStr,
	s_chn_pNoiseTypeStr,
	s_chn_pDirectionStr,
	s_chn_pRemoveMercStrings,
	s_chn_sTimeStrings,
	s_chn_pLandTypeStrings,
	s_chn_pInvPanelTitleStrings,
	s_chn_pPOWStrings,
	s_chn_pMilitiaString,
	s_chn_pMilitiaButtonString,
	s_chn_pEpcMenuStrings,
	s_chn_pRepairStrings,
	s_chn_sPreStatBuildString,
	s_chn_sStatGainStrings,
	s_chn_pHelicopterEtaStrings,
	s_chn_sMapLevelString,
	s_chn_gsLoyalString,
	s_chn_gsUndergroundString,
	s_chn_gsTimeStrings,
	s_chn_sFacilitiesStrings,
	s_chn_pMapPopUpInventoryText,
	s_chn_pwTownInfoStrings,
	s_chn_pwMineStrings,
	s_chn_pwMiscSectorStrings,
	s_chn_pMapInventoryErrorString,
	s_chn_pMapInventoryStrings,
	s_chn_pMovementMenuStrings,
	s_chn_pUpdateMercStrings,
	s_chn_pMapScreenBorderButtonHelpText,
	s_chn_pMapScreenBottomFastHelp,
	s_chn_pMapScreenBottomText,
	s_chn_pMercDeadString,
	s_chn_pSenderNameList,
	s_chn_pNewMailStrings,
	s_chn_pDeleteMailStrings,
	s_chn_pEmailHeaders,
	s_chn_pEmailTitleText,
	s_chn_pFinanceTitle,
	s_chn_pFinanceSummary,
	s_chn_pFinanceHeaders,
	s_chn_pTransactionText,
	s_chn_pMoralStrings,
	s_chn_pSkyriderText,
	s_chn_str_left_equipment,
	s_chn_pMapScreenStatusStrings,
	s_chn_pMapScreenPrevNextCharButtonHelpText,
	s_chn_pEtaString,
	s_chn_pShortVehicleStrings,
	s_chn_pTrashItemText,
	s_chn_pMapErrorString,
	s_chn_pMapPlotStrings,
	s_chn_pBullseyeStrings,
	s_chn_pMiscMapScreenMouseRegionHelpText,
	s_chn_str_he_leaves_where_drop_equipment,
	s_chn_str_she_leaves_where_drop_equipment,
	s_chn_str_he_leaves_drops_equipment,
	s_chn_str_she_leaves_drops_equipment,
	s_chn_pImpPopUpStrings,
	s_chn_pImpButtonText,
	s_chn_pExtraIMPStrings,
	s_chn_pFilesTitle,
	s_chn_pFilesSenderList,
	s_chn_pHistoryLocations,
	s_chn_pHistoryStrings,
	s_chn_pHistoryHeaders,
	s_chn_pHistoryTitle,
	s_chn_pShowBookmarkString,
	s_chn_pWebPagesTitles,
	s_chn_pWebTitle,
	s_chn_pPersonnelString,
	s_chn_pErrorStrings,
	s_chn_pDownloadString,
	s_chn_pBookMarkStrings,
	s_chn_pLaptopIcons,
	s_chn_gsAtmStartButtonText,
	s_chn_pPersonnelTeamStatsStrings,
	s_chn_pPersonnelCurrentTeamStatsStrings,
	s_chn_pPersonelTeamStrings,
	s_chn_pPersonnelDepartedStateStrings,
	s_chn_pMapHortIndex,
	s_chn_pMapVertIndex,
	s_chn_pMapDepthIndex,
	s_chn_pLaptopTitles,
	s_chn_pDayStrings,
	s_chn_pMilitiaConfirmStrings,
	s_chn_pDeliveryLocationStrings,
	s_chn_pSkillAtZeroWarning,
	s_chn_pIMPBeginScreenStrings,
	s_chn_pIMPFinishButtonText,
	s_chn_pIMPFinishStrings,
	s_chn_pIMPVoicesStrings,
	s_chn_pPersTitleText,
	s_chn_pPausedGameText,
	s_chn_zOptionsToggleText,
	s_chn_zOptionsScreenHelpText,
	s_chn_pDoctorWarningString,
	s_chn_pMilitiaButtonsHelpText,
	s_chn_pMapScreenJustStartedHelpText,
	s_chn_pLandMarkInSectorString,
	s_chn_gzMercSkillText,
	s_chn_gzNonPersistantPBIText,
	s_chn_gzMiscString,
	s_chn_wMapScreenSortButtonHelpText,
	s_chn_pNewNoiseStr,
	s_chn_gzLateLocalizedString,
	s_chn_pAntiHackerString,
	s_chn_pMessageStrings,
	s_chn_ItemPickupHelpPopup,
	s_chn_TacticalStr,
	s_chn_LargeTacticalStr,
	s_chn_zDialogActions,
	s_chn_zDealerStrings,
	s_chn_zTalkMenuStrings,
	s_chn_gzMoneyAmounts,
	s_chn_gzProsLabel,
	s_chn_gzConsLabel,
	s_chn_gMoneyStatsDesc,
	s_chn_gWeaponStatsDesc,
	s_chn_sKeyDescriptionStrings,
	s_chn_zHealthStr,
	s_chn_zVehicleName,
	s_chn_pExitingSectorHelpText,
	s_chn_InsContractText,
	s_chn_InsInfoText,
	s_chn_MercAccountText,
	s_chn_MercInfo,
	s_chn_MercNoAccountText,
	s_chn_MercHomePageText,
	s_chn_sFuneralString,
	s_chn_sFloristText,
	s_chn_sOrderFormText,
	s_chn_sFloristGalleryText,
	s_chn_sFloristCards,
	s_chn_BobbyROrderFormText,
	s_chn_BobbyRText,
	s_chn_str_bobbyr_guns_num_guns_that_use_ammo,
	s_chn_BobbyRaysFrontText,
	s_chn_AimSortText,
	s_chn_str_aim_sort_price,
	s_chn_str_aim_sort_experience,
	s_chn_str_aim_sort_marksmanship,
	s_chn_str_aim_sort_medical,
	s_chn_str_aim_sort_explosives,
	s_chn_str_aim_sort_mechanical,
	s_chn_str_aim_sort_ascending,
	s_chn_str_aim_sort_descending,
	s_chn_AimPolicyText,
	s_chn_AimMemberText,
	s_chn_CharacterInfo,
	s_chn_VideoConfercingText,
	s_chn_AimPopUpText,
	s_chn_AimLinkText,
	s_chn_AimHistoryText,
	s_chn_AimFiText,
	s_chn_AimAlumniText,
	s_chn_AimScreenText,
	s_chn_AimBottomMenuText,
	s_chn_zMarksMapScreenText,
	s_chn_gpStrategicString,
	s_chn_gpGameClockString,
	s_chn_SKI_Text,
	s_chn_SkiMessageBoxText,
	s_chn_zSaveLoadText,
	s_chn_zOptionsText,
	s_chn_gzGIOScreenText,
	s_chn_gzHelpScreenText,
	s_chn_gzLaptopHelpText,
	s_chn_gzMoneyWithdrawMessageText,
	s_chn_gzCopyrightText,
	s_chn_BrokenLinkText,
	s_chn_gzBobbyRShipmentText,
	s_chn_zGioDifConfirmText,
	s_chn_gzCreditNames,
	s_chn_gzCreditNameTitle,
	s_chn_gzCreditNameFunny,
	s_chn_pContractButtonString,
	s_chn_gzIntroScreen,
	s_chn_pUpdatePanelButtons,
	s_chn_sRepairsDoneString,
	s_chn_str_ceramic_plates_smashed,
	s_chn_str_arrival_rerouted,

	s_chn_str_stat_health,
	s_chn_str_stat_agility,
	s_chn_str_stat_dexterity,
	s_chn_str_stat_strength,
	s_chn_str_stat_leadership,
	s_chn_str_stat_wisdom,
	s_chn_str_stat_exp_level,
	s_chn_str_stat_marksmanship,
	s_chn_str_stat_mechanical,
	s_chn_str_stat_explosive,
	s_chn_str_stat_medical,

	s_chn_str_stat_list,
	s_chn_str_aim_sort_list,

	g_chn_zNewTacticalMessages,
	g_chn_str_iron_man_mode_warning,
	g_chn_str_dead_is_dead_mode_warning,
	g_chn_str_dead_is_dead_mode_enter_name,

	s_chn_gs_dead_is_dead_mode_tab_name,
};
