#ifndef __TACTICAL_SAVE_H_
#define __TACTICAL_SAVE_H_


#include "World_Items.h"
#include "WorldDef.h"
#include "Rotting_Corpses.h"

#include "Soldier_Profile_Type.h"


#define		MAPS_DIR		"Temp/"


// Add
BOOLEAN AddMapModification( INT16 sMapX, INT16 sMapY, INT8 bMapZ );


//Load the Map modifications from the saved game file
BOOLEAN	LoadMapTempFilesFromSavedGameFile( HWFILE hFile );

//Save the Map Temp files to the saved game file
BOOLEAN SaveMapTempFilesToSavedGameFile( HWFILE hFile );


//Retrieves the number of items in the sectors temp item file
BOOLEAN GetNumberOfWorldItemsFromTempItemFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ, UINT32* puiNumberOfItems);

//Saves the Current Sectors, ( world Items, rotting corpses, ... )  to the temporary file used to store the sectors items
BOOLEAN SaveCurrentSectorsInformationToTempItemFile(void);

//Loads the Currents Sectors information ( world Items, rotting corpses, ... ) from the temporary file used to store the sectores items
BOOLEAN LoadCurrentSectorsInformationFromTempItemsFile(void);

// Loads a World Item array from that sectors temp item file
BOOLEAN LoadWorldItemsFromTempItemFile( INT16 sMapX, INT16 sMapY, INT8 bMapZ, WORLDITEM *pData );

//  Adds an array of Item Objects to the specified location on a unloaded map.
//  If you want to overwrite all the items in the array set fReplaceEntireFile to TRUE.
BOOLEAN AddItemsToUnLoadedSector(INT16 sMapX, INT16 sMapY, INT8 bMapZ, INT16 sGridNo, UINT32 uiNumberOfItems, const OBJECTTYPE* pObject, UINT8 ubLevel, UINT16 usFlags, INT8 bRenderZHeightAboveLevel, INT8 bVisible, BOOLEAN fReplaceEntireFile);


BOOLEAN AddWorldItemsToUnLoadedSector(INT16 sMapX, INT16 sMapY, INT8 bMapZ, INT16 sGridNo, UINT32 uiNumberOfItems, const WORLDITEM* pWorldItem, BOOLEAN fOverWrite);

//Deletes all the Temp files in the Maps\Temp directory
BOOLEAN InitTacticalSave( BOOLEAN fCreateTempDir );


//Call this function to set the new sector a NPC will travel to
void ChangeNpcToDifferentSector(MERCPROFILESTRUCT* npc, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);


// Adds a rotting corpse definition to the end of a sectors rotting corpse temp file
BOOLEAN AddRottingCorpseToUnloadedSectorsRottingCorpseFile( INT16 sMapX, INT16 sMapY, INT8 bMapZ, ROTTING_CORPSE_DEFINITION *pRottingCorpseDef );



//Flags used for the AddDeadSoldierToUnLoadedSector() function
#define		ADD_DEAD_SOLDIER_USE_GRIDNO									0x00000001				// just place the items and corpse on the gridno location
#define		ADD_DEAD_SOLDIER_TO_SWEETSPOT								0x00000002				// Finds the closet free gridno

#define		ADD_DEAD_SOLDIER__USE_JFK_HEADSHOT_CORPSE		0x00000040				// Will ue the JFK headshot



//Pass in the sector to add the dead soldier to.
//The gridno if you are passing in either of the flags ADD_DEAD_SOLDIER_USE_GRIDNO, or the ADD_DEAD_SOLDIER_TO_SWEETSPOT
//
// This function DOES NOT remove the soldier from the soldier struct.  YOU must do it.
BOOLEAN AddDeadSoldierToUnLoadedSector( INT16 sMapX, INT16 sMapY, UINT8 bMapZ, SOLDIERTYPE *pSoldier, INT16  sGridNo, UINT32 uiFlags );


BOOLEAN GetSectorFlagStatus( INT16 sMapX, INT16 sMapY, UINT8 bMapZ, UINT32 uiFlagToSet );
BOOLEAN SetSectorFlag( INT16 sMapX, INT16 sMapY, UINT8 bMapZ, UINT32 uiFlagToSet );
BOOLEAN ReSetSectorFlag( INT16 sMapX, INT16 sMapY, UINT8 bMapZ, UINT32 uiFlagToSet );


//Saves the NPC temp Quote file to the saved game file
BOOLEAN LoadTempNpcQuoteArrayToSaveGameFile( HWFILE hFile );

//Loads the NPC temp Quote file from the saved game file
BOOLEAN SaveTempNpcQuoteArrayToSaveGameFile( HWFILE hFile );

UINT32 MercChecksum( SOLDIERTYPE * pSoldier );
UINT32 ProfileChecksum( MERCPROFILESTRUCT * pProfile );
BOOLEAN JA2EncryptedFileRead(HWFILE hFile, PTR pDest, UINT32 uiBytesToRead);
BOOLEAN JA2EncryptedFileWrite(HWFILE hFile, PTR pDest, UINT32 uiBytesToWrite);

BOOLEAN NewJA2EncryptedFileRead(HWFILE hFile, PTR pDest, UINT32 uiBytesToRead);
BOOLEAN NewJA2EncryptedFileWrite(HWFILE hFile, PTR pDest, UINT32 uiBytesToWrite);


//If hacker's mess with our save/temp files, this is our final line of defence.
void InitExitGameDialogBecauseFileHackDetected(void);

void HandleAllReachAbleItemsInTheSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ );

void GetMapTempFileName(UINT32 uiType, char* pMapName, INT16 sMapX, INT16 sMapY, INT8 bMapZ);


UINT32	GetNumberOfVisibleWorldItemsFromSectorStructureForSector( INT16 sMapX, INT16 sMapY, INT8 bMapZ );
void		SetNumberOfVisibleWorldItemsInSectorStructureForSector( INT16 sMapX, INT16 sMapY, INT8 bMapZ, UINT32 uiNumberOfItems );

#define NEW_ROTATION_ARRAY_SIZE 49
#define BASE_NUMBER_OF_ROTATION_ARRAYS 19


BOOLEAN SaveWorldItemsToTempItemFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ, UINT32 uiNumberOfItems, const WORLDITEM* pData);

#endif
