// -*-coding: utf-8-unix;-*-

#include "gtest/gtest.h"

#include "IMPProfileMigration.h"
#include "Soldier_Profile_Type.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

/* The records are built here by hand, at the offsets the releases put their
 * fields at, rather than through the structs the migration describes them
 * with: a test that asked those structs where a field goes would agree with
 * itself whatever they said. The offsets below are read off files the
 * releases actually wrote. */

namespace
{

// 0.21 led its record with the portrait and the palette strings
size_t const V21_NAME = 0;
size_t const V21_NICKNAME = 32;
size_t const V21_FACE_INDEX = 64;
size_t const V21_PANTS = 72;
size_t const V21_VEST = 104;
size_t const V21_SKIN = 136;
size_t const V21_HAIR = 168;
size_t const V21_SEX = 200;
size_t const V21_RECORD = 648;

// 0.22 put the character's own particulars first and the strings after
size_t const V22_NAME = 0;
size_t const V22_NICKNAME = 32;
size_t const V22_SEX = 64;
size_t const V22_FACE_INDEX = 76;
size_t const V22_PANTS = 96;
size_t const V22_RECORD = 664;

size_t const INVENTORY = 684;

/* An ST::string as a record holds it: a pointer into the game that wrote the
 * file, the length, and as much of the text as fits inside the string. */
void PutString(std::vector<BYTE>& record, size_t const offset, ST::string const& text,
	size_t const claimedSize)
{
	// whatever this address pointed at is long gone; nothing may follow it
	uint64_t const stalePointer = 0x00007f9e1d228f18ULL;
	uint64_t const size = claimedSize;
	std::memcpy(record.data() + offset, &stalePointer, sizeof(stalePointer));
	std::memcpy(record.data() + offset + 8, &size, sizeof(size));
	size_t const inlineRoom = 16;
	std::memcpy(record.data() + offset + 16, text.c_str(), std::min(text.size(), inlineRoom));
}

void PutString(std::vector<BYTE>& record, size_t const offset, ST::string const& text)
{
	PutString(record, offset, text, text.size());
}

void PutByte(std::vector<BYTE>& record, size_t const offset, UINT8 const value)
{
	record[offset] = value;
}

}

TEST(IMPProfileMigrationTest, everyLengthAVersionlessReleaseWroteIsKnown)
{
	auto const v21 = IMPProfileVersionlessLayoutOfSize(V21_RECORD + INVENTORY);
	ASSERT_TRUE(v21.has_value());
	EXPECT_EQ(v21->format, IMPProfileFormat::Release021);
	EXPECT_EQ(v21->recordOffset, 0u);
	EXPECT_EQ(v21->inventoryOffset, V21_RECORD);

	auto const v22 = IMPProfileVersionlessLayoutOfSize(V22_RECORD + INVENTORY);
	ASSERT_TRUE(v22.has_value());
	EXPECT_EQ(v22->format, IMPProfileFormat::Release022);
	EXPECT_EQ(v22->recordOffset, 0u);
	EXPECT_EQ(v22->inventoryOffset, V22_RECORD);

	auto const v104 = IMPProfileVersionlessLayoutOfSize(sizeof(MERCPROFILESTRUCT) + INVENTORY);
	ASSERT_TRUE(v104.has_value());
	EXPECT_EQ(v104->format, IMPProfileFormat::SaveVersion104);
	EXPECT_EQ(v104->recordOffset, 0u);
	EXPECT_EQ(v104->inventoryOffset, sizeof(MERCPROFILESTRUCT));
}

/* The length a file that leads with a version has is not one of these, and
 * must not be: were it taken for a versionless length, the version it leads
 * with would never be read, and the record it stands for never told apart
 * from a record of any other version the same length. */
TEST(IMPProfileMigrationTest, aFileThatLeadsWithAVersionIsNotKnownByItsLength)
{
	EXPECT_FALSE(IMPProfileVersionlessLayoutOfSize(
		sizeof(UINT32) + sizeof(MERCPROFILESTRUCT) + INVENTORY).has_value());
}

TEST(IMPProfileMigrationTest, aVersionNamesTheLayoutItStandsFor)
{
	auto const v1 = IMPProfileLayoutOfVersion(1);
	ASSERT_TRUE(v1.has_value());
	EXPECT_EQ(v1->format, IMPProfileFormat::ProfileVersion1);
	EXPECT_EQ(v1->recordOffset, sizeof(UINT32));
	EXPECT_EQ(v1->recordSize, sizeof(MERCPROFILESTRUCT));
	EXPECT_EQ(v1->inventoryOffset, sizeof(UINT32) + sizeof(MERCPROFILESTRUCT));
}

/* The version this build writes has to be one it can also read back, which is
 * the whole point of writing it down. */
TEST(IMPProfileMigrationTest, theVersionThisBuildWritesIsOneItKnows)
{
	EXPECT_TRUE(IMPProfileLayoutOfVersion(IMP_PROFILE_VERSION).has_value());
}

TEST(IMPProfileMigrationTest, aVersionNoBuildEverWroteIsTurnedAway)
{
	EXPECT_FALSE(IMPProfileLayoutOfVersion(0).has_value());
	EXPECT_FALSE(IMPProfileLayoutOfVersion(IMP_PROFILE_VERSION + 1).has_value());
	EXPECT_FALSE(IMPProfileLayoutOfVersion(104).has_value());
	EXPECT_FALSE(IMPProfileLayoutOfVersion(0xFFFFFFFF).has_value());
}

TEST(IMPProfileMigrationTest, aLengthNoVersionlessReleaseWroteIsTurnedAway)
{
	EXPECT_FALSE(IMPProfileVersionlessLayoutOfSize(0).has_value());
	EXPECT_FALSE(IMPProfileVersionlessLayoutOfSize(V21_RECORD).has_value());
	EXPECT_FALSE(IMPProfileVersionlessLayoutOfSize(V21_RECORD + INVENTORY - 1).has_value());
	EXPECT_FALSE(IMPProfileVersionlessLayoutOfSize(V21_RECORD + INVENTORY + 1).has_value());
	EXPECT_FALSE(IMPProfileVersionlessLayoutOfSize(1000000).has_value());
}

TEST(IMPProfileMigrationTest, aProfileFromV21ComesForwardWhole)
{
	std::vector<BYTE> record(V21_RECORD, 0);
	PutString(record, V21_NAME, "Ned Lee");
	PutString(record, V21_NICKNAME, "Needle");
	PutString(record, V21_PANTS, "BLACKPANTS");
	PutString(record, V21_VEST, "WHITEVEST");
	PutString(record, V21_SKIN, "BLACKSKIN");
	PutString(record, V21_HAIR, "BROWNHEAD");
	PutByte(record, V21_FACE_INDEX, 200);
	PutByte(record, V21_SEX, MALE);

	MERCPROFILESTRUCT const p = IMPProfileMigrate(IMPProfileFormat::Release021, record.data());

	EXPECT_EQ(p.zName, "Ned Lee");
	EXPECT_EQ(p.zNickname, "Needle");
	EXPECT_EQ(p.PANTS, "BLACKPANTS");
	EXPECT_EQ(p.VEST, "WHITEVEST");
	EXPECT_EQ(p.SKIN, "BLACKSKIN");
	EXPECT_EQ(p.HAIR, "BROWNHEAD");
	EXPECT_EQ(p.ubFaceIndex, 200);
	EXPECT_EQ(p.bSex, MALE);
}

TEST(IMPProfileMigrationTest, aProfileFromV22ComesForwardWhole)
{
	std::vector<BYTE> record(V22_RECORD, 0);
	PutString(record, V22_NAME, "Ned Lee");
	PutString(record, V22_NICKNAME, "Needle");
	PutString(record, V22_PANTS, "BLACKPANTS");
	PutByte(record, V22_FACE_INDEX, 200);
	PutByte(record, V22_SEX, FEMALE);

	MERCPROFILESTRUCT const p = IMPProfileMigrate(IMPProfileFormat::Release022, record.data());

	EXPECT_EQ(p.zName, "Ned Lee");
	EXPECT_EQ(p.zNickname, "Needle");
	EXPECT_EQ(p.PANTS, "BLACKPANTS");
	EXPECT_EQ(p.ubFaceIndex, 200);
	EXPECT_EQ(p.bSex, FEMALE);
}

TEST(IMPProfileMigrationTest, whatAnOlderRecordNeverCarriedIsLeftAtItsDefault)
{
	std::vector<BYTE> record(V21_RECORD, 0);
	PutString(record, V21_NICKNAME, "Needle");

	MERCPROFILESTRUCT const p = IMPProfileMigrate(IMPProfileFormat::Release021, record.data());

	// the slot is only held once the player confirms the character
	EXPECT_EQ(p.impSlotState, IMPSlotState::FREE);
	// the voice is the caller's to give: the record has nothing to say about it
	EXPECT_EQ(p.ubVoiceId, 0);
}

TEST(IMPProfileMigrationTest, aStringTooLongToHaveBeenWrittenDownIsGivenUp)
{
	std::vector<BYTE> record(V21_RECORD, 0);
	// a name of 16 characters or more lived on the heap; the file kept only
	// the pointer, so there is nothing here to bring forward
	PutString(record, V21_NAME, "Ned Lee Of Arnhem", 17);
	PutString(record, V21_NICKNAME, "Needle");

	MERCPROFILESTRUCT const p = IMPProfileMigrate(IMPProfileFormat::Release021, record.data());

	EXPECT_TRUE(p.zName.empty());
	EXPECT_EQ(p.zNickname, "Needle");
}

TEST(IMPProfileMigrationTest, aStringThatJustFitsIsKept)
{
	std::vector<BYTE> record(V21_RECORD, 0);
	PutString(record, V21_NAME, "Ned Lee Of Arnh");  // fifteen, the most that fits

	MERCPROFILESTRUCT const p = IMPProfileMigrate(IMPProfileFormat::Release021, record.data());

	EXPECT_EQ(p.zName, "Ned Lee Of Arnh");
}
