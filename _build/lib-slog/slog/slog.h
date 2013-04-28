/****************************************************************************
 *
 * This file is part of SLOG (Simple Logger for C and C++).
 *
 * Copyright (c) 2013 SLOG authors and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif


/** @brief Console logging destinations. */
typedef enum
{
  SLOG_STDOUT,          /**< Print to STDOUT */
  SLOG_STDERR,          /**< Print to STDERR */
  SLOG_NONE             /**< Don't print to console. */
}
SLOGConsole;


/** @brief Logging levels. */
typedef enum
{
  SLOG_DEBUG,
  SLOG_INFO,
  SLOG_WARNING,
  SLOG_ERROR
}
SLOGLevel;


/** @brief Library initialization.
 *
 * This function must be called only once.
 *
 * @param console Destination for console logging.  Can be SLOG_STDOUT, SLOG_STDERR, SLOG_NONE.
 * @param logFile Path to the file for storing logs.  NULL when no logging to file is required.
 */
void SLOG_Init(SLOGConsole console, const char *logFile);


/** @brief Library deinitialization.
 *
 * This function should be called before program exit only once. */
void SLOG_Deinit(void);


/** @brief Set logging level for console and file logging. */
void SLOG_SetLevel(SLOGLevel console, SLOGLevel file);


#ifndef SLOG_DISABLED

  /** @brief Main logging function.
   *
   * This function should not be used directly.
   * Use SLOG* macroses instead.
   *
   * @param level  Severity level of the message.
   * @param tag    Tag of the message, e.g. subsystem name.
   * @param format Format string (the same as for printf) */
  void SLOG_LogMessage(SLOGLevel level, const char *tag, const char *format, ...);


  /** Print debug message macro. */
  #define SLOGD(TAG, FORMAT, ...) SLOG_LogMessage(SLOG_DEBUG, TAG, FORMAT "\n", ##__VA_ARGS__);

  /** Print info message macro. */
  #define SLOGI(TAG, FORMAT, ...) SLOG_LogMessage(SLOG_INFO,  TAG, FORMAT "\n", ##__VA_ARGS__);

  /** Print warning message macro. */
  #define SLOGW(TAG, FORMAT, ...) SLOG_LogMessage(SLOG_WARNING, TAG, FORMAT "\n", ##__VA_ARGS__);

  /** Print error message macro. */
  #define SLOGE(TAG, FORMAT, ...) SLOG_LogMessage(SLOG_ERROR, TAG, FORMAT "\n", ##__VA_ARGS__);

#else

  /* logging disabled */

  /** Empty debug message macro. */
  #define SLOGD(TAG, FORMAT, ...)

  /** Empty info message macro. */
  #define SLOGI(TAG, FORMAT, ...)

  /** Empty warning message macro. */
  #define SLOGW(TAG, FORMAT, ...)

  /** Empty error message macro. */
  #define SLOGE(TAG, FORMAT, ...)

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
