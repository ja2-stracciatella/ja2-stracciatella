#pragma once
#include "Observable.h"
#include "Types.h"
class DataReader;
class DataWriter;
class StrategicMapSecretModel;

extern Observable<UINT8> OnMapSecretFound;

// Reset the state of all map secrets
void InitMapSecrets();

// Whether the town was found by the player. Returns true if the town is not hidden (i.e. not defined in map secrets)
BOOLEAN IsTownFound(INT8 bTownID);

// Whether the map secret was known to the player. Returns true with a warning, if there is no secret at the sector ID.
BOOLEAN IsSecretFoundAt(UINT8 sectorID);

// Marks the town's base sector as a found secret
void SetTownAsFound(INT8 bTownID, BOOLEAN fFound = true);

// set this SAM site as being found by the player
void SetSAMSiteAsFound(UINT8 uiSamIndex);

// Marks a sector's secret as found
void SetSectorSecretAsFound(UINT8 ubSectorID, BOOLEAN fFound = true);

const StrategicMapSecretModel* GetMapSecretBySectorID(UINT8 ubSectorID);

// writes the map secret states of all SAM sites
void InjectSAMSitesFoundToSavedFile(DataWriter& d);

// reads the map secret states for all SAM sites
void ExtractSAMSitesFoundFromSavedFile(DataReader& d);

// write the state of a non-SAM-site map secret at the given index
void InjectMapSecretStateToSave(DataWriter& d, unsigned int index);

// Gets a non-SAM-site map secret state by index
BOOLEAN GetMapSecretStateForSave(unsigned int index);

// reads a non-SAM-site map secret state from save
void ExtractMapSecretStateFromSave(DataReader & d, unsigned int index);

// Sets a non-SAM-site map secret state by index
void SetMapSecretStateFromSave(unsigned int index, BOOLEAN fFound);