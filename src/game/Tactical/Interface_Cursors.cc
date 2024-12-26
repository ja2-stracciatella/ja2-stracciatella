#include "MouseSystem.h"
#include "TileDef.h"
#include "Timer_Control.h"
#include "Touch_UI.h"
#include "Handle_UI.h"
#include "Cursors.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "Interface_Cursors.h"
#include "Interface.h"
#include "Isometric_Utils.h"
#include "Overhead.h"
#include "Interface_Items.h"
#include "GameSettings.h"
#include "Video.h"


#define DISPLAY_AP_INDEX		MOCKFLOOR1

#define SNAPCURSOR_AP_X_STARTVAL	18
#define SNAPCURSOR_AP_Y_STARTVAL	9


const UICursor gUICursors[NUM_UI_CURSORS] =
{
	{NO_UICURSOR,                        0,                                                                                                    0,                         0               },
	{NORMAL_FREEUICURSOR,                UICURSOR_FREEFLOWING,                                                                                 CURSOR_NORMAL,             0               },
	{NORMAL_SNAPUICURSOR,                UICURSOR_SNAPPING,                                                                                    0,                         0               },
	{MOVE_RUN_UICURSOR,                  UICURSOR_FREEFLOWING | UICURSOR_SHOWTILEAPDEPENDENT | UICURSOR_DONTSHOW2NDLEVEL,                      CURSOR_RUN1,               FIRSTPOINTERS2  },
	{MOVE_WALK_UICURSOR,                 UICURSOR_FREEFLOWING | UICURSOR_SHOWTILEAPDEPENDENT | UICURSOR_DONTSHOW2NDLEVEL | UICURSOR_CENTERAPS, CURSOR_WALK1,              FIRSTPOINTERS2  },
	{MOVE_SWAT_UICURSOR,                 UICURSOR_FREEFLOWING | UICURSOR_SHOWTILEAPDEPENDENT | UICURSOR_DONTSHOW2NDLEVEL,                      CURSOR_SWAT1,              FIRSTPOINTERS2  },
	{MOVE_PRONE_UICURSOR,                UICURSOR_FREEFLOWING | UICURSOR_SHOWTILEAPDEPENDENT | UICURSOR_DONTSHOW2NDLEVEL,                      CURSOR_PRONE1,             FIRSTPOINTERS2  },
	{MOVE_VEHICLE_UICURSOR,              UICURSOR_FREEFLOWING | UICURSOR_SHOWTILEAPDEPENDENT | UICURSOR_DONTSHOW2NDLEVEL,                      CURSOR_DRIVEV,             FIRSTPOINTERS2  },

	{CONFIRM_MOVE_RUN_UICURSOR,          UICURSOR_SNAPPING | UICURSOR_SHOWTILE | UICURSOR_DONTSHOW2NDLEVEL,                                    0,                         FIRSTPOINTERS4  },
	{CONFIRM_MOVE_WALK_UICURSOR,         UICURSOR_SNAPPING | UICURSOR_SHOWTILE | UICURSOR_DONTSHOW2NDLEVEL,                                    0,                         FIRSTPOINTERS4  },
	{CONFIRM_MOVE_SWAT_UICURSOR,         UICURSOR_SNAPPING | UICURSOR_SHOWTILE | UICURSOR_DONTSHOW2NDLEVEL,                                    0,                         FIRSTPOINTERS4  },
	{CONFIRM_MOVE_PRONE_UICURSOR,        UICURSOR_SNAPPING | UICURSOR_SHOWTILE | UICURSOR_DONTSHOW2NDLEVEL,                                    0,                         FIRSTPOINTERS4  },
	{CONFIRM_MOVE_VEHICLE_UICURSOR,      UICURSOR_SNAPPING | UICURSOR_SHOWTILE | UICURSOR_DONTSHOW2NDLEVEL,                                    0,                         FIRSTPOINTERS4  },

	{ALL_MOVE_RUN_UICURSOR,              UICURSOR_SNAPPING | UICURSOR_SHOWTILE | UICURSOR_DONTSHOW2NDLEVEL,                                    0,                         FIRSTPOINTERS5  },
	{ALL_MOVE_WALK_UICURSOR,             UICURSOR_SNAPPING | UICURSOR_SHOWTILE | UICURSOR_DONTSHOW2NDLEVEL,                                    0,                         FIRSTPOINTERS5  },
	{ALL_MOVE_SWAT_UICURSOR,             UICURSOR_SNAPPING | UICURSOR_SHOWTILE | UICURSOR_DONTSHOW2NDLEVEL,                                    0,                         FIRSTPOINTERS5  },
	{ALL_MOVE_PRONE_UICURSOR,            UICURSOR_SNAPPING | UICURSOR_SHOWTILE | UICURSOR_DONTSHOW2NDLEVEL,                                    0,                         FIRSTPOINTERS5  },
	{ALL_MOVE_VEHICLE_UICURSOR,          UICURSOR_SNAPPING | UICURSOR_SHOWTILE | UICURSOR_DONTSHOW2NDLEVEL,                                    0,                         FIRSTPOINTERS5  },

	{MOVE_REALTIME_UICURSOR,             UICURSOR_FREEFLOWING | UICURSOR_SHOWTILEAPDEPENDENT | UICURSOR_DONTSHOW2NDLEVEL,                      VIDEO_DEFAULT_TO_NO_CURSOR,           FIRSTPOINTERS2  },
	{MOVE_RUN_REALTIME_UICURSOR,         UICURSOR_FREEFLOWING | UICURSOR_SHOWTILE | UICURSOR_DONTSHOW2NDLEVEL,                                 VIDEO_DEFAULT_TO_NO_CURSOR,           FIRSTPOINTERS7  },

	{CONFIRM_MOVE_REALTIME_UICURSOR,     UICURSOR_FREEFLOWING | UICURSOR_SHOWTILE | UICURSOR_DONTSHOW2NDLEVEL,                                 VIDEO_DEFAULT_TO_NO_CURSOR,           FIRSTPOINTERS4  },
	{ALL_MOVE_REALTIME_UICURSOR,         UICURSOR_FREEFLOWING | UICURSOR_SHOWTILE | UICURSOR_DONTSHOW2NDLEVEL,                                 VIDEO_DEFAULT_TO_NO_CURSOR,           FIRSTPOINTERS5  },

	{ON_OWNED_MERC_UICURSOR,             UICURSOR_SNAPPING,                                                                                    0,                         0               },
	{ON_OWNED_SELMERC_UICURSOR,          UICURSOR_SNAPPING,                                                                                    0,                         0               },
	{ACTION_SHOOT_UICURSOR,              UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGET,             0               },
	{ACTION_NOCHANCE_SHOOT_UICURSOR,     UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETDKBLACK,      0               },
	{ACTION_NOCHANCE_BURST_UICURSOR,     UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETBURSTDKBLACK, 0               },

	{ACTION_FLASH_TOSS_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGET,             0               },
	{ACTION_TOSS_UICURSOR,               UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGET,             0               },
	{ACTION_RED_TOSS_UICURSOR,           UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETRED,          0               },

	{ACTION_FLASH_SHOOT_UICURSOR,        UICURSOR_FREEFLOWING,                                                                                 CURSOR_FLASH_TARGET,       0               },
	{ACTION_FLASH_BURST_UICURSOR,        UICURSOR_FREEFLOWING,                                                                                 CURSOR_FLASH_TARGETBURST,  0               },
	{ACTION_TARGETAIM1_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETON1,          0               },
	{ACTION_TARGETAIM2_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETON2,          0               },
	{ACTION_TARGETAIM3_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETON3,          0               },
	{ACTION_TARGETAIM4_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETON4,          0               },
	{ACTION_TARGETAIM5_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETON5,          0               },
	{ACTION_TARGETAIM6_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETON6,          0               },
	{ACTION_TARGETAIM7_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETON7,          0               },
	{ACTION_TARGETAIM8_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETON8,          0               },
	{ACTION_TARGETAIM8_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETON9,          0               },
	{ACTION_TARGETAIMCANT1_UICURSOR,     UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETW1,           0               },
	{ACTION_TARGETAIMCANT2_UICURSOR,     UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETW2,           0               },
	{ACTION_TARGETAIMCANT3_UICURSOR,     UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETW3,           0               },
	{ACTION_TARGETAIMCANT4_UICURSOR,     UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETW4,           0               },
	{ACTION_TARGETAIMCANT5_UICURSOR,     UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETW5,           0               },
	{ACTION_TARGETRED_UICURSOR,          UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETRED,          0               },
	{ACTION_TARGETBURST_UICURSOR,        UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETBURST,        0               },
	{ACTION_TARGETREDBURST_UICURSOR,     UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETBURSTRED,     0               },
	{ACTION_TARGETCONFIRMBURST_UICURSOR, UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETBURSTCONFIRM, 0               },

	{ACTION_TARGETAIMFULL_UICURSOR,      UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETWR1,          0               },
	{ACTION_TARGETAIMYELLOW1_UICURSOR,   UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETYELLOW1,      0               },
	{ACTION_TARGETAIMYELLOW2_UICURSOR,   UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETYELLOW2,      0               },
	{ACTION_TARGETAIMYELLOW3_UICURSOR,   UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETYELLOW3,      0               },
	{ACTION_TARGETAIMYELLOW4_UICURSOR,   UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETYELLOW4,      0               },


	{ACTION_TARGET_RELOADING,            UICURSOR_FREEFLOWING,                                                                                 CURSOR_TARGETBLACK,        0               },
	{ACTION_PUNCH_GRAY,                  UICURSOR_FREEFLOWING,                                                                                 CURSOR_PUNCHGRAY,          0               },
	{ACTION_PUNCH_RED,                   UICURSOR_FREEFLOWING,                                                                                 CURSOR_PUNCHRED,           0               },
	{ACTION_PUNCH_RED_AIM1_UICURSOR,     UICURSOR_FREEFLOWING,                                                                                 CURSOR_PUNCHRED_ON1,       0               },
	{ACTION_PUNCH_RED_AIM2_UICURSOR,     UICURSOR_FREEFLOWING,                                                                                 CURSOR_PUNCHRED_ON2,       0               },
	{ACTION_PUNCH_YELLOW_AIM1_UICURSOR,  UICURSOR_FREEFLOWING,                                                                                 CURSOR_PUNCHYELLOW_ON1,    0               },
	{ACTION_PUNCH_YELLOW_AIM2_UICURSOR,  UICURSOR_FREEFLOWING,                                                                                 CURSOR_PUNCHYELLOW_ON2,    0               },
	{ACTION_PUNCH_NOGO_AIM1_UICURSOR,    UICURSOR_FREEFLOWING,                                                                                 CURSOR_PUNCHNOGO_ON1,      0               },
	{ACTION_PUNCH_NOGO_AIM2_UICURSOR,    UICURSOR_FREEFLOWING,                                                                                 CURSOR_PUNCHNOGO_ON2,      0               },
	{ACTION_FIRSTAID_GRAY,               UICURSOR_FREEFLOWING,                                                                                 CURSOR_CROSS_REG,          0               },
	{ACTION_FIRSTAID_RED,                UICURSOR_FREEFLOWING,                                                                                 CURSOR_CROSS_ACTIVE,       0               },
	{ACTION_OPEN,                        UICURSOR_FREEFLOWING,                                                                                 CURSOR_HANDGRAB,           0               },
	{CANNOT_MOVE_UICURSOR,               UICURSOR_SNAPPING,                                                                                    0,                         0               },
	{NORMALHANDCURSOR_UICURSOR,          UICURSOR_FREEFLOWING,                                                                                 CURSOR_NORMGRAB,           0               },
	{OKHANDCURSOR_UICURSOR,              UICURSOR_FREEFLOWING,                                                                                 CURSOR_HANDGRAB,           0               },
	{KNIFE_REG_UICURSOR,                 UICURSOR_FREEFLOWING,                                                                                 CURSOR_KNIFE_REG,          0               },
	{KNIFE_HIT_UICURSOR,                 UICURSOR_FREEFLOWING,                                                                                 CURSOR_KNIFE_HIT,          0               },
	{KNIFE_HIT_AIM1_UICURSOR,            UICURSOR_FREEFLOWING,                                                                                 CURSOR_KNIFE_HIT_ON1,      0               },
	{KNIFE_HIT_AIM2_UICURSOR,            UICURSOR_FREEFLOWING,                                                                                 CURSOR_KNIFE_HIT_ON2,      0               },
	{KNIFE_YELLOW_AIM1_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_KNIFE_YELLOW_ON1,   0               },
	{KNIFE_YELLOW_AIM2_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_KNIFE_YELLOW_ON2,   0               },
	{KNIFE_NOGO_AIM1_UICURSOR,           UICURSOR_FREEFLOWING,                                                                                 CURSOR_KNIFE_NOGO_ON1,     0               },
	{KNIFE_NOGO_AIM2_UICURSOR,           UICURSOR_FREEFLOWING,                                                                                 CURSOR_KNIFE_NOGO_ON2,     0               },

	{LOOK_UICURSOR,                      UICURSOR_FREEFLOWING,                                                                                 CURSOR_LOOK,               0               },
	{TALK_NA_UICURSOR,                   UICURSOR_FREEFLOWING,                                                                                 CURSOR_TALK,               0               },
	{TALK_A_UICURSOR,                    UICURSOR_FREEFLOWING,                                                                                 CURSOR_REDTALK,            0               },
	{TALK_OUT_RANGE_NA_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_FLASH_TALK,         CURSOR_BLACKTALK},
	{TALK_OUT_RANGE_A_UICURSOR,          UICURSOR_FREEFLOWING,                                                                                 CURSOR_FLASH_REDTALK,      CURSOR_BLACKTALK},
	{EXIT_NORTH_UICURSOR,                UICURSOR_FREEFLOWING,                                                                                 CURSOR_EXIT_NORTH,         0               },
	{EXIT_SOUTH_UICURSOR,                UICURSOR_FREEFLOWING,                                                                                 CURSOR_EXIT_SOUTH,         0               },
	{EXIT_EAST_UICURSOR,                 UICURSOR_FREEFLOWING,                                                                                 CURSOR_EXIT_EAST,          0               },
	{EXIT_WEST_UICURSOR,                 UICURSOR_FREEFLOWING,                                                                                 CURSOR_EXIT_WEST,          0               },
	{EXIT_GRID_UICURSOR,                 UICURSOR_FREEFLOWING,                                                                                 CURSOR_EXIT_GRID,          0               },
	{NOEXIT_NORTH_UICURSOR,              UICURSOR_FREEFLOWING,                                                                                 CURSOR_NOEXIT_NORTH,       0               },
	{NOEXIT_SOUTH_UICURSOR,              UICURSOR_FREEFLOWING,                                                                                 CURSOR_NOEXIT_SOUTH,       0               },
	{NOEXIT_EAST_UICURSOR,               UICURSOR_FREEFLOWING,                                                                                 CURSOR_NOEXIT_EAST,        0               },
	{NOEXIT_WEST_UICURSOR,               UICURSOR_FREEFLOWING,                                                                                 CURSOR_NOEXIT_WEST,        0               },
	{NOEXIT_GRID_UICURSOR,               UICURSOR_FREEFLOWING,                                                                                 CURSOR_NOEXIT_GRID,        0               },
	{CONFIRM_EXIT_NORTH_UICURSOR,        UICURSOR_FREEFLOWING,                                                                                 CURSOR_CONEXIT_NORTH,      0               },
	{CONFIRM_EXIT_SOUTH_UICURSOR,        UICURSOR_FREEFLOWING,                                                                                 CURSOR_CONEXIT_SOUTH,      0               },
	{CONFIRM_EXIT_EAST_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_CONEXIT_EAST,       0               },
	{CONFIRM_EXIT_WEST_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_CONEXIT_WEST,       0               },
	{CONFIRM_EXIT_GRID_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_CONEXIT_GRID,       0               },

	{GOOD_WIRECUTTER_UICURSOR,           UICURSOR_FREEFLOWING,                                                                                 CURSOR_GOOD_WIRECUT,       0               },
	{BAD_WIRECUTTER_UICURSOR,            UICURSOR_FREEFLOWING,                                                                                 CURSOR_BAD_WIRECUT,        0               },
	{GOOD_REPAIR_UICURSOR,               UICURSOR_FREEFLOWING,                                                                                 CURSOR_REPAIR,             0               },
	{BAD_REPAIR_UICURSOR,                UICURSOR_FREEFLOWING,                                                                                 CURSOR_REPAIRRED,          0               },
	{GOOD_RELOAD_UICURSOR,               UICURSOR_FREEFLOWING,                                                                                 CURSOR_GOOD_RELOAD,        0               },
	{BAD_RELOAD_UICURSOR,                UICURSOR_FREEFLOWING,                                                                                 CURSOR_BAD_RELOAD,         0               },
	{GOOD_JAR_UICURSOR,                  UICURSOR_FREEFLOWING,                                                                                 CURSOR_JARRED,             0               },
	{BAD_JAR_UICURSOR,                   UICURSOR_FREEFLOWING,                                                                                 CURSOR_JAR,                0               },


	{GOOD_THROW_UICURSOR,                UICURSOR_FREEFLOWING,                                                                                 CURSOR_GOOD_THROW,         0               },
	{BAD_THROW_UICURSOR,                 UICURSOR_FREEFLOWING,                                                                                 CURSOR_BAD_THROW,          0               },
	{RED_THROW_UICURSOR,                 UICURSOR_FREEFLOWING,                                                                                 CURSOR_RED_THROW,          0               },
	{FLASH_THROW_UICURSOR,               UICURSOR_FREEFLOWING,                                                                                 CURSOR_FLASH_THROW,        0               },
	{ACTION_THROWAIM1_UICURSOR,          UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKON1,          0               },
	{ACTION_THROWAIM2_UICURSOR,          UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKON2,          0               },
	{ACTION_THROWAIM3_UICURSOR,          UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKON3,          0               },
	{ACTION_THROWAIM4_UICURSOR,          UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKON4,          0               },
	{ACTION_THROWAIM5_UICURSOR,          UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKON5,          0               },
	{ACTION_THROWAIM6_UICURSOR,          UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKON6,          0               },
	{ACTION_THROWAIM7_UICURSOR,          UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKON7,          0               },
	{ACTION_THROWAIM8_UICURSOR,          UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKON8,          0               },
	{ACTION_THROWAIM8_UICURSOR,          UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKON9,          0               },
	{ACTION_THROWAIMCANT1_UICURSOR,      UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKW1,           0               },
	{ACTION_THROWAIMCANT2_UICURSOR,      UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKW2,           0               },
	{ACTION_THROWAIMCANT3_UICURSOR,      UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKW3,           0               },
	{ACTION_THROWAIMCANT4_UICURSOR,      UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKW4,           0               },
	{ACTION_THROWAIMCANT5_UICURSOR,      UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKW5,           0               },
	{ACTION_THROWAIMFULL_UICURSOR,       UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKWR1,          0               },
	{ACTION_THROWAIMYELLOW1_UICURSOR,    UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKYELLOW1,      0               },
	{ACTION_THROWAIMYELLOW2_UICURSOR,    UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKYELLOW2,      0               },
	{ACTION_THROWAIMYELLOW3_UICURSOR,    UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKYELLOW3,      0               },
	{ACTION_THROWAIMYELLOW4_UICURSOR,    UICURSOR_FREEFLOWING,                                                                                 CURSOR_THROWKYELLOW4,      0               },

	{THROW_ITEM_GOOD_UICURSOR,           UICURSOR_FREEFLOWING,                                                                                 CURSOR_ITEM_GOOD_THROW,    0               },
	{THROW_ITEM_BAD_UICURSOR,            UICURSOR_FREEFLOWING,                                                                                 CURSOR_ITEM_BAD_THROW,     0               },
	{THROW_ITEM_RED_UICURSOR,            UICURSOR_FREEFLOWING,                                                                                 CURSOR_ITEM_RED_THROW,     0               },
	{THROW_ITEM_FLASH_UICURSOR,          UICURSOR_FREEFLOWING,                                                                                 CURSOR_ITEM_FLASH_THROW,   0               },

	{PLACE_BOMB_GREY_UICURSOR,           UICURSOR_FREEFLOWING,                                                                                 CURSOR_BOMB_GRAY,          0               },
	{PLACE_BOMB_RED_UICURSOR,            UICURSOR_FREEFLOWING,                                                                                 CURSOR_BOMB_RED,           0               },
	{PLACE_REMOTE_GREY_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_REMOTE_GRAY,        0               },
	{PLACE_REMOTE_RED_UICURSOR,          UICURSOR_FREEFLOWING,                                                                                 CURSOR_REMOTE_RED,         0               },
	{PLACE_TINCAN_GREY_UICURSOR,         UICURSOR_FREEFLOWING,                                                                                 CURSOR_CAN,                0               },
	{PLACE_TINCAN_RED_UICURSOR,          UICURSOR_FREEFLOWING,                                                                                 CURSOR_CANRED,             0               },

	{ENTER_VEHICLE_UICURSOR,             UICURSOR_FREEFLOWING,                                                                                 CURSOR_ENTERV,             0               },

	{INVALID_ACTION_UICURSOR,            UICURSOR_FREEFLOWING,                                                                                 CURSOR_INVALID_ACTION,     0               },

	{floatING_X_UICURSOR,                UICURSOR_FREEFLOWING,                                                                                 CURSOR_X,                  0               },

	{EXCHANGE_PLACES_UICURSOR,           UICURSOR_FREEFLOWING,                                                                                 CURSOR_EXCHANGE_PLACES,    0               },
	{JUMP_OVER_UICURSOR,                 UICURSOR_FREEFLOWING,                                                                                 CURSOR_JUMP_OVER,          0               },

	{REFUEL_GREY_UICURSOR,               UICURSOR_FREEFLOWING,                                                                                 CURSOR_FUEL,               0               },
	{REFUEL_RED_UICURSOR,                UICURSOR_FREEFLOWING,                                                                                 CURSOR_FUEL_RED,           0               },
};


static UINT32 guiCurUICursor = NO_UICURSOR;
UINT16  gusCurMousePos;
UINT16  gusTargetDropPos;
BOOLEAN gfTargetDropPos = false;


void SetUICursor(UICursorID const uiNewCursor)
{
	guiCurUICursor = uiNewCursor;
}


static void DrawSnappingCursor(void);


void DrawUICursor()
{
	LEVELNODE *pNode;
	UINT16 usTileCursor;

	// OK, WE OVERRIDE HERE CURSOR DRAWING FOR THINGS LIKE
	if ( gpItemPointer != NULL )
	{
		gViewportRegion.ChangeCursor(VIDEO_NO_CURSOR);

		// Check if we are in the viewport region...
		if ( gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA || IsPointerOnTacticalTouchUI())
		{
			DrawItemTileCursor( );
		}
		else
		{
			DrawItemFreeCursor( );
		}
		return;
	}

	const GridNo usMapPos = guiCurrentCursorGridNo;
	if (usMapPos != NOWHERE)
	{
		gusCurMousePos = usMapPos;

		if ( guiCurUICursor == NO_UICURSOR )
		{
			gViewportRegion.ChangeCursor(VIDEO_DEFAULT_TO_NO_CURSOR);
			return;
		}

		if ( gUICursors[ guiCurUICursor ].uiFlags & UICURSOR_SHOWTILE )
		{
			if ( gsInterfaceLevel == I_ROOF_LEVEL )
			{
				pNode = AddTopmostToTail( gusCurMousePos, GetSnapCursorIndex( FIRSTPOINTERS3 ) );
			}
			else
			{
				pNode = AddTopmostToTail( gusCurMousePos, GetSnapCursorIndex( gUICursors[ guiCurUICursor ].usAdditionalData ) );
			}
			pNode->ubShadeLevel=DEFAULT_SHADE_LEVEL;
			pNode->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;

			if ( gsInterfaceLevel == I_ROOF_LEVEL )
			{
				// Put one on the roof as well
				LEVELNODE* const n = AddOnRoofToHead(gusCurMousePos, GetSnapCursorIndex(gUICursors[guiCurUICursor].usAdditionalData));
				n->ubShadeLevel        = DEFAULT_SHADE_LEVEL;
				n->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
			}
		}

		gfTargetDropPos = false;

		if ( gUICursors[ guiCurUICursor ].uiFlags & UICURSOR_FREEFLOWING && !( gUICursors[ guiCurUICursor ].uiFlags & UICURSOR_DONTSHOW2NDLEVEL ) )
		{
			gfTargetDropPos  = true;
			gusTargetDropPos = gusCurMousePos;

			if ( gsInterfaceLevel == I_ROOF_LEVEL )
			{
				// If we are over a target, jump to that....
				if (gUIFullTarget != NULL) gusTargetDropPos = gUIFullTarget->sGridNo;

				// Put tile on the floor
				AddTopmostToTail( gusTargetDropPos, FIRSTPOINTERS14 );
				gpWorldLevelData[ gusTargetDropPos ].pTopmostHead->ubShadeLevel=DEFAULT_SHADE_LEVEL;
				gpWorldLevelData[ gusTargetDropPos ].pTopmostHead->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;

			}
		}

		if ( gUICursors[ guiCurUICursor ].uiFlags & UICURSOR_SHOWTILEAPDEPENDENT )
		{
			const SOLDIERTYPE* const sel = GetSelectedMan();
			if (gfUIDisplayActionPointsInvalid || gsCurrentActionPoints == 0)
			{
				// ATE; Is the current guy in steath mode?
				if (sel != NULL && sel->bStealthMode)
				{
					usTileCursor = FIRSTPOINTERS10;
				}
				else
				{
					usTileCursor = FIRSTPOINTERS6;
				}
			}
			else
			{
				if (sel != NULL && sel->bStealthMode)
				{
					usTileCursor = FIRSTPOINTERS9;
				}
				else
				{
					// Add depending on AP status
					usTileCursor = gUICursors[guiCurUICursor].usAdditionalData;
				}
			}

			if ( gsInterfaceLevel == I_ROOF_LEVEL )
			{
				pNode = AddTopmostToTail( gusCurMousePos, GetSnapCursorIndex( FIRSTPOINTERS14 ) );
			}
			else
			{
				pNode = AddTopmostToTail( gusCurMousePos,  GetSnapCursorIndex( usTileCursor ) );
			}

			pNode->ubShadeLevel=DEFAULT_SHADE_LEVEL;
			pNode->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;

			if ( gsInterfaceLevel == I_ROOF_LEVEL )
			{
				// Put one on the roof as well
				LEVELNODE* const n = AddOnRoofToHead(gusCurMousePos, GetSnapCursorIndex(usTileCursor));
				n->ubShadeLevel        = DEFAULT_SHADE_LEVEL;
				n->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
			}
		}


		// If snapping - remove from main viewport
		if ( gUICursors[ guiCurUICursor ].uiFlags & UICURSOR_SNAPPING )
		{
			// Hide mouse region cursor
			gViewportRegion.ChangeCursor(VIDEO_DEFAULT_TO_NO_CURSOR);

			// Set Snapping Cursor
			DrawSnappingCursor( );
		}


		if ( gUICursors[ guiCurUICursor ].uiFlags & UICURSOR_FREEFLOWING )
		{
			switch( guiCurUICursor )
			{
				case  MOVE_VEHICLE_UICURSOR:

					// Set position for APS
					gfUIDisplayActionPointsCenter = false;
					gUIDisplayActionPointsOffX = 16;
					gUIDisplayActionPointsOffY = 14;
					break;

				case	MOVE_WALK_UICURSOR:
				case	MOVE_RUN_UICURSOR:

					// Set position for APS
					gfUIDisplayActionPointsCenter = false;
					gUIDisplayActionPointsOffX = 16;
					gUIDisplayActionPointsOffY = 14;
					break;

				case	MOVE_SWAT_UICURSOR:

					// Set position for APS
					gfUIDisplayActionPointsCenter = false;
					gUIDisplayActionPointsOffX = 16;
					gUIDisplayActionPointsOffY = 10;
					break;

				case	MOVE_PRONE_UICURSOR:

					// Set position for APS
					gfUIDisplayActionPointsCenter = false;
					gUIDisplayActionPointsOffX = 16;
					gUIDisplayActionPointsOffY = 9;
					break;

			}

			gViewportRegion.ChangeCursor(gUICursors[guiCurUICursor].usFreeCursorName);
		}

		if ( gUICursors[ guiCurUICursor ].uiFlags & UICURSOR_CENTERAPS )
		{
			gfUIDisplayActionPointsCenter = true;
		}
	}
}


static void EraseSnappingCursor(void);


void HideUICursor()
{
	// OK, WE OVERRIDE HERE CURSOR DRAWING FOR THINGS LIKE
	if ( gpItemPointer != NULL )
	{
		// Check if we are in the viewport region...
		if (gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA || IsPointerOnTacticalTouchUI()) return;
	}

	if (guiCurUICursor == NO_UICURSOR) return; //Do nothing here

	if ( gUICursors[ guiCurUICursor ].uiFlags & ( UICURSOR_SHOWTILE | UICURSOR_SHOWTILEAPDEPENDENT )  )
	{
		RemoveAllTopmostsOfTypeRange( gusCurMousePos, FIRSTPOINTERS, FIRSTPOINTERS );
		RemoveAllOnRoofsOfTypeRange( gusCurMousePos, FIRSTPOINTERS, FIRSTPOINTERS );
	}


	if ( gUICursors[ guiCurUICursor ].uiFlags & UICURSOR_FREEFLOWING && !( gUICursors[ guiCurUICursor ].uiFlags & UICURSOR_DONTSHOW2NDLEVEL ) )
	{
		if ( gsInterfaceLevel == I_ROOF_LEVEL )
		{
			RemoveTopmost( gusCurMousePos, FIRSTPOINTERS14 );
			RemoveTopmost( gusCurMousePos, FIRSTPOINTERS9 );

			if ( gfTargetDropPos )
			{
				RemoveTopmost( gusTargetDropPos, FIRSTPOINTERS14 );
				RemoveTopmost( gusTargetDropPos, FIRSTPOINTERS9 );
			}
		}

	}


	// If snapping - remove from main viewport
	if ( ( gUICursors[ guiCurUICursor ].uiFlags & UICURSOR_SNAPPING ) )
	{
		// hide Snapping Cursor
		EraseSnappingCursor( );
	}

	if ( gUICursors[ guiCurUICursor ].uiFlags & UICURSOR_FREEFLOWING )
	{
		// Nothing special here...
	}
}


static void DrawSnappingCursor(void)
{
	LEVELNODE *pNewUIElem;
	static BOOLEAN fShowAP = true;

	// If we are in draw item mode, do nothing here but call the fuctiuon
	switch ( guiCurUICursor )
	{
		case NO_UICURSOR:
			break;

		case NORMAL_SNAPUICURSOR:
		{
			LEVELNODE* const n = AddTopmostToHead(gusCurMousePos, FIRSTPOINTERS1);
			n->ubShadeLevel        = DEFAULT_SHADE_LEVEL;
			n->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
			break;
		}

		case ALL_MOVE_RUN_UICURSOR:
		case CONFIRM_MOVE_RUN_UICURSOR:
			if ( gsInterfaceLevel > 0 )
			{
				pNewUIElem = AddUIElem(gusCurMousePos, GOODRUN1, 0, -WALL_HEIGHT - 8);
			}
			else
			{
				pNewUIElem = AddUIElem(gusCurMousePos, GOODRUN1, 0, 0);
			}
			pNewUIElem->ubShadeLevel=DEFAULT_SHADE_LEVEL;
			pNewUIElem->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
			break;

		case ALL_MOVE_WALK_UICURSOR:
		case CONFIRM_MOVE_WALK_UICURSOR:
			if ( gsInterfaceLevel > 0 )
			{
				pNewUIElem = AddUIElem(gusCurMousePos, GOODWALK1, 0, -WALL_HEIGHT - 8);
			}
			else
			{
				pNewUIElem = AddUIElem(gusCurMousePos, GOODWALK1, 0, 0);
			}
			pNewUIElem->ubShadeLevel=DEFAULT_SHADE_LEVEL;
			pNewUIElem->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
			break;

		case ALL_MOVE_SWAT_UICURSOR:
		case CONFIRM_MOVE_SWAT_UICURSOR:
			if ( gsInterfaceLevel > 0 )
			{
				pNewUIElem = AddUIElem(gusCurMousePos, GOODSWAT1, 0, -WALL_HEIGHT - 8);
			}
			else
			{
				pNewUIElem = AddUIElem(gusCurMousePos, GOODSWAT1, 0, 0);
			}
			pNewUIElem->ubShadeLevel=DEFAULT_SHADE_LEVEL;
			pNewUIElem->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
			break;

		case ALL_MOVE_PRONE_UICURSOR:
		case CONFIRM_MOVE_PRONE_UICURSOR:
			if ( gsInterfaceLevel > 0 )
			{
				pNewUIElem = AddUIElem(gusCurMousePos, GOODPRONE1, 0, -WALL_HEIGHT - 8 - 6);
			}
			else
			{
				pNewUIElem = AddUIElem(gusCurMousePos, GOODPRONE1, 0, -6);
			}
			pNewUIElem->ubShadeLevel=DEFAULT_SHADE_LEVEL;
			pNewUIElem->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
			break;

		case ALL_MOVE_VEHICLE_UICURSOR:
		case CONFIRM_MOVE_VEHICLE_UICURSOR:
			if ( gsInterfaceLevel > 0 )
			{
				pNewUIElem = AddUIElem(gusCurMousePos, VEHICLEMOVE1, 0, -WALL_HEIGHT - 8);
			}
			else
			{
				pNewUIElem = AddUIElem(gusCurMousePos, VEHICLEMOVE1, 0, 0);
			}
			pNewUIElem->ubShadeLevel=DEFAULT_SHADE_LEVEL;
			pNewUIElem->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
			break;

		case MOVE_REALTIME_UICURSOR:
			break;

		case CANNOT_MOVE_UICURSOR:

			if ( gsInterfaceLevel > 0 )
			{
				pNewUIElem = AddUIElem(gusCurMousePos, BADMARKER1, 0, -WALL_HEIGHT - 8);
				pNewUIElem->ubShadeLevel=DEFAULT_SHADE_LEVEL;
				pNewUIElem->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;

				if ( gGameSettings.fOptions[ TOPTION_3D_CURSOR ] )
				{
					LEVELNODE* const n = AddTopmostToHead(gusCurMousePos, FIRSTPOINTERS13);
					n->ubShadeLevel        = DEFAULT_SHADE_LEVEL;
					n->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
				}

				LEVELNODE* const n = AddOnRoofToHead(gusCurMousePos, FIRSTPOINTERS14);
				n->ubShadeLevel        = DEFAULT_SHADE_LEVEL;
				n->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
			}
			else
			{
				LEVELNODE* const n = AddTopmostToHead(gusCurMousePos, BADMARKER1);
				n->ubShadeLevel        = DEFAULT_SHADE_LEVEL;
				n->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;

				if ( gGameSettings.fOptions[ TOPTION_3D_CURSOR ] )
				{
					LEVELNODE* const n = AddTopmostToHead(gusCurMousePos, FIRSTPOINTERS13);
					n->ubShadeLevel        = DEFAULT_SHADE_LEVEL;
					n->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
				}
			}
			break;

	}

	// Add action points
	if ( gfUIDisplayActionPoints )
	{

		if ( gfUIDisplayActionPointsInvalid )
		{
			if ( COUNTERDONE( CURSORFLASH ) )
			{
				fShowAP = !fShowAP;
			}
		}
		else
		{
			fShowAP = true;
		}

		if ( gsInterfaceLevel > 0 )
		{
			pNewUIElem = AddUIElem(gusCurMousePos, DISPLAY_AP_INDEX, SNAPCURSOR_AP_X_STARTVAL, SNAPCURSOR_AP_Y_STARTVAL - WALL_HEIGHT - 10);
		}
		else
		{
			pNewUIElem = AddUIElem(gusCurMousePos, DISPLAY_AP_INDEX, SNAPCURSOR_AP_X_STARTVAL, SNAPCURSOR_AP_Y_STARTVAL);
		}
		pNewUIElem->uiFlags |= LEVELNODE_DISPLAY_AP;
		pNewUIElem->uiAPCost = gsCurrentActionPoints;
		pNewUIElem->ubShadeLevel=DEFAULT_SHADE_LEVEL;
		pNewUIElem->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;

		if ( !fShowAP )
		{
			gfUIDisplayActionPointsBlack = true;
		}
	}
}


static void EraseSnappingCursor(void)
{
	RemoveAllTopmostsOfTypeRange( gusCurMousePos, MOCKFLOOR, MOCKFLOOR );
	RemoveAllTopmostsOfTypeRange( gusCurMousePos, FIRSTPOINTERS, LASTPOINTERS );
	RemoveAllObjectsOfTypeRange( gusCurMousePos, FIRSTPOINTERS, LASTPOINTERS );
	RemoveAllOnRoofsOfTypeRange( gusCurMousePos, FIRSTPOINTERS, LASTPOINTERS );
	RemoveAllOnRoofsOfTypeRange( gusCurMousePos, MOCKFLOOR, MOCKFLOOR );
}


UINT16 GetSnapCursorIndex( UINT16 usAdditionalData )
{
	// OK, this function will get the 'true' index for drawing the cursor....
	if ( gGameSettings.fOptions[ TOPTION_3D_CURSOR ] )
	{
		switch( usAdditionalData )
		{
			case FIRSTPOINTERS2:  return FIRSTPOINTERS13;
			case FIRSTPOINTERS3:  return FIRSTPOINTERS14;
			case FIRSTPOINTERS4:  return FIRSTPOINTERS15;
			case FIRSTPOINTERS5:  return FIRSTPOINTERS16;
			case FIRSTPOINTERS6:  return FIRSTPOINTERS17;
			case FIRSTPOINTERS7:  return FIRSTPOINTERS18;
			case FIRSTPOINTERS9:  return FIRSTPOINTERS19;
			case FIRSTPOINTERS10: return FIRSTPOINTERS20;
			default:              return usAdditionalData;
		}
	}
	else
	{
		return( usAdditionalData );
	}
}
