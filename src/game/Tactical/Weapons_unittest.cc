// -*-coding: utf-8-unix;-*-

#include "gtest/gtest.h"

#include "DefaultContentManager.h"
#include "DefaultContentManagerUT.h"
#include "GameInstance.h"
#include "Item_Types.h"
#include "Soldier_Control.h"
#include "Weapons.h"


TEST(WeaponsTest, EffectiveArmour)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();
	ASSERT_TRUE(cm->loadGameData());
	GCM = cm;

	// Immaculate spectra vest
	auto obj = OBJECTTYPE{};
	obj.usItem =  SPECTRA_VEST_18;
	obj.bStatus[0] = 100;
	EXPECT_EQ(EffectiveArmour(&obj), 36);

	// Status should have influence on effective armour
	obj.bStatus[0] = 50;
	EXPECT_EQ(EffectiveArmour(&obj), 18);

	// With ceramic plates
	obj.usAttachItem[0] = CERAMIC_PLATES;
	obj.bAttachStatus[0] = 100;
	EXPECT_EQ(EffectiveArmour(&obj), 33);

	// Status of ceramic plates should have influence on effective armour
	obj.bAttachStatus[0] = 80;
	EXPECT_EQ(EffectiveArmour(&obj), 30);

	delete cm;
}

TEST(WeaponsTest, ExplosiveEffectiveArmour)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();
	ASSERT_TRUE(cm->loadGameData());
	GCM = cm;

	// Immaculate spectra vest
	auto obj = OBJECTTYPE{};
	obj.usItem = SPECTRA_VEST_18;
	obj.bStatus[0] = 100;
	EXPECT_EQ(ExplosiveEffectiveArmour(&obj), 36);

	// Status should have influence on effective armour
	obj.bStatus[0] = 50;
	EXPECT_EQ(ExplosiveEffectiveArmour(&obj), 18);

	// With ceramic plates
	obj.usAttachItem[0] = CERAMIC_PLATES;
	obj.bAttachStatus[0] = 100;
	EXPECT_EQ(ExplosiveEffectiveArmour(&obj), 33);

	// Status of ceramic plates should have influence on effective armour
	obj.bAttachStatus[0] = 80;
	EXPECT_EQ(ExplosiveEffectiveArmour(&obj), 30);

	// Immaculate flak jacket
	obj = OBJECTTYPE{};
	obj.usItem = FLAK_JACKET;
	obj.bStatus[0] = 100;
	EXPECT_EQ(ExplosiveEffectiveArmour(&obj), 30);

	// Status should have influence on effective armour
	obj.bStatus[0] = 50;
	EXPECT_EQ(ExplosiveEffectiveArmour(&obj), 15);

	// With ceramic plates
	obj.usAttachItem[0] = CERAMIC_PLATES;
	obj.bAttachStatus[0] = 100;
	EXPECT_EQ(ExplosiveEffectiveArmour(&obj), 30);

	// Status of ceramic plates should have influence on effective armour
	obj.bAttachStatus[0] = 80;
	EXPECT_EQ(ExplosiveEffectiveArmour(&obj), 27);

	delete cm;
}

TEST(WeaponsTest, ArmourPercent)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();
	ASSERT_TRUE(cm->loadGameData());
	GCM = cm;

	// Fully kitted out soldier without ceramic plates
	auto soldier = SOLDIERTYPE{};
	soldier.inv[0].usItem = SPECTRA_HELMET_18;
	soldier.inv[0].bStatus[0] = 100;
	soldier.inv[1].usItem = SPECTRA_VEST_18;
	soldier.inv[1].bStatus[0] = 100;
	soldier.inv[2].usItem = SPECTRA_LEGGINGS_18;
	soldier.inv[2].bStatus[0] = 100;
	EXPECT_EQ(ArmourPercent(&soldier), 85);

	// Fully kitted out soldier with ceramic plates
	soldier.inv[1].usAttachItem[0] = CERAMIC_PLATES;
	soldier.inv[1].bAttachStatus[0] = 100;
	EXPECT_EQ(ArmourPercent(&soldier), 105);

	soldier.inv[1].usItem = SPECTRA_VEST_Y;
	EXPECT_EQ(ArmourPercent(&soldier), 112);

	soldier.inv[1].usItem = FLAK_JACKET;
	EXPECT_EQ(ArmourPercent(&soldier), 71);

	soldier.inv[1].bStatus[0] = 80;
	soldier.inv[1].bAttachStatus[0] = 80;
	EXPECT_EQ(ArmourPercent(&soldier), 65);

	delete cm;
}

TEST(WeaponsTest, ArmourVersusExplosivesPercent)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();
	ASSERT_TRUE(cm->loadGameData());
	GCM = cm;

	// Fully kitted out soldier without ceramic plates
	auto soldier = SOLDIERTYPE{};
	soldier.inv[0].usItem = SPECTRA_HELMET_18;
	soldier.inv[0].bStatus[0] = 100;
	soldier.inv[1].usItem = SPECTRA_VEST_18;
	soldier.inv[1].bStatus[0] = 100;
	soldier.inv[2].usItem = SPECTRA_LEGGINGS_18;
	soldier.inv[2].bStatus[0] = 100;
	EXPECT_EQ(ArmourVersusExplosivesPercent(&soldier), 85);

	// Fully kitted out soldier with ceramic plates
	soldier.inv[1].usAttachItem[0] = CERAMIC_PLATES;
	soldier.inv[1].bAttachStatus[0] = 100;
	EXPECT_EQ(ArmourVersusExplosivesPercent(&soldier), 105);

	soldier.inv[1].usItem = SPECTRA_VEST_Y;
	EXPECT_EQ(ArmourVersusExplosivesPercent(&soldier), 105);

	soldier.inv[1].usItem = FLAK_JACKET;
	EXPECT_EQ(ArmourVersusExplosivesPercent(&soldier), 97);

	soldier.inv[1].bStatus[0] = 80;
	soldier.inv[1].bAttachStatus[0] = 80;
	EXPECT_EQ(ArmourVersusExplosivesPercent(&soldier), 85);

	delete cm;
}
