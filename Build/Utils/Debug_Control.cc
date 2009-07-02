#include "Types.h"
#include "Debug_Control.h"


#ifdef _ANIMSUBSYSTEM_DEBUG

void AnimDbgMessage( CHAR8 *strMessage)
{
  FILE      *OutFile;

	if ((OutFile = fopen("AnimDebug.txt", "a+")) != NULL)
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

	if ((OutFile = fopen("PhysicsDebug.txt", "a+")) != NULL)
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

	if ((OutFile = fopen("AiDebug.txt", "a+")) != NULL)
	{
	  fprintf(OutFile, "%s\n", strMessage);
		fclose(OutFile);
	}
}

#endif


void LiveMessage(const char *strMessage)
{
  FILE      *OutFile;

	if ((OutFile = fopen("Log.txt", "a+")) != NULL)
	{
	  fprintf(OutFile, "%s\n", strMessage);
		fclose(OutFile);
	}
}
