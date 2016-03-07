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

/************************************************************
 *
 ***********************************************************/
