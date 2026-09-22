#ifndef LOADSAVEMERCPROFILE_H
#define LOADSAVEMERCPROFILE_H

#include "Soldier_Profile_Type.h"

#define MERC_PROFILE_SIZE               (716)           /**< Vanilla Merc profile size */
#define MERC_PROFILE_SIZE_STRAC_LINUX   (796)           /**< Stracciatella Linux Merc profile size */
#define IMP_SAVED_PROFILE_SIZE          (1409)          /**< Saved I.M.P. profile file size */

struct OBJECTTYPE;

/**
* Extract merc profile from the binary data. */
void ExtractMercProfile(BYTE const* const Src, MERCPROFILESTRUCT& p, bool stracLinuxFormat, UINT32 *checksum, bool const isCorrectlyEncoded);

/** Calculates soldier profile checksum. */
UINT32 SoldierProfileChecksum(MERCPROFILESTRUCT const& p);

/** Extract IMP merc profile from file.
* If saved checksum is not correct, exception will be thrown. */
void ExtractImpProfileFromFile(SGPFile *file, INT32 *iProfileId, INT32 *iPortraitNumber, MERCPROFILESTRUCT& p);

void InjectMercProfile(BYTE* Dst, MERCPROFILESTRUCT const&);
void InjectMercProfileIntoFile(HWFILE, MERCPROFILESTRUCT const&);

/** Write a saved I.M.P. profile: the profile and the NUM_INV_SLOTS objects of
 * its inventory, in IMP_SAVED_PROFILE_SIZE bytes. */
void InjectIMPSavedProfile(BYTE* Dst, MERCPROFILESTRUCT const&, OBJECTTYPE const* inv);

/** Whether data of the given size is a saved I.M.P. profile that leads with a
 * version, as every one written since the version existed does. */
bool IMPSavedProfileHasVersion(BYTE const* Src, size_t size);

/** Read a saved I.M.P. profile of the given size into the profile and the
 * NUM_INV_SLOTS objects of its inventory.
 * Throws std::runtime_error when it is not one this game can read. */
void ExtractIMPSavedProfile(BYTE const* Src, size_t size, MERCPROFILESTRUCT&, OBJECTTYPE* inv);

#endif
