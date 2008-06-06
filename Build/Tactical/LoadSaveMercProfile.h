#ifndef LOADSAVEMERCPROFILE_H
#define LOADSAVEMERCPROFILE_H

#include "Soldier_Profile_Type.h"


void ExtractMercProfileUTF16(const BYTE* Src, MERCPROFILESTRUCT* Merc);
void ExtractMercProfile(const BYTE* Src, MERCPROFILESTRUCT* Merc);
void ExtractMercProfileFromFile(HWFILE, MERCPROFILESTRUCT*);

void InjectMercProfile(BYTE* Dst, const MERCPROFILESTRUCT* Merc);
void InjectMercProfileIntoFile(HWFILE, MERCPROFILESTRUCT const*);

#endif
