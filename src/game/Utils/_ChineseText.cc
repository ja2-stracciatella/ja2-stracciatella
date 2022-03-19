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



static ST::string s_cn_WeaponType[WeaponType_SIZE] =
{
	"其他",
	"手枪",
	"自动手枪",
	"冲锋枪",
	"步枪",
	"狙击步枪",
	"突击步枪",
	"轻机枪",
	"霰弹枪",
};

static ST::string s_cn_TeamTurnString[TeamTurnString_SIZE] =
{
	"玩家的回合",
	"敌军的回合",
	"异形的回合",
	"民兵的回合",
	"平民的回合",
	// planning turn
};

static ST::string s_cn_Message[Message_SIZE] =
{
	// In the following 8 strings, the %s is the merc's name, and the %d (if any) is a number.

	"%s被射中了头部, 并且失去了1点智慧!",
	"%s被射中了肩部, 并且失去了1点灵巧!",
	"%s被射中了胸膛, 并且失去了1点力量!",
	"%s被射中了腿部, 并且失去了1点敏捷!",
	"%s被射中了头部, 并且失去了%d点智慧!",
	"%s被射中了肩部, 并且失去了%d点灵巧!",
	"%s被射中了胸膛, 并且失去了%d点力量!",
	"%s被射中了腿部, 并且失去了%d点敏捷!",
	"中断!",

	"您的援军到达了!",

	// In the following four lines, all %s's are merc names

	"%s装填弹药.",
	"%s没有足够的行动点数.",
	// the following 17 strings are used to create lists of gun advantages and disadvantages
	// (separated by commas)
	"耐用",
	"不耐用",
	"容易修复",
	"不易修复",
	"杀伤力高",
	"杀伤力低",
	"射击快",
	"射击慢",
	"射程远",
	"射程近",
	"轻盈",
	"笨重",
	"小巧",
	"射速高",
	"无点射限制",
	"大容弹量",
	"小容弹量",

	// In the following two lines, all %s's are merc names

	"%s的伪装失效了.",
	"%s的伪装被洗掉了.",

	// The first %s is a merc name and the second %s is an item name

	"副武器没弹药了!",//	"Second weapon is out of ammo!",
	"%s偷到了%s.",		//	"%s has stolen the %s.",

	// The %s is a merc name

	"%s的武器不能扫射.",	//	"%s's weapon can't burst fire.",

	"您已经装上了该附件.",//	"You've already got one of those attached.",
	"组合物品?",			//	"Merge items?",

	// Both %s's are item names

	"您不能把%s和%s组合在一起.",

	"无",
	"退出子弹",
	"附件",

	//You cannot use "item(s)" and your "other item" at the same time.
	//Ex:  You cannot use sun goggles and you gas mask at the same time.
	"您不能同时使用%s和%s.",

	"请把光标选中的物品放到另一物品的任意附件格中, 这样就可能合成新物品.",
	"请把光标选中的物品放到另一物品的任意附件格中, 这样就可能合成新物品.(但是这一次, 该物品不相容.)",
	"该区域的敌军尚未被肃清",
	"您还得给%s%s",
	"%s被射中了头部.",
	"放弃战斗?",
	"这个组合是永久性的.您确认要这样做吗?",
	"%s感觉精力充沛!",
	"%s踩到了大理石珠子, 滑倒了!",
	"%s没能从敌人手里抢到%s!",
	"%s修复了%s.",
	"中断: ",
	"投降?",
	"此人拒绝您的包扎.",
	"我不这么认为.",
	"要搭乘Skyrider的直升机, 您得先把佣兵分配到交通工具/直升机.",
	"%s的时间只够给一支枪装填弹药",
	"血猫的回合",
};


// the types of time compression. For example: is the timer paused? at normal speed, 5 minutes per second, etc.
// min is an abbreviation for minutes

static const ST::string s_cn_sTimeStrings[sTimeStrings_SIZE] =
{
	"暂停",
	"普通",
	"5 mins",
	"30 mins",
	"60 mins",
	"6 hrs",
};


// Assignment Strings: what assignment does the merc  have right now? For example, are they on a squad, training,
// administering medical aid (doctor) or training a town. All are abbreviated. 8 letters is the longest it can be.

static const ST::string s_cn_pAssignmentStrings[pAssignmentStrings_SIZE] =
{
	"第1小队",
	"第2小队",
	"第3小队",
	"第4小队",
	"第5小队",
	"第6小队",
	"第7小队",
	"第8小队",
	"第9小队",
	"第10小队",
	"第11小队",
	"第12小队",
	"第13小队",
	"第14小队",
	"第15小队",
	"第16小队",
	"第17小队",
	"第18小队",
	"第19小队",
	"第20小队",
	"编队",// on active duty
	"医生",// administering medical aid
	"病人", // getting medical aid
	"交通工具", // in a vehicle
	"在途中",// in transit - abbreviated form
	"修理", // repairing
	"锻炼", // training themselves
	"民兵", // training a town to revolt
	"教练", // training a teammate
	"学员", // being trained by someone else
	"死亡", // dead
	"无力中", // abbreviation for incapacitated
	"战俘", // Prisoner of war - captured
	"医院", // patient in a hospital
	"空",	// Vehicle is empty
};


static const ST::string s_cn_pMilitiaString[pMilitiaString_SIZE] =
{
	"民兵", // the title of the militia box
	"未分配的民兵", //the number of unassigned militia troops
	"本地区有敌军存在, 您无法重新分配民兵!",
};


static const ST::string s_cn_pMilitiaButtonString[pMilitiaButtonString_SIZE] =
{
	"自动", // auto place the militia troops for the player
	"完成", // done placing militia troops
};

static const ST::string s_cn_pConditionStrings[pConditionStrings_SIZE] =
{
	"极好", //the state of a soldier .. excellent health
	"良好", // good health
	"普通", // fair health
	"受伤", // wounded health
	"疲劳", // tired
	"失血", // bleeding to death
	"昏迷", // knocked out
	"垂死", // near death
	"死亡", // dead
};

static const ST::string s_cn_pEpcMenuStrings[pEpcMenuStrings_SIZE] =
{
	"编队", // set merc on active duty
	"病人",// set as a patient to receive medical aid
	"交通工具", // tell merc to enter vehicle
	"无护送", // let the escorted character go off on their own
	"取消", // close this menu
};


// look at pAssignmentString above for comments

static const ST::string s_cn_pLongAssignmentStrings[pLongAssignmentStrings_SIZE] =
{
	"第1小队",
	"第2小队",
	"第3小队",
	"第4小队",
	"第5小队",
	"第6小队",
	"第7小队",
	"第8小队",
	"第9小队",
	"第10小队",
	"第11小队",
	"第12小队",
	"第13小队",
	"第14小队",
	"第15小队",
	"第16小队",
	"第17小队",
	"第18小队",
	"第19小队",
	"第20小队",
	"编队",// on active duty
	"医生",// administering medical aid
	"病人", // getting medical aid
	"交通工具", // in a vehicle
	"在途中",// in transit - abbreviated form
	"修理", // repairing
	"锻炼", // training themselves
	"民兵", // training a town to revolt
	"教练", // training a teammate
	"学员", // being trained by someone else
	"死亡", // dead
	"无力中", // abbreviation for incapacitated
	"战俘", // Prisoner of war - captured
	"医院", // patient in a hospital
	"空",	// Vehicle is empty
};


// the contract options

static const ST::string s_cn_pContractStrings[pContractStrings_SIZE] =
{
	"合同选项: ",
	"", // a blank line, required
	"雇佣一日",// offer merc a one day contract extension
	"雇佣一周", // 1 week
	"雇佣两周", // 2 week
	"解雇",// end merc's contract
	"取消", // stop showing this menu
};

static const ST::string s_cn_pPOWStrings[pPOWStrings_SIZE] =
{
	"囚禁",  //an acronym for Prisoner of War
	"??",
};

static const ST::string s_cn_pInvPanelTitleStrings[pInvPanelTitleStrings_SIZE] =
{
	"护甲", // the armor rating of the merc
	"负重", // the weight the merc is carrying
	"伪装", // the merc's camouflage rating
};

static const ST::string s_cn_pShortAttributeStrings[pShortAttributeStrings_SIZE] =
{
	"敏捷", // the abbreviated version of : agility
	"灵巧", // dexterity
	"力量", // strength
	"领导", // leadership
	"智慧", // wisdom
	"级别", // experience level
	"枪法", // marksmanship skill
	"爆破", // explosive skill
	"机械", // mechanical skill
	"医疗", // medical skill
};


static const ST::string s_cn_pUpperLeftMapScreenStrings[pUpperLeftMapScreenStrings_SIZE] =
{
	"任务", // the mercs current assignment
	"生命", // the health level of the current merc
	"士气", // the morale of the current merc
	"状态",	// the condition of the current vehicle
};

static const ST::string s_cn_pTrainingStrings[pTrainingStrings_SIZE] =
{
	"锻炼", // tell merc to train self
	"民兵",// tell merc to train town
	"教练", // tell merc to act as trainer
	"学员", // tell merc to be train by other
};

static const ST::string s_cn_pAssignMenuStrings[pAssignMenuStrings_SIZE] =
{
	"编队",
	"医生",
	"病人",
	"交通工具",
	"修理",
	"训练",
	"取消", // cancel this menu
};

static const ST::string s_cn_pRemoveMercStrings[pRemoveMercStrings_SIZE] =
{
	"移除佣兵", // remove dead merc from current team
	"取消",
};

static const ST::string s_cn_pAttributeMenuStrings[pAttributeMenuStrings_SIZE] =
{
	"力量",	//"Strength",
	"灵巧",	//"Dexterity",
	"敏捷",	//"Agility",
	"生命",	//"Health",
	"枪法",	//"Marksmanship",
	"医疗",	//"Medical",
	"机械",	//"Mechanical",
	"领导",	//"Leadership",
	"爆破",	//"Explosives",
	"取消",	//"Cancel",
};

static const ST::string s_cn_pTrainingMenuStrings[pTrainingMenuStrings_SIZE] =
{
	"锻炼", // tell merc to train self
	"民兵",// tell merc to train town
	"教练", // tell merc to act as trainer
	"学员", // tell merc to be train by other
	"取消", // cancel this menu
};


static const ST::string s_cn_pSquadMenuStrings[pSquadMenuStrings_SIZE] =
{
	"第1小队",
	"第2小队",
	"第3小队",
	"第4小队",
	"第5小队",
	"第6小队",
	"第7小队",
	"第8小队",
	"第9小队",
	"第10小队",
	"第11小队",
	"第12小队",
	"第13小队",
	"第14小队",
	"第15小队",
	"第16小队",
	"第17小队",
	"第18小队",
	"第19小队",
	"第20小队",
	"取消",
};


static const ST::string s_cn_pPersonnelScreenStrings[pPersonnelScreenStrings_SIZE] =
{
	"医疗保证金: ", // amount of medical deposit put down on the merc
	"合同剩余时间: ", // cost of current contract
	"击毙数: ", // number of kills by merc
	"击伤数: ",// number of assists on kills by merc
	"日薪: ", // daily cost of merc
	"花费总数: ",// total cost of merc
	"合同花费: ",
	"总日数: ",// total service rendered by merc
	"欠付佣金: ",// amount left on MERC merc to be paid
	"命中率: ",// percentage of shots that hit target
	"战斗次数: ", // number of battles fought
	"受伤次数: ", // number of times merc has been wounded
	"技能: ",
	"无技能",
};


//These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
static const ST::string s_cn_gzMercSkillText[gzMercSkillText_SIZE] =
{
	"无技能",
	"开锁",
	"徒手格斗",		//JA25: modified
	"电子",
	"夜战",			//JA25: modified
	"投掷",
	"教学",
	"重武器",
	"自动武器",
	"潜行",
	"双手武器",
	"偷窃",
	"武术",
	"刀技",
	"狙击手",
	"伪装",					//JA25: modified
	"(专家)",
};


// This is pop up help text for the options that are available to the merc

static const ST::string s_cn_pTacticalPopupButtonStrings[pTacticalPopupButtonStrings_SIZE] =
{
	"站立/行走 (|S)",
	"蹲伏/蹲伏前进(|C)",
	"站立/奔跑 (|R)",
	"匍匐/匍匐前进(|P)",
	"查看(|L)",
	"行动",
	"交谈",
	"检查 (|C|t|r|l)",

	// Pop up door menu
	"用手开门",
	"检查陷阱",
	"开锁",
	"踢门",
	"解除陷阱",
	"关门",
	"开门",
	"使用破门炸药",
	"使用撬棍",
	"取消 (|E|s|c)",
	"关闭",
};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.

static const ST::string s_cn_pDoorTrapStrings[pDoorTrapStrings_SIZE] =
{
	"无陷阱",
	"一个爆炸陷阱",
	"一个带电陷阱",
	"一个警报陷阱",
	"一个无声警报陷阱",
};

// On the map screen, there are four columns. This text is popup help text that identifies the individual columns.

static const ST::string s_cn_pMapScreenMouseRegionHelpText[pMapScreenMouseRegionHelpText_SIZE] =
{
	"选择角色",
	"分配任务",
	"安排行军路线",
	"签约 (|C)",
	"移除佣兵",
	"睡觉",
};

// volumes of noises

static const ST::string s_cn_pNoiseVolStr[pNoiseVolStr_SIZE] =
{
	"微弱的",
	"清晰的",
	"大声的",
	"非常大声的",
};

// types of noises

static const ST::string s_cn_pNoiseTypeStr[pNoiseTypeStr_SIZE] = // OBSOLETE
{
	"未知",
	"移动声",
	"辗扎声",
	"溅泼声",
	"撞击声",
	"枪声",
	"爆炸声",
	"尖叫声",
	"撞击声",
	"撞击声",
	"粉碎声",
	"破碎声",
};

// Directions that are used to report noises

static const ST::string s_cn_pDirectionStr[pDirectionStr_SIZE] =
{
	"东北方",
	"东方",
	"东南方",
	"南方",
	"西南方",
	"西方",
	"西北方",
	"北方"
};

static const ST::string s_cn_gpStrategicString[gpStrategicString_SIZE] =
{
	"%s在%c%d区域被发现了, 另一小队即将到达.",	//STR_DETECTED_SINGULAR
	"%s在%c%d区域被发现了, 其它几个小队即将到达.",	//STR_DETECTED_PLURAL
	"您想调整为同时到达吗?",												//STR_COORDINATE

	//Dialog strings for enemies.

	"敌军给您一个投降的机会.",
	"敌军俘虏了昏迷中的佣兵.",

	//The text that goes on the autoresolve buttons

	"撤退",		//The retreat button
	"完成",		//The done button				//STR_AR_DONE_BUTTON

	//The headers are for the autoresolve type (MUST BE UPPERCASE)

	"防守",							//STR_AR_DEFEND_HEADER
	"攻击",						//STR_AR_ATTACK_HEADER
	"遭遇战",						//STR_AR_ENCOUNTER_HEADER
	"区域",	//The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

	//The battle ending conditions

	"胜利!",								//STR_AR_OVER_VICTORY
	"失败!",								//STR_AR_OVER_DEFEAT
	"投降!",							//STR_AR_OVER_SURRENDERED
	"被俘!",								//STR_AR_OVER_CAPTURED
	"撤退!",								//STR_AR_OVER_RETREATED

	//These are the labels for the different types of enemies we fight in autoresolve.

	"民兵",							//STR_AR_MILITIA_NAME,
	"精兵",							//STR_AR_ELITE_NAME,
	"部队",							//STR_AR_TROOP_NAME,
	"行政人员",								//STR_AR_ADMINISTRATOR_NAME,
	"异形",								//STR_AR_CREATURE_NAME,

	//Label for the length of time the battle took

	"战斗用时",						//STR_AR_TIME_ELAPSED,

	//Labels for status of merc if retreating.  (UPPERCASE)

	"已撤退",						//STR_AR_MERC_RETREATED,
	"正在撤退",						//STR_AR_MERC_RETREATING,
	"撤退",						//STR_AR_MERC_RETREAT,

	//PRE BATTLE INTERFACE STRINGS
	//Goes on the three buttons in the prebattle interface.  The Auto resolve button represents
	//a system that automatically resolves the combat for the player without having to do anything.
	//These strings must be short (two lines -- 6-8 chars per line)

	"自动战斗",							//STR_PB_AUTORESOLVE_BTN,
	"进入战区",							//STR_PB_GOTOSECTOR_BTN,
	"撤退佣兵",							//STR_PB_RETREATMERCS_BTN,

	//The different headers(titles) for the prebattle interface.
	"遭遇敌军",						//STR_PB_ENEMYENCOUNTER_HEADER,
	"敌军入侵",						//STR_PB_ENEMYINVASION_HEADER, // 30
	"敌军伏击",
	"进入敌占区",			//STR_PB_ENTERINGENEMYSECTOR_HEADER
	"异形攻击",					//STR_PB_CREATUREATTACK_HEADER
	"血猫伏击",					//STR_PB_BLOODCATAMBUSH_HEADER
	"进入血猫巢穴",

	//Various single words for direct translation.  The Civilians represent the civilian
	//militia occupying the sector being attacked.  Limited to 9-10 chars

	"地区",
	"敌军",
	"佣兵",
	"民兵",
	"异形",
	"血猫",
	"区域",
	"无人",	//If there are no uninvolved mercs in this fight.
	"N/A",			//Acronym of Not Applicable
	"日",			//One letter abbreviation of day
	"时",			//One letter abbreviation of hour

	//TACTICAL PLACEMENT USER INTERFACE STRINGS
	//The four buttons

	"清除",
	"分散",
	"集中",
	"完成",

	//The help text for the four buttons.  Use \n to denote new line (just like enter).

	"清除所有佣兵的布阵, 然后您可以\n重新手动对他们进行安排.(|c)",
	"每次按本按钮, 就会重新 \n随机分散地布阵佣兵.(|s)",
	"您可以选择您想集中地布阵佣兵的地方.(|g)",
	"当您完成对佣兵布阵后, \n请按本按钮. (|E|n|t|e|r)",
	"开始战斗前, 您必须\n对所有佣兵完成布阵.",

	//Various strings (translate word for word)

	"区域",
	"选择进入的位置",

	//Strings used for various popup message boxes.  Can be as long as desired.

	"看起来不太好.无法进入这里.换个不同的位置吧.",
	"请把佣兵放在地图的高亮区域里.",

	//These entries are for button popup help text for the prebattle interface.  All popup help
	//text supports the use of \n to denote new line.  Do not use spaces before or after the \n.
	"自动解决战斗, 不需要\n载入该区域地图.",
	"当玩家在攻击时, 无法使用\n自动战斗功能.",
	"进入该区域和敌军作战(|E)",
	"将小队撤退到先前的区域.(|R)",				//singular version
	"将所有小队撤退到先前的区域.(|R)", //multiple groups with same previous sector

	//various popup messages for battle conditions.

	//%c%d is the sector -- ex:  A9
	"敌军向您在%c%d区域的民兵发起了攻击.",
	//%c%d is the sector -- ex:  A9
	"异形向您在%c%d区域的民兵发起了攻击.",
	//1st %d refers to the number of civilians eaten by monsters,  %c%d is the sector -- ex:  A9
	//Note:  the minimum number of civilians eaten will be two.
	"异形攻击了区域%s, 吃掉了%d名平民.",
	//%s is the sector location -- ex:  A9: Omerta
	"敌军向您在区域%s的佣兵发起了攻击.您的佣兵中没人能进行战斗.",
	//%s is the sector location -- ex:  A9: Omerta
	"异形向您在区域%s的佣兵发起了攻击.您的佣兵中没人能进行战斗.",

};

//This is the day represented in the game clock.  Must be very short, 4 characters max.
static const ST::string s_cn_gpGameClockString = "日";

//When the merc finds a key, they can get a description of it which
//tells them where and when they found it.
static const ST::string s_cn_sKeyDescriptionStrings[sKeyDescriptionStrings_SIZE] =
{
	"找到的区域: ",
	"找到的日期: ",
};

//The headers used to describe various weapon statistics.

static ST::string s_cn_gWeaponStatsDesc[ gWeaponStatsDesc_SIZE] =
{
	"重量 (%s):",
	"状态:",
	"弹药:", 		// Number of bullets left in a magazine
	"射程",		// Range
	"杀伤:",		// Damage
	"AP:",			// abbreviation for Action Points
	"=",
};

//The headers used for the merc's money.

static const ST::string s_cn_gMoneyStatsDesc[gMoneyStatsDesc_SIZE] =
{
	"剩余",
	"金额: ",//this is the overall balance
	"分割",
	"金额: ", // the amount he wants to separate from the overall balance to get two piles of money

	"当前",
	"余额",
	"提取",
	"金额",
};

//The health of various creatures, enemies, characters in the game. The numbers following each are for comment
//only, but represent the precentage of points remaining.

static const ST::string s_cn_zHealthStr[zHealthStr_SIZE] =
{
	"垂死",	//"DYING",		//	>= 0
	"濒危",	//"CRITICA", 		//	>= 15
	"虚弱",	//"POOR",		//	>= 30
	"受伤",	//"WOUNDED",    	//	>= 45
	"健康",	//"HEALTHY",    	//	>= 60
	"强壮",	//"STRONG",     	// 	>= 75
	"极好",	//"EXCELLENT",		// 	>= 90
};

static const ST::string s_cn_gzMoneyAmounts[gzMoneyAmounts_SIZE] =
{
	"$1000",
	"$100",
	"$10",
	"完成",
	"分割",
	"提取",
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
static const ST::string s_cn_gzProsLabel = "优点:";
static const ST::string s_cn_gzConsLabel = "缺点:";

//Conversation options a player has when encountering an NPC
static ST::string s_cn_zTalkMenuStrings[zTalkMenuStrings_SIZE] =
{
	"再说一次?",	//meaning "Repeat yourself"
	"友好",		//approach in a friendly
	"直率",		//approach directly - let's get down to business
	"恐吓",		//approach threateningly - talk now, or I'll blow your face off
	"给予",
	"招募",
};

//Some NPCs buy, sell or repair items. These different options are available for those NPCs as well.
static ST::string s_cn_zDealerStrings[zDealerStrings_SIZE] =
{
	"买/卖",
	"买",
	"卖",
	"修理",
};

static const ST::string s_cn_zDialogActions = "Done";


//These are vehicles in the game.

static const ST::string s_cn_pVehicleStrings[pVehicleStrings_SIZE] =
{
	"Eldorado",
	"悍马", // a hummer jeep/truck -- military vehicle
	"冰淇淋车",
	"吉普",
	"坦克",
	"直升机",
};

static const ST::string s_cn_pShortVehicleStrings[pVehicleStrings_SIZE] =
{
	"Eldor.",
	"悍马", // a hummer jeep/truck -- military vehicle
	"冰淇淋",
	"吉普",
	"坦克",
	"直升机", 				// the helicopter
};

static const ST::string s_cn_zVehicleName[pVehicleStrings_SIZE] =
{
	"Eldor.",
	"悍马", // a hummer jeep/truck -- military vehicle
	"冰淇淋",
	"吉普",
	"坦克",
	"直升机", 		//an abbreviation for Helicopter
};


//These are messages Used in the Tactical Screen

static ST::string s_cn_TacticalStr[TacticalStr_SIZE] =
{
	"空袭",
	"自动包扎?",

	// CAMFIELD NUKE THIS and add quote #66.

	"%s发现运来的货品短缺了几件.",

	// The %s is a string from pDoorTrapStrings

	"锁上有%s.",
	"未上锁.",
	"锁上未被设置陷阱.",
	// The %s is a merc name
	"%s无对应的钥匙.",
	"锁上未被设置陷阱.",
	"锁住了.",
	"门",
	"有陷阱的",
	"锁住的",
	"没锁的",
	"被打烂的",
	"这里有一个开关.启动它吗?",
	"解除陷阱?",
	"更多...",

	// In the next 2 strings, %s is an item name

	"%s被放在地上了.",
	"%s被交给%s了.",

	// In the next 2 strings, %s is a name

	"%s已经被完全支付.",
	"%s还拖欠%d.",
	"选择引爆的频率", //in this case, frequency refers to a radio signal
	"设定几个回合后爆炸: ", //how much time, in turns, until the bomb blows
	"设定遥控雷管的频率: ",//in this case, frequency refers to a radio signal
	"解除诡雷?",
	"移掉蓝旗?",
	"在这里插上蓝旗吗?",
	"结束回合",

	// In the next string, %s is a name. Stance refers to way they are standing.

	"您确定要攻击%s吗?",
	"车辆无法变动姿势.",
	"机器人无法变动姿势.",

	// In the next 3 strings, %s is a name

	"%s无法在这里变为该姿势.",
	"%s无法在这里被包扎.",
	"%s不需要包扎.",
	"不能移动到那儿.",
	"您的队伍已经满员了.无空位雇佣新队员.",	//there's no room for a recruit on the player's team

	// In the next string, %s is a name

	"%s已经被招募.",

	// Here %s is a name and %d is a number

	"尚拖欠%s$%d.",

	// In the next string, %s is a name

	"护送%s吗?",

	// In the next string, the first %s is a name and the second %s is an amount of money (including $ sign)

	"要雇佣%s吗(每日得支付%s)?",

	// This line is used repeatedly to ask player if they wish to participate in a boxing match.

	"您要进行拳击比赛吗?",

	// In the next string, the first %s is an item name and the
	// second %s is an amount of money (including $ sign)

	"要买%s吗(得支付%s)?",

	// In the next string, %s is a name

	"%s接受第%d小队的护送.",

	// These messages are displayed during play to alert the player to a particular situation

	"卡壳",					//weapon is jammed.
	"机器人需要%s口径的子弹.",	//Robot is out of ammo
	"扔到那儿?那不可能.",		//Merc can't throw to the destination he selected

	// These are different buttons that the player can turn on and off.

	"潜行模式 (|Z)",
	"地图屏幕 (|M)",
	"结束回合 (|D)",
	"谈话",
	"禁音",
	"起身 (|P|g|U|p)",
	"光标层次 (|T|a|b)",
	"攀爬/跳跃",
	"伏下 (|P|g|D|n)",
	"检查",
	"上一佣兵",
	"下一佣兵 (|S|p|a|c|e)",
	"选项 (|O)",
	"扫射模式 (|B)",
	"查看/转向(|L)",
	"生命: %d/%d\n精力: %d/%d\n士气: %s",
	"厄?",		//this means "what?"
	"继续",		//an abbrieviation for "Continued"
	"对%s关闭禁音模式.",
	"对%s打开禁音模式.",
	"耐久度: %d/%d\n油量: %d/%d",
	"下车",
	"切换小队 ( |S|h|i|f|t |S|p|a|c|e )",
	"驾驶",
	"N/A",		//this is an acronym for "Not Applicable."
	"使用 (拳头)",
	"使用 (武器)",
	"使用 (刀具)",
	"使用 (爆炸品)",
	"使用 (医疗用品)",
	"(抓住)",
	"(装填弹药)",
	"(给予)",
	"%s被触发了.",
	"%s已到达.",
	"%s用完了行动点数(AP).",
	"%s无法行动.",
	"%s包扎好了.",
	"%s用完了绷带.",
	"这个区域中有敌军.",
	"视野中无敌军.",
	"没有足够的行动点数(AP).",
	"没人使用遥控器.",
	"扫射光了子弹!",
	"敌兵",
	"异形",
	"民兵",
	"平民",
	"离开区域",
	"确定",
	"取消",
	"选择佣兵",
	"小队的所有佣兵",
	"前往区域",
	"前往地图",
	"您不能从这边离开此区域.",
	"%s太远了.",
	"不显示树冠",
	"显示树冠",
	"乌鸦"	,				//Crow, as in the large black bird
	"颈部",
	"头部",
	"躯体",
	"腿部",
	"要告诉女王她想知道的情报吗?",
	"获得指纹ID",
	"指纹ID无效.无法使用该武器.",
	"达成目标",
	"路被堵住了",
	"存钱/取钱",	//Help text over the $ button on the Single Merc Panel
	"没人需要包扎.",
	"卡壳",											// Short form of JAMMED, for small inv slots
	"无法到达那里.", // used ( now ) for when we click on a cliff
	"那人拒绝移动.",
	// In the following message, '%s' would be replaced with a quantity of money (e.g. $200)
	"您同意支付%s吗?",
	"您要接受免费治疗吗?",
	"您同意让佣兵和Daryl结婚吗?",
	"钥匙面板",
	"您不能这样用EPC.",
	"不杀Krott?",
	"超出武器的有效射程.",
	"矿工",
	"车辆只能在区域间旅行",
	"现在不能自动包扎",
	"%s被堵住了",
	"被Deidranna的军队俘虏的佣兵, 被关押在这里!",
	"锁被击中了",
	"锁被破坏了",
	"其他人在使用这扇门.",
	"耐久度: %d/%d\n油量: %d/%d",
	"%s看不见%s.", // Cannot see person trying to talk to
};

//Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different circumstances in the "exiting sector" interface.
static const ST::string s_cn_pExitingSectorHelpText[pExitingSectorHelpText_SIZE] =
{
	//Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box is checked.
	"如果勾中, 将立即进入邻近的区域.",
	"如果勾中, 您将被立即自动放置在地图屏幕, \n因为您的佣兵要花些时间来行军.",

	//If you attempt to leave a sector when you have multiple squads in a hostile sector.
	"该区域被敌军占据.您不能将佣兵留在这里.\n在进入其它区域前, 您必须把这里的问题解决.",

	//Because you only have one squad in the sector, and the "move all" option is checked, the "go to sector" option is locked to on.
	//The helptext explains why it is locked.
	"让留下的佣兵离开本区域, \n将立即进入邻近的区域.",
	"让留下的佣兵离开本区域, \n您将被立即自动放置在地图屏幕, \n因为您的佣兵要花些时间来行军.",

	//If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being escorted.  The "single" button is disabled.
	"%s需要被您的佣兵护送, 他 (她) 无法独自离开本区域.",

	//If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc will be prohibited from leaving alone.
	//There are several strings depending on the gender of the merc and how many EPCs are in the squad.
	//DO NOT USE THE NEWLINE HERE AS IT IS USED FOR BOTH HELPTEXT AND SCREEN MESSAGES!
	"%s无法独自离开本区域, 因为他得护送%s.", //male singular
	"%s无法独自离开本区域, 因为她得护送%s.", //female singular
	"%s无法独自离开本区域, 因为他得护送多人.", //male plural
	"%s无法独自离开本区域, 因为她得护送多人.", //female plural

	//If one or more of your mercs in the selected squad aren't in range of the traversal area, then the  "move all" option is disabled,
	//and this helptext explains why.
	"如果要让小队在区域间移动的话, \n您的全部队员都必须在附近.",

	//Standard helptext for single movement.  Explains what will happen (splitting the squad)
	"如果勾中,  %s将独自行军, \n而且被自动重新分配到一个单独的小队中.",

	//Standard helptext for all movement.  Explains what will happen (moving the squad)
	"如果勾中, 您当前选中的小队\n将会离开本区域, 开始行军.",

	//This strings is used BEFORE the "exiting sector" interface is created.  If you have an EPC selected and you attempt to tactically
	//traverse the EPC while the escorting mercs aren't near enough (or dead, dying, or unconscious), this message will appear and the
	//"exiting sector" interface will not appear.  This is just like the situation where
	//This string is special, as it is not used as helptext.  Do not use the special newline character (\n) for this string.
	"%s正在被您的佣兵护送, 无法独自离开本区域.您的佣兵必须在附近以护送%s离开.",
};



static const ST::string s_cn_pRepairStrings[pRepairStrings_SIZE] =
{
	"物品",	// tell merc to repair items in inventor
	"SAM基地",		// tell merc to repair SAM site - SAM is an acronym for Surface to Air Missile
	"取消",		// cancel this menu
	"机器人",		// repair the robot
};


// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."

static const ST::string s_cn_sPreStatBuildString[sPreStatBuildString_SIZE] =
{
	"丧失",// the merc has lost a statistic
	"获得",// the merc has gained a statistic
	"点",// singular
	"点",// plural
	"级",//singular
	"级",//plural
};

static const ST::string s_cn_sStatGainStrings[sStatGainStrings_SIZE] =
{
	"生命.",
	"敏捷.",
	"灵巧.",
	"智慧.",
	"医疗技能.",
	"爆破技能.",
	"机械技能.",
	"枪法技能.",
	"级别",
	"力量",
	"领导",
};


static const ST::string s_cn_pHelicopterEtaStrings[pHelicopterEtaStrings_SIZE] =
{
	"总距离: ", 			// total distance for helicopter to travel
	"安全: ", 			// distance to travel to destination
	"不安全: ",		// distance to return from destination to airport
	"总价: ", 		// total cost of trip by helicopter
	"耗时: ", 			// ETA is an acronym for "estimated time of arrival"
	"直升机油量不够, 必须在敌占区着陆.", 	// warning that the sector the helicopter is going to use for refueling is under enemy control ->
	"乘客: ",
	"选择Skyrider还是 '着陆点'?",
	"Skyrider",
	"着陆点",
};

static const ST::string s_cn_sMapLevelString = "地层:"; // what level below the ground is the player viewing in mapscreen

static const ST::string s_cn_gsLoyalString = "%d%% 忠诚"; // the loyalty rating of a town ie : Loyal 53%


// error message for when player is trying to give a merc a travel order while he's underground.
static const ST::string s_cn_gsUndergroundString = "不能在地下行军.";

static const ST::string s_cn_gsTimeStrings[gsTimeStrings_SIZE] =
{
	"h",				// hours abbreviation
	"m",				// minutes abbreviation
	"s",				// seconds abbreviation
	"d",				// days abbreviation
};

// text for the various facilities in the sector

static const ST::string s_cn_sFacilitiesStrings[sFacilitiesStrings_SIZE] =
{
	"无",
	"医院",
	"工厂",
	"监狱",
	"军事基地",
	"机场",
	"靶场",		// a field for soldiers to practise their shooting skills
};

// text for inventory pop up button

static const ST::string s_cn_pMapPopUpInventoryText[pMapPopUpInventoryText_SIZE] =
{
	"存货",
	"离开",
};

// town strings

static const ST::string s_cn_pwTownInfoStrings[pwTownInfoStrings_SIZE] =
{
	"大小",				// 0 // size of the town in sectors
	"占领度",				// how much of town is controlled
	"矿区",			// mine associated with this town
	"忠诚度",				// 5 // the loyalty level of this town
	"主要设施",			// main facilities in this town
	"民兵训练度",		// 10 // state of civilian training in town
	"民兵",			// the state of the trained civilians in the town
};

// Mine strings

static const ST::string s_cn_pwMineStrings[pwMineStrings_SIZE] =
{
	"矿井",			// 0
	"银块",
	"金块",
	"当前日产量",
	"最高产量",
	"废弃",				// 5
	"关闭",
	"矿脉耗尽",
	"生产",
	"状态",
	"生产率",
	"矿石类型",			// 10
	"占领度",
	"忠诚度",
};

// blank sector strings

static const ST::string s_cn_pwMiscSectorStrings[pwMiscSectorStrings_SIZE] =
{
	"敌军",
	"区域",
	"物品数量",
	"未知",
	"已占领",
	"是",
	"否",
};

// error strings for inventory

static const ST::string s_cn_pMapInventoryErrorString[pMapInventoryErrorString_SIZE] =
{
	"无法选择该佣兵.",  //MARK CARTER
	"%s不在此区域, 不能拿到该物品.",
	"在战斗时, 您只能动手捡起物品.",
	"在战斗时, 您只能动手放下物品.",
	"%s不在该区域, 不能放下那个物品.",
};

static const ST::string s_cn_pMapInventoryStrings[pMapInventoryStrings_SIZE] =
{
	"位置",		// sector these items are in
	"物品总数", 		// total number of items in sector
};


// movement menu text

static const ST::string s_cn_pMovementMenuStrings[pMovementMenuStrings_SIZE] =
{
	"在%s区域调动佣兵", 	// title for movement box
	"安排行军路线",	// done with movement menu, start plotting movement
	"取消",		// cancel this menu
	"其它",		// title for group of mercs not on squads nor in vehicles
};


static const ST::string s_cn_pUpdateMercStrings[pUpdateMercStrings_SIZE] =
{
	"糟了: ", 			// an error has occured
	"佣兵合同到期了: ", 	// this pop up came up due to a merc contract ending
	"佣兵完成了分配的任务: ", // this pop up....due to more than one merc finishing assignments
	"佣兵醒来了, 继续干活: ", // this pop up ....due to more than one merc waking up and returing to work
	"佣兵困倦了: ", // this pop up ....due to more than one merc being tired and going to sleep
	"合同快到期了: ", 	// this pop up came up due to a merc contract ending
};

// map screen map border buttons help text

static const ST::string s_cn_pMapScreenBorderButtonHelpText[pMapScreenBorderButtonHelpText_SIZE] =
{
	"显示城镇 (|W)",
	"显示矿井 (|M)",
	"显示队伍和敌人 (|T)",
	"显示领空 (|A)",
	"显示物品 (|I)",
	"显示民兵和敌人 (|Z)",
};


static const ST::string s_cn_pMapScreenBottomFastHelp[pMapScreenBottomFastHelp_SIZE] =
{
	"笔记本电脑 (|L)",
	"战术屏幕 (|E|s|c)",
	"选项 (|O)",
	"时间压缩 (|+)", 	// time compress more
	"时间压缩 (|-)", 	// time compress less
	"上一信息 (|U|p)\n上页 (|P|g|U|p)", // previous message in scrollable list
	"下一信息 (|D|o|w|n)\n下页 (|P|g|D|n)", 	// next message in the scrollable list
	"开始/停止时间压缩 (|S|p|a|c|e)",	// start/stop time compression
};

static const ST::string s_cn_pMapScreenBottomText =	"帐户余额"; // current balance in player bank account
static const ST::string s_cn_pMercDeadString = "%s阵亡.";
static const ST::string s_cn_pDayStrings = "日";

// the list of email sender names

static const ST::string s_cn_pSenderNameList[pSenderNameList_SIZE] =
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
	"Stephen",
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
	"M.I.S. Insurance",
	"Bobby Rays",
	"Kingpin",
	"John Kulba",
	"A.I.M.",
};


// new mail notify string
static const ST::string s_cn_pNewMailStrings = "您有新的邮件...";


// confirm player's intent to delete messages

static const ST::string s_cn_pDeleteMailStrings[pDeleteMailStrings_SIZE] =
{
	"删除邮件?",
	"删除未读的邮件?",
};


// the sort header strings

static const ST::string s_cn_pEmailHeaders[pEmailHeaders_SIZE] =
{
	"来自: ",
	"标题: ",
	"日期: ",
};

// email titlebar text
static const ST::string s_cn_pEmailTitleText =
	"邮箱";


// the financial screen strings
static const ST::string s_cn_pFinanceTitle =
	"Bookkeeper Plus"; // the name we made up for the financial program in the game

static const ST::string s_cn_pFinanceSummary[pFinanceSummary_SIZE] =
{
	"收入: ", 				// credit (subtract from) to player's account
	"支出: ",				// debit (add to) to player's account
	"昨日实际收入: ",
	"昨日其它存款: ",
	"昨日支出: ",
	"昨日日终余额: ",
	"今日实际收入: ",
	"今日其它存款: ",
	"今日支出: ",
	"今日当前余额: ",
	"预期收入: ",
	"明日预计余额: ", 		// projected balance for player for tommorow
};


// headers to each list in financial screen

static const ST::string s_cn_pFinanceHeaders[pFinanceHeaders_SIZE] =
{
	"天数",			// the day column
	"收入", 			// the credits column (to ADD money to your account)
	"支出",				// the debits column (to SUBTRACT money from your account)
	"交易记录", // transaction type - see TransactionText below
	"余额",		// balance at this point in time
	"页数",				// page number
	"日", 			// the day(s) of transactions this page displays
};


static const ST::string s_cn_pTransactionText[pTransactionText_SIZE] =
{
	"自然增值利息",		// interest the player has accumulated so far
	"匿名存款",
	"交易费用",
	"已雇佣",			// Merc was hired
	"在Bobby Ray购买货品", // Bobby Ray is the name of an arms dealer
	"在M.E.R.C开户.",
	"%s的医疗保证金",  	// medical deposit for merc
	"IMP心理剖析分析", 		// IMP is the acronym for International Mercenary Profiling
	"为%s购买保险",
	"缩短%s的保险期限",
	"延长%s的保险期限",			// johnny contract extended
	"取消%s的保险",
	"%s的保险索赔",	// insurance claim for merc
	"1日", 				// merc's contract extended for a day
	"1周",				// merc's contract extended for a week
	"2周",				// ... for 2 weeks
	"矿井收入",
	"", //String nuked
	"买花",
	"%s的医疗保证金的全额退款",
	"%s的医疗保证金的部分退款",
	"%s的医疗保证金无退款",
	"付给%s金钱",// %s is the name of the npc being paid
	"支付给%s的佣金", 	// transfer funds to a merc
	"%s退回的佣金",	 // transfer funds from a merc
	"在%s训练民兵",	 // initial cost to equip a town's militia
	"向%s购买了物品.", //is used for the Shop keeper interface.  The dealers name will be appended to the end of the string.
	"%s存款",
};

// helicopter pilot payment

static const ST::string s_cn_pSkyriderText[pSkyriderText_SIZE] =
{
	"付给 Skyrider $%d",	// skyrider was paid an amount of money
	"还欠 Skyrider $%d",	// skyrider is still owed an amount of money
	"Skyrider没有乘客. 如果您试图运送此区域的佣兵, 首先要分配他们进入 '交通工具' －> '直升机'.",
};


// strings for different levels of merc morale

static const ST::string s_cn_pMoralStrings[pMoralStrings_SIZE] =
{
	"高涨",
	"良好",
	"稳定",
	"低下",
	"惊慌",
	"糟糕",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.
static const ST::string s_cn_str_left_equipment   =
	"%s的装备现在可以在%s (%c%d)获得.";

// Status that appears on the Map Screen

static const ST::string s_cn_pMapScreenStatusStrings[pMapScreenStatusStrings_SIZE] =
{
	"生命",
	"精力",
	"士气",
	"状态",	// the condition of the current vehicle (its "health")
	"油量",	// the fuel level of the current vehicle (its "energy")
};


static const ST::string s_cn_pMapScreenPrevNextCharButtonHelpText[pMapScreenPrevNextCharButtonHelpText_SIZE] =
{
	"上一佣兵 (|L|e|f|t)",	//"Previous Merc (|L|e|f|t)", 			// previous merc in the list
	"下一佣兵 (|R|i|g|h|t)", 				// next merc in the list
};


static const ST::string s_cn_pEtaString =
	"耗时:"; // eta is an acronym for Estimated Time of Arrival

static const ST::string s_cn_pTrashItemText[pTrashItemText_SIZE] =
{
	"您将不会再见到它了.您确定吗?",	 	// do you want to continue and lose the item forever
	"此物品看起来非常重要.您真的非常非常确定要扔掉它吗?", // does the user REALLY want to trash this item
};


static const ST::string s_cn_pMapErrorString[pMapErrorString_SIZE] =
{
	"小队不能行军, 因为有人在睡觉.",

//1-5
	"首先要回到地面, 才能移动小队.",
	"行军?那里是敌占区!",
	"必须给佣兵分配小队或者交通工具才能开始行军.",
	"您现在没有任何队员.", 	// you have no members, can't do anything
	"佣兵无法遵从命令.",		// merc can't comply with your order
//6-10
	"需要有人护送才能行军.请把他分进一个小队里.", // merc can't move unescorted .. for a male
	"需要有人护送才能行军.请把她分进一个小队里.", // for a female
	"佣兵尚未到达Arulco!",
	"看来得先谈妥合同.",
	"无法发出行军命令.目前有空袭.",
//11-15
	"行军? 这里正在战斗中!",
	"您在区域%s被血猫伏击了!",
	"您刚刚进入了 %s 区域, 这里是血猫的巢穴!", // HEADROCK HAM 3.6: Added argument.
	"",
	"在%s的SAM基地被敌军占领了.",
//16-20
	"在%s的矿井被敌军占领了.您的日收入下降为每日%s.",
	"敌军未遭到抵抗, 就占领了%s",
	"您至少有一名佣兵不能被分配此任务.",
	"%s无法加入%s, 因为它已经满员了",
	"%s无法加入%s, 因为它太远了.",
//21-25
	"在%s的矿井被敌军占领了!",
	"敌军入侵了%s处的SAM基地",
	"敌军入侵了%s",
	"敌军在%s出没.",
	"敌军占领了%s.",
//26-30
	"您的佣兵中至少有一人不能睡眠.",
	"您的佣兵中至少有一人不能醒来.",
	"训练完毕, 才会出现民兵.",
	"现在无法对%s下达行军命令.",
	"不在城镇边界的民兵无法行军到另一个区域.",
//31-35
	"您不能在%s拥有民兵.",
	"车是空的, 无法移动!",
	"%s受伤太严重了, 无法行军!",
	"您必须首先离开博物馆!",
	"%s死了!",
//36-40
	"%s无法转到%s因为它在移动中",
	"%s无法那样进入交通工具",
	"%s无法加入%s",
	"在您雇佣新的佣兵前, 您不能压缩时间.",
	"车辆只能在公路上开!",
//41-45
	"在佣兵移动时, 您不能重新分配任务",
	"车辆没油了!",
	"%s太累了, 以致不能行军.",
	"车上无人能够驾驶.",
	"该小队的佣兵现在不能移动.",
//46-50
	"其他佣兵现在不能移动.",
	"车辆被损坏得太严重了!",
	"每个区域只能由两名佣兵来训练民兵.",
	"没有遥控员, 机器人无法移动.请把他们分配在同一个小队.",
};


// help text used during strategic route plotting
static ST::string s_cn_pMapPlotStrings[pMapPlotStrings_SIZE] =
{
	"再点击一下目的地以确认您的最后路线, 或者点击下一个区域以设置更多的路点.",
	"行军路线已确认.",
	"目的地未改变.",
	"行军路线已取消.",
	"行军路线已缩短.",
};


// help text used when moving the merc arrival sector
static const ST::string s_cn_pBullseyeStrings[pBullseyeStrings_SIZE] =
{
	"点击您想让佣兵着陆的区域.",
	"好的.佣兵将在%s着陆",
	"佣兵不能飞往那里, 领空不安全!",
	"取消.着陆区域未改变",
	"%s上的领空现在不安全了!着陆区域被改为%s.",
};


// help text for mouse regions

static const ST::string s_cn_pMiscMapScreenMouseRegionHelpText[pMiscMapScreenMouseRegionHelpText_SIZE] =
{
	"进入存货屏幕 (|E|n|t|e|r)",
	"扔掉物品",	//"Throw Item Away",
	"离开存货屏幕 (|E|n|t|e|r)",
};


static const ST::string s_cn_str_he_leaves_where_drop_equipment  =
	"让%s把装备留在他所在的(%s), 离开Arulco, 或者留在%s (%s), 飞离Arulco?";
static const ST::string s_cn_str_she_leaves_where_drop_equipment =
	"让%s把装备留在她所在的(%s), 离开Arulco, 或者留在%s (%s), 飞离Arulco?";
static const ST::string s_cn_str_he_leaves_drops_equipment       =
	"%s要离开了, 他的装备将被留在%s.";
static const ST::string s_cn_str_she_leaves_drops_equipment      =
	"%s要离开了, 她的装备将被留在%s.";


// Text used on IMP Web Pages

static const ST::string s_cn_pImpPopUpStrings[pImpPopUpStrings_SIZE] =
{
	"无效的授权号",
	"您试图重新开始整个测试.您确定吗?",
	"请输入正确的全名和性别.",
	"对您的财政状况的预先分析显示了您无法负担心理剖析的费用.",
	"现在不是个有效的选择.",
	"要进行心理剖析, 您的队伍中必须至少留一个空位.",
	"测试完毕.",
};


// button labels used on the IMP site

static const ST::string s_cn_pImpButtonText[pImpButtonText_SIZE] =
{
	"关于我们",			// about the IMP site
	"开始",		// begin profiling
	"性格",		// personality section
	"属性", 		// personal stats/attributes section
	"外表", 			// changed from portrait - SANDRO
	"嗓音%d",		// the voice selection
	"完成",		// done profiling
	"重新开始",	// start over profiling
	"是的, 我选择了高亮突出的回答.",
	"是",
	"否",
	"完成",			// finished answering questions
	"上一个",			// previous question..abbreviated form
	"下一个", 			// next question
	"是的, 我确定.",	// yes, I am certain
	"不, 我想重新开始.",
	"是",
	"否",
	"后退",		// back one page
	"取消",		// cancel selection
	"是的, 我确定.",
	"不, 让我再看看.",
	"注册",			// the IMP site registry..when name and gender is selected
	"分析中", 		// analyzing your profile results
	"完成",
	"嗓音",
};

static const ST::string s_cn_pExtraIMPStrings[pExtraIMPStrings_SIZE] =
{
	"选择性格, 开始实际分析.",
	"既然您已经完成了性格选择, 下面选择您的能力.",
	"能力确认完毕, 您可以继续选择头像.",
	"选择最符合您的声音, 完成整个过程.",
};

static const ST::string s_cn_gzIMPSkillTraitsText[gzIMPSkillTraitsText_SIZE] =
{
		s_cn_gzMercSkillText[1],
		"徒手格斗", //"Hand to hand combat",
		s_cn_gzMercSkillText[3],
		"夜间行动", //"Night operations",
		s_cn_gzMercSkillText[5],
		s_cn_gzMercSkillText[6],
		s_cn_gzMercSkillText[7],
		s_cn_gzMercSkillText[8],
		"潜行", //"Stealth",
		s_cn_gzMercSkillText[10],
		s_cn_gzMercSkillText[13],
		"屋顶狙击", //"Rooftop Sniping",
		"伪装", //"Camouflage",
		s_cn_gzMercSkillText[12],

		"None",
		"I.M.P 专长", //"I.M.P. Specialties",
};

static const ST::string s_cn_pFilesTitle =
	"File Viewer";

static const ST::string s_cn_pFilesSenderList[pFilesSenderList_SIZE] =
{
	"侦察报告",
	"1号通缉令",
	"2号通缉令",
	"3号通缉令",
	"4号通缉令",
	"5号通缉令",
	"6号通缉令", // sixth intercept file
};

// Text having to do with the History Log
static const ST::string s_cn_pHistoryTitle =
	"历史记录";

static const ST::string s_cn_pHistoryHeaders[pHistoryHeaders_SIZE] =
{
	"日",		// the day the history event occurred
	"页",	// the current page in the history report we are in
	"日",	// the days the history report occurs over
	"位置",	// location (in sector) the event occurred
	"事件",	// the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
static const ST::string s_cn_pHistoryStrings[pHistoryStrings_SIZE] =
{
	"",																						// leave this line blank
	//1-5
	"从A.I.M雇佣了%s.",
	"从M.E.R.C雇佣了%s.",
	"%s死了.",	//"%s died.", 															// merc was killed
	"在M.E.R.C开户.",
	"接受Enrico Chivaldori的委托",	//"Accepted Assignment From Enrico Chivaldori",
	//6-10
	"IMP已生成",
	"为%s购买保险.",
	"取消%s的保险合同.",
	"收到%s的保险索赔.",
	"延长一日%s的合同.",
	//11-15
	"延长一周%s的合同.",
	"延长两周%s的合同.",
	"%s被解雇了.",
	"%s退出了.",
	"开始任务.",
	//16-20
	"完成任务.",
	"和%s的矿主交谈",
	"解放了%s",
	"启用作弊",
	"食物会在明天送达Omerta",
	//21-25
	"%s离队并成为了Daryl Hick的妻子",
	"%s的合同到期了.",
	"招募了%s.",
	"Enrico抱怨进展缓慢",
	"战斗胜利",
	//26-30
	"%s的矿井开始缺乏矿石",
	"%s的矿井采完了矿石",
	"%s的矿井关闭了",	//"%s mine was shut down",
	"%s的矿井复工了",
	"发现一个叫Tixa的监狱.",
	//31-35
	"打听到一个叫Orta的秘密武器工厂.",
	"在Orta的科学家捐赠了大量的火箭枪.",
	"Deidranna女王在利用死尸做某些事情.",
	"Frank谈到了在San Mona的拳击比赛.",
	"一个病人说他在矿井里看到了一些东西.",
	//36-40
	"遇到一个叫Devin的人, 他出售爆炸物.",
	"偶遇Mike, 前AIM名人!",
	"遇到Tony, 他做武器买卖.",
	"从Krott中士那里得到一把火箭枪.",
	"把Angel皮衣店的契约给了Kyle.",
	//41-45
	"Madlab提议做一个机器人.",
	"Gabby能制作对付虫子的隐形药.",
	"Keith歇业了.",
	"Howard给Deidranna女王提供氰化物.",
	"遇到Keith -Cambria的杂货商.",
	//46-50
	"遇到Howrd, 一个在Balime的医药商.",
	"遇到Perko, 他开了一家小修理档口..",
	"遇到在Balime的Sam, 他有一家五金店.",
	"Franz做电子产品和其他货物的生意.",
	"Arnold在Grumm开了一家修理店.",
	//51-55
	"Fredo在Grumm修理电子产品.",
	"收到在Balime的有钱人的捐款.",
	"遇到一个叫Jake的废品商人.",
	"一个流浪者给了我们一张电子钥匙卡.",
	"贿赂了Walter, 让他打开地下室的门.",
	//56-60
	"如果Dave有汽油, 他会免费进行加油.",
	"贿赂Pablo.",
	"Kingping拿回了San Mona矿井中的钱.",
	"%s赢了拳击赛",
	"%s输了拳击赛",
	//61-65
	"%s丧失了拳击赛的参赛资格",
	"在废弃的矿井里找到一大笔钱.",
	"遭遇Kingpin派出的杀手.",
	"该区域失守",	//ENEMY_INVASION_CODE
	"成功防御该区域",
	//66-70
	"作战失败",			//ENEMY_ENCOUNTER_CODE
	"致命的伏击",		//ENEMY_AMBUSH_CODE
	"杀光了敌军的伏兵",
	"攻击失败",		//ENTERING_ENEMY_SECTOR_CODE
	"攻击成功!",
	//71-75
	"异形攻击",		//CREATURE_ATTACK_CODE
	"被血猫吃掉了",	//BLOODCAT_AMBUSH_CODE
	"宰掉了血猫",
	"%s被干掉了",
	"把一个恐怖分子的头颅给了Carmen.",
	"Slay走了",
	"干掉了%s",
};

static const ST::string s_cn_pHistoryLocations =
	"N/A"; // N/A is an acronym for Not Applicable

// icon text strings that appear on the laptop

static const ST::string s_cn_pLaptopIcons[pLaptopIcons_SIZE] =
{
	"邮箱",
	"网页",
	"财务",
	"人事",
	"日志",
	"文件",
	"关闭",
	"sir-FER 4.0",			// our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added

static const ST::string s_cn_pBookMarkStrings[pBookMarkStrings_SIZE] =
{
	"A.I.M.",
	"Bobby Ray's",
	"I.M.P",
	"M.E.R.C.",
	"公墓",
	"花店",
	"M.I.S 保险公司",
	"取消",
};

// When loading or download a web page

static const ST::string s_cn_pDownloadString[pDownloadString_SIZE] =
{
	"下载中",
	"重载中",
};

//This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

static const ST::string s_cn_gsAtmStartButtonText[gsAtmStartButtonText_SIZE] =
{
	"状态",			// view stats of the merc
	"物品清单", 			// view the inventory of the merc
	"雇佣合同",
};

// Web error messages. Please use foreign language equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator

static const ST::string s_cn_pErrorStrings =
	"主机连接时断时续.预计需要较长的传输时间.";


static const ST::string s_cn_pPersonnelString =
	"佣兵:"; // mercs we have


static const ST::string s_cn_pWebTitle =
	"sir-FER 4.0"; // our name for the version of the browser, play on company name


// The titles for the web program title bar, for each page loaded

static const ST::string s_cn_pWebPagesTitles[pWebPagesTitles_SIZE] =
{
	"A.I.M.",
	"A.I.M. 成员",
	"A.I.M. 肖像",		// a mug shot is another name for a portrait
	"A.I.M. 排序",
	"A.I.M.",
	"A.I.M. 前成员",
	"A.I.M. 规则",
	"A.I.M. 历史",
	"A.I.M. 链接",
	"M.E.R.C.",
	"M.E.R.C. 账号",
	"M.E.R.C. 注册",
	"M.E.R.C. 索引",
	"Bobby Ray's",
	"Bobby Ray's - 枪械",
	"Bobby Ray's - 弹药",
	"Bobby Ray's - 护甲",
	"Bobby Ray's - 杂货",	//"Bobby Ray's - Misc",							//misc is an abbreviation for miscellaneous
	"Bobby Ray's - 二手货",
	"Bobby Ray's - 邮购",
	"I.M.P.",
	"I.M.P.",
	"联合花卉服务公司",
	"联合花卉服务公司 - 花卉",
	"联合花卉服务公司 - 订单",
	"联合花卉服务公司 - 贺卡",
	"Malleus, Incus & Stapes 保险公司",
	"信息",
	"合同",
	"评论",
	"McGillicutty's 公墓",
	"无法找到UR",
	"Bobby Ray's - 最近的运货",
	"",
	"",
};

static const ST::string s_cn_pShowBookmarkString[pShowBookmarkString_SIZE] =
{
	"Sir-Help",
	"再次点击页面以放进收藏夹.",
};

static const ST::string s_cn_pLaptopTitles[pLaptopTitles_SIZE] =
{
	"邮箱",
	"File Viewer",
	"人事",
	"Bookkeeper Plus",
	"历史日志",
};

static const ST::string s_cn_pPersonnelDepartedStateStrings[pPersonnelDepartedStateStrings_SIZE] =
{
	//reasons why a merc has left.
	"阵亡",
	"解雇",
	"结婚",
	"合同到期",
	"退出",
};
// personnel strings appearing in the Personnel Manager on the laptop

static const ST::string s_cn_pPersonelTeamStrings[pPersonelTeamStrings_SIZE] =
{
	"当前成员: ",
	"离队成员: ",
	"每日花费: ",
	"最高日薪: ",
	"最低日薪: ",
	"阵亡: ",
	"解雇: ",
	"其它: ",
};


static const ST::string s_cn_pPersonnelCurrentTeamStatsStrings[pPersonnelCurrentTeamStatsStrings_SIZE] =
{
	"最低",
	"平均",
	"最高",
};


static const ST::string s_cn_pPersonnelTeamStatsStrings[pPersonnelTeamStatsStrings_SIZE] =
{
	"生命",
	"敏捷",
	"灵巧",
	"力量",
	"领导",
	"智慧",
	"级别",
	"枪法",
	"机械",
	"爆破",
	"医疗",
};


// horizontal and vertical indices on the map screen

static const ST::string s_cn_pMapVertIndex[pMapVertIndex_SIZE] =
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
	"",
	"M",
	"N",
	"O",
	"P",
};

static const ST::string s_cn_pMapHortIndex[pMapHortIndex_SIZE] =
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

static const ST::string s_cn_pMapDepthIndex[pMapDepthIndex_SIZE] =
{
	"",
	"-1",
	"-2",
	"-3",
};

// text that appears on the contract button

static const ST::string s_cn_pContractButtonString =
	"合同";

// text that appears on the update panel buttons

static const ST::string s_cn_pUpdatePanelButtons[pUpdatePanelButtons_SIZE] =
{
	"继续",
	"停止",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle

static ST::string s_cn_LargeTacticalStr[LargeTacticalStr_SIZE] =
{
	"您在这个地区战败了!",
	"敌人冷酷无情地处死了您的队员!",
	"您昏迷的队员被俘虏了!",
	"您的队员成了敌军的俘虏.",
};


//Insurance Contract.c
//The text on the buttons at the bottom of the screen.

static const ST::string s_cn_InsContractText[InsContractText_SIZE] =
{
	"上页",
	"下页",
	"接受",
	"清除",
};



//Insurance Info
// Text on the buttons on the bottom of the screen

static const ST::string s_cn_InsInfoText[InsInfoText_SIZE] =
{
	"上页",
	"下页",
};



//For use at the M.E.R.C. web site. Text relating to the player's account with MERC

static const ST::string s_cn_MercAccountText[MercAccountText_SIZE] =
{
	// Text on the buttons on the bottom of the screen
	"支付",
	"主页",
	"账号 #: ",
	"佣兵",
	"日数",
	"日薪",	//5
	"索价",
	"合计: ",
	"您确定要支付%s吗?",		//the %s is a string that contains the dollar amount ( ex. "$150" )
};



//For use at the M.E.R.C. web site. Text relating a MERC mercenary


static const ST::string s_cn_MercInfo[MercInfo_SIZE] =
{
	"上一位",	//"Previous",
	"雇佣",	//"Hire",
	"下一位",	//"Next",
	"附加信息",	//"Additional Info",
	"主页",	//"Home",
	"已雇佣",	//"Hired",
	"日薪: ",	//"Salary:",
	"每日",		//"Per Day",
	"阵亡",	//"Deceased",

	"看起来您雇佣了太多的佣兵.最多只能雇18人.",	//"Looks like you're trying to hire too many mercs. Your limit is 18.",
	"不可雇佣",
};



// For use at the M.E.R.C. web site. Text relating to opening an account with MERC

static const ST::string s_cn_MercNoAccountText[MercNoAccountText_SIZE] =
{
	//Text on the buttons at the bottom of the screen
	"开户",	//"Open Account",
	"取消",	//"Cancel",
	"您没有帐户, 注册一个吗?",
};



// For use at the M.E.R.C. web site. MERC Homepage

static const ST::string s_cn_MercHomePageText[MercHomePageText_SIZE] =
{
	//Description of various parts on the MERC page
	"Speck T. Kline, 创办人和拥有者",
	"开户点击这里",	//"To open an account press here",
	"查看帐户点击这里",	//"To view account press here",
	"查看文件点击这里",	//"To view files press here",
	// The version number on the video conferencing system that pops up when Speck is talking
	"Speck Com v3.2",
};

// For use at MiGillicutty's Web Page
static const ST::string s_cn_sFuneralString[sFuneralString_SIZE] =
{
	"McGillicutty公墓: 1983开业, 办理家庭悼念业务.",
	"葬礼部经理兼A.I.M.前佣兵 Murray \"Pops\" McGillicutty是一名经验丰富, 业务熟练的殡仪业者.",
	"Pops跟死亡和葬礼打了一辈子交道, 他非常熟悉该业务.",
	"McGillicutty公墓提供各种各样的悼念服务: 从可以依靠着哭泣的肩膀到对严重变形的遗体做美容美体服务.",
	"McGillicutty公墓是您所爱的人的安息地.",

	// Text for the various links available at the bottom of the page
	"献花",
	"骨灰盒",
	"火葬服务",
	"安排葬礼",
	"葬礼规则",

	// The text that comes up when you click on any of the links ( except for send flowers ).
	"很抱歉, 由于家里有人去世, 本网站的剩余部分尚未完成.一旦解决了宣读遗嘱和财产分配问题, 本网站会尽快建设好.",
	"很抱歉, 但现在还是测试期间, 请以后再来访问.",
};

// Text for the florist Home page

static const ST::string s_cn_sFloristText[sFloristText_SIZE] =
{
	//Text on the button on the bottom of the page

	"花廊",

	//Address of United Florist

	"\"全球送货上门\"",
	"1-555-SCENT-ME",
	"333 NoseGay大夫, Seedy City, 美国加州 90210",
	"http://www.scent-me.com",

	// detail of the florist page

	"我们快速高效!",
	"保证把鲜花在第二天送到世界上大部分地区, 但有少量限制.",
	"保证价格是世界上最低廉的!",
	"向我们反应比我们价格更低的送花服务广告, 我们会送您一打绝对免费的玫瑰.",
	"自从1981年来, 我们送植物, 送动物, 送鲜花.",
	"我们雇请了被颁发过勋章的前轰炸机飞行员, 他们能把您的鲜花空投在指定位置的十英里半径内. 总是这样 - 每次这样!",
	"让我们满足您对鲜花的品位.",
	"让Bruce - 我们的世界闻名的花卉设计师 - 从我们的花房里为您亲手摘取最新鲜, 最优质的花束.",
	"还有请记住, 如果我们没有您要的花, 我们能种出来 - 很快!",
};



//Florist OrderForm

static const ST::string s_cn_sOrderFormText[sOrderFormText_SIZE] =
{
	//Text on the buttons

	"后退",	//"Back",
	"发送",	//"Send",
	"清除",	//"Clear",
	"花廊",	//"Gallery",

	"花卉名称: ",			//"Name of Bouquet:",
	"价格: ",			//"Price:",//5
	"订单号: ",			//"Order Number:",
	"送货日期",			//"Delivery Date",
	"第二天",			//"next day",
	"慢慢送去",			//"gets there when it gets there",
	"送货目的地",			//"Delivery Location",//10
	"额外服务",			//"Additional Services",
	"变形的花卉($10)",		//"Crushed Bouquet($10)",
	"黑玫瑰($20)",			//"Black Roses($20)",
	"枯萎的花卉($10)",		//"Wilted Bouquet($10)",
	"水果蛋糕(如果有的话)($10)",	//"Fruit Cake (if available)($10)",	//15
	"私人密语: ",			//"Personal Sentiments:",
	"您写的话不能多于75字.",
	"...或者选择我们提供的",	//"...or select from one of our",

	"标准贺卡",			//"STANDARDIZED CARDS",
	"传单信息",			//"Billing Information",	//20

	//The text that goes beside the area where the user can enter their name

	"姓名: ",				//"Name:",
};




//Florist Gallery.c

static const ST::string s_cn_sFloristGalleryText[sFloristGalleryText_SIZE] =
{
	//text on the buttons

	"上页",	//abbreviation for previous
	"下页",	//abbreviation for next

	"点击您想要订购的花卉.",
	"请注意: 为了防止运输中的枯萎和变形, 每束花另收$10包装费.",

	//text on the button

	"主页",	//"Home",
};

//Florist Cards

static const ST::string s_cn_sFloristCards[sFloristCards_SIZE] =
{
	"请点击您想要订购的贺卡",	//"Click on your selection",
	"后退",			//"Back",
};



// Text for Bobby Ray's Mail Order Site

static const ST::string s_cn_BobbyROrderFormText[BobbyROrderFormText_SIZE] =
{
	"订单",		//"Order Form",		//Title of the page
	"数量",			//"Qty",			// The number of items ordered
	"重量 (%s)",		//"Weight (%s)",	// The weight of the item
	"物品名称",		//"Item Name",		// The name of the item
	"物品单价",		//"Unit Price",			// the item's weight
	"总价",		//"Total",		//5	// The total price of all of items of the same type
	"全部物品总价",		//"Sub-Total",		// The sub total of all the item totals added
	"运费 (视目的地而定)",	//	"S&H (See Delivery Loc.)",	// S&H is an acronym for Shipping and Handling
	"全部费用",		//"Grand Total",		// The grand total of all item totals + the shipping and handling
	"送货目的地",	//"Delivery Location",
	"运输速度",	//"Shipping Speed",		//10	// See below
	"运费(每%s)",	//"Cost (per %s.)",		// The cost to ship the items
	"连夜速递",	//"Overnight Express",		// Gets deliverd the next day
	"2工作日",	//"2 Business Days",		// Gets delivered in 2 days
	"标准服务",	//"Standard Service",		// Gets delivered in 3 days
	"清除订单",		//"Clear Order",	//15// Clears the order page
	"确认订单",	//"Accept Order",		// Accept the order
	"后退",		//"Back",	// text on the button that returns to the previous page
	"主页",		//"Home",	// Text on the button that returns to the home page
	"*代表二手货",	//"* Denotes Used Items",	// Disclaimer stating that the item is used
	"您无法支付所需费用.",	//"You can't afford to pay for this.",		//20	// A popup message that to warn of not enough money
	"<无>",		//"<NONE>",		// Gets displayed when there is no valid city selected
	"您确定要把该订单里订购的物品送往%s吗?",	//"Are you sure you want to send this order to %s?",	// A popup that asks if the city selected is the correct one
	"包裹重量**",	//"Package Weight**",		// Displays the weight of the package
	"** 最小重量: ",		//"** Min. Wt.",			// Disclaimer states that there is a minimum weight for the package
	"运货",		//"Shipments",
};


// This text is used when on the various Bobby Ray Web site pages that sell items

static const ST::string s_cn_BobbyRText[BobbyRText_SIZE] =
{
	"订购",				//"To Order",	// Title
	// instructions on how to order
	"请点击该物品.如果要订购多件, 请连续点击.右击以减少要订购的数量.一旦选好了您要订购的, 请前往订单页面.",

	//Text on the buttons to go the various links

	"上页",	//"Previous Items",		//
	"枪械",	//"Guns", 			//3
	"弹药",	//"Ammo",			//4
	"护甲",	//"Armor",			//5
	"其他",	//"Misc.",			//6	//misc is an abbreviation for miscellaneous
	"二手货",	//"Used",			//7
	"下页",	//"More Items",
	"订货单",	//"ORDER FORM",
	"主页",	//"Home",			//10

	//The following lines provide information on the items

	"重量: ",	//"Weight:",		// Weight of all the items of the same type
	"口径: ",	//"Cal:",			// the caliber of the gun
	"载弹量: ",	//"Mag:",			// number of rounds of ammo the Magazine can hold
	"射程: ",	//"Rng:",			// The range of the gun
	"杀伤力: ",	//"Dam:",			// Damage of the weapon
	"射速: ",	//"ROF:",			// Weapon's Rate Of Fire, acronym ROF
	"单价: ",	//"Cost:",			// Cost of the item
	"库存: ",	//"In stock:",			// The number of items still in the store's inventory
	"购买量: ",	//"Qty on Order:",		// The number of items on order
	"已损坏",	//"Damaged",			// If the item is damaged
	"小计: ",	//"SubTotal:",			// The total cost of all items on order
	"* %％ 可用",	//"* %% Functional",		// if the item is damaged, displays the percent function of the item

	//Popup that tells the player that they can only order 10 items at a time

	"唉! 我们这里的在线订单一次只接受10件物品的订购.如果您想要订购更多商品 (我们希望如此), 请接受我们的歉意, 再开一份订单.",

	// A popup that tells the user that they are trying to order more items then the store has in stock

	"抱歉, 我们只有这么多, 请稍后再来订购.",

	//A popup that tells the user that the store is temporarily sold out

	"抱歉, 现在缺货.",

};


// The following line is used on the Ammunition page.  It is used for help text
// to display how many items the player's merc has that can use this type of
// ammo.
static const ST::string s_cn_str_bobbyr_guns_num_guns_that_use_ammo =
	"您的队伍有%d件武器使用此类型弹药";


// Text for Bobby Ray's Home Page

static const ST::string s_cn_BobbyRaysFrontText[BobbyRaysFrontText_SIZE] =
{
	//Details on the web site

	"这里有最新最火爆的军用物资",	//"This is the place to be for the newest and hottest in weaponry and military supplies",
	"我们提供完美的方案满足您所有破坏欲望!",	//"We can find the perfect solution for all your explosives needs",
	"二手货",	//"Used and refitted items",

	//Text for the various links to the sub pages

	"杂货",	//"Miscellaneous",
	"枪械",	//"GUNS",
	"弹药",	//"AMMUNITION",		//5
	"护甲",	//"ARMOR",

	//Details on the web site

	"独此一家, 别无分店.",	//"If we don't sell it, you can't get it!",
	"建设中",	//"Under Construction",
};



// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug shot page

static const ST::string s_cn_AimSortText[AimSortText_SIZE] =
{
	"A.I.M成员",	//"A.I.M. Members",				// Title
	// Title for the way to sort
	"排序: ",	//"Sort By:",

	//Text of the links to other AIM pages

	"查看佣兵的肖像索引",	//"View the mercenary mug shot index",
	"查看单独的佣兵档案",	//"Review the individual mercenary's file",
	"浏览A.I.M前成员",	//"Browse the A.I.M. Alumni Gallery",
};


// text to display how the entries will be sorted
static const ST::string s_cn_str_aim_sort_price        =
	"费用";
static const ST::string s_cn_str_aim_sort_experience   =
	"级别";
static const ST::string s_cn_str_aim_sort_marksmanship =
	"枪法";
static const ST::string s_cn_str_aim_sort_medical      =
	"医疗";
static const ST::string s_cn_str_aim_sort_explosives   =
	"爆破";
static const ST::string s_cn_str_aim_sort_mechanical   =
	"机械";
static const ST::string s_cn_str_aim_sort_ascending    =
	"升序";
static const ST::string s_cn_str_aim_sort_descending   =
	"降序";


//Aim Policies.c
//The page in which the AIM policies and regulations are displayed

static const ST::string s_cn_AimPolicyText[AimPolicyText_SIZE] =
{
	// The text on the buttons at the bottom of the page

	"上页",	//"Previous Page",
	"AIM主页",	//"AIM HomePage",
	"规则索引",	//"Policy Index",
	"下页",	//"Next Page",
	"不同意",	//Disagree",
	"同意",	//"Agree",
};



//Aim Member.c
//The page in which the players hires AIM mercenaries

// Instructions to the user to either start video conferencing with the merc, or to go the mug shot index

static const ST::string s_cn_AimMemberText[AimMemberText_SIZE] =
{
	"鼠标左击",	//"Left Click",
	"联系佣兵.",	//"to Contact Merc.",
	"鼠标右击",	//"Right Click",
	"回到肖像索引.",	//"for Mug Shot Index.",
};

//Aim Member.c
//The page in which the players hires AIM mercenaries

static const ST::string s_cn_CharacterInfo[CharacterInfo_SIZE] =
{
	// the contract expenses' area

	"费用",	//"Fee",
	"合同",	//"Contract",
	"一日",	//"one day",
	"一周",	//"one week",
	"两周",	//"two weeks",

	// text for the buttons that either go to the previous merc,
	// start talking to the merc, or go to the next merc

	"上一位",	//"Previous",
	"联系",	//"Contact",
	"下一位",	//"Next",

	"附加信息",	//"Additional Info",				// Title for the additional info for the merc's bio
	"现役成员",	//"Active Members",		//20		// Title of the page
	"可选装备: ",				// Displays the optional gear cost
	"所需医疗保证金",			// If the merc required a medical deposit, this is displayed
};


//Aim Member.c
//The page in which the player's hires AIM mercenaries

//The following text is used with the video conference popup

static const ST::string s_cn_VideoConfercingText[VideoConfercingText_SIZE] =
{
	"合同总价: ",	//"Contract Charge:",				//Title beside the cost of hiring the merc

	//Text on the buttons to select the length of time the merc can be hired

	"一日",	//"One Day",
	"一周",	//"One Week",
	"两周",	//"Two Weeks",

	//Text on the buttons to determine if you want the merc to come with the equipment

	"不买装备",	//"No Equipment",
	"购买装备",	//"Buy Equipment",

	// Text on the Buttons

	"转帐",	//"TRANSFER FUNDS",			// to actually hire the merc
	"取消",	//"CANCE",				// go back to the previous menu
	"雇佣",	//"HIRE",				// go to menu in which you can hire the merc
	"挂断",	//"HANG UP",				// stops talking with the merc
	"完成",	//"OK",
	"留言",	//"LEAVE MESSAGE",			// if the merc is not there, you can leave a message

	//Text on the top of the video conference popup

	"视频通讯: ",	//"Video Conferencing with",
	"建立连接...",	//"Connecting. . .",

	"包括医保",	//"with medical"			//Displays if you are hiring the merc with the medical deposit
};



//Aim Member.c
//The page in which the player hires AIM mercenaries

// The text that pops up when you select the TRANSFER FUNDS button

static const ST::string s_cn_AimPopUpText[AimPopUpText_SIZE] =
{
	"电子转帐成功",	//"ELECTRONIC FUNDS TRANSFER SUCCESSFU",	// You hired the merc
	"无法处理转帐",	//"UNABLE TO PROCESS TRANSFER",		// Player doesn't have enough money, message 1
	"资金不足",	//"INSUFFICIENT FUNDS",				// Player doesn't have enough money, message 2

	// if the merc is not available, one of the following is displayed over the merc's face

	"执行任务中",	//"On Assignment"
	"请留言",	//"Please Leave Message",
	"阵亡",	//"Deceased",

	//If you try to hire more mercs than game can support

	"您的队伍现有18人, 已经满员了.",	//"You have a full team of 18 mercs already.",

	"预录消息",	//"Pre-recorded message",
	"留言已记录",	//"Message recorded",
};


//AIM Link.c

static const ST::string s_cn_AimLinkText =
	"A.I.M 链接"; // The title of the AIM links page



//Aim History

// This page displays the history of AIM

static const ST::string s_cn_AimHistoryText[AimHistoryText_SIZE] =
{
	"A.I.M 历史",	//"A.I.M. History",					//Title

	// Text on the buttons at the bottom of the page

	"上页",	//"Previous Page",
	"主页",	//"Home",
	"A.I.M 前成员",	//"A.I.M. Alumni",
	"下页",	//"Next Page",
};


//Aim Mug Shot Index

//The page in which all the AIM members' portraits are displayed in the order selected by the AIM sort page.

static const ST::string s_cn_AimFiText[AimFiText_SIZE] =
{
	// displays the way in which the mercs were sorted

	"费用",	//"Price",
	"级别",	//"Experience",
	"枪法",	//"Marksmanship",
	"医疗",	//"Medical",
	"爆破",	//"Explosives",
	"机械",	//"Mechanical",

	// The title of the page, the above text gets added at the end of this text

	"根据%s升序排列的A.I.M成员",	//"A.I.M. Members Sorted Ascending By %s",
	"根据%s降序排列的A.I.M成员",	//"A.I.M. Members Sorted Descending By %s",

	// Instructions to the players on what to do

	"鼠标左击",	//"Left Click",
	"选择佣兵.",	//"To Select Merc",			//10
	"鼠标右击",	//"Right Click",
	"回到排序选项.",	//"For Sorting Options",

	// Gets displayed on top of the merc's portrait if they are...

	"阵亡",	//"Deceased",						//14
	"任务中",	//"On Assign",
};



//AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer with AIM

static const ST::string s_cn_AimAlumniText[AimAlumniText_SIZE] =
{
	// Text of the buttons

	"页 1",
	"页 2",
	"页 3",

	"A.I.M. 前成员",	// Title of the page

	"好"			// Stops displaying information on selected merc
};






//AIM Home Page

static const ST::string s_cn_AimScreenText[AimScreenText_SIZE] =
{
	// AIM disclaimers

	"A.I.M. 和 A.I.M. 标志已在世界大多数国家注册.",
	"版权所有, 仿冒必究.",
	"版权 1998-1999 A.I.M., Ltd. 所有权利保留.",

	//Text for an advertisement that gets displayed on the AIM page

	"联合花卉服务公司",
	"\"我们将花空运到任何地方\"",				//10
	"把活干好",
	"... 第一次",
	"枪械和杂货, 只此一家, 别无分店.",
};


//Aim Home Page

static const ST::string s_cn_AimBottomMenuText[AimBottomMenuText_SIZE] =
{
	//Text for the links at the bottom of all AIM pages
	"主页",	//"Home",
	"成员",	//"Members",
	"前成员",	//"Alumni",
	"规则",	//"Policies",
	"历史",	//"History",
	"链接",	//"Links",
};



//ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.

static const ST::string s_cn_SKI_Text[SKI_SIZE ] =
{
	"库存商品",	//"MERCHANDISE IN STOCK",	//Header for the merchandise available
	"页面",			//"PAGE",	//The current store inventory page being displayed
	"总价格",			//"TOTAL COST",		//The total cost of the the items in the Dealer inventory area
	"总价值",			//"TOTAL VALUE",	//The total value of items player wishes to sell
	"估价",			//"EVALUATE",		//Button text for dealer to evaluate items the player wants to sell
	"确认交易",			//"TRANSACTION",	//Button text which completes the deal. Makes the transaction.
	"完成",			//"DONE",	//Text for the button which will leave the shopkeeper interface.
	"修理费",			//"REPAIR COST",	//The amount the dealer will charge to repair the merc's goods
	"1小时",			//"1 HOUR",// SINGULAR! The text underneath the inventory slot when an item is given to the dealer to be repaired
	"%d小时",		//"%d HOURS",// PLURAL!   The text underneath the inventory slot when an item is given to the dealer to be repaired
	"已经修好",		//"REPAIRED",// Text appearing over an item that has just been repaired by a NPC repairman dealer
	"您没有空余的位置来放东西了.",	//"There is not enough room in your offer area.",//Message box that tells the user there is no more room to put there stuff
	"%d分钟",		//"%d MINUTES",	// The text underneath the inventory slot when an item is given to the dealer to be repaired
	"把物品放在地上.",	//"Drop Item To Ground.",
};


static const ST::string s_cn_SkiMessageBoxText[SkiMessageBoxText_SIZE] =
{
	"您要从主帐户中提取%s来支付吗?",
	"资金不足.您缺少%s.",
	"您要从主帐户中提取%s来支付吗?",
	"请求商人开始交易",
	"请求商人修理选定物品",
	"结束对话",
	"当前余额",
};


//OptionScreen.c

static const ST::string s_cn_zOptionsText[zOptionsText_SIZE] =
{
	//button Text
	"保存游戏",	//"Save Game",
	"载入游戏",	//"Load Game",
	"退出",	//"Quit",
	"完成",	//"Done",

	//Text above the slider bars
	"效果音",	//"Effects",
	"语音",	//"Speech",
	"音乐",	//"Music",

	//Confirmation pop when the user selects..
	"退出并回到游戏主菜单?",

	"您必须选择 '语音' 和 '对话显示' 中至少一项.",
};


//SaveLoadScreen
static const ST::string s_cn_zSaveLoadText[zSaveLoadText_SIZE] =
{
	"保存游戏",
	"载入游戏",
	"取消",
	"保存选中",
	"载入选中",

	"保存游戏成功",
	"保存游戏错误!",
	"载入游戏成功",
	"载入游戏错误!",

	"存档的游戏版本不同于当前的游戏版本, 读取它游戏很可能可以正常进行.",
	"您确定要删除名为 \"%s\" 的存档吗？",

	"注意:"
	"尝试载入旧版本的存档. 存档将会被自动更新.",
	"您启用了不同的模组, 而不是保存游戏时使用的模组. 模组可能无法正常工作.",
	"要继续吗?"

	"您确认要覆盖名为 \"%s\" 的存档吗?",

	"保存中...",			//When saving a game, a message box with this string appears on the screen

	"正常的武器",
	"大量的武器",
	"真实风格",
	"科幻风格",
	"难度",

	"未启用任何MOD",
	"Mods:"
};



//MapScreen
static const ST::string s_cn_zMarksMapScreenText[zMarksMapScreenText_SIZE] =
{
	"地图层次",	//"Map Level",
	"您没有民兵.要拥有民兵, 您得先训练镇民.",	//"You have no militia.  You need to train town residents in order to have a town militia.",
	"每日收入",	//"Daily Income",
	"佣兵有人寿保险",	//"Merc has life insurance",
	"%s不疲劳.",	//"%s isn't tired.",
	"%s行军中, 不能睡觉",	//"%s is on the move and can't sleep",
	"%s太累了, 等会儿再试.",	//"%s is too tired, try a little later.",
	"%s正在开车.",	//"%s is driving.",
	"有人在睡觉时, 整个队伍不能行动.",	//"Squad can't move with a sleeping merc on it.",

	// stuff for contracts
	"您能支付合同所需费用, 但是您的钱不够给该佣兵购买人寿保险.",
	"要给%s花费保险金%s 以延长保险合同%d天.您要付费吗?",
	"区域存货",	//"Sector Inventory",
	"佣兵有医疗保证金.",	//"Merc has a medical deposit.",

	// other items
	"医生", 	//"Medics", // people acting a field medics and bandaging wounded mercs
	"病人", 	//"Patients", // people who are being bandaged by a medic
	"完成", 	//"Done", // Continue on with the game after autobandage is complete
	"停止", 	//"Stop", // Stop autobandaging of patients by medics now
	"%s没工具箱.",	//"%s doesn't have a repair kit.",
	"%s没医药箱.",	//"%s doesn't have a medical kit.",
	"现在没有足够的人愿意加入民兵.",
	"%s的民兵已经训练满了.",	//"%s is full of militia.",
	"佣兵有一份限时的合同.",	//"Merc has a finite contract.",
	"佣兵的合同没投保", //"Merc's contract is not insured",
};


static const ST::string s_cn_pLandMarkInSectorString =
	"%d小队在%s地区发现有人";

// confirm the player wants to pay X dollars to build a militia force in town
static const ST::string s_cn_pMilitiaConfirmStrings[pMilitiaConfirmStrings_SIZE] =
{
	"训练一队民兵要花费$",
	"同意支付吗?",
	"您付不起.",
	"继续在%s (%s %d)训练民兵吗?",
	"花费$",
	"( Y/N )",   // abbreviated yes/no
	"在%d地区训练民兵将花费$%d.%s",
	"您无法支付$%d以供在这里训练民兵.",
	"%s的忠诚度必须达到%d以上方可训练民兵.",
	"您不能在%s训练民兵了.",
};


//Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom of the single merc panel
static const ST::string s_cn_gzMoneyWithdrawMessageText[gzMoneyWithdrawMessageText_SIZE] =
{
	"您每次最多能提取$20,000.",
	"您确认要把%s存入您的帐户吗?",
};

static const ST::string s_cn_gzCopyrightText =
	"版权所有 (C) 1999 Sir-tech 加拿大 Ltd.  所有权利保留.";

//option Text
static const ST::string s_cn_zOptionsToggleText[zOptionsToggleText_SIZE] =
{
	"语音",		//"Speech",
	"确认静默",		//"Mute Confirmations",
	"显示对话文字",	//"Subtitles",
	"显示对话文字时暂停",		//"Pause Text Dialogue",
	"烟火效果",		//"Animate Smoke",
	"血腥效果",		//"Blood n Gore",
	"不移动鼠标",		//"Never Move My Mouse!",
	"旧的选择方式",	//"Old Selection Method",
	"显示移动路径",	//"Show Movement Path",
	"显示未击中",		//"Show Misses",
	"实时确认",		//"Real Time Confirmation",
	"显示睡觉/醒来时的提示",	//"Display sleep/wake notifications",
	"使用公制系统",		//"Use Metric System",
	"佣兵移动时高亮显示",	//"Merc Lights during Movement",
	"锁定佣兵",	//"Snap Cursor to Mercs",
	"锁定门",		//"Snap Cursor to Doors",
	"物品闪亮",	//"Make Items Glow",
	"显示树冠",	//"Show Tree Tops",
	"显示轮廓",		//"Show Wireframes",
	"显示3D光标",	//"Show 3D Cursor",
};

//This is the help text associated with the above toggles.
static const ST::string s_cn_zOptionsScreenHelpText[zOptionsToggleText_SIZE] =
{
	//speech
	"如果您想听到人物对话, 打开此选项.",

	//Mute Confirmation
	"打开或关闭人物的口头确认.",

	//Subtitles
	"是否显示对话的文字.",

	//Key to advance speech
	"如果 '显示对话文字' 已打开, 此选项会让您有足够的时间来阅读NPC的对话.",

	//Toggle smoke animation
	"如果烟火效果使得游戏变慢, 关闭该选项.",

	//Blood n Gore
	"如果鲜血使您觉得恶心, 关闭该选项.",

	//Never move my mouse
	"关闭该选项会使您的光标自动移到弹出的确认对话框上.",

	//Old selection method
	"打开此选项, 使用铁血联盟1代的佣兵选择方式.",

	//Show movement path
	"打开此选项, 会实时显示移动路径(关闭此选项.如果您想要显示路径的话, 使用SHIFT键).",

	//show misses
	"打开此选项时, 会显示未击中目标的子弹落点.",

	//Real Time Confirmation
	"当打开时, 进入实时模式会有一个确认对话框.",

	//Sleep/Wake notification
	"当打开时, 被分配任务的佣兵睡觉和醒来时会提示您.",

	//Use the metric system
	"当打开时, 使用公制系统, 否则使用英制系统.",

	//Merc Lighted movement
	"当打开时, 佣兵移动时会照亮地表.关闭该选项会使游戏的显示速度变快.",

	//Smart cursor
	"当打开时, 光标移动到佣兵身上时会高亮显示佣兵.",

	//snap cursor to the door
	"当打开时, 光标靠近门时会自动定位到门上.",

	//glow items
	"当打开时, 物品会不断的闪烁.(|I)",

	//toggle tree tops
	"当打开时, 显示树冠.(|T)",

	//toggle wireframe
	"显示未探明的墙的轮廓.(|W)",

	"打开时, 移动时的光标为3D式样.(|H)",

};


static const ST::string s_cn_gzGIOScreenText[gzGIOScreenText_SIZE] =
{
	"游戏初始设置",
	"游戏风格",
	"真实",
	"科幻",
	"武器数量", // changed by SANDRO
	"大量武器",
	"少量武器", // changed by SANDRO
	"难度",
	"新手",
	"老手",
	"专家",
	"确定",	// TODO.Translate
	"取消",
	"额外难度",
	"可随时存盘",
	"铁人模式",
	"Dead is Dead"
};

// This string is used in the IMP character generation.  It is possible to
// select 0 ability in a skill meaning you can't use it.  This text is
// confirmation to the player.
static const ST::string s_cn_pSkillAtZeroWarning    =	"您确定吗? '0' 意味着您不能拥有这项技能.";
static const ST::string s_cn_pIMPBeginScreenStrings =	"( 最多8个字符 )";
static const ST::string s_cn_pIMPFinishButtonText  =	"分析中";
static const ST::string s_cn_pIMPFinishStrings     =	"谢谢您, %s"; //%s is the name of the merc
static const ST::string s_cn_pIMPVoicesStrings     =	"嗓音"; // the strings for imp voices screen

// title for program
static const ST::string s_cn_pPersTitleText = "人事管理";

// paused game strings
static const ST::string s_cn_pPausedGameText[pPausedGameText_SIZE] =
{
	"游戏暂停",	//"Game Paused",
	"继续游戏 (|P|a|u|s|e)",	//"Resume Game (|P|a|u|s|e)",
	"暂停游戏 (|P|a|u|s|e)",
};


static const ST::string s_cn_pMessageStrings[pMessageStrings_SIZE] =
{
	"退出游戏",	//"Exit Game?",
	"确定",	//"OK",
	"是",	//"YES",
	"否",	//"NO",
	"取消",	//"CANCE",
	"再次雇佣",	//"REHIRE",
	"撒谎",	//"LIE",		//
	"无描述",	//"No description", //Save slots that don't have a description.
	"游戏已保存.",	//"Game Saved.",
	"日",	//"Day",
	"位佣兵",	//"Mercs",
	"空",	//"Empty Slot", //An empty save game slot
	"rpm",					//Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
	"分钟",	//"min",					//Abbreviation for minute.
	"米",	//"m",						//One character abbreviation for meter (metric distance measurement unit).
	"发",  //"rnds",				//Abbreviation for rounds (# of bullets)
	"公斤",	//"kg",					//Abbreviation for kilogram (metric weight measurement unit)
	"磅",	//"lb",					//Abbreviation for pounds (Imperial weight measurement unit)
	"主页",	//"Home",				//Home as in homepage on the internet.
	"USD", //"USD",					//Abbreviation to US dollars
	"n/a",					//Lowercase acronym for not applicable.
	"与此同时",	//"Meanwhile",	//Meanwhile
	"%s已到达%s%s区域",	//"%s has arrived in sector %s%s", //Name/Squad has arrived in sector A9.  Order must not change without notifying																		//SirTech

	"版本", //"Version",
	"无快速存档",	//"Empty Quick Save Slot",
	"该位置用来放Quick Save (快速存档). 请在战术屏幕或者地图屏幕按Alt+S进行快速存档.",
	"打开的",	//"Opened",
	"关闭的",	//"Closed",
	"磁盘空间不足. 只有%sMB可用空间, 《铁血联盟2》需要%sMB.",
	"%s抓住了%s.",	//"%s has caught %s.",	//'Merc name' has caught 'item' -- let SirTech know if name comes after item.
	"%s打了针剂.", 	//"%s has taken the drug.", //'Merc name' has taken the drug
	"%s无医疗技能",	//"%s has no medical skill",//'Merc name' has no medical skill.

	//CDRom errors (such as ejecting CD while attempting to read the CD)
	"The integrity of the game has been compromised.",
	"ERROR: Ejected CD-ROM",

	//When firing heavier weapons in close quarters, you may not have enough room to do so.
	"没有空间施展您的武器.",	//"There is no room to fire from here.",

	//Can't change stance due to objects in the way...
	"现在无法改变姿势.",	//"Cannot change stance at this time.",

	//Simple text indications that appear in the game, when the merc can do one of these things.
	"放下",	//"Drop",
	"投掷",	//"Throw",
	"交给",	//"Pass",

	"把%s交给了%s.", //"%s passed to %s.", //"Item" passed to "merc".  Please try to keep the item %s before the merc %s, otherwise,
											 //must notify SirTech.
	"没有足够空位以把%s交给%s",	//"No room to pass %s to %s.", //pass "item" to "merc".  Same instructions as above.

	//A list of attachments appear after the items.  Ex:  Kevlar vest ( Ceramic Plate 'Attached )'
	"附加 )",	//" Attached )",

	//Cheat modes
	"开启作弊等级1",	//"Cheat level ONE reached",
	"开启作弊等级2",	//"Cheat level TWO reached",

	//Toggling various stealth modes
	"小队进入潜行模式.",	//"Squad on stealth mode.",
	"小队退出潜行模式.",	//"Squad off stealth mode.",
	"%s进入潜行模式.",	//"%s on stealth mode.",
	"%s退出潜行模式.",	//"%s off stealth mode.",

	//Wireframes are shown through buildings to reveal doors and windows that can't otherwise be seen in
	//an isometric engine.  You can toggle this mode freely in the game.
	"打开显示轮廓",	//"Extra Wireframes On",
	"关闭显示轮廓",	//"Extra Wireframes Off",

	//These are used in the cheat modes for changing levels in the game.  Going from a basement level to
	//an upper level, etc.
	"无法从这层上去...",	//"Can't go up from this level...",
	"没有更低的层了...",	//"There are no lower levels...",
	"进入地下室%d层...",	//"Entering basement level %d...",
	"离开地下室...",			//"Leaving basement...",

	"的",	//"'s",		// used in the shop keeper inteface to mark the ownership of the item eg Red's gun
	"跟随模式关.",	//"Follow mode OFF.",
	"跟随模式开.",	//"Follow mode ON.",
	"3D光标关.",	//"3D Cursor OFF.",
	"3D光标开.",	//"3D Cursor ON.",
	"%d小队激活.",	//"Squad %d active.",
	"您无法支付%s的%s日薪",	//"You cannot afford to pay for %s's daily salary of %s",	//first %s is the mercs name, the seconds is a string containing the salary
	"跳过",			//"Skip",
	"%s不能独自离开",	//"%s cannot leave alone.",
	"一个文件名为SaveGame99.sav的存档被创建了.如果需要的话, 将其更名为SaveGame01 - SaveGame10, 然后您就能载入此存档了.",	//"A save has been created called, SaveGame99.sav.  If needed, rename it to SaveGame01 - SaveGame10 and then you will have access to it in the Load screen.",
	"%s喝了点%s",	//"%s drank some %s",
	"一个包裹到达Drassen.",	//"A package has arrived in Drassen.",
 	"%s将于%d日%s到达指定的着陆点(区域%s).",	//"%s should arrive at the designated drop-off point (sector %s) on day %d, at approximately %s.",		//first %s is mercs name, next is the sector location and name where they will be arriving in, lastely is the day an the time of arrival
	"日志已经更新.",	//"History log updated.",
};


static const ST::string s_cn_ItemPickupHelpPopup[ItemPickupHelpPopup_SIZE] =
{
	"确认",		//"OK",
	"向上滚动",		//"Scroll Up",
	"选择全部",		//"Select All",
	"向下滚动",		//"Scroll Down",
	"取消",		//"Cancel",
};

static const ST::string s_cn_pDoctorWarningString[pDoctorWarningString_SIZE] =
{
	"%s不够近, 不能被治疗.",
	"您的医生不能包扎完每个人.",
};

static const ST::string s_cn_pMilitiaButtonsHelpText[pMilitiaButtonsHelpText_SIZE] =
{
	"调离(右击)/安置(左击)新兵",
	"调离(右击)/安置(左击)老兵",
	"调离(右击)/安置(左击)精兵",
	"在各个区域平均分布民兵",
};

// to inform the player to hire some mercs to get things going
static const ST::string s_cn_pMapScreenJustStartedHelpText =
	"去AIM雇几位佣兵 (*提示* 在笔记本电脑里)";

static const ST::string s_cn_pAntiHackerString =
	"错误. 丢失或损坏文件, 游戏将退出.";


static const ST::string s_cn_gzLaptopHelpText[gzLaptopHelpText_SIZE] =
{
	//Buttons:
	"查看邮件",
	"浏览网页",
	"查看文件和邮件的附件",
	"阅读事件日志",
	"查看队伍信息",
	"查看财务简报和记录",
	"关闭笔记本电脑",

	//Bottom task bar icons (if they exist):
	"您有新的邮件",
	"您有新的文件",

	//Bookmarks:
	"国际佣兵联盟",
	"Bobby Ray网上武器店",
	"佣兵心理剖析研究所",
	"廉价新兵中心",
	"McGillicutty公墓",
	"联合花卉服务公司",
	"A.I.M指定保险代理人",
};


static const ST::string s_cn_gzHelpScreenText =
	"退出帮助界面";

static const ST::string s_cn_gzNonPersistantPBIText[gzNonPersistantPBIText_SIZE] =
{
	"战斗正在进行中, 您只能在战术屏幕进行撤退.",
	"进入该区域, 继续战斗.(|E)",
	"自动解决这次战斗.(|A)",
	"当您进攻时, 不能自动解决战斗.",
	"当您遭遇伏兵时, 不能自动解决战斗.",
	"当在矿井里和异形作战时, 不能自动解决战斗.",
	"还有敌对的平民时, 不能自动解决战斗.",
	"有血猫时, 不能自动解决战斗.",
	"战斗进行中",
	"您不能在这时撤退.",
};

static const ST::string s_cn_gzMiscString[gzMiscString_SIZE] =
{
	"在没有您的佣兵的支援下, 民兵继续战斗...",
	"现在车辆不需要加油.",	//"The vehicle does not need anymore fuel right now.",
	"油箱装了%d％的油.",	//"The fuel tank is %d%% full.",
	"Deidranna女王的军队重新完全占领了%s.",
	"您丢失了加油点.",	//"You have lost a refueling site.",
};

static const ST::string s_cn_gzIntroScreen = "找不到介绍视频";

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
static const ST::string s_cn_pNewNoiseStr[pNewNoiseStr_SIZE] =
{
	"%s听到%s声音来自%s.",
	"%s听到%s移动声来自%s.",
	"%s听到%s吱吱声来自%s.",
	"%s听到%s溅水声来自%s.",
	"%s听到%s撞击声来自%s.",
	"%s听到%s爆炸声发向%s.",
	"%s听到%s尖叫声发向%s.",
	"%s听到%s撞击声发向%s.",
	"%s听到%s撞击声发向%s.",
	"%s听到%s粉碎声来自%s.",
	"%s听到%s破碎声来自%s.",
};

static const ST::string s_cn_wMapScreenSortButtonHelpText[wMapScreenSortButtonHelpText_SIZE] =
{
	"按姓名排序 (|F|1)",
	"按任务排序 (|F|2)",
	"按睡眠状态排序 (|F|3)",
	"按地点排序 (|F|4)",
	"按目的地排序 (|F|5)",
	"按预计离队时间排序 (|F|6)",
};



static const ST::string s_cn_BrokenLinkText[BrokenLinkText_SIZE] =
{
	"404错误",	//"Error 404",
	"网站未找到",	//"Site not found.",
};


static const ST::string s_cn_gzBobbyRShipmentText[gzBobbyRShipmentText_SIZE] =
{
	"近期货物",	//"Recent Shipments",
	"订单 #",		//"Order #",
	"物品数量",	//"Number Of Items",
	"订购于",		//"Ordered On",
};


static const ST::string s_cn_gzCreditNames[gzCreditNames_SIZE]=
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


static const ST::string s_cn_gzCreditNameTitle[gzCreditNameTitle_SIZE]=
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

static const ST::string s_cn_gzCreditNameFunny[gzCreditNameFunny_SIZE]=
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

static const ST::string s_cn_sRepairsDoneString[sRepairsDoneString_SIZE] =
{
	"%s 修复了自己的物品.",
	"%s 修复了所有人的枪和护甲.",
	"%s 修复了所有人装备的物品.",
	"%s 修复了所有人携带的物品",//"%s finished repairing everyone's large carried items",
};

static const ST::string s_cn_zGioDifConfirmText[zGioDifConfirmText_SIZE]=
{
	"您选择了 '新手' 模式. 此设置是为那些刚玩铁血联盟的玩家准备的, 他们刚接触策略游戏, 或者他们希望快点结束战斗. 您的选择会在整个游戏中生效, 所以请作出明智的选择.您真的要玩 '新手' 模式吗?",
	"您选择了 '老手' 模式. 此设置是为那些已经熟悉铁血联盟或类似游戏的玩家准备的. 您的选择会在整个游戏中生效, 所以请作出明智的选择. 您真的要玩 '老手' 模式吗?",
	"您选择了 '专家' 模式. 我们警告您, 如果您被装在尸袋里运回来, 不要来向我们抱怨. 您的选择会在整个游戏中生效, 所以请作出明智的选择. 您真的要玩 '专家' 模式吗?",
};

static const ST::string s_cn_gzLateLocalizedString[gzLateLocalizedString_SIZE] =
{
	//1-5
	"由于无人在用遥控器, 机器人无法离开本区域.",

	//This message comes up if you have pending bombs waiting to explode in tactical.
	"您现在无法压缩时间.请等待炸弹爆炸!",

	//'Name' refuses to move.
	"%s拒绝移动.",

	//%s a merc name
	"%s精力不足, 无法改变姿势.",	//"%s does not have enough energy to change stance.",

	//A message that pops up when a vehicle runs out of gas.
	"%s汽油耗尽, 在%c%d抛锚了.",	//"The %s has run out of gas and is now stranded in %c%d.",

	//6-10

	// the following two strings are combined with the pNewNoise strings above to report noises
	// heard above or below the merc
	"上方",
	"下方",

	//The following strings are used in autoresolve for autobandaging related feedback.
	"佣兵中没人有医疗技能.",
	"没有足够的医疗物品进行包扎.",
	"没有足够的医疗物品给所有人进行包扎.",
	"佣兵中没人需要包扎.",	//"None of your mercs need bandaging.",
	"自动包扎佣兵.",	//"Bandages mercs automatically.",
	"全部佣兵已被包扎完毕.",	//"All your mercs are bandaged.",

	//14
	"Arulco",

	"(屋顶)",

	"生命: %d/%d",

	//In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
	//"vs." is the abbreviation of versus.
	"%d vs. %d",

	"%s满了.",

	"%s现在不用包扎, 他 (她) 需要认真的治疗和休息.",

	//20
	//Happens when you get shot in the legs, and you fall down.
	"%s被击中腿部, 并且倒下了!",
	//Name can't speak right now.
	"%s现在不能说话.",

	//22-24 plural versions
	"%d名新兵被提升为精兵.",
	"%d名新兵被提升为老兵.",
	"%d名老兵被提升为精兵.",

	//25
	"开关",

	//26
	//Name has gone psycho -- when the game forces the player into burstmode (certain unstable characters)
	"%疯狂了!",

	//27-28
	//Messages why a player can't time compress.
	"现在压缩时间不安全, 因为您有佣兵在区域%s.",
	"现在压缩时间不安全, 因为您有佣兵在被异形感染的矿井.",

	//29-31 singular versions
	"1名新兵晋升为精兵.",
	"1名新兵晋升为老兵.",
	"1名老兵晋升为精兵.",

	//32-34
	"%s无语.",	//"%s doesn't say anything.",
	"回到地面?",
	"(%d小队)",	//"(Squad %d)",

	//35
	//Ex: "Red has repaired Scope's MP5K".  Careful to maintain the proper order (Red before Scope, Scope before MP5K)
	"%s修复了%s的%s.",

	//36
	"血猫",

	//37-38 "Name trips and falls"
	"%s踩到陷阱, 跌倒了.",
	"此物品不能从这里捡起.",

	//39
	"您现有的佣兵中没人能进行战斗.民兵们将独自和异形作战.",

	//40-43
	//%s is the name of merc.
	"%s用完了医药箱里的药品!",	//"%s ran out of medical kits!",
	"%s无所需技能来医疗他人!",	//"%s lacks the necessary skill to doctor anyone!",
	"%s用完工具箱里的工具!",	//"%s ran out of tool kits!",
	"%s无所需技能来修理物品!",	//"%s lacks the necessary skill to repair anything!",

	//44-45
	"Repair Time",
	"%s看不到此人.",

	//46-48
	"%s的增程枪管掉下来了!",	//"%s's gun barrel extender falls off!",
	"只允许不多于%d名佣兵在此区域训练民兵.",	//"No more than %d militia trainers are permitted per sector.", //"No more than %d militia trainers are permitted in this sector.",//ham3.6
	"您确定吗?",			//"Are you sure?",

	//49-50
	"时间压缩",
	"车辆的油箱已经加满油了.",

	//51-52 Fast help text in mapscreen.
	"继续时间压缩 (|S|p|a|c|e)",
	"停止时间压缩 (|E|s|c)",

	//53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
	"%s修理好了卡壳的%s",	//"%s has unjammed the %s",
	"%s修理好了卡壳的%s的%s",	//"%s has unjammed %s's %s",

	//55
	"查看区域存货时候无法压缩时间",	//"Can't compress time while viewing sector inventory.",

	//56
	//Displayed with the version information when cheats are enabled.
	"当前/最大进展: %d%%/%d%%",

	"护送John和Mary?",

	// 58
	"开关被激活.",
};

static const ST::string s_cn_str_ceramic_plates_smashed =
	"%s的陶瓷片已经粉碎了!";

static const ST::string s_cn_str_arrival_rerouted =
	"援兵的会合地被挪至%s, 因降落地点%s目前由敌人占据.";

static const ST::string s_cn_str_stat_health       =
	"生命";
static const ST::string s_cn_str_stat_agility      =
	"敏捷";
static const ST::string s_cn_str_stat_dexterity    =
	"灵巧";
static const ST::string s_cn_str_stat_strength     =
	"力量";
static const ST::string s_cn_str_stat_leadership   =
	"领导";
static const ST::string s_cn_str_stat_wisdom       =
	"智慧";
static const ST::string s_cn_str_stat_exp_level    =
	"级别";
static const ST::string s_cn_str_stat_marksmanship =
	"枪法";
static const ST::string s_cn_str_stat_mechanical   =
	"机械";
static const ST::string s_cn_str_stat_explosive    =
	"爆破";
static const ST::string s_cn_str_stat_medical      =
	"医疗";

static const ST::string s_cn_str_stat_list[str_stat_list_SIZE] =
{
	s_cn_str_stat_health,
	s_cn_str_stat_agility,
	s_cn_str_stat_dexterity,
	s_cn_str_stat_strength,
	s_cn_str_stat_leadership,
	s_cn_str_stat_wisdom,
	s_cn_str_stat_exp_level,
	s_cn_str_stat_marksmanship,
	s_cn_str_stat_mechanical,
	s_cn_str_stat_explosive,
	s_cn_str_stat_medical
};

static const ST::string s_cn_str_aim_sort_list[str_aim_sort_list_SIZE] =
{
	s_cn_str_aim_sort_price,
	s_cn_str_aim_sort_experience,
	s_cn_str_aim_sort_marksmanship,
	s_cn_str_aim_sort_medical,
	s_cn_str_aim_sort_explosives,
	s_cn_str_aim_sort_mechanical,
	s_cn_str_aim_sort_ascending,
	s_cn_str_aim_sort_descending,
};

// VERY TRUNCATED FILE COPIED FROM JA2.5 FOR ITS FEATURES FOR JA2 GOLD

const ST::string g_cn_zNewTacticalMessages[] =
{
	"到目标的距离: %d格",
	"射程: %d格, 到目标的距离: %d格",
	"显示遮盖",
	"视线",
	"这是铁人模式的游戏, 敌人在周围时您不能保存.",	//	@@@  new text
	"(战斗中不能保存)", //@@@@ new text
	"(不能载入之前的存档)", // Stracciatella: New Dead is Dead game mode text
	"这是Dead is Dead模式的游戏, 您不能通过保存界面保存.",
};

//@@@:  New string as of March 3, 2000.
const ST::string g_cn_str_iron_man_mode_warning =
	"您选择了铁人模式. 此模式使游戏更具挑战性, 例如进入敌占区时不能保存游戏. 设定将在整个游戏进程中生效.  您确定想玩铁人模式吗?";

// Stracciatella: New Dead is Dead game mode warning
const ST::string g_cn_str_dead_is_dead_mode_warning =
	"您选择了DEAD IS DEAD模式. 此模式使游戏更具挑战性, 例如您不能在犯错后载入之前的存档. 游戏将在安全时自动保存. 设定将在整个游戏进程中生效.  您确定想玩DEAD IS DEAD模式吗?";
const ST::string g_cn_str_dead_is_dead_mode_enter_name =
	"您现在将进入保存界面. 选择一个位置并为您的存档命名. 您的游戏将持续存于此位置, 且无法更改!";

static const ST::string s_cn_gs_dead_is_dead_mode_tab_name[gs_dead_is_dead_mode_tab_name_SIZE] =
{
	"普通", 			// Normal Tab
	"DiD", 			// Dead is Dead Tab
};

// English language resources.
LanguageRes g_LanguageResChinese = {

	s_cn_WeaponType,

	s_cn_Message,
	s_cn_TeamTurnString,
	s_cn_pAssignMenuStrings,
	s_cn_pTrainingStrings,
	s_cn_pTrainingMenuStrings,
	s_cn_pAttributeMenuStrings,
	s_cn_pVehicleStrings,
	s_cn_pShortAttributeStrings,
	s_cn_pContractStrings,
	s_cn_pAssignmentStrings,
	s_cn_pConditionStrings,
	s_cn_pPersonnelScreenStrings,
	s_cn_pUpperLeftMapScreenStrings,
	s_cn_pTacticalPopupButtonStrings,
	s_cn_pSquadMenuStrings,
	s_cn_pDoorTrapStrings,
	s_cn_pLongAssignmentStrings,
	s_cn_pMapScreenMouseRegionHelpText,
	s_cn_pNoiseVolStr,
	s_cn_pNoiseTypeStr,
	s_cn_pDirectionStr,
	s_cn_pRemoveMercStrings,
	s_cn_sTimeStrings,
	s_cn_pInvPanelTitleStrings,
	s_cn_pPOWStrings,
	s_cn_pMilitiaString,
	s_cn_pMilitiaButtonString,
	s_cn_pEpcMenuStrings,
	s_cn_pRepairStrings,
	s_cn_sPreStatBuildString,
	s_cn_sStatGainStrings,
	s_cn_pHelicopterEtaStrings,
	s_cn_sMapLevelString,
	s_cn_gsLoyalString,
	s_cn_gsUndergroundString,
	s_cn_gsTimeStrings,
	s_cn_sFacilitiesStrings,
	s_cn_pMapPopUpInventoryText,
	s_cn_pwTownInfoStrings,
	s_cn_pwMineStrings,
	s_cn_pwMiscSectorStrings,
	s_cn_pMapInventoryErrorString,
	s_cn_pMapInventoryStrings,
	s_cn_pMovementMenuStrings,
	s_cn_pUpdateMercStrings,
	s_cn_pMapScreenBorderButtonHelpText,
	s_cn_pMapScreenBottomFastHelp,
	s_cn_pMapScreenBottomText,
	s_cn_pMercDeadString,
	s_cn_pSenderNameList,
	s_cn_pNewMailStrings,
	s_cn_pDeleteMailStrings,
	s_cn_pEmailHeaders,
	s_cn_pEmailTitleText,
	s_cn_pFinanceTitle,
	s_cn_pFinanceSummary,
	s_cn_pFinanceHeaders,
	s_cn_pTransactionText,
	s_cn_pMoralStrings,
	s_cn_pSkyriderText,
	s_cn_str_left_equipment,
	s_cn_pMapScreenStatusStrings,
	s_cn_pMapScreenPrevNextCharButtonHelpText,
	s_cn_pEtaString,
	s_cn_pShortVehicleStrings,
	s_cn_pTrashItemText,
	s_cn_pMapErrorString,
	s_cn_pMapPlotStrings,
	s_cn_pBullseyeStrings,
	s_cn_pMiscMapScreenMouseRegionHelpText,
	s_cn_str_he_leaves_where_drop_equipment,
	s_cn_str_she_leaves_where_drop_equipment,
	s_cn_str_he_leaves_drops_equipment,
	s_cn_str_she_leaves_drops_equipment,
	s_cn_pImpPopUpStrings,
	s_cn_pImpButtonText,
	s_cn_pExtraIMPStrings,
	s_cn_pFilesTitle,
	s_cn_pFilesSenderList,
	s_cn_pHistoryLocations,
	s_cn_pHistoryStrings,
	s_cn_pHistoryHeaders,
	s_cn_pHistoryTitle,
	s_cn_pShowBookmarkString,
	s_cn_pWebPagesTitles,
	s_cn_pWebTitle,
	s_cn_pPersonnelString,
	s_cn_pErrorStrings,
	s_cn_pDownloadString,
	s_cn_pBookMarkStrings,
	s_cn_pLaptopIcons,
	s_cn_gsAtmStartButtonText,
	s_cn_pPersonnelTeamStatsStrings,
	s_cn_pPersonnelCurrentTeamStatsStrings,
	s_cn_pPersonelTeamStrings,
	s_cn_pPersonnelDepartedStateStrings,
	s_cn_pMapHortIndex,
	s_cn_pMapVertIndex,
	s_cn_pMapDepthIndex,
	s_cn_pLaptopTitles,
	s_cn_pDayStrings,
	s_cn_pMilitiaConfirmStrings,
	s_cn_pSkillAtZeroWarning,
	s_cn_pIMPBeginScreenStrings,
	s_cn_pIMPFinishButtonText,
	s_cn_pIMPFinishStrings,
	s_cn_pIMPVoicesStrings,
	s_cn_pPersTitleText,
	s_cn_pPausedGameText,
	s_cn_zOptionsToggleText,
	s_cn_zOptionsScreenHelpText,
	s_cn_pDoctorWarningString,
	s_cn_pMilitiaButtonsHelpText,
	s_cn_pMapScreenJustStartedHelpText,
	s_cn_pLandMarkInSectorString,
	s_cn_gzMercSkillText,
	s_cn_gzNonPersistantPBIText,
	s_cn_gzMiscString,
	s_cn_wMapScreenSortButtonHelpText,
	s_cn_pNewNoiseStr,
	s_cn_gzLateLocalizedString,
	s_cn_pAntiHackerString,
	s_cn_pMessageStrings,
	s_cn_ItemPickupHelpPopup,
	s_cn_TacticalStr,
	s_cn_LargeTacticalStr,
	s_cn_zDialogActions,
	s_cn_zDealerStrings,
	s_cn_zTalkMenuStrings,
	s_cn_gzMoneyAmounts,
	s_cn_gzProsLabel,
	s_cn_gzConsLabel,
	s_cn_gMoneyStatsDesc,
	s_cn_gWeaponStatsDesc,
	s_cn_sKeyDescriptionStrings,
	s_cn_zHealthStr,
	s_cn_zVehicleName,
	s_cn_pExitingSectorHelpText,
	s_cn_InsContractText,
	s_cn_InsInfoText,
	s_cn_MercAccountText,
	s_cn_MercInfo,
	s_cn_MercNoAccountText,
	s_cn_MercHomePageText,
	s_cn_sFuneralString,
	s_cn_sFloristText,
	s_cn_sOrderFormText,
	s_cn_sFloristGalleryText,
	s_cn_sFloristCards,
	s_cn_BobbyROrderFormText,
	s_cn_BobbyRText,
	s_cn_str_bobbyr_guns_num_guns_that_use_ammo,
	s_cn_BobbyRaysFrontText,
	s_cn_AimSortText,
	s_cn_str_aim_sort_price,
	s_cn_str_aim_sort_experience,
	s_cn_str_aim_sort_marksmanship,
	s_cn_str_aim_sort_medical,
	s_cn_str_aim_sort_explosives,
	s_cn_str_aim_sort_mechanical,
	s_cn_str_aim_sort_ascending,
	s_cn_str_aim_sort_descending,
	s_cn_AimPolicyText,
	s_cn_AimMemberText,
	s_cn_CharacterInfo,
	s_cn_VideoConfercingText,
	s_cn_AimPopUpText,
	s_cn_AimLinkText,
	s_cn_AimHistoryText,
	s_cn_AimFiText,
	s_cn_AimAlumniText,
	s_cn_AimScreenText,
	s_cn_AimBottomMenuText,
	s_cn_zMarksMapScreenText,
	s_cn_gpStrategicString,
	s_cn_gpGameClockString,
	s_cn_SKI_Text,
	s_cn_SkiMessageBoxText,
	s_cn_zSaveLoadText,
	s_cn_zOptionsText,
	s_cn_gzGIOScreenText,
	s_cn_gzHelpScreenText,
	s_cn_gzLaptopHelpText,
	s_cn_gzMoneyWithdrawMessageText,
	s_cn_gzCopyrightText,
	s_cn_BrokenLinkText,
	s_cn_gzBobbyRShipmentText,
	s_cn_zGioDifConfirmText,
	s_cn_gzCreditNames,
	s_cn_gzCreditNameTitle,
	s_cn_gzCreditNameFunny,
	s_cn_pContractButtonString,
	s_cn_gzIntroScreen,
	s_cn_pUpdatePanelButtons,
	s_cn_sRepairsDoneString,
	s_cn_str_ceramic_plates_smashed,
	s_cn_str_arrival_rerouted,

	s_cn_str_stat_health,
	s_cn_str_stat_agility,
	s_cn_str_stat_dexterity,
	s_cn_str_stat_strength,
	s_cn_str_stat_leadership,
	s_cn_str_stat_wisdom,
	s_cn_str_stat_exp_level,
	s_cn_str_stat_marksmanship,
	s_cn_str_stat_mechanical,
	s_cn_str_stat_explosive,
	s_cn_str_stat_medical,

	s_cn_str_stat_list,
	s_cn_str_aim_sort_list,

	g_cn_zNewTacticalMessages,
	g_cn_str_iron_man_mode_warning,
	g_cn_str_dead_is_dead_mode_warning,
	g_cn_str_dead_is_dead_mode_enter_name,

	s_cn_gs_dead_is_dead_mode_tab_name,
	s_cn_gzIMPSkillTraitsText,
};
