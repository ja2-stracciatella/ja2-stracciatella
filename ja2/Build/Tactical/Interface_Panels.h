#ifndef __INTERFACE_PANELS
#define __INTERFACE_PANELS


typedef enum
{
	STANCEUP_BUTTON = 0,
	UPDOWN_BUTTON,
	CLIMB_BUTTON,
	STANCEDOWN_BUTTON,
	HANDCURSOR_BUTTON,
	PREVMERC_BUTTON,
	NEXTMERC_BUTTON,
	OPTIONS_BUTTON,
	BURSTMODE_BUTTON,
	LOOK_BUTTON,
	TALK_BUTTON,
	MUTE_BUTTON,
	SM_DONE_BUTTON,
	SM_MAP_SCREEN_BUTTON,
	NUM_SM_BUTTONS
};


typedef enum
{
	TEAM_DONE_BUTTON = 0,
	TEAM_MAP_SCREEN_BUTTON,
	CHANGE_SQUAD_BUTTON,
	NUM_TEAM_BUTTONS
};


#define NEW_ITEM_CYCLE_COUNT	19
#define NEW_ITEM_CYCLES				4
#define	NUM_TEAM_SLOTS							6


#define	PASSING_ITEM_DISTANCE_OKLIFE			3
#define	PASSING_ITEM_DISTANCE_NOTOKLIFE		2

#define	SHOW_LOCATOR_NORMAL									1
#define SHOW_LOCATOR_FAST										2

BOOLEAN CreateSMPanelButtons( );
void    RemoveSMPanelButtons( );
BOOLEAN InitializeSMPanel(  );
BOOLEAN ShutdownSMPanel( );
void RenderSMPanel( BOOLEAN *pfDirty );
void EnableSMPanelButtons( BOOLEAN fEnable, BOOLEAN fFromItemPickup );


BOOLEAN CreateTEAMPanelButtons( );
void    RemoveTEAMPanelButtons( );
BOOLEAN InitializeTEAMPanel(  );
BOOLEAN ShutdownTEAMPanel( );
void RenderTEAMPanel( BOOLEAN fDirty );
void UpdateTEAMPanel( );


void SetSMPanelCurrentMerc( UINT8 ubNewID );
void SetTEAMPanelCurrentMerc( UINT8 ubNewID );
UINT16 GetSMPanelCurrentMerc(  );
void UpdateSMPanel( );

BOOLEAN InitTEAMSlots( );
void AddPlayerToInterfaceTeamSlot( UINT8 ubID );
BOOLEAN RemovePlayerFromInterfaceTeamSlot( UINT8 ubID );
BOOLEAN GetPlayerIDFromInterfaceTeamSlot( UINT8 ubPanelSlot, UINT8 *pubID );
void RemoveAllPlayersFromSlot( );
	BOOLEAN PlayerExistsInSlot( UINT8 ubID );
BOOLEAN RemovePlayerFromTeamSlotGivenMercID( UINT8 ubMercID );
void CheckForAndAddMercToTeamPanel( SOLDIERTYPE *pSoldier );

void DisableTacticalTeamPanelButtons( BOOLEAN fDisable );
void			RenderTownIDString( );
void KeyRingSlotInvClickCallback( MOUSE_REGION * pRegion, INT32 iReason );


// ATE TO BE MOVED TO INTERFACE_ITEMS.C
INT8						gbNewItem[ NUM_INV_SLOTS ];
INT8						gbNewItemCycle[ NUM_INV_SLOTS ];
UINT8						gubNewItemMerc;

void ShowRadioLocator( UINT8 ubID, UINT8 ubLocatorSpeed );
void EndRadioLocator( UINT8 ubID );


MOUSE_REGION		gSMPanelRegion;


UINT32					guiSecItemHiddenVO;

extern BOOLEAN	gfDisableTacticalPanelButtons;


typedef struct
{
	UINT8	ubID;
	BOOLEAN	fOccupied;

} TEAM_PANEL_SLOTS_TYPE;

TEAM_PANEL_SLOTS_TYPE		gTeamPanel[ NUM_TEAM_SLOTS ];



//Used when the shop keeper interface is active
void DisableSMPpanelButtonsWhenInShopKeeperInterface( BOOLEAN fDontDrawButtons );
//void DisableSMPpanelButtonsWhenInShopKeeperInterface( );

//
void ReEvaluateDisabledINVPanelButtons( );
void CheckForReEvaluateDisabledINVPanelButtons( );

void CheckForDisabledForGiveItem( );
void ReevaluateItemHatches( SOLDIERTYPE *pSoldier, BOOLEAN fEnable );

void HandlePanelFaceAnimations( SOLDIERTYPE *pSoldier );

void GoToMapScreenFromTactical( void );

void HandleTacticalEffectsOfEquipmentChange( SOLDIERTYPE *pSoldier, UINT32 uiInvPos, UINT16 usOldItem, UINT16 usNewItem );

void FinishAnySkullPanelAnimations( );

UINT8 FindNextMercInTeamPanel( SOLDIERTYPE *pSoldier, BOOLEAN fGoodForLessOKLife, BOOLEAN fOnlyRegularMercs );

#endif
