#ifdef PRECOMPILEDHEADERS
	#include "Tactical All.h"
#else
	#include <stdio.h>
	#include <string.h>
	#include "WCheck.h"
	#include "stdlib.h"
	#include "Debug.h"
	#include "Soldier_Control.h"
	#include "Weapons.h"
	#include "Handle_Items.h"
	#include "WorldDef.h"
	#include "WorldMan.h"
	#include "Rotting_Corpses.h"
	#include "Tile_Cache.h"
	#include "Isometric_Utils.h"
	#include "Animation_Control.h"
	#include "Utilities.h"
	#include "Game_Clock.h"
	#include "Soldier_Create.h"
	#include "RenderWorld.h"
	#include "Tile_Animation.h"
	#include "Merc_Entering.h"
	#include "Sound_Control.h"
	#include "Strategic.h"
	#include "StrategicMap.h"
	#include "Handle_UI.h"
	#include "Squads.h"
	#include "English.h"
	#include "Cursors.h"
	#include "Cursor_Control.h"
	#include "Merc_Hiring.h"
	#include "Message.h"
	#include "Text.h"
	#include "Strategic_Turns.h"
	#include "AI.h"
	#include "Dialogue_Control.h"
	#include "Music_Control.h"
#endif

#define		MAX_MERC_IN_HELI		20
#define		MAX_HELI_SCRIPT			30
#define		ME_SCRIPT_DELAY				100
#define		NUM_PER_HELI_RUN		6

enum
{
	HELI_APPROACH,
	HELI_MOVETO,
	HELI_BEGINDROP,
	HELI_DROP,
	HELI_ENDDROP,
	HELI_MOVEAWAY,
	HELI_EXIT,
	NUM_HELI_STATES

} HeliStateEnums;

enum
{
	HELI_REST,
	HELI_MOVE_DOWN,
	HELI_MOVE_UP,
	HELI_MOVESMALL_DOWN,
	HELI_MOVESMALL_UP,
	HELI_MOVEY,
	HELI_MOVELARGERY,
	HELI_HANDLE_DROP,
	HELI_SHOW_HELI,

	HELI_GOTO_BEGINDROP,
	HELI_GOTO_DROP,
	HELI_GOTO_EXIT,
	HELI_GOTO_MOVETO,
	HELI_GOTO_MOVEAWAY,
	HELI_DONE,


} HeliCodes;


UINT8			ubHeliScripts[ NUM_HELI_STATES ][ MAX_HELI_SCRIPT ] =
{
	// HELI_APPROACH
	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,

	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,

	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,

	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,

	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,

	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_GOTO_MOVETO,

	// MOVE TO
	HELI_SHOW_HELI,
	HELI_MOVEY,
	HELI_MOVEY,
	HELI_MOVEY,
	HELI_MOVEY,

	HELI_MOVEY,
	HELI_MOVEY,
	HELI_MOVEY,
	HELI_MOVEY,
	HELI_MOVEY,

	HELI_MOVEY,
	HELI_MOVEY,
	HELI_MOVEY,
	HELI_MOVEY,
	HELI_MOVEY,

	HELI_MOVEY,
	HELI_MOVEY,
	HELI_MOVEY,
	HELI_MOVEY,
	HELI_MOVEY,

	HELI_MOVEY,
	HELI_MOVEY,
	HELI_MOVEY,
	HELI_MOVEY,
	HELI_MOVEY,

	HELI_MOVEY,
	HELI_MOVEY,
	HELI_MOVEY,
	HELI_MOVEY,
	HELI_GOTO_BEGINDROP,

	// HELI_BEGIN_DROP
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,

	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,

	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,

	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,

	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,

	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_MOVE_DOWN,
	HELI_GOTO_DROP,

	// Heli Begin Drop
	HELI_MOVESMALL_UP,
	HELI_MOVESMALL_UP,
	HELI_MOVESMALL_UP,
	HELI_MOVESMALL_UP,
	HELI_MOVESMALL_UP,

	HELI_MOVESMALL_DOWN,
	HELI_MOVESMALL_DOWN,
	HELI_MOVESMALL_DOWN,
	HELI_MOVESMALL_DOWN,
	HELI_MOVESMALL_DOWN,

	HELI_MOVESMALL_UP,
	HELI_MOVESMALL_UP,
	HELI_MOVESMALL_UP,
	HELI_MOVESMALL_UP,
	HELI_MOVESMALL_UP,

	HELI_MOVESMALL_DOWN,
	HELI_MOVESMALL_DOWN,
	HELI_MOVESMALL_DOWN,
	HELI_MOVESMALL_DOWN,
	HELI_MOVESMALL_DOWN,

	HELI_MOVESMALL_UP,
	HELI_MOVESMALL_UP,
	HELI_MOVESMALL_UP,
	HELI_MOVESMALL_UP,
	HELI_MOVESMALL_UP,

	HELI_MOVESMALL_DOWN,
	HELI_MOVESMALL_DOWN,
	HELI_MOVESMALL_DOWN,
	HELI_MOVESMALL_DOWN,
	HELI_GOTO_DROP,

	// HELI END DROP
	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_MOVE_UP,

	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_MOVE_UP,

	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_MOVE_UP,

	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_MOVE_UP,

	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_MOVE_UP,

	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_MOVE_UP,
	HELI_GOTO_MOVEAWAY,

	// MOVE AWAY
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,

	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,

	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,

	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,

	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,

	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_MOVELARGERY,
	HELI_GOTO_EXIT,

	// HELI EXIT
	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,

	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,

	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,

	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,

	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,

	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_REST,
	HELI_DONE,
};


BOOLEAN		gfHandleHeli = FALSE;
UINT8			gusHeliSeats[ MAX_MERC_IN_HELI ];
INT8			gbNumHeliSeatsOccupied = 0;

BOOLEAN		gfFirstGuyDown = FALSE;

UINT32		uiSoundSample;
INT16			gsGridNoSweetSpot;
INT16			gsHeliXPos;
INT16			gsHeliYPos;
FLOAT			gdHeliZPos;
INT16			gsHeliScript;
UINT8			gubHeliState;
UINT32		guiHeliLastUpdate;
INT8			gbCurDrop;
INT8			gbExitCount;
INT8			gbHeliRound;

BOOLEAN		fFadingHeliIn = FALSE;
BOOLEAN		fFadingHeliOut = FALSE;

BOOLEAN		gfIngagedInDrop = FALSE;

ANITILE		*gpHeli;
BOOLEAN		gfFirstHeliRun;


void HandleFirstHeliDropOfGame( );





void ResetHeliSeats( )
{
	gbNumHeliSeatsOccupied = 0;
}

void AddMercToHeli( UINT8 ubID )
{
	INT32 cnt;

	if ( gbNumHeliSeatsOccupied < MAX_MERC_IN_HELI )
	{
		// Check if it already exists!
		for ( cnt = 0; cnt < gbNumHeliSeatsOccupied; cnt++ )
		{
			if ( gusHeliSeats[ cnt ] == ubID )
			{
				return;
			}
		}

		gusHeliSeats[ gbNumHeliSeatsOccupied ] = ubID;
		gbNumHeliSeatsOccupied++;
	}
}


void StartHelicopterRun( INT16 sGridNoSweetSpot )
{
	INT16 sX, sY;

	gsGridNoSweetSpot = sGridNoSweetSpot;

	if ( gbNumHeliSeatsOccupied == 0 )
	{
		return;
	}

	InterruptTime();
	PauseGame();
	LockPauseState( 20 );

	ConvertGridNoToCenterCellXY( sGridNoSweetSpot, &sX, &sY );

	gsHeliXPos					= sX - ( 2 * CELL_X_SIZE );
	gsHeliYPos					= sY - ( 10 * CELL_Y_SIZE );
	//gsHeliXPos					= sX - ( 3 * CELL_X_SIZE );
	//gsHeliYPos					= sY + ( 4 * CELL_Y_SIZE );
	gdHeliZPos					= 0;
	gsHeliScript				= 0;
	gbCurDrop						= 0;
	gbExitCount					= 0;
	gbHeliRound					= 1;

	gubHeliState				= HELI_APPROACH;
	guiHeliLastUpdate		= GetJA2Clock( );

	// Start sound
	uiSoundSample = PlayJA2Sample( HELI_1, RATE_11025, 0, 10000, MIDDLEPAN );
	fFadingHeliIn				= TRUE;

	gfHandleHeli			= TRUE;

	gfFirstGuyDown		= TRUE;

	guiPendingOverrideEvent = LU_BEGINUILOCK;
}


void HandleHeliDrop( )
{
	UINT8 ubScriptCode;
	UINT32	uiClock;
	//INT16 sWorldX, sWorldY;
	INT32 iVol;
	INT32 cnt;
	ANITILE_PARAMS	AniParams;


	if ( gfHandleHeli )
	{
		if ( gCurrentUIMode != LOCKUI_MODE )
		{
			guiPendingOverrideEvent = LU_BEGINUILOCK;
		}

		if ( _KeyDown( ESC ) )
		{
			// Loop through all mercs not yet placed
			for ( cnt = gbCurDrop; cnt < gbNumHeliSeatsOccupied; cnt++ )
			{
				// Add merc to sector
				MercPtrs[ gusHeliSeats[ cnt ] ]->ubStrategicInsertionCode = INSERTION_CODE_NORTH;
				UpdateMercInSector( MercPtrs[ gusHeliSeats[ cnt ] ], 9, 1, 0 );

				// Check for merc arrives quotes...
				HandleMercArrivesQuotes( MercPtrs[ gusHeliSeats[ cnt ] ] );

				ScreenMsg( FONT_MCOLOR_WHITE, MSG_INTERFACE, TacticalStr[ MERC_HAS_ARRIVED_STR ], MercPtrs[ gusHeliSeats[ cnt ] ]->name );

			}

			// Remove heli
			DeleteAniTile( gpHeli );

      RebuildCurrentSquad( );

			// Remove sound
			if( uiSoundSample!=NO_SAMPLE )
			{
				SoundStop( uiSoundSample );
			}

			gfHandleHeli = FALSE;
			gfIgnoreScrolling = FALSE;
			gbNumHeliSeatsOccupied = 0;
			UnLockPauseState();
			UnPauseGame();


			// Select our first guy
			SelectSoldier( gusHeliSeats[ 0 ], FALSE, TRUE );

			//guiCurrentEvent = LU_ENDUILOCK;
			//gCurrentUIMode  = LOCKUI_MODE;
			guiPendingOverrideEvent = LU_ENDUILOCK;
			//UIHandleLUIEndLock( NULL );

			HandleFirstHeliDropOfGame( );
			return;

		}

		gfIgnoreScrolling = TRUE;

		uiClock = GetJA2Clock( );

		if ( ( uiClock - guiHeliLastUpdate ) >  ME_SCRIPT_DELAY )
		{
			guiHeliLastUpdate = uiClock;

			if(fFadingHeliIn)
			{
				if( uiSoundSample!=NO_SAMPLE )
				{
					iVol=SoundGetVolume( uiSoundSample );
					iVol=__min( HIGHVOLUME, iVol+5);
					SoundSetVolume(uiSoundSample, iVol);
					if(iVol==HIGHVOLUME)
							fFadingHeliIn=FALSE;
				}
				else
				{
					fFadingHeliIn=FALSE;
				}
			}
			else if(fFadingHeliOut)
			{
				if( uiSoundSample!=NO_SAMPLE )
				{
					iVol=SoundGetVolume(uiSoundSample);

					iVol=__max( 0, iVol-5);

					SoundSetVolume(uiSoundSample, iVol);
					if(iVol==0)
					{
						// Stop sound
						SoundStop( uiSoundSample );
						fFadingHeliOut=FALSE;
						gfHandleHeli = FALSE;
						gfIgnoreScrolling = FALSE;
						gbNumHeliSeatsOccupied = 0;
						guiPendingOverrideEvent = LU_ENDUILOCK;
						UnLockPauseState();
						UnPauseGame();

            RebuildCurrentSquad( );

						HandleFirstHeliDropOfGame( );
					}
				}
				else
				{
					fFadingHeliOut=FALSE;
					gfHandleHeli = FALSE;
					gfIgnoreScrolling = FALSE;
					gbNumHeliSeatsOccupied = 0;
					guiPendingOverrideEvent = LU_ENDUILOCK;
					UnLockPauseState();
					UnPauseGame();

          RebuildCurrentSquad( );

					HandleFirstHeliDropOfGame( );
				}
			}

			if ( gsHeliScript == MAX_HELI_SCRIPT )
			{
				return;
			}

			ubScriptCode = ubHeliScripts[ gubHeliState ][ gsHeliScript ];

			// Switch on mode...
			if ( gubHeliState == HELI_DROP )
			{
				if ( !gfIngagedInDrop )
				{
					INT8 bEndVal;

					bEndVal = ( gbHeliRound * NUM_PER_HELI_RUN );

					if ( bEndVal > gbNumHeliSeatsOccupied )
					{
						bEndVal = gbNumHeliSeatsOccupied;
					}

					// OK, Check if we have anybody left to send!
					if ( gbCurDrop < bEndVal )
					{
						//sWorldX = CenterX( gsGridNoSweetSpot );
						//sWorldY = CenterY( gsGridNoSweetSpot );
						EVENT_InitNewSoldierAnim( MercPtrs[ gusHeliSeats[ gbCurDrop ] ], HELIDROP, 0 , FALSE );

						// Change insertion code
						MercPtrs[ gusHeliSeats[ gbCurDrop ] ]->ubStrategicInsertionCode = INSERTION_CODE_NORTH;

						UpdateMercInSector( MercPtrs[ gusHeliSeats[ gbCurDrop ] ], 9, 1, 0 );
						//EVENT_SetSoldierPosition( MercPtrs[ gusHeliSeats[ gbCurDrop ] ], sWorldX, sWorldY );

						// IF the first guy down, set squad!
						if ( gfFirstGuyDown )
						{
							gfFirstGuyDown = FALSE;
							SetCurrentSquad( MercPtrs[ gusHeliSeats[ gbCurDrop ] ]->bAssignment, TRUE );
						}
						ScreenMsg( FONT_MCOLOR_WHITE, MSG_INTERFACE, TacticalStr[ MERC_HAS_ARRIVED_STR ], MercPtrs[ gusHeliSeats[ gbCurDrop ] ]->name );

						gbCurDrop++;

						gfIngagedInDrop = TRUE;
					}
					else
					{
						if( gbExitCount == 0 )
						{
							gbExitCount = 2;
						}
						else
						{
							gbExitCount--;

							if ( gbExitCount == 1 )
							{
								// Goto leave
								gsHeliScript				= -1;
								gubHeliState				= HELI_ENDDROP;

							}
						}
					}
				}
			}

			switch( ubScriptCode )
			{
				case HELI_REST:

					break;

				case HELI_MOVE_DOWN:

					gdHeliZPos -= 1;
					gpHeli->pLevelNode->sRelativeZ = ( INT16 )gdHeliZPos;
					break;

				case HELI_MOVE_UP:

					gdHeliZPos += 1;
					gpHeli->pLevelNode->sRelativeZ = ( INT16 )gdHeliZPos;
					break;

				case HELI_MOVESMALL_DOWN:

					gdHeliZPos -= 0.25;
					gpHeli->pLevelNode->sRelativeZ = ( INT16 )gdHeliZPos;
					break;

				case HELI_MOVESMALL_UP:

					gdHeliZPos += 0.25;
					gpHeli->pLevelNode->sRelativeZ = ( INT16 )gdHeliZPos;
					break;


				case HELI_MOVEY:

					gpHeli->sRelativeY += 4;
					break;

				case HELI_MOVELARGERY:

					gpHeli->sRelativeY += 6;
					break;

				case HELI_GOTO_BEGINDROP:

					gsHeliScript				= -1;
					gubHeliState				= HELI_BEGINDROP;
					break;

				case HELI_SHOW_HELI:

					// Start animation
					memset( &AniParams, 0, sizeof( ANITILE_PARAMS ) );
					AniParams.sGridNo							= gsGridNoSweetSpot;
					AniParams.ubLevelID						= ANI_SHADOW_LEVEL;
					AniParams.sDelay							= 90;
					AniParams.sStartFrame					= 0;
					AniParams.uiFlags							= ANITILE_CACHEDTILE | ANITILE_FORWARD | ANITILE_LOOPING;
					AniParams.sX									= gsHeliXPos;
					AniParams.sY									= gsHeliYPos;
					AniParams.sZ									= (INT16)gdHeliZPos;
					strcpy( AniParams.zCachedFile, "TILECACHE\\HELI_SH.STI" );

					gpHeli = CreateAnimationTile( &AniParams );
					break;

				case HELI_GOTO_DROP:

					// Goto drop animation
					gdHeliZPos -= 0.25;
					gpHeli->pLevelNode->sRelativeZ = ( INT16 )gdHeliZPos;
					gsHeliScript				= -1;
					gubHeliState				= HELI_DROP;
					break;

				case HELI_GOTO_MOVETO:

					// Goto drop animation
					gsHeliScript				= -1;
					gubHeliState				= HELI_MOVETO;
					break;

				case HELI_GOTO_MOVEAWAY:

					// Goto drop animation
					gsHeliScript				= -1;
					gubHeliState				= HELI_MOVEAWAY;
					break;

				case HELI_GOTO_EXIT:

					if ( gbCurDrop < gbNumHeliSeatsOccupied )
					{
						// Start another run......
						INT16 sX, sY;

						ConvertGridNoToCenterCellXY( gsGridNoSweetSpot, &sX, &sY );

						gsHeliXPos					= sX - ( 2 * CELL_X_SIZE );
						gsHeliYPos					= sY - ( 10 * CELL_Y_SIZE );
						gdHeliZPos					= 0;
						gsHeliScript				= 0;
						gbExitCount					= 0;
						gubHeliState				= HELI_APPROACH;
						gbHeliRound++;

						// Ahh, but still delete the heli!
						DeleteAniTile( gpHeli );
						gpHeli = NULL;
					}
					else
					{
						// Goto drop animation
						gsHeliScript				= -1;
						gubHeliState				= HELI_EXIT;

						// Delete helicopter image!
						DeleteAniTile( gpHeli );
						gpHeli = NULL;
						gfIgnoreScrolling = FALSE;

						// Select our first guy
						SelectSoldier( gusHeliSeats[ 0 ], FALSE, TRUE );
					}
					break;

				case HELI_DONE:

					// End
					fFadingHeliOut			= TRUE;
					break;
			}

			gsHeliScript++;

		}
	}
}


void BeginMercEntering( SOLDIERTYPE *pSoldier, INT16 sGridNo )
{
	ResetHeliSeats( );

	AddMercToHeli( pSoldier->ubID );


	StartHelicopterRun( sGridNo );

	// Make sure AI does nothing.....
	PauseAIUntilManuallyUnpaused();
}


void HandleFirstHeliDropOfGame( )
{
	// Are we in the first heli drop?
	if ( gfFirstHeliRun )
	{
		SyncStrategicTurnTimes();

		// Call people to area
		CallAvailableEnemiesTo( gsGridNoSweetSpot );

		// Say quote.....
		SayQuoteFromAnyBodyInSector( QUOTE_ENEMY_PRESENCE );

		// Start music
		SetMusicMode( MUSIC_TACTICAL_ENEMYPRESENT );

		gfFirstHeliRun = FALSE;

	}

	// Send message to turn on ai again....
	CharacterDialogueWithSpecialEvent( 0, 0,   0 , DIALOGUE_TACTICAL_UI , FALSE , FALSE , DIALOGUE_SPECIAL_EVENT_ENABLE_AI ,0, 0 );

}


