#ifndef __DEBUG_CONTROL_
#define __DEBUG_CONTROL_

#include "Types.h"


//#define		_PHYSICSSUBSYSTEM_DEBUG
//#define		_AISUBSYSTEM_DEBUG

#ifdef JA2BETAVERSION
//  #define			_ANIMSUBSYSTEM_DEBUG
#endif


void LiveMessage( CHAR8 *strMessage);


#ifdef _ANIMSUBSYSTEM_DEBUG

#define AnimDebugMsg(c)				AnimDbgMessage( (c) )

extern	void		AnimDbgMessage( CHAR8 *Str);

#else

#define AnimDebugMsg(c)

#endif


#ifdef _PHYSICSSUBSYSTEM_DEBUG

#define PhysicsDebugMsg(c)				PhysicsDbgMessage( (c) )

extern	void		PhysicsDbgMessage( CHAR8 *Str);

#else

#define PhysicsDebugMsg(c)

#endif



#ifdef _AISUBSYSTEM_DEBUG

#define AiDebugMsg(c)				AiDbgMessage( (c) )

extern	void		AiDbgMessage( CHAR8 *Str);

#else

#define AiDebugMsg(c)

#endif

#endif
