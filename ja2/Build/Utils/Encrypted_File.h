#ifndef __ENCRYPTED_H_
#define __ENCRYPTED_H_

#include "Types.h"

BOOLEAN LoadEncryptedData(HWFILE hFile, STR16 pDestString, UINT32 uiSeekFrom, UINT32 uiSeekAmount);
BOOLEAN LoadEncryptedDataFromFile(const char* Filename, wchar_t DestString[], UINT32 Seek, UINT32 ReadBytes);

#endif
