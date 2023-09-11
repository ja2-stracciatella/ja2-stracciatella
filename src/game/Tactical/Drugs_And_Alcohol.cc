#include "Font_Control.h"
#include "Soldier_Control.h"
#include "Soldier_Profile.h"
#include "Drugs_And_Alcohol.h"
#include "GameInstance.h"
#include "ContentManager.h"
#include "ItemModel.h"
#include "Items.h"
#include "Morale.h"
#include "Points.h"
#include "Message.h"
#include "Random.h"
#include "Text.h"
#include "Interface.h"
#include "Timer_Control.h"


UINT8 ubDrugTravelRate[]	= {4,	2};
UINT8 ubDrugWearoffRate[]	= {2,	2};
UINT8 ubDrugEffect[]		= {15, 8};
UINT8 ubDrugSideEffect[]	= {20, 10};
UINT8 ubDrugSideEffectRate[]	= {2,	1};

INT32	giDrunkModifier[] =
{
	100, // Sober
	75, // Feeling good,
	65, // Borderline
	50, // Drunk
	100, // HungOver
};

#define HANGOVER_AP_REDUCE	5
#define HANGOVER_BP_REDUCE	200


static UINT8 GetDrugType(UINT16 usItem)
{
	if ( usItem == ADRENALINE_BOOSTER )
	{
		return( DRUG_TYPE_ADRENALINE );
	}

	if ( usItem == REGEN_BOOSTER )
	{
		return( DRUG_TYPE_REGENERATION );
	}

	if ( usItem == ALCOHOL || usItem == WINE || usItem == BEER )
	{
		return( DRUG_TYPE_ALCOHOL );
	}


	return( NO_DRUG );
}


BOOLEAN ApplyDrugs( SOLDIERTYPE *pSoldier, OBJECTTYPE *pObject )
{
	UINT8 ubDrugType;
	UINT8 ubKitPoints;
	UINT16 usItem;

	usItem = pObject->usItem;

	// If not a syringe, return

	ubDrugType = GetDrugType( usItem );

	// Determine what type of drug....
	if ( ubDrugType == NO_DRUG )
	{
		return( FALSE );
	}

	// do switch for Larry!!
	if ( pSoldier->ubProfile == LARRY_NORMAL )
	{
		pSoldier = SwapLarrysProfiles( pSoldier );
	}
	else if ( pSoldier->ubProfile == LARRY_DRUNK )
	{
		gMercProfiles[ LARRY_DRUNK ].bNPCData = 0;
	}

	if ( ubDrugType < NUM_COMPLEX_DRUGS )
	{

		// Add effects
		if ( ( pSoldier->bFutureDrugEffect[ ubDrugType ] + ubDrugEffect[ ubDrugType ] ) < 127 )
		{
			pSoldier->bFutureDrugEffect[ ubDrugType ] += ubDrugEffect[ ubDrugType ];
		}
		pSoldier->bDrugEffectRate[ ubDrugType ] = ubDrugTravelRate[ ubDrugType ];

		// Increment times used during lifetime...
		// CAP!
		if ( ubDrugType == DRUG_TYPE_ADRENALINE )
		{
			if ( gMercProfiles[ pSoldier->ubProfile ].ubNumTimesDrugUseInLifetime != 255 )
			{
				gMercProfiles[ pSoldier->ubProfile ].ubNumTimesDrugUseInLifetime++;
			}
		}

		// Increment side effects..
		if ( ( pSoldier->bDrugSideEffect[ ubDrugType ] + ubDrugSideEffect[ ubDrugType ] ) < 127 )
		{
			pSoldier->bDrugSideEffect[ ubDrugType ] += ( ubDrugSideEffect[ ubDrugType ] );
		}
		// Stop side effects until were done....
		pSoldier->bDrugSideEffectRate[ ubDrugType ] = 0;


		if ( ubDrugType == DRUG_TYPE_ALCOHOL )
		{
			// ATE: use kit points...
			if ( usItem == ALCOHOL )
			{
				ubKitPoints = 10;
			}
			else if ( usItem == WINE )
			{
				ubKitPoints = 20;
			}
			else
			{
				ubKitPoints = 100;
			}

			UseKitPoints(*pObject, ubKitPoints, *pSoldier);
		}
		else
		{
			// Remove the object....
			if (--pObject->ubNumberOfObjects == 0) DeleteObj(pObject);

			// ATE: Make guy collapse from heart attack if too much stuff taken....
			if ( pSoldier->bDrugSideEffectRate[ ubDrugType ] > ( ubDrugSideEffect[ ubDrugType ] * 3 ) )
			{
				// Keel over...
				DeductPoints( pSoldier, 0, 10000 );

				// Permanently lower certain stats...
				pSoldier->bWisdom    -= 5;
				pSoldier->bDexterity -= 5;
				pSoldier->bStrength  -= 5;

				if (pSoldier->bWisdom < 1)
					pSoldier->bWisdom = 1;
				if (pSoldier->bDexterity < 1)
					pSoldier->bDexterity = 1;
				if (pSoldier->bStrength < 1)
					pSoldier->bStrength = 1;

				// export stat changes to profile
				gMercProfiles[ pSoldier->ubProfile ].bWisdom    = pSoldier->bWisdom;
				gMercProfiles[ pSoldier->ubProfile ].bDexterity = pSoldier->bDexterity;
				gMercProfiles[ pSoldier->ubProfile ].bStrength  = pSoldier->bStrength;

				// make those stats RED for a while...
				pSoldier->uiChangeWisdomTime = GetJA2Clock();
				pSoldier->usValueGoneUp &= ~( WIS_INCREASE );
				pSoldier->uiChangeDexterityTime = GetJA2Clock();
				pSoldier->usValueGoneUp &= ~( DEX_INCREASE );
				pSoldier->uiChangeStrengthTime = GetJA2Clock();
				pSoldier->usValueGoneUp &= ~( STRENGTH_INCREASE );
			}
		}
	}
	else
	{
		if ( ubDrugType == DRUG_TYPE_REGENERATION )
		{
			UINT8 const n      = --pObject->ubNumberOfObjects;
			INT8  const status = pObject->bStatus[n];
			if (n == 0) DeleteObj(pObject);

			// each use of a regen booster over 1, each day, reduces the effect
			INT8 bRegenPointsGained = REGEN_POINTS_PER_BOOSTER * status / 100;
			// are there fractional %s left over?
			if (status % (100 / REGEN_POINTS_PER_BOOSTER) != 0)
			{
				// chance of an extra point
				if (PreRandom(100 / REGEN_POINTS_PER_BOOSTER) < (UINT32)(status % (100 / REGEN_POINTS_PER_BOOSTER)))
				{
					bRegenPointsGained++;
				}
			}

			bRegenPointsGained -= pSoldier->bRegenBoostersUsedToday;
			if (bRegenPointsGained > 0)
			{
				// can't go above the points you get for a full boost
				pSoldier->bRegenerationCounter = std::min(pSoldier->bRegenerationCounter + bRegenPointsGained, REGEN_POINTS_PER_BOOSTER);
			}
			pSoldier->bRegenBoostersUsedToday++;
		}
	}

	if ( ubDrugType == DRUG_TYPE_ALCOHOL )
	{
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(pMessageStrings[ MSG_DRANK_SOME ], pSoldier->name, GCM->getItem(usItem)->getShortName()) );
	}
	else
	{
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(pMessageStrings[ MSG_MERC_TOOK_DRUG ], pSoldier->name) );
	}

	// Dirty panel
	DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2);

	return( TRUE );
}

void HandleEndTurnDrugAdjustments( SOLDIERTYPE *pSoldier )
{
	INT32 cnt, cnt2;
	INT32 iNumLoops;
	//INT8 bBandaged;

	for ( cnt = 0; cnt < NUM_COMPLEX_DRUGS; cnt++ )
	{
		// If side effect rate is non-zero....
		if ( pSoldier->bDrugSideEffectRate[ cnt ] > 0 )
		{
			// Subtract some...
			pSoldier->bDrugSideEffect[ cnt ] -= pSoldier->bDrugSideEffectRate[ cnt ];

			// If we're done, we're done!
			if ( pSoldier->bDrugSideEffect[ cnt ] <= 0 )
			{
				pSoldier->bDrugSideEffect[ cnt ] = 0;
				DirtyMercPanelInterface(pSoldier, DIRTYLEVEL1);
			}
		}

		// IF drug rate is -ve, it's being worn off...
		if ( pSoldier->bDrugEffectRate[ cnt ] < 0 )
		{
			pSoldier->bDrugEffect[ cnt ] -= ( -1 * pSoldier->bDrugEffectRate[ cnt ] );

			// Have we run out?
			if ( pSoldier->bDrugEffect[ cnt ] <= 0 )
			{
				pSoldier->bDrugEffect[ cnt ] = 0;

				// Dirty panel
				DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2);

				// Start the bad news!
				pSoldier->bDrugSideEffectRate[ cnt ] = ubDrugSideEffectRate[ cnt ];

				// The drug rate is 0 now too
				pSoldier->bDrugEffectRate[ cnt ] = 0;

				// Once for each 'level' of crash....
				iNumLoops = ( pSoldier->bDrugSideEffect[ cnt ] / ubDrugSideEffect[ cnt ] ) + 1;

				for ( cnt2 = 0; cnt2 < iNumLoops; cnt2++ )
				{
					// OK, give a much BIGGER morale downer
					if ( cnt == DRUG_TYPE_ALCOHOL )
					{
						HandleMoraleEvent(pSoldier, MORALE_ALCOHOL_CRASH, pSoldier->sSector);
					}
					else
					{
						HandleMoraleEvent(pSoldier, MORALE_DRUGS_CRASH, pSoldier->sSector);
					}
				}
			}
		}

		// Add increase in effect....
		if ( pSoldier->bDrugEffectRate[ cnt ] > 0 )
		{
			// Sip some in....
			pSoldier->bFutureDrugEffect[ cnt ] -= pSoldier->bDrugEffectRate[ cnt ];
			pSoldier->bDrugEffect[ cnt ] += pSoldier->bDrugEffectRate[ cnt ];

			// Refresh morale w/ new drug value...
			RefreshSoldierMorale( pSoldier );

			// Check if we need to stop 'adding'
			if ( pSoldier->bFutureDrugEffect[ cnt ] <= 0 )
			{
				pSoldier->bFutureDrugEffect[ cnt ] = 0;
				// Change rate to -ve..
				pSoldier->bDrugEffectRate[ cnt ] = -ubDrugWearoffRate[ cnt ];
			}
		}
	}

	if ( pSoldier->bRegenerationCounter > 0)
	{
		//bBandaged = BANDAGED( pSoldier );

		// increase life
		pSoldier->bLife = std::min(pSoldier->bLife + LIFE_GAIN_PER_REGEN_POINT, int(pSoldier->bLifeMax));

		if ( pSoldier->bLife == pSoldier->bLifeMax )
		{
			pSoldier->bBleeding = 0;
		}
		else if ( pSoldier->bBleeding + pSoldier->bLife > pSoldier->bLifeMax )
		{
			// got to reduce amount of bleeding
			pSoldier->bBleeding = (pSoldier->bLifeMax - pSoldier->bLife);
		}

		// decrement counter
		pSoldier->bRegenerationCounter--;
	}
}

INT8 GetDrugEffect( SOLDIERTYPE *pSoldier, UINT8 ubDrugType  )
{
	return( pSoldier->bDrugEffect[ ubDrugType ] );
}


void HandleAPEffectDueToDrugs(const SOLDIERTYPE* const pSoldier, UINT8* const pubPoints)
{
	INT8  bDrunkLevel;
	INT16 sPoints = (*pubPoints);

	// Are we in a side effect or good effect?
	if ( pSoldier->bDrugEffect[ DRUG_TYPE_ADRENALINE ] )
	{
		// Adjust!
		sPoints += pSoldier->bDrugEffect[ DRUG_TYPE_ADRENALINE ];
	}
	else if ( pSoldier->bDrugSideEffect[ DRUG_TYPE_ADRENALINE ] )
	{
		// Adjust!
		sPoints -= pSoldier->bDrugSideEffect[ DRUG_TYPE_ADRENALINE ];

		if ( sPoints < AP_MINIMUM )
		{
			sPoints = AP_MINIMUM;
		}
	}

	bDrunkLevel = GetDrunkLevel( pSoldier );

	if ( bDrunkLevel == HUNGOVER )
	{
		// Reduce....
		sPoints -= HANGOVER_AP_REDUCE;

		if ( sPoints < AP_MINIMUM )
		{
			sPoints = AP_MINIMUM;
		}
	}

	(*pubPoints) = (UINT8)sPoints;
}


void HandleBPEffectDueToDrugs( SOLDIERTYPE *pSoldier, INT16 *psPointReduction )
{
	INT8 bDrunkLevel;

	// Are we in a side effect or good effect?
	if ( pSoldier->bDrugEffect[ DRUG_TYPE_ADRENALINE ] )
	{
		// Adjust!
		(*psPointReduction) -= ( pSoldier->bDrugEffect[ DRUG_TYPE_ADRENALINE ] * BP_RATIO_RED_PTS_TO_NORMAL );
	}
	else if ( pSoldier->bDrugSideEffect[ DRUG_TYPE_ADRENALINE ] )
	{
		// Adjust!
		(*psPointReduction) += ( pSoldier->bDrugSideEffect[ DRUG_TYPE_ADRENALINE ] * BP_RATIO_RED_PTS_TO_NORMAL );
	}

	bDrunkLevel = GetDrunkLevel( pSoldier );

	if ( bDrunkLevel == HUNGOVER )
	{
		// Reduce....
		(*psPointReduction) += HANGOVER_BP_REDUCE;
	}
}


INT8 GetDrunkLevel(const SOLDIERTYPE* pSoldier)
{
	INT8 bNumDrinks;

	// If we have a -ve effect ...
	if ( pSoldier->bDrugEffect[ DRUG_TYPE_ALCOHOL ] == 0 && pSoldier->bDrugSideEffect[ DRUG_TYPE_ALCOHOL ] == 0 )
	{
		return( SOBER );
	}

	if ( pSoldier->bDrugEffect[ DRUG_TYPE_ALCOHOL ] == 0 && pSoldier->bDrugSideEffect[ DRUG_TYPE_ALCOHOL ] != 0 )
	{
		return( HUNGOVER );
	}

	// Calculate how many drinks we have had....
	bNumDrinks = ( pSoldier->bDrugEffect[ DRUG_TYPE_ALCOHOL ] / ubDrugEffect[ DRUG_TYPE_ALCOHOL ] );

	if ( bNumDrinks <= 3 )
	{
		return( FEELING_GOOD );
	}
	else if ( bNumDrinks <= 4 )
	{
		return( BORDERLINE );
	}
	else
	{
		return( DRUNK );
	}
}


INT32 EffectStatForBeingDrunk(const SOLDIERTYPE* pSoldier, INT32 iStat)
{
	return( ( iStat * giDrunkModifier[ GetDrunkLevel( pSoldier ) ] / 100 ) );
}


BOOLEAN MercUnderTheInfluence(const SOLDIERTYPE* pSoldier)
{
	// Are we in a side effect or good effect?
	return pSoldier->bDrugEffect[DRUG_TYPE_ADRENALINE] ||
		pSoldier->bDrugSideEffect[DRUG_TYPE_ADRENALINE] ||
		GetDrunkLevel(pSoldier) != SOBER;
}
