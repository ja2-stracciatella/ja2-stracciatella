#ifndef SGP_LOGGER_H_
#define SGP_LOGGER_H_

#include "Platform.h"
#include "RustInterface.h"
#include <string_view>
#include <string_theory/string>
#include <stracciatella.h>

void LogMessage(bool isAssert, LogLevel level, const char* file, const ST::string& str);
void LogMessage(bool isAssert, LogLevel level, const char *file, const char *format, ...);

/** Get filename relative to src directory */
constexpr size_t GetSourcePathSize(const char* filename)
{
	std::string_view s(filename);
	size_t i = s.rfind(SOURCE_ROOT);

	return (i != std::string::npos)
		? i + std::string_view(SOURCE_ROOT).length()
		: 0;
}

template <size_t p> constexpr const char* ToRelativePath(const char* filename)
{	// this forces compile-time evaluation of p; we don't have consteval yet
	return &filename[p];
}

// Determine the src path at compile time, for performant log calls to Rust that use non-absolute file name
#define SOURCE_PATH_SIZE (GetSourcePathSize(__FILE__))
#define __FILENAME__ (ToRelativePath<SOURCE_PATH_SIZE>(__FILE__))

/** Print debug message macro. */
#define SLOGD(FORMAT, ...) LogMessage(false, LogLevel::Debug, __FILENAME__, FORMAT, ##__VA_ARGS__)

/** Print info message macro. */
#define SLOGI(FORMAT, ...) LogMessage(false, LogLevel::Info,  __FILENAME__, FORMAT, ##__VA_ARGS__)

/** Print warning message macro. */
#define SLOGW(FORMAT, ...) LogMessage(false, LogLevel::Warn, __FILENAME__, FORMAT, ##__VA_ARGS__)

/** Print error message macro. */
#define SLOGE(FORMAT, ...) LogMessage(false, LogLevel::Error, __FILENAME__, FORMAT, ##__VA_ARGS__)

/** Print error message macro. */
#define SLOGA(FORMAT, ...) LogMessage(true, LogLevel::Error, __FILENAME__, FORMAT, ##__VA_ARGS__)

#endif//SGP_LOGGER_H_
