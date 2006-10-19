#ifndef __SYSTEM_UTILS
#define __SYSTEM_UTILS

#include "Font.h"
#include "Isometric_Utils.h"
#include "Interface.h"

// Global variables for video objects
extern UINT32					guiRENDERBUFFER;
extern UINT32					guiSAVEBUFFER;
extern UINT32					guiEXTRABUFFER;

extern BOOLEAN gfExtraBuffer;

BOOLEAN	InitializeGameVideoObjects( );


#endif
