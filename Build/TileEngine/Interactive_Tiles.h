#ifndef __INTERACTIVE_TILES_H
#define __INTERACTIVE_TILES_H

#include "Soldier_Control.h"
#include "Types.h"
#include "WorldDef.h"

#define INTTILE_DOOR_TILE_ONE			1

#define INTTILE_DOOR_OPENSPEED	70

#define	INTILE_CHECK_FULL						1
#define	INTILE_CHECK_SELECTIVE			2


BOOLEAN StartInteractiveObject( INT16 sGridNo, UINT16 usStructureID, SOLDIERTYPE *pSoldier, UINT8 ubDirection );
BOOLEAN StartInteractiveObjectFromMouse( SOLDIERTYPE *pSoldier, UINT8 ubDirection );
UINT32  GetInteractiveTileCursor( UINT32 uiOldCursor, BOOLEAN fConfirm );
BOOLEAN InteractWithInteractiveObject( SOLDIERTYPE *pSoldier, STRUCTURE *pStructure, UINT8 ubDirection );
BOOLEAN SoldierHandleInteractiveObject( SOLDIERTYPE *pSoldier );

BOOLEAN CalcInteractiveObjectAPs(INT16 sGridNo, const STRUCTURE* pStructure, INT16* psAPCost, INT16* psBPCost);

void HandleStructChangeFromGridNo( SOLDIERTYPE *pSoldier, INT16 sGridNo );


void BeginCurInteractiveTileCheck( UINT8 bCheckFlags );
void EndCurInteractiveTileCheck(void);
void LogMouseOverInteractiveTile( INT16 sGridNo );
BOOLEAN ShouldCheckForMouseDetections(void);

void CycleIntTileFindStack( UINT16 usMapPos );
void SetActionModeDoorCursorText(void);

LEVELNODE *GetCurInteractiveTile(void);
LEVELNODE *GetCurInteractiveTileGridNo( INT16 *psGridNo );
LEVELNODE *GetCurInteractiveTileGridNoAndStructure( INT16 *psGridNo, STRUCTURE **ppStructure );
LEVELNODE *ConditionalGetCurInteractiveTileGridNoAndStructure( INT16 *psGridNo, STRUCTURE **ppStructure, BOOLEAN fRejectOnTopItems );

BOOLEAN CheckVideoObjectScreenCoordinateInData(HVOBJECT hSrcVObject, UINT16 usIndex, INT32 iTestX, INT32 iTestY);

#endif
