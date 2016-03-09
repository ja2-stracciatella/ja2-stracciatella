#ifndef __DEBUG_CONTROL_
#define __DEBUG_CONTROL_

#include "Types.h"


//#define		_PHYSICSSUBSYSTEM_DEBUG
//#define		_AISUBSYSTEM_DEBUG

#ifdef JA2BETAVERSION
//  #define			_ANIMSUBSYSTEM_DEBUG
#endif


void LiveMessage(const char *strMessage);

#ifdef _PHYSICSSUBSYSTEM_DEBUG

#define PhysicsDebugMsg(c)				PhysicsDbgMessage( (c) )

extern	void		PhysicsDbgMessage( CHAR8 *Str);

#else

#define PhysicsDebugMsg(c) (void)0

#endif



#ifdef _AISUBSYSTEM_DEBUG

#define AiDebugMsg(c)				AiDbgMessage( (c) )

extern	void		AiDbgMessage( CHAR8 *Str);

#else

#define AiDebugMsg(c) (void)0

#endif

#endif
