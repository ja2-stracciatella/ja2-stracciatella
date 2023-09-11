#ifndef __EDITOR_UNDO_H
#define __EDITOR_UNDO_H

#include "Types.h"

BOOLEAN AddToUndoList( INT32 iMapIndex );
void AddLightToUndoList(INT32 iMapIndex, INT32 iLightRadius);

void    RemoveAllFromUndoList(void);
BOOLEAN ExecuteUndoList(void);

void EnableUndo(void);
void DisableUndo(void);

void DetermineUndoState(void);

// Undo command flags
#define MAX_UNDO_COMMAND_LENGTH		10

#endif
