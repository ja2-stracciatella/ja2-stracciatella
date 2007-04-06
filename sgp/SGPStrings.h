#ifndef SGPSTRINGS_H
#define SGPSTRINGS_H

#include <stdio.h> 
#include <string.h>
#include <strings.h>
#include <wchar.h>


#ifdef __linux__

size_t wcslcpy(wchar_t* dst, const wchar_t* src, size_t size);
size_t strlcpy(char* dst, const char* src, size_t size);

#endif

#endif
