#ifdef PRECOMPILEDHEADERS
	#include "Strategic All.h"
#else
	#include "sgp.h"
	#include "Game Clock.h"
	#include "Font Control.h"
	#include "render dirty.h"
	#include "Timer Control.h"
	#include "overhead.h"
	#include "environment.h"
	#include "Game Clock.h"
	#include "message.h"
	#include "worlddef.h"
	#include "rotting corpses.h"
	#include "soldier create.h"
	#include "soldier add.h"
	#include "strategic turns.h"
	#include "isometric utils.h"
	#include "animation data.h"
	#include "animation control.h"
	#include "Tactical Turns.h"
	#include "strategic turns.h"
	#include "rt time defines.h"
	#include "assignments.h"
#endif

#define	NUM_SEC_PER_STRATEGIC_TURN					( NUM_SEC_IN_MIN * 15 )	// Every fifteen minutes


UINT32		guiLastStrategicTime = 0;
UINT32		guiLastTacticalRealTime = 0;


void StrategicTurnsNewGame( )
{
	// Sync game start time
	SyncStrategicTurnTimes( );
}


void SyncStrategicTurnTimes( )
{
	guiLastStrategicTime =  GetWorldTotalSeconds( );
	guiLastTacticalRealTime =  GetJA2Clock( );
}

void HandleStrategicTurn( )
{
	UINT32	uiTime;
	UINT32	uiCheckTime;

	// OK, DO THIS CHECK EVERY ONCE AND A WHILE...
	if ( COUNTERDONE( STRATEGIC_OVERHEAD ) )
	{
		RESETCOUNTER( STRATEGIC_OVERHEAD );

		// if the game is paused, or we're in mapscreen and time is not being compressed
		if( ( GamePaused() == TRUE ) ||
				( ( guiCurrentScreen == MAP_SCREEN ) && !IsTimeBeingCompressed() ) )
		{
			// don't do any of this
			return;
		}

		//Kris -- What to do?
		if( giTimeCompressMode == NOT_USING_TIME_COMPRESSION )
		{
			SetGameTimeCompressionLevel( TIME_COMPRESS_X1 );
		}


		uiTime = GetJA2Clock( );

		// Do not handle turns update if in turnbased combat
		if ( ( gTacticalStatus.uiFlags & TURNBASED ) && ( gTacticalStatus.uiFlags & INCOMBAT ) )
		{
			guiLastTacticalRealTime = uiTime;
		}
		else
		{
			if ( giTimeCompressMode == TIME_COMPRESS_X1 || giTimeCompressMode == 0 )
			{
				uiCheckTime = NUM_REAL_SEC_PER_TACTICAL_TURN;
			}
			else
			{
				// OK, if we have compressed time...., adjust our check value to be faster....
				if( giTimeCompressSpeeds[ giTimeCompressMode ] > 0 )
				{
				  uiCheckTime = NUM_REAL_SEC_PER_TACTICAL_TURN / ( giTimeCompressSpeeds[ giTimeCompressMode ] * RT_COMPRESSION_TACTICAL_TURN_MODIFIER );
				}
			}

			if ( ( uiTime - guiLastTacticalRealTime ) > uiCheckTime )
			{
				HandleTacticalEndTurn( );

				guiLastTacticalRealTime = uiTime;
			}
		}

	}
}


void HandleStrategicTurnImplicationsOfExitingCombatMode( void )
{
	SyncStrategicTurnTimes();
	HandleTacticalEndTurn( GetWorldTotalSeconds() );
}
