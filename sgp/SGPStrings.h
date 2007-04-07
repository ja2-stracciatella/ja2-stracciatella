#ifndef SGPSTRINGS_H
#define SGPSTRINGS_H

#include <stdio.h> 
#include <string.h>
#include <strings.h>
#include <wchar.h>


#if defined(__linux__) || defined(_WIN32)

size_t wcslcpy(wchar_t* dst, const wchar_t* src, size_t size);
size_t strlcpy(char* dst, const char* src, size_t size);

#endif


#ifdef _WIN32

#include <stdarg.h>


void WINsnprintf(char* s, size_t n, const char* format, ...);
void WINswprintf(wchar_t* s, size_t n, const wchar_t* format, ...);
void WINvswprintf(wchar_t* s, size_t n, const wchar_t* format, va_list arg);

#define snprintf  WINsnprintf
#define swprintf  WINswprintf
#define vswprintf WINvswprintf

// XXX this is not ideal as it drops the context, but should work
#define wcstok(str, sep, last) wcstok(str, sep)

#endif

#endif
