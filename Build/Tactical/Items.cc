#include <stdexcept>

#include "Font_Control.h"
#include "Handle_Items.h"
#include "Items.h"
#include "Action_Items.h"
#include "TileDef.h"
#include "Weapons.h"
#include "Interface_Cursors.h"
#include "Soldier_Control.h"
#include "Overhead.h"
#include "Handle_UI.h"
#include "Points.h"
#include "Sound_Control.h"
#include "Isometric_Utils.h"
#include "Animation_Data.h"
#include "Random.h"
#include "Campaign.h"
#include "Interface.h"
#include "Interface_Panels.h"
#include "Explosion_Control.h"
#include "Keys.h"
#include "WCheck.h"
#include "Soldier_Profile.h"
#include "SkillCheck.h"
#include "LOS.h"
#include "Message.h"
#include "Text.h"
#include "FOV.h"
#include "Interface_Control.h"
#include "ShopKeeper_Interface.h"
#include "GameSettings.h"
#include "Environment.h"
#include "Auto_Resolve.h"
#include "Interface_Items.h"
#include "Game_Clock.h"
#include "Smell.h"
#include "StrategicMap.h"
#include "Campaign_Types.h"
#include "Soldier_Macros.h"
#include "MemMan.h"
#include "Debug.h"


#define ANY_MAGSIZE 255


// weight units are 100g each

////////////////////////////////////////////////////////////////////////////
//ATE: When adding new items, make sure to update text.c with text description
///////////////////////////////////////////////////////////////////////////

const INVTYPE Item[] =
{
	//  							CLASS								GRPH	GRA-			PER
	//CLASS						INDEX		CURSOR			TYPE	PHIC	WT	PCKT	PRICE COOL	DESCRIPTION							REL		REPAIR	FLAGS
	//---------				-----		-------     ----	--	  --  ----  -----	----	-----------							---		------		-----
	{	IC_PUNCH,					0,			PUNCHCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			ITEM_NOT_BUYABLE },
	//---WEAPONS---
	// NB For convenience, in accessing the Weapons table, the class index
	// of a weapon must be equal to its position in the Item table
	{	IC_GUN,			 			1,			TARGETCURS,	0,		1,		6,	1,		 350,	2,		/* Glock 17        */	+2,		+2,			IF_STANDARD_GUN },
	{	IC_GUN,			 			2,			TARGETCURS,	0,		2,		6,	1,		 480,	2,		/* Glock 18        */	+1,		+1,			IF_STANDARD_GUN | ITEM_BIGGUNLIST },
	{	IC_GUN,			 			3,			TARGETCURS,	0,		3,		11,	1,		 450,	2,		/* Beretta 92F     */	-1,		-1,			IF_STANDARD_GUN | ITEM_BIGGUNLIST },
	{	IC_GUN,			 			4,			TARGETCURS,	0,		4,		11,	1,		 470,	2,		/* Beretta 93R     */	-2,		-2,			IF_STANDARD_GUN },
	{	IC_GUN,			 			5,			TARGETCURS,	0,		5,		11,	1,		 250,	1,		/* .38 S&W Special */	+4,		+4,			IF_STANDARD_GUN },
	{	IC_GUN,			 			6,			TARGETCURS,	0,		6,		10,	1,		 300,	1,		/* .357 Barracuda  */	+3,		+3,			IF_STANDARD_GUN | ITEM_BIGGUNLIST },
	{	IC_GUN,			 			7,			TARGETCURS,	0,		7,		17,	1,		 300,	1,		/* .357 DesertEagle*/ -1,		-1,			IF_STANDARD_GUN },
	{	IC_GUN,			 			8,			TARGETCURS,	0,		8,		11,	1,		 400,	2,		/* .45 M1911			 */  0,		0,			IF_STANDARD_GUN | ITEM_BIGGUNLIST },
	{	IC_GUN,			 			9,			TARGETCURS,	0,		9,		21,	0,		 980,	3,		/* H&K MP5K      	 */	-1,		0,			IF_STANDARD_GUN },
	{	IC_GUN,						10,			TARGETCURS,	0,		10,		28,	0,		1170,	4,		/* .45 MAC-10	     */	-2,		-1,			IF_STANDARD_GUN },

	{	IC_GUN,						11,			TARGETCURS,	0,		11,		48,	0,		 700,	3,		/* Thompson M1A1   */	+3,		-3,			IF_TWOHANDED_GUN | ITEM_BIGGUNLIST },
	{	IC_GUN,						12,			TARGETCURS,	0,		12,		26,	0,		1330,	5,		/* Colt Commando   */	 0,		-1,			IF_TWOHANDED_GUN },
	{	IC_GUN,						13,			TARGETCURS,	0,		13,		31,	0,		 770,	3,		/* H&K MP53		 		 */	-1,		-1,			IF_TWOHANDED_GUN | ITEM_BIGGUNLIST },
	{	IC_GUN,						14,			TARGETCURS,	0,		14,		39,	0,		1180,	4,		/* AKSU-74         */ -2,		-1,			IF_TWOHANDED_GUN | ITEM_BIGGUNLIST },
	{	IC_GUN,						15,			TARGETCURS,	0,		15,		28,	0,		2750,	9,		/* 5.7mm FN P90    */ -2,		-4,			IF_STANDARD_GUN | ITEM_NOT_BUYABLE },
	{	IC_GUN,						16,			TARGETCURS,	0,		16,		19,	0,		 620,	3,		/* Type-85         */ -4,		+2,			IF_TWOHANDED_GUN | ITEM_BIGGUNLIST },
	{	IC_GUN,						17,			TARGETCURS,	0,		17,		39,	0,		1350,	5,		/* SKS             */ -4,		-2,			IF_TWOHANDED_GUN | ITEM_BIGGUNLIST },
	{	IC_GUN,						18,			TARGETCURS,	0,		18,		43,	0,		1930,	6,		/* Dragunov        */ +2,		+2,			IF_TWOHANDED_GUN | ITEM_BIGGUNLIST },
	{	IC_GUN,						19,			TARGETCURS,	0,		19,		66,	0,		1950,	6,		/* M24             */ +4,		+4,			IF_TWOHANDED_GUN },
	{	IC_GUN,						20,			TARGETCURS,	0,		20,		36,	0,		2380,	8,		/* Steyr AUG       */ +1,		-2,			IF_TWOHANDED_GUN | ITEM_BIGGUNLIST },

	{	IC_GUN,						21,			TARGETCURS,	0,		21,		41,	0,		1620,	6,		/* H&K G41         */ +1,		-1,			IF_TWOHANDED_GUN },
	{	IC_GUN,						22,			TARGETCURS,	0,		22,		29,	0,		1100,	4,		/* Ruger Mini-14   */  0,		-1,			IF_TWOHANDED_GUN },
	{	IC_GUN,						23,			TARGETCURS,	0,		23,		36,	0,		2680,	8,		/* C-7             */ -1,		-1,			IF_TWOHANDED_GUN },
	{	IC_GUN,						24,			TARGETCURS,	0,		24,		36,	0,		1970,	7,		/* FA-MAS          */ -2,		-2,			IF_TWOHANDED_GUN | ITEM_BIGGUNLIST },
	{	IC_GUN,						25,			TARGETCURS,	0,		25,		36,	0,		1830,	6,		/* AK-74           */ -1,		-2,			IF_TWOHANDED_GUN | ITEM_BIGGUNLIST },
	{	IC_GUN,						26,			TARGETCURS,	0,		26,		43,	0,		1450,	5,		/* AKM             */ +2,		+2,			IF_TWOHANDED_GUN | ITEM_BIGGUNLIST },
	{	IC_GUN,						27,			TARGETCURS,	0,		27,		29,	0,		2120,	7,		/* M-14            */ +1,		-1,			IF_TWOHANDED_GUN },
	{	IC_GUN,						28,			TARGETCURS,	0,		28,		43,	0,		2680,	8,		/* FN-FAL          */	 0,		-1,			IF_TWOHANDED_GUN },
	{	IC_GUN,						29,			TARGETCURS,	0,		29,		44,	0,		1570,	5,		/* H&K G3A3        */ +1,		-1,			IF_TWOHANDED_GUN | ITEM_BIGGUNLIST },
	{	IC_GUN,						30,			TARGETCURS,	0,		30,		38,	0,		2530,	8,		/* H&K G11         */	+3,		-4,			IF_TWOHANDED_GUN | ITEM_NOT_BUYABLE },

	{	IC_GUN,						31,			TARGETCURS,	0,		31,		36,	0,		 670,	3,		/* Remington M870  */	+3,		+3,			IF_TWOHANDED_GUN },
	{	IC_GUN,						32,			TARGETCURS,	0,		32,		38,	0,		 980,	4,		/* SPAS-15         */ -2,		-2,			IF_TWOHANDED_GUN | ITEM_BIGGUNLIST },
	{	IC_GUN,						33,			TARGETCURS,	0,		33,		41,	0,		2900,	9,		/* CAWS            */	-3,		-3,			IF_TWOHANDED_GUN | ITEM_NOT_BUYABLE },
	{	IC_GUN,						34,			TARGETCURS,	0,		34,		68,	0,		3100,	10,		/* FN Minimi       */	-1,		-2,			IF_TWOHANDED_GUN },
	{	IC_GUN,						35,			TARGETCURS,	0,		35,		48,	0,		3180,	10,		/* RPK-74          */ -1,		-2,			IF_TWOHANDED_GUN | ITEM_BIGGUNLIST },
	{	IC_GUN,						36,			TARGETCURS,	0,		36,		93,	0,		3420,	10,		/* H&K 21E         */	+2,		+1,			IF_TWOHANDED_GUN | ITEM_BIGGUNLIST },
	{	IC_BLADE,					37,			KNIFECURS,  1,		79,		9,	2,		 100,	2,		/* combat knife    */	+2,		+2,			IF_STANDARD_BLADE},
	{	IC_THROWING_KNIFE,38,			TARGETCURS, 1,		53,		1,	4,		  50,	3,		/* throwing knife  */	-1,		-1,			IF_STANDARD_BLADE},
	{	IC_THROWN,				39,			TOSSCURS,		1,		57,		5,	2,		   0,	0,		/* rock            */ 0,		0,			ITEM_NOT_BUYABLE},
	{	IC_LAUNCHER,			40,			TRAJECTORYCURS,	0,		37,		26,	0,		 900,	7,		/* grenade launcher*/	0,		-1,			IF_TWOHANDED_GUN },

	{	IC_LAUNCHER,			41,			TRAJECTORYCURS,	0,		0,		77,	0,		1800,	10,		/* mortar */					0,		-2,			IF_TWOHANDED_GUN},
	{	IC_THROWN,				42,			TOSSCURS,		1,		60,		4,	3,			 0,	0,		/* another rock    */	0,		0,			ITEM_NOT_BUYABLE},
	{	IC_BLADE,					43,			KNIFECURS,	0,		0,		0,	1, 		   0,	0,		/* yng male claws */	0,		0,			ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR | ITEM_DEFAULT_UNDROPPABLE },
	{	IC_BLADE,					44,			KNIFECURS,	0,		0,		0,	1, 		   0,	0,		/* yng fem claws  */	0,		0,			ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR | ITEM_DEFAULT_UNDROPPABLE },
	{	IC_BLADE,					45,			KNIFECURS,	0,		0,		0,	1,		   0,	0,		/* old male claws */	0,		0,			ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR | ITEM_DEFAULT_UNDROPPABLE },
	{	IC_BLADE,					46,			KNIFECURS,	0,		0,		0,	1,		   0,	0,		/* old fem claws  */	0,		0,			ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR | ITEM_DEFAULT_UNDROPPABLE },
	{	IC_TENTACLES,			47,			KNIFECURS,	0,		0,		0,	1,		   0,	0,		/* queen tentacles*/	0,		0,			ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR | ITEM_DEFAULT_UNDROPPABLE },
	{	IC_GUN,						48,			TARGETCURS,	0,		0,		0,	1,		   0,	0,		/* queen spit */			0,		0,			ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR | ITEM_DEFAULT_UNDROPPABLE },
	{	IC_PUNCH,					49,			PUNCHCURS,	1,		102,	1,	4,		  20,	2,		/* brass knuckles */	0,		0,			IF_STANDARD_BLADE },
	{	IC_LAUNCHER,			50,			INVALIDCURS,0,		39,		13,	0,		 500,	8,		/* underslung g.l.*/	0,		0,			IF_STANDARD_GUN},

	{	IC_GUN,						51,			TARGETCURS,	0,		38,		21,	0,		 500,	9,		/* rocket Launcher*/	0,		-3,			IF_TWOHANDED_GUN },	// now repairable
	{	IC_BLADE,					52,			KNIFECURS,	0,		0,		0,	1, 		   0,	0,		/* bloodcat claws*/		0,		0,			ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR | ITEM_DEFAULT_UNDROPPABLE },
	{	IC_BLADE,					53,			KNIFECURS,	0,		0,		0,	1, 		   0,	0,		/* bloodcat bite */		0,		0,			ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR | ITEM_DEFAULT_UNDROPPABLE },
	{	IC_BLADE,					54,			KNIFECURS,	0,		41,		13,	0,		 200,	3,		/* machete */					0,		+3,			IF_STANDARD_BLADE},
	{	IC_GUN,						55,			TARGETCURS,	0,		45,		40,	0,		5000,	0,		/* rocket rifle */		0,		-5,			IF_TWOHANDED_GUN | ITEM_NOT_BUYABLE | ITEM_ELECTRONIC },
	{	IC_GUN,						56,			TARGETCURS,	0,		40,		12,	0,		1000,	0,		/* Automag III */			0,		-2,			IF_STANDARD_GUN | ITEM_NOT_BUYABLE },
	{	IC_GUN,						57,			TARGETCURS,	0,		0,		0,	0,		   0,	0,		/* infant spit */			0,		0,			ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR | ITEM_DEFAULT_UNDROPPABLE },
	{	IC_GUN,						58,			TARGETCURS,	0,		0,		0,	0,		   0,	0,		/* yng male spit */		0,		0,			ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR | ITEM_DEFAULT_UNDROPPABLE },
	{	IC_GUN,						59,			TARGETCURS,	0,		0,		0,	0,		   0,	0,		/* old male spit */		0,		0,			ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR | ITEM_DEFAULT_UNDROPPABLE },
	{	IC_GUN,						60,			TARGETCURS,	0,		37,		26,	0,		   0,	0,		/* tank cannon  */		0,		0,			ITEM_NOT_BUYABLE },

	{	IC_GUN,						61,			TARGETCURS,		0,		46,	 12,	1,		 500,	5,		/* dart gun */				0,		+3,			IF_STANDARD_GUN },
	{	IC_THROWING_KNIFE,62,			TARGETCURS,		1,		95,		1,	4,		  50,	0,		/*bloody throw.knife*/0,		+4,			IF_STANDARD_BLADE | ITEM_NOT_BUYABLE },
	{	IC_GUN,						63,			TARGETCURS,		0,		48,	 18,	0,		   0,	0,		/* flamethrower */		0,		0,			IF_STANDARD_GUN | ITEM_NOT_BUYABLE },
	{	IC_PUNCH,					64,			PUNCHCURS,		1,		85,	 30,	0,		  40,	1,		/* Crowbar       */		0,		-4,			ITEM_METAL | ITEM_DAMAGEABLE },
	{	IC_GUN,						65,			TARGETCURS,		0,		45,		40,	0,	 10000,	0,		/* rocket rifle */		0,		-5,			IF_TWOHANDED_GUN | ITEM_NOT_BUYABLE | ITEM_ELECTRONIC },
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	//---AMMO---
	{	IC_AMMO,					0,			INVALIDCURS,	1,		32,		2,	8,		  15,	2,		/* CLIP9_15 */				0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					1,			INVALIDCURS,	1,		35,		3,	4,		  30,	4,		/* CLIP9_30 */				0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					2,			INVALIDCURS,	1,		33,		2,	8,		  45,	4,		/* CLIP9_15_AP */			0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					3,			INVALIDCURS,	1,		36,		3,	4,		  90,	6,		/* CLIP9_30_AP */			0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					4,			INVALIDCURS,	1,		34,		2,	8,		  30,	3,		/* CLIP9_15_HP */			0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					5,			INVALIDCURS,	1,		37,		3,	4,		  60,	5,		/* CLIP9_30_HP */			0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					6,			INVALIDCURS,	1,		24,		1,	8,		   5,	1,		/* CLIP38_6 */				0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					7,			INVALIDCURS,	1,		25,		1,	8,		  15,	3,		/* CLIP38_6_AP */			0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					8,			INVALIDCURS,	1,		26,		1,	8,		  10,	2,		/* CLIP38_6_HP */			0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					9,			INVALIDCURS,	1,		14,		2,	8,		  10,	2,		/* CLIP45_7 */				0,		0,			IF_STANDARD_CLIP},

	{	IC_AMMO,					10,			INVALIDCURS,	1,		 4,	 10,	4,		  45,	3,		/* CLIP45_30 */				0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					11,			INVALIDCURS,	1,		15,		2,	8,		  45,	4,		/* CLIP45_7_AP */			0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					12,			INVALIDCURS,	1,		 5,	 10,	4,		 135,	5,		/* CLIP45_30_AP */		0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					13,			INVALIDCURS,	1,		16,		2,	8,		  30,	3,		/* CLIP45_7_HP */			0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					14,			INVALIDCURS,	1,		 6,	 10,	4,		  90,	4,		/* CLIP45_30_HP */		0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					15,			INVALIDCURS,	1,		11,		1,	8,		  10,	1,		/* CLIP357_6 */				0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					16,			INVALIDCURS,	1,		17,		3,	8,			15,	1,		/* CLIP357_9 */				0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					17,			INVALIDCURS,	1,		12,		1,	8,		  30,	3,		/* CLIP357_6_AP */		0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					18,			INVALIDCURS,	1,		18,		3,	8,		  45,	3,		/* CLIP357_9_AP */		0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					19,			INVALIDCURS,	1,		13,		1,	8,		  20,	2,		/* CLIP357_6_HP */		0,		0,			IF_STANDARD_CLIP},

	{	IC_AMMO,					20,			INVALIDCURS,	1,		19,		3,	8,		  30,	2,		/* CLIP357_9_HP */		0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					21,			INVALIDCURS,	1,		 9,		6,	4,		 150,	5,		/* CLIP545_30_AP */		0,		0,			IF_STANDARD_CLIP | ITEM_BIGGUNLIST },
	{	IC_AMMO,					22,			INVALIDCURS,	1,		10,		6,	4,		 100,	4,		/* CLIP545_30_HP */		0,		0,			IF_STANDARD_CLIP | ITEM_BIGGUNLIST },
	{	IC_AMMO,					23,			INVALIDCURS,	1,		 7,		5,	4,		 150,	4,		/* CLIP556_30_AP */		0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					24,			INVALIDCURS,	1,		 8,		5,	4,		 100,	3,		/* CLIP556_30_HP */		0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					25,			INVALIDCURS,	1,		22,		3,	6,		  60,	6,		/* CLIP762W_10_AP */	0,		0,			IF_STANDARD_CLIP | ITEM_BIGGUNLIST },
	{	IC_AMMO,					26,			INVALIDCURS,	1,		29,		8,	4,		 180,	4,		/* CLIP762W_30_AP */	0,		0,			IF_STANDARD_CLIP | ITEM_BIGGUNLIST },
	{	IC_AMMO,					27,			INVALIDCURS,	1,		23,		3,	6,		  40,	5,		/* CLIP762W_10_HP */	0,		0,			IF_STANDARD_CLIP | ITEM_BIGGUNLIST },
	{	IC_AMMO,					28,			INVALIDCURS,	1,		30,		8,	4,		 120,	3,		/* CLIP762W_30_HP */	0,		0,			IF_STANDARD_CLIP | ITEM_BIGGUNLIST },
	{	IC_AMMO,					29,			INVALIDCURS,	1,		 3,		1,	6,		  30,	7,		/* CLIP762N_5_AP */		0,		0,			IF_STANDARD_CLIP},

	{	IC_AMMO,					30,			INVALIDCURS,	1,		27,		8,	4,		 120,	6,		/* CLIP762N_20_AP */	0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					31,			INVALIDCURS,	1,		 2,		1,	6,		  20,	6,		/* CLIP762N_5_HP */		0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					32,			INVALIDCURS,	1,		28,		8,	4,		  80,	5,		/* CLIP762N_20_HP */	0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					33,			INVALIDCURS,	1,		31,		5,	4,		 700,	8,		/* CLIP47_50_SAP */		0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					34,			INVALIDCURS,	1,		20,		9 ,	4,		 750,	9,		/* CLIP57_50_SAP */		0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					35,			INVALIDCURS,	1,		21,		9,	4,		 500,	9,		/* CLIP57_50_HP */		0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					37,			INVALIDCURS,	2,		22,		5,	6,		  20,	3,		/* CLIP12G_7 */				0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					36,			INVALIDCURS,	2,		 4,		5,	6,		  20,	3,		/* CLIP12G_7_BUCKSHOT */	0,		0,	IF_STANDARD_CLIP},
	{	IC_AMMO,					39,			INVALIDCURS,	1,		 0,	 10,	6,		 300,	9,		/* CLIPCAWS_10_SAP */	0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					38,			INVALIDCURS,	1,		 1,	 10,	6,		 300,	9,		/* CLIPCAWS_10_FLECH */		0,		0,	IF_STANDARD_CLIP},

	{	IC_AMMO,					40,			INVALIDCURS,	1,		110, 10,	4,		 500,	9,		/* CLIPROCKET_AP */		0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					41,			INVALIDCURS,	1,		115, 10,	4,		 500,	9,		/* CLIPROCKET_HE */		0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					42,			INVALIDCURS,	1,		114, 10,	4,		 500,	9,		/* CLIPROCKET_HEAT */	0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					43,			INVALIDCURS,	1,		119,	1,	8,		  10,	4,		/* sleep dart */			0,		0,			IF_STANDARD_CLIP},
	{	IC_AMMO,					44,			INVALIDCURS,	0,		49,		8,	4,		   0,	0,		/* flameThrwr clip */	0,		0,			IF_STANDARD_CLIP | ITEM_NOT_BUYABLE },
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},

	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,					0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},

	//---EXPLOSIVES---

	{	IC_GRENADE,	 			0,			TOSSCURS,			1,		38,		6,	4,		 100,	6,		/* stun grenade				*/	0,		-2,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_REPAIRABLE },
	{	IC_GRENADE,	 			1,			TOSSCURS,			1,		48,		6,	4,		 120,	5,		/* tear gas grenade   */	0,		-2,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_REPAIRABLE},
	{	IC_GRENADE,	 			2,			TOSSCURS,			1,		41,		6,	4,		 500,	8, 		/* mustard gas grenade*/	0,		-3,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_REPAIRABLE},
	{	IC_GRENADE,	 			3,			TOSSCURS,			1,		50,		3,	6,		 150,	6,		/* mini hand grenade  */	0,		-4,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_REPAIRABLE},
	{	IC_GRENADE,	 			4,			TOSSCURS,			1,		49,		6,	4,		 200,	7,		/* reg hand grenade   */	0,		-4,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_REPAIRABLE},
	{	IC_BOMB,		 			5,			INVALIDCURS,	2,		 3,	 11,	2,		 400,	7,		/* RDX                */	0,		-4,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE},
	{	IC_BOMB,		 			6,			INVALIDCURS,	2,		 0,	 11,	1,		 500,	6,		/* TNT (="explosives")*/	0,		-4,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE},
	{	IC_BOMB,		 			7,			INVALIDCURS,	2,		23,	 11,	1,		1000,	8,		/* HMX (=RDX+TNT)     */	0,		-4,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE},
	{	IC_BOMB,		 			8,			INVALIDCURS,	1,		45,	 11,	1,		 750,	7,		/* C1  (=RDX+min oil) */	0,		-4,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE},
	{	IC_BOMB,		 			9,			INVALIDCURS,	1,		40,	 41,	2,		 400,	9,		/* mortar shell       */	0,		-4,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_REPAIRABLE},

	{	IC_BOMB,					10,			BOMBCURS,			1,		46,		8,	1,		 300,	5,		/* mine               */	0,		-4,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_REPAIRABLE },
	{	IC_BOMB,					11,			INVALIDCURS,	1,		44,	 11,	1,		1500,	9,		/* C4  ("plastique")  */	0,		-4,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE},
	{	IC_BOMB,					12,			BOMBCURS,			1,		42,		4,	2,		   0,	0,		/* trip flare				  */	0,		-2,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_NOT_BUYABLE | ITEM_REPAIRABLE },
	{	IC_BOMB,					13,			BOMBCURS,			1,		43,		4,	2,		   0,	0,		/* trip klaxon        */	0,		-2,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_NOT_BUYABLE | ITEM_REPAIRABLE },
	{	IC_BOMB,					14,			INVALIDCURS,	1,		107,	2,	4,		 250,	6,		/* shaped charge			*/	0,		-4,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_REPAIRABLE},
	{	IC_GRENADE,				15,			TOSSCURS,			2,		24,		1,	6,		  50,	3,		/* break light (flare)*/	0,		0,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE},
	{	IC_GRENADE,				16,			INVALIDCURS,	1,		97,  10,	4,		 400,	8,		/* 40mm HE grenade		*/	0,		-4,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_REPAIRABLE},
	{	IC_GRENADE,				17,			INVALIDCURS,	1,		111, 10,	4,		 250,	6,		/* 40mm tear gas grnd */	0,		-2,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_REPAIRABLE},
	{	IC_GRENADE,				18,			INVALIDCURS,	1,		113, 10,	4,		 200,	5,		/* 40mm stun grenade  */	0,		-2,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_REPAIRABLE},
	{	IC_GRENADE,				19,			INVALIDCURS,	1,		112, 10,	4,		 100,	7,		/* 40mm smoke grenade */	0,		-2,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_REPAIRABLE},

	{	IC_GRENADE,				20,			TOSSCURS,			1,		98,		6,	4,		  50,	4,		/* smoke hand grenade */	0,		-2,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_REPAIRABLE},
	{	IC_BOMB,					21,			INVALIDCURS,	1,		40,	 41,	8,		 450,	0,		/* tank shell       */		0,		-4,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR },
	{	IC_BOMB,					22,			INVALIDCURS,	1,		40,	 41,	2,		 450,	0,		/* fake struct ignite*/		0,		-4,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_NOT_BUYABLE },
	{	IC_GRENADE,				23,			TOSSCURS,			2,		37,		6,	4,		  50,	0,		/* creature cocktail*/		0,		0,			ITEM_DAMAGEABLE | ITEM_METAL },
	{	IC_BOMB,					24,			INVALIDCURS,	1,		40,	 41,	2,		 450,	0,		/* fake struct xplod*/		0,		-4,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_NOT_BUYABLE },
	{	IC_BOMB,					25,			INVALIDCURS,	1,		40,	 41,	2,		 450,	0,		/* fake vehicle xplod*/		0,		-4,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_NOT_BUYABLE },
	{	IC_GRENADE,	 			26,			TOSSCURS,			1,		48,		6,	4,		   0,	0,		/* BIG tear gas grenade*/	0,		-2,			ITEM_DAMAGEABLE | ITEM_METAL | ITEM_REPAIRABLE | ITEM_NOT_BUYABLE },
	{	IC_GRENADE,				27,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* small creature gas */	0,		0,			0},
	{	IC_GRENADE,				28,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* big creature gas */		0,		0,			0},
	{	IC_GRENADE,				29,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* very sm creat gas */	  0,		0,			0},

	//---ARMOUR---
	{ IC_ARMOUR,			 0,			INVALIDCURS,	1,		66,		20,	0,		 300, 2,		/* Flak jacket     */ 0,		+2,			IF_STANDARD_ARMOUR},
	{ IC_ARMOUR,			 1,			INVALIDCURS,	2,		18,		22,	0,		 350, 0,		/* Flak jacket w X */ 0,		+1,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{ IC_ARMOUR,			 2,			INVALIDCURS,	2,		11,		18,	0,		 400, 0,		/* Flak jacket w Y */ 0,		+3,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{ IC_ARMOUR,			 3,			INVALIDCURS,	1,		64,		32,	0,		 500, 4,		/* Kevlar jacket   */ 0,		0,			IF_STANDARD_ARMOUR},
	{ IC_ARMOUR,			 4,			INVALIDCURS,	2,		16,		35,	0,		 600, 0,		/* Kevlar jack w X */ 0,		-1,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{ IC_ARMOUR,			 5,			INVALIDCURS,	2,		 9,		29,	0,		 700, 0,		/* Kevlar jack w Y */ 0,		+1,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{ IC_ARMOUR,			 6,			INVALIDCURS,	1,		65,		32,	0,		1000, 8,		/* Spectra jacket  */ 0,		-2,			IF_STANDARD_ARMOUR},
	{ IC_ARMOUR,			 7,			INVALIDCURS,	2,		17,		35,	0,		1100, 0,		/* Spectra jack w X*/ 0,		-3,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{ IC_ARMOUR,			 8,			INVALIDCURS,	2,		10,		29,	0,		1200, 0,		/* Spectra jack w Y*/ 0,		-1,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{ IC_ARMOUR,			 9,			INVALIDCURS,	1,		67,		39,	0,		 650,	5,		/* Kevlar leggings */	0,		0,			IF_STANDARD_ARMOUR},

	{ IC_ARMOUR,			10,			INVALIDCURS,	2,		19,		43,	0,		 800,	0,		/* Kevlar legs w X */	0,		-1,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{ IC_ARMOUR,			11,			INVALIDCURS,	2,		12,		35,	0,		 950,	0,		/* Kevlar legs w Y */	0,		+1,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{ IC_ARMOUR,			12,			INVALIDCURS,	1,		68,		39,	0,		 900,	8,		/* Spectra leggings*/	0,		-2,			IF_STANDARD_ARMOUR},
	{ IC_ARMOUR,			13,			INVALIDCURS,	2,		20,		43,	0,		1100,	0,		/* Spectra legs w X*/	0,		-3,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{ IC_ARMOUR,			14,			INVALIDCURS,	2,		13,		35,	0,		1300,	0,		/* Spectra legs w Y*/	0,		-1,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{ IC_ARMOUR,			15,			INVALIDCURS,	1,		61,		14,	0,		  50,	2,		/* Steel helmet    */	0,		+2,			IF_STANDARD_ARMOUR | ITEM_METAL},
	{ IC_ARMOUR,			16,			INVALIDCURS,	1,		63,		14,	0,		 200,	4,		/* Kevlar helmet   */	0,		0,			IF_STANDARD_ARMOUR},
	{ IC_ARMOUR,			17,			INVALIDCURS,	2,		15,		15,	0,		 250,	0,		/* Kevlar helm w X */	0,		-1,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{ IC_ARMOUR,			18,			INVALIDCURS,	2,		 8,		13,	0,		 300,	0,		/* Kevlar helm w Y */	0,		+1,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{ IC_ARMOUR,			19,			INVALIDCURS,	1,		62,		14,	0,		 450,	7,		/* Spectra helmet  */	0,		-2,			IF_STANDARD_ARMOUR},

	{ IC_ARMOUR,			20,			INVALIDCURS,	2,		14,		15,	0,		 550,	0,		/* Spectra helm w X*/	0,		-3,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{ IC_ARMOUR,			21,			INVALIDCURS,	2,		 7,		13,	0,		 650,	0,		/* Spectra helm w Y*/	0,		-1,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{ IC_ARMOUR,			22,			INVALIDCURS,	1,		81,		12,	2,		 250,	5,		/* Ceramic plates  */ 0,		-4,			(IF_STANDARD_ARMOUR | ITEM_ATTACHMENT) & (~ITEM_REPAIRABLE) },
	{ IC_ARMOUR,			23,			INVALIDCURS,	1,		0,		0,	0,			 0,	0,		/* Infant crt hide */ 0,		0,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR | ITEM_DEFAULT_UNDROPPABLE},
	{ IC_ARMOUR,			24,			INVALIDCURS,	1,		0,		0,	0,			 0,	0,		/* Yng male hide */		0,		0,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR | ITEM_DEFAULT_UNDROPPABLE},
	{ IC_ARMOUR,			25,			INVALIDCURS,	1,		0,		0,	0,			 0,	0,		/* Old male hide */		0,		0,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR | ITEM_DEFAULT_UNDROPPABLE},
	{ IC_ARMOUR,			26,			INVALIDCURS,	1,		0,		0,	0,			 0,	0,		/* Queen cret hide */ 0,		0,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR | ITEM_DEFAULT_UNDROPPABLE},
	{ IC_ARMOUR,			27,			INVALIDCURS,	1,		96,		20,	0,		 200,	2,		/* Leather jacket  */ 0,		+4,			IF_STANDARD_ARMOUR },
	// NOTE: THE FOLLOWING ITEM'S PRICE VALUE IS IN DIALOGUE AND SHOULD NOT BE CHANGED
	{ IC_ARMOUR,			28,			INVALIDCURS,	1,		116,	20,	0,		 950,	0,		/* L jacket w kev  */ 0,		+2,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{ IC_ARMOUR,			29,			INVALIDCURS,	1,		117,	20,	0,		1200,	0,		/* L jacket w kev 18*/0,		+1,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },

	{ IC_ARMOUR,			30,			INVALIDCURS,	1,		118,	20,	0,		1500,	0,		/* L jacket w kev c*/ 0,		+3,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{	IC_ARMOUR,			31,			INVALIDCURS,	1,		0,		0,	0,		   0,	0,		/* yng fem hide */		0,		0,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR | ITEM_DEFAULT_UNDROPPABLE},
	{	IC_ARMOUR,			32,			INVALIDCURS,	1,		0,		0,	0,		   0,	0,		/* old fem hide */		0,		0,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE | ITEM_NOT_EDITOR | ITEM_DEFAULT_UNDROPPABLE},
	{	IC_ARMOUR,			33,			INVALIDCURS,	2,		25,		3,	1,		  10,	1,		/* t-shirt */					0,		0,			ITEM_DAMAGEABLE | ITEM_SHOW_STATUS | ITEM_UNAERODYNAMIC},
	{	IC_ARMOUR,			33,			INVALIDCURS,	2,		34,		3,	1,		  10,	1,		/* t-shirt D. rules*/	0,		0,			ITEM_DAMAGEABLE | ITEM_SHOW_STATUS | ITEM_UNAERODYNAMIC},
	{ IC_ARMOUR,			34,			INVALIDCURS,	1,		137,		32,	0,		 700, 6,		/* Kevlar2 jacket  */ 0,		-1,			IF_STANDARD_ARMOUR},
	{ IC_ARMOUR,			35,			INVALIDCURS,	2,		40,		35,	0,		 800, 0,		/* Kevlar2 jack w X*/ 0,		-2,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{ IC_ARMOUR,			36,			INVALIDCURS,	2,		41,		29,	0,		 900, 0,		/* Kevlar2 jack w Y*/ 0,		0,			IF_STANDARD_ARMOUR | ITEM_NOT_BUYABLE },
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},

	//---MISC---
	{ IC_MEDKIT,			0,			AIDCURS,			1,		73,		5,	4,		 100,	1,		/* First aid kit */		0,		0,			IF_STANDARD_KIT},
	{ IC_MEDKIT,			0,			AIDCURS,			1,		86,		18,	0,		 300,	1,		/* Medical Kit   */		0,		0,			IF_STANDARD_KIT | ITEM_METAL},
	{ IC_KIT,					0,			REPAIRCURS,		2,		21,		50, 0,		 250,	1,		/* Tool Kit	     */		0,		0,			IF_STANDARD_KIT | ITEM_METAL},
	{ IC_KIT,					0,			INVALIDCURS,	1,		78,		3,	1,		 250,	3,	  /* Locksmith kit */		0,		-2,			IF_STANDARD_KIT | ITEM_METAL | ITEM_REPAIRABLE},
	{ IC_KIT,					0,			INVALIDCURS,	1,		58,		1,	4,		 250,	5,	  /* Camouflage kit*/		0,		0,			IF_STANDARD_KIT},
	{	IC_MISC,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_MISC,				0,			INVALIDCURS,	1,		52,		5,	4,		 300,	5,		/* Silencer      */		0,		0,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL | ITEM_ATTACHMENT},
	{	IC_MISC,				0,			INVALIDCURS,	1,		76,		9,	4,		 500,	6,		/* Sniper scope  */		0,		0,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL | ITEM_ATTACHMENT},
	{	IC_MISC,				0,			INVALIDCURS,	1,		69,		5,	2,		  50,	4,		/* Bipod         */		0,		+5,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL | ITEM_ATTACHMENT},
	{	IC_FACE,				0,			INVALIDCURS,	1,		77,		9,	1,		 400,	7,		/* Extended ear	 */		0,		-3,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL | ITEM_ELECTRONIC },

	{	IC_FACE,				0,			INVALIDCURS,	1,		74,		9,	1,		 800,	7,		/* Night goggles */		0,		-1,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL | ITEM_ELECTRONIC },
	{	IC_FACE,				0,			INVALIDCURS,	1,		55,		2,	4,		 150,	3,		/* Sun goggles	 */		0,		+3,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE },
	{	IC_FACE,				0,			INVALIDCURS,	1,	  75,		9,	1,		 100,	4,		/* Gas mask   	 */		0,		+1,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE },
	{	IC_KIT,					0,			INVALIDCURS,	2,		 5,	 10,	4,		  10,	1,		/* Canteen       */		0,		0,			ITEM_DAMAGEABLE},
	{	IC_MISC,				0,			INVALIDCURS,	1,		80,	 10,	1,		 200,	4,		/* Metal detector*/		0,		-2,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL | ITEM_ELECTRONIC },
	{	IC_MISC,				0,			INVALIDCURS,	2,		 6,		1,	4,		 900,	7,		/* Compound 18	 */		0,		0,			ITEM_DAMAGEABLE},
	{	IC_MISC,				0,			INVALIDCURS,	2,		42,		1,	4,		2500,	0,		/* Jar/QueenBlood*/		0,		0,			ITEM_DAMAGEABLE | ITEM_NOT_BUYABLE},
	{	IC_MISC,				0,			INVALIDCURS,	2,		 2,		1,	4,		 500,	1,		/* Jar/Elixir    */		0,		0,			ITEM_DAMAGEABLE },
	{	IC_MONEY,				0,			INVALIDCURS,	2,		 1,		1,	1,		   0,	0,		/* Money         */		0,		0,			ITEM_NOT_BUYABLE | ITEM_UNAERODYNAMIC},
	{ IC_MISC,				0,			JARCURS,			1,		71,		1,	2,			10, 1,		/* Glass jar		 */		0,		0,			ITEM_DAMAGEABLE},

	{ IC_MISC,				0,			INVALIDCURS,	1,		72,		5,	2,			50, 1,		/* Jar/CreatureBlood*/0,		0,			ITEM_DAMAGEABLE | ITEM_NOT_BUYABLE},
	{	IC_MISC,				0,			INVALIDCURS,	1,		70,		1,	8,		 150,	4,		/* Adren Booster */		0,		0,			ITEM_DAMAGEABLE | ITEM_UNAERODYNAMIC},
	{ IC_MISC,				0,			INVALIDCURS,	1,		47,		1,	4,		 100, 3,		/* Detonator     */		0,		+1,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_ATTACHMENT | ITEM_METAL},
	{ IC_MISC,				0,			INVALIDCURS,	1,		47,		1,	4,		 200, 6,		/* Rem Detonator */		0,		-1,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_ATTACHMENT | ITEM_METAL | ITEM_ELECTRONIC },
	{	IC_MISC,				0,			INVALIDCURS,	1,		93,		1,	8,		   0,	0,		/* Videotape     */		0,		0,			ITEM_NOT_BUYABLE},
	{	IC_MISC,				0,			INVALIDCURS,	1,		94,		1,	8,		   0,	0,		/* Deed          */		0,		0,			ITEM_NOT_BUYABLE | ITEM_UNAERODYNAMIC},
	{	IC_MISC,				0,			INVALIDCURS,	1,		56,		1,	1,		   0,	0,		/* Letter				 */		0,		0,			ITEM_NOT_BUYABLE | ITEM_UNAERODYNAMIC},
	{	IC_MISC,				0,			INVALIDCURS,	1,		59,		1,	1,		   0,	0,		/* Diskette	  	 */		0,		0,			ITEM_NOT_BUYABLE},
	{	IC_MISC,				0,			INVALIDCURS,	1,	124,		0,	1,		3000,	0,		/* Chalice	     */		0,		0,			ITEM_NOT_BUYABLE},
	{	IC_MISC,				0,			INVALIDCURS,	1,	120,		1,	4,		  50,	0,		/* Bloodcat claws*/		0,		0,			ITEM_NOT_BUYABLE | ITEM_DAMAGEABLE},

	{	IC_MISC,				0,			INVALIDCURS,	1,	121,		1,	4,		 100,	0,		/* Bloodcat teeth*/		0,		0,			ITEM_NOT_BUYABLE | ITEM_DAMAGEABLE},
	{	IC_MISC,				0,			INVALIDCURS,	3,		9,	 60,	0,		 400, 0,		/* Bloodcat pelt */		0,		0,			ITEM_NOT_BUYABLE | ITEM_DAMAGEABLE},
	{	IC_MISC,				0,			INVALIDCURS,	1,		54,		0,	99,		   0,	0,		/* Switch        */		0,		0,			ITEM_NOT_BUYABLE | ITEM_METAL},
	{	IC_MISC,				0,			INVALIDCURS,	1,		47,		0,	99,		   0,	0,		/* Action item   */		0,		0,			ITEM_NOT_BUYABLE },
	{	IC_MISC,				0,			INVALIDCURS,	1,		70,		1,	6,		 300,	6,		/* Regen Booster */		0,		0,			ITEM_DAMAGEABLE | ITEM_UNAERODYNAMIC},
	{	IC_MISC,				0,			INVALIDCURS,	1,		70,		0,	99,		   0,	0,		/* syringe 3     */		0,		0,			ITEM_NOT_BUYABLE | ITEM_UNAERODYNAMIC},
	{	IC_MISC,				0,			INVALIDCURS,	1,		70,		0,	99,		   0,	0,		/* syringe 4     */		0,		0,			ITEM_NOT_BUYABLE | ITEM_UNAERODYNAMIC},
	{	IC_MISC,				0,			INVALIDCURS,	1,		70,		0,	99,		   0,	0,		/* syringe 5     */		0,		0,			ITEM_NOT_BUYABLE | ITEM_UNAERODYNAMIC},
	{ IC_MISC,				0,			INVALIDCURS,	1,		72,		5,	2,			10, 1,		/* Jar/Human Blood*/	0,		0,			ITEM_DAMAGEABLE},
	{	IC_MISC,				0,			INVALIDCURS,	1,		0,		0,	0,		   0,	0,		/* ownership     */		0,		0,			ITEM_NOT_BUYABLE},

	{	IC_MISC,				0,			INVALIDCURS,	1,		51,		4,	4,		 750,	8,		/* Laser scope   */		0,		-1,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL | ITEM_ATTACHMENT | ITEM_ELECTRONIC },
	{	IC_MISC,				0,			REMOTECURS,		1,		54,		9,	4,		 400,	6,		/* Remote bomb trig*/	0,		-2,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL | ITEM_ELECTRONIC },
	{	IC_MISC,				0,			WIRECUTCURS,	1,		88,		4,	2,		  20,	2,		/* Wirecutters   */		0,		-4,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL},
	{	IC_MISC,				0,			INVALIDCURS,	1,		90,		9,	4,		  30,	2,		/* Duckbill      */		0,		+5,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL | ITEM_ATTACHMENT},
	{	IC_MISC,				0,			INVALIDCURS,	1,		106,	20,	1,		  30,	1,		/* Alcohol  */				0,		0,			ITEM_DAMAGEABLE},
	{	IC_FACE,				0,			INVALIDCURS,	1,		74,		11,	1,		1500,	10,		/* UV goggles */			0,		-1,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL | ITEM_ELECTRONIC },
	{	IC_MISC,				0,			INVALIDCURS,	0,		44,		21,	0,		  30,	0,		/* discarded LAW*/		0,		0,			IF_TWOHANDED_GUN | ITEM_NOT_BUYABLE },
	{	IC_MISC,				0,			INVALIDCURS,	3,		0,		40,	0,		   0,	0,		/* head - generic */	0,		0,			ITEM_DAMAGEABLE},
	{	IC_MISC,				0,			INVALIDCURS,	3,		1,		40,	0,		   0,	0,		/* head - Imposter*/	0,		0,			ITEM_DAMAGEABLE},
	{	IC_MISC,				0,			INVALIDCURS,	3,		2,		40,	0,		   0,	0,		/* head - T-Rex */		0,		0,			ITEM_DAMAGEABLE},

	{	IC_MISC,				0,			INVALIDCURS,	3,		3,		40,	0,		   0,	0,		/* head - Slay */			0,		0,			ITEM_DAMAGEABLE},
	{	IC_MISC,				0,			INVALIDCURS,	3,		4,		40,	0,		   0,	0,		/* head - Druggist */	0,		0,			ITEM_DAMAGEABLE},
	{	IC_MISC,				0,			INVALIDCURS,	3,		5,		40,	0,		   0,	0,		/* head - Matron */		0,		0,			ITEM_DAMAGEABLE},
	{	IC_MISC,				0,			INVALIDCURS,	3,		6,		40,	0,		   0,	0,		/* head - Tiffany */	0,		0,			ITEM_DAMAGEABLE},
	{	IC_MISC,				0,			INVALIDCURS,	1,		100,	12,	1,		  20,	1,		/* wine     */				0,		0,			ITEM_DAMAGEABLE},
	{	IC_MISC,				0,			INVALIDCURS,	1,		101,	4,	4,		  10,	1,		/* beer		  */				0,		0,			ITEM_DAMAGEABLE},
	{	IC_MISC,				0,			INVALIDCURS,	1,		99,		0,	2,		  20,	3,		/* pornos   */				0,		0,			ITEM_DAMAGEABLE},
	{	IC_MISC,				0,			INVALIDCURS,	0,		43,		20,	0,		 900,	6,		/* video camera */		0,		-4,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL | ITEM_ELECTRONIC  },
	{	IC_FACE,				0,			INVALIDCURS,	0,		42,		5,	1,		2500,	0,		/* robot remote */		0,		-5,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL | ITEM_ELECTRONIC  },
	{	IC_MISC,				0,			INVALIDCURS,	1,		103, 20,	0,		 500,	0,		/* creature claws */	0,		0,			ITEM_NOT_BUYABLE | ITEM_DAMAGEABLE},

	{	IC_MISC,				0,			INVALIDCURS,	3,		7,	 40,	0,		 250,	0,		/* creature flesh */	0,		0,			ITEM_NOT_BUYABLE | ITEM_DAMAGEABLE},
	{	IC_MISC,				0,			INVALIDCURS,	1,		104, 10,	0,		1000,	0,		/* creature organ */	0,		0,			ITEM_NOT_BUYABLE | ITEM_DAMAGEABLE},
	{	IC_MISC,				0,			REMOTECURS,		1,		54,		9,	4,		 400,	6,		/* Remote trigger*/		0,		-2,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL},
	{	IC_MISC,				0,			INVALIDCURS,	0,		47,		2,	8,		 500,	2,		/* gold watch */			0,		-4,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL},
	{	IC_MISC,				0,			INVALIDCURS,	1,		136, 100,	0,		 200,	2,		/* golf clubs */			0,		0,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL},
	{	IC_FACE,				0,			INVALIDCURS,	3,		11,		5,	1,		 100,	1,		/* walkman */					0,		-4,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL | ITEM_ELECTRONIC },
	{	IC_MISC,				0,			INVALIDCURS,	3,		8,	 50,	0,		 300,	2,		/* portable tv */			0,		-3,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_ELECTRONIC },
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_MISC,				0,			INVALIDCURS,	3,		10,	 10,	1,		  30,	1,		/* cigars */					0,		0,			ITEM_DAMAGEABLE },
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},

	{	IC_KEY,					0,			INVALIDCURS,	1,		82,		1,	8,		   0,	0,		/* dull gold key */		0,		0,			ITEM_NOT_BUYABLE | ITEM_METAL},
	{	IC_KEY,					1,			INVALIDCURS,	1,		83,		1,	8,		   0,	0,		/* silver key */			0,		0,			ITEM_NOT_BUYABLE | ITEM_METAL},
	{	IC_KEY,					2,			INVALIDCURS,	1,		84,		1,	8,		   0,	0,		/* diamond-shpd key */0,		0,			ITEM_NOT_BUYABLE | ITEM_METAL},
	{	IC_KEY,					3,			INVALIDCURS,	1,		87,		1,	8,		   0,	0,		/* bright gold key */	0,		0,			ITEM_NOT_BUYABLE | ITEM_METAL},
	{	IC_KEY,					4,			INVALIDCURS,	1,		91,		1,	8,		   0,	0,		/* gold key */				0,		0,			ITEM_NOT_BUYABLE | ITEM_METAL},
	{	IC_KEY,					5,			INVALIDCURS,	1,		92,		1,	8,		   0,	0,		/* small gold key */	0,		0,			ITEM_NOT_BUYABLE | ITEM_METAL},
	{	IC_KEY,					6,			INVALIDCURS,	1,		108,	1,	8,		   0,	0,		/* electronic */			0,		0,			ITEM_NOT_BUYABLE | ITEM_METAL | ITEM_ELECTRONIC },
	{	IC_KEY,					7,			INVALIDCURS,	1,		109,	1,	8,		   0,	0,		/* passcard       */	0,		0,			ITEM_NOT_BUYABLE | ITEM_METAL},
	{	IC_KEY,					8,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					9,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},

	{	IC_KEY,					10,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					11,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					12,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					13,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					14,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					15,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					16,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					17,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					18,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					19,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},

	{	IC_KEY,					20,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					21,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					22,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					23,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					24,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					25,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					26,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					27,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					28,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					29,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},

	{	IC_KEY,					30,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_KEY,					31,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* key      */				0,		0,			ITEM_NOT_EDITOR | ITEM_METAL},
	{	IC_MISC,				0,			INVALIDCURS,	2,		31,		4,	0,		 150,	2,		/* silver platter */	0,		0,			ITEM_DAMAGEABLE | ITEM_METAL},
	{	IC_MISC,				0,			INVALIDCURS,	2,		30,		1,	6,		   5,	1,		/* duct tape */				0,		0,			ITEM_DAMAGEABLE | ITEM_HIDDEN_ADDON },
	{	IC_MISC,				0,			INVALIDCURS,	2,		28,		3,	1,		  20,	0,		/* aluminum rod */		0,		0,			ITEM_NOT_BUYABLE | ITEM_DAMAGEABLE | ITEM_METAL | ITEM_HIDDEN_ADDON},
	{	IC_MISC,				0,			INVALIDCURS,	2,		32,		1,	8,		   0,	0,		/* spring */					0,		0,			ITEM_NOT_BUYABLE | ITEM_DAMAGEABLE | ITEM_METAL | ITEM_UNAERODYNAMIC | ITEM_HIDDEN_ADDON},
	{	IC_MISC,				0,			INVALIDCURS,	2,		33,		4,	1,		  25,	0,		/* a. rod & spring */	0,		0,			ITEM_NOT_BUYABLE | ITEM_REPAIRABLE | ITEM_DAMAGEABLE | ITEM_METAL | ITEM_INSEPARABLE},
	{	IC_MISC,				0,			INVALIDCURS,	1,	 133,		4,	1,		  20,	0,		/* steel rod */				0,		0,			ITEM_NOT_BUYABLE | ITEM_DAMAGEABLE | ITEM_METAL | ITEM_HIDDEN_ADDON},
	{	IC_MISC,				0,			INVALIDCURS,	1,	 132,		2,	6,		   5,	3,		/* quick glue */			0,		0,			ITEM_DAMAGEABLE  | ITEM_HIDDEN_ADDON},
	{	IC_MISC,				0,			INVALIDCURS,	1,	 134,		6,	1,		 150,	0,		/* gun barrel xtndr */0,		0,			ITEM_NOT_BUYABLE | ITEM_REPAIRABLE | ITEM_DAMAGEABLE | ITEM_METAL | ITEM_INSEPARABLE},

	{	IC_MISC,				0,			INVALIDCURS,	2,		35,		1,	8,		   0,	0,		/* string */					0,		0,			ITEM_NOT_BUYABLE | ITEM_UNAERODYNAMIC  | ITEM_HIDDEN_ADDON},
	{	IC_MISC,				0,			INVALIDCURS,	2,		27,		1,	1,		   0,	0,		/* tin can */					0,		0,			ITEM_NOT_BUYABLE | ITEM_UNAERODYNAMIC  | ITEM_HIDDEN_ADDON},
	{	IC_MISC,				0,			TINCANCURS,		2,		36,		2,	4,		   0,	0,		/* string & tin can */0,		0,			ITEM_NOT_BUYABLE | ITEM_UNAERODYNAMIC},
	{	IC_MISC,				0,			INVALIDCURS,	1,	122,		3,	6,		   5,	0,		/* marbles */					0,		0,			ITEM_NOT_BUYABLE | ITEM_DAMAGEABLE },
	{	IC_MISC,				0,			INVALIDCURS,	1,	127,		6,	1,		 200,	6,		/* lame boy */				0,		0,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_ELECTRONIC | ITEM_HIDDEN_ADDON},
	{	IC_MISC,				0,			INVALIDCURS,	1,	128,		1,	8,		   5,	1,		/* copper wire */			0,		0,			ITEM_METAL | ITEM_HIDDEN_ADDON },
	{	IC_MISC,				0,			INVALIDCURS,	1,	126,		7,	1,		  50,	0,		/* display unit */		0,		0,			ITEM_NOT_BUYABLE | ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_ELECTRONIC | ITEM_HIDDEN_ADDON},
	{	IC_MISC,				0,			INVALIDCURS,	1,	130,		1,	1,		 100,	5,		/* fumble pak */			0,		0,			ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_ELECTRONIC},
	{	IC_MISC,				0,			INVALIDCURS,	1,	131,		1,	2,		  10,	5,		/* xray bulb */				0,		0,			ITEM_DAMAGEABLE | ITEM_HIDDEN_ADDON},
	{	IC_MISC,				0,			INVALIDCURS,	2,	 29,		1,	8,		   1,	1,		/* chewing gum */			0,		0,			ITEM_DAMAGEABLE | ITEM_HIDDEN_ADDON},

	{	IC_MISC,				0,			INVALIDCURS,	1,	129, 		3,	1,		 100,	0,		/* flash device */		0,		0,			ITEM_NOT_BUYABLE | ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL | ITEM_ELECTRONIC | ITEM_HIDDEN_ADDON},
	{	IC_MISC,				0,			INVALIDCURS,	2,	 26,		1,	6,		   5,	1,		/* batteries */				0,		0,			ITEM_DAMAGEABLE},
	{	IC_MISC,				0,			INVALIDCURS,	1,	123,		1,	8,		   0,	0,		/* elastic */					0,		0,			ITEM_NOT_BUYABLE | ITEM_UNAERODYNAMIC},
	{	IC_MISC,				0,			REMOTECURS,		1,	125,	 10,	1,		2500,	0,		/* xray device */			0,		0,			ITEM_NOT_BUYABLE | ITEM_DAMAGEABLE | ITEM_REPAIRABLE | ITEM_METAL | ITEM_ELECTRONIC},
	{	IC_MONEY,				0,			INVALIDCURS,	2,	 38,		2,	1,		 100,	0,		/* silver   */				0,		0,			ITEM_NOT_BUYABLE | ITEM_DAMAGEABLE},
	{	IC_MONEY,				0,			INVALIDCURS,	1,	135,		2,	1,		 300,	0,		/* gold			*/				0,		0,			ITEM_NOT_BUYABLE | ITEM_DAMAGEABLE},
	{	IC_KIT,					0,			REFUELCURS,	  2,	 39,	 20,	0,		 250,	0,		/* gas can */					0,		0,			ITEM_DAMAGEABLE},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},

	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},

	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
	{	IC_NONE,				0,			INVALIDCURS,	0,		0,		0,	0,		   0,	0,		/* nothing! */				0,		0,			0},
};
CASSERT(lengthof(Item) == MAXITEMS)


struct AttachmentInfoStruct
{
	UINT16	usItem;
	UINT32	uiItemClass;
	INT8		bAttachmentSkillCheck;
	INT8		bAttachmentSkillCheckMod;
};


// NB hack:  if an item appears in this array with an item class of IC_MISC,
// it is a slot used for noting the skill check required for a merge or multi-item attachment

static const AttachmentInfoStruct AttachmentInfo[] =
{
	{ SILENCER,									IC_GUN,			NO_CHECK,																			0 },
	{ SNIPERSCOPE,							IC_GUN,			NO_CHECK,																			0 },
	{ LASERSCOPE,								IC_GUN,			NO_CHECK,																			0 },
	{ BIPOD,										IC_GUN,			NO_CHECK,																			0 },
	{ UNDER_GLAUNCHER,					IC_GUN,			NO_CHECK,																			0 },
	{ DUCKBILL,									IC_GUN,			NO_CHECK,																			0 },
	{	SPRING_AND_BOLT_UPGRADE,	IC_GUN,			ATTACHING_SPECIAL_ITEM_CHECK,									0 },
	{ GUN_BARREL_EXTENDER,			IC_GUN,			ATTACHING_SPECIAL_ITEM_CHECK,								  0 },
	{ DETONATOR,								IC_BOMB,		ATTACHING_DETONATOR_CHECK,										0 },
	{ REMDETONATOR,							IC_BOMB,		ATTACHING_REMOTE_DETONATOR_CHECK,						-10 },
	{ REMDETONATOR,							IC_BOMB,		ATTACHING_REMOTE_DETONATOR_CHECK,						-10 },
	{ XRAY_BULB,								IC_NONE,		ATTACHING_SPECIAL_ELECTRONIC_ITEM_CHECK,		-15 },
	{ COPPER_WIRE,							IC_NONE,		ATTACHING_SPECIAL_ELECTRONIC_ITEM_CHECK,		+20 },
	{ CERAMIC_PLATES,						IC_ARMOUR,	NO_CHECK,																			0 },
	{ NONE,                     0,          0,                                            0 }
};

UINT16 Attachment[][2] =
{
	{SILENCER, GLOCK_17},
	{SILENCER, GLOCK_18},
	{SILENCER, BERETTA_92F},
	{SILENCER, BERETTA_93R},
	{SILENCER, SW38},
	{SILENCER, BARRACUDA},
	{SILENCER, DESERTEAGLE},
	{SILENCER, M1911},
	{SILENCER, MP5K},
	{SILENCER, MAC10},
	{SILENCER, THOMPSON},
	{SILENCER, P90},

	{SNIPERSCOPE, COMMANDO},
	{SNIPERSCOPE, AKSU74},
	{SNIPERSCOPE, TYPE85},
	{SNIPERSCOPE, SKS},
	{SNIPERSCOPE, DRAGUNOV},
	{SNIPERSCOPE, M24},
	{SNIPERSCOPE, AUG},
	{SNIPERSCOPE, G41},
	{SNIPERSCOPE, MINI14},
	{SNIPERSCOPE, C7},
	{SNIPERSCOPE, FAMAS},
	{SNIPERSCOPE, AK74},
	{SNIPERSCOPE, AKM},
	{SNIPERSCOPE, M14},
	{SNIPERSCOPE, FNFAL},
	{SNIPERSCOPE, G3A3},
	{SNIPERSCOPE, G11},
	{SNIPERSCOPE, M870},
	{SNIPERSCOPE, SPAS15},
	{SNIPERSCOPE, CAWS},
	{SNIPERSCOPE, MINIMI},
	{SNIPERSCOPE, RPK74},
	{SNIPERSCOPE, HK21E},

	{LASERSCOPE, GLOCK_17},
	{LASERSCOPE, GLOCK_18},
	{LASERSCOPE, BERETTA_92F},
	{LASERSCOPE, BERETTA_93R},
	{LASERSCOPE, SW38},
	{LASERSCOPE, BARRACUDA},
	{LASERSCOPE, DESERTEAGLE},
	{LASERSCOPE, M1911},
	{LASERSCOPE, MP5K},
	{LASERSCOPE, MAC10},
	{LASERSCOPE, THOMPSON},
	{LASERSCOPE, COMMANDO},
	{LASERSCOPE, MP53},
	{LASERSCOPE, AKSU74},
	{LASERSCOPE, P90},
	{LASERSCOPE, TYPE85},
	{LASERSCOPE, SKS},
	{LASERSCOPE, DRAGUNOV},
	{LASERSCOPE, M24},
	{LASERSCOPE, AUG},
	{LASERSCOPE, G41},
	{LASERSCOPE, MINI14},
	{LASERSCOPE, C7},
	{LASERSCOPE, FAMAS},
	{LASERSCOPE, AK74},
	{LASERSCOPE, AKM},
	{LASERSCOPE, M14},
	{LASERSCOPE, FNFAL},
	{LASERSCOPE, G3A3},
	{LASERSCOPE, G11},
	{LASERSCOPE, M870},
	{LASERSCOPE, SPAS15},
	{LASERSCOPE, CAWS},
	{LASERSCOPE, MINIMI},
	{LASERSCOPE, RPK74},
	{LASERSCOPE, HK21E},
	{LASERSCOPE, AUTOMAG_III},

	{BIPOD,	SKS},
	{BIPOD,	DRAGUNOV},
	{BIPOD,	M24},
	{BIPOD,	AUG},
	{BIPOD,	G41},
	{BIPOD,	MINI14},
	{BIPOD,	C7},
	{BIPOD,	FAMAS},
	{BIPOD,	AK74},
	{BIPOD,	AKM},
	{BIPOD,	M14},
	{BIPOD,	FNFAL},
	{BIPOD,	G3A3},
	{BIPOD,	G11},
	{BIPOD,	CAWS},
	{BIPOD,	MINIMI},
	{BIPOD,	RPK74},
	{BIPOD,	HK21E},

	{DUCKBILL, M870},
	{DUCKBILL, SPAS15},
	{DUCKBILL, CAWS},

	{UNDER_GLAUNCHER, COMMANDO},
	{UNDER_GLAUNCHER, AKSU74},
	{UNDER_GLAUNCHER, AUG},
	{UNDER_GLAUNCHER, G41},
	{UNDER_GLAUNCHER, MINI14},
	{UNDER_GLAUNCHER, C7},
	{UNDER_GLAUNCHER, FAMAS},
	{UNDER_GLAUNCHER, AK74},
	{UNDER_GLAUNCHER, AKM},
	{UNDER_GLAUNCHER, M14},
	{UNDER_GLAUNCHER, FNFAL},
	{UNDER_GLAUNCHER, G3A3},
	{UNDER_GLAUNCHER, MINIMI},
	{UNDER_GLAUNCHER, RPK74},
	{UNDER_GLAUNCHER, HK21E},

	{SPRING_AND_BOLT_UPGRADE, GLOCK_17},
	{SPRING_AND_BOLT_UPGRADE, GLOCK_18},
	{SPRING_AND_BOLT_UPGRADE, BERETTA_92F},
	{SPRING_AND_BOLT_UPGRADE, BERETTA_93R},
	{SPRING_AND_BOLT_UPGRADE, SW38},
	{SPRING_AND_BOLT_UPGRADE, BARRACUDA},
	{SPRING_AND_BOLT_UPGRADE, DESERTEAGLE},
	{SPRING_AND_BOLT_UPGRADE, M1911},
	{SPRING_AND_BOLT_UPGRADE, MP5K},
	{SPRING_AND_BOLT_UPGRADE, MAC10},
	{SPRING_AND_BOLT_UPGRADE, THOMPSON},
	{SPRING_AND_BOLT_UPGRADE, COMMANDO},
	{SPRING_AND_BOLT_UPGRADE, MP53},
	{SPRING_AND_BOLT_UPGRADE, AKSU74},
	{SPRING_AND_BOLT_UPGRADE, P90},
	{SPRING_AND_BOLT_UPGRADE, TYPE85},
	{SPRING_AND_BOLT_UPGRADE, SKS},
	{SPRING_AND_BOLT_UPGRADE, DRAGUNOV},
	{SPRING_AND_BOLT_UPGRADE, M24},
	{SPRING_AND_BOLT_UPGRADE, AUG},
	{SPRING_AND_BOLT_UPGRADE, G41},
	{SPRING_AND_BOLT_UPGRADE, MINI14},
	{SPRING_AND_BOLT_UPGRADE, C7},
	{SPRING_AND_BOLT_UPGRADE, FAMAS},
	{SPRING_AND_BOLT_UPGRADE, AK74},
	{SPRING_AND_BOLT_UPGRADE, AKM},
	{SPRING_AND_BOLT_UPGRADE, M14},
	{SPRING_AND_BOLT_UPGRADE, FNFAL},
	{SPRING_AND_BOLT_UPGRADE, G3A3},
	{SPRING_AND_BOLT_UPGRADE, G11},
	{SPRING_AND_BOLT_UPGRADE, M870},
	{SPRING_AND_BOLT_UPGRADE, SPAS15},
	{SPRING_AND_BOLT_UPGRADE, CAWS},
	{SPRING_AND_BOLT_UPGRADE, MINIMI},
	{SPRING_AND_BOLT_UPGRADE, RPK74},
	{SPRING_AND_BOLT_UPGRADE, HK21E},
	{SPRING_AND_BOLT_UPGRADE, AUTOMAG_III},

	{GUN_BARREL_EXTENDER, GLOCK_17},
	{GUN_BARREL_EXTENDER, GLOCK_18},
	{GUN_BARREL_EXTENDER, BERETTA_92F},
	{GUN_BARREL_EXTENDER, BERETTA_93R},
	{GUN_BARREL_EXTENDER, SW38},
	{GUN_BARREL_EXTENDER, BARRACUDA},
	{GUN_BARREL_EXTENDER, DESERTEAGLE},
	{GUN_BARREL_EXTENDER, M1911},
	{GUN_BARREL_EXTENDER, MP5K},
	{GUN_BARREL_EXTENDER, MAC10},
	{GUN_BARREL_EXTENDER, THOMPSON},
	{GUN_BARREL_EXTENDER, COMMANDO},
	{GUN_BARREL_EXTENDER, MP53},
	{GUN_BARREL_EXTENDER, AKSU74},
	{GUN_BARREL_EXTENDER, P90},
	{GUN_BARREL_EXTENDER, TYPE85},
	{GUN_BARREL_EXTENDER, SKS},
	{GUN_BARREL_EXTENDER, DRAGUNOV},
	{GUN_BARREL_EXTENDER, M24},
	{GUN_BARREL_EXTENDER, AUG},
	{GUN_BARREL_EXTENDER, G41},
	{GUN_BARREL_EXTENDER, MINI14},
	{GUN_BARREL_EXTENDER, C7},
	{GUN_BARREL_EXTENDER, FAMAS},
	{GUN_BARREL_EXTENDER, AK74},
	{GUN_BARREL_EXTENDER, AKM},
	{GUN_BARREL_EXTENDER, M14},
	{GUN_BARREL_EXTENDER, FNFAL},
	{GUN_BARREL_EXTENDER, G3A3},
	{GUN_BARREL_EXTENDER, G11},
	{GUN_BARREL_EXTENDER, M870},
	{GUN_BARREL_EXTENDER, SPAS15},
	{GUN_BARREL_EXTENDER, CAWS},
	{GUN_BARREL_EXTENDER, MINIMI},
	{GUN_BARREL_EXTENDER, RPK74},
	{GUN_BARREL_EXTENDER, HK21E},
	{GUN_BARREL_EXTENDER, AUTOMAG_III},

	{DETONATOR, TNT},
	{DETONATOR, HMX},
	{DETONATOR, C1},
	{DETONATOR, C4},

	{REMDETONATOR, TNT},
	{REMDETONATOR, HMX},
	{REMDETONATOR, C1},
	{REMDETONATOR, C4},

	{CERAMIC_PLATES, FLAK_JACKET},
	{CERAMIC_PLATES, FLAK_JACKET_18},
	{CERAMIC_PLATES, FLAK_JACKET_Y},
	{CERAMIC_PLATES, KEVLAR_VEST},
	{CERAMIC_PLATES, KEVLAR_VEST_18},
	{CERAMIC_PLATES, KEVLAR_VEST_Y},
	{CERAMIC_PLATES, SPECTRA_VEST},
	{CERAMIC_PLATES, SPECTRA_VEST_18},
	{CERAMIC_PLATES, SPECTRA_VEST_Y},
	{CERAMIC_PLATES, KEVLAR2_VEST},
	{CERAMIC_PLATES, KEVLAR2_VEST_18},
	{CERAMIC_PLATES, KEVLAR2_VEST_Y},

	{SPRING,					ALUMINUM_ROD},
	{QUICK_GLUE,			STEEL_ROD},
	{DUCT_TAPE,				STEEL_ROD},
	{XRAY_BULB,				FUMBLE_PAK},
	{CHEWING_GUM,			FUMBLE_PAK},
	{BATTERIES,				XRAY_DEVICE},
	{COPPER_WIRE,			LAME_BOY},
	{0, 0}
};

UINT16 Launchable[][2] =
{
	{GL_HE_GRENADE, GLAUNCHER},
	{GL_HE_GRENADE, UNDER_GLAUNCHER},
	{GL_TEARGAS_GRENADE, GLAUNCHER},
	{GL_TEARGAS_GRENADE, UNDER_GLAUNCHER},
	{GL_STUN_GRENADE, GLAUNCHER},
	{GL_STUN_GRENADE, UNDER_GLAUNCHER},
	{GL_SMOKE_GRENADE, GLAUNCHER},
	{GL_SMOKE_GRENADE, UNDER_GLAUNCHER},
	{MORTAR_SHELL, MORTAR},
	{TANK_SHELL, TANK_CANNON},
	{0, 0}
};

static UINT16 const CompatibleFaceItems[][2] =
{
	{ NIGHTGOGGLES, EXTENDEDEAR },
	{ NIGHTGOGGLES, WALKMAN     },
	{ SUNGOGGLES,   EXTENDEDEAR },
	{ SUNGOGGLES,   WALKMAN     },
	{ UVGOGGLES,    EXTENDEDEAR },
	{ UVGOGGLES,    WALKMAN     },
	{ GASMASK,      EXTENDEDEAR },
	{ GASMASK,      WALKMAN     },

	{ NOTHING,      NOTHING     }
};


enum MergeType
{
	DESTRUCTION,
	COMBINE_POINTS,
	TREAT_ARMOUR,
	EXPLOSIVE,
	EASY_MERGE,
	ELECTRONIC_MERGE
};


struct MergeInfo
{
	UINT16 item1;
	UINT16 item2;
	UINT16 result;
	UINT16 action;
};


static MergeInfo const Merge[] =
{ // first item			second item						resulting item,					merge type
	{FIRSTAIDKIT,			FIRSTAIDKIT,					FIRSTAIDKIT,						COMBINE_POINTS},
	{MEDICKIT,				MEDICKIT,							MEDICKIT,								COMBINE_POINTS},
	{LOCKSMITHKIT,		LOCKSMITHKIT,					LOCKSMITHKIT,						COMBINE_POINTS},
	{TOOLKIT,					TOOLKIT,							TOOLKIT,								COMBINE_POINTS},
	{GAS_CAN,					GAS_CAN,							GAS_CAN,								COMBINE_POINTS},
	{CAMOUFLAGEKIT,		CAMOUFLAGEKIT,				CAMOUFLAGEKIT,					COMBINE_POINTS},
	{BEER,						BEER,									BEER,										COMBINE_POINTS},
	{WINE,						WINE,									WINE,										COMBINE_POINTS},
	{ALCOHOL,					ALCOHOL,							ALCOHOL,								COMBINE_POINTS},

	{COMPOUND18,			FLAK_JACKET,							FLAK_JACKET_18,							TREAT_ARMOUR},
	{COMPOUND18,			KEVLAR_VEST,							KEVLAR_VEST_18,							TREAT_ARMOUR},
	{COMPOUND18,			KEVLAR2_VEST,							KEVLAR2_VEST_18,						TREAT_ARMOUR},
	{COMPOUND18,			SPECTRA_VEST,							SPECTRA_VEST_18,						TREAT_ARMOUR},
	{COMPOUND18,			LEATHER_JACKET_W_KEVLAR,	LEATHER_JACKET_W_KEVLAR_18,	TREAT_ARMOUR},
	{COMPOUND18,			KEVLAR_LEGGINGS,					KEVLAR_LEGGINGS_18,					TREAT_ARMOUR},
	{COMPOUND18,			SPECTRA_LEGGINGS,					SPECTRA_LEGGINGS_18,				TREAT_ARMOUR},
	{COMPOUND18,			KEVLAR_HELMET,						KEVLAR_HELMET_18,						TREAT_ARMOUR},
	{COMPOUND18,			SPECTRA_HELMET,						SPECTRA_HELMET_18,					TREAT_ARMOUR},
	{COMPOUND18,			FLAK_JACKET_Y,						NOTHING,										DESTRUCTION},
	{COMPOUND18,			KEVLAR_VEST_Y,						NOTHING,										DESTRUCTION},
	{COMPOUND18,			KEVLAR2_VEST_Y,						NOTHING,										DESTRUCTION},
	{COMPOUND18,			SPECTRA_VEST_Y,						NOTHING,										DESTRUCTION},
	{COMPOUND18,			LEATHER_JACKET_W_KEVLAR_Y,NOTHING,										DESTRUCTION},
	{COMPOUND18,			KEVLAR_LEGGINGS_Y,				NOTHING,										DESTRUCTION},
	{COMPOUND18,			SPECTRA_LEGGINGS_Y,				NOTHING,										DESTRUCTION},
	{COMPOUND18,			KEVLAR_HELMET_Y,					NOTHING,										DESTRUCTION},
	{COMPOUND18,			SPECTRA_HELMET_Y,					NOTHING,										DESTRUCTION},

	{JAR_QUEEN_CREATURE_BLOOD,	FLAK_JACKET,					FLAK_JACKET_Y,					TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	KEVLAR_VEST,					KEVLAR_VEST_Y,					TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	SPECTRA_VEST,					SPECTRA_VEST_Y,					TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	LEATHER_JACKET_W_KEVLAR,	LEATHER_JACKET_W_KEVLAR_Y,	TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	KEVLAR2_VEST,					KEVLAR2_VEST_Y,				TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	KEVLAR_LEGGINGS,			KEVLAR_LEGGINGS_Y,			TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	SPECTRA_LEGGINGS,			SPECTRA_LEGGINGS_Y,			TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	KEVLAR_HELMET,				KEVLAR_HELMET_Y,				TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	SPECTRA_HELMET,				SPECTRA_HELMET_Y,				TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	FLAK_JACKET_18,				NOTHING,								DESTRUCTION},
	{JAR_QUEEN_CREATURE_BLOOD,	KEVLAR_VEST_18,				NOTHING,								DESTRUCTION},
	{JAR_QUEEN_CREATURE_BLOOD,	KEVLAR2_VEST_18,			NOTHING,								DESTRUCTION},
	{JAR_QUEEN_CREATURE_BLOOD,	SPECTRA_VEST_18,			NOTHING,								DESTRUCTION},
	{JAR_QUEEN_CREATURE_BLOOD,	LEATHER_JACKET_W_KEVLAR_18,	NOTHING,					DESTRUCTION},
	{JAR_QUEEN_CREATURE_BLOOD,	KEVLAR_LEGGINGS_18,		NOTHING,								DESTRUCTION},
	{JAR_QUEEN_CREATURE_BLOOD,	SPECTRA_LEGGINGS_18,	NOTHING,								DESTRUCTION},
	{JAR_QUEEN_CREATURE_BLOOD,	KEVLAR_HELMET_18,			NOTHING,								DESTRUCTION},
	{JAR_QUEEN_CREATURE_BLOOD,	SPECTRA_HELMET_18,		NOTHING,								DESTRUCTION},

	{RDX,							TNT,									HMX,										EXPLOSIVE},
	{RDX,							C1,										C4,											EXPLOSIVE},
	{TNT,							RDX,									HMX,										EXPLOSIVE},
	{C1,							RDX,									C4,											EXPLOSIVE},

	{STRING,					TIN_CAN,							STRING_TIED_TO_TIN_CAN,	EASY_MERGE},
	{TIN_CAN,					STRING,								STRING_TIED_TO_TIN_CAN,	EASY_MERGE},

	{FLASH_DEVICE,		DISPLAY_UNIT,					XRAY_DEVICE,						ELECTRONIC_MERGE},
	{DISPLAY_UNIT,		FLASH_DEVICE,					XRAY_DEVICE,						ELECTRONIC_MERGE},

	{ NOTHING,  NOTHING, NOTHING, DESTRUCTION}
};

struct ComboMergeInfoStruct
{
	UINT16	usItem;
	UINT16	usAttachment[2];
	UINT16	usResult;
};


ComboMergeInfoStruct AttachmentComboMerge[] =
{
	// base item							attach 1								attach 2						 result
	{ALUMINUM_ROD,						{SPRING,								NOTHING},						SPRING_AND_BOLT_UPGRADE	},
	{STEEL_ROD,								{QUICK_GLUE,						DUCT_TAPE},					GUN_BARREL_EXTENDER },
	{FUMBLE_PAK,							{XRAY_BULB,							CHEWING_GUM},				FLASH_DEVICE },
	{LAME_BOY,								{COPPER_WIRE,						NOTHING},						DISPLAY_UNIT },
	{NOTHING,									{NOTHING,								NOTHING},						NOTHING },
};

UINT16 ReplacementGuns[][2] =
{
	{ BARRACUDA,		DESERTEAGLE	},
	{ M1911,				GLOCK_17		},
	{ GLOCK_18,			BERETTA_93R	},
	{ BERETTA_92F,	GLOCK_17		},
	{ TYPE85,				BERETTA_93R	},
	{	THOMPSON,			MP5K				},
	{	MP53,					MP5K				},
	{ SPAS15,				M870				},
	{ AKSU74,				MAC10				},
	{ SKS,					MINI14			},
	{	AKM,					G41					},
	{ G3A3,					G41					},
	{ AK74,					G41					},
	{	DRAGUNOV,			M24					},
	{ FAMAS,				M14					},
	{ AUG,					C7					},
	{ RPK74,				MINIMI			},
	{ HK21E,				MINIMI			},
	{ 0,						0						}
};

UINT16 ReplacementAmmo[][2] =
{
	{ CLIP545_30_AP,		CLIP556_30_AP  },
	{ CLIP545_30_HP,		CLIP556_30_HP  },
	{ CLIP762W_10_AP,		CLIP762N_5_AP  },
	{ CLIP762W_30_AP,		CLIP762N_20_AP },
	{ CLIP762W_10_HP,		CLIP762N_5_HP  },
	{ CLIP762W_30_HP,		CLIP762N_20_HP },
	{ 0,								0							 }
};

BOOLEAN ItemIsLegal( UINT16 usItemIndex )
{
	//if the user has selected the reduced gun list
	if( !gGameOptions.fGunNut )
	{
		//if the item is a gun, or ammo
		if( (Item[ usItemIndex ].usItemClass == IC_GUN) || (Item[ usItemIndex ].usItemClass == IC_AMMO ))
		{
			// and the item is only available with the extended guns
			if( ExtendedGunListGun( usItemIndex ) )
			{
				return(FALSE);
			}
		}
	}

	return(TRUE);
}

// also used for ammo
BOOLEAN ExtendedGunListGun( UINT16 usGun )
{
	return( (Item[ usGun ].fFlags & ITEM_BIGGUNLIST) != 0 );
}

UINT16 StandardGunListReplacement( UINT16 usGun )
{
	UINT8 ubLoop;

	if ( ExtendedGunListGun( usGun ) )
	{
		ubLoop = 0;
		while ( ReplacementGuns[ ubLoop ][ 0 ] != 0 )
		{
			if ( ReplacementGuns[ ubLoop ][ 0 ] == usGun )
			{
				return( ReplacementGuns[ ubLoop ][ 1 ] );
			}
			ubLoop++;
		}
		// ERROR!
		AssertMsg( 0, String( "Extended gun with no replacement %d, CC:0", usGun ) );
		return( NOTHING );
	}
	else
	{
		return( NOTHING );
	}
}

UINT16 StandardGunListAmmoReplacement( UINT16 usAmmo )
{
	UINT8 ubLoop;

	if ( ExtendedGunListGun( usAmmo ) )
	{
		ubLoop = 0;
		while ( ReplacementAmmo[ ubLoop ][ 0 ] != 0 )
		{
			if ( ReplacementAmmo[ ubLoop ][ 0 ] == usAmmo )
			{
				return( ReplacementAmmo[ ubLoop ][ 1 ] );
			}
			ubLoop++;
		}
		// ERROR!
		AssertMsg( 0, String( "Extended gun with no replacement %d, CC:0", usAmmo ) );
		return( NOTHING );
	}
	else
	{
		return( NOTHING );
	}
}


BOOLEAN WeaponInHand(const SOLDIERTYPE* const pSoldier)
{
	if ( Item[pSoldier->inv[HANDPOS].usItem].usItemClass & (IC_WEAPON | IC_THROWN) )
	{
		OBJECTTYPE const& o = pSoldier->inv[HANDPOS];
		if (HasObjectImprint(o))
		{
			if (pSoldier->ubProfile != NO_PROFILE)
			{
				if (pSoldier->inv[HANDPOS].ubImprintID != pSoldier->ubProfile)
				{
					return( FALSE );
				}
			}
			else
			{
				if (pSoldier->inv[HANDPOS].ubImprintID != (NO_PROFILE + 1) )
				{
					return( FALSE );
				}
			}
		}
		if (pSoldier->inv[HANDPOS].bGunStatus >= USABLE)
		{
			return( TRUE );
		}
	}
	// return -1 or some "broken" value if weapon is broken?
	return( FALSE );
}

UINT8 ItemSlotLimit( UINT16 usItem, INT8 bSlot )
{
	UINT8 ubSlotLimit;

	if ( bSlot < BIGPOCK1POS )
	{
		return( 1 );
	}
	else
	{
		ubSlotLimit = Item[usItem].ubPerPocket;
		if (bSlot >= SMALLPOCK1POS && ubSlotLimit > 1)
		{
			ubSlotLimit /= 2;
		}
		return( ubSlotLimit );
	}
}

UINT32 MoneySlotLimit( INT8 bSlot )
{
	if ( bSlot >= SMALLPOCK1POS )
	{
		return( MAX_MONEY_PER_SLOT / 2 );
	}
	else
	{
		return( MAX_MONEY_PER_SLOT );
	}
}


INT8 FindObj(const SOLDIERTYPE* pSoldier, UINT16 usItem)
{
	INT8 bLoop;

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if (pSoldier->inv[bLoop].usItem == usItem)
		{
			return( bLoop );
		}
	}
	return( NO_SLOT );
}

INT8 FindUsableObj( SOLDIERTYPE * pSoldier, UINT16 usItem )
{
	INT8 bLoop;

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if ( pSoldier->inv[bLoop].usItem == usItem && pSoldier->inv[bLoop].bStatus[0] >= USABLE )
		{
			return( bLoop );
		}
	}
	return( NO_SLOT );
}


static INT8 FindObjExcludingSlot(const SOLDIERTYPE* pSoldier, UINT16 usItem, INT8 bExcludeSlot)
{
	INT8 bLoop;

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if (bLoop == bExcludeSlot)
		{
			continue;
		}
		if (pSoldier->inv[bLoop].usItem == usItem)
		{
			return( bLoop );
		}
	}
	return( NO_SLOT );
}

INT8 FindExactObj( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj )
{
	INT8 bLoop;

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if (pObj == &pSoldier->inv[bLoop]) return bLoop;
	}
	return( NO_SLOT );
}


INT8 FindObjWithin( SOLDIERTYPE * pSoldier, UINT16 usItem, INT8 bLower, INT8 bUpper )
{
	INT8	bLoop;

	for (bLoop = bLower; bLoop <= bUpper; bLoop++)
	{
		if (pSoldier->inv[bLoop].usItem == usItem)
		{
			return( bLoop );
		}
	}
	return( ITEM_NOT_FOUND );
}


INT8 FindObjInObjRange(const SOLDIERTYPE* const pSoldier, UINT16 usItem1, UINT16 usItem2)
{
	INT8		bLoop;
	UINT16	usTemp;

	if (usItem1 > usItem2 )
	{
		// swap the two...
		usTemp = usItem2;
		usItem2 = usItem1;
		usItem1 = usTemp;
	}

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		usTemp = pSoldier->inv[bLoop].usItem;
		if ( usTemp >= usItem1 && usTemp <= usItem2 )
		{
			return( bLoop );
		}
	}

	return( ITEM_NOT_FOUND );
}


INT8 FindObjClass(const SOLDIERTYPE* const pSoldier, const UINT32 usItemClass)
{
	INT8 bLoop;

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if (Item[pSoldier->inv[bLoop].usItem].usItemClass & usItemClass)
		{
			return( bLoop );
		}
	}
	return( NO_SLOT );
}


INT8 FindAIUsableObjClass( SOLDIERTYPE * pSoldier, 	UINT32 usItemClass )
{
	// finds the first object of the specified class which does NOT have
	// the "unusable by AI" flag set.

	// uses & rather than == so that this function can search for any weapon
	INT8 bLoop;

	// This is for the AI only so:

	// Do not consider tank cannons or rocket launchers to be "guns"

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if ( (Item[pSoldier->inv[bLoop].usItem].usItemClass & usItemClass) && !(pSoldier->inv[bLoop].fFlags & OBJECT_AI_UNUSABLE) && (pSoldier->inv[bLoop].bStatus[0] >= USABLE ) )
		{
			if ( usItemClass == IC_GUN && EXPLOSIVE_GUN( pSoldier->inv[bLoop].usItem ) )
			{
				continue;
			}
			return( bLoop );
		}
	}
	return( NO_SLOT );
}

INT8 FindAIUsableObjClassWithin( SOLDIERTYPE * pSoldier, 	UINT32 usItemClass, INT8 bLower, INT8 bUpper )
{
	INT8 bLoop;

	// This is for the AI only so:
	// Do not consider tank cannons or rocket launchers to be "guns"

	for (bLoop = bLower; bLoop <= bUpper; bLoop++)
	{
		if ( (Item[pSoldier->inv[bLoop].usItem].usItemClass & usItemClass) && !(pSoldier->inv[bLoop].fFlags & OBJECT_AI_UNUSABLE) && (pSoldier->inv[bLoop].bStatus[0] >= USABLE ) )
		{
			if ( usItemClass == IC_GUN && EXPLOSIVE_GUN( pSoldier->inv[bLoop].usItem ) )
			{
				continue;
			}
			return( bLoop );
		}
	}
	return( NO_SLOT );
}

INT8 FindEmptySlotWithin( SOLDIERTYPE * pSoldier, INT8 bLower, INT8 bUpper )
{
	INT8	bLoop;

	for (bLoop = bLower; bLoop <= bUpper; bLoop++)
	{
		if (pSoldier->inv[bLoop].usItem == 0)
		{
			if (bLoop == SECONDHANDPOS && Item[pSoldier->inv[HANDPOS].usItem].fFlags & ITEM_TWO_HANDED)
			{
				continue;
			}
			else
			{
				return( bLoop );
			}
		}
	}
	return( ITEM_NOT_FOUND );
}


static BOOLEAN GLGrenadeInSlot(SOLDIERTYPE* pSoldier, INT8 bSlot)
{
	switch (pSoldier->inv[bSlot].usItem)
	{
		case GL_HE_GRENADE:
		case GL_TEARGAS_GRENADE:
		case GL_STUN_GRENADE:
		case GL_SMOKE_GRENADE:
			return(TRUE);
		default:
			return(FALSE);
	}
}

// for grenade launchers
INT8 FindGLGrenade( SOLDIERTYPE * pSoldier )
{
	INT8 bLoop;

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if (GLGrenadeInSlot( pSoldier, bLoop ))
		{
			return( bLoop );
		}
	}
	return( NO_SLOT );
}

INT8 FindThrowableGrenade( SOLDIERTYPE * pSoldier )
{
	INT8 bLoop;
	BOOLEAN fCheckForFlares = FALSE;

	// JA2Gold: give some priority to looking for flares when at night
	// this is AI only so we can put in some customization for night
	if (GetTimeOfDayAmbientLightLevel() == NORMAL_LIGHTLEVEL_NIGHT)
	{
		if (pSoldier->bLife > (pSoldier->bLifeMax / 2))
		{
			fCheckForFlares = TRUE;
		}
	}
	if (fCheckForFlares)
	{
		// Do a priority check for flares first
		for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
		{
			if (pSoldier->inv[ bLoop ].usItem == BREAK_LIGHT)
			{
				return( bLoop );
			}
		}
	}

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if ( (Item[ pSoldier->inv[ bLoop ].usItem ].usItemClass & IC_GRENADE) && !GLGrenadeInSlot( pSoldier, bLoop ) )
		{
			return( bLoop );
		}
	}
	return( NO_SLOT );
}


INT8 FindAttachment(const OBJECTTYPE* pObj, UINT16 usItem)
{
	INT8	bLoop;

	for (bLoop = 0; bLoop < MAX_ATTACHMENTS; bLoop++)
	{
		if (pObj->usAttachItem[bLoop] == usItem)
		{
			return( bLoop );
		}
	}
	return( ITEM_NOT_FOUND );
}


INT8 FindAttachmentByClass(OBJECTTYPE const* const pObj, UINT32 const uiItemClass)
{
	INT8	bLoop;

	for (bLoop = 0; bLoop < MAX_ATTACHMENTS; bLoop++)
	{
		if (Item[ pObj->usAttachItem[bLoop] ].usItemClass == uiItemClass)
		{
			return( bLoop );
		}
	}
	return( ITEM_NOT_FOUND );
}

INT8 FindLaunchable( SOLDIERTYPE * pSoldier, UINT16 usWeapon )
{
	INT8	bLoop;

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if ( ValidLaunchable( pSoldier->inv[ bLoop ].usItem , usWeapon ) )
		{
			return( bLoop );
		}
	}
	return( ITEM_NOT_FOUND );
}


INT8 FindLaunchableAttachment(const OBJECTTYPE* const pObj, const UINT16 usWeapon)
{
	INT8		bLoop;

	for ( bLoop = 0; bLoop < MAX_ATTACHMENTS; bLoop++ )
	{
		if ( pObj->usAttachItem[ bLoop ] != NOTHING && ValidLaunchable( pObj->usAttachItem[ bLoop ], usWeapon ) )
		{
			return( bLoop );
		}
	}

	return( ITEM_NOT_FOUND );
}


bool ItemHasAttachments(OBJECTTYPE const& o)
{
	return
		o.usAttachItem[0] != NOTHING ||
		o.usAttachItem[1] != NOTHING ||
		o.usAttachItem[2] != NOTHING ||
		o.usAttachItem[3] != NOTHING;
}


// Determine if it is possible to add this attachment to the CLASS of this item
// (i.e. to any item in the class)
static BOOLEAN ValidAttachmentClass(UINT16 usAttachment, UINT16 usItem)
{
	for (const AttachmentInfoStruct* i = AttachmentInfo; i->usItem != NONE; ++i)
	{
		// see comment for AttachmentInfo array for why we skip IC_NONE
		if (i->uiItemClass == IC_NONE) continue;

		if (i->usItem == usAttachment && i->uiItemClass == Item->usItemClass)
		{
			return TRUE;
		}
	}
	return FALSE;
}


static const AttachmentInfoStruct* GetAttachmentInfo(const UINT16 usItem)
{
	for (const AttachmentInfoStruct* i = AttachmentInfo; i->usItem != NONE; ++i)
	{
		if (i->usItem == usItem) return i;
	}
	return NULL;
}

//Determine if it is possible to add this attachment to the item.
BOOLEAN ValidAttachment( UINT16 usAttachment, UINT16 usItem )
{
	INT32 iLoop = 0;

	// look for the section of the array pertaining to this attachment...
	while( 1 )
	{
		if (Attachment[iLoop][0] == usAttachment)
		{
			break;
		}
		iLoop++;
		if (Attachment[iLoop][0] == 0)
		{
			// the proposed item cannot be attached to anything!
			return( FALSE );
		}
	}
	// now look through this section for the item in question
	while( 1 )
	{
		if (Attachment[iLoop][1] == usItem)
		{
			break;
		}
		iLoop++;
		if (Attachment[iLoop][0] != usAttachment)
		{
			// the proposed item cannot be attached to the item in question
			return( FALSE );
		}
	}
	return( TRUE );
}


BOOLEAN ValidItemAttachment(const OBJECTTYPE* const pObj, const UINT16 usAttachment, const BOOLEAN fAttemptingAttachment)
{
	BOOLEAN		fSameItem = FALSE, fSimilarItems = FALSE;
	UINT16		usSimilarItem = NOTHING;

	if ( !ValidAttachment( usAttachment, pObj->usItem ) )
	{
		// check for an underslung grenade launcher attached to the gun
		if ( (FindAttachment( pObj, UNDER_GLAUNCHER ) != ITEM_NOT_FOUND) && ValidLaunchable( usAttachment, UNDER_GLAUNCHER ) )
		{
			return ( TRUE );
			/*
			if ( fAttemptingAttachment )
			{
				// if there is no other grenade attached already, then we can attach it
				if (FindAttachmentByClass( pObj, IC_GRENADE) != ITEM_NOT_FOUND)
				{
					return( FALSE );
				}
				// keep going, it can be attached to the grenade launcher
			}
			else
			{
				// logically, can be added
				return( TRUE );
			}
			*/
		}
		else
		{
			if ( fAttemptingAttachment && ValidAttachmentClass( usAttachment, pObj->usItem ) )
			{
				// well, maybe the player thought he could
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, Message[STR_CANT_ATTACH], ItemNames[usAttachment], ItemNames[pObj->usItem]);
			}

			return( FALSE );
		}
	}
	// special conditions go here
	// can't have two of the same attachment on an item
	/*
	if (FindAttachment( pObj, usAttachment ) != ITEM_NOT_FOUND)
	{
		fSameItem = TRUE;
	}
	*/

	// special code for items which won't attach if X is present
	switch( usAttachment )
	{
		case BIPOD:
			if ( FindAttachment( pObj, UNDER_GLAUNCHER) != ITEM_NOT_FOUND )
			{
				fSimilarItems = TRUE;
				usSimilarItem = UNDER_GLAUNCHER;
			}
			break;
		case UNDER_GLAUNCHER:
			if ( FindAttachment( pObj, BIPOD ) != ITEM_NOT_FOUND )
			{
				fSimilarItems = TRUE;
				usSimilarItem = BIPOD;
			}
			break;
	/*
		case LASERSCOPE:
			if (FindAttachment( pObj, SNIPERSCOPE ) != ITEM_NOT_FOUND)
			{
				return( FALSE );
			}
			break;
		case SNIPERSCOPE:
			if (FindAttachment( pObj, LASERSCOPE ) != ITEM_NOT_FOUND)
			{
				return( FALSE );
			}
			break;
			*/
		case DETONATOR:
			if( FindAttachment( pObj, REMDETONATOR ) != ITEM_NOT_FOUND )
			{
				fSameItem = TRUE;
			}
			break;
		case REMDETONATOR:
			if( FindAttachment( pObj, DETONATOR ) != ITEM_NOT_FOUND )
			{
				fSameItem = TRUE;
			}
			break;
	}

	if (fAttemptingAttachment)
	{
		if (fSameItem)
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, Message[ STR_ATTACHMENT_ALREADY ] );
			return( FALSE );
		}
		else if (fSimilarItems)
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, Message[ STR_CANT_USE_TWO_ITEMS ], ItemNames[ usSimilarItem ], ItemNames[ usAttachment ] );
			return( FALSE );
		}
	}

	return( TRUE );
}

//Determines if it is possible to equip this weapon with this ammo.
BOOLEAN ValidAmmoType( UINT16 usItem, UINT16 usAmmoType )
{
	if (Item[usItem].usItemClass == IC_GUN && Item[usAmmoType].usItemClass == IC_AMMO)
	{
		if (Weapon[usItem].ubCalibre == Magazine[Item[usAmmoType].ubClassIndex].ubCalibre)
		{
			return( TRUE );
		}
	}
	return( FALSE );
}


BOOLEAN CompatibleFaceItem(UINT16 const item1, UINT16 const item2)
{
	if (item2 == NOTHING) return TRUE;
	for (UINT16 const (*i)[2] = CompatibleFaceItems; (*i)[0] != NOTHING; ++i)
	{
		if ((*i)[0] == item1 && (*i)[1] == item2) return TRUE;
		if ((*i)[0] == item2 && (*i)[1] == item1) return TRUE;
	}
	return FALSE;
}


//Determines if this item is a two handed item.
static BOOLEAN TwoHandedItem(UINT16 usItem)
{
	if (Item[usItem].fFlags & ITEM_TWO_HANDED)
	{
		return( TRUE );
	}
	return FALSE;
}

BOOLEAN ValidLaunchable( UINT16 usLaunchable, UINT16 usItem )
{
	INT32 iLoop = 0;

	// look for the section of the array pertaining to this launchable item...
	while( 1 )
	{
		if (Launchable[iLoop][0] == usLaunchable)
		{
			break;
		}
		iLoop++;
		if (Launchable[iLoop][0] == 0)
		{
			// the proposed item cannot be attached to anything!
			return( FALSE );
		}
	}
	// now look through this section for the item in question
	while( 1 )
	{
		if (Launchable[iLoop][1] == usItem)
		{
			break;
		}
		iLoop++;
		if (Launchable[iLoop][0] != usLaunchable)
		{
			// the proposed item cannot be attached to the item in question
			return( FALSE );
		}
	}
	return( TRUE );
}


UINT16 GetLauncherFromLaunchable( UINT16 usLaunchable )
{
	INT32 iLoop = 0;

	// look for the section of the array pertaining to this launchable item...
	while( 1 )
	{
		if (Launchable[iLoop][0] == usLaunchable)
		{
			break;
		}
		iLoop++;
		if (Launchable[iLoop][0] == 0)
		{
			// the proposed item cannot be attached to anything!
			return( NOTHING );
		}
	}

	return( Launchable[iLoop][1] );
}


static BOOLEAN EvaluateValidMerge(UINT16 const usMerge, UINT16 const usItem, UINT16* const pusResult, UINT8* const pubType)
{
	// NB "usMerge" is the object being merged with (e.g. compound 18)
	// "usItem" is the item being merged "onto" (e.g. kevlar vest)

	if (usMerge == usItem && Item[usItem].usItemClass == IC_AMMO)
	{
		*pusResult = usItem;
		*pubType   = COMBINE_POINTS;
		return TRUE;
	}

	for (MergeInfo const* m = Merge; m->item1 != NOTHING; ++m)
	{
		if (m->item1 != usMerge) continue;
		if (m->item2 != usItem)  continue;

		*pusResult = m->result;
		*pubType   = m->action;
		return TRUE;
	}

	return FALSE;
}

BOOLEAN ValidMerge( UINT16 usMerge, UINT16 usItem )
{
	UINT16	usIgnoreResult;
	UINT8		ubIgnoreType;
	return( EvaluateValidMerge( usMerge, usItem, &usIgnoreResult, &ubIgnoreType ) );
}


UINT8 CalculateObjectWeight(OBJECTTYPE const* const o)
{
	INVTYPE const& item   = Item[o->usItem];
	UINT16         weight = item.ubWeight; // Start with base weight

	if (item.ubPerPocket <= 1)
	{
		// Account for any attachments
		for (UINT16 const* i = o->usAttachItem; i != endof(o->usAttachItem); ++i)
		{
			if (*i == NOTHING) continue;
			weight += Item[*i].ubWeight;
		}

		if (Item[o->usItem].usItemClass == IC_GUN && o->ubGunShotsLeft > 0)
		{ // Add in weight of ammo
			weight += Item[o->usGunAmmoItem].ubWeight;
		}
	}

	/* Make sure it really fits into that UINT8, in case we ever add anything real
	 * heavy with attachments/ammo */
	Assert(weight <= 255);
	return weight;
}


UINT32 CalculateCarriedWeight(SOLDIERTYPE const* const s)
{
	UINT32 total_weight = 0;
	CFOR_ALL_SOLDIER_INV_SLOTS(i, *s)
	{
		UINT16 weight = i->ubWeight;
		if (Item[i->usItem].ubPerPocket > 1)
		{ // Account for # of items
			weight *= i->ubNumberOfObjects;
		}
		total_weight += weight;
	}

	UINT8 strength_for_carrying = EffectiveStrength(s);
	if (strength_for_carrying > 80)
	{
		strength_for_carrying += strength_for_carrying - 80;
	}

	/* For now, assume soldiers can carry 1/2 their strength in kg without
	 * penalty. Instead of multiplying by 100 for percent, and then dividing by 10
	 * to account for weight units being in 10ths of kilos, not kilos... we just
	 * start with 10 instead of 100! */
	UINT32 const percent = 10 * total_weight / (strength_for_carrying / 2);
	return percent;
}


void DeleteObj(OBJECTTYPE * pObj )
{
	memset( pObj, 0, sizeof(OBJECTTYPE) );
}


void SwapObjs( OBJECTTYPE * pObj1, OBJECTTYPE * pObj2 )
{
	OBJECTTYPE Temp = *pObj1;
	*pObj1 = *pObj2;
	*pObj2 = Temp;
/*
	//if we are in the shop keeper interface, switch the items
	if( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE )
	{
		memset( &gMoveingItem, 0, sizeof( INVENTORY_IN_SLOT ) );
		gMoveingItem.fActive = TRUE;
		gMoveingItem.sItemIndex = pObj1->usItem;

		gMoveingItem.ubLocationOfObject = PLAYERS_INVENTORY;
		gMoveingItem.ubIdOfMercWhoOwnsTheItem = gpItemPointerSoldier->ubProfile;

		//Get the item from the slot.
		memcpy( &gMoveingItem.ItemObject, &pObj1, sizeof( OBJECTTYPE ) );

	}
*/
}

void RemoveObjFrom( OBJECTTYPE * pObj, UINT8 ubRemoveIndex )
{
	// remove 1 object from an OBJECTTYPE, starting at index bRemoveIndex
	UINT8 ubLoop;

	if (pObj->ubNumberOfObjects < ubRemoveIndex)
	{
		// invalid index!
		return;
	}
	else if (pObj->ubNumberOfObjects == 1)
	{
		// delete!
		DeleteObj( pObj );
	}
	else
	{
		// shift down all the values that should be down
		for (ubLoop = ubRemoveIndex + 1; ubLoop < pObj->ubNumberOfObjects; ubLoop++)
		{
			pObj->bStatus[ubLoop - 1] = pObj->bStatus[ubLoop];
		}
		// and set the upper value to 0
		pObj->bStatus[pObj->ubNumberOfObjects - 1] = 0;
		// make the number of objects recorded match the array
		pObj->ubNumberOfObjects--;
	}
}

void RemoveObjs( OBJECTTYPE * pObj, UINT8 ubNumberToRemove )
{
	// remove a certain number of objects from an OBJECTTYPE, starting at index 0
	UINT8 ubLoop;

	if (ubNumberToRemove == 0)
	{
		return;
	}
	if (ubNumberToRemove >= pObj->ubNumberOfObjects)
	{
		// delete!
		DeleteObj( pObj );
	}
	else
	{
		for (ubLoop = 0; ubLoop < ubNumberToRemove; ubLoop++)
		{
			RemoveObjFrom( pObj, 0 );
		}
		pObj->ubWeight = CalculateObjectWeight( pObj );
	}
}

void GetObjFrom( OBJECTTYPE * pObj, UINT8 ubGetIndex, OBJECTTYPE * pDest )
{
	if (!pDest || ubGetIndex >= pObj->ubNumberOfObjects)
	{
		return;
	}
	if (pObj->ubNumberOfObjects == 1)
	{
		*pDest = *pObj;
		DeleteObj( pObj );
	}
	else
	{
		pDest->usItem = pObj->usItem;
		pDest->bStatus[0] = pObj->bStatus[ubGetIndex];
		pDest->ubNumberOfObjects = 1;
		pDest->ubWeight = CalculateObjectWeight( pDest );
		RemoveObjFrom( pObj, ubGetIndex );
		pObj->ubWeight = CalculateObjectWeight( pObj );
	}
}


void DamageObj( OBJECTTYPE * pObj, INT8 bAmount )
{
	if (bAmount >= pObj->bStatus[0])
	{
		pObj->bStatus[0] = 1;
	}
	else
	{
		pObj->bStatus[0] -= bAmount;
	}
}


void StackObjs(OBJECTTYPE* pSourceObj, OBJECTTYPE* pTargetObj, UINT8 ubNumberToCopy)
{
	UINT8		ubLoop;

	// copy over N status values
	for (ubLoop = 0; ubLoop < ubNumberToCopy; ubLoop++)
	{
		pTargetObj->bStatus[ubLoop + pTargetObj->ubNumberOfObjects] = pSourceObj->bStatus[ubLoop ];
	}

	// now in the source object, move the rest down N places
	for (ubLoop = ubNumberToCopy; ubLoop < pSourceObj->ubNumberOfObjects; ubLoop++)
	{
		pSourceObj->bStatus[ubLoop - ubNumberToCopy] = pSourceObj->bStatus[ubLoop];
	}

	pTargetObj->ubNumberOfObjects += ubNumberToCopy;
	RemoveObjs( pSourceObj, ubNumberToCopy );
	pSourceObj->ubWeight = CalculateObjectWeight( pSourceObj );
	pTargetObj->ubWeight = CalculateObjectWeight( pTargetObj );
}


void CleanUpStack(OBJECTTYPE* const o, OBJECTTYPE* const cursor_o)
{
	INVTYPE const& item = Item[o->usItem];
	if (!(item.usItemClass & IC_AMMO) &&
			!(item.usItemClass & IC_KIT)  &&
			!(item.usItemClass & IC_MEDKIT))
	{
		return;
	}

	INT8 const max_points = item.usItemClass & IC_AMMO ?
		Magazine[item.ubClassIndex].ubMagSize : 100;

	if (cursor_o && cursor_o->usItem == o->usItem)
	{
		for (INT8 i = (INT8)cursor_o->ubNumberOfObjects - 1; i >= 0; --i)
		{
			INT8& src_status = cursor_o->bStatus[i];
			if (src_status <= 0) continue;

			// take the points here and distribute over the lower #d items
			for (INT8 k = o->ubNumberOfObjects - 1; k >= 0; --k)
			{
				INT8& dst_status = o->bStatus[k];
				if (dst_status >= max_points) continue;

				INT8 const points_to_move = MIN(max_points - dst_status, src_status);
				dst_status += points_to_move;
				src_status -= points_to_move;
				if (src_status != 0) continue;

				// done!
				--cursor_o->ubNumberOfObjects;
				break;
			}
		}
	}

	for (INT8 i = (INT8)o->ubNumberOfObjects - 1; i >= 0; --i)
	{
		INT8& src_status = o->bStatus[i];
		if (src_status <= 0) continue;

		// take the points here and distribute over the lower #d items
		for (INT8 k = i - 1; k >= 0; --k)
		{
			INT8& dst_status = o->bStatus[k];
			if (dst_status >= max_points) continue;

			INT8 const points_to_move = MIN(max_points - dst_status, src_status);
			dst_status += points_to_move;
			src_status -= points_to_move;
			if (src_status != 0) continue;

			// done!
			--o->ubNumberOfObjects;
			break;
		}
	}
}


BOOLEAN PlaceObjectAtObjectIndex( OBJECTTYPE * pSourceObj, OBJECTTYPE * pTargetObj, UINT8 ubIndex )
{
	INT8 bTemp;

	if (pSourceObj->usItem != pTargetObj->usItem)
	{
		return( TRUE );
	}
	if (ubIndex < pTargetObj->ubNumberOfObjects)
	{
		// swap
		bTemp = pSourceObj->bStatus[0];
		pSourceObj->bStatus[0] = pTargetObj->bStatus[ubIndex];
		pTargetObj->bStatus[ubIndex] = bTemp;
		return( TRUE );
	}
	else
	{
		// add to end
		StackObjs( pSourceObj, pTargetObj, 1 );
		return( FALSE );
	}
}

#define RELOAD_NONE 0
#define RELOAD_PLACE 1
#define RELOAD_SWAP 2
#define RELOAD_TOPOFF 3
#define RELOAD_AUTOPLACE_OLD 4

BOOLEAN ReloadGun( SOLDIERTYPE * pSoldier, OBJECTTYPE * pGun, OBJECTTYPE * pAmmo )
{
	OBJECTTYPE	OldAmmo;
	UINT8				ubBulletsToMove;
	BOOLEAN			fSameAmmoType;
	BOOLEAN			fSameMagazineSize;
	BOOLEAN			fReloadingWithStack;
	BOOLEAN			fEmptyGun;
	INT8				bReloadType;
	UINT16			usNewAmmoItem;

	INT8 bAPs = 0; // XXX HACK000E
	if ( (gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT) )
	{
		bAPs = GetAPsToReloadGunWithAmmo( pGun, pAmmo );
		if ( !EnoughPoints( pSoldier, bAPs, 0,TRUE ) )
		{
			return( FALSE );
		}

	}

	if ( Item[ pGun->usItem ].usItemClass == IC_LAUNCHER || pGun->usItem == TANK_CANNON )
	{
		if (!AttachObject(pSoldier, pGun, pAmmo))
		{
			// abort
			return( FALSE );
		}
	}
	else
	{
		fEmptyGun = (pGun->ubGunShotsLeft == 0);
		fReloadingWithStack = (pAmmo->ubNumberOfObjects > 1);
		fSameAmmoType = ( pGun->ubGunAmmoType == Magazine[Item[pAmmo->usItem].ubClassIndex].ubAmmoType );
		fSameMagazineSize = ( Magazine[ Item[ pAmmo->usItem ].ubClassIndex ].ubMagSize == Weapon[pGun->usItem].ubMagSize );

		if (fEmptyGun)
		{
			bReloadType = RELOAD_PLACE;
		}
		else
		{
			// record old ammo
			memset( &OldAmmo, 0, sizeof( OBJECTTYPE ));
			OldAmmo.usItem = pGun->usGunAmmoItem;
			OldAmmo.ubNumberOfObjects = 1;
			OldAmmo.ubShotsLeft[0] = pGun->ubGunShotsLeft;

			if (fSameMagazineSize)
			{
				if (fSameAmmoType)
				{
					if ( (gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT) )
					{
						bReloadType = RELOAD_SWAP;
					}
					else
					{
						bReloadType = RELOAD_TOPOFF;
					}
				}
				else
				{
					if (!fReloadingWithStack)
					{
						bReloadType = RELOAD_SWAP;
					}
					else
					{
						bReloadType = RELOAD_AUTOPLACE_OLD;
					}
				}
			}
			else  // diff sized magazines
			{
				if (fSameAmmoType)
				{
					bReloadType = RELOAD_TOPOFF;
				}
				else
				{
					bReloadType = RELOAD_AUTOPLACE_OLD;
				}
			}
		}

		if (fSameMagazineSize)
		{
			// record new ammo item for gun
			usNewAmmoItem = pAmmo->usItem;

			if (bReloadType == RELOAD_TOPOFF)
			{
				ubBulletsToMove = __min( pAmmo->ubShotsLeft[0], Weapon[pGun->usItem].ubMagSize - pGun->ubGunShotsLeft );
			}
			else
			{
				ubBulletsToMove = pAmmo->ubShotsLeft[0];
			}

		}
		else if (Magazine[Item[pAmmo->usItem].ubClassIndex].ubMagSize > Weapon[pGun->usItem].ubMagSize)
		{
			usNewAmmoItem = pAmmo->usItem - 1;
			if (bReloadType == RELOAD_TOPOFF)
			{
				ubBulletsToMove = __min( pAmmo->ubShotsLeft[0], Weapon[pGun->usItem].ubMagSize - pGun->ubGunShotsLeft );
			}
			else
			{
				ubBulletsToMove = __min( pAmmo->ubShotsLeft[0], Weapon[pGun->usItem].ubMagSize );
			}
		}
		else // mag is smaller than weapon mag
		{
			usNewAmmoItem = pAmmo->usItem + 1;
			if (bReloadType == RELOAD_TOPOFF)
			{
				ubBulletsToMove = __min( pAmmo->ubShotsLeft[0], Weapon[pGun->usItem].ubMagSize - pGun->ubGunShotsLeft );
			}
			else
			{
				ubBulletsToMove = __min( pAmmo->ubShotsLeft[0], Weapon[pGun->usItem].ubMagSize );
			}
		}


		switch( bReloadType )
		{

			case RELOAD_PLACE:
				pGun->ubGunShotsLeft = ubBulletsToMove;
				pGun->ubGunAmmoType = Magazine[Item[pAmmo->usItem].ubClassIndex].ubAmmoType;
				pGun->usGunAmmoItem = usNewAmmoItem;
				break;

			case RELOAD_SWAP:
				pGun->ubGunShotsLeft = ubBulletsToMove;
				pGun->ubGunAmmoType = Magazine[Item[pAmmo->usItem].ubClassIndex].ubAmmoType;
				pGun->usGunAmmoItem = usNewAmmoItem;
				if (fReloadingWithStack)
				{
					// add to end of stack
					StackObjs( &OldAmmo, pAmmo, 1 );
				}
				else
				{
					// Copying the old ammo to the cursor in turnbased could screw up for the player
					// (suppose his inventory is full!)

					if ( (gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT) && !EnoughPoints( pSoldier, (INT8) (bAPs + AP_PICKUP_ITEM), 0, FALSE ) )
					{
						// try autoplace
						if ( !AutoPlaceObject( pSoldier, &OldAmmo, FALSE ) )
						{
							// put it on the ground
							AddItemToPool(pSoldier->sGridNo, &OldAmmo, VISIBLE, pSoldier->bLevel, 0 , -1);
						}
						// delete the object now in the cursor
						DeleteObj( pAmmo );
					}
					else
					{
						// copy the old ammo to the cursor
						*pAmmo = OldAmmo;
					}
				}
				break;
			case RELOAD_AUTOPLACE_OLD:
				if ( !AutoPlaceObject( pSoldier, &OldAmmo, TRUE ) )
				{
					// error msg!
					return( FALSE );
				}
				// place first ammo in gun
				pGun->ubGunShotsLeft = ubBulletsToMove;
				pGun->ubGunAmmoType = Magazine[Item[pAmmo->usItem].ubClassIndex].ubAmmoType;
				pGun->usGunAmmoItem = usNewAmmoItem;

				break;

			case RELOAD_TOPOFF:
				// ADD that many bullets to gun
				pGun->ubGunShotsLeft += ubBulletsToMove;
				break;

		}

		if ( ! ( bReloadType == RELOAD_SWAP && !fReloadingWithStack ) )
		{
			// remove # of bullets, delete 1 object if necessary

			pAmmo->ubShotsLeft[0] -= ubBulletsToMove;
			if (pAmmo->ubShotsLeft[0] == 0)
			{
				RemoveObjs( pAmmo, 1 );
			}

		}
	}

	// OK, let's play a sound of reloading too...
	// If this guy is visible...
	if ( pSoldier->bVisible != -1 )
	{
		// Play some effects!
		SoundID const usReloadSound = Weapon[pGun->usItem].sReloadSound;
		if (usReloadSound != NO_SOUND && !IsAutoResolveActive())
		{
			PlayJA2Sample(usReloadSound, HIGHVOLUME, 1, MIDDLEPAN);
		}
	}

	if (pSoldier->bTeam == gbPlayerNum)
	{
		// spit out a message if this is one of our folks reloading
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_PLAYER_RELOADS], pSoldier->name );
	}

	DeductPoints( pSoldier, bAPs, 0 );
	pGun->ubWeight = CalculateObjectWeight( pGun );

	if ( pGun->bGunAmmoStatus >= 0 )
	{
		// make sure gun ammo status is 100, if gun isn't jammed
		pGun->bGunAmmoStatus = 100;
	}

	return( TRUE );
}

BOOLEAN EmptyWeaponMagazine( OBJECTTYPE * pWeapon, OBJECTTYPE *pAmmo )
{
	CHECKF( pAmmo != NULL );

	if ( pWeapon->ubGunShotsLeft > 0 )
	{
		// start by erasing ammo item, just in case...
		DeleteObj( pAmmo );

		pAmmo->ubShotsLeft[0]			= pWeapon->ubGunShotsLeft;
		pAmmo->usItem							= pWeapon->usGunAmmoItem;
		pAmmo->ubNumberOfObjects	= 1;

		pWeapon->ubGunShotsLeft		= 0;
		pWeapon->ubGunAmmoType	  = 0;
		//pWeapon->usGunAmmoItem		= 0; // leaving the ammo item the same for auto-reloading purposes

		// Play some effects!
		SoundID const usReloadSound = Weapon[pWeapon->usItem].sReloadSound;
		if (usReloadSound != NO_SOUND)
		{
			PlayJA2Sample(usReloadSound, HIGHVOLUME, 1, MIDDLEPAN);
		}

		pWeapon->ubWeight = CalculateObjectWeight( pWeapon );

		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}

/*
BOOLEAN ReloadLauncher( OBJECTTYPE * pLauncher, OBJECTTYPE * pAmmo )
{
	BOOLEAN			fOldAmmo;
	OBJECTTYPE	OldAmmo;

	if (pLauncher->ubGunShotsLeft == 0)
	{
		fOldAmmo = FALSE;
	}
	else
	{
		if (pAmmo->ubNumberOfObjects > 1)
		{
			// can't do the swap out to the cursor
			return( FALSE );
		}
		// otherwise temporarily store the launcher's old ammo
		memset( &OldAmmo, 0, sizeof( OBJECTTYPE ));
		fOldAmmo = TRUE;
		OldAmmo.usItem = pLauncher->usGunAmmoItem;
		OldAmmo.ubNumberOfObjects = 1;
		OldAmmo.bStatus[0] = pLauncher->bGunAmmoStatus;
	}

	// put the new ammo in the gun
	pLauncher->usGunAmmoItem = pAmmo->usItem;
	pLauncher->ubGunShotsLeft = 1;
	pLauncher->ubGunAmmoType = AMMO_GRENADE;
	pLauncher->bGunAmmoStatus = pAmmo->bStatus[0];


	if (fOldAmmo)
	{
		// copy the old ammo back to the item in the cursor
		*pAmmo = OldAmmo;
	}
	else
	{
		// reduce the number of objects in the cursor by 1
		RemoveObjs( pAmmo, 1 );
	}
	return( TRUE );
}
*/


INT8 FindAmmo(SOLDIERTYPE const* const s, AmmoKind const calibre, UINT8 const mag_size, INT8 const exclude_slot)
{
	for (INT8 slot = HANDPOS; slot != NUM_INV_SLOTS; ++slot)
	{
		if (slot == exclude_slot) continue;
		INVTYPE const& item = Item[s->inv[slot].usItem];
		if (item.usItemClass != IC_AMMO) continue;
		MAGTYPE const& m = Magazine[item.ubClassIndex];
		if (m.ubCalibre != calibre) continue;
		if (m.ubMagSize != mag_size && mag_size != ANY_MAGSIZE) continue;
		return slot;
	}
	return NO_SLOT;
}


INT8 FindAmmoToReload( SOLDIERTYPE * pSoldier, INT8 bWeaponIn, INT8 bExcludeSlot )
{
	OBJECTTYPE *	pObj;
	INT8					bSlot;

	if (pSoldier == NULL)
	{
		return( NO_SLOT );
	}
	pObj = &(pSoldier->inv[bWeaponIn]);
	if ( Item[pObj->usItem].usItemClass == IC_GUN && pObj->usItem != TANK_CANNON )
	{
		// look for same ammo as before
		bSlot = FindObjExcludingSlot( pSoldier, pObj->usGunAmmoItem, bExcludeSlot );
		if (bSlot != NO_SLOT)
		{
			// reload using this ammo!
			return( bSlot );
		}
		// look for any ammo that matches which is of the same calibre and magazine size
		bSlot = FindAmmo( pSoldier, Weapon[pObj->usItem].ubCalibre, Weapon[pObj->usItem].ubMagSize, bExcludeSlot );
		if (bSlot != NO_SLOT)
		{
			return( bSlot );
		}
		else
		{
			// look for any ammo that matches which is of the same calibre (different size okay)
			return( FindAmmo( pSoldier, Weapon[pObj->usItem].ubCalibre, ANY_MAGSIZE, bExcludeSlot ) );
		}
	}
	else
	{
		switch( pObj->usItem )
		{
			case MORTAR:
				return( FindObj( pSoldier, MORTAR_SHELL ) );
			case TANK_CANNON:
				return( FindObj( pSoldier, TANK_SHELL ) );
			case GLAUNCHER:
			case UNDER_GLAUNCHER:
				return( FindObjInObjRange( pSoldier, GL_HE_GRENADE, GL_SMOKE_GRENADE ) );
			default:
				return( NO_SLOT );
		}
	}
}

BOOLEAN AutoReload( SOLDIERTYPE * pSoldier )
{
	OBJECTTYPE *	pObj;
	INT8					bSlot, bAPCost;
	BOOLEAN				fRet;

	CHECKF( pSoldier );
	pObj = &(pSoldier->inv[HANDPOS]);

	if (Item[pObj->usItem].usItemClass == IC_GUN || Item[pObj->usItem].usItemClass == IC_LAUNCHER)
	{
		bSlot = FindAmmoToReload( pSoldier, HANDPOS, NO_SLOT );
		if (bSlot != NO_SLOT)
		{
			// reload using this ammo!
			fRet = ReloadGun( pSoldier, pObj, &(pSoldier->inv[bSlot]) );
			// if we are valid for two-pistol shooting (reloading) and we have enough APs still
			// then do a reload of both guns!
			if (fRet && IsValidSecondHandShotForReloadingPurposes(pSoldier))
			{
				pObj = &(pSoldier->inv[SECONDHANDPOS]);
				bSlot = FindAmmoToReload( pSoldier, SECONDHANDPOS, NO_SLOT );
				if (bSlot != NO_SLOT)
				{
					// ce would reload using this ammo!
					bAPCost = GetAPsToReloadGunWithAmmo( pObj, &(pSoldier->inv[bSlot] ) );
					if ( EnoughPoints( pSoldier, (INT16) bAPCost, 0, FALSE ) )
					{
						// reload the 2nd gun too
						fRet = ReloadGun( pSoldier, pObj, &(pSoldier->inv[bSlot]) );
					}
					else
					{
						ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[ STR_RELOAD_ONLY_ONE_GUN ], pSoldier->name );
					}
				}
			}

			DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
			return( fRet );
		}
	}

	// couldn't reload
	return( FALSE );
}


static ComboMergeInfoStruct const* GetAttachmentComboMerge(OBJECTTYPE const& o)
{
	for (ComboMergeInfoStruct const* i = AttachmentComboMerge;; ++i)
	{
		ComboMergeInfoStruct const& m = *i;
		if (m.usItem == NOTHING) break;
		if (m.usItem != o.usItem) continue;

		// Search for all the appropriate attachments
		for (UINT16 const* k = m.usAttachment; k != endof(m.usAttachment); ++k)
		{
			UINT16 const attachment = *k;
			if (attachment == NOTHING) continue;
			if (FindAttachment(&o, attachment) == -1) return 0; // Didn't find something required
		}

		return &m; // Found everything required
	}

	return 0;
}


static void PerformAttachmentComboMerge(OBJECTTYPE& o, ComboMergeInfoStruct const& m)
{
	/* This object has been validated as available for an attachment combo merge.
	 * - Find all attachments in list and destroy them
	 * - Status of new object should be average of items including attachments
	 * - Change object */
	UINT32 total_status          = o.bStatus[0];
	INT8   n_status_contributors = 1;
	for (UINT16 const* i = m.usAttachment; i != endof(m.usAttachment); ++i)
	{
		if (*i == NOTHING) continue;

		INT8 const attach_pos = FindAttachment(&o, *i);
		AssertMsg(attach_pos != -1, "Attachment combo merge couldn't find a necessary attachment");

		total_status += o.bAttachStatus[attach_pos];
		++n_status_contributors;

		o.usAttachItem[attach_pos]  = NOTHING;
		o.bAttachStatus[attach_pos] = 0;
	}

	o.usItem     = m.usResult;
	o.bStatus[0] = total_status / n_status_contributors;
}


bool AttachObject(SOLDIERTYPE* const s, OBJECTTYPE* const pTargetObj, OBJECTTYPE* const pAttachment)
{
	OBJECTTYPE& target     = *pTargetObj;
	OBJECTTYPE& attachment = *pAttachment;
	bool const valid_launchable = ValidLaunchable(attachment.usItem, target.usItem);
	if (valid_launchable || ValidItemAttachment(&target, attachment.usItem, TRUE))
	{
		// find an attachment position...
		// second half of this 'if' is for attaching GL grenades to a gun
		INT8 attach_pos;
		if (valid_launchable || (GL_HE_GRENADE <= attachment.usItem && attachment.usItem <= GL_SMOKE_GRENADE))
		{
			// try replacing if possible
			attach_pos = FindAttachmentByClass(&target, Item[attachment.usItem].usItemClass);
			if (attach_pos != NO_SLOT && attachment.ubNumberOfObjects > 1)
			{ // we can only do a swap if there is only 1 grenade being attached
				return false;
			}
		}
		else
		{
			// try replacing if possible
			attach_pos = FindAttachment(&target, attachment.usItem);
		}

		if (attach_pos == NO_SLOT)
		{
			attach_pos = FindAttachment(&target, NOTHING);
			if (attach_pos == NO_SLOT) return false;
		}

		AttachmentInfoStruct const* attach_info = 0;
		if (s)
		{
			attach_info = GetAttachmentInfo(attachment.usItem);
			// in-game (not behind the scenes) attachment
			if (attach_info && attach_info->bAttachmentSkillCheck != NO_CHECK)
			{
				INT32 const iCheckResult = SkillCheck(s, attach_info->bAttachmentSkillCheck, attach_info->bAttachmentSkillCheckMod);
				if (iCheckResult < 0)
				{
					// the attach failure damages both items
					DamageObj(&target,     (INT8)-iCheckResult);
					DamageObj(&attachment, (INT8)-iCheckResult);
					// there should be a quote here!
					DoMercBattleSound(s, BATTLE_SOUND_CURSE1);
					if (gfInItemDescBox) DeleteItemDescriptionBox();
					return false;
				}
			}

			if (ValidItemAttachment(&target, attachment.usItem, TRUE)) // not launchable
			{ // Attachment sounds
				INVTYPE const& tgt_item = Item[target.usItem];
				SoundID const  sound    =
					tgt_item.usItemClass & IC_WEAPON ? ATTACH_TO_GUN         :
					tgt_item.usItemClass & IC_ARMOUR ? ATTACH_CERAMIC_PLATES :
					tgt_item.usItemClass & IC_BOMB   ? ATTACH_DETONATOR      :
					NO_SOUND;
				if (sound != NO_SOUND) PlayLocationJA2Sample(s->sGridNo, sound, MIDVOLUME, 1);
			}
		}

		UINT16 const temp_item   = target.usAttachItem[attach_pos];
		UINT16 const temp_status = target.bAttachStatus[attach_pos];

		target.usAttachItem[attach_pos]  = attachment.usItem;
		target.bAttachStatus[attach_pos] = attachment.bStatus[0];

		// Transfer any attachment (max 1) from the grenade launcher to the gun
		if (attachment.usItem          == UNDER_GLAUNCHER &&
				attachment.usAttachItem[0] != NOTHING)
		{
			INT8 const second_attach_pos = FindAttachment(&target, NOTHING);
			if (second_attach_pos == ITEM_NOT_FOUND)
			{ // Not enough room for all attachments - cancel!
				target.usAttachItem[attach_pos]  = NOTHING;
				target.bAttachStatus[attach_pos] = 0;
				return false;
			}
			else
			{
				target.usAttachItem[second_attach_pos]  = attachment.usAttachItem[0];
				target.bAttachStatus[second_attach_pos] = attachment.bAttachStatus[0];
				attachment.usAttachItem[0]  = NOTHING;
				attachment.bAttachStatus[0] = 0;
			}
		}

		if (temp_item != NOTHING)
		{ // overwrite/swap!
			CreateItem(temp_item, temp_status, &attachment);
		}
		else
		{
			RemoveObjs(&attachment, 1);
		}

		// Check for attachment merge combos here
		if (ComboMergeInfoStruct const* const m = GetAttachmentComboMerge(target))
		{
			PerformAttachmentComboMerge(target, *m);
			if (attach_info && attach_info->bAttachmentSkillCheckMod < 20)
			{
				StatChange(*s, MECHANAMT, 20 - attach_info->bAttachmentSkillCheckMod, FROM_SUCCESS);
			}
		}

		target.ubWeight = CalculateObjectWeight(&target);
		return true;
	}

	// check for merges
	UINT16 merge_result;
	UINT8  merge_kind;
	if (!EvaluateValidMerge(attachment.usItem, target.usItem, &merge_result, &merge_kind)) return false;

	if (merge_kind != COMBINE_POINTS)
	{
		if (!EnoughPoints(s, AP_MERGE, 0, TRUE)) return false;
		DeductPoints(s, AP_MERGE, 0);
	}

	switch (merge_kind)
	{
		case COMBINE_POINTS:
		{
			// transfer points...
			INVTYPE const& tgt_item = Item[target.usItem];
			UINT8   const  limit    =
				tgt_item.usItemClass == IC_AMMO ? Magazine[tgt_item.ubClassIndex].ubMagSize :
				100;

			// count down through # of attaching items and add to status of item in position 0
			for (INT8 bLoop = attachment.ubNumberOfObjects - 1; bLoop >= 0; --bLoop)
			{
				if (target.bStatus[0] + attachment.bStatus[bLoop] <= limit)
				{ // Consume this one totally and continue
					target.bStatus[0] += attachment.bStatus[bLoop];
					RemoveObjFrom(&attachment, bLoop);
					// reset loop limit
					bLoop = attachment.ubNumberOfObjects; // add 1 to counteract the -1 from the loop
				}
				else
				{ // Add part of this one and then we're done
					attachment.bStatus[bLoop] -= limit - target.bStatus[0];
					target.bStatus[0]          = limit;
					break;
				}
			}
			break;
		}

		case DESTRUCTION:
			// The merge destroyed both items!
			DeleteObj(&target);
			DeleteObj(&attachment);
			DoMercBattleSound(s, BATTLE_SOUND_CURSE1);
			break;

		case ELECTRONIC_MERGE:
			if (s)
			{
				INT32 const iCheckResult = SkillCheck(s, ATTACHING_SPECIAL_ELECTRONIC_ITEM_CHECK, -30);
				if (iCheckResult < 0)
				{
					DamageObj(&target,     (INT8)-iCheckResult);
					DamageObj(&attachment, (INT8)-iCheckResult);
					DoMercBattleSound(s, BATTLE_SOUND_CURSE1);
					return false;
				}
				// grant experience!
			}
			goto default_merge;

		case EXPLOSIVE:
			if (s)
			{
				// requires a skill check, and gives experience
				INT32 const iCheckResult = SkillCheck(s, ATTACHING_DETONATOR_CHECK, -30);
				if (iCheckResult < 0)
				{
					// could have a chance of detonation
					// for now, damage both objects
					DamageObj(&target,     (INT8)-iCheckResult);
					DamageObj(&attachment, (INT8)-iCheckResult);
					DoMercBattleSound(s, BATTLE_SOUND_CURSE1);
					return false;
				}
				StatChange(*s, EXPLODEAMT, 25, FROM_SUCCESS);
			}
			goto default_merge;

		default:
default_merge:
			// the merge will combine the two items
			target.usItem = merge_result;
			if (merge_kind != TREAT_ARMOUR)
			{
				target.bStatus[0] = (target.bStatus[0] + attachment.bStatus[0]) / 2;
			}
			DeleteObj(&attachment);
			target.ubWeight = CalculateObjectWeight(&target);
			if (s && s->bTeam == gbPlayerNum)
			{
				DoMercBattleSound(s, BATTLE_SOUND_COOL1);
			}
			break;
	}
	return true;
}


BOOLEAN CanItemFitInPosition(SOLDIERTYPE* pSoldier, OBJECTTYPE* pObj, INT8 bPos, BOOLEAN fDoingPlacement)
{
	UINT8					ubSlotLimit;
	INT8					bNewPos;

	switch( bPos )
	{
		case SECONDHANDPOS:
			if (Item[pSoldier->inv[HANDPOS].usItem].fFlags & ITEM_TWO_HANDED)
			{
				return( FALSE );
			}
			break;
		case HANDPOS:
			if (Item[ pObj->usItem ].fFlags & ITEM_TWO_HANDED)
			{
				if (pSoldier->inv[HANDPOS].usItem != NOTHING && pSoldier->inv[SECONDHANDPOS].usItem != NOTHING)
				{
					// two items in hands; try moving the second one so we can swap
					if (Item[pSoldier->inv[SECONDHANDPOS].usItem].ubPerPocket == 0)
					{
						bNewPos = FindEmptySlotWithin( pSoldier, BIGPOCK1POS, BIGPOCK4POS );
					}
					else
					{
						bNewPos = FindEmptySlotWithin( pSoldier, BIGPOCK1POS, SMALLPOCK8POS );
					}
					if (bNewPos == NO_SLOT)
					{
						// nowhere to put second item
						return( FALSE );
					}

					if ( fDoingPlacement )
					{
						// otherwise move it.
						pSoldier->inv[bNewPos] = pSoldier->inv[SECONDHANDPOS];
						DeleteObj( &(pSoldier->inv[SECONDHANDPOS]) );
					}
				}
			}
			break;
		case VESTPOS:
		case HELMETPOS:
		case LEGPOS:
			if (Item[pObj->usItem].usItemClass != IC_ARMOUR)
			{
				return( FALSE );
			}
			switch (bPos)
			{
				case VESTPOS:
					if (Armour[Item[pObj->usItem].ubClassIndex].ubArmourClass != ARMOURCLASS_VEST)
					{
						return( FALSE );
					}
					break;
				case HELMETPOS:
					if (Armour[Item[pObj->usItem].ubClassIndex].ubArmourClass != ARMOURCLASS_HELMET)
					{
						return( FALSE );
					}
					break;
				case LEGPOS:
					if (Armour[Item[pObj->usItem].ubClassIndex].ubArmourClass != ARMOURCLASS_LEGGINGS)
					{
						return( FALSE );
					}
					break;
				default:
					break;
			}
			break;
		case HEAD1POS:
		case HEAD2POS:
			if (Item[pObj->usItem].usItemClass != IC_FACE)
			{
				return( FALSE );
			}
		default:
			break;
	}

	ubSlotLimit = ItemSlotLimit( pObj->usItem, bPos );
	if (ubSlotLimit == 0 && bPos >= SMALLPOCK1POS )
	{
		// doesn't fit!
		return( FALSE );
	}

	return( TRUE );
}


static BOOLEAN DropObjIfThereIsRoom(SOLDIERTYPE* pSoldier, INT8 bPos, OBJECTTYPE* pObj)
{
	// try autoplacing item in bSlot elsewhere, then do a placement
	BOOLEAN fAutoPlacedOld;

	fAutoPlacedOld = AutoPlaceObject( pSoldier, &(pSoldier->inv[bPos]), FALSE );
	if ( fAutoPlacedOld )
	{
		return( PlaceObject( pSoldier, bPos, pObj ) );
	}
	else
	{
		return( FALSE );
	}
}


BOOLEAN PlaceObject( SOLDIERTYPE * pSoldier, INT8 bPos, OBJECTTYPE * pObj )
{
	// returns object to have in hand after placement... same as original in the
	// case of error

	UINT8					ubSlotLimit, ubNumberToDrop, ubLoop;
	OBJECTTYPE *	pInSlot;
	BOOLEAN				fObjectWasRobotRemote = FALSE;

	if ( pObj->usItem == ROBOT_REMOTE_CONTROL )
	{
		fObjectWasRobotRemote = TRUE;
	}

	if ( !CanItemFitInPosition( pSoldier, pObj, bPos, TRUE ) )
	{
		return( FALSE );
	}

	// If the position is either head slot, then the item must be IC_FACE (checked in
	// CanItemFitInPosition).
	if ( bPos == HEAD1POS )
	{
		if ( !CompatibleFaceItem( pObj->usItem, pSoldier->inv[ HEAD2POS ].usItem ) )
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, Message[STR_CANT_USE_TWO_ITEMS], ItemNames[pObj->usItem], ItemNames[pSoldier->inv[HEAD2POS].usItem]);
			return( FALSE );
		}
	}
	else if ( bPos == HEAD2POS )
	{
		if ( !CompatibleFaceItem( pObj->usItem, pSoldier->inv[ HEAD1POS ].usItem ) )
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, Message[STR_CANT_USE_TWO_ITEMS], ItemNames[pObj->usItem], ItemNames[pSoldier->inv[HEAD1POS].usItem]);
			return( FALSE );
		}
	}

	if ( Item[ pObj->usItem ].usItemClass == IC_KEY && pSoldier->uiStatusFlags & SOLDIER_PC )
	{
		if ( KeyTable[ pObj->ubKeyID ].usDateFound == 0 )
		{
			KeyTable[ pObj->ubKeyID ].usDateFound = (UINT16) GetWorldDay();
			KeyTable[ pObj->ubKeyID ].usSectorFound = SECTOR( pSoldier->sSectorX, pSoldier->sSectorY );
		}
	}

	ubSlotLimit = ItemSlotLimit( pObj->usItem, bPos );

	pInSlot = &(pSoldier->inv[bPos]);

	if (pInSlot->ubNumberOfObjects == 0)
	{
		// placement in an empty slot
		ubNumberToDrop = pObj->ubNumberOfObjects;

		if (ubNumberToDrop > __max( ubSlotLimit, 1 ) )
		{
			// drop as many as possible into pocket
			ubNumberToDrop = __max( ubSlotLimit, 1 );
		}

		// could be wrong type of object for slot... need to check...
		// but assuming it isn't
		*pInSlot = *pObj;
/*
		//if we are in the shopkeeper interface
		if( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE )
		{
			memset( &gMoveingItem, 0, sizeof( INVENTORY_IN_SLOT ) );
			SetSkiCursor( CURSOR_NORMAL );
		}
*/

		if (ubNumberToDrop != pObj->ubNumberOfObjects)
		{
			// in the InSlot copy, zero out all the objects we didn't drop
			for (ubLoop = ubNumberToDrop; ubLoop < pObj->ubNumberOfObjects; ubLoop++)
			{
				pInSlot->bStatus[ubLoop] = 0;
			}
		}
		pInSlot->ubNumberOfObjects = ubNumberToDrop;

		// remove a like number of objects from pObj
		RemoveObjs( pObj, ubNumberToDrop );
		if (pObj->ubNumberOfObjects == 0)
		{
			// dropped everything
			if (bPos == HANDPOS && Item[pInSlot->usItem].fFlags & ITEM_TWO_HANDED)
			{
				// We just performed a successful drop of a two-handed object into the
				// main hand
				if (pSoldier->inv[SECONDHANDPOS].usItem != 0)
				{
					// swap what WAS in the second hand into the cursor
					SwapObjs( pObj, &(pSoldier->inv[SECONDHANDPOS]));
				}
			}
		}
	}
	else
	{
		// replacement/reloading/merging/stacking
		// keys have an additional check for key ID being the same
		if ( (pObj->usItem == pInSlot->usItem) && ( Item[ pObj->usItem ].usItemClass != IC_KEY || pObj->ubKeyID == pInSlot->ubKeyID ) )
		{
			if (Item[ pObj->usItem ].usItemClass == IC_MONEY)
			{

				UINT32 uiMoneyMax = MoneySlotLimit( bPos );

				// always allow money to be combined!
				// IGNORE STATUS!

				if (pInSlot->uiMoneyAmount + pObj->uiMoneyAmount > uiMoneyMax)
				{
					// remove X dollars
					pObj->uiMoneyAmount -= (uiMoneyMax - pInSlot->uiMoneyAmount);
					// set in slot to maximum
					pInSlot->uiMoneyAmount = uiMoneyMax;
				}
				else
				{
					pInSlot->uiMoneyAmount += pObj->uiMoneyAmount;
					DeleteObj( pObj );
/*
					if( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE )
					{
						memset( &gMoveingItem, 0, sizeof( INVENTORY_IN_SLOT ) );
						SetSkiCursor( CURSOR_NORMAL );
					}
*/
				}
			}
			else if ( ubSlotLimit == 1 || (ubSlotLimit == 0 && bPos >= HANDPOS && bPos <= BIGPOCK4POS ) )
			{
				if (pObj->ubNumberOfObjects <= 1)
				{
					// swapping
					SwapObjs( pObj, pInSlot );
				}
				else
				{
					return( DropObjIfThereIsRoom( pSoldier, bPos, pObj ) );
				}
			}
			else if (ubSlotLimit == 0) // trying to drop into a small pocket
			{
				return( DropObjIfThereIsRoom( pSoldier, bPos, pObj ) );
			}
			else
			{
				// stacking
				ubNumberToDrop = ubSlotLimit - pInSlot->ubNumberOfObjects;
				if (ubNumberToDrop > pObj->ubNumberOfObjects)
				{
					ubNumberToDrop = pObj->ubNumberOfObjects;
				}
				StackObjs( pObj, pInSlot, ubNumberToDrop );
			}
		}
		else
		{
			// replacement, unless reloading...
			switch (Item[pInSlot->usItem].usItemClass)
			{
				case IC_GUN:
					if (Item[pObj->usItem].usItemClass == IC_AMMO)
					{
						if (Weapon[pInSlot->usItem].ubCalibre == Magazine[Item[pObj->usItem].ubClassIndex].ubCalibre)
						{
							// reload...
							return( ReloadGun( pSoldier, pInSlot, pObj ) );
						}
						else
						{
							// invalid ammo
							break;
							//return( FALSE );
						}
					}
					break;
				case IC_LAUNCHER:
				{
					if ( ValidLaunchable( pObj->usItem, pInSlot->usItem ) )
					{
						// reload...
						return( ReloadGun( pSoldier, pInSlot, pObj ) );
					}
				}
				break;
			}

			if ( (Item[pObj->usItem].fFlags & ITEM_TWO_HANDED) && (bPos == HANDPOS) )
			{
				if (pSoldier->inv[SECONDHANDPOS].usItem != 0)
				{
					// both pockets have something in them, so we can't swap
					return( FALSE );
				}
				else
				{
					SwapObjs( pObj, pInSlot );
				}
			}
			else if (pObj->ubNumberOfObjects <= __max( ubSlotLimit, 1 ) )
			{
				// swapping
				SwapObjs( pObj, pInSlot );
			}
			else
			{
				return( DropObjIfThereIsRoom( pSoldier, bPos, pObj ) );
			}

		}
	}

	// ATE: Put this in to see if we should update the robot, if we were given a controller...
	if ( pSoldier->bTeam == gbPlayerNum && fObjectWasRobotRemote )
	{
		UpdateRobotControllerGivenController( pSoldier );
	}

	return( TRUE );
}


static BOOLEAN InternalAutoPlaceObject(SOLDIERTYPE* pSoldier, OBJECTTYPE* pObj, BOOLEAN fNewItem, INT8 bExcludeSlot)
{
	INT8			bSlot;
	UINT8			ubPerSlot;

	// statuses of extra objects would be 0 if the # exceeds the maximum
	Assert( pObj->ubNumberOfObjects <= MAX_OBJECTS_PER_SLOT);

	const INVTYPE* const pItem = &Item[pObj->usItem];
	ubPerSlot = pItem->ubPerPocket;

	// Overrides to the standard system: put guns in hand, armour on body (if slot empty)
	switch (pItem->usItemClass)
	{
		case IC_GUN:
		case IC_BLADE:
		case IC_LAUNCHER:
		case IC_BOMB:
		case IC_GRENADE:
			if (!(pItem->fFlags & ITEM_TWO_HANDED))
			{
				if (pSoldier->inv[HANDPOS].usItem == NONE)
				{
					// put the one-handed weapon in the guy's hand...
					PlaceObject( pSoldier, HANDPOS, pObj );
					SetNewItem( pSoldier, HANDPOS, fNewItem );
					if ( pObj->ubNumberOfObjects == 0 )
					{
						return( TRUE );
					}
				}
				else if ( !(Item[pSoldier->inv[HANDPOS].usItem].fFlags & ITEM_TWO_HANDED) && pSoldier->inv[SECONDHANDPOS].usItem == NONE)
				{
					// put the one-handed weapon in the guy's 2nd hand...
					PlaceObject( pSoldier, SECONDHANDPOS, pObj );
					SetNewItem( pSoldier, SECONDHANDPOS, fNewItem );
					if ( pObj->ubNumberOfObjects == 0 )
					{
						return( TRUE );
					}
				}
			}
			// two-handed objects are best handled in the main loop for large objects,
			// which checks the hands first anyhow
			break;

		case IC_ARMOUR:
			switch (Armour[Item[pObj->usItem].ubClassIndex].ubArmourClass)
			{
				case ARMOURCLASS_VEST:
					if (pSoldier->inv[VESTPOS].usItem == NONE)
					{
						// put on the armour!
						PlaceObject( pSoldier, VESTPOS, pObj );
						SetNewItem( pSoldier, VESTPOS, fNewItem );
						if ( pObj->ubNumberOfObjects == 0 )
						{
							return( TRUE );
						}
					}
					break;
				case ARMOURCLASS_LEGGINGS:
					if (pSoldier->inv[LEGPOS].usItem == NONE)
					{
						// put on the armour!
						PlaceObject( pSoldier, LEGPOS, pObj );
						SetNewItem( pSoldier, LEGPOS, fNewItem );
						if ( pObj->ubNumberOfObjects == 0 )
						{
							return( TRUE );
						}
					}
					break;
				case ARMOURCLASS_HELMET:
					if (pSoldier->inv[HELMETPOS].usItem == NONE)
					{
						// put on the armour!
						PlaceObject( pSoldier, HELMETPOS, pObj );
						SetNewItem( pSoldier, HELMETPOS, fNewItem );
						if ( pObj->ubNumberOfObjects == 0 )
						{
							return( TRUE );
						}
					}
					break;
				default:
					break;
			}
			// otherwise stuff it in a slot somewhere
			break;
		case IC_FACE:
			if ( (pSoldier->inv[HEAD1POS].usItem == NOTHING) && CompatibleFaceItem( pObj->usItem, pSoldier->inv[HEAD2POS].usItem ) )
			{
				PlaceObject( pSoldier, HEAD1POS, pObj );
				SetNewItem( pSoldier, HEAD1POS, fNewItem );
				if ( pObj->ubNumberOfObjects == 0 )
				{
					return( TRUE );
				}
			}
			else if ( (pSoldier->inv[HEAD2POS].usItem == NOTHING) && CompatibleFaceItem( pObj->usItem, pSoldier->inv[HEAD1POS].usItem ) )
			{
				PlaceObject( pSoldier, HEAD2POS, pObj );
				SetNewItem( pSoldier, HEAD2POS, fNewItem );
				if ( pObj->ubNumberOfObjects == 0 )
				{
					return( TRUE );
				}
			}
			break;
		default:
			break;
	}

	if (ubPerSlot == 0)
	{
		// Large object; look for an empty hand/large pocket and dump it in there
		// FindObjWithin with 0 will search for empty slots!
		bSlot = HANDPOS;
		while (1)
		{
			bSlot = FindEmptySlotWithin( pSoldier, bSlot, BIGPOCK4POS );
			if (bSlot == ITEM_NOT_FOUND)
			{
				return( FALSE );
			}
			if (bSlot == SECONDHANDPOS)
			{
				if (pSoldier->inv[HANDPOS].usItem != NONE)
				{
					bSlot++;
					continue;
				}
			}
			// this might fail if we're trying to place in HANDPOS,
			// and SECONDHANDPOS is full
			PlaceObject( pSoldier, bSlot, pObj );
			SetNewItem( pSoldier, bSlot, fNewItem );
			if (pObj->ubNumberOfObjects == 0)
			{
				return( TRUE );
			}
			bSlot++;
		}
	}
	else
	{
		// Small items; don't allow stack/dumping for keys right now as that
		// would require a bunch of functions for finding the same object by two values...
		if ( ubPerSlot > 1 || Item[ pObj->usItem ].usItemClass == IC_KEY || Item[ pObj->usItem ].usItemClass == IC_MONEY )
		{
			// First, look for slots with the same object, and dump into them.
			bSlot = HANDPOS;
			while( 1 )
			{
				bSlot = FindObjWithin( pSoldier, pObj->usItem, bSlot, SMALLPOCK8POS );
				if (bSlot == ITEM_NOT_FOUND)
				{
					break;
				}
				if ( bSlot != bExcludeSlot )
				{
					if ( ( (Item[ pObj->usItem ].usItemClass == IC_MONEY) && pSoldier->inv[ bSlot ].uiMoneyAmount < MoneySlotLimit( bSlot ) ) || (Item[ pObj->usItem ].usItemClass != IC_MONEY && pSoldier->inv[bSlot].ubNumberOfObjects < ItemSlotLimit( pObj->usItem, bSlot ) ) )
					{
						// NEW: If in SKI, don't auto-place anything into a stackable slot that's currently hatched out!  Such slots
						// will disappear in their entirety if sold/moved, causing anything added through here to vanish also!
						if( !( ( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE ) && ShouldSoldierDisplayHatchOnItem( pSoldier->ubProfile, bSlot ) ) )
						{
							PlaceObject( pSoldier, bSlot, pObj );
							SetNewItem( pSoldier, bSlot, fNewItem );
							if (pObj->ubNumberOfObjects == 0)
							{
								return( TRUE );
							}
						}
					}
				}
				bSlot++;
			}
		}
		// Search for empty slots to dump into, starting with small pockets
		bSlot = SMALLPOCK1POS;
		while( 1 )
		{
			bSlot = FindEmptySlotWithin( pSoldier, bSlot, SMALLPOCK8POS );
			if (bSlot == ITEM_NOT_FOUND)
			{
				break;
			}
			PlaceObject( pSoldier, bSlot, pObj );
			SetNewItem( pSoldier, bSlot, fNewItem );
			if (pObj->ubNumberOfObjects == 0)
			{
				return( TRUE );
			}
			bSlot++;
		}
		// now check hands/large pockets
		bSlot = HANDPOS;
		while (1)
		{
			bSlot = FindEmptySlotWithin( pSoldier, bSlot, BIGPOCK4POS );
			if (bSlot == ITEM_NOT_FOUND)
			{
				break;
			}
			PlaceObject( pSoldier, bSlot, pObj );
			SetNewItem( pSoldier, bSlot, fNewItem );
			if (pObj->ubNumberOfObjects == 0)
			{
				return( TRUE );
			}
			bSlot++;
		}
	}
	return( FALSE );
}

BOOLEAN AutoPlaceObject( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj, BOOLEAN fNewItem )
{
	return( InternalAutoPlaceObject( pSoldier, pObj, fNewItem, NO_SLOT ) );
}

BOOLEAN RemoveObjectFromSlot( SOLDIERTYPE * pSoldier, INT8 bPos, OBJECTTYPE * pObj )
{
	CHECKF( pObj );
	if (pSoldier->inv[bPos].ubNumberOfObjects == 0)
	{
		return( FALSE );
	}
	else
	{
		*pObj = pSoldier->inv[bPos];
		DeleteObj( &(pSoldier->inv[bPos]) );
		return( TRUE );
	}
}

BOOLEAN RemoveKeyFromSlot( SOLDIERTYPE * pSoldier, INT8 bKeyRingPosition, OBJECTTYPE * pObj )
{
	UINT8 ubItem = 0;

	CHECKF( pObj );

	if( ( pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber == 0 ) || ( pSoldier->pKeyRing[ bKeyRingPosition ].ubKeyID == INVALID_KEY_NUMBER ) )
	{
		return( FALSE );
	}
	else
	{
		//*pObj = pSoldier->inv[bPos];

		// create an object
		ubItem = pSoldier->pKeyRing[ bKeyRingPosition ].ubKeyID;

		if( pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber > 1 )
		{
			pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber--;
		}
		else
		{

			pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber = 0;
			pSoldier->pKeyRing[ bKeyRingPosition ].ubKeyID = INVALID_KEY_NUMBER;
		}

		CreateKeyObject(pObj, 1, ubItem);
		return TRUE;
	}
}


BOOLEAN RemoveKeysFromSlot( SOLDIERTYPE * pSoldier, INT8 bKeyRingPosition, UINT8 ubNumberOfKeys ,OBJECTTYPE * pObj )
{
	UINT8 ubItems = 0;

	CHECKF( pObj );


	if( ( pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber == 0 ) || ( pSoldier->pKeyRing[ bKeyRingPosition ].ubKeyID == INVALID_KEY_NUMBER ) )
	{
		return( FALSE );
	}
	else
	{
		//*pObj = pSoldier->inv[bPos];

		if( pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber < ubNumberOfKeys )
		{
			ubNumberOfKeys = pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber;
		}


		ubItems = pSoldier->pKeyRing[ bKeyRingPosition ].ubKeyID;
		if( pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber - ubNumberOfKeys > 0 )
		{
			pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber--;
		}
		else
		{
			pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber = 0;
			pSoldier->pKeyRing[ bKeyRingPosition ].ubKeyID = INVALID_KEY_NUMBER;
		}

		CreateKeyObject(pObj, ubNumberOfKeys, ubItems);
		return TRUE;
	}
}

// return number added
UINT8 AddKeysToSlot( SOLDIERTYPE * pSoldier, INT8 bKeyRingPosition, OBJECTTYPE * pObj )
{
	UINT8 ubNumberNotAdded = 0;

	if ( pSoldier->uiStatusFlags & SOLDIER_PC ) // redundant but what the hey
	{
		if ( KeyTable[ pObj->ubKeyID ].usDateFound == 0 )
		{
			KeyTable[ pObj->ubKeyID ].usDateFound = (UINT16) GetWorldDay();
			KeyTable[ pObj->ubKeyID ].usSectorFound = SECTOR( pSoldier->sSectorX, pSoldier->sSectorY );
		}
	}

	// check if we are going to far
	if ( ( pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber + pObj->ubNumberOfObjects ) > Item[ pObj->usItem ].ubPerPocket )
	{
		// only take what we can
		ubNumberNotAdded = pObj->ubNumberOfObjects - ( Item[ pObj->usItem ].ubPerPocket - pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber );

		// set to max
		pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber = Item[ pObj->usItem ].ubPerPocket;

		if( pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber == 0 )
		{
			pSoldier->pKeyRing[ bKeyRingPosition ].ubKeyID = pObj->ubKeyID;
		}

		// return number used
		return( pObj->ubNumberOfObjects - ubNumberNotAdded );
	}
	else
	{
		// check
		if( pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber == 0 )
		{
			pSoldier->pKeyRing[ bKeyRingPosition ].ubKeyID = pObj->ubKeyID;
		}

		pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber += pObj->ubNumberOfObjects;
	}

	return( pObj->ubNumberOfObjects );
}

UINT8 SwapKeysToSlot( SOLDIERTYPE * pSoldier, INT8 bKeyRingPosition, OBJECTTYPE * pObj )
{
	// swap keys in keyring slot and keys in pocket
	OBJECTTYPE	TempObj;

	// create temp object to hold keys currently in key ring slot
	CreateKeyObject( &TempObj, pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber, pSoldier->pKeyRing[ bKeyRingPosition ].ubKeyID );

	pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber = pObj->ubNumberOfObjects;
	pSoldier->pKeyRing[ bKeyRingPosition ].ubKeyID = pObj->ubKeyID;

	// swap params?
	*pObj = TempObj;

	return( 1 );
}


void CreateKeyObject(OBJECTTYPE* const pObj, UINT8 const ubNumberOfKeys, UINT8 const ubKeyID)
{
	CreateItems(FIRST_KEY + LockTable[ubKeyID].usKeyItem, 100, ubNumberOfKeys, pObj);
	pObj->ubKeyID = ubKeyID;
}


void AllocateObject(OBJECTTYPE** const pObj)
{
	// create a key object
	*pObj = MALLOC(OBJECTTYPE);
}


BOOLEAN DeleteKeyObject( OBJECTTYPE * pObj )
{
	if (pObj == NULL) return FALSE;

	// free up space
	MemFree( pObj );

	return( TRUE );
}


UINT16 TotalPoints(const OBJECTTYPE* const pObj)
{
	UINT16	usPoints = 0;
	UINT8		ubLoop;

	for (ubLoop = 0; ubLoop < pObj->ubNumberOfObjects; ubLoop++)
	{
		usPoints += pObj->bStatus[ubLoop];
	}
	return( usPoints );
}

UINT16 UseKitPoints( OBJECTTYPE * pObj, UINT16 usPoints, SOLDIERTYPE *pSoldier )
{
	// start consuming from the last kit in, so we end up with fewer fuller kits rather than
	// lots of half-empty ones.
	INT8		bLoop;
	UINT16 usOriginalPoints = usPoints;

	for (bLoop = pObj->ubNumberOfObjects - 1; bLoop >= 0; bLoop--)
	{
		if (usPoints < (UINT16) pObj->bStatus[bLoop])
		{
			pObj->bStatus[bLoop] -= (INT8) usPoints;
			return( usOriginalPoints );
		}
		else
		{
			// consume this kit totally
			usPoints -= pObj->bStatus[bLoop];
			pObj->bStatus[bLoop] = 0;

			pObj->ubNumberOfObjects--;
		}
	}

	// check if pocket/hand emptied..update inventory, then update panel
	if( pObj->ubNumberOfObjects == 0 )
	{
		// Delete object
		DeleteObj( pObj );

		// dirty interface panel
		DirtyMercPanelInterface(  pSoldier, DIRTYLEVEL2 );
	}

	return( usOriginalPoints -  usPoints );
}


static UINT16 MagazineClassIndexToItemType(UINT16 usMagIndex)
{
	UINT16				usLoop;

	// Note: if any ammo items in the item table are separated from the main group,
	// this function will have to be rewritten to scan the item table for an item
	// with item class ammo, which has class index usMagIndex
	for (usLoop = FIRST_AMMO; usLoop < MAXITEMS; usLoop++)
	{
		if (Item[usLoop].ubClassIndex == usMagIndex)
		{
			return( usLoop );
		}
	}

	return(NONE);
}


UINT16 DefaultMagazine( UINT16 usItem )
{
	UINT16				usLoop;

	if (!(Item[usItem].usItemClass & IC_GUN))
	{
		throw std::logic_error("Tried to get default ammo for item which is not a gun");
	}

	WEAPONTYPE const* const pWeapon = &Weapon[usItem];
	usLoop = 0;
	while ( Magazine[usLoop].ubCalibre != NOAMMO )
	{
		if (Magazine[usLoop].ubCalibre == pWeapon->ubCalibre &&
				Magazine[usLoop].ubMagSize == pWeapon->ubMagSize)
		{
			return(MagazineClassIndexToItemType(usLoop));
		}

		usLoop++;
	}

	throw std::logic_error("Found no default ammo for gun");
}


static UINT16 FindReplacementMagazine(AmmoKind const ubCalibre, UINT8 const ubMagSize, UINT8 const ubAmmoType)
{
	UINT8 ubLoop;
	UINT16 usDefault;

	ubLoop = 0;
	usDefault = NOTHING;

	while ( Magazine[ubLoop].ubCalibre != NOAMMO )
	{
		if (Magazine[ubLoop].ubCalibre == ubCalibre &&
				Magazine[ubLoop].ubMagSize == ubMagSize )
		{
			if ( Magazine[ubLoop].ubAmmoType == ubAmmoType )
			{
				return( MagazineClassIndexToItemType( ubLoop ) );
			}
			else if ( usDefault == NOTHING )
			{
				// store this one to use if all else fails
				usDefault = MagazineClassIndexToItemType( ubLoop );
			}

		}

		ubLoop++;
	}

	return( usDefault );

}

UINT16 FindReplacementMagazineIfNecessary( UINT16 usOldGun, UINT16 usOldAmmo, UINT16 usNewGun )
{
	UINT16 usNewAmmo = NOTHING;

	if ( (Magazine[ Item[ usOldAmmo ].ubClassIndex ].ubCalibre == Weapon[ usOldGun ].ubCalibre) &&
			 (Magazine[ Item[ usOldAmmo ].ubClassIndex ].ubMagSize == Weapon[ usOldGun ].ubMagSize) )
	{
		// must replace this!
		usNewAmmo = FindReplacementMagazine( Weapon[ usNewGun ].ubCalibre, Weapon[ usNewGun ].ubMagSize, Magazine[ Item[ usOldAmmo ].ubClassIndex ].ubAmmoType );
	}

	return( usNewAmmo );
}

// increase this if any gun can have more types that this
#define MAX_AMMO_TYPES_PER_GUN		3

UINT16 RandomMagazine( UINT16 usItem, UINT8 ubPercentStandard )
{
	// Note: if any ammo items in the item table are separated from the main group,
	// this function will have to be rewritten to scan the item table for an item
	// with item class ammo, which has class index ubLoop
	UINT16				usLoop;
	UINT16				usPossibleMagIndex[ MAX_AMMO_TYPES_PER_GUN ];
	UINT16				usPossibleMagCnt = 0;
	UINT8					ubMagChosen;

	if (!(Item[usItem].usItemClass & IC_GUN))
	{
		return( 0 );
	}

	WEAPONTYPE const* const pWeapon = &Weapon[usItem];

	// find & store all possible mag types that fit this gun
	usLoop = 0;
	while ( Magazine[ usLoop ].ubCalibre != NOAMMO )
	{
		if (Magazine[usLoop].ubCalibre == pWeapon->ubCalibre &&
				Magazine[usLoop].ubMagSize == pWeapon->ubMagSize)
		{
			// store it! (make sure array is big enough)
			Assert(usPossibleMagCnt < MAX_AMMO_TYPES_PER_GUN);
			usPossibleMagIndex[usPossibleMagCnt++] = usLoop;
		}

		usLoop++;
	}

	// no matches?
	if (usPossibleMagCnt == 0)
	{
		return( 0 );
	}
	else
	// only one match?
	if (usPossibleMagCnt == 1)
	{
		// use that, no choice
		return(MagazineClassIndexToItemType(usPossibleMagIndex[ 0 ] ));
	}
	else	// multiple choices
	{
		// Pick one at random, using supplied probability to pick the default
		if (Random(100) < ubPercentStandard)
		{
			ubMagChosen = 0;
		}
		else
		{
			// pick a non-standard type instead
			ubMagChosen = ( UINT8 ) (1 + Random(( UINT32 ) ( usPossibleMagCnt - 1 )));
		}

		return( MagazineClassIndexToItemType(usPossibleMagIndex[ ubMagChosen ] ) );
	}
}


static void CreateGun(UINT16 usItem, INT8 bStatus, OBJECTTYPE* pObj)
{
	UINT16 usAmmo;

	pObj->usItem = usItem;
	pObj->ubNumberOfObjects = 1;
	pObj->bGunStatus = bStatus;
	pObj->ubImprintID = NO_PROFILE;
	pObj->ubWeight = CalculateObjectWeight( pObj );

	if (Weapon[ usItem ].ubWeaponClass == MONSTERCLASS)
	{
		pObj->ubGunShotsLeft = Weapon[ usItem ].ubMagSize;
		pObj->ubGunAmmoType = AMMO_MONSTER;
	}
	else if ( EXPLOSIVE_GUN( usItem ) )
	{
		if ( usItem == ROCKET_LAUNCHER )
		{
			pObj->ubGunShotsLeft = 1;
		}
		else
		{
			// cannon
			pObj->ubGunShotsLeft = 0;
		}
		pObj->bGunAmmoStatus = 100;
		pObj->ubGunAmmoType = 0;
	}
	else
	{
		usAmmo = DefaultMagazine( usItem );
		pObj->usGunAmmoItem = usAmmo;
		pObj->ubGunAmmoType = Magazine[ Item[ usAmmo ].ubClassIndex].ubAmmoType;
		pObj->bGunAmmoStatus = 100;
		pObj->ubGunShotsLeft = Magazine[ Item[ usAmmo ].ubClassIndex ].ubMagSize;
		/*
		if (usItem == CAWS)
		{
			pObj->usAttachItem[0] = DUCKBILL;
			pObj->bAttachStatus[0] = 100;
		}
		*/
	}
}


static void CreateMagazine(UINT16 usItem, OBJECTTYPE* pObj)
{
	pObj->usItem = usItem;
	pObj->ubNumberOfObjects = 1;
	pObj->ubShotsLeft[0] = Magazine[ Item[usItem].ubClassIndex ].ubMagSize;
	pObj->ubWeight = CalculateObjectWeight( pObj );
}


void CreateItem(UINT16 const usItem, INT8 const bStatus, OBJECTTYPE* const pObj)
{
	memset( pObj, 0, sizeof( OBJECTTYPE ) );
	if (usItem >= MAXITEMS)
	{
		throw std::logic_error("Tried to create item with invalid ID");
	}

	if (Item[ usItem ].usItemClass == IC_GUN)
	{
		CreateGun( usItem, bStatus, pObj );
	}
	else if (Item[ usItem ].usItemClass == IC_AMMO)
	{
		CreateMagazine(usItem, pObj);
	}
	else
	{
		pObj->usItem = usItem;
		pObj->ubNumberOfObjects = 1;
		if (usItem == MONEY)
		{
			// special case... always set status to 100 when creating
			// and use status value to determine amount!
			pObj->bStatus[0] = 100;
			pObj->uiMoneyAmount = bStatus * 50;
		}
		else
		{
			pObj->bStatus[0] = bStatus;
		}
		pObj->ubWeight = CalculateObjectWeight( pObj );
	}

	if (Item[ usItem ].fFlags & ITEM_DEFAULT_UNDROPPABLE)
	{
		pObj->fFlags |= OBJECT_UNDROPPABLE;
	}
}


void CreateItems(UINT16 const usItem, INT8 const bStatus, UINT8 ubNumber, OBJECTTYPE* const pObj)
{
	UINT8		ubLoop;

	// can't create any more than this, the loop for setting the bStatus[] of others will overwrite memory!
	Assert( ubNumber <= MAX_OBJECTS_PER_SLOT );

	// ARM: to avoid whacking memory once Assertions are removed...  Items will be lost in this situation!
	if ( ubNumber > MAX_OBJECTS_PER_SLOT )
	{
		ubNumber = MAX_OBJECTS_PER_SLOT;
	}

	CreateItem(usItem, bStatus, pObj);
	for (ubLoop = 1; ubLoop < ubNumber; ubLoop++)
	{
		// we reference status[0] here because the status value might actually be a
		// # of rounds of ammo, in which case the value won't be the bStatus value
		// passed in.
		pObj->bStatus[ubLoop] = pObj->bStatus[0];
	}
	pObj->ubNumberOfObjects = ubNumber;
}


void CreateMoney(UINT32 const uiMoney, OBJECTTYPE* const pObj)
{
	CreateItem(MONEY, 100, pObj);
	pObj->uiMoneyAmount = uiMoney;
}


BOOLEAN ArmBomb( OBJECTTYPE * pObj, INT8 bSetting )
{
	BOOLEAN fRemote = FALSE;
	BOOLEAN fPressure = FALSE;
	BOOLEAN fTimed = FALSE;
	BOOLEAN	fSwitch = FALSE;

	if (pObj->usItem == ACTION_ITEM)
	{
		switch( pObj->bActionValue )
		{
			case ACTION_ITEM_SMALL_PIT:
			case ACTION_ITEM_LARGE_PIT:
				fPressure = TRUE;
				break;
			default:
				fRemote = TRUE;
				break;

		}
	}
	else if ( FindAttachment( pObj, DETONATOR ) != ITEM_NOT_FOUND )
	{
		fTimed = TRUE;
	}
	else if ( (FindAttachment( pObj, REMDETONATOR ) != ITEM_NOT_FOUND) || (pObj->usItem == ACTION_ITEM) )
	{
		fRemote = TRUE;
	}
	else if ( pObj->usItem == MINE || pObj->usItem == TRIP_FLARE || pObj->usItem == TRIP_KLAXON || pObj->usItem == ACTION_ITEM )
	{
		fPressure = TRUE;
	}
	else if ( pObj->usItem == SWITCH )
	{
		// this makes a remote detonator into a pressure-sensitive trigger
		if ( bSetting == PANIC_FREQUENCY )
		{
			// panic trigger is only activated by expending APs, not by
			// stepping on it... so don't define a detonator type
			fSwitch = TRUE;
		}
		else
		{
			fPressure = TRUE;
		}
	}
	else
	{
		// no sorta bomb at all!
		return( FALSE );
	}

	if (fRemote)
	{
		pObj->bDetonatorType = BOMB_REMOTE;
		pObj->bFrequency = bSetting;
	}
	else if (fPressure)
	{
		pObj->bDetonatorType = BOMB_PRESSURE;
		pObj->bFrequency = 0;
	}
	else if (fTimed)
	{
		pObj->bDetonatorType = BOMB_TIMED;
		// In realtime the player could choose to put down a bomb right before a turn expires, SO
		// add 1 to the setting in RT
		pObj->bDelay = bSetting;
		if ( !(gTacticalStatus.uiFlags & TURNBASED && gTacticalStatus.uiFlags & INCOMBAT) )
		{
			pObj->bDelay++;
		}

	}
	else if (fSwitch)
	{
		pObj->bDetonatorType = BOMB_SWITCH;
		pObj->bFrequency = bSetting;
	}
	else
	{
		return( FALSE );
	}

	pObj->fFlags |= OBJECT_ARMED_BOMB;
	pObj->usBombItem = pObj->usItem;
	return( TRUE );
}


static void RenumberAttachments(OBJECTTYPE* pObj)
{
	// loop through attachment positions and make sure we don't have any empty
	// attachment slots before filled ones
	INT8			bAttachPos;
	INT8			bFirstSpace;
	BOOLEAN		fDone = FALSE;

	while (!fDone)
	{
		bFirstSpace = -1;
		for (bAttachPos = 0; bAttachPos < MAX_ATTACHMENTS; bAttachPos++)
		{
			if (pObj->usAttachItem[ bAttachPos ] == NOTHING)
			{
				if (bFirstSpace == -1)
				{
					bFirstSpace = bAttachPos;
				}
			}
			else
			{
				if (bFirstSpace != -1)
				{
					// move the attachment!
					pObj->usAttachItem[ bFirstSpace ] = pObj->usAttachItem[ bAttachPos ];
					pObj->bAttachStatus[ bFirstSpace ] = pObj->bAttachStatus[ bAttachPos ];
					pObj->usAttachItem[ bAttachPos ] = NOTHING;
					pObj->bAttachStatus[ bAttachPos ] = 0;
					// restart loop at beginning, or quit if we reached the end of the
					// attachments
					break;
				}
			}
		}
		if (bAttachPos == MAX_ATTACHMENTS)
		{
			// done!!
			fDone = TRUE;
		}
	}

}

BOOLEAN RemoveAttachment( OBJECTTYPE * pObj, INT8 bAttachPos, OBJECTTYPE * pNewObj )
{
	INT8		bGrenade;

	CHECKF( pObj );

	if (bAttachPos < 0 || bAttachPos >= MAX_ATTACHMENTS)
	{
		return( FALSE );
	}
	if (pObj->usAttachItem[bAttachPos] == NOTHING )
	{
		return( FALSE );
	}

	if ( Item[ pObj->usAttachItem[bAttachPos] ].fFlags & ITEM_INSEPARABLE )
	{
		return( FALSE );
	}

	// if pNewObj is passed in NULL, then we just delete the attachment
	if (pNewObj != NULL)
	{
		CreateItem( pObj->usAttachItem[bAttachPos], pObj->bAttachStatus[bAttachPos], pNewObj );
	}

	pObj->usAttachItem[bAttachPos] = NOTHING;
	pObj->bAttachStatus[bAttachPos] = 0;

	if (pNewObj && pNewObj->usItem == UNDER_GLAUNCHER)
	{
		// look for any grenade; if it exists, we must make it an
		// attachment of the grenade launcher
		bGrenade = FindAttachmentByClass( pObj, IC_GRENADE );
		if (bGrenade != ITEM_NOT_FOUND)
		{
			pNewObj->usAttachItem[0] = pObj->usAttachItem[bGrenade];
			pNewObj->bAttachStatus[0] = pObj->bAttachStatus[bGrenade];
			pObj->usAttachItem[bGrenade] = NOTHING;
			pObj->bAttachStatus[bGrenade] = 0;
			pNewObj->ubWeight = CalculateObjectWeight( pNewObj );
		}
	}

	RenumberAttachments( pObj );

	pObj->ubWeight = CalculateObjectWeight( pObj );
	return( TRUE );
}


void SetNewItem(SOLDIERTYPE* pSoldier, UINT8 ubInvPos, BOOLEAN fNewItem)
{
	if( fNewItem )
	{
		pSoldier->bNewItemCount[ ubInvPos ]						 = -1;
		pSoldier->bNewItemCycleCount[ ubInvPos ]			 = NEW_ITEM_CYCLE_COUNT;
		pSoldier->fCheckForNewlyAddedItems             = TRUE;
	}
}


BOOLEAN PlaceObjectInSoldierProfile( UINT8 ubProfile, OBJECTTYPE *pObject )
{
	INT8				bLoop, bLoop2;
	UINT16			usItem;
	INT8				bStatus;
	BOOLEAN			fReturnVal = FALSE;

	usItem	= pObject->usItem;
	bStatus = pObject->bStatus[0];
	SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubProfile);

	if ( Item[ usItem ].usItemClass == IC_MONEY && gMercProfiles[ ubProfile ].uiMoney > 0 )
	{
		gMercProfiles[ ubProfile ].uiMoney += pObject->uiMoneyAmount;
		SetMoneyInSoldierProfile( ubProfile, gMercProfiles[ ubProfile ].uiMoney );
		return( TRUE );
	}

	for (bLoop = BIGPOCK1POS; bLoop < SMALLPOCK8POS; bLoop++)
	{
		if ( gMercProfiles[ ubProfile ].bInvNumber[ bLoop ] == 0 && (pSoldier == NULL || pSoldier->inv[ bLoop ].usItem == NOTHING ) )
		{

			// CJC: Deal with money by putting money into # stored in profile
			if ( Item[ usItem ].usItemClass == IC_MONEY )
			{
				gMercProfiles[ ubProfile ].uiMoney += pObject->uiMoneyAmount;
				// change any gold/silver to money
				usItem = MONEY;
			}
			else
			{
				gMercProfiles[ ubProfile ].inv[ bLoop ] = usItem;
				gMercProfiles[ ubProfile ].bInvStatus[ bLoop ] = bStatus;
				gMercProfiles[ ubProfile ].bInvNumber[ bLoop ] = pObject->ubNumberOfObjects;
			}

			fReturnVal = TRUE;
			break;
		}
	}

	//uiMoneyAmount
	if ( fReturnVal )
	{
		// ATE: Manage soldier pointer as well....
		//pSoldier = FindSoldierByProfileID(ubProfile);

		// Do we have a valid profile?
		if ( pSoldier != NULL )
		{
			// OK, place in soldier...
			if ( usItem == MONEY )
			{
				CreateMoney( gMercProfiles[ ubProfile ].uiMoney, &(pSoldier->inv[ bLoop ] ) );
			}
			else
			{
				if ( pSoldier->ubProfile == MADLAB )
				{
					// remove attachments and drop them
					OBJECTTYPE			Attachment;

					for ( bLoop2 = MAX_ATTACHMENTS - 1; bLoop2 >= 0; bLoop2-- )
					{
						// remove also checks for existence attachment
						if (RemoveAttachment(pObject, bLoop2, &Attachment))
						{
							// drop it in Madlab's tile
							AddItemToPool(pSoldier->sGridNo, &Attachment, VISIBLE, 0, 0, 0);
						}
					}
				}

				CreateItem( usItem, bStatus, &(pSoldier->inv[ bLoop ] ) );
			}
		}
	}

	return( fReturnVal );
}


static void RemoveInvObject(SOLDIERTYPE* pSoldier, UINT16 usItem);


BOOLEAN RemoveObjectFromSoldierProfile( UINT8 ubProfile, UINT16 usItem )
{
	INT8 bLoop;
	BOOLEAN	fReturnVal = FALSE;

	if ( usItem == NOTHING )
	{
		return( TRUE );
	}

	for (bLoop = 0; bLoop < 19; bLoop++)
	{
		if ( gMercProfiles[ ubProfile ].inv[ bLoop ] == usItem )
		{
			gMercProfiles[ ubProfile ].inv[ bLoop ] = NOTHING;
			gMercProfiles[ ubProfile ].bInvStatus[ bLoop ] = 0;
			gMercProfiles[ ubProfile ].bInvNumber[ bLoop ] = 0;

			fReturnVal = TRUE;
			break;
		}
	}

	// ATE: Manage soldier pointer as well....
	SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubProfile);

	// Do we have a valid profile?
	if ( pSoldier != NULL )
	{
		// Remove item...
		RemoveInvObject( pSoldier, usItem );
	}

	return( fReturnVal );
}


void SetMoneyInSoldierProfile( UINT8 ubProfile, UINT32 uiMoney )
{
	//INT8						bSlot;
	OBJECTTYPE			Object;
	//SOLDIERTYPE *		pSoldier;
	BOOLEAN					fRet;

	// remove all money from soldier
	do
	{
		fRet = RemoveObjectFromSoldierProfile( ubProfile, MONEY );
	}
	while (fRet);

	gMercProfiles[ ubProfile ].uiMoney = 0;

	if (uiMoney > 0)
	{
		// now add the amount specified
		CreateMoney( uiMoney, &Object );
		PlaceObjectInSoldierProfile( ubProfile, &Object );
	}
}

INT8 FindObjectInSoldierProfile( UINT8 ubProfile, UINT16 usItem )
{
	INT8	bLoop;

	for (bLoop = 0; bLoop < 19; bLoop++)
	{
		if ( gMercProfiles[ ubProfile ].bInvNumber[ bLoop ] > 0 )
		{
			if ( gMercProfiles[ ubProfile ].inv[ bLoop ] == usItem )
			{
				return( bLoop );
			}
		}
	}
	return( NO_SLOT );
}

BOOLEAN ObjectExistsInSoldierProfile( UINT8 ubProfile, UINT16 usItem )
{
	INT8	bSlot;

	bSlot = FindObjectInSoldierProfile( ubProfile, usItem );
	return( bSlot != NO_SLOT );
}


static void RemoveInvObject(SOLDIERTYPE* pSoldier, UINT16 usItem)
{
	INT8 bInvPos;

	// find object
	bInvPos = FindObj( pSoldier, usItem );
	if (bInvPos != NO_SLOT)
	{

		// Erase!
		memset( &(pSoldier->inv[ bInvPos ]), 0, sizeof( OBJECTTYPE ) );

		//Dirty!
		DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
	}

}


static INT8 CheckItemForDamage(UINT16 usItem, INT32 iMaxDamage)
{
	INT8	bDamage = 0;

	// if the item is protective armour, reduce the amount of damage
	// by its armour value
	if (Item[usItem].usItemClass == IC_ARMOUR)
	{
		iMaxDamage -= (iMaxDamage * Armour[Item[usItem].ubClassIndex].ubProtection) / 100;
	}
	// metal items are tough and will be damaged less
	if (Item[usItem].fFlags & ITEM_METAL)
	{
		iMaxDamage /= 2;
	}
	else if ( usItem == BLOODCAT_PELT )
	{
		iMaxDamage *= 2;
	}
	if (iMaxDamage > 0)
	{
		bDamage = (INT8) PreRandom( iMaxDamage );
	}
	return( bDamage );
}


static BOOLEAN CheckForChainReaction(UINT16 usItem, INT8 bStatus, INT8 bDamage, BOOLEAN fOnGround)
{
	INT32 iChance;

	iChance = Explosive[Item[usItem].ubClassIndex].ubVolatility;
	if (iChance > 0)
	{

		// Scale the base chance by the damage caused to the item
		// (bigger the shock, bigger chance) and the condition of
		// the item after being hit!
		if (fOnGround)
		{
			// improve chance to make it practical to blow up explosives on the ground
			iChance = 50 + (iChance - 1) * 10;
		}

		iChance = iChance * ( 100 + ( (100 - bStatus) + bDamage ) / 2 ) / 100;
		if ((INT32) PreRandom( 100 ) < iChance)
		{
			return( TRUE );
		}
	}
	return( FALSE );
}


static BOOLEAN DamageItem(OBJECTTYPE* pObject, INT32 iDamage, BOOLEAN fOnGround)
{
	INT8		bLoop;
	INT8		bDamage;

	if ( (Item[pObject->usItem].fFlags & ITEM_DAMAGEABLE || Item[ pObject->usItem ].usItemClass == IC_AMMO) && pObject->ubNumberOfObjects > 0)
	{

		for (bLoop = 0; bLoop < pObject->ubNumberOfObjects; bLoop++)
		{
			// if the status of the item is negative then it's trapped/jammed;
			// leave it alone
			if (pObject->usItem != NOTHING && pObject->bStatus[bLoop] > 0)
			{
				bDamage = CheckItemForDamage( pObject->usItem, iDamage );
				switch( pObject->usItem )
				{
					case JAR_CREATURE_BLOOD:
					case JAR:
					case JAR_HUMAN_BLOOD:
					case JAR_ELIXIR:
						if ( PreRandom( bDamage ) > 5 )
						{
							// smash!
							bDamage = pObject->bStatus[ bLoop ];
						}
						break;
					default:
						break;
				}
				if ( Item[ pObject->usItem ].usItemClass == IC_AMMO  )
				{
					if ( PreRandom( 100 ) < (UINT32) bDamage )
					{
						// destroy clip completely
						pObject->bStatus[ bLoop ] = 1;
					}
				}
				else
				{
					pObject->bStatus[bLoop] -= bDamage;
					if (pObject->bStatus[bLoop] < 1)
					{
						pObject->bStatus[bLoop] = 1;
					}
				}
				// I don't think we increase viewrange based on items any more
				// FUN STUFF!  Check for explosives going off as a result!
				if (Item[pObject->usItem].usItemClass & IC_EXPLOSV)
				{
					if (CheckForChainReaction( pObject->usItem, pObject->bStatus[bLoop], bDamage, fOnGround ))
					{
						return( TRUE );
					}
				}

				// remove item from index AFTER checking explosions because need item data for explosion!
				if ( pObject->bStatus[bLoop] == 1 )
				{
					if ( pObject->ubNumberOfObjects > 1 )
					{
						RemoveObjFrom( pObject, bLoop );
						// since an item was just removed, the items above the current were all shifted down one;
						// to process them properly, we have to back up 1 in the counter
						bLoop = bLoop - 1;
					}
				}
			}
		}

		for (bLoop = 0; bLoop < MAX_ATTACHMENTS; bLoop++)
		{
			if (pObject->usAttachItem[bLoop] != NOTHING && pObject->bAttachStatus[bLoop] > 0)
			{
				pObject->bAttachStatus[bLoop] -= CheckItemForDamage( pObject->usAttachItem[bLoop], iDamage );
				if (pObject->bAttachStatus[bLoop] < 1)
				{
					pObject->bAttachStatus[bLoop] = 1;
				}
			}
		}
	}

	return( FALSE );
}

void CheckEquipmentForDamage( SOLDIERTYPE *pSoldier, INT32 iDamage )
{
	BOOLEAN			fBlowsUp;
	UINT8				ubNumberOfObjects;

	if ( TANK( pSoldier ) )
	{
		return;
	}

	FOR_ALL_SOLDIER_INV_SLOTS(i, *pSoldier)
	{
		ubNumberOfObjects = i->ubNumberOfObjects;
		fBlowsUp = DamageItem(i, iDamage, FALSE);
		if (fBlowsUp)
		{
			// blow it up!
			SOLDIERTYPE* const owner = (gTacticalStatus.ubAttackBusyCount ? pSoldier->attacker : pSoldier);
			IgniteExplosion(owner, 0, pSoldier->sGridNo, i->usItem, pSoldier->bLevel);

			// Remove item!
			DeleteObj(i);

			DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
		}
		else if (ubNumberOfObjects != i->ubNumberOfObjects)
		{
			DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
		}
	}
}

void CheckEquipmentForFragileItemDamage( SOLDIERTYPE *pSoldier, INT32 iDamage )
{
	// glass jars etc can be damaged by falling over
	UINT8				ubNumberOfObjects;
	BOOLEAN			fPlayedGlassBreak = FALSE;

	FOR_ALL_SOLDIER_INV_SLOTS(i, *pSoldier)
	{
		switch (i->usItem)
		{
			case JAR_CREATURE_BLOOD:
			case JAR:
			case JAR_HUMAN_BLOOD:
			case JAR_ELIXIR:
				ubNumberOfObjects = i->ubNumberOfObjects;
				DamageItem(i, iDamage, FALSE);
				if (!fPlayedGlassBreak && ubNumberOfObjects != i->ubNumberOfObjects)
				{
					PlayLocationJA2Sample(pSoldier->sGridNo, GLASS_CRACK, MIDVOLUME, 1);
					fPlayedGlassBreak = TRUE;
					// only dirty once
					DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
				}
				break;
			default:
				break;
		}
	}
}


BOOLEAN DamageItemOnGround(OBJECTTYPE* const pObject, const INT16 sGridNo, const INT8 bLevel, const INT32 iDamage, SOLDIERTYPE* const owner)
{
	BOOLEAN			fBlowsUp;

	fBlowsUp = DamageItem( pObject, iDamage, TRUE );
	if ( fBlowsUp )
	{
		// OK, Ignite this explosion!
		IgniteExplosion(owner, 0, sGridNo, pObject->usItem, bLevel);

		// Remove item!
		return( TRUE );
	}
	else if ( (pObject->ubNumberOfObjects < 2) && (pObject->bStatus[0] < USABLE) )
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


BOOLEAN IsMedicalKitItem(const OBJECTTYPE* pObject)
{
	return pObject->usItem == MEDICKIT;
}


void SwapHandItems( SOLDIERTYPE * pSoldier )
{
	BOOLEAN		fOk;

	CHECKV( pSoldier );
	if (pSoldier->inv[HANDPOS].usItem == NOTHING || pSoldier->inv[SECONDHANDPOS].usItem == NOTHING)
	{
		// whatever is in the second hand can be swapped to the main hand!
		SwapObjs( &(pSoldier->inv[HANDPOS]), &(pSoldier->inv[SECONDHANDPOS]) );
		DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
	}
	else
	{
		if (TwoHandedItem( pSoldier->inv[SECONDHANDPOS].usItem ) )
		{
			// must move the item in the main hand elsewhere in the inventory
			fOk = InternalAutoPlaceObject( pSoldier, &(pSoldier->inv[HANDPOS]), FALSE, HANDPOS );
			if (!fOk)
			{
				return;
			}
			// the main hand is now empty so a swap is going to work...
		}
		SwapObjs( &(pSoldier->inv[HANDPOS]), &(pSoldier->inv[SECONDHANDPOS]) );
		DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
	}
}


void WaterDamage( SOLDIERTYPE *pSoldier )
{
	// damage guy's equipment and camouflage due to water
	INT8		bDamage, bDieSize;
	UINT32	uiRoll;

	if ( pSoldier->bOverTerrainType == DEEP_WATER )
	{
		FOR_ALL_SOLDIER_INV_SLOTS(i, *pSoldier)
		{
			// if there's an item here that can get water damaged...
			if (i->usItem && Item[i->usItem].fFlags & ITEM_WATER_DAMAGES)
			{
				// roll the 'ol 100-sided dice
				uiRoll = PreRandom(100);

				// 10% chance of getting damage!
				if (uiRoll < 10)
				{
					// lose between 1 and 10 status points each time
					bDamage = (INT8) (10 - uiRoll);

					// but don't let anything drop lower than 1%
					i->bStatus[0] -= bDamage;
					if (i->bStatus[0] < 1)
					{
						i->bStatus[0] = 1;
					}
				}
			}
		}
	}
	if (pSoldier->bCamo > 0 && !HAS_SKILL_TRAIT( pSoldier, CAMOUFLAGED ) )
	{
		// reduce camouflage by 2% per tile of deep water
		// and 1% for medium water
		if ( pSoldier->bOverTerrainType == DEEP_WATER )
		{
			pSoldier->bCamo = __max( 0, pSoldier->bCamo - 2 );
		}
		else
		{
			pSoldier->bCamo = __max( 0, pSoldier->bCamo - 1 );
		}
		if (pSoldier->bCamo == 0)
		{
			// Reload palettes....
			if ( pSoldier->bInSector )
			{
				CreateSoldierPalettes( pSoldier );
			}
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_CAMO_WASHED_OFF], pSoldier->name);
		}
	}
	if ( pSoldier->bTeam == gbPlayerNum && pSoldier->bMonsterSmell > 0 )
	{
		if ( pSoldier->bOverTerrainType == DEEP_WATER )
		{
			bDieSize = 10;
		}
		else
		{
			bDieSize = 20;
		}
		if ( Random( bDieSize ) == 0 )
		{
			pSoldier->bMonsterSmell--;
		}
	}

	DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
}


BOOLEAN ApplyCamo(SOLDIERTYPE* const pSoldier, OBJECTTYPE* const pObj, BOOLEAN* const pfGoodAPs)
{
	INT8		bPointsToUse;
	UINT16	usTotalKitPoints;

  (*pfGoodAPs) = TRUE;

	if (pObj->usItem != CAMOUFLAGEKIT)
	{
		return( FALSE );
	}

	if (!EnoughPoints( pSoldier, AP_CAMOFLAGE, 0, TRUE ) )
	{
    (*pfGoodAPs) = FALSE;
		return( TRUE );
	}

	usTotalKitPoints = TotalPoints( pObj );
	if (usTotalKitPoints == 0)
	{
		// HUH???
		return( FALSE );
	}

	if (pSoldier->bCamo == 100)
	{
		// nothing more to add
		return( FALSE );
	}

	// points are used up at a rate of 50% kit = 100% camo on guy
	// add 1 to round off
	bPointsToUse = (100 - pSoldier->bCamo + 1 ) / 2;
	bPointsToUse = __min( bPointsToUse, usTotalKitPoints );
	pSoldier->bCamo = __min( 100, pSoldier->bCamo + bPointsToUse * 2);

	UseKitPoints( pObj, bPointsToUse, pSoldier );

	DeductPoints( pSoldier, AP_CAMOFLAGE, 0 );

	// Reload palettes....
	if ( pSoldier->bInSector )
	{
		CreateSoldierPalettes( pSoldier );
	}

	return( TRUE );
}

BOOLEAN ApplyCanteen( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj, BOOLEAN *pfGoodAPs )
{
	INT16		sPointsToUse;
	UINT16	usTotalKitPoints;

  (*pfGoodAPs) = TRUE;

	if (pObj->usItem != CANTEEN)
	{
		return( FALSE );
	}

	usTotalKitPoints = TotalPoints( pObj );
	if (usTotalKitPoints == 0)
	{
		// HUH???
		return( FALSE );
	}

	if (!EnoughPoints( pSoldier, AP_DRINK, 0, TRUE ) )
	{
    (*pfGoodAPs) = FALSE;
		return( TRUE );
	}

  if ( pSoldier->bTeam == gbPlayerNum )
  {
    if ( gMercProfiles[ pSoldier->ubProfile ].bSex == MALE )
    {
		  PlayJA2Sample(DRINK_CANTEEN_MALE, MIDVOLUME, 1, MIDDLEPAN);
    }
    else
    {
		  PlayJA2Sample(DRINK_CANTEEN_FEMALE, MIDVOLUME, 1, MIDDLEPAN);
    }
  }

	sPointsToUse = __min( 20, usTotalKitPoints );

	// CJC Feb 9.  Canteens don't seem effective enough, so doubled return from them
	DeductPoints( pSoldier, AP_DRINK, (INT16) (2 * sPointsToUse * -(100 - pSoldier->bBreath) ) );

	UseKitPoints( pObj, sPointsToUse, pSoldier );

	return( TRUE );
}

#define MAX_HUMAN_CREATURE_SMELL (NORMAL_HUMAN_SMELL_STRENGTH - 1)

BOOLEAN ApplyElixir( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj, BOOLEAN *pfGoodAPs )
{
	INT16		sPointsToUse;
	UINT16	usTotalKitPoints;

  (*pfGoodAPs) = TRUE;

	if (pObj->usItem != JAR_ELIXIR )
	{
		return( FALSE );
	}

	usTotalKitPoints = TotalPoints( pObj );
	if (usTotalKitPoints == 0)
	{
		// HUH???
		return( FALSE );
	}

	if (!EnoughPoints( pSoldier, AP_CAMOFLAGE, 0, TRUE ) )
	{
    (*pfGoodAPs) = FALSE;
		return( TRUE );
	}

	DeductPoints( pSoldier, AP_CAMOFLAGE, 0 );

	sPointsToUse = ( MAX_HUMAN_CREATURE_SMELL - pSoldier->bMonsterSmell ) * 2;
	sPointsToUse = __min( sPointsToUse, usTotalKitPoints );

	UseKitPoints( pObj, sPointsToUse, pSoldier );

	pSoldier->bMonsterSmell += sPointsToUse / 2;

	return( TRUE );
}


bool ItemIsCool(OBJECTTYPE const& o)
{
	if (o.bStatus[0] < 60) return false;
	INVTYPE const& item = Item[o.usItem];
	if (item.usItemClass & IC_WEAPON)
	{
		if (Weapon[o.usItem].ubDeadliness >= 30) return true;
	}
	else if (item.usItemClass & IC_ARMOUR)
	{
		if (Armour[item.ubClassIndex].ubProtection >= 20) return true;
	}

	return false;
}

void ActivateXRayDevice( SOLDIERTYPE * pSoldier )
{
	INT8					bBatteries;

	// check for batteries
	bBatteries = FindAttachment( &(pSoldier->inv[HANDPOS]), BATTERIES );
	if ( bBatteries == NO_SLOT )
	{
		// doesn't work without batteries!
		return;
	}

	// use up 8-12 percent of batteries
	pSoldier->inv[ HANDPOS ].bAttachStatus[ bBatteries ] -= (INT8) (8 + Random( 5 ));
	if ( pSoldier->inv[ HANDPOS ].bAttachStatus[ bBatteries ] <= 0 )
	{
		// destroy batteries
		pSoldier->inv[ HANDPOS ].usAttachItem[ bBatteries ] = NOTHING;
		pSoldier->inv[ HANDPOS ].bAttachStatus[ bBatteries ] = 0;
	}

	// first, scan through all mercs and turn off xrayed flag for anyone
	// previously xrayed by this guy
	FOR_ALL_MERCS(i)
	{
		SOLDIERTYPE* const tgt = *i;
		if (tgt->ubMiscSoldierFlags &  SOLDIER_MISC_XRAYED &&
				tgt->xrayed_by          == pSoldier)
		{
			tgt->ubMiscSoldierFlags &= ~SOLDIER_MISC_XRAYED;
			tgt->xrayed_by           = NULL;
		}
	}
	// now turn on xray for anyone within range
	FOR_ALL_MERCS(i)
	{
		SOLDIERTYPE* const tgt = *i;
		if (tgt->bTeam != pSoldier->bTeam &&
				PythSpacesAway(pSoldier->sGridNo, tgt->sGridNo) < XRAY_RANGE)
		{
			tgt->ubMiscSoldierFlags |= SOLDIER_MISC_XRAYED;
			tgt->xrayed_by           = pSoldier;
		}
	}
	pSoldier->uiXRayActivatedTime = GetWorldTotalSeconds();
}

void TurnOffXRayEffects( SOLDIERTYPE * pSoldier )
{
	if ( !pSoldier->uiXRayActivatedTime )
	{
		return;
	}

	// scan through all mercs and turn off xrayed flag for anyone
	// xrayed by this guy
	FOR_ALL_MERCS(i)
	{
		SOLDIERTYPE* const tgt = *i;
		if (tgt->ubMiscSoldierFlags &  SOLDIER_MISC_XRAYED &&
				tgt->xrayed_by          == pSoldier)
		{
			tgt->ubMiscSoldierFlags &= ~SOLDIER_MISC_XRAYED;
			tgt->xrayed_by           = NULL;
		}
	}
	pSoldier->uiXRayActivatedTime = 0;
}


bool HasObjectImprint(OBJECTTYPE const& o)
{
	return
		(o.usItem == ROCKET_RIFLE || o.usItem == AUTO_ROCKET_RIFLE) &&
		o.ubImprintID != NO_PROFILE;
}


#ifdef JA2TESTVERSION
void DumpItemsList( void )
{
  CHAR8 zPrintFileName[60];
  FILE *FDump;
	UINT16 usItem;

  // open output file
 	strcpy(zPrintFileName, "ItemDump.txt");
  FDump = fopen(zPrintFileName, "w");

  if (FDump == NULL)
    return;

	// print headings
	fprintf(FDump, "            ITEM              COOLNESS  VALUE\n");
	fprintf(FDump, "============================  ========  =====\n");

	for( usItem = 0; usItem < MAXITEMS; usItem++ )
	{
		const INVTYPE* const pItem = &Item[usItem];
		if (pItem->ubCoolness > 0 )
		{
			fprintf(FDump, "%28ls     %2d     $%4d\n", ItemNames[ usItem ], pItem->ubCoolness, pItem->usPrice );
		}
	}

  fclose(FDump);
}
#endif // JA2TESTVERSION
