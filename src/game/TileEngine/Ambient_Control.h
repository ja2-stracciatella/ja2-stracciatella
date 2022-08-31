#ifndef __AMBIENT_CONTROL
#define __AMBIENT_CONTROL

#include "Ambient_Types.h"

void HandleNewSectorAmbience( UINT8 ubAmbientID );
UINT32 SetupNewAmbientSound( UINT32 uiAmbientID );

void StopAmbients(void);
void DeleteAllAmbients(void);

inline AMBIENTDATA_STRUCT gAmbData[MAX_AMBIENT_SOUNDS];
inline INT16 gsNumAmbData = 0;

#endif
