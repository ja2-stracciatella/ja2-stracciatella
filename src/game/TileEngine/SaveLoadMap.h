#ifndef _SAVELOADMAP__H_
#define _SAVELOADMAP__H_

#include "WorldDef.h"
#include "Exit_Grids.h"

//Used for the ubType in the MODIFY_MAP struct
enum
{
	SLM_NONE,

	//Adding a map graphic
	SLM_LAND,
	SLM_OBJECT,
	SLM_STRUCT,
	SLM_SHADOW,
	SLM_MERC, //Should never be used
	SLM_ROOF,
	SLM_ONROOF,
	SLM_TOPMOST, //Should never be used

	// For Removing
	SLM_REMOVE_LAND,
	SLM_REMOVE_OBJECT,
	SLM_REMOVE_STRUCT,
	SLM_REMOVE_SHADOW,
	SLM_REMOVE_MERC, //Should never be used
	SLM_REMOVE_ROOF,
	SLM_REMOVE_ONROOF,
	SLM_REMOVE_TOPMOST, //Should never be used

	//Smell, or Blood is used
	SLM_BLOOD_SMELL,

	// Damage a particular struct
	SLM_DAMAGED_STRUCT,

	//Exit Grids
	SLM_EXIT_GRIDS,

	// State of Openable structs
	SLM_OPENABLE_STRUCT,

	// Modify window graphic & structure
	SLM_WINDOW_HIT,
};

struct MODIFY_MAP
{
	UINT16 usGridNo;		//The gridno the graphic will be applied to
	UINT16 usImageType;		//graphic index
	UINT16 usSubImageIndex;	//
	//UINT16 usIndex;
	UINT8  ubType;		// the layer it will be applied to

	UINT8 ubExtra;	// Misc. variable used to strore arbritary values
};


// Use this as sentry, so map changes are added to the map temp file
class ApplyMapChangesToMapTempFile
{
	public:
		ApplyMapChangesToMapTempFile(bool const active = true)
		{
			active_ = active;
		}

		~ApplyMapChangesToMapTempFile() { active_ = false; }

		static bool IsActive() { return active_; }

	private:
		static bool active_;
};


//Applies a change TO THE MAP TEMP file
void AddStructToMapTempFile( UINT32 iMapIndex, UINT16 usIndex );


void AddObjectToMapTempFile( UINT32 uiMapIndex, UINT16 usIndex );


void LoadAllMapChangesFromMapTempFileAndApplyThem(void);


void RemoveStructFromMapTempFile( UINT32 uiMapIndex, UINT16 usIndex );

void AddRemoveObjectToMapTempFile( UINT32 uiMapIndex, UINT16 usIndex );

void SaveBloodSmellAndRevealedStatesFromMapToTempFile(void);


void SaveRevealedStatusArrayToRevealedTempFile(const SGPSector& sector);

void LoadRevealedStatusArrayFromRevealedTempFile(void);


void RemoveStructFromUnLoadedMapTempFile(UINT32 uiMapIndex, UINT16 usIndex, const SGPSector& sSector);
void AddStructToUnLoadedMapTempFile(UINT32 uiMapIndex, UINT16 usIndex, const SGPSector& sSector);

//Adds the exit grid to
void AddExitGridToMapTempFile( UINT16 usGridNo, EXITGRID *pExitGrid, const SGPSector& sector);


//This function removes a struct with the same MapIndex and graphic index from the given sectors temp file
BOOLEAN RemoveGraphicFromTempFile( UINT32 uiMapIndex, UINT16 usIndex, INT16 sSectorX, INT16 sSectorY, UINT8 ubSectorZ );


void AddWindowHitToMapTempFile( UINT32 uiMapIndex );

void ChangeStatusOfOpenableStructInUnloadedSector(const SGPSector& sector, UINT16 usGridNo, BOOLEAN fChangeToOpen);

#endif
