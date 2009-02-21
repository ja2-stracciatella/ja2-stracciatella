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


void BeginUIPlan(SOLDIERTYPE* const pSoldier)
{
	gubNumUIPlannedMoves = 0;
	gpUIPlannedSoldier				= pSoldier;
	gpUIStartPlannedSoldier		= pSoldier;
	gfInUIPlanMode			 = TRUE;

	gfPlotNewMovement    = TRUE;

	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Entering Planning Mode" );
}


static void SelectPausedFireAnimation(SOLDIERTYPE* pSoldier);


BOOLEAN AddUIPlan( UINT16 sGridNo, UINT8 ubPlanID )
{
	INT16							sAPCost = 0;
	INT8							bDirection;
	INT32							iLoop;
	SOLDIERCREATE_STRUCT		MercCreateStruct;

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
		sAPCost = PlotPath(gpUIPlannedSoldier, sGridNo, COPYROUTE, NO_PLOT, gpUIPlannedSoldier->usUIMovementMode, gpUIPlannedSoldier->bActionPoints);
		// Adjust for running if we are not already running
		if (  gpUIPlannedSoldier->usUIMovementMode == RUNNING )
		{
			sAPCost += AP_START_RUN_COST;
		}

		if ( EnoughPoints( gpUIPlannedSoldier, sAPCost, 0, FALSE ) )
		{
			memset( &MercCreateStruct, 0, sizeof( MercCreateStruct ) );
			MercCreateStruct.bTeam				= PLAYER_PLAN;
			MercCreateStruct.ubProfile		= NO_PROFILE;
			MercCreateStruct.bBodyType		= gpUIPlannedSoldier->ubBodyType;
			MercCreateStruct.sInsertionGridNo		= sGridNo;

			// Get Grid Corrdinates of mouse
			SOLDIERTYPE* const pPlanSoldier = TacticalCreateSoldier(MercCreateStruct);
			if (pPlanSoldier != NULL)
			{
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

				EVENT_SetSoldierPosition(pPlanSoldier, sGridNo, SSP_NONE);
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
				SetSelectedMan(pPlanSoldier);

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
		INT16 sXPos;
		INT16 sYPos;
		ConvertGridNoToCenterCellXY( sGridNo, &sXPos, &sYPos );

		// If this is a player guy, show message about no APS
		if ( EnoughPoints( gpUIPlannedSoldier, sAPCost, 0, FALSE ) )
		{
			// CHECK IF WE ARE A PLANNED SOLDIER OR NOT< IF SO< CREATE!
			if ( gpUIPlannedSoldier->ubID < MAX_NUM_SOLDIERS )
			{
				memset( &MercCreateStruct, 0, sizeof( MercCreateStruct ) );
				MercCreateStruct.bTeam				= PLAYER_PLAN;
				MercCreateStruct.ubProfile		= NO_PROFILE;
				MercCreateStruct.bBodyType		= gpUIPlannedSoldier->ubBodyType;
				MercCreateStruct.sInsertionGridNo		= sGridNo;

				// Get Grid Corrdinates of mouse
				SOLDIERTYPE* const pPlanSoldier = TacticalCreateSoldier(MercCreateStruct);
				if (pPlanSoldier != NULL)
				{
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

					EVENT_SetSoldierPositionXY(pPlanSoldier, gpUIPlannedSoldier->dXPos, gpUIPlannedSoldier->dYPos, SSP_NONE);
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
					SetSelectedMan(pPlanSoldier);

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
	// Zero out any planned soldiers
	FOR_ALL_PLANNING_SOLDIERS(s)
	{
		if (s->sPlannedTargetX != -1)
		{
			SetRenderFlags(RENDER_FLAG_FULL);
		}
		TacticalRemoveSoldier(s);
	}
	gfInUIPlanMode			 = FALSE;
	SetSelectedMan(gpUIStartPlannedSoldier);

	gfPlotNewMovement    = TRUE;

	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Leaving Planning Mode" );

}

BOOLEAN InUIPlanMode( )
{
	return( gfInUIPlanMode );
}


static void SelectPausedFireAnimation(SOLDIERTYPE* const s)
{
	// Determine which animation to do...depending on stance and gun in hand...
	UINT16 state;
	switch (gAnimControl[s->usAnimState].ubEndHeight)
	{
		case ANIM_STAND:  state = (s->bDoBurst > 0 ? STANDING_BURST : SHOOT_RIFLE_STAND); break;
		case ANIM_PRONE:  state = SHOOT_RIFLE_PRONE; break;
		case ANIM_CROUCH: state = SHOOT_RIFLE_CROUCH; break;
		default:          return;
	}
	ChangeSoldierState(s, state, 2, FALSE);
}
