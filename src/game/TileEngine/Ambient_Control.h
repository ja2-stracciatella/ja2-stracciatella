#ifndef __AMBIENT_CONTROL
#define __AMBIENT_CONTROL

#include "Ambient_Types.h"

void HandleNewSectorAmbience( UINT8 ubAmbientID );
UINT32 SetupNewAmbientSound( UINT32 uiAmbientID );

void StopAmbients(void);
void DeleteAllAmbients(void);

extern AMBIENTDATA_STRUCT		gAmbData[ MAX_AMBIENT_SOUNDS ];
extern INT16								gsNumAmbData;

#endif
