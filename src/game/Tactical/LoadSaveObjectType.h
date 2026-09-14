#ifndef LOADSAVEOBJECTTYPE_H
#define LOADSAVEOBJECTTYPE_H

#include "Item_Types.h"
#include "LoadSaveData.h"


// Bytes one OBJECTTYPE takes up in a saved game, as written by InjectObject().
#define SIZE_OF_SAVED_OBJECTTYPE 36

void ExtractObject(DataReader& d, OBJECTTYPE* o);

void InjectObject(DataWriter& d, const OBJECTTYPE* o);

#endif
