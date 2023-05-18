#pragma once

/* Platform and compiler-specifics */


/**************************************************************
 * File system case-sensitivity
 *************************************************************/

#ifdef _WIN32
#define CASE_SENSITIVE_FS   0
#define PATH_SEPARATOR      '\\'
#define PATH_SEPARATOR_STR  "\\"
#else
#define CASE_SENSITIVE_FS   1
#define PATH_SEPARATOR      '/'
#define PATH_SEPARATOR_STR  "/"
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#define SOURCE_ROOT ("src\\")
#else
#define SOURCE_ROOT ("src/")
#endif


/**************************************************************
 * Misc
 *************************************************************/

#if defined(_MSC_VER)
	/* Visual Studio */
	#define __func__ __FUNCTION__
#endif

#include <algorithm> // std::min, std::max

/**************************************************************
 *
 *************************************************************/
