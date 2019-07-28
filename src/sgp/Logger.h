#pragma once

#include "RustInterface.h"

#define NUMBER_OF_TOPICS  63
#define TAG_LENGTH        15

#define __FILENAME__ (__FILE__ + SOURCE_PATH_SIZE)

void LogMessage(LogLevel level, const char *file, const char *format, ...);

/** Print debug message macro. */
#define SLOGD(TAG, FORMAT, ...) LogMessage(LogLevel::Debug, __FILENAME__, FORMAT, ##__VA_ARGS__)

/** Print info message macro. */
#define SLOGI(TAG, FORMAT, ...) LogMessage(LogLevel::Info,  __FILENAME__, FORMAT, ##__VA_ARGS__)

/** Print warning message macro. */
#define SLOGW(TAG, FORMAT, ...) LogMessage(LogLevel::Warn, __FILENAME__, FORMAT, ##__VA_ARGS__)

/** Print error message macro. */
#define SLOGE(TAG, FORMAT, ...) LogMessage(LogLevel::Error, __FILENAME__, FORMAT, ##__VA_ARGS__)