#include "Stubs.h"
#include "Types.h"


size_t GetPrivateProfileString(const char* app_name, const char* key_name, const char* default_value, char* out_buf, size_t buf_size, const char* filename)
{
	FIXME
	strlcpy(out_buf, default_value, buf_size);
	return strlen(out_buf);
}


void OutputDebugString(const char* str)
{
	fputs(str, stderr);
}
