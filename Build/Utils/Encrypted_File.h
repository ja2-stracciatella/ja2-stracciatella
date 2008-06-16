#ifndef ENCRYPTED_FILE_H
#define ENCRYPTED_FILE_H

#include "Types.h"

void LoadEncryptedData(HWFILE, wchar_t* DestString, UINT32 seek_chars, UINT32 read_chars);
void LoadEncryptedDataFromFile(char const* Filename, wchar_t DestString[], UINT32 seek_chars, UINT32 read_chars);

#endif
