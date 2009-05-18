#ifndef SGPSTRINGS_H
#define SGPSTRINGS_H

#include <cwchar>
#include <stdio.h>
#include <string.h>
#include <strings.h>


#if defined(__linux__) || defined(_WIN32)

size_t wcslcpy(wchar_t* dst, const wchar_t* src, size_t size);
size_t strlcpy(char* dst, const char* src, size_t size);

#endif


#ifdef _WIN32

#include <stdarg.h>


int WINsnprintf(char* s, size_t n, const char* fmt, ...);
int WINswprintf(wchar_t* s, size_t n, const wchar_t* fmt, ...);
int WINvswprintf(wchar_t* s, size_t n, const wchar_t* fmt, va_list arg);

#define snprintf  WINsnprintf
#define swprintf  WINswprintf
#define vswprintf WINvswprintf

#endif

/*
 * Replace the path and/or extension of a filename.
 * If path is not null, the path of filename is replaced by path, otherwise it
 * is kept.  The extension of filename is replaced by ext.
 */
void ReplacePath(char* buf, size_t size, char const* path, char const* filename, char const* ext);

#endif
