#ifndef LOADSAVEMERCPROFILE_H
#define LOADSAVEMERCPROFILE_H

#include "Soldier_Profile_Type.h"
#include "IEncodingCorrector.h"

#define MERC_PROFILE_SIZE               (716)           /**< Vanilla Merc profile size */
#define MERC_PROFILE_SIZE_STRAC_LINUX   (796)           /**< Stracciatella Linux Merc profile size */

/**
* Extract merc profile from the binary data.
* @param encodingCorrection Perform encoding correction - it is necessary for loading strings from the game data files. */
void ExtractMercProfile(BYTE const* const Src, MERCPROFILESTRUCT& p, bool stracLinuxFormat, UINT32 *checksum,
			const IEncodingCorrector *fixer);

/** Calculates soldier profile checksum. */
UINT32 SoldierProfileChecksum(MERCPROFILESTRUCT const& p);

/** Extract IMP merc profile from file.
* If saved checksum is not correct, exception will be thrown. */
void ExtractImpProfileFromFile(SGPFile *file, INT32 *iProfileId, INT32 *iPortraitNumber, MERCPROFILESTRUCT& p);

void InjectMercProfile(BYTE* Dst, MERCPROFILESTRUCT const&);
void InjectMercProfileIntoFile(HWFILE, MERCPROFILESTRUCT const&);

/** Load raw merc profiles.
* @param f Open file with profile data.
* @param numProfiles Number of profiles to load
* @param profiles Array for storing profile data */
void LoadRawMercProfiles(HWFILE const f, int numProfiles, MERCPROFILESTRUCT *profiles, const IEncodingCorrector *fixer);

#endif
