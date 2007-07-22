#ifndef ENCRYPTED_FILE_H
#define ENCRYPTED_FILE_H

#include "Types.h"

BOOLEAN LoadEncryptedData(HWFILE File, wchar_t* DestString, UINT32 Seek, UINT32 ReadBytes);
BOOLEAN LoadEncryptedDataFromFile(const char* Filename, wchar_t DestString[], UINT32 Seek, UINT32 ReadBytes);

#endif
