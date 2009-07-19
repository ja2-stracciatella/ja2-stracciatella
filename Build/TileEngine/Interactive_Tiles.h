#ifndef __INTERACTIVE_TILES_H
#define __INTERACTIVE_TILES_H

#include "Interface_Cursors.h"
#include "JA2Types.h"
#include "WorldDef.h"


#define INTTILE_DOOR_OPENSPEED	70


BOOLEAN StartInteractiveObject( INT16 sGridNo, UINT16 usStructureID, SOLDIERTYPE *pSoldier, UINT8 ubDirection );
BOOLEAN StartInteractiveObjectFromMouse( SOLDIERTYPE *pSoldier, UINT8 ubDirection );
UICursorID GetInteractiveTileCursor(UICursorID old_cursor, BOOLEAN fConfirm);
void InteractWithInteractiveObject(SOLDIERTYPE&, STRUCTURE&, UINT8 direction);
bool SoldierHandleInteractiveObject(SOLDIERTYPE&);

BOOLEAN CalcInteractiveObjectAPs(INT16 sGridNo, const STRUCTURE* pStructure, INT16* psAPCost, INT16* psBPCost);

void HandleStructChangeFromGridNo(SOLDIERTYPE*, GridNo);


void BeginCurInteractiveTileCheck(void);
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
