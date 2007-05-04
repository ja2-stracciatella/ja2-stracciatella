#ifndef LOADSAVEMERCPROFILE_H
#define LOADSAVEMERCPROFILE_H

#include "FileMan.h"
#include "Soldier_Profile_Type.h"


void ExtractMercProfileUTF16(const BYTE* Src, MERCPROFILESTRUCT* Merc);
void ExtractMercProfile(const BYTE* Src, MERCPROFILESTRUCT* Merc);
BOOLEAN ExtractMercProfileFromFile(HWFILE File, MERCPROFILESTRUCT* Merc);

void InjectMercProfile(BYTE* Dst, const MERCPROFILESTRUCT* Merc);
BOOLEAN InjectMercProfileIntoFile(HWFILE File, const MERCPROFILESTRUCT* Merc);

#endif
