// -*-coding: utf-8-unix;-*-

#include "gtest/gtest.h"

#include "LoadSaveSoldierType.h"
#include "Soldier_Control.h"
#include "Overhead_Types.h"
#include "Types.h"
#include "TestUtils.h"
#include "DefaultContentManagerUT.h"

#include <cstring>
#include <memory>

#ifndef NO_MAGICENUM_LIB
#include <magic_enum.hpp>
#endif

using LoadSaveSoldierTypeTest = DefaultContentManagerUT::BaseTest;

class LoadSaveSoldierTypeTestImpl : public LoadSaveSoldierTypeTest
{
protected:
	void SetUp() override
	{
		// Initialize a test soldier with known values
		memset(&testSoldier, 0, sizeof(SOLDIERTYPE));
		
		// Basic identification
		testSoldier.ubID = 42;
		testSoldier.ubProfile = 123;
		testSoldier.name = "TestMerc";
		
		// Body and stats
		testSoldier.ubBodyType = 0; // REGMALE
		testSoldier.bActionPoints = 25;
		testSoldier.bInitialActionPoints = 25;
		testSoldier.uiStatusFlags = 0x00000008; // SOLDIER_PC
		
		// Core stats
		testSoldier.bLife = 85;
		testSoldier.bLifeMax = 85;
		testSoldier.bAgility = 75;
		testSoldier.bDexterity = 80;
		testSoldier.bStrength = 70;
		testSoldier.bMarksmanship = 90;
		testSoldier.bMedical = 60;
		testSoldier.bMechanical = 55;
		testSoldier.bExplosive = 65;
		testSoldier.bLeadership = 50;
		testSoldier.bWisdom = 45;
		testSoldier.bExpLevel = 3;
		
		// Side and team
		testSoldier.bSide = Side::FRIENDLY;
		testSoldier.bTeam = OUR_TEAM;
		testSoldier.ubSoldierClass = SOLDIER_CLASS_ADMINISTRATOR;
		testSoldier.ubCivilianGroup = NON_CIV_GROUP;
		
		// Position and movement
		testSoldier.dXPos = 1500.5f;
		testSoldier.dYPos = 2000.25f;
		testSoldier.sGridNo = 12345;
		testSoldier.sInitialGridNo = 12000;
		testSoldier.bDirection = NORTH;
		testSoldier.sHeightAdjustment = 0;
		testSoldier.sDesiredHeight = 0;
		
		// Combat stats
		testSoldier.bBreath = 100;
		testSoldier.bBreathMax = 100;
		testSoldier.bStealthMode = 0;
		testSoldier.sBreathRed = 0;
		testSoldier.bBleeding = 0;
		testSoldier.sFractLife = 0;
		
		// Skills and traits
		testSoldier.ubSkillTrait1 = 0;
		testSoldier.ubSkillTrait2 = 0;
		
		// Palette information
		testSoldier.HeadPal = "HEAD_01";
		testSoldier.PantsPal = "PANTS_01";
		testSoldier.VestPal = "VEST_01";
		testSoldier.SkinPal = "SKIN_01";
		
		// Contract information
		testSoldier.iTotalContractLength = 7 * 24 * 60; // 7 days in minutes
		testSoldier.iEndofContractTime = 1000000;
		testSoldier.iStartContractTime = 500000;
		testSoldier.ubWhatKindOfMercAmI = 0; // MERC
		testSoldier.bAssignment = 0; // NO_ASSIGNMENT
		
		// Sector information
		testSoldier.sSector.x = 9;
		testSoldier.sSector.y = 1;
		testSoldier.sSector.z = 0;
		testSoldier.fBetweenSectors = FALSE;
		
		// Initialize inventory slots to empty
		for (int i = 0; i < NUM_INV_SLOTS; i++)
		{
			testSoldier.inv[i] = OBJECTTYPE{};
		}
		
		// Initialize pathing data
		testSoldier.ubPathDataSize = 0;
		testSoldier.ubPathIndex = 0;
		memset(testSoldier.ubPathingData, 0, sizeof(testSoldier.ubPathingData));
		
		// Initialize other arrays
		memset(testSoldier.bOppList, 0, sizeof(testSoldier.bOppList));
		memset(testSoldier.usPatrolGrid, 0, sizeof(testSoldier.usPatrolGrid));
		memset(testSoldier.sSpreadLocations, 0, sizeof(testSoldier.sSpreadLocations));
		memset(testSoldier.bNewItemCount, 0, sizeof(testSoldier.bNewItemCount));
		memset(testSoldier.bNewItemCycleCount, 0, sizeof(testSoldier.bNewItemCycleCount));
		memset(testSoldier.bFutureDrugEffect, 0, sizeof(testSoldier.bFutureDrugEffect));
		memset(testSoldier.bDrugEffectRate, 0, sizeof(testSoldier.bDrugEffectRate));
		memset(testSoldier.bDrugEffect, 0, sizeof(testSoldier.bDrugEffect));
		memset(testSoldier.bDrugSideEffectRate, 0, sizeof(testSoldier.bDrugSideEffectRate));
		memset(testSoldier.bDrugSideEffect, 0, sizeof(testSoldier.bDrugSideEffect));
		memset(testSoldier.sLastTwoLocations, 0, sizeof(testSoldier.sLastTwoLocations));
		
		// Initialize pointers to null
		testSoldier.pTempObject = nullptr;
		testSoldier.pKeyRing = nullptr;
		testSoldier.opponent = nullptr;
		testSoldier.attacker = nullptr;
		testSoldier.previous_attacker = nullptr;
		testSoldier.next_to_previous_attacker = nullptr;
		testSoldier.service_partner = nullptr;
		testSoldier.pThrowParams = nullptr;
		testSoldier.pLevelNode = nullptr;
		testSoldier.pMercPath = nullptr;
		testSoldier.suppressor = nullptr;
		testSoldier.target = nullptr;
		testSoldier.pAniTile = nullptr;
		testSoldier.auto_bandaging_medic = nullptr;
		testSoldier.robot_remote_holder = nullptr;
		testSoldier.CTGTTarget = nullptr;
		testSoldier.xrayed_by = nullptr;
		testSoldier.pForcedShade = nullptr;
		testSoldier.light = nullptr;
		testSoldier.muzzle_flash = nullptr;
		testSoldier.face = nullptr;
		testSoldier.effect_shade = nullptr;
		
		// Initialize shade arrays
		for (int i = 0; i < NUM_SOLDIER_SHADES; i++)
		{
			testSoldier.pShades[i] = nullptr;
		}
		for (int i = 0; i < 20; i++)
		{
			testSoldier.pGlowShades[i] = nullptr;
		}
	}
	
	SOLDIERTYPE testSoldier;
	static constexpr size_t BUFFER_SIZE = 3000; // Should be larger than expected soldier size
};

TEST_F(LoadSaveSoldierTypeTestImpl, RoundTripSerialization)
{
	// Create buffer for serialization
	std::unique_ptr<BYTE[]> buffer(new BYTE[BUFFER_SIZE]);
	memset(buffer.get(), 0, BUFFER_SIZE);
	
	// Inject the soldier data
	InjectSoldierType(buffer.get(), &testSoldier);
	
	// Extract the soldier data
	SOLDIERTYPE extractedSoldier;
	ExtractSoldierType(buffer.get(), &extractedSoldier, false, 0);
	
	// Verify basic identification
	EXPECT_EQ(extractedSoldier.ubID, testSoldier.ubID);
	EXPECT_EQ(extractedSoldier.ubProfile, testSoldier.ubProfile);
	EXPECT_EQ(extractedSoldier.name, testSoldier.name);
	
	// Verify body and stats
	EXPECT_EQ(extractedSoldier.ubBodyType, testSoldier.ubBodyType);
	EXPECT_EQ(extractedSoldier.bActionPoints, testSoldier.bActionPoints);
	EXPECT_EQ(extractedSoldier.bInitialActionPoints, testSoldier.bInitialActionPoints);
	EXPECT_EQ(extractedSoldier.uiStatusFlags, testSoldier.uiStatusFlags);
	
	// Verify core stats
	EXPECT_EQ(extractedSoldier.bLife, testSoldier.bLife);
	EXPECT_EQ(extractedSoldier.bLifeMax, testSoldier.bLifeMax);
	EXPECT_EQ(extractedSoldier.bAgility, testSoldier.bAgility);
	EXPECT_EQ(extractedSoldier.bDexterity, testSoldier.bDexterity);
	EXPECT_EQ(extractedSoldier.bStrength, testSoldier.bStrength);
	EXPECT_EQ(extractedSoldier.bMarksmanship, testSoldier.bMarksmanship);
	EXPECT_EQ(extractedSoldier.bMedical, testSoldier.bMedical);
	EXPECT_EQ(extractedSoldier.bMechanical, testSoldier.bMechanical);
	EXPECT_EQ(extractedSoldier.bExplosive, testSoldier.bExplosive);
	EXPECT_EQ(extractedSoldier.bLeadership, testSoldier.bLeadership);
	EXPECT_EQ(extractedSoldier.bWisdom, testSoldier.bWisdom);
	EXPECT_EQ(extractedSoldier.bExpLevel, testSoldier.bExpLevel);
	
	// Verify side and team
	EXPECT_EQ(extractedSoldier.bSide, testSoldier.bSide);
	EXPECT_EQ(extractedSoldier.bTeam, testSoldier.bTeam);
	EXPECT_EQ(extractedSoldier.ubSoldierClass, testSoldier.ubSoldierClass);
	EXPECT_EQ(extractedSoldier.ubCivilianGroup, testSoldier.ubCivilianGroup);
	
	// Verify position
	EXPECT_FLOAT_EQ(extractedSoldier.dXPos, testSoldier.dXPos);
	EXPECT_FLOAT_EQ(extractedSoldier.dYPos, testSoldier.dYPos);
	EXPECT_EQ(extractedSoldier.sGridNo, testSoldier.sGridNo);
	EXPECT_EQ(extractedSoldier.sInitialGridNo, testSoldier.sInitialGridNo);
	EXPECT_EQ(extractedSoldier.bDirection, testSoldier.bDirection);
	EXPECT_EQ(extractedSoldier.sHeightAdjustment, testSoldier.sHeightAdjustment);
	EXPECT_EQ(extractedSoldier.sDesiredHeight, testSoldier.sDesiredHeight);
	
	// Verify combat stats
	EXPECT_EQ(extractedSoldier.bBreath, testSoldier.bBreath);
	EXPECT_EQ(extractedSoldier.bBreathMax, testSoldier.bBreathMax);
	EXPECT_EQ(extractedSoldier.bStealthMode, testSoldier.bStealthMode);
	EXPECT_EQ(extractedSoldier.sBreathRed, testSoldier.sBreathRed);
	EXPECT_EQ(extractedSoldier.bBleeding, testSoldier.bBleeding);
	EXPECT_EQ(extractedSoldier.sFractLife, testSoldier.sFractLife);
	
	// Verify skills and traits
	EXPECT_EQ(extractedSoldier.ubSkillTrait1, testSoldier.ubSkillTrait1);
	EXPECT_EQ(extractedSoldier.ubSkillTrait2, testSoldier.ubSkillTrait2);
	
	// Verify palette information
	EXPECT_EQ(extractedSoldier.HeadPal, testSoldier.HeadPal);
	EXPECT_EQ(extractedSoldier.PantsPal, testSoldier.PantsPal);
	EXPECT_EQ(extractedSoldier.VestPal, testSoldier.VestPal);
	EXPECT_EQ(extractedSoldier.SkinPal, testSoldier.SkinPal);
	
	// Verify contract information
	EXPECT_EQ(extractedSoldier.iTotalContractLength, testSoldier.iTotalContractLength);
	EXPECT_EQ(extractedSoldier.iEndofContractTime, testSoldier.iEndofContractTime);
	EXPECT_EQ(extractedSoldier.iStartContractTime, testSoldier.iStartContractTime);
	EXPECT_EQ(extractedSoldier.ubWhatKindOfMercAmI, testSoldier.ubWhatKindOfMercAmI);
	EXPECT_EQ(extractedSoldier.bAssignment, testSoldier.bAssignment);
	
	// Verify sector information
	EXPECT_EQ(extractedSoldier.sSector.x, testSoldier.sSector.x);
	EXPECT_EQ(extractedSoldier.sSector.y, testSoldier.sSector.y);
	EXPECT_EQ(extractedSoldier.sSector.z, testSoldier.sSector.z);
	EXPECT_EQ(extractedSoldier.fBetweenSectors, testSoldier.fBetweenSectors);
}

#ifndef NO_MAGICENUM_LIB
TEST_F(LoadSaveSoldierTypeTestImpl, DifferentSideValues)
{
	// Test all possible Side enum values
	magic_enum::enum_for_each<Side>([this](Side side) {
		testSoldier.bSide = side;
		
		std::unique_ptr<BYTE[]> buffer(new BYTE[BUFFER_SIZE]);
		memset(buffer.get(), 0, BUFFER_SIZE);
		
		InjectSoldierType(buffer.get(), &testSoldier);
		
		SOLDIERTYPE extractedSoldier;
		ExtractSoldierType(buffer.get(), &extractedSoldier, false, 0);
		
		EXPECT_EQ(extractedSoldier.bSide, side);
	});
}
#endif

TEST_F(LoadSaveSoldierTypeTestImpl, DifferentTeamValues)
{
	// Test all possible Team enum values
	Team teams[] = {OUR_TEAM, ENEMY_TEAM, CREATURE_TEAM, MILITIA_TEAM, CIV_TEAM};
	
	for (Team team : teams)
	{
		testSoldier.bTeam = team;
		
		std::unique_ptr<BYTE[]> buffer(new BYTE[BUFFER_SIZE]);
		memset(buffer.get(), 0, BUFFER_SIZE);
		
		InjectSoldierType(buffer.get(), &testSoldier);
		
		SOLDIERTYPE extractedSoldier;
		ExtractSoldierType(buffer.get(), &extractedSoldier, false, 0);
		
		EXPECT_EQ(extractedSoldier.bTeam, team);
	}
}

TEST_F(LoadSaveSoldierTypeTestImpl, DifferentSoldierClassValues)
{
	// Test different soldier class values
	UINT8 classes[] = {SOLDIER_CLASS_NONE, SOLDIER_CLASS_ADMINISTRATOR, SOLDIER_CLASS_ELITE, 
	                  SOLDIER_CLASS_ARMY, SOLDIER_CLASS_GREEN_MILITIA, SOLDIER_CLASS_REG_MILITIA, 
	                  SOLDIER_CLASS_ELITE_MILITIA, SOLDIER_CLASS_CREATURE, SOLDIER_CLASS_MINER};
	
	for (UINT8 soldierClass : classes)
	{
		testSoldier.ubSoldierClass = soldierClass;
		
		std::unique_ptr<BYTE[]> buffer(new BYTE[BUFFER_SIZE]);
		memset(buffer.get(), 0, BUFFER_SIZE);
		
		InjectSoldierType(buffer.get(), &testSoldier);
		
		SOLDIERTYPE extractedSoldier;
		ExtractSoldierType(buffer.get(), &extractedSoldier, false, 0);
		
		EXPECT_EQ(extractedSoldier.ubSoldierClass, soldierClass);
	}
}

TEST_F(LoadSaveSoldierTypeTestImpl, StringFields)
{
	// Test various string values
	testSoldier.name = "LongName12"; // SOLDIERTYPE_NAME_LENGTH is 10
	testSoldier.HeadPal = "HEAD_SPECIAL_01";
	testSoldier.PantsPal = "PANTS_CAMO_02";
	testSoldier.VestPal = "VEST_ARMOR_03";
	testSoldier.SkinPal = "SKIN_DARK_04";
	
	std::unique_ptr<BYTE[]> buffer(new BYTE[BUFFER_SIZE]);
	memset(buffer.get(), 0, BUFFER_SIZE);
	
	InjectSoldierType(buffer.get(), &testSoldier);
	
	SOLDIERTYPE extractedSoldier;
	ExtractSoldierType(buffer.get(), &extractedSoldier, false, 0);
	
	EXPECT_EQ(extractedSoldier.name, testSoldier.name);
	EXPECT_EQ(extractedSoldier.HeadPal, testSoldier.HeadPal);
	EXPECT_EQ(extractedSoldier.PantsPal, testSoldier.PantsPal);
	EXPECT_EQ(extractedSoldier.VestPal, testSoldier.VestPal);
	EXPECT_EQ(extractedSoldier.SkinPal, testSoldier.SkinPal);
}

TEST_F(LoadSaveSoldierTypeTestImpl, PathingData)
{
	// Test pathing data
	testSoldier.ubPathDataSize = 5;
	testSoldier.ubPathIndex = 2;
	for (int i = 0; i < 5; i++)
	{
		testSoldier.ubPathingData[i] = i + 10;
	}
	
	std::unique_ptr<BYTE[]> buffer(new BYTE[BUFFER_SIZE]);
	memset(buffer.get(), 0, BUFFER_SIZE);
	
	InjectSoldierType(buffer.get(), &testSoldier);
	
	SOLDIERTYPE extractedSoldier;
	ExtractSoldierType(buffer.get(), &extractedSoldier, false, 0);
	
	EXPECT_EQ(extractedSoldier.ubPathDataSize, 5);
	EXPECT_EQ(extractedSoldier.ubPathIndex, 2);
	for (int i = 0; i < 5; i++)
	{
		EXPECT_EQ(extractedSoldier.ubPathingData[i], i + 10);
	}
}

TEST_F(LoadSaveSoldierTypeTestImpl, InventorySlots)
{
	// Test inventory slots (basic test - full inventory testing would be more complex)
	// For now, just verify that empty inventory slots are preserved
	std::unique_ptr<BYTE[]> buffer(new BYTE[BUFFER_SIZE]);
	memset(buffer.get(), 0, BUFFER_SIZE);
	
	InjectSoldierType(buffer.get(), &testSoldier);
	
	SOLDIERTYPE extractedSoldier;
	ExtractSoldierType(buffer.get(), &extractedSoldier, false, 0);
	
	// Verify that all inventory slots are empty (as initialized)
	for (int i = 0; i < NUM_INV_SLOTS; i++)
	{
		EXPECT_EQ(extractedSoldier.inv[i].usItem, 0);
		EXPECT_EQ(extractedSoldier.inv[i].ubNumberOfObjects, 0);
	}
}

TEST_F(LoadSaveSoldierTypeTestImpl, ContractInformation)
{
	// Test contract-related fields
	testSoldier.iTotalContractLength = 14 * 24 * 60; // 14 days
	testSoldier.iEndofContractTime = 2000000;
	testSoldier.iStartContractTime = 1000000;
	testSoldier.iNextActionSpecialData = 12345;
	testSoldier.ubWhatKindOfMercAmI = 1; // AIM_MERC
	testSoldier.bAssignment = 1; // TRAIN_TOWN
	testSoldier.fForcedToStayAwake = TRUE;
	testSoldier.bTrainStat = 0; // STRENGTH
	testSoldier.iVehicleId = 5;
	testSoldier.fHitByGasFlags = 0x0F;
	testSoldier.usMedicalDeposit = 5000;
	testSoldier.usLifeInsurance = 10000;
	testSoldier.iStartOfInsuranceContract = 500000;
	testSoldier.uiLastAssignmentChangeMin = 1000;
	testSoldier.iTotalLengthOfInsuranceContract = 30 * 24 * 60; // 30 days
	
	std::unique_ptr<BYTE[]> buffer(new BYTE[BUFFER_SIZE]);
	memset(buffer.get(), 0, BUFFER_SIZE);
	
	InjectSoldierType(buffer.get(), &testSoldier);
	
	SOLDIERTYPE extractedSoldier;
	ExtractSoldierType(buffer.get(), &extractedSoldier, false, 0);
	
	EXPECT_EQ(extractedSoldier.iTotalContractLength, testSoldier.iTotalContractLength);
	EXPECT_EQ(extractedSoldier.iEndofContractTime, testSoldier.iEndofContractTime);
	EXPECT_EQ(extractedSoldier.iStartContractTime, testSoldier.iStartContractTime);
	EXPECT_EQ(extractedSoldier.iNextActionSpecialData, testSoldier.iNextActionSpecialData);
	EXPECT_EQ(extractedSoldier.ubWhatKindOfMercAmI, testSoldier.ubWhatKindOfMercAmI);
	EXPECT_EQ(extractedSoldier.bAssignment, testSoldier.bAssignment);
	EXPECT_EQ(extractedSoldier.fForcedToStayAwake, testSoldier.fForcedToStayAwake);
	EXPECT_EQ(extractedSoldier.bTrainStat, testSoldier.bTrainStat);
	EXPECT_EQ(extractedSoldier.iVehicleId, testSoldier.iVehicleId);
	EXPECT_EQ(extractedSoldier.fHitByGasFlags, testSoldier.fHitByGasFlags);
	EXPECT_EQ(extractedSoldier.usMedicalDeposit, testSoldier.usMedicalDeposit);
	EXPECT_EQ(extractedSoldier.usLifeInsurance, testSoldier.usLifeInsurance);
	EXPECT_EQ(extractedSoldier.iStartOfInsuranceContract, testSoldier.iStartOfInsuranceContract);
	EXPECT_EQ(extractedSoldier.uiLastAssignmentChangeMin, testSoldier.uiLastAssignmentChangeMin);
	EXPECT_EQ(extractedSoldier.iTotalLengthOfInsuranceContract, testSoldier.iTotalLengthOfInsuranceContract);
}
