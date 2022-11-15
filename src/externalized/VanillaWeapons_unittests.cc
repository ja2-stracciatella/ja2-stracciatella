#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

#include "game/Tactical/Items.h"

#include "AmmoTypeModel.h"
#include "CalibreModel.h"
#include "DefaultContentManager.h"
#include "DefaultContentManagerUT.h"
#include "GameInstance.h"
#include "GamePolicy.h"
#include "MagazineModel.h"
#include "WeaponModels.h"
#include <utility>

TEST(Items, weaponsLoading)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();
	ASSERT_TRUE(cm != NULL);
	ASSERT_TRUE(cm->loadGameData());
	EXPECT_TRUE(cm->getWeaponByName("MP5K") != NULL);
	EXPECT_TRUE(cm->getWeapon(ItemId(9) /* MP5K */) != NULL);
	EXPECT_EQ(cm->getWeaponByName("MP5K"), cm->getWeapon(ItemId(9) /* MP5K */));
	delete cm;
}

TEST(Items, bug120_cawsAmmo)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();
	ASSERT_TRUE(cm->loadGameData());

	// test SAP clip parameters
	const MagazineModel* sapClip = cm->getMagazineByName("CLIPCAWS_10_SAP");
	EXPECT_EQ(sapClip->calibre->internalName, ST::string("AMMOCAWS"));
	EXPECT_EQ(sapClip->ammoType->internalName, ST::string("AMMO_SUPER_AP"));

	// test FLECH clip parameters
	const MagazineModel* flechClip = cm->getMagazineByName("CLIPCAWS_10_FLECH");
	EXPECT_EQ(flechClip->calibre->internalName, ST::string("AMMOCAWS"));
	EXPECT_EQ(flechClip->ammoType->internalName, ST::string("AMMO_BUCKSHOT"));

	delete cm;
}

TEST(Items, bug120_12gAmmo)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();
	ASSERT_TRUE(cm->loadGameData());

	// test SAP clip parameters
	const MagazineModel* clip = cm->getMagazineByName("CLIP12G_7");
	EXPECT_EQ(clip->calibre->internalName,           ST::string("AMMO12G"));
	EXPECT_EQ(clip->ammoType->internalName,          ST::string("AMMO_REGULAR"));

	// test FLECH clip parameters
	const MagazineModel* clipBuckshot = cm->getMagazineByName("CLIP12G_7_BUCKSHOT");
	EXPECT_EQ(clipBuckshot->calibre->internalName,           ST::string("AMMO12G"));
	EXPECT_EQ(clipBuckshot->ammoType->internalName,          ST::string("AMMO_BUCKSHOT"));

	delete cm;
}

TEST(Items, bug120_cawsDefaultMag)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();
	ASSERT_TRUE(cm->loadGameData());

	GCM = cm;

	const WeaponModel *caws = cm->getWeaponByName("CAWS");
	ASSERT_TRUE(caws != NULL);

	const MagazineModel* flechClip = cm->getMagazineByName("CLIPCAWS_10_FLECH");
	const MagazineModel* sapClip = cm->getMagazineByName("CLIPCAWS_10_SAP");
	ASSERT_TRUE(flechClip != NULL);
	ASSERT_TRUE(sapClip != NULL);

	EXPECT_EQ(DefaultMagazine(caws->getItemIndex()), flechClip->getItemIndex());

	EXPECT_EQ(FindReplacementMagazine(sapClip->calibre, 10, AMMO_BUCKSHOT), flechClip->getItemIndex());
	EXPECT_EQ(FindReplacementMagazine(sapClip->calibre, 10, AMMO_SUPER_AP), sapClip->getItemIndex());

	delete cm;
}

TEST(Items, bug120_spas15DefaultMag)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();
	ASSERT_TRUE(cm->loadGameData());

	GCM = cm;

	const WeaponModel *spas15 = cm->getWeaponByName("SPAS15");
	ASSERT_TRUE(spas15 != NULL);

	const MagazineModel* clipBuckshot = cm->getMagazineByName("CLIP12G_7_BUCKSHOT");
	const MagazineModel* clip = cm->getMagazineByName("CLIP12G_7");
	ASSERT_TRUE(clipBuckshot != NULL);
	ASSERT_TRUE(clip != NULL);

	EXPECT_EQ(DefaultMagazine(spas15->getItemIndex()), clipBuckshot->getItemIndex());

	EXPECT_EQ(FindReplacementMagazine(clip->calibre, 7, AMMO_BUCKSHOT), clipBuckshot->getItemIndex());
	EXPECT_EQ(FindReplacementMagazine(clip->calibre, 7, AMMO_REGULAR),  clip->getItemIndex());

	delete cm;
}

TEST(Items, ValidLaunchable)
{
	EXPECT_TRUE(ValidLaunchable(MORTAR_SHELL, MORTAR));
	EXPECT_FALSE(ValidLaunchable(MORTAR_SHELL, MORTAR_SHELL));
	EXPECT_FALSE(ValidLaunchable(MORTAR_SHELL, TANK_CANNON));
	EXPECT_FALSE(ValidLaunchable(MORTAR, MORTAR_SHELL));
	EXPECT_TRUE(ValidLaunchable(GL_HE_GRENADE, GLAUNCHER));
	EXPECT_TRUE(ValidLaunchable(GL_HE_GRENADE, UNDER_GLAUNCHER));

	// Check if the function handles some random garbage input
	EXPECT_FALSE(ValidLaunchable(BATTERIES, WINE));
	EXPECT_FALSE(ValidLaunchable(ItemId(0xf123), ItemId(0x97b2)));
}

TEST(Items, GetLauncherFromLaunchable)
{
	EXPECT_EQ(GetLauncherFromLaunchable(GL_TEARGAS_GRENADE), GLAUNCHER);
	EXPECT_EQ(GetLauncherFromLaunchable(MORTAR_SHELL), MORTAR);
	EXPECT_EQ(GetLauncherFromLaunchable(TANK_SHELL), TANK_CANNON);
	EXPECT_EQ(GetLauncherFromLaunchable(TANK_CANNON), NOTHING);

	// Check if the function handles some random garbage input
	EXPECT_EQ(GetLauncherFromLaunchable(G11), NOTHING);
	EXPECT_EQ(GetLauncherFromLaunchable(ItemId(0xe941)), NOTHING);
}

TEST(Items, ValidAttachment)
{
	std::unique_ptr<DefaultContentManager> cm(DefaultContentManagerUT::createDefaultCMForTesting());
	ASSERT_TRUE(cm->loadGameData());
	auto const oldGCM = std::exchange(GCM, cm.release());

	bool& extra_attachments = const_cast<GamePolicy *>(GCM->getGamePolicy())->extra_attachments;

	extra_attachments = false;
	EXPECT_TRUE(ValidAttachment(DETONATOR, HMX));
	EXPECT_TRUE(ValidAttachment(DETONATOR, TNT));
	EXPECT_TRUE(ValidAttachment(CHEWING_GUM, FUMBLE_PAK));
	EXPECT_FALSE(ValidAttachment(UVGOGGLES, SPECTRA_HELMET));;
	EXPECT_FALSE(ValidAttachment(SUNGOGGLES, SPECTRA_HELMET));;
	EXPECT_FALSE(ValidAttachment(ADRENALINE_BOOSTER, KEVLAR_LEGGINGS_Y));
	EXPECT_FALSE(ValidAttachment(AUTO_ROCKET_RIFLE, BRASS_KNUCKLES));
	EXPECT_FALSE(ValidAttachment(ItemId(0xf083), ItemId(0x8c12))); // Random junk crashes the old version of ValidAttachment

	// Next test relies on a certain order of the vests
	static_assert(SPECTRA_VEST_Y.inner() - FLAK_JACKET.inner() == 8);
	int count = 0;
	for (int i = FLAK_JACKET.inner(); i <= SPECTRA_VEST_Y.inner(); i++)
	{
		if (ValidAttachment(CERAMIC_PLATES, ItemId(i))) ++count;
	}
	EXPECT_EQ(count, 9);

	extra_attachments = true;
	EXPECT_TRUE(ValidAttachment(DETONATOR, HMX));
	EXPECT_TRUE(ValidAttachment(DETONATOR, TNT));
	EXPECT_TRUE(ValidAttachment(CHEWING_GUM, FUMBLE_PAK));
	EXPECT_TRUE(ValidAttachment(UVGOGGLES, SPECTRA_HELMET));;
	EXPECT_TRUE(ValidAttachment(UVGOGGLES, KEVLAR_HELMET_18));;
	EXPECT_TRUE(ValidAttachment(SUNGOGGLES, SPECTRA_HELMET));;
	EXPECT_TRUE(ValidAttachment(ADRENALINE_BOOSTER, KEVLAR_LEGGINGS_Y));
	EXPECT_FALSE(ValidAttachment(SUNGOGGLES, SPECTRA_VEST));;
	EXPECT_FALSE(ValidAttachment(AUTO_ROCKET_RIFLE, BRASS_KNUCKLES));
	EXPECT_FALSE(ValidAttachment(ItemId(0xf083), ItemId(0x8c12)));

	delete GCM;
	GCM = oldGCM;
}

TEST(Items, CompatibleFaceItem)
{
	EXPECT_TRUE(CompatibleFaceItem(NIGHTGOGGLES, EXTENDEDEAR));
	EXPECT_TRUE(CompatibleFaceItem(EXTENDEDEAR, NIGHTGOGGLES));
	EXPECT_FALSE(CompatibleFaceItem(EXTENDEDEAR, EXTENDEDEAR));
	EXPECT_TRUE(CompatibleFaceItem(WALKMAN, GASMASK));
	EXPECT_FALSE(CompatibleFaceItem(UVGOGGLES, RDX));
	EXPECT_TRUE(CompatibleFaceItem(ItemId(0xda83), NOTHING)); // item2 == NOTHING is a special case
	EXPECT_FALSE(CompatibleFaceItem(ItemId(0x75e4), ItemId(0xcafe)));
	for (int i = 0; i <= 0xffff; ++i)
	{
		EXPECT_FALSE(CompatibleFaceItem(ItemId(i), STEEL_HELMET));
	}
}

#endif
