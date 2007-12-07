#ifndef STUBS_H
#define STUBS_H

#include <stddef.h>


#define FILE_ATTRIBUTE_DIRECTORY 0 /* XXX */


size_t GetPrivateProfileString(const char* app_name, const char* key_name, const char* default_value, char* out_buf, size_t buf_size, const char* filename);


void OutputDebugString(const char*);

#endif
