#ifndef __EDITORMERCS_H
#define __EDITORMERCS_H

#include "Overhead_Types.h"
#include "Scheduling.h"

#include <string_theory/string>


//Merc editing modes.  These are used to determine which buttons to show and hide.
enum
{
	MERC_NOMODE,					//used for shutting down mercs tab, to extract any changed information

	MERC_GETITEMMODE,			//when selecting a specific piece of inventory from inventorymode

	MERC_TEAMMODE,				//selecting a team with no merc selected.
	MERC_BASICMODE,				//basic placement mode

	MERC_GENERALMODE,			//detailed placement mode for general information and NPC info
	MERC_ATTRIBUTEMODE,		//detailed placement mode for specifying attributes
	MERC_INVENTORYMODE,		//detailed placement mode for specifying inventory
	MERC_APPEARANCEMODE,	//detailed placement mode for specifying appearance
	MERC_PROFILEMODE,			//specifying a valid profile index will generate the merc automatically.
	MERC_SCHEDULEMODE,		//specifying a schedule for that particular individual
};

extern UINT8 gubCurrMercMode;

#define EDIT_NUM_COLORS				4
#define EDIT_COLOR_HEAD				0
#define EDIT_COLOR_PANTS			1
#define EDIT_COLOR_SKIN				2
#define EDIT_COLOR_VEST				3
#define EDIT_MERC_NONE				0
#define EDIT_MERC_NEXT_COLOR	13

extern SOLDIERTYPE* g_selected_merc;
extern INT16 gsSelectedMercGridNo;

enum _ForUseWithIndicateSelectedMerc
{
	SELECT_NEXT_CREATURE	= -7,
	SELECT_NEXT_REBEL			=	-6,
	SELECT_NEXT_CIV				= -5,
	SELECT_NEXT_ENEMY			= -4,
	SELECT_NEXT_TEAMMATE	= -3,
	SELECT_NEXT_MERC			= -2,
	SELECT_NO_MERC				= -1
	// >= 0 select merc with matching ID
};

extern const ST::string gszScheduleActions[NUM_SCHEDULE_ACTIONS];

extern const ST::string gszCivGroupNames[NUM_CIV_GROUPS];

void IndicateSelectedMerc( INT16 sID );

void GameInitEditorMercsInfo(void);
void GameShutdownEditorMercsInfo(void);
void EntryInitEditorMercsInfo(void);
void UpdateMercsInfo(void);

void ProcessMercEditing(void);
void AddMercToWorld( INT32 iMapIndex );
void HandleRightClickOnMerc( INT32 iMapIndex );
void SetMercEditingMode( UINT8 ubNewMode );

void ResetAllMercPositions(void);

void EraseMercWaypoint(void);
void AddMercWaypoint( UINT32 iMapIndex );

void SetEnemyColorCode( UINT8 ubSoldierClass );

void SpecifyEntryPoint( UINT32 iMapIndex );

//Modify stats of current soldiers
void SetMercOrders( INT8 bOrders );
void SetMercAttitude( INT8 bAttitude );
void SetMercDirection( INT8 bDirection );
void SetMercRelativeEquipment( INT8 bLevel );
void SetMercRelativeAttributes( INT8 bLevel );

void DeleteSelectedMerc(void);

void ExtractCurrentMercModeInfo( BOOLEAN fKillTextInputMode );

void HandleMercInventoryPanel( INT16 sX, INT16 sY, INT8 bEvent );

extern UINT16 gusMercsNewItemIndex;
extern BOOLEAN gfRenderMercInfo;

void ChangeCivGroup( UINT8 ubNewCivGroup );

#define MERCINV_LGSLOT_WIDTH		48
#define MERCINV_SMSLOT_WIDTH		24
#define MERCINV_SLOT_HEIGHT			18

extern BOOLEAN					gfRoofPlacement;

extern void SetEnemyDroppableStatus( UINT32 uiSlot, BOOLEAN fDroppable );

void RenderMercStrings(void);

extern BOOLEAN gfShowPlayers;
extern BOOLEAN gfShowEnemies;
extern BOOLEAN gfShowCreatures;
extern BOOLEAN gfShowRebels;
extern BOOLEAN gfShowCivilians;
void SetMercTeamVisibility( INT8 bTeam, BOOLEAN fVisible );

extern UINT8 gubCurrentScheduleActionIndex;
extern BOOLEAN gfSingleAction;
extern BOOLEAN gfUseScheduleData2;

void UpdateScheduleAction( UINT8 ubNewAction );
void FindScheduleGridNo( UINT8 ubScheduleData );
void ClearCurrentSchedule(void);
void CancelCurrentScheduleAction(void);
void RegisterCurrentScheduleAction( INT32 iMapIndex );
void StartScheduleAction(void);

void InitDetailedPlacementForMerc(void);
void KillDetailedPlacementForMerc(void);

void CopyMercPlacement(INT32 iMapIndex);
void PasteMercPlacement(INT32 iMapIndex);

void ExtractAndUpdateMercSchedule(void);

void DeleteSelectedMercsItem(void);

#endif
