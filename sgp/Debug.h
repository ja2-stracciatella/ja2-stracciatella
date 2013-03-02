#ifndef __DEBUG_MANAGER_
#define __DEBUG_MANAGER_

#include "Types.h"
#include "TopicIDs.h"


#if defined ( SGP_DEBUG ) || defined ( FORCE_ASSERTS_ON )

// If DEBUG_ is defined, we need to initialize all the debug macros. Otherwise all the
// debug macros will be substituted by blank lines at compile time
//*******************************************************************************************
// Debug Mode
//*******************************************************************************************

//Modified the Assertion code.  As of the writing of this code, there are no other functions that
//make use of _FailMessage.  With that assumption made, we can then make two functions, the first Assert, taking
//one argument, and passing a NULL string.  The second one, AssertMsg(), accepts a string as the second parameter.
//This string that has vanished for Assert is now built inside of fail message.  This is the case for both Asserts, but the second one
//also is added.  Ex:
//Assert( pointer );
//Assert( pointer, "This pointer is null and you tried to access it in function A ");
//It'll make debugging a little simpler.  In anal cases, you could build the string first, then assert
//with it.
extern	void _FailMessage(const char *pString, UINT32 uiLineNum, const char *pSourceFile);

#define Assert(a)       (a) ? (void)0 : _FailMessage(NULL, __LINE__, __FILE__)
#define AssertMsg(a, b) (a) ? (void)0 : _FailMessage(b   , __LINE__, __FILE__)

#else

#define Assert(a)       ((void)0)
#define AssertMsg(a, b) ((void)0)

#endif


#ifdef SGP_DEBUG

BOOLEAN	InitializeDebugManager(void);

// If DEBUG_ is defined, we need to initialize all the debug macros. Otherwise all the
// debug macros will be substituted by blank lines at compile time
//*******************************************************************************************
// Debug Mode
//*******************************************************************************************

// These are the debug macros (the ones the use will use). The user should never call
// the actual debug functions directly

enum DebugLevel
{
	DBG_LEVEL_0,
	DBG_LEVEL_1, // for basic stuff
	DBG_LEVEL_2, // for ordinary, I usually want to see them, messages
	DBG_LEVEL_3  // nitty gritty detail
};

extern void DebugMsg(TopicID TopicId, DebugLevel uiDebugLevel, const char* Str);
#define FastDebugMsg(a) _DebugMessage(a, __LINE__, __FILE__)

extern void _DebugMessage(const char* Message, UINT32 uiLineNum, const char* SourceFile);

const char* String(const char* fmt, ...);

#else

//*******************************************************************************************
// Release Mode
//*******************************************************************************************

#define DebugMsg(a, b, c)
#define FastDebugMsg(a)

#endif

#endif
