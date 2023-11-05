#include "Directories.h"
#include "Font.h"
#include "Font_Control.h"
#include "HImage.h"
#include "Medical.h"
#include "MercPortrait.h"
#include "Overhead.h"
#include "MessageBoxScreen.h"
#include "ScreenIDs.h"
#include "Handle_UI.h"
#include "Game_Clock.h"
#include "Items.h"
#include "Tactical_Placement_GUI.h"
#include "Text.h"
#include "Interface.h"
#include "Auto_Bandage.h"
#include "RenderWorld.h"
#include "Strategic_Turns.h"
#include "Event_Pump.h"
#include "Dialogue_Control.h"
#include "AI.h"
#include "Interface_Control.h"
#include "StrategicMap.h"
#include "Animation_Control.h"
#include "Map_Screen_Interface.h"
#include "Soldier_Profile.h"
#include "Assignments.h"
#include "Cursors.h"
#include "VObject.h"
#include "Video.h"
#include "VSurface.h"
#include "Button_System.h"
#include "UILayout.h"

#include "Soldier.h"

#include <string_theory/string>

#include <algorithm>
#include <iterator>

// max number of merc faces per row in autobandage box
#define NUMBER_MERC_FACES_AUTOBANDAGE_BOX 4


static BOOLEAN gfBeginningAutoBandage = FALSE;
static UINT32  guiAutoBandageSeconds  = 0;
static BOOLEAN fAutoBandageComplete   = FALSE;
static BOOLEAN fEndAutoBandage        = FALSE;

BOOLEAN gfAutoBandageFailed;

// the button and associated image for ending autobandage
static GUIButtonRef iEndAutoBandageButton[2];


static MOUSE_REGION gAutoBandageRegion;


// the lists of the doctor and patient
static const SOLDIERTYPE* gdoctor_list[MAX_CHARACTER_COUNT];
static const SOLDIERTYPE* gpatient_list[MAX_CHARACTER_COUNT];

// faces for update panel
static SGPVObject* giAutoBandagesSoldierFaces[2 * MAX_CHARACTER_COUNT];

// has the button for autobandage end been setup yet
static BOOLEAN fAutoEndBandageButtonCreated = FALSE;


static void BeginAutoBandageCallBack(MessageBoxReturnValue);


void BeginAutoBandage( )
{
	BOOLEAN fFoundAGuy = FALSE;
	BOOLEAN fFoundAMedKit = FALSE;

	// If we are in combat, we con't...
	if ( (gTacticalStatus.uiFlags & INCOMBAT) || (NumEnemyInSector() != 0) )
	{
		DoMessageBox(MSG_BOX_BASIC_STYLE, g_langRes->Message[STR_SECTOR_NOT_CLEARED], GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL);
		return;
	}

	// check for anyone needing bandages
	CFOR_EACH_IN_TEAM(pSoldier, OUR_TEAM)
	{
		// if the soldier isn't active or in sector, we have problems..leave
		if (!pSoldier->bInSector || pSoldier->uiStatusFlags & SOLDIER_VEHICLE || pSoldier->bAssignment == VEHICLE)
		{
			continue;
		}

		// can this character be helped out by a teammate?
		if (CanCharacterBeAutoBandagedByTeammate(pSoldier))
		{
			fFoundAGuy = TRUE;
			if ( fFoundAGuy && fFoundAMedKit )
			{
				break;
			}
		}
		if ( FindObjClass( pSoldier, IC_MEDKIT ) != NO_SLOT )
		{
			fFoundAMedKit = TRUE;
			if ( fFoundAGuy && fFoundAMedKit )
			{
				break;
			}
		}

	}

	if ( !fFoundAGuy )
	{
		DoMessageBox(MSG_BOX_BASIC_STYLE, TacticalStr[AUTOBANDAGE_NOT_NEEDED], GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL);
	}
	else if ( !fFoundAMedKit )
	{
		DoMessageBox(MSG_BOX_BASIC_STYLE, gzLateLocalizedString[STR_LATE_09], GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL);
	}
	else
	{
		if ( ! CanAutoBandage( FALSE ) )
		{
			DoMessageBox(MSG_BOX_BASIC_STYLE, TacticalStr[CANT_AUTOBANDAGE_PROMPT], GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL);
		}
		else
		{
			// Confirm if we want to start or not....
			DoMessageBox(MSG_BOX_BASIC_STYLE, TacticalStr[BEGIN_AUTOBANDAGE_PROMPT_STR], GAME_SCREEN, MSG_BOX_FLAG_YESNO, BeginAutoBandageCallBack, NULL);
		}
	}
}


void HandleAutoBandagePending( )
{
	// OK, if we have a pending autobandage....
	// check some conditions
	if ( gTacticalStatus.fAutoBandagePending )
	{
		// All dailogue done, music, etc...
		//if ( gubMusicMode != MUSIC_TACTICAL_VICTORY && DialogueQueueIsEmpty( ) )
		if (!DialogueQueueIsEmptyAndNobodyIsTalking())
		{
			return;
		}

		// Do any guys have pending actions...?
		FOR_EACH_IN_TEAM(s, OUR_TEAM)
		{
			if (s->sSector == gWorldSector &&
				!s->fBetweenSectors &&
				Soldier{s}.hasPendingAction())
			{
				return;
			}
		}

		// Do was have any menus up?
		if (AreWeInAUIMenu() || gCurrentUIMode == LOCKUI_MODE)
		{
			return;
		}



		// If here, all's a go!
		gTacticalStatus.fAutoBandagePending = FALSE;
		BeginAutoBandage( );
	}
}


void SetAutoBandagePending( BOOLEAN fSet )
{
	gTacticalStatus.fAutoBandagePending = fSet;
}


// Should we ask buddy ti auto bandage...?
void ShouldBeginAutoBandage( )
{
	// If we are in combat, we con't...
	if ( gTacticalStatus.uiFlags & INCOMBAT )
	{
		return;
	}

	// ATE: If not in endgame
	if ( gTacticalStatus.uiFlags & IN_DEIDRANNA_ENDGAME )
	{
		return;
	}

	if ( CanAutoBandage( FALSE ) )
	{
	// OK, now setup as a pending event...
		SetAutoBandagePending( TRUE );
	}
}


static void DisplayAutoBandageUpdatePanel(void);


BOOLEAN HandleAutoBandage( )
{
	InputAtom InputEvent;

	if ( gTacticalStatus.fAutoBandageMode )
	{
		if ( gfBeginningAutoBandage )
		{
			//Shadow area
			FRAME_BUFFER->ShadowRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			InvalidateScreen( );
		}

		DisplayAutoBandageUpdatePanel( );

		// Handle strategic engine
		HandleStrategicTurn( );

		HandleTeamServices( OUR_TEAM );

		if ( guiAutoBandageSeconds <= 120 )
		{
			guiAutoBandageSeconds += 5;
		}

		// Execute Tactical Overhead
		ExecuteOverhead( );

		// Deque all game events
		DequeAllGameEvents();

		while (DequeueSpecificEvent(&InputEvent, KEYBOARD_EVENTS))
		{
			if ( InputEvent.usEvent == KEY_UP )
			{
				if ((InputEvent.usParam == SDLK_ESCAPE && !fAutoBandageComplete) ||
					((InputEvent.usParam == SDLK_RETURN ||
					InputEvent.usParam == SDLK_SPACE) && fAutoBandageComplete))
				{
					AutoBandage( FALSE );
				}
			}
		}

		gfBeginningAutoBandage = FALSE;

		if( fEndAutoBandage )
		{
			AutoBandage( FALSE );
			fEndAutoBandage = FALSE;
		}

		return( TRUE );
	}



	return( FALSE );
}


void SetAutoBandageComplete( void )
{
	// this will set the fact autobandage is complete
	fAutoBandageComplete = TRUE;
}


static void DestroyTerminateAutoBandageButton(void);
static void RemoveFacesForAutoBandage(void);
static void SetUpAutoBandageUpdatePanel(void);


void AutoBandage( BOOLEAN fStart )
{
	if ( fStart )
	{
		gTacticalStatus.fAutoBandageMode = TRUE;
		gTacticalStatus.uiFlags |= OUR_MERCS_AUTO_MOVE;

		gfAutoBandageFailed = FALSE;


		// ste up the autobandage panel
		SetUpAutoBandageUpdatePanel( );

		// Lock UI!
		//guiPendingOverrideEvent = LU_BEGINUILOCK;
		HandleTacticalUI( );

		PauseGame();
		// Compress time...
		//SetGameTimeCompressionLevel( TIME_COMPRESS_5MINS );

		FOR_EACH_IN_TEAM(s, OUR_TEAM)
		{
			s->bSlotItemTakenFrom   = NO_SLOT;
			s->auto_bandaging_medic = NULL;
		}

		// build a mask
		MSYS_DefineRegion(&gAutoBandageRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST - 1, CURSOR_NORMAL, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);

		gfBeginningAutoBandage = TRUE;

	}
	else
	{
		gTacticalStatus.fAutoBandageMode = FALSE;
		gTacticalStatus.uiFlags &= ( ~OUR_MERCS_AUTO_MOVE );

		FOR_EACH_IN_TEAM(s, OUR_TEAM)
		{
			ActionDone(s);
			if (s->bSlotItemTakenFrom != NO_SLOT)
			{
				// swap our old hand item back to the main hand
				SwapObjs(&s->inv[HANDPOS], &s->inv[s->bSlotItemTakenFrom]);
			}

			// ATE: Mkae everyone stand up!
			if (s->bLife >= OKLIFE && !s->bCollapsed)
			{
				if (gAnimControl[s->usAnimState].ubHeight != ANIM_STAND)
				{
					ChangeSoldierStance(s, ANIM_STAND);
				}
			}
		}

		FOR_EACH_IN_TEAM(s, OUR_TEAM)
		{
			ActionDone(s);

			// If anyone is still doing aid animation, stop!
			if (s->usAnimState == GIVING_AID) SoldierGotoStationaryStance(s);
		}

		// UnLock UI!
		guiPendingOverrideEvent = LU_ENDUILOCK;
		HandleTacticalUI( );

		UnPauseGame();
		// Bring time back...
		//SetGameTimeCompressionLevel( TIME_COMPRESS_X1 );

		//Warp game time by the amount of time it took to autobandage.
		WarpGameTime( guiAutoBandageSeconds, WARPTIME_NO_PROCESSING_OF_EVENTS );

		DestroyTerminateAutoBandageButton( );

		// build a mask
		MSYS_RemoveRegion( &gAutoBandageRegion );

		// clear faces for auto bandage
		RemoveFacesForAutoBandage( );

		SetRenderFlags( RENDER_FLAG_FULL );
		fInterfacePanelDirty = DIRTYLEVEL2;

		if( gfAutoBandageFailed )
		{
			// inform player some mercs could not be bandaged
			DoScreenIndependantMessageBox(pDoctorWarningString[ 1 ], MSG_BOX_FLAG_OK, NULL );
			gfAutoBandageFailed = FALSE;
		}
	}
	guiAutoBandageSeconds = 0;

	ResetAllMercSpeeds( );
}


static void BeginAutoBandageCallBack(MessageBoxReturnValue const bExitValue)
{
	if( bExitValue == MSG_BOX_RETURN_YES )
	{
		fRestoreBackgroundForMessageBox = TRUE;
		AutoBandage( TRUE );
	}
}


static void AddFacesToAutoBandageBox(void);


// the update box for autobandaging mercs
static void SetUpAutoBandageUpdatePanel(void)
{
	const SOLDIERTYPE** next_doctor  = gdoctor_list;
	const SOLDIERTYPE** next_patient = gpatient_list;
	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (CanCharacterAutoBandageTeammate(s))      *next_doctor++  = s;
		if (CanCharacterBeAutoBandagedByTeammate(s)) *next_patient++ = s;
	}

	// makes sure there is someone to doctor and patient...
	if (next_doctor  == gdoctor_list)  next_patient = gpatient_list;
	if (next_patient == gpatient_list) next_doctor  = gdoctor_list;

	// Clear the rest of the lists
	while (next_doctor  != endof(gdoctor_list))  *next_doctor++  = NULL;
	while (next_patient != endof(gpatient_list)) *next_patient++ = NULL;

	// now add the faces
	AddFacesToAutoBandageBox( );

	fAutoBandageComplete = FALSE;
}


static void CreateTerminateAutoBandageButton(INT16 sX, INT16 sY);
static BOOLEAN RenderSoldierSmallFaceForAutoBandagePanel(INT32 iIndex, INT16 sCurrentXPosition, INT16 sCurrentYPosition);


static void DisplayAutoBandageUpdatePanel(void)
{
	INT32 iNumberDoctors = 0, iNumberPatients = 0;
	INT32 iNumberDoctorsHigh = 0, iNumberPatientsHigh = 0;
	INT32 iNumberDoctorsWide = 0, iNumberPatientsWide = 0;
	INT32 iTotalPixelsHigh = 0, iTotalPixelsWide = 0;
	INT32 iCurPixelY = 0;
	INT16 sXPosition = 0, sYPosition = 0;
	INT32 iCounterA = 0, iCounterB = 0;
	INT32 iIndex = 0;
	INT16 sCurrentXPosition = 0, sCurrentYPosition = 0;
	INT16 sX = 0, sY = 0;

	// are even in autobandage mode?
	if (!gTacticalStatus.fAutoBandageMode)
	{
		// nope,
		return;
	}

	// make sure there is someone to doctor and patient
	if (gdoctor_list[0] == NULL || gpatient_list[0] == NULL)
	{
		// nope, nobody here
		return;
	}

	// grab number of doctors
	for (iCounterA = 0; gdoctor_list[iCounterA] != NULL; ++iCounterA)
	{
		iNumberDoctors++;
	}

	// grab number of patients
	for (iCounterA = 0; gpatient_list[iCounterA] != NULL; ++iCounterA)
	{
		iNumberPatients++;
	}

	// build dimensions of box

	if( iNumberDoctors < NUMBER_MERC_FACES_AUTOBANDAGE_BOX )
	{
		// nope, get the base amount
		iNumberDoctorsWide = ( iNumberDoctors % NUMBER_MERC_FACES_AUTOBANDAGE_BOX );
	}
	else
	{
		iNumberDoctorsWide = NUMBER_MERC_FACES_AUTOBANDAGE_BOX;
	}



	// set the min number of mercs
	if( iNumberDoctorsWide < 3 )
	{
		iNumberDoctorsWide = 2;
	}
	else
	{
		// a full row
		iNumberDoctorsWide = NUMBER_MERC_FACES_AUTOBANDAGE_BOX;
	}

	// the doctors
	iNumberDoctorsHigh = ( iNumberDoctors / ( NUMBER_MERC_FACES_AUTOBANDAGE_BOX ) + 1 );

	if( iNumberDoctors % NUMBER_MERC_FACES_AUTOBANDAGE_BOX )
	{
		// now the patients
		iNumberDoctorsHigh = ( iNumberDoctors / ( NUMBER_MERC_FACES_AUTOBANDAGE_BOX ) + 1 );
	}
	else
	{
		// now the patients
		iNumberDoctorsHigh = ( iNumberDoctors / ( NUMBER_MERC_FACES_AUTOBANDAGE_BOX )  );
	}

	if( iNumberPatients < NUMBER_MERC_FACES_AUTOBANDAGE_BOX )
	{
		// nope, get the base amount
		iNumberPatientsWide = ( iNumberPatients % NUMBER_MERC_FACES_AUTOBANDAGE_BOX );
	}
	else
	{
		iNumberPatientsWide = NUMBER_MERC_FACES_AUTOBANDAGE_BOX;
	}

	// set the min number of mercs
	if( iNumberPatientsWide < 3 )
	{
		iNumberPatientsWide = 2;
	}
	else
	{
		// a full row
		iNumberPatientsWide = NUMBER_MERC_FACES_AUTOBANDAGE_BOX;
	}

	if( iNumberPatients % NUMBER_MERC_FACES_AUTOBANDAGE_BOX )
	{
		// now the patients
		iNumberPatientsHigh = ( iNumberPatients / ( NUMBER_MERC_FACES_AUTOBANDAGE_BOX ) + 1  );
	}
	else
	{
		// now the patients
		iNumberPatientsHigh = ( iNumberPatients / ( NUMBER_MERC_FACES_AUTOBANDAGE_BOX )  );
	}



	// now the actual pixel dimensions

	iTotalPixelsHigh = ( iNumberPatientsHigh + iNumberDoctorsHigh ) * TACT_UPDATE_MERC_FACE_X_HEIGHT;

	// see which is wider, and set to this
	if( iNumberDoctorsWide > iNumberPatientsWide )
	{
		iNumberPatientsWide = iNumberDoctorsWide;
	}
	else
	{
		iNumberDoctorsWide = iNumberPatientsWide;
	}

	iTotalPixelsWide = TACT_UPDATE_MERC_FACE_X_WIDTH * iNumberDoctorsWide;

	// now get the x and y position for the box
	sXPosition = (SCREEN_WIDTH          - iTotalPixelsWide) / 2;
	sYPosition = (INV_INTERFACE_START_Y - iTotalPixelsHigh) / 2;

	const SGPVObject* const hBackGroundHandle = guiUpdatePanelTactical;

	// first the doctors on top
	for( iCounterA = 0; iCounterA < iNumberDoctorsHigh; iCounterA++ )
	{
		for( iCounterB = 0; iCounterB < iNumberDoctorsWide; iCounterB ++ )
		{
			sCurrentXPosition = sXPosition + ( iCounterB * TACT_UPDATE_MERC_FACE_X_WIDTH );
			sCurrentYPosition = sYPosition + ( iCounterA * TACT_UPDATE_MERC_FACE_X_HEIGHT );

			// slap down background piece
			BltVideoObject( FRAME_BUFFER , hBackGroundHandle, 15, sCurrentXPosition, sCurrentYPosition);


			iIndex = iCounterA * iNumberDoctorsWide + iCounterB;

			const SOLDIERTYPE* const doctor = gdoctor_list[iIndex];
			if (doctor != NULL)
			{

				sCurrentXPosition += TACT_UPDATE_MERC_FACE_X_OFFSET;
				sCurrentYPosition += TACT_UPDATE_MERC_FACE_Y_OFFSET;

				// there is a face
				RenderSoldierSmallFaceForAutoBandagePanel( iIndex, sCurrentXPosition, sCurrentYPosition );

				// display the mercs name
				ST::string Name = doctor->name;
				FindFontCenterCoordinates(sCurrentXPosition, sCurrentYPosition, TACT_UPDATE_MERC_FACE_X_WIDTH - 25, 0, Name, TINYFONT1, &sX, &sY);
				SetFontAttributes(TINYFONT1, FONT_LTRED);

				sY+= 35;
				sCurrentXPosition -= TACT_UPDATE_MERC_FACE_X_OFFSET;
				sCurrentYPosition -= TACT_UPDATE_MERC_FACE_Y_OFFSET;

				MPrint(sX, sY, Name);
				//sCurrentYPosition-= TACT_UPDATE_MERC_FACE_Y_OFFSET;
			}
		}
	}

	for( iCounterB = 0; iCounterB < iNumberPatientsWide; iCounterB ++ )
	{
		// slap down background piece
		BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 16,
				sXPosition + (iCounterB * TACT_UPDATE_MERC_FACE_X_WIDTH),
				sCurrentYPosition + (TACT_UPDATE_MERC_FACE_X_HEIGHT));
		BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 16,
				sXPosition + (iCounterB * TACT_UPDATE_MERC_FACE_X_WIDTH),
				sYPosition - 9);


	}

	// bordering patient title
	BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 11, sXPosition - 4,
			sYPosition + ((iNumberDoctorsHigh) * TACT_UPDATE_MERC_FACE_X_HEIGHT));
	BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 13, sXPosition + iTotalPixelsWide,
			sYPosition + ((iNumberDoctorsHigh) * TACT_UPDATE_MERC_FACE_X_HEIGHT));


	SetFontAttributes(TINYFONT1, FONT_WHITE);

	//iCurPixelY = sYPosition;
	iCurPixelY = sYPosition + ((iCounterA - 1) * TACT_UPDATE_MERC_FACE_X_HEIGHT);


	ST::string Medics = zMarksMapScreenText[13];
	FindFontCenterCoordinates(sXPosition, sCurrentYPosition, iTotalPixelsWide, 0, Medics, TINYFONT1, &sX, &sY);
	// print medic
	MPrint(sX, sYPosition - 7, Medics);

	//DisplayWrappedString(sXPosition,  sCurrentYPosition - 40, iTotalPixelsWide, 0, TINYFONT1, FONT_WHITE, pUpdateMercStrings[0], FONT_BLACK, 0);


	sYPosition += 9;

	// now the patients
	for( iCounterA = 0; iCounterA < iNumberPatientsHigh; iCounterA++ )
	{
		for( iCounterB = 0; iCounterB < iNumberPatientsWide; iCounterB ++ )
		{

			sCurrentXPosition = sXPosition + ( iCounterB * TACT_UPDATE_MERC_FACE_X_WIDTH );
			sCurrentYPosition = sYPosition + ( ( iCounterA + iNumberDoctorsHigh ) * TACT_UPDATE_MERC_FACE_X_HEIGHT );


			// slap down background piece
			BltVideoObject( FRAME_BUFFER , hBackGroundHandle, 15, sCurrentXPosition, 	sCurrentYPosition);


			iIndex = iCounterA * iNumberPatientsWide + iCounterB;

			const SOLDIERTYPE* const patient = gpatient_list[iIndex];
			if (patient != NULL)
			{

				sCurrentXPosition += TACT_UPDATE_MERC_FACE_X_OFFSET;
				sCurrentYPosition += TACT_UPDATE_MERC_FACE_Y_OFFSET;

				// there is a face
				RenderSoldierSmallFaceForAutoBandagePanel( iIndex + iNumberDoctors, sCurrentXPosition, sCurrentYPosition );

				// display the mercs name
				ST::string Name = patient->name;
				FindFontCenterCoordinates(sCurrentXPosition, sCurrentYPosition, TACT_UPDATE_MERC_FACE_X_WIDTH - 25, 0, Name, TINYFONT1, &sX, &sY);
				SetFontAttributes(TINYFONT1, FONT_LTRED);
				sY+= 35;
				MPrint(sX, sY, Name);
			}

		}
	}


	// BORDER PIECES!!!!

	// bordering patients squares
	for( iCounterA = 0; iCounterA < iNumberPatientsHigh; iCounterA++ )
	{
		BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 3, sXPosition - 4,
				sYPosition + ((iCounterA + iNumberDoctorsHigh) * TACT_UPDATE_MERC_FACE_X_HEIGHT));
		BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 5, sXPosition + iTotalPixelsWide,
				sYPosition + ((iCounterA + iNumberDoctorsHigh) * TACT_UPDATE_MERC_FACE_X_HEIGHT));
	}


	// back up 11 pixels
	sYPosition-=9;

	// pieces bordering doctor squares
	for( iCounterA = 0; iCounterA < iNumberDoctorsHigh; iCounterA++ )
	{
		BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 3, sXPosition - 4,
				sYPosition + ((iCounterA) * TACT_UPDATE_MERC_FACE_X_HEIGHT));
		BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 5, sXPosition + iTotalPixelsWide,
				sYPosition + ((iCounterA) * TACT_UPDATE_MERC_FACE_X_HEIGHT));
	}

	// bordering doctor title
	BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 11, sXPosition - 4, sYPosition - 9);
	BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 13, sXPosition + iTotalPixelsWide , sYPosition - 9);


	// now the top pieces
	for( iCounterA = 0; iCounterA < iNumberPatientsWide; iCounterA++ )
	{
		// the top bottom
		BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 1,
				sXPosition + TACT_UPDATE_MERC_FACE_X_WIDTH * (iCounterA), sYPosition - 13);
	}

	// the top corners
	BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 0, sXPosition -4, sYPosition - 13);
	BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 2, sXPosition + iTotalPixelsWide, sYPosition - 13);

	iTotalPixelsHigh+= 9;

	// the bottom
	BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 17, sXPosition - 4, sYPosition + iTotalPixelsHigh);
	BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 18,
			sXPosition + iTotalPixelsWide - TACT_UPDATE_MERC_FACE_X_WIDTH,
			sYPosition + iTotalPixelsHigh);

	if( iNumberPatientsWide == 2 )
	{
		BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 6, sXPosition - 4, sYPosition + iTotalPixelsHigh);
		CreateTerminateAutoBandageButton((INT16)(sXPosition), (INT16)(sYPosition + iTotalPixelsHigh + 3));
	}
	else
	{
		BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 6,
				sXPosition + TACT_UPDATE_MERC_FACE_X_WIDTH - 4,
				sYPosition + iTotalPixelsHigh);
		CreateTerminateAutoBandageButton((INT16)(sXPosition + TACT_UPDATE_MERC_FACE_X_WIDTH),
							(INT16)(sYPosition + iTotalPixelsHigh + 3));
	}

	SetFontAttributes(TINYFONT1, FONT_WHITE);

	ST::string Patients = zMarksMapScreenText[14];
	FindFontCenterCoordinates(sXPosition, sCurrentYPosition, iTotalPixelsWide, 0, Patients, TINYFONT1, &sX, &sY);
	// print patient
	MPrint(sX, iCurPixelY + TACT_UPDATE_MERC_FACE_X_HEIGHT + 2, Patients);

	MarkAButtonDirty( iEndAutoBandageButton[ 0 ] );
	MarkAButtonDirty( iEndAutoBandageButton[ 1 ] );

	iEndAutoBandageButton[0]->Draw();
	iEndAutoBandageButton[1]->Draw();

	iTotalPixelsHigh+= 35;

	// if autobandage is complete, set the fact by enabling the done button
	bool const complete = fAutoBandageComplete;
	EnableButton(iEndAutoBandageButton[0],  complete);
	EnableButton(iEndAutoBandageButton[1], !complete);

	// now make sure it goes to the screen
	InvalidateRegion(sXPosition - 4, sYPosition - 18, (INT16)(sXPosition + iTotalPixelsWide + 4),
				(INT16)(sYPosition + iTotalPixelsHigh));
}


static void StopAutoBandageButtonCallback(GUI_BUTTON* btn, UINT32 reason);


static void MakeButton(UINT idx, INT16 x, INT16 y, const ST::string& text)
{
	GUIButtonRef const btn = QuickCreateButtonImg(INTERFACEDIR "/group_confirm_tactical.sti", 7, 8, x, y,
							MSYS_PRIORITY_HIGHEST - 1, StopAutoBandageButtonCallback);
	iEndAutoBandageButton[idx] = btn;
	btn->SpecifyGeneralTextAttributes(text, MAP_SCREEN_FONT, FONT_MCOLOR_BLACK, FONT_BLACK);
}


static void CreateTerminateAutoBandageButton(INT16 sX, INT16 sY)
{
	// create the kill autobandage button
	if( fAutoEndBandageButtonCreated )
	{
		// button created, leave
		return;
	}

	fAutoEndBandageButtonCreated = TRUE;

	MakeButton(0, sX,      sY, zMarksMapScreenText[15]); // the continue button
	MakeButton(1, sX + 70, sY, zMarksMapScreenText[16]); // the cancel button
}


static void StopAutoBandageButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		fEndAutoBandage = TRUE;
	}
}


static void DestroyTerminateAutoBandageButton(void)
{
	// destroy the kill autobandage button
	if (!fAutoEndBandageButtonCreated)
	{
		// not around, don't destroy what ain't there
		return;
	}

	fAutoEndBandageButtonCreated = FALSE;

	// remove button
	RemoveButton( iEndAutoBandageButton[ 0 ] );
	RemoveButton( iEndAutoBandageButton[ 1 ] );
}


static void AddFacesToAutoBandageBox(void)
{
	INT32 iCounter = 0;
	INT32 iNumberOfDoctors = 0;

	// reset
	std::fill(std::begin(giAutoBandagesSoldierFaces), std::end(giAutoBandagesSoldierFaces), nullptr);

	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		const SOLDIERTYPE* const doctor = gdoctor_list[iCounter];
		if (doctor != NULL)
		{
			giAutoBandagesSoldierFaces[iCounter] = Load65Portrait(GetProfile(doctor->ubProfile));
			iNumberOfDoctors++;
		}
	}

	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		const SOLDIERTYPE* const patient = gpatient_list[iCounter];
		if (patient != NULL)
		{
			giAutoBandagesSoldierFaces[iCounter + iNumberOfDoctors] = Load65Portrait(GetProfile(patient->ubProfile));
		}
	}

	// grab panels
	giMercPanelImage = AddVideoObjectFromFile(INTERFACEDIR "/panels.sti");
}


static void RemoveFacesForAutoBandage(void)
{
	INT32 iCounter = 0, iNumberOfDoctors = 0;


	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if (gdoctor_list[iCounter] != NULL)
		{
			DeleteVideoObject(giAutoBandagesSoldierFaces[iCounter]);
			iNumberOfDoctors++;
		}
	}


	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if (gpatient_list[iCounter] != NULL)
		{
			DeleteVideoObject(giAutoBandagesSoldierFaces[iCounter + iNumberOfDoctors]);
		}
	}

	DeleteVideoObject(giMercPanelImage);
}


static BOOLEAN RenderSoldierSmallFaceForAutoBandagePanel(INT32 iIndex, INT16 sCurrentXPosition, INT16 sCurrentYPosition)
{
	INT32 iStartY = 0;
	INT32 iCounter = 0, iIndexCount = 0;

	// fill the background for the info bars black
	ColorFillVideoSurfaceArea( FRAME_BUFFER, sCurrentXPosition+36, sCurrentYPosition+2, sCurrentXPosition+44,	sCurrentYPosition+30, 0 );

	// put down the background
	BltVideoObject(FRAME_BUFFER, giMercPanelImage, 0, sCurrentXPosition, sCurrentYPosition);

	// grab the face
	BltVideoObject(FRAME_BUFFER, giAutoBandagesSoldierFaces[iIndex], 0, sCurrentXPosition + 2, sCurrentYPosition + 2);


	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if (gdoctor_list[iCounter] != NULL) ++iIndexCount;
	}

	// see if we are looking into doctor or patient lists?
	const SOLDIERTYPE* const pSoldier = (iIndexCount > iIndex ? gdoctor_list[iIndex] : gpatient_list[iIndex - iIndexCount]);

	// is the merc alive?
	if( !pSoldier->bLife )
		return( FALSE );


	//yellow one for bleeding
	iStartY = sCurrentYPosition + 29 - 27*pSoldier->bLifeMax/100;
	ColorFillVideoSurfaceArea(FRAME_BUFFER, sCurrentXPosition+36, iStartY, sCurrentXPosition+37,
					sCurrentYPosition+29, RGB(107, 107, 57));
	ColorFillVideoSurfaceArea(FRAME_BUFFER, sCurrentXPosition+37, iStartY, sCurrentXPosition+38,
					sCurrentYPosition+29, RGB(222, 181, 115));

	//pink one for bandaged.
	iStartY += 27*pSoldier->bBleeding/100;
	ColorFillVideoSurfaceArea(FRAME_BUFFER, sCurrentXPosition+36, iStartY, sCurrentXPosition+37,
					sCurrentYPosition+29, RGB(156, 57, 57));
	ColorFillVideoSurfaceArea(FRAME_BUFFER, sCurrentXPosition+37, iStartY, sCurrentXPosition+38,
					sCurrentYPosition+29, RGB(222, 132, 132));

	//red one for actual health
	iStartY = sCurrentYPosition + 29 - 27*pSoldier->bLife/100;
	ColorFillVideoSurfaceArea(FRAME_BUFFER, sCurrentXPosition+36, iStartY, sCurrentXPosition+37,
					sCurrentYPosition+29, RGB(107, 8, 8));
	ColorFillVideoSurfaceArea(FRAME_BUFFER, sCurrentXPosition+37, iStartY, sCurrentXPosition+38,
					sCurrentYPosition+29, RGB(206, 0, 0));

	//BREATH BAR
	iStartY = sCurrentYPosition + 29 - 27*pSoldier->bBreathMax/100;
	ColorFillVideoSurfaceArea(FRAME_BUFFER, sCurrentXPosition+39, iStartY, sCurrentXPosition+40,
					sCurrentYPosition+29, RGB(8, 8, 132));
	ColorFillVideoSurfaceArea(FRAME_BUFFER, sCurrentXPosition+40, iStartY, sCurrentXPosition+41,
					sCurrentYPosition+29, RGB(8, 8, 107));

	//MORALE BAR
	iStartY = sCurrentYPosition + 29 - 27*pSoldier->bMorale/100;
	ColorFillVideoSurfaceArea(FRAME_BUFFER, sCurrentXPosition+42, iStartY, sCurrentXPosition+43,
					sCurrentYPosition+29, RGB(8, 156, 8));
	ColorFillVideoSurfaceArea(FRAME_BUFFER, sCurrentXPosition+43, iStartY, sCurrentXPosition+44,
					sCurrentYPosition+29, RGB(8, 107, 8));

	return( TRUE );
}
