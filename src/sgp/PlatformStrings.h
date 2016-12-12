#pragma once

#include "Platform.h"

/* Platform and compiler-specifics */

/**************************************************************
 * String functions
 *************************************************************/

#if defined(_MSC_VER)
  #define strcasecmp      _stricmp
  #define strncasecmp     _strnicmp
#else
  #include <strings.h>
#endif

/**************************************************************
 *
 *************************************************************/
