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

#include "slog/slog.h"
#define DEBUG_TAG_MEMORY	"Memory Manager"


#undef _DEBUG // XXX TODO

#ifdef _DEBUG
//#	define DEBUG_MEM_LEAKS // turns on tracking of every MemAlloc and MemFree!
#endif


#ifdef JA2
#	include "MouseSystem.h"
#	include "MessageBoxScreen.h"

static const wchar_t* const gzJA2ScreenNames[] =
{
	L"EDIT_SCREEN",
	L"?",
	L"?",
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
#	ifdef JA2BETAVERSION
	L"AIVIEWER_SCREEN",
	L"QUEST_DEBUG_SCREEN",
#	endif
};
#endif

#ifdef EXTREME_MEMORY_DEBUGGING
struct MEMORY_NODE
{
	PTR pBlock;
	MEMORY_NODE* next;
	char* pCode;
	size_t uiSize;
};

static MEMORY_NODE* gpMemoryHead        = NULL;
static MEMORY_NODE* gpMemoryTail        = NULL;
static UINT32       guiMemoryNodes      = 0;
static UINT32       guiTotalMemoryNodes = 0;
#endif

// debug variable for total memory currently allocated
static size_t guiMemTotal      = 0;
static size_t guiMemAlloced    = 0;
static size_t guiMemFreed      = 0;
static UINT32 MemDebugCounter  = 0;
static BOOLEAN fMemManagerInit = FALSE;


void InitializeMemoryManager(void)
{
	MemDebugCounter = 0;
	guiMemTotal     = 0;
	guiMemAlloced   = 0;
	guiMemFreed     = 0;
	fMemManagerInit = TRUE;

#ifdef EXTREME_MEMORY_DEBUGGING
	gpMemoryHead        = NULL;
	gpMemoryTail        = NULL;
	guiMemoryNodes      = 0;
	guiTotalMemoryNodes = 0;
#endif
}


// Shuts down the memory manager.
void ShutdownMemoryManager(void)
{
	if (MemDebugCounter != 0)
	{
		SLOGE(DEBUG_TAG_MEMORY, "Memory leak detected: \n\
					%d memory blocks still allocated\n\
					%d bytes memory total STILL allocated\n\
					%d bytes memory total was allocated\n\
					%d bytes memory total was freed",
					MemDebugCounter, guiMemTotal, guiMemAlloced, guiMemFreed);
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


#ifdef _DEBUG

PTR MemAllocReal(size_t uiSize, const char* pcFile, INT32 iLine)
{
	if (uiSize == 0)
	{
		return NULL;
	}

	if (!fMemManagerInit)
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("MemAlloc: Warning -- Memory manager not initialized -- Line %d in %s", iLine, pcFile));


	PTR ptr = _malloc_dbg(uiSize, _NORMAL_BLOCK, pcFile, iLine);
	if (!ptr)
	{
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("MemAlloc failed: %d bytes (line %d file %s)", uiSize, iLine, pcFile));
		throw std::bad_alloc();
	}

	guiMemTotal   += uiSize;
	guiMemAlloced += uiSize;
	MemDebugCounter++;

#ifdef DEBUG_MEM_LEAKS
	DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_1, String("MemAlloc %p: %d bytes (line %d file %s)", ptr, uiSize, iLine, pcFile));
#endif

	return ptr;
}


void MemFreeReal(PTR ptr, const char* pcFile, INT32 iLine)
{
	if (!fMemManagerInit)
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("MemFree: Warning -- Memory manager not initialized -- Line %d in %s", iLine, pcFile));

	if (ptr != NULL)
	{
		UINT32 uiSize = _msize(ptr);
		guiMemTotal -= uiSize;
		guiMemFreed += uiSize;
		_free_dbg(ptr, _NORMAL_BLOCK);

#ifdef DEBUG_MEM_LEAKS
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_1, String("MemFree  %p: %d bytes (line %d file %s)", ptr, uiSize, iLine, pcFile));
#endif
	}
	else
	{
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("MemFree ERROR: NULL ptr received (line %d file %s)", iLine, pcFile));
	}

	/* count even a NULL ptr as a MemFree, not because it's really a memory leak,
	 * but because it is still an error of some sort (nobody should ever be
	 * freeing NULL pointers), and this will help in tracking it down if the
	 * above DebugMsg is not noticed. */
	MemDebugCounter--;
}


PTR MemReallocReal(PTR ptr, UINT32 uiSize, const char* pcFile, INT32 iLine)
{
	if (!fMemManagerInit)
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("MemRealloc: Warning -- Memory manager not initialized -- Line %d in %s", iLine, pcFile));

	UINT32 uiOldSize = 0;
	if (ptr != NULL)
	{
		uiOldSize = _msize(ptr);
		guiMemTotal -= uiOldSize;
		guiMemFreed += uiOldSize;
		MemDebugCounter--;
	}

	// Note that the ptr changes to ptrNew...
	PTR ptrNew = _realloc_dbg(ptr, uiSize, _NORMAL_BLOCK, pcFile, iLine);
	if (ptrNew == NULL)
	{
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("MemReAlloc failed: ptr %d, %d -> %d bytes (line %d file %s)", ptr, uiOldSize, uiSize, iLine, pcFile));
		if (uiSize != 0)
		{
			// ptr is left untouched, so undo the math above
			guiMemTotal += uiOldSize;
			guiMemFreed -= uiOldSize;
			MemDebugCounter++;
		}
		throw std::bad_alloc();
	}

#ifdef DEBUG_MEM_LEAKS
	DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_1, String("MemRealloc %p: Resizing %d bytes to %d bytes (line %d file %s) - New ptr %p", ptr, uiOldSize, uiSize, iLine, pcFile, ptrNew));
#endif
	guiMemTotal   += uiSize;
	guiMemAlloced += uiSize;
	MemDebugCounter++;

	return ptrNew;
}

#endif


#ifdef EXTREME_MEMORY_DEBUGGING

PTR MemAllocXDebug(size_t size, const char* szCodeString, INT32 iLineNum)
{
	if (size == 0)
	{
		return NULL;
	}

	void* const ptr = XMalloc(size);

	// Set into video object list
	MEMORY_NODE* Node = malloc(sizeof(*Node));
	Assert(Node); // out of memory?
	Node->next = NULL;
	if (gpMemoryTail)
	{ //Add node after tail
		gpMemoryTail->next = Node;
	}
	else
	{ //new list
		gpMemoryHead = Node;
	}
	gpMemoryTail = Node;

	//record the code location of the calling creating function.
	const char* pStr = strrchr(szCodeString, '/');
	pStr = (pStr ? pStr + 1 : szCodeString);
	char str[70];
	sprintf(str, "%s -- line(%d)", pStr, iLineNum);
	gpMemoryTail->pCode = strdup(str);

	//record the size
	gpMemoryTail->uiSize = size;

	//Set the hVObject into the node.
	gpMemoryTail->pBlock = ptr;

	guiMemoryNodes++;
	guiTotalMemoryNodes++;
	return ptr;
}

void MemFreeXDebug(PTR ptr, const char* szCodeString, INT32 iLineNum)
{
	if (!ptr) return;

	MEMORY_NODE* prev = NULL;
	for (MEMORY_NODE* curr = gpMemoryHead; curr; prev = curr, curr = curr->next)
	{
		if (curr->pBlock != ptr) continue;

		//Found the node, so detach it and delete it.
		free(ptr);

		if (curr == gpMemoryHead)
		{ //Advance the head, because we are going to remove the head node.
			gpMemoryHead = gpMemoryHead->next;
		}
		if (curr == gpMemoryTail)
		{ //Back up the tail, because we are going to remove the tail node.
			gpMemoryTail = prev;
		}
		//Detach the node from the vobject list
		if (prev)
		{ //Make the next node point to the prev
			prev->next = curr->next;
		}
		//The node is now detached.  Now deallocate it.
		free(curr);
		curr = NULL;
		guiMemoryNodes--;
		return;
	}
}


PTR	MemReallocXDebug(PTR ptr, size_t size, const char* szCodeString, INT32 iLineNum)
{
	if (!ptr && size)
	{
		return MemAllocXDebug(size, szCodeString, iLineNum);
	}

	for (MEMORY_NODE* curr = gpMemoryHead; curr; curr = curr->next)
	{
		if (curr->pBlock != ptr) continue;

		// Note that the ptr changes to ptrNew...
		void* const ptrNew = XRealloc(ptr, size);

		curr->pBlock = ptrNew;
		curr->uiSize = size;

		free(curr->pCode);

		//record the code location of the calling creating function.
		const char* pStr = strrchr(szCodeString, '/');
		pStr = (pStr ? pStr + 1 : szCodeString);
		char str[70];
		sprintf(str, "%s -- line(%d)", pStr, iLineNum);
		curr->pCode = strdup(str);
		return ptrNew;
	}
	throw std::logic_error("Tried to reallocate with invalid pointer");
}


struct DUMPINFO
{
	size_t Size;
	UINT32 Counter;
	char   Code[70];
};


void DumpMemoryInfoIntoFile(const char* filename, BOOLEAN fAppend)
{
	FILE* fp = fopen(filename, fAppend ? "a" : "w");
	Assert(fp);

	if (!guiMemoryNodes)
	{
		fprintf(fp, "NO MEMORY LEAKS DETECTED!  CONGRATULATIONS!\n");
		fclose(fp);
		return;
	}

	DUMPINFO* Info = calloc(guiMemoryNodes, sizeof(*Info));

	UINT32 uiUniqueID = 0;
	for (const MEMORY_NODE* curr = gpMemoryHead; curr; curr = curr->next)
	{
		const char* Code = curr->pCode;
		BOOLEAN fFound = FALSE;
		for (UINT32 i = 0; i < uiUniqueID; i++)
		{
			if (strcasecmp(Code, Info[i].Code) == 0)
			{ //same string
				fFound = TRUE;
				Info[i].Counter++;
				Info[i].Size += curr->uiSize;
				break;
			}
		}
		if (!fFound)
		{
			strcpy(Info[uiUniqueID].Code, Code);
			Info[uiUniqueID].Size    = curr->uiSize;
			Info[uiUniqueID].Counter = 1;
			uiUniqueID++;
		}
	}

	//Now dump the info.
	fprintf(fp, "--------------------------------------------------------------------------------\n");
	fprintf(fp, "%d unique memory allocation locations exist in %d memory nodes\n", uiUniqueID, guiMemoryNodes);
	fprintf(fp, "--------------------------------------------------------------------------------\n");
	size_t TotalWasted = 0;
	for (UINT32 i = 0; i < uiUniqueID; i++)
	{
		fprintf(fp, "%d occurrences of %s (total size %d bytes)\n", Info[i].Counter, Info[i].Code, Info[i].Size);
		TotalWasted += Info[i].Size;
	}
	fprintf(fp, "--------------------------------------------------------------------------------\n");
	fprintf(fp, "%dKB of memory total wasn't cleaned up!\n", TotalWasted / 1024);
	fprintf(fp, "--------------------------------------------------------------------------------\n");

	fclose(fp);
	free(Info);
}

#endif
