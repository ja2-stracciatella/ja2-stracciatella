#pragma once

#include "StringEncodingTypes.h"
#include "Types.h"

#include <string_theory/string>

ST::string LoadEncryptedString(SGPFile* File, uint32_t seek_chars, uint32_t read_chars);
ST::string LoadEncryptedData(ST::string& err_msg, STRING_ENC_TYPE encType, SGPFile* File, UINT32 seek_chars, UINT32 read_chars);
