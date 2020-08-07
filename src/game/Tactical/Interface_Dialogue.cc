#include "Interface_Dialogue.h"

#include "AI.h"
#include "AIInternals.h"
#include "Animation_Control.h"
#include "Animation_Data.h"
#include "Arms_Dealer_Init.h"
#include "Assignments.h"
#include "BobbyRMailOrder.h"
#include "Boxing.h"
#include "Button_System.h"
#include "Campaign.h"
#include "Cheats.h"
#include "ContentManager.h"
#include "Cursors.h"
#include "Debug.h"
#include "Dialogue_Control.h"
#include "Directories.h"
#include "End_Game.h"
#include "Faces.h"
#include "Fade_Screen.h"
#include "Files.h"
#include "FindLocations.h"
#include "Font.h"
#include "Font_Control.h"
#include "GameInstance.h"
#include "GameLoop.h"
#include "GameRes.h"
#include "GameScreen.h"
#include "GameSettings.h"
#include "Game_Clock.h"
#include "Game_Event_Hook.h"
#include "HImage.h"
#include "Handle_Doors.h"
#include "Handle_Items.h"
#include "Handle_UI.h"
#include "History.h"
#include "Interactive_Tiles.h"
#include "Interface.h"
#include "Interface_Control.h"
#include "Interface_Panels.h"
#include "Isometric_Utils.h"
#include "Items.h"
#include "Keys.h"
#include "Local.h"
#include "Logger.h"
#include "MapScreen.h"
#include "Map_Screen_Helicopter.h"
#include "Map_Screen_Interface.h"
#include "Meanwhile.h"
#include "MercTextBox.h"
#include "Message.h"
#include "MessageBoxScreen.h"
#include "Morale.h"
#include "NPC.h"
#include "NpcActionParamsModel.h"
#include "OppList.h"
#include "Overhead.h"
#include "Personnel.h"
#include "PreBattle_Interface.h"
#include "Queen_Command.h"
#include "Quests.h"
#include "RenderWorld.h"
#include "Render_Dirty.h"
#include "Render_Fun.h"
#include "SaveLoadMap.h"
#include "ScreenIDs.h"
#include "ShopKeeper_Interface.h"
#include "SkillCheck.h"
#include "Soldier_Control.h"
#include "Soldier_Create.h"
#include "Soldier_Profile.h"
#include "Sound_Control.h"
#include "Squads.h"
#include "Strategic.h"
#include "StrategicMap.h"
#include "Strategic_AI.h"
#include "Strategic_Mines.h"
#include "Strategic_Movement.h"
#include "Strategic_Town_Loyalty.h"
#include "Structure.h"
#include "SysUtil.h"
#include "Tactical_Save.h"
#include "Text.h"
#include "TileDat.h"
#include "Timer_Control.h"
#include "UILayout.h"
#include "VObject.h"
#include "VSurface.h"
#include "Video.h"
#include "WCheck.h"
#include "WorldMan.h"

#include <string_theory/format>
#include <string_theory/string>


static const INT16 sBasementEnterGridNos[] = { 13362, 13363, 13364, 13365, 13525, 13524 };
static const INT16 sBasementExitGridNos[] = { 8047, 8207, 8208, 8048, 7888, 7728, 7727, 7567 };


#define TALK_PANEL_FACE_X			6
#define TALK_PANEL_FACE_Y			9
#define TALK_PANEL_NAME_X			5
#define TALK_PANEL_NAME_Y			114
#define TALK_PANEL_NAME_WIDTH			92
#define TALK_PANEL_NAME_HEIGHT			15
#define TALK_PANEL_REGION_STARTX		102
#define TALK_PANEL_REGION_STARTY		14
#define TALK_PANEL_REGION_SPACEY		16
#define TALK_PANEL_REGION_HEIGHT		12
#define TALK_PANEL_REGION_WIDTH		95

#define TALK_PANEL_MENUTEXT_STARTX		102
#define TALK_PANEL_MENUTEXT_STARTY		16
#define TALK_PANEL_MENUTEXT_SPACEY		16
#define TALK_PANEL_MENUTEXT_HEIGHT		13
#define TALK_PANEL_MENUTEXT_WIDTH		95
#define TALK_PANEL_BUTTON_X			112
#define TALK_PANEL_BUTTON_Y			114
#define TALK_PANEL_SHADOW_AREA_X		97
#define TALK_PANEL_SHADOW_AREA_Y		9
#define TALK_PANEL_SHADOW_AREA_WIDTH		107
#define TALK_PANEL_SHADOW_AREA_HEIGHT		102

#define TALK_PANEL_DEFAULT_SUBTITLE_WIDTH	200

#define TALK_PANEL_CALC_SUBTITLE_WIDTH		280
#define TALK_PANEL_CALC_SUBTITLE_HEIGHT	125


#define TALK_PANEL_POPUP_LEFT			0
#define TALK_PANEL_POPUP_TOP			1
#define TALK_PANEL_POPUP_BOTTOM		2
#define TALK_PANEL_POPUP_RIGHT			3

// chance vince will say random quote to player during conv.
#define CHANCE_FOR_DOCTOR_TO_SAY_RANDOM_QUOTE	20


static Approach const ubTalkMenuApproachIDs[] =
{
	APPROACH_REPEAT,
	APPROACH_FRIENDLY,
	APPROACH_DIRECT,
	APPROACH_THREATEN,
	APPROACH_BUYSELL,
	APPROACH_RECRUIT
};


// GLOBAL NPC STRUCT
NPC_DIALOGUE_TYPE gTalkPanel;
BOOLEAN gfInTalkPanel = FALSE;
SOLDIERTYPE *gpSrcSoldier  = NULL;
SOLDIERTYPE *gpDestSoldier  = NULL;
UINT8 gubSrcSoldierProfile;
static UINT8 gubNiceNPCProfile  = NO_PROFILE;
static UINT8 gubNastyNPCProfile = NO_PROFILE;

static UINT8        gubTargetNPC;
static UINT8        gubTargetRecord;
static Approach     gubTargetApproach;
static BOOLEAN      gfShowDialogueMenu;
BOOLEAN gfWaitingForTriggerTimer;
static UINT32       guiWaitingForTriggerTime;
MercPopUpBox*       g_interface_dialogue_box;
static UINT8        ubRecordThatTriggeredLiePrompt;
static BOOLEAN      gfConversationPending = FALSE;
static SOLDIERTYPE* gpPendingDestSoldier;
static SOLDIERTYPE* gpPendingSrcSoldier;
static Approach     gbPendingApproach;
static UINT8        g_pending_approach_record;
static OBJECTTYPE*  g_pending_approach_object;

INT32 giHospitalTempBalance; // stores amount of money for current doctoring
INT32 giHospitalRefund; // stores amount of money given to hospital for doctoring that wasn't used
INT8	gbHospitalPriceModifier; // stores discount being offered

enum
{
	HOSPITAL_UNSET = 0,
	HOSPITAL_NORMAL,
	HOSPITAL_BREAK,
	HOSPITAL_COST,
	HOSPITAL_FREEBIE,
	HOSPITAL_RANDOM_FREEBIE,
};


static void InternalInitiateConversation(SOLDIERTYPE* pDestSoldier, SOLDIERTYPE* pSrcSoldier, Approach, UINT8 approach_record, OBJECTTYPE* approach_object);


BOOLEAN InitiateConversationFull(SOLDIERTYPE* const pDestSoldier, SOLDIERTYPE* const pSrcSoldier, Approach const bApproach, UINT8 const approach_record, OBJECTTYPE* const approach_object)
try
{
	// ATE: OK, let's check the status of the Q
	// If it has something in it....delay this until after....
	if (DialogueQueueIsEmptyAndNobodyIsTalking())
	{
		gfConversationPending = FALSE;

		// Initiate directly....
		InternalInitiateConversation(pDestSoldier, pSrcSoldier, bApproach, approach_record, approach_object);
		return TRUE;
	}
	else
	{
		// Wait.....
		gfConversationPending = TRUE;

		gpPendingDestSoldier      = pDestSoldier;
		gpPendingSrcSoldier       = pSrcSoldier;
		gbPendingApproach         = bApproach;
		g_pending_approach_record = approach_record;
		g_pending_approach_object = approach_object;

		//Engaged on conv...
		gTacticalStatus.uiFlags |= ENGAGED_IN_CONV;

		// Turn off lock UI ( if on )
		guiPendingOverrideEvent = LU_ENDUILOCK;
		HandleTacticalUI( );

		// Turn on talking ui
		guiPendingOverrideEvent = TA_TALKINGMENU;
		HandleTacticalUI( );

		return( FALSE );
	}
}
catch (...) { return FALSE; /* XXX fishy, exception should probably propagate */ }


BOOLEAN InitiateConversation(SOLDIERTYPE* const pDestSoldier, SOLDIERTYPE* const pSrcSoldier, Approach const bApproach)
{
	return InitiateConversationFull(pDestSoldier, pSrcSoldier, bApproach, 0, 0);
}


void HandlePendingInitConv( )
{
	if ( gfConversationPending )
	{
		// OK, if pending, remove and now call........
		InternalInitiateConversation(gpPendingDestSoldier, gpPendingSrcSoldier, gbPendingApproach,
						g_pending_approach_record, g_pending_approach_object);
	}
}


static void InitTalkingMenu(UINT8 ubCharacterNum, INT16 sGridNo);


static void InternalInitiateConversation(SOLDIERTYPE* const pDestSoldier, SOLDIERTYPE* const pSrcSoldier, Approach const bApproach, UINT8 const approach_record, OBJECTTYPE* const approach_object)
{
	// OK, init talking menu
	BOOLEAN	fFromPending;

	fFromPending = gfConversationPending;

	// Set pending false
	gfConversationPending = FALSE;

	// ATE: If we are already in menu, delete!
	if ( gfInTalkPanel )
	{
		DeleteTalkingMenu( );
	}

	try
	{
		InitTalkingMenu(pDestSoldier->ubProfile, pDestSoldier->sGridNo);
	}
	catch (...)
	{
		// If failed, and we were pending, unlock UI
		if ( fFromPending )
		{
			gTacticalStatus.uiFlags &= (~ENGAGED_IN_CONV);
		}
		SLOGD("Cannot initiate conversation menu.. check for face file for ID: %d.", pDestSoldier->ubProfile );
		throw;
	}

	// Set soldier pointer
	gpSrcSoldier = pSrcSoldier;
	gpDestSoldier = pDestSoldier;

	// Say first line...
	// CHRIS LOOK HERE
	if ( gpSrcSoldier != NULL )
	{
		gubSrcSoldierProfile = gpSrcSoldier->ubProfile;
	}
	else
	{
		gubSrcSoldierProfile = NO_PROFILE;
	}

	// find which squad this guy is, then set selected squad to this guy
	if ( pSrcSoldier->bTeam == OUR_TEAM && gTacticalStatus.ubCurrentTeam == OUR_TEAM )
	{
		SetCurrentSquad( gpSrcSoldier->bAssignment, FALSE );

		SelectSoldier(pSrcSoldier, SELSOLDIER_NONE);
	}

	ConverseFull(gTalkPanel.ubCharNum, gubSrcSoldierProfile, bApproach, approach_record, approach_object);

	// If not from pedning...
	if ( !fFromPending )
	{
		//Engaged on conv...
		gTacticalStatus.uiFlags |= ENGAGED_IN_CONV;

		// Turn off lock UI ( if on )
		guiPendingOverrideEvent = LU_ENDUILOCK;
		HandleTacticalUI( );

		// Turn on talking ui
		guiPendingOverrideEvent = TA_TALKINGMENU;
		HandleTacticalUI( );
	}
	else
	{
		guiPendingOverrideEvent = TA_TALKINGMENU;
		HandleTacticalUI( );
	}
}


// This fuction will allocate and setup an NPCDiaogue structure. Loads the face for the character..
static void InitTalkingMenu(UINT8 const ubCharacterNum, INT16 const sGridNo)
{
	INT16 sXMapPos, sYMapPos, sScreenX, sScreenY;
	INT16 sX, sY;

	// Get XY values
	{
		// Get XY locations for gridno.
		ConvertGridNoToXY( sGridNo, &sXMapPos, &sYMapPos );

		// Get screen XY pos from map XY
		// Be carefull to convert to cell cords
		CellXYToScreenXY( (INT16)((sXMapPos*CELL_X_SIZE)), (INT16)((sYMapPos*CELL_Y_SIZE)), &sScreenX, &sScreenY);


		// First get mouse xy screen location
		sX = sScreenX;
		sY = sScreenY;

		InternalInitTalkingMenu(ubCharacterNum, sX, sY);
	}
}


static void CalculatePopupTextOrientation(INT16 sWidth, INT16 sHeight);
static void TalkPanelMoveCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void TalkPanelClickCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void TalkPanelBaseRegionClickCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void TalkPanelNameRegionClickCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void TalkPanelNameRegionMoveCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void DoneTalkingButtonClickCallback(GUI_BUTTON* btn, INT32 reason);


void InternalInitTalkingMenu(UINT8 const ubCharacterNum, INT16 sX, INT16 sY)
{
	INT16 sCenterYVal, sCenterXVal;

	// disable scroll messages
	HideMessagesDuringNPCDialogue( );

	//ATE: OK, let go of any other dialogues up.....
	EraseInterfaceMenus( FALSE );


	gTalkPanel.ubCharNum = ubCharacterNum;
	gTalkPanel.bCurSelect = -1;
	gTalkPanel.bOldCurSelect = -1;
	gTalkPanel.fHandled = FALSE;
	gTalkPanel.fOnName = FALSE;

	gTalkPanel.uiPanelVO = AddVideoObjectFromFile(INTERFACEDIR "/talkbox1.sti");

	ETRLEObject const& ETRLEProps = gTalkPanel.uiPanelVO->SubregionProperties(0);
	gTalkPanel.usWidth = ETRLEProps.usWidth;
	gTalkPanel.usHeight = ETRLEProps.usHeight;

	// Check coords
	{

		// CHECK FOR LEFT/RIGHT
		sCenterXVal = gTalkPanel.usWidth / 2;

		sX -= sCenterXVal;

		// Check right
		if (sX + gTalkPanel.usWidth > SCREEN_WIDTH)
		{
			sX = SCREEN_WIDTH - gTalkPanel.usWidth;
		}

		// Check left
		if ( sX < 0 )
		{
			sX = 0;
		}

		// Now check for top
		// Center in the y
		sCenterYVal = gTalkPanel.usHeight / 2;

		sY -= sCenterYVal;

		if ( sY < gsVIEWPORT_WINDOW_START_Y )
		{
			sY = gsVIEWPORT_WINDOW_START_Y;
		}

		// Check for bottom
		sY = MIN(sY, gsVIEWPORT_WINDOW_END_Y - gTalkPanel.usHeight);
	}

	//Set values
	gTalkPanel.sX = sX;
	gTalkPanel.sY = sY;

	CalculatePopupTextOrientation( TALK_PANEL_CALC_SUBTITLE_WIDTH, TALK_PANEL_CALC_SUBTITLE_HEIGHT );

	// Create face ( a big face! )....
	FACETYPE& f = InitFace(ubCharacterNum, 0, FACE_BIGFACE | FACE_POTENTIAL_KEYWAIT);
	gTalkPanel.face = &f;

	// Create mouse regions...
	sX = gTalkPanel.sX + TALK_PANEL_REGION_STARTX;
	sY = gTalkPanel.sY + TALK_PANEL_REGION_STARTY;


	//Define main region
	MSYS_DefineRegion(&gTalkPanel.ScreenRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);

	//Define main region
	MSYS_DefineRegion(&(gTalkPanel.BackRegion), (INT16)(gTalkPanel.sX), (INT16)(gTalkPanel.sY),
				(INT16)(gTalkPanel.sX + gTalkPanel.usWidth),
				(INT16)(gTalkPanel.sY + gTalkPanel.usHeight),
				MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL, MSYS_NO_CALLBACK,
				TalkPanelBaseRegionClickCallback);

	//Define name region
	MSYS_DefineRegion(&(gTalkPanel.NameRegion), (INT16)(gTalkPanel.sX + TALK_PANEL_NAME_X),
				(INT16)(gTalkPanel.sY + TALK_PANEL_NAME_Y),
				(INT16)(gTalkPanel.sX + TALK_PANEL_NAME_WIDTH + TALK_PANEL_NAME_X),
				(INT16)(gTalkPanel.sY + TALK_PANEL_NAME_HEIGHT + TALK_PANEL_NAME_Y), MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL, TalkPanelNameRegionMoveCallback,
				TalkPanelNameRegionClickCallback );

	for (INT32 cnt = 0; cnt < 6; ++cnt)
	{
		// Build a mouse region here that is over any others.....
		MSYS_DefineRegion(&(gTalkPanel.Regions[cnt]), (INT16)(sX), (INT16)(sY),
					(INT16)(sX + TALK_PANEL_REGION_WIDTH ),
					(INT16)( sY + TALK_PANEL_REGION_HEIGHT ), MSYS_PRIORITY_HIGHEST,
					CURSOR_NORMAL, TalkPanelMoveCallback, TalkPanelClickCallback);
		MSYS_SetRegionUserData( &(gTalkPanel.Regions[cnt]), 0, cnt );

		sY += TALK_PANEL_REGION_SPACEY;
	}

	// Build save buffer
	// Create a buffer for him to go!
	// OK, ignore screen widths, height, only use BPP
	gTalkPanel.uiSaveBuffer = AddVideoSurface(f.usFaceWidth, f.usFaceHeight, PIXEL_DEPTH);

	// Set face to auto
	SetAutoFaceActive(gTalkPanel.uiSaveBuffer, FACE_AUTO_RESTORE_BUFFER, f, 0, 0);
	f.uiFlags |= FACE_INACTIVE_HANDLED_ELSEWHERE;

	// Load buttons, create button
	gTalkPanel.iButtonImages = LoadButtonImage(INTERFACEDIR "/talkbox2.sti", 3, 4);

	gTalkPanel.uiCancelButton = CreateIconAndTextButton(gTalkPanel.iButtonImages, zDialogActions, MILITARYFONT1,
								33, DEFAULT_SHADOW, 33, DEFAULT_SHADOW,
								gTalkPanel.sX + TALK_PANEL_BUTTON_X,
								gTalkPanel.sY + TALK_PANEL_BUTTON_Y,
								MSYS_PRIORITY_HIGHEST,
								DoneTalkingButtonClickCallback );

	gTalkPanel.uiCancelButton->SpecifyHilitedTextColors(FONT_MCOLOR_WHITE, DEFAULT_SHADOW);

	// Turn off dirty flags
	gTalkPanel.uiCancelButton->uiFlags &= ~BUTTON_DIRTY;

	// Render once!
	RenderAutoFace(f);

	gfInTalkPanel = TRUE;

	gfIgnoreScrolling = TRUE;
}


static void DoneTalkingButtonClickCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// OK, pickup item....
		gTalkPanel.fHandled = TRUE;
		gTalkPanel.fHandledTalkingVal = gTalkPanel.face->fTalking;
		gTalkPanel.fHandledCanDeleteVal = TRUE;
	}
}


void DeleteTalkingMenu( )
{
	INT32 cnt;

	if ( !gfInTalkPanel )
		return;

	// Delete sound if playing....
	ShutupaYoFace(gTalkPanel.face);

	// Delete screen region
	MSYS_RemoveRegion( &(gTalkPanel.ScreenRegion));

	// Delete main region
	MSYS_RemoveRegion( &(gTalkPanel.BackRegion));

	// Delete name region
	MSYS_RemoveRegion( &(gTalkPanel.NameRegion));

	// Delete mouse regions
	for ( cnt = 0; cnt < 6; cnt++ )
	{
		MSYS_RemoveRegion( &(gTalkPanel.Regions[cnt]));
	}

	if ( gTalkPanel.fTextRegionOn )
	{
		// Remove
		MSYS_RemoveRegion( &(gTalkPanel.TextRegion) );
		gTalkPanel.fTextRegionOn = FALSE;
	}

	DeleteVideoSurface(gTalkPanel.uiSaveBuffer);

	// Remove video object
	DeleteVideoObject(gTalkPanel.uiPanelVO);

	// Remove face....
	DeleteFace(gTalkPanel.face);

	// Remove button
	RemoveButton( gTalkPanel.uiCancelButton );

	// Remove button images
	UnloadButtonImage( gTalkPanel.iButtonImages );

	// Set cursor back to normal mode...
	guiPendingOverrideEvent = A_CHANGE_TO_MOVE;

	// Rerender world
	SetRenderFlags( RENDER_FLAG_FULL );

	gfInTalkPanel = FALSE;

	gfIgnoreScrolling = FALSE;

	// Set this guy up as NOT engaged in conversation
	gpDestSoldier->uiStatusFlags &= (~SOLDIER_ENGAGEDINACTION);

	//NOT Engaged on conv...
	if ( !giNPCReferenceCount )
	{
		gTacticalStatus.uiFlags &= (~ENGAGED_IN_CONV);
	}

	// restore scroll messages in tactical
	UnHideMessagesDuringNPCDialogue( );

	if ( CheckFact( FACT_NEED_TO_SAY_SOMETHING, 0 ) )
	{
		if ( DialogueQueueIsEmpty() && !gfWaitingForTriggerTimer )
		{
			UINT8	ubNPC;
			BOOLEAN fNice = FALSE;

			if ( gubNiceNPCProfile != NO_PROFILE )
			{
				ubNPC = gubNiceNPCProfile;
				fNice = TRUE;
			}
			else
			{
				ubNPC = gubNastyNPCProfile;
			}

			if ( ubNPC != NO_PROFILE )
			{
				const SOLDIERTYPE* const pNPC = FindSoldierByProfileID(ubNPC);
				if (pNPC)
				{
					// find someone to say their "nice guy" line
					if ( fNice )
					{
						SayQuote58FromNearbyMercInSector( pNPC->sGridNo, 10, QUOTE_LISTEN_LIKABLE_PERSON, gMercProfiles[ ubNPC ].bSex );
					}
					else
					{
						SayQuoteFromNearbyMercInSector( pNPC->sGridNo, 10, QUOTE_ANNOYING_PC );
					}
					gubNiceNPCProfile = NO_PROFILE;
					gubNastyNPCProfile = NO_PROFILE;
					SetFactFalse( FACT_NEED_TO_SAY_SOMETHING );
				}
			}
		}
	}

	if (g_interface_dialogue_box)
	{
		RemoveMercPopupBox(g_interface_dialogue_box);
		g_interface_dialogue_box = 0;
	}
}


static void CalculatePopupTextPosition(INT16 sWidth, INT16 sHeight);
static void TextRegionClickCallback(MOUSE_REGION* pRegion, INT32 iReason);


void RenderTalkingMenu()
{
	NPC_DIALOGUE_TYPE* const tp  = &gTalkPanel;
	ProfileID          const pid = tp->ubCharNum;

	if (!gfInTalkPanel) return;

	// Render box!
	BltVideoObject(FRAME_BUFFER, tp->uiPanelVO, 0, tp->sX, tp->sY);

	// Render name
	SetFontAttributes(MILITARYFONT1, tp->fOnName ? FONT_WHITE : 33);
	ST::string name = GetProfile(pid).zNickname;
	INT16 sFontX;
	INT16 sFontY;
	FindFontCenterCoordinates(tp->sX + TALK_PANEL_NAME_X, tp->sY + TALK_PANEL_NAME_Y, TALK_PANEL_NAME_WIDTH, TALK_PANEL_NAME_HEIGHT, name, MILITARYFONT1, &sFontX, &sFontY);
	MPrint(sFontX, sFontY, name);

	SetFontShadow(DEFAULT_SHADOW);

	BltVideoSurface(FRAME_BUFFER, tp->uiSaveBuffer, tp->sX + TALK_PANEL_FACE_X, tp->sY + TALK_PANEL_FACE_Y, NULL);

	MarkButtonsDirty();

	// If guy is talking.... shadow area
	if (tp->face->fTalking || !DialogueQueueIsEmpty())
	{
		INT32 const x = tp->sX + TALK_PANEL_SHADOW_AREA_X;
		INT32 const y = tp->sY + TALK_PANEL_SHADOW_AREA_Y;
		FRAME_BUFFER->ShadowRect(x, y, x + TALK_PANEL_SHADOW_AREA_WIDTH, y + TALK_PANEL_SHADOW_AREA_HEIGHT);

		// Disable mouse regions....
		for (INT32 cnt = 0; cnt < 6; ++cnt)
		{
			tp->Regions[cnt].Disable();
		}

		DisableButton(tp->uiCancelButton);

		tp->bCurSelect = -1;
	}
	else
	{
		// Enable mouse regions....
		for (INT32 cnt = 0; cnt < 6; ++cnt)
		{
			tp->Regions[cnt].Enable();
		}

		EnableButton(tp->uiCancelButton);

		// Restore selection....
		tp->bCurSelect = tp->bOldCurSelect;
	}

	InvalidateRegion(tp->sX, tp->sY, tp->sX + tp->usWidth, tp->sY + tp->usHeight);

	if (tp->fSetupSubTitles)
	{
		if (g_interface_dialogue_box)
		{
			// Remove any old ones....
			RemoveMercPopupBox(g_interface_dialogue_box);
			g_interface_dialogue_box = 0;
		}

		UINT16 usTextBoxWidth;
		UINT16 usTextBoxHeight;
		g_interface_dialogue_box = PrepareMercPopupBox(0, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, tp->zQuoteStr, TALK_PANEL_DEFAULT_SUBTITLE_WIDTH, 0, 0, 0, &usTextBoxWidth, &usTextBoxHeight);
		SetFont(MILITARYFONT1); // PrepareMercPopupBox() overwrites the current font

		tp->fSetupSubTitles = FALSE;

		CalculatePopupTextOrientation(usTextBoxWidth, usTextBoxHeight);
		CalculatePopupTextPosition(   usTextBoxWidth, usTextBoxHeight);

		//Define main region
		if (tp->fTextRegionOn)
		{
			// Remove
			MSYS_RemoveRegion(&tp->TextRegion);
			tp->fTextRegionOn = FALSE;
		}

		UINT16 const x = tp->sPopupX;
		UINT16 const y = tp->sPopupY;
		MSYS_DefineRegion(&tp->TextRegion, x, y, x + usTextBoxWidth, y + usTextBoxHeight, MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL, MSYS_NO_CALLBACK, TextRegionClickCallback);

		tp->fTextRegionOn = TRUE;
	}

	if (tp->fRenderSubTitlesNow)
	{
		RenderMercPopUpBox(g_interface_dialogue_box, tp->sPopupX, tp->sPopupY,  FRAME_BUFFER);
	}

	// Create menu selections....
	INT16 const x = tp->sX + TALK_PANEL_MENUTEXT_STARTX;
	INT16       y = tp->sY + TALK_PANEL_MENUTEXT_STARTY;
	for (INT32 cnt = 0; cnt < 6; cnt++)
	{
		if (tp->bCurSelect == cnt)
		{
			SetFontForeground(FONT_WHITE);
			SetFontShadow(DEFAULT_SHADOW);
		}
		else
		{
			SetFontForeground(FONT_BLACK);
			SetFontShadow(MILITARY_SHADOW);
		}

		ST::string str;
		ST::string buf;
		if (cnt == 4 && IsMercADealer(pid))
		{
			str = zDealerStrings[GetTypeOfArmsDealer(GetArmsDealerIDFromMercID(pid))];
		}
		else if (cnt != 0 &&
#ifndef _DEBUG
			CHEATER_CHEAT_LEVEL() &&
#endif
			gubSrcSoldierProfile != NO_PROFILE &&
			pid                  != NO_PROFILE)
		{
			UINT8 const desire = CalcDesireToTalk(pid, gubSrcSoldierProfile, ubTalkMenuApproachIDs[cnt]);
			buf = ST::format("{} ({})", zTalkMenuStrings[cnt], desire);
			str = buf;
		}
		else
		{
			str = zTalkMenuStrings[cnt];
		}
		INT16 sFontX;
		INT16 sFontY;
		FindFontCenterCoordinates(x, y, TALK_PANEL_MENUTEXT_WIDTH, TALK_PANEL_MENUTEXT_HEIGHT, str, MILITARYFONT1, &sFontX, &sFontY);
		MPrint(sFontX, sFontY, str);

		y += TALK_PANEL_MENUTEXT_SPACEY;
	}

	SetFontShadow(DEFAULT_SHADOW);
}


static void TalkPanelMoveCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT32 uiItemPos;

	uiItemPos = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_MOVE)
	{
		// Set current selected guy....
		gTalkPanel.bCurSelect =(INT8)uiItemPos;
		gTalkPanel.bOldCurSelect = gTalkPanel.bCurSelect;
	}
	else if ( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		gTalkPanel.bCurSelect = -1;
		gTalkPanel.bOldCurSelect = -1;
	}

}


static void TalkPanelClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT32  uiItemPos;
	BOOLEAN fDoConverse = TRUE;
	uiItemPos = MSYS_GetRegionUserData( pRegion, 0 );


	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// Donot do this if we are talking already
		if (!gTalkPanel.face->fTalking)
		{
			if ( ubTalkMenuApproachIDs[ uiItemPos ] == APPROACH_BUYSELL )
			{
				//if its an arms dealer
				if( IsMercADealer( gTalkPanel.ubCharNum ) )
				{
					if ( NPCHasUnusedRecordWithGivenApproach( gTalkPanel.ubCharNum, APPROACH_BUYSELL ) )
					{
						TriggerNPCWithGivenApproach(gTalkPanel.ubCharNum, APPROACH_BUYSELL);
					}
					else
					{
						DeleteTalkingMenu( );

						//Enter the shopkeeper interface
						EnterShopKeeperInterfaceScreen( gTalkPanel.ubCharNum );
					}

					/*
					// check if this is a shopkeeper who has been shutdown
					if (!HandleShopKeepHasBeenShutDown(gTalkPanel.ubCharNum))
					{
						DeleteTalkingMenu( );

						EnterShopKeeperInterfaceScreen( gTalkPanel.ubCharNum );
					}*/
				}
				else
				{
					// Do something different if we selected the 'give' approach
					// Close panel, set UI guy to wait a sec, open inv if not done so yet
					gTalkPanel.fHandled = TRUE;
					gTalkPanel.fHandledTalkingVal = gTalkPanel.face->fTalking;
					gTalkPanel.fHandledCanDeleteVal = TRUE;

					// open inv panel...
					SetNewPanel(gpSrcSoldier);

					// Wait!
					gpDestSoldier->bNextAction = AI_ACTION_WAIT;
					gpDestSoldier->usNextActionData = 10000;

					// UNless he's has a pending action, delete what he was doing!
					// Cancel anything he was doing
					if ( gpDestSoldier->bAction != AI_ACTION_PENDING_ACTION )
					{
						CancelAIAction(gpDestSoldier);
					}
				}
			}
			else
			{
				if ( fDoConverse )
				{
					//Speak
					Converse(gTalkPanel.ubCharNum, gubSrcSoldierProfile, ubTalkMenuApproachIDs[uiItemPos]);
				}
			}
		}
	}
}


static void TalkPanelBaseRegionClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	static BOOLEAN fLButtonDown = FALSE;

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		fLButtonDown = TRUE;
	}

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP && fLButtonDown )
	{
		// Only do this if we are talking already
		if (gTalkPanel.face->fTalking)
		{
			// Stop speech, cancel
			InternalShutupaYoFace(gTalkPanel.face, FALSE);

			fLButtonDown = FALSE;
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		fLButtonDown = FALSE;
	}
}


static void TalkPanelNameRegionClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// Donot do this if we are talking already
		if (!gTalkPanel.face->fTalking)
		{
			// Say who are you?
			Converse(gTalkPanel.ubCharNum, gubSrcSoldierProfile, NPC_WHOAREYOU);
		}
	}
}


static void TalkPanelNameRegionMoveCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// Donot do this if we are talking already
	if (gTalkPanel.face->fTalking) return;

	if (iReason & MSYS_CALLBACK_REASON_MOVE)
	{
		// Set current selected guy....
		gTalkPanel.fOnName = TRUE;
	}
	else if ( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		gTalkPanel.fOnName = FALSE;
	}

}


BOOLEAN HandleTalkingMenu( )
{
	if ( !gfInTalkPanel )
	{
		return( FALSE );
	}

	if ( gTalkPanel.fHandled )
	{
		return( HandleTalkingMenuEscape( gTalkPanel.fHandledCanDeleteVal, FALSE ) );
	}

	return( FALSE );
}


void TalkingMenuDialogue(UINT16 const usQuoteNum)
{
	// Set back current select....
	gTalkPanel.bCurSelect = -1;
	gTalkPanel.fOnName = FALSE;
	//gTalkPanel.fHandled = FALSE;

	CharacterDialogue(gTalkPanel.ubCharNum, usQuoteNum, gTalkPanel.face, DIALOGUE_NPC_UI, FALSE);
}


bool ProfileCurrentlyTalkingInDialoguePanel(UINT8 const ubProfile)
{
	return gfInTalkPanel &&
		gpDestSoldier &&
		gpDestSoldier->ubProfile == ubProfile;
}


BOOLEAN HandleTalkingMenuEscape( BOOLEAN fCanDelete , BOOLEAN fFromEscKey )
{
	BOOLEAN fTalking = FALSE;

	if ( !gfInTalkPanel )
	{
		return( FALSE );
	}

	// If we are in the process of speaking, stop this quote an move on...
	// If we have been 'handled' by an outside source, check what was our talking value at the time
	if ( gTalkPanel.fHandled )
	{
		fTalking = gTalkPanel.fHandledTalkingVal;
	}
	else
	{
		fTalking = gTalkPanel.face->fTalking;
	}

	// Set to false
	gTalkPanel.fHandled = FALSE;

	if ( !fFromEscKey )
	{
		if ( fTalking )
		{
			ShutupaYoFace(gTalkPanel.face);
		}
		// Else if our queue is empty, delete emnu
		else
		{
			if ( DialogueQueueIsEmpty( ) && fCanDelete )
			{
				// Delete panel
				DeleteTalkingMenu( );
				// reset records which are on a can-say-once-per-convo basis
				ResetOncePerConvoRecordsForNPC( gpDestSoldier->ubProfile );
				return( TRUE );
			}
		}
	}
	else
	{
		if ( DialogueQueueIsEmpty( ) && fCanDelete )
		{
			// Delete panel
			DeleteTalkingMenu( );
			// reset records which are on a can-say-once-per-convo basis
			ResetOncePerConvoRecordsForNPC( gpDestSoldier->ubProfile );
			return( TRUE );
		}
	}
	return( FALSE );
}


void HandleTalkingMenuBackspace(void)
{
	if ( !gfInTalkPanel )
	{
		return;
	}

	const FACETYPE* const pFace = gTalkPanel.face;

	// If we are in the process of speaking, stop this quote an move on...
	// If we have been 'handled' by an outside source, check what was our talking value at the time
	/*
	BOOLEAN fTalking;
	if ( gTalkPanel.fHandled )
	{
		fTalking = gTalkPanel.fHandledTalkingVal;
	}
	else
	{
		fTalking = pFace->fTalking;
	}

	// Set to false
	gTalkPanel.fHandled = FALSE;


	if ( fTalking )	*/
	if (pFace->fTalking) ShutupaYoFace(gTalkPanel.face);
}


static void CalculatePopupTextOrientation(INT16 sWidth, INT16 sHeight)
{
	BOOLEAN fOKLeft = FALSE, fOKTop = FALSE, fOKBottom = FALSE, fOK = FALSE;
	INT16   sX, sY;

	// Check Left
	sX = gTalkPanel.sX - sWidth;

	if ( sX > 0 )
	{
		fOKLeft = TRUE;
	}

	// Check bottom
	sY = gTalkPanel.sY + sHeight + gTalkPanel.usHeight;

	if ( sY < 340 )
	{
		fOKBottom = TRUE;
	}

	// Check top
	sY = gTalkPanel.sY - sHeight;

	if ( sY > gsVIEWPORT_WINDOW_START_Y )
	{
		fOKTop = TRUE;
	}

	// OK, now decide where to put it!

	// First precidence is bottom
	if ( fOKBottom )
	{
		gTalkPanel.ubPopupOrientation = TALK_PANEL_POPUP_BOTTOM;

		fOK = TRUE;
	}

	if ( !fOK )
	{
		// Try left
		if ( fOKLeft )
		{
			// Our panel should not be heigher than our dialogue talking panel, so don't bother with the height checks!
			gTalkPanel.ubPopupOrientation = TALK_PANEL_POPUP_LEFT;
			fOK = TRUE;
		}

	}

	// Now at least top should work
	if ( !fOK )
	{
		// Try top!
		if ( fOKTop )
		{
			gTalkPanel.ubPopupOrientation = TALK_PANEL_POPUP_TOP;

			fOK = TRUE;
		}
	}

	// failed all the above
	if( !fOK )
	{
		// when all else fails go right
		gTalkPanel.ubPopupOrientation = TALK_PANEL_POPUP_RIGHT;
		fOK = TRUE;

	}
	// If we don't have anything here... our viewport/box is too BIG! ( which should never happen
}


static void CalculatePopupTextPosition(INT16 sWidth, INT16 sHeight)
{
	switch( gTalkPanel.ubPopupOrientation )
	{
		case TALK_PANEL_POPUP_LEFT:

			// Set it here!
			gTalkPanel.sPopupX = gTalkPanel.sX - sWidth;
			// Center in height!
			gTalkPanel.sPopupY = gTalkPanel.sY + ( gTalkPanel.usHeight / 2 ) -  ( sHeight / 2 );
			break;
		case TALK_PANEL_POPUP_RIGHT:
			// Set it here!
			gTalkPanel.sPopupX = gTalkPanel.sX + gTalkPanel.usWidth + 1;
			// Center in height!
			gTalkPanel.sPopupY = gTalkPanel.sY + ( gTalkPanel.usHeight / 2 ) -  ( sHeight / 2 );
			break;
		case TALK_PANEL_POPUP_BOTTOM:

			// Center in X
			gTalkPanel.sPopupX = gTalkPanel.sX + ( gTalkPanel.usWidth / 2 ) - ( sWidth / 2 );
			// Calc height
			gTalkPanel.sPopupY = gTalkPanel.sY + gTalkPanel.usHeight;
			break;

		case TALK_PANEL_POPUP_TOP:

			// Center in X
			gTalkPanel.sPopupX = gTalkPanel.sX + ( gTalkPanel.usWidth / 2 ) - ( sWidth / 2 );
			// Calc height
			gTalkPanel.sPopupY = gTalkPanel.sY - sHeight;
			break;
	}
}


static void HandleNPCGotoGridNo(UINT8 ubTargetNPC, UINT16 usGridNo, UINT8 ubQuoteNum);


void NPCGotoGridNo(ProfileID const ubTargetNPC, UINT16 const usGridNo, UINT8 const ubRecordNum)
{
	class DialogueEventGotoGridno : public DialogueEvent
	{
		public:
			DialogueEventGotoGridno(ProfileID const npc, GridNo const gridno, UINT8 const record) :
				gridno_(gridno),
				npc_(npc),
				record_(record)
			{}

			bool Execute()
			{
				HandleNPCGotoGridNo(npc_, gridno_, record_);
				return false;
			}

		private:
			GridNo    const gridno_;
			ProfileID const npc_;
			UINT8     const record_;
	};

	DialogueEvent::Add(new DialogueEventGotoGridno(ubTargetNPC, usGridNo, ubRecordNum));
}


void NPCDoAction(UINT8 const ubTargetNPC, UINT16 const usActionCode, UINT8 const ubQuoteNum)
{
	class DialogueEventDoAction : public DialogueEvent
	{
		public:
			DialogueEventDoAction(ProfileID npc, UINT16 const action, UINT8 const quote) :
				action_(action),
				npc_(npc),
				quote_(quote)
			{}

			bool Execute()
			{
				HandleNPCDoAction(npc_, action_, quote_);
				return false;
			}

		private:
			UINT16    const action_;
			ProfileID const npc_;
			UINT8     const quote_;
	};

	DialogueEvent::Add(new DialogueEventDoAction(ubTargetNPC, usActionCode, ubQuoteNum));
}


static BOOLEAN SourceSoldierPointerIsValidAndReachableForGive(SOLDIERTYPE* pGiver)
{
	INT16 sAdjGridNo;

	if ( !gpSrcSoldier )
	{
		return( FALSE );
	}
	if ( !gpSrcSoldier->bActive || !gpSrcSoldier->bInSector )
	{
		return( FALSE );
	}
	if ( gpSrcSoldier->bLife < OKLIFE || ( gpSrcSoldier->bBreath < OKBREATH && gpSrcSoldier->bCollapsed ) )
	{
		return( FALSE );
	}

	if ( !pGiver )
	{
		return( FALSE );
	}

	// pointer should always be valid anyhow
	if ( PythSpacesAway( pGiver->sGridNo, gpSrcSoldier->sGridNo ) > MaxDistanceVisible() )
	{
		return( FALSE );
	}

	sAdjGridNo = FindAdjacentGridEx( pGiver, gpSrcSoldier->sGridNo, NULL, NULL, FALSE, FALSE );
	if ( sAdjGridNo == -1 )
	{
		return( FALSE );
	}

	return( TRUE );
}


void HandleNPCItemGiven( UINT8 ubNPC, OBJECTTYPE *pObject, INT8 bInvPos )
{

	// Give it to the NPC soldier
	//AutoPlaceObject( gpDestSoldier, pObject, FALSE );

	// OK< if the timer is < 5000, use who was last in the talk panel box.
	if ( !SourceSoldierPointerIsValidAndReachableForGive( gpDestSoldier ) )
	{
		// just drop it

		// have to walk up to the merc closest to ubNPC
		SOLDIERTYPE* const pNPC = FindSoldierByProfileID(ubNPC);
		if ( pNPC )
		{
			AddItemToPool(pNPC->sGridNo, &pNPC->inv[bInvPos], VISIBLE, 0, 0, 0);
			TriggerNPCWithGivenApproach(ubNPC, APPROACH_DONE_GIVING_ITEM);
		}
	}
	else
	{
		// Remove dialogue!
		DeleteTalkingMenu( );

		// Give this to buddy!
		SoldierGiveItem( gpDestSoldier, gpSrcSoldier, pObject, bInvPos );
	}

}


static void HandleNPCTrigger(void);


void HandleNPCTriggerNPC(UINT8 const ubTargetNPC, UINT8 const ubTargetRecord, BOOLEAN const fShowDialogueMenu, Approach const ubTargetApproach)
{
	SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
	if ( pSoldier == NULL )
	{
		return;
	}

	// Save values for trigger function
	gubTargetNPC = ubTargetNPC;
	gubTargetRecord = ubTargetRecord;
	gubTargetApproach = ubTargetApproach;
	gfShowDialogueMenu = fShowDialogueMenu;


	if ( pSoldier->bTeam == OUR_TEAM )
	{
		// make sure they are in the right alert status to receive orders (it's a bug that
		// this could be set for the player...)
		pSoldier->bAlertStatus = STATUS_GREEN;
		pSoldier->uiStatusFlags &= (~SOLDIER_ENGAGEDINACTION );
	}

	// OH BOY, CHECK IF THIS IS THE SAME PERSON WHO IS ON THE MENU
	// RIGHT NOW, IF SO , HANDLE DIRECTLY WITHOUT THIS DELAY!
	// IF WE ARE TO DISPLAY MENU AS WELL....
	if ( gfInTalkPanel )
	{
		if ( gpDestSoldier == pSoldier && fShowDialogueMenu )
		{
			HandleNPCTrigger( );
			return;
		}
	}

	if ( fShowDialogueMenu )
	{

		// ALRIGHTY, NOW DO THIS!
		// WAIT IN NON-INTERACTIVE MODE UNTIL TIMER IS DONE
		// SO WE CAN SEE NPC RADIO LOCATOR
		// THEN AFTER THIS IS DONE, DO THE STUFF
		// Setup timer!
		gfWaitingForTriggerTimer = TRUE;
		guiWaitingForTriggerTime = GetJA2Clock( );

		// Setup locator!
		ShowRadioLocator(pSoldier, SHOW_LOCATOR_FAST);

		// If he's visible, locate...
		if (pSoldier->bVisible != -1) LocateSoldier(pSoldier, FALSE);

		guiPendingOverrideEvent = LU_BEGINUILOCK;

	}
	else
	{
		HandleNPCTrigger( );
	}

	// Being already here, we know that this is not the guy whose panel is up.
	// Only close the panel if dialogue is involved in the new record.
	if ( RecordHasDialogue( ubTargetNPC, ubTargetRecord ) )
	{
		// Shutdown any panel we had up...
		DeleteTalkingMenu( );
	}
}


static void HandleNPCTrigger(void)
{
	INT16 sPlayerGridNo;

	SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(gubTargetNPC);
	if (!pSoldier)
	{
		return;
	}

	if ( gfInTalkPanel )
	{
		if ( pSoldier == gpDestSoldier )
		{
			if ( gfShowDialogueMenu )
			{
				// Converse another way!
				ConverseFull(gubTargetNPC, NO_PROFILE, gubTargetApproach, gubTargetRecord, 0);
			}
			else if (gpSrcSoldier != NULL) // if we can, reinitialize menu
			{
				InitiateConversationFull(gpDestSoldier, gpSrcSoldier, gubTargetApproach, gubTargetRecord, 0);
			}
		}
		else
		{
			ConverseFull(gubTargetNPC, NO_PROFILE, gubTargetApproach, gubTargetRecord, 0);
		}
	}
	else
	{
		// Now start new one...
		if ( gfShowDialogueMenu )
		{
			if ( SourceSoldierPointerIsValidAndReachableForGive( pSoldier ) )
			{
				InitiateConversationFull(pSoldier, gpSrcSoldier, gubTargetApproach, gubTargetRecord, 0);
					return;
			}
			else
			{
				sPlayerGridNo = ClosestPC( pSoldier, NULL );
				if (sPlayerGridNo != NOWHERE )
				{
					SOLDIERTYPE* const player = WhoIsThere2(sPlayerGridNo, 0);
					if (player != NULL)
					{
						InitiateConversationFull(pSoldier, player, gubTargetApproach, gubTargetRecord, 0);
						return;
					}
				}
			}
			// else
			InitiateConversationFull(pSoldier, pSoldier, gubTargetApproach, gubTargetRecord, 0);
		}
		else
		{
			// Converse another way!
			ConverseFull(gubTargetNPC, NO_PROFILE, gubTargetApproach, gubTargetRecord, 0);
		}
	}

}


void HandleWaitTimerForNPCTrigger( )
{
	if ( gfWaitingForTriggerTimer )
	{
		if ( ( GetJA2Clock( ) - guiWaitingForTriggerTime ) > 500 )
		{
			guiPendingOverrideEvent = LU_ENDUILOCK;

			UIHandleLUIEndLock( NULL );

			HandleNPCTrigger( );

			gfWaitingForTriggerTimer = FALSE;
		}

	}

}


static void HandleNPCGotoGridNo(UINT8 const ubTargetNPC, UINT16 const usGridNo, UINT8 const ubQuoteNum)
{
	// OK, Move to gridNo!

	// Shotdown any panel we had up...
	DeleteTalkingMenu( );

	// Get merc id for NPC
	SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
	if ( !pSoldier )
	{
		return;
	}

	// zap any delay in this soldier
	ZEROTIMECOUNTER( pSoldier->AICounter );
	if (pSoldier->bNextAction == AI_ACTION_WAIT)
	{
		pSoldier->bNextAction = AI_ACTION_NONE;
		pSoldier->usNextActionData = 0;
	}

	// if player controlled, set under AI control flag
	if (pSoldier->bTeam == OUR_TEAM)
	{
		pSoldier->uiStatusFlags |= SOLDIER_PCUNDERAICONTROL;
	}

	// OK, set in motion!
	pSoldier->bNextAction = AI_ACTION_WALK;

	// Set dest!
	pSoldier->usNextActionData = usGridNo;

	// UNless he's has a pending action, delete what he was doing!
	// Cancel anything he was doing
	if ( pSoldier->bAction != AI_ACTION_PENDING_ACTION )
	{
		CancelAIAction(pSoldier);
	}
	// Go for it!

	// Set flags to do stuff once there...
	pSoldier->ubQuoteRecord = ( ubQuoteNum + 1 );
	pSoldier->ubQuoteActionID = ActionIDForMovementRecord( ubTargetNPC, ubQuoteNum );

	// Set absolute dest
	pSoldier->sAbsoluteFinalDestination = usGridNo;

	// handle this guy's AI right away so that we can get him moving
	pSoldier->fAIFlags |= AI_HANDLE_EVERY_FRAME;
}


static void HandleStuffForNPCEscorted(UINT8 ubNPC)
{
	switch( ubNPC )
	{
		case MARIA:
			break;
		case JOEY:
			break;
		case SKYRIDER:
			SetFactTrue( FACT_SKYRIDER_EVER_ESCORTED );
			if ( gubQuest[ QUEST_ESCORT_SKYRIDER ] == QUESTNOTSTARTED )
			{
				StartQuest( QUEST_ESCORT_SKYRIDER, gWorldSectorX, gWorldSectorY );
			}
			break;
		case JOHN:
		{
			// recruit Mary as well
			RecruitEPC( MARY );

			SOLDIERTYPE* const pSoldier = FindSoldierByProfileIDOnPlayerTeam(MARY);
			if ( pSoldier )
			{
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(TacticalStr[ NOW_BING_ESCORTED_STR ], gMercProfiles[ MARY ].zNickname, ( pSoldier->bAssignment + 1 )) );
			}

			if ( gubQuest[ QUEST_ESCORT_TOURISTS ] == QUESTNOTSTARTED )
			{
				StartQuest( QUEST_ESCORT_TOURISTS, gWorldSectorX, gWorldSectorY );
			}
			break;
		}

		case MARY:
		{
			// recruit John as well
			RecruitEPC( JOHN );

			SOLDIERTYPE* const pSoldier = FindSoldierByProfileIDOnPlayerTeam(JOHN);
			if ( pSoldier )
			{
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(TacticalStr[ NOW_BING_ESCORTED_STR ], gMercProfiles[ JOHN ].zNickname, ( pSoldier->bAssignment + 1 )) );
			}

			if ( gubQuest[ QUEST_ESCORT_TOURISTS ] == QUESTNOTSTARTED )
			{
				StartQuest( QUEST_ESCORT_TOURISTS, gWorldSectorX, gWorldSectorY );
			}
			break;
		}
	}
}


static void StartDialogueMessageBox(UINT8 ubProfileID, UINT16 usMessageBoxType);
static BOOLEAN NPCOpenThing(SOLDIERTYPE* pSoldier, BOOLEAN fDoor);
static void DoneFadeOutActionBasement(void);
static void DoneFadeOutActionLeaveBasement(void);
static void CarmenLeavesSectorCallback(void);


void HandleNPCDoAction( UINT8 ubTargetNPC, UINT16 usActionCode, UINT8 ubQuoteNum )
{
	//ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, ST::format("Handling {}, action {} at {}", gMercProfiles[ubTargetNPC].zNickname, usActionCode, GetJA2Clock());

	// Switch on action code!
	if (usActionCode > NPC_ACTION_TURN_TO_FACE_NEAREST_MERC && usActionCode < NPC_ACTION_LAST_TURN_TO_FACE_PROFILE)
	{
		SOLDIERTYPE* const pSoldier  = FindSoldierByProfileID(ubTargetNPC);
		SOLDIERTYPE* const pSoldier2 = FindSoldierByProfileID((UINT8)(usActionCode - NPC_ACTION_TURN_TO_FACE_NEAREST_MERC));
		if (pSoldier && pSoldier2)
		{
			// see if we are facing this person
			const UINT8 ubDesiredMercDir = GetDirectionToGridNoFromGridNo(pSoldier->sGridNo, pSoldier2->sGridNo);
			// if not already facing in that direction,
			if (pSoldier->bDirection != ubDesiredMercDir)
			{
				EVENT_SetSoldierDesiredDirection( pSoldier, ubDesiredMercDir );
			}
		}
	}
	else
	{
		auto params = GCM->getNpcActionParams(usActionCode);
		switch( usActionCode )
		{
			case NPC_ACTION_DONT_ACCEPT_ITEM:
				// do nothing; this is just to skip annoying debug msgs
				break;

			case NPC_ACTION_GOTO_HIDEOUT:
			{
				// OK, we want to goto the basement level!

				//DEF: First thing, Add the exit grid to the map temps file
				EXITGRID ExitGrid;
				ExitGrid.ubGotoSectorX = 10;
				ExitGrid.ubGotoSectorY = 1;
				ExitGrid.ubGotoSectorZ = 1;
				ExitGrid.usGridNo = 12722;

				{ ApplyMapChangesToMapTempFile app;
					AddExitGridToWorld( params->getGridNo(7887), &ExitGrid );
				}

				// For one, loop through our current squad and move them over
				//ATE:Alrighty, instead of being a dufuss here, let's actually use the current
				// Squad here to search for...

				INT8 bNumDone = 0;
				FOR_EACH_IN_TEAM(pSoldier, OUR_TEAM)
				{
					// Are we in this sector, On the current squad?
					if (pSoldier->bLife >= OKLIFE && pSoldier->bInSector && pSoldier->bAssignment == CurrentSquad())
					{
						gfTacticalTraversal = TRUE;
						SetGroupSectorValue(10, 1, 1, *GetGroup(pSoldier->ubGroupID));

						// Set insertion gridno
						if ( bNumDone < 6 )
						{
							// Set next sectore
							pSoldier->sSectorX = 10;
							pSoldier->sSectorY = 1;
							pSoldier->bSectorZ = 1;

							// Set gridno
							pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
							pSoldier->usStrategicInsertionData = sBasementEnterGridNos[ bNumDone ];
						}
						bNumDone++;
					}
				}

				// MOVE NPCS!
				ChangeNpcToDifferentSector(GetProfile(FATIMA),  10, 1, 1);
				ChangeNpcToDifferentSector(GetProfile(DIMITRI), 10, 1, 1);

				gFadeOutDoneCallback = DoneFadeOutActionBasement;

				FadeOutGameScreen( );
				break;
			}

			case NPC_ACTION_FATIMA_GIVE_LETTER:
			{
				// Delete menu, give item to megual
				DeleteTalkingMenu( );

				SOLDIERTYPE* const pSoldier  = FindSoldierByProfileID(FATIMA);
				SOLDIERTYPE* const pSoldier2 = FindSoldierByProfileID(MIGUEL);

				// Give item!
				if ( !pSoldier || !pSoldier2 )
				{
					return;
				}

				// Look for letter....
				{
					INT8 bInvPos = FindObj(pSoldier, LETTER);

					AssertMsg( bInvPos != NO_SLOT, "Interface Dialogue.C:  Gift item does not exist in NPC." );

					SoldierGiveItem( pSoldier, pSoldier2, &(pSoldier->inv[ bInvPos ] ), bInvPos );
				}
				break;
			}

			case NPC_ACTION_FACE_CLOSEST_PLAYER:
			{
				// Get pointer for player
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if ( !pSoldier )
				{
					return;
				}

				pSoldier->ubQuoteRecord = ubQuoteNum;
				pSoldier->ubQuoteActionID = QUOTE_ACTION_ID_TURNTOWARDSPLAYER;

				// handle AI for this person every frame until a player merc is near
				pSoldier->fAIFlags |= AI_HANDLE_EVERY_FRAME;
				break;
			}

			case NPC_ACTION_OPEN_CLOSEST_DOOR:
			{
				// Delete menu
				DeleteTalkingMenu( );

				// Get pointer to NPC
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if ( !pSoldier )
				{
					return;
				}


				// FInd, open Closest Door
				NPCOpenThing( pSoldier, TRUE );

				break;
			}

			case NPC_ACTION_LOWER_GUN:
			{
				// Get pointer for player
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if ( !pSoldier )
				{
					return;
				}

				if (pSoldier->inv[HANDPOS].usItem != NOTHING)
				{
					UINT16	usGun;
					INT8		bNewSlot;

					usGun = pSoldier->inv[HANDPOS].usItem;

					ReLoadSoldierAnimationDueToHandItemChange( pSoldier, pSoldier->inv[HANDPOS].usItem, NOTHING );
					AutoPlaceObject( pSoldier, &(pSoldier->inv[HANDPOS]), FALSE );

					bNewSlot = FindObj( pSoldier, usGun );
					if (bNewSlot != NO_SLOT)
					{
						MERCPROFILESTRUCT& p = GetProfile(ubTargetNPC);
						if (p.inv[bNewSlot] == NOTHING)
						{
							// find where the gun is stored in the profile data and
							// move it to the new location
							INT8 const bOldSlot = FindObjectInSoldierProfile(p, usGun);
							if ( bOldSlot != NO_SLOT  )
							{
								// rearrange profile... NB # of guns can only be 1 so this is easy
								p.inv[bOldSlot]        = NOTHING;
								p.bInvNumber[bOldSlot] = 0;

								p.inv[bNewSlot]        = usGun;
								p.bInvNumber[bNewSlot] = 1;
							}
						}
					}
				}
				break;
			}

			case NPC_ACTION_READY_GUN:
			{
				// Get pointer for player
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if (pSoldier && pSoldier->inv[HANDPOS].usItem != NOTHING)
				{
					const INT16 sGridNo = pSoldier->sGridNo + DirectionInc(pSoldier->bDirection);
					SoldierReadyWeapon(pSoldier, sGridNo, FALSE);
				}
				break;
			}

			case NPC_ACTION_START_RUNNING:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if (pSoldier)
				{
					pSoldier->fUIMovementFast = TRUE;
				}
				break;
			}

			case NPC_ACTION_STOP_RUNNING:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if (pSoldier)
				{
					pSoldier->fUIMovementFast = FALSE;
				}
				break;
			}

			case NPC_ACTION_BOOST_TOWN_LOYALTY:
				HandleLoyaltyChangeForNPCAction( ubTargetNPC );
				break;

			// case NPC_ACTION_PENALIZE_TOWN_LOYALTY:
			// break;

			case NPC_ACTION_STOP_PLAYER_GIVING_FIRST_AID:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if (pSoldier)
				{
					ReceivingSoldierCancelServices( pSoldier );
				}
				break;
			}

			case NPC_ACTION_FACE_NORTH:
			{
				// handle this separately to keep the code clean...
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if (pSoldier)
				{
					SendSoldierSetDesiredDirectionEvent( pSoldier, NORTHWEST );
				}
				break;
			}

			case NPC_ACTION_FACE_NORTH_EAST:
			case NPC_ACTION_FACE_EAST:
			case NPC_ACTION_FACE_SOUTH_EAST:
			case NPC_ACTION_FACE_SOUTH:
			case NPC_ACTION_FACE_SOUTH_WEST:
			case NPC_ACTION_FACE_WEST:
			case NPC_ACTION_FACE_NORTH_WEST:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if (pSoldier)
				{
					// screen NORTHEAST corresponds to in-game NORTH
					SendSoldierSetDesiredDirectionEvent( pSoldier, (UINT16) (NORTH + (usActionCode - NPC_ACTION_FACE_NORTH_EAST)) );
				}
				break;
			}

			case NPC_ACTION_RECRUIT:
				// gonna work for free!
				gMercProfiles[ ubTargetNPC ].sSalary = 0;
				// fallthrough

			case NPC_ACTION_RECRUIT_WITH_SALARY:

				// Delete menu
				DeleteTalkingMenu( );

				if ( PlayerTeamFull( ) )
				{
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ CANNOT_RECRUIT_TEAM_FULL ] );
				}
				else
				{
					RecruitRPC( ubTargetNPC );
					// OK, update UI with message that we have been recruited
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(TacticalStr[ HAS_BEEN_RECRUITED_STR ], gMercProfiles[ ubTargetNPC ].zNickname) );
				}
				break;

			case NPC_ACTION_BECOME_ENEMY:
			{
				// CJC: disable because of new system?
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if ( !pSoldier )
				{
					return;
				}

				if (pSoldier->ubCivilianGroup)
				{
					CivilianGroupMemberChangesSides( pSoldier );
				}
				else
				{
					//MakeCivHostile( pSoldier, 2 );
				}
				if (pSoldier->ubProfile != NO_PROFILE && pSoldier->bLife >= OKLIFE)
				{
					// trigger quote!
					//TriggerNPCWithIHateYouQuote( pSoldier->ubProfile );
					AddToShouldBecomeHostileOrSayQuoteList(pSoldier);
				}
				break;
			}

			case NPC_ACTION_CLOSE_DIALOGUE_PANEL:
				DeleteTalkingMenu();
				break;

			case NPC_ACTION_TRIGGER_FRIEND_WITH_HOSTILE_QUOTE:
				// CJC: disabled because of new system
				//TriggerFriendWithHostileQuote( ubTargetNPC );
				DeleteTalkingMenu();
				break;

			case NPC_ACTION_LEAVE_HIDEOUT:
			{
				// Delete menu, give item to megual
				DeleteTalkingMenu( );

				//if we are already leaving the sector, dont leave the sector again
				if( gubWaitingForAllMercsToExitCode != 0 )
					return;

				// OK, we want to goto the basement level!
				// For one, loop through our current squad and move them over
				INT8 bNumDone = 0;
				FOR_EACH_IN_TEAM(pSoldier, OUR_TEAM)
				{
					// Are we in this sector, On the current squad?
					if (pSoldier->bLife >= OKLIFE && pSoldier->bInSector)
					{
						gfTacticalTraversal = TRUE;
						SetGroupSectorValue(10, 1, 0, *GetGroup(pSoldier->ubGroupID));

						// Set insertion gridno
						if ( bNumDone < 8 )
						{
							// Set next sectore
							pSoldier->sSectorX = 10;
							pSoldier->sSectorY = 1;
							pSoldier->bSectorZ = 0;

							// Set gridno
							pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
							pSoldier->usStrategicInsertionData = sBasementExitGridNos[ bNumDone ];
						}
						bNumDone++;
					}
				}

				gFadeOutDoneCallback = DoneFadeOutActionLeaveBasement;

				FadeOutGameScreen( );

				// turn off engaged in conv stuff
				gTacticalStatus.uiFlags &= ~ENGAGED_IN_CONV;
				// Decrement refrence count...
				giNPCReferenceCount = 0;
				break;
			}

			case NPC_ACTION_TRAVERSE_MAP_EAST:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if ( !pSoldier )
				{
					return;
				}

				pSoldier->ubQuoteActionID = QUOTE_ACTION_ID_TRAVERSE_EAST;
				break;
			}

			case NPC_ACTION_TRAVERSE_MAP_SOUTH:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if ( !pSoldier )
				{
					return;
				}

				pSoldier->ubQuoteActionID = QUOTE_ACTION_ID_TRAVERSE_SOUTH;
				break;
			}

			case NPC_ACTION_TRAVERSE_MAP_WEST:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if ( !pSoldier )
				{
					return;
				}

				pSoldier->ubQuoteActionID = QUOTE_ACTION_ID_TRAVERSE_WEST;
				break;
			}

			case NPC_ACTION_TRAVERSE_MAP_NORTH:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if ( !pSoldier )
				{
					return;
				}

				pSoldier->ubQuoteActionID = QUOTE_ACTION_ID_TRAVERSE_NORTH;
				break;
			}

			case NPC_ACTION_REPORT_SHIPMENT_SIZE:
				// for now, hard-coded to small shipment...
				// medium is quote 14, large is 15
				if (CheckFact( FACT_MEDIUM_SIZED_SHIPMENT_WAITING, 0 ))
				{
					TalkingMenuDialogue( 14 );
				}
				else if (CheckFact( FACT_LARGE_SIZED_SHIPMENT_WAITING, 0 ))
				{
					TalkingMenuDialogue( 15 );
				}
				else
				{
					TalkingMenuDialogue( 13 );
				}
				if (CheckFact( FACT_PABLOS_BRIBED, 0 ) && Random( 100 ) < 75)
				{
					TalkingMenuDialogue( 16 );
					if (Random( 100 ) < 75)
					{
						TalkingMenuDialogue( 21 );
					}
				}
				break;

			case NPC_ACTION_RETURN_STOLEN_SHIPMENT_ITEMS:
			{
				AddFutureDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, 480 + Random( 60 ), NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS + NPC_ACTION_RETURN_STOLEN_SHIPMENT_ITEMS, 1 );
				// also make Pablo neutral again and exit combat if we're in combat
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if ( !pSoldier )
				{
					return;
				}
				if ( !pSoldier->bNeutral )
				{
					DeleteTalkingMenu();
					SetSoldierNeutral( pSoldier );
					pSoldier->bBleeding = 0; // make sure he doesn't bleed now...
					RecalculateOppCntsDueToBecomingNeutral( pSoldier );
					if ( gTacticalStatus.uiFlags & INCOMBAT )
					{
						CheckForEndOfCombatMode( FALSE );
					}
				}
				break;
			}

			case NPC_ACTION_THREATENINGLY_RAISE_GUN:
			{
				// Get pointer for NPC
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if ( !pSoldier )
				{
					return;
				}

				const INT8 bItemIn = FindAIUsableObjClass(pSoldier, IC_GUN);
				if (bItemIn != NO_SLOT && bItemIn != HANDPOS)
				{
					SwapObjs( &(pSoldier->inv[HANDPOS]), &(pSoldier->inv[bItemIn]) );
					const INT16 sGridNo = pSoldier->sGridNo + DirectionInc(pSoldier->bDirection);
					SoldierReadyWeapon(pSoldier, sGridNo, FALSE);
				}
				// and face the nearest merc!
			}
				// fallthrough

			case NPC_ACTION_TURN_TO_FACE_NEAREST_MERC:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if (pSoldier)
				{
					const INT16 sGridNo = ClosestPC(pSoldier, NULL);
					if (sGridNo != NOWHERE)
					{
						// see if we are facing this person
						const UINT8 ubDesiredMercDir = GetDirectionToGridNoFromGridNo(pSoldier->sGridNo, sGridNo);
						// if not already facing in that direction,
						if (pSoldier->bDirection != ubDesiredMercDir)
						{
							EVENT_SetSoldierDesiredDirection( pSoldier, ubDesiredMercDir );
						}
					}
				}
				break;
			}

			case NPC_ACTION_SEND_PACOS_INTO_HIDING:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(PACOS);
				const INT16 sGridNo = params->getGridNo(16028);
				if (pSoldier)
				{
					if (NewOKDestination( pSoldier, sGridNo, TRUE, 0 ) )
					{
						// go for it!
						NPCGotoGridNo(PACOS, sGridNo, ubQuoteNum);
					}
					else
					{
						const INT16 sAdjustedGridNo = FindAdjacentGridEx(pSoldier, sGridNo, NULL, NULL, FALSE, FALSE);
						if (sAdjustedGridNo != -1)
						{
							NPCGotoGridNo(PACOS, sAdjustedGridNo, ubQuoteNum);
						}
					}
				}
				break;
			}

			case NPC_ACTION_HAVE_PACOS_FOLLOW:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(PACOS);
				const INT16 sGridNo = params->getGridNo(18193);
				if (pSoldier)
				{
					if (NewOKDestination( pSoldier, sGridNo, TRUE, 0 ) )
					{
						// go for it!
						NPCGotoGridNo(PACOS, sGridNo, ubQuoteNum);
					}
					else
					{
						const INT16 sAdjustedGridNo = FindAdjacentGridEx(pSoldier, sGridNo, NULL, NULL, FALSE, FALSE);
						if (sAdjustedGridNo != -1)
						{
							NPCGotoGridNo(PACOS, sAdjustedGridNo, ubQuoteNum);
						}
					}
				}
				break;
			}

			case NPC_ACTION_SET_DELAYED_PACKAGE_TIMER:
				AddFutureDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, GetWorldMinutesInDay(), FACT_SHIPMENT_DELAYED_24_HOURS, 1 );
				break;

			case NPC_ACTION_SET_RANDOM_PACKAGE_DAMAGE_TIMER:
				AddFutureDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, GetWorldMinutesInDay(), NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS + NPC_ACTION_SET_RANDOM_PACKAGE_DAMAGE_TIMER, 1 );
				break;

			case NPC_ACTION_ENABLE_CAMBRIA_DOCTOR_BONUS:
			{
				// add event in a
				const INT32 iRandom = Random(24);
				AddFutureDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, ( GetWorldMinutesInDay() + ( ( 24 + iRandom ) * 60 ) ), NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS + NPC_ACTION_ENABLE_CAMBRIA_DOCTOR_BONUS, 1 );
				break;
			}

			case NPC_ACTION_TRIGGER_END_OF_FOOD_QUEST:
				AddHistoryToPlayersLog( HISTORY_TALKED_TO_FATHER_WALKER, 0, GetWorldTotalMin(), gWorldSectorX, gWorldSectorY );
				AddFutureDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, GetWorldMinutesInDay(), NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS + NPC_ACTION_TRIGGER_END_OF_FOOD_QUEST, 1 );
				break;

			case NPC_ACTION_REMOVE_CONRAD:
				break;

			case NPC_ACTION_REDUCE_CONRAD_SALARY_CONDITIONS:
				gMercProfiles[ CONRAD ].sSalary = 3300;
				// fallthrough
			case NPC_ACTION_ASK_ABOUT_ESCORTING_EPC:
				// Confirm if we want to start escorting or not....
			case NPC_ACTION_ASK_ABOUT_PAYING_RPC:
			case NPC_ACTION_ASK_ABOUT_PAYING_RPC_WITH_DAILY_SALARY:
				// Ask whether the player will pay the RPC
			case NPC_ACTION_DARREN_REQUESTOR:
				// Darren asks if the player wants to fight
			case NPC_ACTION_FIGHT_AGAIN_REQUESTOR:
				// Darren asks if the player wants to fight again
			case NPC_ACTION_BUY_LEATHER_KEVLAR_VEST: // from Angel
			case NPC_ACTION_MEDICAL_REQUESTOR: // at hospital
			case NPC_ACTION_MEDICAL_REQUESTOR_2: // at hospital
			case NPC_ACTION_BUY_VEHICLE_REQUESTOR: // from Dave
			case NPC_ACTION_KROTT_REQUESTOR:
				// Vince or Willis asks about payment? for medical attention
				if (ubTargetNPC != gpDestSoldier->ubProfile)
				{
					SLOGE("Inconsistency between HandleNPCDoAction and target profile IDs" );
				}
				else
				{
					DeleteTalkingMenu();
					StartDialogueMessageBox( ubTargetNPC, usActionCode );
				}
				break;

			case NPC_ACTION_REPORT_BALANCE:
				ScreenMsg( FONT_YELLOW, MSG_INTERFACE, st_format_printf(TacticalStr[ BALANCE_OWED_STR ], gMercProfiles[ubTargetNPC].zNickname, -gMercProfiles[ubTargetNPC].iBalance) );
				break;

			case NPC_ACTION_DELAYED_MAKE_BRENDA_LEAVE:
				// set event to invoke Brenda's (#85) record 9 a minute from
				// now
				DeleteTalkingMenu();
				AddSameDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, GetWorldMinutesInDay() + 360, NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS + NPC_ACTION_DELAYED_MAKE_BRENDA_LEAVE );
				break;

			case NPC_ACTION_SEX:
				// Delete menu
				//DeleteTalkingMenu( );
				//gFadeOutDoneCallback = DoneFadeOutActionSex;
				//FadeOutGameScreen( );
				SetPendingNewScreen( SEX_SCREEN );
				break;

			case NPC_ACTION_KYLE_GETS_MONEY:
			{
				// add a money item with $10000 to the tile in front of Kyle
				// and then have him pick it up
				{
					OBJECTTYPE Object;
					INT16      sGridNo  = params->getGridNo(14952);
					UINT32     uiAmount = params->getAmount(10000);
					SLOGI(ST::format("add a money item with ${} to tile {} in front of Kyle", uiAmount, sGridNo));

					SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
					if (pSoldier)
					{
						CreateMoney(uiAmount, &Object);
						INT32 const iWorldItem = AddItemToPool(sGridNo, &Object, INVISIBLE, pSoldier->bLevel, 0, 0);

						// shouldn't have any current action but make sure everything
						// is clear... and set pending action so the guy won't move
						// until the pickup is completed
						CancelAIAction(pSoldier);
						pSoldier->bAction = AI_ACTION_PENDING_ACTION;
						pSoldier->ubQuoteRecord = NPC_ACTION_KYLE_GETS_MONEY;

						SoldierPickupItem( pSoldier, iWorldItem, sGridNo, ITEM_IGNORE_Z_LEVEL );
					}
				}
				break;
			}

			case NPC_ACTION_DRINK_DRINK_DRINK:
				gMercProfiles[ ubTargetNPC ].bNPCData++;
				break;

			case NPC_ACTION_MARTHA_DIES:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(MARTHA);
				if (pSoldier)
				{
					DeleteTalkingMenu();
					EVENT_SoldierGotHit(pSoldier, 1, 100, 10, pSoldier->bDirection, 320, NULL, FIRE_WEAPON_NO_SPECIAL, AIM_SHOT_TORSO, NOWHERE);
				}
				break;
			}

			//case NPC_ACTION_DARREN_GIVEN_CASH:
			// Handled in NPC.c
			//	break;

			case NPC_ACTION_START_BOXING_MATCH:
				if ( CheckOnBoxers() )
				{
					DeleteTalkingMenu();
					PlayLocationJA2Sample(11237, BOXING_BELL, MIDVOLUME, 5);
					ClearAllBoxerFlags();
					SetBoxingState( BOXING_WAITING_FOR_PLAYER );
				}
				break;

			case NPC_ACTION_ADD_JOEY_TO_WORLD:
				gMercProfiles[ JOEY ].sSectorX = 4;
				gMercProfiles[ JOEY ].sSectorY = MAP_ROW_D;
				gMercProfiles[ JOEY ].bSectorZ = 1;
				AddFutureDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, GetWorldMinutesInDay(), NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS + NPC_ACTION_ADD_JOEY_TO_WORLD, 3 );
				break;

			case NPC_ACTION_MARK_KINGPIN_QUOTE_0_USED:
				// set Kingpin's quote 0 as used so he doesn't introduce himself
				GetProfile(KINGPIN).ubLastDateSpokenTo = GetWorldDay();
				break;

			case NPC_ACTION_TRIGGER_LAYLA_13_14_OR_15:
				if (CheckFact( FACT_CARLA_AVAILABLE, 0 ))
				{
					TriggerNPCRecord(MADAME, 13);
				}
				else if (CheckFact( FACT_CINDY_AVAILABLE, 0))
				{
					TriggerNPCRecord(MADAME, 14);
				}
				else if (CheckFact( FACT_BAMBI_AVAILABLE, 0))
				{
					TriggerNPCRecord(MADAME, 15);
				}
				break;

			{
			case NPC_ACTION_OPEN_CARLAS_DOOR: 
			case NPC_ACTION_OPEN_CINDYS_DOOR: 
			case NPC_ACTION_OPEN_BAMBIS_DOOR: 
			case NPC_ACTION_OPEN_MARIAS_DOOR: 
				INT16 sGridNo = params->getGridNo(10852);

				// JA3Gold: unlock the doors instead of opening them
				{
					DOOR* pDoor;

					pDoor = FindDoorInfoAtGridNo( sGridNo );
					if (pDoor)
					{
						pDoor->fLocked = FALSE;
					}
				}
				/*
				// handle door opening here
				{
					STRUCTURE * pStructure;
					pStructure = FindStructure( sGridNo, STRUCTURE_ANYDOOR );
					if (pStructure)
					{



						pStructure->

						if (pStructure->fFlags & STRUCTURE_OPEN)
						{
							// it's already open - this MIGHT be an error but probably not
							// because we are basically just ensuring that the door is open
						}
						else
						{
							if (pStructure->fFlags & STRUCTURE_BASE_TILE)
							{
								HandleDoorChangeFromGridNo( NULL, sGridNo, FALSE );
							}
							else
							{
								HandleDoorChangeFromGridNo( NULL, pStructure->sBaseGridNo, FALSE );
							}
						}
					}
				}*/
				break;
			}

			case NPC_ACTION_POSSIBLY_ADVERTISE_CINDY:
				if ( CheckFact( FACT_CINDY_AVAILABLE, 0 ) )
				{
					TriggerNPCRecord( MADAME, 14 );
				}
				// else fall through
			case NPC_ACTION_POSSIBLY_ADVERTISE_BAMBI:
				if ( CheckFact( FACT_BAMBI_AVAILABLE, 0 ) )
				{
					TriggerNPCRecord( MADAME, 15 );
				}
				break;

			case NPC_ACTION_LAYLA_GIVEN_WRONG_AMOUNT_OF_CASH:
				if ( CheckFact( FACT_NO_GIRLS_AVAILABLE, 0 ) )
				{
					TriggerNPCRecord( MADAME, 27 );
				}
				// else mention girls available...
				else if ( CheckFact( FACT_CARLA_AVAILABLE, 0 ) )
				{
					// Mention Carla
					TriggerNPCRecord( MADAME, 28 );
					break;
				}
				// else
				// fallthrough
			case NPC_ACTION_LAYLAS_NEXT_LINE_AFTER_CARLA:
				if ( CheckFact( FACT_CINDY_AVAILABLE, 0 ) )
				{
					// Mention Cindy
					TriggerNPCRecord( MADAME, 29 );
					break;
				}
				// else
				// fallthrough
			case NPC_ACTION_LAYLAS_NEXT_LINE_AFTER_CINDY:
				if ( CheckFact( FACT_BAMBI_AVAILABLE, 0 ) )
				{
					// Mention Bambi
					TriggerNPCRecord( MADAME, 30 );
					break;
				}
				// else
				// fallthrough
			case NPC_ACTION_LAYLAS_NEXT_LINE_AFTER_BAMBI:
				if ( gubQuest[ QUEST_RESCUE_MARIA ] == QUESTINPROGRESS )
				{
					// Mention Maria
					TriggerNPCRecord( MADAME, 31);
					break;
				}
				// else
				// fallthrough
			case NPC_ACTION_LAYLAS_NEXT_LINE_AFTER_MARIA:
				if ( CheckFact( FACT_MULTIPLE_MERCS_CLOSE, MADAME ) )
				{
					// if more than 1 merc nearby, say comment about 2 guys pergirl
					TriggerNPCRecord( MADAME, 32 );
				}

				break;

			case NPC_ACTION_PROMPT_PLAYER_TO_LIE:
				//
				ubRecordThatTriggeredLiePrompt = ubQuoteNum;
				DeleteTalkingMenu();

				StartDialogueMessageBox( ubTargetNPC, usActionCode );
				break;

			case NPC_ACTION_REMOVE_JOE_QUEEN:
			{
				// Find queen and joe and remove from sector...
				SOLDIERTYPE* const queen = FindSoldierByProfileID(QUEEN);
				if (queen) TacticalRemoveSoldier(*queen);

				SOLDIERTYPE* const joe = FindSoldierByProfileID(JOE);
				if (joe) TacticalRemoveSoldier(*joe);
				break;
			}

			case NPC_ACTION_REMOVE_ELLIOT_END_MEANWHILE:
			{
				SOLDIERTYPE* const elliot = FindSoldierByProfileID(ELLIOT);
				if (elliot) TacticalRemoveSoldier(*elliot);

				// End meanwhile....
				// End meanwhile....
				DeleteTalkingMenu();
				EndMeanwhile( );
				break;
			}

			case NPC_ACTION_NO_SCI_FI_END_MEANWHILE:

				if ( !( gGameOptions.fSciFi ) )
				{
					// End meanwhile....
					// End meanwhile....
					DeleteTalkingMenu();
					EndMeanwhile( );
				}
				else
				{
					TriggerNPCRecord( QUEEN, 8 );
				}
			break;
			case NPC_ACTION_TRIGGER_MARRY_DARYL_PROMPT:
				DeleteTalkingMenu();
				StartDialogueMessageBox( ubTargetNPC, usActionCode );
				break;

			case NPC_ACTION_HAVE_MARRIED_NPC_LEAVE_TEAM:
			{
				// get the soldier
				SOLDIERTYPE* const pSoldier  = FindSoldierByProfileID(gMercProfiles[DARYL].bNPCData);
				SOLDIERTYPE* const pSoldier2 = gpDestSoldier;

				if ( !pSoldier || !pSoldier2 )
				{
					return;
				}

				//set the fact that the merc is being married ( used in the personnel screen )
				gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags2 |= PROFILE_MISC_FLAG2_MARRIED_TO_HICKS;

				AddHistoryToPlayersLog( HISTORY_MERC_MARRIED_OFF, pSoldier->ubProfile, GetWorldTotalMin(), gWorldSectorX, gWorldSectorY );

				// if Flo is going off with Daryl, then set that fact true
				if (pSoldier->ubProfile == FLO)
				{
					SetFactTrue( FACT_PC_MARRYING_DARYL_IS_FLO );
				}

				HandleMoraleEvent( pSoldier, MORALE_MERC_MARRIED, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );

				UpdateDarrelScriptToGoTo( pSoldier );
				TriggerNPCRecord( DARREL, 10 );

				//Since the married merc is leaving the team, add them to the departed list for the personnel screen
				AddCharacterToOtherList( pSoldier );
				break;
			}

			case NPC_ACTION_TRIGGER_ANGEL_17_OR_18:
				if (CheckFact( FACT_ANGEL_SOLD_VEST, ANGEL ))
				{
					TriggerNPCRecord( ANGEL, 18 );
				}
				else
				{
					TriggerNPCRecord( ANGEL, 17 );
				}
				break;

			case NPC_ACTION_TRIGGER_ANGEL_16_OR_19:
				if (CheckFact( FACT_PLAYER_IN_SAME_ROOM, ANGEL ) )
				{
					TriggerNPCRecord( ANGEL, 16 );
				}
				else
				{
					TriggerNPCRecord( ANGEL, 19 );
				}
				break;

			case NPC_ACTION_TRIGGER_ANGEL_21_OR_22:
				if (CheckFact( FACT_PLAYER_IN_SAME_ROOM, ANGEL ) )
				{
					TriggerNPCRecord( ANGEL, 22 );
				}
				else
				{
					TriggerNPCRecord( ANGEL, 21 );
				}
				break;

			case NPC_ACTION_TRIGGER_MARIA:
				if (!CheckFact(FACT_MARIA_ESCAPE_NOTICED, MARIA))
				{
					TriggerNPCRecord( MARIA, 8 );
				}
				else
				{
					if ( CheckFact( FACT_PLAYER_IN_SAME_ROOM, MARIA ) )
					{
						TriggerNPCRecord( MARIA, 9 );
					}
					else
					{
						TriggerNPCRecord( MARIA, 10 );
					}
				}
				break;

			case NPC_ACTION_ANGEL_LEAVES_DEED:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if (pSoldier)
				{
					const INT8 bItemIn = FindObj(pSoldier, DEED);
					if (bItemIn != NO_SLOT)
					{
						AddItemToPool(params->getGridNo(12541), &pSoldier->inv[bItemIn], INVISIBLE, 0, 0, 0);
						DeleteObj( &(pSoldier->inv[ bItemIn ]) );
						RemoveObjectFromSoldierProfile( ubTargetNPC, DEED );
					}
				}
				DeleteTalkingMenu();
				NPCDoAction( ubTargetNPC, NPC_ACTION_UN_RECRUIT_EPC, 0 );
				NPCDoAction( ubTargetNPC, NPC_ACTION_GRANT_EXPERIENCE_3, 0 );
				TriggerNPCRecord( ANGEL, 24 );
				break;
			}

			case NPC_ACTION_SET_GIRLS_AVAILABLE:

				// Initially assume none available
				SetFactTrue( FACT_NO_GIRLS_AVAILABLE );

				// Carla... available 66% of the time
				if ( Random( 3 ) )
				{
					SetFactTrue( FACT_CARLA_AVAILABLE );
					SetFactFalse( FACT_NO_GIRLS_AVAILABLE );
				}
				else
				{
					SetFactFalse( FACT_CARLA_AVAILABLE );
				}
				// Cindy... available 50% of time
				if ( Random( 2 ) )
				{
					SetFactTrue( FACT_CINDY_AVAILABLE );
					SetFactFalse( FACT_NO_GIRLS_AVAILABLE );
				}
				else
				{
					SetFactFalse( FACT_CINDY_AVAILABLE );
				}
				// Bambi... available 50% of time
				if ( Random( 2 ) )
				{
					SetFactTrue( FACT_BAMBI_AVAILABLE );
					SetFactFalse( FACT_NO_GIRLS_AVAILABLE );
				}
				else
				{
					SetFactFalse( FACT_BAMBI_AVAILABLE );
				}
				break;

			case NPC_ACTION_SET_DELAY_TILL_GIRLS_AVAILABLE:
				AddSameDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, GetWorldMinutesInDay() + 1 + Random( 2 ), NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS + NPC_ACTION_SET_DELAY_TILL_GIRLS_AVAILABLE );
				break;

			case NPC_ACTION_SET_WAITED_FOR_GIRL_FALSE:
				SetFactFalse( FACT_PLAYER_WAITED_FOR_GIRL );
				break;

			case NPC_ACTION_UN_RECRUIT_EPC:
			case NPC_ACTION_SET_EPC_TO_NPC:
				if ( ubTargetNPC == ANGEL )
				{
					// hack to Mari!
					ubTargetNPC = MARIA;
				}
				UnRecruitEPC( ubTargetNPC );
				switch( ubTargetNPC )
				{
					case MARY:
						UnRecruitEPC( JOHN );
						break;
					case JOHN:
						UnRecruitEPC( MARY );
						break;
				}
				break;

			case NPC_ACTION_REMOVE_DOREEN:
				// make Doreen disappear next time we do a sector traversal
				gMercProfiles[ DOREEN ].sSectorX = 0;
				gMercProfiles[ DOREEN ].sSectorY = 0;
				gMercProfiles[ DOREEN ].bSectorZ = 0;
				break;

			case NPC_ACTION_FREE_KIDS:
				AddFutureDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, 420, NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS + NPC_ACTION_FREE_KIDS, 1 );
				break;

			case NPC_ACTION_TRIGGER_FATHER_18_20_OR_15:
				if (!CheckFact(FACT_132, FATHER))
				{
					TriggerNPCRecord( FATHER, 18 );
				}
				else if (!CheckFact(FACT_133, FATHER))
				{
					TriggerNPCRecord( FATHER, 20 );
				}
				else if (!CheckFact(FACT_134, FATHER))
				{
					TriggerNPCRecord( FATHER, 15 );
				}
				else
				{
					TriggerNPCRecord( FATHER, 26 );
				}
				break;

			case NPC_ACTION_ENTER_COMBAT:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if (pSoldier )
				{
					if ( pSoldier->ubCivilianGroup != NON_CIV_GROUP )
					{
						if ( gTacticalStatus.fCivGroupHostile[ pSoldier->ubCivilianGroup ] == CIV_GROUP_NEUTRAL )
						{
							CivilianGroupMemberChangesSides( pSoldier );
						}
					}
					else
					{
						// make hostile
						MakeCivHostile( pSoldier, 2 );
					}
					DeleteTalkingMenu();
					if ( ! ( gTacticalStatus.uiFlags & INCOMBAT ) )
					{
						EnterCombatMode( pSoldier->bTeam );
					}
				}
				break;
			}

			case NPC_ACTION_DECIDE_ACTIVE_TERRORISTS:
				// only (now) add all terrorist files to laptop
				AddFilesAboutTerrorists();

				// Carmen has received 0 terrorist heads recently
				GetProfile(CARMEN).bNPCData2 = 0;
				break;

			case NPC_ACTION_CHECK_LAST_TERRORIST_HEAD:
			{
				MERCPROFILESTRUCT& carmen = GetProfile(CARMEN);
				--carmen.bNPCData;  // decrement head count
				++carmen.bNPCData2; // increment number of heads on hand

				if (gWorldSectorX == 13 && gWorldSectorY == MAP_ROW_C && gbWorldSectorZ == 0)
				{
					TriggerNPCRecord(CARMEN, 20);
				}
				else
				{
					TriggerNPCRecord(CARMEN, 21);
				}
				// CJC Nov 28 2002 - fixed history record which didn't have location specified
				AddHistoryToPlayersLog( HISTORY_GAVE_CARMEN_HEAD, 0, GetWorldTotalMin(), gWorldSectorX, gWorldSectorY );
				break;
			}

			case NPC_ACTION_CARMEN_LEAVES_FOR_GOOD:
				gMercProfiles[ ubTargetNPC ].ubMiscFlags2 |= PROFILE_MISC_FLAG2_LEFT_COUNTRY;
				// fall through!
			case NPC_ACTION_CARMEN_LEAVES_FOR_C13:
				// set "don't add to sector" cause he'll only appear after an event...
				gMercProfiles[ ubTargetNPC ].ubMiscFlags2 |= PROFILE_MISC_FLAG2_DONT_ADD_TO_SECTOR;

				SetCustomizableTimerCallbackAndDelay(params->getAmount(10000), CarmenLeavesSectorCallback, TRUE );
				break;

			case NPC_ACTION_CARMEN_LEAVES_ON_NEXT_SECTOR_LOAD:
				if (gMercProfiles[ CARMEN ].bNPCData == 0)
				{
					SetFactTrue( FACT_ALL_TERRORISTS_KILLED );
					// the next time we change sectors, quest 2 will be set to done

					IncrementTownLoyaltyEverywhere( LOYALTY_BONUS_TERRORISTS_DEALT_WITH );
				}
				// anyhow Carmen has given the player money so reset his balance to 0.
				gMercProfiles[ CARMEN ].uiMoney = 0;
				break;

			case NPC_ACTION_END_COMBAT:
				ExitCombatMode();
				break;

			case NPC_ACTION_BECOME_FRIENDLY_END_COMBAT:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if (pSoldier )
				{
					DeleteTalkingMenu();

					SetSoldierNeutral( pSoldier );
					RecalculateOppCntsDueToBecomingNeutral( pSoldier );
					if ( gTacticalStatus.bNumFoughtInBattle[ CIV_TEAM ] == 0 )
					{
						gTacticalStatus.fEnemyInSector = FALSE;
					}
					if ( !gTacticalStatus.fEnemyInSector )
					{
						ExitCombatMode();
					}

					CancelAIAction(pSoldier);
					// make stand up if not standing already
					if ( ubTargetNPC == SLAY && pSoldier->ubBodyType == CRIPPLECIV )
					{
						HandleNPCDoAction( SLAY, NPC_ACTION_GET_OUT_OF_WHEELCHAIR, ubQuoteNum );
					}
					else if (!PTR_STANDING)
					{
						pSoldier->bNextAction = AI_ACTION_CHANGE_STANCE;
						pSoldier->usNextActionData = ANIM_STAND;
					}
				}
				break;
			}

			case NPC_ACTION_TERRORIST_REVEALS_SELF:
				// WAS swap names in profile
				// NOW overwrite name with true name in profile
				// copy new nickname into soldier structure
				{
					gMercProfiles[ubTargetNPC].zNickname = gMercProfiles[ubTargetNPC].zName;
					SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
					if ( pSoldier )
					{
						pSoldier->name = gMercProfiles[ubTargetNPC].zNickname;
					}
				}
				break;

			case NPC_ACTION_END_MEANWHILE:

				// End meanwhile....
				DeleteTalkingMenu();
				EndMeanwhile( );
				break;

			case NPC_ACTION_START_BLOODCAT_QUEST:
				StartQuest( QUEST_BLOODCATS, gWorldSectorX, gWorldSectorY );
				break;

			case NPC_ACTION_START_MINE:
			{
				const UINT8 ubMineIndex = GetHeadMinersMineIndex(ubTargetNPC);

				// record the fact that player has spoken with this head miner so he can now get production from that mine
				PlayerSpokeToHeadMiner( ubTargetNPC );

				if ( IsMineShutDown( ubMineIndex ) )
				{
					RestartMineProduction( ubMineIndex );
				}
				break;
			}

			case NPC_ACTION_STOP_MINE:
			{
				const UINT8 ubMineIndex = GetHeadMinersMineIndex(ubTargetNPC);

				if ( !IsMineShutDown( ubMineIndex ) )
				{
					ShutOffMineProduction( ubMineIndex );
				}
				break;
			}

			case NPC_ACTION_RESET_MINE_CAPTURED:
				ResetQueenRetookMine( ubTargetNPC );
				break;

			case NPC_ACTION_SET_OSWALD_RECORD_13_USED:
			case NPC_ACTION_SET_CALVIN_RECORD_13_USED:
			case NPC_ACTION_SET_CARL_RECORD_13_USED:
			case NPC_ACTION_SET_FRED_RECORD_13_USED:
			case NPC_ACTION_SET_MATT_RECORD_13_USED:
				SetQuoteRecordAsUsed( ubTargetNPC, 13 );
				break;

			case NPC_ACTION_TRIGGER_MATT:
				if ( CheckFact( FACT_DYNAMO_IN_J9, MATT ) )
				{
					TriggerNPCRecord( MATT, 15 );
				}
				else if ( CheckFact( FACT_DYNAMO_ALIVE, MATT ) )
				{
					TriggerNPCRecord( MATT, 16 );
				}
				break;

			case NPC_ACTION_MADLAB_GIVEN_GUN:
				SetFactFalse( FACT_MADLAB_EXPECTING_FIREARM );
				if ( CheckFact( FACT_MADLAB_EXPECTING_VIDEO_CAMERA, 0 ) )
				{
					TriggerNPCRecord( ubTargetNPC, 14 );
				}
				else
				{
					TriggerNPCRecord( ubTargetNPC, 18 );
				}
				break;

			case NPC_ACTION_MADLAB_GIVEN_CAMERA:
				SetFactFalse( FACT_MADLAB_EXPECTING_VIDEO_CAMERA );
				if ( CheckFact( FACT_MADLAB_EXPECTING_FIREARM, 0 ) )
				{
					TriggerNPCRecord( ubTargetNPC, 17 );
				}
				else
				{
					TriggerNPCRecord( ubTargetNPC, 18 );
				}
				break;

			case NPC_ACTION_MADLAB_ATTACHES_GOOD_CAMERA:
			{
				SetFactFalse( FACT_MADLAB_HAS_GOOD_CAMERA );
				SOLDIERTYPE* const pSoldier  = FindSoldierByProfileID(MADLAB);
				SOLDIERTYPE* const pSoldier2 = FindSoldierByProfileID(ROBOT);
				if ( pSoldier && pSoldier2 )
				{
					// Give weapon to robot
					INT8 bSlot;

					bSlot = FindObjClass( pSoldier, IC_GUN );
					if ( bSlot != NO_SLOT )
					{
						AutoPlaceObject( pSoldier2, &( pSoldier->inv[ bSlot ] ) , FALSE );
					}
				}
				// Allow robot to be controlled by remote!
				RecruitEPC( ROBOT );
				break;
			}

			case NPC_ACTION_READY_ROBOT:
				AddFutureDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, 420, NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS + NPC_ACTION_READY_ROBOT, 1 );
				break;

			case NPC_ACTION_HANDLE_END_OF_FIGHT:
				ExitBoxing();
				DeleteTalkingMenu();
				/*
				switch( gTacticalStatus.bBoxingState )
				{
					case WON_ROUND:
						TriggerNPCRecord( DARREN, 23 );
						break;
					case LOST_ROUND:
						TriggerNPCRecord( DARREN, 24 );
						break;
					default:
						break;
				}*/
				break;

			case NPC_ACTION_DARREN_PAYS_PLAYER:
			{
				// should change to split up cash
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if ( pSoldier )
				{
					INT16 sNearestPC;
					INT8  bMoneySlot;
					INT8  bEmptySlot;

					sNearestPC = ClosestPC( pSoldier, NULL );
					SOLDIERTYPE* const pSoldier2 = (sNearestPC != NOWHERE ? WhoIsThere2(sNearestPC, 0) : NULL);

					if (pSoldier2)
					{
						bMoneySlot = FindObjClass( pSoldier, IC_MONEY );
						bEmptySlot = FindObj( pSoldier, NOTHING );

						// have to separate out money from Darren's stash equal to the amount
						// of the bet times 2 (returning the player's money, after all!)
						if (bMoneySlot != NO_SLOT && bEmptySlot != NO_SLOT)
						{
							CreateMoney( gMercProfiles[ ubTargetNPC ].iBalance * 2, &(pSoldier->inv[ bEmptySlot ] ) );
							pSoldier->inv[ bMoneySlot ].uiMoneyAmount -= gMercProfiles[ ubTargetNPC ].iBalance * 2;
							if (bMoneySlot < bEmptySlot)
							{
								// move main stash to later in inventory!
								SwapObjs( &(pSoldier->inv[ bEmptySlot ] ), &(pSoldier->inv[ bMoneySlot ] ) );
								SoldierGiveItem( pSoldier, pSoldier2, &(pSoldier->inv[ bMoneySlot ] ), bMoneySlot );
							}
							else
							{
								SoldierGiveItem( pSoldier, pSoldier2, &(pSoldier->inv[ bEmptySlot ] ), bEmptySlot );
							}
						}
					}
				}
				break;
			}

			case NPC_ACTION_TRIGGER_SPIKE_OR_DARREN:
			{
				const SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(KINGPIN);
				if (pSoldier)
				{
					UINT8 const room = GetRoom(pSoldier->sGridNo);
					if (1 <= room && room <= 3)
					{	// Kingpin is in the club
						TriggerNPCRecord( DARREN, 31 );
						break;
					}
				}
				// Kingpin not in club
				TriggerNPCRecord( SPIKE, 11 );
				break;
			}

			case NPC_ACTION_OPEN_CLOSEST_CABINET:
			{
				// Delete menu
				DeleteTalkingMenu( );

				// Get pointer to NPC
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if ( !pSoldier )
				{
					return;
				}

				// FInd, open Closest non-door
				NPCOpenThing( pSoldier, FALSE );
				break;
			}

			case NPC_ACTION_GET_ITEMS_FROM_CLOSEST_CABINET:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if (pSoldier)
				{
					// shouldn't have any current action but make sure everything
					// is clear... and set pending action so the guy won't move
					// until the pickup is completed
					CancelAIAction(pSoldier);
					pSoldier->bAction = AI_ACTION_PENDING_ACTION;

					// make sure the pickup starts dammit!
					pSoldier->fInNonintAnim = FALSE;
					pSoldier->fRTInNonintAnim = FALSE;

					INT16 sGridNo;
					if ( pSoldier->ubProfile == ARMAND )
					{
						sGridNo = 6968;
					}
					else
					{
						sGridNo = FindNearestOpenableNonDoor( pSoldier->sGridNo );
					}

					SoldierPickupItem( pSoldier, ITEM_PICKUP_ACTION_ALL, sGridNo, ITEM_IGNORE_Z_LEVEL );
				}
				break;
			}

			case NPC_ACTION_SHOW_TIXA:
				SetTixaAsFound();
				AddHistoryToPlayersLog( HISTORY_DISCOVERED_TIXA, 0, GetWorldTotalMin(), gWorldSectorX, gWorldSectorY );
				break;
			case NPC_ACTION_SHOW_ORTA:
				SetOrtaAsFound();
				AddHistoryToPlayersLog( HISTORY_DISCOVERED_ORTA, 0, GetWorldTotalMin(), gWorldSectorX, gWorldSectorY );
				break;

			case NPC_ACTION_SLAP:
			{
				// OK, LET'S FIND THE QUEEN AND MAKE HER DO SLAP ANIMATION
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(QUEEN);
				if (pSoldier)
				{
					EVENT_InitNewSoldierAnim( pSoldier, QUEEN_SLAP, 0 , FALSE );
				}
				break;
			}

			{
				INT16    sGridNo;
				Approach approach;

			case NPC_ACTION_PUNCH_PC_SLOT_0:
				sGridNo  = gsInterrogationGridNo[0];
				approach = APPROACH_DONE_PUNCH_0;
				goto action_punch_pc;

			case NPC_ACTION_PUNCH_PC_SLOT_1:
				sGridNo  = gsInterrogationGridNo[1];
				approach = APPROACH_DONE_PUNCH_1;
				goto action_punch_pc;

			case NPC_ACTION_PUNCH_PC_SLOT_2:
				sGridNo  = gsInterrogationGridNo[2];
				approach = APPROACH_DONE_PUNCH_2;
				goto action_punch_pc;

action_punch_pc:
				DeleteTalkingMenu( );
				// OK, LET'S FIND THE QUEEN AND MAKE HER DO SLAP ANIMATION
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if (pSoldier)
				{
					// Target a different merc....
					const SOLDIERTYPE* const pTarget = WhoIsThere2(sGridNo, 0);

					// Use a different approach....
					pSoldier->uiPendingActionData4 = approach;

					if ( pTarget && pTarget->bActive && pTarget->bInSector && pTarget->bLife != 0 )
					{
						pSoldier->bNextAction = AI_ACTION_KNIFE_MOVE;
						pSoldier->usNextActionData = pTarget->sGridNo;
						pSoldier->fAIFlags |= AI_HANDLE_EVERY_FRAME;

						// UNless he's has a pending action, delete what he was doing!
						// Cancel anything he was doing
						if ( pSoldier->bAction != AI_ACTION_PENDING_ACTION )
						{
							CancelAIAction(pSoldier);
						}

						//HandleItem( pSoldier, pTarget->sGridNo, 0, NOTHING, FALSE );

						pSoldier->uiStatusFlags |= SOLDIER_NPC_DOING_PUNCH;
					}
					else
					{
						TriggerNPCWithGivenApproach(pSoldier->ubProfile, approach);
					}
				}
				break;
			}

			case NPC_ACTION_SHOOT_ELLIOT:
			{
				DeleteTalkingMenu( );

				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if (pSoldier)
				{
					SOLDIERTYPE* const pTarget = FindSoldierByProfileID(ELLIOT);
					if ( pTarget )
					{
						// Set special flag....
						pTarget->uiStatusFlags |= SOLDIER_NPC_SHOOTING;
						pSoldier->uiStatusFlags |= SOLDIER_NPC_SHOOTING;

						pSoldier->bAimShotLocation = AIM_SHOT_HEAD;

						// Add gun to inventory.....
						CreateItem( (UINT16) (DESERTEAGLE), 100, &( pSoldier->inv[ HANDPOS ] ) );

						// Make shoot
						pSoldier->bNextAction = AI_ACTION_FIRE_GUN;
						pSoldier->usNextActionData = pTarget->sGridNo;
						pSoldier->fAIFlags |= AI_HANDLE_EVERY_FRAME;

						// UNless he's has a pending action, delete what he was doing!
						// Cancel anything he was doing
						if ( pSoldier->bAction != AI_ACTION_PENDING_ACTION )
						{
							CancelAIAction(pSoldier);
						}

						// change elliot portrait...
						gMercProfiles[ ELLIOT ].bNPCData = 17;
					}
				}
				break;
			}

				//Emmons: is this line of code part of something missing
				//or no longer necessary?  CJC
				//if ( pSoldier->uiStatusFlags & SOLDIER_NPC_SHOOTING )

			case NPC_ACTION_PUNCH_FIRST_LIVING_PC:
			{
				// Punch first living pc....
				// OK, LET'S FIND THE QUEEN AND MAKE HER DO SLAP ANIMATION
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if (pSoldier)
				{
					INT32 cnt;
					BOOLEAN fGoodTarget = FALSE;

					for ( cnt = 0; cnt < 3; cnt++ )
					{
						const SOLDIERTYPE* const pTarget = WhoIsThere2(gsInterrogationGridNo[cnt], 0);
						if (pTarget == NULL) continue;

						pSoldier->uiPendingActionData4 = APPROACH_DONE_PUNCH_1;

						if (pTarget->bActive && pTarget->bInSector)
						{
							// If we are elliot, we can't do unconocious guys....
							if (pSoldier->ubProfile == ELLIOT)
							{
								if (pTarget->bLife >= OKLIFE) fGoodTarget = TRUE;
							}
							else
							{
								if (pTarget->bLife != 0) fGoodTarget = TRUE;
							}
						}

						if ( fGoodTarget )
						{
							pSoldier->bNextAction = AI_ACTION_KNIFE_MOVE;
							pSoldier->usNextActionData = pTarget->sGridNo;
							pSoldier->fAIFlags |= AI_HANDLE_EVERY_FRAME;

							// UNless he's has a pending action, delete what he was doing!
							// Cancel anything he was doing
							if ( pSoldier->bAction != AI_ACTION_PENDING_ACTION )
							{
								CancelAIAction(pSoldier);
							}

							pSoldier->uiStatusFlags |= SOLDIER_NPC_DOING_PUNCH;
							break;
						}
					}

					if ( cnt == 3 )
					{
						// If here, nobody was found...
						TriggerNPCWithGivenApproach(pSoldier->ubProfile, APPROACH_DONE_PUNCH_1);
					}
				}
				break;
			}

			case NPC_ACTION_FRUSTRATED_SLAP:
			{
				// OK, LET'S FIND THE QUEEN AND MAKE HER DO SLAP ANIMATION
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if (pSoldier)
				{
					EVENT_InitNewSoldierAnim( pSoldier, QUEEN_FRUSTRATED_SLAP, 0 , FALSE );
				}
				break;
			}

			case NPC_ACTION_START_TIMER_ON_KEITH_GOING_OUT_OF_BUSINESS:
				// start timer to place keith out of business
				AddStrategicEvent( EVENT_KEITH_GOING_OUT_OF_BUSINESS, GetWorldTotalMin() + ( 6 * 24 * 60 ), 0 );
				break;

			case NPC_ACTION_KEITH_GOING_BACK_IN_BUSINESS:
				// keith is back in business
				SetFactFalse( FACT_KEITH_OUT_OF_BUSINESS );
				break;
			case NPC_ACTION_TRIGGER_QUEEN_BY_CITIES_CONTROLLED:
				switch ( GetNumberOfWholeTownsUnderControlButExcludeCity( OMERTA ) )
				{
					case 0:
					case 1:
						TriggerNPCRecord( QUEEN, 11 );
						break;
					case 2:
						TriggerNPCRecord( QUEEN, 7 );
						break;
					case 3:
						TriggerNPCRecord( QUEEN, 8 );
						break;
					case 4:
						TriggerNPCRecord( QUEEN, 9 );
						break;
					case 5:
						TriggerNPCRecord( QUEEN, 10 );
						break;
					default:
						TriggerNPCRecord( QUEEN, 11 );
						break;
				}
				break;

			case NPC_ACTION_SEND_SOLDIERS_TO_DRASSEN:
			case NPC_ACTION_SEND_SOLDIERS_TO_BATTLE_LOCATION:
			case NPC_ACTION_SEND_SOLDIERS_TO_OMERTA:
			case NPC_ACTION_ADD_MORE_ELITES:
			case NPC_ACTION_GIVE_KNOWLEDGE_OF_ALL_MERCS:
				break;

			case NPC_ACTION_TRIGGER_QUEEN_BY_SAM_SITES_CONTROLLED:
				switch ( GetNumberOfSAMSitesUnderPlayerControl() )
				{
					case 0:
					case 1:
						TriggerNPCRecord( QUEEN, 7 );
						break;
					case 2:
						TriggerNPCRecord( QUEEN, 8 );
						break;
					case 3:
						TriggerNPCRecord( QUEEN, 9 );
						break;
					default:
						break;
				}
				break;
			case NPC_ACTION_TRIGGER_ELLIOT_BY_BATTLE_RESULT:
				if ( gTacticalStatus.fLastBattleWon )
				{
					TriggerNPCRecord( ELLIOT, 6 );
				}
				else
				{
					TriggerNPCRecord( ELLIOT, 8 );
				}
				break;

			case NPC_ACTION_TRIGGER_ELLIOT_BY_SAM_DISABLED:
				if ( IsThereAFunctionalSAMSiteInSector( gTacticalStatus.ubLastBattleSectorX, gTacticalStatus.ubLastBattleSectorY, 0 ) )
				{
					TriggerNPCRecord( QUEEN, 6 );
				}
				else
				{
					TriggerNPCRecord( ELLIOT, 2 );
				}
				break;

			case NPC_ACTION_TRIGGER_VINCE_BY_LOYALTY:
				if( CheckFact( FACT_NPC_OWED_MONEY, ubTargetNPC ) && ( ubTargetNPC == VINCE ) )
				{
					// check if we owe vince cash
					TriggerNPCRecord( ubTargetNPC, 21 );
				}
				else if (!CheckFact(FACT_VINCE_EXPLAINED_HAS_TO_CHARGE, ubTargetNPC))
				{
					TriggerNPCRecord( ubTargetNPC, 15 );
				}
				else
				{
					if ( CheckFact( FACT_LOYALTY_LOW, ubTargetNPC ) )
					{
						gbHospitalPriceModifier = HOSPITAL_NORMAL;
						TriggerNPCRecord( ubTargetNPC, 16 );
					}
					else if ( CheckFact( FACT_LOYALTY_OKAY, ubTargetNPC ) )
					{
						gbHospitalPriceModifier = HOSPITAL_BREAK;
						TriggerNPCRecord( ubTargetNPC, 17 );
					}
					else
					{
						const INT32 iRandom = PreRandom(100);
						if( ubTargetNPC == VINCE )
						{
							if (gbHospitalPriceModifier == HOSPITAL_RANDOM_FREEBIE ||
								gbHospitalPriceModifier == HOSPITAL_FREEBIE        ||
								(!CheckFact(FACT_HOSPITAL_FREEBIE_DECISION_MADE, 0) && iRandom < CHANCE_FOR_DOCTOR_TO_SAY_RANDOM_QUOTE))
							{
								SetFactTrue( FACT_HOSPITAL_FREEBIE_DECISION_MADE );
								if ( CheckFact( FACT_LOYALTY_HIGH, ubTargetNPC ) && CheckFact( FACT_24_HOURS_SINCE_DOCTOR_TALKED_TO, ubTargetNPC ) )
								{
									// loyalty high... freebie
									gbHospitalPriceModifier = HOSPITAL_FREEBIE;
									TriggerNPCRecord( ubTargetNPC, 19 );
								}
								else
								{
									// random vince quote
									gbHospitalPriceModifier = HOSPITAL_RANDOM_FREEBIE;
									TriggerNPCRecord( ubTargetNPC, 20 );
								}
							}
							else // loyalty good
							{
								gbHospitalPriceModifier = HOSPITAL_COST;
								TriggerNPCRecord( ubTargetNPC, 18 );
							}
						}
						else
						{
							// it's steve willis

							// in discount mode?
							if ( CheckFact( FACT_WILLIS_GIVES_DISCOUNT, ubTargetNPC ) )
							{
								// yep inform player
								gbHospitalPriceModifier = HOSPITAL_COST;
								TriggerNPCRecord( ubTargetNPC, 21 );
							}
							else if ( CheckFact( FACT_WILLIS_HEARD_ABOUT_JOEY_RESCUE, ubTargetNPC ) )
							{
								// joey rescued.... note this will set FACT_WILLIS_GIVES_DISCOUNT so this will only ever be handled once
								gbHospitalPriceModifier = HOSPITAL_FREEBIE;
								TriggerNPCRecord( ubTargetNPC, 20 );
							}
							else if (CheckFact(FACT_LOYALTY_HIGH, ubTargetNPC)               &&
								CheckFact(FACT_24_HOURS_SINCE_DOCTOR_TALKED_TO, ubTargetNPC) && (
								gbHospitalPriceModifier == HOSPITAL_FREEBIE ||
								(!CheckFact(FACT_HOSPITAL_FREEBIE_DECISION_MADE, 0) && iRandom < CHANCE_FOR_DOCTOR_TO_SAY_RANDOM_QUOTE)))
							{
								// loyalty high... freebie
								gbHospitalPriceModifier = HOSPITAL_FREEBIE;
								TriggerNPCRecord( ubTargetNPC, 19 );
							}
							else // loyalty good
							{
								gbHospitalPriceModifier = HOSPITAL_COST;
								TriggerNPCRecord( ubTargetNPC, 18 );
							}
						}
					}
				}
				break;
			case NPC_ACTION_CHECK_DOCTORING_MONEY_GIVEN:
				break;

			case NPC_ACTION_DOCTOR_ESCORT_PATIENTS:
			{
				// find anyone in sector who is wounded; set to hospital patient
				const SOLDIERTYPE* const pSoldier2 = FindSoldierByProfileID(ubTargetNPC);
				if ( pSoldier2 )
				{
					//HOSPITAL_PATIENT_DISTANCE
					FOR_EACH_IN_TEAM(i, OUR_TEAM)
					{
						SOLDIERTYPE& s = *i;
						// Are we in this sector, On the current squad?
						if (s.bInSector &&
							0 < s.bLife && s.bLife < s.bLifeMax &&
							s.bAssignment != ASSIGNMENT_HOSPITAL &&
							PythSpacesAway(s.sGridNo, pSoldier2->sGridNo) < HOSPITAL_PATIENT_DISTANCE)
						{
							SetSoldierAssignmentHospital(s);
							TriggerNPCRecord(s.ubProfile, 2);
							s.bHospitalPriceModifier = gbHospitalPriceModifier;
							// make sure this person doesn't have an absolute dest any more
							s.sAbsoluteFinalDestination = NOWHERE;
						}
					}

					SetFactFalse( FACT_HOSPITAL_FREEBIE_DECISION_MADE );
					gbHospitalPriceModifier = HOSPITAL_NORMAL;
				}
				break;
			}

			case NPC_ACTION_START_DOCTORING:
				{

					// reset fact he is expecting money fromt he player
					SetFactFalse( FACT_VINCE_EXPECTING_MONEY );

					// check fact
					if( CheckFact( FACT_PLAYER_STOLE_MEDICAL_SUPPLIES, ubTargetNPC ) )
					{
						TriggerNPCRecord( ubTargetNPC, 28 );
					}
					else
					{
						TriggerNPCRecord( ubTargetNPC, 35 );
					}
				}
				break;

			case NPC_ACTION_VINCE_UNRECRUITABLE:
				SetFactFalse( FACT_VINCE_RECRUITABLE );
				break;
			case NPC_ACTION_ELLIOT_DECIDE_WHICH_QUOTE_FOR_PLAYER_ATTACK:
				if( DidFirstBattleTakePlaceInThisTown( DRASSEN ) )
				{
					TriggerNPCRecord( ELLIOT, 2 );
				}
				else if( DidFirstBattleTakePlaceInThisTown( CAMBRIA ) )
				{
					TriggerNPCRecord( ELLIOT, 3 );
				}
				else
				{
					TriggerNPCRecord( ELLIOT, 1 );
				}
				break;
			case NPC_ACTION_QUEEN_DECIDE_WHICH_QUOTE_FOR_PLAYER_ATTACK:
				if( DidFirstBattleTakePlaceInThisTown( DRASSEN ) )
				{
					TriggerNPCRecord( QUEEN, 8 );
				}
				else
				{
					TriggerNPCRecord( QUEEN, 9 );
				}

				break;
			case NPC_ACTION_KROTT_ALIVE_LOYALTY_BOOST:
				if ( gMercProfiles[ SERGEANT ].bMercStatus != MERC_IS_DEAD )
				{
					// give loyalty bonus to Alma
					IncrementTownLoyalty( ALMA, LOYALTY_BONUS_FOR_SERGEANT_KROTT );
				}
				break;

			case NPC_ACTION_MAKE_NPC_FIRST_BARTENDER:
				gMercProfiles[ ubTargetNPC ].bNPCData = 1;
				break;
			case NPC_ACTION_MAKE_NPC_SECOND_BARTENDER:
				gMercProfiles[ ubTargetNPC ].bNPCData = 2;
				break;
			case NPC_ACTION_MAKE_NPC_THIRD_BARTENDER:
				gMercProfiles[ ubTargetNPC ].bNPCData = 3;
				break;
			case NPC_ACTION_MAKE_NPC_FOURTH_BARTENDER:
				gMercProfiles[ ubTargetNPC ].bNPCData = 4;
				break;

			case NPC_ACTION_END_DEMO:
				DeleteTalkingMenu();
				// hack!!
				EndGameMessageBoxCallBack( MSG_BOX_RETURN_YES );
				break;

			case NPC_ACTION_SEND_ENRICO_MIGUEL_EMAIL:
				AddFutureDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, 390 + Random( 60 ), NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS + NPC_ACTION_SEND_ENRICO_MIGUEL_EMAIL, 1 );
				break;

			case NPC_ACTION_PLAYER_SAYS_NICE_LATER:
				SetFactTrue( FACT_NEED_TO_SAY_SOMETHING );
				gubNiceNPCProfile = gpDestSoldier->ubProfile;
				break;

			case NPC_ACTION_PLAYER_SAYS_NASTY_LATER:
				SetFactTrue( FACT_NEED_TO_SAY_SOMETHING );
				gubNastyNPCProfile = gpDestSoldier->ubProfile;
				break;

			case NPC_ACTION_CHOOSE_DOCTOR:
			{
				// find doctors available and trigger record 12 or 13
				SOLDIERTYPE* pSoldier = FindSoldierByProfileID(STEVE); // Steve Willis, 80
				if (pSoldier)
				{
					if (!pSoldier->bInSector || pSoldier->bTeam != CIV_TEAM || !pSoldier->bNeutral || pSoldier->bLife < OKLIFE)
					{
						pSoldier = NULL;
					}
				}

				SOLDIERTYPE* pSoldier2 = FindSoldierByProfileID(VINCE); // Vince, 69
				if (pSoldier2)
				{
					if (!pSoldier2->bInSector || pSoldier2->bTeam != CIV_TEAM || !pSoldier2->bNeutral || pSoldier2->bLife < OKLIFE)
					{
						pSoldier2 = NULL;
					}
				}

				if ( pSoldier == NULL && pSoldier2 == NULL )
				{
					// uh oh
					break;
				}

				if( ( pSoldier ) && ( pSoldier2 ) )
				{
					if( pSoldier->sGridNo == params->getGridNo(10343) )
					{
						pSoldier2 = NULL;
					}
					else if( pSoldier2->sGridNo == params->getGridNo(10343) )
					{
						pSoldier = NULL;
					}
				}

				if ( pSoldier && pSoldier2 )
				{
					// both doctors available... turn one off randomly
					if ( Random( 2 ) )
					{
						pSoldier = NULL;
					}
					else
					{
						pSoldier2 = NULL;
					}
				}

				// only one pointer left...

				if ( pSoldier )
				{
					TriggerNPCRecord( ubTargetNPC, 12 );
				}
				else
				{
					TriggerNPCRecord( ubTargetNPC, 13 );
				}
				break;
			}

			case NPC_ACTION_INVOKE_CONVERSATION_MODE:
				if ( !gfInTalkPanel )
				{
					INT16		sNearestPC;

					SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
					if ( pSoldier )
					{
						SOLDIERTYPE* pSoldier2 = NULL;

						sNearestPC = ClosestPC( pSoldier, NULL );
						if ( sNearestPC != NOWHERE )
						{
							pSoldier2 = WhoIsThere2(sNearestPC, 0);
						}

						if ( pSoldier2 )
						{
							InitiateConversation(pSoldier, pSoldier2, NPC_INITIAL_QUOTE);
						}
					}
				}
				break;

			case NPC_ACTION_KINGPIN_TRIGGER_25_OR_14:
				if ( CheckFact( FACT_KINGPIN_INTRODUCED_SELF, ubTargetNPC ) == TRUE)
				{
					TriggerNPCRecord( ubTargetNPC, 25 );
				}
				else
				{
					TriggerNPCRecord( ubTargetNPC, 14 );
				}
				break;

			case NPC_ACTION_INITIATE_SHOPKEEPER_INTERFACE:
				// check if this is a shopkeeper who has been shutdown
				if (!HandleShopKeepHasBeenShutDown(gTalkPanel.ubCharNum))
				{
					DeleteTalkingMenu( );

					EnterShopKeeperInterfaceScreen( gTalkPanel.ubCharNum );
				}
				break;

			case NPC_ACTION_DRINK_WINE:
				gMercProfiles[ ubTargetNPC ].bNPCData += 2;
				break;

			case NPC_ACTION_DRINK_BOOZE:
				gMercProfiles[ ubTargetNPC ].bNPCData += 4;
				break;

			case NPC_ACTION_TRIGGER_ANGEL_22_OR_24:
				if (CheckFact(FACT_ANGEL_LEFT_DEED, ubTargetNPC))
				{
					TriggerNPCRecord( ubTargetNPC, 22 );
				}
				else
				{
					TriggerNPCRecord( ubTargetNPC, 24 );
				}
				break;

			case NPC_ACTION_GRANT_EXPERIENCE_1:
				AwardExperienceBonusToActiveSquad( EXP_BONUS_MINIMUM );
				break;
			case NPC_ACTION_GRANT_EXPERIENCE_2:
				AwardExperienceBonusToActiveSquad( EXP_BONUS_SMALL );
				break;
			case NPC_ACTION_GRANT_EXPERIENCE_3:
				AwardExperienceBonusToActiveSquad( EXP_BONUS_AVERAGE );
				break;
			case NPC_ACTION_GRANT_EXPERIENCE_4:
				AwardExperienceBonusToActiveSquad( EXP_BONUS_LARGE );
				break;
			case NPC_ACTION_GRANT_EXPERIENCE_5:
				AwardExperienceBonusToActiveSquad( EXP_BONUS_MAXIMUM );
				break;
			case NPC_ACTION_TRIGGER_YANNI:
				if ( CheckFact( FACT_CHALICE_STOLEN, 0 ) == TRUE )
				{
					TriggerNPCRecord( YANNI, 8 );
				}
				else
				{
					TriggerNPCRecord( YANNI, 7 );
				}
				break;
			case NPC_ACTION_TRIGGER_MARY_OR_JOHN_RECORD_9:
			{
				const SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(MARY);
				if ( pSoldier && pSoldier->bLife >= OKLIFE )
				{
					TriggerNPCRecord( MARY, 9 );
				}
				else
				{
					TriggerNPCRecord( JOHN, 9 );
				}
				break;
			}

			case NPC_ACTION_TRIGGER_MARY_OR_JOHN_RECORD_10:
			{
				const SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(MARY);
				if ( pSoldier && pSoldier->bLife >= OKLIFE )
				{
					TriggerNPCRecord( MARY, 10 );
				}
				else
				{
					TriggerNPCRecord( JOHN, 10 );
				}
				break;
			}

			case NPC_ACTION_GET_OUT_OF_WHEELCHAIR:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if ( pSoldier && pSoldier->ubBodyType == CRIPPLECIV )
				{
					DeleteTalkingMenu();
					EVENT_InitNewSoldierAnim( pSoldier, CRIPPLE_KICKOUT, 0, TRUE );
				}
				break;
			}

			case NPC_ACTION_GET_OUT_OF_WHEELCHAIR_AND_BECOME_HOSTILE:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if ( pSoldier && pSoldier->ubBodyType == CRIPPLECIV )
				{
					DeleteTalkingMenu();
					EVENT_InitNewSoldierAnim( pSoldier, CRIPPLE_KICKOUT, 0, TRUE );
				}
				TriggerFriendWithHostileQuote( ubTargetNPC );
				break;
			}

			case NPC_ACTION_ADD_JOHNS_GUN_SHIPMENT:
				AddJohnsGunShipment();
				// also close panel
				DeleteTalkingMenu();
				break;

			case NPC_ACTION_SET_FACT_105_FALSE:
				SetFactFalse( FACT_FRANK_HAS_BEEN_BRIBED );
				break;

			case NPC_ACTION_CANCEL_WAYPOINTS:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if ( pSoldier )
				{
					pSoldier->bOrders = ONGUARD;
				}
				break;
			}

			case NPC_ACTION_MAKE_RAT_DISAPPEAR:
				// hack a flag to determine when RAT should leave
				gMercProfiles[ RAT ].bNPCData = 1;
				break;

			case NPC_ACTION_TRIGGER_MICKY_BY_SCI_FI:
				if ( gGameOptions.fSciFi )
				{
					TriggerNPCRecord( MICKY, 7 );
				}
				else
				{
					TriggerNPCRecord( MICKY, 8 );
				}
				break;
			case NPC_ACTION_TRIGGER_KROTT_11_OR_12:
				if ( CheckFact( FACT_KROTT_GOT_ANSWER_NO, SERGEANT ) )
				{
					TriggerNPCRecord( SERGEANT, 11 );
				}
				else
				{
					TriggerNPCRecord( SERGEANT, 12 );
				}
				break;

			case NPC_ACTION_CHANGE_MANNY_POSITION:
			{
				gMercProfiles[ MANNY ].ubMiscFlags3 |= PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE;
				gMercProfiles[ MANNY ].ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
				gMercProfiles[ MANNY ].usStrategicInsertionData = params->getGridNo(19904);
				gMercProfiles[ MANNY ].fUseProfileInsertionInfo = TRUE;
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(MANNY);
				if ( pSoldier )
				{
					pSoldier->bOrders = STATIONARY;
				}
				// close his panel too
				DeleteTalkingMenu();
				break;
			}

			case NPC_ACTION_MAKE_BRENDA_STATIONARY:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(BRENDA);
				if ( pSoldier )
				{
					gMercProfiles[ BRENDA ].ubMiscFlags3 |= PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE;
					gMercProfiles[ BRENDA ].ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
					gMercProfiles[ BRENDA ].usStrategicInsertionData = pSoldier->sGridNo;
					gMercProfiles[ BRENDA ].fUseProfileInsertionInfo = TRUE;
					pSoldier->bOrders = STATIONARY;
				}
				break;
			}

			case NPC_ACTION_MAKE_MIGUEL_STATIONARY:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(MIGUEL);
				if ( pSoldier )
				{
					gMercProfiles[ MIGUEL ].ubMiscFlags3 |= PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE;
					gMercProfiles[ MIGUEL ].ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
					gMercProfiles[ MIGUEL ].usStrategicInsertionData = pSoldier->sGridNo;
					gMercProfiles[ MIGUEL ].fUseProfileInsertionInfo = TRUE;
					pSoldier->bOrders = STATIONARY;
				}
				break;
			}

			case NPC_ACTION_TRIGGER_DARREN_OR_KINGPIN_IMPRESSED:
				if ( BoxerAvailable() )
				{
					TriggerNPCRecord( DARREN, 25 );
				}
				// else check for Kingpin being impressed
				// fallthrough
			case NPC_ACTION_TRIGGER_KINGPIN_IMPRESSED:
				if ( gfLastBoxingMatchWonByPlayer && !BoxerAvailable() )
				{
					TriggerNPCRecord( KINGPIN, 15 );
				}
				break;

			case NPC_ACTION_ADD_RAT:
				// add Rat
				gMercProfiles[ RAT ].sSectorX = 9;
				gMercProfiles[ RAT ].sSectorY = MAP_ROW_G;
				gMercProfiles[ RAT ].bSectorZ = 0;
				break;

			case NPC_ACTION_ENDGAME_STATE_1:

				// What is the status of fact for creatures?
				if ( gubQuest[ 15 ] == QUESTINPROGRESS )
				{
					// This is not the end, 'cause momma creature is still alive
					TriggerNPCRecordImmediately( 136, 8 );
					EndQueenDeathEndgame( );
				}
				else
				{
					// Continue with endgame cimematic..
					DeleteTalkingMenu( );
					EndQueenDeathEndgameBeginEndCimenatic( );
				}
				break;

			case NPC_ACTION_ENDGAME_STATE_2:

				// Just end queen killed dequence.......
				DeleteTalkingMenu( );
				EndQueenDeathEndgame( );
				break;

			case NPC_ACTION_MAKE_ESTONI_A_FUEL_SITE:
				// Jake's script also sets the fact, but we need to be sure it happens before updating availability
				SetFactTrue( FACT_ESTONI_REFUELLING_POSSIBLE );
				UpdateRefuelSiteAvailability();
				break;

			case NPC_ACTION_24_HOURS_SINCE_JOEY_RESCUED:
				AddFutureDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, GetWorldMinutesInDay(), FACT_24_HOURS_SINCE_JOEY_RESCUED, 1 );
				break;

			case NPC_ACTION_24_HOURS_SINCE_DOCTORS_TALKED_TO:
				AddFutureDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, GetWorldMinutesInDay(), FACT_24_HOURS_SINCE_DOCTOR_TALKED_TO, 1 );
				break;

			case NPC_ACTION_REMOVE_MERC_FOR_MARRIAGE:
			{
				SOLDIERTYPE* pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if ( pSoldier )
				{
					pSoldier = ChangeSoldierTeam( pSoldier, CIV_TEAM );
				}
				// remove profile from map
				gMercProfiles[ pSoldier->ubProfile ].sSectorX = 0;
				gMercProfiles[ pSoldier->ubProfile ].sSectorY = 0;
				pSoldier->ubProfile = NO_PROFILE;
				// set to 0 civ group
				pSoldier->ubCivilianGroup = 0;
				break;
			}

			case NPC_ACTION_TIMER_FOR_VEHICLE:
				AddFutureDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, GetWorldMinutesInDay(), NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS + NPC_ACTION_TIMER_FOR_VEHICLE, 1 );
				break;

			case NPC_ACTION_RESET_SHIPMENT_ARRIVAL_STUFF:
				break;

			case NPC_ACTION_TRIGGER_JOE_32_OR_33:
				if ( gbWorldSectorZ > 0 )
				{
					TriggerNPCRecord( JOE, 32 );
				}
				else
				{
					TriggerNPCRecord( JOE, 33 );
				}
				break;
			case NPC_ACTION_REMOVE_NPC:
			{
				SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTargetNPC);
				if ( pSoldier )
				{
					EndAIGuysTurn(*pSoldier);
					RemoveManAsTarget( pSoldier );
					TacticalRemoveSoldier(*pSoldier);
					gMercProfiles[ ubTargetNPC ].sSectorX = 0;
					gMercProfiles[ ubTargetNPC ].sSectorY = 0;
					CheckForEndOfBattle( TRUE );
				}
				break;
			}

			{
				UINT8 code;
				case NPC_ACTION_HISTORY_GOT_ROCKET_RIFLES:
					code = HISTORY_GOT_ROCKET_RIFLES;
					goto add_log;
				case NPC_ACTION_HISTORY_DEIDRANNA_DEAD_BODIES:
					code = HISTORY_DEIDRANNA_DEAD_BODIES;
					goto add_log;
				case NPC_ACTION_HISTORY_BOXING_MATCHES:
					code = HISTORY_BOXING_MATCHES;
					goto add_log;
				case NPC_ACTION_HISTORY_SOMETHING_IN_MINES:
					code = HISTORY_SOMETHING_IN_MINES;
					goto add_log;
				case NPC_ACTION_HISTORY_DEVIN:
					code = HISTORY_DEVIN;
					goto add_log;
				case NPC_ACTION_HISTORY_MIKE:
					code = HISTORY_MIKE;
					goto add_log;
				case NPC_ACTION_HISTORY_TONY:
					code = HISTORY_TONY;
					goto add_log;
				case NPC_ACTION_HISTORY_KROTT:
					code = HISTORY_KROTT;
					goto add_log;
				case NPC_ACTION_HISTORY_KYLE:
					code = HISTORY_KYLE;
					goto add_log;
				case NPC_ACTION_HISTORY_MADLAB:
					code = HISTORY_MADLAB;
					goto add_log;
				case NPC_ACTION_HISTORY_GABBY:
					code = HISTORY_GABBY;
					goto add_log;
				case NPC_ACTION_HISTORY_KEITH_OUT_OF_BUSINESS:
					code = HISTORY_KEITH_OUT_OF_BUSINESS;
					goto add_log;
				case NPC_ACTION_HISTORY_HOWARD_CYANIDE:
					code = HISTORY_HOWARD_CYANIDE;
					goto add_log;
				case NPC_ACTION_HISTORY_KEITH:
					code = HISTORY_KEITH;
					goto add_log;
				case NPC_ACTION_HISTORY_HOWARD:
					code = HISTORY_HOWARD;
					goto add_log;
				case NPC_ACTION_HISTORY_PERKO:
					code = HISTORY_PERKO;
					goto add_log;
				case NPC_ACTION_HISTORY_SAM:
					code = HISTORY_SAM;
					goto add_log;
				case NPC_ACTION_HISTORY_FRANZ:
					code = HISTORY_FRANZ;
					goto add_log;
				case NPC_ACTION_HISTORY_ARNOLD:
					code = HISTORY_ARNOLD;
					goto add_log;
				case NPC_ACTION_HISTORY_FREDO:
					code = HISTORY_FREDO;
					goto add_log;
				case NPC_ACTION_HISTORY_RICHGUY_BALIME:
					code = HISTORY_RICHGUY_BALIME;
					goto add_log;
				case NPC_ACTION_HISTORY_JAKE:
					code = HISTORY_JAKE;
					goto add_log;
				case NPC_ACTION_HISTORY_BUM_KEYCARD:
					code = HISTORY_BUM_KEYCARD;
					goto add_log;
				case NPC_ACTION_HISTORY_WALTER:
					code = HISTORY_WALTER;
					goto add_log;
				case NPC_ACTION_HISTORY_DAVE:
					code = HISTORY_DAVE;
					goto add_log;
				case NPC_ACTION_HISTORY_PABLO:
					code = HISTORY_PABLO;
					goto add_log;
				case NPC_ACTION_HISTORY_KINGPIN_MONEY:
					code = HISTORY_KINGPIN_MONEY;
					goto add_log;
add_log:
					AddHistoryToPlayersLog(code, 0, GetWorldTotalMin(),
								gWorldSectorX, gWorldSectorY);
					break;
			}

			case NPC_ACTION_SEND_TROOPS_TO_SAM:
				break;
			case NPC_ACTION_PUT_PACOS_IN_BASEMENT:
				gMercProfiles[ PACOS ].sSectorX = 10;
				gMercProfiles[ PACOS ].sSectorY = MAP_ROW_A;
				gMercProfiles[ PACOS ].bSectorZ = 0;
				break;
			case NPC_ACTION_HISTORY_ASSASSIN:
				AddHistoryToPlayersLog(HISTORY_ASSASSIN, 0, GetWorldTotalMin(),
							gWorldSectorX, gWorldSectorY);
				break;
			case NPC_ACTION_TRIGGER_HANS_BY_ROOM:
				{
					if ( gpSrcSoldier )
					{
						if (GetRoom(gpSrcSoldier->sGridNo) == 49)
						{
							TriggerNPCRecord( HANS, 18 );
						}
						else
						{
							TriggerNPCRecord( HANS, 14 );
						}

					}
				}
				break;
			case 	NPC_ACTION_TRIGGER_MADLAB_31:
				if ( PlayerTeamFull( ) )
				{
					DeleteTalkingMenu( );
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ CANNOT_RECRUIT_TEAM_FULL ] );
				}
				else
				{
					TriggerNPCRecord( MADLAB, 31 );
				}
				break;
			case NPC_ACTION_TRIGGER_MADLAB_32:
				if ( PlayerTeamFull( ) )
				{
					DeleteTalkingMenu( );
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ CANNOT_RECRUIT_TEAM_FULL ] );
				}
				else
				{
					TriggerNPCRecord( MADLAB, 32 );
				}
				break;
			case NPC_ACTION_TRIGGER_BREWSTER_BY_WARDEN_PROXIMITY:
			{
				const SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(BREWSTER);
				if ( pSoldier )
				{
					const INT16 sGridNo = GetGridNoOfCorpseGivenProfileID(WARDEN);
					if ( sGridNo != NOWHERE && PythSpacesAway( pSoldier->sGridNo, sGridNo ) <= 10 )
					{
						TriggerNPCRecord( BREWSTER, 16 );
					}
					else
					{
						TriggerNPCRecord( BREWSTER, 17 );
					}
				}
				break;
			}

			case NPC_ACTION_FILL_UP_CAR:
			{
				SOLDIERTYPE* pSoldier = FindSoldierByProfileIDOnPlayerTeam(PROF_HUMMER);
				if ( !pSoldier )
				{
					// ice cream truck?
					pSoldier = FindSoldierByProfileIDOnPlayerTeam(PROF_ICECREAM);
				}
				else if ( pSoldier->sBreathRed == 10000 )
				{
					SOLDIERTYPE* const pSoldier2 = FindSoldierByProfileIDOnPlayerTeam(PROF_ICECREAM);
					if ( pSoldier2 )
					{
						// try refueling this vehicle instead
						pSoldier = pSoldier2;
					}
				}
				if ( pSoldier )
				{
					pSoldier->sBreathRed = 10000;
					pSoldier->bBreath    = 100;
					ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, gzLateLocalizedString[STR_LATE_50]);
				}
				break;
			}

			case NPC_ACTION_WALTER_GIVEN_MONEY_INITIALLY:
				if ( gMercProfiles[ WALTER ].iBalance >= params->getGridNo(WALTER_BRIBE_AMOUNT) )
				{
					TriggerNPCRecord( WALTER, 16 );
				}
				else
				{
					TriggerNPCRecord( WALTER, 14 );
				}
				break;
			case NPC_ACTION_WALTER_GIVEN_MONEY:
				if ( gMercProfiles[ WALTER ].iBalance >= params->getGridNo(WALTER_BRIBE_AMOUNT) )
				{
					TriggerNPCRecord( WALTER, 16 );
				}
				else
				{
					TriggerNPCRecord( WALTER, 15 );
				}
				break;
			default:
				SLOGD(ST::format("No code support for NPC action {}", usActionCode));
				break;
		}
	}
}


UINT32 CalcPatientMedicalCost(const SOLDIERTYPE* const pSoldier)
{
	UINT32 uiCost;

	if ( !pSoldier )
	{
		return( 0 );
	}

	if ( pSoldier->bHospitalPriceModifier == HOSPITAL_FREEBIE || pSoldier->bHospitalPriceModifier == HOSPITAL_RANDOM_FREEBIE )
	{
		return( 0 );
	}

	uiCost = 10 * ( pSoldier->bLifeMax - pSoldier->bLife );

	if ( pSoldier->bLife < OKLIFE )
	{
		// charge additional $25 for every point below OKLIFE he is
		uiCost += ( 25 * ( OKLIFE - pSoldier->bLife ) );
	}

	// also charge $2 for each point of bleeding that must be stopped
	if ( pSoldier->bBleeding > 0 )
	{
		uiCost += ( 2 * pSoldier->bBleeding );
	}

	if ( pSoldier->bHospitalPriceModifier == HOSPITAL_BREAK )
	{
		uiCost = (uiCost * 85) / 100;
	}
	else if ( pSoldier->bHospitalPriceModifier == HOSPITAL_COST )
	{
		// 30% discount
		uiCost = (uiCost * 70) / 100;
	}
	else if ( pSoldier->bHospitalPriceModifier == HOSPITAL_UNSET )
	{
		if ( gbHospitalPriceModifier == HOSPITAL_BREAK )
		{
			// 15% discount
			uiCost = (uiCost * 85) / 100;
		}
		else if ( gbHospitalPriceModifier == HOSPITAL_COST )
		{
			// 30% discount
			uiCost = (uiCost * 70) / 100;
		}
	}

	return( uiCost );
}

UINT32 CalcMedicalCost( UINT8 ubId )
{
	UINT32 uiCostSoFar;
	INT16  sGridNo = 0;

	uiCostSoFar = 0;

	// find the doctor's soldiertype to get his position
	const SOLDIERTYPE* const pNPC = FindSoldierByProfileID(ubId);
	if (!pNPC)
	{
		return( 0 );
	}

	sGridNo = pNPC->sGridNo;

	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bInSector &&
			0 < s->bLife && s->bLife < s->bLifeMax &&
			s->bAssignment != ASSIGNMENT_HOSPITAL &&
			PythSpacesAway(sGridNo, s->sGridNo) <= HOSPITAL_PATIENT_DISTANCE)
		{
			uiCostSoFar += CalcPatientMedicalCost(s);
		}
	}

	// round up to nearest 10 dollars
	uiCostSoFar = ( (uiCostSoFar + 9) / 10 );
	uiCostSoFar *= 10;

	// always ask for at least $10
	uiCostSoFar = __max( 10, uiCostSoFar );

	return( uiCostSoFar );
}


static UINT16 gusDialogueMessageBoxType;


static void DialogueMessageBoxCallBack(MessageBoxReturnValue);


static void StartDialogueMessageBox(UINT8 ubProfileID, UINT16 usMessageBoxType)
{
	INT32   iTemp;
	ST::string zTemp;
	ST::string zTemp2;

	gusDialogueMessageBoxType = usMessageBoxType;
	switch( gusDialogueMessageBoxType )
	{
		case NPC_ACTION_ASK_ABOUT_ESCORTING_EPC:
			if ((ubProfileID == JOHN && gMercProfiles[MARY].bMercStatus != MERC_IS_DEAD) ||
				(ubProfileID == MARY && gMercProfiles[JOHN].bMercStatus != MERC_IS_DEAD) )
			{
				zTemp = gzLateLocalizedString[STR_LATE_57];
			}
			else
			{
				zTemp = st_format_printf(TacticalStr[ ESCORT_PROMPT ], gMercProfiles[ubProfileID].zNickname);
			}
			DoMessageBox(MSG_BOX_BASIC_STYLE, zTemp, GAME_SCREEN, MSG_BOX_FLAG_YESNO, DialogueMessageBoxCallBack, NULL);
			break;
		case NPC_ACTION_ASK_ABOUT_PAYING_RPC:
		case NPC_ACTION_ASK_ABOUT_PAYING_RPC_WITH_DAILY_SALARY:
		case NPC_ACTION_REDUCE_CONRAD_SALARY_CONDITIONS:
			zTemp2 = ST::format("${}", gMercProfiles[ubProfileID].sSalary);
			zTemp = st_format_printf(TacticalStr[ HIRE_PROMPT ], gMercProfiles[ubProfileID].zNickname, zTemp2);
			DoMessageBox(MSG_BOX_BASIC_STYLE, zTemp, GAME_SCREEN, MSG_BOX_FLAG_YESNO, DialogueMessageBoxCallBack, NULL);
			break;
		case NPC_ACTION_DARREN_REQUESTOR:
		case NPC_ACTION_FIGHT_AGAIN_REQUESTOR:
			DoMessageBox(MSG_BOX_BASIC_STYLE, TacticalStr[BOXING_PROMPT], GAME_SCREEN, MSG_BOX_FLAG_YESNO, DialogueMessageBoxCallBack, NULL);
			break;
		case NPC_ACTION_BUY_LEATHER_KEVLAR_VEST:
			zTemp2 = ST::format("${}", GCM->getItem(LEATHER_JACKET_W_KEVLAR)->getPrice());
			zTemp = st_format_printf(TacticalStr[ BUY_VEST_PROMPT ], ItemNames[LEATHER_JACKET_W_KEVLAR], zTemp2);
			DoMessageBox(MSG_BOX_BASIC_STYLE, zTemp, GAME_SCREEN, MSG_BOX_FLAG_YESNO, DialogueMessageBoxCallBack, NULL);
			break;
		case NPC_ACTION_PROMPT_PLAYER_TO_LIE:
			DoMessageBox(MSG_BOX_BASIC_STYLE, TacticalStr[YESNOLIE_STR], GAME_SCREEN, MSG_BOX_FLAG_YESNOLIE, DialogueMessageBoxCallBack, NULL);
			break;
		case NPC_ACTION_MEDICAL_REQUESTOR_2:
			DoMessageBox(MSG_BOX_BASIC_STYLE, TacticalStr[FREE_MEDICAL_PROMPT], GAME_SCREEN, MSG_BOX_FLAG_YESNO, DialogueMessageBoxCallBack, NULL);
			break;
		case NPC_ACTION_MEDICAL_REQUESTOR:
			iTemp = (INT32) CalcMedicalCost( ubProfileID );
			if ( giHospitalRefund > iTemp )
			{
				iTemp = 10;
			}
			else
			{
				iTemp -= giHospitalRefund;
			}
			zTemp2 = ST::format("${}", iTemp);
			zTemp = st_format_printf(TacticalStr[ PAY_MONEY_PROMPT ], zTemp2);

			DoMessageBox(MSG_BOX_BASIC_STYLE, zTemp, GAME_SCREEN, MSG_BOX_FLAG_YESNO, DialogueMessageBoxCallBack, NULL);
			break;
		case NPC_ACTION_BUY_VEHICLE_REQUESTOR:
			zTemp = st_format_printf(TacticalStr[PAY_MONEY_PROMPT], "$10000");
			DoMessageBox(MSG_BOX_BASIC_STYLE, zTemp, GAME_SCREEN, MSG_BOX_FLAG_YESNO, DialogueMessageBoxCallBack, NULL);
			break;
		case NPC_ACTION_TRIGGER_MARRY_DARYL_PROMPT:
			DoMessageBox(MSG_BOX_BASIC_STYLE, TacticalStr[MARRY_DARYL_PROMPT], GAME_SCREEN, MSG_BOX_FLAG_YESNO, DialogueMessageBoxCallBack, NULL);
			break;
		case NPC_ACTION_KROTT_REQUESTOR:
			DoMessageBox(MSG_BOX_BASIC_STYLE, TacticalStr[SPARE_KROTT_PROMPT], GAME_SCREEN, MSG_BOX_FLAG_YESNO, DialogueMessageBoxCallBack, NULL);
			break;
		default:
			break;
	}
}


static void DialogueMessageBoxCallBack(MessageBoxReturnValue const ubExitValue)
{
	UINT8 ubProfile;
	SOLDIERTYPE *pSoldier;

	ubProfile = gpDestSoldier->ubProfile;

	switch( gusDialogueMessageBoxType )
	{
		case NPC_ACTION_ASK_ABOUT_ESCORTING_EPC:
			if ( ubExitValue == MSG_BOX_RETURN_YES )
			{
				// Delete menu
				DeleteTalkingMenu( );

				if ( PlayerTeamFull() )
				{
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ CANNOT_RECRUIT_TEAM_FULL ] );
				}
				else
				{
					if ( ubProfile == JOHN )
					{
						// Mary might be alive, and if so we need to ensure two places
						pSoldier = FindSoldierByProfileID(MARY);
						if (pSoldier != NULL &&
								NumberOfMercsOnPlayerTeam() > gTacticalStatus.Team[OUR_TEAM].bLastID - 3u)
						{
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ CANNOT_RECRUIT_TEAM_FULL ] );
							break;
						}
					}

					RecruitEPC( ubProfile );

					// Get soldier pointer
					pSoldier = FindSoldierByProfileID(ubProfile);
					if ( !pSoldier )
					{
						return;
					}

					// OK, update UI with message that we have been recruited
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(TacticalStr[ NOW_BING_ESCORTED_STR ], gMercProfiles[ ubProfile ].zNickname, ( pSoldier->bAssignment + 1 )) );

					// Change Squads....
					SetCurrentSquad( pSoldier ->bAssignment, FALSE );

					HandleStuffForNPCEscorted( ubProfile );
				}
			}
			break;
		case NPC_ACTION_ASK_ABOUT_PAYING_RPC:
		case NPC_ACTION_ASK_ABOUT_PAYING_RPC_WITH_DAILY_SALARY:
			if ( ubExitValue == MSG_BOX_RETURN_YES )
			{
				TriggerNPCRecord( ubProfile, 1 );
			}
			else
			{
				TriggerNPCRecord( ubProfile, 0 );
			}
			break;
		case NPC_ACTION_REDUCE_CONRAD_SALARY_CONDITIONS:
			if ( ubExitValue == MSG_BOX_RETURN_YES )
			{
				TriggerNPCRecord( ubProfile, 1 );
			}
			else
			{
				TriggerNPCRecord( ubProfile, 2 );
			}
			break;

		case NPC_ACTION_DARREN_REQUESTOR:
			if ( ubExitValue == MSG_BOX_RETURN_YES )
			{
				TriggerNPCRecord( ubProfile, 13 );
			}
			else
			{
				TriggerNPCRecord( ubProfile, 12 );
			}
			break;
		case NPC_ACTION_FIGHT_AGAIN_REQUESTOR:
			if ( ubExitValue == MSG_BOX_RETURN_YES )
			{
				TriggerNPCRecord( ubProfile, 30 );
			}
			else
			{
				TriggerNPCRecord( ubProfile, 12 );
			}
			break;
		case NPC_ACTION_BUY_LEATHER_KEVLAR_VEST:
			if ( ubExitValue == MSG_BOX_RETURN_YES )
			{
				TriggerNPCRecord( ubProfile, 27 );
			}
			else
			{
				TriggerNPCRecord( ubProfile, 28 );
			}
			break;
		case NPC_ACTION_PROMPT_PLAYER_TO_LIE:
			if ( ubExitValue == MSG_BOX_RETURN_YES )
			{
				TriggerNPCRecord( ubProfile, (UINT8)( ubRecordThatTriggeredLiePrompt + 1 ) );
			}
			else if ( ubExitValue == MSG_BOX_RETURN_NO )
			{
				TriggerNPCRecord( ubProfile, (UINT8)( ubRecordThatTriggeredLiePrompt + 2 ) );
			}
			else
			{
				// He tried to lie.....
				// Find the best conscious merc with a chance....
				SOLDIERTYPE* pLier = NULL;
				FOR_EACH_IN_TEAM(s, OUR_TEAM)
				{
					if (s->bInSector &&
						s->bLife >= OKLIFE &&
						s->bBreath >= OKBREATH &&
						(!pLier || (EffectiveWisdom(s) + EffectiveLeadership(s) > EffectiveWisdom(pLier) + EffectiveLeadership(s))))
					{
						pLier = s;
					}
				}

				if ( pLier && SkillCheck( pLier, LIE_TO_QUEEN_CHECK, 0 ) >= 0 )
				{
					// SUCCESS..
					TriggerNPCRecord( ubProfile, (UINT8)( ubRecordThatTriggeredLiePrompt + 4 ) );
				}
				else
				{
					// NAUGHY BOY
					TriggerNPCRecord( ubProfile, (UINT8)( ubRecordThatTriggeredLiePrompt + 3 ) );
				}
			}
			break;
		case NPC_ACTION_MEDICAL_REQUESTOR:
		case NPC_ACTION_MEDICAL_REQUESTOR_2:
			if (ubProfile == VINCE)
			{
				if ( gusDialogueMessageBoxType == NPC_ACTION_MEDICAL_REQUESTOR )
				{
					if ( ubExitValue == MSG_BOX_RETURN_YES )
					{
						// give the guy the cash
						TriggerNPCRecord( VINCE, 23 );
					}
					else
					{
						// no cash, no help
						TriggerNPCRecord( VINCE, 24 );
					}
				}
				else
				{
					if ( ubExitValue == MSG_BOX_RETURN_YES )
					{
						//HandleNPCDoAction( VINCE, NPC_ACTION_CHECK_DOCTORING_MONEY_GIVEN, 0 );
						if ( CheckFact( FACT_WOUNDED_MERCS_NEARBY, VINCE) )
						{
							TriggerNPCRecord( VINCE, 26 );
						}
						else if(  CheckFact( FACT_ONE_WOUNDED_MERC_NEARBY, VINCE ) )
						{
							TriggerNPCRecord( VINCE, 25 );
						}
						giHospitalTempBalance = 0;
					}
					else
					{
						// just don't want the help
						TriggerNPCRecord( VINCE, 34 );
					}
				}

				DeleteTalkingMenu();
			}
			else // Steven Willis
			{
				if( gusDialogueMessageBoxType == NPC_ACTION_MEDICAL_REQUESTOR )
				{
					if ( ubExitValue == MSG_BOX_RETURN_YES )
					{
						// give the guy the cash
						TriggerNPCRecord( STEVE, 23 );
					}
					else
					{
						// no cahs, no help
						TriggerNPCRecord( STEVE, 24 );
					}
				}
				else
				{
					if ( ubExitValue == MSG_BOX_RETURN_YES )
					{
						//HandleNPCDoAction( STEVE, NPC_ACTION_CHECK_DOCTORING_MONEY_GIVEN, 0 );
						if ( CheckFact( FACT_WOUNDED_MERCS_NEARBY, STEVE) )
						{
							TriggerNPCRecord( STEVE, 26 );
						}
						else if(  CheckFact( FACT_ONE_WOUNDED_MERC_NEARBY, STEVE ) )
						{
							TriggerNPCRecord( STEVE, 25 );
						}
						gMercProfiles[ VINCE ].iBalance = 0;
					}
					else
					{
						// just don't want the help
						TriggerNPCRecord( STEVE, 30 );
					}
				}

				DeleteTalkingMenu();
			}
			break;
		case NPC_ACTION_BUY_VEHICLE_REQUESTOR:
			if ( ubExitValue == MSG_BOX_RETURN_YES )
			{
				TriggerNPCRecord( GERARD, 9 ); // Dave Gerard
			}
			else
			{
				TriggerNPCRecord( GERARD, 8 );
			}
			break;
		case NPC_ACTION_KROTT_REQUESTOR:
			if ( ubExitValue == MSG_BOX_RETURN_YES )
			{
				TriggerNPCRecord( SERGEANT, 7 );
			}
			else
			{
				TriggerNPCRecord( SERGEANT, 6 );
			}
			break;
		case NPC_ACTION_TRIGGER_MARRY_DARYL_PROMPT:
			gMercProfiles[ gpSrcSoldier->ubProfile ].ubMiscFlags2 |= PROFILE_MISC_FLAG2_ASKED_BY_HICKS;
			if ( ubExitValue == MSG_BOX_RETURN_YES )
			{
				gMercProfiles[ DARYL ].bNPCData = (INT8) gpSrcSoldier->ubProfile;

				// create key for Daryl to give to player
				pSoldier = FindSoldierByProfileID(DARYL);
				if ( pSoldier )
				{
					OBJECTTYPE Key;

					CreateKeyObject( &Key, 1, 38 );
					AutoPlaceObject( pSoldier, &Key, FALSE );
				}
				TriggerNPCRecord( DARYL, 11 );
			}
			else
			{
				TriggerNPCRecord( DARYL, 12 );
			}
			break;
		default:
			break;
	}
}


static void DoneFadeInActionBasement(void);


static void DoneFadeOutActionBasement(void)
{
	// OK, insertion data found, enter sector!
	SetCurrentWorldSector( 10, 1, 1 );

	// OK, once down here, adjust the above map with crate info....
	gfTacticalTraversal = FALSE;
	gpTacticalTraversalGroup = NULL;
	gpTacticalTraversalChosenSoldier = NULL;

	// Remove crate
	RemoveStructFromUnLoadedMapTempFile( 7887, SECONDOSTRUCT1, 10, 1, 0 );
	// Add crate
	AddStructToUnLoadedMapTempFile( 8207, SECONDOSTRUCT1, 10, 1, 0 );

	// Add trapdoor
	AddStructToUnLoadedMapTempFile( 7887, DEBRIS2MISC1, 10, 1, 0 );


	gFadeInDoneCallback = DoneFadeInActionBasement;

	FadeInGameScreen( );

}

static void DoneFadeInActionBasement(void)
{
	// Start conversation, etc

	// Look for someone to talk to
	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		// Are we in this sector, On the current squad?
		if (s->bLife >= OKLIFE && s->bInSector && s->bAssignment == CurrentSquad())
		{
			break;
		}
	}

	const SOLDIERTYPE* const pNPCSoldier = FindSoldierByProfileID(CARLOS);
	if ( !pNPCSoldier )
	{
		return;
	}

	TriggerNPCRecordImmediately( pNPCSoldier->ubProfile, 1 );
}


static void DoneFadeInActionLeaveBasement(void);


static void DoneFadeOutActionLeaveBasement(void)
{
	// OK, insertion data found, enter sector!
	SetCurrentWorldSector( 10, 1, 0 );

	gfTacticalTraversal = FALSE;
	gpTacticalTraversalGroup = NULL;
	gpTacticalTraversalChosenSoldier = NULL;

	gFadeInDoneCallback = DoneFadeInActionLeaveBasement;

	FadeInGameScreen( );

}


static void DoneFadeInActionLeaveBasement(void)
{
	// Start conversation, etc
}


static BOOLEAN NPCOpenThing(SOLDIERTYPE* pSoldier, BOOLEAN fDoor)
{
	STRUCTURE *pStructure;
	INT16 sStructGridNo;
	INT16 sActionGridNo;
	UINT8 ubDirection;
	INT16 sGridNo;
	DOOR  *pDoor;

	// Find closest door and get struct data for it!
	if ( fDoor )
	{
		sStructGridNo = FindClosestDoor( pSoldier );

		if ( sStructGridNo == NOWHERE )
		{
			return( FALSE );
		}

		pStructure = FindStructure( sStructGridNo, STRUCTURE_ANYDOOR );
	}
	else
	{
		// for Armand, hard code to tile 6968
		if ( pSoldier->ubProfile == ARMAND )
		{
			sStructGridNo = 6968;
		}
		else
		{
			sStructGridNo = FindNearestOpenableNonDoor( pSoldier->sGridNo );
		}

		if ( sStructGridNo == NOWHERE )
		{
			return( FALSE );
		}

		pStructure = FindStructure( sStructGridNo, STRUCTURE_OPENABLE );
	}

	if ( pStructure == NULL )
	{
		return( FALSE );
	}

	if ( pStructure->fFlags & STRUCTURE_OPEN )
	{
		// it's already open!
		TriggerNPCWithGivenApproach(pSoldier->ubProfile, APPROACH_DONE_OPEN_STRUCTURE);
		return( FALSE );
	}

	// anything that an NPC opens this way should become unlocked!

	pDoor = FindDoorInfoAtGridNo( sStructGridNo );
	if ( pDoor )
	{
		pDoor->fLocked = FALSE;
	}

	sActionGridNo =  FindAdjacentGridEx( pSoldier, sStructGridNo, &ubDirection, NULL, FALSE, TRUE );
	if ( sActionGridNo == -1 )
	{
		return( FALSE );
	}

	// Set dest gridno
	sGridNo = sActionGridNo;

	StartInteractiveObject(sStructGridNo, *pStructure, *pSoldier, ubDirection);

	// check if we are at this location
	if ( pSoldier->sGridNo == sGridNo )
	{
		InteractWithOpenableStruct(*pSoldier, *pStructure, ubDirection);
	}
	else
	{
		SendGetNewSoldierPathEvent(pSoldier, sGridNo);
	}

	pSoldier->bAction = AI_ACTION_PENDING_ACTION;

	return( TRUE );

}


static void TextRegionClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	static BOOLEAN fLButtonDown = FALSE;

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		fLButtonDown = TRUE;
	}

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP && fLButtonDown )
	{
		InternalShutupaYoFace(gTalkPanel.face, FALSE);
	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		fLButtonDown = FALSE;
	}
}


static void CarmenLeavesSectorCallback(void)
{
	if (gWorldSectorX == 13 && gWorldSectorY == MAP_ROW_C && gbWorldSectorZ == 0)
	{
		TriggerNPCRecord(CARMEN, 34);
	}
	else if (gWorldSectorX == 9 && gWorldSectorY == MAP_ROW_G && gbWorldSectorZ == 0)
	{
		TriggerNPCRecord(CARMEN, 35);
	}
	else if (gWorldSectorX == 5 && gWorldSectorY == MAP_ROW_C && gbWorldSectorZ == 0)
	{
		TriggerNPCRecord(CARMEN, 36);
	}

}
