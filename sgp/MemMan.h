//**************************************************************************
//
// Filename :	MemMan.h
//
//	Purpose :	prototypes for the memory manager
//
// Modification history :
//
//		11sep96:HJH				- Creation
//
//**************************************************************************

#ifndef _MEMMAN_H
#define _MEMMAN_H

//**************************************************************************
//
//				Includes
//
//**************************************************************************

#include "Types.h"

//**************************************************************************
//
//				Defines
//
//**************************************************************************

//**************************************************************************
//
//				Typedefs
//
//**************************************************************************

//**************************************************************************
//
//				Function Prototypes
//
//**************************************************************************


#ifdef __cplusplus
extern "C" {
#endif

extern UINT32	MemDebugCounter;
extern UINT32 guiMemTotal;
extern UINT32 guiMemAlloced;
extern UINT32 guiMemFreed;

extern BOOLEAN	InitializeMemoryManager( void );
extern void		MemDebug( BOOLEAN f );
extern void		ShutdownMemoryManager( void );

// Creates and adds a video object to list
#ifdef EXTREME_MEMORY_DEBUGGING
	//This is the most effective way to debug memory leaks.  Each memory leak will be recorded in a linked
	//list containing a string referring to the location in code the memory was allocated in addition to
	//the number of occurrences.  The shutdown code will report all unhandled memory with exact location allocated.
	void DumpMemoryInfoIntoFile( UINT8 *filename, BOOLEAN fAppend );
	BOOLEAN _AddAndRecordMemAlloc( UINT32 size, UINT32 uiLineNum, UINT8 *pSourceFile );
	#define		MemAlloc( size )			MemAllocXDebug( (size), __FILE__, __LINE__, NULL )
	#define		MemFree( ptr )				MemFreeXDebug( (ptr), __FILE__, __LINE__, NULL )
	#define		MemRealloc( ptr, size )	MemReallocXDebug( (ptr), (size), __FILE__, __LINE__, NULL )
	extern PTR		MemAllocXDebug( UINT32 size, const char *szCodeString, INT32 iLineNum, void *pSpecial );
	extern void		MemFreeXDebug( PTR ptr, const char *szCodeString, INT32 iLineNum, void *pSpecial );
	extern PTR		MemReallocXDebug( PTR ptr, UINT32 size, const char *szCodeString, INT32 iLineNum, void *pSpecial );
#else
	#ifdef _DEBUG
		//This is another debug feature.  Not as sophistocated, but definately not the pig the extreme system is.
		//This system reports all memory allocations/deallocations in the debug output.
		#define		MemAlloc( size )			MemAllocReal( (size), __FILE__, __LINE__ )
		#define		MemFree( ptr )				MemFreeReal( (ptr), __FILE__, __LINE__ )
		#define		MemRealloc( ptr, size )	MemReallocReal( (ptr), (size), __FILE__, __LINE__ )
		extern PTR		MemAllocReal( UINT32 size, const char *, INT32 );
		extern void		MemFreeReal( PTR ptr, const char *, INT32  );
		extern PTR		MemReallocReal( PTR ptr, UINT32 size, const char *, INT32 );
	#else
		//Release build verison
		#include <stdlib.h>
		#define		MemAlloc( size )				malloc( (size) )
		#define		MemFree( ptr )					free( (ptr) )
		#define		MemRealloc( ptr, size )	realloc( (ptr), (size) )
	#endif
#endif



extern PTR     *MemAllocLocked( UINT32 size );
extern void		MemFreeLocked( PTR, UINT32 size );

// get total free on the system at this moment
extern UINT32	MemGetFree( void );

// get the total on the system
extern UINT32 MemGetTotalSystem( void );

extern BOOLEAN	MemCheckPool( void );

#ifdef __cplusplus
}
#endif


#endif
