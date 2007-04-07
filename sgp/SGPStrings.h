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

void WINsnprintf(char* restrict s, size_t n, const char* restrict format, ...);
void WINswprintf(wchar_t* restrict s, size_t n, const wchar_t* restrict format, ...);
void WINvswprintf(wchar_t* restrict s, size_t n, const wchar_t* restrict format, va_list arg);

#define snprintf  WINsnprintf
#define swprintf  WINswprintf
#define vswprintf WINvswprintf

#endif

#endif
