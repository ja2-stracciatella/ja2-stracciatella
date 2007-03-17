#ifndef __SGP_
#define __SGP_

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern BOOLEAN		gfProgramIsRunning; // Turn this to FALSE to exit program
extern CHAR8			gzCommandLine[100];	// Command line given

void ShutdownWithErrorBox(CHAR8 *pcMessage);

#ifdef __cplusplus
}
#endif

#endif
