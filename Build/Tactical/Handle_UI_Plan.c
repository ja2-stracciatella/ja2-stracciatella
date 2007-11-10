#include "Font_Control.h"
#include "Handle_UI_Plan.h"
#include "Overhead.h"
#include "Isometric_Utils.h"
#include "PathAI.h"
#include "Handle_UI.h"
#include "Points.h"
#include "Weapons.h"
#include "RenderWorld.h"
#include "Animation_Control.h"
#include "Message.h"
#include "Soldier_Create.h"
#include "Interface.h"


UINT8						gubNumUIPlannedMoves			= 0;
SOLDIERTYPE			*gpUIPlannedSoldier			  = NULL;
SOLDIERTYPE			*gpUIStartPlannedSoldier = NULL;
BOOLEAN					gfInUIPlanMode					  = FALSE;


BOOLEAN BeginUIPlan( SOLDIERTYPE *pSoldier )
{
	gubNumUIPlannedMoves = 0;
	gpUIPlannedSoldier				= pSoldier;
	gpUIStartPlannedSoldier		= pSoldier;
	gfInUIPlanMode			 = TRUE;

	gfPlotNewMovement    = TRUE;

	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Entering Planning Mode" );

	return( TRUE );
}


static void SelectPausedFireAnimation(SOLDIERTYPE* pSoldier);


BOOLEAN AddUIPlan( UINT16 sGridNo, UINT8 ubPlanID )
{
	INT16							sXPos, sYPos;
	INT16							sAPCost = 0;
	INT8							bDirection;
	INT32							iLoop;
	SOLDIERCREATE_STRUCT		MercCreateStruct;
	UINT8							ubNewIndex;

	// Depeding on stance and direction facing, add guy!

	// If we have a planned action here, ignore!


	// If not OK Dest, ignore!
	if ( !NewOKDestination( gpUIPlannedSoldier, sGridNo, FALSE, (INT8)gsInterfaceLevel ) )
	{
		return( FALSE );
	}


	if ( ubPlanID == UIPLAN_ACTION_MOVETO )
	{
		// Calculate cost to move here
		sAPCost = PlotPath( gpUIPlannedSoldier, sGridNo, COPYROUTE, NO_PLOT, TEMPORARY, (UINT16) gpUIPlannedSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,  gpUIPlannedSoldier->bActionPoints );
		// Adjust for running if we are not already running
		if (  gpUIPlannedSoldier->usUIMovementMode == RUNNING )
		{
			sAPCost += AP_START_RUN_COST;
		}

		if ( EnoughPoints( gpUIPlannedSoldier, sAPCost, 0, FALSE ) )
		{
			memset( &MercCreateStruct, 0, sizeof( MercCreateStruct ) );
			MercCreateStruct.bTeam				= SOLDIER_CREATE_AUTO_TEAM;
			MercCreateStruct.ubProfile		= NO_PROFILE;
			MercCreateStruct.fPlayerPlan	= TRUE;
			MercCreateStruct.bBodyType		= gpUIPlannedSoldier->ubBodyType;
			MercCreateStruct.sInsertionGridNo		= sGridNo;

			// Get Grid Corrdinates of mouse
			if ( TacticalCreateSoldier( &MercCreateStruct, &ubNewIndex ) )
			{
				SOLDIERTYPE* pPlanSoldier = GetSoldier(ubNewIndex);

				pPlanSoldier->sPlannedTargetX = -1;
				pPlanSoldier->sPlannedTargetY = -1;

				// Compare OPPLISTS!
				// Set ones we don't know about but do now back to old ( ie no new guys )
				for (iLoop = 0; iLoop < MAX_NUM_SOLDIERS; iLoop++ )
				{
					if ( gpUIPlannedSoldier->bOppList[ iLoop ] < 0 )
					{
							pPlanSoldier->bOppList[ iLoop ] = gpUIPlannedSoldier->bOppList[ iLoop ];
					}
				}

				// Get XY from Gridno
				ConvertGridNoToCenterCellXY( sGridNo, &sXPos, &sYPos );

				EVENT_SetSoldierPosition( pPlanSoldier, sXPos, sYPos );
				EVENT_SetSoldierDestination( pPlanSoldier, sGridNo );
				pPlanSoldier->bVisible = 1;
				pPlanSoldier->usUIMovementMode = gpUIPlannedSoldier->usUIMovementMode;


				pPlanSoldier->bActionPoints = gpUIPlannedSoldier->bActionPoints - sAPCost;

				pPlanSoldier->ubPlannedUIAPCost = (UINT8)pPlanSoldier->bActionPoints;

				// Get direction
				bDirection = (INT8)gpUIPlannedSoldier->usPathingData[ gpUIPlannedSoldier->usPathDataSize - 1 ];

				// Set direction
				pPlanSoldier->bDirection = bDirection;
				pPlanSoldier->bDesiredDirection = bDirection;

				// Set walking animation
				ChangeSoldierState( pPlanSoldier, pPlanSoldier->usUIMovementMode, 0, FALSE );

				// Change selected soldier
				gusSelectedSoldier = (UINT16)pPlanSoldier->ubID;

				// Change global planned mode to this guy!
				gpUIPlannedSoldier = pPlanSoldier;

				gubNumUIPlannedMoves++;

				gfPlotNewMovement    = TRUE;

				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Adding Merc Move to Plan" );

			}
		}
		else
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Merc will not have enough action points" );
		}
	}
	else if ( ubPlanID == UIPLAN_ACTION_FIRE )
	{
 	  sAPCost = CalcTotalAPsToAttack( gpUIPlannedSoldier, sGridNo, TRUE, (INT8)(gpUIPlannedSoldier->bShownAimTime /2) );

		// Get XY from Gridno
		ConvertGridNoToCenterCellXY( sGridNo, &sXPos, &sYPos );


		// If this is a player guy, show message about no APS
		if ( EnoughPoints( gpUIPlannedSoldier, sAPCost, 0, FALSE ) )
		{
			// CHECK IF WE ARE A PLANNED SOLDIER OR NOT< IF SO< CREATE!
			if ( gpUIPlannedSoldier->ubID < MAX_NUM_SOLDIERS )
			{
				memset( &MercCreateStruct, 0, sizeof( MercCreateStruct ) );
				MercCreateStruct.bTeam				= SOLDIER_CREATE_AUTO_TEAM;
				MercCreateStruct.ubProfile		= NO_PROFILE;
				MercCreateStruct.fPlayerPlan	= TRUE;
				MercCreateStruct.bBodyType		= gpUIPlannedSoldier->ubBodyType;
				MercCreateStruct.sInsertionGridNo		= sGridNo;

				// Get Grid Corrdinates of mouse
				if ( TacticalCreateSoldier( &MercCreateStruct, &ubNewIndex ) )
				{
					SOLDIERTYPE* pPlanSoldier = GetSoldier(ubNewIndex);

					pPlanSoldier->sPlannedTargetX = -1;
					pPlanSoldier->sPlannedTargetY = -1;

					// Compare OPPLISTS!
					// Set ones we don't know about but do now back to old ( ie no new guys )
					for (iLoop = 0; iLoop < MAX_NUM_SOLDIERS; iLoop++ )
					{
						if ( gpUIPlannedSoldier->bOppList[ iLoop ] < 0 )
						{
								pPlanSoldier->bOppList[ iLoop ] = gpUIPlannedSoldier->bOppList[ iLoop ];
						}
					}

					EVENT_SetSoldierPosition( pPlanSoldier, gpUIPlannedSoldier->dXPos, gpUIPlannedSoldier->dYPos );
					EVENT_SetSoldierDestination( pPlanSoldier, gpUIPlannedSoldier->sGridNo );
					pPlanSoldier->bVisible = 1;
					pPlanSoldier->usUIMovementMode = gpUIPlannedSoldier->usUIMovementMode;


					pPlanSoldier->bActionPoints = gpUIPlannedSoldier->bActionPoints - sAPCost;

					pPlanSoldier->ubPlannedUIAPCost = (UINT8)pPlanSoldier->bActionPoints;

					// Get direction
					bDirection = (INT8)gpUIPlannedSoldier->usPathingData[ gpUIPlannedSoldier->usPathDataSize - 1 ];

					// Set direction
					pPlanSoldier->bDirection = bDirection;
					pPlanSoldier->bDesiredDirection = bDirection;

					// Set walking animation
					ChangeSoldierState( pPlanSoldier, pPlanSoldier->usUIMovementMode, 0, FALSE );

					// Change selected soldier
					gusSelectedSoldier = (UINT16)pPlanSoldier->ubID;

					// Change global planned mode to this guy!
					gpUIPlannedSoldier = pPlanSoldier;

					gubNumUIPlannedMoves++;
				}


			}

			gpUIPlannedSoldier->bActionPoints = gpUIPlannedSoldier->bActionPoints - sAPCost;

			gpUIPlannedSoldier->ubPlannedUIAPCost = (UINT8)gpUIPlannedSoldier->bActionPoints;

			// Get direction from gridno
			bDirection = (INT8)GetDirectionFromGridNo( sGridNo, gpUIPlannedSoldier );

			// Set direction
			gpUIPlannedSoldier->bDirection = bDirection;
			gpUIPlannedSoldier->bDesiredDirection = bDirection;

			// Set to shooting animation
			SelectPausedFireAnimation( gpUIPlannedSoldier );

			gpUIPlannedSoldier->sPlannedTargetX = sXPos;
			gpUIPlannedSoldier->sPlannedTargetY = sYPos;

			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Adding Merc Shoot to Plan" );

		}
		else
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Merc will not have enough action points" );
		}
	}
	return( TRUE );
}


void EndUIPlan(  )
{
	int				cnt;
	SOLDIERTYPE *pSoldier;

	// Zero out any planned soldiers
	for( cnt = MAX_NUM_SOLDIERS; cnt < TOTAL_SOLDIERS; cnt++ )
	{
		pSoldier = MercPtrs[ cnt ];

		if ( pSoldier->bActive )
		{
			if ( pSoldier->sPlannedTargetX != -1 )
			{
				SetRenderFlags(RENDER_FLAG_FULL );
			}
			TacticalRemoveSoldier(pSoldier);
		}


	}
	gfInUIPlanMode			 = FALSE;
	gusSelectedSoldier   = gpUIStartPlannedSoldier->ubID;

	gfPlotNewMovement    = TRUE;

	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Leaving Planning Mode" );

}

BOOLEAN InUIPlanMode( )
{
	return( gfInUIPlanMode );
}


static void SelectPausedFireAnimation(SOLDIERTYPE* pSoldier)
{
	// Determine which animation to do...depending on stance and gun in hand...

	switch ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight )
	{
		case ANIM_STAND:

			if ( pSoldier->bDoBurst > 0 )
			{
				ChangeSoldierState( pSoldier, STANDING_BURST, 2 , FALSE );
			}
			else
			{
				ChangeSoldierState( pSoldier, SHOOT_RIFLE_STAND, 2 , FALSE );
			}
			break;

		case ANIM_PRONE:
			ChangeSoldierState( pSoldier, SHOOT_RIFLE_PRONE, 2 , FALSE );
			break;

		case ANIM_CROUCH:
			ChangeSoldierState( pSoldier, SHOOT_RIFLE_CROUCH, 2 , FALSE );
			break;

	}

}
