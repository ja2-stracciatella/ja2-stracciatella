#ifndef __TACTICAL_SAVE_H_
#define __TACTICAL_SAVE_H_

#include "Campaign_Types.h"
#include "Handle_Items.h"
#include "World_Items.h"
#include "WorldDef.h"
#include "Rotting_Corpses.h"

#include "Soldier_Profile_Type.h"


//Load the Map modifications from the saved game file
void LoadMapTempFilesFromSavedGameFile(HWFILE, UINT32 savegame_version);

//Save the Map Temp files to the saved game file
void SaveMapTempFilesToSavedGameFile(HWFILE);


//Saves the Current Sectors, ( world Items, rotting corpses, ... )  to the temporary file used to store the sectors items
void SaveCurrentSectorsInformationToTempItemFile(void);

//Loads the Currents Sectors information ( world Items, rotting corpses, ... ) from the temporary file used to store the sectores items
void LoadCurrentSectorsInformationFromTempItemsFile();

// Loads a World Item array from that sectors temp item file
void LoadWorldItemsFromTempItemFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ, UINT32* item_count, WORLDITEM** items);

//  Adds an array of Item Objects to the specified location on a unloaded map.
//  If you want to overwrite all the items in the array set fReplaceEntireFile to TRUE.
void AddItemsToUnLoadedSector(INT16 sMapX, INT16 sMapY, INT8 bMapZ, INT16 sGridNo, UINT32 uiNumberOfItems, OBJECTTYPE const* pObject, UINT8 ubLevel, UINT16 usFlags, INT8 bRenderZHeightAboveLevel, Visibility);


void AddWorldItemsToUnLoadedSector(INT16 sMapX, INT16 sMapY, INT8 bMapZ, UINT32 uiNumberOfItems, const WORLDITEM* pWorldItem);

// Delete all the files in the temp directory.
void InitTacticalSave();


//Call this function to set the new sector a NPC will travel to
void ChangeNpcToDifferentSector(MERCPROFILESTRUCT&, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);


// Adds a rotting corpse definition to the end of a sectors rotting corpse temp file
void AddRottingCorpseToUnloadedSectorsRottingCorpseFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ, ROTTING_CORPSE_DEFINITION const*);



//Flags used for the AddDeadSoldierToUnLoadedSector() function
#define ADD_DEAD_SOLDIER_USE_GRIDNO			0x00000001 // just place the items and corpse on the gridno location
#define ADD_DEAD_SOLDIER_TO_SWEETSPOT			0x00000002 // Finds the closet free gridno

#define ADD_DEAD_SOLDIER__USE_JFK_HEADSHOT_CORPSE	0x00000040 // Will ue the JFK headshot



//Pass in the sector to add the dead soldier to.
//The gridno if you are passing in either of the flags ADD_DEAD_SOLDIER_USE_GRIDNO, or the ADD_DEAD_SOLDIER_TO_SWEETSPOT
//
// This function DOES NOT remove the soldier from the soldier struct.  YOU must do it.
void AddDeadSoldierToUnLoadedSector(INT16 sMapX, INT16 sMapY, UINT8 bMapZ, SOLDIERTYPE* pSoldier, INT16 sGridNo, UINT32 uiFlags);


BOOLEAN GetSectorFlagStatus(INT16 sMapX, INT16 sMapY, UINT8 bMapZ, SectorFlags);
void    SetSectorFlag(INT16 sMapX, INT16 sMapY, UINT8 bMapZ, SectorFlags);
void    ReSetSectorFlag(INT16 sMapX, INT16 sMapY, UINT8 bMapZ, SectorFlags);


//Saves the NPC temp Quote file to the saved game file
void LoadTempNpcQuoteArrayToSaveGameFile(HWFILE);

//Loads the NPC temp Quote file from the saved game file
void SaveTempNpcQuoteArrayToSaveGameFile(HWFILE);

void JA2EncryptedFileRead( HWFILE, BYTE*       data, UINT32 uiBytesToRead);
void JA2EncryptedFileWrite(HWFILE, BYTE const* data, UINT32 uiBytesToWrite);

void NewJA2EncryptedFileRead( HWFILE, BYTE*       data, UINT32 uiBytesToRead);
void NewJA2EncryptedFileWrite(HWFILE, BYTE const* data, UINT32 uiBytesToWrite);


//If hacker's mess with our save/temp files, this is our final line of defence.
void InitExitGameDialogBecauseFileHackDetected(void);

void HandleAllReachAbleItemsInTheSector(INT16 x, INT16 y, INT8 z);

void GetMapTempFileName(SectorFlags uiType, char* pMapName, INT16 sMapX, INT16 sMapY, INT8 bMapZ);


UINT32	GetNumberOfVisibleWorldItemsFromSectorStructureForSector( INT16 sMapX, INT16 sMapY, INT8 bMapZ );
void		SetNumberOfVisibleWorldItemsInSectorStructureForSector( INT16 sMapX, INT16 sMapY, INT8 bMapZ, UINT32 uiNumberOfItems );

#define NEW_ROTATION_ARRAY_SIZE			49
#define BASE_NUMBER_OF_ROTATION_ARRAYS			19


void SaveWorldItemsToTempItemFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ, UINT32 uiNumberOfItems, WORLDITEM const* pData);

#endif
