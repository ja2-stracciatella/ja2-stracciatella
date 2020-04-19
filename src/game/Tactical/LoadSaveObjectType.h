#ifndef LOADSAVEOBJECTTYPE_H
#define LOADSAVEOBJECTTYPE_H

#include "Item_Types.h"
#include "LoadSaveData.h"


void ExtractObject(DataReader& d, OBJECTTYPE* o);

void InjectObject(DataWriter& d, const OBJECTTYPE* o);

#endif
