#ifndef _AMBIENT_TYPES_H
#define _AMBIENT_TYPES_H

#include "sgp/Types.h"


#define MAX_AMBIENT_SOUNDS		100


#define AMB_TOD_DAWN					0
#define AMB_TOD_DAY						1
#define AMB_TOD_DUSK					2
#define AMB_TOD_NIGHT					3

struct AMBIENTDATA_STRUCT
{
	UINT32				uiMinTime;
	UINT32				uiMaxTime;
	UINT8					ubTimeCatagory;
	SGPFILENAME		zFilename;
	UINT32				uiVol;
};


#endif
