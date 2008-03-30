// Purpose: prototypes for the memory manager
//
// Modification history :
//		11sep96:HJH				- Creation

#ifndef _MEMMAN_H
#define _MEMMAN_H

#include "Types.h"


#ifdef __cplusplus
extern "C" {
#endif

extern size_t guiMemTotal;

BOOLEAN InitializeMemoryManager(void);
void    ShutdownMemoryManager(void);

// Creates and adds a video object to list
#if defined EXTREME_MEMORY_DEBUGGING
/* This is the most effective way to debug memory leaks.  Each memory leak
 * will be recorded in a linked list containing a string referring to the
 * location in code the memory was allocated in addition to the number of
 * occurrences.  The shutdown code will report all unhandled memory with
 * exact location allocated. */
void DumpMemoryInfoIntoFile(const char* filename, BOOLEAN fAppend);
#	define MemAlloc(size)        MemAllocXDebug((size), __FILE__, __LINE__)
#	define MemFree(ptr)          MemFreeXDebug((ptr), __FILE__, __LINE__)
#	define MemRealloc(ptr, size) MemReallocXDebug((ptr), (size), __FILE__, __LINE__)
PTR  MemAllocXDebug(size_t size, const char* szCodeString, INT32 iLineNum);
void MemFreeXDebug(PTR ptr, const char* szCodeString, INT32 iLineNum);
PTR  MemReallocXDebug(PTR ptr, size_t size, const char* szCodeString, INT32 iLineNum);
#else
#	if defined _DEBUG && 0 // XXX TODO
/* This is another debug feature.  Not as sophistocated, but definately not the
 * pig the extreme system is.  This system reports all memory
 * allocations/deallocations in the debug output. */
#		define MemAlloc(size)        MemAllocReal((size), __FILE__, __LINE__)
#		define MemFree(ptr)          MemFreeReal((ptr), __FILE__, __LINE__)
#		define MemRealloc(ptr, size) MemReallocReal((ptr), (size), __FILE__, __LINE__)
PTR  MemAllocReal(UINT32 size, const char*, INT32);
void MemFreeReal(PTR ptr, const char*, INT32);
PTR  MemReallocReal(PTR ptr, UINT32 size, const char*, INT32);
#	else
// Release build version
#		include <stdlib.h>
#		define MemAlloc(size)        malloc((size))
#		define MemFree(ptr)          free((ptr))
#		define MemRealloc(ptr, size) realloc((ptr), (size))
#	endif
#endif

#define MALLOC(type)         (type*)MemAlloc(sizeof(type))
#define MALLOCE(type, extra) (type*)MemAlloc(sizeof(type) + (extra))
#define MALLOCN(type, count) (type*)MemAlloc(sizeof(type) * (count))

#define REALLOC(ptr, type, count) (type*)MemRealloc(ptr, sizeof(type) * (count))

#ifdef __cplusplus
}
#endif

#endif
