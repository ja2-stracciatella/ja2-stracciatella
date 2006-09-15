#ifndef __SGP_
#define __SGP_

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern BOOLEAN		gfProgramIsRunning; // Turn this to FALSE to exit program
extern CHAR8			gzCommandLine[100];	// Command line given
extern UINT8			gbPixelDepth;				// GLOBAL RUN-TIME SETTINGS
extern BOOLEAN		gfDontUseDDBlits;		// GLOBAL FOR USE OF DD BLITTING

#if !defined(JA2) && !defined(UTILS)
extern BOOLEAN		gfLoadAtStartup;
extern CHAR8		*gzStringDataOverride;
extern BOOLEAN		gfUsingBoundsChecker;
extern BOOLEAN		gfCapturingVideo;

#endif

// function prototypes
void SGPExit(void);
void ShutdownWithErrorBox(CHAR8 *pcMessage);

#ifdef __cplusplus
}
#endif

#endif
