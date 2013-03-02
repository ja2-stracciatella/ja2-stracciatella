#pragma once

/* Platform and compiler-specifics */


/**************************************************************
 * File system case-sensitivity
 *************************************************************/

#ifdef _WIN32
#define CASE_SENSITIVE_FS   0
#else
#define CASE_SENSITIVE_FS   1
#endif

/* #if CASE_SENSITIVE_FS */
/* #include <dirent.h> */
/* #endif */

/**************************************************************
 * Warnings
 *************************************************************/

/* Disable some warnings on Visual Studio */
#if defined(_MSC_VER)
#pragma warning( disable : 4018 )                       /* example: '>' : signed/unsigned mismatch */
#pragma warning( disable : 4200 )                       /* example: nonstandard extension used : zero-sized array in struct/union */
#pragma warning( disable : 4244 )                       /* example: conversion from 'int' to 'real', possible loss of data */
#pragma warning( disable : 4800 )                       /* example: 'const UINT32' : forcing value to bool 'true' or 'false' (performance warning) */
#pragma warning( disable : 4996 )                       /* example: 'wcscpy': This function or variable may be unsafe. Consider using wcscpy_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details. */
#endif

/**************************************************************
 * Misc
 *************************************************************/

#if defined(_MSC_VER)
  /* Visual Studio */
  #define __func__ __FUNCTION__
#else
  /* Not Visual Studio */
  #define __max(a, b) ((a) > (b) ? (a) : (b))
  #define __min(a, b) ((a) < (b) ? (a) : (b))
#endif

/**************************************************************
 *
 *************************************************************/
