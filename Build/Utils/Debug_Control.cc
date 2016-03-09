#include "Types.h"
#include "Debug_Control.h"

void LiveMessage(const char *strMessage)
{
  FILE      *OutFile;

	if ((OutFile = fopen("Log.txt", "a+")) != NULL)
	{
	  fprintf(OutFile, "%s\n", strMessage);
		fclose(OutFile);
	}
}
