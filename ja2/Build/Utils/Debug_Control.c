#ifdef PRECOMPILEDHEADERS
	#include "Utils_All.h"
#else
	#include "Types.h"
	#include "Debug_Control.h"
	#include "stdio.h"
#endif



#ifdef _ANIMSUBSYSTEM_DEBUG

void AnimDbgMessage( CHAR8 *strMessage)
{
  FILE      *OutFile;

	if ((OutFile = fopen("AnimDebug.txt", "a+t")) != NULL)
	{
	  fprintf(OutFile, "%s\n", strMessage);
		fclose(OutFile);
	}
}

#endif


#ifdef _PHYSICSSUBSYSTEM_DEBUG

void PhysicsDbgMessage( CHAR8 *strMessage)
{
  FILE      *OutFile;

	if ((OutFile = fopen("PhysicsDebug.txt", "a+t")) != NULL)
	{
	  fprintf(OutFile, "%s\n", strMessage);
		fclose(OutFile);
	}
}

#endif



#ifdef _AISUBSYSTEM_DEBUG

void AiDbgMessage( CHAR8 *strMessage)
{
  FILE      *OutFile;

	if ((OutFile = fopen("AiDebug.txt", "a+t")) != NULL)
	{
	  fprintf(OutFile, "%s\n", strMessage);
		fclose(OutFile);
	}
}

#endif


void LiveMessage(const char *strMessage)
{
  FILE      *OutFile;

	if ((OutFile = fopen("Log.txt", "a+t")) != NULL)
	{
	  fprintf(OutFile, "%s\n", strMessage);
		fclose(OutFile);
	}
}
