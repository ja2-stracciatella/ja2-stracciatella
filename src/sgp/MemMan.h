// Purpose: prototypes for the memory manager
//
// Modification history :
//		11sep96:HJH				- Creation

#ifndef _MEMMAN_H
#define _MEMMAN_H

#include "Types.h"

#include <algorithm>
#include <stdlib.h>

#define MemAlloc(size)        XMalloc((size))
#define MemFree(ptr)          free((ptr))
#define MemRealloc(ptr, size) XRealloc((ptr), (size))

void InitializeMemoryManager(void);
void ShutdownMemoryManager(void);
void* XMalloc(size_t size);
void* XRealloc(void* ptr, size_t size);

static inline void* MallocZ(const size_t n)
{
	void* const p = MemAlloc(n);
	std::fill_n(static_cast<uint8_t*>(p), n, 0);
	return p;
}

template<typename T> static inline void FreeNull(T*& r) throw()
{
	T* const p = r;
	if (!p) return;
	r = 0;
	delete p;
}

template<typename T> static inline void FreeNullArray(T*& r) throw()
{
	T* const p = r;
	if (!p) return;
	r = 0;
	delete[] p;
}

#define MALLOC(type)             (type*)MemAlloc(sizeof(type))
#define MALLOCE(type, member, n) (type*)MemAlloc(sizeof(type) + sizeof(*((type*)0)->member) * (n))
#define MALLOCN(type, count)     (type*)MemAlloc(sizeof(type) * (count))
#define MALLOCNZ(type, count)    (type*)MallocZ(sizeof(type) * (count))
#define MALLOCZ(type)            (type*)MallocZ(sizeof(type))
#define REALLOC(ptr, type, count) (type*)MemRealloc(ptr, sizeof(type) * (count))

#endif /* _MEMMAN_H */
