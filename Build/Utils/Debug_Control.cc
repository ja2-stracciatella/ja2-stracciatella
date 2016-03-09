#include "Types.h"
#include "Debug_Control.h"

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
