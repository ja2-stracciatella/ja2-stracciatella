#include "Logger.h"

#include <stdio.h>
#if defined(_MSC_VER)
  /* Visual Studio */
  #include <io.h>
  #define write _write
  #define snprintf(buf, size, format, ...) _snprintf_s(buf, size, _TRUNCATE, format, ##__VA_ARGS__)
  #define vsnprintf(buf, size, format, args) vsnprintf_s(buf, size, _TRUNCATE, format, args)
  #pragma warning( disable : 4996 )  /* disable some VS warnings, e.g. 'fopen': This function or variable may be unsafe. */
#else
  #include <unistd.h>
#endif

void LogMessage(LogLevel level, const char *file, const char *format, ...) {
  char message[256];
  va_list args;
  va_start(args, format);
  vsnprintf(message, 256, format, args);
  va_end(args);

  Logger_LogWithCustomMetadata(level, message, file);

  #ifdef ENABLE_ASSERTS
    if (tag == DEBUG_TAG_ASSERTS)
    {
      abort();
    }
  #endif
}