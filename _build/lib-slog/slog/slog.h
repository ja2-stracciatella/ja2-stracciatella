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

#define NUMBER_OF_TOPICS  59
#define TAG_LENGTH        15

typedef enum
{
  DEBUG_TAG_GAMELOOP,
  DEBUG_TAG_SMAP,
  DEBUG_TAG_AI,
  DEBUG_TAG_SCHEDULER,
  DEBUG_TAG_PATHAI,
  DEBUG_TAG_ANIMATIONS,
  DEBUG_TAG_GAP,
  DEBUG_TAG_WEAPONS,
  DEBUG_TAG_OVERHEAD,
  DEBUG_TAG_KEYS,
  DEBUG_TAG_POINTS,
  DEBUG_TAG_MORALE,
  DEBUG_TAG_MERCHIRE,
  DEBUG_TAG_OPPLIST,
  DEBUG_TAG_LIGHTING,
  DEBUG_TAG_RENDERWORLD,
  DEBUG_TAG_TILES,
  DEBUG_TAG_EXPLOSION,
  DEBUG_TAG_PHYSICS,
  DEBUG_TAG_AMBIENT,
  DEBUG_TAG_SAVELOAD,
  DEBUG_TAG_MUSICCTL,
  DEBUG_TAG_SMK,
  DEBUG_TAG_EVENTPUMP,
  DEBUG_TAG_QUESTS,
  DEBUG_TAG_EDITOR,
  DEBUG_TAG_RESOURCES,
  DEBUG_TAG_JA2SCREENS,
  DEBUG_TAG_INIT,
  DEBUG_TAG_FONT,
  DEBUG_TAG_SGP,
  DEBUG_TAG_STCI,
  DEBUG_TAG_CONTAINER,
  DEBUG_TAG_VSURFACE,
  DEBUG_TAG_SOUND,
  DEBUG_TAG_MEMORY,
  DEBUG_TAG_GAMESCREEN,
  DEBUG_TAG_BOBBYRAY,
  DEBUG_TAG_SAI,
  DEBUG_TAG_AIRRAID,
  DEBUG_TAG_BULLETS,
  DEBUG_TAG_HANDLEITEMS,
  DEBUG_TAG_INTERFACE,
  DEBUG_TAG_LOS,
  DEBUG_TAG_TACTSAVE,
  DEBUG_TAG_TEAMTURN,
  DEBUG_TAG_WORLDDEF,
  DEBUG_TAG_HIMAGE,
  DEBUG_TAG_LIBDB,
  DEBUG_TAG_FILEMAN,
  DEBUG_TAG_MODPACK,
  DEBUG_TAG_DEFAULTCM,
  DEBUG_TAG_SOLDIER,
  DEBUG_TAG_ASSERTS,
  DEBUG_TAG_QUEENCMD,
  DEBUG_TAG_NPC,
  DEBUG_TAG_CAMPAIGN,
  DEBUG_TAG_LOYALTY,
  DEBUG_TAG_AUTORESOLVE
}
SLOGTopics;

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
  void SLOG_LogMessage(SLOGLevel level, SLOGTopics topic, const char *format, ...);


  /** Print debug message macro. */
  #define SLOGD(TAG, FORMAT, ...) SLOG_LogMessage(SLOG_DEBUG, TAG, FORMAT "\n", ##__VA_ARGS__)

  /** Print info message macro. */
  #define SLOGI(TAG, FORMAT, ...) SLOG_LogMessage(SLOG_INFO,  TAG, FORMAT "\n", ##__VA_ARGS__)

  /** Print warning message macro. */
  #define SLOGW(TAG, FORMAT, ...) SLOG_LogMessage(SLOG_WARNING, TAG, FORMAT "\n", ##__VA_ARGS__)

  /** Print error message macro. */
  #define SLOGE(TAG, FORMAT, ...) SLOG_LogMessage(SLOG_ERROR, TAG, FORMAT "\n", ##__VA_ARGS__)

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
