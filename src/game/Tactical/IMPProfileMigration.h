#ifndef IMP_PROFILE_MIGRATION_H
#define IMP_PROFILE_MIGRATION_H

#include "Soldier_Profile_Type.h"
#include "Types.h"

#include <cstddef>
#include <optional>

/* An I.M.P. profile file carries the profile record as a dump of
 * MERCPROFILESTRUCT, so every version that moved a field in that record left
 * behind files no later version can read where they stand. Such a file is
 * brought forward a release at a time, the way a saved game is: each hop knows
 * only the layout on either side of it, and the next change to the record adds
 * a hop at the end rather than a path of its own from every version before it.
 *
 * No field has ever been dropped, so nothing is lost on the way. What a file
 * cannot give back is a string of 16 characters or more: an ST::string keeps
 * only its first 16 inside itself and the rest on the heap, and all the record
 * ever held of that heap was a pointer into the game that wrote it. */
/* The version a profile written by this build leads with. It is the record's
 * own, raised when the record's layout changes and on no other occasion. The
 * saved game version is raised whenever a save's contents are invalidated,
 * which happens for all manner of reasons that leave this record exactly
 * where it stood, and the two are counted apart for that reason. */
constexpr UINT32 IMP_PROFILE_VERSION = 1;

/* Every layout the record has ever had, oldest first. A name here says which
 * count it is a number on, because three of them meet in this list and a bare
 * number would not say which: the releases that wrote a format before profiles
 * carried a version at all, the saved game version that named the last of
 * those, and the profile version that names every format since.
 *
 * So do not call the next one V105 or anything like it. What follows
 * ProfileVersion1 is ProfileVersion2, whatever the saved game version happens
 * to be on the day the record changes. */
enum class IMPProfileFormat
{
	// Written before profiles carried a version: nothing inside such a file
	// says which layout its record is in, and only its length can.
	Release021,      // 0.21: the palette strings led the record
	Release022,      // 0.22: the portrait fields arrived and the record was reordered
	SaveVersion104,  // master, once an I.M.P. slot carried a voice of its own

	// Written behind a version, which is what says which layout it is in.
	ProfileVersion1, // the record as save version 104 left it, now led by a version
};

struct IMPProfileLayout
{
	IMPProfileFormat format;
	size_t recordOffset;
	size_t recordSize;
	size_t inventoryOffset;
};

/* What a profile file of this length holds, of those written before profiles
 * carried a version, or nothing at all when no versionless release ever wrote
 * a file that long. Such a file opens with the record itself and has nothing
 * in it to say which layout that record is in, so its length is the only
 * thing that can, and these lengths are facts about files already on disk
 * that stay as they are however the record changes afterwards.
 *
 * A file of any other length leads with a version, and it is the version, not
 * the length, that says which layout its record is in. That is the whole of
 * what the length is asked. */
std::optional<IMPProfileLayout> IMPProfileVersionlessLayoutOfSize(size_t fileSize);

/* The layout a version number stands for, or nothing at all when no build
 * ever wrote a profile of that version. This is the whole of what a version
 * means, and the assertion beside it is what keeps the two in step: raising
 * IMP_PROFILE_VERSION without naming the layout it stands for gives way
 * there. */
std::optional<IMPProfileLayout> IMPProfileLayoutOfVersion(UINT32 version);

/* Lifts a record of the given layout into the one this version uses. The
 * record must be as long as the layout says it is. */
MERCPROFILESTRUCT IMPProfileMigrate(IMPProfileFormat format, BYTE const* record);

#endif
