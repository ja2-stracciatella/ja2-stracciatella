#include "Debug.h"
#include <stdarg.h>
#include "Types.h"

const char* String(const char* const fmt, ...)
{
	static char TmpDebugString[8][512];
	static UINT StringIndex = 0;

	// Record string index. This index is used since we live in a multitasking environment.
	// It is still not bulletproof, but it's better than a single string
	char* ResultString = TmpDebugString[StringIndex];
	StringIndex = (StringIndex + 1) % lengthof(TmpDebugString);

	va_list ArgPtr;
	va_start(ArgPtr, fmt);
	vsprintf(ResultString, fmt, ArgPtr);
	va_end(ArgPtr);

	return ResultString;
}
