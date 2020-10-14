#pragma once

#include "Observable.h"
#include "JA2Types.h"

#define NUMBER_OF_SAMS 4

// min condition for sam site to be functional
#define MIN_CONDITION_FOR_SAM_SITE_TO_WORK 80

extern Observable<> OnAirspaceControlUpdated;

void InitializeSAMSites();

void UpdateSAMDoneRepair(INT16 const x, INT16 const y, INT16 const z);

void UpdateAirspaceControl();

bool IsThisSectorASAMSector(INT16 x, INT16 y, INT8 z);

bool DoesSAMExistHere(INT16 const x, INT16 const y, INT16 const z, GridNo const gridno);

// number of SAM sites under player control
INT32 GetNumberOfSAMSitesUnderPlayerControl();

// is there a FUNCTIONAL SAM site in this sector?
bool IsThereAFunctionalSAMSiteInSector(INT16 x, INT16 y, INT8 z);

INT8 GetSAMIdFromSector(INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);
