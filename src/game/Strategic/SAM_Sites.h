#pragma once

#include "Observable.h"
#include "JA2Types.h"

#define NUMBER_OF_SAMS 4

// min condition for sam site to be functional
#define MIN_CONDITION_FOR_SAM_SITE_TO_WORK 80

extern Observable<> OnAirspaceControlUpdated;

void InitializeSAMSites();

void UpdateSAMDoneRepair(const SGPSector& sector);

void UpdateAirspaceControl();

bool IsThisSectorASAMSector(const SGPSector& sector);

bool DoesSAMExistHere(const SGPSector& sector, GridNo const gridno);

// number of SAM sites under player control
INT32 GetNumberOfSAMSitesUnderPlayerControl();

bool IsThereAFunctionalSAMSiteInSector(const SGPSector& sector);

INT8 GetSAMIdFromSector(INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);
