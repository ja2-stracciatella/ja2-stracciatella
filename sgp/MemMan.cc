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
}


// Shuts down the memory manager.
void ShutdownMemoryManager(void)
{
	if (MemDebugCounter != 0)
	{
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, " ");
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, "***** WARNING - WARNING - WARNING *****");
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, "***** WARNING - WARNING - WARNING *****");
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, "***** WARNING - WARNING - WARNING *****");
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, " ");
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, "  >>>>> MEMORY LEAK DETECTED!!! <<<<<  ");
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("%d memory blocks still allocated", MemDebugCounter));
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("%d bytes memory total STILL allocated", guiMemTotal));
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("%d bytes memory total was allocated", guiMemAlloced));
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, String("%d bytes memory total was freed", guiMemFreed));

		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, " ");
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, "***** WARNING - WARNING - WARNING *****");
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, "***** WARNING - WARNING - WARNING *****");
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, "***** WARNING - WARNING - WARNING *****");
		DebugMsg(TOPIC_MEMORY_MANAGER, DBG_LEVEL_0, " ");

#if defined JA2BETAVERSION
		FILE* fp = fopen("MemLeakInfo.txt", "a");
		if (fp)
		{
			fprintf(fp, "\n\n");
			fprintf(fp, ">>>>> MEMORY LEAK DETECTED!!! <<<<<\n");
			fprintf(fp, "  %d bytes memory total was allocated\n", guiMemAlloced);
			fprintf(fp, "- %d bytes memory total was freed\n", guiMemFreed);
			fprintf(fp, "_______________________________________________\n");
			fprintf(fp, "%d bytes memory total STILL allocated\n", guiMemTotal);
			fprintf(fp, "%d memory blocks still allocated\n", MemDebugCounter);
			fprintf(fp, "guiScreenExitedFrom = %ls\n", gzJA2ScreenNames[gMsgBox.uiExitScreen]);
			fprintf(fp, "\n\n");
			fclose(fp);
		}
#endif
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
