#include "Font.h"
#include "Local.h"
#include "Gameloop.h"
#include "HImage.h"
#include "VObject.h"
#include "SysUtil.h"
#include "Overhead.h"
#include "MouseSystem.h"
#include "Button_System.h"
#include "Interface.h"
#include "VSurface.h"
#include "WCheck.h"
#include "Input.h"
#include "Handle_UI.h"
#include "RenderWorld.h"
#include "Sys_Globals.h"
#include "Cursors.h"
#include "Radar_Screen.h"
#include "Font_Control.h"
#include "Render_Dirty.h"
#include "Utilities.h"
#include "Sound_Control.h"
#include "Interface_Panels.h"
#include "Animation_Control.h"
#include "Soldier_Control.h"
#include "PathAI.h"
#include "Weapons.h"
#include "Lighting.h"
#include "Faces.h"
#include "MapScreen.h"
#include "Message.h"
#include "Text.h"
#include "Interface_Items.h"
#include "Interface_Control.h"
#include "Interface_Utils.h"
#include "Game_Clock.h"
#include "Soldier_Macros.h"
#include "StrategicMap.h"
#include "Soldier_Functions.h"
#include "GameScreen.h"
#include "Assignments.h"
#include "Points.h"
#include "Squads.h"
#include "Strategic.h"
#include "Map_Screen_Interface_Map.h"
#include "Overhead_Map.h"
#include "Map_Screen_Interface.h"
#include "Options_Screen.h"
#include "ShopKeeper_Interface.h"
#include "English.h"
#include "Keys.h"
#include "Soldier_Add.h"
#include "Vehicles.h"
#include "GameSettings.h"
#include "Dialogue_Control.h"
#include "Items.h"
#include "Drugs_And_Alcohol.h"
#include "LOS.h"
#include "OppList.h"
#include "VObject_Blitters.h"
#include "Finances.h"
#include "LaptopSave.h"
#include "Cursor_Control.h"
#include "MessageBoxScreen.h"
#include "WordWrap.h"
#include "Boxing.h"
#include "Video.h"
#include "Debug.h"
#include "JAScreens.h"
#include "ScreenIDs.h"


// DEFINES FOR VARIOUS PANELS
#define	SM_ITEMDESC_START_X					214
#define	SM_ITEMDESC_START_Y					1 + INV_INTERFACE_START_Y
#define	SM_ITEMDESC_HEIGHT					128
#define	SM_ITEMDESC_WIDTH					358

// SINGLE MERC SCREEN

#define SM_SELMERC_AP_X       67
#define SM_SELMERC_AP_Y      393
#define SM_SELMERC_AP_HEIGHT  10
#define SM_SELMERC_AP_WIDTH   17

#define SM_SELMERC_HEALTH_X					69
#define SM_SELMERC_HEALTH_Y					387

#define SM_SELMERCNAME_X       11
#define SM_SELMERCNAME_Y      393
#define SM_SELMERCNAME_WIDTH   53
#define SM_SELMERCNAME_HEIGHT  10

#define	SM_SELMERC_FACE_X						13
#define	SM_SELMERC_FACE_Y						346
#define	SM_SELMERC_FACE_HEIGHT			42
#define	SM_SELMERC_FACE_WIDTH				48

#define	SM_SELMERC_PLATE_X						4
#define	SM_SELMERC_PLATE_Y						342
#define	SM_SELMERC_PLATE_HEIGHT				65
#define	SM_SELMERC_PLATE_WIDTH				83


#define SM_BODYINV_X									244
#define SM_BODYINV_Y									346


#define STATS_TITLE_FONT_COLOR				6
#define STATS_TEXT_FONT_COLOR					5


#define	SM_TALKB_X									155
#define	SM_TALKB_Y									448
#define	SM_MUTEB_X									91
#define	SM_MUTEB_Y									448
#define	SM_STANCEUPB_X							187
#define SM_STANCEUPB_Y							380
#define	SM_UPDOWNB_X								91
#define SM_UPDOWNB_Y								413
#define	SM_CLIMBB_X									187
#define SM_CLIMBB_Y									348
#define	SM_STANCEDOWNB_X						187
#define SM_STANCEDOWNB_Y						448
#define	SM_HANDCURSORB_X						123
#define SM_HANDCURSORB_Y						413
#define	SM_PREVMERCB_X							9
#define SM_PREVMERCB_Y							410
#define	SM_NEXTMERCB_X							51
#define SM_NEXTMERCB_Y							410
#define	SM_OPTIONSB_X								9
#define SM_OPTIONSB_Y								445
#define	SM_BURSTMODEB_X							155
#define SM_BURSTMODEB_Y							413
#define	SM_LOOKB_X									123
#define SM_LOOKB_Y									448
#define	SM_STEALTHMODE_X						187
#define	SM_STEALTHMODE_Y						413
#define SM_DONE_X										543
#define SM_DONE_Y										344
#define SM_MAPSCREEN_X							589
#define SM_MAPSCREEN_Y							344


#define	SM_POSITIONB_X							106 + INTERFACE_START_X
#define SM_POSITIONB_Y							34 + INV_INTERFACE_START_Y
#define	SM_POSITIONB_WIDTH					19
#define SM_POSITIONB_HEIGHT					24


#define	SM_PERCENT_WIDTH						20
#define	SM_PERCENT_HEIGHT						10
#define	SM_ARMOR_X									347
#define	SM_ARMOR_Y									419
#define	SM_ARMOR_LABEL_X						363
#define	SM_ARMOR_LABEL_Y						409
#define	SM_ARMOR_PERCENT_X					368
#define	SM_ARMOR_PERCENT_Y					419

#define	SM_WEIGHT_LABEL_X						430
#define	SM_WEIGHT_LABEL_Y						447
#define	SM_WEIGHT_PERCENT_X					449
#define	SM_WEIGHT_PERCENT_Y					446
#define	SM_WEIGHT_X									428
#define	SM_WEIGHT_Y									446

#define	SM_CAMMO_LABEL_X						430
#define	SM_CAMMO_LABEL_Y						462
#define	SM_CAMMO_PERCENT_X					449
#define	SM_CAMMO_PERCENT_Y					461
#define	SM_CAMMO_X									428
#define	SM_CAMMO_Y									461


#define SM_STATS_WIDTH							30
#define SM_STATS_HEIGHT							8
#define	SM_AGI_X										99
#define	SM_AGI_Y										347
#define	SM_DEX_X										99
#define	SM_DEX_Y										357
#define	SM_STR_X										99
#define	SM_STR_Y										367
#define	SM_CHAR_X										99
#define	SM_CHAR_Y										377
#define	SM_WIS_X										99
#define	SM_WIS_Y										387
#define	SM_EXPLVL_X									148
#define	SM_EXPLVL_Y									347
#define	SM_MRKM_X										148
#define	SM_MRKM_Y										357
#define	SM_EXPL_X										148
#define	SM_EXPL_Y										367
#define	SM_MECH_X										148
#define	SM_MECH_Y										377
#define	SM_MED_X										148
#define	SM_MED_Y										387

#define MONEY_X											460
#define MONEY_Y											445
#define MONEY_WIDTH									30
#define MONEY_HEIGHT								22


#define TM_FACE_WIDTH								48
#define TM_FACE_HEIGHT							43

#define	TM_APPANEL_HEIGHT						56
#define	TM_APPANEL_WIDTH						16

#define TM_ENDTURN_X								507
#define TM_ENDTURN_Y								( 9 + INTERFACE_START_Y )
#define TM_ROSTERMODE_X							507
#define TM_ROSTERMODE_Y							( 45 + INTERFACE_START_Y )
#define TM_DISK_X										507
#define TM_DISK_Y										( 81 + INTERFACE_START_Y )

#define TM_NAME_WIDTH								60
#define TM_NAME_HEIGHT							9
#define	TM_LIFEBAR_HEIGHT						42
#define	TM_FACEHIGHTL_WIDTH					84
#define	TM_FACEHIGHTL_HEIGHT				114
#define TM_AP_HEIGHT								10
#define TM_AP_WIDTH									15

#define	TM_INV_WIDTH								58
#define	TM_INV_HEIGHT								23
#define	TM_INV_HAND1STARTX					8
#define	TM_INV_HAND1STARTY					( 67 + INTERFACE_START_Y )
#define	TM_INV_HAND2STARTX					8
#define	TM_INV_HAND2STARTY					( 93 + INTERFACE_START_Y )
#define	TM_INV_HAND_SEP							83
#define	TM_INV_HAND_SEPY						24

#define	TM_BARS_REGION_HEIGHT				47
#define	TM_BARS_REGION_WIDTH				26

#define	INDICATOR_BOX_WIDTH					12
#define	INDICATOR_BOX_HEIGHT				10


enum
{
	STANCEUP_IMAGES = 0,
	UPDOWN_IMAGES,
	CLIMB_IMAGES,
	STANCEDOWN_IMAGES,
	HANDCURSOR_IMAGES,
	PREVMERC_IMAGES,
	NEXTMERC_IMAGES,
	OPTIONS_IMAGES,
//BURSTMODE_IMAGES,
	LOOK_IMAGES,
	TALK_IMAGES,
	MUTE_IMAGES,
	STANCE_IMAGES,
	DONE_IMAGES,
	MAPSCREEN_IMAGES,
	NUM_SM_BUTTON_IMAGES
};



enum
{
	ENDTURN_IMAGES = 0,
	ROSTERMODE_IMAGES,
	DISK_IMAGES,
	NUM_TEAM_BUTTON_IMAGES
};


static INT32 iSMPanelImages[NUM_SM_BUTTON_IMAGES];
static INT32 iBurstButtonImages[NUM_WEAPON_MODES];
static INT32 iTEAMPanelImages[NUM_TEAM_BUTTON_IMAGES];

static INT32 giSMStealthImages = -1;
INT32										giSMStealthButton = -1;

BOOLEAN									gfSwitchPanel = FALSE;
UINT8										gbNewPanel		= SM_PANEL;
UINT8										gubNewPanelParam = 0;

BOOLEAN									gfUIStanceDifferent = FALSE;
static BOOLEAN gfAllDisabled = FALSE;

BOOLEAN									gfSMDisableForItems = FALSE;

BOOLEAN									gfDisableTacticalPanelButtons = FALSE;

BOOLEAN									gfAddingMoneyToMercFromPlayersAccount=FALSE;

BOOLEAN	gfCheckForMouseOverItem = FALSE;
UINT32	guiMouseOverItemTime		= 0;
INT8		gbCheckForMouseOverItemPos = 0;
UINT8		gubSelectSMPanelToMerc   = NOBODY;
static BOOLEAN gfReEvaluateDisabledINVPanelButtons = FALSE;

UINT32 guiBrownBackgroundForTeamPanel;

extern BOOLEAN							gfInKeyRingPopup;
extern UINT32	 guiVEHINV;
extern BOOLEAN	gfBeginEndTurn;
extern	BOOLEAN InternalInitItemDescriptionBox( OBJECTTYPE *pObject, INT16 sX, INT16 sY, UINT8 ubStatusIndex, SOLDIERTYPE *pSoldier );
extern	BOOLEAN	gfInItemPickupMenu;
extern void HandleAnyMercInSquadHasCompatibleStuff( UINT8 ubSquad, OBJECTTYPE *pObject, BOOLEAN fReset );
extern BOOLEAN InternalHandleCompatibleAmmoUI( SOLDIERTYPE *pSoldier, OBJECTTYPE *pTestObject, BOOLEAN fOn  );
extern void SetNewItem( SOLDIERTYPE *pSoldier, UINT8 ubInvPos, BOOLEAN fNewItem );

extern void CleanUpStack( OBJECTTYPE * pObj, OBJECTTYPE * pCursorObj );

void HandlePlayerTeamMemberDeathAfterSkullAnimation( SOLDIERTYPE *pSoldier );


UINT8 gubHandPos;
UINT16 gusOldItemIndex;
UINT16 gusNewItemIndex;
BOOLEAN gfDeductPoints;


// ARRAY FOR INV PANEL INTERFACE ITEM POSITIONS
INV_REGION_DESC gSMInvPocketXY[] =
{
	344,	347,				// HELMETPOS
	344,	376,				// VESTPOS
	344,	436,				// LEGPOS,
	226,	347,				// HEAD1POS
	226,	371,				// HEAD2POS
	226,	424,				// HANDPOS,
	226,	448,				// SECONDHANDPOS
	468,	346,				// BIGPOCK1
	468,	370,				// BIGPOCK2
	468,	394,				// BIGPOCK3
	468,	418,				// BIGPOCK4
	396,	346,				// SMALLPOCK1
	396,	370,				// SMALLPOCK2
	396,	394,				// SMALLPOCK3
	396,	418,				// SMALLPOCK4
	432,	346,				// SMALLPOCK5
	432,	370,				// SMALLPOCK6
	432,	394,				// SMALLPOCK7
	432,	418					// SMALLPOCK8
};

INV_REGION_DESC gSMCamoXY =
{
	SM_BODYINV_X, SM_BODYINV_Y		// X, Y Location of cammo region
};


INT16 sTEAMAPPanelXY[] =
{
	69,	 ( 6 + INTERFACE_START_Y ),
	151, ( 6 + INTERFACE_START_Y ),
	234, ( 6 + INTERFACE_START_Y ),
	317, ( 6 + INTERFACE_START_Y ),
	401, ( 6 + INTERFACE_START_Y ),
	484, ( 6 + INTERFACE_START_Y )
};


INT16					sTEAMFacesXY[] =
{
	13,	( 366 ),
	97, ( 366 ),
	180, ( 366 ),
	263, ( 366 ),
	346, ( 366 ),
	429, ( 366 )
};

INT16					sTEAMNamesXY[] =
{
	7,	( 415 ),
	90, ( 415 ),
	173, ( 415 ),
	256, ( 415 ),
	339, ( 415 ),
	422, ( 415 )
};

INT16					sTEAMFaceHighlXY[] =
{
	4,	( 2 + INTERFACE_START_Y ),
	87, ( 2 + INTERFACE_START_Y ),
	170, ( 2 + INTERFACE_START_Y ),
	253, ( 2 + INTERFACE_START_Y ),
	336, ( 2 + INTERFACE_START_Y ),
	419, ( 2 + INTERFACE_START_Y )
};


INT16					sTEAMApXY[] =
{
	70,	( 413 ),
	153, ( 413 ),
	235, ( 413 ),
	319, ( 413 ),
	402, ( 413 ),
	485, ( 413 )
};


INT16					sTEAMBarsXY[] =
{
	61,	 ( 363 ),
	145, ( 363 ),
	228, ( 363 ),
	311, ( 363 ),
	394, ( 363 ),
	477, ( 362 )
};



INT16					sTEAMHandInvXY[] =
{
	TM_INV_HAND1STARTX + ( 0 * TM_INV_HAND_SEP ),											TM_INV_HAND1STARTY,
	TM_INV_HAND1STARTX + ( 1 * TM_INV_HAND_SEP ),											TM_INV_HAND1STARTY,
	TM_INV_HAND1STARTX + ( 2 * TM_INV_HAND_SEP ),											TM_INV_HAND1STARTY,
	TM_INV_HAND1STARTX + ( 3 * TM_INV_HAND_SEP ),											TM_INV_HAND1STARTY,
	TM_INV_HAND1STARTX + ( 4 * TM_INV_HAND_SEP ),											TM_INV_HAND1STARTY,
	TM_INV_HAND1STARTX + ( 5 * TM_INV_HAND_SEP ),											TM_INV_HAND1STARTY
};



INT32					iSMPanelButtons[ NUM_SM_BUTTONS ];
INT32					iTEAMPanelButtons[ NUM_TEAM_BUTTONS ];

// Video Surface for Single Merc Panel
UINT32					guiSMPanel;
UINT32					guiSMObjects;
UINT32					guiSMObjects2;
UINT32					guiSecItemHiddenVO;

UINT32					guiTEAMPanel;
UINT32					guiTEAMObjects;

// Globals for various mouse regions
MOUSE_REGION		gSM_SELMERCPanelRegion;
MOUSE_REGION		gSM_SELMERCBarsRegion;
MOUSE_REGION		gSM_SELMERCMoneyRegion;
MOUSE_REGION		gSM_SELMERCEnemyIndicatorRegion;
MOUSE_REGION		gTEAM_PanelRegion;
MOUSE_REGION		gTEAM_FaceRegions[ 6 ];
MOUSE_REGION		gTEAM_BarsRegions[ 6 ];
MOUSE_REGION		gTEAM_LeftBarsRegions[ 6 ];
MOUSE_REGION		gTEAM_FirstHandInv[ 6 ];
MOUSE_REGION		gTEAM_SecondHandInv[ 6 ];
MOUSE_REGION		gTEAM_EnemyIndicator[ 6 ];

BOOLEAN		gfTEAM_HandInvDispText[ 6 ][ NUM_INV_SLOTS ];
BOOLEAN		gfSM_HandInvDispText[ NUM_INV_SLOTS ];


// Globals - for one - the current merc here
UINT16					gusSMCurrentMerc = 0;
SOLDIERTYPE			*gpSMCurrentMerc = NULL;
extern	SOLDIERTYPE *gpItemPopupSoldier;
INT8						gbSMCurStanceObj;
extern	INT8		gbCompatibleApplyItem;



INT8		gbStanceButPos[2][3][3] =
{
	// NON-STEALTH
	16,		14,		15,
	10,		8,		9,
	22,		20,		21,

	// STEALTH MODE
	13,		11,		12,
	7,		5,		6,
	19,		17,		18
};


// keyring stuff
void KeyRingItemPanelButtonCallback( MOUSE_REGION * pRegion, INT32 iReason );


extern void UpdateItemHatches();


// Wraps up check for AP-s get from a different soldier for in a vehicle...
static INT8 GetUIApsToDisplay(SOLDIERTYPE* pSoldier)
{
	SOLDIERTYPE *pVehicle;

	if ( pSoldier->uiStatusFlags & SOLDIER_DRIVER )
	{
    pVehicle = GetSoldierStructureForVehicle( pSoldier->iVehicleId );

		if ( pVehicle != NULL )
		{
			return( pVehicle->bActionPoints );
		}
		else
		{
			return( 0 );
		}
	}
	else
	{
		return ( pSoldier->bActionPoints );
	}
}

void CheckForDisabledForGiveItem( )
{
	INT16			sDist;
	INT16			sDistVisible;
	INT16			sDestGridNo;
	INT8			bDestLevel;
	INT32			cnt;
	SOLDIERTYPE	*pSoldier;
	UINT8			ubSrcSoldier;


	Assert( gpSMCurrentMerc != NULL);

	if ( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE )
	{
		gfSMDisableForItems = !CanMercInteractWithSelectedShopkeeper( gpSMCurrentMerc );
		return;
	}


	// Default to true
	gfSMDisableForItems = TRUE;

	// ATE: Is the current merc unconscious.....
	if ( gpSMCurrentMerc->bLife < OKLIFE && gpItemPointer != NULL )
	{
		// Go through each merc and see if there is one closeby....
		cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;
		for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pSoldier++)
		{
			if ( pSoldier->bActive && pSoldier->bLife >= OKLIFE && !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) && !AM_A_ROBOT( pSoldier ) && pSoldier->bInSector && IsMercOnCurrentSquad( pSoldier ) )
			{
				sDist = PythSpacesAway( gpSMCurrentMerc->sGridNo, pSoldier->sGridNo );

				sDistVisible = DistanceVisible( pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, gpSMCurrentMerc->sGridNo, gpSMCurrentMerc->bLevel );

				// Check LOS....
				if ( SoldierTo3DLocationLineOfSightTest( pSoldier, gpSMCurrentMerc->sGridNo,  gpSMCurrentMerc->bLevel, 3, (UINT8) sDistVisible, TRUE ) )
				{
					if ( sDist <= PASSING_ITEM_DISTANCE_NOTOKLIFE )
					{
						gfSMDisableForItems = FALSE;
						break;	// found one, no need to keep looking
					}
				}
			}
		}
	}
	else
	{
		ubSrcSoldier = (UINT8)gusSelectedSoldier;

		if ( gpItemPointer != NULL )
		{
			ubSrcSoldier = gpItemPointerSoldier->ubID;
		}

		// OK buddy, check our currently selected merc and disable/enable if not close enough...
		if ( ubSrcSoldier != NOBODY )
		{
			if ( gusSMCurrentMerc != ubSrcSoldier )
			{
				sDestGridNo = MercPtrs[ gusSMCurrentMerc ]->sGridNo;
				bDestLevel	= MercPtrs[ gusSMCurrentMerc ]->bLevel;

				// Get distance....
				sDist = PythSpacesAway( MercPtrs[ ubSrcSoldier ]->sGridNo, sDestGridNo );

				// is he close enough to see that gridno if he turns his head?
				sDistVisible = DistanceVisible( MercPtrs[ ubSrcSoldier ], DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sDestGridNo, bDestLevel );

				// Check LOS....
				if ( SoldierTo3DLocationLineOfSightTest( MercPtrs[ ubSrcSoldier ], sDestGridNo,  bDestLevel, 3, (UINT8) sDistVisible, TRUE )  )
				{
					// UNCONSCIOUS GUYS ONLY 1 tile AWAY
					if ( MercPtrs[ gusSMCurrentMerc ]->bLife < CONSCIOUSNESS )
					{
						if ( sDist <= PASSING_ITEM_DISTANCE_NOTOKLIFE )
						{
							gfSMDisableForItems = FALSE;
						}
					}
					else if ( sDist <= PASSING_ITEM_DISTANCE_OKLIFE )
					{
						gfSMDisableForItems = FALSE;
					}
				}
			}
			else
			{
				gfSMDisableForItems = FALSE;
			}
		}
		else
		{
			gfSMDisableForItems = FALSE;
		}
	}
}


static void UpdateSMPanel(void);


void SetSMPanelCurrentMerc( UINT8 ubNewID )
{
	gubSelectSMPanelToMerc = NOBODY;

	gusSMCurrentMerc = ubNewID;

	gpSMCurrentMerc = MercPtrs[ ubNewID ];

	// Set to current guy's interface level
	//if ( gsInterfaceLevel != gpSMCurrentMerc->bUIInterfaceLevel )
	//{
	//	SetRenderFlags(RENDER_FLAG_FULL);
	//	ErasePath(FALSE);
	//	gsInterfaceLevel = gpSMCurrentMerc->bUIInterfaceLevel;
	//}

	// Disable all faces
	SetAllAutoFacesInactive( );

	// Turn off compat ammo....
	if ( gpItemPointer == NULL )
	{
		HandleCompatibleAmmoUI( gpSMCurrentMerc, (INT8)HANDPOS, FALSE );
		gfCheckForMouseOverItem = FALSE;
	}
	else
	{
		// Turn it all false first....
		InternalHandleCompatibleAmmoUI( gpSMCurrentMerc, gpItemPointer, FALSE );
		InternalHandleCompatibleAmmoUI( gpSMCurrentMerc, gpItemPointer, TRUE );
	}

	// Remove item desc panel if one up....
	if ( gfInItemDescBox )
	{
		DeleteItemDescriptionBox( );
	}

	if ( gfInItemPickupMenu )
	{
		gfSMDisableForItems = TRUE;
	}
	else
	{
		if ( ( gpItemPointer != NULL || guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE || gpSMCurrentMerc->bLife < OKLIFE ) )
		{
			CheckForDisabledForGiveItem( );
		}
		else
		{
			gfSMDisableForItems = FALSE;
		}
	}

	if ( gpItemPointer != NULL )
	{
		ReevaluateItemHatches( gpSMCurrentMerc, FALSE );
	}


	DisableInvRegions( gfSMDisableForItems );

	fInterfacePanelDirty = DIRTYLEVEL2;

	gfUIStanceDifferent = TRUE;

	UpdateSMPanel( );

}


static void HandleMouseOverSoldierFaceForContMove(SOLDIERTYPE* pSoldier, BOOLEAN fOn);
static BOOLEAN IsMouseInRegion(MOUSE_REGION* pRegion);


void UpdateForContOverPortrait( SOLDIERTYPE *pSoldier, BOOLEAN fOn )
{
	INT32 cnt;

	if ( gsCurInterfacePanel == SM_PANEL )
	{
		if ( gpSMCurrentMerc != NULL )
		{
			// Check if mouse is in region and if so, adjust...
			if ( IsMouseInRegion( &gSM_SELMERCPanelRegion ) )
			{
				HandleMouseOverSoldierFaceForContMove( gpSMCurrentMerc, fOn );
			}
		}
	}
	else
	{
		for ( cnt = 0; cnt < 6; cnt++ )
		{
			if ( gTeamPanel[ cnt ].ubID == pSoldier->ubID )
			{
				if ( IsMouseInRegion( &gTEAM_FaceRegions[ cnt ] ) )
				{
					HandleMouseOverSoldierFaceForContMove( pSoldier, fOn );
				}
			}
		}
	}
}


static void BtnStealthModeCallback(GUI_BUTTON* btn, INT32 reason);
static void CheckForReEvaluateDisabledINVPanelButtons(void);


static void UpdateSMPanel(void)
{
	BOOLEAN						fNearHeigherLevel;
	BOOLEAN						fNearLowerLevel;
	INT8							bDirection;
	UINT8							ubStanceState;

	if ( gpSMCurrentMerc->sGridNo == NOWHERE )
	{
		return;
	}

	// Stance
	ubStanceState = gpSMCurrentMerc->ubDesiredHeight;

	if ( ubStanceState == NO_DESIRED_HEIGHT )
	{
		ubStanceState = gAnimControl[ gpSMCurrentMerc->usAnimState ].ubEndHeight;
	}


	switch( ubStanceState )
	{
		case ANIM_STAND:

			gbSMCurStanceObj = 0;
			DisableButton( iSMPanelButtons[ STANCEUP_BUTTON ] );
			EnableButton( iSMPanelButtons[ STANCEDOWN_BUTTON ] );

			// Disable if we cannot do this!
			if ( !IsValidStance( gpSMCurrentMerc, ANIM_CROUCH ) )
			{
				DisableButton( iSMPanelButtons[ STANCEDOWN_BUTTON ] );
			}
			break;

		case ANIM_PRONE:
			gbSMCurStanceObj = 2;
			DisableButton( iSMPanelButtons[ STANCEDOWN_BUTTON ] );
			EnableButton( iSMPanelButtons[ STANCEUP_BUTTON ] );
			break;

		case ANIM_CROUCH:
			gbSMCurStanceObj = 1;
			EnableButton( iSMPanelButtons[ STANCEUP_BUTTON ] );
			EnableButton( iSMPanelButtons[ STANCEDOWN_BUTTON ] );

			// Disable if we cannot do this!
			if ( !IsValidStance( gpSMCurrentMerc, ANIM_PRONE ) )
			{
				DisableButton( iSMPanelButtons[ STANCEDOWN_BUTTON ] );
			}
			break;
	}

	// Stance button done wether we're disabled or not...
	if ( gfUIStanceDifferent )
	{
		//Remove old
		if ( giSMStealthButton != -1 )
		{
				RemoveButton( giSMStealthButton );
		}
		if ( giSMStealthImages != -1 )
		{
			UnloadButtonImage( giSMStealthImages );
		}

		// Make new
		giSMStealthImages = UseLoadedButtonImage( iSMPanelImages[ STANCE_IMAGES  ] ,gbStanceButPos[ gpSMCurrentMerc->bStealthMode ][gbSMCurStanceObj][0] ,gbStanceButPos[ gpSMCurrentMerc->bStealthMode ][gbSMCurStanceObj][1],-1,gbStanceButPos[ gpSMCurrentMerc->bStealthMode ][gbSMCurStanceObj][2],-1 );

		giSMStealthButton = QuickCreateButton( giSMStealthImages, SM_STEALTHMODE_X, SM_STEALTHMODE_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH - 1,
										DEFAULT_MOVE_CALLBACK, BtnStealthModeCallback );

		SetButtonFastHelpText( giSMStealthButton, TacticalStr[ TOGGLE_STEALTH_MODE_POPUPTEXT ] );

		gfUIStanceDifferent = FALSE;

		if ( gfAllDisabled )
		{
			if ( giSMStealthButton != -1 )
			{
					DisableButton( giSMStealthButton );
			}
		}
	}

	if ( gfAllDisabled )
	{
		return;
	}


	CheckForReEvaluateDisabledINVPanelButtons( );

	// Check for any newly added items we need.....
	if ( gpSMCurrentMerc->fCheckForNewlyAddedItems )
	{
		// Startup any newly added items....
		CheckForAnyNewlyAddedItems( gpSMCurrentMerc );

		gpSMCurrentMerc->fCheckForNewlyAddedItems = FALSE;
	}



	// Set Disable /Enable UI based on buddy's stats
	if (ButtonList[ iSMPanelButtons[ BURSTMODE_BUTTON ] ]->ImageNum != (UINT32) iBurstButtonImages[ gpSMCurrentMerc->bWeaponMode ] )
	{
		ButtonList[ iSMPanelButtons[ BURSTMODE_BUTTON ] ]->ImageNum = iBurstButtonImages[ gpSMCurrentMerc->bWeaponMode ];
		ButtonList[ iSMPanelButtons[ BURSTMODE_BUTTON ] ]->uiFlags |= BUTTON_DIRTY;
	}

	/*
	if ( gpSMCurrentMerc->bDoBurst )
	{
		if ( !ButtonList[ iSMPanelButtons[ BURSTMODE_BUTTON ] ]->ubToggleButtonActivated )
		{
			ButtonList[ iSMPanelButtons[ BURSTMODE_BUTTON ] ]->uiFlags |= BUTTON_CLICKED_ON;
		}
	}
	else
	{
		if ( !ButtonList[ iSMPanelButtons[ BURSTMODE_BUTTON ] ]->ubToggleButtonActivated )
		{
			ButtonList[ iSMPanelButtons[ BURSTMODE_BUTTON ] ]->uiFlags &= ( ~BUTTON_CLICKED_ON );
		}
	}
	*/


	// Toggle MUTE button...
	if ( gpSMCurrentMerc->uiStatusFlags & SOLDIER_MUTE )
	{
		if ( !ButtonList[ iSMPanelButtons[ MUTE_BUTTON ] ]->ubToggleButtonActivated )
		{
			ButtonList[ iSMPanelButtons[ MUTE_BUTTON ] ]->uiFlags |= BUTTON_CLICKED_ON;
		}
	}
	else
	{
		if ( !ButtonList[ iSMPanelButtons[ MUTE_BUTTON ] ]->ubToggleButtonActivated )
		{
			ButtonList[ iSMPanelButtons[ MUTE_BUTTON ] ]->uiFlags &= ( ~BUTTON_CLICKED_ON );
		}
	}


	DisableButton( iSMPanelButtons[ CLIMB_BUTTON ] );

	GetMercClimbDirection( gpSMCurrentMerc->ubID, &fNearLowerLevel, &fNearHeigherLevel );

	if ( fNearLowerLevel || fNearHeigherLevel )
	{
		if ( fNearLowerLevel )
		{
			if ( EnoughPoints( gpSMCurrentMerc, GetAPsToClimbRoof( gpSMCurrentMerc, TRUE ), 0, FALSE ) )
			{
				EnableButton( iSMPanelButtons[ CLIMB_BUTTON ] );
			}
		}

		if ( fNearHeigherLevel )
		{
			if ( EnoughPoints( gpSMCurrentMerc, GetAPsToClimbRoof( gpSMCurrentMerc, FALSE ), 0, FALSE ) )
			{
				EnableButton( iSMPanelButtons[ CLIMB_BUTTON ] );
			}
		}
	}

	if ( FindFenceJumpDirection( gpSMCurrentMerc, gpSMCurrentMerc->sGridNo, gpSMCurrentMerc->bDirection, &bDirection ) )
	{
		EnableButton( iSMPanelButtons[ CLIMB_BUTTON ] );
	}

	if ( (gTacticalStatus.ubCurrentTeam != gbPlayerNum) || (gTacticalStatus.uiFlags & REALTIME ) || !(gTacticalStatus.uiFlags & INCOMBAT ) )
	{
		DisableButton( iSMPanelButtons[ SM_DONE_BUTTON ] );
	}
	else if ( !gfAllDisabled )
	{
		EnableButton( iSMPanelButtons[ SM_DONE_BUTTON ] );
	}

//	if ( gpSMCurrentMerc->bUIInterfaceLevel > 0 )
	if ( gsInterfaceLevel > 0 )
	{
		if ( !ButtonList[ iSMPanelButtons[ UPDOWN_BUTTON ] ]->ubToggleButtonActivated )
		{
			ButtonList[ iSMPanelButtons[ UPDOWN_BUTTON ] ]->uiFlags |= (BUTTON_CLICKED_ON);
		}
	}
	else
	{
		if ( !ButtonList[ iSMPanelButtons[ UPDOWN_BUTTON ] ]->ubToggleButtonActivated )
		{
			ButtonList[ iSMPanelButtons[ UPDOWN_BUTTON ] ]->uiFlags &= ( ~BUTTON_CLICKED_ON );
		}
	}

	if ( gCurrentUIMode == HANDCURSOR_MODE )
	{
		if ( !ButtonList[ iSMPanelButtons[ HANDCURSOR_BUTTON ] ]->ubToggleButtonActivated )
		{
			ButtonList[ iSMPanelButtons[ HANDCURSOR_BUTTON ] ]->uiFlags |= BUTTON_CLICKED_ON;
		}
	}
	else
	{
		if ( !ButtonList[ iSMPanelButtons[ HANDCURSOR_BUTTON ] ]->ubToggleButtonActivated )
		{
			ButtonList[ iSMPanelButtons[ HANDCURSOR_BUTTON ] ]->uiFlags &= ( ~BUTTON_CLICKED_ON );
		}
	}

	if ( gCurrentUIMode == TALKCURSOR_MODE )
	{
		if ( !ButtonList[ iSMPanelButtons[ TALK_BUTTON ] ]->ubToggleButtonActivated )
		{
			ButtonList[ iSMPanelButtons[ TALK_BUTTON ] ]->uiFlags |= BUTTON_CLICKED_ON;
		}
	}
	else
	{
		if ( !ButtonList[ iSMPanelButtons[ TALK_BUTTON ] ]->ubToggleButtonActivated )
		{
			ButtonList[ iSMPanelButtons[ TALK_BUTTON ] ]->uiFlags &= ( ~BUTTON_CLICKED_ON );
		}
	}

	if ( gCurrentUIMode == LOOKCURSOR_MODE )
	{
		if ( !ButtonList[ iSMPanelButtons[ LOOK_BUTTON ] ]->ubToggleButtonActivated )
		{
			ButtonList[ iSMPanelButtons[ LOOK_BUTTON ] ]->uiFlags |= BUTTON_CLICKED_ON;
		}
	}
	else
	{
		if ( !ButtonList[ iSMPanelButtons[ LOOK_BUTTON ] ]->ubToggleButtonActivated )
		{
			ButtonList[ iSMPanelButtons[ LOOK_BUTTON ] ]->uiFlags &= ( ~BUTTON_CLICKED_ON );
		}
	}

	// If not selected ( or dead ), disable/gray some buttons
	if ( gusSelectedSoldier != gpSMCurrentMerc->ubID || ( gpSMCurrentMerc->bLife < OKLIFE ) || (gTacticalStatus.ubCurrentTeam != gbPlayerNum) || gfSMDisableForItems )
	{
		DisableButton( iSMPanelButtons[ CLIMB_BUTTON ] );
		DisableButton( iSMPanelButtons[ BURSTMODE_BUTTON ] );
		DisableButton( iSMPanelButtons[ STANCEUP_BUTTON ] );
		DisableButton( iSMPanelButtons[ STANCEDOWN_BUTTON ] );
		DisableButton( iSMPanelButtons[ LOOK_BUTTON ] );
		DisableButton( iSMPanelButtons[ UPDOWN_BUTTON ] );
		DisableButton( iSMPanelButtons[ HANDCURSOR_BUTTON ] );
		if ( giSMStealthButton != -1 )
		{
				DisableButton( giSMStealthButton );
		}
	}
	else
	{
		// Enable some buttons!
		if ( IsGunBurstCapable( gpSMCurrentMerc, HANDPOS , FALSE ) || FindAttachment( &(gpSMCurrentMerc->inv[HANDPOS]), UNDER_GLAUNCHER ) != ITEM_NOT_FOUND )
		{
			EnableButton( iSMPanelButtons[ BURSTMODE_BUTTON ] );
		}
		else
		{
			DisableButton( iSMPanelButtons[ BURSTMODE_BUTTON ] );
		}
		EnableButton( iSMPanelButtons[ LOOK_BUTTON ] );
		EnableButton( iSMPanelButtons[ UPDOWN_BUTTON ] );
		EnableButton( iSMPanelButtons[ HANDCURSOR_BUTTON ] );

		if ( giSMStealthButton != -1 )
		{
				EnableButton( giSMStealthButton );
		}
	}

	// CJC Dec 4 2002: or if item pickup menu is up
	if ( (gTacticalStatus.uiFlags & ENGAGED_IN_CONV) || gfInItemPickupMenu)
	{
		DisableButton( iSMPanelButtons[ SM_MAP_SCREEN_BUTTON ] );
	}
	else
	{
		EnableButton( iSMPanelButtons[ SM_MAP_SCREEN_BUTTON ] );
	}

}

extern BOOLEAN CanItemFitInPosition( SOLDIERTYPE *pSoldier, OBJECTTYPE *pObj, INT8 bPos, BOOLEAN fDoingPlacement );
extern INT8		 gbInvalidPlacementSlot[ NUM_INV_SLOTS ];


void ReevaluateItemHatches( SOLDIERTYPE *pSoldier, BOOLEAN fAllValid )
{
	INT32	cnt;

	// if there's an item in the cursor and we're not supposed to just make them all valid
	if ( ( gpItemPointer != NULL ) && !fAllValid )
	{
		// check all inventory positions and mark the ones where cursor item won't fit as invalid
		for ( cnt = 0; cnt < NUM_INV_SLOTS; cnt++ )
		{
			gbInvalidPlacementSlot[ cnt ] = !CanItemFitInPosition( pSoldier, gpItemPointer, (INT8)cnt, FALSE );

			// !!! ATTACHING/MERGING ITEMS IN MAP SCREEN IS NOT SUPPORTED !!!
			// CJC: seems to be supported now...
			//if( guiCurrentScreen != MAP_SCREEN )
			{
				// Check attachments, override to valid placement if valid merge...
				if ( ValidAttachment( gpItemPointer->usItem, pSoldier->inv[ cnt ].usItem ) )
				{
					gbInvalidPlacementSlot[ cnt ] = FALSE;
				}

				if ( ValidMerge( gpItemPointer->usItem, pSoldier->inv[ cnt ].usItem ) )
				{
					gbInvalidPlacementSlot[ cnt ] = FALSE;
				}
			}
		}
	}
	else
	{
		// mark all inventory positions as valid
		for ( cnt = 0; cnt < NUM_INV_SLOTS; cnt++ )
		{
			gbInvalidPlacementSlot[ cnt ] = FALSE;
		}
	}

	fInterfacePanelDirty = DIRTYLEVEL2;
}


void EnableSMPanelButtons( BOOLEAN fEnable , BOOLEAN fFromItemPickup )
{
	if ( fFromItemPickup )
	{
		// If we have the item pointer up...
		// CJC Dec 4 2002: or if item pickup menu is up
		//if ( gpItemPointer != NULL )
		if ( gpItemPointer != NULL || gfInItemPickupMenu )
		{
			DisableTacticalTeamPanelButtons( TRUE );
		}
		else
		{
			DisableTacticalTeamPanelButtons( FALSE );
		}

		fInterfacePanelDirty = DIRTYLEVEL2;
	}


	if ( gsCurInterfacePanel == SM_PANEL )
	{
		if ( fFromItemPickup )
		{
			// If we have the item pointer up...
			if ( gpItemPointer != NULL )
			{
				ReevaluateItemHatches( gpSMCurrentMerc, fEnable );

				// Turn it all false first....
				InternalHandleCompatibleAmmoUI( gpSMCurrentMerc, gpItemPointer, FALSE );
				InternalHandleCompatibleAmmoUI( gpSMCurrentMerc, gpItemPointer, TRUE );

				gfCheckForMouseOverItem = FALSE;

				// Highlight guys ....
				HandleAnyMercInSquadHasCompatibleStuff( (INT8)CurrentSquad( ), gpItemPointer, FALSE );

			}
			else
			{
				//InternalHandleCompatibleAmmoUI( gpSMCurrentMerc, gpItemPointer, FALSE );
				gfCheckForMouseOverItem = FALSE;

				HandleAnyMercInSquadHasCompatibleStuff( (INT8) CurrentSquad( ), NULL, TRUE );
			}

			if ( fEnable )
			{
				ReevaluateItemHatches( gpSMCurrentMerc, fEnable );
			}

			fInterfacePanelDirty = DIRTYLEVEL2;
		}

		if ( fEnable )
		{
			// only enable the following if NOT in shopkeeper's interface
			if ( !(guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE ) )
			{
				EnableButton( iSMPanelButtons[ CLIMB_BUTTON ] );
				EnableButton( iSMPanelButtons[ BURSTMODE_BUTTON ] );
				EnableButton( iSMPanelButtons[ STANCEUP_BUTTON ] );
				EnableButton( iSMPanelButtons[ STANCEDOWN_BUTTON ] );
				EnableButton( iSMPanelButtons[ LOOK_BUTTON ] );
				EnableButton( iSMPanelButtons[ UPDOWN_BUTTON ] );
				EnableButton( iSMPanelButtons[ HANDCURSOR_BUTTON ] );
				if ( giSMStealthButton != -1 )
				{
					EnableButton( giSMStealthButton );
				}

				if ( gfDisableTacticalPanelButtons )
				{
					DisableButton( iSMPanelButtons[ OPTIONS_BUTTON ] );
					DisableButton( iSMPanelButtons[ SM_DONE_BUTTON ] );
					DisableButton( iSMPanelButtons[ SM_MAP_SCREEN_BUTTON ] );
				}
				else
				{
					EnableButton( iSMPanelButtons[ OPTIONS_BUTTON ] );
					EnableButton( iSMPanelButtons[ SM_DONE_BUTTON ] );
					EnableButton( iSMPanelButtons[ SM_MAP_SCREEN_BUTTON ] );
				}


				//enable the radar map region
				MSYS_EnableRegion(&gRadarRegion);

				gfSMDisableForItems = FALSE;

      	DisableInvRegions( gfSMDisableForItems );
			}

			if ( !fFromItemPickup )
			{
				EnableButton( iSMPanelButtons[ NEXTMERC_BUTTON ] );
				EnableButton( iSMPanelButtons[ PREVMERC_BUTTON ] );
			}
		}
		else
		{
			DisableButton( iSMPanelButtons[ CLIMB_BUTTON ] );
			DisableButton( iSMPanelButtons[ BURSTMODE_BUTTON ] );
			DisableButton( iSMPanelButtons[ STANCEUP_BUTTON ] );
			DisableButton( iSMPanelButtons[ STANCEDOWN_BUTTON ] );
			DisableButton( iSMPanelButtons[ LOOK_BUTTON ] );
			DisableButton( iSMPanelButtons[ UPDOWN_BUTTON ] );
			DisableButton( iSMPanelButtons[ HANDCURSOR_BUTTON ] );
			if ( giSMStealthButton != -1 )
			{
					DisableButton( giSMStealthButton );
			}

			if ( !fFromItemPickup )
			{
				DisableButton( iSMPanelButtons[ NEXTMERC_BUTTON ] );
				DisableButton( iSMPanelButtons[ PREVMERC_BUTTON ] );
			}

			DisableButton( iSMPanelButtons[ OPTIONS_BUTTON ] );
			DisableButton( iSMPanelButtons[ SM_DONE_BUTTON ] );
			DisableButton( iSMPanelButtons[ SM_MAP_SCREEN_BUTTON ] );

			//disable the radar map
			MSYS_DisableRegion(&gRadarRegion);
		}

		gfAllDisabled = !fEnable;

	}
}

UINT16 GetSMPanelCurrentMerc( )
{
	return( gusSMCurrentMerc );
}


static void InvPanelButtonClickCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void SMInvClickCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void SMInvClickCamoCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void SMInvMoneyButtonCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void SMInvMoveCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void SMInvMoveCammoCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectedMercButtonCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectedMercButtonMoveCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectedMercEnemyIndicatorCallback(MOUSE_REGION* pRegion, INT32 iReason);


BOOLEAN InitializeSMPanel(  )
{
	// failing the CHECKF after this will cause you to lose your mouse
	guiSMPanel = AddVideoObjectFromFile("INTERFACE/inventory_bottom_panel.STI");
	CHECKF(guiSMPanel != NO_VOBJECT);
	guiSMObjects = AddVideoObjectFromFile("INTERFACE/inventory_gold_front.sti");
	CHECKF(guiSMObjects != NO_VOBJECT);
	guiSMObjects2 = AddVideoObjectFromFile("INTERFACE/inv_frn.sti");
	CHECKF(guiSMObjects2 != NO_VOBJECT);
	guiSecItemHiddenVO = AddVideoObjectFromFile("INTERFACE/secondary_gun_hidden.sti");
	CHECKF(guiSecItemHiddenVO != NO_VOBJECT);
	guiBrownBackgroundForTeamPanel = AddVideoObjectFromFile("INTERFACE/Bars.sti");
	CHECKF(guiBrownBackgroundForTeamPanel != NO_VOBJECT);

	// Clear inv display stuff
	memset( gfSM_HandInvDispText, 0, sizeof( gfSM_HandInvDispText ) );

	// INit viewport region
	// Set global mouse regions
	// Define region for viewport
	MSYS_DefineRegion( &gViewportRegion, 0, 0 ,gsVIEWPORT_END_X, gsVIEWPORT_WINDOW_END_Y, MSYS_PRIORITY_NORMAL,
						 VIDEO_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );

	// Create buttons
	CHECKF( CreateSMPanelButtons( ) );

	// Set viewports
	// Define region for panel
	MSYS_DefineRegion(&gSMPanelRegion, 0, INV_INTERFACE_START_Y, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_NORMAL, CURSOR_NORMAL, MSYS_NO_CALLBACK, InvPanelButtonClickCallback);

	//DEfine region for selected guy panel
	MSYS_DefineRegion( &gSM_SELMERCPanelRegion, SM_SELMERC_FACE_X, SM_SELMERC_FACE_Y, SM_SELMERC_FACE_X + SM_SELMERC_FACE_WIDTH, SM_SELMERC_FACE_Y + SM_SELMERC_FACE_HEIGHT, MSYS_PRIORITY_NORMAL,
						 MSYS_NO_CURSOR, SelectedMercButtonMoveCallback, SelectedMercButtonCallback );

	//DEfine region for selected guy panel
	MSYS_DefineRegion( &gSM_SELMERCEnemyIndicatorRegion, SM_SELMERC_FACE_X + 1, SM_SELMERC_FACE_Y + 1, SM_SELMERC_FACE_X + INDICATOR_BOX_WIDTH, SM_SELMERC_FACE_Y + INDICATOR_BOX_HEIGHT, MSYS_PRIORITY_NORMAL,
						 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, SelectedMercEnemyIndicatorCallback );

	//DEfine region for money button
	MSYS_DefineRegion( &gSM_SELMERCMoneyRegion, MONEY_X, MONEY_Y, MONEY_X + MONEY_WIDTH, MONEY_Y + MONEY_HEIGHT, MSYS_PRIORITY_HIGH,
						 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, SMInvMoneyButtonCallback );

	SetRegionFastHelpText( &(gSM_SELMERCMoneyRegion), TacticalStr[ MONEY_BUTTON_HELP_TEXT ] );



	// Check if mouse is in region and if so, adjust...
	if ( IsMouseInRegion( &gSM_SELMERCPanelRegion ) )
	{
		HandleMouseOverSoldierFaceForContMove( gpSMCurrentMerc, TRUE );
	}


	//DEfine region for selected guy panel
	MSYS_DefineRegion( &gSM_SELMERCBarsRegion, 62, 342, 85, 391, MSYS_PRIORITY_NORMAL,
						 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, SelectedMercButtonCallback );


	InitInvSlotInterface( gSMInvPocketXY, &gSMCamoXY, SMInvMoveCallback, SMInvClickCallback, SMInvMoveCammoCallback, SMInvClickCamoCallback, FALSE );

	InitKeyRingInterface( KeyRingItemPanelButtonCallback );


	// this is important! It will disable buttons like SM_MAP_SCREEN_BUTTON when they're supposed to be disabled - the previous
	// disabled state is lost everytime panel is reinitialized, because all the buttons are created from scratch!
	if ( gpItemPointer == NULL )
	{
		// empty cursor - enable, not from item pickup
		EnableSMPanelButtons( TRUE, FALSE );
	}
	else
	{
		// full cursor - disable, from item pickup
		EnableSMPanelButtons( FALSE, TRUE );
	}

	return( TRUE );
}


static void BtnBurstModeCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnClimbCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnHandCursorCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnLookCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnMapScreenCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnMuteCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnNextMercCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnOptionsCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnPrevMercCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnSMDoneCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnStanceDownCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnStanceUpCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnTalkCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnUpdownCallback(GUI_BUTTON* btn, INT32 reason);


BOOLEAN CreateSMPanelButtons( )
{
	giSMStealthImages = -1;
	giSMStealthButton = -1;
	gfUIStanceDifferent = TRUE;
	gfAllDisabled	= FALSE;

	// Load button Graphics
	iSMPanelImages[STANCEUP_IMAGES] = LoadButtonImage("INTERFACE/inventory_buttons.sti", -1, 0, -1, 10, -1);

	iSMPanelImages[ UPDOWN_IMAGES  ]				= UseLoadedButtonImage( iSMPanelImages[ STANCEUP_IMAGES  ] ,-1,9,-1,19,-1 );
	iSMPanelImages[ CLIMB_IMAGES  ]					= UseLoadedButtonImage( iSMPanelImages[ STANCEUP_IMAGES  ] ,-1,3,-1,13,-1 );
	iSMPanelImages[ STANCEDOWN_IMAGES  ]		= UseLoadedButtonImage( iSMPanelImages[ STANCEUP_IMAGES  ] ,-1,8,-1,18,-1 );
	iSMPanelImages[ HANDCURSOR_IMAGES  ]		= UseLoadedButtonImage( iSMPanelImages[ STANCEUP_IMAGES  ] ,-1,1,-1,11,-1 );
	iSMPanelImages[ PREVMERC_IMAGES  ]			= UseLoadedButtonImage( iSMPanelImages[ STANCEUP_IMAGES  ] ,-1,20,-1,22,-1 );
	iSMPanelImages[ NEXTMERC_IMAGES  ]			= UseLoadedButtonImage( iSMPanelImages[ STANCEUP_IMAGES  ] ,-1,21,-1,23,-1 );
	//iSMPanelImages[ BURSTMODE_IMAGES  ]			= UseLoadedButtonImage( iSMPanelImages[ STANCEUP_IMAGES  ] ,-1,7,-1,17,-1 );
	iSMPanelImages[ LOOK_IMAGES  ]					= UseLoadedButtonImage( iSMPanelImages[ STANCEUP_IMAGES  ] ,-1,2,-1,12,-1 );
	iSMPanelImages[ TALK_IMAGES  ]					= UseLoadedButtonImage( iSMPanelImages[ STANCEUP_IMAGES  ] ,-1,6,-1,16,-1 );
	iSMPanelImages[ MUTE_IMAGES  ]					= UseLoadedButtonImage( iSMPanelImages[ STANCEUP_IMAGES  ] ,-1,5,-1,15,-1 );
	iSMPanelImages[ OPTIONS_IMAGES  ]				= UseLoadedButtonImage( iSMPanelImages[ STANCEUP_IMAGES  ] ,-1,24,-1,25,-1 );

	iBurstButtonImages[ WM_NORMAL ]					= UseLoadedButtonImage( iSMPanelImages[ STANCEUP_IMAGES  ], -1, 7, -1, -1, -1 );
	iBurstButtonImages[ WM_BURST ]					= UseLoadedButtonImage( iSMPanelImages[ STANCEUP_IMAGES  ], -1, 17, -1, -1, -1 );
	iBurstButtonImages[ WM_ATTACHED ]				= UseLoadedButtonImage( iSMPanelImages[ STANCEUP_IMAGES  ], -1, 26, -1, -1, -1 );

	// Load button Graphics
	iSMPanelImages[STANCE_IMAGES] = LoadButtonImage("INTERFACE/invadd-ons.sti", 0, 0, -1, 2, -1);

	// Load button Graphics
	iSMPanelImages[DONE_IMAGES] = LoadButtonImage("INTERFACE/inventory_buttons_2.sti", -1, 1, -1, 3, -1);
	iSMPanelImages[ MAPSCREEN_IMAGES  ]			= UseLoadedButtonImage( iSMPanelImages[ DONE_IMAGES  ] ,-1,0,-1,2,-1 );


	// Create buttons

	// SET BUTTONS TO -1
	memset( iSMPanelButtons, -1, sizeof( iSMPanelButtons ) );

	iSMPanelButtons[ SM_MAP_SCREEN_BUTTON ] = QuickCreateButton( iSMPanelImages[ MAPSCREEN_IMAGES ], SM_MAPSCREEN_X, SM_MAPSCREEN_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH - 1,
										DEFAULT_MOVE_CALLBACK, BtnMapScreenCallback );
	SetButtonFastHelpText( iSMPanelButtons[ SM_MAP_SCREEN_BUTTON ], TacticalStr[ MAPSCREEN_POPUPTEXT ]);

	iSMPanelButtons[SM_DONE_BUTTON] = QuickCreateButton(iSMPanelImages[DONE_IMAGES], SM_DONE_X, SM_DONE_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH - 1, DEFAULT_MOVE_CALLBACK, BtnSMDoneCallback);
	SetButtonFastHelpText( iSMPanelButtons[ SM_DONE_BUTTON ], TacticalStr[ END_TURN_POPUPTEXT ] );

	iSMPanelButtons[ TALK_BUTTON ] = QuickCreateButton( iSMPanelImages[ TALK_IMAGES ], SM_TALKB_X, SM_TALKB_Y,
										BUTTON_NEWTOGGLE, MSYS_PRIORITY_HIGH - 1,
										MSYS_NO_CALLBACK, BtnTalkCallback );
	SetButtonFastHelpText( iSMPanelButtons[ TALK_BUTTON ], TacticalStr[ TALK_CURSOR_POPUPTEXT ] );

	iSMPanelButtons[ MUTE_BUTTON ] = QuickCreateButton( iSMPanelImages[ MUTE_IMAGES ], SM_MUTEB_X, SM_MUTEB_Y,
										BUTTON_NEWTOGGLE, MSYS_PRIORITY_HIGH - 1,
										DEFAULT_MOVE_CALLBACK, BtnMuteCallback );
	SetButtonFastHelpText( iSMPanelButtons[ MUTE_BUTTON ], TacticalStr[ TOGGLE_MUTE_POPUPTEXT ] );

	iSMPanelButtons[ STANCEUP_BUTTON ] = QuickCreateButton( iSMPanelImages[ STANCEUP_IMAGES ], SM_STANCEUPB_X, SM_STANCEUPB_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH - 1,
										DEFAULT_MOVE_CALLBACK, BtnStanceUpCallback );
	if ( iSMPanelButtons[ STANCEUP_BUTTON ] == -1 )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button" );
		return( FALSE );
	}
	SetButtonFastHelpText( iSMPanelButtons[ STANCEUP_BUTTON ], TacticalStr[ CHANGE_STANCE_UP_POPUPTEXT ] );

	//SetButtonFastHelpText( iSMPanelButtons[ STANCEUP_BUTTON ],L"Change Stance Up");


	iSMPanelButtons[ UPDOWN_BUTTON ] = QuickCreateButton( iSMPanelImages[ UPDOWN_IMAGES ], SM_UPDOWNB_X, SM_UPDOWNB_Y,
										BUTTON_NEWTOGGLE, MSYS_PRIORITY_HIGH - 1,
										MSYS_NO_CALLBACK, BtnUpdownCallback );
	if ( iSMPanelButtons[ UPDOWN_BUTTON ] == -1 )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button" );
		return( FALSE );
	}
	//SetButtonFastHelpText( iSMPanelButtons[ UPDOWN_BUTTON ],L"Whatever");
	SetButtonFastHelpText( iSMPanelButtons[ UPDOWN_BUTTON ], TacticalStr[ CURSOR_LEVEL_POPUPTEXT ] );

	iSMPanelButtons[ CLIMB_BUTTON ] = QuickCreateButton( iSMPanelImages[ CLIMB_IMAGES ], SM_CLIMBB_X, SM_CLIMBB_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH - 1,
										DEFAULT_MOVE_CALLBACK, BtnClimbCallback );
	if ( iSMPanelButtons[ CLIMB_BUTTON ] == -1 )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button" );
		return( FALSE );
	}
	//SetButtonFastHelpText( iSMPanelButtons[ CLIMB_BUTTON ],L"Climb");
	SetButtonFastHelpText( iSMPanelButtons[ CLIMB_BUTTON ], TacticalStr[ JUMPCLIMB_POPUPTEXT ] );

	iSMPanelButtons[ STANCEDOWN_BUTTON ] = QuickCreateButton( iSMPanelImages[ STANCEDOWN_IMAGES ], SM_STANCEDOWNB_X, SM_STANCEDOWNB_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH - 1,
										DEFAULT_MOVE_CALLBACK, BtnStanceDownCallback );
	if ( iSMPanelButtons[ STANCEDOWN_BUTTON ] == -1 )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button" );
		return( FALSE );
	}
	//SetButtonFastHelpText( iSMPanelButtons[ STANCEDOWN_BUTTON ],L"Change Stance Down");
	SetButtonFastHelpText( iSMPanelButtons[ STANCEDOWN_BUTTON ], TacticalStr[ CHANGE_STANCE_DOWN_POPUPTEXT ] );

	iSMPanelButtons[ HANDCURSOR_BUTTON ] = QuickCreateButton( iSMPanelImages[ HANDCURSOR_IMAGES ], SM_HANDCURSORB_X, SM_HANDCURSORB_Y,
										BUTTON_NEWTOGGLE, MSYS_PRIORITY_HIGH - 1,
										MSYS_NO_CALLBACK, BtnHandCursorCallback );
	if ( iSMPanelButtons[ HANDCURSOR_BUTTON ] == -1 )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button" );
		return( FALSE );
	}
	//SetButtonFastHelpText( iSMPanelButtons[ HANDCURSOR_BUTTON ],L"Change Stance Down");
	SetButtonFastHelpText( iSMPanelButtons[ HANDCURSOR_BUTTON ], TacticalStr[ EXAMINE_CURSOR_POPUPTEXT ] );

	iSMPanelButtons[ PREVMERC_BUTTON ] = QuickCreateButton( iSMPanelImages[ PREVMERC_IMAGES ], SM_PREVMERCB_X, SM_PREVMERCB_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH - 1,
										DEFAULT_MOVE_CALLBACK, BtnPrevMercCallback );
	if ( iSMPanelButtons[ PREVMERC_BUTTON ] == -1 )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button" );
		return( FALSE );
	}
	//SetButtonFastHelpText( iSMPanelButtons[ PREVMERC_BUTTON ],L"Change Stance Down");
	SetButtonFastHelpText( iSMPanelButtons[ PREVMERC_BUTTON ], TacticalStr[ PREV_MERC_POPUPTEXT ] );

	iSMPanelButtons[ NEXTMERC_BUTTON ] = QuickCreateButton( iSMPanelImages[ NEXTMERC_IMAGES ], SM_NEXTMERCB_X, SM_NEXTMERCB_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH - 1,
										DEFAULT_MOVE_CALLBACK, BtnNextMercCallback );
	if ( iSMPanelButtons[ NEXTMERC_BUTTON ] == -1 )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button" );
		return( FALSE );
	}
	SetButtonFastHelpText( iSMPanelButtons[ NEXTMERC_BUTTON ], TacticalStr[ NEXT_MERC_POPUPTEXT ] );
	//SetButtonFastHelpText( iSMPanelButtons[ NEXTMERC_BUTTON ],L"Change Stance Down");

	iSMPanelButtons[ OPTIONS_BUTTON ] = QuickCreateButton( iSMPanelImages[ OPTIONS_IMAGES ], SM_OPTIONSB_X, SM_OPTIONSB_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH - 1,
										DEFAULT_MOVE_CALLBACK, BtnOptionsCallback );
	if ( iSMPanelButtons[ OPTIONS_BUTTON ] == -1 )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button" );
		return( FALSE );
	}
	//SetButtonFastHelpText( iSMPanelButtons[ OPTIONS_BUTTON ],L"Change Stance Down");
	SetButtonFastHelpText( iSMPanelButtons[ OPTIONS_BUTTON ], TacticalStr[ CHANGE_OPTIONS_POPUPTEXT ] );

	/*iSMPanelButtons[ BURSTMODE_BUTTON ] = QuickCreateButton( iSMPanelImages[ BURSTMODE_IMAGES ], SM_BURSTMODEB_X, SM_BURSTMODEB_Y,
										BUTTON_NEWTOGGLE, MSYS_PRIORITY_HIGH - 1,
										MSYS_NO_CALLBACK, BtnBurstModeCallback );*/
	iSMPanelButtons[ BURSTMODE_BUTTON ] = QuickCreateButton( iBurstButtonImages[ WM_NORMAL ], SM_BURSTMODEB_X, SM_BURSTMODEB_Y, BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGH, MSYS_NO_CALLBACK, BtnBurstModeCallback );

	if ( iSMPanelButtons[ BURSTMODE_BUTTON ] == -1 )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button" );
		return( FALSE );
	}
	SetButtonFastHelpText( iSMPanelButtons[ BURSTMODE_BUTTON ], TacticalStr[ TOGGLE_BURSTMODE_POPUPTEXT ] );
	//SetButtonFastHelpText( iSMPanelButtons[ BURSTMODE_BUTTON ],L"Change Stance Down");

	iSMPanelButtons[ LOOK_BUTTON ] = QuickCreateButton( iSMPanelImages[ LOOK_IMAGES ], SM_LOOKB_X, SM_LOOKB_Y,
										BUTTON_NEWTOGGLE, MSYS_PRIORITY_HIGH - 1,
										MSYS_NO_CALLBACK, BtnLookCallback );
	if ( iSMPanelButtons[ LOOK_BUTTON ] == -1 )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button" );
		return( FALSE );
	}
	//SetButtonFastHelpText( iSMPanelButtons[ LOOK_BUTTON ],L"Change Stance Down");
	SetButtonFastHelpText( iSMPanelButtons[ LOOK_BUTTON ], TacticalStr[ LOOK_CURSOR_POPUPTEXT ] );

	return( TRUE );
}

void    RemoveSMPanelButtons( )
{
	UINT32 cnt;

	for ( cnt = 0; cnt < NUM_SM_BUTTONS; cnt++ )
	{
		if ( iSMPanelButtons[ cnt ] != -1 )
		{
			RemoveButton( iSMPanelButtons[ cnt ] );
		}
	}

	for ( cnt = 0; cnt < NUM_SM_BUTTON_IMAGES; cnt++ )
	{
		UnloadButtonImage( iSMPanelImages[ cnt ] );
	}

	if ( giSMStealthButton != -1 )
	{
		RemoveButton( giSMStealthButton );
	}

	if ( giSMStealthImages != -1 )
	{
		UnloadButtonImage( giSMStealthImages );
	}

	UnloadButtonImage( iBurstButtonImages[ WM_NORMAL ] );
	UnloadButtonImage( iBurstButtonImages[ WM_BURST ] );
	UnloadButtonImage( iBurstButtonImages[ WM_ATTACHED ] );

}




BOOLEAN ShutdownSMPanel( )
{

	// All buttons and regions and video objects and video surfaces will be deleted at shutddown of SGM
	// We may want to delete them at the interm as well, to free up room for other panels
	DeleteVideoObjectFromIndex( guiSMPanel );
	DeleteVideoObjectFromIndex( guiSMObjects );
	DeleteVideoObjectFromIndex( guiSMObjects2 );
	DeleteVideoObjectFromIndex( guiSecItemHiddenVO );
	DeleteVideoObjectFromIndex( guiBrownBackgroundForTeamPanel );

	gubSelectSMPanelToMerc = NOBODY;

	// CJC: delete key ring if open
	DeleteKeyRingPopup(); // function will abort if key ring is not up

	// ATE: Delete desc panel if it was open....
	if ( gfInItemDescBox )
	{
		DeleteItemDescriptionBox( );
	}


	// Shotdown item slot interface
	ShutdownInvSlotInterface( );

	// shutdown keyring interface
	ShutdownKeyRingInterface( );

	MSYS_RemoveRegion( &gSMPanelRegion );
	MSYS_RemoveRegion( &gSM_SELMERCPanelRegion );
	MSYS_RemoveRegion( &gSM_SELMERCBarsRegion );
	MSYS_RemoveRegion( &gSM_SELMERCMoneyRegion );
	MSYS_RemoveRegion( &gSM_SELMERCEnemyIndicatorRegion );

	HandleMouseOverSoldierFaceForContMove( gpSMCurrentMerc, FALSE );

	MSYS_RemoveRegion( &gViewportRegion );


	RemoveSMPanelButtons( );

	return( TRUE );
}


static void CheckForFacePanelStartAnims(SOLDIERTYPE* pSoldier, INT16 sPanelX, INT16 sPanelY);
static void HandleSoldierFaceFlash(SOLDIERTYPE* pSoldier, INT16 sFaceX, INT16 sFaceY);
static void UpdateStatColor(UINT32 uiTimer, BOOLEAN fIncrease);


void RenderSMPanel( BOOLEAN *pfDirty )
{
	INT16 sFontX, sFontY;
	UINT16	usX, usY;
	wchar_t sString[9];
	UINT32	cnt;
	static wchar_t pStr[200];

	if ( gubSelectSMPanelToMerc != NOBODY )
	{
		// Give him the panel!
		SetSMPanelCurrentMerc( gubSelectSMPanelToMerc );
	}


	// ATE: Don't do anything if we are in stack popup and are refreshing stuff....
	if ( ( InItemStackPopup( ) || ( InKeyRingPopup( ) ) ) && (*pfDirty) == DIRTYLEVEL1 )
	{
		return;
	}

	if ( gfCheckForMouseOverItem )
	{
		if ( ( GetJA2Clock( ) - guiMouseOverItemTime ) > 100 )
		{
			if ( HandleCompatibleAmmoUI( gpSMCurrentMerc, (INT8)gbCheckForMouseOverItemPos, TRUE ) )
			{
				(*pfDirty) = DIRTYLEVEL2;
			}

			gfCheckForMouseOverItem = FALSE;
		}
	}

	HandleNewlyAddedItems( gpSMCurrentMerc, pfDirty );

	if ( InItemDescriptionBox( ) )
	{
		HandleItemDescriptionBox( pfDirty );
	}

	if ( *pfDirty == DIRTYLEVEL2 )
	{
		//if ( InItemStackPopup( ) )
		//{

		//}

		if ( InItemDescriptionBox( ) )
		{
			BltVideoObjectFromIndex( guiSAVEBUFFER, guiSMPanel, 0, INTERFACE_START_X, INV_INTERFACE_START_Y);
			RenderSoldierFace( gpSMCurrentMerc, SM_SELMERC_FACE_X, SM_SELMERC_FACE_Y, TRUE );


			// ATE: Need these lines here to fix flash bug with face selection box
			if ( gfSMDisableForItems )
			{
				BltVideoObjectFromIndex( guiSAVEBUFFER, guiSMObjects2, 0, SM_SELMERC_PLATE_X, SM_SELMERC_PLATE_Y);
				RestoreExternBackgroundRect( SM_SELMERC_PLATE_X, SM_SELMERC_PLATE_Y, SM_SELMERC_PLATE_WIDTH , SM_SELMERC_PLATE_HEIGHT );
			}
			else
			{
				if ( gusSelectedSoldier == gpSMCurrentMerc->ubID && gTacticalStatus.ubCurrentTeam == OUR_TEAM && OK_INTERRUPT_MERC(  gpSMCurrentMerc ) )
				{
					BltVideoObjectFromIndex( guiSAVEBUFFER, guiSMObjects, 0, SM_SELMERC_PLATE_X, SM_SELMERC_PLATE_Y);
					RestoreExternBackgroundRect( SM_SELMERC_PLATE_X, SM_SELMERC_PLATE_Y, SM_SELMERC_PLATE_WIDTH , SM_SELMERC_PLATE_HEIGHT );
				}
			}

			RenderItemDescriptionBox( );
		}
		else
		{

			BltVideoObjectFromIndex( guiSAVEBUFFER, guiSMPanel, 0, INTERFACE_START_X, INV_INTERFACE_START_Y);

			RenderInvBodyPanel( gpSMCurrentMerc, SM_BODYINV_X, SM_BODYINV_Y );

			// CHECK FOR PANEL STUFF / DEATHS / CLOSURES
			CheckForFacePanelStartAnims( gpSMCurrentMerc, SM_SELMERC_FACE_X, SM_SELMERC_FACE_Y );

			// Hitlight

			if ( gfSMDisableForItems )
			{
				BltVideoObjectFromIndex( guiSAVEBUFFER, guiSMObjects2, 0, SM_SELMERC_PLATE_X, SM_SELMERC_PLATE_Y);
				RestoreExternBackgroundRect( SM_SELMERC_PLATE_X, SM_SELMERC_PLATE_Y, SM_SELMERC_PLATE_WIDTH , SM_SELMERC_PLATE_HEIGHT );
			}
			else
			{
				if ( gusSelectedSoldier == gpSMCurrentMerc->ubID && gTacticalStatus.ubCurrentTeam == OUR_TEAM && OK_INTERRUPT_MERC(  gpSMCurrentMerc ) )
				{
					BltVideoObjectFromIndex( guiSAVEBUFFER, guiSMObjects, 0, SM_SELMERC_PLATE_X, SM_SELMERC_PLATE_Y);
					RestoreExternBackgroundRect( SM_SELMERC_PLATE_X, SM_SELMERC_PLATE_Y, SM_SELMERC_PLATE_WIDTH , SM_SELMERC_PLATE_HEIGHT );
				}
			}

			// Render faceplate
			//BltVideoObjectFromIndex( guiSAVEBUFFER, guiSMObjects2, 1, SM_SELMERC_NAMEPLATE_X, SM_SELMERC_NAMEPLATE_Y);
			//RestoreExternBackgroundRect( SM_SELMERC_NAMEPLATE_X, SM_SELMERC_NAMEPLATE_Y, SM_SELMERC_NAMEPLATE_WIDTH, SM_SELMERC_NAMEPLATE_HEIGHT );

			// Blit position
			//BltVideoObjectFromIndex( guiSAVEBUFFER, guiSMObjects, gbSMCurStanceObj, SM_POSITIONB_X, SM_POSITIONB_Y);
			//RestoreExternBackgroundRect( SM_POSITIONB_X, SM_POSITIONB_Y, SM_POSITIONB_WIDTH , SM_POSITIONB_HEIGHT );


			SetFont( BLOCKFONT2 );


			// Render Values for stats!
			// Set font drawing to saved buffer
			SetFontDestBuffer(guiSAVEBUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

			SetFontBackground( FONT_MCOLOR_BLACK );
			SetFontForeground( STATS_TITLE_FONT_COLOR );
			for( cnt = 0; cnt < 5; cnt++ )
			{
				mprintf( 92, ( INT16 )( 347 + cnt * 10 ), pShortAttributeStrings[ cnt  ] );
				mprintf( 137, ( INT16 )( 347 + cnt * 10 ), pShortAttributeStrings[ cnt + 5 ] );
			}

				mprintf( SM_ARMOR_LABEL_X - StringPixLength( pInvPanelTitleStrings[0], BLOCKFONT2 ) / 2, SM_ARMOR_LABEL_Y, pInvPanelTitleStrings[ 0 ] );
			mprintf( SM_ARMOR_PERCENT_X, SM_ARMOR_PERCENT_Y, L"%%" );

			mprintf( SM_WEIGHT_LABEL_X - StringPixLength( pInvPanelTitleStrings[1], BLOCKFONT2 ), SM_WEIGHT_LABEL_Y, pInvPanelTitleStrings[ 1 ] );
			mprintf( SM_WEIGHT_PERCENT_X, SM_WEIGHT_PERCENT_Y, L"%%" );

			mprintf( SM_CAMMO_LABEL_X - StringPixLength( pInvPanelTitleStrings[2], BLOCKFONT2 ), SM_CAMMO_LABEL_Y, pInvPanelTitleStrings[ 2 ] );
			mprintf( SM_CAMMO_PERCENT_X, SM_CAMMO_PERCENT_Y, L"%%" );

			UpdateStatColor( gpSMCurrentMerc-> uiChangeAgilityTime, ( BOOLEAN ) ( gpSMCurrentMerc->usValueGoneUp & AGIL_INCREASE? TRUE: FALSE ) );

			swprintf( sString, lengthof(sString), L"%2d", gpSMCurrentMerc->bAgility );
			FindFontRightCoordinates(SM_AGI_X, SM_AGI_Y ,SM_STATS_WIDTH ,SM_STATS_HEIGHT ,sString, BLOCKFONT2, &usX, &usY);
			mprintf( usX, usY , sString );

			UpdateStatColor( gpSMCurrentMerc-> uiChangeDexterityTime,( BOOLEAN ) ( gpSMCurrentMerc->usValueGoneUp & DEX_INCREASE? TRUE: FALSE )  );

			swprintf( sString, lengthof(sString), L"%2d", gpSMCurrentMerc->bDexterity );
			FindFontRightCoordinates(SM_DEX_X, SM_DEX_Y ,SM_STATS_WIDTH ,SM_STATS_HEIGHT ,sString, BLOCKFONT2, &usX, &usY);
			mprintf( usX, usY , sString );

			UpdateStatColor( gpSMCurrentMerc-> uiChangeStrengthTime, ( BOOLEAN )( gpSMCurrentMerc->usValueGoneUp & STRENGTH_INCREASE?TRUE: FALSE ) );

			swprintf( sString, lengthof(sString), L"%2d", gpSMCurrentMerc->bStrength );
			FindFontRightCoordinates(SM_STR_X, SM_STR_Y ,SM_STATS_WIDTH ,SM_STATS_HEIGHT ,sString, BLOCKFONT2, &usX, &usY);
			mprintf( usX, usY , sString );

			UpdateStatColor( gpSMCurrentMerc-> uiChangeLeadershipTime, ( BOOLEAN )( gpSMCurrentMerc->usValueGoneUp & LDR_INCREASE? TRUE: FALSE )  );

			swprintf( sString, lengthof(sString), L"%2d", gpSMCurrentMerc->bLeadership );
			FindFontRightCoordinates(SM_CHAR_X, SM_CHAR_Y ,SM_STATS_WIDTH ,SM_STATS_HEIGHT ,sString, BLOCKFONT2, &usX, &usY);
			mprintf( usX, usY , sString );

			UpdateStatColor( gpSMCurrentMerc-> uiChangeWisdomTime,( BOOLEAN ) ( gpSMCurrentMerc->usValueGoneUp & WIS_INCREASE? TRUE: FALSE )  );

			swprintf( sString, lengthof(sString), L"%2d", gpSMCurrentMerc->bWisdom );
			FindFontRightCoordinates(SM_WIS_X, SM_WIS_Y ,SM_STATS_WIDTH ,SM_STATS_HEIGHT ,sString, BLOCKFONT2, &usX, &usY);
			mprintf( usX, usY , sString );

			UpdateStatColor( gpSMCurrentMerc-> uiChangeLevelTime, ( BOOLEAN ) ( gpSMCurrentMerc->usValueGoneUp & LVL_INCREASE? TRUE: FALSE )  );

			swprintf( sString, lengthof(sString), L"%2d", gpSMCurrentMerc->bExpLevel );
			FindFontRightCoordinates(SM_EXPLVL_X, SM_EXPLVL_Y ,SM_STATS_WIDTH ,SM_STATS_HEIGHT ,sString, BLOCKFONT2, &usX, &usY);
			mprintf( usX, usY , sString );

			UpdateStatColor( gpSMCurrentMerc-> uiChangeMarksmanshipTime, ( BOOLEAN ) ( gpSMCurrentMerc->usValueGoneUp & MRK_INCREASE? TRUE: FALSE )  );

			swprintf( sString, lengthof(sString), L"%2d", gpSMCurrentMerc->bMarksmanship );
			FindFontRightCoordinates(SM_MRKM_X, SM_MRKM_Y ,SM_STATS_WIDTH ,SM_STATS_HEIGHT ,sString, BLOCKFONT2, &usX, &usY);
			mprintf( usX, usY , sString );

			UpdateStatColor( gpSMCurrentMerc-> uiChangeExplosivesTime, ( BOOLEAN ) ( gpSMCurrentMerc->usValueGoneUp & EXP_INCREASE? TRUE: FALSE )  );

			swprintf( sString, lengthof(sString), L"%2d", gpSMCurrentMerc->bExplosive );
			FindFontRightCoordinates(SM_EXPL_X, SM_EXPL_Y ,SM_STATS_WIDTH ,SM_STATS_HEIGHT ,sString, BLOCKFONT2, &usX, &usY);
			mprintf( usX, usY , sString );

			UpdateStatColor( gpSMCurrentMerc-> uiChangeMechanicalTime, ( BOOLEAN ) ( gpSMCurrentMerc->usValueGoneUp & MECH_INCREASE ? TRUE: FALSE )  );

			swprintf( sString, lengthof(sString), L"%2d", gpSMCurrentMerc->bMechanical );
			FindFontRightCoordinates(SM_MECH_X, SM_MECH_Y ,SM_STATS_WIDTH ,SM_STATS_HEIGHT ,sString, BLOCKFONT2, &usX, &usY);
			mprintf( usX, usY , sString );

			UpdateStatColor( gpSMCurrentMerc-> uiChangeMedicalTime, ( BOOLEAN ) ( gpSMCurrentMerc->usValueGoneUp & MED_INCREASE? TRUE: FALSE )  );

			swprintf( sString, lengthof(sString), L"%2d", gpSMCurrentMerc->bMedical );
			FindFontRightCoordinates(SM_MED_X, SM_MED_Y ,SM_STATS_WIDTH ,SM_STATS_HEIGHT ,sString, BLOCKFONT2, &usX, &usY);
			mprintf( usX, usY , sString );

			if ( gpSMCurrentMerc->bLife >= OKLIFE )
			{
				SetFontBackground( FONT_MCOLOR_BLACK );
				SetFontForeground( STATS_TEXT_FONT_COLOR );
			}
			else
			{
				SetFontBackground( FONT_MCOLOR_BLACK );
				SetFontForeground( FONT_MCOLOR_DKGRAY );
			}

			// Display armour value!
			swprintf( sString, lengthof(sString), L"%3d", ArmourPercent( gpSMCurrentMerc ) );
			FindFontRightCoordinates(SM_ARMOR_X, SM_ARMOR_Y ,SM_PERCENT_WIDTH ,SM_PERCENT_HEIGHT ,sString, BLOCKFONT2, &usX, &usY);
			mprintf( usX, usY , sString );

			// Display wieght value!
			swprintf( sString, lengthof(sString), L"%3d", CalculateCarriedWeight( gpSMCurrentMerc ) );
			FindFontRightCoordinates(SM_WEIGHT_X, SM_WEIGHT_Y ,SM_PERCENT_WIDTH ,SM_PERCENT_HEIGHT ,sString, BLOCKFONT2, &usX, &usY);
			mprintf( usX, usY , sString );

			// Display cammo value!
			swprintf( sString, lengthof(sString), L"%3d", gpSMCurrentMerc->bCamo );
			FindFontRightCoordinates(SM_CAMMO_X, SM_CAMMO_Y ,SM_PERCENT_WIDTH ,SM_PERCENT_HEIGHT ,sString, BLOCKFONT2, &usX, &usY);
			mprintf( usX, usY , sString );


			// reset to frame buffer!
			SetFontDestBuffer(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

			RestoreExternBackgroundRect(INTERFACE_START_X, INV_INTERFACE_START_Y, SCREEN_WIDTH - INTERFACE_START_X, SCREEN_HEIGHT - INV_INTERFACE_START_Y);


			RenderSoldierFace( gpSMCurrentMerc, SM_SELMERC_FACE_X, SM_SELMERC_FACE_Y, TRUE );

		}

		// Render Name!
		SetFont( BLOCKFONT2 );

		if ( gpSMCurrentMerc->bStealthMode )
		{
			SetFontBackground( FONT_MCOLOR_BLACK );
			SetFontForeground( FONT_MCOLOR_LTYELLOW );
		}
		else
		{
			SetFontBackground( FONT_MCOLOR_BLACK );
			SetFontForeground( FONT_MCOLOR_LTGRAY );
		}

		RestoreExternBackgroundRect( SM_SELMERCNAME_X, SM_SELMERCNAME_Y, SM_SELMERCNAME_WIDTH, SM_SELMERCNAME_HEIGHT );
		FindFontCenterCoordinates(SM_SELMERCNAME_X, SM_SELMERCNAME_Y, SM_SELMERCNAME_WIDTH, SM_SELMERCNAME_HEIGHT, gpSMCurrentMerc->name, BLOCKFONT2, &sFontX, &sFontY);
		mprintf(sFontX, sFontY, L"%ls", gpSMCurrentMerc->name);
	}

	if ( *pfDirty != DIRTYLEVEL0 )
	{
		// UPdate stats!
		if ( gpSMCurrentMerc->bLife != 0 )
		{
      if ( gpSMCurrentMerc->uiStatusFlags & SOLDIER_VEHICLE )
      {
			  swprintf( pStr, lengthof(pStr), TacticalStr[ VEHICLE_VITAL_STATS_POPUPTEXT ], gpSMCurrentMerc->bLife, gpSMCurrentMerc->bLifeMax, gpSMCurrentMerc->bBreath, gpSMCurrentMerc->bBreathMax );
			  SetRegionFastHelpText( &(gSM_SELMERCBarsRegion), pStr );
      }
      else if ( gpSMCurrentMerc->uiStatusFlags & SOLDIER_ROBOT )
      {
			  swprintf( pStr, lengthof(pStr), gzLateLocalizedString[ 16 ], gpSMCurrentMerc->bLife, gpSMCurrentMerc->bLifeMax );
			  SetRegionFastHelpText( &(gTEAM_BarsRegions[ cnt ]), pStr );
      }
      else
      {
			  const wchar_t* Morale = GetMoraleString(gpSMCurrentMerc);
			  swprintf(pStr, lengthof(pStr), TacticalStr[MERC_VITAL_STATS_POPUPTEXT], gpSMCurrentMerc->bLife, gpSMCurrentMerc->bLifeMax, gpSMCurrentMerc->bBreath, gpSMCurrentMerc->bBreathMax, Morale);
			  SetRegionFastHelpText( &(gSM_SELMERCBarsRegion), pStr );
      }
		}
		else
		{
			SetRegionFastHelpText( &(gSM_SELMERCBarsRegion), L"" );
		}

		// display AP
		if ( !( gpSMCurrentMerc->uiStatusFlags & SOLDIER_DEAD ) )
		{
			if ( gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT ) && gpSMCurrentMerc->bLife >= OKLIFE )
			{
				SetFont( TINYFONT1 );
				//if ( gpSMCurrentMerc->sLastTarget != NOWHERE && !EnoughPoints( gpSMCurrentMerc, MinAPsToAttack( gpSMCurrentMerc, gpSMCurrentMerc->sLastTarget, FALSE ), 0, FALSE ) || GetUIApsToDisplay( gpSMCurrentMerc ) < 0 )
				if ( !EnoughPoints( gpSMCurrentMerc, MinAPsToAttack( gpSMCurrentMerc, gpSMCurrentMerc->sLastTarget, FALSE ), 0, FALSE ) || GetUIApsToDisplay( gpSMCurrentMerc ) < 0 )
				{
					SetFontBackground( FONT_MCOLOR_BLACK );
					SetFontForeground( FONT_MCOLOR_DKRED );
				}
				else
				{
					if ( MercUnderTheInfluence( gpSMCurrentMerc ) )
					{
						SetFontBackground( FONT_MCOLOR_BLACK );
						SetFontForeground( FONT_MCOLOR_LTBLUE );
					}
					else if ( gpSMCurrentMerc->bStealthMode )
					{
						SetFontBackground( FONT_MCOLOR_BLACK );
						SetFontForeground( FONT_MCOLOR_LTYELLOW );
					}
					else
					{
						SetFontBackground( FONT_MCOLOR_BLACK );
						SetFontForeground( FONT_MCOLOR_LTGRAY );
					}
				}

				RestoreExternBackgroundRect(SM_SELMERC_AP_X, SM_SELMERC_AP_Y, SM_SELMERC_AP_WIDTH, SM_SELMERC_AP_HEIGHT);
				VarFindFontCenterCoordinates( SM_SELMERC_AP_X, SM_SELMERC_AP_Y, SM_SELMERC_AP_WIDTH, SM_SELMERC_AP_HEIGHT, TINYFONT1, &sFontX, &sFontY, L"%d", GetUIApsToDisplay( gpSMCurrentMerc ) );
				mprintf(sFontX, sFontY, L"%d", GetUIApsToDisplay(gpSMCurrentMerc));
			}

			DrawSoldierUIBars(gpSMCurrentMerc, SM_SELMERC_HEALTH_X, SM_SELMERC_HEALTH_Y, TRUE, FRAME_BUFFER);
		}

	}

	UpdateSMPanel( );


	//HandlePanelFaceAnimations( gpSMCurrentMerc );

	HandleSoldierFaceFlash( gpSMCurrentMerc, SM_SELMERC_FACE_X, SM_SELMERC_FACE_Y );

	// Render items in guy's hand!
	HandleRenderInvSlots( gpSMCurrentMerc, *pfDirty );

	if ( gfSMDisableForItems && (*pfDirty) != DIRTYLEVEL0 )
	{
		UINT8	 *pDestBuf;
		UINT32 uiDestPitchBYTES;
		SGPRect ClipRect;

		ClipRect.iLeft	 = 87;
		ClipRect.iRight  = 536;
		ClipRect.iTop		 = INV_INTERFACE_START_Y;
		ClipRect.iBottom = SCREEN_HEIGHT;
		pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
		Blt16BPPBufferHatchRect( (UINT16*)pDestBuf, uiDestPitchBYTES, &ClipRect );
		UnLockVideoSurface( FRAME_BUFFER );

	}

}


static void UpdateStatColor(UINT32 uiTimer, BOOLEAN fIncrease)
{
	if ( gpSMCurrentMerc->bLife >= OKLIFE )
	{
		if( ( GetJA2Clock()  < CHANGE_STAT_RECENTLY_DURATION + uiTimer) && ( uiTimer != 0 ) )
		{
			if( fIncrease )
			{
				SetFontForeground( FONT_LTGREEN );
			}
			else
			{
				SetFontForeground( FONT_RED );
			}
		}
		else
		{
			SetFontBackground( FONT_MCOLOR_BLACK );
			SetFontForeground( STATS_TEXT_FONT_COLOR );
		}
	}
	else
	{
		SetFontBackground( FONT_MCOLOR_BLACK );
		SetFontForeground( FONT_MCOLOR_DKGRAY );
	}
}


static void SMInvMoveCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT32 uiHandPos;

	uiHandPos = MSYS_GetRegionUserData( pRegion, 0 );

	if ( gpSMCurrentMerc->inv[ uiHandPos ].usItem == NOTHING )
		return;

	if (iReason == MSYS_CALLBACK_REASON_GAIN_MOUSE)
	{
		if ( gpItemPointer == NULL )
		{
			// Setup a timer....
			guiMouseOverItemTime = GetJA2Clock( );
			gfCheckForMouseOverItem = TRUE;
			gbCheckForMouseOverItemPos = (INT8)uiHandPos;
		}
	}
	if (iReason == MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		//gfSM_HandInvDispText[ uiHandPos ] = 1;
		if ( gpItemPointer == NULL )
		{
			HandleCompatibleAmmoUI( gpSMCurrentMerc, (INT8)uiHandPos, FALSE );
			gfCheckForMouseOverItem = FALSE;
			fInterfacePanelDirty		= DIRTYLEVEL2;
		}
	}
}


static void InvPanelButtonClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
}


static void SMInvMoveCammoCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason == MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		// Setup a timer....
		guiMouseOverItemTime = GetJA2Clock( );
		gfCheckForMouseOverItem = TRUE;
		gbCheckForMouseOverItemPos = NO_SLOT;
	}
	if (iReason == MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		//gfSM_HandInvDispText[ uiHandPos ] = 1;
		HandleCompatibleAmmoUI( gpSMCurrentMerc, (INT8)NO_SLOT, FALSE );
		gfCheckForMouseOverItem = FALSE;
	}
}


static void SMInvClickCamoCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	//UINT16 usNewItemIndex;
	UINT8	 ubSrcID, ubDestID;
  BOOLEAN fGoodAPs;

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		// Apply camo ( if we have something in cursor... )
		// If we do not have an item in hand, start moving it
		if ( gpItemPointer != NULL )
		{
			// ATE: OK, get source, dest guy if different... check for and then charge appropriate APs
			ubSrcID  = gpSMCurrentMerc->ubID;
			ubDestID = gpItemPointerSoldier->ubID;

			//if ( ubSrcID == ubDestID )
			{
				// We are doing this ourselve, continue
				if ( gpSMCurrentMerc->bLife >= CONSCIOUSNESS )
				{
					//usNewItemIndex = gpItemPointer->usItem;

					// Try to apply camo....
					if ( ApplyCammo( gpSMCurrentMerc, gpItemPointer, &fGoodAPs ) )
					{
            if ( fGoodAPs )
            {
						  // Dirty
						  fInterfacePanelDirty = DIRTYLEVEL2;

						  // Check if it's the same now!
						  if ( gpItemPointer->ubNumberOfObjects == 0 )
						  {
							  gbCompatibleApplyItem = FALSE;
							  EndItemPointer( );
						  }

						  // Say OK acknowledge....
						  DoMercBattleSound( gpSMCurrentMerc, BATTLE_SOUND_COOL1 );
            }
					}
					else if ( ApplyCanteen( gpSMCurrentMerc, gpItemPointer, &fGoodAPs ) )
					{
						// Dirty
            if ( fGoodAPs )
            {
						  fInterfacePanelDirty = DIRTYLEVEL2;

						  // Check if it's the same now!
						  if ( gpItemPointer->ubNumberOfObjects == 0 )
						  {
							  gbCompatibleApplyItem = FALSE;
							  EndItemPointer( );
						  }
            }
					}
					else if ( ApplyElixir( gpSMCurrentMerc, gpItemPointer, &fGoodAPs ) )
					{
            if ( fGoodAPs )
            {
  						// Dirty
						  fInterfacePanelDirty = DIRTYLEVEL2;

						  // Check if it's the same now!
						  if ( gpItemPointer->ubNumberOfObjects == 0 )
						  {
							  gbCompatibleApplyItem = FALSE;
							  EndItemPointer( );
						  }

						  // Say OK acknowledge....
   					  DoMercBattleSound( gpSMCurrentMerc, BATTLE_SOUND_COOL1 );
            }
					}
					else if ( ApplyDrugs( gpSMCurrentMerc, gpItemPointer ) )
					{
						// Dirty
						fInterfacePanelDirty = DIRTYLEVEL2;

						// Check if it's the same now!
						if ( gpItemPointer->ubNumberOfObjects == 0 )
						{
							gbCompatibleApplyItem = FALSE;
							EndItemPointer( );
						}

						/*
						// COMMENTED OUT DUE TO POTENTIAL SERIOUS PROBLEMS WITH STRATEGIC EVENT SYSTEM

						if ( gpSMCurrentMerc->ubProfile == LARRY_NORMAL )
						{
							// Larry's taken something!
							gpSMCurrentMerc = SwapLarrysProfiles( gpSMCurrentMerc );
						}
						*/


						// Say OK acknowledge....
						DoMercBattleSound( gpSMCurrentMerc, BATTLE_SOUND_COOL1 );

					}
					else
					{
						// Send message
						ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ CANNOT_DO_INV_STUFF_STR ] );
					}
				}
			}
		}
	}
}


BOOLEAN HandleNailsVestFetish( SOLDIERTYPE *pSoldier, UINT32 uiHandPos, UINT16 usReplaceItem )
{
	BOOLEAN fRefuse = FALSE;

	if (pSoldier->ubProfile == NAILS)
	{
		// if this the VEST POS?
		if ( uiHandPos == VESTPOS )
		{
			// Are we trying to pick it up?
			if ( usReplaceItem == NOTHING )
			{
				fRefuse = TRUE;
			}
			else
			{
				// Do we have nothing or the leather vest or kevlar leather vest?
				if ( usReplaceItem == LEATHER_JACKET ||
						 usReplaceItem == LEATHER_JACKET_W_KEVLAR ||
						 usReplaceItem == LEATHER_JACKET_W_KEVLAR_18 ||
						 usReplaceItem == LEATHER_JACKET_W_KEVLAR_Y ||
						 usReplaceItem == COMPOUND18 ||
						 usReplaceItem == JAR_QUEEN_CREATURE_BLOOD )
				{
					// This is good....
					fRefuse = FALSE;
				}
				else
				{
					fRefuse = TRUE;
				}
			}

			if ( fRefuse )
			{
				// Say quote!
				TacticalCharacterDialogue( pSoldier, 61 );
				return( TRUE );
			}
		}
	}

	return( FALSE );
}


static BOOLEAN UIHandleItemPlacement(UINT8 ubHandPos, UINT16 usOldItemIndex, UINT16 usNewItemIndex, BOOLEAN fDeductPoints)
{
	if ( _KeyDown(CTRL) )
	{
		CleanUpStack( &( gpSMCurrentMerc->inv[ ubHandPos ] ), gpItemPointer );
		if ( gpItemPointer->ubNumberOfObjects == 0 )
		{
			EndItemPointer( );
		}
		return( TRUE );
	}

	// Try to place here
	if ( PlaceObject( gpSMCurrentMerc, ubHandPos, gpItemPointer ) )
	{
		if ( fDeductPoints )
		{
			// Deduct points
			if ( gpItemPointerSoldier->bLife >= CONSCIOUSNESS )
			{
				DeductPoints( gpItemPointerSoldier,  2, 0 );
			}
			if ( gpSMCurrentMerc->bLife >= CONSCIOUSNESS )
			{
				DeductPoints( gpSMCurrentMerc,  2, 0 );
			}
		}

		HandleTacticalEffectsOfEquipmentChange( gpSMCurrentMerc, ubHandPos, usOldItemIndex, usNewItemIndex );

		// Dirty
		fInterfacePanelDirty = DIRTYLEVEL2;

		// Check if cursor is empty now
		if ( gpItemPointer->ubNumberOfObjects == 0 )
		{
			EndItemPointer( );
		}

		if ( gpItemPointerSoldier != gpSMCurrentMerc )
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_ITEM_PASSED_TO_MERC ], ShortItemNames[ usNewItemIndex ], gpSMCurrentMerc->name );
		}

		// UPDATE ITEM POINTER.....
		gpItemPointerSoldier = gpSMCurrentMerc;

		if ( gpItemPointer != NULL )
		{
			ReevaluateItemHatches( gpSMCurrentMerc, FALSE );
		}

		// Set cursor back to normal mode...
		guiPendingOverrideEvent = A_CHANGE_TO_MOVE;

		return( TRUE );

	}
	else
	{
		return( FALSE );
	}

}


static void MergeMessageBoxCallBack(UINT8 ubExitValue);


static void SMInvClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT32 uiHandPos;
	// Copyies of values
	UINT16 usOldItemIndex, usNewItemIndex;
	UINT16 usItemPrevInItemPointer;
	BOOLEAN fNewItem = FALSE;
	static BOOLEAN	fRightDown = FALSE;
	static BOOLEAN	fLeftDown = FALSE;


	uiHandPos = MSYS_GetRegionUserData( pRegion, 0 );

	if ( (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
	{
		return;
	}

	//if we are in the shop keeper interface
	if( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE )
	{
		// and this inventory slot is hatched out
		if( ShouldSoldierDisplayHatchOnItem( gpSMCurrentMerc->ubProfile, (INT16)uiHandPos ) )
		{
			// it means that item is a copy of one in the player's offer area, so we treat it as if the slot was empty (ignore)
			// if the cursor has an item in it, we still ignore the click, because handling swaps in this situation would be
			// ugly, we'd have to the the swap, then make the bOwnerSlot of the item just picked up a -1 in its offer area spot.
			return;
		}
	}


	//if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	//{
	//	fLeftDown = TRUE;
	//}
	//else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP && fLeftDown )
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		fLeftDown = FALSE;

		// If we do not have an item in hand, start moving it
		if ( gpItemPointer == NULL )
		{

			// Return if empty
			if ( gpSMCurrentMerc->inv[ uiHandPos ].usItem == NOTHING )
				return;

			if ( gpSMCurrentMerc->ubID != gusSelectedSoldier )
			{
				SelectSoldier( gpSMCurrentMerc->ubID, FALSE, FALSE );
			}

			// OK, check if this is Nails, and we're in the vest position , don't allow it to come off....
			if ( HandleNailsVestFetish( gpSMCurrentMerc, uiHandPos, NOTHING ) )
			{
				return;
			}

			if ( _KeyDown(CTRL) )
			{
				CleanUpStack( &( gpSMCurrentMerc->inv[ uiHandPos ] ), NULL );
				return;
			}

			// Turn off new item glow!
			gpSMCurrentMerc->bNewItemCount[ uiHandPos ] = 0;

			usOldItemIndex = gpSMCurrentMerc->inv[ uiHandPos ].usItem;

			// move item into the mouse cursor
			BeginItemPointer( gpSMCurrentMerc, (UINT8)uiHandPos );

			//if we are in the shopkeeper interface
			if( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE )
			{
				// pick up item from regular inventory slot into cursor OR try to sell it ( unless CTRL is held down )
				BeginSkiItemPointer( PLAYERS_INVENTORY, ( INT8 ) uiHandPos, ( BOOLEAN )!gfKeyState[ CTRL ] );
			}

			HandleTacticalEffectsOfEquipmentChange( gpSMCurrentMerc, uiHandPos, usOldItemIndex, NOTHING );

			// HandleCompatibleAmmoUI( gpSMCurrentMerc, (INT8)uiHandPos, FALSE );
		}
		else	// item in cursor
		{
			UINT8			ubSrcID, ubDestID;
			BOOLEAN		fOKToGo = FALSE;
			BOOLEAN		fDeductPoints = FALSE;

			// ATE: OK, get source, dest guy if different... check for and then charge appropriate APs
			ubSrcID  = gpSMCurrentMerc->ubID;
			ubDestID = gpItemPointerSoldier->ubID;

			if ( ubSrcID == ubDestID )
			{
				// We are doing this ourselve, continue
				fOKToGo = TRUE;
			}
			else
			{
				// These guys are different....
				fDeductPoints = TRUE;

				// First check points for src guy
				if ( gpItemPointerSoldier->bLife >= CONSCIOUSNESS )
				{
					if ( EnoughPoints( gpItemPointerSoldier, 3, 0, TRUE ) )
					{
						fOKToGo = TRUE;
					}
				}
				else
				{
					fOKToGo = TRUE;
				}

				// Should we go on?
				if ( fOKToGo )
				{
					if ( gpSMCurrentMerc->bLife >= CONSCIOUSNESS )
					{
						if ( EnoughPoints( gpSMCurrentMerc, 3, 0, TRUE ) )
						{
							fOKToGo = TRUE;
						}
						else
						{
							fOKToGo = FALSE;
						}
					}
				}
			}

			if ( fOKToGo )
			{
				// OK, check if this is Nails, and we're in the vest position , don't allow it to come off....
				if ( HandleNailsVestFetish( gpSMCurrentMerc, uiHandPos, gpItemPointer->usItem ) )
				{
					return;
				}

				usOldItemIndex = gpSMCurrentMerc->inv[ uiHandPos ].usItem;
				usNewItemIndex = gpItemPointer->usItem;

				if ( uiHandPos == HANDPOS || uiHandPos == SECONDHANDPOS || uiHandPos == HELMETPOS || uiHandPos == VESTPOS || uiHandPos == LEGPOS )
				{
					//if ( ValidAttachmentClass( usNewItemIndex, usOldItemIndex ) )
					if ( ValidAttachment( usNewItemIndex, usOldItemIndex ) )
					{
						// it's an attempt to attach; bring up the inventory panel
						if ( !InItemDescriptionBox( ) )
						{
							InitItemDescriptionBox( gpSMCurrentMerc, (UINT8)uiHandPos, SM_ITEMDESC_START_X, SM_ITEMDESC_START_Y, 0 );
						}
						return;
					}
					else if ( ValidMerge( usNewItemIndex, usOldItemIndex ) )
					{
						// bring up merge requestor
						gubHandPos = (UINT8) uiHandPos;
						gusOldItemIndex = usOldItemIndex;
						gusNewItemIndex = usNewItemIndex;
						gfDeductPoints = fDeductPoints;

						if( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE )
						{
							//the only way to merge items is to pick them up.  In SKI when you pick up an item, the cursor is
							//locked in a region, free it up.
							FreeMouseCursor();

							DoMessageBox( MSG_BOX_BASIC_STYLE, Message[ STR_MERGE_ITEMS ], SHOPKEEPER_SCREEN, ( UINT8 )MSG_BOX_FLAG_YESNO, MergeMessageBoxCallBack, NULL );
						}
						else
							DoMessageBox( MSG_BOX_BASIC_STYLE, Message[ STR_MERGE_ITEMS ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_YESNO, MergeMessageBoxCallBack, NULL );
						return;
					}
					// else handle normally
				}


				// remember the item type currently in the item pointer
				usItemPrevInItemPointer = gpItemPointer->usItem;

				if( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE )
				{
					// If it's just been purchased or repaired, mark it as a "new item"
					fNewItem = ( BOOLEAN ) ( gMoveingItem.uiFlags & ( ARMS_INV_JUST_PURCHASED | ARMS_INV_ITEM_REPAIRED ) );
				}

				// try to place the item in the cursor into this inventory slot
				if ( UIHandleItemPlacement( (UINT8) uiHandPos, usOldItemIndex, usNewItemIndex, fDeductPoints ) )
				{
					// it worked!  if we're in the SKI...
					if( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE )
					{
						SetNewItem( gpSMCurrentMerc, ( UINT8 ) uiHandPos, fNewItem );

						// and the cursor is now empty
						if( gpItemPointer == NULL )
						{
							// clean up
							memset( &gMoveingItem, 0, sizeof( INVENTORY_IN_SLOT ) );
							SetSkiCursor( CURSOR_NORMAL );
						}
						else
						{
							// if we're holding something else in the pointer now
							if ( usItemPrevInItemPointer != gpItemPointer->usItem )
							{
								// pick up item swapped out of inventory slot into cursor (don't try to sell)
								BeginSkiItemPointer( PLAYERS_INVENTORY, -1, FALSE );
							}
							else
							{
								// otherwise, leave the cursor as is, means more items were picked up at once than can be placed in this slot
								// we deal with this by leaving the remainder in the cursor, to be put down elsewhere using subsequent clicks
							}
						}
					}

					// Setup a timer....
					//guiMouseOverItemTime = GetJA2Clock( );
					//gfCheckForMouseOverItem = TRUE;
					//gbCheckForMouseOverItemPos = (INT8)uiHandPos;
				}

				/*
				// Try to place here
				if ( PlaceObject( gpSMCurrentMerc, (UINT8)uiHandPos, gpItemPointer ) )
				{

					if ( fDeductPoints )
					{
						// Deduct points
						if ( gpItemPointerSoldier->bLife >= CONSCIOUSNESS )
						{
							DeductPoints( gpItemPointerSoldier,  2, 0 );
						}
						if ( gpSMCurrentMerc->bLife >= CONSCIOUSNESS )
						{
							DeductPoints( gpSMCurrentMerc,  2, 0 );
						}
					}

					HandleTacticalEffectsOfEquipmentChange( gpSMCurrentMerc, uiHandPos, usOldItemIndex, usNewItemIndex );

					// Dirty
					fInterfacePanelDirty = DIRTYLEVEL2;

					// Check if it's the same now!
					if ( gpItemPointer->ubNumberOfObjects == 0 )
					{
						EndItemPointer( );
					}

					// Setup a timer....
					guiMouseOverItemTime = GetJA2Clock( );
					gfCheckForMouseOverItem = TRUE;
					gbCheckForMouseOverItemPos = (INT8)uiHandPos;

				}
				*/
			}
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		fRightDown = TRUE;
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP && fRightDown )
	{
		fRightDown = FALSE;

		// Return if empty
		if ( gpSMCurrentMerc->inv[ uiHandPos ].usItem == NOTHING )
			return;

		// CJC: OK, get source, dest guy if different, don't allow panels to be brought up
		/*
		if ( gpItemPointer && gpSMCurrentMerc->ubID != gpItemPointerSoldier->ubID )
		{
			return;
		}
		*/

		// Turn off new item glow!
		gpSMCurrentMerc->bNewItemCount[ uiHandPos ] = 0;

		// Some global stuff here - for esc, etc
		// Check for # of slots in item
		if( ( gpSMCurrentMerc->inv[ uiHandPos ].ubNumberOfObjects > 1 && ItemSlotLimit( gpSMCurrentMerc->inv[ uiHandPos ].usItem, (UINT8)uiHandPos ) > 0 ) && ( guiCurrentScreen != MAP_SCREEN ) )
		{
			if ( !InItemStackPopup( )  )
			{
				//InitItemStackPopup( gpSMCurrentMerc, (UINT8)uiHandPos, SM_ITEMDESC_START_X, SM_ITEMDESC_START_Y, SM_ITEMDESC_WIDTH, SM_ITEMDESC_HEIGHT );
				InitItemStackPopup(gpSMCurrentMerc, (UINT8)uiHandPos, 216, INV_INTERFACE_START_Y, 314, SCREEN_HEIGHT - INV_INTERFACE_START_Y);
			}
		}
		else
		{
			if ( !InItemDescriptionBox( ) )
			{
				InitItemDescriptionBox( gpSMCurrentMerc, (UINT8)uiHandPos, SM_ITEMDESC_START_X, SM_ITEMDESC_START_Y, 0 );
			}
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		fRightDown = FALSE;
		fLeftDown = FALSE;
	}

}


static void MergeMessageBoxCallBack(UINT8 ubExitValue)
{
	if ( ubExitValue == MSG_BOX_RETURN_YES )
	{
		AttachObject( gpItemPointerSoldier, &( gpSMCurrentMerc->inv[ gubHandPos ] ), gpItemPointer );

		// re-evaluate repairs
		gfReEvaluateEveryonesNothingToDo = TRUE;

		if (gpItemPointer->usItem == NOTHING)
		{
			// merge item consumed
			EndItemPointer();
			fInterfacePanelDirty = DIRTYLEVEL2;
		}
	}
	else
	{
		UIHandleItemPlacement( gubHandPos, gusOldItemIndex, gusNewItemIndex, gfDeductPoints );
	}
}


static void HandleMouseOverSoldierFaceForContMove(SOLDIERTYPE* pSoldier, BOOLEAN fOn)
{
	FACETYPE *pFace;
	INT16			sGridNo;

	if ( pSoldier == NULL )
	{
		return;
	}

	if ( fOn )
	{
		// Check if we are waiting to continue move...
		if ( CheckForMercContMove( pSoldier ) )
		{
			// Display 'cont' on face....
			// Get face
			pFace = &gFacesData[ pSoldier->iFaceIndex ];

			pFace->fDisplayTextOver = FACE_DRAW_TEXT_OVER;
			wcscpy( pFace->zDisplayText, TacticalStr[ CONTINUE_OVER_FACE_STR ] );

			sGridNo = pSoldier->sFinalDestination;

			if ( pSoldier->bGoodContPath )
			{
				sGridNo = pSoldier->sContPathLocation;
			}

			// While our mouse is here, draw a path!
			PlotPath( pSoldier, sGridNo, NO_COPYROUTE, PLOT, TEMPORARY, (UINT16)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints );
		}
	}
	else
	{
		// Remove 'cont' on face....
		// Get face
		pFace = &gFacesData[ pSoldier->iFaceIndex ];

		pFace->fDisplayTextOver = FACE_ERASE_TEXT_OVER;

		// Erase path!
		ErasePath( TRUE );
	}

	fInterfacePanelDirty = DIRTYLEVEL2;
}


static void SelectedMercButtonMoveCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if ( gpSMCurrentMerc == NULL )
	{
		return;
	}

	if (iReason & MSYS_CALLBACK_REASON_MOVE )
	{
		HandleMouseOverSoldierFaceForContMove( gpSMCurrentMerc, TRUE );
	}
	else if ( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		HandleMouseOverSoldierFaceForContMove( gpSMCurrentMerc, FALSE );
	}
}


static void SelectedMercButtonCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	SOLDIERTYPE *pVehicle;

	if ( gpSMCurrentMerc == NULL )
	{
		return;
	}

	//if we are in the shop keeper interface
	if( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE )
		return;

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// ATE: Don't if this guy can't....
		if ( !gfSMDisableForItems )
		{
			if ( gpSMCurrentMerc->uiStatusFlags & ( SOLDIER_DRIVER | SOLDIER_PASSENGER ) )
			{
				pVehicle = GetSoldierStructureForVehicle( gpSMCurrentMerc->iVehicleId );

				HandleLocateSelectMerc( pVehicle->ubID, 0 );
			}
			else
			{
				if ( CheckForMercContMove( gpSMCurrentMerc ) )
				{
					// Continue
					ContinueMercMovement( gpSMCurrentMerc );
					ErasePath( TRUE );
				}
				else
				{
					HandleLocateSelectMerc( gpSMCurrentMerc->ubID, 0 );
				}
			}
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		// ATE: Cannot get out by right clicking...
		//if ( gpItemPointer == NULL )
		{
			//if ( !gfSMDisableForItems || ( gfSMDisableForItems && gpItemPointer == NULL ) )
			{
				// Delete desc
				if ( InItemDescriptionBox( ) )
				{
					DeleteItemDescriptionBox( );
				}

				gfSwitchPanel = TRUE;
				gbNewPanel		= TEAM_PANEL;
			}
		}
	}
}


static void SelectedMercEnemyIndicatorCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if ( gpSMCurrentMerc == NULL )
	{
		return;
	}

	//if we are in the shop keeper interface
	if( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE )
		return;

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// ATE: Don't if this guy can't....
		if ( !gfSMDisableForItems )
		{
			if ( gpSMCurrentMerc->uiStatusFlags & ( SOLDIER_DRIVER | SOLDIER_PASSENGER ) )
			{
			}
			else
			{
				if ( gpSMCurrentMerc->bOppCnt > 0 )
				{
					CycleVisibleEnemies( gpSMCurrentMerc );
				}
				else
				{
					SelectedMercButtonCallback( pRegion, iReason );
				}
			}
		}
	}
}


static void BtnStanceUpCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		INT8 bNewStance = gAnimControl[gpSMCurrentMerc->usAnimState].ubEndHeight;
		switch (bNewStance)
		{
			case ANIM_CROUCH: bNewStance = ANIM_STAND;  break;
			case ANIM_PRONE:  bNewStance = ANIM_CROUCH; break;
		}
		UIHandleSoldierStanceChange(gpSMCurrentMerc->ubID, bNewStance);
	}
}


void BtnUpdownCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// Change interface level via HandleUI handler
		UIHandleChangeLevel(NULL);

		// Remember soldier's new value
		gpSMCurrentMerc->bUIInterfaceLevel = gsInterfaceLevel;
	}
}


static void BtnClimbCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		BOOLEAN fNearHeigherLevel;
		BOOLEAN fNearLowerLevel;
		GetMercClimbDirection(gpSMCurrentMerc->ubID, &fNearLowerLevel, &fNearHeigherLevel);
		if (fNearLowerLevel)   BeginSoldierClimbDownRoof(gpSMCurrentMerc);
		if (fNearHeigherLevel) BeginSoldierClimbUpRoof(gpSMCurrentMerc);

		INT8 bDirection;
		if (FindFenceJumpDirection(gpSMCurrentMerc, gpSMCurrentMerc->sGridNo, gpSMCurrentMerc->bDirection, &bDirection))
		{
			BeginSoldierClimbFence(gpSMCurrentMerc);
		}
	}
}


static void BtnStanceDownCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		INT8 bNewStance = gAnimControl[gpSMCurrentMerc->usAnimState].ubEndHeight;
		switch (bNewStance)
		{
			case ANIM_STAND:  bNewStance = ANIM_CROUCH; break;
			case ANIM_CROUCH: bNewStance = ANIM_PRONE;  break;
		}
		UIHandleSoldierStanceChange(gpSMCurrentMerc->ubID, bNewStance);
	}
}


static void BtnStealthModeCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gpSMCurrentMerc->bStealthMode = !gpSMCurrentMerc->bStealthMode;
		gfUIStanceDifferent		= TRUE;
		gfPlotNewMovement			= TRUE;
		fInterfacePanelDirty	= DIRTYLEVEL2;
	}
}


static void BtnHandCursorCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		ToggleHandCursorMode(&guiCurrentEvent);
	}
}


static void BtnTalkCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		ToggleTalkCursorMode(&guiCurrentEvent);
	}
}


static void BtnMuteCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gpSMCurrentMerc->uiStatusFlags ^= SOLDIER_MUTE;
		const wchar_t* msg = (gpSMCurrentMerc->uiStatusFlags & SOLDIER_MUTE ? TacticalStr[MUTE_ON_STR] : TacticalStr[MUTE_OFF_STR]);
		ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, msg, gpSMCurrentMerc->name);
	}
}


static void SelectMerc(UINT16 id)
{
	gubSelectSMPanelToMerc = id;

	if (!gfInItemPickupMenu)
	{
		if (guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE)
		{
			// Refresh background for player slots (in case item values change due to Flo's discount)
			gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
			LocateSoldier(id, DONTSETLOCATOR);
		}
		else
		{
			LocateSoldier(id, SETLOCATOR);
		}
	}

	// If the user is in the shop keeper interface and is in the item desc
	if (guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE &&
			InItemDescriptionBox())
	{
		DeleteItemDescriptionBox();
	}
}


static void BtnPrevMercCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		SelectMerc(FindPrevActiveAndAliveMerc(gpSMCurrentMerc, TRUE, TRUE));
	}
}


static void BtnNextMercCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		SelectMerc(FindNextActiveAndAliveMerc(gpSMCurrentMerc, TRUE, TRUE));
	}
}


static void BtnOptionsCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		guiPreviousOptionScreen = guiCurrentScreen;
		LeaveTacticalScreen(OPTIONS_SCREEN);
	}
}


static void BtnSMDoneCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gfBeginEndTurn = TRUE;
	}
}


static void BtnMapScreenCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// Enter mapscreen...
		//gfEnteringMapScreen = TRUE;
		GoToMapScreenFromTactical();
	}
}


static void BtnBurstModeCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		ChangeWeaponMode(gpSMCurrentMerc);
	}
}


static void BtnLookCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		ToggleLookCursorMode(NULL);
	}
}


static void EnemyIndicatorClickCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void MercFacePanelCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void MercFacePanelMoveCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void TMClickFirstHandInvCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void TMClickSecondHandInvCallback(MOUSE_REGION* pRegion, INT32 iReason);


// TEAM PANEL!!!!!!!!!!!!!!
BOOLEAN InitializeTEAMPanel(  )
{
	UINT32					cnt, posIndex;

	// INit viewport region
	// Set global mouse regions
	// Define region for viewport
	MSYS_DefineRegion( &gViewportRegion, 0, 0 ,gsVIEWPORT_END_X, gsVIEWPORT_END_Y, MSYS_PRIORITY_NORMAL,
						 VIDEO_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );

	guiTEAMPanel = AddVideoObjectFromFile("INTERFACE/bottom_bar.sti");
	CHECKF(guiTEAMPanel != NO_VOBJECT);
	guiTEAMObjects = AddVideoObjectFromFile("INTERFACE/gold_front.sti");
	CHECKF(guiTEAMObjects != NO_VOBJECT);
	guiBrownBackgroundForTeamPanel = AddVideoObjectFromFile("INTERFACE/Bars.sti");
	CHECKF(guiBrownBackgroundForTeamPanel != NO_VOBJECT);

	// Clear inv display stuff
	memset( gfTEAM_HandInvDispText, 0, sizeof( gfTEAM_HandInvDispText ) );


	// Create buttons
	CHECKF( CreateTEAMPanelButtons( ) );

	// Set viewports
	// Define region for panel
	MSYS_DefineRegion(&gTEAM_PanelRegion, 0, gsVIEWPORT_END_Y, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_NORMAL, CURSOR_NORMAL, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);

	for ( posIndex = 0, cnt = 0; cnt < 6; cnt++, posIndex +=2 )
	{
		MSYS_DefineRegion( &gTEAM_FaceRegions[ cnt ], sTEAMFacesXY[ posIndex ], sTEAMFacesXY[ posIndex + 1 ] ,(INT16)(sTEAMFacesXY[ posIndex ] + TM_FACE_WIDTH ), (INT16)(sTEAMFacesXY[ posIndex + 1 ] + TM_FACE_HEIGHT), MSYS_PRIORITY_NORMAL,
							 MSYS_NO_CURSOR, MercFacePanelMoveCallback, MercFacePanelCallback );
		MSYS_SetRegionUserData( &gTEAM_FaceRegions[ cnt ], 0, cnt );


		MSYS_DefineRegion( &gTEAM_EnemyIndicator[ cnt ], (INT16)(sTEAMFacesXY[ posIndex ] + 1), (INT16)(sTEAMFacesXY[ posIndex + 1 ] + 1 ) ,(INT16)(sTEAMFacesXY[ posIndex ] + INDICATOR_BOX_WIDTH ), (INT16)(sTEAMFacesXY[ posIndex + 1 ] + INDICATOR_BOX_HEIGHT), MSYS_PRIORITY_NORMAL,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, EnemyIndicatorClickCallback );
		MSYS_SetRegionUserData( &gTEAM_EnemyIndicator[ cnt ], 0, cnt );



		if ( IsMouseInRegion( &gTEAM_FaceRegions[ cnt ] ) )
		{
			if ( gTeamPanel[ cnt ].ubID != NOBODY )
			{
				HandleMouseOverSoldierFaceForContMove( MercPtrs[ gTeamPanel[ cnt ].ubID ], TRUE );
			}
		}


		MSYS_DefineRegion( &gTEAM_BarsRegions[ cnt ], sTEAMBarsXY[ posIndex ], sTEAMBarsXY[ posIndex + 1 ] ,(INT16)(sTEAMBarsXY[ posIndex ] + TM_BARS_REGION_WIDTH ), (INT16)(sTEAMBarsXY[ posIndex + 1 ] + TM_BARS_REGION_HEIGHT ), MSYS_PRIORITY_NORMAL,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MercFacePanelCallback );
		MSYS_SetRegionUserData( &gTEAM_BarsRegions[ cnt ], 0, cnt );

		MSYS_DefineRegion( &gTEAM_LeftBarsRegions[ cnt ], (INT16)(sTEAMFacesXY[ posIndex ] - 8 ), sTEAMFacesXY[ posIndex + 1 ] ,(INT16)(sTEAMFacesXY[ posIndex ] ), (INT16)(sTEAMFacesXY[ posIndex + 1 ] + TM_BARS_REGION_HEIGHT ), MSYS_PRIORITY_NORMAL,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MercFacePanelCallback );
		MSYS_SetRegionUserData( &gTEAM_LeftBarsRegions[ cnt ], 0, cnt );


		MSYS_DefineRegion( &gTEAM_FirstHandInv[ cnt ], sTEAMHandInvXY[ posIndex ], sTEAMHandInvXY[ posIndex + 1 ], (INT16)(sTEAMHandInvXY[ posIndex ] + TM_INV_WIDTH ) ,(INT16)(sTEAMHandInvXY[ posIndex + 1 ] + TM_INV_HEIGHT ), MSYS_PRIORITY_NORMAL,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, TMClickFirstHandInvCallback );

		// Add user data
		MSYS_SetRegionUserData( &gTEAM_FirstHandInv[ cnt ], 0, cnt );



		MSYS_DefineRegion( &gTEAM_SecondHandInv[ cnt ], sTEAMHandInvXY[ posIndex ], (INT16)( sTEAMHandInvXY[ posIndex + 1 ] + 24 ), (INT16)(sTEAMHandInvXY[ posIndex ] + TM_INV_WIDTH) ,(INT16)(sTEAMHandInvXY[ posIndex + 1 ] + TM_INV_HEIGHT + 24 ), MSYS_PRIORITY_NORMAL,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, TMClickSecondHandInvCallback );

		// Add user data
		MSYS_SetRegionUserData( &gTEAM_SecondHandInv[ cnt ], 0, cnt );

	}


	//DEfine region for selected guy panel
	//MSYS_DefineRegion( &gSM_SELMERCPanalRegion, SM_SELMERC_FACE_X, SM_SELMERC_FACE_Y, SM_SELMERC_FACE_X + SM_SELMERC_FACE_WIDTH, SM_SELMERC_FACE_Y + SM_SELMERC_FACE_HEIGHT, MSYS_PRIORITY_NORMAL,
	//					 CURSOR_NORMAL, MSYS_NO_CALLBACK, SelectedMercButtonCallback );

	return( TRUE );
}

BOOLEAN ShutdownTEAMPanel( )
{
	UINT32					cnt;

	// All buttons and regions and video objects and video surfaces will be deleted at shutddown of SGM
	// We may want to delete them at the interm as well, to free up room for other panels
	// Delete video objects!
	DeleteVideoObjectFromIndex( guiTEAMPanel );
	DeleteVideoObjectFromIndex( guiTEAMObjects );
	DeleteVideoObjectFromIndex( guiBrownBackgroundForTeamPanel );


	MSYS_RemoveRegion( &gTEAM_PanelRegion);

	// Remove viewport
	MSYS_RemoveRegion( &gViewportRegion );

	for ( cnt = 0; cnt < 6; cnt++ )
	{
		MSYS_RemoveRegion( &gTEAM_EnemyIndicator[ cnt ] );
		MSYS_RemoveRegion( &gTEAM_FaceRegions[ cnt ] );
		MSYS_RemoveRegion( &gTEAM_BarsRegions[ cnt ] );
		MSYS_RemoveRegion( &gTEAM_LeftBarsRegions[ cnt ] );
		MSYS_RemoveRegion( &gTEAM_FirstHandInv[ cnt ] );
		MSYS_RemoveRegion( &gTEAM_SecondHandInv[ cnt ] );

		if ( gTeamPanel[ cnt ].ubID != NOBODY )
		{
			HandleMouseOverSoldierFaceForContMove( MercPtrs[ gTeamPanel[ cnt ].ubID ], FALSE );
		}

	}

	// Remove buttons
	RemoveTEAMPanelButtons( );

	// turn off the display of the squad list
	if( fRenderRadarScreen == FALSE )
	{
		// start rendering radar region again,
	  fRenderRadarScreen = TRUE;

		// remove squad panel
		//CreateDestroyMouseRegionsForSquadList( );
	}
	return( TRUE );
}


static void RenderSoldierTeamInv(SOLDIERTYPE* pSoldier, INT16 sX, INT16 sY, UINT8 ubPanelNum, BOOLEAN fDirty);
static void UpdateTEAMPanel(void);


void RenderTEAMPanel( BOOLEAN fDirty )
{
	INT16 sFontX, sFontY;
	UINT32				cnt, posIndex;
	SOLDIERTYPE		*pSoldier;
	static wchar_t pStr[200];

	if ( fDirty == DIRTYLEVEL2 )
	{
		MarkAButtonDirty( iTEAMPanelButtons[ TEAM_DONE_BUTTON ] );
		MarkAButtonDirty( iTEAMPanelButtons[ TEAM_MAP_SCREEN_BUTTON ] );
		MarkAButtonDirty( iTEAMPanelButtons[ CHANGE_SQUAD_BUTTON ] );


		// Blit video surface
		BltVideoObjectFromIndex(guiSAVEBUFFER, guiTEAMPanel, 0, INTERFACE_START_X, INTERFACE_START_Y);
		RestoreExternBackgroundRect(INTERFACE_START_X, INTERFACE_START_Y, SCREEN_WIDTH - INTERFACE_START_X, SCREEN_HEIGHT - INTERFACE_START_Y);

		// LOOP THROUGH ALL MERCS ON TEAM PANEL
		for ( cnt = 0, posIndex = 0; cnt < NUM_TEAM_SLOTS; cnt++, posIndex+= 2 )
		{
			// GET SOLDIER
			if ( !gTeamPanel[ cnt ].fOccupied )
			{
				//BLIT CLOSE PANEL
				BltVideoObjectFromIndex( guiSAVEBUFFER, guiCLOSE, 5, sTEAMFacesXY[ posIndex ], sTEAMFacesXY[ posIndex + 1 ]);
				RestoreExternBackgroundRect( sTEAMFacesXY[ posIndex ], sTEAMFacesXY[ posIndex + 1 ], TM_FACE_WIDTH, TM_FACE_HEIGHT );

				// BLIT AP CLOSE PANEL
				//BltVideoObjectFromIndex( FRAME_BUFFER, guiTEAMObjects, 3, sTEAMAPPanelXY[ posIndex ], sTEAMAPPanelXY[ posIndex + 1 ]);
				//RestoreExternBackgroundRect( (INT16)(sTEAMAPPanelXY[ posIndex ]), (INT16)(sTEAMAPPanelXY[ posIndex + 1 ]), TM_APPANEL_WIDTH, TM_APPANEL_HEIGHT );
				//InvalidateRegion( (INT16)(sTEAMAPPanelXY[ posIndex ]), (INT16)(sTEAMAPPanelXY[ posIndex + 1 ]), (INT16)(sTEAMAPPanelXY[ posIndex ] + TM_APPANEL_WIDTH), (INT16)(sTEAMAPPanelXY[ posIndex + 1 ] + TM_APPANEL_HEIGHT ) );

				if ( gTacticalStatus.ubCurrentTeam != OUR_TEAM || INTERRUPT_QUEUED )
				{
					// Hatch out...
					BltVideoObjectFromIndex( guiSAVEBUFFER, guiTEAMObjects, 1, sTEAMFaceHighlXY[ posIndex ], sTEAMFaceHighlXY[ posIndex + 1 ]);
					RestoreExternBackgroundRect( sTEAMFaceHighlXY[ posIndex ], sTEAMFaceHighlXY[ posIndex + 1 ], TM_FACEHIGHTL_WIDTH, TM_FACEHIGHTL_HEIGHT );
				}

			}
			else
			{
				pSoldier = MercPtrs[ gTeamPanel[ cnt ].ubID ];

				if ( pSoldier->uiStatusFlags & ( SOLDIER_DRIVER ) )
				{
					// Get soldier pointer for vehicle.....
					SOLDIERTYPE *pVehicle;

					pVehicle = GetSoldierStructureForVehicle( pSoldier->iVehicleId );

					//OK, for each item, set dirty text if applicable!
					swprintf( pStr, lengthof(pStr), TacticalStr[ DRIVER_POPUPTEXT ], pVehicle->bLife, pVehicle->bLifeMax, pVehicle->bBreath, pVehicle->bBreathMax );
					SetRegionFastHelpText( &(gTEAM_FirstHandInv[ cnt ]), pStr );
				}
				// Add text for first hand popup
				else
				{
					GetHelpTextForItem( pStr, lengthof(pStr), &( pSoldier->inv[ HANDPOS ] ), pSoldier );

					//OK, for each item, set dirty text if applicable!
					SetRegionFastHelpText( &(gTEAM_FirstHandInv[ cnt ]), pStr );
				}

				// Add text for seonc hand popup
				if ( pSoldier->uiStatusFlags & ( SOLDIER_PASSENGER | SOLDIER_DRIVER ) )
				{
					//OK, for each item, set dirty text if applicable!
					SetRegionFastHelpText( &(gTEAM_SecondHandInv[ cnt ]), TacticalStr[ EXIT_VEHICLE_POPUPTEXT ] );
				}
				else
				{
					GetHelpTextForItem( pStr, lengthof(pStr), &( pSoldier->inv[ SECONDHANDPOS ]  ), pSoldier );

					//OK, for each item, set dirty text if applicable!
					SetRegionFastHelpText( &(gTEAM_SecondHandInv[ cnt ]), pStr );
				}


				// Render Selected guy if selected
				if ( gusSelectedSoldier == pSoldier->ubID && gTacticalStatus.ubCurrentTeam == OUR_TEAM && OK_INTERRUPT_MERC( pSoldier ) )
				{
					BltVideoObjectFromIndex(guiSAVEBUFFER, guiTEAMObjects, 0, sTEAMFaceHighlXY[posIndex], sTEAMFaceHighlXY[posIndex + 1]);
					RestoreExternBackgroundRect(sTEAMFaceHighlXY[posIndex], sTEAMFaceHighlXY[posIndex + 1], TM_FACEHIGHTL_WIDTH, TM_FACEHIGHTL_HEIGHT);

					// Render faceplate
					//BltVideoObjectFromIndex(guiSAVEBUFFER, guiTEAMObjects, 1, sTEAMFacesXY[posIndex] + TM_FACEPLATE_OFFSET_X, sTEAMFacesXY[posIndex + 1] + TM_FACEPLATE_OFFSET_Y);
					//RestoreExternBackgroundRect((INT16)(sTEAMFacesXY[posIndex] + TM_FACEPLATE_OFFSET_X), (INT16)(sTEAMFacesXY[posIndex + 1] + TM_FACEPLATE_OFFSET_Y), TM_FACEPLATE_WIDTH, TM_FACEPLATE_HEIGHT);
				}
				else if ( gTacticalStatus.ubCurrentTeam != OUR_TEAM || !OK_INTERRUPT_MERC( pSoldier ) )
				{
					BltVideoObjectFromIndex( guiSAVEBUFFER, guiTEAMObjects, 1, sTEAMFaceHighlXY[ posIndex ], sTEAMFaceHighlXY[ posIndex + 1 ]);
					RestoreExternBackgroundRect( sTEAMFaceHighlXY[ posIndex ], sTEAMFaceHighlXY[ posIndex + 1 ], TM_FACEHIGHTL_WIDTH, TM_FACEHIGHTL_HEIGHT );
						// Render non-selection
						//BltVideoObjectFromIndex( guiSAVEBUFFER, guiTEAMObjects, 4, sTEAMFaceHighlXY[ posIndex ], sTEAMFaceHighlXY[ posIndex + 1 ]);
						//RestoreExternBackgroundRect( sTEAMFaceHighlXY[ posIndex ], sTEAMFaceHighlXY[ posIndex + 1 ], TM_FACEHIGHTL_WIDTH, TM_FACEHIGHTL_HEIGHT );

						// Render faceplate
						//BltVideoObjectFromIndex( guiSAVEBUFFER, guiTEAMObjects, 1, sTEAMFacesXY[ posIndex ] + TM_FACEPLATE_OFFSET_X, sTEAMFacesXY[ posIndex + 1 ] + TM_FACEPLATE_OFFSET_Y);
						//RestoreExternBackgroundRect( (INT16)(sTEAMFacesXY[ posIndex ] + TM_FACEPLATE_OFFSET_X), (INT16)(sTEAMFacesXY[ posIndex + 1 ] + TM_FACEPLATE_OFFSET_Y ), TM_FACEPLATE_WIDTH, TM_FACEPLATE_HEIGHT );


				}

				CheckForFacePanelStartAnims( pSoldier, sTEAMFacesXY[ posIndex ], sTEAMFacesXY[ posIndex + 1 ] );

				RenderSoldierFace( pSoldier, sTEAMFacesXY[ posIndex ], sTEAMFacesXY[ posIndex + 1 ], TRUE );

				// Restore AP/LIFE POSIITONS

				// Render name!
				SetFont( BLOCKFONT2 );

				if ( pSoldier->bStealthMode )
				{
					SetFontBackground( FONT_MCOLOR_BLACK );
					SetFontForeground( FONT_MCOLOR_LTYELLOW );
				}
				else
				{
					SetFontBackground( FONT_MCOLOR_BLACK );
					SetFontForeground( FONT_MCOLOR_LTGRAY );
				}

				// RENDER ON SAVE BUFFER!
				SetFontDestBuffer(guiSAVEBUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
				FindFontCenterCoordinates(sTEAMNamesXY[posIndex] + 2, sTEAMNamesXY[posIndex + 1], TM_NAME_WIDTH, TM_NAME_HEIGHT, pSoldier->name, BLOCKFONT2, &sFontX, &sFontY);
				mprintf( sFontX, sFontY, L"%ls", pSoldier->name );
				gprintfRestore( sFontX, sFontY, L"%ls", pSoldier->name );
				// reset to frame buffer!
				SetFontDestBuffer(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			}
		}

		RenderTownIDString( );

	}

	// Loop through all mercs and make go
	for ( cnt = 0, posIndex = 0; cnt < NUM_TEAM_SLOTS; cnt++, posIndex+= 2 )
	{
		// GET SOLDIER
		if ( gTeamPanel[ cnt ].fOccupied )
		{
			pSoldier = MercPtrs[ gTeamPanel[ cnt ].ubID ];

			// Update animations....
			if ( pSoldier->fClosePanel || pSoldier->fClosePanelToDie )
			{
				pSoldier->sPanelFaceX = gFacesData[ pSoldier->iFaceIndex ].usFaceX;
				pSoldier->sPanelFaceY = gFacesData[ pSoldier->iFaceIndex ].usFaceY;
			}


			if ( fDirty != DIRTYLEVEL0 )
			{
				// UPdate stats!
        if ( fDirty == DIRTYLEVEL2 )
        {
				  if ( pSoldier->bLife != 0 )
				  {
            if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
            {
			        swprintf( pStr, lengthof(pStr), TacticalStr[ VEHICLE_VITAL_STATS_POPUPTEXT ], pSoldier->bLife, pSoldier->bLifeMax, pSoldier->bBreath, pSoldier->bBreathMax );
			        SetRegionFastHelpText( &(gTEAM_BarsRegions[ cnt ]), pStr );
            }
            else if ( pSoldier->uiStatusFlags & SOLDIER_ROBOT )
            {
			        swprintf( pStr, lengthof(pStr), gzLateLocalizedString[ 16 ], pSoldier->bLife, pSoldier->bLifeMax );
			        SetRegionFastHelpText( &(gTEAM_BarsRegions[ cnt ]), pStr );
            }
            else
            {
					    const wchar_t* Morale = GetMoraleString(pSoldier);
					    swprintf(pStr, lengthof(pStr), TacticalStr[MERC_VITAL_STATS_POPUPTEXT], pSoldier->bLife, pSoldier->bLifeMax, pSoldier->bBreath, pSoldier->bBreathMax, Morale);
					    SetRegionFastHelpText( &(gTEAM_BarsRegions[ cnt ]), pStr );
            }
				  }
				  else
				  {
					  SetRegionFastHelpText( &(gTEAM_BarsRegions[ cnt ]), L"" );
				  }
        }

				if ( !( pSoldier->uiStatusFlags & SOLDIER_DEAD ) )
				{
					DrawSoldierUIBars(pSoldier, 69 + 83 * cnt, 365 + TM_LIFEBAR_HEIGHT, TRUE, FRAME_BUFFER);

					if ( gTacticalStatus.uiFlags & TURNBASED && pSoldier->bLife >= OKLIFE )
					{
						// Render APs
						SetFont( TINYFONT1 );

						//if ( pSoldier->sLastTarget != NOWHERE && !EnoughPoints( pSoldier, MinAPsToAttack( pSoldier, pSoldier->sLastTarget, TRUE ), 0, FALSE ) || GetUIApsToDisplay( pSoldier ) < 0 )
						if ( !EnoughPoints( pSoldier, MinAPsToAttack( pSoldier, pSoldier->sLastTarget, TRUE ), 0, FALSE ) || GetUIApsToDisplay( pSoldier ) < 0 )
						{
							SetFontBackground( FONT_MCOLOR_BLACK );
							SetFontForeground( FONT_MCOLOR_DKRED );
						}
						else
						{
							if ( MercUnderTheInfluence( pSoldier ) )
							{
								SetFontBackground( FONT_MCOLOR_BLACK );
								SetFontForeground( FONT_MCOLOR_LTBLUE );
							}
							else if ( pSoldier->bStealthMode )
							{
								SetFontBackground( FONT_MCOLOR_BLACK );
								SetFontForeground( FONT_MCOLOR_LTYELLOW );
							}
							else
							{
								SetFontBackground( FONT_MCOLOR_BLACK );
								SetFontForeground( FONT_MCOLOR_LTGRAY );
							}
						}
						RestoreExternBackgroundRect( sTEAMApXY[ posIndex ], sTEAMApXY[ posIndex + 1 ], TM_AP_WIDTH, TM_AP_HEIGHT );

						if (gTacticalStatus.uiFlags & INCOMBAT )
						{
							VarFindFontCenterCoordinates( sTEAMApXY[ posIndex ], sTEAMApXY[ posIndex + 1], TM_AP_WIDTH, TM_AP_HEIGHT, TINYFONT1, &sFontX, &sFontY, L"%d", GetUIApsToDisplay( pSoldier ) );
							mprintf( sFontX, sTEAMApXY[ posIndex + 1], L"%d", GetUIApsToDisplay( pSoldier ) );
						}
					}
				}
				else
				{
					// Erase APs
					RestoreExternBackgroundRect( sTEAMApXY[ posIndex ], sTEAMApXY[ posIndex + 1 ], TM_AP_WIDTH, TM_AP_HEIGHT );

				}

			}

			RenderSoldierTeamInv( pSoldier, sTEAMHandInvXY[ posIndex ], sTEAMHandInvXY[ posIndex + 1 ], (UINT8)cnt, fDirty );

			HandleSoldierFaceFlash( pSoldier, sTEAMFacesXY[ posIndex ], sTEAMFacesXY[ posIndex + 1 ] );


			if ( !(guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
			{
				//HandlePanelFaceAnimations( pSoldier );
			}

		}

	}
	UpdateTEAMPanel( );

	if( fRenderRadarScreen == TRUE )
	{
		CreateMouseRegionForPauseOfClock( CLOCK_REGION_START_X, CLOCK_REGION_START_Y );
	}
	else
	{
		RemoveMouseRegionForPauseOfClock( );
	}
}


static void BtnEndTurnCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnRostermodeCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnSquadCallback(GUI_BUTTON* btn, INT32 reason);


BOOLEAN CreateTEAMPanelButtons( )
{
	// Load button Graphics
	iTEAMPanelImages[ENDTURN_IMAGES] = LoadButtonImage("INTERFACE/bottom_bar_buttons.sti", -1, 0, -1, 3, -1);
	iTEAMPanelImages[ ROSTERMODE_IMAGES  ]	= UseLoadedButtonImage( iTEAMPanelImages[ ENDTURN_IMAGES  ] ,-1, 1,-1, 4,-1 );
	iTEAMPanelImages[ DISK_IMAGES  ]				= UseLoadedButtonImage( iTEAMPanelImages[ ENDTURN_IMAGES  ] ,-1, 2,-1, 5, -1 );

	// Create buttons
	iTEAMPanelButtons[ TEAM_DONE_BUTTON ] = QuickCreateButton( iTEAMPanelImages[ ENDTURN_IMAGES ], TM_ENDTURN_X, TM_ENDTURN_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH - 1,
										DEFAULT_MOVE_CALLBACK, BtnEndTurnCallback );
	if ( iTEAMPanelButtons[ TEAM_DONE_BUTTON ] == -1 )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button" );
		return( FALSE );
	}
	SetButtonFastHelpText( iTEAMPanelButtons[ TEAM_DONE_BUTTON ], TacticalStr[ END_TURN_POPUPTEXT ] );



	iTEAMPanelButtons[ TEAM_MAP_SCREEN_BUTTON ] = QuickCreateButton( iTEAMPanelImages[ ROSTERMODE_IMAGES ], TM_ROSTERMODE_X, TM_ROSTERMODE_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH - 1,
										DEFAULT_MOVE_CALLBACK, BtnRostermodeCallback );
	if ( iTEAMPanelButtons[ TEAM_MAP_SCREEN_BUTTON ] == -1 )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button" );
		return( FALSE );
	}
	SetButtonFastHelpText( iTEAMPanelButtons[ TEAM_MAP_SCREEN_BUTTON ], TacticalStr[ MAPSCREEN_POPUPTEXT ] );

	iTEAMPanelButtons[ CHANGE_SQUAD_BUTTON ] = QuickCreateButton( iTEAMPanelImages[ DISK_IMAGES ], TM_DISK_X, TM_DISK_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH - 1,
										DEFAULT_MOVE_CALLBACK, BtnSquadCallback );
	if ( iTEAMPanelButtons[ CHANGE_SQUAD_BUTTON ] == -1 )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button" );
		return( FALSE );
	}
	SetButtonFastHelpText( iTEAMPanelButtons[ CHANGE_SQUAD_BUTTON ], TacticalStr[ CHANGE_SQUAD_POPUPTEXT ] );

	return( TRUE );
}

void    RemoveTEAMPanelButtons( )
{
	UINT32 cnt;

	for ( cnt = 0; cnt < NUM_TEAM_BUTTONS; cnt++ )
	{
		RemoveButton( iTEAMPanelButtons[ cnt ] );
	}

	for ( cnt = 0; cnt < NUM_TEAM_BUTTON_IMAGES; cnt++ )
	{
		UnloadButtonImage( iTEAMPanelImages[ cnt ] );
	}

}


static void BtnEndTurnCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		UIHandleEndTurn(NULL);
	}
}


static void BtnRostermodeCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (guiCurrentScreen == GAME_SCREEN) GoToMapScreenFromTactical();
	}
}


// callback to handle squad switching callback
static void BtnSquadCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		ToggleRadarScreenRender();
	}
}


void SetTEAMPanelCurrentMerc( UINT8 ubNewID )
{
	// Ignore ther ID given!
	// ALWYAS USE CURRENT MERC!

	fInterfacePanelDirty = DIRTYLEVEL2;

	UpdateTEAMPanel( );

}


static void UpdateTEAMPanel(void)
{
	INT32		cnt;

	if ( (gTacticalStatus.ubCurrentTeam != gbPlayerNum) || (gTacticalStatus.uiFlags & REALTIME ) || !(gTacticalStatus.uiFlags & INCOMBAT ) )
	{
		DisableButton( iTEAMPanelButtons[ TEAM_DONE_BUTTON ] );
	}
	else
	{
		EnableButton( iTEAMPanelButtons[ TEAM_DONE_BUTTON ] );
	}

	if ( gTacticalStatus.uiFlags & ENGAGED_IN_CONV )
	{
		DisableButton( iTEAMPanelButtons[ TEAM_MAP_SCREEN_BUTTON ] );
	}
	else
	{
		EnableButton( iTEAMPanelButtons[ TEAM_MAP_SCREEN_BUTTON ] );
	}


	if ( gfDisableTacticalPanelButtons )
	{
		DisableButton( iTEAMPanelButtons[ TEAM_DONE_BUTTON ] );
		DisableButton( iTEAMPanelButtons[ TEAM_MAP_SCREEN_BUTTON ] );
		DisableButton( iTEAMPanelButtons[ CHANGE_SQUAD_BUTTON ] );

		// OK, disable item regions.......
		for ( cnt = 0; cnt < 6; cnt++ )
		{
			MSYS_DisableRegion( &gTEAM_EnemyIndicator[ cnt ] );

			MSYS_DisableRegion( &gTEAM_FirstHandInv[ cnt ] );

			MSYS_DisableRegion( &gTEAM_SecondHandInv[ cnt ] );
		}

		//disable the radar map region
		// If NOT in overhead map
		if ( !InOverheadMap( ) )
		{
			MSYS_DisableRegion(&gRadarRegion);
		}

	}
	else
	{
		EnableButton( iTEAMPanelButtons[ CHANGE_SQUAD_BUTTON ] );

		for ( cnt = 0; cnt < 6; cnt++ )
		{
			MSYS_EnableRegion( &gTEAM_EnemyIndicator[ cnt ] );

			MSYS_EnableRegion( &gTEAM_FirstHandInv[ cnt ] );

			MSYS_EnableRegion( &gTEAM_SecondHandInv[ cnt ] );
		}

		//Enable the radar map region
		MSYS_EnableRegion(&gRadarRegion);
	}
}


static void HandleMouseOverTeamFaceForContMove(BOOLEAN fOn)
{
	FACETYPE *pFace;

	if ( fOn )
	{
		// Check if we are waiting to continue move...
		if ( CheckForMercContMove( gpSMCurrentMerc ) )
		{
			// Display 'cont' on face....
			// Get face
			pFace = &gFacesData[ gpSMCurrentMerc->iFaceIndex ];

			pFace->fDisplayTextOver = FACE_DRAW_TEXT_OVER;
			wcscpy( pFace->zDisplayText, TacticalStr[ CONTINUE_OVER_FACE_STR ] );

			// While our mouse is here, draw a path!
			PlotPath( gpSMCurrentMerc, gpSMCurrentMerc->sFinalDestination, NO_COPYROUTE, PLOT, TEMPORARY, (UINT16)gpSMCurrentMerc->usUIMovementMode, NOT_STEALTH, FORWARD, gpSMCurrentMerc->bActionPoints );
		}
	}
	else
	{
		// Remove 'cont' on face....
		// Get face
		pFace = &gFacesData[ gpSMCurrentMerc->iFaceIndex ];

		pFace->fDisplayTextOver = FACE_ERASE_TEXT_OVER;

		// Erase path!
		ErasePath( TRUE );
	}
}


static void MercFacePanelMoveCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT8 ubID, ubSoldierID;
	SOLDIERTYPE	*pSoldier;

	ubID = (UINT8) MSYS_GetRegionUserData( pRegion, 0 );

	// If our flags are set to do this, gofoit!
	if ( (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
	{
		return;
	}

	if ( ubID == NOBODY )
	{
		return;
	}

	// Now use soldier ID values
	ubSoldierID = gTeamPanel[ ubID ].ubID;

	if ( ubSoldierID == NOBODY )
	{
		return;
	}

	pSoldier = MercPtrs[ ubSoldierID ];

	if ( !pSoldier->bActive )
	{
		return;
	}


	if (iReason & MSYS_CALLBACK_REASON_MOVE )
	{
		HandleMouseOverSoldierFaceForContMove( pSoldier, TRUE );
	}
	else if ( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		HandleMouseOverSoldierFaceForContMove( pSoldier, FALSE );
	}

}


static void EnemyIndicatorClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT8 ubID, ubSoldierID;

	ubID = (UINT8) MSYS_GetRegionUserData( pRegion, 0 );

	if ( ubID == NOBODY )
	{
		return;
	}

	// Now use soldier ID values
	ubSoldierID = gTeamPanel[ ubID ].ubID;

	if ( ubSoldierID == NOBODY )
	{
		return;
	}

	if ( !MercPtrs[ ubSoldierID ]->bActive )
	{
		return;
	}


	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		if ( MercPtrs[ ubSoldierID ]->uiStatusFlags & ( SOLDIER_DRIVER | SOLDIER_PASSENGER ) )
		{
		}
		else
		{
			SOLDIERTYPE *pSoldier;

			pSoldier = MercPtrs[ ubSoldierID ];

			if ( pSoldier->bOppCnt > 0 )
			{	// Cycle....
				CycleVisibleEnemies( pSoldier );
			}
			else
			{
				MercFacePanelCallback( pRegion, iReason );
			}
		}
	}
}


static void MercFacePanelCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT8 ubID, ubSoldierID;
	SOLDIERTYPE *pVehicle;

	ubID = (UINT8) MSYS_GetRegionUserData( pRegion, 0 );

	// If our flags are set to do this, gofoit!
	if ( (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
	{
		if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
		{
			SetInfoChar( ubID );
		}
		return;
	}

	if ( ubID == NOBODY )
	{
		return;
	}


	// Now use soldier ID values
	ubSoldierID = gTeamPanel[ ubID ].ubID;

	if ( ubSoldierID == NOBODY )
	{
		return;
	}

	if ( !MercPtrs[ ubSoldierID ]->bActive )
	{
		return;
	}


	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		if ( !gfInItemPickupMenu && gpItemPointer == NULL )
		{
			if ( MercPtrs[ ubSoldierID ]->uiStatusFlags & ( SOLDIER_DRIVER | SOLDIER_PASSENGER ) )
			{
				pVehicle = GetSoldierStructureForVehicle( MercPtrs[ ubSoldierID ]->iVehicleId );

				HandleLocateSelectMerc( pVehicle->ubID, 0 );
			}
			else
			{
				if ( !InOverheadMap( ) )
				{
					// If we can continue a move, do so!
					if ( CheckForMercContMove( MercPtrs[ ubSoldierID ] ) )
					{
						// Continue
						ContinueMercMovement( MercPtrs[ ubSoldierID ] );
						ErasePath( TRUE );
					}
					else
					{
						HandleLocateSelectMerc( ubSoldierID, 0 );
					}
				}
				else
				{
					SelectSoldier( ubSoldierID, TRUE, FALSE );
				}
			}
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		if ( !InOverheadMap( ) )
		{
			// Only if guy is not dead!
			if ( !( MercPtrs[ ubSoldierID ]->uiStatusFlags & SOLDIER_DEAD ) && !AM_AN_EPC( MercPtrs[ ubSoldierID ] ) && !( MercPtrs[ ubSoldierID ]->uiStatusFlags & ( SOLDIER_DRIVER | SOLDIER_PASSENGER ) ) )
			{
				gfSwitchPanel = TRUE;
				gbNewPanel = SM_PANEL;
				gubNewPanelParam = (UINT8)ubSoldierID;
			}
		}
	}

}

extern void InternalSelectSoldier( UINT16 usSoldierID, BOOLEAN fAcknowledge, BOOLEAN fForceReselect, BOOLEAN fFromUI );

void HandleLocateSelectMerc( UINT8 ubID, INT8 bFlag  )
{
	BOOLEAN fSelect = FALSE;

	if( !MercPtrs[ ubID ]->bActive )
	{
		return;
	}

	if ( gpItemPointer != NULL )
	{
		// ATE: Disable for feel purposes....
		//return;
	}


	// ATE: No matter what we do... if below OKLIFE, just locate....
	if ( MercPtrs[ ubID ]->bLife < OKLIFE )
	{
		LocateSoldier( ubID, SETLOCATOR );
		return;
	}

	if ( _KeyDown( ALT ) )
	{
		if ( gGameSettings.fOptions[ TOPTION_OLD_SELECTION_METHOD ] )
		{
			// Select merc
			InternalSelectSoldier( ubID, TRUE, FALSE, TRUE);
			MercPtrs[ ubID ]->fFlashLocator = FALSE;
			ResetMultiSelection( );
		}
		else
		{
			// Just locate....
			LocateSoldier( ubID, SETLOCATOR );
		}
	}
	else
	{
		if ( MercPtrs[ ubID ]->fFlashLocator == FALSE )
		{
			if ( gGameSettings.fOptions[ TOPTION_OLD_SELECTION_METHOD ] )
			{
				// If we are currently selected, slide to location
				if ( ubID == gusSelectedSoldier )
				{
					SlideTo( NOWHERE, gusSelectedSoldier, NOBODY ,SETLOCATOR);
				}
				else
				{
					LocateSoldier( ubID, SETLOCATOR );
				}
			}
			else
			{
				if ( ubID == gusSelectedSoldier )
				{
					SlideTo( NOWHERE, gusSelectedSoldier, NOBODY ,SETLOCATOR );
				}
				else
				{
					fSelect = TRUE;
					//ShowRadioLocator( ubID, SHOW_LOCATOR_NORMAL );
				}
			}
		}
		else
		{
			if ( gGameSettings.fOptions[ TOPTION_OLD_SELECTION_METHOD ] )
			{
				// If we are currently selected, slide to location
				if ( ubID == gusSelectedSoldier )
				{
					SlideTo( NOWHERE, gusSelectedSoldier, NOBODY ,DONTSETLOCATOR);
				}
				else
				{
					LocateSoldier( ubID, DONTSETLOCATOR );
				}

				fSelect = TRUE;
			}
			else
			{
				if ( ubID == gusSelectedSoldier )
				{
					LocateSoldier( ubID, DONTSETLOCATOR );
				}
				else
				{
					fSelect = TRUE;
				}
			}
		}

		if ( bFlag == LOCATEANDSELECT_MERC )
		{
			fSelect = TRUE;
		}

		if ( fSelect )
		{
			// Select merc, only if alive!
			if ( !( MercPtrs[ ubID ]->uiStatusFlags & SOLDIER_DEAD ) )
			{
				InternalSelectSoldier( ubID, TRUE, FALSE, TRUE );
			}
		}
	}

	ResetMultiSelection( );

	// Handle locate select merc....
	HandleMouseOverSoldierFaceForContMove( MercPtrs[ ubID ], TRUE );

}



void ShowRadioLocator( UINT8 ubID, UINT8 ubLocatorSpeed )
{
	RESETTIMECOUNTER( MercPtrs[ ubID ]->FlashSelCounter, FLASH_SELECTOR_DELAY );

	//LocateSoldier( ubID, FALSE );	// IC - this is already being done outside of this function :)
	MercPtrs[ ubID ]->fFlashLocator = TRUE;
	//gbPanelSelectedGuy = ubID;	IC - had to move this outside to make this function versatile
	MercPtrs[ ubID ]->sLocatorFrame = 0;

	if ( ubLocatorSpeed == SHOW_LOCATOR_NORMAL )
	{
		// If we are an AI guy, and we have the baton, make lower...
		// ( MercPtrs[ ubID ]->uiStatusFlags & SOLDIER_UNDERAICONTROL && MercPtrs[ ubID ]->bTeam != gbPlayerNum )
		//
		//ercPtrs[ ubID ]->ubNumLocateCycles = 3;
		//
		//se
		//
			MercPtrs[ ubID ]->ubNumLocateCycles = 5;
		//
	}
	else
	{
		MercPtrs[ ubID ]->ubNumLocateCycles = 3;
	}
}

void EndRadioLocator( UINT8 ubID )
{
	MercPtrs[ ubID ]->fFlashLocator = FALSE;
	MercPtrs[ ubID ]->fShowLocator = FALSE;
}


static void CheckForFacePanelStartAnims(SOLDIERTYPE* pSoldier, INT16 sPanelX, INT16 sPanelY)
{

	if ( !pSoldier->bActive )
	{
		return;
	}


	if ( pSoldier->fUIdeadMerc  )
	{
//		pSoldier->sPanelFaceX	= sPanelX;
//		pSoldier->sPanelFaceY	= sPanelY;
	}


}


void FinishAnySkullPanelAnimations( )
{
	SOLDIERTYPE *pTeamSoldier;
	INT32		cnt2;

	cnt2 = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

	// run through list
	for ( pTeamSoldier = MercPtrs[ cnt2 ]; cnt2 <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt2++,pTeamSoldier++ )
	{
		if ( pTeamSoldier->bActive && pTeamSoldier->bLife == 0 )
		{
			if ( pTeamSoldier->fUIdeadMerc || pTeamSoldier->fClosePanelToDie )
			{
				HandlePlayerTeamMemberDeathAfterSkullAnimation( pTeamSoldier );

				pTeamSoldier->fUIdeadMerc = FALSE;
				pTeamSoldier->fClosePanelToDie = FALSE;
			}
		}
	}

}

void HandlePanelFaceAnimations( SOLDIERTYPE *pSoldier )
{
	if ( pSoldier->bTeam != gbPlayerNum )
	{
		return;
	}


	if ( !pSoldier->bActive )
	{
		return;
	}

	if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		// Don't do this for a vehice.
		return;
	}

	// If this is the first time we are active, open panel!
	if ( pSoldier->fUInewMerc )
	{
		pSoldier->fUInewMerc = FALSE;
	}

	if ( pSoldier->fUICloseMerc )
	{
		pSoldier->fUICloseMerc = FALSE;
	}

	if ( pSoldier->fUIdeadMerc  )
	{
		pSoldier->sPanelFaceX = gFacesData[ pSoldier->iFaceIndex ].usFaceX;
		pSoldier->sPanelFaceY = gFacesData[ pSoldier->iFaceIndex ].usFaceY;

		pSoldier->fUIdeadMerc = FALSE;
		pSoldier->fClosePanel		= TRUE;
		pSoldier->fClosePanelToDie = TRUE;
		pSoldier->ubClosePanelFrame = 0;
		pSoldier->ubDeadPanelFrame = 0;
		RESETTIMECOUNTER( pSoldier->PanelAnimateCounter, 160 );
	}

	if ( pSoldier->fClosePanel )
	{
		 if ( TIMECOUNTERDONE( pSoldier->PanelAnimateCounter, 160 ) )
		 {
				pSoldier->ubClosePanelFrame++;

				if ( pSoldier->ubClosePanelFrame > 5 )
				{
					pSoldier->fClosePanel = FALSE;
					pSoldier->ubClosePanelFrame = 5;

					if ( pSoldier->fClosePanelToDie )
					{
						pSoldier->fDeadPanel = TRUE;
						//PlayJA2Sample(HEADCR_1, HIGHVOLUME, 1, MIDDLEPAN);
					}
					else
					{
						if ( !gFacesData[ pSoldier->iFaceIndex ].fDisabled )
						{
							RestoreExternBackgroundRect( pSoldier->sPanelFaceX, pSoldier->sPanelFaceY, TM_FACE_WIDTH, TM_FACE_HEIGHT );
						}
					}
				}
				RESETTIMECOUNTER( pSoldier->PanelAnimateCounter, 160 );
		 }
	}

	if ( pSoldier->fClosePanel )
	{
		if (!gFacesData[pSoldier->iFaceIndex].fDisabled)
		{
			RestoreExternBackgroundRect(pSoldier->sPanelFaceX, pSoldier->sPanelFaceY, TM_FACE_WIDTH, TM_FACE_HEIGHT);
			BltVideoObjectFromIndex(FRAME_BUFFER, guiCLOSE, pSoldier->ubClosePanelFrame, pSoldier->sPanelFaceX, pSoldier->sPanelFaceY);
			InvalidateRegion(pSoldier->sPanelFaceX, pSoldier->sPanelFaceY, pSoldier->sPanelFaceX + TM_FACE_WIDTH, pSoldier->sPanelFaceY + TM_FACE_HEIGHT);
		}
	}


	if ( pSoldier->fDeadPanel )
	{
		 if ( TIMECOUNTERDONE(  pSoldier->PanelAnimateCounter, 160 ) )
		 {
				pSoldier->ubDeadPanelFrame++;

				if ( pSoldier->ubDeadPanelFrame == 4 )
				{
					ScreenMsg( FONT_RED, MSG_SKULL_UI_FEEDBACK, pMercDeadString[ 0 ], pSoldier->name );

					PlayJA2Sample(DOORCR_1, HIGHVOLUME, 1, MIDDLEPAN);
					PlayJA2Sample(HEADCR_1, HIGHVOLUME, 1, MIDDLEPAN);
				}

				if ( pSoldier->ubDeadPanelFrame > 5 )
				{
					pSoldier->fDeadPanel = FALSE;
					pSoldier->ubDeadPanelFrame = 5;
					pSoldier->fClosePanelToDie = FALSE;

					// Finish!
					if ( !gFacesData[ pSoldier->iFaceIndex ].fDisabled )
					{
						BltVideoObjectFromIndex( guiSAVEBUFFER, guiDEAD, pSoldier->ubDeadPanelFrame, pSoldier->sPanelFaceX, pSoldier->sPanelFaceY);

						// Blit hatch!
						BltVideoObjectFromIndex( guiSAVEBUFFER, guiHATCH, 0, pSoldier->sPanelFaceX, pSoldier->sPanelFaceY);

						RestoreExternBackgroundRect( pSoldier->sPanelFaceX, pSoldier->sPanelFaceY, TM_FACE_WIDTH, TM_FACE_HEIGHT );
					}
					HandlePlayerTeamMemberDeathAfterSkullAnimation( pSoldier );

				}
				RESETTIMECOUNTER( pSoldier->PanelAnimateCounter, 160 );
		 }
	}

	if ( pSoldier->fDeadPanel )
	{
		// Render panel!
		if (!gFacesData[pSoldier->iFaceIndex].fDisabled)
		{
			BltVideoObjectFromIndex(FRAME_BUFFER, guiDEAD, pSoldier->ubDeadPanelFrame, pSoldier->sPanelFaceX, pSoldier->sPanelFaceY);

			// Blit hatch!
			BltVideoObjectFromIndex(guiSAVEBUFFER, guiHATCH, 0, pSoldier->sPanelFaceX, pSoldier->sPanelFaceY);

			InvalidateRegion(pSoldier->sPanelFaceX, pSoldier->sPanelFaceY, pSoldier->sPanelFaceX + TM_FACE_WIDTH, pSoldier->sPanelFaceY + TM_FACE_HEIGHT);
		}
	}


	if ( pSoldier->fOpenPanel )
	{
		 if ( TIMECOUNTERDONE( pSoldier->PanelAnimateCounter, 160 ) )
		 {
				pSoldier->bOpenPanelFrame--;

				if ( pSoldier->bOpenPanelFrame < 0 )
				{
					pSoldier->fOpenPanel = FALSE;
					pSoldier->bOpenPanelFrame = 0;

					if ( !gFacesData[ pSoldier->iFaceIndex ].fDisabled )
					{
						RestoreExternBackgroundRect( pSoldier->sPanelFaceX, pSoldier->sPanelFaceY, TM_FACE_WIDTH, TM_FACE_HEIGHT );
					}
				}
				RESETTIMECOUNTER( pSoldier->PanelAnimateCounter, 160 );
		 }
	}

	if ( pSoldier->fOpenPanel )
	{
		if (!gFacesData[pSoldier->iFaceIndex].fDisabled)
		{
			RestoreExternBackgroundRect(pSoldier->sPanelFaceX, pSoldier->sPanelFaceY, TM_FACE_WIDTH, TM_FACE_HEIGHT);
			BltVideoObjectFromIndex(FRAME_BUFFER, guiCLOSE, pSoldier->bOpenPanelFrame, pSoldier->sPanelFaceX, pSoldier->sPanelFaceY);
			//InvalidateRegion(sTEAMFacesXY[ubOpenPanelID], sTEAMFacesXY[ubOpenPanelID + 1], sTEAMFacesXY[ubOpenPanelID] + TM_FACE_WIDTH, sTEAMFacesXY[ubOpenPanelID + 1] + TM_FACE_HEIGHT);
		}
	}

}


static void HandleSoldierFaceFlash(SOLDIERTYPE* pSoldier, INT16 sFaceX, INT16 sFaceY)
{
}


static void RenderSoldierTeamInv(SOLDIERTYPE* pSoldier, INT16 sX, INT16 sY, UINT8 ubPanelNum, BOOLEAN fDirty)
{
	if ( pSoldier->bActive && !(pSoldier->uiStatusFlags & SOLDIER_DEAD ) )
	{
		if ( pSoldier->uiStatusFlags & SOLDIER_DRIVER )
		{
			BltVideoObjectFromIndex( guiSAVEBUFFER, guiVEHINV, 0, sX, sY);
			RestoreExternBackgroundRect( sX, sY, (INT16)( TM_INV_WIDTH ) , (INT16)( TM_INV_HEIGHT ) );
		}
		else
		{
			// Look in primary hand
			INVRenderItem( guiSAVEBUFFER, pSoldier, &(pSoldier->inv[ HANDPOS ]), sX, sY, TM_INV_WIDTH, TM_INV_HEIGHT, fDirty, &(gfTEAM_HandInvDispText[ ubPanelNum ][ HANDPOS ] ), 0 , FALSE, 0);
		}

		if ( pSoldier->uiStatusFlags & ( SOLDIER_PASSENGER | SOLDIER_DRIVER ) )
		{
			BltVideoObjectFromIndex( guiSAVEBUFFER, guiVEHINV, 1, sX, (INT16)(sY + TM_INV_HAND_SEPY));
			RestoreExternBackgroundRect( sX, (INT16)(sY + TM_INV_HAND_SEPY), (INT16)(TM_INV_WIDTH ) , (INT16)( TM_INV_HEIGHT ) );
		}
		else
		{
			// Do secondary hand
			INVRenderItem( guiSAVEBUFFER, pSoldier, &(pSoldier->inv[ SECONDHANDPOS ]), sX, (INT16)(sY + TM_INV_HAND_SEPY), TM_INV_WIDTH, TM_INV_HEIGHT, fDirty, &(gfTEAM_HandInvDispText[ ubPanelNum ][ SECONDHANDPOS ] ), 0 , FALSE, 0);
		}

	}
}


static void TMClickFirstHandInvCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT8		ubID, ubSoldierID;
	UINT16	usOldHandItem;

	ubID = (UINT8) MSYS_GetRegionUserData( pRegion, 0 );


	if ( ubID == NOBODY )
		return;

	// Now use soldier ID values
	ubSoldierID = gTeamPanel[ ubID ].ubID;

	if ( ubSoldierID == NOBODY )
		return;

	if (iReason == MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		// Change to use cursor mode...
		guiPendingOverrideEvent = A_ON_TERRAIN;
	}

	if (iReason == MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		if ( !AM_A_ROBOT( MercPtrs[ ubSoldierID ] ) )
		{
			usOldHandItem = MercPtrs[ ubSoldierID ]->inv[HANDPOS].usItem;
			//SwapOutHandItem( MercPtrs[ ubSoldierID ] );
			SwapHandItems( MercPtrs[ ubSoldierID ] );
			ReLoadSoldierAnimationDueToHandItemChange( MercPtrs[ ubSoldierID ], usOldHandItem, MercPtrs[ ubSoldierID ]->inv[HANDPOS].usItem );
			fInterfacePanelDirty = DIRTYLEVEL2;
		}
	}
}


static void TMClickSecondHandInvCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT8 ubID, ubSoldierID;
	UINT16	usOldHandItem;

	ubID = (UINT8) MSYS_GetRegionUserData( pRegion, 0 );

	if ( ubID == NOBODY )
		return;

	// Now use soldier ID values
	ubSoldierID = gTeamPanel[ ubID ].ubID;

	if ( ubSoldierID == NOBODY )
		return;

	if (iReason == MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if ( MercPtrs[ ubSoldierID ]->uiStatusFlags & ( SOLDIER_PASSENGER | SOLDIER_DRIVER ) )
		{
			ExitVehicle( MercPtrs[ ubSoldierID ] );
		}
	}

	if (iReason == MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		if ( MercPtrs[ ubSoldierID ]->uiStatusFlags & ( SOLDIER_PASSENGER | SOLDIER_DRIVER ) )
		{
		}
		else
		{
			if ( !AM_A_ROBOT( MercPtrs[ ubSoldierID ] ) )
			{
				usOldHandItem = MercPtrs[ ubSoldierID ]->inv[HANDPOS].usItem;
				SwapHandItems( MercPtrs[ ubSoldierID ] );
				ReLoadSoldierAnimationDueToHandItemChange( MercPtrs[ ubSoldierID ], usOldHandItem, MercPtrs[ ubSoldierID ]->inv[HANDPOS].usItem );
				fInterfacePanelDirty = DIRTYLEVEL2;
			}
		}
	}
}


static BOOLEAN PlayerExistsInSlot(UINT8 ubID)
{
	INT32 cnt;

	for ( cnt = 0; cnt < NUM_TEAM_SLOTS; cnt++ )
	{
		if ( gTeamPanel[ cnt ].fOccupied	)
		{
			if ( gTeamPanel[ cnt ].ubID == ubID )
			{
				return( TRUE );
			}
		}
	}

	return( FALSE );
}


static INT8 GetTeamSlotFromPlayerID(UINT8 ubID)
{
	INT8 cnt;

	for ( cnt = 0; cnt < NUM_TEAM_SLOTS; cnt++ )
	{
		if ( gTeamPanel[ cnt ].fOccupied	)
		{
			if ( gTeamPanel[ cnt ].ubID == ubID )
			{
				return( cnt );
			}
		}
	}

	return( -1 );
}


static BOOLEAN RemovePlayerFromInterfaceTeamSlot(UINT8 ubPanelSlot);


BOOLEAN RemovePlayerFromTeamSlotGivenMercID( UINT8 ubMercID )
{
	INT32 cnt;

	for ( cnt = 0; cnt < NUM_TEAM_SLOTS; cnt++ )
	{
		if ( gTeamPanel[ cnt ].fOccupied	)
		{
			if ( gTeamPanel[ cnt ].ubID == ubMercID )
			{
				RemovePlayerFromInterfaceTeamSlot( (UINT8)cnt );
				return( TRUE );
			}
		}
	}

	return( FALSE );
}


void AddPlayerToInterfaceTeamSlot( UINT8 ubID )
{
	INT32  cnt;

	// If we are a vehicle don't ever add.....
	if ( MercPtrs[ ubID ]->uiStatusFlags & SOLDIER_VEHICLE )
	{
		return;
	}

	if ( !PlayerExistsInSlot( ubID ) )
	{
		// Find a free slot
		for ( cnt = 0; cnt < NUM_TEAM_SLOTS; cnt++ )
		{
			if ( !gTeamPanel[ cnt ].fOccupied	)
			{
				gTeamPanel[ cnt ].fOccupied = TRUE;
				gTeamPanel[ cnt ].ubID			= ubID;

				MSYS_SetRegionUserData( &gTEAM_FirstHandInv[ cnt ], 0, cnt );
				MSYS_SetRegionUserData( &gTEAM_FaceRegions[ cnt ], 0, cnt );

				// DIRTY INTERFACE
				fInterfacePanelDirty = DIRTYLEVEL2;

				// Set ID to do open anim
				MercPtrs[ ubID ]->fUInewMerc						= TRUE;

				break;
			}
		}
	}
}

BOOLEAN InitTEAMSlots( )
{
	INT32 cnt;

	for ( cnt = 0; cnt < NUM_TEAM_SLOTS; cnt++ )
	{
		gTeamPanel[ cnt ].fOccupied = FALSE;
		gTeamPanel[ cnt ].ubID			= NOBODY;

	}

	return( TRUE );
}


BOOLEAN GetPlayerIDFromInterfaceTeamSlot( UINT8 ubPanelSlot, UINT8 *pubID )
{
	if ( ubPanelSlot >= NUM_TEAM_SLOTS )
	{
		return( FALSE );
	}

	if ( gTeamPanel[ ubPanelSlot ].fOccupied )
	{
		*pubID = gTeamPanel[ ubPanelSlot ].ubID;
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}

void RemoveAllPlayersFromSlot( )
{
	int cnt;

	for ( cnt = 0; cnt < NUM_TEAM_SLOTS; cnt++ )
	{
		RemovePlayerFromInterfaceTeamSlot( (UINT8)cnt );
	}
}


static BOOLEAN RemovePlayerFromInterfaceTeamSlot(UINT8 ubPanelSlot)
{
	if ( ubPanelSlot >= NUM_TEAM_SLOTS )
	{
		return( FALSE );
	}

	if ( gTeamPanel[ ubPanelSlot ].fOccupied )
	{
		if ( !( MercPtrs[ gTeamPanel[ ubPanelSlot ].ubID ]->uiStatusFlags & SOLDIER_DEAD ) )
    {
		  // Set Id to close
		  MercPtrs[ gTeamPanel[ ubPanelSlot ].ubID ]->fUICloseMerc		= TRUE;
    }

		// Set face to inactive...
		SetAutoFaceInActive( MercPtrs[ gTeamPanel[ ubPanelSlot ].ubID ]->iFaceIndex );


		gTeamPanel[ ubPanelSlot ].fOccupied = FALSE;
		gTeamPanel[ ubPanelSlot ].ubID = NOBODY;

		MSYS_SetRegionUserData( &gTEAM_FirstHandInv[ ubPanelSlot ], 0, NOBODY );
		MSYS_SetRegionUserData( &gTEAM_FaceRegions[ ubPanelSlot ],	0, NOBODY );

		// DIRTY INTERFACE
		fInterfacePanelDirty = DIRTYLEVEL2;

		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


void RenderTownIDString( )
{
	CHAR16	zTownIDString[80];
	INT16 sFontX, sFontY;

	// Render town, position
	SetFont( COMPFONT );
	SetFontForeground( 183 );
	SetFontBackground( FONT_BLACK );

	GetSectorIDString( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, zTownIDString, lengthof(zTownIDString), TRUE );
	ReduceStringLength( zTownIDString, lengthof(zTownIDString), 80, COMPFONT );
	FindFontCenterCoordinates(548, 425, 80, 16, zTownIDString, COMPFONT, &sFontX, &sFontY);
	mprintf( sFontX, sFontY, L"%ls", zTownIDString );
}


void CheckForAndAddMercToTeamPanel( SOLDIERTYPE *pSoldier )
{

	if ( pSoldier->bActive  )
	{
		// Add to interface if the are ours
		if ( pSoldier->bTeam == gbPlayerNum )
		{
			// Are we in the loaded sector?
			if ( pSoldier->sSectorX == gWorldSectorX && pSoldier->sSectorY == gWorldSectorY && pSoldier->bSectorZ == gbWorldSectorZ && !pSoldier->fBetweenSectors && pSoldier->bInSector )
			{
				// IF on duty....
				if( ( pSoldier->bAssignment ==  CurrentSquad( ) )|| ( SoldierIsDeadAndWasOnSquad( pSoldier, ( INT8 )( CurrentSquad( ) ) ) ) )
				{

					if( pSoldier->bAssignment == ASSIGNMENT_DEAD )
					{
						pSoldier->fUICloseMerc = FALSE;
					}
					// ATE: ALrighty, if we have the insertion code of helicopter..... don't add just yet!
					/// ( will add in heli code )
					if ( pSoldier->ubStrategicInsertionCode != INSERTION_CODE_CHOPPER )
					{
						AddPlayerToInterfaceTeamSlot( pSoldier->ubID );
					}

					// ARE WE A VEHICLE.....
					if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
					{
						AddPassangersToTeamPanel( pSoldier->bVehicleID );
					}
				}
			}
			else
			{
				// Make sure we are NOT in this world!
				// Set gridno to one that's not visib;l
				RemoveSoldierFromGridNo( pSoldier );

				// Remove slot from tactical
				RemoveMercSlot( pSoldier );

			}
		}
	}
}


UINT8 FindNextMercInTeamPanel( SOLDIERTYPE *pSoldier, BOOLEAN fGoodForLessOKLife, BOOLEAN fOnlyRegularMercs )
{
	INT32 cnt;
  INT32 bFirstID;
	SOLDIERTYPE             *pTeamSoldier;


  bFirstID = GetTeamSlotFromPlayerID( pSoldier->ubID );

  if ( bFirstID == -1 )
  {
    return( pSoldier->ubID );
  }

	for ( cnt = ( bFirstID + 1 ); cnt < NUM_TEAM_SLOTS; cnt++ )
	{
	  if ( gTeamPanel[ cnt ].fOccupied )
	  {
		  // Set Id to close
		  pTeamSoldier = MercPtrs[ gTeamPanel[ cnt ].ubID ];

		  if ( fOnlyRegularMercs )
		  {
			  if ( pTeamSoldier->bActive && ( AM_AN_EPC( pTeamSoldier ) || AM_A_ROBOT( pTeamSoldier ) ) )
			  {
				  continue;
			  }
		  }

		  if ( fGoodForLessOKLife )
		  {
			  if ( pTeamSoldier->bLife > 0 && pTeamSoldier->bActive && pTeamSoldier->bInSector && pTeamSoldier->bTeam == gbPlayerNum && pTeamSoldier->bAssignment < ON_DUTY  && OK_INTERRUPT_MERC( pTeamSoldier ) && pSoldier->bAssignment == pTeamSoldier->bAssignment )
			  {
				  return( (UINT8)gTeamPanel[ cnt ].ubID );
			  }
		  }
		  else
		  {
			  if ( OK_CONTROLLABLE_MERC( pTeamSoldier) && OK_INTERRUPT_MERC( pTeamSoldier ) && pSoldier->bAssignment == pTeamSoldier->bAssignment )
			  {
				  return( (UINT8)gTeamPanel[ cnt ].ubID );
			  }
		  }
	  }
  }

	// none found,
	// Now loop back
	for ( cnt = 0; cnt < bFirstID; cnt++ )
	{
	  if ( gTeamPanel[ cnt ].fOccupied )
	  {
      pTeamSoldier = MercPtrs[ gTeamPanel[ cnt ].ubID ];

		  if ( fOnlyRegularMercs )
		  {
			  if ( pTeamSoldier->bActive && ( AM_AN_EPC( pTeamSoldier ) || AM_A_ROBOT( pTeamSoldier ) ) )
			  {
				  continue;
			  }
		  }

		  if ( fGoodForLessOKLife )
		  {
			  if ( pTeamSoldier->bLife > 0 && pTeamSoldier->bActive && pTeamSoldier->bInSector && pTeamSoldier->bTeam == gbPlayerNum && pTeamSoldier->bAssignment < ON_DUTY  && OK_INTERRUPT_MERC( pTeamSoldier ) && pSoldier->bAssignment == pTeamSoldier->bAssignment )
			  {
				  return( (UINT8)gTeamPanel[ cnt ].ubID );
			  }
		  }
		  else
		  {
			  if ( OK_CONTROLLABLE_MERC( pTeamSoldier) && OK_INTERRUPT_MERC( pTeamSoldier ) && pSoldier->bAssignment == pTeamSoldier->bAssignment )
			  {
				  return( (UINT8)gTeamPanel[ cnt ].ubID );
			  }
		  }
    }
	}

	// IF we are here, keep as we always were!
	return( pSoldier->ubID );

}


void DisableTacticalTeamPanelButtons( BOOLEAN fDisable )
{
	gfDisableTacticalPanelButtons = fDisable;
}


void BeginKeyPanelFromKeyShortcut( )
{
	SOLDIERTYPE *pSoldier = NULL;
	INT16 sStartYPosition = 0;
	INT16 sWidth = 0, sHeight = 0;

	if ( gsCurInterfacePanel != SM_PANEL )
	{
		return;
	}

	if ( gpSMCurrentMerc == NULL )
	{
		return;
	}

	if ( gfInKeyRingPopup )
	{
		return;
	}


	sStartYPosition = INV_INTERFACE_START_Y;
	sWidth  = SCREEN_WIDTH;
	sHeight = SCREEN_HEIGHT - INV_INTERFACE_START_Y;
	pSoldier = gpSMCurrentMerc;

	//if we are in the shop keeper interface
	if( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE )
		return;

	InitKeyRingPopup( pSoldier, 0, sStartYPosition, sWidth, sHeight );
}


void KeyRingItemPanelButtonCallback( MOUSE_REGION * pRegion, INT32 iReason )
{
	SOLDIERTYPE *pSoldier = NULL;
	INT16 sStartYPosition = 0;
	INT16 sWidth = 0, sHeight = 0;

	if( guiCurrentScreen == MAP_SCREEN )
	{
		if( bSelectedInfoChar == -1 )
		{
			return;
		}

		if( gCharactersList[ bSelectedInfoChar ].fValid == FALSE )
		{
			return;
		}




		pSoldier = &( Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ] );
		sStartYPosition = MAP_START_KEYRING_Y;
		sWidth = 261;
		sHeight = ( 359 - 107 );
	}
	else
	{
		if ( gpSMCurrentMerc == NULL )
		{
			return;
		}

		sStartYPosition = INV_INTERFACE_START_Y;
		sWidth   = SCREEN_WIDTH;
		sHeight  = SCREEN_HEIGHT - INV_INTERFACE_START_Y;
		pSoldier = gpSMCurrentMerc;
	}

	//if we are in the shop keeper interface
	if( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE )
		return;

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if( guiCurrentScreen == MAP_SCREEN )
		{
			// shade the background
			ShadowVideoSurfaceRect( FRAME_BUFFER , 0, 107, 261, 359 );
			InvalidateRegion( 0, 107, 261, 359 );
		}

		InitKeyRingPopup( pSoldier, 0, sStartYPosition, sWidth, sHeight );
	}
}

void KeyRingSlotInvClickCallback( MOUSE_REGION * pRegion, INT32 iReason )
{
	UINT32 uiKeyRing;
	// Copyies of values
	UINT16	usOldItemIndex;
	static BOOLEAN	fRightDown = FALSE;
	static BOOLEAN	fLeftDown = FALSE;
	INT32 iNumberOfKeysTaken = 0;

	uiKeyRing = MSYS_GetRegionUserData( pRegion, 0 );

	//if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	//{
	//	fLeftDown = TRUE;
	//}
	//else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP && fLeftDown )
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		fLeftDown = FALSE;

		//if we are in the shop keeper interface
		if( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE )
		{
			INVENTORY_IN_SLOT InvSlot;

			if( gMoveingItem.sItemIndex == 0 )
			{
				//Delete the contents of the item cursor
				memset( &gMoveingItem, 0, sizeof( INVENTORY_IN_SLOT ) );
			}
			else
			{
				memset( &InvSlot, 0, sizeof( INVENTORY_IN_SLOT ) );

				// Return if empty
				//if ( gpSMCurrentMerc->inv[ uiHandPos ].usItem == NOTHING )
				//	return;


				// Fill out the inv slot for the item
				//InvSlot.sItemIndex = gpSMCurrentMerc->inv[ uiHandPos ].usItem;
//			InvSlot.ubNumberOfItems = gpSMCurrentMerc->inv[ uiHandPos ].ubNumberOfObjects;
//			InvSlot.ubItemQuality = gpSMCurrentMerc->inv[ uiHandPos ].bGunStatus;
				//InvSlot.ItemObject = gpSMCurrentMerc->inv[uiHandPos];
				//InvSlot.ubLocationOfObject = PLAYERS_INVENTORY;

				//InvSlot.ubIdOfMercWhoOwnsTheItem = gpSMCurrentMerc->ubProfile;


				//Add the item to the Players Offer Area
				//AddItemToPlayersOfferArea( gpSMCurrentMerc->ubProfile, &InvSlot, (UINT8)uiHandPos );

				// Dirty
				fInterfacePanelDirty = DIRTYLEVEL2;
			}
			return;
		}

		// If we do not have an item in hand, start moving it
		if ( gpItemPointer == NULL )
		{
			// Return if empty
			if( ( gpItemPopupSoldier->pKeyRing[ uiKeyRing ].ubKeyID == INVALID_KEY_NUMBER ) || ( gpItemPopupSoldier->pKeyRing[ uiKeyRing ].ubNumber == 0 ) )
				return;

			// If our flags are set to do this, gofoit!
			if ( (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
			{
			}
			else
			{
				if ( gpItemPopupSoldier->ubID != gusSelectedSoldier )
				{
					SelectSoldier( gpItemPopupSoldier->ubID, FALSE, FALSE );
				}
			}

			usOldItemIndex =  ( UINT16 )uiKeyRing ;

			BeginKeyRingItemPointer( gpItemPopupSoldier, (UINT8)usOldItemIndex );
			//BeginItemPointer( gpSMCurrentMerc, (UINT8)uiHandPos );

		}
		else
		{
			UINT8			ubSrcID, ubDestID;
			BOOLEAN		fOKToGo = FALSE;
			BOOLEAN		fDeductPoints = FALSE;

			if( ( gpItemPointer ->usItem < FIRST_KEY ) || ( gpItemPointer ->usItem > KEY_32 ) )
			{
				return;
			}

			// ATE: OK, get source, dest guy if different... check for and then charge appropriate APs
			ubSrcID  = ( UINT8 )gCharactersList[ bSelectedInfoChar ].usSolID;
			if ( gpItemPointerSoldier )
			{
				ubDestID = gpItemPointerSoldier->ubID;
			}
			else
			{
				ubDestID = ubSrcID;
			}

			if ( ubSrcID == ubDestID )
			{
				// We are doing this ourselve, continue
				fOKToGo = TRUE;
			}
			else
			{
				// These guys are different....
				fDeductPoints = TRUE;

				// First check points for src guy
				if ( gpItemPointerSoldier->bLife >= CONSCIOUSNESS )
				{
					if ( EnoughPoints( gpItemPointerSoldier, 2, 0, TRUE ) )
					{
						fOKToGo = TRUE;
					}
				}
				else
				{
					fOKToGo = TRUE;
				}

				// Should we go on?
				if ( fOKToGo )
				{
					if ( gpSMCurrentMerc->bLife >= CONSCIOUSNESS )
					{
						if ( EnoughPoints( gpSMCurrentMerc, 2, 0, TRUE ) )
						{
							fOKToGo = TRUE;
						}
						else
						{
							fOKToGo = FALSE;
						}
					}
				}
			}

			if ( fOKToGo )
			{
				//usOldItemIndex = gpSMCurrentMerc->inv[ uiHandPos ].usItem;
				//usNewItemIndex = gpItemPointer->usItem;

				if ( gpItemPopupSoldier->pKeyRing[ uiKeyRing ].ubKeyID == INVALID_KEY_NUMBER || gpItemPopupSoldier->pKeyRing[ uiKeyRing ].ubKeyID == gpItemPointer->ubKeyID)
				{
					// Try to place here
					if ( ( iNumberOfKeysTaken = AddKeysToSlot( gpItemPopupSoldier, ( INT8 )uiKeyRing, gpItemPointer ) ) )
					{

						if ( fDeductPoints )
						{
							// Deduct points
							if ( gpItemPointerSoldier->bLife >= CONSCIOUSNESS )
							{
								DeductPoints( gpItemPointerSoldier,  2, 0 );
							}
							if ( gpItemPopupSoldier->bLife >= CONSCIOUSNESS )
							{
								DeductPoints( gpItemPopupSoldier,  2, 0 );
							}
						}

						// Dirty
						fInterfacePanelDirty = DIRTYLEVEL2;

						gpItemPointer->ubNumberOfObjects -= ( UINT8 )iNumberOfKeysTaken;

						// Check if it's the same now!
						if ( gpItemPointer->ubNumberOfObjects == 0 )
						{
							if ( guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN )
							{
								MAPEndItemPointer();
							}
							else
							{
								EndItemPointer();
							}
						}

						// Setup a timer....
						//guiMouseOverItemTime = GetJA2Clock( );
						//gfCheckForMouseOverItem = TRUE;
						//gbCheckForMouseOverItemPos = (INT8)uiHandPos;

					}
				}
				else
				{
					// Swap!
					SwapKeysToSlot( gpItemPopupSoldier, (INT8) uiKeyRing, gpItemPointer );

					if ( fDeductPoints )
					{
						// Deduct points
						if ( gpItemPointerSoldier && gpItemPointerSoldier->bLife >= CONSCIOUSNESS )
						{
							DeductPoints( gpItemPointerSoldier,  2, 0 );
						}
						if ( gpSMCurrentMerc->bLife >= CONSCIOUSNESS )
						{
							DeductPoints( gpSMCurrentMerc,  2, 0 );
						}
					}

					// Dirty
					fInterfacePanelDirty = DIRTYLEVEL2;

				}
			}
		}

		UpdateItemHatches();
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		fRightDown = TRUE;
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP && fRightDown )
	{
		fRightDown = FALSE;

		// Return if empty
		if( ( gpItemPopupSoldier->pKeyRing[ uiKeyRing ].ubKeyID == INVALID_KEY_NUMBER ) || ( gpItemPopupSoldier->pKeyRing[ uiKeyRing ].ubNumber == 0 ) )
		{
			DeleteKeyRingPopup( );
			fTeamPanelDirty = TRUE;
			return;
		}
		// Some global stuff here - for esc, etc
		// Check for # of slots in item
		if ( !InItemDescriptionBox( ) )
		{
			if ( guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN )
			{
				//InitKeyItemDescriptionBox( gpItemPopupSoldier, (UINT8)uiKeyRing, MAP_ITEMDESC_START_X, MAP_ITEMDESC_START_Y, 0 );
      }
      else
      {
				InitKeyItemDescriptionBox( gpItemPopupSoldier, (UINT8)uiKeyRing, SM_ITEMDESC_START_X, SM_ITEMDESC_START_Y, 0 );
      }
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		fRightDown = FALSE;
		fLeftDown = FALSE;
	}

}


void DisableSMPpanelButtonsWhenInShopKeeperInterface(void)
{
	//Go through the buttons that will be under the ShopKeepers ATM panel and disable them
	DisableButton( iSMPanelButtons[ STANCEUP_BUTTON ] );
	DisableButton( iSMPanelButtons[ UPDOWN_BUTTON ] );
	DisableButton( iSMPanelButtons[ CLIMB_BUTTON ] );
	DisableButton( iSMPanelButtons[ STANCEDOWN_BUTTON ] );
	DisableButton( iSMPanelButtons[ HANDCURSOR_BUTTON ] );
	DisableButton( iSMPanelButtons[ BURSTMODE_BUTTON ] );
	DisableButton( iSMPanelButtons[ LOOK_BUTTON ] );
	DisableButton( iSMPanelButtons[ TALK_BUTTON ] );
	DisableButton( iSMPanelButtons[ MUTE_BUTTON ] );

	DisableButton( giSMStealthButton );

	//Make sure the options button is disabled
	DisableButton( iSMPanelButtons[ OPTIONS_BUTTON ] );

	//Make sure the mapscreen button is disabled
	DisableButton( iSMPanelButtons[ SM_MAP_SCREEN_BUTTON ] );

	DisableButton( iSMPanelButtons[ STANCEUP_BUTTON ] );
	DisableButton( iSMPanelButtons[ UPDOWN_BUTTON ] );
	DisableButton( iSMPanelButtons[ CLIMB_BUTTON ] );
	DisableButton( iSMPanelButtons[ STANCEDOWN_BUTTON ] );
	DisableButton( iSMPanelButtons[ HANDCURSOR_BUTTON ] );
	DisableButton( iSMPanelButtons[ BURSTMODE_BUTTON ] );
	DisableButton( iSMPanelButtons[ LOOK_BUTTON ] );
	DisableButton( iSMPanelButtons[ TALK_BUTTON ] );
	DisableButton( iSMPanelButtons[ MUTE_BUTTON ] );

	DisableButton( giSMStealthButton );
}


static BOOLEAN IsMouseInRegion(MOUSE_REGION* pRegion)
{
	if ( (gusMouseXPos >= pRegion->RegionTopLeftX ) && (gusMouseXPos <= pRegion->RegionBottomRightX ) && (gusMouseYPos >= pRegion->RegionTopLeftY ) && (gusMouseYPos <= pRegion->RegionBottomRightY ) )
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


static void EnableButtonsForInItemBox(BOOLEAN fEnable)
{
	if ( !fEnable )
	{
		DisableButton( iSMPanelButtons[ NEXTMERC_BUTTON ] );
		DisableButton( iSMPanelButtons[ PREVMERC_BUTTON ] );
		DisableButton( iSMPanelButtons[ SM_MAP_SCREEN_BUTTON ] );
	}
	else
	{
		EnableButton( iSMPanelButtons[ NEXTMERC_BUTTON ] );
		EnableButton( iSMPanelButtons[ PREVMERC_BUTTON ] );

		if ( !gfDisableTacticalPanelButtons )
		{
			EnableButton( iSMPanelButtons[ SM_MAP_SCREEN_BUTTON ] );
		}
	}
}


static void ConfirmationToDepositMoneyToPlayersAccount(UINT8 ubExitValue);


static void SMInvMoneyButtonCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason == MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		//If the current merc is to far away, dont allow anything to be done
		if( gfSMDisableForItems )
			return;

		//if the player has an item in his hand,
		if( gpItemPointer != NULL )
		{
			//and the item is money
			if( Item[ gpItemPointer->usItem ].usItemClass == IC_MONEY )
			{
				CHAR16	zText[512];
				CHAR16	zMoney[64];

        // Make sure we go back to movement mode...
		    guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
		    HandleTacticalUI( );

				SPrintMoney(zMoney, gpItemPointer->uiMoneyAmount);

				//ask the user if they are sure they want to deposit the money
				swprintf( zText, lengthof(zText), gzMoneyWithdrawMessageText[ CONFIRMATION_TO_DEPOSIT_MONEY_TO_ACCOUNT ], zMoney );

				if( guiCurrentScreen == SHOPKEEPER_SCREEN )
				{
					//if we are in the shop keeper interface, free the cursor
					if( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE )
					{
						FreeMouseCursor();
					}

					DoMessageBox( MSG_BOX_BASIC_STYLE, zText, SHOPKEEPER_SCREEN, ( UINT8 )MSG_BOX_FLAG_YESNO, ConfirmationToDepositMoneyToPlayersAccount, NULL );
				}
				else
					DoMessageBox( MSG_BOX_BASIC_STYLE, zText, GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_YESNO, ConfirmationToDepositMoneyToPlayersAccount, NULL );
			}
		}

		//else bring up the money item description box to remove money from the players account
		else
		{
			//set the flag indicating we are removing money from the players account
			gfAddingMoneyToMercFromPlayersAccount = TRUE;

			//create the temp object from the players account balance
//			if( LaptopSaveInfo.iCurrentBalance > MAX_MONEY_PER_SLOT )
//				CreateMoney( MAX_MONEY_PER_SLOT, &gItemPointer );
//			else
				CreateMoney( LaptopSaveInfo.iCurrentBalance, &gItemPointer );

			InternalInitItemDescriptionBox( &gItemPointer, SM_ITEMDESC_START_X, SM_ITEMDESC_START_Y, 0, gpSMCurrentMerc );
		}
	}
}


static void ConfirmationToDepositMoneyToPlayersAccount(UINT8 ubExitValue)
{
	if ( ubExitValue == MSG_BOX_RETURN_YES )
	{
		//add the money to the players account
		AddTransactionToPlayersBook( MERC_DEPOSITED_MONEY_TO_PLAYER_ACCOUNT, gpSMCurrentMerc->ubProfile, GetWorldTotalMin(), gpItemPointer->uiMoneyAmount );

		// dirty shopkeeper
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;

		EndItemPointer( );
	}
}


void ReEvaluateDisabledINVPanelButtons( )
{
	gfReEvaluateDisabledINVPanelButtons = TRUE;
}


static void CheckForReEvaluateDisabledINVPanelButtons(void)
{
	// OK, if we currently have an item pointer up....
	if ( gfReEvaluateDisabledINVPanelButtons )
	{
		if ( gpItemPointer || gfInItemPickupMenu )
		{
			EnableSMPanelButtons( FALSE, TRUE );
		}
		else
		{
			EnableSMPanelButtons( TRUE, TRUE );
		}

		gfReEvaluateDisabledINVPanelButtons = FALSE;
	}
}


static void AbandonBoxingCallback(UINT8 ubExitValue)
{
	if ( ubExitValue == MSG_BOX_RETURN_YES )
	{
		// ok, proceed!
		SetBoxingState( NOT_BOXING );
		gfEnteringMapScreen = TRUE;
	}
	// otherwise do nothing
}

void GoToMapScreenFromTactical( void )
{
	if ( gTacticalStatus.bBoxingState != NOT_BOXING )
	{
		// pop up dialogue asking whether the player wants to abandon the fight
		DoMessageBox( MSG_BOX_BASIC_STYLE, Message[ STR_ABANDON_FIGHT ], GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_YESNO, AbandonBoxingCallback, NULL );
		return;
	}
	// ok, proceed!
	gfEnteringMapScreen = TRUE;
}


void HandleTacticalEffectsOfEquipmentChange( SOLDIERTYPE *pSoldier, UINT32 uiInvPos, UINT16 usOldItem, UINT16 usNewItem )
{
	// if in attached weapon mode and don't have weapon with GL attached in hand, reset weapon mode
	if ( pSoldier->bWeaponMode == WM_ATTACHED && FindAttachment( &(pSoldier->inv[ HANDPOS ]), UNDER_GLAUNCHER ) == NO_SLOT )
	{
		pSoldier->bWeaponMode = WM_NORMAL;
		pSoldier->bDoBurst = FALSE;
	}

	// if he is loaded tactically
	if ( pSoldier->bInSector )
	{
		// If this is our main hand
		if ( uiInvPos == HANDPOS || uiInvPos == SECONDHANDPOS )
		{
			// check if we need to change animation!
			ReLoadSoldierAnimationDueToHandItemChange( pSoldier, usOldItem, usNewItem );
		}

		// if this is head gear
		if ( uiInvPos == HEAD1POS || uiInvPos == HEAD2POS )
		{
			// Could be because of GOGGLES change...  Re-create light...
			DeleteSoldierLight( pSoldier );
			PositionSoldierLight( pSoldier );
		}
	}
	else
	{
		// as a minimum
		if ( (Item[ pSoldier->inv[ HANDPOS ].usItem ].usItemClass & IC_WEAPON) && Weapon[ pSoldier->inv[ HANDPOS ].usItem ].ubShotsPerBurst == 0 )
		{
			pSoldier->bDoBurst		= FALSE;
			pSoldier->bWeaponMode = WM_NORMAL;
		}
	}
}
