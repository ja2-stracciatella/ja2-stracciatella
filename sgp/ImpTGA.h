//**************************************************************************
//
// Filename :	impTGA.h
//
//	Purpose :	.tga file importer function prototypes
//
// Modification history :
//
//		20nov96:HJH				- Creation
//
//**************************************************************************

#ifndef _impTGA_h
#define _impTGA_h

#include "Types.h"
#include "HImage.h"


#ifdef __cplusplus
extern "C" {
#endif

BOOLEAN LoadTGAFileToImage( HIMAGE hImage,  UINT16 fContents );

#ifdef __cplusplus
}
#endif

#endif
