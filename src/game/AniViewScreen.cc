#include "AniViewScreen.h"
#include "WorldDef.h"
#include "RenderWorld.h"
#include "Input.h"
#include "Font.h"
#include "Overhead.h"
#include "SysUtil.h"
#include "Font_Control.h"
#include "Radar_Screen.h"
#include "Soldier_Control.h"
#include "Animation_Control.h"
#include "Animation_Data.h"
#include "Render_Dirty.h"
#include "Sys_Globals.h"
#include "Soldier_Ani.h"
#include "English.h"
#include "MessageBoxScreen.h"
#include "Video.h"
#include "MemMan.h"

#include <string_theory/format>

#include <sstream>


BOOLEAN	gfAniEditMode = FALSE;
static UINT16		usStartAnim = 0;
static UINT8		ubStartHeight = 0;
static SOLDIERTYPE *pSoldier;

static BOOLEAN fOKFiles = FALSE;
static UINT8   ubNumStates = 0;
static UINT16  *pusStates = NULL;
static INT8    ubCurLoadedState = 0;


static void CycleAnimations(void)
{
	INT32 cnt;

	// FInd the next animation with start height the same...
	for ( cnt = usStartAnim + 1; cnt < NUMANIMATIONSTATES; cnt++ )
	{
		if ( gAnimControl[ cnt ].ubHeight == ubStartHeight )
		{
			usStartAnim = ( UINT8) cnt;
			EVENT_InitNewSoldierAnim( pSoldier, usStartAnim, 0 , TRUE );
			return;
		}
	}

	usStartAnim = 0;
	EVENT_InitNewSoldierAnim( pSoldier, usStartAnim, 0 , TRUE );
}


static void BuildListFile(void);


ScreenID AniEditScreenHandle(void)
{
	InputAtom		InputEvent;
	static BOOLEAN		fFirstTime = TRUE;
	static UINT16		usOldState;
	static BOOLEAN		fToggle = FALSE;
	static BOOLEAN		fToggle2 = FALSE;

	// Make backups
	if ( fFirstTime )
	{
		gfAniEditMode = TRUE;

		usStartAnim   = 0;
		ubStartHeight = ANIM_STAND;

		fFirstTime = FALSE;
		fToggle    = FALSE;
		fToggle2   = FALSE;
		ubCurLoadedState = 0;

		pSoldier = GetSelectedMan();

		gTacticalStatus.uiFlags |= LOADING_SAVED_GAME;

		EVENT_InitNewSoldierAnim( pSoldier, usStartAnim, 0 , TRUE );

		BuildListFile( );

	}

	RenderWorld( );
	ExecuteBaseDirtyRectQueue( );
	EndFrameBufferRender( );

	SetFont( LARGEFONT1 );
	GDirtyPrint( 0,  0, "SOLDIER ANIMATION VIEWER");
	GDirtyPrint(0, 20, ST::format("Current Animation: {} {}", gAnimControl[usStartAnim].zAnimStr, gAnimSurfaceDatabase[pSoldier->usAnimSurface].Filename));

	switch( ubStartHeight )
	{
		case ANIM_STAND:  GDirtyPrint(0, 40, "Current Stance: STAND");  break;
		case ANIM_CROUCH: GDirtyPrint(0, 40, "Current Stance: CROUCH"); break;
		case ANIM_PRONE:  GDirtyPrint(0, 40, "Current Stance: PRONE");  break;
	}

	if ( fToggle )
	{
		GDirtyPrint(0, 60, "FORCE ON");
	}

	if ( fToggle2 )
	{
		GDirtyPrint( 0, 70, "LOADED ORDER ON");
		GDirtyPrint(0, 90, ST::format("LOADED ORDER : {}", gAnimControl[pusStates[ubCurLoadedState]].zAnimStr));
	}

	if (DequeueEvent(&InputEvent))
	{
		if (InputEvent.usEvent == KEY_DOWN && InputEvent.usParam == SDLK_ESCAPE)
		{
			fFirstTime = TRUE;

			gfAniEditMode = FALSE;

			fFirstTimeInGameScreen = TRUE;

			gTacticalStatus.uiFlags &= (~LOADING_SAVED_GAME);

			if ( fOKFiles )
			{
				delete[] pusStates;
			}

				fOKFiles = FALSE;

				return( GAME_SCREEN );
		}

		if (InputEvent.usEvent == KEY_UP && InputEvent.usParam == SDLK_SPACE)
		{
			if ( !fToggle && !fToggle2 )
			{
				CycleAnimations( );
			}
		}

		if ((InputEvent.usEvent == KEY_UP) && (InputEvent.usParam == 's' ))
		{
			if ( !fToggle )
			{
				UINT16 usAnim=0;
				usOldState = usStartAnim;

				switch( ubStartHeight )
				{
					case ANIM_STAND:
						usAnim = STANDING;
						break;

					case ANIM_CROUCH:
						usAnim = CROUCHING;
						break;

					case ANIM_PRONE:
						usAnim = PRONE;
						break;
				}

				EVENT_InitNewSoldierAnim( pSoldier, usAnim, 0 , TRUE );
			}
			else
			{
				EVENT_InitNewSoldierAnim( pSoldier, usOldState, 0 , TRUE );
			}

			fToggle = !fToggle;
		}

		if ((InputEvent.usEvent == KEY_UP) && (InputEvent.usParam == 'l' ))
		{
			if ( !fToggle2 )
			{
				usOldState = usStartAnim;

				EVENT_InitNewSoldierAnim( pSoldier, pusStates[ ubCurLoadedState ], 0 , TRUE );
			}
			else
			{
				EVENT_InitNewSoldierAnim( pSoldier, usOldState, 0 , TRUE );
			}

			fToggle2 = !fToggle2;
		}

		if (InputEvent.usEvent == KEY_UP && InputEvent.usParam == SDLK_PAGEUP)
		{
			if ( fOKFiles && fToggle2 )
			{
				ubCurLoadedState++;

				if ( ubCurLoadedState == ubNumStates )
				{
					ubCurLoadedState = 0;
				}

				EVENT_InitNewSoldierAnim( pSoldier, pusStates[ ubCurLoadedState ], 0 , TRUE );

			}
		}

		if (InputEvent.usEvent == KEY_UP && InputEvent.usParam == SDLK_PAGEDOWN)
		{
			if ( fOKFiles && fToggle2 )
			{
				ubCurLoadedState--;

				if ( ubCurLoadedState == 0 )
				{
					ubCurLoadedState = ubNumStates;
				}

				EVENT_InitNewSoldierAnim( pSoldier, pusStates[ ubCurLoadedState ], 0 , TRUE );
			}
		}

		if ((InputEvent.usEvent == KEY_UP) && (InputEvent.usParam == 'c' ))
		{
			// CLEAR!
			usStartAnim = 0;
			EVENT_InitNewSoldierAnim( pSoldier, usStartAnim, 0 , TRUE );
		}

		if (InputEvent.usEvent == KEY_UP && InputEvent.usParam == SDLK_RETURN)
		{
			if ( ubStartHeight == ANIM_STAND )
			{
				ubStartHeight = ANIM_CROUCH;
			}
			else if ( ubStartHeight == ANIM_CROUCH )
			{
				ubStartHeight = ANIM_PRONE;
			}
			else
			{
				ubStartHeight = ANIM_STAND;
			}
		}

	}


	return( ANIEDIT_SCREEN );

}


static UINT16 GetAnimStateFromName(const char* zName)
{
	INT32 cnt;

	// FInd the next animation with start height the same...
	for ( cnt = 0; cnt < NUMANIMATIONSTATES; cnt++ )
	{
		if (strcasecmp(gAnimControl[cnt].zAnimStr, zName) == 0) return cnt;
	}

	return( 5555 );
}


static void BuildListFile(void)
{
	char currFilename[128];
	int numEntries = 0;
	int	cnt;
	UINT16 usState;
	ST::string zError;

	RustPointer<VecU8> vec(Fs_read("anitest.dat"));
	if (!vec)
	{
		RustPointer<char> err(getRustError());
		SLOGE("BuildListFile: %s", err.get());
		return;
	}
	ST::string data(reinterpret_cast<const char*>(VecU8_as_ptr(vec.get())), VecU8_len(vec.get()));
	vec.reset(nullptr);

	//count STIs inside header and verify each one's existance.
	std::stringstream ss(data.c_str());
	while (ss.getline(currFilename, 128))
	{
		numEntries++;
	}

	// Allocate array
	pusStates = new UINT16[numEntries]{};

	fOKFiles = TRUE;

	cnt = 0;
	ss.str(data.c_str());
	while (ss.getline(currFilename, 128))
	{
		// Remove newline
		currFilename[ strcspn( currFilename, "\r\n" ) ] = '\0';

		usState = GetAnimStateFromName( currFilename );

		if ( usState != 5555 )
		{
			cnt++;
			ubNumStates	= (UINT8)cnt;
			pusStates[ cnt ] = usState;
		}
		else
		{
			zError = ST::format("Animation str {} is not known: ", currFilename);
			DoMessageBox(MSG_BOX_BASIC_STYLE, zError, ANIEDIT_SCREEN, MSG_BOX_FLAG_YESNO, NULL, NULL);
			return;
		}
	}
}
