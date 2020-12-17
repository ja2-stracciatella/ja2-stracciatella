#include "Logger.h"
#include "SGPStrings.h"
#include "Types.h"

#include <string_theory/string>

#include <stdio.h>
#if defined(_MSC_VER)
#define vsnprintf(buf, size, format, args) vsnprintf_s(buf, size, _TRUNCATE, format, args)
#endif

void LogMessage(bool isAssert, LogLevel level, const char* file, const ST::string& str)
{
	Logger_log(level, str.c_str(), file);

	#ifdef ENABLE_ASSERTS
	if (isAssert)
	{
		abort();
	}
	#endif
}

void LogMessage(bool isAssert, LogLevel level, const char *file, const char *format, ...)
{
	char message[256];
	va_list args;
	va_start(args, format);
	vsnprintf(message, 256, format, args);
	va_end(args);

	ST::string err_msg;
	ST::string str = st_checked_buffer_to_string(err_msg, ST::char_buffer(message, lengthof(message)));
	if (!err_msg.empty())
	{
		STLOGW("LogMessage: {}", err_msg);
	}
	LogMessage(isAssert, level, file, str);
}
