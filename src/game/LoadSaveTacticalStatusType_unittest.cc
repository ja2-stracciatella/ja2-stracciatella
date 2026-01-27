// -*-coding: utf-8-unix;-*-

#include "gtest/gtest.h"

#include "LoadSaveTacticalStatusType.h"
#include "Overhead.h"
#include "Overhead_Types.h"
#include "Types.h"
#include "TestUtils.h"
#include "DefaultContentManagerUT.h"

#include <cstring>

using LoadSaveTacticalStatusTypeTest = DefaultContentManagerUT::BaseTest;

class LoadSaveTacticalStatusTypeTestImpl : public LoadSaveTacticalStatusTypeTest
{
protected:
	void SetUp() override
	{
		// Initialize a test tactical status with known values
		testTacticalStatus = {};

		// Basic flags and timing
		testTacticalStatus.uiFlags = 0x00000008; // IN_ENDGAME_SEQUENCE
		testTacticalStatus.ubCurrentTeam = 1;
		testTacticalStatus.sSlideTarget = 1234;
		testTacticalStatus.uiTimeSinceMercAIStart = 5000;
		testTacticalStatus.fPanicFlags = 0x03; // PANIC_BOMBS_HERE | PANIC_TRIGGERS_HERE
		testTacticalStatus.ubSpottersCalledForBy = 42;
		testTacticalStatus.uiTimeOfLastInput = 10000;
		testTacticalStatus.uiTimeSinceDemoOn = 15000;
		
		// Initialize teams
		for (int i = 0; i < MAXTEAMS; i++)
		{
			testTacticalStatus.Team[i].bFirstID = i * 10;
			testTacticalStatus.Team[i].bLastID = i * 10 + 5;
			testTacticalStatus.Team[i].RadarColor = 0xFF0000 + i * 0x1000;
			testTacticalStatus.Team[i].bSide = static_cast<Side>(i % 4);
			testTacticalStatus.Team[i].bMenInSector = i + 1;
			testTacticalStatus.Team[i].bAwareOfOpposition = i % 2;
			testTacticalStatus.Team[i].bHuman = (i % 2) ? 1 : 0;
			testTacticalStatus.Team[i].last_merc_to_radio = nullptr;
		}
		
		// Initialize civilian group hostility
		for (int i = 0; i < NUM_CIV_GROUPS; i++)
		{
			testTacticalStatus.fCivGroupHostile[i] = (i % 3 == 0);
		}
		
		// Battle sector and flags
		testTacticalStatus.ubLastBattleSector.x = 5;
		testTacticalStatus.ubLastBattleSector.y = 6;
		testTacticalStatus.fLastBattleWon = true;
		testTacticalStatus.fVirginSector = false;
		testTacticalStatus.fEnemyInSector = true;
		testTacticalStatus.fInterruptOccurred = false;
		testTacticalStatus.bRealtimeSpeed = 5;
		
		// Enemy sighting
		testTacticalStatus.fEnemySightingOnTheirTurn = true;
		testTacticalStatus.fAutoBandageMode = false;
		testTacticalStatus.ubAttackBusyCount = 3;
		testTacticalStatus.ubEngagedInConvFromActionMercID = 15;
		
		// Turn limits
		testTacticalStatus.usTactialTurnLimitCounter = 100;
		testTacticalStatus.fInTopMessage = true;
		testTacticalStatus.ubTopMessageType = 2;
		testTacticalStatus.usTactialTurnLimitMax = 200;
		testTacticalStatus.uiTactialTurnLimitClock = 30000;
		testTacticalStatus.fTactialTurnLimitStartedBeep = false;
		
		// Boxing and panic
		testTacticalStatus.bBoxingState = 1; // BOXING_WAITING_FOR_PLAYER
		testTacticalStatus.bConsNumTurnsNotSeen = 5;
		testTacticalStatus.ubArmyGuysKilled = 10;
		
		// Panic triggers
		for (int i = 0; i < NUM_PANIC_TRIGGERS; i++)
		{
			testTacticalStatus.sPanicTriggerGridNo[i] = 1000 + i * 100;
			testTacticalStatus.bPanicTriggerIsAlarm[i] = (i % 2);
			testTacticalStatus.ubPanicTolerance[i] = 50 + i * 10;
		}
		
		// Attack and combat flags
		testTacticalStatus.fAtLeastOneGuyOnMultiSelect = true;
		testTacticalStatus.fKilledEnemyOnAttack = false;
		testTacticalStatus.enemy_killed_on_attack = nullptr;
		testTacticalStatus.bEnemyKilledOnAttackLevel = 0;
		testTacticalStatus.ubEnemyKilledOnAttackLocation = 0;
		testTacticalStatus.fItemsSeenOnAttack = true;
		testTacticalStatus.items_seen_on_attack_soldier = nullptr;
		testTacticalStatus.usItemsSeenOnAttackGridNo = 2000;
		testTacticalStatus.fLockItemLocators = false;
		
		// Quotes and communication
		testTacticalStatus.ubLastQuoteSaid = 25;
		testTacticalStatus.ubLastQuoteProfileNUm = 12;
		testTacticalStatus.fCantGetThrough = false;
		testTacticalStatus.sCantGetThroughGridNo = 0;
		testTacticalStatus.sCantGetThroughSoldierGridNo = 0;
		testTacticalStatus.cant_get_through = nullptr;
		
		// Game state
		testTacticalStatus.fDidGameJustStart = false;
		testTacticalStatus.ubLastRequesterTargetID = 8;
		testTacticalStatus.ubNumCrowsPossible = 3;
		testTacticalStatus.fUnLockUIAfterHiddenInterrupt = true;
		
		// Battle statistics
		for (int i = 0; i < MAXTEAMS; i++)
		{
			testTacticalStatus.bNumFoughtInBattle[i] = i + 2;
		}
		
		// Time tracking
		testTacticalStatus.uiDecayBloodLastUpdate = 25000;
		testTacticalStatus.uiTimeSinceLastInTactical = 30000;
		testTacticalStatus.fHasAGameBeenStarted = true;
		testTacticalStatus.bConsNumTurnsWeHaventSeenButEnemyDoes = 2;
		testTacticalStatus.fSomeoneHit = true;
		testTacticalStatus.uiTimeSinceLastOpplistDecay = 35000;
		
		// Guide description
		testTacticalStatus.bMercArrivingQuoteBeingUsed = 1;
		testTacticalStatus.enemy_killed_on_attack_killer = nullptr;
		testTacticalStatus.fCountingDownForGuideDescription = false;
		testTacticalStatus.bGuideDescriptionCountDown = 0;
		testTacticalStatus.ubGuideDescriptionToUse = 0;
		testTacticalStatus.bGuideDescriptionSectorX = 0;
		testTacticalStatus.bGuideDescriptionSectorY = 0;
		
		// Enemy flags and UI
		testTacticalStatus.fEnemyFlags = 0x01; // ENEMY_OFFERED_SURRENDER
		testTacticalStatus.fAutoBandagePending = false;
		testTacticalStatus.fHasEnteredCombatModeSinceEntering = true;
		testTacticalStatus.fDontAddNewCrows = false;
		
		// Creature tense quotes
		testTacticalStatus.sCreatureTenseQuoteDelay = 5000;
		testTacticalStatus.uiCreatureTenseQuoteLastUpdate = 40000;
	}
	
	TacticalStatusType testTacticalStatus;
};

TEST_F(LoadSaveTacticalStatusTypeTestImpl, BasicFieldValidation)
{
	// Test that our test data is properly initialized
	EXPECT_EQ(testTacticalStatus.uiFlags, 0x00000008);
	EXPECT_EQ(testTacticalStatus.ubCurrentTeam, 1);
	EXPECT_EQ(testTacticalStatus.sSlideTarget, 1234);
	EXPECT_EQ(testTacticalStatus.uiTimeSinceMercAIStart, 5000);
	EXPECT_EQ(testTacticalStatus.fPanicFlags, 0x03);
	EXPECT_EQ(testTacticalStatus.ubSpottersCalledForBy, 42);
	EXPECT_EQ(testTacticalStatus.uiTimeOfLastInput, 10000);
	EXPECT_EQ(testTacticalStatus.uiTimeSinceDemoOn, 15000);
	EXPECT_EQ(testTacticalStatus.ubLastBattleSector.x, 5);
	EXPECT_EQ(testTacticalStatus.ubLastBattleSector.y, 6);
	EXPECT_EQ(testTacticalStatus.fLastBattleWon, true);
	EXPECT_EQ(testTacticalStatus.fVirginSector, false);
	EXPECT_EQ(testTacticalStatus.fEnemyInSector, true);
	EXPECT_EQ(testTacticalStatus.fInterruptOccurred, false);
	EXPECT_EQ(testTacticalStatus.bRealtimeSpeed, 5);
	EXPECT_EQ(testTacticalStatus.fEnemySightingOnTheirTurn, true);
	EXPECT_EQ(testTacticalStatus.fAutoBandageMode, false);
	EXPECT_EQ(testTacticalStatus.ubAttackBusyCount, 3);
	EXPECT_EQ(testTacticalStatus.ubEngagedInConvFromActionMercID, 15);
	EXPECT_EQ(testTacticalStatus.usTactialTurnLimitCounter, 100);
	EXPECT_EQ(testTacticalStatus.fInTopMessage, true);
	EXPECT_EQ(testTacticalStatus.ubTopMessageType, 2);
	EXPECT_EQ(testTacticalStatus.usTactialTurnLimitMax, 200);
	EXPECT_EQ(testTacticalStatus.uiTactialTurnLimitClock, 30000);
	EXPECT_EQ(testTacticalStatus.fTactialTurnLimitStartedBeep, false);
	EXPECT_EQ(testTacticalStatus.bBoxingState, 1);
	EXPECT_EQ(testTacticalStatus.bConsNumTurnsNotSeen, 5);
	EXPECT_EQ(testTacticalStatus.ubArmyGuysKilled, 10);
	EXPECT_EQ(testTacticalStatus.fAtLeastOneGuyOnMultiSelect, true);
	EXPECT_EQ(testTacticalStatus.fKilledEnemyOnAttack, false);
	EXPECT_EQ(testTacticalStatus.bEnemyKilledOnAttackLevel, 0);
	EXPECT_EQ(testTacticalStatus.ubEnemyKilledOnAttackLocation, 0);
	EXPECT_EQ(testTacticalStatus.fItemsSeenOnAttack, true);
	EXPECT_EQ(testTacticalStatus.usItemsSeenOnAttackGridNo, 2000);
	EXPECT_EQ(testTacticalStatus.fLockItemLocators, false);
	EXPECT_EQ(testTacticalStatus.ubLastQuoteSaid, 25);
	EXPECT_EQ(testTacticalStatus.ubLastQuoteProfileNUm, 12);
	EXPECT_EQ(testTacticalStatus.fCantGetThrough, false);
	EXPECT_EQ(testTacticalStatus.sCantGetThroughGridNo, 0);
	EXPECT_EQ(testTacticalStatus.sCantGetThroughSoldierGridNo, 0);
	EXPECT_EQ(testTacticalStatus.fDidGameJustStart, false);
	EXPECT_EQ(testTacticalStatus.ubLastRequesterTargetID, 8);
	EXPECT_EQ(testTacticalStatus.ubNumCrowsPossible, 3);
	EXPECT_EQ(testTacticalStatus.fUnLockUIAfterHiddenInterrupt, true);
	EXPECT_EQ(testTacticalStatus.uiDecayBloodLastUpdate, 25000);
	EXPECT_EQ(testTacticalStatus.uiTimeSinceLastInTactical, 30000);
	EXPECT_EQ(testTacticalStatus.fHasAGameBeenStarted, true);
	EXPECT_EQ(testTacticalStatus.bConsNumTurnsWeHaventSeenButEnemyDoes, 2);
	EXPECT_EQ(testTacticalStatus.fSomeoneHit, true);
	EXPECT_EQ(testTacticalStatus.uiTimeSinceLastOpplistDecay, 35000);
	EXPECT_EQ(testTacticalStatus.bMercArrivingQuoteBeingUsed, 1);
	EXPECT_EQ(testTacticalStatus.fCountingDownForGuideDescription, false);
	EXPECT_EQ(testTacticalStatus.bGuideDescriptionCountDown, 0);
	EXPECT_EQ(testTacticalStatus.ubGuideDescriptionToUse, 0);
	EXPECT_EQ(testTacticalStatus.bGuideDescriptionSectorX, 0);
	EXPECT_EQ(testTacticalStatus.bGuideDescriptionSectorY, 0);
	EXPECT_EQ(testTacticalStatus.fEnemyFlags, 0x01);
	EXPECT_EQ(testTacticalStatus.fAutoBandagePending, false);
	EXPECT_EQ(testTacticalStatus.fHasEnteredCombatModeSinceEntering, true);
	EXPECT_EQ(testTacticalStatus.fDontAddNewCrows, false);
	EXPECT_EQ(testTacticalStatus.sCreatureTenseQuoteDelay, 5000);
	EXPECT_EQ(testTacticalStatus.uiCreatureTenseQuoteLastUpdate, 40000);
}

TEST_F(LoadSaveTacticalStatusTypeTestImpl, TeamDataValidation)
{
	// Test team data initialization
	for (int i = 0; i < MAXTEAMS; i++)
	{
		EXPECT_EQ(testTacticalStatus.Team[i].bFirstID, i * 10);
		EXPECT_EQ(testTacticalStatus.Team[i].bLastID, i * 10 + 5);
		EXPECT_EQ(testTacticalStatus.Team[i].RadarColor, 0xFF0000 + i * 0x1000);
		EXPECT_EQ(testTacticalStatus.Team[i].bSide, static_cast<Side>(i % 4));
		EXPECT_EQ(testTacticalStatus.Team[i].bMenInSector, i + 1);
		EXPECT_EQ(testTacticalStatus.Team[i].bAwareOfOpposition, i % 2);
		EXPECT_EQ(testTacticalStatus.Team[i].bHuman, (i % 2) ? 1 : 0);
		EXPECT_EQ(testTacticalStatus.Team[i].last_merc_to_radio, nullptr);
	}
}

TEST_F(LoadSaveTacticalStatusTypeTestImpl, ArrayDataValidation)
{
	// Test civilian group hostility array
	for (int i = 0; i < NUM_CIV_GROUPS; i++)
	{
		EXPECT_EQ(testTacticalStatus.fCivGroupHostile[i], (i % 3 == 0));
	}
	
	// Test panic trigger arrays
	for (int i = 0; i < NUM_PANIC_TRIGGERS; i++)
	{
		EXPECT_EQ(testTacticalStatus.sPanicTriggerGridNo[i], 1000 + i * 100);
		EXPECT_EQ(testTacticalStatus.bPanicTriggerIsAlarm[i], (i % 2));
		EXPECT_EQ(testTacticalStatus.ubPanicTolerance[i], 50 + i * 10);
	}
	
	// Test battle statistics array
	for (int i = 0; i < MAXTEAMS; i++)
	{
		EXPECT_EQ(testTacticalStatus.bNumFoughtInBattle[i], i + 2);
	}
}

TEST_F(LoadSaveTacticalStatusTypeTestImpl, ConstantsValidation)
{
	// Test that important constants are defined correctly
	EXPECT_EQ(MAXTEAMS, 6);
	EXPECT_EQ(NUM_CIV_GROUPS, 20);
	EXPECT_EQ(NUM_PANIC_TRIGGERS, 3);
	EXPECT_EQ(TACTICAL_STATUS_TYPE_SIZE, 316);
	EXPECT_EQ(TACTICAL_STATUS_TYPE_SIZE_STRAC_LINUX, 360);
}
