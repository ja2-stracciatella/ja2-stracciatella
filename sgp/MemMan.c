//**************************************************************************
//
// Filename :	MemMan.cpp
//
//	Purpose :	function definitions for the memory manager
//
// Modification history :
//
//		11sep96:HJH	- Creation
//    29may97:ARM - Fix & improve MemDebugCounter handling, logging of
//                    MemAlloc/MemFree, and reporting of any errors
//
//**************************************************************************

//**************************************************************************
//
//				Includes
//
//**************************************************************************

//#ifdef JA2_PRECOMPILED_HEADERS
//	#include "JA2 SGP ALL.H"
//#elif defined( WIZ8_PRECOMPILED_HEADERS )
//	#include "WIZ8 SGP ALL.H"
//#else
	#include "types.h"
	#include <windows.h>
	#include <malloc.h>
	#include <stdlib.h>
	#include <string.h>
	#include "MemMan.h"
	#include "Debug.h"
	#include <stdio.h>
	#ifdef _DEBUG
		#include <crtdbg.h>
	#endif
//#endif

#ifdef _DEBUG
	//#define DEBUG_MEM_LEAKS // turns on tracking of every MemAlloc and MemFree!
#endif

//**************************************************************************
//
//				Variables
//
//**************************************************************************

#ifdef JA2
#include "mousesystem.h"
#include "MessageBoxScreen.h"
STR16 gzJA2ScreenNames[] =
{
	L"EDIT_SCREEN",
	L"SAVING_SCREEN",
	L"LOADING_SCREEN",
	L"ERROR_SCREEN",
	L"INIT_SCREEN",
	L"GAME_SCREEN",
	L"ANIEDIT_SCREEN",
	L"PALEDIT_SCREEN",
	L"DEBUG_SCREEN",
	L"MAP_SCREEN",
	L"LAPTOP_SCREEN",
	L"LOADSAVE_SCREEN",
	L"MAPUTILITY_SCREEN",
	L"FADE_SCREEN",
	L"MSG_BOX_SCREEN",
	L"MAINMENU_SCREEN",
	L"AUTORESOLVE_SCREEN",
	L"SAVE_LOAD_SCREEN",
	L"OPTIONS_SCREEN",
	L"SHOPKEEPER_SCREEN",
	L"SEX_SCREEN",
	L"GAME_INIT_OPTIONS_SCREEN",
	L"DEMO_EXIT_SCREEN",
	L"INTRO_SCREEN",
	L"CREDIT_SCREEN",
#ifdef JA2BETAVERSION
	L"AIVIEWER_SCREEN",
	L"QUEST_DEBUG_SCREEN",
#endif
};
#endif

#ifdef EXTREME_MEMORY_DEBUGGING
	typedef struct MEMORY_NODE
	{
		PTR pBlock;
		struct MEMORY_NODE *next, *prev;
		UINT8	*pCode;
		UINT32 uiSize;
	}MEMORY_NODE;

	MEMORY_NODE  *gpMemoryHead = NULL;
	MEMORY_NODE  *gpMemoryTail = NULL;
	UINT32				guiMemoryNodes = 0;
	UINT32				guiTotalMemoryNodes = 0;
#endif

static BOOLEAN gfMemDebug = TRUE;
// debug variable for total memory currently allocated
UINT32	guiMemTotal = 0;
UINT32  guiMemAlloced = 0;
UINT32  guiMemFreed = 0;
UINT32	MemDebugCounter = 0;
BOOLEAN fMemManagerInit = FALSE;

//**************************************************************************
//
//				Function Prototypes
//
//**************************************************************************

void			DebugPrint( void );

//**************************************************************************
//
//				Functions
//
//**************************************************************************

//**************************************************************************
//
// MemInit
//
//
//
// Parameter List :
// Return Value :
// Modification history :
//
//		12sep96:HJH		-> modified for use by Wizardry
//
//**************************************************************************

BOOLEAN InitializeMemoryManager( void )
{
	// Register the memory manager with the debugger
	RegisterDebugTopic(TOPIC_MEMORY_MANAGER, "Memory Manager");
	MemDebugCounter = 0;
	guiMemTotal = 0;
	guiMemAlloced = 0;
	guiMemFreed = 0;
	fMemManagerInit = TRUE;

	#ifdef EXTREME_MEMORY_DEBUGGING
		gpMemoryHead = NULL;
		gpMemoryTail = NULL;
		guiMemoryNodes = 0;
		guiTotalMemoryNodes = 0;
	#endif

	return(TRUE);
}

//**************************************************************************
//
// MemDebug
//
//		To set whether or not we should print debug info.
//
// Parameter List :
// Return Value :
// Modification history :
//
//		12sep96:HJH		-> modified for use by Wizardry
//
//**************************************************************************

void MemDebug( BOOLEAN f )
{
	gfMemDebug = f;
}

//**************************************************************************
//
// MemShutdown
//
//		Shuts down the memory manager.
//
// Parameter List :
// Return Value :
// Modification history :
//
//		12sep96:HJH		-> modified for use by Wizardry
//
//**************************************************************************

void ShutdownMemoryManager( void )
{
	if ( MemDebugCounter != 0 )
	{
		DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String(" "));
		DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("***** WARNING - WARNING - WARNING *****"));
		DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("***** WARNING - WARNING - WARNING *****"));
		DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("***** WARNING - WARNING - WARNING *****"));
		DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String(" "));
		DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("  >>>>> MEMORY LEAK DETECTED!!! <<<<<  "));
		DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("%d memory blocks still allocated", MemDebugCounter ));
		DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("%d bytes memory total STILL allocated", guiMemTotal ));
		DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("%d bytes memory total was allocated", guiMemAlloced));
		DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("%d bytes memory total was freed", guiMemFreed));

		DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String(" "));
		DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("***** WARNING - WARNING - WARNING *****"));
		DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("***** WARNING - WARNING - WARNING *****"));
		DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("***** WARNING - WARNING - WARNING *****"));
		DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String(" "));

		#ifndef EXTREME_MEMORY_DEBUGGING
			#ifdef JA2BETAVERSION
			{
				FILE *fp;
				fp = fopen( "MemLeakInfo.txt", "a" );
				if( fp )
				{
					fprintf( fp, "\n\n" );
					fprintf( fp, ">>>>> MEMORY LEAK DETECTED!!! <<<<<\n" );
					fprintf( fp, "  %d bytes memory total was allocated\n", guiMemAlloced );
					fprintf( fp, "- %d bytes memory total was freed\n", guiMemFreed );
					fprintf( fp, "_______________________________________________\n" );
					fprintf( fp, "%d bytes memory total STILL allocated\n", guiMemTotal );
					fprintf( fp, "%d memory blocks still allocated\n", MemDebugCounter );
					fprintf( fp, "guiScreenExitedFrom = %S\n", gzJA2ScreenNames[ gMsgBox.uiExitScreen ] );
					fprintf( fp, "\n\n" );
				}
				fclose( fp );
			}
			#endif
		#endif
	}


	UnRegisterDebugTopic( TOPIC_MEMORY_MANAGER, "Memory Manager Un-initialized" );

	fMemManagerInit = FALSE;
}


#ifdef _DEBUG

PTR MemAllocReal( UINT32 uiSize, const char *pcFile, INT32 iLine )
{
	PTR	ptr;

	if( !uiSize )
	{
		return NULL;
	}

	if ( !fMemManagerInit )
    DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("MemAlloc: Warning -- Memory manager not initialized -- Line %d in %s", iLine, pcFile) );


	ptr = _malloc_dbg( uiSize, _NORMAL_BLOCK, pcFile, iLine );
  if (ptr != NULL)
  {
		guiMemTotal   += uiSize;
		guiMemAlloced += uiSize;
		MemDebugCounter++;
  }
  else
	{
    DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("MemAlloc failed: %d bytes (line %d file %s)", uiSize, iLine, pcFile) );
	}

#ifdef DEBUG_MEM_LEAKS
  DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_1, String("MemAlloc %p: %d bytes (line %d file %s)", ptr, uiSize, iLine, pcFile) );
#endif

	return( ptr );
}


void MemFreeReal( PTR ptr, const char *pcFile, INT32 iLine )
{
	UINT32 uiSize;

	if ( !fMemManagerInit )
    DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("MemFree: Warning -- Memory manager not initialized -- Line %d in %s", iLine, pcFile) );

  if (ptr != NULL)
  {
		uiSize = _msize(ptr);
		guiMemTotal -= uiSize;
		guiMemFreed += uiSize;
		_free_dbg( ptr, _NORMAL_BLOCK );

#ifdef DEBUG_MEM_LEAKS
	  DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_1, String("MemFree  %p: %d bytes (line %d file %s)", ptr, uiSize, iLine, pcFile) );
#endif
  }
  else
  {
    DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("MemFree ERROR: NULL ptr received (line %d file %s)", iLine, pcFile) );
  }

  // count even a NULL ptr as a MemFree, not because it's really a memory leak, but because it is still an error of some
  // sort (nobody should ever be freeing NULL pointers), and this will help in tracking it down if the above DbgMessage
  // is not noticed.
  MemDebugCounter--;
}


PTR MemReallocReal( PTR ptr, UINT32 uiSize, const char *pcFile, INT32 iLine )
{
	PTR	ptrNew;
	UINT32 uiOldSize;


	if ( !fMemManagerInit )
    DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("MemRealloc: Warning -- Memory manager not initialized -- Line %d in %s", iLine, pcFile) );

  if(ptr != NULL)
	{
		uiOldSize = _msize(ptr);
		guiMemTotal -= uiOldSize;
		guiMemFreed += uiOldSize;
	  MemDebugCounter--;
	}

	// Note that the ptr changes to ptrNew...
	ptrNew = _realloc_dbg( ptr, uiSize, _NORMAL_BLOCK, pcFile, iLine );
  if (ptrNew == NULL)
  {
		DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("MemReAlloc failed: ptr %d, %d -> %d bytes (line %d file %s)", ptr, uiOldSize, uiSize, iLine, pcFile) );
		if ( uiSize != 0 )
		{
			// ptr is left untouched, so undo the math above
			guiMemTotal += uiOldSize;
			guiMemFreed -= uiOldSize;
			MemDebugCounter++;
		}
  }
  else
  {
#ifdef DEBUG_MEM_LEAKS
  	DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_1, String("MemRealloc %p: Resizing %d bytes to %d bytes (line %d file %s) - New ptr %p", ptr, uiOldSize, uiSize, iLine, pcFile, ptrNew ) );
#endif

		guiMemTotal   += uiSize;
		guiMemAlloced += uiSize;
		MemDebugCounter++;
  }

	return( ptrNew );
}

#endif


PTR *MemAllocLocked( UINT32 uiSize )
{
	PTR	ptr;

	if ( !fMemManagerInit )
    DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("MemAllocLocked: Warning -- Memory manager not initialized!!! ") );


	ptr = VirtualAlloc( NULL, uiSize, MEM_COMMIT, PAGE_READWRITE );

	if ( ptr )
	{
    VirtualLock( ptr, uiSize );

		guiMemTotal   += uiSize;
		guiMemAlloced += uiSize;
		MemDebugCounter++;
  }
  else
	{
    DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("MemAllocLocked failed: %d bytes", uiSize) );
	}

#ifdef DEBUG_MEM_LEAKS
  DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_1, String("MemAllocLocked %p: %d bytes", ptr, uiSize) );
#endif

	return( ptr );
}


void MemFreeLocked( PTR ptr, UINT32 uiSize )
{
	if ( !fMemManagerInit )
    DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("MemFreeLocked: Warning -- Memory manager not initialized!!! ") );


  if (ptr != NULL)
  {
	  VirtualUnlock( ptr, uiSize );
	  VirtualFree( ptr, uiSize, MEM_RELEASE );

		guiMemTotal -= uiSize;
		guiMemFreed += uiSize;
  }
  else
  {
    DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("MemFreeLocked ERROR: NULL ptr received, size %d", uiSize) );
  }

  // count even a NULL ptr as a MemFree, not because it's really a memory leak, but because it is still an error of some
  // sort (nobody should ever be freeing NULL pointers), and this will help in tracking it down if the above DbgMessage
  // is not noticed.
  MemDebugCounter--;

#ifdef DEBUG_MEM_LEAKS
  DbgMessage( TOPIC_MEMORY_MANAGER, DBG_LEVEL_1, String("MemFreeLocked  %p", ptr) );
#endif
}


//**************************************************************************
//
// MemGetFree
//
//
//
// Parameter List :
// Return Value :
// Modification history :
//
//		??sep96:HJH		-> modified for use by Wizardry
//
//**************************************************************************

UINT32 MemGetFree( void )
{
	MEMORYSTATUS ms;

	ms.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus( &ms );

	return( ms.dwAvailPhys );
}


//**************************************************************************
//
// MemGetTotalSystem
//
//
//
// Parameter List :
// Return Value :
// Modification history :
//
//		May98:HJH		-> Carter
//
//**************************************************************************

UINT32 MemGetTotalSystem( void )
{
	MEMORYSTATUS ms;

	ms.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus( &ms );

	return( ms.dwTotalPhys );
}


//**************************************************************************
//
// MemCheckPool
//
//
//
// Parameter List :
// Return Value :
// Modification history :
//
//		23sep96:HJH		-> modified for use by Wizardry
//
//**************************************************************************

BOOLEAN MemCheckPool( void )
{
	BOOLEAN fRet = TRUE;

#ifdef _DEBUG

	fRet = _CrtCheckMemory();
	Assert( fRet );

#endif

	return(fRet);
}

#ifdef EXTREME_MEMORY_DEBUGGING

PTR MemAllocXDebug( UINT32 size, const char *szCodeString, INT32 iLineNum, void *pSpecial )
{
	PTR	ptr;
	UINT16 usLength;
	UINT8 str[70];
	UINT8 *pStr;

	if( !size )
	{
		return NULL;
	}

	if( !pSpecial  )
	{
		ptr = malloc( size );
	}
	else
	{
		ptr = pSpecial;
	}

  if( ptr )
  {
		// Set into video object list
		if( gpMemoryHead )
		{ //Add node after tail
			gpMemoryTail->next = (MEMORY_NODE*)malloc( sizeof( MEMORY_NODE ) );
			Assert( gpMemoryTail->next ); //out of memory?
			gpMemoryTail->next->prev = gpMemoryTail;
			gpMemoryTail->next->next = NULL;
			gpMemoryTail = gpMemoryTail->next;
		}
		else
		{ //new list
			gpMemoryHead = (MEMORY_NODE*)malloc( sizeof( MEMORY_NODE ) );
			Assert( gpMemoryHead ); //out of memory?
			gpMemoryHead->prev = gpMemoryHead->next = NULL;
			gpMemoryTail = gpMemoryHead;
		}

		//record the code location of the calling creating function.
		pStr = strrchr( szCodeString, '\\' );
		pStr++;
		sprintf( str, "%s -- line(%d)", pStr, iLineNum );
		usLength = strlen( str ) + 1;
		gpMemoryTail->pCode = (UINT8*)malloc( usLength );
		memset( gpMemoryTail->pCode, 0, usLength );
		strcpy( gpMemoryTail->pCode, str );

		//record the size
		gpMemoryTail->uiSize = size;

		//Set the hVObject into the node.
		gpMemoryTail->pBlock = ptr;

		guiMemoryNodes++;
		guiTotalMemoryNodes++;
	}
	return( ptr );
}

void MemFreeXDebug( PTR ptr, const char *szCodeString, INT32 iLineNum, void *pSpecial )
{
	MEMORY_NODE *curr;

  if( ptr )
  {
		curr = gpMemoryHead;
		while( curr )
		{
			if( curr->pBlock == ptr )
			{ //Found the node, so detach it and delete it.

				if( !pSpecial )
				{
					free( ptr );
				}

				if( curr == gpMemoryHead )
				{ //Advance the head, because we are going to remove the head node.
					gpMemoryHead = gpMemoryHead->next;
				}
				if( curr == gpMemoryTail )
				{ //Back up the tail, because we are going to remove the tail node.
					gpMemoryTail = gpMemoryTail->prev;
				}
				//Detach the node from the vobject list
				if( curr->next )
				{ //Make the prev node point to the next
					curr->next->prev = curr->prev;
				}
				if( curr->prev )
				{ //Make the next node point to the prev
					curr->prev->next = curr->next;
				}
				//The node is now detached.  Now deallocate it.
				free( curr );
				curr = NULL;
				guiMemoryNodes--;
				return;
			}
			curr = curr->next;
		}
  }
}

PTR	MemReallocXDebug( PTR ptr, UINT32 size, const char *szCodeString, INT32 iLineNum, void *pSpecial )
{
	MEMORY_NODE *curr;
	PTR	ptrNew;
	UINT16 usLength;
	UINT8 str[70];
	UINT8 *pStr;

	if( !ptr && size )
	{
		return MemAllocXDebug( size, szCodeString, iLineNum, pSpecial );
	}

	curr = gpMemoryHead;
	while( curr )
	{
		if( curr->pBlock == ptr )
		{
			// Note that the ptr changes to ptrNew...
			if( !pSpecial )
			{
				ptrNew = realloc( ptr, size );
			}
			else
			{
				ptrNew = pSpecial;
			}

			if( ptrNew )
			{
				curr->pBlock = ptrNew;
				curr->uiSize = size;

				free( curr->pCode );

				//record the code location of the calling creating function.
				pStr = strrchr( szCodeString, '\\' );
				pStr++;
				sprintf( str, "%s -- line(%d)", pStr, iLineNum );
				usLength = strlen( str ) + 1;
				curr->pCode = (UINT8*)malloc( usLength );
				memset( curr->pCode, 0, usLength );
				strcpy( curr->pCode, str );
			}
			else
			{
				ptr = ptr;
			}
			return ptrNew;
		}
		curr = curr->next;
	}
	return 0;
}

typedef struct DUMPFILENAME
{
	UINT8 str[70];
}DUMPFILENAME;

void DumpMemoryInfoIntoFile( UINT8 *filename, BOOLEAN fAppend )
{
	MEMORY_NODE *curr;
	FILE *fp;
	DUMPFILENAME *pCode;
	UINT32 *puiCounter, *puiSize;
	UINT8 tempCode[ 70 ];
	UINT32 i, uiUniqueID, uiTotalKbWasted = 0, uiBytesRemainder = 0;
	BOOLEAN fFound;

	if( fAppend )
	{
		fp = fopen( filename, "a" );
	}
	else
	{
		fp = fopen( filename, "w" );
	}
	Assert( fp );

	if( !guiMemoryNodes )
	{
		fprintf( fp, "NO MEMORY LEAKS DETECTED!  CONGRATULATIONS!\n" );
		fclose( fp );
		return;
	}

	//Allocate enough strings and counters for each node.
	pCode = (DUMPFILENAME*)malloc( sizeof( DUMPFILENAME ) * guiMemoryNodes );
	memset( pCode, 0, sizeof( DUMPFILENAME ) * guiMemoryNodes );
	puiSize = (UINT32*)malloc( 4 * guiMemoryNodes );
	memset( puiSize, 0, 4 * guiMemoryNodes );
	puiCounter = (UINT32*)malloc( 4 * guiMemoryNodes );
	memset( puiCounter, 0, 4 * guiMemoryNodes );

	//Loop through the list and record every unique filename and count them
	uiUniqueID = 0;
	curr = gpMemoryHead;
	while( curr )
	{
		strcpy( tempCode, curr->pCode );
		fFound = FALSE;
		for( i = 0; i < uiUniqueID; i++ )
		{
			if( !_stricmp( tempCode, pCode[i].str ) )
			{ //same string
				fFound = TRUE;
				(puiCounter[ i ])++;
				(puiSize[ i ]) += curr->uiSize;
				break;
			}
		}
		if( !fFound )
		{
			strcpy( pCode[i].str, tempCode );
			(puiSize[ i ]) += curr->uiSize;
			(puiCounter[ i ])++;
			uiUniqueID++;
		}
		curr = curr->next;
	}

	//Now dump the info.
	fprintf( fp, "--------------------------------------------------------------------------------\n" );
	fprintf( fp, "%d unique memory allocation locations exist in %d memory nodes\n", uiUniqueID, guiMemoryNodes );
	fprintf( fp, "--------------------------------------------------------------------------------\n" );
	for( i = 0; i < uiUniqueID; i++ )
	{
		fprintf( fp, "%d occurrences of %s (total size %d bytes)\n", puiCounter[i], pCode[i].str, puiSize[i] );
		uiBytesRemainder += puiSize[i];
		if( uiBytesRemainder >= 1024 )
		{
			uiTotalKbWasted += uiBytesRemainder/1024;
			uiBytesRemainder %= 1024;
		}
	}
	fprintf( fp, "--------------------------------------------------------------------------------\n" );
	fprintf( fp, "%dKB of memory total wasn't cleaned up!\n", uiTotalKbWasted );
	fprintf( fp, "--------------------------------------------------------------------------------\n" );

	fclose( fp );

	//Free all memory associated with this operation.
	free( pCode );
	free( puiCounter );
	free( puiSize );
}

BOOLEAN _AddAndRecordMemAlloc( UINT32 size, UINT32 uiLineNum, UINT8 *pSourceFile )
{
	return 0;
}

#endif
