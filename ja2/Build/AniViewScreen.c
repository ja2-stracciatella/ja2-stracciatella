#ifdef PRECOMPILEDHEADERS
	#include "JA2 All.h"
#else
	#include <stdio.h>
	#include <stdarg.h>
	#include <time.h>
	#include "sgp.h"
	#include "gameloop.h"
	#include "himage.h"
	#include "vobject.h"
	#include "vobject_private.h"
	#include "vobject_blitters.h"
	#include "wcheck.h"
	#include "worlddef.h"
	#include "renderworld.h"
	#include "input.h"
	#include "font.h"
	#include "screenids.h"
	#include "container.h"
	#include "overhead.h"
	#include "Isometric Utils.h"
	#include "sysutil.h"
	#include "Font Control.h"
	#include "Radar Screen.h"
	#include "Soldier Control.h"
	#include "Animation Control.h"
	#include "Animation Data.h"
	#include "Event Pump.h"
	#include "Timer Control.h"
	#include "Render Dirty.h"
	#include "Sys Globals.h"
	#include "interface.h"
	#include "soldier ani.h"
	#include <wchar.h>
	#include <tchar.h>
	#include "english.h"
	#include "Fileman.h"
	#include "messageboxscreen.h"
#endif


void BuildListFile( );


BOOLEAN	gfAniEditMode = FALSE;
static UINT16		usStartAnim = 0;
static UINT8		ubStartHeight = 0;
static SOLDIERTYPE *pSoldier;

static BOOLEAN fOKFiles = FALSE;
static UINT8	 ubNumStates = 0;
static UINT16   *pusStates = NULL;
static INT8   ubCurLoadedState = 0;

void CycleAnimations( )
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


UINT32 AniEditScreenInit(void)
{

  return TRUE;
}

// The ShutdownGame function will free up/undo all things that were started in InitializeGame()
// It will also be responsible to making sure that all Gaming Engine tasks exit properly

UINT32 AniEditScreenShutdown(void)
{

	return TRUE;
}



UINT32  AniEditScreenHandle(void)
{
  InputAtom					InputEvent;
	static BOOLEAN		fFirstTime = TRUE;
	static UINT16			usOldState;
	static BOOLEAN		fToggle = FALSE;
	static BOOLEAN		fToggle2 = FALSE;

//	EV_S_SETPOSITION SSetPosition;

	// Make backups
	if ( fFirstTime )
	{
		gfAniEditMode = TRUE;

		usStartAnim   = 0;
		ubStartHeight = ANIM_STAND;

		fFirstTime = FALSE;
		fToggle		 = FALSE;
		fToggle2   = FALSE;
		ubCurLoadedState = 0;

		pSoldier = MercPtrs[ gusSelectedSoldier ];

		gTacticalStatus.uiFlags |= LOADING_SAVED_GAME;

		EVENT_InitNewSoldierAnim( pSoldier, usStartAnim, 0 , TRUE );

		BuildListFile( );

	}



	/////////////////////////////////////////////////////
	StartFrameBufferRender( );

	RenderWorld( );

	ExecuteBaseDirtyRectQueue( );


	/////////////////////////////////////////////////////
	EndFrameBufferRender( );


	SetFont( LARGEFONT1 );
	mprintf( 0,0,L"SOLDIER ANIMATION VIEWER" );
	gprintfdirty( 0,0,L"SOLDIER ANIMATION VIEWER" );


	mprintf( 0,20,L"Current Animation: %S %S", gAnimControl[ usStartAnim ].zAnimStr, gAnimSurfaceDatabase[ pSoldier->usAnimSurface ].Filename );
	gprintfdirty( 0,20,L"Current Animation: %S %S", gAnimControl[ usStartAnim ].zAnimStr, gAnimSurfaceDatabase[ pSoldier->usAnimSurface ].Filename );


	switch( ubStartHeight )
	{
		case ANIM_STAND:

			mprintf( 0,40,L"Current Stance: STAND" );
			break;

		case ANIM_CROUCH:

			mprintf( 0,40,L"Current Stance: CROUCH" );
			break;

		case ANIM_PRONE:

			mprintf( 0,40,L"Current Stance: PRONE" );
			break;
	}
	gprintfdirty( 0,40,L"Current Animation: %S", gAnimControl[ usStartAnim ].zAnimStr );


	if ( fToggle )
	{
		mprintf( 0,60,L"FORCE ON" );
		gprintfdirty( 0,60,L"FORCE OFF" );
	}

	if ( fToggle2 )
	{
		mprintf( 0,70,L"LOADED ORDER ON" );
		gprintfdirty( 0,70,L"LOADED ORDER ON" );

		mprintf( 0,90,L"LOADED ORDER : %S", gAnimControl[ pusStates[ ubCurLoadedState ] ].zAnimStr );
		gprintfdirty( 0,90,L"LOADED ORDER : %S", gAnimControl[ pusStates[ ubCurLoadedState ] ].zAnimStr );

	}

  if (DequeueEvent(&InputEvent) == TRUE)
  {
    if ((InputEvent.usEvent == KEY_DOWN)&&(InputEvent.usParam == ESC))
    {
			 fFirstTime = TRUE;

			 gfAniEditMode = FALSE;

	  	 fFirstTimeInGameScreen = TRUE;

			 gTacticalStatus.uiFlags &= (~LOADING_SAVED_GAME);

			 if ( fOKFiles )
			 {
					 MemFree( pusStates );
			 }

			 fOKFiles = FALSE;

			 return( GAME_SCREEN );
    }

		if ((InputEvent.usEvent == KEY_UP) && (InputEvent.usParam == SPACE ))
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


		if ((InputEvent.usEvent == KEY_UP) && (InputEvent.usParam == PGUP ))
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


		if ((InputEvent.usEvent == KEY_UP) && (InputEvent.usParam == PGDN ))
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

		if ((InputEvent.usEvent == KEY_UP) && (InputEvent.usParam == ENTER ))
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


UINT16 GetAnimStateFromName( INT8 *zName )
{
	INT32 cnt;

	// FInd the next animation with start height the same...
	for ( cnt = 0; cnt < NUMANIMATIONSTATES; cnt++ )
	{
		if ( stricmp( gAnimControl[ cnt ].zAnimStr, zName ) == 0 )
		{
			return( (UINT16) cnt );
		}
	}

	return( 5555 );
}


void BuildListFile( )
{
	FILE *infoFile;
	char currFilename[128];
	int numEntries = 0;
	int	cnt;
	UINT16 usState;
	INT16 zError[128];


	//Verify the existance of the header text file.
	infoFile = fopen( "ANITEST.DAT", "rb");
	if(!infoFile)
	{
		return;
	}
	//count STIs inside header and verify each one's existance.
	while( !feof( infoFile ) )
	{
		fgets( currFilename, 128, infoFile );
		//valid entry in header, continue on...

		numEntries++;
	}
	fseek( infoFile, 0, SEEK_SET ); //reset header file

	// Allocate array
	pusStates = MemAlloc( sizeof( UINT16 ) * numEntries );

	fOKFiles = TRUE;

	cnt = 0;
	while( !feof( infoFile ) )
	{
		fgets( currFilename, 128, infoFile );

		// Remove newline
		currFilename[ strlen( currFilename ) -1 ] = '\0';
		currFilename[ strlen( currFilename ) -1 ] = '\0';

		usState = GetAnimStateFromName( currFilename );

		if ( usState != 5555 )
		{
			cnt++;
			ubNumStates	= (UINT8)cnt;
			pusStates[ cnt ] = usState;
		}
		else
		{
			swprintf( zError, L"Animation str %S is not known: ", currFilename );
			DoMessageBox( MSG_BOX_BASIC_STYLE, zError, ANIEDIT_SCREEN, ( UINT8 )MSG_BOX_FLAG_YESNO, NULL, NULL );
			fclose( infoFile );
			return;
		}
	}

}
