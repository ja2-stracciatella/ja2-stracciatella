#ifndef IMP_PROFILE_MIGRATION_H
#define IMP_PROFILE_MIGRATION_H

#include "Item_Types.h"
#include "Soldier_Profile_Type.h"
#include "Types.h"

#include <cstddef>
#include <optional>

/* An I.M.P. profile file was once a copy of MERCPROFILESTRUCT as it lay in
 * memory, and every version that moved a field in that struct left behind files
 * no later version could read where they stand. Since profiles carried a
 * version they have been written field by field instead (see
 * InjectIMPSavedProfile), so this is a closed list: it brings forward the files
 * of before, and a change to the profile record never needs to come here.
 *
 * Only files from 64-bit builds are known. A 32-bit build laid the same struct
 * out smaller, and its files are refused.
 *
 * No field has ever been dropped, so nothing is lost on the way. What a file
 * cannot give back is a string of 16 characters or more: an ST::string keeps
 * only its first 16 inside itself and the rest on the heap, and all the record
 * ever held of that heap was a pointer into the game that wrote it. */

/* Every layout a profile file had before profiles carried a version, oldest
 * first. Nothing inside such a file says which layout its record is in, and
 * only its length can. */
enum class IMPProfileFormat
{
	Release021,      // 0.21: the palette strings led the record
	Release022,      // 0.22: the portrait fields arrived and the record was reordered
	SaveVersion104,  // master, once an I.M.P. slot carried a voice of its own
};

struct IMPProfileLayout
{
	IMPProfileFormat format;
	size_t recordOffset;
	size_t recordSize;
	size_t inventoryOffset;
};

/* What a profile file of this length holds, of those written before profiles
 * carried a version, or nothing at all when no such file was ever that long.
 * These lengths are facts about files already on disk, and stay as they are
 * however the record changes afterwards. */
std::optional<IMPProfileLayout> IMPProfileVersionlessLayoutOfSize(size_t fileSize);

/* Lifts a record of the given layout into the one this version uses. The
 * record must be as long as the layout says it is. */
MERCPROFILESTRUCT IMPProfileMigrate(IMPProfileFormat format, BYTE const* record);

/* Reads the NUM_INV_SLOTS objects of the inventory beside such a record, which
 * has never changed shape, into inv. Items this game does not know are
 * dropped. */
void IMPProfileMigrateInventory(BYTE const* inventory, OBJECTTYPE* inv);

#endif
