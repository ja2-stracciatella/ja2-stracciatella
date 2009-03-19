#ifndef LOADSAVEMERCPROFILE_H
#define LOADSAVEMERCPROFILE_H

#include "Soldier_Profile_Type.h"


void ExtractMercProfileUTF16(BYTE const* Src, MERCPROFILESTRUCT&);
void ExtractMercProfile(BYTE const* Src, MERCPROFILESTRUCT&);
void ExtractMercProfileFromFile(HWFILE, MERCPROFILESTRUCT&);

void InjectMercProfile(BYTE* Dst, MERCPROFILESTRUCT const&);
void InjectMercProfileIntoFile(HWFILE, MERCPROFILESTRUCT const&);

#endif
