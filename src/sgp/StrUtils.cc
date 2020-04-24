#include "StrUtils.h"

#include <stdarg.h>
#include <cstdio>

#include "PlatformStrings.h"

/** Build formatted string. */
ST::string FormattedString(const char* fmt, ...)
{
	char buf[512];
	va_list ArgPtr;
	va_start(ArgPtr, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ArgPtr);
	va_end(ArgPtr);

	buf[sizeof(buf) - 1] = 0;

	return ST::string(buf);
}
