//**************************************************************************
//
// Filename :	debug.h
//
//	Purpose :	prototypes for the debug manager
//
// Modification history :
//
//		xxxxx96:LH				- Creation
//		xxnov96:HJH				- made it work
//
//**************************************************************************

#ifndef __DEBUG_MANAGER_
#define __DEBUG_MANAGER_

#include <crtdbg.h>

#include "types.h"
#include "TopicOps.h"
#include "TopicIDs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INVALID_TOPIC		0xffff
#define MAX_TOPICS_ALLOTED	1024


extern BOOLEAN	gfRecordToFile;
extern BOOLEAN	gfRecordToDebugger;
extern UINT32		guiProfileStart, guiExecutions, guiProfileTime;
extern INT32		giProfileCount;

#define	PROFILE(x)		guiProfileStart=GetTickCount();				\
											guiExecutions=x;											\
											for(giProfileCount=0; giProfileCount < x; giProfileCount++)

#define PROFILE_REPORT()		guiProfileTime=(GetTickCount()-guiProfileStart);	\
														_RPT3(_CRT_WARN, "*** PROFILE REPORT: %d executions took %dms, average of %.2fms per iteration.\n", guiExecutions, guiProfileTime, (FLOAT)guiProfileTime/guiExecutions);

extern void			_Null(void);
extern	UINT8		*String(const char *String, ...);



#if defined ( _DEBUG ) || defined ( FORCE_ASSERTS_ON )

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
extern	void		_FailMessage(UINT8 *pString, UINT32 uiLineNum, UINT8 *pSourceFile );

#define Assert(a)										(a) ? _Null() : _FailMessage( NULL, __LINE__, __FILE__ )
#define AssertMsg(a,b)							(a) ? _Null() : _FailMessage(    b, __LINE__, __FILE__ )

extern UINT8			gubAssertString[128];


#else

#define Assert(a)
#define AssertMsg(a,b)

//*******************************************************************************************
#endif

// Moved these out of the defines - debug mgr always initialized
#define InitializeDebugManager()		DbgInitialize()
#define ShutdownDebugManager()			DbgShutdown()

extern	BOOLEAN	DbgInitialize(void);
extern	void		DbgShutdown(void);


#ifdef SGP_DEBUG
// If DEBUG_ is defined, we need to initialize all the debug macros. Otherwise all the
// debug macros will be substituted by blank lines at compile time
//*******************************************************************************************
// Debug Mode
//*******************************************************************************************

extern BOOLEAN		gfDebugTopics[MAX_TOPICS_ALLOTED];
// These are the debug macros (the ones the use will use). The user should never call
// the actual debug functions directly

// Force a breakpoint in the debugger
#define DebugBreakpoint()						__asm { int 3 }


#define DbgMessage(a, b, c)					DbgMessageReal( (UINT16)(a), (UINT8)(TOPIC_MESSAGE), (UINT8)(b), (CHAR8 *)(c) )
#define FastDebugMsg(a)							_DebugMessage( (UINT8 *)(a), (UINT32)(__LINE__), (UINT8 *)(__FILE__) )

#define UnRegisterDebugTopic(a, b)	DbgTopicRegistration( (UINT8)TOPIC_UNREGISTER, (UINT16 *)(&(a)), (CHAR8 *)(b) )
#define ClearAllDebugTopics( )			DbgClearAllTopics( )

#define ErrorMsg(a)									_DebugMessage( (UINT8 *)(a), (UINT32)(__LINE__), (UINT8 *)(__FILE__))

// Enable the debug topic we want
#if defined( JA2 ) || defined( UTIL )
#define RegisterJA2DebugTopic(a, b)	DbgTopicRegistration( TOPIC_REGISTER, &(a), (b) )
#define RegisterDebugTopic(a, b)
#define DebugMsg(a, b, c)						DbgMessageReal( (a), TOPIC_MESSAGE, (b), (c) )
#else
#define RegisterJA2DebugTopic(a, b)
#define RegisterDebugTopic(a, b)		DbgTopicRegistration( (UINT8)TOPIC_REGISTER, (UINT16 *)(&(a)), (CHAR8 *)(b) )
#define DebugMsg(a)									_DebugMessage((UINT8 *)(a), (UINT32)(__LINE__), (UINT8 *)(__FILE__))
#endif

// public interface to debug methods:
extern	void		DbgMessageReal(UINT16 TopicId, UINT8 uiCommand, UINT8 uiDebugLevel, CHAR8 *Str);
extern	BOOLEAN	DbgSetDebugLevel(UINT16 TopicId, UINT8 uiDebugLevel);
extern	void		DbgFailedAssertion( BOOLEAN fExpression, char *szFile, int nLine );
//extern	void		_FailMessage(UINT8 *pString, UINT32 uiLineNum, UINT8 *pSourceFile );
extern	void		DbgTopicRegistration( UINT8 ubCmd, UINT16 *usTopicID, CHAR8 *zMessage );
extern	void		DbgClearAllTopics( void );
extern	void		_DebugMessage(UINT8 *pSourceFile, UINT32 uiLineNum, UINT8 *pString);

//*******************************************************************************************

#else

//*******************************************************************************************
// Release Mode
//*******************************************************************************************
#define DebugBreakpoint()

#define RegisterDebugTopic(a, b)
#define UnRegisterDebugTopic(a, b)
#define ClearAllDebugTopics( )

#define FastDebugMsg(a)
#define ErrorMsg(a)

#define DbgTopicRegistration(a, b, c);
#define DbgMessage(a, b, c)

#if defined( JA2 ) || defined( UTIL )
#define RegisterJA2DebugTopic(a, b)
#define DebugMsg(a, b, c)
#else
#define DebugMsg(a)
#endif

//*******************************************************************************************
#endif


#ifdef __cplusplus
}
#endif

#endif
