#ifndef __VSURFACE_PRIVATE_
#define __VSURFACE_PRIVATE_

#include "VSurface.h"

// ***********************************************************************
//
// PRIVATE, INTERNAL Header used by other SGP Internal modules
//
// Allows direct access to underlying Direct Draw Implementation
//
// ***********************************************************************

LPDIRECTDRAWSURFACE2 GetVideoSurfaceDDSurface( HVSURFACE hVSurface );

#endif
