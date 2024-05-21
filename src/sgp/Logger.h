#ifndef SGP_LOGGER_H_
#define SGP_LOGGER_H_

#include "Platform.h"
#include "RustInterface.h"
#include <string_view>
#include <string_theory/format>
#include <string_theory/string>

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

template<typename... Args>
constexpr void LogMessageST([[maybe_unused]] bool isAssert, LogLevel level, const char* file, Args && ... args)
{
	if (level <= Logger_getLevel()) {
		Logger_log(level, ST::format(std::forward<Args>(args)...).c_str(), file);
	}

	#ifdef ENABLE_ASSERTS
	if (isAssert)
	{
		abort();
	}
	#endif
}

/** Print debug message macro. */
#define SLOGD(...) LogMessageST(false, LogLevel::Debug, __FILENAME__, ##__VA_ARGS__)

/** Print info message macro. */
#define SLOGI(...) LogMessageST(false, LogLevel::Info,  __FILENAME__, ##__VA_ARGS__)

/** Print warning message macro. */
#define SLOGW(...) LogMessageST(false, LogLevel::Warn, __FILENAME__, ##__VA_ARGS__)

/** Print error message macro. */
#define SLOGE(...) LogMessageST(false, LogLevel::Error, __FILENAME__, ##__VA_ARGS__)

/** Print error message macro and assert if ENABLE_ASSERTS is defined. */
#define SLOGA(...) LogMessageST(true, LogLevel::Error, __FILENAME__, ##__VA_ARGS__)

#endif//SGP_LOGGER_H_
