#pragma once

/** Debug information logger. */

/************************************************************
 * General logging
 ***********************************************************/

/** Available log levels. */
enum
{
  LOGLEVEL_DEBUG,               /**< Log everything */
  LOGLEVEL_INFO,                /**< Log info messages, warnings and errors */
  LOGLEVEL_WARNING,             /**< Log warnings and errors */
  LOGLEVEL_ERROR,               /**< Log only errors */
};

/** Current log level. */
#define LOG_LEVEL       (LOGLEVEL_INFO)

#if LOG_LEVEL >= LOGLEVEL_WARNING
#define LOG_WARNING                     printf
#else
#define LOG_WARNING                     /* empty function name */
#endif

#if LOG_LEVEL >= LOGLEVEL_DEBUG
#define LOG_DEBUG                       printf
#else
#define LOG_DEBUG                       /* empty function name */
#endif

/************************************************************
 *
 ***********************************************************/
