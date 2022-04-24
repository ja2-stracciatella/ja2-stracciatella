//	Purpose :	function definitions for the memory manager
//
// Modification history :
//
//		11sep96:HJH	- Creation
//    29may97:ARM - Fix & improve MemDebugCounter handling, logging of
//                    MemAlloc/MemFree, and reporting of any errors
#include <stdexcept>
#include <new> // std::bad_alloc

#include "Types.h"
#include <stdlib.h>
#include "MemMan.h"
#include "Debug.h"
#include "Logger.h"
#include "MouseSystem.h"
#include "MessageBoxScreen.h"

// debug variable for total memory currently allocated
static size_t guiMemAlloced    = 0;
static size_t guiMemFreed      = 0;
static UINT32 MemDebugCounter  = 0;
static BOOLEAN fMemManagerInit = FALSE;


void InitializeMemoryManager(void)
{
	MemDebugCounter = 0;
	guiMemAlloced   = 0;
	guiMemFreed     = 0;
	fMemManagerInit = TRUE;
}


// Shuts down the memory manager.
void ShutdownMemoryManager(void)
{
	if (MemDebugCounter != 0)
	{
		SLOGE("Memory leak detected: \n\
					{} memory blocks still allocated\n\
					{} bytes memory total was allocated\n\
					{} bytes memory total was freed",
					MemDebugCounter, guiMemAlloced, guiMemFreed);
	}
	fMemManagerInit = FALSE;
}

void* XMalloc(size_t const size)
{
	void* const p = malloc(size);
	if (!p) throw std::bad_alloc();
	return p;
}

void* XRealloc(void* const ptr, size_t const size)
{
	void* const p = realloc(ptr, size);
	if (!p) throw std::bad_alloc();
	return p;
}

/// Allocate memory in C++.
///
/// This is a global replacement of `operator new`.
/// The other versions of `operator new` call this replacement by default in C++11.
///
/// @see https://en.cppreference.com/w/cpp/memory/new/operator_new
void* operator new(std::size_t size)
{
	return MemAlloc(size);
}

/// Deallocate memory in C++.
///
/// This is a global replacement of `operator delete`.
/// The other versions of `operator delete` call this replacement by default in C++11.
///
/// @see https://en.cppreference.com/w/cpp/memory/new/operator_delete
void operator delete(void* ptr) noexcept
{
	MemFree(ptr);
}
